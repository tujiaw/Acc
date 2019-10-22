#include "ForegroundWindowGuard.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QDebug>
ForegroundWindowGuard::ForegroundWindowGuard()
{
	m_lockTimeOut = 0;
	HWND  hCurrWnd = ::GetForegroundWindow();
	unsigned long dwThisTID = ::GetCurrentThreadId();
	unsigned long dwCurrTID = ::GetWindowThreadProcessId(hCurrWnd,0);

	//we need to bypass some limitations from Microsoft :)
	if(dwThisTID != dwCurrTID) {
		::AttachThreadInput(dwThisTID, dwCurrTID, TRUE);
		::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&m_lockTimeOut,0);
		::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,0,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
		::AllowSetForegroundWindow(ASFW_ANY);
	}
}

ForegroundWindowGuard::~ForegroundWindowGuard()
{
	HWND  hCurrWnd = ::GetForegroundWindow();
	unsigned long dwThisTID = ::GetCurrentThreadId();
	unsigned long dwCurrTID = ::GetWindowThreadProcessId(hCurrWnd,0);
	if(dwThisTID != dwCurrTID) {
		::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,(PVOID)m_lockTimeOut,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
		::AttachThreadInput(dwThisTID, dwCurrTID, FALSE);
	}
}
