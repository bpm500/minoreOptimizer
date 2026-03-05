// PerfCounterObjects.cpp: implementation of the CPerfCounterObjects class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PerfCounterObjects.h"
#include "MultiString.h"
//////////////////////////////////////////////////////////////////////
#include <pdh.h>
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPerfCounterObjects::CPerfCounterObjects()
{
	m_lpEngCounters		= NULL;
	m_dwEngCountersSize	= 0;
	m_lpLocCounters		= NULL;
	m_dwLocCountersSize	= 0;
}
//////////////////////////////////////////////////////////////////////
CPerfCounterObjects::~CPerfCounterObjects()
{
	Uninit();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterObjects::Init()
{
	Uninit();

	//init English index/name pairs map, we'll use it to convert English names to localized ones 

	DWORD dwType;
	DWORD dwSize = 0;

	RegQueryValueEx(HKEY_PERFORMANCE_DATA, "Counter 009", NULL, &dwType, NULL, &dwSize);

	if (dwSize)
	{
		m_lpEngCounters		= new char[dwSize];
		m_dwEngCountersSize	= dwSize;

		RegQueryValueEx(HKEY_PERFORMANCE_DATA, "Counter 009", NULL, &dwType, (LPBYTE)m_lpEngCounters, &dwSize);
	}

	//init loclized index/name pairs map, we'll use it to convert localized names back to English ones 

	CString strLocValueName;
	strLocValueName.Format("Counter %03X", GetSystemDefaultLCID() & 0xFF);
		//format registry value name for localized index/name pairs map

	dwSize = 0;

	RegQueryValueEx(HKEY_PERFORMANCE_DATA, strLocValueName, NULL, &dwType, NULL, &dwSize);

	if (dwSize)
	{

		m_lpLocCounters		= new char[dwSize];
		m_dwLocCountersSize	= dwSize;

		RegQueryValueEx(HKEY_PERFORMANCE_DATA, strLocValueName, NULL, &dwType, (LPBYTE)m_lpLocCounters, &dwSize);
	}
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterObjects::GetLocalizedName(LPCSTR lpEngName, LPSTR lpLocName, DWORD dwSize)
{
	strcpy_s(lpLocName, dwSize, lpEngName);
		//init result with English name (some third party objects/counters have no localization)

	if (m_lpEngCounters)
	{
		CMultiString mstr(m_lpEngCounters, m_dwEngCountersSize);
			//process English counter names stored in REG_MULT_SZ in index/name pairs map

		DWORD dwIndex = mstr.FindIndexInMap(lpEngName);
			//try to find index by English name in the map

		if (dwIndex != 0xFFFFFFFF)
			//if we found the index, we can translate the name with PDH API
		{
			BOOL bResult = (PdhLookupPerfNameByIndex(NULL, dwIndex, lpLocName, &dwSize) == ERROR_SUCCESS);

			if (bResult)
				//some third party objects/counters may have valid index but empty localized entry, so we need to handle
				//such cases
			{
				if (!strlen(lpLocName))
					strcpy_s(lpLocName, dwSize, lpEngName);
						//reinit result with English name if empty localization is returned
			}
			else
				//reinit result with English name on PdhLookupPerfNameByIndex failure
				strcpy_s(lpLocName, dwSize, lpEngName);

			return bResult;
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterObjects::GetEnglishName(LPCSTR lpLocName, LPSTR lpEngName, DWORD dwSize)
{
	strcpy_s(lpEngName, dwSize, lpLocName);
		//init result with localized name (some third party objects/counters have no localization)

	if (m_lpLocCounters)
	{
		CMultiString mstrLoc(m_lpLocCounters, m_dwLocCountersSize);
			//process localized counter names stored in REG_MULT_SZ in index/name pairs map

		DWORD dwIndex = mstrLoc.FindIndexInMap(lpLocName);
			//try to find index by localized name in the map

		if (dwIndex != 0xFFFFFFFF)
			//if we found the index, we can translate the name using english counter names map (sadly there is no PDH API for that)
		{
			if (m_lpEngCounters)
			{
				CMultiString mstrEng(m_lpEngCounters, m_dwEngCountersSize);
					//process English counter names names stored in REG_MULT_SZ in index/name pairs map

				LPCSTR lpResult = mstrEng.FindNameInMap(dwIndex);
					//try to find English name by index in the map

				if (lpResult)
				{
					strcpy_s(lpEngName, dwSize, lpResult);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
LPPERFCOUNTER_OBJECT_DESC CPerfCounterObjects::GetObject(LPCSTR lpName)
{
	//first, try to find existing object by name

	POSITION pos = GetHeadPosition();

	while (pos)
	{
		LPPERFCOUNTER_OBJECT_DESC lpDesc = GetNext(pos);

		if (!_stricmp(lpDesc->szName, lpName))
			return lpDesc;
	}

	//second, allocate new object and populate its counters/instances

	LPPERFCOUNTER_OBJECT_DESC lpDesc = new PERFCOUNTER_OBJECT_DESC;
	ZeroMemory(lpDesc, sizeof(PERFCOUNTER_OBJECT_DESC));
		//allocate and zap new object descriptor

	strcpy_s(lpDesc->szName, sizeof(lpDesc->szName), lpName);
		//init object name

	DWORD dwCountersListSize	= 0;
	DWORD dwInstancesListSize	= 0;

	PdhEnumObjectItems(NULL, NULL, lpName, NULL, &dwCountersListSize, NULL, &dwInstancesListSize, PERF_DETAIL_WIZARD, 0);
		//get size of counters and instances lists

	if (dwCountersListSize)
		//allocate and zap counters list if necessary
	{
		lpDesc->lpCounters = new char[dwCountersListSize];
		ZeroMemory((LPSTR)lpDesc->lpCounters, dwCountersListSize);
	}

	if (dwInstancesListSize)
		//allocate and zap instances list if necessary
	{
		lpDesc->lpInstances = new char[dwInstancesListSize];
		ZeroMemory((LPSTR)lpDesc->lpInstances, dwInstancesListSize);
	}

	PdhEnumObjectItems(NULL, NULL, lpName, (LPSTR)lpDesc->lpCounters, &dwCountersListSize, (LPSTR)lpDesc->lpInstances, &dwInstancesListSize, PERF_DETAIL_WIZARD, 0);
		//init counters and instances lists

	AddTail(lpDesc);
		//add object descriptor to the list

	return lpDesc;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CPerfCounterObjects::GetInstance(LPCSTR lpName, DWORD dwIndex)
{
	LPPERFCOUNTER_OBJECT_DESC lpDesc = GetObject(lpName);
		//find existing object in the list or allocate new one

	if (lpDesc && lpDesc->lpInstances)
		//if instances list is available for the object, we can return an instance by index
	{
		CMultiString mstr(lpDesc->lpInstances);
			//process instances stored in REG_MULTI_SZ list

		return mstr.GetIndex(dwIndex);
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CPerfCounterObjects::GetCounter(LPCSTR lpName, DWORD dwIndex)
{
	LPPERFCOUNTER_OBJECT_DESC lpDesc = GetObject(lpName);
		//find existing object in the list or allocate new one

	if (lpDesc && lpDesc->lpCounters)
		//if counters list is available for the object, we can return the counter by index
	{
		CMultiString mstr(lpDesc->lpCounters);
			//process counters stored in REG_MULTI_SZ list

		return mstr.GetIndex(dwIndex);
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterObjects::Uninit()
{
	if (m_lpEngCounters)
		delete [] m_lpEngCounters;
	m_lpEngCounters		= NULL;
	m_dwEngCountersSize = 0;

	if (m_lpLocCounters)
		delete [] m_lpLocCounters;
	m_lpLocCounters		= NULL;
	m_dwLocCountersSize = 0;

	POSITION pos = GetHeadPosition();

	while (pos)
	{
		LPPERFCOUNTER_OBJECT_DESC lpDesc = GetNext(pos);

		if (lpDesc->lpCounters)
			delete [] lpDesc->lpCounters;

		if (lpDesc->lpInstances)
			delete [] lpDesc->lpInstances;

		delete lpDesc;
	}

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterObjects::Enum()
{
	Init();

	DWORD dwObjectsListSize	= 0;

	PdhEnumObjects(NULL, NULL, NULL, &dwObjectsListSize, PERF_DETAIL_WIZARD, TRUE);
		//get size of objects list

	if (dwObjectsListSize)
	{
		LPSTR lpObjects = new char[dwObjectsListSize];
		ZeroMemory(lpObjects, dwObjectsListSize);
			//allocate and zap objects list

		PdhEnumObjects(NULL, NULL, lpObjects, &dwObjectsListSize, PERF_DETAIL_WIZARD, FALSE);
			//init objects list 

		CMultiString mstr(lpObjects);
			//enumarate objects stored in REG_MULTI_SZ list
		
		LPCSTR lpObject = mstr.GetNext();

		while (lpObject)
			//process objects list
		{
			GetObject(lpObject);
				//just call GetObject to add new object descriptor to the list and 
				//populate its counters and instances

			lpObject = mstr.GetNext();
		}

		delete [] lpObjects;
			//delete objects list
	}
}
//////////////////////////////////////////////////////////////////////
int CPerfCounterObjects::SortHelper(const void *arg1, const void *arg2) 
{
	LPCSTR a1 = (*(LPPERFCOUNTER_OBJECT_DESC*)arg1)->szSortKey;
	LPCSTR a2 = (*(LPPERFCOUNTER_OBJECT_DESC*)arg2)->szSortKey;

	return strcmp(a1, a2);
}
/////////////////////////////////////////////////////////////////////////////
void CPerfCounterObjects::Sort(BOOL bLocalized)
{
	DWORD dwObjects = GetCount();

	if (dwObjects)
	{
		LPVOID* lpSortedObjects = new LPVOID[dwObjects];
			//allocate memory for temporary array of ptrs to object descriptors, which we'll use for sorting

		POSITION	pos			= GetHeadPosition();
		DWORD		dwObject	= 0;

		while (pos)
			//populate temporary array of ptrs to object descriptors
		{
			LPPERFCOUNTER_OBJECT_DESC lpDesc = GetNext(pos);

			if (bLocalized)
				strcpy_s(lpDesc->szSortKey, sizeof(lpDesc->szSortKey), lpDesc->szName);
					//init sort key with localized object name
			else
				GetEnglishName(lpDesc->szName, lpDesc->szSortKey, sizeof(lpDesc->szSortKey));
					//init sort key with English object name

			lpSortedObjects[dwObject++] = lpDesc;
		}

		qsort(lpSortedObjects, dwObjects, sizeof(LPVOID), SortHelper);
			//sort temporary array of ptrs to object descriptors

		RemoveAll();

		for (DWORD dwObject=0; dwObject<dwObjects; dwObject++)
			AddTail((LPPERFCOUNTER_OBJECT_DESC)lpSortedObjects[dwObject]);
				//recreate the list using sorted temporary array of ptrs to object descriptors

		delete [] lpSortedObjects;
			//free temporary array
	}

}
//////////////////////////////////////////////////////////////////////


