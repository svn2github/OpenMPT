// mptrack.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "mptrack.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "moddoc.h"
#include "globals.h"
#include "dlsbank.h"
#include "snddev.h"
#include "vstplug.h"
#include "CreditStatic.h"
#include "hyperEdit.h"
#include "commctrl.h"
#include "version.h"
#include "test/test.h"
#include <afxadv.h>
#include <shlwapi.h>
#include "UpdateCheck.h"
#include "../common/StringFixer.h"
#include "ExceptionHandler.h"

// rewbs.memLeak
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//end  rewbs.memLeak

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only CTrackApp object

CTrackApp theApp;


/////////////////////////////////////////////////////////////////////////////
// Document Template

//=============================================
class CModDocTemplate: public CMultiDocTemplate
//=============================================
{
public:
	CModDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass):
		CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass) {}
	
	#if (_MSC_VER < MSVC_VER_2010)
		virtual CDocument* OpenDocumentFile(LPCTSTR path, BOOL makeVisible = TRUE);
	#else
		virtual CDocument* OpenDocumentFile(LPCTSTR path, BOOL addToMru = TRUE, BOOL makeVisible = TRUE);
	#endif
};

#if (_MSC_VER < MSVC_VER_2010)
	CDocument *CModDocTemplate::OpenDocumentFile(LPCTSTR path, BOOL makeVisible)
#else
	CDocument *CModDocTemplate::OpenDocumentFile(LPCTSTR path, BOOL addToMru, BOOL makeVisible)
#endif
//-----------------------------------------------------------------------------------------
{
	if (path)
	{
		TCHAR s[_MAX_EXT];
		_tsplitpath(path, NULL, NULL, NULL, s);
		if (!_tcsicmp(s, _TEXT(".dll")))
		{
			CVstPluginManager *pPluginManager = theApp.GetPluginManager();
			if (pPluginManager)
			{
				pPluginManager->AddPlugin(path);
				return NULL;
			}
		}
	}

	#if (_MSC_VER < MSVC_VER_2010)
		CDocument *pDoc = CMultiDocTemplate::OpenDocumentFile(path, makeVisible); 
	#else
		CDocument *pDoc = CMultiDocTemplate::OpenDocumentFile(path, addToMru, makeVisible);
	#endif
	if (pDoc)
	{
		CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
		if (pMainFrm) pMainFrm->OnDocumentCreated(static_cast<CModDoc *>(pDoc));
	}
	else //Case: pDoc == 0, opening document failed.
	{
		if(path != NULL)
		{
			if(PathFileExists(path) == FALSE)
			{
				CString str;
				str.Format(GetStrI18N(_TEXT("Unable to open \"%s\": file does not exist.")), path);
				Reporting::Error(str);
			}
			else //Case: Valid path but opening fails.
			{		
				const int nOdc = AfxGetApp()->m_pDocManager->GetOpenDocumentCount();
				CString str;
				str.Format(GetStrI18N(_TEXT("Opening \"%s\" failed. This can happen if "
					"no more documents can be opened or if the file type was not "
					"recognised. If the former is true, it's "
					"recommended to close some documents as otherwise crash is likely"
					"(currently there %s %d document%s open).")),
					path, (nOdc == 1) ? "is" : "are", nOdc, (nOdc == 1) ? "" : "s");
				Reporting::Notification(str);
			}
		}
	}
	return pDoc;
}


#ifdef _DEBUG
#define DDEDEBUG
#endif


//======================================
class CModDocManager: public CDocManager
//======================================
{
public:
	CModDocManager() {}
	virtual BOOL OnDDECommand(LPTSTR lpszCommand);
};


BOOL CModDocManager::OnDDECommand(LPTSTR lpszCommand)
//---------------------------------------------------
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
	if ((lpszCommand) && (*lpszCommand) && (theApp.m_pMainWnd))
	{
		CHAR s[_MAX_PATH], *pszCmd, *pszData;
		int len;

		lstrcpyn(s, lpszCommand, CountOf(s));
		len = strlen(s) - 1;
		while ((len > 0) && (strchr("(){}[]\'\" ", s[len]))) s[len--] = 0;
		pszCmd = s;
		while (pszCmd[0] == '[') pszCmd++;
		pszData = pszCmd;
		while ((pszData[0] != '(') && (pszData[0]))
		{
			if (((BYTE)pszData[0]) <= (BYTE)0x20) *pszData = 0;
			pszData++;
		}
		while ((*pszData) && (strchr("(){}[]\'\" ", *pszData)))
		{
			*pszData = 0;
			pszData++;
		}
		// Edit/Open
		if ((!lstrcmpi(pszCmd, "Edit"))
		 || (!lstrcmpi(pszCmd, "Open")))
		{
			if (pszData[0])
			{
				bResult = TRUE;
				bActivate = TRUE;
				OpenDocumentFile(pszData);
			}
		} else
		// New
		if (!lstrcmpi(pszCmd, "New"))
		{
			OpenDocumentFile(NULL);
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


/////////////////////////////////////////////////////////////////////////////
// Common Tables

const LPCSTR szNoteNames[12] =
{
	"C-", "C#", "D-", "D#", "E-", "F-",
	"F#", "G-", "G#", "A-", "A#", "B-"
};

const LPCTSTR szDefaultNoteNames[NOTE_MAX] = {
	TEXT("C-0"), TEXT("C#0"), TEXT("D-0"), TEXT("D#0"), TEXT("E-0"), TEXT("F-0"), TEXT("F#0"), TEXT("G-0"), TEXT("G#0"), TEXT("A-0"), TEXT("A#0"), TEXT("B-0"),
	TEXT("C-1"), TEXT("C#1"), TEXT("D-1"), TEXT("D#1"), TEXT("E-1"), TEXT("F-1"), TEXT("F#1"), TEXT("G-1"), TEXT("G#1"), TEXT("A-1"), TEXT("A#1"), TEXT("B-1"),
	TEXT("C-2"), TEXT("C#2"), TEXT("D-2"), TEXT("D#2"), TEXT("E-2"), TEXT("F-2"), TEXT("F#2"), TEXT("G-2"), TEXT("G#2"), TEXT("A-2"), TEXT("A#2"), TEXT("B-2"),
	TEXT("C-3"), TEXT("C#3"), TEXT("D-3"), TEXT("D#3"), TEXT("E-3"), TEXT("F-3"), TEXT("F#3"), TEXT("G-3"), TEXT("G#3"), TEXT("A-3"), TEXT("A#3"), TEXT("B-3"),
	TEXT("C-4"), TEXT("C#4"), TEXT("D-4"), TEXT("D#4"), TEXT("E-4"), TEXT("F-4"), TEXT("F#4"), TEXT("G-4"), TEXT("G#4"), TEXT("A-4"), TEXT("A#4"), TEXT("B-4"),
	TEXT("C-5"), TEXT("C#5"), TEXT("D-5"), TEXT("D#5"), TEXT("E-5"), TEXT("F-5"), TEXT("F#5"), TEXT("G-5"), TEXT("G#5"), TEXT("A-5"), TEXT("A#5"), TEXT("B-5"),
	TEXT("C-6"), TEXT("C#6"), TEXT("D-6"), TEXT("D#6"), TEXT("E-6"), TEXT("F-6"), TEXT("F#6"), TEXT("G-6"), TEXT("G#6"), TEXT("A-6"), TEXT("A#6"), TEXT("B-6"),
	TEXT("C-7"), TEXT("C#7"), TEXT("D-7"), TEXT("D#7"), TEXT("E-7"), TEXT("F-7"), TEXT("F#7"), TEXT("G-7"), TEXT("G#7"), TEXT("A-7"), TEXT("A#7"), TEXT("B-7"),
	TEXT("C-8"), TEXT("C#8"), TEXT("D-8"), TEXT("D#8"), TEXT("E-8"), TEXT("F-8"), TEXT("F#8"), TEXT("G-8"), TEXT("G#8"), TEXT("A-8"), TEXT("A#8"), TEXT("B-8"),
	TEXT("C-9"), TEXT("C#9"), TEXT("D-9"), TEXT("D#9"), TEXT("E-9"), TEXT("F-9"), TEXT("F#9"), TEXT("G-9"), TEXT("G#9"), TEXT("A-9"), TEXT("A#9"), TEXT("B-9"),
};


TCHAR CTrackApp::m_szExePath[_MAX_PATH] = TEXT("");

/////////////////////////////////////////////////////////////////////////////
// MPTRACK Command Line options

//================================================
class CMPTCommandLineInfo: public CCommandLineInfo
//================================================
{
public:
	bool m_bNoAcm, m_bNoDls, m_bNoMp3, m_bSafeMode, m_bWavEx, m_bNoPlugins, m_bDebug,
		 m_bPortable, m_bNoSettingsOnNewVersion;

public:
	CMPTCommandLineInfo()
	{ 
		m_bNoAcm = m_bNoDls = m_bNoMp3 = m_bSafeMode = m_bWavEx = 
		m_bNoPlugins = m_bDebug = m_bNoSettingsOnNewVersion = m_bPortable = false; 
	}
	virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
};


void CMPTCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
//-----------------------------------------------------------------------------
{
	if ((lpszParam) && (bFlag))
	{
		if (!lstrcmpi(lpszParam, "nologo")) { m_bShowSplash = FALSE; return; } else
		if (!lstrcmpi(lpszParam, "nodls")) { m_bNoDls = true; return; } else
		if (!lstrcmpi(lpszParam, "noacm")) { m_bNoAcm = true; return; } else
		if (!lstrcmpi(lpszParam, "nomp3")) { m_bNoMp3 = true; return; } else
		if (!lstrcmpi(lpszParam, "wavex")) { m_bWavEx = true; return; } else
		if (!lstrcmpi(lpszParam, "noplugs")) { m_bNoPlugins = true; return; } else
		if (!lstrcmpi(lpszParam, "debug")) { m_bDebug = true; return; } else
		if (!lstrcmpi(lpszParam, "portable")) { m_bPortable = true; return; } else
		if (!lstrcmpi(lpszParam, "noSettingsOnNewVersion")) { m_bNoSettingsOnNewVersion = true; return; }
	}
	CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}


/////////////////////////////////////////////////////////////////////////////
// Midi Library

LPMIDILIBSTRUCT CTrackApp::glpMidiLibrary = NULL;

BOOL CTrackApp::ImportMidiConfig(LPCSTR lpszConfigFile, BOOL bNoWarn)
//-------------------------------------------------------------------
{
	TCHAR szFileName[_MAX_PATH], s[_MAX_PATH], szUltraSndPath[_MAX_PATH];
	
	if ((!lpszConfigFile) || (!lpszConfigFile[0])) return FALSE;
	if (!glpMidiLibrary)
	{
		glpMidiLibrary = new MIDILIBSTRUCT;
		if (!glpMidiLibrary) return FALSE;
		MemsetZero(*glpMidiLibrary);
	}
	if (CDLSBank::IsDLSBank(lpszConfigFile))
	{
		ConfirmAnswer result = cnfYes;
		if (!bNoWarn)
		{
			result = Reporting::Confirm("You are about to replace the current MIDI library:\n"
									"Do you want to replace only the missing instruments? (recommended)",
									"Warning", true);
		}
		if (result == cnfCancel) return FALSE;
		const bool bReplaceAll = (result == cnfNo);
		CDLSBank dlsbank;
		if (dlsbank.Open(lpszConfigFile))
		{
			for (UINT iIns=0; iIns<256; iIns++)
			{
				if ((bReplaceAll) || (!glpMidiLibrary->MidiMap[iIns]) || (!glpMidiLibrary->MidiMap[iIns][0]))
				{
					DWORD dwProgram = (iIns < 128) ? iIns : 0xFF;
					DWORD dwKey = (iIns < 128) ? 0xFF : iIns & 0x7F;
					DWORD dwBank = (iIns < 128) ? 0 : F_INSTRUMENT_DRUMS;
					if (dlsbank.FindInstrument((iIns < 128) ? FALSE : TRUE,	dwBank, dwProgram, dwKey))
					{
						if (!glpMidiLibrary->MidiMap[iIns])
						{
							if ((glpMidiLibrary->MidiMap[iIns] = new CHAR[_MAX_PATH]) == NULL) break;
						}
						strcpy(glpMidiLibrary->MidiMap[iIns], lpszConfigFile);
					}
				}
			}
		}
		return TRUE;
	}
	GetPrivateProfileString(_T("Ultrasound"), _T("PatchDir"), _T(""), szUltraSndPath, CountOf(szUltraSndPath), lpszConfigFile);
	if (!strcmp(szUltraSndPath, _T(".\\"))) szUltraSndPath[0] = 0;
	if (!szUltraSndPath[0]) GetCurrentDirectory(CountOf(szUltraSndPath), szUltraSndPath);
	for (UINT iMidi=0; iMidi<256; iMidi++)
	{
		szFileName[0] = 0;
		wsprintf(s, (iMidi < 128) ? _T("Midi%d") : _T("Perc%d"), iMidi & 0x7f);
		GetPrivateProfileString(_T("Midi Library"), s, _T(""), szFileName, CountOf(szFileName), lpszConfigFile);
		// Check for ULTRASND.INI
		if (!szFileName[0])
		{
			LPCSTR pszSection = (iMidi < 128) ? _T("Melodic Patches") : _T("Drum Patches");
			wsprintf(s, _T("%d"), iMidi & 0x7f);
			GetPrivateProfileString(pszSection, s, _T(""), szFileName, CountOf(szFileName), lpszConfigFile);
			if (!szFileName[0])
			{
				pszSection = (iMidi < 128) ? _T("Melodic Bank 0") : _T("Drum Bank 0");
				GetPrivateProfileString(pszSection, s, "", szFileName, CountOf(szFileName), lpszConfigFile);
			}
			if (szFileName[0])
			{
				s[0] = 0;
				if (szUltraSndPath[0])
				{
					strcpy(s, szUltraSndPath);
					int len = strlen(s)-1;
					if ((len) && (s[len-1] != '\\')) strcat(s, _T("\\"));
				}
				_tcsncat(s, szFileName, CountOf(s));
				_tcsncat(s, ".pat", CountOf(s));
				_tcscpy(szFileName, s);
			}
		}
		if (szFileName[0])
		{
			if (!glpMidiLibrary->MidiMap[iMidi])
			{
				if ((glpMidiLibrary->MidiMap[iMidi] = new TCHAR[_MAX_PATH]) == nullptr) return FALSE;
			}
			theApp.RelativePathToAbsolute(szFileName);
			_tcscpy(glpMidiLibrary->MidiMap[iMidi], szFileName);
		}
	}
	return FALSE;
}


BOOL CTrackApp::ExportMidiConfig(LPCSTR lpszConfigFile)
//-----------------------------------------------------
{
	TCHAR szFileName[_MAX_PATH], s[128];
	
	if ((!glpMidiLibrary) || (!lpszConfigFile) || (!lpszConfigFile[0])) return FALSE;
	for(size_t iMidi = 0; iMidi < 256; iMidi++) if (glpMidiLibrary->MidiMap[iMidi])
	{
		if (iMidi < 128)
			wsprintf(s, _T("Midi%d"), iMidi);
		else
			wsprintf(s, _T("Perc%d"), iMidi & 0x7F);

		strcpy(szFileName, glpMidiLibrary->MidiMap[iMidi]);

		if(szFileName[0])
		{
			if(theApp.IsPortableMode())
				theApp.AbsolutePathToRelative(szFileName);
			if (!WritePrivateProfileString("Midi Library", s, szFileName, lpszConfigFile)) break;
		}
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// DLS Banks support

#define MPTRACK_REG_DLS		"Software\\Olivier Lapicque\\ModPlug Tracker\\DLS Banks"
CDLSBank *CTrackApp::gpDLSBanks[MAX_DLS_BANKS];


BOOL CTrackApp::LoadDefaultDLSBanks()
//-----------------------------------
{
	CHAR szFileName[MAX_PATH];
	HKEY key;

	CString storedVersion = CMainFrame::GetPrivateProfileCString("Version", "Version", "", theApp.GetConfigFileName());
	//If version number stored in INI is 1.17.02.40 or later, load DLS from INI file.
	//Else load DLS from Registry
	if (storedVersion >= "1.17.02.40")
	{
		CHAR s[MAX_PATH];
		UINT numBanks = CMainFrame::GetPrivateProfileLong("DLS Banks", "NumBanks", 0, theApp.GetConfigFileName());
		for(size_t i = 0; i < numBanks; i++)
		{
			wsprintf(s, _T("Bank%d"), i + 1);
			TCHAR szPath[_MAX_PATH];
			GetPrivateProfileString("DLS Banks", s, "", szPath, INIBUFFERSIZE, theApp.GetConfigFileName());
			theApp.RelativePathToAbsolute(szPath);
			AddDLSBank(szPath);
		}
	} else
	{
		LoadRegistryDLS();
	}

	SaveDefaultDLSBanks(); // This will avoid a crash the next time if we crash while loading the bank

	szFileName[0] = 0;
	GetSystemDirectory(szFileName, sizeof(szFileName));
	lstrcat(szFileName, "\\GM.DLS");
	if (!AddDLSBank(szFileName))
	{
		GetWindowsDirectory(szFileName, sizeof(szFileName));
		lstrcat(szFileName, "\\SYSTEM32\\DRIVERS\\GM.DLS");
		if (!AddDLSBank(szFileName))
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DirectMusic", 0, KEY_READ, &key) == ERROR_SUCCESS)
			{
				DWORD dwRegType = REG_SZ;
				DWORD dwSize = sizeof(szFileName);
				szFileName[0] = 0;
				if (RegQueryValueEx(key, "GMFilePath", NULL, &dwRegType, (LPBYTE)&szFileName, &dwSize) == ERROR_SUCCESS)
				{
					AddDLSBank(szFileName);
				}
				RegCloseKey(key);
			}
		}
	}
	if (glpMidiLibrary) ImportMidiConfig(szFileName, TRUE);

	return TRUE;
}

void CTrackApp::LoadRegistryDLS()
//-------------------------------
{
	CHAR szFileNameX[_MAX_PATH];
	HKEY keyX;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,	MPTRACK_REG_DLS, 0, KEY_READ, &keyX) == ERROR_SUCCESS)
	{
		DWORD dwRegType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		DWORD d = 0;
		if (RegQueryValueEx(keyX, "NumBanks", NULL, &dwRegType, (LPBYTE)&d, &dwSize) == ERROR_SUCCESS)
		{
			CHAR s[64];
			for (UINT i=0; i<d; i++)
			{
				wsprintf(s, "Bank%d", i+1);
				szFileNameX[0] = 0;
				dwRegType = REG_SZ;
				dwSize = sizeof(szFileNameX);
				RegQueryValueEx(keyX, s, NULL, &dwRegType, (LPBYTE)szFileNameX, &dwSize);
				AddDLSBank(szFileNameX);
			}
		}
		RegCloseKey(keyX);
	}
}


BOOL CTrackApp::SaveDefaultDLSBanks()
//-----------------------------------
{
	TCHAR s[64];
	TCHAR szPath[_MAX_PATH];
	DWORD nBanks = 0;
	for (UINT i=0; i<MAX_DLS_BANKS; i++)
	{
		
		if (!gpDLSBanks[i] || !gpDLSBanks[i]->GetFileName() || !gpDLSBanks[i]->GetFileName()[0])
			continue;
		
		_tcsncpy(szPath, gpDLSBanks[i]->GetFileName(), CountOf(szPath) - 1);
		if(theApp.IsPortableMode())
		{
			theApp.AbsolutePathToRelative(szPath);
		}

		wsprintf(s, _T("Bank%d"), nBanks+1);
		WritePrivateProfileString("DLS Banks", s, szPath, theApp.GetConfigFileName());
		nBanks++;

	}
	CMainFrame::WritePrivateProfileLong("DLS Banks", "NumBanks", nBanks, theApp.GetConfigFileName());
	return TRUE;
}


BOOL CTrackApp::RemoveDLSBank(UINT nBank)
//---------------------------------------
{
	if ((nBank >= MAX_DLS_BANKS) || (!gpDLSBanks[nBank])) return FALSE;
	delete gpDLSBanks[nBank];
	gpDLSBanks[nBank] = NULL;
	return TRUE;
}


BOOL CTrackApp::AddDLSBank(LPCSTR lpszFileName)
//---------------------------------------------
{
	if ((!lpszFileName) || (!lpszFileName[0]) || (!CDLSBank::IsDLSBank(lpszFileName))) return FALSE;
	for (UINT j=0; j<MAX_DLS_BANKS; j++) if (gpDLSBanks[j])
	{
		if (!lstrcmpi(lpszFileName, gpDLSBanks[j]->GetFileName())) return TRUE;
	}
	for (UINT i=0; i<MAX_DLS_BANKS; i++) if (!gpDLSBanks[i])
	{
		gpDLSBanks[i] = new CDLSBank;
		gpDLSBanks[i]->Open(lpszFileName);
		return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CTrackApp

MODTYPE CTrackApp::m_nDefaultDocType = MOD_TYPE_IT;
MEMORYSTATUS CTrackApp::gMemStatus;

// -> CODE#0023
// -> DESC="IT project files (.itp)"
BOOL CTrackApp::m_nProject = FALSE;
// -! NEW_FEATURE#0023

BEGIN_MESSAGE_MAP(CTrackApp, CWinApp)
	//{{AFX_MSG_MAP(CTrackApp)
	ON_COMMAND(ID_FILE_NEW,		OnFileNew)
	ON_COMMAND(ID_FILE_NEWMOD,	OnFileNewMOD)
	ON_COMMAND(ID_FILE_NEWS3M,	OnFileNewS3M)
	ON_COMMAND(ID_FILE_NEWXM,	OnFileNewXM)
	ON_COMMAND(ID_FILE_NEWIT,	OnFileNewIT)
// -> CODE#0023
// -> DESC="IT project files (.itp)"
	ON_COMMAND(ID_NEW_ITPROJECT,OnFileNewITProject)	
// -! NEW_FEATURE#0023
	ON_COMMAND(ID_NEW_MPT,		OnFileNewMPT)
	ON_COMMAND(ID_FILE_OPEN,	OnFileOpen)
	ON_COMMAND(ID_APP_ABOUT,	OnAppAbout)
	ON_COMMAND(ID_HELP_INDEX,	CWinApp::OnHelpIndex)
	ON_COMMAND(ID_HELP_FINDER,	CWinApp::OnHelpFinder)
	ON_COMMAND(ID_HELP_USING,	CWinApp::OnHelpUsing)
	ON_COMMAND(ID_HELP_SEARCH,	OnHelpSearch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrackApp construction

CTrackApp::CTrackApp()
//--------------------
{
	#if (_MSC_VER >= MSVC_VER_2005)
		_CrtSetDebugFillThreshold(0); // Disable buffer filling in secure enhanced CRT functions.
	#endif

	ExceptionHandler::RegisterMainThread();

	m_bPortableMode = false;
	m_pModTemplate = NULL;
	m_pPluginManager = NULL;
	m_bInitialized = FALSE;
	m_bExWaveSupport = FALSE;
	m_bDebugMode = FALSE;
	m_hAlternateResourceHandle = NULL;
	m_szConfigFileName[0] = 0;
	for (size_t i = 0; i < MAX_DLS_BANKS; i++) gpDLSBanks[i] = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// GetDSoundVersion

static DWORD GetDSoundVersion()
//-----------------------------
{
	DWORD dwVersion = 0x600;
	HKEY key = NULL;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DirectX", 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		CHAR szVersion[32] = "";
		DWORD dwSize = sizeof(szVersion);
		DWORD dwType = REG_SZ;
		if (RegQueryValueEx(key, "Version", NULL, &dwType, (LPBYTE)szVersion, &dwSize) == ERROR_SUCCESS)
		{
			// "4.06.03.xxxx"
			dwVersion = ((szVersion[3] - '0') << 8) | ((szVersion[5] - '0') << 4) | ((szVersion[6] - '0'));
			if (dwVersion < 0x600) dwVersion = 0x600;
			if (dwVersion > 0x800) dwVersion = 0x800;
		}
		RegCloseKey(key);
	}
	return dwVersion;
}


/////////////////////////////////////////////////////////////////////////////
// CTrackApp initialization

#ifdef WIN32	// Legacy stuff
// Move a config file called sFileName from the App's directory (or one of its sub directories specified by sSubDir) to
// %APPDATA%. If specified, it will be renamed to sNewFileName. Existing files are never overwritten.
// Returns true on success.
bool CTrackApp::MoveConfigFile(TCHAR sFileName[_MAX_PATH], TCHAR sSubDir[_MAX_PATH], TCHAR sNewFileName[_MAX_PATH])
//-----------------------------------------------------------------------------------------------------------------
{
	// copy a config file from the exe directory to the new config dirs
	TCHAR sOldPath[_MAX_PATH], sNewPath[_MAX_PATH];
	strcpy(sOldPath, m_szExePath);
	if(sSubDir[0])
		strcat(sOldPath, sSubDir);
	strcat(sOldPath, sFileName);

	strcpy(sNewPath, m_szConfigDirectory);
	if(sSubDir[0])
		strcat(sNewPath, sSubDir);
	if(sNewFileName[0])
		strcat(sNewPath, sNewFileName);
	else
		strcat(sNewPath, sFileName);

	if(PathFileExists(sNewPath) == 0 && PathFileExists(sOldPath) != 0)
	{
		return (MoveFile(sOldPath, sNewPath) != 0);
	}
	return false;
}
#endif	// WIN32 Legacy Stuff


// Set up paths were configuration data is written to. Set overridePortable to true if application's own directory should always be used.
void CTrackApp::SetupPaths(bool overridePortable)
//-----------------------------------------------
{
	if(GetModuleFileName(NULL, m_szExePath, CountOf(m_szExePath)))
	{
		TCHAR szDrive[_MAX_DRIVE] = "", szDir[_MAX_PATH] = "";
		_splitpath(m_szExePath, szDrive, szDir, NULL, NULL);
		strcpy(m_szExePath, szDrive);
		strcat(m_szExePath, szDir);

		GetFullPathName(m_szExePath, CountOf(szDir), szDir, NULL);
		strcpy(m_szExePath, szDir);
	}

	m_szConfigDirectory[0] = 0;
	// Try to find a nice directory where we should store our settings (default: %APPDATA%)
	bool bIsAppDir = overridePortable;
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, m_szConfigDirectory)))
	{
		if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, m_szConfigDirectory)))
		{
			bIsAppDir = true;
		}
	}

	// Check if the user prefers to use the app's directory
	strcpy(m_szConfigFileName, m_szExePath); // config file
	strcat(m_szConfigFileName, "mptrack.ini");
	if(GetPrivateProfileInt("Paths", "UseAppDataDirectory", 1, m_szConfigFileName) == 0)
	{
		bIsAppDir = true;
	}

	if(!bIsAppDir)
	{
		// Store our app settings in %APPDATA% or "My Files"
		strcat(m_szConfigDirectory, "\\OpenMPT\\");

		// Path doesn't exist yet, so it has to be created
		if(PathIsDirectory(m_szConfigDirectory) == 0)
		{
			CreateDirectory(m_szConfigDirectory, 0);
		}

		#ifdef WIN32	// Legacy stuff
		// Move the config files if they're still in the old place.
		MoveConfigFile("mptrack.ini");
		MoveConfigFile("plugin.cache");
		MoveConfigFile("mpt_intl.ini");
		#endif	// WIN32 Legacy Stuff
	} else
	{
		strcpy(m_szConfigDirectory, m_szExePath);
	}
	
	// Create tunings dir
	CString sTuningPath;
	sTuningPath.Format(TEXT("%stunings\\"), m_szConfigDirectory);
	CMainFrame::GetSettings().SetDefaultDirectory(sTuningPath, DIR_TUNING);

	if(PathIsDirectory(CMainFrame::GetSettings().GetDefaultDirectory(DIR_TUNING)) == 0)
	{
		CreateDirectory(CMainFrame::GetSettings().GetDefaultDirectory(DIR_TUNING), 0);
	}

	if(!bIsAppDir)
	{
		// Import old tunings
		TCHAR sOldTunings[_MAX_PATH];
		strcpy(sOldTunings, m_szExePath);
		strcat(sOldTunings, "tunings\\");

		if(PathIsDirectory(sOldTunings) != 0)
		{
			TCHAR sSearchPattern[_MAX_PATH];
			strcpy(sSearchPattern, sOldTunings);
			strcat(sSearchPattern, "*.*");
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			hFind = FindFirstFile(sSearchPattern, &FindFileData);
			if(hFind != INVALID_HANDLE_VALUE) 
			{
				do 
				{
					MoveConfigFile(FindFileData.cFileName, "tunings\\");
				} while(FindNextFile(hFind, &FindFileData) != 0);
			}
			FindClose(hFind);
			RemoveDirectory(sOldTunings);
		}
	}

	// Set up default file locations
	strcpy(m_szConfigFileName, m_szConfigDirectory); // config file
	strcat(m_szConfigFileName, "mptrack.ini");

	strcpy(m_szStringsFileName, m_szConfigDirectory); // I18N file
	strcat(m_szStringsFileName, "mpt_intl.ini");

	strcpy(m_szPluginCacheFileName, m_szConfigDirectory); // plugin cache
	strcat(m_szPluginCacheFileName, "plugin.cache");

	TCHAR szTemplatePath[MAX_PATH];
	_tcscpy(szTemplatePath, m_szConfigDirectory);
	_tcscat(szTemplatePath, _T("TemplateModules\\"));
	CMainFrame::GetSettings().SetDefaultDirectory(szTemplatePath, DIR_TEMPLATE_FILES_USER);

	m_bPortableMode = bIsAppDir;
}

BOOL CTrackApp::InitInstance()
//----------------------------
{

	// Initialize OLE MFC support
	AfxOleInit();
	// Standard initialization

	// Change the registry key under which our settings are stored.
	//SetRegistryKey(_T("Olivier Lapicque"));
	// Start loading
	BeginWaitCursor();

	//m_hAlternateResourceHandle = LoadLibrary("mpt_intl.dll");

	MemsetZero(gMemStatus);
	GlobalMemoryStatus(&gMemStatus);
#if 0
	Log("Physical: %lu\n", gMemStatus.dwTotalPhys);
	Log("Page File: %lu\n", gMemStatus.dwTotalPageFile);
	Log("Virtual: %lu\n", gMemStatus.dwTotalVirtual);
#endif
	// Allow allocations of at least 16MB
	if (gMemStatus.dwTotalPhys < 16*1024*1024) gMemStatus.dwTotalPhys = 16*1024*1024;

	CMainFrame::GetSettings().m_nSampleUndoMaxBuffer = gMemStatus.dwTotalPhys / 10; // set sample undo buffer size
	if(CMainFrame::GetSettings().m_nSampleUndoMaxBuffer < (1 << 20)) CMainFrame::GetSettings().m_nSampleUndoMaxBuffer = (1 << 20);

	ASSERT(nullptr == m_pDocManager);
	m_pDocManager = new CModDocManager();

	ASSERT((sizeof(MODCHANNEL) & 7) == 0);

	// Parse command line for standard shell commands, DDE, file open
	CMPTCommandLineInfo cmdInfo;
	if (GetDSoundVersion() >= 0x0700) cmdInfo.m_bWavEx = true;
	ParseCommandLine(cmdInfo);


	// Set up paths to store configuration in
	SetupPaths(cmdInfo.m_bPortable);

	//Force use of custom ini file rather than windowsDir\executableName.ini
	if (m_pszProfileName)
	{
		free((void *)m_pszProfileName);
	}
	m_pszProfileName = _tcsdup(m_szConfigFileName); 

	int mruListLength = GetPrivateProfileInt("Misc", "MRUListLength", 10, m_pszProfileName);
	Limit(mruListLength, 0, 15);
	LoadStdProfileSettings((UINT)mruListLength);  // Load standard INI file options (including MRU)

	// Register document templates
	m_pModTemplate = new CModDocTemplate(
		IDR_MODULETYPE,
		RUNTIME_CLASS(CModDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CModControlView));
	AddDocTemplate(m_pModTemplate);

	// Initialize Audio
	CSoundFile::InitSysInfo();
	if (CSoundFile::gdwSysInfo & SYSMIX_ENABLEMMX)
	{
		CMainFrame::GetSettings().m_dwSoundSetup |= SOUNDSETUP_ENABLEMMX;
		CMainFrame::GetSettings().m_nSrcMode = SRCMODE_SPLINE;
	}
	if (CSoundFile::gdwSysInfo & SYSMIX_MMXEX)
	{
		CMainFrame::GetSettings().m_nSrcMode = SRCMODE_POLYPHASE;
	}
	// Load Midi Library
	if (m_szConfigFileName[0]) ImportMidiConfig(m_szConfigFileName);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame(/*cmdInfo.m_csExtension*/);
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) return FALSE;
	m_pMainWnd = pMainFrame;

	if (cmdInfo.m_bShowSplash)
	{
		StartSplashScreen();
	}
	m_bDebugMode = cmdInfo.m_bDebug;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();

	// Register MOD extensions
	//RegisterExtensions();

	// Load DirectSound (if available)
	m_bExWaveSupport = cmdInfo.m_bWavEx;
	SndDevInitialize();

	// Load DLS Banks
	if (!cmdInfo.m_bNoDls) LoadDefaultDLSBanks();

	// Initialize ACM Support
	if (GetProfileInt("Settings", "DisableACM", 0)) cmdInfo.m_bNoAcm = true;
	if (!cmdInfo.m_bNoMp3) GetACMConvert().InitializeACM(cmdInfo.m_bNoAcm);

	// Initialize Plugins
	if (!cmdInfo.m_bNoPlugins) InitializeDXPlugins();

	// Initialize localized strings
	ImportLocalizedStrings();

	// Initialize CMainFrame
	pMainFrame->Initialize();
	InitCommonControls();
	m_dwLastPluginIdleCall = 0;	//rewbs.VSTCompliance
	pMainFrame->m_InputHandler->UpdateMainMenu();	//rewbs.customKeys

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
	{
		EndWaitCursor();
		StopSplashScreen();
		return FALSE;
	}

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	m_dwTimeStarted = timeGetTime();
	m_bInitialized = TRUE;

	if (CUpdateCheck::GetUpdateCheckPeriod() != 0)
	{
		CUpdateCheck *updateCheck = CUpdateCheck::Create(true);
		updateCheck->DoUpdateCheck();
	}

	// Open settings if the previous execution was with an earlier version.
	if (!cmdInfo.m_bNoSettingsOnNewVersion && MptVersion::ToNum(CMainFrame::GetSettings().gcsPreviousVersion) < MptVersion::num)
	{
		StopSplashScreen();
		m_pMainWnd->PostMessage(WM_COMMAND, ID_VIEW_OPTIONS);
	}

	EndWaitCursor();

#ifdef ENABLE_TESTS
	MptTest::DoTests();
#endif

	return TRUE;
}


int CTrackApp::ExitInstance()
//---------------------------
{
	SndDevUninitialize();
	if (glpMidiLibrary)
	{
		if (m_szConfigFileName[0]) ExportMidiConfig(m_szConfigFileName);
		for (UINT iMidi=0; iMidi<256; iMidi++)
		{
			if (glpMidiLibrary->MidiMap[iMidi])
			{
				delete[] glpMidiLibrary->MidiMap[iMidi];
			}
		}
		delete glpMidiLibrary;
		glpMidiLibrary = NULL;
	}
	SaveDefaultDLSBanks();
	for (UINT i=0; i<MAX_DLS_BANKS; i++)
	{
		if (gpDLSBanks[i])
		{
			delete gpDLSBanks[i];
			gpDLSBanks[i] = NULL;
		}
	}

	// Uninitialize Plugins
	UninitializeDXPlugins();

	// Uninitialize ACM
	GetACMConvert().UninitializeACM();

	return CWinApp::ExitInstance();
}


////////////////////////////////////////////////////////////////////////////////
// Chords

void CTrackApp::LoadChords(PMPTCHORD pChords)
//-------------------------------------------
{	
	if (!m_szConfigFileName[0]) return;
	for (UINT i=0; i<3*12; i++)
	{
		LONG chord;
		if ((chord = GetPrivateProfileInt("Chords", szDefaultNoteNames[i], -1, m_szConfigFileName)) >= 0)
		{
			if ((chord & 0xFFFFFFC0) || (!pChords[i].notes[0]))
			{
				pChords[i].key = (BYTE)(chord & 0x3F);
				pChords[i].notes[0] = (BYTE)((chord >> 6) & 0x3F);
				pChords[i].notes[1] = (BYTE)((chord >> 12) & 0x3F);
				pChords[i].notes[2] = (BYTE)((chord >> 18) & 0x3F);
			}
		}
	}
}


void CTrackApp::SaveChords(PMPTCHORD pChords)
//-------------------------------------------
{
	CHAR s[64];
	
	if (!m_szConfigFileName[0]) return;
	for (UINT i=0; i<3*12; i++)
	{
		wsprintf(s, "%d", (pChords[i].key) | (pChords[i].notes[0] << 6) | (pChords[i].notes[1] << 12) | (pChords[i].notes[2] << 18));
		if (!WritePrivateProfileString("Chords", szDefaultNoteNames[i], s, m_szConfigFileName)) break;
	}
}


////////////////////////////////////////////////////////////////////////////////
// App Messages


void CTrackApp::OnFileNew()
//-------------------------
{
	if (!m_bInitialized) return;

	// Default module type
	MODTYPE nNewType = CMainFrame::GetSettings().defaultModType;
	bool bIsProject = false;

	// Get active document to make the new module of the same type
	CModDoc *pModDoc = CMainFrame::GetMainFrame()->GetActiveDoc();
	if(pModDoc != nullptr)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		if(pSndFile != nullptr)
		{
			nNewType = pSndFile->GetBestSaveFormat();
			bIsProject = ((pSndFile->m_dwSongFlags & SONG_ITPROJECT) != 0);
		}
	}

	switch(nNewType)
	{
	case MOD_TYPE_MOD:
		OnFileNewMOD();
		break;
	case MOD_TYPE_S3M:
		OnFileNewS3M();
		break;
	case MOD_TYPE_XM:
		OnFileNewXM();
		break;
	case MOD_TYPE_IT:
		if(bIsProject)
			OnFileNewITProject();
		else
			OnFileNewIT();
		break;
	case MOD_TYPE_MPT:
	default:
		OnFileNewMPT();
		break;
	}
}


void CTrackApp::OnFileNewMOD()
//----------------------------
{
// -> CODE#0023
// -> DESC="IT project files (.itp)"
	SetAsProject(FALSE);
// -! NEW_FEATURE#0023

	SetDefaultDocType(MOD_TYPE_MOD);
	if (m_pModTemplate) m_pModTemplate->OpenDocumentFile(NULL);
}


void CTrackApp::OnFileNewS3M()
//----------------------------
{
// -> CODE#0023
// -> DESC="IT project files (.itp)"
	SetAsProject(FALSE);
// -! NEW_FEATURE#0023

	SetDefaultDocType(MOD_TYPE_S3M);
	if (m_pModTemplate) m_pModTemplate->OpenDocumentFile(NULL);
}


void CTrackApp::OnFileNewXM()
//---------------------------
{
// -> CODE#0023
// -> DESC="IT project files (.itp)"
	SetAsProject(FALSE);
// -! NEW_FEATURE#0023

	SetDefaultDocType(MOD_TYPE_XM);
	if (m_pModTemplate) m_pModTemplate->OpenDocumentFile(NULL);
}


void CTrackApp::OnFileNewIT()
//---------------------------
{
// -> CODE#0023
// -> DESC="IT project files (.itp)"
	SetAsProject(FALSE);
// -! NEW_FEATURE#0023

	SetDefaultDocType(MOD_TYPE_IT);
	if (m_pModTemplate) m_pModTemplate->OpenDocumentFile(NULL);
}

void CTrackApp::OnFileNewMPT()
//---------------------------
{
	SetAsProject(FALSE);
	SetDefaultDocType(MOD_TYPE_MPT);
	if (m_pModTemplate) m_pModTemplate->OpenDocumentFile(NULL);
}



// -> CODE#0023
// -> DESC="IT project files (.itp)"
void CTrackApp::OnFileNewITProject()
//----------------------------------
{
	SetAsProject(TRUE);
	SetDefaultDocType(MOD_TYPE_IT);
	if (m_pModTemplate) m_pModTemplate->OpenDocumentFile(NULL);
}
// -! NEW_FEATURE#0023


void CTrackApp::OnFileOpen()
//--------------------------
{
	static int nFilterIndex = 0;
	FileDlgResult files = ShowOpenSaveFileDialog(true, "", "",
		"All Modules|*.mod;*.nst;*.wow;*.s3m;*.stm;*.669;*.mtm;*.xm;*.it;*.itp;*.mptm;*.ult;*.mdz;*.s3z;*.xmz;*.itz;mod.*;*.far;*.mdl;*.okt;*.dmf;*.ptm;*.mdr;*.med;*.ams;*.dbm;*.dsm;*.mid;*.rmi;*.smf;*.umx;*.amf;*.psm;*.mt2;*.gdm;*.imf;*.j2b"
#ifndef NO_MO3_SUPPORT
		";*.mo3"
#endif
		"|"
		"Compressed Modules (*.mdz;*.s3z;*.xmz;*.itz"
#ifndef NO_MO3_SUPPORT
		";*.mo3"
#endif
		")|*.mdz;*.s3z;*.xmz;*.itz;*.mdr;*.zip;*.rar;*.lha;*.gz"
#ifndef NO_MO3_SUPPORT
		";*.mo3"
#endif
		"|"
		"ProTracker Modules (*.mod,*.nst)|*.mod;mod.*;*.mdz;*.nst;*.m15|"
		"ScreamTracker Modules (*.s3m,*.stm)|*.s3m;*.stm;*.s3z|"
		"FastTracker Modules (*.xm)|*.xm;*.xmz|"
		"Impulse Tracker Modules (*.it)|*.it;*.itz|"
		// -> CODE#0023
		// -> DESC="IT project files (.itp)"
		"Impulse Tracker Projects (*.itp)|*.itp;*.itpz|"
		// -! NEW_FEATURE#0023
		"OpenMPT Modules (*.mptm)|*.mptm;*.mptmz|"
		"Other Modules (mtm,okt,mdl,669,far,...)|*.mtm;*.669;*.ult;*.wow;*.far;*.mdl;*.okt;*.dmf;*.ptm;*.med;*.ams;*.dbm;*.dsm;*.umx;*.amf;*.psm;*.mt2;*.gdm;*.imf;*.j2b|"
		"Wave Files (*.wav)|*.wav|"
		"Midi Files (*.mid,*.rmi)|*.mid;*.rmi;*.smf|"
		"All Files (*.*)|*.*||",
		CMainFrame::GetSettings().GetWorkingDirectory(DIR_MODS),
		true,
		&nFilterIndex);
	if(files.abort) return;

	CMainFrame::GetSettings().SetWorkingDirectory(files.workingDirectory.c_str(), DIR_MODS, true);

	for(size_t counter = 0; counter < files.filenames.size(); counter++)
	{
		OpenDocumentFile(files.filenames[counter].c_str());
	}
}


void CTrackApp::RegisterExtensions()
//----------------------------------
{
	HKEY key;
	CHAR s[512] = "";
	CHAR exename[512] = "";

	GetModuleFileName(AfxGetInstanceHandle(), s, sizeof(s));
	GetShortPathName(s, exename, sizeof(exename));
	if (RegCreateKey(HKEY_CLASSES_ROOT,
					"OpenMPTFile\\shell\\Edit\\command",
					&key) == ERROR_SUCCESS)
	{
		strcpy(s, exename);
		strcat(s, " \"%1\"");
		RegSetValueEx(key, NULL, NULL, REG_SZ, (LPBYTE)s, strlen(s)+1);
		RegCloseKey(key);
	}
	if (RegCreateKey(HKEY_CLASSES_ROOT,
					"OpenMPTFile\\shell\\Edit\\ddeexec",
					&key) == ERROR_SUCCESS)
	{
		strcpy(s, "[Edit(\"%1\")]");
		RegSetValueEx(key, NULL, NULL, REG_SZ, (LPBYTE)s, strlen(s)+1);
		RegCloseKey(key);
	}
}


void CTrackApp::OnHelpSearch()
//----------------------------
{
	CHAR s[80] = "";
	WinHelp((DWORD)&s, HELP_KEY);
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

//===============================
class CPaletteBitmap: public CWnd
//===============================
{
protected:
	LPBITMAPINFO m_lpBmp, m_lpCopy;
	HPALETTE m_hPal;
	LPBYTE m_lpRotoZoom;
	DWORD m_dwStartTime, m_dwFrameTime;
	UINT m_nRotoWidth, m_nRotoHeight;
	BOOL m_bFirst;

public:
	CPaletteBitmap() { m_hPal = NULL; m_lpBmp = NULL; m_lpRotoZoom = NULL; m_lpCopy = NULL; m_bFirst = TRUE; }
	~CPaletteBitmap();
	void LoadBitmap(LPCSTR lpszResource);
	BOOL Animate();
	UINT GetWidth() const { return (m_lpBmp) ? m_lpBmp->bmiHeader.biWidth : 0; }
	UINT GetHeight() const { return (m_lpBmp) ? m_lpBmp->bmiHeader.biHeight : 0; }

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *) { return TRUE; }
	afx_msg void OnLButtonDblClk(UINT, CPoint) { m_dwStartTime = timeGetTime(); } 
	DECLARE_MESSAGE_MAP()
};

static CPaletteBitmap *gpRotoZoom = NULL;

BEGIN_MESSAGE_MAP(CPaletteBitmap, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


void CPaletteBitmap::LoadBitmap(LPCSTR lpszResource)
//--------------------------------------------------
{
	m_hPal = NULL;
	m_lpBmp = NULL;
	// True Color Mode ?
	HDC hdc = ::GetDC(AfxGetApp()->m_pMainWnd->m_hWnd);
	int nbits = GetDeviceCaps(hdc, BITSPIXEL);
	::ReleaseDC(AfxGetApp()->m_pMainWnd->m_hWnd, hdc);
	// Creating palette
	HINSTANCE hInstance = AfxGetInstanceHandle();
	HRSRC hrsrc = FindResource(hInstance, lpszResource, RT_BITMAP);
	HGLOBAL hglb = LoadResource(hInstance, hrsrc);
	LPBITMAPINFO p = (LPBITMAPINFO)LockResource(hglb);
	m_lpBmp = p;
	m_nRotoWidth = m_nRotoHeight = 0;
	if (p)
	{
		char pal_buf[sizeof(LOGPALETTE) + 512*sizeof(PALETTEENTRY)];
		LPLOGPALETTE pal = (LPLOGPALETTE)&pal_buf[0];
		for (int i=0; i<256; i++)
		{
			pal->palPalEntry[i].peRed = p->bmiColors[i].rgbRed;
			pal->palPalEntry[i].peGreen = p->bmiColors[i].rgbGreen;
			pal->palPalEntry[i].peBlue = p->bmiColors[i].rgbBlue;
			pal->palPalEntry[i].peFlags = 0;
		}
		pal->palVersion = 0x300;
		pal->palNumEntries = 256;
		if (nbits <= 8) m_hPal = CreatePalette(pal);
		if ((p->bmiHeader.biWidth == 256) && (p->bmiHeader.biHeight == 128))
		{
			UINT n;
			CRect rect;
			GetClientRect(&rect);
			m_nRotoWidth = (rect.Width() + 3) & ~3;
			m_nRotoHeight = rect.Height();
			if ((n = m_nRotoWidth - rect.Width()) > 0)
			{
				GetWindowRect(&rect);
				SetWindowPos(NULL, 0,0, rect.Width()+n, rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
			}
			m_lpRotoZoom = new BYTE[m_nRotoWidth*m_nRotoHeight];
			if (m_lpRotoZoom)
			{
				memset(m_lpRotoZoom, 0, m_nRotoWidth*m_nRotoHeight);
				m_lpCopy = (LPBITMAPINFO)(new char [sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD)]);
				if (m_lpCopy)
				{
					memcpy(m_lpCopy, p, sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
					m_lpCopy->bmiHeader.biWidth = m_nRotoWidth;
					m_lpCopy->bmiHeader.biHeight = m_nRotoHeight;
					m_lpCopy->bmiHeader.biSizeImage = m_nRotoWidth * m_nRotoHeight;
				}
				gpRotoZoom = this;
			}

		}
	}
	m_dwStartTime = timeGetTime();
	m_dwFrameTime = 0;
}


CPaletteBitmap::~CPaletteBitmap()
//-------------------------------
{
	if (gpRotoZoom == this) gpRotoZoom = NULL;
	if (m_hPal)
	{
		DeleteObject(m_hPal);
		m_hPal = NULL;
	}
	if (m_lpRotoZoom)
	{
		delete[] m_lpRotoZoom;
		m_lpRotoZoom = NULL;
	}
	if (m_lpCopy)
	{
		delete[] m_lpCopy;
		m_lpCopy = NULL;
	}
}


void CPaletteBitmap::OnPaint()
//----------------------------
{
	CPaintDC dc(this);
	HDC hdc = dc.m_hDC;
	HPALETTE oldpal = NULL;
	LPBITMAPINFO lpdib;
	if (m_hPal)
	{
		oldpal = SelectPalette(hdc, m_hPal, FALSE);
		RealizePalette(hdc);
	}
	if ((lpdib = m_lpBmp) != NULL)
	{
		if ((m_lpRotoZoom) && (m_lpCopy))
		{
			lpdib = m_lpCopy;
			SetDIBitsToDevice(hdc,
						0,
						0,
						m_nRotoWidth,
						m_nRotoHeight,
						0,
						0,
						0,
						lpdib->bmiHeader.biHeight,
						m_lpRotoZoom,
						lpdib,
						DIB_RGB_COLORS);
		} else
		{
			CRect rect;
			GetClientRect(&rect);
			StretchDIBits(hdc,
						0,
						0,
						rect.right,
						rect.bottom,
						0,
						0,
						lpdib->bmiHeader.biWidth,
						lpdib->bmiHeader.biHeight,
						&lpdib->bmiColors[256],
						lpdib,
						DIB_RGB_COLORS,
						SRCCOPY);
		}
	}
	if (oldpal) SelectPalette(hdc, oldpal, FALSE);
}

////////////////////////////////////////////////////////////////////
// RotoZoomer

const int __SinusTable[256] =
{
	   0,   6,  12,  18,  25,  31,  37,  43,  49,  56,  62,  68,  74,  80,  86,  92,
	  97, 103, 109, 115, 120, 126, 131, 136, 142, 147, 152, 157, 162, 167, 171, 176,
	 181, 185, 189, 193, 197, 201, 205, 209, 212, 216, 219, 222, 225, 228, 231, 234,
	 236, 238, 241, 243, 244, 246, 248, 249, 251, 252, 253, 254, 254, 255, 255, 255,
	 256, 255, 255, 255, 254, 254, 253, 252, 251, 249, 248, 246, 244, 243, 241, 238,
	 236, 234, 231, 228, 225, 222, 219, 216, 212, 209, 205, 201, 197, 193, 189, 185,
	 181, 176, 171, 167, 162, 157, 152, 147, 142, 136, 131, 126, 120, 115, 109, 103,
	  97,  92,  86,  80,  74,  68,  62,  56,  49,  43,  37,  31,  25,  18,  12,   6,
	   0,  -6, -12, -18, -25, -31, -37, -43, -49, -56, -62, -68, -74, -80, -86, -92,
	 -97,-103,-109,-115,-120,-126,-131,-136,-142,-147,-152,-157,-162,-167,-171,-176,
	-181,-185,-189,-193,-197,-201,-205,-209,-212,-216,-219,-222,-225,-228,-231,-234,
	-236,-238,-241,-243,-244,-246,-248,-249,-251,-252,-253,-254,-254,-255,-255,-255,
	-256,-255,-255,-255,-254,-254,-253,-252,-251,-249,-248,-246,-244,-243,-241,-238,
	-236,-234,-231,-228,-225,-222,-219,-216,-212,-209,-205,-201,-197,-193,-189,-185,
	-181,-176,-171,-167,-162,-157,-152,-147,-142,-136,-131,-126,-120,-115,-109,-103,
	 -97, -92, -86, -80, -74, -68, -62, -56, -49, -43, -37, -31, -25, -18, -12,  -6
};

#define Sinus(x)	__SinusTable[(x)&0xFF]
#define Cosinus(x)	__SinusTable[((x)+0x40)&0xFF]

#define PI	3.14159265358979323f
BOOL CPaletteBitmap::Animate()
//----------------------------
{	
	//included random hacking by rewbs to get funny animation.
	LPBYTE dest, src;
	DWORD t = (timeGetTime() - m_dwStartTime) / 10;
	LONG Dist, Phi, srcx, srcy, spdx, spdy, sizex, sizey;
	bool dir;

	if ((!m_lpRotoZoom) || (!m_lpBmp) || (!m_nRotoWidth) || (!m_nRotoHeight)) return FALSE;
	Sleep(2); 	//give away some CPU

	if (t > 256)
		m_bFirst = FALSE;

	dir = ((t/256) % 2 != 0); //change dir every 256 t
	t = t%256;
	if (!dir) t = (256-t);
	
	sizex = m_nRotoWidth;
	sizey = m_nRotoHeight;
	m_dwFrameTime = t;
	src = (LPBYTE)&m_lpBmp->bmiColors[256];
	dest = m_lpRotoZoom;
	Dist = t;
	Phi = t;
	spdx = 70000 + Sinus(Phi) * 10000 / 256;
	spdy = 0;
	spdx =(Cosinus(Phi)+Sinus(Phi<<2))*(Dist<<9)/sizex;
	spdy =(Sinus(Phi)+Cosinus(Phi>>2))*(Dist<<9)/sizey;
	srcx = 0x800000 - ((spdx * sizex) >> 1) + (spdy * sizey);	
	srcy = 0x800000 - ((spdy * sizex) >> 1) + (spdx * sizey);	
	for (UINT y=sizey; y; y--)
	{
		UINT oldx = srcx, oldy = srcy;
		for (UINT x=sizex; x; x--)
		{
			srcx += spdx;
			srcy += spdy;
			*dest++ = src[((srcy & 0x7F0000) >> 8) | ((srcx & 0xFF0000) >> 16)];
		}
		srcx=oldx-spdy;
		srcy=oldy+spdx;
	}
	InvalidateRect(NULL, FALSE);
	UpdateWindow();
	return TRUE;
}


//=============================
class CAboutDlg: public CDialog
//=============================
{
protected:
	CPaletteBitmap m_bmp;
	CCreditStatic m_static;
	CHyperEdit m_heContact;

public:
	CAboutDlg() {}
	~CAboutDlg();

// Implementation
protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);
};

static CAboutDlg *gpAboutDlg = NULL;


CAboutDlg::~CAboutDlg()
//---------------------
{
	gpAboutDlg = NULL;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
//------------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModTypeDlg)
	DDX_Control(pDX, IDC_EDIT1,			m_heContact);
	//}}AFX_DATA_MAP
}

void CAboutDlg::OnOK()
//--------------------
{
	gpRotoZoom = NULL;
	gpAboutDlg = NULL;
	DestroyWindow();
	delete this;
}


void CAboutDlg::OnCancel()
//------------------------
{
	OnOK();
}


BOOL CAboutDlg::OnInitDialog()
//----------------------------
{
	CHAR s[256];
	CDialog::OnInitDialog();
	m_bmp.SubclassDlgItem(IDC_BITMAP1, this);
	m_bmp.LoadBitmap(MAKEINTRESOURCE(IDB_MPTRACK));
	wsprintf(s, "Build Date: %s", gszBuildDate);
	SetDlgItemText(IDC_EDIT2, s);
	SetDlgItemText(IDC_EDIT3, CString("OpenMPT ") + MptVersion::str + " (development build)");

	m_heContact.SetWindowText(
		"Contact / Discussion:\r\n"
		"http://forum.openmpt.org/\r\n"
		"\r\nUpdates:\r\n"
		"http://openmpt.org/download");

	const char* const pArrCredit = { 
		"OpenMPT / ModPlug Tracker|"
		"Copyright � 2004-2011 Contributors|"
		"Copyright � 1997-2003 Olivier Lapicque (olivier@modplug.com)|"
		"|"
		"Contributors:|"
		"Ahti Lepp�nen (2005-2011)|"
		"Johannes Schultz (2008-2011)|"
		"Robin Fernandes (2004-2007)|"
		"Sergiy Pylypenko (2007)|"
		"Eric Chavanon (2004-2005)|"
		"Trevor Nunes (2004)|"
		"Olivier Lapicque (1997-2003)|"
		"|"
		"Thanks to:||"
		"Konstanty for the XMMS-ModPlug resampling implementation |"
		"http://modplug-xmms.sourceforge.net/|"
		"Stephan M. Bernsee for pitch shifting source code|"
		"http://www.dspdimension.com|"
		"Olli Parviainen for SoundTouch Library (time stretching)|"
		"http://www.surina.net/soundtouch/|"
		"Hermann Seib for his example VST Host implementation|"
		"http://www.hermannseib.com/english/vsthost.htm|"
		"Ian Luck for UNMO3|"
		"http://www.un4seen.com/mo3.html|"
		"Jean-loup Gailly and Mark Adler for zlib|"
		"http://zlib.net/|"
		"coda for sample drawing code|"
		"http://coda.s3m.us/|"
		"Storlek for all the IT compatibility hints and testcases|"
		"as well as the IMF, OKT and ULT loaders|"
		"http://schismtracker.org/|"
		"kode54 for the PSM and J2B loaders|"
		"http://kode54.foobar2000.org/|"
		"Pel K. Txnder for the scrolling credits control :)|"
		"http://tinyurl.com/4yze8|"
		"|The people at ModPlug forums for crucial contribution|"
		"in the form of ideas, testing and support; thanks|"
		"particularly to:|"
		"33, Anboi, BooT-SectoR-ViruZ, Bvanoudtshoorn|"
		"christofori, Diamond, Ganja, Georg, Goor00, jmkz,|"
		"KrazyKatz, LPChip, Nofold, Rakib, Sam Zen|"
		"Skaven, Skilletaudio, Snu, Squirrel Havoc, Waxhead|"
		"|||||||"
		"VST PlugIn Technology by Steinberg Media Technologies GmbH|"
		"ASIO Technology by Steinberg Media Technologies GmbH|"
		"||||||" 
	};

    m_static.SubclassDlgItem(IDC_CREDITS,this);
    m_static.SetCredits(pArrCredit);
    m_static.SetSpeed(DISPLAY_SLOW);
    m_static.SetColor(BACKGROUND_COLOR, RGB(138, 165, 219)); // Background Colour
    m_static.SetTransparent(); // Set parts of bitmaps with RGB(192,192,192) transparent
    m_static.SetGradient(GRADIENT_LEFT_DARK);  // Background goes from blue to black from left to right
    // m_static.SetBkImage(IDB_BITMAP1); // Background image
    m_static.StartScrolling();
    return TRUE;  // return TRUE unless you set the focus to a control
                    // EXCEPTION: OCX Property Pages should return FALSE
}


// App command to run the dialog
void CTrackApp::OnAppAbout()
//--------------------------
{
	if (gpAboutDlg) return;
	gpAboutDlg = new CAboutDlg();
	gpAboutDlg->Create(IDD_ABOUTBOX, m_pMainWnd);
}


/////////////////////////////////////////////////////////////////////////////
// Splash Screen

//=================================
class CSplashScreen: public CDialog
//=================================
{
protected:
	CPaletteBitmap m_Bmp;

public:
	CSplashScreen() {}
	~CSplashScreen();
	BOOL Initialize(CWnd *);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel() { OnOK(); }
};

static CSplashScreen *gpSplashScreen = NULL;

CSplashScreen::~CSplashScreen()
//-----------------------------
{
	gpSplashScreen = NULL;
}


BOOL CSplashScreen::Initialize(CWnd *parent)
//------------------------------------------
{
	Create(IDD_SPLASHSCREEN, parent);
	return TRUE;
}


BOOL CSplashScreen::OnInitDialog()
//--------------------------------
{
	CRect rect;
	int cx, cy, newcx, newcy;
	
	CDialog::OnInitDialog();
	m_Bmp.SubclassDlgItem(IDC_SPLASH, this);
	m_Bmp.LoadBitmap(MAKEINTRESOURCE(IDB_SPLASHNOFOLDFIN));
	GetWindowRect(&rect);
	cx = rect.Width();
	cy = rect.Height();
	newcx = m_Bmp.GetWidth();
	newcy = m_Bmp.GetHeight();
	if ((newcx) && (newcy))
	{
		LONG ExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		ExStyle |= WS_EX_TOPMOST;
		SetWindowLong(m_hWnd, GWL_EXSTYLE, ExStyle);
		rect.left -= (newcx - cx) / 2;
		rect.top -= (newcy - cy) / 2;
		SetWindowPos(&wndTop, rect.left, rect.top, newcx, newcy, 0);
		m_Bmp.SetWindowPos(NULL, 0,0, newcx, newcy, SWP_NOZORDER);
	}
	return TRUE;
}


void CSplashScreen::OnOK()
//------------------------
{
	if (gpSplashScreen)
	{
		EndWaitCursor();
		gpSplashScreen = NULL;
	}
	DestroyWindow();
	delete this;
}


VOID CTrackApp::StartSplashScreen()
//---------------------------------
{
	if (!gpSplashScreen)
	{
		gpSplashScreen = new CSplashScreen();
		if (gpSplashScreen)
		{
			gpSplashScreen->Initialize(m_pMainWnd);
			gpSplashScreen->ShowWindow(SW_SHOW);
			gpSplashScreen->UpdateWindow();
			gpSplashScreen->BeginWaitCursor();
		}
	}
}


VOID CTrackApp::StopSplashScreen()
//--------------------------------
{
	if (gpSplashScreen)
	{
		gpSplashScreen->EndWaitCursor();
		gpSplashScreen->DestroyWindow();
		if (gpSplashScreen)
		{
			delete gpSplashScreen;
			gpSplashScreen = NULL;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Idle-time processing

BOOL CTrackApp::OnIdle(LONG lCount)
//---------------------------------
{
	BOOL b = CWinApp::OnIdle(lCount);
	if ((gpSplashScreen) && (m_bInitialized))
	{
		if (timeGetTime() - m_dwTimeStarted > 1000)		//Set splash screen duration here -rewbs
		{
			StopSplashScreen();
		}
	}
	if (gpRotoZoom)
	{
		if (gpRotoZoom->Animate()) return TRUE;
	}

	// Call plugins idle routine for open editor
	DWORD curTime = timeGetTime();
	// TODO: is it worth the overhead of checking that 10ms have passed,
	//       or should we just do it on every idle message?
	if (m_pPluginManager)
	{
		//rewbs.vstCompliance: call @ 50Hz
		if (curTime - m_dwLastPluginIdleCall > 20) //20ms since last call?
		{
			m_pPluginManager->OnIdle();
			m_dwLastPluginIdleCall = curTime;
		}
	}

	return b;
}


/////////////////////////////////////////////////////////////////////////////
// DIB


RGBQUAD rgb2quad(COLORREF c)
//--------------------------
{
	RGBQUAD r;
	r.rgbBlue = GetBValue(c);
	r.rgbGreen = GetGValue(c);
	r.rgbRed = GetRValue(c);
	r.rgbReserved = 0;
	return r;
}


void DibBlt(HDC hdc, int x, int y, int sizex, int sizey, int srcx, int srcy, LPMODPLUGDIB lpdib)
//----------------------------------------------------------------------------------------------
{
	if (!lpdib) return;
	SetDIBitsToDevice(	hdc,
						x,
						y,
						sizex,
						sizey,
						srcx,
						lpdib->bmiHeader.biHeight - srcy - sizey,
						0,
						lpdib->bmiHeader.biHeight,
						lpdib->lpDibBits,
						(LPBITMAPINFO)lpdib,
						DIB_RGB_COLORS);
}


LPMODPLUGDIB LoadDib(LPCSTR lpszName)
//-----------------------------------
{
	HINSTANCE hInstance = AfxGetInstanceHandle();
	HRSRC hrsrc = FindResource(hInstance, lpszName, RT_BITMAP);
	HGLOBAL hglb = LoadResource(hInstance, hrsrc);
	LPBITMAPINFO p = (LPBITMAPINFO)LockResource(hglb);
	if (p)
	{
		LPMODPLUGDIB pmd = new MODPLUGDIB;
		pmd->bmiHeader = p->bmiHeader;
		for (int i=0; i<16; i++) pmd->bmiColors[i] = p->bmiColors[i];
		LPBYTE lpDibBits = (LPBYTE)p;
		lpDibBits += p->bmiHeader.biSize + 16 * sizeof(RGBQUAD);
		pmd->lpDibBits = lpDibBits;
		return pmd;
	} else return NULL;
}


void DrawButtonRect(HDC hdc, LPRECT lpRect, LPCSTR lpszText, BOOL bDisabled, BOOL bPushed, DWORD dwFlags)
//-------------------------------------------------------------------------------------------------------
{
	RECT rect;
	HGDIOBJ oldpen = ::SelectObject(hdc, (bPushed) ? CMainFrame::penDarkGray : CMainFrame::penLightGray);
	::MoveToEx(hdc, lpRect->left, lpRect->bottom-1, NULL);
	::LineTo(hdc, lpRect->left, lpRect->top);
	::LineTo(hdc, lpRect->right-1, lpRect->top);
	::SelectObject(hdc, (bPushed) ? CMainFrame::penLightGray : CMainFrame::penDarkGray);
	::LineTo(hdc, lpRect->right-1, lpRect->bottom-1);
	::LineTo(hdc, lpRect->left, lpRect->bottom-1);
	rect.left = lpRect->left + 1;
	rect.top = lpRect->top + 1;
	rect.right = lpRect->right - 1;
	rect.bottom = lpRect->bottom - 1;
	::FillRect(hdc, &rect, CMainFrame::brushGray);
	::SelectObject(hdc, oldpen);
	if ((lpszText) && (lpszText[0]))
	{
		if (bPushed)
		{
			rect.top++;
			rect.left++;
		}
		::SetTextColor(hdc, GetSysColor((bDisabled) ? COLOR_GRAYTEXT : COLOR_BTNTEXT));
		::SetBkMode(hdc, TRANSPARENT);
		HGDIOBJ oldfont = ::SelectObject(hdc, CMainFrame::GetGUIFont());
		::DrawText(hdc, lpszText, -1, &rect, dwFlags | DT_SINGLELINE | DT_NOPREFIX);
		::SelectObject(hdc, oldfont);
	}
}


//////////////////////////////////////////////////////////////////////////////////
// Misc functions


UINT MsgBox(UINT nStringID, CWnd *parent, LPCSTR lpszTitle, UINT n)
//-----------------------------------------------------------------
{
	CString str;
	str.LoadString(nStringID);
	return Reporting::CustomNotification(str, CString(lpszTitle), n, parent);
}


void ErrorBox(UINT nStringID, CWnd *parent)
//-----------------------------------------
{
	MsgBox(nStringID, parent, "Error!", MB_OK | MB_ICONERROR);
}


////////////////////////////////////////////////////////////////////////////////
// CFastBitmap 8-bit output / 4-bit input
// useful for lots of small blits with color mapping 
// combined in one big blit

void CFastBitmap::Init(LPMODPLUGDIB lpTextDib)
//--------------------------------------------
{
	m_nBlendOffset = 0;
	m_pTextDib = lpTextDib;
	MemsetZero(m_Dib);
	m_nTextColor = 0;
	m_nBkColor = 1;
	m_Dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Dib.bmiHeader.biWidth = FASTBMP_MAXWIDTH;
	m_Dib.bmiHeader.biHeight = FASTBMP_MAXHEIGHT;
	m_Dib.bmiHeader.biPlanes = 1;
	m_Dib.bmiHeader.biBitCount = 8;
	m_Dib.bmiHeader.biCompression = BI_RGB;
	m_Dib.bmiHeader.biSizeImage = 0;
	m_Dib.bmiHeader.biXPelsPerMeter = 96;
	m_Dib.bmiHeader.biYPelsPerMeter = 96;
	m_Dib.bmiHeader.biClrUsed = 0;
	m_Dib.bmiHeader.biClrImportant = 256; // MAX_MODPALETTECOLORS;
	m_n4BitPalette[0] = (BYTE)m_nTextColor;
	m_n4BitPalette[4] = MODCOLOR_SEPSHADOW;
	m_n4BitPalette[12] = MODCOLOR_SEPFACE;
	m_n4BitPalette[14] = MODCOLOR_SEPHILITE;
	m_n4BitPalette[15] = (BYTE)m_nBkColor;
}


void CFastBitmap::Blit(HDC hdc, int x, int y, int cx, int cy)
//-----------------------------------------------------------
{
	SetDIBitsToDevice(	hdc,
						x,
						y,
						cx,
						cy,
						0,
						FASTBMP_MAXHEIGHT - cy,
						0,
						FASTBMP_MAXHEIGHT,
						m_Dib.DibBits,
						(LPBITMAPINFO)&m_Dib,
						DIB_RGB_COLORS);
}


void CFastBitmap::SetColor(UINT nIndex, COLORREF cr)
//--------------------------------------------------
{
	if (nIndex < 256)
	{
		m_Dib.bmiColors[nIndex].rgbRed = GetRValue(cr);
		m_Dib.bmiColors[nIndex].rgbGreen = GetGValue(cr);
		m_Dib.bmiColors[nIndex].rgbBlue = GetBValue(cr);
	}
}


void CFastBitmap::SetAllColors(UINT nBaseIndex, UINT nColors, COLORREF *pcr)
//--------------------------------------------------------------------------
{
	for (UINT i=0; i<nColors; i++)
	{
		SetColor(nBaseIndex+i, pcr[i]);
	}
}


void CFastBitmap::SetBlendColor(COLORREF cr)
//------------------------------------------
{
	UINT r = GetRValue(cr);
	UINT g = GetGValue(cr);
	UINT b = GetBValue(cr);
	for (UINT i=0; i<0x80; i++)
	{
		UINT m = (m_Dib.bmiColors[i].rgbRed >> 2)
				+ (m_Dib.bmiColors[i].rgbGreen >> 1)
				+ (m_Dib.bmiColors[i].rgbBlue >> 2);
		m_Dib.bmiColors[i|0x80].rgbRed = static_cast<BYTE>((m + r)>>1);
		m_Dib.bmiColors[i|0x80].rgbGreen = static_cast<BYTE>((m + g)>>1);
		m_Dib.bmiColors[i|0x80].rgbBlue = static_cast<BYTE>((m + b)>>1);
	}
}


// Monochrome 4-bit bitmap (0=text, !0 = back)
void CFastBitmap::TextBlt(int x, int y, int cx, int cy, int srcx, int srcy, LPMODPLUGDIB lpdib)
//---------------------------------------------------------------------------------------------
{
	const BYTE *psrc;
	BYTE *pdest;
	UINT x1, x2;
	int srcwidth, srcinc;
	
	m_n4BitPalette[0] = (BYTE)m_nTextColor;
	m_n4BitPalette[15] = (BYTE)m_nBkColor;
	if (x < 0)
	{
		cx += x;
		x = 0;
	}
	if (y < 0)
	{
		cy += y;
		y = 0;
	}
	if ((x >= FASTBMP_MAXWIDTH) || (y >= FASTBMP_MAXHEIGHT)) return;
	if (x+cx >= FASTBMP_MAXWIDTH) cx = FASTBMP_MAXWIDTH - x;
	if (y+cy >= FASTBMP_MAXHEIGHT) cy = FASTBMP_MAXHEIGHT - y;
	if (!lpdib) lpdib = m_pTextDib;
	if ((cx <= 0) || (cy <= 0) || (!lpdib)) return;
	srcwidth = (lpdib->bmiHeader.biWidth+1) >> 1;
	srcinc = srcwidth;
	if (((int)lpdib->bmiHeader.biHeight) > 0)
	{
		srcy = lpdib->bmiHeader.biHeight - 1 - srcy;
		srcinc = -srcinc;
	}
	x1 = srcx & 1;
	x2 = x1 + cx;
	pdest = m_Dib.DibBits + ((FASTBMP_MAXHEIGHT - 1 - y) << FASTBMP_XSHIFT) + x;
	psrc = lpdib->lpDibBits + (srcx >> 1) + (srcy * srcwidth);
	for (int iy=0; iy<cy; iy++)
	{
		LPBYTE p = pdest;
		UINT ix = x1;
		if (ix&1)
		{
			UINT b = psrc[ix >> 1];
			*p++ = m_n4BitPalette[b & 0x0F]+m_nBlendOffset;
			ix++;
		}
		while (ix+1 < x2)
		{
			UINT b = psrc[ix >> 1];
			p[0] = m_n4BitPalette[b >> 4]+m_nBlendOffset;
			p[1] = m_n4BitPalette[b & 0x0F]+m_nBlendOffset;
			ix+=2;
			p+=2;
		}
		if (x2&1)
		{
			UINT b = psrc[ix >> 1];
			*p++ = m_n4BitPalette[b >> 4]+m_nBlendOffset;
		}
		pdest -= FASTBMP_MAXWIDTH;
		psrc += srcinc;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// CMappedFile

CMappedFile::CMappedFile()
//------------------------
{
	m_hFMap = NULL;
	m_lpData = NULL;
}


CMappedFile::~CMappedFile()
//-------------------------
{
}


BOOL CMappedFile::Open(LPCSTR lpszFileName)
//-----------------------------------------
{
	return m_File.Open(lpszFileName, CFile::modeRead|CFile::typeBinary);
}


void CMappedFile::Close()
//-----------------------
{
	if (m_lpData) Unlock();
	m_File.Close();
}


DWORD CMappedFile::GetLength()
//----------------------------
{
	return static_cast<DWORD>(m_File.GetLength());
}


LPBYTE CMappedFile::Lock(DWORD dwMaxLen)
//--------------------------------------
{
	DWORD dwLen = GetLength();
	LPBYTE lpStream;

	if (!dwLen) return NULL;
	if ((dwMaxLen) && (dwLen > dwMaxLen)) dwLen = dwMaxLen;
	HANDLE hmf = CreateFileMapping(
							(HANDLE)m_File.m_hFile,
							NULL,
							PAGE_READONLY,
							0, 0,
							NULL
							);
	if (hmf)
	{
		lpStream = (LPBYTE)MapViewOfFile(
								hmf,
								FILE_MAP_READ,
								0, 0,
								0
							);
		if (lpStream)
		{
			m_hFMap = hmf;
			m_lpData = lpStream;
			return lpStream;
		}
		CloseHandle(hmf);
	}
	if (dwLen > CTrackApp::gMemStatus.dwTotalPhys) return NULL;
	if ((lpStream = (LPBYTE)GlobalAllocPtr(GHND, dwLen)) == NULL) return NULL;
	m_File.Read(lpStream, dwLen);
	m_lpData = lpStream;
	return lpStream;
}


BOOL CMappedFile::Unlock()
//------------------------
{
	if (m_hFMap)
	{
		if (m_lpData)
		{
			UnmapViewOfFile(m_lpData);
			m_lpData = NULL;
		}
		CloseHandle(m_hFMap);
		m_hFMap = NULL;
	}
	if (m_lpData)
	{
		GlobalFreePtr(m_lpData);
		m_lpData = NULL;
	}
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////////
//
// DirectX Plugins
//

BOOL CTrackApp::InitializeDXPlugins()
//-----------------------------------
{
	TCHAR s[_MAX_PATH], tmp[32];
	LONG nPlugins;

	m_pPluginManager = new CVstPluginManager;
	if (!m_pPluginManager) return FALSE;
	nPlugins = GetPrivateProfileInt("VST Plugins", "NumPlugins", 0, m_szConfigFileName);

	#ifndef NO_VST
		char buffer[64];
		GetPrivateProfileString("VST Plugins", "HostProductString", CVstPluginManager::s_szHostProductString, buffer, CountOf(buffer), m_szConfigFileName);

		// Version <= 1.19.03.00 had buggy handling of custom host information. If last open was from
		// such OpenMPT version, clear the related settings to get a clean start.
		const CString sPreviousVer = CMainFrame::GetSettings().gcsPreviousVersion;
		if (!sPreviousVer.IsEmpty() && 
			MptVersion::ToNum(sPreviousVer) < MAKE_VERSION_NUMERIC(1, 19, 03, 01) &&
			strcmp(buffer, "OpenMPT") == 0)
		{
			// Remove keys by calling write with nullptr.
			WritePrivateProfileString(_T("VST Plugins"), _T("HostProductString"), nullptr, m_szConfigFileName);
			WritePrivateProfileString(_T("VST Plugins"), _T("HostVendorString"), nullptr, m_szConfigFileName);
			WritePrivateProfileString(_T("VST Plugins"), _T("HostVendorVersion"), nullptr, m_szConfigFileName);
		}

		GetPrivateProfileString("VST Plugins", "HostProductString", CVstPluginManager::s_szHostProductString, buffer, CountOf(buffer), m_szConfigFileName);
		strcpy(CVstPluginManager::s_szHostProductString, buffer);
		GetPrivateProfileString("VST Plugins", "HostVendorString", CVstPluginManager::s_szHostVendorString, buffer, CountOf(buffer), m_szConfigFileName);
		strcpy(CVstPluginManager::s_szHostVendorString, buffer);
		CVstPluginManager::s_nHostVendorVersion = GetPrivateProfileInt("VST Plugins", "HostVendorVersion", CVstPluginManager::s_nHostVendorVersion, m_szConfigFileName);
	#endif


	CString nonFoundPlugs;
	const CString failedPlugin = CMainFrame::GetPrivateProfileCString("VST Plugins", "FailedPlugin", "", m_szConfigFileName);

	for (LONG iPlug=0; iPlug<nPlugins; iPlug++)
	{
		s[0] = 0;
		wsprintf(tmp, "Plugin%d", iPlug);
		GetPrivateProfileString("VST Plugins", tmp, "", s, sizeof(s), m_szConfigFileName);
		if (s[0])
		{
			RelativePathToAbsolute(s);

			if(!failedPlugin.Compare(s))
			{
				const CString text = "The following plugin has previously crashed OpenMPT during initialisation:\n\n" + failedPlugin + "\n\nDo you still want to load it?";
				if(Reporting::Confirm(text, false, true) == cnfNo)
				{
					continue;
				}
			}
			m_pPluginManager->AddPlugin(s, TRUE, true, &nonFoundPlugs);
		}
	}
	if(nonFoundPlugs.GetLength() > 0)
	{
		nonFoundPlugs.Insert(0, "Problems were encountered with plugins:\n");
		Reporting::Notification(nonFoundPlugs);
	}
	return FALSE;
}


BOOL CTrackApp::UninitializeDXPlugins()
//-------------------------------------
{
	TCHAR s[_MAX_PATH], tmp[32];
	PVSTPLUGINLIB pPlug;
	UINT iPlug;

	if (!m_pPluginManager) return FALSE;

#ifndef NO_VST
	pPlug = m_pPluginManager->GetFirstPlugin();
	iPlug = 0;
	while (pPlug)
	{
		if (pPlug->dwPluginId1 != kDmoMagic)
		{
			s[0] = 0;
			wsprintf(tmp, "Plugin%d", iPlug);
			strcpy(s, pPlug->szDllPath);
			if(theApp.IsPortableMode())
			{
				AbsolutePathToRelative(s);
			}
			WritePrivateProfileString("VST Plugins", tmp, s, m_szConfigFileName);
			iPlug++;
		}
		pPlug = pPlug->pNext;
	}
	wsprintf(s, "%d", iPlug);
	WritePrivateProfileString("VST Plugins", "NumPlugins", s, m_szConfigFileName);
#endif // NO_VST

	delete m_pPluginManager;
	m_pPluginManager = nullptr;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////////
// Internet-related functions

BOOL CTrackApp::OpenURL(const LPCSTR lpszURL)
//-------------------------------------------
{
	if ((lpszURL) && (lpszURL[0]) && (theApp.m_pMainWnd))
	{
		if (((ULONG)ShellExecute(
					theApp.m_pMainWnd->m_hWnd,
					"open",
					lpszURL,
					NULL,
					NULL,
					SW_SHOW)) >= 32) return TRUE;
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////////
// Debug

void Log(LPCSTR format,...)
//-------------------------
{
	#ifdef _DEBUG
		CHAR cBuf[1024];
		va_list va;
		va_start(va, format);
		wvsprintf(cBuf, format, va);
		OutputDebugString(cBuf);
		#ifdef LOG_TO_FILE
			FILE *f = fopen("c:\\mptrack.log", "a");
			if (f)
			{
				fwrite(cBuf, 1, strlen(cBuf), f);
				fclose(f);
			}
		#endif //LOG_TO_FILE
		va_end(va);
	#endif //_DEBUG
}


//////////////////////////////////////////////////////////////////////////////////
// Localized strings

VOID CTrackApp::ImportLocalizedStrings()
//--------------------------------------
{
	//DWORD dwLangId = ((DWORD)GetUserDefaultLangID()) & 0xfff;
	// TODO: look up [Strings.lcid], [Strings.(lcid&0xff)] & [Strings] in mpt_intl.ini
}


BOOL CTrackApp::GetLocalizedString(LPCSTR pszName, LPSTR pszStr, UINT cbSize)
//---------------------------------------------------------------------------
{
	CHAR s[32];
	DWORD dwLangId = ((DWORD)GetUserDefaultLangID()) & 0xffff;

	pszStr[0] = 0;
	if (!m_szStringsFileName[0]) return FALSE;
	wsprintf(s, "Strings.%04X", dwLangId);
	GetPrivateProfileString(s, pszName, "", pszStr, cbSize, m_szStringsFileName);
	if (pszStr[0]) return TRUE;
	wsprintf(s, "Strings.%04X", dwLangId&0xff);
	GetPrivateProfileString(s, pszName, "", pszStr, cbSize, m_szStringsFileName);
	if (pszStr[0]) return TRUE;
	wsprintf(s, "Strings", dwLangId&0xff);
	GetPrivateProfileString(s, pszName, "", pszStr, cbSize, m_szStringsFileName);
	if (pszStr[0]) return TRUE;
	return FALSE;
}


/* Open or save one or multiple files using the system's file dialog
 * Parameter list:
 * - load: true: load dialog. false: save dialog.
 * - defaultExtension: dialog should use this as the default extension for the file(s)
 * - defaultFilename: dialog should use this as the default filename
 * - extFilter: list of possible extensions. format: "description|extensions|...|description|extensions||"
 * - workingDirectory: default directory of the dialog
 * - allowMultiSelect: allow the user to select multiple files? (will be ignored if load == false)
 * - filterIndex: pointer to a variable holding the index of the last extension filter used.
 */
FileDlgResult CTrackApp::ShowOpenSaveFileDialog(const bool load, const std::string defaultExtension, const std::string defaultFilename, const std::string extFilter, const std::string workingDirectory, const bool allowMultiSelect, int *filterIndex)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{
	FileDlgResult result;
	result.workingDirectory = workingDirectory;
	result.first_file = "";
	result.filenames.clear();
	result.extension = defaultExtension;
	result.abort = true;

	// we can't save multiple files.
	const bool multiSelect = allowMultiSelect && load;

	// First, set up the dialog...
	CFileDialog dlg(load ? TRUE : FALSE,
		defaultExtension.empty() ? NULL : defaultExtension.c_str(),
		defaultFilename.empty() ? NULL : defaultFilename.c_str(),
		load ? (OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | (multiSelect ? OFN_ALLOWMULTISELECT : 0))
		     : (OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN),
		extFilter.empty() ? NULL : extFilter.c_str(),
		theApp.m_pMainWnd);
	if(!workingDirectory.empty())
		dlg.m_ofn.lpstrInitialDir = workingDirectory.c_str();
	if(filterIndex != nullptr)
		dlg.m_ofn.nFilterIndex = (DWORD)(*filterIndex);

	vector<TCHAR> filenameBuffer;
	if(multiSelect)
	{
		const size_t bufferSize = 2048; // Note: This is possibly the maximum buffer size in MFC 7(this note was written November 2006).
		filenameBuffer.resize(bufferSize, 0);
		dlg.GetOFN().lpstrFile = &filenameBuffer[0];
		dlg.GetOFN().nMaxFile = bufferSize;
	}

	// Do it!
	CMainFrame::GetInputHandler()->Bypass(true);
	if(dlg.DoModal() != IDOK)
	{
		CMainFrame::GetInputHandler()->Bypass(false);
		return result;
	}
	CMainFrame::GetInputHandler()->Bypass(false);

	// Retrieve variables
	if(filterIndex != nullptr)
		*filterIndex = dlg.m_ofn.nFilterIndex;

	if(multiSelect)
	{
		// multiple files might have been selected
		POSITION pos = dlg.GetStartPosition();
		while(pos != NULL)
		{
			std::string filename = dlg.GetNextPathName(pos);
			result.filenames.push_back(filename);
		}

	} else
	{
		// only one file
		std::string filename = dlg.GetPathName();
		result.filenames.push_back(filename);
	}

	if(!result.filenames.empty())
	{
		// some file has been selected.
		result.workingDirectory = result.filenames.back();
		result.first_file = result.filenames.front();
		result.abort = false;
	}

	result.extension = dlg.GetFileExt();

	return result;
}


// Convert an absolute path to a path that's relative to OpenMPT's directory.
// Paths are relative to the executable path.
// nLength specifies the maximum number of character that can be written into szPath,
// including the trailing null char.
template <size_t nLength>
void CTrackApp::AbsolutePathToRelative(TCHAR (&szPath)[nLength])
//---------------------------------------------------------------
{
	STATIC_ASSERT(nLength >= 3);

	if(_tcslen(szPath) == 0)
		return;

	const size_t nStrLength = nLength - 1;	// "usable" length, i.e. not including the null char.
	TCHAR szExePath[nLength], szTempPath[nLength];
	_tcsncpy(szExePath, GetAppDirPath(), nStrLength);
	StringFixer::SetNullTerminator(szExePath);

	if(!_tcsncicmp(szExePath, szPath, _tcslen(szExePath)))
	{
		// Path is OpenMPT's directory or a sub directory ("C:\OpenMPT\Somepath" => ".\Somepath")
		_tcscpy(szTempPath, _T(".\\"));	// ".\"
		_tcsncat(szTempPath, &szPath[_tcslen(szExePath)], nStrLength - 2);	// "Somepath"
		_tcscpy(szPath, szTempPath);
	} else if(!_tcsncicmp(szExePath, szPath, 2))
	{
		// Path is on the same drive as OpenMPT ("C:\Somepath" => "\Somepath")
		_tcsncpy(szTempPath, &szPath[2], nStrLength);	// "\Somepath"
		_tcscpy(szPath, szTempPath);
	}
	StringFixer::SetNullTerminator(szPath);
}


// Convert a relative path to an absolute path.
// Paths are relative to the executable path.
// nLength specifies the maximum number of character that can be written into szPath,
// including the trailing null char.
template <size_t nLength>
void CTrackApp::RelativePathToAbsolute(TCHAR (&szPath)[nLength])
//---------------------------------------------------------------
{
	STATIC_ASSERT(nLength >= 3);

	if(_tcslen(szPath) == 0)
		return;

	const size_t nStrLength = nLength - 1;	// "usable" length, i.e. not including the null char.
	TCHAR szExePath[nLength], szTempPath[nLength] = _T("");
	_tcsncpy(szExePath, GetAppDirPath(), nStrLength);
	StringFixer::SetNullTerminator(szExePath);

	if(!_tcsncicmp(szPath, _T("\\"), 1) && _tcsncicmp(szPath, _T("\\\\"), 2))
	{
		// Path is on the same drive as OpenMPT ("\Somepath\" => "C:\Somepath\"), but ignore network paths starting with "\\"
		_tcsncat(szTempPath, szExePath, 2);	// "C:"
		_tcsncat(szTempPath, szPath, nStrLength - 2);	// "\Somepath\"
		_tcscpy(szPath, szTempPath);
	} else if(!_tcsncicmp(szPath, _T(".\\"), 2))
	{
		// Path is OpenMPT's directory or a sub directory (".\Somepath\" => "C:\OpenMPT\Somepath\")
		_tcsncpy(szTempPath, szExePath, nStrLength);	// "C:\OpenMPT\"
		if(_tcslen(szTempPath) < nStrLength)
		{
			_tcsncat(szTempPath, &szPath[2], nStrLength - _tcslen(szTempPath));	//	"Somepath"
		}
		_tcscpy(szPath, szTempPath);
	}
	StringFixer::SetNullTerminator(szPath);
}

void CTrackApp::RemoveMruItem(const int nItem)
//--------------------------------------------
{
	if (m_pRecentFileList && nItem >= 0 && nItem < m_pRecentFileList->GetSize())
		m_pRecentFileList->Remove(nItem);
}
