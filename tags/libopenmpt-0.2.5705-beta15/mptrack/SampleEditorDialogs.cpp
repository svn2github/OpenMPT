/*
 * SampleEditorDialogs.cpp
 * -----------------------
 * Purpose: Code for various dialogs that are used in the sample editor.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "resource.h"
#include "Reporting.h"
#include "MPTrackUtil.h"
#include "../common/misc_util.h"
#include "../soundlib/Snd_defs.h"
#include "../soundlib/ModSample.h"
#include "SampleEditorDialogs.h"


OPENMPT_NAMESPACE_BEGIN


//////////////////////////////////////////////////////////////////////////
// Sample amplification dialog

void CAmpDlg::DoDataExchange(CDataExchange* pDX)
//----------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAmpDlg)
	DDX_Control(pDX, IDC_COMBO1,	m_fadeBox);
	//}}AFX_DATA_MAP
}

CAmpDlg::CAmpDlg(CWnd *parent, int16 factor, Fade::Law fadeLaw, int16 factorMin, int16 factorMax)
//-----------------------------------------------------------------------------------------------
	: CDialog(IDD_SAMPLE_AMPLIFY, parent)
	, m_nFactor(factor)
	, m_nFactorMin(factorMin)
	, m_nFactorMax(factorMax)
	, m_bFadeIn(FALSE)
	, m_bFadeOut(FALSE)
	, m_fadeLaw(fadeLaw)
{}

BOOL CAmpDlg::OnInitDialog()
//--------------------------
{
	CDialog::OnInitDialog();
	CSpinButtonCtrl *spin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1);
	if (spin)
	{
		spin->SetRange32(m_nFactorMin, m_nFactorMax);
		spin->SetPos32(m_nFactor);
	}
	SetDlgItemInt(IDC_EDIT1, m_nFactor);
	m_edit.SubclassDlgItem(IDC_EDIT1, this);
	m_edit.AllowFractions(false);
	m_edit.AllowNegative(m_nFactorMin < 0);

	const struct
	{
		TCHAR *name;
		Fade::Law id;
	} fadeLaws[] =
	{
		{ _T("Linear"),       Fade::kLinear },
		{ _T("Exponential"),  Fade::kPow },
		{ _T("Square Root"),  Fade::kSqrt },
		{ _T("Logarithmic"),  Fade::kLog },
		{ _T("Quarter Sine"), Fade::kQuarterSine },
		{ _T("Half Sine"),    Fade::kHalfSine },
	};
	// Create icons for fade laws
	const int cx = Util::ScalePixels(16, m_hWnd);
	const int cy = Util::ScalePixels(16, m_hWnd);
	m_list.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 0, 1);
	std::vector<COLORREF> bits;
	const COLORREF col = GetSysColor(COLOR_WINDOWTEXT);
	for(size_t i = 0; i < CountOf(fadeLaws); i++)
	{
		bits.assign(cx * cy, RGB(255, 0, 255));
		Fade::Func fadeFunc = Fade::GetFadeFunc(static_cast<Fade::Law>(i));
		for(int x = 0; x < cx; x++)
		{
			int32 val = cy - fadeFunc(cy, x, cx) - 1;
			Limit(val, 0, cy - 1);
			bits[x + val * cx] = col;
			// Draw another pixel for fake interpolation
			val = cy - fadeFunc(cy, x * 2 + 1, cx * 2) - 1;
			Limit(val, 0, cy - 1);
			bits[x + val * cx] = col;
		}
		CBitmap bitmap;
		bitmap.CreateBitmap(cx, cy, 1, 32, &bits[0]);
		m_list.Add(&bitmap, RGB(255, 0, 255));
		bitmap.DeleteObject();
	}
	m_fadeBox.SetImageList(&m_list);

	// Add fade laws to list
	COMBOBOXEXITEM cbi;
	MemsetZero(cbi);
	cbi.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;
	for(size_t i = 0; i < CountOf(fadeLaws); i++)
	{
		cbi.iItem = i;
		cbi.pszText = fadeLaws[i].name;
		cbi.iImage = cbi.iSelectedImage = i;
		cbi.lParam = fadeLaws[i].id;
		m_fadeBox.InsertItem(&cbi);
		if(fadeLaws[i].id == m_fadeLaw) m_fadeBox.SetCurSel(i);
	}

	return TRUE;
}


void CAmpDlg::OnDestroy()
//-----------------------
{
	m_list.DeleteImageList();
}


void CAmpDlg::OnOK()
//------------------
{
	int nVal = static_cast<int>(GetDlgItemInt(IDC_EDIT1));
	Limit(nVal, m_nFactorMin, m_nFactorMax);
	m_nFactor = static_cast<int16>(nVal);
	m_bFadeIn = (IsDlgButtonChecked(IDC_CHECK1) != BST_UNCHECKED);
	m_bFadeOut = (IsDlgButtonChecked(IDC_CHECK2) != BST_UNCHECKED);
	m_fadeLaw = static_cast<Fade::Law>(m_fadeBox.GetItemData(m_fadeBox.GetCurSel()));
	CDialog::OnOK();
}


//////////////////////////////////////////////////////////////
// Sample import dialog

SampleIO CRawSampleDlg::m_nFormat(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::unsignedPCM);

BOOL CRawSampleDlg::OnInitDialog()
//--------------------------------
{
	CDialog::OnInitDialog();
	UpdateDialog();
	return TRUE;
}


void CRawSampleDlg::OnOK()
//------------------------
{
	if(IsDlgButtonChecked(IDC_RADIO1)) m_nFormat |= SampleIO::_8bit;
	if(IsDlgButtonChecked(IDC_RADIO2)) m_nFormat |= SampleIO::_16bit;
	if(IsDlgButtonChecked(IDC_RADIO3)) m_nFormat |= SampleIO::unsignedPCM;
	if(IsDlgButtonChecked(IDC_RADIO4)) m_nFormat |= SampleIO::signedPCM;
	if(IsDlgButtonChecked(IDC_RADIO5)) m_nFormat |= SampleIO::mono;
	if(IsDlgButtonChecked(IDC_RADIO6)) m_nFormat |= SampleIO::stereoInterleaved;
	m_bRememberFormat = IsDlgButtonChecked(IDC_CHK_REMEMBERSETTINGS) != BST_UNCHECKED;
	CDialog::OnOK();
}


void CRawSampleDlg::UpdateDialog()
//--------------------------------
{
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, (m_nFormat.GetBitDepth() == 8) ? IDC_RADIO1 : IDC_RADIO2 );
	CheckRadioButton(IDC_RADIO3, IDC_RADIO4, (m_nFormat.GetEncoding() == SampleIO::unsignedPCM) ? IDC_RADIO3 : IDC_RADIO4);
	CheckRadioButton(IDC_RADIO5, IDC_RADIO6, (m_nFormat.GetChannelFormat() == SampleIO::mono) ? IDC_RADIO5 : IDC_RADIO6);
	CheckDlgButton(IDC_CHK_REMEMBERSETTINGS, (m_bRememberFormat ? BST_CHECKED : BST_UNCHECKED));
}


/////////////////////////////////////////////////////////////////////////
// Add silence dialog - add silence to a sample

BEGIN_MESSAGE_MAP(CAddSilenceDlg, CDialog)
	ON_COMMAND(IDC_RADIO_ADDSILENCE_BEGIN,		OnEditModeChanged)
	ON_COMMAND(IDC_RADIO_ADDSILENCE_END,		OnEditModeChanged)
	ON_COMMAND(IDC_RADIO_RESIZETO,				OnEditModeChanged)
END_MESSAGE_MAP()


BOOL CAddSilenceDlg::OnInitDialog()
//---------------------------------
{
	CDialog::OnInitDialog();

	CSpinButtonCtrl *spin = (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_ADDSILENCE);
	if (spin)
	{
		spin->SetRange32(0, int32_max);
		spin->SetPos32(m_nSamples);
	}

	int iRadioButton = IDC_RADIO_ADDSILENCE_END;
	switch(m_nEditOption)
	{
	case addsilence_at_beginning:
		iRadioButton = IDC_RADIO_ADDSILENCE_BEGIN;
		break;
	case addsilence_at_end:
		iRadioButton = IDC_RADIO_ADDSILENCE_END;
		break;
	case addsilence_resize:
		iRadioButton = IDC_RADIO_RESIZETO;
		break;
	}
	CButton *radioEnd = (CButton *)GetDlgItem(iRadioButton);
	radioEnd->SetCheck(true);

	SetDlgItemInt(IDC_EDIT_ADDSILENCE, (m_nEditOption == addsilence_resize) ? m_nLength : m_nSamples, FALSE);

	return TRUE;
}


void CAddSilenceDlg::OnOK()
//-------------------------
{
	m_nSamples = GetDlgItemInt(IDC_EDIT_ADDSILENCE, nullptr, FALSE);
	m_nEditOption = GetEditMode();
	CDialog::OnOK();
}


void CAddSilenceDlg::OnEditModeChanged()
//--------------------------------------
{
	enmAddSilenceOptions cNewEditOption = GetEditMode();
	if(cNewEditOption != addsilence_resize && m_nEditOption == addsilence_resize)
	{
		// switch to "add silenece"
		m_nLength = GetDlgItemInt(IDC_EDIT_ADDSILENCE);
		SetDlgItemInt(IDC_EDIT_ADDSILENCE, m_nSamples);
	} else if(cNewEditOption == addsilence_resize && m_nEditOption != addsilence_resize)
	{
		// switch to "resize"
		m_nSamples = GetDlgItemInt(IDC_EDIT_ADDSILENCE);
		SetDlgItemInt(IDC_EDIT_ADDSILENCE, m_nLength);
	}
	m_nEditOption = cNewEditOption;
}


enmAddSilenceOptions CAddSilenceDlg::GetEditMode()
//------------------------------------------------
{
	if(IsDlgButtonChecked(IDC_RADIO_ADDSILENCE_BEGIN)) return addsilence_at_beginning;
	else if(IsDlgButtonChecked(IDC_RADIO_ADDSILENCE_END)) return addsilence_at_end;
	else if(IsDlgButtonChecked(IDC_RADIO_RESIZETO)) return addsilence_resize;
	return addsilence_at_end;
}


/////////////////////////////////////////////////////////////////////////
// Sample grid dialog

void CSampleGridDlg::DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSampleGridDlg)
	DDX_Control(pDX, IDC_EDIT1,			m_EditSegments);
	DDX_Control(pDX, IDC_SPIN1,			m_SpinSegments);
	//}}AFX_DATA_MAP
}


BOOL CSampleGridDlg::OnInitDialog()
//---------------------------------
{
	CDialog::OnInitDialog();
	m_SpinSegments.SetRange32(0, m_nMaxSegments);
	m_SpinSegments.SetPos(m_nSegments);
	SetDlgItemInt(IDC_EDIT1, m_nSegments, FALSE);
	GetDlgItem(IDC_EDIT1)->SetFocus();
	return TRUE;
}


void CSampleGridDlg::OnOK()
//-------------------------
{
	m_nSegments = GetDlgItemInt(IDC_EDIT1, NULL, FALSE);
	CDialog::OnOK();
}


/////////////////////////////////////////////////////////////////////////
// Sample cross-fade dialog

uint32 CSampleXFadeDlg::m_fadeLength  = 20000;
uint32 CSampleXFadeDlg::m_fadeLaw = 50000;
bool CSampleXFadeDlg::m_afterloopFade = true;

BEGIN_MESSAGE_MAP(CSampleXFadeDlg, CDialog)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT1,	OnFadeLengthChanged)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipText)
END_MESSAGE_MAP()


void CSampleXFadeDlg::DoDataExchange(CDataExchange* pDX)
//------------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSampleGridDlg)
	DDX_Control(pDX, IDC_EDIT1,			m_EditSamples);
	DDX_Control(pDX, IDC_SPIN1,			m_SpinSamples);
	DDX_Control(pDX, IDC_SLIDER1,		m_SliderLength);
	DDX_Control(pDX, IDC_SLIDER2,		m_SliderFadeLaw);
	//}}AFX_DATA_MAP
}


BOOL CSampleXFadeDlg::OnInitDialog()
//----------------------------------
{
	CDialog::OnInitDialog();
	m_editLocked = true;
	m_SpinSamples.SetRange32(0, std::min(m_loopLength, m_maxLength));
	GetDlgItem(IDC_EDIT1)->SetFocus();
	m_SliderLength.SetRange(0, 100000);
	m_SliderLength.SetPos(m_fadeLength);
	m_SliderFadeLaw.SetRange(0, 100000);
	m_SliderFadeLaw.SetPos(m_fadeLaw);
	CheckDlgButton(IDC_CHECK1, m_afterloopFade ? BST_CHECKED : BST_UNCHECKED);

	SmpLength numSamples = PercentToSamples(m_SliderLength.GetPos());
	numSamples = Util::Min(numSamples, m_loopLength, m_maxLength);
	m_SpinSamples.SetPos(numSamples);
	SetDlgItemInt(IDC_EDIT1, numSamples, FALSE);

	m_editLocked = false;
	return TRUE;
}


void CSampleXFadeDlg::OnOK()
//--------------------------
{
	m_fadeLength = m_SliderLength.GetPos();
	m_fadeLaw = m_SliderFadeLaw.GetPos();
	m_afterloopFade = IsDlgButtonChecked(IDC_CHECK1) != BST_UNCHECKED;
	Limit(m_fadeLength, uint32(0), uint32(100000));
	CDialog::OnOK();
}


void CSampleXFadeDlg::OnFadeLengthChanged()
//-----------------------------------------
{
	if(m_editLocked) return;
	SmpLength numSamples = GetDlgItemInt(IDC_EDIT1, NULL, FALSE);
	numSamples = Util::Min(numSamples, m_loopLength, m_maxLength);
	m_SliderLength.SetPos(SamplesToPercent(numSamples));
}


void CSampleXFadeDlg::OnHScroll(UINT, UINT, CScrollBar *sb)
//---------------------------------------------------------
{
	if(sb == (CScrollBar *)(&m_SliderLength))
	{
		m_editLocked = true;
		SmpLength numSamples = PercentToSamples(m_SliderLength.GetPos());
		if(numSamples > m_maxLength)
		{
			numSamples = m_maxLength;
			m_SliderLength.SetPos(SamplesToPercent(numSamples));
		}
		m_SpinSamples.SetPos(numSamples);
		SetDlgItemInt(IDC_EDIT1, numSamples, FALSE);
		m_editLocked = false;
	}
}


BOOL CSampleXFadeDlg::OnToolTipText(UINT, NMHDR *pNMHDR, LRESULT *pResult)
//------------------------------------------------------------------------
{
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	if(pTTT->uFlags & TTF_IDISHWND)
	{
		// idFrom is actually the HWND of the tool
		nID = (UINT_PTR)::GetDlgCtrlID((HWND)nID);
	}
	switch(nID)
	{
	case IDC_SLIDER1:
		{
			uint32 percent = m_SliderLength.GetPos();
			wsprintf(pTTT->szText, _T("%u.%03u%% of the loop (%u samples)"), percent / 1000, percent % 1000, PercentToSamples(percent));
		}
		break;
	case IDC_SLIDER2:
		_tcscpy(pTTT->szText, _T("Slide towards constant power for fixing badly looped samples."));
		break;
	default:
		return FALSE;
	}
	*pResult = 0;

	// bring the tooltip window above other popup windows
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////
// Resampling dialog

CResamplingDlg::ResamplingOption CResamplingDlg::lastChoice = CResamplingDlg::Upsample;
uint32 CResamplingDlg::lastFrequency = 0;

BEGIN_MESSAGE_MAP(CResamplingDlg, CDialog)
	ON_EN_SETFOCUS(IDC_EDIT1, OnFocusEdit)
END_MESSAGE_MAP()

BOOL CResamplingDlg::OnInitDialog()
//---------------------------------
{
	CDialog::OnInitDialog();
	CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1 + lastChoice);
	TCHAR s[32];
	wsprintf(s, _T("&Upsample (%u Hz)"), frequency * 2);
	SetDlgItemText(IDC_RADIO1, s);
	wsprintf(s, _T("&Downsample (%u Hz)"), frequency / 2);
	SetDlgItemText(IDC_RADIO2, s);
	
	if(!lastFrequency) lastFrequency = frequency;
	SetDlgItemInt(IDC_EDIT1, lastFrequency, FALSE);
	CSpinButtonCtrl *spin = static_cast<CSpinButtonCtrl *>(GetDlgItem(IDC_SPIN1));
	spin->SetRange32(100, 999999);
	spin->SetPos32(lastFrequency);
	return TRUE;
}


void CResamplingDlg::OnOK()
//-------------------------
{
	if(IsDlgButtonChecked(IDC_RADIO1))
	{
		lastChoice = Upsample;
		frequency *= 2;
	} else if(IsDlgButtonChecked(IDC_RADIO2))
	{
		lastChoice = Downsample;
		frequency /= 2;
	} else
	{
		lastChoice = Custom;
		frequency = GetDlgItemInt(IDC_EDIT1, NULL, FALSE);
		if(frequency >= 100)
		{
			lastFrequency = frequency;
		} else
		{
			MessageBeep(MB_ICONWARNING);
			GetDlgItem(IDC_EDIT1)->SetFocus();
			return;
		}
	}

	CDialog::OnOK();
}


OPENMPT_NAMESPACE_END
