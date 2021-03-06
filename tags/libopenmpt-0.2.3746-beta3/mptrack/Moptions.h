/*
 * Moptions.h
 * ----------
 * Purpose: Implementation of various setup dialogs.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once 

class COptionsKeyboard;

// Tab Order
enum {
	OPTIONS_PAGE_GENERAL=0,
	OPTIONS_PAGE_SOUNDCARD,
	OPTIONS_PAGE_MIXER,
	OPTIONS_PAGE_PLAYER,
	OPTIONS_PAGE_EQ,
	OPTIONS_PAGE_KEYBOARD,
	OPTIONS_PAGE_COLORS,
	OPTIONS_PAGE_MIDI,
	OPTIONS_PAGE_AUTOSAVE,
	OPTIONS_PAGE_UPDATE,
	OPTIONS_PAGE_ADVANCED,
};



//=========================================
class COptionsGeneral: public CPropertyPage
//=========================================
{
protected:
	CCheckListBox m_CheckList;

public:
	COptionsGeneral():CPropertyPage(IDD_OPTIONS_GENERAL) {}
	void BrowseForFolder(UINT nID);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnOptionSelChanged();
	afx_msg void OnSettingsChanged() { SetModified(TRUE); }
	afx_msg void OnBrowseSongs()	{ BrowseForFolder(IDC_OPTIONS_DIR_MODS); }
	afx_msg void OnBrowseSamples()	{ BrowseForFolder(IDC_OPTIONS_DIR_SAMPS); }
	afx_msg void OnBrowseInstruments() { BrowseForFolder(IDC_OPTIONS_DIR_INSTS); }
	afx_msg void OnBrowsePlugins() { BrowseForFolder(IDC_OPTIONS_DIR_VSTS); }
	afx_msg void OnBrowsePresets() { BrowseForFolder(IDC_OPTIONS_DIR_VSTPRESETS); }

//rewbs.customkeys: COptionsKeyboard moved to separate file
	DECLARE_MESSAGE_MAP();
};


#if defined(MPT_SETTINGS_CACHE)

//==========================================
class COptionsAdvanced: public CPropertyPage
//==========================================
{
protected:
	CListBox m_List;
	std::map<int, SettingPath> m_IndexToPath;

public:
	COptionsAdvanced():CPropertyPage(IDD_OPTIONS_ADVANCED) {}

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnOptionDblClick();
	afx_msg void OnSettingsChanged() { SetModified(TRUE); }

	void ReInit();

	DECLARE_MESSAGE_MAP();
};

#endif // MPT_SETTINGS_CACHE


//========================================
class COptionsColors: public CPropertyPage
//========================================
{
protected:
	COLORREF CustomColors[MAX_MODCOLORS];
	UINT m_nColorItem;
	CComboBox m_ComboItem;
	CButton m_BtnColor1, m_BtnColor2, m_BtnColor3, m_BtnPreview;
	CStatic m_TxtColor1, m_TxtColor2, m_TxtColor3;
	LPMODPLUGDIB m_pPreviewDib;

public:
	COptionsColors():CPropertyPage(IDD_OPTIONS_COLORS) { m_nColorItem = 0; m_pPreviewDib = NULL; }
	~COptionsColors() { if (m_pPreviewDib) delete m_pPreviewDib; m_pPreviewDib = NULL; }
	void SelectColor(COLORREF *);

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnSetActive();
	afx_msg void OnUpdateDialog();
	afx_msg void OnDrawItem(int nIdCtl, LPDRAWITEMSTRUCT lpdis);
	afx_msg void OnColorSelChanged();
	afx_msg void OnSettingsChanged();
	afx_msg void OnSelectColor1();
	afx_msg void OnSelectColor2();
	afx_msg void OnSelectColor3();
	afx_msg void OnPresetMPT();
	afx_msg void OnPresetFT2();
	afx_msg void OnPresetIT();
	afx_msg void OnPresetBuzz();
	afx_msg void OnLoadColorScheme();
	afx_msg void OnSaveColorScheme();
	afx_msg void OnPreviewChanged();
	DECLARE_MESSAGE_MAP();
};
