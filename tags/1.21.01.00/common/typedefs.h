/*
 * typedefs.h
 * ----------
 * Purpose: Basic data type definitions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#ifndef nullptr
#define nullptr		0
#endif

//  CountOf macro computes the number of elements in a statically-allocated array.
#if _MSC_VER >= 1400
	#define CountOf(x) _countof(x)
#else
	#define CountOf(x) (sizeof(x)/sizeof(x[0]))
#endif

//Compile time assert.
#ifndef C_ASSERT
#define C_ASSERT(expr)				typedef char __C_ASSERT__[(expr)?1:-1]
#endif
#define STATIC_ASSERT(expr)			C_ASSERT(expr)
#ifndef static_assert
#define static_assert(expr, msg)	C_ASSERT(expr)
#endif

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

const int8 int8_min	    = -127-1;
const int16 int16_min   = -32767-1;
const int32 int32_min   = -2147483647-1;
const int64 int64_min   = -9223372036854775807-1;

const int8 int8_max     = 127;
const int16 int16_max   = 32767;
const int32 int32_max   = 2147483647;
const int64 int64_max   = 9223372036854775807;

const uint8 uint8_max   = 255;
const uint16 uint16_max = 65535;
const uint32 uint32_max = 4294967295;
const uint64 uint64_max = 18446744073709551615;

typedef float float32;
