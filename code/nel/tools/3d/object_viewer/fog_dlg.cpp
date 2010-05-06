// fog_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "fog_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CFogDlg dialog


CFogDlg::CFogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFogDlg)
	m_FogStart = 0.0f;
	m_FogEnd = 100.0f;
	//}}AFX_DATA_INIT
}


void CFogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFogDlg)
	DDX_Text(pDX, IDC_EDIT1, m_FogStart);
	DDX_Text(pDX, IDC_EDIT2, m_FogEnd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFogDlg, CDialog)
	//{{AFX_MSG_MAP(CFogDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFogDlg message handlers
