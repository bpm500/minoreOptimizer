// PerfCounterAddSourceDlg.cpp : implementation file
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PerfCounterDataSources.h"
#include "PerfCounterAddSourceDlg.h"
#include "PerfCounterGlobals.h"
#include "MAHMSharedMemory.h"
#include "MultiString.h"

#include <float.h>
//////////////////////////////////////////////////////////////////////
// CPerfCounterAddSourceDlg dialog
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CPerfCounterAddSourceDlg, CDialog)
//////////////////////////////////////////////////////////////////////
CPerfCounterAddSourceDlg::CPerfCounterAddSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPerfCounterAddSourceDlg::IDD, pParent)
{
	m_nTimerID	= 0;
	m_hBrush	= NULL;
	m_lpSources	= NULL;
}
//////////////////////////////////////////////////////////////////////
CPerfCounterAddSourceDlg::~CPerfCounterAddSourceDlg()
{
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCES_LIST, m_sourcesListCtrl);
}
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPerfCounterAddSourceDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CPerfCounterAddSourceDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, &CPerfCounterAddSourceDlg::OnBnClickedExportButton)
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////
// CPerfCounterAddSourceDlg message handlers
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterAddSourceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWaitCursor waitCursor;

	LocalizeWnd(m_hWnd);
	AdjustWindowPos(this, GetParent());

	m_sourcesListCtrl.ShowScrollBar(SB_VERT, TRUE);

	CRect cr; m_sourcesListCtrl.GetClientRect(&cr);

	DWORD dwWidth = cr.Width();

	m_sourcesListCtrl.SetResourceHandle(g_hModule);
	m_sourcesListCtrl.InsertColumn(0, LocalizeStr("Counter")	, LVCFMT_LEFT, dwWidth);

	CHeaderCtrl* pHeader = m_sourcesListCtrl.GetHeaderCtrl();

	DWORD dwStyle = GetWindowLong(pHeader->m_hWnd ,GWL_STYLE );
	dwStyle &= ~HDS_FULLDRAG;
	SetWindowLong(pHeader->m_hWnd  , GWL_STYLE, dwStyle );

	m_sourcesListCtrl.DrawRootButtons(TRUE);

	m_objects.Enum();
	m_objects.Sort(FALSE);

	waitCursor.Restore();

	InitSourcesList();

	m_nTimerID = SetTimer(0x1234, 1000, NULL);

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::OnDestroy()
{
	if (m_nTimerID)
		KillTimer(m_nTimerID);

	m_nTimerID = NULL;

	MSG msg; 
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

	if (m_hBrush)
	{
		DeleteObject(m_hBrush);

		m_hBrush = NULL;
	}

	CDialog::OnDestroy();
}
//////////////////////////////////////////////////////////////////////
HBRUSH CPerfCounterAddSourceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	COLORREF clrBk		= g_dwHeaderBgndColor;
	COLORREF clrText	= g_dwHeaderTextColor;

	UINT nID			= pWnd->GetDlgCtrlID();

	if (nID == IDC_COUNTER_PROPERTIES_HEADER)
	{
		if (!m_hBrush)
 			m_hBrush = CreateSolidBrush(clrBk);

		pDC->SetBkColor(clrBk);
		pDC->SetTextColor(clrText);
	}
	else 
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_hBrush;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::InitSourcesList()
{
	POSITION pos = m_objects.GetHeadPosition();

	while (pos)
	{
		LPPERFCOUNTER_OBJECT_DESC lpDesc = m_objects.GetNext(pos);

		char szEnglishObjectName[MAX_PATH];
		m_objects.GetEnglishName(lpDesc->szName, szEnglishObjectName, sizeof(szEnglishObjectName));
			//system enumerates object names in localized format, so we convert them back to English to provide language independent 
			//plugin settings

		PLC_ITEM_DESC* pRoot = m_sourcesListCtrl.InsertItem(0, szEnglishObjectName, "", NULL, 0, 0, 0, 0);

		if (lpDesc->lpCounters)
		{
			CMultiString mstr(lpDesc->lpCounters);

			LPCSTR lpCounter = mstr.GetNext();

			while (lpCounter)
			{
				char szEnglishCounterName[MAX_PATH];
				m_objects.GetEnglishName(lpCounter, szEnglishCounterName, sizeof(szEnglishCounterName));
					//system enumerates counter names in localized format, so we convert them back to English to provide language independent 
					//plugin settings

				m_sourcesListCtrl.InsertItem(0, szEnglishCounterName, "", pRoot, (DWORD)lpDesc, 0, 0, 0);

				lpCounter = mstr.GetNext();
			}
		}
	}

	m_sourcesListCtrl.Init();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::UpdateSourcesList()
{
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::OnBnClickedOk()
{
	int				nSelCount			= m_sourcesListCtrl.GetSelCount();
	int				nFocusedItem		= m_sourcesListCtrl.GetFocusedItem();
	PLC_ITEM_DESC*	lpFocusedItemDesc	= NULL;

	if (!nSelCount && (nFocusedItem != -1))
	{
		lpFocusedItemDesc = (PLC_ITEM_DESC*)m_sourcesListCtrl.GetItemData(nFocusedItem);

		if (lpFocusedItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
			lpFocusedItemDesc = NULL;
	}

	if (m_lpSources)
	{
		CList<PLC_ITEM_DESC*, PLC_ITEM_DESC*>* lpItemsList = m_sourcesListCtrl.GetItemsList();

		POSITION pos = lpItemsList->GetHeadPosition();

		while (pos)
		{
			PLC_ITEM_DESC*				lpItemDesc		= lpItemsList->GetNext(pos);
			PERFCOUNTER_OBJECT_DESC*	lpObjectDesc	= (PERFCOUNTER_OBJECT_DESC*)lpItemDesc->dwUserData0;

			if ((lpItemDesc->dwFlags & PLC_ITEM_FLAG_SELECTED) || (lpItemDesc == lpFocusedItemDesc))
			{

				LPPERFCOUNTER_DATA_SOURCE_DESC lpSourceDesc = new PERFCOUNTER_DATA_SOURCE_DESC; ZeroMemory(lpSourceDesc, sizeof(PERFCOUNTER_DATA_SOURCE_DESC));
					//allocate and zap new source descriptor

				char szEnglishObjectName[MAX_PATH];
				m_objects.GetEnglishName(lpObjectDesc->szName, szEnglishObjectName, sizeof(szEnglishObjectName));
					//system enumerates object names in localized format, so we convert them back to English to provide language independent 
					//plugin settings
				strcpy_s(lpSourceDesc->szObjectName, sizeof(lpSourceDesc->szObjectName), szEnglishObjectName);
					//object name

				lpSourceDesc->dwInstanceIndex = 0xFFFFFFFF;
					//instance index
				if (lpObjectDesc->lpInstances)
				{
					char szEnglishInstanceName[MAX_PATH];
					m_objects.GetEnglishName(lpObjectDesc->lpInstances, szEnglishInstanceName, sizeof(szEnglishInstanceName));
						//system enumerates instamce names in localized format, so we convert them back to English to provide language independent 
						//plugin settings

					strcpy_s(lpSourceDesc->szInstanceName, sizeof(lpSourceDesc->szInstanceName), szEnglishInstanceName);
						//instance name
				}

				strcpy_s(lpSourceDesc->szCounterName, sizeof(lpSourceDesc->szCounterName), lpItemDesc->szText);
					//counter name

				lpSourceDesc->dwID = MONITORING_SOURCE_ID_PLUGIN_MISC;
					//data source ID
				lpSourceDesc->dwInstance = 0xFFFFFFFF;
					//data source instance

				strcpy_s(lpSourceDesc->szName, sizeof(lpSourceDesc->szName), lpItemDesc->szText);
					//data source name

				lpSourceDesc->fltMinLimit	= 0.0f;
				lpSourceDesc->fltMaxLimit	= 100.0f;
					//data source range

				lpSourceDesc->dwOff			= 0;
				lpSourceDesc->dwMul			= 1;
				lpSourceDesc->dwDiv			= 1;
					//data source formula

				lpSourceDesc->fltData		= FLT_MAX;
					//current data

				lpSourceDesc->bSelected		= TRUE;
					//select new source in GUI

				m_lpSources->AddTail(lpSourceDesc);
			}
		}
	}

	OnOK();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::SetSources(CPerfCounterDataSources* lpSources)
{
	m_lpSources = lpSources;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_sourcesListCtrl.IsHeaderDragging())
	{
		if (m_sourcesListCtrl.GetItemCount())
			UpdateSourcesList();
		else
			InitSourcesList();
	}

	CDialog::OnTimer(nIDEvent);
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterAddSourceDlg::OnBnClickedExportButton()
{
	CWaitCursor waitCursor;

	CString strLog = "\n";

	POSITION pos = m_objects.GetHeadPosition();

	while (pos)
	{
		LPPERFCOUNTER_OBJECT_DESC lpDesc = m_objects.GetNext(pos);

		char szEnglishObjectName[MAX_PATH];
		m_objects.GetEnglishName(lpDesc->szName, szEnglishObjectName, sizeof(szEnglishObjectName));
			//system enumerates object names in localized format, so we convert them back to English to provide language independent 
			//plugin settings
			
		CString strBuf;

		strBuf.Format("Object %s\n", szEnglishObjectName);
		strLog += strBuf;

		if (lpDesc->lpCounters)
		{
			strLog += "  Counters\n";

			CMultiString mstr(lpDesc->lpCounters);

			LPCSTR lpCounter = mstr.GetNext();

			while (lpCounter)
			{
				char szEnglishCounterName[MAX_PATH];
				m_objects.GetEnglishName(lpCounter, szEnglishCounterName, sizeof(szEnglishCounterName));
					//system enumerates counter names in localized format, so we convert them back to English to provide language independent 
					//plugin settings

				strBuf.Format("    %s\n", szEnglishCounterName);
				strLog += strBuf;

				lpCounter = mstr.GetNext();
			}
		}

		if (lpDesc->lpInstances)
		{
			strLog += "  Instances\n";

			CMultiString mstr(lpDesc->lpInstances);

			LPCSTR lpInstance = mstr.GetNext();

			while (lpInstance)
			{
				char szEnglishInstanceName[MAX_PATH];
				m_objects.GetEnglishName(lpInstance, szEnglishInstanceName, sizeof(szEnglishInstanceName));
					//system enumerates instamce names in localized format, so we convert them back to English to provide language independent 
					//plugin settings

				strBuf.Format("    %s\n", szEnglishInstanceName);
				strLog += strBuf;

				lpInstance = mstr.GetNext();
			}
		}
	}

	AppendLog(strLog, FALSE);

	CString strMessage;
	strMessage.Format(LocalizeStr("Counters exported to %s!"), GetLogPath()),
	MessageBox(strMessage, LocalizeStr("Export"), MB_OK);
}
//////////////////////////////////////////////////////////////////////
