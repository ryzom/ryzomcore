// blended_bitmap.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "blended_bitmap.h"
#include "nel/misc/time_nl.h"


/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

/////////////////////////////////////////////////////////////////////////////
// CBlendedBitmap

CBlendedBitmap::CBlendedBitmap() : CWnd(), _BMRes(-1), _DIB(0)
{
	_BGColor = ::GetSysColor(_BGSysColor);	
	_BGSysColor = COLOR_3DFACE;
	resetDIBRect();
}

CBlendedBitmap::~CBlendedBitmap()
{
	releaseDIB();
}


BEGIN_MESSAGE_MAP(CBlendedBitmap, CWnd)
	//{{AFX_MSG_MAP(CBlendedBitmap)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBlendedBitmap::releaseDIB()
{
	if (_DIB)
	{
		DeleteObject(_DIB);
		_DIB = 0;
		resetDIBRect();
	}
}

void CBlendedBitmap::resetDIBRect()
{
	_DIBRect.left   = 0;
	_DIBRect.right  = 0;
	_DIBRect.top    = 0;
	_DIBRect.bottom = 0;
}

void CBlendedBitmap::setBitmap(int bitmapRes)
{
	if (bitmapRes == _BMRes) return;	
	releaseDIB();
	BITMAP dibInfo;
	_DIB = (HBITMAP) LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(bitmapRes), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (_DIB)
	{		
		if (!GetObject(_DIB, sizeof(BITMAP), (void *) &dibInfo))
		{						
			releaseDIB();
		}
		_DIBRect.left   = 0;
		_DIBRect.top    = 0;
		_DIBRect.right  = dibInfo.bmWidth;
		_DIBRect.bottom = dibInfo.bmHeight;
		_SrcRect = _DIBRect;
	}

	_BMRes = bitmapRes;
	Invalidate();	
}

void CBlendedBitmap::setSrcRect(const RECT &r)
{
	_SrcRect = r;
	Invalidate();
}


void CBlendedBitmap::setBgColor(COLORREF color)
{
	if (color == _BGColor) return;
	_BGColor = color;
	_BGSysColor = -1;

	Invalidate();
}

void CBlendedBitmap::setSysBgColor(int sysColor)
{
	if (sysColor == _BGSysColor) return;	
	_BGSysColor = sysColor;
	Invalidate();
}



/////////////////////////////////////////////////////////////////////////////
// CBlendedImage message handlers

void CBlendedBitmap::OnPaint() 
{	
	CPaintDC dc(this); // device context for painting
	::CRect r;
	GetClientRect(&r);
	COLORREF bgColor = (_BGSysColor != - 1) ? GetSysColor(_BGSysColor) : _BGColor;
	if (!_DIB)
	{		
		dc.FillSolidRect(&r, bgColor);		
		return;
	}		
	CDC memoryDC;
	memoryDC.CreateCompatibleDC(&dc);
	CBitmap targetBitmap;	
	targetBitmap.CreateCompatibleBitmap(&dc, r.right - r.left, r.bottom - r.top);
	/* CGdiObject *oldMemoryDCObj =  (CGdiObject *) */ memoryDC.SelectObject(targetBitmap);
	CDC srcDC;
	srcDC.CreateCompatibleDC(&dc);
	// selecting a dib here garantee that no conversion occurs ; alpha is preserved
	/* CGdiObject *oldSRCDCObj = (CGdiObject *) */ srcDC.SelectObject(_DIB);
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;	
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = 0x01; // AC_SRC_ALPHA		
	memoryDC.FillSolidRect(&r, bgColor);
	BOOL res = ::AlphaBlend(memoryDC.m_hDC, r.left, r.top, r.right - r.left, r.bottom - r.top, srcDC.m_hDC, _SrcRect.left, _SrcRect.top, _SrcRect.right - _SrcRect.left, _SrcRect.bottom - _SrcRect.top, bf);
	nlassert(res != ERROR_INVALID_PARAMETER);	
	// blit from memory dc into target area to have flicker free display
	dc.BitBlt(r.left, r.top, r.right - r.left, r.bottom - r.top, &memoryDC, 0, 0, SRCCOPY);	

	//srcDC.SelectObject(oldSRCDCObj);
	//memoryDC.SelectObject(oldMemoryDCObj);

}


BOOL CBlendedBitmap::OnEraseBkgnd(CDC* pDC) 
{
	
	return CWnd::OnEraseBkgnd(pDC);
}

