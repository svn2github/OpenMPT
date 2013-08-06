/*
 * Tables.cpp
 * ----------
 * Purpose: Effect, interpolation, data and other pre-calculated tables.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Tables.h"
#include <math.h>
#include "Sndfile.h"

#include "Resampler.h"
// rewbs.resamplerConf
#include "WindowedFIR.h"
// end  rewbs.resamplerConf


/////////////////////////////////////////////////////////////////////////////
// Common Tables

const LPCSTR szNoteNames[12] =
{
	"C-", "C#", "D-", "D#", "E-", "F-",
	"F#", "G-", "G#", "A-", "A#", "B-"
};

const LPCTSTR szDefaultNoteNames[NOTE_MAX] = {
	MPT_TEXT("C-0"), MPT_TEXT("C#0"), MPT_TEXT("D-0"), MPT_TEXT("D#0"), MPT_TEXT("E-0"), MPT_TEXT("F-0"), MPT_TEXT("F#0"), MPT_TEXT("G-0"), MPT_TEXT("G#0"), MPT_TEXT("A-0"), MPT_TEXT("A#0"), MPT_TEXT("B-0"),
	MPT_TEXT("C-1"), MPT_TEXT("C#1"), MPT_TEXT("D-1"), MPT_TEXT("D#1"), MPT_TEXT("E-1"), MPT_TEXT("F-1"), MPT_TEXT("F#1"), MPT_TEXT("G-1"), MPT_TEXT("G#1"), MPT_TEXT("A-1"), MPT_TEXT("A#1"), MPT_TEXT("B-1"),
	MPT_TEXT("C-2"), MPT_TEXT("C#2"), MPT_TEXT("D-2"), MPT_TEXT("D#2"), MPT_TEXT("E-2"), MPT_TEXT("F-2"), MPT_TEXT("F#2"), MPT_TEXT("G-2"), MPT_TEXT("G#2"), MPT_TEXT("A-2"), MPT_TEXT("A#2"), MPT_TEXT("B-2"),
	MPT_TEXT("C-3"), MPT_TEXT("C#3"), MPT_TEXT("D-3"), MPT_TEXT("D#3"), MPT_TEXT("E-3"), MPT_TEXT("F-3"), MPT_TEXT("F#3"), MPT_TEXT("G-3"), MPT_TEXT("G#3"), MPT_TEXT("A-3"), MPT_TEXT("A#3"), MPT_TEXT("B-3"),
	MPT_TEXT("C-4"), MPT_TEXT("C#4"), MPT_TEXT("D-4"), MPT_TEXT("D#4"), MPT_TEXT("E-4"), MPT_TEXT("F-4"), MPT_TEXT("F#4"), MPT_TEXT("G-4"), MPT_TEXT("G#4"), MPT_TEXT("A-4"), MPT_TEXT("A#4"), MPT_TEXT("B-4"),
	MPT_TEXT("C-5"), MPT_TEXT("C#5"), MPT_TEXT("D-5"), MPT_TEXT("D#5"), MPT_TEXT("E-5"), MPT_TEXT("F-5"), MPT_TEXT("F#5"), MPT_TEXT("G-5"), MPT_TEXT("G#5"), MPT_TEXT("A-5"), MPT_TEXT("A#5"), MPT_TEXT("B-5"),
	MPT_TEXT("C-6"), MPT_TEXT("C#6"), MPT_TEXT("D-6"), MPT_TEXT("D#6"), MPT_TEXT("E-6"), MPT_TEXT("F-6"), MPT_TEXT("F#6"), MPT_TEXT("G-6"), MPT_TEXT("G#6"), MPT_TEXT("A-6"), MPT_TEXT("A#6"), MPT_TEXT("B-6"),
	MPT_TEXT("C-7"), MPT_TEXT("C#7"), MPT_TEXT("D-7"), MPT_TEXT("D#7"), MPT_TEXT("E-7"), MPT_TEXT("F-7"), MPT_TEXT("F#7"), MPT_TEXT("G-7"), MPT_TEXT("G#7"), MPT_TEXT("A-7"), MPT_TEXT("A#7"), MPT_TEXT("B-7"),
	MPT_TEXT("C-8"), MPT_TEXT("C#8"), MPT_TEXT("D-8"), MPT_TEXT("D#8"), MPT_TEXT("E-8"), MPT_TEXT("F-8"), MPT_TEXT("F#8"), MPT_TEXT("G-8"), MPT_TEXT("G#8"), MPT_TEXT("A-8"), MPT_TEXT("A#8"), MPT_TEXT("B-8"),
	MPT_TEXT("C-9"), MPT_TEXT("C#9"), MPT_TEXT("D-9"), MPT_TEXT("D#9"), MPT_TEXT("E-9"), MPT_TEXT("F-9"), MPT_TEXT("F#9"), MPT_TEXT("G-9"), MPT_TEXT("G#9"), MPT_TEXT("A-9"), MPT_TEXT("A#9"), MPT_TEXT("B-9"),
};


///////////////////////////////////////////////////////////
// File Formats Information (name, extension, etc)

struct ModFormatInfo
{
	MODTYPE format;		// MOD_TYPE_XXXX
	const char *name;			// "ProTracker"
	const char *extension;	// "mod"
};

// remember to also update libopenmpt/libopenmpt_foobar2000.cpp (all other plugins read these dynamically)
static const ModFormatInfo modFormatInfo[] =
{
	{ MOD_TYPE_MOD,		"ProTracker",				"mod" },
	{ MOD_TYPE_S3M,		"ScreamTracker III",		"s3m" },
	{ MOD_TYPE_XM,		"FastTracker II",			"xm" },
	{ MOD_TYPE_IT,		"Impulse Tracker",			"it" },
#ifdef MODPLUG_TRACKER
	{ MOD_TYPE_IT,		"Impulse Tracker Project",	"itp" },
#endif
	{ MOD_TYPE_MPT,		"OpenMPT",					"mptm" },
	{ MOD_TYPE_STM,		"ScreamTracker II",			"stm" },
	{ MOD_TYPE_MOD,		"NoiseTracker",				"nst" },
	{ MOD_TYPE_MOD,		"Soundtracker",				"m15" },
	{ MOD_TYPE_MOD,		"Soundtracker",				"stk" },
	{ MOD_TYPE_MOD,		"Mod's Grave",				"wow" },
	{ MOD_TYPE_ULT,		"UltraTracker",				"ult" },
	{ MOD_TYPE_669,		"Composer 669 / UNIS 669",	"669" },
	{ MOD_TYPE_MTM,		"MultiTracker",				"mtm" },
	{ MOD_TYPE_MED,		"OctaMed",					"med" },
	{ MOD_TYPE_FAR,		"Farandole Composer",		"far" },
	{ MOD_TYPE_MDL,		"DigiTracker",				"mdl" },
	{ MOD_TYPE_AMS,		"Extreme's Tracker",		"ams" },
	{ MOD_TYPE_AMS2,	"Velvet Studio",			"ams" },
	{ MOD_TYPE_DSM,		"DSIK Format",				"dsm" },
	{ MOD_TYPE_AMF,		"DSMI",						"amf" },
	{ MOD_TYPE_AMF0,	"ASYLUM",					"amf" },
	{ MOD_TYPE_OKT,		"Oktalyzer",				"okt" },
	{ MOD_TYPE_DMF,		"X-Tracker",				"dmf" },
	{ MOD_TYPE_PTM,		"PolyTracker",				"ptm" },
	{ MOD_TYPE_PSM,		"Epic Megagames MASI",		"psm" },
	{ MOD_TYPE_MT2,		"MadTracker 2",				"mt2" },
	{ MOD_TYPE_DBM,		"DigiBooster Pro",			"dbm" },
	{ MOD_TYPE_DIGI,	"DigiBooster",				"digi" },
	{ MOD_TYPE_IMF,		"Imago Orpheus",			"imf" },
	{ MOD_TYPE_J2B,		"Galaxy Sound System",		"j2b" },

	// Container formats
	{ MOD_TYPE_GDM,		"General Digital Music",	"gdm" },
	{ MOD_TYPE_UMX,		"Unreal Music",				"umx" },
	{ MOD_TYPE_UMX,		"Unreal Sounds",			"uax" },
#ifndef NO_MO3
	{ MOD_TYPE_MO3,		"MO3",						"mo3" },
#endif // NO_MO3

#ifndef NO_ARCHIVE_SUPPORT
	// Compressed modules
	{ MOD_TYPE_MOD,		"ProTracker",				"mdz" },
	{ MOD_TYPE_MOD,		"ProTracker",				"mdr" },
	{ MOD_TYPE_S3M,		"ScreamTracker III",		"s3z" },
	{ MOD_TYPE_XM,		"FastTracker II",			"xmz" },
	{ MOD_TYPE_IT,		"Impulse Tracker",			"itz" },
	{ MOD_TYPE_MPT,		"OpenMPT",					"mptmz" },
#endif
};

#ifdef MODPLUG_TRACKER
static const ModFormatInfo otherFormatInfo[] =
{
	// Other stuff
	{ MOD_TYPE_WAV,		"Wave",						"wav" },
	{ MOD_TYPE_MID,		"MIDI",						"mid" },
	{ MOD_TYPE_MID,		"MIDI",						"rmi" },
	{ MOD_TYPE_MID,		"MIDI",						"smf" },
};
#endif


std::vector<const char *> CSoundFile::GetSupportedExtensions(bool otherFormats)
//-----------------------------------------------------------------------------
{
	std::vector<const char *> exts;
	for(size_t i = 0; i < CountOf(modFormatInfo); i++)
	{
		// Avoid dupes in list
		if(i == 0 || strcmp(modFormatInfo[i].extension, modFormatInfo[i - 1].extension))
		{
			exts.push_back(modFormatInfo[i].extension);
		}
	}
#ifdef MODPLUG_TRACKER
	if(otherFormats)
	{
		for(size_t i = 0; i < CountOf(otherFormatInfo); i++)
		{
			exts.push_back(otherFormatInfo[i].extension);
		}
	}
#else
	UNREFERENCED_PARAMETER(otherFormats);
#endif
	return exts;
}


const char * CSoundFile::ModTypeToString(MODTYPE modtype)
//-------------------------------------------------------
{
	for(size_t i = 0; i < CountOf(modFormatInfo); i++)
	{
		if(modFormatInfo[i].format & modtype)
		{
			return modFormatInfo[i].extension;
		}
	}
	return "";
}


std::string CSoundFile::ModTypeToTracker(MODTYPE modtype)
//-------------------------------------------------------
{
	std::set<std::string> retvals;
	std::string retval;
	for(size_t i = 0; i < CountOf(modFormatInfo); i++)
	{
		if(modFormatInfo[i].format & modtype)
		{
			std::string name = modFormatInfo[i].name;
			if(retvals.find(name) == retvals.end())
			{
				retvals.insert(name);
				if(!retval.empty())
				{
					retval += " / ";
				}
				retval += name;
			}
		}
	}
	return retval;
}


///////////////////////////////////////////////////////////////////////

const uint8 ImpulseTrackerPortaVolCmd[16] =
{
	0x00, 0x01, 0x04, 0x08, 0x10, 0x20, 0x40, 0x60,
	0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// Period table for Protracker octaves 0-5:
const uint16 ProTrackerPeriodTable[6*12] =
{
	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907,
	856,808,762,720,678,640,604,570,538,508,480,453,
	428,404,381,360,339,320,302,285,269,254,240,226,
	214,202,190,180,170,160,151,143,135,127,120,113,
	107,101,95,90,85,80,75,71,67,63,60,56,
	53,50,47,45,42,40,37,35,33,31,30,28
};


const uint16 ProTrackerTunedPeriods[16*12] = 
{
	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907,
	1700,1604,1514,1430,1348,1274,1202,1134,1070,1010,954,900,
	1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,894,
	1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,888,
	1664,1570,1482,1398,1320,1246,1176,1110,1048,990,934,882,
	1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,874,
	1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,868,
	1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914,862,
	1814,1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,
	1800,1700,1604,1514,1430,1350,1272,1202,1134,1070,1010,954,
	1788,1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,
	1774,1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,
	1762,1664,1570,1482,1398,1320,1246,1176,1110,1048,988,934,
	1750,1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,
	1736,1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,
	1724,1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914 
};

// Table for Invert Loop and Funk Repeat effects (EFx, .MOD only)
const uint8 ModEFxTable[16] =
{
	 0,  5,  6,  7,  8, 10, 11, 13,
	16, 19, 22, 26, 32, 43, 64, 128
}; 

// S3M C-4 periods
const uint16 FreqS3MTable[16] = 
{
	1712,1616,1524,1440,1356,1280,
	1208,1140,1076,1016,960,907,
	0,0,0,0
};

// S3M FineTune frequencies
const uint16 S3MFineTuneTable[16] = 
{
	7895,7941,7985,8046,8107,8169,8232,8280,
	8363,8413,8463,8529,8581,8651,8723,8757,	// 8363*2^((i-8)/(12*8))
};


// Sinus table
const int8 ModSinusTable[64] =
{
	0,12,25,37,49,60,71,81,90,98,106,112,117,122,125,126,
	127,126,125,122,117,112,106,98,90,81,71,60,49,37,25,12,
	0,-12,-25,-37,-49,-60,-71,-81,-90,-98,-106,-112,-117,-122,-125,-126,
	-127,-126,-125,-122,-117,-112,-106,-98,-90,-81,-71,-60,-49,-37,-25,-12
};

// Triangle wave table (ramp down)
const int8 ModRampDownTable[64] =
{
	0,-4,-8,-12,-16,-20,-24,-28,-32,-36,-40,-44,-48,-52,-56,-60,
	-64,-68,-72,-76,-80,-84,-88,-92,-96,-100,-104,-108,-112,-116,-120,-124,
	127,123,119,115,111,107,103,99,95,91,87,83,79,75,71,67,
	63,59,55,51,47,43,39,35,31,27,23,19,15,11,7,3
};

// Square wave table
const int8 ModSquareTable[64] =
{
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
	-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127
};

// Random wave table
const int8 ModRandomTable[64] =
{
	98,-127,-43,88,102,41,-65,-94,125,20,-71,-86,-70,-32,-16,-96,
	17,72,107,-5,116,-69,-62,-40,10,-61,65,109,-18,-38,-13,-76,
	-23,88,21,-94,8,106,21,-112,6,109,20,-88,-30,9,-127,118,
	42,-34,89,-4,-51,-72,21,-29,112,123,84,-101,-92,98,-54,-95
};

// Impulse Tracker tables (ITTECH.TXT)

// Sinus table
const int8 ITSinusTable[256] =
{
	  0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	 24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	 59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	 45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
 	 24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
 	  0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2,
};

// Triangle wave table (ramp down)
const int8 ITRampDownTable[256] =
{
	 64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
	 56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
	 48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
	 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
	 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
	 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
	 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
	  8,  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,
	  0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
	 -8, -9, -9,-10,-10,-11,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,
	-16,-17,-17,-18,-18,-19,-19,-20,-20,-21,-21,-22,-22,-23,-23,-24,
	-24,-25,-25,-26,-26,-27,-27,-28,-28,-29,-29,-30,-30,-31,-31,-32,
	-32,-33,-33,-34,-34,-35,-35,-36,-36,-37,-37,-38,-38,-39,-39,-40,
	-40,-41,-41,-42,-42,-43,-43,-44,-44,-45,-45,-46,-46,-47,-47,-48,
	-48,-49,-49,-50,-50,-51,-51,-52,-52,-53,-53,-54,-54,-55,-55,-56,
	-56,-57,-57,-58,-58,-59,-59,-60,-60,-61,-61,-62,-62,-63,-63,-64,
};

// Square wave table
const int8 ITSquareTable[256] =
{
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

// volume fade tables for Retrig Note:
const int8 retrigTable1[16] =
{ 0, 0, 0, 0, 0, 0, 10, 8, 0, 0, 0, 0, 0, 0, 24, 32 };

const int8 retrigTable2[16] =
{ 0, -1, -2, -4, -8, -16, 0, 0, 0, 1, 2, 4, 8, 16, 0, 0 };




const uint16 XMPeriodTable[104] = 
{
	907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
	808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
	720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
	640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
	570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
	508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
	453,450,447,443,440,437,434,431
};


const uint32 XMLinearTable[768] = 
{
	535232,534749,534266,533784,533303,532822,532341,531861,
	531381,530902,530423,529944,529466,528988,528511,528034,
	527558,527082,526607,526131,525657,525183,524709,524236,
	523763,523290,522818,522346,521875,521404,520934,520464,
	519994,519525,519057,518588,518121,517653,517186,516720,
	516253,515788,515322,514858,514393,513929,513465,513002,
	512539,512077,511615,511154,510692,510232,509771,509312,
	508852,508393,507934,507476,507018,506561,506104,505647,
	505191,504735,504280,503825,503371,502917,502463,502010,
	501557,501104,500652,500201,499749,499298,498848,498398,
	497948,497499,497050,496602,496154,495706,495259,494812,
	494366,493920,493474,493029,492585,492140,491696,491253,
	490809,490367,489924,489482,489041,488600,488159,487718,
	487278,486839,486400,485961,485522,485084,484647,484210,
	483773,483336,482900,482465,482029,481595,481160,480726,
	480292,479859,479426,478994,478562,478130,477699,477268,
	476837,476407,475977,475548,475119,474690,474262,473834,
	473407,472979,472553,472126,471701,471275,470850,470425,
	470001,469577,469153,468730,468307,467884,467462,467041,
	466619,466198,465778,465358,464938,464518,464099,463681,
	463262,462844,462427,462010,461593,461177,460760,460345,
	459930,459515,459100,458686,458272,457859,457446,457033,
	456621,456209,455797,455386,454975,454565,454155,453745,
	453336,452927,452518,452110,451702,451294,450887,450481,
	450074,449668,449262,448857,448452,448048,447644,447240,
	446836,446433,446030,445628,445226,444824,444423,444022,
	443622,443221,442821,442422,442023,441624,441226,440828,
	440430,440033,439636,439239,438843,438447,438051,437656,
	437261,436867,436473,436079,435686,435293,434900,434508,
	434116,433724,433333,432942,432551,432161,431771,431382,
	430992,430604,430215,429827,429439,429052,428665,428278,
	427892,427506,427120,426735,426350,425965,425581,425197,
	424813,424430,424047,423665,423283,422901,422519,422138,
	421757,421377,420997,420617,420237,419858,419479,419101,
	418723,418345,417968,417591,417214,416838,416462,416086,
	415711,415336,414961,414586,414212,413839,413465,413092,
	412720,412347,411975,411604,411232,410862,410491,410121,
	409751,409381,409012,408643,408274,407906,407538,407170,
	406803,406436,406069,405703,405337,404971,404606,404241,
	403876,403512,403148,402784,402421,402058,401695,401333,
	400970,400609,400247,399886,399525,399165,398805,398445,
	398086,397727,397368,397009,396651,396293,395936,395579,
	395222,394865,394509,394153,393798,393442,393087,392733,
	392378,392024,391671,391317,390964,390612,390259,389907,
	389556,389204,388853,388502,388152,387802,387452,387102,
	386753,386404,386056,385707,385359,385012,384664,384317,
	383971,383624,383278,382932,382587,382242,381897,381552,
	381208,380864,380521,380177,379834,379492,379149,378807,
	378466,378124,377783,377442,377102,376762,376422,376082,
	375743,375404,375065,374727,374389,374051,373714,373377,
	373040,372703,372367,372031,371695,371360,371025,370690,
	370356,370022,369688,369355,369021,368688,368356,368023,
	367691,367360,367028,366697,366366,366036,365706,365376,
	365046,364717,364388,364059,363731,363403,363075,362747,
	362420,362093,361766,361440,361114,360788,360463,360137,
	359813,359488,359164,358840,358516,358193,357869,357547,
	357224,356902,356580,356258,355937,355616,355295,354974,
	354654,354334,354014,353695,353376,353057,352739,352420,
	352103,351785,351468,351150,350834,350517,350201,349885,
	349569,349254,348939,348624,348310,347995,347682,347368,
	347055,346741,346429,346116,345804,345492,345180,344869,
	344558,344247,343936,343626,343316,343006,342697,342388,
	342079,341770,341462,341154,340846,340539,340231,339924,
	339618,339311,339005,338700,338394,338089,337784,337479,
	337175,336870,336566,336263,335959,335656,335354,335051,
	334749,334447,334145,333844,333542,333242,332941,332641,
	332341,332041,331741,331442,331143,330844,330546,330247,
	329950,329652,329355,329057,328761,328464,328168,327872,
	327576,327280,326985,326690,326395,326101,325807,325513,
	325219,324926,324633,324340,324047,323755,323463,323171,
	322879,322588,322297,322006,321716,321426,321136,320846,
	320557,320267,319978,319690,319401,319113,318825,318538,
	318250,317963,317676,317390,317103,316817,316532,316246,
	315961,315676,315391,315106,314822,314538,314254,313971,
	313688,313405,313122,312839,312557,312275,311994,311712,
	311431,311150,310869,310589,310309,310029,309749,309470,
	309190,308911,308633,308354,308076,307798,307521,307243,
	306966,306689,306412,306136,305860,305584,305308,305033,
	304758,304483,304208,303934,303659,303385,303112,302838,
	302565,302292,302019,301747,301475,301203,300931,300660,
	300388,300117,299847,299576,299306,299036,298766,298497,
	298227,297958,297689,297421,297153,296884,296617,296349,
	296082,295815,295548,295281,295015,294749,294483,294217,
	293952,293686,293421,293157,292892,292628,292364,292100,
	291837,291574,291311,291048,290785,290523,290261,289999,
	289737,289476,289215,288954,288693,288433,288173,287913,
	287653,287393,287134,286875,286616,286358,286099,285841,
	285583,285326,285068,284811,284554,284298,284041,283785,
	283529,283273,283017,282762,282507,282252,281998,281743,
	281489,281235,280981,280728,280475,280222,279969,279716,
	279464,279212,278960,278708,278457,278206,277955,277704,
	277453,277203,276953,276703,276453,276204,275955,275706,
	275457,275209,274960,274712,274465,274217,273970,273722,
	273476,273229,272982,272736,272490,272244,271999,271753,
	271508,271263,271018,270774,270530,270286,270042,269798,
	269555,269312,269069,268826,268583,268341,268099,267857 
};


const int8 ft2VibratoTable[256] = 
{
	0,-2,-3,-5,-6,-8,-9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,
	-43,-44,-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,
	-56,-57,-58,-59,-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,
	-63,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-63,-63,
	-63,-62,-62,-62,-61,-61,-60,-60,-59,-59,-58,-57,-56,-56,
	-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,
	-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,-24,-23,
	-22,-20,-19,-17,-16,-14,-12,-11,-9,-8,-6,-5,-3,-2,0,
	2,3,5,6,8,9,11,12,14,16,17,19,20,22,23,24,26,27,29,30,
	32,33,34,36,37,38,39,41,42,43,44,45,46,47,48,49,50,51,
	52,53,54,55,56,56,57,58,59,59,60,60,61,61,62,62,62,63,
	63,63,64,64,64,64,64,64,64,64,64,64,64,63,63,63,62,62,
	62,61,61,60,60,59,59,58,57,56,56,55,54,53,52,51,50,49,
	48,47,46,45,44,43,42,41,39,38,37,36,34,33,32,30,29,27,
	26,24,23,22,20,19,17,16,14,12,11,9,8,6,5,3,2 
};



const uint32 FineLinearSlideUpTable[16] =
{
	65536, 65595, 65654, 65714,	65773, 65832, 65892, 65951,
	66011, 66071, 66130, 66190, 66250, 66309, 66369, 66429
};


const uint32 FineLinearSlideDownTable[16] =
{
	65535, 65477, 65418, 65359, 65300, 65241, 65182, 65123,
	65065, 65006, 64947, 64888, 64830, 64772, 64713, 64645
};


const uint32 LinearSlideUpTable[256] = 
{
	65536, 65773, 66010, 66249, 66489, 66729, 66971, 67213, 
	67456, 67700, 67945, 68190, 68437, 68685, 68933, 69182, 
	69432, 69684, 69936, 70189, 70442, 70697, 70953, 71209, 
	71467, 71725, 71985, 72245, 72507, 72769, 73032, 73296, 
	73561, 73827, 74094, 74362, 74631, 74901, 75172, 75444, 
	75717, 75991, 76265, 76541, 76818, 77096, 77375, 77655, 
	77935, 78217, 78500, 78784, 79069, 79355, 79642, 79930, 
	80219, 80509, 80800, 81093, 81386, 81680, 81976, 82272, 
	82570, 82868, 83168, 83469, 83771, 84074, 84378, 84683, 
	84989, 85297, 85605, 85915, 86225, 86537, 86850, 87164, 
	87480, 87796, 88113, 88432, 88752, 89073, 89395, 89718, 
	90043, 90369, 90695, 91023, 91353, 91683, 92015, 92347, 
	92681, 93017, 93353, 93691, 94029, 94370, 94711, 95053, 
	95397, 95742, 96088, 96436, 96785, 97135, 97486, 97839, 
	98193, 98548, 98904, 99262, 99621, 99981, 100343, 100706, 
	101070, 101435, 101802, 102170, 102540, 102911, 103283, 103657, 
	104031, 104408, 104785, 105164, 105545, 105926, 106309, 106694, 
	107080, 107467, 107856, 108246, 108637, 109030, 109425, 109820, 
	110217, 110616, 111016, 111418, 111821, 112225, 112631, 113038, 
	113447, 113857, 114269, 114682, 115097, 115514, 115931, 116351, 
	116771, 117194, 117618, 118043, 118470, 118898, 119328, 119760, 
	120193, 120628, 121064, 121502, 121941, 122382, 122825, 123269, 
	123715, 124162, 124611, 125062, 125514, 125968, 126424, 126881, 
	127340, 127801, 128263, 128727, 129192, 129660, 130129, 130599, 
	131072, 131546, 132021, 132499, 132978, 133459, 133942, 134426, 
	134912, 135400, 135890, 136381, 136875, 137370, 137866, 138365, 
	138865, 139368, 139872, 140378, 140885, 141395, 141906, 142419, 
	142935, 143451, 143970, 144491, 145014, 145538, 146064, 146593, 
	147123, 147655, 148189, 148725, 149263, 149803, 150344, 150888, 
	151434, 151982, 152531, 153083, 153637, 154192, 154750, 155310, 
	155871, 156435, 157001, 157569, 158138, 158710, 159284, 159860, 
	160439, 161019, 161601, 162186, 162772, 163361, 163952, 164545, 
};



const uint32 LinearSlideDownTable[256] = 
{
	65536, 65299, 65064, 64830, 64596, 64363, 64131, 63900, 
	63670, 63440, 63212, 62984, 62757, 62531, 62305, 62081, 
	61857, 61634, 61412, 61191, 60970, 60751, 60532, 60314, 
	60096, 59880, 59664, 59449, 59235, 59021, 58809, 58597, 
	58385, 58175, 57965, 57757, 57548, 57341, 57134, 56928, 
	56723, 56519, 56315, 56112, 55910, 55709, 55508, 55308, 
	55108, 54910, 54712, 54515, 54318, 54123, 53928, 53733, 
	53540, 53347, 53154, 52963, 52772, 52582, 52392, 52204, 
	52015, 51828, 51641, 51455, 51270, 51085, 50901, 50717, 
	50535, 50353, 50171, 49990, 49810, 49631, 49452, 49274, 
	49096, 48919, 48743, 48567, 48392, 48218, 48044, 47871, 
	47698, 47526, 47355, 47185, 47014, 46845, 46676, 46508, 
	46340, 46173, 46007, 45841, 45676, 45511, 45347, 45184, 
	45021, 44859, 44697, 44536, 44376, 44216, 44056, 43898, 
	43740, 43582, 43425, 43268, 43112, 42957, 42802, 42648, 
	42494, 42341, 42189, 42037, 41885, 41734, 41584, 41434, 
	41285, 41136, 40988, 40840, 40693, 40546, 40400, 40254, 
	40109, 39965, 39821, 39677, 39534, 39392, 39250, 39108, 
	38967, 38827, 38687, 38548, 38409, 38270, 38132, 37995, 
	37858, 37722, 37586, 37450, 37315, 37181, 37047, 36913, 
	36780, 36648, 36516, 36384, 36253, 36122, 35992, 35862, 
	35733, 35604, 35476, 35348, 35221, 35094, 34968, 34842, 
	34716, 34591, 34466, 34342, 34218, 34095, 33972, 33850, 
	33728, 33606, 33485, 33364, 33244, 33124, 33005, 32886, 
	32768, 32649, 32532, 32415, 32298, 32181, 32065, 31950, 
	31835, 31720, 31606, 31492, 31378, 31265, 31152, 31040, 
	30928, 30817, 30706, 30595, 30485, 30375, 30266, 30157, 
	30048, 29940, 29832, 29724, 29617, 29510, 29404, 29298, 
	29192, 29087, 28982, 28878, 28774, 28670, 28567, 28464, 
	28361, 28259, 28157, 28056, 27955, 27854, 27754, 27654, 
	27554, 27455, 27356, 27257, 27159, 27061, 26964, 26866, 
	26770, 26673, 26577, 26481, 26386, 26291, 26196, 26102, 
};


// LUT for 2 * damping factor
const float ITResonanceTable[128] = 
{
	1.0000000000000000f, 0.9786446094512940f, 0.9577452540397644f, 0.9372922182083130f,
	0.9172759056091309f, 0.8976871371269226f, 0.8785166740417481f, 0.8597555756568909f,
	0.8413951396942139f, 0.8234267830848694f, 0.8058421611785889f, 0.7886331081390381f,
	0.7717915177345276f, 0.7553095817565918f, 0.7391796708106995f, 0.7233941555023193f,
	0.7079457640647888f, 0.6928272843360901f, 0.6780316829681397f, 0.6635520458221436f,
	0.6493816375732422f, 0.6355138421058655f, 0.6219421625137329f, 0.6086603403091431f,
	0.5956621170043945f, 0.5829415321350098f, 0.5704925656318665f, 0.5583094954490662f,
	0.5463865399360657f, 0.5347182154655457f, 0.5232990980148315f, 0.5121238231658936f,
	0.5011872053146362f, 0.4904841780662537f, 0.4800096750259399f, 0.4697588682174683f,
	0.4597269892692566f, 0.4499093294143677f, 0.4403013288974762f, 0.4308985173702240f,
	0.4216965138912201f, 0.4126909971237183f, 0.4038778245449066f, 0.3952528536319733f,
	0.3868120610713959f, 0.3785515129566193f, 0.3704673945903778f, 0.3625559210777283f,
	0.3548133969306946f, 0.3472362160682678f, 0.3398208320140839f, 0.3325638175010681f,
	0.3254617750644684f, 0.3185114264488220f, 0.3117094635963440f, 0.3050527870655060f,
	0.2985382676124573f, 0.2921628654003143f, 0.2859236001968384f, 0.2798175811767578f,
	0.2738419771194458f, 0.2679939568042755f, 0.2622708380222321f, 0.2566699385643005f,
	0.2511886358261108f, 0.2458244115114212f, 0.2405747324228287f, 0.2354371547698975f,
	0.2304092943668366f, 0.2254888117313385f, 0.2206734120845795f, 0.2159608304500580f,
	0.2113489061594009f, 0.2068354636430740f, 0.2024184018373489f, 0.1980956792831421f,
	0.1938652694225311f, 0.1897251904010773f, 0.1856735348701477f, 0.1817083954811096f,
	0.1778279393911362f, 0.1740303486585617f, 0.1703138649463654f, 0.1666767448186874f,
	0.1631172895431519f, 0.1596338599920273f, 0.1562248021364212f, 0.1528885662555695f,
	0.1496235728263855f, 0.1464282870292664f, 0.1433012634515762f, 0.1402409970760346f,
	0.1372461020946503f, 0.1343151479959488f, 0.1314467936754227f, 0.1286396980285645f,
	0.1258925348520279f, 0.1232040524482727f, 0.1205729842185974f, 0.1179980933666229f,
	0.1154781952500343f, 0.1130121126770973f, 0.1105986908078194f, 0.1082368120551109f,
	0.1059253737330437f, 0.1036632955074310f, 0.1014495193958283f, 0.0992830246686935f,
	0.0971627980470657f, 0.0950878411531448f, 0.0930572077631950f, 0.0910699293017387f,
	0.0891250967979431f, 0.0872217938303947f, 0.0853591337800026f, 0.0835362523794174f,
	0.0817523002624512f, 0.0800064504146576f, 0.0782978758215904f, 0.0766257941722870f,
	0.0749894231557846f, 0.0733879879117012f, 0.0718207582831383f, 0.0702869966626167f,
	0.0687859877943993f, 0.0673170387744904f, 0.0658794566988945f, 0.0644725710153580f,
};


// Reversed sinc coefficients

const int16 CResampler::FastSincTable[256*4] =
{ // Cubic Spline
    0, 16384,     0,     0,   -31, 16383,    32,     0,   -63, 16381,    65,     0,   -93, 16378,   100,    -1, 
 -124, 16374,   135,    -1,  -153, 16368,   172,    -3,  -183, 16361,   209,    -4,  -211, 16353,   247,    -5, 
 -240, 16344,   287,    -7,  -268, 16334,   327,    -9,  -295, 16322,   368,   -12,  -322, 16310,   410,   -14, 
 -348, 16296,   453,   -17,  -374, 16281,   497,   -20,  -400, 16265,   541,   -23,  -425, 16248,   587,   -26, 
 -450, 16230,   634,   -30,  -474, 16210,   681,   -33,  -497, 16190,   729,   -37,  -521, 16168,   778,   -41, 
 -543, 16145,   828,   -46,  -566, 16121,   878,   -50,  -588, 16097,   930,   -55,  -609, 16071,   982,   -60, 
 -630, 16044,  1035,   -65,  -651, 16016,  1089,   -70,  -671, 15987,  1144,   -75,  -691, 15957,  1199,   -81, 
 -710, 15926,  1255,   -87,  -729, 15894,  1312,   -93,  -748, 15861,  1370,   -99,  -766, 15827,  1428,  -105, 
 -784, 15792,  1488,  -112,  -801, 15756,  1547,  -118,  -818, 15719,  1608,  -125,  -834, 15681,  1669,  -132, 
 -850, 15642,  1731,  -139,  -866, 15602,  1794,  -146,  -881, 15561,  1857,  -153,  -896, 15520,  1921,  -161, 
 -911, 15477,  1986,  -168,  -925, 15434,  2051,  -176,  -939, 15390,  2117,  -184,  -952, 15344,  2184,  -192, 
 -965, 15298,  2251,  -200,  -978, 15251,  2319,  -208,  -990, 15204,  2387,  -216, -1002, 15155,  2456,  -225, 
-1014, 15106,  2526,  -234, -1025, 15055,  2596,  -242, -1036, 15004,  2666,  -251, -1046, 14952,  2738,  -260, 
-1056, 14899,  2810,  -269, -1066, 14846,  2882,  -278, -1075, 14792,  2955,  -287, -1084, 14737,  3028,  -296, 
-1093, 14681,  3102,  -306, -1102, 14624,  3177,  -315, -1110, 14567,  3252,  -325, -1118, 14509,  3327,  -334, 
-1125, 14450,  3403,  -344, -1132, 14390,  3480,  -354, -1139, 14330,  3556,  -364, -1145, 14269,  3634,  -374, 
-1152, 14208,  3712,  -384, -1157, 14145,  3790,  -394, -1163, 14082,  3868,  -404, -1168, 14018,  3947,  -414, 
-1173, 13954,  4027,  -424, -1178, 13889,  4107,  -434, -1182, 13823,  4187,  -445, -1186, 13757,  4268,  -455, 
-1190, 13690,  4349,  -465, -1193, 13623,  4430,  -476, -1196, 13555,  4512,  -486, -1199, 13486,  4594,  -497, 
-1202, 13417,  4676,  -507, -1204, 13347,  4759,  -518, -1206, 13276,  4842,  -528, -1208, 13205,  4926,  -539, 
-1210, 13134,  5010,  -550, -1211, 13061,  5094,  -560, -1212, 12989,  5178,  -571, -1212, 12915,  5262,  -581, 
-1213, 12842,  5347,  -592, -1213, 12767,  5432,  -603, -1213, 12693,  5518,  -613, -1213, 12617,  5603,  -624, 
-1212, 12542,  5689,  -635, -1211, 12466,  5775,  -645, -1210, 12389,  5862,  -656, -1209, 12312,  5948,  -667, 
-1208, 12234,  6035,  -677, -1206, 12156,  6122,  -688, -1204, 12078,  6209,  -698, -1202, 11999,  6296,  -709, 
-1200, 11920,  6384,  -720, -1197, 11840,  6471,  -730, -1194, 11760,  6559,  -740, -1191, 11679,  6647,  -751, 
-1188, 11598,  6735,  -761, -1184, 11517,  6823,  -772, -1181, 11436,  6911,  -782, -1177, 11354,  6999,  -792, 
-1173, 11271,  7088,  -802, -1168, 11189,  7176,  -812, -1164, 11106,  7265,  -822, -1159, 11022,  7354,  -832, 
-1155, 10939,  7442,  -842, -1150, 10855,  7531,  -852, -1144, 10771,  7620,  -862, -1139, 10686,  7709,  -872, 
-1134, 10602,  7798,  -882, -1128, 10516,  7886,  -891, -1122, 10431,  7975,  -901, -1116, 10346,  8064,  -910, 
-1110, 10260,  8153,  -919, -1103, 10174,  8242,  -929, -1097, 10088,  8331,  -938, -1090, 10001,  8420,  -947, 
-1083,  9915,  8508,  -956, -1076,  9828,  8597,  -965, -1069,  9741,  8686,  -973, -1062,  9654,  8774,  -982, 
-1054,  9566,  8863,  -991, -1047,  9479,  8951,  -999, -1039,  9391,  9039, -1007, -1031,  9303,  9127, -1015, 
-1024,  9216,  9216, -1024, -1015,  9127,  9303, -1031, -1007,  9039,  9391, -1039,  -999,  8951,  9479, -1047, 
 -991,  8863,  9566, -1054,  -982,  8774,  9654, -1062,  -973,  8686,  9741, -1069,  -965,  8597,  9828, -1076, 
 -956,  8508,  9915, -1083,  -947,  8420, 10001, -1090,  -938,  8331, 10088, -1097,  -929,  8242, 10174, -1103, 
 -919,  8153, 10260, -1110,  -910,  8064, 10346, -1116,  -901,  7975, 10431, -1122,  -891,  7886, 10516, -1128, 
 -882,  7798, 10602, -1134,  -872,  7709, 10686, -1139,  -862,  7620, 10771, -1144,  -852,  7531, 10855, -1150, 
 -842,  7442, 10939, -1155,  -832,  7354, 11022, -1159,  -822,  7265, 11106, -1164,  -812,  7176, 11189, -1168, 
 -802,  7088, 11271, -1173,  -792,  6999, 11354, -1177,  -782,  6911, 11436, -1181,  -772,  6823, 11517, -1184, 
 -761,  6735, 11598, -1188,  -751,  6647, 11679, -1191,  -740,  6559, 11760, -1194,  -730,  6471, 11840, -1197, 
 -720,  6384, 11920, -1200,  -709,  6296, 11999, -1202,  -698,  6209, 12078, -1204,  -688,  6122, 12156, -1206, 
 -677,  6035, 12234, -1208,  -667,  5948, 12312, -1209,  -656,  5862, 12389, -1210,  -645,  5775, 12466, -1211, 
 -635,  5689, 12542, -1212,  -624,  5603, 12617, -1213,  -613,  5518, 12693, -1213,  -603,  5432, 12767, -1213, 
 -592,  5347, 12842, -1213,  -581,  5262, 12915, -1212,  -571,  5178, 12989, -1212,  -560,  5094, 13061, -1211, 
 -550,  5010, 13134, -1210,  -539,  4926, 13205, -1208,  -528,  4842, 13276, -1206,  -518,  4759, 13347, -1204, 
 -507,  4676, 13417, -1202,  -497,  4594, 13486, -1199,  -486,  4512, 13555, -1196,  -476,  4430, 13623, -1193, 
 -465,  4349, 13690, -1190,  -455,  4268, 13757, -1186,  -445,  4187, 13823, -1182,  -434,  4107, 13889, -1178, 
 -424,  4027, 13954, -1173,  -414,  3947, 14018, -1168,  -404,  3868, 14082, -1163,  -394,  3790, 14145, -1157, 
 -384,  3712, 14208, -1152,  -374,  3634, 14269, -1145,  -364,  3556, 14330, -1139,  -354,  3480, 14390, -1132, 
 -344,  3403, 14450, -1125,  -334,  3327, 14509, -1118,  -325,  3252, 14567, -1110,  -315,  3177, 14624, -1102, 
 -306,  3102, 14681, -1093,  -296,  3028, 14737, -1084,  -287,  2955, 14792, -1075,  -278,  2882, 14846, -1066, 
 -269,  2810, 14899, -1056,  -260,  2738, 14952, -1046,  -251,  2666, 15004, -1036,  -242,  2596, 15055, -1025, 
 -234,  2526, 15106, -1014,  -225,  2456, 15155, -1002,  -216,  2387, 15204,  -990,  -208,  2319, 15251,  -978, 
 -200,  2251, 15298,  -965,  -192,  2184, 15344,  -952,  -184,  2117, 15390,  -939,  -176,  2051, 15434,  -925, 
 -168,  1986, 15477,  -911,  -161,  1921, 15520,  -896,  -153,  1857, 15561,  -881,  -146,  1794, 15602,  -866, 
 -139,  1731, 15642,  -850,  -132,  1669, 15681,  -834,  -125,  1608, 15719,  -818,  -118,  1547, 15756,  -801, 
 -112,  1488, 15792,  -784,  -105,  1428, 15827,  -766,   -99,  1370, 15861,  -748,   -93,  1312, 15894,  -729, 
  -87,  1255, 15926,  -710,   -81,  1199, 15957,  -691,   -75,  1144, 15987,  -671,   -70,  1089, 16016,  -651, 
  -65,  1035, 16044,  -630,   -60,   982, 16071,  -609,   -55,   930, 16097,  -588,   -50,   878, 16121,  -566, 
  -46,   828, 16145,  -543,   -41,   778, 16168,  -521,   -37,   729, 16190,  -497,   -33,   681, 16210,  -474, 
  -30,   634, 16230,  -450,   -26,   587, 16248,  -425,   -23,   541, 16265,  -400,   -20,   497, 16281,  -374, 
  -17,   453, 16296,  -348,   -14,   410, 16310,  -322,   -12,   368, 16322,  -295,    -9,   327, 16334,  -268, 
   -7,   287, 16344,  -240,    -5,   247, 16353,  -211,    -4,   209, 16361,  -183,    -3,   172, 16368,  -153, 
   -1,   135, 16374,  -124,    -1,   100, 16378,   -93,     0,    65, 16381,   -63,     0,    32, 16383,   -31, 
};





/////////////////////////////////////////////////////////////////////////////////////////////


// Compute Bessel function Izero(y) using a series approximation
static double izero(double y)
{
    double s=1, ds=1, d=0;
	do
	{
        d = d + 2; ds = ds * (y*y)/(d*d);
        s = s + ds;
    } while (ds > 1E-7 * s);
	return s;
}

static void getsinc(SINC_TYPE *psinc, double beta, double lowpass_factor)
{
	const double izero_beta = izero(beta);
	const double kPi = 4.0*atan(1.0)*lowpass_factor;
	for (int isrc=0; isrc<8*SINC_PHASES; isrc++)
	{
		double fsinc;
		int ix = 7 - (isrc & 7);
		ix = (ix*SINC_PHASES)+(isrc>>3);
		if (ix == (4*SINC_PHASES))
		{
			fsinc = 1.0;
		} else
		{
			double x = (double)(ix - (4*SINC_PHASES)) * (double)(1.0/SINC_PHASES);
			fsinc = sin(x*kPi) * izero(beta*sqrt(1-x*x*(1.0/16.0))) / (izero_beta*x*kPi); // Kaiser window
		}
		double coeff = fsinc * lowpass_factor;
		int n = (int)std::floor(coeff * (1<<SINC_QUANTSHIFT) + 0.5);
		ASSERT(n <= int16_max);
		ASSERT(n > int16_min);
		*psinc++ = static_cast<SINC_TYPE>(n);
	}
}


#if 0

// this code is currently unused

static double GetSpline(double x, double c0, double c1, double c2, double c3)
{
	double Xo = c1;
	double Xa = c0 - Xo;
	double Xb = c2 - Xo;
	double Ux = (Xb-Xa)/2;
	double Vx = (c3 - Xo)/2;
	double a = Vx+Ux-2*Xb;
	double b = 3*Xb-2*Ux-Vx;
	return (((a*x+b)*x)+Ux)*x+Xo;
}


static void getdownsample2x(short int *psinc)
{
	for (int i=0; i<SINC_PHASES; i++)
	{
		double x = (double)i * (double)(0.5/SINC_PHASES);
		psinc[i*8+7] = (short int)(GetSpline(x,     0, 0, 0, 1) * 8192);
		psinc[i*8+6] = (short int)(GetSpline(x+0.5, 0, 0, 0, 1) * 8192);
		psinc[i*8+5] = (short int)(GetSpline(x,     0, 0, 1, 0) * 8192);
		psinc[i*8+4] = (short int)(GetSpline(x+0.5, 0, 0, 1, 0) * 8192);
		psinc[i*8+3] = (short int)(GetSpline(x,     0, 1, 0, 0) * 8192);
		psinc[i*8+2] = (short int)(GetSpline(x+0.5, 0, 1, 0, 0) * 8192);
		psinc[i*8+1] = (short int)(GetSpline(x,     1, 0, 0, 0) * 8192);
		psinc[i*8+0] = (short int)(GetSpline(x+0.5, 1, 0, 0, 0) * 8192);
	}
}

#endif


#ifdef MODPLUG_TRACKER
bool CResampler::StaticTablesInitialized = false;
SINC_TYPE CResampler::gDownsample13x[SINC_PHASES*8];	// Downsample 1.333x
SINC_TYPE CResampler::gDownsample2x[SINC_PHASES*8];		// Downsample 2x
#endif


void CResampler::InitializeTables(bool force)
{
	#ifdef MODPLUG_TRACKER
		if(!StaticTablesInitialized)
		{
			//ericus' downsampling improvement.
			//getsinc(gDownsample13x, 8.5, 3.0/4.0);
			//getdownsample2x(gDownsample2x);
			getsinc(gDownsample13x, 8.5, 0.5);	   
			getsinc(gDownsample2x, 2.7625, 0.425); 
			//end ericus' downsampling improvement.
			StaticTablesInitialized = true;
		}
	#endif
	if((m_OldSettings == m_Settings) && !force) return;
	m_WindowedFIR.InitTable(m_Settings.gdWFIRCutoff, m_Settings.gbWFIRType);
	getsinc(gKaiserSinc, 9.6377, m_Settings.gdWFIRCutoff);
	#ifndef MODPLUG_TRACKER
		getsinc(gDownsample13x, 8.5, 0.5);
		getsinc(gDownsample2x, 2.7625, 0.425);
	#endif
	m_OldSettings = m_Settings;
}

