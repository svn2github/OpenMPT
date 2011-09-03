/*
 *
 * Reporting.cpp
 * -------------
 * Purpose: A class for showing notifications, prompts, etc...
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 */


#include "Stdafx.h"
#include "Reporting.h"
#include "../mptrack/Mainfrm.h"

UINT Reporting::Notification(CString text, UINT flags /* = MB_OK*/, HWND parent /* = NULL*/)
//------------------------------------------------------------------------------------------
{
	return Notification(text, MAINFRAME_TITLE, flags, parent);
};


UINT Reporting::Notification(CString text, CString caption, UINT flags /* = MB_OK*/, HWND parent /* = NULL*/)
//-----------------------------------------------------------------------------------------------------------
{
	if(parent == NULL && CMainFrame::GetMainFrame() != nullptr)
	{
		parent = CMainFrame::GetMainFrame()->m_hWnd;
	}

	return ::MessageBox(parent, text, caption, flags);
};
