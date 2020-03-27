/*
 * Copyright(c) 2014-2018 Vanxum Inc.. All rights reserved.
 *
 * Authors:
 *    Initial	Lian <lianronggang@vanxum.com>		01/12/2017
 *    Updated	xxx<xxx@vanxum.com>					xx/xx/201x
 */

//#ifdef defined ((__GNUC__)&&((__GNUC__ >3)||(defined(__GNUC_MINOR__)&&(__GNUC__ ==3)&&(__GNUC_MINOR__ >=4)))) || \
//				((_MSC_VER)&&(_MSC_VER>=1020)) || \
//				(__CC_ARM)
#ifdef PRAGMA_ONCE
#pragma once
#endif

#ifndef VMTL_PORTABLE_H
#define VMTL_PORTABLE_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#if !defined(_WIN32) && !defined(_WIN32_WCE) //UNIX-like flavour
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#else //WIN32 flavour
#include <winsock2.h>
#include <ws2tcpip.h>

typedef  unsigned __int64 uint64_t; // VC
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;
typedef  unsigned char uint8_t;
typedef  int int32_t;
typedef  __int64 int64_t;
typedef __int16 int16_t;

#endif

#endif //VMTL_PORTABLE_H
/// @Fin

