// PerfCounterSelectInstanceDlg.cpp : implementation file
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PerfCounterGlobals.h"
#include "PerfCounterSelectInstanceDlg.h"
//////////////////////////////////////////////////////////////////////
// CPerfCounterSelectInstanceDlg dialog
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CPerfCounterSelectInstanceDlg, CDialog)
//////////////////////////////////////////////////////////////////////
CPerfCounterSelectInstanceDlg::CPerfCounterSelectInstanceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPerfCounterSelectInstanceDlg::IDD, pParent)
	, m_bUseIndex(FALSE)
{
	m_strObjectName		= "";
	m_strObjectInstance = "";
}
//////////////////////////////////////////////////////////////////////
CPerfCounterSelectInstanceDlg::~CPerfCounterSelectInstanceDlg()
{
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSelectInstanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INSTANCE_COMBO, m_instancesCombo);
	DDX_Check(pDX, IDC_USE_INDEX_CHECK, m_bUseIndex);
}
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPerfCounterSelectInstanceDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPerfCounterSelectInstanceDlg::OnBnClickedOk)
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////
// CPerfCounterSelectInstanceDlg message handlers
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterSelectInstanceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	LocalizeWnd(m_hWnd);

	if (!m_strObjectName.IsEmpty())
	{
		m_objects.Init();

		LONG dwInstanceIndex = 0;

		if (ValidateInt(m_strObjectInstance))
		{
			sscanf_s(m_strObjectInstance, "%d", &dwInstanceIndex);

			m_bUseIndex = TRUE;
		}
		else
			m_bUseIndex = FALSE;

		UpdateData(FALSE);

		char szLocalizedObjectName[MAX_PATH];
		m_objects.GetLocalizedName(m_strObjectName, szLocalizedObjectName, sizeof(szLocalizedObjectName));

		for (DWORD dwInstance=0;;dwInstance++)
		{
			LPCSTR lpInstance =	m_objects.GetInstance(szLocalizedObjectName, dwInstance);

			if (lpInstance)
			{
				int iItem = m_instancesCombo.AddString(lpInstance);

				if (m_bUseIndex)
				{
					if (iItem == dwInstanceIndex)
						m_instancesCombo.SetCurSel(iItem);

				}
				else
				{
					if (!strcmp(lpInstance, m_strObjectInstance))
						m_instancesCombo.SetCurSel(iItem);
				}
			}
			else
				break;
		}
	}

	if (m_instancesCombo.GetCount())
	{
		if (m_instancesCombo.GetCurSel() == -1)
			m_instancesCombo.SetCurSel(0);
	}
	else
		m_instancesCombo.EnableWindow(FALSE);

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterSelectInstanceDlg::ValidateInt(LPCSTR lpLine)
{
	BOOL bValid = FALSE;

	if ((*lpLine == '+') || (*lpLine == '-'))
		lpLine++;

	while ((*lpLine >= '0') && (*lpLine <= '9'))
	{
		lpLine++;
		bValid = TRUE;
	}

	if (*lpLine)
		return FALSE;

	return bValid;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSelectInstanceDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	int iSel = m_instancesCombo.GetCurSel();

	if (iSel != -1)
	{
		if (m_bUseIndex)
			m_strObjectInstance.Format("%d", iSel);
		else
			m_instancesCombo.GetLBText(iSel, m_strObjectInstance);
	}

	OnOK();
}
//////////////////////////////////////////////////////////////////////
