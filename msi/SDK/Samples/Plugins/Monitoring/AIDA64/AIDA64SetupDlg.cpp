// AIDA64SetupDlg.cpp : implementation file
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AIDA64SetupDlg.h"
#include "AIDA64AddSourceDlg.h"
#include "AIDA64SetupSourceDlg.h"
#include "AIDA64DataSources.h"
#include "AIDA64Globals.h"
#include "MAHMSharedMemory.h"

#include <shlwapi.h>
#include <float.h>
//////////////////////////////////////////////////////////////////////
// CAIDA64SetupDlg dialog
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CAIDA64SetupDlg, CDialog)
//////////////////////////////////////////////////////////////////////
CAIDA64SetupDlg::CAIDA64SetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAIDA64SetupDlg::IDD, pParent)
{
	m_nTimerID	= 0;
	m_hBrush	= NULL;
}
//////////////////////////////////////////////////////////////////////
CAIDA64SetupDlg::~CAIDA64SetupDlg()
{
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCES_LIST, m_sourcesListCtrl);
}
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CAIDA64SetupDlg, CDialog)
	ON_BN_CLICKED(IDC_SETUP_SOURCE_BUTTON, &CAIDA64SetupDlg::OnBnClickedSetupSourceButton)
	ON_BN_CLICKED(IDC_ADD_SOURCE_BUTTON, &CAIDA64SetupDlg::OnBnClickedAddSourceButton)
	ON_BN_CLICKED(IDC_REMOVE_SOURCE_BUTTON, &CAIDA64SetupDlg::OnBnClickedRemoveSourceButton)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CAIDA64SetupDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_DEFAULTS_BUTTON, &CAIDA64SetupDlg::OnBnClickedDefaultsButton)
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////
// CAIDA64SetupDlg message handlers
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::OnBnClickedSetupSourceButton()
{
	int iSelectedItem = m_sourcesListCtrl.GetFocusedItem();

	if (iSelectedItem != -1)
	{
		LPAIDA64_DATA_SOURCE_DESC lpDesc = m_sources.GetSourceDesc(iSelectedItem);

		if (lpDesc)
		{
			CAIDA64SetupSourceDlg dlg;
			dlg.SetSourceDesc(lpDesc);

			if (IDOK == dlg.DoModal())
				UpdateSourcesList();
		}
	}

}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::OnBnClickedAddSourceButton()
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
					LPAIDA64_DATA_SOURCE_DESC lpSourceDesc = (LPAIDA64_DATA_SOURCE_DESC)lpItemDesc->dwUserData0;

					LPAIDA64_DATA_SOURCE_DESC lpSourceCopy	= new AIDA64_DATA_SOURCE_DESC;

					CopyMemory(lpSourceCopy, lpSourceDesc, sizeof(AIDA64_DATA_SOURCE_DESC));

					if (GetAsyncKeyState(VK_SHIFT) < 0)
					{
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
			InitSourcesList();
			UpdateButtons();
		}
	}
	else
		//add new sources
	{
		CAIDA64AddSourceDlg dlg;
		dlg.SetSources(&m_sources);

		if (IDOK == dlg.DoModal())
		{
			InitSourcesList();
			UpdateButtons();
		}
	}
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::OnBnClickedRemoveSourceButton()
{	
	int iItems	= m_sourcesListCtrl.GetItemCount();

	for (int iItem=iItems-1; iItem>=0; iItem--)
	{
		if (m_sourcesListCtrl.GetItemState(iItem, LVIS_SELECTED))
		{
			PLC_ITEM_DESC* lpItemDesc = m_sourcesListCtrl.GetItemDesc(iItem);
		
			if (lpItemDesc)
			{
				LPAIDA64_DATA_SOURCE_DESC lpSourceDesc = (LPAIDA64_DATA_SOURCE_DESC)lpItemDesc->dwUserData0;

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
BOOL CAIDA64SetupDlg::OnInitDialog()
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
void CAIDA64SetupDlg::OnDestroy()
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
HBRUSH CAIDA64SetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	COLORREF clrBk		= g_dwHeaderBgndColor;
	COLORREF clrText	= g_dwHeaderTextColor;

	UINT nID			= pWnd->GetDlgCtrlID();

	if ((nID == IDC_SENSOR_PROPERTIES_HEADER) || 
		(nID == IDC_SOURCE_PROPERTIES_HEADER))
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
void CAIDA64SetupDlg::InitSourcesList()
{
	m_sourcesListCtrl.SetRedraw(FALSE);
	m_sourcesListCtrl.DeleteAllItems();
	m_sourcesListCtrl.Uninit();

	POSITION pos = m_sources.GetHeadPosition();

	while (pos)
	{
		AIDA64_DATA_SOURCE_DESC* lpDesc = m_sources.GetNext(pos);

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
void CAIDA64SetupDlg::OnBnClickedOk()
{
	//save local sources list

	m_sources.Save(GetCfgPath());

	OnOK();
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_sourcesListCtrl.IsHeaderDragging())
		UpdateSourcesList();

	CDialog::OnTimer(nIDEvent);
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::UpdateSourcesList()
{
	for (int iItem=0; iItem<m_sourcesListCtrl.GetItemCount(); iItem++)
	{
		LPAIDA64_DATA_SOURCE_DESC	lpSourceDesc	= m_sources.GetSourceDesc(iItem);
		PLC_ITEM_DESC*				lpItemDesc		= m_sourcesListCtrl.GetItemDesc(iItem);

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
void CAIDA64SetupDlg::UpdateButtons()
{
	BOOL bEnable = FALSE;

	int nSelectedItem = m_sourcesListCtrl.GetFocusedItem();

	if (nSelectedItem != -1)
		bEnable = TRUE;

	GetDlgItem(IDC_SETUP_SOURCE_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_REMOVE_SOURCE_BUTTON)->EnableWindow(bEnable);
}
//////////////////////////////////////////////////////////////////////
LRESULT CAIDA64SetupDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == UM_SEL_CHANGED)
		UpdateButtons();

	return CDialog::DefWindowProc(message, wParam, lParam);
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::OnBnClickedDefaultsButton()
{
	//reset local sources list

	m_sources.Reset(GetCfgPath());

	InitSourcesList();
	UpdateButtons();
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupDlg::SelectSources()
{
	int iFocus = -1;

	for (int iItem=0; iItem<m_sourcesListCtrl.GetItemCount(); iItem++)
	{
		PLC_ITEM_DESC* lpItemDesc = m_sourcesListCtrl.GetItemDesc(iItem);

		if (lpItemDesc)
		{
			LPAIDA64_DATA_SOURCE_DESC lpSourceDesc	= (LPAIDA64_DATA_SOURCE_DESC)lpItemDesc->dwUserData0;

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
