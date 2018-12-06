/*
 * view_tre.cpp
 * ------------
 * Purpose: Tree view for managing open songs, sound files, file browser, ...
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Mainfrm.h"
#include "InputHandler.h"
#include "ImageLists.h"
#include "View_tre.h"
#include "Mptrack.h"
#include "Moddoc.h"
#include "Dlsbank.h"
#include "dlg_misc.h"
#include "../common/mptFileIO.h"
#include "../common/FileReader.h"
#include "FileDialog.h"
#include "Globals.h"
#include "ExternalSamples.h"
#include "FolderScanner.h"
#include "../soundlib/mod_specifications.h"
#include "../soundlib/plugins/PlugInterface.h"
#include "../soundlib/MIDIEvents.h"
#include <tchar.h>


OPENMPT_NAMESPACE_BEGIN


CSoundFile *CModTree::m_SongFile = nullptr;

/////////////////////////////////////////////////////////////////////////////
// CModTreeDropTarget


BOOL CModTreeDropTarget::Register(CModTree *pWnd)
{
	m_pModTree = pWnd;
	return COleDropTarget::Register(pWnd);
}


DROPEFFECT CModTreeDropTarget::OnDragEnter(CWnd *pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if ((m_pModTree) && (m_pModTree == pWnd)) return m_pModTree->OnDragEnter(pDataObject, dwKeyState, point);
	return DROPEFFECT_NONE;
}


DROPEFFECT CModTreeDropTarget::OnDragOver(CWnd *pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if ((m_pModTree) && (m_pModTree == pWnd)) return m_pModTree->OnDragOver(pDataObject, dwKeyState, point);
	return DROPEFFECT_NONE;
}


BOOL CModTreeDropTarget::OnDrop(CWnd *pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	if ((m_pModTree) && (m_pModTree == pWnd)) return m_pModTree->OnDrop(pDataObject, dropEffect, point);
	return FALSE;
}


ModTreeDocInfo::ModTreeDocInfo(CModDoc &modDoc)
	: modDoc(modDoc)
{
	const CSoundFile &sndFile = modDoc.GetSoundFile();
	tiPatterns.resize(sndFile.Patterns.Size(), nullptr);
	tiOrders.resize(sndFile.Order.GetNumSequences());
	tiSequences.resize(sndFile.Order.GetNumSequences(), nullptr);
	samplesPlaying.reset();
	instrumentsPlaying.reset();
}


/////////////////////////////////////////////////////////////////////////////
// CModTree

BEGIN_MESSAGE_MAP(CModTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CViewModTree)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(NM_DBLCLK,		&CModTree::OnItemDblClk)
	ON_NOTIFY_REFLECT(NM_RETURN,		&CModTree::OnItemReturn)
	ON_NOTIFY_REFLECT(NM_RCLICK,		&CModTree::OnItemRightClick)
	ON_NOTIFY_REFLECT(NM_CLICK,			&CModTree::OnItemLeftClick)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED,	&CModTree::OnItemExpanded)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG,	&CModTree::OnBeginLDrag)
	ON_NOTIFY_REFLECT(TVN_BEGINRDRAG,	&CModTree::OnBeginRDrag)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT,&CModTree::OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT,	&CModTree::OnEndLabelEdit)
	ON_COMMAND(ID_MODTREE_REFRESH,		&CModTree::OnRefreshTree)
	ON_COMMAND(ID_MODTREE_EXECUTE,		&CModTree::OnExecuteItem)
	ON_COMMAND(ID_MODTREE_REMOVE,		&CModTree::OnDeleteTreeItem)
	ON_COMMAND(ID_MODTREE_PLAY,			&CModTree::OnPlayTreeItem)
	ON_COMMAND(ID_MODTREE_REFRESHINSTRLIB, &CModTree::OnRefreshInstrLib)
	ON_COMMAND(ID_MODTREE_OPENITEM,		&CModTree::OnOpenTreeItem)
	ON_COMMAND(ID_MODTREE_MUTE,			&CModTree::OnMuteTreeItem)
	ON_COMMAND(ID_MODTREE_SOLO,			&CModTree::OnSoloTreeItem)
	ON_COMMAND(ID_MODTREE_UNMUTEALL,	&CModTree::OnUnmuteAllTreeItem)
	ON_COMMAND(ID_MODTREE_DUPLICATE,	&CModTree::OnDuplicateTreeItem)
	ON_COMMAND(ID_MODTREE_INSERT,		&CModTree::OnInsertTreeItem)
	ON_COMMAND(ID_MODTREE_SWITCHTO,		&CModTree::OnSwitchToTreeItem)
	ON_COMMAND(ID_MODTREE_CLOSE,		&CModTree::OnCloseItem)
	ON_COMMAND(ID_MODTREE_SETPATH,		&CModTree::OnSetItemPath)
	ON_COMMAND(ID_MODTREE_SAVEITEM,		&CModTree::OnSaveItem)
	ON_COMMAND(ID_MODTREE_SAVEALL,		&CModTree::OnSaveAll)
	ON_COMMAND(ID_MODTREE_RELOADITEM,	&CModTree::OnReloadItem)
	ON_COMMAND(ID_MODTREE_RELOADALL,	&CModTree::OnReloadAll)
	ON_COMMAND(ID_MODTREE_FINDMISSING,	&CModTree::OnFindMissing)
	ON_COMMAND(ID_ADD_SOUNDBANK,		&CModTree::OnAddDlsBank)
	ON_COMMAND(ID_IMPORT_MIDILIB,		&CModTree::OnImportMidiLib)
	ON_COMMAND(ID_EXPORT_MIDILIB,		&CModTree::OnExportMidiLib)
	ON_COMMAND(ID_SOUNDBANK_PROPERTIES,	&CModTree::OnSoundBankProperties)
	ON_COMMAND(ID_MODTREE_SHOWDIRS,		&CModTree::OnShowDirectories)
	ON_COMMAND(ID_MODTREE_SHOWALLFILES,	&CModTree::OnShowAllFiles)
	ON_COMMAND(ID_MODTREE_SOUNDFILESONLY,&CModTree::OnShowSoundFiles)
	ON_COMMAND(ID_MODTREE_GOTO_INSDIR,	&CModTree::OnGotoInstrumentDir)
	ON_COMMAND(ID_MODTREE_GOTO_SMPDIR,	&CModTree::OnGotoSampleDir)
	ON_MESSAGE(WM_MOD_KEYCOMMAND,		&CModTree::OnCustomKeyMsg)	//rewbs.customKeys
	ON_MESSAGE(WM_MOD_MIDIMSG,			&CModTree::OnMidiMsg)
	//}}AFX_MSG_MAP
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CViewModTree construction/destruction

CModTree::CModTree(CModTree *pDataTree)
	: m_pDataTree(pDataTree)
{
	if(m_pDataTree != nullptr)
	{
		// Set up instrument library monitoring thread
		m_hWatchDirKillThread = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hSwitchWatchDir = CreateEvent(NULL,  FALSE, FALSE, NULL);
		m_WatchDirThread = std::thread([&](){MonitorInstrumentLibrary();});
	}
	MemsetZero(m_tiMidi);
	MemsetZero(m_tiPerc);
}


CModTree::~CModTree()
{
	DocInfo.clear();

	delete m_SongFile;
	m_SongFile = nullptr;

	if(m_pDataTree != nullptr)
	{
		SetEvent(m_hWatchDirKillThread);
		m_WatchDirThread.join();
		CloseHandle(m_hSwitchWatchDir);
		CloseHandle(m_hWatchDirKillThread);
	}
}


void CModTree::Init()
{
	m_MediaFoundationExtensions = FileType(CSoundFile::GetMediaFoundationFileTypes()).GetExtensions();

	DWORD dwRemove = TVS_SINGLEEXPAND;
	DWORD dwAdd = TVS_EDITLABELS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (IsSampleBrowser())
	{
		dwRemove |= (TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS);
		dwAdd &= ~(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS);
	}
	if (TrackerSettings::Instance().m_dwPatternSetup & PATTERN_SINGLEEXPAND)
	{
		dwRemove &= ~TVS_SINGLEEXPAND;
		dwAdd |= TVS_SINGLEEXPAND;
		m_dwStatus |= TREESTATUS_SINGLEEXPAND;
	}
	ModifyStyle(dwRemove, dwAdd);
	ModifyStyleEx(0, WS_EX_ACCEPTFILES);

	if(!IsSampleBrowser())
	{
		std::vector<TCHAR> curDir(::GetCurrentDirectory(0, nullptr), '\0');
		::GetCurrentDirectory(static_cast<DWORD>(curDir.size()), curDir.data());
		const mpt::PathString dirs[] =
		{
			TrackerSettings::Instance().PathSamples.GetDefaultDir(),
			TrackerSettings::Instance().PathInstruments.GetDefaultDir(),
			TrackerSettings::Instance().PathSongs.GetDefaultDir(),
			mpt::PathString::FromNative(curDir.data())
		};
		for(auto &path : dirs)
		{
			m_InstrLibPath = path;
			if(!m_InstrLibPath.empty()) break;
		}
		m_InstrLibPath.EnsureTrailingSlash();
		m_pDataTree->InsLibSetFullPath(m_InstrLibPath, mpt::PathString());
	}

	SetImageList(&CMainFrame::GetMainFrame()->m_MiscIcons, TVSIL_NORMAL);
	if (!IsSampleBrowser())
	{
		// Create Midi Library
		m_hMidiLib = InsertItem(_T("MIDI Library"), IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT, TVI_LAST);
		for (UINT iMidGrp=0; iMidGrp<17; iMidGrp++)
		{
			InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, mpt::ToCString(mpt::CharsetASCII, szMidiGroupNames[iMidGrp]), IMAGE_FOLDER, IMAGE_FOLDER, 0, 0, (MODITEM_HDR_MIDIGROUP << MIDILIB_SHIFT) | iMidGrp, m_hMidiLib, TVI_LAST);
		}
	}
	m_hInsLib = InsertItem(_T("Instrument Library"), IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT, TVI_LAST);
	RefreshMidiLibrary();
	RefreshDlsBanks();
	RefreshInstrumentLibrary();
	m_DropTarget.Register(this);
}


BOOL CModTree::PreTranslateMessage(MSG *pMsg)
{
	if (!pMsg) return TRUE;

	if(m_doLabelEdit)
	{
		if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		{
			// End editing by making edit box lose focus.
			SetFocus();
			return TRUE;
		}
		return CTreeCtrl::PreTranslateMessage(pMsg);
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
		case VK_SPACE:
			if(!(pMsg->lParam & 0x40000000)) OnPlayTreeItem();
			return TRUE;

		case VK_RETURN:
			if(!(pMsg->lParam & 0x40000000))
			{
				HTREEITEM hItem = GetSelectedItem();
				if(hItem)
				{
					if(CMainFrame::GetInputHandler()->CtrlPressed())
					{
						if(IsSampleBrowser())
						{
							// Ctrl+Enter: Load sample into currently selected sample or instrument slot
							CModScrollView *view = static_cast<CModScrollView *>(CMainFrame::GetMainFrame()->GetActiveView());
							const ModItem modItem = GetModItem(hItem);
							if(view && (modItem.type == MODITEM_INSLIB_SAMPLE || modItem.type == MODITEM_INSLIB_INSTRUMENT))
							{
								const mpt::PathString file = InsLibGetFullPath(hItem);
								const char *className = view->GetRuntimeClass()->m_lpszClassName;
								if(!strcmp("CViewSample", className))
								{
									view->SendCtrlMessage(CMainFrame::GetInputHandler()->ShiftPressed() ? CTRLMSG_SMP_OPENFILE_NEW : CTRLMSG_SMP_OPENFILE, (LPARAM)&file);
								} else if(!strcmp("CViewInstrument", className))
								{
									view->SendCtrlMessage(CMainFrame::GetInputHandler()->ShiftPressed() ? CTRLMSG_INS_OPENFILE_NEW : CTRLMSG_INS_OPENFILE, (LPARAM)&file);
								}
								// In case a message box like "create instrument for sample?" showed up
								SetFocus();
							}
						} else
						{
							// Ctrl+Enter: Edit item
							EditLabel(hItem);
						}
					} else
					{
						if(!ExecuteItem(hItem))
						{
							if(ItemHasChildren(hItem))
							{
								Expand(hItem, TVE_TOGGLE);
							}
						}
					}
				}
			}
			return TRUE;

		case VK_TAB:
			// Tab: Switch between folder and file view.
			if(this == CMainFrame::GetMainFrame()->GetUpperTreeview())
				CMainFrame::GetMainFrame()->GetLowerTreeview()->SetFocus();
			else
				CMainFrame::GetMainFrame()->GetUpperTreeview()->SetFocus();
			return TRUE;
		
		case VK_BACK:
			// Backspace: Go up one directory
			if(GetParentRootItem(GetSelectedItem()) == m_hInsLib || IsSampleBrowser())
			{
				CMainFrame::GetMainFrame()->GetUpperTreeview()->InstrumentLibraryChDir(P_(".."), false);
				return TRUE;
			}
			break;

		case VK_INSERT:
			InsertOrDupItem(!CMainFrame::GetInputHandler()->ShiftPressed());
			return TRUE;

		case VK_APPS:
			// Handle Application (menu) key
			if(HTREEITEM item = GetSelectedItem())
			{
				CRect rect;
				GetItemRect(item, &rect, FALSE);
				ClientToScreen(rect);
				OnItemRightClick(item, rect.TopLeft());
			}
			return TRUE;
		}
	} else if(pMsg->message == WM_CHAR)
	{
		ModItem item = GetModItem(GetSelectedItem());
		switch(item.type)
		{
		case MODITEM_MIDIINSTRUMENT:
		case MODITEM_MIDIPERCUSSION:
		case MODITEM_INSLIB_SAMPLE:
		case MODITEM_INSLIB_INSTRUMENT:
		case MODITEM_DLSBANK_INSTRUMENT:
			// Avoid cycling through tree-view elements on key hold
			return true;
		}
	}

	//We handle keypresses before Windows has a chance to handle them (for alt etc..)
	if ((pMsg->message == WM_SYSKEYUP)   || (pMsg->message == WM_KEYUP) ||
		(pMsg->message == WM_SYSKEYDOWN) || (pMsg->message == WM_KEYDOWN))
	{
		CInputHandler* ih = CMainFrame::GetInputHandler();

		//Translate message manually
		UINT nChar = (UINT)pMsg->wParam;
		UINT nRepCnt = LOWORD(pMsg->lParam);
		UINT nFlags = HIWORD(pMsg->lParam);
		KeyEventType kT = ih->GetKeyEventType(nFlags);
		InputTargetContext ctx = (InputTargetContext)(kCtxViewTree);

		if (ih->KeyEvent(ctx, nChar, nRepCnt, nFlags, kT) != kcNull)
			return true; // Mapped to a command, no need to pass message on.
	}
	return CTreeCtrl::PreTranslateMessage(pMsg);
}


mpt::PathString CModTree::InsLibGetFullPath(HTREEITEM hItem) const
{
	mpt::PathString fullPath = m_InstrLibPath;
	fullPath.EnsureTrailingSlash();
	return fullPath + mpt::PathString::FromCString(GetItemText(hItem));
}


bool CModTree::InsLibSetFullPath(const mpt::PathString &libPath, const mpt::PathString &songName)
{
	if(!songName.empty() && mpt::PathString::CompareNoCase(m_SongFileName, songName))
	{
		// Load module for previewing its instruments
		InputFile f(libPath + songName);
		if(f.IsValid())
		{
			FileReader file = GetFileReader(f);
			if(file.IsValid())
			{
				if(m_SongFile != nullptr)
				{
					m_SongFile->Destroy();
				} else
				{
					m_SongFile = new (std::nothrow) CSoundFile;
				}
				if(m_SongFile != nullptr)
				{
					if(!m_SongFile->Create(file, CSoundFile::loadNoPatternOrPluginData, nullptr))
					{
						return false;
					}
					// Destroy some stuff that we're not going to use anyway.
					m_SongFile->Patterns.DestroyPatterns();
					m_SongFile->m_songMessage.clear();
				}
			}
		} else
		{
			return false;
		}
	}
	m_InstrLibPath = libPath;
	m_SongFileName = songName;
	return true;
}


bool CModTree::SetSoundFile(FileReader &file)
{
	CSoundFile *sndFile = new (std::nothrow) CSoundFile;
	if(sndFile == nullptr || !sndFile->Create(file, CSoundFile::loadNoPatternOrPluginData))
	{
		delete sndFile;
		return false;
	}

	if(m_SongFile != nullptr)
	{
		m_SongFile->Destroy();
		delete m_SongFile;
	}
	m_SongFile = sndFile;
	m_SongFile->Patterns.DestroyPatterns();
	m_SongFile->m_songMessage.clear();
	const mpt::PathString fileName = file.GetFileName();
	m_InstrLibPath = fileName.GetPath();
	m_SongFileName = fileName.GetFullFileName();
	RefreshInstrumentLibrary();
	return true;
}


void CModTree::OnOptionsChanged()
{
	DWORD dwRemove = TVS_SINGLEEXPAND, dwAdd = 0;
	m_dwStatus &= ~TREESTATUS_SINGLEEXPAND;
	if (TrackerSettings::Instance().m_dwPatternSetup & PATTERN_SINGLEEXPAND)
	{
		dwRemove = 0;
		dwAdd = TVS_SINGLEEXPAND;
		m_dwStatus |= TREESTATUS_SINGLEEXPAND;
	}
	ModifyStyle(dwRemove, dwAdd);
}


void CModTree::AddDocument(CModDoc &modDoc)
{
	// Check if document is already in the list
	if(std::find_if(DocInfo.begin(), DocInfo.end(), [&modDoc](const std::unique_ptr<ModTreeDocInfo> &doc) { return &(doc->modDoc) == &modDoc; }) != DocInfo.end())
		return;

	try
	{
		DocInfo.push_back(std::make_unique<ModTreeDocInfo>(modDoc));
		auto &pInfo = DocInfo.back();

		UpdateView(*pInfo, UpdateHint().ModType());
		if(pInfo->hSong)
		{
			Expand(pInfo->hSong, TVE_EXPAND);
			EnsureVisible(pInfo->hSong);
			SelectItem(pInfo->hSong);
		}
	} MPT_EXCEPTION_CATCH_OUT_OF_MEMORY(e)
	{
		MPT_EXCEPTION_DELETE_OUT_OF_MEMORY(e);
	}
}


void CModTree::RemoveDocument(CModDoc &modDoc)
{
	auto doc = std::find_if(DocInfo.begin(), DocInfo.end(), [&modDoc](const std::unique_ptr<ModTreeDocInfo> &doc) { return &(doc->modDoc) == &modDoc; });
	if(doc != DocInfo.end())
	{
		DeleteItem((**doc).hSong);
		DocInfo.erase(doc);
	}
	// Refresh all item IDs
	for(size_t i = 0; i < DocInfo.size(); i++)
	{
		SetItemData(DocInfo[i]->hSong, i);
	}
}


// Get CModDoc that is associated with a tree item
ModTreeDocInfo *CModTree::GetDocumentInfoFromItem(HTREEITEM hItem)
{
	hItem = GetParentRootItem(hItem);
	if(hItem != nullptr)
	{
		// Root item has moddoc pointer
		const size_t doc = GetItemData(hItem);
		if(doc < DocInfo.size() && hItem == DocInfo[doc]->hSong)
		{
			return DocInfo[doc].get();
		}
	}
	return nullptr;
}


// Get modtree doc information for a given CModDoc
ModTreeDocInfo *CModTree::GetDocumentInfoFromModDoc(CModDoc &modDoc)
{
	auto doc = std::find_if(DocInfo.begin(), DocInfo.end(), [&modDoc](const std::unique_ptr<ModTreeDocInfo> &doc) { return &(doc->modDoc) == &modDoc; });
	if(doc != DocInfo.end())
		return doc->get();
	else
		return nullptr;
}


/////////////////////////////////////////////////////////////////////////////
// CViewModTree drawing

void CModTree::RefreshMidiLibrary()
{
	CString s;
	CString stmp;
	TVITEM tvi;
	const MidiLibrary &midiLib = CTrackApp::GetMidiLibrary();

	if (IsSampleBrowser()) return;
	// Midi Programs
	HTREEITEM parent = GetChildItem(m_hMidiLib);
	for(UINT iMidi = 0; iMidi < 128; iMidi++)
	{
		DWORD dwImage = IMAGE_INSTRMUTE;
		s = mpt::cfmt::val(iMidi) + _T(": ") + mpt::ToCString(mpt::CharsetASCII, szMidiProgramNames[iMidi]);
		const LPARAM param = (MODITEM_MIDIINSTRUMENT << MIDILIB_SHIFT) | iMidi;
		if(!midiLib[iMidi].empty())
		{
			s += _T(": ") + midiLib[iMidi].GetFullFileName().ToCString();
			dwImage = IMAGE_INSTRUMENTS;
		}
		if (!m_tiMidi[iMidi])
		{
			m_tiMidi[iMidi] = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
							s, dwImage, dwImage, 0, 0, param, parent, TVI_LAST);
		} else
		{
			tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvi.hItem = m_tiMidi[iMidi];
			tvi.pszText = stmp.GetBuffer(s.GetLength() + 1);
			tvi.cchTextMax = stmp.GetAllocLength();
			tvi.iImage = tvi.iSelectedImage = dwImage;
			GetItem(&tvi);
			s.ReleaseBuffer();
			if(s != stmp || tvi.iImage != (int)dwImage)
			{
				SetItem(m_tiMidi[iMidi], TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
					s, dwImage, dwImage, 0, 0, param);
			}
		}
		if((iMidi % 8u) == 7u)
		{
			parent = GetNextSiblingItem(parent);
		}
	}
	// Midi Percussions
	for (UINT iPerc=24; iPerc<=84; iPerc++)
	{
		DWORD dwImage = IMAGE_NOSAMPLE;
		s = mpt::ToCString(CSoundFile::GetNoteName((ModCommand::NOTE)(iPerc + NOTE_MIN), CSoundFile::GetDefaultNoteNames()))
			+ _T(": ") + mpt::ToCString(mpt::CharsetASCII, szMidiPercussionNames[iPerc - 24]);
		const LPARAM param = (MODITEM_MIDIPERCUSSION << MIDILIB_SHIFT) | iPerc;
		if(!midiLib[iPerc | 0x80].empty())
		{
			s += _T(": ") + midiLib[iPerc | 0x80].GetFullFileName().ToCString();
			dwImage = IMAGE_SAMPLES;
		}
		if (!m_tiPerc[iPerc])
		{
			m_tiPerc[iPerc] = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
							s, dwImage, dwImage, 0, 0, param, parent, TVI_LAST);
		} else
		{
			tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvi.hItem = m_tiPerc[iPerc];
			tvi.pszText = stmp.GetBuffer(s.GetLength() + 1);
			tvi.cchTextMax = stmp.GetAllocLength();
			tvi.iImage = tvi.iSelectedImage = dwImage;
			GetItem(&tvi);
			s.ReleaseBuffer();
			if(s != stmp || tvi.iImage != (int)dwImage)
			{
				SetItem(m_tiPerc[iPerc], TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE,
							s, dwImage, dwImage, 0, 0, param);
			}
		}
	}
}


void CModTree::RefreshDlsBanks()
{
	const mpt::Charset charset = mpt::CharsetLocale;
	TCHAR s[256];
	HTREEITEM hDlsRoot = m_hMidiLib;

	if (IsSampleBrowser()) return;

	if(m_tiDLS.size() < CTrackApp::gpDLSBanks.size())
	{
		m_tiDLS.resize(CTrackApp::gpDLSBanks.size(), nullptr);
	}

	for(size_t iDls=0; iDls < CTrackApp::gpDLSBanks.size(); iDls++)
	{
		if(CTrackApp::gpDLSBanks[iDls])
		{
			if(!m_tiDLS[iDls])
			{
				TVSORTCB tvs;
				CDLSBank *pDlsBank = CTrackApp::gpDLSBanks[iDls];
				// Add DLS file folder
				m_tiDLS[iDls] = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
					pDlsBank->GetFileName().GetFullFileName().AsNative().c_str(), IMAGE_FOLDER, IMAGE_FOLDER, 0, 0, iDls, TVI_ROOT, hDlsRoot);
				// Memorize Banks
				std::map<uint16, HTREEITEM> banks;
				// Add Drum Kits folder
				HTREEITEM hDrums = InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE,
						_T("Drum Kits"), IMAGE_FOLDER, IMAGE_FOLDER, 0, 0, 0, m_tiDLS[iDls], TVI_LAST);
				// Add Instruments
				UINT nInstr = pDlsBank->GetNumInstruments();
				for (UINT iIns=0; iIns<nInstr; iIns++)
				{
					const DLSINSTRUMENT *pDlsIns = pDlsBank->GetInstrument(iIns);
					if (pDlsIns)
					{
						TCHAR szName[256];
						wsprintf(szName, _T("%u: %s"), pDlsIns->ulInstrument & 0x7F, mpt::ToCString(charset, pDlsIns->szName).GetString());
						// Drum Kit
						if (pDlsIns->ulBank & F_INSTRUMENT_DRUMS)
						{
							HTREEITEM hKit = InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM,
								szName, IMAGE_FOLDER, IMAGE_FOLDER, 0, 0, pDlsIns->ulInstrument & 0x7F, hDrums, TVI_LAST);
							for (UINT iRgn=0; iRgn<pDlsIns->nRegions; iRgn++)
							{
								UINT keymin = pDlsIns->Regions[iRgn].uKeyMin;
								UINT keymax = pDlsIns->Regions[iRgn].uKeyMax;

								const CHAR *regionName = pDlsBank->GetRegionName(iIns, iRgn);
								if(regionName == nullptr && (keymin >= 24) && (keymin <= 84))
								{
									regionName = szMidiPercussionNames[keymin - 24];
								} else
								{
									regionName = "";
								}

								if (keymin >= keymax)
								{
									wsprintf(szName, _T("%s%u: %s"),
										mpt::ToCString(CSoundFile::GetDefaultNoteName(keymin % 12)).GetString(),
										keymin / 12,
										mpt::ToCString(charset, regionName).GetString());
								} else
								{
									wsprintf(szName, _T("%s%u-%s%u: %s"),
										mpt::ToCString(CSoundFile::GetDefaultNoteName(keymin % 12)).GetString(),
										keymin / 12,
										mpt::ToCString(CSoundFile::GetDefaultNoteName(keymax % 12)).GetString(),
										keymax / 12,
										mpt::ToCString(charset, regionName).GetString());
								}
								LPARAM lParam = DlsItem::EncodeValuePerc((uint8)(iRgn), (uint16)iIns);
								InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM,
										szName, IMAGE_INSTRUMENTS, IMAGE_INSTRUMENTS, 0, 0, lParam, hKit, TVI_LAST);
							}
							tvs.hParent = hKit;
							tvs.lpfnCompare = ModTreeDrumCompareProc;
							tvs.lParam = reinterpret_cast<LPARAM>(CTrackApp::gpDLSBanks[iDls]);
							SortChildrenCB(&tvs);
						} else
						// Melodic
						{
							uint16 mbank = (pDlsIns->ulBank & 0x7F7F);
							auto hbank = banks.find(mbank);
							if(hbank == banks.end())
							{
								wsprintf(s, (mbank) ? _T("Melodic Bank %02d.%02d") : _T("Melodic"), mbank >> 8, mbank & 0x7F);
								// Find out where to insert this bank in the tree
								hbank = banks.insert(std::make_pair(mbank, nullptr)).first;
								HTREEITEM insertAfter = (hbank == banks.begin()) ? TVI_FIRST : std::prev(hbank)->second;
								hbank->second = InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE,
									s, IMAGE_FOLDER, IMAGE_FOLDER, 0, 0, 0,
									m_tiDLS[iDls], insertAfter);
							}
							LPARAM lParam = DlsItem::EncodeValueInstr((pDlsIns->ulInstrument & 0x7F), (uint16)iIns);
							InsertItem(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM,
								szName, IMAGE_INSTRUMENTS, IMAGE_INSTRUMENTS, 0, 0, lParam, hbank->second, TVI_LAST);
						}
					}
				}
				// Sort items
				for(auto &b : banks)
				{
					tvs.hParent = b.second;
					tvs.lpfnCompare = ModTreeInsLibCompareProc;
					tvs.lParam = (LPARAM)this;
					SortChildrenCB(&tvs);
				}
				if (hDrums != NULL)
				{
					tvs.hParent = hDrums;
					tvs.lpfnCompare = ModTreeInsLibCompareProc;
					tvs.lParam = (LPARAM)this;
					SortChildrenCB(&tvs);
				}
			}
			hDlsRoot = m_tiDLS[iDls];
		} else
		{
			if (m_tiDLS[iDls])
			{
				DeleteItem(m_tiDLS[iDls]);
				m_tiDLS[iDls] = nullptr;
			}
		}
	}
}


void CModTree::RefreshInstrumentLibrary()
{
	SetRedraw(FALSE);
	// Check if the currently selected item should be selected after refreshing
	CString oldItem;
	if((IsSampleBrowser() || GetParentRootItem(GetSelectedItem()) == m_hInsLib)
		&& GetItemText(GetSampleBrowser()->m_hInsLib) == (m_SongFileName.empty() ? m_InstrLibPath : m_SongFileName).ToCString())
	{
		oldItem = GetItemText(GetSelectedItem());
	}
	EmptyInstrumentLibrary();
	FillInstrumentLibrary(oldItem);
	SetRedraw(TRUE);
	if(!IsSampleBrowser())
	{
		m_pDataTree->InsLibSetFullPath(m_InstrLibPath, m_SongFileName);
		m_pDataTree->RefreshInstrumentLibrary();
	}
}


void CModTree::UpdateView(ModTreeDocInfo &info, UpdateHint hint)
{
	TCHAR s[256], stmp[256];
	TVITEM tvi;
	MemsetZero(tvi);
	const FlagSet<HintType> hintType = hint.GetType();
	if (IsSampleBrowser() || hintType == HINT_NONE) return;

	const CModDoc &modDoc = info.modDoc;
	const CSoundFile &sndFile = modDoc.GetSoundFile();

	// Create headers
	s[0] = 0;
	const GeneralHint generalHint = hint.ToType<GeneralHint>();
	if(generalHint.GetType()[HINT_MODTYPE | HINT_MODGENERAL] || (!info.hSong))
	{
		// Module folder + sub folders
		CString name = modDoc.GetPathNameMpt().GetFullFileName().ToCString();
		if(name.IsEmpty()) name = mpt::PathString::FromCString(modDoc.GetTitle()).SanitizeComponent().ToCString();

		if(!info.hSong)
		{
			info.hSong = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, name, IMAGE_FOLDER, IMAGE_FOLDER, 0, 0, (LPARAM)(DocInfo.size() - 1), TVI_ROOT, TVI_FIRST);
			info.hOrders = InsertItem(_T("Sequence"), IMAGE_FOLDER, IMAGE_FOLDER, info.hSong, TVI_LAST);
			info.hPatterns = InsertItem(_T("Patterns"), IMAGE_FOLDER, IMAGE_FOLDER, info.hSong, TVI_LAST);
			info.hSamples = InsertItem(_T("Samples"), IMAGE_FOLDER, IMAGE_FOLDER, info.hSong, TVI_LAST);
		} else if(generalHint.GetType()[HINT_MODGENERAL | HINT_MODTYPE])
		{
			if(name != GetItemText(info.hSong))
			{
				SetItemText(info.hSong, name);
			}
		}
	}

	if (sndFile.GetModSpecifications().instrumentsMax > 0)
	{
		if(!info.hInstruments) info.hInstruments = InsertItem(_T("Instruments"), IMAGE_FOLDER, IMAGE_FOLDER, info.hSong, info.hSamples);
	} else
	{
		if(info.hInstruments)
		{
			DeleteItem(info.hInstruments);
			info.hInstruments = NULL;
		}
	}
	if (!info.hComments) info.hComments = InsertItem(_T("Comments"), IMAGE_COMMENTS, IMAGE_COMMENTS, info.hSong, TVI_LAST);
	// Add effects
	const PluginHint pluginHint = hint.ToType<PluginHint>();
	if (pluginHint.GetType()[HINT_MODTYPE | HINT_PLUGINNAMES])
	{
		HTREEITEM hItem = info.hEffects ? GetChildItem(info.hEffects) : nullptr;
		PLUGINDEX firstPlug = 0, lastPlug = MAX_MIXPLUGINS - 1;
		if(pluginHint.GetPlugin() && hItem)
		{
			// Only update one specific plugin name
			firstPlug = lastPlug = pluginHint.GetPlugin() - 1;
			while(hItem && GetItemData(hItem) != firstPlug)
			{
				hItem = GetNextSiblingItem(hItem);
			}
		}
		bool hasPlugs = false;
		for(PLUGINDEX i = firstPlug; i <= lastPlug; i++)
		{
			const SNDMIXPLUGIN &plugin = sndFile.m_MixPlugins[i];
			if(plugin.IsValidPlugin())
			{
				// Now we can be sure that we want to create this folder.
				if(!info.hEffects)
				{
					info.hEffects = InsertItem(_T("Plugins"), IMAGE_FOLDER, IMAGE_FOLDER, info.hSong, info.hInstruments ? info.hInstruments : info.hSamples);
				}

				wsprintf(s, _T("FX%u: %s"), i + 1, mpt::ToCString(m_SongFile->GetCharsetInternal(), plugin.GetName()).GetString());
				int nImage = IMAGE_NOPLUGIN;
				if(plugin.pMixPlugin != nullptr) nImage = (plugin.pMixPlugin->IsInstrument()) ? IMAGE_PLUGININSTRUMENT : IMAGE_EFFECTPLUGIN;
				
				if(hItem)
				{
					// Replace existing item
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
					tvi.hItem = hItem;
					tvi.pszText = stmp;
					tvi.cchTextMax = CountOf(stmp);
					GetItem(&tvi);
					if(tvi.iImage != nImage || tvi.lParam != i || _tcscmp(s, tvi.pszText))
					{
						SetItem(hItem, TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, nImage, nImage, 0, 0, i);
					}
					hItem = GetNextSiblingItem(hItem);
				} else
				{
					InsertItem(TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, nImage, nImage, 0, 0, i, info.hEffects, TVI_LAST);
				}
				hasPlugs = true;
			}
		}
		if(!hasPlugs && firstPlug == lastPlug)
		{
			// If we only updated one plugin, we still need to check all the other slots if there is any plugin in them.
			for(const auto &plug : sndFile.m_MixPlugins)
			{
				if(plug.IsValidPlugin())
				{
					hasPlugs = true;
					break;
				}
			}
		}
		if(!hasPlugs && info.hEffects)
		{
			DeleteItem(info.hEffects);
			info.hEffects = nullptr;
		} else if(!pluginHint.GetPlugin())
		{
			// Delete superfluous tree items
			while(hItem)
			{
				HTREEITEM nextItem = GetNextSiblingItem(hItem);
				DeleteItem(hItem);
				hItem = nextItem;
			}
		}
	}
	// Add Orders
	const PatternHint patternHint = hint.ToType<PatternHint>();
	const SequenceHint seqHint = hint.ToType<SequenceHint>();
	if (info.hOrders && (seqHint.GetType()[HINT_MODTYPE | HINT_MODSEQUENCE | HINT_SEQNAMES] || patternHint.GetType()[HINT_PATNAMES]))
	{
		const PATTERNINDEX nPat = patternHint.GetPattern();
		bool adjustParentNode = false;	// adjust sequence name of "Sequence" node?

		// (only one seq remaining || previously only one sequence): update parent item
		if((info.tiSequences.size() > 1 && sndFile.Order.GetNumSequences() == 1) || (info.tiSequences.size() == 1 && sndFile.Order.GetNumSequences() > 1))
		{
			for(auto &seq : info.tiOrders)
			{
				for(auto &ord : seq)
				{
					if(ord) DeleteItem(ord);
					ord = nullptr;
				}
			}
			for(auto &seq : info.tiSequences)
			{
				if(seq) DeleteItem(seq);
				seq = nullptr;
			}
			info.tiOrders.resize(sndFile.Order.GetNumSequences());
			info.tiSequences.resize(sndFile.Order.GetNumSequences(), nullptr);
			adjustParentNode = true;
		}

		// If there are too many sequences, delete them.
		for(size_t nSeq = sndFile.Order.GetNumSequences(); nSeq < info.tiSequences.size(); nSeq++) if (info.tiSequences[nSeq])
		{
			for(auto &ord : info.tiOrders[nSeq]) if (ord)
			{
				DeleteItem(ord); ord = nullptr;
			}
			DeleteItem(info.tiSequences[nSeq]); info.tiSequences[nSeq] = nullptr;
		}
		if (info.tiSequences.size() < sndFile.Order.GetNumSequences()) // Resize tiSequences if needed.
		{
			info.tiSequences.resize(sndFile.Order.GetNumSequences(), nullptr);
			info.tiOrders.resize(sndFile.Order.GetNumSequences());
		}

		HTREEITEM hAncestorNode = info.hOrders;

		SEQUENCEINDEX nSeqMin = 0, nSeqMax = sndFile.Order.GetNumSequences() - 1;
		SEQUENCEINDEX nHintParam = seqHint.GetSequence();
		if (seqHint.GetType()[HINT_SEQNAMES] && (nHintParam <= nSeqMax)) nSeqMin = nSeqMax = nHintParam;

		// Adjust caption of the "Sequence" node (if only one sequence exists, it should be labeled with the sequence name)
		if((seqHint.GetType()[HINT_SEQNAMES] && sndFile.Order.GetNumSequences() == 1) || adjustParentNode)
		{
			CString seqName = sndFile.Order(0).GetName().c_str();
			if(seqName.IsEmpty() || sndFile.Order.GetNumSequences() > 1)
				seqName = _T("Sequence");
			else
				seqName = _T("Sequence: ") + seqName;
			SetItem(info.hOrders, TVIF_TEXT, seqName, 0, 0, 0, 0, 0);
		}

		// go through all sequences
		CString seqName;
		for(SEQUENCEINDEX nSeq = nSeqMin; nSeq <= nSeqMax; nSeq++)
		{
			if(sndFile.Order.GetNumSequences() > 1)
			{
				// more than one sequence -> add folder
				if(sndFile.Order(nSeq).GetName().empty())
				{
					seqName.Format(_T("Sequence %u"), nSeq);
				} else
				{
					seqName.Format(_T("%u: "), nSeq);
					seqName += sndFile.Order(nSeq).GetName().c_str();
				}

				UINT state = (nSeq == sndFile.Order.GetCurrentSequenceIndex()) ? TVIS_BOLD : 0;

				if(info.tiSequences[nSeq] == NULL)
				{
					info.tiSequences[nSeq] = InsertItem(seqName, IMAGE_FOLDER, IMAGE_FOLDER, info.hOrders, TVI_LAST);
				}
				// Update bold item
				_tcscpy(stmp, seqName);
				tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
				tvi.state = 0;
				tvi.stateMask = TVIS_BOLD;
				tvi.hItem = info.tiSequences[nSeq];
				tvi.pszText = stmp;
				tvi.cchTextMax = CountOf(stmp);
				LPARAM param = (nSeq << SEQU_SHIFT) | ORDERINDEX_INVALID;
				GetItem(&tvi);
				if(tvi.state != state || tvi.pszText != seqName || tvi.lParam != param)
					SetItem(info.tiSequences[nSeq], TVIF_TEXT | TVIF_STATE | TVIF_PARAM, seqName, 0, 0, state, TVIS_BOLD, param);

				hAncestorNode = info.tiSequences[nSeq];
			}

			const ORDERINDEX ordLength = sndFile.Order(nSeq).GetLengthTailTrimmed();
			// If there are items past the new sequence length, delete them.
			for(size_t nOrd = ordLength; nOrd < info.tiOrders[nSeq].size(); nOrd++) if (info.tiOrders[nSeq][nOrd])
			{
				DeleteItem(info.tiOrders[nSeq][nOrd]); info.tiOrders[nSeq][nOrd] = NULL;
			}
			if (info.tiOrders[nSeq].size() < ordLength) // Resize tiOrders if needed.
				info.tiOrders[nSeq].resize(ordLength, nullptr);
			const bool patNamesOnly = patternHint.GetType()[HINT_PATNAMES];

			//if (hintFlagPart == HINT_PATNAMES) && (dwHintParam < sndFile.Order().GetLength())) imin = imax = dwHintParam;
			CString patName;
			for (ORDERINDEX iOrd = 0; iOrd < ordLength; iOrd++)
			{
				if(patNamesOnly && sndFile.Order(nSeq)[iOrd] != nPat)
					continue;
				UINT state = (iOrd == info.nOrdSel && nSeq == info.nSeqSel) ? TVIS_BOLD : 0;
				if (sndFile.Order(nSeq)[iOrd] < sndFile.Patterns.Size())
				{
					patName = mpt::ToCString(sndFile.GetCharsetInternal(), sndFile.Patterns[sndFile.Order(nSeq)[iOrd]].GetName());
					if(!patName.IsEmpty())
					{
						wsprintf(s, (TrackerSettings::Instance().m_dwPatternSetup & PATTERN_HEXDISPLAY) ? _T("[%02Xh] %u: ") : _T("[%02u] %u: "),
							iOrd, sndFile.Order(nSeq)[iOrd]);
						_tcscat(s, patName.GetString());
					} else
					{
						wsprintf(s, (TrackerSettings::Instance().m_dwPatternSetup & PATTERN_HEXDISPLAY) ? _T("[%02Xh] Pattern %u") : _T("[%02u] Pattern %u"),
							iOrd, sndFile.Order(nSeq)[iOrd]);
					}
				} else
				{
					if(sndFile.Order(nSeq)[iOrd] == sndFile.Order.GetIgnoreIndex())
					{
						// +++ Item
						wsprintf(s, _T("[%02u] Skip"), iOrd);
					} else
					{
						// --- Item
						wsprintf(s, _T("[%02u] Stop"), iOrd);
					}
				}

				LPARAM param = (nSeq << SEQU_SHIFT) | iOrd;
				if (info.tiOrders[nSeq][iOrd])
				{
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_STATE;
					tvi.state = 0;
					tvi.stateMask = TVIS_BOLD;
					tvi.hItem = info.tiOrders[nSeq][iOrd];
					tvi.pszText = stmp;
					tvi.cchTextMax = CountOf(stmp);
					GetItem(&tvi);
					if(tvi.state != state || _tcscmp(s, stmp))
						SetItem(info.tiOrders[nSeq][iOrd], TVIF_TEXT | TVIF_STATE | TVIF_PARAM, s, 0, 0, state, TVIS_BOLD, param);
				} else
				{
					info.tiOrders[nSeq][iOrd] = InsertItem(TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, IMAGE_PARTITION, IMAGE_PARTITION, 0, 0, param, hAncestorNode, TVI_LAST);
				}
			}
		}
	}
	// Add Patterns
	if (info.hPatterns && patternHint.GetType()[HINT_MODTYPE | HINT_PATNAMES])
	{
		const PATTERNINDEX nPat = patternHint.GetPattern();
		PATTERNINDEX minPat = 0, maxPat = sndFile.Patterns.Size();
		if(patternHint.GetType()[HINT_PATNAMES] && nPat < sndFile.Patterns.Size())
		{
			minPat = nPat;
			maxPat = nPat + 1;
		}

		for(size_t pat = sndFile.Patterns.Size(); pat < info.tiPatterns.size(); pat++)
		{
			DeleteItem(info.tiPatterns[pat]);
		}
		info.tiPatterns.resize(sndFile.Patterns.Size(), nullptr);

		mpt::winstring patName;
		for(PATTERNINDEX pat = minPat; pat < maxPat; pat++)
		{
			if(sndFile.Patterns.IsValidPat(pat))
			{
				patName = mpt::ToWin(sndFile.GetCharsetInternal(), sndFile.Patterns[pat].GetName());
				wsprintf(s, _T("%u"), pat);
				if(!patName.empty())
				{
					_tcscat(s, _T(": "));
					_tcscat(s, patName.c_str());
				}
				if (info.tiPatterns[pat])
				{
					tvi.mask = TVIF_TEXT | TVIF_HANDLE;
					tvi.hItem = info.tiPatterns[pat];
					tvi.pszText = stmp;
					tvi.cchTextMax = CountOf(stmp);
					GetItem(&tvi);
					if (_tcscmp(s, stmp)) SetItem(info.tiPatterns[pat], TVIF_TEXT, s, 0, 0, 0, 0, 0);
				} else
				{
					info.tiPatterns[pat] = InsertItem(s, IMAGE_PATTERNS, IMAGE_PATTERNS, info.hPatterns, TVI_LAST);
				}
				SetItemData(info.tiPatterns[pat], pat);
			} else if(pat < info.tiPatterns.size() && info.tiPatterns[pat])
			{
				DeleteItem(info.tiPatterns[pat]);
				info.tiPatterns[pat] = nullptr;
			}
		}
	}
	// Add Samples
	const SampleHint sampleHint = hint.ToType<SampleHint>();
	if (info.hSamples && sampleHint.GetType()[HINT_MODTYPE | HINT_SMPNAMES | HINT_SAMPLEINFO | HINT_SAMPLEDATA])
	{
		const SAMPLEINDEX hintSmp = sampleHint.GetSample();
		SAMPLEINDEX smin = 1, smax = MAX_SAMPLES - 1;
		if (sampleHint.GetType()[HINT_SMPNAMES | HINT_SAMPLEINFO | HINT_SAMPLEDATA] && hintSmp > 0 && hintSmp < MAX_SAMPLES)
		{
			smin = smax = hintSmp;
		}
		HTREEITEM hChild = GetNthChildItem(info.hSamples, smin - 1);
		for(SAMPLEINDEX nSmp = smin; nSmp <= smax; nSmp++)
		{
			HTREEITEM hNextChild = GetNextSiblingItem(hChild);
			if (nSmp <= sndFile.GetNumSamples())
			{
				const ModSample &sample = sndFile.GetSample(nSmp);
				const bool sampleExists = (sample.HasSampleData());

				static constexpr int Images[] =
				{
					IMAGE_NOSAMPLE,  IMAGE_NOSAMPLE,        IMAGE_NOSAMPLE,
					IMAGE_SAMPLES,   IMAGE_SAMPLEACTIVE,    IMAGE_SAMPLEMUTE,
					IMAGE_EXTSAMPLE, IMAGE_EXTSAMPLEACTIVE, IMAGE_EXTSAMPLEMUTE,
					IMAGE_OPLINSTR,  IMAGE_OPLINSTRACTIVE,  IMAGE_OPLINSTRMUTE,
				};

				int image = 0;
				if(sampleExists)
					image = 3;
				if(sample.uFlags[SMP_KEEPONDISK])
					image = 6;
				if(sample.uFlags[CHN_ADLIB])
					image = 9;

				if(info.modDoc.IsSampleMuted(nSmp))
					image += 2;
				else if(info.samplesPlaying[nSmp])
					image++;

				if(sample.uFlags[SMP_KEEPONDISK] && !sampleExists) 
					image = IMAGE_EXTSAMPLEMISSING;
				else
					image = Images[image];

				if(sndFile.GetType() == MOD_TYPE_MPT)
				{
					const TCHAR *status = _T("");
					if(sample.uFlags[SMP_KEEPONDISK])
					{
						status = sampleExists ? _T(" [external]") : _T(" [MISSING]");
					}
					wsprintf(s, _T("%3d: %s%s%s"), nSmp, sample.uFlags.test_all(SMP_MODIFIED | SMP_KEEPONDISK) ? _T("* ") : _T(""), mpt::ToCString(sndFile.GetCharsetInternal(), sndFile.m_szNames[nSmp]).GetString(), status);
				} else
				{
					wsprintf(s, _T("%3d: %s"), nSmp, mpt::ToCString(sndFile.GetCharsetInternal(), sndFile.m_szNames[nSmp]).GetString());
				}

				if (!hChild)
				{
					hChild = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, image, image, 0, 0, nSmp, info.hSamples, TVI_LAST);
				} else
				{
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
					tvi.hItem = hChild;
					tvi.pszText = stmp;
					tvi.cchTextMax = CountOf(stmp);
					tvi.iImage = tvi.iSelectedImage = image;
					GetItem(&tvi);
					if(tvi.iImage != image || _tcscmp(s, stmp) || GetItemData(hChild) != nSmp)
					{
						SetItem(hChild, TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, image, image, 0, 0, nSmp);
					}
				}
			} else if(hChild != nullptr)
			{
				DeleteItem(hChild);
			} else
			{
				break;
			}
			hChild = hNextChild;
		}
	}
	// Add Instruments
	const InstrumentHint instrHint = hint.ToType<InstrumentHint>();
	if (info.hInstruments && instrHint.GetType()[HINT_MODTYPE | HINT_INSNAMES | HINT_INSTRUMENT])
	{
		INSTRUMENTINDEX smin = 1, smax = MAX_INSTRUMENTS - 1;
		const INSTRUMENTINDEX hintIns = instrHint.GetInstrument();
		if (instrHint.GetType()[HINT_INSNAMES | HINT_INSTRUMENT] && hintIns > 0 && hintIns < MAX_INSTRUMENTS)
		{
			smin = smax = hintIns;
		}
		HTREEITEM hChild = GetNthChildItem(info.hInstruments, smin - 1);
		for (INSTRUMENTINDEX nIns = smin; nIns <= smax; nIns++)
		{
			HTREEITEM hNextChild = GetNextSiblingItem(hChild);
			if (nIns <= sndFile.GetNumInstruments())
			{
				wsprintf(s, _T("%3u: %s"), nIns, mpt::ToCString(sndFile.GetCharsetInternal(), sndFile.GetInstrumentName(nIns)).GetString());

				int nImage = IMAGE_INSTRUMENTS;
				if(info.instrumentsPlaying[nIns]) nImage = IMAGE_INSTRACTIVE;
				if(!sndFile.Instruments[nIns] || info.modDoc.IsInstrumentMuted(nIns)) nImage = IMAGE_INSTRMUTE;

				if (!hChild)
				{
					hChild = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, nImage, nImage, 0, 0, nIns, info.hInstruments, TVI_LAST);
				} else
				{
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
					tvi.hItem = hChild;
					tvi.pszText = stmp;
					tvi.cchTextMax = CountOf(stmp);
					tvi.iImage = tvi.iSelectedImage = nImage;
					GetItem(&tvi);
					if(tvi.iImage != nImage || _tcscmp(s, stmp) || GetItemData(hChild) != nIns)
					{
						SetItem(hChild, TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, s, nImage, nImage, 0, 0, nIns);
					}
				}
			} else if(hChild != nullptr)
			{
				DeleteItem(hChild);
			} else
			{
				break;
			}
			hChild = hNextChild;
		}
	}
}


CModTree::ModItem CModTree::GetModItem(HTREEITEM hItem)
{
	if (!hItem) return ModItem(MODITEM_NULL);
	// First, test root items
	if (hItem == m_hInsLib) return ModItem(MODITEM_HDR_INSTRUMENTLIB);
	if (hItem == m_hMidiLib) return ModItem(MODITEM_HDR_MIDILIB);

	// The immediate parent of the item (NULL if this item is on the root level of the tree)
	HTREEITEM hItemParent = GetParentItem(hItem);
	// Parent of the parent.
	HTREEITEM hItemParentParent = GetParentItem(hItemParent);
	// Get the root parent of the selected item, which can be the item itself.
	HTREEITEM hRootParent = hItem;
	if(!IsSampleBrowser())
	{
		hRootParent = GetParentRootItem(hItem);
	}

	uint32 itemData = static_cast<uint32>(GetItemData(hItem));
	uint32 rootItemData = static_cast<uint32>(GetItemData(hRootParent));

	// Midi Library
	if(hRootParent == m_hMidiLib && hRootParent != hItem && !IsSampleBrowser())
	{
		return ModItem(static_cast<ModItemType>(itemData >> MIDILIB_SHIFT), itemData & MIDILIB_MASK);
	}
	// Instrument Library
	if(hRootParent == m_hInsLib || (IsSampleBrowser() && hItem != m_hInsLib))
	{
		TVITEM tvi;
		tvi.mask = TVIF_IMAGE|TVIF_HANDLE;
		tvi.hItem = hItem;
		tvi.iImage = 0;
		if (GetItem(&tvi))
		{
			switch(tvi.iImage)
			{
			case IMAGE_SAMPLES:
			case IMAGE_OPLINSTR:
				// Sample
				return ModItem(MODITEM_INSLIB_SAMPLE);
			case IMAGE_INSTRUMENTS:
				// Instrument
				return ModItem(MODITEM_INSLIB_INSTRUMENT);
			case IMAGE_FOLDERSONG:
				// Song
				return ModItem(MODITEM_INSLIB_SONG);
			default:
				return ModItem(MODITEM_INSLIB_FOLDER);
			}
		}
		return ModItem(MODITEM_NULL);
	}
	if (IsSampleBrowser()) return ModItem(MODITEM_NULL);
	// Songs
	if(rootItemData < DocInfo.size())
	{
		m_nDocNdx = rootItemData;
		auto &pInfo = DocInfo[rootItemData];

		if(hItem == pInfo->hSong) return ModItem(MODITEM_HDR_SONG);
		if(hRootParent == pInfo->hSong)
		{
			if (hItem == pInfo->hPatterns) return ModItem(MODITEM_HDR_PATTERNS);
			if (hItem == pInfo->hOrders) return ModItem(MODITEM_HDR_ORDERS);
			if (hItem == pInfo->hSamples) return ModItem(MODITEM_HDR_SAMPLES);
			if (hItem == pInfo->hInstruments) return ModItem(MODITEM_HDR_INSTRUMENTS);
			if (hItem == pInfo->hComments) return ModItem(MODITEM_COMMENTS);
			// Order List or Sequence item?
			if ((hItemParent == pInfo->hOrders) || (hItemParentParent == pInfo->hOrders))
			{
				const ORDERINDEX ord = (ORDERINDEX)(itemData & SEQU_MASK);
				const SEQUENCEINDEX seq = (SEQUENCEINDEX)(itemData >> SEQU_SHIFT);
				if(ord == ORDERINDEX_INVALID)
				{
					return ModItem(MODITEM_SEQUENCE, seq);
				} else
				{
					return ModItem(MODITEM_ORDER, ord, seq);
				}
			}

			ModItem modItem(MODITEM_NULL, itemData);
			if (hItemParent == pInfo->hPatterns)
			{
				// Pattern
				modItem.type = MODITEM_PATTERN;
			} else if (hItemParent == pInfo->hSamples)
			{
				// Sample
				modItem.type = MODITEM_SAMPLE;
			} else if (hItemParent == pInfo->hInstruments)
			{
				// Instrument
				modItem.type = MODITEM_INSTRUMENT;
			} else if (hItemParent == pInfo->hEffects)
			{
				// Effect
				modItem.type = MODITEM_EFFECT;
			}
			return modItem;
		}
	}

	// DLS banks
	if(itemData < m_tiDLS.size() && hItem == m_tiDLS[itemData])
		return ModItem(MODITEM_DLSBANK_FOLDER, itemData);

	// DLS Instruments
	if(hRootParent != nullptr)
	{
		if(rootItemData < m_tiDLS.size() && m_tiDLS[rootItemData] == hRootParent)
		{
			if (hItem == m_tiDLS[rootItemData])
				return ModItem(MODITEM_DLSBANK_FOLDER, (uint32)rootItemData);

			if ((itemData & DLS_TYPEMASK) == DLS_TYPEPERC
				|| (itemData & DLS_TYPEMASK) == DLS_TYPEINST)
			{
				return DlsItem(rootItemData, itemData);
			}
		}
	}
	return ModItem(MODITEM_NULL);
}


BOOL CModTree::ExecuteItem(HTREEITEM hItem)
{
	if (hItem)
	{
		const ModItem modItem = GetModItem(hItem);
		uint32 modItemID = modItem.val1;
		CModDoc *modDoc = m_nDocNdx < DocInfo.size() ? &(DocInfo[m_nDocNdx]->modDoc) : nullptr;

		switch(modItem.type)
		{
		case MODITEM_COMMENTS:
			if (modDoc) modDoc->ActivateView(IDD_CONTROL_COMMENTS, 0);
			return TRUE;

		/*case MODITEM_SEQUENCE:
			if (modDoc) modDoc->ActivateView(IDD_CONTROL_PATTERNS, (dwItem << SEQU_SHIFT) | SEQU_INDICATOR);
			return TRUE;*/

		case MODITEM_ORDER:
			if (modDoc) modDoc->ActivateView(IDD_CONTROL_PATTERNS, modItemID | (uint32(modItem.val2) << SEQU_SHIFT) | SEQU_INDICATOR);
			return TRUE;

		case MODITEM_PATTERN:
			if (modDoc) modDoc->ActivateView(IDD_CONTROL_PATTERNS, modItemID);
			return TRUE;

		case MODITEM_SAMPLE:
			if (modDoc) modDoc->ActivateView(IDD_CONTROL_SAMPLES, modItemID);
			return TRUE;

		case MODITEM_INSTRUMENT:
			if (modDoc) modDoc->ActivateView(IDD_CONTROL_INSTRUMENTS, modItemID);
			return TRUE;

		case MODITEM_MIDIPERCUSSION:
			modItemID |= 0x80;
		case MODITEM_MIDIINSTRUMENT:
			OpenMidiInstrument(modItemID);
			return TRUE;

		case MODITEM_EFFECT:
		case MODITEM_INSLIB_SAMPLE:
		case MODITEM_INSLIB_INSTRUMENT:
			PlayItem(hItem, NOTE_MIDDLEC);
			return TRUE;

		case MODITEM_INSLIB_SONG:
		case MODITEM_INSLIB_FOLDER:
			InstrumentLibraryChDir(mpt::PathString::FromCString(GetItemText(hItem)), modItem.type == MODITEM_INSLIB_SONG);
			return TRUE;

		case MODITEM_HDR_SONG:
			if (modDoc) modDoc->ActivateWindow();
			return TRUE;

		case MODITEM_DLSBANK_INSTRUMENT:
			PlayItem(hItem, NOTE_MIDDLEC);
			return TRUE;

		case MODITEM_HDR_INSTRUMENTLIB:
			if(IsSampleBrowser())
			{
				BrowseForFolder dlg(m_InstrLibPath, _T("Select a new instrument library folder..."));
				if(dlg.Show())
				{
					mpt::PathString dir = dlg.GetDirectory();
					dir.EnsureTrailingSlash();
					CMainFrame::GetMainFrame()->GetUpperTreeview()->InstrumentLibraryChDir(dir, false);
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}


void CModTree::PlayDLSItem(CDLSBank &dlsBank, const DlsItem &item, ModCommand::NOTE note)
{
	UINT rgn = 0, instr = item.GetInstr();
	if(item.IsPercussion())
	{
		// Drum
		rgn = item.GetRegion();
	} else if(item.IsInstr())
	{
		// Melodic
		rgn = dlsBank.GetRegionFromKey(instr, note - NOTE_MIN);
	}
	CMainFrame::GetMainFrame()->PlayDLSInstrument(dlsBank, instr, rgn, note);
}


BOOL CModTree::PlayItem(HTREEITEM hItem, ModCommand::NOTE note, int volume)
{
	if (hItem)
	{
		const ModItem modItem = GetModItem(hItem);
		uint32 modItemID = modItem.val1;
		CModDoc *modDoc = m_nDocNdx < DocInfo.size() ? &(DocInfo[m_nDocNdx]->modDoc) : nullptr;

		switch(modItem.type)
		{
		case MODITEM_SAMPLE:
			if (modDoc)
			{
				if (note == NOTE_NOTECUT)
				{
					modDoc->NoteOff(0, true); // cut previous playing samples
				} else if (note & 0x80)
				{
					modDoc->NoteOff(note & 0x7F, true);
				} else
				{
					modDoc->NoteOff(0, true); // cut previous playing samples
					modDoc->PlayNote(PlayNoteParam(note & 0x7F).Sample(static_cast<SAMPLEINDEX>(modItemID)).Volume(volume));
				}
			}
			return TRUE;

		case MODITEM_INSTRUMENT:
			if (modDoc)
			{
				if (note == NOTE_NOTECUT)
				{
					modDoc->NoteOff(0, true);
				} else if (note & 0x80)
				{
					modDoc->NoteOff(note & 0x7F, true);
				} else
				{
					modDoc->NoteOff(0, true);
					modDoc->PlayNote(PlayNoteParam(note & 0x7F).Instrument(static_cast<INSTRUMENTINDEX>(modItemID)).Volume(volume));
				}
			}
			return TRUE;

		case MODITEM_EFFECT:
			if ((modDoc) && (modItemID < MAX_MIXPLUGINS))
			{
				modDoc->TogglePluginEditor(modItemID);
			}
			return TRUE;

		case MODITEM_INSLIB_SAMPLE:
		case MODITEM_INSLIB_INSTRUMENT:
			if(note != NOTE_NOTECUT)
			{
				CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
				if(!m_SongFileName.empty())
				{
					// Preview sample / instrument in module
					const size_t n = ConvertStrTo<size_t>(GetItemText(hItem));
					if (pMainFrm && m_SongFile)
					{
						if (modItem.type == MODITEM_INSLIB_INSTRUMENT)
						{
							pMainFrm->PlaySoundFile(*m_SongFile, static_cast<INSTRUMENTINDEX>(n), SAMPLEINDEX_INVALID, note, volume);
						} else
						{
							pMainFrm->PlaySoundFile(*m_SongFile, INSTRUMENTINDEX_INVALID, static_cast<SAMPLEINDEX>(n), note, volume);
						}
					}
				} else
				{
					// Preview sample / instrument file
					if (pMainFrm) pMainFrm->PlaySoundFile(InsLibGetFullPath(hItem), note, volume);
				}
			} else
			{
				CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
				if (pMainFrm) pMainFrm->StopPreview();
			}

			return TRUE;

		case MODITEM_MIDIPERCUSSION:
			modItemID |= 0x80;
		case MODITEM_MIDIINSTRUMENT:
			{
				const MidiLibrary &midiLib = CTrackApp::GetMidiLibrary();
				if(modItemID < midiLib.size() && !midiLib[modItemID].empty())
				{
					CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
					CDLSBank *dlsBank = nullptr;
					if(!mpt::PathString::CompareNoCase(m_cachedBankName, midiLib[modItemID]))
					{
						dlsBank = m_cachedBank.get();
					}
					if(dlsBank == nullptr && CDLSBank::IsDLSBank(midiLib[modItemID]))
					{
						m_cachedBank = std::make_unique<CDLSBank>();
						if(m_cachedBank->Open(midiLib[modItemID]))
						{
							m_cachedBankName = midiLib[modItemID];
							dlsBank = m_cachedBank.get();
						}
					}
					if(dlsBank != nullptr)
					{
						uint32 item;
						if(modItemID < 0x80)
						{
							item = modItemID | DLS_TYPEINST;
						} else
						{
							dlsBank->FindInstrument(true, 0xFFFF, 0xFF, modItemID & 0x7F, &item);
							item |= dlsBank->GetRegionFromKey(item, modItemID & 0x7F) << DLS_REGIONSHIFT;
							item |= DLS_TYPEPERC;
						}
						PlayDLSItem(*dlsBank, DlsItem(0, item), note);
					} else
					{
						pMainFrm->PlaySoundFile(midiLib[modItemID], note, volume);
					}
				}
			}
			return TRUE;

		case MODITEM_DLSBANK_INSTRUMENT:
			{
				const DlsItem &item = *static_cast<const DlsItem *>(&modItem);
				CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
				uint32 bank = item.GetBankIndex();
				if ((bank < CTrackApp::gpDLSBanks.size()) && (CTrackApp::gpDLSBanks[bank]) && (pMainFrm))
				{
					PlayDLSItem(*CTrackApp::gpDLSBanks[bank], item, note);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


BOOL CModTree::SetMidiInstrument(UINT nIns, const mpt::PathString &fileName)
{
	MidiLibrary &midiLib = CTrackApp::GetMidiLibrary();
	if(nIns < 128)
	{
		midiLib[nIns] = fileName;
		RefreshMidiLibrary();
		return TRUE;
	}
	return FALSE;
}


BOOL CModTree::SetMidiPercussion(UINT nPerc, const mpt::PathString &fileName)
{
	MidiLibrary &midiLib = CTrackApp::GetMidiLibrary();
	if(nPerc < 128)
	{
		UINT nIns = nPerc | 0x80;
		midiLib[nIns] = fileName;
		RefreshMidiLibrary();
		return TRUE;
	}
	return FALSE;
}


void CModTree::DeleteTreeItem(HTREEITEM hItem)
{
	const ModItem modItem = GetModItem(hItem);
	uint32 modItemID = modItem.val1;
	TCHAR s[64];

	CModDoc *modDoc = m_nDocNdx < DocInfo.size() ? &(DocInfo[m_nDocNdx]->modDoc) : nullptr;
	if(modItem.IsSongItem() && modDoc == nullptr)
	{
		return;
	}

	switch(modItem.type)
	{
	case MODITEM_SEQUENCE:
		wsprintf(s, _T("Remove sequence %u?"), modItemID);
		if(Reporting::Confirm(s, false, true) == cnfNo) break;
		modDoc->GetSoundFile().Order.RemoveSequence((SEQUENCEINDEX)(modItemID));
		modDoc->UpdateAllViews(nullptr, SequenceHint().Data());
		break;

	case MODITEM_ORDER:
		// might be slightly annoying to ask for confirmation here, and it's rather easy to restore the orderlist anyway.
		if(modDoc->RemoveOrder((SEQUENCEINDEX)(modItem.val2), (ORDERINDEX)(modItem.val1)))
		{
			modDoc->UpdateAllViews(nullptr, SequenceHint().Data());
		}
		break;

	case MODITEM_PATTERN:
		{
			PATTERNINDEX pat = static_cast<PATTERNINDEX>(modItemID);
			bool isUsed = false;
			// First, find all used patterns in all sequences.
			for(const auto &sequence : modDoc->GetSoundFile().Order)
			{
				if(std::find(sequence.cbegin(), sequence.cend(), pat) != sequence.cend())
				{
					isUsed = true;
					break;
				}
			}
			wsprintf(s, _T("Remove pattern %u?\nThis pattern is currently%s used."), modItemID, isUsed ? _T("") : _T(" not"));
			if(Reporting::Confirm(s, false, isUsed) == cnfYes && modDoc->RemovePattern(pat))
			{
				modDoc->UpdateAllViews(nullptr, PatternHint(pat).Data().Names());
			}
		}
		break;

	case MODITEM_SAMPLE:
		wsprintf(s, _T("Remove sample %u?"), modItemID);
		if(!modDoc->GetSoundFile().GetSample(static_cast<SAMPLEINDEX>(modItemID)).HasSampleData() || Reporting::Confirm(s, false, true) == cnfYes)
		{
			modDoc->GetSampleUndo().PrepareUndo((SAMPLEINDEX)modItemID, sundo_replace, "Delete");
			const SAMPLEINDEX oldNumSamples = modDoc->GetNumSamples();
			if (modDoc->RemoveSample((SAMPLEINDEX)modItemID))
			{
				modDoc->UpdateAllViews(nullptr, SampleHint(modDoc->GetNumSamples() != oldNumSamples ? 0 : (SAMPLEINDEX)modItemID).Info().Data().Names());
			}
		}
		break;

	case MODITEM_INSTRUMENT:
		wsprintf(s, _T("Remove instrument %u?"), modItemID);
		if(Reporting::Confirm(s, false, true) == cnfYes)
		{
			modDoc->GetInstrumentUndo().PrepareUndo((INSTRUMENTINDEX)modItemID, "Delete");
			const INSTRUMENTINDEX oldNumInstrs = modDoc->GetNumInstruments();
			if(modDoc->RemoveInstrument((INSTRUMENTINDEX)modItemID))
			{
				modDoc->UpdateAllViews(nullptr, InstrumentHint(modDoc->GetNumInstruments() != oldNumInstrs ? 0 : INSTRUMENTINDEX(modItemID)).Info().Envelope().ModType());
			}
		}
		break;

	case MODITEM_MIDIINSTRUMENT:
		SetMidiInstrument(modItemID, P_(""));
		RefreshMidiLibrary();
		break;
	case MODITEM_MIDIPERCUSSION:
		SetMidiPercussion(modItemID, P_(""));
		RefreshMidiLibrary();
		break;

	case MODITEM_DLSBANK_FOLDER:
		CTrackApp::RemoveDLSBank(modItemID);
		RefreshDlsBanks();
		break;

	case MODITEM_INSLIB_SONG:
	case MODITEM_INSLIB_SAMPLE:
	case MODITEM_INSLIB_INSTRUMENT:
		{
			// Create double-null-terminated path
			const mpt::winstring fullPath = InsLibGetFullPath(hItem).AsNative() + _T('\0');
			SHFILEOPSTRUCT fos;
			MemsetZero(fos);
			fos.hwnd = m_hWnd;
			fos.wFunc = FO_DELETE;
			fos.pFrom = fullPath.c_str();
			fos.fFlags = CMainFrame::GetInputHandler()->ShiftPressed() ? 0 : FOF_ALLOWUNDO;
			if(!SHFileOperation(&fos) && !fos.fAnyOperationsAborted)
			{
				HTREEITEM newSel = GetNextSiblingItem(hItem);
				if(!newSel) newSel = GetPrevSiblingItem(hItem);
				SelectItem(newSel);
				RefreshInstrumentLibrary();
				SetFocus();
			}
		}
		break;
	}
}


BOOL CModTree::OpenTreeItem(HTREEITEM hItem)
{
	const ModItem modItem = GetModItem(hItem);

	switch(modItem.type)
	{
	case MODITEM_INSLIB_SONG:
		theApp.OpenDocumentFile(InsLibGetFullPath(hItem).ToCString());
		break;
	case MODITEM_HDR_INSTRUMENTLIB:
		CTrackApp::OpenDirectory(m_InstrLibPath);
		break;
	case MODITEM_INSLIB_FOLDER:
		// Open path in Explorer
		CTrackApp::OpenDirectory(InsLibGetFullPath(hItem));
		break;
	}
	return TRUE;
}


BOOL CModTree::OpenMidiInstrument(DWORD dwItem)
{
	std::vector<FileType> mediaFoundationTypes = CSoundFile::GetMediaFoundationFileTypes();
	FileDialog dlg = OpenFileDialog()
		.EnableAudioPreview()
		.ExtensionFilter(
			"All Instruments and Banks|*.xi;*.pat;*.iti;*.sfz;*.wav;*.w64;*.caf;*.aif;*.aiff;*.sf2;*.sbk;*.dls;*.mss;*.flac;*.opus;*.ogg;*.oga;*.mp1;*.mp2;*.mp3" + ToFilterOnlyString(mediaFoundationTypes, true).ToLocale() + "|"
			"FastTracker II Instruments (*.xi)|*.xi|"
			"GF1 Patches (*.pat)|*.pat|"
			"Wave Files (*.wav)|*.wav|"
			"Wave64 Files (*.w64)|*.w64|"
			"CAF Files (*.caf)|*.caf|"
	#ifdef MPT_WITH_FLAC
			"FLAC Files (*.flac,*.oga)|*.flac;*.oga|"
	#endif // MPT_WITH_FLAC
	#if defined(MPT_WITH_OPUSFILE)
			"Opus Files (*.opus,*.oga)|*.opus;*.oga|"
	#endif // MPT_WITH_OPUSFILE
	#if defined(MPT_WITH_VORBISFILE) || defined(MPT_WITH_STBVORBIS)
			"Ogg Vorbis Files (*.ogg,*.oga)|*.ogg;*.oga|"
	#endif // VORBIS
	#if defined(MPT_ENABLE_MP3_SAMPLES)
			"MPEG Files (*.mp1,*.mp2,*.mp3)|*.mp1;*.mp2;*.mp3|"
	#endif // MPT_ENABLE_MP3_SAMPLES
	#if defined(MPT_WITH_MEDIAFOUNDATION)
			+ ToFilterString(mediaFoundationTypes, FileTypeFormatShowExtensions).ToLocale() +
	#endif
			"Impulse Tracker Instruments (*.iti)|*.iti;*.its|"
			"SFZ Instruments (*.sfz)|*.sfz|"
			"SoundFont 2.0 Banks (*.sf2)|*.sf2;*.sbk|"
			"DLS Sound Banks (*.dls;*.mss)|*.dls;*.mss|"
			"All Files (*.*)|*.*||");
	if(!dlg.Show()) return FALSE;

	if (dwItem & 0x80)
		return SetMidiPercussion(dwItem & 0x7F, dlg.GetFirstFile());
	else
		return SetMidiInstrument(dwItem, dlg.GetFirstFile());
}


// Empty Instrument Library
void CModTree::EmptyInstrumentLibrary()
{
	HTREEITEM h;
	if (!m_hInsLib) return;
	if (!IsSampleBrowser())
	{
		DeleteChildren(m_hInsLib);
	} else
	{
		while ((h = GetNextItem(m_hInsLib, TVGN_NEXT)) != NULL)
		{
			DeleteItem(h);
		}
	}
}


// Refresh Instrument Library
void CModTree::FillInstrumentLibrary(const TCHAR *selectedItem)
{
	if (!m_hInsLib) return;

	SetRedraw(FALSE);
	if(!m_SongFileName.empty() && IsSampleBrowser() && m_SongFile)
	{
		// Fill browser with samples / instruments of module file
		SetItemText(m_hInsLib, m_SongFileName.AsNative().c_str());
		SetItemImage(m_hInsLib, IMAGE_FOLDERSONG, IMAGE_FOLDERSONG);
		for(INSTRUMENTINDEX ins = 1; ins <= m_SongFile->GetNumInstruments(); ins++)
		{
			ModInstrument *pIns = m_SongFile->Instruments[ins];
			if(pIns)
			{
				TCHAR s[MAX_INSTRUMENTNAME + 10];
				_sntprintf(s, CountOf(s), _T("%3d: %s"), ins, mpt::ToWin(m_SongFile->GetCharsetInternal(), pIns->name).c_str());
				ModTreeInsert(s, IMAGE_INSTRUMENTS, selectedItem);
			}
		}
		for(SAMPLEINDEX smp = 1; smp <= m_SongFile->GetNumSamples(); smp++)
		{
			const ModSample &sample = m_SongFile->GetSample(smp);
			if(sample.HasSampleData() || sample.uFlags[CHN_ADLIB])
			{
				TCHAR s[MAX_SAMPLENAME + 10];
				_sntprintf(s, CountOf(s), _T("%3d: %s"), smp, mpt::ToWin(m_SongFile->GetCharsetInternal(), m_SongFile->m_szNames[smp]).c_str());
				ModTreeInsert(s, sample.uFlags[CHN_ADLIB] ? IMAGE_OPLINSTR : IMAGE_SAMPLES, selectedItem);
			}
		}
	} else
	{
		if(!IsSampleBrowser())
		{
			SetItemText(m_hInsLib, _T("Instrument Library (") + m_InstrLibPath.ToCString() + _T(")"));
		} else
		{
			SetItemText(m_hInsLib, m_InstrLibPath.ToCString());
			SetItemImage(m_hInsLib, IMAGE_FOLDER, IMAGE_FOLDER);
		}

		// Enumerating Drives...
		if(!IsSampleBrowser())
		{
			CImageList &images = CMainFrame::GetMainFrame()->m_MiscIcons;
			// Avoid adding the same images again and again...
			images.SetImageCount(IMGLIST_NUMIMAGES);

			TCHAR s[16];
			_tcscpy(s, _T("?:\\"));
			for(UINT iDrive = 'A'; iDrive <= 'Z'; iDrive++)
			{
				s[0] = (TCHAR)iDrive;
				UINT nDriveType = GetDriveType(s);
				if(nDriveType != DRIVE_UNKNOWN && nDriveType != DRIVE_NO_ROOT_DIR)
				{
					SHFILEINFO fileInfo;
					SHGetFileInfo(s, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_SMALLICON);
					ModTreeInsert(s, images.Add(fileInfo.hIcon), selectedItem);
					DestroyIcon(fileInfo.hIcon);
				}
			}
		}

		// Enumerating Directories and samples/instruments
		const mpt::PathString path = m_InstrLibPath + P_("*.*");
		const bool showDirs = !IsSampleBrowser() || TrackerSettings::Instance().showDirsInSampleBrowser, showInstrs = IsSampleBrowser();

		HANDLE hFind;
		WIN32_FIND_DATA wfd;
		MemsetZero(wfd);
		if((hFind = FindFirstFile(path.AsNative().c_str(), &wfd)) != INVALID_HANDLE_VALUE)
		{
			auto modExts = CSoundFile::GetSupportedExtensions(false);
			auto instrExts = { "xi", "iti", "sfz", "sf2", "sbk", "dls", "mss", "pat" };
			auto sampleExts = { "wav", "flac", "ogg", "opus", "mp1", "mp2", "mp3", "smp", "raw", "s3i", "its", "aif", "aiff", "au", "snd", "svx", "voc", "8sv", "8svx", "16sv", "16svx", "w64", "caf", "sb0", "sb2", "sbi" };
			auto allExtsBlacklist = { "txt", "diz", "nfo", "doc", "ini", "pdf", "zip", "rar", "lha", "exe", "dll", "mol" };

			do
			{
				// Up Directory
				if(!_tcscmp(wfd.cFileName, _T("..")))
				{
					if(showDirs)
					{
						ModTreeInsert(wfd.cFileName, IMAGE_FOLDERPARENT, selectedItem);
					}
				} else if (wfd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_SYSTEM))
				{
					// Ignore these files
					continue;
				} else if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Directory
					if(_tcscmp(wfd.cFileName, _T(".")) && showDirs)
					{
						ModTreeInsert(wfd.cFileName, IMAGE_FOLDER, selectedItem);
					}
				} else if(wfd.nFileSizeHigh > 0 || wfd.nFileSizeLow >= 16)
				{
					// Get lower-case file extension without dot.
					mpt::PathString extPS = mpt::PathString::FromNative(wfd.cFileName).GetFileExt();
					std::string ext = extPS.ToUTF8();
					if(!ext.empty())
					{
						ext.erase(0, 1);
						ext = mpt::ToLowerCaseAscii(ext);
						extPS = mpt::PathString::FromUTF8(ext);
					}
					// Amiga-style extensions (i.e. mod.songname)
					std::string prefixExt = mpt::ToCharset(mpt::CharsetUTF8, mpt::winstring(wfd.cFileName));
					auto dotPos = prefixExt.find('.');
					if(dotPos != std::string::npos)
						prefixExt.erase(dotPos);
					else
						prefixExt.clear();

					if(std::find(instrExts.begin(), instrExts.end(), ext) != instrExts.end())
					{
						// Instruments
						if(showInstrs)
						{
							ModTreeInsert(wfd.cFileName, IMAGE_INSTRUMENTS, selectedItem);
						}
					} else if(std::find(sampleExts.begin(), sampleExts.end(), ext) != sampleExts.end())
					{
						// Samples
						if(showInstrs)
						{
							ModTreeInsert(wfd.cFileName, IMAGE_SAMPLES, selectedItem);
						}
					} else if(std::find(modExts.begin(), modExts.end(), ext) != modExts.end() || std::find(modExts.begin(), modExts.end(), prefixExt) != modExts.end())
					{
						// Songs
						if(showDirs)
						{
							ModTreeInsert(wfd.cFileName, IMAGE_FOLDERSONG, selectedItem);
						}
					} else if((!extPS.empty() && std::find(m_MediaFoundationExtensions.begin(), m_MediaFoundationExtensions.end(), extPS) != m_MediaFoundationExtensions.end())
						|| (m_showAllFiles && std::find(allExtsBlacklist.begin(), allExtsBlacklist.end(), ext) == allExtsBlacklist.end()))
					{
						// MediaFoundation samples / other files
						if(showInstrs)
						{
							ModTreeInsert(wfd.cFileName, IMAGE_SAMPLES, selectedItem);
						}
					}
				}
			} while (FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
	}
	
	// Sort items
	TVSORTCB tvs;
	tvs.hParent = (!IsSampleBrowser()) ? m_hInsLib : TVI_ROOT;
	tvs.lpfnCompare = ModTreeInsLibCompareProc;
	tvs.lParam = (LPARAM)this;
	SortChildren(tvs.hParent);
	SortChildrenCB(&tvs);
	SetRedraw(TRUE);

	{
		MPT_LOCK_GUARD<mpt::mutex> l(m_WatchDirMutex);
		if(m_InstrLibPath != m_WatchDir)
		{
			m_WatchDir = m_InstrLibPath;
			SetEvent(m_hSwitchWatchDir);
		}
	}
}


// Monitor changes in the instrument library folder.
void CModTree::MonitorInstrumentLibrary()
{
	mpt::SetCurrentThreadPriority(mpt::ThreadPriorityLowest);
	mpt::log::Trace::SetThreadId(mpt::log::Trace::ThreadKindWatchdir, GetCurrentThreadId());
	DWORD result;
	mpt::PathString lastWatchDir;
	HANDLE hWatchDir = INVALID_HANDLE_VALUE;
	DWORD lastRefresh = GetTickCount();
	DWORD timeout = INFINITE;
	DWORD interval = TrackerSettings::Instance().FSUpdateInterval;
	do
	{
		{
			MPT_LOCK_GUARD<mpt::mutex> l(m_WatchDirMutex);
			if(m_WatchDir != lastWatchDir)
			{
				if(hWatchDir != INVALID_HANDLE_VALUE)
				{
					FindCloseChangeNotification(hWatchDir);
					hWatchDir = INVALID_HANDLE_VALUE;
					lastWatchDir = mpt::PathString();
				}
				if(!m_WatchDir.empty())
				{
					hWatchDir = FindFirstChangeNotification(m_WatchDir.AsNative().c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
					lastWatchDir = m_WatchDir;
				}
			}
		}
		const HANDLE waitHandles[3] = { m_hWatchDirKillThread, m_hSwitchWatchDir, hWatchDir };
		result = WaitForMultipleObjects(hWatchDir != INVALID_HANDLE_VALUE ? 3 : 2, waitHandles, FALSE, timeout);
		DWORD now = GetTickCount();
		if(result == WAIT_TIMEOUT)
		{
			PostMessage(WM_COMMAND, ID_MODTREE_REFRESHINSTRLIB);
			lastRefresh = now;
			timeout = INFINITE;
		} else if(result == WAIT_OBJECT_0 + 1)
		{
			// nothing
			// will switch to new dir in next loop iteration
		} else if(result == WAIT_OBJECT_0 + 2)
		{
			FindNextChangeNotification(hWatchDir);
			timeout = 0; // update timeout later	
		}
		if(timeout != INFINITE)
		{
			// Update timeout. Can happen either because we just got a change event,
			// or because we got a directory switch event and still have a change
			// notification pending.
			if(now - lastRefresh >= interval)
			{
				PostMessage(WM_COMMAND, ID_MODTREE_REFRESHINSTRLIB);
				lastRefresh = now;
				timeout = INFINITE;
			} else
			{
				timeout = lastRefresh + interval - now;
			}
		}
	} while(result != WAIT_OBJECT_0);
	if(hWatchDir != INVALID_HANDLE_VALUE)
	{
		FindCloseChangeNotification(hWatchDir);
		hWatchDir = INVALID_HANDLE_VALUE;
		lastWatchDir = mpt::PathString();
	}
}


// Insert sample browser item.
void CModTree::ModTreeInsert(const TCHAR *name, int image, const TCHAR *selectIfMatch)
{
	DWORD dwId = 0;
	switch(image)
	{
	case IMAGE_FOLDERPARENT:
		dwId = 1;
		break;
	case IMAGE_FOLDER:
		dwId = 2;
		break;
	case IMAGE_FOLDERSONG:
		dwId = 3;
		break;
	case IMAGE_SAMPLES:
	case IMAGE_OPLINSTR:
		if(!m_SongFileName.empty()) { dwId = 5; break; }
	case IMAGE_INSTRUMENTS:
		dwId = 4;
		break;
	}
	HTREEITEM item = InsertItem(TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT,
		name,
		image, image,
		0, 0,
		(LPARAM)dwId,
		(!IsSampleBrowser()) ? m_hInsLib : TVI_ROOT,
		TVI_LAST);
	if(selectIfMatch != nullptr && !_tcscmp(name, selectIfMatch))
	{
		SelectItem(item);
	}
}


int CALLBACK CModTree::ModTreeInsLibCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	lParam1 &= 0x7FFFFFFF;
	lParam2 &= 0x7FFFFFFF;
	return static_cast<int>(lParam1 - lParam2);
}


int CALLBACK CModTree::ModTreeDrumCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM pDLSBank)
{
	lParam1 &= 0x7FFFFFFF;
	lParam2 &= 0x7FFFFFFF;
	if ((lParam1 & 0xFF00FFFF) == (lParam2 & 0xFF00FFFF))
	{
		if(pDLSBank)
		{
			const DLSINSTRUMENT *pDlsIns = reinterpret_cast<CDLSBank *>(pDLSBank)->GetInstrument(lParam1 & 0xFFFF);
			lParam1 = (lParam1 >> 16) & 0xFF;
			lParam2 = (lParam2 >> 16) & 0xFF;
			if ((pDlsIns) && (lParam1 < (LONG)pDlsIns->nRegions) && (lParam2 < (LONG)pDlsIns->nRegions))
			{
				lParam1 = pDlsIns->Regions[lParam1].uKeyMin;
				lParam2 = pDlsIns->Regions[lParam2].uKeyMin;
			}
		}
	}
	return static_cast<int>(lParam1 - lParam2);
}


void CModTree::InstrumentLibraryChDir(mpt::PathString dir, bool isSong)
{
	if(dir.empty()) return;
	if(IsSampleBrowser())
	{
		CMainFrame::GetMainFrame()->GetUpperTreeview()->InstrumentLibraryChDir(dir, isSong);
		return;
	}

	BeginWaitCursor();

	bool ok = false;
	if(isSong)
	{
		ok = m_pDataTree->InsLibSetFullPath(m_InstrLibPath, dir);
		if(ok)
		{
			m_pDataTree->RefreshInstrumentLibrary();
			m_InstrLibHighlightPath = dir;
		}
	} else
	{
		if(dir == P_(".."))
		{
			// Go one dir up.
			mpt::winstring prevDir = m_InstrLibPath.GetPath().AsNative();
			mpt::winstring::size_type pos = prevDir.find_last_of(_T("\\/"), prevDir.length() - 2);
			if(pos != mpt::winstring::npos)
			{
				m_InstrLibHighlightPath = mpt::PathString::FromNative(prevDir.substr(pos + 1, prevDir.length() - pos - 2));	// Highlight previously accessed directory
				prevDir = prevDir.substr(0, pos + 1);
			}
			dir = mpt::PathString::FromNative(prevDir);
		} else
		{
			// Drives are formatted like "E:\", folders are just folder name without slash.
			do
			{
				if(!dir.HasTrailingSlash())
				{
					dir = m_InstrLibPath + dir;
					dir.EnsureTrailingSlash();
				}
				m_InstrLibHighlightPath = P_("..");	// Highlight first entry

				FolderScanner scan(dir, FolderScanner::kFilesAndDirectories);
				mpt::PathString name;
				if(scan.Next(name) && !scan.Next(name) && name.IsDirectory())
				{
					// There is only one directory and nothing else in the path,
					// so skip this directory and automatically descend further down into the tree.
					dir = name;
					dir.EnsureTrailingSlash();
					continue;
				}
			} while(false);
		}

		if(dir.IsDirectory())
		{
			m_SongFileName = P_("");
			delete m_SongFile;
			m_SongFile = nullptr;
			m_InstrLibPath = dir;
			PostMessage(WM_COMMAND, ID_MODTREE_REFRESHINSTRLIB);
			ok = true;
		}
	}
	EndWaitCursor();

	if(ok)
	{
		MPT_LOCK_GUARD<mpt::mutex> l(m_WatchDirMutex);
		m_WatchDir = mpt::PathString();
	} else
	{
		Reporting::Error(mpt::cformat(_T("Unable to browse to \"%1\""))(dir), _T("Instrument Library"));
	}
}


bool CModTree::GetDropInfo(DRAGONDROP &dropInfo, mpt::PathString &fullPath)
{
	dropInfo.pModDoc = (m_nDragDocNdx < DocInfo.size() ? &(DocInfo[m_nDragDocNdx]->modDoc) : nullptr);
	dropInfo.dwDropType = DRAGONDROP_NOTHING;
	dropInfo.dwDropItem = m_itemDrag.val1;
	dropInfo.lDropParam = 0;
	switch(m_itemDrag.type)
	{
	case MODITEM_ORDER:
		dropInfo.dwDropType = DRAGONDROP_ORDER;
		break;

	case MODITEM_PATTERN:
		dropInfo.dwDropType = DRAGONDROP_PATTERN;
		break;

	case MODITEM_SAMPLE:
		dropInfo.dwDropType = DRAGONDROP_SAMPLE;
		break;

	case MODITEM_INSTRUMENT:
		dropInfo.dwDropType = DRAGONDROP_INSTRUMENT;
		break;

	case MODITEM_SEQUENCE:
	case MODITEM_HDR_ORDERS:
		dropInfo.dwDropType = DRAGONDROP_SEQUENCE;
		break;

	case MODITEM_INSLIB_SAMPLE:
	case MODITEM_INSLIB_INSTRUMENT:
		if(!m_SongFileName.empty())
		{
			const uint32 n = ConvertStrTo<uint32>(GetItemText(m_hItemDrag));
			dropInfo.dwDropType = (m_itemDrag.type == MODITEM_INSLIB_SAMPLE) ? DRAGONDROP_SAMPLE : DRAGONDROP_INSTRUMENT;
			dropInfo.dwDropItem = n;
			dropInfo.pModDoc = nullptr;
			dropInfo.lDropParam = (LPARAM)m_SongFile;
		} else
		{
			fullPath = InsLibGetFullPath(m_hItemDrag);
			dropInfo.dwDropType = DRAGONDROP_SOUNDFILE;
			dropInfo.lDropParam = (LPARAM)&fullPath;
		}
		break;

	case MODITEM_MIDIPERCUSSION:
		dropInfo.dwDropItem |= 0x80;
	case MODITEM_MIDIINSTRUMENT:
		{
			MidiLibrary &midiLib = CTrackApp::GetMidiLibrary();
			if(!midiLib[dropInfo.dwDropItem & 0xFF].empty())
			{
				fullPath = midiLib[dropInfo.dwDropItem & 0xFF];
				dropInfo.dwDropType = DRAGONDROP_MIDIINSTR;
				dropInfo.lDropParam = (LPARAM)&fullPath;
			}
		}
		break;

	case MODITEM_INSLIB_SONG:
		fullPath = InsLibGetFullPath(m_hItemDrag);
		dropInfo.pModDoc = NULL;
		dropInfo.dwDropType = DRAGONDROP_SONG;
		dropInfo.dwDropItem = 0;
		dropInfo.lDropParam = (LPARAM)&fullPath;
		break;

	case MODITEM_DLSBANK_INSTRUMENT:
		{
			const DlsItem &item = *static_cast<const DlsItem *>(&m_itemDrag);
			ASSERT(item.IsInstr() || item.IsPercussion());
			dropInfo.dwDropType = DRAGONDROP_DLS;
			// dwDropItem = DLS Bank #
			dropInfo.dwDropItem = item.GetBankIndex();	// bank #
			// Melodic: (Instrument)
			// Drums:	(0x80000000) | (Region << 16) | (Instrument)
			dropInfo.lDropParam = (LPARAM)((m_itemDrag.val1 & (DLS_TYPEPERC | DLS_REGIONMASK | DLS_INSTRMASK)));
		}
		break;
	}
	return (dropInfo.dwDropType != DRAGONDROP_NOTHING);
}


bool CModTree::CanDrop(HTREEITEM hItem, bool bDoDrop)
{
	const ModItem modItemDrop = GetModItem(hItem);
	const uint32 modItemDropID = modItemDrop.val1;
	const uint32 modItemDragID = m_itemDrag.val1;

	const ModTreeDocInfo *pInfoDrag = (m_nDragDocNdx < DocInfo.size() ? DocInfo[m_nDragDocNdx].get() : nullptr);
	const ModTreeDocInfo *pInfoDrop = (m_nDocNdx < DocInfo.size() ? DocInfo[m_nDocNdx].get() : nullptr);
	CModDoc *pModDoc = (pInfoDrop) ? &pInfoDrop->modDoc : nullptr;
	CSoundFile *pSndFile = (pModDoc) ? &pModDoc->GetSoundFile() : nullptr;
	const bool sameModDoc = pInfoDrag && (pModDoc == &pInfoDrag->modDoc);

	switch(modItemDrop.type)
	{
	case MODITEM_ORDER:
	case MODITEM_SEQUENCE:
		if ((m_itemDrag.type == MODITEM_ORDER) && (pModDoc) && sameModDoc)
		{
			// drop an order somewhere
			if (bDoDrop)
			{
				SEQUENCEINDEX nSeqFrom = (SEQUENCEINDEX)m_itemDrag.val2, nSeqTo = (SEQUENCEINDEX)modItemDrop.val2;
				ORDERINDEX nOrdFrom = (ORDERINDEX)m_itemDrag.val1, nOrdTo = (ORDERINDEX)modItemDrop.val1;
				if(modItemDrop.type == MODITEM_SEQUENCE)
				{
					// drop on sequence -> attach
					nSeqTo = (SEQUENCEINDEX)modItemDrop.val1;
					nOrdTo = pSndFile->Order(nSeqTo).GetLengthTailTrimmed();
				}

				if (nSeqFrom != nSeqTo || nOrdFrom != nOrdTo)
				{
					if(pModDoc->MoveOrder(nOrdFrom, nOrdTo, true, false, nSeqFrom, nSeqTo) == true)
					{
						pModDoc->SetModified();
					}
				}
			}
			return true;
		}
		break;

	case MODITEM_HDR_ORDERS:
		// Drop your sequences here.
		// At the moment, only dropping sequences into another module is possible.
		if((m_itemDrag.type == MODITEM_SEQUENCE || m_itemDrag.type == MODITEM_HDR_ORDERS) && pSndFile && pInfoDrag && !sameModDoc)
		{
			if(bDoDrop && pInfoDrag != nullptr)
			{
				// copy mod sequence over.
				CSoundFile &dragSndFile = pInfoDrag->modDoc.GetSoundFile();
				const SEQUENCEINDEX nOrigSeq = (SEQUENCEINDEX)modItemDragID;
				const ModSequence &origSeq = dragSndFile.Order(nOrigSeq);

				if(pSndFile->GetModSpecifications().sequencesMax > 1)
				{
					pSndFile->Order.AddSequence(false);
				}
				else
				{
					if(Reporting::Confirm(_T("Replace the current orderlist?"), _T("Sequence import")) == cnfNo)
						return false;
				}
				pSndFile->Order().resize(std::min(pSndFile->GetModSpecifications().ordersMax, origSeq.GetLength()), pSndFile->Order.GetInvalidPatIndex());
				for(ORDERINDEX nOrd = 0; nOrd < std::min(pSndFile->GetModSpecifications().ordersMax, origSeq.GetLengthTailTrimmed()); nOrd++)
				{
					PATTERNINDEX nOrigPat = dragSndFile.Order(nOrigSeq)[nOrd];
					// translate pattern index
					if(nOrigPat == dragSndFile.Order.GetInvalidPatIndex())
						pSndFile->Order()[nOrd] = pSndFile->Order.GetInvalidPatIndex();
					else if(nOrigPat == dragSndFile.Order.GetIgnoreIndex() && pSndFile->GetModSpecifications().hasIgnoreIndex)
						pSndFile->Order()[nOrd] = pSndFile->Order.GetIgnoreIndex();
					else if(nOrigPat == dragSndFile.Order.GetIgnoreIndex() && !pSndFile->GetModSpecifications().hasIgnoreIndex)
						pSndFile->Order()[nOrd] = pSndFile->Order.GetInvalidPatIndex();
					else if(nOrigPat >= pSndFile->GetModSpecifications().patternsMax)
						pSndFile->Order()[nOrd] = pSndFile->Order.GetInvalidPatIndex();
					else
						pSndFile->Order()[nOrd] = nOrigPat;
				}
				pModDoc->UpdateAllViews(nullptr, SequenceHint().Data());
				pModDoc->SetModified();
			}
			return true;
		}
		break;

	case MODITEM_SAMPLE:
		if(m_itemDrag.type == MODITEM_SAMPLE && pInfoDrag != nullptr)
		{
			if(bDoDrop)
			{
				if(sameModDoc)
				{
					// Reorder samples in a module
					const SAMPLEINDEX from = static_cast<SAMPLEINDEX>(modItemDragID - 1), to = static_cast<SAMPLEINDEX>(modItemDropID - 1);

					std::vector<SAMPLEINDEX> newOrder(pModDoc->GetNumSamples());
					for(SAMPLEINDEX smp = 0; smp < pModDoc->GetNumSamples(); smp++)
					{
						newOrder[smp] = smp + 1;
					}

					newOrder.erase(newOrder.begin() + from);
					newOrder.insert(newOrder.begin() + to, from + 1);

					pModDoc->ReArrangeSamples(newOrder);
				} else
				{
					// Load sample into other module
					pSndFile->ReadSampleFromSong(static_cast<SAMPLEINDEX>(modItemDropID), pInfoDrag->modDoc.GetSoundFile(), static_cast<SAMPLEINDEX>(modItemDragID));
				}
				pModDoc->UpdateAllViews(nullptr, SampleHint().Info().Data().Names());
				pModDoc->UpdateAllViews(nullptr, PatternHint().Data());
				pModDoc->UpdateAllViews(nullptr, InstrumentHint().Info());
				pModDoc->SetModified();
				SelectItem(hItem);
			}
			return true;
		}
		break;

	case MODITEM_INSTRUMENT:
		if(m_itemDrag.type == MODITEM_INSTRUMENT && pInfoDrag != nullptr)
		{
			if(bDoDrop)
			{
				if(sameModDoc)
				{
					// Reorder instruments in a module
					const INSTRUMENTINDEX from = static_cast<INSTRUMENTINDEX>(modItemDragID - 1), to = static_cast<INSTRUMENTINDEX>(modItemDropID - 1);

					std::vector<INSTRUMENTINDEX> newOrder(pModDoc->GetNumInstruments());
					for(INSTRUMENTINDEX ins = 0; ins < pModDoc->GetNumInstruments(); ins++)
					{
						newOrder[ins] = ins + 1;
					}

					newOrder.erase(newOrder.begin() + from);
					newOrder.insert(newOrder.begin() + to, from + 1);

					pModDoc->ReArrangeInstruments(newOrder);
				} else
				{
					// Load instrument into other module
					pSndFile->ReadInstrumentFromSong(static_cast<INSTRUMENTINDEX>(modItemDropID), pInfoDrag->modDoc.GetSoundFile(), static_cast<INSTRUMENTINDEX>(modItemDragID));
				}
				pModDoc->UpdateAllViews(nullptr, InstrumentHint().Info().Envelope().Names());
				pModDoc->UpdateAllViews(nullptr, PatternHint().Data());
				pModDoc->SetModified();
				SelectItem(hItem);
			}
			return true;
		}
		break;

	case MODITEM_MIDIINSTRUMENT:
	case MODITEM_MIDIPERCUSSION:
		if ((m_itemDrag.type == MODITEM_INSLIB_SAMPLE) || (m_itemDrag.type == MODITEM_INSLIB_INSTRUMENT))
		{
			if (bDoDrop)
			{
				mpt::PathString fullPath = InsLibGetFullPath(m_hItemDrag);
				if (modItemDrop.type == MODITEM_MIDIINSTRUMENT)
					SetMidiInstrument(modItemDropID, fullPath);
				else
					SetMidiPercussion(modItemDropID, fullPath);
			}
			return true;
		}
		break;
	}
	return false;
}


void CModTree::UpdatePlayPos(CModDoc &modDoc, Notification *pNotify)
{
	ModTreeDocInfo *pInfo = GetDocumentInfoFromModDoc(modDoc);
	if(pInfo == nullptr) return;

	const CSoundFile &sndFile = modDoc.GetSoundFile();
	ORDERINDEX nNewOrd = (pNotify) ? pNotify->order : ORDERINDEX_INVALID;
	SEQUENCEINDEX nNewSeq = sndFile.Order.GetCurrentSequenceIndex();
	if (nNewOrd != pInfo->nOrdSel || nNewSeq != pInfo->nSeqSel)
	{
		// Remove bold state from old item
		if(pInfo->nSeqSel < pInfo->tiOrders.size() && pInfo->nOrdSel < pInfo->tiOrders[pInfo->nSeqSel].size())
			SetItemState(pInfo->tiOrders[pInfo->nSeqSel][pInfo->nOrdSel], 0, TVIS_BOLD);

		pInfo->nOrdSel = nNewOrd;
		pInfo->nSeqSel = nNewSeq;
		if(pInfo->nSeqSel < pInfo->tiOrders.size() && pInfo->nOrdSel < pInfo->tiOrders[pInfo->nSeqSel].size())
			SetItemState(pInfo->tiOrders[pInfo->nSeqSel][pInfo->nOrdSel], TVIS_BOLD, TVIS_BOLD);
		else
			UpdateView(*pInfo, SequenceHint().Data());
	}

	// Update sample / instrument playing status icons (will only detect instruments with samples, though)

	if((TrackerSettings::Instance().m_dwPatternSetup & PATTERN_LIVEUPDATETREE) == 0) return;
	// TODO: Is there a way to find out if the treeview is actually visible?
	/*static int nUpdateCount = 0;
	nUpdateCount++;
	if(nUpdateCount < 5) return; // don't update too often
	nUpdateCount = 0;*/

	// check whether the lists are actually visible (don't waste resources)
	const bool updateSamples = IsItemExpanded(pInfo->hSamples), updateInstruments = IsItemExpanded(pInfo->hInstruments);

	pInfo->samplesPlaying.reset();
	pInfo->instrumentsPlaying.reset();

	if(!updateSamples && !updateInstruments) return;

	for(const auto &chn : sndFile.m_PlayState.Chn)
	{
		if(chn.pCurrentSample != nullptr && chn.nLength != 0 && chn.IsSamplePlaying())
		{
			if(updateSamples)
			{
				for(SAMPLEINDEX nSmp = sndFile.GetNumSamples(); nSmp >= 1; nSmp--)
				{
					if(chn.pModSample == &sndFile.GetSample(nSmp))
					{
						pInfo->samplesPlaying.set(nSmp);
						break;
					}
				}
			}
			if(updateInstruments)
			{
				for(INSTRUMENTINDEX nIns = sndFile.GetNumInstruments(); nIns >= 1; nIns--)
				{
					if(chn.pModInstrument == sndFile.Instruments[nIns])
					{
						pInfo->instrumentsPlaying.set(nIns);
						break;
					}
				}
			}
		}
	}
	// what should be updated?
	if(updateSamples) UpdateView(*pInfo, SampleHint().Info());
	if(updateInstruments) UpdateView(*pInfo, InstrumentHint().Info());
}



/////////////////////////////////////////////////////////////////////////////
// CViewModTree message handlers


void CModTree::OnUpdate(CModDoc *pModDoc, UpdateHint hint, CObject *pHint)
{
	if (pHint != this)
	{
		for (auto &doc : DocInfo)
		{
			if (&doc->modDoc == pModDoc || !pModDoc)
			{
				UpdateView(*doc, hint);
				break;
			}
		}
	}
}

void CModTree::OnItemExpanded(LPNMHDR pnmhdr, LRESULT *pResult)
{
	LPNMTREEVIEW pnm = (LPNMTREEVIEW)pnmhdr;
	if ((pnm->itemNew.iImage == IMAGE_FOLDER) || (pnm->itemNew.iImage == IMAGE_OPENFOLDER))
	{
		int iNewImage = (pnm->itemNew.state & TVIS_EXPANDED) ? IMAGE_OPENFOLDER : IMAGE_FOLDER;
		SetItemImage(pnm->itemNew.hItem, iNewImage, iNewImage);
	}
	if (pResult) *pResult = TRUE;
}


void CModTree::OnBeginDrag(HTREEITEM hItem, bool bLeft, LRESULT *pResult)
{
	if (!(m_dwStatus & TREESTATUS_DRAGGING))
	{
		bool bDrag = false;

		m_hDropWnd = NULL;
		m_hItemDrag = hItem;
		if (m_hItemDrag != NULL)
		{
			if (!ItemHasChildren(m_hItemDrag)) SelectItem(m_hItemDrag);
		}
		m_itemDrag = GetModItem(m_hItemDrag);
		m_nDragDocNdx = m_nDocNdx;
		switch(m_itemDrag.type)
		{
		case MODITEM_ORDER:
		case MODITEM_PATTERN:
		case MODITEM_SAMPLE:
		case MODITEM_INSTRUMENT:
		case MODITEM_SEQUENCE:
		case MODITEM_MIDIINSTRUMENT:
		case MODITEM_MIDIPERCUSSION:
		case MODITEM_INSLIB_SAMPLE:
		case MODITEM_INSLIB_INSTRUMENT:
		case MODITEM_INSLIB_SONG:
			bDrag = true;
			break;
		case MODITEM_HDR_ORDERS:
			// can we drag an order header? (only in MPTM format and if there's only one sequence)
			{
				const CModDoc *pModDoc = (m_nDragDocNdx < DocInfo.size() ? &(DocInfo[m_nDragDocNdx]->modDoc) : nullptr);
				if(pModDoc && pModDoc->GetSoundFile().Order.GetNumSequences() == 1)
					bDrag = true;
			}
			break;
		default:
			if (m_itemDrag.type == MODITEM_DLSBANK_INSTRUMENT) bDrag = true;
		}
		if (bDrag)
		{
			m_dwStatus |= (bLeft) ? TREESTATUS_LDRAG : TREESTATUS_RDRAG;
			m_hItemDrop = NULL;
			SetCapture();
		}
	}
	if (pResult) *pResult = TRUE;
}


void CModTree::OnBeginRDrag(LPNMHDR pnmhdr, LRESULT *pResult)
{
	if (pnmhdr)
	{
		LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmhdr;
		OnBeginDrag(pnmtv->itemNew.hItem, false, pResult);
	}
}


void CModTree::OnBeginLDrag(LPNMHDR pnmhdr, LRESULT *pResult)
{
	if (pnmhdr)
	{
		LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmhdr;
		OnBeginDrag(pnmtv->itemNew.hItem, true, pResult);
	}
}


void CModTree::OnItemDblClk(LPNMHDR, LRESULT *pResult)
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	HTREEITEM hItem = GetSelectedItem();
	if ((hItem) && (hItem == HitTest(pt)))
	{
		ExecuteItem(hItem);
	}
	if (pResult) *pResult = 0;
}


void CModTree::OnItemReturn(LPNMHDR, LRESULT *pResult)
{
	HTREEITEM hItem = GetSelectedItem();
	if (hItem) ExecuteItem(hItem);
	if (pResult) *pResult = 0;
}


void CModTree::OnItemRightClick(LPNMHDR, LRESULT *pResult)
{
	CPoint pt, ptClient;
	UINT flags;

	GetCursorPos(&pt);
	ptClient = pt;
	ScreenToClient(&ptClient);
	OnItemRightClick(HitTest(ptClient, &flags), pt);
	if(pResult) *pResult = 0;
}


void CModTree::OnItemRightClick(HTREEITEM hItem, CPoint pt)
{
	HMENU hMenu;
	if (m_dwStatus & TREESTATUS_LDRAG)
	{
		if (ItemHasChildren(hItem))
		{
			Expand(hItem, TVE_TOGGLE);
		} else
		{
			m_hItemDrop = NULL;
			m_hDropWnd = NULL;
			OnEndDrag(TREESTATUS_DRAGGING);
		}
	} else
	{
		if (m_dwStatus & TREESTATUS_DRAGGING)
		{
			m_hItemDrop = NULL;
			m_hDropWnd = NULL;
			OnEndDrag(TREESTATUS_DRAGGING);
		}
		hMenu = ::CreatePopupMenu();
		if (hMenu)
		{
			const CModDoc *modDoc = GetDocumentFromItem(hItem);
			const CSoundFile *sndFile = modDoc != nullptr ? &modDoc->GetSoundFile() : nullptr;

			UINT nDefault = 0;
			BOOL bSep = FALSE;

			const ModItem modItem = GetModItem(hItem);
			const uint32 modItemID = modItem.val1;

			SelectItem(hItem);
			switch(modItem.type)
			{
			case MODITEM_HDR_SONG:
				nDefault = ID_MODTREE_EXECUTE;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&View"));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_CLOSE, _T("&Close"));
				break;

			case MODITEM_COMMENTS:
				nDefault = ID_MODTREE_EXECUTE;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&View Comments"));
				break;

			case MODITEM_ORDER:
			case MODITEM_PATTERN:
				nDefault = ID_MODTREE_EXECUTE;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&Edit Pattern"));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE,
								(modItem.type == MODITEM_ORDER) ? _T("&Delete from list") : _T("&Delete Pattern"));
				break;

			case MODITEM_SEQUENCE:
				{
					bool isCurSeq = false;
					if(sndFile && sndFile->GetModSpecifications().sequencesMax > 1)
					{
						if(sndFile->Order((SEQUENCEINDEX)modItemID).GetLength() == 0)
						{
							nDefault = ID_MODTREE_SWITCHTO;
						}
						isCurSeq = (sndFile->Order.GetCurrentSequenceIndex() == (SEQUENCEINDEX)modItemID);
					}

					if(!isCurSeq)
					{
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_SWITCHTO, _T("&Switch to Seqeuence"));
					}
					AppendMenu(hMenu, MF_STRING | (sndFile->Order.GetNumSequences() < MAX_SEQUENCES ? 0 : MF_GRAYED), ID_MODTREE_INSERT, _T("&Insert Sequence"));
					AppendMenu(hMenu, MF_STRING | (sndFile->Order.GetNumSequences() < MAX_SEQUENCES ? 0 : MF_GRAYED), ID_MODTREE_DUPLICATE , _T("D&uplicate Sequence"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("&Delete Sequence"));
				}
				break;


			case MODITEM_HDR_ORDERS:
				if(sndFile && sndFile->GetModSpecifications().sequencesMax > 1)
				{
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_INSERT, _T("&Insert Sequence"));
					if(sndFile->Order.GetNumSequences() == 1) // this is a sequence
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_DUPLICATE, _T("D&uplicate Sequence"));
				}
				break;

			case MODITEM_SAMPLE:
				{
					nDefault = ID_MODTREE_EXECUTE;
					AppendMenu(hMenu, MF_STRING, nDefault, _T("&View Sample"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_PLAY, _T("&Play Sample"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_INSERT, _T("&Insert Sample"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_DUPLICATE, _T("D&uplicate Sample"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("&Delete Sample"));
					if ((modDoc) && (!modDoc->GetNumInstruments()))
					{
						AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
						AppendMenu(hMenu, (modDoc->IsSampleMuted((SAMPLEINDEX)modItemID) ? MF_CHECKED : 0) | MF_STRING, ID_MODTREE_MUTE, _T("&Mute Sample"));
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_SOLO, _T("S&olo Sample"));
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_UNMUTEALL, _T("&Unmute all"));
					}
					if(sndFile != nullptr)
					{
						SAMPLEINDEX smpID = static_cast<SAMPLEINDEX>(modItem.val1);
						const ModSample &sample = sndFile->GetSample(smpID);
						const bool hasPath = sndFile->SampleHasPath(smpID);
						const bool menuForThisSample = (sample.HasSampleData() && sndFile->GetType() == MOD_TYPE_MPT) || hasPath;

						bool anyPath = false, anyModified = false, anyMissing = false;
						for(SAMPLEINDEX smp = 1; smp <= sndFile->GetNumSamples(); smp++)
						{
							if(sndFile->SampleHasPath(smp) && smp != smpID)
							{
								anyPath = true;
								if(sndFile->GetSample(smp).HasSampleData() && sndFile->GetSample(smp).uFlags[SMP_MODIFIED])
								{
									anyModified = true;
								}
							}
							if(sndFile->IsExternalSampleMissing(smp))
							{
								anyMissing = true;
							}
							if(anyPath && anyModified && anyMissing) break;
						}

						if(menuForThisSample || anyPath || anyModified)
						{
							AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
							if(menuForThisSample) AppendMenu(hMenu, MF_STRING | ((sndFile->GetType() == MOD_TYPE_MPT || hasPath) ? 0 : MF_GRAYED), ID_MODTREE_SETPATH, _T("Set P&ath"));
							if(menuForThisSample) AppendMenu(hMenu, MF_STRING | ((hasPath && sample.HasSampleData() && sample.uFlags[SMP_MODIFIED]) ? 0 : MF_GRAYED), ID_MODTREE_SAVEITEM, _T("&Save"));
							if(anyModified) AppendMenu(hMenu, MF_STRING, ID_MODTREE_SAVEALL, _T("&Save All"));
							if(menuForThisSample) AppendMenu(hMenu, MF_STRING | (hasPath ? 0 : MF_GRAYED), ID_MODTREE_RELOADITEM, _T("&Reload"));
							if(anyPath) AppendMenu(hMenu, MF_STRING, ID_MODTREE_RELOADALL, _T("&Reload All"));
							if(anyMissing) AppendMenu(hMenu, MF_STRING, ID_MODTREE_FINDMISSING, _T("&Find Missing Samples"));
						}
					}
				}
				break;

			case MODITEM_INSTRUMENT:
				{
					nDefault = ID_MODTREE_EXECUTE;
					AppendMenu(hMenu, MF_STRING, nDefault, _T("&View Instrument"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_PLAY, _T("&Play Instrument"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_INSERT, _T("&Insert Instrument"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_DUPLICATE, _T("D&uplicate Instrument"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("&Delete Instrument"));
					if (modDoc)
					{
						AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
						AppendMenu(hMenu, (modDoc->IsInstrumentMuted((INSTRUMENTINDEX)modItemID) ? MF_CHECKED : 0) | MF_STRING, ID_MODTREE_MUTE, _T("&Mute Instrument"));
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_SOLO, _T("S&olo Instrument"));
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_UNMUTEALL, _T("&Unmute all"));
					}
				}
				break;

			case MODITEM_EFFECT:
				{
					nDefault = ID_MODTREE_EXECUTE;
					AppendMenu(hMenu, MF_STRING, nDefault, _T("&Edit"));

					if(modDoc != nullptr)
					{
						AppendMenu(hMenu, (modDoc->GetSoundFile().m_MixPlugins[modItemID].IsBypassed() ? MF_CHECKED : 0) | MF_STRING, ID_MODTREE_MUTE, _T("&Bypass"));
					}
				}
				break;

			case MODITEM_MIDIINSTRUMENT:
			case MODITEM_MIDIPERCUSSION:
				nDefault = ID_MODTREE_EXECUTE;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&Map Instrument"));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_PLAY, _T("&Play Instrument"));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("&Unmap Instrument"));
				AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
			case MODITEM_HDR_MIDILIB:
			case MODITEM_HDR_MIDIGROUP:
				AppendMenu(hMenu, MF_STRING, ID_IMPORT_MIDILIB, _T("&Import MIDI Library"));
				AppendMenu(hMenu, MF_STRING, ID_EXPORT_MIDILIB, _T("E&xport MIDI Library"));
				bSep = TRUE;
				break;

			case MODITEM_HDR_INSTRUMENTLIB:
				if(!IsSampleBrowser())
					break;
				MPT_FALLTHROUGH;
			case MODITEM_INSLIB_FOLDER:
				nDefault = ID_MODTREE_EXECUTE;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&Browse..."));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_OPENITEM, _T("&Open in Explorer"));
				{
					auto insDir = TrackerSettings::Instance().PathInstruments.GetDefaultDir();
					auto smpDir = TrackerSettings::Instance().PathSamples.GetDefaultDir();
					if(!insDir.empty() && insDir != m_InstrLibPath)
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_GOTO_INSDIR, _T("Go to &Instrument directory"));
					if(!smpDir.empty() && smpDir != insDir && smpDir != m_InstrLibPath)
						AppendMenu(hMenu, MF_STRING, ID_MODTREE_GOTO_SMPDIR, _T("Go to Sa&mple directory"));
				}
				break;

			case MODITEM_INSLIB_SONG:
				nDefault = ID_MODTREE_EXECUTE;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&Browse Song..."));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_OPENITEM, _T("&Edit Song"));
				AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("&Delete"));
				break;

			case MODITEM_INSLIB_SAMPLE:
			case MODITEM_INSLIB_INSTRUMENT:
				nDefault = ID_MODTREE_PLAY;
				if(!m_SongFileName.empty())
				{
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_PLAY, _T("&Play"));
				} else
				{
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_PLAY, _T("&Play File"));
					AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("&Delete"));
				}
				break;

			case MODITEM_DLSBANK_FOLDER:
				nDefault = ID_SOUNDBANK_PROPERTIES;
				AppendMenu(hMenu, MF_STRING, nDefault, _T("&Properties"));
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_REMOVE, _T("Re&move this bank"));
			case MODITEM_NULL:
				AppendMenu(hMenu, MF_STRING, ID_ADD_SOUNDBANK, _T("Add Sound &Bank..."));
				bSep = TRUE;
				break;

			case MODITEM_DLSBANK_INSTRUMENT:
				nDefault = ID_MODTREE_PLAY;
				AppendMenu(hMenu, MF_STRING, ID_MODTREE_PLAY, _T("&Play Instrument"));
				break;
			}
			if (nDefault) SetMenuDefaultItem(hMenu, nDefault, FALSE);
			if ((modItem.type == MODITEM_INSLIB_FOLDER)
			 || (modItem.type == MODITEM_INSLIB_SONG)
			 || (modItem.type == MODITEM_HDR_INSTRUMENTLIB))
			{
				if ((bSep) || (nDefault)) AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
				AppendMenu(hMenu, TrackerSettings::Instance().showDirsInSampleBrowser ? (MF_STRING|MF_CHECKED) : MF_STRING, ID_MODTREE_SHOWDIRS, _T("Show &Directories in Sample Browser"));
				AppendMenu(hMenu, (m_showAllFiles) ? (MF_STRING|MF_CHECKED) : MF_STRING, ID_MODTREE_SHOWALLFILES, _T("Show &All Files"));
				AppendMenu(hMenu, (m_showAllFiles) ? MF_STRING : (MF_STRING|MF_CHECKED), ID_MODTREE_SOUNDFILESONLY, _T("Show &Sound Files"));
				bSep = TRUE;
			}
			if ((bSep) || (nDefault)) AppendMenu(hMenu, MF_SEPARATOR, NULL, _T(""));
			AppendMenu(hMenu, MF_STRING, ID_MODTREE_REFRESH, _T("&Refresh"));
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x + 4, pt.y, 0, m_hWnd, NULL);
			DestroyMenu(hMenu);
		}
	}
}


void CModTree::OnItemLeftClick(LPNMHDR, LRESULT *pResult)
{
	if (!(m_dwStatus & TREESTATUS_RDRAG))
	{
		POINT pt;
		UINT flags;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		HTREEITEM hItem = HitTest(pt, &flags);
		if (hItem != NULL)
		{
			const ModItem modItem = GetModItem(hItem);
			const uint32 modItemID = modItem.val1;

			switch(modItem.type)
			{
			case MODITEM_INSLIB_FOLDER:
			case MODITEM_INSLIB_SONG:
				if (m_dwStatus & TREESTATUS_SINGLEEXPAND) ExecuteItem(hItem);
				break;

			case MODITEM_SAMPLE:
			case MODITEM_INSTRUMENT:
				{
					CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
					CFrameWnd *pFrame = pMainFrm->GetActiveFrame();
					if (pFrame)
					{
						pFrame->SendMessage(WM_MOD_INSTRSELECTED,
							(modItem.type == MODITEM_INSTRUMENT) ? TRUE : FALSE,
							(LPARAM)modItemID);
					}
				}
				break;

			case MODITEM_HDR_SONG:
				ExecuteItem(hItem);
				break;
			}
		}
	}
	if (pResult) *pResult = 0;
}


void CModTree::OnEndDrag(DWORD dwMask)
{
	if (m_dwStatus & dwMask)
	{
		m_dwStatus &= ~dwMask;
		if (!(m_dwStatus & TREESTATUS_DRAGGING))
		{
			ReleaseCapture();
			SetCursor(CMainFrame::curArrow);
			SelectDropTarget(NULL);
			if (m_hItemDrop != NULL)
			{
				CanDrop(m_hItemDrop, TRUE);
			} else
			if (m_hDropWnd)
			{
				DRAGONDROP dropinfo;
				mpt::PathString fullPath;
				if(GetDropInfo(dropinfo, fullPath))
				{
					if (dropinfo.dwDropType == DRAGONDROP_SONG)
					{
						theApp.OpenDocumentFile(fullPath.ToCString());
					} else
					{
						::SendMessage(m_hDropWnd, WM_MOD_DRAGONDROPPING, TRUE, (LPARAM)&dropinfo);
					}
				}
			}
		}
	}
}


void CModTree::OnLButtonUp(UINT nFlags, CPoint point)
{
	OnEndDrag(TREESTATUS_LDRAG);
	CTreeCtrl::OnLButtonUp(nFlags, point);
}


void CModTree::OnRButtonUp(UINT nFlags, CPoint point)
{
	OnEndDrag(TREESTATUS_RDRAG);
	CTreeCtrl::OnRButtonUp(nFlags, point);
}


void CModTree::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dwStatus & TREESTATUS_DRAGGING)
	{
		HTREEITEM hItem;
		UINT flags;

		// Bug?
		if (!(nFlags & (MK_LBUTTON|MK_RBUTTON)))
		{
			m_itemDrag = ModItem(MODITEM_NULL);
			m_hItemDrag = NULL;
			OnEndDrag(TREESTATUS_DRAGGING);
			return;
		}
		CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
		if (pMainFrm)
		{
			CRect rect;
			GetClientRect(&rect);
			if (rect.PtInRect(point))
			{
				m_hDropWnd = m_hWnd;
				bool bCanDrop = CanDrop( HitTest(point, &flags), false);
				SetCursor((bCanDrop) ? CMainFrame::curDragging : CMainFrame::curNoDrop2);
			} else
			{
				CPoint screenPt = point;
				ClientToScreen(&screenPt);
				HWND hwnd = ::WindowFromPoint(screenPt);
				if (hwnd != m_hDropWnd)
				{
					bool canDrop = false;
					m_hDropWnd = hwnd;
					if (hwnd == m_hWnd)
					{
						canDrop = true;
					} else if (hwnd != NULL)
					{
						DRAGONDROP dropinfo;
						mpt::PathString fullPath;
						if(GetDropInfo(dropinfo, fullPath))
						{
							if (dropinfo.dwDropType == DRAGONDROP_SONG)
							{
								canDrop = true;
							} else if(::SendMessage(hwnd, WM_MOD_DRAGONDROPPING, FALSE, (LPARAM)&dropinfo))
							{
								canDrop = true;
							}
						}
					}
					SetCursor(canDrop ? CMainFrame::curDragging : CMainFrame::curNoDrop);
					if (canDrop)
					{
						if (GetDropHilightItem() != m_hItemDrag)
						{
							SelectDropTarget(m_hItemDrag);
						}
						m_hItemDrop = NULL;
						return;
					}
				}
			}
			if ((point.x >= -1) && (point.x <= rect.right + GetSystemMetrics(SM_CXVSCROLL)))
			{
				if (point.y <= 0)
				{
					HTREEITEM hfirst = GetFirstVisibleItem();
					if (hfirst != NULL)
					{
						HTREEITEM hprev = GetPrevVisibleItem(hfirst);
						if (hprev != NULL) SelectSetFirstVisible(hprev);
					}
				} else if (point.y >= rect.bottom-1)
				{
					hItem = HitTest(point, &flags);
					HTREEITEM hNext = GetNextItem(hItem, TVGN_NEXTVISIBLE);
					if (hNext != NULL)
					{
						EnsureVisible(hNext);
					}
				}
			}
			if ((hItem = HitTest(point, &flags)) != NULL)
			{
				SelectDropTarget(hItem);
				m_hItemDrop = hItem;
			}
		}
	}
	CTreeCtrl::OnMouseMove(nFlags, point);
}


void CModTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_DELETE:
		DeleteTreeItem(GetSelectedItem());
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CModTree::OnRefreshTree()
{
	BeginWaitCursor();
	for (auto &doc : DocInfo)
	{
		UpdateView(*doc, UpdateHint().ModType());
	}
	RefreshMidiLibrary();
	RefreshDlsBanks();
	RefreshInstrumentLibrary();
	EndWaitCursor();
}


void CModTree::OnExecuteItem()
{
	ExecuteItem(GetSelectedItem());
}


void CModTree::OnDeleteTreeItem()
{
	DeleteTreeItem(GetSelectedItem());
}


void CModTree::OnPlayTreeItem()
{
	PlayItem(GetSelectedItem(), NOTE_MIDDLEC);
}


void CModTree::OnOpenTreeItem()
{
	OpenTreeItem(GetSelectedItem());
}


void CModTree::OnMuteTreeItem()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);
	const uint32 modItemID = modItem.val1;

	ModTreeDocInfo *info = GetDocumentInfoFromItem(hItem);
	if (info)
	{
		CModDoc &modDoc = info->modDoc;
		if ((modItem.type == MODITEM_SAMPLE) && !modDoc.GetNumInstruments())
		{
			modDoc.MuteSample((SAMPLEINDEX)modItemID, !modDoc.IsSampleMuted((SAMPLEINDEX)modItemID));
			UpdateView(*info, SampleHint((SAMPLEINDEX)modItemID).Info().Names());
		} else
		if ((modItem.type == MODITEM_INSTRUMENT) && modDoc.GetNumInstruments())
		{
			modDoc.MuteInstrument((INSTRUMENTINDEX)modItemID, !modDoc.IsInstrumentMuted((INSTRUMENTINDEX)modItemID));
			UpdateView(*info, InstrumentHint((INSTRUMENTINDEX)modItemID).Info().Names());
		} else if ((modItem.type == MODITEM_EFFECT))
		{
			IMixPlugin *pPlugin = modDoc.GetSoundFile().m_MixPlugins[modItemID].pMixPlugin;
			if(pPlugin == nullptr)
				return;
			pPlugin->ToggleBypass();
			modDoc.SetModified();
			//UpdateView(GetDocumentIDFromModDoc(pModDoc), HINT_MIXPLUGINS);
		}
	}
}


void CModTree::OnSoloTreeItem()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);
	const uint32 modItemID = modItem.val1;

	ModTreeDocInfo *info = GetDocumentInfoFromItem(hItem);
	if (info)
	{
		CModDoc &modDoc = info->modDoc;
		INSTRUMENTINDEX nInstruments = modDoc.GetNumInstruments();
		if ((modItem.type == MODITEM_SAMPLE) && (!nInstruments))
		{
			for (SAMPLEINDEX nSmp = 1; nSmp <= modDoc.GetNumSamples(); nSmp++)
			{
				modDoc.MuteSample(nSmp, nSmp != modItemID);
			}
			UpdateView(*info, SampleHint().Info().Names());
		} else if ((modItem.type == MODITEM_INSTRUMENT) && (nInstruments))
		{
			for (INSTRUMENTINDEX nIns = 1; nIns <= nInstruments; nIns++)
			{
				modDoc.MuteInstrument(nIns, nIns != modItemID);
			}
			UpdateView(*info, InstrumentHint().Info().Names());
		}
	}
}


void CModTree::OnUnmuteAllTreeItem()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);

	ModTreeDocInfo *info = GetDocumentInfoFromItem(hItem);
	if (info)
	{
		CModDoc &modDoc = info->modDoc;
		if ((modItem.type == MODITEM_SAMPLE) || (modItem.type == MODITEM_INSTRUMENT))
		{
			for (SAMPLEINDEX nSmp = 1; nSmp <= modDoc.GetNumSamples(); nSmp++)
			{
				modDoc.MuteSample(nSmp, false);
			}
			UpdateView(*info, SampleHint().Info().Names());
			for (INSTRUMENTINDEX nIns = 1; nIns <= modDoc.GetNumInstruments(); nIns++)
			{
				modDoc.MuteInstrument(nIns, false);
			}
			UpdateView(*info, InstrumentHint().Info().Names());
		}
	}
}


// Helper function for generating an insert vector for samples/instruments
template<typename T>
std::vector<T> GenerateInsertVector(size_t howMany, size_t insertPos, T insertId)
{
	std::vector<T> newOrder(howMany);
	for(T i = 0; i < howMany; i++)
	{
		newOrder[i] = i + 1;
	}
	newOrder.insert(newOrder.begin() + insertPos, insertId);
	return newOrder;
}


void CModTree::InsertOrDupItem(bool insert)
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);
	const uint32 modItemID = modItem.val1;

	ModTreeDocInfo *info = GetDocumentInfoFromItem(hItem);
	if (info)
	{
		CModDoc &modDoc = info->modDoc;
		CSoundFile &sndFile = modDoc.GetSoundFile();
		if(modItem.type == MODITEM_SEQUENCE || modItem.type == MODITEM_HDR_ORDERS)
		{
			// Duplicate / insert sequence
			if(insert)
			{
				sndFile.Order.AddSequence(false);
			} else
			{
				sndFile.Order.SetSequence((SEQUENCEINDEX)modItemID);
				sndFile.Order.AddSequence(true);
			}
			modDoc.SetModified();
			UpdateView(*info, SequenceHint(MAX_SEQUENCES).Data().Names());
			modDoc.UpdateAllViews(nullptr, SequenceHint().Data().Names());
		} else if(modItem.type == MODITEM_SAMPLE)
		{
			// Duplicate / insert sample
			std::vector<SAMPLEINDEX> newOrder = GenerateInsertVector<SAMPLEINDEX>(sndFile.GetNumSamples(), modItemID, static_cast<SAMPLEINDEX>(insert ? 0 : modItemID));
			if(modDoc.ReArrangeSamples(newOrder) != SAMPLEINDEX_INVALID)
			{
				modDoc.SetModified();
				modDoc.UpdateAllViews(nullptr, SampleHint().Info().Data().Names());
				modDoc.UpdateAllViews(nullptr, PatternHint().Data());
			} else
			{
				Reporting::Error("Maximum number of samples reached.");
			}
		} else if(modItem.type == MODITEM_INSTRUMENT)
		{
			// Duplicate / insert instrument
			std::vector<INSTRUMENTINDEX> newOrder = GenerateInsertVector<INSTRUMENTINDEX>(sndFile.GetNumInstruments(), modItemID, static_cast<INSTRUMENTINDEX>(insert ? 0 : modItemID));
			if(modDoc.ReArrangeInstruments(newOrder) != INSTRUMENTINDEX_INVALID)
			{
				modDoc.UpdateAllViews(NULL, InstrumentHint().Info().Envelope().Names());
				modDoc.UpdateAllViews(nullptr, PatternHint().Data());
				modDoc.SetModified();
			} else
			{
				Reporting::Error("Maximum number of instruments reached.");
			}
		}
	}
}


void CModTree::OnSwitchToTreeItem()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);

	CModDoc *pModDoc = GetDocumentFromItem(hItem);
	if (pModDoc && (modItem.type == MODITEM_SEQUENCE))
	{
		pModDoc->ActivateView(IDD_CONTROL_PATTERNS, uint32(modItem.val1 << SEQU_SHIFT) | SEQU_INDICATOR);
	}
}


void CModTree::OnSetItemPath()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);
	CModDoc *pModDoc = GetDocumentFromItem(hItem);

	if(pModDoc && modItem.val1)
	{
		SAMPLEINDEX smpID = static_cast<SAMPLEINDEX>(modItem.val1);
		const mpt::PathString path = pModDoc->GetSoundFile().GetSamplePath(smpID);
		FileDialog dlg = OpenFileDialog()
			.ExtensionFilter("All Samples|*.wav;*.flac|All files(*.*)|*.*||");	// Only show samples that we actually can save as well.
		if(path.empty())
			dlg.WorkingDirectory(TrackerSettings::Instance().PathSamples.GetWorkingDir());
		else
			dlg.DefaultFilename(path);
		if(!dlg.Show()) return;
		TrackerSettings::Instance().PathSamples.SetWorkingDir(dlg.GetWorkingDirectory());

		if(dlg.GetFirstFile() != pModDoc->GetSoundFile().GetSamplePath(smpID))
		{
			pModDoc->GetSoundFile().SetSamplePath(smpID, dlg.GetFirstFile());
			pModDoc->SetModified();
		}
		OnReloadItem();
	}
}


void CModTree::OnSaveItem()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);
	CModDoc *pModDoc = GetDocumentFromItem(hItem);

	if(pModDoc && modItem.val1)
	{
		SAMPLEINDEX smpID = static_cast<SAMPLEINDEX>(modItem.val1);
		pModDoc->SaveSample(smpID);
		if(pModDoc) pModDoc->UpdateAllViews(NULL, SampleHint(smpID).Info());
		OnRefreshTree();
	}
}


void CModTree::OnSaveAll()
{
	CModDoc *pModDoc = GetDocumentFromItem(GetSelectedItem());
	if(pModDoc != nullptr)
	{
		pModDoc->SaveAllSamples();
		if(pModDoc) pModDoc->UpdateAllViews(nullptr, SampleHint().Info());
		OnRefreshTree();
	}
}


void CModTree::OnReloadItem()
{
	HTREEITEM hItem = GetSelectedItem();

	const ModItem modItem = GetModItem(hItem);
	CModDoc *pModDoc = GetDocumentFromItem(hItem);

	if(pModDoc && modItem.val1)
	{
		SAMPLEINDEX smpID = static_cast<SAMPLEINDEX>(modItem.val1);
		CSoundFile &sndFile = pModDoc->GetSoundFile();
		pModDoc->GetSampleUndo().PrepareUndo(smpID, sundo_replace, "Replace");
		if(!sndFile.LoadExternalSample(smpID, sndFile.GetSamplePath(smpID)))
		{
			pModDoc->GetSampleUndo().RemoveLastUndoStep(smpID);
			Reporting::Error(_T("Unable to load sample:\n") + sndFile.GetSamplePath(smpID).AsNative());
		} else
		{
			if(!sndFile.GetSample(smpID).uFlags[SMP_KEEPONDISK])
			{
				pModDoc->SetModified();
			}
			pModDoc->UpdateAllViews(NULL, SampleHint(smpID).Info().Data().Names());
		}

		OnRefreshTree();
	}
}


void CModTree::OnReloadAll()
{
	CModDoc *pModDoc = GetDocumentFromItem(GetSelectedItem());
	if(pModDoc != nullptr)
	{
		CSoundFile &sndFile = pModDoc->GetSoundFile();
		bool anyMissing = false;
		for(SAMPLEINDEX smp = 1; smp <= sndFile.GetNumSamples(); smp++)
		{
			const mpt::PathString &path = sndFile.GetSamplePath(smp);
			if(path.empty()) continue;

			pModDoc->GetSampleUndo().PrepareUndo(smp, sundo_replace, "Replace");
			if(!sndFile.LoadExternalSample(smp, path))
			{
				pModDoc->GetSampleUndo().RemoveLastUndoStep(smp);
				anyMissing = true;
			} else
			{
				if(!sndFile.GetSample(smp).uFlags[SMP_KEEPONDISK])
				{
					pModDoc->SetModified();
				}
			}
		}
		pModDoc->UpdateAllViews(nullptr, SampleHint().Info().Data().Names());
		OnRefreshTree();
		if(anyMissing)
		{
			OnFindMissing();
		}
	}
}


// Find missing external samples
void CModTree::OnFindMissing()
{
	CModDoc *pModDoc = GetDocumentFromItem(GetSelectedItem());
	if(pModDoc == nullptr)
	{
		return;
	}
	ExternalSamplesDlg dlg(*pModDoc, CMainFrame::GetMainFrame());
	dlg.DoModal();
}


void CModTree::OnAddDlsBank()
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm) pMainFrm->OnAddDlsBank();
}


void CModTree::OnImportMidiLib()
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pMainFrm) pMainFrm->OnImportMidiLib();
}


void CModTree::OnExportMidiLib()
{
	FileDialog dlg = SaveFileDialog()
		.DefaultExtension("ini")
		.DefaultFilename("mptrack.ini")
		.ExtensionFilter("Text and INI files (*.txt,*.ini)|*.txt;*.ini|"
			"All Files (*.*)|*.*||");
	if(!dlg.Show()) return;

	CTrackApp::ExportMidiConfig(dlg.GetFirstFile());
}


///////////////////////////////////////////////////////////////////////
// Drop support

DROPEFFECT CModTree::OnDragEnter(COleDataObject*, DWORD, CPoint)
{
	return DROPEFFECT_LINK;
}


DROPEFFECT CModTree::OnDragOver(COleDataObject*, DWORD, CPoint point)
{
	UINT flags;
	HTREEITEM hItem = HitTest(point, &flags);

	const ModItem modItem = GetModItem(hItem);

	switch(modItem.type)
	{
	case MODITEM_MIDIINSTRUMENT:
	case MODITEM_MIDIPERCUSSION:
		if (hItem != GetDropHilightItem())
		{
			SelectDropTarget(hItem);
			EnsureVisible(hItem);
		}
		m_hItemDrag = hItem;
		m_itemDrag = modItem;
		return DROPEFFECT_LINK;
	// Folders:
	case MODITEM_HDR_MIDILIB:
	case MODITEM_HDR_MIDIGROUP:
		EnsureVisible(GetChildItem(hItem));
		break;
	}
	return DROPEFFECT_NONE;
}


BOOL CModTree::OnDrop(COleDataObject* pDataObject, DROPEFFECT, CPoint)
{
	STGMEDIUM stgm;
	HDROP hDropInfo;
	UINT nFiles;
	BOOL bOk = FALSE;

	if (!pDataObject) return FALSE;
	if (!pDataObject->GetData(CF_HDROP, &stgm)) return FALSE;
	if (stgm.tymed != TYMED_HGLOBAL) return FALSE;
	if (stgm.hGlobal == NULL) return FALSE;
	hDropInfo = (HDROP)stgm.hGlobal;
	nFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	if (nFiles)
	{
		UINT size = ::DragQueryFile(hDropInfo, 0, nullptr, 0) + 1;
		std::vector<TCHAR> fileName(size, _T('\0'));
		if(DragQueryFile(hDropInfo, 0, fileName.data(), size))
		{
			switch(m_itemDrag.type)
			{
			case MODITEM_MIDIINSTRUMENT:
				bOk = SetMidiInstrument(m_itemDrag.val1, mpt::PathString::FromNative(fileName.data()));
				break;
			case MODITEM_MIDIPERCUSSION:
				bOk = SetMidiPercussion(m_itemDrag.val1, mpt::PathString::FromNative(fileName.data()));
				break;
			}
		}
	}
	// After dropping some file on the MIDI library, we will need to do this or you
	// won't be able to select a new item until you started a new (internal) dragondrop.
	if(m_hItemDrag)
	{
		OnBeginDrag(m_hItemDrag, true, nullptr);
		OnEndDrag(TREESTATUS_DRAGGING);
	}
	m_itemDrag = ModItem(MODITEM_NULL);
	m_hItemDrag = NULL;
	::DragFinish(hDropInfo);
	return bOk;
}


void CModTree::OnRefreshInstrLib()
{
	HTREEITEM hActive;

	BeginWaitCursor();
	RefreshInstrumentLibrary();
	if (!IsSampleBrowser())
	{
		hActive = NULL;
		if(!m_InstrLibHighlightPath.empty() || !m_SongFileName.empty())
		{
			HTREEITEM hItem = GetChildItem(m_hInsLib);
			while (hItem != NULL)
			{
				const mpt::PathString str = mpt::PathString::FromCString(GetItemText(hItem));
				if(!mpt::PathString::CompareNoCase(str, m_InstrLibHighlightPath))
				{
					hActive = hItem;
					break;
				}
				hItem = GetNextItem(hItem, TVGN_NEXT);
			}
			if(m_SongFileName.empty()) m_InstrLibHighlightPath = P_("");
		}
		SelectSetFirstVisible(m_hInsLib);
		if (hActive != NULL) SelectItem(hActive);
	}
	EndWaitCursor();
}


void CModTree::OnShowDirectories()
{
	TrackerSettings::Instance().showDirsInSampleBrowser = !TrackerSettings::Instance().showDirsInSampleBrowser;
	OnRefreshInstrLib();
}


void CModTree::OnShowAllFiles()
{
	if (!m_showAllFiles)
	{
		m_showAllFiles = true;
		OnRefreshInstrLib();
	}
}


void CModTree::OnShowSoundFiles()
{
	if (m_showAllFiles)
	{
		m_showAllFiles = false;
		OnRefreshInstrLib();
	}
}


void CModTree::OnGotoInstrumentDir()
{
	CMainFrame::GetMainFrame()->GetUpperTreeview()->InstrumentLibraryChDir(TrackerSettings::Instance().PathInstruments.GetDefaultDir(), false);
}


void CModTree::OnGotoSampleDir()
{
	CMainFrame::GetMainFrame()->GetUpperTreeview()->InstrumentLibraryChDir(TrackerSettings::Instance().PathSamples.GetDefaultDir(), false);
}


void CModTree::OnSoundBankProperties()
{
	const ModItem modItem = GetModItem(GetSelectedItem());
	if(modItem.type == MODITEM_DLSBANK_FOLDER
		&& modItem.val1 < CTrackApp::gpDLSBanks.size() && CTrackApp::gpDLSBanks[modItem.val1])
	{
		CSoundBankProperties dlg(*CTrackApp::gpDLSBanks[modItem.val1], this);
		dlg.DoModal();
	}
}


LRESULT CModTree::OnCustomKeyMsg(WPARAM wParam, LPARAM /*lParam*/)
{
	if(wParam == kcNull)
		return NULL;

	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	ModCommand::NOTE note = NOTE_NONE;
	const bool start = wParam >= kcTreeViewStartNotes && wParam <= kcTreeViewEndNotes,
		stop = (wParam >= kcTreeViewStartNoteStops && wParam <= kcTreeViewEndNoteStops) && !IsSampleBrowser();
	if(start || stop)
	{
		note = static_cast<ModCommand::NOTE>(wParam - (start ? kcTreeViewStartNotes : kcTreeViewStartNoteStops) + 1 + pMainFrm->GetBaseOctave() * 12);
	} else if(wParam == kcTreeViewStopPreview)
	{
		note = NOTE_NOTECUT;
	}
	if(note != NOTE_NONE)
	{
		if(stop) note |= 0x80;

		if(PlayItem(GetSelectedItem(), note))
			return wParam;
		else
			return NULL;
	}

	return NULL;
}


LRESULT CModTree::OnMidiMsg(WPARAM midiData_, LPARAM)
{
	uint32 midiData = static_cast<uint32>(midiData_);
	// Handle MIDI messages assigned to shortcuts
	CInputHandler *ih = CMainFrame::GetInputHandler();
	ih->HandleMIDIMessage(kCtxViewTree, midiData) != kcNull
		|| ih->HandleMIDIMessage(kCtxAllContexts, midiData) != kcNull;

	uint8 midiByte1 = MIDIEvents::GetDataByte1FromEvent(midiData);
	int volume;
	switch(MIDIEvents::GetTypeFromEvent(midiData))
	{
	case MIDIEvents::evNoteOn:
		volume = MIDIEvents::GetDataByte2FromEvent(midiData);
		if(volume > 0)
		{
			PlayItem(GetSelectedItem(), midiByte1 + NOTE_MIN, Util::muldivr(volume, 256, 127));
			return 1;
		}
		MPT_FALLTHROUGH;
	case MIDIEvents::evNoteOff:
		PlayItem(GetSelectedItem(), NOTE_NOTECUT);
		return 1;
	}
	return 0;
}


void CModTree::OnKillFocus(CWnd* pNewWnd)
{
	CTreeCtrl::OnKillFocus(pNewWnd);
	CMainFrame::GetMainFrame()->m_bModTreeHasFocus = false;
	if(pNewWnd != nullptr)
		CMainFrame::GetMainFrame()->SetMidiRecordWnd(pNewWnd->m_hWnd);
}


void CModTree::OnSetFocus(CWnd* pOldWnd)
{
	CTreeCtrl::OnSetFocus(pOldWnd);
	CMainFrame::GetMainFrame()->m_bModTreeHasFocus = true;
	CMainFrame::GetMainFrame()->SetMidiRecordWnd(m_hWnd);
}


bool CModTree::IsItemExpanded(HTREEITEM hItem)
{
	// checks if a treeview item is expanded.
	if(hItem == NULL) return false;
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.state = 0;
	tvi.stateMask = TVIS_EXPANDED;
	tvi.hItem = hItem;
	GetItem(&tvi);
	return (tvi.state & TVIS_EXPANDED) != 0;
}


void CModTree::OnCloseItem()
{
	HTREEITEM hItem = GetSelectedItem();
	CModDoc *pModDoc = GetDocumentFromItem(hItem);
	if(pModDoc == nullptr) return;
	// Spam our message to the first available view
	POSITION pos = pModDoc->GetFirstViewPosition();
	if(pos == nullptr) return;
	CView* pView = pModDoc->GetNextView(pos);
	if (pView) pView->PostMessage(WM_COMMAND, ID_FILE_CLOSE);
}


// Delete all children of a tree item
void CModTree::DeleteChildren(HTREEITEM hItem)
{
	if(hItem != nullptr)
	{
		HTREEITEM hChildItem;
		while((hChildItem = GetChildItem(hItem)) != nullptr)
		{
			DeleteItem(hChildItem);
		}
	}
}


// Get the n-th child of a tree node
HTREEITEM CModTree::GetNthChildItem(HTREEITEM hItem, int index)
{
	HTREEITEM hChildItem = nullptr;
	if(hItem != nullptr && ItemHasChildren(hItem))
	{
		hChildItem = GetChildItem(hItem);
		while(index-- > 0)
		{
			hChildItem = GetNextSiblingItem(hChildItem);
		}
	}
	return hChildItem;
}


// Gets the root parent of an item, i.e. if C is a child of B and B is a child of A, GetParentRootItem(C) returns A.
// A root item is considered to be its own parent, i.e. the returned value is only ever NULL if the input value was NULL.
HTREEITEM CModTree::GetParentRootItem(HTREEITEM hItem)
{
	while(hItem != nullptr)
	{
		const HTREEITEM h = GetParentItem(hItem);
		if(h == nullptr || h == hItem) break;
		hItem = h;
	}
	return hItem;
}


// Editing sample, instrument, order, pattern, etc. labels
void CModTree::OnBeginLabelEdit(NMHDR *nmhdr, LRESULT *result)
{
	NMTVDISPINFO *info = reinterpret_cast<NMTVDISPINFO *>(nmhdr);
	CEdit *editCtrl = GetEditControl();
	const ModItem modItem = GetModItem(info->item.hItem);
	const CModDoc *modDoc = modItem.IsSongItem() ? GetDocumentFromItem(info->item.hItem) : nullptr;

	if(editCtrl != nullptr && modDoc != nullptr)
	{
		const CSoundFile &sndFile = modDoc->GetSoundFile();
		const CModSpecifications &modSpecs = sndFile.GetModSpecifications();
		std::string text;
		m_doLabelEdit = false;

		switch(modItem.type)
		{
		case MODITEM_ORDER:
			{
				PATTERNINDEX pat = sndFile.Order(static_cast<SEQUENCEINDEX>(modItem.val2)).at(static_cast<ORDERINDEX>(modItem.val1));
				if(pat == sndFile.Order.GetInvalidPatIndex())
					text = "---";
				else if(pat == sndFile.Order.GetIgnoreIndex())
					text = "+++";
				else
					text = mpt::fmt::val(pat);
				m_doLabelEdit = true;
			}
			break;

		case MODITEM_HDR_ORDERS:
			if(sndFile.Order.GetNumSequences() != 1 || sndFile.GetModSpecifications().sequencesMax <= 1)
			{
				break;
			}
			MPT_FALLTHROUGH;
		case MODITEM_SEQUENCE:
			if(modItem.val1 < sndFile.Order.GetNumSequences())
			{
				text = sndFile.Order(static_cast<SEQUENCEINDEX>(modItem.val1)).GetName();
				m_doLabelEdit = true;
			}
			break;

		case MODITEM_PATTERN:
			if(modItem.val1 < sndFile.Patterns.GetNumPatterns() && modSpecs.hasPatternNames)
			{
				text = sndFile.Patterns[modItem.val1].GetName();
				editCtrl->SetLimitText(MAX_PATTERNNAME - 1);
				m_doLabelEdit = true;
			}
			break;

		case MODITEM_SAMPLE:
			if(modItem.val1 <= sndFile.GetNumSamples())
			{
				text = sndFile.m_szNames[modItem.val1];
				editCtrl->SetLimitText(modSpecs.sampleNameLengthMax);
				m_doLabelEdit = true;
			}
			break;

		case MODITEM_INSTRUMENT:
			if(modItem.val1 <= sndFile.GetNumInstruments() && sndFile.Instruments[modItem.val1] != nullptr)
			{
				text = sndFile.Instruments[modItem.val1]->name;
				editCtrl->SetLimitText(modSpecs.instrNameLengthMax);
				m_doLabelEdit = true;
			}
			break;
		}

		if(m_doLabelEdit)
		{
			CMainFrame::GetInputHandler()->Bypass(true);
			editCtrl->SetWindowText(mpt::ToCString(sndFile.GetCharsetInternal(), text));
			*result = FALSE;
			return;
		}
	}
	*result = TRUE;
}


// End editing sample, instrument, order, pattern, etc. labels
void CModTree::OnEndLabelEdit(NMHDR *nmhdr, LRESULT *result)
{
	CMainFrame::GetInputHandler()->Bypass(false);
	m_doLabelEdit = false;

	NMTVDISPINFO *info = reinterpret_cast<NMTVDISPINFO *>(nmhdr);
	const ModItem modItem = GetModItem(info->item.hItem);
	CModDoc *modDoc = modItem.IsSongItem() ? GetDocumentFromItem(info->item.hItem) : nullptr;

	if(info->item.pszText != nullptr && modDoc != nullptr)
	{
		CSoundFile &sndFile = modDoc->GetSoundFile();
		const CModSpecifications &modSpecs = sndFile.GetModSpecifications();

		const std::string itemText = mpt::ToCharset(sndFile.GetCharsetInternal(), CString(info->item.pszText));
		switch(modItem.type)
		{
		case MODITEM_ORDER:
			if(!itemText.empty())
			{
				PATTERNINDEX pat = ConvertStrTo<PATTERNINDEX>(itemText);
				bool valid = true;
				if(itemText[0] == '-')
				{
					pat = sndFile.Order.GetInvalidPatIndex();
				} else if(itemText[0] == '+')
				{
					if(modSpecs.hasIgnoreIndex)
						pat = sndFile.Order.GetIgnoreIndex();
					else
						valid = false;
				} else
				{
					valid = (pat < sndFile.Patterns.GetNumPatterns());
				}
				PATTERNINDEX &target = sndFile.Order(static_cast<SEQUENCEINDEX>(modItem.val2))[static_cast<ORDERINDEX>(modItem.val1)];
				if(valid && pat != target)
				{
					target = pat;
					modDoc->SetModified();
					modDoc->UpdateAllViews(nullptr, SequenceHint().Data());
				}
			} else
			{
				MessageBeep(MB_ICONWARNING);
			}
			break;

		case MODITEM_HDR_ORDERS:
		case MODITEM_SEQUENCE:
			if(modItem.val1 < sndFile.Order.GetNumSequences() && sndFile.Order(static_cast<SEQUENCEINDEX>(modItem.val1)).GetName() != itemText)
			{
				sndFile.Order(static_cast<SEQUENCEINDEX>(modItem.val1)).SetName(itemText);
				modDoc->SetModified();
				modDoc->UpdateAllViews(nullptr, SequenceHint(static_cast<SEQUENCEINDEX>(modItem.val1)).Data().Names());
			}
			break;

		case MODITEM_PATTERN:
			if(modItem.val1 < sndFile.Patterns.GetNumPatterns() && modSpecs.hasPatternNames && sndFile.Patterns[modItem.val1].GetName() != itemText)
			{
				sndFile.Patterns[modItem.val1].SetName(itemText);
				modDoc->SetModified();
				modDoc->UpdateAllViews(nullptr, PatternHint(static_cast<PATTERNINDEX>(modItem.val1)).Data().Names());
			}
			break;

		case MODITEM_SAMPLE:
			if(modItem.val1 <= sndFile.GetNumSamples() && sndFile.m_szNames[modItem.val1] != itemText)
			{
				mpt::String::CopyN(sndFile.m_szNames[modItem.val1], itemText.c_str(), modSpecs.sampleNameLengthMax);
				modDoc->SetModified();
				modDoc->UpdateAllViews(nullptr, SampleHint(static_cast<SAMPLEINDEX>(modItem.val1)).Info().Names());
			}
			break;

		case MODITEM_INSTRUMENT:
			if(modItem.val1 <= sndFile.GetNumInstruments() && sndFile.Instruments[modItem.val1] != nullptr && sndFile.Instruments[modItem.val1]->name != itemText)
			{
				mpt::String::CopyN(sndFile.Instruments[modItem.val1]->name, itemText.c_str(), modSpecs.instrNameLengthMax);
				modDoc->SetModified();
				modDoc->UpdateAllViews(nullptr, InstrumentHint(static_cast<INSTRUMENTINDEX>(modItem.val1)).Info().Names());
			}
			break;
		}
	}
	*result = FALSE;
}


// Drop files from Windows
void CModTree::OnDropFiles(HDROP hDropInfo)
{
	bool refreshDLS = false;
	const UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	CMainFrame::GetMainFrame()->SetForegroundWindow();
	for(UINT f = 0; f < nFiles; f++)
	{
		UINT size = ::DragQueryFile(hDropInfo, f, nullptr, 0) + 1;
		std::vector<TCHAR> fileName(size, _T('\0'));
		if(::DragQueryFile(hDropInfo, f, fileName.data(), size))
		{
			mpt::PathString file(mpt::PathString::FromNative(fileName.data()));
			if(IsSampleBrowser())
			{
				// Set sample browser location to this directory or file
				const bool isSong = !file.IsDirectory();
				CModTree *dirBrowser = CMainFrame::GetMainFrame()->GetUpperTreeview();
				dirBrowser->m_InstrLibPath = file.GetPath();
				if(isSong) dirBrowser->RefreshInstrumentLibrary();
				dirBrowser->InstrumentLibraryChDir(file.GetFullFileName(), isSong);
				break;
			} else
			{
				if(CTrackApp::AddDLSBank(file))
				{
					refreshDLS = true;
				} else
				{
					// Pass message on
					theApp.OpenDocumentFile(file.ToCString());
				}
			}
		}
	}
	if(refreshDLS)
	{
		RefreshDlsBanks();
	}
	::DragFinish(hDropInfo);
}


OPENMPT_NAMESPACE_END
