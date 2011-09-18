/*
 * CloseMainDialog.h
 * -----------------
 * Purpose: Header file for unsaved documents dialog.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 */

#ifndef CLOSEMAINDIALOG_H
#define CLOSEMAINDIALOG_H
#pragma once

//===================================
class CloseMainDialog: public CDialog
//===================================
{
protected:
	
	CListBox m_List;

	CString FormatTitle(const CModDoc *pModDoc, bool fullPath);

public:
	CloseMainDialog() : CDialog(IDD_CLOSEDOCUMENTS) { CMainFrame::GetInputHandler()->Bypass(true); };
	~CloseMainDialog() { CMainFrame::GetInputHandler()->Bypass(false); };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnSaveAll();
	afx_msg void OnSaveNone();

	afx_msg void OnSwitchFullPaths();

	DECLARE_MESSAGE_MAP()

};

#endif // CLOSEMAINDIALOG_H