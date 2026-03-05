#pragma once
//////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "PerfCounterObjects.h"
#include "afxwin.h"
//////////////////////////////////////////////////////////////////////
// CPerfCounterSelectInstanceDlg dialog
//////////////////////////////////////////////////////////////////////
class CPerfCounterSelectInstanceDlg : public CDialog
{
	DECLARE_DYNAMIC(CPerfCounterSelectInstanceDlg)

public:
	BOOL ValidateInt(LPCSTR lpLine);

	CPerfCounterSelectInstanceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPerfCounterSelectInstanceDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_INSTANCE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CPerfCounterObjects			m_objects;
	CString						m_strObjectName;
	CString						m_strObjectInstance;

	virtual BOOL OnInitDialog();
	CComboBox m_instancesCombo;
	afx_msg void OnBnClickedOk();
	BOOL m_bUseIndex;
};
//////////////////////////////////////////////////////////////////////
