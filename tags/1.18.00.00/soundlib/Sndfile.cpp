/*
 * OpenMPT
 *
 * Sndfile.cpp
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
 *          OpenMPT devs
*/

#include "stdafx.h"
#include "../mptrack/mptrack.h"
#include "../mptrack/mainfrm.h"
#include "../mptrack/moddoc.h"
#include "../mptrack/version.h"
#include "../mptrack/serialization_utils.h"
#include "sndfile.h"
#include "wavConverter.h"
#include "tuningcollection.h"
#include <vector>
#include <algorithm>

#define str_SampleAllocationError	(GetStrI18N(_TEXT("Sample allocation error")))
#define str_Error					(GetStrI18N(_TEXT("Error")))

#ifndef NO_COPYRIGHT
#ifndef NO_MMCMP_SUPPORT
#define MMCMP_SUPPORT
#endif // NO_MMCMP_SUPPORT
#ifndef NO_ARCHIVE_SUPPORT
#define UNRAR_SUPPORT
#define UNLHA_SUPPORT
#define ZIPPED_MOD_SUPPORT
LPCSTR glpszModExtensions = "mod|s3m|xm|it|stm|nst|ult|669|wow|mtm|med|far|mdl|ams|dsm|amf|okt|dmf|ptm|psm|mt2|umx|gdm|imf|j2b"
#ifndef NO_UNMO3_SUPPORT
"|mo3"
#endif
;
//Should there be mptm?
#endif // NO_ARCHIVE_SUPPORT
#else // NO_COPYRIGHT: EarSaver only loads mod/s3m/xm/it/wav
#define MODPLUG_BASIC_SUPPORT
#endif

#ifdef ZIPPED_MOD_SUPPORT
#include "unzip32.h"
#endif

#ifdef UNRAR_SUPPORT
#include "unrar32.h"
#endif

#ifdef UNLHA_SUPPORT
#include "../unlha/unlha32.h"
#endif

#ifdef MMCMP_SUPPORT
extern BOOL MMCMP_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength);
#endif

// External decompressors
extern void AMSUnpack(const char *psrc, UINT inputlen, char *pdest, UINT dmax, char packcharacter);
extern WORD MDLReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n);
extern int DMFUnpack(LPBYTE psample, LPBYTE ibuf, LPBYTE ibufmax, UINT maxlen);
extern DWORD ITReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n);
extern void ITUnpack8Bit(LPSTR pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, BOOL b215);
extern void ITUnpack16Bit(LPSTR pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, BOOL b215);


#define MAX_PACK_TABLES		3


// Compression table
static char UnpackTable[MAX_PACK_TABLES][16] = 
//--------------------------------------------
{
	// CPU-generated dynamic table
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	// u-Law table
	0, 1, 2, 4, 8, 16, 32, 64,
	-1, -2, -4, -8, -16, -32, -48, -64,
	// Linear table
	0, 1, 2, 3, 5, 7, 12, 19,
	-1, -2, -3, -5, -7, -12, -19, -31,
};

// -> CODE#0027
// -> DESC="per-instrument volume ramping setup (refered as attack)"

/*---------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
MODULAR (in/out) MODINSTRUMENT :
-----------------------------------------------------------------------------------------------

* to update:
------------

- both following functions need to be updated when adding a new member in MODINSTRUMENT :

void WriteInstrumentHeaderStruct(MODINSTRUMENT * input, FILE * file);
BYTE * GetInstrumentHeaderFieldPointer(MODINSTRUMENT * input, __int32 fcode, __int16 fsize);

- see below for body declaration.


* members:
----------

- 32bit identification CODE tag (must be unique)
- 16bit content SIZE in byte(s)
- member field


* CODE tag naming convention:
-----------------------------

- have a look below in current tag dictionnary
- take the initial ones of the field name
- 4 caracters code (not more, not less)
- must be filled with '.' caracters if code has less than 4 caracters
- for arrays, must include a '[' caracter following significant caracters ('.' not significant!!!)
- use only caracters used in full member name, ordered as they appear in it
- match caracter attribute (small,capital)

Example with "PanEnv.nLoopEnd" , "PitchEnv.nLoopEnd" & "VolEnv.Values[MAX_ENVPOINTS]" members : 
- use 'PLE.' for PanEnv.nLoopEnd
- use 'PiLE' for PitchEnv.nLoopEnd
- use 'VE[.' for VolEnv.Values[MAX_ENVPOINTS]


* In use CODE tag dictionary (alphabetical order): [ see in Sndfile.cpp ]
-------------------------------------------------------------------------

						!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						!!! SECTION TO BE UPDATED !!!
						!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		[EXT]	means external (not related) to MODINSTRUMENT content

C...	[EXT]	nChannels
ChnS	[EXT]	IT/MPTM: Channel settings for channels 65-127 if needed (doesn't fit to IT header).
CS..			nCutSwing
CWV.	[EXT]	dwCreatedWithVersion
DCT.			nDCT;
dF..			dwFlags;
DGV.	[EXT]	nDefaultGlobalVolume
DT..	[EXT]	nDefaultTempo;			
DNA.			nDNA;
EBIH	[EXT]	embeded instrument header tag (ITP file format)
FM..			nFilterMode;
fn[.			filename[12];
FO..			nFadeOut;
GV..			nGlobalVol;
IFC.			nIFC;
IFR.			nIFR;
K[.				Keyboard[128];
LSWV	[EXT]	nPlugMixMode
MB..			wMidiBank;
MC..			nMidiChannel;
MDK.			nMidiDrumKey;
MIMA	[EXT]									MIdi MApping directives
MiP.			nMixPlug;
MP..			nMidiProgram;
MPTS	[EXT]									Extra song info tag
MPTX	[EXT]									EXTRA INFO tag
MSF.	[EXT]									Mod(Specific)Flags
n[..			name[32];
NNA.			nNNA;
NM[.			NoteMap[128];
P...			nPan;
PE..			PanEnv.nNodes;
PE[.			PanEnv.Values[MAX_ENVPOINTS];
PiE.			PitchEnv.nNodes;
PiE[			PitchEnv.Values[MAX_ENVPOINTS];
PiLE			PitchEnv.nLoopEnd;
PiLS			PitchEnv.nLoopStart;
PiP[			PitchEnv.Ticks[MAX_ENVPOINTS];
PiSB			PitchEnv.nSustainStart;
PiSE			PitchEnv.nSustainEnd;
PLE.			PanEnv.nLoopEnd;
PLS.			PanEnv.nLoopStart;
PMM.	[EXT]	nPlugMixMode;
PP[.			PanEnv.Ticks[MAX_ENVPOINTS];
PPC.			nPPC;
PPS.			nPPS;
PS..			nPanSwing;
PSB.			PanEnv.nSustainStart;
PSE.			PanEnv.nSustainEnd;
PTTL			wPitchToTempoLock;
PVEH			nPluginVelocityHandling;
PVOH			nPluginVolumeHandling;
R...			nResampling;
RP..	[EXT]	nRestartPos;
RPB.	[EXT]	nRowsPerBeat;
RPM.	[EXT]	nRowsPerMeasure;
RS..			nResSwing;
SEP@	[EXT]									chunk SEPARATOR tag
SPA.	[EXT]	m_nSamplePreAmp;
TM..	[EXT]	nTempoMode;
VE..			VolEnv.nNodes;
VE[.			VolEnv.Values[MAX_ENVPOINTS];
VLE.			VolEnv.nLoopEnd;
VLS.			VolEnv.nLoopStart;
VP[.			VolEnv.Ticks[MAX_ENVPOINTS];
VR..			nVolRamp;
VS..			nVolSwing;
VSB.			VolEnv.nSustainStart;
VSE.			VolEnv.nSustainEnd;
VSTV	[EXT]	nVSTiVolume;
PERN			PitchEnv.nReleaseNode
AERN			PanEnv.nReleaseNode
VERN			VolEnv.nReleaseNode
PFLG			PitchEnv.dwFlag
AFLG			PanEnv.dwFlags
VFLG			VolEnv.dwFlags
-----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------*/

// --------------------------------------------------------------------------------------------
// Convenient macro to help WRITE_HEADER declaration for single type members ONLY (non-array)
// --------------------------------------------------------------------------------------------
#define WRITE_MPTHEADER_sized_member(name,type,code) \
fcode = #@code;\
fwrite(& fcode , 1 , sizeof( __int32 ) , file);\
fsize = sizeof( type );\
fwrite(& fsize , 1 , sizeof( __int16 ) , file);\
fwrite(&input-> name , 1 , fsize , file);

// --------------------------------------------------------------------------------------------
// Convenient macro to help WRITE_HEADER declaration for array members ONLY
// --------------------------------------------------------------------------------------------
#define WRITE_MPTHEADER_array_member(name,type,code,arraysize) \
fcode = #@code;\
fwrite(& fcode , 1 , sizeof( __int32 ) , file);\
fsize = sizeof( type ) * arraysize;\
fwrite(& fsize , 1 , sizeof( __int16 ) , file);\
fwrite(&input-> name , 1 , fsize , file);

// Write (in 'file') 'input' MODINSTRUMENT with 'code' & 'size' extra field infos for each member
void WriteInstrumentHeaderStruct(MODINSTRUMENT * input, FILE * file)
{
__int32 fcode;
__int16 fsize;
WRITE_MPTHEADER_sized_member(	nFadeOut				, UINT			, FO..							)
WRITE_MPTHEADER_sized_member(	dwFlags					, DWORD			, dF..							)
WRITE_MPTHEADER_sized_member(	nGlobalVol				, UINT			, GV..							)
WRITE_MPTHEADER_sized_member(	nPan					, UINT			, P...							)
WRITE_MPTHEADER_sized_member(	VolEnv.nNodes			, UINT			, VE..							)
WRITE_MPTHEADER_sized_member(	PanEnv.nNodes			, UINT			, PE..							)
WRITE_MPTHEADER_sized_member(	PitchEnv.nNodes			, UINT			, PiE.							)
WRITE_MPTHEADER_sized_member(	VolEnv.nLoopStart		, BYTE			, VLS.							)
WRITE_MPTHEADER_sized_member(	VolEnv.nLoopEnd			, BYTE			, VLE.							)
WRITE_MPTHEADER_sized_member(	VolEnv.nSustainStart	, BYTE			, VSB.							)
WRITE_MPTHEADER_sized_member(	VolEnv.nSustainEnd		, BYTE			, VSE.							)
WRITE_MPTHEADER_sized_member(	PanEnv.nLoopStart		, BYTE			, PLS.							)
WRITE_MPTHEADER_sized_member(	PanEnv.nLoopEnd			, BYTE			, PLE.							)
WRITE_MPTHEADER_sized_member(	PanEnv.nSustainStart	, BYTE			, PSB.							)
WRITE_MPTHEADER_sized_member(	PanEnv.nSustainEnd		, BYTE			, PSE.							)
WRITE_MPTHEADER_sized_member(	PitchEnv.nLoopStart		, BYTE			, PiLS							)
WRITE_MPTHEADER_sized_member(	PitchEnv.nLoopEnd		, BYTE			, PiLE							)
WRITE_MPTHEADER_sized_member(	PitchEnv.nSustainStart	, BYTE			, PiSB							)
WRITE_MPTHEADER_sized_member(	PitchEnv.nSustainEnd	, BYTE			, PiSE							)
WRITE_MPTHEADER_sized_member(	nNNA					, BYTE			, NNA.							)
WRITE_MPTHEADER_sized_member(	nDCT					, BYTE			, DCT.							)
WRITE_MPTHEADER_sized_member(	nDNA					, BYTE			, DNA.							)
WRITE_MPTHEADER_sized_member(	nPanSwing				, BYTE			, PS..							)
WRITE_MPTHEADER_sized_member(	nVolSwing				, BYTE			, VS..							)
WRITE_MPTHEADER_sized_member(	nIFC					, BYTE			, IFC.							)
WRITE_MPTHEADER_sized_member(	nIFR					, BYTE			, IFR.							)
WRITE_MPTHEADER_sized_member(	wMidiBank				, WORD			, MB..							)
WRITE_MPTHEADER_sized_member(	nMidiProgram			, BYTE			, MP..							)
WRITE_MPTHEADER_sized_member(	nMidiChannel			, BYTE			, MC..							)
WRITE_MPTHEADER_sized_member(	nMidiDrumKey			, BYTE			, MDK.							)
WRITE_MPTHEADER_sized_member(	nPPS					, signed char	, PPS.							)
WRITE_MPTHEADER_sized_member(	nPPC					, unsigned char	, PPC.							)
WRITE_MPTHEADER_array_member(	VolEnv.Ticks			, WORD			, VP[.		, MAX_ENVPOINTS		)
WRITE_MPTHEADER_array_member(	PanEnv.Ticks			, WORD			, PP[.		, MAX_ENVPOINTS		)
WRITE_MPTHEADER_array_member(	PitchEnv.Ticks			, WORD			, PiP[		, MAX_ENVPOINTS		)
WRITE_MPTHEADER_array_member(	VolEnv.Values			, BYTE			, VE[.		, MAX_ENVPOINTS		)
WRITE_MPTHEADER_array_member(	PanEnv.Values			, BYTE			, PE[.		, MAX_ENVPOINTS		)
WRITE_MPTHEADER_array_member(	PitchEnv.Values			, BYTE			, PiE[		, MAX_ENVPOINTS		)
WRITE_MPTHEADER_array_member(	NoteMap					, BYTE			, NM[.		, 128				)
WRITE_MPTHEADER_array_member(	Keyboard				, WORD			, K[..		, 128				)
WRITE_MPTHEADER_array_member(	name					, CHAR			, n[..		, 32				)
WRITE_MPTHEADER_array_member(	filename				, CHAR			, fn[.		, 12				)
WRITE_MPTHEADER_sized_member(	nMixPlug				, BYTE			, MiP.							)
WRITE_MPTHEADER_sized_member(	nVolRamp				, USHORT		, VR..							)
WRITE_MPTHEADER_sized_member(	nResampling				, USHORT		, R...							)
WRITE_MPTHEADER_sized_member(	nCutSwing				, BYTE			, CS..							)
WRITE_MPTHEADER_sized_member(	nResSwing				, BYTE			, RS..							)
WRITE_MPTHEADER_sized_member(	nFilterMode				, BYTE			, FM..							)
WRITE_MPTHEADER_sized_member(	nPluginVelocityHandling	, BYTE			, PVEH							)
WRITE_MPTHEADER_sized_member(	nPluginVolumeHandling	, BYTE			, PVOH							)
WRITE_MPTHEADER_sized_member(	wPitchToTempoLock		, WORD			, PTTL							)
WRITE_MPTHEADER_sized_member(	PitchEnv.nReleaseNode	, BYTE			, PERN							)
WRITE_MPTHEADER_sized_member(	PanEnv.nReleaseNode		, BYTE		    , AERN							)
WRITE_MPTHEADER_sized_member(	VolEnv.nReleaseNode		, BYTE			, VERN							)
WRITE_MPTHEADER_sized_member(	PitchEnv.dwFlags		, DWORD			, PFLG							)
WRITE_MPTHEADER_sized_member(	PanEnv.dwFlags			, DWORD		    , AFLG							)
WRITE_MPTHEADER_sized_member(	VolEnv.dwFlags			, DWORD			, VFLG							)
}

// --------------------------------------------------------------------------------------------
// Convenient macro to help GET_HEADER declaration for single type members ONLY (non-array)
// --------------------------------------------------------------------------------------------
#define GET_MPTHEADER_sized_member(name,type,code) \
case( #@code ):\
if( fsize <= sizeof( type ) ) pointer = (BYTE *)&input-> name ;\
break;

// --------------------------------------------------------------------------------------------
// Convenient macro to help GET_HEADER declaration for array members ONLY
// --------------------------------------------------------------------------------------------
#define GET_MPTHEADER_array_member(name,type,code,arraysize) \
case( #@code ):\
if( fsize <= sizeof( type ) * arraysize ) pointer = (BYTE *)&input-> name ;\
break;

// Return a pointer on the wanted field in 'input' MODINSTRUMENT given field code & size
BYTE * GetInstrumentHeaderFieldPointer(MODINSTRUMENT * input, __int32 fcode, __int16 fsize)
{
if(input == NULL) return NULL;
BYTE * pointer = NULL;

switch(fcode){
GET_MPTHEADER_sized_member(	nFadeOut				, UINT			, FO..							)
GET_MPTHEADER_sized_member(	dwFlags					, DWORD			, dF..							)
GET_MPTHEADER_sized_member(	nGlobalVol				, UINT			, GV..							)
GET_MPTHEADER_sized_member(	nPan					, UINT			, P...							)
GET_MPTHEADER_sized_member(	VolEnv.nNodes			, UINT			, VE..							)
GET_MPTHEADER_sized_member(	PanEnv.nNodes			, UINT			, PE..							)
GET_MPTHEADER_sized_member(	PitchEnv.nNodes			, UINT			, PiE.							)
GET_MPTHEADER_sized_member(	VolEnv.nLoopStart		, BYTE			, VLS.							)
GET_MPTHEADER_sized_member(	VolEnv.nLoopEnd			, BYTE			, VLE.							)
GET_MPTHEADER_sized_member(	VolEnv.nSustainStart	, BYTE			, VSB.							)
GET_MPTHEADER_sized_member(	VolEnv.nSustainEnd		, BYTE			, VSE.							)
GET_MPTHEADER_sized_member(	PanEnv.nLoopStart		, BYTE			, PLS.							)
GET_MPTHEADER_sized_member(	PanEnv.nLoopEnd			, BYTE			, PLE.							)
GET_MPTHEADER_sized_member(	PanEnv.nSustainStart	, BYTE			, PSB.							)
GET_MPTHEADER_sized_member(	PanEnv.nSustainEnd		, BYTE			, PSE.							)
GET_MPTHEADER_sized_member(	PitchEnv.nLoopStart		, BYTE			, PiLS							)
GET_MPTHEADER_sized_member(	PitchEnv.nLoopEnd		, BYTE			, PiLE							)
GET_MPTHEADER_sized_member(	PitchEnv.nSustainStart	, BYTE			, PiSB							)
GET_MPTHEADER_sized_member(	PitchEnv.nSustainEnd	, BYTE			, PiSE							)
GET_MPTHEADER_sized_member(	nNNA					, BYTE			, NNA.							)
GET_MPTHEADER_sized_member(	nDCT					, BYTE			, DCT.							)
GET_MPTHEADER_sized_member(	nDNA					, BYTE			, DNA.							)
GET_MPTHEADER_sized_member(	nPanSwing				, BYTE			, PS..							)
GET_MPTHEADER_sized_member(	nVolSwing				, BYTE			, VS..							)
GET_MPTHEADER_sized_member(	nIFC					, BYTE			, IFC.							)
GET_MPTHEADER_sized_member(	nIFR					, BYTE			, IFR.							)
GET_MPTHEADER_sized_member(	wMidiBank				, WORD			, MB..							)
GET_MPTHEADER_sized_member(	nMidiProgram			, BYTE			, MP..							)
GET_MPTHEADER_sized_member(	nMidiChannel			, BYTE			, MC..							)
GET_MPTHEADER_sized_member(	nMidiDrumKey			, BYTE			, MDK.							)
GET_MPTHEADER_sized_member(	nPPS					, signed char	, PPS.							)
GET_MPTHEADER_sized_member(	nPPC					, unsigned char	, PPC.							)
GET_MPTHEADER_array_member(	VolEnv.Ticks			, WORD			, VP[.		, MAX_ENVPOINTS		)
GET_MPTHEADER_array_member(	PanEnv.Ticks			, WORD			, PP[.		, MAX_ENVPOINTS		)
GET_MPTHEADER_array_member(	PitchEnv.Ticks			, WORD			, PiP[		, MAX_ENVPOINTS		)
GET_MPTHEADER_array_member(	VolEnv.Values			, BYTE			, VE[.		, MAX_ENVPOINTS		)
GET_MPTHEADER_array_member(	PanEnv.Values			, BYTE			, PE[.		, MAX_ENVPOINTS		)
GET_MPTHEADER_array_member(	PitchEnv.Values			, BYTE			, PiE[		, MAX_ENVPOINTS		)
GET_MPTHEADER_array_member(	NoteMap					, BYTE			, NM[.		, 128				)
GET_MPTHEADER_array_member(	Keyboard				, WORD			, K[..		, 128				)
GET_MPTHEADER_array_member(	name					, CHAR			, n[..		, 32				)
GET_MPTHEADER_array_member(	filename				, CHAR			, fn[.		, 12				)
GET_MPTHEADER_sized_member(	nMixPlug				, BYTE			, MiP.							)
GET_MPTHEADER_sized_member(	nVolRamp				, USHORT		, VR..							)
GET_MPTHEADER_sized_member(	nResampling				, UINT			, R...							)
GET_MPTHEADER_sized_member(	nCutSwing				, BYTE			, CS..							)
GET_MPTHEADER_sized_member(	nResSwing				, BYTE			, RS..							)
GET_MPTHEADER_sized_member(	nFilterMode				, BYTE			, FM..							)
GET_MPTHEADER_sized_member(	wPitchToTempoLock		, WORD			, PTTL							)
GET_MPTHEADER_sized_member(	nPluginVelocityHandling	, BYTE			, PVEH							)
GET_MPTHEADER_sized_member(	nPluginVolumeHandling	, BYTE			, PVOH							)
GET_MPTHEADER_sized_member(	PitchEnv.nReleaseNode	, BYTE			, PERN							)
GET_MPTHEADER_sized_member(	PanEnv.nReleaseNode		, BYTE		    , AERN							)
GET_MPTHEADER_sized_member(	VolEnv.nReleaseNode		, BYTE			, VERN							)
GET_MPTHEADER_sized_member(	PitchEnv.dwFlags     	, BYTE			, PFLG							)
GET_MPTHEADER_sized_member(	PanEnv.dwFlags     		, BYTE		    , AFLG							)
GET_MPTHEADER_sized_member(	VolEnv.dwFlags     		, BYTE			, VFLG							)
}

return pointer;
}

// -! NEW_FEATURE#0027


CTuning* MODINSTRUMENT::s_DefaultTuning = 0;

const ROWINDEX CPatternSizesMimic::operator [](const int i) const
//-----------------------------------------------------------
{
	return m_rSndFile.Patterns[i].GetNumRows();
}


//////////////////////////////////////////////////////////
// CSoundFile

CTuningCollection* CSoundFile::s_pTuningsSharedBuiltIn(0);
CTuningCollection* CSoundFile::s_pTuningsSharedLocal(0);
uint8 CSoundFile::s_DefaultPlugVolumeHandling = PLUGIN_VOLUMEHANDLING_IGNORE;


CSoundFile::CSoundFile() :
	PatternSize(*this), Patterns(*this),
	Order(*this),
	m_PlaybackEventer(*this),
	m_pModSpecs(&ModSpecs::itEx),
	m_MIDIMapper(*this)
//----------------------
{
	m_nType = MOD_TYPE_NONE;
	m_dwSongFlags = 0;
	m_nChannels = 0;
	m_nMixChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_nPatternNames = 0;
	m_lpszPatternNames = NULL;
	m_lpszSongComments = NULL;
	m_nFreqFactor = m_nTempoFactor = 128;
	m_nMasterVolume = 128;
	m_nMinPeriod = MIN_PERIOD;
	m_nMaxPeriod = 0x7FFF;
	m_nRepeatCount = 0;
	m_nSeqOverride = 0;
	m_bPatternTransitionOccurred = false;
	m_nRowsPerBeat = 4;
	m_nRowsPerMeasure = 16;
	m_nTempoMode = tempo_mode_classic;
	m_bIsRendering = false;
	m_nMaxSample = 0;

	m_ModFlags = 0;

	m_pModDoc = NULL;
	m_dwLastSavedWithVersion=0;
	m_dwCreatedWithVersion=0;
	memset(m_bChannelMuteTogglePending, 0, sizeof(m_bChannelMuteTogglePending));


// -> CODE#0023
// -> DESC="IT project files (.itp)"
	for(UINT i = 0 ; i < MAX_INSTRUMENTS ; i++){
		m_szInstrumentPath[i][0] = '\0';
		instrumentModified[i] = FALSE;
	}
// -! NEW_FEATURE#0023

	memset(Chn, 0, sizeof(Chn));
	memset(ChnMix, 0, sizeof(ChnMix));
	memset(Samples, 0, sizeof(Samples));
	memset(ChnSettings, 0, sizeof(ChnSettings));
	memset(Instruments, 0, sizeof(Instruments));
	Order.Init();
	Patterns.ClearPatterns();
	memset(m_szNames, 0, sizeof(m_szNames));
	memset(m_MixPlugins, 0, sizeof(m_MixPlugins));
	memset(&m_SongEQ, 0, sizeof(m_SongEQ));
	m_lTotalSampleCount=0;

	m_pConfig = new CSoundFilePlayConfig();
	m_pTuningsTuneSpecific = new CTuningCollection("Tune specific tunings");
	
	BuildDefaultInstrument();
}


CSoundFile::~CSoundFile()
//-----------------------
{
	delete m_pConfig;
	delete m_pTuningsTuneSpecific;
	Destroy();
}


BOOL CSoundFile::Create(LPCBYTE lpStream, CModDoc *pModDoc, DWORD dwMemLength)
//---------------------------------------------------------------------------
{
	m_pModDoc=pModDoc;
	m_nType = MOD_TYPE_NONE;
	m_dwSongFlags = 0;
	m_nChannels = 0;
	m_nMixChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_nFreqFactor = m_nTempoFactor = 128;
	m_nMasterVolume = 128;
	m_nDefaultGlobalVolume = 256;
	m_nGlobalVolume = 256;
	m_nOldGlbVolSlide = 0;
	m_nDefaultSpeed = 6;
	m_nDefaultTempo = 125;
	m_nPatternDelay = 0;
	m_nFrameDelay = 0;
	m_nNextRow = 0;
	m_nRow = 0;
	m_nPattern = 0;
	m_nCurrentPattern = 0;
	m_nNextPattern = 0;
	m_nSeqOverride = 0;
	m_nRestartPos = 0;
	m_nMinPeriod = 16;
	m_nMaxPeriod = 32767;
	m_nSamplePreAmp = 48;
	m_nVSTiVolume = 48;
	m_nPatternNames = 0;
	m_nMaxOrderPosition = 0;
	m_lpszPatternNames = NULL;
	m_lpszSongComments = NULL;
	m_nMixLevels = mixLevels_original;	// Will be overridden if appropriate.
	memset(Samples, 0, sizeof(Samples));
	memset(ChnMix, 0, sizeof(ChnMix));
	memset(Chn, 0, sizeof(Chn));
	memset(Instruments, 0, sizeof(Instruments));
	//Order.assign(MAX_ORDERS, Order.GetInvalidPatIndex());
	Order.resize(1);
	Patterns.ClearPatterns();
	memset(m_szNames, 0, sizeof(m_szNames));
	memset(m_MixPlugins, 0, sizeof(m_MixPlugins));
	memset(&m_SongEQ, 0, sizeof(m_SongEQ));
	ResetMidiCfg();
	//for (UINT npt=0; npt<Patterns.Size(); npt++) PatternSize[npt] = 64;
	for (CHANNELINDEX nChn = 0; nChn < MAX_BASECHANNELS; nChn++)
	{
		InitChannel(nChn);
	}
	if (lpStream)
	{
#ifdef ZIPPED_MOD_SUPPORT
		CZipArchive archive(glpszModExtensions);
		if (CZipArchive::IsArchive((LPBYTE)lpStream, dwMemLength))
		{
			if (archive.UnzipArchive((LPBYTE)lpStream, dwMemLength))
			{
				lpStream = archive.GetOutputFile();
				dwMemLength = archive.GetOutputFileLength();
			}
		}
#endif
#ifdef UNRAR_SUPPORT
		CRarArchive unrar((LPBYTE)lpStream, dwMemLength, glpszModExtensions);
		if (unrar.IsArchive())
		{
			if (unrar.ExtrFile())
			{
				lpStream = unrar.GetOutputFile();
				dwMemLength = unrar.GetOutputFileLength();
			}
		}
#endif
#ifdef UNLHA_SUPPORT
		CLhaArchive unlha((LPBYTE)lpStream, dwMemLength, glpszModExtensions);
		if (unlha.IsArchive())
		{
			if (unlha.ExtractFile())
			{
				lpStream = unlha.GetOutputFile();
				dwMemLength = unlha.GetOutputFileLength();
			}
		}
#endif
#ifdef MMCMP_SUPPORT
		BOOL bMMCmp = MMCMP_Unpack(&lpStream, &dwMemLength);
#endif
		if ((!ReadXM(lpStream, dwMemLength))
// -> CODE#0023
// -> DESC="IT project files (.itp)"
		 && (!ReadITProject(lpStream, dwMemLength))
// -! NEW_FEATURE#0023
		 && (!ReadIT(lpStream, dwMemLength))
		 /*&& (!ReadMPT(lpStream, dwMemLength))*/
		 && (!ReadS3M(lpStream, dwMemLength))
		 && (!ReadWav(lpStream, dwMemLength))
#ifndef MODPLUG_BASIC_SUPPORT
		 && (!ReadSTM(lpStream, dwMemLength))
		 && (!ReadMed(lpStream, dwMemLength))
#ifndef FASTSOUNDLIB
		 && (!ReadMTM(lpStream, dwMemLength))
		 && (!ReadMDL(lpStream, dwMemLength))
		 && (!ReadDBM(lpStream, dwMemLength))
		 && (!Read669(lpStream, dwMemLength))
		 && (!ReadFAR(lpStream, dwMemLength))
		 && (!ReadAMS(lpStream, dwMemLength))
		 && (!ReadOKT(lpStream, dwMemLength))
		 && (!ReadPTM(lpStream, dwMemLength))
		 && (!ReadUlt(lpStream, dwMemLength))
		 && (!ReadDMF(lpStream, dwMemLength))
		 && (!ReadDSM(lpStream, dwMemLength))
		 && (!ReadUMX(lpStream, dwMemLength))
		 && (!ReadAMF(lpStream, dwMemLength))
		 && (!ReadPSM(lpStream, dwMemLength))
		 && (!ReadMT2(lpStream, dwMemLength))
#ifdef MODPLUG_TRACKER
		 && (!ReadMID(lpStream, dwMemLength))
#endif // MODPLUG_TRACKER
#endif
#endif // MODPLUG_BASIC_SUPPORT
		 && (!ReadGDM(lpStream, dwMemLength))
		 && (!ReadIMF(lpStream, dwMemLength))
		 && (!ReadAM(lpStream, dwMemLength))
		 && (!ReadJ2B(lpStream, dwMemLength))
		 && (!ReadMO3(lpStream, dwMemLength))
		 && (!ReadMod(lpStream, dwMemLength))) m_nType = MOD_TYPE_NONE;
#ifdef ZIPPED_MOD_SUPPORT
		if ((!m_lpszSongComments) && (archive.GetComments(FALSE)))
		{
			m_lpszSongComments = archive.GetComments(TRUE);
		}
#endif
#ifdef MMCMP_SUPPORT
		if (bMMCmp)
		{
			GlobalFreePtr(lpStream);
			lpStream = NULL;
		}
#endif
	} else {
		// New song
		m_dwCreatedWithVersion = MptVersion::num;
	}

	// Adjust song names
	for (UINT iSmp=0; iSmp<MAX_SAMPLES; iSmp++)
	{
		LPSTR p = m_szNames[iSmp];
		int j = 31;
		p[j] = 0;
		while ((j>=0) && (p[j]<=' ')) p[j--] = 0;
		while (j>=0)
		{
			if (((BYTE)p[j]) < ' ') p[j] = ' ';
			j--;
		}
	}
	// Adjust channels
	for (UINT ich=0; ich<MAX_BASECHANNELS; ich++)
	{
		if (ChnSettings[ich].nVolume > 64) ChnSettings[ich].nVolume = 64;
		if (ChnSettings[ich].nPan > 256) ChnSettings[ich].nPan = 128;
		Chn[ich].nPan = ChnSettings[ich].nPan;
		Chn[ich].nGlobalVol = ChnSettings[ich].nVolume;
		Chn[ich].dwFlags = ChnSettings[ich].dwFlags;
		Chn[ich].nVolume = 256;
		Chn[ich].nCutOff = 0x7F;
		Chn[ich].nEFxSpeed = 0;
		//IT compatibility 15. Retrigger
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			Chn[ich].nRetrigParam = Chn[ich].nRetrigCount = 1;
		}
	}
	// Checking instruments
	MODSAMPLE *pSmp = Samples;
	for (UINT iIns=0; iIns<MAX_INSTRUMENTS; iIns++, pSmp++)
	{
		if (pSmp->pSample)
		{
			if (pSmp->nLoopEnd > pSmp->nLength) pSmp->nLoopEnd = pSmp->nLength;
			if (pSmp->nLoopStart >= pSmp->nLoopEnd)
			{
				pSmp->nLoopStart = 0;
				pSmp->nLoopEnd = 0;
			}
			if (pSmp->nSustainEnd > pSmp->nLength) pSmp->nSustainEnd = pSmp->nLength;
			if (pSmp->nSustainStart >= pSmp->nSustainEnd)
			{
				pSmp->nSustainStart = 0;
				pSmp->nSustainEnd = 0;
			}
		} else
		{
			pSmp->nLength = 0;
			pSmp->nLoopStart = 0;
			pSmp->nLoopEnd = 0;
			pSmp->nSustainStart = 0;
			pSmp->nSustainEnd = 0;
		}
		if (!pSmp->nLoopEnd) pSmp->uFlags &= ~CHN_LOOP;
		if (!pSmp->nSustainEnd) pSmp->uFlags &= ~CHN_SUSTAINLOOP;
		if (pSmp->nGlobalVol > 64) pSmp->nGlobalVol = 64;
	}
	// Check invalid instruments
	while ((m_nInstruments > 0) && (!Instruments[m_nInstruments])) m_nInstruments--;
	// Set default values
	if (m_nDefaultTempo < 32) m_nDefaultTempo = 125;
	if (!m_nDefaultSpeed) m_nDefaultSpeed = 6;
	m_nMusicSpeed = m_nDefaultSpeed;
	m_nMusicTempo = m_nDefaultTempo;
	m_nGlobalVolume = m_nDefaultGlobalVolume;
	m_lHighResRampingGlobalVolume = m_nGlobalVolume<<VOLUMERAMPPRECISION;
	m_nGlobalVolumeDestination = m_nGlobalVolume;
	m_nSamplesToGlobalVolRampDest=0;
	m_nNextPattern = 0;
	m_nCurrentPattern = 0;
	m_nPattern = 0;
	m_nBufferCount = 0;
	m_dBufferDiff = 0;
	m_nTickCount = m_nMusicSpeed;
	m_nNextRow = 0;
	m_nRow = 0;

	switch(m_nTempoMode) {
		case tempo_mode_alternative: 
			m_nSamplesPerTick = gdwMixingFreq / m_nMusicTempo; break;
		case tempo_mode_modern: 
			m_nSamplesPerTick = gdwMixingFreq * (60/m_nMusicTempo / (m_nMusicSpeed * m_nRowsPerBeat)); break;
		case tempo_mode_classic: default:
			m_nSamplesPerTick = (gdwMixingFreq * 5 * m_nTempoFactor) / (m_nMusicTempo << 8);
	}

	if ((m_nRestartPos >= Order.size()) || (Order[m_nRestartPos] >= Patterns.Size())) m_nRestartPos = 0;
	// Load plugins only when m_pModDoc != 0.  (can be == 0 for example when examining module samples in treeview.

	string sNotFound;
	bool bSearchIDs[MAX_MIXPLUGINS];
	memset(bSearchIDs, false, MAX_MIXPLUGINS * sizeof(bool));
	UINT iShowNotFound = 0;

	if (gpMixPluginCreateProc && GetpModDoc())
	{
		for (PLUGINDEX iPlug = 0; iPlug < MAX_MIXPLUGINS; iPlug++)
		{
			if ((m_MixPlugins[iPlug].Info.dwPluginId1)
			 || (m_MixPlugins[iPlug].Info.dwPluginId2))
			{
				gpMixPluginCreateProc(&m_MixPlugins[iPlug], this);
				if (m_MixPlugins[iPlug].pMixPlugin)
				{
					// plugin has been found
					m_MixPlugins[iPlug].pMixPlugin->RestoreAllParameters(m_MixPlugins[iPlug].defaultProgram); //rewbs.plugDefaultProgram: added param
				}
				else
				{
					// plugin not found - add to list
					bool bFound = false;
					for(PLUGINDEX iPlugFind = 0; iPlugFind < iPlug; iPlugFind++)
						if(m_MixPlugins[iPlugFind].Info.dwPluginId2 == m_MixPlugins[iPlug].Info.dwPluginId2)
						{
							bFound = true;
							break;
						}

					if(bFound == false)
					{
						sNotFound = sNotFound + m_MixPlugins[iPlug].Info.szLibraryName + "\n";
						bSearchIDs[iPlug] = true; // set this flag so we will find the needed plugins later when calling KVRAudio
					}
					iShowNotFound++;
				}
			}
		}
	}

	// Display a nice message so the user sees what plugins are missing
	// TODO: Use IDD_MODLOADING_WARNINGS dialog (NON-MODAL!) to display all warnings that are encountered when loading a module.
	if(iShowNotFound)
	{
		if(iShowNotFound == 1)
		{
			sNotFound = "The following plugin has not been found:\n\n" + sNotFound + "\nDo you want to search for it on KVRAudio?";
		}
		else
		{
			sNotFound =	"The following plugins have not been found:\n\n" + sNotFound + "\nDo you want to search for them on KVRAudio?"
						"\nWARNING: A browser window / tab is opened for every plugin. If you do not want that, you can visit http://www.kvraudio.com/search.php";
		}
		if (::MessageBox(0, sNotFound.c_str(), "OpenMPT - Plugins missing", MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES)
			for (PLUGINDEX iPlug = 0; iPlug < MAX_MIXPLUGINS; iPlug++)
				if (bSearchIDs[iPlug] == true)
				{
					CString sUrl;
					sUrl.Format("http://www.kvraudio.com/search.php?q=%s&lq=db", m_MixPlugins[iPlug].Info.szLibraryName);
					CTrackApp::OpenURL(sUrl);
				}
	}

	// Set up mix levels
	m_pConfig->SetMixLevels(m_nMixLevels);
	RecalculateGainForAllPlugs();

	if (m_nType)
	{
		SetModSpecsPointer(m_pModSpecs, m_nType);
		const ORDERINDEX nMinLength = (std::min)(ModSequenceSet::s_nCacheSize, GetModSpecifications().ordersMax);
		if (Order.GetLength() < nMinLength)
			Order.resize(nMinLength);
		return TRUE;
	}

	return FALSE;
}


BOOL CSoundFile::Destroy()
//------------------------
{
	size_t i;
	for (i=0; i<Patterns.Size(); i++) if (Patterns[i])
	{
		FreePattern(Patterns[i]);
		Patterns[i] = NULL;
	}
	m_nPatternNames = 0;

	delete[] m_lpszPatternNames;
	m_lpszPatternNames = NULL;

	delete[] m_lpszSongComments;
	m_lpszSongComments = NULL;

	for (i=1; i<MAX_SAMPLES; i++)
	{
		MODSAMPLE *pSmp = &Samples[i];
		if (pSmp->pSample)
		{
			FreeSample(pSmp->pSample);
			pSmp->pSample = nullptr;
		}
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		delete Instruments[i];
		Instruments[i] = NULL;
	}
	for (i=0; i<MAX_MIXPLUGINS; i++)
	{
		if ((m_MixPlugins[i].nPluginDataSize) && (m_MixPlugins[i].pPluginData))
		{
			m_MixPlugins[i].nPluginDataSize = 0;
			delete[] m_MixPlugins[i].pPluginData;
			m_MixPlugins[i].pPluginData = NULL;
		}
		m_MixPlugins[i].pMixState = NULL;
		if (m_MixPlugins[i].pMixPlugin)
		{
			m_MixPlugins[i].pMixPlugin->Release();
			m_MixPlugins[i].pMixPlugin = NULL;
		}
	}
	m_nType = MOD_TYPE_NONE;
	m_nChannels = m_nSamples = m_nInstruments = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Memory Allocation

MODCOMMAND *CSoundFile::AllocatePattern(UINT rows, UINT nchns)
//------------------------------------------------------------
{
	MODCOMMAND *p = new MODCOMMAND[rows*nchns];
	if (p) memset(p, 0, rows*nchns*sizeof(MODCOMMAND));
	return p;
}


void CSoundFile::FreePattern(LPVOID pat)
//--------------------------------------
{
	
	if (pat) delete pat;
}


LPSTR CSoundFile::AllocateSample(UINT nbytes)
//-------------------------------------------
{
	if (nbytes>0xFFFFFFD6)
		return NULL;
	LPSTR p = (LPSTR)GlobalAllocPtr(GHND, (nbytes+39) & ~7);
	if (p) p += 16;
	return p;
}


void CSoundFile::FreeSample(LPVOID p)
//-----------------------------------
{
	if (p)
	{
		GlobalFreePtr(((LPSTR)p)-16);
	}
}


//////////////////////////////////////////////////////////////////////////
// Misc functions

void CSoundFile::ResetMidiCfg()
//-----------------------------
{
	memset(&m_MidiCfg, 0, sizeof(m_MidiCfg));
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_START*32], "FF");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_STOP*32], "FC");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_NOTEON*32], "9c n v");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_NOTEOFF*32], "9c n 0");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_PROGRAM*32], "Cc p");
	lstrcpy(&m_MidiCfg.szMidiSFXExt[0], "F0F000z");
	for (int iz=0; iz<16; iz++) wsprintf(&m_MidiCfg.szMidiZXXExt[iz*32], "F0F001%02X", iz*8);
}


UINT CSoundFile::GetSongComments(LPSTR s, UINT len, UINT linesize)
//----------------------------------------------------------------
{
	LPCSTR p = m_lpszSongComments;
	if (!p) return 0;
	UINT i = 2, ln=0;
	if ((len) && (s)) s[0] = '\x0D';
	if ((len > 1) && (s)) s[1] = '\x0A';
	while ((*p)	&& (i+2 < len))
	{
		BYTE c = (BYTE)*p++;
		if ((c == 0x0D) || ((c == ' ') && (ln >= linesize)))
			{ if (s) { s[i++] = '\x0D'; s[i++] = '\x0A'; } else i+= 2; ln=0; }
		else
		if (c >= 0x20) { if (s) s[i++] = c; else i++; ln++; }
	}
	if (s) s[i] = 0;
	return i;
}


UINT CSoundFile::GetRawSongComments(LPSTR s, UINT len, UINT linesize)
//-------------------------------------------------------------------
{
	LPCSTR p = m_lpszSongComments;
	if (!p) return 0;
	UINT i = 0, ln=0;
	while ((*p)	&& (i < len-1))
	{
		BYTE c = (BYTE)*p++;
		if ((c == 0x0D)	|| (c == 0x0A))
		{
			if (ln) 
			{
				while (ln < linesize) { if (s) s[i] = ' '; i++; ln++; }
				ln = 0;
			}
		} else
		if ((c == ' ') && (!ln))
		{
			UINT k=0;
			while ((p[k]) && (p[k] >= ' '))	k++;
			if (k <= linesize)
			{
				if (s) s[i] = ' ';
				i++;
				ln++;
			}
		} else
		{
			if (s) s[i] = c;
			i++;
			ln++;
			if (ln == linesize) ln = 0;
		}
	}
	if (ln)
	{
		while ((ln < linesize) && (i < len))
		{
			if (s) s[i] = ' ';
			i++;
			ln++;
		}
	}
	if (s) s[i] = 0;
	return i;
}


BOOL CSoundFile::SetWaveConfig(UINT nRate,UINT nBits,UINT nChannels,BOOL bMMX)
//----------------------------------------------------------------------------
{
	BOOL bReset = FALSE;
	DWORD d = gdwSoundSetup & ~SNDMIX_ENABLEMMX;
	if (bMMX) d |= SNDMIX_ENABLEMMX;
	if ((gdwMixingFreq != nRate) || (gnBitsPerSample != nBits) || (gnChannels != nChannels) || (d != gdwSoundSetup)) bReset = TRUE;
	gnChannels = nChannels;
	gdwSoundSetup = d;
	gdwMixingFreq = nRate;
	gnBitsPerSample = nBits;
	InitPlayer(bReset);
	return TRUE;
}


BOOL CSoundFile::SetDspEffects(BOOL bSurround,BOOL bReverb,BOOL bMegaBass,BOOL bNR,BOOL bEQ)
//------------------------------------------------------------------------------------------
{
	DWORD d = gdwSoundSetup & ~(SNDMIX_SURROUND | SNDMIX_REVERB | SNDMIX_MEGABASS | SNDMIX_NOISEREDUCTION | SNDMIX_EQ);
	if (bSurround) d |= SNDMIX_SURROUND;
	if ((bReverb) && (gdwSysInfo & SYSMIX_ENABLEMMX)) d |= SNDMIX_REVERB;
	if (bMegaBass) d |= SNDMIX_MEGABASS;
	if (bNR) d |= SNDMIX_NOISEREDUCTION;
	if (bEQ) d |= SNDMIX_EQ;
	gdwSoundSetup = d;
	InitPlayer(FALSE);
	return TRUE;
}


BOOL CSoundFile::SetResamplingMode(UINT nMode)
//--------------------------------------------
{
	DWORD d = gdwSoundSetup & ~(SNDMIX_NORESAMPLING|SNDMIX_SPLINESRCMODE|SNDMIX_POLYPHASESRCMODE|SNDMIX_FIRFILTERSRCMODE);
	switch(nMode)
	{
	case SRCMODE_NEAREST:	d |= SNDMIX_NORESAMPLING; break;
	case SRCMODE_LINEAR:	break; // default
	//rewbs.resamplerConf
	//case SRCMODE_SPLINE:	d |= SNDMIX_HQRESAMPLER; break;
	//case SRCMODE_POLYPHASE:	d |= (SNDMIX_HQRESAMPLER|SNDMIX_ULTRAHQSRCMODE); break;
	case SRCMODE_SPLINE:	d |= SNDMIX_SPLINESRCMODE; break;
	case SRCMODE_POLYPHASE:	d |= SNDMIX_POLYPHASESRCMODE; break;
	case SRCMODE_FIRFILTER:	d |= SNDMIX_FIRFILTERSRCMODE; break;
	default: return FALSE;
	//end rewbs.resamplerConf
	}
	gdwSoundSetup = d;
	return TRUE;
}


void CSoundFile::SetMasterVolume(UINT nVol, bool adjustAGC)
//---------------------------------------------------------
{
	if (nVol < 1) nVol = 1;
	if (nVol > 0x200) nVol = 0x200;	// x4 maximum
	if ((nVol < m_nMasterVolume) && (nVol) && (gdwSoundSetup & SNDMIX_AGC) && (adjustAGC))
	{
		gnAGC = gnAGC * m_nMasterVolume / nVol;
		if (gnAGC > AGC_UNITY) gnAGC = AGC_UNITY;
	}
	m_nMasterVolume = nVol;
}


void CSoundFile::SetAGC(BOOL b)
//-----------------------------
{
	if (b)
	{
		if (!(gdwSoundSetup & SNDMIX_AGC))
		{
			gdwSoundSetup |= SNDMIX_AGC;
			gnAGC = AGC_UNITY;
		}
	} else gdwSoundSetup &= ~SNDMIX_AGC;
}


PATTERNINDEX CSoundFile::GetNumPatterns() const
//---------------------------------------------
{
	PATTERNINDEX max = 0;
	for(PATTERNINDEX i = 0; i < Patterns.Size(); i++)
	{
		if(Patterns.IsValidPat(i))
			max = i;
	}
	return max;
}


UINT CSoundFile::GetMaxPosition() const
//-------------------------------------
{
	UINT max = 0;
	UINT i = 0;

	while ((i < Order.size()) && (Order[i] != Order.GetInvalidPatIndex()))
	{
		if (Order[i] < Patterns.Size()) max += PatternSize[Order[i]];
		i++;
	}
	return max;
}


UINT CSoundFile::GetCurrentPos() const
//------------------------------------
{
	UINT pos = 0;

	for (UINT i=0; i<m_nCurrentPattern; i++) if (Order[i] < Patterns.Size())
		pos += PatternSize[Order[i]];
	return pos + m_nRow; 
}

double  CSoundFile::GetCurrentBPM() const
//---------------------------------------
{
	double bpm;

	if (m_nTempoMode == tempo_mode_modern) {		// With modern mode, we trust that true bpm 
		bpm = static_cast<double>(m_nMusicTempo);	// is  be close enough to what user chose.
	}												// This avoids oscillation due to tick-to-tick corrections.

	else {												//with other modes, we calculate it:
		double ticksPerBeat = m_nMusicSpeed*m_nRowsPerBeat;		//ticks/beat = ticks/row  * rows/beat
		double samplesPerBeat = m_nSamplesPerTick*ticksPerBeat;	//samps/beat = samps/tick * ticks/beat
		bpm =  gdwMixingFreq/samplesPerBeat*60;					//beats/sec  = samps/sec  / samps/beat
	}															//beats/min  =  beats/sec * 60
	
	return bpm;
}

void CSoundFile::SetCurrentPos(UINT nPos)
//---------------------------------------
{
	ORDERINDEX nPattern;
	BYTE resetMask = (!nPos) ? CHNRESET_SETPOS_FULL : CHNRESET_SETPOS_BASIC;

	for (CHANNELINDEX i=0; i<MAX_CHANNELS; i++)
		ResetChannelState(i, resetMask);
	
	if (!nPos)
	{
		m_nGlobalVolume = m_nDefaultGlobalVolume;
		m_nMusicSpeed = m_nDefaultSpeed;
		m_nMusicTempo = m_nDefaultTempo;
	}
	//m_dwSongFlags &= ~(SONG_PATTERNLOOP|SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	m_dwSongFlags &= ~(SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	for (nPattern = 0; nPattern < Order.size(); nPattern++)
	{
		UINT ord = Order[nPattern];
		if(ord == Order.GetIgnoreIndex()) continue;
		if (ord == Order.GetInvalidPatIndex()) break;
		if (ord < Patterns.Size())
		{
			if (nPos < (UINT)PatternSize[ord]) break;
			nPos -= PatternSize[ord];
		}
	}
	// Buggy position ?
	if ((nPattern >= Order.size())
	 || (Order[nPattern] >= Patterns.Size())
	 || (nPos >= PatternSize[Order[nPattern]]))
	{
		nPos = 0;
		nPattern = 0;
	}
	UINT nRow = nPos;
	if ((nRow) && (Order[nPattern] < Patterns.Size()))
	{
		MODCOMMAND *p = Patterns[Order[nPattern]];
		if ((p) && (nRow < PatternSize[Order[nPattern]]))
		{
			BOOL bOk = FALSE;
			while ((!bOk) && (nRow > 0))
			{
				UINT n = nRow * m_nChannels;
				for (UINT k=0; k<m_nChannels; k++, n++)
				{
					if (p[n].note)
					{
						bOk = TRUE;
						break;
					}
				}
				if (!bOk) nRow--;
			}
		}
	}
	m_nNextPattern = nPattern;
	m_nNextRow = nRow;
	m_nTickCount = m_nMusicSpeed;
	m_nBufferCount = 0;
	m_nPatternDelay = 0;
	m_nFrameDelay = 0;
	//m_nSeqOverride = 0;
}



void CSoundFile::SetCurrentOrder(ORDERINDEX nOrder)
//-----------------------------------------------
{
	//while ((nPos < Order.size()) && (Order[nPos] == 0xFE)) nPos++;
	while ((nOrder < Order.size()) && (Order[nOrder] == Order.GetIgnoreIndex())) nOrder++;
	if ((nOrder >= Order.size()) || (Order[nOrder] >= Patterns.Size())) return;
	for (CHANNELINDEX j = 0; j < MAX_CHANNELS; j++)
	{
		Chn[j].nPeriod = 0;
		Chn[j].nNote = NOTE_NONE;
		Chn[j].nPortamentoDest = 0;
		Chn[j].nCommand = 0;
		Chn[j].nPatternLoopCount = 0;
		Chn[j].nPatternLoop = 0;
		Chn[j].nVibratoPos = Chn[j].nTremoloPos = Chn[j].nPanbrelloPos = 0;
		//IT compatibility 15. Retrigger
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			Chn[j].nRetrigCount = 0;
			Chn[j].nRetrigParam = 1;
		}
		Chn[j].nTremorCount = 0;
	}
	if (!nOrder)
	{
		SetCurrentPos(0);
	} else
	{
		m_nNextPattern = nOrder;
		m_nRow = m_nNextRow = 0;
		m_nPattern = 0;
		m_nTickCount = m_nMusicSpeed;
		m_nBufferCount = 0;
		m_nTotalCount = 0;
		m_nPatternDelay = 0;
		m_nFrameDelay = 0;
	}
	//m_dwSongFlags &= ~(SONG_PATTERNLOOP|SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	m_dwSongFlags &= ~(SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
}

//rewbs.VSTCompliance
void CSoundFile::SuspendPlugins()	
//------------------------------
{
	for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++)
	{
		if (!m_MixPlugins[iPlug].pMixPlugin)	
			continue;  //most common branch
		
		IMixPlugin *pPlugin = m_MixPlugins[iPlug].pMixPlugin;
		if (m_MixPlugins[iPlug].pMixState)
		{
			pPlugin->NotifySongPlaying(false);
			pPlugin->HardAllNotesOff();
			pPlugin->Suspend();
		}
	}
	m_lTotalSampleCount=0;
}

void CSoundFile::ResumePlugins()	
//------------------------------
{
	for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++)
	{
		if (!m_MixPlugins[iPlug].pMixPlugin)	
			continue;  //most common branch

		if (m_MixPlugins[iPlug].pMixState)
		{
            IMixPlugin *pPlugin = m_MixPlugins[iPlug].pMixPlugin;
			pPlugin->NotifySongPlaying(true);
			pPlugin->Resume();
		}
	}
	m_lTotalSampleCount=GetSampleOffset();

}


void CSoundFile::StopAllVsti()
//----------------------------
{
	for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++) {
		if (!m_MixPlugins[iPlug].pMixPlugin)	
			continue;  //most common branch
		
		IMixPlugin *pPlugin = m_MixPlugins[iPlug].pMixPlugin;
		if (m_MixPlugins[iPlug].pMixState) {
			pPlugin->HardAllNotesOff();
		}
	}
	m_lTotalSampleCount=GetSampleOffset();
}


void CSoundFile::RecalculateGainForAllPlugs()
//------------------------------------------
{
	for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++) {
		if (!m_MixPlugins[iPlug].pMixPlugin)	
			continue;  //most common branch
		
		IMixPlugin *pPlugin = m_MixPlugins[iPlug].pMixPlugin;
		if (m_MixPlugins[iPlug].pMixState) {
			pPlugin->RecalculateGain();
		}
	}
}

//end rewbs.VSTCompliance

void CSoundFile::ResetChannels()
//------------------------------
{
	m_dwSongFlags &= ~(SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	m_nBufferCount = 0;
	for (UINT i=0; i<MAX_CHANNELS; i++)
	{
		Chn[i].nROfs = Chn[i].nLOfs = 0;
	}
}



void CSoundFile::LoopPattern(PATTERNINDEX nPat, ROWINDEX nRow)
//------------------------------------------------------------
{
	if ((nPat < 0) || (nPat >= Patterns.Size()) || (!Patterns[nPat]))
	{
		m_dwSongFlags &= ~SONG_PATTERNLOOP;
	} else
	{
		if ((nRow < 0) || (nRow >= (int)PatternSize[nPat])) nRow = 0;
		m_nPattern = nPat;
		m_nRow = m_nNextRow = nRow;
		m_nTickCount = m_nMusicSpeed;
		m_nPatternDelay = 0;
		m_nFrameDelay = 0;
		m_nBufferCount = 0;
		m_dwSongFlags |= SONG_PATTERNLOOP;
	//	m_nSeqOverride = 0;
	}
}
//rewbs.playSongFromCursor
void CSoundFile::DontLoopPattern(PATTERNINDEX nPat, ROWINDEX nRow)
//----------------------------------------------------------------
{
	if ((nPat < 0) || (nPat >= Patterns.Size()) || (!Patterns[nPat])) nPat = 0;
	if ((nRow < 0) || (nRow >= (int)PatternSize[nPat])) nRow = 0;
	m_nPattern = nPat;
	m_nRow = m_nNextRow = nRow;
	m_nTickCount = m_nMusicSpeed;
	m_nPatternDelay = 0;
	m_nFrameDelay = 0;
	m_nBufferCount = 0;
	m_dwSongFlags &= ~SONG_PATTERNLOOP;
	//m_nSeqOverride = 0;
}

ORDERINDEX CSoundFile::FindOrder(PATTERNINDEX nPat, UINT startFromOrder, bool direction)
//-------------------------------------------------------------------------------------
{
	ORDERINDEX foundAtOrder = ORDERINDEX_INVALID;
	ORDERINDEX candidateOrder = 0;

	for (ORDERINDEX p = 0; p < Order.size(); p++)
	{
		if (direction)
		{
			candidateOrder = (startFromOrder + p) % Order.size();			//wrap around MAX_ORDERS
		} else {
			candidateOrder = (startFromOrder - p + Order.size()) % Order.size();	//wrap around 0 and MAX_ORDERS
		}
		if (Order[candidateOrder] == nPat) {
			foundAtOrder = candidateOrder;
			break;
		}
	}

	return foundAtOrder;
}
//end rewbs.playSongFromCursor


MODTYPE CSoundFile::GetBestSaveFormat() const
//-------------------------------------------
{
	if ((!m_nSamples) || (!m_nChannels)) return MOD_TYPE_NONE;
	if (!m_nType) return MOD_TYPE_NONE;
	if (m_nType & (MOD_TYPE_MOD|MOD_TYPE_OKT))
		return MOD_TYPE_MOD;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_STM|MOD_TYPE_ULT|MOD_TYPE_FAR|MOD_TYPE_PTM))
		return MOD_TYPE_S3M;
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MED|MOD_TYPE_MTM|MOD_TYPE_MT2))
		return MOD_TYPE_XM;
	if(m_nType & MOD_TYPE_MPT)
		return MOD_TYPE_MPT;
	return MOD_TYPE_IT;
}


MODTYPE CSoundFile::GetSaveFormats() const
//----------------------------------------
{
	UINT n = 0;
	if ((!m_nSamples) || (!m_nChannels) || (m_nType == MOD_TYPE_NONE)) return 0;
	switch(m_nType)
	{
	case MOD_TYPE_MOD:	n = MOD_TYPE_MOD;
	case MOD_TYPE_S3M:	n = MOD_TYPE_S3M;
	}
	n |= MOD_TYPE_XM | MOD_TYPE_IT | MOD_TYPE_MPT;
	if (!m_nInstruments)
	{
		if (m_nSamples < 32) n |= MOD_TYPE_MOD;
		n |= MOD_TYPE_S3M;
	}
	return n;
}


LPCTSTR CSoundFile::GetSampleName(UINT nSample) const
//---------------------------------------------------
{
	if (nSample<MAX_SAMPLES) {
		return m_szNames[nSample];
	} else {
		return gszEmpty;
	}
}


CString CSoundFile::GetInstrumentName(UINT nInstr) const
//------------------------------------------------------
{
	if ((nInstr >= MAX_INSTRUMENTS) || (!Instruments[nInstr]))
		return TEXT("");

	const size_t nSize = ARRAYELEMCOUNT(Instruments[nInstr]->name);
	CString str;
	LPTSTR p = str.GetBuffer(nSize + 1);
	ArrayCopy(p, Instruments[nInstr]->name, nSize);
	p[nSize] = 0;
	str.ReleaseBuffer();
	return str;
}


bool CSoundFile::InitChannel(CHANNELINDEX nChn)
//---------------------------------------------
{
	if(nChn >= MAX_BASECHANNELS) return true;

	ChnSettings[nChn].nPan = 128;
	ChnSettings[nChn].nVolume = 64;
	ChnSettings[nChn].dwFlags = 0;
	ChnSettings[nChn].nMixPlugin = 0;
	ChnSettings[nChn].szName[0] = 0;

	ResetChannelState(nChn, CHNRESET_TOTAL);

	if(m_pModDoc)
	{
		m_pModDoc->Record1Channel(nChn, false);
		m_pModDoc->Record2Channel(nChn, false);
	}
	m_bChannelMuteTogglePending[nChn] = false;

	return false;
}

void CSoundFile::ResetChannelState(CHANNELINDEX i, BYTE resetMask)
//-------------------------------------------------------
{
	if(i >= MAX_CHANNELS) return;

	if(resetMask & 2)
	{
		Chn[i].nNote = Chn[i].nNewNote = Chn[i].nNewIns = 0;
		Chn[i].pModSample = nullptr;
		Chn[i].pModInstrument = nullptr;
		Chn[i].nPortamentoDest = 0;
		Chn[i].nCommand = 0;
		Chn[i].nPatternLoopCount = 0;
		Chn[i].nPatternLoop = 0;
		Chn[i].nFadeOutVol = 0;
		Chn[i].dwFlags |= CHN_KEYOFF|CHN_NOTEFADE;
		//IT compatibility 15. Retrigger
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			Chn[i].nRetrigParam = 1;
			Chn[i].nRetrigCount = 0;
		}
		Chn[i].nTremorCount = 0;
		Chn[i].nEFxSpeed = 0;
	}

	if(resetMask & 4)
	{
		Chn[i].nPeriod = 0;
		Chn[i].nPos = Chn[i].nLength = 0;
		Chn[i].nLoopStart = 0;
		Chn[i].nLoopEnd = 0;
		Chn[i].nROfs = Chn[i].nLOfs = 0;
		Chn[i].pSample = nullptr;
		Chn[i].pModSample = nullptr;
		Chn[i].pModInstrument = nullptr;
		Chn[i].nCutOff = 0x7F;
		Chn[i].nResonance = 0;
		Chn[i].nFilterMode = 0;
		Chn[i].nLeftVol = Chn[i].nRightVol = 0;
		Chn[i].nNewLeftVol = Chn[i].nNewRightVol = 0;
		Chn[i].nLeftRamp = Chn[i].nRightRamp = 0;
		Chn[i].nVolume = 256;
		Chn[i].nVibratoPos = Chn[i].nTremoloPos = Chn[i].nPanbrelloPos = 0;

		//-->Custom tuning related
		Chn[i].m_ReCalculateFreqOnFirstTick = false;
		Chn[i].m_CalculateFreq = false;
		Chn[i].m_PortamentoFineSteps = 0;
		Chn[i].m_PortamentoTickSlide = 0;
		Chn[i].m_Freq = 0;
		Chn[i].m_VibratoDepth = 0;
		//<--Custom tuning related.
	}

	if(resetMask & 1)
	{
		if(i < MAX_BASECHANNELS)
		{
			Chn[i].dwFlags = ChnSettings[i].dwFlags;
			Chn[i].nPan = ChnSettings[i].nPan;
			Chn[i].nGlobalVol = ChnSettings[i].nVolume;
		}
		else
		{
			Chn[i].dwFlags = 0;
			Chn[i].nPan = 128;
			Chn[i].nGlobalVol = 64;
		}
		Chn[i].nRestorePanOnNewNote = 0;
		Chn[i].nRestoreCutoffOnNewNote = 0;
		Chn[i].nRestoreResonanceOnNewNote = 0;
		
	}
}


CHANNELINDEX CSoundFile::ReArrangeChannels(const vector<CHANNELINDEX>& newOrder)
//-------------------------------------------------------------------
{
    //newOrder[i] tells which current channel should be placed to i:th position in
    //the new order, or if i is not an index of current channels, then new channel is
    //added to position i. If index of some current channel is missing from the
    //newOrder-vector, then the channel gets removed.
	
	CHANNELINDEX nRemainingChannels = newOrder.size();	

	if(nRemainingChannels > GetModSpecifications().channelsMax || nRemainingChannels < GetModSpecifications().channelsMin) 	
	{
		CString str;
		str.Format(GetStrI18N(_TEXT("Can't apply change: Number of channels should be within [%u,%u]")), GetModSpecifications().channelsMin, GetModSpecifications().channelsMax);
		CMainFrame::GetMainFrame()->MessageBox(str , "ReArrangeChannels", MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	BEGIN_CRITICAL();
	for (PATTERNINDEX nPat = 0; nPat < Patterns.Size(); nPat++) 
	{
		if (Patterns[nPat])
		{
			MODCOMMAND *p = Patterns[nPat];
			MODCOMMAND *newp = CSoundFile::AllocatePattern(PatternSize[nPat], nRemainingChannels);
			if (!newp)
			{
				END_CRITICAL();
				CMainFrame::GetMainFrame()->MessageBox("ERROR: Pattern allocation failed in ReArrangechannels(...)" , "ReArrangeChannels", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			MODCOMMAND *tmpsrc = p, *tmpdest = newp;
			for (ROWINDEX nRow = 0; nRow<PatternSize[nPat]; nRow++) //Scrolling rows
			{
				for (CHANNELINDEX nChn = 0; nChn < nRemainingChannels; nChn++, tmpdest++) //Scrolling channels.
				{
					if(newOrder[nChn] < m_nChannels) //Case: getting old channel to the new channel order.
						*tmpdest = tmpsrc[nRow*m_nChannels+newOrder[nChn]];
					else //Case: figure newOrder[k] is not the index of any current channel, so adding a new channel.
						*tmpdest = MODCOMMAND::Empty();
							
				}
			}
			Patterns[nPat] = newp;
			CSoundFile::FreePattern(p);
		}
	}

	MODCHANNELSETTINGS settings[MAX_BASECHANNELS];
	MODCHANNEL chns[MAX_BASECHANNELS];		
	UINT recordStates[MAX_BASECHANNELS];
	bool chnMutePendings[MAX_BASECHANNELS];

	for(CHANNELINDEX nChn = 0; nChn < m_nChannels; nChn++)
	{
		settings[nChn] = ChnSettings[nChn];
		chns[nChn] = Chn[nChn];
		if(m_pModDoc)
			recordStates[nChn] = m_pModDoc->IsChannelRecord(nChn);
		chnMutePendings[nChn] = m_bChannelMuteTogglePending[nChn];
	}
	
	if(m_pModDoc)
		m_pModDoc->ReinitRecordState();

	for (CHANNELINDEX nChn = 0; nChn < nRemainingChannels; nChn++)
	{
		if(newOrder[nChn] < m_nChannels)
		{
				ChnSettings[nChn] = settings[newOrder[nChn]];
				Chn[nChn] = chns[newOrder[nChn]];
				if(m_pModDoc)
				{
					if(recordStates[newOrder[nChn]] == 1) m_pModDoc->Record1Channel(nChn, true);
					if(recordStates[newOrder[nChn]] == 2) m_pModDoc->Record2Channel(nChn, true);
				}
				m_bChannelMuteTogglePending[nChn] = chnMutePendings[newOrder[nChn]];
		}
		else
		{
			InitChannel(nChn);
		}
	}
	// Reset MOD panning (won't affect other module formats)
	SetupMODPanning();

	m_nChannels = nRemainingChannels;
	END_CRITICAL();

	return static_cast<CHANNELINDEX>(m_nChannels);
}

bool CSoundFile::MoveChannel(UINT chnFrom, UINT chnTo)
//-----------------------------------------------------
{
    //Implementation of move channel using ReArrangeChannels(...). So this function
    //only creates correct newOrder-vector used in the ReArrangeChannels(...).
	if(chnFrom == chnTo) return false;
    if(chnFrom >= m_nChannels || chnTo >= m_nChannels)
    {
		CString str = "Error: Bad move indexes in CSoundFile::MoveChannel(...)";	
		CMainFrame::GetMainFrame()->MessageBox(str , "MoveChannel(...)", MB_OK | MB_ICONINFORMATION);
		return true;
    }
	vector<CHANNELINDEX> newOrder;
	//First creating new order identical to current order...
	for(CHANNELINDEX i = 0; i<GetNumChannels(); i++)
	{
		newOrder.push_back(i);
	}
	//...and then add the move channel effect.
	if(chnFrom < chnTo)
	{
		CHANNELINDEX temp = newOrder[chnFrom];
		for(UINT i = chnFrom; i<chnTo; i++)
		{
			newOrder[i] = newOrder[i+1];
		}
		newOrder[chnTo] = temp;
	}
	else //case chnFrom > chnTo(can't be equal, since it has been examined earlier.)
	{
		CHANNELINDEX temp = newOrder[chnFrom];
		for(UINT i = chnFrom; i>=chnTo+1; i--)
		{
			newOrder[i] = newOrder[i-1];
		}
		newOrder[chnTo] = temp;
     }

	if(newOrder.size() != ReArrangeChannels(newOrder))
	{
		CMainFrame::GetMainFrame()->MessageBox("BUG: Channel number changed in MoveChannel()" , "", MB_OK | MB_ICONINFORMATION);
	}
	return false;
}


#ifndef NO_PACKING
UINT CSoundFile::PackSample(int &sample, int next)
//------------------------------------------------
{
	UINT i = 0;
	int delta = next - sample;
	if (delta >= 0)
	{
		for (i=0; i<7; i++) if (delta <= (int)CompressionTable[i+1]) break;
	} else
	{
		for (i=8; i<15; i++) if (delta >= (int)CompressionTable[i+1]) break;
	}
	sample += (int)CompressionTable[i];
	return i;
}


bool CSoundFile::CanPackSample(LPSTR pSample, UINT nLen, UINT nPacking, BYTE *result/*=NULL*/)
//--------------------------------------------------------------------------------------------
{
	int pos, old, oldpos, besttable = 0;
	DWORD dwErr, dwTotal, dwResult;
	int i,j;
	
	if (result) *result = 0;
	if ((!pSample) || (nLen < 1024)) return false;
	// Try packing with different tables
	dwResult = 0;
	for (j=1; j<MAX_PACK_TABLES; j++)
	{
		memcpy(CompressionTable, UnpackTable[j], 16);
		dwErr = 0;
		dwTotal = 1;
		old = pos = oldpos = 0;
		for (i=0; i<(int)nLen; i++)
		{
			int s = (int)pSample[i];
			PackSample(pos, s);
			dwErr += abs(pos - oldpos);
			dwTotal += abs(s - old);
			old = s;
			oldpos = pos;
		}
		dwErr = _muldiv(dwErr, 100, dwTotal);
		if (dwErr >= dwResult)
		{
			dwResult = dwErr;
			besttable = j;
		}
	}
	memcpy(CompressionTable, UnpackTable[besttable], 16);
	if (result)
	{
		if (dwResult > 100) *result	= 100; else *result = (BYTE)dwResult;
	}
	return (dwResult >= nPacking) ? true : false;
}
#endif // NO_PACKING

#ifndef MODPLUG_NO_FILESAVE

UINT CSoundFile::WriteSample(FILE *f, MODSAMPLE *pSmp, UINT nFlags, UINT nMaxLen)
//-----------------------------------------------------------------------------------
{
	UINT len = 0, bufcount;
	char buffer[4096];
	signed char *pSample = (signed char *)pSmp->pSample;
	UINT nLen = pSmp->nLength;
	
	if ((nMaxLen) && (nLen > nMaxLen)) nLen = nMaxLen;
// -> CODE#0023
// -> DESC="IT project files (.itp)"
//	if ((!pSample) || (f == NULL) || (!nLen)) return 0;
	if ((!pSample) || (!nLen)) return 0;
// NOTE : also added all needed 'if(f)' in this function
// -! NEW_FEATURE#0023
	switch(nFlags)
	{
#ifndef NO_PACKING
	// 3: 4-bit ADPCM data
	case RS_ADPCM4:
		{
			int pos; 
			len = (nLen + 1) / 2;
			if(f) fwrite(CompressionTable, 16, 1, f);
			bufcount = 0;
			pos = 0;
			for (UINT j=0; j<len; j++)
			{
				BYTE b;
				// Sample #1
				b = PackSample(pos, (int)pSample[j*2]);
				// Sample #2
				b |= PackSample(pos, (int)pSample[j*2+1]) << 4;
				buffer[bufcount++] = (char)b;
				if (bufcount >= sizeof(buffer))
				{
					if(f) fwrite(buffer, 1, bufcount, f);
					bufcount = 0;
				}
			}
			if (bufcount) if(f) fwrite(buffer, 1, bufcount, f);
			len += 16;
		}
		break;
#endif // NO_PACKING

	// 16-bit samples
	case RS_PCM16U:
	case RS_PCM16D:
	case RS_PCM16S:
		{
			short int *p = (short int *)pSample;
			int s_old = 0, s_ofs;
			len = nLen * 2;
			bufcount = 0;
			s_ofs = (nFlags == RS_PCM16U) ? 0x8000 : 0;
			for (UINT j=0; j<nLen; j++)
			{
				int s_new = *p;
				p++;
				if (pSmp->uFlags & CHN_STEREO)
				{
					s_new = (s_new + (*p) + 1) >> 1;
					p++;
				}
				if (nFlags == RS_PCM16D)
				{
					*((short *)(&buffer[bufcount])) = (short)(s_new - s_old);
					s_old = s_new;
				} else
				{
					*((short *)(&buffer[bufcount])) = (short)(s_new + s_ofs);
				}
				bufcount += 2;
				if (bufcount >= sizeof(buffer) - 1)
				{
					if(f) fwrite(buffer, 1, bufcount, f);
					bufcount = 0;
				}
			}
			if (bufcount) if(f) fwrite(buffer, 1, bufcount, f);
		}
		break;

	// 8-bit Stereo samples (not interleaved)
	case RS_STPCM8S:
	case RS_STPCM8U:
	case RS_STPCM8D:
		{
			int s_ofs = (nFlags == RS_STPCM8U) ? 0x80 : 0;
			for (UINT iCh=0; iCh<2; iCh++)
			{
				signed char *p = pSample + iCh;
				int s_old = 0;

				bufcount = 0;
				for (UINT j=0; j<nLen; j++)
				{
					int s_new = *p;
					p += 2;
					if (nFlags == RS_STPCM8D)
					{
						buffer[bufcount++] = (char)(s_new - s_old);
						s_old = s_new;
					} else
					{
						buffer[bufcount++] = (char)(s_new + s_ofs);
					}
					if (bufcount >= sizeof(buffer))
					{
						if(f) fwrite(buffer, 1, bufcount, f);
						bufcount = 0;
					}
				}
				if (bufcount) if(f) fwrite(buffer, 1, bufcount, f);
			}
		}
		len = nLen * 2;
		break;

	// 16-bit Stereo samples (not interleaved)
	case RS_STPCM16S:
	case RS_STPCM16U:
	case RS_STPCM16D:
		{
			int s_ofs = (nFlags == RS_STPCM16U) ? 0x8000 : 0;
			for (UINT iCh=0; iCh<2; iCh++)
			{
				signed short *p = ((signed short *)pSample) + iCh;
				int s_old = 0;

				bufcount = 0;
				for (UINT j=0; j<nLen; j++)
				{
					int s_new = *p;
					p += 2;
					if (nFlags == RS_STPCM16D)
					{
						*((short *)(&buffer[bufcount])) = (short)(s_new - s_old);
						s_old = s_new;
					} else
					{
						*((short *)(&buffer[bufcount])) = (short)(s_new + s_ofs);
					}
					bufcount += 2;
					if (bufcount >= sizeof(buffer))
					{
						if(f) fwrite(buffer, 1, bufcount, f);
						bufcount = 0;
					}
				}
				if (bufcount) if(f) fwrite(buffer, 1, bufcount, f);
			}
		}
		len = nLen*4;
		break;

	//	Stereo signed interleaved
	case RS_STIPCM8S:
	case RS_STIPCM16S:
		len = nLen * 2;
		if (nFlags == RS_STIPCM16S) len *= 2;
		if(f) fwrite(pSample, 1, len, f);
		break;

	// Default: assume 8-bit PCM data
	default:
		len = nLen;
		bufcount = 0;
		{
			signed char *p = pSample;
			int sinc = (pSmp->uFlags & CHN_16BIT) ? 2 : 1;
			int s_old = 0, s_ofs = (nFlags == RS_PCM8U) ? 0x80 : 0;
			if (pSmp->uFlags & CHN_16BIT) p++;
			for (UINT j=0; j<len; j++)
			{
				int s_new = (signed char)(*p);
				p += sinc;
				if (pSmp->uFlags & CHN_STEREO)
				{
					s_new = (s_new + ((int)*p) + 1) >> 1;
					p += sinc;
				}
				if (nFlags == RS_PCM8D)
				{
					buffer[bufcount++] = (char)(s_new - s_old);
					s_old = s_new;
				} else
				{
					buffer[bufcount++] = (char)(s_new + s_ofs);
				}
				if (bufcount >= sizeof(buffer))
				{
					if(f) fwrite(buffer, 1, bufcount, f);
					bufcount = 0;
				}
			}
			if (bufcount) if(f) fwrite(buffer, 1, bufcount, f);
		}
	}
	return len;
}

#endif // MODPLUG_NO_FILESAVE


// Flags:
//	0 = signed 8-bit PCM data (default)
//	1 = unsigned 8-bit PCM data
//	2 = 8-bit ADPCM data with linear table
//	3 = 4-bit ADPCM data
//	4 = 16-bit ADPCM data with linear table
//	5 = signed 16-bit PCM data
//	6 = unsigned 16-bit PCM data

UINT CSoundFile::ReadSample(MODSAMPLE *pSmp, UINT nFlags, LPCSTR lpMemFile, DWORD dwMemLength, const WORD format)
//-----------------------------------------------------------------------------------------------------------------------
{
	if ((!pSmp) || (pSmp->nLength < 2) || (!lpMemFile)) return 0;

	if(pSmp->nLength > MAX_SAMPLE_LENGTH)
		pSmp->nLength = MAX_SAMPLE_LENGTH;
	
	UINT len = 0, mem = pSmp->nLength+6;

	pSmp->uFlags &= ~(CHN_16BIT|CHN_STEREO);
	if (nFlags & RSF_16BIT)
	{
		mem *= 2;
		pSmp->uFlags |= CHN_16BIT;
	}
	if (nFlags & RSF_STEREO)
	{
		mem *= 2;
		pSmp->uFlags |= CHN_STEREO;
	}

	if ((pSmp->pSample = AllocateSample(mem)) == NULL)
	{
		pSmp->nLength = 0;
		return 0;
	}

	// Check that allocated memory size is not less than what the modinstrument itself
	// thinks it is.
	if( mem < pSmp->GetSampleSizeInBytes() )
	{
		pSmp->nLength = 0;
		FreeSample(pSmp->pSample);
		pSmp->pSample = nullptr;
		MessageBox(0, str_SampleAllocationError, str_Error, MB_ICONERROR);
		return 0;
	}

	switch(nFlags)
	{
	// 1: 8-bit unsigned PCM data
	case RS_PCM8U:
		{
			len = pSmp->nLength;
			if (len > dwMemLength) len = pSmp->nLength = dwMemLength;
			LPSTR pSample = pSmp->pSample;
			for (UINT j=0; j<len; j++) pSample[j] = (char)(lpMemFile[j] - 0x80); 
		}
		break;

	// 2: 8-bit ADPCM data with linear table
	case RS_PCM8D:
		{
			len = pSmp->nLength;
			if (len > dwMemLength) break;
			LPSTR pSample = pSmp->pSample;
			const char *p = (const char *)lpMemFile;
			int delta = 0;
			for (UINT j=0; j<len; j++)
			{
				delta += p[j];
				*pSample++ = (char)delta;
			}
		}
		break;

	// 3: 4-bit ADPCM data
	case RS_ADPCM4:
		{
			len = (pSmp->nLength + 1) / 2;
			if (len > dwMemLength - 16) break;
			memcpy(CompressionTable, lpMemFile, 16);
			lpMemFile += 16;
			LPSTR pSample = pSmp->pSample;
			char delta = 0;
			for (UINT j=0; j<len; j++)
			{
				BYTE b0 = (BYTE)lpMemFile[j];
				BYTE b1 = (BYTE)(lpMemFile[j] >> 4);
				delta = (char)GetDeltaValue((int)delta, b0);
				pSample[0] = delta;
				delta = (char)GetDeltaValue((int)delta, b1);
				pSample[1] = delta;
				pSample += 2;
			}
			len += 16;
		}
		break;

	// 4: 16-bit ADPCM data with linear table
	case RS_PCM16D:
		{
			len = pSmp->nLength * 2;
			if (len > dwMemLength) break;
			short int *pSample = (short int *)pSmp->pSample;
			short int *p = (short int *)lpMemFile;
			int delta16 = 0;
			for (UINT j=0; j<len; j+=2)
			{
				delta16 += *p++;
				*pSample++ = (short int)delta16;
			}
		}
		break;

	// 5: 16-bit signed PCM data
	case RS_PCM16S:
		len = pSmp->nLength * 2;
		if (len <= dwMemLength) memcpy(pSmp->pSample, lpMemFile, len);
		break;

	// 16-bit signed mono PCM motorola byte order
	case RS_PCM16M:
		len = pSmp->nLength * 2;
		if (len > dwMemLength) len = dwMemLength & ~1;
		if (len > 1)
		{
			char *pSample = (char *)pSmp->pSample;
			char *pSrc = (char *)lpMemFile;
			for (UINT j=0; j<len; j+=2)
			{
				pSample[j] = pSrc[j+1];
				pSample[j+1] = pSrc[j];
			}
		}
		break;

	// 6: 16-bit unsigned PCM data
	case RS_PCM16U:
		{
			len = pSmp->nLength * 2;
			if (len > dwMemLength) break;
			short int *pSample = (short int *)pSmp->pSample;
			short int *pSrc = (short int *)lpMemFile;
			for (UINT j=0; j<len; j+=2) *pSample++ = (*(pSrc++)) - 0x8000;
		}
		break;

	// 16-bit signed stereo big endian
	case RS_STPCM16M:
		len = pSmp->nLength * 2;
		if (len*2 <= dwMemLength)
		{
			char *pSample = (char *)pSmp->pSample;
			char *pSrc = (char *)lpMemFile;
			for (UINT j=0; j<len; j+=2)
			{
				pSample[j*2] = pSrc[j+1];
				pSample[j*2+1] = pSrc[j];
				pSample[j*2+2] = pSrc[j+1+len];
				pSample[j*2+3] = pSrc[j+len];
			}
			len *= 2;
		}
		break;

	// 8-bit stereo samples
	case RS_STPCM8S:
	case RS_STPCM8U:
	case RS_STPCM8D:
		{
			len = pSmp->nLength;
			char *psrc = (char *)lpMemFile;
			char *pSample = (char *)pSmp->pSample;
			if (len*2 > dwMemLength) break;
			for (UINT c=0; c<2; c++)
			{
				int iadd = 0;
				if (nFlags == RS_STPCM8U) iadd = -128;
				if (nFlags == RS_STPCM8D)
				{
					for (UINT j=0; j<len; j++)
					{
						iadd += psrc[0];
						psrc++;
						pSample[j*2] = (char)iadd;
					}
				} else
				{
					for (UINT j=0; j<len; j++)
					{
						pSample[j*2] = (char)(psrc[0] + iadd);
						psrc++;
					}
				}
				pSample++;
			}
			len *= 2;
		}
		break;

	// 16-bit stereo samples
	case RS_STPCM16S:
	case RS_STPCM16U:
	case RS_STPCM16D:
		{
			len = pSmp->nLength;
			short int *psrc = (short int *)lpMemFile;
			short int *pSample = (short int *)pSmp->pSample;
			if (len*4 > dwMemLength) break;
			for (UINT c=0; c<2; c++)
			{
				int iadd = 0;
				if (nFlags == RS_STPCM16U) iadd = -0x8000;
				if (nFlags == RS_STPCM16D)
				{
					for (UINT j=0; j<len; j++)
					{
						iadd += psrc[0];
						psrc++;
						pSample[j*2] = (short int)iadd;
					}
				} else
				{
					for (UINT j=0; j<len; j++)
					{
						pSample[j*2] = (short int) (psrc[0] + iadd);
						psrc++;
					}
				}
				pSample++;
			}
			len *= 4;
		}
		break;

	// IT 2.14 compressed samples
	case RS_IT2148:
	case RS_IT21416:
	case RS_IT2158:
	case RS_IT21516:
		len = dwMemLength;
		if (len < 4) break;
		if ((nFlags == RS_IT2148) || (nFlags == RS_IT2158))
			ITUnpack8Bit(pSmp->pSample, pSmp->nLength, (LPBYTE)lpMemFile, dwMemLength, (nFlags == RS_IT2158));
		else
			ITUnpack16Bit(pSmp->pSample, pSmp->nLength, (LPBYTE)lpMemFile, dwMemLength, (nFlags == RS_IT21516));
		break;

#ifndef MODPLUG_BASIC_SUPPORT
#ifndef FASTSOUNDLIB
	// 8-bit interleaved stereo samples
	case RS_STIPCM8S:
	case RS_STIPCM8U:
		{
			int iadd = 0;
			if (nFlags == RS_STIPCM8U) { iadd = -0x80; }
			len = pSmp->nLength;
			if (len*2 > dwMemLength) len = dwMemLength >> 1;
			LPBYTE psrc = (LPBYTE)lpMemFile;
			LPBYTE pSample = (LPBYTE)pSmp->pSample;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (char)(psrc[0] + iadd);
				pSample[j*2+1] = (char)(psrc[1] + iadd);
				psrc+=2;
			}
			len *= 2;
		}
		break;

	// 16-bit interleaved stereo samples
	case RS_STIPCM16S:
	case RS_STIPCM16U:
		{
			int iadd = 0;
			if (nFlags == RS_STIPCM16U) iadd = -32768;
			len = pSmp->nLength;
			if (len*4 > dwMemLength) len = dwMemLength >> 2;
			short int *psrc = (short int *)lpMemFile;
			short int *pSample = (short int *)pSmp->pSample;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (short int)(psrc[0] + iadd);
				pSample[j*2+1] = (short int)(psrc[1] + iadd);
				psrc += 2;
			}
			len *= 4;
		}
		break;

	// AMS compressed samples
	case RS_AMS8:
	case RS_AMS16:
		len = 9;
		if (dwMemLength > 9)
		{
			const char *psrc = lpMemFile;
			char packcharacter = lpMemFile[8], *pdest = (char *)pSmp->pSample;
			len += *((LPDWORD)(lpMemFile+4));
			if (len > dwMemLength) len = dwMemLength;
			UINT dmax = pSmp->nLength;
			if (pSmp->uFlags & CHN_16BIT) dmax <<= 1;
			AMSUnpack(psrc+9, len-9, pdest, dmax, packcharacter);
		}
		break;

	// PTM 8bit delta to 16-bit sample
	case RS_PTM8DTO16:
		{
			len = pSmp->nLength * 2;
			if (len > dwMemLength) break;
			signed char *pSample = (signed char *)pSmp->pSample;
			signed char delta8 = 0;
			for (UINT j=0; j<len; j++)
			{
				delta8 += lpMemFile[j];
				*pSample++ = delta8;
			}
		}
		break;

	// Huffman MDL compressed samples
	case RS_MDL8:
	case RS_MDL16:
		len = dwMemLength;
		if (len >= 4)
		{
			LPBYTE pSample = (LPBYTE)pSmp->pSample;
			LPBYTE ibuf = (LPBYTE)lpMemFile;
			DWORD bitbuf = *((DWORD *)ibuf);
			UINT bitnum = 32;
			BYTE dlt = 0, lowbyte = 0;
			ibuf += 4;
			for (UINT j=0; j<pSmp->nLength; j++)
			{
				BYTE hibyte;
				BYTE sign;
				if (nFlags == RS_MDL16) lowbyte = (BYTE)MDLReadBits(bitbuf, bitnum, ibuf, 8);
				sign = (BYTE)MDLReadBits(bitbuf, bitnum, ibuf, 1);
				if (MDLReadBits(bitbuf, bitnum, ibuf, 1))
				{
					hibyte = (BYTE)MDLReadBits(bitbuf, bitnum, ibuf, 3);
				} else
				{
					hibyte = 8;
					while (!MDLReadBits(bitbuf, bitnum, ibuf, 1)) hibyte += 0x10;
					hibyte += MDLReadBits(bitbuf, bitnum, ibuf, 4);
				}
				if (sign) hibyte = ~hibyte;
				dlt += hibyte;
				if (nFlags != RS_MDL16)
					pSample[j] = dlt;
				else
				{
					pSample[j<<1] = lowbyte;
					pSample[(j<<1)+1] = dlt;
				}
			}
		}
		break;

	case RS_DMF8:
	case RS_DMF16:
		len = dwMemLength;
		if (len >= 4)
		{
			UINT maxlen = pSmp->nLength;
			if (pSmp->uFlags & CHN_16BIT) maxlen <<= 1;
			LPBYTE ibuf = (LPBYTE)lpMemFile, ibufmax = (LPBYTE)(lpMemFile+dwMemLength);
			len = DMFUnpack((LPBYTE)pSmp->pSample, ibuf, ibufmax, maxlen);
		}
		break;

#ifdef MODPLUG_TRACKER
	// Mono PCM 24/32-bit signed & 32 bit float -> load sample, and normalize it to 16-bit
	case RS_PCM24S:
	case RS_PCM32S:
		len = pSmp->nLength * 3;
		if (nFlags == RS_PCM32S) len += pSmp->nLength;
		if (len > dwMemLength) break;
		if (len > 4*8)
		{
			if(nFlags == RS_PCM24S)
			{
				char* pSrc = (char*)lpMemFile;
				char* pDest = (char*)pSmp->pSample;
				CopyWavBuffer<3, 2, WavSigned24To16, MaxFinderSignedInt<3> >(pSrc, len, pDest, pSmp->GetSampleSizeInBytes());
			}
			else //RS_PCM32S
			{
				char* pSrc = (char*)lpMemFile;
				char* pDest = (char*)pSmp->pSample;
				if(format == 3)
					CopyWavBuffer<4, 2, WavFloat32To16, MaxFinderFloat32>(pSrc, len, pDest, pSmp->GetSampleSizeInBytes());
				else
					CopyWavBuffer<4, 2, WavSigned32To16, MaxFinderSignedInt<4> >(pSrc, len, pDest, pSmp->GetSampleSizeInBytes());
			}
		}
		break;

	// Stereo PCM 24/32-bit signed & 32 bit float -> convert sample to 16 bit
	case RS_STIPCM24S:
	case RS_STIPCM32S:
		if(format == 3 && nFlags == RS_STIPCM32S) //Microsoft IEEE float
		{
			len = pSmp->nLength * 6;
			//pSmp->nLength tells(?) the number of frames there
			//are. One 'frame' of 1 byte(== 8 bit) mono data requires
			//1 byte of space, while one frame of 3 byte(24 bit) 
			//stereo data requires 3*2 = 6 bytes. This is(?)
			//why there is factor 6.

			len += pSmp->nLength * 2;
			//Compared to 24 stereo, 32 bit stereo needs 16 bits(== 2 bytes)
			//more per frame.

			if(len > dwMemLength) break;
			char* pSrc = (char*)lpMemFile;
			char* pDest = (char*)pSmp->pSample;
			if (len > 8*8)
			{
				CopyWavBuffer<4, 2, WavFloat32To16, MaxFinderFloat32>(pSrc, len, pDest, pSmp->GetSampleSizeInBytes());
			}
		}
		else
		{
			len = pSmp->nLength * 6;
			if (nFlags == RS_STIPCM32S) len += pSmp->nLength * 2;
			if (len > dwMemLength) break;
			if (len > 8*8)
			{
				char* pSrc = (char*)lpMemFile;
				char* pDest = (char*)pSmp->pSample;
				if(nFlags == RS_STIPCM32S)
				{
					CopyWavBuffer<4,2,WavSigned32To16, MaxFinderSignedInt<4> >(pSrc, len, pDest, pSmp->GetSampleSizeInBytes());
				}
				if(nFlags == RS_STIPCM24S)
				{
					CopyWavBuffer<3,2,WavSigned24To16, MaxFinderSignedInt<3> >(pSrc, len, pDest, pSmp->GetSampleSizeInBytes());
				}

			}
		}
		break;

	// 16-bit signed big endian interleaved stereo
	case RS_STIPCM16M:
		{
			len = pSmp->nLength;
			if (len*4 > dwMemLength) len = dwMemLength >> 2;
			LPCBYTE psrc = (LPCBYTE)lpMemFile;
			short int *pSample = (short int *)pSmp->pSample;
			for (UINT j=0; j<len; j++)
			{
				pSample[j*2] = (signed short)(((UINT)psrc[0] << 8) | (psrc[1]));
				pSample[j*2+1] = (signed short)(((UINT)psrc[2] << 8) | (psrc[3]));
				psrc += 4;
			}
			len *= 4;
		}
		break;

#endif // MODPLUG_TRACKER
#endif // !FASTSOUNDLIB
#endif // !MODPLUG_BASIC_SUPPORT

	// Default: 8-bit signed PCM data
	default:
		len = pSmp->nLength;
		if (len > dwMemLength) len = pSmp->nLength = dwMemLength;
		memcpy(pSmp->pSample, lpMemFile, len);
	}
	if (len > dwMemLength)
	{
		if (pSmp->pSample)
		{
			pSmp->nLength = 0;
			FreeSample(pSmp->pSample);
			pSmp->pSample = nullptr;
		}
		return 0;
	}
	AdjustSampleLoop(pSmp);
	return len;
}


void CSoundFile::AdjustSampleLoop(MODSAMPLE *pSmp)
//----------------------------------------------------
{
	if ((!pSmp->pSample) || (!pSmp->nLength)) return;
	if (pSmp->nLoopEnd > pSmp->nLength) pSmp->nLoopEnd = pSmp->nLength;
	if (pSmp->nLoopStart >= pSmp->nLoopEnd)
	{
		pSmp->nLoopStart = pSmp->nLoopEnd = 0;
		pSmp->uFlags &= ~CHN_LOOP;
	}
	UINT len = pSmp->nLength;
	if (pSmp->uFlags & CHN_16BIT)
	{
		short int *pSample = (short int *)pSmp->pSample;
		// Adjust end of sample
		if (pSmp->uFlags & CHN_STEREO)
		{
			pSample[len*2+6] = pSample[len*2+4] = pSample[len*2+2] = pSample[len*2] = pSample[len*2-2];
			pSample[len*2+7] = pSample[len*2+5] = pSample[len*2+3] = pSample[len*2+1] = pSample[len*2-1];
		} else
		{
			pSample[len+4] = pSample[len+3] = pSample[len+2] = pSample[len+1] = pSample[len] = pSample[len-1];
		}
		if ((pSmp->uFlags & (CHN_LOOP|CHN_PINGPONGLOOP|CHN_STEREO)) == CHN_LOOP)
		{
			// Fix bad loops
			if ((pSmp->nLoopEnd+3 >= pSmp->nLength) || (m_nType & MOD_TYPE_S3M))
			{
				pSample[pSmp->nLoopEnd] = pSample[pSmp->nLoopStart];
				pSample[pSmp->nLoopEnd+1] = pSample[pSmp->nLoopStart+1];
				pSample[pSmp->nLoopEnd+2] = pSample[pSmp->nLoopStart+2];
				pSample[pSmp->nLoopEnd+3] = pSample[pSmp->nLoopStart+3];
				pSample[pSmp->nLoopEnd+4] = pSample[pSmp->nLoopStart+4];
			}
		}
	} else
	{
		LPSTR pSample = pSmp->pSample;
#ifndef FASTSOUNDLIB
		// Crappy samples (except chiptunes) ?
		if ((pSmp->nLength > 0x100) && (m_nType & (MOD_TYPE_MOD|MOD_TYPE_S3M))
		 && (!(pSmp->uFlags & CHN_STEREO)))
		{
			int smpend = pSample[pSmp->nLength-1], smpfix = 0, kscan;
			for (kscan=pSmp->nLength-1; kscan>0; kscan--)
			{
				smpfix = pSample[kscan-1];
				if (smpfix != smpend) break;
			}
			int delta = smpfix - smpend;
			if (((!(pSmp->uFlags & CHN_LOOP)) || (kscan > (int)pSmp->nLoopEnd))
			 && ((delta < -8) || (delta > 8)))
			{
				while (kscan<(int)pSmp->nLength)
				{
					if (!(kscan & 7))
					{
						if (smpfix > 0) smpfix--;
						if (smpfix < 0) smpfix++;
					}
					pSample[kscan] = (char)smpfix;
					kscan++;
				}
			}
		}
#endif
		// Adjust end of sample
		if (pSmp->uFlags & CHN_STEREO)
		{
			pSample[len*2+6] = pSample[len*2+4] = pSample[len*2+2] = pSample[len*2] = pSample[len*2-2];
			pSample[len*2+7] = pSample[len*2+5] = pSample[len*2+3] = pSample[len*2+1] = pSample[len*2-1];
		} else
		{
			pSample[len+4] = pSample[len+3] = pSample[len+2] = pSample[len+1] = pSample[len] = pSample[len-1];
		}
		if ((pSmp->uFlags & (CHN_LOOP|CHN_PINGPONGLOOP|CHN_STEREO)) == CHN_LOOP)
		{
			if ((pSmp->nLoopEnd+3 >= pSmp->nLength) || (m_nType & (MOD_TYPE_MOD|MOD_TYPE_S3M)))
			{
				pSample[pSmp->nLoopEnd] = pSample[pSmp->nLoopStart];
				pSample[pSmp->nLoopEnd+1] = pSample[pSmp->nLoopStart+1];
				pSample[pSmp->nLoopEnd+2] = pSample[pSmp->nLoopStart+2];
				pSample[pSmp->nLoopEnd+3] = pSample[pSmp->nLoopStart+3];
				pSample[pSmp->nLoopEnd+4] = pSample[pSmp->nLoopStart+4];
			}
		}
	}
}


/////////////////////////////////////////////////////////////
// Transpose <-> Frequency conversions

// returns 8363*2^((transp*128+ftune)/(12*128))
DWORD CSoundFile::TransposeToFrequency(int transp, int ftune)
//-----------------------------------------------------------
{
	const float _fbase = 8363;
	const float _factor = 1.0f/(12.0f*128.0f);
	int result;
	DWORD freq;

	transp = (transp << 7) + ftune;
	_asm {
	fild transp
	fld _factor
	fmulp st(1), st(0)
	fist result
	fisub result
	f2xm1
	fild result
	fld _fbase
	fscale
	fstp st(1)
	fmul st(1), st(0)
	faddp st(1), st(0)
	fistp freq
	}
	UINT derr = freq % 11025;
	if (derr <= 8) freq -= derr;
	if (derr >= 11015) freq += 11025-derr;
	derr = freq % 1000;
	if (derr <= 5) freq -= derr;
	if (derr >= 995) freq += 1000-derr;
	return freq;
}


// returns 12*128*log2(freq/8363)
int CSoundFile::FrequencyToTranspose(DWORD freq)
//----------------------------------------------
{
	const float _f1_8363 = 1.0f / 8363.0f;
	const float _factor = 128 * 12;
	LONG result;
	
	if (!freq) return 0;
	_asm {
	fld _factor
	fild freq
	fld _f1_8363
	fmulp st(1), st(0)
	fyl2x
	fistp result
	}
	return result;
}


void CSoundFile::FrequencyToTranspose(MODSAMPLE *psmp)
//--------------------------------------------------------
{
	int f2t = FrequencyToTranspose(psmp->nC5Speed);
	int transp = f2t >> 7;
	int ftune = f2t & 0x7F; //0x7F == 111 1111
	if (ftune > 80)
	{
		transp++;
		ftune -= 128;
	}
	if (transp > 127) transp = 127;
	if (transp < -127) transp = -127;
	psmp->RelativeTone = transp;
	psmp->nFineTune = ftune;
}


void CSoundFile::CheckCPUUsage(UINT nCPU)
//---------------------------------------
{
	if (nCPU > 100) nCPU = 100;
	gnCPUUsage = nCPU;
	if (nCPU < 90)
	{
		m_dwSongFlags &= ~SONG_CPUVERYHIGH;
	} else
	if ((m_dwSongFlags & SONG_CPUVERYHIGH) && (nCPU >= 94))
	{
		UINT i=MAX_CHANNELS;
		while (i >= 8)
		{
			i--;
			if (Chn[i].nLength)
			{
				Chn[i].nLength = Chn[i].nPos = 0;
				nCPU -= 2;
				if (nCPU < 94) break;
			}
		}
	} else
	if (nCPU > 90)
	{
		m_dwSongFlags |= SONG_CPUVERYHIGH;
	}
}


BOOL CSoundFile::SetPatternName(PATTERNINDEX  nPat, LPCSTR lpszName)
//------------------------------------------------------------------
{
	CHAR szName[MAX_PATTERNNAME] = "";
	if (nPat >= Patterns.Size()) return FALSE;
	if (lpszName) lstrcpyn(szName, lpszName, MAX_PATTERNNAME);
	SpaceToNullString(szName); //szName[MAX_PATTERNNAME-1] = 0;
	if (!m_lpszPatternNames) m_nPatternNames = 0;
	if (nPat >= m_nPatternNames)
	{
		//if (!lpszName[0]) return TRUE;
		UINT len = (nPat+1)*MAX_PATTERNNAME;
		CHAR *p = new CHAR[len];
		if (!p) return FALSE;
		memset(p, 0, len);
		if (m_lpszPatternNames)
		{
			memcpy(p, m_lpszPatternNames, m_nPatternNames * MAX_PATTERNNAME);
			delete[] m_lpszPatternNames;
			m_lpszPatternNames = NULL;
		}
		m_lpszPatternNames = p;
		m_nPatternNames = nPat + 1;
	}
	memcpy(m_lpszPatternNames + nPat * MAX_PATTERNNAME, szName, MAX_PATTERNNAME);
	return TRUE;
}


BOOL CSoundFile::GetPatternName(PATTERNINDEX nPat, LPSTR lpszName, UINT cbSize) const
//-----------------------------------------------------------------------------------
{
	if ((!lpszName) || (!cbSize)) return FALSE;
	lpszName[0] = 0;
	if (cbSize > MAX_PATTERNNAME) cbSize = MAX_PATTERNNAME;
	if ((m_lpszPatternNames) && (nPat < m_nPatternNames))
	{
		memcpy(lpszName, m_lpszPatternNames + nPat * MAX_PATTERNNAME, cbSize);
		lpszName[cbSize-1] = 0;
		return TRUE;
	}
	return FALSE;
}


#ifndef FASTSOUNDLIB

UINT CSoundFile::DetectUnusedSamples(BYTE *pbIns)
//-----------------------------------------------
{
	UINT nExt = 0;

	if (!pbIns) return 0;
	if (m_nInstruments)
	{
		memset(pbIns, 0, (MAX_SAMPLES+7)/8);
		for (UINT ipat=0; ipat<Patterns.Size(); ipat++)
		{
			MODCOMMAND *p = Patterns[ipat];
			if (p)
			{
				UINT jmax = PatternSize[ipat] * m_nChannels;
				for (UINT j=0; j<jmax; j++, p++)
				{
					if ((p->note) && (p->note <= NOTE_MAX))
					{
						if ((p->instr) && (p->instr < MAX_INSTRUMENTS))
						{
							MODINSTRUMENT *pIns = Instruments[p->instr];
							if (pIns)
							{
								UINT n = pIns->Keyboard[p->note-1];
								if (n < MAX_SAMPLES) pbIns[n>>3] |= 1<<(n&7);
							}
						} else
						{
							for (UINT k=1; k<=m_nInstruments; k++)
							{
								MODINSTRUMENT *pIns = Instruments[k];
								if (pIns)
								{
									UINT n = pIns->Keyboard[p->note-1];
									if (n < MAX_SAMPLES) pbIns[n>>3] |= 1<<(n&7);
								}
							}
						}
					}
				}
			}
		}
		for (UINT ichk=1; ichk<=m_nSamples; ichk++)
		{
			if ((0 == (pbIns[ichk>>3]&(1<<(ichk&7)))) && (Samples[ichk].pSample)) nExt++;
		}
	}
	return nExt;
}


bool CSoundFile::RemoveSelectedSamples(bool *pbIns)
//-------------------------------------------------
{
	if (!pbIns) return false;
	for (SAMPLEINDEX nSmp=1; nSmp<MAX_SAMPLES; nSmp++)
	{
		if ((!pbIns[nSmp]) && (Samples[nSmp].pSample))
		{
			DestroySample(nSmp);
			if ((nSmp == m_nSamples) && (nSmp > 1)) m_nSamples--;
		}
	}
	return true;
}


bool CSoundFile::DestroySample(SAMPLEINDEX nSample)
//-------------------------------------------------
{
	if ((!nSample) || (nSample >= MAX_SAMPLES)) return false;
	if (!Samples[nSample].pSample) return true;
	MODSAMPLE *pSmp = &Samples[nSample];
	LPSTR pSample = pSmp->pSample;
	pSmp->pSample = nullptr;
	pSmp->nLength = 0;
	pSmp->uFlags &= ~(CHN_16BIT);
	for (UINT i=0; i<MAX_CHANNELS; i++)
	{
		if (Chn[i].pSample == pSample)
		{
			Chn[i].nPos = Chn[i].nLength = 0;
			Chn[i].pSample = Chn[i].pCurrentSample = NULL;
		}
	}
	FreeSample(pSample);
	return true;
}

// -> CODE#0020
// -> DESC="rearrange sample list"
bool CSoundFile::MoveSample(SAMPLEINDEX from, SAMPLEINDEX to)
//-----------------------------------------------------------
{
	if (!from || from >= MAX_SAMPLES || !to || to >= MAX_SAMPLES) return false;
	if (/*!Ins[from].pSample ||*/ Samples[to].pSample) return true;

	MODSAMPLE *pinsf = &Samples[from];
	MODSAMPLE *pinst = &Samples[to];

	memcpy(pinst, pinsf, sizeof(MODSAMPLE));

	pinsf->pSample = nullptr;
	pinsf->nLength = 0;
	pinsf->uFlags &= ~(CHN_16BIT);

	return true;
}
// -! NEW_FEATURE#0020
#endif // FASTSOUNDLIB

//rewbs.plugDocAware
/*PSNDMIXPLUGIN CSoundFile::GetSndPlugMixPlug(IMixPlugin *pPlugin) 
{
	for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++)
	{
		if (m_MixPlugins[iPlug].pMixPlugin == pPlugin)
			return &(m_MixPlugins[iPlug]);
	}
	
	return NULL;
}*/
//end rewbs.plugDocAware


void CSoundFile::BuildDefaultInstrument() 
//---------------------------------------
{
// m_defaultInstrument is currently only used to get default values for extented properties. 
// In the future we can make better use of this.
	memset(&m_defaultInstrument, 0, sizeof(MODINSTRUMENT));
	m_defaultInstrument.nResampling = SRCMODE_DEFAULT;
	m_defaultInstrument.nFilterMode = FLTMODE_UNCHANGED;
	m_defaultInstrument.nPPC = 5*12;
	m_defaultInstrument.nGlobalVol=64;
	m_defaultInstrument.nPan = 0x20 << 2;
	m_defaultInstrument.nIFC = 0xFF;
	m_defaultInstrument.PanEnv.nReleaseNode=ENV_RELEASE_NODE_UNSET;
	m_defaultInstrument.PitchEnv.nReleaseNode=ENV_RELEASE_NODE_UNSET;
	m_defaultInstrument.VolEnv.nReleaseNode=ENV_RELEASE_NODE_UNSET;
	m_defaultInstrument.wPitchToTempoLock = 0;
	m_defaultInstrument.pTuning = m_defaultInstrument.s_DefaultTuning;
	m_defaultInstrument.nPluginVelocityHandling = PLUGIN_VELOCITYHANDLING_CHANNEL;
	m_defaultInstrument.nPluginVolumeHandling = CSoundFile::s_DefaultPlugVolumeHandling;
}


void CSoundFile::DeleteStaticdata()
//---------------------------------
{
	delete s_pTuningsSharedLocal; s_pTuningsSharedLocal = 0;
	delete s_pTuningsSharedBuiltIn; s_pTuningsSharedBuiltIn = 0;
}

bool CSoundFile::SaveStaticTunings()
//----------------------------------
{
	if(s_pTuningsSharedLocal->Serialize())
	{
		ErrorBox(IDS_ERR_TUNING_SERIALISATION, NULL);
		return true;
	}
	return false;
}

void SimpleMessageBox(const char* message, const char* title)
//-----------------------------------------------------------
{
	MessageBox(0, message, title, MB_ICONINFORMATION);
}

bool CSoundFile::LoadStaticTunings()
//-----------------------------------
{
	if(s_pTuningsSharedLocal || s_pTuningsSharedBuiltIn) return true;
	//For now not allowing to reload tunings(one should be careful when reloading them
	//since various parts may use addresses of the tuningobjects).

	CTuning::MessageHandler = &SimpleMessageBox;

	s_pTuningsSharedBuiltIn = new CTuningCollection;
	s_pTuningsSharedLocal = new CTuningCollection("Local tunings");

	// Load built-in tunings.
	const char* pData = nullptr;
	HGLOBAL hglob = nullptr;
	size_t nSize = 0;
	if (LoadResource(MAKEINTRESOURCE(IDR_BUILTIN_TUNINGS), TEXT("TUNING"), pData, nSize, hglob) != nullptr)
	{
		std::istrstream iStrm(pData, nSize);
		s_pTuningsSharedBuiltIn->Deserialize(iStrm);
		FreeResource(hglob);
	}
	if(s_pTuningsSharedBuiltIn->GetNumTunings() == 0)
	{
		ASSERT(false);
		CTuningRTI* pT = new CTuningRTI;
		//Note: Tuning collection class handles deleting.
		pT->CreateGeometric(1,1);
		if(s_pTuningsSharedBuiltIn->AddTuning(pT))
			delete pT;
	}
		
	// Load local tunings.
	CString sPath;
	sPath.Format(TEXT("%slocal_tunings%s"), CMainFrame::GetDefaultDirectory(DIR_TUNING), CTuningCollection::s_FileExtension);
	s_pTuningsSharedLocal->SetSavefilePath(sPath);
	s_pTuningsSharedLocal->Deserialize();

	// Enabling adding/removing of tunings for standard collection
	// only for debug builds.
	#ifdef DEBUG
		s_pTuningsSharedBuiltIn->SetConstStatus(CTuningCollection::EM_ALLOWALL);
	#else
		s_pTuningsSharedBuiltIn->SetConstStatus(CTuningCollection::EM_CONST);
	#endif

	MODINSTRUMENT::s_DefaultTuning = NULL;

	return false;
}



void CSoundFile::SetDefaultInstrumentValues(MODINSTRUMENT *pIns) 
//-----------------------------------------------------------------
{
	pIns->nResampling = m_defaultInstrument.nResampling;
	pIns->nFilterMode = m_defaultInstrument.nFilterMode;
	pIns->PitchEnv.nReleaseNode = m_defaultInstrument.PitchEnv.nReleaseNode;
	pIns->PanEnv.nReleaseNode = m_defaultInstrument.PanEnv.nReleaseNode;
	pIns->VolEnv.nReleaseNode = m_defaultInstrument.VolEnv.nReleaseNode;
	pIns->pTuning = m_defaultInstrument.pTuning;
	pIns->nPluginVelocityHandling = m_defaultInstrument.nPluginVelocityHandling;
	pIns->nPluginVolumeHandling = m_defaultInstrument.nPluginVolumeHandling;

}



long CSoundFile::GetSampleOffset() 
//-------------------------------
{
	//TODO: This is where we could inform patterns of the exact song position when playback starts.
	//order: m_nNextPattern
	//long ticksFromStartOfPattern = m_nRow*m_nMusicSpeed;
	//return ticksFromStartOfPattern*m_nSamplesPerTick;
	return 0;
}

string CSoundFile::GetNoteName(const CTuning::NOTEINDEXTYPE& note, const int inst) const
//----------------------------------------------------------------------------------
{
	if(inst >= MAX_INSTRUMENTS || inst < -1 || note < 1 || note > NOTE_MAX) return "BUG";
	if(inst == -1)
		return szDefaultNoteNames[note-1];
	
	if(m_nType == MOD_TYPE_MPT && Instruments[inst] && Instruments[inst]->pTuning)
		return Instruments[inst]->pTuning->GetNoteName(note-NOTE_MIDDLEC);
	else
		return szDefaultNoteNames[note-1];
}


void CSoundFile::SetModSpecsPointer(const CModSpecifications*& pModSpecs, const MODTYPE type)
//------------------------------------------------------------------------------------------
{
	switch(type)
	{
		case MOD_TYPE_MPT:
			pModSpecs = &ModSpecs::mptm;
		break;

		case MOD_TYPE_IT:
			pModSpecs = &ModSpecs::itEx;
		break;

		case MOD_TYPE_XM:
			pModSpecs = &ModSpecs::xmEx;
		break;

		case MOD_TYPE_S3M:
			pModSpecs = &ModSpecs::s3mEx;
		break;

		case MOD_TYPE_MOD:
		default:
			pModSpecs = &ModSpecs::modEx;
			break;
	}
}

uint16 CSoundFile::GetModFlagMask(const MODTYPE oldtype, const MODTYPE newtype) const
//-----------------------------------------------------------------------------------
{
	const MODTYPE combined = oldtype | newtype;

	// XM <-> IT/MPT conversion.
	if(combined == (MOD_TYPE_IT|MOD_TYPE_XM) || combined == (MOD_TYPE_MPT|MOD_TYPE_XM))
		return (1 << MSF_COMPATIBLE_PLAY) + (1 << MSF_MIDICC_BUGEMULATION);

	// IT <-> MPT conversion.
	if(combined == (MOD_TYPE_IT|MOD_TYPE_MPT))
		return uint16_max;

	return 0;
}

void CSoundFile::ChangeModTypeTo(const MODTYPE& newType)
//---------------------------------------------------
{
	const MODTYPE oldtype = m_nType;
	m_nType = newType;
	SetModSpecsPointer(m_pModSpecs, m_nType);
	SetupMODPanning(); // Setup LRRL panning scheme if needed

	m_ModFlags = m_ModFlags & GetModFlagMask(oldtype, newType);

	Order.OnModTypeChanged(oldtype);
	Patterns.OnModTypeChanged(oldtype);
}


bool CSoundFile::SetTitle(const char* titleCandidate, size_t strSize)
//-------------------------------------------------------------------
{
	if(strcmp(m_szNames[0], titleCandidate))
	{
		memset(m_szNames[0], 0, sizeof(m_szNames[0]));
		memcpy(m_szNames[0], titleCandidate, min(sizeof(m_szNames[0])-1, strSize));
		return true;
	}
	return false;
}

double CSoundFile::GetPlaybackTimeAt(ORDERINDEX ord, ROWINDEX row)
//----------------------------------------------------------------
{
	bool targetReached = false;
	const double t = GetLength(targetReached, FALSE, TRUE, ord, row);
	if(targetReached) return t;
	else return -1; //Given position not found from play sequence.
}


const CModSpecifications& CSoundFile::GetModSpecifications(const MODTYPE type)
//----------------------------------------------------------------------------
{
	const CModSpecifications* p = 0;
	SetModSpecsPointer(p, type);
	return *p;
}

/* Try to write an (volume) effect in a given channel or any channel of a pattern in a specific row.
   Usage: nPat - Pattern that should be modified
          nRow - Row that should be modified
		  nEffect - (Volume) Effect that should be written
		  nParam - Effect that should be written
          bIsVolumeEffect  - Indicates whether the given effect is a volume effect or not
		  nChn - Channel that should be modified - use CHANNELINDEX_INVALID to allow all channels of the given row
		  bAllowMultipleEffects - If false, No effect will be written if an effect of the same type is already present in the channel(s)
		  bAllowNextRow - Indicates whether it is allowed to use the next row if there's no space for the effect
		  bRetry - For internal use only. Indicates whether an effect "rewrite" has already taken place (for recursive calls)
*/ 
bool CSoundFile::TryWriteEffect(PATTERNINDEX nPat, ROWINDEX nRow, BYTE nEffect, BYTE nParam, bool bIsVolumeEffect, CHANNELINDEX nChn, bool bAllowMultipleEffects, bool bAllowNextRow, bool bRetry)
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{
	// NOTE: Effect remapping is only implemented for a few basic effects.
	CHANNELINDEX nScanChnMin = nChn, nScanChnMax = nChn;
	MODCOMMAND *p = Patterns[nPat], *m;

	// Scan all channels
	if(nChn == CHANNELINDEX_INVALID)
	{
		nScanChnMin = 0;
		nScanChnMax = m_nChannels - 1;
	}

	// Scan channel(s) for same effect type - if an effect of the same type is already present, exit.
	if(bAllowMultipleEffects == false)
	{
		for(CHANNELINDEX i = nScanChnMin; i <= nScanChnMax; i++)
		{
			m = p + nRow * m_nChannels + i;
			if(!bIsVolumeEffect && m->command == nEffect)
				return true;
			if(bIsVolumeEffect && m->volcmd == nEffect)
				return true;
		}
	}

	// Easy case: check if there's some space left to put the effect somewhere
	for(CHANNELINDEX i = nScanChnMin; i <= nScanChnMax; i++)
	{
		m = p + nRow * m_nChannels + i;
		if(!bIsVolumeEffect && m->command == CMD_NONE)
		{
			m->command = nEffect;
			m->param = nParam;
			return true;
		}
		if(bIsVolumeEffect && m->volcmd == VOLCMD_NONE)
		{
			m->volcmd = nEffect;
			m->vol = nParam;
			return true;
		}
	}

	// Ok, apparently there's no space. If we haven't tried already, try to map it to the volume column or effect column instead.
	if(bRetry == true) {
		// Move some effects that also work in the volume column, so there's place for our new effect.
		if(!bIsVolumeEffect)
		{
			for(CHANNELINDEX i = nScanChnMin; i <= nScanChnMax; i++)
			{
				m = p + nRow * m_nChannels + i;
				switch(m->command)
				{
				case CMD_VOLUME:
					m->volcmd = VOLCMD_VOLUME;
					m->vol = m->param;
					m->command = nEffect;
					m->param = nParam;
					return true;

				case CMD_PANNING8:
					if(m_nType & MOD_TYPE_S3M && nParam > 0x80)
						break;

					m->volcmd = VOLCMD_PANNING;
					m->command = nEffect;

					if(m_nType & MOD_TYPE_S3M)
					{
						m->vol = m->param >> 1;
					}
					else
					{
						m->vol = (m->param >> 2) + 1;
					}

					m->param = nParam;
					return true;
				}
			}
		}

		// Let's try it again by writing into the "other" effect column.
		BYTE nNewEffect = CMD_NONE;
		if(bIsVolumeEffect)
		{
			switch(nEffect)
			{
			case VOLCMD_PANNING:
				nNewEffect = CMD_PANNING8;
				if(m_nType & MOD_TYPE_S3M)
					nParam <<= 1;
				else
					nParam = min(nParam << 2, 0xFF);
				break;
			case VOLCMD_VOLUME:
				nNewEffect = CMD_VOLUME;
				break;
			}
		} else
		{
			switch(nEffect)
			{
			case CMD_PANNING8:
				nNewEffect = VOLCMD_PANNING;
				if(m_nType & MOD_TYPE_S3M)
				{
					if(nParam <= 0x80)
						nParam >>= 1;
					else
						nNewEffect = CMD_NONE;
				}
				else
				{
					nParam = (nParam >> 2) + 1;
				}
				break;
			case CMD_VOLUME:
				nNewEffect = CMD_VOLUME;
				break;
			}
		}
		if(nNewEffect != CMD_NONE)
		{
			if(TryWriteEffect(nPat, nRow, nNewEffect, nParam, !bIsVolumeEffect, nChn, bAllowMultipleEffects, bAllowNextRow, false) == true) return true;
		}
	}

	// Try in the next row if possible (this may also happen if we already retried)
	if(bAllowNextRow && (nRow + 1 < Patterns[nPat].GetNumRows()))
	{
		return TryWriteEffect(nPat, nRow + 1, nEffect, nParam, bIsVolumeEffect, nChn, bAllowMultipleEffects, bAllowNextRow, bRetry);
	}

	return false;
}


void CSoundFile::SetupMODPanning(bool bForceSetup)
//------------------------------------------------
{
	// Setup LRRL panning, max channel volume
	if((m_nType & MOD_TYPE_MOD) == 0 && bForceSetup == false) return;

	for (CHANNELINDEX nChn = 0; nChn < MAX_BASECHANNELS; nChn++)
	{
		ChnSettings[nChn].nVolume = 64;
		if (gdwSoundSetup & SNDMIX_MAXDEFAULTPAN)
			ChnSettings[nChn].nPan = (((nChn & 3) == 1) || ((nChn & 3) == 2)) ? 256 : 0;
		else
			ChnSettings[nChn].nPan = (((nChn & 3) == 1) || ((nChn & 3) == 2)) ? 0xC0 : 0x40;
	}
}

// Convert an Exx command (MOD) to Sxx command (S3M)
void CSoundFile::MODExx2S3MSxx(MODCOMMAND *m)
//-------------------------------------------
{
	if(m->command != CMD_MODCMDEX) return;
	m->command = CMD_S3MCMDEX;
	switch(m->param & 0xF0)
	{
	case 0x10:	m->command = CMD_PORTAMENTOUP; m->param |= 0xF0; break;
	case 0x20:	m->command = CMD_PORTAMENTODOWN; m->param |= 0xF0; break;
	case 0x30:	m->param = (m->param & 0x0F) | 0x10; break;
	case 0x40:	m->param = (m->param & 0x03) | 0x30; break;
	case 0x50:	m->param = (m->param & 0x0F) | 0x20; break;
	case 0x60:	m->param = (m->param & 0x0F) | 0xB0; break;
	case 0x70:	m->param = (m->param & 0x03) | 0x40; break;
	case 0x90:	m->command = CMD_RETRIG; m->param = 0x80 | (m->param & 0x0F); break;
	case 0xA0:	if (m->param & 0x0F) { m->command = CMD_VOLUMESLIDE; m->param = (m->param << 4) | 0x0F; } else m->command = 0; break;
	case 0xB0:	if (m->param & 0x0F) { m->command = CMD_VOLUMESLIDE; m->param |= 0xF0; } else m->command = 0; break;
		// rest are the same
	}
}

// Convert an Sxx command (S3M) to Exx command (MOD)
void CSoundFile::S3MSxx2MODExx(MODCOMMAND *m)
//-------------------------------------------
{
	if(m->command != CMD_S3MCMDEX) return;
	m->command = CMD_MODCMDEX;
	switch(m->param & 0xF0)
	{
	case 0x10:	m->param = (m->param & 0x0F) | 0x30; break;
	case 0x20:	m->param = (m->param & 0x0F) | 0x50; break;
	case 0x30:	m->param = (m->param & 0x0F) | 0x40; break;
	case 0x40:	m->param = (m->param & 0x0F) | 0x70; break;
	case 0x50:	
	case 0x60:	
	case 0x70:  if(((m->param & 0xF0) == 0x70) && ((m->param & 0x0F) > 0x0A)) { m->command = CMD_NONE; break; }	// no pitch env in XM format
	case 0x90:
	case 0xA0:	m->command = CMD_XFINEPORTAUPDOWN; break;
	case 0xB0:	m->param = (m->param & 0x0F) | 0x60; break;
		// rest are the same
	}
}

// Convert a mod command from one format to another. 
void CSoundFile::ConvertCommand(MODCOMMAND *m, MODTYPE nOldType, MODTYPE nNewType)
//--------------------------------------------------------------------------------
{
	// helper variables
	const bool oldTypeIsMOD = (nOldType == MOD_TYPE_MOD), oldTypeIsXM = (nOldType == MOD_TYPE_XM),
		oldTypeIsS3M = (nOldType == MOD_TYPE_S3M), oldTypeIsIT = (nOldType == MOD_TYPE_IT),
		oldTypeIsMPT = (nOldType == MOD_TYPE_MPT), oldTypeIsMOD_XM = (oldTypeIsMOD || oldTypeIsXM),
		oldTypeIsS3M_IT_MPT = (oldTypeIsS3M || oldTypeIsIT || oldTypeIsMPT),
		oldTypeIsIT_MPT = (oldTypeIsIT || oldTypeIsMPT);

	const bool newTypeIsMOD = (nNewType == MOD_TYPE_MOD), newTypeIsXM =  (nNewType == MOD_TYPE_XM), 
		newTypeIsS3M = (nNewType == MOD_TYPE_S3M), newTypeIsIT = (nNewType == MOD_TYPE_IT),
		newTypeIsMPT = (nNewType == MOD_TYPE_MPT), newTypeIsMOD_XM = (newTypeIsMOD || newTypeIsXM), 
		newTypeIsS3M_IT_MPT = (newTypeIsS3M || newTypeIsIT || newTypeIsMPT), 
		newTypeIsIT_MPT = (newTypeIsIT || newTypeIsMPT);

	//////////////////////////
	// Convert 8-bit Panning
	if(m->command == CMD_PANNING8)
	{
		if(newTypeIsS3M)
		{
			m->param = (m->param + 1) >> 1;
		}
		else if(oldTypeIsS3M)
		{
			if(m->param == 0xA4)
			{
				// surround remap
				m->command = (nNewType & (MOD_TYPE_IT|MOD_TYPE_MPT)) ? CMD_S3MCMDEX : CMD_XFINEPORTAUPDOWN;
				m->param = 0x91;
			}
			else
			{
				m->param = min(m->param << 1, 0xFF);
			}
		}
	} // End if(m->command == CMD_PANNING8)

	/////////////////////////////////////////////////////
	// Convert param control, extended envelope control
	if(oldTypeIsMPT)
	{
		if(m->note == NOTE_PC || m->note == NOTE_PCS)
		{
			m->param = (BYTE)(min(MODCOMMAND::maxColumnValue, m->GetValueEffectCol()) * 0x7F / MODCOMMAND::maxColumnValue);
			m->command = (m->note == NOTE_PC) ? CMD_MIDI : CMD_SMOOTHMIDI; // might be removed later
			m->volcmd = VOLCMD_NONE;
			m->note = NOTE_NONE;
		}

		// adjust extended envelope control commands
		if((m->command == CMD_S3MCMDEX) && ((m->param & 0xF0) == 0x70) && ((m->param & 0x0F) > 0x0C))
		{
			m->param = 0x7C;
		}
	} // End if(oldTypeIsMPT)

	/////////////////////////////////////////
	// Convert MOD / XM to S3M / IT / MPTM
	if(oldTypeIsMOD_XM && newTypeIsS3M_IT_MPT)
	{
		switch(m->command)
		{
		case CMD_MODCMDEX:
			MODExx2S3MSxx(m);
			break;
		case CMD_VOLUME:
			if (!m->volcmd)
			{
				m->volcmd = VOLCMD_VOLUME;
				m->vol = m->param;
				if (m->vol > 0x40) m->vol = 0x40;
				m->command = m->param = 0;
			}
			break;
		case CMD_PORTAMENTOUP:
			if (m->param > 0xDF) m->param = 0xDF;
			break;
		case CMD_PORTAMENTODOWN:
			if (m->param > 0xDF) m->param = 0xDF;
			break;
		case CMD_XFINEPORTAUPDOWN:
			switch(m->param & 0xF0)
			{
			case 0x10:	m->command = CMD_PORTAMENTOUP; m->param = (m->param & 0x0F) | 0xE0; break;
			case 0x20:	m->command = CMD_PORTAMENTODOWN; m->param = (m->param & 0x0F) | 0xE0; break;
			case 0x50:
			case 0x60:
			case 0x70:
			case 0x90:
			case 0xA0:
				m->command = CMD_S3MCMDEX;
				// surround remap (this is the "official" command)
				if(nNewType & MOD_TYPE_S3M && m->param == 0x91)
				{
					m->command = CMD_PANNING8;
					m->param = 0xA4;
				}
				break;
			}
			break;
		case CMD_KEYOFF:
			if(m->note == 0)
			{
				m->note = (newTypeIsS3M) ? NOTE_NOTECUT : NOTE_KEYOFF;
				m->command = CMD_S3MCMDEX;
				if(m->param == 0)
					m->instr = 0;
				m->param = 0xD0 | (m->param & 0x0F);
			}
			break;
		case CMD_PANNINGSLIDE:
			// swap L/R
			m->param = ((m->param & 0x0F) << 4) | (m->param >> 4);
		default:
			break;
		}
	} // End if(oldTypeIsMOD_XM && newTypeIsS3M_IT_MPT)


	/////////////////////////////////////////
	// Convert S3M / IT / MPTM to MOD / XM
	else if(oldTypeIsS3M_IT_MPT && newTypeIsMOD_XM)
	{
		// convert note cut/off/fade
		if(m->note == NOTE_NOTECUT || m->note == NOTE_FADE)
			m->note = NOTE_KEYOFF;

		switch(m->command)
		{
		case CMD_S3MCMDEX:
			S3MSxx2MODExx(m);
			break;
		case CMD_VOLUMESLIDE:
			if ((m->param & 0xF0) && ((m->param & 0x0F) == 0x0F))
			{
				m->command = CMD_MODCMDEX;
				m->param = (m->param >> 4) | 0xA0;
			} else
				if ((m->param & 0x0F) && ((m->param & 0xF0) == 0xF0))
				{
					m->command = CMD_MODCMDEX;
					m->param = (m->param & 0x0F) | 0xB0;
				}
				break;
		case CMD_PORTAMENTOUP:
			if (m->param >= 0xF0)
			{
				m->command = CMD_MODCMDEX;
				m->param = (m->param & 0x0F) | 0x10;
			} else
				if (m->param >= 0xE0)
				{
					m->command = CMD_MODCMDEX;
					m->param = (((m->param & 0x0F)+3) >> 2) | 0x10;
				} else m->command = CMD_PORTAMENTOUP;
			break;
		case CMD_PORTAMENTODOWN:
			if (m->param >= 0xF0)
			{
				m->command = CMD_MODCMDEX;
				m->param = (m->param & 0x0F) | 0x20;
			} else
				if (m->param >= 0xE0)
				{
					m->command = CMD_MODCMDEX;
					m->param = (((m->param & 0x0F)+3) >> 2) | 0x20;
				} else m->command = CMD_PORTAMENTODOWN;
			break;
		case CMD_SPEED:
			{
				m->param = min(m->param, (nNewType == MOD_TYPE_XM) ? 0x1F : 0x20);
			}
			break;
		case CMD_TEMPO:
			if(m->param < 0x20) m->command = CMD_NONE; // no tempo slides
			break;
		case CMD_PANNINGSLIDE:
			// swap L/R
			m->param = ((m->param & 0x0F) << 4) | (m->param >> 4);
			// remove fine slides
			if((m->param > 0xF0) || ((m->param & 0x0F) == 0x0F && m->param != 0x0F))
				m->command = CMD_NONE;
		default:
			break;
		}
	} // End if(oldTypeIsS3M_IT_MPT && newTypeIsMOD_XM)


	///////////////////////
	// Convert IT to S3M
	else if (oldTypeIsIT_MPT && newTypeIsS3M)
	{
		if(m->note == NOTE_KEYOFF || m->note == NOTE_FADE)
			m->note = NOTE_NOTECUT;

		switch(m->command)
		{
		case CMD_S3MCMDEX:
			if(m->param == 0x91)
			{
				// surround remap (this is the "official" command)
				m->command = CMD_PANNING8;
				m->param = 0xA4;
			}
			break;
		case CMD_SMOOTHMIDI:
			m->command = CMD_MIDI;
			break;
		default:
			break;
		}
	} // End if (oldTypeIsIT_MPT && newTypeIsS3M)

	///////////////////////////////////
	// MOD <-> XM: Speed/Tempo update
	if(oldTypeIsMOD && newTypeIsXM)
	{
		switch(m->command)
		{
		case CMD_SPEED:
			m->param = min(m->param, 0x1F);
			break;
		}
	} else if(oldTypeIsXM && newTypeIsMOD)
	{
		switch(m->command)
		{
		case CMD_TEMPO:
			m->param = max(m->param, 0x21);
			break;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// Convert MOD to anything - adjust effect memory, remove Invert Loop
	if (oldTypeIsMOD)
	{
		switch(m->command)
		{
		case CMD_TONEPORTAVOL: // lacks memory -> 500 is the same as 300
			if(m->param == 0x00) m->command = CMD_TONEPORTAMENTO;
			break;
		case CMD_VIBRATOVOL: // lacks memory -> 600 is the same as 400
			if(m->param == 0x00) m->command = CMD_VIBRATO;
			break;

		case CMD_MODCMDEX: // This would turn into "Set Active Macro", so let's better remove it
			if((m->param & 0xF0) == 0xF0) m->command = CMD_NONE;
			break;
		}
	} // End if (oldTypeIsMOD && newTypeIsXM)

	/////////////////////////////////////////////////////////////////////
	// Convert anything to MOD - remove volume column, remove Set Macro
	if (newTypeIsMOD)
	{
		// convert note off events
		if(m->note >= NOTE_MIN_SPECIAL)
		{
			m->note = NOTE_NONE;
			// no effect present, so just convert note off to volume 0
			if(m->command == CMD_NONE)
			{
				m->command = CMD_VOLUME;
				m->param = 0;
			// EDx effect present, so convert it to ECx
			} else if((m->command == CMD_MODCMDEX) && ((m->param & 0xF0) == 0xD0))
			{
				m->param = 0xC0 | (m->param & 0x0F);
			}
		}

		if(m->command) switch(m->command)
		{
				case CMD_RETRIG: // MOD only has E9x
					m->command = CMD_MODCMDEX;
					m->param = 0x90 | (m->param & 0x0F);
					break;
				case CMD_MODCMDEX: // This would turn into "Invert Loop", so let's better remove it
					if((m->param & 0xF0) == 0xF0) m->command = CMD_NONE;
					break;
		}

		else switch(m->volcmd)
		{
				case VOLCMD_VOLUME:
					m->command = CMD_VOLUME;
					m->param = m->vol;
					break;
				case VOLCMD_PANNING:
					m->command = CMD_PANNING8;
					m->param = CLAMP(m->vol << 2, 0, 0xFF);
					break;
				case VOLCMD_VOLSLIDEDOWN:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol;
					break;
				case VOLCMD_VOLSLIDEUP:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol << 4;
					break;
				case VOLCMD_FINEVOLDOWN:
					m->command = CMD_MODCMDEX;
					m->param = 0xB0 | m->vol;
					break;
				case VOLCMD_FINEVOLUP:
					m->command = CMD_MODCMDEX;
					m->param = 0xA0 | m->vol;
					break;
				case VOLCMD_PORTADOWN:
					m->command = CMD_PORTAMENTODOWN;
					m->param = m->vol << 2;
					break;
				case VOLCMD_PORTAUP:
					m->command = CMD_PORTAMENTOUP;
					m->param = m->vol << 2;
					break;
				case VOLCMD_TONEPORTAMENTO:
					m->command = CMD_TONEPORTAMENTO;
					m->param = m->vol << 2;
					break;
				case VOLCMD_VIBRATODEPTH:
					m->command = CMD_VIBRATO;
					m->param = m->vol;
					break;
				case VOLCMD_VIBRATOSPEED:
					m->command = CMD_VIBRATO;
					m->param = m->vol << 4;
					break;
					// OpenMPT-specific commands
				case VOLCMD_OFFSET:
					m->command = CMD_OFFSET;
					m->param = m->vol << 3;
					break;
				case VOLCMD_VELOCITY:
					m->command = CMD_VOLUME;
					m->param = m->vol * 7;
					break;
				default:
					break;
		}
		m->volcmd = CMD_NONE;
	} // End if (newTypeIsMOD)

	///////////////////////////////////////////////////
	// Convert anything to S3M - adjust volume column
	if (newTypeIsS3M)
	{
		if(!m->command) switch(m->volcmd)
		{
				case VOLCMD_VOLSLIDEDOWN:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VOLSLIDEUP:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_FINEVOLDOWN:
					m->command = CMD_VOLUMESLIDE;
					m->param = 0xF0 | m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_FINEVOLUP:
					m->command = CMD_VOLUMESLIDE;
					m->param = (m->vol << 4) | 0x0F;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PORTADOWN:
					m->command = CMD_PORTAMENTODOWN;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PORTAUP:
					m->command = CMD_PORTAMENTOUP;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_TONEPORTAMENTO:
					m->command = CMD_TONEPORTAMENTO;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VIBRATODEPTH:
					m->command = CMD_VIBRATO;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VIBRATOSPEED:
					m->command = CMD_VIBRATO;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PANSLIDELEFT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PANSLIDERIGHT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
					// OpenMPT-specific commands
				case VOLCMD_OFFSET:
					m->command = CMD_OFFSET;
					m->param = m->vol << 3;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VELOCITY:
					m->volcmd = CMD_VOLUME;
					m->vol *= 7;
					break;
				default:
					break;
		}
	} // End if (newTypeIsS3M)

	//////////////////////////////////////////////////
	// Convert anything to XM - adjust volume column
	if (newTypeIsXM)
	{
		if(!m->command) switch(m->volcmd)
		{
				case VOLCMD_PORTADOWN:
					m->command = CMD_PORTAMENTODOWN;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PORTAUP:
					m->command = CMD_PORTAMENTOUP;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
					// OpenMPT-specific commands
				case VOLCMD_OFFSET:
					m->command = CMD_OFFSET;
					m->param = m->vol << 3;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VELOCITY:
					m->volcmd = CMD_VOLUME;
					m->vol *= 7;
					break;
				default:
					break;
		}
	} // End if (newTypeIsXM)

	///////////////////////////////////////////////////
	// Convert anything to IT - adjust volume column
	if (newTypeIsIT_MPT)
	{
		if(!m->command) switch(m->volcmd)
		{
				case VOLCMD_VOLSLIDEDOWN:
				case VOLCMD_VOLSLIDEUP:
				case VOLCMD_FINEVOLDOWN:
				case VOLCMD_FINEVOLUP:
				case VOLCMD_PORTADOWN:
				case VOLCMD_PORTAUP:
				case VOLCMD_TONEPORTAMENTO:
				case VOLCMD_VIBRATODEPTH:
					// OpenMPT-specific commands
				case VOLCMD_OFFSET:
				case VOLCMD_VELOCITY:
					m->vol = min(m->vol, 9);
					break;
				case VOLCMD_PANSLIDELEFT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PANSLIDERIGHT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VIBRATOSPEED:
					m->command = CMD_VIBRATO;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				default:
					break;
		}
	} // End if (newTypeIsIT)

	if(!CSoundFile::GetModSpecifications(nNewType).HasNote(m->note))
		m->note = NOTE_NONE;

	// ensure the commands really exist in this format
	if(CSoundFile::GetModSpecifications(nNewType).HasCommand(m->command) == false)
		m->command = CMD_NONE;
	if(CSoundFile::GetModSpecifications(nNewType).HasVolCommand(m->volcmd) == false)
		m->volcmd = CMD_NONE;

}