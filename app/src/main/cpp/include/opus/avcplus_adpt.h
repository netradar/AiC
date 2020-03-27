/**
  * @file avcplus_adpt.h
  * @author Initial	Lian <lianronggang@vanxum.com>		1/2/2018
  *         Updated	Lian <lianronggang@vanxum.com>		12/3/2018
  * @brief define all things needed for adapting to plain H.264 and H.264 plus
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

#ifndef AVCPLUS_ADPT_H
#define AVCPLUS_ADPT_H //{AVCPLUS_ADPT_H


#include "adaptor.h"


#define MAX_AVC_DATA_PACKET_LENGTH 2000000  	//AVC数据分组最大长度
#define MAX_H264P_DATA_PACKET_LENGTH		MAX_AVC_DATA_PACKET_LENGTH


#define AVC_FRAME_TYPE_H264_I6  6   //H264 0x06 I帧
#define AVC_FRAME_TYPE_H264_I7  7   //H264 0x07 I帧
#define AVC_FRAME_TYPE_H264_I8  8   //H264 0x08 I帧
#define AVC_FRAME_TYPE_H264     9   //H264 普通帧


// H.264 video data map
typedef struct _avcAdaptor_t {
	adptOps_t ops;
	/* placeholder */
} avcAdaptor_t;

//uint64_t pts;  //capture timestamp or delay from capture time (us)
typedef struct _avcDescription_t {
    APT_PUBLIC_DESP_FIELDS
} avcDesp_t;

typedef enum _avcEarlyManipulation_t {
	kAvcInvalidEDS = 0,
	kAvcClear = 1,
	kAvcSendNormal = 2
	//placeholder
} avcEDS_t;

//apt definition of avc's sending queue
typedef avcDesp_t avcBareApt_t; //rename it as the adaptor type

typedef struct _avcParseResult_t {
	//avcSubStrm_t sub_stream;
	avcEDS_t early_manipulate;
	/* placeholder */
} avcParseRes_t;


// 264plus video data map
typedef struct _h264plusAdaptor_t {
	adptOps_t ops;
	/* placeholder */
} h264pAdaptor_t;

typedef struct _h264pDescription_t {
    APT_PUBLIC_DESP_FIELDS
	/* placeholder */
} h264pDesp_t;

typedef enum _h264pEarlyManipulation_t {
	kH264pInvalidEDS = 0,
	kH264pClear = 1,
	kH264pSendNormal = 2
	//placeholder
} h264pEDS_t;

//apt definition of avc's sending queue
typedef h264pDesp_t h264plusApt_t; //rename it as the adaptor type

typedef struct _h264pParseResult_t {
	//h264pSubStrm_t sub_stream;
	h264pEDS_t early_manipulate;
	/* placeholder */
} h264pParseRes_t;


// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int avc_data_prepare(uint32_t length, void *descriptor, void *result) {
	if (length > MAX_AVC_DATA_PACKET_LENGTH)
	{
		printf("[avc-apt|err|%s]data is too big!\n", __func__);
		return -1;
	}

	///@todo { h.264 plain data early processing }
	//printf("[avc-apt|warning|%s]this is a stub!\n", __func__);
	return 0;
}

/// for WRED or RED or Non-RED processing, defined by the stream generator,
/// the invokers include but not limited to bw limiter
// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int avc_non_red(void *descriptor) {
	int drop_or_not = 0;
	avcDesp_t *desp = (avcDesp_t *) descriptor;

	printf("[avc-apt|warning|%s]this is a stub!\n", __func__);

	return drop_or_not;
}

// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int h264p_data_prepare(uint32_t length, void *descriptor, void *result) {
	if (length > MAX_H264P_DATA_PACKET_LENGTH)
	{
		printf("[h264p-apt|err|%s]data is too big!\n", __func__);
		return -1;
	}

	///@todo { 264plus data early processing }
	printf("[h264p-apt|warning|%s]this is a stub!\n", __func__);
	return 0;
}

/// for WRED or RED or Non-RED processing, defined by the stream generator,
/// the invokers include but not limited to bw limiter
// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int h264p_non_red(void *descriptor) {
	int drop_or_not = 1;
	h264pDesp_t *desp = (h264pDesp_t *) descriptor;

	printf("[h264p-apt|warning|%s]this is a stub!\n", __func__);

	return drop_or_not;
}

#endif //} AVCPLUS_ADPT_H

// @FIN

