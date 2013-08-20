/*
 * PatternEditorDialogs.h
 * ----------------------
 * Purpose: Code for various dialogs that are used in the pattern editor.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "dlg_misc.h"	// for keyboard control
#include "Moddoc.h"		// for SplitKeyboardSettings
#include "EffectInfo.h"
#include "PatternCursor.h"


/////////////////////////////////////////////////////////////////////////
// Search/Replace

//=========================================
class CFindReplaceTab: public CPropertyPage
//=========================================
{
protected:
	CSoundFile &sndFile;
	EffectInfo effectInfo;
	bool m_bReplace;	// is this the replace tab?

public:
	FlagSet<FindReplace::Flags> m_Flags;
	ModCommand m_Cmd;
	CHANNELINDEX m_nMinChannel, m_nMaxChannel;
	signed char cInstrRelChange;
	bool m_bPatSel;

	enum findItem
	{
		findAny = NOTE_MIN_SPECIAL - 1
	};


	enum replaceItem
	{
		replaceNotePlusOne = NOTE_MAX + 1,
		replaceNoteMinusOne = NOTE_MAX + 2,
		replaceNotePlusOctave = NOTE_MAX + 3,
		replaceNoteMinusOctave = NOTE_MAX + 4,

		replaceInstrumentPlusOne = MAX_INSTRUMENTS + 1,
		replaceInstrumentMinusOne = MAX_INSTRUMENTS + 2,
	};

	// Make sure there's unused notes between NOTE_MAX and NOTE_MIN_SPECIAL.
	STATIC_ASSERT(NOTE_MIN_SPECIAL - 4 > NOTE_MAX);

protected:
	void ChangeEffect();
	void ChangeVolCmd();

public:
	CFindReplaceTab(UINT nIDD, bool bReplaceTab, CSoundFile &sf) : CPropertyPage(nIDD), effectInfo(sf), sndFile(sf) { m_bReplace = bReplaceTab; }

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	// When a combobox is focussed, check the corresponding checkbox.
	void CheckOnChange(int nIDButton) { CheckDlgButton(nIDButton, BST_CHECKED); CheckReplace(nIDButton); };
	afx_msg void OnNoteChanged()	{ CheckOnChange(IDC_CHECK1); };
	afx_msg void OnInstrChanged()	{ CheckOnChange(IDC_CHECK2); };
	afx_msg void OnVolCmdChanged()	{ CheckOnChange(IDC_CHECK3); ChangeVolCmd(); };
	afx_msg void OnVolumeChanged()	{ CheckOnChange(IDC_CHECK4); };
	afx_msg void OnEffectChanged()	{ CheckOnChange(IDC_CHECK5); ChangeEffect(); };
	afx_msg void OnParamChanged()	{ CheckOnChange(IDC_CHECK6); };
	// When a checkbox is checked, also check "Replace By".
	afx_msg void OnCheckNote()		{ CheckReplace(IDC_CHECK1); };
	afx_msg void OnCheckInstr()		{ CheckReplace(IDC_CHECK2); };
	afx_msg void OnCheckVolCmd()	{ CheckReplace(IDC_CHECK3); };
	afx_msg void OnCheckVolume()	{ CheckReplace(IDC_CHECK4); };
	afx_msg void OnCheckEffect()	{ CheckReplace(IDC_CHECK5); };
	afx_msg void OnCheckParam()		{ CheckReplace(IDC_CHECK6); };
	// Check "Replace By"
	afx_msg void CheckReplace(int nIDButton)	{ if(m_bReplace && IsDlgButtonChecked(nIDButton)) CheckDlgButton(IDC_CHECK7, BST_CHECKED); };

	afx_msg void OnCheckChannelSearch();
	DECLARE_MESSAGE_MAP()
};


//=========================================
class CPatternPropertiesDlg: public CDialog
//=========================================
{
protected:
	CModDoc &modDoc;
	PATTERNINDEX m_nPattern;

public:
	CPatternPropertiesDlg(CModDoc &modParent, PATTERNINDEX nPat, CWnd *parent=NULL):CDialog(IDD_PATTERN_PROPERTIES, parent), modDoc(modParent), m_nPattern(nPat) { }

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnHalfRowNumber();
	afx_msg void OnDoubleRowNumber();
	afx_msg void OnOverrideSignature();
	DECLARE_MESSAGE_MAP()
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
	CSoundFile &sndFile;
	EffectInfo effectInfo;
	CEditCommand *m_pParent;
	bool m_bInitialized;

public:
	CPageEditCommand(CSoundFile &sf, CEditCommand *parent, UINT id) : CPropertyPage(id), sndFile(sf), effectInfo(sf), m_pParent(parent), m_bInitialized(false) {};

	virtual ~CPageEditCommand() {}
	virtual BOOL OnInitDialog();
	virtual void Init(ModCommand&)=0;
	virtual void UpdateDialog() {}
};


//==========================================
class CPageEditNote: public CPageEditCommand
//==========================================
{
protected:
	ModCommand::NOTE m_nNote;
	ModCommand::INSTR m_nInstr;

public:
	CPageEditNote(CSoundFile &sf, CEditCommand *parent) : CPageEditCommand(sf, parent, IDD_PAGEEDITNOTE) {}
	void Init(ModCommand &m) { m_nNote = m.note; m_nInstr = m.instr; }
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
	ModCommand::VOLCMD m_nVolCmd;
	ModCommand::VOL m_nVolume;
	bool m_bIsParamControl;

public:
	CPageEditVolume(CSoundFile &sf, CEditCommand *parent) : CPageEditCommand(sf, parent, IDD_PAGEEDITVOLUME) {};
	void Init(ModCommand &m) { m_nVolCmd = m.volcmd; m_nVolume = m.vol; m_bIsParamControl = m.IsPcNote(); };
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
	ModCommand::COMMAND m_nCommand;
	ModCommand::PARAM m_nParam;
	PLUGINDEX m_nPlugin;
	UINT m_nPluginParam;
	bool m_bIsParamControl;
	// -> CODE#0010
	// -> DESC="add extended parameter mechanism to pattern effects"
	UINT m_nXParam, m_nMultiplier;
	// -! NEW_FEATURE#0010

	ModCommand* m_pModcommand;

public:
	CPageEditEffect(CSoundFile &sf, CEditCommand *parent) : CPageEditCommand(sf, parent, IDD_PAGEEDITEFFECT) {}
	// -> CODE#0010
	// -> DESC="add extended parameter mechanism to pattern effects"
	void Init(ModCommand &m) { m_nCommand = m.command; m_nParam = m.param; m_pModcommand = &m; m_bIsParamControl = m.IsPcNote(); m_nPlugin = m.instr; m_nPluginParam = m.GetValueVolCol();}
	void XInit(UINT xparam = 0, UINT multiplier = 1) { m_nXParam = xparam; m_nMultiplier = multiplier; }
	// -! NEW_FEATURE#0010
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
	ROWINDEX m_nRow;
	PATTERNINDEX m_nPattern;
	CHANNELINDEX m_nChannel;
	ModCommand m_Command;
	bool m_bModified;

public:
	CEditCommand();

public:
	BOOL SetParent(CWnd *parent, CModDoc *pModDoc);
	BOOL ShowEditWindow(PATTERNINDEX nPat, const PatternCursor &cursor);
	// -> CODE#0010
	// -> DESC="add extended parameter mechanism to pattern effects"
	void OnSelListChange();
	// -! NEW_FEATURE#0010
	void UpdateNote(ModCommand::NOTE note, ModCommand::INSTR instr);
	void UpdateVolume(ModCommand::VOLCMD volcmd, ModCommand::VOL vol);
	void UpdateEffect(ModCommand::COMMAND command, ModCommand::PARAM param);

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
public:
	afx_msg void OnDestroy();
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
	MPTChord &GetChord();

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
// Keyboard Split Settings (pattern editor)

//=========================================
class CSplitKeyboadSettings: public CDialog
//=========================================
{
protected:
	CComboBox m_CbnSplitInstrument, m_CbnSplitNote, m_CbnOctaveModifier, m_CbnSplitVolume;
	CSoundFile &sndFile;

public:
	SplitKeyboardSettings &m_Settings;

	CSplitKeyboadSettings(CWnd *parent, CSoundFile &sf, SplitKeyboardSettings &settings) : CDialog(IDD_KEYBOARD_SPLIT, parent), m_Settings(settings), sndFile(sf) { }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnOctaveModifierChanged();

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////
// Show channel properties from pattern editor

//===========================================
class QuickChannelProperties : public CDialog
//===========================================
{
protected:
	CModDoc *document;
	CHANNELINDEX channel;
	PATTERNINDEX pattern;
	bool visible;
	bool settingsChanged;

	CSliderCtrl volSlider, panSlider;
	CSpinButtonCtrl volSpin, panSpin;
	CEdit nameEdit;

public:
	QuickChannelProperties();
	~QuickChannelProperties();

	void Show(CModDoc *modDoc, CHANNELINDEX chn, PATTERNINDEX ptn, CPoint position);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	void UpdateDisplay();
	void PrepareUndo();

	afx_msg void OnActivate(UINT nState, CWnd *, BOOL);
	afx_msg void OnVolChanged();
	afx_msg void OnPanChanged();
	afx_msg void OnHScroll(UINT, UINT, CScrollBar *);
	afx_msg void OnMuteChanged();
	afx_msg void OnSurroundChanged();
	afx_msg void OnNameChanged();
	afx_msg void OnPrevChannel();
	afx_msg void OnNextChannel();
	afx_msg LRESULT OnCustomKeyMsg(WPARAM, LPARAM);

	BOOL PreTranslateMessage(MSG *pMsg);

	DECLARE_MESSAGE_MAP();
};
