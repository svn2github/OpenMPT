/*
 * StdAfx.h
 * --------
 * Purpose: Include file for standard system include files, or project specific include files that are used frequently, but are changed infrequently. Also includes the global build settings from BuildSettings.h.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once


// has to be first
#include "BuildSettings.h"


#if defined(MODPLUG_TRACKER)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxcview.h>
#include <afxdlgs.h>
#include <afxole.h>

#include <windowsx.h>

#if MPT_COMPILER_MSVC
#pragma warning(disable:4201)
#endif
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#if MPT_COMPILER_MSVC
#pragma warning(default:4201)
#endif

#endif // MODPLUG_TRACKER


#ifndef NO_PCH

#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#endif // NO_PCH


// this will be available everywhere
#include "../common/typedefs.h"
// this adds:
// <memory>
// <new>
// <string>
// <vector>
// <cstdarg>
// <cstdint>
// <cstdio>
// <stdio.h>
// <windows.h> or just basic typedefs found in there


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
