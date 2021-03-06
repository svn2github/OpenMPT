/*
 * BuildSettings.h
 * ---------------
 * Purpose: Global, user settable compile time flags (and some global system header configuration)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once



#include "CompilerDetect.h"



#ifdef MODPLUG_TRACKER

//#define NO_PCH

// Use inline assembly at all
#define ENABLE_ASM

#else

// Do not use precompiled headers (prevents include of commonly used headers in stdafx.h)
#define NO_PCH

// Do not use inline asm in library builds. There is just about no codepath which would use it anyway.
//#define ENABLE_ASM

#endif



// inline assembly requires MSVC compiler
#if defined(ENABLE_ASM) && MPT_COMPILER_MSVC && defined(_M_IX86)

// Generate general x86 inline assembly.
#define ENABLE_X86

// Generate inline assembly using MMX instructions (only used when the CPU supports it).
#define ENABLE_MMX

// Generate inline assembly using SSE instructions (only used when the CPU supports it).
#define ENABLE_SSE

// Generate inline assembly using AMD specific instruction set extensions (only used when the CPU supports it).
#define ENABLE_X86_AMD

#endif // ENABLE_ASM



#if defined(MODPLUG_TRACKER) && defined(LIBOPENMPT_BUILD)

#error "either MODPLUG_TRACKER or LIBOPENMPT_BUILD has to be defined"

#elif defined(MODPLUG_TRACKER)

// Enable built-in test suite.
#ifdef _DEBUG
#define ENABLE_TESTS
#endif

// Disable any file saving functionality (not really useful except for the player library)
//#define MODPLUG_NO_FILESAVE

// Disable any debug logging
#ifndef _DEBUG
#define NO_LOGGING
#endif

// Disable all runtime asserts
#ifndef _DEBUG
#define NO_ASSERTS
#endif

// Disable std::istream support in class FileReader (this is generally not needed for the tracker, local files can easily be mmapped as they have been before introducing std::istream support)
#define NO_FILEREADER_STD_ISTREAM

// Disable unarchiving support
//#define NO_ARCHIVE_SUPPORT

// Disable the built-in reverb effect
//#define NO_REVERB

// Disable built-in miscellaneous DSP effects (surround, mega bass, noise reduction) 
//#define NO_DSP

// Disable the built-in equalizer.
//#define NO_EQ

// Disable the built-in automatic gain control
//#define NO_AGC

// Define to build without ASIO support; makes build possible without ASIO SDK.
//#define NO_ASIO 

// (HACK) Define to build without VST support; makes build possible without VST SDK.
//#define NO_VST

// Define to build without portaudio.
//#define NO_PORTAUDIO

// Define to build without MO3 support.
//#define NO_MO3

// Define to build without DirectSound support.
//#define NO_DSOUND

// Define to build without FLAC support
//#define NO_FLAC

// Define to build without zlib support
//#define NO_ZLIB

// Define to build without miniz support
#define NO_MINIZ

// Define to build without MP3 import support (via mpg123)
//#define NO_MP3_SAMPLES

// Do not build libmodplug emulation layer (only makes sense for library)
#define NO_LIBMODPLUG

// Do not build xmplay input plugin code (only makes sense for library)
#define NO_XMPLAY

// Do not build winamp input plugin code (only makes sense for library)
#define NO_WINAMP

// Do not build libopenmpt C api
#define NO_LIBOPENMPT_C

// Do not build libopenmpt C++ api
#define NO_LIBOPENMPT_CXX

#elif defined(LIBOPENMPT_BUILD)

#if defined(LIBOPENMPT_BUILD_TEST)
#define ENABLE_TESTS
#else
#define MODPLUG_NO_FILESAVE
#endif
//#define NO_ASSERTS
//#define NO_LOGGING
//#define NO_FILEREADER_STD_ISTREAM
#define NO_ARCHIVE_SUPPORT
#define NO_REVERB
#define NO_DSP
#define NO_EQ
#define NO_AGC
#define NO_ASIO
#define NO_VST
#define NO_PORTAUDIO
#if !defined(_WIN32) || (defined(_WIN32) && !defined(_M_IX86))
#define NO_MO3
#endif
#define NO_DSOUND
#define NO_FLAC
#if !defined(MPT_WITH_ZLIB)
#ifndef NO_ZLIB
#define NO_ZLIB
#endif
#endif
//#define NO_MINIZ
#define NO_MP3_SAMPLES
#if defined(LIBOPENMPT_BUILD_TEST)
#define NO_LIBMODPLUG
#endif
#if !defined(_WIN32) || (defined(_WIN32) && !defined(_M_IX86)) || defined(LIBOPENMPT_BUILD_TEST)
#define NO_WINAMP
#endif
#if !defined(_WIN32) || (defined(_WIN32) && !defined(_M_IX86)) || defined(LIBOPENMPT_BUILD_TEST)
#define NO_XMPLAY
#endif
//#define NO_LIBOPENMPT_C
//#define NO_LIBOPENMPT_CXX

#else

#error "either MODPLUG_TRACKER or LIBOPENMPT_BUILD has to be defined"

#endif



// fixing stuff up

#if !defined(MODPLUG_TRACKER) && defined(NO_MO3)
// For library builds, windows.h is only required for LoadLibrary.
//#define NO_WINDOWS_H
#endif

#if !defined(ENABLE_MMX) && !defined(NO_REVERB)
#define NO_REVERB // reverb requires mmx
#endif

#if !defined(ENABLE_X86) && !defined(NO_DSP)
#define NO_DSP // DSP requires x86 inline asm
#endif

#if defined(ENABLE_TESTS) && defined(MODPLUG_NO_FILESAVE)
#undef MODPLUG_NO_FILESAVE // tests recommend file saving
#endif

#if !defined(NO_ZLIB) && !defined(NO_MINIZ)
// Only one deflate implementation should be used. Prefer zlib.
#define NO_MINIZ
#endif

#if defined(MPT_PLATFORM_BIG_ENDIAN) && !defined(MODPLUG_NO_FILESAVE)
#define MODPLUG_NO_FILESAVE // file saving is broken on big endian
#endif

#if !defined(NO_LIBMODPLUG)
#if !defined(LIBOPENMPT_BUILD) || (defined(LIBOPENMPT_BUILD) && defined(_WIN32) && !defined(LIBOPENMPT_BUILD_DLL))
#define NO_LIBMODPLUG // libmodplug interface emulation requires libopenmpt dll build on windows
#endif
#endif

#if !defined(NO_WINAMP)
#if !defined(LIBOPENMPT_BUILD) || (defined(LIBOPENMPT_BUILD) && !defined(LIBOPENMPT_BUILD_DLL))
#define NO_WINAMP // winamp plugin requires libopenmpt dll build
#endif
#endif

#if !defined(NO_XMPLAY)
#if !defined(LIBOPENMPT_BUILD) || (defined(LIBOPENMPT_BUILD) && !defined(LIBOPENMPT_BUILD_DLL))
#define NO_XMPLAY // xmplay plugin requires libopenmpt dll build
#endif
#endif



#if MPT_COMPILER_MSVC
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#if defined(_WIN32) && !defined(NO_WINDOWS_H)

#if MPT_COMPILER_MSVC && MPT_MSVC_AT_LEAST(2010,0)
#define _WIN32_WINNT        0x0501 // _WIN32_WINNT_WINXP
#else
#define _WIN32_WINNT        0x0500 // _WIN32_WINNT_WIN2000
#endif
#define WINVER              _WIN32_WINNT
#define WIN32_LEAN_AND_MEAN

// windows.h excludes
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOPROFILER        // Profiler interface.
#define NOMCX             // Modem Configuration Extensions

// mmsystem.h excludes
#define MMNODRV
//#define MMNOSOUND
//#define MMNOWAVE
//#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
//#define MMNOTIMER
#define MMNOJOY
#define MMNOMCI
//#define MMNOMMIO
//#define MMNOMMSYSTEM

// mmreg.h excludes
#define NOMMIDS
//#define NONEWWAVE
#define NONEWRIFF
#define NOJPEGDIB
#define NONEWIC
#define NOBITMAP

#endif

#if MPT_COMPILER_MSVC
#define _CRT_SECURE_NO_WARNINGS		// Define to disable the "This function or variable may be unsafe" warnings.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES			1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT	1
#define _SCL_SECURE_NO_WARNINGS
#endif
