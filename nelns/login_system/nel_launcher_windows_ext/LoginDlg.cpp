// LoginDlg.cpp : implementation file
//

#include "stdafx.h"

#include "nel/misc/string_mapper.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

#include "nel_launcherDlg.h"
#include "nel_launcher.h"
#include "MsgDlg.h"
#include "LoginDlg.h"
#include "patch.h"
#include "Md5.h"

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog
using namespace std;

#define LOGIN_BG_X		0
#define LOGIN_BG_Y		0

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	APP.ResetConnection();
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_NCLBUTTONUP()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(pWnd && pWnd->GetDlgCtrlID() == IDC_ERR_STATIC)
	{
		hbr	= m_brush;
		pDC->SetBkColor(RGB(12, 42, 52));
		pDC->SetTextColor(RGB(255, 0, 0));
	}
	else if(nCtlColor == CTLCOLOR_STATIC)
	{
		hbr	= m_brush;
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 127));
	}
	else if(nCtlColor == CTLCOLOR_EDIT)
	{
		hbr	= m_brushEdit;
		pDC->SetTextColor(RGB(255, 255, 127));
		pDC->SetBkColor(RGB(12, 42, 52));
	}
	return hbr;
}

CString CLoginDlg::GetLogin()
{
	return APP.m_csLogin;
}

CString CLoginDlg::GetPassword()
{
	return APP.m_csPassword;
}

BOOL CLoginDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString csLogin	= APP.GetRegKeyValue(_T("Pseudo"));

	m_brush.CreateStockObject(NULL_BRUSH);
	m_brushEdit.CreateSolidBrush(RGB(12, 42, 52));

	SetDlgItemText(IDC_ERR_STATIC, "");	
	SetDlgItemText(IDC_LOGIN_EDIT, csLogin);
	if(csLogin.IsEmpty())
		GotoDlgCtrl(GetDlgItem(IDC_LOGIN_EDIT));
	else
		GotoDlgCtrl(GetDlgItem(IDC_PWD_EDIT));

	GetDlgItem(IDC_LOGINU_STATIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_LOGIND_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_QUITU_STATIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_QUITD_STATIC)->ShowWindow(SW_HIDE);

	((CEdit*)GetDlgItem(IDC_LOGIN_EDIT))->SetLimitText(12);

	m_pictBG.LoadPicture(IDP_BG_LOGIN);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoginDlg::OnLogin() 
{
	unsigned char	lpszBuffer[16];
	CMD5		md5;
	CString		csHash;
	char		httpBuff[1024];
	int			nBytes;
	CString		csBuff;
	CWaitCursor	wc;
	CString		csUrl;
	string		Version = getVersion();
	CTime		t	= CTime::GetCurrentTime();

	APP.m_bAuthGame	= FALSE;

	GetDlgItemText(IDC_LOGIN_EDIT, APP.m_csLogin);
	GetDlgItemText(IDC_PWD_EDIT, APP.m_csPassword);
	SetDlgItemText(IDC_ERR_STATIC, "");	

	md5.SetContent(APP.m_csLogin + APP.m_csPassword + t.Format("%Y%m%d") + MAGIC_KEY_MD5);
	md5.GetDigest(lpszBuffer);
	csHash	= md5.ConvertToAscii(lpszBuffer);

	csUrl = "http://" + APP.m_config.m_csHost + APP.m_config.m_csUrlMain + "?digest=" + csHash + "&newlogin=" + APP.m_csLogin + "&clientApplication=ryzom";

	TRY 
	{
		CInternetSession session("nel_launcher");
		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000);
		session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 3);
		CFile*	pf	= session.OpenURL(csUrl, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);

		while(nBytes = pf->Read(httpBuff, 1024)) 
		{
			csBuff	+= httpBuff;
		}
	}
	CATCH_ALL(error)
	{
		SetDlgItemText(IDC_ERR_STATIC, "Cannot connect to server !");
		return;
	}
	END_CATCH_ALL;

	if(csBuff.Find("<!-- DEBUG: id:") != -1)
	{
		// Authentication OK !
		APP.m_bAuthGame	= TRUE;
		APP.SetRegKey(_T("Pseudo"), APP.m_csLogin);
		APP.SetRegKey(_T("Login"), "yes");

		AuthWeb();

		EndDialog(IDOK);
	}
	else
	{
		APP.m_bAuthGame	= FALSE;
		SetDlgItemText(IDC_ERR_STATIC, "Invalid login or password !");
	}
}

BOOL CLoginDlg::AuthWeb()
{
	CString csBuff;
	char		httpBuff[1024];
	CString		csHash;
	CMD5		md5SPIP;
	int			nBytes;
	unsigned char	lpszBuffer[16];

	// Authentication for web pages
	TRY 
	{
		CInternetSession session;
		CString csIdSession, csSession;

		csBuff.Empty();
		memset(httpBuff, 0, 1024);
		session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000);
		session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 3);

		CFile*	pf	= session.OpenURL(URL_LOGIN_WEB, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);

		while(nBytes = pf->Read(httpBuff, 1024)) 
		{
			csBuff	+= httpBuff;
		}
		pf->Close();
		if(csBuff.Find('/') != -1)
		{
			csSession	= csBuff.Left(csBuff.Find('/'));
			csIdSession	= csBuff.Mid(csSession.GetLength()+1);

			md5SPIP.SetContent(APP.m_csPassword + csSession);
			memset(lpszBuffer, 0, 16);
			md5SPIP.GetDigest(lpszBuffer);
			csHash	= md5SPIP.ConvertToAscii(lpszBuffer);
			APP.m_bAuthWeb	= TRUE;

			TRY
			{
				CHttpConnection*	phttp	= session.GetHttpConnection(_T(RYZOM_HOST));
				CHttpFile*			pfile	= phttp->OpenRequest(CHttpConnection::HTTP_VERB_POST, "/betatest/betatest_login_valid.php");
				if(pfile)
				{
					CString csHeaders = _T("Content-Type: application/x-www-form-urlencoded");
					CString csFormParams = _T("txtLogin="+APP.m_csLogin+"&digest="+csHash+"&idsession="+csIdSession);
					
					TRY
					{
						csBuff.Empty();
						pfile->SendRequest(csHeaders, (LPVOID)(LPCTSTR)csFormParams, csFormParams.GetLength());

					   UINT nRead = pfile->Read(csBuff.GetBuffer(15000), 14999);
					   csBuff.ReleaseBuffer();
					   csBuff.SetAt(nRead, 0);
					   if(csBuff.Find("/news/") == -1)
					   {
							APP.m_bAuthWeb	= FALSE;
					   }
					}
					CATCH_ALL(error)
					{
						APP.m_bAuthWeb	= FALSE;
					}
					END_CATCH_ALL;

					delete pfile;
				}
				else
				{
					APP.m_bAuthWeb	= FALSE;
				}
			}
			CATCH_ALL(error)
			{
				APP.m_bAuthWeb	= FALSE;
			}
			END_CATCH_ALL;
		}
	}
	CATCH_ALL(error)
	{
		APP.m_bAuthWeb	= FALSE;
	}
	END_CATCH_ALL;

	return APP.m_bAuthWeb;
}

void CLoginDlg::OnQuit() 
{
	EndDialog(IDCANCEL);
}

void CLoginDlg::OnOK()
{
	OnLogin();
}

BOOL CLoginDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	POINT	p;
	CRect	r1, r2;

	GetCursorPos(&p);

	GetDlgItem(IDC_LOGIND_STATIC)->GetWindowRect(&r1);
	GetDlgItem(IDC_QUITD_STATIC)->GetWindowRect(&r2);

	if(PtInRect(&r1, p) || PtInRect(&r2, p))
		SetCursor(APP.m_hcPointer);
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
	return TRUE;
}


void CLoginDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	POINT	p;
	CRect	r1, r2;

	SetCapture();

	GetCursorPos(&p);

	GetDlgItem(IDC_LOGIND_STATIC)->GetWindowRect(&r1);
	GetDlgItem(IDC_QUITD_STATIC)->GetWindowRect(&r2);
	
	if(PtInRect(&r1, p))
	{
		GetDlgItem(IDC_LOGIND_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LOGINU_STATIC)->ShowWindow(SW_HIDE);
	}
	else if(PtInRect(&r2, p))
	{
		GetDlgItem(IDC_QUITD_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUITU_STATIC)->ShowWindow(SW_HIDE);
	}
	else
		CDialog::OnLButtonDown(nFlags, point);
}

void CLoginDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	POINT	p;
	CRect	r1, r2;

	ReleaseCapture();

	GetCursorPos(&p);

	GetDlgItem(IDC_LOGIND_STATIC)->GetWindowRect(&r1);
	GetDlgItem(IDC_QUITD_STATIC)->GetWindowRect(&r2);
	
	if(PtInRect(&r1, p))
	{
		GetDlgItem(IDC_LOGIND_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LOGINU_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LOGINU_STATIC)->UpdateWindow();
		OnLogin();
	}
	else if(PtInRect(&r2, p))
	{
		GetDlgItem(IDC_QUITD_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUITU_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUITU_STATIC)->UpdateWindow();
		OnQuit();
	}
	else
	{
		GetDlgItem(IDC_LOGIND_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LOGINU_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUITD_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUITU_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LOGINU_STATIC)->UpdateWindow();
		GetDlgItem(IDC_QUITU_STATIC)->UpdateWindow();
		CDialog::OnLButtonUp(nFlags, point);
	}
}

void CLoginDlg::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{	
	ReleaseCapture();

	GetDlgItem(IDC_LOGIND_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LOGINU_STATIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_QUITD_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_QUITU_STATIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_LOGINU_STATIC)->UpdateWindow();
	GetDlgItem(IDC_QUITU_STATIC)->UpdateWindow();

	CDialog::OnNcLButtonUp(nHitTest, point);
}

BOOL CLoginDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect	r;

	GetClientRect(&r);
	m_pictBG.Display(*pDC, r);

    return TRUE;

}
