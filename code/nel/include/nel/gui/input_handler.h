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


#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "nel/misc/events.h"
#include "nel/gui/event_descriptor.h"
#include "nel/gui/input_event_listener.h"

namespace NLGUI{
	
	/**
	  @brief The input event entry point of the GUI library.

	  Translates the NEL input events and forwards them.
	*/
	class CInputHandler
	{
	public:
		CInputHandler();
		~CInputHandler();

		bool handleEvent( const NLMISC::CEvent &evnt );
		bool handleSetFocusEvent( const NLMISC::CEvent &evnt );
		bool handleKeyboardEvent( const NLMISC::CEvent &evnt );
		bool handleMouseEvent( const NLMISC::CEvent &evnt );
		bool handleMouseMoveEvent( const NLMISC::CEvent &evnt );
		bool handleMouseButtonDownEvent( const NLMISC::CEvent &evnt );
		bool handleMouseButtonUpEvent( const NLMISC::CEvent &evnt );
		bool handleMouseDblClickEvent( const NLMISC::CEvent &evnt );
		bool handleMouseWheelEvent( const NLMISC::CEvent &evnt );

		void setListener( IInputEventListener* listener );

	private:
		IInputEventListener *listener;
	};

}

#endif

