#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "moddoc.h"
#include "globals.h"
#include "ctrl_ins.h"
#include "view_ins.h"
#include "dlg_misc.h"

#pragma warning(disable:4244)

/////////////////////////////////////////////////////////////////////////
// CNoteMapWnd

#define ID_NOTEMAP_EDITSAMPLE	40000

BEGIN_MESSAGE_MAP(CNoteMapWnd, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_NOTEMAP_COPY,			OnMapCopy)
	ON_COMMAND(ID_NOTEMAP_RESET,		OnMapReset)
	ON_COMMAND(ID_INSTRUMENT_SAMPLEMAP, OnEditSampleMap)
	ON_COMMAND(ID_INSTRUMENT_DUPLICATE, OnInstrumentDuplicate)
	ON_COMMAND_RANGE(ID_NOTEMAP_EDITSAMPLE, ID_NOTEMAP_EDITSAMPLE+MAX_SAMPLES, OnEditSample)
END_MESSAGE_MAP()


BOOL CNoteMapWnd::PreTranslateMessage(MSG* pMsg)
//----------------------------------------------
{
	UINT wParam;
	if (!pMsg) return TRUE;
	wParam = pMsg->wParam;
	if ((pMsg->message == WM_CHAR) && (m_pModDoc))
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];
		if ((penv) && (m_nNote < 120))
		{
			if ((m_bIns) && (((wParam >= '0') && (wParam <= '9')) || (wParam == ' ')))
			{
				UINT n = m_nOldIns;
				if (wParam != ' ')
				{
					n = (10*penv->Keyboard[m_nNote] + (wParam - '0')) % 10000;
					if ((n >= MAX_SAMPLES) || ((pSndFile->m_nSamples < 1000) && (n >= 1000))) n = (n % 1000);
					if ((n >= MAX_SAMPLES) || ((pSndFile->m_nSamples < 100) && (n >= 100))) n = (n % 100); else
					if ((n > 31) && (pSndFile->m_nSamples < 32) && (n % 10)) n = (n % 10);
				}
				if (n != penv->Keyboard[m_nNote])
				{
					penv->Keyboard[m_nNote] = n;
					m_pModDoc->SetModified();
					InvalidateRect(NULL, FALSE);
				}
				if (wParam == ' ')
				{
					if (m_nNote < 119) m_nNote++;
					InvalidateRect(NULL, FALSE);
				}
				return TRUE;
			} else
			if ((!m_bIns) || (pSndFile->m_nType & MOD_TYPE_IT))
			{
				UINT note = CMainFrame::GetNoteFromKey(wParam, HIWORD(pMsg->lParam));
				UINT n = penv->NoteMap[m_nNote];
				BOOL bOk = FALSE;
				if ((note > 0) && (note <= 120))
				{
					n = note;
					bOk = TRUE;
				} else
				if ((wParam >= '0') && (wParam <= '9'))
				{
					if (n)
					{
						n = ((n-1) % 12) + (wParam-'0')*12 + 1;
					} else
					{
						n = (m_nNote % 12) + (wParam-'0')*12 + 1;
					}
				} else
				if (wParam == ' ')
				{
					n = (m_nOldNote) ? m_nOldNote : m_nNote+1;
				}
				if (n != penv->NoteMap[m_nNote])
				{
					penv->NoteMap[m_nNote] = n;
					m_pModDoc->SetModified();
					InvalidateRect(NULL, FALSE);
				}
				if (bOk) SetCurrentNote(m_nNote+1);
				return TRUE;
			}
			return TRUE;
		}
	} else
	if (pMsg->message == WM_KEYDOWN)
	{
		BOOL bRedraw = FALSE, bOk = TRUE;
		switch(wParam)
		{
		case VK_RIGHT:
			if (!m_bIns) { m_bIns = TRUE; bRedraw = TRUE; } else
			if (m_nNote < 119) { m_nNote++; m_bIns = FALSE; bRedraw = TRUE; }
			break;
		case VK_LEFT:
			if (m_bIns) { m_bIns = FALSE; bRedraw = TRUE; } else
			if (m_nNote) { m_nNote--; m_bIns = TRUE; bRedraw = TRUE; }
			break;
		case VK_UP:
			if (m_nNote > 0) { m_nNote--; bRedraw = TRUE; }
			break;
		case VK_DOWN:
			if (m_nNote < 119) { m_nNote++; bRedraw = TRUE; }
			break;
		case VK_PRIOR:
			if (m_nNote > 3) { m_nNote-=3; bRedraw = TRUE; } else
			if (m_nNote > 0) { m_nNote = 0; bRedraw = TRUE; }
			break;
		case VK_NEXT:
			if (m_nNote+3 < 120) { m_nNote+=3; bRedraw = TRUE; } else
			if (m_nNote < 119) { m_nNote = 119; bRedraw = TRUE; }
			break;
		case VK_TAB:
			return TRUE;
		case VK_RETURN:
			if (m_pModDoc)
			{
				INSTRUMENTHEADER *penv = m_pModDoc->GetSoundFile()->Headers[m_nInstrument];
				if (m_bIns) m_nOldIns = penv->Keyboard[m_nNote];
				else m_nOldNote = penv->NoteMap[m_nNote];
			}
			return TRUE;
		default:
			bOk = FALSE;
		}
		if (bRedraw) InvalidateRect(NULL, FALSE);
		if (bOk) return TRUE;
	}
	return CStatic::PreTranslateMessage(pMsg);
}


BOOL CNoteMapWnd::SetCurrentInstrument(CModDoc *pModDoc, UINT nIns)
//-----------------------------------------------------------------
{
	if ((pModDoc != m_pModDoc) || (nIns != m_nInstrument))
	{
		m_pModDoc = pModDoc;
		if (nIns < MAX_INSTRUMENTS) m_nInstrument = nIns;
		InvalidateRect(NULL, FALSE);
	}
	return TRUE;
}


BOOL CNoteMapWnd::SetCurrentNote(UINT nNote)
//------------------------------------------
{
	if (nNote == m_nNote) return TRUE;
	if (nNote >= 120) return FALSE;
	m_nNote = nNote;
	InvalidateRect(NULL, FALSE);
	return TRUE;
}


void CNoteMapWnd::OnPaint()
//-------------------------
{
	HGDIOBJ oldfont = NULL;
	CRect rcClient;
	CPaintDC dc(this);
	HDC hdc;

	GetClientRect(&rcClient);
	if (!m_hFont)
	{
		m_hFont = CMainFrame::GetGUIFont();
		colorText = GetSysColor(COLOR_WINDOWTEXT);
		colorTextSel = GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	hdc = dc.m_hDC;
	oldfont = ::SelectObject(hdc, m_hFont);
	dc.SetBkMode(TRANSPARENT);
	if ((m_cxFont <= 0) || (m_cyFont <= 0))
	{
		CSize sz;
		sz = dc.GetTextExtent("C#0.", 4);
		m_cyFont = sz.cy + 2;
		m_cxFont = rcClient.right / 3;
	}
	dc.IntersectClipRect(&rcClient);
	if ((m_pModDoc) && (m_cxFont > 0) && (m_cyFont > 0))
	{
		BOOL bFocus = (::GetFocus() == m_hWnd) ? TRUE : FALSE;
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];
		CHAR s[64];
		CRect rect;

		int nNotes = (rcClient.bottom + m_cyFont - 1) / m_cyFont;
		int nPos = m_nNote - (nNotes/2);
		int ypaint = 0;
		for (int ynote=0; ynote<nNotes; ynote++, ypaint+=m_cyFont, nPos++)
		{
			BOOL bHighLight;

			// Note
			s[0] = 0;
			if ((nPos >= 0) && (nPos < 120)) wsprintf(s, "%s%d", szNoteNames[nPos % 12], nPos/12);
			rect.SetRect(0, ypaint, m_cxFont, ypaint+m_cyFont);
			DrawButtonRect(hdc, &rect, s, FALSE, FALSE);
			// Mapped Note
			bHighLight = ((bFocus) && (nPos == (int)m_nNote) /*&& (!m_bIns)*/) ? TRUE : FALSE;
			rect.left = rect.right;
			rect.right = m_cxFont*2-1;
			strcpy(s, "...");
			if ((penv) && (nPos >= 0) && (nPos < 120) && (penv->NoteMap[nPos]))
			{
				UINT n = penv->NoteMap[nPos];
				if (n == 0xFF) strcpy(s, "==="); else
				if (n == 0xFE) strcpy(s, "^^^"); else
				if (n <= 120) wsprintf(s, "%s%d", szNoteNames[(n-1)%12], (n-1)/12);
			}
			FillRect(hdc, &rect, (bHighLight) ? CMainFrame::brushHighLight : CMainFrame::brushWindow);
			if ((nPos == (int)m_nNote) && (!m_bIns))
			{
				rect.InflateRect(-1, -1);
				dc.DrawFocusRect(&rect);
				rect.InflateRect(1, 1);
			}
			dc.SetTextColor((bHighLight) ? colorTextSel : colorText);
			dc.DrawText(s, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			// Sample
			bHighLight = ((bFocus) && (nPos == (int)m_nNote) /*&& (m_bIns)*/) ? TRUE : FALSE;
			rect.left = rcClient.left + m_cxFont*2+3;
			rect.right = rcClient.right;
			strcpy(s, " ..");
			if ((penv) && (nPos >= 0) && (nPos < 120) && (penv->Keyboard[nPos]))
			{
				wsprintf(s, "%3d", penv->Keyboard[nPos]);
			}
			FillRect(hdc, &rect, (bHighLight) ? CMainFrame::brushHighLight : CMainFrame::brushWindow);
			if ((nPos == (int)m_nNote) && (m_bIns))
			{
				rect.InflateRect(-1, -1);
				dc.DrawFocusRect(&rect);
				rect.InflateRect(1, 1);
			}
			dc.SetTextColor((bHighLight) ? colorTextSel : colorText);
			dc.DrawText(s, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
		rect.SetRect(rcClient.left+m_cxFont*2-1, rcClient.top, rcClient.left+m_cxFont*2+3, ypaint);
		DrawButtonRect(hdc, &rect, "", FALSE, FALSE);
		if (ypaint < rcClient.bottom)
		{
			rect.SetRect(rcClient.left, ypaint, rcClient.right, rcClient.bottom); 
			FillRect(hdc, &rect, CMainFrame::brushGray);
		}
	}
	if (oldfont) ::SelectObject(hdc, oldfont);
}


void CNoteMapWnd::OnSetFocus(CWnd *pOldWnd)
//-----------------------------------------
{
	CWnd::OnSetFocus(pOldWnd);
	InvalidateRect(NULL, FALSE);
}


void CNoteMapWnd::OnKillFocus(CWnd *pNewWnd)
//------------------------------------------
{
	CWnd::OnKillFocus(pNewWnd);
	InvalidateRect(NULL, FALSE);
}


void CNoteMapWnd::OnLButtonDown(UINT, CPoint pt)
//----------------------------------------------
{
	if ((pt.x >= m_cxFont) && (pt.x < m_cxFont*2) && (m_bIns))
	{
		m_bIns = FALSE;
		InvalidateRect(NULL, FALSE);
	}
	if ((pt.x > m_cxFont*2) && (pt.x <= m_cxFont*3) && (!m_bIns))
	{
		m_bIns = TRUE;
		InvalidateRect(NULL, FALSE);
	}
	if ((pt.x >= m_cxFont) && (m_cyFont))
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		int nNotes = (rcClient.bottom + m_cyFont - 1) / m_cyFont;
		UINT n = (pt.y / m_cyFont) + m_nNote - (nNotes/2);
		if ((n < m_nNote) && (m_nNote > 0))
		{
			m_nNote--;
			InvalidateRect(NULL, FALSE);
		} else
		if ((n > m_nNote) && (m_nNote < 119))
		{
			m_nNote++;
			InvalidateRect(NULL, FALSE);
		}
	}
	SetFocus();
}


void CNoteMapWnd::OnLButtonDblClk(UINT, CPoint)
//---------------------------------------------
{
	// Double-click edits sample map
	OnEditSampleMap();
}


void CNoteMapWnd::OnRButtonDown(UINT, CPoint pt)
//----------------------------------------------
{
	if (m_pModDoc)
	{
		CHAR s[64];
		CSoundFile *pSndFile;
		INSTRUMENTHEADER *penv;
		
		pSndFile = m_pModDoc->GetSoundFile();
		penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			HMENU hMenu = ::CreatePopupMenu();
			HMENU hSubMenu = ::CreatePopupMenu();

			if (hMenu)
			{
				AppendMenu(hMenu, MF_STRING, ID_INSTRUMENT_SAMPLEMAP, "Edit Sample Map");
				if (hSubMenu)
				{
					BYTE smpused[(MAX_SAMPLES+7)/8];
					memset(smpused, 0, sizeof(smpused));
					for (UINT i=1; i<120; i++)
					{
						UINT nsmp = penv->Keyboard[i];
						if (nsmp < MAX_SAMPLES) smpused[nsmp>>3] |= 1 << (nsmp & 7);
					}
					for (UINT j=1; j<MAX_SAMPLES; j++)
					{
						if (smpused[j>>3] & (1 << (j & 7)))
						{
							wsprintf(s, "%d: ", j);
							UINT l = strlen(s);
							memcpy(s+l, pSndFile->m_szNames[j], 32);
							s[l+32] = 0;
							AppendMenu(hSubMenu, MF_STRING, ID_NOTEMAP_EDITSAMPLE+j, s);
						}
					}
					AppendMenu(hMenu, MF_POPUP, (UINT)hSubMenu, "Edit Sample");
					AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				}
				wsprintf(s, "Map all notes to sample %d", penv->Keyboard[m_nNote]);
				AppendMenu(hMenu, MF_STRING, ID_NOTEMAP_COPY, s);
				AppendMenu(hMenu, MF_STRING, ID_NOTEMAP_RESET, "Reset note mapping");
				AppendMenu(hMenu, MF_STRING, ID_INSTRUMENT_DUPLICATE, "Duplicate Instrument\tShift+New");
				SetMenuDefaultItem(hMenu, ID_INSTRUMENT_SAMPLEMAP, FALSE);
				ClientToScreen(&pt);
				::TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
				::DestroyMenu(hMenu);
				if (hSubMenu) ::DestroyMenu(hSubMenu);
			}
		}
	}
}


void CNoteMapWnd::OnMapCopy()
//---------------------------
{
	if (m_pModDoc)
	{
		CSoundFile *pSndFile;
		INSTRUMENTHEADER *penv;
		
		pSndFile = m_pModDoc->GetSoundFile();
		penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			BOOL bModified = FALSE;
			UINT n = penv->Keyboard[m_nNote];
			for (UINT i=0; i<120; i++) if (penv->Keyboard[i] != n)
			{
				penv->Keyboard[i] = n;
				bModified = TRUE;
			}
			if (bModified)
			{
				m_pModDoc->SetModified();
				InvalidateRect(NULL, FALSE);
			}
		}
	}
}


void CNoteMapWnd::OnMapReset()
//----------------------------
{
	if (m_pModDoc)
	{
		CSoundFile *pSndFile;
		INSTRUMENTHEADER *penv;
		
		pSndFile = m_pModDoc->GetSoundFile();
		penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			BOOL bModified = FALSE;
			for (UINT i=0; i<120; i++) if (penv->NoteMap[i] != i+1)
			{
				penv->NoteMap[i] = i+1;
				bModified = TRUE;
			}
			if (bModified)
			{
				m_pModDoc->SetModified();
				InvalidateRect(NULL, FALSE);
			}
		}
	}
}


void CNoteMapWnd::OnEditSample(UINT nID)
//--------------------------------------
{
	UINT nSample = nID - ID_NOTEMAP_EDITSAMPLE;
	if (m_pParent) m_pParent->EditSample(nSample);
}


void CNoteMapWnd::OnEditSampleMap()
//---------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_INSTRUMENT_SAMPLEMAP);
}


void CNoteMapWnd::OnInstrumentDuplicate()
//---------------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_INSTRUMENT_DUPLICATE);
}



/////////////////////////////////////////////////////////////////////////
// CCtrlInstruments

BEGIN_MESSAGE_MAP(CCtrlInstruments, CModControlDlg)
	//{{AFX_MSG_MAP(CCtrlInstruments)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(IDC_INSTRUMENT_NEW,		OnInstrumentNew)
	ON_COMMAND(IDC_INSTRUMENT_OPEN,		OnInstrumentOpen)
	ON_COMMAND(IDC_INSTRUMENT_SAVEAS,	OnInstrumentSave)
	ON_COMMAND(IDC_INSTRUMENT_PLAY,		OnInstrumentPlay)
	ON_COMMAND(ID_PREVINSTRUMENT,		OnPrevInstrument)
	ON_COMMAND(ID_NEXTINSTRUMENT,		OnNextInstrument)
	ON_COMMAND(ID_INSTRUMENT_DUPLICATE, OnInstrumentDuplicate)
	ON_COMMAND(IDC_CHECK1,				OnSetPanningChanged)
	ON_COMMAND(IDC_CHECK2,				OnEnableCutOff)
	ON_COMMAND(IDC_CHECK3,				OnEnableResonance)
	ON_EN_CHANGE(IDC_EDIT_INSTRUMENT,	OnInstrumentChanged)
	ON_EN_CHANGE(IDC_SAMPLE_NAME,		OnNameChanged)
	ON_EN_CHANGE(IDC_SAMPLE_FILENAME,	OnFileNameChanged)
	ON_EN_CHANGE(IDC_EDIT7,				OnFadeOutVolChanged)
	ON_EN_CHANGE(IDC_EDIT8,				OnGlobalVolChanged)
	ON_EN_CHANGE(IDC_EDIT9,				OnPanningChanged)
	ON_EN_CHANGE(IDC_EDIT10,			OnMPRChanged)
	ON_EN_CHANGE(IDC_EDIT15,			OnPPSChanged)
	ON_CBN_SELCHANGE(IDC_COMBO1,		OnNNAChanged)
	ON_CBN_SELCHANGE(IDC_COMBO2,		OnDCTChanged)
	ON_CBN_SELCHANGE(IDC_COMBO3,		OnDCAChanged)
	ON_CBN_SELCHANGE(IDC_COMBO4,		OnPPCChanged)
	ON_CBN_SELCHANGE(IDC_COMBO5,		OnMCHChanged)
	ON_COMMAND(ID_INSTRUMENT_SAMPLEMAP, OnEditSampleMap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCtrlInstruments::DoDataExchange(CDataExchange* pDX)
//-------------------------------------------------------
{
	CModControlDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCtrlInstruments)
	DDX_Control(pDX, IDC_TOOLBAR1,				m_ToolBar);
	DDX_Control(pDX, IDC_NOTEMAP,				m_NoteMap);
	DDX_Control(pDX, IDC_SAMPLE_NAME,			m_EditName);
	DDX_Control(pDX, IDC_SAMPLE_FILENAME,		m_EditFileName);
	DDX_Control(pDX, IDC_SPIN_INSTRUMENT,		m_SpinInstrument);
	DDX_Control(pDX, IDC_COMBO1,				m_ComboNNA);
	DDX_Control(pDX, IDC_COMBO2,				m_ComboDCT);
	DDX_Control(pDX, IDC_COMBO3,				m_ComboDCA);
	DDX_Control(pDX, IDC_COMBO4,				m_ComboPPC);
	DDX_Control(pDX, IDC_COMBO5,				m_CbnMidiCh);
	DDX_Control(pDX, IDC_SPIN7,					m_SpinFadeOut);
	DDX_Control(pDX, IDC_SPIN8,					m_SpinGlobalVol);
	DDX_Control(pDX, IDC_SPIN9,					m_SpinPanning);
	DDX_Control(pDX, IDC_SPIN10,				m_SpinMidiPR);
	DDX_Control(pDX, IDC_SPIN12,				m_SpinPPS);
	DDX_Control(pDX, IDC_EDIT1,					m_EditCutOff);
	DDX_Control(pDX, IDC_EDIT8,					m_EditGlobalVol);
	DDX_Control(pDX, IDC_EDIT9,					m_EditPanning);
	DDX_Control(pDX, IDC_EDIT15,				m_EditPPS);
	DDX_Control(pDX, IDC_CHECK1,				m_CheckPanning);
	DDX_Control(pDX, IDC_CHECK2,				m_CheckCutOff);
	DDX_Control(pDX, IDC_CHECK3,				m_CheckResonance);
	DDX_Control(pDX, IDC_SLIDER1,				m_SliderVolSwing);
	DDX_Control(pDX, IDC_SLIDER2,				m_SliderPanSwing);
	DDX_Control(pDX, IDC_SLIDER3,				m_SliderCutOff);
	DDX_Control(pDX, IDC_SLIDER4,				m_SliderResonance);
	//}}AFX_DATA_MAP
}


CCtrlInstruments::CCtrlInstruments()
//----------------------------------
{
	m_nInstrument = 1;
	m_nLockCount = 1;
}


CCtrlInstruments::~CCtrlInstruments()
//-----------------------------------
{
}


CRuntimeClass *CCtrlInstruments::GetAssociatedViewClass()
//-------------------------------------------------------
{
	return RUNTIME_CLASS(CViewInstrument);
}


BOOL CCtrlInstruments::OnInitDialog()
//-----------------------------------
{
	CHAR s[64];
	CModControlDlg::OnInitDialog();
	m_bInitialized = FALSE;
	if ((!m_pModDoc) || (!m_pSndFile)) return TRUE;
	m_NoteMap.Init(this);
	m_ToolBar.Init();
	m_ToolBar.AddButton(IDC_INSTRUMENT_NEW, 7);
	m_ToolBar.AddButton(IDC_INSTRUMENT_OPEN, 12);
	m_ToolBar.AddButton(IDC_INSTRUMENT_SAVEAS, 13);
	m_ToolBar.AddButton(IDC_INSTRUMENT_PLAY, 14);
	m_SpinInstrument.SetRange(0, 0);
	m_SpinInstrument.EnableWindow(FALSE);
	m_EditName.SetLimitText(32);
	m_EditFileName.SetLimitText(20);
	// NNA
	m_ComboNNA.AddString("Note Cut");
	m_ComboNNA.AddString("Continue");
	m_ComboNNA.AddString("Note Off");
	m_ComboNNA.AddString("Note Fade");
	// DCT
	m_ComboDCT.AddString("Disabled");
	m_ComboDCT.AddString("Note");
	m_ComboDCT.AddString("Sample");
	m_ComboDCT.AddString("Instrument");
	// DCA
	m_ComboDCA.AddString("Note Cut");
	m_ComboDCA.AddString("Note Off");
	m_ComboDCA.AddString("Note Fade");
	// FadeOut Volume
	m_SpinFadeOut.SetRange(0, 32000);
	// Global Volume
	m_SpinGlobalVol.SetRange(0, 64);
	// Panning
	m_SpinPanning.SetRange(0, 256);
	// Midi Program
	m_SpinMidiPR.SetRange(0, 255);
	// Midi Channel
	for (UINT ich=0; ich<1+16+MAX_MIXPLUGINS; ich++)
	{
		UINT n = 0;
		s[0] = 0;
		if (!ich) { strcpy(s, "Default"); n=0; } else
		if ((ich >= 1) && (ich <= 16)) { wsprintf(s, "midi ch%d", ich); n=ich; } else
		{ wsprintf(s, "FX%d send", ich-16); n=(ich-16) | 0x80; }
		if (s[0]) m_CbnMidiCh.SetItemData(m_CbnMidiCh.AddString(s), n);
	}
	// Vol/Pan Swing
	m_SliderVolSwing.SetRange(0, 64);
	m_SliderPanSwing.SetRange(0, 64);
	// Filter
	m_SliderCutOff.SetRange(0x00, 0x7F);
	m_SliderResonance.SetRange(0x00, 0x7F);
	// Pitch/Pan Separation
	m_SpinPPS.SetRange(-32, +32);
	// Pitch/Pan Center
	for (UINT n=0; n<=120; n++)
	{
		wsprintf(s, "%s%d", szNoteNames[n % 12], n/12);
		m_ComboPPC.SetItemData(m_ComboPPC.AddString(s), n);
	}
	m_SpinInstrument.SetFocus();
	return FALSE;
}


void CCtrlInstruments::RecalcLayout()
//-----------------------------------
{
}


BOOL CCtrlInstruments::SetCurrentInstrument(UINT nIns, BOOL bUpdNum)
//------------------------------------------------------------------
{
	if ((!m_pModDoc) || (!m_pSndFile)) return FALSE;
	if (m_pSndFile->m_nInstruments < 1) return FALSE;
	if ((nIns < 1) || (nIns > m_pSndFile->m_nInstruments)) return FALSE;
	LockControls();
	if ((m_nInstrument != nIns) || (!m_bInitialized))
	{
		m_nInstrument = nIns;
		m_NoteMap.SetCurrentInstrument(m_pModDoc, m_nInstrument);
		UpdateView((m_nInstrument << 24) | HINT_INSTRUMENT | HINT_ENVELOPE, NULL);
	} else
	{
		// Just in case
		m_NoteMap.SetCurrentInstrument(m_pModDoc, m_nInstrument);
	}
	if (bUpdNum)
	{
		SetDlgItemInt(IDC_EDIT_INSTRUMENT, m_nInstrument);
		m_SpinInstrument.SetRange(1, m_pSndFile->m_nInstruments);
		m_SpinInstrument.EnableWindow((m_pSndFile->m_nInstruments) ? TRUE : FALSE);
		// Is this a bug ?
		m_SliderCutOff.InvalidateRect(NULL, FALSE);
		m_SliderResonance.InvalidateRect(NULL, FALSE);
	}
	PostViewMessage(VIEWMSG_SETCURRENTINSTRUMENT, m_nInstrument);
	UnlockControls();
	return TRUE;
}


void CCtrlInstruments::OnActivatePage(LPARAM lParam)
//--------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((pModDoc) && (m_pParent))
	{
		if (lParam < 0)
		{
			int nIns = m_pParent->GetInstrumentChange();
			if (nIns > 0) lParam = nIns;
			m_pParent->InstrumentChanged(-1);
		} else
		if (lParam > 0)
		{
			m_pParent->InstrumentChanged(lParam);
		}
	}
	SetCurrentInstrument((lParam > 0) ? lParam : m_nInstrument);
	// Initial Update
	if (!m_bInitialized) UpdateView((m_nInstrument << 24) | HINT_INSTRUMENT | HINT_ENVELOPE | HINT_MODTYPE, NULL);
	CChildFrame *pFrame = (CChildFrame *)GetParentFrame();
	if (pFrame) PostViewMessage(VIEWMSG_LOADSTATE, (LPARAM)pFrame->GetInstrumentViewState());
	SwitchToView();
}


void CCtrlInstruments::OnDeactivatePage()
//---------------------------------------
{
	if (m_pModDoc) m_pModDoc->NoteOff(0, TRUE);
	CChildFrame *pFrame = (CChildFrame *)GetParentFrame();
	if ((pFrame) && (m_hWndView)) SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)pFrame->GetInstrumentViewState());
}


LRESULT CCtrlInstruments::OnModCtrlMsg(WPARAM wParam, LPARAM lParam)
//------------------------------------------------------------------
{
	switch(wParam)
	{
	case CTRLMSG_INS_PREVINSTRUMENT:
		OnPrevInstrument();
		break;

	case CTRLMSG_INS_NEXTINSTRUMENT:
		OnNextInstrument();
		break;

	case CTRLMSG_INS_OPENFILE:
		if (lParam) return OpenInstrument((LPCSTR)lParam);
		break;

	case CTRLMSG_INS_SONGDROP:
		if (lParam)
		{
			LPDRAGONDROP pDropInfo = (LPDRAGONDROP)lParam;
			CSoundFile *pSndFile = (CSoundFile *)(pDropInfo->lDropParam);
			if (pDropInfo->pModDoc) pSndFile = pDropInfo->pModDoc->GetSoundFile();
			if (pSndFile) return OpenInstrument(pSndFile, pDropInfo->dwDropItem);
		}
		break;

	case CTRLMSG_INS_NEWINSTRUMENT:
		OnInstrumentNew();
		break;

	case CTRLMSG_SETCURRENTINSTRUMENT:
		SetCurrentInstrument(lParam);
		break;

	case CTRLMSG_INS_SAMPLEMAP:
		OnEditSampleMap();
		break;

	default:
		return CModControlDlg::OnModCtrlMsg(wParam, lParam);
	}
	return 0;
}


void CCtrlInstruments::UpdateView(DWORD dwHintMask, CObject *pObj)
//----------------------------------------------------------------
{
	if ((pObj == this) || (!m_pModDoc) || (!m_pSndFile)) return;
	if (dwHintMask & HINT_MPTOPTIONS)
	{
		m_ToolBar.UpdateStyle();
	}
	if (!(dwHintMask & (HINT_INSTRUMENT|HINT_ENVELOPE|HINT_MODTYPE))) return;
	if (((dwHintMask >> 24) != m_nInstrument) && (dwHintMask & (HINT_INSTRUMENT|HINT_ENVELOPE)) && (!(dwHintMask & HINT_MODTYPE))) return;
	LockControls();
	if (!m_bInitialized) dwHintMask |= HINT_MODTYPE;
	if (dwHintMask & HINT_MODTYPE)
	{
		BOOL b = ((m_pSndFile->m_nType == MOD_TYPE_IT) && (m_pSndFile->m_nInstruments)) ? TRUE : FALSE;
		m_ComboNNA.EnableWindow(b);
		m_ComboDCT.EnableWindow(b);
		m_ComboDCA.EnableWindow(b);
		m_ComboPPC.EnableWindow(b);
		m_SpinPPS.EnableWindow(b);
		m_EditGlobalVol.EnableWindow(b);
		m_SpinGlobalVol.EnableWindow(b);
		m_EditPanning.EnableWindow(b);
		m_SpinPanning.EnableWindow(b);
		m_CheckPanning.EnableWindow(b);
		m_EditPPS.EnableWindow(b);
		m_EditCutOff.EnableWindow(b);
		m_CheckCutOff.EnableWindow(b);
		m_CheckResonance.EnableWindow(b);
		m_SliderVolSwing.EnableWindow(b);
		m_SliderPanSwing.EnableWindow(b);
		m_SliderCutOff.EnableWindow(b);
		m_SliderResonance.EnableWindow(b);
		m_SpinInstrument.SetRange(1, m_pSndFile->m_nInstruments);
		m_SpinInstrument.EnableWindow((m_pSndFile->m_nInstruments) ? TRUE : FALSE);
	}
	if (dwHintMask & (HINT_INSTRUMENT|HINT_MODTYPE))
	{
		CHAR s[128];
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			memcpy(s, penv->name, 32);
			s[32] = 0;
			m_EditName.SetWindowText(s);
			memcpy(s, penv->filename, 12);
			s[12] = 0;
			m_EditFileName.SetWindowText(s);
			// Fade Out Volume
			SetDlgItemInt(IDC_EDIT7, penv->nFadeOut);
			// Global Volume
			SetDlgItemInt(IDC_EDIT8, penv->nGlobalVol);
			// Panning
			SetDlgItemInt(IDC_EDIT9, penv->nPan);
			m_CheckPanning.SetCheck((penv->dwFlags & ENV_SETPANNING) ? TRUE : FALSE);
			// Midi
			SetDlgItemInt(IDC_EDIT10, penv->nMidiProgram);
			if (penv->nMidiChannel < 17) m_CbnMidiCh.SetCurSel(penv->nMidiChannel); else
			if (penv->nMidiChannel & 0x80) m_CbnMidiCh.SetCurSel((penv->nMidiChannel&0x7f)+16); else
				m_CbnMidiCh.SetCurSel(0);
			// NNA, DCT, DCA
			m_ComboNNA.SetCurSel(penv->nNNA);
			m_ComboDCT.SetCurSel(penv->nDCT);
			m_ComboDCA.SetCurSel(penv->nDNA);
			// Pitch/Pan Separation
			m_ComboPPC.SetCurSel(penv->nPPC);
			SetDlgItemInt(IDC_EDIT15, penv->nPPS);
			// Filter
			if (m_pSndFile->m_nType & MOD_TYPE_IT)
			{
				m_CheckCutOff.SetCheck((penv->nIFC & 0x80) ? TRUE : FALSE);
				m_CheckResonance.SetCheck((penv->nIFR & 0x80) ? TRUE : FALSE);
				m_SliderVolSwing.SetPos(penv->nVolSwing);
				m_SliderPanSwing.SetPos(penv->nPanSwing);
				m_SliderCutOff.SetPos(penv->nIFC & 0x7F);
				m_SliderResonance.SetPos(penv->nIFR & 0x7F);
				UpdateFilterText();
			}
		} else
		{
			m_EditName.SetWindowText("");
			m_EditFileName.SetWindowText("");
		}
		m_NoteMap.InvalidateRect(NULL, FALSE);
	}
	if (!m_bInitialized)
	{
		// First update
		m_bInitialized = TRUE;
		UnlockControls();
	}
	UnlockControls();
}


VOID CCtrlInstruments::UpdateFilterText()
//---------------------------------------
{
	if ((m_nInstrument) && (m_pModDoc))
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			CHAR s[64];
			wsprintf(s, "%d Hz", pSndFile->CutOffToFrequency(penv->nIFC & 0x7F));
			m_EditCutOff.SetWindowText(s);
		}
	}
}


BOOL CCtrlInstruments::OpenInstrument(LPCSTR lpszFileName)
//--------------------------------------------------------
{
	CHAR szName[_MAX_FNAME], szExt[_MAX_EXT];
	CMappedFile f;
	BOOL bFirst, bOk;
	DWORD len;
	LPBYTE lpFile;
	
	BeginWaitCursor();
	if ((!lpszFileName) || (!f.Open(lpszFileName)))
	{
		EndWaitCursor();
		return FALSE;
	}
	bFirst = FALSE;
	len = f.GetLength();
	if (len > CTrackApp::gMemStatus.dwTotalPhys) len = CTrackApp::gMemStatus.dwTotalPhys;
	lpFile = f.Lock(len);
	bOk = FALSE;
	if (!lpFile) goto OpenError;
	BEGIN_CRITICAL();
	if (!m_pSndFile->m_nInstruments)
	{
		bFirst = TRUE;
		m_pSndFile->m_nInstruments = 1;
		m_NoteMap.SetCurrentInstrument(m_pModDoc, 1);
		m_pModDoc->SetModified();
	}
	if (!m_nInstrument) m_nInstrument = 1;
	if (m_pSndFile->ReadInstrumentFromFile(m_nInstrument, lpFile, len))
	{
		bOk = TRUE;
	}
	END_CRITICAL();
	f.Unlock();
OpenError:
	f.Close();
	EndWaitCursor();
	if (bOk)
	{
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			CHAR szPath[_MAX_PATH], szNewPath[_MAX_PATH];
			_splitpath(lpszFileName, szPath, szNewPath, szName, szExt);
			strcat(szNewPath, szPath);
			strcpy(CMainFrame::m_szCurInsDir, szNewPath);
			if (!penv->name[0])
			{
				szName[31] = 0;
				memset(penv->name, 0, 32);
				strcpy(penv->name, szName);
			}
			if (!penv->filename[0])
			{
				strcat(szName, szExt);
				szName[11] = 0;
				strcpy(penv->filename, szName);
				penv->filename[11] = 0;
			}
			SetCurrentInstrument(m_nInstrument);
			if (m_pModDoc)
			{
				m_pModDoc->SetModified();
				m_pModDoc->UpdateAllViews(NULL, (m_nInstrument << 24) | HINT_INSTRUMENT | HINT_ENVELOPE | HINT_INSNAMES | HINT_SMPNAMES);
			}
		} else bOk = FALSE;
	}
	if (bFirst) m_pModDoc->UpdateAllViews(NULL, HINT_MODTYPE | HINT_INSNAMES | HINT_SMPNAMES);
	if (!bOk) ErrorBox(IDS_ERR_FILETYPE, this);
	return TRUE;
}


BOOL CCtrlInstruments::OpenInstrument(CSoundFile *pSndFile, UINT nInstr)
//----------------------------------------------------------------------
{
	if ((!pSndFile) || (!nInstr) || (nInstr > pSndFile->m_nInstruments)) return FALSE;
	BeginWaitCursor();
	BEGIN_CRITICAL();
	BOOL bFirst = FALSE;
	if (!m_pSndFile->m_nInstruments)
	{
		bFirst = TRUE;
		m_pSndFile->m_nInstruments = 1;
		m_NoteMap.SetCurrentInstrument(m_pModDoc, 1);
		m_pModDoc->SetModified();
		bFirst = TRUE;
	}
	if (!m_nInstrument)
	{
		m_nInstrument = 1;
		bFirst = TRUE;
	}
	m_pSndFile->ReadInstrumentFromSong(m_nInstrument, pSndFile, nInstr);
	END_CRITICAL();
	m_pModDoc->SetModified();
	m_pModDoc->UpdateAllViews(NULL, (m_nInstrument << 24) | HINT_INSTRUMENT | HINT_ENVELOPE | HINT_INSNAMES | HINT_SMPNAMES);
	if (bFirst) m_pModDoc->UpdateAllViews(NULL, HINT_MODTYPE | HINT_INSNAMES | HINT_SMPNAMES);
	m_pModDoc->SetModified();
	EndWaitCursor();
	return TRUE;
}


BOOL CCtrlInstruments::EditSample(UINT nSample)
//---------------------------------------------
{
	if ((nSample > 0) && (nSample < MAX_SAMPLES))
	{
		if (m_pParent)
		{
			m_pParent->PostMessage(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_SAMPLES, nSample);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CCtrlInstruments::GetToolTipText(UINT uId, LPSTR pszText)
//------------------------------------------------------------
{
	if ((pszText) && (uId))
	{
		switch(uId)
		{
		case IDC_EDIT1:
			if ((m_pSndFile) && (m_pSndFile->Headers[m_nInstrument]))
			{
				INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
				wsprintf(pszText, "Z%02X", penv->nIFC & 0x7f);
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// CCtrlInstruments Messages

void CCtrlInstruments::OnInstrumentChanged()
//------------------------------------------
{
	if ((!IsLocked()) && (m_pSndFile))
	{
		UINT n = GetDlgItemInt(IDC_EDIT_INSTRUMENT);
		if ((n > 0) && (n <= m_pSndFile->m_nInstruments) && (n != m_nInstrument))
		{
			SetCurrentInstrument(n, FALSE);
			if (m_pParent) m_pParent->InstrumentChanged(n);
		}
	}
}


void CCtrlInstruments::OnPrevInstrument()
//---------------------------------------
{
	if (m_pSndFile)
	{
		if (m_nInstrument > 1)
			SetCurrentInstrument(m_nInstrument-1);
		else
			SetCurrentInstrument(m_pSndFile->m_nInstruments);
		if (m_pParent) m_pParent->InstrumentChanged(m_nInstrument);
	}
}


void CCtrlInstruments::OnNextInstrument()
//---------------------------------------
{
	if (m_pSndFile)
	{
		if (m_nInstrument < m_pSndFile->m_nInstruments)
			SetCurrentInstrument(m_nInstrument+1);
		else
			SetCurrentInstrument(1);
		if (m_pParent) m_pParent->InstrumentChanged(m_nInstrument);
	}
}


void CCtrlInstruments::OnInstrumentNew()
//--------------------------------------
{
	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		if ((pSndFile->m_nType & MOD_TYPE_IT)
		 && (pSndFile->m_nInstruments > 0)
		 && (CMainFrame::gnHotKeyMask & HOTKEYF_SHIFT))
		{
			OnInstrumentDuplicate();
			return;
		}
		BOOL bFirst = (pSndFile->m_nInstruments) ? FALSE : TRUE;
		LONG smp = m_pModDoc->InsertInstrument(0);
		if (smp > 0)
		{
			SetCurrentInstrument(smp);
			m_pModDoc->UpdateAllViews(NULL, (smp << 24) | HINT_INSTRUMENT | HINT_INSNAMES | HINT_ENVELOPE);
		}
		if (bFirst) m_pModDoc->UpdateAllViews(NULL, (smp << 24) | HINT_MODTYPE | HINT_INSTRUMENT | HINT_INSNAMES);
		if (m_pParent) m_pParent->InstrumentChanged(m_nInstrument);
	}
	SwitchToView();
}


void CCtrlInstruments::OnInstrumentDuplicate()
//--------------------------------------------
{
	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		if ((pSndFile->m_nType & MOD_TYPE_IT) && (pSndFile->m_nInstruments > 0))
		{
			BOOL bFirst = (pSndFile->m_nInstruments) ? FALSE : TRUE;
			LONG smp = m_pModDoc->InsertInstrument(0, m_nInstrument);
			if (smp > 0)
			{
				SetCurrentInstrument(smp);
				m_pModDoc->UpdateAllViews(NULL, (smp << 24) | HINT_INSTRUMENT | HINT_INSNAMES | HINT_ENVELOPE);
			}
			if (bFirst) m_pModDoc->UpdateAllViews(NULL, (smp << 24) | HINT_MODTYPE | HINT_INSTRUMENT | HINT_INSNAMES);
			if (m_pParent) m_pParent->InstrumentChanged(m_nInstrument);
		}
	}
	SwitchToView();
}


void CCtrlInstruments::OnInstrumentOpen()
//---------------------------------------
{
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
					"All Instruments|*.xi;*.pat;*.iti;*.wav;*.aif;*.aiff|"
					"FastTracker II Instruments (*.xi)|*.xi|"
					"GF1 Patches (*.pat)|*.pat|"
					"Impulse Tracker Instruments (*.iti)|*.iti|"
					"All Files (*.*)|*.*||",
					this);
	if (CMainFrame::m_szCurInsDir[0])
	{
		dlg.m_ofn.lpstrInitialDir = CMainFrame::m_szCurInsDir;
	}
	if (dlg.DoModal() != IDOK) return;
	if (!OpenInstrument(dlg.GetPathName())) ErrorBox(IDS_ERR_FILEOPEN, this);
	if (m_pParent) m_pParent->InstrumentChanged(m_nInstrument);
	SwitchToView();
}


void CCtrlInstruments::OnInstrumentSave()
//---------------------------------------
{
	CHAR szFileName[_MAX_PATH] = "", drive[_MAX_DRIVE], path[_MAX_PATH], ext[_MAX_EXT];
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	
	if (!penv) return;
	if (penv->filename[0])
	{
		memcpy(szFileName, penv->filename, 12);
		szFileName[12] = 0;
	} else
	{
		memcpy(szFileName, penv->name, 22);
		szFileName[22] = 0;
	}
	CFileDialog dlg(FALSE, (m_pSndFile->m_nType & MOD_TYPE_IT) ? "iti" : "xi",
			szFileName,
			OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
			"FastTracker II Instruments (*.xi)|*.xi|"
			"Impulse Tracker Instruments (*.iti)|*.iti||",
			this);
	if (CMainFrame::m_szCurInsDir[0])
	{
		dlg.m_ofn.lpstrInitialDir = CMainFrame::m_szCurInsDir;
	}
	if (dlg.DoModal() != IDOK) return;
	BeginWaitCursor();
	_splitpath(dlg.GetPathName(), drive, path, NULL, ext);
	BOOL bOk = FALSE;
	if (!lstrcmpi(ext, ".iti"))
		bOk = m_pSndFile->SaveITIInstrument(m_nInstrument, dlg.GetPathName());
	else
		bOk = m_pSndFile->SaveXIInstrument(m_nInstrument, dlg.GetPathName());
	EndWaitCursor();
	if (!bOk) ErrorBox(IDS_ERR_SAVEINS, this); else
	{
		strcpy(CMainFrame::m_szCurInsDir, drive);
		strcat(CMainFrame::m_szCurInsDir, path);
		m_pModDoc->UpdateAllViews(NULL, (m_nInstrument << 24) | HINT_INSTRUMENT);
	}
	SwitchToView();
}


void CCtrlInstruments::OnInstrumentPlay()
//---------------------------------------
{
	if ((m_pModDoc) && (m_pSndFile))
	{
		if (m_pModDoc->IsNotePlaying(NOTE_MIDDLEC, 0, m_nInstrument))
		{
			m_pModDoc->NoteOff(NOTE_MIDDLEC, TRUE);
		} else
		{
			m_pModDoc->PlayNote(NOTE_MIDDLEC, m_nInstrument, 0, TRUE);
		}
	}
	SwitchToView();
}


void CCtrlInstruments::OnNameChanged()
//------------------------------------
{
	if (!IsLocked())
	{
		CHAR s[64];
		s[0] = 0;
		m_EditName.GetWindowText(s, sizeof(s));
		for (UINT i=strlen(s); i<=32; i++) s[i] = 0;
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if ((penv) && (strncmp(s, penv->name, 32)))
		{
			memcpy(penv->name, s, 32);
			m_pModDoc->SetModified();
			m_pModDoc->UpdateAllViews(NULL, (m_nInstrument << 24) | HINT_INSNAMES, this);
		}
	}
}


void CCtrlInstruments::OnFileNameChanged()
//----------------------------------------
{
	if (!IsLocked())
	{
		CHAR s[64];
		s[0] = 0;
		m_EditFileName.GetWindowText(s, sizeof(s));
		for (UINT i=strlen(s); i<=12; i++) s[i] = 0;
		INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
		if ((penv) && (strncmp(s, penv->filename, 12)))
		{
			memcpy(penv->filename, s, 12);
			m_pModDoc->SetModified();
		}
	}
}


void CCtrlInstruments::OnFadeOutVolChanged()
//------------------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int nVol = GetDlgItemInt(IDC_EDIT7);
		if (nVol < 0) nVol = 0;
		if (nVol > 16384) nVol = 16384;
		if (nVol != (int)penv->nFadeOut)
		{
			penv->nFadeOut = nVol;
			m_pModDoc->SetModified();
		}
	}
}


void CCtrlInstruments::OnGlobalVolChanged()
//-----------------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int nVol = GetDlgItemInt(IDC_EDIT8);
		if (nVol < 0) nVol = 0;
		if (nVol > 64) nVol = 64;
		if (nVol != (int)penv->nGlobalVol)
		{
			penv->nGlobalVol = nVol;
			if (m_pSndFile->m_nType == MOD_TYPE_IT) m_pModDoc->SetModified();
		}
	}
}


void CCtrlInstruments::OnSetPanningChanged()
//------------------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		BOOL b = m_CheckPanning.GetCheck();
		if (b) penv->dwFlags |= ENV_SETPANNING;
		else penv->dwFlags &= ~ENV_SETPANNING;
	}
}


void CCtrlInstruments::OnPanningChanged()
//---------------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int nPan = GetDlgItemInt(IDC_EDIT9);
		if (nPan < 0) nPan = 0;
		if (nPan > 256) nPan = 256;
		if (nPan != (int)penv->nPan)
		{
			penv->nPan = nPan;
			if (m_pSndFile->m_nType == MOD_TYPE_IT) m_pModDoc->SetModified();
		}
	}
}


void CCtrlInstruments::OnNNAChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		penv->nNNA = m_ComboNNA.GetCurSel();
	}
}
	
	
void CCtrlInstruments::OnDCTChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		penv->nDCT = m_ComboDCT.GetCurSel();
	}
}
	

void CCtrlInstruments::OnDCAChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		penv->nDNA = m_ComboDCA.GetCurSel();
	}
}


void CCtrlInstruments::OnMPRChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int n = GetDlgItemInt(IDC_EDIT10);
		if ((n >= 0) && (n <= 255)) penv->nMidiProgram = n;
	}
}


void CCtrlInstruments::OnMCHChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int n = m_CbnMidiCh.GetItemData(m_CbnMidiCh.GetCurSel());
		penv->nMidiChannel = (BYTE)(n & 0xff);
	}
}


void CCtrlInstruments::OnPPSChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int n = GetDlgItemInt(IDC_EDIT15);
		if ((n >= -32) && (n <= 32)) penv->nPPS = (signed char)n;
	}
}


void CCtrlInstruments::OnPPCChanged()
//-----------------------------------
{
	INSTRUMENTHEADER *penv = m_pSndFile->Headers[m_nInstrument];
	if ((!IsLocked()) && (penv))
	{
		int n = m_ComboPPC.GetCurSel();
		if ((n >= 0) && (n <= 119)) penv->nPPC = n;
	}
}


void CCtrlInstruments::OnEnableCutOff()
//-------------------------------------
{
	BOOL bCutOff = IsDlgButtonChecked(IDC_CHECK2);

	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			if (bCutOff)
			{
				penv->nIFC |= 0x80;
			} else
			{
				penv->nIFC &= 0x7F;
			}
			for (UINT i=0; i<MAX_CHANNELS; i++)
			{
				if (pSndFile->Chn[i].pHeader == penv)
				{
					if (bCutOff)
					{
						pSndFile->Chn[i].nCutOff = penv->nIFC & 0x7f;
					} else
					{
						pSndFile->Chn[i].nCutOff = 0x7f;
					}
				}
			}
		}
		m_pModDoc->SetModified();
		SwitchToView();
	}
}


void CCtrlInstruments::OnEnableResonance()
//----------------------------------------
{
	BOOL bReso = IsDlgButtonChecked(IDC_CHECK3);

	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			if (bReso)
			{
				penv->nIFR |= 0x80;
			} else
			{
				penv->nIFR &= 0x7F;
			}
			for (UINT i=0; i<MAX_CHANNELS; i++)
			{
				if (pSndFile->Chn[i].pHeader == penv)
				{
					if (bReso)
					{
						pSndFile->Chn[i].nResonance = penv->nIFC & 0x7f;
					} else
					{
						pSndFile->Chn[i].nResonance = 0;
					}
				}
			}
		}
		m_pModDoc->SetModified();
		SwitchToView();
	}
}


void CCtrlInstruments::OnVScroll(UINT nCode, UINT nPos, CScrollBar *pSB)
//----------------------------------------------------------------------
{
	CModControlDlg::OnVScroll(nCode, nPos, pSB);
	if (nCode == SB_ENDSCROLL) SwitchToView();
}


void CCtrlInstruments::OnHScroll(UINT nCode, UINT nPos, CScrollBar *pSB)
//----------------------------------------------------------------------
{
	CModControlDlg::OnHScroll(nCode, nPos, pSB);
	if ((m_nInstrument) && (m_pModDoc) && (!IsLocked()) && (nCode != SB_ENDSCROLL))
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];

		if (penv)
		{
			short int n;

			// Volume Swing
			n = m_SliderVolSwing.GetPos();
			if ((n >= 0) && (n <= 64) && (n != (int)penv->nVolSwing))
			{
				penv->nVolSwing = (BYTE)n;
				m_pModDoc->SetModified();
			}
			// Pan Swing
			n = m_SliderPanSwing.GetPos();
			if ((n >= 0) && (n <= 64) && (n != (int)penv->nPanSwing))
			{
				penv->nPanSwing = (BYTE)n;
				m_pModDoc->SetModified();
			}
			// Filter CutOff
			n = m_SliderCutOff.GetPos();
			if ((n >= 0) && (n < 0x80) && (n != (int)(penv->nIFC & 0x7F)))
			{
				penv->nIFC &= 0x80;
				penv->nIFC |= (BYTE)n;
				m_pModDoc->SetModified();
				UpdateFilterText();
			}
			// Filter Resonance
			n = m_SliderResonance.GetPos();
			if ((n >= 0) && (n < 0x80) && (n != (int)(penv->nIFR & 0x7F)))
			{
				penv->nIFR &= 0x80;
				penv->nIFR |= (BYTE)n;
				m_pModDoc->SetModified();
			}
			// Update channels
			for (UINT i=0; i<MAX_CHANNELS; i++)
			{
				if (pSndFile->Chn[i].pHeader == penv)
				{
					if (penv->nIFC & 0x80) pSndFile->Chn[i].nCutOff = penv->nIFC & 0x7F;
					if (penv->nIFR & 0x80) pSndFile->Chn[i].nResonance = penv->nIFR & 0x7F;
				}
			}
		}
	}
	if ((nCode == SB_ENDSCROLL) || (nCode == SB_THUMBPOSITION))
	{
		SwitchToView();
	}
}


void CCtrlInstruments::OnEditSampleMap()
//--------------------------------------
{
	if ((m_nInstrument) && (m_pModDoc))
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		INSTRUMENTHEADER *penv = pSndFile->Headers[m_nInstrument];
		if (penv)
		{
			CSampleMapDlg dlg(pSndFile, m_nInstrument, this);
			if (dlg.DoModal() == IDOK)
			{
				m_pModDoc->SetModified();
				m_pModDoc->UpdateAllViews(NULL, (m_nInstrument << 24) | HINT_INSTRUMENT, this);
				m_NoteMap.InvalidateRect(NULL, FALSE);
			}
		}
	}
}


