#pragma once
//////////////////////////////////////////////////////////////////////
#include "afxcmn.h"
#include "resource.h"

#include "AIDA64DataSources.h"
#include "PropertyListCtrl.h"
//////////////////////////////////////////////////////////////////////
// CAIDA64SetupDlg dialog
//////////////////////////////////////////////////////////////////////
class CAIDA64SetupDlg : public CDialog
{
	DECLARE_DYNAMIC(CAIDA64SetupDlg)

public:
	void InitSourcesList();
	void UpdateSourcesList();
	void UpdateButtons();
	void SelectSources();

	CAIDA64SetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAIDA64SetupDlg();

// Dialog Data
	enum { IDD = IDD_SETUP_DIALOG };

protected:
	UINT				m_nTimerID;
	HBRUSH				m_hBrush;
	CAIDA64DataSources	m_sources;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CPropertyListCtrl m_sourcesListCtrl;

	afx_msg void OnBnClickedSetupSourceButton();
	afx_msg void OnBnClickedAddSourceButton();
	afx_msg void OnBnClickedRemoveSourceButton();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedDefaultsButton();
protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
//////////////////////////////////////////////////////////////////////
