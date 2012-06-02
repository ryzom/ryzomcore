// StatesView.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "StatesView.h"


#include "Logic_editorDoc.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "EditorFormView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatesView

IMPLEMENT_DYNCREATE(CStatesView, CTreeView)

CStatesView::CStatesView()
{
}

CStatesView::~CStatesView()
{
}


BEGIN_MESSAGE_MAP(CStatesView, CTreeView)
	//{{AFX_MSG_MAP(CStatesView)
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatesView drawing

void CStatesView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CStatesView diagnostics

#ifdef _DEBUG
void CStatesView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CStatesView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG








void CStatesView::expand(UINT nCode, HTREEITEM hItem) 
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






/////////////////////////////////////////////////////////////////////////////
// CStatesView message handlers

void CStatesView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	/************* TO DO : REDRAW ONLY IF Some states have changed *****/

	SetRedraw(FALSE);

	CTreeView::OnUpdate(pSender,lHint,pHint);
	
	
	BOOL bSelection = FALSE;


	CLogic_editorDoc *pDoc =  static_cast<CLogic_editorDoc*>(GetDocument());
	ASSERT_VALID(pDoc);

	CTreeCtrl &m_tree = GetTreeCtrl();

	m_tree.DeleteAllItems();
	m_mapItemToEvent.RemoveAll();

	m_tree.InsertItem( "States");

	// get the states map
	CState *pState;
	CString stateName;
	POSITION pos = pDoc->m_states.GetStartPosition();
	HTREEITEM hItem;

	while (pos != NULL)
	{
		pDoc->m_states.GetNextAssoc( pos, stateName, (void*&)pState );
		hItem = m_tree.InsertItem( stateName , m_tree.GetRootItem());

		// if this is the selected state (and there was no selected event), select it)
		if ( m_pSelectedEvent == NULL && m_pSelectedState == pState )
		{
			m_tree.SelectItem( hItem);
			bSelection = TRUE;
		}
		
		// insert every event as children nodes
		CEvent *pEvent = NULL;
		HTREEITEM hItemCurrent;

		POSITION eventPos = pState->m_evEvents.GetHeadPosition();
		while (eventPos != NULL)
		{
			pEvent = pState->m_evEvents.GetNext( eventPos );
			if (pEvent != NULL)
			{
				hItemCurrent = m_tree.InsertItem( pEvent->getEventAsString() , hItem);
				m_mapItemToEvent.SetAt( hItemCurrent, pEvent );
				// if this is the selected event, select it
				if ( m_pSelectedEvent == pEvent )
				{
					m_tree.SelectItem( hItemCurrent);
					bSelection = TRUE;
				}
			}
		}		
	}

	// expand all tree
	expand(/*TVE_EXPAND ,NULL*/ );

	//
	if (!bSelection)
	{
		m_pSelectedEvent = NULL;
		m_pSelectedState = NULL;
	}
		
	SetRedraw(/*TRUE*/);	
}

BOOL CStatesView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// modify arborescence style
	cs.style |=(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS/*|TVS_EDITLABELS*/);
//	cs.style &= ~TVS_DISABLEDRAGDROP;
	
	return CTreeView::PreCreateWindow(cs);
}

void CStatesView::OnKillFocus(CWnd* pNewWnd) 
{
//	CTreeView::OnKillFocus(pNewWnd);	
}

void CStatesView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	
	m_pSelectedEvent = NULL;
	m_pSelectedState = NULL;
	
	
	//
	CTreeCtrl &treeCtrl = GetTreeCtrl();
		
	HTREEITEM hItem;
	hItem = treeCtrl.GetSelectedItem();

	//
	if (hItem == treeCtrl.GetRootItem())
		return;

	// get the event selected (if any)
	if ( ! m_mapItemToEvent.Lookup(hItem, m_pSelectedEvent) )
		m_pSelectedEvent = NULL;
	else
	{
		// nothing special
	}
	
	// get the state (if any)
	while ( treeCtrl.GetParentItem( hItem ) != treeCtrl.GetRootItem() )
		hItem = treeCtrl.GetParentItem( hItem );

	CString stateName = treeCtrl.GetItemText( hItem );

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (GetDocument());
	
	//  to update the fields of the condition page in the form view
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	CEditorFormView *pFormView = static_cast<CEditorFormView *> ( pChild->m_wndSplitter.GetPane(0,1) );
	CStatePage *pStatePage = static_cast<CStatePage*> ( pFormView->m_pPropertySheet->GetPage(3) );

	if ( pDoc->m_states.Lookup( stateName, (void*&) m_pSelectedState) )
	{		
		pFormView->m_pPropertySheet->SetActivePage( pStatePage );
		
		if ( this->m_pSelectedEvent == NULL || pStatePage->m_pSelectedEvent != this->m_pSelectedEvent)
		{
			pStatePage->m_pSelectedEvent = this->m_pSelectedEvent;
			pStatePage->m_pSelectedState = this->m_pSelectedState;

			pStatePage->Update();
		}
	}
	else //this->m_pSelectedState == NULL
	{
		if (pStatePage->m_pSelectedState != NULL)
		{
			pStatePage->m_pSelectedEvent = this->m_pSelectedEvent; // = NULL
			pStatePage->m_pSelectedState = this->m_pSelectedState; // = NULL

			pStatePage->Update();
		}
	}
	//
	*pResult = 0;
}

void CStatesView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	m_pSelectedEvent = NULL;
	m_pSelectedState = NULL;
}
