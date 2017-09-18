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

std::wstring CFileVersionInfo::GetCompanyName() const
{
	return m_strCompanyName;
}


std::wstring CFileVersionInfo::GetFileDescription() const
{
	return m_strFileDescription;
}


std::wstring CFileVersionInfo::GetFileVersion() const
{
	return m_strFileVersion;
}


std::wstring CFileVersionInfo::GetInternalName() const
{
	return m_strInternalName;
}


std::wstring CFileVersionInfo::GetLegalCopyright() const
{
	return m_strLegalCopyright;
}


std::wstring CFileVersionInfo::GetOriginalFileName() const
{
	return m_strOriginalFileName;
}


std::wstring CFileVersionInfo::GetProductName() const
{
	return m_strProductName;
}


std::wstring CFileVersionInfo::GetProductVersion() const
{
	return m_strProductVersion;
}


std::wstring CFileVersionInfo::GetComments() const
{
	return m_strComments;
}


std::wstring CFileVersionInfo::GetLegalTrademarks() const
{
	return m_strLegalTrademarks;
}


std::wstring CFileVersionInfo::GetPrivateBuild() const
{
	return m_strPrivateBuild;
}


std::wstring CFileVersionInfo::GetSpecialBuild() const
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


		std::wstring	strSubBlock;
		TCHAR buf[1024];
		wsprintf(buf, L"\\StringFileInfo\\%04X%04X\\", dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);
		strSubBlock = buf;


		// catch string table
		std::wstring sBuf;

		sBuf = strSubBlock;
		sBuf += L"CompanyName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strCompanyName = std::wstring((LPCTSTR)lpInfo);

		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"FileDescription";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strFileDescription = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"FileVersion";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strFileVersion = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"InternalName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strInternalName = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"LegalCopyright";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strLegalCopyright = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"OriginalFileName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strOriginalFileName = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"ProductName";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strProductName = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"ProductVersion";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strProductVersion = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"Comments";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strComments = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"LegalTrademarks";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strLegalTrademarks = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"PrivateBuild";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strPrivateBuild = std::wstring((LPCTSTR)lpInfo);


		sBuf.clear();
		sBuf = strSubBlock;
		sBuf += L"SpecialBuild";
		if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
			m_strSpecialBuild = std::wstring((LPCTSTR)lpInfo);

		delete[] lpData;
	}
	catch (BOOL)
	{
		delete[] lpData;
		return FALSE;
	}

	return TRUE;
}