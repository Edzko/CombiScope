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

// ChildView.h : interface of the CChildView class
//


#pragma once

#define MAX_TRACE  36
#define MAX_FILENAME 250
#define FLUKE_BG  RGB(0xFF,0xC0,0)
#define CPYLEN 100000
#define PI 3.14159265359F

#define _USEDOUBLEBUFFER_ 1


typedef struct
{
	char y_unit[10], x_unit[10], tdate[20], ttime[20], trace_id[10];
	float x_zero, y_zero, x_res, y_res, delta_t;
	double Vpp, Vmin, Vmax, tpulse, tperiod, freqprim;
	int y_range, ntrace;
	int *data, *fdata;
	float fftmax;
	COLORREF color;
	int width;
	bool Enable;
	//char dummy[1000];  // store sme dummy space, for forward compatibility with datafiles, in case we want to add to the structure.
} TRACE_T;

typedef struct
{
	int pos;
	COLORREF color;
	int width;
	double value;
} CURSOR_T;


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();


// Attributes
public:
	bool myUpdate, zooming, zoomMode;
	bool showInfo;
	bool cursorMoving;
	bool fftMode, xLog, yLog;
	bool subSample;
	bool HaveView;  // flag to trigger the first time we have a valid window handle

	char tlabel[20], tunit[20];
	char CommPort[50];
	char BaudRates[5][20];
	char pmid[200];
	int progress;
	int SelectedBaudrate, SelectedComport, ComData[50];

	int left, top, width, height, infowidth;
	int xgrid, ygrid;
	int topMargin;
	int cursorFcn;
	int timeout;
	int ic; // selected cursor index
	int it; // selected trace index
	int is; // spectrum trace. 
	int wndFcn;

	int npoints; // global number of acquisition points per trace
	float xres; // global scope x-res
	double t, dt, y, y00, deltay;

	double viewVmin, viewVmax, viewTmin, viewTmax;
	double tviewVmin, tviewVmax, tviewTmin, tviewTmax;

	int cdata[35000], mdata[65000], fdata[35000];
	float fft_data[35000];

	COLORREF cc[16], gridCol, lblCol, bgCol;
	CPoint zoomA, zoomB;
	TRACE_T trace[MAX_TRACE];
	HANDLE hCommPort;
	CString  data;
	HKEY hKey, hmruKey;
	HBITMAP hBmp;
	CURSOR_T Cursor[4];
	CPoint mpoint;
	
	char DefFullName[MAX_FILENAME], DefName[MAX_FILENAME];
	OPENFILENAME lfn;
	char mru[10][MAX_FILENAME];

	DCB	dcbCommPort;
	BYTE byteSize, parity, stopBits;
	COMMTIMEOUTS CommTimeouts;

// Operations
public:
	void Draw(HDC dc0, int CID);
	void getCursorPos();
	void getCursorValue();
	void RegisterMru(char *fn);
	void SetTraceData(int n);
	void GetTrace(int iTrace);
	void AutoSetup();
	void ResetZoom();
	void Connect(int port, int baud);
	BOOL InitCommPort(int CommPortNum, int baud,
		BYTE ByteSize, BYTE Parity, BYTE StopBits);
	void OpenFile();
	void SaveAsCSV();
	void SaveAsBMP();
	void SaveAsPNG();
	void SendMail();
	void Spectrum(int _trace);
	void fft_float(float *DataPoints, unsigned int SampleCount);
	unsigned int NumberOfBitsNeeded(unsigned int p_nSamples);
	unsigned int ReverseBits(unsigned int p_nIndex, unsigned int p_nBits);

	void OnFftWnd(UINT id);
	void OnChannel(UINT id);
	void OnMemory(UINT id);

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnConnect();
	afx_msg void OnHardcopy();
	afx_msg void OnPort();
	afx_msg void OnBaud();
	afx_msg void OnCopy();
	afx_msg void OnConfig();
	afx_msg void OnGettraces();
	afx_msg void OnRun();
	afx_msg void OnColor();
	afx_msg void OnFftwindow();
	afx_msg void OnC0();
	afx_msg void OnCx();
	afx_msg void OnCxy();
	afx_msg void OnCy();
	afx_msg void OnCursors();
	afx_msg void OnFileOpen();
	afx_msg void OnZoom();
	afx_msg void OnInfo();
	afx_msg void OnXLog();
	afx_msg void OnYLog();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClose();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFileNew();
	afx_msg void OnFilePrint();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSave();
	afx_msg void OnFft();
	afx_msg void OnXlog();
	afx_msg void OnYlog();
	afx_msg void OnFileMruFile1();
	afx_msg void OnFileMruFile2();
	afx_msg void OnFileMruFile3();
	afx_msg void OnFileMruFile4();
	afx_msg void OnFileMruFile5();
	afx_msg void OnFileMruFile6();
	afx_msg void OnFileMruFile7();
	afx_msg void OnFileMruFile8();
	afx_msg void OnFileMruFile9();
//	afx_msg void OnUpdateFileMruFile1(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile2(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile3(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile4(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile5(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile6(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile7(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile8(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateFileMruFile9(CCmdUI *pCmdUI);
};

