/*
 * $Id:
 */

// CloseWarning.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "CloseWarning.h"

#include "nel/misc/i18n.h"


using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CCloseWarning dialog

CCloseWarning::CCloseWarning(CWnd* pParent /*=NULL*/)
	: CDialog(CCloseWarning::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCloseWarning)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCloseWarning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCloseWarning)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCloseWarning, CDialog)
	//{{AFX_MSG_MAP(CCloseWarning)
	ON_WM_CLOSE()
	ON_COMMAND(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCloseWarning message handlers

BOOL CCloseWarning::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetIcon(AfxGetApp()->LoadIcon(IDI_MAIN_ICON), TRUE);
	GetDlgItem(IDC_CLOSE_WARNING)->SetWindowText(CI18N::get("uiBGD_CloseWarning").toString().c_str());
	GetDlgItem(IDC_DONT_SHOW_AGAIN)->SetWindowText(CI18N::get("uiBGD_DontShowAgain").toString().c_str());
	GetDlgItem(IDOK)->SetWindowText(CI18N::get("uiBGD_OK").toString().c_str());
	this->SetWindowText(CI18N::get("uiBGD_RBG").toString().c_str());
	((CButton *) GetDlgItem(IDC_DONT_SHOW_AGAIN))->SetCheck(theApp.getCloseWarningFlag() ? 0 : 1);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCloseWarning::OnOK()
{
	theApp.setCloseWarningFlag(((CButton *) GetDlgItem(IDC_DONT_SHOW_AGAIN))->GetCheck() ? 0 : 1);
	CDialog::OnOK();
}


void CCloseWarning::OnClose() 
{	
	CDialog::OnClose();	
}
