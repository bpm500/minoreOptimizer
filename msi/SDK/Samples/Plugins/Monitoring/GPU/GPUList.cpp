// GPUList.cpp: implementation of the CGPUList class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "GPUList.h"
#include "GPUGlobals.h"

#include <float.h>
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPUList::CGPUList()
{
}
//////////////////////////////////////////////////////////////////////
CGPUList::~CGPUList()
{
	Uninit();
}
//////////////////////////////////////////////////////////////////////
void CGPUList::Uninit()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		delete GetNext(pos);

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
BOOL CGPUList::Init()
{
	Uninit();

	//enumerate D3DKMT devices

	if (!m_wrapper.Init())
		return FALSE;

	//enumerate host GPUs and reconcile host GPU indices with D3DKMT devices

	DWORD dwGpuNum = GetGpuNum();

	for (DWORD dwGpu=0; dwGpu<dwGpuNum; dwGpu++)
		//enumarate host GPUs
	{
		CString strID = GetGpuID(dwGpu);
			//get host GPUID string 

		DWORD dwVendorID	= 0;
		DWORD dwDeviceID	= 0;
		DWORD dwSubsysID	= 0;
		DWORD dwRevision	= 0;
		DWORD dwBus			= 0;
		DWORD dwDev			= 0;
		DWORD dwFn			= 0;

		if (sscanf_s(strID, "VEN_%04X&DEV_%04X&SUBSYS_%08X&REV_%02X&BUS_%d&DEV_%d&FN_%d", &dwVendorID, &dwDeviceID, &dwSubsysID, &dwRevision, &dwBus, &dwDev, &dwFn) == 7)
			//extract GPU identification info from host GPUID string
		{
			if (m_wrapper.FindDevice(dwBus, dwDev, dwFn))
				//try to reconcile host GPU devices with enumerated D3DKMT devices using GPU location (PCI bus, device, function)
			{
				LPGPU_DESC lpDesc	= new GPU_DESC;
				ZeroMemory(lpDesc, sizeof(GPU_DESC));

				lpDesc->dwGpu		= dwGpu;
				lpDesc->dwBus		= dwBus;
				lpDesc->dwDev		= dwDev;
				lpDesc->dwFn		= dwFn;

				for (DWORD dwNode=0; dwNode<MAX_NODES; dwNode++)
					lpDesc->fltGpuUsage[dwNode] = FLT_MAX;

				lpDesc->fltMemUsageDedicated	= FLT_MAX;
				lpDesc->fltMemUsageShared		= FLT_MAX;

				AddTail(lpDesc);
			}
		}
	}

	Update();

	return GetCount() != 0;
}
//////////////////////////////////////////////////////////////////////
void CGPUList::Update()
{
	DWORD dwTickCount = GetTickCount();

	POSITION pos = GetHeadPosition();

	while (pos)
		//loop through all GPUs
	{
		//update GPU usages
		
		LPGPU_DESC lpDesc = GetNext(pos);

		LARGE_INTEGER	qwNodeRunningTime[MAX_NODES]	= { 0 };
		DWORD			dwNodeCount						= MAX_NODES;

		m_wrapper.GetGpuRunningTime("DISPLAY", lpDesc->dwBus, lpDesc->dwDev, lpDesc->dwFn, qwNodeRunningTime, &dwNodeCount);
			//get per-node GPU running times

		if (dwNodeCount != MAX_NODES)
		{
			if (lpDesc->dwTickCount)
			{
				if (dwTickCount > lpDesc->dwTickCount)
				{
					for (DWORD dwNode=0; dwNode<lpDesc->dwNodeCount; dwNode++)
						//calclulate per-node GPU usage from two sampled GPU running times
					{
						FLOAT fltGpuUsage = ((qwNodeRunningTime[dwNode].QuadPart - lpDesc->qwNodeRunningTime[dwNode].QuadPart) / (100.0f * (dwTickCount - lpDesc->dwTickCount)));

						if (fltGpuUsage < 0.0f)
							fltGpuUsage = 0.0f;

						if (fltGpuUsage > 100.0f)
							fltGpuUsage = 100.0f;

						lpDesc->qwNodeRunningTime[dwNode].QuadPart	= qwNodeRunningTime[dwNode].QuadPart;
						lpDesc->fltGpuUsage[dwNode]					= fltGpuUsage;
					}

					lpDesc->dwTickCount = dwTickCount;
					lpDesc->dwNodeCount = dwNodeCount;
				}
			}
			else
			{
				for (DWORD dwNode=0; dwNode<dwNodeCount; dwNode++)
					lpDesc->qwNodeRunningTime[dwNode].QuadPart	= qwNodeRunningTime[dwNode].QuadPart;

				lpDesc->dwTickCount = dwTickCount;
				lpDesc->dwNodeCount = dwNodeCount;
			}
		}

		//update memory usages

		DWORD dwMemUsageDedicated	= VIDEOMEMORY_USAGE_INVALID;
		DWORD dwMemUsageShared		= VIDEOMEMORY_USAGE_INVALID;

		m_wrapper.GetVideomemoryUsage("DISPLAY", lpDesc->dwBus, lpDesc->dwDev, lpDesc->dwFn, &dwMemUsageDedicated, &dwMemUsageShared, 0);

		if (dwMemUsageDedicated != VIDEOMEMORY_USAGE_INVALID)
			lpDesc->fltMemUsageDedicated = dwMemUsageDedicated / 1024.0f;

		if (dwMemUsageShared != VIDEOMEMORY_USAGE_INVALID)
			lpDesc->fltMemUsageShared = dwMemUsageShared / 1024.0f;

		//update foreground process specific memory usages

		DWORD dwProcessMemUsageDedicated	= VIDEOMEMORY_USAGE_INVALID;
		DWORD dwProcessMemUsageShared		= VIDEOMEMORY_USAGE_INVALID;

		HANDLE hProcess = OpenForegroundProcess();

		if (hProcess)
		{
			m_wrapper.GetVideomemoryUsage("DISPLAY", lpDesc->dwBus, lpDesc->dwDev, lpDesc->dwFn, &dwProcessMemUsageDedicated, &dwProcessMemUsageShared, hProcess);

			CloseHandle(hProcess);
		}

		if (dwProcessMemUsageDedicated != VIDEOMEMORY_USAGE_INVALID)
			lpDesc->fltProcessMemUsageDedicated = dwProcessMemUsageDedicated / 1024.0f;

		if (dwProcessMemUsageShared != VIDEOMEMORY_USAGE_INVALID)
			lpDesc->fltProcessMemUsageShared = dwProcessMemUsageShared / 1024.0f;
	}
}
//////////////////////////////////////////////////////////////////////

