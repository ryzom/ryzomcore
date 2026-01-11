// ConditionsView.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "ConditionsView.h"

#include "Logic_editorDoc.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "EditorFormView.h"

#include "TMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




#ifndef ID_MOVEASCHILD
#define ID_MOVEASCHILD 32857
#endif

#ifndef ID_MOVEASSIBLING
#define ID_MOVEASSIBLING 32858
#endif

#ifndef ID_COPYASSIBLING
#define ID_COPYASSIBLING 32859
#endif

#ifndef ID_COPYASCHILD
#define ID_COPYASCHILD 32860
#endif




/////////////////////////////////////////////////////////////////////////////
// CConditionsView

IMPLEMENT_DYNCREATE(CConditionsView, CTreeView)

CConditionsView::CConditionsView()
{
}

CConditionsView::~CConditionsView()
{
}


BEGIN_MESSAGE_MAP(CConditionsView, CTreeView)
	//{{AFX_MSG_MAP(CConditionsView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginrdrag)
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_MOVEASCHILD,		OnMoveAsChild)
	ON_COMMAND(ID_MOVEASSIBLING,	OnMoveAsSibling)
	ON_COMMAND(ID_COPYASCHILD,		OnCopyAsChild)
	ON_COMMAND(ID_COPYASSIBLING,	OnCopyAsSibling)
END_MESSAGE_MAP()



// Creating Drag Image for a CTreeCtrl without images 
// Contribution of Pal K Tonder on www.codeguru.com
// The method CreateDragImage is used during drag'n drop to create a drag image. The problem is that it only works for a CTreeCtrl with images.
// The following method, CreateDragImageEx, checks whether the CTreeCtrl has a valid normal CImageList. If so, it just calls the standard CreateDragImage method and returns. 
// If, on the other hand, no valid CImageList is found, a bitmap is created based on the size of item rect, and the item text is drawn into it. Then a CImageList is
// created and the bitmap is added to it. 
CImageList* CConditionsView::CreateDragImageEx(HTREEITEM hItem)
{
	CTreeCtrl & tree = GetTreeCtrl();

	if(tree.GetImageList(TVSIL_NORMAL) != NULL)
		return tree.CreateDragImage(hItem);

	CRect rect;
	tree.GetItemRect(hItem, rect, TRUE);

	rect.OffsetRect(-rect.left, -rect.top);

	// Create bitmap
	CClientDC	dc (this);
	CDC 		memDC;	

	if(!memDC.CreateCompatibleDC(&dc))
		return NULL;

	CBitmap bitmap;
	if(!bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height()))
		return NULL;

	CBitmap* pOldMemDCBitmap = memDC.SelectObject( &bitmap );
	CFont* pOldFont = memDC.SelectObject(GetFont());

	memDC.FillSolidRect(&rect, RGB(0, 255, 0)); // Here green is used as mask color
	memDC.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	memDC.TextOut(rect.left, rect.top, tree.GetItemText(hItem));

	memDC.SelectObject( pOldFont );
	memDC.SelectObject( pOldMemDCBitmap );

	// Create imagelist
	CImageList* pImageList = new CImageList;
	pImageList->Create(rect.Width(), rect.Height(), 
	 ILC_COLOR | ILC_MASK, 0, 1);
	pImageList->Add(&bitmap, RGB(0, 255, 0)); // Here green is used as mask color

	return pImageList;
}







/////////////////////////////////////////////////////////////////////////////
// CConditionsView drawing

void CConditionsView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CConditionsView diagnostics

#ifdef _DEBUG
void CConditionsView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CConditionsView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG




void CConditionsView::expand(UINT nCode, HTREEITEM hItem) 
{
	HTREEITEM hElement;

	HTreeItemArray pile;
	CTreeCtrl &m_tree = GetTreeCtrl();

	if (hItem == NULL)
		hElement = m_tree.GetRootItem();
	else
	{
		m_tree.Expand(hItem,nCode);
		hElement = m_tree.GetChildItem(hItem);
	}
	
	pile.RemoveAll();

	while ((hElement != NULL) || (pile.GetSize() != 0))
	{
		while ((hElement == NULL) && (pile.GetSize()!=0))
		{
			hElement = pile[pile.GetSize()-1];
			pile.RemoveAt(pile.GetSize()-1);
			hElement = m_tree.GetNextSiblingItem(hElement);
		}
		if (hElement)
		{
			pile.Add(hElement);
			m_tree.Expand(hElement,nCode);
			hElement = m_tree.GetChildItem(hElement);
		}
	}
}


HMENU CConditionsView::GetRDragMenu()
{
	CMenu menu;

	menu.CreatePopupMenu();
	menu.AppendMenu( MF_STRING, ID_MOVEASCHILD, _T("Move As Child") );
	menu.AppendMenu( MF_STRING, ID_MOVEASSIBLING, _T("Move As Sibling") );
	menu.AppendMenu( MF_STRING, ID_COPYASCHILD, _T("Copy As Child") );
	menu.AppendMenu( MF_STRING, ID_COPYASSIBLING, _T("Copy As Sibling") );

	return menu.Detach();
}




/////////////////////////////////////////////////////////////////////////////
// CConditionsView message handlers


void CConditionsView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	m_pSelectedCondition = NULL;
	m_pDragCondition	 = NULL;
	m_pSelectedNode		 = NULL;

	m_pDragImage	 = NULL;
	m_hitemDrag		 = NULL;
	m_hitemDrop		 = NULL;
	m_bRDragging	 = FALSE;
	m_bRDragCanceled = FALSE;
}




void CConditionsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	/** TO DO : REDRAW ONLY IF Some CONDITIONS have changed **/

	SetRedraw(FALSE);

	CTreeView::OnUpdate(pSender,lHint,pHint);
	

	BOOL bSelection = FALSE;

	// get a pointer on associated document
	CLogic_editorDoc *pDoc =  static_cast<CLogic_editorDoc*>(GetDocument());
	ASSERT_VALID(pDoc);

	CTreeCtrl &m_tree = GetTreeCtrl();

	m_tree.DeleteAllItems();
	m_mapItemToNode.RemoveAll();

	TPositionList heap;

	HTREEITEM hItem = m_tree.InsertItem(_T("Conditions"), 0, 0);

	// get the states map
	CCondition *pCondition;
	CString condName;
	POSITION pos = pDoc->m_conditions.GetStartPosition();

	while (pos != NULL)
	{
		pDoc->m_conditions.GetNextAssoc( pos, condName, (void*&)pCondition );
		hItem = m_tree.InsertItem( condName , m_tree.GetRootItem());

		// 
		if ( m_pSelectedCondition == pCondition && 	m_pSelectedNode == NULL)
		{
			m_tree.SelectItem( hItem );
			bSelection = TRUE;
		}

		CConditionNode *pNode, *pNodeTemp;
		HTREEITEM parentHItem = hItem;

		POSITION nodePos = pCondition->m_ctConditionTree.GetHeadPosition();
		while (nodePos != NULL)
		{
			pNode = pCondition->m_ctConditionTree.GetNext( nodePos );

			if (pNode != NULL)
			{
				hItem = m_tree.InsertItem( pNode->getNodeAsString() , parentHItem);
				// memorize the association hItem/pNode in the map
				m_mapItemToNode.SetAt( hItem, pNode);
				if (m_pSelectedNode == pNode)
				{
					m_tree.SelectItem( hItem );
					bSelection = TRUE;
				}

				for (;;)
				{
					// get the deepest first level node in the sub tree
					POSITION subPos = pNode->m_ctSubTree.GetHeadPosition();
					while (subPos != NULL)
					{
						pNodeTemp = pNode->m_ctSubTree.GetNext(subPos);
						
						if (pNodeTemp != NULL)
						{
							pNode = pNodeTemp;

							hItem = m_tree.InsertItem( pNode->getNodeAsString() , hItem);
							m_mapItemToNode.SetAt( hItem, pNode);

							if (m_pSelectedNode == pNode)
							{
								m_tree.SelectItem( hItem );
								bSelection = TRUE;
							}

							// push the position
							heap.AddTail( subPos );

							subPos = pNode->m_ctSubTree.GetHeadPosition();
						}
					}					

					// get sibling if any, if none, get next node
					do
					{
						pNode = pNode->getParentNode();
						hItem = m_tree.GetParentItem( hItem );

						// pop subPos
						if( heap.IsEmpty() )
							break;

						subPos = heap.GetTail();
						heap.RemoveTail();
						
						if (subPos != NULL)
						{
							pNodeTemp = pNode->m_ctSubTree.GetNext( subPos );
							if (pNodeTemp != NULL)
							{
								pNode = pNodeTemp;

								hItem = m_tree.InsertItem( pNode->getNodeAsString() , hItem);
								// memorize the association hItem/pNode in the map
								m_mapItemToNode.SetAt( hItem, pNode);

								if (m_pSelectedNode == pNode)
								{
									m_tree.SelectItem( hItem );
									bSelection = TRUE;
								}

								//push
								heap.AddTail( subPos );

								subPos = pNode->m_ctSubTree.GetHeadPosition();
							}				
							else
								break;
						}

					}
					while (subPos == NULL);
					
					if (pNode == NULL)
						break;
				}
			}
		}
	}

	// expand all tree
	expand(/*TVE_EXPAND ,NULL*/ );

	if (!bSelection)
	{
		m_pSelectedNode		= NULL;
		m_pSelectedCondition = NULL;
	}
	else
	{
		// assure the selected item is visible (if any)
		hItem = m_tree.GetSelectedItem();
		ASSERT (hItem != NULL);
		m_tree.EnsureVisible( hItem );
	}

	SetRedraw(/*TRUE*/);	
}


BOOL CConditionsView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// modify arborescence style
	cs.style |=(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS/*|TVS_EDITLABELS*/);
//	cs.style &= ~TVS_DISABLEDRAGDROP;
	
	return CTreeView::PreCreateWindow(cs);
}





void CConditionsView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	//
	m_pSelectedCondition = NULL;
	m_pSelectedNode		 = NULL;

	// get the selected item and update the good frame (open the condition edition page in the form too)
	CTreeCtrl &treeCtrl = GetTreeCtrl();
		
	HTREEITEM hItem;
	hItem = treeCtrl.GetSelectedItem();

	//
	if (hItem == treeCtrl.GetRootItem())
		return;
	
	// get the node selected (if any)
	if ( ! m_mapItemToNode.Lookup(hItem, m_pSelectedNode) )
		m_pSelectedNode = NULL;
	
	// get the condition (if any)
	while ( treeCtrl.GetParentItem( hItem ) != treeCtrl.GetRootItem() )
		hItem = treeCtrl.GetParentItem( hItem );

	CString condName = treeCtrl.GetItemText( hItem );

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (GetDocument());
	
	//  to update the fields of the condition page in the form view
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	CEditorFormView *pFormView = static_cast<CEditorFormView *> ( pChild->m_wndSplitter.GetPane(0,1) );
	CConditionPage *pCondPage = static_cast<CConditionPage*> ( pFormView->m_pPropertySheet->GetPage(2) );

	if ( pDoc->m_conditions.Lookup( condName, (void*&) m_pSelectedCondition) )
	{
		pFormView->m_pPropertySheet->SetActivePage( pCondPage );

		if ( pCondPage->m_pSelectedConditionNode != m_pSelectedNode || m_pSelectedNode == NULL)
		{
			pCondPage->m_pSelectedCondition		= this->m_pSelectedCondition;
			pCondPage->m_pSelectedConditionNode = this->m_pSelectedNode;

			pCondPage->Update();
		}
	}
	else
	{
		if (pCondPage->m_pSelectedCondition != NULL)
		{
			pCondPage->m_pSelectedConditionNode = this->m_pSelectedNode; // = NULL
			pCondPage->m_pSelectedCondition		= this->m_pSelectedCondition; // = NULL

			pCondPage->Update();
		}
	}

	this->SetFocus();
	
	//
	*pResult = 0;
}

void CConditionsView::OnKillFocus(CWnd* pNewWnd) 
{
//	keep the current selection, so do not call CTreeView::OnKillFocus()
//		CTreeView::OnKillFocus(pNewWnd);
}



void CConditionsView::OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	CTreeCtrl &tree = GetTreeCtrl();

	CPoint		ptAction;

	GetCursorPos(&ptAction);
	ScreenToClient(&ptAction);
	m_bRDragging = TRUE;
	m_bRDragCanceled = FALSE;
	m_hitemDrag = pNMTreeView->itemNew.hItem;
	m_hitemDrop = NULL;
	m_pDragImage = CreateDragImageEx(m_hitemDrag);	// get the image list for dragging
	m_pDragImage->BeginDrag(0, CPoint(10,-15));
	tree.SelectDropTarget(NULL);			// to prevent image corruption.
	m_pDragImage->DragEnter(NULL, ptAction);
	SetCapture();


	// get the drag condition
	HTREEITEM hItem = m_hitemDrag;
	//
	if (hItem == tree.GetRootItem())
		return;
		
	// get the condition (if any)
	while ( tree.GetParentItem( hItem ) != tree.GetRootItem() )
		hItem = tree.GetParentItem( hItem );

	CString condName = tree.GetItemText( hItem );

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (GetDocument());

	pDoc->m_conditions.Lookup( condName, (void*&) m_pDragCondition);

}

void CConditionsView::OnMouseMove(UINT nFlags, CPoint point) 
{
	HTREEITEM			hitem;
	UINT				flags;

	CTreeCtrl &tree = GetTreeCtrl();

	if (m_bRDragging)
	{
		POINT pt = point;
		ClientToScreen( &pt );
		CImageList::DragMove(pt);
		if ((hitem = tree.HitTest(point, &flags)) != NULL)
		{
			CImageList::DragShowNolock(FALSE);
			tree.SelectDropTarget(hitem);
			m_hitemDrop = hitem;
			CImageList::DragShowNolock(TRUE);
		}
	}
	
	CTreeView::OnMouseMove(nFlags, point);
}


void CConditionsView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CTreeCtrl	&tree = GetTreeCtrl();

	if (m_bRDragging)
	{
		m_bRDragging = FALSE;
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		ReleaseCapture();
		delete m_pDragImage;

		if( m_hitemDrag == m_hitemDrop )
			return;
		
		CTMenu menu;
		menu.Attach( GetRDragMenu() );

		menu.AddMenuTitle( tree.GetItemText(m_hitemDrag) );

		// if the target is a condition and the dragged item is a node, do not allow '..as sibling' operations
		CConditionNode *pNode;
		if ( m_mapItemToNode.Lookup(m_hitemDrag, pNode) && ! m_mapItemToNode.Lookup(m_hitemDrop, pNode) )
		{
			menu.EnableMenuItem( ID_MOVEASSIBLING, MF_GRAYED );
			menu.EnableMenuItem( ID_COPYASSIBLING, MF_GRAYED );
		}
		

		if( menu == NULL ) return;

		ClientToScreen( &point );
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			point.x, point.y,
			this); 

		tree.SelectDropTarget(NULL);
	}
	else
		CTreeView::OnRButtonUp(nFlags, point);
}



void CConditionsView::OnMoveAsChild()
{
	CTreeCtrl &tree = GetTreeCtrl();

	// Check that the dragged item is not an ancestor
	HTREEITEM htiParent = m_hitemDrop;
	while( (htiParent = tree.GetParentItem( htiParent )) != NULL )
	{
		if( htiParent == m_hitemDrag )
			return;
	}

	CConditionNode* pTargetNode = NULL;
	CCondition *	pTargetCond = NULL;

	CConditionNode *pDragNode;
	// get the drag object
	if ( m_mapItemToNode.Lookup( m_hitemDrag, pDragNode ) )
	{
		CConditionNode *pParentNode = pDragNode->getParentNode();
		// parent is a node
		if (pParentNode != NULL)
		{
			POSITION pos = pParentNode->m_ctSubTree.Find( pDragNode );
			if (pos != NULL)
			{
				pParentNode->m_ctSubTree.RemoveAt( pos );				
			}
			else
				return;
		}
		// parent is a condition
		else
		{
			POSITION pos = m_pDragCondition->m_ctConditionTree.Find( pDragNode );
			if (pos != NULL)
			{
				m_pDragCondition->m_ctConditionTree.RemoveAt( pos );				
			}
			else
				return;
		}
	}	
	else
		return;

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (GetDocument());

	
	// target is a node
	if ( m_mapItemToNode.Lookup( m_hitemDrop, pTargetNode ) )
	{
		pTargetNode->m_ctSubTree.AddTail( pDragNode );
		pDragNode->m_pParentNode = pTargetNode;
	}
	// target is a condition
	else
	{
		CString name = tree.GetItemText( m_hitemDrop );
		if (pDoc->m_conditions.Lookup( name, (void*&)pTargetCond ) )
		{
			pTargetCond->m_ctConditionTree.AddTail( pDragNode );		
			pDragNode->m_pParentNode = NULL;
		}
		else
		{
			AfxMessageBox(_T("Error : root condition not found"));
			delete pDragNode;
		}
	}

	// for the moment, update all views, but moving branch may be faster
	pDoc->UpdateAllViews(NULL);
/*
	Expand( m_hitemDrop, TVE_EXPAND ) ;

	HTREEITEM htiNew = CopyBranch( m_hitemDrag, m_hitemDrop, TVI_LAST );
	DeleteItem(m_hitemDrag);
	SelectItem( htiNew );
*/
}



void CConditionsView::OnMoveAsSibling()
{
	CTreeCtrl &tree = GetTreeCtrl();
	if (m_hitemDrop == tree.GetRootItem() )
		return;

	m_hitemDrop = tree.GetParentItem( m_hitemDrop);

	OnMoveAsChild();
}



void CConditionsView::OnCopyAsChild()
{
	CTreeCtrl &tree = GetTreeCtrl();
	// Check that the dragged item is not an ancestor
	HTREEITEM htiParent = m_hitemDrop;
	while( (htiParent = tree.GetParentItem( htiParent )) != NULL )
	{
		if( htiParent == m_hitemDrag )
			return;
	}


	CConditionNode* pTargetNode = NULL;
	CCondition *	pTargetCond = NULL;

	CConditionNode *pDragNode = NULL;
	CConditionNode *pNewNode = NULL;

	// get the drag object
	if ( m_mapItemToNode.Lookup( m_hitemDrag, pDragNode ) )
	{
		pNewNode = new CConditionNode(*pDragNode);
	}	
	else
		return;

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (GetDocument());

	
	// target is a node
	if ( m_mapItemToNode.Lookup( m_hitemDrop, pTargetNode ) )
	{
		pTargetNode->m_ctSubTree.AddTail( pNewNode );
		pNewNode->m_pParentNode = pTargetNode;
	}
	// target is a condition
	else
	{
		CString name = tree.GetItemText( m_hitemDrop );
		if (pDoc->m_conditions.Lookup( name, (void*&)pTargetCond ) )
		{
			pTargetCond->m_ctConditionTree.AddTail( pNewNode );		
			pNewNode->m_pParentNode = NULL;
		}
		else
		{
			AfxMessageBox(_T("Error : root condition not found"));
			delete pNewNode;
		}
	}

	// for the moment, update all views, but moving branch may be faster
	pDoc->UpdateAllViews(NULL);
	/*
	HTREEITEM htiNew = CopyBranch( m_hitemDrag, m_hitemDrop, TVI_LAST );
	*/
}



void CConditionsView::OnCopyAsSibling()
{
	CTreeCtrl &tree = GetTreeCtrl();
	if (m_hitemDrop == tree.GetRootItem() )
		return;

	m_hitemDrop = tree.GetParentItem( m_hitemDrop);

	OnCopyAsChild();
}
