typedef struct
{
	char y_unit[10],x_unit[10],tdate[20],ttime[20],trace_id[10];
	float x_zero, y_zero,x_res, y_res, delta_t;
	double Vpp,Vmin,Vmax,tpulse,tperiod,freqprim;
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

#define MAX_TRACE  36
#define MAX_FILENAME 250
#define FLUKE_BG  RGB(0xFF,0xC0,0)
#define CPYLEN 1000000

extern HINSTANCE hInst;								// current instance
extern HWND m_hWnd;
extern HWND m_hStatusWnd; // Status Bar Window Handle

extern int timeout;
extern char tlabel[20], tunit[20];
extern HANDLE hCommPort;
extern DCB	dcbCommPort;
extern COMMTIMEOUTS CommTimeouts;
extern DWORD baudRate;			// current baud rate
extern BYTE byteSize, parity, stopBits;			
extern double t, dt, y, y00, deltay;
extern CString  data;
extern HBITMAP hBmp;
extern TRACE_T trace[MAX_TRACE];
extern int cdata[35000], mdata[65000];
extern int npoints;
extern int wndFcn;
extern CURSOR_T Cursor[4];
extern int ic; // selected cursor index
extern int it; // selected trace index
extern int xgrid, ygrid;
extern int width, infowidth;
extern bool myUpdate, zooming, zoomMode;
extern bool fftMode, xLog, yLog;
extern CPoint zoomA, zoomB;
extern double viewVmin,viewVmax,viewTmin,viewTmax;	// zoomed window
extern double tviewVmin,tviewVmax,tviewTmin,tviewTmax; // temporary values while zooming
extern bool showInfo;
extern int cursorFcn;
extern COLORREF cc[16], gridCol, lblCol, bgCol;

extern char DefFullName[MAX_FILENAME], DefName[MAX_FILENAME];
extern OPENFILENAME lfn;

extern void GetTrace(int iTrace);
extern void Draw(HDC dc, int CID);
extern BOOL InitCommPort(int CommPortNum, int baud,BYTE ByteSize, BYTE Parity, BYTE StopBits);
extern void GetCOMPorts(CComboBox *pCOM);
extern void Connect(int port, int baud);
extern void Init();
extern void OpenTraceFile(BOOL ask);
extern void SaveTraceFile(BOOL ask);
extern void SaveAsCSV();
extern void RegisterMru(char *fn);
extern void Print();
extern void ResetZoom();
extern void getCursorPos();
extern void UpdateStatus(void);
extern bool GetReg(char *name, char *key, char *buffer, int *buflen, int regtype);
extern bool SetReg(char *name, char *key, char *val, int buflen, int regtype);
extern void Spectrum(int _trace);
extern void SetTraceData(int n);


#define YEAR ((((__DATE__ [7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))
#define MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
              : __DATE__ [2] == 'b' ? 2 \
              : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
              : __DATE__ [2] == 'y' ? 5 \
              : __DATE__ [2] == 'l' ? 7 \
              : __DATE__ [2] == 'g' ? 8 \
              : __DATE__ [2] == 'p' ? 9 \
              : __DATE__ [2] == 't' ? 10 \
              : __DATE__ [2] == 'v' ? 11 : 12)
#define DAY ((__DATE__ [4]==' ' ? 0 : __DATE__[4]-'0')*10+(__DATE__[5]-'0'))
#define COMPILE_HOUR (((__TIME__[0]-'0')*10) + (__TIME__[1]-'0') + (__TIME__[6]=='P' ? 12 : 0))
#define COMPILE_MINUTE (((__TIME__[3]-'0')*10) + (__TIME__[4]-'0'))
#define COMPILE_SECOND (((__TIME__[6]-'0')*10) + (__TIME__[7]-'0'))
