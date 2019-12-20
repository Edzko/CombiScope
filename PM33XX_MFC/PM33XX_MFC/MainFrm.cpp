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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <Setupapi.h>
#include "PM33XX_MFC.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_AQUA);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	HWND h_mApp;
	DWORD welcome, nc = sizeof(int), rType = REG_DWORD;
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	RegCreateKey(HKEY_CURRENT_USER, "Software\\TEC\\PM33XX", &hKey);
	RegCreateKey(HKEY_CURRENT_USER, "Software\\TEC\\PM33XX\\MRU", &hmruKey);
	LONG lRtn = ::RegQueryValueEx(hKey, "Welcome", 0, (LPDWORD)&rType, (BYTE*)&welcome, (DWORD*)&nc);
	if (lRtn != ERROR_SUCCESS) welcome = 1;
	// Show splash screen
	if (welcome)
	{
		RECT rc0, rcw;
		::GetClientRect(::GetDesktopWindow(), &rc0);
		h_mApp = CreateDialog(NULL, MAKEINTRESOURCE(IDD_WELCOME), NULL, NULL);
		::GetClientRect(h_mApp, &rcw);
		::SetWindowPos(h_mApp, HWND_TOPMOST, (rc0.left + rc0.right - rcw.right) / 2, (rc0.top + rc0.bottom - rcw.bottom) / 2, 0, 0, SWP_NOSIZE);
		::ShowWindow(h_mApp, SW_SHOW);
		::UpdateWindow(h_mApp);
		Sleep(2500);
		::DestroyWindow(h_mApp);
	}

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CMFCRibbonComboBox *pBaud = (CMFCRibbonComboBox*)m_wndRibbonBar.FindByID(ID_BAUD);
	pBaud->RemoveAllItems();
	for (int i = 0; i < 5; i++)
		pBaud->AddItem(m_wndView.BaudRates[i]);
	pBaud->SelectItem(3);

	GetComPorts();

	rType = REG_SZ;
	nc = MAX_FILENAME;
	//char mru[MAX_FILENAME];
	for (int i = 0; i < 10; i++)
	{
		char keyname[10] = "mru0";
		keyname[3] = '0' + i;
		//LONG lRtn = ::RegQueryValueEx(hmruKey, keyname, 0, (LPDWORD)&rType, (BYTE*)mru, (DWORD*)&nc);
		//if (lRtn != ERROR_SUCCESS) strcpy_s(mru, MAX_FILENAME, "");
		//CRecentFileList
		//CMFCRibbonApplicationButton *pMP = (CMFCRibbonApplicationButton*)m_wndRibbonBar.GetApplicationButton();
		//CMFCRibbonButton *pMRU = (CMFCRibbonButton*)pMP->FindByID(ID_FILE_MRU_FILE1);
		//pMRU->SetText(mru);
		//pBut->SetText(m_wndView.mru[i]);

		//AfxGetApp()->AddToRecentFileList(m_wndView.mru[i]);
	}


	//CString strTitlePane1;
	//CString strTitlePane2;
	//bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	//ASSERT(bNameValid);
	//bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	//ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, "", TRUE), "");
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, "", TRUE), "");
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE3, "", TRUE), "");
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE4, "", TRUE), "");
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE5, "", TRUE), "");
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE6, "", TRUE), "");

	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, "", TRUE), "");

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	return 0;
}

void CMainFrame::UpdateButtonImg(int id, int idx)
{
	CMFCRibbonButton *pBut = (CMFCRibbonButton*)m_wndRibbonBar.FindByID(id);
	pBut->SetImageIndex(idx, 1);
	m_wndRibbonBar.Invalidate();
	m_wndRibbonBar.UpdateWindow();
}

DWORD CMainFrame::GetSelectedColor()
{
	CMFCRibbonColorButton *pColorBut = (CMFCRibbonColorButton*)m_wndRibbonBar.FindByID(ID_COLOR);
	return pColorBut->GetColor();
}
void CMainFrame::SetSelectedColor()
{
	CMFCRibbonColorButton *pColorBut = (CMFCRibbonColorButton*)m_wndRibbonBar.FindByID(ID_COLOR);
	if (m_wndView.it >= 0)
		pColorBut->SetColor(m_wndView.trace[m_wndView.it].color);
	if (m_wndView.ic == 4)
		pColorBut->SetColor(m_wndView.bgCol);
	else if (m_wndView.ic >= 0)
		pColorBut->SetColor(m_wndView.Cursor[m_wndView.ic].color);
	
	m_wndRibbonBar.Invalidate();
	m_wndRibbonBar.UpdateWindow();
}

void CMainFrame::GetComPorts()
{
	CMFCRibbonComboBox *pPorts = (CMFCRibbonComboBox*)m_wndRibbonBar.FindByID(ID_PORT);
	int iLB = 0, idx = 0;
	HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	SP_DEVICE_INTERFACE_DATA interfaceData;
	ZeroMemory(&interfaceData, sizeof(interfaceData));
	interfaceData.cbSize = sizeof(interfaceData);
	pPorts->RemoveAllItems();
	for (int nDevice = 0; nDevice < 2000; nDevice++)
	{
		SP_DEVINFO_DATA devInfoData;
		ZeroMemory(&devInfoData, sizeof(devInfoData));
		devInfoData.cbSize = sizeof(devInfoData);

		BOOL nErr = SetupDiEnumDeviceInfo(hDevInfo,
			nDevice,
			&devInfoData);
		if (nErr == FALSE)
		{
			DWORD nLastErr = GetLastError();
			if (nLastErr == ERROR_NO_MORE_ITEMS)
				break;
		}
		char friendlyName[300];
		char manufName[300];
		if (SetupDiGetDeviceRegistryProperty(hDevInfo,
			&devInfoData,
			SPDRP_FRIENDLYNAME,
			NULL,
			(BYTE *)friendlyName,
			sizeof(friendlyName),
			NULL))
		{
			TRACE1("Friendlyname: %s\r\n", friendlyName);
			char *pCom = strstr(friendlyName, "(COM");
			if (pCom)
			{
				if (SetupDiGetDeviceRegistryProperty(hDevInfo,
					&devInfoData,
					SPDRP_MFG,
					NULL,
					(BYTE *)manufName,
					sizeof(manufName),
					NULL))
				{
					TRACE1("Manufacturer: %s\r\n", manufName);

					// NOTE: Intrepid ValueCAN may show up as FTDI 
					//if ((ComType == USB) || (strcmp(manufName,"FTDI")==0) || (strcmp(manufName,"ICS")==0))
					//if ((ComType == USB) || (strcmp(manufName,"ICS")==0))
					{


						pPorts->AddItem(friendlyName);


						//idx = pCOM->AddString(CString(friendlyName));
						int iCom = pCom[4] - '0';
						if (pCom[5] != ')') iCom = 10 * iCom + pCom[5] - '0';
						m_wndView.ComData[idx++] = iCom;
					}
				}
			}
		}
	}

	// If there is a recent COMPORT entry in registry that matches, then select it
	int iCOM;
	DWORD rType = REG_DWORD, nc = sizeof(int);
	LONG lRtn = ::RegQueryValueEx(hKey, "COM", 0, (LPDWORD)&rType, (BYTE*)&iCOM, (DWORD*)&nc);
	if (lRtn != ERROR_SUCCESS) iCOM = 0;
	if ((iCOM >= 0) && (pPorts->GetCount() >= iCOM))
		pPorts->SelectItem(iCOM);
	else pPorts->SelectItem(0);

}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}


void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // force Print Preview mode closed
	}
}

void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}

void CMainFrame::UpdateStatus()
{
	//m_wndStatusBar.AdjustLayout();

	CMFCRibbonStatusBarPane *pPane;
	pPane = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(0);

	CString txt;
	if (m_wndView.hCommPort == INVALID_HANDLE_VALUE)
		pPane->SetText("Not Connected");
	else
		pPane->SetText("Connected");

	pPane = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(1);
	pPane->SetText(m_wndView.CommPort);

	pPane = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(2);
	pPane->SetText(m_wndView.BaudRates[m_wndView.SelectedBaudrate]);

	pPane = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(3);
	pPane->SetText(m_wndView.pmid);

	pPane = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(4);
	pPane->SetText(m_wndView.lfn.lpstrFile);

	
}