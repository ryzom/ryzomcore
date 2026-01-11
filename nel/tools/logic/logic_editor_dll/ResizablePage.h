#if !defined(AFX_RESIZABLEPAGE_H__INCLUDED_)
#define AFX_RESIZABLEPAGE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ResizablePage.h : header file
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000 by Paolo Messina
// (ppescher@yahoo.com)
//
// Free for non-commercial use.
// You may change the code to your needs,
// provided that credits to the original 
// author is given in the modified files.
//  
/////////////////////////////////////////////////////////////////////////////

// useful compatibility constants (the only one required is NOANCHOR)

#if !defined(__SIZE_ANCHORS_)
#define __SIZE_ANCHORS_

const CSize
	NOANCHOR(-1,-1),
	TOP_LEFT(0,0), TOP_CENTER(50,0), TOP_RIGHT(100,0),
	MIDDLE_LEFT(0,50), MIDDLE_CENTER(50,50), MIDDLE_RIGHT(100,50),
	BOTTOM_LEFT(0,100), BOTTOM_CENTER(50,100), BOTTOM_RIGHT(100,100);

#endif // !defined(__SIZE_ANCHORS_)

/////////////////////////////////////////////////////////////////////////////
// CResizablePage window

class CResizablePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CResizablePage)

// Construction
public:
	CResizablePage();
	CResizablePage(UINT nIDTemplate, UINT nIDCaption = 0);
	CResizablePage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);

// Attributes
public:

private:
	// internal status
	CString m_sSection;			// section name and
	CString m_sEntry;			// entry for save/restore

	BOOL m_bInitDone;			// if all internal vars initialized

	CPtrList m_plLayoutList;	// list of repositionable controls

	class Layout
	{
	public:
		HWND hwnd;

		BOOL adj_hscroll;
		BOOL need_refresh;

		// upper-left corner
		SIZE tl_type;
		SIZE tl_margin;
		
		// bottom-right corner
		SIZE br_type;
		SIZE br_margin;
	
	public:
		Layout(HWND hw, SIZE tl_t, SIZE tl_m, 
			SIZE br_t, SIZE br_m, BOOL hscroll, BOOL refresh)
		{
			hwnd = hw;

			adj_hscroll = hscroll;
			need_refresh = refresh;

			tl_type = tl_t;
			tl_margin = tl_m;
			
			br_type = br_t;
			br_margin = br_m;
		};
	};

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizablePage)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResizablePage();

// used internally
private:
	void Construct();
	void ArrangeLayout();

// callable from derived classes
protected:
	void AddAnchor(HWND wnd, CSize tl_type,
		CSize br_type = NOANCHOR);	// add anchors to a control
	void AddAnchor(UINT ctrl_ID, CSize tl_type,
		CSize br_type = NOANCHOR)	// add anchors to a control
	{
		AddAnchor(::GetDlgItem(*this, ctrl_ID), tl_type, br_type);
	};

// Generated message map functions
protected:
	//{{AFX_MSG(CResizablePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZABLEPAGE_H__INCLUDED_)
