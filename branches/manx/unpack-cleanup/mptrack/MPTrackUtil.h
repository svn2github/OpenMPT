/*
 * MPTrackUtil.h
 * -------------
 * Purpose: Various useful utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once


#include <string>


LPCCH LoadResource(LPCTSTR lpName, LPCTSTR lpType, LPCCH& pData, size_t& nSize, HGLOBAL& hglob);

std::string GetErrorMessage(DWORD nErrorCode);

namespace Util { namespace sdOs
{
	/// Checks whether file or folder exists and whether it has the given mode.
	enum FileMode {FileModeExists = 0, FileModeRead = 4, FileModeWrite = 2, FileModeReadWrite = 6};
	bool IsPathFileAvailable(const mpt::PathString &pszFilePath, FileMode fm);

}} // namespace Util::sdOs

