//rewbs.defaultPlugGUI

#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "vstplug.h"
#include "fxp.h"
#include "AbstractVstEditor.h"

BEGIN_MESSAGE_MAP(CAbstractVstEditor, CDialog)
	ON_WM_CLOSE()
	ON_COMMAND(ID_PRESET_LOAD,			OnLoadPreset)
	ON_COMMAND(ID_PRESET_SAVE,			OnSavePreset)
	ON_COMMAND(ID_PRESET_RANDOM,		OnRandomizePreset)
	ON_COMMAND_RANGE(ID_PRESET_SET, ID_PRESET_SET+MAX_PLUGPRESETS, OnSetPreset)
END_MESSAGE_MAP()

CAbstractVstEditor::CAbstractVstEditor(CVstPlugin *pPlugin)
{
	m_nCurProg = -1;
	m_pVstPlugin = pPlugin;
	m_pMenu = new CMenu();
	m_pPresetMenu = new CMenu();
	m_pMenu->LoadMenu(IDR_VSTMENU);
}

CAbstractVstEditor::~CAbstractVstEditor()
{
#ifdef VST_LOG
	Log("~CVstEditor()\n");
#endif
	if (m_pVstPlugin)
	{
		m_pMenu->DestroyMenu();
		m_pPresetMenu->DestroyMenu();
		m_pVstPlugin->m_pEditor = NULL;
		m_pVstPlugin = NULL;
	}
}

VOID CAbstractVstEditor::OnLoadPreset()
//-------------------------------------
{
	if (m_pVstPlugin)
	{
		CFileDialog dlg(TRUE, "fxp", NULL,
					OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_NOREADONLYRETURN,
					"VST Program (*.fxp)|*.fxp||",	theApp.m_pMainWnd);
		if (!(dlg.DoModal() == IDOK))	return;

		//TODO: exception handling to distinguish errors at this level.
		if (!(m_pVstPlugin->LoadProgram(dlg.GetFileName())))
			::AfxMessageBox("Error loading preset. Are you sure it is for this plugin?");
	}
}

VOID CAbstractVstEditor::OnSavePreset()
//-------------------------------------
{
	if (m_pVstPlugin)
	{
		CFileDialog dlg(FALSE, "fxp", NULL,
					OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_NOREADONLYRETURN,
					"VST Program (*.fxp)|*.fxp||",	theApp.m_pMainWnd);
		if (!(dlg.DoModal() == IDOK))	return;

		//TODO: exception handling
		if (!(m_pVstPlugin->SaveProgram(dlg.GetFileName())))
			::AfxMessageBox("Error saving preset. Are you sure it is for this plugin?");
	}
	return;
}

VOID CAbstractVstEditor::OnRandomizePreset()
//-----------------------------------------
{
	if (m_pVstPlugin)
	{
		if (::AfxMessageBox("Are you sure you want to randomize parameters?\nYou will lose current parameter values.", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
			m_pVstPlugin->RandomizeParams();
		UpdateAll();
	}
}

VOID CAbstractVstEditor::SetupMenu()
//----------------------------------
{
	if (m_pVstPlugin)
	{
		long numProgs = m_pVstPlugin->GetNumPrograms();
		long curProg  = m_pVstPlugin->GetCurrentProgram();
		char s[256];
		char sname[256];
		int k;

		if (m_pPresetMenu->m_hMenu)
		{
			m_pPresetMenu->DestroyMenu();
			m_pMenu->DeleteMenu(1, MF_BYPOSITION);	
		}
		m_pPresetMenu->CreatePopupMenu();
		
		for (long p=0; p<numProgs; p++)
		{
			k = 0;
			m_pVstPlugin->GetProgramNameIndexed(p, -1, sname);

			if(sname[0] < 32)	//Invalid name?
			{
				wsprintf(s, "%02X - Program %d",p,p);
			}
			else
			{
				while(k < sizeof(sname)-1 && sname[k] != 0 && sname[k] < 'a' && sname[k] < 'z' && sname[k] < 'A' && sname[k] < 'Z') k++;
				wsprintf(s, "%02X - %s",p,&sname[k]);
			}

			if (p==m_nCurProg)
				m_pPresetMenu->AppendMenu(MF_STRING|MF_CHECKED, ID_PRESET_SET+p, (LPCTSTR)s);
			else
				m_pPresetMenu->AppendMenu(MF_STRING, ID_PRESET_SET+p, (LPCTSTR)s);
		}
		if (numProgs)
			m_pMenu->InsertMenu(1, MF_BYPOSITION|MF_POPUP, (UINT) m_pPresetMenu->m_hMenu, (LPCTSTR)"&Factory");
		else
			m_pMenu->InsertMenu(1, MF_BYPOSITION|MF_POPUP|MF_GRAYED, (UINT) m_pPresetMenu->m_hMenu, (LPCTSTR)"&Factory");
	}
	::SetMenu(m_hWnd, m_pMenu->m_hMenu);
}

void CAbstractVstEditor::OnSetPreset(UINT nID)
{
	int nIndex=nID-ID_PRESET_SET;
	if (nIndex>=0)
	{
		m_nCurProg=nIndex;
		m_pVstPlugin->SetCurrentProgram(nIndex);
		SetupMenu();
	}
}

//end rewbs.defaultPlugGUI