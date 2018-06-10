/*
 * KeyConfigDlg.cpp
 * ----------------
 * Purpose: Implementation of OpenMPT's keyboard configuration dialog.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "KeyConfigDlg.h"
#include "FileDialog.h"
#include "../soundlib/mod_specifications.h"
#include "../soundlib/MIDIEvents.h"


OPENMPT_NAMESPACE_BEGIN


//***************************************************************************************//
// CCustEdit: customised CEdit control to catch keypresses.
// (does what CHotKeyCtrl does,but better)
//***************************************************************************************//

BEGIN_MESSAGE_MAP(CCustEdit, CEdit)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE(WM_MOD_MIDIMSG,		&CCustEdit::OnMidiMsg)
END_MESSAGE_MAP()


LRESULT CCustEdit::OnMidiMsg(WPARAM dwMidiDataParam, LPARAM)
{
	uint32 midiData = static_cast<uint32>(dwMidiDataParam);
	if(MIDIEvents::GetTypeFromEvent(midiData) == MIDIEvents::evControllerChange && MIDIEvents::GetDataByte2FromEvent(midiData) != 0 && isFocussed)
	{
		SetKey(ModMidi, MIDIEvents::GetDataByte1FromEvent(midiData));
		m_pOptKeyDlg->OnSetKeyChoice();
	}
	return 1;
}


BOOL CCustEdit::PreTranslateMessage(MSG *pMsg)
{
	if(pMsg)
	{
		if(pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
		{
			//if (!(pMsg->lparam & 0x40000000)) { // only on first presss
				SetKey(CMainFrame::GetInputHandler()->GetModifierMask(), static_cast<UINT>(pMsg->wParam));
			//}
			return -1; // Keypress handled, don't pass on message.
		} else if(pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP)
		{
			//if a key has been released but custom edit box is empty, we have probably just
			//navigated into the box with TAB or SHIFT-TAB. No need to set keychoice.
			if(code != 0 && !isDummy)
				m_pOptKeyDlg->OnSetKeyChoice();
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}


void CCustEdit::SetKey(FlagSet<Modifiers> inMod, UINT inCode)
{
	mod = inMod;
	code = inCode;
	//Setup display
	SetWindowText(KeyCombination::GetKeyText(mod, code));
}


void CCustEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);
	// Lock the input handler
	CMainFrame::GetInputHandler()->Bypass(true);
	// Accept MIDI input
	CMainFrame::GetMainFrame()->SetMidiRecordWnd(m_hWnd);

	isFocussed = true;
}


void CCustEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	//unlock the input handler
	CMainFrame::GetInputHandler()->Bypass(false);
	isFocussed = false;
}


//***************************************************************************************//
// COptionsKeyboard:
//
//***************************************************************************************//

// Initialisation

BEGIN_MESSAGE_MAP(COptionsKeyboard, CPropertyPage)
	ON_LBN_SELCHANGE(IDC_CHOICECOMBO,		&COptionsKeyboard::OnKeyChoiceSelect)
	ON_LBN_SELCHANGE(IDC_COMMAND_LIST,		&COptionsKeyboard::OnCommandKeySelChanged)
	ON_LBN_SELCHANGE(IDC_KEYCATEGORY,		&COptionsKeyboard::OnCategorySelChanged)
	ON_EN_UPDATE(IDC_CHORDDETECTWAITTIME,	&COptionsKeyboard::OnChordWaitTimeChanged) //rewbs.autochord
	ON_COMMAND(IDC_DELETE,					&COptionsKeyboard::OnDeleteKeyChoice)
	ON_COMMAND(IDC_RESTORE,					&COptionsKeyboard::OnRestoreKeyChoice)
	ON_COMMAND(IDC_LOAD,					&COptionsKeyboard::OnLoad)
	ON_COMMAND(IDC_SAVE,					&COptionsKeyboard::OnSave)
	ON_COMMAND(IDC_CHECKKEYDOWN,			&COptionsKeyboard::OnCheck)
	ON_COMMAND(IDC_CHECKKEYHOLD,			&COptionsKeyboard::OnCheck)
	ON_COMMAND(IDC_CHECKKEYUP,				&COptionsKeyboard::OnCheck)
	ON_COMMAND(IDC_NOTESREPEAT,				&COptionsKeyboard::OnNotesRepeat)
	ON_COMMAND(IDC_NONOTESREPEAT,			&COptionsKeyboard::OnNoNotesRepeat)
	ON_COMMAND(IDC_CLEARLOG,				&COptionsKeyboard::OnClearLog)
	ON_COMMAND(IDC_RESTORE_KEYMAP,			&COptionsKeyboard::OnRestoreDefaultKeymap)
	ON_EN_CHANGE(IDC_FIND,					&COptionsKeyboard::OnSearchTermChanged)
	ON_EN_CHANGE(IDC_FINDHOTKEY,			&COptionsKeyboard::OnFindHotKey)
	ON_EN_SETFOCUS(IDC_FINDHOTKEY,			&COptionsKeyboard::OnClearHotKey)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void COptionsKeyboard::DoDataExchange(CDataExchange *pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KEYCATEGORY,	m_cmbCategory);
	DDX_Control(pDX, IDC_COMMAND_LIST,	m_lbnCommandKeys);
	DDX_Control(pDX, IDC_CHOICECOMBO,	m_cmbKeyChoice);
	DDX_Control(pDX, IDC_CHORDDETECTWAITTIME, m_eChordWaitTime);//rewbs.autochord
	DDX_Control(pDX, IDC_KEYREPORT,		m_eReport);
	DDX_Control(pDX, IDC_CUSTHOTKEY,	m_eCustHotKey);
	DDX_Control(pDX, IDC_FINDHOTKEY,	m_eFindHotKey);
	DDX_Control(pDX, IDC_CHECKKEYDOWN,	m_bKeyDown);
	DDX_Control(pDX, IDC_CHECKKEYHOLD,	m_bKeyHold);
	DDX_Control(pDX, IDC_CHECKKEYUP,	m_bKeyUp);
	DDX_Control(pDX, IDC_FIND,			m_eFind);
}


BOOL COptionsKeyboard::OnSetActive()
{
	CMainFrame::m_nLastOptionsPage = OPTIONS_PAGE_KEYBOARD;
	return CPropertyPage::OnSetActive();
}



BOOL COptionsKeyboard::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_nCurCategory = -1;
	m_nCurHotKey = -1;
	m_nCurKeyChoice = -1;
	m_bModified = false;
	m_bChoiceModified = false;
	m_sFullPathName = TrackerSettings::Instance().m_szKbdFile;

	plocalCmdSet = new CCommandSet();
	plocalCmdSet->Copy(CMainFrame::GetInputHandler()->m_activeCommandSet.get());

	//Fill category combo and automatically selects first category
	DefineCommandCategories();
	for(size_t c = 0; c < commandCategories.size(); c++)
	{
		if (commandCategories[c].name && !commandCategories[c].commands.empty())
			m_cmbCategory.SetItemData(m_cmbCategory.AddString(commandCategories[c].name), c);
	}
	m_cmbCategory.SetCurSel(0);
	UpdateDialog();

	m_eCustHotKey.SetParent(m_hWnd, IDC_CUSTHOTKEY, this);
	m_eFindHotKey.SetParent(m_hWnd, IDC_FINDHOTKEY, this);
	m_eReport.FmtLines(TRUE);
	m_eReport.SetWindowText(_T(""));

	m_eChordWaitTime.SetWindowText(mpt::cfmt::val(TrackerSettings::Instance().gnAutoChordWaitTime));
	return TRUE;
}


void CommandCategory::AddCommands(CommandID first, CommandID last)
{
	int count = last - first + 1, val = first;
	commands.insert(commands.end(), count, kcNull);
	std::generate(commands.end() - count, commands.end(), [&val] { return static_cast<CommandID>(val++); });
}


// Filter commands: We only need user to see a select set off commands
// for each category
void COptionsKeyboard::DefineCommandCategories()
{
	{
		CommandCategory newCat(_T("Global keys"), kCtxAllContexts);

		newCat.AddCommands(kcStartFile, kcEndFile);
		newCat.separators.push_back(kcEndFile);			//--------------------------------------
		newCat.AddCommands(kcStartPlayCommands, kcEndPlayCommands);
		newCat.separators.push_back(kcEndPlayCommands);	//--------------------------------------
		newCat.AddCommands(kcStartEditCommands, kcEndEditCommands);
		newCat.separators.push_back(kcEndEditCommands);	//--------------------------------------
		newCat.AddCommands(kcStartView, kcEndView);
		newCat.separators.push_back(kcEndView);			//--------------------------------------
		newCat.AddCommands(kcStartMisc, kcEndMisc);
		newCat.separators.push_back(kcEndMisc);			//--------------------------------------
		newCat.commands.push_back(kcDummyShortcut);

		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  General [Top]"), kCtxCtrlGeneral);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  General [Bottom]"), kCtxViewGeneral);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Pattern Editor [Top]"), kCtxCtrlPatterns);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Pattern Editor - Order List"), kCtxCtrlOrderlist);

		newCat.AddCommands(kcStartOrderlistCommands, kcEndOrderlistCommands);
		newCat.separators.push_back(kcEndOrderlistNavigation);			//--------------------------------------
		newCat.separators.push_back(kcEndOrderlistEdit);				//--------------------------------------
		newCat.separators.push_back(kcEndOrderlistNum);					//--------------------------------------

		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Pattern Editor - Quick Channel Settings"), kCtxChannelSettings);
		newCat.AddCommands(kcStartChnSettingsCommands, kcEndChnSettingsCommands);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("    Pattern Editor - General"), kCtxViewPatterns);

		newCat.AddCommands(kcStartPlainNavigate, kcEndPlainNavigate);
		newCat.separators.push_back(kcEndPlainNavigate);				//--------------------------------------
		newCat.AddCommands(kcStartJumpSnap, kcEndJumpSnap);
		newCat.separators.push_back(kcEndJumpSnap);						//--------------------------------------
		newCat.AddCommands(kcStartHomeEnd, kcEndHomeEnd);
		newCat.separators.push_back(kcEndHomeEnd);						//--------------------------------------
		newCat.AddCommands(kcPrevPattern, kcNextPattern);
		newCat.separators.push_back(kcNextPattern);						//--------------------------------------
		newCat.AddCommands(kcStartSelect, kcEndSelect);
		newCat.separators.push_back(kcEndSelect);						//--------------------------------------
		newCat.commands.push_back(kcCopyAndLoseSelection);
		newCat.AddCommands(kcClearRow, kcInsertAllRows);
		newCat.separators.push_back(kcInsertAllRows);					//--------------------------------------
		newCat.AddCommands(kcStartChannelKeys, kcEndChannelKeys);
		newCat.separators.push_back(kcEndChannelKeys);					//--------------------------------------
		newCat.AddCommands(kcBeginTranspose, kcEndTranspose);
		newCat.separators.push_back(kcEndTranspose);					//--------------------------------------
		newCat.AddCommands(kcPatternAmplify, kcPatternShrinkSelection);
		newCat.separators.push_back(kcPatternShrinkSelection);			//--------------------------------------
		newCat.AddCommands(kcStartPatternEditMisc, kcEndPatternEditMisc);
		newCat.separators.push_back(kcEndPatternEditMisc);				//--------------------------------------
		newCat.AddCommands(kcStartPatternClipboard, kcEndPatternClipboard);
		newCat.separators.push_back(kcEndPatternClipboard);				//--------------------------------------

		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("        Pattern Editor - Note Column"), kCtxViewPatternsNote);

		newCat.AddCommands(kcVPStartNotes, kcVPEndNotes);
		newCat.separators.push_back(kcVPEndNotes);			//--------------------------------------
		newCat.AddCommands(kcSetOctave0, kcSetOctave9);
		newCat.separators.push_back(kcVPEndNotes);			//--------------------------------------
		newCat.AddCommands(kcStartNoteMisc, kcEndNoteMisc);

		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("        Pattern Editor - Instrument Column"), kCtxViewPatternsIns);
		newCat.AddCommands(kcSetIns0, kcSetIns9);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("        Pattern Editor - Volume Column"), kCtxViewPatternsVol);
		newCat.AddCommands(kcSetVolumeStart, kcSetVolumeEnd);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("        Pattern Editor - Effect Column"), kCtxViewPatternsFX);
		newCat.AddCommands(kcSetFXStart, kcSetFXEnd);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("        Pattern Editor - Effect Parameter Column"), kCtxViewPatternsFXparam);
		newCat.AddCommands(kcSetFXParam0, kcSetFXParamF);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Sample [Top]"), kCtxCtrlSamples);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("    Sample Editor"), kCtxViewSamples);

		newCat.AddCommands(kcStartSampleEditing, kcEndSampleEditing);
		newCat.separators.push_back(kcEndSampleEditing);		//--------------------------------------
		newCat.AddCommands(kcStartSampleMisc, kcEndSampleMisc);
		newCat.separators.push_back(kcEndSampleMisc);			//--------------------------------------
		newCat.AddCommands(kcStartSampleCues, kcEndSampleCueGroup);

		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Instrument Editor"), kCtxCtrlInstruments);
		newCat.AddCommands(kcStartInstrumentCtrlMisc, kcEndInstrumentCtrlMisc);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("    Envelope Editor"), kCtxViewInstruments);
		newCat.AddCommands(kcStartInstrumentMisc, kcEndInstrumentMisc);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Comments [Top]"), kCtxCtrlComments);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Comments [Bottom]"), kCtxViewComments);
		commandCategories.push_back(newCat);
	}

	{
		CommandCategory newCat(_T("  Plugin Editor"), kCtxVSTGUI);
		newCat.AddCommands(kcStartVSTGUICommands, kcEndVSTGUICommands);
		commandCategories.push_back(newCat);
	}

}


// Pure GUI methods

void COptionsKeyboard::UpdateDialog()
{
	OnCategorySelChanged();		// Fills command list and automatically selects first command.
	OnCommandKeySelChanged();	// Fills command key choice list for that command and automatically selects first choice.
}


void COptionsKeyboard::OnKeyboardChanged()
{
	OnSettingsChanged();
	UpdateDialog();
}


void COptionsKeyboard::OnCategorySelChanged()
{
	int cat = static_cast<int>(m_cmbCategory.GetItemData(m_cmbCategory.GetCurSel()));

	if(cat >= 0 && cat != m_nCurCategory)
	{
		// Changed category
		UpdateShortcutList(cat);
	}
}


// Force last active category to be selected in dropdown menu.
void COptionsKeyboard::UpdateCategory()
{
	for(int i = 0; i < m_cmbCategory.GetCount(); i++)
	{
		if((int)m_cmbCategory.GetItemData(i) == m_nCurCategory)
		{
			m_cmbCategory.SetCurSel(i);
			break;
		}
	}
}

void COptionsKeyboard::OnSearchTermChanged()
{
	CString findString;
	m_eFind.GetWindowText(findString);

	if(findString.IsEmpty())
	{
		UpdateCategory();
	}
	UpdateShortcutList(findString.IsEmpty() ? m_nCurCategory : -1);
}


void COptionsKeyboard::OnFindHotKey()
{
	if(m_eFindHotKey.code == 0)
	{
		UpdateCategory();
	}
	UpdateShortcutList(m_eFindHotKey.code == 0 ? m_nCurCategory : -1);
}


void COptionsKeyboard::OnClearHotKey()
{
	// Focus key search: Clear input
	m_eFindHotKey.SetKey(ModNone, 0);
}


// Fills command list and automatically selects first command.
void COptionsKeyboard::UpdateShortcutList(int category)
{
	CString findString;
	m_eFind.GetWindowText(findString);
	findString.MakeLower();

	const bool searchByName = !findString.IsEmpty(), searchByKey = (m_eFindHotKey.code != 0);
	const bool doSearch = (searchByName || searchByKey);

	int firstCat = category, lastCat = category;
	if(category == -1)
	{
		// We will search in all categories
		firstCat = 0;
		lastCat = static_cast<int>(commandCategories.size()) - 1;
	}

	CommandID curCommand  = static_cast<CommandID>(m_lbnCommandKeys.GetItemData( m_lbnCommandKeys.GetCurSel()));
	m_lbnCommandKeys.ResetContent();

	for(int cat = firstCat; cat <= lastCat; cat++)
	{
		// When searching, we also add the category names to the list.
		bool addCategoryName = (firstCat != lastCat);

		for(size_t cmd = 0; cmd < commandCategories[cat].commands.size(); cmd++)
		{
			CommandID com = (CommandID)commandCategories[cat].commands[cmd];

			CString cmdText = plocalCmdSet->GetCommandText(com);
			bool addKey = true;

			if(searchByKey)
			{
				addKey = false;
				int numChoices = plocalCmdSet->GetKeyListSize(com);
				for(int choice = 0; choice < numChoices; choice++)
				{
					const KeyCombination &kc = plocalCmdSet->GetKey(com, choice);
					if(kc.KeyCode() == m_eFindHotKey.code && kc.Modifier() == m_eFindHotKey.mod)
					{
						addKey = true;
						break;
					}
				}
			}
			if(searchByName && addKey)
			{
				addKey = (cmdText.MakeLower().Find(findString) >= 0);
			}

			if(addKey)
			{
				m_nCurCategory = cat;

				if(!plocalCmdSet->isHidden(com))
				{
					if(doSearch && addCategoryName)
					{
						const CString catName = _T("------ ") + commandCategories[cat].name.Trim() + _T(" ------");
						m_lbnCommandKeys.SetItemData(m_lbnCommandKeys.AddString(catName), DWORD_PTR(-1));
						addCategoryName = false;
					}

					int item = m_lbnCommandKeys.AddString(plocalCmdSet->GetCommandText(com));
					m_lbnCommandKeys.SetItemData(item, com);

					if(curCommand == com)
					{
						// Keep selection on previously selected string
						m_lbnCommandKeys.SetCurSel(item);
					}
				}

				if(commandCategories[cat].SeparatorAt(com))
					m_lbnCommandKeys.SetItemData(m_lbnCommandKeys.AddString(_T("------------------------------------------------------")), DWORD_PTR(-1));
			}
		}

	}

	if(m_lbnCommandKeys.GetCurSel() == -1)
	{
		m_lbnCommandKeys.SetCurSel(0);
	}
	OnCommandKeySelChanged();
}


// Fills  key choice list and automatically selects first key choice
void COptionsKeyboard::OnCommandKeySelChanged()
{
	CommandID nCmd  = static_cast<CommandID>(m_lbnCommandKeys.GetItemData( m_lbnCommandKeys.GetCurSel()));
	CString str;

	//Separator
	if (nCmd == kcNull)
	{
		m_cmbKeyChoice.SetWindowText(_T(""));
		m_cmbKeyChoice.EnableWindow(FALSE);
		m_eCustHotKey.SetWindowText(_T(""));
		m_eCustHotKey.EnableWindow(FALSE);
		m_bKeyDown.SetCheck(0);
		m_bKeyDown.EnableWindow(FALSE);
		m_bKeyHold.SetCheck(0);
		m_bKeyHold.EnableWindow(FALSE);
		m_bKeyUp.SetCheck(0);
		m_bKeyUp.EnableWindow(FALSE);
		m_nCurHotKey=-1;
	}

	//Fill "choice" list
	else if ((nCmd >= 0) && (nCmd != m_nCurHotKey) || m_bForceUpdate)	//have we changed command?
	{
		m_bForceUpdate = false;

		m_cmbKeyChoice.EnableWindow(TRUE);
		m_eCustHotKey.EnableWindow(TRUE);
		m_bKeyDown.EnableWindow(TRUE);
		m_bKeyHold.EnableWindow(TRUE);
		m_bKeyUp.EnableWindow(TRUE);

		m_nCurHotKey = nCmd;
		m_nCurCategory = GetCategoryFromCommandID(nCmd);

		m_cmbKeyChoice.ResetContent();
		int numChoices=plocalCmdSet->GetKeyListSize(nCmd);
		if ((nCmd<kcNumCommands) && (numChoices>0))
		{
			for (int i=0; i<numChoices; i++)
			{
				CString s = mpt::cformat(_T("Choice %1 (of %2)"))(i+1, numChoices);
				m_cmbKeyChoice.SetItemData(m_cmbKeyChoice.AddString(s), i);
			}
		}
		m_cmbKeyChoice.SetItemData(m_cmbKeyChoice.AddString(_T("<new>")), numChoices);
		m_cmbKeyChoice.SetCurSel(0);
		m_nCurKeyChoice = -1;
		OnKeyChoiceSelect();
	}
}

//Fills or clears key choice info
void COptionsKeyboard::OnKeyChoiceSelect()
{
	//CString str;
	int choice = static_cast<int>(m_cmbKeyChoice.GetItemData( m_cmbKeyChoice.GetCurSel()));
	CommandID cmd = (CommandID)m_nCurHotKey;

	//If nothing there, clear
	if (choice>=plocalCmdSet->GetKeyListSize(cmd) || choice<0)
	{
		m_nCurKeyChoice = choice;
		m_bForceUpdate=true;
		m_eCustHotKey.SetKey(ModNone, 0);
		m_bKeyDown.SetCheck(0);
		m_bKeyHold.SetCheck(0);
		m_bKeyUp.SetCheck(0);
		return;
	}

	//else, if changed, Fill
	if (choice != m_nCurKeyChoice || m_bForceUpdate)
	{
		m_nCurKeyChoice = choice;
		m_bForceUpdate = false;
		KeyCombination kc = plocalCmdSet->GetKey(cmd, choice);
		m_eCustHotKey.SetKey(kc.Modifier(), kc.KeyCode());

		if (kc.EventType() & kKeyEventDown) m_bKeyDown.SetCheck(1);
		else m_bKeyDown.SetCheck(0);
		if (kc.EventType() & kKeyEventRepeat) m_bKeyHold.SetCheck(1);
		else m_bKeyHold.SetCheck(0);
		if (kc.EventType() & kKeyEventUp) m_bKeyUp.SetCheck(1);
		else m_bKeyUp.SetCheck(0);
	}
}

//rewbs.autochord
void COptionsKeyboard::OnChordWaitTimeChanged()
{
	CString s;
	UINT val;
	m_eChordWaitTime.GetWindowText(s);
	val = _tstoi(s);
	if (val>5000)
	{
		val = 5000;
		m_eChordWaitTime.SetWindowText(_T("5000"));
	}
	OnSettingsChanged();
}
//end rewbs.autochord

// Change handling


void COptionsKeyboard::OnRestoreKeyChoice()
{
	KeyCombination kc;
	CommandID cmd = (CommandID)m_nCurHotKey;

	CInputHandler *ih=CMainFrame::GetInputHandler();

	// Do nothing if there's nothing to restore
	if (cmd<0 || m_nCurKeyChoice<0 || m_nCurKeyChoice>=ih->GetKeyListSize(cmd))
	{
		// Annoying message box is annoying.
		//Reporting::Error("Nothing to restore for this slot.", "Invalid key data", this);
		::MessageBeep(MB_ICONWARNING);
		return;
	}

	// Restore current key combination choice for currently selected command.
	kc = ih->m_activeCommandSet->GetKey(cmd, m_nCurKeyChoice);
	plocalCmdSet->Remove(m_nCurKeyChoice, cmd);
	plocalCmdSet->Add(kc, cmd, true, m_nCurKeyChoice);

	ForceUpdateGUI();
	return;
}

void COptionsKeyboard::OnDeleteKeyChoice()
{
	CommandID cmd = (CommandID)m_nCurHotKey;

	// Do nothing if there's no key defined for this slot.
	if (m_nCurHotKey<0 || m_nCurKeyChoice<0 || m_nCurKeyChoice>=plocalCmdSet->GetKeyListSize(cmd))
	{
		// Annoying message box is annoying.
		//Reporting::Warning("No key currently set for this slot.", "Invalid key data", this);
		::MessageBeep(MB_ICONWARNING);
		return;
	}

	//Delete current key combination choice for currently selected command.
	plocalCmdSet->Remove(m_nCurKeyChoice, cmd);

	ForceUpdateGUI();
	return;
}


void COptionsKeyboard::OnSetKeyChoice()
{
	CommandID cmd = (CommandID)m_nCurHotKey;
	if (cmd<0)
	{
		Reporting::Warning("Invalid slot.", "Invalid key data", this);
		return;
	}

	FlagSet<KeyEventType> event = kKeyEventNone;
	if(m_bKeyDown.GetCheck()) event |= kKeyEventDown;
	if(m_bKeyHold.GetCheck()) event |= kKeyEventRepeat;
	if(m_bKeyUp.GetCheck()) event |= kKeyEventUp;

	KeyCombination kc((commandCategories[m_nCurCategory]).id, m_eCustHotKey.mod, m_eCustHotKey.code, event);
	//detect invalid input
	if (!kc.KeyCode())
	{
		Reporting::Warning("You need to say to which key you'd like to map this command to.", "Invalid key data", this);
		return;
	}
	if (!kc.EventType())
	{
		::MessageBeep(MB_ICONWARNING);
		kc.EventType(kKeyEventDown);
	}

	bool add = true;
	std::pair<CommandID, KeyCombination> conflictCmd;
	if((conflictCmd = plocalCmdSet->IsConflicting(kc, cmd)).first != kcNull
		&& conflictCmd.first != cmd
		&& !plocalCmdSet->IsCrossContextConflict(kc, conflictCmd.second))
	{
		ConfirmAnswer delOld = Reporting::Confirm(_T("New shortcut (") + kc.GetKeyText() + _T(") has the same key combination as ") + plocalCmdSet->GetCommandText(conflictCmd.first) + _T(" in ") + conflictCmd.second.GetContextText() + _T(".\nDo you want to delete the other shortcut, only keeping the new one?"), _T("Shortcut Conflict"), true, false, this);
		if(delOld == cnfYes)
		{
			plocalCmdSet->Remove(conflictCmd.second, conflictCmd.first);
		} else if(delOld == cnfCancel)
		{
			// Cancel altogther; restore original choice
			add = false;
			if(m_nCurKeyChoice >= 0 && m_nCurKeyChoice < plocalCmdSet->GetKeyListSize(cmd))
			{
				KeyCombination origKc = plocalCmdSet->GetKey(cmd, m_nCurKeyChoice);
				m_eCustHotKey.SetKey(origKc.Modifier(), origKc.KeyCode());
			} else
			{
				m_eCustHotKey.SetWindowText(_T(""));
			}
		}
	}

	if(add)
	{
		CString report, reportHistory;
		//process valid input
		plocalCmdSet->Remove(m_nCurKeyChoice, cmd);
		report = plocalCmdSet->Add(kc, cmd, true, m_nCurKeyChoice);

		//Update log
		m_eReport.GetWindowText(reportHistory);
		m_eReport.SetWindowText(report + reportHistory);
		ForceUpdateGUI();
	}

	m_bModified = false;
	return;
}


void COptionsKeyboard::OnOK()
{
	CMainFrame::GetInputHandler()->SetNewCommandSet(plocalCmdSet);

	CString cs;
	m_eChordWaitTime.GetWindowText(cs);
	TrackerSettings::Instance().gnAutoChordWaitTime = _tstoi(cs);

	CPropertyPage::OnOK();
}


void COptionsKeyboard::OnDestroy()
{
	CPropertyPage::OnDestroy();
	delete plocalCmdSet;
}


void COptionsKeyboard::OnLoad()
{
	FileDialog dlg = OpenFileDialog()
		.DefaultExtension("mkb")
		.DefaultFilename(m_sFullPathName)
		.ExtensionFilter("OpenMPT Key Bindings (*.mkb)|*.mkb||")
		.WorkingDirectory(TrackerSettings::Instance().m_szKbdFile);
	if(!dlg.Show(this)) return;

	m_sFullPathName = dlg.GetFirstFile();
	plocalCmdSet->LoadFile(m_sFullPathName);
	ForceUpdateGUI();
}


void COptionsKeyboard::OnSave()
{
	FileDialog dlg = SaveFileDialog()
		.DefaultExtension("mkb")
		.DefaultFilename(m_sFullPathName)
		.ExtensionFilter("OpenMPT Key Bindings (*.mkb)|*.mkb||")
		.WorkingDirectory(TrackerSettings::Instance().m_szKbdFile);
	if(!dlg.Show(this)) return;

	m_sFullPathName = dlg.GetFirstFile();
	plocalCmdSet->SaveFile(m_sFullPathName);
}


void COptionsKeyboard::OnNotesRepeat()
{
	plocalCmdSet->QuickChange_NotesRepeat(true);
	ForceUpdateGUI();
}


void COptionsKeyboard::OnNoNotesRepeat()
{
	plocalCmdSet->QuickChange_NotesRepeat(false);
	ForceUpdateGUI();
}


void COptionsKeyboard::ForceUpdateGUI()
{
	m_bForceUpdate = true;						// m_nCurKeyChoice and m_nCurHotKey haven't changed, yet we still want to update.
	int ntmpChoice = m_nCurKeyChoice;			// next call will overwrite m_nCurKeyChoice
	OnCommandKeySelChanged();					// update keychoice list
	m_cmbKeyChoice.SetCurSel(ntmpChoice);		// select fresh keychoice (thus restoring m_nCurKeyChoice)
	OnKeyChoiceSelect();						// update key data
	OnSettingsChanged();						// Enable "apply" button
}


void COptionsKeyboard::OnClearLog()
{
	m_eReport.SetWindowText(_T(""));
	ForceUpdateGUI();
}


void COptionsKeyboard::OnRestoreDefaultKeymap()
{
	if(Reporting::Confirm("Discard all custom changes and restore default key configuration?", false, true, this) == cnfYes)
	{
		plocalCmdSet->LoadDefaultKeymap();
		ForceUpdateGUI();
	}
}


int COptionsKeyboard::GetCategoryFromCommandID(CommandID command) const
{
	for(size_t cat = 0; cat < commandCategories.size(); cat++)
	{
		for(size_t cmd = 0; cmd < commandCategories[cat].commands.size(); cmd++)
		{
			if(commandCategories[cat].commands[cmd] == command)
			{
				return static_cast<int>(cat);
			}
		}
	}
	return -1;
}


OPENMPT_NAMESPACE_END
