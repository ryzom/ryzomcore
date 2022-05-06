#if !defined(AFX_TILELIST_H__89BE3DF9_A079_4616_BE82_6BC2A3825736__INCLUDED_)
#define AFX_TILELIST_H__89BE3DF9_A079_4616_BE82_6BC2A3825736__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TileList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TileList window

class TileList : public CListBox
{
// Construction
public:
	TileList();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TileList)
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );	
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TileList();

	// Generated message map functions
protected:
	//{{AFX_MSG(TileList)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TILELIST_H__89BE3DF9_A079_4616_BE82_6BC2A3825736__INCLUDED_)
