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
#include "nel/logic/logic_event.h"

#include "nel/logic/logic_state_machine.h"

#include "nel/net/service.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

namespace NLLOGIC
{

//----------------------------------- MESSAGE ----------------------------------


//-------------------------------------------------
// serial
//
//-------------------------------------------------
/*void CLogicEventMessage::serial( IStream &f )
{
	f.xmlPush("EVENT_MESSAGE");

	f.serial( Destination );
	f.serial( DestinationId );
	f.serial( MessageId );
	f.serial( Arguments );

	f.xmlPop();

} // serial //*/

void CLogicEventMessage::write (xmlNodePtr node, const char *subName) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)string(string(subName)+string("EVENT_MESSAGE")).c_str(), NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"Destination", (const xmlChar*)Destination.c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"DestinationId", (const xmlChar*)toString(DestinationId).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"MessageId", (const xmlChar*)toString(MessageId).c_str());
	xmlSetProp (elmPtr, (const xmlChar*)"Arguments", (const xmlChar*)toString(Arguments).c_str());
}

void CLogicEventMessage::read (xmlNodePtr node, const char *subName)
{
	xmlCheckNodeName (node, string(string(subName)+string("EVENT_MESSAGE")).c_str());

	Destination = getXMLProp (node, "Destination");
	DestinationId = atoiInt64(getXMLProp (node, "DestinationId").c_str());
	MessageId = getXMLProp (node, "MessageId");
	Arguments = getXMLProp (node, "Arguments");
}





//----------------------------------- ACTION ----------------------------------


//-------------------------------------------------
// enableSendMessage
//
//-------------------------------------------------
void CLogicEventAction::enableSendMessage()
{
	EventMessage.ToSend = true;

} // enableSendMessage //


//-------------------------------------------------
// serial
//
//-------------------------------------------------
/*void CLogicEventAction::serial( IStream &f )
{
	f.xmlPush("EVENT_ACTION");

	f.serial( IsStateChange );
	if( IsStateChange )
	{
		f.serial( StateChange );
	}
	else
	{
		f.serial( EventMessage );
	}

	f.xmlPop();

} // serial //*/

void CLogicEventAction::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"EVENT_ACTION", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"IsStateChange", (const xmlChar*)toString(IsStateChange).c_str());
	if (IsStateChange)
	{
		xmlSetProp (elmPtr, (const xmlChar*)"StateChange", (const xmlChar*)StateChange.c_str());
	}
	else
	{
		EventMessage.write(elmPtr);
	}
}

void CLogicEventAction::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "EVENT_ACTION");

	NLMISC::fromString(getXMLProp(node, "IsStateChange"), IsStateChange);
	if (IsStateChange)
	{
		StateChange = getXMLProp (node, "StateChange");
	}
	else
	{
		EventMessage.read(node);
	}
}








//----------------------------------- EVENT ----------------------------------



//-------------------------------------------------
// reset
//
//-------------------------------------------------
void CLogicEvent::reset()
{
	EventAction.EventMessage.Sent = false;
	EventAction.EventMessage.ToSend = false;

} // reset //



//-------------------------------------------------
// setLogicStateMachine
//
//-------------------------------------------------
void CLogicEvent::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CLogicEvent::setLogicStateMachine> The state machine is null");
	}
	else
	{
		// init the logic state machine for this event
		_LogicStateMachine = logicStateMachine;
	}

} // setLogicStateMachine //



//-------------------------------------------------
// testCondition
//
//-------------------------------------------------
bool CLogicEvent::testCondition()
{
	if( _LogicStateMachine )
	{
		if( ConditionName != "no_condition" )
		{
			CLogicCondition cond;
			if( _LogicStateMachine->getCondition( ConditionName, cond ) )
			{
				return cond.testLogic();
			}
			else
			{
				nlwarning("(LOGIC)<CLogicEvent::testCondition> Condition %s not found in the state machine",ConditionName.c_str());
				return false;
			}
		}
		else
		{
			nlwarning("(LOGIC)<CLogicEvent::testCondition> Condition undefined");
			return false;
		}
	}
	else
	{
		nlwarning("(LOGIC)<CLogicEvent::testCondition> The state machine managing this event is Null");
	}

	return false;

} // testCondition //


//-------------------------------------------------
// serial
//
//-------------------------------------------------
/*void CLogicEvent::serial( IStream &f )
{
	f.xmlPush("EVENT");

	f.serial( ConditionName );
	f.serial( EventAction );

	f.xmlPop();

} // serial //*/

void CLogicEvent::write (xmlNodePtr node) const
{
	xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"EVENT", NULL);
	xmlSetProp (elmPtr, (const xmlChar*)"ConditionName", (const xmlChar*)ConditionName.c_str());
	EventAction.write(elmPtr);
}

void CLogicEvent::read (xmlNodePtr node)
{
	xmlCheckNodeName (node, "EVENT");

	ConditionName = getXMLProp (node, "ConditionName");
	EventAction.read(node);
}

} // NLLOGIC


