#pragma once
//////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "afxcmn.h"
#include "PerfCounterObjects.h"

#include "PropertyListCtrl.h"
//////////////////////////////////////////////////////////////////////
// CPerfCounterAddSourceDlg dialog
//////////////////////////////////////////////////////////////////////
class CPerfCounterAddSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CPerfCounterAddSourceDlg)

public:
	void SetSources(CPerfCounterDataSources* lpSources);
	void InitSourcesList();
	void UpdateSourcesList();

	CPerfCounterAddSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPerfCounterAddSourceDlg();

// Dialog Data
	enum { IDD = IDD_ADD_SOURCE_DIALOG };

protected:
	UINT				m_nTimerID;
	HBRUSH				m_hBrush;
	CPerfCounterObjects	m_objects;
	CPerfCounterDataSources* m_lpSources;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CPropertyListCtrl	m_sourcesListCtrl;

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedExportButton();
};
//////////////////////////////////////////////////////////////////////
