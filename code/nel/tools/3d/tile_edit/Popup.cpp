// Popup.cpp : implementation file
//

#include "stdafx.h"
#include "tile_edit.h"
#include "Popup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Popup

Popup::Popup()
{
}

Popup::~Popup()
{
}

LRESULT Popup::DefWindowProc(UINT message,WPARAM wParam,LPARAM lParam)
{
/*	if (message==WM_INIT)
	{
		int toto = 0;
	}*/
	if (message==WM_PAINT)
	{
		int toto = 0;
	}
	return CWnd::DefWindowProc(message,wParam,lParam);
}

BEGIN_MESSAGE_MAP(Popup, CWnd)
	//{{AFX_MSG_MAP(Popup)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Popup message handlers
