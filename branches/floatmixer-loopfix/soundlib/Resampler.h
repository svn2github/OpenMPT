/*
 * Resampler.h
 * -----------
 * Purpose: Holds the tables for all available resamplers.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once


#include "WindowedFIR.h"
#include "MixerSettings.h"


#define SINC_WIDTH       8

#define SINC_PHASES_BITS 12
#define SINC_PHASES      (1<<SINC_PHASES_BITS)

#define SINC_TYPE       int16
#define SINC_QUANTSHIFT 15

#define SINC_MASK (SINC_PHASES-1)
STATIC_ASSERT((SINC_MASK & 0xffff) == SINC_MASK); // exceeding fractional freq


//======================
class CResamplerSettings
//======================
{
public:
	ResamplingMode SrcMode;
	double gdWFIRCutoff;
	uint8 gbWFIRType;
public:
	CResamplerSettings()
	{
		SrcMode = SRCMODE_FIRFILTER;
		gdWFIRCutoff = 0.97;
		gbWFIRType = WFIR_KAISER4T;
	}
	bool operator == (const CResamplerSettings &cmp) const
	{
		return SrcMode == cmp.SrcMode && gdWFIRCutoff == cmp.gdWFIRCutoff && gbWFIRType == cmp.gbWFIRType;
	}
	bool operator != (const CResamplerSettings &cmp) const { return !(*this == cmp); }
};


//==============
class CResampler
//==============
{
public:
	CResamplerSettings m_Settings;
	CWindowedFIR m_WindowedFIR;
	static const int16 FastSincTable[256*4];
	SINC_TYPE gKaiserSinc[SINC_PHASES*8];		// Upsampling
#ifdef MODPLUG_TRACKER
	static bool StaticTablesInitialized;
	static SINC_TYPE gDownsample13x[SINC_PHASES*8];	// Downsample 1.333x
	static SINC_TYPE gDownsample2x[SINC_PHASES*8];		// Downsample 2x
#else
	// no global data which has to be initialized by hand in the library
	SINC_TYPE gDownsample13x[SINC_PHASES*8];	// Downsample 1.333x
	SINC_TYPE gDownsample2x[SINC_PHASES*8];		// Downsample 2x
#endif
	float gKaiserSincf[SINC_PHASES*8];				// Upsampling
	static float gDownsample13xf[SINC_PHASES*8];	// Downsample 1.333x
	static float gDownsample2xf[SINC_PHASES*8];		// Downsample 2x
private:
	CResamplerSettings m_OldSettings;
public:
	CResampler() { InitializeTables(true); }
	~CResampler() {}
	void InitializeTables(bool force=false);
	bool IsHQ() const { return m_Settings.SrcMode >= SRCMODE_SPLINE && m_Settings.SrcMode < SRCMODE_DEFAULT; }
};
