// MsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "MsgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgDlg dialog


CMsgDlg::CMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_fMsg.CreateFont(12, 6, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, "Arial");
	m_fTitle.CreateFont(14, 8, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, "Arial");
}

CMsgDlg::CMsgDlg(CString csTitle, CString csMsg, CWnd* pParent /*=NULL*/)
	: CDialog(CMsgDlg::IDD, pParent)
{
	m_csTitle	= csTitle;
	m_csMsg		= csMsg;
}

void CMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgDlg)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgDlg message handlers

BOOL CMsgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetDlgItemText(IDC_TITLE_STATIC, m_csTitle);
	SetDlgItemText(IDC_MSG_STATIC, m_csMsg);

	GetDlgItem(IDC_TITLE_STATIC)->SetFont(&m_fTitle);
	GetDlgItem(IDC_MSG_STATIC)->SetFont(&m_fMsg);

	m_brush.CreateStockObject(NULL_BRUSH);
	
	m_pictBG.LoadPicture(IDP_BACKGROUND);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CMsgDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(pWnd->GetDlgCtrlID() == IDC_MSG_STATIC)
	{
		hbr	= m_brush;
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 0));
	}
	else if(pWnd->GetDlgCtrlID() == IDC_TITLE_STATIC)
	{
		hbr	= m_brush;
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 127));
	}
	return hbr;
}

BOOL CMsgDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect	r;

	GetClientRect(&r);
	m_pictBG.Display(*pDC, r, 0, -240);

    return TRUE;
}
