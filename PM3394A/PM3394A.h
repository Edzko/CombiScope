// PM3394A.h : main header file for the PM3394A application
//

#if !defined(AFX_PM3394A_H__21E60070_549C_4F21_B79F_AFC2C7D816B7__INCLUDED_)
#define AFX_PM3394A_H__21E60070_549C_4F21_B79F_AFC2C7D816B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPM3394AApp:
// See PM3394A.cpp for the implementation of this class
//

class CPM3394AApp : public CWinApp
{
public:
	CPM3394AApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPM3394AApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPM3394AApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PM3394A_H__21E60070_549C_4F21_B79F_AFC2C7D816B7__INCLUDED_)
