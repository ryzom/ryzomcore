// set_value_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "set_value_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSetValueDlg dialog


CSetValueDlg::CSetValueDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetValueDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetValueDlg)
	Value = _T("");
	//}}AFX_DATA_INIT
}


void CSetValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetValueDlg)
	DDX_Text(pDX, IDC_VALUE, Value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetValueDlg, CDialog)
	//{{AFX_MSG_MAP(CSetValueDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetValueDlg message handlers

BOOL CSetValueDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetWindowText (Title);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
