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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef RY_PLAYER_MORPH_H
#define RY_PLAYER_MORPH_H

#include "game_share/base_types.h"
#include "game_share/inventories.h"

#include <string>
#include <vector>

namespace NLMISC
{
	class CLog;
}

class CStaticCreatures;
class CCharacter;
class CEntityBase;

/// creature form of a morphed player, NULL for anyone else
const CStaticCreatures *getPlayerMorphForm(const TDataSetRow &rowId);

/// creature form an entity uses ai actions with: the morph of a player, the sheet of a creature
const CStaticCreatures *getActingCreatureForm(const TDataSetRow &rowId);

/// true for a player in its own homin body, false for a morphed player or a non-player
bool isHominBody(const CEntityBase *entity);

/// true when a morphed player asks to change what it wears: an equipment or hand slot, or an equipped item
bool isMorphLockedItem(const CCharacter *c, INVENTORIES::TInventory inventory, uint32 slot);

/// ask the client of a living, dismounted player to morph into <role, or creature sheet> [<attackable 0|1>
/// [<duration in seconds, 0 for none>]], as /a morph and the Ark morph command do; false with the reason in log
bool morphPlayer(CCharacter *c, const std::vector<std::string> &args, NLMISC::CLog &log);

/// end the morph of a player as unmorph does: a dead morph stands up as the homin it was
void unmorphPlayer(CCharacter *c);

/// give the player its homin look and values back
void endPlayerMorph(CCharacter *c);

/// a role morph that lost enough hit points turns into the next role of the event
void checkMorphTransformation(CCharacter *c);

/// end a morph or drop a morph request whose duration is over
void checkMorphEnd(CCharacter *c);

/// a dying role morph gives the loot of its role to the player who killed it
void dropMorphLoot(CCharacter *c, const TDataSetRow &killerRowId);

#endif
