// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef RY_SABRINA_AI_INTERFACE_H
#define RY_SABRINA_AI_INTERFACE_H

// std
#include <vector>
// misc
#include "nel/misc/types_nl.h"
// game share
#include "game_share/ai_event.h"
#include "game_share/ai_event_report.h"

/**
 * The Sabrina AI Service Interface singleton
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSabrinaAIInterface
{	
public:
	// init method called at service init by CSabrinaPhraseManager::init()
	static void init();

	// update called each tick by CSabrinaPhraseManager::update() - dispatches events to the AI services
	static void flushEvents();

	// release method called at service release	by CSabrinaPhraseManager::release()
	static void release();

	/**
	 * register a service to the event broadcast for AI
	 * \param serviceName name of the registered service
	 */
	static void registerAIService( uint8 serviceId );

	/**
	 * unregister a service to the event broadcast
	 * \param serviceName name of the service to remove
	 */
	static void unregisterAIService( uint8 serviceId );

	/**
	 * add an AI event report for the current tick
	 * \param report the AI event report to add
	 */
	static void addAiEventReport( const CAiEventReport &report );

	/**
	 * add an AI event for the current tick
	 * \param report the event to add
	 */
	static void addAIEvent( const IAIEvent *event );

private:
	// this is a singleton so prohibit construction
	CSabrinaAIInterface();

	/// list of registered services for Event Broadcast for AI
	static std::vector<uint8>			_RegisteredServices;

	/// the list of ai events
	typedef std::list<IAIEvent*>		TAIEventList;
	static TAIEventList					_AIEvents;

	/// the ai event reports
	static std::vector<CAiEventReport>	_AIEventReports;
};


#endif 
