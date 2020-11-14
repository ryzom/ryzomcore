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



#ifndef RY_CONSTANTS_H
#define RY_CONSTANTS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"

// define for xp gains calculation
#define MAX_DELTA_LVL		50
#define MIN_DELTA_LVL		-50
#define NB_DELTA_LVL		101
#define MIDDLE_DELTA_LVL	50

// define for sheath
#define NB_SHEATH			1

// square of the max talking distance in meters
const sint32 MaxTalkingDistSquare = 64; // 8 m

// same, but for "talking" to outpost building
const sint32 MaxTalkingOutpostBuildingDistSquare=	256;	// 16 m

// square of max distance for commands to animals in meters
const sint32 MaxAnimalCommandDistSquare = 900; // 30 m

// raw material source bar speeds
const float DeltaMoveBarPerSec = 60.0f; // 60 units per second
const float DeltaResetBarPerSec = 130.0f; // 130 units per second
const float DeltaTimeBarPerSec = 5.0f; // 5 units per second

// ring editor/animator dynamic channel offset (most significant bit of short id set to 1)
const uint64	RingDynChanOffset = UINT64_CONSTANT(0x8000000000);

#endif
