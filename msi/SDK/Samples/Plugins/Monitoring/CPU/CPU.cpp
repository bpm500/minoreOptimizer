// CPU.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
#include <intrin.h>
 
#include "CPU.h"
#include "CPUGlobals.h"
#include "CPUTopology.h"
#include "MAHMSharedMemory.h"
#include "MSIAfterburnerMonitoringSourceDesc.h"
#include "MSIAfterburnerExports.h"
#include "ProcessAffinityMask.h"
#include "TimestampedDataCache.h"
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE CPUDLL = { NULL, NULL };
//////////////////////////////////////////////////////////////////////
#define MAX_CPU												256
//////////////////////////////////////////////////////////////////////
#define CPU_ARCH_UNKNOWN									0x00000000
#define CPU_ARCH_INTEL_NETBURST								0x80860001		
#define CPU_ARCH_INTEL_CORE									0x80860002
#define CPU_ARCH_INTEL_ATOM									0x80860003
#define CPU_ARCH_INTEL_NEHALEM								0x80860004
#define CPU_ARCH_INTEL_SANDYBRIDGE							0x80860005
#define CPU_ARCH_INTEL_IVYBRIDGE							0x80860006
#define CPU_ARCH_INTEL_HASWELL								0x80860007 
#define CPU_ARCH_INTEL_BROADWELL							0x80860008
#define CPU_ARCH_INTEL_SILVERMONT							0x80860009
#define CPU_ARCH_INTEL_SKYLAKE								0x8086000A
#define CPU_ARCH_INTEL_AIRMONT								0x8086000B
#define CPU_ARCH_INTEL_KABYLAKE								0x8086000C
#define CPU_ARCH_INTEL_GOLDMONT								0x8086000D
#define CPU_ARCH_INTEL_GOLDMONTPLUS							0x8086000E
#define CPU_ARCH_INTEL_CANNONLAKE							0x8086000F
#define CPU_ARCH_INTEL_ICELAKE								0x80860010
#define CPU_ARCH_INTEL_COMETLAKE							0x80860011
#define CPU_ARCH_INTEL_TREMONT								0x80860012
#define CPU_ARCH_INTEL_JASPERLAKE							0x80860013
#define CPU_ARCH_INTEL_TIGERLAKE							0x80860014
#define CPU_ARCH_INTEL_ROCKETLAKE							0x80860015
#define CPU_ARCH_INTEL_ALDERLAKE							0x80860016
#define CPU_ARCH_INTEL_RAPTORLAKE							0x80860017
#define CPU_ARCH_INTEL_ARROWLAKE							0x80860018

#define CPU_ARCH_AMD_FAMILY_10H								0x10220001		
#define CPU_ARCH_AMD_FAMILY_11H								0x10220002		
#define CPU_ARCH_AMD_FAMILY_12H								0x10220003		
#define CPU_ARCH_AMD_FAMILY_14H								0x10220004		
#define CPU_ARCH_AMD_FAMILY_15H_MODEL_00					0x10220005		
#define CPU_ARCH_AMD_FAMILY_15H_MODEL_10					0x10220006		
#define CPU_ARCH_AMD_FAMILY_15H_MODEL_30					0x10220007		
#define CPU_ARCH_AMD_FAMILY_16H_MODEL_00					0x10220008		
#define CPU_ARCH_AMD_FAMILY_16H_MODEL_30					0x10220009		
#define CPU_ARCH_AMD_FAMILY_17H_MODEL_00					0x1022000A		
#define CPU_ARCH_AMD_FAMILY_17H_MODEL_10					0x1022000B		
#define CPU_ARCH_AMD_FAMILY_17H_MODEL_30					0x1022000C		
#define CPU_ARCH_AMD_FAMILY_17H_MODEL_60					0x1022000D		
#define CPU_ARCH_AMD_FAMILY_17H_MODEL_70					0x1022000E		
#define CPU_ARCH_AMD_FAMILY_19H_MODEL_20					0x1022000F		
#define CPU_ARCH_AMD_FAMILY_19H_MODEL_40					0x10220010		
#define CPU_ARCH_AMD_FAMILY_19H_MODEL_50					0x10220011	
#define CPU_ARCH_AMD_FAMILY_19H_MODEL_60					0x10220012
#define CPU_ARCH_AMD_FAMILY_19H_MODEL_70					0x10220013
#define CPU_ARCH_AMD_FAMILY_1AH_MODEL_40					0x10220014
//////////////////////////////////////////////////////////////////////
typedef struct CPU_ARCH_DESC
{
	DWORD dwFamily;			//CPU family ID
	DWORD dwModel;			//CPU model ID
	DWORD dwStepping;		//CPU stepping, can be zero if we don't care
	DWORD dwNumCores;		//CPU cores number, can be zero if we don't care
							//NOTE: we use it to distinguish Core 2 Duo and Quad CPUs, which share exactly the same family, model and stepping
	DWORD dwArchitecture;	//CPU architecture
	DWORD dwProperty;		//vendor specific property, contains hardcoded Tjmax for early Intel CPU architectures and
							//DeviceID for miscellaneous control PCI device 18H function 3 for early AMD CPU architectures

} CPU_ARCH_DESC, *LPCPU_ARCH_DESC;
/////////////////////////////////////////////////////////////////////////////
CPU_ARCH_DESC g_arrIntelCpuArch[] =
{
	{ 0x0F, 0x00, 0   , 0,	CPU_ARCH_INTEL_NETBURST					, 100			}, // Intel Pentium 4 (180nm)
	{ 0x0F, 0x01, 0   , 0,	CPU_ARCH_INTEL_NETBURST					, 100			}, // Intel Pentium 4 (130nm)
	{ 0x0F, 0x02, 0   , 0,	CPU_ARCH_INTEL_NETBURST					, 100			}, // Intel Pentium 4 (130nm) 
	{ 0x0F, 0x03, 0   , 0,	CPU_ARCH_INTEL_NETBURST					, 100			}, // Intel Pentium 4, Celeron D (90nm)   
	{ 0x0F, 0x04, 0   , 0,	CPU_ARCH_INTEL_NETBURST					, 100			}, // Intel Pentium 4, Pentium D, Celeron D (90nm) 
	{ 0x0F, 0x06, 0   , 0,	CPU_ARCH_INTEL_NETBURST					, 100			}, // Intel Pentium 4, Pentium D, Celeron D (65nm) 

	{ 0x06, 0x0F, 0x06, 2,	CPU_ARCH_INTEL_CORE						, 90			}, // Intel Core 2 (65nm) dual core B2
	{ 0x06, 0x0F, 0x06, 4,	CPU_ARCH_INTEL_CORE						, 100			}, // Intel Core 2 (65nm) quad core B2
	{ 0x06, 0x0F, 0x06, 0,	CPU_ARCH_INTEL_CORE						, 95			}, // Intel Core 2 (65nm) B2
	{ 0x06, 0x0F, 0x0B, 0,	CPU_ARCH_INTEL_CORE						, 100			}, // Intel Core 2 (65nm) G0
	{ 0x06, 0x0F, 0x0D, 0,	CPU_ARCH_INTEL_CORE						, 95			}, // Intel Core 2 (65nm) M0 
	{ 0x06, 0x0F, 0   , 0,	CPU_ARCH_INTEL_CORE						, 95			}, // Intel Core 2 (65nm)
	{ 0x06, 0x17, 0   , 0,	CPU_ARCH_INTEL_CORE						, 100			}, // Intel Core 2 (45nm)

	{ 0x06, 0x1C, 0x02, 0,	CPU_ARCH_INTEL_ATOM						, 90			}, // Intel Atom (45nm) C0
	{ 0x06, 0x1C, 0x0A, 0,	CPU_ARCH_INTEL_ATOM						, 100			}, // Intel Atom (45nm) A0, B0
	{ 0x06, 0x1C, 0   , 0,	CPU_ARCH_INTEL_ATOM						, 90			}, // Intel Atom (45nm)

	{ 0x06, 0x1A, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Core i7 LGA1366 (45nm)
	{ 0x06, 0x1E, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Core i5, i7 LGA1156 (45nm)
	{ 0x06, 0x1F, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Core i5, i7 
	{ 0x06, 0x25, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Core i3, i5, i7 LGA1156 (32nm)
	{ 0x06, 0x2C, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Core i7 LGA1366 (32nm) 6 Core
	{ 0x06, 0x2E, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Xeon 7500 (45nm)
	{ 0x06, 0x2F, 0   , 0,	CPU_ARCH_INTEL_NEHALEM					, 0				}, // Intel Xeon (32nm)

	{ 0x06, 0x2A, 0   , 0,	CPU_ARCH_INTEL_SANDYBRIDGE				, 0				}, // Intel Core i5, i7 2xxx LGA1155 (32nm)
	{ 0x06, 0x2D, 0   , 0,	CPU_ARCH_INTEL_SANDYBRIDGE				, 0				}, // Intel Xeon, i7 3xxx LGA2011 (32nm)

	{ 0x06, 0x3A, 0   , 0,	CPU_ARCH_INTEL_IVYBRIDGE				, 0				}, // Intel Core i5, i7 3xxx LGA1155 (22nm)
	{ 0x06, 0x3E, 0   , 0,	CPU_ARCH_INTEL_IVYBRIDGE				, 0				}, // Intel Core i7 4xxx LGA2011 (22nm)

	{ 0x06, 0x3C, 0   , 0,	CPU_ARCH_INTEL_HASWELL					, 0				}, // Intel Core i5, i7 4xxx LGA1150 (22nm)              
	{ 0x06, 0x3F, 0   , 0,	CPU_ARCH_INTEL_HASWELL					, 0				}, // Intel Xeon E5-2600/1600 v3, Core i7-59xx LGA2011-v3, Haswell-E (22nm)
	{ 0x06, 0x45, 0   , 0,	CPU_ARCH_INTEL_HASWELL					, 0				}, // Intel Core i5, i7 4xxxU (22nm)              
	{ 0x06, 0x46, 0   , 0,	CPU_ARCH_INTEL_HASWELL					, 0				}, 

	{ 0x06, 0x3D, 0   , 0,	CPU_ARCH_INTEL_BROADWELL				, 0				}, // Intel Core M-5xxx (14nm)
	{ 0x06, 0x47, 0   , 0,	CPU_ARCH_INTEL_BROADWELL				, 0				}, // Intel i5, i7 5xxx, Xeon E3-1200 v4 (14nm)
	{ 0x06, 0x4F, 0   , 0,	CPU_ARCH_INTEL_BROADWELL				, 0				}, // Intel Xeon E5-26xx v4
	{ 0x06, 0x56, 0   , 0,	CPU_ARCH_INTEL_BROADWELL				, 0				}, // Intel Xeon D-15xx

	{ 0x06, 0x36, 0   , 0,	CPU_ARCH_INTEL_ATOM						, 0				}, // Intel Atom S1xxx, D2xxx, N2xxx (32nm)

	{ 0x06, 0x37, 0   , 0,	CPU_ARCH_INTEL_SILVERMONT				, 0				}, // Intel Atom E3xxx, Z3xxx (22nm)
	{ 0x06, 0x4A, 0   , 0,	CPU_ARCH_INTEL_SILVERMONT				, 0				}, // Intel Atom Z3xxx (22nm)
	{ 0x06, 0x4D, 0   , 0,	CPU_ARCH_INTEL_SILVERMONT				, 0				}, // Intel Atom C2xxx (22nm)
	{ 0x06, 0x5D, 0   , 0,	CPU_ARCH_INTEL_SILVERMONT				, 0				}, // Intel Atom x3-C3xxx (22nm) 

	{ 0x06, 0x4E, 0   , 0,	CPU_ARCH_INTEL_SKYLAKE					, 0				},
	{ 0x06, 0x5E, 0   , 0,	CPU_ARCH_INTEL_SKYLAKE					, 0				}, // Intel Core i5, i7 6xxxx LGA1151 (14nm)
	{ 0x06, 0x55, 0   , 0,	CPU_ARCH_INTEL_SKYLAKE					, 0				}, // Intel Core i7, i9 7xxxx LGA2066 (14nm)

	{ 0x06, 0x4C, 0   , 0,	CPU_ARCH_INTEL_AIRMONT					, 0				}, // Intel Atom (14nm)

	{ 0x06, 0x8E, 0   , 0,	CPU_ARCH_INTEL_KABYLAKE					, 0				}, 
	{ 0x06, 0x9E, 0   , 0,	CPU_ARCH_INTEL_KABYLAKE					, 0				}, // Intel Core i5, i7 7xxxx (14nm), 8xxxx (14nm++) 

	{ 0x06, 0x5C, 0   , 0,	CPU_ARCH_INTEL_GOLDMONT					, 0				}, // Intel Atom (14nm)
	{ 0x06, 0x5F, 0   , 0,	CPU_ARCH_INTEL_GOLDMONT					, 0				}, // Intel Atom (14nm)

	{ 0x06, 0x7A, 0   , 0,	CPU_ARCH_INTEL_GOLDMONTPLUS				, 0				}, // Intel Atom (14nm)

	{ 0x06, 0x66, 0   , 0,	CPU_ARCH_INTEL_CANNONLAKE				, 0				}, // Intel Core i3 8121U (10nm)

	{ 0x06, 0x7D, 0   , 0,	CPU_ARCH_INTEL_ICELAKE					, 0				}, // Intel Core i3, i5, i7 10xxGx (10nm) 
	{ 0x06, 0x7E, 0   , 0,	CPU_ARCH_INTEL_ICELAKE					, 0				}, // Intel Core i3, i5, i7 10xxGx (14nm) 
	{ 0x06, 0x6A, 0   , 0,	CPU_ARCH_INTEL_ICELAKE					, 0				}, // Intel Xeon (10nm)
	{ 0x06, 0x6C, 0   , 0,	CPU_ARCH_INTEL_ICELAKE					, 0				},

	{ 0x06, 0xA5, 0   , 0,	CPU_ARCH_INTEL_COMETLAKE				, 0				},  
	{ 0x06, 0xA6, 0   , 0,	CPU_ARCH_INTEL_COMETLAKE				, 0				}, // Intel Core i3, i5, i7 10xxx (14nm)

	{ 0x06, 0x86, 0   , 0,	CPU_ARCH_INTEL_TREMONT					, 0				}, // Intel Atom (10nm)

	{ 0x06, 0x9C, 0   , 0,	CPU_ARCH_INTEL_JASPERLAKE				, 0				}, // Intel Atom (10nm)

	{ 0x06, 0x8C, 0   , 0,	CPU_ARCH_INTEL_TIGERLAKE				, 0				}, // Intel Atom (10nm++)
	{ 0x06, 0x8D, 0   , 0,	CPU_ARCH_INTEL_TIGERLAKE				, 0				},

	{ 0x06, 0xA7, 0   , 0,	CPU_ARCH_INTEL_ROCKETLAKE				, 0				}, // Intel Core i3, i5, i7, i9 11xxx (14nm)

	{ 0x06, 0x97, 0   , 0,	CPU_ARCH_INTEL_ALDERLAKE				, 0				}, // Intel Core i3, i5, i7, i9 12xxx (10nm)
	{ 0x06, 0x9A, 0   , 0,	CPU_ARCH_INTEL_ALDERLAKE				, 0				}, 
	{ 0x06, 0xBE, 0   , 0,	CPU_ARCH_INTEL_ALDERLAKE				, 0				}, 

	{ 0x06, 0xB7, 0   , 0,	CPU_ARCH_INTEL_RAPTORLAKE				, 0				}, // Intel Core i3, i5, i7, i9 13xxx (7nm)
	{ 0x06, 0xBA, 0   , 0,	CPU_ARCH_INTEL_RAPTORLAKE				, 0				},
	{ 0x06, 0xBF, 0   , 0,	CPU_ARCH_INTEL_RAPTORLAKE				, 0				},

	{ 0x06, 0xC6, 0   , 0,	CPU_ARCH_INTEL_ARROWLAKE				, 0				}, // Intel Core Ultra 5, 7, 9 2xx 
};
/////////////////////////////////////////////////////////////////////////////
CPU_ARCH_DESC g_arrAMDCpuArch[] =
{
	{ 0x10, 0	, 0   , 0, CPU_ARCH_AMD_FAMILY_10H					, 0x12031022	},	//AMD K10

	{ 0x11, 0	, 0   , 0, CPU_ARCH_AMD_FAMILY_11H					, 0x13031022	},	//AMD Turion X2 Ultra

	{ 0x12, 0	, 0   , 0, CPU_ARCH_AMD_FAMILY_12H					, 0x17031022	},	//AMD K10 Llano

	{ 0x14, 0	, 0   , 0, CPU_ARCH_AMD_FAMILY_14H					, 0x17031022	},	//AMD Bobcat

	{ 0x15, 0x00, 0   , 0, CPU_ARCH_AMD_FAMILY_15H_MODEL_00			, 0x16031022	},	//AMD Bulldozer
	{ 0x15, 0x10, 0   , 0, CPU_ARCH_AMD_FAMILY_15H_MODEL_10			, 0x14031022	},	//AMD Piledriver
	{ 0x15, 0x30, 0   , 0, CPU_ARCH_AMD_FAMILY_15H_MODEL_30			, 0x141D1022	},	//AMD Steamroller

	{ 0x16, 0x00, 0   , 0, CPU_ARCH_AMD_FAMILY_16H_MODEL_00			, 0x15331022	},	//AMD Jaguar
	{ 0x16, 0x30, 0   , 0, CPU_ARCH_AMD_FAMILY_16H_MODEL_30			, 0x15831022	},	//AMD Puma

	{ 0x17, 0x00, 0   , 0, CPU_ARCH_AMD_FAMILY_17H_MODEL_00			, 0				}, //AMD Ryzen 1xxx/2xxx
	{ 0x17, 0x10, 0   , 0, CPU_ARCH_AMD_FAMILY_17H_MODEL_10			, 0				}, //AMD Ryzen 2xxx/3xxx APU/mobile
	{ 0x17, 0x30, 0   , 0, CPU_ARCH_AMD_FAMILY_17H_MODEL_30			, 0				}, //AMD Threadripper 3xxx
	{ 0x17, 0x60, 0   , 0, CPU_ARCH_AMD_FAMILY_17H_MODEL_60			, 0				}, //AMD Ryzen 4xxx APU/mobile
	{ 0x17, 0x70, 0   , 0, CPU_ARCH_AMD_FAMILY_17H_MODEL_70			, 0				}, //AMD Ryzen 3xxx 

	{ 0x19, 0x20, 0   , 0, CPU_ARCH_AMD_FAMILY_19H_MODEL_20			, 0				}, //AMD Ryzen 5xxx
	{ 0x19, 0x40, 0   , 0, CPU_ARCH_AMD_FAMILY_19H_MODEL_40			, 0				}, //AMD Ryzen 6xxx
	{ 0x19, 0x50, 0   , 0, CPU_ARCH_AMD_FAMILY_19H_MODEL_40			, 0				}, //AMD Ryzen 5xxx APU/mobile
	{ 0x19, 0x60, 0   , 0, CPU_ARCH_AMD_FAMILY_19H_MODEL_60			, 0				}, //AMD Ryzen 7xxx
	{ 0x19, 0x70, 0   , 0, CPU_ARCH_AMD_FAMILY_19H_MODEL_70			, 0				}, //AMD Ryzen 7xxx APU/mobile

	{ 0x1A, 0x40, 0   , 0, CPU_ARCH_AMD_FAMILY_1AH_MODEL_40			, 0				}  //AMD Ryzen 9xxx
};
/////////////////////////////////////////////////////////////////////////////
READ_BUS_DATA_ULONG_PROC	g_pReadBusDataUlong				= NULL;
READ_BUS_DATA_USHORT_PROC	g_pReadBusDataUshort			= NULL;
READ_BUS_DATA_UCHAR_PROC	g_pReadBusDataUchar				= NULL;

WRITE_BUS_DATA_ULONG_PROC	g_pWriteBusDataUlong			= NULL;
WRITE_BUS_DATA_USHORT_PROC	g_pWriteBusDataUshort			= NULL;	
WRITE_BUS_DATA_UCHAR_PROC	g_pWriteBusDataUchar			= NULL;

READ_MSR_PROC				g_pReadMSR						= NULL;
WRITE_MSR_PROC				g_pWriteMSR						= NULL;

GET_TIMESTAMP_PROC			g_pGetTimestamp					= NULL;

HINSTANCE					g_hModule						= 0;

BOOL						g_bEnableLog					= FALSE;

DWORD						g_dwCpuCount					= 0;
char						g_szCpuString[0x20]				= { 0 };
char						g_szCpuBrandString[0x40]		= { 0 };
DWORD						g_dwCpuStepping					= 0;
DWORD						g_dwCpuModel					= 0;
DWORD						g_dwCpuModelExt					= 0;
DWORD						g_dwCpuFamily					= 0;
DWORD						g_dwCpuFamilyExt				= 0;
DWORD						g_dwCpuArch						= 0;

CCPUTopology				g_cpuTopology;
CTimestampedDataCache		g_cpuTemperatureCache;

//Intel CPU architecture specific variables

DWORD						g_dwTjmaxOverride				= 0;
DWORD						g_dwTjmax[MAX_CPU]				= { 0 };

//AMD CPU architecture specific variables

DWORD						g_dwMiscControlDevFn[MAX_CPU]	= { 0 };
//////////////////////////////////////////////////////////////////////
// This helper function is used to check if CPU is Intel
//////////////////////////////////////////////////////////////////////
BOOL IsIntelCpu()
{
	return !_stricmp(g_szCpuString, "GenuineIntel");
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to check if CPU is AMD
//////////////////////////////////////////////////////////////////////
BOOL IsAMDCpu()
{
	return !_stricmp(g_szCpuString, "AuthenticAMD");
}
//////////////////////////////////////////////////////////////////////
BOOL IsAMDZenArchitecture()
{
	if ((g_dwCpuArch == CPU_ARCH_AMD_FAMILY_17H_MODEL_00) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_17H_MODEL_10) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_17H_MODEL_30) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_17H_MODEL_60) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_17H_MODEL_70) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_19H_MODEL_20) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_19H_MODEL_40) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_19H_MODEL_50) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_19H_MODEL_60) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_19H_MODEL_70) ||
		(g_dwCpuArch == CPU_ARCH_AMD_FAMILY_1AH_MODEL_40))
		return TRUE;

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
// This helper function is returning CPU string
//////////////////////////////////////////////////////////////////////
LPCSTR GetCpuString()
{
	return g_szCpuString;
}
//////////////////////////////////////////////////////////////////////
// This helper function is returning CPU brand string
//////////////////////////////////////////////////////////////////////
LPCSTR GetCpuBrandString()
{
	return g_szCpuBrandString;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to read DWORD from PCR register 
// if host application supports PCI configuration space access 
//////////////////////////////////////////////////////////////////////
DWORD ReadBusDataUlong(DWORD bus, DWORD devfn, DWORD offset)
{
	if (g_pReadBusDataUlong)
		return g_pReadBusDataUlong(bus, devfn, offset);

	return 0;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to read WORD from PCR register 
// if host application supports PCI configuration space access 
//////////////////////////////////////////////////////////////////////
WORD ReadBusDataUshort(DWORD bus, DWORD devfn, DWORD offset)
{
	if (g_pReadBusDataUshort)
		return g_pReadBusDataUshort(bus, devfn, offset);

	return 0;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to read BYTE from PCR register 
// if host application supports PCI configuration space access
//////////////////////////////////////////////////////////////////////
BYTE ReadBusDataUchar(DWORD bus, DWORD devfn, DWORD offset)
{
	if (g_pReadBusDataUchar)
		return g_pReadBusDataUchar(bus, devfn, offset);

	return 0;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to write DWORD to PCR register 
// if host application supports PCI configuration space access 
//////////////////////////////////////////////////////////////////////
void WriteBusDataUlong(DWORD bus, DWORD devfn, DWORD offset, DWORD data)
{
	if (g_pWriteBusDataUlong)
		g_pWriteBusDataUlong(bus, devfn, offset, data);
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to write WORD to PCR register 
// if host application supports PCI configuration space access 
//////////////////////////////////////////////////////////////////////
void WriteBusDataUshort(DWORD bus, DWORD devfn, DWORD offset, WORD data)
{
	if (g_pWriteBusDataUshort)
		g_pWriteBusDataUshort(bus, devfn, offset, data);
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to write BYTE to PCR register 
// if host application supports PCI configuration space access 
//////////////////////////////////////////////////////////////////////
void WriteBusDataUchar(DWORD bus, DWORD devfn, DWORD offset, BYTE data)
{
	if (g_pWriteBusDataUchar)
		g_pWriteBusDataUchar(bus, devfn, offset, data);
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to read MSR register if host application
// supports MSR access
//////////////////////////////////////////////////////////////////////
BOOL ReadMSR(DWORD dwIndex, LPDWORD lpHigh, LPDWORD lpLow)
{
	if (g_pReadMSR)
		return g_pReadMSR(dwIndex, lpHigh, lpLow);

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to write MSR register if host application
// supports MSR access
//////////////////////////////////////////////////////////////////////
BOOL WriteMSR(DWORD dwIndex, DWORD dwHigh, DWORD dwLow)
{
	if (g_pWriteMSR)
		return g_pWriteMSR(dwIndex, dwHigh, dwLow);

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to read hardware monitoring timestamp
// if host application supports reporting it
//////////////////////////////////////////////////////////////////////
DWORD GetTimestamp()
{
	if (g_pGetTimestamp)
		return g_pGetTimestamp();

	return 0;
}
//////////////////////////////////////////////////////////////////////
// This helper function is returning CPU family
//////////////////////////////////////////////////////////////////////
DWORD GetCpuFamily()
{
	if (IsIntelCpu())
	//	http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf
	//
	// The Extended Family ID needs to be examined only when the Family ID is 0FH. Integrate the fields into a display
	// using the following rule:
	// IF Family_ID != 0FH
	// THEN DisplayFamily = Family_ID;
	// ELSE DisplayFamily = Extended_Family_ID + Family_ID;
	// (* Right justify and zero-extend 4-bit field. *)
	// FI;
	// (* Show DisplayFamily as HEX field. *)
	{
		if (g_dwCpuFamily != 0x0F)
			return g_dwCpuFamily;

		return g_dwCpuFamily + g_dwCpuFamilyExt;
	}

	if (IsAMDCpu())
	// http://support.amd.com/TechDocs/25481.pdf
	//
	// The Family is an 8-bit value and is defined as: Family[7:0] = ({0000b,BaseFamily[3:0]} + ExtendedFamily[7:0]).
	// For example, if BaseFamily[3:0] = Fh and ExtendedFamily[7:0] = 01h, then Family[7:0] = 10h. If
	// BaseFamily[3:0] is less than Fh then ExtendedFamily[7:0] is reserved and Family is equal to BaseFamily[3:0].
	{
		if (g_dwCpuFamily < 0x0F)
			return g_dwCpuFamily;

		return g_dwCpuFamily + g_dwCpuFamilyExt;
	}

	return g_dwCpuFamily;
}
//////////////////////////////////////////////////////////////////////
// This helper function is returning CPU model
//////////////////////////////////////////////////////////////////////
DWORD GetCpuModel()
{
	if (IsIntelCpu())
	//	http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf
	//
	// The Extended Model ID needs to be examined only when the Family ID is 06H or 0FH. Integrate the field into a
	// display using the following rule:
	// IF (Family_ID = 06H or Family_ID = 0FH)
	// THEN DisplayModel = (Extended_Model_ID < 4) + Model_ID;
	// (* Right justify and zero-extend 4-bit field; display Model_ID as HEX field.*)
	// ELSE DisplayModel = Model_ID;
	// FI;
	// (* Show DisplayModel as HEX field. *)
	{
		if ((g_dwCpuFamily == 0x06) ||
			(g_dwCpuFamily == 0x0F))
			return g_dwCpuModel + (g_dwCpuModelExt<<4);

		return g_dwCpuModel;
	}

	if (IsAMDCpu())
	// http://support.amd.com/TechDocs/25481.pdf
	//
	//Model is an 8-bit value and is defined as: Model[7:0] = {ExtendedModel[3:0],BaseModel[3:0]}. For example,
	//if ExtendedModel[3:0] = Eh and BaseModel[3:0] = 8h, then Model[7:0] = E8h. If BaseFamily[3:0] is less than
	//0Fh then ExtendedModel[3:0] is reserved and Model is equal to BaseModel[3:0].
	{
		if (g_dwCpuFamily < 0x0F)
			return g_dwCpuModel;

		return g_dwCpuModel + (g_dwCpuModelExt<<4);
	}

	return g_dwCpuModel;
}
//////////////////////////////////////////////////////////////////////
// This helper function is returning CPU stepping
//////////////////////////////////////////////////////////////////////
DWORD GetCpuStepping()
{
	return g_dwCpuStepping;
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to init AMD CPU specific variables
// 
// Source : https://github.com/openhardwaremonitor/openhardwaremonitor/blob/master/Hardware/CPU/AMD10CPU.cs
//////////////////////////////////////////////////////////////////////
void InitAMDCpu()
{
	DWORD dwCpuFamily	= GetCpuFamily();
	DWORD dwCpuModel	= GetCpuModel();

	if (IsAMDCpu())
	{
		//init CPU architecture and DeviceID for miscellaneous control PCI device 18H function 3 used for temperature reporting on pre-Zen CPU architectures

		DWORD dwDeviceID = 0;

		for (DWORD dwArch = 0; dwArch < _countof(g_arrAMDCpuArch); dwArch++)
		{
			if (g_arrAMDCpuArch[dwArch].dwFamily != dwCpuFamily)
				//select architecture descriptor with matching family
				continue;

			if (dwCpuFamily >= 0x15)
				//select architecture descriptor with matching model (for Family 15h and newer architecures only)
			{
				if (g_arrAMDCpuArch[dwArch].dwModel != (dwCpuModel & 0xF0))
					continue;
			}

			g_dwCpuArch = g_arrAMDCpuArch[dwArch].dwArchitecture;
			dwDeviceID = g_arrAMDCpuArch[dwArch].dwProperty;
		}

		if (g_bEnableLog)
		{
			CString strLog;
			strLog.Format("Miscellaneous control PCI DeviceID : %X", dwDeviceID);
			AppendLog(strLog, TRUE);
		}

		if (dwDeviceID)
			//if we detected supported CPU architecture, miscellaneous control PCI DeviceID is not zero
		{
			for (DWORD dwCpu = 0; dwCpu < min(g_dwCpuCount, 8); dwCpu++)
				//init miscellaneous control PCI location for each physical CPU

				//Note: we don't need to detect real number of physical CPUs, because each physical CPU has independent
				//miscellaneous control device in PCI configuration space
			{
				DWORD dwDev = 0x18 + dwCpu;
				DWORD dwFn = 0x03;

				DWORD dwDevFn = (dwDev << 3) | dwFn;

				DWORD dwCpuDeviceID = ReadBusDataUlong(0, dwDevFn, 0);
				//try to read each physical CPU miscellaneous control PCI DeviceID from PCI configuration space

				if (dwCpuDeviceID == dwDeviceID)
					//if it matches expected miscellaneous control PCI DeviceID, then we initialized it properly 
					g_dwMiscControlDevFn[dwCpu] = dwDevFn;

				if (g_bEnableLog)
				{
					CString strLog;
					strLog.Format("CPU%d bus 0, device %X, function %X : %X", dwCpu, dwDev, dwFn, dwCpuDeviceID);
					AppendLog(strLog, TRUE);
				}

				if (dwCpuDeviceID == 0xFFFFFFFF)
					//if we couldn't read physical CPU miscellaneous control PCI DeviceID then we finished enumerating
					//physical CPUs
					break;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////
// This helper function is used to init Intel CPU specific variables
//////////////////////////////////////////////////////////////////////
void InitIntelCpu()
{
	DWORD dwCpuFamily	= GetCpuFamily();
	DWORD dwCpuModel	= GetCpuModel();
	DWORD dwCpuStepping	= GetCpuStepping();

	if (IsIntelCpu())
		//detect Intel CPU architecture and maximum junction temperature for each logical processor
	{
		for (DWORD dwArch=0; dwArch<_countof(g_arrIntelCpuArch); dwArch++)
		{
			if (g_arrIntelCpuArch[dwArch].dwFamily != dwCpuFamily)
				//select architecture descriptor with matching family
				continue;

			if (g_arrIntelCpuArch[dwArch].dwModel != dwCpuModel)
				//select architecture descriptor with matching model
				continue;

			if (g_arrIntelCpuArch[dwArch].dwStepping)
				//select architecture descriptor with matching stepping (this field is optional and can be zero, if it is not important)
			{
				if (g_arrIntelCpuArch[dwArch].dwStepping != dwCpuStepping)
					continue;
			}

			if (g_arrIntelCpuArch[dwArch].dwNumCores)
				//select architecture descriptor with matching number of cores (this field is optional and can be zero, if it is not important)
			{
				if (g_arrIntelCpuArch[dwArch].dwNumCores != g_dwCpuCount)
					continue;
			}

			g_dwCpuArch	= g_arrIntelCpuArch[dwArch].dwArchitecture;

			//NOTE: we use future proof per-core Tjmax definition, which is not actually necessary in current Intel thermal monitoring implementation

			for (DWORD dwCpu=0; dwCpu<g_dwCpuCount; dwCpu++)
			{
				if (g_arrIntelCpuArch[dwArch].dwProperty)
				{
					g_dwTjmax[dwCpu] = g_arrIntelCpuArch[dwArch].dwProperty;

					if (g_bEnableLog)
					{
						CString strLog;
						strLog.Format("CPU%d Tjmax : %d°C hardcoded", dwCpu, g_dwTjmax[dwCpu]);
						AppendLog(strLog, TRUE);

					}
				}
				else
				{
					CProcessAffinityMask affinity(g_dwCpuCount, dwCpu);

					DWORD dwLow = 0;
					DWORD dwHigh = 0;

					if (ReadMSR(0x1A2/*MSR_TEMPERATURE_TARGET*/, &dwHigh, &dwLow))
						g_dwTjmax[dwCpu] = (dwLow >> 16) & 0x7F;

					if (g_bEnableLog)
					{
						CString strLog;
						strLog.Format("CPU%d Tjmax : %d°C", dwCpu, g_dwTjmax[dwCpu]);
						AppendLog(strLog, TRUE);

					}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to init vendor agnostic and vendor specific 
// CPU variables
/////////////////////////////////////////////////////////////////////////////
void InitCpu()
{
	//init CPU count

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo); 

	g_dwCpuCount				= sysInfo.dwNumberOfProcessors;

	//init CPU model related variables

	int cpuInfo[4]	= { 0 };

	__cpuid(cpuInfo, 0);

	ZeroMemory(g_szCpuString, sizeof(g_szCpuString));
	ZeroMemory(g_szCpuBrandString, sizeof(g_szCpuBrandString));
	*((int*)g_szCpuString)		= cpuInfo[1];
	*((int*)(g_szCpuString+4))	= cpuInfo[3];
	*((int*)(g_szCpuString+8))	= cpuInfo[2];

	__cpuid(cpuInfo, 1);

	g_dwCpuStepping				= (cpuInfo[0]	 )&0x0F;
	g_dwCpuModel				= (cpuInfo[0]>>4 )&0x0F;
	g_dwCpuFamily				= (cpuInfo[0]>>8 )&0x0F;

	g_dwCpuModelExt				= (cpuInfo[0]>>16)&0x0F;
	g_dwCpuFamilyExt			= (cpuInfo[0]>>20)&0xFF;

	__cpuid(cpuInfo, 0x80000000);

	int nMaxEx = cpuInfo[0];

	if  (nMaxEx >= 0x80000002)
	{
		__cpuid(cpuInfo, 0x80000002);

		CopyMemory(g_szCpuBrandString, cpuInfo, sizeof(cpuInfo));
	}

	if  (nMaxEx >= 0x80000003)
	{
		__cpuid(cpuInfo, 0x80000003);

		CopyMemory(g_szCpuBrandString + 16, cpuInfo, sizeof(cpuInfo));
	}

	if  (nMaxEx >= 0x80000004)
	{
		__cpuid(cpuInfo, 0x80000004);

		CopyMemory(g_szCpuBrandString + 32, cpuInfo, sizeof(cpuInfo));
	}

	if (g_bEnableLog)
	{
		CString strLog;
		strLog.Format("%s (family %X, model %X, stepping %X), %d logical processors", GetCpuBrandString(), GetCpuFamily(), GetCpuModel(), GetCpuStepping(), g_dwCpuCount);
		AppendLog(strLog, FALSE);
	}

	//init CPU topology

	g_cpuTopology.Init();

	if (g_bEnableLog)
	{
		for (DWORD dwCpu=0; dwCpu<g_dwCpuCount; dwCpu++)
		{
			CString strLog;
			strLog.Format("CPU%d : package %d, core %d", dwCpu, g_cpuTopology.GetPckg(dwCpu), g_cpuTopology.GetCore(dwCpu));
			AppendLog(strLog, TRUE);
		}
	}

	//init Intel CPU specific variables

	if (IsIntelCpu())
		InitIntelCpu();

	//init AMD CPU specific variables

	if (IsAMDCpu())
		InitAMDCpu();

	if (g_bEnableLog)
	{
		CString strLog;
		strLog.Format("CPU architecture %X detected", g_dwCpuArch);
		AppendLog(strLog, TRUE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// This helper function checks if CPU supports temperature monitoring
/////////////////////////////////////////////////////////////////////////////
BOOL IsCpuTemperatureReportingSupported(DWORD dwCpu)
{
	if ((dwCpu < g_dwCpuCount) || (dwCpu == 0xFFFFFFFF))
	{
		if (IsIntelCpu())
			return IsIntelCpuTemperatureReportingSupported(dwCpu);

		if (IsAMDCpu())
		{
			dwCpu = g_cpuTopology.GetPckg(dwCpu);

			return IsAMDCpuTemperatureReportingSupported(dwCpu);
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is returning the maximum junction temperature for 
// Intel CPUs
/////////////////////////////////////////////////////////////////////////////
DWORD GetTjmax(DWORD dwCpu)
{
	if ((dwCpu < g_dwCpuCount) || (dwCpu == 0xFFFFFFFF))
	{
		if (g_dwTjmaxOverride)
			return g_dwTjmaxOverride;

		return g_dwTjmax[(dwCpu == 0xFFFFFFFF) ? 0 : dwCpu];
			//use special handling for package temperature
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is reporting CPU temperature
/////////////////////////////////////////////////////////////////////////////
float GetCpuTemperature(DWORD dwCpu)
{
	if ((dwCpu < g_dwCpuCount) || (dwCpu == 0xFFFFFFFF))
	{
		if (IsIntelCpu())
			return GetIntelCpuTemperature(dwCpu);

		if (IsAMDCpu())
		{
			//On AMD CPUs there are no per-core thermal sensors, so we duplicate readings of the same single 
			//per-package temperature sensor for all cores. It is not necessary to reread the sensor for each
			//core, so we can save a lot of CPU time if we physically read it just once per polling iteration
			//and return precached values on subsequent calls for the rest cores.

			dwCpu = g_cpuTopology.GetPckg(dwCpu);

			//We can use host GetTimestamp function to read hardware monitoring timestamp, which is updated by
			//host on each new hardware polling iteration, so we can always determine if we need to reread the
			//sensor or can return previously cached value

			DWORD dwTimestamp = GetTimestamp();

			if (dwTimestamp)
			{
				LPTIMESTAMPED_DATA lpData = g_cpuTemperatureCache.GetData(dwCpu, dwTimestamp);

				if (lpData)
					//precached data is available for this timestamp, so we can just return it
					return (FLOAT)lpData->dwData;

				//otherwise we need to reread the sensor and add it to cache

				float fltTemperature = GetAMDCpuTemperature(dwCpu);

				g_cpuTemperatureCache.SetData(dwCpu, dwTimestamp, (DWORD)fltTemperature);

				return fltTemperature;
			}

			return GetAMDCpuTemperature(dwCpu);
		}
	}

	return FLT_MAX;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is checking if temperature reporting is supported
// on Intel CPUs
/////////////////////////////////////////////////////////////////////////////
BOOL IsIntelCpuTemperatureReportingSupported(DWORD dwCpu)
{
	if (IsIntelCpu())
	{
		if (dwCpu == 0xFFFFFFFF)
			//package temperature
		{
			int cpuInfo[4]	= { 0 };
			__cpuid(cpuInfo, 6);

			if (cpuInfo[0] & 0x40)
				return TRUE;
		}
		else
			//core temperature
		{
			CProcessAffinityMask affinity(g_dwCpuCount, dwCpu);

			int cpuInfo[4]	= { 0 };
			__cpuid(cpuInfo, 6);

			if (cpuInfo[0] & 1)
				return TRUE;
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
// This helper function is checking if temperature reporting is supported
// on AMD CPUs
/////////////////////////////////////////////////////////////////////////////
BOOL IsAMDCpuTemperatureReportingSupported(DWORD dwCpu)
{
	if (IsAMDCpu())
	{
		if (dwCpu == 0xFFFFFFFF)
			//package temperature
		{
			return FALSE;
		}
		else
			//core tempeature
		{
			if (IsAMDZenArchitecture())
			{
				if (!dwCpu)
					return TRUE;

				return FALSE;
			}
			else
			{
				if (g_dwMiscControlDevFn[dwCpu])
					return TRUE;

				return FALSE;
			}
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is reading CPU temperature on Intel CPUs
/////////////////////////////////////////////////////////////////////////////
float GetIntelCpuTemperature(DWORD dwCpu)
{
	float fltResult = FLT_MAX;

	if (IsIntelCpu())
	{
		DWORD dwTjmax = GetTjmax(dwCpu);

		if (dwTjmax)
		{
			if (dwCpu == 0xFFFFFFFF)
				//package temperature
			{
				DWORD dwLow		= 0;
				DWORD dwHigh	= 0;

				if (ReadMSR(0x1B1/*IA32_PACKAGE_THERM_STATUS*/, &dwHigh, &dwLow))
				{
					DWORD dwTdist = (dwLow>>16) & 0x7F;

					fltResult = (float)(dwTjmax - dwTdist);
				}
			}
			else
				//core temperature
			{
				CProcessAffinityMask affinity(g_dwCpuCount, dwCpu);

				DWORD dwLow		= 0;
				DWORD dwHigh	= 0;

				if (ReadMSR(0x19C/*IA32_THERM_STATUS*/, &dwHigh, &dwLow))
				{
					DWORD dwTdist = (dwLow>>16) & 0x7F;

					fltResult = (float)(dwTjmax - dwTdist);
				}
			}
		}

		return fltResult;
	}

	return FLT_MAX;
}
//////////////////////////////////////////////////////////////////////
// This helper function is reading system management network register on AMD CPUs
/////////////////////////////////////////////////////////////////////////////
DWORD GetAMDSMNRegister(DWORD dwRegister)
{
	HANDLE hMutex	= CreateMutex(NULL, FALSE, "Global\\Access_PCI");
	if (hMutex)
		WaitForSingleObject(hMutex, INFINITE);

	WriteBusDataUlong(0, 0, 0x60, dwRegister);
	DWORD dwResult = ReadBusDataUlong(0, 0, 0x64);

	if (hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return dwResult;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is reading CPU temperature on AMD CPUs
/////////////////////////////////////////////////////////////////////////////
float GetAMDCpuTemperature(DWORD dwCpu)
{
	if (IsAMDCpu())
	{
		float fltResult = FLT_MAX;

		if (dwCpu == 0xFFFFFFFF)
			//package temperature
		{
		}
		else
			//core temperature
		{
			if (IsAMDZenArchitecture())
			{
				if (!dwCpu)
				{
					DWORD dwTempControl = GetAMDSMNRegister(0x59800/*F17H_M01H_REPORTED_TEMP_CTRL_OFFSET*/);

					if ((dwTempControl & 0x80000) == 0x80000)
						fltResult = ((dwTempControl>>21) & 0x7FF) / 8.0f - 49.0f;
					else
						fltResult = ((dwTempControl>>21) & 0x7FF) / 8.0f;

					if (strstr(g_szCpuBrandString, "AMD Ryzen 7 2700X"))
						fltResult = fltResult - 10.0f;

					if (strstr(g_szCpuBrandString, "AMD Ryzen 7 1800X") ||
						strstr(g_szCpuBrandString, "AMD Ryzen 7 1700X") ||
						strstr(g_szCpuBrandString, "AMD Ryzen 5 1600X"))
						fltResult = fltResult - 20.0f;

					if (strstr(g_szCpuBrandString, "AMD Ryzen Threadripper 19") ||
						strstr(g_szCpuBrandString, "AMD Ryzen Threadripper 29"))
						fltResult = fltResult - 27.0f;
				}
			}
			else
			{
				if (g_dwMiscControlDevFn[dwCpu])
				{
					DWORD dwTempControl;

					DWORD dwCpuFamily = GetCpuFamily();

					switch (dwCpuFamily)
					{
					case 0x10:
					case 0x11:
					case 0x12:
					case 0x14:
					case 0x15:
					case 0x16:
						dwTempControl = ReadBusDataUlong(0, g_dwMiscControlDevFn[dwCpu], 0xA4);
						break;
					}

					switch (dwCpuFamily)
					{
					case 0x10:
					case 0x11:
					case 0x12:
					case 0x14:
						fltResult = ((dwTempControl>>21) & 0x7FF) / 8.0f;
						break;
					case 0x15:
						if ((dwTempControl & 0x30000) == 0x30000) 
							fltResult = ((dwTempControl>>21) & 0x7FC) / 8.0f - 49.0f;
						else
							fltResult = ((dwTempControl>>21) & 0x7FF) / 8.0f;
						break;
					case 0x16:
						if (((dwTempControl & 0x30000) == 0x30000) || 
							((dwTempControl & 0x80000) == 0x80000))
							fltResult = ((dwTempControl>>21) & 0x7FF) / 8.0f - 49.0f;
						else
							fltResult = ((dwTempControl>>21) & 0x7FF) / 8.0f;
						break;
					}
				}
			}
		}

		return fltResult;
	}

	return FLT_MAX;
}
//////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(CPUDLL, hInstance))
			return 0;

		new CDynLinkLibrary(CPUDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(CPUDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get a number of
// data sources in this plugin
//////////////////////////////////////////////////////////////////////
CPU_API DWORD GetSourcesNum()
{
	if (!g_dwCpuCount)
	{
		//load plugin settings 

		g_bEnableLog			= GetPrivateProfileInt("Debug"		, "EnableLog"		, 0, GetCfgPath());
		g_dwTjmaxOverride		= GetPrivateProfileInt("Settings"	, "TjmaxOverride"	, 0, GetCfgPath());

		//get host application module handle

		HMODULE hHost = GetModuleHandle(NULL);

		//get ptrs to required plugin API functions

		g_pReadBusDataUlong		= (READ_BUS_DATA_ULONG_PROC		)GetProcAddress(hHost, "ReadBusDataUlong"	);
		g_pReadBusDataUshort	= (READ_BUS_DATA_USHORT_PROC	)GetProcAddress(hHost, "ReadBusDataUshort"	);
		g_pReadBusDataUchar		= (READ_BUS_DATA_UCHAR_PROC		)GetProcAddress(hHost, "ReadBusDataUchar"	);

		g_pWriteBusDataUlong	= (WRITE_BUS_DATA_ULONG_PROC	)GetProcAddress(hHost, "WriteBusDataUlong"	);
		g_pWriteBusDataUshort	= (WRITE_BUS_DATA_USHORT_PROC	)GetProcAddress(hHost, "WriteBusDataUshort"	);
		g_pWriteBusDataUchar	= (WRITE_BUS_DATA_UCHAR_PROC	)GetProcAddress(hHost, "WriteBusDataUchar"	);

		g_pReadMSR				= (READ_MSR_PROC				)GetProcAddress(hHost, "ReadMSR"			);
		g_pWriteMSR				= (WRITE_MSR_PROC				)GetProcAddress(hHost, "WriteMSR"			);

		g_pGetTimestamp			= (GET_TIMESTAMP_PROC			)GetProcAddress(hHost, "GetTimestamp"		);		

		//init CPU related variables

		InitCpu();
	}

	if (g_dwCpuCount)
		//package temeprature +  core temepratures
		return 1 + g_dwCpuCount; 

	return 0;
}
//////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to get descriptor
// for the specified data source
//////////////////////////////////////////////////////////////////////
CPU_API BOOL GetSourceDesc(DWORD dwIndex, LPMONITORING_SOURCE_DESC pDesc)
{
	if (!g_dwCpuCount)
	{
		//load plugin settings 

		g_bEnableLog			= GetPrivateProfileInt("Debug"		, "EnableLog"		, 0, GetCfgPath());
		g_dwTjmaxOverride		= GetPrivateProfileInt("Settings"	, "TjmaxOverride"	, 0, GetCfgPath());

		//get host application module handle

		HMODULE hHost = GetModuleHandle(NULL);

		//get ptrs to required plugin API functions

		g_pReadBusDataUlong		= (READ_BUS_DATA_ULONG_PROC		)GetProcAddress(hHost, "ReadBusDataUlong"	);
		g_pReadBusDataUshort	= (READ_BUS_DATA_USHORT_PROC	)GetProcAddress(hHost, "ReadBusDataUshort"	);
		g_pReadBusDataUchar		= (READ_BUS_DATA_UCHAR_PROC		)GetProcAddress(hHost, "ReadBusDataUchar"	);

		g_pWriteBusDataUlong	= (WRITE_BUS_DATA_ULONG_PROC	)GetProcAddress(hHost, "WriteBusDataUlong"	);
		g_pWriteBusDataUshort	= (WRITE_BUS_DATA_USHORT_PROC	)GetProcAddress(hHost, "WriteBusDataUshort"	);
		g_pWriteBusDataUchar	= (WRITE_BUS_DATA_UCHAR_PROC	)GetProcAddress(hHost, "WriteBusDataUchar"	);

		g_pReadMSR				= (READ_MSR_PROC				)GetProcAddress(hHost, "ReadMSR"	);
		g_pWriteMSR				= (WRITE_MSR_PROC				)GetProcAddress(hHost, "WriteMSR"	);

		g_pGetTimestamp			= (GET_TIMESTAMP_PROC			)GetProcAddress(hHost, "GetTimestamp"		);		

		//init CPU related variables

		InitCpu();
	}

	if (IsCpuTemperatureReportingSupported(dwIndex - 1))
	{
		if (dwIndex)
		{
			sprintf_s(pDesc->szName				, sizeof(pDesc->szName)			, "CPU%d temperature", dwIndex);
			sprintf_s(pDesc->szGroup			, sizeof(pDesc->szGroup)		, "CPU%d", dwIndex);

			strcpy_s(pDesc->szNameTemplate		, sizeof(pDesc->szNameTemplate)	, "CPU%d temperature");
			strcpy_s(pDesc->szGroupTemplate		, sizeof(pDesc->szGroupTemplate), "CPU%d");
		}
		else
		{
			strcpy_s(pDesc->szName				, sizeof(pDesc->szName)			, "CPU temperature");
			strcpy_s(pDesc->szGroup				, sizeof(pDesc->szGroup)		, "CPU");
		}

		strcpy_s(pDesc->szUnits					, sizeof(pDesc->szUnits)		, "°C");

		pDesc->dwID			= MONITORING_SOURCE_ID_CPU_TEMPERATURE;
		pDesc->dwInstance	= dwIndex - 1;

		pDesc->fltMaxLimit	= 200.0f;
		pDesc->fltMinLimit	= 0.0f;

		return TRUE;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// This exported function is called by MSI Afterburner to poll data sources
/////////////////////////////////////////////////////////////////////////////
CPU_API FLOAT GetSourceData(DWORD dwIndex)
{
	return GetCpuTemperature(dwIndex - 1);
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to return fully qualified path to .cfg file
/////////////////////////////////////////////////////////////////////////////
CString GetCfgPath()
{
	char szCfgPath[MAX_PATH];
	GetModuleFileName(g_hModule, szCfgPath, MAX_PATH);
	
	PathRenameExtension(szCfgPath, ".cfg");

	return szCfgPath;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function is used to return fully qualified path to .log file
/////////////////////////////////////////////////////////////////////////////
CString GetLogPath()
{
	char szLogPath[MAX_PATH];
	GetModuleFileName(g_hModule, szLogPath, MAX_PATH);
	
	PathRenameExtension(szLogPath, ".log");

	return szLogPath;
}
/////////////////////////////////////////////////////////////////////////////
// This helper function adds specified line to log file
/////////////////////////////////////////////////////////////////////////////
void AppendLog(LPCSTR lpLine, BOOL bAppend)
{
	CString strLogPath = GetLogPath();

	CStdioFile logFile;

	if (bAppend)
	{
		if (!logFile.Open(strLogPath, CFile::modeWrite|CFile::shareDenyWrite))
			if (!logFile.Open(strLogPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite))
				return;
	}
	else
		if (!logFile.Open(strLogPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite))
			return;

	logFile.SeekToEnd();


	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	CString strTime; strTime.Format("%.2d-%.2d-%.2d, %.2d:%.2d:%.2d ", sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	logFile.WriteString(strTime);
	logFile.WriteString(lpLine);
	logFile.WriteString("\n");

	logFile.Close();
	logFile.Flush();
}
/////////////////////////////////////////////////////////////////////////////

