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
#include "pthread.h"

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


void *recvMediaThread(void *obj);
void *recvCtrlThread(void *obj);

typedef struct {
    void *conn_;
    int sock_[2];
    int chn_num_;
    uint16_t stream_id_;
    uint16_t hidStream_id;
    int bind_id_;
    int hidBind_id;
    int local_port_;
    int remote_port_;
    char local_ip_[16];
    char remote_ip_[16];
    int is_recv_thread_start_;
    int is_vmtl_connected_;
    vmtlConnCb_t cb_;

} Context;

Context cxtHid;
Context cxtVideo;
#define SESSION_ID 0x327

/*ThreadSafeLogCallBack myLog;

void myLog(const char *log_module, int level,const char *tag,int line, const char *fmt, ...)
{
    Log("vmtl", vmtlLevelWarning, "xxx", __LINE__ , fmt_string)
}*/

 vmtlCbRetVal_t onReportData(void *obj, char *data, int len, outDataDesp_t *desp) {
    Context *cxt = (Context *)obj;

     LOGI("onReportData len is %d",len);

    JNIEnv *env;
    javaVM->AttachCurrentThread(&env, NULL);

    jbyteArray jbuff = (env)->NewByteArray(len);

    (env)->SetByteArrayRegion(jbuff,0,len,(jbyte *)data);
    env->CallVoidMethod(javaObj,reportVideoData_callBack, jbuff,len);

    (env)->DeleteLocalRef(jbuff);

    return kRetNormal;
}
 int onSource(void *obj, const char *sendData, int len, int channel, void *sourceContext, outDataDesp_t *desp) {
    Context *cxt = (Context *)obj;

    LOGI("onSource channe is %d", channel);
    LOGI("onSource ctx is %p", cxt);
    LOGI("onSource ctx sock is %d", cxt->sock_[channel]);


    if( send(cxt->sock_[channel], sendData + VMTL_MAX_RELAY_PACKET_LEN, len, 0) <= 0 ) {

    }
    LOGI("onSource after ");

    return 0;
}
 int createSocket(int localPort, int remotePort, const char* localIP, const char* remoteIP) {
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

void _recvThread(Context *cxt, int channel)
{
    outDataDesp_t desp = {0};
    char *sink_buff = NULL;
    int recv_len = 0;

    while(cxt->is_recv_thread_start_)
    {
        LOGI("cxt conn is %p channel is %d",cxt->conn_,channel);
        vmtlNotification_t conn_state = vmtl_get_conn_status(cxt->conn_);
        if (conn_state != kConnected && conn_state != kConnecting) { continue; }

        vmtl_get_sink_buff(cxt->conn_, channel, &sink_buff);
        if (!sink_buff)
        {
            LOGE("failed to get sink buff!");
            continue;
        }
        LOGI("recve before channel is %d ",channel);
        recv_len = recv(cxt->sock_[channel], sink_buff, 1500, 0); //1500 is MAX_VMTL_MTU
        if (recv_len <= 0)
        {
            LOGI("recv error is  ");
            continue;
        }

        LOGI("recve len is %d ", recv_len);
        vmtl_sink_data(cxt->conn_, sink_buff, recv_len, channel, NULL);

        //printf("recv %d bytes\n", recv_len);
    }
}
 void* recvMediaThread(void *obj)
{
    Context *cxt = (Context *)obj;

    _recvThread(cxt, 0);

    return NULL;
}

 void* recvCtrlThread(void *obj)
{
    Context *cxt = (Context *)obj;

    _recvThread(cxt, 1);
    return NULL;
}

 void createRecvThreads(Context *cxt)
{
    pthread_attr_t attr;
    pthread_t media_thread, ctrl_thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    cxt->is_recv_thread_start_ = 1;

    if(pthread_create(&media_thread, NULL, recvMediaThread, (void *)cxt) != 0) {
        LOGE("thread error media");
    }

    if(pthread_create(&ctrl_thread, NULL , recvCtrlThread, (void *)cxt) != 0) {
       LOGE("thread error ctrl");
    }

    pthread_attr_destroy(&attr);
}
 void onVmtlEventNotify(void *obj, vmtlNotification_t event, void *data) {
    Context *cxt = (Context *)obj;
    switch (event) {
        case kConnected:
            cxt->is_vmtl_connected_ = 1;

            break;

        default:
            break;
    }
}

 void echoToServer(Context *cxt, int channel)
{
    if( send(cxt->sock_[channel], "ECHO", 4, 0) <= 0 ) {
        LOGE("send ECHO failed, error: %s",strerror(errno));
    } else {
        LOGI("channel %d ECHO to server", channel);
    }
}
 void byeToServer(Context *cxt, int channel)
{
    if( send(cxt->sock_[channel], "BYE", 4, 0) <= 0 ) {
        LOGE("send BYE failed, error: %s",strerror(errno));
    } else {
        LOGI("channel %d BYE to server", channel);
    }
}
int initVideo(const char *local_ip,const char *remote_ip,int local_port,int remote_port)
{

    cxtVideo.is_recv_thread_start_ = 0;
    cxtVideo.is_vmtl_connected_    = 0;
    cxtVideo.chn_num_ = 2;
    cxtVideo.cb_.notifier = onVmtlEventNotify;
    cxtVideo.cb_.report_data = onReportData;
    cxtVideo.cb_.source = onSource;

   vmtl_create_stream(kVGTP, kStrmDirRecv, "test_rcv_avc_strm", &cxtVideo.stream_id_);

//    vmtl_create_stream(kVGTP, kStrmDirSend, "test_snd_avc_strm", &cxtVideo.hidStream_id);

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
        return -1;
    }
    else
    {
        LOGI("socket create successfully");
    }

    echoToServer(&cxtVideo, 0);
    echoToServer(&cxtVideo, 1);
    createRecvThreads(&cxtVideo);
    LOGI("createRecvThreads");

    vmtl_bind_stream(cxtVideo.conn_, cxtVideo.stream_id_, &cxtVideo.bind_id_);
  //  vmtl_bind_stream(cxtVideo.conn_, cxtVideo.hidStream_id, &cxtVideo.hidBind_id);


    LOGI("vmtl_bind_stream");
    connConfig_t cc_conf = {0};
    cc_conf.session = (void *) SESSION_ID;
    vmtl_config_conn(cxtVideo.conn_, &cc_conf);
    LOGI("vmtl_config_conn");
    vmtl_conn_active(cxtVideo.conn_, kConnAtypeClient);
    LOGI("vmtl_conn_active");

    return 0;

}
int initAudio()
{
     return 0;
}
int initHid(const char *local_ip,const char *remote_ip,int local_port,int remote_port)
{
    cxtHid.is_recv_thread_start_ = 0;
    cxtHid.is_vmtl_connected_    = 0;
    cxtHid.chn_num_ = 2;
    cxtHid.cb_.notifier = onVmtlEventNotify;
    cxtHid.cb_.report_data = onReportData;
    cxtHid.cb_.source = onSource;

    vmtl_create_stream(kVGTP, kStrmDirSend, "test_rcv_avc_strm", &cxtHid.stream_id_);

 //   vmtl_create_conn(&cxtHid, &cxtHid.conn_, cxtHid.cb_, cxtHid.chn_num_, kConnPropBigStream);

    cxtHid.local_port_ = local_port;
    cxtHid.remote_port_ = remote_port;
    strcpy(cxtHid.local_ip_, local_ip);
    strcpy(cxtHid.remote_ip_, remote_ip);
    cxtHid.sock_[0] = createSocket(cxtHid.local_port_, cxtHid.remote_port_, cxtHid.local_ip_, cxtHid.remote_ip_);
    cxtHid.sock_[1] = createSocket(cxtHid.local_port_ + 1, cxtHid.remote_port_ + 1, cxtHid.local_ip_, cxtHid.remote_ip_);

    if(cxtHid.sock_[0]<0||cxtHid.sock_[1]<0)
    {
        LOGE("socket create failed");
        return -1;
    }
    else
    {
        LOGI("socket create successfully");
    }

    echoToServer(&cxtHid, 0);
    echoToServer(&cxtHid, 1);
    createRecvThreads(&cxtHid);


    vmtl_bind_stream(cxtHid.conn_, cxtHid.stream_id_, &cxtHid.bind_id_);
  //  vmtl_bind_stream(cxtHid.conn_, cxtHid.hidStream_id, &cxtHid.hidBind_id);


    connConfig_t cc_conf = {0};
    cc_conf.session = (void *) SESSION_ID;
    vmtl_config_conn(cxtHid.conn_, &cc_conf);

    vmtl_conn_active(cxtHid.conn_, kConnAtypeServer);

}
int stopVideo()
{

     return 0;
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

    initVideo(local_ip,remote_ip,local_port,remote_port);

   // env->ReleaseStringUTFChars(local_ip_, local_ip);
   // env->ReleaseStringUTFChars(remote_ip_, remote_ip);
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
JNIEXPORT void JNICALL Java_com_vanxum_Aic_NetworkJni_stopVmtl
        (JNIEnv *, jobject)
{
    stopVideo();
  //  stopAudio();
  //  stopHid();
}
/*
 * Class:     com_vanxum_Aic_NetworkJni
 * Method:    sendInputEvent
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_vanxum_Aic_NetworkJni_sendInputEvent
  (JNIEnv *env, jobject,jbyteArray event)
{
    jbyte *pBytes = env->GetByteArrayElements(event, 0);
}
