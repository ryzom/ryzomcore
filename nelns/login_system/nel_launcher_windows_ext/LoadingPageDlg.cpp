// LoadingPageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "LoadingPageDlg.h"
#include "nel_launcherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadingPageDlg dialog


CLoadingPageDlg::CLoadingPageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadingPageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadingPageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoadingPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadingPageDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadingPageDlg, CDialog)
	//{{AFX_MSG_MAP(CLoadingPageDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadingPageDlg message handlers

BOOL CLoadingPageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//	GetDlgItem(IDC_BG_STATIC)->MoveWindow(-BROWSER_X, -BROWSER_Y, BG_W, BG_H);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
