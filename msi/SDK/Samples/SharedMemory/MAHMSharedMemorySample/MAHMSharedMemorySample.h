// MAHMSharedMemorySample.h : main header file for the MAHMSHAREDMEMORYSAMPLE application
//

#if !defined(AFX_MAHMSHAREDMEMORYSAMPLE_H__5750FFA3_4F09_45FA_9D29_491BCD6B95E6__INCLUDED_)
#define AFX_MAHMSHAREDMEMORYSAMPLE_H__5750FFA3_4F09_45FA_9D29_491BCD6B95E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleApp:
// See MAHMSharedMemorySample.cpp for the implementation of this class
//

class CMAHMSharedMemorySampleApp : public CWinApp
{
public:
	CMAHMSharedMemorySampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMAHMSharedMemorySampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMAHMSharedMemorySampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAHMSHAREDMEMORYSAMPLE_H__5750FFA3_4F09_45FA_9D29_491BCD6B95E6__INCLUDED_)
