/**
  * @file vmtl.h
  * @author Initial	Lian <lianronggang@vanxum.com>		01/12/2017
  *         Updated	xxx<xxx@vanxum.com>					dd/mm/yyyy
  * @brief declaration of public APIs and data structures
  *
  * This file provides external APIs of vmtl, including services
  * requests and status enquiries, the relative data structures
  * are presented as well. 
  * 
  * Copyright(c) 2014-present-future Vanxum Inc.. All rights reserved.
  * 
  */

#ifdef PRAGMA_ONCE
#pragma once
#endif

#ifndef VMTL_H
#define VMTL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#if defined(BUILDING_VMTL_DLL) //compiling (providing) dll
#define VMTL_IMPORT __declspec(dllexport)
#elif defined(USING_VMTL_DLL) //linking (using) dll
#define VMTL_IMPORT __declspec(dllimport)
#else
#define VMTL_IMPORT
#endif
#else
#define VMTL_IMPORT __attribute__ ((visibility("default")))
#endif

#ifdef _WIN32
#else //unix-like OS
#include <sys/socket.h>
#endif

#include "portable.h"
#include "adaptor.h"
#include "vgtp_adpt.h"
#include "avcplus_adpt.h"
#include "audio_adpt.h"
#include "hid_adpt.h"


#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN  ///< Intel/AMD x86, x64, Digital VAX, Digital Alpha and ARMEL
//#undef LITTLE_ENDIAN  ///< if Motorola 680x0, Sun SPARC, PowerPC, ARMEB and most RISC machines, uncomment this
#endif

#define LEGACY_VGR  ///< for being compatible with x86 VGR's interface functions
#define SHORT_LIVED_TCP  ///< some leisure service streams use tcp instead of unreliable udp
#undef SHORT_LIVED_TCP  // if need tcp mentioned above, comment this line
#define MAX_ALGTHM_OPTIONS 5 ///< how mang kinds of algorithms can be apply to one connection
#define MAX_VERSION_STRING_LENGTH 44 ///< maximum length of version string
#define MAX_STRM_TYPE_NAME_LEN       10
#define VMTL_PROTOCOL_TYPE 128  ///< 1000,0000  different with ice 0000,0000
#define VMTL_OUTPUT_DATATYPE_RAW    0  //passthrough raw (non-vmtl transporting protocol wrapped)
#define VMTL_OUTPUT_DATATYPE_MEDIA  2

#define MAX_BOUND_STRM_NUM_PER_CONN     10
#define VMTL_MAX_RELAY_PACKET_LEN 20U    ///< maximum reserved bytes for packing

#define FREE_PORT_AUTO_DETECT
#define MULTI_CHNS_SUPPORTED


typedef enum _vmtlStatus_t {
    kInitializing = 0,
    kInitialized,
    kStateRunning,
    kStateHalt,
    kInitFail,
    kStateException
} vmtlStatus_t;

typedef enum _vmtlNotification_t {
    kConnectInit, //the connection was just created but not conduct the activing op
    kConnecting,
    kConnected,
    kDisconnected, //set this once release connection
    kConnReleasing, //during releasing
    kConnFailed, //fail to make connection work, ask for destroying a connection
    kVersionMismatch,
    kBrNarrow,  //cut down current input bitrate of main stream in given connection
    kBrBroaden,  //widen current input bitrate of main stream in given connection
    kLagWarning, //delay too much
    kLagWarnClr, //clear
    //network link is too poor(too narrow bw, high retry ratio, high lag, too many tamper, etc.)
    //to hold the main stream transporting
    kPoorLinkWarn,
    kPoorLinkWarnClr, //clear
    kSwitchSignFrame, //tell media source to switch significant(key) frame
    kChannelDown, //(*vmtl_notifier)'s data parameter will provide more info
    kChannelUp,
    kPassTiemCost, //report time cost of delivering data by network (S<=>R)
    kMaxNotification
} vmtlNotification_t;

///@note each stream contains explicit dctc, ice and auxiliary sub-streams
///(eg: ddc in visual-media streams), those do not count in
typedef enum _streamMode_t {
    kUni = 1,      //like audio, ipc-es, vgtp1.x
    kComposite2s,    //contains two sub-streams
    kComposite3s,    //like vgtp2.x, contains three sub-streams: primary, carried video, feedback
    kMultiplex,
    kModeMax
} streamMode_t;

///an entity (eg: codec) can create kStrmIdIdxMax stream for itself
typedef enum _streamIdIndex_t {
    kStrmIdIdxMain,     ///< main data of entity
    kStrmIdIdxService,  ///< serive data for messages, events and etc.
    kStrmIdIdxSync,     ///< sync event for main media data
    kStrmIdIdxMax
} streamIdIndex_t;

/**
  * Pre-defined stream types:
  * kDCTC: dedicated channel for transport controlling only for internal
  * kVGTP: media stream for vGTP
  * kAVC: media stream for H.264/265 such as video player redirection or IP camera
  * kAudio: aux-media stream, generally attached to some media stream for bare audio
  * kVoice: real time aux-media stream, generally attached to some media stream for human's voice or speech, such as VoIP, MCU
  * kRvsInput: real time hid stream, use its FEC algorithm to anti-lost
  * kDDC: bidirectional service for common use such as display parameters negotiation
  * kUSBAuth: bidirectional real time service for AAA info
  * kEMRP: bidirectional service for multi-cast controlling
  * kSync: bidirectional real time service-rt for same-source synchronization
  * kRaw: non-vmtl-transporting-head wrapped for pass-through raw
  * kSnapShot: fat service for snapshot or thumbnail
  * kFileTrans: file transport with reliability = 3, i.e., using TCP
  * kMediaFwd: forwarding media stream as unit of packet, only meaning in forward mode
  */
typedef enum _streamType_t {
    kDCTC = 0,
    kVGTP = 1,
    kAVC,
    kAudio,
    kVoice,
    kHid,
    kDDC,
    kUSBAuth,
    kEMRP,
    kSync,
    kRaw,
    kSnapShot,
    kFileTrans,
    kMediaFwd,
    kStreamTypeMax
} streamType_t;

//stream's transporting type
typedef enum _streamTransType_t {
    kMediaRT = 0,    //main big media realtiem-media
    kMediaNRT,        //main big media media
    kAtchMediaRT,   //attached non-big realtiem-media
    kAtchMediaNRT,    //attached non-big media
    kCtrlRT,        //realtime controlling signal
    kCtrlNRT,        //controlling signal
    kServiceRT,        //realtime service message
    kServiceNRT,    //service message
    kStrmTransTypeMax
} streamTransType_t;

typedef enum _connAgentType_t {
    kConnAtypePending = 0, //we don't know agent type of a connection before upper layer negotiation completed
    kConnAtypeServer = 1, //refer to the sender of main stream data despite other inverse-directional signals
    kConnAtypeClient  // //refer to the receiver of main stream data despite other inverse-directional signals
} connAgentType_t;

typedef enum _agentType_t {
    kAtypePending = 0,  //we do not know or have no idea of vmtl's agent type, or bi-directional scenarios
    kAtypeServer = 1,   //refer to the sender of main stream data despite other inverse-directional signals
    kAtypeClient,        //refer to the receiver of main stream data despite other inverse-directional signals
    kAtypeForward
} agentType_t;

typedef enum _connProperty_t {
    kConnPropDefault = 0,  //connection for small data streams, ex: ctrl, input, audio
    kConnPropBigStream = 1   //connection for big data streams (exclude or include), ex: video, HD audio, file
} connProp_t;

/* DO NOT CHANGE THE ORDER OF FIELDS! */
typedef struct _vmtlMode_t {
    agentType_t atype;  //not on-the-fly
    uint32_t overall_bw; //the overall uplink bandwidth assigned to vmtl, unit: KB/s, the default is 10240(80Mbps)
    //int port_range_from; //not on-the-fly: local port numbers range from, close interval, [0,0] means no limitation
    //int port_range_to; //not on-the-fly: local port numbers range to, close interval, [0,0] means no limitation
    uint8_t ice_enable:1; //whether enable NAT holing -- 0:off, 1:on
    uint8_t relay:1; //whether enable relay -- 0:off, 1:on
    uint8_t trace:1; //do trace voluntarily if set, or do trace only if commands given
} vmtlMode_t;

typedef enum _streamDirection_t {
    kStrmDirSend,
    kStrmDirRecv,
    kStrmDirTwoWay
} strmDir_t;

typedef enum _transportDirection_t {
    kTransModeOneWay,
    kTransModeRoundTrip
} transMode_t;

typedef enum _carriedDataType_t {
    kCarryDataTypeDefault,      ///< control signal, general service like ddc, snapshot, ...
    kCarryDataTypeAuxMedia,     ///< attached media like (HD) audio, speech, ...
    kCarryDataTypeMedia         ///< visual media like remote desktop, ...
} carryData_t;

typedef enum _transportPriority_t {
    kTransPriUrgent,    ///< ctrl, hid, speech, fb, ...
    kTransPriLive,      ///< aka: realtime, remote desktop display, attached audio, ...
    kTransPriStream,    ///< local video redirection, service, ...
    kTransPriFile,      ///< file + concurrent
    kTransPriMax
} transPri_t;

//opaque -- 0:可被vmtl解析的明文数据，外部需提供数据关联策略 1:隐晦数据，vmtl透传，只保证传输要求, 默认0
typedef struct _streamPorperty_t {
    char name[MAX_STRM_TYPE_NAME_LEN];   ///< for the customized streams
    transMode_t trans_mode;     ///< one way or round trip
    carryData_t data_type;      ///< type of carried data by this stream
    int common_pkt_size;        ///< common pkt size delivered to vmtl one time
    int buffer_size;            ///< how many data buffered
    transPri_t priority;        ///< transporting priority
    uint8_t reliability:3;      ///< if guaranteed delivery -- 0:udp, 1:udp-r1(report, generally for media), 2:udp-r2(ARQ), 3:tcp
    uint8_t feedback:1;         ///< does need feedback or control sub-stream? (only for one way) This sub strm is URGENT+OPAQUE implicitly
    uint8_t opaque:1;           ///< 0:clear text data(can be parsed by vmtl via external descriptions), 1:opaque data(pass through by vmtl)
    uint8_t crypt:1;            ///< if do encryption, the encryption type is designated by crypt_cb()
    uint8_t fec:1;              ///< forward error correction or data cover -- 0:no, 1:yes
    uint8_t sa:1;               ///< streams aggregation only makes sense in forward mode
    adaptor_t policy_map_set;   ///< parse_cb(), nred_cb(), red_cb(), crypt_cb(), ...
} streamProp_t;

typedef struct _vmtlGlance_t {
    /// @todo {definition of vmtlGlance_t, try to use c++ stl list}
    /// @todo { convert to human-read item format }
    uint64_t standing_time; //running time
    uint64_t tx_rate; //current overall upload throughput of vmtl, unit: byte/s
    uint64_t rx_rate; //current overall download throughput of vmtl, unit: byte/s
    uint16_t tot_streams; //total streams created
    uint16_t tot_sessions; //total logic service sessions created
    uint16_t tot_conns;  //total connections created
} vmtlGlance_t;

/** subordinate priority in a data class, 
  * default 0 means fare among connections in same class
  */
typedef enum _subPriority_t {
    kPriLow = 1, //like negotiating service or unconcerned media
    kPriNormal, //like general media or ctrl
    kPriHigh //like concerning media or ctrl, such as input
} subPri_t;

/**
 * session: sometimes multiple connections are involved in one service session (as users' POV),
 * the upper layer should denote which connections belong to a same logic session
 * so that vmtl can conduct tht macro-throttling and network measure.
 * The upper applications can provide id numbers as well as pointer addresses or any
 * unique represented numerals
 */
typedef struct _connectionConfig_t {
	void *session; //session should be inactive, 0 is reserved
    uint32_t bw_thhd;  //on-the-fly: user-customized per session, <= overall_bw, unit: KBps
    uint16_t mtu; //session should be inactive: mtu per session = payload + tuh + pack + udp head + ip head
    uint8_t is_wan; //session should be inactive: LAN or WAN per session
} connConfig_t;

typedef struct _algorithmInfo_t {
    const char algthm_name[16];
} algthmInfo_t;

///vmtl relay forwarding structure
typedef struct _relayForward_t {
    char *buf;       ///< 数据包缓存区基地址
    char *pack_ptr;  ///< real payload 游标地址
    int buf_size;       ///< 缓存区总长度
    int pack_size;      ///< 真实报文长度
    int chn_type;       ///< 通道id
    char tunnel;        ///< 是否pack(中继)
} relayForward_t;

typedef enum TypeDesp {
    //common
    TypeDesp_None = 0,
    TypeDesp_Cid_Sync,
    TypeDesp_Heart_Beat,
    TypeDesp_DDC,

    //forward模式下的缓存数据,from begin to end
    TypeDesp_Data_Begin = 8,
    //Image encoding type, vgtp
    TypeDesp_I_REF,
    TypeDesp_P_REF,
    TypeDesp_P_NORM,
    TypeDesp_AVC,
    //Audio
    TypeDesp_Audio = 16,
    TypeDesp_Data_End = 31,

    //vgtp
    TypeDesp_FDBK = 32,
    TypeDesp_SPS,
    TypeDesp_SPS_ACK,
    //RvsInput
    TypeDesp_RvsInput,

} TypeDesp;

typedef struct _outDataDescription_t {
    uint16_t type;      //stream type
    uint8_t sub_type;   //stream sub type
    uint16_t sn;        //sequence number
} outDataDesp_t;

typedef enum _vmtlCallbackReturnValue_t {
    kRetNormal = 0,
    kRetCommonError = 1,
    kRetRecvBuffFull,
    kRetDecodeFailed,
    kRetMax
} vmtlCbRetVal_t;

#define IS_VMTL_WRAPPED(packet_)  (2 == ((unsigned char) packet_[0] >> 6) || 3 == ((unsigned char) packet_[0] >> 6) || 0x7e == (unsigned char) packet_[0])

///外部或中继打包回调函数指针类型定义
typedef int (*Relay_Packet)(void *obj, relayForward_t *param);

///外部或中继拆包回调函数指针类型定义
typedef int (*Relay_Unpacket)(void *obj, relayForward_t *param);

/** 
  * @brief callback to notify some vmtl events(with or w/o carried data)
  * to upper application layer
  * 
  * @param obj owner obj, indicate whom the notification to,
  * in VIPA case, obj is the Transport adaptor of NetworkStream
  * @param event an vmtl event to report, eg: connecting, connected
  * @param data the helper data, eg: connection obj, logic connection obj
  */
typedef void (*vmtl_notify)(void *obj, vmtlNotification_t event, void *data);

/** 
  * @brief callback to report received data to upper application layer
  * 
  * @param obj owner obj, indicate whom the data is reported to, eg: Decoder
  * @param data point to received data
  * @param len data length in byte
  * @param data_type 0-ice, 1-ddc, 2-media  , convert adapt types to here 3 types
  * @param param  ip and port for ice
  * @todo { the variable list would not include send_no }
  */
typedef vmtlCbRetVal_t (*vmtl_report_data)(void *obj, char *data, int len, outDataDesp_t *desp);

/** 
  * @brief callback to reply vmtl cmds in a third part tool
  * 
  * @param fmt :the format string represents the reply content of a vmtl command
  */
typedef void (*vmtl_cmd_reply)(const char* fmt, ...);

typedef void (*ThreadSafeLogCallBack)(const char *log_module, int level,const char *tag,int line, const char *fmt, ...);

/** 
  * @brief callback the data that vmtl processed
  * 
  * @param handle : connContainer_t*
  * @param sendData : data
  * @param len : data length
  * @param channel : ch0:media ch1:ctrl
  * @param sourceContext: sourceContext
  *
  */
typedef int (*vmtl_source)(void *obj, const char *data, int len, int chn_id, void *context, outDataDesp_t *desp);


typedef struct _vmtlConnectionCallback_t {
    vmtl_notify notifier;
    vmtl_report_data report_data;
    vmtl_source source;
    void *context;
} vmtlConnCb_t;

/** 
  * @brief initializes a vmtl entity, invoked by apps
  * 
  * vmtl_init() initializes the context of vmtl, vmtl's
  * state should be kInitialized after this, this function
  * can be invoked repeatedly by different applications.
  * 
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_init(void);
/** 
  * @brief configs the vmtl entity, invoked after vmtl_init()
  * 
  * vmtl_mode_set() is able to run partly on the fly,
  * in addition, if user want to update a subset of
  * configuration arbitrarily, vmtl_mode_get() shall be
  * invoked firstly to avoid the unchanged item(s) overwritten
  *
  * @deprecated { Note: once vmtl_mode_set() called, customized configuration
  * will take over vmtl if there is any collision. }
  *
  * @pre { vmtl_init() }
  * 
  * @param mode provides vmtl's configuration set
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_mode_get(vmtlMode_t *mode);
VMTL_IMPORT int vmtl_mode_set(vmtlMode_t *mode);

/** 
  * @brief acquire current state of vmtl entity
  * 
  * vmtl_get_status() is able to run on the fly, in addition, user
  * can arbitrarily update a subset of configuration by just
  * leaving the unchanged item(s) zero.
  * Note: once vmtl_modeset() called, customized configuration
  * will take over vmtl if there is any collision.
  * 
  * @return vmtlStatus_t enum type indicates the current state of vmtl
  */
VMTL_IMPORT vmtlStatus_t vmtl_get_status(void);

/** 
  * @brief Get current glance of vmtl in runtime
  * 
  * vmtl_get_glance() retreive the vmtlGlance srtuct from the Trace module for user-apps
  * including but not limited to: all streams registered / all connections each stream /
  * all queues each connection / statistics data
  * 
  * @retval vmtl_glance bw/RTT/lost rate/statistics/...
  *
  * @note it is NTS so that not call this too much frequently
  */
VMTL_IMPORT int vmtl_get_glance(vmtlGlance_t *vmtl_glance);

/** 
  * @brief register a new stream type to vmtl besides the pre-defined stream types
  * 
  * vmtl_register_stream_type(): register a user-defined stream type
  * Note: the pre-defined types of stream does not need to register
  * 
  * @param sp point to property description of the registering stream type
  * @retval type_id the id of registered stream type
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_register_stream_type(streamProp_t *sp, uint16_t *type_id);

/** 
  * @brief unregister the given stream type
  * 
  * @param type_id is the stream type id to be unregistered
  * @return 0:success, -1:failed
  *
  * @pre { all streams refer to this type have been destroyed }
  */
VMTL_IMPORT int vmtl_unregister_stream_type(uint16_t type_id);

/** 
  * @brief create a stream instance per given type
  * 
  * vmtl_create_stream(): the created stream is independent of connections
  * before bind it to any connection
  * 
  * @param type_id - indicate a registered stream type
  * @param dir - direction of stream only for one way, leave it if two way
  * @param alias - alias name (more than 20 bytes would be trimmed) of this stream if non-null (optional)
  * @retval stream_id - unique id of stream instance
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_create_stream(uint16_t type_id, strmDir_t dir, const char *alias, uint16_t *stream_id);
VMTL_IMPORT int vmtl_destroy_stream(uint16_t id);

/** 
  * @brief bind a stream instance to the specified connection
  * 
  * vmtl_bind_stream(): a stream can be bound to a connection
  * only if the connection is in state of kConnectInit or kConnected
  * 
  * @param connc - point to a connection obj
  * @param stream_id - unique id of stream instance
  * @retval bound_id - unique id of bound index
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_bind_stream(void *connc, uint16_t stream_id, int *bind_id);

/** 
  * @brief unbind a stream instance from the specified connection
  * 
  * vmtl_unbind_stream(): a stream can be unbound from a connection
  * only if the connection is in state of kConnectInit or kConnected
  * 
  * @param connc - point to a connection obj
  * @param stream_id - unique id of stream instance
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_unbind_stream(void *connc, uint16_t stream_id);

/* a new buffer management module that enables all UDP sockets in one process can share protocol buffer.
The goodness it brings is the much less memory usage for multiple UDP connections compared to previous versions.
automatically resize its buffer in order to reduce memory usage while providing maximum throughput. */

/** 
  * @brief create a connection(Inc. channels) with sync style
  *
  * First of all, a local(eg: sender in general) receives a session call
  * from a peer(eg: receiver in general), then the local creates a counterpart
  * connection as well as the same thing the peer did before.
  * a connection instance is created and initialized in this phase,
  * the upper application will activate this connection in next phase
  * by calling vmtl_conn_active()
  * vmtl_create_conn() returns the connection obj on site right away
  *
  *	@param ip - given local IP addr for service, also can be a domain name
  *	@param cb includes function points to notify events or report data to caller, must-be valid
  *	@param owner_obj represents the caller
  *	@param chn_num defines how many channels contained in this connection
  *	@param prop indicates if this connection is used for some big media streams
  *	@retval conn - point to pointer of the connection obj created
  *	@retval port - point to the start number of a series of local port numbers for service
  *	@return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_create_conn(void *owner_obj, void **conn, vmtlConnCb_t cb, int chn_num, connProp_t prop);

/** 
  * @brief get current status of given conn
  * 
  * vmtl_get_conn_status() returns current status of given conn,
  * kConnecting means vmtl is trying to get the given connection connected
  * kConnected means vmtl has made it
  * kDisconnected means the given connection has been shut by vmtl
  *
  * @param conn point to a connection obj
  *	@return kConnecting or kConnected or kDisconnected
  */
VMTL_IMPORT vmtlNotification_t vmtl_get_conn_status(void *conn);

/** 
  * @brief config a connection
  * 
  * vmtl_get_conn_config() get specified connection obj's configuration
  *
  * @param connc connection obj
  *	@retval config point to the configuration of connc
  *	@return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_get_conn_config(void *connc, connConfig_t *conf);

/** 
  * @brief config a connection
  * 
  * vmtl_config_conn() config a Connection obj and the
  * relevant socket(s) options on the fly
  *
  * @param connc connection obj
  *	@param config the configuration of conn
  *	@return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_config_conn(void *connc, connConfig_t *config);

/** 
  * @brief send out data
  * 
  * vmtl_send_data() is ......
  *
  * @param connc reprensents a connection obj created for this stream
  * @param bound_id the stream of given data belongs is bound to this connection obj
  *	@param data point to buffer in which data would be sent
  * @param length data size
  * @param data_desp description of data, null is allowed
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_send_data(void *connc, int bound_id, const char *data, uint32_t length, void *data_desp);

/** 
  * @brief send out data
  * 
  * vmtl_send_data2() is ......
  *
  * @param connc reprensents a connection obj created for this stream
  * @param stream_id : the stream given data belongs to
  *	@param data point to buffer in which data would be sent
  * @param length data size
  * @param data_desp description of data, null is allowed
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_send_data2(void *connc, uint16_t stream_id, const char *data, uint32_t length, void *data_desp);

/**
 * ICE穿透时所用特殊发送接口
 * @param connc connecion obj
 * @param chn_type 当前通道类别
 * @param szFromIP 源地址
 * @param nFromPort 源端口
 * @param szRemoteIP 目标地址
 * @param nRemotePort 目标端口
 * @param data 数据
 * @param length 长度
 * @return 0:成功, -1:失败
 */
VMTL_IMPORT int vmtl_send_special_data(void *connc, int chn_type, const char *data, uint32_t length, char tunnel);

/** 
  * @brief receive bunch of data
  * 
  * vmtl_recv_data() is ......
  *
  * @param connc a Connection obj created for this media/signalling stream
  *	@param ***data point to buffer in which received data is hold
  * @param **length indicates the length of each pkt
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_recv_data(void *connc, char ***data, int **length);

/** 
  * @brief get a buff to sink
  * 
  * vmtl_get_sink_buff() get a vmtl's buff for external sink func
  * to avoid extra memory copy when recv()
  *
  * @param connc represents the target connection obj
  * @param chn_id target channel in connc
  * @param sink_buff point to pointer of sink buff
  *	@return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_get_sink_buff(void *connc, int chn_id, char **sink_buff);

/** 
  * @brief sink data to vmtl
  * 
  * @param connc : connContainer_t*
  * @param sendData : data
  * @param len : data length
  * @param channel : ch0:media ch1:ctrl
  * @param sinkContext
  * @return 0 if success
  */
VMTL_IMPORT int vmtl_sink_data(void *connc, const char *data, int len, int chn_id, void *context);

/** 
  * @brief bring a connection to active (aka: connected) state
  * 
  * vmtl_conn_active() is the substitute for activeOperation()
  *
  * @note if legal conn_agent_type had been set in
  * vmtl_create_conn() sometimes before, this will be ignored
  *
  * @param connc represents the target connection obj
  * @param conn_agent_type represents the main (media) stream's direction of this connection
  *	@return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_conn_active(void *connc, connAgentType_t conn_agent_type);

/** 
  * @brief suspend a connection to (aka: connecting) state
  * 
  * vmtl_conn_deactive() will stop the specified connection,
  * note: all streams bound to the connection would be unbound implicitly
  *
  * @param connc represents the target connection obj
  *	@return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_conn_deactive(void *connc);

/** 
  * @brief teardown connection(essentially indicates underlying channels)
  * for a service stream
  * 
  * vmtl_release_conn() destroy related objs, close socket(s), free queues,
  * unregister the stream's adapting methods such as pkt-processing.
  * The peer(receiver in general) initiates an end of the SIP
  * session and call this function prior to the listener, when the
  * listener receives the SIP request, will do the same thing.
  * 
  * @param connc point to a connection obj
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_release_conn(void *connc);

/** 
  * @brief a customer call this when it indeed uses vmtl no more
  * 
  * vmtl_uninit() will anyway accept customers' killing request
  * even though it perhaps was a bad judgement. The good news is
  * vmtl will not really yield until all references clear.
  *
  * @return 0:success, -1:failed
  *
  * @pre { all streams have been unregistered }
  */
VMTL_IMPORT int vmtl_uninit(void);

/**
 * @brief forward vmtl's internal events to given connection
 *
 * in general, vmtl_transfer_event() is used in forward mode
 *
 * @param connc : point to connection obj
 * @param evt : internal event or command or signal
 * @param data : carried data (optional)
 * @return 0:success, -1:failed
 */
VMTL_IMPORT int vmtl_transfer_event(void *connc, int evt, void *data);

/**
 * @brief parse vmtl packet, extract data description
 *
 * @param data : vmtl packet including head and payload
 * @retval packet_desp : data description includes stream type, stream sub type and sequence number
 * @return 0:success, -1:failed
 */
VMTL_IMPORT int vmtl_parse_packet(const char *data, outDataDesp_t *packet_desp);

// internal version of vmtl_parse_packet(), only for debug
VMTL_IMPORT int __vmtl_parse_packet(void *conn, const char *data, void *result);

/** 
  * @brief acquire specific session's system time difference
  * 
  * if measurement has not been launched yet, start it first
  * specific session's difference of sender's and receiver's
  * system time goes to report via this API so that upper layers
  * can do synchronisation
  * 
  * @param connection obj which belongs its session
  * @retval pointer to diff value
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_get_systime_diff(void *connc, int64_t *diff);

/** 
  * @brief get vmtl's version number
  * 
  * Version number is a number line which represents
  * vmtl's version.
  * 
  * @return vmtl's version number, -1 if failed
  */
VMTL_IMPORT int vmtl_get_vernum(void);

/** 
  * @brief get vmtl's version string
  * 
  * Version string is the human-readable format of vmtl's
  * version number. The version consists of major version,
  * minor version, bug fix version, project stage and
  * revision. An example is as v1.2.3d4.
  * 
  * @retval ppstr point to a string
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_get_verstr(char **ppstr);

/**
  * @brief set vmtl's log level
  *
  * set vmtl's log level to none/error/warn/info/debug
  *
  * @param log_level :log level string
  */
VMTL_IMPORT void vmtl_set_loglevel(const char *log_level);

/**
  * @brief set a callback for log print
  *
  * set an external callback for log print function as
  * replacement of the internal one.
  *
  * @param cb :log print function
  * @return 0:success, -1:failed
  */
VMTL_IMPORT int vmtl_log_redirect(ThreadSafeLogCallBack cb);

/**
  * @brief process vmtl cmd
  *
  * a third part tool uses this to process vmtl cmdline, the
  * reply will be shown using the callback registered by
  * vmtl_cmd_reply_reg().
  *
  * @param cmd :the cmd string
  * @return 0:success, -1:failed
  *
  * @pre { a valid reply callback exists }
  */
VMTL_IMPORT void vmtl_cmdline_parser(const char *cmd);

/**
  * @brief register a callback for vmtl cmd reply
  *
  * register an external callback for presenting vmtl cmd reply
  * in a third part tool
  *
  * @param cb :cmd reply function
  * @return 0:success, -1:failed
  *
  * @pre { using non VMTL_STANDALONE_CMDLINE mode }
  */
VMTL_IMPORT int vmtl_cmd_reply_reg(vmtl_cmd_reply cb);


#ifdef __cplusplus
}
#endif

#endif //VMTL_H

/// @Fin

