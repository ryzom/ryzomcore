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

#ifndef RY_POSITIONORENTITYPOSRESOLVER_H
#define RY_POSITIONORENTITYPOSRESOLVER_H

#include "nel/misc/entity_id.h"
#include "nel/misc/vector.h"
#include "game_share/position_or_entity_type.h"
#include "camera_animation_manager/position_or_entity_helper.h"
#include "camera_animation_manager/camera_animation_info.h"


/// Function that returns the stored position if it contains a position
/// Or the current entity's position if it contains an entity
NLMISC::CVector resolvePositionOrEntityPosition(const CPositionOrEntity& posOrEntity, const TCameraAnimationInputInfo& currCamInfo);
NLMISC::CVector resolvePositionOrEntityPosition(const CPositionOrEntity& posOrEntity);

/// Functions that returns the stored look at target as a normalized direction vector
NLMISC::CVector resolvePositionOrEntityTargetDir(const CPositionOrEntity& posOrEntity, const TCameraAnimationInputInfo& currCamInfo,
	const NLMISC::CVector& currCamPos);

#endif /* RY_POSITIONORENTITYPOSRESOLVER_H */
