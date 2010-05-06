// dialog_progress.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "dialog_progress.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogProgress dialog


CDialogProgress::CDialogProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogProgress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogProgress)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogProgress)
	DDX_Control(pDX, IDC_STATIC_PROGRESS_DESC, ProgressText);
	DDX_Control(pDX, IDC_PROGRESS1, ProgressBar);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogProgress, CDialog)
	//{{AFX_MSG_MAP(CDialogProgress)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogProgress message handlers
