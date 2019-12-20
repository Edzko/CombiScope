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

// PM33XX_MFC.h : main header file for the PM33XX_MFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CPM33XXMFCApp:
// See PM33XX_MFC.cpp for the implementation of this class
//

class CPM33XXMFCApp : public CWinAppEx
{
public:
	CPM33XXMFCApp() noexcept;
	void GetMRU(int imru, char *fname);

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);
};

extern CPM33XXMFCApp theApp;

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
