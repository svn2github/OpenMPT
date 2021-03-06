/*
 * ModDoc.h
 * --------
 * Purpose: Converting between various module formats.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "Sndfile.h"
#include "../common/misc_util.h"
#include "Undo.h"
#include "Notification.h"
#include "../common/Logging.h"
#include <time.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bit Mask for updating view (hints of what changed)
#define HINT_MODTYPE		0x00001
#define HINT_MODCOMMENTS	0x00002
#define HINT_MODGENERAL		0x00004
#define HINT_MODSEQUENCE	0x00008
#define HINT_MODCHANNELS	0x00010
#define HINT_PATTERNDATA	0x00020
#define HINT_PATTERNROW		0x00040
#define HINT_PATNAMES		0x00080
#define HINT_MPTOPTIONS		0x00100
#define HINT_SAMPLEINFO		0x00400
#define HINT_SAMPLEDATA		0x00800
#define HINT_INSTRUMENT		0x01000
#define HINT_ENVELOPE		0x02000
#define HINT_SMPNAMES		0x04000
#define HINT_INSNAMES		0x08000
#define HINT_UNDO			0x10000
#define HINT_MIXPLUGINS		0x20000
#define HINT_SPEEDCHANGE	0x40000	//rewbs.envRowGrid
#define HINT_SEQNAMES		0x80000
#define HINT_MAXHINTFLAG	HINT_SEQNAMES
//Bits 0-19 are reserved.
#define HINT_MASK_FLAGS		(2*HINT_MAXHINTFLAG - 1) //When applied to hint parameter, should give the flag part.
#define HINT_MASK_ITEM		(~HINT_MASK_FLAGS) //To nullify update hintbits from hint parameter.
#define HintFlagPart(x)		((x) & HINT_MASK_FLAGS)

//If fails, hint flagbits|itembits does not enable all bits; 
//might be worthwhile to check the reason.
STATIC_ASSERT( (HINT_MASK_ITEM | HINT_MASK_FLAGS) == -1 ); 

//If fails, hint param flag and item parts overlap; might be a problem.
STATIC_ASSERT( (HINT_MASK_ITEM & HINT_MASK_FLAGS) == 0 );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE : be careful when adding new flags !!!
// -------------------------------------------------------------------------------------------------------------------------
// those flags are passed through a 32bits parameter which can also contain instrument/sample/pattern row... number :
// HINT_SAMPLEINFO & HINT_SAMPLEDATA & HINT_SMPNAMES : can be used with a sample number 12bit coded (passed as bit 20 to 31)
// HINT_PATTERNROW : is used with a row number 10bit coded (passed as bit 22 to 31)
// HINT_INSTRUMENT & HINT_INSNAMES : can be used with an instrument number 8bit coded (passed as bit 24 to 31)
// new flags can be added BUT be carefull that they will not be used in a case they should aliased with, ie, a sample number
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Updateview hints can, in addition to the actual hints, contain
//addition data such as pattern or instrument index. The
//values below define the number of bits used for these.
#define HINT_BITS_PATTERN	12
#define HINT_BITS_ROWS		10
#define HINT_BITS_SAMPLE	12
#define HINT_BITS_INST		8
#define HINT_BITS_CHNTAB	8
#define HINT_BITS_SEQUENCE	6

//Defines bit shift values used for setting/retrieving the additional hint data to/from hint parameter.
#define HINT_SHIFT_PAT		(32 - HINT_BITS_PATTERN)
#define HINT_SHIFT_ROW		(32 - HINT_BITS_ROWS)
#define HINT_SHIFT_SMP		(32 - HINT_BITS_SAMPLE)
#define HINT_SHIFT_INS		(32 - HINT_BITS_INST)
#define HINT_SHIFT_CHNTAB	(32 - HINT_BITS_CHNTAB)
#define HINT_SHIFT_SEQUENCE	(32 - HINT_BITS_SEQUENCE)

//Check that hint bit counts are not too large given the number of hint flags.
STATIC_ASSERT( ((-1 << HINT_SHIFT_PAT) & HINT_MASK_ITEM) == (-1 << HINT_SHIFT_PAT) ); 
STATIC_ASSERT( ((-1 << HINT_SHIFT_ROW) & HINT_MASK_ITEM) == (-1 << HINT_SHIFT_ROW) ); 
STATIC_ASSERT( ((-1 << HINT_SHIFT_SMP) & HINT_MASK_ITEM) == (-1 << HINT_SHIFT_SMP) ); 
STATIC_ASSERT( ((-1 << HINT_SHIFT_INS) & HINT_MASK_ITEM) == (-1 << HINT_SHIFT_INS) ); 
STATIC_ASSERT( ((-1 << HINT_SHIFT_CHNTAB) & HINT_MASK_ITEM) == (-1 << HINT_SHIFT_CHNTAB) ); 
STATIC_ASSERT( ((-1 << HINT_SHIFT_SEQUENCE) & HINT_MASK_ITEM) == (-1 << HINT_SHIFT_SEQUENCE) ); 


/////////////////////////////////////////////////////////////////////////
// File edit history

#define HISTORY_TIMER_PRECISION	18.2f

//================
struct FileHistory
//================
{
	// Date when the file was loaded in the the tracker or created.
	tm loadDate;
	// Time the file was open in the editor, in 1/18.2th seconds (frequency of a standard DOS timer, to keep compatibility with Impulse Tracker easy).
	uint32 openTime;
};

/////////////////////////////////////////////////////////////////////////
// Split Keyboard Settings (pattern editor)

//==========================
struct SplitKeyboardSettings
//==========================
{
	enum
	{
		splitOctaveRange = 9,
	};

	bool IsSplitActive() const { return (octaveLink && (octaveModifier != 0)) || (splitInstrument > 0) || (splitVolume != 0); }

	int octaveModifier;	// determines by how many octaves the notes should be transposed up or down
	ModCommand::NOTE splitNote;
	ModCommand::INSTR splitInstrument;
	ModCommand::VOL splitVolume;
	bool octaveLink;	// apply octaveModifier

	SplitKeyboardSettings()
	//---------------------
	{
		splitInstrument = 0;
		splitNote = NOTE_MIDDLEC - 1;
		splitVolume = 0;
		octaveModifier = 0;
		octaveLink = false;
	}
};

enum InputTargetContext;


struct LogEntry
{
	LogLevel level;
	std::string message;
	LogEntry() : level(LogInformation) {}
	LogEntry(LogLevel l, const std::string &m) : level(l), message(m) {}
};


enum LogMode
{
	LogModeInstantReporting,
	LogModeGather,
};


class ScopedLogCapturer
{
private:
	CModDoc &m_modDoc;
	LogMode m_oldLogMode;
	std::string m_title;
	CWnd *m_pParent;
	bool m_showLog;
public:
	ScopedLogCapturer(CModDoc &modDoc, const std::string &title = "", CWnd *parent = nullptr, bool showLog = true);
	~ScopedLogCapturer();
	void ShowLog(bool force = false);
	void ShowLog(const std::string &preamble, bool force = false);
};


//=============================
class CModDoc: public CDocument
//=============================
{
protected:
	friend ScopedLogCapturer;
	mutable std::vector<LogEntry> m_Log;
	LogMode m_LogMode;
	CSoundFile m_SndFile;

	HWND m_hWndFollow;
	FlagSet<Notification::Type, uint16> m_notifyType;
	Notification::Item m_notifyItem;
	CSize m_szOldPatternScrollbarsPos;

	CPatternUndo m_PatternUndo;
	CSampleUndo m_SampleUndo;
	SplitKeyboardSettings m_SplitKeyboardSettings;	// this is maybe not the best place to keep them, but it should do the job
	std::vector<FileHistory> m_FileHistory;	// File edit history
	time_t m_creationTime;

	bool bModifiedAutosave; // Modified since last autosave?
	bool m_ShowSavedialog;
public:
	bool m_bHasValidPath; //becomes true if document is loaded or saved.

protected:
	std::bitset<MAX_BASECHANNELS> m_bsMultiRecordMask;
	std::bitset<MAX_BASECHANNELS> m_bsMultiSplitRecordMask;
public:
	std::bitset<MAX_INSTRUMENTS> m_bsInstrumentModified;	// which instruments have been modified? (for ITP functionality)

protected: // create from serialization only
	CModDoc();
	DECLARE_SERIAL(CModDoc)

// public members
public:
	CSoundFile *GetSoundFile() { return &m_SndFile; }
	const CSoundFile *GetSoundFile() const { return &m_SndFile; }
	CSoundFile &GetrSoundFile() { return m_SndFile; }
	const CSoundFile &GetrSoundFile() const { return m_SndFile; }

	void SetModified(BOOL bModified=TRUE) { SetModifiedFlag(bModified); bModifiedAutosave = (bModified != FALSE); }
	bool ModifiedSinceLastAutosave() { bool bRetval = bModifiedAutosave; bModifiedAutosave = false; return bRetval; } // return "IsModified" value and reset it until the next SetModified() (as this is only used for polling)
	void SetShowSaveDialog(bool b) {m_ShowSavedialog = b;}
	void PostMessageToAllViews(UINT uMsg, WPARAM wParam=0, LPARAM lParam=0);
	void SendMessageToActiveViews(UINT uMsg, WPARAM wParam=0, LPARAM lParam=0);
	MODTYPE GetModType() const { return m_SndFile.m_nType; }
	INSTRUMENTINDEX GetNumInstruments() const { return m_SndFile.m_nInstruments; }
	SAMPLEINDEX GetNumSamples() const { return m_SndFile.m_nSamples; }

	// Logging for general progress and error events.
	void AddToLog(LogLevel level, const std::string &text) const;
	void AddToLog(const std::string &text) const { AddToLog(LogInformation, text); }

	const std::vector<LogEntry> & GetLog() const { return m_Log; }
	std::string GetLogString() const;
	LogLevel GetMaxLogLevel() const;
protected:
	LogMode GetLogMode() const { return m_LogMode; }
	void SetLogMode(LogMode mode) { m_LogMode = mode; }
	void ClearLog();
	UINT ShowLog(const std::string &preamble, const std::string &title = "", CWnd *parent = nullptr);
	UINT ShowLog(const std::string &title = "", CWnd *parent = nullptr) { return ShowLog("", title, parent); }

public:

	void ClearFilePath() { m_strPathName.Empty(); }

	void ViewPattern(UINT nPat, UINT nOrd);
	void ViewSample(UINT nSmp);
	void ViewInstrument(UINT nIns);
	HWND GetFollowWnd() const { return m_hWndFollow; }
	void SetFollowWnd(HWND hwnd);

	void SetNotifications(Notification::Type type, Notification::Item item = 0) { m_notifyType = type; m_notifyItem = item; }
	FlagSet<Notification::Type, uint16> GetNotificationType() const { return m_notifyType; }
	Notification::Item GetNotificationItem() const { return m_notifyItem; }

	void ActivateWindow();

	void SongProperties();

	void PrepareUndoForAllPatterns(bool storeChannelInfo = false);
	CPatternUndo &GetPatternUndo() { return m_PatternUndo; }
	CSampleUndo &GetSampleUndo() { return m_SampleUndo; }
	SplitKeyboardSettings &GetSplitKeyboardSettings() { return m_SplitKeyboardSettings; }

	std::vector<FileHistory> &GetFileHistory() { return m_FileHistory; }
	const std::vector<FileHistory> &GetFileHistory() const { return m_FileHistory; }
	time_t GetCreationTime() const { return m_creationTime; }
	
// operations
public:
	bool ChangeModType(MODTYPE wType);

	bool ChangeNumChannels(CHANNELINDEX nNewChannels, const bool showCancelInRemoveDlg = true);
	bool RemoveChannels(const std::vector<bool> &keepMask);
	CHANNELINDEX ReArrangeChannels(const std::vector<CHANNELINDEX> &fromToArray, const bool createUndoPoint = true);
	void CheckUsedChannels(std::vector<bool> &usedMask, CHANNELINDEX maxRemoveCount = MAX_BASECHANNELS) const;

	SAMPLEINDEX ReArrangeSamples(const std::vector<SAMPLEINDEX> &newOrder);

	INSTRUMENTINDEX ReArrangeInstruments(const std::vector<INSTRUMENTINDEX> &newOrder, deleteInstrumentSamples removeSamples = doNoDeleteAssociatedSamples);

	bool ConvertInstrumentsToSamples();
	bool ConvertSamplesToInstruments();
	UINT RemovePlugs(const std::vector<bool> &keepMask);

	PATTERNINDEX InsertPattern(ORDERINDEX nOrd = ORDERINDEX_INVALID, ROWINDEX nRows = 64);
	SAMPLEINDEX InsertSample(bool bLimit = false);
	INSTRUMENTINDEX InsertInstrument(SAMPLEINDEX lSample = SAMPLEINDEX_INVALID, INSTRUMENTINDEX lDuplicate = INSTRUMENTINDEX_INVALID);
	void InitializeInstrument(ModInstrument *pIns);
	bool RemoveOrder(SEQUENCEINDEX nSeq, ORDERINDEX nOrd);
	bool RemovePattern(PATTERNINDEX nPat);
	bool RemoveSample(SAMPLEINDEX nSmp);
	bool RemoveInstrument(INSTRUMENTINDEX nIns);

	void ProcessMIDI(uint32 midiData, INSTRUMENTINDEX ins, IMixPlugin *plugin, InputTargetContext ctx);
	CHANNELINDEX PlayNote(UINT note, INSTRUMENTINDEX nins, SAMPLEINDEX nsmp, bool pause, LONG nVol=-1, SmpLength loopStart = 0, SmpLength loopEnd = 0, CHANNELINDEX nCurrentChn = CHANNELINDEX_INVALID, const SmpLength sampleOffset = 0);
	bool NoteOff(UINT note, bool fade = false, INSTRUMENTINDEX ins = INSTRUMENTINDEX_INVALID, CHANNELINDEX currentChn = CHANNELINDEX_INVALID, CHANNELINDEX stopChn = CHANNELINDEX_INVALID); //rewbs.vstiLive: add params

	bool IsNotePlaying(UINT note, SAMPLEINDEX nsmp = 0, INSTRUMENTINDEX nins = 0);
	bool MuteChannel(CHANNELINDEX nChn, bool bMute);
	bool UpdateChannelMuteStatus(CHANNELINDEX nChn);
	bool MuteSample(SAMPLEINDEX nSample, bool bMute);
	bool MuteInstrument(INSTRUMENTINDEX nInstr, bool bMute);

	bool SoloChannel(CHANNELINDEX nChn, bool bSolo);
	bool IsChannelSolo(CHANNELINDEX nChn) const;

	bool SurroundChannel(CHANNELINDEX nChn, bool bSurround);
	bool SetChannelGlobalVolume(CHANNELINDEX nChn, uint16 nVolume);
	bool SetChannelDefaultPan(CHANNELINDEX nChn, uint16 nPan);
	bool IsChannelMuted(CHANNELINDEX nChn) const;
	bool IsSampleMuted(SAMPLEINDEX nSample) const;
	bool IsInstrumentMuted(INSTRUMENTINDEX nInstr) const;
	
	bool NoFxChannel(CHANNELINDEX nChn, bool bNoFx, bool updateMix = true);
	bool IsChannelNoFx(CHANNELINDEX nChn) const;
	bool IsChannelRecord1(CHANNELINDEX channel) const;
	bool IsChannelRecord2(CHANNELINDEX channel) const;
	BYTE IsChannelRecord(CHANNELINDEX channel) const;
	void Record1Channel(CHANNELINDEX channel, bool select = true);
	void Record2Channel(CHANNELINDEX channel, bool select = true);
	void ReinitRecordState(bool unselect = true);

	CHANNELINDEX GetNumChannels() const { return m_SndFile.m_nChannels; }
	UINT GetPatternSize(PATTERNINDEX nPat) const;
	BOOL AdjustEndOfSample(UINT nSample);
	bool IsChildSample(INSTRUMENTINDEX nIns, SAMPLEINDEX nSmp) const;
	INSTRUMENTINDEX FindSampleParent(SAMPLEINDEX sample) const;
	UINT FindInstrumentChild(UINT nIns) const;
	bool MoveOrder(ORDERINDEX nSourceNdx, ORDERINDEX nDestNdx, bool bUpdate = true, bool bCopy = false, SEQUENCEINDEX nSourceSeq = SEQUENCEINDEX_INVALID, SEQUENCEINDEX nDestSeq = SEQUENCEINDEX_INVALID);
	BOOL ExpandPattern(PATTERNINDEX nPattern);
	BOOL ShrinkPattern(PATTERNINDEX nPattern);

	bool CopyEnvelope(UINT nIns, enmEnvelopeTypes nEnv);
	bool PasteEnvelope(UINT nIns, enmEnvelopeTypes nEnv);

	LRESULT ActivateView(UINT nIdView, DWORD dwParam);
	void UpdateAllViews(CView *pSender, LPARAM lHint=0L, CObject *pHint=NULL);
	HWND GetEditPosition(ROWINDEX &row, PATTERNINDEX &pat, ORDERINDEX &ord);
	LRESULT OnCustomKeyMsg(WPARAM, LPARAM);
	void TogglePluginEditor(UINT m_nCurrentPlugin);
	void RecordParamChange(int slot, long param);
	void LearnMacro(int macro, long param);
	void SetElapsedTime(ORDERINDEX nOrd, ROWINDEX nRow);

	// Global settings to pattern effect conversion
	bool RestartPosToPattern();
	bool GlobalVolumeToPattern();

	bool HasMPTHacks(const bool autofix = false);

	void FixNullStrings();

// Fix: save pattern scrollbar position when switching to other tab
	CSize GetOldPatternScrollbarsPos() const { return m_szOldPatternScrollbarsPos; };
	void SetOldPatternScrollbarsPos( CSize s ){ m_szOldPatternScrollbarsPos = s; };

	void OnFileWaveConvert(ORDERINDEX nMinOrder, ORDERINDEX nMaxOrder);

	// Returns formatted ModInstrument name.
	// [in] bEmptyInsteadOfNoName: In case of unnamed instrument string, "(no name)" is returned unless this 
	//                             parameter is true is case which an empty name is returned.
	// [in] bIncludeIndex: True to include instrument index in front of the instrument name, false otherwise.
	CString GetPatternViewInstrumentName(INSTRUMENTINDEX nInstr, bool bEmptyInsteadOfNoName = false, bool bIncludeIndex = true) const;

	// Check if a given channel contains data.
	bool IsChannelUnused(CHANNELINDEX nChn) const;

// protected members
protected:

	BOOL InitializeMod();
	void* GetChildFrame(); //rewbs.customKeys

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName) {return OnSaveDocument(lpszPathName, false);}
	virtual void OnCloseDocument();
	void SafeFileClose();
	BOOL OnSaveDocument(LPCTSTR lpszPathName, const bool bTemplateFile);

// -> CODE#0023
// -> DESC="IT project files (.itp)"
	virtual BOOL SaveModified();
	bool SaveInstrument(INSTRUMENTINDEX instr);
// -! NEW_FEATURE#0023

	virtual BOOL DoSave(LPCSTR lpszPathName, BOOL bSaveAs=TRUE);
	virtual void DeleteContents();
	virtual void SetModifiedFlag(BOOL bModified=TRUE);
	//}}AFX_VIRTUAL

	uint8 GetPlaybackMidiChannel(const ModInstrument *pIns, CHANNELINDEX nChn) const;


// Implementation
public:
	virtual ~CModDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
public:
	//{{AFX_MSG(CModDoc)
	afx_msg void OnFileWaveConvert();
	afx_msg void OnFileMP3Convert();
	afx_msg void OnFileMidiConvert();
	afx_msg void OnFileCompatibilitySave();
	afx_msg void OnPlayerPlay();
	afx_msg void OnPlayerStop();
	afx_msg void OnPlayerPause();
	afx_msg void OnPlayerPlayFromStart();
	afx_msg void OnPanic();
	afx_msg void OnEditGlobals();
	afx_msg void OnEditPatterns();
	afx_msg void OnEditSamples();
	afx_msg void OnEditInstruments();
	afx_msg void OnEditComments();
	afx_msg void OnEditGraph();	//rewbs.Graph
	afx_msg void OnInsertPattern();
	afx_msg void OnInsertSample();
	afx_msg void OnInsertInstrument();
	afx_msg void OnShowCleanup();
	afx_msg void OnEstimateSongLength();
	afx_msg void OnApproximateBPM();
	afx_msg void OnUpdateXMITMPTOnly(CCmdUI *p);
	afx_msg void OnUpdateITMPTOnly(CCmdUI *p);
	afx_msg void OnUpdateHasMIDIMappings(CCmdUI *p);
	afx_msg void OnUpdateCompatExportableOnly(CCmdUI *p);
	afx_msg void OnPatternRestart() { OnPatternRestart(true); } //rewbs.customKeys
	afx_msg void OnPatternRestart(bool loop); //rewbs.customKeys
	afx_msg void OnPatternPlay(); //rewbs.customKeys
	afx_msg void OnPatternPlayNoLoop(); //rewbs.customKeys
	afx_msg void OnViewEditHistory();
	afx_msg void OnViewMPTHacks();
	afx_msg void OnSaveTemplateModule();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

	void ChangeFileExtension(MODTYPE nNewType);
	CHANNELINDEX FindAvailableChannel();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
