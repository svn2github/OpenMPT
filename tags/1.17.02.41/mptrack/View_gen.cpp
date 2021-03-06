#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "moddoc.h"
#include "globals.h"
#include "ctrl_gen.h"
#include "view_gen.h"
#include "vstplug.h"
#include "EffectVis.h"
#include "movefxslotdialog.h"
#include "ChannelManagerDlg.h"
#include ".\view_gen.h"

#define ID_FXCOMMANDS_BASE	41000


IMPLEMENT_SERIAL(CViewGlobals, CFormView, 0)

BEGIN_MESSAGE_MAP(CViewGlobals, CFormView)
	//{{AFX_MSG_MAP(CViewGlobals)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()

// -> CODE#0015
// -> DESC="channels management dlg"
	ON_WM_ACTIVATE()
// -! NEW_FEATURE#0015
	ON_COMMAND(IDC_CHECK1,		OnMute1)
	ON_COMMAND(IDC_CHECK3,		OnMute2)
	ON_COMMAND(IDC_CHECK5,		OnMute3)
	ON_COMMAND(IDC_CHECK7,		OnMute4)
	ON_COMMAND(IDC_CHECK2,		OnSurround1)
	ON_COMMAND(IDC_CHECK4,		OnSurround2)
	ON_COMMAND(IDC_CHECK6,		OnSurround3)
	ON_COMMAND(IDC_CHECK8,		OnSurround4)
	ON_COMMAND(IDC_CHECK9,		OnMixModeChanged)
	ON_COMMAND(IDC_CHECK10,		OnBypassChanged)
	ON_COMMAND(IDC_CHECK11,		OnDryMixChanged)
	ON_COMMAND(IDC_BUTTON1,		OnSelectPlugin)
	ON_COMMAND(IDC_BUTTON2,		OnEditPlugin)
	ON_COMMAND(IDC_BUTTON3,		OnSetParameter)
	ON_COMMAND(IDC_BUTTON4,		OnNextPlugin)
	ON_COMMAND(IDC_BUTTON5,		OnPrevPlugin)
	ON_COMMAND(IDC_MOVEFXSLOT,  OnMovePlugToSlot)
	ON_COMMAND(IDC_INSERTFXSLOT,OnInsertSlot)
	ON_COMMAND(IDC_CLONEPLUG,   OnClonePlug)


// -> CODE#0002
// -> DESC="VST plugins presets"
	ON_COMMAND(IDC_BUTTON6,		OnLoadParam)
	ON_COMMAND(IDC_BUTTON8,		OnSaveParam)
// -! NEW_FEATURE#0002

// -> CODE#0014
// -> DESC="vst wet/dry slider"
	ON_COMMAND(IDC_BUTTON7,		OnSetWetDry)
// -! NEW_FEATURE#0014
	ON_EN_UPDATE(IDC_EDIT1,		OnEditVol1)
	ON_EN_UPDATE(IDC_EDIT3,		OnEditVol2)
	ON_EN_UPDATE(IDC_EDIT5,		OnEditVol3)
	ON_EN_UPDATE(IDC_EDIT7,		OnEditVol4)
	ON_EN_UPDATE(IDC_EDIT2,		OnEditPan1)
	ON_EN_UPDATE(IDC_EDIT4,		OnEditPan2)
	ON_EN_UPDATE(IDC_EDIT6,		OnEditPan3)
	ON_EN_UPDATE(IDC_EDIT8,		OnEditPan4)
	ON_EN_UPDATE(IDC_EDIT9,		OnEditName1)
	ON_EN_UPDATE(IDC_EDIT10,	OnEditName2)
	ON_EN_UPDATE(IDC_EDIT11,	OnEditName3)
	ON_EN_UPDATE(IDC_EDIT12,	OnEditName4)
	ON_EN_UPDATE(IDC_EDIT13,	OnPluginNameChanged)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnFx1Changed)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnFx2Changed)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnFx3Changed)
	ON_CBN_SELCHANGE(IDC_COMBO4, OnFx4Changed)
	ON_CBN_SELCHANGE(IDC_COMBO5, OnPluginChanged)
	ON_CBN_SELCHANGE(IDC_COMBO6, OnParamChanged)
	ON_CBN_SELCHANGE(IDC_COMBO7, OnOutputRoutingChanged)

// -> CODE#0002
// -> DESC="VST plugins presets"
	ON_CBN_SELCHANGE(IDC_COMBO8, OnProgramChanged)
// -! NEW_FEATURE#0002

// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
	ON_COMMAND(IDC_CHECK12,		 OnWetDryExpandChanged)
	ON_CBN_SELCHANGE(IDC_COMBO9, OnSpecialMixProcessingChanged)
// -! BEHAVIOUR_CHANGE#0028

	ON_COMMAND_RANGE(ID_FXCOMMANDS_BASE, ID_FXCOMMANDS_BASE+10, OnFxCommands)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCTRL1,	OnTabSelchange)
	ON_MESSAGE(WM_MOD_UNLOCKCONTROLS,		OnUnlockControls)
	ON_MESSAGE(WM_MOD_VIEWMSG,	OnModViewMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CViewGlobals::DoDataExchange(CDataExchange* pDX)
//---------------------------------------------------
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewGlobals)
	DDX_Control(pDX, IDC_TABCTRL1,	m_TabCtrl);
	DDX_Control(pDX, IDC_COMBO1,	m_CbnEffects[0]);
	DDX_Control(pDX, IDC_COMBO2,	m_CbnEffects[1]);
	DDX_Control(pDX, IDC_COMBO3,	m_CbnEffects[2]);
	DDX_Control(pDX, IDC_COMBO4,	m_CbnEffects[3]);
	DDX_Control(pDX, IDC_COMBO5,	m_CbnPlugin);
	DDX_Control(pDX, IDC_COMBO6,	m_CbnParam);
	DDX_Control(pDX, IDC_COMBO7,	m_CbnOutput);

// -> CODE#0002
// -> DESC="VST plugins presets"
	DDX_Control(pDX, IDC_COMBO8,	m_CbnPreset);
// -! NEW_FEATURE#0002

// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
	DDX_Control(pDX, IDC_COMBO9,	m_CbnSpecialMixProcessing);
	DDX_Control(pDX, IDC_SPIN10,	m_SpinMixGain);					// update#02
// -! BEHAVIOUR_CHANGE#0028

	DDX_Control(pDX, IDC_SLIDER1,	m_sbVolume[0]);
	DDX_Control(pDX, IDC_SLIDER2,	m_sbPan[0]);
	DDX_Control(pDX, IDC_SLIDER3,	m_sbVolume[1]);
	DDX_Control(pDX, IDC_SLIDER4,	m_sbPan[1]);
	DDX_Control(pDX, IDC_SLIDER5,	m_sbVolume[2]);
	DDX_Control(pDX, IDC_SLIDER6,	m_sbPan[2]);
	DDX_Control(pDX, IDC_SLIDER7,	m_sbVolume[3]);
	DDX_Control(pDX, IDC_SLIDER8,	m_sbPan[3]);
	DDX_Control(pDX, IDC_SLIDER9,	m_sbValue);
	DDX_Control(pDX, IDC_SLIDER10,  m_sbDryRatio);	//rewbs.VSTdrywet
	DDX_Control(pDX, IDC_SPIN1,		m_spinVolume[0]);
	DDX_Control(pDX, IDC_SPIN2,		m_spinPan[0]);
	DDX_Control(pDX, IDC_SPIN3,		m_spinVolume[1]);
	DDX_Control(pDX, IDC_SPIN4,		m_spinPan[1]);
	DDX_Control(pDX, IDC_SPIN5,		m_spinVolume[2]);
	DDX_Control(pDX, IDC_SPIN6,		m_spinPan[2]);
	DDX_Control(pDX, IDC_SPIN7,		m_spinVolume[3]);
	DDX_Control(pDX, IDC_SPIN8,		m_spinPan[3]);
	DDX_Control(pDX, IDC_BUTTON1,	m_BtnSelect);
	DDX_Control(pDX, IDC_BUTTON2,	m_BtnEdit);
	//}}AFX_DATA_MAP
}

void CViewGlobals::OnInitialUpdate()
//----------------------------------
{
	CChildFrame *pFrame = (CChildFrame *)GetParentFrame();
	int nMapMode = MM_TEXT;
	SIZE sizeTotal, sizePage, sizeLine;

	m_nActiveTab = -1;
	m_nCurrentPlugin = 0;
	m_nCurrentParam = 0;
// -> CODE#0002
// -> DESC="VST plugins presets"
	m_nCurrentPreset = 0;
// -! NEW_FEATURE#0002
	CFormView::OnInitialUpdate();

	if (pFrame)
	{
		GENERALVIEWSTATE *pState = pFrame->GetGeneralViewState();
		if (pState->cbStruct == sizeof(GENERALVIEWSTATE))
		{
			m_TabCtrl.SetCurSel(pState->nTab);
			m_nActiveTab = pState->nTab;
			m_nCurrentPlugin = pState->nPlugin;
			m_nCurrentParam = pState->nParam;
		}
	}
	GetDeviceScrollSizes(nMapMode, sizeTotal, sizePage, sizeLine);
	m_rcClient.SetRect(0, 0, sizeTotal.cx, sizeTotal.cy);
	RecalcLayout();
	// Initializing scroll ranges
	for (int ichn=0; ichn<4; ichn++)
	{
		// Volume Slider
		m_sbVolume[ichn].SetRange(0, 64);
		m_sbVolume[ichn].SetTicFreq(8);
		// Pan Slider
		m_sbPan[ichn].SetRange(0, 64);
		m_sbPan[ichn].SetTicFreq(8);
		// Volume Spin
		m_spinVolume[ichn].SetRange(0, 64);
		// Pan Spin
		m_spinPan[ichn].SetRange(0, 256);
	}
	m_sbValue.SetPos(0);
	m_sbValue.SetRange(0, 100);

	m_sbValue.SetPos(0);			// rewbs.dryRatio 20040122
	m_sbValue.SetRange(0, 100);		// rewbs.dryRatio 20040122

// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
	m_CbnSpecialMixProcessing.AddString("Default");
	m_CbnSpecialMixProcessing.AddString("Wet subtract");
	m_CbnSpecialMixProcessing.AddString("Dry subtract");
	m_CbnSpecialMixProcessing.AddString("Mix subtract");
	m_CbnSpecialMixProcessing.AddString("Middle subtract");
	m_CbnSpecialMixProcessing.AddString("LR balance");
	m_SpinMixGain.SetRange(0,80);		// update#02
	m_SpinMixGain.SetPos(10);			// update#02
	SetDlgItemText(IDC_STATIC2, "Gain: x 1.0");	// update#02
// -! BEHAVIOUR_CHANGE#0028

	UpdateView(HINT_MODTYPE);
	OnParamChanged();
// -> CODE#0014
// -> DESC="vst wet/dry slider"
	//OnWetDryChanged();
// -! NEW_FEATURE#0014
	m_nLockCount = 0;

	
}


VOID CViewGlobals::OnDestroy()
//----------------------------
{
	CChildFrame *pFrame = (CChildFrame *)GetParentFrame();
	if (pFrame)
	{
		GENERALVIEWSTATE *pState = pFrame->GetGeneralViewState();
		pState->cbStruct = sizeof(GENERALVIEWSTATE);
		pState->nTab = m_nActiveTab;
		pState->nPlugin = m_nCurrentPlugin;
		pState->nParam = m_nCurrentParam;
	}
	CFormView::OnDestroy();
}


// -> CODE#0015
// -> DESC="channels management dlg"
void CViewGlobals::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);

	CMainFrame * pMainFrm = CMainFrame::GetMainFrame();
	BOOL activeDoc = pMainFrm ? pMainFrm->GetActiveDoc() == GetDocument() : FALSE;

	if(activeDoc && CChannelManagerDlg::sharedInstance(FALSE) && CChannelManagerDlg::sharedInstance()->IsDisplayed())
		CChannelManagerDlg::sharedInstance()->SetDocument((void*)this);
}
// -! NEW_FEATURE#0015


void CViewGlobals::RecalcLayout()
//-------------------------------
{
	if (m_TabCtrl.m_hWnd != NULL)
	{
		CRect rect;
		GetClientRect(&rect);
		if (rect.right < m_rcClient.right) rect.right = m_rcClient.right;
		if (rect.bottom < m_rcClient.bottom) rect.bottom = m_rcClient.bottom;
		m_TabCtrl.SetWindowPos(NULL, 0,0, rect.right, rect.bottom, SWP_NOZORDER|SWP_NOMOVE);
	}
}


int CViewGlobals::GetDlgItemIntEx(UINT nID)
//-----------------------------------------
{
	CHAR s[80];
	s[0] = 0;
	GetDlgItemText(nID, s, sizeof(s));
	if ((s[0] < '0') || (s[0] > '9')) return -1;
	return atoi(s);
}


void CViewGlobals::OnUpdate(CView* pView, LPARAM lHint, CObject*pHint)
//--------------------------------------------------------------------
{
	if (((lHint & 0xFFFF) == HINT_MODCHANNELS) && ((lHint >> 24) != m_nActiveTab)) return;
	if (pView != this) UpdateView(lHint, pHint);
}


void CViewGlobals::OnSize(UINT nType, int cx, int cy)
//---------------------------------------------------
{
	CFormView::OnSize(nType, cx, cy);
	if (((nType == SIZE_RESTORED) || (nType == SIZE_MAXIMIZED)) && (cx > 0) && (cy > 0) && (m_hWnd))
	{
		RecalcLayout();
	}
}


void CViewGlobals::UpdateView(DWORD dwHintMask, CObject *)
//--------------------------------------------------------
{
	CHAR s[128];
	TC_ITEM tci;
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	int nTabCount, nTabIndex;
	
	if (!pModDoc) return;
	if (!(dwHintMask & (HINT_MODTYPE|HINT_MODCHANNELS|HINT_MIXPLUGINS))) return;
	pSndFile = pModDoc->GetSoundFile();
	nTabCount = (pSndFile->m_nChannels + 3) / 4;
	if (nTabCount != m_TabCtrl.GetItemCount())
	{
		UINT nOldSel = m_TabCtrl.GetCurSel();
		if (!m_TabCtrl.GetItemCount()) nOldSel = m_nActiveTab;
		m_TabCtrl.SetRedraw(FALSE);
		m_TabCtrl.DeleteAllItems();
		for (int iItem=0; iItem<nTabCount; iItem++)
		{
			wsprintf(s, "%d - %d", iItem * 4 + 1, iItem * 4 + 4);
			tci.mask = TCIF_TEXT | TCIF_PARAM;
			tci.pszText = s;
			tci.lParam = iItem * 4;
			m_TabCtrl.InsertItem(iItem, &tci);
		}
		if (nOldSel >= (UINT)nTabCount) nOldSel = 0;
		m_TabCtrl.SetCurSel(nOldSel);
		m_TabCtrl.SetRedraw(TRUE);
		InvalidateRect(NULL, FALSE);
	}
	nTabIndex = m_TabCtrl.GetCurSel();
	if ((nTabIndex < 0) || (nTabIndex >= nTabCount)) return; // ???
	if ((m_nActiveTab != nTabIndex) || (dwHintMask & (HINT_MODTYPE|HINT_MODCHANNELS)))
	{
		LockControls();
		m_nActiveTab = nTabIndex;
		for (int ichn=0; ichn<4; ichn++)
		{
			UINT nChn = nTabIndex*4+ichn;
			BOOL bEnable = (nChn < pSndFile->m_nChannels) ? TRUE : FALSE;
			// Text
			s[0] = 0;
			if (bEnable) wsprintf(s, "Channel %d", nChn+1);
			SetDlgItemText(IDC_TEXT1+ichn, s);
			// Mute
			CheckDlgButton(IDC_CHECK1+ichn*2, (pSndFile->ChnSettings[nChn].dwFlags & CHN_MUTE) ? TRUE : FALSE);
			// Surround
			CheckDlgButton(IDC_CHECK2+ichn*2, (pSndFile->ChnSettings[nChn].dwFlags & CHN_SURROUND) ? TRUE : FALSE);
			// Volume
			int vol = pSndFile->ChnSettings[nChn].nVolume;
			m_sbVolume[ichn].SetPos(vol);
			SetDlgItemInt(IDC_EDIT1+ichn*2, vol);
			// Pan
			int pan = pSndFile->ChnSettings[nChn].nPan;
			m_sbPan[ichn].SetPos(pan/4);
			SetDlgItemInt(IDC_EDIT2+ichn*2, pan);
			memcpy(s, pSndFile->ChnSettings[nChn].szName, MAX_CHANNELNAME);
			s[MAX_CHANNELNAME-1] = 0;
			SetDlgItemText(IDC_EDIT9+ichn, s);
			// Channel effect
			m_CbnEffects[ichn].SetRedraw(FALSE);
			m_CbnEffects[ichn].ResetContent();
			m_CbnEffects[ichn].SetItemData(m_CbnEffects[ichn].AddString("No plugin"), 0);
			int fxsel = 0;
			for (UINT ifx=0; ifx<MAX_MIXPLUGINS; ifx++)
			{
				if ((pSndFile->m_MixPlugins[ifx].Info.dwPluginId1)
				 || (pSndFile->m_MixPlugins[ifx].Info.dwPluginId2)
				 || (pSndFile->m_MixPlugins[ifx].Info.szName[0]
				 || (pSndFile->ChnSettings[nChn].nMixPlugin == ifx+1)))
				{
					wsprintf(s, "FX%d: %s", ifx+1, pSndFile->m_MixPlugins[ifx].Info.szName);
					int n = m_CbnEffects[ichn].AddString(s);
					m_CbnEffects[ichn].SetItemData(n, ifx+1);
					if (pSndFile->ChnSettings[nChn].nMixPlugin == ifx+1) fxsel = n;
				}
			}
			m_CbnEffects[ichn].SetRedraw(TRUE);
			m_CbnEffects[ichn].SetCurSel(fxsel);
			// Enable/Disable controls for this channel
			BOOL bIT = ((bEnable) && (pSndFile->m_nType == MOD_TYPE_IT));
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK1+ichn*2), bEnable);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK2+ichn*2), bIT);
			::EnableWindow(m_sbVolume[ichn].m_hWnd, bEnable);
			::EnableWindow(m_sbPan[ichn].m_hWnd, bEnable);
			::EnableWindow(m_spinVolume[ichn], bEnable);
			::EnableWindow(m_spinPan[ichn], bEnable);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT1+ichn*2), bEnable);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT2+ichn*2), bEnable);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT9+ichn), ((bEnable) && (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT))));
		}
		UnlockControls();
	}
	// Update plugins
	if (dwHintMask & (HINT_MIXPLUGINS|HINT_MODTYPE))
	{
		m_CbnPlugin.SetRedraw(FALSE);
		m_CbnPlugin.ResetContent();
		for (UINT iPlug=0; iPlug<MAX_MIXPLUGINS; iPlug++)
		{
			PSNDMIXPLUGIN p = &pSndFile->m_MixPlugins[iPlug];
			p->Info.szLibraryName[63] = 0;
			if (p->Info.szLibraryName[0])
				wsprintf(s, "FX%d: %s", iPlug+1, p->Info.szLibraryName);
			else
				wsprintf(s, "FX%d: undefined", iPlug+1);
			m_CbnPlugin.AddString(s);
		}
		m_CbnPlugin.SetRedraw(TRUE);
		m_CbnPlugin.SetCurSel(m_nCurrentPlugin);
		if (m_nCurrentPlugin >= MAX_MIXPLUGINS) m_nCurrentPlugin = 0;
		PSNDMIXPLUGIN pPlugin = &(pSndFile->m_MixPlugins[m_nCurrentPlugin]);
		SetDlgItemText(IDC_EDIT13, pPlugin->Info.szName);
		CheckDlgButton(IDC_CHECK9, (pPlugin->Info.dwInputRouting & MIXPLUG_INPUTF_MASTEREFFECT) ? TRUE : FALSE);
		CheckDlgButton(IDC_CHECK10, (pPlugin->Info.dwInputRouting & MIXPLUG_INPUTF_BYPASS) ? TRUE : FALSE);
		CheckDlgButton(IDC_CHECK11, (pPlugin->Info.dwInputRouting & MIXPLUG_INPUTF_WETMIX) ? TRUE : FALSE);
		CVstPlugin *pVstPlugin = (pPlugin->pMixPlugin) ? (CVstPlugin *)pPlugin->pMixPlugin : NULL;
		m_BtnEdit.EnableWindow(((pVstPlugin) && ((pVstPlugin->HasEditor()) || (pVstPlugin->GetNumCommands()))) ? TRUE : FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_MOVEFXSLOT), (pVstPlugin)?TRUE:FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_INSERTFXSLOT), (pVstPlugin)?TRUE:FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_CLONEPLUG), (pVstPlugin)?TRUE:FALSE);
		//rewbs.DryRatio
		int n = static_cast<int>(pPlugin->fDryRatio*100);
		wsprintf(s, "(%d%% wet, %d%% dry)", 100-n, n);
		SetDlgItemText(IDC_STATIC8, s);
	m_sbDryRatio.SetPos(n);
		//end rewbs.DryRatio
		
// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
		if(pVstPlugin && pVstPlugin->isInstrument()){ // ericus 18/02/2005 : disable mix mode for VSTi
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_COMBO9), FALSE);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK12), FALSE);
		}
		else{
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_COMBO9), TRUE);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK12), TRUE);
			m_CbnSpecialMixProcessing.SetCurSel( (pPlugin->Info.dwInputRouting>>8) & 0xff ); // update#02 (fix)
			CheckDlgButton(IDC_CHECK12, (pPlugin->Info.dwInputRouting & MIXPLUG_INPUTF_MIXEXPAND) ? TRUE : FALSE);
		}
		// update#02
		DWORD gain = (pPlugin->Info.dwInputRouting>>16) & 0xff;
		if(gain == 0) gain = 10;
		float value = 0.1f * (float)gain;
		sprintf(s,"Gain: x %1.1f",value);
		SetDlgItemText(IDC_STATIC2, s);
		m_SpinMixGain.SetPos(gain);
// -! BEHAVIOUR_CHANGE#0028

		if (pVstPlugin)
		{
// -> CODE#0002
// -> DESC="VST plugins presets"
			CHAR sname[64];
// -! NEW_FEATURE#0002
			UINT nParams = pVstPlugin->GetNumParameters();
			m_CbnParam.SetRedraw(FALSE);
			m_CbnParam.ResetContent();
			for (UINT i=0; i<nParams; i++)
			{
				pVstPlugin->GetParamName(i, sname, sizeof(sname));
				wsprintf(s, "%02X: %s", i|0x80, sname);
				m_CbnParam.SetItemData(m_CbnParam.AddString(s), i);
			}
            m_CbnParam.SetRedraw(TRUE);
			if (m_nCurrentParam >= nParams) m_nCurrentParam = 0;
			m_CbnParam.SetCurSel(m_nCurrentParam);
			OnParamChanged();
			pVstPlugin->GetPluginType(s);

// -> CODE#0002
// -> DESC="VST plugins presets"
			CHAR s2[32];
			//UINT k = 0, nProg = min(pVstPlugin->GetNumPrograms(), 256);  //Limit number of progs to 256 because of insane plugs like synth1
			UINT k = 0, nProg = pVstPlugin->GetNumPrograms(); 
			m_CbnPreset.SetRedraw(FALSE);
			m_CbnPreset.ResetContent();
			wsprintf(s2, "current");
			m_CbnPreset.SetItemData(m_CbnPreset.AddString(s2), 0);
			for (i=0; i<nProg; i++)
			{
				k = 0;
				pVstPlugin->GetProgramNameIndexed(i, 0, sname);

				if(sname[0] < 32)
					wsprintf(s2, "%02X - Program %d",i,i);
				else{
					while(k < sizeof(sname)-1 && sname[k] != 0 && sname[k] < 'a' && sname[k] < 'z' && sname[k] < 'A' && sname[k] < 'Z') k++;
					wsprintf(s2, "%02X - %s",i,&sname[k]);
				}

				m_CbnPreset.SetItemData(m_CbnPreset.AddString(s2), i+1);
			}
			m_nCurrentPreset = 0;
			m_CbnPreset.SetRedraw(TRUE);
			m_CbnPreset.SetCurSel(0);
			m_sbValue.EnableWindow(TRUE);
			m_sbDryRatio.EnableWindow(TRUE);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT14), TRUE);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON3), TRUE);
// -! NEW_FEATURE#0002

		} else
		{
			s[0] = 0;
			if (m_CbnParam.GetCount() > 0) m_CbnParam.ResetContent();
			m_nCurrentParam = 0;
// -> CODE#0002
// -> DESC="VST plugins presets"
			CHAR s2[32];
			m_CbnPreset.SetRedraw(FALSE);
			m_CbnPreset.ResetContent();
			wsprintf(s2, "none");
			m_CbnPreset.SetItemData(m_CbnPreset.AddString(s2), 0);
			m_nCurrentPreset = 0;
			m_CbnPreset.SetRedraw(TRUE);
			m_CbnPreset.SetCurSel(0);
			m_sbValue.EnableWindow(FALSE);
			m_sbDryRatio.EnableWindow(FALSE);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT14), FALSE);
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON3), FALSE);
// -! NEW_FEATURE#0002
		}
		SetDlgItemText(IDC_TEXT6, s);
		int outputsel = 0;
		m_CbnOutput.SetRedraw(FALSE);
		m_CbnOutput.ResetContent();
		m_CbnOutput.SetItemData(m_CbnOutput.AddString("Default"), 0);
		for (UINT iOut=m_nCurrentPlugin+1; iOut<MAX_MIXPLUGINS; iOut++)
		{
			PSNDMIXPLUGIN p = &pSndFile->m_MixPlugins[iOut];
			if (p->Info.szLibraryName[0])
			{
				wsprintf(s, "FX%d: %s", iOut+1, p->Info.szLibraryName);
				int n = m_CbnOutput.AddString(s);
				m_CbnOutput.SetItemData(n, 0x80|iOut);
				if ((pSndFile->m_MixPlugins[m_nCurrentPlugin].Info.dwOutputRouting & 0x80)
				 && ((pSndFile->m_MixPlugins[m_nCurrentPlugin].Info.dwOutputRouting & 0x7f) == iOut))
				{
					outputsel = n;
				}
			}
		}
		m_CbnOutput.SetRedraw(TRUE);
		m_CbnOutput.SetCurSel(outputsel);
	}
}


void CViewGlobals::OnTabSelchange(NMHDR*, LRESULT* pResult)
//---------------------------------------------------------
{
	UpdateView(HINT_MODCHANNELS);
	if (pResult) *pResult = 0;
}


void CViewGlobals::OnMute1()
//--------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK1)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4;
	
	if (pModDoc)
	{
		pModDoc->MuteChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnMute2()
//--------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK3)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4 + 1;
	
	if (pModDoc)
	{
		pModDoc->MuteChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnMute3()
//--------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK5)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4 + 2;
	
	if (pModDoc)
	{
		pModDoc->MuteChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnMute4()
//--------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK7)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4 + 3;
	
	if (pModDoc)
	{
		pModDoc->MuteChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnSurround1()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK2)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4;
	
	if (pModDoc)
	{
		pModDoc->SurroundChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnSurround2()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK4)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4 + 1;
	
	if (pModDoc)
	{
		pModDoc->SurroundChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnSurround3()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK6)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4 + 2;
	
	if (pModDoc)
	{
		pModDoc->SurroundChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnSurround4()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL b = (IsDlgButtonChecked(IDC_CHECK8)) ? TRUE : FALSE;
	UINT nChn = m_nActiveTab * 4 + 3;
	
	if (pModDoc)
	{
		pModDoc->SurroundChannel(nChn, b);
		pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
}


void CViewGlobals::OnEditVol1()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4;
	int vol = GetDlgItemIntEx(IDC_EDIT1);
	if ((pModDoc) && (vol >= 0) && (vol <= 64) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelGlobalVolume(nChn, vol))
		{
			m_sbVolume[0].SetPos(vol);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditVol2()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4 + 1;
	int vol = GetDlgItemIntEx(IDC_EDIT3);
	if ((pModDoc) && (vol >= 0) && (vol <= 64) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelGlobalVolume(nChn, vol))
		{
			m_sbVolume[1].SetPos(vol);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditVol3()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4 + 2;
	int vol = GetDlgItemIntEx(IDC_EDIT5);
	if ((pModDoc) && (vol >= 0) && (vol <= 64) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelGlobalVolume(nChn, vol))
		{
			m_sbVolume[2].SetPos(vol);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditVol4()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4 + 3;
	int vol = GetDlgItemIntEx(IDC_EDIT7);
	if ((pModDoc) && (vol >= 0) && (vol <= 64) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelGlobalVolume(nChn, vol))
		{
			m_sbVolume[3].SetPos(vol);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditPan1()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4;
	int pan = GetDlgItemIntEx(IDC_EDIT2);
	if ((pModDoc) && (pan >= 0) && (pan <= 256) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelDefaultPan(nChn, pan))
		{
			m_sbPan[0].SetPos(pan/4);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditPan2()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4 + 1;
	int pan = GetDlgItemIntEx(IDC_EDIT4);
	if ((pModDoc) && (pan >= 0) && (pan <= 256) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelDefaultPan(nChn, pan))
		{
			m_sbPan[1].SetPos(pan/4);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditPan3()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4 + 2;
	int pan = GetDlgItemIntEx(IDC_EDIT6);
	if ((pModDoc) && (pan >= 0) && (pan <= 256) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelDefaultPan(nChn, pan))
		{
			m_sbPan[2].SetPos(pan/4);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditPan4()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	UINT nChn = m_nActiveTab * 4 + 3;
	int pan = GetDlgItemIntEx(IDC_EDIT8);
	if ((pModDoc) && (pan >= 0) && (pan <= 256) && (!m_nLockCount))
	{
		if (pModDoc->SetChannelDefaultPan(nChn, pan))
		{
			m_sbPan[3].SetPos(pan/4);
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//---------------------------------------------------------------------------
{
	CHAR s[64];
	CModDoc *pModDoc;
	UINT nChn;

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);

	pModDoc = GetDocument();
	nChn = m_nActiveTab * 4;
	if ((pModDoc) && (!IsLocked()) && (nChn < 64))
	{
		BOOL bUpdate = FALSE;
		short int pos;
		
		LockControls();
		for (UINT iCh=0; iCh<4; iCh++)
		{
			// Volume sliders
			pos = (short int)m_sbVolume[iCh].GetPos();
			if ((pos >= 0) && (pos <= 64))
			{
				if (pModDoc->SetChannelGlobalVolume(nChn+iCh, pos))
				{
					SetDlgItemInt(IDC_EDIT1+iCh*2, pos);
					bUpdate = TRUE;
				}
			}
			// Pan sliders
			pos = (short int)m_sbPan[iCh].GetPos();
			if ((pos >= 0) && (pos <= 64) && ((UINT)pos != pModDoc->GetSoundFile()->ChnSettings[nChn+iCh].nPan/4))
			{
				if (pModDoc->SetChannelDefaultPan(nChn+iCh, pos*4))
				{
					SetDlgItemInt(IDC_EDIT2+iCh*2, pos*4);
					bUpdate = TRUE;
				}
			}
		}


		//rewbs.dryRatio
		if ((pScrollBar) && (pScrollBar->m_hWnd == m_sbDryRatio.m_hWnd))
		{
			int n = m_sbDryRatio.GetPos();
			if ((n >= 0) && (n <= 100) && (m_nCurrentPlugin < MAX_MIXPLUGINS))
			{				
				CSoundFile *pSndFile = pModDoc->GetSoundFile();
				PSNDMIXPLUGIN pPlugin;

				pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
				if (pPlugin->pMixPlugin)
				{	
					wsprintf(s, "(%d%% wet, %d%% dry)", 100-n, n);
					SetDlgItemText(IDC_STATIC8, s);
					pPlugin->fDryRatio = static_cast<float>(n)/100.0f;
					if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
				}
			}
		}
		//end rewbs.dryRatio

		if (bUpdate) pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		UnlockControls();

		if ((pScrollBar) && (pScrollBar->m_hWnd == m_sbValue.m_hWnd))
		{
			int n = (short int)m_sbValue.GetPos();
			if ((n >= 0) && (n <= 100) && (m_nCurrentPlugin < MAX_MIXPLUGINS))
			{
				CSoundFile *pSndFile = pModDoc->GetSoundFile();
				PSNDMIXPLUGIN pPlugin;

				pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
				if (pPlugin->pMixPlugin)
				{
					CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
					UINT nParams = pVstPlugin->GetNumParameters();
					if (m_nCurrentParam < nParams)
					{
						FLOAT fValue = 0.01f * n;
						wsprintf(s, "%d.%02d", n/100, n%100);
						SetDlgItemText(IDC_EDIT14, s);
						if ((nSBCode == SB_THUMBPOSITION) || (nSBCode == SB_ENDSCROLL))
						{
							pVstPlugin->SetParameter(m_nCurrentParam, fValue);
							OnParamChanged();
							if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
						}
					}
				}
			}
		}
	}
}


void CViewGlobals::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//---------------------------------------------------------------------------
{
// -> CODE#0028	update#02
// -> DESC="effect plugin mixing mode combo"
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	CHAR s[32];

	if((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;

	if(nSBCode != SB_ENDSCROLL && pScrollBar && pScrollBar == (CScrollBar*)&m_SpinMixGain){

		pSndFile = pModDoc->GetSoundFile();
		pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];

		if(pPlugin->pMixPlugin){
			DWORD gain = nPos;
			if(gain == 0) gain = 1;

			pPlugin->Info.dwInputRouting = (pPlugin->Info.dwInputRouting & 0xff00ffff) | (gain<<16);
			pPlugin->pMixPlugin->RecalculateGain();
			
			float fValue = 0.1f * (float)gain;
			sprintf(s,"Gain: x %1.1f",fValue);
			SetDlgItemText(IDC_STATIC2, s);

			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
		}
	}
// -! BEHAVIOUR_CHANGE#0028

	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CViewGlobals::OnEditName1()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (!m_nLockCount))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		CHAR s[MAX_CHANNELNAME+4];
		UINT nChn = m_nActiveTab * 4;
		
		memset(s, 0, sizeof(s));
		GetDlgItemText(IDC_EDIT9, s, sizeof(s));
		s[MAX_CHANNELNAME+1] = 0;
		if ((pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) && (nChn < pSndFile->m_nChannels) && (strncmp(s, pSndFile->ChnSettings[nChn].szName, MAX_CHANNELNAME)))
		{
			memcpy(pSndFile->ChnSettings[nChn].szName, s, MAX_CHANNELNAME);
			pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditName2()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (!m_nLockCount))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		CHAR s[MAX_CHANNELNAME+4];
		UINT nChn = m_nActiveTab * 4 + 1;
		
		memset(s, 0, sizeof(s));
		GetDlgItemText(IDC_EDIT10, s, sizeof(s));
		s[MAX_CHANNELNAME+1] = 0;
		if ((pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) && (nChn < pSndFile->m_nChannels) && (strncmp(s, pSndFile->ChnSettings[nChn].szName, MAX_CHANNELNAME)))
		{
			memcpy(pSndFile->ChnSettings[nChn].szName, s, MAX_CHANNELNAME);
			pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditName3()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (!m_nLockCount))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		CHAR s[MAX_CHANNELNAME+4];
		UINT nChn = m_nActiveTab * 4 + 2;
		
		memset(s, 0, sizeof(s));
		GetDlgItemText(IDC_EDIT11, s, sizeof(s));
		s[MAX_CHANNELNAME+1] = 0;
		if ((pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) && (nChn < pSndFile->m_nChannels) && (strncmp(s, pSndFile->ChnSettings[nChn].szName, MAX_CHANNELNAME)))
		{
			memcpy(pSndFile->ChnSettings[nChn].szName, s, MAX_CHANNELNAME);
			pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnEditName4()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (!m_nLockCount))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		CHAR s[MAX_CHANNELNAME+4];
		UINT nChn = m_nActiveTab * 4 + 3;
		
		memset(s, 0, sizeof(s));
		GetDlgItemText(IDC_EDIT12, s, sizeof(s));
		s[MAX_CHANNELNAME+1] = 0;
		if ((pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) && (nChn < pSndFile->m_nChannels) && (strncmp(s, pSndFile->ChnSettings[nChn].szName, MAX_CHANNELNAME)))
		{
			memcpy(pSndFile->ChnSettings[nChn].szName, s, MAX_CHANNELNAME);
			pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnFx1Changed()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		UINT nChn = m_nActiveTab * 4;
		int nfx = m_CbnEffects[0].GetItemData(m_CbnEffects[0].GetCurSel());
		if ((nfx >= 0) && (nfx <= MAX_MIXPLUGINS) && (nChn < pSndFile->m_nChannels)
		 && (pSndFile->ChnSettings[nChn].nMixPlugin != (UINT)nfx))
		{
			pSndFile->ChnSettings[nChn].nMixPlugin = nfx;
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnFx2Changed()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		UINT nChn = m_nActiveTab * 4 + 1;
		int nfx = m_CbnEffects[1].GetItemData(m_CbnEffects[1].GetCurSel());
		if ((nfx >= 0) && (nfx <= MAX_MIXPLUGINS) && (nChn < pSndFile->m_nChannels)
		 && (pSndFile->ChnSettings[nChn].nMixPlugin != (UINT)nfx))
		{
			pSndFile->ChnSettings[nChn].nMixPlugin = nfx;
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnFx3Changed()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		UINT nChn = m_nActiveTab * 4 + 2;
		int nfx = m_CbnEffects[2].GetItemData(m_CbnEffects[2].GetCurSel());
		if ((nfx >= 0) && (nfx <= MAX_MIXPLUGINS) && (nChn < pSndFile->m_nChannels)
		 && (pSndFile->ChnSettings[nChn].nMixPlugin != (UINT)nfx))
		{
			pSndFile->ChnSettings[nChn].nMixPlugin = nfx;
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnFx4Changed()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		UINT nChn = m_nActiveTab * 4 + 3;
		int nfx = m_CbnEffects[3].GetItemData(m_CbnEffects[3].GetCurSel());
		if ((nfx >= 0) && (nfx <= MAX_MIXPLUGINS) && (nChn < pSndFile->m_nChannels)
		 && (pSndFile->ChnSettings[nChn].nMixPlugin != (UINT)nfx))
		{
			pSndFile->ChnSettings[nChn].nMixPlugin = nfx;
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
			pModDoc->UpdateAllViews(this, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnPluginNameChanged()
//--------------------------------------
{
	CHAR s[64];
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (m_nCurrentPlugin < MAX_MIXPLUGINS))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		memset(s, 0, 32);
		GetDlgItemText(IDC_EDIT13, s, 32);
		s[31] = 0;
		if (strcmp(s, pSndFile->m_MixPlugins[m_nCurrentPlugin].Info.szName))
		{
			memcpy(pSndFile->m_MixPlugins[m_nCurrentPlugin].Info.szName, s, 32);
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
			pModDoc->UpdateAllViews(NULL, HINT_MODCHANNELS | (m_nActiveTab << 24));
		}
	}
}


void CViewGlobals::OnPrevPlugin()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((m_nCurrentPlugin > 0) && (pModDoc))
	{
		m_nCurrentPlugin--;
		pModDoc->UpdateAllViews(NULL, HINT_MIXPLUGINS | HINT_MODCHANNELS | (m_nActiveTab << 24));
// -> CODE#0014
// -> DESC="vst wet/dry slider"
		//OnWetDryChanged();

	}
}


void CViewGlobals::OnNextPlugin()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((m_nCurrentPlugin < MAX_MIXPLUGINS-1) && (pModDoc))
	{
		m_nCurrentPlugin++;
		pModDoc->UpdateAllViews(NULL, HINT_MIXPLUGINS | HINT_MODCHANNELS | (m_nActiveTab << 24));
// -> CODE#0014
// -> DESC="vst wet/dry slider"
		//OnWetDryChanged();

	}
}


void CViewGlobals::OnPluginChanged()
//----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	int nPlugin = m_CbnPlugin.GetCurSel();
	if ((pModDoc) && (nPlugin >= 0) && (nPlugin < MAX_MIXPLUGINS))
	{
		m_nCurrentPlugin = nPlugin;
		pModDoc->UpdateAllViews(NULL, HINT_MIXPLUGINS | HINT_MODCHANNELS | (m_nActiveTab << 24));
	}
// -> CODE#0002
// -> DESC="VST plugins presets"
	m_nCurrentPreset = 0;
	m_CbnPreset.SetCurSel(0);
// -! NEW_FEATURE#0002

// -> CODE#0014
// -> DESC="vst wet/dry slider"
	//OnWetDryChanged();
// -! NEW_FEATURE#0014
}


void CViewGlobals::OnSelectPlugin()
//---------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (m_nCurrentPlugin < MAX_MIXPLUGINS))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		PSNDMIXPLUGIN pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
		CSelectPluginDlg dlg(pPlugin, pModDoc, this); //rewbs.plugDocAware
		if (dlg.DoModal() == IDOK)
		{
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT))
			{
				pModDoc->SetModified();
			}
		}
		OnPluginChanged();
		OnParamChanged();
// -> CODE#0014
// -> DESC="vst wet/dry slider"
		//OnWetDryChanged();
// -! NEW_FEATURE#0014
	}
}


void CViewGlobals::OnParamChanged()
//---------------------------------
{
	int cursel = m_CbnParam.GetCurSel();
	CModDoc *pModDoc = GetDocument();
	CHAR s[256];
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	
	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (pPlugin->pMixPlugin)
	{
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		UINT nParams = pVstPlugin->GetNumParameters();
		if ((cursel >= 0) && (cursel < (int)nParams)) m_nCurrentParam = cursel;
		if (m_nCurrentParam < nParams)
		{
			CHAR sunits[64], sdisplay[64];
			pVstPlugin->GetParamLabel(m_nCurrentParam, sunits);
			pVstPlugin->GetParamDisplay(m_nCurrentParam, sdisplay);
			wsprintf(s, "Value: %s %s", sdisplay, sunits);
			SetDlgItemText(IDC_TEXT5, s);
			float fValue = pVstPlugin->GetParameter(m_nCurrentParam);
			int nValue = (int)(fValue * 100.0f + 0.5f);
			sprintf(s, "%f", fValue); //wsprintf(s, "%d.%02d", nValue/100, nValue%100); // ericus 25/01/2005
			SetDlgItemText(IDC_EDIT14, s);
			m_sbValue.SetPos(nValue);
			return;
		}
	}
	SetDlgItemText(IDC_TEXT5, "Value:");
	SetDlgItemText(IDC_EDIT14, "");
	m_sbValue.SetPos(0);
}

// -> CODE#0002
// -> DESC="VST plugins presets"
void CViewGlobals::OnProgramChanged()
{
	int cursel = m_CbnPreset.GetCurSel();
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	
	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (pPlugin->pMixPlugin)
	{
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		UINT nParams = pVstPlugin->GetNumPrograms();
		if ((cursel > 0) && (cursel <= (int)nParams)) m_nCurrentPreset = cursel;
		if (m_nCurrentPreset > 0 && m_nCurrentPreset <= nParams){
			pVstPlugin->SetCurrentProgram(m_nCurrentPreset-1);
		}
		if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
	}
}

void CViewGlobals::OnLoadParam()
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile = pModDoc ? pModDoc->GetSoundFile() : NULL;
	PSNDMIXPLUGIN pPlugin = pSndFile ? &pSndFile->m_MixPlugins[m_nCurrentPlugin] : NULL;
	CVstPlugin *pVstPlugin = pPlugin ? (CVstPlugin *)pPlugin->pMixPlugin : NULL;

	//rewbs.fxpPresets: changed Eric's code to use fxp load/save
	if(pVstPlugin == NULL) return;

	CFileDialog dlg(TRUE, "fxp", NULL,
				OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_NOREADONLYRETURN,
				"VST FX Program (*.fxp)|*.fxp||",	theApp.m_pMainWnd);
	if (!(dlg.DoModal() == IDOK))	return;
    
	//TODO: exception handling
	if (!(pVstPlugin->LoadProgram(dlg.GetFileName()))) {
		::AfxMessageBox("Error loading preset.Are you sure it is for this plugin?");
	} else {
		if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
	}

	//end rewbs.fxpPresets
}

void CViewGlobals::OnSaveParam()
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile = pModDoc ? pModDoc->GetSoundFile() : NULL;
	PSNDMIXPLUGIN pPlugin = pSndFile ? &pSndFile->m_MixPlugins[m_nCurrentPlugin] : NULL;
	CVstPlugin *pVstPlugin = pPlugin ? (CVstPlugin *)pPlugin->pMixPlugin : NULL;

	if(pVstPlugin == NULL) return;

	//rewbs.fxpPresets: changed Eric's code to use fxp load/save
	CFileDialog dlg(FALSE, "fxp", NULL,
				OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_NOREADONLYRETURN,
				"VST Program (*.fxp)|*.fxp||",	theApp.m_pMainWnd);
	if (!(dlg.DoModal() == IDOK))	return;

	//TODO: exception handling
	if (!(pVstPlugin->SaveProgram(dlg.GetFileName())))
		::AfxMessageBox("Error saving preset.");
	//end rewbs.fxpPresets

}
// -! NEW_FEATURE#0002


VOID CViewGlobals::OnSetParameter()
//---------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CHAR s[256];
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	
	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (pPlugin->pMixPlugin)
	{
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		UINT nParams = pVstPlugin->GetNumParameters();
		GetDlgItemText(IDC_EDIT14, s, sizeof(s));
		if ((m_nCurrentParam < nParams) && (s[0]))
		{
			FLOAT fValue = (FLOAT)atof(s);
			pVstPlugin->SetParameter(m_nCurrentParam, fValue);
            OnParamChanged();
			if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
		}
	}
}


// -> CODE#0014
// -> DESC="vst wet/dry slider"
VOID CViewGlobals::OnSetWetDry()
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	
	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (pPlugin->pMixPlugin){
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		UINT value = GetDlgItemIntEx(IDC_EDIT15);
		pPlugin->fDryRatio = (float)value / 100.0f;
		if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
		//OnWetDryChanged();
	}
}

/*
void CViewGlobals::OnWetDryChanged()
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	CHAR s[32];
	
	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;

	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];

	if (pPlugin->pMixPlugin){
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		UINT value = (UINT)(pPlugin->fDryRatio * 100.0f);
		wsprintf(s, "%d", value);
		SetDlgItemText(IDC_EDIT15, s);
		m_sbWetDry.SetPos(value);
		return;
	}

	SetDlgItemText(IDC_EDIT15, "");
	m_sbWetDry.SetPos(0);
}
// -! NEW_FEATURE#0014
*/

VOID CViewGlobals::OnMixModeChanged()
//-----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;

	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (IsDlgButtonChecked(IDC_CHECK9))
	{
		pPlugin->Info.dwInputRouting |= MIXPLUG_INPUTF_MASTEREFFECT;
	} else
	{
		pPlugin->Info.dwInputRouting &= ~MIXPLUG_INPUTF_MASTEREFFECT;
	}
	
	if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
}


VOID CViewGlobals::OnBypassChanged()
//----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;

	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (IsDlgButtonChecked(IDC_CHECK10))
	{
		pPlugin->Info.dwInputRouting |= MIXPLUG_INPUTF_BYPASS;
	} else
	{
		pPlugin->Info.dwInputRouting &= ~MIXPLUG_INPUTF_BYPASS;
	}

	if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
}


// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
void CViewGlobals::OnWetDryExpandChanged()
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;

	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (IsDlgButtonChecked(IDC_CHECK12))
	{
		pPlugin->Info.dwInputRouting |= MIXPLUG_INPUTF_MIXEXPAND;
	} else
	{
		pPlugin->Info.dwInputRouting &= ~MIXPLUG_INPUTF_MIXEXPAND;
	}
	
	if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
}

VOID CViewGlobals::OnSpecialMixProcessingChanged()
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile = pModDoc ? pModDoc->GetSoundFile() : NULL;
	PSNDMIXPLUGIN pPlugin = m_nCurrentPlugin < MAX_MIXPLUGINS && pSndFile ? &pSndFile->m_MixPlugins[m_nCurrentPlugin] : NULL;

	if(!pPlugin) return;
	pPlugin->Info.dwInputRouting = (pPlugin->Info.dwInputRouting & 0xffff00ff) | (m_CbnSpecialMixProcessing.GetCurSel()<<8);	// update#02 (fix)
	if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
}
// -! BEHAVIOUR_CHANGE#0028


VOID CViewGlobals::OnDryMixChanged()
//----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;

	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (IsDlgButtonChecked(IDC_CHECK11))
	{
		pPlugin->Info.dwInputRouting |= MIXPLUG_INPUTF_WETMIX;
	} else
	{
		pPlugin->Info.dwInputRouting &= ~MIXPLUG_INPUTF_WETMIX;
	}

	if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
}


VOID CViewGlobals::OnEditPlugin()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pModDoc->TogglePluginEditor(m_nCurrentPlugin);
	return;	
}


VOID CViewGlobals::OnFxCommands(UINT id)
//--------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	UINT nIndex = id - ID_FXCOMMANDS_BASE;

	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	if (pPlugin->pMixPlugin)
	{
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		pVstPlugin->ExecuteCommand(nIndex);
		if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
	}
}


VOID CViewGlobals::OnOutputRoutingChanged()
//-----------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	PSNDMIXPLUGIN pPlugin;
	CSoundFile *pSndFile;
	int nroute;

	if ((m_nCurrentPlugin >= MAX_MIXPLUGINS) || (!pModDoc)) return;
	pSndFile = pModDoc->GetSoundFile();
	pPlugin = &pSndFile->m_MixPlugins[m_nCurrentPlugin];
	nroute = m_CbnOutput.GetItemData(m_CbnOutput.GetCurSel());
	pPlugin->Info.dwOutputRouting = nroute;
	if (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) pModDoc->SetModified();
}



LRESULT CViewGlobals::OnModViewMsg(WPARAM wParam, LPARAM lParam)
//-----------------------------------------------------------------
{
	switch(wParam)
	{
		case VIEWMSG_SETFOCUS:
		case VIEWMSG_SETACTIVE:
			GetParentFrame()->SetActiveView(this);
			SetFocus();
			return 0;
		default:
			return 0;
	}
}

void CViewGlobals::OnMovePlugToSlot() 
//-----------------------------------
{
	CMoveFXSlotDialog dlg((CWnd*)this);
	CArray<UINT, UINT> emptySlots;
	BuildEmptySlotList(emptySlots);

	dlg.SetupMove(m_nCurrentPlugin, emptySlots);

	if (dlg.DoModal() == IDOK) { 
		MovePlug(m_nCurrentPlugin, dlg.m_nToSlot);
		m_CbnPlugin.SetCurSel(dlg.m_nToSlot);
		OnPluginChanged();
	}

}

bool CViewGlobals::MovePlug(UINT src, UINT dest)
//----------------------------------------------
{
	//AfxMessageBox("Moving %d to %d", src, dest);
	CModDoc *pModDoc = GetDocument();
	CSoundFile* pSndFile = pModDoc->GetSoundFile();
	
	BEGIN_CRITICAL();
		
	// Move plug data
	memcpy(&(pSndFile->m_MixPlugins[dest]), &(pSndFile->m_MixPlugins[src]), sizeof(SNDMIXPLUGIN));
	memset(&(pSndFile->m_MixPlugins[src]), 0, sizeof(SNDMIXPLUGIN));
	
	//Prevent plug from pointing backwards.
	if (pSndFile->m_MixPlugins[dest].Info.dwOutputRouting & 0x80) {
		UINT nOutput = pSndFile->m_MixPlugins[dest].Info.dwOutputRouting & 0x7f;
		if (nOutput<=dest) {
			pSndFile->m_MixPlugins[dest].Info.dwOutputRouting = 0;
		}
	}
	
	// Update current plug
	if (pSndFile->m_MixPlugins[dest].pMixPlugin) {
		((CVstPlugin*)pSndFile->m_MixPlugins[dest].pMixPlugin)->SetSlot(dest);
		((CVstPlugin*)pSndFile->m_MixPlugins[dest].pMixPlugin)->UpdateMixStructPtr(&(pSndFile->m_MixPlugins[dest]));
	}
	
	// Update all other plugs' outputs
	for (int nPlug=0; nPlug<src; nPlug++) {
		if (pSndFile->m_MixPlugins[nPlug].Info.dwOutputRouting & 0x80) {
			if ((pSndFile->m_MixPlugins[nPlug].Info.dwOutputRouting & 0x7f) == src) {
				pSndFile->m_MixPlugins[nPlug].Info.dwOutputRouting = ((BYTE)dest)|0x80;
			}
		}
	}
	// Update channels
	for (int nChn=0; nChn<pSndFile->m_nChannels; nChn++) {
		if (pSndFile->ChnSettings[nChn].nMixPlugin == src+1) {
			pSndFile->ChnSettings[nChn].nMixPlugin = dest+1;
		}
	}

	// Update instruments
	for (int nIns=1; nIns<=pSndFile->m_nInstruments; nIns++) {
		if (pSndFile->Headers[nIns] && (pSndFile->Headers[nIns]->nMixPlug == src+1)) {
			pSndFile->Headers[nIns]->nMixPlug = dest+1;
		}
	}

	END_CRITICAL();

	return true;
}

void CViewGlobals::BuildEmptySlotList(CArray<UINT, UINT> &emptySlots) 
//-------------------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile* pSndFile = pModDoc->GetSoundFile();
	
	emptySlots.RemoveAll();

	for (UINT nSlot=0; nSlot<MAX_MIXPLUGINS; nSlot++) {
		if (pSndFile->m_MixPlugins[nSlot].pMixPlugin == NULL) {
			emptySlots.Add(nSlot);	
		}
	}
	return;
}

void CViewGlobals::OnInsertSlot()
//-------------------------------
{
	CString prompt;
	CModDoc *pModDoc = GetDocument();
	CSoundFile* pSndFile = pModDoc->GetSoundFile();
	prompt.Format("Insert empty slot before slot FX%d?", m_nCurrentPlugin+1);
	if (pSndFile->m_MixPlugins[MAX_MIXPLUGINS-1].pMixPlugin) {
		prompt.Append("\nWarning: plugin data in last slot will be lost."); 
	}
	if (AfxMessageBox(prompt, MB_YESNO) == IDYES) {

		//Delete last plug...
		if (pSndFile->m_MixPlugins[MAX_MIXPLUGINS-1].pMixPlugin) {
			pSndFile->m_MixPlugins[MAX_MIXPLUGINS-1].pMixPlugin->Release();
			memset(&(pSndFile->m_MixPlugins[MAX_MIXPLUGINS-1]), 0, sizeof(SNDMIXPLUGIN));
			//possible mem leak here...
		}

		for (int nSlot=MAX_MIXPLUGINS-1; nSlot>(int)m_nCurrentPlugin; nSlot--) {
			if (pSndFile->m_MixPlugins[nSlot-1].pMixPlugin) {
				MovePlug(nSlot-1, nSlot);
			}
		}

		m_CbnPlugin.SetCurSel(m_nCurrentPlugin);
		OnPluginChanged();
	}

}

void CViewGlobals::OnClonePlug()
//------------------------------
{
	AfxMessageBox("Not yet implemented.");
}

