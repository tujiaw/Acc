#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <winver.h>

class CFileVersionInfo
{

	// construction/destruction
public:
	CFileVersionInfo();
	virtual ~CFileVersionInfo();

	// operations
public:
	BOOL Create(HMODULE hModule = NULL);
	BOOL Create(LPCTSTR lpszFileName);

	// attribute operations
public:
	WORD GetFileVersion(int nIndex) const;
	WORD GetProductVersion(int nIndex) const;
	DWORD GetFileFlagsMask() const;
	DWORD GetFileFlags() const;
	DWORD GetFileOs() const;
	DWORD GetFileType() const;
	DWORD GetFileSubtype() const;

	std::wstring GetCompanyName() const;
	std::wstring GetFileDescription() const;
	std::wstring GetFileVersion() const;
	std::wstring GetInternalName() const;
	std::wstring GetLegalCopyright() const;
	std::wstring GetOriginalFileName() const;
	std::wstring GetProductName() const;
	std::wstring GetProductVersion() const;
	std::wstring GetComments() const;
	std::wstring GetLegalTrademarks() const;
	std::wstring GetPrivateBuild() const;
	std::wstring GetSpecialBuild() const;

	// implementation helpers
protected:
	virtual void Reset();
	BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);

	// attributes
private:
	VS_FIXEDFILEINFO m_FileInfo;

	std::wstring m_strCompanyName;
	std::wstring m_strFileDescription;
	std::wstring m_strFileVersion;
	std::wstring m_strInternalName;
	std::wstring m_strLegalCopyright;
	std::wstring m_strOriginalFileName;
	std::wstring m_strProductName;
	std::wstring m_strProductVersion;
	std::wstring m_strComments;
	std::wstring m_strLegalTrademarks;
	std::wstring m_strPrivateBuild;
	std::wstring m_strSpecialBuild;
};