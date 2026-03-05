#pragma once
//////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "PerfCounterDataSources.h"
#include "afxwin.h"
//////////////////////////////////////////////////////////////////////
// CPerfCounterSetupSourceDlg dialog
//////////////////////////////////////////////////////////////////////
class CPerfCounterSetupSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CPerfCounterSetupSourceDlg)

public:
	void SetSourceDesc(LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc);
	BOOL ValidateInt(LPCSTR lpLine);
	BOOL ValidateSource();

	CPerfCounterSetupSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPerfCounterSetupSourceDlg();

// Dialog Data
	enum { IDD = IDD_SETUP_SOURCE_DIALOG };

protected:
	LPPERFCOUNTER_DATA_SOURCE_DESC	m_lpDesc;
	HBRUSH							m_hBrush;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CString m_strObjectName;
	CString m_strObjectInstance;
	CString m_strCounterName;
	CComboBox m_sourceIdCombo;
	CString m_strSourceInstance;
	CString m_strSourceName;
	CString m_strSourceUnits;
	CString m_strSourceFormat;
	CString m_strSourceGroup;
	CString m_strSourceMin;
	CString m_strSourceMax;
	afx_msg void OnBnClickedOk();
	CString m_strOff;
	CString m_strMul;
	CString m_strDiv;
	afx_msg void OnBnClickedSelectInstanceButton();
	BOOL m_bDynamic;
};
//////////////////////////////////////////////////////////////////////
