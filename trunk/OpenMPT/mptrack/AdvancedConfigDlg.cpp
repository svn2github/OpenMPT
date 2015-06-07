/*
 * AdvancedConfigDlg.cpp
 * ---------------------
 * Purpose: Implementation of the advanced settings dialog.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Mainfrm.h"
#include "AdvancedConfigDlg.h"
#include "Settings.h"
#include "dlg_misc.h"

#if defined(MPT_SETTINGS_CACHE)

OPENMPT_NAMESPACE_BEGIN

BEGIN_MESSAGE_MAP(COptionsAdvanced, CPropertyPage)
	ON_NOTIFY(NM_DBLCLK,	IDC_LIST1,	OnOptionDblClick)
	ON_EN_CHANGE(IDC_EDIT1,				OnFindStringChanged)
END_MESSAGE_MAP()

void COptionsAdvanced::DoDataExchange(CDataExchange* pDX)
//-------------------------------------------------------
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModTypeDlg)
	DDX_Control(pDX, IDC_LIST1,			m_List);
	//}}AFX_DATA_MAP
}


BOOL COptionsAdvanced::PreTranslateMessage(MSG *msg)
//--------------------------------------------------
{
	if(msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN)
	{
		OnOptionDblClick(nullptr, nullptr);
		return TRUE;
	}
	return FALSE;
}


BOOL COptionsAdvanced::OnInitDialog()
//-----------------------------------
{
	CPropertyPage::OnInitDialog();

	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	ListView_EnableGroupView(m_List.m_hWnd, FALSE); // try to set known state
	int enableGroupsResult1 = static_cast<int>(ListView_EnableGroupView(m_List.m_hWnd, TRUE));
	int enableGroupsResult2 = static_cast<int>(ListView_EnableGroupView(m_List.m_hWnd, TRUE));
	// Looks like we have to check enabling and check that a second enabling does
	// not change anything.
	// Just checking if enabling fails with -1 does no work for older control
	// versions because they just do not know the window message at all and return
	// 0, always. At least Wine does behave this way.
	if(enableGroupsResult1 == 1 && enableGroupsResult2 == 0)
	{
		m_listGrouped = true;
	} else
	{
		// Did not behave as documented or expected, the actual state of the
		// control is unknown by now.
		// Play safe and set and assume the traditional ungrouped mode again.
		ListView_EnableGroupView(m_List.m_hWnd, FALSE);
		m_listGrouped = false;
	}

	if(m_listGrouped)
	{
		const CListCtrlEx::Header headers[] =
		{
			{ _T("Setting"), 100, LVCFMT_LEFT },
			{ _T("Type"),    40, LVCFMT_LEFT },
			{ _T("Value"),   190, LVCFMT_LEFT },
			{ _T("Default"), 62, LVCFMT_LEFT },
		};
		m_List.SetHeaders(headers);
	} else
	{
		const CListCtrlEx::Header headers[] =
		{
			{ _T("Setting"), 150, LVCFMT_LEFT },
			{ _T("Type"),    40, LVCFMT_LEFT },
			{ _T("Value"),   150, LVCFMT_LEFT },
			{ _T("Default"), 52, LVCFMT_LEFT },
		};
		m_List.SetHeaders(headers);
	}

	ReInit();
	return TRUE;
}


void COptionsAdvanced::ReInit()
//-----------------------------
{
	m_List.SetRedraw(FALSE);
	m_List.DeleteAllItems();
	m_List.SetItemCount(theApp.GetSettings().size());

	m_indexToPath.clear();
	m_indexToPath.reserve(theApp.GetSettings().size());
	m_groups.clear();
	int numGroups = 0;
	
	CStringW findStr;
	HWND findWnd = ::GetDlgItem(m_hWnd, IDC_EDIT1);
	int findLen = ::GetWindowTextLengthW(findWnd);
	::GetWindowTextW(findWnd, findStr.GetBufferSetLength(findLen), findLen + 1);
	findStr.ReleaseBuffer();
	findStr.MakeLower();

	LVITEMW lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | (m_listGrouped ? LVIF_GROUPID : 0);
	lvi.iSubItem = 0;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.cchTextMax = 0;
	lvi.iImage = 0;
	lvi.iIndent = 0;
	lvi.iGroupId = 0;

	int i = 0;
	for(SettingsContainer::SettingsMap::const_iterator it = theApp.GetSettings().begin(); it != theApp.GetSettings().end(); ++it)
	{
		const SettingPath &path = it->first;
		const SettingState &state = it->second;
		const mpt::ustring &section = path.GetRefSection();
		const mpt::ustring &key = path.GetRefKey();
		const SettingValue &value = state.GetRefValue();
		const SettingValue &defaultValue = state.GetRefDefault();

		if(!findStr.IsEmpty())
		{
			mpt::ustring str = path.FormatAsString() + MPT_USTRING("=") + value.FormatValueAsString();
			CStringW::StrTraits::StringLowercase(&str[0], str.size() + 1);
			if(str.find(findStr) == mpt::ustring::npos)
			{
				continue;
			}
		}

		int index;
		lvi.iItem = i++;
		lvi.lParam = m_indexToPath.size();

		if(m_listGrouped)
		{

			UNORDERED_MAP<mpt::ustring, int>::const_iterator gi = m_groups.find(section);
			if(gi == m_groups.end())
			{
				LVGROUP group;
	#if _WIN32_WINNT >= 0x0600
				group.cbSize = LVGROUP_V5_SIZE;
	#else
				group.cbSize = sizeof(group);
	#endif
				group.mask = LVGF_HEADER | LVGF_GROUPID;
				group.pszHeader = const_cast<wchar_t *>(section.c_str());
				group.cchHeader = 0;
				group.pszFooter = nullptr;
				group.cchFooter = 0;
				group.iGroupId = lvi.iGroupId = numGroups++;
				group.stateMask = LVGS_COLLAPSIBLE;
				group.state = LVGS_COLLAPSIBLE;
				group.uAlign = LVGA_HEADER_LEFT;
				ListView_InsertGroup(m_List.m_hWnd, -1, &group);
				m_groups.insert(std::make_pair(section, lvi.iGroupId));
			} else
			{
				lvi.iGroupId = gi->second;
			}

			lvi.pszText = const_cast<wchar_t *>(key.c_str());
			index = static_cast<int>(m_List.SendMessage(LVM_INSERTITEMW, 0, (LPARAM)(&lvi)));

		} else
		{

			const mpt::ustring sectionAndKey = path.FormatAsString();
			lvi.pszText = const_cast<wchar_t *>(sectionAndKey.c_str());
			index = static_cast<int>(m_List.SendMessage(LVM_INSERTITEMW, 0, (LPARAM)(&lvi)));

		}

		m_List.SetItemText(index, 1, value.FormatTypeAsString().c_str());
		m_List.SetItemText(index, 2, value.FormatValueAsString().c_str());
		m_List.SetItemText(index, 3, defaultValue.FormatValueAsString().c_str());
		m_indexToPath.push_back(path);
	}

	m_List.SetItemCount(i);
	m_List.SetRedraw(TRUE);
	m_List.Invalidate(FALSE);
}


void COptionsAdvanced::OnOK()
//---------------------------
{
	CPropertyPage::OnOK();
}


BOOL COptionsAdvanced::OnSetActive()
//----------------------------------
{
	ReInit();
	CMainFrame::m_nLastOptionsPage = OPTIONS_PAGE_ADVANCED;
	return CPropertyPage::OnSetActive();
}


void COptionsAdvanced::OnOptionDblClick(NMHDR *, LRESULT *)
//---------------------------------------------------------
{
	const int index = m_List.GetSelectionMark();
	const SettingPath path = m_indexToPath[m_List.GetItemData(index)];
	SettingValue val = theApp.GetSettings().GetMap().find(path)->second;
	if(val.GetType() == SettingTypeBool)
	{
		val = !val.as<bool>();
	} else
	{
		CInputDlg inputDlg(this, _T("Enter new value for ") + mpt::ToCString(path.FormatAsString()), mpt::ToCString(val.FormatValueAsString()));
		if(inputDlg.DoModal() != IDOK)
		{
			return;
		}
		val.SetFromString(inputDlg.resultString);
	}
	theApp.GetSettings().Write(path, val);
	m_List.SetItemText(index, 2, val.FormatValueAsString().c_str());
	m_List.SetSelectionMark(index);
	OnSettingsChanged();
}

OPENMPT_NAMESPACE_END

#endif // MPT_SETTINGS_CACHE
