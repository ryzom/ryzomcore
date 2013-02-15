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

#include "pvp_duel.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "pvp_manager/pvp.h"
#include "pvp_manager/pvp_manager_2.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;


//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation CPVPDuel::getPVPRelation( CCharacter * user, CEntityBase * target, bool curative ) const
{
	if( !user || !target )
	{
		nlwarning("<CPVPDuel::getPVPRelation> user: %p  target: %p",user,target);
		return PVP_RELATION::Unknown;
	}

	if( target->getId().getType() != RYZOMID::player )
	{
		return PVP_RELATION::Unknown;
	}

	// people are enemies if they are in the same duel
	if ( user->getDuelOpponent() == target )
	{
		CPVPManager2::getInstance()->setPVPFactionEnemyReminder( false );
		return PVP_RELATION::Ennemy;
	}

	// if target is in duel then he's neutral pvp
	CCharacter * targetChar = static_cast<CCharacter*>(target);
	if( targetChar->getDuelOpponent() != NULL )
	{
		return PVP_RELATION::NeutralPVP;
	}

	return PVP_RELATION::Neutral;
}

//----------------------------------------------------------------------------
bool CPVPDuel::isTPValid( CCharacter* actor, CGameItemPtr TeleportTicket ) const
{
	if( actor->getPvPRecentActionFlag() )
		return false;
	return true;
}

