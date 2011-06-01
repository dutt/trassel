#ifndef _TYPEDEFS_
#define _TYPEDEFS_

typedef unsigned int uint;

#if defined(WIN32) || defined(_WIN32)
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#else
# include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#endif

#define FLAGSATTRIBUTE(T) \
inline T operator|(T l, T r) \
{ return (T)((uint32)l|r); } \
inline static T operator&(T l, T r) \
{ return (T)((uint32)l&r); } \
inline static T& operator|=(T& l, T r) \
{ return l = (T)((uint32)l|r); } \
inline static T& operator&=(T& l, T r) \
{ return l = (T)((uint32)l&r); } \
inline static T operator*(bool l, T r) \
{ return (T)(l*(uint32)r); } \
inline static T operator*(T l, bool r) \
{ return (T)((uint32)l*r); } \
inline static T operator~(T a) \
{ return (T)~(uint32)a; } 
#endif
