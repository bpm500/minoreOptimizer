// AIDA64.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
 
#include "AIDA64.h"
#include "AIDA64DataSources.h"
#include "AIDA64SetupDlg.h"
#include "AIDA64SetupSourceDlg.h"
#include "AIDA64Globals.h"
#include "MAHMSharedMemory.h"
#include "MSIAfterburnerMonitoringSourceDesc.h"
#include "MSIAfterburnerExports.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE AIDA64DLL = { NULL, NULL };
/////////////////////////////////////////////////////////////////////////////
GET_HOST_APP_PROPERTY_PROC	g_pGetHostAppProperty			= NULL;
LOCALIZEWND_PROC			g_pLocalizeWnd					= NULL;
LOCALIZESTR_PROC			g_pLocalizeStr					= NULL;

HINSTANCE					g_hModule						= 0;
DWORD						g_dwSpawn						= 0;
DWORD						g_dwSpawnPeriod					= 0;
DWORD						g_dwHeaderBgndColor				= 0x700000;
DWORD						g_dwHeaderTextColor				= 0xFFFFFF;

CAIDA64DataSources			g_sources;
/////////////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(AIDA64DLL, hInstance))
			return 0;

		new CDynLinkLibrary(AIDA64DLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(AIDA64DLL);
	}
	return 1;
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
AIDA64_API BOOL SetupSource(DWORD dwIndex, HWND hWnd)
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

		g_dwSpawn				= GetPrivateProfileInt("Settings", "Spawn"				, 0, strCfgPath);
		g_dwSpawnPeriod			= GetPrivateProfileInt("Settings", "SpawnPeriod"		, 0, strCfgPath);

		//get skin color scheme

		if (g_pGetHostAppProperty)
		{
			g_pGetHostAppProperty(HOST_APP_PROPERTY_SKIN_COLOR_HEADER_BGND, &g_dwHeaderBgndColor, sizeof(DWORD));
			g_pGetHostAppProperty(HOST_APP_PROPERTY_SKIN_COLOR_HEADER_TEXT, &g_dwHeaderTextColor, sizeof(DWORD));
		}

		if (dwIndex == 0xFFFFFFFF)
			//global plugin setup
		{
			CAIDA64SetupDlg dlg;

			if (IDOK == dlg.DoModal()) 
				//reload global sources list
			{
				g_sources.Init(GetCfgPath());

				return TRUE;
			}
		}
		else
			//specific data source setup
		{
			POSITION pos = g_sources.FindIndex(dwIndex);

			if (pos)
			{
				LPAIDA64_DATA_SOURCE_DESC lpDesc = g_sources.GetAt(pos);

				CAIDA64SetupSourceDlg dlg;
				dlg.SetSourceDesc(lpDesc);

				if (IDOK == dlg.DoModal())
				{
					g_sources.Save(GetCfgPath());

					return TRUE;
				}
			}
		}

		return FALSE;
	}

	//hWnd is NULL, simply return TRUE as all the sources exported by our plugin
	//have additional properties

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get a number of
// data sources in this plugin
/////////////////////////////////////////////////////////////////////////////
AIDA64_API DWORD GetSourcesNum()
{
	if (!g_sources.GetCount())
		//we'll init sources list if it is not initialized yet
		g_sources.Init(GetCfgPath());

	return g_sources.GetCount();
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get descriptor
// for the specified data source
/////////////////////////////////////////////////////////////////////////////
AIDA64_API BOOL GetSourceDesc(DWORD dwIndex, LPMONITORING_SOURCE_DESC pDesc)
{
	if (!g_sources.GetCount())
		//we'll init sources list if it is not initialized yet
	{
		CString strCfgPath = GetCfgPath();

		//initialize sources list

		g_sources.Init(strCfgPath);

		//initialize settings

		g_dwSpawn			= GetPrivateProfileInt("Settings", "Spawn"				, 0, strCfgPath);
		g_dwSpawnPeriod		= GetPrivateProfileInt("Settings", "SpawnPeriod"		, 0, strCfgPath);
	}

	if (dwIndex < (DWORD)g_sources.GetCount())
	{
		LPAIDA64_DATA_SOURCE_DESC pAIDA64Desc = g_sources.GetSourceDesc(dwIndex);
			//try to get AIDA64 source by index

		if (pAIDA64Desc)
		{
			//init monitoring source descriptor by AIDA64 monitoring source descriptor

			CString strName, strNameTemplate;
			FormatName(pAIDA64Desc->szName, pAIDA64Desc->dwInstance, strName, strNameTemplate);

			CString strGroup, strGroupTemplate;
			FormatName(pAIDA64Desc->szGroup, pAIDA64Desc->dwInstance, strGroup, strGroupTemplate);
	
			strcpy_s(pDesc->szName	, sizeof(pDesc->szName)		, strName);
			strcpy_s(pDesc->szUnits	, sizeof(pDesc->szUnits)	, pAIDA64Desc->szUnits);
			strcpy_s(pDesc->szGroup	, sizeof(pDesc->szGroup)	, strGroup);
			strcpy_s(pDesc->szFormat, sizeof(pDesc->szFormat)	, pAIDA64Desc->szFormat);

			pDesc->dwID				= pAIDA64Desc->dwID;
			pDesc->dwInstance		= pAIDA64Desc->dwInstance;

			pDesc->fltMaxLimit		= pAIDA64Desc->fltMaxLimit;
			pDesc->fltMinLimit		= pAIDA64Desc->fltMinLimit;
			
			strcpy_s(pDesc->szNameTemplate	, sizeof(pDesc->szNameTemplate)		, strNameTemplate);
			strcpy_s(pDesc->szGroupTemplate	, sizeof(pDesc->szGroupTemplate)	, strGroupTemplate);

			return TRUE;
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to poll data sources
/////////////////////////////////////////////////////////////////////////////
AIDA64_API FLOAT GetSourceData(DWORD dwIndex)
{
	return g_sources.GetSourceData(dwIndex);
}
/////////////////////////////////////////////////////////////////////////////
// This helper function adds specified line to log file
/////////////////////////////////////////////////////////////////////////////
void AppendLog(LPCSTR lpLine, BOOL bAppend)
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
/////////////////////////////////////////////////////////////////////////////
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
// This helper function is used to format fully qualified data source name
// from the template (e.g. "GPU%d usage") and zero based data source instance
// index
/////////////////////////////////////////////////////////////////////////////
void FormatName(LPCSTR lpSrcTmpl, DWORD dwSrcInst, CString& strDstName, CString& strDstTmpl)
{
	if (strstr(lpSrcTmpl, "%d"))
		//template contains instance index format specifier, so it is valid template and we need
		//to format a name 
	{
		if (dwSrcInst != 0xFFFFFFFF)
			//instance index is valid, so we can format the name now
		{
			strDstName.Format(lpSrcTmpl, dwSrcInst + 1);
				//insance indices are zero based, but we use 1-based indexes in GUI
			strDstTmpl = lpSrcTmpl;
		}
		else
			//instance index is invalid, so we strip instance index format specifier from the name
		{
			strDstName = lpSrcTmpl;
			strDstName.Replace("%d", "");
			strDstTmpl = "";
		}
	}
	else
		//template doesn't contain instance index format specifier, so return it as a name as is
	{
		strDstName = lpSrcTmpl;
		strDstTmpl = "";
	}
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
