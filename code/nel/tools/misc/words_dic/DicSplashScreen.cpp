// DicSplashScreen.cpp : implementation file
//

#include "stdafx.h"
#include "words_dic.h"
#include "DicSplashScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDicSplashScreen dialog


CDicSplashScreen::CDicSplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(CDicSplashScreen::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDicSplashScreen)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDicSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDicSplashScreen)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDicSplashScreen, CDialog)
	//{{AFX_MSG_MAP(CDicSplashScreen)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDicSplashScreen message handlers
