// D3DKMTWrapper.h: interface for the CD3DKMTWrapper class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#ifndef _D3DKMTWRAPPER_H_INCLUDED_
#define _D3DKMTWRAPPER_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
#include "D3DKMTInterface.h"
#include "D3DKMTDeviceList.h"
//////////////////////////////////////////////////////////////////////
#define MAX_NODES	256
//////////////////////////////////////////////////////////////////////
#define VIDEOMEMORY_USAGE_INVALID					0xFFFFFFFF
#define VIDEOMEMORY_USAGE_INVALID64					0xFFFFFFFFFFFFFFFF
//////////////////////////////////////////////////////////////////////
class CD3DKMTWrapper  
{
public:
	BOOL Init();
	void GetVideomemoryUsage(LPCSTR lpDisplayName, DWORD dwBus, DWORD dwDev, DWORD dwFn, LPDWORD lpDedicatedVideomemoryUsage, LPDWORD lpSharedVideomemoryUsage, HANDLE hProcess);
	void GetGpuRunningTime(LPCSTR lpDisplayName, DWORD dwBus, DWORD dwDev, DWORD dwFn, LARGE_INTEGER* lpNodeRunningTimeArr, LPDWORD lpNodeCount);

	LPD3DKMTDEVICE_DESC FindDevice(DWORD dwBus, DWORD dwDev, DWORD dwFn);

	CD3DKMTDeviceList* GetDeviceList();

	CD3DKMTWrapper();
	virtual ~CD3DKMTWrapper();

protected:
	CD3DKMTInterface	m_interface;
	CD3DKMTDeviceList	m_deviceList;
	OSVERSIONINFO		m_osVersion;

	void	GetVideomemoryUsageByGdiDisplayName(LPCSTR lpDisplayName, LPDWORD lpDedicatedVideomemoryUsage, LPDWORD lpSharedVideomemoryUsage, HANDLE hProcess);
	void	GetVideomemoryUsageByDeviceName(LPCSTR lpDeviceName, LPDWORD lpDedicatedVideomemoryUsage, LPDWORD lpSharedVideomemoryUsage, HANDLE hProcess);

	void	GetVideomemoryUsageVista(D3DKMT_HANDLE hAdapter, LPDWORD lpDedicatedVideomemoryUsage, LPDWORD lpSharedVideomemoryUsage, HANDLE hProcess);
	void	GetVideomemoryUsageWin7(LUID AdapterLuid, LPDWORD lpDedicatedVideomemoryUsage, LPDWORD lpSharedVideomemoryUsage, HANDLE hProcess);
	void	GetVideomemoryUsageWin8(LUID AdapterLuid, LPDWORD lpDedicatedVideomemoryUsage, LPDWORD lpSharedVideomemoryUsage, HANDLE hProcess);
	DWORD	GetStatisticsBlockSize(_D3DKMT_STATISTICS_VISTA* pStatistics, DWORD dwIndex);

	void	GetGpuRunningTimeByGdiDisplayName(LPCSTR lpDisplayName, LARGE_INTEGER* lpNodeRunningTimeArr, LPDWORD lpNodeCount);
	void	GetGpuRunningTimeByDeviceName(LPCSTR lpDeviceName, LARGE_INTEGER* lpNodeRunningTimeArr, LPDWORD lpNodeCount);

	void	GetGpuRunningTimeVista(D3DKMT_HANDLE hAdapter, LARGE_INTEGER* lpNodeRunningTimeArr, LPDWORD lpNodeCount);
	void	GetGpuRunningTimeWin7(LUID AdapterLuid, LARGE_INTEGER* lpNodeRunningTimeArr, LPDWORD lpNodeCount);
	void	GetGpuRunningTimeWin8(LUID AdapterLuid, LARGE_INTEGER* lpNodeRunningTimeArr, LPDWORD lpNodeCount);
};
//////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////
