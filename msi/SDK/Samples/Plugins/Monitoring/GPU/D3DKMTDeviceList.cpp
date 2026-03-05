// D3DKMTDeviceList.cpp: implementation of the CD3DKMTDeviceList class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "D3DKMTDeviceList.h"
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
#include <setupapi.h>
#include <devguid.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CD3DKMTDeviceList::CD3DKMTDeviceList()
{
}
//////////////////////////////////////////////////////////////////////
CD3DKMTDeviceList::~CD3DKMTDeviceList()
{
	Uninit();
}
//////////////////////////////////////////////////////////////////////
void CD3DKMTDeviceList::Uninit()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		delete GetNext(pos);

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
BOOL CD3DKMTDeviceList::Init()
{
	Uninit();

	//pass 1 : enumerate device names

	GUID GUID_DISPLAY_DEVICE_ARRIVAL = { 0x1CA05180, 0xA699, 0x450A, 0x9A, 0x0C, 0xDE, 0x4F, 0xBE, 0x3D, 0xDD, 0x89 };

	HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DISPLAY_DEVICE_ARRIVAL, 0, 0, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
		    
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return FALSE;

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData; 
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	for (DWORD dwInterface=0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DISPLAY_DEVICE_ARRIVAL, dwInterface, &DeviceInterfaceData); dwInterface++)
	{
		DWORD dwRequiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, NULL, 0, &dwRequiredSize, NULL);

		if (dwRequiredSize)
		{
			PSP_DEVICE_INTERFACE_DETAIL_DATA lpDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new BYTE[dwRequiredSize];
			lpDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, lpDeviceInterfaceDetailData, dwRequiredSize, NULL, NULL))
			{
				LPD3DKMTDEVICE_DESC lpDesc = new D3DKMTDEVICE_DESC;	
				ZeroMemory(lpDesc, sizeof(D3DKMTDEVICE_DESC));

				strcpy_s(lpDesc->DeviceName, sizeof(lpDesc->DeviceName), lpDeviceInterfaceDetailData->DevicePath);

				AddTail(lpDesc);
			}

			delete [] lpDeviceInterfaceDetailData;
		}
	}

    SetupDiDestroyDeviceInfoList(hDevInfo);

	//pass 2 : reconcile device names with physical devices and locations

	hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_DISPLAY, 0, 0, DIGCF_PRESENT);
	
	SP_DEVINFO_DATA DeviceInfoData;	
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD dwPhysicalDevice=0; SetupDiEnumDeviceInfo(hDevInfo, dwPhysicalDevice, &DeviceInfoData); dwPhysicalDevice++)
    {
		DWORD	Bus;
		DWORD	DevFn;

		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, 0x15/*SPDRP_BUSNUMBER*/	, NULL, (LPBYTE)&Bus	, 4, NULL) &&
			SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, 0x1C/*SPDRP_ADDRESS*/	, NULL, (LPBYTE)&DevFn	, 4, NULL))
		{
			DWORD dwRequiredSize = 0;
			SetupDiGetDeviceInstanceId(hDevInfo, &DeviceInfoData, NULL, 0, &dwRequiredSize);

			if (dwRequiredSize)
			{
				LPSTR lpInstanceID = new char[dwRequiredSize];
					//allocate buffer for instance ID

				if (SetupDiGetDeviceInstanceId(hDevInfo, &DeviceInfoData, lpInstanceID, dwRequiredSize, 0))
				{
					CString strInstanceID = lpInstanceID;

					strInstanceID.MakeUpper();

					POSITION pos = GetHeadPosition();

					while (pos)
					{
						LPD3DKMTDEVICE_DESC lpDesc = GetNext(pos);

						CString strDeviceName = lpDesc->DeviceName;

						strDeviceName.MakeUpper();
						strDeviceName.Replace("#", "\\");

						if (strDeviceName.Find(strInstanceID) != -1)
						{
							lpDesc->Bus	= Bus;

							if (DevFn >= 0x10000)
								lpDesc->Dev	= DevFn>>16;
							else
								lpDesc->Dev	= DevFn>>3;

							lpDesc->Fn	= DevFn & 7;
							break;
						}
					}
				}

				delete [] lpInstanceID;
			}
		}
	}

    SetupDiDestroyDeviceInfoList(hDevInfo);

	return GetCount() != 0;
}
//////////////////////////////////////////////////////////////////////

