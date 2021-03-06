/* Modplug XMMS Plugin
 * Copyright (C) 1999 Kenton Varda and Olivier Lapicque
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __MODPLUGXMMS_MODPROPS_H_INCLUDED__
#define __MODPLUGXMMS_MODPROPS_H_INCLUDED__

#ifndef __MODPLUGXMMS_STDDEFS_H__INCLUDED__
#include"stddefs.h"
#endif

struct ModProperties
{
	bool    mSurround;
	bool    mOversamp;
	bool    mMegabass;
	bool    mNoiseReduction;
	bool    mVolumeRamp;
	bool    mReverb;
	bool    mFadeout;
	bool    mFastinfo;

	uint8   mChannels;
	uint8   mBits;
	uint32  mFrequency;

	uint32  mReverbDepth;
	uint32  mReverbDelay;
	uint32  mBassAmount;
	uint32  mBassRange;
	uint32  mSurroundDepth;
	uint32  mSurroundDelay;
	uint32  mFadeTime;
};

#endif //included