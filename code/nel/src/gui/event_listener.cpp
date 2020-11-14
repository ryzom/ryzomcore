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


#include "stdpch.h"
#include "nel/misc/event_server.h"
#include "nel/gui/event_listener.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	CEventListener::CEventListener()
	{
		inputHandler.setListener( CWidgetManager::getInstance() );
	}

	CEventListener::~CEventListener()
	{
	}

	void CEventListener::addToServer( NLMISC::CEventServer *server )
	{
		server->addListener( NLMISC::EventMouseMoveId,	 this );
		server->addListener( NLMISC::EventMouseDownId,	 this );
		server->addListener( NLMISC::EventMouseUpId,	 this );
		server->addListener( NLMISC::EventMouseWheelId,	 this );
		server->addListener( NLMISC::EventMouseDblClkId, this );

		eventServer = server;
	}

	void CEventListener::removeFromServer()
	{
		eventServer->removeListener( NLMISC::EventMouseMoveId,   this );
		eventServer->removeListener( NLMISC::EventMouseDownId,   this );
		eventServer->removeListener( NLMISC::EventMouseUpId,     this );
		eventServer->removeListener( NLMISC::EventMouseWheelId,  this );
		eventServer->removeListener( NLMISC::EventMouseDblClkId, this );

		eventServer = NULL;
	}

	void CEventListener::operator ()(const NLMISC::CEvent &evnt)
	{
		inputHandler.handleEvent( evnt );
	}
}

