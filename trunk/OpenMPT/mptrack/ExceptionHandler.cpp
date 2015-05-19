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
#include "../common/WriteMemoryDump.h"
#include "../common/version.h"
#include "../common/mptFileIO.h"

#include <WinIoCtl.h>


OPENMPT_NAMESPACE_BEGIN


bool ExceptionHandler::fullMemDump = false;

bool ExceptionHandler::stopSoundDeviceOnCrash = true;
bool ExceptionHandler::stopSoundDeviceBeforeDump = false;


enum DumpMode
{
	DumpModeCrash   = 0,  // crash
	DumpModeWarning = 1,  // assert
	DumpModeDebug   = 2,  // debug output (e.g. trace log)
};


struct CrashOutputDirectory
{
	bool valid;
	mpt::PathString path;
	CrashOutputDirectory()
		: valid(true)
	{
		const mpt::PathString timestampDir = mpt::PathString::FromCStringSilent((CTime::GetCurrentTime()).Format("%Y-%m-%d %H.%M.%S\\"));
		// Create a crash directory
		path = Util::GetTempDirectory() + MPT_PATHSTRING("OpenMPT Crash Files\\");
		if(!path.IsDirectory())
		{
			CreateDirectoryW(path.AsNative().c_str(), nullptr);
		}
		DWORD attributes = GetFileAttributesW(path.AsNative().c_str());
		if(attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_COMPRESSED))
		{
			// Set compressed attribute in order to save disk space.
			// Debugging information should clutter the users computer as little as possible.
			// Performance is not important here.
			// Ignore any errors.
			HANDLE hDir = CreateFileW(path.AsNative().c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if(hDir != INVALID_HANDLE_VALUE)
			{
				USHORT format = COMPRESSION_FORMAT_DEFAULT;
				DWORD dummy = 0;
				/* BOOL result = */ DeviceIoControl(hDir, FSCTL_SET_COMPRESSION, (LPVOID)&format, sizeof(format), NULL, 0, &dummy /*required*/ , NULL);
				CloseHandle(hDir);
			}
			// Compression will be inherited by children directories and files automatically.
		}
		path += timestampDir;
		if(!path.IsDirectory())
		{
			if(!CreateDirectoryW(path.AsNative().c_str(), nullptr))
			{
				valid = false;
			}
		}
	}
};


class DebugReporter
{
private:
	bool stateFrozen;
	const DumpMode mode;
	const CrashOutputDirectory crashDirectory;
	bool writtenMiniDump;
	bool writtenTraceLog;
	int rescuedFiles;
private:
	static bool FreezeState(DumpMode mode);
	static bool Cleanup(DumpMode mode);
	bool GenerateDump(_EXCEPTION_POINTERS *pExceptionInfo);
	bool GenerateTraceLog();
	int RescueFiles();
	bool HasWrittenDebug() const { return writtenMiniDump || writtenTraceLog; }
	static void StopSoundDevice();
public:
	DebugReporter(DumpMode mode, _EXCEPTION_POINTERS *pExceptionInfo);
	~DebugReporter();
	void ReportError(mpt::ustring errorMessage);
};


DebugReporter::DebugReporter(DumpMode mode, _EXCEPTION_POINTERS *pExceptionInfo)
//------------------------------------------------------------------------------
	: stateFrozen(FreezeState(mode))
	, mode(mode)
	, writtenMiniDump(false)
	, writtenTraceLog(false)
	, rescuedFiles(0)
{
	if(mode == DumpModeCrash || mode == DumpModeWarning)
	{
		writtenMiniDump = GenerateDump(pExceptionInfo);
	}
	if(mode == DumpModeCrash || mode == DumpModeWarning || mode == DumpModeDebug)
	{
		writtenTraceLog = GenerateTraceLog();
	}
	if(mode == DumpModeCrash || mode == DumpModeWarning)
	{
		rescuedFiles = RescueFiles();
	}
}


DebugReporter::~DebugReporter()
//-----------------------------
{
	Cleanup(mode);
}


bool DebugReporter::GenerateDump(_EXCEPTION_POINTERS *pExceptionInfo)
//-------------------------------------------------------------------
{
	return WriteMemoryDump(pExceptionInfo, (crashDirectory.path + MPT_PATHSTRING("crash.dmp")).AsNative().c_str(), ExceptionHandler::fullMemDump);
}


bool DebugReporter::GenerateTraceLog()
//------------------------------------
{
	return mpt::log::Trace::Dump(crashDirectory.path + MPT_PATHSTRING("trace.log"));
}


// Rescue modified files...
int DebugReporter::RescueFiles()
//------------------------------
{
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
				CTrackApp::OpenDirectory(crashDirectory.path);
			}

			mpt::PathString filename;
			filename += crashDirectory.path;
			filename += mpt::PathString::FromWide(mpt::ToWString(++numFiles));
			filename += MPT_PATHSTRING("_");
			filename += mpt::PathString::FromCStringSilent(pModDoc->GetTitle()).SanitizeComponent();
			filename += MPT_PATHSTRING(".");
			filename += mpt::PathString::FromUTF8(pModDoc->GetSoundFile()->GetModSpecifications().fileExtension);

			try
			{
				pModDoc->OnSaveDocument(filename);
			} catch(...)
			{
				continue;
			}
		}
	}
	return numFiles;
}


void DebugReporter::ReportError(mpt::ustring errorMessage)
//---------------------------------------------------
{

	if(!crashDirectory.valid)
	{
		errorMessage += MPT_ULITERAL("\n\n");
		errorMessage += MPT_ULITERAL("Could not create the following directory for saving debug information and modified files to:\n");
		errorMessage += crashDirectory.path.ToUnicode();
	}

	if(HasWrittenDebug())
	{
		errorMessage += MPT_ULITERAL("\n\n");
		errorMessage += MPT_ULITERAL("Debug information has been saved to\n");
		errorMessage += crashDirectory.path.ToUnicode();
	}

	if(rescuedFiles > 0)
	{
		errorMessage += MPT_ULITERAL("\n\n");
		if(rescuedFiles == 1)
		{
			errorMessage += MPT_ULITERAL("1 modified file has been rescued, but it cannot be guaranteed that it is still intact.");
		} else
		{
			errorMessage += mpt::String::Print(MPT_USTRING("%1 modified files have been rescued, but it cannot be guaranteed that they are still intact."), rescuedFiles);
		}
	}

	errorMessage += MPT_ULITERAL("\n\n");
	errorMessage += mpt::String::Print(MPT_USTRING("OpenMPT %1 (%2)\n")
		, mpt::ToUnicode(mpt::CharsetUTF8, MptVersion::GetVersionStringExtended())
		, mpt::ToUnicode(mpt::CharsetUTF8, MptVersion::GetVersionUrlString())
		);

	{
		mpt::ofstream f(crashDirectory.path + MPT_PATHSTRING("error.txt"), std::ios::binary);
		f.imbue(std::locale::classic());
		f << mpt::String::Replace(mpt::ToCharset(mpt::CharsetUTF8, errorMessage), "\n", "\r\n");
	}

	Reporting::Error(errorMessage, (mode == DumpModeWarning) ? "OpenMPT Warning" : "OpenMPT Crash", CMainFrame::GetMainFrame());

}


// Freezes the state as much aspossible in order to avoid further confusion by
// other (possibly still running) threads
bool DebugReporter::FreezeState(DumpMode mode)
//--------------------------------------------
{
	MPT_TRACE();

	// seal the trace log as early as possible
	mpt::log::Trace::Seal();

	if(mode == DumpModeCrash || mode == DumpModeWarning)
	{
		if(CMainFrame::GetMainFrame() && CMainFrame::GetMainFrame()->gpSoundDevice && CMainFrame::GetMainFrame()->gpSoundDevice->DebugIsFragileDevice())
		{
			// For fragile devices, always stop the device. Stop before the dumping if not in realtime context.
			if(!CMainFrame::GetMainFrame()->gpSoundDevice->DebugInRealtimeCallback())
			{
				StopSoundDevice();
			}
		} else
		{
			if(ExceptionHandler::stopSoundDeviceOnCrash && ExceptionHandler::stopSoundDeviceBeforeDump)
			{
				StopSoundDevice();
			}
		}
	}

	return true;
}


void DebugReporter::StopSoundDevice()
//-----------------------------------
{
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
				pMainFrame->m_NotifyTimer = 0;
			}
		} catch(...)
		{
		}
	}
}


bool DebugReporter::Cleanup(DumpMode mode)
//----------------------------------------
{
	MPT_TRACE();

	if(mode == DumpModeCrash || mode == DumpModeWarning)
	{
		if(CMainFrame::GetMainFrame() && CMainFrame::GetMainFrame()->gpSoundDevice && CMainFrame::GetMainFrame()->gpSoundDevice->DebugIsFragileDevice())
		{
			// For fragile devices, always stop the device. Stop after the dumping if in realtime context.
			if(CMainFrame::GetMainFrame()->gpSoundDevice->DebugInRealtimeCallback())
			{
				StopSoundDevice();
			}
		} else
		{
			if(ExceptionHandler::stopSoundDeviceOnCrash && !ExceptionHandler::stopSoundDeviceBeforeDump)
			{
				StopSoundDevice();
			}
		}
	}

	return true;
}


// Different entry points for different situations in which we want to dump some information


LONG ExceptionHandler::UnhandledExceptionFilter(_EXCEPTION_POINTERS *pExceptionInfo)
//----------------------------------------------------------------------------------
{
	DebugReporter report(DumpModeCrash, pExceptionInfo);

	CString errorMessage;
	errorMessage.Format(_T("Unhandled exception 0x%X at address %p occurred."), pExceptionInfo->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionAddress);
	report.ReportError(mpt::ToUnicode(errorMessage));

	// Let Windows handle the exception...
	return EXCEPTION_CONTINUE_SEARCH;
}


#if defined(MPT_ASSERT_HANDLER_NEEDED)

MPT_NOINLINE void AssertHandler(const char *file, int line, const char *function, const char *expr, const char *msg)
//------------------------------------------------------------------------------------------------------------------
{
	DebugReporter report(msg ? DumpModeWarning : DumpModeCrash, nullptr);
	if(IsDebuggerPresent())
	{
		OutputDebugString(_T("ASSERT("));
		OutputDebugString(mpt::ToWinAPI(mpt::CharsetASCII, expr).c_str());
		OutputDebugString(_T(") failed\n"));
		DebugBreak();
	} else
	{
		CStringA errorMessage;
		if(msg)
		{
			errorMessage.Format("Internal state inconsistency detected at %s(%d). This is just a warning that could potentially lead to a crash later on: %s [%s].", file, line, msg, function);
		} else
		{
			errorMessage.Format("Internal error occured at %s(%d): ASSERT(%s) failed in [%s].", file, line, expr, function);
		}
		report.ReportError(mpt::ToUnicode(mpt::CharsetLocale, errorMessage.GetString()));
	}
}

#endif MPT_ASSERT_HANDLER_NEEDED


void DebugTraceDump()
//-------------------
{
	DebugReporter report(DumpModeDebug, nullptr);
}


OPENMPT_NAMESPACE_END
