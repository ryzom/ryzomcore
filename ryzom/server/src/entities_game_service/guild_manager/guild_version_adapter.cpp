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
#include "guild_version_adapter.h"
#include "player_manager/character_version_adapter.h"
#include "guild_manager/guild.h"
#include "player_manager/character.h"


///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

/////////////
// GLOBALS 
/////////////
CGuildVersionAdapter	*CGuildVersionAdapter::_Instance = NULL;


//---------------------------------------------------
// currentVersionNumber:
//
//---------------------------------------------------
uint32 CGuildVersionAdapter::currentVersionNumber() const
{
	////////////////////////////////////
	// VERSION History
	// 0 : 
	// 1 : (25/11/2004) saves must be updated because events mps dont have the right sheet... So we have to replace them
	// 2 : (03/12/2004) creation dates where not saved...
	// 3 : (01/11/2004) guild allegiance of previous existing guild setted to undefined...
	// 4 : (07/03/2006) give full hp to all tools
	////////////////////////////////////
	return 4;
}


//---------------------------------------------------
void CGuildVersionAdapter::adaptGuildFromVersion( CGuild & guild ) const
{
	// Do NOT break between case labels
	switch (guild.getVersion())
	{
	case 0: break; //adaptToVersion1(guild); // All guild are adapted to version 3 with the lasts EGS restart, the bug about version number saved
	case 1: break; //adaptToVersion2(guild); // imply adapter can't be used for the next egs reboot, all guild are set to version 3.
	case 2: break; //adaptToVersion3(guild); // when we change guild save and have currentVersion = 4, we can re-active adapter (we can re-active
	case 3: adaptToVersion4(guild);
		
	default:; // the adapter at the next patch in fact...
	}

	// store the guild version
	guild.setVersion(currentVersionNumber());

}

//---------------------------------------------------
void CGuildVersionAdapter::adaptToVersion1(CGuild & guild) const
{
	if ( guild.getInventory() != NULL )
	{
		CCharacterVersionAdapter::getInstance()->updateInventoryToVersion6( guild.getInventory(), INVENTORIES::guild, NULL );
	}
}

//---------------------------------------------------
void CGuildVersionAdapter::adaptToVersion2(CGuild & guild) const
{
	guild.setCreationDate( CTickEventHandler::getGameCycle() );
}

//---------------------------------------------------
void CGuildVersionAdapter::adaptToVersion3(CGuild & guild) const
{
	// Set faction allegiance to PVP_CLAN::None.  This is a "limbo" status for existing guilds.
	guild.setDeclaredCult(PVP_CLAN::None);
	guild.setDeclaredCiv(PVP_CLAN::None);
}


//---------------------------------------------------
void CGuildVersionAdapter::adaptToVersion4(CGuild & guild) const
{
	CCharacterVersionAdapter::getInstance()->setToolsToMaxHP( guild.getInventory() );
}

