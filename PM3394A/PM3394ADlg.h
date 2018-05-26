// PM3394ADlg.h : header file
//

#if !defined(AFX_PM3394ADLG_H__55EAA47D_1D51_4431_8FC4_AEBA1D456AA1__INCLUDED_)
#define AFX_PM3394ADLG_H__55EAA47D_1D51_4431_8FC4_AEBA1D456AA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlimage.h>

typedef struct
{
	char y_unit[10],x_unit[10],tdate[20],ttime[20],trace_id[10];
	float x_zero, y_zero,x_res, y_res, delta_t;
	double Vpp,Vmin,Vmax,tpulse,tperiod,freqprim;
	int y_range, ntrace;
	int data[1000];
	COLORREF color;
	int width;
} TRACE_T;

typedef struct
{
	int pos;
	COLORREF color;
	int width;
} CURSOR_T;

#define MAX_TRACE  36

/////////////////////////////////////////////////////////////////////////////
// CPM3394ADlg dialog

class CPM3394ADlg : public CDialog
{
// Construction
public:
	int timeout;
	CPM3394ADlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CPM3394ADlg();
	char tlabel[20], tunit[20];
	void GetTrace(int iTrace);
	void Draw(CDC *dc, int CID);
	BOOL InitCommPort(int CommPortNum, int baud,BYTE ByteSize, BYTE Parity, BYTE StopBits);
	void GetCOMPorts(CComboBox *pCOM);
	void Connect(int port);
	HANDLE hCommPort;
	DCB	dcbCommPort;
	COMMTIMEOUTS CommTimeouts;
	int Baud;
	DWORD baudRate;			// current baud rate
	BYTE byteSize, parity, stopBits;			
	double t, dt, y, y0, deltay;
	CString  data;
	CStatusBar sb;
	CWnd *pPlotBox;
	CButton *pCursors, *pInfo;
	CConfig *cfgDlg;
	CComboBox *pCom;
	HBITMAP hBmp;
	CImage img;
	TRACE_T trace[MAX_TRACE];
	
	HBITMAP hbmHP, hbmTRACE,hbmCOPY, hbmCONNECT, hbmHELP,hbmEXCEL,hbmINFO,hbmDISK;
	HBITMAP hbmSTOP, hbmCONFIG, hbmCURSORS, hbmPRINT, hbmGETSETUP, hbmAUTOSETUP;
	HBITMAP hbmZOOM, hbmTRIGGER;
	
	CURSOR_T Cursor[4];
	int ic; // cursor index
	int xgrid, ygrid;
	bool myUpdate, zooming;
	CPoint zoomA, zoomB;
	double viewVmin,viewVmax,viewTmin,viewTmax;
	double tviewVmin,tviewVmax,tviewTmin,tviewTmax;

	CToolTipCtrl tt;
	BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	
	COLORREF cc[16], gridCol, lblCol, bgCol;
	
	char DefFullName[100], DefName[100];
	OPENFILENAME lfn;
	

	// Dialog Data
	//{{AFX_DATA(CPM3394ADlg)
	enum { IDD = IDD_PM3394A_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPM3394ADlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPM3394ADlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAutoSetup();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnHp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGetTraces();
	afx_msg void OnBnClickedGetSetup();
	afx_msg void OnBnClickedHelpme();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedCursors();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedConfig();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPrint();
	afx_msg void OnBnClickedInfo();
	afx_msg void OnBnClickedDisk();
	afx_msg void OnBnClickedDiskRead();
	afx_msg void OnBnClickedExcel();
	afx_msg void OnBnClickedTrigger();
	afx_msg void OnBnClickedZoom();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PM3394ADLG_H__55EAA47D_1D51_4431_8FC4_AEBA1D456AA1__INCLUDED_)
