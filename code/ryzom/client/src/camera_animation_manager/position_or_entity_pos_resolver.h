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


/// Function that returns the stored position if it contains a position
/// Or the current entity's position if it contains an entity
NLMISC::CVector resolvePositionOrEntityPosition(const TPositionOrEntity& posOrEntity);


#endif /* RY_POSITIONORENTITYPOSRESOLVER_H */
