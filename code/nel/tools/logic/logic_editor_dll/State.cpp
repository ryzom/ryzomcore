// State.cpp: implementation of the CState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "State.h"

#include "nel/logic/logic_state.h"

#include <string>

using namespace std;
using namespace NLLOGIC;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const CString & CEvent::getEventAsString() const
{
	eventString.Empty();

	eventString = m_sConditionName;

	if (m_bActionIsMessage)
	{
		eventString += " MSG ";
		eventString += this->m_sMessageID;

	}
	else
	{
		eventString += " State Chg to ";
		eventString += m_sStateChange;
	}

	return eventString;
}






bool operator==( const CEvent &ev1, const CEvent &ev2)
{
	return ( (ev1.m_sConditionName == ev2.m_sConditionName) 
			&& (ev1.m_bActionIsMessage == ev2.m_bActionIsMessage )
			&& (ev1.m_sArguments == ev2.m_sArguments )
			&& (ev1.m_sMessageDestination == ev2.m_sMessageDestination)
			&& (ev1.m_sMessageID == ev2.m_sMessageID)
			&& (ev1.m_sStateChange == ev2.m_sStateChange)
		   );
}



//-----------------------------------------------
//	cEventToCLogicEvent
//
//-----------------------------------------------
void cEventToCLogicEvent( CEvent& event, CLogicEvent& logicEvent )
{
	/// condition name
	logicEvent.ConditionName = string( (LPCSTR)event.m_sConditionName );
	
	/// event action
	logicEvent.EventAction.IsStateChange = !event.m_bActionIsMessage;

	if( logicEvent.EventAction.IsStateChange )
	{
		/// state name for state change
		logicEvent.EventAction.StateChange = string( (LPCSTR)event.m_sStateChange );
	}
	else
	{
		/// message destination
		logicEvent.EventAction.EventMessage.Destination = string( (LPCSTR)event.m_sMessageDestination );
		
		/// message id
		logicEvent.EventAction.EventMessage.MessageId = "LOGIC"; //string( (LPCSTR)event.m_sMessageID ); //TEMP!!!

		/// message arguments
		logicEvent.EventAction.EventMessage.Arguments = string( (LPCSTR)event.m_sArguments );
	}

} // cEventToCLogicEvent //





//////////////////////////////////////////////////////////////////////
//  CState Construction/Destruction
//////////////////////////////////////////////////////////////////////


CState::CState(const CString &name )
: m_sName( name )
{
}



CState::CState( const CState &state )
{
	this->m_sName = state.m_sName;

	// copy events (allocate new objects)
	CEvent	*pEvent;
	CEvent	*newEvent;
	POSITION pos = state.m_evEvents.GetHeadPosition();
	while (pos != NULL)
	{
		pEvent = state.m_evEvents.GetNext( pos );
		if ( pEvent != NULL)
		{
			newEvent = new CEvent( *pEvent );
			this->m_evEvents.AddTail( newEvent );
		}
	}
}




CState::~CState()
{
	// delete all events
	POSITION pos = m_evEvents.GetHeadPosition();
	while (pos != NULL)
	{
		CEvent *ev = m_evEvents.GetNext( pos );
		if ( ev != NULL)
		{
			delete ev;
			ev = NULL;
		}
	}
}


BOOL CState::removeEvent( CEvent *event)
{
	// go through all the events and remove the first event equal to the one in param
	POSITION pos = m_evEvents.GetHeadPosition();
	POSITION oldpos;
	
	while (pos != NULL)
	{
		oldpos = pos;
		CEvent *ev = m_evEvents.GetNext( pos );
		if (*ev == *event)
		{
			m_evEvents.RemoveAt(oldpos);
			return TRUE;
		}
	}

	return FALSE;
}


//-----------------------------------------------
//	cStateToCLogicState
//
//-----------------------------------------------
void cStateToCLogicState( CState& state, CLogicState& logicState )
{
	/// state name
	logicState.setName( string( (LPCSTR)state.m_sName ) );

	POSITION pos;
	for( pos = state.m_evEvents.GetHeadPosition(); pos != NULL; )
	{
		CEvent * pEvent = state.m_evEvents.GetNext( pos );
		
		CLogicEvent logicEvent;
		cEventToCLogicEvent( *pEvent, logicEvent );
		
		logicState.addEvent( logicEvent );
	}

} // cStateToCLogicState //



//-----------------------------------------------
//	cLogicStateToCState
//
//-----------------------------------------------
void cLogicStateToCState( const CLogicState& logicState, CState& state )
{
	state.m_sName = CString( logicState._StateName.c_str() );
	
	vector<CLogicEventMessage>::const_iterator itMsg;
	for( itMsg = logicState._EntryMessages.begin(); itMsg != logicState._EntryMessages.end(); ++itMsg )
	{
		//TODO
	}
	for( itMsg = logicState._ExitMessages.begin(); itMsg != logicState._ExitMessages.end(); ++itMsg )
	{
		//TODO
	}

	
	vector<CLogicEvent>::const_iterator itEvt;
	for( itEvt = logicState._Events.begin(); itEvt != logicState._Events.end(); ++itEvt )
	{
		CEvent * event = new CEvent();
		event->m_sConditionName = CString((*itEvt).ConditionName.c_str());
		
		event->m_bActionIsMessage = !(*itEvt).EventAction.IsStateChange;
		
		event->m_sMessageDestination = CString( (*itEvt).EventAction.EventMessage.Destination.c_str() );
		event->m_sMessageID = CString( (*itEvt).EventAction.EventMessage.MessageId.c_str() );
		event->m_sArguments = CString( (*itEvt).EventAction.EventMessage.Arguments.c_str() );
	
		event->m_sStateChange = CString( (*itEvt).EventAction.StateChange.c_str() );
		
		state.addEvent( event );
	}


} // cLogicStateToCState //
