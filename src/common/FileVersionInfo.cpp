#include "FileVersionInfo.h"
#include <tchar.h>

#pragma comment (lib, "version.lib")	


CFileVersionInfo::CFileVersionInfo()
{
	Reset();
}


CFileVersionInfo::~CFileVersionInfo()
{}

BOOL CFileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
	LPWORD lpwData;
	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
	{
		if (*lpwData == wLangId)
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	if (!bPrimaryEnough)
		return FALSE;

	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
	{
		if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF))
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	return FALSE;
}

WORD CFileVersionInfo::GetFileVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


WORD CFileVersionInfo::GetProductVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


DWORD CFileVersionInfo::GetFileFlagsMask() const
{
	return m_FileInfo.dwFileFlagsMask;
}


DWORD CFileVersionInfo::GetFileFlags() const
{
	return m_FileInfo.dwFileFlags;
}


DWORD CFileVersionInfo::GetFileOs() const
{
	return m_FileInfo.dwFileOS;
}


DWORD CFileVersionInfo::GetFileType() const
{
	return m_FileInfo.dwFileType;
}


DWORD CFileVersionInfo::GetFileSubtype() const
{
	return m_FileInfo.dwFileSubtype;
}

stlString CFileVersionInfo::GetCompanyName() const
{
	return m_strCompanyName;
}


stlString CFileVersionInfo::GetFileDescription() const
{
	return m_strFileDescription;
}


stlString CFileVersionInfo::GetFileVersion() const
{
	return m_strFileVersion;
}


stlString CFileVersionInfo::GetInternalName() const
{
	return m_strInternalName;
}


stlString CFileVersionInfo::GetLegalCopyright() const
{
	return m_strLegalCopyright;
}


stlString CFileVersionInfo::GetOriginalFileName() const
{
	return m_strOriginalFileName;
}


stlString CFileVersionInfo::GetProductName() const
{
	return m_strProductName;
}


stlString CFileVersionInfo::GetProductVersion() const
{
	return m_strProductVersion;
}


stlString CFileVersionInfo::GetComments() const
{
	return m_strComments;
}


stlString CFileVersionInfo::GetLegalTrademarks() const
{
	return m_strLegalTrademarks;
}


stlString CFileVersionInfo::GetPrivateBuild() const
{
	return m_strPrivateBuild;
}


stlString CFileVersionInfo::GetSpecialBuild() const
{
	return m_strSpecialBuild;
}


void CFileVersionInfo::Reset()
{
	ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
	m_strCompanyName.clear();
	m_strFileDescription.clear();
	m_strFileVersion.clear();
	m_strInternalName.clear();
	m_strLegalCopyright.clear();
	m_strOriginalFileName.clear();
	m_strProductName.clear();
	m_strProductVersion.clear();
	m_strComments.clear();
	m_strLegalTrademarks.clear();
	m_strPrivateBuild.clear();
	m_strSpecialBuild.clear();
}

BOOL CFileVersionInfo::Create(LPCTSTR lpszFileName)
{
	Reset();

	DWORD	dwHandle;
	DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
	if (!dwFileVersionInfoSize)
		return FALSE;

	LPVOID	lpData = (LPVOID)new BYTE[dwFileVersionInfoSize];
	if (!lpData)
		return FALSE;

	try
	{
		if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, lpData))
			throw FALSE;

		// catch default information
		LPVOID	lpInfo;
		UINT		unInfoLen;
		if (VerQueryValue(lpData, L"\\", &lpInfo, &unInfoLen))
		{
			//ASSERT(unInfoLen == sizeof(m_FileInfo));
			if (unInfoLen == sizeof(m_FileInfo))
				memcpy(&m_FileInfo, lpInfo, unInfoLen);
		}

		// find best matching language and codepage
		VerQueryValue(lpData, L"\\VarFileInfo\\Translation", &lpInfo, &unInfoLen);

		DWORD	dwLangCode = 0;
		if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
		{
			if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
			{
				if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
				{
					if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
						// use the first one we can get
						dwLangCode = *((DWORD*)lpInfo);
				}
			}
		}


		stlString	strSubBlock;
		TCHAR buf[1024];
		wsprintf(buf, L"\\StringFileInfo\\%04X%04X\\", dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);
		strSubBlock = buf;


		// catch string table
		stlString sBuf;

		sBuf = strSubBlock;
		sBuf += L"CompanyName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strCompanyName = stlString((LPCTSTR)lpInfo);

		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"FileDescription";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strFileDescription = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"FileVersion";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strFileVersion = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"InternalName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strInternalName = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"LegalCopyright";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strLegalCopyright = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"OriginalFileName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strOriginalFileName = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"ProductName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strProductName = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"ProductVersion";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strProductVersion = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"Comments";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strComments = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"LegalTrademarks";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strLegalTrademarks = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"PrivateBuild";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strPrivateBuild = stlString((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"SpecialBuild";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strSpecialBuild = stlString((LPCTSTR)lpInfo);

		delete[] lpData;
	}
	catch (BOOL)
	{
		delete[] lpData;
		return FALSE;
	}

	return TRUE;
}