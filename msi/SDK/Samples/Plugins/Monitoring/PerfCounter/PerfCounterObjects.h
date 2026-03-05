// PerfCounterObjects.h: interface for the CPerfCounterObjects class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#ifndef _PERFCOUNTEROBJECTS_H_INCLUDED_
#define _PERFCOUNTEROBJECTS_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include <afxtempl.h>
#include <pdh.h>
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
typedef struct PERFCOUNTER_OBJECT_DESC
{
	char	szName[MAX_PATH];
		//localized performance counter object name
	LPCSTR	lpCounters;	
		//list of object specific counters 
	LPCSTR	lpInstances;
		//list of object specific instances

	char	szSortKey[MAX_PATH];
		//sort key (can be either localized or English object name depending on selected sort mode)

} PERFCOUNTER_OBJECT_DESC, *LPPERFCOUNTER_OBJECT_DESC;
//////////////////////////////////////////////////////////////////////
class CPerfCounterObjects : public CList<LPPERFCOUNTER_OBJECT_DESC,LPPERFCOUNTER_OBJECT_DESC>
{
public:
	void						Init();
	void						Enum();
	void						Uninit();
	void						Sort(BOOL bLocalized);

	LPCSTR						GetCounter(LPCSTR lpObjectName, DWORD dwIndex);
	LPCSTR						GetInstance(LPCSTR lpObjectName, DWORD dwIndex);

	BOOL						GetLocalizedName(LPCSTR lpEngName, LPSTR lpLocName, DWORD dwSize);
	BOOL						GetEnglishName(LPCSTR lpLocName, LPSTR lpEngName, DWORD dwSize);

	CPerfCounterObjects();
	virtual ~CPerfCounterObjects();

private:
	LPCSTR						m_lpEngCounters;
	DWORD						m_dwEngCountersSize;
	LPCSTR						m_lpLocCounters;
	DWORD						m_dwLocCountersSize;

	static int					SortHelper(const void *arg1, const void *arg2);

	LPPERFCOUNTER_OBJECT_DESC	GetObject(LPCSTR lpObjectName);
};
//////////////////////////////////////////////////////////////////////
#endif 
//////////////////////////////////////////////////////////////////////
