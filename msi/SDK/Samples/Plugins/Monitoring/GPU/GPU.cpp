// GPU.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
 
#include "GPU.h"
#include "GPUList.h"
#include "GPUGlobals.h"
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
static AFX_EXTENSION_MODULE GPUDLL = { NULL, NULL };
//////////////////////////////////////////////////////////////////////
GET_GPU_NUM_PROC			g_pGetGpuNum					= NULL;
GET_GPU_ID_PROC				g_pGetGpuID						= NULL;

HINSTANCE					g_hModule						= 0;

CGPUList					g_gpus;
CList<DWORD,DWORD>			g_sources;
//////////////////////////////////////////////////////////////////////
#define GPU_USAGE_TYPE										0
#define MEM_USAGE_DEDICATED_TYPE							1
#define MEM_USAGE_SHARED_TYPE								2
#define PROCESS_MEM_USAGE_DEDICATED_TYPE					3
#define PROCESS_MEM_USAGE_SHARED_TYPE						4

#define GPU_SHIFT											0
#define GPU_MASK											0xFF
#define TYPE_SHIFT											8
#define TYPE_MASK											0xFF
#define NODE_SHIFT											16
#define NODE_MASK											0xFF
//////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(GPUDLL, hInstance))
			return 0;

		new CDynLinkLibrary(GPUDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(GPUDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
// This helper funciton is used to return number of GPUs visible to
// host
//////////////////////////////////////////////////////////////////////
DWORD GetGpuNum()
{
	if (g_pGetGpuNum)
		return g_pGetGpuNum();

	return 0;
}
//////////////////////////////////////////////////////////////////////
// This helper funtion is used to return GPU identifier for each GPU
// visible to host
//////////////////////////////////////////////////////////////////////
LPCSTR GetGpuID(DWORD gpu)
{
	if (g_pGetGpuID)
		return g_pGetGpuID(gpu);

	return "";
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get a number of
// data sources in this plugin
//////////////////////////////////////////////////////////////////////
GPU_API DWORD GetSourcesNum()
{
	if (!g_sources.GetCount())
	{
		//get host application module handle

		HMODULE hHost = GetModuleHandle(NULL);

		//get ptrs to required plugin API functions

		g_pGetGpuNum			= (GET_GPU_NUM_PROC				)GetProcAddress(hHost, "GetGpuNum"			);
		g_pGetGpuID				= (GET_GPU_ID_PROC				)GetProcAddress(hHost, "GetGpuID"			);

		//init GPU list

		g_gpus.Init();

		//init sources list, per-node GPU usage + dedicated memory usage + shared memory usage are available for each GPU

		POSITION pos = g_gpus.GetHeadPosition();

		while (pos)
		{
			LPGPU_DESC lpDesc = g_gpus.GetNext(pos);

			DWORD dwNodeCount = lpDesc->dwNodeCount;

			for (DWORD dwNode=0; dwNode<dwNodeCount; dwNode++)
				g_sources.AddTail(lpDesc->dwGpu + (GPU_USAGE_TYPE<<TYPE_SHIFT) + (dwNode<<NODE_SHIFT));

			g_sources.AddTail(lpDesc->dwGpu + (MEM_USAGE_DEDICATED_TYPE			<< TYPE_SHIFT));
			g_sources.AddTail(lpDesc->dwGpu + (MEM_USAGE_SHARED_TYPE			<< TYPE_SHIFT));
			g_sources.AddTail(lpDesc->dwGpu + (PROCESS_MEM_USAGE_DEDICATED_TYPE	<< TYPE_SHIFT));
			g_sources.AddTail(lpDesc->dwGpu + (PROCESS_MEM_USAGE_SHARED_TYPE	<< TYPE_SHIFT));
		}
	}

	return g_sources.GetCount();
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get descriptor
// for the specified data source
//////////////////////////////////////////////////////////////////////
GPU_API BOOL GetSourceDesc(DWORD dwIndex, LPMONITORING_SOURCE_DESC pDesc)
{
	if (!g_sources.GetCount())
	{
		//get host application module handle

		HMODULE hHost = GetModuleHandle(NULL);

		//get ptrs to required plugin API functions

		g_pGetGpuNum			= (GET_GPU_NUM_PROC				)GetProcAddress(hHost, "GetGpuNum"			);
		g_pGetGpuID				= (GET_GPU_ID_PROC				)GetProcAddress(hHost, "GetGpuID"			);

		//init GPU list

		g_gpus.Init();

		//init sources list, per-node GPU usage + dedicated memory usage + shared memory usage are available for each GPU

		POSITION pos = g_gpus.GetHeadPosition();

		while (pos)
		{
			LPGPU_DESC lpDesc = g_gpus.GetNext(pos);

			DWORD dwNodeCount = lpDesc->dwNodeCount;

			for (DWORD dwNode=0; dwNode<dwNodeCount; dwNode++)
				g_sources.AddTail(lpDesc->dwGpu + (GPU_USAGE_TYPE<<TYPE_SHIFT) + (dwNode<<NODE_SHIFT));

			g_sources.AddTail(lpDesc->dwGpu + (MEM_USAGE_DEDICATED_TYPE			<< TYPE_SHIFT));
			g_sources.AddTail(lpDesc->dwGpu + (MEM_USAGE_SHARED_TYPE			<< TYPE_SHIFT));
			g_sources.AddTail(lpDesc->dwGpu + (PROCESS_MEM_USAGE_DEDICATED_TYPE	<< TYPE_SHIFT));
			g_sources.AddTail(lpDesc->dwGpu + (PROCESS_MEM_USAGE_SHARED_TYPE	<< TYPE_SHIFT));
		}
	}

	POSITION pos = g_sources.FindIndex(dwIndex);

	if (pos)
	{
		DWORD dwSource = g_sources.GetAt(pos);

		DWORD dwGpu		= (dwSource>>GPU_SHIFT	) & GPU_MASK;
		DWORD dwType	= (dwSource>>TYPE_SHIFT	) & TYPE_MASK;
		DWORD dwNode	= (dwSource>>NODE_SHIFT	) & NODE_MASK;

		DWORD dwGpuNum	= GetGpuNum();

		switch (dwType)
		{
		case GPU_USAGE_TYPE:
			if (dwGpuNum > 1)
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU%d engine %d usage", dwGpu + 1, dwNode);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "GPU%d", dwGpu + 1);

				sprintf_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU%%d engine %d usage", dwNode);
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "GPU%d");
			}
			else
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU engine %d usage", dwNode);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "GPU");

				sprintf_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU engine %d usage", dwNode);
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "GPU");
			}

			strcpy_s(pDesc->szUnits				, sizeof(pDesc->szUnits)			, "%");

			pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_GPU;
			pDesc->dwInstance	= dwGpu;

			pDesc->fltMaxLimit	= 100.0f;
			pDesc->fltMinLimit	= 0.0f;

			return TRUE;

		case MEM_USAGE_DEDICATED_TYPE:
			if (dwGpuNum > 1)
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU%d dedicated memory usage", dwGpu + 1);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM%d", dwGpu + 1);

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU%d dedicated memory usage");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM%d");
			}
			else
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU dedicated memory usage");
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM");

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU dedicated memory usage");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM");
			}

			strcpy_s(pDesc->szUnits				, sizeof(pDesc->szUnits)			, "MB");

			pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_GPU;
			pDesc->dwInstance	= dwGpu;

			pDesc->fltMaxLimit	= 8192.0f;
			pDesc->fltMinLimit	= 0.0f;

			return TRUE;

		case MEM_USAGE_SHARED_TYPE:
			if (dwGpuNum > 1)
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU%d shared memory usage", dwGpu + 1);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM%d", dwGpu + 1);

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU%d shared memory usage");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM%d");
			}
			else
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU shared memory usage");
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM");

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU shared memory usage");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM");
			}

			strcpy_s(pDesc->szUnits				, sizeof(pDesc->szUnits)			, "MB");

			pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_GPU;
			pDesc->dwInstance	= dwGpu;

			pDesc->fltMaxLimit	= 8192.0f;
			pDesc->fltMinLimit	= 0.0f;

			return TRUE;

		case PROCESS_MEM_USAGE_DEDICATED_TYPE:
			if (dwGpuNum > 1)
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU%d dedicated memory usage \\ process", dwGpu + 1);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM%d", dwGpu + 1);

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU%d dedicated memory usage \\ process");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM%d");
			}
			else
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU dedicated memory usage \\ process");
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM");

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU dedicated memory usage \\ process");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM");
			}

			strcpy_s(pDesc->szUnits				, sizeof(pDesc->szUnits)			, "MB");

			pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_GPU;
			pDesc->dwInstance	= dwGpu;

			pDesc->fltMaxLimit	= 8192.0f;
			pDesc->fltMinLimit	= 0.0f;

			return TRUE;

		case PROCESS_MEM_USAGE_SHARED_TYPE:
			if (dwGpuNum > 1)
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU%d shared memory usage \\ process", dwGpu + 1);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM%d", dwGpu + 1);

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU%d shared memory usage \\ process");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM%d");
			}
			else
			{
				sprintf_s(pDesc->szName				, sizeof(pDesc->szName)				, "GPU shared memory usage \\ process", dwGpu + 1);
				sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)			, "MEM");

				strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)		, "GPU shared memory usage \\ process");
				strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate)	, "MEM");
			}

			strcpy_s(pDesc->szUnits				, sizeof(pDesc->szUnits)			, "MB");

			pDesc->dwID			= MONITORING_SOURCE_ID_PLUGIN_GPU;
			pDesc->dwInstance	= dwGpu;

			pDesc->fltMaxLimit	= 8192.0f;
			pDesc->fltMinLimit	= 0.0f;

			return TRUE;
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This helper funtion is used to return previously sampled per-node GPU or
// memory usage and invalidate sampled value
/////////////////////////////////////////////////////////////////////////////
FLOAT GetSourceData(DWORD dwGpu, DWORD dwType, DWORD dwNode)
{
	FLOAT result = FLT_MAX;

	POSITION pos = g_gpus.GetHeadPosition();

	while (pos)
	{
		LPGPU_DESC lpDesc = g_gpus.GetNext(pos);

		if (lpDesc->dwGpu == dwGpu)
		{
			switch (dwType)
			{
			case GPU_USAGE_TYPE:
				result = lpDesc->fltGpuUsage[dwNode];
				lpDesc->fltGpuUsage[dwNode] = FLT_MAX;
				break;
			case MEM_USAGE_DEDICATED_TYPE:
				result = lpDesc->fltMemUsageDedicated;
				lpDesc->fltMemUsageDedicated = FLT_MAX;
				break;
			case MEM_USAGE_SHARED_TYPE:
				result = lpDesc->fltMemUsageShared;
				lpDesc->fltMemUsageShared = FLT_MAX;
				break;
			case PROCESS_MEM_USAGE_DEDICATED_TYPE:
				result = lpDesc->fltProcessMemUsageDedicated;
				lpDesc->fltProcessMemUsageDedicated = FLT_MAX;
				break;
			case PROCESS_MEM_USAGE_SHARED_TYPE:
				result = lpDesc->fltProcessMemUsageShared;
				lpDesc->fltProcessMemUsageShared = FLT_MAX;
				break;
			}
		}
	}

	return result;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to poll data sources
/////////////////////////////////////////////////////////////////////////////
GPU_API FLOAT GetSourceData(DWORD dwIndex)
{
	FLOAT result = FLT_MAX;

	POSITION pos = g_sources.FindIndex(dwIndex);

	if (pos)
	{
		DWORD dwSource	= g_sources.GetAt(pos);

		DWORD dwGpu		= (dwSource>>GPU_SHIFT	) & GPU_MASK;
		DWORD dwType	= (dwSource>>TYPE_SHIFT	) & TYPE_MASK;
		DWORD dwNode	= (dwSource>>NODE_SHIFT	) & NODE_MASK;

		result = GetSourceData(dwGpu, dwType, dwNode);
			//try to get source data, it will be set to FLT_MAX if we need to resample it

		if (result == FLT_MAX)
			//resample data if necessary
		{
			g_gpus.Update();

			result = GetSourceData(dwGpu, dwType, dwNode);
		}
	}

	return result;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to open foreground process
/////////////////////////////////////////////////////////////////////////////
HANDLE OpenForegroundProcess()
{
	HWND	hWnd		= GetForegroundWindow();
	DWORD	dwProcessId	= 0;

	if (hWnd)
	{
		if (IsFrameWindow(hWnd))
			hWnd = GetCoreWindow(hWnd);

		GetWindowThreadProcessId(hWnd, &dwProcessId);
	}

	if (dwProcessId)
		return OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, dwProcessId);

	return NULL;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to check if window is UWP frame window
/////////////////////////////////////////////////////////////////////////////
BOOL IsFrameWindow(HWND hFrameWnd)
{
	char szClassName[MAX_PATH] = { 0 };

	if (!GetClassName(hFrameWnd, szClassName, sizeof(szClassName)))
		return FALSE;

	return strcmp(szClassName, "ApplicationFrameWindow") == 0;

}
//////////////////////////////////////////////////////////////////////
// This helper function is used to get UWP core window from UWP frame window
/////////////////////////////////////////////////////////////////////////////
HWND GetCoreWindow(HWND hFrameWnd)
{
	DWORD dwFrameProcessID = 0;
	GetWindowThreadProcessId(hFrameWnd, &dwFrameProcessID);

	HWND hCoreWnd = FindWindowEx(hFrameWnd, NULL, NULL, NULL);

	while (hCoreWnd) 
	{
		DWORD dwCoreProcessID = 0;
		GetWindowThreadProcessId(hCoreWnd, &dwCoreProcessID);

		if (dwCoreProcessID != dwFrameProcessID)
			return hCoreWnd;

		hCoreWnd = FindWindowEx(hFrameWnd, hCoreWnd, NULL, NULL);
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
