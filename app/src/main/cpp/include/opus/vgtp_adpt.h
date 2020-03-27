/**
  * @file vgtp_adpt.h
  * @author Initial	Lian <lianronggang@vanxum.com>		16/12/2017
  *         Updated	xxx<xxx@vanxum.com>					dd/mm/yyyy
  * @brief define all things needed for adapting to vgtp IE
  *
  * This file includes data structures, early data processing
  * callback/function set ......
  * 
  * Copyright(c) 2014-2018 Vanxum Inc.. All rights reserved.
  * 
  */


#ifdef PRAGMA_ONCE
#pragma once
#endif

#ifndef VGTP_ADPT_H
#define VGTP_ADPT_H //{VGTP_ADPT_H


#include "adaptor.h"


#define MAX_VGTP_LAYER_COUNT 10  //最大层数

#define VGTP_FRAME_TYPE_IREF     1   //I 参考帧
#define VGTP_FRAME_TYPE_PREF     2   //p 参考帧
#define VGTP_FRAME_TYPE_NORM     3   //p 普通帧
#define VGTP_FRAME_TYPE_FDBK     4   //vgtp反馈

///@note the value must be same as AVC stream's
#define VGTP_FRAME_TYPE_H264_I6  6   //H264 0x06 I帧
#define VGTP_FRAME_TYPE_H264_I7  7   //H264 0x07 I帧
#define VGTP_FRAME_TYPE_H264_I8  8   //H264 0x08 I帧
#define VGTP_FRAME_TYPE_H264     9   //H264 普通帧

#define VGTP_FRAME_TYPE_SPS      11   //sps
#define VGTP_FRAME_TYPE_SPS_ACK  12   //sps ack


//vgtp data map
typedef struct _vgtpAdaptor_t {
    adptOps_t ops;
    /* placeholder */
} vgtpAdaptor_t;

//uint64_t pts;   // capture timestamp or delay from capture time (us)
//uint8_t type;   // 0:I-Ref, 1:P-Ref, 2:P-Normal, 3:H264, 4:feedback
typedef struct _vgtpDescription_t {
    APT_PUBLIC_DESP_FIELDS  //should be the first place as in adaptor.h
} vgtpDesp_t;

typedef enum _vgtpEarlyManipulation_t {
    kVgtpInvalidEDS = 0,
    kVgtpClear = 1,
    kVgtpAsUsual = 2
    //placeholder
} vgtpEDS_t;

//vgtp的队列的数据apt定义
typedef vgtpDesp_t vgtpMediaApt_t; //rename it as the adaptor type

/// for WRED or RED or Non-RED processing, defined by the stream generator,
/// the invokers include but not limited to bw limiter
// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int vgtp_non_red(void *descriptor) {
    int drop_or_not = 1;
    vgtpDesp_t *desp = (vgtpDesp_t *) descriptor;
    switch (desp->sub_type) {
        case VGTP_FRAME_TYPE_IREF:
        //case VGTP_FRAME_TYPE_PREF:
        case VGTP_FRAME_TYPE_H264_I6:
        case VGTP_FRAME_TYPE_H264_I7:
        case VGTP_FRAME_TYPE_H264_I8:
            drop_or_not = 0;
            break;
        default:
            break;
    }
    return drop_or_not;
}

#endif //} VGTP_ADPT_H

// @Fin

