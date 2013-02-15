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
#include "teleport_effect.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/s_effect.h"

CTeleportEffect::CTeleportEffect( const TDataSetRow & creatorRowId, 
				const TDataSetRow & targetRowId, 
				EFFECT_FAMILIES::TEffectFamily family, 
				sint32 effectValue, 
				NLMISC::TGameCycle endDate,
				const CStaticItem & form
				) :	CSTimedEffect(creatorRowId, targetRowId, family, true, effectValue,0, endDate),
					_Form( form )
				
{
}


void CTeleportEffect::removed()
{
	CCharacter * user = PlayerManager.getChar( _CreatorRowId );
	if ( user )
	{
		user->useTeleport(_Form);
	}
}
