/*
 * mptPathString.cpp
 * -----------------
 * Purpose: Wrapper class around the platform-native representation of path names. Should be the only type that is used to store path names.
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "mptPathString.h"

#include "misc_util.h"

#if MPT_OS_WINDOWS
#include <windows.h>
#if defined(MODPLUG_TRACKER)
#include <shlwapi.h>
#endif
#endif


OPENMPT_NAMESPACE_BEGIN


#if MPT_OS_WINDOWS
#define MPT_PATHSTRING_LITERAL(x) ( L ## x )
#else
#define MPT_PATHSTRING_LITERAL(x) ( x )
#endif


#if MPT_OS_WINDOWS

namespace mpt
{


RawPathString PathString::AsNativePrefixed() const
//------------------------------------------------
{
	if(path.length() <= MAX_PATH || path.substr(0, 4) == L"\\\\?\\")
	{
		// Path is short enough or already in prefixed form
		return path;
	}
	const RawPathString absPath = mpt::GetAbsolutePath(path).AsNative();
	if(absPath.substr(0, 2) == L"\\\\")
	{
		// Path is a network share: \\server\foo.bar -> \\?\UNC\server\foo.bar
		return L"\\\\?\\UNC" + absPath.substr(1);
	} else
	{
		// Regular file: C:\foo.bar -> \\?\C:\foo.bar
		return L"\\\\?\\" + absPath;
	}
}


int PathString::CompareNoCase(const PathString & a, const PathString & b)
//-----------------------------------------------------------------------
{
	return lstrcmpiW(a.ToWide().c_str(), b.ToWide().c_str());
}

} // namespace mpt

#endif // MPT_OS_WINDOWS


namespace mpt
{


#if MPT_OS_WINDOWS && defined(MPT_ENABLE_DYNBIND)

void PathString::SplitPath(PathString *drive, PathString *dir, PathString *fname, PathString *ext) const
//------------------------------------------------------------------------------------------------------
{
	// We cannot use CRT splitpath here, because:
	//  * limited to _MAX_PATH or similar
	//  * no support for UNC paths
	//  * no support for \\?\ prefixed paths

	if(drive) *drive = mpt::PathString();
	if(dir) *dir = mpt::PathString();
	if(fname) *fname = mpt::PathString();
	if(ext) *ext = mpt::PathString();

	mpt::RawPathString p = path;

	// remove \\?\\ prefix
	if(p.substr(0, 8) == MPT_PATHSTRING_LITERAL("\\\\?\\UNC\\"))
	{
		p = MPT_PATHSTRING_LITERAL("\\\\") + p.substr(8);
	} else if(p.substr(0, 4) == MPT_PATHSTRING_LITERAL("\\\\?\\"))
	{
		p = p.substr(4);
	}

	if (p.length() >= 2 && (
		p.substr(0, 2) == MPT_PATHSTRING_LITERAL("\\\\")
		|| p.substr(0, 2) == MPT_PATHSTRING_LITERAL("\\/")
		|| p.substr(0, 2) == MPT_PATHSTRING_LITERAL("/\\")
		|| p.substr(0, 2) == MPT_PATHSTRING_LITERAL("//")
		))
	{ // UNC
		mpt::RawPathString::size_type first_slash = p.substr(2).find_first_of(MPT_PATHSTRING_LITERAL("\\/"));
		if(first_slash != mpt::RawPathString::npos)
		{
			mpt::RawPathString::size_type second_slash = p.substr(2 + first_slash + 1).find_first_of(MPT_PATHSTRING_LITERAL("\\/"));
			if(second_slash != mpt::RawPathString::npos)
			{
				if(drive) *drive = mpt::PathString::FromNative(p.substr(0, 2 + first_slash + 1 + second_slash));
				p = p.substr(2 + first_slash + 1 + second_slash);
			} else
			{
				if(drive) *drive = mpt::PathString::FromNative(p);
				p = mpt::RawPathString();
			}
		} else
		{
			if(drive) *drive = mpt::PathString::FromNative(p);
			p = mpt::RawPathString();
		}
	} else
	{ // local
		if(p.length() >= 2 && (p[1] == MPT_PATHSTRING_LITERAL(':')))
		{
			if(drive) *drive = mpt::PathString::FromNative(p.substr(0, 2));
			p = p.substr(2);
		} else
		{
			if(drive) *drive = mpt::PathString();
		}
	}
	mpt::RawPathString::size_type last_slash = p.find_last_of(MPT_PATHSTRING_LITERAL("\\/"));
	if(last_slash != mpt::RawPathString::npos)
	{
		if(dir) *dir = mpt::PathString::FromNative(p.substr(0, last_slash + 1));
		p = p.substr(last_slash + 1);
	} else
	{
		if(dir) *dir = mpt::PathString();
	}
	mpt::RawPathString::size_type last_dot = p.find_last_of(MPT_PATHSTRING_LITERAL("."));
	if(last_dot == mpt::RawPathString::npos)
	{
		if(fname) *fname = mpt::PathString::FromNative(p);
		if(ext) *ext = mpt::PathString();
	} else
	{
		if(fname) *fname = mpt::PathString::FromNative(p.substr(0, last_dot));
		if(ext) *ext = mpt::PathString::FromNative(p.substr(last_dot));
	}

}

PathString PathString::GetDrive() const
//-------------------------------------
{
	PathString drive;
	SplitPath(&drive, nullptr, nullptr, nullptr);
	return drive;
}
PathString PathString::GetDir() const
//-----------------------------------
{
	PathString dir;
	SplitPath(nullptr, &dir, nullptr, nullptr);
	return dir;
}
PathString PathString::GetPath() const
//------------------------------------
{
	PathString drive, dir;
	SplitPath(&drive, &dir, nullptr, nullptr);
	return drive + dir;
}
PathString PathString::GetFileName() const
//----------------------------------------
{
	PathString fname;
	SplitPath(nullptr, nullptr, &fname, nullptr);
	return fname;
}
PathString PathString::GetFileExt() const
//---------------------------------------
{
	PathString ext;
	SplitPath(nullptr, nullptr, nullptr, &ext);
	return ext;
}
PathString PathString::GetFullFileName() const
//--------------------------------------------
{
	PathString name, ext;
	SplitPath(nullptr, nullptr, &name, &ext);
	return name + ext;
}


bool PathString::IsDirectory() const
//----------------------------------
{
	// Using PathIsDirectoryW here instead would increase libopenmpt dependencies by shlwapi.dll.
	// GetFileAttributesW also does the job just fine.
	DWORD dwAttrib = ::GetFileAttributesW(path.c_str());
	return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool PathString::IsFile() const
//-----------------------------
{
	DWORD dwAttrib = ::GetFileAttributesW(path.c_str());
	return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

#endif // MPT_OS_WINDOWS && MPT_ENABLE_DYNBIND


#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS

bool PathString::FileOrDirectoryExists() const
//--------------------------------------------
{
	return ::PathFileExistsW(path.c_str()) != FALSE;
}

#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS


#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS

PathString PathString::ReplaceExt(const mpt::PathString &newExt) const
//--------------------------------------------------------------------
{
	return GetDrive() + GetDir() + GetFileName() + newExt;
}


PathString PathString::SanitizeComponent() const
//----------------------------------------------
{
	PathString result = *this;
	SanitizeFilename(result);
	return result;
}


// Convert an absolute path to a path that's relative to "&relativeTo".
PathString PathString::AbsolutePathToRelative(const PathString &relativeTo) const
//-------------------------------------------------------------------------------
{
	mpt::PathString result = path;
	if(path.empty())
	{
		return result;
	}
	if(!_wcsnicmp(relativeTo.AsNative().c_str(), AsNative().c_str(), relativeTo.AsNative().length()))
	{
		// Path is OpenMPT's directory or a sub directory ("C:\OpenMPT\Somepath" => ".\Somepath")
		result = MPT_PATHSTRING(".\\"); // ".\"
		result += mpt::PathString::FromNative(AsNative().substr(relativeTo.AsNative().length()));
	} else if(!_wcsnicmp(relativeTo.AsNative().c_str(), AsNative().c_str(), 2))
	{
		// Path is on the same drive as OpenMPT ("C:\Somepath" => "\Somepath")
		result = mpt::PathString::FromNative(AsNative().substr(2));
	}
	return result;
}


// Convert a path that is relative to "&relativeTo" to an absolute path.
PathString PathString::RelativePathToAbsolute(const PathString &relativeTo) const
//-------------------------------------------------------------------------------
{
	mpt::PathString result = path;
	if(path.empty())
	{
		return result;
	}
	if(AsNative().length() >= 2 && AsNative().at(0) == L'\\' && AsNative().at(1) != L'\\')
	{
		// Path is on the same drive as OpenMPT ("\Somepath\" => "C:\Somepath\"), but ignore network paths starting with "\\"
		result = mpt::PathString::FromNative(relativeTo.AsNative().substr(0, 2));
		result += path;
	} else if(AsNative().length() >= 2 && AsNative().substr(0, 2) == L".\\")
	{
		// Path is OpenMPT's directory or a sub directory (".\Somepath\" => "C:\OpenMPT\Somepath\")
		result = relativeTo; // "C:\OpenMPT\"
		result += mpt::PathString::FromNative(AsNative().substr(2));
	}
	return result;
}


#if defined(_MFC_VER)

mpt::PathString PathString::TunnelOutofCString(const CString &path)
//-----------------------------------------------------------------
{
	#ifdef UNICODE
		return mpt::PathString::FromWide(path.GetString());
	#else
		// Since MFC code can call into our code from a lot of places, we cannot assume
		// that filenames we get from MFC are always encoded in our hacked UTF8-in-CString encoding.
		// Instead, we use a rough heuristic: if the string is parseable as UTF8, we assume it is.
		// This fails for CP_ACP strings, that are also valid UTF8. That's the trade-off here.
		if(mpt::IsUTF8(path.GetString()))
		{
			// utf8
			return mpt::PathString::FromUTF8(path.GetString());
		} else
		{
			// ANSI
			return mpt::PathString::FromWide(mpt::ToWide(path));
		}
	#endif
}


CString PathString::TunnelIntoCString(const mpt::PathString &path)
//----------------------------------------------------------------
{
	#ifdef UNICODE
		return path.ToWide().c_str();
	#else
		return path.ToUTF8().c_str();
	#endif
}

#endif // MFC

#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS

} // namespace mpt



namespace mpt
{


bool IsPathSeparator(mpt::RawPathString::value_type c) {
#if MPT_OS_WINDOWS
	return (c == MPT_PATHSTRING_LITERAL('\\')) || (c == MPT_PATHSTRING_LITERAL('/'));
#else
	return c == MPT_PATHSTRING_LITERAL('/');
#endif
}

bool PathIsAbsolute(const mpt::PathString &path) {
	mpt::RawPathString rawpath = path.AsNative();
#if MPT_OS_WINDOWS
	if(rawpath.substr(0, 8) == MPT_PATHSTRING_LITERAL("\\\\?\\UNC\\"))
	{
		return true;
	}
	if(rawpath.substr(0, 4) == MPT_PATHSTRING_LITERAL("\\\\?\\"))
	{
		return true;
	}
	if(rawpath.substr(0, 2) == MPT_PATHSTRING_LITERAL("\\\\"))
	{
		return true; // UNC
	}
	if(rawpath.substr(0, 2) == MPT_PATHSTRING_LITERAL("//"))
	{
		return true; // UNC
	}
	return (rawpath.length()) >= 3 && (rawpath[1] == ':') && IsPathSeparator(rawpath[2]);
#else
	return (rawpath.length() >= 1) && IsPathSeparator(rawpath[0]);
#endif
}


#if MPT_OS_WINDOWS

mpt::PathString GetAbsolutePath(const mpt::PathString &path)
//----------------------------------------------------------
{
	DWORD size = GetFullPathNameW(path.AsNative().c_str(), 0, nullptr, nullptr);
	if(size == 0)
	{
		return path;
	}
	std::vector<WCHAR> fullPathName(size, L'\0');
	if(GetFullPathNameW(path.AsNative().c_str(), size, &fullPathName[0], nullptr) == 0)
	{
		return path;
	}
	return mpt::PathString::FromNative(&fullPathName[0]);
}

#ifdef MODPLUG_TRACKER

bool DeleteWholeDirectoryTree(mpt::PathString path)
{
	if(path.AsNative().empty())
	{
		return false;
	}
	if(!path.FileOrDirectoryExists())
	{
		return true;
	}
	if(!path.IsDirectory())
	{
		return false;
	}
	path.EnsureTrailingSlash();
	HANDLE hFind = NULL;
	WIN32_FIND_DATAW wfd;
	MemsetZero(wfd);
	hFind = FindFirstFileW((path + MPT_PATHSTRING("*.*")).AsNative().c_str(), &wfd);
	if(hFind != NULL && hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			mpt::PathString filename = mpt::PathString::FromNative(wfd.cFileName);
			if(filename != MPT_PATHSTRING(".") && filename != MPT_PATHSTRING(".."))
			{
				filename = path + filename;
				if(filename.IsDirectory())
				{
					if(!DeleteWholeDirectoryTree(filename))
					{
						return false;
					}
				} else if(filename.IsFile())
				{
					if(DeleteFileW(filename.AsNative().c_str()) == 0)
					{
						return false;
					}
				}
			}
		} while(FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}
	if(RemoveDirectoryW(path.AsNative().c_str()) == 0)
	{
		return false;
	}
	return true;
}

#endif // MODPLUG_TRACKER

#endif // MPT_OS_WINDOWS

#if defined(MPT_ENABLE_TEMPFILE)
#if MPT_OS_WINDOWS

mpt::PathString GetTempDirectory()
//--------------------------------
{
	DWORD size = GetTempPathW(0, nullptr);
	if(size)
	{
		std::vector<WCHAR> tempPath(size + 1);
		if(GetTempPathW(size + 1, &tempPath[0]))
		{
			return mpt::PathString::FromNative(&tempPath[0]);
		}
	}
	// use app directory as fallback
	return mpt::GetAppPath();
}

mpt::PathString CreateTempFileName(const mpt::PathString &fileNamePrefix, const mpt::PathString &fileNameExtension)
//-----------------------------------------------------------------------------------------------------------------
{
	mpt::PathString filename = mpt::GetTempDirectory();
	filename += (!fileNamePrefix.empty() ? fileNamePrefix + MPT_PATHSTRING("_") : mpt::PathString());
	filename += mpt::PathString::FromUnicode(mpt::UUID::GenerateLocalUseOnly().ToUString());
	filename += (!fileNameExtension.empty() ? MPT_PATHSTRING(".") + fileNameExtension : mpt::PathString());
	return filename;
}

TempFileGuard::TempFileGuard(const mpt::PathString &filename)
	: filename(filename)
{
	return;
}

mpt::PathString TempFileGuard::GetFilename() const
{
	return filename;
}

TempFileGuard::~TempFileGuard()
{
	if(!filename.empty())
	{
		DeleteFileW(filename.AsNative().c_str());
	}
}

#ifdef MODPLUG_TRACKER

TempDirGuard::TempDirGuard(const mpt::PathString &dirname_)
	: dirname(dirname_.WithTrailingSlash())
{
	if(dirname.empty())
	{
		return;
	}
	if(::CreateDirectoryW(dirname.AsNative().c_str(), NULL) == 0)
	{ // fail
		dirname = mpt::PathString();
	}
}

mpt::PathString TempDirGuard::GetDirname() const
{
	return dirname;
}

TempDirGuard::~TempDirGuard()
{
	if(!dirname.empty())
	{
		DeleteWholeDirectoryTree(dirname);
	}
}

#endif // MODPLUG_TRACKER

#endif // MPT_OS_WINDOWS
#endif // MPT_ENABLE_TEMPFILE

} // namespace mpt



#if defined(MODPLUG_TRACKER)

static inline char SanitizeFilenameChar(char c)
//---------------------------------------------
{
	if(	c == '\\' ||
		c == '\"' ||
		c == '/'  ||
		c == ':'  ||
		c == '?'  ||
		c == '<'  ||
		c == '>'  ||
		c == '|'  ||
		c == '*')
	{
		c = '_';
	}
	return c;
}

static inline wchar_t SanitizeFilenameChar(wchar_t c)
//---------------------------------------------------
{
	if(	c == L'\\' ||
		c == L'\"' ||
		c == L'/'  ||
		c == L':'  ||
		c == L'?'  ||
		c == L'<'  ||
		c == L'>'  ||
		c == L'|'  ||
		c == L'*')
	{
		c = L'_';
	}
	return c;
}

void SanitizeFilename(mpt::PathString &filename)
//----------------------------------------------
{
	mpt::RawPathString tmp = filename.AsNative();
	for(mpt::RawPathString::iterator it = tmp.begin(); it != tmp.end(); ++it)
	{
		*it = SanitizeFilenameChar(*it);
	}
	filename = mpt::PathString::FromNative(tmp);
}

void SanitizeFilename(char *beg, char *end)
//-----------------------------------------
{
	for(char *it = beg; it != end; ++it)
	{
		*it = SanitizeFilenameChar(*it);
	}
}

void SanitizeFilename(wchar_t *beg, wchar_t *end)
//-----------------------------------------------
{
	for(wchar_t *it = beg; it != end; ++it)
	{
		*it = SanitizeFilenameChar(*it);
	}
}

void SanitizeFilename(std::string &str)
//-------------------------------------
{
	for(size_t i = 0; i < str.length(); i++)
	{
		str[i] = SanitizeFilenameChar(str[i]);
	}
}

void SanitizeFilename(std::wstring &str)
//--------------------------------------
{
	for(size_t i = 0; i < str.length(); i++)
	{
		str[i] = SanitizeFilenameChar(str[i]);
	}
}

#if defined(_MFC_VER)
void SanitizeFilename(CString &str)
//---------------------------------
{
	for(int i = 0; i < str.GetLength(); i++)
	{
		str.SetAt(i, SanitizeFilenameChar(str.GetAt(i)));
	}
}
#endif // MFC

#endif // MODPLUG_TRACKER


#if defined(MODPLUG_TRACKER)


mpt::PathString FileType::AsFilterString(FlagSet<FileTypeFormat> format) const
//----------------------------------------------------------------------------
{
	mpt::PathString filter;
	if(GetShortName().empty() || GetExtensions().empty())
	{
		return filter;
	}
	if(!GetDescription().empty())
	{
		filter += mpt::PathString::FromUnicode(GetDescription());
	} else
	{
		filter += mpt::PathString::FromUnicode(GetShortName());
	}
	const std::vector<mpt::PathString> extensions = GetExtensions();
	if(format[FileTypeFormatShowExtensions])
	{
		filter += MPT_PATHSTRING(" (");
		bool first = true;
		for(std::vector<mpt::PathString>::const_iterator it = extensions.begin(); it != extensions.end(); ++it)
		{
			if(first)
			{
				first = false;
			} else
			{
				filter += MPT_PATHSTRING(",");
			}
			filter += MPT_PATHSTRING("*.");
			filter += (*it);
		}
		filter += MPT_PATHSTRING(")");
	}
	filter += MPT_PATHSTRING("|");
	{
		bool first = true;
		for(std::vector<mpt::PathString>::const_iterator it = extensions.begin(); it != extensions.end(); ++it)
		{
			if(first)
			{
				first = false;
			} else
			{
				filter += MPT_PATHSTRING(";");
			}
			filter += MPT_PATHSTRING("*.");
			filter += (*it);
		}
	}
	filter += MPT_PATHSTRING("|");
	return filter;
}


mpt::PathString FileType::AsFilterOnlyString() const
//--------------------------------------------------
{
	mpt::PathString filter;
	const std::vector<mpt::PathString> extensions = GetExtensions();
	{
		bool first = true;
		for(std::vector<mpt::PathString>::const_iterator it = extensions.begin(); it != extensions.end(); ++it)
		{
			if(first)
			{
				first = false;
			} else
			{
				filter += MPT_PATHSTRING(";");
			}
			filter += MPT_PATHSTRING("*.");
			filter += (*it);
		}
	}
	return filter;
}


mpt::PathString ToFilterString(const FileType &fileType, FlagSet<FileTypeFormat> format)
//--------------------------------------------------------------------------------------
{
	return fileType.AsFilterString(format);
}


mpt::PathString ToFilterString(const std::vector<FileType> &fileTypes, FlagSet<FileTypeFormat> format)
//----------------------------------------------------------------------------------------------------
{
	mpt::PathString filter;
	for(std::vector<FileType>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it)
	{
		filter += it->AsFilterString(format);
	}
	return filter;
}


mpt::PathString ToFilterOnlyString(const FileType &fileType, bool prependSemicolonWhenNotEmpty)
//---------------------------------------------------------------------------------------------
{
	mpt::PathString filter = fileType.AsFilterOnlyString();
	return filter.empty() ? filter : (prependSemicolonWhenNotEmpty ? MPT_PATHSTRING(";") : MPT_PATHSTRING("")) + filter;
}


mpt::PathString ToFilterOnlyString(const std::vector<FileType> &fileTypes, bool prependSemicolonWhenNotEmpty)
//-----------------------------------------------------------------------------------------------------------
{
	mpt::PathString filter;
	for(std::vector<FileType>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it)
	{
		filter += it->AsFilterOnlyString();
	}
	return filter.empty() ? filter : (prependSemicolonWhenNotEmpty ? MPT_PATHSTRING(";") : MPT_PATHSTRING("")) + filter;
}


#endif // MODPLUG_TRACKER


OPENMPT_NAMESPACE_END
