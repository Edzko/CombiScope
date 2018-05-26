// Config.cpp : implementation file
//

#include "stdafx.h"
#include "Config.h"
#include "PM3394ADlg.h"
#include "PM3394A.h"

CPM3394ADlg *pDlg;

// CConfig dialog

IMPLEMENT_DYNAMIC(CConfig, CDialog)

CConfig::CConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CConfig::IDD, pParent)
{
	pDlg = (CPM3394ADlg*)pParent;
	
	//Trace 1;Trace 2;Trace 3;Trace 4;Trace 5;Trace 6;Trace 7;Trace 8;Trace 9;Trace 10;
	//Grid;	Cursor X1;Cursor X2;Cursor Y1;Cursor Y2;Labels;Background;

	
}

CConfig::~CConfig()
{
	pButMap.DeleteObject();
	pUnused.DeleteObject();
	pcbDC.DeleteDC();
	cbg.DeleteObject();
}

void CConfig::SetParent(CWnd *_dlg)
{
	
}


void CConfig::GetActiveItems(UINT id)
{
	char ts[50];
	nrefCol = 0;
	pItem->ResetContent();
	for (int i=0;i<36;i++)
	{
		int k = i / 4;
		if (((CButton*)GetDlgItem(1100+i))->GetCheck())
		{
			refCol[nrefCol++] = &pDlg->trace[i].color;
			if (i<4) sprintf_s(ts,50,"Trace Acq %i",i+1);
			else sprintf_s(ts,50,"Trace Mem %i.%i",k,i-4*k+1);
			pItem->AddString(CString(ts));
		}
	}
	
	for (int i=0;i<4;i++)
		refCol[nrefCol++] = &pDlg->Cursor[i].color;
	pItem->AddString(_T("Cursor X1"));
	pItem->AddString(_T("Cursor X2"));
	pItem->AddString(_T("Cursor Y1"));
	pItem->AddString(_T("Cursor Y2"));

	refCol[nrefCol++] = &pDlg->gridCol;
	pItem->AddString(_T("Grid lines"));
	refCol[nrefCol++] = &pDlg->lblCol;
	pItem->AddString(_T("Axes Labels"));
	refCol[nrefCol++] = &pDlg->bgCol;
	pItem->AddString(_T("Background"));

	pItem->SetCurSel(0);

	pDlg->Invalidate();
	pDlg->UpdateWindow();
}

BOOL CConfig::OnInitDialog()
{
	CDialog::OnInitDialog();

	pItem = (CComboBox*)GetDlgItem(IDC_CONFIGELEMENT);
	for(int i=1100;i<1104;i++) ((CButton*)GetDlgItem(i))->SetCheck(true);
	GetActiveItems(0);

	CDC *dc = GetDC();
	pButMap.CreateCompatibleBitmap(dc,190,100);
	pUnused.CreateCompatibleBitmap(dc,190,100);
	hbmBUT = (HBITMAP)pButMap.GetSafeHandle();
	pcbDC.CreateCompatibleDC(GetDC());


	pBaud = (CComboBox*)GetDlgItem(IDC_BAUDRATE);
	int baud = AfxGetApp()->GetProfileInt(_T(""),_T("Baud"),1);
	pBaud->SetCurSel(baud);

	pWidth = (CComboBox*)GetDlgItem(IDC_TRACEWIDTH);


	return TRUE;
}

void CConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CConfig, CDialog)
	ON_BN_CLICKED(IDC_COLORBUTTON, &CConfig::OnBnClickedColorbutton)
	ON_CBN_SELCHANGE(IDC_CONFIGELEMENT, &CConfig::OnCbnSelchangeConfigelement)
	ON_CBN_SELCHANGE(IDC_BAUDRATE, &CConfig::OnCbnSelchangeBaudrate)
	ON_CBN_SELCHANGE(IDC_TRACEWIDTH, &CConfig::OnCbnSelchangeTracewidth)
	ON_COMMAND_RANGE(1100,1136,GetActiveItems)
END_MESSAGE_MAP()

void CConfig::OnBnClickedColorbutton()
{
	int _i = pItem->GetCurSel();
	CHOOSECOLOR chc;
	chc.Flags = CC_SOLIDCOLOR | CC_FULLOPEN | CC_PREVENTFULLOPEN | CC_RGBINIT;
	chc.lStructSize = sizeof(chc);
	chc.hwndOwner = NULL;
	chc.lCustData = NULL;
	chc.lpfnHook = NULL;
	chc.hInstance = NULL;
	chc.lpCustColors = pDlg->cc;
	chc.lpTemplateName = NULL;
	//
	chc.rgbResult = *refCol[_i];
	if (ChooseColor(&chc))
	{
		*refCol[_i] = chc.rgbResult;

		// store new color in reg profile
		char ps[10];
		if (_i>pItem->GetCount()-8) _i += 44 - pItem->GetCount();
		sprintf_s(ps,10,"TC%i",_i);
		AfxGetApp()->WriteProfileInt(_T(""),CString(ps),(int)chc.rgbResult);
		
		pDlg->Invalidate();
		pDlg->UpdateWindow();
	}
	OnCbnSelchangeConfigelement();
}

void CConfig::OnCbnSelchangeConfigelement()
{
	RECT rc;
	CButton *pB;
	
	int _i = pItem->GetCurSel();
	TRACE1("Selection=%i\r\n",_i);

	if (_i<36) 
	{
		pWidth->EnableWindow(true); 
		pWidth->SetCurSel(pDlg->trace[_i].width-1);
	} else if (_i<40) 
	{
		pWidth->EnableWindow(true); 
		pWidth->SetCurSel(pDlg->Cursor[_i-36].width-1);
	}
	else 
		pWidth->EnableWindow(false);

	cbg.DeleteObject();
	cbg.CreateSolidBrush(*refCol[_i]);
	
	rc.left=0;rc.top=0;rc.bottom=100;rc.right=190;
	
	pcbDC.SelectObject(&pButMap);
	pcbDC.FillRect(&rc,&cbg);
	pcbDC.SelectObject(&pUnused);

	pB = (CButton*)GetDlgItem(IDC_COLORBUTTON);
	pB->SetBitmap(hbmBUT); 
}

void CConfig::OnCbnSelchangeBaudrate()
{
	int bd[4] = {4800,9600,19200,38400};
	int ibd = pBaud->GetCurSel();
	AfxGetApp()->WriteProfileInt(_T(""),_T("Baud"),ibd);
	pDlg->Baud = bd[ibd];
}

void CConfig::OnCbnSelchangeTracewidth()
{
	int _i = pItem->GetCurSel();
	if (_i<36)
		pDlg->trace[_i].width = pWidth->GetCurSel()+1;
	else 
		pDlg->Cursor[_i-36].width = pWidth->GetCurSel()+1;
	pDlg->Invalidate();
	pDlg->UpdateWindow();
}
