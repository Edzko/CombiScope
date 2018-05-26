// PM3394A.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PM3394A.h"
#include "Config.h"
#include "PM3394ADlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPM3394AApp

BEGIN_MESSAGE_MAP(CPM3394AApp, CWinApp)
	//{{AFX_MSG_MAP(CPM3394AApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPM3394AApp construction

CPM3394AApp::CPM3394AApp()
{
	EnableHtmlHelp();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPM3394AApp object

CPM3394AApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPM3394AApp initialization

BOOL CPM3394AApp::InitInstance()
{

	HWND FLUKEWND;
	if ((FLUKEWND=FindWindow(_T("#32770"),_T("PM3394A")))!=0) {
		// window exists; another instance of this
		// application is already running -> exit
		// perhaps should refresh:
		SetForegroundWindow(FLUKEWND);
		return false;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetRegistryKey(_T("FlukeView4"));

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// flash welcome
	int welcome = AfxGetApp()->GetProfileInt(_T(""),_T("Welcome"),2);
	if (welcome)
	{
		CDialog pWelcome;
		pWelcome.Create(IDD_WELCOME);
		pWelcome.ShowWindow(SW_SHOW);
		Sleep(1500);
		pWelcome.DestroyWindow();
		if (welcome!=1) 
			AfxGetApp()->WriteProfileInt(_T(""),_T("Welcome"),1); 
	}

	CPM3394ADlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
