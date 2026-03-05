#ifndef _PINGGLOBALS_H_INCLUDED_
#define _PINGGLOBALS_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include "MSIAfterburnerExports.h"
#include "PingThread.h"
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
extern HINSTANCE					g_hModule;
extern BOOL							g_bEnableLog;
extern char							g_szAddr[MAX_PATH];
extern DWORD						g_dwHeaderBgndColor;
extern DWORD						g_dwHeaderTextColor;

extern GET_HOST_APP_PROPERTY_PROC	g_pGetHostAppProperty;
extern LOCALIZEWND_PROC				g_pLocalizeWnd;
extern LOCALIZESTR_PROC				g_pLocalizeStr;
extern CPingThread*					g_pThread;
//////////////////////////////////////////////////////////////////////
CString GetCfgPath();
CString	GetLogPath();
LPCSTR	LocalizeStr(LPCSTR lpStr);
void	LocalizeWnd(HWND hWnd);
void	AppendLog(LPCSTR lpLine, BOOL bAppend);
void	AdjustWindowPos(CWnd* pWnd, CWnd* pParent);
/////////////////////////////////////////////////////////////////////////////
#define APPEND_LOG(s) { AppendLog(s, TRUE); }	
#define APPEND_LOG1(s, p1) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1); AppendLog(czLog, TRUE); }
#define APPEND_LOG2(s, p1, p2) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2); AppendLog(czLog, TRUE); }
#define APPEND_LOG3(s, p1, p2, p3) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2, p3); AppendLog(czLog, TRUE); }
#define APPEND_LOG4(s, p1, p2, p3, p4) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2, p3, p4); AppendLog(czLog, TRUE); }
#define APPEND_LOG5(s, p1, p2, p3, p4, p5) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2, p3, p4, p5); AppendLog(czLog, TRUE); }
#define APPEND_LOG6(s, p1, p2, p3, p4, p5, p6) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2, p3, p4, p5, p6); AppendLog(czLog, TRUE); }
#define APPEND_LOG7(s, p1, p2, p3, p4, p5, p6, p7) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2, p3, p4, p5, p6, p7); AppendLog(czLog, TRUE); }
#define APPEND_LOG8(s, p1, p2, p3, p4, p5, p6, p7, p8) { char czLog[256]; sprintf_s(czLog, sizeof(czLog), s, p1, p2, p3, p4, p5, p6, p7, p8); AppendLog(czLog, TRUE); }
//////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////