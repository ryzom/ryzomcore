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

#ifndef INPUTEVENTLISTENER_H
#define INPUTEVENTLISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/gui/event_descriptor.h"

namespace NLGUI
{
	/**
	 @brief Interface for accepting GUI input events.
	 */
	class IInputEventListener
	{
	public:
		virtual ~IInputEventListener(){}
		virtual bool handleEvent( const CEventDescriptor &eventDesc ) = 0;
	};
}

#endif
