#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "moddoc.h"
#include "globals.h"
#include "ctrl_smp.h"
#include "view_smp.h"
#include "dlg_misc.h"
#include "mpdlgs.h"

#define	BASENOTE_MIN	(1*12)	// C-1
#define	BASENOTE_MAX	(9*12)	// C-9

#pragma warning(disable:4244)

BEGIN_MESSAGE_MAP(CCtrlSamples, CModControlDlg)
	//{{AFX_MSG_MAP(CCtrlSamples)
	ON_WM_VSCROLL()
	ON_COMMAND(IDC_SAMPLE_NEW,			OnSampleNew)
	ON_COMMAND(IDC_SAMPLE_OPEN,			OnSampleOpen)
	ON_COMMAND(IDC_SAMPLE_SAVEAS,		OnSampleSave)
	ON_COMMAND(IDC_SAMPLE_PLAY,			OnSamplePlay)
	ON_COMMAND(IDC_SAMPLE_NORMALIZE,	OnNormalize)
	ON_COMMAND(IDC_SAMPLE_AMPLIFY,		OnAmplify)
	ON_COMMAND(IDC_SAMPLE_UPSAMPLE,		OnUpsample)
	ON_COMMAND(IDC_SAMPLE_DOWNSAMPLE,	OnDownsample)
	ON_COMMAND(IDC_SAMPLE_REVERSE,		OnReverse)
	ON_COMMAND(IDC_SAMPLE_SILENCE,		OnSilence)
	ON_COMMAND(IDC_CHECK1,				OnSetPanningChanged)
	ON_COMMAND(ID_PREVINSTRUMENT,		OnPrevInstrument)
	ON_COMMAND(ID_NEXTINSTRUMENT,		OnNextInstrument)
	ON_EN_CHANGE(IDC_SAMPLE_NAME,		OnNameChanged)
	ON_EN_CHANGE(IDC_SAMPLE_FILENAME,	OnFileNameChanged)
	ON_EN_CHANGE(IDC_EDIT_SAMPLE,		OnSampleChanged)
	ON_EN_CHANGE(IDC_EDIT1,				OnLoopStartChanged)
	ON_EN_CHANGE(IDC_EDIT2,				OnLoopEndChanged)
	ON_EN_CHANGE(IDC_EDIT3,				OnSustainStartChanged)
	ON_EN_CHANGE(IDC_EDIT4,				OnSustainEndChanged)
	ON_EN_CHANGE(IDC_EDIT5,				OnFineTuneChanged)
	ON_EN_CHANGE(IDC_EDIT7,				OnVolumeChanged)
	ON_EN_CHANGE(IDC_EDIT8,				OnGlobalVolChanged)
	ON_EN_CHANGE(IDC_EDIT9,				OnPanningChanged)
	ON_EN_CHANGE(IDC_EDIT14,			OnVibSweepChanged)
	ON_EN_CHANGE(IDC_EDIT15,			OnVibDepthChanged)
	ON_EN_CHANGE(IDC_EDIT16,			OnVibRateChanged)
	ON_CBN_SELCHANGE(IDC_COMBO_BASENOTE,OnBaseNoteChanged)
	ON_CBN_SELCHANGE(IDC_COMBO_ZOOM,	OnZoomChanged)
	ON_CBN_SELCHANGE(IDC_COMBO1,		OnLoopTypeChanged)
	ON_CBN_SELCHANGE(IDC_COMBO2,		OnSustainTypeChanged)
	ON_CBN_SELCHANGE(IDC_COMBO3,		OnVibTypeChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CCtrlSamples::DoDataExchange(CDataExchange* pDX)
//---------------------------------------------------
{
	CModControlDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCtrlSamples)
	DDX_Control(pDX, IDC_TOOLBAR1,				m_ToolBar1);
	DDX_Control(pDX, IDC_TOOLBAR2,				m_ToolBar2);
	DDX_Control(pDX, IDC_SAMPLE_NAME,			m_EditName);
	DDX_Control(pDX, IDC_SAMPLE_FILENAME,		m_EditFileName);
	DDX_Control(pDX, IDC_SAMPLE_NAME,			m_EditName);
	DDX_Control(pDX, IDC_SAMPLE_FILENAME,		m_EditFileName);
	DDX_Control(pDX, IDC_COMBO_ZOOM,			m_ComboZoom);
	DDX_Control(pDX, IDC_COMBO_BASENOTE,		m_CbnBaseNote);
	DDX_Control(pDX, IDC_SPIN_SAMPLE,			m_SpinSample);
	DDX_Control(pDX, IDC_EDIT_SAMPLE,			m_EditSample);
	DDX_Control(pDX, IDC_CHECK1,				m_CheckPanning);
	DDX_Control(pDX, IDC_SPIN1,					m_SpinLoopStart);
	DDX_Control(pDX, IDC_SPIN2,					m_SpinLoopEnd);
	DDX_Control(pDX, IDC_SPIN3,					m_SpinSustainStart);
	DDX_Control(pDX, IDC_SPIN4,					m_SpinSustainEnd);
	DDX_Control(pDX, IDC_SPIN5,					m_SpinFineTune);
	DDX_Control(pDX, IDC_SPIN7,					m_SpinVolume);
	DDX_Control(pDX, IDC_SPIN8,					m_SpinGlobalVol);
	DDX_Control(pDX, IDC_SPIN9,					m_SpinPanning);
	DDX_Control(pDX, IDC_SPIN11,				m_SpinVibSweep);
	DDX_Control(pDX, IDC_SPIN12,				m_SpinVibDepth);
	DDX_Control(pDX, IDC_SPIN13,				m_SpinVibRate);
	DDX_Control(pDX, IDC_COMBO1,				m_ComboLoopType);
	DDX_Control(pDX, IDC_COMBO2,				m_ComboSustainType);
	DDX_Control(pDX, IDC_COMBO3,				m_ComboAutoVib);
	DDX_Control(pDX, IDC_EDIT1,					m_EditLoopStart);
	DDX_Control(pDX, IDC_EDIT2,					m_EditLoopEnd);
	DDX_Control(pDX, IDC_EDIT3,					m_EditSustainStart);
	DDX_Control(pDX, IDC_EDIT4,					m_EditSustainEnd);
	DDX_Control(pDX, IDC_EDIT5,					m_EditFineTune);
	DDX_Control(pDX, IDC_EDIT7,					m_EditVolume);
	DDX_Control(pDX, IDC_EDIT8,					m_EditGlobalVol);
	DDX_Control(pDX, IDC_EDIT9,					m_EditPanning);
	DDX_Control(pDX, IDC_EDIT14,				m_EditVibSweep);
	DDX_Control(pDX, IDC_EDIT15,				m_EditVibDepth);
	DDX_Control(pDX, IDC_EDIT16,				m_EditVibRate);
	//}}AFX_DATA_MAP
}


CCtrlSamples::CCtrlSamples()
//--------------------------
{
	m_nSample = 1;
	m_nLockCount = 1;
}


CRuntimeClass *CCtrlSamples::GetAssociatedViewClass()
//---------------------------------------------------
{
	return RUNTIME_CLASS(CViewSample);
}


BOOL CCtrlSamples::OnInitDialog()
//-------------------------------
{
	CModControlDlg::OnInitDialog();
	if (!m_pSndFile) return TRUE;
	m_bInitialized = FALSE;
	// Zoom Selection
	m_ComboZoom.AddString("Auto");
	m_ComboZoom.AddString("1:1");
	m_ComboZoom.AddString("1:2");
	m_ComboZoom.AddString("1:4");
	m_ComboZoom.AddString("1:8");
	m_ComboZoom.AddString("1:16");
	m_ComboZoom.AddString("1:32");
	m_ComboZoom.AddString("1:64");
	m_ComboZoom.SetCurSel(0);
	// File ToolBar
	m_ToolBar1.Init();
	m_ToolBar1.AddButton(IDC_SAMPLE_NEW, 6);
	m_ToolBar1.AddButton(IDC_SAMPLE_OPEN, 12);
	m_ToolBar1.AddButton(IDC_SAMPLE_SAVEAS, 13);
	// Edit ToolBar
	m_ToolBar2.Init();
	m_ToolBar2.AddButton(IDC_SAMPLE_PLAY, 14);
	m_ToolBar2.AddButton(IDC_SAMPLE_NORMALIZE, 8);
	m_ToolBar2.AddButton(IDC_SAMPLE_AMPLIFY, 9);
	m_ToolBar2.AddButton(IDC_SAMPLE_UPSAMPLE, 10);
	m_ToolBar2.AddButton(IDC_SAMPLE_DOWNSAMPLE, 29);
	m_ToolBar2.AddButton(IDC_SAMPLE_REVERSE, 11);
	m_ToolBar2.AddButton(IDC_SAMPLE_SILENCE, 22);
	// Setup Controls
	m_EditName.SetLimitText(32);
	m_EditFileName.SetLimitText(22);
	m_SpinVolume.SetRange(0, 64);
	m_SpinGlobalVol.SetRange(0, 64);
	m_SpinPanning.SetRange(0, 256);
	m_ComboAutoVib.AddString("Sine");
	m_ComboAutoVib.AddString("Square");
	m_ComboAutoVib.AddString("Ramp Up");
	m_ComboAutoVib.AddString("Ramp Down");
	m_ComboAutoVib.AddString("Random");
	m_SpinVibSweep.SetRange(0, 64);
	m_SpinVibDepth.SetRange(0, 64);
	m_SpinVibRate.SetRange(0, 64);
	for (UINT i=BASENOTE_MIN; i<BASENOTE_MAX; i++)
	{
		CHAR s[32];
		wsprintf(s, "%s%d", szNoteNames[i%12], i/12);
		m_CbnBaseNote.AddString(s);
	}
	return TRUE;
}


void CCtrlSamples::RecalcLayout()
//-------------------------------
{
}


BOOL CCtrlSamples::SetCurrentSample(UINT nSmp, LONG lZoom, BOOL bUpdNum)
//----------------------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	if (!pModDoc) return FALSE;
	pSndFile = pModDoc->GetSoundFile();
	if (pSndFile->m_nSamples < 1) pSndFile->m_nSamples = 1;
	if ((nSmp < 1) || (nSmp > pSndFile->m_nSamples)) return FALSE;
	LockControls();
	if (m_nSample != nSmp)
	{
		m_nSample = nSmp;
		UpdateView((m_nSample << 24) | HINT_SAMPLEINFO, NULL);
	}
	if (bUpdNum)
	{
		SetDlgItemInt(IDC_EDIT_SAMPLE, m_nSample);
		m_SpinSample.SetRange(1, pSndFile->m_nSamples);
	}
	if (lZoom < 0)
		lZoom = m_ComboZoom.GetCurSel();
	else
		m_ComboZoom.SetCurSel(lZoom);
	PostViewMessage(VIEWMSG_SETCURRENTSAMPLE, (lZoom << 16) | m_nSample);
	UnlockControls();
	return TRUE;
}


void CCtrlSamples::OnActivatePage(LPARAM lParam)
//----------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((pModDoc) && (m_pParent))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		if (lParam < 0)
		{
			int nIns = m_pParent->GetInstrumentChange();
			if (pSndFile->m_nInstruments)
			{
				if ((nIns > 0) && (!pModDoc->IsChildSample(m_nSample, nIns)))
				{
					UINT k = pModDoc->FindInstrumentChild(nIns);
					if (k > 0) lParam = k;
				}
			} else
			{
				if (nIns > 0) lParam = nIns;
				m_pParent->InstrumentChanged(-1);
			}
		} else
		if (lParam > 0)
		{
			if (pSndFile->m_nInstruments)
			{
				UINT k = m_pParent->GetInstrumentChange();
				if (!pModDoc->IsChildSample(k, lParam))
				{
					UINT nins = pModDoc->FindSampleParent(lParam);
					if (nins) m_pParent->InstrumentChanged(nins);
				}
			} else
			{
				m_pParent->InstrumentChanged(lParam);
			}
		}
	}
	SetCurrentSample((lParam > 0) ? lParam : m_nSample);
	// Initial Update
	if (!m_bInitialized) UpdateView((m_nSample << 24) | HINT_SAMPLEINFO | HINT_MODTYPE, NULL);
	CChildFrame *pFrame = (CChildFrame *)GetParentFrame();
	if ((pFrame) && (m_hWndView)) PostViewMessage(VIEWMSG_LOADSTATE, (LPARAM)pFrame->GetSampleViewState());
	SwitchToView();
}


void CCtrlSamples::OnDeactivatePage()
//-----------------------------------
{
	CChildFrame *pFrame = (CChildFrame *)GetParentFrame();
	if ((pFrame) && (m_hWndView)) SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)pFrame->GetSampleViewState());
	if (m_pModDoc) m_pModDoc->NoteOff(0, TRUE);
}


LRESULT CCtrlSamples::OnModCtrlMsg(WPARAM wParam, LPARAM lParam)
//---------------------------------------------------------------
{
	switch(wParam)
	{
	case CTRLMSG_SMP_PREVINSTRUMENT:
		OnPrevInstrument();
		break;

	case CTRLMSG_SMP_NEXTINSTRUMENT:
		OnNextInstrument();
		break;

	case CTRLMSG_SMP_OPENFILE:
		if (lParam) return OpenSample((LPCSTR)lParam);
		break;

	case CTRLMSG_SMP_SONGDROP:
		if (lParam)
		{
			LPDRAGONDROP pDropInfo = (LPDRAGONDROP)lParam;
			CSoundFile *pSndFile = (CSoundFile *)(pDropInfo->lDropParam);
			if (pDropInfo->pModDoc) pSndFile = pDropInfo->pModDoc->GetSoundFile();
			if (pSndFile) return OpenSample(pSndFile, pDropInfo->dwDropItem);
		}
		break;

	case CTRLMSG_SMP_SETZOOM:
		SetCurrentSample(m_nSample, lParam, FALSE);
		break;

	case CTRLMSG_SETCURRENTINSTRUMENT:
		SetCurrentSample(lParam, -1, TRUE);
		break;

	default:
		return CModControlDlg::OnModCtrlMsg(wParam, lParam);
	}
	return 0;
}


BOOL CCtrlSamples::GetToolTipText(UINT uId, LPSTR pszText)
//--------------------------------------------------------
{
	if ((pszText) && (uId))
	{
		switch(uId)
		{
		case IDC_EDIT5:
		case IDC_SPIN5:
		case IDC_COMBO_BASENOTE:
			if ((m_pSndFile) && (m_pSndFile->m_nType & MOD_TYPE_XM) && (m_nSample))
			{
				MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
				UINT nFreqHz = CSoundFile::TransposeToFrequency(pins->RelativeTone, pins->nFineTune);
				wsprintf(pszText, "%ldHz", nFreqHz);
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}


void CCtrlSamples::UpdateView(DWORD dwHintMask, CObject *pObj)
//------------------------------------------------------------
{
	if ((pObj == this) || (!m_pModDoc) || (!m_pSndFile)) return;
	if (dwHintMask & HINT_MPTOPTIONS)
	{
		m_ToolBar1.UpdateStyle();
		m_ToolBar2.UpdateStyle();
	}
	if (!(dwHintMask & (HINT_SAMPLEINFO|HINT_MODTYPE))) return;
	if (((dwHintMask >> 24) != (m_nSample&0xff)) && (!(dwHintMask & HINT_MODTYPE))) return;
	LockControls();
	if (!m_bInitialized) dwHintMask |= HINT_MODTYPE;
	// Updating Ranges
	if (dwHintMask & HINT_MODTYPE)
	{
		BOOL b;
		// Loop Type
		m_ComboLoopType.ResetContent();
		m_ComboLoopType.AddString("Off");
		m_ComboLoopType.AddString("On");
		// Sustain Loop Type
		m_ComboSustainType.ResetContent();
		m_ComboSustainType.AddString("Off");
		m_ComboSustainType.AddString("On");
		// Bidirectional Loops
		if (m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT))
		{
			m_ComboLoopType.AddString("Bidi");
			m_ComboSustainType.AddString("Bidi");
		}
		// Loop Start
		m_SpinLoopStart.SetRange(-1, 1);
		m_SpinLoopStart.SetPos(0);
		// Loop End
		m_SpinLoopEnd.SetRange(-1, 1);
		m_SpinLoopEnd.SetPos(0);
		// Sustain Loop Start
		m_SpinSustainStart.SetRange(-1, 1);
		m_SpinSustainStart.SetPos(0);
		// Sustain Loop End
		m_SpinSustainEnd.SetRange(-1, 1);
		m_SpinSustainEnd.SetPos(0);
		// Sustain Loops only available in IT
		b = (m_pSndFile->m_nType == MOD_TYPE_IT) ? TRUE : FALSE;
		m_ComboSustainType.EnableWindow(b);
		m_SpinSustainStart.EnableWindow(b);
		m_SpinSustainEnd.EnableWindow(b);
		m_EditSustainStart.EnableWindow(b);
		m_EditSustainEnd.EnableWindow(b);
		// Finetune / C-4 Speed / BaseNote
		b = (m_pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)) ? TRUE : FALSE;
		SetDlgItemText(IDC_TEXT7, (b) ? "Freq. (Hz)" : "Finetune");
		m_SpinFineTune.SetRange(-1, 1);
		m_EditFileName.EnableWindow(b);
		// AutoVibrato
		b = (m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) ? TRUE : FALSE;
		m_ComboAutoVib.EnableWindow(b);
		m_SpinVibSweep.EnableWindow(b);
		m_SpinVibDepth.EnableWindow(b);
		m_SpinVibRate.EnableWindow(b);
		m_EditVibSweep.EnableWindow(b);
		m_EditVibDepth.EnableWindow(b);
		m_EditVibRate.EnableWindow(b);
		// Global Volume
		b = (m_pSndFile->m_nType == MOD_TYPE_IT) ? TRUE : FALSE;
		m_EditGlobalVol.EnableWindow(b);
		m_SpinGlobalVol.EnableWindow(b);
		// Panning
		b = (m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) ? TRUE : FALSE;
		m_CheckPanning.EnableWindow(b);
		m_EditPanning.EnableWindow(b);
		m_SpinPanning.EnableWindow(b);
	}
	// Updating Values
	if (dwHintMask & (HINT_MODTYPE|HINT_SAMPLEINFO))
	{
		MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
		CHAR s[128];
		DWORD d;
		
		// Length / Type
		wsprintf(s, "%d-bit %s, len: %d", (pins->uFlags & CHN_16BIT) ? 16 : 8, (pins->uFlags & CHN_STEREO) ? "stereo" : "mono", pins->nLength);
		SetDlgItemText(IDC_TEXT5, s);
		// Name
		memcpy(s, m_pSndFile->m_szNames[m_nSample], 32);
		s[31] = 0;
		SetDlgItemText(IDC_SAMPLE_NAME, s);
		// File Name
		memcpy(s, pins->name, 22);
		s[21] = 0;
		if (m_pSndFile->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM)) s[0] = 0;
		SetDlgItemText(IDC_SAMPLE_FILENAME, s);
		// Volume
		SetDlgItemInt(IDC_EDIT7, pins->nVolume >> 2);
		// Global Volume
		SetDlgItemInt(IDC_EDIT8, pins->nGlobalVol);
		// Panning
		CheckDlgButton(IDC_CHECK1, (pins->uFlags & CHN_PANNING) ? MF_CHECKED : 0);
		SetDlgItemInt(IDC_EDIT9, pins->nPan);
		// FineTune / C-4 Speed / BaseNote
		int transp = 0;
		if (m_pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
		{
			wsprintf(s, "%lu", pins->nC4Speed);
			m_EditFineTune.SetWindowText(s);
			transp = CSoundFile::FrequencyToTranspose(pins->nC4Speed) >> 7;
		} else
		{
			SetDlgItemInt(IDC_EDIT5, (int)pins->nFineTune);
			transp = (int)pins->RelativeTone;
		}
		int basenote = 60 - transp;
		if (basenote < BASENOTE_MIN) basenote = BASENOTE_MIN;
		if (basenote >= BASENOTE_MAX) basenote = BASENOTE_MAX-1;
		basenote -= BASENOTE_MIN;
		if (basenote != m_CbnBaseNote.GetCurSel()) m_CbnBaseNote.SetCurSel(basenote);
		// AutoVibrato
		m_ComboAutoVib.SetCurSel(pins->nVibType);
		SetDlgItemInt(IDC_EDIT14, (UINT)pins->nVibSweep);
		SetDlgItemInt(IDC_EDIT15, (UINT)pins->nVibDepth);
		SetDlgItemInt(IDC_EDIT16, (UINT)pins->nVibRate);
		// Loop
		d = 0;
		if (pins->uFlags & CHN_LOOP) d = (pins->uFlags & CHN_PINGPONGLOOP) ? 2 : 1;
		m_ComboLoopType.SetCurSel(d);
		wsprintf(s, "%lu", pins->nLoopStart);
		m_EditLoopStart.SetWindowText(s);
		wsprintf(s, "%lu", pins->nLoopEnd);
		m_EditLoopEnd.SetWindowText(s);
		// Sustain Loop
		d = 0;
		if (pins->uFlags & CHN_SUSTAINLOOP) d = (pins->uFlags & CHN_PINGPONGSUSTAIN) ? 2 : 1;
		m_ComboSustainType.SetCurSel(d);
		wsprintf(s, "%lu", pins->nSustainStart);
		m_EditSustainStart.SetWindowText(s);
		wsprintf(s, "%lu", pins->nSustainEnd);
		m_EditSustainEnd.SetWindowText(s);
	}
	if (!m_bInitialized)
	{
		// First update
		m_bInitialized = TRUE;
		UnlockControls();
	}
	UnlockControls();
}


BOOL CCtrlSamples::OpenSample(LPCSTR lpszFileName)
//------------------------------------------------
{
	CMappedFile f;
	CHAR szName[_MAX_FNAME], szExt[_MAX_EXT];
	LPBYTE lpFile;
	DWORD len;
	BOOL bOk;
	
	BeginWaitCursor();
	if ((!lpszFileName) || (!f.Open(lpszFileName)))
	{
		EndWaitCursor();
		return FALSE;
	}
	len = f.GetLength();
	if (len > CTrackApp::gMemStatus.dwTotalPhys) len = CTrackApp::gMemStatus.dwTotalPhys;
	bOk = FALSE;
	lpFile = f.Lock(len);
	if (!lpFile) goto OpenError;
	BEGIN_CRITICAL();
	if (m_pSndFile->ReadSampleFromFile(m_nSample, lpFile, len))
	{
		bOk = TRUE;
	}
	END_CRITICAL();
	if (!bOk)
	{
		CRawSampleDlg dlg(this);
		EndWaitCursor();
		if (dlg.DoModal() == IDOK)
		{
			BeginWaitCursor();
			UINT flags = 0;
			MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
			BEGIN_CRITICAL();
			m_pSndFile->DestroySample(m_nSample);
			pins->nLength = len;
			pins->uFlags = RS_PCM8S;
			pins->nGlobalVol = 64;
			pins->nVolume = 256;
			pins->nPan = 128;
			pins->name[0] = 0;
			if (!pins->nC4Speed) pins->nC4Speed = 22050;
			if (dlg.m_nFormat & 1)
			{
				pins->nLength >>= 1;
				pins->uFlags |= CHN_16BIT;
				flags = RS_PCM16S;
			}
			if (!(dlg.m_nFormat & 2))
			{
				flags++;
			}
			// Interleaved Stereo Sample
			if (dlg.m_nFormat & 4)
			{
				pins->uFlags |= CHN_STEREO;
				pins->nLength >>= 1;
				flags |= 0x40|RSF_STEREO;
			}
			LPSTR p16 = (LPSTR)lpFile;
			DWORD l16 = len;
			if ((pins->uFlags & CHN_16BIT) && (len & 1))
			{
				p16++;
				l16--;
			}
			if (m_pSndFile->ReadSample(pins, flags, p16, l16))
			{
				bOk = TRUE;
			}
			END_CRITICAL();
		}
	}
	f.Unlock();
OpenError:
	f.Close();
	EndWaitCursor();
	if (bOk)
	{
		CHAR szPath[_MAX_PATH], szNewPath[_MAX_PATH];
		MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
		_splitpath(lpszFileName, szNewPath, szPath, szName, szExt);
		strcat(szNewPath, szPath);
		strcpy(CMainFrame::m_szCurSmpDir, szNewPath);
		if (!pins->name[0])
		{
			memset(szPath, 0, 32);
			strcpy(szPath, szName);
			if (m_pSndFile->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM))
			{
				// MOD/XM
				strcat(szPath, szExt);
				szPath[31] = 0;
				memcpy(m_pSndFile->m_szNames[m_nSample], szPath, 32);
			} else
			{
				// S3M/IT
				szPath[31] = 0;
				if (!m_pSndFile->m_szNames[m_nSample][0]) memcpy(m_pSndFile->m_szNames[m_nSample], szPath, 32);
				if (strlen(szPath) < 9) strcat(szPath, szExt);
			}
			szPath[21] = 0;
			memcpy(pins->name, szPath, 22);
		}
		if ((m_pSndFile->m_nType & MOD_TYPE_XM) && (!(pins->uFlags & CHN_PANNING)))
		{
			pins->nPan = 128;
			pins->uFlags |= CHN_PANNING;
		}
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA | HINT_SAMPLEINFO | HINT_SMPNAMES, NULL);
		m_pModDoc->SetModified();
	}
	return TRUE;
}


BOOL CCtrlSamples::OpenSample(CSoundFile *pSndFile, UINT nSample)
//---------------------------------------------------------------
{
	if ((!pSndFile) || (!nSample) || (nSample > pSndFile->m_nSamples)) return FALSE;
	BeginWaitCursor();
	BEGIN_CRITICAL();
	m_pSndFile->DestroySample(m_nSample);
	m_pSndFile->ReadSampleFromSong(m_nSample, pSndFile, nSample);
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	if ((m_pSndFile->m_nType & MOD_TYPE_XM) && (!(pins->uFlags & CHN_PANNING)))
	{
		pins->nPan = 128;
		pins->uFlags |= CHN_PANNING;
	}
	END_CRITICAL();
	m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA | HINT_SAMPLEINFO | HINT_SMPNAMES, NULL);
	m_pModDoc->SetModified();
	EndWaitCursor();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////
// CCtrlSamples messages

void CCtrlSamples::OnSampleChanged()
//----------------------------------
{
	if ((!IsLocked()) && (m_pSndFile))
	{
		UINT n = GetDlgItemInt(IDC_EDIT_SAMPLE);
		if ((n > 0) && (n <= m_pSndFile->m_nSamples) && (n != m_nSample))
		{
			SetCurrentSample(n, -1, FALSE);
			if (m_pParent)
			{
				if (m_pSndFile->m_nInstruments)
				{
					UINT k = m_pParent->GetInstrumentChange();
					if (!m_pModDoc->IsChildSample(k, m_nSample))
					{
						UINT nins = m_pModDoc->FindSampleParent(m_nSample);
						if (nins) m_pParent->InstrumentChanged(nins);
					}
				} else
				{
					m_pParent->InstrumentChanged(m_nSample);
				}
			}
		}
	}
}


void CCtrlSamples::OnZoomChanged()
//--------------------------------
{
	if (!IsLocked()) SetCurrentSample(m_nSample);
	SwitchToView();
}


void CCtrlSamples::OnSampleNew()
//------------------------------
{
	LONG smp = m_pModDoc->InsertSample(TRUE);
	if (smp > 0)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		SetCurrentSample(smp);
		m_pModDoc->UpdateAllViews(NULL, (smp << 24) | HINT_SAMPLEINFO | HINT_SAMPLEDATA | HINT_SMPNAMES);
		if ((pSndFile->m_nInstruments) && (!m_pModDoc->FindSampleParent(smp)))
		{
			if (MessageBox("This sample is not used by any instrument. Do you want to create a new instrument using this sample ?",
					NULL, MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				UINT nins = m_pModDoc->InsertInstrument(smp);
				m_pModDoc->UpdateAllViews(NULL, (nins << 24) | HINT_INSTRUMENT | HINT_INSNAMES | HINT_ENVELOPE);
				m_pParent->InstrumentChanged(nins);
			}
		}
	}
	SwitchToView();
}


void CCtrlSamples::OnSampleOpen()
//-------------------------------
{
	static int nLastIndex = 0;
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
					"All Samples|*.wav;*.pat;*.s3i;*.smp;*.snd;*.raw;*.xi;*.aif;*.aiff;*.its;*.8sv;*.8svx;*.svx;*.pcm|"
					"Wave Files (*.wav)|*.wav|"
					"XI Samples (*.xi)|*.xi|"
					"Impulse Tracker Samples (*.its)|*.its|"
					"ScreamTracker Samples (*.s3i,*.smp)|*.s3i;*.smp|"
					"GF1 Patches (*.pat)|*.pat|"
					"AIFF Files (*.aiff;*.8svx)|*.aif;*.aiff;*.8sv;*.8svx;*.svx|"
					"Raw Samples (*.raw,*.snd,*.pcm)|*.raw;*.snd;*.pcm|"
					"All Files (*.*)|*.*||",
					this);
	if (CMainFrame::m_szCurSmpDir[0])
	{
		dlg.m_ofn.lpstrInitialDir = CMainFrame::m_szCurSmpDir;
	}
	dlg.m_ofn.nFilterIndex = nLastIndex;
	if (dlg.DoModal() != IDOK) return;
	nLastIndex = dlg.m_ofn.nFilterIndex;
	if (!OpenSample(dlg.GetPathName())) ErrorBox(IDS_ERR_FILEOPEN, this);
	SwitchToView();
}


void CCtrlSamples::OnSampleSave()
//-------------------------------
{
	CHAR szFileName[_MAX_PATH] = "";

	if ((!m_pSndFile) || (!m_nSample) || (!m_pSndFile->Ins[m_nSample].pSample))
	{
		SwitchToView();
		return;
	}
	if (m_pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
	{
		memcpy(szFileName, m_pSndFile->Ins[m_nSample].name, 22);
		szFileName[22] = 0;
	} else
	{
		memcpy(szFileName, m_pSndFile->m_szNames[m_nSample], 32);
		szFileName[32] = 0;
	}
	if (!szFileName[0]) strcpy(szFileName, "untitled");
	CFileDialog dlg(FALSE, "wav",
			szFileName,
			OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
			"Wave File (*.wav)|*.wav||",
			this);
	if (CMainFrame::m_szCurSmpDir[0])
	{
		dlg.m_ofn.lpstrInitialDir = CMainFrame::m_szCurSmpDir;
	}
	if (dlg.DoModal() != IDOK) return;
	BeginWaitCursor();
	BOOL bOk = m_pSndFile->SaveWAVSample(m_nSample, dlg.GetPathName());
	EndWaitCursor();
	if (!bOk)
	{
		ErrorBox(IDS_ERR_SAVESMP, this);
	} else
	{
		CHAR drive[_MAX_DRIVE], path[_MAX_PATH];
		_splitpath(dlg.GetPathName(), drive, path, NULL, NULL);
		strcpy(CMainFrame::m_szCurSmpDir, drive);
		strcat(CMainFrame::m_szCurSmpDir, path);
	}
	SwitchToView();
}


void CCtrlSamples::OnSamplePlay()
//-------------------------------
{
	if ((m_pModDoc) && (m_pSndFile))
	{
		if ((m_pSndFile->IsPaused()) && (m_pModDoc->IsNotePlaying(0, m_nSample, 0)))
		{
			m_pModDoc->NoteOff(0, TRUE);
		} else
		{
			m_pModDoc->PlayNote(NOTE_MIDDLEC, 0, m_nSample, TRUE);
		}
	}
	SwitchToView();
}


void CCtrlSamples::OnNormalize()
//------------------------------
{
	BeginWaitCursor();
	if ((m_pModDoc) && (m_pSndFile) && (m_pSndFile->Ins[m_nSample].pSample))
	{
		BOOL bOk = FALSE;
		MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	
		if (pins->uFlags & CHN_16BIT)
		{
			UINT len = pins->nLength;
			signed short *p = (signed short *)pins->pSample;
			if (pins->uFlags & CHN_STEREO) len *= 2;
			int max = 1;
			for (UINT i=0; i<len; i++)
			{
				if (p[i] > max) max = p[i];
				if (-p[i] > max) max = -p[i];
			}
			if (max < 32767)
			{
				max++;
				for (UINT j=0; j<len; j++)
				{
					int l = p[j];
					p[j] = (l << 15) / max;
				}
				bOk = TRUE;
			}
		} else
		{
			UINT len = pins->nLength;
			signed char *p = (signed char *)pins->pSample;
			if (pins->uFlags & CHN_STEREO) len *= 2;
			int max = 1;
			for (UINT i=0; i<len; i++)
			{
				if (p[i] > max) max = p[i];
				if (-p[i] > max) max = -p[i];
			}
			if (max < 127)
			{
				max++;
				for (UINT j=0; j<len; j++)
				{
					int l = p[j];
					p[j] = (l << 7) / max;
				}
				bOk = TRUE;
			}
		}
		if (bOk)
		{
			m_pModDoc->AdjustEndOfSample(m_nSample);
			m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, NULL);
			m_pModDoc->SetModified();
		}
	}
	EndWaitCursor();
	SwitchToView();
}


void CCtrlSamples::OnAmplify()
//----------------------------
{
	static UINT snOldAmp = 100;
	SAMPLEVIEWSTATE viewstate;
	DWORD dwStart, dwEnd;
	MODINSTRUMENT *pins;
	CAmpDlg dlg(this, snOldAmp);
	
	memset(&viewstate, 0, sizeof(viewstate));
	SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)&viewstate);
	if ((!m_pModDoc) || (!m_pSndFile) || (!m_pSndFile->Ins[m_nSample].pSample)) return;
	if (dlg.DoModal() != IDOK) return;
	snOldAmp = dlg.m_nFactor;
	BeginWaitCursor();
	pins = &m_pSndFile->Ins[m_nSample];
	dwStart = viewstate.dwBeginSel;
	dwEnd = viewstate.dwEndSel;
	if (dwEnd > pins->nLength) dwEnd = pins->nLength;
	if (dwStart > dwEnd) dwStart = dwEnd;
	if (dwStart >= dwEnd)
	{
		dwStart = 0;
		dwEnd = pins->nLength;
	}
	if (pins->uFlags & CHN_STEREO) { dwStart *= 2; dwEnd *= 2; }
	UINT len = dwEnd - dwStart;
	LONG lAmp = dlg.m_nFactor;
	if ((dlg.m_bFadeIn) && (dlg.m_bFadeOut)) lAmp *= 4;
	if (pins->uFlags & CHN_16BIT)
	{
		signed short *p = ((signed short *)pins->pSample) + dwStart;

		for (UINT i=0; i<len; i++)
		{
			LONG l = (p[i] * lAmp) / 100;
			if (dlg.m_bFadeIn) l = (LONG)((l * (LONGLONG)i) / len);
			if (dlg.m_bFadeOut) l = (LONG)((l * (LONGLONG)(len-i)) / len);
			if (l < -32768) l = -32768;
			if (l > 32767) l = 32767;
			p[i] = (signed short)l;
		}
	} else
	{
		signed char *p = ((signed char *)pins->pSample) + dwStart;

		for (UINT i=0; i<len; i++)
		{
			LONG l = (p[i] * lAmp) / 100;
			if (dlg.m_bFadeIn) l = (LONG)((l * (LONGLONG)i) / len);
			if (dlg.m_bFadeOut) l = (LONG)((l * (LONGLONG)(len-i)) / len);
			if (l < -128) l = -128;
			if (l > 127) l = 127;
			p[i] = (signed char)l;
		}
	}
	m_pModDoc->AdjustEndOfSample(m_nSample);
	m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, NULL);
	m_pModDoc->SetModified();
	EndWaitCursor();
	SwitchToView();
}


const int gSinc2x16Odd[16] =
{ // Kaiser window, beta=7.400
  -19,    97,  -295,   710, -1494,  2958, -6155, 20582, 20582, -6155,  2958, -1494,   710,  -295,    97,   -19,
};


void CCtrlSamples::OnUpsample()
//-----------------------------
{
	SAMPLEVIEWSTATE viewstate;
	MODINSTRUMENT *pins;
	DWORD dwStart, dwEnd, dwNewLen;
	UINT smplsize, newsmplsize;
	PVOID pOriginal, pNewSample;

	SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)&viewstate);
	if ((!m_pSndFile) || (!m_pSndFile->Ins[m_nSample].pSample)) return;
	BeginWaitCursor();
	pins = &m_pSndFile->Ins[m_nSample];
	dwStart = viewstate.dwBeginSel;
	dwEnd = viewstate.dwEndSel;
	if (dwEnd > pins->nLength) dwEnd = pins->nLength;
	if (dwStart >= dwEnd)
	{
		dwStart = 0;
		dwEnd = pins->nLength;
	}
	smplsize = (pins->uFlags & CHN_16BIT) ? 2 : 1;
	if (pins->uFlags & CHN_STEREO) smplsize *= 2;
	newsmplsize = (pins->uFlags & CHN_STEREO) ? 4 : 2;
	pOriginal = pins->pSample;
	dwNewLen = pins->nLength + (dwEnd-dwStart);
	pNewSample = NULL;
	if (dwNewLen+4 <= MAX_SAMPLE_LENGTH) pNewSample = CSoundFile::AllocateSample((dwNewLen+4)*newsmplsize);
	if (pNewSample)
	{
		UINT nCh = (pins->uFlags & CHN_STEREO) ? 2 : 1;
		for (UINT iCh=0; iCh<nCh; iCh++)
		{
			int len = dwEnd-dwStart;
			int maxndx = pins->nLength;
			if (pins->uFlags & CHN_16BIT)
			{
				signed short *psrc = ((signed short *)pOriginal)+iCh;
				signed short *pdest = ((signed short *)pNewSample)+dwStart*nCh+iCh;
				for (int i=0; i<len; i++)
				{
					int accum = 0x80;
					for (int j=0; j<16; j++)
					{
						int ndx = dwStart + i + j - 7;
						if (ndx < 0) ndx = 0;
						if (ndx > maxndx) ndx = maxndx;
						accum += (psrc[ndx*nCh] * gSinc2x16Odd[j] + 0x40) >> 7;
					}
					accum >>= 8;
					if (accum > 0x7fff) accum = 0x7fff;
					if (accum < -0x8000) accum = -0x8000;
					pdest[0] = psrc[(dwStart+i)*nCh];
					pdest[nCh] = (short int)(accum);
					pdest += nCh*2;
				}
			} else
			{
				signed char *psrc = ((signed char *)pOriginal)+iCh;
				signed short *pdest = ((signed short *)pNewSample)+dwStart*nCh+iCh;
				for (int i=0; i<len; i++)
				{
					int accum = 0x40;
					for (int j=0; j<16; j++)
					{
						int ndx = dwStart + i + j - 7;
						if (ndx < 0) ndx = 0;
						if (ndx > maxndx) ndx = maxndx;
						accum += psrc[ndx*nCh] * gSinc2x16Odd[j];
					}
					accum >>= 7;
					if (accum > 0x7fff) accum = 0x7fff;
					if (accum < -0x8000) accum = -0x8000;
					pdest[0] = (signed short)((int)psrc[(dwStart+i)*nCh]<<8);
					pdest[nCh] = (signed short)(accum);
					pdest += nCh*2;
				}
			}
		}
		if (pins->uFlags & CHN_16BIT)
		{
			if (dwStart > 0) memcpy(pNewSample, pOriginal, dwStart*smplsize);
			if (dwEnd < pins->nLength) memcpy(((LPSTR)pNewSample)+(dwStart+(dwEnd-dwStart)*2)*smplsize, ((LPSTR)pOriginal)+(dwEnd*smplsize), (pins->nLength-dwEnd)*smplsize);
		} else
		{
			if (dwStart > 0)
			{
				for (UINT i=0; i<dwStart*nCh; i++)
				{
					((signed short *)pNewSample)[i] = (signed short)(((signed char *)pOriginal)[i] << 8);
				}
			}
			if (dwEnd < pins->nLength)
			{
				signed short *pdest = ((signed short *)pNewSample) + (dwEnd-dwStart)*nCh;
				for (UINT i=dwEnd*nCh; i<pins->nLength*nCh; i++)
				{
					pdest[i] = (signed short)(((signed char *)pOriginal)[i] << 8);
				}
			}
		}
		if (pins->nLoopStart >= dwEnd) pins->nLoopStart += (dwEnd-dwStart); else
		if (pins->nLoopStart > dwStart) pins->nLoopStart += (pins->nLoopStart - dwStart);
		if (pins->nLoopEnd >= dwEnd) pins->nLoopEnd += (dwEnd-dwStart); else
		if (pins->nLoopEnd > dwStart) pins->nLoopEnd += (pins->nLoopEnd - dwStart);
		if (pins->nSustainStart >= dwEnd) pins->nSustainStart += (dwEnd-dwStart); else
		if (pins->nSustainStart > dwStart) pins->nSustainStart += (pins->nSustainStart - dwStart);
		if (pins->nSustainEnd >= dwEnd) pins->nSustainEnd += (dwEnd-dwStart); else
		if (pins->nSustainEnd > dwStart) pins->nSustainEnd += (pins->nSustainEnd - dwStart);
		BEGIN_CRITICAL();
		for (UINT iFix=0; iFix<MAX_CHANNELS; iFix++)
		{
			if ((PVOID)m_pSndFile->Chn[iFix].pSample == pOriginal)
			{
				m_pSndFile->Chn[iFix].pSample = (LPSTR)pNewSample;
				m_pSndFile->Chn[iFix].pCurrentSample = (LPSTR)pNewSample;
				m_pSndFile->Chn[iFix].dwFlags |= CHN_16BIT;
			}
		}
		pins->uFlags |= CHN_16BIT;
		pins->pSample = (LPSTR)pNewSample;
		pins->nLength = dwNewLen;
		if (viewstate.dwEndSel <= viewstate.dwBeginSel)
		{
			if (pins->nC4Speed < 200000) pins->nC4Speed *= 2;
			if (pins->RelativeTone < 84) pins->RelativeTone += 12;
		}
		CSoundFile::FreeSample(pOriginal);
		END_CRITICAL();
		m_pModDoc->AdjustEndOfSample(m_nSample);
		if (viewstate.dwEndSel > viewstate.dwBeginSel)
		{
			viewstate.dwBeginSel = dwStart;
			viewstate.dwEndSel = dwEnd + (dwEnd-dwStart);
			SendViewMessage(VIEWMSG_LOADSTATE, (LPARAM)&viewstate);
		}
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA | HINT_SAMPLEINFO, NULL);
		m_pModDoc->SetModified();
	}
	EndWaitCursor();
	SwitchToView();
}


void CCtrlSamples::OnDownsample()
//-------------------------------
{
	SAMPLEVIEWSTATE viewstate;
	MODINSTRUMENT *pins;
	DWORD dwStart, dwEnd, dwRemove, dwNewLen;
	UINT smplsize;
	PVOID pOriginal, pNewSample;

	SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)&viewstate);
	if ((!m_pSndFile) || (!m_pSndFile->Ins[m_nSample].pSample)) return;
	BeginWaitCursor();
	pins = &m_pSndFile->Ins[m_nSample];
	dwStart = viewstate.dwBeginSel;
	dwEnd = viewstate.dwEndSel;
	if (dwEnd > pins->nLength) dwEnd = pins->nLength;
	if (dwStart >= dwEnd)
	{
		dwStart = 0;
		dwEnd = pins->nLength;
	}
	smplsize = (pins->uFlags & CHN_16BIT) ? 2 : 1;
	if (pins->uFlags & CHN_STEREO) smplsize *= 2;
	pOriginal = pins->pSample;
	dwRemove = (dwEnd-dwStart+1)>>1;
	dwNewLen = pins->nLength - dwRemove;
	dwEnd = dwStart+dwRemove*2;
	pNewSample = NULL;
	if ((dwNewLen > 32) && (dwRemove)) pNewSample = CSoundFile::AllocateSample((dwNewLen+4)*smplsize);
	if (pNewSample)
	{
		UINT nCh = (pins->uFlags & CHN_STEREO) ? 2 : 1;
		for (UINT iCh=0; iCh<nCh; iCh++)
		{
			int len = dwRemove;
			int maxndx = pins->nLength;
			if (pins->uFlags & CHN_16BIT)
			{
				signed short *psrc = ((signed short *)pOriginal)+iCh;
				signed short *pdest = ((signed short *)pNewSample)+dwStart*nCh+iCh;
				for (int i=0; i<len; i++)
				{
					int accum = 0x100 + ((int)psrc[(dwStart+i*2)*nCh] << 8);
					for (int j=0; j<16; j++)
					{
						int ndx = dwStart + (i + j)*2 - 15;
						if (ndx < 0) ndx = 0;
						if (ndx > maxndx) ndx = maxndx;
						accum += (psrc[ndx*nCh] * gSinc2x16Odd[j] + 0x40) >> 7;
					}
					accum >>= 9;
					if (accum > 0x7fff) accum = 0x7fff;
					if (accum < -0x8000) accum = -0x8000;
					pdest[0] = (short int)accum;
					pdest += nCh;
				}
			} else
			{
				signed char *psrc = ((signed char *)pOriginal)+iCh;
				signed char *pdest = ((signed char *)pNewSample)+dwStart*nCh+iCh;
				for (int i=0; i<len; i++)
				{
					int accum = 0x100 + ((int)psrc[(dwStart+i*2)*nCh] << 8);
					for (int j=0; j<16; j++)
					{
						int ndx = dwStart + (i + j)*2 - 15;
						if (ndx < 0) ndx = 0;
						if (ndx > maxndx) ndx = maxndx;
						accum += (psrc[ndx*nCh] * gSinc2x16Odd[j] + 0x40) >> 7;
					}
					accum >>= 9;
					if (accum > 0x7f) accum = 0x7f;
					if (accum < -0x80) accum = -0x80;
					pdest[0] = (signed char)accum;
					pdest += nCh;
				}
			}
		}
		if (dwStart > 0) memcpy(pNewSample, pOriginal, dwStart*smplsize);
		if (dwEnd < pins->nLength) memcpy(((LPSTR)pNewSample)+(dwStart+dwRemove)*smplsize, ((LPSTR)pOriginal)+((dwStart+dwRemove*2)*smplsize), (pins->nLength-dwEnd)*smplsize);
		if (pins->nLoopStart >= dwEnd) pins->nLoopStart -= dwRemove; else
		if (pins->nLoopStart > dwStart) pins->nLoopStart -= (pins->nLoopStart - dwStart)/2;
		if (pins->nLoopEnd >= dwEnd) pins->nLoopEnd -= dwRemove; else
		if (pins->nLoopEnd > dwStart) pins->nLoopEnd -= (pins->nLoopEnd - dwStart)/2;
		if (pins->nLoopEnd > dwNewLen) pins->nLoopEnd = dwNewLen;
		if (pins->nSustainStart >= dwEnd) pins->nSustainStart -= dwRemove; else
		if (pins->nSustainStart > dwStart) pins->nSustainStart -= (pins->nSustainStart - dwStart)/2;
		if (pins->nSustainEnd >= dwEnd) pins->nSustainEnd -= dwRemove; else
		if (pins->nSustainEnd > dwStart) pins->nSustainEnd -= (pins->nSustainEnd - dwStart)/2;
		if (pins->nSustainEnd > dwNewLen) pins->nSustainEnd = dwNewLen;
		BEGIN_CRITICAL();
		for (UINT iFix=0; iFix<MAX_CHANNELS; iFix++)
		{
			if ((PVOID)m_pSndFile->Chn[iFix].pSample == pOriginal)
			{
				m_pSndFile->Chn[iFix].pSample = (LPSTR)pNewSample;
				m_pSndFile->Chn[iFix].pCurrentSample = NULL;
				m_pSndFile->Chn[iFix].nLength = 0;
			}
		}
		if (viewstate.dwEndSel <= viewstate.dwBeginSel)
		{
			if (pins->nC4Speed > 2000) pins->nC4Speed /= 2;
			if (pins->RelativeTone > -84) pins->RelativeTone -= 12;
		}
		pins->nLength = dwNewLen;
		pins->pSample = (LPSTR)pNewSample;
		CSoundFile::FreeSample(pOriginal);
		END_CRITICAL();
		m_pModDoc->AdjustEndOfSample(m_nSample);
		if (viewstate.dwEndSel > viewstate.dwBeginSel)
		{
			viewstate.dwBeginSel = dwStart;
			viewstate.dwEndSel = dwStart + dwRemove;
			SendViewMessage(VIEWMSG_LOADSTATE, (LPARAM)&viewstate);
		}
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA | HINT_SAMPLEINFO, NULL);
		m_pModDoc->SetModified();
	}
	EndWaitCursor();
	SwitchToView();
}


void CCtrlSamples::OnReverse()
//----------------------------
{
	SAMPLEVIEWSTATE viewstate;
	MODINSTRUMENT *pins;
	DWORD dwBeginSel, dwEndSel;
	LPVOID pSample;
	UINT rlen, smplsize;
	
	memset(&viewstate, 0, sizeof(viewstate));
	SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)&viewstate);
	if ((!m_pSndFile) || (!m_pSndFile->Ins[m_nSample].pSample)) return;
	BeginWaitCursor();
	dwBeginSel = viewstate.dwBeginSel;
	dwEndSel = viewstate.dwEndSel;
	pins = &m_pSndFile->Ins[m_nSample];
	rlen = pins->nLength;
	pSample = pins->pSample;
	smplsize = (pins->uFlags & CHN_16BIT) ? 2 : 1;
	if (pins->uFlags & CHN_STEREO) smplsize *= 2;
	if ((dwEndSel > dwBeginSel) && (dwEndSel <= rlen))
	{
		rlen = dwEndSel - dwBeginSel;
		pSample = ((LPBYTE)pins->pSample) +  dwBeginSel * smplsize;
	}
	if (rlen >= 2)
	{
		
		if (smplsize == 4)
		{
			UINT len = rlen / 2;
			UINT max = rlen - 1;
			int *p = (int *)pSample;

			for (UINT i=0; i<len; i++)
			{
				int tmp = p[max-i];
				p[max-i] = p[i];
				p[i] = tmp;
			}
		} else
		if (smplsize == 2)
		{
			UINT len = rlen / 2;
			UINT max = rlen - 1;
			signed short *p = (signed short *)pSample;

			for (UINT i=0; i<len; i++)
			{
				signed short tmp = p[max-i];
				p[max-i] = p[i];
				p[i] = tmp;
			}
		} else
		{
			UINT len = rlen / 2;
			UINT max = rlen - 1;
			signed char *p = (signed char *)pSample;

			for (UINT i=0; i<len; i++)
			{
				signed char tmp = p[max-i];
				p[max-i] = p[i];
				p[i] = tmp;
			}
		}
		m_pModDoc->AdjustEndOfSample(m_nSample);
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, NULL);
		m_pModDoc->SetModified();
	}
	EndWaitCursor();
	SwitchToView();
}


void CCtrlSamples::OnSilence()
//----------------------------
{
	SAMPLEVIEWSTATE viewstate;
	MODINSTRUMENT *pins;
	DWORD dwBeginSel, dwEndSel;
	
	memset(&viewstate, 0, sizeof(viewstate));
	SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)&viewstate);
	if ((!m_pSndFile) || (!m_pSndFile->Ins[m_nSample].pSample)) return;
	BeginWaitCursor();
	dwBeginSel = viewstate.dwBeginSel;
	dwEndSel = viewstate.dwEndSel;
	pins = &m_pSndFile->Ins[m_nSample];
	if (dwEndSel > pins->nLength) dwEndSel = pins->nLength;
	if (dwEndSel > dwBeginSel+1)
	{
		int len = dwEndSel - dwBeginSel;
		if (pins->uFlags & CHN_STEREO)
		{
			int smplsize = (pins->uFlags & CHN_16BIT) ? 4 : 2;
			signed char *p = ((signed char *)pins->pSample) + dwBeginSel*smplsize;
			memset(p, 0, len*smplsize);
		} else
		if (pins->uFlags & CHN_16BIT)
		{
			short int *p = ((short int *)pins->pSample) + dwBeginSel;
			int dest = (dwEndSel < pins->nLength) ? p[len-1] : 0;
			int base = (dwBeginSel) ? p[0] : 0;
			int delta = dest - base;
			for (int i=0; i<len; i++)
			{
				int n = base + (int)(((LONGLONG)delta * (LONGLONG)i) / (len-1));
				p[i] = (signed short)n;
			}
		} else
		{
			signed char *p = ((signed char *)pins->pSample) + dwBeginSel;
			int dest = (dwEndSel < pins->nLength) ? p[len-1] : 0;
			int base = (dwBeginSel) ? p[0] : 0;
			int delta = dest - base;
			for (int i=0; i<len; i++)
			{
				int n = base + (delta * i) / (len-1);
				p[i] = (signed char)n;
			}
		}
		m_pModDoc->AdjustEndOfSample(m_nSample);
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, NULL);
		m_pModDoc->SetModified();
	}
	EndWaitCursor();
	SwitchToView();
}


void CCtrlSamples::OnPrevInstrument()
//-----------------------------------
{
	if (m_pSndFile)
	{
		if (m_nSample > 1)
			SetCurrentSample(m_nSample-1);
		else
			SetCurrentSample(m_pSndFile->m_nSamples);
	}
}


void CCtrlSamples::OnNextInstrument()
//-----------------------------------
{
	if (m_pSndFile)
	{
		if (m_nSample < m_pSndFile->m_nSamples)
			SetCurrentSample(m_nSample+1);
		else
			SetCurrentSample(1);
	}
}


void CCtrlSamples::OnNameChanged()
//--------------------------------
{
	CHAR s[64];

	if ((IsLocked()) || (!m_pSndFile) || (!m_nSample)) return;
	s[0] = 0;
	m_EditName.GetWindowText(s, sizeof(s));
	for (UINT i=strlen(s); i<32; i++) s[i] = 0;
	s[31] = 0;
	if (strncmp(s, m_pSndFile->m_szNames[m_nSample], 32))
	{
		memcpy(m_pSndFile->m_szNames[m_nSample], s, 32);
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | (HINT_SMPNAMES|HINT_SAMPLEINFO), this);
		m_pModDoc->SetModified();
	}
}


void CCtrlSamples::OnFileNameChanged()
//------------------------------------
{
	CHAR s[32];
	
	if ((IsLocked()) || (!m_pSndFile)) return;
	s[0] = 0;
	m_EditFileName.GetWindowText(s, sizeof(s));
	s[21] = 0;
	for (UINT i=strlen(s); i<22; i++) s[i] = 0;
	if (strncmp(s, m_pSndFile->Ins[m_nSample].name, 22))
	{
		memcpy(m_pSndFile->Ins[m_nSample].name, s, 22);
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEINFO, this);
		if (m_pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)) m_pModDoc->SetModified();
	}
}


void CCtrlSamples::OnVolumeChanged()
//----------------------------------
{
	if (IsLocked()) return;
	int nVol = GetDlgItemInt(IDC_EDIT7);
	if (nVol < 0) nVol = 0;
	if (nVol > 64) nVol = 64;
	nVol <<= 2;
	if (nVol != m_pSndFile->Ins[m_nSample].nVolume)
	{
		m_pSndFile->Ins[m_nSample].nVolume = nVol;
		m_pModDoc->SetModified();
	}
}


void CCtrlSamples::OnGlobalVolChanged()
//-------------------------------------
{
	if (IsLocked()) return;
	int nVol = GetDlgItemInt(IDC_EDIT8);
	if (nVol < 0) nVol = 0;
	if (nVol > 64) nVol = 64;
	if (nVol != m_pSndFile->Ins[m_nSample].nGlobalVol)
	{
		m_pSndFile->Ins[m_nSample].nGlobalVol = nVol;
		m_pModDoc->SetModified();
	}
}


void CCtrlSamples::OnSetPanningChanged()
//--------------------------------------
{
	if (IsLocked()) return;
	BOOL b = FALSE;
	if (m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT))
	{
		b = IsDlgButtonChecked(IDC_CHECK1);
	}
	if (b)
	{
		if (!(m_pSndFile->Ins[m_nSample].uFlags & CHN_PANNING))
		{
			m_pSndFile->Ins[m_nSample].uFlags |= CHN_PANNING;
			if (m_pSndFile->m_nType == MOD_TYPE_IT) m_pModDoc->SetModified();
		}
	} else
	{
		if (m_pSndFile->Ins[m_nSample].uFlags & CHN_PANNING)
		{
			m_pSndFile->Ins[m_nSample].uFlags &= ~CHN_PANNING;
			if (m_pSndFile->m_nType == MOD_TYPE_IT) m_pModDoc->SetModified();
		}
	}
}


void CCtrlSamples::OnPanningChanged()
//-----------------------------------
{
	if (IsLocked()) return;
	int nPan = GetDlgItemInt(IDC_EDIT9);
	if (nPan < 0) nPan = 0;
	if (nPan > 256) nPan = 256;
	if (nPan != m_pSndFile->Ins[m_nSample].nPan)
	{
		m_pSndFile->Ins[m_nSample].nPan = nPan;
		if (m_pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) m_pModDoc->SetModified();
	}
}


void CCtrlSamples::OnFineTuneChanged()
//------------------------------------
{
	if (IsLocked()) return;
	int n = GetDlgItemInt(IDC_EDIT5);
	if (m_pSndFile->m_nType & (MOD_TYPE_IT|MOD_TYPE_S3M))
	{
		if ((n >= 2000) && (n <= 256000) && (n != (int)m_pSndFile->Ins[m_nSample].nC4Speed))
		{
			m_pSndFile->Ins[m_nSample].nC4Speed = n;
			int transp = CSoundFile::FrequencyToTranspose(n) >> 7;
			int basenote = 60 - transp;
			if (basenote < BASENOTE_MIN) basenote = BASENOTE_MIN;
			if (basenote >= BASENOTE_MAX) basenote = BASENOTE_MAX-1;
			basenote -= BASENOTE_MIN;
			if (basenote != m_CbnBaseNote.GetCurSel())
			{
				LockControls();
				m_CbnBaseNote.SetCurSel(basenote);
				UnlockControls();
			}
		}
	} else
	{
		if ((n >= -128) && (n <= 127))
			m_pSndFile->Ins[m_nSample].nFineTune = n;
	}
}


void CCtrlSamples::OnBaseNoteChanged()
//-------------------------------------
{
	if (IsLocked()) return;
	int n = 60 - (m_CbnBaseNote.GetCurSel() + BASENOTE_MIN);
	if (m_pSndFile->m_nType & (MOD_TYPE_IT|MOD_TYPE_S3M))
	{
		LONG ft = CSoundFile::FrequencyToTranspose(m_pSndFile->Ins[m_nSample].nC4Speed) & 0x7f;
		n = CSoundFile::TransposeToFrequency(n, ft);
		if ((n >= 500) && (n <= 256000) && (n != (int)m_pSndFile->Ins[m_nSample].nC4Speed))
		{
			CHAR s[32];
			m_pSndFile->Ins[m_nSample].nC4Speed = n;
			wsprintf(s, "%lu", n);
			LockControls();
			m_EditFineTune.SetWindowText(s);
			UnlockControls();
		}
	} else
	{
		if ((n >= -128) && (n < 128))
		{
			m_pSndFile->Ins[m_nSample].RelativeTone = n;
		}
	}
}


void CCtrlSamples::OnVibTypeChanged()
//-----------------------------------
{
	if (IsLocked()) return;
	int n = m_ComboAutoVib.GetCurSel();
	if (n >= 0) m_pSndFile->Ins[m_nSample].nVibType = (BYTE)n;
}


void CCtrlSamples::OnVibDepthChanged()
//------------------------------------
{
	if (IsLocked()) return;
	int lmin = 0, lmax = 0;
	m_SpinVibDepth.GetRange(lmin, lmax);
	int n = GetDlgItemInt(IDC_EDIT15);
	if ((n >= lmin) && (n <= lmax))
		m_pSndFile->Ins[m_nSample].nVibDepth = n;
}


void CCtrlSamples::OnVibSweepChanged()
//------------------------------------
{
	if (IsLocked()) return;
	int lmin = 0, lmax = 0;
	m_SpinVibSweep.GetRange(lmin, lmax);
	int n = GetDlgItemInt(IDC_EDIT14);
	if ((n >= lmin) && (n <= lmax))
		m_pSndFile->Ins[m_nSample].nVibSweep = n;
}


void CCtrlSamples::OnVibRateChanged()
//-----------------------------------
{
	if (IsLocked()) return;
	int lmin = 0, lmax = 0;
	m_SpinVibRate.GetRange(lmin, lmax);
	int n = GetDlgItemInt(IDC_EDIT16);
	if ((n >= lmin) && (n <= lmax))
		m_pSndFile->Ins[m_nSample].nVibRate = n;
}


void CCtrlSamples::OnLoopTypeChanged()
//------------------------------------
{
	if ((IsLocked()) || (!m_pSndFile)) return;
	int n = m_ComboLoopType.GetCurSel();
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	switch(n)
	{
	case 0:	// Off
		pins->uFlags &= ~(CHN_LOOP|CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
		break;
	case 1:	// On
		pins->uFlags &= ~CHN_PINGPONGLOOP;
		pins->uFlags |= CHN_LOOP;
		break;
	case 2:	// PingPong
		pins->uFlags |= CHN_LOOP|CHN_PINGPONGLOOP;
		break;
	}
	m_pModDoc->AdjustEndOfSample(m_nSample);
}


void CCtrlSamples::OnLoopStartChanged()
//--------------------------------------
{
	if ((IsLocked()) || (!m_pSndFile)) return;
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	LONG n = GetDlgItemInt(IDC_EDIT1);
	if ((n >= 0) && (n < (LONG)pins->nLength) && ((n < (LONG)pins->nLoopEnd) || (!(pins->uFlags & CHN_LOOP))))
	{
		pins->nLoopStart = n;
		m_pModDoc->AdjustEndOfSample(m_nSample);
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, this);
	}
}


void CCtrlSamples::OnLoopEndChanged()
//------------------------------------
{
	if ((IsLocked()) || (!m_pSndFile)) return;
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	LONG n = GetDlgItemInt(IDC_EDIT2);
	if ((n >= 0) && (n <= (LONG)pins->nLength) && ((n > (LONG)pins->nLoopStart) || (!(pins->uFlags & CHN_LOOP))))
	{
		pins->nLoopEnd = n;
		m_pModDoc->AdjustEndOfSample(m_nSample);
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, this);
	}
}


void CCtrlSamples::OnSustainTypeChanged()
//---------------------------------------
{
	if ((IsLocked()) || (!m_pSndFile)) return;
	int n = m_ComboSustainType.GetCurSel();
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	switch(n)
	{
	case 0:	// Off
		pins->uFlags &= ~(CHN_SUSTAINLOOP|CHN_PINGPONGSUSTAIN|CHN_PINGPONGFLAG);
		break;
	case 1:	// On
		pins->uFlags &= ~CHN_PINGPONGSUSTAIN;
		pins->uFlags |= CHN_SUSTAINLOOP;
		break;
	case 2:	// PingPong
		pins->uFlags |= CHN_SUSTAINLOOP|CHN_PINGPONGSUSTAIN;
		break;
	}
}


void CCtrlSamples::OnSustainStartChanged()
//----------------------------------------
{
	if ((IsLocked()) || (!m_pSndFile)) return;
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	LONG n = GetDlgItemInt(IDC_EDIT3);
	if ((n >= 0) && (n <= (LONG)pins->nLength)
	 && ((n < (LONG)pins->nSustainEnd) || (!(pins->uFlags & CHN_SUSTAINLOOP))))
	{
		pins->nSustainStart = n;
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, this);
	}
}


void CCtrlSamples::OnSustainEndChanged()
//--------------------------------------
{
	if ((IsLocked()) || (!m_pSndFile)) return;
	MODINSTRUMENT *pins = &m_pSndFile->Ins[m_nSample];
	LONG n = GetDlgItemInt(IDC_EDIT4);
	if ((n >= 0) && (n <= (LONG)pins->nLength)
	 && ((n > (LONG)pins->nSustainStart) || (!(pins->uFlags & CHN_SUSTAINLOOP))))
	{
		pins->nSustainEnd = n;
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, this);
	}
}


#define SMPLOOP_ACCURACY	7	// 5%
#define BIDILOOP_ACCURACY	2	// 5%


BOOL MPT_LoopCheck(int sstart0, int sstart1, int send0, int send1)
{
	int dse0 = send0 - sstart0;
	if ((dse0 < -SMPLOOP_ACCURACY) || (dse0 > SMPLOOP_ACCURACY)) return FALSE;
	int dse1 = send1 - sstart1;
	if ((dse1 < -SMPLOOP_ACCURACY) || (dse1 > SMPLOOP_ACCURACY)) return FALSE;
	int dstart = sstart1 - sstart0;
	int dend = send1 - send0;
	if (!dstart) dstart = dend >> 7;
	if (!dend) dend = dstart >> 7;
	if ((dstart ^ dend) < 0) return FALSE;
	int delta = dend - dstart;
	return ((delta > -SMPLOOP_ACCURACY) && (delta < SMPLOOP_ACCURACY)) ? TRUE : FALSE;
}


BOOL MPT_BidiEndCheck(int spos0, int spos1, int spos2)
{
	int delta0 = spos1 - spos0;
	int delta1 = spos2 - spos1;
	int delta2 = spos2 - spos0;
	if (!delta0) delta0 = delta1 >> 7;
	if (!delta1) delta1 = delta0 >> 7;
	if ((delta1 ^ delta0) < 0) return FALSE;
	return ((delta0 >= -1) && (delta0 <= 0) && (delta1 >= -1) && (delta1 <= 0) && (delta2 >= -1) && (delta2 <= 0));
}


BOOL MPT_BidiStartCheck(int spos0, int spos1, int spos2)
{
	int delta1 = spos1 - spos0;
	int delta0 = spos2 - spos1;
	int delta2 = spos2 - spos0;
	if (!delta0) delta0 = delta1 >> 7;
	if (!delta1) delta1 = delta0 >> 7;
	if ((delta1 ^ delta0) < 0) return FALSE;
	return ((delta0 >= -1) && (delta0 <= 0) && (delta1 > -1) && (delta1 <= 0) && (delta2 >= -1) && (delta2 <= 0));
}



void CCtrlSamples::OnVScroll(UINT nCode, UINT, CScrollBar *)
//----------------------------------------------------------
{
	CHAR s[256];
	if ((IsLocked()) || (!m_pSndFile)) return;
	UINT nsample = m_nSample, pinc = 1;
	MODINSTRUMENT *pins = &m_pSndFile->Ins[nsample];
	LPSTR pSample = pins->pSample;
	short int pos;
	BOOL bRedraw = FALSE;
	
	LockControls();
	if ((!pins->nLength) || (!pSample)) goto NoSample;
	if (pins->uFlags & CHN_16BIT)
	{
		pSample++;
		pinc *= 2;
	}
	if (pins->uFlags & CHN_STEREO) pinc *= 2;
	// Loop Start
	if ((pos = (short int)m_SpinLoopStart.GetPos()) != 0)
	{
		BOOL bOk = FALSE;
		LPSTR p = pSample+pins->nLoopStart*pinc;
		int find0 = (int)pSample[pins->nLoopEnd*pinc-pinc];
		int find1 = (int)pSample[pins->nLoopEnd*pinc];
		// Find Next LoopStart Point
		if (pos > 0)
		{
			for (UINT i=pins->nLoopStart+1; i+16<pins->nLoopEnd; i++)
			{
				p += pinc;
				bOk = (pins->uFlags & CHN_PINGPONGLOOP) ? MPT_BidiStartCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nLoopStart = i;
					break;
				}
			}
		} else
		// Find Prev LoopStart Point
		{
			for (UINT i=pins->nLoopStart; i; )
			{
				i--;
				p -= pinc;
				bOk = (pins->uFlags & CHN_PINGPONGLOOP) ? MPT_BidiStartCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nLoopStart = i;
					break;
				}
			}
		}
		if (bOk)
		{
			wsprintf(s, "%u", pins->nLoopStart);
			m_EditLoopStart.SetWindowText(s);
			m_pModDoc->AdjustEndOfSample(m_nSample);
			bRedraw = TRUE;
		}
		m_SpinLoopStart.SetPos(0);
	}
	// Loop End
	pos = (short int)m_SpinLoopEnd.GetPos();
	if ((pos) && (pins->nLoopEnd))
	{
		BOOL bOk = FALSE;
		LPSTR p = pSample+pins->nLoopEnd*pinc;
		int find0 = (int)pSample[pins->nLoopStart*pinc];
		int find1 = (int)pSample[pins->nLoopStart*pinc+pinc];
		// Find Next LoopEnd Point
		if (pos > 0)
		{
			for (UINT i=pins->nLoopEnd+1; i<=pins->nLength; i++, p+=pinc)
			{
				bOk = (pins->uFlags & CHN_PINGPONGLOOP) ? MPT_BidiEndCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nLoopEnd = i;
					break;
				}
			}
		} else
		// Find Prev LoopEnd Point
		{
			for (UINT i=pins->nLoopEnd; i>pins->nLoopStart+16; )
			{
				i--;
				p -= pinc;
				bOk = (pins->uFlags & CHN_PINGPONGLOOP) ? MPT_BidiEndCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nLoopEnd = i;
					break;
				}
			}
		}
		if (bOk)
		{
			wsprintf(s, "%u", pins->nLoopEnd);
			m_EditLoopEnd.SetWindowText(s);
			m_pModDoc->AdjustEndOfSample(m_nSample);
			bRedraw = TRUE;
		}
		m_SpinLoopEnd.SetPos(0);
	}
	// Sustain Loop Start
	pos = (short int)m_SpinSustainStart.GetPos();
	if ((pos) && (pins->nSustainEnd))
	{
		BOOL bOk = FALSE;
		LPSTR p = pSample+pins->nSustainStart*pinc;
		int find0 = (int)pSample[pins->nSustainEnd*pinc-pinc];
		int find1 = (int)pSample[pins->nSustainEnd*pinc];
		// Find Next Sustain LoopStart Point
		if (pos > 0)
		{
			for (UINT i=pins->nSustainStart+1; i+16<pins->nSustainEnd; i++)
			{
				p += pinc;
				bOk = (pins->uFlags & CHN_PINGPONGSUSTAIN) ? MPT_BidiStartCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nSustainStart = i;
					break;
				}
			}
		} else
		// Find Prev Sustain LoopStart Point
		{
			for (UINT i=pins->nSustainStart; i; )
			{
				i--;
				p -= pinc;
				bOk = (pins->uFlags & CHN_PINGPONGSUSTAIN) ? MPT_BidiStartCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nSustainStart = i;
					break;
				}
			}
		}
		if (bOk)
		{
			wsprintf(s, "%u", pins->nSustainStart);
			m_EditSustainStart.SetWindowText(s);
			bRedraw = TRUE;
		}
		m_SpinSustainStart.SetPos(0);
	}
	// Sustain Loop End
	pos = (short int)m_SpinSustainEnd.GetPos();
	if (pos)
	{
		BOOL bOk = FALSE;
		LPSTR p = pSample+pins->nSustainEnd*pinc;
		int find0 = (int)pSample[pins->nSustainStart*pinc];
		int find1 = (int)pSample[pins->nSustainStart*pinc+pinc];
		// Find Next LoopEnd Point
		if (pos > 0)
		{
			for (UINT i=pins->nSustainEnd+1; i+1<pins->nLength; i++, p+=pinc)
			{
				bOk = (pins->uFlags & CHN_PINGPONGSUSTAIN) ? MPT_BidiEndCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nSustainEnd = i;
					break;
				}
			}
		} else
		// Find Prev LoopEnd Point
		{
			for (UINT i=pins->nSustainEnd; i>pins->nSustainStart+16; )
			{
				i--;
				p -= pinc;
				bOk = (pins->uFlags & CHN_PINGPONGSUSTAIN) ? MPT_BidiEndCheck(p[0], p[pinc], p[pinc*2]) : MPT_LoopCheck(find0, find1, p[0], p[pinc]);
				if (bOk)
				{
					pins->nSustainEnd = i;
					break;
				}
			}
		}
		if (bOk)
		{
			wsprintf(s, "%u", pins->nSustainEnd);
			m_EditSustainEnd.SetWindowText(s);
			bRedraw = TRUE;
		}
		m_SpinSustainEnd.SetPos(0);
	}
NoSample:
	// FineTune / C-5 Speed
	if ((pos = (short int)m_SpinFineTune.GetPos()) != 0)
	{
		if (m_pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
		{
			LONG d = pins->nC4Speed;
			if (d < 1) d = 8363;
			d += (pos * 25);
			if (d > 96000) d = 96000;
			if (d < 2000) d = 2000;
			pins->nC4Speed = d;
			int transp = CSoundFile::FrequencyToTranspose(pins->nC4Speed) >> 7;
			int basenote = 60 - transp;
			if (basenote < BASENOTE_MIN) basenote = BASENOTE_MIN;
			if (basenote >= BASENOTE_MAX) basenote = BASENOTE_MAX-1;
			basenote -= BASENOTE_MIN;
			if (basenote != m_CbnBaseNote.GetCurSel()) m_CbnBaseNote.SetCurSel(basenote);
			wsprintf(s, "%lu", pins->nC4Speed);
			m_EditFineTune.SetWindowText(s);
		} else
		{
			LONG d = pins->nFineTune + pos;
			if (d < -128) d = -128;
			if (d > 127) d = 127;
			pins->nFineTune = d;
			wsprintf(s, "%d", d);
			m_EditFineTune.SetWindowText(s);
		}
		m_SpinFineTune.SetPos(0);
	}
	if ((nCode == SB_ENDSCROLL) || (nCode == SB_THUMBPOSITION)) SwitchToView();
	if (bRedraw)
	{
		m_pModDoc->UpdateAllViews(NULL, (m_nSample << 24) | HINT_SAMPLEDATA, this);
	}
	UnlockControls();
}





