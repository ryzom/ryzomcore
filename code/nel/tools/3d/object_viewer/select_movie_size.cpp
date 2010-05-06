// SelectMovieSize.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "select_movie_size.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectMovieSize dialog


CSelectMovieSize::CSelectMovieSize(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectMovieSize::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMovieSize)
	Width = 800;
	Height = 600;
	//}}AFX_DATA_INIT
}


void CSelectMovieSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMovieSize)
	DDX_Text(pDX, IDC_WIDTH, Width);
	DDX_Text(pDX, IDC_HEIGHT, Height);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectMovieSize, CDialog)
	//{{AFX_MSG_MAP(CSelectMovieSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectMovieSize message handlers

void CSelectMovieSize::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
