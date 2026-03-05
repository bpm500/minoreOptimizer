// AIDA64DataSources.cpp: implementation of the CAIDA64DataSources class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AIDA64DataSources.h"
#include "MAHMSharedMemory.h"
//////////////////////////////////////////////////////////////////////
#include <float.h>
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
extern DWORD	g_dwSpawn;
extern DWORD	g_dwSpawnPeriod;
//////////////////////////////////////////////////////////////////////
CAIDA64DataSources::CAIDA64DataSources()
{
}
//////////////////////////////////////////////////////////////////////
CAIDA64DataSources::~CAIDA64DataSources()
{
	Uninit();
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64DataSources::Init(LPCSTR lpFilename)
{
	CString strFilename			= lpFilename;
	CString strFilenameUser		= lpFilename + CString(".user");

	if (!_taccess(strFilenameUser, 0))
		strFilename = strFilenameUser;

	Uninit();

	char szBuf[MAX_PATH];
		//local string buffer

	for (DWORD dwSrc=0; dwSrc<INT_MAX; dwSrc++)
		//loop through all sources
	{
		CString strSrc;	strSrc.Format("Source%d", dwSrc);
			//format source section name

		GetPrivateProfileString(strSrc, "ID", "", szBuf, MAX_PATH, strFilename);
			//read source ID from .cfg
		if (!strlen(szBuf))
			//break the loop if source doesn't exist (i.e. ID is empty)
			break;

		LPAIDA64_DATA_SOURCE_DESC lpDesc = new AIDA64_DATA_SOURCE_DESC; ZeroMemory(lpDesc, sizeof(AIDA64_DATA_SOURCE_DESC));
			//allocate and zap new source descriptor

		strcpy_s(lpDesc->szID, sizeof(lpDesc->szID), szBuf);
			//init ID

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
			//reset source data

		lpDesc->dwID = MONITORING_SOURCE_ID_PLUGIN_MISC;
			//init default source ID

		GetPrivateProfileString(strSrc, "SourceID"	, "", szBuf				, sizeof(szBuf)				, strFilename);
			//read source ID from .cfg
		sscanf_s(szBuf, "%X", &lpDesc->dwID);
			//scan source ID string and convert it to DWORD
		GetPrivateProfileString(strSrc, "SourceInstance", "", szBuf			, sizeof(szBuf)				, strFilename);
			//read source instance index from .cfg
		sscanf_s(szBuf, "%X", &lpDesc->dwInstance);
			//scan source instance index string and convert it to DWORD
		
		AddTail(lpDesc);
			//add descriptor to the list
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CAIDA64DataSources::Uninit()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		delete GetNext(pos);

	RemoveAll();

	if (m_hostApp.IsSpawned() && m_hostApp.IsRunning())
		//close AIDA64 if we spawned it before
	{
		m_hostApp.Close();
	}
}
//////////////////////////////////////////////////////////////////////
void CAIDA64DataSources::ParseSensor(LPCSTR lpType, LPCSTR lpID, LPCSTR lpLabel, LPCSTR lpValue, DWORD dwContext)
{
	UpdateSource(lpID, lpValue);
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64DataSources::UpdateSource(LPCSTR lpID, LPCSTR lpValue)
{
	BOOL bResult = FALSE;

	POSITION pos = GetHeadPosition();

	while (pos)
		//loop through all sources and find desired source by ID
	{
		LPAIDA64_DATA_SOURCE_DESC lpDesc = GetNext(pos);
			//get current source descriptor

		if (!_stricmp(lpID, lpDesc->szID))
			//compare source ID with specified one
		{
			if (sscanf_s(lpValue, "%f", &lpDesc->fltData) == 1)
				//scan value string and store FP data into source descriptor
				bResult = TRUE;
					//do not return immediately to allow updating multiple sources mapped to the same sensor
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////
void CAIDA64DataSources::UpdateSources()
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "AIDA64_SensorValues");
		//try to open AIDA64 shared memory mapping file

	if (hMapFile)
	{
		LPCSTR pMapAddr = (LPCSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			//try to map view of file to get access to AIDA64 XML-styled shared memory
		
		if (pMapAddr)
		{
			CString strCache = pMapAddr;
				//precache AIDA64 XML-styled shared memory

			ParseXML(strCache);
				//parse precached AIDA64 XML-styled shared memory

			UnmapViewOfFile(pMapAddr);
		}
		else
			InvalidateSources();
				//invalidate sources if we cannot map view of file (AIDA64 is shutting down)

		CloseHandle(hMapFile);
	}
	else
		InvalidateSources();
			//invalidate sources if we cannot map view of file (AIDA64 is not running)

	if (g_dwSpawn)
		//if spawning is enabled, we'll try to spawn AIDA64 process if it is installed
	{
		if (!m_hostApp.IsRunning())
		{
			m_hostApp.Spawn(g_dwSpawnPeriod);
		}

		if (g_dwSpawn > 1)
			//respawn mode, we'll try to respawn AIDA64 on each iteration
		{
		}
		else
			//oneshot spawn mode
			g_dwSpawn = 0;
	}
}
//////////////////////////////////////////////////////////////////////
LPAIDA64_DATA_SOURCE_DESC CAIDA64DataSources::GetSourceDesc(DWORD dwSource)
{
	POSITION pos = FindIndex(dwSource);

	if (pos)
		return GetAt(pos);

	return NULL;
}
//////////////////////////////////////////////////////////////////////
FLOAT CAIDA64DataSources::GetSourceData(DWORD dwSource)
{
	LPAIDA64_DATA_SOURCE_DESC lpDesc = GetSourceDesc(dwSource);
		//try to find source descriptor by index

	if (lpDesc)
	{
		if (lpDesc->fltData != FLT_MAX)
			//check if source data is valid 
		{
			FLOAT fltData	= lpDesc->fltData;
			lpDesc->fltData = FLT_MAX;
				//reset data to force it to be reread from AIDA64 on the next call
			return fltData;
				//return source data
		}
		else
		{
			UpdateSources();
				//if source data was invalid, we'll try reread it from AIDA64

			if (lpDesc->fltData != FLT_MAX)
				//check if source data became valid 
			{
				FLOAT fltData	= lpDesc->fltData;
				lpDesc->fltData = FLT_MAX;
					//reset data to force it to be reread from AIDA64 on the next call
				return fltData;
					//return source data
			}
		}
	}

	return FLT_MAX;
		//data is invalid, AIDA64 is not loaded or the source is not supported by hardware
}
//////////////////////////////////////////////////////////////////////
void CAIDA64DataSources::InvalidateSources()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		//loop through all sources
	{
		LPAIDA64_DATA_SOURCE_DESC lpDesc = GetNext(pos);
			//get current source descriptor

		lpDesc->fltData = FLT_MAX;
			//reset source data
	}
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64DataSources::Save(LPCSTR lpFilename)
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
		LPAIDA64_DATA_SOURCE_DESC lpDesc = GetNext(pos);
			//get source descriptor

		CString strSrc;	strSrc.Format("Source%d", iSource++);
			//format source section name

		WritePrivateProfileString(strSrc, "ID", lpDesc->szID, strFilename);
			//sensor ID

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
		CString strSourceID;
		strSourceID.Format("%X", lpDesc->dwID);
		WritePrivateProfileString(strSrc, "SourceID", strSourceID, strFilename);
			//source ID
		if (lpDesc->dwInstance)
		{
			CString strSourceInstance;
			strSourceInstance.Format("%X", lpDesc->dwInstance);
			WritePrivateProfileString(strSrc, "SourceInstance", strSourceInstance, strFilename);
		}
			//source instance
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64DataSources::Reset(LPCSTR lpFilename)
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
BOOL CAIDA64DataSources::RemoveSource(LPAIDA64_DATA_SOURCE_DESC lpDesc)
{
	if (lpDesc)
	{
		POSITION pos = Find(lpDesc);

		if (pos)
		{
			RemoveAt(pos);

			return TRUE;
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
