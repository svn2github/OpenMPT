// moddoc.cpp : implementation of the CModDoc class
//

#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "moddoc.h"
#include "childfrm.h"
#include "mpdlgs.h"
#include "dlg_misc.h"
#include "dlsbank.h"
#include "mod2wave.h"
#include "mod2midi.h"
#include "vstplug.h"
#include "version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR* const FileFilterMOD = TEXT("ProTracker Modules (*.mod)|*.mod||");
const TCHAR* const FileFilterXM = TEXT("FastTracker Modules (*.xm)|*.xm||");
const TCHAR* const FileFilterS3M = TEXT("ScreamTracker Modules (*.s3m)|*.s3m||");
const TCHAR* const FileFilterIT = TEXT("Impulse Tracker Modules (*.it)|*.it||");
const TCHAR* const FileFilterITP = TEXT("Impulse Tracker Projects (*.itp)|*.itp||");
const TCHAR* const FileFilterMPT = TEXT("OpenMPT Modules (*.mptm)|*.mptm||");

/////////////////////////////////////////////////////////////////////////////
// CModDoc

IMPLEMENT_SERIAL(CModDoc, CDocument, 0 /* schema number*/ )

BEGIN_MESSAGE_MAP(CModDoc, CDocument)
	//{{AFX_MSG_MAP(CModDoc)
	ON_COMMAND(ID_FILE_SAVEASWAVE,		OnFileWaveConvert)
	ON_COMMAND(ID_FILE_SAVEASMP3,		OnFileMP3Convert)
	ON_COMMAND(ID_FILE_SAVEMIDI,		OnFileMidiConvert)
	ON_COMMAND(ID_FILE_SAVECOMPAT,		OnFileCompatibilitySave)
	ON_COMMAND(ID_PLAYER_PLAY,			OnPlayerPlay)
	ON_COMMAND(ID_PLAYER_PAUSE,			OnPlayerPause)
	ON_COMMAND(ID_PLAYER_STOP,			OnPlayerStop)
	ON_COMMAND(ID_PLAYER_PLAYFROMSTART,	OnPlayerPlayFromStart)
	ON_COMMAND(ID_VIEW_GLOBALS,			OnEditGlobals)
	ON_COMMAND(ID_VIEW_PATTERNS,		OnEditPatterns)
	ON_COMMAND(ID_VIEW_SAMPLES,			OnEditSamples)
	ON_COMMAND(ID_VIEW_INSTRUMENTS,		OnEditInstruments)
	ON_COMMAND(ID_VIEW_COMMENTS,		OnEditComments)
	ON_COMMAND(ID_VIEW_GRAPH,			OnEditGraph) //rewbs.graph
	ON_COMMAND(ID_INSERT_PATTERN,		OnInsertPattern)
	ON_COMMAND(ID_INSERT_SAMPLE,		OnInsertSample)
	ON_COMMAND(ID_INSERT_INSTRUMENT,	OnInsertInstrument)
	ON_COMMAND(ID_CLEANUP_SAMPLES,		OnCleanupSamples)
	ON_COMMAND(ID_CLEANUP_INSTRUMENTS,	OnCleanupInstruments)
	ON_COMMAND(ID_CLEANUP_PLUGS,		OnCleanupPlugs)
	ON_COMMAND(ID_CLEANUP_PATTERNS,		OnCleanupPatterns)
	ON_COMMAND(ID_CLEANUP_SONG,			OnCleanupSong)
	ON_COMMAND(ID_CLEANUP_REARRANGE,	OnRearrangePatterns)
	ON_COMMAND(ID_INSTRUMENTS_REMOVEALL,OnRemoveAllInstruments)
// -> CODE#0020
// -> DESC="rearrange sample list"
	ON_COMMAND(ID_REARRANGE_SAMPLES,	RearrangeSampleList)
// -! NEW_FEATURE#0020
	ON_COMMAND(ID_ESTIMATESONGLENGTH,	OnEstimateSongLength)
	ON_COMMAND(ID_APPROX_BPM,			OnApproximateBPM)
	ON_COMMAND(ID_PATTERN_PLAY,			OnPatternPlay)				//rewbs.patPlayAllViews
	ON_COMMAND(ID_PATTERN_PLAYNOLOOP,	OnPatternPlayNoLoop)		//rewbs.patPlayAllViews
	ON_COMMAND(ID_PATTERN_RESTART,		OnPatternRestart)			//rewbs.patPlayAllViews
	ON_UPDATE_COMMAND_UI(ID_INSERT_INSTRUMENT,		OnUpdateXMITMPTOnly)
	ON_UPDATE_COMMAND_UI(ID_INSTRUMENTS_REMOVEALL,	OnUpdateInstrumentOnly)
	ON_UPDATE_COMMAND_UI(ID_CLEANUP_INSTRUMENTS,	OnUpdateInstrumentOnly)
	ON_UPDATE_COMMAND_UI(ID_VIEW_INSTRUMENTS,		OnUpdateXMITMPTOnly)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COMMENTS,			OnUpdateXMITMPTOnly)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIDIMAPPING,		OnUpdateHasMIDIMappings)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEASMP3,			OnUpdateMP3Encode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////
// CModDoc diagnostics

#ifdef _DEBUG
void CModDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CModDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CModDoc construction/destruction

CModDoc::CModDoc()
//----------------
{
	m_bHasValidPath=false;
	m_bPaused = TRUE;
	m_lpszLog = NULL;
	m_hWndFollow = NULL;
	memset(PatternUndo, 0, sizeof(PatternUndo));
#ifdef _DEBUG
	MODCHANNEL *p = m_SndFile.Chn;
	if (((DWORD)p) & 7) Log("MODCHANNEL is not aligned (0x%08X)\n", p);
#endif
// Fix: save pattern scrollbar position when switching to other tab
	m_szOldPatternScrollbarsPos = CSize(-10,-10);
// -> CODE#0015
// -> DESC="channels management dlg"
	ReinitRecordState();
// -! NEW_FEATURE#0015
	m_ShowSavedialog = false;
}


CModDoc::~CModDoc()
//-----------------
{
	ClearUndo();
	ClearLog();
}


void CModDoc::SetModifiedFlag(BOOL bModified)
//-------------------------------------------
{
	BOOL bChanged = (bModified != IsModified());
	CDocument::SetModifiedFlag(bModified);
	if (bChanged) UpdateFrameCounts();
}


BOOL CModDoc::OnNewDocument()
//---------------------------
{
	if (!CDocument::OnNewDocument()) return FALSE;

	m_SndFile.Create(NULL, this, 0);
	m_SndFile.ChangeModTypeTo(CTrackApp::GetDefaultDocType());

	if(CTrackApp::IsProject()) {
		m_SndFile.m_dwSongFlags |= SONG_ITPROJECT;
	}

	if (m_SndFile.m_nType & (MOD_TYPE_XM | MOD_TYPE_IT | MOD_TYPE_MPT)) {
		m_SndFile.m_dwSongFlags |= SONG_LINEARSLIDES;
		m_SndFile.m_dwSongFlags |= SONG_EMBEDMIDICFG;
	}


	// Refresh mix levels now that the correct mod type has been set
	m_SndFile.m_nMixLevels = m_SndFile.GetModSpecifications().defaultMixLevels;
	m_SndFile.m_pConfig->SetMixLevels(m_SndFile.m_nMixLevels);

	theApp.GetDefaultMidiMacro(&m_SndFile.m_MidiCfg);
	ReinitRecordState();
	InitializeMod();
	SetModifiedFlag(FALSE);
	return TRUE;
}


BOOL CModDoc::OnOpenDocument(LPCTSTR lpszPathName)
//------------------------------------------------
{
	BOOL bModified = TRUE;
	CMappedFile f;

	if (!lpszPathName) return OnNewDocument();
	BeginWaitCursor();
	if (f.Open(lpszPathName))
	{
		DWORD dwLen = f.GetLength();
		if (dwLen)
		{
			LPBYTE lpStream = f.Lock();
			if (lpStream)
			{
				m_SndFile.Create(lpStream, this, dwLen);
				f.Unlock();
			}
		}
		f.Close();
	}

	EndWaitCursor();
	if ((m_SndFile.m_nType == MOD_TYPE_NONE) || (!m_SndFile.m_nChannels)) return FALSE;
	// Midi Import
	if (m_SndFile.m_nType == MOD_TYPE_MID)
	{
		CDLSBank *pCachedBank = NULL, *pEmbeddedBank = NULL;
		CHAR szCachedBankFile[_MAX_PATH] = "";

		if (CDLSBank::IsDLSBank(lpszPathName))
		{
			pEmbeddedBank = new CDLSBank();
			pEmbeddedBank->Open(lpszPathName);
		}
		m_SndFile.ChangeModTypeTo(MOD_TYPE_IT);
		BeginWaitCursor();
		LPMIDILIBSTRUCT lpMidiLib = CTrackApp::GetMidiLibrary();
		// Scan Instruments
		if (lpMidiLib) for (UINT nIns=1; nIns<=m_SndFile.m_nInstruments; nIns++) if (m_SndFile.Headers[nIns])
		{
			LPCSTR pszMidiMapName;
			INSTRUMENTHEADER *penv = m_SndFile.Headers[nIns];
			UINT nMidiCode;
			BOOL bEmbedded = FALSE;

			if (penv->nMidiChannel == 10)
				nMidiCode = 0x80 | (penv->nMidiDrumKey & 0x7F);
			else
				nMidiCode = penv->nMidiProgram & 0x7F;
			pszMidiMapName = lpMidiLib->MidiMap[nMidiCode];
			if (pEmbeddedBank)
			{
				UINT nDlsIns = 0, nDrumRgn = 0;
				UINT nProgram = penv->nMidiProgram;
				UINT dwKey = (nMidiCode < 128) ? 0xFF : (nMidiCode & 0x7F);
				if ((pEmbeddedBank->FindInstrument(	(nMidiCode >= 128),
													(penv->wMidiBank & 0x3FFF),
													nProgram, dwKey, &nDlsIns))
				 || (pEmbeddedBank->FindInstrument(	(nMidiCode >= 128),	0xFFFF,
													(nMidiCode >= 128) ? 0xFF : nProgram,
													dwKey, &nDlsIns)))
				{
					if (dwKey < 0x80) nDrumRgn = pEmbeddedBank->GetRegionFromKey(nDlsIns, dwKey);
					if (pEmbeddedBank->ExtractInstrument(&m_SndFile, nIns, nDlsIns, nDrumRgn))
					{
						if ((dwKey >= 24) && (dwKey < 100))
						{
							lstrcpyn(penv->name, szMidiPercussionNames[dwKey-24], sizeof(penv->name));
						}
						bEmbedded = TRUE;
					}
				}
			}
			if ((pszMidiMapName) && (pszMidiMapName[0]) && (!bEmbedded))
			{
				// Load From DLS Bank
				if (CDLSBank::IsDLSBank(pszMidiMapName))
				{
					CDLSBank *pDLSBank = NULL;
					
					if ((pCachedBank) && (!lstrcmpi(szCachedBankFile, pszMidiMapName)))
					{
						pDLSBank = pCachedBank;
					} else
					{
						if (pCachedBank) delete pCachedBank;
						pCachedBank = new CDLSBank;
						strcpy(szCachedBankFile, pszMidiMapName);
						if (pCachedBank->Open(pszMidiMapName)) pDLSBank = pCachedBank;
					}
					if (pDLSBank)
					{
						UINT nDlsIns = 0, nDrumRgn = 0;
						UINT nProgram = penv->nMidiProgram;
						UINT dwKey = (nMidiCode < 128) ? 0xFF : (nMidiCode & 0x7F);
						if ((pDLSBank->FindInstrument(	(nMidiCode >= 128),
														(penv->wMidiBank & 0x3FFF),
														nProgram, dwKey, &nDlsIns))
						 || (pDLSBank->FindInstrument(	(nMidiCode >= 128), 0xFFFF,
														(nMidiCode >= 128) ? 0xFF : nProgram,
														dwKey, &nDlsIns)))
						{
							if (dwKey < 0x80) nDrumRgn = pDLSBank->GetRegionFromKey(nDlsIns, dwKey);
							pDLSBank->ExtractInstrument(&m_SndFile, nIns, nDlsIns, nDrumRgn);
							if ((dwKey >= 24) && (dwKey < 24+61))
							{
								lstrcpyn(penv->name, szMidiPercussionNames[dwKey-24], sizeof(penv->name));
							}
						}
					}
				} else
				{
					// Load from Instrument or Sample file
					CHAR szName[_MAX_FNAME], szExt[_MAX_EXT];
					CFile f;

					if (f.Open(pszMidiMapName, CFile::modeRead))
					{
						DWORD len = f.GetLength();
						LPBYTE lpFile;
						if ((len) && ((lpFile = (LPBYTE)GlobalAllocPtr(GHND, len)) != NULL))
						{
							f.Read(lpFile, len);
							m_SndFile.ReadInstrumentFromFile(nIns, lpFile, len);
							_splitpath(pszMidiMapName, NULL, NULL, szName, szExt);
							strncat(szName, szExt, sizeof(szName));
							penv = m_SndFile.Headers[nIns];
							if (!penv->filename[0]) lstrcpyn(penv->filename, szName, sizeof(penv->filename));
							if (!penv->name[0])
							{
								if (nMidiCode < 128)
								{
									lstrcpyn(penv->name, szMidiProgramNames[nMidiCode], sizeof(penv->name));
								} else
								{
									UINT nKey = nMidiCode & 0x7F;
									if (nKey >= 24)
										lstrcpyn(penv->name, szMidiPercussionNames[nKey-24], sizeof(penv->name));
								}
							}
						}
						f.Close();
					}
				}
			}
		}
		if (pCachedBank) delete pCachedBank;
		if (pEmbeddedBank) delete pEmbeddedBank;
		EndWaitCursor();
	}
	// Convert to MOD/S3M/XM/IT
	switch(m_SndFile.m_nType)
	{
	case MOD_TYPE_MOD:
	case MOD_TYPE_S3M:
	case MOD_TYPE_XM:
	case MOD_TYPE_IT:
	case MOD_TYPE_MPT:
		bModified = FALSE;
		break;
	case MOD_TYPE_AMF0:
	case MOD_TYPE_MTM:
	case MOD_TYPE_669:
		m_SndFile.ChangeModTypeTo(MOD_TYPE_MOD);
		break;
	case MOD_TYPE_MED:
	case MOD_TYPE_OKT:
	case MOD_TYPE_AMS:
	case MOD_TYPE_MT2:
		m_SndFile.ChangeModTypeTo(MOD_TYPE_XM);
		if ((m_SndFile.m_nDefaultTempo == 125) && (m_SndFile.m_nDefaultSpeed == 6) && (!m_SndFile.m_nInstruments))
		{
			m_SndFile.m_nType = MOD_TYPE_MOD;
			for (UINT i=0; i<m_SndFile.Patterns.Size(); i++)
				if ((m_SndFile.Patterns[i]) && (m_SndFile.PatternSize[i] != 64))
					m_SndFile.m_nType = MOD_TYPE_XM;
		}
		break;
	case MOD_TYPE_FAR:
	case MOD_TYPE_PTM:
	case MOD_TYPE_STM:
	case MOD_TYPE_DSM:
	case MOD_TYPE_AMF:
	case MOD_TYPE_PSM:
		m_SndFile.ChangeModTypeTo(MOD_TYPE_S3M);
		break;
	default:
		m_SndFile.ChangeModTypeTo(MOD_TYPE_IT);
	}

// -> CODE#0015
// -> DESC="channels management dlg"
	ReinitRecordState();
// -! NEW_FEATURE#0015
	if (m_SndFile.m_dwLastSavedWithVersion > MptVersion::num) {
		char s[256];
		wsprintf(s, "Warning: this song was last saved with a more recent version of OpenMPT.\r\nSong saved with: v%s. Current version: v%s.\r\n", 
			MptVersion::ToStr(m_SndFile.m_dwLastSavedWithVersion),
			MptVersion::str);
		::AfxMessageBox(s);
	}

	SetModifiedFlag(FALSE); // (bModified);
	m_bHasValidPath=true;

	return TRUE;
}


BOOL CModDoc::OnSaveDocument(LPCTSTR lpszPathName)
//------------------------------------------------
{
	static int greccount = 0;
	TCHAR fext[_MAX_EXT]="";
	UINT nType = m_SndFile.m_nType, dwPacking = 0;
	BOOL bOk = FALSE;
	m_SndFile.m_dwLastSavedWithVersion = MptVersion::num;
	if (!lpszPathName) return FALSE;
	_tsplitpath(lpszPathName, NULL, NULL, NULL, fext);
	if (!lstrcmpi(fext, ".mod")) nType = MOD_TYPE_MOD; else
	if (!lstrcmpi(fext, ".s3m")) nType = MOD_TYPE_S3M; else
	if (!lstrcmpi(fext, ".xm")) nType = MOD_TYPE_XM; else
// -> CODE#0023
// -> DESC="IT project files (.itp)"
//	if (!lstrcmpi(fext, ".it")) nType = MOD_TYPE_IT; else
	if (!lstrcmpi(fext, ".it") || !lstrcmpi(fext, ".itp")) nType = MOD_TYPE_IT; else
	if (!lstrcmpi(fext, ".mptm")) nType = MOD_TYPE_MPT; else
// -! NEW_FEATURE#0023
	if (!greccount)
	{
		greccount++;
		bOk = DoSave(NULL, TRUE);
		greccount--;
        return bOk;
	}
	BeginWaitCursor();
	switch(nType)
	{
	case MOD_TYPE_MOD:	bOk = m_SndFile.SaveMod(lpszPathName, dwPacking); break;
	case MOD_TYPE_S3M:	bOk = m_SndFile.SaveS3M(lpszPathName, dwPacking); break;
	case MOD_TYPE_XM:	bOk = m_SndFile.SaveXM(lpszPathName, dwPacking); break;
	case MOD_TYPE_IT:	bOk = (m_SndFile.m_dwSongFlags & SONG_ITPROJECT || !lstrcmpi(fext, ".itp")) ? m_SndFile.SaveITProject(lpszPathName) : m_SndFile.SaveIT(lpszPathName, dwPacking); break;
	case MOD_TYPE_MPT:	bOk = m_SndFile.SaveIT(lpszPathName, dwPacking); break;
	}
	EndWaitCursor();
	if (bOk)
	{
		if (nType == m_SndFile.m_nType) SetPathName(lpszPathName);
	} else
	{
		if(nType == MOD_TYPE_IT && m_SndFile.m_dwSongFlags & SONG_ITPROJECT) ::MessageBox(NULL,"ITP projects need to have a path set for each instrument...",NULL,MB_ICONERROR | MB_OK);
		else ErrorBox(IDS_ERR_SAVESONG, CMainFrame::GetMainFrame());
	}
	return bOk;
}


// -> CODE#0023
// -> DESC="IT project files (.itp)"
BOOL CModDoc::SaveModified()
//--------------------------
{
	if((m_SndFile.m_nType & MOD_TYPE_IT) && m_SndFile.m_dwSongFlags & SONG_ITPROJECT && !(m_SndFile.m_dwSongFlags & SONG_ITPEMBEDIH)){

		BOOL unsavedInstrument = FALSE;

		for(UINT i = 0 ; i < m_SndFile.m_nInstruments ; i++){
			if(m_SndFile.instrumentModified[i]) { unsavedInstrument = TRUE; break; }
		}

		if(unsavedInstrument && ::MessageBox(NULL,"Do you want to save modified instruments ?",NULL,MB_ICONQUESTION | MB_YESNO | MB_APPLMODAL) == IDYES){

			for(UINT i = 0 ; i < m_SndFile.m_nInstruments ; i++){
				if(m_SndFile.m_szInstrumentPath[i][0] != '\0'){
					int size = strlen(m_SndFile.m_szInstrumentPath[i]);
					BOOL iti = _stricmp(&m_SndFile.m_szInstrumentPath[i][size-3],"iti") == 0;
					BOOL xi  = _stricmp(&m_SndFile.m_szInstrumentPath[i][size-2],"xi") == 0;

					if(iti || (!iti && !xi  && m_SndFile.m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT)))
						m_SndFile.SaveITIInstrument(i+1, m_SndFile.m_szInstrumentPath[i]);
					if(xi  || (!xi  && !iti && m_SndFile.m_nType == MOD_TYPE_XM))
						m_SndFile.SaveXIInstrument(i+1, m_SndFile.m_szInstrumentPath[i]);
				}
			}
		}
	}

	return CDocument::SaveModified();
}
// -! NEW_FEATURE#0023


void CModDoc::OnCloseDocument()
//-----------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm) pMainFrm->OnDocumentClosed(this);
	CDocument::OnCloseDocument();
}


void CModDoc::DeleteContents()
//----------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm) pMainFrm->StopMod(this);
	m_SndFile.Destroy();
// -> CODE#0015
// -> DESC="channels management dlg"
	ReinitRecordState();
// -! NEW_FEATURE#0015
}


BOOL CModDoc::DoSave(LPCSTR lpszPathName, BOOL)
//---------------------------------------------
{
	CHAR s[_MAX_PATH] = "";
	CHAR path[_MAX_PATH]="", drive[_MAX_DRIVE]="";
	CHAR fname[_MAX_FNAME]="", fext[_MAX_EXT]="";
	LPCSTR lpszFilter = NULL, lpszDefExt = NULL;
	
	switch(m_SndFile.GetType())
	{
	case MOD_TYPE_MOD:
		MsgBoxHidable(ModCompatibilityExportTip);
		lpszDefExt = "mod";
		lpszFilter = FileFilterMOD;
		strcpy(fext, ".mod");
		break;
	case MOD_TYPE_S3M:
		lpszDefExt = "s3m";
		lpszFilter = FileFilterS3M;
		strcpy(fext, ".s3m");
		break;
	case MOD_TYPE_XM:
		lpszDefExt = "xm";
		lpszFilter = FileFilterXM;
		strcpy(fext, ".xm");
		break;
	case MOD_TYPE_IT:
// -> CODE#0023
// -> DESC="IT project files (.itp)"
//		lpszDefExt = "it";
//		lpszFilter = "Impulse Tracker Modules (*.it)|*.it||";
//		strcpy(fext, ".it");
		if(m_SndFile.m_dwSongFlags & SONG_ITPROJECT){
			lpszDefExt = "itp";
			lpszFilter = FileFilterITP;
			strcpy(fext, ".itp");
		}
		else
		{
			MsgBoxHidable(ItCompatibilityExportTip);
			lpszDefExt = "it";
			lpszFilter = FileFilterIT;
			strcpy(fext, ".it");
		}
// -! NEW_FEATURE#0023
		break;
	case MOD_TYPE_MPT:
			lpszDefExt = "mptm";
			lpszFilter = FileFilterMPT;
			strcpy(fext, ".mptm");
		break;
	default:	
		ErrorBox(IDS_ERR_SAVESONG, CMainFrame::GetMainFrame());
		return FALSE;
	}
	if ((!lpszPathName) || (!lpszPathName[0]) || m_ShowSavedialog)
	{
		_splitpath(m_strPathName, drive, path, fname, NULL);
		if (!fname[0]) strcpy(fname, m_strTitle);
		strcpy(s, drive);
		strcat(s, path);
		strcat(s, fname);
		strcat(s, fext);
		CFileDialog dlg(FALSE, lpszDefExt, s,
			OFN_HIDEREADONLY| OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
			lpszFilter,
			theApp.m_pMainWnd);
		if (CMainFrame::m_szCurModDir[0])
		{
			dlg.m_ofn.lpstrInitialDir = CMainFrame::m_szCurModDir;
		}
		if (dlg.DoModal() != IDOK) return FALSE;
		strcpy(s, dlg.GetPathName());
		_splitpath(s, drive, path, fname, fext);
		strcpy(CMainFrame::m_szCurModDir, drive);
		strcat(CMainFrame::m_szCurModDir, path);
	} else
	{
		_splitpath(lpszPathName, drive, path, fname, NULL);
		strcpy(s, drive);
		strcat(s, path);
		strcat(s, fname);
		strcat(s, fext);
	}
	// Do we need to create a backup file ?
	if ((CMainFrame::m_dwPatternSetup & PATTERN_CREATEBACKUP)
	 && (IsModified()) && (!lstrcmpi(s, m_strPathName)))
	{
		CFileStatus rStatus;
		if (CFile::GetStatus(s, rStatus))
		{
			CHAR bkname[_MAX_PATH] = "";
			_splitpath(s, bkname, path, fname, NULL);
			strcat(bkname, path);
			strcat(bkname, fname);
			strcat(bkname, ".bak");
			if (CFile::GetStatus(bkname, rStatus))
			{
				DeleteFile(bkname);
			}
			MoveFile(s, bkname);
		}
	}
	if (OnSaveDocument(s))
	{
		SetModified(FALSE);
		m_bHasValidPath=true;
		m_ShowSavedialog = false;
		UpdateAllViews(NULL, HINT_MODGENERAL); // Update treeview (e.g. filename might have changed).
		return TRUE;
	} else
	{
// -> CODE#0023
// -> DESC="IT project files (.itp)"
//		ErrorBox(IDS_ERR_SAVESONG, CMainFrame::GetMainFrame());	// done in OnSaveDocument()
// -! NEW_FEATURE#0023
		return FALSE;
	}
}


void CModDoc::InitPlayer()
//------------------------
{
	m_SndFile.ResetChannels();
}


BOOL CModDoc::InitializeMod()
//---------------------------
{
	// New module ?
	if (!m_SndFile.m_nChannels)
	{
// -> CODE#0006
// -> DESC="misc quantity changes"
//		m_SndFile.m_nChannels = (m_SndFile.m_nType & MOD_TYPE_MOD) ? 8 : 16;
		m_SndFile.m_nChannels = (m_SndFile.m_nType & MOD_TYPE_MOD) ? 8 : 32;
// -! BEHAVIOUR_CHANGE#0006
		if (m_SndFile.Order[0] >= m_SndFile.Patterns.Size())	m_SndFile.Order[0] = 0;
		if (!m_SndFile.Patterns[0])
		{
			m_SndFile.Patterns.Insert(0, 64);
		}
		strcpy(m_SndFile.m_szNames[0], "untitled");
		m_SndFile.m_nMusicTempo = m_SndFile.m_nDefaultTempo = 125;
		m_SndFile.m_nMusicSpeed = m_SndFile.m_nDefaultSpeed = 6;
		m_SndFile.m_nGlobalVolume = m_SndFile.m_nDefaultGlobalVolume = 128;
		for (UINT init=0; init<MAX_BASECHANNELS; init++)
		{
			m_SndFile.ChnSettings[init].dwFlags = 0;
			m_SndFile.ChnSettings[init].nVolume = 64;
			if (m_SndFile.m_nType & (MOD_TYPE_XM|MOD_TYPE_IT | MOD_TYPE_MPT))
				m_SndFile.ChnSettings[init].nPan = 128;
			else
				m_SndFile.ChnSettings[init].nPan = (init & 0x01) ? 64 : 192;
			m_SndFile.Chn[init].nGlobalVol = 64;
		}
	}
	if (!m_SndFile.m_nSamples)
	{
		strcpy(m_SndFile.m_szNames[1], "untitled");
		m_SndFile.m_nSamples = 1;
		m_SndFile.Ins[1].nVolume = 256;
		m_SndFile.Ins[1].nGlobalVol = 64;
		m_SndFile.Ins[1].nPan = 128;
		m_SndFile.Ins[1].nC4Speed = 8363;
		if ((!m_SndFile.m_nInstruments) && (m_SndFile.m_nType & MOD_TYPE_XM))
		{
			m_SndFile.m_nInstruments = 1;
			m_SndFile.Headers[1] = new INSTRUMENTHEADER;
			InitializeInstrument(m_SndFile.Headers[1], 1);
		}
		if (m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT|MOD_TYPE_XM))
		{
			m_SndFile.m_dwSongFlags |= SONG_LINEARSLIDES;
		}
	}
	m_SndFile.SetCurrentPos(0);

	return TRUE;
}


void CModDoc::PostMessageToAllViews(UINT uMsg, WPARAM wParam, LPARAM lParam)
//--------------------------------------------------------------------------
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView) pView->PostMessage(uMsg, wParam, lParam);
	} 
}


void CModDoc::SendMessageToActiveViews(UINT uMsg, WPARAM wParam, LPARAM lParam)
//-----------------------------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm)
	{
		CMDIChildWnd *pMDI = pMainFrm->MDIGetActive();
		if (pMDI) pMDI->SendMessageToDescendants(uMsg, wParam, lParam);
	}
}


void CModDoc::ViewPattern(UINT nPat, UINT nOrd)
//---------------------------------------------
{
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_PATTERNS, ((nPat+1) << 16) | nOrd);
}


void CModDoc::ViewSample(UINT nSmp)
//---------------------------------
{
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_SAMPLES, nSmp);
}


void CModDoc::ViewInstrument(UINT nIns)
//-------------------------------------
{
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_INSTRUMENTS, nIns);
}


BOOL CModDoc::AddToLog(LPCSTR lpszLog)
//------------------------------------
{
	UINT len;
	if ((!lpszLog) || (!lpszLog[0])) return FALSE;
	len = strlen(lpszLog);
	if (m_lpszLog)
	{
		LPSTR p = new CHAR[strlen(m_lpszLog)+len+2];
		if (p)
		{
			strcpy(p, m_lpszLog);
			strcat(p, lpszLog);
			delete[] m_lpszLog;
			m_lpszLog = p;
		}
	} else
	{
		m_lpszLog = new CHAR[len+2];
		if (m_lpszLog) strcpy(m_lpszLog, lpszLog);
	}
	return TRUE;
}


BOOL CModDoc::ClearLog()
//----------------------
{
	if (m_lpszLog)
	{
		delete[] m_lpszLog;
		m_lpszLog = NULL;
	}
	return TRUE;
}


UINT CModDoc::ShowLog(LPCSTR lpszTitle, CWnd *parent)
//---------------------------------------------------
{
	if (!lpszTitle) lpszTitle = MAINFRAME_TITLE;
	if (m_lpszLog)
	{
#if 0
		CShowLogDlg dlg(parent);
		return dlg.ShowLog(m_lpszLog, lpszTitle);
#else
		return ::MessageBox((parent) ? parent->m_hWnd : NULL, m_lpszLog, lpszTitle, MB_OK|MB_ICONINFORMATION);
#endif
	}
	return IDCANCEL;
}

UINT CModDoc::PlayNote(UINT note, UINT nins, UINT nsmp, BOOL bpause, LONG nVol, LONG loopstart, LONG loopend, int nCurrentChn) //rewbs.vstiLive: added current chan param
//-----------------------------------------------------------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	UINT nChn = m_SndFile.m_nChannels;
	
	if ((!pMainFrm) || (!note)) return FALSE;
	if (nVol > 256) nVol = 256;
	if (note < 128)
	{
		BEGIN_CRITICAL();
		
		//kill notes if required.
		if ( (bpause) || (m_SndFile.IsPaused()) || pMainFrm->GetModPlaying() != this) { 
			//OnPlayerPause();				  // pause song - pausing VSTis is too slow
			pMainFrm->SetLastMixActiveTime(); // mark activity

			// All notes off
			for (UINT i=0; i<MAX_CHANNELS; i++)	{
				if ((i < m_SndFile.m_nChannels) || (m_SndFile.Chn[i].nMasterChn)) {
					m_SndFile.Chn[i].dwFlags |= CHN_KEYOFF | CHN_NOTEFADE;
					m_SndFile.Chn[i].nFadeOutVol = 0;
				}
			}
		}

		//find a channel if required
		//if (nCurrentChn<0) { 
			nChn = FindAvailableChannel();
		//}

		MODCHANNEL *pChn = &m_SndFile.Chn[nChn];
		
		//stop channel, just in case.
		if (pChn->nLength)	{
			pChn->nPos = pChn->nPosLo = pChn->nLength = 0;
		}

		//reset channel properties; in theory the chan is completely unused anyway.
		pChn->dwFlags &= 0xFF;
		pChn->dwFlags &= ~(CHN_MUTE);
		pChn->nGlobalVol = 64;
		pChn->nInsVol = 64;
		pChn->nPan = 128;
		pChn->nRightVol = pChn->nLeftVol = 0;
		pChn->nROfs = pChn->nLOfs = 0;
		pChn->nCutOff = 0x7F;
		pChn->nResonance = 0;
		pChn->nVolume = 256;
		pChn->nMasterChn = 0;	//remove NNA association
		pChn->nNewNote = note;

		if (nins) {									//Set instrument
			m_SndFile.resetEnvelopes(pChn);
			m_SndFile.InstrumentChange(pChn, nins);
		} 
		else if ((nsmp) && (nsmp < MAX_SAMPLES)) {	//Or set sample
			MODINSTRUMENT *pins = &m_SndFile.Ins[nsmp];
			pChn->pCurrentSample = pins->pSample;
			pChn->pHeader = NULL;
			pChn->pInstrument = pins;
			pChn->pSample = pins->pSample;
			pChn->nFineTune = pins->nFineTune;
			pChn->nC4Speed = pins->nC4Speed;
			pChn->nPos = pChn->nPosLo = pChn->nLength = 0;
			pChn->nLoopStart = pins->nLoopStart;
			pChn->nLoopEnd = pins->nLoopEnd;
			pChn->dwFlags = pins->uFlags & (0xFF & ~CHN_MUTE);
			pChn->nPan = 128;
			if (pins->uFlags & CHN_PANNING) pChn->nPan = pins->nPan;
			pChn->nInsVol = pins->nGlobalVol;
			pChn->nFadeOutVol = 0x10000;
		}

		m_SndFile.NoteChange(nChn, note, FALSE, TRUE, TRUE);
		if (nVol >= 0) pChn->nVolume = nVol;
		
		// handle sample looping.
		// Fix: Bug report 1700.
		//if ((loopstart + 16 < loopend) && (loopstart >= 0) && (loopend <= (LONG)pChn->nLength)) 	{
		if ((loopstart + 16 < loopend) && (loopstart >= 0) && (pChn->pInstrument != 0))
		{
			pChn->nPos = loopstart;
			pChn->nPosLo = 0;
			pChn->nLoopStart = loopstart;
			pChn->nLoopEnd = loopend;
			pChn->nLength = min(UINT(loopend), pChn->pInstrument->nLength);
		}

		// handle extra-loud flag
		if ((!(CMainFrame::m_dwPatternSetup & PATTERN_NOEXTRALOUD)) && (nsmp)) {
			pChn->dwFlags |= CHN_EXTRALOUD;
		} else {
			pChn->dwFlags &= ~CHN_EXTRALOUD;
		}

		/*
		if (bpause) {   
			if ((loopstart + 16 < loopend) && (loopstart >= 0) && (loopend <= (LONG)pChn->nLength)) 	{
				pChn->nPos = loopstart;
				pChn->nPosLo = 0;
				pChn->nLoopStart = loopstart;
				pChn->nLoopEnd = loopend;
				pChn->nLength = loopend;
			}
			m_SndFile.m_nBufferCount = 0;
			m_SndFile.m_dwSongFlags |= SONG_PAUSED;
			if ((!(CMainFrame::m_dwPatternSetup & PATTERN_NOEXTRALOUD)) && (nsmp)) pChn->dwFlags |= CHN_EXTRALOUD;
		} else pChn->dwFlags &= ~CHN_EXTRALOUD;
		*/

		//rewbs.vstiLive
		if (nins <= m_SndFile.m_nInstruments)
		{
			INSTRUMENTHEADER *penv = m_SndFile.Headers[nins];
			if (penv && penv->nMidiChannel > 0 && penv->nMidiChannel < 17) // instro sends to a midi chan
			{
				// UINT nPlugin = m_SndFile.GetBestPlugin(nChn, PRIORITISE_INSTRUMENT, EVEN_IF_MUTED);
				 
				UINT nPlugin = 0;
				if (pChn->pHeader) 
					nPlugin = pChn->pHeader->nMixPlug;  					// first try intrument VST
				if ((!nPlugin) || (nPlugin > MAX_MIXPLUGINS) && (nCurrentChn >=0))
					nPlugin = m_SndFile.ChnSettings[nCurrentChn].nMixPlugin; // Then try Channel VST
				
   				if ((nPlugin) && (nPlugin <= MAX_MIXPLUGINS))
				{
					IMixPlugin *pPlugin =  m_SndFile.m_MixPlugins[nPlugin-1].pMixPlugin;
					if (pPlugin) pPlugin->MidiCommand(penv->nMidiChannel, penv->nMidiProgram, penv->wMidiBank, note, pChn->nVolume, MAX_BASECHANNELS);
					//if (pPlugin) pPlugin->MidiCommand(penv->nMidiChannel, penv->nMidiProgram, penv->wMidiBank, note, pChn->GetVSTVolume(), MAX_BASECHANNELS);
				}
			}
		}
		//end rewbs.vstiLive

		END_CRITICAL();

		if (pMainFrm->GetModPlaying() != this)
		{
			m_SndFile.m_dwSongFlags |= SONG_PAUSED;
			pMainFrm->PlayMod(this, m_hWndFollow, m_dwNotifyType);
		}
		CMainFrame::EnableLowLatencyMode();
	} else
	{
		BEGIN_CRITICAL();
		m_SndFile.NoteChange(nChn, note);
		if (bpause) m_SndFile.m_dwSongFlags |= SONG_PAUSED;
		END_CRITICAL();
	}
	return nChn;
}


BOOL CModDoc::NoteOff(UINT note, BOOL bFade, UINT nins, UINT nCurrentChn) //rewbs.vstiLive: added chan and nins
//--------------------------------------------------------------------------
{
	BEGIN_CRITICAL();


	//rewbs.vstiLive
	if (nins>0 && nins<=m_SndFile.m_nInstruments) {

		INSTRUMENTHEADER *penv = m_SndFile.Headers[nins];
		if (penv && penv->nMidiChannel > 0 && penv->nMidiChannel < 17) // instro sends to a midi chan
		{

			UINT nPlugin = penv->nMixPlug;  		// First try intrument VST
			if (((!nPlugin) || (nPlugin > MAX_MIXPLUGINS)) && //no good plug yet
				(nCurrentChn<MAX_CHANNELS)) // chan OK
				nPlugin = m_SndFile.ChnSettings[nCurrentChn].nMixPlugin;// Then try Channel VST
			
			if ((nPlugin) && (nPlugin <= MAX_MIXPLUGINS))
			{
				IMixPlugin *pPlugin =  m_SndFile.m_MixPlugins[nPlugin-1].pMixPlugin;
				if (pPlugin) pPlugin->MidiCommand(penv->nMidiChannel, penv->nMidiProgram, penv->wMidiBank, note+0xFF, 0, MAX_BASECHANNELS);

			}
		}
	}
	//end rewbs.vstiLive

	MODCHANNEL *pChn = &m_SndFile.Chn[m_SndFile.m_nChannels];
	for (UINT i=m_SndFile.m_nChannels; i<MAX_CHANNELS; i++, pChn++) if (!pChn->nMasterChn)
	{
		DWORD mask = (bFade) ? CHN_NOTEFADE : (CHN_NOTEFADE|CHN_KEYOFF);

		// Fade all channels > m_nChannels which are playing this note. 
		// Could conflict with NNAs.
		if ((!(pChn->dwFlags & mask)) && (pChn->nLength) && ((note == pChn->nNewNote) || (!note)))
		{
			m_SndFile.KeyOff(i);
			if (!m_SndFile.m_nInstruments) pChn->dwFlags &= ~CHN_LOOP;
			if (bFade) pChn->dwFlags |= CHN_NOTEFADE;
			if (note) break;
		}
	}
	END_CRITICAL();
	return TRUE;
}


BOOL CModDoc::IsNotePlaying(UINT note, UINT nsmp, UINT nins)
//----------------------------------------------------------
{
	MODCHANNEL *pChn = &m_SndFile.Chn[m_SndFile.m_nChannels];
	for (UINT i=m_SndFile.m_nChannels; i<MAX_CHANNELS; i++, pChn++) if (!pChn->nMasterChn)
	{
		if ((pChn->nLength) && (!(pChn->dwFlags & (CHN_NOTEFADE|CHN_KEYOFF|CHN_MUTE)))
		 && ((note == pChn->nNewNote) || (!note))
		 && ((pChn->pInstrument == &m_SndFile.Ins[nsmp]) || (!nsmp))
		 && ((pChn->pHeader == m_SndFile.Headers[nins]) || (!nins))) return TRUE;
	}
	return FALSE;
}


BOOL CModDoc::MuteChannel(UINT nChn, BOOL doMute)
//----------------------------------------------
{
	DWORD muteType = (CMainFrame::m_dwPatternSetup&PATTERN_SYNCMUTE)? CHN_SYNCMUTE:CHN_MUTE;

	if (nChn >= m_SndFile.m_nChannels) {
		return FALSE;
	}

	//Mark channel as muted in channel settings
	if (doMute)	{
		m_SndFile.ChnSettings[nChn].dwFlags |= CHN_MUTE;
	} else {
		m_SndFile.ChnSettings[nChn].dwFlags &= ~CHN_MUTE;
	}
	
	//Mute pattern channel
	if (doMute) {
		m_SndFile.Chn[nChn].dwFlags |= muteType;
		//Kill VSTi notes on muted channel.
		UINT nPlug = m_SndFile.GetBestPlugin(nChn, PRIORITISE_INSTRUMENT, EVEN_IF_MUTED);
		if ((nPlug) && (nPlug<=MAX_MIXPLUGINS))	{
			CVstPlugin *pPlug = (CVstPlugin*)m_SndFile.m_MixPlugins[nPlug-1].pMixPlugin;
			INSTRUMENTHEADER* penv = m_SndFile.Chn[nChn].pHeader;
			if (pPlug && penv) {
				pPlug->MidiCommand(penv->nMidiChannel, penv->nMidiProgram, penv->wMidiBank, 0xFF, 0, nChn);
			}
		}
	} else {
		//on unmute alway cater for both mute types - this way there's no probs if user changes mute mode.
		m_SndFile.Chn[nChn].dwFlags &= ~(CHN_SYNCMUTE|CHN_MUTE);
	}

	//mute any NNA'd channels
	for (UINT i=m_SndFile.m_nChannels; i<MAX_CHANNELS; i++) {
		if (m_SndFile.Chn[i].nMasterChn == nChn+1)	{
			if (doMute) { 
				m_SndFile.Chn[i].dwFlags |= muteType;
			} else {
				//on unmute alway cater for both mute types - this way there's no probs if user changes mute mode.
				m_SndFile.Chn[i].dwFlags &= ~(CHN_SYNCMUTE|CHN_MUTE);
			}
		}
	}

	//Mark IT as modified
	if (m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) {
		CMainFrame::GetMainFrame()->ThreadSafeSetModified(this);
	}

	return TRUE;
}

// -> CODE#0012
// -> DESC="midi keyboard split"
BOOL CModDoc::IsChannelSolo(UINT nChn) const
//------------------------------------------
{
	if (nChn >= m_SndFile.m_nChannels) return TRUE;
	return (m_SndFile.ChnSettings[nChn].dwFlags & CHN_SOLO) ? TRUE : FALSE;
}

BOOL CModDoc::SoloChannel(UINT nChn, BOOL bSolo)
//---------------------------------------------
{
	if (nChn >= m_SndFile.m_nChannels) return FALSE;
	if (m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) SetModified();
	if (bSolo)	m_SndFile.ChnSettings[nChn].dwFlags |= CHN_SOLO;
	else		m_SndFile.ChnSettings[nChn].dwFlags &= ~CHN_SOLO;
	return TRUE;
}
// -! NEW_FEATURE#0012


// -> CODE#0015
// -> DESC="channels management dlg"
BOOL CModDoc::IsChannelNoFx(UINT nChn) const
//------------------------------------------
{
	if (nChn >= m_SndFile.m_nChannels) return TRUE;
	return (m_SndFile.ChnSettings[nChn].dwFlags & CHN_NOFX) ? TRUE : FALSE;
}

BOOL CModDoc::NoFxChannel(UINT nChn, BOOL bNoFx, BOOL updateMix)
//--------------------------------------------------------------
{
	if (nChn >= m_SndFile.m_nChannels) return FALSE;
	if (m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) SetModified();
	if (bNoFx){
		m_SndFile.ChnSettings[nChn].dwFlags |= CHN_NOFX;
		if(updateMix) m_SndFile.Chn[nChn].dwFlags |= CHN_NOFX;
	}
	else{
		m_SndFile.ChnSettings[nChn].dwFlags &= ~CHN_NOFX;
		if(updateMix) m_SndFile.Chn[nChn].dwFlags &= ~CHN_NOFX;
	}
	return TRUE;
}

BOOL CModDoc::IsChannelRecord1(UINT channel)
//------------------------------------------
{
	UINT m = 1 << (channel&7);
	return (MultiRecordMask[channel>>3] & m) ? TRUE : FALSE;
}

BOOL CModDoc::IsChannelRecord2(UINT channel)
//------------------------------------------
{
	UINT m = 1 << (channel&7);
	return (MultiSplitRecordMask[channel>>3] & m) ? TRUE : FALSE;
}

BYTE CModDoc::IsChannelRecord(UINT channel)
//-----------------------------------------
{
	if(IsChannelRecord1(channel)) return 1;
	if(IsChannelRecord2(channel)) return 2;
	return 0;
}

void CModDoc::Record1Channel(UINT channel, BOOL select)
//-----------------------------------------------------
{
	UINT m = 1 << (channel&7);

	if(!select){
		if(MultiRecordMask[channel>>3] & m) MultiRecordMask[channel>>3] ^= m;
		if(MultiSplitRecordMask[channel>>3] & m) MultiSplitRecordMask[channel>>3] ^= m;
	}
	else{
		MultiRecordMask[channel>>3] ^= m;
		if(MultiSplitRecordMask[channel>>3] & m) MultiSplitRecordMask[channel>>3] ^= m;
	}
}

void CModDoc::Record2Channel(UINT channel, BOOL select)
//-----------------------------------------------------
{
	UINT m = 1 << (channel&7);

	if(!select){
		if(MultiRecordMask[channel>>3] & m) MultiRecordMask[channel>>3] ^= m;
		if(MultiSplitRecordMask[channel>>3] & m) MultiSplitRecordMask[channel>>3] ^= m;
	}
	else{
		MultiSplitRecordMask[channel>>3] ^= m;
		if(MultiRecordMask[channel>>3] & m) MultiRecordMask[channel>>3] ^= m;
	}
}

void CModDoc::ReinitRecordState(BOOL unselect)
//--------------------------------------------
{
	memset(MultiRecordMask, unselect ? 0 : 0xff, sizeof(MultiRecordMask));
	memset(MultiSplitRecordMask, unselect ? 0 : 0xff, sizeof(MultiSplitRecordMask));	
}
// -! NEW_FEATURE#0015


BOOL CModDoc::MuteSample(UINT nSample, BOOL bMute)
//------------------------------------------------
{
	if ((nSample < 1) || (nSample > m_SndFile.m_nSamples)) return FALSE;
	if (bMute) m_SndFile.Ins[nSample].uFlags |= CHN_MUTE;
	else m_SndFile.Ins[nSample].uFlags &= ~CHN_MUTE;
	return TRUE;
}

BOOL CModDoc::MuteInstrument(UINT nInstr, BOOL bMute)
//---------------------------------------------------
{
	if ((nInstr < 1) || (nInstr > m_SndFile.m_nInstruments) || (!m_SndFile.Headers[nInstr])) return FALSE;
	if (bMute) m_SndFile.Headers[nInstr]->dwFlags |= ENV_MUTE;
	else m_SndFile.Headers[nInstr]->dwFlags &= ~ENV_MUTE;
	return TRUE;
}


BOOL CModDoc::SurroundChannel(UINT nChn, BOOL bSurround)
//------------------------------------------------------
{
	DWORD d = (bSurround) ? CHN_SURROUND : 0;
	
	if (nChn >= m_SndFile.m_nChannels) return FALSE;
	if (!(m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT))) d = 0;
	if (d != (m_SndFile.ChnSettings[nChn].dwFlags & CHN_SURROUND))
	{
		if (m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) SetModified();
		if (d)	m_SndFile.ChnSettings[nChn].dwFlags |= CHN_SURROUND;
		else	m_SndFile.ChnSettings[nChn].dwFlags &= ~CHN_SURROUND;
		
	}
	if (d)	m_SndFile.Chn[nChn].dwFlags |= CHN_SURROUND;
	else	m_SndFile.Chn[nChn].dwFlags &= ~CHN_SURROUND;
	return TRUE;
}


BOOL CModDoc::SetChannelGlobalVolume(UINT nChn, UINT nVolume)
//-----------------------------------------------------------
{
	BOOL bOk = FALSE;
	if ((nChn >= m_SndFile.m_nChannels) || (nVolume > 64)) return FALSE;
	if (m_SndFile.ChnSettings[nChn].nVolume != nVolume)
	{
		m_SndFile.ChnSettings[nChn].nVolume = nVolume;
		if (m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) SetModified();
		bOk = TRUE;
	}
	m_SndFile.Chn[nChn].nGlobalVol = nVolume;
	return bOk;
}


BOOL CModDoc::SetChannelDefaultPan(UINT nChn, UINT nPan)
//------------------------------------------------------
{
	BOOL bOk = FALSE;
	if ((nChn >= m_SndFile.m_nChannels) || (nPan > 256)) return FALSE;
	if (m_SndFile.ChnSettings[nChn].nPan != nPan)
	{
		m_SndFile.ChnSettings[nChn].nPan = nPan;
		if (m_SndFile.m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT | MOD_TYPE_MPT)) SetModified();
		bOk = TRUE;
	}
	m_SndFile.Chn[nChn].nPan = nPan;
	return bOk;
}


BOOL CModDoc::IsChannelMuted(UINT nChn) const
//-------------------------------------------
{
	if (nChn >= m_SndFile.m_nChannels) return TRUE;
	return (m_SndFile.ChnSettings[nChn].dwFlags & CHN_MUTE) ? TRUE : FALSE;
}


BOOL CModDoc::IsSampleMuted(UINT nSample) const
//---------------------------------------------
{
	if ((!nSample) || (nSample > m_SndFile.m_nSamples)) return FALSE;
	return (m_SndFile.Ins[nSample].uFlags & CHN_MUTE) ? TRUE : FALSE;
}


BOOL CModDoc::IsInstrumentMuted(UINT nInstr) const
//------------------------------------------------
{
	if ((!nInstr) || (nInstr > m_SndFile.m_nInstruments) || (!m_SndFile.Headers[nInstr])) return FALSE;
	return (m_SndFile.Headers[nInstr]->dwFlags & ENV_MUTE) ? TRUE : FALSE;
}


UINT CModDoc::GetPatternSize(UINT nPat) const
//-------------------------------------------
{
	if ((nPat < m_SndFile.Patterns.Size()) && (m_SndFile.Patterns[nPat])) return m_SndFile.PatternSize[nPat];
	return 0;
}


void CModDoc::SetFollowWnd(HWND hwnd, DWORD dwType)
//-------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	m_hWndFollow = hwnd;
	m_dwNotifyType = dwType;
	if (pMainFrm) pMainFrm->SetFollowSong(this, m_hWndFollow, TRUE, m_dwNotifyType);
}


BOOL CModDoc::IsChildSample(UINT nIns, UINT nSmp) const
//-----------------------------------------------------
{
	INSTRUMENTHEADER *penv;
	if ((nIns < 1) || (nIns > m_SndFile.m_nInstruments)) return FALSE;
	penv = m_SndFile.Headers[nIns];
	if (penv)
	{
		for (UINT i=0; i<NOTE_MAX; i++)
		{
			if (penv->Keyboard[i] == nSmp) return TRUE;
		}
	}
	return FALSE;
}


UINT CModDoc::FindSampleParent(UINT nSmp) const
//---------------------------------------------
{
	if ((!m_SndFile.m_nInstruments) || (!nSmp)) return 0;
	for (UINT i=1; i<=m_SndFile.m_nInstruments; i++)
	{
		INSTRUMENTHEADER *penv = m_SndFile.Headers[i];
		if (penv)
		{
			for (UINT j=0; j<NOTE_MAX; j++)
			{
				if (penv->Keyboard[j] == nSmp) return i;
			}
		}
	}
	return 0;
}


UINT CModDoc::FindInstrumentChild(UINT nIns) const
//------------------------------------------------
{
	INSTRUMENTHEADER *penv;
	if ((!nIns) || (nIns > m_SndFile.m_nInstruments)) return 0;
	penv = m_SndFile.Headers[nIns];
	if (penv)
	{
		for (UINT i=0; i<NOTE_MAX; i++)
		{
			UINT n = penv->Keyboard[i];
			if ((n) && (n <= m_SndFile.m_nSamples)) return n;
		}
	}
	return 0;
}


LRESULT CModDoc::ActivateView(UINT nIdView, DWORD dwParam)
//--------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (!pMainFrm) return 0;
	CMDIChildWnd *pMDIActive = pMainFrm->MDIGetActive();
	if (pMDIActive)
	{
		CView *pView = pMDIActive->GetActiveView();
		if ((pView) && (pView->GetDocument() == this))
		{
			return ((CChildFrame *)pMDIActive)->ActivateView(nIdView, dwParam);
		}
	}
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = GetNextView(pos);
		if ((pView) && (pView->GetDocument() == this))
		{
			CChildFrame *pChildFrm = (CChildFrame *)pView->GetParentFrame();
			pChildFrm->MDIActivate();
			return pChildFrm->ActivateView(nIdView, dwParam);
		}
	}
	return 0;
}


void CModDoc::UpdateAllViews(CView *pSender, LPARAM lHint, CObject *pHint)
//------------------------------------------------------------------------
{
	CDocument::UpdateAllViews(pSender, lHint, pHint);
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm) pMainFrm->UpdateTree(this, lHint, pHint);
}


/////////////////////////////////////////////////////////////////////////////
// CModDoc commands

void CModDoc::OnFileWaveConvert()
//-------------------------------
{
	CHAR path[_MAX_PATH]="", drive[_MAX_DRIVE]="";
	CHAR s[_MAX_PATH], fname[_MAX_FNAME]="";
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	if ((!pMainFrm) || (!m_SndFile.GetType())) return;
	_splitpath(GetPathName(), drive, path, fname, NULL);
	strcpy(s, fname);
	strcat(s, ".wav");
	CFileDialog dlg(FALSE, "wav", s,
					OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
					"Wave Files (*.wav)|*.wav||", pMainFrm);
	dlg.m_ofn.lpstrInitialDir = pMainFrm->GetExportDir();

	CWaveConvert wsdlg(pMainFrm);
	if (wsdlg.DoModal() != IDOK) return;
	if (dlg.DoModal() != IDOK) return; //rewbs: made filename dialog appear after wav settings dialog
	s[0] = 0;
	_splitpath(dlg.GetPathName(), s, path, NULL, NULL);
	strcat(s, path);
	pMainFrm->SetExportDir(s);
	strcpy(s, dlg.GetPathName());

	// Saving as wave file
// -> CODE#0024
// -> DESC="wav export update"
	UINT p = 0,n = 1;
	DWORD flags[MAX_BASECHANNELS];
	CHAR channel[MAX_CHANNELNAME+10];

	// Channel mode : save song in multiple wav files (one for each enabled channels)
	if(wsdlg.m_bChannelMode){
		n = m_SndFile.m_nChannels;
		for(UINT i = 0 ; i < n ; i++){
			// Save channels' flags
			flags[i] = m_SndFile.ChnSettings[i].dwFlags;
			// Mute each channel
			m_SndFile.ChnSettings[i].dwFlags |= CHN_MUTE;
		}
		// Keep position of the caracter just before ".wav" in path string
		p = strlen(s) - 4;
	}

	CDoWaveConvert dwcdlg(&m_SndFile, s, &wsdlg.WaveFormat.Format, wsdlg.m_bNormalize, pMainFrm);
	dwcdlg.m_dwFileLimit = wsdlg.m_dwFileLimit;
	dwcdlg.m_bGivePlugsIdleTime = wsdlg.m_bGivePlugsIdleTime;
	dwcdlg.m_dwSongLimit = wsdlg.m_dwSongLimit;
	dwcdlg.m_nMaxPatterns = (wsdlg.m_bSelectPlay) ? wsdlg.m_nMaxOrder - wsdlg.m_nMinOrder + 1 : 0;
	//if(wsdlg.m_bHighQuality) CSoundFile::SetResamplingMode(SRCMODE_POLYPHASE);
// -! NEW_FEATURE#0024

	BOOL bplaying = FALSE;
	UINT pos = m_SndFile.GetCurrentPos();
	bplaying = TRUE;
	pMainFrm->PauseMod();

// rewbs.fix3239: moved position definition into loop below
/*  m_SndFile.SetCurrentPos(0);
	if (wsdlg.m_bSelectPlay)
	{
		m_SndFile.SetCurrentOrder(wsdlg.m_nMinOrder);
		m_SndFile.m_nCurrentPattern = wsdlg.m_nMinOrder;
		m_SndFile.GetLength(TRUE, FALSE);
		m_SndFile.m_nMaxOrderPosition = wsdlg.m_nMaxOrder + 1;
	}
*/
//end rewbs.fix3239: moved position definition into loop below
	// Saving file

// -> CODE#0024
// -> DESC="wav export update"
//		CDoWaveConvert dwcdlg(&m_SndFile, s, &wsdlg.WaveFormat.Format, wsdlg.m_bNormalize, pMainFrm);
//		dwcdlg.m_dwFileLimit = wsdlg.m_dwFileLimit;
//		dwcdlg.m_dwSongLimit = wsdlg.m_dwSongLimit;
//		dwcdlg.m_nMaxPatterns = (wsdlg.m_bSelectPlay) ? wsdlg.m_nMaxOrder - wsdlg.m_nMinOrder + 1 : 0;
//		if (wsdlg.m_bHighQuality)
//		{
//			CSoundFile::SetResamplingMode(SRCMODE_POLYPHASE);
//		}
//		dwcdlg.DoModal();

	for(UINT i = 0 ; i < n ; i++){

		// Channel mode
		if(wsdlg.m_bChannelMode){
			// Add channel number & name (if available) to path string
			if(m_SndFile.ChnSettings[i].szName[0] > 0x20)
				wsprintf(channel, "-%03d_%s.wav", i+1,m_SndFile.ChnSettings[i].szName);
			else
				wsprintf(channel, "-%03d.wav", i+1);
			s[p] = '\0';
			strcat(s,channel);
			// Unmute channel to process
			m_SndFile.ChnSettings[i].dwFlags &= ~CHN_MUTE;
		}

		// Render song (or current channel if channel mode and channel not initially disabled)
		if(!wsdlg.m_bChannelMode || !(flags[i] & CHN_MUTE)){
			// rewbs.fix3239
			m_SndFile.SetCurrentPos(0);
			m_SndFile.m_dwSongFlags &= ~SONG_PATTERNLOOP;
			if (wsdlg.m_bSelectPlay) {
				m_SndFile.SetCurrentOrder(wsdlg.m_nMinOrder);
				m_SndFile.m_nCurrentPattern = wsdlg.m_nMinOrder;
				m_SndFile.GetLength(TRUE, FALSE);
				m_SndFile.m_nMaxOrderPosition = wsdlg.m_nMaxOrder + 1;
			}
			//end rewbs.fix3239
			if( dwcdlg.DoModal() != IDOK ) break;	// UPDATE#03
		}

		// Re-mute processed channel
		if(wsdlg.m_bChannelMode) m_SndFile.ChnSettings[i].dwFlags |= CHN_MUTE;
	}

	// Restore channels' flags
	if(wsdlg.m_bChannelMode){
		for(UINT i = 0 ; i < n ; i++) m_SndFile.ChnSettings[i].dwFlags = flags[i];
	}
// -! NEW_FEATURE#0024

	m_SndFile.SetCurrentPos(pos);
	m_SndFile.GetLength(TRUE);
	CMainFrame::UpdateAudioParameters(TRUE);
}


void CModDoc::OnFileMP3Convert()
//------------------------------
{
	CHAR path[_MAX_PATH]="", drive[_MAX_DRIVE]="";
	CHAR s[_MAX_PATH], fname[_MAX_FNAME]="", fext[_MAX_EXT] = "";
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	if ((!pMainFrm) || (!m_SndFile.GetType())) return;
	_splitpath(GetPathName(), drive, path, fname, NULL);
	strcpy(s, drive);
	strcat(s, path);
	strcat(s, fname);
	CFileDialog dlg(FALSE, "mp3", s,
					OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
					"MPEG Layer III Files (*.mp3)|*.mp3|Layer3 Wave Files (*.wav)|*.wav||", pMainFrm);
	dlg.m_ofn.nFilterIndex = 0;
	dlg.m_ofn.lpstrInitialDir = pMainFrm->GetExportDir();
	if (dlg.DoModal() == IDOK)
	{
		MPEGLAYER3WAVEFORMAT wfx;
		HACMDRIVERID hadid;

		s[0] = 0;
		_splitpath(dlg.GetPathName(), s, path, NULL, NULL);
		strcat(s, path);
		pMainFrm->SetExportDir(s);
		strcpy(s, dlg.GetPathName());
		_splitpath(s, NULL, NULL, NULL, fext);
		if (strlen(fext) <= 1)
		{
			int l = strlen(s) - 1;
			if ((l >= 0) && (s[l] == '.')) s[l] = 0;
			strcpy(fext, (dlg.m_ofn.nFilterIndex == 2) ? ".wav" : ".mp3");
			strcat(s, fext);
		}
		CLayer3Convert wsdlg(&m_SndFile, pMainFrm);
		if (m_SndFile.m_szNames[0][0]) wsdlg.m_bSaveInfoField = TRUE;
		if (wsdlg.DoModal() != IDOK) return;
		wsdlg.GetFormat(&wfx, &hadid);
		// Saving as mpeg file
		BOOL bplaying = FALSE;
		UINT pos = m_SndFile.GetCurrentPos();
		bplaying = TRUE;
		pMainFrm->PauseMod();
		m_SndFile.SetCurrentPos(0);

		m_SndFile.m_dwSongFlags &= ~SONG_PATTERNLOOP;

		// Saving file
		PTAGID3INFO pTag = (wsdlg.m_bSaveInfoField) ? &wsdlg.m_id3tag : NULL;
		CDoAcmConvert dwcdlg(&m_SndFile, s, &wfx.wfx, hadid, pTag, pMainFrm);
		dwcdlg.m_dwFileLimit = wsdlg.m_dwFileLimit;
		dwcdlg.m_dwSongLimit = wsdlg.m_dwSongLimit;
		dwcdlg.DoModal();
		m_SndFile.SetCurrentPos(pos);
		m_SndFile.GetLength(TRUE);
		CMainFrame::UpdateAudioParameters(TRUE);
	}
}


void CModDoc::OnFileMidiConvert()
//-------------------------------------
{
	CHAR path[_MAX_PATH]="", drive[_MAX_DRIVE]="";
	CHAR s[_MAX_PATH], fname[_MAX_FNAME]="";
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	if ((!pMainFrm) || (!m_SndFile.GetType())) return;
	_splitpath(GetPathName(), drive, path, fname, NULL);
	strcpy(s, drive);
	strcat(s, path);
	strcat(s, fname);
	strcat(s, ".mid");
	CFileDialog dlg(FALSE, "mid", s,
					OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
					"Midi Files (*.mid,*.rmi)|*.mid;*.rmi||", pMainFrm);
	dlg.m_ofn.nFilterIndex = 0;
	if (dlg.DoModal() == IDOK)
	{
		CModToMidi mididlg(dlg.GetPathName(), &m_SndFile, pMainFrm);
		if (mididlg.DoModal() == IDOK)
		{
			BeginWaitCursor();
			mididlg.DoConvert();
			EndWaitCursor();
		}
	}
}

//HACK: This is a quick fix. Needs to be better integrated into player and GUI.
void CModDoc::OnFileCompatibilitySave()
//-------------------------------------
{
	CHAR path[_MAX_PATH]="", drive[_MAX_DRIVE]="";
	CHAR s[_MAX_PATH], fname[_MAX_FNAME]="";
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CString ext, pattern;
	
	UINT type = m_SndFile.GetType();

	if ((!pMainFrm) || (!m_SndFile.GetType())) return;
	switch (type)
	{
		/*case MOD_TYPE_XM:
			ext = "xm";
			pattern = "Fast Tracker Files (*.xm)|*.xm||";
			break;*/
		case MOD_TYPE_MOD:
			ext = MOD_STD_SPECS.fileExtension;
			pattern = FileFilterMOD;
			if( AfxMessageBox(GetStrI18N(TEXT(
				"Compared to regular MOD save, compatibility export makes "
				"small adjustments to the save file in order to make the file compatible with "
				"ProTracker. Note that this feature is not complete and the "
				"file is not guaranteed to be free of MPT-specific features.\n\n "
				"Important: beginning of some samples may be adjusted in the process. Proceed?")), MB_ICONINFORMATION|MB_YESNO) != IDYES
				)
				return;
			break;
		case MOD_TYPE_IT:
			ext = IT_STD_SPECS.fileExtension;
			pattern = FileFilterIT;
			::MessageBox(NULL,"Warning: the exported file will not contain any of MPT's file-format hacks.", "Compatibility export warning.",MB_ICONINFORMATION | MB_OK);
			break;
		default:
			::MessageBox(NULL,"Compatibility export is currently only available for MOD and IT modules.", "Can't do compatibility export.",MB_ICONINFORMATION | MB_OK);
			return;
	}

	_splitpath(GetPathName(), drive, path, fname, NULL);
	strcpy(s, drive);
	strcat(s, path);
	strcat(s, fname);
	if (!strstr(fname, "compat")) {
		strcat(s, ".compat.");
	} else {
		strcat(s, ".");
	}
	strcat(s, ext);
	CFileDialog dlg(FALSE, ext, s,
					OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
					pattern, pMainFrm);
	if (dlg.DoModal() != IDOK){ 
		return;
	}
	switch (type)
	{
		case MOD_TYPE_MOD:
			m_SndFile.SaveMod(dlg.GetPathName(), 0, true);
			SetModified(); // Compatibility save may adjust samples so set modified...
			m_ShowSavedialog = true;	// ...and force save dialog to appear when saving.
			break;
		case MOD_TYPE_XM:
			m_SndFile.SaveCompatXM(dlg.GetPathName());
			break;
		case MOD_TYPE_IT:
			m_SndFile.SaveCompatIT(dlg.GetPathName());
			break;
	}
}


void CModDoc::OnPlayerPlay()
//--------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm)
	{
		CChildFrame *pChildFrm = (CChildFrame *) GetChildFrame();
 		if (strcmp("CViewPattern", pChildFrm->GetCurrentViewClassName()) == 0)
		{
			//User has sent play song command: set loop pattern checkbox to false.
			pChildFrm->SendViewMessage(VIEWMSG_PATTERNLOOP, 0);
		}
                 
		BOOL bPlaying = (pMainFrm->GetModPlaying() == this) ? TRUE : FALSE;
		if ((bPlaying) && (!(m_SndFile.m_dwSongFlags & (SONG_PAUSED|SONG_STEP/*|SONG_PATTERNLOOP*/))))
		{
			OnPlayerPause();
			return;
		}
		BEGIN_CRITICAL();
		for (UINT i=m_SndFile.m_nChannels; i<MAX_CHANNELS; i++) if (!m_SndFile.Chn[i].nMasterChn)
		{
			m_SndFile.Chn[i].dwFlags |= (CHN_NOTEFADE|CHN_KEYOFF);
			if (!bPlaying) m_SndFile.Chn[i].nLength = 0;
		}
		if (bPlaying) {
			m_SndFile.StopAllVsti();
		} else {
			m_SndFile.ResumePlugins();
		}
		END_CRITICAL();
		//m_SndFile.m_dwSongFlags &= ~(SONG_STEP|SONG_PAUSED);
		m_SndFile.m_dwSongFlags &= ~(SONG_STEP|SONG_PAUSED|SONG_PATTERNLOOP);
		pMainFrm->PlayMod(this, m_hWndFollow, m_dwNotifyType);
	}
}


void CModDoc::OnPlayerPause()
//---------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm)
	{
		if (pMainFrm->GetModPlaying() == this)
		{
			BOOL bLoop = (m_SndFile.m_dwSongFlags & SONG_PATTERNLOOP) ? TRUE : FALSE;
			UINT nPat = m_SndFile.m_nPattern;
			UINT nRow = m_SndFile.m_nRow;
			UINT nNextRow = m_SndFile.m_nNextRow;
			pMainFrm->PauseMod();
			BEGIN_CRITICAL();
			if ((bLoop) && (nPat < m_SndFile.Patterns.Size()))
			{
				if ((m_SndFile.m_nCurrentPattern < m_SndFile.Order.size()) && (m_SndFile.Order[m_SndFile.m_nCurrentPattern] == nPat))
				{
					m_SndFile.m_nNextPattern = m_SndFile.m_nCurrentPattern;
					m_SndFile.m_nNextRow = nNextRow;
					m_SndFile.m_nRow = nRow;
				} else
				{
					for (UINT i=0; i<m_SndFile.Order.size(); i++)
					{
						if (m_SndFile.Order[i] == m_SndFile.Order.GetInvalidPatIndex()) break;
						if (m_SndFile.Order[i] == nPat)
						{
							m_SndFile.m_nCurrentPattern = i;
							m_SndFile.m_nNextPattern = i;
							m_SndFile.m_nNextRow = nNextRow;
							m_SndFile.m_nRow = nRow;
							break;
						}
					}
				}
			}
			END_CRITICAL();
		} else
		{
			pMainFrm->PauseMod();
		}
	}
}


void CModDoc::OnPlayerStop()
//--------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm) pMainFrm->StopMod();
}


void CModDoc::OnPlayerPlayFromStart()
//-----------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm)
	{
		CChildFrame *pChildFrm = (CChildFrame *) GetChildFrame();
		if (strcmp("CViewPattern", pChildFrm->GetCurrentViewClassName()) == 0)
		{
			//User has sent play song command: set loop pattern checkbox to false.
			pChildFrm->SendViewMessage(VIEWMSG_PATTERNLOOP, 0);
		}
		

		pMainFrm->PauseMod();
		//m_SndFile.m_dwSongFlags &= ~SONG_STEP;
		m_SndFile.m_dwSongFlags &= ~(SONG_STEP|SONG_PATTERNLOOP);
		m_SndFile.SetCurrentPos(0);
		pMainFrm->ResetElapsedTime();
		BEGIN_CRITICAL();
		m_SndFile.ResumePlugins();
		END_CRITICAL();
		pMainFrm->PlayMod(this, m_hWndFollow, m_dwNotifyType);
		CMainFrame::EnableLowLatencyMode(FALSE);
	}
}
/*
void CModDoc::OnPlayerPlayFromPos(UINT pos)
//-----------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm)
	{
		pMainFrm->PauseMod();
		m_SndFile.m_dwSongFlags &= ~SONG_STEP;
		m_SndFile.SetCurrentPos(pos);
		pMainFrm->ResetElapsedTime();
		pMainFrm->PlayMod(this, m_hWndFollow, m_dwNotifyType);
		CMainFrame::EnableLowLatencyMode(FALSE);
	}
}
*/

void CModDoc::OnEditGlobals()
//---------------------------
{
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW,	IDD_CONTROL_GLOBALS);
}


void CModDoc::OnEditPatterns()
//----------------------------
{
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_PATTERNS, -1);
}


void CModDoc::OnEditSamples()
//---------------------------
{
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_SAMPLES);
}


void CModDoc::OnEditInstruments()
//-------------------------------
{
	//if (m_SndFile.m_nInstruments) rewbs.cosmetic: allow keyboard access to instruments even with no instruments
	SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_INSTRUMENTS);
}


void CModDoc::OnEditComments()
//----------------------------
{
	if (m_SndFile.m_nType & (MOD_TYPE_XM|MOD_TYPE_IT | MOD_TYPE_MPT)) SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_COMMENTS);
}

//rewbs.graph
void CModDoc::OnEditGraph()
//----------------------------
{
	if (m_SndFile.m_nType & (MOD_TYPE_XM|MOD_TYPE_IT | MOD_TYPE_MPT)) SendMessageToActiveViews(WM_MOD_ACTIVATEVIEW, IDD_CONTROL_GRAPH);
}
//end rewbs.graph


void CModDoc::OnCleanupSamples()
//------------------------------
{
	ClearLog();
	RemoveUnusedSamples();
	UpdateAllViews(NULL, HINT_MODTYPE);
	ShowLog("Sample Cleanup", CMainFrame::GetMainFrame());
}


void CModDoc::OnCleanupInstruments()
//----------------------------------
{
	ClearLog();
	RemoveUnusedInstruments();
	UpdateAllViews(NULL, HINT_MODTYPE);
	ShowLog("Instrument Cleanup", CMainFrame::GetMainFrame());
}

void CModDoc::OnCleanupPlugs()
//----------------------------------
{
	ClearLog();
	RemoveUnusedPlugs();
	UpdateAllViews(NULL, HINT_MODTYPE);
	ShowLog("Plugin Cleanup", CMainFrame::GetMainFrame());
}



void CModDoc::OnCleanupPatterns()
//-------------------------------
{
	ClearLog();
	RemoveUnusedPatterns();
	UpdateAllViews(NULL, HINT_MODTYPE|HINT_MODSEQUENCE);
	ShowLog("Pattern Cleanup", CMainFrame::GetMainFrame());
}


void CModDoc::OnCleanupSong()
//---------------------------
{
	ClearLog();
	RemoveUnusedPatterns();
	RemoveUnusedInstruments();
	RemoveUnusedSamples();
	RemoveUnusedPlugs();
	UpdateAllViews(NULL, HINT_MODTYPE|HINT_MODSEQUENCE);
	ShowLog("Song Cleanup", CMainFrame::GetMainFrame());
}


void CModDoc::OnRearrangePatterns()
//---------------------------------
{
	ClearLog();
	RemoveUnusedPatterns(FALSE);
	UpdateAllViews(NULL, HINT_MODTYPE|HINT_MODSEQUENCE);
	ShowLog("Pattern Rearrange", CMainFrame::GetMainFrame());
}


void CModDoc::OnUpdateInstrumentOnly(CCmdUI *p)
//---------------------------------------------
{
	if (p) p->Enable((m_SndFile.m_nInstruments) ? TRUE : FALSE);
}

void CModDoc::OnUpdateHasMIDIMappings(CCmdUI *p)
//----------------------------------------------
{
	if(!p) return;
	if(m_SndFile.GetModSpecifications().MIDIMappingDirectivesMax > 0)
		p->Enable();
	else
		p->Enable(FALSE);
}


void CModDoc::OnUpdateXMITMPTOnly(CCmdUI *p)
//---------------------------------------
{
	if (p)
		p->Enable((m_SndFile.GetType() & (MOD_TYPE_XM|MOD_TYPE_IT|MOD_TYPE_MPT)) ? TRUE : FALSE);
}


void CModDoc::OnUpdateMP3Encode(CCmdUI *p)
//----------------------------------------
{
	if (p) p->Enable(theApp.CanEncodeLayer3());
}


void CModDoc::OnInsertPattern()
//-----------------------------
{
	LONG pat = InsertPattern();
	if (pat >= 0)
	{
		UINT ord = 0;
		for (UINT i=0; i<m_SndFile.Order.size(); i++)
		{
			if (m_SndFile.Order[i] == pat) ord = i;
			if (m_SndFile.Order[i] == m_SndFile.Order.GetInvalidPatIndex()) break;
		}
		ViewPattern(pat, ord);
	}
}


void CModDoc::OnInsertSample()
//----------------------------
{
	LONG smp = InsertSample();
	if (smp >= 0) ViewSample(smp);
}


void CModDoc::OnInsertInstrument()
//--------------------------------
{
	LONG ins = InsertInstrument();
	if (ins >= 0) ViewInstrument(ins);
}


void CModDoc::OnRemoveAllInstruments()
//------------------------------------
{
	if (!m_SndFile.m_nInstruments) return;
	if (CMainFrame::GetMainFrame()->MessageBox("This will remove all the instruments in the song,\n"
		"Do you want to continue?", "Warning", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	if (CMainFrame::GetMainFrame()->MessageBox("Do you want to convert all instruments to samples ?\n",
		NULL, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		ConvertInstrumentsToSamples();
	}
	char removeSamples = -1;
	if (::MessageBox(NULL, "Remove samples associated with an instrument if they are unused?", "Removing instrument", MB_YESNO | MB_ICONQUESTION) == IDYES) {
		removeSamples = 1;
	}

	for (UINT i=1; i<=m_SndFile.m_nInstruments; i++) {
		m_SndFile.DestroyInstrument(i,removeSamples);
	}
	m_SndFile.m_nInstruments = 0;
	SetModified();
	UpdateAllViews(NULL, HINT_MODTYPE);
}


void CModDoc::OnEstimateSongLength()
//----------------------------------
{
	CHAR s[256];
	DWORD dwSongLength = m_SndFile.GetSongTime();
	wsprintf(s, "Approximate song length: %dmn%02ds", dwSongLength/60, dwSongLength%60);
	CMainFrame::GetMainFrame()->MessageBox(s, NULL, MB_OK|MB_ICONINFORMATION);
}

void CModDoc::OnApproximateBPM()
//----------------------------------
{
	//Convert BPM to string:
	CString Message;
	double bpm = CMainFrame::GetMainFrame()->GetApproxBPM();


	switch(m_SndFile.m_nTempoMode) {
		case tempo_mode_alternative: 
			Message.Format("Using alternative tempo interpretation.\n\nAssuming:\n. %d ticks per second\n. %d ticks per row\n. %d rows per beat\nthe tempo is approximately: %.20g BPM",
			m_SndFile.m_nMusicTempo, m_SndFile.m_nMusicSpeed, m_SndFile.m_nRowsPerBeat, bpm); 
			break;

		case tempo_mode_modern: 
			Message.Format("Using modern tempo interpretation.\n\nThe tempo is: %.20g BPM", bpm); 
			break;

		case tempo_mode_classic: 
		default:
			Message.Format("Using standard tempo interpretation.\n\nAssuming:\n. A mod tempo (tick duration factor) of %d\n. %d ticks per row\n. %d rows per beat\nthe tempo is approximately: %.20g BPM",
			m_SndFile.m_nMusicTempo, m_SndFile.m_nMusicSpeed, m_SndFile.m_nRowsPerBeat, bpm); 
			break;
	}
/*
	if (CMainFrame::m_dwPatternSetup & PATTERN_MODERNSPEED) {
		Message.Format("Using modern speed interpretation.\n\nAssuming:\n. %d ticks per second\n. %d ticks per row\n. %d rows per beat\nthe tempo is approximately: %.20g BPM",
		m_SndFile.m_nMusicTempo, m_SndFile.m_nMusicSpeed, CMainFrame::m_nRowSpacing2, bpm); 
	} 
	else if (CMainFrame::m_dwPatternSetup & PATTERN_ALTERNTIVEBPMSPEED) {
		Message.Format("Using alternative tempo interpretation.\n\nAssuming:\n. %d ticks per second\n. %d ticks per row\n. %d rows per beat\nthe tempo is approximately: %.20g BPM",
		m_SndFile.m_nMusicTempo, m_SndFile.m_nMusicSpeed, CMainFrame::m_nRowSpacing2, bpm); 
	} else {
		Message.Format("Using standard tempo interpretation.\n\nAssuming:\n. A mod tempo (tick duration factor) of %d\n. %d ticks per row\n. %d rows per beat\nthe tempo is approximately: %.20g BPM",
		m_SndFile.m_nMusicTempo, m_SndFile.m_nMusicSpeed, CMainFrame::m_nRowSpacing2, bpm); 
	}
	*/
	CMainFrame::GetMainFrame()->MessageBox(Message, NULL, MB_OK|MB_ICONINFORMATION);
}



///////////////////////////////////////////////////////////////////////////
// Effects description

typedef struct MPTEFFECTINFO
{
	DWORD dwEffect;		// CMD_XXXX
	DWORD dwParamMask;	// 0 = default
	DWORD dwParamValue;	// 0 = default
	DWORD dwFlags;		// FXINFO_XXXX
	DWORD dwFormats;	// MOD_TYPE_XXX combo
	LPCSTR pszName;		// ie "Tone Portamento"
} MPTEFFECTINFO;

#define MOD_TYPE_MODXM	(MOD_TYPE_MOD|MOD_TYPE_XM)
#define MOD_TYPE_S3MIT	(MOD_TYPE_S3M|MOD_TYPE_IT)
#define MOD_TYPE_S3MITMPT (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT)
#define MOD_TYPE_NOMOD	(MOD_TYPE_S3M|MOD_TYPE_XM|MOD_TYPE_IT|MOD_TYPE_MPT)
#define MOD_TYPE_XMIT	(MOD_TYPE_XM|MOD_TYPE_IT)
#define MOD_TYPE_XMITMPT (MOD_TYPE_XM|MOD_TYPE_IT|MOD_TYPE_MPT)
#define MOD_TYPE_ITMPT (MOD_TYPE_IT|MOD_TYPE_MPT)
#define MAX_FXINFO		66					//rewbs.smoothVST, increased from 64... I wonder what this will break?


const MPTEFFECTINFO gFXInfo[MAX_FXINFO] =
{
	{CMD_ARPEGGIO,		0,0,		0,	0xFFFFFFFF,		"Arpeggio"},
	{CMD_PORTAMENTOUP,	0,0,		0,	0xFFFFFFFF,		"Portamento Up"},
	{CMD_PORTAMENTODOWN,0,0,		0,	0xFFFFFFFF,		"Portamento Down"},
	{CMD_TONEPORTAMENTO,0,0,		0,	0xFFFFFFFF,		"Tone portamento"},
	{CMD_VIBRATO,		0,0,		0,	0xFFFFFFFF,		"Vibrato"},
	{CMD_TONEPORTAVOL,	0,0,		0,	0xFFFFFFFF,		"Volslide+Toneporta"},
	{CMD_VIBRATOVOL,	0,0,		0,	0xFFFFFFFF,		"VolSlide+Vibrato"},
	{CMD_TREMOLO,		0,0,		0,	0xFFFFFFFF,		"Tremolo"},
	{CMD_PANNING8,		0,0,		0,	0xFFFFFFFF,		"Set Panning"},
	{CMD_OFFSET,		0,0,		0,	0xFFFFFFFF,		"Set Offset"},
	{CMD_VOLUMESLIDE,	0,0,		0,	0xFFFFFFFF,		"Volume Slide"},
	{CMD_POSITIONJUMP,	0,0,		0,	0xFFFFFFFF,		"Position Jump"},
	{CMD_VOLUME,		0,0,		0,	MOD_TYPE_MODXM,	"Set Volume"},
	{CMD_PATTERNBREAK,	0,0,		0,	0xFFFFFFFF,		"Pattern Break"},
	{CMD_RETRIG,		0,0,		0,	MOD_TYPE_NOMOD,	"Retrigger Note"},
	{CMD_SPEED,			0,0,		0,	0xFFFFFFFF,		"Set Speed"},
	{CMD_TEMPO,			0,0,		0,	0xFFFFFFFF,		"Set Tempo"},
	{CMD_TREMOR,		0,0,		0,	MOD_TYPE_NOMOD,	"Tremor"},
	{CMD_CHANNELVOLUME,	0,0,		0,	MOD_TYPE_S3MITMPT,	"Set channel volume"},
	{CMD_CHANNELVOLSLIDE,0,0,		0,	MOD_TYPE_S3MITMPT,	"Channel volslide"},
	{CMD_GLOBALVOLUME,	0,0,		0,	MOD_TYPE_NOMOD,	"Set global volume"},
	{CMD_GLOBALVOLSLIDE,0,0,		0,	MOD_TYPE_NOMOD,	"Global volume slide"},
	{CMD_KEYOFF,		0,0,		0,	MOD_TYPE_XM,	"Key off"},
	{CMD_FINEVIBRATO,	0,0,		0,	MOD_TYPE_S3MITMPT,	"Fine vibrato"},
	{CMD_PANBRELLO,		0,0,		0,	MOD_TYPE_NOMOD,	"Panbrello"},
	{CMD_PANNINGSLIDE,	0,0,		0,	MOD_TYPE_NOMOD,	"Panning slide"},
	{CMD_SETENVPOSITION,0,0,		0,	MOD_TYPE_XM,	"Envelope position"},
	{CMD_MIDI,			0,0,		0x7F,	MOD_TYPE_NOMOD,	"Midi macro"},
	{CMD_SMOOTHMIDI,	0,0,		0x7F,	MOD_TYPE_NOMOD,	"Smooth Midi macro"},	//rewbs.smoothVST
	// Extended MOD/XM effects
	{CMD_MODCMDEX,		0xF0,0x10,	0,	MOD_TYPE_MODXM,	"Fine porta up"},
	{CMD_MODCMDEX,		0xF0,0x20,	0,	MOD_TYPE_MODXM,	"Fine porta down"},
	{CMD_MODCMDEX,		0xF0,0x30,	0,	MOD_TYPE_MODXM,	"Glissando Control"},
	{CMD_MODCMDEX,		0xF0,0x40,	0,	MOD_TYPE_MODXM,	"Vibrato waveform"},
	{CMD_MODCMDEX,		0xF0,0x50,	0,	MOD_TYPE_MOD,	"Set finetune"},
	{CMD_MODCMDEX,		0xF0,0x60,	0,	MOD_TYPE_MODXM,	"Pattern loop"},
	{CMD_MODCMDEX,		0xF0,0x70,	0,	MOD_TYPE_MODXM,	"Tremolo waveform"},
	{CMD_MODCMDEX,		0xF0,0x80,	0,	MOD_TYPE_MODXM,	"Set panning"},
	{CMD_MODCMDEX,		0xF0,0x90,	0,	MOD_TYPE_MODXM,	"Retrigger note"},
	{CMD_MODCMDEX,		0xF0,0xA0,	0,	MOD_TYPE_MODXM,	"Fine volslide up"},
	{CMD_MODCMDEX,		0xF0,0xB0,	0,	MOD_TYPE_MODXM,	"Fine volslide down"},
	{CMD_MODCMDEX,		0xF0,0xC0,	0,	MOD_TYPE_MODXM,	"Note cut"},
	{CMD_MODCMDEX,		0xF0,0xD0,	0,	MOD_TYPE_MODXM,	"Note delay"},
	{CMD_MODCMDEX,		0xF0,0xE0,	0,	MOD_TYPE_MODXM,	"Pattern delay"},
	{CMD_MODCMDEX,		0xF0,0xF0,	0,	MOD_TYPE_XM,	"Set active macro"},
	// Extended S3M/IT effects
	{CMD_S3MCMDEX,		0xF0,0x10,	0,	MOD_TYPE_S3MITMPT,	"Glissando control"},
	{CMD_S3MCMDEX,		0xF0,0x20,	0,	MOD_TYPE_S3M,	"Set finetune"},
	{CMD_S3MCMDEX,		0xF0,0x30,	0,	MOD_TYPE_S3MITMPT,	"Vibrato waveform"},
	{CMD_S3MCMDEX,		0xF0,0x40,	0,	MOD_TYPE_S3MITMPT,	"Tremolo waveform"},
	{CMD_S3MCMDEX,		0xF0,0x50,	0,	MOD_TYPE_S3MITMPT,	"Panbrello waveform"},
	{CMD_S3MCMDEX,		0xF0,0x60,	0,	MOD_TYPE_S3MITMPT,	"Fine pattern delay"},
	{CMD_S3MCMDEX,		0xF0,0x80,	0,	MOD_TYPE_S3MITMPT,	"Set panning"},
	{CMD_S3MCMDEX,		0xF0,0xA0,	0,	MOD_TYPE_S3MITMPT,	"Set high offset"},
	{CMD_S3MCMDEX,		0xF0,0xB0,	0,	MOD_TYPE_S3MITMPT,	"Pattern loop"},
	{CMD_S3MCMDEX,		0xF0,0xC0,	0,	MOD_TYPE_S3MITMPT,	"Note cut"},
	{CMD_S3MCMDEX,		0xF0,0xD0,	0,	MOD_TYPE_S3MITMPT,	"Note delay"},
	{CMD_S3MCMDEX,		0xF0,0xE0,	0,	MOD_TYPE_S3MITMPT,	"Pattern delay"},
	{CMD_S3MCMDEX,		0xF0,0xF0,	0,	MOD_TYPE_ITMPT,	"Set active macro"},
	// MPT XM extensions and special effects
	{CMD_XFINEPORTAUPDOWN,0xF0,0x10,0,	MOD_TYPE_XM,	"Extra fine porta up"},
	{CMD_XFINEPORTAUPDOWN,0xF0,0x20,0,	MOD_TYPE_XM,	"Extra fine porta down"},
	{CMD_XFINEPORTAUPDOWN,0xF0,0x50,0,	MOD_TYPE_XM,	"Panbrello waveform"},
	{CMD_XFINEPORTAUPDOWN,0xF0,0x60,0,	MOD_TYPE_XM,	"Fine pattern delay"},
	{CMD_XFINEPORTAUPDOWN,0xF0,0x90,0,	MOD_TYPE_XM,	"Sound control"},
	{CMD_XFINEPORTAUPDOWN,0xF0,0xA0,0,	MOD_TYPE_XM,	"Set high offset"},
	// MPT IT extensions and special effects
	{CMD_S3MCMDEX,		0xF0,0x90,	0,	MOD_TYPE_S3MITMPT,	"Sound control"},
	{CMD_S3MCMDEX,		0xF0,0x70,	0,	MOD_TYPE_ITMPT,	"Instr. control"},
// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
	{CMD_XPARAM,		0x00,0x00,	0,	MOD_TYPE_XMITMPT,	"X param"}
// -! NEW_FEATURE#0010
};


UINT CModDoc::GetNumEffects() const
//---------------------------------
{
	return MAX_FXINFO;
}


BOOL CModDoc::IsExtendedEffect(UINT ndx) const
//--------------------------------------------
{
	return ((ndx < MAX_FXINFO) && (gFXInfo[ndx].dwParamMask));
}


BOOL CModDoc::GetEffectName(LPSTR pszDescription, UINT command, UINT param, BOOL bXX, int nChn) //rewbs.xinfo: added chan arg
//---------------------------------------------------------------------------------------------
{
	BOOL bSupported;
	int fxndx = -1;
	pszDescription[0] = 0;
	UINT i = 0;
	for (i=0; i<MAX_FXINFO; i++)
	{
		if ((command == gFXInfo[i].dwEffect) // Effect
		 && ((param & gFXInfo[i].dwParamMask) == gFXInfo[i].dwParamValue)) // Value
		{
			fxndx = i;
			break;
		}
	}
	if (fxndx < 0) return FALSE;
	bSupported = ((m_SndFile.m_nType & gFXInfo[fxndx].dwFormats) != 0);
	if (gFXInfo[fxndx].pszName)
	{
		if ((bXX) && (bSupported))
		{
			strcpy(pszDescription, " xx: ");
			LPCSTR pszCmd = (m_SndFile.m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM)) ? gszModCommands : gszS3mCommands;
			pszDescription[0] = pszCmd[command];
			if ((gFXInfo[fxndx].dwParamMask & 0xF0) == 0xF0) pszDescription[1] = szHexChar[gFXInfo[i].dwParamValue >> 4];
			if ((gFXInfo[fxndx].dwParamMask & 0x0F) == 0x0F) pszDescription[2] = szHexChar[gFXInfo[i].dwParamValue & 0x0F];
		}
		strcat(pszDescription, gFXInfo[fxndx].pszName);
		//rewbs.xinfo
		//Get channel specific info
		if (nChn>=0 && nChn<m_SndFile.m_nChannels)
		{
			CString chanSpec = "";
			CString macroText= "_no macro_";
			switch (command)
			{
			case CMD_MODCMDEX:
			case CMD_S3MCMDEX:
				if ((param&0xF0) == 0xF0)	//Set Macro
				{
					macroText = &m_SndFile.m_MidiCfg.szMidiSFXExt[(param&0x0F)*32];
					chanSpec.Format(" to %d: ", param&0x0F);
				}
				break;
			case CMD_MIDI:
			case CMD_SMOOTHMIDI:
				if (param<0x80)
				{
					macroText = &m_SndFile.m_MidiCfg.szMidiSFXExt[m_SndFile.Chn[nChn].nActiveMacro*32];
					chanSpec.Format(": currently %d: ", m_SndFile.Chn[nChn].nActiveMacro);
				}
				else
				{
					chanSpec = " (Fixed)";
				}
				break;
			}
			
			if (macroText != "_no macro_")
			{
				switch (GetMacroType(macroText))
				{
				case sfx_unused: chanSpec.Append("Unused"); break;
				case sfx_cutoff: chanSpec.Append("Cutoff"); break;
				case sfx_reso: chanSpec.Append("Resonance"); break;
				case sfx_mode: chanSpec.Append("Filter Mode"); break;
				case sfx_drywet: chanSpec.Append("Plug wet/dry ratio"); break;
				case sfx_cc: {
					int nCC = MacroToMidiCC(macroText);
					CString temp; 
					temp.Format("MidiCC %d", nCC);
					chanSpec.Append(temp); 
					break;
				}
				case sfx_plug: {
					int nParam = MacroToPlugParam(macroText);
					char paramName[128];
					memset(paramName, 0, sizeof(paramName));				
					UINT nPlug = m_SndFile.GetBestPlugin(nChn, PRIORITISE_CHANNEL, EVEN_IF_MUTED);
					if ((nPlug) && (nPlug<=MAX_MIXPLUGINS)) {
						CVstPlugin *pPlug = (CVstPlugin*)m_SndFile.m_MixPlugins[nPlug-1].pMixPlugin;
						if (pPlug) 
							pPlug->GetParamName(nParam, paramName, sizeof(paramName));
						if (paramName[0] == 0)
							strcpy(paramName, "N/A");
					}
					if (paramName[0] == 0)
						strcpy(paramName, "N/A - no plug");
					CString temp; 
					temp.Format("param %d (%s)", nParam, paramName);
					chanSpec.Append(temp); 
					break; }
				case sfx_custom: 
				default: chanSpec.Append("Custom");
				}
			}
			if (chanSpec != "") {
				strcat(pszDescription, chanSpec);
			}

		}
		//end rewbs.xinfo
	}
	return bSupported;
}


LONG CModDoc::GetIndexFromEffect(UINT command, UINT param)
//--------------------------------------------------------
{
	for (UINT i=0; i<MAX_FXINFO; i++)
	{
		if ((command == gFXInfo[i].dwEffect) // Effect
		 && ((param & gFXInfo[i].dwParamMask) == gFXInfo[i].dwParamValue)) // Value
		{
			return i;
		}
	}
	return -1;
}


//Returns command and corrects parameter refParam if necessary
UINT CModDoc::GetEffectFromIndex(UINT ndx, int &refParam)
//-------------------------------------------------------
{
	//if (pParam) *pParam = -1;
	if (ndx >= MAX_FXINFO) {
		refParam = 0;
		return 0;
	}

	//Cap parameter to match FX if necessary.
	if (gFXInfo[ndx].dwParamMask) {
		if (refParam < static_cast<int>(gFXInfo[ndx].dwParamValue)) {
			refParam = gFXInfo[ndx].dwParamValue;	 // for example: delay with param < D0 becomes SD0
		} else if (refParam > static_cast<int>(gFXInfo[ndx].dwParamValue)+15) {
			refParam = gFXInfo[ndx].dwParamValue+15; // for example: delay with param > DF becomes SDF
		}
	}
	if (gFXInfo[ndx].dwFlags) {
		if (refParam > gFXInfo[ndx].dwFlags) {
			refParam = gFXInfo[ndx].dwFlags;	//used for Zxx macro control: limit to 7F max.
		}
	}

	return gFXInfo[ndx].dwEffect;
}

UINT CModDoc::GetEffectMaskFromIndex(UINT ndx)
//-------------------------------------------------------
{
	if (ndx >= MAX_FXINFO) {
		return 0;
	}

	return gFXInfo[ndx].dwParamValue;

}

BOOL CModDoc::GetEffectInfo(UINT ndx, LPSTR s, BOOL bXX, DWORD *prangeMin, DWORD *prangeMax)
//------------------------------------------------------------------------------------------
{
	if (s) s[0] = 0;
	if (prangeMin) *prangeMin = 0;
	if (prangeMax) *prangeMax = 0;
	if ((ndx >= MAX_FXINFO) || (!(m_SndFile.m_nType & gFXInfo[ndx].dwFormats))) return FALSE;
	if (s) GetEffectName(s, gFXInfo[ndx].dwEffect, gFXInfo[ndx].dwParamValue, bXX);
	if ((prangeMin) && (prangeMax))
	{
		UINT nmin = 0, nmax = 0xFF, nType = m_SndFile.m_nType;
		if (gFXInfo[ndx].dwParamMask == 0xF0)
		{
			nmin = gFXInfo[ndx].dwParamValue;
			nmax = nmin | 0x0F;
		}
		switch(gFXInfo[ndx].dwEffect)
		{
		case CMD_ARPEGGIO:
			if (nType & MOD_TYPE_MOD) nmin = 1;
			break;
		case CMD_VOLUME:
			nmax = 0x40;
			break;
		case CMD_SPEED:
			nmin = 1;
			nmax = 0x7F;
			if (nType & MOD_TYPE_MOD) nmax = 0x20; else
			if (nType & MOD_TYPE_XM) nmax = 0x1F;
			break;
		case CMD_TEMPO:
			nmin = 0x20;
			if (nType & MOD_TYPE_MOD) nmin = 0x21; else
// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
//			if (nType & MOD_TYPE_S3MIT) nmin = 1;
			if (nType & MOD_TYPE_S3MITMPT) nmin = 0;
// -! NEW_FEATURE#0010
			break;
		case CMD_VOLUMESLIDE:
		case CMD_TONEPORTAVOL:
		case CMD_VIBRATOVOL:
		case CMD_GLOBALVOLSLIDE:
		case CMD_CHANNELVOLSLIDE:
			nmax = (nType & MOD_TYPE_S3MITMPT) ? 58 : 30;
			break;
		case CMD_PANNING8:
			if (nType & (MOD_TYPE_MOD|MOD_TYPE_S3M)) nmax = 0x80;
			break;
		case CMD_GLOBALVOLUME:
			nmax = (nType & MOD_TYPE_IT | MOD_TYPE_MPT) ? 128 : 64;
			break;
		}
		*prangeMin = nmin;
		*prangeMax = nmax;
	}
	return TRUE;
}


UINT CModDoc::MapValueToPos(UINT ndx, UINT param)
//-----------------------------------------------
{
	UINT pos;
	
	if (ndx >= MAX_FXINFO) return 0;
	pos = param;
	if (gFXInfo[ndx].dwParamMask == 0xF0)
	{
		pos &= 0x0F;
		pos |= gFXInfo[ndx].dwParamValue;
	}
	switch(gFXInfo[ndx].dwEffect)
	{
	case CMD_VOLUMESLIDE:
	case CMD_TONEPORTAVOL:
	case CMD_VIBRATOVOL:
	case CMD_GLOBALVOLSLIDE:
	case CMD_CHANNELVOLSLIDE:
		if (m_SndFile.m_nType & MOD_TYPE_S3MITMPT)
		{
			if (!param) pos = 29; else
			if (((param & 0x0F) == 0x0F) && (param & 0xF0))
				pos = 29 + (param >> 4); else
			if (((param & 0xF0) == 0xF0) && (param & 0x0F))
				pos = 29 - (param & 0x0F); else
			if (param & 0x0F)
				pos = 15 - (param & 0x0F);
			else
				pos = (param >> 4) + 44;
		} else
		{
			if (param & 0x0F) pos = 15 - (param & 0x0F);
			else pos = (param >> 4) + 15;
		}
		break;
	}
	return pos;
}


UINT CModDoc::MapPosToValue(UINT ndx, UINT pos)
//---------------------------------------------
{
	UINT param;
	
	if (ndx >= MAX_FXINFO) return 0;
	param = pos;
	if (gFXInfo[ndx].dwParamMask == 0xF0) param |= gFXInfo[ndx].dwParamValue;
	switch(gFXInfo[ndx].dwEffect)
	{
	case CMD_VOLUMESLIDE:
	case CMD_TONEPORTAVOL:
	case CMD_VIBRATOVOL:
	case CMD_GLOBALVOLSLIDE:
	case CMD_CHANNELVOLSLIDE:
		if (m_SndFile.m_nType & MOD_TYPE_S3MITMPT)
		{
			if (pos < 15) param = 15-pos; else
			if (pos < 29) param = (29-pos) | 0xF0; else
			if (pos == 29) param = 0; else
			if (pos < 44) param = ((pos - 29) << 4) | 0x0F; else
			if (pos < 59) param = (pos-43) << 4;
		} else
		{
			if (pos < 15) param = 15 - pos; else
			param = (pos - 15) << 4;
		}
		break;
	}
	return param;
}


BOOL CModDoc::GetEffectNameEx(LPSTR pszName, UINT ndx, UINT param)
//----------------------------------------------------------------
{
	CHAR s[64];
	if (pszName) pszName[0] = 0;
	if ((!pszName) || (ndx >= MAX_FXINFO) || (!gFXInfo[ndx].pszName)) return FALSE;
	wsprintf(pszName, "%s: ", gFXInfo[ndx].pszName);
	s[0] = 0;
	switch(gFXInfo[ndx].dwEffect)
	{
	case CMD_ARPEGGIO:
		if (param)
			wsprintf(s, "note+%d note+%d", param >> 4, param & 0x0F);
		else
			strcpy(s, "continue");
		break;

	case CMD_PORTAMENTOUP:
		if (param)
			wsprintf(s, "+%d", param);
		else
			strcpy(s, "continue");
		break;

	case CMD_PORTAMENTODOWN:
		if (param)
			wsprintf(s, "-%d", param);
		else
			strcpy(s, "continue");
		break;

	case CMD_TONEPORTAMENTO:
		if (param)
			wsprintf(s, "speed %d", param);
		else
			strcpy(s, "continue");
		break;

	case CMD_VIBRATO:
	case CMD_TREMOLO:
	case CMD_PANBRELLO:
	case CMD_FINEVIBRATO:
		if (param)
			wsprintf(s, "speed=%d depth=%d", param >> 4, param & 0x0F);
		else
			strcpy(s, "continue");
		break;

	case CMD_SPEED:
		wsprintf(s, "%d frames/row", param);
		break;

	case CMD_TEMPO:
		if (param < 0x10)
			wsprintf(s, "-%dbpm (slower)", param & 0x0F);
		else if (param < 0x20)
			wsprintf(s, "+%dbpm (faster)", param & 0x0F);
		else
			wsprintf(s, "%dbpm", param);
		break;


	case CMD_RETRIG:
		switch(param >> 4) {
			case  0: strcpy(s, "continue"); break;
			case  1: strcpy(s, "vol -1"); break;
			case  2: strcpy(s, "vol -2"); break;
			case  3: strcpy(s, "vol -4"); break;
			case  4: strcpy(s, "vol -8"); break;
			case  5: strcpy(s, "vol -16"); break;
			case  6: strcpy(s, "vol *0.66"); break;
			case  7: strcpy(s, "vol *0.5"); break;
			case  8: strcpy(s, "vol *1"); break;
			case  9: strcpy(s, "vol +1"); break;
			case 10: strcpy(s, "vol +2"); break;
			case 11: strcpy(s, "vol +4"); break;
			case 12: strcpy(s, "vol +8"); break;
			case 13: strcpy(s, "vol +16"); break;
			case 14: strcpy(s, "vol *1.5"); break;
			case 15: strcpy(s, "vol *2"); break;
		}
		char spd[10];
		wsprintf(spd, " speed %d", param & 0x0F);
		strcat(s, spd);
		break;

	case CMD_VOLUMESLIDE:
	case CMD_TONEPORTAVOL:
	case CMD_VIBRATOVOL:
	case CMD_GLOBALVOLSLIDE:
	case CMD_CHANNELVOLSLIDE:
		if (!param)
		{
			wsprintf(s, "continue");
		} else
		if ((m_SndFile.m_nType & MOD_TYPE_S3MITMPT) && ((param & 0x0F) == 0x0F) && (param & 0xF0))
		{
			wsprintf(s, "fine +%d", param >> 4);
		} else
		if ((m_SndFile.m_nType & MOD_TYPE_S3MITMPT) && ((param & 0xF0) == 0xF0) && (param & 0x0F))
		{
			wsprintf(s, "fine -%d", param & 0x0F);
		} else
		if (param & 0x0F)
		{
			wsprintf(s, "-%d", param & 0x0F);
		} else
		{
			wsprintf(s, "+%d", param >> 4);
		}
		break;

	case CMD_PATTERNBREAK:
		wsprintf(pszName, "Break to row %d", param);
		break;

	case CMD_POSITIONJUMP:
		wsprintf(pszName, "Jump to position %d", param);
		break;

	case CMD_OFFSET:
		if (param)
// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
//			wsprintf(pszName, "Set Offset to %u", param << 8);
			wsprintf(pszName, "Set Offset to %u", param);
// -! NEW_FEATURE#0010
		else
			strcpy(s, "continue");
		break;

	case CMD_MIDI:
		if (param < 0x80)
		{
			wsprintf(pszName, "SFx macro: z=%02X (%d)", param, param);
		} else
		{
			wsprintf(pszName, "Fixed Macro Z%02X", param);
		}
		break;
	
	//rewbs.smoothVST
	case CMD_SMOOTHMIDI:
		if (param < 0x80)
		{
			wsprintf(pszName, "SFx macro: z=%02X (%d)", param, param);
		} else
		{
			wsprintf(pszName, "Fixed Macro Z%02X", param);
		}
		break;
	//end rewbs.smoothVST

	default:
		if (gFXInfo[ndx].dwParamMask == 0xF0)
		{
			// Sound control names
			if (((gFXInfo[ndx].dwEffect == CMD_XFINEPORTAUPDOWN) || (gFXInfo[ndx].dwEffect == CMD_S3MCMDEX))
			 && ((gFXInfo[ndx].dwParamValue & 0xF0) == 0x90) && ((param & 0xF0) == 0x90))
			{
				switch(param & 0x0F)
				{
				case 0x00:	strcpy(s, "90: Surround Off"); break;
				case 0x01:	strcpy(s, "91: Surround On"); break;
				case 0x08:	strcpy(s, "98: Reverb Off"); break;
				case 0x09:	strcpy(s, "99: Reverb On"); break;
				case 0x0A:	strcpy(s, "9A: Center surround"); break;
				case 0x0B:	strcpy(s, "9B: Quad surround"); break;
				case 0x0C:	strcpy(s, "9C: Global filters"); break;
				case 0x0D:	strcpy(s, "9D: Local filters"); break;
				case 0x0E:	strcpy(s, "9E: Play Forward"); break;
				case 0x0F:	strcpy(s, "9F: Play Backward"); break;
				default:	wsprintf(s, "%02X: undefined", param);
				}
			} else
			if (((gFXInfo[ndx].dwEffect == CMD_XFINEPORTAUPDOWN) || (gFXInfo[ndx].dwEffect == CMD_S3MCMDEX))
			 && ((gFXInfo[ndx].dwParamValue & 0xF0) == 0x70) && ((param & 0xF0) == 0x70))
			{
				switch(param & 0x0F)
				{
				case 0x00:	strcpy(s, "70: Past note cut"); break;
				case 0x01:	strcpy(s, "71: Past note off"); break;
				case 0x02:	strcpy(s, "72: Past note fade"); break;
				case 0x03:	strcpy(s, "73: NNA note cut"); break;
				case 0x04:	strcpy(s, "74: NNA continue"); break;
				case 0x05:	strcpy(s, "75: NNA note off"); break;
				case 0x06:	strcpy(s, "76: NNA note fade"); break;
				case 0x07:	strcpy(s, "77: Volume Env Off"); break;
				case 0x08:	strcpy(s, "78: Volume Env On"); break;
				case 0x09:	strcpy(s, "79: Pan Env Off"); break;
				case 0x0A:	strcpy(s, "7A: Pan Env On"); break;
				case 0x0B:	strcpy(s, "7B: Pitch Env Off"); break;
				case 0x0C:	strcpy(s, "7C: Pitch Env On"); break;
				default:	wsprintf(s, "%02X: undefined", param); break;
				}
			} else
			{
				wsprintf(s, "%d", param & 0x0F);
				if ((gFXInfo[ndx].dwEffect == CMD_S3MCMDEX) || (gFXInfo[ndx].dwEffect == CMD_MODCMDEX))
				{
					switch(param & 0xF0)
					{
					case 0x30:
					case 0x40:
					case 0x50:
						if(gFXInfo[ndx].dwEffect == CMD_S3MCMDEX)
						{
							switch(param & 0x0F)
							{
								case 0x00: case 0x04: case 0x08: case 0x0C: strcpy(s, "sine wave"); break;
								case 0x01: case 0x05: case 0x09: case 0x0D: strcpy(s, "ramp down"); break;
								case 0x02: case 0x06: case 0x0A: case 0x0E: strcpy(s, "square wave"); break;
								case 0x03: case 0x07: case 0x0B: case 0x0F: strcpy(s, "random"); break;
							}
						}
						break;
					case 0x60:	if (gFXInfo[ndx].dwEffect == CMD_MODCMDEX) break;
					case 0xB0:  
						if (gFXInfo[ndx].dwEffect == CMD_S3MCMDEX)
						{
							if((param & 0x0F) == 0x00)
								strcpy(s, "loop start");
							else
								strcat(s, " times");
						}
						break;
					case 0xC0:
					case 0xD0:	strcat(s, " frames"); break;
					case 0xE0:	strcat(s, " rows"); break;
					case 0xF0:	wsprintf(s, "SF%X", param & 0x0F); break;
					}
				}
			}
			
		} else
		{
			wsprintf(s, "%d", param);
		}
	}
	strcat(pszName, s);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////
// Volume column effects description

typedef struct MPTVOLCMDINFO
{
	DWORD dwVolCmd;		// VOLCMD_XXXX
	DWORD dwFormats;	// MOD_TYPE_XXX combo
	LPCSTR pszName;		// ie "Set Volume"
} MPTVOLCMDINFO;

#define MAX_VOLINFO		15			//rewbs.volOff & rewbs.velocity: increased from 13


const MPTVOLCMDINFO gVolCmdInfo[MAX_VOLINFO] =
{
	{VOLCMD_VOLUME,			MOD_TYPE_NOMOD,		"v: Set Volume"},
	{VOLCMD_PANNING,		MOD_TYPE_NOMOD,		"p: Set Panning"},
	{VOLCMD_VOLSLIDEUP,		MOD_TYPE_XMITMPT,		"c: Volume slide up"},
	{VOLCMD_VOLSLIDEDOWN,	MOD_TYPE_XMITMPT,		"d: Volume slide down"},
	{VOLCMD_FINEVOLUP,		MOD_TYPE_XMITMPT,		"a: Fine volume up"},
	{VOLCMD_FINEVOLDOWN,	MOD_TYPE_XMITMPT,		"b: Fine volume down"},
	{VOLCMD_VIBRATOSPEED,	MOD_TYPE_XMITMPT,		"u: Vibrato speed"},
	{VOLCMD_VIBRATO,		MOD_TYPE_XM,		"h: Vibrato depth"},
	{VOLCMD_PANSLIDELEFT,	MOD_TYPE_XM,		"l: Pan slide left"},
	{VOLCMD_PANSLIDERIGHT,	MOD_TYPE_XM,		"r: Pan slide right"},
	{VOLCMD_TONEPORTAMENTO,	MOD_TYPE_XMITMPT,		"g: Tone portamento"},
	{VOLCMD_PORTAUP,		MOD_TYPE_ITMPT,		"f: Portamento up"},
	{VOLCMD_PORTADOWN,		MOD_TYPE_ITMPT,		"e: Portamento down"},
	{VOLCMD_VELOCITY,		MOD_TYPE_ITMPT,		":: velocity"},		//rewbs.velocity
	{VOLCMD_OFFSET,		MOD_TYPE_ITMPT,		"o: offset"},		//rewbs.volOff
};


UINT CModDoc::GetNumVolCmds() const
//---------------------------------
{
	return MAX_VOLINFO;
}


LONG CModDoc::GetIndexFromVolCmd(UINT volcmd)
//-------------------------------------------
{
	for (UINT i=0; i<MAX_VOLINFO; i++)
	{
		if (gVolCmdInfo[i].dwVolCmd == volcmd) return i;
	}
	return -1;
}


UINT CModDoc::GetVolCmdFromIndex(UINT ndx)
//----------------------------------------
{
	return (ndx < MAX_VOLINFO) ? gVolCmdInfo[ndx].dwVolCmd : 0;
}


BOOL CModDoc::GetVolCmdInfo(UINT ndx, LPSTR s, DWORD *prangeMin, DWORD *prangeMax)
//--------------------------------------------------------------------------------
{
	if (s) s[0] = 0;
	if (prangeMin) *prangeMin = 0;
	if (prangeMax) *prangeMax = 0;
	if (ndx >= MAX_VOLINFO) return FALSE;
	if (s) strcpy(s, gVolCmdInfo[ndx].pszName);
	if ((prangeMin) && (prangeMax))
	{
		switch(gVolCmdInfo[ndx].dwVolCmd)
		{
		case VOLCMD_VOLUME:
			*prangeMax = 64;
			break;

		case VOLCMD_PANNING:
			*prangeMax = 64;
			if (m_SndFile.m_nType & MOD_TYPE_XM)
			{
				*prangeMin = 2;		// 0*4+2
				*prangeMax = 62;	// 15*4+2
			}
			break;

		default:
			*prangeMax = (m_SndFile.m_nType & MOD_TYPE_XM) ? 15 : 9;
		}
	}
	return (gVolCmdInfo[ndx].dwFormats & m_SndFile.m_nType) ? TRUE : FALSE;
}


//rewbs.customKeys
void* CModDoc::GetChildFrame()
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (!pMainFrm) return 0;
	CMDIChildWnd *pMDIActive = pMainFrm->MDIGetActive();
	if (pMDIActive)
	{
		CView *pView = pMDIActive->GetActiveView();
		if ((pView) && (pView->GetDocument() == this))
			return ((CChildFrame *)pMDIActive);
	}
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = GetNextView(pos);
		if ((pView) && (pView->GetDocument() == this))
			return (CChildFrame *)pView->GetParentFrame();
	}

	return NULL;
}

HWND CModDoc::GetEditPosition(UINT &row, UINT &pat, UINT &ord)
//------------------------------------------------------------
{
	HWND followSonghWnd;
	PATTERNVIEWSTATE *patternViewState;
	CChildFrame *pChildFrm = (CChildFrame *) GetChildFrame();
	CSoundFile *pSndFile = GetSoundFile();

	if (strcmp("CViewPattern", pChildFrm->GetCurrentViewClassName()) == 0) // dirty HACK
	{
		followSonghWnd = pChildFrm->GetHwndView();
		PATTERNVIEWSTATE patternViewState;
		pChildFrm->SendViewMessage(VIEWMSG_SAVESTATE, (LPARAM)(&patternViewState));
		
		pat = patternViewState.nPattern;
		row = patternViewState.nRow;
		ord = patternViewState.nOrder;	
	}
	else	//patern editor object does not exist (i.e. is not active)  - use saved state.
	{
		followSonghWnd = NULL;
		patternViewState = pChildFrm->GetPatternViewState();

		pat = patternViewState->nPattern;
		row = patternViewState->nRow;
		ord = patternViewState->nOrder;
	}
	//rewbs.fix3185: if position is invalid, go to start of song.
	if (ord >= m_SndFile.Order.size()) {
		ord = 0;
		pat = pSndFile->Order[ord];
	}
	if (pat >= m_SndFile.Patterns.Size()) {
		pat=0;
	}
	if (row >= pSndFile->PatternSize[pat]) {
		row=0;
	}
	//end rewbs.fix3185

	//ensure order correlates with pattern.
	if (pSndFile->Order[ord]!=pat) {
		int tentativeOrder = pSndFile->FindOrder(pat);
		if (tentativeOrder != -1) {	//ensure a valid order exists.
			ord = tentativeOrder;
		}
	}


	return followSonghWnd;

}

int CModDoc::GetMacroType(CString value)
{
	if (value.Compare("")==0) return sfx_unused;
	if (value.Compare("F0F000z")==0) return sfx_cutoff;
	if (value.Compare("F0F001z")==0) return sfx_reso;
	if (value.Compare("F0F002z")==0) return sfx_mode;
	if (value.Compare("F0F003z")==0) return sfx_drywet;
	if (value.Compare("BK00z")>=0 && value.Compare("BKFFz")<=0 && value.GetLength()==5)
		return sfx_cc;
	if (value.Compare("F0F079z")>0 && value.Compare("F0F1G")<0 && value.GetLength()==7)
		return sfx_plug; 
	return sfx_custom; //custom/unknown
}

int CModDoc::MacroToPlugParam(CString macro)
{
	int code=0;
	char* param = (char *) (LPCTSTR) macro;
	param +=4;
	if ((param[0] >= '0') && (param[0] <= '9')) code = (param[0] - '0') << 4; else
	if ((param[0] >= 'A') && (param[0] <= 'F')) code = (param[0] - 'A' + 0x0A) << 4;
	if ((param[1] >= '0') && (param[1] <= '9')) code += (param[1] - '0'); else
	if ((param[1] >= 'A') && (param[1] <= 'F')) code += (param[1] - 'A' + 0x0A);

	if (macro.GetLength()>=4 && macro.GetAt(3)=='0') {
		return (code-128);
	} else {
		return (code+128);
	}
}
int CModDoc::MacroToMidiCC(CString macro) {
	int code=0;
	char* param = (char *) (LPCTSTR) macro;
	param +=2;
	if ((param[0] >= '0') && (param[0] <= '9')) code = (param[0] - '0') << 4; else
	if ((param[0] >= 'A') && (param[0] <= 'F')) code = (param[0] - 'A' + 0x0A) << 4;
	if ((param[1] >= '0') && (param[1] <= '9')) code += (param[1] - '0'); else
	if ((param[1] >= 'A') && (param[1] <= 'F')) code += (param[1] - 'A' + 0x0A);

	return code;
}

int CModDoc::FindMacroForParam(long param) {
//------------------------------------------
	for (int macro=0; macro<16; macro++) {	//what's the named_const for num macros?? :D
		CString macroString = &(GetSoundFile()->m_MidiCfg.szMidiSFXExt[macro*32]);
		if (GetMacroType(macroString) == sfx_plug &&  MacroToPlugParam(macroString) == param) {
			return macro;
		}
	}

	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////
// Playback


void CModDoc::OnPatternRestart()
//------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CChildFrame *pChildFrm = (CChildFrame *) GetChildFrame();

	if ((pMainFrm) && (pChildFrm))
	{
		if (strcmp("CViewPattern", pChildFrm->GetCurrentViewClassName()) == 0)
		{
			//User has sent play pattern command: set loop pattern checkbox to true.
			pChildFrm->SendViewMessage(VIEWMSG_PATTERNLOOP, 1);
		}
                   
		CSoundFile *pSndFile = GetSoundFile();
		UINT nPat,nOrd,nRow;
		HWND followSonghWnd;

		followSonghWnd = GetEditPosition(nRow, nPat, nOrd);
		CModDoc *pModPlaying = pMainFrm->GetModPlaying();
		
		BEGIN_CRITICAL();

		// Cut instruments/samples
		for (UINT i=0; i<MAX_CHANNELS; i++)
		{
			pSndFile->Chn[i].nPatternLoopCount = 0;
			pSndFile->Chn[i].nPatternLoop = 0;
			pSndFile->Chn[i].nFadeOutVol = 0;
			pSndFile->Chn[i].dwFlags |= CHN_NOTEFADE | CHN_KEYOFF;
		}
		if ((nOrd < m_SndFile.Order.size()) && (pSndFile->Order[nOrd] == nPat)) pSndFile->m_nCurrentPattern = pSndFile->m_nNextPattern = nOrd;
		pSndFile->m_dwSongFlags &= ~(SONG_PAUSED|SONG_STEP);
		pSndFile->LoopPattern(nPat);
		pSndFile->m_nNextRow = 0;
		pSndFile->ResetTotalTickCount();
		//rewbs.vstCompliance
		if (pModPlaying == this) {
			pSndFile->StopAllVsti();
		} else {
			pSndFile->ResumePlugins();
		}
		//end rewbs.vstCompliance
		END_CRITICAL();
		
		pMainFrm->ResetElapsedTime();
		if (pModPlaying != this) {
			pMainFrm->PlayMod(this, followSonghWnd, m_dwNotifyType|MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS); //rewbs.fix2977
		}
	}
	//SwitchToView();
}

void CModDoc::OnPatternPlay()
//---------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CChildFrame *pChildFrm = (CChildFrame *) GetChildFrame();

	if ((pMainFrm) && (pChildFrm))
	{
		if (strcmp("CViewPattern", pChildFrm->GetCurrentViewClassName()) == 0)
		{
			//User has sent play pattern command: set loop pattern checkbox to true.
			pChildFrm->SendViewMessage(VIEWMSG_PATTERNLOOP, 1);
		}
                   
		CSoundFile *pSndFile = GetSoundFile();
		UINT nRow,nPat,nOrd;
		HWND followSonghWnd;

		followSonghWnd = GetEditPosition(nRow,nPat,nOrd);
		CModDoc *pModPlaying = pMainFrm->GetModPlaying();
	
		BEGIN_CRITICAL();
		// Cut instruments/samples
		for (UINT i=pSndFile->m_nChannels; i<MAX_CHANNELS; i++)
		{
			pSndFile->Chn[i].dwFlags |= CHN_NOTEFADE | CHN_KEYOFF;
		}
		if ((nOrd < m_SndFile.Order.size()) && (pSndFile->Order[nOrd] == nPat)) pSndFile->m_nCurrentPattern = pSndFile->m_nNextPattern = nOrd;
		pSndFile->m_dwSongFlags &= ~(SONG_PAUSED|SONG_STEP);
		pSndFile->LoopPattern(nPat);
		pSndFile->m_nNextRow = nRow;
		//rewbs.VSTCompliance		
		if (pModPlaying == this) {
			pSndFile->StopAllVsti();
		} else {
			pSndFile->ResumePlugins();
		}
		//end rewbs.VSTCompliance
		END_CRITICAL();

		pMainFrm->ResetElapsedTime();
		if (pModPlaying != this) {
			pMainFrm->PlayMod(this, followSonghWnd, m_dwNotifyType|MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS);  //rewbs.fix2977
		}
	}
	//SwitchToView();

}

void CModDoc::OnPatternPlayNoLoop()
//---------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CChildFrame *pChildFrm = (CChildFrame *) GetChildFrame();

	if ((pMainFrm) && (pChildFrm))
	{
		if (strcmp("CViewPattern", pChildFrm->GetCurrentViewClassName()) == 0)
		{
			//User has sent play song command: set loop pattern checkbox to false.
			pChildFrm->SendViewMessage(VIEWMSG_PATTERNLOOP, 0);
		}
                   
		CSoundFile *pSndFile = GetSoundFile();
		UINT nPat,nOrd,nRow;
		HWND followSonghWnd;

		followSonghWnd = GetEditPosition(nRow,nPat,nOrd);
		CModDoc *pModPlaying = pMainFrm->GetModPlaying();

		BEGIN_CRITICAL();
		// Cut instruments/samples
		for (UINT i=pSndFile->m_nChannels; i<MAX_CHANNELS; i++)
		{
			pSndFile->Chn[i].dwFlags |= CHN_NOTEFADE | CHN_KEYOFF;
		}
		pSndFile->m_dwSongFlags &= ~(SONG_PAUSED|SONG_STEP);
		pSndFile->SetCurrentOrder(nOrd);
		if (pSndFile->Order[nOrd] == nPat)
			pSndFile->DontLoopPattern(nPat, nRow);
		else
			pSndFile->LoopPattern(nPat);
		pSndFile->m_nNextRow = nRow;
		//end rewbs.VSTCompliance
		if (pModPlaying == this) {
			pSndFile->StopAllVsti();	
		} else {
			pSndFile->ResumePlugins();
		}
		//rewbs.VSTCompliance
		END_CRITICAL();

		pMainFrm->ResetElapsedTime();
		
		if (pModPlaying != this)	{
			pMainFrm->PlayMod(this, followSonghWnd, m_dwNotifyType|MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS);  //rewbs.fix2977
		}
	}
	//SwitchToView();
}

LRESULT CModDoc::OnCustomKeyMsg(WPARAM wParam, LPARAM /*lParam*/)
//------------------------------------------------------------
{
	if (wParam == kcNull)
		return NULL;

	switch(wParam)
	{
		case kcViewGeneral: OnEditGlobals(); break;
		case kcViewPattern: OnEditPatterns(); break;
		case kcViewSamples: OnEditSamples(); break;
		case kcViewInstruments: OnEditInstruments(); break;
		case kcViewComments: OnEditComments(); break;
		case kcViewGraph: OnEditGraph(); break; //rewbs.graph
		case kcViewSongProperties: SongProperties(); break;

		case kcFileSaveAsWave:	OnFileWaveConvert(); break;
		case kcFileSaveAsMP3:	OnFileMP3Convert(); break;
		case kcFileSaveMidi:	OnFileMidiConvert(); break;
		case kcFileExportCompat:  OnFileCompatibilitySave(); break;
		case kcEstimateSongLength: OnEstimateSongLength(); break;
		case kcApproxRealBPM:	OnApproximateBPM(); break;
		case kcFileSave:		DoSave(m_strPathName, 0); break;
		case kcFileSaveAs:		DoSave(NULL, 1); break;
		case kcFileClose:		OnFileClose(); break;

		case kcPlayPatternFromCursor: OnPatternPlay(); break;
		case kcPlayPatternFromStart: OnPatternRestart(); break;
		case kcPlaySongFromCursor: OnPatternPlayNoLoop(); break;
		case kcPlaySongFromStart: OnPlayerPlayFromStart(); break;
		case kcPlayPauseSong: OnPlayerPlay(); break;
		case kcStopSong: OnPlayerStop(); break;
//		case kcPauseSong: OnPlayerPause(); break;
			

	}

	return wParam;
}
//end rewbs.customKeys

void CModDoc::TogglePluginEditor(UINT m_nCurrentPlugin)
{
	PSNDMIXPLUGIN pPlugin;

	pPlugin = &m_SndFile.m_MixPlugins[m_nCurrentPlugin];
	if (m_nCurrentPlugin<MAX_MIXPLUGINS && pPlugin && pPlugin->pMixPlugin)
	{
		CVstPlugin *pVstPlugin = (CVstPlugin *)pPlugin->pMixPlugin;
		pVstPlugin->ToggleEditor();
	}

	return;
}

void CModDoc::ChangeFileExtension(UINT nNewType)
//----------------------------------------------
{
	//Not making path if path is empty(case only(?) for new file)
	if(GetPathName().GetLength() > 0)
	{
		CHAR path[_MAX_PATH], drive[_MAX_PATH], fname[_MAX_FNAME], ext[_MAX_EXT];
		_splitpath(GetPathName(), drive, path, fname, ext);

		CString newPath = drive;
		newPath += path;

		//Catch case where we don't have a filename yet.
		if (fname[0] == 0) {
			newPath += GetTitle();
		} else {
			newPath += fname;
		}

		switch(nNewType)
		{
		case MOD_TYPE_XM:  newPath += ".xm"; break;
		case MOD_TYPE_IT:  m_SndFile.m_dwSongFlags & SONG_ITPROJECT ? newPath+=".itp" : newPath+=".it"; break;
		case MOD_TYPE_MPT: newPath += ".mptm"; break;
		case MOD_TYPE_S3M: newPath += ".s3m"; break;
		case MOD_TYPE_MOD: newPath += ".mod"; break;
		default: ASSERT(false);		
		}
	
		if(nNewType != MOD_TYPE_IT ||
			(nNewType == MOD_TYPE_IT &&
				(
					(!strcmp(ext, ".it") && (m_SndFile.m_dwSongFlags & SONG_ITPROJECT)) ||
					(!strcmp(ext, ".itp") && !(m_SndFile.m_dwSongFlags & SONG_ITPROJECT))
				)
			)
		  ) 
			m_ShowSavedialog = true;
			//Forcing savedialog to appear after extension change - otherwise
			//unnotified file overwriting may occur.

		SetPathName(newPath, FALSE);

		
		
	}

	UpdateAllViews(NULL, HINT_MODTYPE);
}



UINT CModDoc::FindAvailableChannel()
//-------------------------------------------
{
	// Search for available channel
	for (UINT j=m_SndFile.m_nChannels; j<MAX_CHANNELS; j++)	{
		MODCHANNEL *p = &m_SndFile.Chn[j];
		if (!p->nLength) {
			return j;
		}
	}

	// Not found: look for one that's stopped
	for (UINT j=m_SndFile.m_nChannels; j<MAX_CHANNELS; j++)	{
		MODCHANNEL *p = &m_SndFile.Chn[j];
		if (p->dwFlags & CHN_NOTEFADE) {
			return j;
		}
	}
	
	//Last resort: go for first virutal channel.
	return m_SndFile.m_nChannels;
}

void CModDoc::RecordParamChange(int plugSlot, long paramIndex) 
//------------------------------------------------------
{
	CVstPlugin *pPlug = (CVstPlugin*)m_SndFile.m_MixPlugins[plugSlot].pMixPlugin;
	if (pPlug) {
		UINT value = pPlug->GetZxxParameter(paramIndex);
		SendMessageToActiveViews(WM_MOD_RECORDPARAM, paramIndex, value);
	}
}

void CModDoc::LearnMacro(int macroToSet, long paramToUse)
//-------------------------------------------------------
{
	if (macroToSet<0 || macroToSet>NMACROS) {
		return;
	}
	
	//if macro already exists for this param, alert user and return
	for (int checkMacro=0; checkMacro<NMACROS; checkMacro++)	{
		CString macroText = &(GetSoundFile()->m_MidiCfg.szMidiSFXExt[checkMacro*32]);
 		int macroType = GetMacroType(macroText);
		
		if (macroType==sfx_plug && MacroToPlugParam(macroText)==paramToUse) {
			CString message;
			message.Format("Param %d can already be controlled with macro %X", paramToUse, checkMacro);
			CMainFrame::GetMainFrame()->MessageBox(message, "Macro exists for this param",MB_ICONINFORMATION | MB_OK);
			return;
		}
	}

	//set new macro
	CHAR *pMacroToSet = &(GetSoundFile()->m_MidiCfg.szMidiSFXExt[macroToSet*32]);
	if (paramToUse<128) {
		wsprintf(pMacroToSet, "F0F0%Xz",paramToUse+128);
	} else if (paramToUse<384) {
		wsprintf(pMacroToSet, "F0F1%Xz",paramToUse-128);
	} else {
		CString message;
		message.Format("Param %d beyond controllable range.", paramToUse);
		::MessageBox(NULL,message, "Macro not assigned for this param",MB_ICONINFORMATION | MB_OK);
		return;
	}

	CString message;
	message.Format("Param %d can now be controlled with macro %X", paramToUse, macroToSet);
	::MessageBox(NULL,message, "Macro assigned for this param",MB_ICONINFORMATION | MB_OK);
	
	return;
}

void CModDoc::SongProperties()
//----------------------------
{
	CModTypeDlg dlg(GetSoundFile(), CMainFrame::GetMainFrame());
	if (dlg.DoModal() == IDOK)
	{	
		BOOL bShowLog = FALSE;
		ClearLog();
		if(dlg.m_nType)
		{
			if (!ChangeModType(dlg.m_nType)) return;
			bShowLog = TRUE;
		}
		if ((dlg.m_nChannels >= 4) && (dlg.m_nChannels != GetSoundFile()->m_nChannels))
		{
			const bool showCancelInRemoveDlg = m_SndFile.GetModSpecifications().channelsMax >= m_SndFile.GetNumChannels();
			if(ChangeNumChannels(dlg.m_nChannels, showCancelInRemoveDlg)) bShowLog = TRUE;
		}

		if (bShowLog) ShowLog("Conversion Status", CMainFrame::GetMainFrame());
		SetModified();
	}
}

