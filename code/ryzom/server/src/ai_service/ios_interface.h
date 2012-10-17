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




#ifndef RYAI_IOS_INTERFACE_H
#define RYAI_IOS_INTERFACE_H

// Include
#include "nel/net/unified_network.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"

//#include "ai_entity_id.h"


// the class
class CIOSInterface
{
public:
	// classic init(), update() and release()
	static void init();
	static void update();
	static void release();

	// add bots to the world, remove them from the world or move them
	static void addEntity(CAIEntityId id, NLMISC::CSheetId sheet);
	static void removeEntity(CAIEntityId id);

private:
	// messages that are filled up by the above static methods
	// messages are dispatched when a threshold size is reached
	// the messages are also flushed by update()
	//static NLNET::CMessage _addEntityMsg;
	//static NLNET::CMessage _removeEntityMsg;
};

#endif
