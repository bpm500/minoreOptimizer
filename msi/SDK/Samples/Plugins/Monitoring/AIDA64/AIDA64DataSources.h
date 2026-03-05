// AIDA64DataSources.h: interface for the CAIDA64DataSources class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#ifndef _AIDA64DATASOURCES_H_INCLUDED_
#define _AIDA64DATASOURCES_H_INCLUDED_
//////////////////////////////////////////////////////////////////////
#include "AIDA64App.h"
#include "AIDA64Parser.h"

#include <afxtempl.h>
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
typedef struct AIDA64_DATA_SOURCE_DESC
{
	char	szID[MAX_PATH];
		//internal AIDA64 source ID (e.g. TCPU)

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
	FLOAT	fltGridDim;
		//grid dimension 

	FLOAT	fltData;
		//source data

	DWORD	dwID;
		//source ID, MONITORING_SOURCE_ID_... 
	DWORD	dwInstance;
		//zero based source instance index, e.g. 0 for "GPU1 temperature" in multiGPU system, 1 for "GPU2 temperature" in multiGPU system etc

	BOOL bSelected;
		//internal source selection marker for GUI

} AIDA64_DATA_SOURCE_DESC, *LPAIDA64_DATA_SOURCE_DESC;
//////////////////////////////////////////////////////////////////////
class CAIDA64DataSources : public CList<LPAIDA64_DATA_SOURCE_DESC,LPAIDA64_DATA_SOURCE_DESC>,
						   public CAIDA64Parser
{
public:
	BOOL						Init(LPCSTR lpFilename);
	BOOL						Reset(LPCSTR lpFilename);
	BOOL						Save(LPCSTR lpFilename);
	void						Uninit();
	void						InvalidateSources();
	BOOL						RemoveSource(LPAIDA64_DATA_SOURCE_DESC lpDesc);

	LPAIDA64_DATA_SOURCE_DESC	GetSourceDesc(DWORD dwSource);
	FLOAT						GetSourceData(DWORD dwSource);

	virtual void				ParseSensor(LPCSTR lpType, LPCSTR lpID, LPCSTR lpLabel, LPCSTR lpValue, DWORD dwContext);

	CAIDA64DataSources();
	virtual ~CAIDA64DataSources();

private:
	CAIDA64App					m_hostApp;

	BOOL						UpdateSource(LPCSTR lpID, LPCSTR lpValue);
	void						UpdateSources();
};
//////////////////////////////////////////////////////////////////////
#endif 
//////////////////////////////////////////////////////////////////////
