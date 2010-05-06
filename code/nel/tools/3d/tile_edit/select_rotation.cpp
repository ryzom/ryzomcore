// select_rotation.cpp : implementation file
//

#include "stdafx.h"
#include "tile_edit_exe.h"
#include "select_rotation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SelectRotation dialog


SelectRotation::SelectRotation(CWnd* pParent /*=NULL*/)
	: CDialog(SelectRotation::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectRotation)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SelectRotation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectRotation)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SelectRotation, CDialog)
	//{{AFX_MSG_MAP(SelectRotation)
	ON_BN_CLICKED(ID_ROT0, OnRot0)
	ON_BN_CLICKED(ID_ROT1, OnRot1)
	ON_BN_CLICKED(ID_ROT2, OnRot2)
	ON_BN_CLICKED(ID_ROT3, OnRot3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectRotation message handlers

void SelectRotation::OnRot0() 
{
	// TODO: Add your control notification handler code here
	RotSelected=0;
	OnOK ();
}

void SelectRotation::OnRot1() 
{
	// TODO: Add your control notification handler code here
	RotSelected=1;
	OnOK ();
}

void SelectRotation::OnRot2() 
{
	// TODO: Add your control notification handler code here
	RotSelected=2;
	OnOK ();
}

void SelectRotation::OnRot3() 
{
	// TODO: Add your control notification handler code here
	RotSelected=3;
	OnOK ();
}
