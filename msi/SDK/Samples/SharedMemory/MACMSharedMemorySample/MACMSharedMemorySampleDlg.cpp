// MACMSharedMemorySampleDlg.cpp : implementation file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MACMSharedMemorySample.h"
#include "MACMSharedMemorySampleDlg.h"
/////////////////////////////////////////////////////////////////////////////
const UINT WM_MACM_CMD_NOTIFICATION	= ::RegisterWindowMessage(_T("MACMCmdNotification"));
/////////////////////////////////////////////////////////////////////////////
#include <io.h>
/////////////////////////////////////////////////////////////////////////////
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
// CMACMSharedMemorySampleDlg dialog
/////////////////////////////////////////////////////////////////////////////
CMACMSharedMemorySampleDlg::CMACMSharedMemorySampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMACMSharedMemorySampleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMACMSharedMemorySampleDlg)
	m_bSync = FALSE;
	m_strCoreClockCur = _T("");
	m_strCoreClockMax = _T("");
	m_strCoreClockMin = _T("");
	m_strMemoryClockCur = _T("");
	m_strMemoryClockMax = _T("");
	m_strMemoryClockMin = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_hIcon				= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hBrush			= NULL;	

	m_hMapFile			= NULL;
	m_pMapAddr			= NULL;
	m_pSnapshot			= NULL;

	m_nTimerID			= 0;

	m_strInstallPath	= "";
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMACMSharedMemorySampleDlg)
	DDX_Control(pDX, IDC_MEMORY_CLOCK_SLIDER, m_ctrlMemoryClockSlider);
	DDX_Control(pDX, IDC_CORE_CLOCK_SLIDER, m_ctrlCoreClockSlider);
	DDX_Control(pDX, IDC_MASTER_GPU_COMBO, m_ctrlMasterGpuCombo);
	DDX_Check(pDX, IDC_SYNC_CHECK, m_bSync);
	DDX_Text(pDX, IDC_CORE_CLOCK_CUR, m_strCoreClockCur);
	DDX_Text(pDX, IDC_CORE_CLOCK_MAX, m_strCoreClockMax);
	DDX_Text(pDX, IDC_CORE_CLOCK_MIN, m_strCoreClockMin);
	DDX_Text(pDX, IDC_MEMORY_CLOCK_CUR, m_strMemoryClockCur);
	DDX_Text(pDX, IDC_MEMORY_CLOCK_MAX, m_strMemoryClockMax);
	DDX_Text(pDX, IDC_MEMORY_CLOCK_MIN, m_strMemoryClockMin);
	//}}AFX_DATA_MAP
}
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CMACMSharedMemorySampleDlg, CDialog)
	//{{AFX_MSG_MAP(CMACMSharedMemorySampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_APPLY_BUTTON, OnApplyButton)
	ON_BN_CLICKED(IDC_DEFAULTS_BUTTON, OnDefaultsButton)
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, OnRefreshButton)
	ON_WM_CTLCOLOR()
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_MASTER_GPU_COMBO, OnSelchangeMasterGpuCombo)
	ON_BN_CLICKED(IDC_SYNC_CHECK, OnSyncCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CMACMSharedMemorySampleDlg message handlers
/////////////////////////////////////////////////////////////////////////////
BOOL CMACMSharedMemorySampleDlg::OnInitDialog()
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

	Connect();

	m_nTimerID = SetTimer(0x1234, 1000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CMACMSharedMemorySampleDlg::OnPaint() 
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
HCURSOR CMACMSharedMemorySampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
/////////////////////////////////////////////////////////////////////////////
// This function is used to connect to MSI Afterburner Control
// shared memory
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::Connect()
{
	Disconnect();
		//we must disconnect from the previously connected shared memory before
		//connecting to new one

	m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "MACMSharedMemory");

	if (m_hMapFile)
		m_pMapAddr = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	InitControls();
}
/////////////////////////////////////////////////////////////////////////////
// This function is used to disconnect from MSI Afterburner Control
// shared memory
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::Disconnect()
{
	if (m_pMapAddr)
		UnmapViewOfFile(m_pMapAddr);

	m_pMapAddr = NULL;

	if (m_hMapFile)
		CloseHandle(m_hMapFile);

	m_hMapFile = NULL;
}
/////////////////////////////////////////////////////////////////////////////
// we'll update sample's window on timer once per second
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnTimer(UINT nIDEvent) 
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
	
	//if we're not connected to MSI Afterburner Control shared memory yet - do it now

	if (!m_pMapAddr)
		Connect();

	if (m_pMapAddr)
		//if we're connected to shared memory, we must check if it is valid or not and reconnect if necessary
	{
		LPMACM_SHARED_MEMORY_HEADER lpHeader = (LPMACM_SHARED_MEMORY_HEADER)m_pMapAddr;

		if (lpHeader->dwSignature == 0xDEAD)
			//if the memory is marked as dead (e.g. MSI Afterburner was unloaded), we should disconnect from it and
			//try to connect again
			Connect();
	}

	if (m_pMapAddr)
		//if we're connected to shared memory, we must track timestamp changes to detect settings changes from Afterburner side
	{
		LPMACM_SHARED_MEMORY_HEADER	lpHeader = (LPMACM_SHARED_MEMORY_HEADER)m_pMapAddr;

		if (lpHeader->dwSignature == 'MACM')
		{
			if (m_pSnapshot)
			{
				LPMACM_SHARED_MEMORY_HEADER	lpHeaderSnapshot = (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;

				if (lpHeader->time != lpHeaderSnapshot->time)
					InitControls();
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnDestroy() 
{
	if (m_nTimerID)
		KillTimer(m_nTimerID);

	m_nTimerID = NULL;

	MSG msg; 
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

	if (m_hBrush)
	{
		DeleteObject(m_hBrush);

		m_hBrush = NULL;
	}

	Disconnect();
	DestroySnapshot();

	CDialog::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnApplyButton() 
{
	FlushSnapshot();	

	InitControls();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnDefaultsButton() 
{
	if (m_pSnapshot)
	{
		LPMACM_SHARED_MEMORY_HEADER		lpHeader	= (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;
		LPMACM_SHARED_MEMORY_GPU_ENTRY	lpGpuEntry	= (LPMACM_SHARED_MEMORY_GPU_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + lpHeader->dwMasterGpu * lpHeader->dwGpuEntrySize);

		if (lpGpuEntry->dwFlags & MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_CORE_CLOCK)
			lpGpuEntry->dwCoreClockCur = lpGpuEntry->dwCoreClockDef;

		if (lpGpuEntry->dwFlags & MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_MEMORY_CLOCK)
			lpGpuEntry->dwMemoryClockCur = lpGpuEntry->dwMemoryClockDef;
	}

	OnApplyButton();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnRefreshButton() 
{
	if (!m_pMapAddr)
	{
		if (!m_strInstallPath.IsEmpty())
			ShellExecute(GetSafeHwnd(), "open", m_strInstallPath, "-m", NULL, SW_SHOWNORMAL);
	}

	InitControls();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::UpdateControls()
{
	DWORD commonControls[] =
	{
		IDC_MASTER_GPU_COMBO,
		IDC_SYNC_CHECK,
		IDC_DEFAULTS_BUTTON,
		IDC_APPLY_BUTTON,
		0
	};

	DWORD coreClockControls[] = 
	{
		IDC_CORE_CLOCK_CAPTION,
		IDC_CORE_CLOCK_SLIDER,
		IDC_CORE_CLOCK_CUR,
		IDC_CORE_CLOCK_MIN,
		IDC_CORE_CLOCK_MAX,
		0
	};

	DWORD memoryClockControls[] = 
	{
		IDC_MEMORY_CLOCK_CAPTION,
		IDC_MEMORY_CLOCK_SLIDER,
		IDC_MEMORY_CLOCK_CUR,
		IDC_MEMORY_CLOCK_MIN,
		IDC_MEMORY_CLOCK_MAX,
		0
	};

	if (m_pSnapshot)
	{
		EnableControls(TRUE, commonControls);

		LPMACM_SHARED_MEMORY_HEADER		lpHeader	= (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;
		LPMACM_SHARED_MEMORY_GPU_ENTRY	lpGpuEntry	= (LPMACM_SHARED_MEMORY_GPU_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + lpHeader->dwMasterGpu * lpHeader->dwGpuEntrySize);

		//master GPU selection controls

		m_ctrlMasterGpuCombo.ResetContent();

		for (DWORD dwGpu=0; dwGpu<lpHeader->dwNumGpuEntries; dwGpu++)
		{
			CString strGpu;
			strGpu.Format("GPU%d", dwGpu + 1);

			m_ctrlMasterGpuCombo.AddString(strGpu);
		}

		m_ctrlMasterGpuCombo.SetCurSel(lpHeader->dwMasterGpu);

		//GPU synchronizartion controls

		m_bSync = ((lpHeader->dwFlags & MACM_SHARED_MEMORY_FLAG_SYNC) != 0);

		//core clock controls

		if (lpGpuEntry->dwFlags & MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_CORE_CLOCK)
		{
			EnableControls(TRUE, coreClockControls);

			m_ctrlCoreClockSlider.SetRange(lpGpuEntry->dwCoreClockMin, lpGpuEntry->dwCoreClockMax);
			m_ctrlCoreClockSlider.SetPos(lpGpuEntry->dwCoreClockCur);

			m_strCoreClockMin.Format("%d MHz", lpGpuEntry->dwCoreClockMin / 1000);
			m_strCoreClockMax.Format("%d MHz", lpGpuEntry->dwCoreClockMax / 1000);
			m_strCoreClockCur.Format("%d MHz", lpGpuEntry->dwCoreClockCur / 1000);
		}
		else
		{
			EnableControls(FALSE, coreClockControls);

			m_ctrlCoreClockSlider.SetRange(0, 0);
			m_ctrlCoreClockSlider.SetPos(0);

			m_strCoreClockMin = "";
			m_strCoreClockMax = "";
			m_strCoreClockCur = "";
		}

		//memory clock controls

		if (lpGpuEntry->dwFlags & MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_MEMORY_CLOCK)
		{
			EnableControls(TRUE, memoryClockControls);

			m_ctrlMemoryClockSlider.SetRange(lpGpuEntry->dwMemoryClockMin, lpGpuEntry->dwMemoryClockMax);
			m_ctrlMemoryClockSlider.SetPos(lpGpuEntry->dwMemoryClockCur);

			m_strMemoryClockMin.Format("%d MHz", lpGpuEntry->dwMemoryClockMin / 1000);
			m_strMemoryClockMax.Format("%d MHz", lpGpuEntry->dwMemoryClockMax / 1000);
			m_strMemoryClockCur.Format("%d MHz", lpGpuEntry->dwMemoryClockCur / 1000);
		}
		else
		{
			EnableControls(FALSE, memoryClockControls);

			m_ctrlMemoryClockSlider.SetRange(0, 0);
			m_ctrlMemoryClockSlider.SetPos(0);

			m_strMemoryClockMin = "";
			m_strMemoryClockMax = "";
			m_strMemoryClockCur = "";
		}

		UpdateData(FALSE);
	}
	else
	{
		EnableControls(FALSE, commonControls);

		m_ctrlMasterGpuCombo.ResetContent();
		m_ctrlMasterGpuCombo.AddString("Failed to connect to MSI Afterburner");
		m_ctrlMasterGpuCombo.SetCurSel(0);

		EnableControls(FALSE, coreClockControls);

		m_ctrlCoreClockSlider.SetRange(0, 0);
		m_ctrlCoreClockSlider.SetPos(0);

		m_strCoreClockMin = "";
		m_strCoreClockMax = "";
		m_strCoreClockCur = "";

		EnableControls(FALSE, memoryClockControls);

		m_ctrlMemoryClockSlider.SetRange(0, 0);
		m_ctrlMemoryClockSlider.SetPos(0);

		m_strMemoryClockMin = "";
		m_strMemoryClockMax = "";
		m_strMemoryClockCur = "";
	}
}
/////////////////////////////////////////////////////////////////////////////
HBRUSH CMACMSharedMemorySampleDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	COLORREF clrBk		= RGB(0,0,96);
	COLORREF clrText	= RGB(255,255,255);

	UINT nID			= pWnd->GetDlgCtrlID();

	if ((nID == IDC_MASTER_GPU_CAPTION	) || 
		(nID == IDC_OVERCLOCKING_CAPTION))
	{
		if (!m_hBrush)
 			 m_hBrush = CreateSolidBrush(clrBk);

		pDC->SetBkColor(clrBk);
		pDC->SetTextColor(clrText);
	}
	else 
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_hBrush;
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::SendCommandNotification()
{
	HWND hWnd = ::FindWindow(NULL, "MSI Afterburner ");

	if (hWnd)
		::PostMessage(hWnd, WM_MACM_CMD_NOTIFICATION, 0, 0);
}
/////////////////////////////////////////////////////////////////////////////
DWORD CMACMSharedMemorySampleDlg::PollCommand()
{
	HANDLE hMutex	= CreateMutex(NULL, FALSE, "Global\\Access_MACMSharedMemory");
	if (hMutex)
		WaitForSingleObject(hMutex, INFINITE);

	DWORD dwCommand = 0xFFFFFFFF;

	if (m_pMapAddr)
	{
		LPMACM_SHARED_MEMORY_HEADER	lpHeader = (LPMACM_SHARED_MEMORY_HEADER)m_pMapAddr;

		if (lpHeader->dwSignature == 'MACM')
			dwCommand = lpHeader->dwCommand;
	}

	if (hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return dwCommand;
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::WaitForCommandCompletion()
{
	SendCommandNotification();

	DWORD dwTimeout = GetTickCount() + 5000;

	do
	{
		DWORD dwCommand = PollCommand();

		if (!dwCommand)
			return;

		if (dwCommand == 0xFFFFFFFF)
			return;

		Sleep(100);
	}
	while (GetTickCount() < dwTimeout);
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == (CScrollBar*)&m_ctrlCoreClockSlider)
	{
		DWORD dwCoreClock = m_ctrlCoreClockSlider.GetPos() / 1000;

		if (m_pSnapshot)
		{
			LPMACM_SHARED_MEMORY_HEADER		lpHeader	= (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;
			LPMACM_SHARED_MEMORY_GPU_ENTRY	lpGpuEntry	= (LPMACM_SHARED_MEMORY_GPU_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + lpHeader->dwMasterGpu * lpHeader->dwGpuEntrySize);

			if (lpGpuEntry->dwFlags & MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_CORE_CLOCK)
				lpGpuEntry->dwCoreClockCur = dwCoreClock * 1000;
		}

		m_strCoreClockCur.Format("%d MHz", dwCoreClock);

		UpdateData(FALSE);
	}
	
	if (pScrollBar == (CScrollBar*)&m_ctrlMemoryClockSlider)
	{
		DWORD dwMemoryClock = m_ctrlMemoryClockSlider.GetPos() / 1000;

		if (m_pSnapshot)
		{
			LPMACM_SHARED_MEMORY_HEADER		lpHeader	= (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;
			LPMACM_SHARED_MEMORY_GPU_ENTRY	lpGpuEntry	= (LPMACM_SHARED_MEMORY_GPU_ENTRY)((LPBYTE)lpHeader + lpHeader->dwHeaderSize + lpHeader->dwMasterGpu * lpHeader->dwGpuEntrySize);

			if (lpGpuEntry->dwFlags & MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_MEMORY_CLOCK)
				lpGpuEntry->dwMemoryClockCur = dwMemoryClock * 1000;
		}

		m_strMemoryClockCur.Format("%d MHz", dwMemoryClock);

		UpdateData(FALSE);
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnSelchangeMasterGpuCombo() 
{
	if (m_pSnapshot)
	{
		LPMACM_SHARED_MEMORY_HEADER	lpHeader = (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;

		lpHeader->dwMasterGpu = m_ctrlMasterGpuCombo.GetCurSel();
	}

	UpdateControls();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::OnSyncCheck() 
{
	if (m_pSnapshot)
	{
		LPMACM_SHARED_MEMORY_HEADER	lpHeader = (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;

		UpdateData(TRUE);

		if (m_bSync)
			lpHeader->dwFlags |= MACM_SHARED_MEMORY_FLAG_SYNC;
		else
			lpHeader->dwFlags &= ~MACM_SHARED_MEMORY_FLAG_SYNC;
	}

}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::CaptureSnapshot()
{
	DestroySnapshot();

	HANDLE hMutex	= CreateMutex(NULL, FALSE, "Global\\Access_MACMSharedMemory");
	if (hMutex)
		WaitForSingleObject(hMutex, INFINITE);

	if (m_pMapAddr)
	{
		LPMACM_SHARED_MEMORY_HEADER	lpHeader = (LPMACM_SHARED_MEMORY_HEADER)m_pMapAddr;

		if (lpHeader->dwSignature == 'MACM')
		{
			DWORD dwSize = lpHeader->dwHeaderSize + lpHeader->dwNumGpuEntries * lpHeader->dwGpuEntrySize;

			if (dwSize)
			{
				m_pSnapshot = new BYTE[dwSize];

				memcpy(m_pSnapshot, m_pMapAddr, dwSize);
			}
		}
	}

	if (hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::DestroySnapshot()
{
	if (m_pSnapshot)
		delete [] m_pSnapshot;

	m_pSnapshot = NULL;
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::FlushSnapshot()
{
	HANDLE hMutex	= CreateMutex(NULL, FALSE, "Global\\Access_MACMSharedMemory");
	if (hMutex)
		WaitForSingleObject(hMutex, INFINITE);

	if (m_pMapAddr && m_pSnapshot)
	{
		LPMACM_SHARED_MEMORY_HEADER	lpHeader			= (LPMACM_SHARED_MEMORY_HEADER)m_pMapAddr;
		LPMACM_SHARED_MEMORY_HEADER	lpHeaderSnapshot	= (LPMACM_SHARED_MEMORY_HEADER)m_pSnapshot;

		if (lpHeader->dwSignature == 'MACM')
		{
			DWORD dwSize			= lpHeader->dwHeaderSize + lpHeader->dwNumGpuEntries * lpHeader->dwGpuEntrySize;
			DWORD dwSnapshotSize	= lpHeaderSnapshot->dwHeaderSize + lpHeaderSnapshot->dwNumGpuEntries * lpHeaderSnapshot->dwGpuEntrySize;

			if (dwSize == dwSnapshotSize)
			{
				memcpy(m_pMapAddr, m_pSnapshot, dwSize);

				lpHeader->dwCommand = MACM_SHARED_MEMORY_COMMAND_FLUSH;
			}
		}
	}

	if (hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	WaitForCommandCompletion();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::InitControls()
{
	CaptureSnapshot();

	UpdateControls();
}
/////////////////////////////////////////////////////////////////////////////
void CMACMSharedMemorySampleDlg::EnableControls(BOOL bEnable, LPDWORD lpControlIDs)
{
	DWORD dwIndex = 0;

	while (lpControlIDs[dwIndex])
	{
		CWnd* pControl = GetDlgItem(lpControlIDs[dwIndex]);

		if (pControl)
			pControl->EnableWindow(bEnable);

		dwIndex++;
	}
}
/////////////////////////////////////////////////////////////////////////////
