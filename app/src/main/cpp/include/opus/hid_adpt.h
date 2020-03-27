/**
  * @file hid_adpt.h
  * @author Initial	Lian <lianronggang@vanxum.com>		30/3/2018
  *         Updated	Xxx  <xxx@vanxum.com>		        dd/mm/yyyy
  * @brief define all things needed for adapting to reversed input
  *        events from human input device
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

#ifndef HID_ADPT_H
#define HID_ADPT_H //{HID_ADPT_H


#include "adaptor.h"


#define MAX_HID_DATA_PACKET_LENGTH 255     //unit:byte
#define HID_DATA_TYPE_MOUSE     1   //mouse moving
#define HID_DATA_TYPE_KEYB      2   //keyboard or click or kb-status
#define HID_DATA_TYPE_ICON      3   //cursor icon

/// rvs-input data map
typedef struct _hidAdaptor_t {
    adptOps_t ops;
    /* placeholder */
} hidAdaptor_t;

typedef struct _hidDescription_t {
    APT_PUBLIC_DESP_FIELDS
    /* placeholder */
} hidDesp_t;

typedef enum _hidEarlyManipulation_t {
    kHidInvalidEDS = 0,
    kHidClear = 1,
    kHidAsUsual,
    kHidSendFat, //fat data whose size is bigger than MAX_HID_DATA_PACKET_LENGTH
    kHidEdsMax
} hidEDS_t;

//apt definition of rvs-input's sending queue
typedef hidDesp_t rvsInputApt_t; //rename it as the adaptor type

typedef struct _hidParseResult_t {
    uint8_t comp_type;
    hidEDS_t early_manipulate;
    /* placeholder */
} hidParseRes_t;

// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int hid_data_prepare(uint32_t length, void *desp, void *result)
{
    hidParseRes_t *ret = (hidParseRes_t *) result;

    if (length > MAX_HID_DATA_PACKET_LENGTH)
    {
        printf("[hid-apt|err|%s]data is fat one!\n", __func__);
        ret->early_manipulate = kHidSendFat;
        //return -1;
    }

    ret->early_manipulate = kHidAsUsual;

    //printf("[hid-apt|warning|%s]this is a stub!\n", __func__);
    return 0;
}


#endif //}HID_ADPT_H

// @FIN
