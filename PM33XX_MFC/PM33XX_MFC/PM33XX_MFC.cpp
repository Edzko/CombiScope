// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// PM33XX_MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PM33XX_MFC.h"
#include "MainFrm.h"
#include "build.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPM33XXMFCApp

BEGIN_MESSAGE_MAP(CPM33XXMFCApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPM33XXMFCApp::OnAppAbout)
END_MESSAGE_MAP()


// CPM33XXMFCApp construction

CPM33XXMFCApp::CPM33XXMFCApp() noexcept
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("PM33XXMFC.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CPM33XXMFCApp object

CPM33XXMFCApp theApp;


// CPM33XXMFCApp initialization

BOOL CPM33XXMFCApp::InitInstance()
{
	HWND h_mApp;
	LPCSTR szWindowClass = "COMBISCOPE_WND";
	char szTitle[500];					// The title bar text

	// Check to see if there is already another instance of this App
	if ((h_mApp = FindWindow(szWindowClass, szTitle)) != 0) {
		SetForegroundWindow(h_mApp);
		return false;
	}


	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey("TEC");
	LoadStdProfileSettings(9);

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CFrameWnd* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr,
		nullptr);

	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

int CPM33XXMFCApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CPM33XXMFCApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CPM33XXMFCApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CPM33XXMFCApp customization load/save methods

void CPM33XXMFCApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CPM33XXMFCApp::LoadCustomState()
{
}

void CPM33XXMFCApp::SaveCustomState()
{
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	char txt[250];
	sprintf_s(txt, 250, "PM33XX Fluke CombiScope Interface Version 2.0.%i", __MY_BUILD);
	SetDlgItemText(IDC_ABOUTTXT1, txt);
	sprintf_s(txt, 250, "Built on %02i/%02i/%4i at %02i:%02i:%02i",
		MONTH, DAY, YEAR, COMPILE_HOUR, COMPILE_MINUTE, COMPILE_SECOND);
	SetDlgItemText(IDC_ABOUTTXT2, txt);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CPM33XXMFCApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	CWinAppEx::AddToRecentFileList(lpszPathName);
}

void CPM33XXMFCApp::GetMRU(int imru, char *fname)
{
	CString csfname = (*m_pRecentFileList)[imru];
	strcpy_s(fname, MAX_FILENAME, csfname.GetBuffer());
	csfname.ReleaseBuffer();
}