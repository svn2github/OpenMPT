/*
 * ExceptionHandler.cpp
 * --------------------
 * Purpose: Code for handling crashes (unhandled exceptions) in OpenMPT.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Mainfrm.h"
#include "../sounddev/SoundDevice.h"
#include "Moddoc.h"
#include <shlwapi.h>
#include "ExceptionHandler.h"
#include "dbghelp.h"
#include "../common/version.h"


bool ExceptionHandler::fullMemDump = false;


typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);


enum DumpMode
{
	DumpModeCrash   = 0,
	DumpModeWarning = 1,
};

static void GenerateDump(CString &errorMessage, _EXCEPTION_POINTERS *pExceptionInfo=NULL, DumpMode mode=DumpModeCrash)
//--------------------------------------------------------------------------------------------------------------------
{
	CMainFrame* pMainFrame = CMainFrame::GetMainFrame();

	const mpt::PathString timestampDir = mpt::PathString::FromWide(mpt::ToWide((CTime::GetCurrentTime()).Format("%Y-%m-%d %H.%M.%S\\")));
	mpt::PathString baseRescuePath;
	{
		// Create a crash directory
		WCHAR tempPath[MAX_PATH];
		GetTempPathW(CountOf(tempPath), tempPath);
		baseRescuePath = mpt::PathString::FromNative(tempPath) + MPT_PATHSTRING("OpenMPT Crash Files\\");
		if(!PathIsDirectoryW(baseRescuePath.AsNative().c_str()))
		{
			CreateDirectoryW(baseRescuePath.AsNative().c_str(), nullptr);
		}
		baseRescuePath += timestampDir;
		if(!PathIsDirectoryW(baseRescuePath.AsNative().c_str()) && !CreateDirectoryW(baseRescuePath.AsNative().c_str(), nullptr))
		{
			errorMessage.AppendFormat("\n\nCould not create the following directory for saving debug information and modified files to:\n%s", mpt::ToCString(baseRescuePath.ToWide()));
		}
	}

	// Create minidump...
	HMODULE hDll = ::LoadLibraryW(L"DBGHELP.DLL");
	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			const mpt::PathString filename = baseRescuePath + MPT_PATHSTRING("crash.dmp");

			HANDLE hFile = ::CreateFileW(filename.AsNative().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				if(pExceptionInfo)
				{
					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;
				}

				pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
					ExceptionHandler::fullMemDump ?
						(MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithProcessThreadData | MiniDumpWithFullMemoryInfo
#if MPT_COMPILER_MSVC && MPT_MSVC_AT_LEAST(2010,0)
						| MiniDumpIgnoreInaccessibleMemory | MiniDumpWithTokenInformation
#endif
						)
					:
						MiniDumpNormal,
					pExceptionInfo ? &ExInfo : NULL, NULL, NULL);
				::CloseHandle(hFile);

				errorMessage.AppendFormat("\n\nDebug information has been saved to\n%s", baseRescuePath);
			}
		}
		::FreeLibrary(hDll);
	}

	// Rescue modified files...
	int numFiles = 0;
	std::vector<CModDoc *> documents = theApp.GetOpenDocuments();
	for(std::vector<CModDoc *>::iterator doc = documents.begin(); doc != documents.end(); doc++)
	{
		CModDoc *pModDoc = *doc;
		if(pModDoc->IsModified() && pModDoc->GetSoundFile() != nullptr)
		{
			if(numFiles == 0)
			{
				// Show the rescue directory in Explorer...
				CTrackApp::OpenDirectory(baseRescuePath);
			}
			CString filename;
			filename.Format("%s%d_%s.%s", baseRescuePath, ++numFiles, pModDoc->GetTitle(), pModDoc->GetSoundFile()->GetModSpecifications().fileExtension);

			try
			{
				pModDoc->OnSaveDocument(filename);
			} catch(...)
			{
				continue;
			}
		}
	}

	if(numFiles > 0)
	{
		errorMessage.AppendFormat("\n\n%d modified file%s been rescued, but it cannot be guaranteed that %s still intact.", numFiles, (numFiles == 1 ? " has" : "s have"), (numFiles == 1 ? "it is" : "they are"));
	}
	
	errorMessage.AppendFormat("\n\nOpenMPT %s (%s)\n",
		MptVersion::GetVersionStringExtended().c_str(),
		MptVersion::GetVersionUrlString().c_str()
		);

	if(mode == DumpModeWarning)
	{
		Reporting::Error(errorMessage, "OpenMPT Warning", pMainFrame);
	} else
	{
		Reporting::Error(errorMessage, "OpenMPT Crash", pMainFrame);
	}

}


// Try to close the audio device and rescue unsaved work if an unhandled exception occours...
LONG ExceptionHandler::UnhandledExceptionFilter(_EXCEPTION_POINTERS *pExceptionInfo)
//----------------------------------------------------------------------------------
{
	// Shut down audio device...
	CMainFrame* pMainFrame = CMainFrame::GetMainFrame();
	if(pMainFrame)
	{
		try
		{
			if(pMainFrame->gpSoundDevice)
			{
				pMainFrame->gpSoundDevice->Close();
			}
			if(pMainFrame->m_NotifyTimer)
			{
				pMainFrame->KillTimer(pMainFrame->m_NotifyTimer);
				pMainFrame->m_NotifyTimer =  0;
			}
		} catch(...)
		{
		}
	}

	CString errorMessage;
	errorMessage.Format("Unhandled exception 0x%X at address %p occoured.", pExceptionInfo->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionAddress);

	GenerateDump(errorMessage, pExceptionInfo);

	// Let Windows handle the exception...
	return EXCEPTION_CONTINUE_SEARCH;
}


#if defined(MPT_ASSERT_HANDLER_NEEDED)

noinline void AssertHandler(const char *file, int line, const char *function, const char *expr, const char *msg)
//--------------------------------------------------------------------------------------------------------------
{
	if(IsDebuggerPresent())
	{
		OutputDebugString("ASSERT(");
		OutputDebugString(expr);
		OutputDebugString(") failed\n");
		DebugBreak();
	} else
	{
		if(msg)
		{
			CString errorMessage;
			errorMessage.Format("Internal state inconsistency detected at %s(%d). This is just a warning that could potentially lead to a crash later on: %s [%s].", file, line, msg, function);
			GenerateDump(errorMessage, NULL, DumpModeWarning);
		} else
		{
			CString errorMessage;
			errorMessage.Format("Internal error occured at %s(%d): ASSERT(%s) failed in [%s].", file, line, expr, function);
			GenerateDump(errorMessage);
		}
	}
}

#endif MPT_ASSERT_HANDLER_NEEDED

