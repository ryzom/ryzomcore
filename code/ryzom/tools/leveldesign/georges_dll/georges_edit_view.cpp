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

//

#include "stdafx.h"
#include "georges_edit.h"

#include "georges_edit_doc.h"
#include "georges_edit_view.h"
#include "left_view.h"

using namespace NLGEORGES;


/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditView

IMPLEMENT_DYNCREATE(CGeorgesEditView, CScrollView)

BEGIN_MESSAGE_MAP(CGeorgesEditView, CScrollView)
	//{{AFX_MSG_MAP(CGeorgesEditView)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_OPEN_SELECTED, OnOpenSelected)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_FETCH1, OnEditFetch1)
	ON_COMMAND(ID_EDIT_FETCH2, OnEditFetch2)
	ON_COMMAND(ID_EDIT_FETCH3, OnEditFetch3)
	ON_COMMAND(ID_EDIT_FETCH4, OnEditFetch4)
	ON_COMMAND(ID_EDIT_HOLD1, OnEditHold1)
	ON_COMMAND(ID_EDIT_HOLD2, OnEditHold2)
	ON_COMMAND(ID_EDIT_HOLD3, OnEditHold3)
	ON_COMMAND(ID_EDIT_HOLD4, OnEditHold4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditView construction/destruction

CGeorgesEditView::CGeorgesEditView()
{
	VirtualWidth = 100;
	VirtualHeight = 100;
}

CGeorgesEditView::~CGeorgesEditView()
{
}

BOOL CGeorgesEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditView drawing

void CGeorgesEditView::OnDraw(CDC* pDC)
{
	CGeorgesEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

void CGeorgesEditView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	//  its list control through a call to GetListCtrl().
	CGeorgesEditDoc* pDoc = GetDocument();

	// Save modified state
	pDoc->NoModification = true;

	SIZE size;
	size.cx = VirtualWidth;
	size.cy = VirtualHeight;
	SetScrollSizes (MM_TEXT, size);

	RECT rect;
	GetClientRect (&rect);

	// Create the tab
	TabCtrl.Create (WS_VISIBLE, rect, this, 0);
	TabCtrl.SetImageList (&(theApp.ImageList.ImageList));
	TabCtrl.SetWindowPos (NULL, 0, 0, std::max ((int)VirtualWidth, (int)(rect.right-rect.left)), std::max ((int)VirtualHeight, (int)(rect.bottom-rect.top)), SWP_NOZORDER|SWP_NOOWNERZORDER);

	// Update the tab
	updateTab ();

	// Create the type dialog
	RECT tabClient;
	TabCtrl.GetClientRect (&tabClient);
	TypeDialog.View = this;
	TypeDialog.Create (CBaseDialog::IDD, &TabCtrl);
	TypeDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);

	// Create the header dialog
	HeaderDialog.View = this;
	HeaderDialog.Create (CBaseDialog::IDD, &TabCtrl);
	HeaderDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);

	// Create the dfn dialog
	DfnDialog.View = this;
	DfnDialog.Create (CBaseDialog::IDD, &TabCtrl);
	DfnDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);

	// Create the dfn dialog
	FormDialog.View = this;
	FormDialog.Create (CBaseDialog::IDD, &TabCtrl);
	FormDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);

	// Set tab font
	CFont* font = TypeDialog.GetFont ();
	TabCtrl.SetFont (font);
	TabCtrl.ShowWindow (SW_SHOW);

	// Update document
	updateDocument ();

	// Save modified state
	pDoc->NoModification = false;

	// Is it a form ?
	theApp.onNewDocView (pDoc);

	// Notify the doc plugin
	uint i;
	CGeorgesEditDocSub *selection = pDoc->getSelectedObject ();
	for (i=0; i<pDoc->PluginArray.size (); i++)
	{
		// Activate / desactivate plugin
		pDoc->PluginArray[i].PluginInterface->onNodeChanged ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditView diagnostics

#ifdef _DEBUG
void CGeorgesEditView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CGeorgesEditView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CGeorgesEditDoc* CGeorgesEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesEditDocType)) ||
		m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesEditDocDfn)) ||
		m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesEditDocForm)));
	return (CGeorgesEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditView message handlers
void CGeorgesEditView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window
}

void CGeorgesEditView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

	UpdateData ();

	if (IsWindow (TabCtrl))
	{
		CPoint currentPos = GetDeviceScrollPosition ();

		RECT rect;
		GetClientRect (&rect);

		if (IsWindow (TypeDialog) && TypeDialog.IsWindowVisible ())
		{
			TypeDialog.resizeWidgets ();
		}

		// Resize header dialog
		if (IsWindow (HeaderDialog) && HeaderDialog.IsWindowVisible ())
		{
			HeaderDialog.resizeWidgets ();
		}

		// Resize dfn dialog
		if (IsWindow (DfnDialog) && DfnDialog.IsWindowVisible ())
		{
			DfnDialog.resizeWidgets ();
		}

		// Resize dfn dialog
		if (IsWindow (FormDialog) && FormDialog.IsWindowVisible ())
		{
			FormDialog.resizeWidgets ();
		}

		// Create the tab
		TabCtrl.SetWindowPos (NULL, -currentPos.x, -currentPos.y, std::max ((int)VirtualWidth, (int)(rect.right-rect.left)), std::max ((int)VirtualHeight, (int)(rect.bottom-rect.top)), SWP_NOZORDER|SWP_NOOWNERZORDER);

		// Resize type dialog
		RECT tabClient;
		TabCtrl.GetClientRect (&tabClient);

		// Resize type dialog
		if (IsWindow (TypeDialog))
		{
			TypeDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, 
				tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}

		// Resize header dialog
		if (IsWindow (HeaderDialog))
		{
			HeaderDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, 
				tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);
		}

		// Resize dfn dialog
		if (IsWindow (DfnDialog))
			DfnDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, 
				tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);

		// Resize dfn dialog
		if (IsWindow (FormDialog))
			FormDialog.SetWindowPos (NULL, WidgetLeftMargin, WidgetTopMargin, tabClient.right - tabClient.left - WidgetLeftMargin - WidgetRightMargin, 
				tabClient.bottom - tabClient.top - WidgetTopMargin - WidgetBottomMargin, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}

	UpdateData (FALSE);
}

void CGeorgesEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// Not me ?
	if (pSender != this)
	{
		// Update
		updateTab ();
		updateDocument ();
	}	
}

void CGeorgesEditView::updateTab ()
{
	if (IsWindow (TabCtrl))
	{
		// Clear the tab
		TabCtrl.DeleteAllItems ();

		CGeorgesEditDoc *doc = GetDocument();
		if (doc)
		{
			// Current selection
			CGeorgesEditDocSub *subObject = doc->getSelectedObject ();
			if (subObject)
			{
				// Get parent
				CGeorgesEditDocSub *parent = subObject->getParent ();
				if (parent)
				{
					// For each subobject
					int tabSelected = -1;
					for (uint i=0; i<parent->getChildrenCount(); i++)
					{
						// Init the tab
						CGeorgesEditDocSub *child = parent->getChild (i);
						int image = child->getItemImage (doc);
						TabCtrl.InsertItem (i, child->getName ().c_str(), image);

						// This is the selection ?
						if (subObject == child)
							tabSelected = i;
					}

					// Should be found
					nlassert (tabSelected!=-1);
					TabCtrl.SetCurSel (tabSelected);

					UpdateData (FALSE);
				}
			}
		}
	}
}

void CGeorgesEditView::updateDocument ()
{
	if (IsWindow (TabCtrl) && IsWindow (DfnDialog) && IsWindow (TypeDialog) && IsWindow (FormDialog) && IsWindow (HeaderDialog) )
	{
		CGeorgesEditDoc *doc = GetDocument();
		if (doc)
		{
			// Hide all the window
			TypeDialog.ShowWindow (SW_HIDE);
			HeaderDialog.ShowWindow (SW_HIDE);
			DfnDialog.ShowWindow (SW_HIDE);
			FormDialog.ShowWindow (SW_HIDE);

			// Current selection
			CGeorgesEditDocSub *subObject = doc->getSelectedObject ();
			if (subObject)
			{
				switch (subObject->getType ())
				{
				case CGeorgesEditDocSub::Type:
					{
						// Get the document
						TypeDialog.getFromDocument (*doc->getTypePtr ());

						// Show the window
						TypeDialog.resizeWidgets ();
						TypeDialog.ShowWindow (SW_SHOW);
					}
					break;
				case CGeorgesEditDocSub::Header:
					{
						// Get the document
						HeaderDialog.getFromDocument (*doc->getHeaderPtr ());

						// Show the window
						HeaderDialog.resizeWidgets ();
						HeaderDialog.ShowWindow (SW_SHOW);
					}
					break;
				case CGeorgesEditDocSub::Dfn:
					{
						// Get the document
						DfnDialog.getFromDocument (*doc->getDfnPtr ());

						// Show the window
						DfnDialog.resizeWidgets ();
						DfnDialog.ShowWindow (SW_SHOW);
					}
					break;
				case CGeorgesEditDocSub::Form:
					{
						// Get the document
						FormDialog.getFromDocument ();

						// Show the window
						FormDialog.resizeWidgets ();
						FormDialog.ShowWindow (SW_SHOW);
					}
					break;
				}
			}
		}
	}
}

BOOL CGeorgesEditView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	LPNMHDR pnmh = (LPNMHDR) lParam;
	switch (pnmh->code)
	{
	case TCN_SELCHANGE:
		{
			UpdateData ();

			// Get the document
			CGeorgesEditDoc *doc = GetDocument();
			if (doc)
			{
				// Current selection
				CGeorgesEditDocSub *subObject = doc->getSelectedObject ();
				if (subObject)
				{
					// Get parent
					CGeorgesEditDocSub *parent = subObject->getParent ();
					if (parent)
					{
						// Get the child
						CGeorgesEditDocSub *child = parent->getChild (TabCtrl.GetCurSel ());

						// Sub selection changed
						CLeftView* pView = doc->getLeftView ();
						doc->changeSubSelection (child, pView);
						TabCtrl.SetFocus ();
					}
				}
			}
		}
		break;
	}

	return CScrollView::OnNotify(wParam, lParam, pResult);
}

void CGeorgesEditView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);

	TabCtrl.UpdateData ();
	if ( (TabCtrl.GetItemCount ()) && (pOldWnd != &TabCtrl) )
	{
		TabCtrl.SetFocus ();
	}
	else
	{
		if (TypeDialog.IsWindowVisible ())
			TypeDialog.SetFocus ();		
		if (HeaderDialog.IsWindowVisible ())
			HeaderDialog.SetFocus ();		
		if (DfnDialog.IsWindowVisible ())
			DfnDialog.SetFocus ();
		if (FormDialog.IsWindowVisible ())
			FormDialog.SetFocus ();
	}
}

BOOL CGeorgesEditView::PreTranslateMessage(MSG* pMsg) 
{
	if (theApp.m_pMainWnd->PreTranslateMessage(pMsg))
		return TRUE;

	if ((pMsg->message == WM_KEYDOWN) && ((int) pMsg->wParam == VK_TAB) )
	{
		// Shift ?
		if (GetAsyncKeyState (VK_SHIFT) & (1<<15))
			setFocusLeftView ();
		else
			SetFocus ();
		return TRUE;
	}
	
	return CScrollView::PreTranslateMessage(pMsg);
}

void CGeorgesEditView::OnOpenSelected() 
{
	if (HeaderDialog.IsWindowVisible ())
	{
		HeaderDialog.onOpenSelected ();
	}
	if (TypeDialog.IsWindowVisible ())
	{
		TypeDialog.onOpenSelected ();
	}
	if (DfnDialog.IsWindowVisible ())
	{
		DfnDialog.onOpenSelected ();
	}
	if (FormDialog.IsWindowVisible ())
	{
		FormDialog.onOpenSelected ();
	}
}

void CGeorgesEditView::setFocusLastWidget ()
{
	if (HeaderDialog.IsWindowVisible ())
	{
		HeaderDialog.setFocusLastWidget ();
	}
	if (TypeDialog.IsWindowVisible ())
	{
		TypeDialog.setFocusLastWidget ();
	}
	if (DfnDialog.IsWindowVisible ())
	{
		DfnDialog.setFocusLastWidget ();
	}
	if (FormDialog.IsWindowVisible ())
	{
		FormDialog.setFocusLastWidget ();
	}
}

bool CGeorgesEditView::isFocusable () const
{
	return (HeaderDialog.IsWindowVisible () || TypeDialog.IsWindowVisible () || DfnDialog.IsWindowVisible () || FormDialog.IsWindowVisible ());
}

void CGeorgesEditView::setFocusLeftView ()
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		CLeftView* pView = doc->getLeftView ();
		doc->switchToView (pView);
	}
}

LRESULT CGeorgesEditView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// When this message arrive, update the view
	if (message == WM_UPDATE_ALL_VIEWS)
		GetDocument ()->UpdateAllViews (NULL);
	
	return CScrollView::WindowProc(message, wParam, lParam);
}

void CGeorgesEditView::OnEditCopy() 
{
	CWnd *focus = GetFocus ();
	if (focus)
		focus->SendMessage (WM_COPY);
}

void CGeorgesEditView::OnEditCut() 
{
	CWnd *focus = GetFocus ();
	if (focus)
		focus->SendMessage (WM_CUT);
}

void CGeorgesEditView::OnEditPaste() 
{
	CWnd *focus = GetFocus ();
	if (focus)
		focus->SendMessage (WM_PASTE);
}
/*
void CGeorgesEditView::OnEditUndo() 
{
	CWnd *focus = GetFocus ();
	if (focus)
		focus->SendMessage (WM_UNDO);
}
*/
void CGeorgesEditView::setViewSize (uint width, uint height)
{
	VirtualWidth = width;
	VirtualHeight = height;
	SIZE size;
	size.cx = VirtualWidth;
	size.cy = VirtualHeight;
	SetScrollSizes (MM_TEXT, size);
}

BOOL CGeorgesEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	int nMapMode;
	SIZE sizeTotal;
	SIZE sizePage;
	SIZE sizeLine;
	GetDeviceScrollSizes( nMapMode, sizeTotal, sizePage, sizeLine );

	RECT rect;
	GetClientRect (&rect);
	if (rect.bottom < sizeTotal.cy)
	{
		CPoint currentPos = GetDeviceScrollPosition ();
		currentPos.y -= zDelta;
		ScrollToPosition( currentPos );
	}
	
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}


void CGeorgesEditView::OnEditRedo() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		doc->OnEditRedo();
	}
}

void CGeorgesEditView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnUpdateEditRedo(pCmdUI) ;
	}
}

void CGeorgesEditView::OnEditUndo() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditUndo();
	}
}

void CGeorgesEditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnUpdateEditUndo(pCmdUI) ;
	}
}

void CGeorgesEditView::OnEditFetch1() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch1();
	}
}

void CGeorgesEditView::OnEditFetch2() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch2();
	}
}

void CGeorgesEditView::OnEditFetch3() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch3();
	}
}

void CGeorgesEditView::OnEditFetch4() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch4();
	}
}

void CGeorgesEditView::OnEditHold1() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold1();
	}
}

void CGeorgesEditView::OnEditHold2() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold2();
	}
}

void CGeorgesEditView::OnEditHold3() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold3();
	}
}

void CGeorgesEditView::OnEditHold4() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold4();
	}
}
