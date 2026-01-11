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

#ifndef AI_VARIABLES_H
#define AI_VARIABLES_H

#include "nel/misc/variable.h"

namespace AISStat
{

// Managers
extern NLMISC::CVariable<uint32> StatMgrTotalUpdCtr;
extern NLMISC::CVariable<uint32> StatMgrFaunaUpdCtr;
extern NLMISC::CVariable<uint32> StatMgrNpcUpdCtr;
extern NLMISC::CVariable<uint32> StatMgrPetUpdCtr;
extern NLMISC::CVariable<uint32> StatMgrPlayerUpdCtr;

// Groups
extern NLMISC::CVariable<uint32> StatGrpTotalUpdCtr;
extern NLMISC::CVariable<uint32> StatGrpFaunaUpdCtr;
extern NLMISC::CVariable<uint32> StatGrpNpcUpdCtr;
extern NLMISC::CVariable<uint32> StatGrpPetUpdCtr;

// Bots
extern NLMISC::CVariable<uint32> StatBotTotalUpdCtr;
extern NLMISC::CVariable<uint32> StatBotFaunaUpdCtr;
extern NLMISC::CVariable<uint32> StatBotNpcUpdCtr;

// Pathfinding
extern NLMISC::CVariable<uint32> StatPathFindingCalls;
extern NLMISC::CVariable<float> StatPathFindingIterations;
extern NLMISC::CVariable<float> StatPathFindingHeapSize;
extern NLMISC::CVariable<uint32> StatPathFindingCallsInside;
extern NLMISC::CVariable<float> StatPathFindingIterationsInside;
extern NLMISC::CVariable<float> StatPathFindingHeapSizeInside;

// Player visibility
extern NLMISC::CVariable<uint32> StatGrpFastUpdCtr;
extern NLMISC::CVariable<uint32> StatGrpSlowUpdCtr;
extern NLMISC::CVariable<uint32> StatVision0Ctr;
extern NLMISC::CVariable<uint32> StatVision1To64Ctr;
extern NLMISC::CVariable<uint32> StatVision65To127Ctr;
extern NLMISC::CVariable<uint32> StatVision128To254Ctr;
extern NLMISC::CVariable<uint32> StatVision255Ctr;

// Managers
extern uint32 MgrTotalUpdCtr;
extern uint32 MgrFaunaUpdCtr;
extern uint32 MgrNpcUpdCtr;
extern uint32 MgrPetUpdCtr;
extern uint32 MgrPlayerUpdCtr;
// Groups
extern uint32 GrpTotalUpdCtr;
extern uint32 GrpFaunaUpdCtr;
extern uint32 GrpNpcUpdCtr;
extern uint32 GrpPetUpdCtr;
// Bots
extern uint32 BotTotalUpdCtr;
extern uint32 BotFaunaUpdCtr;
extern uint32 BotNpcUpdCtr;
// Pathfinding
extern uint32 PathFindingCalls;
extern uint32 PathFindingIterations;
extern uint32 PathFindingHeapSize;
extern uint32 PathFindingCallsInside;
extern uint32 PathFindingIterationsInside;
extern uint32 PathFindingHeapSizeInside;
// Player visibility
extern uint32 GrpFastUpdCtr;
extern uint32 GrpSlowUpdCtr;
extern uint32 VisionCtr[256];

inline
void countersBegin()
{
	// Managers
	MgrTotalUpdCtr  = 0;
	MgrFaunaUpdCtr  = 0;
	MgrNpcUpdCtr    = 0;
	MgrPetUpdCtr    = 0;
	MgrPlayerUpdCtr = 0;
	// Groups
	GrpTotalUpdCtr  = 0;
	GrpFaunaUpdCtr  = 0;
	GrpNpcUpdCtr    = 0;
	GrpPetUpdCtr    = 0;
	// Bots
	BotTotalUpdCtr  = 0;
	BotFaunaUpdCtr  = 0;
	BotNpcUpdCtr    = 0;
	// Pathfinding
	PathFindingCalls      = 0;
	PathFindingIterations = 0;
	PathFindingHeapSize   = 0;
	PathFindingCallsInside      = 0;
	PathFindingIterationsInside = 0;
	PathFindingHeapSizeInside   = 0;
	// Player visibility
	AISStat::GrpFastUpdCtr   = 0;
	AISStat::GrpSlowUpdCtr   = 0;
	for (size_t i=0; i<256; ++i)
		VisionCtr[i] = 0;
}

inline
void countersEnd()
{
	// Managers
	StatMgrTotalUpdCtr  = MgrTotalUpdCtr;
	StatMgrFaunaUpdCtr  = MgrFaunaUpdCtr;
	StatMgrNpcUpdCtr    = MgrNpcUpdCtr;
	StatMgrPetUpdCtr    = MgrPetUpdCtr;
	StatMgrPlayerUpdCtr = MgrPlayerUpdCtr;
	// Groups
	StatGrpTotalUpdCtr  = GrpTotalUpdCtr;
	StatGrpFaunaUpdCtr  = GrpFaunaUpdCtr;
	StatGrpNpcUpdCtr    = GrpNpcUpdCtr;
	StatGrpPetUpdCtr    = GrpPetUpdCtr;
	// Bots
	StatBotTotalUpdCtr  = BotTotalUpdCtr;
	StatBotFaunaUpdCtr  = BotFaunaUpdCtr;
	StatBotNpcUpdCtr    = BotNpcUpdCtr;
	// Pathfinding
	StatPathFindingCalls      = PathFindingCalls;
	StatPathFindingIterations = PathFindingCalls!=0?(float)PathFindingIterations/(float)PathFindingCalls:0.f;
	StatPathFindingHeapSize   = PathFindingCalls!=0?(float)PathFindingHeapSize  /(float)PathFindingCalls:0.f;
	StatPathFindingCallsInside      = PathFindingCallsInside;
	StatPathFindingIterationsInside = PathFindingCallsInside!=0?(float)PathFindingIterationsInside/(float)PathFindingCallsInside:0.f;
	StatPathFindingHeapSizeInside   = PathFindingCallsInside!=0?(float)PathFindingHeapSizeInside  /(float)PathFindingCallsInside:0.f;
	// Player visibility
	StatGrpFastUpdCtr   = GrpFastUpdCtr;
	StatGrpSlowUpdCtr   = GrpSlowUpdCtr;
	StatVision0Ctr      = VisionCtr[0];
	uint32 total = 0;
	for (size_t i=1; i<=64; ++i)
		total += VisionCtr[i];
	StatVision1To64Ctr  = total;
	total = 0;
	for (size_t i=65; i<=127; ++i)
		total += VisionCtr[i];
	StatVision65To127Ctr = total;
	total = 0;
	for (size_t i=128; i<=254; ++i)
		total += VisionCtr[i];
	StatVision128To254Ctr = total;
	StatVision255Ctr    = VisionCtr[255];
}

}

extern NLMISC::CVariable<float> AggroReturnDistCheck;
extern NLMISC::CVariable<float> AggroReturnDistCheckFauna;
extern NLMISC::CVariable<float> AggroReturnDistCheckNpc;
extern NLMISC::CVariable<float> AggroD1Radius;
extern NLMISC::CVariable<float> AggroD2Radius;
extern NLMISC::CVariable<float> AggroPrimaryGroupDist;
extern NLMISC::CVariable<float> AggroPrimaryGroupCoef;
extern NLMISC::CVariable<float> AggroSecondaryGroupDist;
extern NLMISC::CVariable<float> AggroSecondaryGroupCoef;
extern NLMISC::CVariable<float> AggroPropagationRadius;

extern NLMISC::CVariable<uint32> HealSpecificDowntime;
extern NLMISC::CVariable<uint32> HealSpecificDowntimeSelf;

#endif // AI_VARIABLES_H
