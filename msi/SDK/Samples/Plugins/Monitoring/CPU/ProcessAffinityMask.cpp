// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ProcessAffinityMask.h"
//////////////////////////////////////////////////////////////////////
CProcessAffinityMask::CProcessAffinityMask(DWORD dwCpuCount, DWORD dwCpu)
{
	m_dwCpuCount	= dwCpuCount;
	m_dwCpu			= dwCpu;

	//get process affinity mask

	m_hProcess		= GetCurrentProcess();

	GetProcessAffinityMask(m_hProcess, &m_dwProcessAffinityMask, &m_dwSystemAffinityMask);

	if (m_dwCpuCount > 1)
		//if we're under multi-CPU system then we need to force OS to execute our code
		//on specified CPU in order to read proper MSR
		SetProcessAffinityMask(m_hProcess, 1<<m_dwCpu);
			//force OS to execute our code on specified CPU
}
//////////////////////////////////////////////////////////////////////
CProcessAffinityMask::~CProcessAffinityMask()
{
	if (m_dwCpuCount > 1)
		SetProcessAffinityMask(m_hProcess, m_dwProcessAffinityMask);
			//restore the previous process affinity mask 
}
//////////////////////////////////////////////////////////////////////
