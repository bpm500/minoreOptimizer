#ifndef _PERFCOUNTERGLOBALS_H_INCLUDED_
#define _PERFCOUNTERGLOBALS_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include "PerfCounterDataSources.h"
#include "MSIAfterburnerExports.h"
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
extern GET_HOST_APP_PROPERTY_PROC	g_pGetHostAppProperty;
extern LOCALIZEWND_PROC				g_pLocalizeWnd;
extern LOCALIZESTR_PROC				g_pLocalizeStr;

extern HINSTANCE					g_hModule;
extern DWORD						g_dwSpawn;
extern DWORD						g_dwSpawnPeriod;
extern DWORD						g_dwHeaderBgndColor;
extern DWORD						g_dwHeaderTextColor;

extern CPerfCounterDataSources		g_sources;
/////////////////////////////////////////////////////////////////////////////
LPCSTR	LocalizeStr(LPCSTR lpStr);
void	LocalizeWnd(HWND hWnd);
void	AppendLog(LPCSTR lpLine, BOOL bAppend);
void	AdjustWindowPos(CWnd* pWnd, CWnd* pParent);
void	FormatName(LPCSTR lpSrcTmpl, DWORD dwSrcInst, CString& strDstName, CString& strDstTmpl);
CString GetCfgPath();
CString	GetLogPath();
//////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////