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

#ifndef NL_EVENT_EMITTER_H
#define NL_EVENT_EMITTER_H

#include "types_nl.h"
#include "event_server.h"
#include "smart_ptr.h"



namespace NLMISC {

/*===================================================================*/

class CEventServer;

/**
 * CEventEmitter
 * Send events to the event server
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class IEventEmitter	: public NLMISC::CRefCount
{
public:
	/// dtor
	virtual ~IEventEmitter() {}
	/**
	 * sends all events to server
	 * (should call CEventServer method postEvent() )
	 * \param server
	 */
	virtual void submitEvents(CEventServer & server, bool allWindows) = 0;
};


} // NLMISC


#endif // NL_EVENT_EMITTER_H

/* End of event_emitter.h */
