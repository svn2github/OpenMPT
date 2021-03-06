/*
 * CheckBox.h
 * ----------
 * Purpose: Wrapper class for a Win32 check box.
 * Notes  : (currently none)
 * Authors: Johannes Schultz (OpenMPT Devs)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "WindowBase.h"


OPENMPT_NAMESPACE_BEGIN


//================================
class CheckBox : public WindowBase
//================================
{ 
public:
	// Create a new check box.
	void Create(HWND parent, const TCHAR *text, int x, int y, int width, int height)
	{
		// Remove old instance, if necessary
		Destroy();

		hwnd = CreateWindow(_T("BUTTON"),
			text,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_AUTOCHECKBOX,
			ScaleX(parent, x),
			ScaleY(parent, y),
			ScaleX(parent, width),
			ScaleY(parent, height),
			parent,
			NULL,
			NULL,
			0);

		SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
	}


	// Select an item.
	void SetState(bool state)
	{
		if(hwnd != nullptr)
		{
			Button_SetCheck(hwnd, state ? BST_CHECKED : BST_UNCHECKED);
		}
	}


	// Get the currently selected item.
	bool GetState() const
	{
		if(hwnd != nullptr)
		{
			return Button_GetCheck(hwnd) != BST_UNCHECKED;
		}
		return false;
	}

};


OPENMPT_NAMESPACE_END
