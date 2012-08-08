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

#include "camera_animation_manager/position_or_entity_pos_resolver.h"
#include "entity_cl.h"
#include "game_share/position_or_entity_type.h"
#include "entities.h"

NLMISC::CVector resolvePositionOrEntityPosition(const TPositionOrEntity& posOrEntity)
{
	if (!posOrEntity.isValid())
	{
		nlwarning("<resolvePositionOrEntityPosition> invalid position or entity");
		return NLMISC::CVector();
	}

	if (posOrEntity.isPosition())
		return posOrEntity.getPosition();

	CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(posOrEntity.getEntityId());
	if (!entity)
	{
		nlwarning("<resolvePositionOrEntityPosition> invalid entity with compressed id %d", posOrEntity.getEntityId());
		return NLMISC::CVector();
	}
	else
	{
		NLMISC::CVector pos = entity->pos();
		return pos;
	}
	return NLMISC::CVector();
}

