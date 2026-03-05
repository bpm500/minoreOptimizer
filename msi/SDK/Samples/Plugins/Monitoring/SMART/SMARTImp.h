// SMARTImp.h: interface for the CTextTable class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
#define MAX_DRIVE	16
#define MAX_SENSOR	2
//////////////////////////////////////////////////////////////////////
#define SMART_CAPS_TEMPERATURE_REPORTING_IDE				0x00000001
#define SMART_CAPS_TEMPERATURE_REPORTING_NVME				0x00000002
#define SMART_CAPS_TEMPERATURE_REPORTING_NVME_2				0x00000004
//////////////////////////////////////////////////////////////////////
class CSMART
{
public:
	void	Init();
	DWORD	GetCaps(DWORD dwDrive);
	BOOL	IsTemperatureReportingSupported(DWORD dwDrive, DWORD dwSensor);
	float	GetTemperature(DWORD dwDrive, DWORD dwSensor);

	void	SetPollingInterval(DWORD dwPollingInterval);
	DWORD	GetPollingInterval();

	void	Enable(BOOL bEnable);
	BOOL	IsEnabled();

	CSMART();
	virtual ~CSMART();

protected:
	float GetTemperatureIDE(DWORD dwDrive);
	float GetTemperatureNVMe(DWORD dwDrive, float* lpTemperature2);

	BOOL m_bEnabled;

	DWORD m_dwPollingInterval;
	DWORD m_dwCaps[MAX_DRIVE];
	DWORD m_dwTimestamp[MAX_DRIVE];
	float m_fltTemperature[MAX_DRIVE][MAX_SENSOR];
};
//////////////////////////////////////////////////////////////////////
