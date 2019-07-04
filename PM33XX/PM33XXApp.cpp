// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// RibbonApp.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
#include <afxwin.h>
#include <atlbase.h>
#include <afxsock.h>
#include <atlimage.h>
#include <math.h>
#include "Commdlg.h"
#include "PM33XX.h"
#include "resource.h"
#include "ribbonres.h"
#include "ribbon.h"
#include "mru.h"
#include "build.h"

CComModule _Module;
extern HRESULT InitRibbon(HWND hWindowFrame);
extern void DestroyRibbon();
unsigned int SBID[5] = {3001,3002,3003,3004,3005};



#define _USEDOUBLEBUFFER_ 1

#define MAX_LOADSTRING 500
#define PI 3.14159265359F

// Global Variables:
HINSTANCE hInst;								// current instance
HWND m_hWnd;
HWND m_hStatusWnd; // Status Bar Window Handle
char szTitle[MAX_LOADSTRING];					// The title bar text
LPCSTR szWindowClass = "COMBISCOPE_WND";			// the main window class name
bool showInfo;
int left, top, width, height, infowidth; 
// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR nIDjoy, nIDwlan, nIDscan;
BOOL HaveView;  // flag to trigger the first time we have a valid window handle
int cursorFcn;
char pmid[200] = "--";
int timeout;
char tlabel[20], tunit[20];
void GetTrace(int iTrace);
HANDLE hCommPort;
char CommPort[12];
DCB	dcbCommPort;
COMMTIMEOUTS CommTimeouts;
DWORD baudRate;			// current baud rate
BYTE byteSize, parity, stopBits;			
double t, dt, y, y00, deltay;
CString  data;
HBITMAP hBmp;
TRACE_T trace[MAX_TRACE];
int cdata[35000], mdata[65000], fdata[35000];
float fft_data[35000];
int npoints; // global number of acquisition points per trace
float xres; // global scope x-res
int wndFcn;
CPoint mpoint;  // current mouse coordinates
CURSOR_T Cursor[4];
bool cursorMoving;
int ic; // selected cursor index
int it; // selected trace index
int is; // spectrum trace. 
int xgrid, ygrid;
bool myUpdate, zooming, zoomMode;
bool fftMode, xLog, yLog;
bool subSample;
CPoint zoomA, zoomB;
double viewVmin,viewVmax,viewTmin,viewTmax;
double tviewVmin,tviewVmax,tviewTmin,tviewTmax;

//CToolTipCtrl tt;
//BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

COLORREF cc[16], gridCol, lblCol, bgCol;

char DefFullName[MAX_FILENAME], DefName[MAX_FILENAME];
OPENFILENAME lfn;

char pTrace[CPYLEN];
char pTraceLine[100];

// For registry functions
char szKeyName[200], mruKey[10], msgA[200];
LONG lRet;
HKEY hKey;
DWORD dwLength;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	char mruKey[10];
	int welcome;
	CoInitialize(NULL);
	MSG msg;
	HWND h_mApp;
	HACCEL hAccelTable;

	// Check to see if there is already another instance of this App
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);    
	if ((h_mApp=FindWindow(szWindowClass,szTitle))!=0) {
		SetForegroundWindow(h_mApp);
		return false;
	}

	strcpy_s(szKeyName,200,"Software\\TEC\\PM33XX");

	// Load persistent global settings from Registry
	GetReg("","Welcome",(char*)&welcome,NULL,REG_DWORD);

	// Show splash screen
	if (welcome)
	{
		RECT rc0,rcw;
		GetClientRect(GetDesktopWindow(),&rc0);
		h_mApp = CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_WELCOME),NULL,NULL,0);
		GetClientRect(h_mApp,&rcw);
		SetWindowPos(h_mApp,HWND_TOPMOST,(rc0.left+rc0.right-rcw.right)/2,(rc0.top+rc0.bottom-rcw.bottom)/2,0,0,SWP_NOSIZE);
		ShowWindow(h_mApp,SW_SHOW);
		Sleep(2500);
		DestroyWindow(h_mApp);
	}

	// Initialize global strings
	MyRegisterClass(hInstance);

	if (!AfxWinInit(hInstance,hPrevInstance,lpCmdLine,0))
	{
		return FALSE;
	}
	// Load most recently used files
	strcpy_s(mruKey,10,"mru0");
	for (int i=0;i<10;i++)
	{
		mruKey[3] = '0'+i;
		dwLength = MAXFILENAME;
		GetReg("MRU",mruKey,mru[i],(int*)&dwLength,REG_SZ);
	}

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PM33XXACCEL));

	HMENU pSysMenu = GetSystemMenu(m_hWnd,FALSE);
	if (pSysMenu != NULL)
	{
		AppendMenu(pSysMenu, MF_SEPARATOR,NULL,NULL);
		//AppendMenu(pSysMenu, MF_STRING, IDM_SHOWTIPS, L"Show Tips");
		AppendMenu(pSysMenu, MF_STRING, IDM_ABOUT, "About PM33XX CombiScope Interface");
	}


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CoUninitialize();

	// store recently used files list
	strcpy_s(mruKey,10,"mru0");
	for (int i=0;i<10;i++)
	{
		mruKey[3] = '0'+i;
		SetReg("MRU",mruKey,mru[i],(int)strlen(mru[i]),REG_SZ);
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PM3394));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_PM3394));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{       

	hInst = hInstance; // Store instance handle in our global variable

	if(!AfxSocketInit())
	{
		AfxMessageBox("Failed to Initialize Sockets",MB_OK| MB_ICONSTOP);
		return FALSE;
	}

	m_hWnd = CreateWindow(szWindowClass, szTitle, 
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL,
		hInstance, NULL);
	if (!m_hWnd)
	{
		return FALSE;
	}

   m_hStatusWnd = CreateStatusWindow(WS_CHILD | WS_VISIBLE, _T(""), m_hWnd, ID_STATUS);

	HRESULT hr = InitRibbon(m_hWnd);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//myUpdate = true;
	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return TRUE;
}

void getCursorValue()
{
	Cursor[0].value = viewVmin + ((height - ((float)Cursor[0].pos - top + topMargin + 15)) / height) * (viewVmax - viewVmin) + y00;   // y1
	Cursor[1].value = viewVmin + ((height - ((float)Cursor[1].pos - top + topMargin + 15)) / height) * (viewVmax - viewVmin) + y00;   // y2

	Cursor[2].value = viewTmin + (viewTmax - viewTmin) * (Cursor[2].pos - left - 10) / width;  // t1
	Cursor[3].value = viewTmin + (viewTmax - viewTmin) * (Cursor[3].pos - left - 10) / width;  // t2
	TRACE2("Value(0) = %f, Pos = %i\r\n",Cursor[1].value,Cursor[1].pos);

	if (Cursor[0].value<viewVmin) Cursor[0].value=viewVmin; else if (Cursor[0].value>viewVmax) Cursor[0].value=viewVmax;
	if (Cursor[1].value<viewVmin) Cursor[1].value=viewVmin; else if (Cursor[1].value>viewVmax) Cursor[1].value=viewVmax;

	if (Cursor[2].value<viewTmin) Cursor[2].value=viewTmin; else if (Cursor[2].value>viewTmax) Cursor[2].value=viewTmax;
	if (Cursor[3].value<viewTmin) Cursor[3].value=viewTmin; else if (Cursor[3].value>viewTmax) Cursor[3].value=viewTmax;
}
void getCursorPos()
{
	Cursor[0].pos = (int)(top + (height - (Cursor[0].value - viewVmin - y00) /  (viewVmax - viewVmin)*height) ) ;
	Cursor[1].pos = (int)(top + (height - (Cursor[1].value - viewVmin - y00) /  (viewVmax - viewVmin)*height) ) ;

	Cursor[2].pos = left +  (Cursor[2].value - viewTmin) / (viewTmax - viewTmin) * width;
	Cursor[3].pos = left +  (Cursor[3].value - viewTmin) / (viewTmax - viewTmin) * width;
	TRACE2("Pos(2) = %i, width=%i\r\n",Cursor[2].pos,width);

	if (Cursor[0].pos<top) Cursor[0].pos=top; else if (Cursor[0].pos>top+height) Cursor[0].pos=top+height;
	if (Cursor[1].pos<top) Cursor[1].pos=top; else if (Cursor[1].pos>top+height) Cursor[1].pos=top+height;

	if (Cursor[2].pos<left) Cursor[2].pos=left; else if (Cursor[2].pos>left+width) Cursor[2].pos=left+width;
	if (Cursor[3].pos<left) Cursor[3].pos=left; else if (Cursor[3].pos>left+width) Cursor[3].pos=left+width;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
	case WM_SYSCOMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_CREATE:
		Init();
		UpdateStatus();
		break;

	case WM_LBUTTONDBLCLK:
		{
			CPoint point;
			//point.x = (LONG)((double)GET_X_LPARAM(lParam)/zoomscale); 
			//point.y = (LONG)((double)(GET_Y_LPARAM(lParam)-topMargin)/zoomscale); 
			//OnLButtonDblClk((int)wParam,point);
			HDC mfdc; 
			HDC dc, bdc, hScreen;
			HBITMAP bitmap;
			HBRUSH wndBrush;
			CRect wndrect;
			
			wndBrush = CreateSolidBrush(RGB(255,255,255));
			dc = GetDC(m_hWnd);
			GetClientRect(m_hWnd,&wndrect);
			int w=wndrect.Width(), h=wndrect.Height();

			// Create Enhanced Metafile
			hScreen = GetDC( NULL );
			mfdc = CreateEnhMetaFile(dc,NULL,NULL,_T("FLUKE\0COMBISCOPE\0\0"));
			if (mfdc) {
				myUpdate = false;
				Draw(mfdc,1);
			}

			// Create DIB bitmap file
			bdc = CreateCompatibleDC(dc);
			bitmap = CreateCompatibleBitmap(dc,1024,768);
			SelectObject (bdc, bitmap);
			FillRect(bdc,wndrect,wndBrush);
			myUpdate = false;
			Draw(bdc,1);

			HLOCAL hTrace;
			sprintf_s(pTrace,CPYLEN,"Time [%s]\t",tunit);
			for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
			{
				if (trace[iTrace].ntrace>0) 
				{
					sprintf_s(pTraceLine,100,"Trace %i\t",iTrace+1);
					strcat_s(pTrace,CPYLEN,pTraceLine);
				}
			}
			strcat_s(pTrace,CPYLEN,"\r\n");

			int smin = (int)(viewTmin/dt*npoints/xgrid);
			int smax = (int)(viewTmax/dt*npoints/xgrid);
			for (int i=smin;i<smax;i++)
			{
				sprintf_s(pTraceLine,100,"%1.3f\t",(viewTmin + (i-smin)*(viewTmax-viewTmin)/(smax-smin)));
				strcat_s(pTrace,CPYLEN,pTraceLine);
				for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
				{
					if (trace[iTrace].ntrace>0) 
					{
						{
							sprintf_s(pTraceLine,100,"%f\t",(float)trace[iTrace].data[i]*ygrid*deltay/51200.0);
							strcat_s(pTrace,CPYLEN,pTraceLine);
						}
					}
				}
				strcat_s(pTrace,CPYLEN,"\r\n");
			}
			hTrace = LocalAlloc(LPTR,strlen(pTrace)+1);
			memcpy(hTrace,pTrace,strlen(pTrace)+1);				

			// Fill clipboard
			HENHMETAFILE hmf;
			if( OpenClipboard(m_hWnd) )
			{
				EmptyClipboard();

				SetClipboardData (CF_TEXT, hTrace);
				
				SetClipboardData (CF_BITMAP, bitmap ) ;
				
				if (mfdc) 
				{
					hmf = CloseEnhMetaFile(mfdc);
					if (hmf)
					{
						SetClipboardData (CF_ENHMETAFILE, hmf);
						CloseClipboard();
					}
				}
			}

			ReleaseDC(NULL, hScreen);
			if (mfdc)
			{
				if (hmf == NULL) 
					DeleteEnhMetaFile(hmf);
				DeleteDC(mfdc);
			}
			DeleteObject(wndBrush);
			DeleteDC(bdc);
			DeleteObject(bitmap);
			ReleaseDC(m_hWnd,dc);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			CPoint point, tp;
			point.x = GET_X_LPARAM(lParam) - 28; 
			point.y = GET_Y_LPARAM(lParam) - topMargin - 20; 
			
			int e = 1000;
			if (zoomMode)
			{
				zooming = true;
				zoomA = point;
				zoomB = point;
				tviewTmin = viewTmin + (viewTmax - viewTmin)/(width)*(point.x-left);
				tviewTmax = tviewTmin;
				tviewVmin = viewVmin + (viewVmax - viewVmin)/height*(point.y-top);
				tviewVmax = tviewVmin;
			} else {
				ic = -1;
				it = -1;
				cursorMoving = false;
				if (cursorFcn & 1)
				{
					if (abs(point.x-Cursor[2].pos)<e) {e = abs(point.x-Cursor[2].pos);ic = 2;}
					if (abs(point.x-Cursor[3].pos)<e) {e = abs(point.x-Cursor[3].pos);ic = 3;}
				}
				if (cursorFcn & 2)
				{
					if (abs(point.y-Cursor[0].pos)<e) {e = abs(point.y-Cursor[0].pos);ic = 0;}
					if (abs(point.y-Cursor[1].pos)<e) {e = abs(point.y-Cursor[1].pos);ic = 1;}
				}
				// if no cursor was selected, then look for a trace
				if ((ic == -1) && (fftMode==false))
				{
					for (int iTrace=0;iTrace<MAX_TRACE;iTrace++)
					{
						if (trace[iTrace].Enable) for (int i=0;i<npoints;i++)
						{
							tp.x = left + i*width/npoints + 13;
							tp.y = top + height/2 - trace[iTrace].data[i]*height/51200 - topMargin - 16;
							if (((point.x-tp.x)*(point.x-tp.x)+(point.y-tp.y)*(point.y-tp.y)) < 10)
							{
								it = iTrace;
								TRACE1("Selected trace index = %i\r\n",it);
								break;
							}
						}
						if (it>=0) break; // trace found
					}
					if (it == -1)
					{
						// check if on grid. Then set -2
						if ((point.x>0) && (point.x<width) && (point.y>0) && (point.y<height + 50)) ic = 4; // background
						
					}
				} else
					cursorMoving = true;
			}
		}
		g_pFramework->InvalidateUICommand(IDM_SELECTCOLOR,UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Color);
		g_pFramework->InvalidateUICommand(IDM_SELECTCOLOR,UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);
		break;
	case WM_MOUSEMOVE:
		{
			mpoint.x = GET_X_LPARAM(lParam) - 28; 
			mpoint.y = GET_Y_LPARAM(lParam) - topMargin - 20; 
			// If cursor movement, then set the active cursor to the current mouse location
			if (zooming)
			{
				zoomB = mpoint;
				myUpdate = true;
				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
			} else {
				if (cursorMoving)
				{
					if (ic<2) Cursor[ic].pos=mpoint.y; else Cursor[ic].pos=mpoint.x;
					getCursorValue();	
					myUpdate = true;
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			CPoint point;
			point.x = GET_X_LPARAM(lParam) - 28; 
			point.y = GET_Y_LPARAM(lParam) - topMargin - 20; 
			if (zooming)
			{ 
				tviewTmin = viewTmin + (double)(viewTmax - viewTmin)/(width)*(min(zoomA.x,zoomB.x)-left - 10);
				tviewTmax = viewTmin + (double)(viewTmax - viewTmin)/(width)*(max(zoomA.x,zoomB.x)-left - 10);
				tviewVmin = viewVmax - (double)(viewVmax - viewVmin)/height*(max(zoomA.y,zoomB.y)-top+topMargin + 20);
				tviewVmax = viewVmax - (double)(viewVmax - viewVmin)/height*(min(zoomA.y,zoomB.y)-top+topMargin + 20);

				TRACE2("Time: [%1.4f, %1.4f]\r\n",tviewTmin,tviewTmax);
				TRACE2("Volt: [%1.4f, %1.4f]\r\n",tviewVmin,tviewVmax);

				viewTmin = tviewTmin;
				viewTmax = tviewTmax;
				viewVmin = tviewVmin;
				viewVmax = tviewVmax;
				zoomA.x=0;zoomA.y=0;
				zoomB.x=0;zoomB.y=0;

				myUpdate = false;
				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				zooming = false;

				// if cursors are off, then remain in zooming mode
				if (cursorFcn) 
				{
					zoomMode = false;
					g_pFramework->InvalidateUICommand(IDM_ZOOM, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
				}
			} else {
				cursorMoving = false;
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			CPoint point;
			//point.x = GET_X_LPARAM(lParam); 
			//point.y = (GET_Y_LPARAM(lParam)-topMargin); 
			zoomMode = false;
			g_pFramework->InvalidateUICommand(IDM_ZOOM, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
			ResetZoom();
		}
		break;
	case WM_MOUSEWHEEL:
		{
			CPoint point;
			//point.x = GET_X_LPARAM(lParam); 
			//point.y = (GET_Y_LPARAM(lParam)-topMargin); 
			//return OnMouseWheel(LOWORD(wParam),HIWORD(wParam),point);
		}
		break;

	case WM_CHAR:
		if (wParam == 'h')
		{
			CImage img;
			// load PNG bitmap file
			if (img.Load(_T("pm33xx.png"))==NULL) 
				hBmp = img.Detach();

			// Display bitmap in Dialog Window
			InvalidateRect(m_hWnd,NULL,TRUE);
			UpdateWindow(m_hWnd);
		}
		if (wParam == 's')
		{
			subSample = !subSample;
			InvalidateRect(m_hWnd,NULL,TRUE);
			UpdateWindow(m_hWnd);
		}
		//return OnChar((int)wParam);
		break;

	case WM_KEYDOWN:
		//return OnKeyDown(wParam,lParam);
		if (wParam == VK_F1)
		{
			::HtmlHelp(m_hWnd,"PM33XX.chm",HH_DISPLAY_TOC,0);
		}
		break;

	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			
			Draw(hdc,2);	
			myUpdate = false;

			EndPaint(hWnd, &ps);
		}

		if (!HaveView)
		{
			//SetTimer(m_hWnd,0,WLAN_UDP_TIMER_DT,WifiProc);  // Receive timer
			//SetTimer(m_hWnd,1,JOYSTICK_TIMER_DT,ControlProc);  // Joystick timer, half rate for WLAN
			//nIDscan = SetTimer(m_hWnd,2,100,ScanProc);  // to trigger a WLAN scan
			//TRACE1("Scan Timer = %li\r\n",nIDscan);
			HaveView = TRUE;
		}
		break;

	case WM_ERASEBKGND:
#ifdef _USEDOUBLEBUFFER_
		if (myUpdate) 
			return FALSE;
		else 
#endif
			return DefWindowProc(hWnd, message, wParam, lParam);
		break;

		/*
	case WM_SETCURSOR:

		if (MyCursor==NULL) 
			return false;
		else {
			SetCursor(LoadCursor(m_hWnd,MyCursor));
			return true;
		}
		break;
		*/

	case WM_SIZE:
		SetWindowPos(m_hStatusWnd,HWND_TOP,0,0,0,0,SWP_SHOWWINDOW);	
		InvalidateRect(m_hWnd,NULL,TRUE);
		UpdateWindow(m_hWnd);
		UpdateStatus();
		break;

	//case WM_SIZING:
		//RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,SBID[4]);
		//pPlotBox->SetWindowPos(&wndTop,12,97,pRect->right-pRect->left-33,pRect->bottom-pRect->top-165,0);
		//pPlotBox->SetWindowPos(&wndTop,12,97,pRect->right-pRect->left-41,pRect->bottom-pRect->top-169,0);
	//	InvalidateRect(m_hWnd,NULL,TRUE);
	//	UpdateWindow(m_hWnd);
	//	break;

	case WM_DESTROY:
		if (hCommPort != INVALID_HANDLE_VALUE) 
		{
			PurgeComm(hCommPort, PURGE_TXABORT | PURGE_RXABORT);
			CloseHandle(hCommPort);
		}	
		/*
		for (int iTrace=0;iTrace<MAX_TRACE;iTrace++)
		{
			if (trace[iTrace].data)
				delete(trace[iTrace].data);
		}
		*/
		DestroyWindow(m_hStatusWnd);
		DestroyRibbon();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char txt[250];
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		sprintf_s(txt,250,"PM33XX Fluke CombiScope Interface Version 2.0.%i",__MY_BUILD);
		SetDlgItemText(hDlg,IDC_ABOUTTXT1, txt);
		sprintf_s(txt,250,"Built on %02i/%02i/%4i at %02i:%02i:%02i",
			MONTH, DAY, YEAR, COMPILE_HOUR, COMPILE_MINUTE, COMPILE_SECOND);
		SetDlgItemText(hDlg,IDC_ABOUTTXT2, txt);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

bool GetReg(char *name, char *key, char *buffer, int *buflen, int regtype)
{
	int szLen;
	bool nSuccess = true;
	char thisKeyName[200];
	strcpy_s(thisKeyName,200,szKeyName);
	if (strlen(name)>0)
	{
		strcat_s(thisKeyName,200,"\\");
		strcat_s(thisKeyName,200,name);
	}
	lRet = ::RegCreateKeyEx(HKEY_CURRENT_USER, thisKeyName, 0L, NULL, 0L, KEY_READ, NULL, &hKey, NULL);
	szLen = sizeof(int);
	if (buflen==NULL) buflen = &szLen;
	if (lRet == ERROR_SUCCESS)
	{
		lRet = ::RegQueryValueEx(hKey, key, NULL, (LPDWORD)&regtype, (BYTE*)buffer, (DWORD*)buflen);
		if (lRet != ERROR_SUCCESS)
		{
			// code in Winerror.h
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,lRet,NULL,msgA,200,NULL);
			ATLTRACE2("Error: %s\r\n",msgA);
			nSuccess = false;
		}
		RegCloseKey(hKey);
	}
	else
		nSuccess = false;
	return nSuccess;
}

bool SetReg(char *name, char *key, char *val, int buflen, int regtype)
{
	bool nSuccess = true;
	char thisKeyName[200];
	strcpy_s(thisKeyName,200,szKeyName);
	if (strlen(name)>0)
	{
		strcat_s(thisKeyName,200,"\\");
		strcat_s(thisKeyName,200,name);
	}
	lRet = ::RegCreateKeyEx(HKEY_CURRENT_USER, thisKeyName, 0L, NULL, 0L, KEY_WRITE, NULL, &hKey, NULL);
	if(lRet == ERROR_SUCCESS)
	{
		lRet = ::RegSetValueEx(hKey, key, NULL, regtype, ((BYTE*)val), buflen);
		if (lRet != ERROR_SUCCESS)
		{
			// code in Winerror.h
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,lRet,NULL,msgA,200,NULL);
			ATLTRACE2("Error: %s\r\n",msgA);
			nSuccess = false;
		}
		RegCloseKey(hKey);
	}
	else
		nSuccess = false;
	return nSuccess;
}

void RegisterMru(char *fn)
{
	int imru;
	// if file name exist in mru, then bring to the top
	for (imru=0;imru<9;imru++)
		if (strcmp(mru[imru],fn)==0) break; 
	for (int i=imru;i>0;i--)
		strcpy_s(mru[i],MAXFILENAME,mru[i-1]);
	strcpy_s(mru[0],MAXFILENAME,fn);
}

void SetTraceData(int n)
{
	int it, itt;

	switch(n)
	{
	case 512:
		for (it=0;it<4;it++) trace[it].fdata = &fdata[512*it];
		for (it=4;it<MAX_TRACE;it++) trace[it].data = &mdata[512*(it-4)];
		break;
	case 8192:
		for (it=0;it<4;it++) trace[it].fdata = &fdata[8192*it];
		for (it=4;it<MAX_TRACE;it+=8) 
			for (int itt=0;itt<8;itt++) trace[it+itt].data = &mdata[itt*8192];
		break;
	case 16384:
		for (it=0;it<4;it+=2) trace[it].fdata = fdata;
		for (it=1;it<4;it+=2) trace[it].fdata = &fdata[16384];
		for (it=4;it<MAX_TRACE;it+=4) 
			for (itt=0;itt<4;itt++) trace[it+itt].data = &mdata[itt*16384];
		break;
	case 32768:
		for (it=0;it<4;it++) trace[it].fdata = fdata;
		for (it=4;it<MAX_TRACE;it+=2) trace[it].data = mdata;
		for (it=5;it<MAX_TRACE;it+=2) trace[it].data = &mdata[32768];
		break;
	}

	switch(n)
	{
	case 512:
		for (it=0;it<4;it++) trace[it].data = &cdata[512*it];
		for (it=4;it<MAX_TRACE;it++) trace[it].data = &mdata[512*(it-4)];
		break;
	case 8192:
		for (it=0;it<4;it++) trace[it].data = &cdata[8192*it];
		for (it=4;it<MAX_TRACE;it+=8) 
			for (int itt=0;itt<8;itt++) trace[it+itt].data = &mdata[itt*8192];
		break;
	case 16384:
		for (it=0;it<4;it+=2) trace[it].data = cdata;
		for (it=1;it<4;it+=2) trace[it].data = &cdata[16384];
		for (it=4;it<MAX_TRACE;it+=4) 
			for (itt=0;itt<4;itt++) trace[it+itt].data = &mdata[itt*16384];
		break;
	case 32768:
		for (it=0;it<4;it++) trace[it].data = cdata;
		for (it=4;it<MAX_TRACE;it+=2) trace[it].data = mdata;
		for (it=5;it<MAX_TRACE;it+=2) trace[it].data = &mdata[32768];
		break;
	}
	
}

void Init()
{
	myUpdate = false;
	char tcr[50];

	// setup default colors for traces
	for (int i=0;i<36;i++) 
	{
		sprintf_s(tcr,50,"TC%i",i+1);
		if (GetReg("",tcr,(char*)&trace[i].color,NULL,REG_DWORD) == FALSE)
		{
			if (i<4) 
				trace[i].color = RGB(255,0,0);
			else
				trace[i].color = RGB(0,200,0);
		}
	}
	for (int i=0;i<4;i++)
	{
		sprintf_s(tcr,50,"TC%i",i+37);
		if (GetReg("",tcr,(char*)&Cursor[i].color,NULL,REG_DWORD) == FALSE)
			Cursor[i].color = RGB(255,0,0);
		Cursor[i].width=1;
	}

	if (GetReg("","GC",(char*)&gridCol,NULL,REG_DWORD) == FALSE)
		gridCol = RGB(100,100,100);
	if (GetReg("","LC",(char*)&lblCol,NULL,REG_DWORD) == FALSE)
		lblCol = RGB(0,0,128);
	if (GetReg("","BC",(char*)&bgCol,NULL,REG_DWORD) == FALSE)
		bgCol = RGB(255,255,255);
	topMargin = 146;

	for (int i=0;i<MAX_TRACE;i++) 
	{
		trace[i].width = 2;
		trace[i].ntrace=0;
		if (i<4)
			trace[i].Enable=true;
		else
			trace[i].Enable=false;
	}

	hCommPort = INVALID_HANDLE_VALUE;

	npoints = 0;
	xres = 0.0F;
	timeout = 0;
	data = "";
	hBmp = NULL;
	dt = 0.1;
	deltay = 0.25;
	y00 = 0.0;
	strcpy_s(tunit,20,"ms");
	strcpy_s(tlabel,20,"Time [");
	strcat_s(tlabel,20,tunit);
	strcat_s(tlabel,20,"]");
	Cursor[0].pos = 200;
	Cursor[1].pos = 210;
	Cursor[2].pos = 100;
	Cursor[3].pos = 110;
	ic = -1;
	it = -1;
	xgrid = 10;
	ygrid = 8;
	zooming = false;
	zoomMode = false;
	fftMode = false;
	subSample = true;
	xLog = false;
	yLog = false;
	zoomA.x=0;zoomA.y=0;
	zoomB.x=0;zoomB.y=0;
	viewTmin = 0.0;
	viewTmax = 1.0;
	viewVmin = ygrid/-2.0*deltay;
	viewVmax = ygrid/2.0*deltay;
	wndFcn = 0;
	// if not exist ver.inf
	// system("ver > ver.inf");
	// load ver

	lfn.lStructSize       = sizeof(lfn);
	lfn.hwndOwner         = NULL;
	lfn.hInstance         = NULL;
	lfn.lpstrCustomFilter = NULL;
	lfn.nMaxCustFilter    = 0;
	lfn.nFileOffset       = 0;
	lfn.nFileExtension    = 0;
	lfn.lpstrDefExt       = NULL;
	lfn.lCustData         = NULL;
	lfn.lpfnHook 		  = NULL;
	lfn.lpTemplateName    = NULL;
	lfn.Flags             = OFN_SHOWHELP | OFN_EXPLORER;
	lfn.lpstrFile         = (LPSTR)DefFullName;
	lfn.nMaxFile          = MAX_FILENAME;
	lfn.lpstrFileTitle    = (LPSTR)DefName;
	lfn.nMaxFileTitle     = MAX_FILENAME;

	showInfo = false;
	cursorMoving = false;
	strcpy_s(DefFullName,MAX_FILENAME,"");
	strcpy_s(DefName,MAX_FILENAME,"");
}

void UpdateStatus()
{
	char sstr[100];
	if (m_hStatusWnd)
	{
		RECT sr;
		HDC hsDC = GetDC(m_hStatusWnd);
		HFONT hfnt = CreateFont(18,0,0,0,FW_DONTCARE,0,0,0,0,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,18,"Calibri");
		SelectObject(hsDC,hfnt);
		if (hCommPort == INVALID_HANDLE_VALUE) strcpy_s(sstr,100,"Not connected"); else strcpy_s(sstr,100,"Connected"); 
		sr.left=0;sr.right=150;sr.top=2;sr.bottom=21;
		DrawStatusText(hsDC,&sr,sstr,0);
		sr.left=152;sr.right=250;
		DrawStatusText(hsDC,&sr,CommPort,0);
		sr.left=252;sr.right=350;
		DrawStatusText(hsDC,&sr,BaudRates[SelectedBaudrate],0);
		sr.left=352;sr.right=850;
		DrawStatusText(hsDC,&sr,pmid,0);
		sr.left=852;sr.right=1200;
		DrawStatusText(hsDC,&sr,lfn.lpstrFile,0);
		DeleteObject(hfnt);
	}	
}


void Draw(HDC dc0, int CID)
{
	HBITMAP dcbmp;
	HDC dcmem, dc;
	HPEN art;
	HBRUSH fil;
	HFONT fnt;
	BITMAP bm;
	int dx, dy, infoy = 0, fnth, yline;
	RECT rc,cwnd;
	POINT tp[35000];
	char valstr[100];


	if (CID==0)   // from print dialog
	{
		//GetClientRect(&cwnd);
		GetClipBox(dc0,&cwnd);

		// if the page is portrait, then cut the height in half
		if (cwnd.right<cwnd.bottom) cwnd.bottom /= 2;

		cwnd.left+=200;
		cwnd.top+=200;
		cwnd.right -= 200;
		fnth = 48;
		infowidth = 800;
		yline = 80;
	} 
	else if (CID==1)
	{
		GetClipBox(dc0,&cwnd);

		cwnd.left+=2;
		cwnd.top+=10;
		fnth = 12;
		infowidth = 200;
		yline = 15;
	} else {
		GetClientRect(m_hWnd,&cwnd);// control window

		cwnd.top += topMargin+20;
		cwnd.bottom -= 30;
		cwnd.left += 30;
		cwnd.right -= 20;
		
		fnth = 12;
		infowidth = 200;
		yline = 15;
	} 


	if (hBmp)
	{
		dcmem = CreateCompatibleDC( dc0 );
		SelectObject (dcmem, hBmp);
		GetObject ( hBmp, sizeof(bm), &bm );
		dx = bm.bmWidth;
		dy = bm.bmHeight;
		dcbmp = CreateCompatibleBitmap(dc0,dx,dy);
		SelectObject(dc0,dcbmp);
		double sx = (double)(cwnd.right - cwnd.left-20) / dx;
		double sy = (double)(cwnd.bottom - cwnd.top-40) / dy;
		if (sx<sy) sy = sx; else sx = sy;
		StretchBlt(dc0,cwnd.left + (cwnd.right - cwnd.left-20 - (int)(sx*dx))/2,
			cwnd.top + (cwnd.bottom - cwnd.top-40 - (int)(sy*dy))/2,
			(int)(sx*dx),
			(int)(sy*dy),
			dcmem,0,0,dx,dy,SRCCOPY);
		//BitBlt(dc0,0,0,dx,dy,dcmem,0,0,SRCCOPY);
		DeleteDC(dcmem);
		DeleteObject(dcbmp);

	} else {
#if _USEDOUBLEBUFFER_
		dc = CreateCompatibleDC(dc0);
		dx = cwnd.right - cwnd.left;
		dy = cwnd.bottom - cwnd.top;
		dcbmp = CreateCompatibleBitmap(dc0,dx,dy);
		SelectObject(dc,dcbmp);
		
		left = 40;
		top = 5;
		width = dx - left-20;
		height = dy - 50;
#else
		dc = dc0;
		left = cwnd.left+25;
		top = cwnd.top;
		width = cwnd.right - cwnd.left-45;
		height = cwnd.bottom - cwnd.top-60;
#endif
		if (showInfo)
		{	
			width -= infowidth;
			infoy = top;
		}

#if 0
		rc.right=left+width;
		rc.left=left;
		rc.top=top;
		rc.bottom=top+height;
#else
		rc.right = dx;
		rc.left = 0;
		rc.top = 0;
		rc.bottom = dy;
#endif

		if (!cursorMoving) getCursorPos();

		fil = CreateSolidBrush(bgCol);
		SelectObject(dc,fil);
		FillRect(dc,&rc,fil);
		DeleteObject(fil);

		SetBkMode(dc,TRANSPARENT);

		// Grid lines
		float _xs = 1.0;  // scaling, based on scope unit string
		if (strcmp(tunit,"ms")==0) _xs = 1.0e3F;
		if (strcmp(tunit,"us")==0) _xs = 1.0e6F;
		if (strcmp(tunit,"ns")==0) _xs = 1.0e9F;

		int efmax = (int)ceilf(log10f((float)1.0F/xres/2.0F));
		int efmin = (int)ceilf(log10f((float)1.0F/xres/2.0F/npoints));

		//TRACE2("LOG X-scale = [%i,%i]\r\n",efmin,efmax);
		float _yminlog = 20.0F*log10f(powf(10.0,-4.0F));
		float _ymaxlog = 20.0F*log10f(powf(10.0,1.0F));
		float _xminlog = 20.0F*log10f(powf(10.0,(float)efmin));
		float _xmaxlog = 20.0F*log10f(powf(10.0,(float)efmax));
		int gridlinewidth = 1;
		if (fftMode)
		{
			art = CreatePen(PS_SOLID,2,gridCol);
			SelectObject(dc,art);
			tp[0].x = left; tp[0].y = top;
			tp[1].x = left+width; tp[1].y = top;
			tp[2].x = left+width; tp[2].y = top+height;
			tp[3].x = left; tp[3].y = top+height;
			tp[4].x = left; tp[4].y = top;
			Polyline(dc,tp,5);
			DeleteObject(art);
			if (xLog)
			{
				art = CreatePen(PS_DASH,1,gridCol);
				SelectObject(dc,art);tp[0].y = top;tp[1].y = top + height;
				for (int i=efmin;i<efmax;i++)
				{
					for (int k=1;k<10;k++)
					{
						float _x = powf(10.0F,(float)i)*k;
						//TRACE1("Xgrid: %1.2f\r\n",_x); 
						tp[0].x = left + (LONG)((20.0F*log10f(_x)-_xminlog)*width/(_xmaxlog-_xminlog));
						tp[1].x = tp[0].x; 
						Polyline(dc,tp,2);
					}
				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID,1,gridCol);
				SelectObject(dc,art);
				for (int i=efmin;i<efmax;i++)
				{
					for (int k=10;k<100;k+=5)
					{
						float _x = powf(10.0F,(float)i)*0.1F*k;
						tp[0].x = left + (LONG)((20.0F*log10f(_x)-_xminlog)*width/(_xmaxlog-_xminlog));
						tp[1].x = tp[0].x;
						tp[0].y = top;
						tp[1].y = top + 5;
						Polyline(dc,tp,2);
						tp[0].y = top + height - 5;
						tp[1].y = top + height;
						Polyline(dc,tp,2);
					}
				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
					SetTextColor(dc,lblCol);
					SelectObject(dc,fnt);
					for (int i=efmin;i<=efmax;i++)
					{
						float _x = powf(10.0F,(float)i);
						rc.left = left+(LONG)((20.0F*log10f(_x)-_xminlog)*width/(_xmaxlog-_xminlog)); 
						rc.right=rc.left;
						rc.top = top+height+5;rc.bottom=rc.top;
						switch(i)
						{ 
						case 0: strcpy_s(valstr,100,"1 Hz");break;
						case 1: strcpy_s(valstr,100,"10 Hz");break;
						case 2: strcpy_s(valstr,100,"100 Hz");break;
						case 3: strcpy_s(valstr,100,"1 kHz");break;
						case 4: strcpy_s(valstr,100,"10 kHz");break;
						case 5: strcpy_s(valstr,100,"100 kHz");break;
						case 6: strcpy_s(valstr,100,"1 MHz");break;
						case 7: strcpy_s(valstr,100,"10 MHz");break;
						case 8: strcpy_s(valstr,100,"100 MHz");break;
						case 9: strcpy_s(valstr,100,"1 GHz");break;
						}
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_CENTER);
					}
					// draw time label
					rc.left = left+width/2; rc.right=rc.left;
					rc.top = top+height+15;rc.bottom=rc.top;
					DrawText(dc,"Frequency",-1,&rc,DT_NOCLIP | DT_CENTER);
					DeleteObject(fnt);				
				}
			}
			else
			{
				art = CreatePen(PS_DASH,1,gridCol);
				SelectObject(dc,art);
				for (int i=1;i<xgrid;i++)
				{
					tp[0].x = left+i*width/xgrid; tp[0].y = top;
					tp[1].x = left+i*width/xgrid; tp[1].y = top + height;
					Polyline(dc,tp,2);
				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID,1,gridCol);
				SelectObject(dc,art);
				for (int i=1;i<40;i++)
				{
					tp[0].x = left+i*width/40; tp[0].y = top;
					tp[1].x = left+i*width/40; tp[1].y = top + 5;
					Polyline(dc,tp,2);
					tp[0].x = left+i*width/40; tp[0].y = top + height - 5;
					tp[1].x = left+i*width/40; tp[1].y = top + height;
					Polyline(dc,tp,2);
				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
					SetTextColor(dc,lblCol);
					SelectObject(dc,fnt);
					t = viewTmin/xres/2.0F/(xgrid*dt)/_xs;
					y = viewVmin; 
					for (int i=0;i<=xgrid;i++)
					{
						rc.left = left+i*width/xgrid; rc.right=rc.left;
						rc.top = top+height+5;rc.bottom=rc.top;
						sprintf_s(valstr,100,"%1.2f",t);
						t += ((viewTmax-viewTmin)/xres/2.0F/xgrid/(xgrid*dt))/_xs; 
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_CENTER);
					}
					// draw time label
					rc.left = left+width/2; rc.right=rc.left;
					rc.top = top+height+15;rc.bottom=rc.top;
					char fLabel[30];
					strcpy_s(fLabel,30,"Freq [");
					if (strcmp(tunit,"ms")==0) strcat_s(fLabel,30,"kHz]");
					if (strcmp(tunit,"us")==0) strcat_s(fLabel,30,"MHz]");
					if (strcmp(tunit,"ns")==0) strcat_s(fLabel,30,"GHz]");
					DrawText(dc,fLabel,(int)strlen(fLabel),&rc,DT_NOCLIP | DT_CENTER);
					DeleteObject(fnt);				
				}
			}
			if (yLog)
			{
				art = CreatePen(PS_DASH,1,gridCol);
				SelectObject(dc,art);
				tp[0].x = left; tp[1].x = left+width;
				for (int i=0;i<5;i++)
				{
					for (int k=1;k<10;k+=2)
					{
						float _y = powf(10.0F,(float)(i-4))*k;
						tp[0].y = top + height - (LONG)((20.0F*log10f(_y)-_yminlog)*height/(_ymaxlog-_yminlog));
						tp[1].y = tp[0].y;
						Polyline(dc,tp,2);
					}

				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID,1,gridCol);
				SelectObject(dc,art);  
				for (int i=0;i<5;i++)
				{
					for (int k=10;k<100;k+=5)
					{
						float _y = powf(10.0F,(float)(i-4))*0.1F*k;
						tp[0].x = left+width; tp[1].x = left+width-5; 
						tp[0].y = top + height - (LONG)((20.0F*log10f(_y)-_yminlog)*height/(_ymaxlog-_yminlog));
						tp[1].y = tp[0].y;
						Polyline(dc,tp,2);
						tp[0].x = left;   
						tp[1].x = left+5; 
						Polyline(dc,tp,2);
					}

				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
					SetTextColor(dc,lblCol);
					SelectObject(dc,fnt);
					t = 0;
					y = 0; 
					for (int i=1;i<=5;i++)
					{
						float _y = powf(10.0F,(float)(i-4));
						rc.left = left-5; rc.right=rc.left;
						rc.top = top + height - 5-(LONG)((20.0F*log10f(_y)-_yminlog)*height/(_ymaxlog-_yminlog));
						rc.bottom=rc.top;
						sprintf_s(valstr,100,"%1.1g",_y);
						y += (viewVmax-viewVmin)/ygrid;
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_RIGHT);
					}
				}
			}
			else
			{
				art = CreatePen(PS_DASH,1,gridCol);
				SelectObject(dc,art);
				for (int i=1;i<ygrid;i++)
				{
					tp[0].x = left; tp[0].y = top + i*height/ygrid;
					tp[1].x = left+width; tp[1].y = top + i*height/ygrid;
					Polyline(dc,tp,2);
				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID,1,gridCol);
				SelectObject(dc,art);
				for (int i=1;i<40;i++)
				{
					tp[0].x = left+width;   tp[0].y = top + i*height/40;
					tp[1].x = left+width-5; tp[1].y = top + i*height/40;
					Polyline(dc,tp,2);
					tp[0].x = left;   tp[0].y = top + i*height/40;
					tp[1].x = left+5; tp[1].y = top + i*height/40;
					Polyline(dc,tp,2);
				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
					SetTextColor(dc,lblCol);
					SelectObject(dc,fnt);
					t = 0;
					y = 0; 
					for (int i=0;i<=ygrid;i++)
					{
						rc.left = left-5; rc.right=rc.left;
						rc.top = top+(ygrid-i)*height/ygrid-5;rc.bottom=rc.top;
						sprintf_s(valstr,100,"%1.2f",y);
						y += (viewVmax-viewVmin)/ygrid;
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_RIGHT);
					}
				}
			}
		}
		else
		{
			// if not in FFT mode
			art = CreatePen(PS_SOLID,2,gridCol);
			SelectObject(dc,art);
			tp[0].x = left; tp[0].y = top;
			tp[1].x = left+width; tp[1].y = top;
			tp[2].x = left+width; tp[2].y = top+height;
			tp[3].x = left; tp[3].y = top+height;
			tp[4].x = left; tp[4].y = top;
			Polyline(dc,tp,5);
			tp[0].x = left; tp[0].y = top + height/2;
			tp[1].x = left+width; tp[1].y = top + height/2;
			Polyline(dc,tp,2);
			tp[0].x = left+width/2; tp[0].y = top;
			tp[1].x = left+width/2; tp[1].y = top + height;
			Polyline(dc,tp,2);
			DeleteObject(art);
			art = CreatePen(PS_DASH,1,gridCol);
			SelectObject(dc,art);
			for (int i=1;i<ygrid;i++)
			{
				tp[0].x = left; tp[0].y = top + i*height/ygrid;
				tp[1].x = left+width; tp[1].y = top + i*height/ygrid;
				Polyline(dc,tp,2);
			}
			for (int i=1;i<xgrid;i++)
			{
				tp[0].x = left+i*width/xgrid; tp[0].y = top;
				tp[1].x = left+i*width/xgrid; tp[1].y = top + height;
				Polyline(dc,tp,2);
			}
			DeleteObject(art);
			art = CreatePen(PS_SOLID,1,gridCol);
			SelectObject(dc,art);
			tp[0].x = left; tp[0].y = top + height/4;
			tp[1].x = left+width; tp[1].y = top + height/4;
			Polyline(dc,tp,2);
			tp[0].x = left; tp[0].y = top + height*3/4;
			tp[1].x = left+width; tp[1].y = top + height*3/4;
			Polyline(dc,tp,2);
			for (int i=1;i<40;i++)
			{
				tp[0].x = left+i*width/40; tp[0].y = top + height/2-5;
				tp[1].x = left+i*width/40; tp[1].y = top + height/2+5;
				Polyline(dc,tp,2);
				tp[0].x = left+width/2-5; tp[0].y = top + i*height/40;
				tp[1].x = left+width/2+5; tp[1].y = top + i*height/40;
				Polyline(dc,tp,2);

				tp[0].x = left+i*width/40; tp[0].y = top + height/4-2;
				tp[1].x = left+i*width/40; tp[1].y = top + height/4+2;
				Polyline(dc,tp,2);
				tp[0].x = left+i*width/40; tp[0].y = top + height*3/4-2;
				tp[1].x = left+i*width/40; tp[1].y = top + height*3/4+2;
				Polyline(dc,tp,2);
			}
			DeleteObject(art);
			art = CreatePen(PS_DOT,1,gridCol);
			SelectObject(dc,art);
			tp[0].x = left; tp[0].y = top + height*3/16;
			tp[1].x = left+width; tp[1].y = top + height*3/16;
			Polyline(dc,tp,2);
			tp[0].x = left; tp[0].y = top + height*13/16;
			tp[1].x = left+width; tp[1].y = top + height*13/16;
			Polyline(dc,tp,2);
			DeleteObject(art);

			//if (myUpdate==false)
			{
				fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
				SetTextColor(dc,lblCol);
				SelectObject(dc,fnt);
				t = viewTmin;
				y = viewVmin; // + y00;
				for (int i=0;i<=xgrid;i++)
				{
					rc.left = left+i*width/xgrid; rc.right=rc.left;
					rc.top = top+height+5;rc.bottom=rc.top;
					sprintf_s(valstr,100,"%1.3f",t);   // resolution could be made dependent on screen width
					t += (viewTmax-viewTmin)/xgrid; 
					DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_CENTER);
				}
				for (int i=0;i<=ygrid;i++)
				{
					rc.left = left-5; rc.right=rc.left;
					rc.top = top+(ygrid-i)*height/ygrid-5;rc.bottom=rc.top;
					if (viewVmax-viewVmin<0.001)
						sprintf_s(valstr,100,"%1.4f",y);
					else
						sprintf_s(valstr,100,"%1.3f",y);
					y += (viewVmax-viewVmin)/ygrid;
					DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_RIGHT);
				}
				// draw time label
				rc.left = left+width/2; rc.right=rc.left;
				rc.top = top+height+15;rc.bottom=rc.top;
				DrawText(dc,CString(tlabel),(int)strlen(tlabel),&rc,DT_NOCLIP | DT_CENTER);
				DeleteObject(fnt);
			}
		}
		IntersectClipRect(dc,left,top,left+width,top+height);
		for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
		{
			if ((trace[iTrace].ntrace>0) && (trace[iTrace].Enable))
			{
				trace[iTrace].Vmax = -1e6;;
				trace[iTrace].Vmin = 1e6; 
				
				int smin = 0;
				if (viewTmin>0.0) smin = (int)(viewTmin/dt*npoints/xgrid);
				int smax = npoints;
				if (viewTmax<xgrid*dt) smax = (int)(viewTmax/dt*npoints/xgrid);
				//TRACE2("Plot from %i to %i \r\n",smin,smax);
				int y000 = (int)((viewVmax+viewVmin)*height/ygrid/deltay/2.0);
				double yscale = deltay/(viewVmax-viewVmin)*ygrid;
				int ii = 0, ss = smax-smin, tpho2 = top + height/2;  // top plus height over 2
				int di = 1;
				if (subSample) {di = 1+ss/width;}
				for (int i=smin;i<smax;i+=di)
				{
					if ((fftMode) && (xLog))
					{
						float _x = (float)i/npoints/xres/2.0F;
						tp[ii].x = left + (LONG)((20.0F*log10f(_x)-_xminlog)*width/(_xmaxlog-_xminlog));
					} else
					{
						tp[ii].x = left + (i-smin)*width/ss;
					}

					if (fftMode) 
					{
						if (yLog)
						{
							float _y = (float)trace[iTrace].fdata[i]/51200.0F;
							tp[ii].y = top + height - (LONG)((20.0F*log10f(_y)-_yminlog)*height/(_ymaxlog-_yminlog));
						}
						else
						{
							tp[ii].y = tpho2 - (LONG)((trace[iTrace].fdata[i]*height/51200 - y000)*yscale);
							if (tp[ii].y>trace[iTrace].Vmax) trace[iTrace].Vmax=(double)tp[ii].y;
							if (tp[ii].y<trace[iTrace].Vmin) trace[iTrace].Vmin=(double)tp[ii].y;
						}
					}
					else
					{
						tp[ii].y = tpho2 - (LONG)((trace[iTrace].data[i]*height/51200 - y000)*yscale);
						if (tp[ii].y>trace[iTrace].Vmax) trace[iTrace].Vmax=(double)tp[ii].y;
						if (tp[ii].y<trace[iTrace].Vmin) trace[iTrace].Vmin=(double)tp[ii].y;
					}
					ii++;
				}

				art = CreatePen(PS_SOLID,trace[iTrace].width,trace[iTrace].color);
				SelectObject(dc,art);
				Polyline(dc,tp,ss/di);
				
				// process min, max and peak-peak voltages
				trace[iTrace].Vmax = (trace[iTrace].Vmax - top)/height*ygrid*deltay-trace[iTrace].y_zero-ygrid/2*deltay;
				trace[iTrace].Vmin = (trace[iTrace].Vmin - top)/height*ygrid*deltay-trace[iTrace].y_zero-ygrid/2*deltay;
				trace[iTrace].Vpp = trace[iTrace].Vmax - trace[iTrace].Vmin;

				// add label to trace
				fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
				SetTextColor(dc,trace[iTrace].color);
				SelectObject(dc,fnt);
				rc.left = left + 20; rc.right=rc.left;
				rc.top = tp[0].y - 5;rc.bottom=rc.top;
				DrawText(dc,trace[iTrace].trace_id,(int)strlen(trace[iTrace].trace_id),&rc,DT_NOCLIP | DT_LEFT);
				DeleteObject(fnt);
				DeleteObject(art);
			}
		}
		SelectClipRgn(dc,NULL);

		for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
		{
			if ((trace[iTrace].ntrace>0) )
			{
				art = CreatePen(PS_SOLID,2,RGB(0,0,0));
				SelectObject(dc,art);
				if (showInfo)
				{

					rc.left = left+width+15; rc.right=rc.left+(LONG)(infowidth);
					rc.top = infoy;rc.bottom=rc.top+93*yline/15;
					fil = CreateSolidBrush(RGB(255,255,245));
					SelectObject(dc,fil);
					//dc->FillRect(&rc,&fil);
					Rectangle(dc,rc.left,rc.top,rc.right,rc.bottom);
					DeleteObject(fil);

					fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
					SetTextColor(dc,trace[iTrace].color);
					SelectObject(dc,fnt);
					infoy += 3;
		
					rc.left = left+width+18; rc.right=rc.left;
					rc.top = infoy;rc.bottom=rc.top;
					if (iTrace<4)
					{
						sprintf_s(valstr,100,"Trace info [%s]:",trace[iTrace].trace_id);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						//sprintf_s(valstr,100,"More information to be displayed here.");
						sprintf_s(valstr,100,"Date: %s",trace[iTrace].tdate);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"Delta_t=%1.1f %1.1f s",trace[iTrace].delta_t,trace[iTrace].x_res);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"Y0=%1.1f %s %1.1f s",trace[iTrace].y_zero,trace[iTrace].y_unit);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"Acq: %i samples",npoints);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						
						sprintf_s(valstr,100,"Vmax=%1.2f Vmin=%1.2f  Vpp=%1.2f",trace[iTrace].Vmax,trace[iTrace].Vmin,trace[iTrace].Vpp);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += (int)(yline*1.4);
					}
					else 
					{
						int traceGroup = iTrace / 4;
						int traceRegister = iTrace - 4 * traceGroup + 1;
						sprintf_s(valstr,100,"Memory info [%i.%i]:",traceGroup,traceRegister);
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"More information to be displayed here.");
						DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += (int)(yline*1.4);
					}
					DeleteObject(fnt);
				}
				DeleteObject(art);
			}
		}
#define CURSOR_TSIZE 10
		if (cursorFcn & 1)
		{
			// TODO: if CID==0  then re-scale to match the time and V
			art = CreatePen(PS_DASHDOT,Cursor[2].width,Cursor[2].color);
			SelectObject(dc,art);
			tp[0].x = Cursor[2].pos; tp[0].y = top;
			tp[1].x = Cursor[2].pos; tp[1].y = top+height;
			Polyline(dc,tp,2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID,1,Cursor[2].color);
			SelectObject(dc,art);
			fil = CreateSolidBrush(Cursor[2].color);
			SelectObject(dc,fil);
			tp[0].y = top + CURSOR_TSIZE;
			tp[1].x = Cursor[2].pos - CURSOR_TSIZE; tp[1].y = top;
			tp[2].x = Cursor[2].pos + CURSOR_TSIZE; tp[2].y = top;
			Polygon(dc,tp,3);
			tp[0].y = top + height - CURSOR_TSIZE;
			tp[1].y = top + height;
			tp[2].y = top + height;
			Polygon(dc,tp,3);
			DeleteObject(fil);
			DeleteObject(art);

			art = CreatePen(PS_DOT,Cursor[3].width,Cursor[3].color);
			SelectObject(dc,art);
			tp[0].x = Cursor[3].pos; tp[0].y = top;
			tp[1].x = Cursor[3].pos; tp[1].y = top+height;
			Polyline(dc,tp,2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID,1,Cursor[3].color);
			SelectObject(dc,art);
			fil = CreateSolidBrush(Cursor[3].color);
			SelectObject(dc,fil);
			tp[0].y = top + CURSOR_TSIZE;
			tp[1].x = Cursor[3].pos - CURSOR_TSIZE; tp[1].y = top;
			tp[2].x = Cursor[3].pos + CURSOR_TSIZE; tp[2].y = top;
			Polygon(dc,tp,3);
			tp[0].y = top + height - CURSOR_TSIZE;
			tp[1].y = top + height;
			tp[2].y = top + height;
			Polygon(dc,tp,3);
			DeleteObject(art);
			DeleteObject(fil);
		}
		if (cursorFcn & 2)
		{
			art = CreatePen(PS_DASHDOT,Cursor[0].width,Cursor[0].color);
			SelectObject(dc,art);
			tp[0].x = left; tp[0].y = Cursor[0].pos;
			tp[1].x = left+width; tp[1].y = Cursor[0].pos;
			Polyline(dc,tp,2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID,1,Cursor[0].color);
			SelectObject(dc,art);
			fil = CreateSolidBrush(Cursor[0].color);
			SelectObject(dc,fil);
			tp[0].x = left + CURSOR_TSIZE;
			tp[1].x = left; tp[1].y = Cursor[0].pos - CURSOR_TSIZE;
			tp[2].x = left; tp[2].y = Cursor[0].pos + CURSOR_TSIZE;
			Polygon(dc,tp,3);
			tp[0].x = left + width - CURSOR_TSIZE;
			tp[1].x = left + width; 
			tp[2].x = left + width; 
			Polygon(dc,tp,3);
			DeleteObject(fil);
			DeleteObject(art);

			art = CreatePen(PS_DOT,Cursor[1].width,Cursor[1].color);
			SelectObject(dc,art);
			tp[0].x = left; tp[0].y = Cursor[1].pos;
			tp[1].x = left+width; tp[1].y = Cursor[1].pos;
			Polyline(dc,tp,2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID,1,Cursor[1].color);
			SelectObject(dc,art);
			fil = CreateSolidBrush(Cursor[1].color);
			SelectObject(dc,fil);
			tp[0].x = left + CURSOR_TSIZE;
			tp[1].x = left; tp[1].y = Cursor[1].pos - CURSOR_TSIZE;
			tp[2].x = left; tp[2].y = Cursor[1].pos + CURSOR_TSIZE;
			Polygon(dc,tp,3);
			tp[0].x = left + width - CURSOR_TSIZE;
			tp[1].x = left + width; 
			tp[2].x = left + width; 
			Polygon(dc,tp,3);
			DeleteObject(fil);
			DeleteObject(art);
		}

		if (cursorFcn>0)
		{
			if (showInfo)
			{
				art = CreatePen(PS_SOLID,1,RGB(0,0,0));
				SelectObject(dc,art);

				rc.left = left+width+15; rc.right=rc.left+(LONG)(infowidth);
				rc.top = infoy;rc.bottom=rc.top+80*yline/15;
				fil = CreateSolidBrush(RGB(255,255,255));
				SelectObject(dc,fil);

				Rectangle(dc,rc.left,rc.top,rc.right,rc.bottom);
				DeleteObject(fil);

				fnt = CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
				SetTextColor(dc,lblCol);
				SelectObject(dc,fnt);

				// tp[i].y = top + height/2 - trace[iTrace].data[i]*height/51200;
				// y = -4*deltay + y00  where deltay is the vertical scale / 8
				// 8 * dt is x-axis
				infoy += 3;
				rc.left = left+width+18; rc.right=rc.left;
				rc.top = infoy;rc.bottom=rc.top;
				DrawText(dc,_T("Cursor info:"),12,&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;

				rc.top = infoy;rc.bottom=rc.top;
				sprintf_s(valstr,100,"X1 = %3.2f - Y1 = %3.2f\r\n", Cursor[2].value, Cursor[0].value);
				DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;

				rc.top = infoy;rc.bottom=rc.top;
				sprintf_s(valstr,100,"X2 = %3.2f - Y2 = %3.2f", Cursor[3].value,Cursor[1].value);
				DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;

				double dt = Cursor[3].value - Cursor[2].value;
				double f = 1.001/dt;
				rc.top = infoy;rc.bottom=rc.top;rc.left += 10;
				char funit[10];
				if (strcmp(tunit,"ms")==0) strcpy_s(funit,10,"kHz");
				else if (strcmp(tunit,"us")==0) strcpy_s(funit,10,"MHz");
				else if (strcmp(tunit,"s")==0) strcpy_s(funit,10,"Hz");
				else if (strcmp(tunit,"ns")==0) strcpy_s(funit,10,"GHz");
				else strcpy_s(funit,10,"-");
				sprintf_s(valstr,100,"T = %3.3f %s  Freq = %3.2f %s",dt,tunit,f,funit);
				CString csvalstr = CString(valstr);   
				DrawText(dc,csvalstr,(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;
				rc.left -= 10;
				POINT pnt[4];
				pnt[0].x = rc.left + 1; pnt[0].y = rc.top + 9;
				pnt[1].x = rc.left + 4; pnt[1].y = rc.top + 2;
				pnt[2].x = rc.left + 7; pnt[2].y = rc.top + 9;
				pnt[3].x = rc.left + 1; pnt[3].y = rc.top + 9;
				Polyline(dc,(POINT*)&pnt,4);

				double dy = Cursor[1].value - Cursor[0].value;
				rc.top = infoy;rc.bottom=rc.top;rc.left += 10;
				sprintf_s(valstr,100,"V = %3.3f   Vavg = %3.2f",
					Cursor[1].value-Cursor[0].value, 
					(Cursor[0].value+Cursor[1].value)/2.0);
				DrawText(dc,CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				
				
				infoy += yline;
				rc.left -= 10;
				pnt[0].x = rc.left + 1; pnt[0].y = rc.top + 9;
				pnt[1].x = rc.left + 4; pnt[1].y = rc.top + 2;
				pnt[2].x = rc.left + 7; pnt[2].y = rc.top + 9;
				pnt[3].x = rc.left + 1; pnt[3].y = rc.top + 9;
				Polyline(dc,(POINT*)&pnt,4);

					
				DeleteObject(fnt);
				DeleteObject(art);
			}
		}
		if ((zooming) && (zoomA.x>0))
		{
			art = CreatePen(PS_DASH,1,RGB(255,0,0));
			SelectObject(dc,art);
			MoveToEx(dc,zoomA.x,zoomA.y,NULL);
			LineTo(dc,zoomA.x,zoomB.y);
			LineTo(dc,zoomB.x,zoomB.y);
			LineTo(dc,zoomB.x,zoomA.y);
			LineTo(dc,zoomA.x,zoomA.y);
			DeleteObject(art);
		}
	}

#if _USEDOUBLEBUFFER_
	if (hBmp == NULL)
	{
		left = cwnd.left;
		top = cwnd.top;
		BitBlt(dc0,left,top,dx,dy,dc,0,0,SRCCOPY);
		DeleteObject(dcbmp);
		DeleteDC(dc);
	}
#endif
}

void SaveTraceFile(BOOL ask)
{
	FILE *storeFile;
	int nSuccess = 1;

	if ((ask == 1) || (strlen(lfn.lpstrFile)==0)) 
		nSuccess = 0;
	if (nSuccess == 0)
	{
		lfn.lpstrTitle        = _T("Select Filename");
		lfn.lpstrFilter       = _T("Trace Files (*.trc)\0*.trc\0All Files (*.*)\0*.*\0\0");
		lfn.lpstrInitialDir   = _T("");
		lfn.nFilterIndex      = 1;
		// if data was loaded, then save it, otherwise read it
		
		nSuccess = GetSaveFileName(&lfn);
		if (nSuccess) 
		{
			RegisterMru(lfn.lpstrFile);
			UpdateStatus();
		}
	}
	if (nSuccess)
	{
		if (strchr((char*)lfn.lpstrFile,'.')==NULL)
			strcat_s((char*)lfn.lpstrFile,100,".trc");
		int nResult = fopen_s(&storeFile,(const char*)lfn.lpstrFile,(const char*)"wbc");  // write;binary;commit
		if (nResult)
		{
			TRACE0("Trace File Open problem.\r\n");
			return;
		}
		fwrite(trace,sizeof(TRACE_T),MAX_TRACE,storeFile);
		fwrite(cdata,sizeof(int),35000,storeFile);
		fwrite(mdata,sizeof(int),65000,storeFile);
		fclose(storeFile);
	} 
}

void OpenTraceFile(BOOL ask)
{
	FILE *storeFile;
	size_t ntr, ncd, nmd;
	int nSuccess = 1;

	if ((ask == 1) || (strlen(lfn.lpstrFile)==0)) nSuccess = 0;
	if (nSuccess == 0)
	{

		lfn.lpstrTitle        = _T("Select Filename");
		lfn.lpstrFilter       = _T("Trace Files (*.trc)\0*.trc\0All Files (*.*)\0*.*\0\0");
		lfn.lpstrInitialDir   = _T("");
		lfn.nFilterIndex      = 1;
		// if data was loaded, then save it, otherwise read it
		nSuccess = GetOpenFileName(&lfn);
	}
	if (nSuccess)
	{
		if (strchr((char*)lfn.lpstrFile,'.')==NULL)
			strcat_s((char *)lfn.lpstrFile,MAX_FILENAME,".trc");
		int nResult = fopen_s(&storeFile,(const char*)lfn.lpstrFile,(const char*)"rb");  // write;binary;commit
		if (nResult)
		{
			TRACE0("Trace File Open problem.\r\n");
			return;
		}
		ntr = fread(trace,sizeof(TRACE_T),MAX_TRACE,storeFile);
		ncd = fread(cdata,sizeof(int),35000,storeFile);
		nmd = fread(mdata,sizeof(int),65000,storeFile);
		TRACE3("Read %i, %i and %i bytes\r\n",ntr,ncd,nmd);
		RegisterMru(lfn.lpstrFile);
		UpdateStatus();
		fclose(storeFile);

		bool foundfirsttrace = false;
		for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
		{
			if (trace[iTrace].ntrace>0) 
			{
				if (!foundfirsttrace)
				{
					npoints = trace[iTrace].ntrace;
					xres = trace[iTrace].x_res;
					dt = trace[iTrace].x_res*npoints/10.24;
					strcpy_s(tunit,20,"s");
					if (trace[iTrace].x_res<0.01) {dt *= 1000.0F; strcpy_s(tunit,20,"ms");}
					if (trace[iTrace].x_res<1e-6) {dt *= 1000.0F; strcpy_s(tunit,20,"us");}
					strcpy_s(tlabel,20,"Time [");strcat_s(tlabel,20,tunit);strcat_s(tlabel,20,"]");
					y00 = trace[iTrace].y_zero;
					deltay = trace[iTrace].y_res*6400*0.5;
					foundfirsttrace = true;
					viewVmax = ygrid/2.0*deltay;
					viewVmin = ygrid/-2.0*deltay;
					viewTmin = 0.0;
					viewTmax = xgrid*dt;
					if (trace[iTrace].ntrace==255) 
					{
						trace[iTrace].ntrace=512;
						TRACE("Warning!! npnts = 255\r\n");
					}
					SetTraceData(npoints);
					it = iTrace;
					fftMode = false;
					g_pFramework->InvalidateUICommand(IDM_SELECTCOLOR,UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);
					g_pFramework->InvalidateUICommand(IDM_FFT,UI_INVALIDATIONS_VALUE, &UI_PKEY_BooleanValue);

					TRACE1("TRACE INFO: %s\r\n",trace[iTrace].trace_id);
					TRACE1("Y_0: %f\r\n",trace[iTrace].y_zero);
					TRACE1("X_0: %f\r\n",trace[iTrace].x_zero);
					TRACE1("Y_res: %f\r\n",trace[iTrace].y_res);
					TRACE1("X_res: %f\r\n",trace[iTrace].x_res);
					TRACE1("Y_rng: %f\r\n",trace[iTrace].y_range);
					TRACE1("D_t: %f\r\n",trace[iTrace].delta_t);
					TRACE1("Samples: %i\r\n",npoints);
				}
			} 
			else
			{
			}
		}
		hBmp = NULL;
		ResetZoom();
		InvalidateRect(m_hWnd,NULL,TRUE);
		UpdateWindow(m_hWnd);
	} 
}

void SaveAsCSV()
{
	FILE *storeFile;
	int nSuccess = 1;
	char eFile[MAX_FILENAME], eFullFile[MAX_FILENAME];
	OPENFILENAME efn;

	memcpy((void*)&efn.lStructSize,(void*)&lfn.lStructSize,sizeof(OPENFILENAME));

	efn.lpstrTitle        = _T("Select Filename");
	efn.lpstrFilter       = _T("Excel Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0\0");
	efn.lpstrInitialDir   = _T("");
	efn.nFilterIndex      = 1;
	efn.lpstrFile		  = eFullFile;
	efn.nMaxFile		  = MAX_FILENAME;
	efn.lpstrTitle		  = eFile;
	efn.nMaxFileTitle     = MAX_FILENAME;

	// if data was loaded, then save it, otherwise read it
	
	nSuccess = GetSaveFileName(&efn);
	
	if (nSuccess)
	{
		if (strchr((char*)efn.lpstrFile,'.')==NULL)
			strcat_s((char*)efn.lpstrFile,MAX_FILENAME,".csv");
		int nResult = fopen_s(&storeFile,(const char*)efn.lpstrFile,(const char*)"wbc");  // write;binary;commit
		if (nResult)
		{
			TRACE0("Trace File Open problem.\r\n");
			return;
		}
		
		sprintf_s(pTrace,CPYLEN,"Time [%s]\t",tunit);
		for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
		{
			if (trace[iTrace].ntrace>0) 
			{
				sprintf_s(pTraceLine,100,"Trace %i\t",iTrace+1);
				strcat_s(pTrace,CPYLEN,pTraceLine);
			}
		}
		strcat_s(pTrace,CPYLEN,"\r\n");

		int smin = (int)(viewTmin/dt*npoints/xgrid);
		int smax = (int)(viewTmax/dt*npoints/xgrid);
		for (int i=smin;i<smax;i++)
		{
			sprintf_s(pTraceLine,100,"%f\t",(viewTmin + (i-smin)*(viewTmax-viewTmin)/(smax-smin)));
			strcat_s(pTrace,CPYLEN,pTraceLine);
			for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
			{
				if (trace[iTrace].ntrace>0) 
				{
					{
						sprintf_s(pTraceLine,100,"%f\t",(float)trace[iTrace].data[i]*ygrid*deltay/51200.0);
						strcat_s(pTrace,CPYLEN,pTraceLine);
					}
				}
			}
			strcat_s(pTrace,CPYLEN,"\r\n");
		}
		fwrite(pTrace,sizeof(char),(int)strlen(pTrace),storeFile);
		fclose(storeFile);
	} 
}

BOOL InitCommPort(int CommPortNum, int baud,
	BYTE ByteSize, BYTE Parity, BYTE StopBits)
{
	// defaults to COM1, BAUD192, BYTESIZE8, NOPARITY, ONESTOPBIT
	sprintf_s(CommPort,12,"COM%i",CommPortNum);
	hCommPort = CreateFile(CString(CommPort), GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING,	FILE_ATTRIBUTE_NORMAL, NULL);
	if (hCommPort == INVALID_HANDLE_VALUE) {
		DWORD dwError = GetLastError();
		// error 2: File Not Found

		//char* tmp = new char[128];
		//sprintf_s(tmp,128, "Can't open serial port.  error = %d", dwError);
		//MessageBox(NULL, tmp, "Open Link Error",MB_ICONSTOP);
		//delete tmp;
		return FALSE;
	}
	BOOL fSuccess = GetCommState(hCommPort, &dcbCommPort);
	if (!fSuccess) {
		//MessageBox(NULL, "Couldn't get comm state.", "Error",MB_OK + MB_ICONSTOP);
		return fSuccess;
	}
	dcbCommPort.DCBlength = sizeof(DCB);
	
	dcbCommPort.BaudRate = baud;
	dcbCommPort.ByteSize = byteSize = ByteSize;
	dcbCommPort.Parity = parity = NOPARITY;
	dcbCommPort.StopBits = stopBits = ONESTOPBIT;
	dcbCommPort.fInX = FALSE;
	dcbCommPort.fOutX = FALSE;
	dcbCommPort.fOutxCtsFlow = FALSE;
	dcbCommPort.fOutxDsrFlow = FALSE;
	dcbCommPort.fDtrControl = DTR_CONTROL_ENABLE;
	dcbCommPort.fRtsControl = RTS_CONTROL_ENABLE;
	fSuccess = SetCommState(hCommPort, &dcbCommPort);
	if (!fSuccess) {
		//MessageBox(NULL, "Couldn't set comm state.", "Error",MB_OK + MB_ICONSTOP);
		return FALSE;
	}
	CommTimeouts.ReadIntervalTimeout = 500;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 1;
	CommTimeouts.WriteTotalTimeoutConstant = 1;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
	if (!fSuccess) {
		//MessageBox(NULL, "Couldn't set comm timeouts.", "Error",MB_OK + MB_ICONSTOP);
		return FALSE;
	}
   return TRUE;
}

void Connect(int port, int baud)
{
	char iddata[1000];
	unsigned long nc;

	if (hCommPort != INVALID_HANDLE_VALUE)
	{
		// close com port first, before attempting to re-connect
		CloseHandle(hCommPort);
		hCommPort = INVALID_HANDLE_VALUE;
		strcpy_s(pmid,200,"--");
		UpdateStatus();
		g_pFramework->InvalidateUICommand(IDM_CONNECT,UI_INVALIDATIONS_PROPERTY, &UI_PKEY_LargeImage);
	}
	else
	{

		if (InitCommPort(port, baud, 8, 0, 1)) {
			FlushFileBuffers(hCommPort);
			memset(iddata,0,1000);
			WriteFile(hCommPort,"ID\r",3,&nc,NULL);	
			Sleep(500);
			ReadFile(hCommPort,iddata,1,&nc,NULL);
			if (nc>0)
			{
				ReadFile(hCommPort,pmid,200,&nc,NULL);
				TRACE1("Instrument ID: %s\r\n",&pmid[1]);
				UpdateStatus();
				g_pFramework->InvalidateUICommand(IDM_CONNECT,UI_INVALIDATIONS_PROPERTY, &UI_PKEY_LargeImage);
			} else {
				CloseHandle(hCommPort);
				hCommPort = INVALID_HANDLE_VALUE;
			}
		} 
	}
}



void GetTrace(int iTrace)
{
	unsigned char tdata[100000];
	unsigned long nc;
	unsigned int blocksize = 250;
	BOOL fSuccess;
	int j,i=0,di;
	short *sdata;
	char cmd[10], sstr[20];
	int rtn;
	int traceGroup = iTrace / 4;
	int traceRegister = iTrace - 4 * traceGroup + 1;


	if (iTrace<4)
		sprintf_s(cmd,10,"QW%i\r",traceRegister);
	else
		sprintf_s(cmd,10,"QW%i%i\r",traceGroup, traceRegister);
	//PurgeComm(hCommPort,PURGE_RXCLEAR | PURGE_TXCLEAR);
	FlushFileBuffers(hCommPort);
	WriteFile(hCommPort,cmd,(int)strlen(cmd),&nc,NULL);
	Sleep(100);
	rtn = ReadFile(hCommPort,tdata,1,&nc,NULL);
	if (nc==1) if (tdata[0]==13)
		rtn = ReadFile(hCommPort,tdata,1,&nc,NULL);
	
	if ((tdata[0]!='0') || (nc==0)) 
	{
		TRACE1("No response. Channel %i disabled.\r\n",iTrace+1);
		trace[iTrace].Enable = false;
		return; // error
	}
	di = 0;
	rtn = ReadFile(hCommPort,&tdata[di],blocksize,&nc,NULL);
	di += nc;

	// interpret header data
	i++;
	j=0;while (tdata[i]!=',') trace[iTrace].trace_id[j++]=tdata[i++];trace[iTrace].trace_id[j]=0;i++;
	j=0;while (tdata[i]!=',') trace[iTrace].y_unit[j++]=tdata[i++];trace[iTrace].y_unit[j]=0;i++;
	j=0;while (tdata[i]!=',') trace[iTrace].x_unit[j++]=tdata[i++];trace[iTrace].x_unit[j]=0;i++;
	sscanf_s((char*)&tdata[i],"%f",&trace[iTrace].y_zero);while (tdata[i++]!=',') {};
	sscanf_s((char*)&tdata[i],"%f",&trace[iTrace].x_zero);while (tdata[i++]!=',') {};
	sscanf_s((char*)&tdata[i],"%f",&trace[iTrace].y_res);while (tdata[i++]!=',') {};
	sscanf_s((char*)&tdata[i],"%f",&trace[iTrace].x_res);while (tdata[i++]!=',') {};
	sscanf_s((char*)&tdata[i],"%i",&trace[iTrace].y_range);while (tdata[i++]!=',') {};
	j=0;while (tdata[i]!=',') trace[iTrace].tdate[j++]=tdata[i++];trace[iTrace].tdate[j]=0;i++;
	j=0;while (tdata[i]!=',') trace[iTrace].ttime[j++]=tdata[i++];trace[iTrace].ttime[j]=0;i++;
	sscanf_s((char*)&tdata[i],"%f",&trace[iTrace].delta_t);while (tdata[i++]!=',') {};
	while (tdata[i++]!=',') {};
	while (tdata[i++]!=',') {};
	while (tdata[i++]!=',') {};
	while (tdata[i++]!=',') {};
	while (tdata[i++]!=',') {};
	sscanf_s((char*)&tdata[i],"%i",&trace[iTrace].ntrace);
	while (tdata[i++]!=',') {};

	TRACE1("TRACE INFO: %s\r\n",trace[iTrace].trace_id);
	TRACE1("Y_0: %f\r\n",trace[iTrace].y_zero);
	TRACE1("X_0: %f\r\n",trace[iTrace].x_zero);
	TRACE1("Y_res: %f\r\n",trace[iTrace].y_res);
	TRACE1("X_res: %f\r\n",trace[iTrace].x_res);
	TRACE1("Y_rng: %f\r\n",trace[iTrace].y_range);
	TRACE1("D_t: %f\r\n",trace[iTrace].delta_t);
	TRACE1("Samples: %i\r\n",trace[iTrace].ntrace);

	TRACE1("Header size = %i bytes\r\n",j);

	RECT sr;
	HDC hsDC = GetDC(m_hStatusWnd);
	HFONT hfnt = CreateFont(18,0,0,0,FW_DONTCARE,0,0,0,0,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,18,"Calibri");
	SelectObject(hsDC,hfnt);
	sr.left=0;sr.right=150;sr.top=2;sr.bottom=21;

	CommTimeouts.ReadIntervalTimeout = 1500;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
	if (!fSuccess) { return; }

	while (nc == blocksize)
	{
		rtn = ReadFile(hCommPort,&tdata[di],blocksize,&nc,NULL);
		di += nc;

		sprintf_s(sstr,20,"Reading %i %%\r\n",50*di/trace[iTrace].ntrace);
		DrawStatusText(hsDC,&sr,sstr,0);
	}
	DeleteObject(hfnt);

	CommTimeouts.ReadIntervalTimeout = 500;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);

#ifdef _DEBUG
	FILE *fid;
	fopen_s(&fid,"c:/temp/pm3394_data.bin","w");
	fwrite(tdata,1,nc,fid);
	fclose(fid);
#endif

	sdata = (short*)&tdata[i-1];
	if (trace[iTrace].ntrace>32768) 
	{
		trace[iTrace].ntrace=0;
	} else {
		// Set data pointers for all channels and memory, since this is a global allocation in the Scope
		if (npoints!=trace[iTrace].ntrace)
		{
			npoints = trace[iTrace].ntrace;
			xres = trace[iTrace].x_res;
			for (i=0;i<MAX_TRACE;i++)
				if (i!=iTrace)
					trace[i].ntrace=0;
		}
		SetTraceData(npoints);
		for (j=0;j<npoints;j++)
			trace[iTrace].data[j]=sdata[j];
	}
	dt = trace[iTrace].x_res*npoints/10.24;
	strcpy_s(tlabel,20,"Time [s]");
	if (trace[iTrace].x_res<0.01) {dt *= 1000.0F; strcpy_s(tlabel,20,"Time [ms]");}
	if (trace[iTrace].x_res<1e-6) {dt *= 1000.0F; strcpy_s(tlabel,20,"Time [us]");}
	y00 = trace[iTrace].y_zero;
	deltay = trace[iTrace].y_res*6400;
	TRACE2("dt = %1.3f %s\r\n",dt,tlabel);
	TRACE1("deltay = %1.3f \r\n",deltay);
	viewVmax = ygrid/2.0*deltay;
	viewVmin = ygrid/-2.0*deltay;
	viewTmin = 0.0;
	viewTmax = xgrid*dt;
	fftMode = false;
	// update fft button
}



// QS
// 25,
// 01068813FB000000,
// 02068813FB000000,
// 0306E803FC000000,
// 04068813FC01CAFF,
// 0E1601010101000003030303E803FDFDE803E803FDFDE803,
// 0F0488000300,
// 100708504000090000,
// 1118E803FA1006014F0043CEFFFF00112EFD0100A0860140420F,
// 120BE803F91000000446000000,
// 130701000000400401,
// 14022400,
// 2020210906FFFA0006FFFA00F401F401400004000400400040007D007701FA000100,
// 21102000C800200313130000000000000000,
// 31082904040004000000,
// 3206080A04000400,
// 41146105010002006400000013050000FF0100000100,
// 42146004010002006400000013050000FF0100000100,
// 33120000040011002100E8030A00F6FF00000CFE,
// 5019072F0000E0FC04000400000000000000000000000000000000,
// 5103B4000F,
// 52200000000000000000000000000000000000000000000000000000000000000000,
// 601620202020202020202020202020202020202020202020,
// 700600FD0500E803,
// 80020000,
// F003203000

// 2020A30906FFFA0006FFFA00F401F401400004000400400040007D007701FA000100

/*
void CPM3394ADlg::OnBnClickedGetSetup()
{
	char tdata[1000];
	unsigned long nc;

	FlushFileBuffers(hCommPort);
	WriteFile(hCommPort,"QS20\r",5,&nc,NULL);	
	ReadFile(hCommPort,tdata,10000,&nc,NULL);

	/*  33 bytes
	voltage/time cursors on/off, 
	rise time on/off, 
	cursor control volt/time,
	Vpp on/off, 
	rise time 10-90%/20-80%, 
	voltage readout Vpp/Vp-Vp+, 
	readout on/off: delta-V, 
	absolute V1&V2, 
	voltage ratio, 
	delta-T, 
	1/delta-T, 
	time ratio, 
	time phase, 
	Vdc, 
	X cursor 1/2, 
	Y cursor 1/2,
	X/Y ratio, 
	cursor source CH1/2/3/4, 
	track & delta control, 
	ref. & delta control, 
	degrees cursors horizontal and vertical selection,
	V1 & V2 readout, 
	dBm/dBmV/ Vrms readout, 
	FFT ref. impedance 50W/600W, 
	digital source cursor 1/2 CHn, 
	Mi_j, 
	magnify factor delta-X/Y ratio.
	*/
/*
}
*/

void Print()
{
	char txt[1000];
	HFONT prnFont;
	RECT rt;
	PRINTDLGEX pPrint;
	DOCINFO pDoc;
	PRINTPAGERANGE pPage;

	pPrint.lStructSize = sizeof(PRINTDLGEX);
	pPrint.hwndOwner = m_hWnd;
	pPrint.hDevMode = NULL;
	pPrint.hDevNames = NULL;
	pPrint.hDC = NULL;
	pPrint.Flags = PD_RETURNDC | PD_COLLATE;
	pPrint.Flags2 = 0;
	pPrint.ExclusionFlags = 0;
	pPrint.nPageRanges = 0;
	pPrint.nMaxPageRanges = 10;
	pPrint.lpPageRanges = &pPage;
	pPrint.nMinPage = 1;
	pPrint.nMaxPage = 1000;
	pPrint.nCopies = 1;
	pPrint.hInstance = 0;
	pPrint.lpPrintTemplateName = NULL;
	pPrint.lpCallback = NULL;
	pPrint.nPropertyPages = 0;
	pPrint.lphPropertyPages = NULL;
	pPrint.nStartPage = START_PAGE_GENERAL;
	pPrint.dwResultAction = 0;

	PrintDlgEx(&pPrint);

	int iPrnX = GetDeviceCaps(pPrint.hDC,HORZRES);
	int iPrnY = GetDeviceCaps(pPrint.hDC,VERTRES);
	
	sprintf_s(txt,1000,"PM3394A");
	
	pDoc.fwType = DI_ROPS_READ_DESTINATION;
	pDoc.lpszDocName = txt;
	pDoc.lpszDatatype = NULL;
	pDoc.lpszOutput = NULL;
	pDoc.cbSize = sizeof(DOCINFO);

	StartDoc(pPrint.hDC,&pDoc);
	StartPage(pPrint.hDC);
	
	// print map
	Draw(pPrint.hDC, 0);
	
	// print version text
	/*
	prnPen.CreatePen(PS_SOLID,2,RGB(0,0,128));
	prnDC.SelectObject(&prnPen);
	prnDC.Rectangle(100,y,iPrnX - 100,y+150+120*nl);
	y -= 30;

	prnDC.SetTextColor(RGB(0,0,0));
	prnFont.CreateFont(120,0,0,0,0,0,0,0,0,0,0,0,12,"Tahoma");
	prnDC.SelectObject(&prnFont);

	prnDC.SetTextColor(RGB(0,0,128));
	
	rt.top=y; rt.bottom=rt.top;
	rt.left=150; rt.right=rt.left;
	prnDC.DrawText("Path statistics:",16,&rt,DT_NOCLIP);

	y += 120;
	prnDC.SetTextColor(RGB(0,0,0));
	
	rt.top=y; rt.bottom=rt.top;
	rt.left=150; rt.right=rt.left;
	if (nWP == 0) dist = 0.0;
	if (areaShape)
		sprintf_s(txt,1000,"Path Length = %1.2f m\r\nArea = %1.2f m2\r\nWaypoints = %i\r\nTarget = (%4.2f,%4.2f)\r\nSpeed = %1.2f m/s",
		dist,area,nWP,tgt.x,tgt.y,spd);
	else
		sprintf_s(txt,1000,"Path Length = %1.2f m\r\nWaypoints = %i\r\nTarget = (%4.2f,%4.2f)\r\nSpeed = %1.2f m/s",
		dist,nWP,tgt.x,tgt.y,spd);
	prnDC.DrawText(txt,(int)strlen(txt),&rt,DT_NOCLIP);
	
	rt.top=y; rt.bottom=rt.top;
	rt.left=iPrnX / 3; rt.right=rt.left;
	sprintf_s(txt,1000,"Stroke Width: %1.3f m\r\nStroke Angle: %1.3f m\r\nStroke Offset: %1.3f m\r\nWaypoint Separation: %1.3f m\r\n",
		StrokeWidth,StrokeAngle,StrokeOffset,WAYPOINT_SEPERATION);
	prnDC.DrawText(txt,(int)strlen(txt),&rt,DT_NOCLIP);
	
	
	prnFont.DeleteObject();
	*/




	// print footer
	SetTextColor(pPrint.hDC,RGB(180,180,180));
	prnFont = CreateFont(80,0,0,0,0,0,0,0,0,0,0,0,10,"Tahoma");
	SelectObject(pPrint.hDC,prnFont);
	rt.left=iPrnX / 2; rt.top=iPrnY - 150; rt.right=rt.left; rt.bottom=rt.top;
	DrawText(pPrint.hDC,"Edzko Smid",11,&rt,DT_NOCLIP | DT_CENTER);
	DeleteObject(prnFont);
	
	EndPage(pPrint.hDC);
	EndDoc(pPrint.hDC);
	
	DeleteObject(prnFont);
}


void ResetZoom()
{
	if (zoomMode)
	{
	} else {
		zooming = false;  // reset zoom
		if (fftMode)
		{
		viewVmax = ygrid*deltay/2.0F;
		viewVmin = 0.0F;
		}
		else
		{
		viewVmax = ygrid/2.0*deltay;
		viewVmin = ygrid/-2.0*deltay;
		}
		viewTmin = 0.0;
		viewTmax = xgrid*dt;		
		myUpdate = false;
		InvalidateRect(m_hWnd,NULL,TRUE);
		UpdateWindow(m_hWnd);
	}
}



unsigned int	ReverseBits(unsigned int p_nIndex, unsigned int p_nBits)
{
	unsigned int i, rev;

	for(i=rev=0; i < p_nBits; i++)
	{
		rev = (rev << 1) | (p_nIndex & 1);
		p_nIndex >>= 1;
	}
	return rev;
}

unsigned int	NumberOfBitsNeeded(unsigned int p_nSamples)
{
	int i;

	if( p_nSamples < 2 )
	{
		return 0;
	}
	for ( i=0; ; i++ )
	{
		if( p_nSamples & (1 << i) ) return i;
	}
}

void fft_float(float *DataPoints, unsigned int SampleCount)
{
	float *p_RealOut = NULL;	// Pointer to array of real part of result
	float *p_ImagOut = NULL;	// Pointer to array of imaginary part of result
	unsigned int NumBits;
	unsigned int i, j, k, n;
	unsigned int BlockSize, BlockEnd;

	float angle_numerator = 2.0 * PI;
	float tr, ti;	// Temp real, temp imaginary

	NumBits = NumberOfBitsNeeded(SampleCount);

	p_RealOut = (float *) malloc(sizeof(float)*SampleCount);	// Get memory for real result
	if (NULL == p_RealOut) return;	// Return if couldn't get memory

	p_ImagOut = (float *) malloc(sizeof(float)*SampleCount);	// Get memory for imag result
	if (NULL == p_ImagOut)	// If couldn't get memory ...
	{
		free(p_RealOut);		// ... free memory that we did get ...
		return;					// ... and return
	}

	for( i = 0; i < SampleCount; i++)
	{
		j = ReverseBits (i, NumBits);
		p_RealOut[j] = DataPoints[i];
		p_ImagOut[j] = 0.0;
	}

	BlockEnd = 1;
	for( BlockSize = 2; BlockSize <= SampleCount; BlockSize <<= 1 )
	{
		float delta_angle = angle_numerator / (float)BlockSize;
		float sm2 = sin(-2 * delta_angle);
		float sm1 = sin(-delta_angle);
		float cm2 = cos(-2 * delta_angle);
		float cm1 = cos(-delta_angle);
		float w = 2 * cm1;
		float ar[3], ai[3];

		for( i = 0; i < SampleCount; i += BlockSize)
		{
			ar[2] = cm2;
			ar[1] = cm1;

			ai[2] = sm2;
			ai[1] = sm1;

			for (j = i, n = 0; n < BlockEnd; j++, n++)
			{
				ar[0] = w*ar[1] - ar[2];
				ar[2] = ar[1];
				ar[1] = ar[0];

				ai[0] = w*ai[1] - ai[2];
				ai[2] = ai[1];
				ai[1] = ai[0];

				k = j + BlockEnd;
				tr = ar[0]*p_RealOut[k] - ai[0]*p_ImagOut[k];
				ti = ar[0]*p_ImagOut[k] + ai[0]*p_RealOut[k];

				p_RealOut[k] = p_RealOut[j] - tr;
				p_ImagOut[k] = p_ImagOut[j] - ti;

				p_RealOut[j] += tr;
				p_ImagOut[j] += ti;
			}
		}

		BlockEnd = BlockSize;
	}
	// Now put results in source waveform, overwriting original data
	for(i=0; i < SampleCount/2; i++)	// Copy unique half of data to every other point in dest
	{
		DataPoints[2*i] = (sqrtf(p_RealOut[i]*p_RealOut[i] + p_ImagOut[i]*p_ImagOut[i]))/SampleCount;
	}
	for(i=0; i < SampleCount; i += 2)	// Average inbetween points
	{
		DataPoints[i+1] = 0.5F * (DataPoints[i] + DataPoints[i+2]);
	}
	free(p_RealOut);		// Free temp memory
	free(p_ImagOut);
}	// End of fft_float()

#define	COS_SERIES_WINDOW(p0, p1, p2, p3, p4)	(p0)	\
				-(p1)*cos(2*PI*i/(npoints-1))	\
				+(p2)*cos(4*PI*i/(npoints-1))	\
				-(p3)*cos(6*PI*i/(npoints-1))	\
				+(p4)*cos(8*PI*i/(npoints-1))

#define	sigma 0.4F	// Choose std deviation (sigma) of 0.4, any value less than 0.5 possible

void Spectrum(int _trace)
{
	int i;
	float src_data[35000];

	for (i=0;i<npoints;i++) 
		src_data[i] = (float)(trace[_trace].data[i]/51200.0F*trace[_trace].y_res);

	// Apply windowing function to source data and save in target
	for ( i = 0; i < npoints; i++ )
	{
		switch(wndFcn)
		{
		case 2: // Gaussian window. Choose std deviation (sigma) of 0.4, any value less than 0.5 possible
			fft_data[i] = (src_data[i] * expf( -0.5F * pow((i - (npoints-1)/2)/(sigma*(npoints-1)/2),2))); 
			break;
		case 3: // Hamm
			fft_data[i] = (src_data[i] * (0.53836F - 0.46164F * cosf(2.0F * PI * i / ( npoints - 1) ) ) ); 
			break;
		case 4: // Hann
			fft_data[i] = (src_data[i] * 0.5F * (1-cosf(2.0F*PI*i/(npoints-1)))); 
			break;
		case 5: // Bart
			fft_data[i] = (src_data[i] * ( 2.0F / (npoints-1)) * (((npoints-1)/2.0F)-fabs(i-((npoints-1)/2.0F)))); 
			break;
		case 6: // Wlch
			fft_data[i] = (src_data[i] * ( 1.0F - pow(((i - npoints/2.0F)/(npoints/2.0F)),2))); 
			break;
		case 7: // Triangle
			fft_data[i] = (src_data[i] * ( 2.0F / (npoints)) * ((npoints/2.0F)-fabs(i-((npoints-1)/2.0F)))); 
			break;
		case 8: // Brhn
			fft_data[i] = (src_data[i] * (0.62F - 0.48F*fabsf(i/(npoints-1)-0.5F) - 0.38F*cosf(2.0F*PI*i/(npoints-1)))); 
			break;
		case 9: // Blkm
			fft_data[i] = (src_data[i] * (0.42F - 0.5F*cosf(2.0F*PI*i/(npoints-1)) - 0.08F*cosf(4.0F*PI*i/(npoints-1)))); 
			break;
		case 10: // Kais
			fft_data[i] = 0.0F;	// Work here - Requires research... Kaiser-Bessel

			// Work here - Requires research... Kaiser-Bessel
			// There is a parameter, "alpha," required to be selected by the user
			// See sample code at bottom, taken from http://ccrma.stanford.edu/courses/422/projects/kbd/
			 break;
		case 11: // Nutl
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW( 0.355768F, 0.487396F, 0.144232F, 0.012604F, 0.0F)); 
			break;
		case 12: // Bkha
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW( 0.35875F, 0.48829F, 0.14128F, 0.01168F, 0.0F)); 
			break;
		case 13: // Bknu
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW( 0.3635819F, 0.4891775F, 0.1365995F, 0.0106411F, 0.0F)); 
			break;
		case 14: // Fltt
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW( 1.0F, 1.93F, 1.29F, 0.388F, 0.032F)); 
			break;
		case 15: // Sine
			fft_data[i] = (src_data[i] * sinf(PI * i / (npoints - 1)));
			break;
		default: // Rectangle window - does nothing
			fft_data[i] = src_data[i]; 
			break;
		}
	}

	fft_float(fft_data, npoints);

	// find min and max
	float fftmin = 1e6F, fftmax = 0.0F;
	int imax;
	for ( i = 0; i < npoints; i++ )
	{
		if (fft_data[i]<fftmin) fftmin = fft_data[i];
		if (fft_data[i]>fftmax) {fftmax = fft_data[i];imax=i;}
	}
	TRACE2("FFT: peak at %i/%i\r\n",imax,npoints);

	for ( i = 0; i < npoints; i++ )
		trace[_trace].fdata[i] = (int)(fft_data[i] * 25600.0F / fftmax);

	trace[_trace].fftmax = fftmax;
}

void SendMail()
{
	ShellExecute(NULL, "open", 
		"mailto:gesmid@gmail.com?Subject=Subject Text&body=Body text", 
		"", "", SW_SHOWNORMAL );
}