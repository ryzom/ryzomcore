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

#include "stdafx.h"
#include "georges_edit.h"

#include "main_frm.h"
#include "georges_edit_doc.h"
#include "georges_edit_view.h"
#include "left_view.h"
#include "action.h"

using namespace std;
using namespace NLGEORGES;
using namespace NLMISC;



// ***************************************************************************
// CLeftView
// ***************************************************************************

IMPLEMENT_DYNCREATE(CLeftView, CView)

// ***************************************************************************

BEGIN_MESSAGE_MAP(CLeftView, CView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_INSERT, OnInsert)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
	ON_COMMAND(ID_EDIT_FETCH1, OnEditFetch1)
	ON_COMMAND(ID_EDIT_FETCH2, OnEditFetch2)
	ON_COMMAND(ID_EDIT_FETCH3, OnEditFetch3)
	ON_COMMAND(ID_EDIT_FETCH4, OnEditFetch4)
	ON_COMMAND(ID_EDIT_HOLD1, OnEditHold1)
	ON_COMMAND(ID_EDIT_HOLD2, OnEditHold2)
	ON_COMMAND(ID_EDIT_HOLD3, OnEditHold3)
	ON_COMMAND(ID_EDIT_HOLD4, OnEditHold4)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_EDIT_COLLAPSEALL, OnEditCollapseall)
	ON_COMMAND(ID_EDIT_EXPANDALL, OnEditExpandall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CLeftView construction/destruction
// ***************************************************************************

CLeftView::CLeftView()
{
}

// ***************************************************************************

CLeftView::~CLeftView()
{
}

// ***************************************************************************

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// ***************************************************************************
// CLeftView drawing
// ***************************************************************************

void CLeftView::OnDraw(CDC* pDC)
{
	CGeorgesEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

// ***************************************************************************

void CLeftView::getSubObject (CGeorgesEditDocSub *subObject, HTREEITEM parent, HTREEITEM item)
{
	// Is this child exist ?
	if (!item)
	{
		int itemImage = subObject->getItemImage (GetDocument());
		item = TreeCtrl.InsertItem (subObject->getName ().c_str(), itemImage, itemImage, parent);
	}

	// Set name
	TreeCtrl.SetItemText (item, subObject->getName ().c_str());

	// Set item data
	TreeCtrl.SetItemData (item, (DWORD)subObject);

	// For each children
	HTREEITEM child = TreeCtrl.GetChildItem (item);
	for (uint i=0; i<subObject->getChildrenCount(); i++)
	{
		getSubObject (subObject->getChild (i), item, child);
		if (child)
		{
			// Next child
			child = TreeCtrl.GetNextSiblingItem (child);
		}
	}

	// Remove old children
	while (child)
	{
		HTREEITEM toDelete = child;
		child = TreeCtrl.GetNextSiblingItem (child);
		nlverify (TreeCtrl.DeleteItem (toDelete));
	}
}

// ***************************************************************************

void CLeftView::expand ()
{
	if (IsWindow (TreeCtrl))
	{
		HTREEITEM item = TreeCtrl.GetRootItem ();
		if (item != NULL)
		{
			item = TreeCtrl.GetChildItem (item);
			if (item != NULL)
			{
				item = TreeCtrl.GetNextSiblingItem (item);
				expand (item);
			}
		}
	}
}

// ***************************************************************************

void CLeftView::expand (HTREEITEM item)
{
	TreeCtrl.Expand (item, TVE_EXPAND);

	HTREEITEM child = TreeCtrl.GetChildItem (item);
	while (child)
	{
		expand (child);
		child = TreeCtrl.GetNextSiblingItem (child);
	}
}

// ***************************************************************************

void CLeftView::collapse (HTREEITEM item)
{
	TreeCtrl.Expand (item, TVE_COLLAPSE);

	HTREEITEM child = TreeCtrl.GetChildItem (item);
	while (child)
	{
		collapse (child);
		child = TreeCtrl.GetNextSiblingItem (child);
	}
}

// ***************************************************************************

void CLeftView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// Create the tree
	RECT rect;
	GetClientRect (&rect);
	TreeCtrl.Create (TVS_EDITLABELS|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|WS_VISIBLE|WS_CHILD, rect, this, 0);
	TreeCtrl.SetImageList( &(theApp.ImageList.ImageList), TVSIL_NORMAL);

	getFromDocument ();
	if (theApp.StartExpanded)
		expand ();

	HTREEITEM child = TreeCtrl.GetChildItem (TreeCtrl.GetRootItem ());
	if (child)
	{
		child = TreeCtrl.GetNextItem (child, TVGN_NEXT);
		if (child)
		{
			if (TreeCtrl.GetSelectedItem ())
				TreeCtrl.SetItemState(TreeCtrl.GetSelectedItem (), 0, TVIS_BOLD);
			TreeCtrl.Select (child, TVGN_CARET);
			TreeCtrl.SetItemState(child, TVIS_BOLD, TVIS_BOLD);
		}
	}
}

// ***************************************************************************
// CLeftView diagnostics
// ***************************************************************************

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CView::AssertValid();
}

// ***************************************************************************

void CLeftView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

// ***************************************************************************

CGeorgesEditDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesEditDocType)) ||
		m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesEditDocDfn)) ||
		m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesEditDocForm)));
	return (CGeorgesEditDoc*)m_pDocument;
}
#endif //_DEBUG

// ***************************************************************************
// CLeftView message handlers
// ***************************************************************************

BOOL CLeftView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// Selection change ?
	NMHDR *pnmh = (LPNMHDR) lParam; 
	switch (pnmh->code)
	{
	case TVN_BEGINLABELEDIT:
		{
			// Get tree selection
			LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO) lParam;
			if (ptvdi->item.hItem)
			{
				// Get the sub object
				CGeorgesEditDocSub *subObject = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (ptvdi->item.hItem);
				if (subObject)
				{
					// Editable ?
					if (subObject->isEditable ())
					{
						// Get some information about the current node
						bool deleteInsert = false;

						// Is a form child ?
						CGeorgesEditDocSub *parent = subObject->getParent ();
						if (parent && parent->getType () == CGeorgesEditDocSub::Form)
						{
							// Does the node in the same form ?
							CGeorgesEditDoc *doc = GetDocument ();
							if (doc)
							{
								// Get the parent node
								const CFormDfn *parentDfn;
								uint indexDfn;
								const CFormDfn *nodeDfn;
								const CType *nodeType;
								CFormElm *parentNode;
								UFormDfn::TEntryType type;
								bool array;
								bool parentVDfnArray;
								CForm *form=doc->getFormPtr ();
								CFormElm *elm = (CFormElm *)doc->getRootNode (subObject->getSlot ());
								nlverify ( elm->getNodeByName (parent->getFormName ().c_str (), &parentDfn, indexDfn, 
									&nodeDfn, &nodeType, &parentNode, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

								// Is a non empty array ?
								if (array && parentNode)
								{
									// Edit the tree
									*pResult = 0;
									return TRUE;
								}
							}
						}
					}
				}
			}
			*pResult = 1;
			return TRUE;
		}
		break;
	case TVN_ENDLABELEDIT:
		{
			// Get tree selection
			LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO) lParam;
			if (ptvdi->item.hItem)
			{
				// Get the sub object
				CGeorgesEditDocSub *subObject = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (ptvdi->item.hItem);
				if (subObject)
				{
					// Editable ?
					if (subObject->isEditable ())
					{
						// Get some information about the current node
						bool deleteInsert = false;

						// Is a form child ?
						CGeorgesEditDocSub *parent = subObject->getParent ();
						if (parent && parent->getType () == CGeorgesEditDocSub::Form)
						{
							// Does the node in the same form ?
							CGeorgesEditDoc *doc = GetDocument ();
							if (doc)
							{
								// Get the parent node
								const CFormDfn *parentDfn;
								uint indexDfn;
								const CFormDfn *nodeDfn;
								const CType *nodeType;
								CFormElm *parentNode;
								UFormDfn::TEntryType type;
								bool array;
								bool parentVDfnArray;
								CForm *form=doc->getFormPtr ();
								CFormElm *elm = (CFormElm *)doc->getRootNode (subObject->getSlot ());
								nlverify ( elm->getNodeByName (parent->getFormName ().c_str (), &parentDfn, indexDfn, 
									&nodeDfn, &nodeType, &parentNode, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

								// Is a non empty array ?
								if (array && parentNode && (ptvdi->item.mask & TVIF_TEXT))
								{
									// Change the node name
									TreeCtrl.SetItemText (ptvdi->item.hItem, ptvdi->item.pszText);
									doc->modify (new CActionString (IAction::FormArrayRename, ptvdi->item.pszText, *doc, 
										subObject->getFormName ().c_str (), toString (subObject->getIdInParent ()).c_str(), 
										doc->getLeftView ()->getCurrentSelectionId (), subObject->getSlot ()));
									return TRUE;
								}
							}
						}
					}
				}
			}
			*pResult = 1;
			return TRUE;
		}
		break;
	case TVN_SELCHANGED:
		{ 
			LPNMTREEVIEW itemData = (LPNMTREEVIEW) lParam;  

			// Unbold old item
			if (itemData->itemOld.hItem)
				TreeCtrl.SetItemState(itemData->itemOld.hItem, 0, TVIS_BOLD);

			CGeorgesEditDoc *doc = GetDocument();
			if (doc)
			{
				CGeorgesEditDocSub *data = (CGeorgesEditDocSub *)TreeCtrl.GetItemData (itemData->itemNew.hItem);
				nlassert (data);
				doc->changeSubSelection (data, this);
				TreeCtrl.SetFocus ();
			}
		}
		break;
	case NM_RCLICK:
		{
			// Get item clicked
			TVHITTESTINFO  tvhti;
			GetCursorPos(&tvhti.pt);
			::ScreenToClient(TreeCtrl, &tvhti.pt);
			tvhti.flags = LVHT_NOWHERE;
			TreeView_HitTest(TreeCtrl, &tvhti);

			// Item clicked ?
			if(TVHT_ONITEM & tvhti.flags)
			{
				// Select the item
				if (TreeCtrl.GetSelectedItem ())
					TreeCtrl.SetItemState(TreeCtrl.GetSelectedItem (), 0, TVIS_BOLD);
				TreeCtrl.SelectItem (tvhti.hItem);
				TreeCtrl.SetItemState(tvhti.hItem, TVIS_BOLD, TVIS_BOLD);

				// Get the sub object
				CGeorgesEditDocSub *subObject = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (tvhti.hItem);
				if (subObject)
				{
					// Editable ?
					if (subObject->isEditable ())
					{
						// Get some information about the current node
						bool deleteInsert = false;

						// Is a form child ?
						CGeorgesEditDocSub *parent = subObject->getParent ();
						if (parent && parent->getType () == CGeorgesEditDocSub::Form)
						{
							// Does the node in the same form ?
							CGeorgesEditDoc *doc = GetDocument ();
							if (doc)
							{
								// Get the node
								const CFormDfn *parentDfn;
								uint indexDfn;
								const CFormDfn *nodeDfn;
								const CType *nodeType;
								CFormElm *node;
								UFormDfn::TEntryType type;
								bool array;
								bool parentVDfnArray;
								CForm *form=doc->getFormPtr ();
								CFormElm *elm = (CFormElm *)doc->getRootNode (subObject->getSlot ());
								nlverify ( elm->getNodeByName (parent->getFormName ().c_str (), &parentDfn, indexDfn, 
									&nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

								// Is the parent array ?
								deleteInsert = array;
							}
						}

						// Show right click menu

						// Create menu
						CMenu mainMenu;
						mainMenu.LoadMenu (IDR_LEFT_VIEW_CONTEXT);
						CMenu *popup = mainMenu.GetSubMenu (0);
					
						// Enable disable items
						popup->EnableMenuItem (4, MF_BYPOSITION|(deleteInsert?MF_ENABLED:MF_GRAYED));
						popup->EnableMenuItem (5, MF_BYPOSITION|(deleteInsert?MF_ENABLED:MF_GRAYED));
						popup->EnableMenuItem (6, MF_BYPOSITION|(deleteInsert?MF_ENABLED:MF_GRAYED));

						::ClientToScreen(TreeCtrl, &tvhti.pt);
						popup->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON,	tvhti.pt.x, tvhti.pt.y, this, NULL);
					}
				}
			}
		}
		break;
	}
	
	return CView::OnNotify(wParam, lParam, pResult);
}

// ***************************************************************************

void CLeftView::getFromDocument () 
{
	CGeorgesEditDoc *doc = GetDocument();
	if (doc && IsWindow (TreeCtrl))
	{
		// Get the tree root 
		//TreeCtrl.Expand (TreeCtrl.GetRootItem (), TVE_EXPAND);
		getSubObject (&doc->RootObject, NULL, TreeCtrl.GetRootItem ());
	}
}

// ***************************************************************************

void CLeftView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (IsWindow (TreeCtrl))
	{
		getFromDocument ();
	}
}

// ***************************************************************************

void CLeftView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (IsWindow (TreeCtrl))
	{
		RECT rect;
		GetClientRect (&rect);
		TreeCtrl.SetWindowPos (NULL, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER|SWP_NOOWNERZORDER);
	}
}

// ***************************************************************************

BOOL CLeftView::PreTranslateMessage(MSG* pMsg) 
{
	if (theApp.m_pMainWnd->PreTranslateMessage(pMsg))
		return TRUE;
	
	// Key ?
	if (pMsg->message == WM_KEYDOWN)
	{
		// Tabulation ?
		if ((int) pMsg->wParam == VK_TAB)
		{
			CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
			if (doc)
			{
				// Right view ?
				CGeorgesEditView* pView = doc->getRightView ();
				if (pView->isFocusable ())
				{
					doc->switchToView (pView);
					
					// Shift ?
					if (GetAsyncKeyState (VK_SHIFT) & (1<<15))
						pView->setFocusLastWidget ();
				}
			}
			return TRUE;
		}
		else if ((int) pMsg->wParam == VK_INSERT)
		{
			OnInsert ();
			return TRUE;
		}
		else if ((int) pMsg->wParam == VK_DELETE)
		{
			OnDelete ();
			return TRUE;
		}
	}

	return CView::PreTranslateMessage(pMsg);
}

// ***************************************************************************

void CLeftView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	TreeCtrl.SetFocus ();
}

// ***************************************************************************

void CLeftView::OnEditCopy() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		// What is selected ?
		HTREEITEM item = TreeCtrl.GetSelectedItem ();
		if (item != NULL)
		{
			// Edit the label ?
			CEdit *edit = TreeCtrl.GetEditControl();
			if (edit)
			{
				edit->SendMessage (WM_COPY);
			}
			else
			{
				// Get the sub data
				CGeorgesEditDocSub *subData = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (item);
				if (subData)
				{
					// Get thte node type
					CGeorgesEditDocSub::TSub subType = subData->getType ();

					// Good type for copy ?
					if (subType == CGeorgesEditDocSub::Form)
					{
						theApp.SerialIntoMemStream (subData->getFormName ().c_str(), doc, subData->getSlot (), true);
					}
				}
			}
		}
	}
}

// ***************************************************************************

void CLeftView::OnEditPaste() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		// What is selected ?
		HTREEITEM item = TreeCtrl.GetSelectedItem ();
		if (item != NULL)
		{
			// Edit the label ?
			CEdit *edit = TreeCtrl.GetEditControl();
			if (edit)
			{
				edit->SendMessage (WM_PASTE);
			}
			else
			{
				// Get the sub data
				CGeorgesEditDocSub *subData = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (item);
				if (subData)
				{
					// Get thte node type
					CGeorgesEditDocSub::TSub subType = subData->getType ();

					// Good type for copy ?
					if (subType == CGeorgesEditDocSub::Form)
					{
						// Document is modified by this view
						if (theApp.FillMemStreamWithClipboard (subData->getFormName ().c_str(), doc, subData->getSlot ()))
						{
							doc->modify (new CActionBuffer (IAction::FormPaste, theApp.MemStream.buffer (), theApp.MemStream.length(), 
								*doc, subData->getFormName ().c_str(), "", doc->getLeftView ()->getCurrentSelectionId (), subData->getSlot ()));
						}
					}
				}
			}
		}
	}
}

// ***************************************************************************

CGeorgesEditDocSub *CLeftView::getSelectedObject ()
{
	HTREEITEM item = TreeCtrl.GetSelectedItem ();
	if (item != NULL)
	{
		return (CGeorgesEditDocSub *)TreeCtrl.GetItemData (item);
	}
	return NULL;
}

// ***************************************************************************

bool CLeftView::changeSubSelection (CGeorgesEditDocSub *sub, HTREEITEM item)
{
	// First time ?
	if (item == NULL)
		item = TreeCtrl.GetRootItem ();

	// Good one ?
	if ( (CGeorgesEditDocSub *)TreeCtrl.GetItemData (item) == sub )
	{
		// Select it.
		if (TreeCtrl.GetSelectedItem ())
			TreeCtrl.SetItemState(TreeCtrl.GetSelectedItem (), 0, TVIS_BOLD);
		nlverify (TreeCtrl.SelectItem (item));
		TreeCtrl.SetItemState(item, TVIS_BOLD, TVIS_BOLD);
		return true;
	}

	// Scan ohters
	HTREEITEM child = TreeCtrl.GetChildItem (item);
	while (child)
	{
		// Good one ?
		if (changeSubSelection (sub, child))
			return true;

		// Get next item
		child = TreeCtrl.GetNextSiblingItem (child);
	}

	// Selection not found
	return false;
}

// ***************************************************************************

bool CLeftView::changeSubSelection (uint &sub, HTREEITEM item)
{
	// First time ?
	if (item == NULL)
		item = TreeCtrl.GetRootItem ();

	// Good one ?
	if ( sub == 0 )
	{
		// Select it.
		if (TreeCtrl.GetSelectedItem ())
			TreeCtrl.SetItemState(TreeCtrl.GetSelectedItem (), 0, TVIS_BOLD);
		nlverify (TreeCtrl.SelectItem (item));
		TreeCtrl.SetItemState(item, TVIS_BOLD, TVIS_BOLD);
		return true;
	}

	// Scan ohters
	HTREEITEM child = TreeCtrl.GetChildItem (item);
	while (child)
	{
		// Good one ?
		if (changeSubSelection (--sub, child))
			return true;

		// Get next item
		child = TreeCtrl.GetNextSiblingItem (child);
	}

	// Selection not found
	return false;
}

// ***************************************************************************

uint CLeftView::getCurrentSelectionId (uint *count, HTREEITEM item)
{
	if (IsWindow (TreeCtrl))
	{
		uint id = 0;

		// Root item
		if (item == NULL)
			item = TreeCtrl.GetRootItem ();

		// First id
		if (count == NULL)
		{
			count = &id;
			*count = 0;
		}

		// Good node ?
		if (TreeCtrl.GetSelectedItem () == item)
		{
			return *count;
		}

		// Look in children
		item = TreeCtrl.GetChildItem (item);
		while (item)
		{
			// Increment id
			(*count) ++;

			// Found ?
			if (getCurrentSelectionId (count, item) != 0xffffffff)
				return *count;

			// Next child
			item = TreeCtrl.GetNextSiblingItem (item);
		}
	}

	// Not found
	return 0xffffffff;
}

// ***************************************************************************

bool CLeftView::setCurrentSelectionId (uint	id, HTREEITEM item)
{
	if (IsWindow (TreeCtrl))
	{
		// Root item
		if (item == NULL)
		{
			item = TreeCtrl.GetRootItem ();
		}

		// Good node ?
		if (id == 0)
		{
			// Select the node
			if (TreeCtrl.GetSelectedItem ())
				TreeCtrl.SetItemState(TreeCtrl.GetSelectedItem (), 0, TVIS_BOLD);
			TreeCtrl.Select (item, TVGN_CARET);
			TreeCtrl.SetItemState(item, TVIS_BOLD, TVIS_BOLD);
			return true;
		}

		// Look in children
		item = TreeCtrl.GetChildItem (item);
		while (item)
		{
			// Increment id
			id++;

			// Found ?
			if (setCurrentSelectionId (id, item))
				return true;

			// Next child
			item = TreeCtrl.GetNextSiblingItem (item);
		}
	}

	// Not found
	return false;
}

// ***************************************************************************

void CLeftView::OnEditCut() 
{
	// Copy
	OnEditCopy ();

	// Delete
	OnDelete ();
}

// ***************************************************************************

void CLeftView::OnInsert() 
{
	// Get tree selection
	HTREEITEM item = TreeCtrl.GetSelectedItem ();
	if (item)
	{
		// Get the sub object
		CGeorgesEditDocSub *subObject = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (item);
		if (subObject)
		{
			// Editable ?
			if (subObject->isEditable ())
			{
				// Get some information about the current node
				bool deleteInsert = false;

				// Is a form child ?
				CGeorgesEditDocSub *parent = subObject->getParent ();
				if (parent && parent->getType () == CGeorgesEditDocSub::Form)
				{
					// Does the node in the same form ?
					CGeorgesEditDoc *doc = GetDocument ();
					if (doc)
					{
						// Get the parent node
						const CFormDfn *parentDfn;
						uint indexDfn;
						const CFormDfn *nodeDfn;
						const CType *nodeType;
						CFormElm *parentNode;
						UFormDfn::TEntryType type;
						bool array;
						bool parentVDfnArray;
						CForm *form=doc->getFormPtr ();
						CFormElm *elm = (CFormElm *)doc->getRootNode (subObject->getSlot ());
						nlverify ( elm->getNodeByName (parent->getFormName ().c_str (), &parentDfn, indexDfn, 
							&nodeDfn, &nodeType, &parentNode, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

						// Is a non empty array ?
						if (array && parentNode)
						{
							// Document modified
							doc->modify (new CActionString (IAction::FormArrayInsert, toString (subObject->getIdInParent ()).c_str (), 
								*doc, subObject->getFormName ().c_str (), "", doc->getLeftView ()->getCurrentSelectionId (), subObject->getSlot ()));
						}
					}
				}
			}
		}
	}
}

// ***************************************************************************

void CLeftView::OnDelete() 
{
	// Get tree selection
	HTREEITEM item = TreeCtrl.GetSelectedItem ();
	if (item)
	{
		// Edit the label ?
		CEdit *edit = TreeCtrl.GetEditControl();
		if (edit)
		{
			edit->SetWindowText ("");
		}
		else
		{
			// Get the sub object
			CGeorgesEditDocSub *subObject = (CGeorgesEditDocSub*)TreeCtrl.GetItemData (item);
			if (subObject)
			{
				// Editable ?
				if (subObject->isEditable ())
				{
					// Get some information about the current node
					bool deleteInsert = false;

					// Is a form child ?
					CGeorgesEditDocSub *parent = subObject->getParent ();
					if (parent && parent->getType () == CGeorgesEditDocSub::Form)
					{
						// Does the node in the same form ?
						CGeorgesEditDoc *doc = GetDocument ();
						if (doc)
						{
							// Get the parent node
							const CFormDfn *parentDfn;
							uint indexDfn;
							const CFormDfn *nodeDfn;
							const CType *nodeType;
							CFormElm *parentNode;
							UFormDfn::TEntryType type;
							bool array;
							bool parentVDfnArray;
							CForm *form=doc->getFormPtr ();
							CFormElm *elm = (CFormElm *)doc->getRootNode (subObject->getSlot ());
							nlverify ( elm->getNodeByName (parent->getFormName ().c_str (), &parentDfn, indexDfn, 
								&nodeDfn, &nodeType, &parentNode, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

							// Is a non empty array ?
							if (array && parentNode)
							{
								// Document modified
								doc->modify (new CActionBuffer (IAction::FormArrayDelete, NULL, 0, *doc, subObject->getFormName ().c_str (), 
									toString (subObject->getIdInParent ()).c_str (), doc->getLeftView ()->getCurrentSelectionId (), subObject->getSlot ()));
							}
						}
					}
				}
			}
		}
	}
}

// ***************************************************************************


void CLeftView::OnEditRedo() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		doc->OnEditRedo();
	}
}

void CLeftView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		doc->OnUpdateEditRedo(pCmdUI) ;
	}
}

void CLeftView::OnEditUndo() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		doc->OnEditUndo();
	}
}

void CLeftView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		doc->OnUpdateEditUndo(pCmdUI) ;
	}
}

void CLeftView::OnEditRename() 
{
	// Edit the tree
	HTREEITEM item = TreeCtrl.GetSelectedItem ();
	if (item)
		TreeCtrl.EditLabel ( item );
}

void CLeftView::OnEditFetch1() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch1();
	}
}

void CLeftView::OnEditFetch2() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch2();
	}
}

void CLeftView::OnEditFetch3() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch3();
	}
}

void CLeftView::OnEditFetch4() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditFetch4();
	}
}

void CLeftView::OnEditHold1() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold1();
	}
}

void CLeftView::OnEditHold2() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold2();
	}
}

void CLeftView::OnEditHold3() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold3();
	}
}

void CLeftView::OnEditHold4() 
{
	CGeorgesEditDoc *doc = (CGeorgesEditDoc*)GetDocument( );
	if (doc)
	{
		if (doc->isForm ())
			((CGeorgesEditDocForm*)doc)->OnEditHold4();
	}
}

void CLeftView::OnEditCollapseall() 
{
	// Edit the tree
	HTREEITEM item = TreeCtrl.GetSelectedItem ();
	if (item)
		collapse (item);
}

void CLeftView::OnEditExpandall() 
{
	// Edit the tree
	HTREEITEM item = TreeCtrl.GetSelectedItem ();
	if (item)
		expand (item);
}
