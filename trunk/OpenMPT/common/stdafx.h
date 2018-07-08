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

#if MPT_OS_WINDOWS

#if !defined(MPT_BUILD_WINESUPPORT)

#ifndef MPT_MFC_FULL
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS	// Do not include support for MFC controls in dialogs (reduces binary bloat; remove this #define if you want to use MFC controls)
#endif // !MPT_MFC_FULL
// cppcheck-suppress missingInclude
#include <afxwin.h>         // MFC core and standard components
// cppcheck-suppress missingInclude
#include <afxext.h>         // MFC extensions
// cppcheck-suppress missingInclude
#include <afxcmn.h>         // MFC support for Windows Common Controls
// cppcheck-suppress missingInclude
#include <afxcview.h>
// cppcheck-suppress missingInclude
#include <afxdlgs.h>
#ifdef MPT_MFC_FULL
// cppcheck-suppress missingInclude
#include <afxlistctrl.h>
#endif // MPT_MFC_FULL
// cppcheck-suppress missingInclude
#include <afxole.h>

#endif // !MPT_BUILD_WINESUPPORT

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <mmsystem.h>

#endif // MPT_OS_WINDOWS

#endif // MODPLUG_TRACKER


#if MPT_COMPILER_MSVC
#include <intrin.h>
#endif


// this will be available everywhere

#include "../common/typedefs.h"
// <limits>
// <memory>
// <new>
// <cstddef>
// <cstdint>
// <stdint.h>

#include "../common/mptTypeTraits.h"
// <type_traits>

#include "../common/mptString.h"
// <algorithm>
// <limits>
// <string>
// <type_traits>
// <cstring>

#include "../common/mptStringFormat.h"

#include "../common/mptPathString.h"

#include "../common/Logging.h"

#include "../common/misc_util.h"
// <algorithm>
// <limits>
// <string>
// <type_traits>
// <vector>
// <cmath>
// <cstdlib>
// <cstring>
// <time.h>

#include <array>

// for std::abs
#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <math.h>


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
