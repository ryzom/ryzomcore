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

#ifndef LOGIC_STATE_H
#define LOGIC_STATE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"

#include "nel/net/service.h"

//#include "game_share/sid.h"

#include "logic_event.h"

namespace NLLOGIC
{

class CLogicStateMachine;

/// map destination names to destination sid
typedef std::map<std::string, NLMISC::CEntityId> TSIdMap;

/**
 * CLogicState
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CLogicState
{
public:

	/// state name
	std::string _StateName;

	/// entry messages
	std::vector<CLogicEventMessage> _EntryMessages;

	/// exit messages
	std::vector<CLogicEventMessage> _ExitMessages;

	/// logic
	std::vector<CLogicEvent> _Events;

	/// state machine containing this state
	CLogicStateMachine * _LogicStateMachine;

	/// messages to send by the service
	std::multimap<NLMISC::CEntityId, NLNET::CMessage> _MessagesToSend;

public:

	/**
	 *	Default constructor
	 */
	CLogicState();

	/**
	 * Set the state machine which contains this state
	 *
	 * \param logicStateMachine is the state machine containing this block
	 */
	void setLogicStateMachine( CLogicStateMachine * logicStateMachine );

	/**
	 * set the state name
	 *
	 * \param name is the new state's name
	 */
	void setName( std::string name ) { _StateName = name; }

	/**
	 * get the state name
	 *
	 * \return the state's name
	 */
	std::string getName() { return _StateName; }

	/**
	 * Add an event
	 *
	 * \param event is the event to add
	 */
	void addEvent( CLogicEvent event );

	/**
	 * Associate message destination name with sid
	 *
	 * \param sIdMap is the map associating destination name with a destination id
	 */
	void addSIdMap( const TSIdMap& sIdMap );

	/**
	 * Test the conditions of this state
	 */
	void processLogic();

	/**
	 *	Get the messages to send
	 *
	  * \param msgs is the map associating all the message to send with their destination id
	 */
	void getMessagesToSend( std::multimap<NLMISC::CEntityId,NLNET::CMessage>& msgs );

	/**
	 * send the entry messages
	 */
	void enterState();

	/**
	 * send the exit messages
	 */
	void exitState();

	/**
	 *	Try to send the entry messages
	 */
	void trySendEntryMessages();

	/**
	 *	Try to send the event messages
	 */
	void trySendEventMessages();

	/**
	 * Fill a map associating all the referenced var in the state with the id of service managing them
	 * (debug purpose)
	 */
	void fillVarMap( std::multimap<NLMISC::CEntityId,std::string >& stateMachineVariables );

	/**
	 * serial
	 */
	//void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	void write (xmlNodePtr node) const;
	void read (xmlNodePtr node);
};


} // NLLOGIC

#endif //LOGIC_STATE



