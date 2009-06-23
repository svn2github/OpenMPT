#include "stdafx.h"
#include "mptrack.h"
#include "TuningDialog.h"
#include <algorithm>
#include "misc_util.h"
#include ".\tuningdialog.h"

const CTuningDialog::TUNINGTREEITEM CTuningDialog::s_notFoundItemTuning = TUNINGTREEITEM();
const HTREEITEM CTuningDialog::s_notFoundItemTree = NULL;

typedef CTuning::UNOTEINDEXTYPE UNOTEINDEXTYPE;
typedef CTuning::RATIOTYPE RATIOTYPE;
#define TT_GENERAL CTuning::TT_GENERAL
#define TT_GROUPGEOMETRIC CTuning::TT_GROUPGEOMETRIC
#define TT_GEOMETRIC CTuning::TT_GEOMETRIC
#define EM_CONST CTuning::EM_CONST
#define EM_CONST_STRICT CTuning::EM_CONST_STRICT
#define TUNINGTYPE CTuning::TUNINGTYPE
#define NOTEINDEXTYPE CTuning::NOTEINDEXTYPE
#define EM_ALLOWALL CTuning::EM_ALLOWALL

using namespace std;


/*
TODOS:
-Clear tuning
-Tooltips.
-Create own dialogs for Tuning collection part, and Tuning part.
*/


// CTuningDialog dialog

IMPLEMENT_DYNAMIC(CTuningDialog, CDialog)
CTuningDialog::CTuningDialog(CWnd* pParent, const TUNINGVECTOR& rVec, CTuning* pTun)
	: CDialog(CTuningDialog::IDD, pParent),
	m_TuningCollections(rVec),
	m_TempTunings("Sandbox"),
	m_NoteEditApply(true),
	m_RatioEditApply(true),
	m_pActiveTuningCollection(NULL),
	m_TreeItemTuningItemMap(s_notFoundItemTree, s_notFoundItemTuning),
	m_TreeCtrlTuning(this),
	m_DoErrorExit(false)
//----------------------------------------
{
	m_pActiveTuning = pTun;
	m_RatioMapWnd.m_pTuning = pTun; //pTun is the tuning to show when dialog opens.

	m_TuningCollections.push_back(&m_TempTunings);
}

CTuningDialog::~CTuningDialog()
//----------------------------
{
	for(size_t i = 0; i<m_TuningCollections.size(); i++)
	{
		if(IsDeletable(m_TuningCollections[i]))
		{
			delete m_TuningCollections[i];
			m_TuningCollections[i] = NULL;
		}
	}
	m_TuningCollections.clear();
	m_DeletableTuningCollections.clear();
}

HTREEITEM CTuningDialog::AddTreeItem(CTuningCollection* pTC, HTREEITEM parent, HTREEITEM insertAfter)
//---------------------------------------------------------------------------------------------------
{
	const HTREEITEM temp = m_TreeCtrlTuning.InsertItem(pTC->GetName().c_str(), parent, insertAfter);
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
	const HTREEITEM temp = m_TreeCtrlTuning.InsertItem(pT->GetName().c_str(), parent, insertAfter);
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
//---------------------------------------------
{
	if(!pTC)
		return;

	m_pActiveTuning = NULL;
	const HTREEITEM temp = m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTC));
	if(temp)
	{
		TUNINGTREEITEM prevTTI = m_TreeItemTuningItemMap.GetMapping_12(m_TreeCtrlTuning.GetNextItem(temp, TVGN_PREVIOUS));
		TUNINGTREEITEM nextTTI = m_TreeItemTuningItemMap.GetMapping_12(m_TreeCtrlTuning.GetNextItem(temp, TVGN_NEXT));

		CTuningCollection* pTCprev = prevTTI.GetTC();
		CTuningCollection* pTCnext = nextTTI.GetTC();
		if(pTCnext == NULL)
			pTCnext = GetpTuningCollection(nextTTI.GetT());
		if(pTCprev == NULL)
			pTCprev = GetpTuningCollection(prevTTI.GetT());
        
		if(pTCnext != NULL && pTCnext != m_pActiveTuningCollection)
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

	m_RatioMapWnd.Init(this, 0);

	SetDlgItemText(IDC_TUNINGTYPE_DESC, "");

	//-->Creating treeview
	m_TreeItemTuningItemMap.ClearMapping();
	for(size_t i = 0; i<m_TuningCollections.size(); i++)
	{
		AddTreeItem(m_TuningCollections[i], NULL, NULL);
	}
	//<-- Creating treeview

	m_pActiveTuningCollection = GetpTuningCollection(m_pActiveTuning);

	//Adding tuning type names to corresponding combobox.
	m_CombobTuningType.AddString(CTuning::s_TuningTypeStrGeneral);
	m_CombobTuningType.AddString(CTuning::s_TuningTypeStrGroupGeometric);
	m_CombobTuningType.AddString(CTuning::s_TuningTypeStrGeometric);

	m_ButtonSet.EnableWindow(FALSE);

	//#ifdef DEBUG
		m_EditTuningCollectionVersion.ShowWindow(SW_SHOW);
		m_EditTuningCollectionEditMask.ShowWindow(SW_SHOW);
	//#endif

	m_EditSteps.SetLimitText(2);
	m_EditFineTuneSteps.SetLimitText(3);

	if(m_pActiveTuning) m_RatioMapWnd.m_nNote =  m_RatioMapWnd.m_nNoteCentre + m_pActiveTuning->GetValidityRange().first + (m_pActiveTuning->GetValidityRange().second - m_pActiveTuning->GetValidityRange().first)/2 + 1;

	UpdateView();

	return TRUE;
}


void CTuningDialog::UpdateView(const int updateMask)
//------------------------------
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
				m_TreeCtrlTuning.SetItemText(treeitem, m_pActiveTuning->GetName().c_str());
			else
				m_TreeCtrlTuning.SetItemText(treeitem, m_pActiveTuningCollection->GetName().c_str());
		}		
	}
	//<--Updating treeview
	

	if(m_pActiveTuningCollection == NULL)
	{
		return;
	}

	//Updating tuning collection part-->
	if(updateMask == 0 || updateMask & UM_TUNINGCOLLECTION)
	{
		m_EditTuningCollectionName.SetWindowText(m_pActiveTuningCollection->GetName().c_str());
		m_EditTuningCollectionVersion.SetWindowText(m_pActiveTuningCollection->GetVersionString().c_str());
		m_EditTuningCollectionEditMask.SetWindowText(m_pActiveTuningCollection->GetEditMaskString().c_str());
		m_EditTuningCollectionItemNum.SetWindowText(Stringify(m_pActiveTuningCollection->GetNumTunings()).c_str());
		m_EditTuningCollectionPath.SetWindowText(m_pActiveTuningCollection->GetSaveFilePath().c_str());
	}
	//<-- Updating tuning collection part

	//Updating tuning part-->
	if(m_pActiveTuning != NULL && (updateMask & UM_TUNINGDATA || updateMask == 0))
	{
		UpdateTuningType();

		m_EditName.SetWindowText(m_pActiveTuning->GetName().c_str());
		m_EditName.Invalidate();

		//Finetunesteps-edit
		m_EditFineTuneSteps.SetWindowText(Stringify(m_pActiveTuning->GetFineStepCount()).c_str());
		m_EditFineTuneSteps.Invalidate();

		//Making sure that ratiomap window is showing and
		//updating its content.
		m_RatioMapWnd.ShowWindow(SW_SHOW);
		m_RatioMapWnd.m_pTuning = m_pActiveTuning;
		m_RatioMapWnd.Invalidate();
		UpdateRatioMapEdits(m_RatioMapWnd.GetShownCentre());


		const UNOTEINDEXTYPE period = m_pActiveTuning->GetGroupSize();
		const RATIOTYPE GroupRatio = m_pActiveTuning->GetGroupRatio();
		if(period > 0)
		{
			m_EditSteps.EnableWindow();
			m_EditSteps.SetWindowText(Stringify(period).c_str());

			m_EditRatioPeriod.EnableWindow();
			m_EditRatioPeriod.SetWindowText(Stringify(GroupRatio).c_str());
		}
		else //case: m_pActiveTuning is of type general.
		{
			//m_EditSteps.EnableWindow(false);
			//m_EditRatioPeriod.EnableWindow(false);
		}

		m_EditRatioPeriod.Invalidate();
		m_EditSteps.Invalidate();

		bool enableControls = true;

		if(m_pActiveTuning->GetEditMask() == EM_CONST ||
		m_pActiveTuning->GetEditMask() == EM_CONST_STRICT)
		{
			CheckDlgButton(IDC_CHECK_READONLY, MF_CHECKED);
			if(m_pActiveTuning->GetEditMask() == EM_CONST_STRICT)
				m_ButtonReadOnly.EnableWindow(FALSE);
			else
				m_ButtonReadOnly.EnableWindow(TRUE);

			enableControls = false;
		}
		else
		{
			CheckDlgButton(IDC_CHECK_READONLY, MF_UNCHECKED);
			m_ButtonReadOnly.EnableWindow();
		}

		m_CombobTuningType.EnableWindow(enableControls);
		m_EditTableSize.SetReadOnly(!enableControls);
		m_EditBeginNote.SetReadOnly(!enableControls);
		m_EditSteps.SetReadOnly(!enableControls);
		m_EditRatioPeriod.SetReadOnly(!enableControls);
		m_EditRatio.SetReadOnly(!enableControls);
		m_EditNotename.SetReadOnly(!enableControls);
		m_EditMiscActions.SetReadOnly(!enableControls);
		m_EditFineTuneSteps.SetReadOnly(!enableControls);
		m_EditName.SetReadOnly(!enableControls);
		
		m_ButtonSet.EnableWindow(enableControls);

		m_CombobTuningType.Invalidate();
		m_EditSteps.Invalidate();
		m_EditRatioPeriod.Invalidate();
	}
	else 
	{
		if(m_pActiveTuning == NULL) //No active tuning, clearing tuning part.
		{
			m_EditName.SetWindowText("");
			m_EditTableSize.SetWindowText("");
			m_EditBeginNote.SetWindowText("");
			m_EditSteps.SetWindowText("");
			m_EditRatioPeriod.SetWindowText("");
			m_EditRatio.SetWindowText("");
			m_EditNotename.SetWindowText("");
			m_EditMiscActions.SetWindowText("");
			m_EditFineTuneSteps.SetWindowText("");
			m_EditName.SetWindowText("");

			SetDlgItemText(IDC_TUNINGTYPE_DESC, "");

			m_CombobTuningType.SetCurSel(-1);

			m_ButtonReadOnly.EnableWindow(FALSE);

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
	DDX_Control(pDX, IDC_EDIT_TABLESIZE, m_EditTableSize);
	DDX_Control(pDX, IDC_EDIT_BEGINNOTE, m_EditBeginNote);
	DDX_Control(pDX, IDC_EDIT_STEPS, m_EditSteps);
	DDX_Control(pDX, IDC_EDIT_RATIOPERIOD, m_EditRatioPeriod);
	DDX_Control(pDX, IDC_EDIT_RATIOVALUE, m_EditRatio);
	DDX_Control(pDX, IDC_EDIT_NOTENAME, m_EditNotename);
	DDX_Control(pDX, IDC_BUTTON_SETVALUES, m_ButtonSet);
	DDX_Control(pDX, IDC_BUTTON_EXPORT, m_ButtonExport);
	DDX_Control(pDX, IDC_BUTTON_IMPORT, m_ButtonImport);
	DDX_Control(pDX, IDC_EDIT_MISC_ACTIONS, m_EditMiscActions);
	DDX_Control(pDX, IDC_EDIT_FINETUNESTEPS, m_EditFineTuneSteps);
	DDX_Control(pDX, IDC_CHECK_READONLY, m_ButtonReadOnly);
	DDX_Control(pDX, IDC_EDIT_NAME, m_EditName);
	DDX_Control(pDX, IDC_TREE_TUNING, m_TreeCtrlTuning);
	DDX_Control(pDX, IDC_EDIT_TUNINGCOLLECTION_NAME, m_EditTuningCollectionName);
	DDX_Control(pDX, IDC_EDIT_TUNINGC_VERSION, m_EditTuningCollectionVersion);
	DDX_Control(pDX, IDC_EDIT_TUNINGC_EDITMASK, m_EditTuningCollectionEditMask);
	DDX_Control(pDX, IDC_EDIT_TUNINGNUM, m_EditTuningCollectionItemNum);
	DDX_Control(pDX, IDC_EDIT_TUNINGCOLLECTION_PATH, m_EditTuningCollectionPath);
}



BEGIN_MESSAGE_MAP(CTuningDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_TTYPE, OnCbnSelchangeComboTtype)
	ON_EN_CHANGE(IDC_EDIT_STEPS, OnEnChangeEditSteps)
	ON_EN_CHANGE(IDC_EDIT_RATIOPERIOD, OnEnChangeEditRatioperiod)
	ON_EN_CHANGE(IDC_EDIT_NOTENAME, OnEnChangeEditNotename)
	ON_BN_CLICKED(IDC_BUTTON_SETVALUES, OnBnClickedButtonSetvalues)
	ON_EN_CHANGE(IDC_EDIT_RATIOVALUE, OnEnChangeEditRatiovalue)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, OnBnClickedButtonImport)
	ON_EN_CHANGE(IDC_EDIT_FINETUNESTEPS, OnEnChangeEditFinetunesteps)
	ON_EN_KILLFOCUS(IDC_EDIT_FINETUNESTEPS, OnEnKillfocusEditFinetunesteps)
	ON_BN_CLICKED(IDC_CHECK_READONLY, OnBnClickedCheckReadonly)
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
	ON_COMMAND(ID_ADDTUNING, OnAddTuning)
	ON_COMMAND(ID_MOVETUNING, OnMoveTuning)
	ON_COMMAND(ID_COPYTUNING, OnCopyTuning)
	ON_COMMAND(ID_REMOVETUNINGCOLLECTION, OnRemoveTuningCollection)
	ON_BN_CLICKED(IDC_BUTTON_TUNINGCOLLECTION_SAVE, OnBnClickedButtonTuningcollectionSave)
END_MESSAGE_MAP()


void CTuningDialog::DoErrorExit()
//-------------------------------
{
	m_DoErrorExit = false;
	m_pActiveTuning = NULL;
	m_pActiveTuningCollection = NULL;
	MsgBox(IDS_ERR_DIALOG, this, NULL, MB_ICONINFORMATION);
    OnOK();
}


// CTuningDialog message handlers

void CTuningDialog::UpdateTuningType()
//------------------------------------
{
	if(m_pActiveTuning)
	{
		if(m_CombobTuningType.GetCount() < 3) m_DoErrorExit = true;

		if(m_pActiveTuning->GetTuningType() == TT_GEOMETRIC)
			m_CombobTuningType.SetCurSel(2);
		else
			if(m_pActiveTuning->GetTuningType() == TT_GROUPGEOMETRIC)
				m_CombobTuningType.SetCurSel(1);
			else
				m_CombobTuningType.SetCurSel(0);
	}
	UpdateTuningDescription();
}


TUNINGTYPE CTuningDialog::GetTuningTypeFromStr(const string& str) const
//--------------------------------------------------------------------------------
{
	return CTuning::GetTuningType(str.c_str());
}

void CTuningDialog::OnCbnSelchangeComboTtype()
//--------------------------------------------
{
	if(m_pActiveTuning != NULL)
	{
		const TUNINGTYPE oldType = m_pActiveTuning->GetTuningType();
		const size_t BS = 20;
		char buffer[BS];
		m_CombobTuningType.GetWindowText(buffer, BS);
		const string strNewType = buffer;
		TUNINGTYPE newType = GetTuningTypeFromStr(strNewType);
		if(!m_pActiveTuning->IsOfType(newType))
		{
			if(MessageBox("This action may change the ratio values; continue?", 0, MB_YESNO) == IDYES)
			{
				m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;

				const size_t BS = 20;
				char buffer[BS];
				m_EditSteps.GetWindowText(buffer, BS);
				NOTEINDEXTYPE steps = ConvertStrTo<NOTEINDEXTYPE>(buffer);

				m_EditRatioPeriod.GetWindowText(buffer, BS);
				RATIOTYPE pr = ConvertStrTo<RATIOTYPE>(buffer);

				if(steps <= 0)
						steps = 1;
					if(pr <= 0)
						pr = 1;

				if(newType == TT_GROUPGEOMETRIC)
					m_pActiveTuning->CreateGroupGeometric(steps, pr, 0);
				else
					if(newType == TT_GEOMETRIC)
						m_pActiveTuning->CreateGeometric(steps, pr);
				
				UpdateView(UM_TUNINGDATA);
			}
			else //Not wanting to discard current values.
			{
				//Restoring tuning type combobox.
				if(oldType == TT_GEOMETRIC)
					m_CombobTuningType.SetCurSel(2);
				else
				{
					if(oldType == TT_GROUPGEOMETRIC)
						m_CombobTuningType.SetCurSel(1);
					else 
						m_CombobTuningType.SetCurSel(0);
				}
			}
		}
	}
	UpdateTuningDescription();
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
	const size_t BS = 5;
	char buffer[BS];
	m_EditNotename.GetWindowText(buffer, BS);
	string str = string(buffer);
	if(str.length() > 0)
	{
		if(str.size() > 3)
			str.resize(3);
		m_pActiveTuning->SetNoteName(currentNote, str);
	}
	else
		m_pActiveTuning->ClearNoteName(currentNote);

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

	const size_t BS = 12;
	char buffer[BS];
	m_EditRatio.GetWindowText(buffer, BS);
	string str = buffer;
	if(str.length() > 0)
	{
		m_pActiveTuning->SetRatio(currentNote, ConvertStrTo<RATIOTYPE>(buffer));
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

		const size_t BS = 20;
		char buffer[BS];
		m_EditMiscActions.GetWindowText(buffer, BS);
		m_pActiveTuning->Multiply(ConvertStrTo<RATIOTYPE>(buffer));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		m_EditMiscActions.SetWindowText("");
		m_RatioMapWnd.Invalidate();
	}
}

void CTuningDialog::UpdateRatioMapEdits(const NOTEINDEXTYPE& note)
//-----------------------------------------------------------
{
	if(m_pActiveTuning == NULL)
		return;

	m_RatioEditApply = false;
	m_EditRatio.SetWindowText(Stringify(m_pActiveTuning->GetRatio(note)).c_str());
	m_NoteEditApply = false;
	m_EditNotename.SetWindowText(m_pActiveTuning->GetNoteName(note).c_str());

	m_EditRatio.Invalidate();
	m_EditNotename.Invalidate();
}



void CTuningDialog::OnBnClickedButtonExport()
//-------------------------------------------
{	
	const CTuning* pT = m_pActiveTuning;
	const CTuningCollection* pTC = m_pActiveTuningCollection;

	if(pT == NULL && pTC == NULL)
	{
		MsgBox(IDS_ERR_NO_TUNING_SELECTION, this, NULL, MB_ICONINFORMATION);
		return;
	}
		
	string filter;
	if(pT != NULL)
		filter = string("Tuning files (*") + CTuning::s_FileExtension + string(")|*") + CTuning::s_FileExtension + string("|");
	if(pTC != NULL)
		filter += string("Tuning collection files (") + CTuningCollection::s_FileExtension + string(")|*") + CTuningCollection::s_FileExtension + string("|");

	CFileDialog dlg(FALSE, CTuning::s_FileExtension.c_str(),
			NULL,
			OFN_HIDEREADONLY| OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
			 filter.c_str(), this);

	bool failure = true;
	
	if (dlg.DoModal() != IDOK) return;
	BeginWaitCursor();
	ofstream fout(dlg.GetPathName(), ios::binary);
	const string ext = "." + dlg.GetFileExt();

	if(ext == CTuning::s_FileExtension)
	{
		if(pT != NULL)
			failure = pT->Serialize(fout);
	}
	else //Case: Saving tuning collection.
		if(ext == CTuningCollection::s_FileExtension)
			if(pTC != NULL)
				failure = pTC->Serialize(fout);

	fout.close();
	EndWaitCursor();

	
	if(failure)
		ErrorBox(IDS_ERR_EXPORT_TUNING, this);
}

void CTuningDialog::OnBnClickedButtonImport()
//-------------------------------------------
{
	//TODO: Ability to import ratios from text file.
	string filter = string("Tuning files (*") + CTuning::s_FileExtension + string(", *") + CTuningCollection::s_FileExtension + string(")|*") + 
					CTuning::s_FileExtension + string(";*") + CTuningCollection::s_FileExtension + string("|");

	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
					filter.c_str(),
					this);

	if (dlg.DoModal() != IDOK) return;

	const string ext = string(".") + string(dlg.GetFileExt());

	bool failure = true;

	if(ext == CTuning::s_FileExtension)
	{
		ifstream fin(dlg.GetPathName(), ios::binary);
		CTuning* pT = CTuningRTI::UnserializeOLD(fin);
		if(pT == 0) {fin.clear(); fin.seekg(0); pT = CTuning::Unserialize(fin);}
		fin.close();
		if(pT)
		{
			failure = false;
			m_TempTunings.AddTuning(pT);
			m_pActiveTuning = pT;

			AddTreeItem(m_pActiveTuning, m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(&m_TempTunings)), NULL);
			UpdateView();
		}
	}
	else
	{
		if(ext == CTuningCollection::s_FileExtension)
		{
			//For now only loading tuning collection as 
			//a separate collection - no possibility to 
			//directly replace some collection.
			CTuningCollection* pNewTCol = new CTuningCollection;
			pNewTCol->SetSavefilePath(static_cast<LPCTSTR>(dlg.GetPathName()));
			failure = pNewTCol->Unserialize();
			if(failure)
			{
				delete pNewTCol; pNewTCol = 0;
			}
			else
			{
				m_TuningCollections.push_back(pNewTCol);
				m_DeletableTuningCollections.push_back(pNewTCol);
				AddTreeItem(pNewTCol, NULL, NULL);
				UpdateView();
			}


		}
	}
	if(failure)
		ErrorBox(IDS_OPERATION_FAIL, this);
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
		const BYTE BS = 5;
		char buffer[BS];
		m_EditFineTuneSteps.GetWindowText(buffer, BS);
		m_EditFineTuneSteps.SetWindowText(Stringify(m_pActiveTuning->SetFineStepCount(ConvertStrTo<CTuning::USTEPINDEXTYPE>(buffer))).c_str());
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		m_EditFineTuneSteps.Invalidate();
	}
}


void CTuningDialog::OnBnClickedCheckReadonly()
//--------------------------------------------
{
	if(m_pActiveTuning == NULL)
		return;

	if(IsDlgButtonChecked(IDC_CHECK_READONLY))
	{
		if(m_pActiveTuning->SetEditMask(EM_CONST))
			CheckDlgButton(IDC_CHECK_READONLY, MF_UNCHECKED);
		else
			UpdateView(UM_TUNINGDATA);
	}
	else
	{
		if(m_pActiveTuning->SetEditMask(EM_ALLOWALL))
			CheckDlgButton(IDC_CHECK_READONLY, MF_CHECKED);
		else
			UpdateView(UM_TUNINGDATA);
	}
	m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
}


void CTuningDialog::OnEnKillfocusEditName()
//-----------------------------------------
{
	if(m_pActiveTuning != NULL)
	{
		const size_t BS = 40;
		char buffer[BS];
		m_EditName.GetWindowText(buffer, BS);
		m_pActiveTuning->SetName(buffer);
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
		const size_t BS = 20;
		char buffer[BS];
		m_EditSteps.GetWindowText(buffer, BS);
		m_pActiveTuning->ChangeGroupsize(ConvertStrTo<UNOTEINDEXTYPE>(buffer));
		m_ModifiedTCs[GetpTuningCollection(m_pActiveTuning)] = true;
		UpdateView(UM_TUNINGDATA);
	}	
}


void CTuningDialog::OnEnKillfocusEditRatioperiod()
//--------------------------------------------
{
	if(m_pActiveTuning)
	{
		const size_t BS = 20;
		char buffer[BS];
		m_EditRatioPeriod.GetWindowText(buffer, BS);
		m_pActiveTuning->ChangeGroupRatio(ConvertStrTo<RATIOTYPE>(buffer));
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
	MODIFIED_MAP::const_iterator iter = m_ModifiedTCs.find(pTc);
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
//-----------------------------------------------------------------
{
	for(size_t i = 0; i<m_TuningCollections.size(); i++)
	{
		CTuningCollection& rCurTCol = *m_TuningCollections.at(i);
		for(size_t j = 0; j<rCurTCol.GetNumTunings(); j++)
		{
			if(pT == &rCurTCol.GetTuning(j))
			{
				return &rCurTCol;
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
//-----------------------------------------------------------------------
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
			if(!pTC->CanEdit(CTuningCollection::EM_REMOVE))
				mask |= MF_GRAYED;

			AppendMenu(popUpMenu, mask, ID_REMOVETUNING, "Remove");

			m_CommandItemDest.Set(pT);
		}
	}
	else //Creating context menu for tuning collection item.
	{
		if(pTC != NULL)
		{
			UINT mask = MF_STRING;

			if(!pTC->CanEdit(CTuningCollection::EM_ADD))
				mask |= MF_GRAYED;

			AppendMenu(popUpMenu, mask, ID_ADDTUNING, "Add tuning");

			mask = MF_STRING;
			if(!IsDeletable(pTC))
				mask |= MF_GRAYED;

			AppendMenu(popUpMenu, mask, ID_REMOVETUNINGCOLLECTION, "Delete tuning collection");

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
	vector<CTuningCollection*>::const_iterator iter = find(m_DeletableTuningCollections.begin(), m_DeletableTuningCollections.end(), pTC);
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
		MsgBox(IDS_UNSUPPORTED_TUNING_DnD, this);
		return;
	}

	m_TreeCtrlTuning.SetDragging();
	m_DragItem = m_TreeItemTuningItemMap.GetMapping_12(pNMTreeView->itemNew.hItem);

	m_TreeCtrlTuning.Select(pNMTreeView->itemNew.hItem, TVGN_CARET);
}

void CTuningDialog::OnEndDrag(HTREEITEM dragDestItem)
//--------------------------------------------------
{
	m_TreeCtrlTuning.SetDragging(false);
	if(m_DragItem == NULL)
		return;

	m_CommandItemSrc = m_DragItem;
	m_DragItem.Reset();

	TUNINGTREEITEM destTunItem = m_TreeItemTuningItemMap.GetMapping_12(dragDestItem);
	if(!destTunItem)
		return;

	CTuningCollection* pTCdest = NULL;
	CTuningCollection* pTCsrc = m_CommandItemSrc.GetTC();
		
	if(pTCsrc == NULL)
		pTCsrc = GetpTuningCollection(m_CommandItemSrc.GetT());

	if(pTCsrc == NULL)
	{
		ASSERT(false);
		return;
	}

	if(destTunItem.GetT()) //Item dragged on tuning
		pTCdest = GetpTuningCollection(destTunItem.GetT());
	else //Item dragged on tuningcollecition
        pTCdest = destTunItem.GetTC();

	//For now, ignoring drags within a tuning collection.
	if(pTCdest == pTCsrc) 
		return;
		
	if(pTCdest)
	{
		UINT mask = MF_STRING;
		HMENU popUpMenu = CreatePopupMenu();
		if(popUpMenu == NULL) return;

		POINT point;
		GetCursorPos(&point);

		if(!pTCdest->CanEdit(CTuningCollection::EM_ADD))
			mask |= MF_GRAYED;
		AppendMenu(popUpMenu, mask, ID_COPYTUNING, "Copy here");

		if(!pTCsrc->CanEdit(CTuningCollection::EM_REMOVE) ||
			!pTCdest->CanEdit(CTuningCollection::EM_ADD))
			mask = MF_STRING | MF_GRAYED;

		AppendMenu(popUpMenu, mask, ID_MOVETUNING, "Move here");

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
		MessageBox("No tuning collection chosen", 0, MB_OK); 
		return true;
	}

	CTuning* pNewTuning = new CTuningRTI(pT);
	if(pTC->AddTuning(pNewTuning))
	{
		MessageBox("Add tuning failed");
		delete pNewTuning;
		return true;
	}
	AddTreeItem(pNewTuning, m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTC)), NULL);
	m_pActiveTuning = pNewTuning;
	m_ModifiedTCs[pTC] = true;
	UpdateView();

	return false;
}

void CTuningDialog::OnAddTuning()
//-------------------------------
{
	if(!m_CommandItemDest.GetTC())
	{
		m_CommandItemDest = s_notFoundItemTuning;
		return;
	}

	CTuningCollection* pTC = m_CommandItemDest.GetTC();
	m_CommandItemDest = s_notFoundItemTuning;
	m_ModifiedTCs[pTC];
	AddTuning(pTC);
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
			string str = string("Remove tuning '") + pT->GetName() + string("' from ' ") + pTC->GetName() + string("'?");
			if(MessageBox(str.c_str(), 0, MB_YESNO) == IDYES)
			{
				if(!pTC->Remove(pT))
				{
					m_ModifiedTCs[pTC] = true;
					DeleteTreeItem(pT);
					UpdateView();
				}
				else
				{
					MessageBox("Tuning removal failed");
				}
			}
		}
	}

	m_CommandItemDest = s_notFoundItemTuning;
}

void CTuningDialog::OnMoveTuning()
//--------------------------------
{
	if(!m_CommandItemDest)
		return;

	CTuning* pT = m_CommandItemSrc.GetT();
	CTuningCollection* pTCsrc = GetpTuningCollection(pT);

	if(pT == NULL)
	{
		m_CommandItemDest = s_notFoundItemTuning;
		return;
	}
		
	CTuningCollection* pTCdest = NULL;
	if(m_CommandItemDest.GetT())
		pTCdest = GetpTuningCollection(m_CommandItemDest.GetT());
	else
		pTCdest = m_CommandItemDest.GetTC();


	HTREEITEM treeItemSrcTC = m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTCsrc));
	HTREEITEM treeItemDestTC = m_TreeItemTuningItemMap.GetMapping_21(TUNINGTREEITEM(pTCdest));
	DeleteTreeItem(pT);
	m_ModifiedTCs[pTCsrc] = true;
	m_ModifiedTCs[pTCdest] = true;
	if(CTuningCollection::TransferTuning(pTCsrc, pTCdest, pT))
	{
		MsgBox(IDS_OPERATION_FAIL, this, NULL, MB_OK);
		AddTreeItem(pT, treeItemSrcTC, NULL);
	}
	else
		AddTreeItem(pT, treeItemDestTC, NULL);

	UpdateView();
}


void CTuningDialog::OnCopyTuning()
//--------------------------------
{
	CTuningCollection* pTC = m_CommandItemDest.GetTC();

	if(!pTC)
		return;
	
	m_CommandItemDest = s_notFoundItemTuning;

	CTuning* pT = m_CommandItemSrc.GetT();
	if(pT == NULL)
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

	TUNINGVECTOR::iterator iter = find(m_TuningCollections.begin(), m_TuningCollections.end(), m_pActiveTuningCollection);
	if(iter == m_TuningCollections.end())
	{
		ASSERT(false);
		return;
	}
	TUNINGVECTOR::iterator DTCiter = find(m_DeletableTuningCollections.begin(), m_DeletableTuningCollections.end(), *iter);
	CTuningCollection* deletableTC = m_pActiveTuningCollection;
	//Note: Order matters in the following lines.
	m_DeletableTuningCollections.erase(DTCiter);
	m_TuningCollections.erase(iter);
	DeleteTreeItem(m_pActiveTuningCollection);
	delete deletableTC; deletableTC = 0;
	
	UpdateView();
}


void CTuningDialog::OnBnClickedButtonTuningcollectionSave()
//---------------------------------------------------------
{
	if(!m_pActiveTuningCollection)
		return;

	if(m_pActiveTuningCollection->Serialize())
	{
		MsgBox(IDS_OPERATION_FAIL, this, NULL, MB_ICONINFORMATION);
	}
	else
	{
		MessageBox("Saving succesful.");
		m_ModifiedTCs[m_pActiveTuningCollection] = false;
	}
}

void CTuningDialog::UpdateTuningDescription()
//-------------------------------------------
{
	switch(m_CombobTuningType.GetCurSel())
	{
		case 0:
			SetDlgItemText(IDC_TUNINGTYPE_DESC, CTuning::GetTuningTypeDescription(TT_GENERAL));
		break;

		case 1:
			SetDlgItemText(IDC_TUNINGTYPE_DESC, CTuning::GetTuningTypeDescription(TT_GROUPGEOMETRIC));
		break;

		case 2:
			SetDlgItemText(IDC_TUNINGTYPE_DESC, CTuning::GetTuningTypeDescription(TT_GEOMETRIC));
		break;

		default:
			if(m_pActiveTuning)
				SetDlgItemText(IDC_TUNINGTYPE_DESC, m_pActiveTuning->GetTuningTypeDescription());
			else
				SetDlgItemText(IDC_TUNINGTYPE_DESC, "Unknown type");
		break;
	}
}


void CTuningDialog::OnOK()
//------------------------
{
	// Prevent return-key from closing the window.
	if(GetKeyState(VK_RETURN) <= -127)
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
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


void CTuningTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
//-----------------------------------------------------------
{
	if(IsDragging())
	{
		HTREEITEM hItem;
		hItem = HitTest(point, NULL);
		m_rParentDialog.OnEndDrag(hItem);
	
		CTreeCtrl::OnLButtonUp(nFlags, point);	
	}
}


