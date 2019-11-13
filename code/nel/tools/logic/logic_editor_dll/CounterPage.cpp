// CounterPage.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "CounterPage.h"
#include "Counter.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "logic_editorDoc.h"
#include "EditorFormView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCounterPage property page

IMPLEMENT_DYNCREATE(CCounterPage, CPropertyPage)

CCounterPage::CCounterPage() : CPropertyPage(CCounterPage::IDD)
{
	//{{AFX_DATA_INIT(CCounterPage)
	m_sMode = _T("Loop");
	m_sWay = _T("Increment");
	m_sCounterName = _T("");
	m_nUpperLimit = 0;
	m_nLowerLimit = 0;
	//}}AFX_DATA_INIT
}

CCounterPage::~CCounterPage()
{
}

void CCounterPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCounterPage)
	DDX_Control(pDX, IDC_EDIT_COUNTER_LOWER_LIMIT, m_neLowerLimit);
	DDX_Control(pDX, IDC_LIST_COUNTERS, m_counters);
	DDX_CBString(pDX, IDC_COMBO_COUNTER_MODE, m_sMode);
	DDX_CBString(pDX, IDC_COMBO_COUNTER_INCDEC, m_sWay);
	DDX_Text(pDX, IDC_COUNTER_NAME, m_sCounterName);
	DDX_Text(pDX, IDC_EDIT_COUNTER_UPPER_LIMIT, m_nUpperLimit);
	DDX_Text(pDX, IDC_EDIT_COUNTER_LOWER_LIMIT, m_nLowerLimit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCounterPage, CPropertyPage)
	//{{AFX_MSG_MAP(CCounterPage)
	ON_BN_CLICKED(IDC_BUTTON_ADD_COUNTER, OnButtonAddCounter)
	ON_LBN_SELCHANGE(IDC_LIST_COUNTERS, OnSelchangeListCounters)
	ON_BN_CLICKED(IDC_BUTTON_COUNTER_REMOVE, OnButtonCounterRemove)
	ON_BN_CLICKED(IDC_BUTTON_COUNTER_APPLY, OnButtonCounterApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCounterPage message handlers

BOOL CCounterPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// init the combo boxes with the right values
//	CComboBox *comboBox = static_cast<CComboBox*> (GetDlgItem(IDC____));

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//---------------------------------------------------------
//	addCounter
//
//---------------------------------------------------------
void CCounterPage::addCounter( CLogic_editorDoc *pDoc, CCounter * pCounter )
{
	// check if a var or a counter with the same name already exist in the page
	if( m_counters.FindString(0,pCounter->m_sName) == LB_ERR )
	{
		m_counters.AddString( pCounter->m_sName );
	}

	// if the doc has not been loaded from file, the counter is not yet in doc
	void * pointer;
	if( (pDoc->m_variables.Find(pCounter->m_sName) == NULL) || (pDoc->m_counters.Lookup(pCounter->m_sName, pointer) == FALSE) )
	{
		pDoc->m_counters.SetAt( pCounter->m_sName, pCounter );
	}

	// update page
	UpdateData(FALSE);

} // addCounter //



//---------------------------------------------------------
//	OnButtonAddCounter
//
//---------------------------------------------------------
void CCounterPage::OnButtonAddCounter() 
{
	UpdateData();

	// if some infos are missing, do nothing
	if (m_sCounterName.IsEmpty() )
	{
		return;
	}

	// if a counter with thsi name already exist, do nothing
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);	

	CCounter *pCounter = new CCounter( m_sCounterName );
	pCounter->mode( m_sMode );
	pCounter->way( m_sWay );
	pCounter->lowerLimit( m_nLowerLimit );
	pCounter->upperLimit( m_nUpperLimit );

	addCounter( pDoc, pCounter );

} // OnButtonAddCounter //




void CCounterPage::OnSelchangeListCounters() 
{
	m_counters.GetText( m_counters.GetCurSel(), m_sCounterName);

	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);	

	void *p;
	if (pDoc->m_counters.Lookup( m_sCounterName, p) == FALSE)
	{
		return;
	}

	CCounter *pCounter = static_cast<CCounter*> (p);

	// set the value in the different fields
	m_sMode = pCounter->mode();
	m_sWay =  pCounter->way();
	m_nLowerLimit =  pCounter->lowerLimit();
	m_nUpperLimit =  pCounter->upperLimit();

	UpdateData(FALSE);
}





void CCounterPage::OnButtonCounterRemove() 
{
	UpdateData();

	//get the selected counter name
	const int sel = m_counters.GetCurSel();

	if (sel != LB_ERR)
	{
		CString name;
		m_counters.GetText( sel, name );

		// get the pointer from the document
		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);	

		pDoc->deleteCounter( name );
		m_counters.DeleteString( sel );
	}
	else
	{
		AfxMessageBox(_T("No counter selected ! Choose a counter first"));
	}
	
}

void CCounterPage::OnButtonCounterApply() 
{
	UpdateData();

	//get the selected counter name
	const int sel = m_counters.GetCurSel();

	if (sel != LB_ERR)
	{
		CString name;
		m_counters.GetText( sel, name );

		// get the pointer from the document
		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);	

		if ( pDoc->changeCounterName( name, m_sCounterName ) == FALSE)
			return;

		CCounter *pCounter = NULL;
		pDoc->m_counters.Lookup( m_sCounterName, (void*&)(pCounter) );

		if (!pCounter)
			return;

		pCounter->lowerLimit( m_nLowerLimit);
		pCounter->upperLimit( m_nUpperLimit);
		pCounter->mode( m_sMode);
		pCounter->way( m_sWay);

		m_counters.DeleteString( sel );
		m_counters.AddString( m_sCounterName );
	}	
	else
	{
		AfxMessageBox(_T("No counter selected ! Choose a counter first"));
	}
}




//---------------------------------------------------------
//	OnSetActive
//
//---------------------------------------------------------
BOOL CCounterPage::OnSetActive() 
{
	// get the child frame
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->MDIGetActive();

	// get the form view
	CEditorFormView *pFormView = static_cast<CEditorFormView *> ( pChild->m_wndSplitter.GetPane(0,1) );
	ASSERT_VALID(pFormView);	
	
	// get the document
	CLogic_editorDoc * pDoc = (CLogic_editorDoc*)pFormView->GetDocument();

	if( pDoc->InitConditionPage )
	{
		// init the counters
		POSITION pos;
		CString eltName;
		for( pos = pDoc->m_counters.GetStartPosition(); pos != NULL; )
		{
			CCounter * pCounter = new CCounter();
			pDoc->m_counters.GetNextAssoc( pos, eltName, (void*&)pCounter );
			addCounter( pDoc, pCounter );
		}
	}
	pDoc->InitConditionPage = FALSE;

	return CPropertyPage::OnSetActive();

} // OnSetActive //
