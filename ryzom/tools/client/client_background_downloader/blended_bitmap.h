#if !defined(AFX_BLENDED_BITMAP_H__32849754_C90E_430D_8213_CC87BA8725B8__INCLUDED_)
#define AFX_BLENDED_BITMAP_H__32849754_C90E_430D_8213_CC87BA8725B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// blended_bitmap.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CBlendedBitmap window

class CBlendedBitmap : public CWnd
{
// Construction
public:
	CBlendedBitmap();

	void setBitmap(int bitmapRes);
	void setBgColor(COLORREF color);
	void setSysBgColor(int sysColor = COLOR_3DFACE);

	// Set the source rectangle to be blitted from the original bitmap
	void setSrcRect(const RECT &r);
	void getBitmapRect(RECT &dest) const { dest = _DIBRect; }

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlendedBitmap)
	protected:	
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBlendedBitmap();

	// Generated message map functions
protected:
	RECT		_SrcRect;
	RECT		_DIBRect;
	HBITMAP		_DIB; 		
	int			_BMRes;
	COLORREF	_BGColor;
	int			_BGSysColor;
	//{{AFX_MSG(CBlendedBitmap)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);	
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	void resetDIBRect();
	void releaseDIB();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLENDED_BITMAP_H__32849754_C90E_430D_8213_CC87BA8725B8__INCLUDED_)
