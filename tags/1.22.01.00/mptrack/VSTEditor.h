/*
 * VSTEditor.h
 * -----------
 * Purpose: Implementation of the custom plugin editor window that is used if a plugin provides an own editor GUI.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "mptrack.h"
#include "MainFrm.h"
#include "VstPlug.h"
#include "AbstractVstEditor.h"

#ifndef NO_VST

//==============================
class COwnerVstEditor: public CAbstractVstEditor
//==============================
{
protected:
/*	CVstPlugin *m_pVstPlugin;
	CMenu *m_pMenu;
	CMenu *m_pPresetMenu;
*/

public:
	COwnerVstEditor(CVstPlugin *pPlugin);
	virtual ~COwnerVstEditor();
	virtual void OnOK();
	virtual void OnCancel();

	// Plugins may request to change the GUI size.
	virtual bool IsResizable() const { return true; };
	virtual bool SetSize(int contentWidth, int contentHeight);

	//Overridden:
	void UpdateParamDisplays() { m_pVstPlugin->Dispatch(effEditIdle, 0,0, NULL, 0); };	//we trust that the plugin GUI can update its display with a bit of idle time.
	afx_msg void OnClose();
	bool OpenEditor(CWnd *parent);
	void DoClose();
};

#endif // NO_VST
