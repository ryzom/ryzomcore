#if !defined(AFX_TMENU_H__3C1D1728_241C_11D3_9CE5_0060973674E2__INCLUDED_)
#define AFX_TMENU_H__3C1D1728_241C_11D3_9CE5_0060973674E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TMenu.h : header file
//

// Titled menu class

/////////////////////////////////////////////////////////////////////////////
// CTMenu window

  
class CTMenu : public CMenu
{
// Construction
public:
	CTMenu();

// Attributes
protected:
	CFont m_Font;
	CString m_strTitle;

// Operations
public:
	void AddMenuTitle(LPCTSTR lpszTitle);

protected:
	HFONT CreatePopupMenuTitleFont();

// Implementation
public:
	virtual ~CTMenu();
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMENU_H__3C1D1728_241C_11D3_9CE5_0060973674E2__INCLUDED_)
