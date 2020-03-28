// progress_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "data_mirror.h"
#include "progress_dialog.h"

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog


#define BAR_MAX 1000

CProgressDialog::CProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDialog)
	Text = _T("");
	//}}AFX_DATA_INIT
}


void CProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDialog)
	DDX_Control(pDX, IDC_PROGRESS, Bar);
	DDX_Text(pDX, IDC_TEXT, Text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
	//{{AFX_MSG_MAP(CProgressDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog message handlers

void CProgressDialog::progress (float progressValue)
{
	UpdateData ();

	float value = getCropedValue (progressValue);
	Text = DisplayString.c_str ();
	Bar.SetPos ((uint) (value * (float)BAR_MAX));

	UpdateData (FALSE);
}

BOOL CProgressDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	Bar.SetRange (0, BAR_MAX);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
