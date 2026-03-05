// PerfCounterDataSources.h: interface for the CPerfCounterDataSources class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#ifndef _PERFCOUNTERDATASOURCES_H_INCLUDED_
#define _PERFCOUNTERDATASOURCES_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include "PerfCounterObjects.h"

#include <afxtempl.h>
#include <pdh.h>
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
typedef struct PERFCOUNTER_DATA_SOURCE_DESC
{
	char	szObjectName[MAX_PATH];
		//English performance counter object name
	char	szInstanceName[MAX_PATH];
		//English performance counter instance name
	DWORD	dwInstanceIndex;
		//direct performance coutner instance index if szInstance is specified, otherwise the index of instance name to be
		//enumerated
	char	szCounterName[MAX_PATH];
		//English performance counter name

	char	szCounterPath[MAX_PATH];
		//full localized performance coutner path
	
	LONG	dwMul;
		//performance counter multiplier
	LONG	dwDiv;
		//performance counter divider
	LONG	dwOff;
		//performance counter offset

	//resulting performance counter value is <offset> + <counter> * <multiplier> / <divider>

	HCOUNTER hCounter;
		//performance counter handle

	BOOL	bDynamic;
		//dynamic mode
	char	szName[MAX_PATH];
		//source name
	char	szUnits[MAX_PATH];
		//source units
	char	szGroup[MAX_PATH];
		//source group
	char	szFormat[MAX_PATH];
		//source output format

	FLOAT	fltMaxLimit;
		//maximum limit
	FLOAT	fltMinLimit;
		//minimum limit

	FLOAT	fltData;
		//source data
	BOOL	bValid;
		//dynamic sources can report no data sometimes (e.g. process specific counters when target process is not running), 
		//so we cannot use (fltData == FLT_MAX) as data validation criteria and need additional flag for that

	DWORD	dwID;
		//source ID, MONITORING_SOURCE_ID_... 
	DWORD	dwInstance;
		//zero based source instance index, e.g. 0 for "GPU1 temperature" in multiGPU system, 1 for "GPU2 temperature" in multiGPU system etc

	BOOL bSelected;
		//internal source selection marker for GUI

} PERFCOUNTER_DATA_SOURCE_DESC, *LPPERFCOUNTER_DATA_SOURCE_DESC;
//////////////////////////////////////////////////////////////////////
class CPerfCounterDataSources : public CList<LPPERFCOUNTER_DATA_SOURCE_DESC,LPPERFCOUNTER_DATA_SOURCE_DESC>
{
public:
	BOOL							Init(LPCSTR lpFilename);
	void							Reinit(LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc);
	void							ReinitSelected();
	BOOL							Reset(LPCSTR lpFilename);
	BOOL							Save(LPCSTR lpFilename);
	void							Uninit();
	void							InvalidateSources();
	BOOL							RemoveSource(LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc);

	LPPERFCOUNTER_DATA_SOURCE_DESC	GetSourceDesc(DWORD dwSource);
	FLOAT							GetSourceData(DWORD dwSource);

	CPerfCounterDataSources();
	virtual ~CPerfCounterDataSources();

private:
	CPerfCounterObjects				m_objects;
	PDH_HQUERY						m_hQuery;

	void							UpdateSources();
};
//////////////////////////////////////////////////////////////////////
#endif 
//////////////////////////////////////////////////////////////////////
