// LogicTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "LogicTreeView.h"

#include "StatesView.h"
#include "ConditionsView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeView

IMPLEMENT_DYNCREATE(CLogicTreeView, CView)

CLogicTreeView::CLogicTreeView()
{
}

CLogicTreeView::~CLogicTreeView()
{
}


BEGIN_MESSAGE_MAP(CLogicTreeView, CView)
	//{{AFX_MSG_MAP(CLogicTreeView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeView drawing

void CLogicTreeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeView diagnostics

#ifdef _DEBUG
void CLogicTreeView::AssertValid() const
{
	CView::AssertValid();
}

void CLogicTreeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeView message handlers


int CLogicTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  // Call base class first
  if (CView::OnCreate(lpCreateStruct) == -1)
    return -1;
 
  // CPaneContainerView is used to control the right pane that CMainFrame
  // sets up. In this case we create a second splitter window.
  if (!m_wndSplitter.CreateStatic(this, 2,1))
  {
	  TRACE0("Failed to create splitter window in LogicTreeView");
	  return -1;
  }

  // The context information is passed on from the framework
  CCreateContext *pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;

  // Create two views
  if ( ! m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CStatesView),CSize(0,250), pContext) )
  {
	  TRACE0("Failed to create states view in splitter window in LogicTreeView");
	  return -1;
  }

  if ( !m_wndSplitter.CreateView(1,0,RUNTIME_CLASS(CConditionsView),CSize(0,0), pContext) )
  {
	  TRACE0("Failed to create conditions view in splitter window in LogicTreeView");
	  return -1;
  }

  return 0;
}

/// resize the view
void CLogicTreeView::OnSize(UINT nType, int cx, int cy) 
{
  CView::OnSize(nType, cx, cy);

  // for simplicity, I'll assume Y border edge is 
  // same as X border edge
  const int xborderWidth = GetSystemMetrics(SM_CXEDGE);
  const int yborderWidth = GetSystemMetrics(SM_CYEDGE);

  // move and grow view to clip border
  m_wndSplitter.MoveWindow(
    -xborderWidth,
    -yborderWidth,
    cx+(xborderWidth*2),
    cy+(yborderWidth*2));
	
}
