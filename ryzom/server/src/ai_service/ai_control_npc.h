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

#ifndef RYAI_CONTROL_NPC_H
#define RYAI_CONTROL_NPC_H

#include "profile.h"

class CBotPlayer;
class CSpawnBotNpc;

//////////////////////////////////////////////////////////////////////////////
// CPlayerControlNpc                                                        //
//////////////////////////////////////////////////////////////////////////////

class CPlayerControlNpc : public NLMISC::CRefCount
{
public:
	/// ctor
	/// \param player : the player which controls the bot
	/// \param bot : the npc which is controlled by the player
	CPlayerControlNpc(CBotPlayer* player, CSpawnBotNpc* bot);
	~CPlayerControlNpc();

	/// check if the object is valid
	/// WARNING: you cannot call any other method if this returns false
	bool isValid() const;

	/// update the control on the npc
	void updateControl(uint ticks);
	CSpawnBotNpc* getSpawnBot() const;

private:
	/// the player controlling the bot
	NLMISC::CRefPtr<CBotPlayer>	_Player;
	CSpawnBotNpc*				_Bot;
	CProfilePtr					_ControlProfileManager;
	uint32						_LastPathUpdate;
	NLMISC::CEntityId			_CPlayerEntityId;
	NLMISC::CEntityId			_CNpcEntityId;

};

#endif
