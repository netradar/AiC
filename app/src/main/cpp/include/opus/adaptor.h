/**
  * @file common_adpt.h
  * @author Initial	Lian <lianronggang@vanxum.com>		1/2/2018
  *         Updated	xxx<xxx@vanxum.com>					dd/mm/yyyy
  * @brief define all things needed for adapting to common streams
  *
  * This file includes data structures, callback/function set ......
  * 
  * Copyright(c) 2014-2018 Vanxum Inc.. All rights reserved.
  * 
  */


#ifdef PRAGMA_ONCE
#pragma once
#endif

#ifndef ADAPTOR_H
#define ADAPTOR_H //{COMMON_ADPT_H


///the pre-defined public fields of xxx's description struct (xxx is a stream)
///@todo { tell strm-bound type by comp_type }
/**
 * pts: data sourcing timestamp
 * frame_id: data sequence number, as passthrough sn in forward mode
 * type: (stream) type of data, 0 is reserved
 * sub_type: sub-type of data, 0 is reserved, max is 31 for now
 * slice_cnt: how many slices contained in this frame, if no slice, make this ZERO!
 * slice_no: number of the current slice, if no slice, don't care this
 */
#define APT_PUBLIC_DESP_FIELDS	\
	uint64_t pts;				\
	uint32_t frame_id;          \
    uint8_t type;               \
    uint8_t sub_type;           \
    uint8_t slice_cnt;          \
    uint8_t slice_no;           \
    uint8_t max_layer;          \
    uint8_t layer;

/** prepare_data: parse service data from app layer to --
  * 1.check validity of incoming service data
  * 2.parse service data, seperate them and map them to vmtl's type concerning transport
  * non_random_early_drop: tell vmtl which data can be dropped
  */
typedef struct _adaptorOps_t {
	int (*prepare_data)(uint32_t length, void *descriptor, void *result); //解析上层来的业务报文
    int (*non_random_early_drop)(void *descriptor); //NRED processing for media stream
} adptOps_t;

// common data map
typedef struct _adaptor_t {
	adptOps_t ops;
} adaptor_t;

/**
 * sample: every stream is able to define its data description fields
 * in struct xxxDesp_t of xxx_adpt.h (xxx can be stream name) with
 * the public fields, a sample is as follows:
 *
 * - APT_PUBLIC_DESP_FIELDS (aka comp_type) indicates which sub-type does
 * the given data belong to, e.g. 1 represents the main media stream, 2
 * represents the sub media stream, 3 represents the auxiliary stream,
 * the biggest can reach to 9 and note 10 & 11 are reserved for ice
 * holing packets and ddc negotiating packets
 *
 * - media_desp_field_x user-defined media stream descriptor field, can be
 * POD type or aggregate type
 */
typedef struct _vmtlDataDescription_t {
    APT_PUBLIC_DESP_FIELDS   ///@note DO NOT CHANGE THE POSITION OF PUBLIC FIELDS
} vmtlDataDesp_t;

#endif //} ADAPTOR_H

// @FIN

