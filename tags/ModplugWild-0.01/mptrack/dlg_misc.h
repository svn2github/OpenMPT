#ifndef _MPT_DLG_MISC_H_
#define _MPT_DLG_MISC_H_

class CSoundFile;
class CModDoc;
class CDLSBank;

//===============================
class CModTypeDlg: public CDialog
//===============================
{
public:
	CComboBox m_TypeBox, m_ChannelsBox;
	CButton m_CheckBox1, m_CheckBox2, m_CheckBox3, m_CheckBox4, m_CheckBox5;
	CSoundFile *m_pSndFile;
	UINT m_nChannels, m_nType;

public:
	CModTypeDlg(CSoundFile *pSndFile, CWnd *parent):CDialog(IDD_MODDOC_MODTYPE, parent) { m_pSndFile = pSndFile; m_nType = m_nChannels = 0; }
	void UpdateDialog();

protected:
	//{{AFX_VIRTUAL(CModTypeDlg)
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL
	//{{AFX_MSG(CModTypeDlg)
	afx_msg void OnCheck1();
	afx_msg void OnCheck2();
	afx_msg void OnCheck3();
	afx_msg void OnCheck4();
	afx_msg void OnCheck5();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//===============================
class CShowLogDlg: public CDialog
//===============================
{
public:
	LPCSTR m_lpszLog, m_lpszTitle;
	CEdit m_EditLog;

public:
	CShowLogDlg(CWnd *parent=NULL):CDialog(IDD_SHOWLOG, parent) { m_lpszLog = NULL; m_lpszTitle = NULL; }
	UINT ShowLog(LPCSTR pszLog, LPCSTR lpszTitle=NULL);

protected:
	//{{AFX_VIRTUAL(CShowLogDlg)
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL
	DECLARE_MESSAGE_MAP()
};


//======================================
class CRemoveChannelsDlg: public CDialog
//======================================
{
public:
	CSoundFile *m_pSndFile;
	BOOL m_bChnMask[MAX_CHANNELS];
	UINT m_nChannels, m_nRemove;

public:
	CRemoveChannelsDlg(CSoundFile *pSndFile, UINT nChns, CWnd *parent=NULL):CDialog(IDD_REMOVECHANNELS, parent)
		{ m_pSndFile = pSndFile; m_nChannels = m_pSndFile->m_nChannels; m_nRemove = nChns; memset(m_bChnMask, 0, sizeof(m_bChnMask)); }

protected:
	//{{AFX_VIRTUAL(CRemoveChannelsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL
	//{{AFX_MSG(CRemoveChannelsDlg)
	afx_msg void OnChannelChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();
};


/////////////////////////////////////////////////////////////////////////
// Search/Replace

#define PATSEARCH_NOTE			0x01
#define PATSEARCH_INSTR			0x02
#define PATSEARCH_VOLCMD		0x04
#define PATSEARCH_VOLUME		0x08
#define PATSEARCH_COMMAND		0x10
#define PATSEARCH_PARAM			0x20
#define PATSEARCH_CHANNEL		0x40
#define PATSEARCH_FULLSEARCH	0x100
#define PATSEARCH_REPLACE		0x200
#define PATSEARCH_REPLACEALL	0x400

//=========================================
class CFindReplaceTab: public CPropertyPage
//=========================================
{
protected:
	BOOL m_bReplace;
	CModDoc *m_pModDoc;

public:
	UINT m_nNote, m_nInstr, m_nVolCmd, m_nVol, m_nCommand, m_nParam, m_nMinChannel, m_nMaxChannel;
	DWORD m_dwFlags;

public:
	CFindReplaceTab(UINT nIDD, BOOL bReplaceTab, CModDoc *pModDoc):CPropertyPage(nIDD) { m_bReplace = bReplaceTab; m_pModDoc = pModDoc; }

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEffectChanged();
	afx_msg void OnCheckChannelSearch();
	DECLARE_MESSAGE_MAP()
};


//=========================================
class CPatternPropertiesDlg: public CDialog
//=========================================
{
protected:
	CModDoc *m_pModDoc;
	UINT m_nPattern;

public:
	CPatternPropertiesDlg(CModDoc *pModDoc, UINT nPat, CWnd *parent=NULL):CDialog(IDD_PATTERN_PROPERTIES, parent) { m_pModDoc = pModDoc; m_nPattern = nPat; }

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};


//////////////////////////////////////////////////////////////////////////
// Command Editing

class CEditCommand;
class CPageEditCommand;

//==========================================
class CPageEditCommand: public CPropertyPage
//==========================================
{
protected:
	CModDoc *m_pModDoc;
	CEditCommand *m_pParent;
	BOOL m_bInitialized;

public:
	CPageEditCommand(CModDoc *pModDoc, CEditCommand *parent, UINT id):CPropertyPage(id) { m_pModDoc = pModDoc; m_pParent = parent; m_bInitialized = FALSE; }
	virtual ~CPageEditCommand() {}
	virtual BOOL OnInitDialog();
	virtual void Init(MODCOMMAND&)=0;
	virtual void UpdateDialog() {}
};


//==========================================
class CPageEditNote: public CPageEditCommand
//==========================================
{
protected:
	UINT m_nNote, m_nInstr;

public:
	CPageEditNote(CModDoc *pModDoc, CEditCommand *parent):CPageEditCommand(pModDoc, parent, IDD_PAGEEDITNOTE) {}
	void Init(MODCOMMAND &m) { m_nNote = m.note; m_nInstr = m.instr; }
	void UpdateDialog();

protected:
	//{{AFX_MSG(CPageEditNote)
	afx_msg void OnNoteChanged();
	afx_msg void OnInstrChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//============================================
class CPageEditVolume: public CPageEditCommand
//============================================
{
protected:
	UINT m_nVolCmd, m_nVolume;

public:
	CPageEditVolume(CModDoc *pModDoc, CEditCommand *parent):CPageEditCommand(pModDoc, parent, IDD_PAGEEDITVOLUME) {}
	void Init(MODCOMMAND &m) { m_nVolCmd = m.volcmd; m_nVolume = m.vol; }
	void UpdateDialog();
	void UpdateRanges();

protected:
	//{{AFX_MSG(CPageEditVolume)
	afx_msg void OnVolCmdChanged();
	afx_msg void OnHScroll(UINT, UINT, CScrollBar *);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//============================================
class CPageEditEffect: public CPageEditCommand
//============================================
{
protected:
	UINT m_nCommand, m_nParam;

public:
	CPageEditEffect(CModDoc *pModDoc, CEditCommand *parent):CPageEditCommand(pModDoc, parent, IDD_PAGEEDITEFFECT) {}
	void Init(MODCOMMAND &m) { m_nCommand = m.command; m_nParam = m.param; }
	void UpdateDialog();
	void UpdateRange(BOOL bSet);
	void UpdateValue(BOOL bSet);

protected:
	//{{AFX_MSG(CPageEditEffect)
	afx_msg void OnCommandChanged();
	afx_msg void OnHScroll(UINT, UINT, CScrollBar *);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//=======================================
class CEditCommand: public CPropertySheet
//=======================================
{
protected:
	CPageEditNote *m_pageNote;
	CPageEditVolume *m_pageVolume;
	CPageEditEffect *m_pageEffect;
	CModDoc *m_pModDoc;
	HWND m_hWndView;
	UINT m_nPattern, m_nRow, m_nChannel;
	MODCOMMAND m_Command;

public:
	CEditCommand();

public:
	BOOL SetParent(CWnd *parent, CModDoc *pModDoc);
	BOOL ShowEditWindow(UINT nPat, DWORD dwCursor);
	void UpdateNote(UINT note, UINT instr);
	void UpdateVolume(UINT volcmd, UINT vol);
	void UpdateEffect(UINT command, UINT param);

protected:
	//{{AFX_VIRTUAL(CEditCommand)
	virtual void OnOK()		{ ShowWindow(SW_HIDE); }
	virtual void OnCancel()	{ ShowWindow(SW_HIDE); }
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	//}}AFX_VIRTUAL
	//{{AFX_MSG(CEditCommand)
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg void OnClose()	{ ShowWindow(SW_HIDE); }
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Samples

//===========================
class CAmpDlg: public CDialog
//===========================
{
public:
	UINT m_nFactor;
	BOOL m_bFadeIn, m_bFadeOut;

public:
	CAmpDlg(CWnd *parent, UINT nFactor=100):CDialog(IDD_SAMPLE_AMPLIFY, parent) { m_nFactor = nFactor; m_bFadeIn = m_bFadeOut = FALSE; }
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};


////////////////////////////////////////////////////////////////////////
// Sound Banks

//========================================
class CSoundBankProperties: public CDialog
//========================================
{
protected:
	CHAR m_szInfo[4096];

public:
	CSoundBankProperties(CDLSBank *pBank, CWnd *parent=NULL);
	virtual BOOL OnInitDialog();
};


////////////////////////////////////////////////////////////////////////
// Midi Macros (Zxx)

//===================================
class CMidiMacroSetup: public CDialog
//===================================
{
public:
	MODMIDICFG m_MidiCfg;
	BOOL m_bEmbed;
	CComboBox m_CbnSFx, m_CbnSFxPreset, m_CbnZxx, m_CbnZxxPreset;
	CEdit m_EditSFx, m_EditZxx;

public:
	CMidiMacroSetup(MODMIDICFG *pcfg, BOOL bEmbed, CWnd *parent=NULL);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void UpdateDialog();
	afx_msg void OnSetAsDefault();
	afx_msg void OnResetCfg();
	afx_msg void OnEmbedMidiCfg();
	afx_msg void OnSFxChanged();
	afx_msg void OnSFxPresetChanged();
	afx_msg void OnZxxPresetChanged();
	afx_msg void OnSFxEditChanged();
	afx_msg void OnZxxEditChanged();
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////
// Keyboard control

enum {
	KBDNOTIFY_MOUSEMOVE=0,
	KBDNOTIFY_LBUTTONDOWN,
	KBDNOTIFY_LBUTTONUP,
};

//=================================
class CKeyboardControl: public CWnd
//=================================
{
public:
	enum {
		KEYFLAG_NORMAL=0,
		KEYFLAG_REDDOT,
	};
protected:
	HWND m_hParent;
	UINT m_nOctaves;
	int m_nSelection;
	BOOL m_bCapture, m_bCursorNotify;
	BYTE KeyFlags[120]; // 10 octaves max

public:
	CKeyboardControl() { m_hParent = NULL; m_nOctaves = 1; m_nSelection = -1; m_bCapture = FALSE; }

public:
	void Init(HWND parent, UINT nOctaves=1, BOOL bCursNotify=FALSE) { m_hParent = parent; 
				m_nOctaves = nOctaves; m_bCursorNotify = bCursNotify; memset(KeyFlags, 0, sizeof(KeyFlags)); }
	void SetFlags(UINT key, UINT flags) { if (key < 120) KeyFlags[key] = (BYTE)flags; }
	UINT GetFlags(UINT key) const { return (key < 120) ? KeyFlags[key] : 0; }
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////
// Chord Editor

//================================
class CChordEditor: public CDialog
//================================
{
protected:
	CKeyboardControl m_Keyboard;
	CComboBox m_CbnShortcut, m_CbnBaseNote, m_CbnNote1, m_CbnNote2, m_CbnNote3;

public:
	CChordEditor(CWnd *parent=NULL):CDialog(IDD_CHORDEDIT, parent) {}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void UpdateKeyboard();
	afx_msg LRESULT OnKeyboardNotify(WPARAM, LPARAM);
	afx_msg void OnChordChanged();
	afx_msg void OnBaseNoteChanged();
	afx_msg void OnNote1Changed();
	afx_msg void OnNote2Changed();
	afx_msg void OnNote3Changed();
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////
// Sample Map

//=================================
class CSampleMapDlg: public CDialog
//=================================
{
protected:
	CKeyboardControl m_Keyboard;
	CComboBox m_CbnSample;
	CSliderCtrl m_SbOctave;
	CSoundFile *m_pSndFile;
	UINT m_nInstrument;
	BYTE KeyboardMap[120];

public:
	CSampleMapDlg(CSoundFile *pSndFile, UINT nInstr, CWnd *parent=NULL):CDialog(IDD_EDITSAMPLEMAP, parent)
		{ m_pSndFile = pSndFile; m_nInstrument = nInstr; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual VOID OnOK();
	afx_msg void OnUpdateSamples();
	afx_msg void OnUpdateKeyboard();
	afx_msg void OnUpdateOctave();
	afx_msg void OnHScroll(UINT, UINT, CScrollBar *);
	afx_msg LRESULT OnKeyboardNotify(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};


#endif
