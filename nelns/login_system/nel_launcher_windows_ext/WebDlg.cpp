// WebDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "nel_launcherDlg.h"
#include "WebDlg.h"
#include "LoginDlg.h"

#include <comdef.h>
#include <mshtml.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWebDlg dialog


CWebDlg::CWebDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWebDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWebDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWebDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWebDlg)
	DDX_Control(pDX, IDC_EXPLORER1, m_explore);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWebDlg, CDialog)
	//{{AFX_MSG_MAP(CWebDlg)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebDlg message handlers

BEGIN_EVENTSINK_MAP(CWebDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CWebDlg)
	ON_EVENT(CWebDlg, IDC_EXPLORER1, 250 /* BeforeNavigate2 */, OnBeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CWebDlg, IDC_EXPLORER1, 259 /* DocumentComplete */, OnDocumentComplete, VTS_DISPATCH VTS_PVARIANT)
	ON_EVENT(CWebDlg, IDC_EXPLORER1, 252 /* NavigateComplete2 */, OnNavigateComplete2Explorer1, VTS_DISPATCH VTS_PVARIANT)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CWebDlg::OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel) 
{
/*	CRect	r;

	GetClientRect(&r);
	ClientToScreen(&r);
	m_dlgLoading.MoveWindow(r.left+BROWSER_X, r.top+BROWSER_Y, BROWSER_W, BROWSER_H);
	m_dlgLoading.ShowWindow(SW_SHOW);*/

	// set the user agent to nel_launcher changing the header
    CString csHeader(Headers->bstrVal);

    if(csHeader.IsEmpty())
	{
		IWebBrowser2 *pBrowser;
		LPDISPATCH pWebDisp;

		pDisp->QueryInterface(IID_IWebBrowser2, (void**) &pBrowser);
		pBrowser->get_Container(&pWebDisp);

		BSTR bstr = SysAllocString(L"User-Agent: nel_launcher\r\n");
		Headers->bstrVal =  bstr;

		pBrowser->Navigate2(URL, Flags, TargetFrameName, PostData, Headers);

		if (!pWebDisp)
			(*Cancel) = true;

		if (pWebDisp)
			pWebDisp->Release();
		if (pBrowser)
			pBrowser->Release();

		SysFreeString(bstr);
		return;
    }	
}

VARIANT CWebDlg::ExecuteScript(CString csCode, CString csLanguage)
{
	COleVariant   varRet;

	IHTMLDocument2* pHTMLDocument2;
	IHTMLWindow2*   pHTMLWindow2;
	LPDISPATCH lpDispatch;
	lpDispatch = m_explore.GetDocument();
   
	BSTR            bstrCode    = _bstr_t((const char *)csCode);
	BSTR            bstrLang    = _bstr_t((const char *)csLanguage);
	HRESULT         hr;
 
	if(lpDispatch)
	{
		hr = lpDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pHTMLDocument2);
		if(hr == S_OK)
		{
			hr = pHTMLDocument2->get_parentWindow(&pHTMLWindow2);
			if(hr == S_OK)
			{
				pHTMLWindow2->execScript(bstrCode, bstrLang, &varRet);
				pHTMLWindow2->Release();
			}
			pHTMLDocument2->Release();
		}
		lpDispatch->Release();
	}
	return varRet;
}

VARIANT CWebDlg::ExecuteJavascript(CString csCode)
{
    return ExecuteScript(csCode, "Javascript");
}

void CWebDlg::OpenUrl(CString csUrl)
{
	m_csUrl	= csUrl;
	m_explore.Navigate(csUrl, NULL, NULL, NULL, NULL);
}

void CWebDlg::OpenUrl(const std::string &url)
{
	m_csUrl	= url.c_str();
	m_explore.Navigate(url.c_str(), NULL, NULL, NULL, NULL);
}

void CWebDlg::OnDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	IHTMLDocument2*	pHTMLDocument2;
	LPDISPATCH		lpDispatch;
	lpDispatch =	m_explore.GetDocument();

    if (lpDispatch)
	{
		HRESULT hr;
		hr = lpDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*) &pHTMLDocument2);
		lpDispatch->Release();

		if (FAILED(hr))
			return;

		if (pHTMLDocument2 == NULL)
			return;

		IHTMLElement* pBody;
		hr = pHTMLDocument2->get_body(&pBody);

		if (FAILED(hr))
			return;

		if (pBody == NULL)
			return;

		BSTR bstr;
		pBody->get_innerHTML(&bstr);
		
		CString csSourceCode(bstr);
		CString csAction;
		CString csComment;

		SysFreeString(bstr);
		pBody->Release();

		if(csSourceCode.Find("newlogin") != -1)
		{
			// Authentication not ok !
			APP.m_bAuthGame	= FALSE;
			APP.m_bAuthWeb	= FALSE;
			((CNel_launcherDlg*)AfxGetMainWnd())->m_wndTabs.DestroyWindow();
			DestroyWindow();
			((CNel_launcherDlg*)AfxGetMainWnd())->Login();
			return;
		}

// ACE: BADDDDDD!!!! the good web page can contains 404 in the cookie because the cookie contains any number we want
/*		else if(csSourceCode.Find("404") != -1)
		{
			// Authentication lost, try to redo the authentication
			CLoginDlg	dlg;

			dlg.AuthWeb();
			OpenUrl(m_csUrl);
			return;
		}
*/		int	iFindAction1	= csSourceCode.Find("<!--nel");
		int	iFindAction2;
		int	iFindComment1;
		int	iFindComment2;

		if(iFindAction1 != -1)
		{
			iFindAction1	= csSourceCode.Find('"', iFindAction1);
			if(iFindAction1 != -1)
			{
				iFindAction2	= csSourceCode.Find('"', iFindAction1+1);
				if(iFindAction2 != -1)
				{
					csAction	= csSourceCode.Mid(iFindAction1+1, iFindAction2 - iFindAction1-1);
					
					iFindComment1	= iFindAction2 + 2;
					if(iFindComment1 != -1)
					{
						iFindComment2	= csSourceCode.Find("-->", iFindAction1);
						if(iFindComment2 != -1)
						{
							csComment	= csSourceCode.Mid(iFindComment1, iFindComment2 - iFindComment1);
							csComment.TrimRight();
							csComment.TrimLeft();

							if(csAction == "launch")
								((CNel_launcherDlg*)GetParent())->launch(csComment);
							else if(csAction == "patch")
								((CNel_launcherDlg*)GetParent())->patch(csComment);
						}
					}
				}
			}
		}
	}
//	m_dlgLoading.ShowWindow(SW_HIDE);
}

BOOL CWebDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect	r;

	m_brushBG.CreateSolidBrush(RGB(0, 16, 0));
	
	GetClientRect(&r);
	GetDlgItem(IDC_EXPLORER1)->MoveWindow(-2, -2, r.Width()-48, r.Height());

//	m_dlgLoading.Create(IDD_LOADING_PAGE, this);
//	m_dlgLoading.ShowWindow(SW_HIDE);

//	ClientToScreen(&r);
//	m_dlgLoading.MoveWindow(r.left+BROWSER_X, r.top+BROWSER_Y, BROWSER_W, BROWSER_H);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWebDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
}

HBRUSH CWebDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(nCtlColor == CTLCOLOR_DLG)
	{
		hbr	= m_brushBG;
		pDC->SetBkColor(RGB(0, 16, 0));
		pDC->SetBkMode(OPAQUE);
	}	
	return hbr;
}

void CWebDlg::OnNavigateComplete2Explorer1(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
}

BOOL CWebDlg::OnEraseBkgnd(CDC* pDC) 
{
    CBrush	backBrush(RGB(0, 16, 0));
    CBrush*	pOldBrush	= pDC->SelectObject(&backBrush);
    CRect r;

	pDC->GetClipBox(&r);     // Erase the area needed.
    pDC->PatBlt(r.left, r.top, r.Width(), r.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush);
    return TRUE;
}
