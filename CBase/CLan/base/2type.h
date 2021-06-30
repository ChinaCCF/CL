#ifndef __clan_base_type__
#define __clan_base_type__

#include "1config.h"
#include <stdint.h> 
#include <float.h>   

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;
 
typedef wchar_t  wchar;

static constexpr s8 max_s8 = (s8)0x7F;
static constexpr s8 min_s8 = (s8)0x80;

static constexpr s16 max_s16 = (s16)0x7FFF;
static constexpr s16 min_s16 = (s16)0x8000;

static constexpr s32 max_s32 = (s32)0x7FFFFFFF;
static constexpr s32 min_s32 = (s32)0x80000000;

static constexpr s64 max_s64 = (s64)0x7FFFFFFFFFFFFFFF; 
static constexpr s64 min_s64 = (s64)0x8000000000000000;
 
static constexpr u8 max_u8 = (u8)0xFF;  
static constexpr u16 max_u16 = (u16)0xFFFF;  
static constexpr u32 max_u32 = (u32)0xFFFFFFFF;  
static constexpr u64 max_u64 = (u64)0xFFFFFFFFFFFFFFFF; 

#if defined(__LLP64__) || defined(__LP64__) || defined(_M_X64) || defined(_WIN64)
#define X64
typedef s64 st;
typedef u64 ut;
#else  
typedef s32 st;
typedef u32 ut;
#endif

#define __cl_W(x) L##x
#define _cl_W(x) __cl_W(x)

//仅用来调试
#ifdef XDEBUG 
#define cl_dbg(x) do{if(!(x)) {char* _XXXXX = nullptr; *_XXXXX = 0;} } while(0)
#else
#define cl_dbg(x) 
#endif

#ifdef _MSC_VER
#   define cl_thread thread_local
#else 
#   define cl_thread
#endif

#endif//__clan_base_type__ 