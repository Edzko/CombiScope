#pragma once

#include "resource.h"
// CConfig dialog

class CConfig : public CDialog
{
	DECLARE_DYNAMIC(CConfig)
	CBrush cbg;
	CBitmap pButMap, pUnused;
	CComboBox *pItem, *pBaud, *pWidth;
	COLORREF *refCol[50];
	int nrefCol;
	CDC pcbDC;
	HBITMAP hbmBUT;
public:
	CConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfig();
	void SetParent(CWnd *pDlg);
	void GetActiveItems(UINT id);
// Dialog Data
	enum { IDD = IDD_CONFIGURATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedColorbutton();
	afx_msg void OnCbnSelchangeConfigelement();
	afx_msg void OnCbnSelchangeBaudrate();
public:
	afx_msg void OnCbnSelchangeTracewidth();
};
