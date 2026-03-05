#pragma once
/////////////////////////////////////////////////////////////////////////////
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CPingSetupDlg dialog
/////////////////////////////////////////////////////////////////////////////
class CPingSetupDlg : public CDialog
{
	DECLARE_DYNAMIC(CPingSetupDlg)

public:
	LPCSTR GetAddr();
	void SetAddr(LPCSTR lpAddr);

	CPingSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPingSetupDlg();

// Dialog Data
	enum { IDD = IDD_PING_SETUP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CString m_strAddr;
	HBRUSH	m_hBrush;

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
};
/////////////////////////////////////////////////////////////////////////////
