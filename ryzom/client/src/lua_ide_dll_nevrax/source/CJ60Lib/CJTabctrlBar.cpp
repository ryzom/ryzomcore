// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// CJTabCtrlBar.cpp : implementation file
//
// DevStudio Style Resizable Docking Tab Control Bar.
//
// The code contained in this file is based on the original
// CSizingTabCtrlBar class written by Dirk Clemens,
//		mailto:dirk_clemens@hotmail.com
//		http://www.codeguru.com/docking/sizing_tabctrl_bar.shtml
//
// Copyright (c) 1998-99 Kirk Stowell   
//		mailto:kstowell@codejockeys.com
//		http://www.codejockeys.com/kstowell/
//
// This source code may be used in compiled form in any way you desire. 
// Source file(s) may be redistributed unmodified by any means PROVIDING
// they are not sold for profit without the authors expressed written consent,
// and providing that this notice and the authors name and all copyright
// notices remain intact. If the source code is used in any commercial
// applications then a statement along the lines of:
//
// "Portions Copyright (c) 1998-99 Kirk Stowell" must be included in the
// startup banner, "About" box or printed documentation. An email letting
// me know that you are using it would be nice as well. That's not much to ask
// considering the amount of work that went into this.
//
// This software is provided "as is" without express or implied warranty. Use
// it at your own risk! The author accepts no liability for any damage/loss of
// business that this product may cause.
//
// ==========================================================================  
//
// Acknowledgements:
//	<>  To Dirk Clemens (dirk_clemens@hotmail.com) for his CSizingTabCtrlBar
//		class, which is where the idea for this came from.
//	<>  Many thanks to all of you, who have encouraged me to update my articles
//		and code, and who sent in bug reports and fixes.
//  <>  Many thanks Zafir Anjum (zafir@codeguru.com) for the tremendous job that
//      he has done with codeguru, enough can not be said!
//	<>  Many thanks to Microsoft for making the source code availiable for MFC. 
//		Since most of this work is a modification from existing classes and 
//		methods, this library would not have been possible.
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	17 Oct 1998	- Initial re-write and release.
//          1.01    03 Jan 1999 - Application freezing bug fixed
//                                by LiangYiBin.Donald(mailto:lybd@yahoo.com)
//			1.02	17 Jan 1999 - Added helper class CCJTabCtrl to eliminate
//								  re-painting problems such as when the app
//								  is minimized then restored.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJTabCtrlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCJTabCtrl - helper class fixes problems with repainting.

class CCJTabCtrl : public CTabCtrl
{
protected:
	//{{AFX_MSG(CCJTabCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CCJTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CCJTabCtrl)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCJTabCtrl::OnEraseBkgnd(CDC* pDC) 
{
	CRect rcChild;
	GetClientRect(&rcChild);
	rcChild.DeflateRect(3,3);
	rcChild.bottom -= 21;
	pDC->ExcludeClipRect(rcChild);
	return CTabCtrl::OnEraseBkgnd(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CCJTabCtrlBar

CCJTabCtrlBar::CCJTabCtrlBar()
{
	m_nActiveTab = 0;
	m_pTabCtrl = NULL;
}

CCJTabCtrlBar::~CCJTabCtrlBar()
{
	safe_delete(m_pTabCtrl);
	while(!m_views.IsEmpty()) {
		TCB_ITEM *pMember=m_views.RemoveHead();
		safe_delete(pMember);
	}
}

#define IDC_TABCTRLBAR 1000

BEGIN_MESSAGE_MAP(CCJTabCtrlBar, CCJControlBar)
	//{{AFX_MSG_MAP(CCJTabCtrlBar)
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCTRLBAR, OnTabSelChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJTabCtrlBar message handlers

void CCJTabCtrlBar::GetChildRect(CRect &rect) 
{
	CCJControlBar::GetChildRect(rect);

	if (IsFloating()) {
		rect.DeflateRect(2,2);
	}
}

void CCJTabCtrlBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CRect rcChild;
	GetChildRect(rcChild);
	rcChild.DeflateRect(3,3);
	rcChild.bottom -= 21;

	CWnd *pWnd;

	for (POSITION pos=m_views.GetHeadPosition(); pos; m_views.GetNext(pos))
	{
		pWnd = m_views.GetAt(pos)->pWnd;
		pWnd->MoveWindow(rcChild);
	}
	
	// *** LiangYiBin.Donald
	if( lpwndpos->flags & SWP_HIDEWINDOW )
	{
		CFrameWnd* pFrame = GetParentFrame();
		POSITION pos = m_views.GetHeadPosition();
		while ( pos != NULL )
		{
			// check whether the views in the controlbar are focused
			TCB_ITEM *pItem = (TCB_ITEM *) m_views.GetNext(pos);
			if ( pFrame != NULL && pFrame->GetActiveView() == pItem->pWnd )
			{
				// To avoid mainwindow freezing, we must deativate the view,
				// because it's not visible now.
				pFrame->SetActiveView(NULL);
			}
		}
	}
	
	CCJControlBar::OnWindowPosChanged(lpwndpos);
}

CImageList* CCJTabCtrlBar::SetTabImageList(CImageList *pImageList)
{
	return m_pTabCtrl->SetImageList (pImageList);
}

BOOL CCJTabCtrlBar::ModifyTabStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return m_pTabCtrl->ModifyStyle(dwRemove, dwAdd);
}

int CCJTabCtrlBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCJControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pTabCtrl = new CCJTabCtrl;
	ASSERT(m_pTabCtrl);

	//Create the Tab Control
	if (!m_pTabCtrl->Create(WS_VISIBLE|WS_CHILD|TCS_BOTTOM|TCS_TOOLTIPS, 
		CRect(0,0,0,0), this, IDC_TABCTRLBAR))
	{
		TRACE0("Unable to create tab control bar\n");
		return -1;
	}
	
	// Get the log font.
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
		sizeof(NONCLIENTMETRICS), &ncm, 0));
	m_TabFont.CreateFontIndirect(&ncm.lfMessageFont);
	m_pTabCtrl->SetFont(&m_TabFont);
	
	// VC5 Support.
#if _MSC_VER >= 1200
	m_pToolTip = m_pTabCtrl->GetToolTips();
	m_pTabCtrl->SetToolTips(m_pToolTip);
#else
	m_pToolTip = m_pTabCtrl->GetTooltips();
	m_pTabCtrl->SetTooltips(m_pToolTip);
#endif

	SetChild(m_pTabCtrl);
	return 0;
}

void CCJTabCtrlBar::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetActiveView(m_pTabCtrl->GetCurSel());
}

//////////////////////////////////////////////////
// The remainder of this class was written by Dirk Clemens...

BOOL CCJTabCtrlBar::AddView(LPCTSTR lpszLabel, CRuntimeClass *pViewClass, CCreateContext *pContext)
{	
	
#ifdef _DEBUG
	ASSERT_VALID(this);
	ASSERT(pViewClass != NULL);
	ASSERT(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CWnd)));
	ASSERT(AfxIsValidAddress(pViewClass, sizeof(CRuntimeClass), FALSE));
#endif
	
	CCreateContext context;
	if (pContext == NULL)
	{
		// *** LiangYiBin.Donald
		context.m_pCurrentDoc = NULL;
		context.m_pCurrentFrame = GetParentFrame();
		context.m_pLastView = NULL;
		context.m_pNewDocTemplate = NULL;
		context.m_pNewViewClass = pViewClass;
		pContext = &context;
	}
	
	CWnd* pWnd;
	TRY
	{
		pWnd = (CWnd*)pViewClass->CreateObject();
		if (pWnd == NULL)
			AfxThrowMemoryException();
	}
	CATCH_ALL(e)
	{
		TRACE0("Out of memory creating a view.\n");
		// Note: DELETE_EXCEPTION(e) not required
		return FALSE;
	}
	END_CATCH_ALL
		
		ASSERT_KINDOF(CWnd, pWnd);
	ASSERT(pWnd->m_hWnd == NULL);       // not yet created
	
	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
	CRect rect;

	// Create with the right size and position
	if (!pWnd->Create(NULL, NULL, dwStyle, rect, this, 0, pContext))
	{
		TRACE0("Warning: couldn't create client pane for view.\n");
		// pWnd will be cleaned up by PostNcDestroy
		return FALSE;
	}
	m_pActiveView = (CView*) pWnd;
	
	TCB_ITEM *pMember=new TCB_ITEM;
	pMember->pWnd=pWnd;
	strcpy(pMember->szLabel, lpszLabel);
	m_views.AddTail(pMember);
	
	// ToolTip support for tabs.
	if((m_views.GetCount()-1)==0) {
		m_pToolTip->AddTool( m_pTabCtrl, lpszLabel,
			NULL, m_views.GetCount()-1 );
	}
	else {
		m_pToolTip->AddTool( m_pTabCtrl, lpszLabel,
			CRect(0,0,0,0), m_views.GetCount()-1 );
	}
	
	int nViews = m_views.GetCount();
	if (nViews!=1)
	{
		pWnd->EnableWindow(FALSE);
		pWnd->ShowWindow(SW_HIDE);
	}
	else
	{
		// *** LiangYiBin.Donald
		//((CFrameWnd *)GetParent())->SetActiveView((CView *)m_pActiveView);
		GetParentFrame()->SetActiveView(m_pActiveView);
	}
	
	TC_ITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE;
	tci.pszText = (LPTSTR)(LPCTSTR)lpszLabel;
	tci.iImage = nViews-1;
	m_pTabCtrl->InsertItem(nViews, &tci);
	
	return TRUE;
}

void CCJTabCtrlBar::RemoveView(int nView)
{
	ASSERT_VALID(this);
	ASSERT(nView >= 0);
	
	// remove the page from internal list
	m_views.RemoveAt(m_views.FindIndex(nView));
}

void CCJTabCtrlBar::SetActiveView(int nNewTab)
{
	ASSERT_VALID(this);
	ASSERT(nNewTab >= 0);
	
	if (nNewTab!=-1 && nNewTab!=m_nActiveTab)
	{
        TCB_ITEM *newMember=m_views.GetAt(m_views.FindIndex(nNewTab));
        TCB_ITEM *oldMember=NULL;
		
        if (m_nActiveTab!=-1)
        {
            oldMember=m_views.GetAt(m_views.FindIndex(m_nActiveTab));
            oldMember->pWnd->EnableWindow(FALSE);
            oldMember->pWnd->ShowWindow(SW_HIDE);
        }
        newMember->pWnd->EnableWindow(TRUE);
        newMember->pWnd->ShowWindow(SW_SHOW);
        newMember->pWnd->SetFocus();
		
        m_pActiveView = (CView *)newMember->pWnd;
		// *** LiangYiBin.Donald
		//((CFrameWnd *)GetParent())->SetActiveView(m_pActiveView);
		GetParentFrame()->SetActiveView(m_pActiveView);
		
        m_nActiveTab = nNewTab;
		// select the tab (if tab programmatically changed)
		m_pTabCtrl->SetCurSel(m_nActiveTab);
    }
}

void CCJTabCtrlBar::SetActiveView(CRuntimeClass *pViewClass)
{
	ASSERT_VALID(this);
	ASSERT(pViewClass != NULL);
	ASSERT(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CWnd)));
	ASSERT(AfxIsValidAddress(pViewClass, sizeof(CRuntimeClass), FALSE));
	
	int nNewTab = 0;
	for (POSITION pos=m_views.GetHeadPosition(); pos; m_views.GetNext(pos))
	{
		TCB_ITEM *pMember=m_views.GetAt(pos);
		if (pMember->pWnd->IsKindOf(pViewClass))
		{
			//first hide old first view
            m_pActiveView->EnableWindow(FALSE);
            m_pActiveView->ShowWindow(SW_HIDE);
			
			// set new active view
			m_pActiveView = (CView*)pMember->pWnd;
			// enable, show, set focus to new view
			m_pActiveView->EnableWindow(TRUE);
			m_pActiveView->ShowWindow(SW_SHOW);
			m_pActiveView->SetFocus();
			
			// *** LiangYiBin.Donald
			//((CFrameWnd *)GetParent())->SetActiveView(m_pActiveView);
			GetParentFrame()->SetActiveView(m_pActiveView);
			
			m_nActiveTab = nNewTab;
			// select the tab
			m_pTabCtrl->SetCurSel(m_nActiveTab);
			
			break;
		}
		nNewTab++;
    }
}

CView* CCJTabCtrlBar::GetActiveView()
{
	return m_pActiveView;
}

CView* CCJTabCtrlBar::GetView(int nView)
{
	ASSERT_VALID(this);
	ASSERT(nView >= 0);
	
	if (nView!=-1)
	{
        TCB_ITEM *pMember=m_views.GetAt(m_views.FindIndex(nView));
		return (CView*)pMember->pWnd;
	}
	else
		return NULL;
}

CView* CCJTabCtrlBar::GetView(CRuntimeClass *pViewClass)
{
	ASSERT_VALID(this);
	ASSERT(pViewClass != NULL);
	ASSERT(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CWnd)));
	ASSERT(AfxIsValidAddress(pViewClass, sizeof(CRuntimeClass), FALSE));
	
	for (POSITION pos=m_views.GetHeadPosition(); pos; m_views.GetNext(pos))
	{
		TCB_ITEM *pMember=m_views.GetAt(pos);
		if (pMember->pWnd->IsKindOf(pViewClass))
		{
			return (CView*)pMember->pWnd;
		}
    }
	return NULL;
}
