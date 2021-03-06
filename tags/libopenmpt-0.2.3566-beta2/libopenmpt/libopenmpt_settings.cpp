/*
 * libopenmpt_settings.cpp
 * -----------------------
 * Purpose: libopenmpt plugin settings
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "libopenmpt_settings.h"
#include "SettingsForm.h"

extern "C" {

void libopenmpt_settings_edit( libopenmpt_settings * s, HWND parent, const char * title ) {
	try {
		libopenmpt::SettingsForm ^ form = gcnew libopenmpt::SettingsForm( title, s );
		bool topmost = false;
		if ( parent && parent != INVALID_HANDLE_VALUE ) {
			topmost = ( GetWindowLong( parent, GWL_EXSTYLE ) & WS_EX_TOPMOST ) ? true : false;
		}
		System::Windows::Forms::IWin32Window ^w = System::Windows::Forms::Control::FromHandle((System::IntPtr)parent);
		if ( topmost ) {
			form->TopMost = true;
		}
		form->ShowDialog(w);
	} catch ( ... ) {
		// nothing
	}
}

#pragma comment(linker, "/EXPORT:libopenmpt_settings_edit=_libopenmpt_settings_edit")

} // extern "C"
