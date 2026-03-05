// AIDA64App.h: interface for the CAIDA64App class.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
class CAIDA64App
{
public:
	CString GetInstallPath();

	BOOL	Spawn(DWORD dwSpawnPeriod);
	BOOL	IsSpawned();
	BOOL	IsRunning();
	BOOL	Close();

	CAIDA64App();
	virtual ~CAIDA64App();

private:
	CString m_strInstallPath;
	DWORD	m_dwSpawnTimestamp;
	BOOL	m_bSpawned;
};
//////////////////////////////////////////////////////////////////////
