// choose_bg_color_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_bg_color_dlg.h"
#include "color_edit.h"



/////////////////////////////////////////////////////////////////////////////
// CChooseBGColorDlg dialog


CChooseBGColorDlg::CChooseBGColorDlg(CObjectViewer *objectViewer, CWnd* pParent)
	: CDialog(CChooseBGColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseBGColorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	nlassert(objectViewer);
	_BGColorWrapper.OV = objectViewer;
	_ColorEdit = new CColorEdit(this);
	_ColorEdit->setWrapper(&_BGColorWrapper);
}

CChooseBGColorDlg::~CChooseBGColorDlg()
{
	_ColorEdit->DestroyWindow();
	delete _ColorEdit;
}



void CChooseBGColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseBGColorDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseBGColorDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseBGColorDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseBGColorDlg message handlers

BOOL CChooseBGColorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RECT r;
	GetDlgItem(IDC_BG_COLOR)->GetWindowRect(&r);
	ScreenToClient(&r);
	_ColorEdit->init(r.left, r.top, this);		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseBGColorDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_CHOOSE_BG_COLOR_DLG);		
	CDialog::OnDestroy();	
}
