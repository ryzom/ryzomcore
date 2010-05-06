// choose_pool_id.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_pool_id.h"

/////////////////////////////////////////////////////////////////////////////
// CChoosePoolID dialog


CChoosePoolID::CChoosePoolID(bool freezeID , CWnd* pParent /*=NULL*/)
	: CDialog(CChoosePoolID::IDD, pParent),  _FreezeID(freezeID)
{
	//{{AFX_DATA_INIT(CChoosePoolID)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChoosePoolID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChoosePoolID)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChoosePoolID, CDialog)
	//{{AFX_MSG_MAP(CChoosePoolID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoosePoolID message handlers

void CChoosePoolID::OnOK() 
{
	CString val;
	GetDlgItem(IDC_POOL_ID)->GetWindowText(val);
	if (::sscanf((LPCTSTR) val, "%d", &PoolID) == 1)
	{
		GetDlgItem(IDC_POOL_NAME)->GetWindowText(val);
		Name = (LPCTSTR) val;
		CDialog::OnOK();
	}
	else
	{
		MessageBox("Invalid value", "error", MB_OK);
	}
}

BOOL CChoosePoolID::OnInitDialog() 
{
	CDialog::OnInitDialog();
	char val[128];
	sprintf(val, "%d", PoolID);
	GetDlgItem(IDC_POOL_ID)->SetWindowText(val);
	GetDlgItem(IDC_POOL_NAME)->SetWindowText(Name.c_str());
	if (_FreezeID)
	{
		GetDlgItem(IDC_POOL_ID)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
