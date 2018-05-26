// PM3394ADlg.cpp : implementation file
//

#include "stdafx.h"
#include <Setupapi.h>
#include <Devguid.h>
#include "Config.h"
#include "PM3394A.h"
#include "PM3394ADlg.h"
#include <Cderr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FLUKE_BG  RGB(0xFF,0xC0,0)
unsigned int SBID[5] = {3001,3002,3003,3004,3005};

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPM3394ADlg dialog

CPM3394ADlg::CPM3394ADlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPM3394ADlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPM3394ADlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CPM3394ADlg::~CPM3394ADlg()
{
	tt.DestroyWindow();
	delete(tt);

	cfgDlg->DestroyWindow();
	delete(cfgDlg);

	sb.DestroyWindow();
}

void CPM3394ADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPM3394ADlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPM3394ADlg, CDialog)
	//{{AFX_MSG_MAP(CPM3394ADlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_SIZING()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDC_AUTOSETUP, &CPM3394ADlg::OnAutoSetup)
	ON_BN_CLICKED(IDC_TRACE, &CPM3394ADlg::OnBnClickedGetTraces)
	ON_BN_CLICKED(IDC_GETSETUP, &CPM3394ADlg::OnBnClickedGetSetup)
	ON_BN_CLICKED(IDC_HELPME, &CPM3394ADlg::OnBnClickedHelpme)
	ON_BN_CLICKED(IDC_HP, &CPM3394ADlg::OnHp)
	ON_BN_CLICKED(IDC_CURSORS, &CPM3394ADlg::OnBnClickedCursors)
	ON_BN_CLICKED(IDC_CONFIG, &CPM3394ADlg::OnBnClickedConfig)
	ON_BN_CLICKED(IDC_CONNECT, &CPM3394ADlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_COPY, &CPM3394ADlg::OnBnClickedCopy)

	ON_NOTIFY_EX_RANGE( TTN_NEEDTEXT, 0, 0xFFFF , &CPM3394ADlg::OnToolTipNotify)
	ON_NOTIFY_EX_RANGE( TTN_NEEDTEXTA, 0, 0xFFFF , &CPM3394ADlg::OnToolTipNotify)
	ON_NOTIFY_EX_RANGE( TTN_NEEDTEXTW, 0, 0xFFFF , &CPM3394ADlg::OnToolTipNotify)

	ON_BN_CLICKED(IDC_PRINT, &CPM3394ADlg::OnBnClickedPrint)
	ON_BN_CLICKED(IDC_INFO, &CPM3394ADlg::OnBnClickedInfo)
	ON_BN_CLICKED(IDC_SAVEFILE, &CPM3394ADlg::OnBnClickedDisk)
	ON_BN_CLICKED(IDC_READFILE, &CPM3394ADlg::OnBnClickedDiskRead)
	ON_BN_CLICKED(IDC_EXCEL, &CPM3394ADlg::OnBnClickedExcel)
	ON_BN_CLICKED(IDC_TRIGGER, &CPM3394ADlg::OnBnClickedTrigger)
	ON_BN_CLICKED(IDC_ZOOM, &CPM3394ADlg::OnBnClickedZoom)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPM3394ADlg message handlers

void CPM3394ADlg::GetCOMPorts(CComboBox *pCOM)
{
	int iLB = 0, idx = 0;
	pCOM->ResetContent();
	HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL,NULL,NULL,DIGCF_PRESENT | DIGCF_ALLCLASSES);
	SP_DEVICE_INTERFACE_DATA interfaceData;
	ZeroMemory(&interfaceData, sizeof(interfaceData));
	interfaceData.cbSize = sizeof(interfaceData);

	for (int nDevice=0;nDevice<2000;nDevice++)
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
			TRACE1("Friendlyname: %s\r\n",friendlyName);
			char *pCom = strstr(friendlyName,"(COM");
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
					TRACE1("Manufacturer: %s\r\n",manufName);

					// NOTE: Intrepid ValueCAN may show up as FTDI 
					//if ((ComType == USB) || (strcmp(manufName,"FTDI")==0) || (strcmp(manufName,"ICS")==0))
					//if ((ComType == USB) || (strcmp(manufName,"ICS")==0))
					{
						idx = pCOM->AddString(CString(friendlyName));
						int iCom = pCom[4]-'0';
						if (pCom[5]!=')') iCom = 10 * iCom + pCom[5] - '0';
						pCOM->SetItemData(idx,iCom);

						//if (iLB<20) mCOM[iLB] = manufName;
						iLB++;
					}
				}
			}
		}
	}
	int iCOM = AfxGetApp()->GetProfileInt(_T(""),_T("COM"),0);
	if ((iCOM>=0) && (pCOM->GetCount()>=iCOM)) 
		pCOM->SetCurSel(iCOM); 
	else pCOM->SetCurSel(0);
}

BOOL CPM3394ADlg::InitCommPort(int CommPortNum, int baud,
	BYTE ByteSize, BYTE Parity, BYTE StopBits)
{
	// defaults to COM1, BAUD192, BYTESIZE8, NOPARITY, ONESTOPBIT
	char CommPort[12];
	sprintf_s(CommPort,12,"COM%i",CommPortNum);
	hCommPort = CreateFile(CString(CommPort), GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING,	FILE_ATTRIBUTE_NORMAL, NULL);
	if (hCommPort == INVALID_HANDLE_VALUE) {
		DWORD dwError = GetLastError();
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
	CommTimeouts.ReadIntervalTimeout = 1000;
	CommTimeouts.ReadTotalTimeoutMultiplier = 100;
	CommTimeouts.ReadTotalTimeoutConstant = 500;
	CommTimeouts.WriteTotalTimeoutMultiplier = 1;
	CommTimeouts.WriteTotalTimeoutConstant = 1;
	fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
	if (!fSuccess) {
		//MessageBox(NULL, "Couldn't set comm timeouts.", "Error",MB_OK + MB_ICONSTOP);
		return FALSE;
	}
   return TRUE;
}

void CPM3394ADlg::Connect(int port)
{
	char iddata[1000];
	unsigned long nc;

	if (hCommPort != INVALID_HANDLE_VALUE)
	{
		// close com port first, before attempting to re-connect
		CloseHandle(hCommPort);
		hCommPort = INVALID_HANDLE_VALUE;
	};

	if (InitCommPort(port, Baud, 8, 0, 1)) {
		FlushFileBuffers(hCommPort);
		memset(iddata,0,1000);
		WriteFile(hCommPort,"ID\r",3,&nc,NULL);	
		Sleep(500);
		ReadFile(hCommPort,iddata,1,&nc,NULL);
		if (nc>0)
		{
			ReadFile(hCommPort,iddata,1000,&nc,NULL);
			TRACE1("Instrument ID: %s\r\n",&iddata[1]);
			sb.SetPaneText(4,CString(&iddata[1]));
			sb.SetPaneText(0,_T("Connected"));
		} else {
			CloseHandle(hCommPort);
			hCommPort = INVALID_HANDLE_VALUE;
		}
	} 
}

BOOL CPM3394ADlg::OnInitDialog()
{
	myUpdate = false;
	Baud = 38400;
	char tcr[50];
	CDialog::OnInitDialog();

	// setup default colors for traces
	for (int i=0;i<36;i++) 
	{
		sprintf_s(tcr,50,"TC%i",i+1);
		trace[0].color = AfxGetApp()->GetProfileInt(_T(""),CString(tcr),(int)RGB(0,200,0));
	}
	
	trace[0].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC1"),(int)RGB(255,0,0));
	trace[1].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC2"),(int)RGB(255,0,0));
	trace[2].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC3"),(int)RGB(255,0,0));
	trace[3].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC4"),(int)RGB(255,0,0));
	Cursor[0].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC37"),(int)RGB(255,0,0));
	Cursor[1].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC38"),(int)RGB(255,0,0));
	Cursor[2].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC39"),(int)RGB(255,0,0));
	Cursor[3].color = AfxGetApp()->GetProfileInt(_T(""),_T("TC40"),(int)RGB(255,0,0));
	Cursor[0].width=1;
	Cursor[1].width=1;
	Cursor[2].width=1;
	Cursor[3].width=1;
	gridCol = AfxGetApp()->GetProfileInt(_T(""),_T("TC41"),(int)RGB(100,100,100));
	lblCol = AfxGetApp()->GetProfileInt(_T(""),_T("TC42"),(int)RGB(0,0,128));
	bgCol = AfxGetApp()->GetProfileInt(_T(""),_T("TC43"),(int)RGB(255,255,255));

	for (int i=0;i<MAX_TRACE;i++) {trace[i].width = 2;trace[i].ntrace=0;}

	cfgDlg = new CConfig(this);
	cfgDlg->Create(IDD_CONFIGURATION);
	
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Add bitmaps to the buttons
	CButton *pB;
	HINSTANCE hInst = ::AfxGetApp()->m_hInstance;
	pB = (CButton*)GetDlgItem(IDC_HP);
	hbmHP = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_HP));
	pB->SetBitmap(hbmHP); 
	pB = (CButton*)GetDlgItem(IDC_TRACE);
	hbmTRACE = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TRACE));
	pB->SetBitmap(hbmTRACE); 
	pB = (CButton*)GetDlgItem(IDC_COPY);
	hbmCOPY = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_COPY));
	pB->SetBitmap(hbmCOPY); 
	pB = (CButton*)GetDlgItem(IDC_CONNECT);
	hbmCONNECT = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CONNECT));
	pB->SetBitmap(hbmCONNECT); 
	pB = (CButton*)GetDlgItem(IDC_HELPME);
	hbmHELP = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_HELP));
	pB->SetBitmap(hbmHELP); 
	pB = (CButton*)GetDlgItem(IDOK);
	hbmSTOP = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_STOP));
	pB->SetBitmap(hbmSTOP); 
	pB = (CButton*)GetDlgItem(IDC_CONFIG);
	hbmCONFIG = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SETTINGS));
	pB->SetBitmap(hbmCONFIG); 
	pB = (CButton*)GetDlgItem(IDC_PRINT);
	hbmPRINT = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_PRINT));
	pB->SetBitmap(hbmPRINT); 
	pB = (CButton*)GetDlgItem(IDC_GETSETUP);
	hbmGETSETUP = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_INFO));
	pB->SetBitmap(hbmGETSETUP); 
	pB = (CButton*)GetDlgItem(IDC_AUTOSETUP);
	hbmAUTOSETUP = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_AA));
	pB->SetBitmap(hbmAUTOSETUP); 
	pB = (CButton*)GetDlgItem(IDC_EXCEL);
	hbmEXCEL = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_EXCEL));
	pB->SetBitmap(hbmEXCEL); 
	pB = (CButton*)GetDlgItem(IDC_SAVEFILE);
	hbmDISK = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_DISK));
	pB->SetBitmap(hbmDISK); 
	pB = (CButton*)GetDlgItem(IDC_READFILE);
	pB->SetBitmap(hbmDISK); 
	pB = (CButton*)GetDlgItem(IDC_ZOOM);
	hbmZOOM = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ZOOM));
	pB->SetBitmap(hbmZOOM); 
	pB = (CButton*)GetDlgItem(IDC_TRIGGER);
	hbmTRIGGER = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_RUN));
	pB->SetBitmap(hbmTRIGGER); 

	// Create tooltips
	tt.Create(this,TTS_ALWAYSTIP);
	EnableToolTips(true);	
	tt.Activate(true);

	// Create and setup status bar
	sb.Create(this);
	sb.SetIndicators(SBID,5);
	sb.SetPaneInfo(0,SBID[0],SBPS_NORMAL,120);
	sb.SetPaneInfo(1,SBID[1],SBPS_NORMAL,60);
	sb.SetPaneInfo(2,SBID[2],SBPS_NORMAL,60);
	sb.SetPaneInfo(3,SBID[3],SBPS_NORMAL,60);
	sb.SetPaneInfo(4,SBID[4],SBPS_STRETCH ,0);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,SBID[4]);
	sb.SetPaneText(1,_T("COM4"));
	sb.SetPaneText(2,_T("38400"));

	pPlotBox = (CWnd*)GetDlgItem(IDC_PLOT);
	
	pCursors = (CButton*)GetDlgItem(IDC_CURSORS);
	hbmCURSORS = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CURSORS));
	pCursors->SetBitmap(hbmCURSORS); 
	
	pInfo = (CButton*)GetDlgItem(IDC_INFO);
	hbmINFO = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_INFO));
	pInfo->SetBitmap(hbmINFO); 
	
	pCom = (CComboBox*)cfgDlg->GetDlgItem(IDC_COMPORTS);

	hCommPort = INVALID_HANDLE_VALUE;
	GetCOMPorts(pCom);	
	sb.SetPaneText(0,_T("Not Connected"));
	// open a file on COM4
	int _cp = (int)pCom->GetItemData(pCom->GetCurSel());
	Connect(_cp);

	timeout = 0;
	data = "";
	hBmp = NULL;
	dt = 0.1;
	deltay = 0.25;
	y0 = 0.0;
	strcpy_s(tlabel,20,"[undefined]");
	strcpy_s(tunit,20,"");
	Cursor[0].pos = 200;
	Cursor[1].pos = 210;
	Cursor[2].pos = 100;
	Cursor[3].pos = 110;
	ic = -1;
	xgrid = 10;
	ygrid = 8;
	zooming = false;
	zoomA.x=0;zoomA.y=0;
	zoomB.x=0;zoomB.y=0;
	viewTmin = 0.0;
	viewTmax = 1.0;
	viewVmin = ygrid/-2.0*deltay;
	viewVmax = ygrid/2.0*deltay;
	
	// if not exist ver.inf
	// system("ver > ver.inf");
	// load ver

	strcpy_s(DefFullName,100,"");
	strcpy_s(DefName,100,"");
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPM3394ADlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CPM3394ADlg::Draw(CDC *dc, int CID)
{
	CWnd *pCID;
	CBitmap dcbmp;
	CDC *dcmem;
	CPen art;
	CBrush fil;
	CFont fnt;
	BITMAP bm;
	int dx, dy, left, top, width, height, infoy = 0, fnth, infowidth, yline;
	RECT rc,cwnd, dwnd;
	POINT tp[10000];
	char valstr[100];
	bool showInfo = false;
	
	if (CID==0)   // from print dialog
	{
		//GetClientRect(&cwnd);
		dc->GetClipBox(&cwnd);
		//if ((cwnd.right>800) &&(cwnd.left==0)) cwnd.right=500;
		//if ((cwnd.bottom>400) &&(cwnd.top==0)) cwnd.bottom=300;
		if (cwnd.right<cwnd.bottom) cwnd.bottom /= 2;
		cwnd.left+=2;
		cwnd.top+=1;
		if (pInfo->GetCheck()) showInfo = true;
		fnth = 48;
		infowidth = 800;
		yline = 80;
	} else if (CID==1)
	{
		dc->GetClipBox(&cwnd);
		if ((cwnd.right>800) && (cwnd.left==0)) cwnd.right=600;
		if ((cwnd.bottom>400) && (cwnd.top==0)) cwnd.bottom=400;
		cwnd.left+=2;
		cwnd.top+=1;
		fnth = 12;
		infowidth = 200;
		yline = 15;
	} else {
		pCID = GetDlgItem(CID);
		pCID->GetWindowRect(&cwnd);// control window
		GetWindowRect(&dwnd);// dialog box window

		cwnd.top -= dwnd.top;
		cwnd.bottom -= dwnd.top;
		cwnd.left -= dwnd.left;
		cwnd.right -= dwnd.left;
		
		if (pInfo->GetCheck()) showInfo = true;
		fnth = 12;
		infowidth = 200;
		yline = 15;
	} 


	if (hBmp)
	{
		dcmem = new CDC();
		dcmem->CreateCompatibleDC( dc );
		dcmem->SelectObject ( hBmp );
		GetObject ( hBmp, sizeof(bm), &bm );
		dx = bm.bmWidth;
		dy = bm.bmHeight;
		dcbmp.CreateCompatibleBitmap(dc,dx,dy);
		dc->SelectObject(dcbmp);
		dc->StretchBlt(cwnd.left,cwnd.top,cwnd.right - cwnd.left-20,cwnd.bottom - cwnd.top-40,dcmem,0,0,dx,dy,SRCCOPY);
		dcmem->DeleteDC();
		delete(dcmem);
	} else {
		left = cwnd.left+25;
		top = cwnd.top;
		width = cwnd.right - cwnd.left-45;
		height = cwnd.bottom - cwnd.top-60;
		
		if (showInfo)
		{	
			width -= infowidth;
			infoy = top;
		}

		rc.right=left+width;
		rc.left=left;
		rc.top=top;
		rc.bottom=top+height;
		fil.CreateSolidBrush(bgCol);
		dc->FillRect(&rc,&fil);
		fil.DeleteObject();

		// Grid lines
		art.CreatePen(PS_SOLID,2,gridCol);
		tp[0].x = left; tp[0].y = top;
		tp[1].x = left+width; tp[1].y = top;
		tp[2].x = left+width; tp[2].y = top+height;
		tp[3].x = left; tp[3].y = top+height;
		tp[4].x = left; tp[4].y = top;
		dc->SelectObject(art);
		dc->Polyline(tp,5);
		tp[0].x = left; tp[0].y = top + height/2;
		tp[1].x = left+width; tp[1].y = top + height/2;
		dc->Polyline(tp,2);
		tp[0].x = left+width/2; tp[0].y = top;
		tp[1].x = left+width/2; tp[1].y = top + height;
		dc->Polyline(tp,2);
		art.DeleteObject();
		art.CreatePen(PS_DASH,1,gridCol);
		dc->SelectObject(art);
		for (int i=1;i<ygrid;i++)
		{
			tp[0].x = left; tp[0].y = top + i*height/ygrid;
			tp[1].x = left+width; tp[1].y = top + i*height/ygrid;
			dc->Polyline(tp,2);
		}
		for (int i=1;i<xgrid;i++)
		{
			tp[0].x = left+i*width/xgrid; tp[0].y = top;
			tp[1].x = left+i*width/xgrid; tp[1].y = top + height;
			dc->Polyline(tp,2);
		}
		art.DeleteObject();
		art.CreatePen(PS_SOLID,1,gridCol);
		dc->SelectObject(art);
		tp[0].x = left; tp[0].y = top + height/4;
		tp[1].x = left+width; tp[1].y = top + height/4;
		dc->Polyline(tp,2);
		tp[0].x = left; tp[0].y = top + height*3/4;
		tp[1].x = left+width; tp[1].y = top + height*3/4;
		dc->Polyline(tp,2);
		for (int i=1;i<40;i++)
		{
			tp[0].x = left+i*width/40; tp[0].y = top + height/2-5;
			tp[1].x = left+i*width/40; tp[1].y = top + height/2+5;
			dc->Polyline(tp,2);
			tp[0].x = left+width/2-5; tp[0].y = top + i*height/40;
			tp[1].x = left+width/2+5; tp[1].y = top + i*height/40;
			dc->Polyline(tp,2);

			tp[0].x = left+i*width/40; tp[0].y = top + height/4-2;
			tp[1].x = left+i*width/40; tp[1].y = top + height/4+2;
			dc->Polyline(tp,2);
			tp[0].x = left+i*width/40; tp[0].y = top + height*3/4-2;
			tp[1].x = left+i*width/40; tp[1].y = top + height*3/4+2;
			dc->Polyline(tp,2);
		}
		art.DeleteObject();
		art.CreatePen(PS_DOT,1,gridCol);
		dc->SelectObject(art);
		tp[0].x = left; tp[0].y = top + height*3/16;
		tp[1].x = left+width; tp[1].y = top + height*3/16;
		dc->Polyline(tp,2);
		tp[0].x = left; tp[0].y = top + height*13/16;
		tp[1].x = left+width; tp[1].y = top + height*13/16;
		dc->Polyline(tp,2);
		art.DeleteObject();

		if (myUpdate==false)
		{
			fnt.CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
			dc->SetTextColor(lblCol);
			dc->SelectObject(&fnt);
			t = viewTmin;
			y = viewVmin; // + y0;
			for (int i=0;i<=xgrid;i++)
			{
				rc.left = left+i*width/xgrid; rc.right=rc.left;
				rc.top = top+height+5;rc.bottom=rc.top;
				sprintf_s(valstr,100,"%1.2f",t);
				t += (viewTmax-viewTmin)/xgrid; 
				dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_CENTER);
			}
			for (int i=0;i<=ygrid;i++)
			{
				rc.left = left-5; rc.right=rc.left;
				rc.top = top+(ygrid-i)*height/ygrid-5;rc.bottom=rc.top;
				sprintf_s(valstr,100,"%1.2f",y);
				y += (viewVmax-viewVmin)/ygrid;
				dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_RIGHT);
			}
			// draw time label
			rc.left = left+width/2; rc.right=rc.left;
			rc.top = top+height+15;rc.bottom=rc.top;
			dc->DrawText(CString(tlabel),(int)strlen(tlabel),&rc,DT_NOCLIP | DT_CENTER);
			fnt.DeleteObject();
		}

		for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
		{
			if ((trace[iTrace].ntrace>0) && ((CButton*)cfgDlg->GetDlgItem(1100+iTrace))->GetCheck())
			{
				trace[iTrace].Vmax = -1e6;;
				trace[iTrace].Vmin = 1e6; 

				for (int i=(int)(viewTmin/dt*trace[iTrace].ntrace);i<(int)(viewTmax/dt*trace[iTrace].ntrace/xgrid);i++)
				{
					tp[i].x = left + i*width/trace[iTrace].ntrace;
					tp[i].y = top + height/2 - trace[iTrace].data[i]*height/51200;
					if (tp[i].y>trace[iTrace].Vmax) 
						trace[iTrace].Vmax=(double)tp[i].y;
					if (tp[i].y<trace[iTrace].Vmin) 
						trace[iTrace].Vmin=(double)tp[i].y;
				}
				art.CreatePen(PS_SOLID,trace[iTrace].width,trace[iTrace].color);
				dc->SelectObject(art);
				dc->Polyline(tp,trace[iTrace].ntrace);
				trace[iTrace].Vmax = (trace[iTrace].Vmax - top)/height*ygrid*deltay-trace[iTrace].y_zero-ygrid/2*deltay;
				trace[iTrace].Vmin = (trace[iTrace].Vmin - top)/height*ygrid*deltay-trace[iTrace].y_zero-ygrid/2*deltay;
				trace[iTrace].Vpp = trace[iTrace].Vmax - trace[iTrace].Vmin;

				if (showInfo)
				{

					rc.left = left+width+15; rc.right=rc.left+(LONG)(0.9*infowidth);
					rc.top = infoy;rc.bottom=rc.top+93*yline/15;
					fil.CreateSolidBrush(RGB(255,255,255));
					//dc->FillRect(&rc,&fil);
					dc->Rectangle(&rc);
					fil.DeleteObject();

					fnt.CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
					dc->SetTextColor(trace[iTrace].color);
					dc->SelectObject(&fnt);
		
					rc.left = left+width+18; rc.right=rc.left;
					rc.top = infoy+3;rc.bottom=rc.top;
					if (iTrace<4)
					{
						sprintf_s(valstr,100,"Trace info [%s]:",trace[iTrace].trace_id);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						//sprintf_s(valstr,100,"More information to be displayed here.");
						sprintf_s(valstr,100,"Date: %s",trace[iTrace].tdate);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"Delta_t=%1.1f %1.1f s",trace[iTrace].delta_t,trace[iTrace].x_res);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"Y0=%1.1f %s %1.1f s",trace[iTrace].y_zero,trace[iTrace].y_unit);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"Acq: %i samples",trace[iTrace].ntrace);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						
						sprintf_s(valstr,100,"Vmax=%1.2f Vmin=%1.2f  Vpp=%1.2f",trace[iTrace].Vmax,trace[iTrace].Vmin,trace[iTrace].Vpp);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += (int)(yline*1.4);
					}
					else 
					{
						int traceGroup = iTrace / 4;
						int traceRegister = iTrace - 4 * traceGroup + 1;
						sprintf_s(valstr,100,"Memory info [%i.%i]:",traceGroup,traceRegister);
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += yline;
						rc.top = infoy;rc.bottom=rc.top;
						sprintf_s(valstr,100,"More information to be displayed here.");
						dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
						infoy += (int)(yline*1.4);
					}
					fnt.DeleteObject();
				}
				art.DeleteObject();
			}
		}

		if (pCursors->GetCheck())
		{
			// TODO: if CID==0  then re-scale to match the time and V
			art.CreatePen(PS_DASHDOT,Cursor[0].width,Cursor[0].color);
			dc->SelectObject(art);
			tp[0].x = left; tp[0].y = Cursor[0].pos;
			tp[1].x = left+width; tp[1].y = Cursor[0].pos;
			dc->Polyline(tp,2);
			art.DeleteObject();

			art.CreatePen(PS_DASHDOT,Cursor[2].width,Cursor[2].color);
			dc->SelectObject(art);
			tp[0].x = Cursor[2].pos; tp[0].y = top;
			tp[1].x = Cursor[2].pos; tp[1].y = top+height;
			dc->Polyline(tp,2);
			art.DeleteObject();

			art.CreatePen(PS_DOT,Cursor[1].width,Cursor[1].color);
			dc->SelectObject(art);
			tp[0].x = left; tp[0].y = Cursor[1].pos;
			tp[1].x = left+width; tp[1].y = Cursor[1].pos;
			dc->Polyline(tp,2);
			art.DeleteObject();

			art.CreatePen(PS_DOT,Cursor[3].width,Cursor[3].color);
			dc->SelectObject(art);
			tp[0].x = Cursor[3].pos; tp[0].y = top;
			tp[1].x = Cursor[3].pos; tp[1].y = top+height;
			dc->Polyline(tp,2);
			art.DeleteObject();

			if (showInfo)
			{
				art.CreatePen(PS_SOLID,1,RGB(0,0,0));
				dc->SelectObject(art);

				rc.left = left+width+15; rc.right=rc.left+(LONG)(0.9*infowidth);
				rc.top = infoy;rc.bottom=rc.top+100*yline/15;
				fil.CreateSolidBrush(RGB(255,255,255));
				//dc->FillRect(&rc,&fil);
				dc->Rectangle(&rc);
				fil.DeleteObject();

				fnt.CreateFont(fnth,0,0,0,0,0,0,0,0,0,0,0,10,_T("Tremuchet MS"));
				dc->SetTextColor(lblCol);
				dc->SelectObject(&fnt);

				// tp[i].y = top + height/2 - trace[iTrace].data[i]*height/51200;
				// y = -4*deltay + y0  where deltay is the vertical scale / 8
				// 8 * dt is x-axis

				rc.left = left+width+18; rc.right=rc.left;
				rc.top = infoy+3;rc.bottom=rc.top;
				dc->DrawText(_T("Cursor info:"),12,&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;

				double t1 = (double)xgrid * (Cursor[2].pos - left) / width * dt;
				double y1 = (double)ygrid * ((height - ((float)Cursor[0].pos - top)) / height - 0.5) * deltay + y0; 
				rc.top = infoy;rc.bottom=rc.top;
				sprintf_s(valstr,100,"X1 = %3.2f - Y1 = %3.2f\r\n", t1, y1);
				dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;

				double t2 = (double)xgrid * (Cursor[3].pos - left) / width * dt;
				double y2 = (double)ygrid * ((height - ((float)Cursor[1].pos - top)) / height - 0.5) * deltay + y0; 
				rc.top = infoy;rc.bottom=rc.top;
				sprintf_s(valstr,100,"X2 = %3.2f - Y2 = %3.2f\r\n", t2,y2);
				dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;

				double dt = t2 - t1;
				double f = 1/dt;
				rc.top = infoy;rc.bottom=rc.top;rc.left += 10;
				char funit[10];
				if (strcmp(tunit,"ms")==0) strcpy_s(funit,10,"kHz");
				else if (strcmp(tunit,"us")==0) strcpy_s(funit,10,"MHz");
				else if (strcmp(tunit,"s")==0) strcpy_s(funit,10,"Hz");
				else if (strcmp(tunit,"ns")==0) strcpy_s(funit,10,"GHz");
				sprintf_s(valstr,100,"T = %3.2f %s  Freq = %3.2f %s\r\n",dt,tunit,f,funit);
				CString csvalstr = CString(valstr);   
				dc->DrawText(csvalstr,(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;
				rc.left -= 10;
				POINT pnt[4];
				pnt[0].x = rc.left + 1; pnt[0].y = rc.top + 9;
				pnt[1].x = rc.left + 4; pnt[1].y = rc.top + 2;
				pnt[2].x = rc.left + 7; pnt[2].y = rc.top + 9;
				pnt[3].x = rc.left + 1; pnt[3].y = rc.top + 9;
				dc->Polyline((POINT*)&pnt,4);

				double dy = y2 - y1;
				rc.top = infoy;rc.bottom=rc.top;rc.left += 10;
				sprintf_s(valstr,100,"V = %3.2f\r\n",dy);
				dc->DrawText(CString(valstr),(int)strlen(valstr),&rc,DT_NOCLIP | DT_LEFT);
				infoy += yline;
				rc.left -= 10;
				pnt[0].x = rc.left + 1; pnt[0].y = rc.top + 9;
				pnt[1].x = rc.left + 4; pnt[1].y = rc.top + 2;
				pnt[2].x = rc.left + 7; pnt[2].y = rc.top + 9;
				pnt[3].x = rc.left + 1; pnt[3].y = rc.top + 9;
				dc->Polyline((POINT*)&pnt,4);

					
				fnt.DeleteObject();
				art.DeleteObject();
			}
		}
		if ((zooming) && (zoomA.x>0))
		{
			tviewTmin = (double)(min(zoomA.x,zoomB.x)-left)/width*viewTmax;
			tviewTmax = (double)(max(zoomA.x,zoomB.x)-left)/width*viewTmax;
			TRACE2("Time: [%1.2f, %1.2f]\r\n",tviewTmin,tviewTmax);
			tviewVmin = viewVmax - (double)(max(zoomA.y,zoomB.y)-top)/height*(viewVmax-viewVmin);
			tviewVmax = viewVmax - (double)(min(zoomA.y,zoomB.y)-top)/height*(viewVmax-viewVmin);
			TRACE2("Volt: [%1.2f, %1.2f]\r\n",tviewVmin,tviewVmax);

			art.CreatePen(PS_DASH,1,RGB(255,0,0));
			dc->SelectObject(art);
			dc->MoveTo(zoomA);
			dc->LineTo(zoomA.x,zoomB.y);
			dc->LineTo(zoomB);
			dc->LineTo(zoomB.x,zoomA.y);
			dc->LineTo(zoomA);
			art.DeleteObject();
		}
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPM3394ADlg::OnPaint() 
{
	CFont fnt;
	CBrush fil;
	
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // device context for painting

		RECT wrc,rc;
		GetWindowRect(&wrc);
		
		rc.right=wrc.right-wrc.left;
		rc.left=0;
		rc.top=0;
		rc.bottom=(wrc.bottom-wrc.top)/14;
		rc.bottom = 50;
		fil.CreateSolidBrush(FLUKE_BG);
		dc.FillRect(&rc,&fil);
		fil.DeleteObject();

		rc.top=10;rc.left=20;
		dc.SetBkMode(TRANSPARENT);
		float w = rc.right*16.0F/640.0F;
		fnt.CreateFont(rc.bottom-20,(int)w,0,0,600,0,0,0,0,0,0,0,12,_T("Arial Bold"));
		dc.SelectObject(&fnt);
		dc.DrawText(_T("FLUKE PM3394B SERIES COMBISCOPE"),31,&rc,0);
		fnt.DeleteObject();

		Draw(&dc,IDC_PLOT);	
		myUpdate = false;
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPM3394ADlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPM3394ADlg::OnAutoSetup() 
{
	unsigned long nc;
	WriteFile(hCommPort,"AS\r",3,&nc,NULL);	
}


void CPM3394ADlg::OnTimer(UINT nIDEvent) 
{

	return;

	// hopefully 5 characters is enough
	char d[1000];
	unsigned long nc;

	ReadFile(hCommPort, d, 1, &nc, NULL);
	if (nc>0) {
		data += d;
		timeout = 0;
		strcpy_s(d,1000,"PM3394AB: Receiving (");
		_itoa_s(nc,&(d[strlen(d)]),1000,10);
	} else {
		timeout++;
		strcpy_s(d,1000,"PM3394AB: Connected.");
	}
	
	SetDlgItemText(IDC_STATUS,CString(d));

	if (timeout>5000) {
		if (data.GetLength()>100)
		{
			// save to disk or send to printer
		}
	}
}

void CPM3394ADlg::OnClose() 
{
	BOOL fSuccess;
	if (hCommPort != INVALID_HANDLE_VALUE) {
		PurgeComm(hCommPort, PURGE_TXABORT | PURGE_RXABORT);
		fSuccess = CloseHandle(hCommPort);
	}	
	CDialog::OnClose();
}

void CPM3394ADlg::OnHp() 
{
	char hpgldata[100000], ccmd[200];
	unsigned long nc, fnc, s;
	RECT rc;
	DWORD rtn = 0;

	//trace[0].ntrace = 0;

	// Request HPGL hardcopy data
	// Note: should make sure that PM3394 sends in HPGL format.
	FlushFileBuffers(hCommPort);
	WriteFile(hCommPort,_T("QP1\r"),4,&nc,NULL);
	Sleep(1000);
	ReadFile(hCommPort,hpgldata,100000,&nc,NULL);

	// Store HPGL data to file
	HANDLE hFile = CreateFile(_T("pm33xx.hgl"),GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) return;
	WriteFile(hFile,hpgldata,nc,&fnc,NULL);
	CloseHandle(hFile);

	// get window width
	pPlotBox->GetWindowRect(&rc);
	// Convert HPGL file to PNG
	// Uses external command hp2xx.exe
	s = (rc.right-rc.left)/2;
	//sprintf_s(ccmd,200,".\\..\\HPGL\\hp2xx.exe -m png -r270 -h%i -w%i pm33xx.hgl",s,s);
	//system(ccmd);
	//system(".\\..\\HPGL\\hp2xx.exe -m png -r270 -h1000 -w1000 pm33xx.hgl");
	sprintf_s(ccmd,200, "-m png -r270 -h%i -w%i pm33xx.hgl",s,s);
	SHELLEXECUTEINFO sexi = {0};
	sexi.cbSize = sizeof(SHELLEXECUTEINFO);
	sexi.hwnd = m_hWnd;
	sexi.fMask = SEE_MASK_NOCLOSEPROCESS;
	sexi.lpFile = _T(".\\..\\HPGL\\hp2xx.exe");
	sexi.lpParameters = CString(ccmd);
	sexi.nShow = SW_HIDE;
	if(ShellExecuteEx(&sexi))
	{
		DWORD wait = WaitForSingleObject(sexi.hProcess, INFINITE);
		if(wait == WAIT_OBJECT_0) GetExitCodeProcess(sexi.hProcess, &rtn);
	}

	// load PNG bitmap file
	if (img.Load(_T("pm33xx.png"))==NULL) 
		hBmp = img.Detach();

	// Display bitmap in Dialog Window
	Invalidate();
	UpdateWindow();
}

void CPM3394ADlg::GetTrace(int iTrace)
{
	char tdata[10000];
	unsigned long nc;
	int j,i=0;
	short *sdata;
	char cmd[10];
	int rtn;
	int traceGroup = iTrace / 4;
	int traceRegister = iTrace - 4 * traceGroup + 1;

	if (iTrace<4)
		sprintf_s(cmd,10,"QW%i\r",traceRegister);
	else
		sprintf_s(cmd,10,"QW%i%i\r",traceGroup, traceRegister);
	FlushFileBuffers(hCommPort);
	WriteFile(hCommPort,cmd,(int)strlen(cmd),&nc,NULL);
	Sleep(100);
	rtn = ReadFile(hCommPort,tdata,1,&nc,NULL);
	if (nc==1) if (tdata[0]==13)
		rtn = ReadFile(hCommPort,tdata,1,&nc,NULL);
	

	if ((tdata[0]!='0') || (nc==0)) return; // error

	ReadFile(hCommPort,tdata,10000,&nc,NULL);

	// interpret data
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

	sdata = (short*)&tdata[i-1];
	if (trace[iTrace].ntrace>10000) 
	{
		trace[iTrace].ntrace=0;
	} else {
		for (j=0;j<trace[iTrace].ntrace;j++)
			trace[iTrace].data[j]=sdata[j];
	}
	dt = trace[iTrace].x_res*trace[iTrace].ntrace/10.24;
	strcpy_s(tlabel,20,"Time [s]");
	if (trace[iTrace].x_res<0.01) {dt *= 1000.0F; strcpy_s(tlabel,20,"Time [ms]");}
	if (trace[iTrace].x_res<1e-6) {dt *= 1000.0F; strcpy_s(tlabel,20,"Time [us]");}
	y0 = trace[iTrace].y_zero;
	deltay = trace[iTrace].y_res*6400;
	viewVmax = ygrid/2.0*deltay;
	viewVmin = ygrid/-2.0*deltay;
	viewTmin = 0.0;
	viewTmax = trace[iTrace].ntrace*dt;
}

void CPM3394ADlg::OnBnClickedGetTraces()
{
	CButton *tcb;
	for (int iTrace=0;iTrace<32;iTrace++)
	{
		trace[iTrace].ntrace = 0;
		tcb = (CButton*)cfgDlg->GetDlgItem(1100+iTrace);
		if (tcb->GetCheck())
			GetTrace(iTrace);
	}

	DeleteObject(hBmp);
	hBmp = 0;

	Invalidate();
	UpdateWindow();
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
}

void CPM3394ADlg::OnBnClickedHelpme()
{
	OnHelp();
}

void CPM3394ADlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,SBID[4]);
	//pPlotBox->SetWindowPos(&wndTop,12,97,pRect->right-pRect->left-33,pRect->bottom-pRect->top-165,0);
	pPlotBox->SetWindowPos(&wndTop,12,97,pRect->right-pRect->left-41,pRect->bottom-pRect->top-169,0);
	Invalidate();
	UpdateWindow();
}

#define CPYCLEN 10000
#define CPYLEN 10000
void CPM3394ADlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CMetaFileDC mfdc;
	CDC cdc, bdc;
	CBitmap bitmap;
	CBrush wndBrush;
	CRect wndrect;
	
	wndBrush.CreateSolidBrush(RGB(255,255,255));
	CClientDC dc = CClientDC(this);
	GetClientRect(&wndrect);
	int w=wndrect.Width(), h=wndrect.Height();
	if( mfdc.CreateEnhanced(NULL,NULL,NULL,_T("FLUKE\0COMBISCOPE\0\0")) )
	{
		VERIFY(cdc.CreateIC(__TEXT("DISPLAY"),NULL,NULL,NULL));
		mfdc.SetAttribDC(cdc.m_hAttribDC);
		myUpdate = false;
		Draw(&mfdc,1);
		// in DEBUG mode this call throws ASSERT exceptions because m_hDC and m_hAttribDC are not equal.
		// in RELEASE mode this works OK.
		// to avoid the exception in DEBUG mode, replace all DC-dependent calls to GDI functions
		// global as in ::SelectObject(m_hDC, hObject)   is a replacement for cdc.SelectObject(hObject)
	
		bdc.CreateCompatibleDC(this->GetDC());
		bitmap.CreateCompatibleBitmap(this->GetDC(),w,h);
		bdc.SelectObject ( &bitmap );
		bdc.FillRect(wndrect,&wndBrush);
		myUpdate = false;
		Draw(&bdc,0);

		HENHMETAFILE hmf;
		/*
		HLOCAL hTrace;
		char pTrace[CPYLEN];
		char pTraceLine[100];
		sprintf_s(pTrace,CPYLEN,"Radio ID\t%i\r\nEdge\t%i\r\nLock\t%i\r\n",Scan.ID,Scan.Edge,Scan.Lock );
		for (int iD=0;iD<Scan.Len;iD++)
		{
			sprintf_s(pTraceLine,100,"%i\t%i\r\n",iD,Scan.Data[iD]);
			strcat_s(pTrace,CPYLEN,pTraceLine);
		}
		hTrace = LocalAlloc(LPTR,strlen(pTrace)+1);
		memcpy(hTrace,pTrace,strlen(pTrace)+1);
		*/	

		/*
		HLOCAL hTrace;
		char pTrace[CPYCLEN];
		char pTraceLine[100];
		
		sprintf_s(pTrace,CPYLEN,"R1\tR2\tR3\tR4\tR5\tR6\tR7\tR8\r\n");
		for (int id=0;id<ncdata;id++) 
		{
			for (int ic=0;ic<CHART_COLS;ic++)
			{
				sprintf_s(pTraceLine,100,"%f\t",cdata[ic][id]);
				strcat_s(pTrace,CPYCLEN,pTraceLine);
			}
			strcat_s(pTrace,CPYCLEN,"\r\n");
		}
		hTrace = LocalAlloc(LPTR,strlen(pTrace)+1);
		memcpy(hTrace,pTrace,strlen(pTrace)+1);
		*/
		if( (hmf = mfdc.CloseEnhanced()) )
		{
			if( OpenClipboard() )
			{
				EmptyClipboard();
				//SetClipboardData (CF_TEXT, hTrace);
				SetClipboardData (CF_BITMAP, bitmap.GetSafeHandle() ) ;
				SetClipboardData (CF_ENHMETAFILE, hmf);
				CloseClipboard();
			}
			else
			{
				//The metafile is deleted only when it has not been set in
				//the clipboard.
				
				::DeleteEnhMetaFile(hmf);
			}
		}

	}
	mfdc.DeleteDC();

	wndBrush.DeleteObject();
	bdc.DeleteDC();
	bitmap.DeleteObject();

	CDialog::OnLButtonDblClk(nFlags, point);
}

BOOL CPM3394ADlg::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{ 
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR; 
    HWND hID = (HWND)pNMHDR->idFrom;
	UINT nID = ::GetDlgCtrlID(hID);
    //TRACE2("Tooltip for %i in window %i\r\n",nID,hID);
	switch (nID)
	{
	case IDC_HP: _tcscpy_s(pTTT->szText,80, _T("Create a hardcopy of the screen from the Combiscope"));break;
	case IDC_TRACE: _tcscpy_s(pTTT->szText,80, _T("Copy the waveforms from the Combiscope"));break;
	case IDC_AUTOSETUP: _tcscpy_s(pTTT->szText,80, _T("Auto setup"));break;
	case IDC_GETSETUP: _tcscpy_s(pTTT->szText,80, _T("Retrieve the current setup from the oscilloscope."));break;
	case IDC_HELPME: _tcscpy_s(pTTT->szText,80, _T("Online Help"));break;
	case IDC_LINK: _tcscpy_s(pTTT->szText,80, _T(""));break;
	case IDC_CURSORS: _tcscpy_s(pTTT->szText,80, _T("Activate cursors in the plot window"));break;
	case IDC_CONFIG: _tcscpy_s(pTTT->szText,80, _T("Configure settings for communication and user interface"));break;
	case IDC_CONNECT: _tcscpy_s(pTTT->szText,80, _T("Connect to the oscilloscope"));break;
	case IDC_COPY: _tcscpy_s(pTTT->szText,80, _T("Copy the current graphics to clipboard"));break;
	case IDOK: _tcscpy_s(pTTT->szText,80, _T("Close Flukeview"));break;
	case IDC_PRINT: _tcscpy_s(pTTT->szText,80, _T("Print the current graphics"));break;
	case IDC_EXCEL: _tcscpy_s(pTTT->szText,80, _T("Store traces to Excel file or load trace from Excel file"));break;
	case IDC_READFILE: _tcscpy_s(pTTT->szText,80, _T("Read traces from file"));break;
	case IDC_SAVEFILE: _tcscpy_s(pTTT->szText,80, _T("Store traces to file"));break;
	default : return FALSE;
    }
    return TRUE;
}


void CPM3394ADlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Check if cursors are on, and close to one of the cursors
	int e = 1000;
	if (zooming)
	{
		zoomA = point;
		zoomB = point;
		tviewTmin = viewTmin;
		tviewTmax = viewTmax;
		tviewVmin = viewVmin;
		tviewVmax = viewVmax;
	} else {
		if (abs(point.y-Cursor[0].pos)<e) {e = abs(point.y-Cursor[0].pos);ic = 0;}
		if (abs(point.y-Cursor[1].pos)<e) {e = abs(point.y-Cursor[1].pos);ic = 1;}
		if (abs(point.x-Cursor[2].pos)<e) {e = abs(point.x-Cursor[2].pos);ic = 2;}
		if (abs(point.x-Cursor[3].pos)<e) {e = abs(point.x-Cursor[3].pos);ic = 3;}
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CPM3394ADlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// release the cursor movement
	if (zooming)
	{ 
		viewTmin = tviewTmin;
		viewTmax = tviewTmax;
		viewVmin = tviewVmin;
		viewVmax = tviewVmax;
		zoomA.x=0;zoomA.y=0;
		zoomB.x=0;zoomB.y=0;
		myUpdate = true;
		Invalidate();
		UpdateWindow();
		zooming = false;
		//((CButton*)GetDlgItem(IDC_ZOOM))->SetCheck(0);
	} else {
		ic = -1;
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CPM3394ADlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// If cursor movement, then set the active cursor to the current mouse location
	if (zooming)
	{
		zoomB = point;
		myUpdate = true;
		Invalidate();
		UpdateWindow();
	} else {
		if (ic>=0)
		{
			if (ic<2) Cursor[ic].pos=point.y; else Cursor[ic].pos=point.x;
			myUpdate = true;
			Invalidate();
			UpdateWindow();
		}
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

BOOL CPM3394ADlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: for zooming

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CPM3394ADlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: For Context Menu

	CDialog::OnRButtonDown(nFlags, point);
}

void CPM3394ADlg::OnBnClickedCursors()
{
	Invalidate();
	UpdateWindow();
}

BOOL CPM3394ADlg::OnEraseBkgnd(CDC* pDC)
{
	if (myUpdate)
		return FALSE;
	else
		return CDialog::OnEraseBkgnd(pDC);
}

void CPM3394ADlg::OnBnClickedConfig()
{
	
	cfgDlg->ShowWindow(SW_SHOW);
	cfgDlg->OnCbnSelchangeConfigelement();
}

void CPM3394ADlg::OnBnClickedConnect()
{
	Connect((int)pCom->GetItemData(pCom->GetCurSel()));
}

void CPM3394ADlg::OnBnClickedCopy()
{
	OnLButtonDblClk(0,NULL);
}

void CPM3394ADlg::OnBnClickedPrint()
{
	char txt[1000];
	CDC prnDC, memDC;
	CFont prnFont;
	CPen prnPen;
	RECT rt;

	CPrintDialog pdlg(FALSE);

	if (pdlg.DoModal() != IDOK) return;

	if( !prnDC.Attach(pdlg.GetPrinterDC()) )
		AfxMessageBox("Invalid Printer DC");
	
	int iPrnX = prnDC.GetDeviceCaps(HORZRES);
	int iPrnY = prnDC.GetDeviceCaps(VERTRES);
	
	sprintf_s(txt,1000,"Line Puzzel");
	prnDC.StartDoc(txt);
	prnDC.StartPage();
	
	// print map
	Draw(&prnDC, 0);
	
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
	prnDC.SetTextColor(RGB(180,180,180));
	prnFont.CreateFont(80,0,0,0,0,0,0,0,0,0,0,0,10,"Tahoma");
	prnDC.SelectObject(&prnFont);
	rt.left=iPrnX / 2; rt.top=iPrnY - 150; rt.right=rt.left; rt.bottom=rt.top;
	prnDC.DrawText("Edzko Smid",11,&rt,DT_NOCLIP | DT_CENTER);
	prnFont.DeleteObject();
	
	prnDC.EndPage();
	prnDC.EndDoc();
	
	prnFont.DeleteObject();
	prnPen.DeleteObject();
	prnDC.Detach();
}

void CPM3394ADlg::OnBnClickedInfo()
{
	Invalidate();
	UpdateWindow();
}

void CPM3394ADlg::OnBnClickedDisk()
{
	FILE *storeFile;
	int nSuccess;

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

	lfn.lpstrTitle        = _T("Select Filename");
	lfn.lpstrFilter       = _T("Trace Files (*.trc)\0*.trc\0All Files (*.*)\0*.*\0\0");
	lfn.lpstrInitialDir   = _T("");
	lfn.nFilterIndex      = 1;
	lfn.lpstrFile         = (LPSTR)DefFullName;
	lfn.nMaxFile          = 100;
	lfn.lpstrFileTitle    = (LPSTR)DefName;
	lfn.nMaxFileTitle     = 100;
	// if data was loaded, then save it, otherwise read it
	
		nSuccess = GetSaveFileName(&lfn);
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
			fclose(storeFile);
		} 
		/*
		else {
			DWORD nError = CommDlgExtendedError();
			TRACE1("Error %X",nError);
			if (nError == CDERR_DIALOGFAILURE)  {};
		}
		*/
}

void CPM3394ADlg::OnBnClickedDiskRead()
{
	FILE *storeFile;
	int nSuccess;

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

	lfn.lpstrTitle        = _T("Select Filename");
	lfn.lpstrFilter       = _T("Trace Files (*.trc)\0*.trc\0All Files (*.*)\0*.*\0\0");
	lfn.lpstrInitialDir   = _T("");
	lfn.nFilterIndex      = 1;
	lfn.lpstrFile         = (LPSTR)DefFullName;
	lfn.nMaxFile          = 100;
	lfn.lpstrFileTitle    = (LPSTR)DefName;
	lfn.nMaxFileTitle     = 100;
	// if data was loaded, then save it, otherwise read it
	nSuccess = GetOpenFileName(&lfn);
	if (nSuccess)
	{
		if (strchr((char*)lfn.lpstrFile,'.')==NULL)
			strcat_s((char *)lfn.lpstrFile,100,".trc");
		int nResult = fopen_s(&storeFile,(const char*)lfn.lpstrFile,(const char*)"rb");  // write;binary;commit
		if (nResult)
		{
			TRACE0("Trace File Open problem.\r\n");
			return;
		}
		fread(trace,sizeof(TRACE_T),MAX_TRACE,storeFile);
		fclose(storeFile);

		bool foundfirsttrace = false;
		for (int iTrace = 0;iTrace<MAX_TRACE;iTrace++)
		{
			if (trace[iTrace].ntrace>0) 
			{
				if (!foundfirsttrace)
				{
					dt = trace[iTrace].x_res*trace[iTrace].ntrace/10.24;
					strcpy_s(tunit,20,"s");
					if (trace[iTrace].x_res<0.01) {dt *= 1000.0F; strcpy_s(tunit,20,"ms");}
					if (trace[iTrace].x_res<1e-6) {dt *= 1000.0F; strcpy_s(tunit,20,"us");}
					strcpy_s(tlabel,20,"Time [");strcat_s(tlabel,20,tunit);strcat_s(tlabel,20,"]");
					y0 = trace[iTrace].y_zero;
					deltay = trace[iTrace].y_res*6400*0.5;
					foundfirsttrace = true;
					viewVmax = ygrid/2.0*deltay;
					viewVmin = ygrid/-2.0*deltay;
					viewTmin = 0.0;
					viewTmax = xgrid*dt;
				}
				((CButton*)cfgDlg->GetDlgItem(1100+iTrace))->SetCheck(true);
			} 
			else
				((CButton*)cfgDlg->GetDlgItem(1100+iTrace))->SetCheck(false);

		}
		hBmp = NULL;
		Invalidate();
		UpdateWindow();
	} 
}

void CPM3394ADlg::OnBnClickedExcel()
{
	// TODO: Add your control notification handler code here
}

void CPM3394ADlg::OnBnClickedTrigger()
{
	unsigned long nc;
	WriteFile(hCommPort,"AT\r",3,&nc,NULL);	
}

void CPM3394ADlg::OnBnClickedZoom()
{
	if (((CButton*)GetDlgItem(IDC_ZOOM))->GetCheck())
	{
		zooming = true;
	} else {
		zooming = false;  // reset zoom
		viewVmax = ygrid/2.0*deltay;
		viewVmin = ygrid/-2.0*deltay;
		viewTmin = 0.0;
		viewTmax = xgrid*dt;		
		myUpdate = false;
		Invalidate();
		UpdateWindow();
	}
}
