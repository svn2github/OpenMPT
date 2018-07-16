/*
 * MPTrackUtil.h
 * -------------
 * Purpose: Various useful utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "BuildSettings.h"


#include <string>


OPENMPT_NAMESPACE_BEGIN


/*
 * Gets resource as raw byte data.
 * [in] lpName and lpType: parameters passed to FindResource().
 * Return: span representing the resource data, valid as long as hInstance is valid.
 */
mpt::const_byte_span GetResource(LPCTSTR lpName, LPCTSTR lpType);


CString GetErrorMessage(DWORD nErrorCode);

namespace Util
{
	// Get horizontal DPI resolution
	MPT_FORCEINLINE int GetDPIx(HWND hwnd)
	{
		HDC dc = ::GetDC(hwnd);
		int dpi = ::GetDeviceCaps(dc, LOGPIXELSX);
		::ReleaseDC(hwnd, dc);
		return dpi;
	}

	// Get vertical DPI resolution
	MPT_FORCEINLINE int GetDPIy(HWND hwnd)
	{
		HDC dc = ::GetDC(hwnd);
		int dpi = ::GetDeviceCaps(dc, LOGPIXELSY);
		::ReleaseDC(hwnd, dc);
		return dpi;
	}

	// Applies DPI scaling factor to some given size
	MPT_FORCEINLINE int ScalePixels(int pixels, HWND hwnd)
	{
		return MulDiv(pixels, GetDPIx(hwnd), 96);
	}

	// Removes DPI scaling factor from some given size
	MPT_FORCEINLINE int ScalePixelsInv(int pixels, HWND hwnd)
	{
		return MulDiv(pixels, 96, GetDPIx(hwnd));
	}
}


OPENMPT_NAMESPACE_END
