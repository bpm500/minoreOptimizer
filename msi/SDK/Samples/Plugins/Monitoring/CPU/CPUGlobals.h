#ifndef _CPUGLOBALS_H_INCLUDED_
#define _CPUGLOBALS_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include "MSIAfterburnerExports.h"
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
DWORD	ReadBusDataUlong(DWORD bus, DWORD devfn, DWORD offset);
WORD	ReadBusDataUshort(DWORD bus, DWORD devfn, DWORD offset);
BYTE	ReadBusDataUchar(DWORD bus, DWORD devfn, DWORD offset);

void	WriteBusDataUlong(DWORD bus, DWORD devfn, DWORD offset, DWORD data);
void	WriteBusDataUshort(DWORD bus, DWORD devfn, DWORD offset, WORD data);
void	WriteBusDataUchar(DWORD bus, DWORD devfn, DWORD offset, BYTE data);

BOOL	ReadMSR(DWORD dwIndex, LPDWORD lpHigh, LPDWORD lpLow);
BOOL	WriteMSR(DWORD dwIndex, DWORD dwHigh, DWORD dwLow);

DWORD	GetTimestamp();

BOOL	IsIntelCpu();
BOOL	IsAMDCpu();

BOOL	IsAMDZenArchitecture();

LPCSTR	GetCpuString();
LPCSTR	GetCpuBrandString();
DWORD	GetCpuFamily();
DWORD	GetCpuModel();
DWORD	GetCpuStepping();

DWORD	GetTjmax(DWORD dwCpu);

void	InitCpu();
void	InitIntelCpu();
void	InitAMDCpu();

BOOL	IsCpuTemperatureReportingSupported(DWORD dwCpu);
BOOL	IsIntelCpuTemperatureReportingSupported(DWORD dwCpu);
BOOL	IsAMDCpuTemperatureReportingSupported(DWORD dwCpu);

float	GetCpuTemperature(DWORD dwCpu);
float	GetIntelCpuTemperature(DWORD dwCpu);
float	GetAMDCpuTemperature(DWORD dwCpu);

CString GetCfgPath();
CString	GetLogPath();
void	AppendLog(LPCSTR lpLine, BOOL bAppend);
/////////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////