// AIDA64SetupSourceDlg.cpp : implementation file
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AIDA64SetupSourceDlg.h"
#include "AIDA64Globals.h"
#include "MAHMSharedMemory.h"
//////////////////////////////////////////////////////////////////////
// CAIDA64SetupSourceDlg dialog
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CAIDA64SetupSourceDlg, CDialog)
//////////////////////////////////////////////////////////////////////
CAIDA64SetupSourceDlg::CAIDA64SetupSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAIDA64SetupSourceDlg::IDD, pParent)
	, m_strSensorID(_T(""))
	, m_strReadingName(_T(""))
	, m_strSourceInstance(_T(""))
	, m_strSourceName(_T(""))
	, m_strSourceUnits(_T(""))
	, m_strSourceFormat(_T(""))
	, m_strSourceGroup(_T(""))
	, m_strSourceMin(_T(""))
	, m_strSourceMax(_T(""))
{
	m_hBrush = NULL;
	m_lpDesc = NULL;
}
//////////////////////////////////////////////////////////////////////
CAIDA64SetupSourceDlg::~CAIDA64SetupSourceDlg()
{
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SENSOR_ID_EDIT, m_strSensorID);
	DDX_Control(pDX, IDC_SOURCE_ID_COMBO, m_sourceIdCombo);
	DDX_Text(pDX, IDC_SOURCE_INSTANCE_EDIT, m_strSourceInstance);
	DDX_Text(pDX, IDC_SOURCE_NAME_EDIT, m_strSourceName);
	DDX_Text(pDX, IDC_SOURCE_UNITS_EDIT, m_strSourceUnits);
	DDX_Text(pDX, IDC_SOURCE_FORMAT_EDIT, m_strSourceFormat);
	DDX_Text(pDX, IDC_SOURCE_GROUP_EDIT, m_strSourceGroup);
	DDX_Text(pDX, IDC_SOURCE_MIN_EDIT, m_strSourceMin);
	DDX_Text(pDX, IDC_SOURCE_MAX_EDIT, m_strSourceMax);
}
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CAIDA64SetupSourceDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CAIDA64SetupSourceDlg::OnBnClickedOk)
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////
// CAIDA64SetupSourceDlg message handlers
//////////////////////////////////////////////////////////////////////
typedef struct AIDA64_READING_TYPE_DESC
{
	DWORD	dwType;
	LPCSTR	lpName;
} AIDA64_READING_TYPE_DESC, *LPAIDA64_READING_TYPE_DESC;
//////////////////////////////////////////////////////////////////////
typedef struct DATA_SOURCE_ID_DESC
{
	DWORD	dwID;
	LPCSTR	lpName;
} DATA_SOURCE_ID_DESC, *LPDATA_SOURCE_ID_DESC;
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64SetupSourceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	LocalizeWnd(m_hWnd);
	AdjustWindowPos(this, GetParent());

	if (m_lpDesc)
	{
		//sensor ID

		m_strSensorID = m_lpDesc->szID;

		//data source id

		DATA_SOURCE_ID_DESC sourceIds[] = 
		{
			{ MONITORING_SOURCE_ID_GPU_TEMPERATURE				, "GPU temperature"			},
			{ MONITORING_SOURCE_ID_PCB_TEMPERATURE				, "PCB temperature"			},
			{ MONITORING_SOURCE_ID_MEM_TEMPERATURE				, "Memory temperature"		},
			{ MONITORING_SOURCE_ID_VRM_TEMPERATURE				, "VRM temperature"			},
			{ MONITORING_SOURCE_ID_FAN_SPEED					, "Fan speed"				},
			{ MONITORING_SOURCE_ID_FAN_TACHOMETER				, "Fan tachometer"			},
			{ MONITORING_SOURCE_ID_CORE_CLOCK					, "Core clock"				},
			{ MONITORING_SOURCE_ID_SHADER_CLOCK					, "Shader clock"			},
			{ MONITORING_SOURCE_ID_MEMORY_CLOCK					, "Memory clock"			},
			{ MONITORING_SOURCE_ID_GPU_USAGE					, "GPU usage"				},
			{ MONITORING_SOURCE_ID_MEMORY_USAGE					, "Memory usage"			},
			{ MONITORING_SOURCE_ID_FB_USAGE						, "FB usage"				},
			{ MONITORING_SOURCE_ID_VID_USAGE					, "VID usage"				},
			{ MONITORING_SOURCE_ID_BUS_USAGE					, "BUS usage"				},
			{ MONITORING_SOURCE_ID_GPU_VOLTAGE					, "GPU voltage"				},
			{ MONITORING_SOURCE_ID_AUX_VOLTAGE					, "Aux voltage"				},
			{ MONITORING_SOURCE_ID_MEMORY_VOLTAGE				, "Memory voltage"			},
			{ MONITORING_SOURCE_ID_FRAMERATE					, "Framerate"				},
			{ MONITORING_SOURCE_ID_FRAMETIME					, "Frametime"				},
			{ MONITORING_SOURCE_ID_FRAMERATE_MIN				, "Framerate Min"			},
			{ MONITORING_SOURCE_ID_FRAMERATE_AVG				, "Framerate Avg"			},
			{ MONITORING_SOURCE_ID_FRAMERATE_MAX				, "Framerate Max"			},
			{ MONITORING_SOURCE_ID_FRAMERATE_1DOT0_PERCENT_LOW	, "Framerate 1% Low"		},
			{ MONITORING_SOURCE_ID_FRAMERATE_0DOT1_PERCENT_LOW	, "Framerate 0.1% Low"		},
			{ MONITORING_SOURCE_ID_GPU_REL_POWER				, "Power percent"			},
			{ MONITORING_SOURCE_ID_GPU_ABS_POWER				, "Power"					},
			{ MONITORING_SOURCE_ID_GPU_TEMP_LIMIT				, "Temp limit"				},
			{ MONITORING_SOURCE_ID_GPU_POWER_LIMIT				, "Power limit"				},
			{ MONITORING_SOURCE_ID_GPU_VOLTAGE_LIMIT			, "Voltage limit"			},
			{ MONITORING_SOURCE_ID_GPU_UTIL_LIMIT				, "No load limit"			},
			{ MONITORING_SOURCE_ID_CPU_USAGE					, "CPU usage"				},
			{ MONITORING_SOURCE_ID_RAM_USAGE					, "RAM usage"				},
			{ MONITORING_SOURCE_ID_PAGEFILE_USAGE				, "Pagefile usage"			},
			{ MONITORING_SOURCE_ID_CPU_TEMPERATURE				, "CPU temperature"			},
			{ MONITORING_SOURCE_ID_GPU_SLI_SYNC_LIMIT			,"SLI sync limit"			},
			{ MONITORING_SOURCE_ID_CPU_CLOCK					, "CPU clock"				},
			{ MONITORING_SOURCE_ID_AUX2_VOLTAGE					, "Aux2 voltage"			},
			{ MONITORING_SOURCE_ID_GPU_TEMPERATURE2				, "GPU temperature 2"		},
			{ MONITORING_SOURCE_ID_PCB_TEMPERATURE2				, "PCB temperature 2"		},
			{ MONITORING_SOURCE_ID_MEM_TEMPERATURE2				, "Memory temperature 2"	},
			{ MONITORING_SOURCE_ID_VRM_TEMPERATURE2				, "VRM temperature 2"		},
			{ MONITORING_SOURCE_ID_GPU_TEMPERATURE3				, "GPU temperature 3"		},
			{ MONITORING_SOURCE_ID_PCB_TEMPERATURE3				, "PCB temperature 3"		},
			{ MONITORING_SOURCE_ID_MEM_TEMPERATURE3				, "Memory temperature 3"	},
			{ MONITORING_SOURCE_ID_VRM_TEMPERATURE3				, "VRM temperature 3"		},
			{ MONITORING_SOURCE_ID_GPU_TEMPERATURE4				, "GPU temperature 4"		},
			{ MONITORING_SOURCE_ID_PCB_TEMPERATURE4				, "PCB temperature 4"		},
			{ MONITORING_SOURCE_ID_MEM_TEMPERATURE4				, "Memory temperature 4"	},
			{ MONITORING_SOURCE_ID_VRM_TEMPERATURE4				, "VRM temperature 4"		},
			{ MONITORING_SOURCE_ID_GPU_TEMPERATURE5				, "GPU temperature 5"		},
			{ MONITORING_SOURCE_ID_PCB_TEMPERATURE5				, "PCB temperature 5"		},
			{ MONITORING_SOURCE_ID_MEM_TEMPERATURE5				, "Memory temperature 5"	},
			{ MONITORING_SOURCE_ID_VRM_TEMPERATURE5				, "VRM temperature 5"		},
			{ MONITORING_SOURCE_ID_PLUGIN_GPU					, "<GPU plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_CPU					, "<CPU plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_MOBO					, "<motherboard plugin>"	},
			{ MONITORING_SOURCE_ID_PLUGIN_RAM					, "<RAM plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_HDD					, "<HDD plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_NET					, "<NET plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_PSU					, "<PSU plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_UPS					, "<UPS plugin>"			},
			{ MONITORING_SOURCE_ID_PLUGIN_MISC					, "<miscellaneous plugin>"	},
			{ MONITORING_SOURCE_ID_CPU_POWER					, "CPU power"				}
		};

		m_sourceIdCombo.SetCurSel(0);

		for (int iIndex=0; iIndex<_countof(sourceIds); iIndex++)
		{
			int iItem = m_sourceIdCombo.AddString(LocalizeStr(sourceIds[iIndex].lpName));

			m_sourceIdCombo.SetItemData(iItem, sourceIds[iIndex].dwID);

			if (m_lpDesc->dwID == sourceIds[iIndex].dwID)
				m_sourceIdCombo.SetCurSel(iItem);
		}

		//data source instance

		if (m_lpDesc->dwInstance != 0xFFFFFFFF)
			m_strSourceInstance.Format("%d", m_lpDesc->dwInstance);
		else
			m_strSourceInstance = "";

		//data source name

		m_strSourceName = m_lpDesc->szName;

		//data source units

		m_strSourceUnits = m_lpDesc->szUnits;

		//data source output format

		m_strSourceFormat = m_lpDesc->szFormat;

		//data source group

		m_strSourceGroup = m_lpDesc->szGroup;

		//data source range

		m_strSourceMin.Format("%.1f", m_lpDesc->fltMinLimit);
		m_strSourceMax.Format("%.1f", m_lpDesc->fltMaxLimit);
 
		UpdateData(FALSE);
	}

	return TRUE; 
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupSourceDlg::SetSourceDesc(LPAIDA64_DATA_SOURCE_DESC lpDesc)
{
	m_lpDesc = lpDesc;
}
//////////////////////////////////////////////////////////////////////
void CAIDA64SetupSourceDlg::OnDestroy()
{
	if (m_hBrush)
	{
		DeleteObject(m_hBrush);

		m_hBrush = NULL;
	}

	CDialog::OnDestroy();
}
//////////////////////////////////////////////////////////////////////
HBRUSH CAIDA64SetupSourceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
void CAIDA64SetupSourceDlg::OnBnClickedOk()
{
	int iItem;

	UpdateData(TRUE);

	if (!ValidateSource())
	{
		MessageBeep(MB_ICONERROR);
		return;
	}

	if (m_lpDesc)
	{
		//sensor ID

		strcpy_s(m_lpDesc->szID, sizeof(m_lpDesc->szID), m_strSensorID);

		//data source id

		iItem = m_sourceIdCombo.GetCurSel();

		if (iItem != -1)
			m_lpDesc->dwID = m_sourceIdCombo.GetItemData(iItem);
		else
			m_lpDesc->dwID = MONITORING_SOURCE_ID_UNKNOWN;

		//data source instance

		if (sscanf_s(m_strSourceInstance, "%d", &m_lpDesc->dwInstance) != 1)
			m_lpDesc->dwInstance = 0xFFFFFFFF;

		//data source name

		strcpy_s(m_lpDesc->szName, sizeof(m_lpDesc->szName), m_strSourceName);

		//data source units

		strcpy_s(m_lpDesc->szUnits, sizeof(m_lpDesc->szUnits), m_strSourceUnits);

		//data source output format

		strcpy_s(m_lpDesc->szFormat, sizeof(m_lpDesc->szFormat), m_strSourceFormat);

		//data source group

		strcpy_s(m_lpDesc->szGroup, sizeof(m_lpDesc->szGroup), m_strSourceGroup);

		//data source range

		if (sscanf_s(m_strSourceMin, "%f", &m_lpDesc->fltMinLimit) != 1)
			m_lpDesc->fltMinLimit = 0.0f;
		if (sscanf_s(m_strSourceMax, "%f", &m_lpDesc->fltMaxLimit) != 1)
			m_lpDesc->fltMaxLimit = 100.0f;
	}

	OnOK();
}
//////////////////////////////////////////////////////////////////////
BOOL CAIDA64SetupSourceDlg::ValidateSource()
{
	m_strSensorID.Trim();

	if (m_strSensorID.IsEmpty())
	{
		GetDlgItem(IDC_SENSOR_ID_EDIT)->SetFocus();
		return FALSE;
	}

	m_strSourceName.Trim();

	if (m_strSourceName.IsEmpty())
	{
		GetDlgItem(IDC_SOURCE_NAME_EDIT)->SetFocus();
		return FALSE;
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
