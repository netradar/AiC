//
// Created by vanxum516 on 2020/3/17.

//

#include "com_vanxum_Aic_OpusJni.h"
#include "opus/opus.h"
OpusDecoder *decoder;
JNIEXPORT jint JNICALL Java_com_vanxum_Aic_OpusJni_OpusInit
  (JNIEnv *, jobject)
  {
    int err;
    decoder = opus_decoder_create(48000, 2, &err);
    if (err<0)
    {
    return -1;
    }
    return 0;
  }

/*
 * Class:     com_vanxum_Aic_OpusJni
 * Method:    Opusdecode
 * Signature: ([B[SI)I
 */
JNIEXPORT jint JNICALL Java_com_vanxum_Aic_OpusJni_Opusdecode
  (JNIEnv *env,jobject thiz, jbyteArray encoded, jshortArray lin, int size)
  {
        int err;
        int i;
        int frame_size;

        int nbBytes;

        if(decoder == NULL){
            return 0;
        }
        jshort *pSamples = env->GetShortArrayElements(lin, 0);
        jbyte *pBytes = env->GetByteArrayElements(encoded, 0);
        jsize nByteSize = env->GetArrayLength(encoded);
        jsize nShortSize = env->GetArrayLength(lin);
        frame_size = opus_decode(decoder, (unsigned char *)pBytes, size, (opus_int16 *)pSamples, 480, 0);

        if(frame_size <0){
            return 0;
        }

        int nSize = frame_size*2;
       // (env)->SetShortArrayRegion(lin, 0, nSize, (jshort*)pSamples);

        return nSize;
  }