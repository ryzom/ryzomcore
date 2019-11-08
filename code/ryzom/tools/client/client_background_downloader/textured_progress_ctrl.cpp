// textured_progress_ctrl.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "textured_progress_ctrl.h"



/////////////////////////////////////////////////////////////////////////////
// CTexturedProgressCtrl

CTexturedProgressCtrl::CTexturedProgressCtrl() : _Initialized(false)
{
}

CTexturedProgressCtrl::~CTexturedProgressCtrl()
{
}


void CTexturedProgressCtrl::init(CBitmap &bg, CBitmap &fg)
{
	_BG.Attach(bg.Detach());
	_FG.Attach(fg.Detach());	
}


BEGIN_MESSAGE_MAP(CTexturedProgressCtrl, CProgressCtrl)
	//{{AFX_MSG_MAP(CTexturedProgressCtrl)
	ON_WM_PAINT()	
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTexturedProgressCtrl message handlers

void CTexturedProgressCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect r;
	GetClientRect(r);		
	CDC srcDC;
	srcDC.CreateCompatibleDC(&dc);

	int rangeLower, rangeUpper;
	GetRange(rangeLower, rangeUpper);
	float progress = (float) (GetPos() - rangeLower) / (rangeUpper - rangeLower);
	int size = (int) ((r.right - r.left) * progress);
	/* CGdiObject *srcDCOldObj =*/ srcDC.SelectObject(&_FG);
	dc.BitBlt(r.left, r.top, size, r.bottom - r.top, &srcDC, 0, 0, SRCCOPY);	
	srcDC.SelectObject(&_BG);
	dc.BitBlt(r.left + size, r.top, (r.right - r.left) - size, r.bottom - r.top, &srcDC, size, 0, SRCCOPY);		
	
	std::string percentStr = NLMISC::toString("%d%%", (int) (100.f * progress));


	CSize sz = dc.GetTextExtent(percentStr.c_str(), percentStr.size());
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(0, 0, 0));
	dc.TextOut(1 + r.left + (r.Width() - sz.cx) / 2, 1 + r.top + (r.Height() - sz.cy) / 2, percentStr.c_str(), percentStr.size());
	dc.SetTextColor(RGB(255, 255, 255));
	dc.TextOut(r.left + (r.Width() - sz.cx) / 2, r.top + (r.Height() - sz.cy) / 2, percentStr.c_str(), percentStr.size());

	//srcDC.SelectObject(srcDCOldObj);

}



void CTexturedProgressCtrl::OnNcPaint() 
{
	// no-op (remove border)
}
