#include "stdafx.h"
#include "mptrack.h"
#include "moddoc.h"
#include "mainfrm.h"
#include "dlg_misc.h"
#include "dlsbank.h"

#pragma warning(disable:4244)

///////////////////////////////////////////////////////////////////////
// CModTypeDlg


BEGIN_MESSAGE_MAP(CModTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CModTypeDlg)
	ON_COMMAND(IDC_CHECK1,		OnCheck1)
	ON_COMMAND(IDC_CHECK2,		OnCheck2)
	ON_COMMAND(IDC_CHECK3,		OnCheck3)
	ON_COMMAND(IDC_CHECK4,		OnCheck4)
	ON_COMMAND(IDC_CHECK5,		OnCheck5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CModTypeDlg::DoDataExchange(CDataExchange* pDX)
//--------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModTypeDlg)
	DDX_Control(pDX, IDC_COMBO1,		m_TypeBox);
	DDX_Control(pDX, IDC_COMBO2,		m_ChannelsBox);
	DDX_Control(pDX, IDC_CHECK1,		m_CheckBox1);
	DDX_Control(pDX, IDC_CHECK2,		m_CheckBox2);
	DDX_Control(pDX, IDC_CHECK3,		m_CheckBox3);
	DDX_Control(pDX, IDC_CHECK4,		m_CheckBox4);
	DDX_Control(pDX, IDC_CHECK5,		m_CheckBox5);
	//}}AFX_DATA_MAP
}


BOOL CModTypeDlg::OnInitDialog()
//------------------------------
{
	CHAR s[256];
	CDialog::OnInitDialog();
	m_nType = m_pSndFile->m_nType;
	m_nChannels = m_pSndFile->m_nChannels;
	m_TypeBox.SetItemData(m_TypeBox.AddString("ProTracker MOD"), MOD_TYPE_MOD);
	m_TypeBox.SetItemData(m_TypeBox.AddString("ScreamTracker S3M"), MOD_TYPE_S3M);
	m_TypeBox.SetItemData(m_TypeBox.AddString("FastTracker XM"), MOD_TYPE_XM);
	m_TypeBox.SetItemData(m_TypeBox.AddString("Impulse Tracker IT"), MOD_TYPE_IT);
	switch(m_nType)
	{
	case MOD_TYPE_S3M:	m_TypeBox.SetCurSel(1); break;
	case MOD_TYPE_XM:	m_TypeBox.SetCurSel(2); break;
	case MOD_TYPE_IT:	m_TypeBox.SetCurSel(3); break;
	default:			m_TypeBox.SetCurSel(0); break;
	}
	for (int i=4; i<=64; i++)
	{
		wsprintf(s, "%d Channels", i);
		m_ChannelsBox.SetItemData(m_ChannelsBox.AddString(s), i);
	}
	m_ChannelsBox.SetCurSel(m_nChannels-4);
	UpdateDialog();
	return TRUE;
}


void CModTypeDlg::UpdateDialog()
//------------------------------
{
	m_CheckBox1.SetCheck((m_pSndFile->m_dwSongFlags & SONG_LINEARSLIDES) ? MF_CHECKED : 0);
	m_CheckBox2.SetCheck((m_pSndFile->m_dwSongFlags & SONG_FASTVOLSLIDES) ? MF_CHECKED : 0);
	m_CheckBox3.SetCheck((m_pSndFile->m_dwSongFlags & SONG_ITOLDEFFECTS) ? MF_CHECKED : 0);
	m_CheckBox4.SetCheck((m_pSndFile->m_dwSongFlags & SONG_ITCOMPATMODE) ? MF_CHECKED : 0);
	m_CheckBox5.SetCheck((m_pSndFile->m_dwSongFlags & SONG_EXFILTERRANGE) ? MF_CHECKED : 0);
	m_CheckBox1.EnableWindow((m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) ? TRUE : FALSE);
	m_CheckBox2.EnableWindow((m_pSndFile->m_nType == MOD_TYPE_S3M) ? TRUE : FALSE);
	m_CheckBox3.EnableWindow((m_pSndFile->m_nType == MOD_TYPE_IT) ? TRUE : FALSE);
	m_CheckBox4.EnableWindow((m_pSndFile->m_nType == MOD_TYPE_IT) ? TRUE : FALSE);
	m_CheckBox5.EnableWindow((m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) ? TRUE : FALSE);
}


void CModTypeDlg::OnCheck1()
//--------------------------
{
	if (m_CheckBox1.GetCheck())
		m_pSndFile->m_dwSongFlags |= SONG_LINEARSLIDES;
	else
		m_pSndFile->m_dwSongFlags &= ~SONG_LINEARSLIDES;
}


void CModTypeDlg::OnCheck2()
//--------------------------
{
	if (m_CheckBox2.GetCheck())
		m_pSndFile->m_dwSongFlags |= SONG_FASTVOLSLIDES;
	else
		m_pSndFile->m_dwSongFlags &= ~SONG_FASTVOLSLIDES;
}


void CModTypeDlg::OnCheck3()
//--------------------------
{
	if (m_CheckBox3.GetCheck())
		m_pSndFile->m_dwSongFlags |= SONG_ITOLDEFFECTS;
	else
		m_pSndFile->m_dwSongFlags &= ~SONG_ITOLDEFFECTS;
}


void CModTypeDlg::OnCheck4()
//--------------------------
{
	if (m_CheckBox4.GetCheck())
		m_pSndFile->m_dwSongFlags |= SONG_ITCOMPATMODE;
	else
		m_pSndFile->m_dwSongFlags &= ~SONG_ITCOMPATMODE;
}


void CModTypeDlg::OnCheck5()
//--------------------------
{
	if (m_CheckBox5.GetCheck())
		m_pSndFile->m_dwSongFlags |= SONG_EXFILTERRANGE;
	else
		m_pSndFile->m_dwSongFlags &= ~SONG_EXFILTERRANGE;
}


void CModTypeDlg::OnOK()
//----------------------
{
	int sel = m_TypeBox.GetCurSel();
	if (sel >= 0)
	{
		m_nType = m_TypeBox.GetItemData(sel);
	}
	sel = m_ChannelsBox.GetCurSel();
	if (sel >= 0)
	{
		m_nChannels = m_ChannelsBox.GetItemData(sel);
		//if (m_nType & MOD_TYPE_XM) m_nChannels = (m_nChannels+1) & 0xFE;
	}
	CDialog::OnOK();
}


//////////////////////////////////////////////////////////////////////////////
// CShowLogDlg

BEGIN_MESSAGE_MAP(CShowLogDlg, CDialog)
	//{{AFX_MSG_MAP(CShowLogDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CShowLogDlg::DoDataExchange(CDataExchange* pDX)
//--------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShowLogDlg)
	DDX_Control(pDX, IDC_EDIT_LOG,		m_EditLog);
	//}}AFX_DATA_MAP
}


BOOL CShowLogDlg::OnInitDialog()
//------------------------------
{
	CDialog::OnInitDialog();
	if (m_lpszTitle) SetWindowText(m_lpszTitle);
	m_EditLog.SetSel(0, -1);
	m_EditLog.ReplaceSel(m_lpszLog);
	m_EditLog.SetFocus();
	m_EditLog.SetSel(0, 0);
	return FALSE;
}


UINT CShowLogDlg::ShowLog(LPCSTR pszLog, LPCSTR lpszTitle)
//--------------------------------------------------------
{
	m_lpszLog = pszLog;
	m_lpszTitle = lpszTitle;
	return DoModal();
}


///////////////////////////////////////////////////////////
// CRemoveChannelsDlg

const WORD nCheckControls[64] =
{
	IDC_CHECK1,  IDC_CHECK2,  IDC_CHECK3,  IDC_CHECK4,
	IDC_CHECK5,  IDC_CHECK6,  IDC_CHECK7,  IDC_CHECK8,
	IDC_CHECK9,  IDC_CHECK10, IDC_CHECK11, IDC_CHECK12,
	IDC_CHECK13, IDC_CHECK14, IDC_CHECK15, IDC_CHECK16,
	IDC_CHECK17, IDC_CHECK18, IDC_CHECK19, IDC_CHECK20,
	IDC_CHECK21, IDC_CHECK22, IDC_CHECK23, IDC_CHECK24,
	IDC_CHECK25, IDC_CHECK26, IDC_CHECK27, IDC_CHECK28,
	IDC_CHECK29, IDC_CHECK30, IDC_CHECK31, IDC_CHECK32,
	IDC_CHECK33, IDC_CHECK34, IDC_CHECK35, IDC_CHECK36,
	IDC_CHECK37, IDC_CHECK38, IDC_CHECK39, IDC_CHECK40,
	IDC_CHECK41, IDC_CHECK42, IDC_CHECK43, IDC_CHECK44,
	IDC_CHECK45, IDC_CHECK46, IDC_CHECK47, IDC_CHECK48,
	IDC_CHECK49, IDC_CHECK50, IDC_CHECK51, IDC_CHECK52,
	IDC_CHECK53, IDC_CHECK54, IDC_CHECK55, IDC_CHECK56,
	IDC_CHECK57, IDC_CHECK58, IDC_CHECK59, IDC_CHECK60,
	IDC_CHECK61, IDC_CHECK62, IDC_CHECK63, IDC_CHECK64
};


const WORD nTextControls[64] =
{
	NULL,		NULL,		NULL,		NULL,
	IDC_TEXT5,	IDC_TEXT6,	IDC_TEXT7,	IDC_TEXT8,
	IDC_TEXT9,	IDC_TEXT10,	IDC_TEXT11,	IDC_TEXT12,
	IDC_TEXT13,	IDC_TEXT14,	IDC_TEXT15,	IDC_TEXT16,
	IDC_TEXT17,	IDC_TEXT18,	IDC_TEXT19,	IDC_TEXT20,
	IDC_TEXT21,	IDC_TEXT22,	IDC_TEXT23,	IDC_TEXT24,
	IDC_TEXT25,	IDC_TEXT26,	IDC_TEXT27,	IDC_TEXT28,
	IDC_TEXT29,	IDC_TEXT30,	IDC_TEXT31,	IDC_TEXT32,
	IDC_TEXT61,	IDC_TEXT62,	IDC_TEXT63,	IDC_TEXT64,
	IDC_TEXT33,	IDC_TEXT34,	IDC_TEXT35,	IDC_TEXT36,
	IDC_TEXT37,	IDC_TEXT38,	IDC_TEXT39,	IDC_TEXT40,
	IDC_TEXT41,	IDC_TEXT42,	IDC_TEXT43,	IDC_TEXT44,
	IDC_TEXT45,	IDC_TEXT46,	IDC_TEXT47,	IDC_TEXT48,
	IDC_TEXT49,	IDC_TEXT50,	IDC_TEXT51,	IDC_TEXT52,
	IDC_TEXT53,	IDC_TEXT54,	IDC_TEXT55,	IDC_TEXT56,
	IDC_TEXT57,	IDC_TEXT58,	IDC_TEXT59,	IDC_TEXT60
};



BEGIN_MESSAGE_MAP(CRemoveChannelsDlg, CDialog)
	//{{AFX_MSG_MAP(CRemoveChannelsDlg)
	ON_COMMAND(IDC_CHECK1,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK2,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK3,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK4,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK5,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK6,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK7,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK8,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK9,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK10,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK11,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK12,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK13,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK14,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK15,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK16,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK17,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK18,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK19,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK20,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK21,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK22,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK23,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK24,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK25,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK26,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK27,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK28,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK29,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK30,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK31,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK32,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK33,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK34,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK35,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK36,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK37,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK38,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK39,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK40,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK41,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK42,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK43,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK44,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK45,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK46,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK47,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK48,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK49,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK50,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK51,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK52,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK53,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK54,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK55,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK56,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK57,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK58,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK59,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK60,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK61,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK62,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK63,	OnChannelChanged)
	ON_COMMAND(IDC_CHECK64,	OnChannelChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CRemoveChannelsDlg::OnInitDialog()
//-------------------------------------
{
	CHAR s[128];
	CDialog::OnInitDialog();
	for (UINT n=0; n<64; n++)
	{
		if (m_bChnMask[n]) CheckDlgButton(nCheckControls[n], MF_CHECKED);
		::ShowWindow(::GetDlgItem(m_hWnd, nCheckControls[n]), (n < m_nChannels) ? SW_SHOW : SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, nTextControls[n]), (n < m_nChannels) ? SW_SHOW : SW_HIDE);
	}
	wsprintf(s, "Select %d channels to remove:", m_nRemove);
	SetDlgItemText(IDC_QUESTION1, s);
	OnChannelChanged();
	return TRUE;
}


void CRemoveChannelsDlg::OnOK()
//-----------------------------
{
	UINT nr = 0;
	memset(m_bChnMask, 0, sizeof(m_bChnMask));
	for (UINT n=0; n<m_nChannels; n++) if (IsDlgButtonChecked(nCheckControls[n]))
	{
		nr++;
		m_bChnMask[n]++;
	}
	if (nr == m_nRemove)
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}


void CRemoveChannelsDlg::OnChannelChanged()
//-----------------------------------------
{
	UINT nr = 0;
	for (UINT n=0; n<m_nChannels; n++) if (IsDlgButtonChecked(nCheckControls[n])) nr++;
	GetDlgItem(IDOK)->EnableWindow((nr == m_nRemove) ? TRUE : FALSE);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Find/Replace Dialog

BEGIN_MESSAGE_MAP(CFindReplaceTab, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO5,	OnEffectChanged)
	ON_COMMAND(IDC_CHECK7,			OnCheckChannelSearch)
END_MESSAGE_MAP()


BOOL CFindReplaceTab::OnInitDialog()
//----------------------------------
{
	CHAR s[256];
	CComboBox *combo;
	CSoundFile *pSndFile;

	CPropertyPage::OnInitDialog();
	if (!m_pModDoc) return TRUE;
	pSndFile = m_pModDoc->GetSoundFile();
	// Search flags
	if (m_dwFlags & PATSEARCH_NOTE) CheckDlgButton(IDC_CHECK1, MF_CHECKED);
	if (m_dwFlags & PATSEARCH_INSTR) CheckDlgButton(IDC_CHECK2, MF_CHECKED);
	if (m_dwFlags & PATSEARCH_VOLCMD) CheckDlgButton(IDC_CHECK3, MF_CHECKED);
	if (m_dwFlags & PATSEARCH_VOLUME) CheckDlgButton(IDC_CHECK4, MF_CHECKED);
	if (m_dwFlags & PATSEARCH_COMMAND) CheckDlgButton(IDC_CHECK5, MF_CHECKED);
	if (m_dwFlags & PATSEARCH_PARAM) CheckDlgButton(IDC_CHECK6, MF_CHECKED);
	if (m_bReplace)
	{
		if (m_dwFlags & PATSEARCH_REPLACE) CheckDlgButton(IDC_CHECK7, MF_CHECKED);
		if (m_dwFlags & PATSEARCH_REPLACEALL) CheckDlgButton(IDC_CHECK8, MF_CHECKED);
	} else
	{
		if (m_dwFlags & PATSEARCH_CHANNEL) CheckDlgButton(IDC_CHECK7, MF_CHECKED);
		CheckRadioButton(IDC_RADIO1, IDC_RADIO2, (m_dwFlags & PATSEARCH_FULLSEARCH) ? IDC_RADIO2 : IDC_RADIO1);
		SetDlgItemInt(IDC_EDIT1, m_nMinChannel+1);
		SetDlgItemInt(IDC_EDIT2, m_nMaxChannel+1);
	}
	// Note
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL)
	{
		combo->SetItemData(combo->AddString("..."), 0);
		if (m_bReplace)
		{
			combo->SetItemData(combo->AddString("note-1"), 0xFC);
			combo->SetItemData(combo->AddString("note+1"), 0xFD);
			combo->SetItemData(combo->AddString("-1 oct"), 0xFA);
			combo->SetItemData(combo->AddString("+1 oct"), 0xFB);
		} else
		{
			combo->SetItemData(combo->AddString("any"), 0xFD);
		}
		for (UINT nNote=1; nNote<=120; nNote++)
		{
			wsprintf(s, "%s%d", szNoteNames[(nNote-1) % 12], (nNote-1)/12);
			combo->SetItemData(combo->AddString(s), nNote);
		}
		combo->SetItemData(combo->AddString("^^"), 0xFE);
		combo->SetItemData(combo->AddString("=="), 0xFF);
		UINT ncount = combo->GetCount();
		for (UINT i=0; i<ncount; i++) if (m_nNote == combo->GetItemData(i))
		{
			combo->SetCurSel(i);
			break;
		}
	}
	// Instrument
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO2)) != NULL)
	{
		combo->SetItemData(combo->AddString(".."), 0);
		if (m_bReplace)
		{
			combo->SetItemData(combo->AddString("ins-1"), 0xFC);
			combo->SetItemData(combo->AddString("ins+1"), 0xFD);
		}
		for (UINT n=1; n<MAX_INSTRUMENTS; n++)
		{
			if (pSndFile->m_nInstruments)
			{
				wsprintf(s, "%02d:%s", n, (pSndFile->Headers[n]) ? pSndFile->Headers[n]->name : "");
			} else
			{
				wsprintf(s, "%02d:%s", n, pSndFile->m_szNames[n]);
			}
			combo->SetItemData(combo->AddString(s), n);
		}
		UINT ncount = combo->GetCount();
		for (UINT i=0; i<ncount; i++) if (m_nInstr == combo->GetItemData(i))
		{
			combo->SetCurSel(i);
			break;
		}
	}
	// Volume Command
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO3)) != NULL)
	{
		combo->SetItemData(combo->AddString(" None"), (DWORD)-1);
		UINT count = m_pModDoc->GetNumVolCmds();
		for (UINT n=0; n<count; n++)
		{
			m_pModDoc->GetVolCmdInfo(n, s);
			if (s[0]) combo->SetItemData(combo->AddString(s), n);
		}
		combo->SetCurSel(0);
		UINT fxndx = m_pModDoc->GetIndexFromVolCmd(m_nVolCmd);
		for (UINT i=0; i<=count; i++) if (fxndx == combo->GetItemData(i))
		{
			combo->SetCurSel(i);
			break;
		}
	}
	// Volume
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO4)) != NULL)
	{
		for (UINT n=0; n<=64; n++)
		{
			wsprintf(s, "%02d", n);
			combo->SetItemData(combo->AddString(s), n);
		}
		UINT ncount = combo->GetCount();
		for (UINT i=0; i<ncount; i++) if (m_nVol == combo->GetItemData(i))
		{
			combo->SetCurSel(i);
			break;
		}
	}
	// Command
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO5)) != NULL)
	{
		combo->SetItemData(combo->AddString(" None"), (DWORD)-1);
		UINT count = m_pModDoc->GetNumEffects();
		for (UINT n=0; n<count; n++)
		{
			m_pModDoc->GetEffectInfo(n, s, TRUE);
			if (s[0]) combo->SetItemData(combo->AddString(s), n);
		}
		combo->SetCurSel(0);
		UINT fxndx = m_pModDoc->GetIndexFromEffect(m_nCommand, m_nParam);
		for (UINT i=0; i<=count; i++) if (fxndx == combo->GetItemData(i))
		{
			combo->SetCurSel(i);
			break;
		}
	}
	OnEffectChanged();
	OnCheckChannelSearch();
	return TRUE;
}


void CFindReplaceTab::OnEffectChanged()
//-------------------------------------
{
	int fxndx = -1;
	CComboBox *combo;
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO5)) != NULL)
	{
		fxndx = combo->GetItemData(combo->GetCurSel());
	}
	// Update Param range
	if (((combo = (CComboBox *)GetDlgItem(IDC_COMBO6)) != NULL) && (m_pModDoc))
	{
		UINT oldcount = combo->GetCount();
		UINT newcount = m_pModDoc->IsExtendedEffect(fxndx) ? 16 : 256;
		if (oldcount != newcount)
		{
			CHAR s[16];
			int newpos;
			if (oldcount) newpos = combo->GetCurSel() % newcount; else newpos = m_nParam % newcount;
			combo->ResetContent();
			for (UINT i=0; i<newcount; i++)
			{
				wsprintf(s, (newcount == 256) ? "%02X" : "%X", i);
				combo->SetItemData(combo->AddString(s), i);
			}
			combo->SetCurSel(newpos);
		}
	}
}


void CFindReplaceTab::OnCheckChannelSearch()
//------------------------------------------
{
	if (!m_bReplace)
	{
		BOOL b = IsDlgButtonChecked(IDC_CHECK7);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT1), b);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT2), b);
	}
}


void CFindReplaceTab::OnOK()
//--------------------------
{
	CComboBox *combo;

	// Search flags
	m_dwFlags = 0;
	if (IsDlgButtonChecked(IDC_CHECK1)) m_dwFlags |= PATSEARCH_NOTE;
	if (IsDlgButtonChecked(IDC_CHECK2)) m_dwFlags |= PATSEARCH_INSTR;
	if (IsDlgButtonChecked(IDC_CHECK3)) m_dwFlags |= PATSEARCH_VOLCMD;
	if (IsDlgButtonChecked(IDC_CHECK4)) m_dwFlags |= PATSEARCH_VOLUME;
	if (IsDlgButtonChecked(IDC_CHECK5)) m_dwFlags |= PATSEARCH_COMMAND;
	if (IsDlgButtonChecked(IDC_CHECK6)) m_dwFlags |= PATSEARCH_PARAM;
	if (m_bReplace)
	{
		if (IsDlgButtonChecked(IDC_CHECK7)) m_dwFlags |= PATSEARCH_REPLACE;
		if (IsDlgButtonChecked(IDC_CHECK8)) m_dwFlags |= PATSEARCH_REPLACEALL;
	} else
	{
		if (IsDlgButtonChecked(IDC_CHECK7)) m_dwFlags |= PATSEARCH_CHANNEL;
		if (IsDlgButtonChecked(IDC_RADIO2)) m_dwFlags |= PATSEARCH_FULLSEARCH;
	}
	// Note
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL)
	{
		m_nNote = combo->GetItemData(combo->GetCurSel());
	}
	// Instrument
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO2)) != NULL)
	{
		m_nInstr = combo->GetItemData(combo->GetCurSel());
	}
	// Volume Command
	if (((combo = (CComboBox *)GetDlgItem(IDC_COMBO3)) != NULL) && (m_pModDoc))
	{
		m_nVolCmd = m_pModDoc->GetVolCmdFromIndex(combo->GetItemData(combo->GetCurSel()));
	}
	// Volume
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO4)) != NULL)
	{
		m_nVol = combo->GetItemData(combo->GetCurSel());
	}
	// Effect
	m_nParam = 0;
	if (((combo = (CComboBox *)GetDlgItem(IDC_COMBO5)) != NULL) && (m_pModDoc))
	{
		int n = -1;
		m_nCommand = m_pModDoc->GetEffectFromIndex(combo->GetItemData(combo->GetCurSel()), &n);
		if (n >= 0) m_nParam = n;
	}
	// Param
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO6)) != NULL)
	{
		m_nParam |= combo->GetItemData(combo->GetCurSel());
	}
	// Min/Max channels
	if (!m_bReplace)
	{
		m_nMinChannel = GetDlgItemInt(IDC_EDIT1) - 1;
		m_nMaxChannel = GetDlgItemInt(IDC_EDIT2) - 1;
		if (m_nMaxChannel < m_nMinChannel) m_nMaxChannel = m_nMinChannel;
	}
	CPropertyPage::OnOK();
}


/////////////////////////////////////////////////////////////////////////////////////////////
// CPatternPropertiesDlg

BOOL CPatternPropertiesDlg::OnInitDialog()
//----------------------------------------
{
	CComboBox *combo;
	CDialog::OnInitDialog();
	combo = (CComboBox *)GetDlgItem(IDC_COMBO1);
	if ((m_pModDoc) && (m_nPattern < MAX_PATTERNS) && (combo))
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		CHAR s[256];
		UINT nrows = pSndFile->PatternSize[m_nPattern];

		for (UINT irow=32; irow<=256; irow++)
		{
			wsprintf(s, "%d", irow);
			combo->AddString(s);
		}
		if (nrows < 32)
		{
			wsprintf(s, "%d", nrows);
			combo->SetWindowText(s);
		} else combo->SetCurSel(nrows-32);
		wsprintf(s, "Pattern #%d:\x0d\x0a %d rows (%dK)",
			m_nPattern,
			pSndFile->PatternSize[m_nPattern],
			(pSndFile->PatternSize[m_nPattern] * pSndFile->m_nChannels * 6)/1024);
		SetDlgItemText(IDC_TEXT1, s);
	}
	return TRUE;
}


void CPatternPropertiesDlg::OnOK()
//--------------------------------
{
	int n = GetDlgItemInt(IDC_COMBO1);
	if ((n >= 2) && (n <= 256) && (m_pModDoc)) m_pModDoc->ResizePattern(m_nPattern, n);
	CDialog::OnOK();
}


////////////////////////////////////////////////////////////////////////////////////////////
// CEditCommand

BEGIN_MESSAGE_MAP(CEditCommand, CPropertySheet)
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


CEditCommand::CEditCommand()
//--------------------------
{
	m_pModDoc = NULL;
	m_hWndView = NULL;
	m_nPattern = 0;
	m_nRow = 0;
	m_nChannel = 0;
	m_pageNote = NULL;
	m_pageVolume = NULL;
	m_pageEffect = NULL;
}


BOOL CEditCommand::SetParent(CWnd *parent, CModDoc *pModDoc)
//----------------------------------------------------------
{
	if ((!parent) || (!pModDoc)) return FALSE;
	m_hWndView = parent->m_hWnd;
	m_pModDoc = pModDoc;
	m_pageNote = new CPageEditNote(m_pModDoc, this);
	m_pageVolume = new CPageEditVolume(m_pModDoc, this);
	m_pageEffect = new CPageEditEffect(m_pModDoc, this);
	AddPage(m_pageNote);
	AddPage(m_pageVolume);
	AddPage(m_pageEffect);
	if (!CPropertySheet::Create(parent,
		WS_SYSMENU|WS_POPUP|WS_CAPTION,	WS_EX_DLGMODALFRAME)) return FALSE;
	ModifyStyleEx(0, WS_EX_TOOLWINDOW|WS_EX_PALETTEWINDOW, SWP_FRAMECHANGED);
	return TRUE;
}


BOOL CEditCommand::PreTranslateMessage(MSG *pMsg)
//-----------------------------------------------
{
	if ((pMsg) && (pMsg->message == WM_KEYDOWN))
	{
		if ((pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_APPS))
		{
			OnClose();
			return TRUE;
		}
	}
	return CPropertySheet::PreTranslateMessage(pMsg);
}


BOOL CEditCommand::ShowEditWindow(UINT nPat, DWORD dwCursor)
//----------------------------------------------------------
{
	CHAR s[64];
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	UINT nRow = dwCursor >> 16;
	UINT nChannel = (dwCursor & 0xFFFF) >> 3;

	if ((nPat >= MAX_PATTERNS) || (!m_pModDoc)
	 || (nRow >= pSndFile->PatternSize[nPat]) || (nChannel >= pSndFile->m_nChannels)
	 || (!pSndFile->Patterns[nPat])) return FALSE;
	m_Command = pSndFile->Patterns[nPat][nRow * pSndFile->m_nChannels + nChannel];
	m_nRow = nRow;
	m_nChannel = nChannel;
	m_nPattern = nPat;
	// Init Pages
	if (m_pageNote) m_pageNote->Init(m_Command);
	if (m_pageVolume) m_pageVolume->Init(m_Command);
	if (m_pageEffect) m_pageEffect->Init(m_Command);
	// Update Window Title
	wsprintf(s, "Note Properties - Row %d, Channel %d", m_nRow, m_nChannel+1);
	SetTitle(s);
	// Activate Page
	UINT nPage = 2;
	dwCursor &= 7;
	if (dwCursor < 2) nPage = 0;
	else if (dwCursor < 3) nPage = 1;
	SetActivePage(nPage);
	if (m_pageNote) m_pageNote->UpdateDialog();
	if (m_pageVolume) m_pageVolume->UpdateDialog();
	if (m_pageEffect) m_pageEffect->UpdateDialog();
	ShowWindow(SW_SHOW);
	return TRUE;
}


void CEditCommand::UpdateNote(UINT note, UINT instr)
//--------------------------------------------------
{
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	if ((m_nPattern >= MAX_PATTERNS) || (!m_pModDoc)
	 || (m_nRow >= pSndFile->PatternSize[m_nPattern])
	 || (m_nChannel >= pSndFile->m_nChannels)
	 || (!pSndFile->Patterns[m_nPattern])) return;
	MODCOMMAND *m = pSndFile->Patterns[m_nPattern]+m_nRow*pSndFile->m_nChannels+m_nChannel;
	if ((m->note != note) || (m->instr != instr))
	{
		m->note = note;
		m->instr = instr;
		m_Command = *m;
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, (m_nRow << 24) | HINT_PATTERNROW, NULL);
	}
}


void CEditCommand::UpdateVolume(UINT volcmd, UINT vol)
//----------------------------------------------------
{
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	if ((m_nPattern >= MAX_PATTERNS) || (!m_pModDoc)
	 || (m_nRow >= pSndFile->PatternSize[m_nPattern])
	 || (m_nChannel >= pSndFile->m_nChannels)
	 || (!pSndFile->Patterns[m_nPattern])) return;
	MODCOMMAND *m = pSndFile->Patterns[m_nPattern]+m_nRow*pSndFile->m_nChannels+m_nChannel;
	if ((m->volcmd != volcmd) || (m->vol != vol))
	{
		m->volcmd = volcmd;
		m->vol = vol;
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, (m_nRow << 24) | HINT_PATTERNROW, NULL);
	}
}


void CEditCommand::UpdateEffect(UINT command, UINT param)
//-------------------------------------------------------
{
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	if ((m_nPattern >= MAX_PATTERNS) || (!m_pModDoc)
	 || (m_nRow >= pSndFile->PatternSize[m_nPattern])
	 || (m_nChannel >= pSndFile->m_nChannels)
	 || (!pSndFile->Patterns[m_nPattern])) return;
	MODCOMMAND *m = pSndFile->Patterns[m_nPattern]+m_nRow*pSndFile->m_nChannels+m_nChannel;
	if ((m->command != command) || (m->param != param))
	{
		m->command = command;
		m->param = param;
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, (m_nRow << 24) | HINT_PATTERNROW, NULL);
	}
}


void CEditCommand::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
//--------------------------------------------------------------------------
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);
	if (nState == WA_INACTIVE) ShowWindow(SW_HIDE);
}


//////////////////////////////////////////////////////////////////////////////////////
// CPageEditCommand

BOOL CPageEditCommand::OnInitDialog()
//-----------------------------------
{
	CPropertyPage::OnInitDialog();
	m_bInitialized = TRUE;
	UpdateDialog();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////
// CPageEditNote

BEGIN_MESSAGE_MAP(CPageEditNote, CPageEditCommand)
	ON_CBN_SELCHANGE(IDC_COMBO1,	OnNoteChanged)
	ON_CBN_SELCHANGE(IDC_COMBO2,	OnInstrChanged)
END_MESSAGE_MAP()


void CPageEditNote::UpdateDialog()
//--------------------------------
{
	char s[64];
	CComboBox *combo;
	CSoundFile *pSndFile;

	if ((!m_bInitialized) || (!m_pModDoc)) return;
	pSndFile = m_pModDoc->GetSoundFile();
	// Note
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL)
	{	
		combo->ResetContent();
		combo->SetItemData(combo->AddString("No note"), 0);
		for (UINT i=1; i<=120; i++)
		{
			wsprintf(s, "%s%d", szNoteNames[(i-1)%12], (i-1)/12);
			combo->SetItemData(combo->AddString(s), i);
		}
		if (pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
		{
			int k = combo->AddString("Note Cut");
			combo->SetItemData(k, 0xFE);
			if (m_nNote == 0xFE) combo->SetCurSel(k);
		}
		if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT))
		{
			int k = combo->AddString("Note Off");
			combo->SetItemData(k, 0xFF);
			if (m_nNote == 0xFF) combo->SetCurSel(k);
		}
		if (m_nNote <= 120) combo->SetCurSel(m_nNote);
	}
	// Instrument
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO2)) != NULL)
	{
		combo->ResetContent();
		combo->SetItemData(combo->AddString("No Instrument"), 0);
		UINT max = pSndFile->m_nInstruments;
		if (!max) max = pSndFile->m_nSamples;
		for (UINT i=1; i<=max; i++)
		{
			wsprintf(s, "%02d:", i);
			int k = strlen(s);
			if (pSndFile->m_nInstruments)
			{
				if (pSndFile->Headers[i])
					memcpy(s+k, pSndFile->Headers[i]->name, 32);
			} else
				memcpy(s+k, pSndFile->m_szNames[i], 32);
			s[k+32] = 0;
			combo->SetItemData(combo->AddString(s), i);
		}
		combo->SetCurSel(m_nInstr);
	}
}


void CPageEditNote::OnNoteChanged()
//---------------------------------
{
	CComboBox *combo;
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL)
	{
		int n = combo->GetCurSel();
		if (n >= 0) m_nNote = combo->GetItemData(n);
	}
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO2)) != NULL)
	{
		int n = combo->GetCurSel();
		if (n >= 0) m_nInstr = combo->GetItemData(n);
	}
	if (m_pParent) m_pParent->UpdateNote(m_nNote, m_nInstr);
}


void CPageEditNote::OnInstrChanged()
//----------------------------------
{
	OnNoteChanged();
}


//////////////////////////////////////////////////////////////////////////////////////
// CPageEditVolume

BEGIN_MESSAGE_MAP(CPageEditVolume, CPageEditCommand)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_COMBO1,	OnVolCmdChanged)
END_MESSAGE_MAP()


void CPageEditVolume::UpdateDialog()
//----------------------------------
{
	CComboBox *combo;

	if ((!m_bInitialized) || (!m_pModDoc)) return;
	UpdateRanges();
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		if (pSndFile->m_nType == MOD_TYPE_MOD)
		{
			combo->EnableWindow(FALSE);
			return;
		}
		combo->EnableWindow(TRUE);
		combo->ResetContent();
		UINT count = m_pModDoc->GetNumVolCmds();
		combo->SetItemData(combo->AddString(" None"), (DWORD)-1);
		combo->SetCurSel(0);
		UINT fxndx = m_pModDoc->GetIndexFromVolCmd(m_nVolCmd);
		for (UINT i=0; i<count; i++)
		{
			CHAR s[64];
			if (m_pModDoc->GetVolCmdInfo(i, s))
			{
				int k = combo->AddString(s);
				combo->SetItemData(k, i);
				if (i == fxndx) combo->SetCurSel(k);
			}
		}
	}
}


void CPageEditVolume::UpdateRanges()
//----------------------------------
{
	CSliderCtrl *slider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER1);
	if ((slider) && (m_pModDoc))
	{
		DWORD rangeMin = 0, rangeMax = 0;
		LONG fxndx = m_pModDoc->GetIndexFromVolCmd(m_nVolCmd);
		BOOL bOk = m_pModDoc->GetVolCmdInfo(fxndx, NULL, &rangeMin, &rangeMax);
		if ((bOk) && (rangeMax > rangeMin))
		{
			slider->EnableWindow(TRUE);
			slider->SetRange(rangeMin, rangeMax);
			UINT pos = m_nVolume;
			if (pos < rangeMin) pos = rangeMin;
			if (pos > rangeMax) pos = rangeMax;
			slider->SetPos(pos);
		} else
		{
			slider->EnableWindow(FALSE);
		}
	}
}


void CPageEditVolume::OnVolCmdChanged()
//-------------------------------------
{
	CComboBox *combo;
	CSliderCtrl *slider;
	if (((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL) && (m_pModDoc))
	{
		int n = combo->GetCurSel();
		if (n >= 0)
		{
			UINT volcmd = m_pModDoc->GetVolCmdFromIndex(combo->GetItemData(n));
			if (volcmd != m_nVolCmd)
			{
				m_nVolCmd = volcmd;
				UpdateRanges();
			}
		}
	}
	if ((slider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER1)) != NULL)
	{
		m_nVolume = slider->GetPos();
	}
	if (m_pParent) m_pParent->UpdateVolume(m_nVolCmd, m_nVolume);
}


void CPageEditVolume::OnHScroll(UINT, UINT, CScrollBar *)
//-------------------------------------------------------
{
	OnVolCmdChanged();
}


//////////////////////////////////////////////////////////////////////////////////////
// CPageEditEffect

BEGIN_MESSAGE_MAP(CPageEditEffect, CPageEditCommand)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_COMBO1,	OnCommandChanged)
END_MESSAGE_MAP()


void CPageEditEffect::UpdateDialog()
//----------------------------------
{
	CHAR s[128];
	CComboBox *combo;
	CSoundFile *pSndFile;
	
	if ((!m_pModDoc) || (!m_bInitialized)) return;
	pSndFile = m_pModDoc->GetSoundFile();
	if ((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL)
	{
		UINT numfx = m_pModDoc->GetNumEffects();
		UINT fxndx = m_pModDoc->GetIndexFromEffect(m_nCommand, m_nParam);
		combo->ResetContent();
		combo->SetItemData(combo->AddString(" None"), (DWORD)-1);
		if (!m_nCommand) combo->SetCurSel(0);
		for (UINT i=0; i<numfx; i++)
		{
			if (m_pModDoc->GetEffectInfo(i, s, TRUE))
			{
				int k = combo->AddString(s);
				combo->SetItemData(k, i);
				if (i == fxndx) combo->SetCurSel(k);
			}
		}
	}
	UpdateRange(FALSE);
}


void CPageEditEffect::UpdateRange(BOOL bSet)
//------------------------------------------
{
	CSliderCtrl *slider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER1);
	if ((slider) && (m_pModDoc))
	{
		DWORD rangeMin = 0, rangeMax = 0;
		LONG fxndx = m_pModDoc->GetIndexFromEffect(m_nCommand, m_nParam);
		BOOL bEnable = ((fxndx >= 0) && (m_pModDoc->GetEffectInfo(fxndx, NULL, FALSE, &rangeMin, &rangeMax)));
		if (bEnable)
		{
			slider->EnableWindow(TRUE);
			slider->SetPageSize(1);
			slider->SetRange(rangeMin, rangeMax);
			DWORD pos = m_pModDoc->MapValueToPos(fxndx, m_nParam);
			if (pos > rangeMax) pos = rangeMin | (pos & 0x0F);
			if (pos < rangeMin) pos = rangeMin;
			if (pos > rangeMax) pos = rangeMax;
			slider->SetPos(pos);
		} else
		{
			slider->SetRange(0,0);
			slider->EnableWindow(FALSE);
		}
		UpdateValue(bSet);
	}
}


void CPageEditEffect::UpdateValue(BOOL bSet)
//------------------------------------------
{
	if (m_pModDoc)
	{
		CHAR s[128] = "";
		LONG fxndx = m_pModDoc->GetIndexFromEffect(m_nCommand, m_nParam);
		if (fxndx >= 0) m_pModDoc->GetEffectNameEx(s, fxndx, m_nParam);
		SetDlgItemText(IDC_TEXT1, s);
	}
	if ((m_pParent) && (bSet)) m_pParent->UpdateEffect(m_nCommand, m_nParam);
}


void CPageEditEffect::OnCommandChanged()
//--------------------------------------
{
	CComboBox *combo;

	if (((combo = (CComboBox *)GetDlgItem(IDC_COMBO1)) != NULL) && (m_pModDoc))
	{
		BOOL bSet = FALSE;
		int n = combo->GetCurSel();
		if (n >= 0)
		{
			int param = -1, ndx = combo->GetItemData(n);
			m_nCommand = (ndx >= 0) ? m_pModDoc->GetEffectFromIndex(ndx, &param) : 0;
			if (param >= 0) m_nParam = param;
			bSet = TRUE;
		}
		UpdateRange(bSet);
	}
}


void CPageEditEffect::OnHScroll(UINT, UINT, CScrollBar *)
//-------------------------------------------------------
{
	CSliderCtrl *slider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER1);
	if ((slider) && (m_pModDoc))
	{
		LONG fxndx = m_pModDoc->GetIndexFromEffect(m_nCommand, m_nParam);
		if (fxndx >= 0)
		{
			int pos = slider->GetPos();
			UINT param = m_pModDoc->MapPosToValue(fxndx, pos);
			if (param != m_nParam)
			{
				m_nParam = param;
				UpdateValue(TRUE);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Samples

BOOL CAmpDlg::OnInitDialog()
//--------------------------
{
	CDialog::OnInitDialog();
	CSpinButtonCtrl *spin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1);
	if (spin)
	{
		spin->SetRange(10, 800);
		spin->SetPos(m_nFactor);
	}
	SetDlgItemInt(IDC_EDIT1, m_nFactor);
	return TRUE;
}


void CAmpDlg::OnOK()
//------------------
{
	m_nFactor = GetDlgItemInt(IDC_EDIT1);
	m_bFadeIn = IsDlgButtonChecked(IDC_CHECK1);
	m_bFadeOut = IsDlgButtonChecked(IDC_CHECK2);
	CDialog::OnOK();
}


////////////////////////////////////////////////////////////////////////////////
// Sound Bank Information

CSoundBankProperties::CSoundBankProperties(CDLSBank *pBank, CWnd *parent):CDialog(IDD_SOUNDBANK_INFO, parent)
//-----------------------------------------------------------------------------------------------------------
{
	SOUNDBANKINFO bi;
	
	m_szInfo[0] = 0;
	if (pBank)
	{
		UINT nType = pBank->GetBankInfo(&bi);
		wsprintf(m_szInfo, "File:\t\"%s\"\r\n", pBank->GetFileName());
		wsprintf(&m_szInfo[strlen(m_szInfo)], "Type:\t%s\r\n", (nType & SOUNDBANK_TYPE_SF2) ? "Sound Font (SF2)" : "Downloadable Sound (DLS)");
		if (bi.szBankName[0])
			wsprintf(&m_szInfo[strlen(m_szInfo)], "Name:\t\"%s\"\r\n", bi.szBankName);
		if (bi.szDescription[0])
			wsprintf(&m_szInfo[strlen(m_szInfo)], "\t\"%s\"\r\n", bi.szDescription);
		if (bi.szCopyRight[0])
			wsprintf(&m_szInfo[strlen(m_szInfo)], "Copyright:\t\"%s\"\r\n", bi.szCopyRight);
		if (bi.szEngineer[0])
			wsprintf(&m_szInfo[strlen(m_szInfo)], "Author:\t\"%s\"\r\n", bi.szEngineer);
		if (bi.szSoftware[0])
			wsprintf(&m_szInfo[strlen(m_szInfo)], "Software:\t\"%s\"\r\n", bi.szSoftware);
		// Last lines: comments
		if (bi.szComments[0])
		{
			strncat(m_szInfo, "\r\nComments:\r\n", sizeof(m_szInfo)-1);
			strncat(m_szInfo, bi.szComments, sizeof(m_szInfo)-1);
		}
	}
}


BOOL CSoundBankProperties::OnInitDialog()
//---------------------------------------
{
	CDialog::OnInitDialog();
	SetDlgItemText(IDC_EDIT1, m_szInfo);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// Midi Macros (Zxx)

BEGIN_MESSAGE_MAP(CMidiMacroSetup, CDialog)
	ON_COMMAND(IDC_CHECK1,			OnEmbedMidiCfg)
	ON_COMMAND(IDC_BUTTON1,			OnSetAsDefault)
	ON_COMMAND(IDC_BUTTON2,			OnResetCfg)
	ON_CBN_SELCHANGE(IDC_COMBO1,	OnSFxChanged)
	ON_CBN_SELCHANGE(IDC_COMBO2,	OnSFxPresetChanged)
	ON_CBN_SELCHANGE(IDC_COMBO3,	OnZxxPresetChanged)
	ON_CBN_SELCHANGE(IDC_COMBO4,	UpdateDialog)
	ON_EN_CHANGE(IDC_EDIT1,			OnSFxEditChanged)
	ON_EN_CHANGE(IDC_EDIT2,			OnZxxEditChanged)
END_MESSAGE_MAP()


CMidiMacroSetup::CMidiMacroSetup(MODMIDICFG *pcfg, BOOL bEmbed, CWnd *parent):CDialog(IDD_MIDIMACRO, parent)
//----------------------------------------------------------------------------------------------------------
{
	m_bEmbed = bEmbed;
	if (pcfg) m_MidiCfg = *pcfg;
}


void CMidiMacroSetup::DoDataExchange(CDataExchange* pDX)
//------------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModTypeDlg)
	DDX_Control(pDX, IDC_COMBO1,	m_CbnSFx);
	DDX_Control(pDX, IDC_COMBO2,	m_CbnSFxPreset);
	DDX_Control(pDX, IDC_COMBO3,	m_CbnZxxPreset);
	DDX_Control(pDX, IDC_COMBO4,	m_CbnZxx);
	DDX_Control(pDX, IDC_EDIT1,		m_EditSFx);
	DDX_Control(pDX, IDC_EDIT2,		m_EditZxx);
	//}}AFX_DATA_MAP
}


BOOL CMidiMacroSetup::OnInitDialog()
//----------------------------------
{
	CHAR s[128];
	CDialog::OnInitDialog();
	CheckDlgButton(IDC_CHECK1, m_bEmbed);
	m_EditSFx.SetLimitText(31);
	m_EditZxx.SetLimitText(31);
	for (UINT isfx=0; isfx<16; isfx++)
	{
		wsprintf(s, "Macro %d", isfx);
		m_CbnSFx.AddString(s);
	}
	m_CbnSFx.SetCurSel(0);
	m_CbnSFxPreset.AddString("Custom");
	m_CbnSFxPreset.AddString("Unused");
	m_CbnSFxPreset.AddString("Set Filter Cutoff");
	m_CbnSFxPreset.AddString("Set Filter Resonance");
	m_CbnSFxPreset.AddString("Set Filter Mode");
	m_CbnSFxPreset.AddString("Set Effect Param #0");
	m_CbnSFxPreset.AddString("Set Effect Param #1");
	m_CbnSFxPreset.AddString("Set Effect Param #2");
	m_CbnSFxPreset.AddString("Set Effect Param #3");
	OnSFxChanged();
	for (UINT zxx=0; zxx<128; zxx++)
	{
		wsprintf(s, "Z%02X", zxx|0x80);
		m_CbnZxx.AddString(s);
	}
	m_CbnZxx.SetCurSel(0);
	m_CbnZxxPreset.AddString("Custom");
	m_CbnZxxPreset.AddString("Z80-Z8F controls resonance");
	m_CbnZxxPreset.AddString("Z80-ZFF controls resonance");
	m_CbnZxxPreset.AddString("Z80-ZFF controls cutoff");
	m_CbnZxxPreset.AddString("Z80-ZFF controls filter mode");
	m_CbnZxxPreset.AddString("Z80-Z9F controls resonance+mode");
	m_CbnZxxPreset.SetCurSel(0);
	UpdateDialog();
	return FALSE;
}


void CMidiMacroSetup::UpdateDialog()
//----------------------------------
{
	CHAR s[32];
	UINT sfx, sfx_preset, zxx;

	sfx = m_CbnSFx.GetCurSel();
	sfx_preset = m_CbnSFxPreset.GetCurSel();
	if (sfx < 16)
	{
		memcpy(s, &m_MidiCfg.szMidiSFXExt[sfx*32], 32);
		s[31] = 0;
		if (sfx_preset)
		{
			m_EditSFx.SetWindowText(s);
			m_EditSFx.EnableWindow(FALSE);
		} else
		{
			m_EditSFx.EnableWindow(TRUE);
			m_EditSFx.SetWindowText(s);
		}
	}
	zxx = m_CbnZxx.GetCurSel();
	if (zxx < 0x80)
	{
		memcpy(s, &m_MidiCfg.szMidiZXXExt[zxx*32], 32);
		s[31] = 0;
		m_EditZxx.SetWindowText(s);
	}
}


void CMidiMacroSetup::OnSetAsDefault()
//------------------------------------
{
	theApp.SetDefaultMidiMacro(&m_MidiCfg);
}


void CMidiMacroSetup::OnResetCfg()
//--------------------------------
{
	theApp.GetDefaultMidiMacro(&m_MidiCfg);
	m_CbnZxxPreset.SetCurSel(0);
	OnSFxChanged();
}


void CMidiMacroSetup::OnEmbedMidiCfg()
//------------------------------------
{
	m_bEmbed = IsDlgButtonChecked(IDC_CHECK1);
}


void CMidiMacroSetup::OnSFxChanged()
//----------------------------------
{
	UINT sfx = m_CbnSFx.GetCurSel();
	if (sfx < 16)
	{
		if (!m_MidiCfg.szMidiSFXExt[sfx*32]) m_CbnSFxPreset.SetCurSel(1); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F000z")) m_CbnSFxPreset.SetCurSel(2); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F001z")) m_CbnSFxPreset.SetCurSel(3); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F002z")) m_CbnSFxPreset.SetCurSel(4); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F080z")) m_CbnSFxPreset.SetCurSel(5); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F081z")) m_CbnSFxPreset.SetCurSel(6); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F082z")) m_CbnSFxPreset.SetCurSel(7); else
		if (!lstrcmpi(&m_MidiCfg.szMidiSFXExt[sfx*32], "F0F083z")) m_CbnSFxPreset.SetCurSel(8); else
		m_CbnSFxPreset.SetCurSel(0);
	}
	UpdateDialog();
}


void CMidiMacroSetup::OnSFxPresetChanged()
//----------------------------------------
{
	UINT sfx = m_CbnSFx.GetCurSel();
	UINT sfx_preset = m_CbnSFxPreset.GetCurSel();

	if (sfx < 16)
	{
		CHAR *pmacro = &m_MidiCfg.szMidiSFXExt[sfx*32];
		switch(sfx_preset)
		{
		case 0: strcpy(pmacro, "z"); break;
		case 1:	pmacro[0] = 0; break; // unused
		case 2: strcpy(pmacro, "F0F000z"); break;
		case 3: strcpy(pmacro, "F0F001z"); break;
		case 4: strcpy(pmacro, "F0F002z"); break;
		case 5: strcpy(pmacro, "F0F080z"); break;
		case 6: strcpy(pmacro, "F0F081z"); break;
		case 7: strcpy(pmacro, "F0F082z"); break;
		case 8: strcpy(pmacro, "F0F083z"); break;
		}
		UpdateDialog();
	}
}


void CMidiMacroSetup::OnZxxPresetChanged()
//----------------------------------------
{
	UINT zxx_preset = m_CbnZxxPreset.GetCurSel();

	if (zxx_preset)
	{
		BeginWaitCursor();
		for (UINT i=0; i<128; i++)
		{
			switch(zxx_preset)
			{
			case 1:
				if (i<16) wsprintf(&m_MidiCfg.szMidiZXXExt[i*32], "F0F001%02X", i*8);
				else m_MidiCfg.szMidiZXXExt[i*32] = 0;
				break;

			case 2:
				wsprintf(&m_MidiCfg.szMidiZXXExt[i*32], "F0F001%02X", i);
				break;

			case 3:
				wsprintf(&m_MidiCfg.szMidiZXXExt[i*32], "F0F000%02X", i);
				break;

			case 4:
				wsprintf(&m_MidiCfg.szMidiZXXExt[i*32], "F0F002%02X", i);
				break;

			case 5:
				if (i<16) wsprintf(&m_MidiCfg.szMidiZXXExt[i*32], "F0F001%02X", i*8);
				else if (i<32) wsprintf(&m_MidiCfg.szMidiZXXExt[i*32], "F0F002%02X", (i-16)*8);
				else m_MidiCfg.szMidiZXXExt[i*32] = 0;
				break;
			}
		}
		UpdateDialog();
		EndWaitCursor();
	}
}


void CMidiMacroSetup::OnSFxEditChanged()
//--------------------------------------
{
	CHAR s[32];
	UINT sfx = m_CbnSFx.GetCurSel();
	if (sfx < 16)
	{
		memset(s, 0, sizeof(s));
		m_EditSFx.GetWindowText(s, 31);
		s[31] = 0;
		memcpy(&m_MidiCfg.szMidiSFXExt[sfx*32], s, 32);
	}
}


void CMidiMacroSetup::OnZxxEditChanged()
//--------------------------------------
{
	CHAR s[32];
	UINT zxx = m_CbnZxx.GetCurSel();
	if (zxx < 128)
	{
		memset(s, 0, sizeof(s));
		m_EditZxx.GetWindowText(s, 31);
		s[31] = 0;
		memcpy(&m_MidiCfg.szMidiZXXExt[zxx*32], s, 32);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard Control

const BYTE whitetab[7] = {0,2,4,5,7,9,11};
const BYTE blacktab[7] = {0xff,1,3,0xff,6,8,10};

BEGIN_MESSAGE_MAP(CKeyboardControl, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


void CKeyboardControl::OnPaint()
//------------------------------
{
	HGDIOBJ oldpen, oldbrush;
	CRect rcClient, rect;
	CPaintDC dc(this);
	HDC hdc = dc.m_hDC;
	HBRUSH brushRed;

	if (!m_nOctaves) m_nOctaves = 1;
	GetClientRect(&rcClient);
	rect = rcClient;
	oldpen = ::SelectObject(hdc, CMainFrame::penBlack);
	oldbrush = ::SelectObject(hdc, CMainFrame::brushWhite);
	brushRed = ::CreateSolidBrush(RGB(0xFF, 0, 0));
	// White notes
	for (UINT note=0; note<m_nOctaves*7; note++)
	{
		rect.right = ((note + 1) * rcClient.Width()) / (m_nOctaves * 7);
		int val = (note/7) * 12 + whitetab[note % 7];
		if (val == m_nSelection) ::SelectObject(hdc, CMainFrame::brushGray);
		dc.Rectangle(&rect);
		if (val == m_nSelection) ::SelectObject(hdc, CMainFrame::brushWhite);
		if ((val < 120) && (KeyFlags[val]))
		{
			::SelectObject(hdc, brushRed);
			dc.Ellipse(rect.left+2, rect.bottom - (rect.right-rect.left) + 2, rect.right-2, rect.bottom-2);
			::SelectObject(hdc, CMainFrame::brushWhite);
		}
		rect.left = rect.right - 1;
	}
	// Black notes
	::SelectObject(hdc, CMainFrame::brushBlack);
	rect = rcClient;
	rect.bottom -= rcClient.Height() / 3;
	for (UINT nblack=0; nblack<m_nOctaves*7; nblack++)
	{
		switch(nblack % 7)
		{
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
			{
				rect.left = (nblack * rcClient.Width()) / (m_nOctaves * 7);
				rect.right = rect.left;
				int delta = rcClient.Width() / (m_nOctaves * 7 * 3);
				rect.left -= delta;
				rect.right += delta;
				int val = (nblack/7)*12 + blacktab[nblack%7];
				if (val == m_nSelection) ::SelectObject(hdc, CMainFrame::brushGray);
				dc.Rectangle(&rect);
				if (val == m_nSelection) ::SelectObject(hdc, CMainFrame::brushBlack);
				if ((val < 120) && (KeyFlags[val]))
				{
					::SelectObject(hdc, brushRed);
					dc.Ellipse(rect.left, rect.bottom - (rect.right-rect.left), rect.right, rect.bottom);
					::SelectObject(hdc, CMainFrame::brushBlack);
				}
			}
			break;
		}
	}
	if (oldpen) ::SelectObject(hdc, oldpen);
	if (oldbrush) ::SelectObject(hdc, oldbrush);
}


void CKeyboardControl::OnMouseMove(UINT, CPoint point)
//----------------------------------------------------
{
	int sel = -1, xmin, xmax;
	CRect rcClient, rect;
	if (!m_nOctaves) m_nOctaves = 1;
	GetClientRect(&rcClient);
	rect = rcClient;
	xmin = rcClient.right;
	xmax = rcClient.left;
	// White notes
	for (UINT note=0; note<m_nOctaves*7; note++)
	{
		int val = (note/7)*12 + whitetab[note % 7];
		rect.right = ((note + 1) * rcClient.Width()) / (m_nOctaves * 7);
		if (val == m_nSelection)
		{
			if (rect.left < xmin) xmin = rect.left;
			if (rect.right > xmax) xmax = rect.right;
		}
		if (rect.PtInRect(point))
		{
			sel = val;
			if (rect.left < xmin) xmin = rect.left;
			if (rect.right > xmax) xmax = rect.right;
		}
		rect.left = rect.right - 1;
	}
	// Black notes
	rect = rcClient;
	rect.bottom -= rcClient.Height() / 3;
	for (UINT nblack=0; nblack<m_nOctaves*7; nblack++)
	{
		switch(nblack % 7)
		{
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
			{
				int val = (nblack/7)*12 + blacktab[nblack % 7];
				rect.left = (nblack * rcClient.Width()) / (m_nOctaves * 7);
				rect.right = rect.left;
				int delta = rcClient.Width() / (m_nOctaves * 7 * 3);
				rect.left -= delta;
				rect.right += delta;
				if (val == m_nSelection)
				{
					if (rect.left < xmin) xmin = rect.left;
					if (rect.right > xmax) xmax = rect.right;
				}
				if (rect.PtInRect(point))
				{
					sel = val;
					if (rect.left < xmin) xmin = rect.left;
					if (rect.right > xmax) xmax = rect.right;
				}
			}
			break;
		}
	}
	// Check for selection change
	if (sel != m_nSelection)
	{
		m_nSelection = sel;
		rcClient.left = xmin;
		rcClient.right = xmax;
		InvalidateRect(&rcClient, FALSE);
		if ((m_bCursorNotify) && (m_hParent))
		{
			::PostMessage(m_hParent, WM_MOD_KBDNOTIFY, KBDNOTIFY_MOUSEMOVE, m_nSelection);
		}
	}
	if (sel >= 0)
	{
		if (!m_bCapture)
		{
			m_bCapture = TRUE;
			SetCapture();
		}
	} else
	{
		if (m_bCapture)
		{
			m_bCapture = FALSE;
			ReleaseCapture();
		}
	}
}


void CKeyboardControl::OnLButtonDown(UINT, CPoint)
//------------------------------------------------
{
	if ((m_nSelection != -1) && (m_hParent))
	{
		::SendMessage(m_hParent, WM_MOD_KBDNOTIFY, KBDNOTIFY_LBUTTONDOWN, m_nSelection);
	}
}


void CKeyboardControl::OnLButtonUp(UINT, CPoint)
//----------------------------------------------
{
	if ((m_nSelection != -1) && (m_hParent))
	{
		::SendMessage(m_hParent, WM_MOD_KBDNOTIFY, KBDNOTIFY_LBUTTONUP, m_nSelection);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
// Chord Editor

BEGIN_MESSAGE_MAP(CChordEditor, CDialog)
	ON_MESSAGE(WM_MOD_KBDNOTIFY,	OnKeyboardNotify)
	ON_CBN_SELCHANGE(IDC_COMBO1,	OnChordChanged)
	ON_CBN_SELCHANGE(IDC_COMBO2,	OnBaseNoteChanged)
	ON_CBN_SELCHANGE(IDC_COMBO3,	OnNote1Changed)
	ON_CBN_SELCHANGE(IDC_COMBO4,	OnNote2Changed)
	ON_CBN_SELCHANGE(IDC_COMBO5,	OnNote3Changed)
END_MESSAGE_MAP()


void CChordEditor::DoDataExchange(CDataExchange* pDX)
//---------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChordEditor)
	DDX_Control(pDX, IDC_KEYBOARD1,		m_Keyboard);
	DDX_Control(pDX, IDC_COMBO1,		m_CbnShortcut);
	DDX_Control(pDX, IDC_COMBO2,		m_CbnBaseNote);
	DDX_Control(pDX, IDC_COMBO3,		m_CbnNote1);
	DDX_Control(pDX, IDC_COMBO4,		m_CbnNote2);
	DDX_Control(pDX, IDC_COMBO5,		m_CbnNote3);
	//}}AFX_DATA_MAP
}


BOOL CChordEditor::OnInitDialog()
//-------------------------------
{
	CMainFrame *pMainFrm;
	CHAR s[128], stmp[32];
	const DWORD *kbdmap;

	CDialog::OnInitDialog();
	m_Keyboard.Init(m_hWnd, 2);
	pMainFrm = CMainFrame::GetMainFrame();
	if (!pMainFrm) return TRUE;
	kbdmap = CMainFrame::GetKeyboardMap();
	// Fills the shortcut key combo box
	for (UINT ikey=0; ikey<3*12; ikey++)
	{
		stmp[0] = 0;
		CMainFrame::GetKeyName(kbdmap[ikey] << 16, stmp, sizeof(stmp)-1);
		if ((stmp[0] > ' ') && (!stmp[1]))
		{
			wsprintf(s, "%s%d: Shift+%c", szNoteNames[ikey % 12], ikey/12, stmp[0]);
			m_CbnShortcut.SetItemData(m_CbnShortcut.AddString(s), ikey);
		}
	}
	m_CbnShortcut.SetCurSel(0);
	// Base Note combo box
	for (UINT ibase=0; ibase<3*12; ibase++)
	{
		wsprintf(s, "%s%d", szNoteNames[ibase % 12], ibase / 12);
		m_CbnBaseNote.SetItemData(m_CbnBaseNote.AddString(s), ibase);
	}
	// Minor notes
	for (int inotes=-1; inotes<24; inotes++)
	{
		if (inotes < 0) strcpy(s, "--"); else
		if (inotes < 12) wsprintf(s, "%s", szNoteNames[inotes % 12]);
		else wsprintf(s, "%s (+%d)", szNoteNames[inotes % 12], inotes / 12);
		m_CbnNote1.AddString(s);
		m_CbnNote2.AddString(s);
		m_CbnNote3.AddString(s);
	}
	// Update Dialog
	OnChordChanged();
	return TRUE;
}


LRESULT CChordEditor::OnKeyboardNotify(WPARAM wParam, LPARAM nKey)
//----------------------------------------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;
	int chord;

	if (wParam != KBDNOTIFY_LBUTTONDOWN) return 0;
	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return 0;
	pChords = pMainFrm->GetChords();
	chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	UINT cnote = 0;
	pChords[chord].notes[0] = 0;
	pChords[chord].notes[1] = 0;
	pChords[chord].notes[2] = 0;
	for (UINT i=0; i<2*12; i++) if (i != (UINT)(pChords[chord].key % 12))
	{
		UINT n = m_Keyboard.GetFlags(i);
		if (i == (UINT)nKey) n = (n) ? 0 : 1;
		if (n)
		{
			if ((cnote < 3) || (i == (UINT)nKey))
			{
				UINT k = (cnote < 3) ? cnote : 2;
				pChords[chord].notes[k] = i+1;
				if (cnote < 3) cnote++;
			}
		}
	}
	OnChordChanged();
	return 0;
}


void CChordEditor::OnChordChanged()
//---------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;
	int chord;

	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return;
	pChords = pMainFrm->GetChords();
	chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	m_CbnBaseNote.SetCurSel(pChords[chord].key);
	m_CbnNote1.SetCurSel(pChords[chord].notes[0]);
	m_CbnNote2.SetCurSel(pChords[chord].notes[1]);
	m_CbnNote3.SetCurSel(pChords[chord].notes[2]);
	UpdateKeyboard();
}


void CChordEditor::UpdateKeyboard()
//---------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;
	int chord;
	UINT note, octave;
	
	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return;
	pChords = pMainFrm->GetChords();
	chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	note = pChords[chord].key % 12;
	octave = pChords[chord].key / 12;
	for (UINT i=0; i<2*12; i++)
	{
		BOOL b = FALSE;

		if (i == note) b = TRUE;
		if ((pChords[chord].notes[0]) && (i+1 == pChords[chord].notes[0])) b = TRUE;
		if ((pChords[chord].notes[1]) && (i+1 == pChords[chord].notes[1])) b = TRUE;
		if ((pChords[chord].notes[2]) && (i+1 == pChords[chord].notes[2])) b = TRUE;
		m_Keyboard.SetFlags(i, (b) ? 1 : 0);
	}
	m_Keyboard.InvalidateRect(NULL, FALSE);
}


void CChordEditor::OnBaseNoteChanged()
//------------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;

	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return;
	pChords = pMainFrm->GetChords();
	int chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	int basenote = m_CbnBaseNote.GetCurSel();
	if (basenote >= 0)
	{
		pChords[chord].key = (BYTE)basenote;
		UpdateKeyboard();
	}
}


void CChordEditor::OnNote1Changed()
//---------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;

	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return;
	pChords = pMainFrm->GetChords();
	int chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	int note = m_CbnNote1.GetCurSel();
	if (note >= 0)
	{
		pChords[chord].notes[0] = (BYTE)note;
		UpdateKeyboard();
	}
}


void CChordEditor::OnNote2Changed()
//---------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;

	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return;
	pChords = pMainFrm->GetChords();
	int chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	int note = m_CbnNote2.GetCurSel();
	if (note >= 0)
	{
		pChords[chord].notes[1] = (BYTE)note;
		UpdateKeyboard();
	}
}


void CChordEditor::OnNote3Changed()
//---------------------------------
{
	CMainFrame *pMainFrm;
	MPTCHORD *pChords;

	if ((pMainFrm = CMainFrame::GetMainFrame()) == NULL) return;
	pChords = pMainFrm->GetChords();
	int chord = m_CbnShortcut.GetCurSel();
	if (chord >= 0) chord = m_CbnShortcut.GetItemData(chord);
	if ((chord < 0) || (chord >= 3*12)) chord = 0;
	int note = m_CbnNote3.GetCurSel();
	if (note >= 0)
	{
		pChords[chord].notes[2] = (BYTE)note;
		UpdateKeyboard();
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// Sample Map
//

BEGIN_MESSAGE_MAP(CSampleMapDlg, CDialog)
	ON_MESSAGE(WM_MOD_KBDNOTIFY,	OnKeyboardNotify)
	ON_WM_HSCROLL()
	ON_COMMAND(IDC_CHECK1,			OnUpdateSamples)
	ON_CBN_SELCHANGE(IDC_COMBO1,	OnUpdateKeyboard)
END_MESSAGE_MAP()

void CSampleMapDlg::DoDataExchange(CDataExchange* pDX)
//----------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSampleMapDlg)
	DDX_Control(pDX, IDC_KEYBOARD1,		m_Keyboard);
	DDX_Control(pDX, IDC_COMBO1,		m_CbnSample);
	DDX_Control(pDX, IDC_SLIDER1,		m_SbOctave);
	//}}AFX_DATA_MAP
}


BOOL CSampleMapDlg::OnInitDialog()
//--------------------------------
{
	CDialog::OnInitDialog();
	if (m_pSndFile)
	{
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			for (UINT i=0; i<120; i++)
			{
				KeyboardMap[i] = penv->Keyboard[i];
			}
		}
	}
	m_Keyboard.Init(m_hWnd, 3, TRUE);
	m_SbOctave.SetRange(0, 7);
	m_SbOctave.SetPos(4);
	OnUpdateSamples();
	OnUpdateOctave();
	return TRUE;
}


VOID CSampleMapDlg::OnHScroll(UINT nCode, UINT nPos, CScrollBar *pBar)
//--------------------------------------------------------------------
{
	CDialog::OnHScroll(nCode, nPos, pBar);
	OnUpdateKeyboard();
	OnUpdateOctave();
}


VOID CSampleMapDlg::OnUpdateSamples()
//-----------------------------------
{
	UINT nOldPos = 0;
	UINT nNewPos = 0;
	BOOL bAll;
	
	if ((!m_pSndFile) || (m_nInstrument >= MAX_INSTRUMENTS)) return;
	if (m_CbnSample.GetCount() > 0)
	{
		nOldPos = m_CbnSample.GetItemData(m_CbnSample.GetCurSel());
	}
	m_CbnSample.ResetContent();
	bAll = IsDlgButtonChecked(IDC_CHECK1);
	for (UINT i=1; i<=m_pSndFile->m_nSamples; i++)
	{
		BOOL bUsed = bAll;

		if (!bUsed)
		{
			for (UINT j=0; j<120; j++)
			{
				if (KeyboardMap[j] == i)
				{
					bUsed = TRUE;
					break;
				}
			}
		}
		if (bUsed)
		{
			CHAR s[64];
			wsprintf(s, "%d: ", i);
			m_pSndFile->GetSampleName(i, s+strlen(s));
			UINT nPos = m_CbnSample.AddString(s);
			m_CbnSample.SetItemData(nPos, i);
			if (i == nOldPos) nNewPos = nPos;
		}
	}
	m_CbnSample.SetCurSel(nNewPos);
	OnUpdateKeyboard();
}


VOID CSampleMapDlg::OnUpdateOctave()
//----------------------------------
{
	CHAR s[64];

	UINT nBaseOctave = m_SbOctave.GetPos() & 7;
	wsprintf(s, "Octaves %d-%d", nBaseOctave, nBaseOctave+2);
	SetDlgItemText(IDC_TEXT1, s);
}



VOID CSampleMapDlg::OnUpdateKeyboard()
//------------------------------------
{
	UINT nSample = m_CbnSample.GetItemData(m_CbnSample.GetCurSel());
	UINT nBaseOctave = m_SbOctave.GetPos() & 7;
	BOOL bRedraw = FALSE;
	for (UINT iNote=0; iNote<3*12; iNote++)
	{
		UINT nOld = m_Keyboard.GetFlags(iNote);
		UINT ndx = nBaseOctave*12+iNote;
		UINT nNew = (KeyboardMap[ndx] == nSample) ? CKeyboardControl::KEYFLAG_REDDOT : CKeyboardControl::KEYFLAG_NORMAL;
		if (nNew != nOld)
		{
			m_Keyboard.SetFlags(iNote, nNew);
			bRedraw = TRUE;
		}
	}
	if (bRedraw) m_Keyboard.InvalidateRect(NULL, FALSE);
}


LRESULT CSampleMapDlg::OnKeyboardNotify(WPARAM wParam, LPARAM lParam)
//-------------------------------------------------------------------
{
	CHAR s[32] = "--";

	if ((lParam >= 0) && (lParam < 3*12) && (m_pSndFile))
	{
		UINT nSample = m_CbnSample.GetItemData(m_CbnSample.GetCurSel());
		UINT nBaseOctave = m_SbOctave.GetPos() & 7;
		wsprintf(s, "%s%d", szNoteNames[lParam%12], lParam/12+nBaseOctave);
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if ((wParam == KBDNOTIFY_LBUTTONDOWN) && (nSample > 0) && (nSample < MAX_SAMPLES) && (penv))
		{
			UINT iNote = nBaseOctave*12+lParam;
			if (KeyboardMap[iNote] == nSample)
			{
				KeyboardMap[iNote] = penv->Keyboard[iNote];
			} else
			{
				KeyboardMap[iNote] = (BYTE)nSample;
			}
			OnUpdateKeyboard();
		}
	}
	SetDlgItemText(IDC_TEXT2, s);
	return 0;
}


VOID CSampleMapDlg::OnOK()
//------------------------
{
	if (m_pSndFile)
	{
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			BOOL bModified = FALSE;
			for (UINT i=0; i<120; i++)
			{
				if (KeyboardMap[i] != penv->Keyboard[i])
				{
					penv->Keyboard[i] = KeyboardMap[i];
					bModified = TRUE;
				}
			}
			if (bModified)
			{
				CDialog::OnOK();
				return;
			}
		}
	}
	CDialog::OnCancel();
}

