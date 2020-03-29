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



// Include
#include "nel/net/unified_network.h"
#include "game_share/tick_event_handler.h"
#include "ios_interface.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


// classic init(), update() and release()
void CIOSInterface::init()
{
}
void CIOSInterface::update()
{
	// flush output messages
}
void CIOSInterface::release()
{
}

// add entities to the world, remove them from the world or move them in the world
void CIOSInterface::addEntity(CAIEntityId id, NLMISC::CSheetId sheet)
{
	nlstop;
	
	NLMISC::CEntityId eid=id.toEntityId();

	// send creature to IOS
	CMessage msgOPS("ADD_CREATURE");
	msgOPS.serial( eid );
	msgOPS.serial( sheet );
	sendMessageViaMirror("IOS", msgOPS);

}


void CIOSInterface::removeEntity(CAIEntityId id)
{
	NLMISC::CEntityId eid=id.toEntityId();

	// remove the actor from the IOS
	CMessage msgOPS("REMOVE_CREATURE");
	msgOPS.serial( eid );
	sendMessageViaMirror( "IOS", msgOPS );
}

