// D3DKMTInterface.cpp: implementation of the CD3DKMTInterface class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "D3DKMTInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CD3DKMTInterface::CD3DKMTInterface()
{
	m_hGDI32_DLL							= 0;
	m_pD3DKMTOpenAdapterFromDeviceName		= NULL;
	m_pD3DKMTOpenAdapterFromGdiDisplayName	= NULL;
	m_pD3DKMTCloseAdapter					= NULL;
	m_pD3DKMTQueryStatistics				= NULL;
}
//////////////////////////////////////////////////////////////////////
CD3DKMTInterface::~CD3DKMTInterface()
{
	Uninit();
}
//////////////////////////////////////////////////////////////////////
BOOL CD3DKMTInterface::Init()
{
	Uninit();

	m_hGDI32_DLL = LoadLibrary("GDI32.DLL");

	if (m_hGDI32_DLL)
	{
		m_pD3DKMTOpenAdapterFromDeviceName		= (D3DKMTOPENADAPTERFROMDEVICENAME		)GetProcAddress(m_hGDI32_DLL, "D3DKMTOpenAdapterFromDeviceName");
		m_pD3DKMTOpenAdapterFromGdiDisplayName	= (D3DKMTOPENADAPTERFROMGDIDISPLAYNAME	)GetProcAddress(m_hGDI32_DLL, "D3DKMTOpenAdapterFromGdiDisplayName");
		m_pD3DKMTCloseAdapter					= (D3DKMTCLOSEADAPTER					)GetProcAddress(m_hGDI32_DLL, "D3DKMTCloseAdapter");
		m_pD3DKMTQueryStatistics				= (D3DKMTQUERYSTATISTICS				)GetProcAddress(m_hGDI32_DLL, "D3DKMTQueryStatistics");

		if (m_pD3DKMTOpenAdapterFromDeviceName		&&
			m_pD3DKMTOpenAdapterFromGdiDisplayName	&&
			m_pD3DKMTCloseAdapter					&&
			m_pD3DKMTQueryStatistics)
			return TRUE;
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
void CD3DKMTInterface::Uninit()
{
	if (m_hGDI32_DLL)
		FreeLibrary(m_hGDI32_DLL);

	m_hGDI32_DLL						= 0;
	m_pD3DKMTOpenAdapterFromDeviceName		= NULL;
	m_pD3DKMTOpenAdapterFromGdiDisplayName	= NULL;
	m_pD3DKMTCloseAdapter					= NULL;
}
//////////////////////////////////////////////////////////////////////
MYNTSTATUS CD3DKMTInterface::D3DKMTOpenAdapterFromDeviceName(D3DKMT_OPENADAPTERFROMDEVICENAME *pData)
{
	if (m_pD3DKMTOpenAdapterFromDeviceName)
		return m_pD3DKMTOpenAdapterFromDeviceName(pData);

	return STATUS_NOT_IMPLEMENTED;
}
//////////////////////////////////////////////////////////////////////
MYNTSTATUS CD3DKMTInterface::D3DKMTOpenAdapterFromGdiDisplayName(D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME *pData)
{
	if (m_pD3DKMTOpenAdapterFromGdiDisplayName)
		return m_pD3DKMTOpenAdapterFromGdiDisplayName(pData);

	return STATUS_NOT_IMPLEMENTED;
}
//////////////////////////////////////////////////////////////////////
MYNTSTATUS CD3DKMTInterface::D3DKMTCloseAdapter(D3DKMT_CLOSEADAPTER *pData)
{
	if (m_pD3DKMTCloseAdapter)
		return m_pD3DKMTCloseAdapter(pData);

	return STATUS_NOT_IMPLEMENTED;
}
//////////////////////////////////////////////////////////////////////
MYNTSTATUS CD3DKMTInterface::D3DKMTQueryStatistics(LPVOID pData)
{
	if (m_pD3DKMTQueryStatistics)
		return m_pD3DKMTQueryStatistics(pData);

	return STATUS_NOT_IMPLEMENTED;
}
//////////////////////////////////////////////////////////////////////
