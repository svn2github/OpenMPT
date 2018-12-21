/*
 * SelectPluginDialog.cpp
 * ----------------------
 * Purpose: Dialog for adding plugins to a song.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#ifndef NO_PLUGINS

#include "Mptrack.h"
#include "Mainfrm.h"
#include "InputHandler.h"
#include "ImageLists.h"
#include "Moddoc.h"
#include "../common/mptStringBuffer.h"
#include "FileDialog.h"
#include "../soundlib/plugins/PluginManager.h"
#include "../soundlib/plugins/PlugInterface.h"
#include "SelectPluginDialog.h"
#include "../pluginBridge/BridgeWrapper.h"
#include "FolderScanner.h"


OPENMPT_NAMESPACE_BEGIN

/////////////////////////////////////////////////////////////////////////////////
// Plugin selection dialog


BEGIN_MESSAGE_MAP(CSelectPluginDlg, CDialog)
	ON_NOTIFY(TVN_SELCHANGED,		IDC_TREE1, &CSelectPluginDlg::OnSelChanged)
	ON_NOTIFY(NM_DBLCLK,			IDC_TREE1, &CSelectPluginDlg::OnSelDblClk)
	ON_COMMAND(IDC_BUTTON1,			&CSelectPluginDlg::OnAddPlugin)
	ON_COMMAND(IDC_BUTTON3,			&CSelectPluginDlg::OnScanFolder)
	ON_COMMAND(IDC_BUTTON2,			&CSelectPluginDlg::OnRemovePlugin)
	ON_COMMAND(IDC_CHECK1,			&CSelectPluginDlg::OnSetBridge)
	ON_COMMAND(IDC_CHECK2,			&CSelectPluginDlg::OnSetBridge)
	ON_EN_CHANGE(IDC_NAMEFILTER,	&CSelectPluginDlg::OnNameFilterChanged)
	ON_EN_CHANGE(IDC_PLUGINTAGS,	&CSelectPluginDlg::OnPluginTagsChanged)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


void CSelectPluginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_treePlugins);
	DDX_Control(pDX, IDC_CHECK1, m_chkBridge);
	DDX_Control(pDX, IDC_CHECK2, m_chkShare);
}


CSelectPluginDlg::CSelectPluginDlg(CModDoc *pModDoc, PLUGINDEX nPlugSlot, CWnd *parent)
	: CDialog(IDD_SELECTMIXPLUGIN, parent)
	, m_pModDoc(pModDoc)
	, m_nPlugSlot(nPlugSlot)
	, m_pPlugin(nullptr)
{
	if(m_pModDoc && 0 <= m_nPlugSlot && m_nPlugSlot < MAX_MIXPLUGINS)
	{
		m_pPlugin = &(pModDoc->GetSoundFile().m_MixPlugins[m_nPlugSlot]);
	}

	CMainFrame::GetInputHandler()->Bypass(true);
}


CSelectPluginDlg::~CSelectPluginDlg()
{
	CMainFrame::GetInputHandler()->Bypass(false);
}


BOOL CSelectPluginDlg::OnInitDialog()
{
	DWORD dwRemove = TVS_EDITLABELS|TVS_SINGLEEXPAND;
	DWORD dwAdd = TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	CDialog::OnInitDialog();
	m_treePlugins.ModifyStyle(dwRemove, dwAdd);
	m_treePlugins.SetImageList(&CMainFrame::GetMainFrame()->m_MiscIcons, TVSIL_NORMAL);

	if (m_pPlugin)
	{
		CString targetSlot;
		targetSlot.Format(_T("&Put in FX%02u"), m_nPlugSlot + 1);
		SetDlgItemText(IDOK, targetSlot);
		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);
	} else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
	}

	const int dpiX = Util::GetDPIx(m_hWnd);
	const int dpiY = Util::GetDPIy(m_hWnd);
	CRect rect
	(
		CPoint(MulDiv(TrackerSettings::Instance().gnPlugWindowX, dpiX, 96), MulDiv(TrackerSettings::Instance().gnPlugWindowY, dpiY, 96)),
		CSize(MulDiv(TrackerSettings::Instance().gnPlugWindowWidth, dpiX, 96), MulDiv(TrackerSettings::Instance().gnPlugWindowHeight, dpiY, 96))
	);
	::MapWindowPoints(GetParent()->m_hWnd, HWND_DESKTOP, (CPoint *)&rect, 2);
	WINDOWPLACEMENT wnd;
	wnd.length = sizeof(wnd);
	GetWindowPlacement(&wnd);
	wnd.showCmd = SW_SHOW;
	wnd.rcNormalPosition = rect;
	SetWindowPlacement(&wnd);

	UpdatePluginsList();
	OnSelChanged(NULL, NULL);
	return TRUE;
}


void CSelectPluginDlg::OnOK()
{
	if(m_pPlugin==nullptr) { CDialog::OnOK(); return; }

	bool changed = false;
	CVstPluginManager *pManager = theApp.GetPluginManager();
	VSTPluginLib *pNewPlug = GetSelectedPlugin();
	VSTPluginLib *pFactory = nullptr;
	IMixPlugin *pCurrentPlugin = nullptr;
	if (m_pPlugin) pCurrentPlugin = m_pPlugin->pMixPlugin;
	if ((pManager) && (pManager->IsValidPlugin(pNewPlug))) pFactory = pNewPlug;

	if (pFactory)
	{
		// Plugin selected
		if ((!pCurrentPlugin) || &pCurrentPlugin->GetPluginFactory() != pFactory)
		{
			CriticalSection cs;

			// Destroy old plugin, if there was one.
			m_pPlugin->Destroy();

			// Initialize plugin info
			MemsetZero(m_pPlugin->Info);
			m_pPlugin->Info.dwPluginId1 = pFactory->pluginId1;
			m_pPlugin->Info.dwPluginId2 = pFactory->pluginId2;
			m_pPlugin->editorX = m_pPlugin->editorY = int32_min;

#ifndef NO_VST
			if(m_pPlugin->Info.dwPluginId1 == Vst::kEffectMagic)
			{
				switch(m_pPlugin->Info.dwPluginId2)
				{
					// Enable drymix by default for these known plugins
				case Vst::FourCC("Scop"):
					m_pPlugin->SetWetMix();
					break;
				}
			}
#endif // NO_VST

			mpt::String::Copy(m_pPlugin->Info.szName, pFactory->libraryName.ToLocale().c_str());
			mpt::String::Copy(m_pPlugin->Info.szLibraryName, pFactory->libraryName.ToUTF8().c_str());

			cs.Leave();

			// Now, create the new plugin
			if(pManager && m_pModDoc)
			{
				pManager->CreateMixPlugin(*m_pPlugin, m_pModDoc->GetSoundFile());
				if (m_pPlugin->pMixPlugin)
				{
					IMixPlugin *p = m_pPlugin->pMixPlugin;
					const CString name = p->GetDefaultEffectName();
					if(!name.IsEmpty())
					{
						mpt::String::Copy(m_pPlugin->Info.szName, mpt::ToCharset(mpt::CharsetLocale, name));
					}
					// Check if plugin slot is already assigned to any instrument, and if not, create one.
					if(p->IsInstrument() && m_pModDoc->HasInstrumentForPlugin(m_nPlugSlot) == INSTRUMENTINDEX_INVALID)
					{
						m_pModDoc->InsertInstrumentForPlugin(m_nPlugSlot);
					}
				} else
				{
					MemsetZero(m_pPlugin->Info);
				}
			}
			changed = true;
		}
	} else if(m_pPlugin->IsValidPlugin())
	{
		// No effect
		CriticalSection cs;
		m_pPlugin->Destroy();
		// Clear plugin info
		MemsetZero(m_pPlugin->Info);
		changed = true;
	}

	//remember window size:
	SaveWindowPos();

	if(changed)
	{
		if(m_pPlugin->Info.dwPluginId2)
			TrackerSettings::Instance().gnPlugWindowLast = m_pPlugin->Info.dwPluginId2;
		if(m_pModDoc)
		{
			m_pModDoc->UpdateAllViews(nullptr, PluginHint(static_cast<PLUGINDEX>(m_nPlugSlot + 1)).Info().Names());
		}
		CDialog::OnOK();
	} else
	{
		CDialog::OnCancel();
	}
}


void CSelectPluginDlg::OnCancel()
{
	//remember window size:
	SaveWindowPos();
	CDialog::OnCancel();
}


VSTPluginLib* CSelectPluginDlg::GetSelectedPlugin()
{
	HTREEITEM item = m_treePlugins.GetSelectedItem();
	if(item)
		return reinterpret_cast<VSTPluginLib *>(m_treePlugins.GetItemData(item));
	else
		return nullptr;
}


void CSelectPluginDlg::SaveWindowPos() const
{
	WINDOWPLACEMENT wnd;
	wnd.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wnd);
	CRect rect = wnd.rcNormalPosition;
	::MapWindowPoints(HWND_DESKTOP, GetParent()->m_hWnd, (CPoint *)&rect, 2);
	const int dpiX = Util::GetDPIx(m_hWnd);
	const int dpiY = Util::GetDPIy(m_hWnd);
	TrackerSettings::Instance().gnPlugWindowX = MulDiv(rect.left, 96, dpiX);
	TrackerSettings::Instance().gnPlugWindowY = MulDiv(rect.top, 96, dpiY);
	TrackerSettings::Instance().gnPlugWindowWidth  = MulDiv(rect.Width(), 96, dpiY);
	TrackerSettings::Instance().gnPlugWindowHeight = MulDiv(rect.Height(), 96, dpiX);
}


BOOL CSelectPluginDlg::PreTranslateMessage(MSG *pMsg)
{
	// Use up/down keys to navigate in tree view, even if search field is focussed.
	if(pMsg != nullptr && pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN) && GetFocus() != &m_treePlugins)
	{
		HTREEITEM selItem = m_treePlugins.GetSelectedItem();
		if(selItem == nullptr)
		{
			selItem = m_treePlugins.GetRootItem();
		}
		while((selItem = m_treePlugins.GetNextItem(selItem, pMsg->wParam == VK_UP ? TVGN_PREVIOUSVISIBLE : TVGN_NEXTVISIBLE)) != nullptr)
		{
			int nImage, nSelectedImage;
			m_treePlugins.GetItemImage(selItem, nImage, nSelectedImage);
			if(nImage != IMAGE_FOLDER)
			{
				m_treePlugins.SelectItem(selItem);
				m_treePlugins.EnsureVisible(selItem);
				return TRUE;
			}
		}
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CSelectPluginDlg::OnNameFilterChanged()
{
	// Update name filter text
	m_nameFilter = mpt::ToLowerCase(GetWindowTextUnicode(*GetDlgItem(IDC_NAMEFILTER)));

	UpdatePluginsList();
}


void CSelectPluginDlg::UpdatePluginsList(const VSTPluginLib *forceSelect)
{
	CVstPluginManager *pManager = theApp.GetPluginManager();

	m_treePlugins.SetRedraw(FALSE);
	m_treePlugins.DeleteAllItems();

	static const struct
	{
		VSTPluginLib::PluginCategory category;
		const TCHAR *description;
	} categories[] =
	{
		{ VSTPluginLib::catEffect,         _T("Audio Effects") },
		{ VSTPluginLib::catGenerator,      _T("Tone Generators") },
		{ VSTPluginLib::catRestoration,    _T("Audio Restauration") },
		{ VSTPluginLib::catSurroundFx,     _T("Surround Effects") },
		{ VSTPluginLib::catRoomFx,         _T("Room Effects") },
		{ VSTPluginLib::catSpacializer,    _T("Spacializers") },
		{ VSTPluginLib::catMastering,      _T("Mastering Plugins") },
		{ VSTPluginLib::catAnalysis,       _T("Analysis Plugins") },
		{ VSTPluginLib::catOfflineProcess, _T("Offline Processing") },
		{ VSTPluginLib::catShell,          _T("Shell Plugins") },
		{ VSTPluginLib::catUnknown,        _T("Unsorted") },
		{ VSTPluginLib::catDMO,            _T("DirectX Media Audio Effects") },
		{ VSTPluginLib::catSynth,          _T("Instrument Plugins") },
	};

	std::bitset<VSTPluginLib::numCategories> categoryUsed;
	HTREEITEM categoryFolders[VSTPluginLib::numCategories];
	for(size_t i = CountOf(categories); i != 0; )
	{
		i--;
		categoryFolders[categories[i].category] = AddTreeItem(categories[i].description, IMAGE_FOLDER, false);
	}

	HTREEITEM noPlug = AddTreeItem(_T("No plugin (empty slot)"), IMAGE_NOPLUGIN, false);
	HTREEITEM currentPlug = noPlug;
	enum PlugMatchQuality
	{
		kNoMatch,
		kSameIdAsLast,
		kSameIdAsLastWithPlatformMatch,
		kSameIdAsCurrent,
		kFoundCurrentPlugin,
	};
	PlugMatchQuality foundPlugin = kNoMatch;

	const int32 lastPluginID = TrackerSettings::Instance().gnPlugWindowLast;
	const bool nameFilterActive = !m_nameFilter.empty();
	const auto currentTags = mpt::String::Split<mpt::ustring>(m_nameFilter, U_(" "));

	if(pManager)
	{
		bool first = true;

		for(auto p : *pManager)
		{
			ASSERT(p);
			const VSTPluginLib &plug = *p;
			if(nameFilterActive)
			{
				// Apply name filter
				bool matches = false;
				// Search in plugin names
				{
					mpt::ustring displayName = mpt::ToLowerCase(plug.libraryName.ToUnicode());
					if(displayName.find(m_nameFilter, 0) != displayName.npos)
					{
						matches = true;
					}
				}
				// Search in plugin tags
				if(!matches)
				{
					mpt::ustring tags = mpt::ToLowerCase(plug.tags);
					for(const auto &tag : currentTags)
					{
						if(!tag.empty() && tags.find(tag, 0) != tags.npos)
						{
							matches = true;
							break;
						}
					}
				}
				// Search in plugin vendors
				if(!matches)
				{
					mpt::ustring vendor = mpt::ToLowerCase(mpt::ToUnicode(plug.vendor));
					if(vendor.find(m_nameFilter, 0) != vendor.npos)
					{
						matches = true;
					}
				}
				if(!matches) continue;
			}

			CString title = plug.libraryName.ToCString();
			if(!plug.IsNativeFromCache())
			{
				title += mpt::cformat(_T(" (%1-Bit)"))(plug.GetDllBits());
			}
			HTREEITEM h = AddTreeItem(title, plug.isInstrument ? IMAGE_PLUGININSTRUMENT : IMAGE_EFFECTPLUGIN, true, categoryFolders[plug.category], reinterpret_cast<LPARAM>(&plug));
			categoryUsed[plug.category] = true;

			if(nameFilterActive)
			{
				// If filter is active, expand nodes.
				m_treePlugins.EnsureVisible(h);
				if(first)
				{
					first = false;
					m_treePlugins.SelectItem(h);
				}
			}

			if(forceSelect != nullptr && &plug == forceSelect)
			{
				// Forced selection (e.g. just after add plugin)
				currentPlug = h;
				foundPlugin = kFoundCurrentPlugin;
			}

			if(m_pPlugin && foundPlugin < kFoundCurrentPlugin)
			{
				//Which plugin should be selected?
				if(m_pPlugin->pMixPlugin)
				{
					// Current slot's plugin
					IMixPlugin *pPlugin = m_pPlugin->pMixPlugin;
					if (&pPlugin->GetPluginFactory() == &plug)
					{
						currentPlug = h;
						foundPlugin = kFoundCurrentPlugin;
					}
				} else if(m_pPlugin->Info.dwPluginId1 != 0 || m_pPlugin->Info.dwPluginId2 != 0)
				{
					// Plugin with matching ID to current slot's plug
					if(plug.pluginId1 == m_pPlugin->Info.dwPluginId1
						&& plug.pluginId2 == m_pPlugin->Info.dwPluginId2)
					{
						currentPlug = h;
						foundPlugin = kSameIdAsCurrent;
					}
				} else if(plug.pluginId2 == lastPluginID && foundPlugin < kSameIdAsLastWithPlatformMatch)
				{
					// Previously selected plugin
					foundPlugin = plug.IsNativeFromCache() ? kSameIdAsLastWithPlatformMatch : kSameIdAsLast;
					currentPlug = h;
				}
			}
		}
	}

	// Remove empty categories
	for(size_t i = 0; i < CountOf(categoryFolders); i++)
	{
		if(!categoryUsed[i])
		{
			m_treePlugins.DeleteItem(categoryFolders[i]);
		}
	}

	m_treePlugins.SetRedraw(TRUE);

	if(!nameFilterActive || currentPlug != noPlug)
	{
		m_treePlugins.SelectItem(currentPlug);
	}
	m_treePlugins.SetItemState(currentPlug, TVIS_BOLD, TVIS_BOLD);
	m_treePlugins.EnsureVisible(currentPlug);
}


HTREEITEM CSelectPluginDlg::AddTreeItem(const TCHAR *title, int image, bool sort, HTREEITEM hParent, LPARAM lParam)
{
	return m_treePlugins.InsertItem(
		TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT,
		title,
		image, image,
		0, 0,
		lParam,
		hParent,
		(sort ? TVI_SORT : TVI_FIRST));
}


void CSelectPluginDlg::OnSelDblClk(NMHDR *, LRESULT *result)
{
	if(m_pPlugin == nullptr) return;

	HTREEITEM hSel = m_treePlugins.GetSelectedItem();
	int nImage, nSelectedImage;
	m_treePlugins.GetItemImage(hSel, nImage, nSelectedImage);

	if ((hSel) && (nImage != IMAGE_FOLDER)) OnOK();
	if (result) *result = 0;
}


void CSelectPluginDlg::OnSelChanged(NMHDR *, LRESULT *result)
{
	CVstPluginManager *pManager = theApp.GetPluginManager();
	VSTPluginLib *pPlug = GetSelectedPlugin();
	bool  showBoxes = false;
	BOOL enableTagsTextBox = FALSE;
	BOOL enableRemoveButton = FALSE;
	if (pManager != nullptr && pManager->IsValidPlugin(pPlug))
	{
		if(pPlug->vendor.IsEmpty())
			SetDlgItemText(IDC_VENDOR, _T(""));
		else
			SetDlgItemText(IDC_VENDOR, _T("Vendor: ") + pPlug->vendor);
		if(pPlug->dllPath.empty())
			SetDlgItemText(IDC_TEXT_CURRENT_VSTPLUG, _T("Built-in plugin"));
		else
			SetDlgItemText(IDC_TEXT_CURRENT_VSTPLUG, pPlug->dllPath.ToCString());
		SetDlgItemText(IDC_PLUGINTAGS, mpt::ToCString(pPlug->tags));
		enableRemoveButton = pPlug->isBuiltIn ? FALSE : TRUE;
#ifndef NO_VST
		if(pPlug->pluginId1 == Vst::kEffectMagic && !pPlug->isBuiltIn)
		{
			bool isBridgeAvailable =
					((pPlug->GetDllBits() == 32) && IsComponentAvailable(pluginBridge32))
				||
					((pPlug->GetDllBits() == 64) && IsComponentAvailable(pluginBridge64))
				;
			if(TrackerSettings::Instance().bridgeAllPlugins || !isBridgeAvailable)
			{
				m_chkBridge.EnableWindow(FALSE);
				m_chkBridge.SetCheck(isBridgeAvailable ? BST_CHECKED : BST_UNCHECKED);
			} else
			{
				bool native = pPlug->IsNative();

				m_chkBridge.EnableWindow(native ? TRUE : FALSE);
				m_chkBridge.SetCheck((pPlug->useBridge || !native) ? BST_CHECKED : BST_UNCHECKED);
			}

			m_chkShare.SetCheck(pPlug->shareBridgeInstance ? BST_CHECKED : BST_UNCHECKED);
			m_chkShare.EnableWindow(m_chkBridge.GetCheck() != BST_UNCHECKED);

			showBoxes = true;
		}
		enableTagsTextBox = TRUE;
#endif
	} else
	{
		SetDlgItemText(IDC_VENDOR, _T(""));
		SetDlgItemText(IDC_TEXT_CURRENT_VSTPLUG, _T(""));
		SetDlgItemText(IDC_PLUGINTAGS, _T(""));
	}
	GetDlgItem(IDC_PLUGINTAGS)->EnableWindow(enableTagsTextBox);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(enableRemoveButton);
	if(!showBoxes)
	{
		m_chkBridge.EnableWindow(FALSE);
		m_chkShare.EnableWindow(FALSE);
		m_chkBridge.SetCheck(BST_UNCHECKED);
		m_chkShare.SetCheck(BST_UNCHECKED);
	}
	if (result) *result = 0;
}


bool CSelectPluginDlg::VerifyPlug(VSTPluginLib *plug, CWnd *parent)
{
#ifndef NO_VST
	// TODO: Keep these lists up-to-date.
	static const struct
	{
		int32 id1;
		int32 id2;
		const char *name;
		const char *problem;
	} problemPlugs[] =
	{
		{ Vst::kEffectMagic, Vst::FourCC("Ni4S"), "Native Instruments B4", "*  v1.1.1 hangs on playback. Do not proceed unless you have v1.1.5 or newer.  *" },
		{ Vst::kEffectMagic, Vst::FourCC("mdaC"), "MDA Degrade", "*  Old versions of this plugin can crash OpenMPT.\nEnsure that you have the latest version of this plugin.  *" },
		{ Vst::kEffectMagic, Vst::FourCC("fV2s"), "Farbrausch V2", "*  This plugin can cause OpenMPT to freeze if being used in a combination with various other plugins.\nIt is recommended to not use V2 in combination with any other plugins or use it brigded mode only.  *" },
		{ Vst::kEffectMagic, Vst::FourCC("frV2"), "Farbrausch V2", "*  This plugin can cause OpenMPT to freeze if being used in a combination with various other plugins.\nIt is recommended to not use V2 in combination with any other plugins or use it brigded mode only.  *" },
		{ Vst::kEffectMagic, Vst::FourCC("MMID"), "MIDI Input Output", "* The MIDI Input / Output plugin is now built right into OpenMPT and should not be loaded from an external file. *" },
	};

	// Plugins that should always be bridged.
	static const struct
	{
		int32 id1;
		int32 id2;
		bool useBridge;
		bool shareInstance;
	} bridgedPlugs[] =
	{
		{ Vst::kEffectMagic, Vst::FourCC("fV2s"), true, true },  // Single instances of V2 can communicate (I think)
		{ Vst::kEffectMagic, Vst::FourCC("frV2"), true, false },
		{ Vst::kEffectMagic, Vst::FourCC("SKV3"), false, true }, // SideKick v3 always has to run in a shared instance
		{ Vst::kEffectMagic, Vst::FourCC("YWS!"), false, true }, // You Wa Shock ! always has to run in a shared instance
	};

	for(const auto &p : problemPlugs)
	{
		if(p.id2 == plug->pluginId2 && p.id1 == plug->pluginId1)
		{
			std::string s = mpt::format("WARNING: This plugin has been identified as %1,\nwhich is known to have the following problem with OpenMPT:\n\n%2\n\nWould you still like to add this plugin to the library?")(p.name, p.problem);
			if(Reporting::Confirm(s, false, false, parent) == cnfNo)
			{
				return false;
			}
			break;
		}
	}

	for(const auto &p : bridgedPlugs)
	{
		if(p.id2 == plug->pluginId2 && p.id1 == plug->pluginId1)
		{
			plug->useBridge = p.useBridge;
			plug->shareBridgeInstance = p.shareInstance;
			plug->WriteToCache();
			break;
		}
	}
#endif // NO_VST
	return true;
}


void CSelectPluginDlg::OnAddPlugin()
{
	FileDialog dlg = OpenFileDialog()
		.AllowMultiSelect()
		.DefaultExtension("dll")
		.ExtensionFilter("VST Plugins (*.dll)|*.dll||")
		.WorkingDirectory(TrackerSettings::Instance().PathPlugins.GetWorkingDir());
	if(!dlg.Show(this)) return;

	TrackerSettings::Instance().PathPlugins.SetWorkingDir(dlg.GetWorkingDirectory());

	CVstPluginManager *pManager = theApp.GetPluginManager();

	VSTPluginLib *plugLib = nullptr;
	bool update = false;

	for(const auto &file : dlg.GetFilenames())
	{
		if (pManager)
		{
			VSTPluginLib *lib = pManager->AddPlugin(file, mpt::ustring(), false);
			if(lib != nullptr)
			{
				update = true;
				if(!VerifyPlug(lib, this))
				{
					pManager->RemovePlugin(lib);
				} else
				{
					plugLib = lib;

					// If this plugin was missing anywhere, try loading it
					ReloadMissingPlugins(lib);
				}
			}
		}
	}
	if(update)
	{
		// Force selection to last added plug.
		UpdatePluginsList(plugLib);
	} else
	{
		Reporting::Error("At least one selected file was not a valid VST Plugin.");
	}
}


void CSelectPluginDlg::OnScanFolder()
{
	BrowseForFolder dlg(TrackerSettings::Instance().PathPlugins.GetWorkingDir(), "Select a folder that should be scanned for VST plugins (including sub-folders)");
	if(!dlg.Show(this)) return;

	TrackerSettings::Instance().PathPlugins.SetWorkingDir(dlg.GetDirectory());
	VSTPluginLib *plugLib = ScanPlugins(dlg.GetDirectory(), this);
	UpdatePluginsList(plugLib);

	// If any of the plugins was missing anywhere, try loading it
	const CVstPluginManager *pManager = theApp.GetPluginManager();
	for(auto p : *pManager)
	{
		ReloadMissingPlugins(p);
	}
}


VSTPluginLib *CSelectPluginDlg::ScanPlugins(const mpt::PathString &path, CWnd *parent)
{
	CVstPluginManager *pManager = theApp.GetPluginManager();
	VSTPluginLib *plugLib = nullptr;
	bool update = false;

	CDialog pluginScanDlg;
	pluginScanDlg.Create(IDD_SCANPLUGINS, parent);
	pluginScanDlg.CenterWindow(parent);
	pluginScanDlg.ModifyStyle(0, WS_SYSMENU, WS_SYSMENU);
	pluginScanDlg.ShowWindow(SW_SHOW);

	FolderScanner scan(path, FolderScanner::kOnlyFiles | FolderScanner::kFindInSubDirectories);
	mpt::PathString fileName;
	int files = 0;
	while(scan.Next(fileName) && pluginScanDlg.IsWindowVisible())
	{
		if(!mpt::PathString::CompareNoCase(fileName.GetFileExt(), P_(".dll")))
		{
			CWnd *text = pluginScanDlg.GetDlgItem(IDC_SCANTEXT);
			CString scanStr = _T("Scanning Plugin...\n") + fileName.ToCString();
			text->SetWindowText(scanStr);
			MSG msg;
			while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			VSTPluginLib *lib = pManager->AddPlugin(fileName, mpt::ustring(), false);
			if(lib)
			{
				update = true;
				if(!VerifyPlug(lib, parent))
				{
					pManager->RemovePlugin(lib);
				} else
				{
					plugLib = lib;
					files++;
				}
			}
		}
	}

	if(update)
	{
		// Force selection to last added plug.
		Reporting::Information(mpt::format("Found %1 plugin%2.")(files, files == 1 ? "" : "s").c_str(), parent);
		return plugLib;
	} else
	{
		Reporting::Error("Could not find any valid VST plugins.");
		return nullptr;
	}
}


// After adding new plugins, check if they were missing in any open songs.
void CSelectPluginDlg::ReloadMissingPlugins(const VSTPluginLib *lib) const
{
	CVstPluginManager *pManager = theApp.GetPluginManager();
	auto docs = theApp.GetOpenDocuments();
	for(auto &modDoc : docs)
	{
		CSoundFile &sndFile = modDoc->GetSoundFile();
		bool updateDoc = false;
		for(auto &plugin : sndFile.m_MixPlugins)
		{
			if(plugin.pMixPlugin == nullptr
				&& plugin.Info.dwPluginId1 == lib->pluginId1
				&& plugin.Info.dwPluginId2 == lib->pluginId2)
			{
				updateDoc = true;
				pManager->CreateMixPlugin(plugin, sndFile);
				if(plugin.pMixPlugin)
				{
					plugin.pMixPlugin->RestoreAllParameters(plugin.defaultProgram);
				}
			}
		}
		if(updateDoc)
		{
			modDoc->UpdateAllViews(nullptr, PluginHint().Info().Names());
			CMainFrame::GetMainFrame()->UpdateTree(modDoc, PluginHint().Info().Names());
		}
	}
}


void CSelectPluginDlg::OnRemovePlugin()
{
	const HTREEITEM pluginToDelete = m_treePlugins.GetSelectedItem();
	VSTPluginLib *pPlug = GetSelectedPlugin();
	CVstPluginManager *pManager = theApp.GetPluginManager();

	if ((pManager) && (pPlug))
	{
		if(pManager->RemovePlugin(pPlug))
		{
			m_treePlugins.DeleteItem(pluginToDelete);
		}
	}
}


void CSelectPluginDlg::OnSetBridge()
{
	VSTPluginLib *plug = GetSelectedPlugin();
	if(plug)
	{
		if(m_chkBridge.IsWindowEnabled())
		{
			// Only update this setting if the current setting isn't an enforced setting (e.g. because plugin isn't native).
			// This has the advantage that plugins don't get force-checked when switching between 32-bit and 64-bit versions of OpenMPT.
			plug->useBridge = m_chkBridge.GetCheck() != BST_UNCHECKED;
		}
		m_chkShare.EnableWindow(m_chkBridge.GetCheck() != BST_UNCHECKED);
		plug->shareBridgeInstance = m_chkShare.GetCheck() != BST_UNCHECKED;
		plug->WriteToCache();
	}
}


void CSelectPluginDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	const int dpiX = Util::GetDPIx(m_hWnd);
	const int dpiY = Util::GetDPIy(m_hWnd);

	if (m_treePlugins)
	{
		const int leftOff = MulDiv(8, dpiX, 96), leftOffFrame = MulDiv(18, dpiX, 96);
		m_treePlugins.MoveWindow(leftOff, MulDiv(36, dpiY, 96), cx - MulDiv(109, dpiX, 96), cy - MulDiv(158, dpiY, 96), FALSE);

		GetDlgItem(IDC_STATIC_VSTNAMEFILTER)->MoveWindow(leftOff, MulDiv(11, dpiY, 96), MulDiv(40, dpiX, 96), MulDiv(21, dpiY, 96), FALSE);
		GetDlgItem(IDC_NAMEFILTER)->MoveWindow(MulDiv(40, dpiX, 96), MulDiv(8, dpiY, 96), cx - MulDiv(141, dpiX, 96), MulDiv(21, dpiY, 96), FALSE);

		GetDlgItem(IDC_PLUGFRAME)->MoveWindow(leftOff, cy - MulDiv(5 * 20 + 15, dpiY, 96), cx - MulDiv(22, dpiX, 96), MulDiv(5 * 20 + 10, dpiY, 96), FALSE);

		GetDlgItem(IDC_STATIC_PLUGINTAGS)->MoveWindow(leftOffFrame, cy - MulDiv(94, dpiY, 96), MulDiv(40, dpiX, 96), MulDiv(21, dpiY, 96), FALSE);
		GetDlgItem(IDC_PLUGINTAGS)->MoveWindow(leftOffFrame + MulDiv(32, dpiX, 96), cy - MulDiv(97, dpiY, 96), cx - MulDiv(76, dpiX, 96), MulDiv(21, dpiY, 96), FALSE);

		m_chkBridge.MoveWindow(leftOffFrame, cy - MulDiv(72, dpiY, 96), MulDiv(110, dpiX, 96), MulDiv(20, dpiY, 96), FALSE);
		m_chkShare.MoveWindow(leftOffFrame + MulDiv(112, dpiX, 96), cy - MulDiv(72, dpiY, 96), cx - MulDiv(148, dpiX, 96), MulDiv(20, dpiY, 96), FALSE);

		GetDlgItem(IDC_TEXT_CURRENT_VSTPLUG)->MoveWindow(leftOffFrame, cy - MulDiv(48, dpiY, 96), cx - MulDiv(37, dpiX, 96), MulDiv(20, dpiY, 96), FALSE);
		GetDlgItem(IDC_VENDOR)->MoveWindow(leftOffFrame, cy - MulDiv(28, dpiY, 96), cx - MulDiv(37, dpiX, 96), MulDiv(20, dpiY, 96), FALSE);

		const int rightOff = cx - MulDiv(90, dpiX, 96);	// Offset of right button column
		const int buttonWidth = MulDiv(80, dpiX, 96), buttonHeight = MulDiv(23, dpiY, 96);
		GetDlgItem(IDOK)->MoveWindow(		rightOff,	MulDiv(8, dpiY, 96),		buttonWidth, buttonHeight, FALSE);
		GetDlgItem(IDCANCEL)->MoveWindow(	rightOff,	MulDiv(39, dpiY, 96),		buttonWidth, buttonHeight, FALSE);
		GetDlgItem(IDC_BUTTON1)->MoveWindow(rightOff,	cy - MulDiv(201, dpiY, 96),	buttonWidth, buttonHeight, FALSE);
		GetDlgItem(IDC_BUTTON3)->MoveWindow(rightOff,	cy - MulDiv(173, dpiY, 96),	buttonWidth, buttonHeight, FALSE);
		GetDlgItem(IDC_BUTTON2)->MoveWindow(rightOff,	cy - MulDiv(145, dpiY, 96),	buttonWidth, buttonHeight, FALSE);
		Invalidate();
	}
}


void CSelectPluginDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = MulDiv(350, Util::GetDPIx(m_hWnd), 96);
	lpMMI->ptMinTrackSize.y = MulDiv(310, Util::GetDPIy(m_hWnd), 96);
	CDialog::OnGetMinMaxInfo(lpMMI);
}


void CSelectPluginDlg::OnPluginTagsChanged()
{
	VSTPluginLib *plug = GetSelectedPlugin();
	if (plug)
	{
		plug->tags = GetWindowTextUnicode(*GetDlgItem(IDC_PLUGINTAGS));
	}
}


OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS
