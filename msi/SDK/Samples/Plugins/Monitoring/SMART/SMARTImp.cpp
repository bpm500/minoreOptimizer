// SMARTImp.cpp: implementation of the CSMART class.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "SMARTImp.h"

#include <float.h>
#include <winioctl.h>
/////////////////////////////////////////////////////////////////////////////
//
// S.M.A.R.T. related constants and structures required to read attributes
//
/////////////////////////////////////////////////////////////////////////////
#define READ_ATTRIBUTE_BUFFER_SIZE		512
/////////////////////////////////////////////////////////////////////////////
#define DFP_RECEIVE_DRIVE_DATA			0x0007c088
/////////////////////////////////////////////////////////////////////////////
#define IDE_EXECUTE_SMART_FUNCTION		0xB0
/////////////////////////////////////////////////////////////////////////////
#define	SMART_READ_ATTRIBUTE_VALUES		0xD0
/////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
/////////////////////////////////////////////////////////////////////////////
typedef struct SENDCMDOUTPARAMS_
{
   DWORD				BufferSize; 
   DRIVERSTATUS			DriverStatus;
} SENDCMDOUTPARAMS_, *LPSENDCMDOUTPARAMS_;
/////////////////////////////////////////////////////////////////////////////
typedef struct  DRIVEATTRIBUTEHDR
{
    WORD				Revision;   
} DRIVEATTRIBUTEHDR, *LPDRIVEATTRIBUTEHDR;
/////////////////////////////////////////////////////////////////////////////
typedef struct  DRIVEATTRIBUTE
{
    BYTE				AttrID;
    WORD				StatusFlags;
    BYTE				AttrValue;  
    BYTE				WorstValue; 
    BYTE				RawValue[6];
    BYTE				Reserved;   
} DRIVEATTRIBUTE, *LPDRIVEATTRIBUTE;
/////////////////////////////////////////////////////////////////////////////
typedef struct READATTRIBUTEBUFFER
{
	DRIVEATTRIBUTEHDR	Hdr;
	DRIVEATTRIBUTE		Attr[30];
	BYTE				Reserved[150];
} READATTRIBUTEBUFFER, *LPREADATTRIBUTEBUFFER;
/////////////////////////////////////////////////////////////////////////////
typedef struct SENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES
{
	SENDCMDOUTPARAMS_	SendCmdOutParams;
	READATTRIBUTEBUFFER	ReadAttributeBuffer;
} SENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES, *LPSENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES;
/////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)
/////////////////////////////////////////////////////////////////////////////
//
// NVMe related constants and structures required to read attributes
//
/////////////////////////////////////////////////////////////////////////////
#define NVME_MAX_LOG_SIZE				4096
/////////////////////////////////////////////////////////////////////////////
typedef enum 
{
    NVME_LOG_PAGE_ERROR_INFO                    = 0x01,
    NVME_LOG_PAGE_HEALTH_INFO                   = 0x02,
    NVME_LOG_PAGE_FIRMWARE_SLOT_INFO            = 0x03,
    NVME_LOG_PAGE_CHANGED_NAMESPACE_LIST        = 0x04,
    NVME_LOG_PAGE_COMMAND_EFFECTS               = 0x05,
    NVME_LOG_PAGE_DEVICE_SELF_TEST              = 0x06,
    NVME_LOG_PAGE_TELEMETRY_HOST_INITIATED      = 0x07,
    NVME_LOG_PAGE_TELEMETRY_CTLR_INITIATED      = 0x08,

    NVME_LOG_PAGE_RESERVATION_NOTIFICATION      = 0x80,
    NVME_LOG_PAGE_SANITIZE_STATUS               = 0x81,

} NVME_LOG_PAGES;
/////////////////////////////////////////////////////////////////////////////
typedef struct 
{
    union 
	{
		struct 
		{
            UCHAR   AvailableSpaceLow   : 1;                    // If set to 1, then the available spare space has fallen below the threshold.
            UCHAR   TemperatureThreshold : 1;                   // If set to 1, then a temperature is above an over temperature threshold or below an under temperature threshold.
            UCHAR   ReliabilityDegraded : 1;                    // If set to 1, then the device reliability has been degraded due to significant media related  errors or any internal error that degrades device reliability.
            UCHAR   ReadOnly            : 1;                    // If set to 1, then the media has been placed in read only mode
            UCHAR   VolatileMemoryBackupDeviceFailed    : 1;    // If set to 1, then the volatile memory backup device has failed. This field is only valid if the controller has a volatile memory backup solution.
            UCHAR   Reserved                            : 3;
        } DUMMYSTRUCTNAME;
        
        UCHAR AsUchar;
        
    } CriticalWarning;    // This field indicates critical warnings for the state of the  controller. Each bit corresponds to a critical warning type; multiple bits may be set.

    UCHAR   Temperature[2];                 // Temperature: Contains the temperature of the overall device (controller and NVM included) in units of Kelvin. If the temperature exceeds the temperature threshold, refer to section 5.12.1.4, then an asynchronous event completion may occur
    UCHAR   AvailableSpare;                 // Available Spare:  Contains a normalized percentage (0 to 100%) of the remaining spare capacity available
    UCHAR   AvailableSpareThreshold;        // Available Spare Threshold:  When the Available Spare falls below the threshold indicated in this field, an asynchronous event  completion may occur. The value is indicated as a normalized percentage (0 to 100%).
    UCHAR   PercentageUsed;                 // Percentage Used
    UCHAR   Reserved0[26];

    UCHAR   DataUnitRead[16];               // Data Units Read:  Contains the number of 512 byte data units the host has read from the controller; this value does not include metadata. This value is reported in thousands (i.e., a value of 1 corresponds to 1000 units of 512 bytes read)  and is rounded up.  When the LBA size is a value other than 512 bytes, the controller shall convert the amount of data read to 512 byte units. For the NVM command set, logical blocks read as part of Compare and Read operations shall be included in this value
    UCHAR   DataUnitWritten[16];            // Data Units Written: Contains the number of 512 byte data units the host has written to the controller; this value does not include metadata. This value is reported in thousands (i.e., a value of 1 corresponds to 1000 units of 512 bytes written)  and is rounded up.  When the LBA size is a value other than 512 bytes, the controller shall convert the amount of data written to 512 byte units. For the NVM command set, logical blocks written as part of Write operations shall be included in this value. Write Uncorrectable commands shall not impact this value.
    UCHAR   HostReadCommands[16];           // Host Read Commands:  Contains the number of read commands  completed by  the controller. For the NVM command set, this is the number of Compare and Read commands. 
    UCHAR   HostWrittenCommands[16];        // Host Write Commands:  Contains the number of write commands  completed by  the controller. For the NVM command set, this is the number of Write commands.
    UCHAR   ControllerBusyTime[16];         // Controller Busy Time:  Contains the amount of time the controller is busy with I/O commands. The controller is busy when there is a command outstanding to an I/O Queue (specifically, a command was issued via an I/O Submission Queue Tail doorbell write and the corresponding  completion queue entry  has not been posted yet to the associated I/O Completion Queue). This value is reported in minutes.
    UCHAR   PowerCycle[16];                 // Power Cycles: Contains the number of power cycles.
    UCHAR   PowerOnHours[16];               // Power On Hours: Contains the number of power-on hours. This does not include time that the controller was powered and in a low power state condition.
    UCHAR   UnsafeShutdowns[16];            // Unsafe Shutdowns: Contains the number of unsafe shutdowns. This count is incremented when a shutdown notification (CC.SHN) is not received prior to loss of power.
    UCHAR   MediaErrors[16];                // Media Errors:  Contains the number of occurrences where the controller detected an unrecovered data integrity error. Errors such as uncorrectable ECC, CRC checksum failure, or LBA tag mismatch are included in this field.
    UCHAR   ErrorInfoLogEntryCount[16];     // Number of Error Information Log Entries:  Contains the number of Error Information log entries over the life of the controller
    ULONG   WarningCompositeTemperatureTime;     // Warning Composite Temperature Time: Contains the amount of time in minutes that the controller is operational and the Composite Temperature is greater than or equal to the Warning Composite Temperature Threshold (WCTEMP) field and less than the Critical Composite Temperature Threshold (CCTEMP) field in the Identify Controller data structure
    ULONG   CriticalCompositeTemperatureTime;    // Critical Composite Temperature Time: Contains the amount of time in minutes that the controller is operational and the Composite Temperature is greater the Critical Composite Temperature Threshold (CCTEMP) field in the Identify Controller data structure
    USHORT  TemperatureSensor1;          // Contains the current temperature reported by temperature sensor 1.
    USHORT  TemperatureSensor2;          // Contains the current temperature reported by temperature sensor 2.
    USHORT  TemperatureSensor3;          // Contains the current temperature reported by temperature sensor 3.
    USHORT  TemperatureSensor4;          // Contains the current temperature reported by temperature sensor 4.
    USHORT  TemperatureSensor5;          // Contains the current temperature reported by temperature sensor 5.
    USHORT  TemperatureSensor6;          // Contains the current temperature reported by temperature sensor 6.
    USHORT  TemperatureSensor7;          // Contains the current temperature reported by temperature sensor 7.
    USHORT  TemperatureSensor8;          // Contains the current temperature reported by temperature sensor 8.
    UCHAR   Reserved1[296];

} NVME_HEALTH_INFO_LOG, *PNVME_HEALTH_INFO_LOG;
/////////////////////////////////////////////////////////////////////////////
CSMART::CSMART()
{
	m_dwPollingInterval	= 0;

	for (DWORD dwDrive=0; dwDrive< MAX_DRIVE; dwDrive++)
	{
		m_dwCaps[dwDrive]				= 0;
		m_dwTimestamp[dwDrive]			= 0;

		for (DWORD dwSensor=0; dwSensor<MAX_SENSOR; dwSensor++)
			m_fltTemperature[dwDrive][dwSensor]	= FLT_MAX;
	}
}
//////////////////////////////////////////////////////////////////////
CSMART::~CSMART()
{
}
//////////////////////////////////////////////////////////////////////
float CSMART::GetTemperatureIDE(DWORD dwDrive)
{
	//format drive name

	char szDrive[MAX_PATH];
	sprintf_s(szDrive, "\\\\.\\PhysicalDrive%d", dwDrive);

	//try to get drive handle and return error on failure

	HANDLE hDrive = CreateFile (szDrive, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive == INVALID_HANDLE_VALUE)
		return FLT_MAX;

	//declare and init input IOCTL parameters

	SENDCMDINPARAMS inp;
	memset(&inp, 0, sizeof(inp));

	inp.cBufferSize						= READ_ATTRIBUTE_BUFFER_SIZE;
	inp.irDriveRegs.bFeaturesReg		= SMART_READ_ATTRIBUTE_VALUES;
	inp.irDriveRegs.bSectorCountReg		= 1;
	inp.irDriveRegs.bSectorNumberReg	= 1;
	inp.irDriveRegs.bCylLowReg			= SMART_CYL_LOW;
	inp.irDriveRegs.bCylHighReg			= SMART_CYL_HI;
	inp.irDriveRegs.bDriveHeadReg		= 0xA0 | ((dwDrive & 1)<<4);
	inp.irDriveRegs.bCommandReg			= IDE_EXECUTE_SMART_FUNCTION;
	inp.bDriveNumber					= (BYTE)dwDrive;

	//declare and init IOCTL output parameters

	SENDCMDOUTPARAMS_SMART_READ_ATTRIBUTE_VALUES out;
	memset(&out, 0, sizeof(out));

	//try to read S.M.A.R.T. attributes via DFP_RECEIVE_DRIVE_DATA IOCL and return error on failure

	DWORD dwBytesReturned;
	if (!DeviceIoControl (hDrive, DFP_RECEIVE_DRIVE_DATA, &inp, sizeof(inp), &out, sizeof(out), &dwBytesReturned, NULL))
	{
		CloseHandle(hDrive);
		return FLT_MAX;
	}

	CloseHandle(hDrive);

	//loop through attributes array and search for temeprature attribute

	for (DWORD dwAttr=0; dwAttr<30; dwAttr++)
	{
		if ((out.ReadAttributeBuffer.Attr[dwAttr].AttrID == 0xC2) ||
			(out.ReadAttributeBuffer.Attr[dwAttr].AttrID == 0xE7) ||
			(out.ReadAttributeBuffer.Attr[dwAttr].AttrID == 0xBE))
			//process temperature attribute
		{
			WORD temp = *((LPWORD)out.ReadAttributeBuffer.Attr[dwAttr].RawValue);
				//get raw temperature (can be either in 1°C or in 0.1°C units depending on hard drive manufacturer)
			
			if (temp > 200)
				//convert 0.1°C based temperature format to 1°C based
				temp = temp / 10;

			return temp;
		}
	}

	return FLT_MAX;
}
//////////////////////////////////////////////////////////////////////
float CSMART::GetTemperatureNVMe(DWORD dwDrive, float* lpTemperature2)
{
	//format drive name

	char szDrive[MAX_PATH];
	sprintf_s(szDrive, "\\\\.\\PhysicalDrive%d", dwDrive);

	//try to get drive handle and return error on failure

	HANDLE hDrive = CreateFile (szDrive, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive == INVALID_HANDLE_VALUE)
		return FLT_MAX;

	//allocate input/output buffer

	DWORD	dwBufferSize	= FIELD_OFFSET(STORAGE_PROPERTY_QUERY, AdditionalParameters) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
	LPBYTE	lpBuffer		= new BYTE[dwBufferSize];

	ZeroMemory(lpBuffer, dwBufferSize);

	//declare and init IOCTL parameters

    PSTORAGE_PROPERTY_QUERY				lpQuery				= (PSTORAGE_PROPERTY_QUERY			)lpBuffer;
    PSTORAGE_PROTOCOL_SPECIFIC_DATA		lpProtocolData		= (PSTORAGE_PROTOCOL_SPECIFIC_DATA	)lpQuery->AdditionalParameters;
    PSTORAGE_PROTOCOL_DATA_DESCRIPTOR	lpProtocolDataDesc	= (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)lpBuffer;

    lpQuery->PropertyId							= StorageDeviceProtocolSpecificProperty;  
    lpQuery->QueryType							= PropertyStandardQuery;  

    lpProtocolData->ProtocolType				= ProtocolTypeNvme;  
    lpProtocolData->DataType					= NVMeDataTypeLogPage;  
    lpProtocolData->ProtocolDataRequestValue	= NVME_LOG_PAGE_HEALTH_INFO;  
    lpProtocolData->ProtocolDataRequestSubValue = 0;  
    lpProtocolData->ProtocolDataOffset			= sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);  
    lpProtocolData->ProtocolDataLength			= sizeof(NVME_HEALTH_INFO_LOG);  

	//try to read attributes via IOCTL_STORAGE_QUERY_PROPERTY IOCL and return error on failure

	DWORD dwBytesReturned;
	if (!DeviceIoControl (hDrive, IOCTL_STORAGE_QUERY_PROPERTY, lpBuffer, dwBufferSize, lpBuffer, dwBufferSize, &dwBytesReturned, NULL))
	{
		CloseHandle(hDrive);
		delete [] lpBuffer;
		return FLT_MAX;
	}

	CloseHandle(hDrive);
	
	//valdiate returned data

	if ((lpProtocolDataDesc->Version	!= sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR)) ||
		(lpProtocolDataDesc->Size		!= sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR))) 
	{
		delete [] lpBuffer;
		return FLT_MAX;
	}

	lpProtocolData = &lpProtocolDataDesc->ProtocolSpecificData;

    if ((lpProtocolData->ProtocolDataOffset < sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA	)) ||
        (lpProtocolData->ProtocolDataLength < sizeof(NVME_HEALTH_INFO_LOG			))) 
	{
		delete [] lpBuffer;
		return FLT_MAX;
    }

	PNVME_HEALTH_INFO_LOG pHealthInfoLog = (PNVME_HEALTH_INFO_LOG)((LPBYTE)lpProtocolData + lpProtocolData->ProtocolDataOffset);

	FLOAT fltTemperature = ((pHealthInfoLog->Temperature[1]<<8) | pHealthInfoLog->Temperature[0]) - 273.0f;
		//convert temperature from Kelvin to Celsius

	if (lpTemperature2)
		*lpTemperature2 = pHealthInfoLog->TemperatureSensor2 ? pHealthInfoLog->TemperatureSensor2 - 273.0f : FLT_MAX;
			//convert secondary temperature from Kelvin to Celsius, if it is supported

	delete [] lpBuffer;

	return fltTemperature;
}
//////////////////////////////////////////////////////////////////////
void CSMART::Init()
{
	for (DWORD dwDrive=0; dwDrive<MAX_DRIVE; dwDrive++)
	{
		m_dwCaps[dwDrive] = 0;

		if (GetTemperatureIDE(dwDrive) != FLT_MAX)
			//physical drive supports temeprature reading via IDE SMART reporting protocol
			m_dwCaps[dwDrive] |= SMART_CAPS_TEMPERATURE_REPORTING_IDE;
		else
		{
			float fltTemperature2 = FLT_MAX;

			if (GetTemperatureNVMe(dwDrive, &fltTemperature2) != FLT_MAX)
				//physical drive supports temperature reading via NVMe SMART reporting protocol
			{
				m_dwCaps[dwDrive] |= SMART_CAPS_TEMPERATURE_REPORTING_NVME;

				if (fltTemperature2 != FLT_MAX)
					//physical drive supports secondary (controller) temperature reading via NVMe SMART reporting protocol
					m_dwCaps[dwDrive] |= SMART_CAPS_TEMPERATURE_REPORTING_NVME_2;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////
DWORD CSMART::GetCaps(DWORD dwDrive)
{
	if (dwDrive < MAX_DRIVE)
		return m_dwCaps[dwDrive];

	return 0;
}
//////////////////////////////////////////////////////////////////////
BOOL CSMART::IsTemperatureReportingSupported(DWORD dwDrive, DWORD dwSensor)
{
	if (dwDrive < MAX_DRIVE)
	{
		switch (dwSensor)
		{
		case 0:
			if (m_dwCaps[dwDrive] & SMART_CAPS_TEMPERATURE_REPORTING_IDE)
				return TRUE;
			if (m_dwCaps[dwDrive] & SMART_CAPS_TEMPERATURE_REPORTING_NVME)
				return TRUE;
			break;
		case 1:
			if (m_dwCaps[dwDrive] & SMART_CAPS_TEMPERATURE_REPORTING_NVME_2)
				return TRUE;
			break;
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
float CSMART::GetTemperature(DWORD dwDrive, DWORD dwSensor)
{
	if ((dwDrive < MAX_DRIVE) && (dwSensor < MAX_SENSOR))

	{
		DWORD dwTimestamp = GetTickCount();

		if (dwTimestamp - m_dwTimestamp[dwDrive] >= m_dwPollingInterval)
		{
			FLOAT fltTemperature	= FLT_MAX;
			FLOAT fltTemperature2	= FLT_MAX;
			
			if (m_dwCaps[dwDrive] & SMART_CAPS_TEMPERATURE_REPORTING_IDE)
				fltTemperature = GetTemperatureIDE(dwDrive);
			else
			if (m_dwCaps[dwDrive] & SMART_CAPS_TEMPERATURE_REPORTING_NVME)
				fltTemperature = GetTemperatureNVMe(dwDrive, &fltTemperature2);

			if (fltTemperature != FLT_MAX)
				m_fltTemperature[dwDrive][0] = fltTemperature;
			if (fltTemperature2 != FLT_MAX)
				m_fltTemperature[dwDrive][1] = fltTemperature2;

			m_dwTimestamp[dwDrive] = dwTimestamp;
		}

		return m_fltTemperature[dwDrive][dwSensor];
	}

	return FLT_MAX;
}
//////////////////////////////////////////////////////////////////////
void CSMART::SetPollingInterval(DWORD dwPollingInterval)
{
	m_dwPollingInterval = dwPollingInterval;
}
//////////////////////////////////////////////////////////////////////
DWORD CSMART::GetPollingInterval()
{
	return m_dwPollingInterval;
}
//////////////////////////////////////////////////////////////////////
