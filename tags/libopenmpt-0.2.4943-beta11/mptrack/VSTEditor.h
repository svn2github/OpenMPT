/*
 * VSTEditor.h
 * -----------
 * Purpose: Implementation of the custom plugin editor window that is used if a plugin provides an own editor GUI.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "AbstractVstEditor.h"

OPENMPT_NAMESPACE_BEGIN

#ifndef NO_VST

//==============================================
class COwnerVstEditor: public CAbstractVstEditor
//==============================================
{
protected:
	CStatic plugWindow;

public:
	COwnerVstEditor(CVstPlugin &plugin);
	virtual ~COwnerVstEditor();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC *) { return TRUE; }
	afx_msg void OnPaint();

	// Plugins may request to change the GUI size.
	virtual bool IsResizable() const { return true; };
	virtual bool SetSize(int contentWidth, int contentHeight);

	//Overridden:
	virtual void UpdateParamDisplays() { CAbstractVstEditor::UpdateParamDisplays(); m_VstPlugin.Dispatch(effEditIdle, 0, 0, nullptr, 0.0f); };	//we trust that the plugin GUI can update its display with a bit of idle time.
	afx_msg void OnClose();
	bool OpenEditor(CWnd *parent);
	void DoClose();
};

#endif // NO_VST

OPENMPT_NAMESPACE_END
