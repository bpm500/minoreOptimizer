// MACMSharedMemorySampleDlg.h : header file
//

#if !defined(AFX_MACMSHAREDMEMORYSAMPLEDLG_H__7725E949_F929_49A9_AFBE_468EFA16948B__INCLUDED_)
#define AFX_MACMSHAREDMEMORYSAMPLEDLG_H__7725E949_F929_49A9_AFBE_468EFA16948B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MACMSharedMemory.h"

/////////////////////////////////////////////////////////////////////////////
// CMACMSharedMemorySampleDlg dialog
/////////////////////////////////////////////////////////////////////////////
class CMACMSharedMemorySampleDlg : public CDialog
{
// Construction
public:
	CMACMSharedMemorySampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMACMSharedMemorySampleDlg)
	enum { IDD = IDD_MACMSHAREDMEMORYSAMPLE_DIALOG };
	CSliderCtrl	m_ctrlMemoryClockSlider;
	CSliderCtrl	m_ctrlCoreClockSlider;
	CComboBox	m_ctrlMasterGpuCombo;
	BOOL		m_bSync;
	CString	m_strCoreClockCur;
	CString	m_strCoreClockMax;
	CString	m_strCoreClockMin;
	CString	m_strMemoryClockCur;
	CString	m_strMemoryClockMax;
	CString	m_strMemoryClockMin;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMACMSharedMemorySampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT	m_nTimerID;

	HANDLE	m_hMapFile;
	LPVOID	m_pMapAddr;
	LPVOID	m_pSnapshot;

	CString	m_strInstallPath;

	void	Connect();
	void	Disconnect();

	void	EnableControls(BOOL bEnable, LPDWORD lpControlIDs);
	void	InitControls();
	void	UpdateControls();

	void	CaptureSnapshot();
	void	DestroySnapshot();
	void	FlushSnapshot();

	void	WaitForCommandCompletion();
	DWORD	PollCommand();
	void	SendCommandNotification();

	HICON	m_hIcon;
	HBRUSH	m_hBrush;

	DWORD	m_dwCoreClockCur;
	DWORD	m_dwCoreClockMin;
	DWORD	m_dwCoreClockMax;
	DWORD	m_dwCoreClockDef;

	// Generated message map functions
	//{{AFX_MSG(CMACMSharedMemorySampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnApplyButton();
	afx_msg void OnDefaultsButton();
	afx_msg void OnRefreshButton();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeMasterGpuCombo();
	afx_msg void OnSyncCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACMSHAREDMEMORYSAMPLEDLG_H__7725E949_F929_49A9_AFBE_468EFA16948B__INCLUDED_)
