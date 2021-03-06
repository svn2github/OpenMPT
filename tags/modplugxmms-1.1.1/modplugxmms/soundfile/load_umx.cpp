/*
 * This program is  free software; you can redistribute it  and modify it
 * under the terms of the GNU  General Public License as published by the
 * Free Software Foundation; either version 2  of the license or (at your
 * option) any later version.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "stdafx.h"
#include "sndfile.h"


BOOL CSoundFile::ReadUMX(const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	if ((!lpStream) || (dwMemLength < 0x800)) return FALSE;
	// Rip Mods from UMX
	if ((*((DWORD *)(lpStream+0x20)) < dwMemLength)
	 && (*((DWORD *)(lpStream+0x18)) <= dwMemLength - 0x10)
	 && (*((DWORD *)(lpStream+0x18)) >= dwMemLength - 0x200))
	{
		for (UINT uscan=0x40; uscan<0x300; uscan++)
		{
			DWORD dwScan = *((DWORD *)(lpStream+uscan));
			// IT
			if (dwScan == 0x4D504D49)
			{
				DWORD dwRipOfs = uscan;
				return ReadIT(lpStream + dwRipOfs, dwMemLength - dwRipOfs);
			}
			// S3M
			if (dwScan == 0x4D524353)
			{
				DWORD dwRipOfs = uscan - 44;
				return ReadS3M(lpStream + dwRipOfs, dwMemLength - dwRipOfs);
			}
			// XM
			if (!strnicmp((LPCSTR)(lpStream+uscan), "Extended Module", 15))
			{
				DWORD dwRipOfs = uscan;
				return ReadXM(lpStream + dwRipOfs, dwMemLength - dwRipOfs);
			}
		}
	}
	return FALSE;
}


