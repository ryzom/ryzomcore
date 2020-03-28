// ResizablePage.cpp : implementation file
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

#include "stdafx.h"
#include "ResizablePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizablePage

IMPLEMENT_DYNCREATE(CResizablePage, CPropertyPage)

inline void CResizablePage::Construct()
{
	m_bInitDone = FALSE;
}

CResizablePage::CResizablePage()
{
	Construct();
}

CResizablePage::CResizablePage(UINT nIDTemplate, UINT nIDCaption)
	: CPropertyPage(nIDTemplate, nIDCaption)
{
	Construct();
}

CResizablePage::CResizablePage(LPCTSTR lpszTemplateName, UINT nIDCaption)
	: CPropertyPage(lpszTemplateName, nIDCaption)
{
	Construct();
}

CResizablePage::~CResizablePage()
{
	Layout *pl;

	POSITION pos = m_plLayoutList.GetHeadPosition();

	while (pos != NULL)
	{
		pl = (Layout*)m_plLayoutList.GetNext(pos);
		delete pl;
	}
}


BEGIN_MESSAGE_MAP(CResizablePage, CPropertyPage)
	//{{AFX_MSG_MAP(CResizablePage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CResizablePage message handlers


BOOL CResizablePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// gets the initial size as the min track size
	CRect rc;
	GetWindowRect(&rc);

	m_bInitDone = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResizablePage::AddAnchor(HWND wnd, CSize tl_type, CSize br_type)
{
	ASSERT(wnd != NULL && ::IsWindow(wnd));
	ASSERT(::IsChild(*this, wnd));
	ASSERT(tl_type != NOANCHOR);

	// get control's window class
	
	CString st;
	GetClassName(wnd, st.GetBufferSetLength(MAX_PATH), MAX_PATH);
	st.ReleaseBuffer();
	st.MakeUpper();

	// add the style 'clipsiblings' to a GroupBox
	// to avoid unnecessary repainting of controls inside
	if (st == "BUTTON")
	{
		DWORD style = GetWindowLong(wnd, GWL_STYLE);
		if (style & BS_GROUPBOX)
			SetWindowLong(wnd, GWL_STYLE, style | WS_CLIPSIBLINGS);
	}

	// wnd classes that don't redraw client area correctly
	// when the hor scroll pos changes due to a resizing
	BOOL hscroll = FALSE;
	if (st == "LISTBOX")
		hscroll = TRUE;

	// wnd classes that need refresh when resized
	BOOL refresh = FALSE;
	if (st == "STATIC")
	{
		DWORD style = GetWindowLong(wnd, GWL_STYLE);

		switch (style & SS_TYPEMASK)
		{
		case SS_LEFT:
		case SS_CENTER:
		case SS_RIGHT:
			// word-wrapped text needs refresh
			refresh = TRUE;
		}

		// centered images or text need refresh
		if (style & SS_CENTERIMAGE)
			refresh = TRUE;

		// simple text never needs refresh
		if ((style & SS_TYPEMASK) == SS_SIMPLE)
			refresh = FALSE;
	}

	// get dialog's and control's rect
	CRect wndrc, objrc;

	GetClientRect(&wndrc);
	::GetWindowRect(wnd, &objrc);
	ScreenToClient(&objrc);
	
	CSize tl_margin, br_margin;

	if (br_type == NOANCHOR)
		br_type = tl_type;
	
	// calculate margin for the top-left corner

	tl_margin.cx = objrc.left - wndrc.Width() * tl_type.cx / 100;
	tl_margin.cy = objrc.top - wndrc.Height() * tl_type.cy / 100;
	
	// calculate margin for the bottom-right corner

	br_margin.cx = objrc.right - wndrc.Width() * br_type.cx / 100;
	br_margin.cy = objrc.bottom - wndrc.Height() * br_type.cy / 100;

	// add to the list
	m_plLayoutList.AddTail(new Layout(wnd, tl_type, tl_margin,
		br_type, br_margin, hscroll, refresh));
}

void CResizablePage::ArrangeLayout()
{
	// init some vars
	CRect wndrc;
	GetClientRect(&wndrc);

	Layout *pl;
	POSITION pos = m_plLayoutList.GetHeadPosition();

	HDWP hdwp = BeginDeferWindowPos((int)m_plLayoutList.GetCount());

	while (pos != NULL)
	{
		pl = (Layout*)m_plLayoutList.GetNext(pos);
	
		CRect objrc, newrc;
		CWnd* wnd = CWnd::FromHandle(pl->hwnd); // temporary solution

		wnd->GetWindowRect(&objrc);
		ScreenToClient(&objrc);
		
		// calculate new top-left corner

		newrc.left = pl->tl_margin.cx + wndrc.Width() * pl->tl_type.cx / 100;
		newrc.top = pl->tl_margin.cy + wndrc.Height() * pl->tl_type.cy / 100;
		
		// calculate new bottom-right corner

		newrc.right = pl->br_margin.cx + wndrc.Width() * pl->br_type.cx / 100;
		newrc.bottom = pl->br_margin.cy + wndrc.Height() * pl->br_type.cy / 100;

		if (!newrc.EqualRect(&objrc))
		{
			BOOL add = TRUE;

			if (pl->adj_hscroll)
			{
				// needs repainting, due to horiz scrolling
				int diff = newrc.Width() - objrc.Width();
				int max = wnd->GetScrollLimit(SB_HORZ);
			
				if (max > 0 && wnd->GetScrollPos(SB_HORZ) > max - diff)
				{
					wnd->MoveWindow(&newrc);
					wnd->Invalidate();
					wnd->UpdateWindow();
					
					add = FALSE;
				}
			}

			if (pl->need_refresh)
			{
				wnd->MoveWindow(&newrc);
				wnd->Invalidate();
				wnd->UpdateWindow();
				
				add = FALSE;
			}

			if (add)
				DeferWindowPos(hdwp, pl->hwnd, NULL, newrc.left, newrc.top,
					newrc.Width(), newrc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	// go re-arrange child windows
	EndDeferWindowPos(hdwp);
}

void CResizablePage::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (m_bInitDone)
		ArrangeLayout();
}
