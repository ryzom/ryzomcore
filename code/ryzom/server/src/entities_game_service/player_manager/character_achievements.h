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

#ifndef EGS_CHARACTER_ACHIEVEMENTS_H
#define EGS_CHARACTER_ACHIEVEMENTS_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
//#include "game_share/persistent_data.h"
#include "zone_manager.h"
#include "creature_manager/creature.h"
#include "creature_manager/creature_manager.h"

//-----------------------------------------------------------------------------

class CCharacter;

/**
 * Dynamic part of the encyclopedia stored in a character
 * This structure is optimized for size because its stored directly in the player persistant data stuff
 * We use CEncyMsgXXX for sending info to the player
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2004
 */
class CCharacterAchievements
{
	NL_INSTANCE_COUNTER_DECL(CCharacterAchievements);
public:

	CCharacterAchievements(CCharacter &c);

	// Construct the encyclopedia album structure from the static sheet that defines encyclopedia
	// This method ensure that we have at least the same number of album and the same number of thema by album 
	// as in the sheets defines the encyclopedia
	void init();

	// remove all
	void clear();

	void mobKill(TDataSetRow creatureRowId);

	void inPlace(const CPlace *region);

	void fameValue(uint32 factionIndex, sint32 playerFame);
	
	void tickUpdate();


private:

	
	
private:
	
	// The parent class
	CCharacter &_Char;

};

#endif // EGS_CHARACTER_ACHIEVEMENTS_H
