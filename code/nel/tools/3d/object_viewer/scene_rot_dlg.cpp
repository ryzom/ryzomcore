// scene_rot_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "scene_rot_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSceneRotDlg dialog


CSceneRotDlg::CSceneRotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSceneRotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSceneRotDlg)
	RotX = _T("");
	RotY = _T("");
	RotZ = _T("");
	//}}AFX_DATA_INIT
}


void CSceneRotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneRotDlg)
	DDX_Text(pDX, IDC_ROTX, RotX);
	DDX_Text(pDX, IDC_ROTY, RotY);
	DDX_Text(pDX, IDC_ROTZ, RotZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSceneRotDlg, CDialog)
	//{{AFX_MSG_MAP(CSceneRotDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneRotDlg message handlers
