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

// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "PM33XX_MFC.h"
#include "MainFrm.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	topMargin = 0;
	hBmp = NULL;
	myUpdate = false;
	char tcr[50];
	RegCreateKey(HKEY_CURRENT_USER, "Software\\TEC\\PM33XX", &hKey);
	RegCreateKey(HKEY_CURRENT_USER, "Software\\TEC\\PM33XX\\MRU", &hmruKey);
	DWORD rType = REG_DWORD, nc;
	LONG lRtn;

	// setup default colors for traces
	for (int i = 0; i < 36; i++)
	{
		sprintf_s(tcr, 50, "TC%i", i + 1);
		lRtn = ::RegQueryValueEx(hKey, tcr, 0, (LPDWORD)&rType, (BYTE*)&trace[i].color, (DWORD*)&nc);
		if (lRtn != ERROR_SUCCESS) 
		{
			if (i < 4)
				trace[i].color = RGB(255, 0, 0);
			else
				trace[i].color = RGB(0, 200, 0);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		sprintf_s(tcr, 50, "TC%i", i + 37);
		lRtn = ::RegQueryValueEx(hKey, tcr, 0, (LPDWORD)&rType, (BYTE*)&Cursor[i].color, (DWORD*)&nc);
		if (lRtn != ERROR_SUCCESS) Cursor[i].color = RGB(255, 0, 0);
		Cursor[i].width = 1;
	}
	cursorFcn = 0;

	lRtn = ::RegQueryValueEx(hKey, "GC", 0, (LPDWORD)&rType, (BYTE*)&gridCol, (DWORD*)&nc);
	if (lRtn != ERROR_SUCCESS) gridCol = RGB(0, 0, 0);
	lRtn = ::RegQueryValueEx(hKey, "LC", 0, (LPDWORD)&rType, (BYTE*)&lblCol, (DWORD*)&nc);
	if (lRtn != ERROR_SUCCESS) lblCol = RGB(0, 0, 128);
	lRtn = ::RegQueryValueEx(hKey, "BC", 0, (LPDWORD)&rType, (BYTE*)&bgCol, (DWORD*)&nc);
	if (lRtn != ERROR_SUCCESS) bgCol = RGB(255, 255, 255);

	for (int i = 0; i < MAX_TRACE; i++)
	{
		trace[i].width = 2;
		trace[i].ntrace = 0;
		if (i < 4)
			trace[i].Enable = true;
		else
			trace[i].Enable = false;
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
	strcpy_s(tunit, 20, "ms");
	strcpy_s(tlabel, 20, "Time [");
	strcat_s(tlabel, 20, tunit);
	strcat_s(tlabel, 20, "]");
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
	zoomA.x = 0; zoomA.y = 0;
	zoomB.x = 0; zoomB.y = 0;
	viewTmin = 0.0;
	viewTmax = 1.0;
	viewVmin = ygrid / -2.0*deltay;
	viewVmax = ygrid / 2.0*deltay;
	wndFcn = 0;
	// if not exist ver.inf
	// system("ver > ver.inf");
	// load ver

	lfn.lStructSize = sizeof(lfn);
	lfn.hwndOwner = NULL;
	lfn.hInstance = NULL;
	lfn.lpstrCustomFilter = NULL;
	lfn.nMaxCustFilter = 0;
	lfn.nFileOffset = 0;
	lfn.nFileExtension = 0;
	lfn.lpstrDefExt = NULL;
	lfn.lCustData = NULL;
	lfn.lpfnHook = NULL;
	lfn.lpTemplateName = NULL;
	lfn.Flags = OFN_SHOWHELP | OFN_EXPLORER;
	lfn.lpstrFile = (LPSTR)DefFullName;
	lfn.nMaxFile = MAX_FILENAME;
	lfn.lpstrFileTitle = (LPSTR)DefName;
	lfn.nMaxFileTitle = MAX_FILENAME;

	showInfo = false;
	cursorMoving = false;
	strcpy_s(DefFullName, MAX_FILENAME, "");
	strcpy_s(DefName, MAX_FILENAME, "");

	strcpy_s(BaudRates[0], 20, "4800");
	strcpy_s(BaudRates[1], 20, "9600");
	strcpy_s(BaudRates[2], 20, "19200");
	strcpy_s(BaudRates[3], 20, "38400");
	strcpy_s(BaudRates[4], 20, "57600");

	strcpy_s(pmid, 200, "--");
	
	SelectedBaudrate = 3;
	SelectedComport = 0;
	progress = 0;
}

CChildView::~CChildView()
{
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CLOSE()
	ON_WM_CHAR()
	ON_WM_ERASEBKGND()
	
	ON_COMMAND(ID_CONNECT, &CChildView::OnConnect)
	ON_COMMAND(ID_HARDCOPY, &CChildView::OnHardcopy)
	ON_COMMAND(ID_PORT, &CChildView::OnPort)
	ON_COMMAND(ID_BAUD, &CChildView::OnBaud)
	ON_COMMAND(ID_COPY, &CChildView::OnCopy)
	ON_COMMAND(ID_CONFIG, &CChildView::OnConfig)
	ON_COMMAND(ID_GETTRACES, &CChildView::OnGettraces)
	ON_COMMAND(ID_RUN, &CChildView::OnRun)
	ON_COMMAND(ID_COLOR, &CChildView::OnColor)
	ON_COMMAND(ID_FFTWINDOW, &CChildView::OnFftwindow)
	ON_COMMAND(ID_C0, &CChildView::OnC0)
	ON_COMMAND(ID_CX, &CChildView::OnCx)
	ON_COMMAND(ID_CXY, &CChildView::OnCxy)
	ON_COMMAND(ID_CY, &CChildView::OnCy)
	ON_COMMAND(ID_CURSORS, &CChildView::OnCursors)
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
	ON_COMMAND(ID_ZOOM, &CChildView::OnZoom)
	ON_COMMAND(ID_INFO, &CChildView::OnInfo)
	ON_COMMAND(ID_XLOG, &CChildView::OnXLog)
	ON_COMMAND(ID_YLOG, &CChildView::OnYLog)
	ON_COMMAND(ID_FILE_NEW, &CChildView::OnFileNew)
	ON_COMMAND(ID_FILE_PRINT, &CChildView::OnFilePrint)
	ON_COMMAND(ID_FILE_SAVE_AS, &CChildView::OnFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE, &CChildView::OnFileSave)
	ON_COMMAND(ID_FFT, &CChildView::OnFft)
	ON_COMMAND(ID_XLOG, &CChildView::OnXlog)
	ON_COMMAND(ID_YLOG, &CChildView::OnYlog)
	ON_COMMAND_RANGE(ID_FLTWND1,ID_FLTWND15, &CChildView::OnFftWnd)
	ON_COMMAND(ID_FILE_MRU_FILE1, &CChildView::OnFileMruFile1)
	ON_COMMAND(ID_FILE_MRU_FILE2, &CChildView::OnFileMruFile2)
	ON_COMMAND(ID_FILE_MRU_FILE3, &CChildView::OnFileMruFile3)
	ON_COMMAND(ID_FILE_MRU_FILE4, &CChildView::OnFileMruFile4)
	ON_COMMAND(ID_FILE_MRU_FILE5, &CChildView::OnFileMruFile5)
	ON_COMMAND(ID_FILE_MRU_FILE6, &CChildView::OnFileMruFile6)
	ON_COMMAND(ID_FILE_MRU_FILE7, &CChildView::OnFileMruFile7)
	ON_COMMAND(ID_FILE_MRU_FILE8, &CChildView::OnFileMruFile8)
	ON_COMMAND(ID_FILE_MRU_FILE9, &CChildView::OnFileMruFile9)

	ON_COMMAND_RANGE(ID_CH1, ID_CH4, &CChildView::OnChannel)
	ON_COMMAND_RANGE(ID_M1, ID_M16, &CChildView::OnMemory)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, &CChildView::OnUpdateFileMruFile1)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE2, &CChildView::OnUpdateFileMruFile2)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE3, &CChildView::OnUpdateFileMruFile3)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE4, &CChildView::OnUpdateFileMruFile4)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE5, &CChildView::OnUpdateFileMruFile5)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE6, &CChildView::OnUpdateFileMruFile6)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE7, &CChildView::OnUpdateFileMruFile7)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE8, &CChildView::OnUpdateFileMruFile8)
//	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE9, &CChildView::OnUpdateFileMruFile9)

END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}





void CChildView::RegisterMru(char *fn)
{
	//HKEY hmruKey = ((CMainFrame*)AfxGetApp()->m_pMainWnd)->hmruKey;
	int imru;
	char mruKey[10];
	
	return;

	
	// if file name exist in mru, then bring to the top
	for (imru = 0; imru < 9; imru++)
		if (strcmp(mru[imru], fn) == 0) break;
	for (int i = imru; i > 0; i--)
		strcpy_s(mru[i], MAX_FILENAME, mru[i - 1]);
	strcpy_s(mru[0], MAX_FILENAME, fn);

	// Store in Registry
	// store recently used files list
	strcpy_s(mruKey, 10, "mru0");
	for (int i = 0; i < 10; i++)
	{
		mruKey[3] = '0' + i;
		LONG lRtn = ::RegSetValueEx(hmruKey, mruKey, 0, REG_SZ, (BYTE*)&mru[i], MAX_FILENAME);
	}
}

void CChildView::getCursorValue()
{
	Cursor[0].value = viewVmin + ((height - ((float)Cursor[0].pos - top + topMargin + 15)) / height) * (viewVmax - viewVmin) + y00;   // y1
	Cursor[1].value = viewVmin + ((height - ((float)Cursor[1].pos - top + topMargin + 15)) / height) * (viewVmax - viewVmin) + y00;   // y2

	Cursor[2].value = viewTmin + (viewTmax - viewTmin) * (Cursor[2].pos - left - 10) / width;  // t1
	Cursor[3].value = viewTmin + (viewTmax - viewTmin) * (Cursor[3].pos - left - 10) / width;  // t2
	TRACE2("Value(0) = %f, Pos = %i\r\n", Cursor[1].value, Cursor[1].pos);

	if (Cursor[0].value < viewVmin) Cursor[0].value = viewVmin; else if (Cursor[0].value > viewVmax) Cursor[0].value = viewVmax;
	if (Cursor[1].value < viewVmin) Cursor[1].value = viewVmin; else if (Cursor[1].value > viewVmax) Cursor[1].value = viewVmax;

	if (Cursor[2].value < viewTmin) Cursor[2].value = viewTmin; else if (Cursor[2].value > viewTmax) Cursor[2].value = viewTmax;
	if (Cursor[3].value < viewTmin) Cursor[3].value = viewTmin; else if (Cursor[3].value > viewTmax) Cursor[3].value = viewTmax;
}
void CChildView::getCursorPos()
{
	Cursor[0].pos = (int)(top + (height - (Cursor[0].value - viewVmin - y00) / (viewVmax - viewVmin)*height));
	Cursor[1].pos = (int)(top + (height - (Cursor[1].value - viewVmin - y00) / (viewVmax - viewVmin)*height));

	Cursor[2].pos = left + (int)((Cursor[2].value - viewTmin) / (viewTmax - viewTmin) * width);
	Cursor[3].pos = left + (int)((Cursor[3].value - viewTmin) / (viewTmax - viewTmin) * width);
	TRACE2("Pos(2) = %i, width=%i\r\n", Cursor[2].pos, width);

	if (Cursor[0].pos < top) Cursor[0].pos = top; else if (Cursor[0].pos > top + height) Cursor[0].pos = top + height;
	if (Cursor[1].pos < top) Cursor[1].pos = top; else if (Cursor[1].pos > top + height) Cursor[1].pos = top + height;

	if (Cursor[2].pos < left) Cursor[2].pos = left; else if (Cursor[2].pos > left + width) Cursor[2].pos = left + width;
	if (Cursor[3].pos < left) Cursor[3].pos = left; else if (Cursor[3].pos > left + width) Cursor[3].pos = left + width;
}


void CChildView::Draw(HDC dc0, int CID)
{
	HBITMAP dcbmp;
	HDC dcmem, dc;
	HPEN art;
	HBRUSH fil;
	HFONT fnt;
	BITMAP bm;
	int dx, dy, infoy = 0, fnth, yline;
	RECT rc, cwnd;
	POINT tp[35000];
	char valstr[100];

	if (CID == 0)   // from print dialog
	{
		//GetClientRect(&cwnd);
		GetClipBox(dc0, &cwnd);

		// if the page is portrait, then cut the height in half
		if (cwnd.right < cwnd.bottom) cwnd.bottom /= 2;

		cwnd.left += 200;
		cwnd.top += 200;
		cwnd.right -= 200;
		fnth = 48;
		infowidth = 800;
		yline = 80;
	}
	else if (CID == 1)
	{
		GetClipBox(dc0, &cwnd);

		cwnd.left += 2;
		cwnd.top += 10;
		fnth = 12;
		infowidth = 200;
		yline = 15;
	}
	else {
		GetClientRect(&cwnd);// control window

		cwnd.top += topMargin + 20;
		cwnd.bottom -= 30;
		cwnd.left += 30;
		cwnd.right -= 20;

		fnth = 12;
		infowidth = 200;
		yline = 15;
	}


	if (hBmp)
	{
		dcmem = CreateCompatibleDC(dc0);
		SelectObject(dcmem, hBmp);
		GetObject(hBmp, sizeof(bm), &bm);
		dx = bm.bmWidth;
		dy = bm.bmHeight;
		dcbmp = CreateCompatibleBitmap(dc0, dx, dy);
		SelectObject(dc0, dcbmp);
		double sx = (double)(cwnd.right - cwnd.left - 20) / dx;
		double sy = (double)(cwnd.bottom - cwnd.top - 40) / dy;
		if (sx < sy) sy = sx; else sx = sy;
		StretchBlt(dc0, cwnd.left + (cwnd.right - cwnd.left - 20 - (int)(sx*dx)) / 2,
			cwnd.top + (cwnd.bottom - cwnd.top - 40 - (int)(sy*dy)) / 2,
			(int)(sx*dx),
			(int)(sy*dy),
			dcmem, 0, 0, dx, dy, SRCCOPY);
		//BitBlt(dc0,0,0,dx,dy,dcmem,0,0,SRCCOPY);
		DeleteDC(dcmem);
		DeleteObject(dcbmp);

	}
	else {
#if _USEDOUBLEBUFFER_
		dc = CreateCompatibleDC(dc0);
		dx = cwnd.right - cwnd.left;
		dy = cwnd.bottom - cwnd.top;
		dcbmp = CreateCompatibleBitmap(dc0, dx, dy);
		SelectObject(dc, dcbmp);

		left = 40;
		top = 5;
		width = dx - left - 20;
		height = dy - 50;
#else
		dc = dc0;
		dx = cwnd.right - cwnd.left;
		dy = cwnd.bottom - cwnd.top;
		left = cwnd.left + 25;
		top = cwnd.top;
		width = cwnd.right - cwnd.left - 45;
		height = cwnd.bottom - cwnd.top - 60;
#endif
		if (showInfo)
		{
			width -= infowidth;
			infoy = top;
		}

#if 0
		rc.right = left + width;
		rc.left = left;
		rc.top = top;
		rc.bottom = top + height;
#else
		rc.right = dx;
		rc.left = 0;
		rc.top = 0;
		rc.bottom = dy;
#endif

		if (!cursorMoving) getCursorPos();

		fil = CreateSolidBrush(bgCol);
		SelectObject(dc, fil);
		FillRect(dc, &rc, fil);
		DeleteObject(fil);

		SetBkMode(dc, TRANSPARENT);

		// Grid lines
		float _xs = 1.0;  // scaling, based on scope unit string
		if (strcmp(tunit, "ms") == 0) _xs = 1.0e3F;
		if (strcmp(tunit, "us") == 0) _xs = 1.0e6F;
		if (strcmp(tunit, "ns") == 0) _xs = 1.0e9F;

		int efmax = (int)ceilf(log10f((float)1.0F / xres / 2.0F));
		int efmin = (int)ceilf(log10f((float)1.0F / xres / 2.0F / npoints));

		//TRACE2("LOG X-scale = [%i,%i]\r\n",efmin,efmax);
		float _yminlog = 20.0F*log10f(powf(10.0, -4.0F));
		float _ymaxlog = 20.0F*log10f(powf(10.0, 1.0F));
		float _xminlog = 20.0F*log10f(powf(10.0, (float)efmin));
		float _xmaxlog = 20.0F*log10f(powf(10.0, (float)efmax));
		int gridlinewidth = 1;
		if (fftMode)
		{
			art = CreatePen(PS_SOLID, 2, gridCol);
			SelectObject(dc, art);
			tp[0].x = left; tp[0].y = top;
			tp[1].x = left + width; tp[1].y = top;
			tp[2].x = left + width; tp[2].y = top + height;
			tp[3].x = left; tp[3].y = top + height;
			tp[4].x = left; tp[4].y = top;
			Polyline(dc, tp, 5);
			DeleteObject(art);
			if (xLog)
			{
				art = CreatePen(PS_DASH, 1, gridCol);
				SelectObject(dc, art); tp[0].y = top; tp[1].y = top + height;
				for (int i = efmin; i < efmax; i++)
				{
					for (int k = 1; k < 10; k++)
					{
						float _x = powf(10.0F, (float)i)*k;
						//TRACE1("Xgrid: %1.2f\r\n",_x); 
						tp[0].x = left + (LONG)((20.0F*log10f(_x) - _xminlog)*width / (_xmaxlog - _xminlog));
						tp[1].x = tp[0].x;
						Polyline(dc, tp, 2);
					}
				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID, 1, gridCol);
				SelectObject(dc, art);
				for (int i = efmin; i < efmax; i++)
				{
					for (int k = 10; k < 100; k += 5)
					{
						float _x = powf(10.0F, (float)i)*0.1F*k;
						tp[0].x = left + (LONG)((20.0F*log10f(_x) - _xminlog)*width / (_xmaxlog - _xminlog));
						tp[1].x = tp[0].x;
						tp[0].y = top;
						tp[1].y = top + 5;
						Polyline(dc, tp, 2);
						tp[0].y = top + height - 5;
						tp[1].y = top + height;
						Polyline(dc, tp, 2);
					}
				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
					SetTextColor(dc, lblCol);
					SelectObject(dc, fnt);
					for (int i = efmin; i <= efmax; i++)
					{
						float _x = powf(10.0F, (float)i);
						rc.left = left + (LONG)((20.0F*log10f(_x) - _xminlog)*width / (_xmaxlog - _xminlog));
						rc.right = rc.left;
						rc.top = top + height + 5; rc.bottom = rc.top;
						switch (i)
						{
						case 0: strcpy_s(valstr, 100, "1 Hz"); break;
						case 1: strcpy_s(valstr, 100, "10 Hz"); break;
						case 2: strcpy_s(valstr, 100, "100 Hz"); break;
						case 3: strcpy_s(valstr, 100, "1 kHz"); break;
						case 4: strcpy_s(valstr, 100, "10 kHz"); break;
						case 5: strcpy_s(valstr, 100, "100 kHz"); break;
						case 6: strcpy_s(valstr, 100, "1 MHz"); break;
						case 7: strcpy_s(valstr, 100, "10 MHz"); break;
						case 8: strcpy_s(valstr, 100, "100 MHz"); break;
						case 9: strcpy_s(valstr, 100, "1 GHz"); break;
						}
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_CENTER);
					}
					// draw time label
					rc.left = left + width / 2; rc.right = rc.left;
					rc.top = top + height + 15; rc.bottom = rc.top;
					DrawText(dc, "Frequency", -1, &rc, DT_NOCLIP | DT_CENTER);
					DeleteObject(fnt);
				}
			}
			else
			{
				art = CreatePen(PS_DASH, 1, gridCol);
				SelectObject(dc, art);
				for (int i = 1; i < xgrid; i++)
				{
					tp[0].x = left + i * width / xgrid; tp[0].y = top;
					tp[1].x = left + i * width / xgrid; tp[1].y = top + height;
					Polyline(dc, tp, 2);
				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID, 1, gridCol);
				SelectObject(dc, art);
				for (int i = 1; i < 40; i++)
				{
					tp[0].x = left + i * width / 40; tp[0].y = top;
					tp[1].x = left + i * width / 40; tp[1].y = top + 5;
					Polyline(dc, tp, 2);
					tp[0].x = left + i * width / 40; tp[0].y = top + height - 5;
					tp[1].x = left + i * width / 40; tp[1].y = top + height;
					Polyline(dc, tp, 2);
				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
					SetTextColor(dc, lblCol);
					SelectObject(dc, fnt);
					t = viewTmin / xres / 2.0F / (xgrid*dt) / _xs;
					y = viewVmin;
					for (int i = 0; i <= xgrid; i++)
					{
						rc.left = left + i * width / xgrid; rc.right = rc.left;
						rc.top = top + height + 5; rc.bottom = rc.top;
						sprintf_s(valstr, 100, "%1.2f", t);
						t += ((viewTmax - viewTmin) / xres / 2.0F / xgrid / (xgrid*dt)) / _xs;
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_CENTER);
					}
					// draw time label
					rc.left = left + width / 2; rc.right = rc.left;
					rc.top = top + height + 15; rc.bottom = rc.top;
					char fLabel[30];
					strcpy_s(fLabel, 30, "Freq [");
					if (strcmp(tunit, "ms") == 0) strcat_s(fLabel, 30, "kHz]");
					if (strcmp(tunit, "us") == 0) strcat_s(fLabel, 30, "MHz]");
					if (strcmp(tunit, "ns") == 0) strcat_s(fLabel, 30, "GHz]");
					DrawText(dc, fLabel, (int)strlen(fLabel), &rc, DT_NOCLIP | DT_CENTER);
					DeleteObject(fnt);
				}
			}
			if (yLog)
			{
				art = CreatePen(PS_DASH, 1, gridCol);
				SelectObject(dc, art);
				tp[0].x = left; tp[1].x = left + width;
				for (int i = 0; i < 5; i++)
				{
					for (int k = 1; k < 10; k += 2)
					{
						float _y = powf(10.0F, (float)(i - 4))*k;
						tp[0].y = top + height - (LONG)((20.0F*log10f(_y) - _yminlog)*height / (_ymaxlog - _yminlog));
						tp[1].y = tp[0].y;
						Polyline(dc, tp, 2);
					}

				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID, 1, gridCol);
				SelectObject(dc, art);
				for (int i = 0; i < 5; i++)
				{
					for (int k = 10; k < 100; k += 5)
					{
						float _y = powf(10.0F, (float)(i - 4))*0.1F*k;
						tp[0].x = left + width; tp[1].x = left + width - 5;
						tp[0].y = top + height - (LONG)((20.0F*log10f(_y) - _yminlog)*height / (_ymaxlog - _yminlog));
						tp[1].y = tp[0].y;
						Polyline(dc, tp, 2);
						tp[0].x = left;
						tp[1].x = left + 5;
						Polyline(dc, tp, 2);
					}

				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
					SetTextColor(dc, lblCol);
					SelectObject(dc, fnt);
					t = 0;
					y = 0;
					for (int i = 1; i <= 5; i++)
					{
						float _y = powf(10.0F, (float)(i - 4));
						rc.left = left - 5; rc.right = rc.left;
						rc.top = top + height - 5 - (LONG)((20.0F*log10f(_y) - _yminlog)*height / (_ymaxlog - _yminlog));
						rc.bottom = rc.top;
						sprintf_s(valstr, 100, "%1.1g", _y);
						y += (viewVmax - viewVmin) / ygrid;
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_RIGHT);
					}
				}
			}
			else
			{
				art = CreatePen(PS_DASH, 1, gridCol);
				SelectObject(dc, art);
				for (int i = 1; i < ygrid; i++)
				{
					tp[0].x = left; tp[0].y = top + i * height / ygrid;
					tp[1].x = left + width; tp[1].y = top + i * height / ygrid;
					Polyline(dc, tp, 2);
				}
				DeleteObject(art);
				art = CreatePen(PS_SOLID, 1, gridCol);
				SelectObject(dc, art);
				for (int i = 1; i < 40; i++)
				{
					tp[0].x = left + width;   tp[0].y = top + i * height / 40;
					tp[1].x = left + width - 5; tp[1].y = top + i * height / 40;
					Polyline(dc, tp, 2);
					tp[0].x = left;   tp[0].y = top + i * height / 40;
					tp[1].x = left + 5; tp[1].y = top + i * height / 40;
					Polyline(dc, tp, 2);
				}
				DeleteObject(art);
				//if (myUpdate==false)
				{
					fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
					SetTextColor(dc, lblCol);
					SelectObject(dc, fnt);
					t = 0;
					y = 0;
					for (int i = 0; i <= ygrid; i++)
					{
						rc.left = left - 5; rc.right = rc.left;
						rc.top = top + (ygrid - i)*height / ygrid - 5; rc.bottom = rc.top;
						sprintf_s(valstr, 100, "%1.2f", y);
						y += (viewVmax - viewVmin) / ygrid;
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_RIGHT);
					}
				}
			}
		}
		else
		{
			// if not in FFT mode
			art = CreatePen(PS_SOLID, 2, gridCol);
			SelectObject(dc, art);
			tp[0].x = left; tp[0].y = top;
			tp[1].x = left + width; tp[1].y = top;
			tp[2].x = left + width; tp[2].y = top + height;
			tp[3].x = left; tp[3].y = top + height;
			tp[4].x = left; tp[4].y = top;
			Polyline(dc, tp, 5);
			tp[0].x = left; tp[0].y = top + height / 2;
			tp[1].x = left + width; tp[1].y = top + height / 2;
			Polyline(dc, tp, 2);
			tp[0].x = left + width / 2; tp[0].y = top;
			tp[1].x = left + width / 2; tp[1].y = top + height;
			Polyline(dc, tp, 2);
			DeleteObject(art);
			art = CreatePen(PS_DASH, 1, gridCol);
			SelectObject(dc, art);
			for (int i = 1; i < ygrid; i++)
			{
				tp[0].x = left; tp[0].y = top + i * height / ygrid;
				tp[1].x = left + width; tp[1].y = top + i * height / ygrid;
				Polyline(dc, tp, 2);
			}
			for (int i = 1; i < xgrid; i++)
			{
				tp[0].x = left + i * width / xgrid; tp[0].y = top;
				tp[1].x = left + i * width / xgrid; tp[1].y = top + height;
				Polyline(dc, tp, 2);
			}
			DeleteObject(art);
			art = CreatePen(PS_SOLID, 1, gridCol);
			SelectObject(dc, art);
			tp[0].x = left; tp[0].y = top + height / 4;
			tp[1].x = left + width; tp[1].y = top + height / 4;
			Polyline(dc, tp, 2);
			tp[0].x = left; tp[0].y = top + height * 3 / 4;
			tp[1].x = left + width; tp[1].y = top + height * 3 / 4;
			Polyline(dc, tp, 2);
			for (int i = 1; i < 40; i++)
			{
				tp[0].x = left + i * width / 40; tp[0].y = top + height / 2 - 5;
				tp[1].x = left + i * width / 40; tp[1].y = top + height / 2 + 5;
				Polyline(dc, tp, 2);
				tp[0].x = left + width / 2 - 5; tp[0].y = top + i * height / 40;
				tp[1].x = left + width / 2 + 5; tp[1].y = top + i * height / 40;
				Polyline(dc, tp, 2);

				tp[0].x = left + i * width / 40; tp[0].y = top + height / 4 - 2;
				tp[1].x = left + i * width / 40; tp[1].y = top + height / 4 + 2;
				Polyline(dc, tp, 2);
				tp[0].x = left + i * width / 40; tp[0].y = top + height * 3 / 4 - 2;
				tp[1].x = left + i * width / 40; tp[1].y = top + height * 3 / 4 + 2;
				Polyline(dc, tp, 2);
			}
			DeleteObject(art);
			art = CreatePen(PS_DOT, 1, gridCol);
			SelectObject(dc, art);
			tp[0].x = left; tp[0].y = top + height * 3 / 16;
			tp[1].x = left + width; tp[1].y = top + height * 3 / 16;
			Polyline(dc, tp, 2);
			tp[0].x = left; tp[0].y = top + height * 13 / 16;
			tp[1].x = left + width; tp[1].y = top + height * 13 / 16;
			Polyline(dc, tp, 2);
			DeleteObject(art);

			//if (myUpdate==false)
			{
				fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
				SetTextColor(dc, lblCol);
				SelectObject(dc, fnt);
				t = viewTmin;
				y = viewVmin; // + y00;
				for (int i = 0; i <= xgrid; i++)
				{
					rc.left = left + i * width / xgrid; rc.right = rc.left;
					rc.top = top + height + 5; rc.bottom = rc.top;
					sprintf_s(valstr, 100, "%1.3f", t);   // resolution could be made dependent on screen width
					t += (viewTmax - viewTmin) / xgrid;
					DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_CENTER);
				}
				for (int i = 0; i <= ygrid; i++)
				{
					rc.left = left - 5; rc.right = rc.left;
					rc.top = top + (ygrid - i)*height / ygrid - 5; rc.bottom = rc.top;
					if (viewVmax - viewVmin < 0.001)
						sprintf_s(valstr, 100, "%1.4f", y);
					else
						sprintf_s(valstr, 100, "%1.3f", y);
					y += (viewVmax - viewVmin) / ygrid;
					DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_RIGHT);
				}
				// draw time label
				rc.left = left + width / 2; rc.right = rc.left;
				rc.top = top + height + 15; rc.bottom = rc.top;
				DrawText(dc, CString(tlabel), (int)strlen(tlabel), &rc, DT_NOCLIP | DT_CENTER);
				DeleteObject(fnt);
			}
		}
		IntersectClipRect(dc, left, top, left + width, top + height);
		for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
		{
			if ((trace[iTrace].ntrace > 0) && (trace[iTrace].Enable))
			{
				trace[iTrace].Vmax = -1e6;;
				trace[iTrace].Vmin = 1e6;

				int smin = 0;
				if (viewTmin > 0.0) smin = (int)(viewTmin / dt * npoints / xgrid);
				int smax = npoints;
				if (viewTmax < xgrid*dt) smax = (int)(viewTmax / dt * npoints / xgrid);
				//TRACE2("Plot from %i to %i \r\n",smin,smax);
				int y000 = (int)((viewVmax + viewVmin)*height / ygrid / deltay / 2.0);
				double yscale = deltay / (viewVmax - viewVmin)*ygrid;
				int ii = 0, ss = smax - smin, tpho2 = top + height / 2;  // top plus height over 2
				int di = 1;
				if (subSample) { di = 1 + ss / width; }
				for (int i = smin; i < smax; i += di)
				{
					if ((fftMode) && (xLog))
					{
						float _x = (float)i / npoints / xres / 2.0F;
						tp[ii].x = left + (LONG)((20.0F*log10f(_x) - _xminlog)*width / (_xmaxlog - _xminlog));
					}
					else
					{
						tp[ii].x = left + (i - smin)*width / ss;
					}

					if (fftMode)
					{
						if (yLog)
						{
							float _y = (float)trace[iTrace].fdata[i] / 51200.0F;
							tp[ii].y = top + height - (LONG)((20.0F*log10f(_y) - _yminlog)*height / (_ymaxlog - _yminlog));
						}
						else
						{
							tp[ii].y = tpho2 - (LONG)((trace[iTrace].fdata[i] * height / 51200 - y000)*yscale);
							if (tp[ii].y > trace[iTrace].Vmax) trace[iTrace].Vmax = (double)tp[ii].y;
							if (tp[ii].y < trace[iTrace].Vmin) trace[iTrace].Vmin = (double)tp[ii].y;
						}
					}
					else
					{
						tp[ii].y = tpho2 - (LONG)((trace[iTrace].data[i] * height / 51200 - y000)*yscale);
						if (tp[ii].y > trace[iTrace].Vmax) trace[iTrace].Vmax = (double)tp[ii].y;
						if (tp[ii].y < trace[iTrace].Vmin) trace[iTrace].Vmin = (double)tp[ii].y;
					}
					ii++;
				}

				art = CreatePen(PS_SOLID, trace[iTrace].width, trace[iTrace].color);
				SelectObject(dc, art);
				Polyline(dc, tp, ss / di);

				// process min, max and peak-peak voltages
				trace[iTrace].Vmax = (trace[iTrace].Vmax - top) / height * ygrid*deltay - trace[iTrace].y_zero - ygrid / 2 * deltay;
				trace[iTrace].Vmin = (trace[iTrace].Vmin - top) / height * ygrid*deltay - trace[iTrace].y_zero - ygrid / 2 * deltay;
				trace[iTrace].Vpp = trace[iTrace].Vmax - trace[iTrace].Vmin;

				// add label to trace
				fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
				SetTextColor(dc, trace[iTrace].color);
				SelectObject(dc, fnt);
				rc.left = left + 20; rc.right = rc.left;
				rc.top = tp[0].y - 5; rc.bottom = rc.top;
				DrawText(dc, trace[iTrace].trace_id, (int)strlen(trace[iTrace].trace_id), &rc, DT_NOCLIP | DT_LEFT);
				DeleteObject(fnt);
				DeleteObject(art);
			}
		}
		SelectClipRgn(dc, NULL);

		for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
		{
			if ((trace[iTrace].ntrace > 0))
			{
				art = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
				SelectObject(dc, art);
				if (showInfo)
				{

					rc.left = left + width + 15; rc.right = rc.left + (LONG)(infowidth);
					rc.top = infoy; rc.bottom = rc.top + 93 * yline / 15;
					fil = CreateSolidBrush(RGB(255, 255, 245));
					SelectObject(dc, fil);
					//dc->FillRect(&rc,&fil);
					Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
					DeleteObject(fil);

					fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
					SetTextColor(dc, trace[iTrace].color);
					SelectObject(dc, fnt);
					infoy += 3;

					rc.left = left + width + 18; rc.right = rc.left;
					rc.top = infoy; rc.bottom = rc.top;
					if (iTrace < 4)
					{
						sprintf_s(valstr, 100, "Trace info [%s]:", trace[iTrace].trace_id);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy; rc.bottom = rc.top;
						//sprintf_s(valstr,100,"More information to be displayed here.");
						sprintf_s(valstr, 100, "Date: %s", trace[iTrace].tdate);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy; rc.bottom = rc.top;
						sprintf_s(valstr, 100, "Delta_t=%1.1f %1.1f s", trace[iTrace].delta_t, trace[iTrace].x_res);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy; rc.bottom = rc.top;
						sprintf_s(valstr, 100, "Y0=%1.1f %s", trace[iTrace].y_zero, trace[iTrace].y_unit);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy; rc.bottom = rc.top;
						sprintf_s(valstr, 100, "Acq: %i samples", npoints);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy; rc.bottom = rc.top;

						sprintf_s(valstr, 100, "Vmax=%1.2f Vmin=%1.2f  Vpp=%1.2f", trace[iTrace].Vmax, trace[iTrace].Vmin, trace[iTrace].Vpp);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += (int)(yline*1.4);
					}
					else
					{
						int traceGroup = iTrace / 4;
						int traceRegister = iTrace - 4 * traceGroup + 1;
						sprintf_s(valstr, 100, "Memory info [%i.%i]:", traceGroup, traceRegister);
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy; rc.bottom = rc.top;
						sprintf_s(valstr, 100, "More information to be displayed here.");
						DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
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
			art = CreatePen(PS_DASHDOT, Cursor[2].width, Cursor[2].color);
			SelectObject(dc, art);
			tp[0].x = Cursor[2].pos; tp[0].y = top;
			tp[1].x = Cursor[2].pos; tp[1].y = top + height;
			Polyline(dc, tp, 2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID, 1, Cursor[2].color);
			SelectObject(dc, art);
			fil = CreateSolidBrush(Cursor[2].color);
			SelectObject(dc, fil);
			tp[0].y = top + CURSOR_TSIZE;
			tp[1].x = Cursor[2].pos - CURSOR_TSIZE; tp[1].y = top;
			tp[2].x = Cursor[2].pos + CURSOR_TSIZE; tp[2].y = top;
			Polygon(dc, tp, 3);
			tp[0].y = top + height - CURSOR_TSIZE;
			tp[1].y = top + height;
			tp[2].y = top + height;
			Polygon(dc, tp, 3);
			DeleteObject(fil);
			DeleteObject(art);

			art = CreatePen(PS_DOT, Cursor[3].width, Cursor[3].color);
			SelectObject(dc, art);
			tp[0].x = Cursor[3].pos; tp[0].y = top;
			tp[1].x = Cursor[3].pos; tp[1].y = top + height;
			Polyline(dc, tp, 2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID, 1, Cursor[3].color);
			SelectObject(dc, art);
			fil = CreateSolidBrush(Cursor[3].color);
			SelectObject(dc, fil);
			tp[0].y = top + CURSOR_TSIZE;
			tp[1].x = Cursor[3].pos - CURSOR_TSIZE; tp[1].y = top;
			tp[2].x = Cursor[3].pos + CURSOR_TSIZE; tp[2].y = top;
			Polygon(dc, tp, 3);
			tp[0].y = top + height - CURSOR_TSIZE;
			tp[1].y = top + height;
			tp[2].y = top + height;
			Polygon(dc, tp, 3);
			DeleteObject(art);
			DeleteObject(fil);
		}
		if (cursorFcn & 2)
		{
			art = CreatePen(PS_DASHDOT, Cursor[0].width, Cursor[0].color);
			SelectObject(dc, art);
			tp[0].x = left; tp[0].y = Cursor[0].pos;
			tp[1].x = left + width; tp[1].y = Cursor[0].pos;
			Polyline(dc, tp, 2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID, 1, Cursor[0].color);
			SelectObject(dc, art);
			fil = CreateSolidBrush(Cursor[0].color);
			SelectObject(dc, fil);
			tp[0].x = left + CURSOR_TSIZE;
			tp[1].x = left; tp[1].y = Cursor[0].pos - CURSOR_TSIZE;
			tp[2].x = left; tp[2].y = Cursor[0].pos + CURSOR_TSIZE;
			Polygon(dc, tp, 3);
			tp[0].x = left + width - CURSOR_TSIZE;
			tp[1].x = left + width;
			tp[2].x = left + width;
			Polygon(dc, tp, 3);
			DeleteObject(fil);
			DeleteObject(art);

			art = CreatePen(PS_DOT, Cursor[1].width, Cursor[1].color);
			SelectObject(dc, art);
			tp[0].x = left; tp[0].y = Cursor[1].pos;
			tp[1].x = left + width; tp[1].y = Cursor[1].pos;
			Polyline(dc, tp, 2);
			DeleteObject(art);

			art = CreatePen(PS_SOLID, 1, Cursor[1].color);
			SelectObject(dc, art);
			fil = CreateSolidBrush(Cursor[1].color);
			SelectObject(dc, fil);
			tp[0].x = left + CURSOR_TSIZE;
			tp[1].x = left; tp[1].y = Cursor[1].pos - CURSOR_TSIZE;
			tp[2].x = left; tp[2].y = Cursor[1].pos + CURSOR_TSIZE;
			Polygon(dc, tp, 3);
			tp[0].x = left + width - CURSOR_TSIZE;
			tp[1].x = left + width;
			tp[2].x = left + width;
			Polygon(dc, tp, 3);
			DeleteObject(fil);
			DeleteObject(art);
		}

		if (cursorFcn > 0)
		{
			if (showInfo)
			{
				art = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				SelectObject(dc, art);

				rc.left = left + width + 15; rc.right = rc.left + (LONG)(infowidth);
				rc.top = infoy; rc.bottom = rc.top + 80 * yline / 15;
				fil = CreateSolidBrush(RGB(255, 255, 255));
				SelectObject(dc, fil);

				Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
				DeleteObject(fil);

				fnt = CreateFont(fnth, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, _T("Tremuchet MS"));
				SetTextColor(dc, lblCol);
				SelectObject(dc, fnt);

				// tp[i].y = top + height/2 - trace[iTrace].data[i]*height/51200;
				// y = -4*deltay + y00  where deltay is the vertical scale / 8
				// 8 * dt is x-axis
				infoy += 3;
				rc.left = left + width + 18; rc.right = rc.left;
				rc.top = infoy; rc.bottom = rc.top;
				DrawText(dc, _T("Cursor info:"), 12, &rc, DT_NOCLIP | DT_LEFT);
				infoy += yline;

				rc.top = infoy; rc.bottom = rc.top;
				sprintf_s(valstr, 100, "X1 = %3.2f - Y1 = %3.2f\r\n", Cursor[2].value, Cursor[0].value);
				DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
				infoy += yline;

				rc.top = infoy; rc.bottom = rc.top;
				sprintf_s(valstr, 100, "X2 = %3.2f - Y2 = %3.2f", Cursor[3].value, Cursor[1].value);
				DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
				infoy += yline;

				double dt = Cursor[3].value - Cursor[2].value;
				double f = 1.001 / dt;
				rc.top = infoy; rc.bottom = rc.top; rc.left += 10;
				char funit[10];
				if (strcmp(tunit, "ms") == 0) strcpy_s(funit, 10, "kHz");
				else if (strcmp(tunit, "us") == 0) strcpy_s(funit, 10, "MHz");
				else if (strcmp(tunit, "s") == 0) strcpy_s(funit, 10, "Hz");
				else if (strcmp(tunit, "ns") == 0) strcpy_s(funit, 10, "GHz");
				else strcpy_s(funit, 10, "-");
				sprintf_s(valstr, 100, "T = %3.3f %s  Freq = %3.2f %s", dt, tunit, f, funit);
				CString csvalstr = CString(valstr);
				DrawText(dc, csvalstr, (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);
				infoy += yline;
				rc.left -= 10;
				POINT pnt[4];
				pnt[0].x = rc.left + 1; pnt[0].y = rc.top + 9;
				pnt[1].x = rc.left + 4; pnt[1].y = rc.top + 2;
				pnt[2].x = rc.left + 7; pnt[2].y = rc.top + 9;
				pnt[3].x = rc.left + 1; pnt[3].y = rc.top + 9;
				Polyline(dc, (POINT*)&pnt, 4);

				double dy = Cursor[1].value - Cursor[0].value;
				rc.top = infoy; rc.bottom = rc.top; rc.left += 10;
				sprintf_s(valstr, 100, "V = %3.3f   Vavg = %3.2f",
					Cursor[1].value - Cursor[0].value,
					(Cursor[0].value + Cursor[1].value) / 2.0);
				DrawText(dc, CString(valstr), (int)strlen(valstr), &rc, DT_NOCLIP | DT_LEFT);


				infoy += yline;
				rc.left -= 10;
				pnt[0].x = rc.left + 1; pnt[0].y = rc.top + 9;
				pnt[1].x = rc.left + 4; pnt[1].y = rc.top + 2;
				pnt[2].x = rc.left + 7; pnt[2].y = rc.top + 9;
				pnt[3].x = rc.left + 1; pnt[3].y = rc.top + 9;
				Polyline(dc, (POINT*)&pnt, 4);


				DeleteObject(fnt);
				DeleteObject(art);
			}
		}
		if ((zooming) && (zoomA.x > 0))
		{
			art = CreatePen(PS_DASH, 1, RGB(255, 0, 0));
			SelectObject(dc, art);
			MoveToEx(dc, zoomA.x, zoomA.y, NULL);
			LineTo(dc, zoomA.x, zoomB.y);
			LineTo(dc, zoomB.x, zoomB.y);
			LineTo(dc, zoomB.x, zoomA.y);
			LineTo(dc, zoomA.x, zoomA.y);
			DeleteObject(art);
		}
	}

#if _USEDOUBLEBUFFER_
	if (hBmp == NULL)
	{
		left = cwnd.left;
		top = cwnd.top;
		BitBlt(dc0, left, top, dx, dy, dc, 0, 0, SRCCOPY);
		DeleteObject(dcbmp);
		DeleteDC(dc);
	}
#endif
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	Draw(dc, 2);
	myUpdate = false;

	if (!HaveView)
		HaveView = TRUE;
	// Do not call CWnd::OnPaint() for painting messages
}

BOOL CChildView::InitCommPort(int CommPortNum, int baud,
	BYTE ByteSize, BYTE Parity, BYTE StopBits)
{
	// defaults to COM1, BAUD192, BYTESIZE8, NOPARITY, ONESTOPBIT
	//sprintf_s(CommPort,12,"COM%i",CommPortNum);
	sprintf_s(CommPort, 50, "\\\\.\\COM%i", CommPortNum);
	hCommPort = CreateFile(CString(CommPort), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
	CommTimeouts.ReadTotalTimeoutConstant = 1000;
	CommTimeouts.WriteTotalTimeoutMultiplier = 1;
	CommTimeouts.WriteTotalTimeoutConstant = 1;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
	if (!fSuccess) {
		//MessageBox(NULL, "Couldn't set comm timeouts.", "Error",MB_OK + MB_ICONSTOP);
		return FALSE;
	}
	return TRUE;
}

void CChildView::Connect(int port, int baud)
{
	char iddata[1000];
	unsigned long nc;

	if (hCommPort != INVALID_HANDLE_VALUE)
	{
		// close com port first, before attempting to re-connect
		CloseHandle(hCommPort);
		hCommPort = INVALID_HANDLE_VALUE;
		strcpy_s(pmid, 200, "--");
		strcpy_s(CommPort, 50, "");
	}
	else
	{

		if (InitCommPort(port, baud, 8, 0, 1)) {
			FlushFileBuffers(hCommPort);
			memset(iddata, 0, 1000);
			WriteFile(hCommPort, "ID\r", 3, &nc, NULL);
			Sleep(500);
			ReadFile(hCommPort, iddata, 1, &nc, NULL);
			if (nc > 0)
			{
				ReadFile(hCommPort, pmid, 200, &nc, NULL);
				if (pmid[0] == 13) pmid[0] = 32;
				for (int i = 0; i < 200; i++) if (pmid[i] == 13) pmid[i] = 0;
				TRACE1("Instrument ID: %s\r\n", &pmid[1]);
			}
			else {
				CloseHandle(hCommPort);
				hCommPort = INVALID_HANDLE_VALUE;
			}
		}
	}
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateStatus();
}

void CChildView::OnConnect()
{
	Connect(ComData[SelectedComport], atoi(BaudRates[SelectedBaudrate]));
}


void CChildView::OnHardcopy()
{
	char hpgldata[100000], ccmd[200];
	unsigned long nc, fnc, s;
	RECT rc;
	CImage img;
	DWORD rtn = 0;
	BOOL fSuccess;

	//trace[0].ntrace = 0;
	if (hCommPort != INVALID_HANDLE_VALUE)
	{

		CommTimeouts.ReadTotalTimeoutConstant = 5000;
		fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
		if (!fSuccess) { return; }

		// Request HPGL hardcopy data
		// Note: should make sure that PM3394 sends in HPGL format.
		FlushFileBuffers(hCommPort);
		WriteFile(hCommPort, _T("QP1\r"), 4, &nc, NULL);
		Sleep(1000);


		RECT sr;
		CDC *hsDC = GetDC();
		HFONT hfnt = CreateFont(18, 0, 0, 0, FW_DONTCARE, 0, 0, 0, 0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 18, "Calibri");
		SelectObject((HDC)hsDC,hfnt);
		sr.left = 0; sr.right = 150; sr.top = 2; sr.bottom = 21;
		int blocksize = 250;
		int di = 0;
		char sstr[20];
		ReadFile(hCommPort, &hpgldata[di], blocksize, &nc, NULL);
		di += nc;
		//while (nc == blocksize)
		while (nc > 0)
		{
			rtn = ReadFile(hCommPort, &hpgldata[di], blocksize, &nc, NULL);
			di += nc;
			sprintf_s(sstr, 20, "Reading %i %%\r\n", di / 200);
			DrawStatusText((HDC)hsDC,&sr, sstr, 0);
			TRACE1("Read = %i\r\n", di);
		}
		DeleteObject(hfnt);

		CommTimeouts.ReadTotalTimeoutConstant = 500;
		fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);

		// Store HPGL data to file
		HANDLE hFile = CreateFile(_T("pm33xx.hgl"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		else
		{
			WriteFile(hFile, hpgldata, di, &fnc, NULL);
			TRACE1("Wrote = %i\r\n", di);
			CloseHandle(hFile);
		}
	}
#ifndef _DEBUG
	if (hCommPort != INVALID_HANDLE_VALUE)
#endif
	{
		GetWindowRect(&rc);
		// Convert HPGL file to PNG. Uses external command hp2xx.exe
		s = (rc.right - rc.left) / 2;
		sprintf_s(ccmd, 200, "-m png -c 1234567 -r270 -h%i -w%i pm33xx.hgl", s, s);
		TRACE1(">hp2xx.exe %s\r\n", ccmd);
		SHELLEXECUTEINFO sexi = { 0 };
		sexi.cbSize = sizeof(SHELLEXECUTEINFO);
		sexi.hwnd = m_hWnd;
		sexi.fMask = SEE_MASK_NOCLOSEPROCESS;
		sexi.lpFile = _T("HPGL\\hp2xx.exe");
		sexi.lpParameters = ccmd;
		sexi.nShow = SW_HIDE;
		//sexi.nShow = SW_SHOW;
		if (ShellExecuteEx(&sexi))
		{
			DWORD wait = WaitForSingleObject(sexi.hProcess, INFINITE);
			if (wait == WAIT_OBJECT_0) GetExitCodeProcess(sexi.hProcess, &rtn);
		}

		// load PNG bitmap file
		if (img.Load(_T("pm33xx.png")) == NULL)
			hBmp = img.Detach();

		// Display bitmap in Dialog Window
		Invalidate(1);
		UpdateWindow();
	}
}


void CChildView::OnPort()
{
	// TODO: Add your command handler code here
}


void CChildView::OnBaud()
{
	// TODO: Add your command handler code here
}


void CChildView::OnCopy()
{
	SendMessage(WM_LBUTTONDBLCLK, 0, 0);  // send double-click message
}


void CChildView::OnConfig()
{
	// TODO: Add your command handler code here
}


void CChildView::OnGettraces()
{
	if (hCommPort != INVALID_HANDLE_VALUE)
	{
		if (hBmp)
		{
			DeleteObject(hBmp);
			hBmp = NULL;
		}
		for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
			if (trace[iTrace].Enable)
				GetTrace(iTrace);
		Invalidate(1);
		UpdateWindow();
	}
}


void CChildView::OnRun()
{
	if (hCommPort != INVALID_HANDLE_VALUE)
	{
		unsigned long nc;
		WriteFile(hCommPort, "AT\r", 3, &nc, NULL);
	}
}

void CChildView::GetTrace(int iTrace)
{
	unsigned char tdata[100000];
	unsigned long nc;
	unsigned int blocksize = 250;
	BOOL fSuccess;
	int j, i = 0, di;
	short *sdata;
	char cmd[10];
	int rtn;
	int traceGroup = iTrace / 4;
	int traceRegister = iTrace - 4 * traceGroup + 1;

	CommTimeouts.ReadIntervalTimeout = 1500;
	CommTimeouts.ReadTotalTimeoutConstant = 10000;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
	if (!fSuccess) { return; }


	if (iTrace < 4)
		sprintf_s(cmd, 10, "QW%i\r", traceRegister);
	else
		sprintf_s(cmd, 10, "QW%i%i\r", traceGroup, traceRegister);
	PurgeComm(hCommPort,PURGE_RXCLEAR | PURGE_TXCLEAR);
	//FlushFileBuffers(hCommPort);
	WriteFile(hCommPort, cmd, (int)strlen(cmd), &nc, NULL);
	//Sleep(100);
	rtn = ReadFile(hCommPort, tdata, 1, &nc, NULL);
	if (nc == 1) if (tdata[0] == 13)
		rtn = ReadFile(hCommPort, tdata, 1, &nc, NULL);

	if ((tdata[0] != '0') || (nc == 0))
	{
		TRACE1("No response. Channel %i disabled.\r\n", iTrace + 1);
		trace[iTrace].Enable = false;
		return; // error
	}
	di = 0;
	rtn = ReadFile(hCommPort, &tdata[di], blocksize, &nc, NULL);
	di += nc;

	// interpret header data
	i++;
	j = 0; while (tdata[i] != ',') trace[iTrace].trace_id[j++] = tdata[i++]; trace[iTrace].trace_id[j] = 0; i++;
	j = 0; while (tdata[i] != ',') trace[iTrace].y_unit[j++] = tdata[i++]; trace[iTrace].y_unit[j] = 0; i++;
	j = 0; while (tdata[i] != ',') trace[iTrace].x_unit[j++] = tdata[i++]; trace[iTrace].x_unit[j] = 0; i++;
	sscanf_s((char*)&tdata[i], "%f", &trace[iTrace].y_zero); while (tdata[i++] != ',') {};
	sscanf_s((char*)&tdata[i], "%f", &trace[iTrace].x_zero); while (tdata[i++] != ',') {};
	sscanf_s((char*)&tdata[i], "%f", &trace[iTrace].y_res); while (tdata[i++] != ',') {};
	sscanf_s((char*)&tdata[i], "%f", &trace[iTrace].x_res); while (tdata[i++] != ',') {};
	sscanf_s((char*)&tdata[i], "%i", &trace[iTrace].y_range); while (tdata[i++] != ',') {};
	j = 0; while (tdata[i] != ',') trace[iTrace].tdate[j++] = tdata[i++]; trace[iTrace].tdate[j] = 0; i++;
	j = 0; while (tdata[i] != ',') trace[iTrace].ttime[j++] = tdata[i++]; trace[iTrace].ttime[j] = 0; i++;
	sscanf_s((char*)&tdata[i], "%f", &trace[iTrace].delta_t); while (tdata[i++] != ',') {};
	while (tdata[i++] != ',') {};
	while (tdata[i++] != ',') {};
	while (tdata[i++] != ',') {};
	while (tdata[i++] != ',') {};
	while (tdata[i++] != ',') {};
	sscanf_s((char*)&tdata[i], "%i", &trace[iTrace].ntrace);
	while (tdata[i++] != ',') {};

	TRACE1("TRACE INFO: %s\r\n", trace[iTrace].trace_id);
	TRACE1("Y_0: %f\r\n", trace[iTrace].y_zero);
	TRACE1("X_0: %f\r\n", trace[iTrace].x_zero);
	TRACE1("Y_res: %f\r\n", trace[iTrace].y_res);
	TRACE1("X_res: %f\r\n", trace[iTrace].x_res);
	TRACE1("Y_rng: %f\r\n", trace[iTrace].y_range);
	TRACE1("D_t: %f\r\n", trace[iTrace].delta_t);
	TRACE1("Samples: %i\r\n", trace[iTrace].ntrace);

	TRACE1("Header size = %i bytes\r\n", j);

	while (nc == blocksize)
	{
		rtn = ReadFile(hCommPort, &tdata[di], blocksize, &nc, NULL);
		di += nc;

		progress = 50 * di / trace[iTrace].ntrace;
		//sprintf_s(sstr, 20, "Reading %i %%\r\n", );
		//DrawStatusText(hsDC, &sr, sstr, 0);
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateStatus();
	}

	CommTimeouts.ReadIntervalTimeout = 500;
	CommTimeouts.ReadTotalTimeoutConstant = 1000;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);

#ifdef _DEBUG
	FILE *fid;
	char fn[500];
	GetEnvironmentVariable("TEMP", fn, 500);
	strcat_s(fn, 500, "pm3394_data.bin");
	fopen_s(&fid, fn, "w");
	fwrite(tdata, 1, nc, fid);
	fclose(fid);
#endif

	sdata = (short*)&tdata[i - 1];
	if (trace[iTrace].ntrace > 32768)
	{
		trace[iTrace].ntrace = 0;
	}
	else {
		// Set data pointers for all channels and memory, since this is a global allocation in the Scope
		if (npoints != trace[iTrace].ntrace)
		{
			npoints = trace[iTrace].ntrace;
			xres = trace[iTrace].x_res;
			for (i = 0; i < MAX_TRACE; i++)
				if (i != iTrace)
					trace[i].ntrace = 0;
		}
		SetTraceData(npoints);
		for (j = 0; j < npoints; j++)
			trace[iTrace].data[j] = sdata[j];
	}
	dt = trace[iTrace].x_res*npoints / 10.24;
	strcpy_s(tlabel, 20, "Time [s]");
	if (trace[iTrace].x_res < 0.01) { dt *= 1000.0F; strcpy_s(tlabel, 20, "Time [ms]"); }
	if (trace[iTrace].x_res < 1e-6) { dt *= 1000.0F; strcpy_s(tlabel, 20, "Time [us]"); }
	y00 = trace[iTrace].y_zero;
	deltay = trace[iTrace].y_res * 6400;
	TRACE2("dt = %1.3f %s\r\n", dt, tlabel);
	TRACE1("deltay = %1.3f \r\n", deltay);
	viewVmax = ygrid / 2.0*deltay;
	viewVmin = ygrid / -2.0*deltay;
	viewTmin = 0.0;
	viewTmax = xgrid * dt;
	fftMode = false;
	progress = 0;
	// update fft button
}






void CChildView::OnColor()
{
	DWORD color = ((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetSelectedColor();
	if (it >= 0)
	{
		char tcr[50];
		trace[it].color = (COLORREF)color;
		sprintf_s(tcr, 50, "TC%i", it + 1);
		//SetReg("", tcr, (char*)&trace[it].color, 4, REG_DWORD);
	}
	if (ic == 4)
	{
		bgCol = (COLORREF)color;
		//SetReg("", "BC", (char*)&bgCol, 4, REG_DWORD);
	}
	else if (ic >= 0)
	{
		char tcr[50];
		Cursor[ic].color = (COLORREF)color;
		sprintf_s(tcr, 50, "TC%i", ic + 37);
		//SetReg("", tcr, (char*)&Cursor[ic].color, 4, REG_DWORD);
	}
	myUpdate = true;
	Invalidate(1);
	UpdateWindow();
}


void CChildView::OnFftwindow()
{
	// TODO: Add your command handler code here
}


void CChildView::OnC0()
{
	cursorFcn = 0;
	Invalidate(1);
	UpdateWindow();
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateButtonImg(ID_CURSORS, 10);
}


void CChildView::OnCx()
{
	cursorFcn = 1;
	Invalidate(1);
	UpdateWindow();
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateButtonImg(ID_CURSORS, 11);
}


void CChildView::OnCxy()
{
	cursorFcn = 3;
	Invalidate(1);
	UpdateWindow();
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateButtonImg(ID_CURSORS, 12);
}


void CChildView::OnCy()
{
	cursorFcn = 2;
	Invalidate(1);
	UpdateWindow();
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateButtonImg(ID_CURSORS, 13);
}


void CChildView::OnCursors()
{
	
}

void CChildView::OpenFile()
{
	FILE *storeFile;
	size_t ntr, ncd, nmd;
	if (strchr((char*)lfn.lpstrFile, '.') == NULL)
		strcat_s((char *)lfn.lpstrFile, MAX_FILENAME, ".trc");
	int nResult = fopen_s(&storeFile, (const char*)lfn.lpstrFile, (const char*)"rb");  // write;binary;commit
	if (nResult)
	{
		TRACE0("Trace File Open problem.\r\n");
		return;
	}
	ntr = fread(trace, sizeof(TRACE_T), MAX_TRACE, storeFile);
	ncd = fread(cdata, sizeof(int), 35000, storeFile);
	nmd = fread(mdata, sizeof(int), 65000, storeFile);
	TRACE3("Read %i, %i and %i bytes\r\n", ntr, ncd, nmd);
	RegisterMru(lfn.lpstrFile);
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateStatus();
	fclose(storeFile);

	bool foundfirsttrace = false;
	for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
	{
		if (trace[iTrace].ntrace > 0)
		{
			if (!foundfirsttrace)
			{
				npoints = trace[iTrace].ntrace;
				xres = trace[iTrace].x_res;
				dt = trace[iTrace].x_res*npoints / 10.24;
				strcpy_s(tunit, 20, "s");
				if (trace[iTrace].x_res < 0.01) { dt *= 1000.0F; strcpy_s(tunit, 20, "ms"); }
				if (trace[iTrace].x_res < 1e-6) { dt *= 1000.0F; strcpy_s(tunit, 20, "us"); }
				strcpy_s(tlabel, 20, "Time ["); strcat_s(tlabel, 20, tunit); strcat_s(tlabel, 20, "]");
				y00 = trace[iTrace].y_zero;
				deltay = trace[iTrace].y_res * 6400 * 0.5;
				foundfirsttrace = true;
				viewVmax = ygrid / 2.0*deltay;
				viewVmin = ygrid / -2.0*deltay;
				viewTmin = 0.0;
				viewTmax = xgrid * dt;
				if (trace[iTrace].ntrace == 255)
				{
					trace[iTrace].ntrace = 512;
					TRACE("Warning!! npnts = 255\r\n");
				}
				SetTraceData(npoints);
				it = iTrace;
				fftMode = false;
				//g_pFramework->InvalidateUICommand(IDM_SELECTCOLOR, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);
				//g_pFramework->InvalidateUICommand(IDM_FFT, UI_INVALIDATIONS_VALUE, &UI_PKEY_BooleanValue);

				TRACE1("TRACE INFO: %s\r\n", trace[iTrace].trace_id);
				TRACE1("Y_0: %f\r\n", trace[iTrace].y_zero);
				TRACE1("X_0: %f\r\n", trace[iTrace].x_zero);
				TRACE1("Y_res: %f\r\n", trace[iTrace].y_res);
				TRACE1("X_res: %f\r\n", trace[iTrace].x_res);
				TRACE1("Y_rng: %f\r\n", trace[iTrace].y_range);
				TRACE1("D_t: %f\r\n", trace[iTrace].delta_t);
				TRACE1("Samples: %i\r\n", npoints);
			}
		}
		else
		{
		}
	}
	hBmp = NULL;
	ResetZoom();
	Invalidate(1);
	UpdateWindow();
}



void CChildView::OnFileOpen()
{
	CString fname;
	COMDLG_FILTERSPEC rgSpec[] =
	{
		{ L"Trace Files", L"*.trc;*.trace" },
		{ L"Bitmap Files", L"*.bmp" },
		{ L"All Files", L"*.*" },
	};
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{

			IKnownFolderManager *pkfm = NULL;
			hr = CoCreateInstance(CLSID_KnownFolderManager,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&pkfm));
			if (SUCCEEDED(hr))
			{
				// Get the known folder.
				IKnownFolder *pKnownFolder = NULL;
				hr = pkfm->GetFolder(FOLDERID_UsersFiles, &pKnownFolder);
				if (SUCCEEDED(hr))
				{
					// File Dialog APIs need an IShellItem that represents the location.
					IShellItem *psi = NULL;
					hr = pKnownFolder->GetShellItem(0, IID_PPV_ARGS(&psi));
					if (SUCCEEDED(hr))
					{
						// Add the place to the bottom of default list in Common File Dialog.
						hr = pFileOpen->AddPlace(psi, FDAP_BOTTOM);
						if (SUCCEEDED(hr))
						{
							// Set the file types to display only. 
						// Notice that this is a 1-based array.
							hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
							if (SUCCEEDED(hr))
							{
								// Set the selected file type index to Word Docs for this example.
								hr = pFileOpen->SetFileTypeIndex(0);
								if (SUCCEEDED(hr))
								{
									// Set the default extension to be ".doc" file.
									hr = pFileOpen->SetDefaultExtension(L"trc;trace");



									// Show the Open dialog box.
									hr = pFileOpen->Show(NULL);

									// Get the file name from the dialog box.
									if (SUCCEEDED(hr))
									{
										IShellItem *pItem;
										hr = pFileOpen->GetResult(&pItem);
										if (SUCCEEDED(hr))
										{
											PWSTR pszFilePath;
											hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

											// Display the file name to the user.
											if (SUCCEEDED(hr))
											{
												//WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, pszFilePath, sizeof(pszFilePath), lfn.lpstrTitle, MAX_FILENAME, NULL, NULL);
												fname = pszFilePath;
												//MessageBox(pszFilePath, "File Path", MB_OK);
												strcpy_s(lfn.lpstrFile, MAX_FILENAME, fname.GetBuffer());
												fname.ReleaseBuffer();
												OpenFile();
												AfxGetApp()->AddToRecentFileList(fname);

												CoTaskMemFree(pszFilePath);
											}
											pItem->Release();
										}
									}
								}
							}
							pFileOpen->Release();
						}
						psi->Release();
					}
					pKnownFolder->Release();
				}
				pkfm->Release();
			}
			pFileOpen->Release();

		}
		CoUninitialize();
	}
}

void CChildView::ResetZoom()
{
	if (zoomMode)
	{
	}
	else {
		zooming = false;  // reset zoom
		if (fftMode)
		{
			viewVmax = ygrid * deltay / 2.0F;
			viewVmin = 0.0F;
		}
		else
		{
			viewVmax = ygrid / 2.0*deltay;
			viewVmin = ygrid / -2.0*deltay;
		}
		viewTmin = 0.0;
		viewTmax = xgrid * dt;
		myUpdate = false;
		Invalidate(1);
		UpdateWindow();
	}
}

void CChildView::SetTraceData(int n)
{
	int it, itt;

	switch (n)
	{
	case 512:
		for (it = 0; it < 4; it++) trace[it].fdata = &fdata[512 * it];
		for (it = 4; it < MAX_TRACE; it++) trace[it].data = &mdata[512 * (it - 4)];
		break;
	case 8192:
		for (it = 0; it < 4; it++) trace[it].fdata = &fdata[8192 * it];
		for (it = 4; it < MAX_TRACE; it += 8)
			for (int itt = 0; itt < 8; itt++) trace[it + itt].data = &mdata[itt * 8192];
		break;
	case 16384:
		for (it = 0; it < 4; it += 2) trace[it].fdata = fdata;
		for (it = 1; it < 4; it += 2) trace[it].fdata = &fdata[16384];
		for (it = 4; it < MAX_TRACE; it += 4)
			for (itt = 0; itt < 4; itt++) trace[it + itt].data = &mdata[itt * 16384];
		break;
	case 32768:
		for (it = 0; it < 4; it++) trace[it].fdata = fdata;
		for (it = 4; it < MAX_TRACE; it += 2) trace[it].data = mdata;
		for (it = 5; it < MAX_TRACE; it += 2) trace[it].data = &mdata[32768];
		break;
	}

	switch (n)
	{
	case 512:
		for (it = 0; it < 4; it++) trace[it].data = &cdata[512 * it];
		for (it = 4; it < MAX_TRACE; it++) trace[it].data = &mdata[512 * (it - 4)];
		break;
	case 8192:
		for (it = 0; it < 4; it++) trace[it].data = &cdata[8192 * it];
		for (it = 4; it < MAX_TRACE; it += 8)
			for (int itt = 0; itt < 8; itt++) trace[it + itt].data = &mdata[itt * 8192];
		break;
	case 16384:
		for (it = 0; it < 4; it += 2) trace[it].data = cdata;
		for (it = 1; it < 4; it += 2) trace[it].data = &cdata[16384];
		for (it = 4; it < MAX_TRACE; it += 4)
			for (itt = 0; itt < 4; itt++) trace[it + itt].data = &mdata[itt * 16384];
		break;
	case 32768:
		for (it = 0; it < 4; it++) trace[it].data = cdata;
		for (it = 4; it < MAX_TRACE; it += 2) trace[it].data = mdata;
		for (it = 5; it < MAX_TRACE; it += 2) trace[it].data = &mdata[32768];
		break;
	}

}


void CChildView::OnZoom()
{
	zoomMode = !zoomMode;
}


void CChildView::OnInfo()
{
	showInfo = !showInfo;
	Invalidate(1);
	UpdateWindow();
}


void CChildView::OnXLog()
{
	xLog = !xLog;
	Invalidate(1);
	UpdateWindow();
}


void CChildView::OnYLog()
{
	yLog = !yLog;
	Invalidate(1);
	UpdateWindow();
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	mpoint = point;
	mpoint.x -= 29;
	mpoint.y -= 20;

	// If cursor movement, then set the active cursor to the current mouse location
	if (zooming)
	{
		zoomB = mpoint;
		myUpdate = true;
		Invalidate(0);
		UpdateWindow();
	}
	else {
		if (cursorMoving)
		{
			if (ic < 2) Cursor[ic].pos = mpoint.y; else Cursor[ic].pos = mpoint.x;
			getCursorValue();
			myUpdate = true;
			Invalidate(0);
			UpdateWindow();
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}


BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	RECT rc;
	GetWindowRect(&rc);
	pt.x -= left + rc.left + 40;
	pt.y += top - rc.top - 46;

	tviewTmin = viewTmin + (double)(viewTmax - viewTmin) / width * pt.x * zDelta * 0.0005;
	tviewTmax = viewTmax - (double)(viewTmax - viewTmin) / width * (width - pt.x) * zDelta * 0.0005;
	tviewVmin = viewVmin + (double)(viewVmax - viewVmin) / height * (height - pt.y) * zDelta * 0.0005;
	tviewVmax = viewVmax - (double)(viewVmax - viewVmin) / height * pt.y * zDelta * 0.0005;

	if (tviewTmin < 0.0) tviewTmin = 0.0;
	if (tviewTmax > xgrid * dt) tviewTmax = xgrid * dt;
	if (tviewVmin < ygrid / -2.0*deltay) tviewVmin = ygrid / -2.0*deltay;
	if (tviewVmax > ygrid / 2.0*deltay) tviewVmax = ygrid / 2.0*deltay;

	viewTmin = tviewTmin;
	viewTmax = tviewTmax;
	viewVmin = tviewVmin;
	viewVmax = tviewVmax;
	zoomA.x = 0; zoomA.y = 0;
	zoomB.x = 0; zoomB.y = 0;

	myUpdate = false;
	Invalidate(1);
	UpdateWindow();

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint tp;
	int e = 10;
	if (zoomMode)
	{
		zooming = true;
		zoomA = point;
		zoomB = point;
		tviewTmin = viewTmin + (viewTmax - viewTmin) / (width)*(point.x - left);
		tviewTmax = tviewTmin;
		tviewVmin = viewVmin + (viewVmax - viewVmin) / height * (point.y - top);
		tviewVmax = tviewVmin;
	}
	else {
		ic = -1;
		it = -1;
		cursorMoving = false;
		if (cursorFcn & 1)
		{
			if (abs(point.x - Cursor[2].pos - 29) < e) { e = abs(point.x - Cursor[2].pos); ic = 2; }
			if (abs(point.x - Cursor[3].pos - 29) < e) { e = abs(point.x - Cursor[3].pos); ic = 3; }
		}
		if (cursorFcn & 2)
		{
			if (abs(point.y - Cursor[0].pos - 19) < e) { e = abs(point.y - Cursor[0].pos); ic = 0; }
			if (abs(point.y - Cursor[1].pos - 19) < e) { e = abs(point.y - Cursor[1].pos); ic = 1; }
		}
		// if no cursor was selected, then look for a trace
		if ((ic == -1) && (fftMode == false))
		{
			for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
			{
				if (trace[iTrace].Enable) for (int i = 0; i < npoints; i++)
				{
					tp.x = left + i * width / npoints + 13;
					tp.y = top + height / 2 - trace[iTrace].data[i] * height / 51200 - topMargin - 16;
					if (((point.x - tp.x)*(point.x - tp.x) + (point.y - tp.y)*(point.y - tp.y)) < 10)
					{
						it = iTrace;
						TRACE1("Selected trace index = %i\r\n", it);
						break;
					}
				}
				if (it >= 0) break; // trace found
			}
			if (it == -1)
			{
				// check if on grid. Then set -2
				if ((point.x > 0) && (point.x < width) && (point.y > 0) && (point.y < height + 50)) ic = 4; // background
			}
		}
		else
			cursorMoving = true;
	}

	((CMainFrame*)AfxGetApp()->m_pMainWnd)->SetSelectedColor();
	//g_pFramework->InvalidateUICommand(IDM_SELECTCOLOR, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Color);
	//g_pFramework->InvalidateUICommand(IDM_SELECTCOLOR, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);

	CWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (zooming)
	{
		tviewTmin = viewTmin + (double)(viewTmax - viewTmin) / (width)*(min(zoomA.x, zoomB.x) - left - 10);
		tviewTmax = viewTmin + (double)(viewTmax - viewTmin) / (width)*(max(zoomA.x, zoomB.x) - left - 10);
		tviewVmin = viewVmax - (double)(viewVmax - viewVmin) / height * (max(zoomA.y, zoomB.y) - top + topMargin + 20);
		tviewVmax = viewVmax - (double)(viewVmax - viewVmin) / height * (min(zoomA.y, zoomB.y) - top + topMargin + 20);

		TRACE2("Time: [%1.4f, %1.4f]\r\n", tviewTmin, tviewTmax);
		TRACE2("Volt: [%1.4f, %1.4f]\r\n", tviewVmin, tviewVmax);

		viewTmin = tviewTmin;
		viewTmax = tviewTmax;
		viewVmin = tviewVmin;
		viewVmax = tviewVmax;
		zoomA.x = 0; zoomA.y = 0;
		zoomB.x = 0; zoomB.y = 0;

		myUpdate = false;
		Invalidate(1);
		UpdateWindow();
		zooming = false;

		// if cursors are off, then remain in zooming mode
		if (cursorFcn)
		{
			zoomMode = false;
			//g_pFramework->InvalidateUICommand(IDM_ZOOM, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
		}
	}
	else {
		cursorMoving = false;
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HDC mfdc;
	CDC *dc, *bdc, *hScreen;
	HBITMAP bitmap;
	CBrush wndBrush;
	CRect wndrect;

	wndBrush.CreateSolidBrush(RGB(255, 255, 255));
	dc = GetDC();
	GetClientRect(&wndrect);
	int w = wndrect.Width(), h = wndrect.Height();

	// Create Enhanced Metafile
	hScreen = GetDC();
	mfdc = CreateEnhMetaFile((HDC)dc, NULL, NULL, "FLUKE\0COMBISCOPE\0\0");
	if (mfdc) {
		myUpdate = false;
		Draw(mfdc, 1);
	}

	// Create DIB bitmap file
	bdc = new CDC();
	bdc->CreateCompatibleDC(dc);
	bitmap = CreateCompatibleBitmap((HDC)bdc, 1024, 768);
	bdc->SelectObject(bitmap);
	bdc->FillRect(&wndrect, &wndBrush);
	myUpdate = false;
	Draw((HDC)bdc,1);

	HLOCAL hTrace;
	char pTrace[CPYLEN],pTraceLine[100];
	sprintf_s(pTrace, CPYLEN, "Time [%s]\t", tunit);
	for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
	{
		if (trace[iTrace].ntrace > 0)
		{
			sprintf_s(pTraceLine, 100, "Trace %i\t", iTrace + 1);
			strcat_s(pTrace, CPYLEN, pTraceLine);
		}
	}
	strcat_s(pTrace, CPYLEN, "\r\n");

	int smin = (int)(viewTmin / dt * npoints / xgrid);
	int smax = (int)(viewTmax / dt * npoints / xgrid);
	for (int i = smin; i < smax; i++)
	{
		sprintf_s(pTraceLine, 100, "%1.3f\t", (viewTmin + (i - smin)*(viewTmax - viewTmin) / (smax - smin)));
		strcat_s(pTrace, CPYLEN, pTraceLine);
		for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
		{
			if (trace[iTrace].ntrace > 0)
			{
				{
					sprintf_s(pTraceLine, 100, "%f\t", (float)trace[iTrace].data[i] * ygrid*deltay / 51200.0);
					strcat_s(pTrace, CPYLEN, pTraceLine);
				}
			}
		}
		strcat_s(pTrace, CPYLEN, "\r\n");
	}
	hTrace = LocalAlloc(LPTR, strlen(pTrace) + 1);
	memcpy(hTrace, pTrace, strlen(pTrace) + 1);

	// Fill clipboard
	HENHMETAFILE hmf;
	if (OpenClipboard())
	{
		EmptyClipboard();

		SetClipboardData(CF_TEXT, hTrace);

		SetClipboardData(CF_BITMAP, bitmap);

		if (mfdc)
		{
			hmf = CloseEnhMetaFile(mfdc);
			if (hmf)
			{
				SetClipboardData(CF_ENHMETAFILE, hmf);
				CloseClipboard();
			}
		}
	}

	ReleaseDC(hScreen);
	if (mfdc)
	{
		if (hmf == NULL)
			DeleteEnhMetaFile(hmf);
		DeleteDC(mfdc);
	}
	DeleteObject(wndBrush);
	ReleaseDC(bdc);
	DeleteObject(bitmap);
	ReleaseDC(dc);

	CWnd::OnLButtonDblClk(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	zoomMode = false;
	//g_pFramework->InvalidateUICommand(IDM_ZOOM, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
	ResetZoom();

	CWnd::OnRButtonDown(nFlags, point);
}


void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_F1)
	{
		::HtmlHelp(m_hWnd, "PM33XX.chm", HH_DISPLAY_TOC, 0);
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CChildView::OnClose()
{
	CWnd::OnClose();
}

void CChildView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'h')
	{
		CImage img;
		// load PNG bitmap file
		if (img.Load(_T("pm33xx.png")) == NULL)
			hBmp = img.Detach();

		// Display bitmap in Dialog Window
		Invalidate(1);
		UpdateWindow();
	}
	if (nChar == 's')
	{
		subSample = !subSample;
		Invalidate(1);
		UpdateWindow();
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}


BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
#if _USEDOUBLEBUFFER_
	if (myUpdate)
		return FALSE;
	else
#endif
	return CWnd::OnEraseBkgnd(pDC);
}

unsigned int CChildView::ReverseBits(unsigned int p_nIndex, unsigned int p_nBits)
{
	unsigned int i, rev;

	for (i = rev = 0; i < p_nBits; i++)
	{
		rev = (rev << 1) | (p_nIndex & 1);
		p_nIndex >>= 1;
	}
	return rev;
}

unsigned int CChildView::NumberOfBitsNeeded(unsigned int p_nSamples)
{
	int i;

	if (p_nSamples < 2)
	{
		return 0;
	}
	for (i = 0; ; i++)
	{
		if (p_nSamples & (1 << i)) return i;
	}
}

void CChildView::fft_float(float *DataPoints, unsigned int SampleCount)
{
	float *p_RealOut = NULL;	// Pointer to array of real part of result
	float *p_ImagOut = NULL;	// Pointer to array of imaginary part of result
	unsigned int NumBits;
	unsigned int i, j, k, n;
	unsigned int BlockSize, BlockEnd;

	float angle_numerator = 2.0 * PI;
	float tr, ti;	// Temp real, temp imaginary

	NumBits = NumberOfBitsNeeded(SampleCount);

	p_RealOut = (float *)malloc(sizeof(float)*SampleCount);	// Get memory for real result
	if (NULL == p_RealOut) return;	// Return if couldn't get memory

	p_ImagOut = (float *)malloc(sizeof(float)*SampleCount);	// Get memory for imag result
	if (NULL == p_ImagOut)	// If couldn't get memory ...
	{
		free(p_RealOut);		// ... free memory that we did get ...
		return;					// ... and return
	}

	for (i = 0; i < SampleCount; i++)
	{
		j = ReverseBits(i, NumBits);
		p_RealOut[j] = DataPoints[i];
		p_ImagOut[j] = 0.0;
	}

	BlockEnd = 1;
	for (BlockSize = 2; BlockSize <= SampleCount; BlockSize <<= 1)
	{
		float delta_angle = angle_numerator / (float)BlockSize;
		float sm2 = sinf(-2 * delta_angle);
		float sm1 = sinf(-delta_angle);
		float cm2 = cosf(-2 * delta_angle);
		float cm1 = cosf(-delta_angle);
		float w = 2 * cm1;
		float ar[3], ai[3];

		for (i = 0; i < SampleCount; i += BlockSize)
		{
			ar[2] = cm2;
			ar[1] = cm1;

			ai[2] = sm2;
			ai[1] = sm1;

			for (j = i, n = 0; n < BlockEnd; j++, n++)
			{
				ar[0] = w * ar[1] - ar[2];
				ar[2] = ar[1];
				ar[1] = ar[0];

				ai[0] = w * ai[1] - ai[2];
				ai[2] = ai[1];
				ai[1] = ai[0];

				k = j + BlockEnd;
				tr = ar[0] * p_RealOut[k] - ai[0] * p_ImagOut[k];
				ti = ar[0] * p_ImagOut[k] + ai[0] * p_RealOut[k];

				p_RealOut[k] = p_RealOut[j] - tr;
				p_ImagOut[k] = p_ImagOut[j] - ti;

				p_RealOut[j] += tr;
				p_ImagOut[j] += ti;
			}
		}

		BlockEnd = BlockSize;
	}
	// Now put results in source waveform, overwriting original data
	for (i = 0; i < SampleCount / 2; i++)	// Copy unique half of data to every other point in dest
	{
		DataPoints[2 * i] = (sqrtf(p_RealOut[i] * p_RealOut[i] + p_ImagOut[i] * p_ImagOut[i])) / SampleCount;
	}
	for (i = 0; i < SampleCount; i += 2)	// Average inbetween points
	{
		DataPoints[i + 1] = 0.5F * (DataPoints[i] + DataPoints[i + 2]);
	}
	free(p_RealOut);		// Free temp memory
	free(p_ImagOut);
}	// End of fft_float()

#define	COS_SERIES_WINDOW(p0, p1, p2, p3, p4)	(p0)	\
				-(p1)*cosf(2*PI*i/(npoints-1))	\
				+(p2)*cosf(4*PI*i/(npoints-1))	\
				-(p3)*cosf(6*PI*i/(npoints-1))	\
				+(p4)*cosf(8*PI*i/(npoints-1))

#define	sigma 0.4F	// Choose std deviation (sigma) of 0.4, any value less than 0.5 possible

void CChildView::Spectrum(int _trace)
{
	int i;
	float src_data[35000];

	for (i = 0; i < npoints; i++)
		src_data[i] = (float)(trace[_trace].data[i] / 51200.0F*trace[_trace].y_res);

	// Apply windowing function to source data and save in target
	for (i = 0; i < npoints; i++)
	{
		switch (wndFcn)
		{
		case 2: // Gaussian window. Choose std deviation (sigma) of 0.4, any value less than 0.5 possible
			fft_data[i] = (src_data[i] * expf(-0.5F * powf((i - (npoints - 1) / 2) / (sigma*(npoints - 1) / 2), 2)));
			break;
		case 3: // Hamm
			fft_data[i] = (src_data[i] * (0.53836F - 0.46164F * cosf(2.0F * PI * i / (npoints - 1))));
			break;
		case 4: // Hann
			fft_data[i] = (src_data[i] * 0.5F * (1 - cosf(2.0F*PI*i / (npoints - 1))));
			break;
		case 5: // Bart
			fft_data[i] = (src_data[i] * (2.0F / (npoints - 1)) * (((npoints - 1) / 2.0F) - fabsf(i - ((npoints - 1) / 2.0F))));
			break;
		case 6: // Wlch
			fft_data[i] = (src_data[i] * (1.0F - powf(((i - npoints / 2.0F) / (npoints / 2.0F)), 2)));
			break;
		case 7: // Triangle
			fft_data[i] = (src_data[i] * (2.0F / (npoints)) * ((npoints / 2.0F) - fabsf(i - ((npoints - 1) / 2.0F))));
			break;
		case 8: // Brhn
			fft_data[i] = (src_data[i] * (0.62F - 0.48F*fabsf(i / (npoints - 1) - 0.5F) - 0.38F*cosf(2.0F*PI*i / (npoints - 1))));
			break;
		case 9: // Blkm
			fft_data[i] = (src_data[i] * (0.42F - 0.5F*cosf(2.0F*PI*i / (npoints - 1)) - 0.08F*cosf(4.0F*PI*i / (npoints - 1))));
			break;
		case 10: // Kais
			fft_data[i] = 0.0F;	// Work here - Requires research... Kaiser-Bessel

			// Work here - Requires research... Kaiser-Bessel
			// There is a parameter, "alpha," required to be selected by the user
			// See sample code at bottom, taken from http://ccrma.stanford.edu/courses/422/projects/kbd/
			break;
		case 11: // Nutl
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW(0.355768F, 0.487396F, 0.144232F, 0.012604F, 0.0F));
			break;
		case 12: // Bkha
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW(0.35875F, 0.48829F, 0.14128F, 0.01168F, 0.0F));
			break;
		case 13: // Bknu
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW(0.3635819F, 0.4891775F, 0.1365995F, 0.0106411F, 0.0F));
			break;
		case 14: // Fltt
			fft_data[i] = (src_data[i] * COS_SERIES_WINDOW(1.0F, 1.93F, 1.29F, 0.388F, 0.032F));
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
	for (i = 0; i < npoints; i++)
	{
		if (fft_data[i] < fftmin) fftmin = fft_data[i];
		if (fft_data[i] > fftmax) { fftmax = fft_data[i]; imax = i; }
	}
	TRACE2("FFT: peak at %i/%i\r\n", imax, npoints);

	for (i = 0; i < npoints; i++)
		trace[_trace].fdata[i] = (int)(fft_data[i] * 25600.0F / fftmax);

	trace[_trace].fftmax = fftmax;
}

void CChildView::SendMail()
{
	ShellExecute(NULL, "open",
		"mailto:gesmid@gmail.com?Subject=Subject Text&body=Body text",
		"", "", SW_SHOWNORMAL);
}

void CChildView::OnFileNew()
{
	// TODO: Add your command handler code here
}


void CChildView::OnFilePrint()
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

	int iPrnX = GetDeviceCaps(pPrint.hDC, HORZRES);
	int iPrnY = GetDeviceCaps(pPrint.hDC, VERTRES);

	sprintf_s(txt, 1000, "PM3394A");

	pDoc.fwType = DI_ROPS_READ_DESTINATION;
	pDoc.lpszDocName = txt;
	pDoc.lpszDatatype = NULL;
	pDoc.lpszOutput = NULL;
	pDoc.cbSize = sizeof(DOCINFO);

	StartDoc(pPrint.hDC, &pDoc);
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
	SetTextColor(pPrint.hDC, RGB(180, 180, 180));
	prnFont = CreateFont(80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, "Tahoma");
	SelectObject(pPrint.hDC, prnFont);
	rt.left = iPrnX / 2; rt.top = iPrnY - 150; rt.right = rt.left; rt.bottom = rt.top;
	DrawText(pPrint.hDC, "Edzko Smid", 11, &rt, DT_NOCLIP | DT_CENTER);
	DeleteObject(prnFont);

	EndPage(pPrint.hDC);
	EndDoc(pPrint.hDC);

	DeleteObject(prnFont);
	
}



void CChildView::OnFileSaveAs()
{
	int nSuccess = 1;

	lfn.lpstrTitle = _T("Select Filename");
	lfn.lpstrFilter = _T("Trace Files (*.trc)\0*.trc\0All Files (*.*)\0*.*\0\0");
	lfn.lpstrInitialDir = _T("");
	lfn.nFilterIndex = 1;
	// if data was loaded, then save it, otherwise read it

	nSuccess = GetSaveFileName(&lfn);
	if (nSuccess)
	{
		RegisterMru(lfn.lpstrFile);
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->UpdateStatus();
	}
	
	if (nSuccess)
	{
		OnFileSave();
	}
}


void CChildView::OnFileSave()
{
	FILE *storeFile;
	if (strlen(lfn.lpstrFile) == 0)
	{
		return;
	}
	if (strchr((char*)lfn.lpstrFile, '.') == NULL)
		strcat_s((char*)lfn.lpstrFile, 100, ".trc");
	int nResult = fopen_s(&storeFile, (const char*)lfn.lpstrFile, (const char*)"wbc");  // write;binary;commit
	if (nResult)
	{
		TRACE0("Trace File Open problem.\r\n");
		return;
	}
	fwrite(trace, sizeof(TRACE_T), MAX_TRACE, storeFile);
	fwrite(cdata, sizeof(int), 35000, storeFile);
	fwrite(mdata, sizeof(int), 65000, storeFile);
	fclose(storeFile);

}

void CChildView::SaveAsCSV()
{
	FILE *storeFile;
	int nSuccess = 1;
	char eFile[MAX_FILENAME], eFullFile[MAX_FILENAME];
	OPENFILENAME efn;
	char pTrace[CPYLEN];
	char pTraceLine[100];


	memcpy((void*)&efn.lStructSize, (void*)&lfn.lStructSize, sizeof(OPENFILENAME));

	efn.lpstrTitle = _T("Select Filename");
	efn.lpstrFilter = _T("Excel Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0\0");
	efn.lpstrInitialDir = _T("");
	efn.nFilterIndex = 1;
	efn.lpstrFile = eFullFile;
	efn.nMaxFile = MAX_FILENAME;
	efn.lpstrTitle = eFile;
	efn.nMaxFileTitle = MAX_FILENAME;

	// if data was loaded, then save it, otherwise read it

	nSuccess = GetSaveFileName(&efn);

	if (nSuccess)
	{
		if (strchr((char*)efn.lpstrFile, '.') == NULL)
			strcat_s((char*)efn.lpstrFile, MAX_FILENAME, ".csv");
		int nResult = fopen_s(&storeFile, (const char*)efn.lpstrFile, (const char*)"wbc");  // write;binary;commit
		if (nResult)
		{
			TRACE0("Trace File Open problem.\r\n");
			return;
		}

		sprintf_s(pTrace, CPYLEN, "Time [%s]\t", tunit);
		for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
		{
			if (trace[iTrace].ntrace > 0)
			{
				sprintf_s(pTraceLine, 100, "Trace %i\t", iTrace + 1);
				strcat_s(pTrace, CPYLEN, pTraceLine);
			}
		}
		strcat_s(pTrace, CPYLEN, "\r\n");

		int smin = (int)(viewTmin / dt * npoints / xgrid);
		int smax = (int)(viewTmax / dt * npoints / xgrid);
		for (int i = smin; i < smax; i++)
		{
			sprintf_s(pTraceLine, 100, "%f\t", (viewTmin + (i - smin)*(viewTmax - viewTmin) / (smax - smin)));
			strcat_s(pTrace, CPYLEN, pTraceLine);
			for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
			{
				if (trace[iTrace].ntrace > 0)
				{
					{
						sprintf_s(pTraceLine, 100, "%f\t", (float)trace[iTrace].data[i] * ygrid*deltay / 51200.0);
						strcat_s(pTrace, CPYLEN, pTraceLine);
					}
				}
			}
			strcat_s(pTrace, CPYLEN, "\r\n");
		}
		fwrite(pTrace, sizeof(char), (int)strlen(pTrace), storeFile);
		fclose(storeFile);
	}
}
void CChildView::SaveAsBMP()
{
	//HDC mfdc; 
	CDC *dc, *bdc;
	HBITMAP bitmap;
	CBrush wndBrush;
	CRect wndrect;

	wndBrush.CreateSolidBrush(RGB(255, 255, 255));
	dc = GetDC();
	GetClientRect(&wndrect);
	int w = wndrect.Width(), h = wndrect.Height();

	//// Create Enhanced Metafile
	//hScreen = GetDC( NULL );
	//mfdc = CreateEnhMetaFile(dc,NULL,NULL,_T("FLUKE\0COMBISCOPE\0\0"));
	//if (mfdc) {
	//	myUpdate = false;
	//	Draw(mfdc,1);
	//}

	// Create DIB bitmap file
	bdc = new CDC();
	bdc->CreateCompatibleDC(dc);
	bitmap = CreateCompatibleBitmap((HDC)bdc, 1024, 768);
	SelectObject((HDC)bdc, bitmap);
	bdc->FillRect(wndrect, &wndBrush);
	myUpdate = false;
	Draw((HDC)bdc, 1);

	CImage img;
	img.Attach(bitmap);
	img.Save("PM33XX.bmp");

	ReleaseDC(dc);
	DeleteObject(bdc);
	DeleteObject(bitmap);
}

void CChildView::SaveAsPNG()
{
	//HDC mfdc; 
	CDC *dc, *bdc;
	HBITMAP bitmap;
	CBrush wndBrush;
	CRect wndrect;

	wndBrush.CreateSolidBrush(RGB(255, 255, 255));
	dc = GetDC();
	GetClientRect(&wndrect);
	int w = wndrect.Width(), h = wndrect.Height();

	//// Create Enhanced Metafile
	//hScreen = GetDC( NULL );
	//mfdc = CreateEnhMetaFile(dc,NULL,NULL,_T("FLUKE\0COMBISCOPE\0\0"));
	//if (mfdc) {
	//	myUpdate = false;
	//	Draw(mfdc,1);
	//}

	// Create DIB bitmap file
	bdc = new CDC();
	bdc->CreateCompatibleDC(dc);
	bitmap = CreateCompatibleBitmap((HDC)dc, 1024, 768);
	SelectObject((HDC)bdc, bitmap);
	FillRect((HDC)bdc, wndrect, wndBrush);
	myUpdate = false;
	Draw((HDC)bdc, 1);

	CImage img;
	img.Attach(bitmap);
	img.Save("PM33XX.png");

	ReleaseDC(dc);
	DeleteObject(bdc);
	DeleteObject(bitmap);
}

void CChildView::AutoSetup()
{
	if (hCommPort)
	{
		unsigned long nc;
		WriteFile(hCommPort, "AS\r", 3, &nc, NULL);
	}
}

void CChildView::OnFft()
{
	fftMode = !fftMode;
	if (fftMode)
	{
		for (int iTrace = 0; iTrace < MAX_TRACE; iTrace++)
			if ((trace[iTrace].ntrace > 0) && (trace[iTrace].Enable))
				Spectrum(iTrace);
	}
	ResetZoom();
	Invalidate(1);
	UpdateWindow();
}

void CChildView::OnFftWnd(UINT id) {
	wndFcn = id - ID_FLTWND1 + 1;
	if (fftMode) {
		fftMode = false; OnFft();
	}
}


void CChildView::OnXlog()
{
	xLog = !xLog;
	if (fftMode) {
		fftMode = false; OnFft();
	}
}


void CChildView::OnYlog()
{
	yLog = !yLog;
	Invalidate(1);
	UpdateWindow();
}


void CChildView::OnFileMruFile1()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(0, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile2()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(1, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile3()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(2, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile4()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(3, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile5()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(4, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile6()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(5, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile7()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(6, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnFileMruFile8()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(7, lfn.lpstrFile);
	OpenFile();
}

void CChildView::OnFileMruFile9()
{
	((CPM33XXMFCApp*)AfxGetApp())->GetMRU(8, lfn.lpstrFile);
	OpenFile();
}


void CChildView::OnChannel(UINT id)
{
	int iTrace = id - ID_CH1;
	trace[iTrace].Enable = !trace[iTrace].Enable;
}

void CChildView::OnMemory(UINT id)
{
}