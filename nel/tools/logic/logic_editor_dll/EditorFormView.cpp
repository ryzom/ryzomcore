// EditorFormView.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "EditorFormView.h"
#include "logic_editorDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorFormView

IMPLEMENT_DYNCREATE(CEditorFormView, CFormView)

CEditorFormView::CEditorFormView()
	: CFormView(CEditorFormView::IDD)
{
	//{{AFX_DATA_INIT(CEditorFormView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pPropertySheet = NULL;
	m_bInitDone = FALSE;
}

CEditorFormView::~CEditorFormView()
{
	if (m_pPropertySheet != NULL)
		delete m_pPropertySheet;
}

void CEditorFormView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorFormView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorFormView, CFormView)
	//{{AFX_MSG_MAP(CEditorFormView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorFormView diagnostics

#ifdef _DEBUG
void CEditorFormView::AssertValid() const
{
	CFormView::AssertValid();
}

void CEditorFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorFormView message handlers

void CEditorFormView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style to ensure proper redrawing of controls during window resizing or views update
	ModifyStyle(NULL,WS_CLIPCHILDREN );

	CRect rectPlaceholder; 
	GetWindowRect(rectPlaceholder);	
	m_pPropertySheet = new CEditorPropertySheet(_T(""),this); 
	
	if (!m_pPropertySheet->Create(this,WS_CHILD | WS_VISIBLE, 0) ) 
	{
		delete m_pPropertySheet; 
		m_pPropertySheet = NULL; 
		return;
	}
	
	// adapt to window and sheets sizes
	m_pPropertySheet->GetWindowRect(m_rectInitialSheet);

	int nHeight,nWidth;

	if (rectPlaceholder.Height() < m_rectInitialSheet.Height() )
		nHeight = m_rectInitialSheet.Height();
	else
		nHeight = rectPlaceholder.Height();

	if (rectPlaceholder.Width() < m_rectInitialSheet.Width() )
		nWidth = m_rectInitialSheet.Width();
	else
		nWidth = rectPlaceholder.Width();

	// center in view window and draw property pages
	m_pPropertySheet->SetWindowPos(NULL, 0, 0, 
									 nWidth, nHeight, 
									 SWP_SHOWWINDOW);//*SWP_NOZORDER | SWP_NOACTIVATE); 

	

	CLogic_editorDoc * pDoc = (CLogic_editorDoc*)GetDocument();

	POSITION pos;
	CString eltName;
	// init the variables
	for( pos = pDoc->m_variables.GetHeadPosition(); pos != NULL; )
	{
		eltName = pDoc->m_variables.GetNext( pos );
		m_pPropertySheet->m_variablePage.addVariable( pDoc, eltName );
	}

	m_bInitDone = TRUE;
}


