/*
 * MainFrm.h
 * ---------
 * Purpose: Implementation of OpenMPT's main window code.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "sndfile.h"
#include "CommandSet.h"
#include "inputhandler.h"
#include "mptrack.h"

class CInputHandler;
class CMainFrame;
class CModDoc;
class CAutoSaver;
class ISoundDevice;
class ISoundSource;

#define NUM_AUDIO_BUFFERS			3
#define MIN_AUDIO_BUFFERSIZE		1024
#define MAX_AUDIO_BUFFERSIZE		32768	// 32K buffers max
#define MAINFRAME_TITLE				"Open ModPlug Tracker"
#define INIBUFFERSIZE				MAX_PATH

enum
{
	CTRLMSG_BASE=0,
	CTRLMSG_SETVIEWWND,
	CTRLMSG_ACTIVATEPAGE,
	CTRLMSG_DEACTIVATEPAGE,
	CTRLMSG_SETFOCUS,
	// Pattern-Specific
	CTRLMSG_SETCURRENTPATTERN,
	CTRLMSG_GETCURRENTPATTERN,
	CTRLMSG_SETCURRENTORDER,
	CTRLMSG_GETCURRENTORDER,
	CTRLMSG_FORCEREFRESH,
	CTRLMSG_PAT_PREVINSTRUMENT,
	CTRLMSG_PAT_NEXTINSTRUMENT,
	CTRLMSG_PAT_SETINSTRUMENT,
	CTRLMSG_PAT_FOLLOWSONG,		//rewbs.customKeys
	CTRLMSG_PAT_LOOP,
	CTRLMSG_PAT_NEWPATTERN,		//rewbs.customKeys
	CTRLMSG_SETUPMACROS,
	CTRLMSG_GETCURRENTINSTRUMENT,
	CTRLMSG_SETCURRENTINSTRUMENT,
	CTRLMSG_PLAYPATTERN,
	CTRLMSG_GETSPACING,
	CTRLMSG_SETSPACING,
	CTRLMSG_ISRECORDING,
	CTRLMSG_PATTERNCHANGED,
	CTRLMSG_PREVORDER,
	CTRLMSG_NEXTORDER,
	CTRLMSG_SETRECORD,
	// Sample-Specific
	CTRLMSG_SMP_PREVINSTRUMENT,
	CTRLMSG_SMP_NEXTINSTRUMENT,
	CTRLMSG_SMP_OPENFILE,
	CTRLMSG_SMP_SETZOOM,
	CTRLMSG_SMP_GETZOOM,
	CTRLMSG_SMP_SONGDROP,
	// Instrument-Specific
	CTRLMSG_INS_PREVINSTRUMENT,
	CTRLMSG_INS_NEXTINSTRUMENT,
	CTRLMSG_INS_OPENFILE,
	CTRLMSG_INS_NEWINSTRUMENT,
	CTRLMSG_INS_SONGDROP,
	CTRLMSG_INS_SAMPLEMAP,
	CTRLMSG_PAT_DUPPATTERN,
};

enum
{
	VIEWMSG_BASE=0,
	VIEWMSG_SETCTRLWND,
	VIEWMSG_SETACTIVE,
	VIEWMSG_SETFOCUS,
	VIEWMSG_SAVESTATE,
	VIEWMSG_LOADSTATE,
	// Pattern-Specific
	VIEWMSG_SETCURRENTPATTERN,
	VIEWMSG_GETCURRENTPATTERN,
	VIEWMSG_FOLLOWSONG,
	VIEWMSG_PATTERNLOOP,
	VIEWMSG_GETCURRENTPOS,
	VIEWMSG_SETRECORD,
	VIEWMSG_SETSPACING,
	VIEWMSG_PATTERNPROPERTIES,
	VIEWMSG_SETVUMETERS,
	VIEWMSG_SETPLUGINNAMES,	//rewbs.patPlugNames
	VIEWMSG_DOMIDISPACING,
	VIEWMSG_EXPANDPATTERN,
	VIEWMSG_SHRINKPATTERN,
	VIEWMSG_COPYPATTERN,
	VIEWMSG_PASTEPATTERN,
	VIEWMSG_AMPLIFYPATTERN,
	VIEWMSG_SETDETAIL,
	// Sample-Specific
	VIEWMSG_SETCURRENTSAMPLE,
	// Instrument-Specific
	VIEWMSG_SETCURRENTINSTRUMENT,
	VIEWMSG_DOSCROLL,

};


#define MODSTATUS_PLAYING		0x01
#define MODSTATUS_BUSY			0x02
#define MODSTATUS_RENDERING     0x04 //rewbs.VSTTimeInfo

#define SOUNDSETUP_ENABLEMMX	0x08
#define SOUNDSETUP_SOFTPANNING	0x10
#define SOUNDSETUP_STREVERSE	0x20
#define SOUNDSETUP_SECONDARY	0x40
#define SOUNDSETUP_RESTARTMASK	SOUNDSETUP_SECONDARY

#define QUALITY_NOISEREDUCTION	0x01
#define QUALITY_MEGABASS		0x02
#define QUALITY_SURROUND		0x08
#define QUALITY_REVERB			0x20
#define QUALITY_AGC				0x40
#define QUALITY_EQ				0x80


// User-defined colors
enum
{
	MODCOLOR_BACKNORMAL = 0,
	MODCOLOR_TEXTNORMAL,
	MODCOLOR_BACKCURROW,
	MODCOLOR_TEXTCURROW,
	MODCOLOR_BACKSELECTED,
	MODCOLOR_TEXTSELECTED,
	MODCOLOR_SAMPLE,
	MODCOLOR_BACKPLAYCURSOR,
	MODCOLOR_TEXTPLAYCURSOR,
	MODCOLOR_BACKHILIGHT,
	MODCOLOR_NOTE,
	MODCOLOR_INSTRUMENT,
	MODCOLOR_VOLUME,
	MODCOLOR_PANNING,
	MODCOLOR_PITCH,
	MODCOLOR_GLOBALS,
	MODCOLOR_ENVELOPES,
	MODCOLOR_VUMETER_LO,
	MODCOLOR_VUMETER_MED,
	MODCOLOR_VUMETER_HI,
	MODCOLOR_SEPSHADOW,
	MODCOLOR_SEPFACE,
	MODCOLOR_SEPHILITE,
	MODCOLOR_BLENDCOLOR,
	MODCOLOR_DODGY_COMMANDS,
	MAX_MODCOLORS,
	// Internal color codes (not saved to color preset files)
	MODCOLOR_2NDHIGHLIGHT,
	MODCOLOR_DEFAULTVOLUME,
	MAX_MODPALETTECOLORS
};

#define NUM_VUMETER_PENS		32

// Pattern Setup (contains also non-pattern related settings)
// Feel free to replace the deprecated flags by new flags, but be sure to
// update TrackerSettings::LoadINISettings() / TrackerSettings::LoadRegistrySettings() as well.
#define PATTERN_PLAYNEWNOTE			0x01		// play new notes while recording
#define PATTERN_LARGECOMMENTS		0x02		// use large font in comments
#define PATTERN_STDHIGHLIGHT		0x04		// enable primary highlight (measures)
#define PATTERN_SMALLFONT			0x08		// use small font in pattern editor
#define PATTERN_CENTERROW			0x10		// always center active row
#define PATTERN_WRAP				0x20		// wrap around cursor in editor
#define PATTERN_EFFECTHILIGHT		0x40		// effect syntax highlighting
#define PATTERN_HEXDISPLAY			0x80		// display row number in hex
#define PATTERN_FLATBUTTONS			0x100		// flat toolbar buttons
#define PATTERN_CREATEBACKUP		0x200		// create .bak files when saving
#define PATTERN_SINGLEEXPAND		0x400		// single click to expand tree
#define PATTERN_PLAYEDITROW			0x800		// play all notes on the current row while entering notes
#define PATTERN_NOEXTRALOUD			0x1000		// no loud samples in sample editor
#define PATTERN_DRAGNDROPEDIT		0x2000		// enable drag and drop editing
#define PATTERN_2NDHIGHLIGHT		0x4000		// activate secondary highlight (beats)
#define PATTERN_MUTECHNMODE			0x8000		// ignore muted channels
#define PATTERN_SHOWPREVIOUS		0x10000		// show prev/next patterns
#define PATTERN_CONTSCROLL			0x20000		// continous pattern scrolling
#define PATTERN_KBDNOTEOFF			0x40000		// Record note-off events
#define PATTERN_FOLLOWSONGOFF		0x80000		// follow song off by default
#define PATTERN_MIDIRECORD			0x100000	// MIDI Record on by default
#define PATTERN_NOCLOSEDIALOG		0x200000	// Don't use OpenMPT's custom close dialog with a list of saved files when closing the main window
#define PATTERN_DBLCLICKSELECT		0x400000	// Double-clicking pattern selects whole channel
#define PATTERN_OLDCTXMENUSTYLE		0x800000	// mpt 1.16 pattern context menu style
#define PATTERN_SYNCMUTE			0x1000000	// maintain sample sync on mute
#define PATTERN_AUTODELAY			0x2000000	// automatically insert delay commands in pattern when entering notes
#define PATTERN_NOTEFADE			0x4000000	// alt. note fade behaviour when entering notes
#define PATTERN_OVERFLOWPASTE		0x8000000	// continue paste in the next pattern instead of cutting off
#define PATTERN_SHOWDEFAULTVOLUME	0x10000000	// if there is no volume command next to note+instr, display the sample's default volume.
#define PATTERN_RESETCHANNELS		0x20000000	// reset channels when looping
#define PATTERN_LIVEUPDATETREE		0x40000000	// update active sample / instr icons in treeview


// Midi Setup
#define MIDISETUP_RECORDVELOCITY			0x01	// Record MIDI velocity
#define MIDISETUP_TRANSPOSEKEYBOARD			0x02	// Apply transpose value to MIDI Notes
#define MIDISETUP_MIDITOPLUG				0x04	// Pass MIDI messages to plugins
#define MIDISETUP_MIDIVOL_TO_NOTEVOL		0x08	// Combine MIDI volume to note velocity
#define MIDISETUP_RECORDNOTEOFF				0x10	// Record MIDI Note Off to pattern
#define MIDISETUP_RESPONDTOPLAYCONTROLMSGS	0x20	// Respond to Restart/Continue/Stop MIDI commands
#define MIDISETUP_AMPLIFYVELOCITY			0x40	// Amplify velocity of recorded notes
#define MIDISETUP_MIDIMACROCONTROL			0x80	// Record MIDI controller changes a MIDI macro changes in pattern
#define MIDISETUP_PLAYPATTERNONMIDIIN       0x100	// Play pattern if MIDI Note is received and playback is paused



// Image List index
enum
{
	IMAGE_COMMENTS=0,
	IMAGE_PATTERNS,
	IMAGE_SAMPLES,
	IMAGE_INSTRUMENTS,
	IMAGE_GENERAL,
	IMAGE_FOLDER,
	IMAGE_OPENFOLDER,
	IMAGE_PARTITION,
	IMAGE_NOSAMPLE,
	IMAGE_NOINSTRUMENT,
	IMAGE_NETWORKDRIVE,
	IMAGE_CDROMDRIVE,
	IMAGE_RAMDRIVE,
	IMAGE_FLOPPYDRIVE,
	IMAGE_REMOVABLEDRIVE,
	IMAGE_FIXEDDRIVE,
	IMAGE_FOLDERPARENT,
	IMAGE_FOLDERSONG,
	IMAGE_DIRECTX,
	IMAGE_WAVEOUT,
	IMAGE_ASIO,
	IMAGE_GRAPH,
	IMAGE_SAMPLEMUTE,
	IMAGE_INSTRMUTE,
	IMAGE_SAMPLEACTIVE,
	IMAGE_INSTRACTIVE,
	IMAGE_NOPLUGIN,
	IMAGE_EFFECTPLUGIN,
	IMAGE_PLUGININSTRUMENT,
};


// Toolbar Image List index
enum
{
	TIMAGE_PATTERN_NEW=0,
	TIMAGE_PATTERN_STOP,
	TIMAGE_PATTERN_PLAY,
	TIMAGE_PATTERN_RESTART,
	TIMAGE_PATTERN_RECORD,
	TIMAGE_MIDI_RECORD, // unused?
	TIMAGE_SAMPLE_NEW,
	TIMAGE_INSTR_NEW,
	TIMAGE_SAMPLE_NORMALIZE,
	TIMAGE_SAMPLE_AMPLIFY,
	TIMAGE_SAMPLE_UPSAMPLE,
	TIMAGE_SAMPLE_REVERSE,
	TIMAGE_OPEN,
	TIMAGE_SAVE,
	TIMAGE_PREVIEW,
	TIMAGE_PAUSE, // unused?
	TIMAGE_PATTERN_VUMETERS,
	TIMAGE_MACROEDITOR,
	TIMAGE_CHORDEDITOR,
	TIMAGE_PATTERN_PROPERTIES,
	TIMAGE_PATTERN_EXPAND,
	TIMAGE_PATTERN_SHRINK,
	TIMAGE_SAMPLE_SILENCE,
	TIMAGE_TAB_SAMPLES,
	TIMAGE_TAB_INSTRUMENTS,
	TIMAGE_TAB_PATTERNS,
	TIMAGE_UNDO,
	TIMAGE_REDO,
	TIMAGE_PATTERN_PLAYROW,
	TIMAGE_SAMPLE_DOWNSAMPLE,
	TIMAGE_PATTERN_DETAIL_LO,
	TIMAGE_PATTERN_DETAIL_MED,
	TIMAGE_PATTERN_DETAIL_HI,
	TIMAGE_PATTERN_PLUGINS,
	TIMAGE_CHANNELMANAGER,
	TIMAGE_SAMPLE_INVERT,
	TIMAGE_SAMPLE_UNSIGN,
	TIMAGE_SAMPLE_DCOFFSET,
	TIMAGE_PATTERN_OVERFLOWPASTE,
	TIMAGE_SAMPLE_FIXLOOP,
	TIMAGE_SAMPLE_AUTOTUNE,
};


// Sample editor toolbar image list index
enum
{
	SIMAGE_CHECKED = 0,
	SIMAGE_ZOOMUP,
	SIMAGE_ZOOMDOWN,
	SIMAGE_NODRAW,
	SIMAGE_DRAW,
	SIMAGE_RESIZE,
	SIMAGE_GENERATE,
	SIMAGE_GRID,
};


// Instrument editor toolbar image list index
enum
{
	IIMAGE_CHECKED = 0,
	IIMAGE_VOLENV,
	IIMAGE_PANENV,
	IIMAGE_PITCHENV,
	IIMAGE_NOPITCHENV,
	IIMAGE_LOOP,
	IIMAGE_SUSTAIN,
	IIMAGE_CARRY,
	IIMAGE_NOCARRY,
	IIMAGE_VOLSWITCH,
	IIMAGE_PANSWITCH,
	IIMAGE_PITCHSWITCH,
	IIMAGE_FILTERSWITCH,
	IIMAGE_NOPITCHSWITCH,
	IIMAGE_NOFILTERSWITCH,
	IIMAGE_SAMPLEMAP,
	IIMAGE_GRID,
	IIMAGE_ZOOMIN,
	IIMAGE_NOZOOMIN,
	IIMAGE_ZOOMOUT,
	IIMAGE_NOZOOMOUT,
};


/////////////////////////////////////////////////////////////////////////
// Player position notification

#define MAX_UPDATE_HISTORY		8

#define MPTNOTIFY_TYPEMASK		0x00FF0000	// HiWord = type, LoWord = subtype (smp/instr #)
#define MPTNOTIFY_PENDING		0x01000000	// Being processed
#define MPTNOTIFY_DEFAULT		0x00010000
#define MPTNOTIFY_POSITION		0x00010000
#define MPTNOTIFY_SAMPLE		0x00020000
#define MPTNOTIFY_VOLENV		0x00040000
#define MPTNOTIFY_PANENV		0x00080000
#define MPTNOTIFY_PITCHENV		0x00100000
#define MPTNOTIFY_VUMETERS		0x00200000
#define MPTNOTIFY_MASTERVU		0x00400000
#define MPTNOTIFY_STOP			0x00800000
#define MPTNOTIFY_POSVALID		0x80000000	// dwPos[i] is valid

typedef struct MPTNOTIFICATION
{
	DWORD dwType;
	DWORD dwLatency;
	ORDERINDEX nOrder;			// Always valid
	PATTERNINDEX nPattern;		// dito
	ROWINDEX nRow;				// dito
	DWORD dwPos[MAX_CHANNELS];	// sample/envelope pos for each channel if >= 0
} MPTNOTIFICATION, *PMPTNOTIFICATION;


/////////////////////////////////////////////////////////////////////////
// EQ Presets

struct EQPRESET
{
	CHAR szName[12];
	UINT Gains[MAX_EQ_BANDS];
	UINT Freqs[MAX_EQ_BANDS];
};


/////////////////////////////////////////////////////////////////////////
// Misc. Macros


#define DeleteGDIObject(h) if (h) { ::DeleteObject(h); h = NULL; }
#define UPDATEDSPEFFECTS() SetDspEffects(\
								GetSettings().m_dwQuality & QUALITY_SURROUND,\
								GetSettings().m_dwQuality & QUALITY_REVERB,\
								GetSettings().m_dwQuality & QUALITY_MEGABASS,\
								GetSettings().m_dwQuality & QUALITY_NOISEREDUCTION,\
								GetSettings().m_dwQuality & QUALITY_EQ)

#include "mainbar.h"
#include "TrackerSettings.h"

//===================================
class CMainFrame: public CMDIFrameWnd
//===================================
{
	DECLARE_DYNAMIC(CMainFrame)
	// static data
public:

	// Globals
	static UINT m_nLastOptionsPage;
    static HHOOK ghKbdHook;
	static DWORD gdwNotificationType;
	
	// GDI
	static HICON m_hIcon;
	static HFONT m_hGUIFont, m_hFixedFont, m_hLargeFixedFont;
	static HBRUSH brushGray, brushBlack, brushWhite, brushText, brushHighLight, brushHighLightRed, brushWindow, brushYellow;
//	static CBrush *pbrushBlack, *pbrushWhite;
	static HPEN penBlack, penDarkGray, penLightGray, penWhite, penHalfDarkGray, penSample, penEnvelope, penEnvelopeHighlight, penSeparator, penScratch, penGray00, penGray33, penGray40, penGray55, penGray80, penGray99, penGraycc, penGrayff;
	static HCURSOR curDragging, curNoDrop, curArrow, curNoDrop2, curVSplit;
	static LPMODPLUGDIB bmpPatterns, bmpNotes, bmpVUMeters, bmpVisNode, bmpVisPcNode;
	static HPEN gpenVuMeter[NUM_VUMETER_PENS * 2];	// General tab VU meters

protected:

	// All application settings
	static TrackerSettings m_Settings;

public:

	// Low-Level Audio
	static CRITICAL_SECTION m_csAudio;
	static ISoundDevice *gpSoundDevice;
	static HANDLE m_hAudioWakeUp, m_hNotifyWakeUp;
	static HANDLE m_hPlayThread, m_hNotifyThread;
	static DWORD m_dwPlayThreadId, m_dwNotifyThreadId;
	static LONG gnLVuMeter, gnRVuMeter;
	static UINT gdwIdleTime;
	static LONG slSampleSize, sdwSamplesPerSec, sdwAudioBufferSize;

	// Midi Input
public:
	static HMIDIIN shMidiIn;


protected:

	CSoundFile m_WaveFile;
	CModTreeBar m_wndTree;
	CStatusBar m_wndStatusBar;
	CMainToolBar m_wndToolBar;
	CImageList m_ImageList;
	CModDoc *m_pModPlaying;
	CSoundFile *m_pSndFile;
	HWND m_hFollowSong, m_hWndMidi;
	DWORD m_dwStatus, m_dwElapsedTime, m_dwTimeSec, m_dwNotifyType;
	UINT m_nTimer, m_nAvgMixChn, m_nMixChn;
	CHAR m_szUserText[512], m_szInfoText[512], m_szXInfoText[512]; //rewbs.xinfo
	// Notification Buffer
	MPTNOTIFICATION NotifyBuffer[MAX_UPDATE_HISTORY];
	// Misc
	bool m_bOptionsLocked; 	 	//rewbs.customKeys
	double m_dTotalCPU;
	CModDoc* m_pJustModifiedDoc;

public:
	CMainFrame(/*CString regKeyExtension*/);
	VOID Initialize();
	

// Low-Level Audio
public:
	static void UpdateAudioParameters(BOOL bReset=FALSE);
	static void EnableLowLatencyMode(BOOL bOn=TRUE);
	static void CalcStereoVuMeters(int *, unsigned long, unsigned long);
	static DWORD WINAPI AudioThread(LPVOID);
	static DWORD WINAPI NotifyThread(LPVOID);
	ULONG AudioRead(PVOID pData, ULONG cbSize);
	VOID AudioDone(ULONG nBytesWritten, ULONG nLatency);
	LONG audioTryOpeningDevice(UINT channels, UINT bits, UINT samplespersec);
	BOOL audioOpenDevice();
	void audioCloseDevice();
	BOOL audioFillBuffers();
	LRESULT OnWOMDone(WPARAM, LPARAM);
	BOOL dsoundFillBuffers(LPBYTE lpBuf, DWORD dwSize);
	BOOL DSoundDone(LPBYTE lpBuffer, DWORD dwBytes);
	BOOL DoNotification(DWORD dwSamplesRead, DWORD dwLatency);

// Midi Input Functions
public:
	BOOL midiOpenDevice();
	void midiCloseDevice();
	void midiReceive();
	void SetMidiRecordWnd(HWND hwnd) { m_hWndMidi = hwnd; }
	HWND GetMidiRecordWnd() const { return m_hWndMidi; }

	static int ApplyVolumeRelatedSettings(const DWORD &dwParam1, const BYTE midivolume);

// static functions
public:
	static CMainFrame *GetMainFrame() { return (CMainFrame *)theApp.m_pMainWnd; }
	static TrackerSettings &GetSettings() { return m_Settings; }
	static VOID UpdateColors();
	static HICON GetModIcon() { return m_hIcon; }
	static HFONT GetGUIFont() { return m_hGUIFont; }
	static HFONT GetFixedFont() { return m_hFixedFont; }
	static HFONT GetLargeFixedFont() { return m_hLargeFixedFont; }
	static void UpdateAllViews(DWORD dwHint, CObject *pHint=NULL);
	static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
	static CInputHandler *m_InputHandler; 	//rewbs.customKeys
	static CAutoSaver *m_pAutoSaver; 		//rewbs.customKeys
	//static CPerformanceCounter *m_pPerfCounter;

	static bool WritePrivateProfileLong(const CString section, const CString key, const long value, const CString iniFile);
	static long GetPrivateProfileLong(const CString section, const CString key, const long defaultValue, const CString iniFile);
	static bool WritePrivateProfileDWord(const CString section, const CString key, const DWORD value, const CString iniFile);
	static DWORD GetPrivateProfileDWord(const CString section, const CString key, const DWORD defaultValue, const CString iniFile);
	static bool WritePrivateProfileCString(const CString section, const CString key, const CString value, const CString iniFile);
	static CString GetPrivateProfileCString(const CString section, const CString key, const CString defaultValue, const CString iniFile);
	

	// Misc functions
public:
	VOID SetUserText(LPCSTR lpszText);
	VOID SetInfoText(LPCSTR lpszText);
	VOID SetXInfoText(LPCSTR lpszText); //rewbs.xinfo
	VOID SetHelpText(LPCSTR lpszText);
	UINT GetBaseOctave();
	CModDoc *GetActiveDoc();
	CView *GetActiveView();  	//rewbs.customKeys
	CImageList *GetImageList() { return &m_ImageList; }
	PMPTCHORD GetChords() { return GetSettings().Chords; }
	VOID OnDocumentCreated(CModDoc *pModDoc);
	VOID OnDocumentClosed(CModDoc *pModDoc);
	VOID UpdateTree(CModDoc *pModDoc, DWORD lHint=0, CObject *pHint=NULL);
	static CInputHandler* GetInputHandler() { return m_InputHandler; }  	//rewbs.customKeys
	bool m_bModTreeHasFocus;  	//rewbs.customKeys
	CWnd *m_pNoteMapHasFocus;  	//rewbs.customKeys
	CWnd* m_pOrderlistHasFocus;
	long GetSampleRate();  		//rewbs.VSTTimeInfo
	long GetTotalSampleCount(); //rewbs.VSTTimeInfo
	double GetApproxBPM();		//rewbs.VSTTimeInfo
	void ThreadSafeSetModified(CModDoc* modified) {m_pJustModifiedDoc=modified;}

	void CreateExampleModulesMenu();
	void CreateTemplateModulesMenu();

	/// Creates submenu whose items are filenames of files in both
	/// AppDirectory\pszFolderName\   (usually C:\program files\OpenMPT\pszFolderName\)
	/// and
	/// ConfigDirectory\pszFolderName  (usually %appdata%\OpenMPT\pszFolderName\)
	/// [in] nMaxCount: Maximum number of items allowed in the menu
	/// [out] vPaths: Receives the full paths of the files added to the menu.
	/// [in] pszFolderName: Name of the folder (should end with \)
	/// [in] nIdRangeBegin: First ID for the menu item.
	static HMENU CreateFileMenu(const size_t nMaxCount, std::vector<CString>& vPaths, const LPCTSTR pszFolderName, const uint16 nIdRangeBegin);

// Player functions
public:
	BOOL PlayMod(CModDoc *, HWND hPat=NULL, DWORD dwNotifyType=0);
	BOOL StopMod(CModDoc *pDoc=NULL);
	BOOL PauseMod(CModDoc *pDoc=NULL);
	BOOL PlaySoundFile(CSoundFile *);
	BOOL PlaySoundFile(LPCSTR lpszFileName, UINT nNote=0);
	BOOL PlaySoundFile(CSoundFile *pSong, UINT nInstrument, UINT nSample, UINT nNote=0);
	BOOL PlayDLSInstrument(UINT nDLSBank, UINT nIns, UINT nRgn);
	BOOL StopSoundFile(CSoundFile *);
	inline BOOL IsPlaying() const { return (m_dwStatus & MODSTATUS_PLAYING); 	}
	inline BOOL IsRendering() const { return (m_dwStatus & MODSTATUS_RENDERING); 	} //rewbs.VSTTimeInfo
	DWORD GetElapsedTime() const { return m_dwElapsedTime; }
	void ResetElapsedTime() { m_dwElapsedTime = 0; }
	void SetElapsedTime(DWORD dwElapsedTime) { m_dwElapsedTime = dwElapsedTime; }
	inline CModDoc *GetModPlaying() const { return (IsPlaying()||IsRendering()) ? m_pModPlaying : NULL; }
	inline CSoundFile *GetSoundFilePlaying() const { return (IsPlaying()||IsRendering()) ? m_pSndFile : NULL; }  //rewbs.VSTTimeInfo
	BOOL InitRenderer(CSoundFile*);  //rewbs.VSTTimeInfo
	BOOL StopRenderer(CSoundFile*);  //rewbs.VSTTimeInfo
	void SwitchToActiveView();
	BOOL SetupSoundCard(DWORD q, DWORD rate, UINT nbits, UINT chns, UINT bufsize, LONG wd);
	BOOL SetupDirectories(LPCTSTR szModDir, LPCTSTR szSampleDir, LPCTSTR szInstrDir, LPCTSTR szVstDir, LPCTSTR szPresetDir);
	BOOL SetupMiscOptions();
	BOOL SetupPlayer(DWORD, DWORD, BOOL bForceUpdate=FALSE);
	BOOL SetupMidi(DWORD d, LONG n);
	void SetPreAmp(UINT n);
	HWND GetFollowSong(const CModDoc *pDoc) const { return (pDoc == m_pModPlaying) ? m_hFollowSong : NULL; }
	BOOL SetFollowSong(CModDoc *, HWND hwnd, BOOL bFollowSong=TRUE, DWORD dwType=MPTNOTIFY_DEFAULT);
	BOOL ResetNotificationBuffer(HWND hwnd=NULL);


// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	//}}AFX_VIRTUAL

	/// Opens either template or example menu item.
	void OpenMenuItemFile(const UINT nId, const bool bTemplateFile);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Message map functions
	//{{AFX_MSG(CMainFrame)
public:
	afx_msg void OnAddDlsBank();
	afx_msg void OnImportMidiLib();
	afx_msg void SetLastMixActiveTime(); //rewbs.VSTCompliance
	afx_msg void OnViewOptions();		 //rewbs.resamplerConf: made public so it's accessible from mod2wav gui :/
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT);
	afx_msg void OnSongProperties();

// -> CODE#0002
// -> DESC="list box to choose VST plugin presets (programs)"
	afx_msg void OnPluginManager();
// -! NEW_FEATURE#0002

	

// -> CODE#0015
// -> DESC="channels management dlg"
	afx_msg void OnChannelManager();
// -! NEW_FEATURE#0015

	afx_msg void OnUpdateTime(CCmdUI *pCmdUI);
	afx_msg void OnUpdateUser(CCmdUI *pCmdUI);
	afx_msg void OnUpdateInfo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateXInfo(CCmdUI *pCmdUI); //rewbs.xinfo
	afx_msg void OnUpdateCPU(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMidiRecord(CCmdUI *pCmdUI);
	afx_msg void OnPlayerPause();
	afx_msg void OnMidiRecord();
	afx_msg void OnPrevOctave();
	afx_msg void OnNextOctave();
	afx_msg void OnOctaveChanged();
	afx_msg void OnPanic();
	afx_msg void OnReportBug();	//rewbs.customKeys
	afx_msg BOOL OnInternetLink(UINT nID);
	afx_msg LRESULT OnUpdatePosition(WPARAM, LPARAM lParam);
	afx_msg void OnExampleSong(UINT nId);
	afx_msg void OnOpenTemplateModule(UINT nId);
	afx_msg LRESULT OnInvalidatePatterns(WPARAM, LPARAM);
	afx_msg LRESULT OnSpecialKey(WPARAM, LPARAM);
	afx_msg LRESULT OnCustomKeyMsg(WPARAM, LPARAM);
	afx_msg void OnViewMIDIMapping();
	afx_msg void OnViewEditHistory();
	afx_msg void OnInternetUpdate();
	afx_msg void OnShowSettingsFolder();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnInitMenu(CMenu* pMenu);
	bool UpdateEffectKeys(void); 
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	// Defines maximum number of items in example modules menu.
	static const size_t nMaxItemsInExampleModulesMenu = 50; 
	static const size_t nMaxItemsInTemplateModulesMenu = 50; 

	/// Array of paths of example modules that are available from help menu.
	static std::vector<CString> s_ExampleModulePaths;
	/// Array of paths of template modules that are available from file menu.
	static std::vector<CString> s_TemplateModulePaths;
};

// Critical section handling done in (safe) RAII style.
// Create a CriticalSection object whenever you need exclusive access to CSoundFile.
// One object = one lock / critical section.
// The critical section is automatically left when the object is destroyed, but
// Enter() and Leave() can also be called manually if needed.
class CriticalSection
{
protected:
	bool inSection;
public:
	CriticalSection() { inSection = false; Enter(); };
	~CriticalSection() { Leave(); };
	void Enter() { if(!inSection) { inSection = true; EnterCriticalSection(&CMainFrame::m_csAudio); } };
	void Leave() { if(inSection) { inSection = false; LeaveCriticalSection(&CMainFrame::m_csAudio); } };
};

const CHAR gszBuildDate[] = __DATE__ " " __TIME__;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
