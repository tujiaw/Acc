#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <string>

#include <winver.h>
typedef std::basic_string <TCHAR> stlString;
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

	stlString GetCompanyName() const;
	stlString GetFileDescription() const;
	stlString GetFileVersion() const;
	stlString GetInternalName() const;
	stlString GetLegalCopyright() const;
	stlString GetOriginalFileName() const;
	stlString GetProductName() const;
	stlString GetProductVersion() const;
	stlString GetComments() const;
	stlString GetLegalTrademarks() const;
	stlString GetPrivateBuild() const;
	stlString GetSpecialBuild() const;

	// implementation helpers
protected:
	virtual void Reset();
	BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);

	// attributes
private:
	VS_FIXEDFILEINFO m_FileInfo;

	stlString m_strCompanyName;
	stlString m_strFileDescription;
	stlString m_strFileVersion;
	stlString m_strInternalName;
	stlString m_strLegalCopyright;
	stlString m_strOriginalFileName;
	stlString m_strProductName;
	stlString m_strProductVersion;
	stlString m_strComments;
	stlString m_strLegalTrademarks;
	stlString m_strPrivateBuild;
	stlString m_strSpecialBuild;
};