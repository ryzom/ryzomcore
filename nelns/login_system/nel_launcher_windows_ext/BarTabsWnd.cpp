// BarTabsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "BarTabsWnd.h"
#include "PictureHlp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBarTabsWnd

CBarTabsWnd::CBarTabsWnd()
{
	m_iTab	= 0;
	m_pobs	= NULL;
}

CBarTabsWnd::~CBarTabsWnd()
{
	Reset();
}


BEGIN_MESSAGE_MAP(CBarTabsWnd, CWnd)
	//{{AFX_MSG_MAP(CBarTabsWnd)
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBarTabsWnd message handlers
void CBarTabsWnd::AddTab(int iID, int iIDFocus)
{
	CPictureHlp*	ppict		= new CPictureHlp;
	CPictureHlp*	ppictFocus	= new CPictureHlp;

	ppict->LoadPicture(iID);
	ppictFocus->LoadPicture(iIDFocus);
	m_parrBmp.Add(ppict);
	m_parrBmpFocus.Add(ppictFocus);
}

int	CBarTabsWnd::GetNbTabs()
{
	return m_parrBmp.GetSize();
}

void CBarTabsWnd::SetFocusPos(int iTab)
{
	if(iTab < GetNbTabs())
	{
		m_iTab	= iTab;
		Invalidate(FALSE);
	}
}

int CBarTabsWnd::GetFocusPos()
{
	return m_iTab;
}

void CBarTabsWnd::Move(int iX, int iY)
{
	MoveWindow(iX, iY, TAB_W * GetNbTabs(), TAB_H);
}

void CBarTabsWnd::SetObserver(CTabsObserver* pobs)
{
	m_pobs	= pobs;
}

void CBarTabsWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int	iTab;

	if(point.y > 0 && point.y < TAB_H)
	{
		iTab	= point.x / TAB_W;
		if(iTab < GetNbTabs() && iTab >= 0 && iTab != m_iTab)
		{
			SetFocusPos(iTab);
			if(m_pobs)
				m_pobs->OnTab(iTab);
		}
	}
}

BOOL CBarTabsWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	POINT	p;
	int		iTab;

	GetCursorPos(&p);
	ScreenToClient(&p);
	iTab	= p.x / TAB_W;
	if(iTab == m_iTab)
		return CWnd::OnSetCursor(pWnd, nHitTest, message);
	else
		SetCursor(APP.m_hcPointer);
	return TRUE;
}

void CBarTabsWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	for(int i = 0; i < GetNbTabs(); i++)
	{
		CRect	r(0, 0, TAB_W, TAB_H);
		// Drawing the tab
		if(i == m_iTab)
			((CPictureHlp*)m_parrBmpFocus[i])->Display(dc, r, TAB_W*i, 0);
		else
			((CPictureHlp*)m_parrBmp[i])->Display(dc, r, TAB_W*i, 0);
	}
}

void CBarTabsWnd::Reset()
{
	int	nTabs	= GetNbTabs();

	for(int i = 0; i < nTabs; i++)
	{
		if(m_parrBmp[i])
			delete (CPictureHlp*)(m_parrBmp[i]);
		if(m_parrBmpFocus[i])
			delete (CPictureHlp*)(m_parrBmpFocus[i]);
	}
	m_parrBmp.RemoveAll();
	m_parrBmpFocus.RemoveAll();
}

void CBarTabsWnd::OnDestroy() 
{
	Reset();	
	
	CWnd::OnDestroy();	
}
