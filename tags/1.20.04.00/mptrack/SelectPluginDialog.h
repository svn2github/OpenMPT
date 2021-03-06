/*
 * SelectPluginDialog.h
 * --------------------
 * Purpose: Dialog for adding plugins to a song.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once
#ifndef SELECTPLUGINDIALOG_H
#define SELECTPLUGINDIALOG_H

#include "Vstplug.h"

//====================================
class CSelectPluginDlg: public CDialog
//====================================
{
protected:
	int m_nPlugSlot;
	SNDMIXPLUGIN *m_pPlugin;
	CModDoc *m_pModDoc;
	CTreeCtrl m_treePlugins;
	CString m_sNameFilter;

	HTREEITEM AddTreeItem(const char *title, int image, bool sort, HTREEITEM hParent = TVI_ROOT, LPARAM lParam = NULL);

public:
	CSelectPluginDlg(CModDoc *pModDoc, int nPlugSlot, CWnd *parent); //rewbs.plugDocAware
	~CSelectPluginDlg();
	void DoClose();
	void UpdatePluginsList(VstInt32 forceSelect = 0);
	bool VerifyPlug(VSTPluginLib *plug);
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddPlugin();
	afx_msg void OnRemovePlugin();
	afx_msg void OnNameFilterChanged();
	afx_msg void OnSelChanged(NMHDR *pNotifyStruct, LRESULT *result);
	afx_msg void OnSelDblClk(NMHDR *pNotifyStruct, LRESULT *result);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};

#endif // SELECTPLUGINDIALOG_H
