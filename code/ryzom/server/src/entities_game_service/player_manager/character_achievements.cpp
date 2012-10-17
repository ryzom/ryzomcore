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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
//#include "egs_sheets/egs_sheets.h"
//#include "egs_sheets/egs_static_encyclo.h"
//#include "game_share/msg_encyclopedia.h"
//#include "game_share/string_manager_sender.h"
//#include "player_manager/player_manager.h"
//#include "player_manager/player.h"
//#include "mission_manager/mission_manager.h"
#include "player_manager/character_achievements.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CCharacterAchievements);

//-----------------------------------------------------------------------------
// methods CCharacterEncyclopedia
//-----------------------------------------------------------------------------

CCharacterAchievements::CCharacterAchievements(CCharacter &c) : _Char(c)
{
	init();
}

//-----------------------------------------------------------------------------

void CCharacterAchievements::init()
{
	nlinfo("hello achievements");
	//load atoms
}

//-----------------------------------------------------------------------------

void CCharacterAchievements::clear()
{
	//clear atoms
}

//-----------------------------------------------------------------------------

void CCharacterAchievements::mobKill(TDataSetRow creatureRowId)
{
	const CCreature *creature = CreatureManager.getCreature(creatureRowId);
	if (creature)
	{
		nlinfo("player has killed a mob: %s!",creature->getType().toString().c_str());
	}
}

void CCharacterAchievements::inPlace(const CPlace *region)
{
	nlinfo("player in region %u",region->getId());
}

void CCharacterAchievements::fameValue(uint32 factionIndex, sint32 playerFame)
{
	nlinfo("fame: f(%u)=>v(%u)",factionIndex,playerFame);
}

void CCharacterAchievements::tickUpdate()
{
	//evaluate atoms
}