// MAHMSharedMemorySampleDlg.h : header file
//

#if !defined(AFX_MAHMSHAREDMEMORYSAMPLEDLG_H__7725E949_F929_49A9_AFBE_468EFA16948B__INCLUDED_)
#define AFX_MAHMSHAREDMEMORYSAMPLEDLG_H__7725E949_F929_49A9_AFBE_468EFA16948B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MAHMSharedMemory.h"

/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleDlg dialog
/////////////////////////////////////////////////////////////////////////////
class CMAHMSharedMemorySampleDlg : public CDialog
{
// Construction
public:
	void UpdateStatus();
	CString DumpGpuEntry(LPMAHM_SHARED_MEMORY_GPU_ENTRY lpGpuEntry);
	CString		DumpEntry(LPMAHM_SHARED_MEMORY_ENTRY lpEntry);
	void		Disconnect();

	CMAHMSharedMemorySampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMAHMSharedMemorySampleDlg)
	enum { IDD = IDD_MAHMSHAREDMEMORYSAMPLE_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMAHMSharedMemorySampleDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT			m_nTimerID;

	HANDLE			m_hMapFile;
	LPVOID			m_pMapAddr;

	CFont			m_font;
	CRichEditCtrl	m_richEditCtrl;

	CString			m_strStatus;
	CString			m_strInstallPath;

	void			Connect();
	BOOL			IsSystemSource(DWORD id);

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMAHMSharedMemorySampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAHMSHAREDMEMORYSAMPLEDLG_H__7725E949_F929_49A9_AFBE_468EFA16948B__INCLUDED_)
