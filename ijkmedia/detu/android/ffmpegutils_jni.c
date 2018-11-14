//
// Created by chao on 2016/11/29.
//
#include "com_player_util_FfmpegUtils.h"
#include "../ffmpegutil.h"
#include "../log.h"
#include <stdio.h>
#include <stdlib.h>

JNIEXPORT jobject JNICALL Java_com_player_util_FfmpegUtils_geVideoThumbBitmap
        (JNIEnv *env, jobject jobj, jstring jstringFileAbsolutePath) {
    const char *fileAbsolutePath = (*env)->GetStringUTFChars(env, jstringFileAbsolutePath, 0);
    int width;
    int height;
    void *outData;
    int size;
    bool ret = videoGetThumb(fileAbsolutePath, &outData, &size, &width, &height);
    (*env)->ReleaseStringUTFChars(env, jstringFileAbsolutePath, fileAbsolutePath);
    if (!ret) {
        return NULL;
    }
    jclass bitmapConfig = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
    jfieldID rgba8888FieldID = (*env)->GetStaticFieldID(env, bitmapConfig, "ARGB_8888",
                                                        "Landroid/graphics/Bitmap$Config;");
    jobject rgba8888Obj = (*env)->GetStaticObjectField(env, bitmapConfig, rgba8888FieldID);

    jclass bitmapClass = (*env)->FindClass(env, "android/graphics/Bitmap");
    jmethodID createBitmapMethodID = (*env)->GetStaticMethodID(env, bitmapClass, "createBitmap",
                                                               "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jobject bitmapObj = (*env)->CallStaticObjectMethod(env, bitmapClass, createBitmapMethodID, width,
                                                       height, rgba8888Obj);
    unsigned char *bitmap = (unsigned char *) outData;

    jintArray pixels = (*env)->NewIntArray(env, width * height);
    for (int i = 0; i < width * height; i++) {
        unsigned char red = bitmap[i * 4];
        unsigned char green = bitmap[i * 4 + 1];
        unsigned char blue = bitmap[i * 4 + 2];
        unsigned char alpha = bitmap[i * 4 + 3];
        int currentPixel = (alpha << 24) | (red << 16) | (green << 8) | (blue);
        (*env)->SetIntArrayRegion(env, pixels, i, 1, &currentPixel);
    }
    jmethodID setPixelsMid = (*env)->GetMethodID(env, bitmapClass, "setPixels", "([IIIIIII)V");
    (*env)->CallVoidMethod(env, bitmapObj, setPixelsMid, pixels, 0, width, 0, 0, width, height);
    free(outData);
    return bitmapObj;
}

JNIEXPORT jboolean JNICALL Java_com_player_util_FfmpegUtils_getVideoThumbJpg
        (JNIEnv * env, jobject jobj, jstring jstringFileAbsolutePath, jstring jstringSaveAbsolutePath) {

}