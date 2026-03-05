// SMART.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
 
#include "SMART.h"
#include "SMARTImp.h"
#include "MAHMSharedMemory.h"
#include "MSIAfterburnerMonitoringSourceDesc.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE SMARTDLL = { NULL, NULL };
/////////////////////////////////////////////////////////////////////////////
HINSTANCE					g_hModule						= 0;
BOOL						g_bInitialized					= FALSE;
CSMART						g_smart;
//////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(SMARTDLL, hInstance))
			return 0;

		new CDynLinkLibrary(SMARTDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(SMARTDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get a number of
// data sources in this plugin
//////////////////////////////////////////////////////////////////////
SMART_API DWORD GetSourcesNum()
{
	return MAX_DRIVE * MAX_SENSOR;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get descriptor
// for the specified data source
//////////////////////////////////////////////////////////////////////
SMART_API BOOL GetSourceDesc(DWORD dwIndex, LPMONITORING_SOURCE_DESC pDesc)
{
	//init global variables, which will be used for HDD temperature monitoring

	if (!g_bInitialized)
		//init SMART polling implementation, if we've not initialized it yet
	{
		g_smart.Init();

		//get fully qualified path to .cfg
		char szCfgPath[MAX_PATH];
		GetModuleFileName(g_hModule, szCfgPath, MAX_PATH);
		
		PathRenameExtension(szCfgPath, ".cfg");

		//load plugin settings
		g_smart.SetPollingInterval(GetPrivateProfileInt("Settings", "MinPollingInterval", 0, szCfgPath));

		g_bInitialized = TRUE;
	}

	DWORD dwDrive	= dwIndex / MAX_SENSOR;
	DWORD dwSensor	= dwIndex % MAX_SENSOR;

	if (dwDrive < MAX_DRIVE)
	{
		DWORD dwCaps = g_smart.GetCaps(dwDrive);

		if (dwSensor)
		{
			if ((dwCaps & SMART_CAPS_TEMPERATURE_REPORTING_NVME_2) == 0)
				return FALSE;

			sprintf_s(pDesc->szName		, "HDD%d temperature 2"	, dwDrive + 1);
			sprintf_s(pDesc->szGroup	, "HDD%d"				, dwDrive + 1);

			strcpy_s(pDesc->szNameTemplate	, sizeof(pDesc->szNameTemplate), "HDD%d temperature 2");
			strcpy_s(pDesc->szGroupTemplate	, sizeof(pDesc->szNameTemplate), "HDD%d");
		}
		else
		{
			if ((dwCaps & (SMART_CAPS_TEMPERATURE_REPORTING_IDE | SMART_CAPS_TEMPERATURE_REPORTING_NVME)) == 0)
				return FALSE;

			sprintf_s(pDesc->szName		, "HDD%d temperature"	, dwDrive + 1);
			sprintf_s(pDesc->szGroup	, "HDD%d"				, dwDrive + 1);

			strcpy_s(pDesc->szNameTemplate	, sizeof(pDesc->szNameTemplate), "HDD%d temperature");
			strcpy_s(pDesc->szGroupTemplate	, sizeof(pDesc->szNameTemplate), "HDD%d");
		}

		strcpy_s(pDesc->szUnits			, sizeof(pDesc->szUnits), "°C");

		pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_HDD;
		pDesc->dwInstance	= dwDrive;

		pDesc->fltMaxLimit	= 100.0f;
		pDesc->fltMinLimit	= 0.0f;

		return TRUE;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to poll data sources
/////////////////////////////////////////////////////////////////////////////
SMART_API FLOAT GetSourceData(DWORD dwIndex)
{
	DWORD dwDrive	= dwIndex / MAX_SENSOR;
	DWORD dwSensor	= dwIndex % MAX_SENSOR;

	return g_smart.GetTemperature(dwDrive, dwSensor);
}
/////////////////////////////////////////////////////////////////////////////
