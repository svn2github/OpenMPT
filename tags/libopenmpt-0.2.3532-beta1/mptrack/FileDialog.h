/*
 * FileDialog.h
 * ------------
 * Purpose: File and folder selection dialogs implementation.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include <string>
#include <vector>

// Generic open / save file dialog. Cannot be instanced by the user, use OpenFileDialog / SaveFileDialog instead.
class FileDialog
{
public:
	typedef std::vector<mpt::PathString> PathList;

protected:
	std::wstring defaultExtension;
	std::wstring defaultFilename;
	std::wstring extFilter;
	mpt::PathString workingDirectory;
	mpt::PathString lastPreviewFile;
	mpt::PathString extension;
	PathList filenames;
	int *filterIndex;
	bool load;
	bool multiSelect;

protected:
	FileDialog(bool load) : filterIndex(nullptr), load(load), multiSelect(false) { }

public:
	// Default extension to use if none is specified.
	FileDialog &DefaultExtension(const mpt::PathString &ext) { defaultExtension = ext.ToWide(); return *this; }
	FileDialog &DefaultExtension(const std::wstring &ext) { defaultExtension = ext; return *this; }
	FileDialog &DefaultExtension(const std::string &ext) { defaultExtension = mpt::ToWide(mpt::CharsetLocale, ext); return *this; }
	// Default suggested filename.
	FileDialog &DefaultFilename(const mpt::PathString &name) { defaultFilename = name.ToWide(); return *this; }
	FileDialog &DefaultFilename(const std::wstring &name) { defaultFilename = name; return *this; }
	FileDialog &DefaultFilename(const std::string &name) { defaultFilename = mpt::ToWide(mpt::CharsetLocale, name); return *this; }
	// List of possible extensions. Format: "description|extensions|...|description|extensions||"
	FileDialog &ExtensionFilter(const mpt::PathString &filter) { extFilter = filter.ToWide(); return *this; }
	FileDialog &ExtensionFilter(const std::wstring &filter) { extFilter = filter; return *this; }
	FileDialog &ExtensionFilter(const std::string &filter) { extFilter = mpt::ToWide(mpt::CharsetLocale, filter); return *this; }
	// Default directory of the dialog.
	FileDialog &WorkingDirectory(const mpt::PathString &dir) { workingDirectory = dir; return *this; }
	// Pointer to a variable holding the index of the last extension filter to use. Holds the selected filter after the dialog has been closed.
	FileDialog &FilterIndex(int *index) { filterIndex = index; return *this; }

	// Show the file selection dialog.
	bool Show();

	// Get some selected file. Mostly useful when only one selected file is possible anyway.
	mpt::PathString GetFirstFile() const
	{
		if(!filenames.empty())
		{
			return filenames.front();
		} else
		{
			return mpt::PathString();
		}
	}
	// Gets a reference to all selected filenames.
	const PathList &GetFilenames() const { return filenames; }
	// Gets directory in which the selected files are placed.
	mpt::PathString GetWorkingDirectory() const { return workingDirectory; }
	// Gets the extension of the first selected file, without dot.
	mpt::PathString GetExtension() const { return extension; }

protected:
	static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
};


// Dialog for opening files
class OpenFileDialog : public FileDialog
{
public:
	OpenFileDialog() : FileDialog(true) { }

	// Enable selection of multiple files
	OpenFileDialog &AllowMultiSelect() { multiSelect = true; return *this; }
};


// Dialog for saving files
class SaveFileDialog : public FileDialog
{
public:
	SaveFileDialog() : FileDialog(false) { }
};


class BrowseForFolder
{
protected:
	mpt::PathString workingDirectory;
	std::wstring caption;

public:
	BrowseForFolder(const mpt::PathString &dir, const CString &caption) : workingDirectory(dir), caption(mpt::ToWide(caption)) { }

	// Show the folder selection dialog.
	bool Show();

	// Gets selected directory.
	mpt::PathString GetDirectory() const { return workingDirectory; }

protected:
	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};
