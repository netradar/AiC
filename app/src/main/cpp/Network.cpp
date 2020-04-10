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
#include <sys/syscall.h>

#define  LOG_TAG    "native-dev"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


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
jmethodID   reportAudioData_callBack;
jmethodID   reportExam_callBack;
jmethodID   reportConnected_callBack;


void *recvMediaThread(void *obj);
void *recvCtrlThread(void *obj);

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

Context cxtHid;
Context cxtVideo;
Context cxtAudio;
int decodeType;
#define SESSION_ID 0x327

typedef enum {
    kNoneLevel,
    kErrorLevel,
    kWarningLevel,
    kInformationLevel,
    kDebugLevel
} DebugLog;
static const char level_char[kDebugLevel + 1] = {'N', 'E', 'W', 'I', 'D'};

#define vmtl_log    false

void myLog(const char *log_module, int level,const char *tag,int line, const char *fmt,va_list ap)
{
    if(!vmtl_log)
        return;
    char buf1[4096] = {0};
    char buf[4096] = {0};
    char buf_time[128];
    vsnprintf(buf, 4096, fmt, ap);

    sprintf(buf1, "%u/%s(%4d):%s",   level_char[level], tag, line,
            buf);
    android_LogPriority log_priority = ANDROID_LOG_DEBUG;
    switch (level) {
        case kDebugLevel:
            log_priority = ANDROID_LOG_DEBUG;
            break;
        case kInformationLevel:
            log_priority = ANDROID_LOG_INFO;
            break;
        case kWarningLevel:
            log_priority = ANDROID_LOG_WARN;
            break;
        case kErrorLevel:
            log_priority = ANDROID_LOG_ERROR;
            break;
        default:
            break;
    }
    __android_log_print(log_priority, tag, "%s", buf1);
}
void redirect_log(const char *log_module, int level,const char *tag,int line, const char *fmt, ...)
{

    va_list ap;
    va_start(ap, fmt);
    myLog(log_module, level, tag, line, fmt, ap);
    va_end(ap);

}

 vmtlCbRetVal_t onReportData(void *obj, char *data, int len, outDataDesp_t *desp) {
    Context *cxt = (Context *)obj;

  //   LOGE("onReportData!");

    JNIEnv *env;
    javaVM->AttachCurrentThread(&env, NULL);

    jbyteArray jbuff = (env)->NewByteArray(len);

    (env)->SetByteArrayRegion(jbuff,0,len,(jbyte *)data);

    if(desp->type==kVGTP)
        env->CallVoidMethod(javaObj,reportVideoData_callBack, jbuff,len);
    else if(desp->type==kAudio)
        env->CallVoidMethod(javaObj,reportAudioData_callBack, jbuff,len);
    else if(desp->type==kHid)
        env->CallVoidMethod(javaObj,reportExam_callBack);

    (env)->DeleteLocalRef(jbuff);

    return kRetNormal;
}
 int onSource(void *obj, const char *sendData, int len, int channel, void *sourceContext, outDataDesp_t *desp) {
    Context *cxt = (Context *)obj;



    if( send(cxt->sock_[channel], sendData + VMTL_MAX_RELAY_PACKET_LEN, len, 0) <= 0 ) {
        LOGI("onSource send failed sock is %d", cxt->sock_[channel]);
    }


    return 0;
}
 int createSocket(int localPort, int remotePort, const char* localIP, const char* remoteIP) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {

        return -1;
    }

    int size = 8*1024*1024;
    int buf_len = -1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR , (const void *)&size, (socklen_t) sizeof(size));
    socklen_t len = sizeof(buf_len);
    getsockopt(sock, SOL_SOCKET, SO_REUSEADDR , (void *) &buf_len, &len);


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

        vmtlNotification_t conn_state = vmtl_get_conn_status(cxt->conn_);
        if (conn_state != kConnected && conn_state != kConnecting) { continue; }

    //    if(cxt->conn_!=NULL)
        vmtl_get_sink_buff(cxt->conn_, channel, &sink_buff);
        if (!sink_buff)
        {
            LOGE("failed to get sink buff!");
            continue;
        }

        recv_len = recv(cxt->sock_[channel], sink_buff, 1500, 0); //1500 is MAX_VMTL_MTU
        if (recv_len <= 0)
        {
            LOGI("recv error is  ");
            continue;
        }

        if(cxt->is_recv_thread_start_)
        vmtl_sink_data(cxt->conn_, sink_buff, recv_len, channel, NULL);

        //printf("recv %d bytes\n", recv_len);
    }

    LOGI("recv thread exited");
}
 void* recvMediaThread(void *obj)
{
    Context *cxt = (Context *)obj;

    LOGI("recv thread2 %d",syscall(SYS_gettid));
    _recvThread(cxt, 0);

    return NULL;
}

 void* recvCtrlThread(void *obj)
{
    Context *cxt = (Context *)obj;
    LOGI("recv thread %d",syscall(SYS_gettid));
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

    if(cxt->stream_id_!=cxtHid.stream_id_)
        return;
    LOGI("vmtl event");
    switch (event) {
        case kConnected:
            cxt->is_vmtl_connected_ = 1;
            JNIEnv *env;
            javaVM->AttachCurrentThread(&env, NULL);
            env->CallVoidMethod(javaObj,reportConnected_callBack);
            LOGI("vmtl event2");

            vmtl_set_loglevel("info");
            vmtl_log_redirect(redirect_log);

            break;

        default:
            break;
    }
}

 void echoToServer(Context *cxt, int channel)
{
    if(cxt->stream_id_==cxtVideo.stream_id_)
        LOGI("video echo11111111111111");
    if(cxt->stream_id_==cxtAudio.stream_id_)
        LOGI("audio echo12222222222222");
    if(cxt->stream_id_==cxtHid.stream_id_)
        LOGI("hid echo333333333333333333333");


    if(decodeType==0)
    {
        if( send(cxt->sock_[channel], "ECHO+H264", 9, 0) <= 0 ) {
            LOGE("send ECHO failed, error: %s",strerror(errno));
        } else {
            LOGI("channel %d ECHO to server", channel);
        }
    }
    else
        {
        if (send(cxt->sock_[channel], "ECHO+H265", 9, 0) <= 0) {
            LOGE("send ECHO failed, error: %s", strerror(errno));
        } else {
            LOGI("channel %d ECHO to server", channel);
        }
    }
}
 void byeToServer(Context *cxt, int channel)
{
    if( send(cxt->sock_[channel], "BYE", 3, 0) <= 0 ) {
        LOGE("send BYE failed, error: %s",strerror(errno));
    } else {
        LOGI("channel %d BYE to server successfully", channel);
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

    vmtl_create_conn(&cxtVideo, &cxtVideo.conn_, cxtVideo.cb_, cxtVideo.chn_num_, kConnPropBigStream);

    cxtVideo.local_port_ = local_port;
    cxtVideo.remote_port_ = remote_port;
    strcpy(cxtVideo.local_ip_, local_ip);
    strcpy(cxtVideo.remote_ip_, remote_ip);



    cxtVideo.sock_[0] = createSocket(cxtVideo.local_port_, cxtVideo.remote_port_, cxtVideo.local_ip_, cxtVideo.remote_ip_);
    cxtVideo.sock_[1] = createSocket(cxtVideo.local_port_ + 1, cxtVideo.remote_port_ + 1, cxtVideo.local_ip_, cxtVideo.remote_ip_);

    if(cxtVideo.sock_[0]<0||cxtVideo.sock_[1]<0)
    {
       // LOGE("socket create failed`111111");
        LOGE("socket create failed`111111, error: %s",strerror(errno));
        return -1;
    }
    else
    {
        LOGI("socket create successfully11111");
    }

   /* byeToServer(&cxtVideo,0);
    byeToServer(&cxtVideo,1);*/
    echoToServer(&cxtVideo, 0);
    echoToServer(&cxtVideo, 1);
    echoToServer(&cxtVideo, 0);
    echoToServer(&cxtVideo, 1);
    createRecvThreads(&cxtVideo);
    LOGI("createRecvThreads");

    vmtl_bind_stream(cxtVideo.conn_, cxtVideo.stream_id_, &cxtVideo.bind_id_);

    LOGI("vmtl_bind_stream");
    connConfig_t cc_conf = {0};
    cc_conf.session = (void *) SESSION_ID;
    vmtl_config_conn(cxtVideo.conn_, &cc_conf);
    LOGI("vmtl_config_conn");
    vmtl_conn_active(cxtVideo.conn_, kConnAtypeClient);
    LOGI("vmtl_conn_active");

    return 0;

}
int initAudio(const char *local_ip,const char *remote_ip,int local_port,int remote_port)
{
    cxtAudio.is_recv_thread_start_ = 0;
    cxtAudio.is_vmtl_connected_    = 0;
    cxtAudio.chn_num_ = 2;
    cxtAudio.cb_.notifier = onVmtlEventNotify;
    cxtAudio.cb_.report_data = onReportData;
    cxtAudio.cb_.source = onSource;

    vmtl_create_stream(kAudio, kStrmDirRecv, "audio_strm", &cxtAudio.stream_id_);

    vmtl_create_conn(&cxtAudio, &cxtAudio.conn_, cxtAudio.cb_, cxtAudio.chn_num_, kConnPropBigStream);

    cxtAudio.local_port_ = local_port;
    cxtAudio.remote_port_ = remote_port;
    strcpy(cxtAudio.local_ip_, local_ip);
    strcpy(cxtAudio.remote_ip_, remote_ip);
    cxtAudio.sock_[0] = createSocket(cxtAudio.local_port_, cxtAudio.remote_port_, cxtAudio.local_ip_, cxtAudio.remote_ip_);
    cxtAudio.sock_[1] = createSocket(cxtAudio.local_port_ + 1, cxtAudio.remote_port_ + 1, cxtAudio.local_ip_, cxtAudio.remote_ip_);

    if(cxtAudio.sock_[0]<0||cxtAudio.sock_[1]<0)
    {
        LOGE("socket create failed22222222");
        return -1;
    }
    else
    {
        LOGI("socket create successfully22222");
    }

  /*  byeToServer(&cxtAudio,0);
    byeToServer(&cxtAudio,1);*/
    echoToServer(&cxtAudio, 0);
    echoToServer(&cxtAudio, 1);
    echoToServer(&cxtAudio, 0);
    echoToServer(&cxtAudio, 1);
    createRecvThreads(&cxtAudio);


    vmtl_bind_stream(cxtAudio.conn_, cxtAudio.stream_id_, &cxtAudio.bind_id_);


    connConfig_t cc_conf = {0};
    cc_conf.session = (void *) SESSION_ID;
    vmtl_config_conn(cxtAudio.conn_, &cc_conf);
    vmtl_conn_active(cxtAudio.conn_, kConnAtypeClient);
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

    vmtl_create_stream(kHid, kStrmDirTwoWay, "hid_strm", &cxtHid.stream_id_);

    vmtl_create_conn(&cxtHid, &cxtHid.conn_, cxtHid.cb_, cxtHid.chn_num_, kConnPropBigStream);

    cxtHid.local_port_ = local_port;
    cxtHid.remote_port_ = remote_port;
    strcpy(cxtHid.local_ip_, local_ip);
    strcpy(cxtHid.remote_ip_, remote_ip);
    cxtHid.sock_[0] = createSocket(cxtHid.local_port_, cxtHid.remote_port_, cxtHid.local_ip_, cxtHid.remote_ip_);
    cxtHid.sock_[1] = createSocket(cxtHid.local_port_ + 1, cxtHid.remote_port_ + 1, cxtHid.local_ip_, cxtHid.remote_ip_);

    if(cxtHid.sock_[0]<0||cxtHid.sock_[1]<0)
    {
        LOGE("socket create failed3333333333");
        return -1;
    }
    else
    {
        LOGI("socket create successfully333333333");
    }
   /* byeToServer(&cxtHid,0);
    byeToServer(&cxtHid,1);*/
    echoToServer(&cxtHid, 0);
    echoToServer(&cxtHid, 1);
    echoToServer(&cxtHid, 0);
    echoToServer(&cxtHid, 1);
    createRecvThreads(&cxtHid);


    vmtl_bind_stream(cxtHid.conn_, cxtHid.stream_id_, &cxtHid.bind_id_);

    LOGI("hid bind id is %d",cxtHid.bind_id_);
    connConfig_t cc_conf = {0};
    cc_conf.session = (void *) SESSION_ID;
    vmtl_config_conn(cxtHid.conn_, &cc_conf);
    vmtl_conn_active(cxtHid.conn_, kConnAtypeServer);
    return 0;
}
int stopVideo()
{

    byeToServer(&cxtVideo,0);
    byeToServer(&cxtVideo,1);
    cxtVideo.is_recv_thread_start_ = 0;
    cxtVideo.is_vmtl_connected_    = 0;
    vmtl_conn_deactive(cxtVideo.conn_);
    vmtl_release_conn(cxtVideo.conn_);
  //  cxtVideo.conn_=NULL;
    vmtl_destroy_stream(cxtVideo.stream_id_);

    if(shutdown(cxtVideo.sock_[0],2)<0)
        LOGI("socket shutdonw error %s ",strerror(errno));
    else
        LOGI("socket shutdonw ok");
    shutdown(cxtVideo.sock_[1],2);
     return 0;
}
int stopAudio()
{
    byeToServer(&cxtAudio,0);
    byeToServer(&cxtAudio,1);
    cxtAudio.is_recv_thread_start_ = 0;
    cxtAudio.is_vmtl_connected_    = 0;
    vmtl_conn_deactive(cxtAudio.conn_);
    vmtl_release_conn(cxtAudio.conn_);
 //   cxtAudio.conn_=NULL;
    vmtl_destroy_stream(cxtAudio.stream_id_);
    shutdown(cxtAudio.sock_[0],2);
    shutdown(cxtAudio.sock_[1],2);
    return 0;
}
int stopHid()
{
    byeToServer(&cxtHid,0);
    byeToServer(&cxtHid,1);
    cxtHid.is_recv_thread_start_ = 0;
    cxtHid.is_vmtl_connected_    = 0;
    vmtl_conn_deactive(cxtHid.conn_);
    vmtl_release_conn(cxtHid.conn_);
 //   cxtHid.conn_=NULL;
    vmtl_destroy_stream(cxtHid.stream_id_);

    shutdown(cxtHid.sock_[0],2);
    shutdown(cxtHid.sock_[1],2);
    return 0;
}
JNIEXPORT jint JNICALL Java_com_vanxum_Aic_NetworkJni_vmtlInit
  (JNIEnv * env, jobject obj, jstring local_ip_,jstring remote_ip_, jint local_port,jint remote_port,jint decodeType_)
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
    reportAudioData_callBack   = env->GetMethodID(cls, "reportAudioData", "([BI)V");
    reportExam_callBack   = env->GetMethodID(cls, "reportExam", "()V");
    reportConnected_callBack   = env->GetMethodID(cls, "reportConnected", "()V");



    decodeType = decodeType_;


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
    initAudio(local_ip,remote_ip,local_port+2,remote_port+2);
    initHid(local_ip,remote_ip,local_port+4,remote_port+4);


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
JNIEXPORT void JNICALL Java_com_vanxum_Aic_NetworkJni_stopVmtl
        (JNIEnv *, jobject)
{
    LOGI("stopVmtl");
    stopVideo();
    stopAudio();
    stopHid();

    vmtl_uninit();
}
/*
 * Class:     com_vanxum_Aic_NetworkJni
 * Method:    sendInputEvent
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_vanxum_Aic_NetworkJni_sendInputEvent
  (JNIEnv *env, jobject,jbyteArray event,int len)
{
    jbyte *pBytes = env->GetByteArrayElements(event, 0);

    vmtlDataDesp_t desp = {0};
    desp.type = kHid;
    desp.sub_type = HID_DATA_TYPE_MOUSE;


    if(!cxtHid.is_vmtl_connected_)
        return;

    vmtl_send_data(cxtHid.conn_, cxtHid.bind_id_, (char *)pBytes, len, &desp);

}

extern "C" JNIEXPORT void JNICALL Java_com_vanxum_Aic_NetworkJni_sendExamPacket
(JNIEnv *, jobject)
{
    vmtlDataDesp_t desp = {0};
    desp.type = kHid;
    desp.sub_type = HID_DATA_TYPE_KEYB;

    if(!cxtHid.is_vmtl_connected_)
        return;
LOGI("send exam");
    vmtl_send_data(cxtHid.conn_, cxtHid.bind_id_, (char *)"EXAM", 4, &desp);
}
