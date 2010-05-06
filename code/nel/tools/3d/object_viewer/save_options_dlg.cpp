// save_options_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "save_options_dlg.h"


//*************************************************************************************************
CSaveOptionsDlg::CSaveOptionsDlg(const CString &caption, const CString &message, CWnd* pParent /*=NULL*/)
	: CDialog(CSaveOptionsDlg::IDD, pParent)
{
	_Choice = Stop;
	_Caption = caption;
	_Message = message;
	//{{AFX_DATA_INIT(CSaveOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//*************************************************************************************************
void CSaveOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveOptionsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveOptionsDlg)
	ON_BN_CLICKED(ID_SAVEALL, OnSaveAll)
	ON_BN_CLICKED(ID_NO, OnNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*************************************************************************************************
void CSaveOptionsDlg::OnOK() 
{	
	_Choice = Yes;
	CDialog::OnOK();
}

//*************************************************************************************************
void CSaveOptionsDlg::OnSaveAll() 
{
	_Choice = SaveAll;
	CDialog::OnOK();
}


//*************************************************************************************************
void CSaveOptionsDlg::OnCancel() 
{
	_Choice = Stop;	
	CDialog::OnCancel();
}

//*************************************************************************************************
void CSaveOptionsDlg::OnNo() 
{
	_Choice = No;
	CDialog::OnOK();	
}

//*************************************************************************************************
BOOL CSaveOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetWindowText((LPCTSTR)_Caption);
	GetDlgItem(IDC_MB_TEXT)->SetWindowText((LPCTSTR) _Message);		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}