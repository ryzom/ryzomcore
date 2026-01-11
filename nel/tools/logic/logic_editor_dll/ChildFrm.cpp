// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "logic_editor.h"

#include "ChildFrm.h"
#include "LogicTreeView.h"
#include "EditorFormView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
		// create static splitter view with two panes
	VERIFY(m_wndSplitter.CreateStatic( this,1,2, WS_CHILD | WS_VISIBLE ));
	VERIFY(m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CLogicTreeView),CSize(180,0),pContext));
	VERIFY(m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CEditorFormView),CSize(0,0),pContext));	
	
	//return CMDIChildWnd::OnCreateClient(lpcs, pContext);
	return TRUE;
}

