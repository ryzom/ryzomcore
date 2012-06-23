// BarWnd.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "BarWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*#define R1	50
#define G1	169
#define B1	143*/
#define R1	20
#define G1	70
#define B1	50
#define R2	0
#define G2	255
#define B2	192

/////////////////////////////////////////////////////////////////////////////
// CBarWnd

CBarWnd::CBarWnd()
{
	m_iPos		= 0;
	m_iRange	= 100;
}

CBarWnd::~CBarWnd()
{
}


BEGIN_MESSAGE_MAP(CBarWnd, CWnd)
	//{{AFX_MSG_MAP(CBarWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBarWnd message handlers

void CBarWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect	r;

	GetClientRect(&r);

	if(r.Width() == 0 || m_iRange == 0)
		return;

	int		iWidth	= m_iPos * r.Width() / m_iRange; 
	double	iRatio;
	int		iR, iG, iB;

	for(int i = 0; i < iWidth; i++)
	{
		iRatio	= double(i) / double(r.Width());
		iR	= int(iRatio * (R2 - R1) + R1);
		iG	= int(iRatio * (G2 - G1) + G1);
		iB	= int(iRatio * (B2 - B1) + B1);
		if(i < 3)
			dc.FillSolidRect(i+1, 0, 1, r.Height(), RGB(iR/1.5, iG/1.5, iB/1.5));
		else
		{
			dc.FillSolidRect(i+1, 0, 1, r.Height(), RGB(iR, iG, iB));
			dc.SetPixelV(i+1, 0, RGB(iR/1.5, iG/1.5, iB/1.5));
			dc.SetPixelV(i+1, 1, RGB(iR/1.5, iG/1.5, iB/1.5));
		}
	}
	dc.FillSolidRect(iWidth+1, 0, r.Width() - iWidth-2, r.Height(), RGB(0, 32, 0));
}

void CBarWnd::SetRange(int iRange)
{
	m_iRange	= iRange;
}

void CBarWnd::UpdatePos(int iPos)
{
	m_iPos	= iPos;
	Invalidate(FALSE);
}
