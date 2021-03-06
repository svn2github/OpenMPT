#include "stdafx.h"
#include "misc_util.h"

/*
 * Loads resource.
 * [in] lpName and lpType: parameters passed to FindResource().
 * [out] pData: Pointer to loaded resource data, nullptr if load not successful.
 * [out] nSize: Size of the data in bytes, zero if load not succesfull.
 * [out] hglob: HGLOBAL returned by LoadResource-function.
 * Return: pData.
 */
LPCCH LoadResource(LPCTSTR lpName, LPCTSTR lpType, LPCCH& pData, size_t& nSize, HGLOBAL& hglob)
//---------------------------------------------------------------------------------------------
{
	pData = nullptr;
	nSize = 0;
	hglob = nullptr;
	HINSTANCE hInstance = AfxGetInstanceHandle();
	HRSRC hrsrc = FindResource(hInstance, lpName, lpType); 
	if (hrsrc != NULL)
	{
		hglob = LoadResource(hInstance, hrsrc);
		if (hglob != NULL)
		{
			pData = reinterpret_cast<const char*>(LockResource(hglob));
			nSize = SizeofResource(hInstance, hrsrc);
		}
	}
	return pData;
}


// Returns error message corresponding to error code returned by GetLastError().
CString GetErrorMessage(DWORD nErrorCode)
//---------------------------------------
{
	const size_t nBufferSize = 256;
	CString sMsg;
	LPTSTR pszBuf = sMsg.GetBuffer(nBufferSize);

    FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					nErrorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					pszBuf,
					nBufferSize,
					NULL );

	sMsg.ReleaseBuffer();

	return sMsg;
}

