// StatePage.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "StatePage.h"

#include "ChildFrm.h"
#include "MainFrm.h"
#include "LogicTreeView.h"
#include "StatesView.h"
#include "EditorFormView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatePage property page

IMPLEMENT_DYNCREATE(CStatePage, CPropertyPage)

CStatePage::CStatePage() : CPropertyPage(CStatePage::IDD)
{
	//{{AFX_DATA_INIT(CStatePage)
	m_nEventMessage = 0;
	m_sConditionName = _T("");
	m_sNextStateName = _T("");
	m_sStateName = _T("");
	m_sMessageID = _T("");
	m_sArgument = _T("");
	m_sDestination = _T("");
	//}}AFX_DATA_INIT
}

CStatePage::~CStatePage()
{
}

void CStatePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatePage)
	DDX_Radio(pDX, IDC_RADIO_STATE_EVENT_MSG, m_nEventMessage);
	DDX_CBString(pDX, IDC_COMBO_STATE_COND_NAMES, m_sConditionName);
	DDX_CBString(pDX, IDC_COMBO_STATE_CHANGE, m_sNextStateName);
	DDX_Text(pDX, IDC_EDIT_STATE_NAME, m_sStateName);
	DDX_CBString(pDX, IDC_COMBO_STATE_MSG_ID, m_sMessageID);
	DDX_Text(pDX, IDC_EDIT_STATE_ARGUMENT, m_sArgument);
	DDX_Text(pDX, IDC_EDIT_STATE_MSG_DEST, m_sDestination);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStatePage, CPropertyPage)
	//{{AFX_MSG_MAP(CStatePage)
	ON_BN_CLICKED(IDC_RADIO_STATE_CHANGE, OnRadioStateChange)
	ON_BN_CLICKED(IDC_RADIO_STATE_EVENT_MSG, OnRadioStateEventMsg)
	ON_BN_CLICKED(IDC_BUTTON_ADD_STATE, OnButtonAddState)
	ON_BN_CLICKED(IDC_BUTTON_ADD_EVENT, OnButtonAddEvent)
	ON_BN_CLICKED(IDC_BUTTON_STATE_REMOVE, OnButtonStateRemove)
	ON_BN_CLICKED(IDC_BUTTON_STATE_APPLY, OnButtonStateApply)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_APPLY, OnButtonEventApply)
	ON_BN_CLICKED(IDC_BUTTON_EVENT_REMOVE, OnButtonEventRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()





void CStatePage::Update()
{
	if (m_pSelectedState != NULL)
	{

		m_sStateName = m_pSelectedState->m_sName;

		if (m_pSelectedEvent != NULL)
		{
			this->m_sConditionName = m_pSelectedEvent->m_sConditionName;

			if (m_pSelectedEvent->m_bActionIsMessage == TRUE)
			{
				OnRadioStateEventMsg();

				this->m_nEventMessage = 0;
				this->m_sArgument = m_pSelectedEvent->m_sArguments;		
				this->m_sDestination = m_pSelectedEvent->m_sMessageDestination;
				this->m_sMessageID = m_pSelectedEvent->m_sMessageID;

				this->m_sNextStateName.Empty();
			}
			else
			{
				OnRadioStateChange();

				this->m_nEventMessage = 1; 

				this->m_sArgument.Empty();
				this->m_sDestination.Empty();
				this->m_sMessageID.Empty();
				this->m_sNextStateName = m_pSelectedEvent->m_sStateChange;
			}
		}
		else
		{
			m_sConditionName.Empty();
			m_sArgument.Empty();
			m_sDestination.Empty();
			m_sMessageID.Empty();
			m_sNextStateName.Empty();
		}
	}
	else
	{
		m_sStateName.Empty();
	}

	// get the document
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);

	// list of conditions
	CComboBox *comboBox = static_cast<CComboBox *> (GetDlgItem(IDC_COMBO_STATE_COND_NAMES) );

	comboBox->ResetContent();

	POSITION pos = pDoc->m_conditions.GetStartPosition();
	void *pointer;
	CString name;

	while (pos != NULL)
	{
		pDoc->m_conditions.GetNextAssoc( pos, name, pointer);
		comboBox->AddString( name );		
	}
	
	// list of states
	comboBox = static_cast<CComboBox *> (GetDlgItem(IDC_COMBO_STATE_CHANGE) );

	comboBox->ResetContent();

	pos = pDoc->m_states.GetStartPosition();
	while (pos != NULL)
	{
		pDoc->m_states.GetNextAssoc( pos, name, pointer);
		comboBox->AddString( name );		
	}
	//

	UpdateData(FALSE);
}




/////////////////////////////////////////////////////////////////////////////
// CStatePage message handlers

BOOL CStatePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_pSelectedState = NULL;
	m_pSelectedEvent = NULL;

	// select the State Change radio button
	if ( m_nEventMessage == 0)
		OnRadioStateEventMsg();
	else if ( m_nEventMessage == 1)
		OnRadioStateChange();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CStatePage::OnRadioStateChange() 
{
	GetDlgItem(IDC_STATIC_STATE_CHANGE)->EnableWindow();
	GetDlgItem(IDC_COMBO_STATE_CHANGE)->EnableWindow();
	
	GetDlgItem(IDC_STATIC_STATE_MSG1)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_STATE_MSG2)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_STATE_MSG3)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_STATE_MSG_ID)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_STATE_ARGUMENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_STATE_MSG_DEST)->EnableWindow(FALSE);
}

void CStatePage::OnRadioStateEventMsg() 
{
	GetDlgItem(IDC_STATIC_STATE_CHANGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_STATE_CHANGE)->EnableWindow(FALSE);

	GetDlgItem(IDC_STATIC_STATE_MSG1)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_STATE_MSG2)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_STATE_MSG3)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_STATE_MSG_ID)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_STATE_ARGUMENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_STATE_MSG_DEST)->EnableWindow(TRUE);
}




//---------------------------------------------------------
//	addState
//
//---------------------------------------------------------
void CStatePage::addState( CLogic_editorDoc *pDoc, CState * state)
{
	// check whether this state is already in the combo box
	CComboBox *comboBox = static_cast<CComboBox *> (GetDlgItem(IDC_COMBO_STATE_CHANGE) );
	if( comboBox->FindStringExact(0, state->m_sName) == LB_ERR )
	{
		// add the state in the combo box
		comboBox->AddString( state->m_sName );
	}

	// check whether this state already exists
	void *pState;
	if( pDoc->m_states.Lookup(state->m_sName, pState) == FALSE )
	{	
		// add the new state to the states tree, and to the vector of states in the document
		pDoc->m_states.SetAt( state->m_sName, state );
	}
	
	// update views
	pDoc->UpdateAllViews( (CView*)this->GetParent() );
	UpdateData(FALSE);

} // addState //



//---------------------------------------------------------
//	OnButtonAddState
//
//---------------------------------------------------------
void CStatePage::OnButtonAddState() 
{
	UpdateData();
	
	if (m_sStateName.IsEmpty())
	{
		AfxMessageBox(_T("State name cannot be empty, please enter a valid name"));
		return;
	}

	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	/// \toto Malkav : check if a state with this name does not already exist in the document
	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);	

	// create the new state
	CState *state = new CState(m_sStateName);

	// add the state
	addState( pDoc, state );
	

} // OnButtonAddState //




void CStatePage::OnButtonAddEvent() 
{
	UpdateData();

	// do nothing is no state is selected
	if (m_pSelectedState == NULL)
		return;

	CEvent *pEvent = new CEvent();

	pEvent->m_sConditionName = this->m_sConditionName;

	if (this->m_nEventMessage == 0)
	{
		pEvent->m_bActionIsMessage = TRUE;
		pEvent->m_sStateChange.Empty();
		pEvent->m_sArguments = this->m_sArgument;
		pEvent->m_sMessageDestination = this->m_sDestination;
		pEvent->m_sMessageID = this->m_sMessageID;
	}
	else // state change
	{
		pEvent->m_bActionIsMessage = FALSE;
		pEvent->m_sStateChange = this->m_sNextStateName;
		pEvent->m_sArguments.Empty();
		pEvent->m_sMessageDestination.Empty();
		pEvent->m_sMessageID.Empty();
	}

	// add the event to the selected state
	m_pSelectedState->addEvent( pEvent );

	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);	

	pDoc->UpdateAllViews( (CView*)this->GetParent() );
}



//---------------------------------------------------------
//	OnSetActive
//
//---------------------------------------------------------
BOOL CStatePage::OnSetActive() 
{
	/*
	// get the child frame
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->MDIGetActive();

	// get the form view
	CEditorFormView *pFormView = static_cast<CEditorFormView *> ( pChild->m_wndSplitter.GetPane(0,1) );
	ASSERT_VALID(pFormView);	
	
	// get the document
	CLogic_editorDoc * pDoc = (CLogic_editorDoc*)pFormView->GetDocument();

	if( pDoc->InitStatePage )
	{
		// init the states
		POSITION pos;
		CString eltName;
		for( pos = pDoc->m_states.GetStartPosition(); pos != NULL; )
		{
			CState * pState = new CState();
			pDoc->m_states.GetNextAssoc( pos, eltName, (void*&)pState );
			addState( pDoc, pState );
		}
	}
	pDoc->InitStatePage = FALSE;
	*/
	
	Update();
	
	return CPropertyPage::OnSetActive();

} // OnSetActive //





void CStatePage::OnButtonStateRemove() 
{
	// get selected state 
	if (m_pSelectedState != NULL)
	{
		if (AfxMessageBox(_T("Your are about to permanently delete this state.\nDoing so will invalidate all references to this state.\nDo you want to continue anyway ?") ,MB_OKCANCEL|MB_DEFBUTTON2) == IDOK)
		{
			CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
			CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
			CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
			ASSERT_VALID(pDoc);	

			pDoc->deleteState( m_pSelectedState->m_sName );

			m_pSelectedState = NULL;
			m_pSelectedEvent = NULL;

			Update();
			pDoc->UpdateAllViews( (CView*)this->GetParent() );
		}		
	}
	else
	{
		AfxMessageBox(_T("No state selected ! Choose a state first"));
	}
}




void CStatePage::OnButtonStateApply() 
{
	UpdateData();

	if (m_pSelectedState != NULL)
	{
		if (m_sStateName.IsEmpty())
		{
			AfxMessageBox(_T("State name cannot be empty, please enter a valid name"));
			return;
		}

		if (AfxMessageBox(_T("Your are about to change this state name.\nDoing so will change all occurence of the old name to the new one.\nDo you want to continue anyway ?") ,MB_OKCANCEL|MB_DEFBUTTON2) == IDOK)
		{
			CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
			CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
			CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
			ASSERT_VALID(pDoc);	

			pDoc->changeStateName( m_pSelectedState->m_sName, m_sStateName );
			
			Update();
			pDoc->UpdateAllViews( (CView*)this->GetParent() );
		}
	}
	else
	{
		AfxMessageBox(_T("No state selected ! Choose a state first"));
	}
}

void CStatePage::OnButtonEventApply() 
{
	// TODO: Add your control notification handler code here
	AfxMessageBox(_T("Features not implemented"));
}

void CStatePage::OnButtonEventRemove() 
{
	// get selected state 
	if (m_pSelectedEvent != NULL && m_pSelectedState != NULL)
	{
		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();
		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);	

		m_pSelectedState->removeEvent( m_pSelectedEvent );

		m_pSelectedEvent = NULL;

//		Update();
		pDoc->UpdateAllViews( (CView*)this->GetParent() );	
	}
	else
	{
		AfxMessageBox(_T("No event selected ! Choose a event first"));
	}
}
