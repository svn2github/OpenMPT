/*
 * MainFrm.h
 * ---------
 * Purpose: Implementation of OpenMPT's main window code.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "Mptrack.h"
#include "InputHandler.h"
#include "../common/AudioCriticalSection.h"
#include "../common/mutex.h"
#include "../soundlib/Sndfile.h"
#include "../soundlib/Dither.h"

class CInputHandler;
class CModDoc;
class CAutoSaver;
class ISoundDevice;
class ISoundSource;

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


#define NUM_VUMETER_PENS		32


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

#define MAX_UPDATE_HISTORY		256 // same as SNDDEV_MAXBUFFERS
#include "Notification.h"

#define TIMERID_GUI 1
#define TIMERID_NOTIFY 2

#include "mainbar.h"
#include "TrackerSettings.h"
struct MODPLUGDIB;

//======================================================================================
class CMainFrame: public CMDIFrameWnd, public ISoundSource, public ISoundMessageReceiver
//======================================================================================
{
	DECLARE_DYNAMIC(CMainFrame)
	// static data
public:

	// Globals
	static UINT m_nLastOptionsPage;
	static HHOOK ghKbdHook;

	// GDI
	static HICON m_hIcon;
	static HFONT m_hGUIFont, m_hFixedFont, m_hLargeFixedFont;
	static HBRUSH brushGray, brushBlack, brushWhite, brushText, brushHighLight, brushHighLightRed, brushWindow, brushYellow;
//	static CBrush *pbrushBlack, *pbrushWhite;
	static HPEN penBlack, penDarkGray, penLightGray, penWhite, penHalfDarkGray, penSample, penEnvelope, penEnvelopeHighlight, penSeparator, penScratch, penGray00, penGray33, penGray40, penGray55, penGray80, penGray99, penGraycc, penGrayff;
	static HCURSOR curDragging, curNoDrop, curArrow, curNoDrop2, curVSplit;
	static MODPLUGDIB *bmpPatterns, *bmpNotes, *bmpVUMeters, *bmpVisNode, *bmpVisPcNode;
	static COLORREF gcolrefVuMeter[NUM_VUMETER_PENS * 2];	// General tab VU meters

public:

	// Low-Level Audio
	ISoundDevice *gpSoundDevice;
	UINT_PTR m_NotifyTimer;
	Dither m_Dither;

	static LONG gnLVuMeter, gnRVuMeter;
	static bool gnClipLeft, gnClipRight;

	// Midi Input
public:
	static HMIDIIN shMidiIn;

protected:

	CModTreeBar m_wndTree;
	CStatusBar m_wndStatusBar;
	CMainToolBar m_wndToolBar;
	CImageList m_ImageList;
	CSoundFile *m_pSndFile; // != NULL only when currently playing or rendering
	HWND m_hWndMidi;
	CSoundFile::samplecount_t m_dwTimeSec;
	UINT_PTR m_nTimer;
	UINT m_nAvgMixChn, m_nMixChn;
	// Misc
	CModDoc* m_pJustModifiedDoc;
	class COptionsSoundcard *m_SoundCardOptionsDialog;
	bool m_bOptionsLocked;

	// Notification Buffer
	Util::mutex m_NotificationBufferMutex; // to avoid deadlocks, this mutex should only be taken as a innermost lock, i.e. do not block on anything while holding this mutex
	Util::fixed_size_queue<Notification,MAX_UPDATE_HISTORY> m_NotifyBuffer;

	// Instrument preview in tree view
	CSoundFile m_WaveFile;

	CHAR m_szUserText[512], m_szInfoText[512], m_szXInfoText[512]; //rewbs.xinfo

public:
	CMainFrame(/*CString regKeyExtension*/);
	void Initialize();


// Low-Level Audio
public:
	static void UpdateDspEffects(CSoundFile &sndFile, bool reset=false);
	static void UpdateAudioParameters(CSoundFile &sndFile, bool reset=false);
	static void CalcStereoVuMeters(int *, unsigned long, unsigned long);

	// from ISoundSource
	void FillAudioBufferLocked(IFillAudioBuffer &callback);
	void AudioRead(const SoundDeviceSettings &settings, std::size_t numFrames, void *buffer);
	void AudioDone(const SoundDeviceSettings &settings, std::size_t numFrames, int64 streamPosition);
	
	// from ISoundMessageReceiver
	void AudioMessage(const std::string &str);

	bool audioTryOpeningDevice();
	bool audioOpenDevice();
	bool audioReopenDevice();
	void audioCloseDevice();
	bool IsAudioDeviceOpen() const;
	bool DoNotification(DWORD dwSamplesRead, int64 streamPosition);

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
	static void UpdateColors();
	static HICON GetModIcon() { return m_hIcon; }
	static HFONT GetGUIFont() { return m_hGUIFont; }
	static HFONT GetFixedFont() { return m_hFixedFont; }
	static HFONT GetLargeFixedFont() { return m_hLargeFixedFont; }
	static void UpdateAllViews(DWORD dwHint, CObject *pHint=NULL);
	static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
	static CInputHandler *m_InputHandler; 	//rewbs.customKeys
	static CAutoSaver *m_pAutoSaver; 		//rewbs.customKeys

	static bool WritePrivateProfileBool(const CString section, const CString key, const bool value, const CString iniFile);
	static bool GetPrivateProfileBool(const CString section, const CString key, const bool defaultValue, const CString iniFile);
	static bool WritePrivateProfileLong(const CString section, const CString key, const long value, const CString iniFile);
	static long GetPrivateProfileLong(const CString section, const CString key, const long defaultValue, const CString iniFile);
	static bool WritePrivateProfileDWord(const CString section, const CString key, const DWORD value, const CString iniFile);
	static DWORD GetPrivateProfileDWord(const CString section, const CString key, const DWORD defaultValue, const CString iniFile);
	static bool WritePrivateProfileCString(const CString section, const CString key, const CString value, const CString iniFile);
	static CString GetPrivateProfileCString(const CString section, const CString key, const CString defaultValue, const CString iniFile);


	// Misc functions
public:
	void SetUserText(LPCSTR lpszText);
	void SetInfoText(LPCSTR lpszText);
	void SetXInfoText(LPCSTR lpszText); //rewbs.xinfo
	void SetHelpText(LPCSTR lpszText);
	UINT GetBaseOctave() const;
	CModDoc *GetActiveDoc();
	CView *GetActiveView();  	//rewbs.customKeys
	CImageList *GetImageList() { return &m_ImageList; }
	void OnDocumentCreated(CModDoc *pModDoc);
	void OnDocumentClosed(CModDoc *pModDoc);
	void UpdateTree(CModDoc *pModDoc, DWORD lHint=0, CObject *pHint=NULL);
	static CInputHandler* GetInputHandler() { return m_InputHandler; }  	//rewbs.customKeys
	bool m_bModTreeHasFocus;  	//rewbs.customKeys
	CWnd *m_pNoteMapHasFocus;  	//rewbs.customKeys
	CWnd* m_pOrderlistHasFocus;
	double GetApproxBPM();
	void ThreadSafeSetModified(CModDoc* modified) {m_pJustModifiedDoc=modified;}
	void SetElapsedTime(double t) { m_dwTimeSec = static_cast<CSoundFile::samplecount_t>(t); }

	CModTree *GetUpperTreeview() { return m_wndTree.m_pModTree; }
	CModTree *GetLowerTreeview() { return m_wndTree.m_pModTreeData; }

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

	// high level synchronous playback functions, do not hold AudioCriticalSection while calling these
	bool PreparePlayback();
	bool StartPlayback();
	void StopPlayback();
	bool PausePlayback();
	static bool IsValidSoundFile(CSoundFile &sndFile) { return sndFile.GetType() ? true : false; }
	static bool IsValidSoundFile(CSoundFile *pSndFile) { return pSndFile && pSndFile->GetType(); }
	void SetPlaybackSoundFile(CSoundFile *pSndFile);
	void UnsetPlaybackSoundFile();
	void GenerateStopNotification();

	bool PlayMod(CModDoc *);
	bool StopMod(CModDoc *pDoc=NULL);
	bool PauseMod(CModDoc *pDoc=NULL);

	bool StopSoundFile(CSoundFile *);
	bool PlaySoundFile(CSoundFile *);
	BOOL PlaySoundFile(LPCSTR lpszFileName, ModCommand::NOTE note);
	BOOL PlaySoundFile(CSoundFile &sndFile, INSTRUMENTINDEX nInstrument, SAMPLEINDEX nSample, ModCommand::NOTE note);
	BOOL PlayDLSInstrument(UINT nDLSBank, UINT nIns, UINT nRgn, ModCommand::NOTE note);

	void InitPreview();
	void PreparePreview(ModCommand::NOTE note);
	void StopPreview() { StopSoundFile(&m_WaveFile); }

	inline bool IsPlaying() const { return m_pSndFile != nullptr; }
	inline CModDoc *GetModPlaying() const { return m_pSndFile ? m_pSndFile->GetpModDoc() : nullptr; }
	inline CSoundFile *GetSoundFilePlaying() const { return m_pSndFile; } // may be nullptr
	BOOL InitRenderer(CSoundFile*);
	BOOL StopRenderer(CSoundFile*);
	void SwitchToActiveView();

	BOOL SetupSoundCard(const SoundDeviceSettings &deviceSettings, SoundDeviceID deviceID);
	BOOL SetupMiscOptions();
	BOOL SetupPlayer();

	BOOL SetupDirectories(LPCTSTR szModDir, LPCTSTR szSampleDir, LPCTSTR szInstrDir, LPCTSTR szVstDir, LPCTSTR szPresetDir);
	BOOL SetupMidi(DWORD d, LONG n);
	void SetPreAmp(UINT n);
	HWND GetFollowSong() const;
	HWND GetFollowSong(const CModDoc *pDoc) const { return (pDoc == GetModPlaying()) ? GetFollowSong() : NULL; }
	void ResetNotificationBuffer();


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

	void OnTimerGUI();
	void OnTimerNotify();

// Message map functions
	//{{AFX_MSG(CMainFrame)
public:
	afx_msg void OnAddDlsBank();
	afx_msg void OnImportMidiLib();
	afx_msg void OnViewOptions();		 //rewbs.resamplerConf: made public so it's accessible from mod2wav gui :/
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR);
	afx_msg void OnSongProperties();

// -> CODE#0002
// -> DESC="list box to choose VST plugin presets (programs)"
	afx_msg void OnPluginManager();
// -! NEW_FEATURE#0002



	afx_msg void OnChannelManager();
	afx_msg void OnClipboardManager();

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
	bool UpdateEffectKeys();
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


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
