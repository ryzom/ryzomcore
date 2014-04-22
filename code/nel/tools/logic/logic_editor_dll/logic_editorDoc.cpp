// logic_editorDoc.cpp : implementation of the CLogic_editorDoc class
//

#include "stdafx.h"
#include "logic_editor.h"

#include "logic_editorDoc.h"
#include "state.h"
#include "EditorFormView.h"
#include "ChildFrm.h"
#include "MainFrm.h"

#include "nel/logic/logic_state_machine.h"

//#include <nel/misc/o_xml.h>
//#include <nel/misc/i_xml.h>
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"


#include <vector>

using namespace NLMISC;
using namespace NLLOGIC;
using namespace std;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CLogic_editorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorDoc

#ifdef NL_NEW
	#undef new 
#endif
IMPLEMENT_DYNCREATE(CLogic_editorDoc, CDocument)

BEGIN_MESSAGE_MAP(CLogic_editorDoc, CDocument)
	//{{AFX_MSG_MAP(CLogic_editorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorDoc construction/destruction

CLogic_editorDoc::CLogic_editorDoc()
{
	InitCounterPage = FALSE;
	InitConditionPage = FALSE;
	InitStatePage = FALSE;

}

CLogic_editorDoc::~CLogic_editorDoc()
{
	// delete all objects
	CString name;
	void *pointer;

	// counters
	POSITION pos = m_counters.GetStartPosition();
	while (pos != NULL)	
	{
		m_counters.GetNextAssoc(pos, name, pointer);
		if (pointer != NULL)
			delete (static_cast<CCounter*>(pointer));
	}

	// conditions
	pos = m_conditions.GetStartPosition();
	while (pos != NULL)	
	{
		m_conditions.GetNextAssoc(pos, name, pointer);
		if (pointer != NULL)
			delete (static_cast<CCondition*>(pointer));
	}

	// states
	pos = m_states.GetStartPosition();
	while (pos != NULL)	
	{
		m_states.GetNextAssoc(pos, name, pointer);
		if (pointer != NULL)
			delete (static_cast<CState*>(pointer));
	}
}


BOOL CLogic_editorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CLogic_editorDoc serialization

void CLogic_editorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorDoc diagnostics

#ifdef _DEBUG
void CLogic_editorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLogic_editorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorDoc commands






BOOL CLogic_editorDoc::changeVarName( CString old, const CString &newName)
{
	if (old == newName)
		return TRUE;

	// find the var name, and change ALL it's occurences !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	void *p;
	if ( m_variables.Find( newName ) != NULL || m_counters.Lookup(newName, p) )
		return FALSE;

	const POSITION pos = m_variables.Find( old );
	
	if (pos == NULL)
		return FALSE;

	m_variables.RemoveAt( pos );
	m_variables.AddTail( newName );

	// find all occurences of old and replace them with newName
	// check ALL condition nodes and change occurences.... may take a long time !
	//POSITION nodePos;
	POSITION condPos = m_conditions.GetStartPosition();

	while (condPos != NULL)
	{
	 // TO DO
	}
	
	
	return TRUE;
}


void CLogic_editorDoc::deleteVar( CString name)
{
	// delete the var name, and change ALL it's occurences !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	POSITION pos = m_variables.Find( name );
	if (pos != NULL)
	{
		m_variables.RemoveAt( pos );
	}
}




BOOL CLogic_editorDoc::changeCounterName( CString old, const CString &newName)
{
	if (old == newName)
		return TRUE;

	// find the condition name, and change ALL it's occurences !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	void *address;
	if ( m_counters.Lookup( newName, address )  ||   m_variables.Find( newName ) != NULL )
		return FALSE;

	if ( m_counters.Lookup( old, address ) == FALSE )
		return FALSE;
	
	m_counters.RemoveKey( old );

	CCounter *pCounter = static_cast<CCounter*>(address);
	pCounter->name(newName);

	m_counters.SetAt( newName, pCounter );

	return TRUE;
}

void CLogic_editorDoc::deleteCounter( CString name)
{
	// delete the counter 'name', and change ALL it's occurences !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	if (! name.IsEmpty() )
	{
		CCounter *pCounter = NULL;
		if ( m_counters.Lookup( name, (void*&)pCounter ) != NULL)
		{			
			m_counters.RemoveKey( name );
			delete pCounter;
			pCounter = NULL;
		}	
	}
}




BOOL CLogic_editorDoc::changeConditionName( CString old, const CString &newName)
{
	if (old == newName)
		return TRUE;

	// find the condition name, and change ALL it's occurences !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	// TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO TO DO 
	void *address;
	if ( m_conditions.Lookup( newName, address ) != NULL)
		return FALSE;

	if ( m_conditions.Lookup( old, address ) == NULL)
		return FALSE;

	m_conditions.RemoveKey( old );	
		
	CCondition *pCondition = static_cast<CCondition*>(address);
	pCondition->m_sName = newName;

	m_conditions.SetAt(newName, pCondition );

	// look in all events to update the name
	CString temp_str;
	CState *pState;
	CEvent *pEvent;

	POSITION eventPos;
	POSITION statePos = m_states.GetStartPosition();
	while (statePos != NULL)
	{
		m_states.GetNextAssoc(statePos, temp_str, (void*&)pState );
		eventPos = pState->m_evEvents.GetHeadPosition();
		while( eventPos != NULL)
		{
			pEvent = pState->m_evEvents.GetNext( eventPos );
			if (pEvent->m_sConditionName == old)
				pEvent->m_sConditionName = newName;
		}
	}

	// look in all sub_cond blocks to update the name
	// ...
	CConditionNode *pNode;

	POSITION nodePos, oldNodePos; 
	POSITION pos = m_conditions.GetStartPosition();

	while (pos != NULL)
	{
		m_conditions.GetNextAssoc(pos, temp_str, (void*&)pCondition );
		if (pCondition !=NULL )
		{
			nodePos = pCondition->m_ctConditionTree.GetHeadPosition();
			while (nodePos != NULL)
			{
				oldNodePos = nodePos;
				pNode = pCondition->m_ctConditionTree.GetNext( nodePos );
				if (pNode)
				{
					pNode->changeConditionName( old, newName );
				}

			}
		}
	}

	return TRUE;
}





void CLogic_editorDoc::deleteCondition( CString name)
{
	// delete the var name, and change ALL it's occurences !!!!!!!!!!!!!!!!!!!!
	if ( name.IsEmpty() )
		return;

	CCondition *pCondition = NULL;

	if ( m_conditions.Lookup( name, (void*&)pCondition ) != NULL)
	{			
		m_conditions.RemoveKey( name );
		delete pCondition;
		pCondition = NULL;
	}		

	// look in all sub_cond blocks for the name
	CString temp_str;
	CConditionNode *pNode;

	POSITION nodePos, oldNodePos; 
	POSITION pos = m_conditions.GetStartPosition();

	CPtrArray nodesToDelete;

	while (pos != NULL)
	{
		m_conditions.GetNextAssoc(pos, temp_str, (void*&)pCondition );
		
		nodesToDelete.RemoveAll();

		if (pCondition !=NULL )
		{
			nodePos = pCondition->m_ctConditionTree.GetHeadPosition();
			while (nodePos != NULL)
			{
				oldNodePos = nodePos;
				pNode = pCondition->m_ctConditionTree.GetNext( nodePos );
				if (pNode != NULL)
				{
					if (pNode->m_sConditionName != name)						
						pNode->conditionDeleted( name );
					else
					{
						nodesToDelete.Add( pNode );
					}
				}

			}
		}
	}
}







BOOL CLogic_editorDoc::changeStateName( CString old, const CString &newName)
{
	if (old == newName)
		return TRUE;

	void *address;
	if ( m_states.Lookup( newName, address ) != NULL)
		return FALSE;

	if ( m_states.Lookup( old, address ) == NULL)
		return FALSE;

	CState *pState = static_cast<CState*>(address);
	
	m_states.RemoveKey( old );

	pState->m_sName = newName;
	m_states.SetAt(newName, pState );


	// look in all events to update the name
	CString temp_str;
	CEvent *pEvent;

	POSITION eventPos;
	POSITION statePos = m_states.GetStartPosition();
	while (statePos != NULL)
	{
		m_states.GetNextAssoc(statePos, temp_str, (void*&)pState );
		eventPos = pState->m_evEvents.GetHeadPosition();
		while( eventPos != NULL)
		{
			pEvent = pState->m_evEvents.GetNext( eventPos );
			if (pEvent->m_sStateChange == old)
				pEvent->m_sStateChange = newName;
		}
	}

	
	return TRUE;
}





void CLogic_editorDoc::deleteState( CString name)
{
	if ( name.IsEmpty() )
		return;

	CState *pState = NULL;
	if ( m_states.Lookup( name, (void*&)pState ) != NULL)
	{			
		m_states.RemoveKey( name );
		delete pState;
		pState = NULL;
	}

	// look in all events and delete those refering to the deleted state
	CString temp_str;
	CEvent *pEvent;

	POSITION eventPos;
	POSITION statePos = m_states.GetStartPosition();
	while (statePos != NULL)
	{
		m_states.GetNextAssoc(statePos, temp_str, (void*&)pState );
		eventPos = pState->m_evEvents.GetHeadPosition();
		while( eventPos != NULL)
		{
			pEvent = pState->m_evEvents.GetNext( eventPos );
			if (pEvent->m_sStateChange == name)
			{
				if ( pState->removeEvent( pEvent ))
					delete pEvent;
			}
		}
	}
}


//------------------------------------------------------
//	OnSaveDocument
//
//------------------------------------------------------
BOOL CLogic_editorDoc::OnSaveDocument( LPCTSTR fileName )
{
	// we don't save a stae machine with no state
	/*if( m_states.GetCount() == 0 )
	{
		return false;
	}*/
		
	POSITION pos;
	CString eltName;

	/*
	COFile fileOut;
	fileOut.open( fileName );
	COXml xmlfileOut;
	xmlfileOut.init(&fileOut);
	*/

/*	COFile xmlfileOut;
	xmlfileOut.open( fileName );//TEMP!!!
*/
			
	CLogicStateMachine logicStateMachine;
		
	/// convert and store the variables
	CString variable;
	for( pos = m_variables.GetHeadPosition(); pos != NULL; )
	{
		variable = m_variables.GetNext( pos );
		CLogicVariable logicVariable;
		logicVariable.setName( string((LPCSTR)variable) );
		logicStateMachine.addVariable( logicVariable );
	}

	// convert and store the counters
	for( pos = m_counters.GetStartPosition(); pos != NULL; )
	{
		CCounter * pCounter = new CCounter();
		// get counter
		m_counters.GetNextAssoc( pos, eltName, (void*&)pCounter );
		// set logic counter from counter
		CLogicCounter logicCounter;
		cCounterToCLogicCounter( *pCounter, logicCounter );
		// set the logic counter name
		logicCounter.setName( (LPCSTR)eltName );
		// add the logic counter
		logicStateMachine.addCounter( logicCounter );
	}

	// convert and store the conditions
	CCondition * pCondition = new CCondition();
	for( pos = m_conditions.GetStartPosition(); pos != NULL; )
	{
		// get condition
		m_conditions.GetNextAssoc( pos, eltName, (void*&)pCondition );
		// set logic condition from condition
		CLogicCondition logicCondition;
		cConditionToCLogicCondition( *pCondition, logicCondition );
		// set the logic condition name
		logicCondition.setName( (LPCSTR)eltName );
		// add the logic condition
		logicStateMachine.addCondition( logicCondition );
	}

	// convert and store the states
	CState * pState = new CState();
	for( pos = m_states.GetStartPosition(); pos != NULL; )
	{
		// get state
		m_states.GetNextAssoc( pos, eltName, (void*&)pState );
		// set the logic state from state
		CLogicState logicState;
		cStateToCLogicState( *pState, logicState );
		// set the logic state's name
		logicState.setName( (LPCSTR)eltName );
		// add the logic state
		logicStateMachine.addState( logicState );
	}

	// set the first state of the state machine
	pos = m_states.GetStartPosition();
	if(pos != NULL)
	{
		m_states.GetNextAssoc( pos, eltName, (void*&)pState );
		logicStateMachine.setCurrentState( string((LPCSTR)eltName) );
	}
	else
	{
		logicStateMachine.setCurrentState( string("") );
	}
	// set the name of the state machine
	logicStateMachine.setName( string(fileName) );


	// Check exceptions
	try
	{
		// File stream
		COFile file;

		// Open the file
		file.open (fileName);

		// Create the XML stream
		COXml output;

		// Init
		if (output.init (&file, "1.0"))
		{
			// Serial the class
			//logicStateMachine.serial( output );
			logicStateMachine.write(output.getDocument());

			// Flush the stream, write all the output file
			output.flush ();
		}

		// Close the file
		file.close ();
	}
 	catch (Exception &)
	{
	}

	// save the logic state machine
	///\todo trap see with stephanec --> logicStateMachine.serial( xmlfileOut );

	return true;

} // OnSaveDocument //



//------------------------------------------------------
//	load
//
//------------------------------------------------------
BOOL CLogic_editorDoc::load( LPCTSTR fileName )
{
	if (!CDocument::OnOpenDocument(fileName))
		return FALSE;

	
	/*
	CIFile fileIn;
	fileIn.open( fileName );
	CIXml xmlfileIn;
	xmlfileIn.init(fileIn);
	*/
//	CIFile xmlfileIn;
//	xmlfileIn.open( fileName ); //TEMP!!!

	// load the logic state machine
//	map<string,CLogicVariable>	variables;
//	map<string,CLogicCounter>	counters;
//	map<string,CLogicCondition>	conditions;
//	map<string,CLogicState>		states;

	CLogicStateMachine logicStateMachine;

	// Check exceptions
	try
	{
		// File stream
		CIFile file;

		// Open the file
		file.open (fileName);

		// Create the XML stream
		CIXml xmlfileIn;

		// Init
		if (xmlfileIn.init (file))
		{
			// Serial the class
			logicStateMachine.read (xmlfileIn.getRootNode());
/*			xmlfileIn.xmlPush("STATE_MACHINE");
			xmlfileIn.serialCont( variables );
			xmlfileIn.serialCont( counters );
			xmlfileIn.serialCont( conditions );
			xmlfileIn.serialCont( states );
			xmlfileIn.xmlPop();
*/		}

		// Close the file
		file.close ();
	}
 	catch (Exception &)
	{
	}
	
	// init the variables
	map<string,CLogicVariable>::const_iterator itVar;
	for( itVar = logicStateMachine.getVariables().begin(); itVar != logicStateMachine.getVariables().end(); ++itVar )
	{
		m_variables.AddTail( CString((*itVar).first.c_str()) );
	}

	// init the counters
	map<string,CLogicCounter>::const_iterator itCounter; 
	for( itCounter = logicStateMachine.getCounters().begin(); itCounter != logicStateMachine.getCounters().end(); ++itCounter )
	{
		CCounter * counter = new CCounter();
		cLogicCounterToCCounter( (*itCounter).second, *counter );
		m_counters[CString((*itCounter).first.c_str())] = counter;
	}

	// init the conditions
	map<string,CLogicCondition>::const_iterator itCondition;
	for( itCondition = logicStateMachine.getConditions().begin(); itCondition != logicStateMachine.getConditions().end(); ++itCondition )
	{
		CCondition * condition = new CCondition();
		cLogicConditionToCCondition( (*itCondition).second, *condition );
		m_conditions[CString((*itCondition).first.c_str())] = condition;
	}

	// init the states
	map<string,CLogicState>::const_iterator itState;
	for( itState = logicStateMachine.getStates().begin(); itState != logicStateMachine.getStates().end(); ++itState )
	{
		CState * state = new CState();
		cLogicStateToCState( (*itState).second, *state );
		m_states[CString((*itState).first.c_str())] = state;
	}

	InitCounterPage = TRUE;
	InitConditionPage = TRUE;
	InitStatePage = TRUE;

/*
	// get the child frame
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CChildFrame *pChild = (CChildFrame *) pFrame->MDIGetActive();

	// form view
	CEditorFormView *pFormView = static_cast<CEditorFormView *> ( pChild->m_wndSplitter.GetPane(0,1) );
	ASSERT_VALID(pFormView);	

	// property sheet
	CEditorPropertySheet * pPropertySheet = pFormView->m_pPropertySheet;

	// variable page
	pPropertySheet->m_variablePage.addVariable( pChild, this, "toto" );
	
*/	
//...

	return TRUE;

} // load //



//------------------------------------------------------
//	OnOpenDocument
//
//------------------------------------------------------
BOOL CLogic_editorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	return load(lpszPathName);

} // OnOpenDocument //


