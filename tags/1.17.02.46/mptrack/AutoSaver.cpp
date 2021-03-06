#include "stdafx.h"
#include ".\autosaver.h"
#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "sndfile.h"
#include "moddoc.h"
#include "AutoSaver.h"
#include "moptions.h"
#include <algorithm>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>


///////////////////////////////////////////////////////////////////////////////////////
// AutoSaver.cpp : implementation file
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////
// Construction/Destruction
///////////////////////////

CAutoSaver::CAutoSaver()
{
}

CAutoSaver::CAutoSaver(bool enabled, int saveInterval, int backupHistory,
					   bool useOriginalPath, CString path, CString fileNameTemplate)
{
	m_nLastSave			 = timeGetTime();
	m_bEnabled			 = enabled;
	m_nSaveInterval		 = saveInterval*60*1000; //minutes to milliseconds
	m_nBackupHistory	 = backupHistory;
	m_bUseOriginalPath	 = useOriginalPath;
	m_csPath			 = path;
	m_csFileNameTemplate = fileNameTemplate;
}

CAutoSaver::~CAutoSaver(void)
{
}

//////////////
// Entry Point
//////////////

bool CAutoSaver::DoSave(DWORD curTime)
{
	bool success = true;
    
	if (CheckTimer(curTime)) { //if time to save

		CDocTemplate *pDocTemplate;
		CModDoc *pModDoc;
		POSITION posTemplate,posDocument;
		CTrackApp *pTrackApp=(CTrackApp*)::AfxGetApp();
		if (!pTrackApp) return false;

		pTrackApp->BeginWaitCursor(); //display hour glass

		posTemplate = pTrackApp->GetFirstDocTemplatePosition();
		while (posTemplate) { //for all "templates" (we should have just 1)
			pDocTemplate = pTrackApp->GetNextDocTemplate(posTemplate);
			posDocument = pDocTemplate->GetFirstDocPosition();

			while (posDocument) { //for all open documents
				pModDoc = (CModDoc*)(pDocTemplate->GetNextDoc(posDocument));
 				if (pModDoc && pModDoc->IsModified()) {
					if (SaveSingleFile(pModDoc)) {
						CleanUpBackups(pModDoc);
					} else {
						m_bEnabled=false;
						AfxMessageBox("Warning: autosave failed and has been disabled. Please:\r\n\r\n- Review your autosave paths\r\n- Check available diskspace & filesystem access rights\r\n- If you are using the ITP format, ensure all instruments exist as independant .iti files");
						success = false;
					}
				}
			} //end all open documents

		} //end pointless template loop (we have just 1 template)
		m_nLastSave = timeGetTime();
		pTrackApp->EndWaitCursor(); //end display hour glass
	}
	
	return success;
}

////////////////
// Member access
////////////////
void CAutoSaver::Enable()
{
	m_bEnabled=true;
}

void CAutoSaver::Disable()
{
	m_bEnabled=false;
}

bool CAutoSaver::IsEnabled()
{
	return m_bEnabled;
}

void CAutoSaver::SetUseOriginalPath(bool useOrgPath)
{
	m_bUseOriginalPath=useOrgPath;
}

bool CAutoSaver::GetUseOriginalPath()
{
	return m_bUseOriginalPath;
}

void CAutoSaver::SetPath(CString path)
{
	m_csPath = path;
}

CString CAutoSaver::GetPath()
{
	return m_csPath;
}

void CAutoSaver::SetFilenameTemplate(CString fnTemplate)
{
	m_csFileNameTemplate = fnTemplate;
}

CString CAutoSaver::GetFilenameTemplate()
{
	return m_csFileNameTemplate;
}

void CAutoSaver::SetHistoryDepth(int history)
{
	if (history<1) {
		m_nBackupHistory=1;
	} else if (history>100) {
		m_nBackupHistory=100;
	} else {
		m_nBackupHistory=history;
	}
}

int CAutoSaver::GetHistoryDepth()
{
	return m_nBackupHistory;
}

void CAutoSaver::SetSaveInterval(int minutes)
{
	if (minutes<1) {
		minutes=1;
	} else if (minutes>10000) {
		minutes=10000;
	}
	
	m_nSaveInterval=minutes*60*1000; //minutes to milliseconds
}

int CAutoSaver::GetSaveInterval()
{
	return m_nSaveInterval/60/1000;
}

///////////////////////////
// Implementation internals
///////////////////////////

bool CAutoSaver::CheckTimer(DWORD curTime) 
{
	DWORD curInterval = curTime-m_nLastSave;
	return (curInterval>=m_nSaveInterval);
}


CString CAutoSaver::BuildFileName(CModDoc* pModDoc)
{
	CString timeStamp = (CTime::GetCurrentTime()).Format("%Y%m%d.%H%M%S");
	CString name;
	
	if (m_bUseOriginalPath) {
		if (pModDoc->m_bHasValidPath) { // Check that the file has a user-chosen path
			name = pModDoc->GetPathName(); 
		} else {						// if it doesnt, put it in executable dir
			name = CMainFrame::m_csExecutablePath + "\\" + pModDoc->GetTitle(); 		
		}
	
	} else {
		name = m_csPath+pModDoc->GetTitle();
	}
	
	
	name.Append(".AutoSave.");					//append backup tag
	name.Append(timeStamp);						//append timestamp
	switch (pModDoc->GetModType()) {			//append extension
		case MOD_TYPE_MOD:
            name.Append(".mod");
			break;
		case MOD_TYPE_IT:
			if((pModDoc->GetSoundFile())->m_dwSongFlags & SONG_ITPROJECT){
				name.Append(".itp");
			} else{
 				name.Append(".it");
			}
			break;
		case MOD_TYPE_MPT:
			name.Append(".mptm");
			break;
		case MOD_TYPE_XM:
			name.Append(".xm");
			break;
		case MOD_TYPE_S3M:
			name.Append(".s3m");
			break;
	}

	return name;
}
bool CAutoSaver::SaveSingleFile(CModDoc *pModDoc) 
{
	// We do not call CModDoc::DoSave as this populates the Recent Files
	// list with backups... hence we have duplicated code.. :(
	bool success=false;
	CSoundFile* pSndFile = pModDoc->GetSoundFile(); 
	
	if (pSndFile) {
		CString fileName = BuildFileName(pModDoc);

		switch (pModDoc->GetModType()) {
			case MOD_TYPE_MOD:
				success = pSndFile->SaveMod(fileName, 0); 
				break;
			case MOD_TYPE_S3M:
				success = pSndFile->SaveS3M(fileName, 0); 
				break;
			case MOD_TYPE_XM:
				success = pSndFile->SaveXM(fileName, 0); 
				break;
			case MOD_TYPE_IT:
				success = (pSndFile->m_dwSongFlags & SONG_ITPROJECT) ? 
						   pSndFile->SaveITProject(fileName) : 
						   pSndFile->SaveIT(fileName, 0); 
				break;
			case MOD_TYPE_MPT:
				success = pSndFile->SaveMPT(fileName, 0);
				break;
			//default:
				//Do nothing
		}
	}
	return success;
}

void CAutoSaver::CleanUpBackups(CModDoc *pModDoc)
{
	CString path;
	
	if (m_bUseOriginalPath) {
		if (pModDoc->m_bHasValidPath) { // Check that the file has a user-chosen path
			CString fullPath = pModDoc->GetPathName();
			path = fullPath.Left(fullPath.GetLength()-pModDoc->GetTitle().GetLength()); //remove file name if necessary
		} else {
			path = CMainFrame::m_csExecutablePath + "\\";
		}
	} else {
		path = m_csPath;
	}

	CString searchPattern = path + pModDoc->GetTitle() + ".AutoSave.*";

	CFileFind finder;
	BOOL bResult = finder.FindFile(searchPattern);
	CArray<CString> foundfiles;
	
	while(bResult) {
		bResult = finder.FindNextFile();
		foundfiles.Add(path+finder.GetFileName());
	}
	finder.Close();
	
	std::sort(foundfiles.GetData(), foundfiles.GetData() + foundfiles.GetSize());
	while (foundfiles.GetSize()>m_nBackupHistory) {
		try	{
			CString toRemove=foundfiles[0];
			CFile::Remove(toRemove);
		} catch (CFileException* pEx){}
		foundfiles.RemoveAt(0);
	}
	
}


///////////////////////////////////////////////////////////////////////////////////////
// CAutoSaverGUI dialog : AutoSaver GUI
///////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CAutoSaverGUI, CPropertyPage)
CAutoSaverGUI::CAutoSaverGUI(CAutoSaver* pAutoSaver)
	: CPropertyPage(CAutoSaverGUI::IDD)
{
	m_pAutoSaver = pAutoSaver;
}

CAutoSaverGUI::~CAutoSaverGUI()
{
}

void CAutoSaverGUI::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAutoSaverGUI, CPropertyPage)
	ON_BN_CLICKED(IDC_AUTOSAVE_BROWSE, OnBnClickedAutosaveBrowse)
	ON_BN_CLICKED(IDC_AUTOSAVE_ENABLE, OnBnClickedAutosaveEnable)
	ON_BN_CLICKED(IDC_AUTOSAVE_USEORIGDIR, OnBnClickedAutosaveUseorigdir)
	ON_BN_CLICKED(IDC_AUTOSAVE_USECUSTOMDIR, OnBnClickedAutosaveUseorigdir)
	ON_EN_UPDATE(IDC_AUTOSAVE_PATH, OnSettingsChanged)
	ON_EN_UPDATE(IDC_AUTOSAVE_HISTORY, OnSettingsChanged)
	ON_EN_UPDATE(IDC_AUTOSAVE_INTERVAL, OnSettingsChanged)
END_MESSAGE_MAP()


// CAutoSaverGUI message handlers

BOOL CAutoSaverGUI::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CheckDlgButton(IDC_AUTOSAVE_ENABLE, m_pAutoSaver->IsEnabled()?BST_CHECKED:BST_UNCHECKED);
	//SetDlgItemText(IDC_AUTOSAVE_FNTEMPLATE, m_pAutoSaver->GetFilenameTemplate());
	SetDlgItemInt(IDC_AUTOSAVE_HISTORY, m_pAutoSaver->GetHistoryDepth()); //TODO
	SetDlgItemText(IDC_AUTOSAVE_PATH, m_pAutoSaver->GetPath());
	SetDlgItemInt(IDC_AUTOSAVE_INTERVAL, m_pAutoSaver->GetSaveInterval());
	CheckDlgButton(IDC_AUTOSAVE_USEORIGDIR, m_pAutoSaver->GetUseOriginalPath()?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_AUTOSAVE_USECUSTOMDIR, m_pAutoSaver->GetUseOriginalPath()?BST_UNCHECKED:BST_CHECKED);

	//enable/disable stuff as appropriate
	OnBnClickedAutosaveEnable();
	OnBnClickedAutosaveUseorigdir();

	return TRUE;
}


void CAutoSaverGUI::OnOK()
{
	CString tempPath;
	IsDlgButtonChecked(IDC_AUTOSAVE_ENABLE) ? m_pAutoSaver->Enable() : m_pAutoSaver->Disable();
	m_pAutoSaver->SetFilenameTemplate(""); //TODO
	m_pAutoSaver->SetHistoryDepth(GetDlgItemInt(IDC_AUTOSAVE_HISTORY));
	m_pAutoSaver->SetSaveInterval(GetDlgItemInt(IDC_AUTOSAVE_INTERVAL));
	m_pAutoSaver->SetUseOriginalPath(IsDlgButtonChecked(IDC_AUTOSAVE_USEORIGDIR));
	GetDlgItemText(IDC_AUTOSAVE_PATH, tempPath);
	if (!tempPath.IsEmpty() && (tempPath.Right(1)!="\\"))
		tempPath.Append("\\");
	m_pAutoSaver->SetPath(tempPath);

	CPropertyPage::OnOK();
}

void CAutoSaverGUI::OnBnClickedAutosaveBrowse()
{
	CHAR szPath[_MAX_PATH] = "";
	BROWSEINFO bi;

	GetDlgItemText(IDC_AUTOSAVE_PATH, szPath, sizeof(szPath));
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = szPath;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pid = SHBrowseForFolder(&bi);
	if (pid != NULL)
	{
		SHGetPathFromIDList(pid, szPath);
		SetDlgItemText(IDC_AUTOSAVE_PATH, szPath);
		OnSettingsChanged();
	}
}


void CAutoSaverGUI::OnBnClickedAutosaveEnable()
{
	bool enabled = IsDlgButtonChecked(IDC_AUTOSAVE_ENABLE);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_INTERVAL), enabled);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_HISTORY), enabled);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_USEORIGDIR), enabled);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_USECUSTOMDIR), enabled);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_PATH), enabled);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_BROWSE), enabled);
	OnSettingsChanged();
	return;
}

void CAutoSaverGUI::OnBnClickedAutosaveUseorigdir()
{
	if (IsDlgButtonChecked(IDC_AUTOSAVE_ENABLE)) {
		bool enabled = IsDlgButtonChecked(IDC_AUTOSAVE_USEORIGDIR);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_PATH), !enabled);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_BROWSE), !enabled);
		OnSettingsChanged();
	}
	return;
}

void CAutoSaverGUI::OnSettingsChanged() {
	SetModified(TRUE);
}

BOOL CAutoSaverGUI::OnSetActive()
//--------------------------------
{
	CMainFrame::m_nLastOptionsPage = OPTIONS_PAGE_AUTOSAVE;
	return CPropertyPage::OnSetActive();
}

BOOL CAutoSaverGUI::OnKillActive() 
//---------------------------------
{
	CString path;
	GetDlgItemText(IDC_AUTOSAVE_PATH, path);
	if (!path.IsEmpty() && (path.Right(1)!="\\")) {
		path.Append("\\");
	}
	bool pathIsOK = !_access(path, 0);

	if (!pathIsOK && IsDlgButtonChecked(IDC_AUTOSAVE_ENABLE) && !IsDlgButtonChecked(IDC_AUTOSAVE_USEORIGDIR))	{
		::AfxMessageBox("Error: backup path does not exist.", MB_OK|MB_ICONEXCLAMATION);
		::SetFocus(::GetDlgItem(m_hWnd, IDC_AUTOSAVE_PATH));
		return 0;
	}

	return CPropertyPage::OnKillActive();
}