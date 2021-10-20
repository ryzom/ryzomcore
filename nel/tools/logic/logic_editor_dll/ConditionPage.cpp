// ConditionPage.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "ConditionPage.h"
#include "Condition.h"

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
// CConditionPage property page

IMPLEMENT_DYNCREATE(CConditionPage, CPropertyPage)

CConditionPage::CConditionPage() : CPropertyPage(CConditionPage::IDD)
{
	//{{AFX_DATA_INIT(CConditionPage)
	m_sType = _T("Comparison");
	m_sOperator = _T("<");
	m_sVarName = _T("");
	m_sSubCondName = _T("");
	m_sConditionName = _T("");
	m_dComparand = 0.0;
	//}}AFX_DATA_INIT
}

CConditionPage::~CConditionPage()
{
}

void CConditionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConditionPage)
	DDX_Control(pDX, IDC_EDIT_NODE_COMPARAND, m_ctrlComparand);
	DDX_CBString(pDX, IDC_COMBO_NODE_TYPE, m_sType);
	DDX_CBString(pDX, IDC_COMBO_NODE_OPERATOR, m_sOperator);
	DDX_CBString(pDX, IDC_COMBO_NODE_VAR_NAME, m_sVarName);
	DDX_CBString(pDX, IDC_COMBO_NODE_COND_NAME, m_sSubCondName);
	DDX_Text(pDX, IDC_EDIT_CONDITION_NAME, m_sConditionName);
	DDX_Text(pDX, IDC_EDIT_NODE_COMPARAND, m_dComparand);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConditionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CConditionPage)
	ON_CBN_SELCHANGE(IDC_COMBO_NODE_TYPE, OnSelchangeComboNodeType)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CONDITION, OnButtonAddCondition)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NODE, OnButtonAddNode)
	ON_BN_CLICKED(IDC_BUTTON_COND_APPLY, OnButtonCondApply)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_CONDITION, OnButtonDeleteCondition)
	ON_BN_CLICKED(IDC_BUTTON_NODE_APPLY, OnButtonNodeApply)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_NODE, OnButtonDeleteNode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()






void CConditionPage::Update()
{
	// get the variables existing in the doc
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);

	// list of variables
	CComboBox *comboBox = static_cast<CComboBox *> (GetDlgItem(IDC_COMBO_NODE_VAR_NAME) );

	comboBox->ResetContent();

	POSITION pos = pDoc->m_variables.GetHeadPosition();

	while (pos != NULL)
	{
		comboBox->AddString( pDoc->m_variables.GetNext(pos) );		
	}

	// counters are variables too
	pos = pDoc->m_counters.GetStartPosition();
	void *pointer;
	CString name;

	while (pos != NULL)
	{
		pDoc->m_counters.GetNextAssoc( pos, name, pointer);
		comboBox->AddString( name );		
	}

	// list of conditions
	comboBox = static_cast<CComboBox *> (GetDlgItem(IDC_COMBO_NODE_COND_NAME) );

	comboBox->ResetContent();

	pos = pDoc->m_conditions.GetStartPosition();
	while (pos != NULL)
	{
		pDoc->m_conditions.GetNextAssoc( pos, name, pointer);
		comboBox->AddString( name );		
	}

	/////
	if ( m_pSelectedCondition != NULL)
	{
		m_sConditionName = m_pSelectedCondition->m_sName;
		
		const CString tempStr = m_sType;
		
		if ( m_pSelectedConditionNode != NULL)
		{
			switch( m_pSelectedConditionNode->m_type )
			{
			case CConditionNode::NOT:
				m_sType = "NOT";
				break;
			case CConditionNode::TERMINATOR:
				m_sType = "Terminator";
				break;
			case CConditionNode::SUB_CONDITION:
				m_sType = "Sub-condition";
				break;
			case CConditionNode::COMPARISON:
				m_sType = "Comparison";
				break;
			}
			
			m_dComparand = m_pSelectedConditionNode->m_dComparand;
			m_sSubCondName = m_pSelectedConditionNode->m_sConditionName;
			m_sOperator = m_pSelectedConditionNode->m_sOperator;
			m_sVarName = m_pSelectedConditionNode->m_sVariableName;			
		}		
		
		UpdateData(FALSE);
		if (tempStr != m_sType)
			OnSelchangeComboNodeType();
		
	}
}



BOOL CConditionPage::checkNodeValidity()
{
	UpdateData();

	if ( m_sType == "NOT" )
	{
		// always valid
		return TRUE;
	}
	else if ( m_sType == "Terminator" )
	{
		// always valid
		return TRUE;
	}
	else if ( m_sType == "Sub-condition" )
	{
		if ( m_sConditionName.IsEmpty() )
			return FALSE;
	}
	else if ( m_sType == "Comparison" )
	{
		if ( m_sOperator.IsEmpty() || m_sVarName.IsEmpty() )
			return FALSE;
	}

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CConditionPage message handlers

BOOL CConditionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_pSelectedCondition = NULL;
	m_pSelectedConditionNode = NULL;

	//disable all optionnal controls
	/*
	GetDlgItem(IDC_COMBO_NODE_VAR_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_NODE_OPERATOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_NODE_COMPARAND)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COMPARISON)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COMPARAND)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_OPERATOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_VAR_NAME)->EnableWindow(FALSE);
	*/
	GetDlgItem(IDC_COMBO_NODE_COND_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COND_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_SUB_COND)->EnableWindow(FALSE);

	Update();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConditionPage::OnSelchangeComboNodeType() 
{
	UpdateData();

	//disable all optionnal controls
	GetDlgItem(IDC_COMBO_NODE_VAR_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_NODE_OPERATOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_NODE_COMPARAND)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COMPARISON)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COMPARAND)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_OPERATOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_VAR_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_NODE_COND_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_COND_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_SUB_COND)->EnableWindow(FALSE);

	if ( m_sType == _T("Comparison") )
	{
		//enable comparison related controls
		GetDlgItem(IDC_COMBO_NODE_VAR_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_NODE_OPERATOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_NODE_COMPARAND)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_COMPARISON)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_COMPARAND)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_OPERATOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_VAR_NAME)->EnableWindow(TRUE);
	}	
	else if ( m_sType == _T("Sub-condition") )
	{
		GetDlgItem(IDC_COMBO_NODE_COND_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_COND_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_SUB_COND)->EnableWindow(TRUE);
	}
/*	else if ( m_sType == _T("NOT") )
	{		
	}
	else // terminator
	{
	}
*/
//	UpdateData(FALSE);
}



//--------------------------------------------------------
//	addCondition
//
//--------------------------------------------------------
void CConditionPage::addCondition( CLogic_editorDoc *pDoc, CCondition * condition )
{
	// check if a condition with the same name already exist
	CCondition *pCondition;
	if (pDoc->m_conditions.Lookup( condition->m_sName, (void*&)pCondition))
	{
		AfxMessageBox(_T("A condition with this name already exist..."));
		return;
	}
	
	// add the condition
	pDoc->m_conditions.SetAt( condition->m_sName, condition );

	// update Views
	pDoc->UpdateAllViews( (CView*)this->GetParent() );	

} // addCondition //



//--------------------------------------------------------
//	OnButtonAddCondition
//
//--------------------------------------------------------
void CConditionPage::OnButtonAddCondition() 
{
	UpdateData();

	if (m_sConditionName.IsEmpty())
	{
		AfxMessageBox(_T("Condition Name cannot be empty, please choose a name"));
		return;
	}

	// if a counter with this name already exist, do nothing
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	// get the document
	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);	
	
	// create condition
	CCondition * pCondition = new CCondition();
	pCondition->m_sName = m_sConditionName;

	// add the condition
	addCondition( pDoc, pCondition );

} // OnButtonAddCondition //




void CConditionPage::OnButtonAddNode() 
{
	// check that data are valid
	if ( checkNodeValidity() == FALSE)
	{
		AfxMessageBox(_T("Invalid node datas - cannot add node"));
		return;
	}

	//
	if (m_pSelectedCondition == NULL)
		return;

	CConditionNode *newNode = new CConditionNode( m_pSelectedConditionNode );

	if (this->m_sType == "NOT")
	{
		newNode->m_type = CConditionNode::NOT;
	}
	else if (this->m_sType == "Terminator")
	{
		newNode->m_type = CConditionNode::TERMINATOR;
	}
	else if (this->m_sType == "Sub-condition")
	{
		newNode->m_type = CConditionNode::SUB_CONDITION;
		newNode->m_sConditionName = this->m_sSubCondName;
	}
	else if (this->m_sType == "Comparison")
	{
		newNode->m_type = CConditionNode::COMPARISON;
		newNode->m_dComparand = this->m_dComparand;	
		newNode->m_sOperator = this->m_sOperator;
		newNode->m_sVariableName =this->m_sVarName;
	}

	if (m_pSelectedConditionNode != NULL)
	{		
		m_pSelectedConditionNode->m_ctSubTree.AddTail( newNode );
	}
	else
	{
		m_pSelectedCondition->m_ctConditionTree.AddTail( newNode );
	}

	//
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);
	// update views
	pDoc->UpdateAllViews( (CView*)this->GetParent() );		
}



//---------------------------------------------------------
//	OnSetActive
//
//---------------------------------------------------------
BOOL CConditionPage::OnSetActive() 
{
	// get the child frame
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->MDIGetActive();

	// get the form view
	CEditorFormView *pFormView = static_cast<CEditorFormView *> ( pChild->m_wndSplitter.GetPane(0,1) );
	ASSERT_VALID(pFormView);	
	
	// get the document
	CLogic_editorDoc * pDoc = (CLogic_editorDoc*)pFormView->GetDocument();

	/*
	if( pDoc->InitConditionPage )
	{
		// init the conditions
		POSITION pos;
		CString eltName;
		for( pos = pDoc->m_conditions.GetStartPosition(); pos != NULL; )
		{
			CCondition * pCondition = new CCondition();
			pDoc->m_conditions.GetNextAssoc( pos, eltName, (void*&)pCondition );
			addCondition( pDoc, pCondition );
		}
	}
	pDoc->InitConditionPage = FALSE;
*/
	
	Update();
		
	return CPropertyPage::OnSetActive();

} // OnSetActive //



void CConditionPage::OnButtonCondApply() 
{
	if (m_pSelectedCondition != NULL)
	{
		UpdateData();

		if (m_sConditionName.IsEmpty())
		{
			AfxMessageBox(_T("Condition Name cannot be empty, please choose a name"));
			return;
		}

		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);

		pDoc->changeConditionName( m_pSelectedCondition->m_sName, m_sConditionName );

		Update();
		pDoc->UpdateAllViews( (CView*)this->GetParent() );
	}
	else
	{
		AfxMessageBox(_T("No condition selected ! Choose a condition first"));
	}
		
}

void CConditionPage::OnButtonDeleteCondition() 
{
	if (m_pSelectedCondition != NULL)
	{
		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);

		pDoc->deleteCondition(m_pSelectedCondition->m_sName );

		m_pSelectedCondition	= NULL;
		m_pSelectedConditionNode = NULL;

		Update();
		pDoc->UpdateAllViews( (CView*)this->GetParent() );
	}
	else
	{
		AfxMessageBox(_T("No condition selected ! Choose a condition first"));
	}	
}

void CConditionPage::OnButtonNodeApply() 
{
	if (this->m_pSelectedConditionNode != NULL)
	{
		// check that data are valid
		if ( checkNodeValidity() == FALSE)
		{
			AfxMessageBox(_T("Invalid node datas - cancel node modification"));
			return;
		}

		m_pSelectedConditionNode->m_sConditionName.Empty();
		m_pSelectedConditionNode->m_sOperator.Empty();
		m_pSelectedConditionNode->m_sVariableName.Empty();
		m_pSelectedConditionNode->m_dComparand = 0;

		if (this->m_sType == "NOT")
		{
			m_pSelectedConditionNode->m_type = CConditionNode::NOT;
		}
		if (this->m_sType == "Terminator")
		{
			m_pSelectedConditionNode->m_type = CConditionNode::TERMINATOR;
		}
		if (this->m_sType == "Sub-condition")
		{
			m_pSelectedConditionNode->m_type = CConditionNode::SUB_CONDITION;
			m_pSelectedConditionNode->m_sConditionName = this->m_sSubCondName;
		}
		if (this->m_sType == "Comparison")
		{
			m_pSelectedConditionNode->m_type = CConditionNode::COMPARISON;
			m_pSelectedConditionNode->m_dComparand = this->m_dComparand;	
			m_pSelectedConditionNode->m_sOperator = this->m_sOperator;
			m_pSelectedConditionNode->m_sVariableName =this->m_sVarName;
		}


		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);

		pDoc->UpdateAllViews( (CView*)this->GetParent() );
	}
	else
	{
		AfxMessageBox(_T("No condition node selected ! Choose a node first"));
	}	
}

void CConditionPage::OnButtonDeleteNode() 
{
	if ( m_pSelectedConditionNode != NULL)
	{
		CConditionNode *parentNode = m_pSelectedConditionNode->getParentNode();
		if ( parentNode != NULL)
		{
			POSITION pos = parentNode->m_ctSubTree.Find( m_pSelectedConditionNode );
			parentNode->m_ctSubTree.RemoveAt( pos );
		}
		else
		{
			ASSERT( m_pSelectedCondition );

			POSITION pos = m_pSelectedCondition->m_ctConditionTree.Find( m_pSelectedConditionNode );
			m_pSelectedCondition->m_ctConditionTree.RemoveAt( pos );
		}

		delete m_pSelectedConditionNode;
		m_pSelectedConditionNode = NULL;

		Update();

		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);

		pDoc->UpdateAllViews( (CView*)this->GetParent() );
	}
	else
	{
		AfxMessageBox(_T("No condition node selected ! Choose a node first"));
	}		
}
