// AIDA64AddSourceDlg.cpp : implementation file
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AIDA64DataSources.h"
#include "AIDA64AddSourceDlg.h"
#include "AIDA64Globals.h"
#include "MAHMSharedMemory.h"

#include <float.h>
//////////////////////////////////////////////////////////////////////
// CAIDA64AddSourceDlg dialog
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CAIDA64AddSourceDlg, CDialog)
//////////////////////////////////////////////////////////////////////
CAIDA64AddSourceDlg::CAIDA64AddSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAIDA64AddSourceDlg::IDD, pParent)
{
	m_nTimerID		= 0;
	m_hBrush		= NULL;
	m_lpSources		= NULL;
}
//////////////////////////////////////////////////////////////////////
CAIDA64AddSourceDlg::~CAIDA64AddSourceDlg()
{
}
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCES_LIST, m_sourcesListCtrl);
}
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CAIDA64AddSourceDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CAIDA64AddSourceDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, &CAIDA64AddSourceDlg::OnBnClickedExportButton)
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////
// CAIDA64AddSourceDlg message handlers
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64AddSourceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	LocalizeWnd(m_hWnd);
	AdjustWindowPos(this, GetParent());

	m_sourcesListCtrl.ShowScrollBar(SB_VERT, TRUE);

	CRect cr; m_sourcesListCtrl.GetClientRect(&cr);

	DWORD dwWidth		= cr.Width() / 2;
	DWORD dwWidthChunk	= cr.Width() % 2;

	m_sourcesListCtrl.SetResourceHandle(g_hModule);
	m_sourcesListCtrl.InsertColumn(0, LocalizeStr("Sensor")	, LVCFMT_LEFT, dwWidth);
	m_sourcesListCtrl.InsertColumn(1, LocalizeStr("Value")	, LVCFMT_LEFT, dwWidth + dwWidthChunk);

	CHeaderCtrl* pHeader = m_sourcesListCtrl.GetHeaderCtrl();

	DWORD dwStyle = GetWindowLong(pHeader->m_hWnd ,GWL_STYLE );
	dwStyle &= ~HDS_FULLDRAG;
	SetWindowLong(pHeader->m_hWnd  , GWL_STYLE, dwStyle );

	m_sourcesListCtrl.DrawRootButtons(TRUE);

	InitSourcesList();

	m_nTimerID = SetTimer(0x1234, 1000, NULL);

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::OnDestroy()
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
HBRUSH CAIDA64AddSourceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	COLORREF clrBk		= g_dwHeaderBgndColor;
	COLORREF clrText	= g_dwHeaderTextColor;

	UINT nID			= pWnd->GetDlgCtrlID();

	if (nID == IDC_SENSOR_PROPERTIES_HEADER)
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
PLC_ITEM_DESC* CAIDA64AddSourceDlg::GetRootItem(LPCSTR lpType)
{
	CString strType = LocalizeStr(lpType);

	CList<PLC_ITEM_DESC*,PLC_ITEM_DESC*>* lpItemsList = m_sourcesListCtrl.GetItemsList();
	
	POSITION pos = lpItemsList->GetHeadPosition();

	while (pos)
	{
		PLC_ITEM_DESC* lpItem = lpItemsList->GetNext(pos);

		if (!_stricmp(lpItem->szText, strType))
			return lpItem;
	}

	return m_sourcesListCtrl.InsertItem(PLC_ITEM_FLAG_EXPANDED, strType, "", NULL, 0, 0, 0, 0);	
}
//////////////////////////////////////////////////////////////////////
typedef struct AIDA64_SENSOR_TYPE_DESC
{
	LPCSTR lpType;
	LPCSTR lpName;
	LPCSTR lpUnits;
} AIDA64_SENSOR_TYPE_DESC, *LPAIDA64_SENSOR_TYPE_DESC;
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::ParseSensor(LPCSTR lpType, LPCSTR lpID, LPCSTR lpLabel, LPCSTR lpValue, DWORD dwContext)
{
	switch (dwContext)
	{
	case PARSE_SENSOR_CONTEXT_INIT:
		{
			CString strName		= lpType;
			CString strUnits	= "";

			AIDA64_SENSOR_TYPE_DESC sensorTypes[] = 
			{
				{ "temp"			, "Temperature"		, "°C"	},
				{ "fan"				, "Fan tachometer"	, "RPM"	},
				{ "duty"			, "Fan speed"		, "%"	},
				{ "volt"			, "Voltage"			, "V"	},
				{ "pwr"				, "Power"			, "W"	}
			};

			for (int iIndex=0; iIndex<_countof(sensorTypes); iIndex++)
			{
				if (!_stricmp(sensorTypes[iIndex].lpType, lpType))
				{
					strName		= sensorTypes[iIndex].lpName;
					strUnits	= sensorTypes[iIndex].lpUnits;
					break;
				}
			}

			PLC_ITEM_DESC* lpRoot	= GetRootItem(strName);
			LPCSTR lpCachedID		= m_cache.GetAt(m_cache.AddTail(lpID));
			LPCSTR lpCachedUnits	= m_cache.GetAt(m_cache.AddTail(strUnits));

			CString strValue;
			strValue.Format("%s %s", lpValue, strUnits);

			m_sourcesListCtrl.InsertItem(PLC_ITEM_FLAG_PROPERTY_COLUMN, lpLabel, strValue, lpRoot, (DWORD)lpCachedID, (DWORD)lpCachedUnits, 0, 0);
		}
		break;
	case PARSE_SENSOR_CONTEXT_UPDATE:
		{
			CList<PLC_ITEM_DESC*,PLC_ITEM_DESC*>* lpItemsList = m_sourcesListCtrl.GetItemsList();
			
			POSITION pos = lpItemsList->GetHeadPosition();

			while (pos)
			{
				PLC_ITEM_DESC* lpItemDesc = lpItemsList->GetNext(pos);

				LPCSTR lpItemID		= (LPCSTR)lpItemDesc->dwUserData0;
				LPCSTR lpItemUnits	= (LPCSTR)lpItemDesc->dwUserData1;

				if (lpItemID)
				{
					if (!_stricmp(lpID, lpItemID))
					{
						CString strValue;
						strValue.Format("%s %s", lpValue, lpItemUnits);
	
						strcpy_s(lpItemDesc->szPropertyText, sizeof(lpItemDesc->szPropertyText), strValue);
					}
				}
			}
		}
		break;
	}
}
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::InitSourcesList()
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "AIDA64_SensorValues");
		//try to open AIDA64 shared memory mapping file

	if (hMapFile)
	{
		LPCSTR pMapAddr = (LPCSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			//try to map view of file to get access to AIDA64 XML-styled shared memory
		
		if (pMapAddr)
		{
			CString strCache = pMapAddr;
				//precache AIDA64 XML-styled shared memory

			ParseXML(strCache, PARSE_SENSOR_CONTEXT_INIT);
				//parse precached AIDA64 XML-styled shared memory

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	m_sourcesListCtrl.Init();
}
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::UpdateSourcesList()
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "AIDA64_SensorValues");
		//try to open AIDA64 shared memory mapping file

	if (hMapFile)
	{
		LPCSTR pMapAddr = (LPCSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			//try to map view of file to get access to AIDA64 XML-styled shared memory
		
		if (pMapAddr)
		{
			CString strCache = pMapAddr;
				//precache AIDA64 XML-styled shared memory

			ParseXML(strCache, PARSE_SENSOR_CONTEXT_UPDATE);
				//parse precached AIDA64 XML-styled shared memory

			UnmapViewOfFile(pMapAddr);

			m_sourcesListCtrl.RedrawVisibleItems();
		}

		CloseHandle(hMapFile);
	}
}
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::OnBnClickedOk()
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
			PLC_ITEM_DESC* lpItemDesc = lpItemsList->GetNext(pos);

			if ((lpItemDesc->dwFlags & PLC_ITEM_FLAG_SELECTED) || (lpItemDesc == lpFocusedItemDesc))
			{
				LPAIDA64_DATA_SOURCE_DESC lpSourceDesc = new AIDA64_DATA_SOURCE_DESC; ZeroMemory(lpSourceDesc, sizeof(AIDA64_DATA_SOURCE_DESC));
					//allocate and zap new source descriptor

				strcpy_s(lpSourceDesc->szID, sizeof(lpSourceDesc->szID), (LPCSTR)lpItemDesc->dwUserData0);
					//sensor ID
			
				lpSourceDesc->dwID = MONITORING_SOURCE_ID_PLUGIN_MISC;
					//data source ID

				strcpy_s(lpSourceDesc->szName, sizeof(lpSourceDesc->szName), lpItemDesc->szText);
					//data source name
				strcpy_s(lpSourceDesc->szUnits, sizeof(lpSourceDesc->szUnits), (LPCSTR)lpItemDesc->dwUserData1);
					//data source units

				lpSourceDesc->fltMinLimit	= 0.0f;
				lpSourceDesc->fltMaxLimit	= 100.0f;
					//data source range

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
void CAIDA64AddSourceDlg::SetSources(CAIDA64DataSources* lpSources)
{
	m_lpSources = lpSources;
}
//////////////////////////////////////////////////////////////////////
void CAIDA64AddSourceDlg::OnTimer(UINT_PTR nIDEvent)
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
void CAIDA64AddSourceDlg::OnBnClickedExportButton()
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "AIDA64_SensorValues");
		//try to open AIDA64 shared memory mapping file

	if (hMapFile)
	{
		LPCSTR pMapAddr = (LPCSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			//try to map view of file to get access to AIDA64 XML-styled shared memory
		
		if (pMapAddr)
		{
			AppendLog(pMapAddr, FALSE);

			CString strMessage;
			strMessage.Format(LocalizeStr("Sensors exported to %s!"), GetLogPath()),
			MessageBox(strMessage, LocalizeStr("Export"), MB_OK);

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}
}
//////////////////////////////////////////////////////////////////////
