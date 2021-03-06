/*
 * Reporting.cpp
 * -------------
 * Purpose: A class for showing notifications, prompts, etc...
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "Stdafx.h"
#include "Reporting.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/Mainfrm.h"
#else
#include <iostream>
#endif // MODPLUG_TRACKER


UINT Reporting::ShowNotification(const char *text, const char *caption, UINT flags, const CWnd *parent)
//-----------------------------------------------------------------------------------------------------
{
#ifdef MODPLUG_TRACKER
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	if(pMainFrm != nullptr && pMainFrm->GetInputHandler() != nullptr)
	{
		pMainFrm->GetInputHandler()->Bypass(true);
	}

	if(parent == nullptr)
	{
		parent = pMainFrm;
	}
	UINT result = ::MessageBox((parent ? parent->m_hWnd : NULL), text, caption, flags);

	if(pMainFrm != nullptr && pMainFrm->GetInputHandler() != nullptr)
	{
		pMainFrm->GetInputHandler()->Bypass(false);
	}

	return result;
#else
	std::cout << caption << std::endl << text << std::endl;
	return 0;
#endif // MODPLUG_TRACKER
}


void Reporting::Notification(const char *text, const CWnd *parent)
//----------------------------------------------------------------
{
	Notification(text, MAINFRAME_TITLE, parent);
}


void Reporting::Notification(const char *text, const char *caption, const CWnd *parent)
//-------------------------------------------------------------------------------------
{
	ShowNotification(text, caption, MB_OK, parent);
}


void Reporting::Information(const char *text, const CWnd *parent)
//---------------------------------------------------------------
{
	Information(text, MAINFRAME_TITLE, parent);
}


void Reporting::Information(const char *text, const char *caption, const CWnd *parent)
//------------------------------------------------------------------------------------
{
	ShowNotification(text, caption, MB_OK | MB_ICONINFORMATION, parent);
}


void Reporting::Warning(const char *text, const CWnd *parent)
//-----------------------------------------------------------
{
	Warning(text, MAINFRAME_TITLE " - Warning", parent);
}


void Reporting::Warning(const char *text, const char *caption, const CWnd *parent)
//--------------------------------------------------------------------------------
{
	ShowNotification(text, caption, MB_OK | MB_ICONWARNING, parent);
}


void Reporting::Error(const char *text, const CWnd *parent)
//---------------------------------------------------------
{
	Error(text, MAINFRAME_TITLE " - Error", parent);
}


void Reporting::Error(const char *text, const char *caption, const CWnd *parent)
//------------------------------------------------------------------------------
{
	ShowNotification(text, caption, MB_OK | MB_ICONERROR, parent);
}


ConfirmAnswer Reporting::Confirm(const char *text, bool showCancel, bool defaultNo, const CWnd *parent)
//-----------------------------------------------------------------------------------------------------
{
	return Confirm(text, MAINFRAME_TITLE " - Confirmation", showCancel, defaultNo, parent);
}


ConfirmAnswer Reporting::Confirm(const char *text, const char *caption, bool showCancel, bool defaultNo, const CWnd *parent)
//--------------------------------------------------------------------------------------------------------------------------
{
	UINT result = ShowNotification(text, caption, (showCancel ? MB_YESNOCANCEL : MB_YESNO) | MB_ICONQUESTION | (defaultNo ? MB_DEFBUTTON2 : 0), parent);
	switch(result)
	{
	case IDYES:
		return cnfYes;
	case IDNO:
		return cnfNo;
	default:
	case IDCANCEL:
		return cnfCancel;
	}
}


RetryAnswer Reporting::RetryCancel(const char *text, const CWnd *parent)
//----------------------------------------------------------------------
{
	return RetryCancel(text, MAINFRAME_TITLE, parent);
}


RetryAnswer Reporting::RetryCancel(const char *text, const char *caption, const CWnd *parent)
//-------------------------------------------------------------------------------------------
{
	UINT result = ShowNotification(text, caption, MB_RETRYCANCEL, parent);
	switch(result)
	{
	case IDRETRY:
		return rtyRetry;
	default:
	case IDCANCEL:
		return rtyCancel;
	}
}
