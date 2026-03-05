// PerfCounterSetupDlg.cpp : implementation file
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PerfCounterSetupDlg.h"
#include "PerfCounterAddSourceDlg.h"
#include "PerfCounterSetupSourceDlg.h"
#include "PerfCounterDataSources.h"
#include "PerfCounterGlobals.h"
#include "MAHMSharedMemory.h"

#include <shlwapi.h>
#include <float.h>
//////////////////////////////////////////////////////////////////////
// CPerfCounterSetupDlg dialog
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CPerfCounterSetupDlg, CDialog)
//////////////////////////////////////////////////////////////////////
CPerfCounterSetupDlg::CPerfCounterSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPerfCounterSetupDlg::IDD, pParent)
{
	m_nTimerID	= 0;
	m_hBrush	= NULL;
}
//////////////////////////////////////////////////////////////////////
CPerfCounterSetupDlg::~CPerfCounterSetupDlg()
{
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCES_LIST, m_sourcesListCtrl);
}
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPerfCounterSetupDlg, CDialog)
	ON_BN_CLICKED(IDC_SETUP_SOURCE_BUTTON, &CPerfCounterSetupDlg::OnBnClickedSetupSourceButton)
	ON_BN_CLICKED(IDC_ADD_SOURCE_BUTTON, &CPerfCounterSetupDlg::OnBnClickedAddSourceButton)
	ON_BN_CLICKED(IDC_REMOVE_SOURCE_BUTTON, &CPerfCounterSetupDlg::OnBnClickedRemoveSourceButton)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CPerfCounterSetupDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_DEFAULTS_BUTTON, &CPerfCounterSetupDlg::OnBnClickedDefaultsButton)
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////
// CPerfCounterSetupDlg message handlers
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnBnClickedSetupSourceButton()
{
	int iSelectedItem = m_sourcesListCtrl.GetFocusedItem();

	if (iSelectedItem != -1)
	{
		LPPERFCOUNTER_DATA_SOURCE_DESC lpDesc = m_sources.GetSourceDesc(iSelectedItem);

		if (lpDesc)
		{
			CPerfCounterSetupSourceDlg dlg;
			dlg.SetSourceDesc(lpDesc);

			if (IDOK == dlg.DoModal())
			{
				m_sources.Reinit(lpDesc);

				UpdateSourcesList();
			}
		}
	}

}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnBnClickedAddSourceButton()
{
	if (GetAsyncKeyState(VK_CONTROL) < 0)
	{
		int iAdded = 0;
		int iItems = m_sourcesListCtrl.GetItemCount();

		for (int iItem=0; iItem<iItems; iItem++)
		{
			if (m_sourcesListCtrl.GetItemState(iItem, LVIS_SELECTED))
			{
				PLC_ITEM_DESC* lpItemDesc = m_sourcesListCtrl.GetItemDesc(iItem);
			
				if (lpItemDesc)
				{
					LPPERFCOUNTER_DATA_SOURCE_DESC lpSourceDesc = (LPPERFCOUNTER_DATA_SOURCE_DESC)lpItemDesc->dwUserData0;

					LPPERFCOUNTER_DATA_SOURCE_DESC lpSourceCopy	= new PERFCOUNTER_DATA_SOURCE_DESC;

					CopyMemory(lpSourceCopy, lpSourceDesc, sizeof(PERFCOUNTER_DATA_SOURCE_DESC));

					lpSourceCopy->hCounter = NULL;
						//do not copy counter handle

					if (GetAsyncKeyState(VK_SHIFT) < 0)
					{
						if (lpSourceCopy->dwInstanceIndex != 0xFFFFFFFF)
							lpSourceCopy->dwInstanceIndex++;

						if (lpSourceCopy->dwInstance != 0xFFFFFFFF)
							lpSourceCopy->dwInstance++;
					}

					lpSourceCopy->bSelected = TRUE;

					m_sources.AddTail(lpSourceCopy);

					iAdded++;
				}
			}
		}

		if (iAdded)
		{
			m_sources.ReinitSelected();
			InitSourcesList();
			UpdateButtons();
		}
	}
	else
		//add new sources
	{
		CPerfCounterAddSourceDlg dlg;
		dlg.SetSources(&m_sources);

		if (IDOK == dlg.DoModal())
		{
			m_sources.ReinitSelected();
			InitSourcesList();
			UpdateButtons();
		}
	}
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnBnClickedRemoveSourceButton()
{	
	int iItems	= m_sourcesListCtrl.GetItemCount();

	for (int iItem=iItems-1; iItem>=0; iItem--)
	{
		if (m_sourcesListCtrl.GetItemState(iItem, LVIS_SELECTED))
		{
			PLC_ITEM_DESC* lpItemDesc = m_sourcesListCtrl.GetItemDesc(iItem);
		
			if (lpItemDesc)
			{
				LPPERFCOUNTER_DATA_SOURCE_DESC lpSourceDesc = (LPPERFCOUNTER_DATA_SOURCE_DESC)lpItemDesc->dwUserData0;

				m_sources.RemoveSource(lpSourceDesc);

				m_sourcesListCtrl.DeleteItem(iItem);
			}
		}
	}

	int iFocus = m_sourcesListCtrl.GetFocusedItem();

	if (iFocus != -1)
		m_sourcesListCtrl.SetItemState(iFocus, LVIS_SELECTED, LVIS_SELECTED);

	UpdateButtons();
}
//////////////////////////////////////////////////////////////////////
BOOL CPerfCounterSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	LocalizeWnd(m_hWnd);
	AdjustWindowPos(this, GetParent());

	//initialize local sources list

	m_sources.Init(GetCfgPath());

	//init sources list control

	m_sourcesListCtrl.ShowScrollBar(SB_VERT, TRUE);

	CRect cr; m_sourcesListCtrl.GetClientRect(&cr);

	DWORD dwWidth		= cr.Width() / 2;
	DWORD dwWidthChunk	= cr.Width() % 2;

	m_sourcesListCtrl.SetResourceHandle(g_hModule);
	m_sourcesListCtrl.InsertColumn(0, LocalizeStr("Source")	, LVCFMT_LEFT, dwWidth);
	m_sourcesListCtrl.InsertColumn(1, LocalizeStr("Value")	, LVCFMT_LEFT, dwWidth + dwWidthChunk);

	CHeaderCtrl* pHeader = m_sourcesListCtrl.GetHeaderCtrl();

	DWORD dwStyle = GetWindowLong(pHeader->m_hWnd ,GWL_STYLE );
	dwStyle &= ~HDS_FULLDRAG;
	SetWindowLong(pHeader->m_hWnd  , GWL_STYLE, dwStyle );

	m_sourcesListCtrl.AllowItemSelection(FALSE);
	m_sourcesListCtrl.AllowEmptyRootExpand(FALSE);
	m_sourcesListCtrl.DrawPropertyCells(FALSE);
	m_sourcesListCtrl.EnableMultiSelection(TRUE);

	InitSourcesList();

	m_nTimerID = SetTimer(0x1234, 1000, NULL);

	UpdateButtons();

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnDestroy()
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
HBRUSH CPerfCounterSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	COLORREF clrBk		= g_dwHeaderBgndColor;
	COLORREF clrText	= g_dwHeaderTextColor;

	UINT nID			= pWnd->GetDlgCtrlID();

	if ((nID == IDC_COUNTER_PROPERTIES_HEADER	) || 
		(nID == IDC_SOURCE_PROPERTIES_HEADER	))
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
void CPerfCounterSetupDlg::InitSourcesList()
{
	m_sourcesListCtrl.SetRedraw(FALSE);
	m_sourcesListCtrl.DeleteAllItems();
	m_sourcesListCtrl.Uninit();

	POSITION pos = m_sources.GetHeadPosition();

	while (pos)
	{
		PERFCOUNTER_DATA_SOURCE_DESC* lpDesc = m_sources.GetNext(pos);

		CString strName, strNameTemplate;
		FormatName(lpDesc->szName, lpDesc->dwInstance, strName, strNameTemplate);

		m_sourcesListCtrl.InsertItem(PLC_ITEM_FLAG_PROPERTY_COLUMN, strName, "", NULL, (DWORD)lpDesc, 0, 0, 0);
	}

	m_sourcesListCtrl.Init();
	SelectSources();
	m_sourcesListCtrl.SetRedraw(TRUE);
	UpdateSourcesList();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnBnClickedOk()
{
	//save local sources list

	m_sources.Save(GetCfgPath());

	OnOK();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_sourcesListCtrl.IsHeaderDragging())
		UpdateSourcesList();

	CDialog::OnTimer(nIDEvent);
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::UpdateSourcesList()
{
	for (int iItem=0; iItem<m_sourcesListCtrl.GetItemCount(); iItem++)
	{
		LPPERFCOUNTER_DATA_SOURCE_DESC	lpSourceDesc	= m_sources.GetSourceDesc(iItem);
		PLC_ITEM_DESC*					lpItemDesc		= m_sourcesListCtrl.GetItemDesc(iItem);

		if (lpSourceDesc && lpItemDesc)
		{
			CString strName, strNameTemplate;
			FormatName(lpSourceDesc->szName, lpSourceDesc->dwInstance, strName, strNameTemplate);

			strcpy_s(lpItemDesc->szText, sizeof(lpItemDesc->szText), strName);

			FLOAT fltData = m_sources.GetSourceData(iItem);

			CString strData;

			if (fltData != FLT_MAX)
			{
				strData.Format(strlen(lpSourceDesc->szFormat) ? lpSourceDesc->szFormat : "%.0f", fltData);
				strData += " ";
				strData += lpSourceDesc->szUnits;
			}
			else
				strData = "N/A";

			strcpy_s(lpItemDesc->szPropertyText, sizeof(lpItemDesc->szPropertyText), strData);
		}
	}

	m_sourcesListCtrl.RedrawVisibleItems();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::UpdateButtons()
{
	BOOL bEnable = FALSE;

	int nSelectedItem = m_sourcesListCtrl.GetFocusedItem();

	if (nSelectedItem != -1)
		bEnable = TRUE;

	GetDlgItem(IDC_SETUP_SOURCE_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_REMOVE_SOURCE_BUTTON)->EnableWindow(bEnable);
}
//////////////////////////////////////////////////////////////////////
LRESULT CPerfCounterSetupDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == UM_SEL_CHANGED)
		UpdateButtons();

	return CDialog::DefWindowProc(message, wParam, lParam);
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::OnBnClickedDefaultsButton()
{
	//reset local sources list

	m_sources.Reset(GetCfgPath());

	InitSourcesList();
	UpdateButtons();
}
//////////////////////////////////////////////////////////////////////
void CPerfCounterSetupDlg::SelectSources()
{
	int iFocus = -1;

	for (int iItem=0; iItem<m_sourcesListCtrl.GetItemCount(); iItem++)
	{
		PLC_ITEM_DESC* lpItemDesc = m_sourcesListCtrl.GetItemDesc(iItem);

		if (lpItemDesc)
		{
			LPPERFCOUNTER_DATA_SOURCE_DESC lpSourceDesc	= (LPPERFCOUNTER_DATA_SOURCE_DESC)lpItemDesc->dwUserData0;

			if (lpSourceDesc->bSelected)
			{
				if (iFocus == -1)
				{
					m_sourcesListCtrl.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

					iFocus = iItem;
				}
				else
					m_sourcesListCtrl.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);

				lpSourceDesc->bSelected = FALSE;
			}
		}
	}

	if (iFocus != -1)
		m_sourcesListCtrl.EnsureVisible(iFocus, FALSE);
}
//////////////////////////////////////////////////////////////////////
