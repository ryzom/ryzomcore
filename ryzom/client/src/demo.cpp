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



/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Client
#include "demo.h"
#include "entities.h"
#include "client_cfg.h"
#include "ingame_database_manager.h"
#include "user_entity.h"
#include "time_client.h"
#include "net_manager.h"
// MISC
#include "nel/misc/sheet_id.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;

////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Update_Demo )

///////////////
// FUNCTIONS //
///////////////
//-----------------------------------------------
// initDemo :
// Call a function for a demo to init.
//-----------------------------------------------
void initDemo()
{
	for(uint i = 0; i<ClientCfg.StartCommands.size()/7; ++i)
	{
		////////////
		// CREATE //
		////////////
		// Which slot to use.
		CLFECOMMON::TCLEntityId entitySlot = i+1;

		// Try to create the sheet with the parameter as a string.
		CSheetId sheetId;
		if(!sheetId.buildSheetId(ClientCfg.StartCommands[i*7]))
		{
			nlwarning("initDemo: cannot create the entity %d '%s'.", i*7, ClientCfg.StartCommands[i*7].c_str());
			continue;
		}

		// Remove the old entity.
		EntitiesMngr.remove(entitySlot, false);

		// Create the new entity.
		TNewEntityInfo emptyEntityInfo;
		emptyEntityInfo.reset();
		CEntityCL *entity = EntitiesMngr.create(entitySlot, sheetId.asInt(), emptyEntityInfo);
		if(entity)
		{
			double xTmp, yTmp, zTmp;
			fromString(ClientCfg.StartCommands[i*7+1], xTmp);
			fromString(ClientCfg.StartCommands[i*7+2], yTmp);
			fromString(ClientCfg.StartCommands[i*7+3], zTmp);

			// Compute the position (FIRST POS AFTER CREATE IS THE START POSITION).
			sint64 x = (sint64)(xTmp*1000.0);
			sint64 y = (sint64)(yTmp*1000.0);
			sint64 z = (sint64)(zTmp*1000.0);

			// Write the position in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(entitySlot) + ":P0", x);
			IngameDbMngr.setProp("Entities:E" + toString(entitySlot) + ":P1", y);
			IngameDbMngr.setProp("Entities:E" + toString(entitySlot) + ":P2", z);

			// Update the position.
			uint gameCycle = NetMngr.getCurrentClientTick();
			uint prop = 0;	// 0: Position ; 3: orientation ; 4: mode ; 5: behaviour ; 6: nameId ; 7: target ; 8: visual1 ; 9: visual2
			EntitiesMngr.updateVisualProperty(gameCycle, entitySlot, prop);

			// Set the direction
			float xTmp2, yTmp2, zTmp2;
			fromString(ClientCfg.StartCommands[i*7+4], xTmp2);
			fromString(ClientCfg.StartCommands[i*7+5], yTmp2);
			fromString(ClientCfg.StartCommands[i*7+6], zTmp2);
			entity->front(CVector(xTmp2, yTmp2, zTmp2));
			entity->dir(UserEntity->front());

/*
			//////////
			// MOVE //
			//////////
			gameCycle = NetMngr.getCurrentClientTick()+100;	// 10sec later.
			x = (sint64)((entity->pos().x+UserEntity->front().x*50.0)*1000.0);
			y = (sint64)((entity->pos().y+UserEntity->front().y*50.0)*1000.0);
			z = (sint64)((entity->pos().z+UserEntity->front().z*50.0)*1000.0);
			// Write the position in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(entitySlot) + ":P0", x);
			IngameDbMngr.setProp("Entities:E" + toString(entitySlot) + ":P1", y);
			IngameDbMngr.setProp("Entities:E" + toString(entitySlot) + ":P2", z);
			// Update the position.
			EntitiesMngr.updateVisualProperty(gameCycle, entitySlot, prop);
*/
		}
		else
			nldebug("initDemo: entity(%s) in slot %d cannot be created.", sheetId.toString().c_str(), entitySlot);
	}
}// initDemo //

//-----------------------------------------------
// updateDemo :
// Call a function for a demo to update.
//-----------------------------------------------
void updateDemo(double /* timeElapsed */)
{
	H_AUTO_USE ( RZ_Client_Update_Demo )
}// updateDemo //
