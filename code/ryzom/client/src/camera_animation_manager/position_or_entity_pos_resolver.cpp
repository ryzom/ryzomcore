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

NLMISC::CVector resolvePositionOrEntityPosition(const CPositionOrEntity& posOrEntity, const TCameraAnimationInputInfo& currCamInfo)
{
	if (!posOrEntity.isValid())
	{
		nlwarning("<resolvePositionOrEntityPosition> invalid position or entity");
		return NLMISC::CVector();
	}

	if (posOrEntity.isPosition())
		return posOrEntity.getPosition();
	else if (posOrEntity.isPreviousPos())
	{
		return currCamInfo.StartCamPos;
	}
	else if (posOrEntity.isReturnPos())
	{
		return currCamInfo.AnimStartCamPos;
	}
	else if (posOrEntity.isEntityId())
	{
		CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(posOrEntity.getEntityId());
		if (!entity)
		{
			nlwarning("<resolvePositionOrEntityPosition> invalid entity with compressed id %d", posOrEntity.getEntityId());
			return NLMISC::CVector();
		}
		else
		{
			NLMISC::CVector pos;
			if (!entity->getHeadPos(pos))
				pos = entity->pos();
		
			return pos;
		}
	}
	return NLMISC::CVector();
}

NLMISC::CVector resolvePositionOrEntityPosition(const CPositionOrEntity& posOrEntity)
{
	if (!posOrEntity.isValid())
	{
		nlwarning("<resolvePositionOrEntityPosition> invalid position or entity");
		return NLMISC::CVector();
	}

	if (posOrEntity.isPosition())
		return posOrEntity.getPosition();
	else if (posOrEntity.isEntityId())
	{
		CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(posOrEntity.getEntityId());
		if (!entity)
		{
			nlwarning("<resolvePositionOrEntityPosition> invalid entity with compressed id %d", posOrEntity.getEntityId());
			return NLMISC::CVector();
		}
		else
		{
			NLMISC::CVector pos;
			if (!entity->getHeadPos(pos))
				pos = entity->pos();

			return pos;
		}
	}
	return NLMISC::CVector();
}

NLMISC::CVector resolvePositionOrEntityTargetDir(const CPositionOrEntity& posOrEntity, const TCameraAnimationInputInfo& currCamInfo,
												const NLMISC::CVector& currCamPos)
{
	if (!posOrEntity.isValid())
	{
		nlwarning("<resolvePositionOrEntityPosition> invalid position or entity");
		return NLMISC::CVector();
	}

	if (posOrEntity.isPosition())
	{
		NLMISC::CVector dir = posOrEntity.getPosition() - currCamPos;
		dir.normalize();
		return dir;
	}
	else if (posOrEntity.isPreviousPos())
	{
		return currCamInfo.StartCamLookAtDir;
	}
	else if (posOrEntity.isReturnPos())
	{
		return currCamInfo.AnimStartCamLookAtDir;
	}
	else if (posOrEntity.isEntityId())
	{
		CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(posOrEntity.getEntityId());
		if (!entity)
		{
			nlwarning("<resolvePositionOrEntityPosition> invalid entity with compressed id %d", posOrEntity.getEntityId());
			return NLMISC::CVector();
		}
		else
		{
			NLMISC::CVector pos;
			if (!entity->getHeadPos(pos))
				pos = entity->pos();

			NLMISC::CVector dir = pos - currCamPos;
			dir.normalize();
			return dir;
		}
	}
	return NLMISC::CVector();
}
