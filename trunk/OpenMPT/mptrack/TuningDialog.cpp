/*
 * tuningDialog.cpp
 * ----------------
 * Purpose: Alternative sample tuning configuration dialog.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "mptrack.h"
#include "TuningDialog.h"
#include "TrackerSettings.h"
#include <algorithm>
#include "../common/mptFileIO.h"
#include "../common/misc_util.h"
#include "tuningdialog.h"
#include "FileDialog.h"
#include "Mainfrm.h"


OPENMPT_NAMESPACE_BEGIN


const mpt::Charset TuningCharset = mpt::CharsetLocale;


const CTuningDialog::TUNINGTREEITEM CTuningDialog::s_notFoundItemTuning = TUNINGTREEITEM();
const HTREEITEM CTuningDialog::s_notFoundItemTree = NULL;

typedef Tuning::UNOTEINDEXTYPE UNOTEINDEXTYPE;
typedef Tuning::RATIOTYPE RATIOTYPE;
typedef Tuning::NOTEINDEXTYPE NOTEINDEXTYPE;
#define TT_GENERAL CTuning::TT_GENERAL
#define TT_GROUPGEOMETRIC CTuning::TT_GROUPGEOMETRIC
#define TT_GEOMETRIC CTuning::TT_GEOMETRIC


// CTuningDialog dialog
CTuningDialog::CTuningDialog(CWnd* pParent, INSTRUMENTINDEX inst, CSoundFile &csf)
	: CDialog(CTuningDialog::IDD, pParent),
	m_sndFile(csf),
	m_NoteEditApply(true),
	m_RatioEditApply(true),
	m_pActiveTuningCollection(NULL),
	m_TreeItemTuningItemMap(s_notFoundItemTree, s_notFoundItemTuning),
	m_TreeCtrlTuning(this),
	m_DoErrorExit(false)
//----------------------------------------
{
	m_TuningCollections.push_back(&(m_sndFile.GetTuneSpecificTunings()));
	m_TuningCollectionsNames[&(m_sndFile.GetTuneSpecificTunings())] = _T("Tunings");
	m_pActiveTuning = m_sndFile.Instruments[inst]->pTuning;
	m_RatioMapWnd.m_pTuning = m_pActiveTuning; //pTun is the tuning to show when dialog opens.
}

CTuningDialog::~CTuningDialog()
//----------------------------
{
	for(auto &tuningCol : m_TuningCollections)
	{
		if(IsDeletable(tuningCol))
		{
			delete tuningCol;
			tuningCol = nullptr;
		}
	}
	m_TuningCollections.clear();
	m_DeletableTuningCollections.clear();
}

HTREEITEM CTuningDialog::AddTreeItem(CTuningCollection* pTC, HTREEITEM parent, HTREEITEM insertAfter)
//---------------------------------------------------------------------------------------------------
{
	const HTREEITEM temp = m_TreeCtrlTuning.InsertItem((IsDeletable(pTC) ? CString(_T("loaded: ")) : CString()) +  m_TuningCollectionsNames[pTC], parent, insertAfter);
	HTREEITEM temp2 = NULL;
	m_TreeItemTuningItemMap.AddPair(temp, TUNINGTREEITEM(pTC));
	for(size_t i = 0; i<pTC->GetNumTunings(); i++)
	{
		temp2 = AddTreeItem(&pTC->GetTuning(i), temp, temp2);
	}
	m_TreeCtrlTuning.EnsureVisible(temp);
	return temp;
}

HTREEITEM CTuningDialog::AddTreeItem(CTuning* pT, HTREEITEM parent, HTREEITEM insertAfter)
//-----------------------------------------------------------------------------------------
{
	const HTREEITEM temp = m_TreeCtrlTuning.InsertItem(mpt::ToCString(TuningCharset, pT->GetName()), parent, insertAfter);
	m_TreeItemTuningItemMap.AddPair(temp, TUNINGTREEITEM(pT));
	m_TreeCtrlTuning.EnsureVisible(temp);
	return temp;
}

void CTuningDialog::DeleteTreeItem(CTuning* pT)
//---------------------------------------------
{
	if(!pT)
		return;

	HTREEITEM temp = m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pT));
	if(temp)
	{
		HTREEITEM nextitem = m_TreeCtrlTuning.GetNextItem(temp, TVGN_NEXT);
		if(!nextitem) nextitem = m_TreeCtrlTuning.GetNextItem(temp, TVGN_PREVIOUS);
		m_pActiveTuning = m_TreeItemTuningItemMap.GetMapping_12(nextitem).GetT();
		m_TreeCtrlTuning.DeleteItem(temp);
		//Note: Item from map is deleted 'automatically' in
		//OnTvnDeleteitemTreeTuning.

	}
}

void CTuningDialog::DeleteTreeItem(CTuningCollection* pTC)
//--------------------------------------------------------
{
	if(!pTC)
		return;

	m_pActiveTuning = nullptr;
	const HTREEITEM temp = m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTC));
	if(temp)
	{
		TUNINGTREEITEM prevTTI = m_TreeItemTuningItemMap.GetMapping_12(m_TreeCtrlTuning.GetNextItem(temp, TVGN_PREVIOUS));
		TUNINGTREEITEM nextTTI = m_TreeItemTuningItemMap.GetMapping_12(m_TreeCtrlTuning.GetNextItem(temp, TVGN_NEXT));

		CTuningCollection* pTCprev = prevTTI.GetTC();
		CTuningCollection* pTCnext = nextTTI.GetTC();
		if(pTCnext == nullptr)
			pTCnext = GetpTuningCollection(nextTTI.GetT());
		if(pTCprev == nullptr)
			pTCprev = GetpTuningCollection(prevTTI.GetT());

		if(pTCnext != nullptr && pTCnext != m_pActiveTuningCollection)
			m_pActiveTuningCollection = pTCnext;
		else
		{
			if(pTCprev != m_pActiveTuningCollection)
				m_pActiveTuningCollection = pTCprev;
			else
				m_pActiveTuningCollection = NULL;
		}

		m_TreeCtrlTuning.DeleteItem(temp);
		//Note: Item from map is deleted 'automatically' in
		//OnTvnDeleteitemTreeTuning.
	}
	else
	{
		ASSERT(false);
		m_DoErrorExit = true;
		m_pActiveTuningCollection = NULL;
	}
}

BOOL CTuningDialog::OnInitDialog()
//--------------------------------
{
	CDialog::OnInitDialog();

	m_EditRatioPeriod.SubclassDlgItem(IDC_EDIT_RATIOPERIOD, this);
	m_EditRatio.SubclassDlgItem(IDC_EDIT_RATIOVALUE, this);
	m_EditRatioPeriod.AllowNegative(false);
	m_EditRatioPeriod.AllowFractions(true);
	m_EditRatio.AllowNegative(false);
	m_EditRatio.AllowFractions(true);

	m_RatioMapWnd.Init(this, 0);

	//-->Creating treeview
	m_TreeItemTuningItemMap.ClearMapping();
	for(const auto &tuningCol : m_TuningCollections)
	{
		AddTreeItem(tuningCol, NULL, NULL);
	}
	//<-- Creating treeview

	m_pActiveTuningCollection = GetpTuningCollection(m_pActiveTuning);

	//Adding tuning type names to corresponding combobox.
	m_CombobTuningType.SetItemData(m_CombobTuningType.AddString(_T("General")), TT_GENERAL);
	m_CombobTuningType.SetItemData(m_CombobTuningType.AddString(_T("GroupGeometric")), TT_GROUPGEOMETRIC);
	m_CombobTuningType.SetItemData(m_CombobTuningType.AddString(_T("Geometric")), TT_GEOMETRIC);
	m_CombobTuningType.EnableWindow(FALSE);

	m_ButtonSet.EnableWindow(FALSE);

	m_EditSteps.SetLimitText(2);
	m_EditFineTuneSteps.SetLimitText(3);

	if(m_pActiveTuning) m_RatioMapWnd.m_nNote =  m_RatioMapWnd.m_nNoteCentre + m_pActiveTuning->GetValidityRange().first + (m_pActiveTuning->GetValidityRange().second - m_pActiveTuning->GetValidityRange().first)/2 + 1;

	UpdateView();

	return TRUE;
}


bool CTuningDialog::CanEdit(CTuning * pT, CTuningCollection * pTC) const
{
	if(!pT)
	{
		return false;
	}
	if(!pTC)
	{
		return false;
	}
	if(pTC != m_TuningCollections[0])
	{
		return false;
	}
	return true;
}


bool CTuningDialog::CanEdit(CTuningCollection * pTC) const
{
	if(!pTC)
	{
		return false;
	}
	if(pTC != m_TuningCollections[0])
	{
		return false;
	}
	return true;
}


void CTuningDialog::UpdateView(const int updateMask)
//--------------------------------------------------
{
	if(m_DoErrorExit)
	{
		DoErrorExit();
		return;
	}

	//-->Updating treeview
	if(updateMask != UM_TUNINGDATA)
	{
		TUNINGTREEITEM tuningitem;
		if(m_pActiveTuning)
			tuningitem.Set(m_pActiveTuning);
		else
		{
			if(m_pActiveTuningCollection)
				tuningitem.Set(m_pActiveTuningCollection);
		}
		HTREEITEM treeitem = m_TreeItemTuningItemMap.GetMapping_21(tuningitem);
		if(treeitem)
		{
			m_TreeCtrlTuning.Select(treeitem, TVGN_CARET);
			if(m_pActiveTuning)
				m_TreeCtrlTuning.SetItemText(treeitem, mpt::ToCString(TuningCharset, m_pActiveTuning->GetName()));
			else
				m_TreeCtrlTuning.SetItemText(treeitem, (IsDeletable(m_pActiveTuningCollection) ? CString(_T("loaded: ")) : CString()) + m_TuningCollectionsNames[m_pActiveTuningCollection]);
		}
	}
	//<--Updating treeview


	if(m_pActiveTuningCollection == NULL)
	{
		return;
	}

	m_ButtonNew.EnableWindow(TRUE);
	m_ButtonImport.EnableWindow(TRUE);
	m_ButtonExport.EnableWindow((m_pActiveTuning || m_pActiveTuningCollection) ? TRUE : FALSE);
	m_ButtonRemove.EnableWindow(((m_pActiveTuning && (m_pActiveTuningCollection == m_TuningCollections[0])) || (!m_pActiveTuning && m_pActiveTuningCollection && m_pActiveTuningCollection != m_TuningCollections[0])) ? TRUE : FALSE);

	//Updating tuning part-->
	if(m_pActiveTuning != NULL && (updateMask & UM_TUNINGDATA || updateMask == 0))
	{
		UpdateTuningType();

		m_EditName.SetWindowText(mpt::ToCString(TuningCharset, m_pActiveTuning->GetName()));
		m_EditName.Invalidate();

		//Finetunesteps-edit
		m_EditFineTuneSteps.SetWindowText(mpt::tfmt::val(m_pActiveTuning->GetFineStepCount()));
		m_EditFineTuneSteps.Invalidate();

		//Making sure that ratiomap window is showing and
		//updating its content.
		m_RatioMapWnd.ShowWindow(SW_SHOW);
		m_RatioMapWnd.m_pTuning = m_pActiveTuning;
		m_RatioMapWnd.Invalidate();
		UpdateRatioMapEdits(m_RatioMapWnd.GetShownCentre());


		const UNOTEINDEXTYPE period = m_pActiveTuning->GetGroupSize();
		const RATIOTYPE GroupRatio = m_pActiveTuning->GetGroupRatio();
		if(m_pActiveTuning->GetType() == TT_GROUPGEOMETRIC || m_pActiveTuning->GetType() == TT_GEOMETRIC)
		{
			m_EditSteps.EnableWindow(TRUE);
			m_EditRatioPeriod.EnableWindow(TRUE);
			m_EditSteps.SetWindowText(mpt::tfmt::val(period));
			m_EditRatioPeriod.SetWindowText(mpt::tfmt::val(GroupRatio));
		} else
		{
			m_EditSteps.EnableWindow(FALSE);
			m_EditRatioPeriod.EnableWindow(FALSE);
			m_EditSteps.SetWindowText(_T(""));
			m_EditRatioPeriod.SetWindowText(_T(""));
		}

		m_EditRatioPeriod.Invalidate();
		m_EditSteps.Invalidate();

		bool enableControls = CanEdit(m_pActiveTuning, m_pActiveTuningCollection);

		m_CombobTuningType.EnableWindow(FALSE);
		m_EditSteps.SetReadOnly(!enableControls);
		m_EditRatioPeriod.SetReadOnly(!enableControls);
		m_EditRatio.SetReadOnly((m_pActiveTuning->GetType() == TT_GEOMETRIC) ? TRUE : !enableControls);
		m_EditNotename.SetReadOnly(!enableControls);
		m_EditMiscActions.SetReadOnly((m_pActiveTuning->GetType() == TT_GEOMETRIC) ? TRUE : !enableControls);
		m_EditFineTuneSteps.SetReadOnly(!enableControls);
		m_EditName.SetReadOnly(!enableControls);

		m_ButtonSet.EnableWindow((m_pActiveTuning->GetType() == TT_GEOMETRIC) ? FALSE : enableControls);

		m_CombobTuningType.Invalidate();
		m_EditSteps.Invalidate();
		m_EditRatioPeriod.Invalidate();
	}
	else
	{
		if(m_pActiveTuning == NULL) //No active tuning, clearing tuning part.
		{
			m_EditName.SetWindowText(_T(""));
			m_EditSteps.SetWindowText(_T(""));
			m_EditRatioPeriod.SetWindowText(_T(""));
			m_EditRatio.SetWindowText(_T(""));
			m_EditNotename.SetWindowText(_T(""));
			m_EditMiscActions.SetWindowText(_T(""));
			m_EditFineTuneSteps.SetWindowText(_T(""));
			m_EditName.SetWindowText(_T(""));

			m_CombobTuningType.SetCurSel(-1);

			m_RatioMapWnd.ShowWindow(SW_HIDE);
			m_RatioMapWnd.m_pTuning = NULL;
			m_RatioMapWnd.Invalidate();
		}
	}
	//<--Updating tuning part
}


void CTuningDialog::DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATICRATIOMAP, m_RatioMapWnd);
	DDX_Control(pDX, IDC_COMBO_TTYPE, m_CombobTuningType);
	DDX_Control(pDX, IDC_EDIT_STEPS, m_EditSteps);
	DDX_Control(pDX, IDC_EDIT_NOTENAME, m_EditNotename);
	DDX_Control(pDX, IDC_BUTTON_SETVALUES, m_ButtonSet);
	DDX_Control(pDX, IDC_BUTTON_TUNING_NEW, m_ButtonNew);
	DDX_Control(pDX, IDC_BUTTON_IMPORT, m_ButtonImport);
	DDX_Control(pDX, IDC_BUTTON_EXPORT, m_ButtonExport);
	DDX_Control(pDX, IDC_BUTTON_TUNING_REMOVE, m_ButtonRemove);
	DDX_Control(pDX, IDC_EDIT_MISC_ACTIONS, m_EditMiscActions);
	DDX_Control(pDX, IDC_EDIT_FINETUNESTEPS, m_EditFineTuneSteps);
	DDX_Control(pDX, IDC_EDIT_NAME, m_EditName);
	DDX_Control(pDX, IDC_TREE_TUNING, m_TreeCtrlTuning);
}



BEGIN_MESSAGE_MAP(CTuningDialog, CDialog)
	ON_EN_CHANGE(IDC_EDIT_STEPS, OnEnChangeEditSteps)
	ON_EN_CHANGE(IDC_EDIT_RATIOPERIOD, OnEnChangeEditRatioperiod)
	ON_EN_CHANGE(IDC_EDIT_NOTENAME, OnEnChangeEditNotename)
	ON_BN_CLICKED(IDC_BUTTON_SETVALUES, OnBnClickedButtonSetvalues)
	ON_EN_CHANGE(IDC_EDIT_RATIOVALUE, OnEnChangeEditRatiovalue)
	ON_BN_CLICKED(IDC_BUTTON_TUNING_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_TUNING_REMOVE, OnBnClickedButtonRemove)
	ON_EN_CHANGE(IDC_EDIT_FINETUNESTEPS, OnEnChangeEditFinetunesteps)
	ON_EN_KILLFOCUS(IDC_EDIT_FINETUNESTEPS, OnEnKillfocusEditFinetunesteps)
	ON_EN_KILLFOCUS(IDC_EDIT_NAME, OnEnKillfocusEditName)
	ON_EN_KILLFOCUS(IDC_EDIT_STEPS, OnEnKillfocusEditSteps)
	ON_EN_KILLFOCUS(IDC_EDIT_RATIOPERIOD, OnEnKillfocusEditRatioperiod)
	ON_EN_KILLFOCUS(IDC_EDIT_RATIOVALUE, OnEnKillfocusEditRatiovalue)
	ON_EN_KILLFOCUS(IDC_EDIT_NOTENAME, OnEnKillfocusEditNotename)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_TUNING, OnTvnSelchangedTreeTuning)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE_TUNING, OnTvnDeleteitemTreeTuning)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_TUNING, OnNMRclickTreeTuning)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE_TUNING, OnTvnBegindragTreeTuning)
	ON_COMMAND(ID_REMOVETUNING, OnRemoveTuning)
	ON_COMMAND(ID_ADDTUNINGGENERAL, OnAddTuningGeneral)
	ON_COMMAND(ID_ADDTUNINGGROUPGEOMETRIC, OnAddTuningGroupGeometric)
	ON_COMMAND(ID_ADDTUNINGGEOMETRIC, OnAddTuningGeometric)
	ON_COMMAND(ID_COPYTUNING, OnCopyTuning)
	ON_COMMAND(ID_REMOVETUNINGCOLLECTION, OnRemoveTuningCollection)
END_MESSAGE_MAP()


void CTuningDialog::DoErrorExit()
//-------------------------------
{
	m_DoErrorExit = false;
	m_pActiveTuning = NULL;
	m_pActiveTuningCollection = NULL;
	Reporting::Message(LogInformation, _T("Dialog encountered an error and needs to close"), this);
	OnOK();
}


// CTuningDialog message handlers

void CTuningDialog::UpdateTuningType()
//------------------------------------
{
	if(m_pActiveTuning)
	{
		if(m_CombobTuningType.GetCount() < 3) m_DoErrorExit = true;

		if(m_pActiveTuning->GetType() == TT_GEOMETRIC)
			m_CombobTuningType.SetCurSel(2);
		else
			if(m_pActiveTuning->GetType() == TT_GROUPGEOMETRIC)
				m_CombobTuningType.SetCurSel(1);
			else
				m_CombobTuningType.SetCurSel(0);
	}
}



bool CTuningDialog::AddTuning(CTuningCollection* pTC, Tuning::TUNINGTYPE type)
//----------------------------------------------------------------------------
{
	if(!pTC)
	{
		Reporting::Notification("No tuning collection chosen");
		return true;
	}

	CTuning* pNewTuning = nullptr;
	if(type == TT_GROUPGEOMETRIC)
	{
		std::vector<Tuning::RATIOTYPE> ratios;
		for(Tuning::NOTEINDEXTYPE n = 0; n < 12; ++n)
		{
			ratios.push_back(std::pow(static_cast<Tuning::RATIOTYPE>(2.0), static_cast<Tuning::RATIOTYPE>(n) / static_cast<Tuning::RATIOTYPE>(12)));
		}
		pNewTuning = CTuning::CreateGroupGeometric("Unnamed", ratios, 2, 15);
	} else if(type == TT_GEOMETRIC)
	{
		pNewTuning = CTuning::CreateGeometric("Unnamed", 12, 2, 15);
	} else
	{
		pNewTuning = CTuning::CreateGeneral("Unnamed");
	}

	if(pTC->AddTuning(pNewTuning))
	{
		Reporting::Notification("Add tuning failed");
		delete pNewTuning;
		return true;
	}
	AddTreeItem(pNewTuning, m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTC)), NULL);
	m_pActiveTuning = pNewTuning;
	m_ModifiedTCs[pTC] = true;
	UpdateView();

	return false;
}


void CTuningDialog::OnEnChangeEditSteps()
//---------------------------------------
{
}

void CTuningDialog::OnEnChangeEditRatioperiod()
//---------------------------------------------
{
}


void CTuningDialog::OnEnChangeEditNotename()
//------------------------------------------
{

	if(!m_NoteEditApply)
	{
		m_NoteEditApply = true;
		return;
	}

	if(!m_pActiveTuning)
		return;

	const NOTEINDEXTYPE currentNote = m_RatioMapWnd.GetShownCentre();
	CString buffer;
	m_EditNotename.GetWindowText(buffer);
	std::string str = mpt::ToCharset(TuningCharset, buffer);
	{
		if(str.size() > 3)
			str.resize(3);
		m_pActiveTuning->SetNoteName(currentNote, str);
	}

	m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
	m_RatioMapWnd.Invalidate();

}

void CTuningDialog::OnEnChangeEditRatiovalue()
//--------------------------------------------
{

	if(!m_RatioEditApply)
	{
		m_RatioEditApply = true;
		return;
	}

	if(!m_pActiveTuning)
		return;

	const NOTEINDEXTYPE currentNote = m_RatioMapWnd.GetShownCentre();

	double ratio = 0.0;
	if(m_EditRatio.GetDecimalValue(ratio))
	{
		m_pActiveTuning->SetRatio(currentNote, static_cast<RATIOTYPE>(ratio));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		UpdateTuningType();
		m_RatioMapWnd.Invalidate();
	}

}

void CTuningDialog::OnBnClickedButtonSetvalues()
//----------------------------------------------
{
	if(m_pActiveTuning)
	{
		if(m_EditMiscActions.GetWindowTextLength() < 1)
			return;

		CString buffer;
		m_EditMiscActions.GetWindowText(buffer);
		m_pActiveTuning->Multiply(ConvertStrTo<RATIOTYPE>(buffer));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		m_EditMiscActions.SetWindowText(_T(""));
		m_RatioMapWnd.Invalidate();
	}
}

void CTuningDialog::UpdateRatioMapEdits(const NOTEINDEXTYPE& note)
//-----------------------------------------------------------
{
	if(m_pActiveTuning == NULL)
		return;

	m_RatioEditApply = false;
	m_EditRatio.SetWindowText(mpt::tfmt::val(m_pActiveTuning->GetRatio(note)));
	m_NoteEditApply = false;
	m_EditNotename.SetWindowText(mpt::ToCString(TuningCharset, m_pActiveTuning->GetNoteName(note, false)));

	m_EditRatio.Invalidate();
	m_EditNotename.Invalidate();
}


void CTuningDialog::OnBnClickedButtonNew()
//----------------------------------------
{
	POINT point;
	GetCursorPos(&point);

	HMENU popUpMenu = CreatePopupMenu();
	if(popUpMenu == NULL) return;

	AppendMenu(popUpMenu, MF_STRING, ID_ADDTUNINGGROUPGEOMETRIC, _T("Add &GroupGeometric tuning"));
	AppendMenu(popUpMenu, MF_STRING, ID_ADDTUNINGGEOMETRIC, _T("Add G&eometric tuning"));
	AppendMenu(popUpMenu, MF_STRING, ID_ADDTUNINGGENERAL, _T("Add Ge&neral tuning"));

	m_CommandItemDest.Set(m_TuningCollections[0]);

	TrackPopupMenu(popUpMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
	DestroyMenu(popUpMenu);
}


void CTuningDialog::OnBnClickedButtonExport()
//-------------------------------------------
{

	if(m_pActiveTuning == NULL && m_pActiveTuningCollection == NULL)
	{
		Reporting::Message(LogInformation, _T("Operation failed - No tuning file selected."), this);
		return;
	}

	bool failure = true;

	if(m_pActiveTuning)
	{

		const CTuning* pT = m_pActiveTuning;

		std::string filter;
		int filters = 0;
		int tuningFilter = -1;
		int sclFilter = -1;
		{
			filters++;
			filter += std::string("Tuning files (*") + CTuning::s_FileExtension + std::string(")|*") + CTuning::s_FileExtension + std::string("|");
			tuningFilter = filters;
		}
		{
			filters++;
			filter += std::string("Scala scale (*.scl)|*") + std::string(".scl")+ std::string("|");
			sclFilter = filters;
		}

		int filterIndex = 0;
		FileDialog dlg = SaveFileDialog()
			.DefaultExtension(CTuning::s_FileExtension)
			.ExtensionFilter(filter)
			.WorkingDirectory(TrackerSettings::Instance().PathTunings.GetWorkingDir())
			.FilterIndex(&filterIndex);

		if (!dlg.Show(this)) return;

		BeginWaitCursor();

		mpt::ofstream fout(dlg.GetFirstFile(), std::ios::binary);

		if(tuningFilter != -1 && filterIndex == tuningFilter)
		{
			failure = (pT->Serialize(fout) != Tuning::SerializationResult::Success);
		} else if(sclFilter != -1 && filterIndex == sclFilter)
		{
			failure = !pT->WriteSCL(fout, dlg.GetFirstFile());
		}

		fout.close();

		EndWaitCursor();

	} else
	{

		const CTuningCollection* pTC = m_pActiveTuningCollection;

		std::string filter = std::string("multiple Tuning files (") + CTuning::s_FileExtension + std::string(")|*") + CTuning::s_FileExtension + std::string("|");

		mpt::PathString fileName;
		if(!m_TuningCollectionsFilenames[pTC].empty())
		{
			fileName = m_TuningCollectionsFilenames[pTC] + MPT_PATHSTRING(" - ");
		}
		if(!m_TuningCollectionsNames[pTC].IsEmpty())
		{
			mpt::PathString name = mpt::PathString::FromUnicode(mpt::ToUnicode(m_TuningCollectionsNames[pTC]));
			SanitizeFilename(name);
			fileName += name + MPT_PATHSTRING(" - ");
		}
		fileName += MPT_PATHSTRING("%tuning_number% - %tuning_name%");

		int filterIndex = 0;
		FileDialog dlg = SaveFileDialog()
			.DefaultExtension(CTuning::s_FileExtension)
			.ExtensionFilter(filter)
			.WorkingDirectory(TrackerSettings::Instance().PathTunings.GetWorkingDir())
			.FilterIndex(&filterIndex);
		dlg.DefaultFilename(fileName);

		if (!dlg.Show(this)) return;

		BeginWaitCursor();

		failure = false;

		auto numberFmt = mpt::FormatSpec().Dec().FillNul().Width(1 + static_cast<int>(std::log10(pTC->GetNumTunings())));

		for(std::size_t i = 0; i < pTC->GetNumTunings(); ++i)
		{
			const CTuning & tuning = pTC->GetTuning(i);
			fileName = dlg.GetFirstFile();
			mpt::ustring tuningName = mpt::ToUnicode(TuningCharset, tuning.GetName());
			if(tuningName.empty())
			{
				tuningName = MPT_USTRING("untitled");
			}
			std::wstring fileNameW = fileName.ToWide();
			std::wstring numberW = numberFmt.ToWString(i + 1);
			SanitizeFilename(numberW);
			fileNameW = mpt::String::Replace(fileNameW, L"%tuning_number%", numberW);
			std::wstring nameW = mpt::ToWide(tuningName);
			SanitizeFilename(nameW);
			fileNameW = mpt::String::Replace(fileNameW, L"%tuning_name%", nameW);
			fileName = mpt::PathString::FromWide(fileNameW);
			mpt::ofstream fout(fileName, std::ios::binary);
			if(tuning.Serialize(fout) != Tuning::SerializationResult::Success)
			{
				failure = true;
			}
			fout.close();
		}

		EndWaitCursor();

	}

	if (failure)
		Reporting::Message(LogError, _T("Export failed"), _T("Error!"), this);

}


void CTuningDialog::OnBnClickedButtonRemove()
//-------------------------------------------
{
	if(m_pActiveTuning)
	{
		if(CanEdit(m_pActiveTuning, m_pActiveTuningCollection))
		{
			m_CommandItemDest.Set(m_pActiveTuning);
			OnRemoveTuning();
		}
	} else if(m_pActiveTuningCollection)
	{
		if(IsDeletable(m_pActiveTuningCollection))
		{
			m_CommandItemDest.Set(m_pActiveTuningCollection);
			OnRemoveTuningCollection();
		}
	}
}


template <typename Tfile, std::size_t N> static bool CheckMagic(Tfile &f, mpt::IO::Offset offset, const uint8(&magic)[N])
//-----------------------------------------------------------------------------------------------------------------------
{
	if(!mpt::IO::SeekAbsolute(f, offset))
	{
		return false;
	}
	uint8 buffer[N];
	MemsetZero(buffer);
	if(mpt::IO::ReadRaw(f, buffer, N) != N)
	{
		return false;
	}
	bool result = (std::memcmp(magic, buffer, N) == 0);
	mpt::IO::SeekBegin(f);
	return result;
}


void CTuningDialog::OnBnClickedButtonImport()
//-------------------------------------------
{
	std::string sFilter = mpt::format("Tuning files (*%1, *%2, *.scl)|*%3;*%4;*.scl|")(
		CTuning::s_FileExtension,
		CTuningCollection::s_FileExtension,
		CTuning::s_FileExtension,
		CTuningCollection::s_FileExtension);

	FileDialog dlg = OpenFileDialog()
		.AllowMultiSelect()
		.ExtensionFilter(sFilter)
		.WorkingDirectory(TrackerSettings::Instance().PathTunings.GetWorkingDir());
	if(!dlg.Show(this))
		return;

	TrackerSettings::Instance().PathTunings.SetWorkingDir(dlg.GetWorkingDirectory());

	std::wstring sLoadReport;

	const auto &files = dlg.GetFilenames();
	for(const auto &file : files)
	{
		mpt::PathString fileName;
		mpt::PathString fileExt;
		file.SplitPath(nullptr, nullptr, &fileName, &fileExt);
		const std::wstring fileNameExt = (fileName + fileExt).ToWide();

		const bool bIsTun = (mpt::PathString::CompareNoCase(fileExt, mpt::PathString::FromUTF8(CTuning::s_FileExtension)) == 0);
		const bool bIsScl = (mpt::PathString::CompareNoCase(fileExt, MPT_PATHSTRING(".scl")) == 0);
		const bool bIsTc = (mpt::PathString::CompareNoCase(fileExt, mpt::PathString::FromUTF8(CTuningCollection::s_FileExtension)) == 0);

		mpt::ifstream fin(file, std::ios::binary);

		// "HSCT", 0x01, 0x00, 0x00, 0x00
		const uint8 magicTColdV1 [] = {  'H', 'S', 'C', 'T',0x01,0x00,0x00,0x00                          };
		// "HSCT", 0x02, 0x00, 0x00, 0x00
		const uint8 magicTColdV2 [] = {  'H', 'S', 'C', 'T',0x02,0x00,0x00,0x00                          };
		// "CTRTI_B.", 0x03, 0x00
		const uint8 magicTUNoldV2[] = {  'C', 'T', 'R', 'T', 'I', '_', 'B', '.',0x02,0x00                };
		// "CTRTI_B.", 0x03, 0x00
		const uint8 magicTUNoldV3[] = {  'C', 'T', 'R', 'T', 'I', '_', 'B', '.',0x03,0x00                };
		// "228", 0x02, "TC"
		const uint8 magicTC      [] = {  '2', '2', '8',0x02, 'T', 'C'                                    };
		// "228", 0x09, "CTB244RTI"
		const uint8 magicTUN     [] = {  '2', '2', '8',0x09, 'C', 'T', 'B', '2', '4', '4', 'R', 'T', 'I' };

		CTuningCollection *pTC = nullptr;
		CString tcName;
		mpt::PathString tcFilename;
		CTuning *pT = nullptr;

		if(bIsTun && CheckMagic(fin, 0, magicTC))
		{
			// OpenMPT since r3115 wrongly wrote .tc files instead of .tun files when exporting.
			// If such a file is detected and only contains a single Tuning, we can work-around that.
			// For .tc files containing multiple Tunings, we sadly cannot decide which one the user wanted.
			// In that case, we import as a Collection (an alternative might be to display a dialog in this case).
			pTC = new CTuningCollection();
			std::string name;
			if(pTC->Deserialize(fin, name) == Tuning::SerializationResult::Success)
			{ // success
				if(pTC->GetNumTunings() == 1)
				{
					Reporting::Message(LogInformation, MPT_USTRING("- Tuning Collection with a Tuning file extension (.tun) detected. It only contains a single Tuning, importing the file as a Tuning.\n"), this);
					pT = new CTuning(pTC->GetTuning(0));
					delete pTC;
					pTC = nullptr;
					// ok
				} else
				{
					Reporting::Message(LogNotification, MPT_USTRING("- Tuning Collection with a Tuning file extension (.tun) detected. It only contains multiple Tunings, importing the file as a Tuning Collection.\n"), this);
					// ok
				}
			} else
			{
				delete pTC;
				pTC = nullptr;
				// fail
			}

		} else if(CheckMagic(fin, 0, magicTC) || CheckMagic(fin, 0, magicTColdV2) || CheckMagic(fin, 0, magicTColdV1))
		{

			pTC = new CTuningCollection();
			std::string name;
			if(pTC->Deserialize(fin, name) != Tuning::SerializationResult::Success)
			{ // failure
				delete pTC;
				pTC = nullptr;
				// fail
			} else
			{
				tcName = mpt::ToCString(TuningCharset, name);
				tcFilename = file;
				// ok
			}

		} else if(CheckMagic(fin, 0, magicTUNoldV3) || CheckMagic(fin, 0, magicTUNoldV2))
		{

			pT = CTuning::CreateDeserializeOLD(fin);

		} else if(CheckMagic(fin, 0, magicTUN))
		{

			pT = CTuning::CreateDeserialize(fin);

		} else if(bIsScl)
		{

			EnSclImport a = ImportScl(file, fileName.ToUnicode(), pT);
			if(a != enSclImportOk)
			{ // failure
				if(pT)
				{
					delete pT;
					pT = nullptr;
				}
			}

		}

		if(pT)
		{
			CTuningCollection &tc = *m_TuningCollections.front();
			if(tc.AddTuning(pT))
			{
				delete pT;
				pT = nullptr;
				if(tc.GetNumTunings() >= CTuningCollection::s_nMaxTuningCount)
				{
					sLoadReport += mpt::format(L"- Failed to load file \"%1\": maximum number(%2) of temporary tunings is already open.\n")(fileNameExt, CTuningCollection::s_nMaxTuningCount);
				} else 
				{
					sLoadReport += mpt::format(L"- Unable to import file \"%1\": unknown reason.\n")(fileNameExt);
				}
			} else
			{
				m_pActiveTuning = pT;
				AddTreeItem(m_pActiveTuning, m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(&tc)), NULL);
			}
		}

		if(pTC)
		{
			m_TuningCollections.push_back(pTC);
			m_TuningCollectionsNames[pTC] = tcName;
			m_TuningCollectionsFilenames[pTC] = tcFilename;
			m_DeletableTuningCollections.push_back(pTC);
			AddTreeItem(pTC, NULL, NULL);
		}

		if(!pT && !pTC)
		{
			sLoadReport += mpt::format(L"- Unable to load \"%1\": unrecognized file format.\n")(fileNameExt);
		}
	}

	if(sLoadReport.length() > 0)
		Reporting::Information(sLoadReport);
	UpdateView();
}


void CTuningDialog::OnEnChangeEditFinetunesteps()
//-----------------------------------------------
{
}


void CTuningDialog::OnEnKillfocusEditFinetunesteps()
//--------------------------------------------------
{
	if(m_pActiveTuning)
	{
		CString buffer;
		m_EditFineTuneSteps.GetWindowText(buffer);
		m_pActiveTuning->SetFineStepCount(ConvertStrTo<Tuning::USTEPINDEXTYPE>(buffer));
		m_EditFineTuneSteps.SetWindowText(mpt::tfmt::val(m_pActiveTuning->GetFineStepCount()));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		m_EditFineTuneSteps.Invalidate();
	}
}


void CTuningDialog::OnEnKillfocusEditName()
//-----------------------------------------
{
	if(m_pActiveTuning != NULL)
	{
		CString buffer;
		m_EditName.GetWindowText(buffer);
		m_pActiveTuning->SetName(mpt::ToCharset(TuningCharset, buffer));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		UpdateView(UM_TUNINGDATA);
		UpdateView(UM_TUNINGCOLLECTION);
	}
}


void CTuningDialog::OnEnKillfocusEditSteps()
//------------------------------------------
{
	if(m_pActiveTuning)
	{
		CString buffer;
		m_EditSteps.GetWindowText(buffer);
		m_pActiveTuning->ChangeGroupsize(ConvertStrTo<UNOTEINDEXTYPE>(buffer));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		UpdateView(UM_TUNINGDATA);
	}
}


void CTuningDialog::OnEnKillfocusEditRatioperiod()
//------------------------------------------------
{
	double ratio = 0.0;
	if(m_pActiveTuning && m_EditRatioPeriod.GetDecimalValue(ratio))
	{
		m_pActiveTuning->ChangeGroupRatio(static_cast<RATIOTYPE>(ratio));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		UpdateView(UM_TUNINGDATA);
	}
}

void CTuningDialog::OnEnKillfocusEditRatiovalue()
//-----------------------------------------------
{
	UpdateView(UM_TUNINGDATA);
}


void CTuningDialog::OnEnKillfocusEditNotename()
//-----------------------------------------------
{
	UpdateView(UM_TUNINGDATA);
}

bool CTuningDialog::GetModifiedStatus(const CTuningCollection* const pTc) const
//-----------------------------------------------------------------------------
{
	auto iter = m_ModifiedTCs.find(pTc);
	if(iter != m_ModifiedTCs.end())
		return (*iter).second;
	else
		return false;

}

CTuningCollection* CTuningDialog::GetpTuningCollection(HTREEITEM ti) const
//------------------------------------------------------------------------
{
	//If treeitem is that of a tuningcollection, return address of
	//that tuning collection. If treeitem is that of a tuning, return
	//the owning tuningcollection
	TUNINGTREEITEM tunItem = m_TreeItemTuningItemMap.GetMapping_12(ti);
	CTuningCollection* pTC = tunItem.GetTC();
	if(pTC)
		return pTC;
	else
	{
		CTuning* pT = tunItem.GetT();
		return GetpTuningCollection(pT);
	}
}

CTuningCollection* CTuningDialog::GetpTuningCollection(const CTuning* const pT) const
//-----------------------------------------------------------------------------------
{
	for(auto &tuningCol : m_TuningCollections)
	{
		for(size_t j = 0; j < tuningCol->GetNumTunings(); j++)
		{
			if(pT == &tuningCol->GetTuning(j))
			{
				return tuningCol;
			}
		}
	}
	return NULL;
}


void CTuningDialog::OnTvnSelchangedTreeTuning(NMHDR *pNMHDR, LRESULT *pResult)
//----------------------------------------------------------------------------
{
	//This methods gets called when selected item in the treeview
	//changes.

	//TODO: This gets called before killfocus messages of edits, which
	//		can be a problem.

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	TUNINGTREEITEM ti = m_TreeItemTuningItemMap.GetMapping_12(pNMTreeView->itemNew.hItem);

	if(ti)
	{
		int updateMask = UM_TUNINGDATA;
		CTuningCollection* pPrevTuningCollection = m_pActiveTuningCollection;
		CTuning* pT = ti.GetT();
		CTuningCollection* pTC = ti.GetTC();
		if(pTC)
		{
			m_pActiveTuningCollection = pTC;
			ASSERT(pT == NULL);
			m_pActiveTuning = NULL;
		}
		else
		{
			m_pActiveTuning = pT;
			m_pActiveTuningCollection = GetpTuningCollection(m_pActiveTuning);

		}
		if(m_pActiveTuningCollection != pPrevTuningCollection) updateMask |= UM_TUNINGCOLLECTION;
		UpdateView(updateMask);
	}
	else
	{
		m_DoErrorExit = true;
	}

	*pResult = 0;
}

void CTuningDialog::OnTvnDeleteitemTreeTuning(NMHDR *pNMHDR, LRESULT *pResult)
//----------------------------------------------------------------------------
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;
	if(pNMTreeView->itemOld.mask & TVIF_HANDLE && pNMTreeView->itemOld.hItem)
	{
		m_TreeItemTuningItemMap.RemoveValue_1(pNMTreeView->itemOld.hItem);
	}
	else
		m_DoErrorExit = true;
}

void CTuningDialog::OnNMRclickTreeTuning(NMHDR *, LRESULT *pResult)
//-----------------------------------------------------------------
{
	*pResult = 0;

	HTREEITEM hItem;
	POINT point, ptClient;

	GetCursorPos(&point);
	ptClient = point;
	m_TreeCtrlTuning.ScreenToClient(&ptClient);
	hItem = m_TreeCtrlTuning.HitTest(ptClient, NULL);
	if(hItem == NULL)
		return;

	m_TreeCtrlTuning.Select(hItem, TVGN_CARET);

	TUNINGTREEITEM tunitem = m_TreeItemTuningItemMap.GetMapping_12(hItem);

	if(!tunitem)
	{
		m_DoErrorExit = true;
		return;
	}

	HMENU popUpMenu = CreatePopupMenu();
	if(popUpMenu == NULL) return;

	CTuning* pT = tunitem.GetT();
	CTuningCollection* pTC = tunitem.GetTC();

	if(pT) //Creating context menu for tuning-item
	{
		pTC = GetpTuningCollection(pT);
		if(pTC != NULL)
		{
			UINT mask = MF_STRING;
			if(!CanEdit(pT, pTC))
			{
				mask |= MF_GRAYED;
			}
			AppendMenu(popUpMenu, mask, ID_REMOVETUNING, _T("&Remove"));

			m_CommandItemDest.Set(pT);
		}
	}
	else //Creating context menu for tuning collection item.
	{
		if(pTC != NULL)
		{
			UINT mask = MF_STRING;

			mask = MF_STRING;
			if (!CanEdit(pTC))
				mask |= MF_GRAYED;
			AppendMenu(popUpMenu, mask, ID_ADDTUNINGGROUPGEOMETRIC, _T("Add &GroupGeometric tuning"));
			AppendMenu(popUpMenu, mask, ID_ADDTUNINGGEOMETRIC, _T("Add G&eometric tuning"));
			AppendMenu(popUpMenu, mask, ID_ADDTUNINGGENERAL, _T("Add Ge&neral tuning"));

			mask = MF_STRING;
			if(!IsDeletable(pTC))
				mask |= MF_GRAYED;
			AppendMenu(popUpMenu, mask, ID_REMOVETUNINGCOLLECTION, _T("&Unload tuning collection"));

			m_CommandItemDest.Set(pTC);
		}
	}

	GetCursorPos(&point);
	TrackPopupMenu(popUpMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
	DestroyMenu(popUpMenu);
}

bool CTuningDialog::IsDeletable(const CTuningCollection* const pTC) const
//--------------------------------------------------------------------------------
{
	auto iter = find(m_DeletableTuningCollections.begin(), m_DeletableTuningCollections.end(), pTC);
	if(iter != m_DeletableTuningCollections.end())
		return true;
	else
		return false;
}


void CTuningDialog::OnTvnBegindragTreeTuning(NMHDR *pNMHDR, LRESULT *pResult)
//---------------------------------------------------------------------------
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	m_CommandItemDest.Reset();
	m_CommandItemSrc.Reset();
	if(pNMTreeView == NULL || pNMTreeView->itemNew.hItem == NULL) return;
	TUNINGTREEITEM tunitem = m_TreeItemTuningItemMap.GetMapping_12(pNMTreeView->itemNew.hItem);

	if(tunitem.GetT() == NULL)
	{
		Reporting::Message(LogNotification, _T("For the time being Drag and Drop is only supported for tuning instances."), this);
		return;
	}

	SetCursor(CMainFrame::curDragging);

	m_TreeCtrlTuning.SetDragging();
	m_DragItem = m_TreeItemTuningItemMap.GetMapping_12(pNMTreeView->itemNew.hItem);

	m_TreeCtrlTuning.Select(pNMTreeView->itemNew.hItem, TVGN_CARET);
}


CTuningCollection *CTuningDialog::CanDrop(HTREEITEM dragDestItem)
//---------------------------------------------------------------
{
	if(m_DragItem == NULL)
		return nullptr;

	TUNINGTREEITEM destTunItem = m_TreeItemTuningItemMap.GetMapping_12(dragDestItem);
	if(!destTunItem)
		return nullptr;

	CTuningCollection* pTCdest = nullptr;
	CTuningCollection* pTCsrc = m_DragItem.GetTC();

	if(pTCsrc == nullptr)
		pTCsrc = GetpTuningCollection(m_DragItem.GetT());

	if(pTCsrc == NULL)
	{
		ASSERT(false);
		return nullptr;
	}

	if(destTunItem.GetT()) //Item dragged on tuning
		pTCdest = GetpTuningCollection(destTunItem.GetT());
	else //Item dragged on tuningcollecition
		pTCdest = destTunItem.GetTC();

	//For now, ignoring drags within a tuning collection.
	if(pTCdest == pTCsrc)
		return nullptr;

	return pTCdest;
}


void CTuningDialog::OnEndDrag(HTREEITEM dragDestItem)
//--------------------------------------------------
{
	SetCursor(CMainFrame::curArrow);
	m_TreeCtrlTuning.SetDragging(false);
	if(m_DragItem == NULL)
		return;

	CTuningCollection* pTCdest = CanDrop(dragDestItem);
	m_CommandItemSrc = m_DragItem;
	m_DragItem.Reset();

	if(!pTCdest)
		return;

	CTuningCollection* pTCsrc = m_CommandItemSrc.GetTC();
	if(pTCsrc == nullptr)
		pTCsrc = GetpTuningCollection(m_CommandItemSrc.GetT());

	if(pTCdest)
	{
		UINT mask = MF_STRING;
		HMENU popUpMenu = CreatePopupMenu();
		if(popUpMenu == NULL) return;

		POINT point;
		GetCursorPos(&point);

		if(!CanEdit(pTCdest))
		{
			mask |= MF_GRAYED;
		}
		AppendMenu(popUpMenu, mask, ID_COPYTUNING, _T("&Copy here"));

		GetCursorPos(&point);
		TrackPopupMenu(popUpMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL);
		DestroyMenu(popUpMenu);

		m_CommandItemDest.Set(pTCdest);
	}
}

bool CTuningDialog::AddTuning(CTuningCollection* pTC, CTuning* pT)
//----------------------------------------------------------------
{
	//Default: pT == NULL

	if(!pTC)
	{
		Reporting::Notification("No tuning collection chosen");
		return true;
	}

	CTuning* pNewTuning = nullptr;
	if(pT)
	{
		pNewTuning = new CTuning(*pT);
	} else
	{
		Reporting::Notification("Add tuning failed");
		return true;
	}
	if(pTC->AddTuning(pNewTuning))
	{
		Reporting::Notification("Add tuning failed");
		delete pNewTuning;
		return true;
	}
	AddTreeItem(pNewTuning, m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTC)), NULL);
	m_pActiveTuning = pNewTuning;
	m_ModifiedTCs[pTC] = true;
	UpdateView();

	return false;
}

void CTuningDialog::OnAddTuningGeneral()
//--------------------------------------
{
	if(!m_CommandItemDest.GetTC())
	{
		m_CommandItemDest = s_notFoundItemTuning;
		return;
	}

	CTuningCollection* pTC = m_CommandItemDest.GetTC();
	m_CommandItemDest = s_notFoundItemTuning;
	m_ModifiedTCs[pTC];
	AddTuning(pTC, TT_GENERAL);
}

void CTuningDialog::OnAddTuningGroupGeometric()
//---------------------------------------------
{
	if(!m_CommandItemDest.GetTC())
	{
		m_CommandItemDest = s_notFoundItemTuning;
		return;
	}

	CTuningCollection* pTC = m_CommandItemDest.GetTC();
	m_CommandItemDest = s_notFoundItemTuning;
	m_ModifiedTCs[pTC];
	AddTuning(pTC, TT_GROUPGEOMETRIC);
}

void CTuningDialog::OnAddTuningGeometric()
//----------------------------------------
{
	if(!m_CommandItemDest.GetTC())
	{
		m_CommandItemDest = s_notFoundItemTuning;
		return;
	}

	CTuningCollection* pTC = m_CommandItemDest.GetTC();
	m_CommandItemDest = s_notFoundItemTuning;
	m_ModifiedTCs[pTC];
	AddTuning(pTC, TT_GEOMETRIC);
}

void CTuningDialog::OnRemoveTuning()
//----------------------------------
{
	CTuning* pT = m_CommandItemDest.GetT();
	if(m_CommandItemDest.GetT())
	{
		CTuningCollection* pTC = GetpTuningCollection(pT);
		if(pTC)
		{
			bool used = false;
			for(INSTRUMENTINDEX i = 1; i <= m_sndFile.GetNumInstruments(); i++)
			{
				if(m_sndFile.Instruments[i]->pTuning == pT)
				{
					used = true;
				}
			}
			if(used)
			{
				CString s = _T("Tuning '") + mpt::ToCString(TuningCharset, pT->GetName()) + _T("' is used by instruments. Remove anyway?");
				if(Reporting::Confirm(s, false, true) == cnfYes)
				{
					CriticalSection cs;
					for(INSTRUMENTINDEX i = 1; i <= m_sndFile.GetNumInstruments(); i++)
					{
						if(m_sndFile.Instruments[i]->pTuning == pT)
						{
							m_sndFile.Instruments[i]->SetTuning(nullptr);
						}
					}
					pTC->Remove(pT);
					cs.Leave();
					m_ModifiedTCs[pTC] = true;
					DeleteTreeItem(pT);
					UpdateView();
				}
			} else
			{
				CString s = _T("Remove tuning '") + mpt::ToCString(TuningCharset, pT->GetName()) + _T("'?");
				if(Reporting::Confirm(s) == cnfYes)
				{
					pTC->Remove(pT);
					m_ModifiedTCs[pTC] = true;
					DeleteTreeItem(pT);
					UpdateView();
				}
			}
		}
	}

	m_CommandItemDest = s_notFoundItemTuning;
}


void CTuningDialog::OnCopyTuning()
//--------------------------------
{
	CTuningCollection* pTC = m_CommandItemDest.GetTC();

	if(!pTC)
		return;

	m_CommandItemDest = s_notFoundItemTuning;

	CTuning* pT = m_CommandItemSrc.GetT();
	if(pT == nullptr)
	{
		return;
	}
	m_ModifiedTCs[pTC] = true;
	AddTuning(pTC, pT);
}

void CTuningDialog::OnRemoveTuningCollection()
//--------------------------------------------
{
	if(!m_pActiveTuningCollection)
		return;

	if(!IsDeletable(m_pActiveTuningCollection))
	{
		ASSERT(false);
		return;
	}

	auto iter = find(m_TuningCollections.begin(), m_TuningCollections.end(), m_pActiveTuningCollection);
	if(iter == m_TuningCollections.end())
	{
		ASSERT(false);
		return;
	}
	auto DTCiter = find(m_DeletableTuningCollections.begin(), m_DeletableTuningCollections.end(), *iter);
	CTuningCollection* deletableTC = m_pActiveTuningCollection;
	//Note: Order matters in the following lines.
	m_DeletableTuningCollections.erase(DTCiter);
	m_TuningCollections.erase(iter);
	DeleteTreeItem(m_pActiveTuningCollection);
	m_TuningCollectionsNames.erase(deletableTC);
	m_TuningCollectionsFilenames.erase(deletableTC);
	delete deletableTC; deletableTC = 0;

	UpdateView();
}


void CTuningDialog::OnOK()
//------------------------
{
	// Prevent return-key from closing the window.
	if(GetKeyState(VK_RETURN) <= -127 && GetFocus() != GetDlgItem(IDOK))
		return;
	else
		CDialog::OnOK();
}


////////////////////////////////////////////////////////
//***************
//CTuningTreeCtrl
//***************
////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTuningTreeCtrl, CTreeCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


void CTuningTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
//----------------------------------------------------------
{
	if(IsDragging())
	{
		HTREEITEM hItem = HitTest(point, nullptr);
		SetCursor((hItem == NULL || m_rParentDialog.CanDrop(hItem) == nullptr) ? CMainFrame::curNoDrop2 : CMainFrame::curDragging);
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}


void CTuningTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
//-----------------------------------------------------------
{
	if(IsDragging())
	{
		HTREEITEM hItem = HitTest(point, nullptr);
		m_rParentDialog.OnEndDrag(hItem);

		CTreeCtrl::OnLButtonUp(nFlags, point);
	}
}


////////////////////////////////////////////////////////
//
// scl import
//
////////////////////////////////////////////////////////

typedef double SclFloat;

CString CTuningDialog::GetSclImportFailureMsg(EnSclImport id)
//----------------------------------------------------------
{
	switch(id)
	{
		case enSclImportFailTooManyNotes:
			return mpt::tformat(_T("OpenMPT supports importing scl-files with at most %1 notes"))(mpt::tfmt::val(s_nSclImportMaxNoteCount));

		case enSclImportFailTooLargeNumDenomIntegers:
			return _T("Invalid numerator or denominator");

		case enSclImportFailZeroDenominator:
			return _T("Zero denominator");

		case enSclImportFailNegativeRatio:
			return _T("Negative ratio");

		case enSclImportFailUnableToOpenFile:
			return _T("Unable to open file");

		case enSclImportLineCountMismatch:
			return _T("Note count error");

		case enSclImportTuningCreationFailure:
			return _T("Unknown tuning creation error");

		case enSclImportAddTuningFailure:
			return _T("Can't add tuning to tuning collection");

		default:
			return _T("");
	}
}


static void SkipCommentLines(std::istream& iStrm, std::string& str)
//-----------------------------------------------------------------
{
	std::string whitespace(" \t");
	while(std::getline(iStrm, str))
	{
		auto start = str.find_first_not_of(whitespace);
		// Lines starting with a ! are comments
		if(start != std::string::npos && str[start] != '!')
			return;
	}
}


static inline SclFloat CentToRatio(const SclFloat& val)
//-----------------------------------------------------
{
	return pow(2.0, val / 1200.0);
}


CTuningDialog::EnSclImport CTuningDialog::ImportScl(const mpt::PathString &filename, const mpt::ustring &name, CTuning * & result)
//--------------------------------------------------------------------------------------------------------------------------------
{
	MPT_ASSERT(result == nullptr);
	result = nullptr;
	mpt::ifstream iStrm(filename, std::ios::in | std::ios::binary);
	if(!iStrm)
	{
		return enSclImportFailUnableToOpenFile;
	}
	return ImportScl(iStrm, name, result);
}


CTuningDialog::EnSclImport CTuningDialog::ImportScl(std::istream& iStrm, const mpt::ustring &name, CTuning * & result)
//--------------------------------------------------------------------------------------------------------------------
{
	MPT_ASSERT(result == nullptr);
	result = nullptr;
	std::string str;

	std::string filename;
	bool first = true;
	std::string whitespace(" \t");
	while(std::getline(iStrm, str))
	{
		auto start = str.find_first_not_of(whitespace);
		// Lines starting with a ! are comments
		if(start != std::string::npos && str[start] != '!')
			break;
		if(first)
		{
			filename = mpt::String::Trim(str.substr(start + 1), std::string(" \t\r\n"));
		}
		first = false;
	}
	std::string description = mpt::String::Trim(str, std::string(" \t\r\n"));

	SkipCommentLines(iStrm, str);
	// str should now contain number of notes.
	const size_t nNotes = 1 + ConvertStrTo<size_t>(str.c_str());
	if (nNotes - 1 > s_nSclImportMaxNoteCount)
		return enSclImportFailTooManyNotes;

	std::vector<std::string> names;
	std::vector<Tuning::RATIOTYPE> fRatios;
	fRatios.reserve(nNotes);
	fRatios.push_back(1);

	char buffer[128];
	MemsetZero(buffer);

	while (iStrm.getline(buffer, sizeof(buffer)))
	{
		LPSTR psz = buffer;
		LPSTR const pEnd = psz + strlen(buffer);

		// Skip tabs and spaces.
		while(psz != pEnd && (*psz == ' ' || *psz == '\t'))
			psz++;

		// Skip empty lines, comment lines and non-text.
		if (*psz == 0 || *psz == '!' || *psz < 32)
			continue;

		char* pNonDigit = pEnd;

		// Check type of first non digit. This tells whether to read cent, ratio or plain number.
		for (pNonDigit = psz; pNonDigit != pEnd; pNonDigit++)
		{
			if (isdigit(*pNonDigit) == 0)
				break;
		}

		if (*pNonDigit == '.') // Reading cents
		{
			SclFloat fCent = ConvertStrTo<SclFloat>(psz);
			fRatios.push_back(static_cast<Tuning::RATIOTYPE>(CentToRatio(fCent)));
		}
		else if (*pNonDigit == '/') // Reading ratios
		{
			*pNonDigit = 0; // Replace '/' with null.
			int64 nNum = ConvertStrTo<int64>(psz);
			psz = pNonDigit + 1;
			int64 nDenom = ConvertStrTo<int64>(psz);

			if (nNum > int32_max || nDenom > int32_max)
				return enSclImportFailTooLargeNumDenomIntegers;
			if (nDenom == 0)
				return enSclImportFailZeroDenominator;

			fRatios.push_back(static_cast<Tuning::RATIOTYPE>((SclFloat)nNum / (SclFloat)nDenom));
		}
		else // Plain numbers.
			fRatios.push_back(static_cast<Tuning::RATIOTYPE>(ConvertStrTo<int32>(psz)));

		std::string remainder = psz;
		remainder = mpt::String::Trim(remainder, std::string("\r\n"));
		if(remainder.find_first_of(" \t") != std::string::npos)
		{
			remainder = remainder.substr(remainder.find_first_of(" \t"));
		} else
		{
			remainder = std::string();
		}
		remainder = mpt::String::Trim(remainder, std::string(" \t"));
		if(!remainder.empty())
		{
			if(remainder[0] == '!')
			{
				remainder = remainder.substr(1);
				remainder = mpt::String::Trim(remainder, std::string(" \t"));
			}
		}
		if(mpt::ToLowerCaseAscii(remainder) == "cents" || mpt::ToLowerCaseAscii(remainder) == "cent")
		{
			remainder = std::string();
		}
		names.push_back(remainder);
		
	}

	if (nNotes != fRatios.size())
		return enSclImportLineCountMismatch;

	for(size_t i = 0; i < fRatios.size(); i++)
	{
		if (fRatios[i] < 0)
			return enSclImportFailNegativeRatio;
	}

	Tuning::RATIOTYPE groupRatio = fRatios.back();
	fRatios.pop_back();

	mpt::ustring tuningName;
	if(!description.empty())
	{
		tuningName = mpt::ToUnicode(mpt::CharsetISO8859_1, description);
	} else if(!filename.empty())
	{
		tuningName = mpt::ToUnicode(mpt::CharsetISO8859_1, filename);
	} else if(!name.empty())
	{
		tuningName = name;
	} else
	{
		tuningName = mpt::format(MPT_USTRING("%1 notes: %2:%3"))(nNotes - 1, mpt::ufmt::fix(groupRatio), 1);
	}

	CTuning* pT = CTuning::CreateGroupGeometric(mpt::ToCharset(mpt::CharsetLocale, tuningName), fRatios, groupRatio, 15);
	if(!pT)
	{
		return enSclImportTuningCreationFailure;
	}

	bool allNamesEmpty = true;
	bool allNamesValid = true;
	for(NOTEINDEXTYPE note = 0; note < mpt::saturate_cast<NOTEINDEXTYPE>(names.size()); ++note)
	{
		if(names[note].empty())
		{
			allNamesValid = false;
		} else
		{
			allNamesEmpty = false;
		}
	}

	if(nNotes - 1 == 12 && !allNamesValid)
	{
		for(NOTEINDEXTYPE note = 0; note < mpt::saturate_cast<NOTEINDEXTYPE>(names.size()); ++note)
		{
			pT->SetNoteName(note, CSoundFile::GetDefaultNoteNames()[note]);
		}
	} else
	{
		for(NOTEINDEXTYPE note = 0; note < mpt::saturate_cast<NOTEINDEXTYPE>(names.size()); ++note)
		{
			if(!names[note].empty())
			{
				pT->SetNoteName(note, names[(note - 1 + names.size()) % names.size()]);
			}
		}
	}

	result = pT;

	return enSclImportOk;
}


OPENMPT_NAMESPACE_END