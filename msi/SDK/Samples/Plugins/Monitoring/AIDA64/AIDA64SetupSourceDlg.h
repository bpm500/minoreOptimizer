#pragma once
//////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "AIDA64DataSources.h"
#include "afxwin.h"
//////////////////////////////////////////////////////////////////////
// CAIDA64SetupSourceDlg dialog
//////////////////////////////////////////////////////////////////////
class CAIDA64SetupSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CAIDA64SetupSourceDlg)

public:
	void SetSourceDesc(LPAIDA64_DATA_SOURCE_DESC lpDesc);
	BOOL ValidateSource();

	CAIDA64SetupSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAIDA64SetupSourceDlg();

// Dialog Data
	enum { IDD = IDD_SETUP_SOURCE_DIALOG };

protected:
	LPAIDA64_DATA_SOURCE_DESC	m_lpDesc;
	HBRUSH						m_hBrush;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CString m_strSensorID;
	CString m_strReadingName;
	CComboBox m_sourceIdCombo;
	CString m_strSourceInstance;
	CString m_strSourceName;
	CString m_strSourceUnits;
	CString m_strSourceFormat;
	CString m_strSourceGroup;
	CString m_strSourceMin;
	CString m_strSourceMax;
	afx_msg void OnBnClickedOk();
};
//////////////////////////////////////////////////////////////////////
