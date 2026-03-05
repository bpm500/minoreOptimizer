// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "CPUTopology.h"
/////////////////////////////////////////////////////////////////////////////
typedef BOOL (WINAPI *GETLOGICALPROCESSORINFORMATIONEX)(LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD);
/////////////////////////////////////////////////////////////////////////////
CCPUTopology::CCPUTopology()
{
	m_lpInfo = NULL;
	m_dwSize = 0;

	ZeroMemory(m_pckgMap, sizeof(m_pckgMap));
	ZeroMemory(m_coreMap, sizeof(m_coreMap));
}
/////////////////////////////////////////////////////////////////////////////
CCPUTopology::~CCPUTopology()
{
	Uninit();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CCPUTopology::Init()
{
	Uninit();

	HMODULE hKernel32 = GetModuleHandle("kernel32");

	if (hKernel32)
	{
		GETLOGICALPROCESSORINFORMATIONEX pGetLogicalProcessorInformationEx = (GETLOGICALPROCESSORINFORMATIONEX)GetProcAddress(hKernel32, "GetLogicalProcessorInformationEx");

		if (pGetLogicalProcessorInformationEx)
		{
			pGetLogicalProcessorInformationEx(RelationAll, NULL, &m_dwSize);	

			if (m_dwSize)
			{
				m_lpInfo = new BYTE[m_dwSize];	
				
				if (pGetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)m_lpInfo, &m_dwSize))
				{
					LPBYTE	lpInfo	= GetInfo();
					DWORD	dwSize	= GetSize();
					DWORD	dwPos	= 0;

					DWORD	dwPckg	= 0;
					DWORD	dwCore	= 0;

					while (dwPos < dwSize)
					{
						PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX lpEntry = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)(lpInfo + dwPos);

						switch (lpEntry->Relationship)
						{
						case RelationProcessorPackage:
							for (DWORD dwGroup=0; dwGroup<lpEntry->Processor.GroupCount; dwGroup++)
							{
								for (DWORD dwCpu=0; dwCpu<MAX_LOGICAL_CPU; dwCpu++)
								{
									if (lpEntry->Processor.GroupMask[dwGroup].Mask & (1<<dwCpu))
										m_pckgMap[dwCpu] = (BYTE)dwPckg;
								}
							}
							dwPckg++;
							break;
						case RelationProcessorCore:
							for (DWORD dwCpu=0; dwCpu<MAX_LOGICAL_CPU; dwCpu++)
							{
								if (lpEntry->Processor.GroupMask[0].Mask & (1<<dwCpu))
									m_coreMap[dwCpu] = (BYTE)dwCore;
							}
							dwCore++;
							break;
						}

						dwPos += lpEntry->Size;
					}

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CCPUTopology::Uninit()
{
	if (m_lpInfo)
		delete [] m_lpInfo;
	m_lpInfo = NULL;

	m_dwSize = 0;

	ZeroMemory(m_pckgMap, sizeof(m_pckgMap));
	ZeroMemory(m_coreMap, sizeof(m_coreMap));
}
/////////////////////////////////////////////////////////////////////////////
LPBYTE CCPUTopology::GetInfo()
{
	return m_lpInfo;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCPUTopology::GetSize()
{
	return m_dwSize;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCPUTopology::GetPckg(DWORD dwCpu)
{
	if (dwCpu < MAX_LOGICAL_CPU)
		return m_pckgMap[dwCpu];

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCPUTopology::GetCore(DWORD dwCpu)
{
	if (dwCpu < MAX_LOGICAL_CPU)
		return m_coreMap[dwCpu];

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
