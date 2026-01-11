// logic_editorView.cpp : implementation of the CLogic_editorView class
//

#include "stdafx.h"
#include "logic_editor.h"

#include "logic_editorDoc.h"
#include "logic_editorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorView

IMPLEMENT_DYNCREATE(CLogic_editorView, CView)

BEGIN_MESSAGE_MAP(CLogic_editorView, CView)
	//{{AFX_MSG_MAP(CLogic_editorView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorView construction/destruction

CLogic_editorView::CLogic_editorView()
{
	// TODO: add construction code here

}

CLogic_editorView::~CLogic_editorView()
{
}

BOOL CLogic_editorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorView drawing

void CLogic_editorView::OnDraw(CDC* pDC)
{
	CLogic_editorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorView diagnostics

#ifdef _DEBUG
void CLogic_editorView::AssertValid() const
{
	CView::AssertValid();
}

void CLogic_editorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLogic_editorDoc* CLogic_editorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLogic_editorDoc)));
	return (CLogic_editorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorView message handlers

