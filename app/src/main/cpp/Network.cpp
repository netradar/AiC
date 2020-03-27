//
// Created by vanxum516 on 2020/3/27.
//

#include <opus/vmtl.h>
#include "com_vanxum_Aic_NetworkJni.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "android/log.h"
#include <string.h>
#include <android/log.h>
#include <jni.h>
#include <stdio.h>

#define  LOG_TAG    "native-dev"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/*
 * Class:     com_vanxum_Aic_NetworkJni
 * Method:    vmtlInit
 * Signature: ([BI)I
 */
JavaVM*     javaVM;
jobject     javaObj;
jmethodID   reportVideoFormat_callBack;
jmethodID   reportAudioFormat_callBack;
jmethodID   reportVideoData_callBack;


typedef struct {
    void *conn_;
    int sock_[2];
    int chn_num_;
    uint16_t stream_id_;
    int bind_id_;
    int local_port_;
    int remote_port_;
    char local_ip_[16];
    char remote_ip_[16];
    int is_recv_thread_start_;
    int is_vmtl_connected_;
    vmtlConnCb_t cb_;

} Context;

Context cxtVideo;
#define SESSION_ID 0x327

static vmtlCbRetVal_t onReportData(void *obj, char *data, int len, outDataDesp_t *desp) {
    Context *cxt = (Context *)obj;


    JNIEnv *env;
    javaVM->AttachCurrentThread(&env, NULL);

    jbyteArray jbuff = (env)->NewByteArray(len);

    (env)->SetByteArrayRegion(jbuff,0,len,(jbyte *)data);
    env->CallVoidMethod(javaObj,reportVideoData_callBack, jbuff,len);

    (env)->DeleteLocalRef(jbuff);

    return kRetNormal;
}
static int onSource(void *obj, const char *sendData, int len, int channel, void *sourceContext, outDataDesp_t *desp) {
    Context *cxt = (Context *)obj;

    if( send(cxt->sock_[channel], sendData + VMTL_MAX_RELAY_PACKET_LEN, len, 0) <= 0 ) {

    }

    return 0;
}
static int createSocket(int localPort, int remotePort, const char* localIP, const char* remoteIP) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {

        return -1;
    }

    int size = 8*1024*1024;
    int buf_len = -1;

    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const void *)&size, (socklen_t) sizeof(size));
    socklen_t len = sizeof(buf_len);
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void *) &buf_len, &len);


    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = inet_addr(localIP);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {

        return -1;
    }

    struct sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(remotePort);
    remoteAddr.sin_addr.s_addr = inet_addr(remoteIP);
    if (connect(sock, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr)) != 0) {

        return -1;
    }


    return sock;
}

static void _recvThread(Context *cxt, int channel)
{
    outDataDesp_t desp = {0};
    char *sink_buff = NULL;
    int recv_len = 0;

    while(cxt->is_recv_thread_start_)
    {
        vmtlNotification_t conn_state = vmtl_get_conn_status(cxt->conn_);
        if (conn_state != kConnected && conn_state != kConnecting) { continue; }

        vmtl_get_sink_buff(cxt->conn_, channel, &sink_buff);
        if (!sink_buff)
        {
            printf("failed to get sink buff!\n");
            continue;
        }

        recv_len = recv(cxt->sock_[channel], sink_buff, 1500, 0); //1500 is MAX_VMTL_MTU
        if (recv_len <= 0) { printf("recv failed with err: %s\n", strerror(errno)); continue; }

        vmtl_sink_data(cxt->conn_, sink_buff, recv_len, channel, NULL);

        //printf("recv %d bytes\n", recv_len);
    }
}
static void* recvMediaThread(void *obj)
{
    Context *cxt = (Context *)obj;

    _recvThread(cxt, 0);
}

static void* recvCtrlThread(void *obj)
{
    Context *cxt = (Context *)obj;

    _recvThread(cxt, 1);
}

static void createRecvThreads(Context *cxt)
{
    pthread_attr_t attr;
    pthread_t media_thread, ctrl_thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    cxt->is_recv_thread_start_ = 1;

    if(pthread_create(&media_thread, &attr, recvMediaThread, (void *)cxt) != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "lichao", "%s", "thread error media");
    }

    if(pthread_create(&ctrl_thread, &attr, recvCtrlThread, (void *)cxt) != 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "lichao", "%s", "thread error ctrl");
    }

    pthread_attr_destroy(&attr);
}
static void onVmtlEventNotify(void *obj, vmtlNotification_t event, void *data) {
    Context *cxt = (Context *)obj;
    switch (event) {
        case kConnected:
            cxt->is_vmtl_connected_ = 1;

            break;

        default:
            break;
    }
}
JNIEXPORT jint JNICALL Java_com_vanxum_Aic_NetworkJni_vmtlInit
  (JNIEnv * env, jobject obj, jstring local_ip_,jstring remote_ip_, jint local_port,jint remote_port)
{
    const char *local_ip = env->GetStringUTFChars(local_ip_, 0);
    const char *remote_ip = env->GetStringUTFChars(remote_ip_, 0);

    JavaVM *g_jvm;
    env->GetJavaVM(&g_jvm);
    javaVM = g_jvm;

    jobject g_obj = env->NewGlobalRef(obj);
    javaObj = g_obj;

    jclass cls = env->GetObjectClass(obj);
    reportVideoData_callBack   = env->GetMethodID(cls, "reportVideoData", "([BI)V");



    cxtVideo.is_recv_thread_start_ = 0;
    cxtVideo.is_vmtl_connected_    = 0;
    cxtVideo.chn_num_ = 2;
    cxtVideo.cb_.notifier = onVmtlEventNotify;
    cxtVideo.cb_.report_data = onReportData;
    cxtVideo.cb_.source = onSource;

    vmtl_init();
    int ver = vmtl_get_vernum();

    char ver_string[MAX_VERSION_STRING_LENGTH];
    char *s = ver_string;
    vmtl_get_verstr(&s);

    vmtlMode_t mode;
    vmtl_mode_get(&mode);
    mode.atype = kAtypeClient;
    vmtl_mode_set(&mode);
    vmtl_mode_get(&mode);

    vmtl_create_stream(kVGTP, kStrmDirRecv, "test_rcv_avc_strm", &cxtVideo.stream_id_);

    LOGI("stread_id is %d ",cxtVideo.stream_id_);



    vmtl_create_conn(&cxtVideo, &cxtVideo.conn_, cxtVideo.cb_, cxtVideo.chn_num_, kConnPropBigStream);

    cxtVideo.local_port_ = local_port;
    cxtVideo.remote_port_ = remote_port;
    strcpy(cxtVideo.local_ip_, local_ip);
    strcpy(cxtVideo.remote_ip_, remote_ip);
    cxtVideo.sock_[0] = createSocket(cxtVideo.local_port_, cxtVideo.remote_port_, cxtVideo.local_ip_, cxtVideo.remote_ip_);
    cxtVideo.sock_[1] = createSocket(cxtVideo.local_port_ + 1, cxtVideo.remote_port_ + 1, cxtVideo.local_ip_, cxtVideo.remote_ip_);

    if(cxtVideo.sock_[0]<0||cxtVideo.sock_[1]<0)
    {
        LOGE("socket create failed");
    }
    else
    {
        LOGI("socket create successfully");
    }
    createRecvThreads(&cxtVideo);


    vmtl_bind_stream(cxtVideo.conn_, cxtVideo.stream_id_, &cxtVideo.bind_id_);


    connConfig_t cc_conf = {0};
    cc_conf.session = (void *) SESSION_ID;
    vmtl_config_conn(cxtVideo.conn_, &cc_conf);

    vmtl_conn_active(cxtVideo.conn_, kConnAtypeClient);

    env->ReleaseStringUTFChars(local_ip_, local_ip);
    env->ReleaseStringUTFChars(remote_ip_, remote_ip);
    return 0;

}
/*
 * Class:     com_vanxum_Aic_NetworkJni
 * Method:    startVmtl
 * Signature: (Z)I
 */
JNIEXPORT jint JNICALL Java_com_vanxum_Aic_NetworkJni_startVmtl
  (JNIEnv *, jobject, jboolean)
{

    return 0;
}

/*
 * Class:     com_vanxum_Aic_NetworkJni
 * Method:    sendInputEvent
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_vanxum_Aic_NetworkJni_sendInputEvent
  (JNIEnv *, jobject)
{

}
