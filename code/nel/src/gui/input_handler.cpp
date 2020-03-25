// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "nel/gui/input_handler.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	CInputHandler::CInputHandler()
	{
		listener = NULL;
	}

	CInputHandler::~CInputHandler()
	{
		listener = NULL;
	}


	bool CInputHandler::handleEvent( const NLMISC::CEvent &evnt )
	{
		if( evnt == NLMISC::EventSetFocusId )
			return handleSetFocusEvent( evnt );
		else
		if( evnt == NLMISC::EventKeyDownId ||
			evnt == NLMISC::EventKeyUpId ||
			evnt == NLMISC::EventCharId ||
			evnt == NLMISC::EventStringId )
			return handleKeyboardEvent( evnt );
		else
		if( evnt == NLMISC::EventMouseMoveId ||
			evnt == NLMISC::EventMouseDownId ||
			evnt == NLMISC::EventMouseUpId ||
			evnt == NLMISC::EventMouseWheelId ||
			evnt == NLMISC::EventMouseDblClkId )
			return handleMouseEvent( evnt );


		return false;
	}

	bool CInputHandler::handleSetFocusEvent( const NLMISC::CEvent &evnt )
	{
		nlassert( evnt == NLMISC::EventSetFocusId );
		const NLMISC::CEventSetFocus *e = reinterpret_cast< const NLMISC::CEventSetFocus* >( &evnt );
		
		return listener->handleEvent( CEventDescriptorSetFocus( e->Get ) );
	}

	bool CInputHandler::handleKeyboardEvent( const NLMISC::CEvent &evnt )
	{
		bool ok = false;
		if( evnt == NLMISC::EventKeyDownId ||
			evnt == NLMISC::EventKeyUpId ||
			evnt == NLMISC::EventCharId ||
			evnt == NLMISC::EventStringId )
			ok = true;

		nlassert( ok );

		return listener->handleEvent( NLGUI::CEventDescriptorKey( reinterpret_cast< const NLMISC::CEventKey& >( evnt ) ) );
	}

	bool CInputHandler::handleMouseEvent( const NLMISC::CEvent &evnt )
	{
		if( evnt == NLMISC::EventMouseMoveId )
			return handleMouseMoveEvent( evnt );
		else
		if( evnt == NLMISC::EventMouseDownId )
			return handleMouseButtonDownEvent( evnt );
		else
		if( evnt == NLMISC::EventMouseUpId )
			return handleMouseButtonUpEvent( evnt );
		else
		if( evnt == NLMISC::EventMouseDblClkId )
			return handleMouseDblClickEvent( evnt );
		else
		if( evnt == NLMISC::EventMouseWheelId )
			return handleMouseWheelEvent( evnt );

		return false;
	}

	bool CInputHandler::handleMouseMoveEvent( const NLMISC::CEvent &evnt )
	{
		const NLMISC::CEventMouseMove &mouseMoveEvent = static_cast< const NLMISC::CEventMouseMove& >( evnt );
		
		CEventDescriptorMouse eventDesc;
		float x = mouseMoveEvent.X;
		float y = mouseMoveEvent.Y;

		// These bloody hacks here are used so that we can send the x, and y float coordinates
		// from the NEL mouse move event, to the GUI event listener, without having to change
		// CEventDescriptorMouse or without having to couple with the consumer class
		eventDesc.setX( *reinterpret_cast< sint32* >( &x ) );
		eventDesc.setY( *reinterpret_cast< sint32* >( &y ) );

		eventDesc.setEventTypeExtended( CEventDescriptorMouse::mousemove );

		return listener->handleEvent( eventDesc );
	}

	bool CInputHandler::handleMouseButtonDownEvent( const NLMISC::CEvent &evnt )
	{
		nlassert( evnt == NLMISC::EventMouseDownId );

		CEventDescriptorMouse eventDesc;
		
		const NLMISC::CEventMouseDown *mouseDownEvent = static_cast< const NLMISC::CEventMouseDown* >( &evnt );
		
		if( mouseDownEvent->Button & NLMISC::leftButton )
		{
			eventDesc.setEventTypeExtended( CEventDescriptorMouse::mouseleftdown );
			return listener->handleEvent( eventDesc );
		}
		
		if(mouseDownEvent->Button & NLMISC::rightButton)
		{
			eventDesc.setEventTypeExtended( CEventDescriptorMouse::mouserightdown );
			return listener->handleEvent( eventDesc );
		}

		return false;
	}

	bool CInputHandler::handleMouseButtonUpEvent( const NLMISC::CEvent &evnt )
	{
		nlassert( evnt == NLMISC::EventMouseUpId );

		CEventDescriptorMouse eventDesc;

		const NLMISC::CEventMouseUp *mouseUpEvent = static_cast< const NLMISC::CEventMouseUp* >( &evnt );
		
		if( mouseUpEvent->Button & NLMISC::leftButton )
		{
			eventDesc.setEventTypeExtended( CEventDescriptorMouse::mouseleftup );
			return listener->handleEvent( eventDesc );
		}
		
		if( mouseUpEvent->Button & NLMISC::rightButton )
		{
			eventDesc.setEventTypeExtended( CEventDescriptorMouse::mouserightup );
			return listener->handleEvent( eventDesc );
		}

		return false;
	}

	bool CInputHandler::handleMouseDblClickEvent( const NLMISC::CEvent &evnt )
	{
		nlassert( evnt == NLMISC::EventMouseDblClkId );

		CEventDescriptorMouse eventDesc;

		const NLMISC::CEventMouseDblClk *dblClickEvent = static_cast< const NLMISC::CEventMouseDblClk* >( &evnt );
		
		if( dblClickEvent->Button & NLMISC::leftButton )
		{
			eventDesc.setEventTypeExtended( CEventDescriptorMouse::mouseleftdblclk );
			return listener->handleEvent (eventDesc);
		}
		
		if( dblClickEvent->Button & NLMISC::rightButton )
		{
			eventDesc.setEventTypeExtended(NLGUI::CEventDescriptorMouse::mouserightdblclk);
			return listener->handleEvent (eventDesc);
		}
		
		return false;
	}

	bool CInputHandler::handleMouseWheelEvent( const NLMISC::CEvent &evnt )
	{
		nlassert( evnt == NLMISC::EventMouseWheelId );

		CEventDescriptorMouse eventDesc;
		sint32 mouseWheel = 0;
		
		const NLMISC::CEventMouseWheel *wheelEvent = static_cast< const NLMISC::CEventMouseWheel* >( &evnt );
		
		if( wheelEvent->Direction )
			mouseWheel = 1;
		else
			mouseWheel = -1;
		
		if( mouseWheel != 0 )
		{
			eventDesc.setEventTypeExtended( CEventDescriptorMouse::mousewheel );
			eventDesc.setWheel( mouseWheel );
			return listener->handleEvent( eventDesc );
		}

		return false;
	}

	void CInputHandler::setListener( IInputEventListener *listener )
	{
		this->listener = listener;
	}
}
