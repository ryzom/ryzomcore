// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "stdlogic.h"
#include "nel/logic/logic_state.h"
#include "nel/logic/logic_state_machine.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace NLLOGIC
{

//---------------------------------------------------
//	CLogicState
//
//---------------------------------------------------
CLogicState::CLogicState()
{
	_StateName = "no_state";
	_LogicStateMachine = 0;

} // CLogicState //


//---------------------------------------------------
//	setLogicStateMachine
//
//---------------------------------------------------
void CLogicState::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CLogicCondition::setLogicStateMachine> The state machine is null");
	}
	else
	{
		// init the logic state machine for this state
		_LogicStateMachine = logicStateMachine;

		// init the logic state machine in each event
		vector<CLogicEvent>::iterator itEvent;
		for( itEvent = _Events.begin(); itEvent != _Events.end(); ++itEvent )
		{
			(*itEvent).setLogicStateMachine( logicStateMachine );
		}
	}

} // setLogicStateMachine //



//---------------------------------------------------
// addEvent :
//
//---------------------------------------------------
void CLogicState::addEvent( CLogicEvent event )
{
	event.setLogicStateMachine( _LogicStateMachine );
	_Events.push_back( event );

} // addEvent //



//---------------------------------------------------
// addSIdMap :
//
// looks in all the messages of the state if the
// destination names can be associated with a sid.
//---------------------------------------------------
void CLogicState::addSIdMap( const TSIdMap& sIdMap )
{
	vector<CLogicEventMessage>::iterator itMsg;

	/// entry messages
	for( itMsg = _EntryMessages.begin(); itMsg != _EntryMessages.end(); ++itMsg )
	{
		TSIdMap::const_iterator itId = sIdMap.find( (*itMsg).Destination );
		// if message destination exists in the map we associate the sid with the message
		if( itId != sIdMap.end() )
		{
			(*itMsg).DestinationId = (*itId).second;
		}
	}
	/// send the entry messages that can be sent
	trySendEntryMessages();


	// event messages
	vector<CLogicEvent>::iterator itEvt;
	for( itEvt = _Events.begin(); itEvt != _Events.end(); ++itEvt )
	{
		string dest = (*itEvt).EventAction.EventMessage.Destination;
		TSIdMap::const_iterator itId = sIdMap.find( dest );
		// if message destination exists in the map we associate the sid with the message
		if( itId != sIdMap.end() )
		{
			(*itEvt).EventAction.EventMessage.DestinationId = (*itId).second;
		}
	}
	/// send the event messages that can be sent
	trySendEventMessages();


	/// exit messages
	for( itMsg = _ExitMessages.begin(); itMsg != _ExitMessages.end(); ++itMsg )
	{
		TSIdMap::const_iterator itId = sIdMap.find( (*itMsg).Destination );
		// if message destination exists in the map we associate the sid with the message
		if( itId != sIdMap.end() )
		{
			(*itMsg).DestinationId = (*itId).second;
		}
	}

} // addSIdMap //


//---------------------------------------------------
// processLogic :
//
//---------------------------------------------------
void CLogicState::processLogic()
{
	// test all conditions managed by this state
	vector<CLogicEvent>::iterator itEvent;
	for( itEvent = _Events.begin(); itEvent != _Events.end(); ++itEvent )
	{
		if( (*itEvent).testCondition() )
		{
			//nlinfo("The condition %s is valid",(*itEvent).ConditionName.c_str());
			if( (*itEvent).EventAction.IsStateChange )
			{
				_LogicStateMachine->setCurrentState( (*itEvent).EventAction.StateChange );
			}
			else
			{
				// this message will be sent as soon as the dest id will be given
				(*itEvent).EventAction.enableSendMessage();

				/// send the event messages that must and can be sent
				trySendEventMessages();
			}
		}
		else
		{
			// reset message send status here to be able to send messages several times in the logic state
			// --> this has to be done if we want messages to be sent every time the condition becomes verified
		}
	}

} // processLogic //


//---------------------------------------------------
// enterState :
//
//---------------------------------------------------
void CLogicState::enterState()
{
	/// send the entry messages that can be sent
	trySendEntryMessages();

} // enterState //


//---------------------------------------------------
// exitState :
//
//---------------------------------------------------
void CLogicState::exitState()
{
	vector<CLogicEventMessage>::iterator itMsg;
	for( itMsg = _ExitMessages.begin(); itMsg != _ExitMessages.end(); ++itMsg )
	{
		if( (*itMsg).DestinationId != CEntityId() )
		{
			CMessage msgOut( (*itMsg).MessageId );
			msgOut.serial( (*itMsg).Arguments );

			_MessagesToSend.insert( make_pair((*itMsg).DestinationId,msgOut) );
		}
	}

	// reset the entry messages send status
	for( itMsg = _EntryMessages.begin(); itMsg != _EntryMessages.end(); ++itMsg )
	{
		(*itMsg).ToSend = false;
		(*itMsg).Sent = false;
	}

	// reset all events
	vector<CLogicEvent>::iterator itEvent;
	for( itEvent = _Events.begin(); itEvent != _Events.end(); ++itEvent )
	{
		(*itEvent).reset();
	}

	// reset the exit messages send status
	for( itMsg = _ExitMessages.begin(); itMsg != _ExitMessages.end(); ++itMsg )
	{
		(*itMsg).ToSend = false;
		(*itMsg).Sent = false;
	}

} // exitState //



//---------------------------------------------------
// trySendEntryMessages :
//
//---------------------------------------------------
void CLogicState::trySendEntryMessages()
{
	/// send the entry messages that can be sent
	vector<CLogicEventMessage>::iterator itMsg;
	for( itMsg = _EntryMessages.begin(); itMsg != _EntryMessages.end(); ++itMsg )
	{
		if( !(*itMsg).Sent && (*itMsg).DestinationId.getType() != 0xfe )
		{
			CMessage msgOut( (*itMsg).MessageId );
			msgOut.serial( (*itMsg).Arguments );

			_MessagesToSend.insert( make_pair((*itMsg).DestinationId,msgOut) );

			(*itMsg).ToSend = false;
			(*itMsg).Sent = true;
		}
	}

} // trySendEntryMessages //



//---------------------------------------------------
// trySendEventMessages :
//
//---------------------------------------------------
void CLogicState::trySendEventMessages()
{
	// test all conditions managed by this state
	vector<CLogicEvent>::iterator itEvent;
	for( itEvent = _Events.begin(); itEvent != _Events.end(); ++itEvent )
	{
		if( (*itEvent).EventAction.EventMessage.ToSend == true )
		{
			if( (*itEvent).EventAction.EventMessage.Sent == false )
			{
				if( (*itEvent).EventAction.EventMessage.DestinationId.getType() != 0xfe )
				{
					CMessage msgOut( (*itEvent).EventAction.EventMessage.MessageId );
					msgOut.serial( (*itEvent).EventAction.EventMessage.Arguments );

					_MessagesToSend.insert( make_pair((*itEvent).EventAction.EventMessage.DestinationId,msgOut) );

					(*itEvent).EventAction.EventMessage.ToSend = false;
					(*itEvent).EventAction.EventMessage.Sent = true;
				}
			}
		}
	}

} // trySendEventMessages //


//---------------------------------------------------
// getMessagesToSend :
//
//---------------------------------------------------
void CLogicState::getMessagesToSend( multimap<CEntityId,CMessage>& msgs )
{
	multimap<CEntityId,CMessage>::iterator itMsg;
	for( itMsg = _MessagesToSend.begin(); itMsg != _MessagesToSend.end(); ++itMsg )
	{
		msgs.insert( *itMsg );
	}

	// erase all the messages
	_MessagesToSend.clear();

} // getMessagesToSend //



//---------------------------------------------------
// fillVarMap :
//
//---------------------------------------------------
void CLogicState::fillVarMap( multimap<CEntityId,string >& stateMachineVariables )
{
	// events
	vector<CLogicEvent>::iterator itEvt;
	for( itEvt = _Events.begin(); itEvt != _Events.end(); ++itEvt )
	{
		// get the condition used in the event
		CLogicCondition condition;
		if( _LogicStateMachine->getCondition((*itEvt).ConditionName,condition) )
		{
			// get vars used in the conditions
			set<string> condVars;
			condition.fillVarSet( condVars );

			// add var with related service
			set<string>::iterator itCV;
			for( itCV = condVars.begin(); itCV != condVars.end(); ++itCV )
			{
				stateMachineVariables.insert( make_pair((*itEvt).EventAction.EventMessage.DestinationId,*itCV) );
			}
		}
	}

} // fillVarMap //



//---------------------------------------------------
// serial :
//
//---------------------------------------------------
/*void CLogicState::serial( IStream &f )
{
	f.xmlPush( "STATE");

	f.serial( _StateName );
	f.serialCont( _EntryMessages );
	f.serialCont( _ExitMessages );
	f.serialCont( _Events );

	f.xmlPop();

} // serial //*/

void CLogicState::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"STATE", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Name", (const xmlChar*)_StateName.c_str());

	uint i;
	for (i = 0; i < _EntryMessages.size(); i++)
	{
		_EntryMessages[i].write(elmPtr, "ENTRY_");
	}
	for (i = 0; i < _ExitMessages.size(); i++)
	{
		_ExitMessages[i].write(elmPtr, "EXIT_");
	}
	for (i = 0; i < _Events.size(); i++)
	{
		_Events[i].write(elmPtr);
	}
}

void CLogicState::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "STATE");

	_StateName = getXMLProp (node, "Name");

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "ENTRY_EVENT_MESSAGE");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "ENTRY_EVENT_MESSAGE");
		while (i<nb)
		{
			CLogicEventMessage v;
			v.read(parent);
			_EntryMessages.push_back(v);

			// Next parent
			parent = CIXml::getNextChildNode (parent, "ENTRY_EVENT_MESSAGE");
			i++;
		}
	}

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "EXIT_EVENT_MESSAGE");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "EXIT_EVENT_MESSAGE");
		while (i<nb)
		{
			CLogicEventMessage v;
			v.read(parent);
			_ExitMessages.push_back(v);

			// Next parent
			parent = CIXml::getNextChildNode (parent, "EXIT_EVENT_MESSAGE");
			i++;
		}
	}

	{
		// Count the parent
		uint nb = CIXml::countChildren (node, "EVENT");
		uint i = 0;
		xmlNodePtr parent = CIXml::getFirstChildNode (node, "EVENT");
		while (i<nb)
		{
			CLogicEvent v;
			v.read(parent);
			_Events.push_back(v);

			// Next parent
			parent = CIXml::getNextChildNode (parent, "EVENT");
			i++;
		}
	}

}


} // NLLOGIC

