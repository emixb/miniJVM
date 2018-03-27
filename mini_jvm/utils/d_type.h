//
// Created by gust on 2017/9/25.
//

#ifndef MINI_JVM_JVM_TYPE_H
#define MINI_JVM_JVM_TYPE_H

#include "sys/types.h"
#include "stdlib.h"
#include "stdint.h"

// x86   x64 ...
#define __JVM_LITTLE_ENDIAN__ 1
// arm
#define __JVM_BIG_ENDIAN__ 0


#if defined(__MINGW_H) || defined(__MINGW32_MAJOR_VERSION)
#define __JVM_OS_MINGW__ 1
#endif
#ifdef _CYGWIN_CONFIG_H
#define __JVM_OS_CYGWIN__ 1
#endif
#ifdef __DARWIN_C_ANSI
#define __JVM_OS_MAC__ 1
#endif
#if defined(__GNU_LIBRARY__) || defined(__ANDROID__)
#define __JVM_OS_LINUX__ 1
#endif

#if defined(_MSC_VER)
#define __JVM_OS_VS__ 1
#endif


//compile
#define __C99
//libary option : -lpthread -lws2_32


//#define __DEBUG

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char u8;
typedef signed char s8;
typedef char c8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;
typedef double f64;
typedef unsigned long long int u64;
typedef signed long long int s64;
typedef void *__refer;


extern s64 heap_size;


//======================= memory manage =============================


void *jvm_calloc(u32 size);

void *jvm_malloc(u32 size);

void jvm_free(void *ptr);

void *jvm_realloc(void *pPtr, u32 size);


#ifdef __cplusplus
}
#endif


#endif //MINI_JVM_JVM_TYPE_H
