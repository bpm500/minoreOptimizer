// GPUList.h: interface for the CGPUList class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#ifndef _GPULIST_H_INCLUDED_
#define _GPULIST_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include <afxtempl.h>
#include "D3DKMTWrapper.h"
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
typedef struct GPU_DESC
{
	//GPU identification related variables

	DWORD			dwGpu;
	DWORD			dwBus;
	DWORD			dwDev;
	DWORD			dwFn;

	//GPU usage calcluation related variables

	DWORD			dwTickCount;
	DWORD			dwNodeCount;
	LARGE_INTEGER	qwNodeRunningTime[MAX_NODES];

	//last calculated per-node GPU usages and dedicated / shared memory usages

	FLOAT			fltGpuUsage[MAX_NODES];
	FLOAT			fltMemUsageDedicated;
	FLOAT			fltMemUsageShared;
	FLOAT			fltProcessMemUsageDedicated;
	FLOAT			fltProcessMemUsageShared;
} GPU_DESC, *LPGPU_DESC;
//////////////////////////////////////////////////////////////////////
class CGPUList : public CList<LPGPU_DESC, LPGPU_DESC>
{
public:
	BOOL Init();
	void Uninit();
	void Update();

	CGPUList();
	virtual ~CGPUList();

protected:
	CD3DKMTWrapper	m_wrapper;
};
//////////////////////////////////////////////////////////////////////
#endif 
//////////////////////////////////////////////////////////////////////
