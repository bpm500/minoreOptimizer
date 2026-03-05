// AIDA64App.cpp: implementation of the CAIDA64App class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AIDA64App.h"

#include <io.h>
//////////////////////////////////////////////////////////////////////
CAIDA64App::CAIDA64App()
{
	m_strInstallPath	= "";
	m_dwSpawnTimestamp	= 0;
	m_bSpawned			= FALSE;
}
//////////////////////////////////////////////////////////////////////
CAIDA64App::~CAIDA64App()
{
}
//////////////////////////////////////////////////////////////////////
CString CAIDA64App::GetInstallPath()
{
	//init AIDA64 installation path

	if (m_strInstallPath.IsEmpty())
	{
		HKEY hKey;

		if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\AIDA64 Extreme_is1", &hKey))
		{
			char buf[MAX_PATH];

			DWORD dwSize = MAX_PATH;
			DWORD dwType;

			if (ERROR_SUCCESS == RegQueryValueEx(hKey, "InstallLocation", 0, &dwType, (LPBYTE)buf, &dwSize))
			{
				if (dwType == REG_SZ)
				{
					m_strInstallPath = buf;
					m_strInstallPath += "aida64.exe";
				}
			}

			RegCloseKey(hKey);
		}
	}

	//validate AIDA64 installation path

	if (_taccess(m_strInstallPath, 0))
		m_strInstallPath = "";

	return m_strInstallPath;
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64App::Spawn(DWORD dwSpawnPeriod)
{
	CString strInstallPath = GetInstallPath();
		//get AIDA64 installation path

	if (!strInstallPath.IsEmpty())
		//spawn AIDA64 if installation path was detected
	{
		DWORD dwTimestamp = GetTickCount();

		if (dwTimestamp > m_dwSpawnTimestamp + dwSpawnPeriod)
			//give AIDA64 some time to start, do not try to respawn the process during spawn period
		{
			if (ShellExecute(NULL, "open", strInstallPath, NULL, NULL, SW_SHOWNORMAL) > (HINSTANCE)32)
				m_bSpawned = TRUE;

			m_dwSpawnTimestamp = dwTimestamp;
		}

		return TRUE;
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64App::IsRunning()
{
	return (FindWindow("TAIDA64", NULL) != NULL);
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64App::IsSpawned()
{
	return m_bSpawned;
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64App::Close()
{
	//RESERVED FOR FUTURE: currently AIDA64 doesn't provide an interface 
	//for closing its running instance

/*
	CString strInstallPath = GetInstallPath();

	if (!strInstallPath.IsEmpty())
	{
		DWORD dwTimestamp = GetTickCount();

		ShellExecute(NULL, "open", strInstallPath, "-exit", NULL, SW_SHOWNORMAL);

		return TRUE;
	}
*/

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
