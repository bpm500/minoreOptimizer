// PerfCounterDataSources.cpp: implementation of the CPerfCounterDataSources class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PerfCounterDataSources.h"
#include "MAHMSharedMemory.h"
#include "MultiString.h"
//////////////////////////////////////////////////////////////////////
#include <float.h>
#include <pdh.h>
#include <io.h>
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPerfCounterDataSources::CPerfCounterDataSources()
{
}
//////////////////////////////////////////////////////////////////////
CPerfCounterDataSources::~CPerfCounterDataSources()
{
	Uninit();
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterDataSources::Init(LPCSTR lpFilename)
{
	CString strFilename			= lpFilename;
	CString strFilenameUser		= lpFilename + CString(".user");

	if (!_taccess(strFilenameUser, 0))
		strFilename = strFilenameUser;

	Uninit();

	//init objects to populate English and localized names map

	m_objects.Init();

	//open query

	if (PdhOpenQuery(0,0,&m_hQuery) != ERROR_SUCCESS)
		return FALSE;

	char szBuf[MAX_PATH];
		//local string buffer

	for (DWORD dwSrc=0; dwSrc<INT_MAX; dwSrc++)
		//loop through all sources
	{
		CString strSrc;	strSrc.Format("Source%d", dwSrc);
			//format source section name

		GetPrivateProfileString(strSrc, "ObjectName", "", szBuf, MAX_PATH, strFilename);
			//read PerfCounter object name from .cfg
		if (!strlen(szBuf))
			//break the loop if source doesn't exist (i.e.  PerfCounter object name is empty)
			break;

		LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = new PERFCOUNTER_DATA_SOURCE_DESC; ZeroMemory(lpDesc, sizeof(PERFCOUNTER_DATA_SOURCE_DESC));
			//allocate and zap new source descriptor

		strcpy_s(lpDesc->szObjectName, sizeof(lpDesc->szObjectName), szBuf);
			//copy PerfCounter object name
		GetPrivateProfileString(strSrc, "InstanceName"	, "", lpDesc->szInstanceName, sizeof(lpDesc->szInstanceName), strFilename);
			//read PerfCounter instance from .cfg
		GetPrivateProfileString(strSrc, "InstanceIndex"	, "-1", szBuf, MAX_PATH, strFilename);
			//read PerfCounter instance index from .cfg (-1 means that index is ignored)
		sscanf_s(szBuf, "%d", &lpDesc->dwInstanceIndex);
			//scan PerfCounter instance index
		GetPrivateProfileString(strSrc, "CounterName"	, "", lpDesc->szCounterName, sizeof(lpDesc->szCounterName), strFilename);
			//read PerfCounter counter name from .cfg

		GetPrivateProfileString(strSrc, "Mul"	, "1", szBuf, MAX_PATH, strFilename);
			//read PerfCounter multiplier from .cfg
		if (sscanf_s(szBuf, "%d", &lpDesc->dwMul) != 1)
			lpDesc->dwMul = 1;
			//scan PerfCounter multiplier
		GetPrivateProfileString(strSrc, "Div"	, "1", szBuf, MAX_PATH, strFilename);
			//read PerfCounter divider from .cfg
		if (sscanf_s(szBuf, "%d", &lpDesc->dwDiv) != 1)
			lpDesc->dwDiv = 1;
			//scan PerfCounter divider
		GetPrivateProfileString(strSrc, "Off"	, "0", szBuf, MAX_PATH, strFilename);
			//read PerfCounter offset from .cfg
		if (sscanf_s(szBuf, "%d", &lpDesc->dwOff) != 1)
			lpDesc->dwOff = 0;
			//scan PerfCounter offset

		lpDesc->bDynamic = (GetPrivateProfileInt(strSrc, "Dynamic", 0, strFilename) != 0);
			//read dynamic mode from .cfg

		GetPrivateProfileString(strSrc, "Name"		, "", lpDesc->szName	, sizeof(lpDesc->szName)	, strFilename);
			//read source name from .cfg
		GetPrivateProfileString(strSrc, "Units"		, "", lpDesc->szUnits	, sizeof(lpDesc->szUnits)	, strFilename);
			//read source units from .cfg
		GetPrivateProfileString(strSrc, "Group"		, "", lpDesc->szGroup	, sizeof(lpDesc->szGroup)	, strFilename);
			//read source group from .cfg
		GetPrivateProfileString(strSrc, "Format"	, "", lpDesc->szFormat	, sizeof(lpDesc->szFormat)	, strFilename);
			//read source format from .cfg

		GetPrivateProfileString(strSrc, "MaxLimit"	, "", szBuf				, sizeof(szBuf)				, strFilename);
			//read maximum limit
		sscanf_s(szBuf, "%f", &lpDesc->fltMaxLimit);
			//scan maximum limit string and convert it to float
		GetPrivateProfileString(strSrc, "MinLimit"	, "", szBuf				, sizeof(szBuf)				, strFilename);
			//read mimimum limit
		sscanf_s(szBuf, "%f", &lpDesc->fltMinLimit);
			//scan minimum limit string and convert it to float

		lpDesc->fltData		= FLT_MAX;
		lpDesc->bValid		= FALSE;
			//reset source data
		
		lpDesc->dwID = MONITORING_SOURCE_ID_PLUGIN_MISC;
			//init default source ID

		GetPrivateProfileString(strSrc, "SourceID"		, "", szBuf				, sizeof(szBuf)				, strFilename);
			//read source ID from .cfg
		sscanf_s(szBuf, "%X", &lpDesc->dwID);
			//scan source ID string and convert it to DWORD
		GetPrivateProfileString(strSrc, "SourceInstance", "", szBuf			, sizeof(szBuf)				, strFilename);
			//read source instance index from .cfg
		sscanf_s(szBuf, "%X", &lpDesc->dwInstance);
			//scan source instance index string and convert it to DWORD
		
		char	szLocalizedObjectName	[MAX_PATH];
		char	szLocalizedInstanceName	[MAX_PATH];
		char	szLocalizedCounterName	[MAX_PATH];

		DWORD	dwInstanceIndex	= lpDesc->dwInstanceIndex;

		m_objects.GetLocalizedName(lpDesc->szObjectName		, szLocalizedObjectName		, sizeof(szLocalizedObjectName));
		m_objects.GetLocalizedName(lpDesc->szInstanceName	, szLocalizedInstanceName	, sizeof(szLocalizedInstanceName));
		m_objects.GetLocalizedName(lpDesc->szCounterName	, szLocalizedCounterName	, sizeof(szLocalizedCounterName));
			//get localized object, instance and counter names

		if (!strlen(szLocalizedInstanceName) && (dwInstanceIndex != -1))
			//if instance name is not specified, then we'll treat specified index as the index in the list of enumerated
			//object's system specific enumerated instance names
		{
			LPCSTR lpInstanceName = m_objects.GetInstance(szLocalizedObjectName, dwInstanceIndex);

			if (lpInstanceName && _stricmp(lpInstanceName, "_Total"))
				//do not allow enumerated _Total instance to be selected by index
			{
				strcpy_s(szLocalizedInstanceName, sizeof(szLocalizedInstanceName), lpInstanceName);

				dwInstanceIndex = -1;
			}
		}

		PDH_COUNTER_PATH_ELEMENTS elements;
		ZeroMemory(&elements, sizeof(elements));

		elements.szMachineName		= NULL;
		elements.szObjectName		= szLocalizedObjectName;
		if (strlen(szLocalizedInstanceName))
			elements.szInstanceName	= szLocalizedInstanceName;
		elements.szParentInstance	= NULL;
		elements.dwInstanceIndex	= dwInstanceIndex;
		elements.szCounterName		= szLocalizedCounterName;
			//prepare counter path elements

		DWORD dwSize = sizeof(lpDesc->szCounterPath);

		PdhMakeCounterPath(&elements, lpDesc->szCounterPath, &dwSize, 0);
			//format full localized counter path

		if (strlen(lpDesc->szCounterPath))
			//try to add counter to query if we succesfully formatted the path
		{
			if (lpDesc->bDynamic || (PdhValidatePath(lpDesc->szCounterPath) == ERROR_SUCCESS))
				//attempts to add a counter with invalid path are VERY SLOW, so we validate the path before trying to add the counter.
				//Dynamic sources (e.g. process specific ones) are the exception, we always try to add them without path validation
				PdhAddCounter(m_hQuery, lpDesc->szCounterPath, 0, &lpDesc->hCounter);
		}

		AddTail(lpDesc);
			//add descriptor to the list
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterDataSources::Uninit()
{
	POSITION pos = GetHeadPosition();

	while (pos)
	{
		LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = GetNext(pos);

		if (lpDesc->hCounter)
			PdhRemoveCounter(lpDesc->hCounter);

		delete lpDesc;
	}

	RemoveAll();

	if (m_hQuery)
		PdhCloseQuery(m_hQuery);
	m_hQuery = NULL;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterDataSources::UpdateSources()
{
	if (m_hQuery)
	{
		if (PdhCollectQueryData(m_hQuery) == ERROR_SUCCESS)
		{
			POSITION pos = GetHeadPosition();

			while (pos)
			{
				LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = GetNext(pos);

				DWORD	dwType;

				PDH_FMT_COUNTERVALUE value;
				ZeroMemory(&value, sizeof(value));

				if (PdhGetFormattedCounterValue(lpDesc->hCounter, PDH_FMT_DOUBLE, &dwType, &value) == ERROR_SUCCESS)
				{
					lpDesc->fltData = (float)value.doubleValue;

					if (lpDesc->dwMul)
						lpDesc->fltData = lpDesc->fltData * lpDesc->dwMul;

					if (lpDesc->dwDiv)
						lpDesc->fltData = lpDesc->fltData / lpDesc->dwDiv;

					lpDesc->fltData = lpDesc->fltData + lpDesc->dwOff;
				}

				lpDesc->bValid	= TRUE;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////
LPPERFCOUNTER_DATA_SOURCE_DESC CPerfCounterDataSources::GetSourceDesc(DWORD dwSource)
{
	POSITION pos = FindIndex(dwSource);

	if (pos)
		return GetAt(pos);

	return NULL;
}
//////////////////////////////////////////////////////////////////////
FLOAT CPerfCounterDataSources::GetSourceData(DWORD dwSource)
{
	LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = GetSourceDesc(dwSource);
		//try to find source descriptor by index

	if (lpDesc)
	{
		if (lpDesc->bValid)
			//check if source data is valid 
		{
			FLOAT fltData	= lpDesc->fltData;
			lpDesc->fltData = FLT_MAX;
			lpDesc->bValid	= FALSE;
				//reset data to force it to be reread from PerfCounter on the next call
			return fltData;
				//return source data
		}
		else
		{
			if (lpDesc->hCounter)
				//do not allow invalid counters to invalidate the rest ones
			{
				UpdateSources();
					//if source data was invalid, we'll try reread it from PerfCounter

				if (lpDesc->bValid)
					//check if source data became valid 
				{
					FLOAT fltData	= lpDesc->fltData;
					lpDesc->fltData = FLT_MAX;
					lpDesc->bValid	= FALSE;
						//reset data to force it to be reread from PerfCounter on the next call
					return fltData;
						//return source data
				}
			}
		}
	}

	return FLT_MAX;
		//data is invalid, PerfCounter is not loaded or the source is not supported by hardware
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterDataSources::InvalidateSources()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		//loop through all sources
	{
		LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = GetNext(pos);
			//get current source descriptor

		lpDesc->fltData = FLT_MAX;
		lpDesc->bValid	= FALSE;
			//reset source data
	}
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterDataSources::Save(LPCSTR lpFilename)
{
	CString strFilename = lpFilename + CString(".user");

	try
	{
		CFile::Remove(strFilename);
	}
	catch(CFileException* /*exception*/)
	{
	}

	CString strSourcesNum;
	strSourcesNum.Format("%d", GetCount());
	WritePrivateProfileString("Global", "Sources", strSourcesNum, strFilename);
		//we'll always write sources counter to ensure that user profile is created even when sources list is empty

	int iSource = 0;

	POSITION pos = GetHeadPosition();

	while (pos)
		//loop through all sources
	{
		LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = GetNext(pos);
			//get source descriptor

		CString strSrc;	strSrc.Format("Source%d", iSource++);
			//format source section name

		WritePrivateProfileString(strSrc, "ObjectName", lpDesc->szObjectName, strFilename);
			//object name

		if (lpDesc->dwInstanceIndex != 0xFFFFFFFF)
		{
			CString strInstanceIndex;
			strInstanceIndex.Format("%d", lpDesc->dwInstanceIndex);
			WritePrivateProfileString(strSrc, "InstanceIndex", strInstanceIndex, strFilename);
				//instance index
		}
		else
		{
			if (strlen(lpDesc->szInstanceName))
				WritePrivateProfileString(strSrc, "InstanceName", lpDesc->szInstanceName, strFilename);
					//instance name
		}

		WritePrivateProfileString(strSrc, "CounterName", lpDesc->szCounterName, strFilename);
			//counter name

		if (lpDesc->bDynamic)
			WritePrivateProfileString(strSrc, "Dynamic", lpDesc->bDynamic ? "1" : "0", strFilename);
			//dynamic mode
		WritePrivateProfileString(strSrc, "Name", lpDesc->szName, strFilename);
			//source name
		if (strlen(lpDesc->szUnits))
			WritePrivateProfileString(strSrc, "Units", lpDesc->szUnits, strFilename);
			//source units
		if (strlen(lpDesc->szGroup))
			WritePrivateProfileString(strSrc, "Group", lpDesc->szGroup, strFilename);
			//source group
		if (strlen(lpDesc->szFormat))
			WritePrivateProfileString(strSrc, "Format", lpDesc->szFormat, strFilename);
			//source format
		CString strSourceMax;
		strSourceMax.Format("%.1f", lpDesc->fltMaxLimit);
		WritePrivateProfileString(strSrc, "MaxLimit", strSourceMax, strFilename);
		CString strSourceMin;
		strSourceMin.Format("%.1f", lpDesc->fltMinLimit);
		WritePrivateProfileString(strSrc, "MinLimit", strSourceMin, strFilename);
			//source range

		if (lpDesc->dwOff)
		{
			CString strOff;
			strOff.Format("%d", lpDesc->dwOff);
			WritePrivateProfileString(strSrc, "Off", strOff, strFilename);
		}
		if (lpDesc->dwMul != 1)
		{
			CString strMul;
			strMul.Format("%d", lpDesc->dwMul);
			WritePrivateProfileString(strSrc, "Mul", strMul, strFilename);
		}
		if (lpDesc->dwDiv != 1)
		{
			CString strDiv;
			strDiv.Format("%d", lpDesc->dwDiv);
			WritePrivateProfileString(strSrc, "Div", strDiv, strFilename);
		}
			//source formula

		CString strSourceID;
		strSourceID.Format("%X", lpDesc->dwID);
		WritePrivateProfileString(strSrc, "SourceID", strSourceID, strFilename);
			//source ID
		if (lpDesc->dwInstance)
		{
			CString strSourceInstance;
			strSourceInstance.Format("%X", lpDesc->dwInstance);
			WritePrivateProfileString(strSrc, "SourceInstance", strSourceInstance, strFilename);
				//source instance
		}
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterDataSources::Reset(LPCSTR lpFilename)
{
	CString strFilename = lpFilename + CString(".user");

	try
	{
		CFile::Remove(strFilename);
	}
	catch(CFileException* /*exception*/)
	{
	}

	return Init(lpFilename);
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterDataSources::Reinit(LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc)
{
	if (lpDesc->hCounter)
		PdhRemoveCounter(lpDesc->hCounter);
	lpDesc->hCounter = NULL;

	char	szLocalizedObjectName	[MAX_PATH];
	char	szLocalizedInstanceName	[MAX_PATH];
	char	szLocalizedCounterName	[MAX_PATH];

	DWORD	dwInstanceIndex	= lpDesc->dwInstanceIndex;

	m_objects.GetLocalizedName(lpDesc->szObjectName		, szLocalizedObjectName		, sizeof(szLocalizedObjectName));
	m_objects.GetLocalizedName(lpDesc->szInstanceName	, szLocalizedInstanceName	, sizeof(szLocalizedInstanceName));
	m_objects.GetLocalizedName(lpDesc->szCounterName	, szLocalizedCounterName	, sizeof(szLocalizedCounterName));
		//get localized object, instance and counter names

	if (!strlen(szLocalizedInstanceName) && (dwInstanceIndex != -1))
		//if instance name is not specified, then we'll treat specified index as the index in the list of enumerated
		//object's system specific enumerated instance names
	{
		LPCSTR lpInstanceName = m_objects.GetInstance(szLocalizedObjectName, dwInstanceIndex);

		if (lpInstanceName && _stricmp(lpInstanceName, "_Total"))
			//do not allow enumerated _Total instance to be selected by index
		{
			strcpy_s(szLocalizedInstanceName, sizeof(szLocalizedInstanceName), lpInstanceName);

			dwInstanceIndex = -1;
		}
	}

	PDH_COUNTER_PATH_ELEMENTS elements;
	ZeroMemory(&elements, sizeof(elements));

	elements.szMachineName		= NULL;
	elements.szObjectName		= szLocalizedObjectName;
	if (strlen(szLocalizedInstanceName))
		elements.szInstanceName	= szLocalizedInstanceName;
	elements.szParentInstance	= NULL;
	elements.dwInstanceIndex	= dwInstanceIndex;
	elements.szCounterName		= szLocalizedCounterName;
		//prepare counter path elements

	DWORD dwSize = sizeof(lpDesc->szCounterPath);

	PdhMakeCounterPath(&elements, lpDesc->szCounterPath, &dwSize, 0);
		//format full localized counter path

	if (strlen(lpDesc->szCounterPath))
		//try to add counter to query if we succesfully formatted the path
	{
		if (lpDesc->bDynamic || (PdhValidatePath(lpDesc->szCounterPath) == ERROR_SUCCESS))
			//attempts to add a counter with invalid path are VERY SLOW, so we validate the path before trying to add the counter.
			//Dynamic sources (e.g. process specific ones) are the exception, we always try to add them without path validation
			PdhAddCounter(m_hQuery, lpDesc->szCounterPath, 0, &lpDesc->hCounter);
	}
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterDataSources::ReinitSelected()
{
	POSITION pos = GetHeadPosition();

	while (pos)
	{
		LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = GetNext(pos);

		if (lpDesc->bSelected)
			Reinit(lpDesc);
	}
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterDataSources::RemoveSource(LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc)
{
	if (lpDesc)
	{
		POSITION pos = Find(lpDesc);

		if (pos)
		{
			RemoveAt(pos);

			if (lpDesc->hCounter)
				PdhRemoveCounter(lpDesc->hCounter);
			delete lpDesc;

			return TRUE;
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
