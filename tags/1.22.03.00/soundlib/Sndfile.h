/*
 * Sndfile.h
 * ---------
 * Purpose: Core class of the playback engine. Every song is represented by a CSoundFile object.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "../soundlib/SoundFilePlayConfig.h"
#include "../soundlib/MixerSettings.h"
#include "../common/misc_util.h"
#include "../common/Logging.h"
#include "mod_specifications.h"
#include <vector>
#include <bitset>
#include <set>
#include "Snd_defs.h"
#include "tuning.h"
#include "MIDIMacros.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/MIDIMapping.h"
#endif // MODPLUG_TRACKER

#include "ModSample.h"
#include "ModInstrument.h"
#include "ModChannel.h"
#include "modcommand.h"
#include "plugins/PlugInterface.h"
#include "RowVisitor.h"
#include "Message.h"

#include "Resampler.h"
#include "../sounddsp/Reverb.h"
#include "../sounddsp/AGC.h"
#include "../sounddsp/DSP.h"
#include "../sounddsp/EQ.h"

// -----------------------------------------------------------------------------------------
// MODULAR ModInstrument FIELD ACCESS : body content at the (near) top of Sndfile.cpp !!!
// -----------------------------------------------------------------------------------------
extern void WriteInstrumentHeaderStruct(ModInstrument * input, FILE * file);
extern char *GetInstrumentHeaderFieldPointer(const ModInstrument * input, uint32 fcode, uint16 fsize);
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////
// Reverberation

#ifndef NO_REVERB

#define NUM_REVERBTYPES			29

LPCSTR GetReverbPresetName(UINT nPreset);

#endif

typedef VOID (* LPSNDMIXHOOKPROC)(int *, unsigned long, unsigned long); // buffer, samples, channels

#include "pattern.h"
#include "patternContainer.h"
#include "ModSequence.h"


#ifdef MODPLUG_TRACKER

// For WAV export (writing pattern positions to file)
struct PatternCuePoint
{
	uint64     offset;			// offset in the file (in samples)
	ORDERINDEX order;			// which order is this?
	bool       processed;		// has this point been processed by the main WAV render function yet?
};

#endif // MODPLUG_TRACKER


// Return values for GetLength()
struct GetLengthType
{
	double duration;		// total time in seconds
	ROWINDEX lastRow;		// last parsed row (dito)
	ROWINDEX endRow;		// last row before module loops (dito)
	ORDERINDEX lastOrder;	// last parsed order (if no target is specified, this is the first order that is parsed twice, i.e. not the *last* played order)
	ORDERINDEX endOrder;	// last order before module loops (UNDEFINED if a target is specified)
	bool targetReached;		// true if the specified order/row combination has been reached while going through the module
};


// Target seek mode for GetLength()
struct GetLengthTarget
{
	union
	{
		double time;
		struct
		{
			ROWINDEX row;
			ORDERINDEX order;
		} pos;
	};

	enum Mode
	{
		NoTarget,		// Don't seek, i.e. return complete module length.
		SeekPosition,	// Seek to given pattern position.
		SeekSeconds,	// Seek to given time.
	} mode;

	// Don't seek, i.e. return complete module length.
	GetLengthTarget()
	{
		mode = NoTarget;
	}

	// Seek to given pattern position if position is valid.
	GetLengthTarget(ORDERINDEX order, ROWINDEX row)
	{
		mode = NoTarget;
		if(order != ORDERINDEX_INVALID && row != ROWINDEX_INVALID)
		{
			mode = SeekPosition;
			pos.row = row;
			pos.order = order;
		}
	}

	// Seek to given time if t is valid (i.e. not negative).
	GetLengthTarget(double t)
	{
		mode = NoTarget;
		if(t >= 0.0)
		{
			mode = SeekSeconds;
			time = t;
		}
	}
};


// Reset mode for GetLength()
enum enmGetLengthResetMode
{
	// Never adjust global variables / mod parameters
	eNoAdjust			= 0x00,
	// Mod parameters (such as global volume, speed, tempo, etc...) will always be memorized if the target was reached (i.e. they won't be reset to the previous values).  If target couldn't be reached, they are reset to their default values.
	eAdjust				= 0x01,
	// Same as above, but global variables will only be memorized if the target could be reached. This does *NOT* influence the visited rows vector - it will *ALWAYS* be adjusted in this mode.
	eAdjustOnSuccess	= 0x02 | eAdjust,
};


// Delete samples assigned to instrument
enum deleteInstrumentSamples
{
	deleteAssociatedSamples,
	doNoDeleteAssociatedSamples,
#ifdef MODPLUG_TRACKER
	askDeleteAssociatedSamples,
#endif // MODPLUG_TRACKER
};


//Note: These are bit indeces. MSF <-> Mod(Specific)Flag.
//If changing these, ChangeModTypeTo() might need modification.
FLAGSET(ModSpecificFlag)
{
	MSF_COMPATIBLE_PLAY		= 1,		//IT/MPT/XM
	MSF_OLDVOLSWING			= 2,		//IT/MPT
	MSF_MIDICC_BUGEMULATION	= 4,		//IT/MPT/XM
	MSF_OLD_MIDI_PITCHBENDS	= 8,		//IT/MPT/XM
};


class CTuningCollection;
#ifdef MODPLUG_TRACKER
class CModDoc;
#endif // MODPLUG_TRACKER


//==============
class CSoundFile
//==============
{
public:
	//Return true if title was changed.
	bool SetTitle(const char*, size_t strSize);

public: //Misc
	void ChangeModTypeTo(const MODTYPE& newType);

	// Returns value in seconds. If given position won't be played at all, returns -1.
	// If updateVars is true, the state of various playback variables will be updated according to the playback position.
	double GetPlaybackTimeAt(ORDERINDEX ord, ROWINDEX row, bool updateVars);

	uint16 GetModFlags() const {return static_cast<uint16>(m_ModFlags);}
	void SetModFlags(const uint16 v) {m_ModFlags = static_cast<ModSpecificFlag>(v);}
	bool GetModFlag(ModSpecificFlag i) const { return m_ModFlags.test(i); }
	void SetModFlag(ModSpecificFlag i, bool val) { m_ModFlags.set(i, val); }

	// Is compatible mode for a specific tracker turned on?
	// Hint 1: No need to poll for MOD_TYPE_MPT, as it will automatically be linked with MOD_TYPE_IT when using TRK_IMPULSETRACKER
	// Hint 2: Always returns true for MOD / S3M format (if that is the format of the current file)
	bool IsCompatibleMode(MODTYPE type) const
	{
		if(GetType() & type & (MOD_TYPE_MOD | MOD_TYPE_S3M))
			return true; // S3M and MOD format don't have compatibility flags, so we will always return true
		return ((GetType() & type) && GetModFlag(MSF_COMPATIBLE_PLAY));
	}

	// Process pingpong loops like Impulse Tracker (see Fastmix.cpp for an explanation)
	bool IsITPingPongMode() const
	{
		return IsCompatibleMode(TRK_IMPULSETRACKER);
	}

	// Check whether a filter algorithm closer to IT's should be used.
	bool UseITFilterMode() const { return IsCompatibleMode(TRK_IMPULSETRACKER) && !m_SongFlags[SONG_EXFILTERRANGE]; }

	//Tuning-->
public:
#ifdef MODPLUG_TRACKER
	static bool LoadStaticTunings();
	bool SaveStaticTunings();
	static void DeleteStaticdata();
	static CTuningCollection& GetBuiltInTunings() {return *s_pTuningsSharedBuiltIn;}
	static CTuningCollection& GetLocalTunings() {return *s_pTuningsSharedLocal;}
#endif
	static CTuning *GetDefaultTuning() {return nullptr;}
	CTuningCollection& GetTuneSpecificTunings() {return *m_pTuningsTuneSpecific;}

	std::string GetNoteName(const int16&, const INSTRUMENTINDEX inst = INSTRUMENTINDEX_INVALID) const;
private:
	CTuningCollection* m_pTuningsTuneSpecific;
#ifdef MODPLUG_TRACKER
	static CTuningCollection* s_pTuningsSharedBuiltIn;
	static CTuningCollection* s_pTuningsSharedLocal;
#endif
	//<--Tuning

public: // get 'controllers'

#ifdef MODPLUG_TRACKER
	CMIDIMapper& GetMIDIMapper() {return m_MIDIMapper;}
	const CMIDIMapper& GetMIDIMapper() const {return m_MIDIMapper;}
#endif // MODPLUG_TRACKER

private: //Effect functions
	void PortamentoMPT(ModChannel*, int);
	void PortamentoFineMPT(ModChannel*, int);

private: //Misc private methods.
	static void SetModSpecsPointer(const CModSpecifications*& pModSpecs, const MODTYPE type);
	ModSpecificFlag GetModFlagMask(MODTYPE oldtype, MODTYPE newtype) const;

private: //'Controllers'

#ifdef MODPLUG_TRACKER
	CMIDIMapper m_MIDIMapper;
#endif // MODPLUG_TRACKER

private: //Misc data
	const CModSpecifications *m_pModSpecs;
	FlagSet<ModSpecificFlag, uint16> m_ModFlags;

private:
	DWORD gdwSysInfo;

private:
	// Front Mix Buffer (Also room for interleaved rear mix)
	int MixSoundBuffer[MIXBUFFERSIZE * 4];
	int MixRearBuffer[MIXBUFFERSIZE * 2];
#ifndef NO_REVERB
	int MixReverbBuffer[MIXBUFFERSIZE * 2];
#endif
	float MixFloatBuffer[MIXBUFFERSIZE * 2];
	LONG gnDryLOfsVol;
	LONG gnDryROfsVol;

public:
	MixerSettings m_MixerSettings;
	CResampler m_Resampler;
#ifndef NO_REVERB
	CReverb m_Reverb;
#endif
#ifndef NO_DSP
	CDSP m_DSP;
#endif
#ifndef NO_EQ
	CEQ m_EQ;
#endif
#ifndef NO_AGC
	CAGC m_AGC;
#endif

#ifdef MODPLUG_TRACKER
	static LPSNDMIXHOOKPROC gpSndMixHook;
#endif
#ifndef NO_VST
	static PMIXPLUGINCREATEPROC gpMixPluginCreateProc;
#endif

	typedef uint32 samplecount_t;	// Number of rendered samples

public:	// for Editing
#ifdef MODPLUG_TRACKER
	CModDoc *m_pModDoc;		// Can be a null pointer for example when previewing samples from the treeview.
#endif // MODPLUG_TRACKER
	FlagSet<MODTYPE> m_nType;
	CHANNELINDEX m_nChannels;
	SAMPLEINDEX m_nSamples;
	INSTRUMENTINDEX m_nInstruments;
	UINT m_nDefaultSpeed, m_nDefaultTempo, m_nDefaultGlobalVolume;
	FlagSet<SongFlags> m_SongFlags;
	UINT m_nMixChannels, m_nMixStat;
	samplecount_t m_nBufferCount;
	double m_dBufferDiff;
	UINT m_nTickCount;
	UINT m_nPatternDelay, m_nFrameDelay;	// m_nPatternDelay = pattern delay (rows), m_nFrameDelay = fine pattern delay (ticks)
	samplecount_t m_lTotalSampleCount;	// rewbs.VSTTimeInfo
	bool m_bPositionChanged;		// Report to plugins that we jumped around in the module
	UINT m_nSamplesPerTick;		// rewbs.betterBPM
	ROWINDEX m_nDefaultRowsPerBeat, m_nDefaultRowsPerMeasure;	// default rows per beat and measure for this module // rewbs.betterBPM
	ROWINDEX m_nCurrentRowsPerBeat, m_nCurrentRowsPerMeasure;	// current rows per beat and measure for this module
	BYTE m_nTempoMode;			// rewbs.betterBPM
	UINT m_nMusicSpeed, m_nMusicTempo;	// Current speed and tempo
	ROWINDEX m_nNextRow, m_nRow;
	ROWINDEX m_nNextPatStartRow; // for FT2's E60 bug
	PATTERNINDEX m_nPattern;
	ORDERINDEX m_nCurrentOrder, m_nNextOrder, m_nRestartPos, m_nSeqOverride;

#ifdef MODPLUG_TRACKER
	// Lock playback between two orders. Lock is active if lock start != ORDERINDEX_INVALID).
	ORDERINDEX m_lockOrderStart, m_lockOrderEnd;
#endif // MODPLUG_TRACKER

	UINT m_nGlobalVolume, m_nSamplesToGlobalVolRampDest, m_nGlobalVolumeRampAmount,
		 m_nGlobalVolumeDestination, m_nSamplePreAmp, m_nVSTiVolume;
	long m_lHighResRampingGlobalVolume;
	bool IsGlobalVolumeUnset() const { return IsFirstTick(); }
	UINT m_nFreqFactor, m_nTempoFactor, m_nOldGlbVolSlide;
	LONG m_nMinPeriod, m_nMaxPeriod;	// min period = highest possible frequency, max period = lowest possible frequency
	LONG m_nRepeatCount;	// -1 means repeat infinitely.
	DWORD m_nGlobalFadeSamples, m_nGlobalFadeMaxSamples;
	UINT m_nMaxOrderPosition;
	UINT ChnMix[MAX_CHANNELS];							// Channels to be mixed
	ModChannel Chn[MAX_CHANNELS];						// Mixing channels... First m_nChannel channels are master channels (i.e. they are never NNA channels)!
	ModChannelSettings ChnSettings[MAX_BASECHANNELS];	// Initial channels settings
	CPatternContainer Patterns;							// Patterns
	ModSequenceSet Order;								// Modsequences. Order[x] returns an index of a pattern located at order x of the current sequence.
protected:
	ModSample Samples[MAX_SAMPLES];						// Sample Headers
public:
	ModInstrument *Instruments[MAX_INSTRUMENTS];		// Instrument Headers
	MIDIMacroConfig m_MidiCfg;							// MIDI Macro config table
	SNDMIXPLUGIN m_MixPlugins[MAX_MIXPLUGINS];			// Mix plugins
	char m_szNames[MAX_SAMPLES][MAX_SAMPLENAME];		// Song and sample names
	std::bitset<MAX_BASECHANNELS> m_bChannelMuteTogglePending;

	DWORD m_dwCreatedWithVersion;
	DWORD m_dwLastSavedWithVersion;

#ifdef MODPLUG_TRACKER
	std::vector<PatternCuePoint> m_PatternCuePoints;	// For WAV export (writing pattern positions to file)
#endif // MODPLUG_TRACKER

protected:
	// Mix level stuff
	CSoundFilePlayConfig m_PlayConfig;
	mixLevels m_nMixLevels;

	// For handling backwards jumps and stuff to prevent infinite loops when counting the mod length or rendering to wav.
	RowVisitor visitedSongRows;

public:
	// Song message
	SongMessage songMessage;
	mpt::String madeWithTracker;

// -> CODE#0023
// -> DESC="IT project files (.itp)"
	mpt::String m_szInstrumentPath[MAX_INSTRUMENTS];
// -! NEW_FEATURE#0023

	bool m_bIsRendering;
	bool m_bPatternTransitionOccurred;

private:
	// logging and user interaction
	ILog *m_pCustomLog;

public:
	CSoundFile();
	~CSoundFile();

public:
	// logging and user interaction
	void SetCustomLog(ILog *pLog) { m_pCustomLog = pLog; }
	void AddToLog(LogLevel level, const std::string &text) const;
	void AddToLog(const std::string &text) const { AddToLog(LogInformation, text); }

public:

	enum ModLoadingFlags
	{
		onlyVerifyHeader	= 0x00,
		loadPatternData		= 0x01,	// If unset, advise loaders to not process any pattern data (if possible)
		loadSampleData		= 0x02,	// If unset, advise loaders to not process any sample data (if possible)
		// Shortcuts
		loadCompleteModule	= loadSampleData | loadPatternData,
		loadNoPatternData	= loadSampleData,
	};

#ifdef MODPLUG_TRACKER
	// Get parent CModDoc. Can be nullptr if previewing from tree view, and is always nullptr if we're not actually compiling OpenMPT.
	CModDoc *GetpModDoc() const { return m_pModDoc; }

	BOOL Create(FileReader file, ModLoadingFlags loadFlags = loadCompleteModule, CModDoc *pModDoc = nullptr);
#else
	BOOL Create(FileReader file, ModLoadingFlags loadFlags);
#endif // MODPLUG_TRACKER

	BOOL Destroy();
	MODTYPE GetType() const { return m_nType; }
	bool TypeIsIT_MPT() const { return (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) != 0; }
	bool TypeIsIT_MPT_XM() const { return (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM)) != 0; }
	bool TypeIsS3M_IT_MPT() const { return (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT | MOD_TYPE_MPT)) != 0; }

	void SetPreAmp(UINT vol);
	UINT GetPreAmp() const { return m_MixerSettings.m_nPreAmp; }

	void SetMixLevels(mixLevels levels);
	mixLevels GetMixLevels() const { return m_nMixLevels; }
	const CSoundFilePlayConfig &GetPlayConfig() const { return m_PlayConfig; }

	INSTRUMENTINDEX GetNumInstruments() const { return m_nInstruments; }
	SAMPLEINDEX GetNumSamples() const { return m_nSamples; }
	UINT GetCurrentPos() const;
	PATTERNINDEX GetCurrentPattern() const { return m_nPattern; }
	ORDERINDEX GetCurrentOrder() const { return m_nCurrentOrder; }
	CHANNELINDEX GetNumChannels() const { return m_nChannels; }

	IMixPlugin* GetInstrumentPlugin(INSTRUMENTINDEX instr);
	const CModSpecifications& GetModSpecifications() const {return *m_pModSpecs;}
	static const CModSpecifications& GetModSpecifications(const MODTYPE type);

	void PatternTranstionChnSolo(const CHANNELINDEX chnIndex);
	void PatternTransitionChnUnmuteAll();
	double GetCurrentBPM() const;
	void DontLoopPattern(PATTERNINDEX nPat, ROWINDEX nRow = 0);		//rewbs.playSongFromCursor
	void SetCurrentPos(UINT nPos);
	void SetCurrentOrder(ORDERINDEX nOrder);
	LPCSTR GetTitle() const { return m_szNames[0]; }
	LPCTSTR GetSampleName(UINT nSample) const;
	const char *GetInstrumentName(INSTRUMENTINDEX nInstr) const;
	UINT GetMusicSpeed() const { return m_nMusicSpeed; }
	UINT GetMusicTempo() const { return m_nMusicTempo; }
	bool IsFirstTick() const { return (m_lTotalSampleCount == 0); }

	//Get modlength in various cases: total length, length to
	//specific order&row etc. Return value is in seconds.
	GetLengthType GetLength(enmGetLengthResetMode adjustMode, GetLengthTarget target = GetLengthTarget());

	void InitializeVisitedRows() { visitedSongRows.Initialize(true); }

public:
	//Returns song length in seconds.
	DWORD GetSongTime() { return static_cast<DWORD>(GetLength(eNoAdjust).duration + 0.5); }

	void RecalculateSamplesPerTick();
	double GetRowDuration(UINT tempo, UINT speed) const;
	UINT GetTickDuration(UINT tempo, UINT speed, ROWINDEX rowsPerBeat);

	// A repeat count value of -1 means infinite loop
	void SetRepeatCount(int n) { m_nRepeatCount = n; }
	int GetRepeatCount() const { return m_nRepeatCount; }
	bool IsPaused() const {	return m_SongFlags[SONG_PAUSED | SONG_STEP]; }	// Added SONG_STEP as it seems to be desirable in most cases to check for this as well.
	void LoopPattern(PATTERNINDEX nPat, ROWINDEX nRow = 0);

	bool InitChannel(CHANNELINDEX nChn);

	// Global variable initializer for loader functions
	void InitializeGlobals();
	void InitializeChannels();

	// Module Loaders
	bool ReadXM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadS3M(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMod(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadM15(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMed(const LPCBYTE lpStream, const DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadIT(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadITProject(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool Read669(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadUlt(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadWav(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDSM(const LPCBYTE lpStream, const DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadFAR(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMS(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMS2(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMDL(const LPCBYTE lpStream, const DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadOKT(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDMF(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPTM(const LPCBYTE lpStream, const DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDBM(const LPCBYTE lpStream, const DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMF_Asylum(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMF_DSMI(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMT2(const LPCBYTE lpStream, const DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPSM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPSM16(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadUMX(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMO3(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadGDM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadIMF(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadJ2B(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDIGI(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMID(const LPCBYTE lpStream, DWORD dwMemLength, ModLoadingFlags loadFlags = loadCompleteModule);

	static std::vector<const char *> GetSupportedExtensions(bool otherFormats);
	static const char * ModTypeToString(MODTYPE modtype);
	static const char * ModTypeToTracker(MODTYPE modtype);

	void UpgradeModFlags();
	void UpgradeSong();

	// Save Functions
#ifndef MODPLUG_NO_FILESAVE
	bool SaveXM(LPCSTR lpszFileName, bool compatibilityExport = false);
	bool SaveS3M(LPCSTR lpszFileName) const;
	bool SaveMod(LPCSTR lpszFileName) const;
	bool SaveIT(LPCSTR lpszFileName, bool compatibilityExport = false);
	bool SaveITProject(LPCSTR lpszFileName); // -> CODE#0023 -> DESC="IT project files (.itp)" -! NEW_FEATURE#0023
	UINT SaveMixPlugins(FILE *f=NULL, BOOL bUpdate=TRUE);
	void WriteInstrumentPropertyForAllInstruments(__int32 code,  __int16 size, FILE* f, UINT nInstruments) const;
	void SaveExtendedInstrumentProperties(UINT nInstruments, FILE* f) const;
	void SaveExtendedSongProperties(FILE* f) const;
	size_t SaveModularInstrumentData(FILE *f, const ModInstrument *pIns) const;
#endif // MODPLUG_NO_FILESAVE
	void LoadExtendedSongProperties(const MODTYPE modtype, FileReader &file, bool* pInterpretMptMade = nullptr);
	static size_t LoadModularInstrumentData(FileReader &file, ModInstrument &ins);

	mpt::String GetSchismTrackerVersion(uint16 cwtv);

	// Reads extended instrument properties(XM/IT/MPTM).
	// If no errors occur and song extension tag is found, returns pointer to the beginning
	// of the tag, else returns NULL.
	void LoadExtendedInstrumentProperties(FileReader &file, bool *pInterpretMptMade = nullptr);

	// MOD Convert function
	MODTYPE GetBestSaveFormat() const;
	void ConvertModCommand(ModCommand &m) const;
	void S3MConvert(ModCommand &m, bool fromIT) const;
	void S3MSaveConvert(uint8 &command, uint8 &param, bool toIT, bool compatibilityExport = false) const;
	void ModSaveCommand(uint8 &command, uint8 &param, const bool toXM, const bool compatibilityExport = false) const;
	void ReadMODPatternEntry(FileReader &file, ModCommand &m) const;

	void SetupMODPanning(bool bForceSetup = false); // Setup LRRL panning, max channel volume

public:
	// Real-time sound functions
	void SuspendPlugins(); //rewbs.VSTCompliance
	void ResumePlugins();  //rewbs.VSTCompliance
	void StopAllVsti();    //rewbs.VSTCompliance
	void RecalculateGainForAllPlugs();
	void ResetChannels();
	UINT Read(LPVOID lpBuffer, UINT cbBuffer);
	UINT CreateStereoMix(int count);
	UINT GetResamplingFlag(const ModChannel *pChannel);
	BOOL FadeSong(UINT msec);
	BOOL GlobalFadeSong(UINT msec);
	void ProcessPlugins(UINT nCount);
	samplecount_t GetTotalSampleCount() const { return m_lTotalSampleCount; }
	bool HasPositionChanged() { bool b = m_bPositionChanged; m_bPositionChanged = false; return b; }
	bool IsRenderingToDisc() const { return m_bIsRendering; }

public:
	// Mixer Config
	void SetMixerSettings(const MixerSettings &mixersettings);
	void SetResamplerSettings(const CResamplerSettings &resamplersettings);
	void InitPlayer(BOOL bReset=FALSE);
	void SetDspEffects(DWORD DSPMask);
	DWORD GetSampleRate() { return m_MixerSettings.gdwMixingFreq; }
	static DWORD GetSysInfo();
#ifndef NO_EQ
	void SetEQGains(const UINT *pGains, UINT nBands, const UINT *pFreqs=NULL, BOOL bReset=FALSE)	{ m_EQ.SetEQGains(pGains, nBands, pFreqs, bReset, m_MixerSettings.gdwMixingFreq); } // 0=-12dB, 32=+12dB
#endif // NO_EQ
	// Float <-> Int conversion routines
	/*static */VOID StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount);
	/*static */VOID FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount);
	/*static */VOID MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount);
	/*static */VOID FloatToMonoMix(const float *pIn, int *pOut, UINT nCount);

public:
	BOOL ReadNote();
	BOOL ProcessRow();
	BOOL ProcessEffects();
	CHANNELINDEX GetNNAChannel(CHANNELINDEX nChn) const;
	void CheckNNA(CHANNELINDEX nChn, UINT instr, int note, bool forceCut);
	void NoteChange(CHANNELINDEX nChn, int note, bool bPorta = false, bool bResetEnv = true, bool bManual = false);
	void InstrumentChange(ModChannel *pChn, UINT instr, bool bPorta = false, bool bUpdVol = true, bool bResetEnv = true);

	// Channel Effects
	void KeyOff(CHANNELINDEX nChn);
	// Global Effects
	void SetTempo(UINT param, bool setAsNonModcommand = false);
	void SetSpeed(UINT param);

protected:
	// Channel effect processing
	int GetVibratoDelta(int type, int position) const;

	void ProcessVolumeSwing(ModChannel *pChn, int &vol);
	void ProcessPanningSwing(ModChannel *pChn);
	void ProcessTremolo(ModChannel *pChn, int &vol);
	void ProcessTremor(ModChannel *pChn, int &vol);

	bool IsEnvelopeProcessed(const ModChannel *pChn, enmEnvelopeTypes env) const;
	void ProcessVolumeEnvelope(ModChannel *pChn, int &vol);
	void ProcessPanningEnvelope(ModChannel *pChn);
	void ProcessPitchFilterEnvelope(ModChannel *pChn, int &period);

	void IncrementEnvelopePosition(ModChannel *pChn, enmEnvelopeTypes envType);
	void IncrementEnvelopePositions(ModChannel *pChn);

	void ProcessInstrumentFade(ModChannel *pChn, int &vol);

	void ProcessPitchPanSeparation(ModChannel *pChn);
	void ProcessPanbrello(ModChannel *pChn);

	void ProcessArpeggio(ModChannel *pChn, int &period, CTuning::NOTEINDEXTYPE &arpeggioSteps);
	void ProcessVibrato(CHANNELINDEX nChn, int &period, CTuning::RATIOTYPE &vibratoFactor);
	void ProcessSampleAutoVibrato(ModChannel *pChn, int &period, CTuning::RATIOTYPE &vibratoFactor, int &nPeriodFrac);

	void ProcessRamping(ModChannel *pChn);

protected:
	// Channel Effects
	void UpdateS3MEffectMemory(ModChannel *pChn, UINT param) const;
	void PortamentoUp(CHANNELINDEX nChn, UINT param, const bool doFinePortamentoAsRegular = false);
	void PortamentoDown(CHANNELINDEX nChn, UINT param, const bool doFinePortamentoAsRegular = false);
	void MidiPortamento(CHANNELINDEX nChn, int param, bool doFineSlides);
	void FinePortamentoUp(ModChannel *pChn, UINT param);
	void FinePortamentoDown(ModChannel *pChn, UINT param);
	void ExtraFinePortamentoUp(ModChannel *pChn, UINT param);
	void ExtraFinePortamentoDown(ModChannel *pChn, UINT param);
	void NoteSlide(ModChannel *pChn, UINT param, bool slideUp);
	void TonePortamento(ModChannel *pChn, UINT param);
	void Vibrato(ModChannel *pChn, UINT param);
	void FineVibrato(ModChannel *pChn, UINT param);
	void VolumeSlide(ModChannel *pChn, UINT param);
	void PanningSlide(ModChannel *pChn, UINT param, bool memory = true);
	void ChannelVolSlide(ModChannel *pChn, UINT param);
	void FineVolumeUp(ModChannel *pChn, UINT param, bool volCol);
	void FineVolumeDown(ModChannel *pChn, UINT param, bool volCol);
	void Tremolo(ModChannel *pChn, UINT param);
	void Panbrello(ModChannel *pChn, UINT param);
	void RetrigNote(CHANNELINDEX nChn, int param, UINT offset=0);  //rewbs.volOffset: added last param
	void SampleOffset(CHANNELINDEX nChn, UINT param);
	void NoteCut(CHANNELINDEX nChn, UINT nTick);
	ROWINDEX PatternLoop(ModChannel *, UINT param);
	void ExtendedMODCommands(CHANNELINDEX nChn, UINT param);
	void ExtendedS3MCommands(CHANNELINDEX nChn, UINT param);
	void ExtendedChannelEffect(ModChannel *, UINT param);
	void InvertLoop(ModChannel* pChn);

	void ProcessMacroOnChannel(CHANNELINDEX nChn);
	void ProcessMIDIMacro(CHANNELINDEX nChn, bool isSmooth, char *macro, uint8 param = 0, PLUGINDEX plugin = 0);
	float CalculateSmoothParamChange(float currentValue, float param) const;
	size_t SendMIDIData(CHANNELINDEX nChn, bool isSmooth, const unsigned char *macro, size_t macroLen, PLUGINDEX plugin);

	void SetupChannelFilter(ModChannel *pChn, bool bReset, int flt_modifier = 256) const;
	// Low-Level effect processing
	void DoFreqSlide(ModChannel *pChn, LONG nFreqSlide);
	void GlobalVolSlide(UINT param, UINT &nOldGlobalVolSlide);
	DWORD IsSongFinished(UINT nOrder, UINT nRow) const;
	void UpdateTimeSignature();

	UINT GetNumTicksOnCurrentRow() const
	{
		return (m_nMusicSpeed  + m_nFrameDelay) * MAX(m_nPatternDelay, 1);
	}

public:
	bool DestroySample(SAMPLEINDEX nSample);
	bool DestroySampleThreadsafe(SAMPLEINDEX nSample);

	// Find an unused sample slot. If it is going to be assigned to an instrument, targetInstrument should be specified.
	// SAMPLEINDEX_INVLAID is returned if no free sample slot could be found.
	SAMPLEINDEX GetNextFreeSample(INSTRUMENTINDEX targetInstrument = INSTRUMENTINDEX_INVALID, SAMPLEINDEX start = 1) const;
	// Find an unused instrument slot.
	// INSTRUMENTINDEX_INVALID is returned if no free instrument slot could be found.
	INSTRUMENTINDEX GetNextFreeInstrument(INSTRUMENTINDEX start = 1) const;
	// Check whether a given sample is used by a given instrument.
	bool IsSampleReferencedByInstrument(SAMPLEINDEX sample, INSTRUMENTINDEX instr) const;

	bool DestroyInstrument(INSTRUMENTINDEX nInstr, deleteInstrumentSamples removeSamples);
	bool IsSampleUsed(SAMPLEINDEX nSample) const;
	bool IsInstrumentUsed(INSTRUMENTINDEX nInstr) const;
	bool RemoveInstrumentSamples(INSTRUMENTINDEX nInstr);
	SAMPLEINDEX DetectUnusedSamples(std::vector<bool> &sampleUsed) const;
	SAMPLEINDEX RemoveSelectedSamples(const std::vector<bool> &keepSamples);
	static void AdjustSampleLoop(ModSample &sample);

	// Samples file I/O
	bool ReadSampleFromFile(SAMPLEINDEX nSample, const LPBYTE lpMemFile, DWORD dwFileLength);
	bool ReadWAVSample(SAMPLEINDEX nSample, FileReader &file, FileReader *wsmpChunk = nullptr);
	bool ReadPATSample(SAMPLEINDEX nSample, const LPBYTE lpMemFile, DWORD dwFileLength);
	bool ReadS3ISample(SAMPLEINDEX nSample, const LPBYTE lpMemFile, DWORD dwFileLength);
	bool ReadAIFFSample(SAMPLEINDEX nSample, FileReader &file);
	bool ReadXISample(SAMPLEINDEX nSample, FileReader &file);
	bool ReadITSSample(SAMPLEINDEX nSample, FileReader &file, bool rewind = true);
	bool ReadITISample(SAMPLEINDEX nSample, FileReader &file);
	bool Read8SVXSample(SAMPLEINDEX nInstr, const LPBYTE lpMemFile, DWORD dwFileLength);
	bool ReadFLACSample(SAMPLEINDEX sample, FileReader &file);
	bool ReadMP3Sample(SAMPLEINDEX sample, FileReader &file);
#ifndef MODPLUG_NO_FILESAVE
	bool SaveWAVSample(SAMPLEINDEX nSample, const LPCSTR lpszFileName) const;
	bool SaveRAWSample(SAMPLEINDEX nSample, const LPCSTR lpszFileName) const;
	bool SaveFLACSample(SAMPLEINDEX nSample, const LPCSTR lpszFileName) const;
#endif

	// Instrument file I/O
	bool ReadInstrumentFromFile(INSTRUMENTINDEX nInstr, const LPBYTE lpMemFile, DWORD dwFileLength);
	bool ReadXIInstrument(INSTRUMENTINDEX nInstr, FileReader &file);
	bool ReadITIInstrument(INSTRUMENTINDEX nInstr, FileReader &file);
	bool ReadPATInstrument(INSTRUMENTINDEX nInstr, const LPBYTE lpMemFile, DWORD dwFileLength);
	bool ReadSampleAsInstrument(INSTRUMENTINDEX nInstr, const LPBYTE lpMemFile, DWORD dwFileLength);
#ifndef MODPLUG_NO_FILESAVE
	bool SaveXIInstrument(INSTRUMENTINDEX nInstr, const LPCSTR lpszFileName) const;
	bool SaveITIInstrument(INSTRUMENTINDEX nInstr, const LPCSTR lpszFileName, bool compress) const;
#endif

	// I/O from another sound file
	bool ReadInstrumentFromSong(INSTRUMENTINDEX targetInstr, const CSoundFile &srcSong, INSTRUMENTINDEX sourceInstr);
	bool ReadSampleFromSong(SAMPLEINDEX targetSample, const CSoundFile &srcSong, SAMPLEINDEX sourceSample);

	// Period/Note functions
	UINT GetNoteFromPeriod(UINT period) const;
	UINT GetPeriodFromNote(UINT note, int nFineTune, UINT nC5Speed) const;
	UINT GetFreqFromPeriod(UINT period, UINT nC5Speed, int nPeriodFrac=0) const;
	// Misc functions
	ModSample &GetSample(SAMPLEINDEX sample) { ASSERT(sample <= m_nSamples && sample < CountOf(Samples)); return Samples[sample]; }
	const ModSample &GetSample(SAMPLEINDEX sample) const { ASSERT(sample <= m_nSamples && sample < CountOf(Samples)); return Samples[sample]; }

	UINT MapMidiInstrument(DWORD dwProgram, UINT nChannel, UINT nNote);
	size_t ITInstrToMPT(FileReader &file, ModInstrument &ins, uint16 trkvers);
	void LoadMixPlugins(FileReader &file);

	DWORD CutOffToFrequency(UINT nCutOff, int flt_modifier=256) const; // [0-127] => [1-10KHz]
#ifdef MODPLUG_TRACKER
	void ProcessMidiOut(CHANNELINDEX nChn);
#endif // MODPLUG_TRACKER
	void ApplyGlobalVolume(int SoundBuffer[], int RearBuffer[], long lTotalSampleCount);

#ifndef MODPLUG_TRACKER
	void ApplyFinalOutputGain(int SoundBuffer[], int RearBuffer[], long lCount); // lCount meaning the number of frames, totally independet from the numer of channels
	void ApplyFinalOutputGainFloat(float *beg, float *end);
#endif

	// System-Dependant functions
public:
	static void *AllocateSample(UINT nbytes);
	static void FreeSample(void *p);

	ModInstrument *AllocateInstrument(INSTRUMENTINDEX instr, SAMPLEINDEX assignedSample = 0);

	// WAV export
	UINT Normalize24BitBuffer(LPBYTE pbuffer, UINT cbsizebytes, DWORD lmax24, DWORD dwByteInc);

private:
	PLUGINDEX GetChannelPlugin(CHANNELINDEX nChn, PluginMutePriority respectMutes) const;
	PLUGINDEX GetActiveInstrumentPlugin(CHANNELINDEX, PluginMutePriority respectMutes) const;
	IMixPlugin * GetChannelInstrumentPlugin(CHANNELINDEX chn) const;

	void HandlePatternTransitionEvents();

public:
	PLUGINDEX GetBestPlugin(CHANNELINDEX nChn, PluginPriority priority, PluginMutePriority respectMutes) const;
	uint8 GetBestMidiChannel(CHANNELINDEX nChn) const;

};

#pragma warning(default : 4324) //structure was padded due to __declspec(align())


extern const LPCSTR szNoteNames[12];
extern const LPCTSTR szDefaultNoteNames[NOTE_MAX];


inline IMixPlugin* CSoundFile::GetInstrumentPlugin(INSTRUMENTINDEX instr)
//-----------------------------------------------------------------------
{
	if(instr > 0 && instr < MAX_INSTRUMENTS && Instruments[instr] && Instruments[instr]->nMixPlug && Instruments[instr]->nMixPlug <= MAX_MIXPLUGINS)
		return m_MixPlugins[Instruments[instr]->nMixPlug-1].pMixPlugin;
	else
		return NULL;
}


///////////////////////////////////////////////////////////
// Low-level Mixing functions

#define SCRATCH_BUFFER_SIZE 64 //Used for plug's final processing (cleanup)
#define VOLUMERAMPPRECISION	12
#define FADESONGDELAY		100

#define MOD2XMFineTune(k)	((int)( (signed char)((k)<<4) ))
#define XM2MODFineTune(k)	((int)( (k>>4)&0x0f ))

// Read instrument property with 'code' and 'size' from 'file' to instrument 'pIns'.
void ReadInstrumentExtensionField(ModInstrument* pIns, const uint32 code, const uint16 size, FileReader &file);

// Read instrument property with 'code' from 'file' to instrument 'pIns'.
void ReadExtendedInstrumentProperty(ModInstrument* pIns, const uint32 code, FileReader &file);

// Read extended instrument properties from 'file' to instrument 'pIns'.
void ReadExtendedInstrumentProperties(ModInstrument* pIns, FileReader &file);

// Convert instrument flags which were read from 'dF..' extension to proper internal representation.
void ConvertReadExtendedFlags(ModInstrument* pIns);
