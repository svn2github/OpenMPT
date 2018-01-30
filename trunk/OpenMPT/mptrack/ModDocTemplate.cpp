/*
 * ModDocTemplate.cpp
 * ------------------
 * Purpose: CDocTemplate and CModDocManager specialization for CModDoc.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "FolderScanner.h"
#include "Mainfrm.h"
#include "Moddoc.h"
#include "ModDocTemplate.h"
#include "Reporting.h"
#include "../soundlib/plugins/PluginManager.h"

OPENMPT_NAMESPACE_BEGIN

CDocument *CModDocTemplate::OpenDocumentFile(LPCTSTR lpszPathName, BOOL addToMru, BOOL makeVisible)
{
	const mpt::PathString filename = (lpszPathName ? mpt::PathString::FromCString(lpszPathName) : mpt::PathString());

	// First, remove document from MRU list.
	if(addToMru)
	{
		theApp.RemoveMruItem(filename);
	}

	CDocument *pDoc = CMultiDocTemplate::OpenDocumentFile(filename.empty() ? nullptr : filename.ToCString().GetString(), addToMru, makeVisible);
	if(pDoc)
	{
		CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
		if (pMainFrm) pMainFrm->OnDocumentCreated(static_cast<CModDoc *>(pDoc));
	} else if(!filename.empty() && CMainFrame::GetMainFrame() && addToMru)
	{
		// Opening the document failed
		CMainFrame::GetMainFrame()->UpdateMRUList();
	}
	return pDoc;
}


CDocument *CModDocTemplate::OpenTemplateFile(const mpt::PathString &filename, bool isExampleTune)
{
	CDocument *doc = OpenDocumentFile(filename.ToCString(), isExampleTune ? TRUE : FALSE, TRUE);
	if(doc)
	{
		CModDoc *modDoc = static_cast<CModDoc *>(doc);
		// Clear path so that saving will not take place in templates/examples folder.
		modDoc->ClearFilePath();
		if(!isExampleTune)
		{
			CMultiDocTemplate::SetDefaultTitle(modDoc);
			m_nUntitledCount++;
			// Name has changed...
			CMainFrame::GetMainFrame()->UpdateTree(modDoc, GeneralHint().General());

			// Reset edit history for template files
			CSoundFile &sndFile = modDoc->GetSoundFile();
			sndFile.GetFileHistory().clear();
			sndFile.m_dwCreatedWithVersion = MptVersion::num;
			sndFile.m_dwLastSavedWithVersion = 0;
			sndFile.m_madeWithTracker.clear();
			sndFile.m_songArtist = TrackerSettings::Instance().defaultArtist;
			sndFile.m_playBehaviour = sndFile.GetDefaultPlaybackBehaviour(sndFile.GetType());
			doc->UpdateAllViews(nullptr, UpdateHint().ModType().AsLPARAM());
		} else
		{
			// Remove extension from title, so that saving the file will not suggest a filename like e.g. "example.it.it".
			const CString title = modDoc->GetTitle();
			const int dotPos = title.ReverseFind(_T('.'));
			if(dotPos >= 0)
			{
				modDoc->SetTitle(title.Left(dotPos));
			}
		}
	}
	return doc;
}


void CModDocTemplate::AddDocument(CDocument *doc)
{
	CMultiDocTemplate::AddDocument(doc);
	m_documents.insert(static_cast<CModDoc *>(doc));
}


void CModDocTemplate::RemoveDocument(CDocument *doc)
{
	CMultiDocTemplate::RemoveDocument(doc);
	m_documents.erase(static_cast<CModDoc *>(doc));
}


bool CModDocTemplate::DocumentExists(const CModDoc *doc) const
{
	return m_documents.count(const_cast<CModDoc *>(doc)) != 0;
}


CDocument *CModDocManager::OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU)
{
	const mpt::PathString filename = (lpszFileName ? mpt::PathString::FromCString(lpszFileName) : mpt::PathString());

	if(filename.IsDirectory())
	{
		FolderScanner scanner(filename, FolderScanner::kOnlyFiles | FolderScanner::kFindInSubDirectories);
		mpt::PathString file;
		CDocument *pDoc = nullptr;
		while(scanner.Next(file))
		{
			pDoc = OpenDocumentFile(file.ToCString(), bAddToMRU);
		}
		return pDoc;
	}

	if(!mpt::PathString::CompareNoCase(filename.GetFileExt(), MPT_PATHSTRING(".dll")))
	{
		CVstPluginManager *pPluginManager = theApp.GetPluginManager();
		if(pPluginManager && pPluginManager->AddPlugin(filename) != nullptr)
		{
			return nullptr;
		}
	}

	CDocument *pDoc = CDocManager::OpenDocumentFile(lpszFileName, bAddToMRU);
	if(pDoc == nullptr && !filename.empty())
	{
		if(!filename.IsFile())
		{
			Reporting::Error(mpt::cformat(_T("Unable to open \"%1\": file does not exist."))(filename.ToCString()));
			theApp.RemoveMruItem(filename);
			CMainFrame::GetMainFrame()->UpdateMRUList();
		} else
		{
			// Case: Valid path but opening failed.
			const int numDocs = theApp.GetOpenDocumentCount();
			Reporting::Notification(mpt::cformat(_T("Opening \"%1\" failed. This can happen if ")
				_T("no more modules can be opened or if the file type was not ")
				_T("recognised (currently there %2 %3 document%4 open)."))(
					filename.ToCString(), (numDocs == 1) ? _T("is") : _T("are"), numDocs, (numDocs == 1) ? _T("") : _T("s")));
		}
	}
	return pDoc;
}


BOOL CModDocManager::OnDDECommand(LPTSTR lpszCommand)
{
	BOOL bResult, bActivate;
#ifdef DDEDEBUG
	Log("OnDDECommand: %s\n", lpszCommand);
#endif
	// Handle any DDE commands recognized by your application
	// and return TRUE.  See implementation of CWinApp::OnDDEComand
	// for example of parsing the DDE command string.
	bResult = FALSE;
	bActivate = FALSE;
	if ((lpszCommand) && lpszCommand[0] && (theApp.m_pMainWnd))
	{
		std::size_t len = _tcslen(lpszCommand);
		std::vector<TCHAR> s(lpszCommand, lpszCommand + len + 1);

		len--;
		while((len > 0) && _tcschr(_T("(){}[]\'\" "), s[len]))
		{
			s[len--] = 0;
		}
		TCHAR *pszCmd = s.data();
		while (pszCmd[0] == _T('[')) pszCmd++;
		TCHAR *pszData = pszCmd;
		while ((pszData[0] != _T('(')) && (pszData[0]))
		{
			if (((BYTE)pszData[0]) <= (BYTE)' ') *pszData = 0;
			pszData++;
		}
		while ((*pszData) && (_tcschr(_T("(){}[]\'\" "), *pszData)))
		{
			*pszData = 0;
			pszData++;
		}
		// Edit/Open
		if ((!lstrcmpi(pszCmd, _T("Edit")))
		 || (!lstrcmpi(pszCmd, _T("Open"))))
		{
			if (pszData[0])
			{
				bResult = TRUE;
				bActivate = TRUE;
				OpenDocumentFile(pszData);
			}
		} else
		// New
		if (!lstrcmpi(pszCmd, _T("New")))
		{
			OpenDocumentFile(_T(""));
			bResult = TRUE;
			bActivate = TRUE;
		}
	#ifdef DDEDEBUG
		Log("%s(%s)\n", pszCmd, pszData);
	#endif
		if ((bActivate) && (theApp.m_pMainWnd->m_hWnd))
		{
			if (theApp.m_pMainWnd->IsIconic()) theApp.m_pMainWnd->ShowWindow(SW_RESTORE);
			theApp.m_pMainWnd->SetActiveWindow();
		}
	}
	// Return FALSE for any DDE commands you do not handle.
#ifdef DDEDEBUG
	if (!bResult)
	{
		Log("WARNING: failure in CModDocManager::OnDDECommand()\n");
	}
#endif
	return bResult;
}


OPENMPT_NAMESPACE_END
