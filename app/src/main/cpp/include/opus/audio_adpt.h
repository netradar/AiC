/**
  * @file audio_adpt.h
  * @author Initial	Lian <lianronggang@vanxum.com>		1/2/2018
  *         Updated	Lian <lianronggang@vanxum.com>		12/3/2018
  * @brief define all things needed for adapting to media audio
  *        and human's voice
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

#ifndef AUDIO_ADPT_H
#define AUDIO_ADPT_H //{AUDIO_ADPT_H


#include "adaptor.h"


#define MAX_AUDIO_DATA_PACKET_LENGTH 512
#define MAX_VOICE_DATA_PACKET_LENGTH	MAX_AUDIO_DATA_PACKET_LENGTH


/// audio data map
typedef struct _audioAdaptor_t {
	adptOps_t ops;
	/* placeholder */
} audioAdaptor_t;

typedef struct _audioDescription_t {
    APT_PUBLIC_DESP_FIELDS
	/* placeholder */
} audioDesp_t;

typedef enum _audioEarlyManipulation_t {
	kAudioInvalidEDS = 0,
	kAudioClear = 1,
	kAudioSendNormal = 2
	/* placeholder */
} audioEDS_t;

//apt definition of audio's sending queue
typedef audioDesp_t audioBareApt_t; //rename it as the adaptor type

typedef struct _audioParseResult_t {
	audioEDS_t early_manipulate;
	/* placeholder */
} audioParseRes_t;


/// voice data map
typedef struct _voiceAdaptor_t {
	adptOps_t ops;
	/* placeholder */
} voiceAdaptor_t;

typedef struct _voiceDescription_t {
    APT_PUBLIC_DESP_FIELDS
	/* placeholder */
} voiceDesp_t;

typedef enum _voiceEarlyManipulation_t {
	kVoiceInvalidEDS = 0,
	kVoiceClear = 1,
	kVoiceSendNormal = 2
	/* placeholder */
} voiceEDS_t;

//apt definition of voice's sending queue
typedef voiceDesp_t voiceApt_t; //rename it as the adaptor type

typedef struct _voiceParseResult_t {
	voiceEDS_t early_manipulate;
	/* placeholder */
} voiceParseRes_t;

// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int audio_data_prepare(uint32_t length, void *desp, void *result)
{
    if (length > MAX_AUDIO_DATA_PACKET_LENGTH)
	{
	    printf("[audio-apt|err|%s]data is too big!\n", __func__);
	    return -1;
    }

	///@todo { audio data early processing }
	//printf("[audio-apt|warning|%s]this is a stub!\n", __func__);
	return 0;
}

/// for WRED or RED or Non-RED processing, defined by the stream generator,
/// the invokers include but not limited to bw limiter
// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int audio_non_red(void *descriptor) {
	int drop_or_not = 1;
	audioDesp_t *desp = (audioDesp_t *) descriptor;

	printf("[audio-apt|warning|%s]this is a stub!\n", __func__);

	return drop_or_not;
}

// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int voice_data_prepare(uint32_t length, void *desp, void *result)
{
	if (length > MAX_VOICE_DATA_PACKET_LENGTH)
	{
		printf("[voice-apt|err|%s]data is too big!\n", __func__);
		return -1;
	}

	///@todo { voice data early processing }
	//printf("[voice-apt|warning|%s]this is a stub!\n", __func__);
	return 0;
}

/// for WRED or RED or Non-RED processing, defined by the stream generator,
/// the invokers include but not limited to bw limiter
// DO NOT CHANGE THE FUNCTION NAME AS WELL AS THE PASS-IN PARAMETERS' ORDER & COUNT!
static inline int voice_non_red(void *descriptor) {
	int drop_or_not = 1;
	voiceDesp_t *desp = (voiceDesp_t *) descriptor;

	printf("[voice-apt|warning|%s]this is a stub!\n", __func__);

	return drop_or_not;
}


#endif //} AUDIO_ADPT_H

// @FIN

