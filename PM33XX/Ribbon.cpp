// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <afxwin.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlhost.h>
#include <atlimage.h>
#include <afxsock.h>
#include <Commdlg.h>
#include <atlimage.h>
#include <initguid.h>
#include <uiribbon.h>
#include <htmlhelp.h>
#include <Setupapi.h>
#include <Devguid.h>
#include <UIRibbonPropertyHelpers.h>
#include "ribbonres.h"
#include "resource.h"
#include "PM33XX.h"
#include "Ribbon.h"
#include "PropertySet.h"
#include "Mru.h"

IUIFramework *g_pFramework = NULL;
CListHandler *pComPortHandler = NULL;
CListHandler *pBaudHandler = NULL;
CListHandler *pCursorHandler = NULL;
CListHandler *pFFTWindowHandler = NULL;
IUnknown *pWlanUnk = NULL, *pWlanCont = NULL;
HWND h_WlanDlg;
int topMargin;


const char BaudRates[5][20] = {"4800","9600","19200","38400"};
int SelectedBaudrate, SelectedComport, ComData[50];

class CApplication
    : public CComObjectRootEx<CComMultiThreadModel>
    , public IUIApplication	
    , public IUICommandHandler
{
public:
    BEGIN_COM_MAP(CApplication)
        COM_INTERFACE_ENTRY(IUIApplication)
        COM_INTERFACE_ENTRY(IUICommandHandler)
    END_COM_MAP()

    STDMETHOD(OnViewChanged)(UINT32 nViewID, __in UI_VIEWTYPE typeID, __in IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode)  
    { 
		HRESULT hr = S_OK;
		
		IUIRibbon* pRibbon;
		int h;
		if ((verb == UI_VIEWVERB_SIZE) && (UI_VIEWTYPE_RIBBON == typeID))
		{
			hr = g_pFramework->GetView(0, IID_PPV_ARGS(&pRibbon));
			hr = pRibbon->GetHeight((UINT32*)&h);
			ATLTRACE2("Ribbon margin = %i\r\n",h);
			topMargin = h;
			InvalidateRect(m_hWnd,NULL,TRUE);
			UpdateWindow(m_hWnd);
		}
        return E_NOTIMPL; 
    }

	STDMETHODIMP CApplication::OnCreateUICommand(UINT32 nCmdID, UI_COMMANDTYPE typeID,
				IUICommandHandler** ppCommandHandler)
	{
		UNREFERENCED_PARAMETER(typeID);
		//ATLTRACE2("Create %i\r\n",nCmdID);
		HRESULT hr = E_FAIL;
		switch (nCmdID)
		{
			case IDM_COMPORTS:
				pComPortHandler = NULL;
				hr = CListHandler::CreateInstance(&pComPortHandler,nCmdID);
				if (SUCCEEDED(hr))
				{
					hr = pComPortHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
					pComPortHandler->Release();
				}
				break;
			case IDM_CURSORS:
				pCursorHandler = NULL;
				hr = CListHandler::CreateInstance(&pCursorHandler,nCmdID);
				if (SUCCEEDED(hr))
				{
					hr = pCursorHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
					pCursorHandler->Release();
				}
				break;
			case IDM_WINDOW:
				pFFTWindowHandler = NULL;
				hr = CListHandler::CreateInstance(&pFFTWindowHandler,nCmdID);
				if (SUCCEEDED(hr))
				{
					hr = pFFTWindowHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
					pFFTWindowHandler->Release();
				}
				break;
			case IDM_BAUD:
				pBaudHandler = NULL;
				SelectedBaudrate = 3;
				hr = CListHandler::CreateInstance(&pBaudHandler,nCmdID);
				if (SUCCEEDED(hr))
				{
					hr = pBaudHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
					pBaudHandler->Release();
				}
				break;
			
			default:
				hr = QueryInterface(IID_PPV_ARGS(ppCommandHandler));  
				break;
		}
		return hr;
	}


    STDMETHOD(OnDestroyUICommand)(UINT32 commandId, __in UI_COMMANDTYPE typeID, __in_opt IUICommandHandler* pCommandHandler) 
    { 
        return E_NOTIMPL; 
    }

    STDMETHODIMP Execute(UINT nCmdID,
        UI_EXECUTIONVERB verb, 
        __in_opt const PROPERTYKEY* key,
        __in_opt const PROPVARIANT* ppropvarValue,
        __in_opt IUISimplePropertySet* pCommandExecutionProperties)
    {
        HRESULT hr = S_OK;

        switch (verb)    
        {  
        case UI_EXECUTIONVERB_EXECUTE:
            switch (nCmdID)
			{
			

			case IDM_HELP:
				::HtmlHelp(m_hWnd,"PM33XX.chm",HH_DISPLAY_TOC,0);
				//::HtmlHelp(m_hWnd,"hlp/ANSManager.chm::/ANS-Manager/ansm_folders.htm",HH_DISPLAY_TOPIC | HH_DISPLAY_TOC,NULL) ;
				break;

			case IDM_PRINT:
				Print();
				break;

			case IDM_OPEN:
				OpenTraceFile(TRUE);
				break;

			case IDM_SAVE:
				SaveTraceFile(FALSE);
				break;

			case IDM_SAVEASANS:
			case IDM_SAVEAS:
				SaveTraceFile(TRUE);
				break;

			case IDM_SAVEASXLS:
				SaveAsCSV();
				break;

			case IDM_SAVEASBMP:
				{
					//HDC mfdc; 
					HDC dc, bdc;
					HBITMAP bitmap;
					HBRUSH wndBrush;
					CRect wndrect;
					
					wndBrush = CreateSolidBrush(RGB(255,255,255));
					dc = GetDC(m_hWnd);
					GetClientRect(m_hWnd,&wndrect);
					int w=wndrect.Width(), h=wndrect.Height();

					//// Create Enhanced Metafile
					//hScreen = GetDC( NULL );
					//mfdc = CreateEnhMetaFile(dc,NULL,NULL,_T("FLUKE\0COMBISCOPE\0\0"));
					//if (mfdc) {
					//	myUpdate = false;
					//	Draw(mfdc,1);
					//}

					// Create DIB bitmap file
					bdc = CreateCompatibleDC(dc);
					bitmap = CreateCompatibleBitmap(dc,1024,768);
					SelectObject (bdc, bitmap);
					FillRect(bdc,wndrect,wndBrush);
					myUpdate = false;
					Draw(bdc,1);

					CImage img;
					img.Attach(bitmap);
					img.Save("PM33XX.bmp");

					ReleaseDC(m_hWnd,dc);
					DeleteObject(bdc);
					DeleteObject(bitmap);
				}
				break;

			case IDM_SAVEASPNG:
				{
					//HDC mfdc; 
					HDC dc, bdc;
					HBITMAP bitmap;
					HBRUSH wndBrush;
					CRect wndrect;
					
					wndBrush = CreateSolidBrush(RGB(255,255,255));
					dc = GetDC(m_hWnd);
					GetClientRect(m_hWnd,&wndrect);
					int w=wndrect.Width(), h=wndrect.Height();

					//// Create Enhanced Metafile
					//hScreen = GetDC( NULL );
					//mfdc = CreateEnhMetaFile(dc,NULL,NULL,_T("FLUKE\0COMBISCOPE\0\0"));
					//if (mfdc) {
					//	myUpdate = false;
					//	Draw(mfdc,1);
					//}

					// Create DIB bitmap file
					bdc = CreateCompatibleDC(dc);
					bitmap = CreateCompatibleBitmap(dc,1024,768);
					SelectObject (bdc, bitmap);
					FillRect(bdc,wndrect,wndBrush);
					myUpdate = false;
					Draw(bdc,1);

					CImage img;
					img.Attach(bitmap);
					img.Save("PM33XX.png");

					ReleaseDC(m_hWnd,dc);
					DeleteObject(bdc);
					DeleteObject(bitmap);
				}
				break;

			case IDM_AUTOSETUP:
				if (hCommPort)
				{
					unsigned long nc;
					WriteFile(hCommPort,"AS\r",3,&nc,NULL);	
				}
				break;

			case IDM_ZOOM:
				zoomMode = !zoomMode;
				break;
/*
			case IDM_ZOOMEXTEND:
				zoomMode = false;
				g_pFramework->InvalidateUICommand(IDM_ZOOM, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
				ResetZoom();
				break;
*/
			case IDM_SELECTCOLOR:
				{
					// Retrieve color.
					if (pCommandExecutionProperties)
					{
						PROPVARIANT var;
						UINT color;
						hr = pCommandExecutionProperties->GetValue(UI_PKEY_Color, &var);
						if (FAILED(hr))
						{
							return hr;
						}
						UIPropertyToUInt32(UI_PKEY_Color, var, &color);

						if (it>=0)
						{
							char tcr[50];
							trace[it].color = (COLORREF)color;
							sprintf_s(tcr,50,"TC%i",it+1);
							SetReg("",tcr,(char*)&trace[it].color,4,REG_DWORD);
						}
						if (ic==4)
						{
							bgCol = (COLORREF)color;
							SetReg("","BC",(char*)&bgCol,4,REG_DWORD);
						}
						else if (ic>=0)
						{
							char tcr[50];
							Cursor[ic].color = (COLORREF)color;
							sprintf_s(tcr,50,"TC%i",ic+37);
							SetReg("",tcr,(char*)&Cursor[ic].color,4,REG_DWORD);
						}
						myUpdate = true;
						InvalidateRect(m_hWnd,NULL,TRUE);
						UpdateWindow(m_hWnd);
					}
				}
				break;

			case IDM_COPY:
				SendMessage(m_hWnd,WM_LBUTTONDBLCLK,0,0);  // send double-click message
				break;

			case IDM_INFO:
				showInfo = !showInfo;
				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				break;
		
			case IDM_CONNECT:
				Connect(ComData[SelectedComport],atoi(BaudRates[SelectedBaudrate]));
				break;

			case IDM_HARDCOPY:
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

						CommTimeouts.ReadIntervalTimeout = 5000;
						fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);
						if (!fSuccess) { break; }

						// Request HPGL hardcopy data
						// Note: should make sure that PM3394 sends in HPGL format.
						FlushFileBuffers(hCommPort);
						WriteFile(hCommPort,_T("QP1\r"),4,&nc,NULL);
						Sleep(1000);

						
						RECT sr;
						HDC hsDC = GetDC(m_hStatusWnd);
						HFONT hfnt = CreateFont(18,0,0,0,FW_DONTCARE,0,0,0,0,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,18,"Calibri");
						SelectObject(hsDC,hfnt);
						sr.left=0;sr.right=150;sr.top=2;sr.bottom=21;
						int blocksize = 250;
						int di = 0;
						char sstr[20];
						ReadFile(hCommPort,&hpgldata[di],blocksize,&nc,NULL);
						di += nc;
						while (nc == blocksize)
						{
							rtn = ReadFile(hCommPort,&hpgldata[di],blocksize,&nc,NULL);
							di += nc;
							sprintf_s(sstr,20,"Reading %i %%\r\n",di/200);
							DrawStatusText(hsDC,&sr,sstr,0);
							TRACE1("Read = %i\r\n",di);
						}
						DeleteObject(hfnt);
						
						CommTimeouts.ReadIntervalTimeout = 500;
						fSuccess = SetCommTimeouts(hCommPort, &CommTimeouts);

						// Store HPGL data to file
						HANDLE hFile = CreateFile(_T("pm33xx.hgl"),GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
						if (hFile == INVALID_HANDLE_VALUE) 
						{
							break;
						}
						else
						{
							WriteFile(hFile,hpgldata,di,&fnc,NULL);
							TRACE1("Wrote = %i\r\n",di);
							CloseHandle(hFile);
						}
					}
#ifndef _DEBUG
					if (hCommPort != INVALID_HANDLE_VALUE)
#endif
					{
						GetWindowRect(m_hWnd,&rc);
						// Convert HPGL file to PNG. Uses external command hp2xx.exe
						s = (rc.right-rc.left)/2;
						sprintf_s(ccmd,200, "-m png -c 1234567 -r270 -h%i -w%i pm33xx.hgl",s,s);
						TRACE1(">hp2xx.exe %s\r\n",ccmd);
						SHELLEXECUTEINFO sexi = {0};
						sexi.cbSize = sizeof(SHELLEXECUTEINFO);
						sexi.hwnd = m_hWnd;
						sexi.fMask = SEE_MASK_NOCLOSEPROCESS;
						sexi.lpFile = _T("..\\HPGL\\hp2xx.exe");
						sexi.lpParameters = ccmd;
						sexi.nShow = SW_HIDE;
						//sexi.nShow = SW_SHOW;
						if(ShellExecuteEx(&sexi))
						{
							DWORD wait = WaitForSingleObject(sexi.hProcess, INFINITE);
							if(wait == WAIT_OBJECT_0) GetExitCodeProcess(sexi.hProcess, &rtn);
						}

						// load PNG bitmap file
						if (img.Load(_T("pm33xx.png"))==NULL) 
							hBmp = img.Detach();

						// Display bitmap in Dialog Window
						InvalidateRect(m_hWnd,NULL,TRUE);
						UpdateWindow(m_hWnd);
					}
				}
				break;

			case IDM_GETTRACES:
				if (hCommPort != INVALID_HANDLE_VALUE)
				{
					if (hBmp)
					{
						DeleteObject(hBmp);
						hBmp = NULL;
					}
					for (int iTrace=0;iTrace<MAX_TRACE;iTrace++)
						if (trace[iTrace].Enable)
							GetTrace(iTrace);
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
				break;

			case IDM_RUN:
				if (hCommPort != INVALID_HANDLE_VALUE)
				{
					unsigned long nc;
					WriteFile(hCommPort,"AT\r",3,&nc,NULL);	
				}
				break;

			case IDM_FFT:
				if (key != NULL && UI_PKEY_BooleanValue == *key)
				{
					hr = UIPropertyToBoolean(*key, *ppropvarValue, (BOOL*)&fftMode);
					if (fftMode) 
					{
						for (int iTrace=0;iTrace<MAX_TRACE;iTrace++)
							if ((trace[iTrace].ntrace>0) && (trace[iTrace].Enable))
								Spectrum(iTrace);
					}
					ResetZoom();
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
				break;

			case IDM_XLOG:
				xLog = !xLog;
				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				break;

			case IDM_YLOG:
				yLog = !yLog;
				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				break;

			case IDM_MRU:
				if (key != NULL && UI_PKEY_SelectedItem == *key)
				{
					UINT uSelectedMRUItem = 0xffffffff;
					if (ppropvarValue != NULL && SUCCEEDED(UIPropertyToUInt32(*key, *ppropvarValue, &uSelectedMRUItem)))
					{
						ATLTRACE2("Loading %s\r\n",mru[uSelectedMRUItem]);
						ASSERT(uSelectedMRUItem < 10);
						strcpy_s(lfn.lpstrFile,MAX_FILENAME,mru[uSelectedMRUItem]);
						strcpy_s(lfn.lpstrFileTitle,MAX_FILENAME,"");
						OpenTraceFile(FALSE);
					}
				}
				break;

			case IDM_RIBBONEXIT:
				PostMessage(m_hWnd,WM_DESTROY,NULL,NULL);
				break;

			case IDM_CH1: 
				if (key != NULL && UI_PKEY_BooleanValue == *key) 
				{
					hr = UIPropertyToBoolean(*key, *ppropvarValue, (BOOL*)&trace[0].Enable); 
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
				break;
			case IDM_CH2: 
				if (key != NULL && UI_PKEY_BooleanValue == *key) 
				{
					hr = UIPropertyToBoolean(*key, *ppropvarValue, (BOOL*)&trace[1].Enable); 
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
				break;
			case IDM_CH3: 
				if (key != NULL && UI_PKEY_BooleanValue == *key) 
				{
					hr = UIPropertyToBoolean(*key, *ppropvarValue, (BOOL*)&trace[2].Enable); 
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
				break;
			case IDM_CH4: 
				if (key != NULL && UI_PKEY_BooleanValue == *key) 
				{
					hr = UIPropertyToBoolean(*key, *ppropvarValue, (BOOL*)&trace[3].Enable); 
					InvalidateRect(m_hWnd,NULL,TRUE);
					UpdateWindow(m_hWnd);
				}
				break;

			default:
				ATLTRACE2("Pressed button ID=%i\r\n",nCmdID);

				break;
			}
        }	    
        return hr;
    }

    STDMETHODIMP UpdateProperty(UINT nCmdID,
        __in REFPROPERTYKEY key,
        __in_opt const PROPVARIANT* ppropvarCurrentValue,
        __out PROPVARIANT* ppropvarNewValue)
    {
        UNREFERENCED_PARAMETER(ppropvarCurrentValue);
		HRESULT hr = E_FAIL;

		switch (nCmdID)
		{

		case IDM_ZOOM:
			if (UI_PKEY_BooleanValue == key)
			hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, zoomMode, ppropvarNewValue);
			break;

		case IDM_CONNECT:
			// use pComportHandler to retrieve a bitmap from application resource library
			if (pComPortHandler) {
				IUIImage *pImg;
				if (hCommPort == INVALID_HANDLE_VALUE)
					hr = pComPortHandler->CreateUIImageFromBitmapResource(MAKEINTRESOURCE(IDB_DISCONNECT), &pImg);
				else
					hr = pComPortHandler->CreateUIImageFromBitmapResource(MAKEINTRESOURCE(IDB_CONNECT), &pImg);

				hr = UIInitPropertyFromImage(UI_PKEY_LargeImage, pImg, ppropvarNewValue);
			}
			break;

		case IDM_MRU:
			if (UI_PKEY_Label == key) { }
			else if (UI_PKEY_RecentItems == key)
			{
				hr = PopulateRibbonRecentItems(ppropvarNewValue);
			}
			break;

		case IDM_SELECTCOLOR:
			if (UI_PKEY_Color == key)
			{
				if (it>=0)
					hr = UIInitPropertyFromUInt32(UI_PKEY_Color, trace[it].color, ppropvarNewValue);
				if (ic==4)
					hr = UIInitPropertyFromUInt32(UI_PKEY_Color, bgCol, ppropvarNewValue);
				else if (ic>=0)
					hr = UIInitPropertyFromUInt32(UI_PKEY_Color, Cursor[ic].color, ppropvarNewValue);
			}
			if (UI_PKEY_Enabled == key)
			{
				bool _enabled;
				if ((it>=0) || (ic>=0)) _enabled = true; else _enabled = false;
				hr = UIInitPropertyFromBoolean(UI_PKEY_Enabled, _enabled, ppropvarNewValue);
			}
			break;

		case IDM_FFT:
			if (UI_PKEY_BooleanValue == key)
				hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, fftMode, ppropvarNewValue);
			break;

		case IDM_CH1: if (UI_PKEY_BooleanValue == key) hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, trace[0].Enable, ppropvarNewValue); break;
		case IDM_CH2: if (UI_PKEY_BooleanValue == key) hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, trace[1].Enable, ppropvarNewValue); break;
		case IDM_CH3: if (UI_PKEY_BooleanValue == key) hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, trace[2].Enable, ppropvarNewValue); break;
		case IDM_CH4: if (UI_PKEY_BooleanValue == key) hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, trace[3].Enable, ppropvarNewValue); break;

		default:
			break;
		}
        return hr;
    }

private:
    BOOL _fEnabled;

};

CComObject<CApplication> *pApplication = NULL;

HRESULT InitRibbon(HWND hWindowFrame)
{	
    HRESULT hr = ::CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pFramework));
    if(FAILED(hr)) return hr;

    pApplication = NULL;
    hr = CComObject<CApplication>::CreateInstance(&pApplication);
    if(FAILED(hr)) return hr;

    hr = g_pFramework->Initialize(hWindowFrame, pApplication);
    if(FAILED(hr)) return hr;

    hr = g_pFramework->LoadUI(GetModuleHandle(NULL), L"APPLICATION_RIBBON");
    if(FAILED(hr)) return hr;

	return S_OK;
}

void DestroyRibbon()
{
    if (g_pFramework)
    {
        g_pFramework->Destroy();
        g_pFramework->Release();
        g_pFramework = NULL;
    }
}


///////////////////  TASK HANDLERS  /////////////////////////////////////////////

PROPVARIANT ppropvarNewValue;
void CListHandler::Update()
{
	g_pFramework->InvalidateUICommand(ID, UI_INVALIDATIONS_VALUE, &UI_PKEY_SelectedItem);
	g_pFramework->InvalidateUICommand(ID, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_ItemsSource);
}

//
//  FUNCTION: Execute()
//
//  PURPOSE: Called by the Ribbon framework when the selects or types in a new border size.
//
//  COMMENTS:
//    This sets the size of the border around the shapes being drawn or displays an error
//    message if the user entered an invalid size.
//
//
STDMETHODIMP CListHandler::Execute(UINT nCmdID,
                   UI_EXECUTIONVERB verb, 
                   const PROPERTYKEY* key,
                   const PROPVARIANT* ppropvarValue,
                   IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = E_FAIL;
    if (verb == UI_EXECUTIONVERB_EXECUTE)
    {
		switch (nCmdID)
		{
		case IDM_COMPORTS:
			if ( key && *key == UI_PKEY_SelectedItem)
			{      
				UINT selected;
				hr = UIPropertyToUInt32(*key, *ppropvarValue, &selected);
				SelectedComport = selected;
				hr = S_OK;
			}	
			break;
		case IDM_BAUD:
			if ( key && *key == UI_PKEY_SelectedItem)
			{      
				UINT selected;
				hr = UIPropertyToUInt32(*key, *ppropvarValue, &selected);
				SelectedBaudrate = selected;
				hr = S_OK;
			}	
			break;
		case IDM_CURSORS:
			if (ppropvarValue == NULL) // The Button part of the Border Style SplitButtonGallery was clicked.
			{
	            
				//g_renderer.UpdateRenderParam(param);
				// Update the visual state of the button- toggled on for border selected, off for no border.
				//g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSTYLES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
				// The Border Sizes combobox needs to be re-enabled if the dash border was just de-selected.
				//g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSIZES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);

				cursorFcn = 0;
				g_pFramework->InvalidateUICommand(IDM_CURSORS, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_LargeImage);

				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				hr = S_OK;
			}
			else if ( key && *key == UI_PKEY_SelectedItem)
			{      
				UINT selected;
				hr = UIPropertyToUInt32(*key, *ppropvarValue, &selected);

				cursorFcn = selected;
	            
				g_pFramework->InvalidateUICommand(IDM_CURSORS, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_LargeImage);

				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				hr = S_OK;
			}			
			break;
		case IDM_WINDOW:
			if ( key && *key == UI_PKEY_SelectedItem)
			{      
				UINT selected;
				hr = UIPropertyToUInt32(*key, *ppropvarValue, &selected);

				wndFcn = selected;
				if (fftMode)
				{
					for (int iTrace=0;iTrace<MAX_TRACE;iTrace++)
					{
						if ((trace[iTrace].ntrace>0) && (trace[iTrace].Enable))
						{
							SetTraceData(trace[it].ntrace);  // reset the data pointer
							Spectrum(iTrace);
						}
					}
				}
	            
				InvalidateRect(m_hWnd,NULL,TRUE);
				UpdateWindow(m_hWnd);
				hr = S_OK;
			}			
			break;
		}
	}
    return hr;
}

//
//  FUNCTION: UpdateProperty()
//
//  PURPOSE: Called by the Ribbon framework when a command property (PKEY) needs to be updated.
//
//  COMMENTS:
//
//    Depending on the value of key, this will populate the gallery, update the selected item or 
//    text, or enable/disable the gallery.
//
//
STDMETHODIMP CListHandler::UpdateProperty(UINT nCmdID,
                              REFPROPERTYKEY key,
                              const PROPVARIANT* ppropvarCurrentValue,
                              PROPVARIANT* ppropvarNewValue)
{
    HRESULT hr = E_FAIL;

	int imageIds[4];
	int labelIds[] = {IDS_CURSOR, IDS_CURSORX, IDS_CURSORY, IDS_CURSORXY};

	imageIds[0] = IDB_CURSOR;
	imageIds[1] = IDB_CURSORX;
	imageIds[2] = IDB_CURSORY;
	imageIds[3] = IDB_CURSORXY;

	switch (nCmdID)
	{
	case IDM_COMPORTS:
		if(key == UI_PKEY_Categories)
			{
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_ItemsSource)
			{
				int iLB = 0, idx = 0;
				HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL,NULL,NULL,DIGCF_PRESENT | DIGCF_ALLCLASSES);
				SP_DEVICE_INTERFACE_DATA interfaceData;
				ZeroMemory(&interfaceData, sizeof(interfaceData));
				interfaceData.cbSize = sizeof(interfaceData);

				IUICollection* pCollection;
				hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
				if (FAILED(hr)) return hr;
				pCollection->Clear();
				
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
									
									// Create a new property set for each item.
									CPropertySet* pItem;
									hr = CPropertySet::CreateInstance(&pItem);
									if (FAILED(hr))
									{
										pCollection->Release();
										return hr;
									}

									// Load the label from the resource file.
									WCHAR wszLabel[MAX_RESOURCE_LENGTH];
									if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, 
										(LPCSTR)friendlyName, (int)strlen(friendlyName)+1, wszLabel, MAX_RESOURCE_LENGTH))
									{
										// Initialize the property set with no image and label that were just loaded and no category.
										pItem->InitializeItemProperties(NULL, wszLabel, UI_COLLECTION_INVALIDINDEX);

										// Add the newly-created property set to the collection supplied by the framework.
										pCollection->Add(pItem);
									}

									pItem->Release();


									//idx = pCOM->AddString(CString(friendlyName));
									int iCom = pCom[4]-'0';
									if (pCom[5]!=')') iCom = 10 * iCom + pCom[5] - '0';
									ComData[idx++] = iCom;
								}
							}
						}
					}
				}

				// If there is a recent COMPORT entry in registry that matches, then select it
				//int iCOM = AfxGetApp()->GetProfileInt(_T(""),_T("COM"),0);
				//if ((iCOM>=0) && (pCOM->GetCount()>=iCOM)) 
				//	pCOM->SetCurSel(iCOM); 
				//else pCOM->SetCurSel(0);
								
				pCollection->Release();
				hr = S_OK;
			}
		else if (key == UI_PKEY_Categories)
			{	
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_SelectedItem)
			{
				hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, 0, ppropvarNewValue);
			}
		else if (key == UI_PKEY_StringValue)
			{
				//char taskLabel[50];
				//sprintf_s(taskLabel,50,"%i %s",Task[SelectedTask]->ID,Task[SelectedTask]->Name);
				//// Load the label for each size from the resource file.
				//WCHAR wszLabel[MAX_RESOURCE_LENGTH];
				//if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, 
				//			(LPCSTR)taskLabel, (int)strlen(taskLabel)+1,wszLabel, MAX_RESOURCE_LENGTH))
				//{
				//	hr = UIInitPropertyFromString(UI_PKEY_StringValue, wszLabel, ppropvarNewValue);
				//}
				TRACE("Updated Task String\r\n");
				hr = S_OK;
			}
		else if (key == UI_PKEY_Enabled)
			{
				// The border size combobox gets disabled when Dash Line is selected for the border style.
				bool active = true;
				hr = UIInitPropertyFromBoolean(UI_PKEY_Enabled, active, ppropvarNewValue);
			}
		else if (key == UI_PKEY_Label)
			{
				//hr = UIInitPropertyFromString(UI_PKEY_StringValue, L"Task List", ppropvarNewValue);
				hr = S_OK;
			}
		else if (key == UI_PKEY_LargeImage)
			{
				//IUIImage *pImg;
				//hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[cursorFcn]), &pImg);
				//hr = UIInitPropertyFromImage(UI_PKEY_LargeImage, pImg, ppropvarNewValue);
				hr = S_OK;
			}
		break;

	case IDM_BAUD:
		if(key == UI_PKEY_Categories)
			{
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_ItemsSource)
			{
				IUICollection* pCollection;
				hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
				if (FAILED(hr)) return hr;
				pCollection->Clear();
				for (int i=0;i<4;i++)
				{
					// Create a new property set for each item.
					CPropertySet* pItem;
					hr = CPropertySet::CreateInstance(&pItem);
					if (FAILED(hr))
					{
						pCollection->Release();
						return hr;
					}

					// Load the label from the resource file.
					WCHAR wszLabel[MAX_RESOURCE_LENGTH];
					if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, 
						(LPCSTR)BaudRates[i], (int)strlen(BaudRates[i])+1, wszLabel, MAX_RESOURCE_LENGTH))
					{
						// Initialize the property set with no image and label that were just loaded and no category.
						pItem->InitializeItemProperties(NULL, wszLabel, UI_COLLECTION_INVALIDINDEX);

						// Add the newly-created property set to the collection supplied by the framework.
						pCollection->Add(pItem);
					}

					pItem->Release();

				}

				// If there is a recent COMPORT entry in registry that matches, then select it
				//int iCOM = AfxGetApp()->GetProfileInt(_T(""),_T("COM"),0);
				//if ((iCOM>=0) && (pCOM->GetCount()>=iCOM)) 
				//	pCOM->SetCurSel(iCOM); 
				//else pCOM->SetCurSel(0);

								
				pCollection->Release();
				hr = S_OK;
			}
		else if (key == UI_PKEY_Categories)
			{	
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_SelectedItem)
			{
				hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, SelectedBaudrate, ppropvarNewValue);
			}
		else if (key == UI_PKEY_StringValue)
			{
				// Load the label for each size from the resource file.
				WCHAR wszLabel[MAX_RESOURCE_LENGTH];
				if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, 
							(LPCSTR)BaudRates[SelectedBaudrate], (int)strlen(BaudRates[SelectedBaudrate])+1,wszLabel, MAX_RESOURCE_LENGTH))
				{
					hr = UIInitPropertyFromString(UI_PKEY_StringValue, wszLabel, ppropvarNewValue);
				}
				TRACE("Updated Task String\r\n");
				hr = S_OK;
			}
		else if (key == UI_PKEY_Enabled)
			{
				// The border size combobox gets disabled when Dash Line is selected for the border style.
				bool active = true;
				hr = UIInitPropertyFromBoolean(UI_PKEY_Enabled, active, ppropvarNewValue);
			}
		else if (key == UI_PKEY_Label)
			{
				//hr = UIInitPropertyFromString(UI_PKEY_StringValue, L"Task List", ppropvarNewValue);
				hr = S_OK;
			}
		else if (key == UI_PKEY_LargeImage)
			{
				//IUIImage *pImg;
				//hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[cursorFcn]), &pImg);
				//hr = UIInitPropertyFromImage(UI_PKEY_LargeImage, pImg, ppropvarNewValue);
				hr = S_OK;
			}
		break;

	case IDM_CURSORS:		
		if(key == UI_PKEY_Categories)
			{
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_ItemsSource)
			{
				IUICollection* pCollection;
				hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
				if (FAILED(hr)) return hr;
				pCollection->Clear();

				// Populate the dropdown with the three border styles.
				for (int i=0; i<_countof(labelIds); i++)
				{
					// Create a new property set for each item.
					CPropertySet* pItem;
					hr = CPropertySet::CreateInstance(&pItem);
					if (FAILED(hr))
					{
						pCollection->Release();
						return hr;
					}

					// Create an IUIImage from a resource id.
					IUIImage* pImg;
					hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[i]), &pImg);
					if (FAILED(hr))
					{
						pCollection->Release();
						pItem->Release();
						return hr;
					}

					// Load the label from the resource file.
					char szLabel[MAX_RESOURCE_LENGTH];
					WCHAR wszLabel[MAX_RESOURCE_LENGTH];
					LoadString(GetModuleHandle(NULL), labelIds[i], szLabel, MAX_RESOURCE_LENGTH);
					if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, (LPCSTR)szLabel, (int)strlen(szLabel)+1, wszLabel, MAX_RESOURCE_LENGTH))
					{
						// Initialize the property set with the image and label that were just loaded and no category.
						pItem->InitializeItemProperties(pImg, wszLabel, UI_COLLECTION_INVALIDINDEX);

						// Add the newly-created property set to the collection supplied by the framework.
						pCollection->Add(pItem);
					}

					pItem->Release();
					pImg->Release();
				}
				pCollection->Release();
				hr = S_OK;
			}
		else if (key == UI_PKEY_Categories)
			{	
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_SelectedItem)
			{
				hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, cursorFcn, ppropvarNewValue);
			}
		else if (key == UI_PKEY_StringValue)
			{
				//char taskLabel[50];
				//sprintf_s(taskLabel,50,"%i %s",Task[SelectedTask]->ID,Task[SelectedTask]->Name);
				//// Load the label for each size from the resource file.
				//WCHAR wszLabel[MAX_RESOURCE_LENGTH];
				//if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, 
				//			(LPCSTR)taskLabel, (int)strlen(taskLabel)+1,wszLabel, MAX_RESOURCE_LENGTH))
				//{
				//	hr = UIInitPropertyFromString(UI_PKEY_StringValue, wszLabel, ppropvarNewValue);
				//}
				TRACE("Updated Task String\r\n");
				hr = S_OK;
			}
		else if (key == UI_PKEY_Enabled)
			{
				// The border size combobox gets disabled when Dash Line is selected for the border style.
				bool active = true;
				hr = UIInitPropertyFromBoolean(UI_PKEY_Enabled, active, ppropvarNewValue);
			}
		else if (key == UI_PKEY_Label)
			{
				//hr = UIInitPropertyFromString(UI_PKEY_StringValue, L"Task List", ppropvarNewValue);
				hr = S_OK;
			}
		else if (key == UI_PKEY_LargeImage)
			{
				IUIImage *pImg;
				hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[cursorFcn]), &pImg);
				hr = UIInitPropertyFromImage(UI_PKEY_LargeImage, pImg, ppropvarNewValue);
			}
		break;


	case IDM_WINDOW:		
		if(key == UI_PKEY_Categories)
			{
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_ItemsSource)
			{
				IUICollection* pCollection;
				hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
				if (FAILED(hr)) return hr;
				pCollection->Clear();

				// Populate the dropdown with the three border styles.
				for (int i=1; i<=15; i++)
				{
					// Create a new property set for each item.
					CPropertySet* pItem;
					hr = CPropertySet::CreateInstance(&pItem);
					if (FAILED(hr))
					{
						pCollection->Release();
						return hr;
					}

					// Create an IUIImage from a resource id.
					IUIImage* pImg;
					hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(IDB_FFTWINDOW), &pImg);
					if (FAILED(hr))
					{
						pCollection->Release();
						pItem->Release();
						return hr;
					}

					WCHAR wszLabel[MAX_RESOURCE_LENGTH];
					switch(i)
					{
					case 2: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Gaussian");break;
					case 3: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Hamming");break;
					case 4: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Hann");break;
					case 5: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Bart");break;
					case 6: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Wlch");break;
					case 7: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Triangle");break;
					case 8: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Brhn");break;
					case 9: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Blkm");break;
					case 10: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Kais");break;
					case 11: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Nutl");break;
					case 12: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Bkha");break;
					case 13: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Bknu");break;
					case 14: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Fltt");break;
					case 15: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Sine");break;
					default: wcscpy_s(wszLabel,MAX_RESOURCE_LENGTH,L"Rectangular");break;
					}
					// Initialize the property set with the image and label that were just loaded and no category.
					pItem->InitializeItemProperties(pImg, wszLabel, UI_COLLECTION_INVALIDINDEX);

					// Add the newly-created property set to the collection supplied by the framework.
					hr = pCollection->Add(pItem);

					pItem->Release();
					pImg->Release();
				}
				pCollection->Release();
				hr = S_OK;
			}
		else if (key == UI_PKEY_Categories)
			{	
				// A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
				// If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
				hr = S_FALSE;
			}
		else if (key == UI_PKEY_SelectedItem)
			{
				//hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, cursorFcn, ppropvarNewValue);
				hr = S_OK;
			}
		else if (key == UI_PKEY_StringValue)
			{
				//char taskLabel[50];
				//sprintf_s(taskLabel,50,"%i %s",Task[SelectedTask]->ID,Task[SelectedTask]->Name);
				//// Load the label for each size from the resource file.
				//WCHAR wszLabel[MAX_RESOURCE_LENGTH];
				//if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, 
				//			(LPCSTR)taskLabel, (int)strlen(taskLabel)+1,wszLabel, MAX_RESOURCE_LENGTH))
				//{
				//	hr = UIInitPropertyFromString(UI_PKEY_StringValue, wszLabel, ppropvarNewValue);
				//}
				TRACE("Updated Window String\r\n");
				hr = S_OK;
			}
		else if (key == UI_PKEY_Enabled)
			{
				// The border size combobox gets disabled when Dash Line is selected for the border style.
				//bool active = true;
				//hr = UIInitPropertyFromBoolean(UI_PKEY_Enabled, active, ppropvarNewValue);
				hr = S_OK;
			}
		else if (key == UI_PKEY_Label)
			{
				//hr = UIInitPropertyFromString(UI_PKEY_StringValue, L"Task List", ppropvarNewValue);
				hr = S_OK;
			}
		else if (key == UI_PKEY_LargeImage)
			{
				//IUIImage *pImg;
				//hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[cursorFcn]), &pImg);
				//hr = UIInitPropertyFromImage(UI_PKEY_LargeImage, pImg, ppropvarNewValue);
				hr = S_OK;
			}
		break;

	} // switch nCmdID
    return hr;
}

// Factory method to create IUIImages from resource identifiers.
HRESULT CListHandler::CreateUIImageFromBitmapResource(LPCTSTR pszResource, IUIImage **ppimg)
{
    HRESULT hr = E_FAIL;

    *ppimg = NULL;

    if (NULL == m_pifbFactory)
    {
        hr = CoCreateInstance(CLSID_UIRibbonImageFromBitmapFactory, NULL, CLSCTX_ALL, IID_PPV_ARGS(&m_pifbFactory));
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Load the bitmap from the resource file.
    HBITMAP hbm = (HBITMAP) LoadImage(GetModuleHandle(NULL), pszResource, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (hbm)
    {
        // Use the factory implemented by the framework to produce an IUIImage.
        hr = m_pifbFactory->CreateImage(hbm, UI_OWNERSHIP_TRANSFER, ppimg);
        if (FAILED(hr))
        {
            DeleteObject(hbm);
        }
    }
    return hr;
}


HRESULT CListHandler::CreateInstance(CListHandler **ppHandler, int id)
{
    if (!ppHandler) return E_POINTER;
    *ppHandler = NULL;
    HRESULT hr = S_OK;
    CListHandler* pHandler = new CListHandler(id);
    if (pHandler != NULL) *ppHandler = pHandler;
    else hr = E_OUTOFMEMORY;
    return hr;
}

// IUnknown methods.
STDMETHODIMP_(ULONG) CListHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CListHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) delete this;
    return cRef;
}

STDMETHODIMP CListHandler::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (iid == __uuidof(IUnknown)) *ppv = static_cast<IUnknown*>(this);
    else if (iid == __uuidof(IUICommandHandler)) *ppv = static_cast<IUICommandHandler*>(this);
    else { *ppv = NULL; return E_NOINTERFACE; }
    AddRef();
    return S_OK;
}


