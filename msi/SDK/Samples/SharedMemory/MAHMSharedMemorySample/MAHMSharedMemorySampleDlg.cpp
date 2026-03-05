// MAHMSharedMemorySampleDlg.cpp : implementation file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MAHMSharedMemorySample.h"
#include "MAHMSharedMemorySampleDlg.h"

#include <float.h>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
/////////////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}
/////////////////////////////////////////////////////////////////////////////
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleDlg dialog
/////////////////////////////////////////////////////////////////////////////
CMAHMSharedMemorySampleDlg::CMAHMSharedMemorySampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMAHMSharedMemorySampleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMAHMSharedMemorySampleDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hMapFile			= NULL;
	m_pMapAddr			= NULL;
	m_nTimerID			= 0;

	m_strInstallPath	= "";
}
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMAHMSharedMemorySampleDlg)
	//}}AFX_DATA_MAP
}
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CMAHMSharedMemorySampleDlg, CDialog)
	//{{AFX_MSG_MAP(CMAHMSharedMemorySampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleDlg message handlers
/////////////////////////////////////////////////////////////////////////////
BOOL CMAHMSharedMemorySampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			

	CWnd* pPlaceholder = GetDlgItem(IDC_PLACEHOLDER);

	if (pPlaceholder)
	{
		CRect rect; 
		pPlaceholder->GetClientRect(&rect);

		if (!m_richEditCtrl.Create(WS_VISIBLE|ES_READONLY|ES_MULTILINE|ES_AUTOHSCROLL|WS_HSCROLL|ES_AUTOVSCROLL|WS_VSCROLL, rect, this, 0))
			return FALSE;

		m_font.CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, BALTIC_CHARSET, 0, 0, 0, 0, "Courier New");
		m_richEditCtrl.SetFont(&m_font);
	}

	m_nTimerID = SetTimer(0x1234, 1000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
/////////////////////////////////////////////////////////////////////////////
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
/////////////////////////////////////////////////////////////////////////////
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
/////////////////////////////////////////////////////////////////////////////
HCURSOR CMAHMSharedMemorySampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
/////////////////////////////////////////////////////////////////////////////
// This function is used to connect to MSI Afterburner hardware monitoring
// shared memory
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::Connect()
{
	Disconnect();
		//we must disconnect from the previously connected shared memory before
		//connecting to new one

	m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "MAHMSharedMemory");

	if (m_hMapFile)
		m_pMapAddr = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
}
/////////////////////////////////////////////////////////////////////////////
// This function is used to disconnect from MSI Afterburner hardware monitoring
// shared memory
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::Disconnect()
{
	if (m_pMapAddr)
		UnmapViewOfFile(m_pMapAddr);

	m_pMapAddr = NULL;

	if (m_hMapFile)
		CloseHandle(m_hMapFile);

	m_hMapFile = NULL;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CMAHMSharedMemorySampleDlg::IsSystemSource(DWORD id)
{
	switch (id)
	{
	case MONITORING_SOURCE_ID_CPU_TEMPERATURE:
	case MONITORING_SOURCE_ID_CPU_USAGE:
	case MONITORING_SOURCE_ID_RAM_USAGE:
	case MONITORING_SOURCE_ID_PAGEFILE_USAGE:
	case MONITORING_SOURCE_ID_CPU_CLOCK:
	case MONITORING_SOURCE_ID_FRAMERATE:
	case MONITORING_SOURCE_ID_FRAMETIME:
		return TRUE;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// we'll update sample's window on timer once per second
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::OnTimer(UINT nIDEvent) 
{
	//init MSI Afterburner installation path

	if (m_strInstallPath.IsEmpty())
	{
		HKEY hKey;

		if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\MSI\\Afterburner", &hKey))
		{
			char buf[MAX_PATH];

			DWORD dwSize = MAX_PATH;
			DWORD dwType;

			if (ERROR_SUCCESS == RegQueryValueEx(hKey, "InstallPath", 0, &dwType, (LPBYTE)buf, &dwSize))
			{
				if (dwType == REG_SZ)
					m_strInstallPath = buf;
			}

			RegCloseKey(hKey);
		}
	}

	//validate MSI Afterburner installation path

	if (_taccess(m_strInstallPath, 0))
		m_strInstallPath = "";
	
	//if we're not connected to MSI Afterburner hardware monitoring shared memory yet - do it now

	if (!m_pMapAddr)
		Connect();

	if (m_pMapAddr)
		//if we're connected to shared memory, we must check if it is valid or not and reconnect if necessary
	{
		LPMAHM_SHARED_MEMORY_HEADER lpHeader = (LPMAHM_SHARED_MEMORY_HEADER)m_pMapAddr;

		if (lpHeader->dwSignature == 0xDEAD)
			//if the memory is marked as dead (e.g. MSI Afterburner was unloaded), we should disconnect from it and
			//try to connect again
			Connect();
	}

	if (m_pMapAddr)
	{
		LPMAHM_SHARED_MEMORY_HEADER	lpHeader = (LPMAHM_SHARED_MEMORY_HEADER)m_pMapAddr;

		if (lpHeader->dwSignature == 'MAHM')
			//check if we're connected to valid memory
		{
			CTime time(lpHeader->time);
			CString strBuf = time.Format("%d-%m-%Y %H:%M:%S");
				//format data polling time

			DWORD dwSources = lpHeader->dwNumEntries;
				//get number of data sources

			m_strStatus.Format("Connected to MSI Afterburner Hardware Monitoring shared memory v%d.%d\n\n%d data source(s) have been polled on %s\n\n", lpHeader->dwVersion>>16, lpHeader->dwVersion & 0xffff, dwSources, strBuf);

			if (lpHeader->dwVersion >= 0x00020000)
				//GPU entries are available only in v2.0 and newer shared memory format
			{
				DWORD dwGpus = lpHeader->dwNumGpuEntries;
					//get number of GPUs

				for (DWORD dwGpu=0; dwGpu<dwGpus; dwGpu++)
					//display info for each GPU
				{
					LPMAHM_SHARED_MEMORY_GPU_ENTRY	lpGpuEntry = (LPMAHM_SHARED_MEMORY_GPU_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + lpHeader->dwNumEntries * lpHeader->dwEntrySize + dwGpu * lpHeader->dwGpuEntrySize);
						//get ptr to the current GPU entry

					CString strStatus = DumpGpuEntry(lpGpuEntry);

					for (DWORD dwSource=0; dwSource<dwSources; dwSource++)
						//display info for each data source
					{
						LPMAHM_SHARED_MEMORY_ENTRY	lpEntry		= (LPMAHM_SHARED_MEMORY_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + dwSource * lpHeader->dwEntrySize);
							//get ptr to the current data source entry

						if (IsSystemSource(lpEntry->dwSrcId))
							//filter system data sources
							continue;

						if (lpEntry->dwGpu != dwGpu)
							//filter data source entries by GPU index
							continue;

						strStatus += DumpEntry(lpEntry);
							//dump data source entry
					}

					if (!strStatus.IsEmpty())
					{
						strBuf.Format("GPU%d\n", dwGpu + 1);
							//keep in mind that GPU indices are 0-based, so we need to correct index when
							//formatting the GPU section header 

						m_strStatus += strBuf;
						m_strStatus += strStatus;
						m_strStatus += "\n";
					}
				}

				//additional pass for system data sources

				CString strStatus;

				for (DWORD dwSource=0; dwSource<dwSources; dwSource++)
					//display info for each data source
				{
					LPMAHM_SHARED_MEMORY_ENTRY	lpEntry		= (LPMAHM_SHARED_MEMORY_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + dwSource * lpHeader->dwEntrySize);
						//get ptr to the current data source entry

					if (!IsSystemSource(lpEntry->dwSrcId))
						//filter GPU-specific data sources
						continue;

					strStatus += DumpEntry(lpEntry);
						//dump data source entry
				}

				if (!strStatus.IsEmpty())
				{
					m_strStatus += "Global\n";
					m_strStatus += strStatus;
					m_strStatus += "\n";
				}
			}
			else
			{
				for (DWORD dwSource=0; dwSource<dwSources; dwSource++)
					//display info for each data source
				{
					LPMAHM_SHARED_MEMORY_ENTRY	lpEntry		= (LPMAHM_SHARED_MEMORY_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + dwSource * lpHeader->dwEntrySize);
						//get ptr to the current data source entry

					m_strStatus += DumpEntry(lpEntry);
						//dump data source entry
				}

				m_strStatus += "\n";
			}
		}
		else
			m_strStatus = "Connected to uninitialized MSI Afterburner Hardware Monitoring shared memory\n\n";

		m_strStatus += "Hints:\n-Press <Space> to open MSI Afterburner hardware monitoring properties\n-Press <1>...<5> to load and apply the profiles from the corresponding MSI Afterburner profile slots";
	}
	else
	{
		if (m_strInstallPath.IsEmpty())
			m_strStatus = "Failed to connect to MSI Afterburner Hardware Monitoring shared memory!\n\nHints:\n-Install MSI Afterburner";
		else
			m_strStatus = "Failed to connect to MSI Afterburner Hardware Monitoring shared memory!\n\nHints:\n-Press <Space> to start MSI Afterburner";
	}
	
	UpdateStatus();

	CDialog::OnTimer(nIDEvent);
}
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::OnDestroy() 
{
	if (m_nTimerID)
		KillTimer(m_nTimerID);

	m_nTimerID = NULL;

	MSG msg; 
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

	Disconnect();

	CDialog::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CMAHMSharedMemorySampleDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nProfile = -1;

		switch (pMsg->wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			nProfile = pMsg->wParam - '0';
			break;
		case ' ':
			if (!m_strInstallPath.IsEmpty())
				ShellExecute(GetSafeHwnd(), "open", m_strInstallPath, m_pMapAddr ? "-propertypage2" : "-m", NULL, SW_SHOWNORMAL);
					//if MSI Afterburner is not running yet then start and force it to be minimized to system tray with -m command line switch,
					//otherwise just start it to force running instance it to be open from system tray
			break;
		}

		if (nProfile != -1)
		{
			CString strCmd;
			strCmd.Format("-profile%d", nProfile);

			if (!m_strInstallPath.IsEmpty())
				ShellExecute(GetSafeHwnd(), "open", m_strInstallPath, strCmd, NULL, SW_SHOWNORMAL);
					//launch MSI Afterburner profiles via the command line
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (m_richEditCtrl.GetSafeHwnd())
	{
		m_richEditCtrl.MoveWindow(0, 0, cx, cy);
		m_richEditCtrl.SendMessage(WM_HSCROLL, SB_THUMBPOSITION, 0);
		m_richEditCtrl.SendMessage(WM_VSCROLL, SB_THUMBPOSITION, 0);
	}
}
/////////////////////////////////////////////////////////////////////////////
CString CMAHMSharedMemorySampleDlg::DumpEntry(LPMAHM_SHARED_MEMORY_ENTRY lpEntry)
{
	CString strValue;

	if (lpEntry->data == FLT_MAX)
		strValue = "N/A";
	else
	{
		strValue.Format(lpEntry->szRecommendedFormat, lpEntry->data);
		strValue += " ";
		strValue += lpEntry->szLocalizedSrcUnits;
	}

	CString strEntry;

	strEntry.Format("%-32s\t : %-16s\t", lpEntry->szLocalizedSrcName, strValue);

	CString strProperties;

	if (lpEntry->dwFlags & MAHM_SHARED_MEMORY_ENTRY_FLAG_SHOW_IN_OSD)
	{
		if (!strProperties.IsEmpty())
			strProperties += ", ";

		strProperties += "OSD";
	}

	if (lpEntry->dwFlags & MAHM_SHARED_MEMORY_ENTRY_FLAG_SHOW_IN_LCD)
	{
		if (!strProperties.IsEmpty())
			strProperties += ", ";

		strProperties += "LCD";
	}

	if (lpEntry->dwFlags & MAHM_SHARED_MEMORY_ENTRY_FLAG_SHOW_IN_TRAY)
	{
		if (!strProperties.IsEmpty())
			strProperties += ", ";

		strProperties += "tray";
	}

	if (!strProperties.IsEmpty())
		strProperties = "in " + strProperties;

	strEntry += strProperties;
	strEntry += "\n";

	return strEntry;
}
/////////////////////////////////////////////////////////////////////////////
CString CMAHMSharedMemorySampleDlg::DumpGpuEntry(LPMAHM_SHARED_MEMORY_GPU_ENTRY lpGpuEntry)
{
	CString strEntry;
	CString strBuf;

	strBuf.Format("%-32s\t : %-16s\n", "GUID", lpGpuEntry->szGpuId);
	strEntry += strBuf;

	if (strlen(lpGpuEntry->szDevice))
	{
		CString strDevice;

		if (strlen(lpGpuEntry->szFamily))
			strDevice.Format("%s on %s GPU", lpGpuEntry->szDevice, lpGpuEntry->szFamily);
		else
			strDevice = lpGpuEntry->szDevice;

		strBuf.Format("%-32s\t : %s\n", "Device", strDevice);
		strEntry += strBuf;
	}

	if (strlen(lpGpuEntry->szDriver))
	{
		strBuf.Format("%-32s\t : %s\n", "Driver", lpGpuEntry->szDriver);
		strEntry += strBuf;
	}

	if (strlen(lpGpuEntry->szBIOS))
	{
		strBuf.Format("%-32s\t : %s\n", "BIOS", lpGpuEntry->szBIOS);
		strEntry += strBuf;
	}

	if (lpGpuEntry->dwMemAmount)
	{
		strBuf.Format("%-32s\t : %d MB\n", "On-board memory", lpGpuEntry->dwMemAmount / 1024);
		strEntry += strBuf;
	}



	return strEntry;
}
/////////////////////////////////////////////////////////////////////////////
void CMAHMSharedMemorySampleDlg::UpdateStatus()
{
	m_richEditCtrl.SetRedraw(FALSE);

	SCROLLINFO horzScrollInfo;
	SCROLLINFO vertScrollInfo;

	m_richEditCtrl.GetScrollInfo(SB_HORZ, &horzScrollInfo);
	m_richEditCtrl.GetScrollInfo(SB_VERT, &vertScrollInfo);

	m_richEditCtrl.SetWindowText(m_strStatus);

	m_richEditCtrl.SendMessage(WM_HSCROLL, (horzScrollInfo.nTrackPos<<16) | SB_THUMBPOSITION, 0);
	m_richEditCtrl.SendMessage(WM_VSCROLL, (vertScrollInfo.nTrackPos<<16) | SB_THUMBPOSITION, 0);

	m_richEditCtrl.SetRedraw(TRUE);
	m_richEditCtrl.RedrawWindow();

}
/////////////////////////////////////////////////////////////////////////////
