// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define POS_PROGRESS_X	100
#define POS_PROGRESS_Y	32
#define POS_PROGRESS_W	300
#define POS_PROGRESS_H	6
#define TXT_PROGRESS_Y	5
#define TXT_PROGRESS_H	15
#define BG_MSG_CLR		RGB(35, 64, 44)

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog


CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_font.CreateFont(12, 6, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, "Arial");
	m_brushBG.CreateSolidBrush(BG_MSG_CLR);
}


void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers
void CProgressDlg::Show(BOOL bShow)
{
	ShowWindow(bShow);
}

void CProgressDlg::SetRange(int iRange)
{
	m_wndBar.SetRange(iRange);
}

void CProgressDlg::UpdatePos(int iPos)
{
	m_wndBar.UpdatePos(iPos);
}

void CProgressDlg::UpdateMsg(CString csMsg)
{
	CString	csLast;

	GetDlgItemText(IDC_MSG_PROGRESS_STATIC, csLast);
	if(csMsg != csLast)
		for(m_iClr = 0; m_iClr < 256; m_iClr += 2)
		{
			SetDlgItemText(IDC_MSG_PROGRESS_STATIC, csMsg);
			::Sleep(2);
		}
}

void CProgressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

BOOL CProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	LPCTSTR	szClass	= AfxRegisterWndClass(NULL);
	CRect	r;

	GetDlgItem(IDC_MSG_PROGRESS_STATIC)->SetFont(&m_font);

	m_wndBar.Create(szClass, _T("PatchBar"), WS_CHILD | WS_VISIBLE, r, this, 1234);
	m_wndBar.MoveWindow(POS_PROGRESS_X, POS_PROGRESS_Y, POS_PROGRESS_W, POS_PROGRESS_H);
	GetDlgItem(IDC_MSG_PROGRESS_STATIC)->MoveWindow(POS_PROGRESS_X+1, TXT_PROGRESS_Y, POS_PROGRESS_W-2, TXT_PROGRESS_H);

	m_pictBG.LoadPicture(IDP_PROGRESS);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProgressDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	m_wndBar.DestroyWindow();
}

HBRUSH CProgressDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(nCtlColor == CTLCOLOR_STATIC)
	{
		hbr	= m_brushBG;
		pDC->SetBkColor(BG_MSG_CLR);
		pDC->SetTextColor(RGB(std::max(m_iClr-100, 0), m_iClr, int(m_iClr/2)));
	}
	return hbr;
}

BOOL CProgressDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect	r;

	GetClientRect(&r);
	m_pictBG.Display(*pDC, r);

    return TRUE;
}
