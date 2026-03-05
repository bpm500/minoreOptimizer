#pragma once
//////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "afxcmn.h"
#include "AIDA64Parser.h"

#include "PropertyListCtrl.h"
//////////////////////////////////////////////////////////////////////
// CAIDA64AddSourceDlg dialog
//////////////////////////////////////////////////////////////////////
#define PARSE_SENSOR_CONTEXT_INIT									1
#define PARSE_SENSOR_CONTEXT_UPDATE									2
//////////////////////////////////////////////////////////////////////
class CAIDA64AddSourceDlg : public CDialog,
							public CAIDA64Parser
{
	DECLARE_DYNAMIC(CAIDA64AddSourceDlg)

public:
	void SetSources(CAIDA64DataSources* lpSources);
	void InitSourcesList();
	void UpdateSourcesList();

	virtual void ParseSensor(LPCSTR lpType, LPCSTR lpID, LPCSTR lpLabel, LPCSTR lpValue, DWORD dwContext);

	CAIDA64AddSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAIDA64AddSourceDlg();

// Dialog Data
	enum { IDD = IDD_ADD_SOURCE_DIALOG };

protected:
	UINT				m_nTimerID;
	HBRUSH				m_hBrush;
	CStringList			m_cache;
	CAIDA64DataSources* m_lpSources;

	PLC_ITEM_DESC*		GetRootItem(LPCSTR lpType);

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
