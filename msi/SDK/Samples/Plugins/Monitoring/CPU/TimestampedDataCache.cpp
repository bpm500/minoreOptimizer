// TimestampedDataCache.cpp: implementation file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "TimestampedDataCache.h"
/////////////////////////////////////////////////////////////////////////////
CTimestampedDataCache::CTimestampedDataCache()
{
}
/////////////////////////////////////////////////////////////////////////////
CTimestampedDataCache::~CTimestampedDataCache()
{
	Destroy();
}
/////////////////////////////////////////////////////////////////////////////
LPTIMESTAMPED_DATA CTimestampedDataCache::SetData(DWORD dwKey, DWORD dwTimestamp, DWORD dwData)
{
	LPTIMESTAMPED_DATA lpData = NULL;

	if (!Lookup(dwKey, lpData))
	{
		lpData = (LPTIMESTAMPED_DATA) new BYTE[sizeof(TIMESTAMPED_DATA)];

		SetAt(dwKey, lpData);
	}

	lpData->dwData		= dwData;
	lpData->dwTimestamp	= dwTimestamp;

	return lpData;
}
/////////////////////////////////////////////////////////////////////////////
LPTIMESTAMPED_DATA CTimestampedDataCache::GetData(DWORD dwKey, DWORD dwTimestamp)
{
	LPTIMESTAMPED_DATA lpData = NULL;

	Lookup(dwKey, lpData);

	if (lpData && (lpData->dwTimestamp == dwTimestamp))
		return lpData;

	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
void CTimestampedDataCache::Destroy()
{
	POSITION pos = GetStartPosition();

	while (pos)
	{
		DWORD				dwKey	= 0;
		LPTIMESTAMPED_DATA	lpData	= NULL;

		GetNextAssoc(pos, dwKey, lpData);

		if (lpData)
			delete [] lpData;
	}
}
/////////////////////////////////////////////////////////////////////////////
