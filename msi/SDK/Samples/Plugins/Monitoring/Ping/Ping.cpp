// Ping.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
#include <intrin.h>
#include <io.h>
 
#include "Ping.h"
#include "PingGlobals.h"
#include "PingThread.h"
#include "PingSetupDlg.h"
#include "MAHMSharedMemory.h"
#include "MSIAfterburnerMonitoringSourceDesc.h"
#include "MSIAfterburnerExports.h"
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE PingDLL = { NULL, NULL };
//////////////////////////////////////////////////////////////////////
HINSTANCE					g_hModule						= 0;
BOOL						g_bEnableLog					= FALSE;
char						g_szAddr[MAX_PATH]				= { 0 };
DWORD						g_dwHeaderBgndColor				= 0;
DWORD						g_dwHeaderTextColor				= 0;

GET_HOST_APP_PROPERTY_PROC	g_pGetHostAppProperty			= NULL;
LOCALIZEWND_PROC			g_pLocalizeWnd					= NULL;
LOCALIZESTR_PROC			g_pLocalizeStr					= NULL;
CPingThread*				g_pThread						= NULL;
//////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(PingDLL, hInstance))
			return 0;

		new CDynLinkLibrary(PingDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(PingDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to shift child window down/right relative
// to the parent window position
/////////////////////////////////////////////////////////////////////////////
void AdjustWindowPos(CWnd* pWnd, CWnd* pParent)
{
	CRect wndRect; pWnd->GetWindowRect(&wndRect);

	if (pParent)
	{
		CRect parentRect; pParent->GetWindowRect(&parentRect);

		wndRect = CRect(parentRect.left, parentRect.top, parentRect.left + wndRect.Width(), parentRect.top + wndRect.Height());
	}

	wndRect.OffsetRect(20, 20);
	
	HMONITOR hMonitor = MonitorFromPoint(wndRect.TopLeft(), MONITOR_DEFAULTTONULL);

	MONITORINFO mi; mi.cbSize = sizeof(MONITORINFO);

	CRect rcWork;

	SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID) &rcWork, 0);

	if (GetMonitorInfo(hMonitor, &mi))
		rcWork = mi.rcWork;
	else
		SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID) &rcWork, 0);

	if (wndRect.bottom > rcWork.bottom)
		wndRect.OffsetRect(0, rcWork.bottom - wndRect.bottom);

	if (wndRect.right > rcWork.right)
		wndRect.OffsetRect(rcWork.right - wndRect.right, 0);

	pWnd->MoveWindow(&wndRect);
}
/////////////////////////////////////////////////////////////////////////////
// This helper funtrion is used to localize string if host application 
// supports localization
/////////////////////////////////////////////////////////////////////////////
LPCSTR LocalizeStr(LPCSTR lpStr)
{
	if (g_pLocalizeStr)
		return g_pLocalizeStr(lpStr);

	return lpStr;
}
/////////////////////////////////////////////////////////////////////////////
// This helper funtrion is used to localize window if host application 
// supports localization
/////////////////////////////////////////////////////////////////////////////
void LocalizeWnd(HWND hWnd)
{
	if (g_pLocalizeWnd)
		g_pLocalizeWnd(hWnd);
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to check if a source
// provides any additional settings and for customizing it when <Setup>
// button is clicked in the source's properties 
//
// The function is called with hWnd = NULL during checking and with parent
// window's hWnd during customization
//
// The function is called with valid source index to customize specific
// source or 0xFFFFFFFF to customize whole plugin 
/////////////////////////////////////////////////////////////////////////////
PING_API BOOL SetupSource(DWORD dwIndex, HWND hWnd)
{
	if (hWnd) 
	{
		//get host application module handle

		HMODULE hHost = GetModuleHandle(NULL);

		//get ptrs to required plugin API functions

		g_pGetHostAppProperty	= (GET_HOST_APP_PROPERTY_PROC)GetProcAddress(hHost, "GetHostAppProperty");
		g_pLocalizeWnd			= (LOCALIZEWND_PROC)GetProcAddress(hHost, "LocalizeWnd");
		g_pLocalizeStr			= (LOCALIZESTR_PROC)GetProcAddress(hHost, "LocalizeStr");

		//initialize settings

		CString strCfgPath		= GetCfgPath();
		CString strCfgPathUser	= strCfgPath + ".user";

		GetPrivateProfileString("Settings", "Addr", "", g_szAddr, sizeof(g_szAddr), !_taccess(strCfgPathUser, 0) ? strCfgPathUser : strCfgPath);

		//get skin color scheme

		if (g_pGetHostAppProperty)
		{
			g_pGetHostAppProperty(HOST_APP_PROPERTY_SKIN_COLOR_HEADER_BGND, &g_dwHeaderBgndColor, sizeof(DWORD));
			g_pGetHostAppProperty(HOST_APP_PROPERTY_SKIN_COLOR_HEADER_TEXT, &g_dwHeaderTextColor, sizeof(DWORD));
		}

		CPingSetupDlg dlg;
		dlg.SetAddr(g_szAddr);

		if (IDOK == dlg.DoModal()) 
		{
			strcpy_s(g_szAddr, sizeof(g_szAddr), dlg.GetAddr());

			CString strCfgPath		= GetCfgPath();
			CString strCfgPathUser	= strCfgPath + ".user";

			WritePrivateProfileString("Settings", "Addr", g_szAddr, strCfgPathUser);

			if (g_pThread)
				g_pThread->SetAddr(g_szAddr);

			return TRUE;
		}

		return FALSE;
	}

	//hWnd is NULL, simply return TRUE as all the sources exported by our plugin
	//have additional properties

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get a number of
// data sources in this plugin
//////////////////////////////////////////////////////////////////////
PING_API DWORD GetSourcesNum()
{
	return 1;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get descriptor
// for the specified data source
//////////////////////////////////////////////////////////////////////
PING_API BOOL GetSourceDesc(DWORD dwIndex, LPMONITORING_SOURCE_DESC pDesc)
{
	strcpy_s(pDesc->szName	, sizeof(pDesc->szName)			, "Ping");
	strcpy_s(pDesc->szGroup	, sizeof(pDesc->szGroup)		, "PING");
	strcpy_s(pDesc->szUnits	, sizeof(pDesc->szUnits)		, "ms");

	pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_NET;
	pDesc->dwInstance	= dwIndex - 1;
	pDesc->fltMaxLimit	= 200.0f;
	pDesc->fltMinLimit	= 0.0f;

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to poll data sources
/////////////////////////////////////////////////////////////////////////////
PING_API FLOAT GetSourceData(DWORD dwIndex)
{
	if (!g_pThread)
	{
		CString strCfgPath		= GetCfgPath();
		CString strCfgPathUser	= strCfgPath + ".user";

		g_bEnableLog = GetPrivateProfileInt("Debug", "EnableLog", 0, strCfgPath);

		GetPrivateProfileString("Settings", "Addr", "", g_szAddr, sizeof(g_szAddr), !_taccess(strCfgPathUser, 0) ? strCfgPathUser : strCfgPath);

		g_pThread = new CPingThread(g_szAddr);

		g_pThread->CreateThread(CREATE_SUSPENDED);
		g_pThread->SetThreadPriority(THREAD_PRIORITY_NORMAL);
		g_pThread->ResumeThread();
	}

	g_pThread->UpdatePingAsync();

	return g_pThread->GetPing();
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner before unloading the
// plugin to let it to uninitialize itself properly
/////////////////////////////////////////////////////////////////////////////
PING_API void Uninit()
{
	if (g_pThread)
		g_pThread->Kill();
	g_pThread = NULL;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to return fully qualified path to .cfg file
/////////////////////////////////////////////////////////////////////////////
CString GetCfgPath()
{
	char szCfgPath[MAX_PATH];
	GetModuleFileName(g_hModule, szCfgPath, MAX_PATH);
	
	PathRenameExtension(szCfgPath, ".cfg");

	return szCfgPath;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to return fully qualified path to .log file
/////////////////////////////////////////////////////////////////////////////
CString GetLogPath()
{
	char szLogPath[MAX_PATH];
	GetModuleFileName(g_hModule, szLogPath, MAX_PATH);
	
	PathRenameExtension(szLogPath, ".log");

	return szLogPath;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function adds specified line to log file
/////////////////////////////////////////////////////////////////////////////
void AppendLog(LPCSTR lpLine, BOOL bAppend)
{
	if (g_bEnableLog)
	{
		CString strLogPath = GetLogPath();

		CStdioFile logFile;

		if (bAppend)
		{
			if (!logFile.Open(strLogPath, CFile::modeWrite|CFile::shareDenyWrite))
				if (!logFile.Open(strLogPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite))
					return;
		}
		else
			if (!logFile.Open(strLogPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite))
				return;

		logFile.SeekToEnd();


		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		CString strTime; strTime.Format("%.2d-%.2d-%.2d, %.2d:%.2d:%.2d ", sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

		logFile.WriteString(strTime);
		logFile.WriteString(lpLine);
		logFile.WriteString("\n");

		logFile.Close();
		logFile.Flush();
	}
}
/////////////////////////////////////////////////////////////////////////////

