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
#include "nel/misc/variable.h"
#include "ai_variables.h"

using namespace NLMISC;

CVariable<TGameCycle>	DynamicMaxUpdatePeriod("ai", "DynamicMaxUpdatePeriod", "The maximum update frequency (in ticks) for the dynamic system", 900, 0, true);

CVariable<bool>	LogAcceptablePos("ai", "LogAcceptablePos", "Log the corrected and invalid world position.", false, 0, true);

CVariable<bool>	LogGroupCreationFailure("ai", "LogGroupCreationFailure", "Log the dynamic group creation failure details.", false, 0, true);

CVariable<bool>	LogAliasTreeOwner("ai", "LogAliasTreeOwner", "Log the alias tree owner creation.", false, 0, true);

CVariable<bool>	LogOutpostDebug("ai", "LogOutpostDebug", "Log the outpost debug log.", false, 0, true);

CVariable<uint32>	RandomPosMaxRetry("ai", "RandomPosMaxRetry", "The total number of try to find the random position around place.", 32*200, 0, true);

CVariable<float>	SpeedFactor("ai", "SpeedFactor", "Speed factor to scall up/down all bot speed .", 1, 0, true);

void validateTimerSpeedUp(IVariable &var)
{
	CVariable<int> &speedUp = static_cast<CVariable<int>&>(var);
	if (speedUp < 1)
		speedUp = 1;
}

CVariable<int> TimerSpeedUp("ai", "TimerSpeedUp", "Accelerate timed event triggering. Must between [1- oo]", 1, 0, true, validateTimerSpeedUp);

CVariable<int> DefaultWanderMinTimer("ai", "DefaultWanderMinTimer", "Min iddle time (in ticks) in wander behavior)", 50, 0, true);
CVariable<int> DefaultWanderMaxTimer("ai", "DefaultWanderMaxTimer", "Min iddle time (in ticks) in wander behavior)", 50, 0, true);

CVariable<sint32> FameForGuardAttack("ai", "FameForGuardAttack", "The fame value under witch guard attack the players", -450000, 0, true);
CVariable<sint32> FameForGuardHelp("ai", "FameForGuardHelp", "The fame value that allow guard to defend player", -450000, 0, true);

CVariable<uint32> DefaultNpcAggroDist("ai", "DefaultNpcAggroDist", "The default aggro distance for NPC", 15, 0, true);
CVariable<float> DefaultEscortRange("ai", "DefaultEscortRange", "The default escort range", 10, 0, true);


namespace AISStat
{

// managers
CVariable<uint32> StatMgrTotalUpdCtr("ai_stat", "StatMgrTotalUpdCtr", "MgrTotalUpdCtr", 0, 100, false);
CVariable<uint32> StatMgrFaunaUpdCtr("ai_stat", "StatMgrFaunaUpdCtr", "MgrFaunaUpdCtr", 0, 100, false);
CVariable<uint32> StatMgrNpcUpdCtr("ai_stat", "StatMgrNpcUpdCtr", "MgrNpcUpdCtr", 0, 100, false);
CVariable<uint32> StatMgrPetUpdCtr("ai_stat", "StatMgrPetUpdCtr", "MgrPetUpdCtr", 0, 100, false);
CVariable<uint32> StatMgrPlayerUpdCtr("ai_stat", "StatMgrPlayerUpdCtr", "MgrPlayerUpdCtr", 0, 100, false);
// groups
CVariable<uint32> StatGrpTotalUpdCtr("ai_stat", "StatGrpTotalUpdCtr", "GrpTotalUpdCtr", 0, 100, false);
CVariable<uint32> StatGrpFaunaUpdCtr("ai_stat", "StatGrpFaunaUpdCtr", "GrpFaunaUpdCtr", 0, 100, false);
CVariable<uint32> StatGrpNpcUpdCtr("ai_stat", "StatGrpNpcUpdCtr", "GrpNpcUpdCtr", 0, 100, false);
CVariable<uint32> StatGrpPetUpdCtr("ai_stat", "StatGrpPetUpdCtr", "GrpPetUpdCtr", 0, 100, false);
// bots
CVariable<uint32> StatBotTotalUpdCtr("ai_stat", "StatBotTotalUpdCtr", "BotTotalUpdCtr", 0, 100, false);
CVariable<uint32> StatBotFaunaUpdCtr("ai_stat", "StatBotFaunaUpdCtr", "BotFaunaUpdCtr", 0, 100, false);
CVariable<uint32> StatBotNpcUpdCtr("ai_stat", "StatBotNpcUpdCtr", "BotNpcUpdCtr", 0, 100, false);
// Pathfinding
CVariable<uint32> StatPathFindingCalls     ("ai_stat", "StatPathFindingCalls",      "PathFindingCalls",      0,   100, false);
CVariable<float>  StatPathFindingIterations("ai_stat", "StatPathFindingIterations", "PathFindingIterations", 0.f, 100, false);
CVariable<float>  StatPathFindingHeapSize  ("ai_stat", "StatPathFindingHeapSize",   "PathFindingHeapSize",   0.f, 100, false);
CVariable<uint32> StatPathFindingCallsInside     ("ai_stat", "StatPathFindingCallsInside",      "PathFindingCallsInside",      0,   100, false);
CVariable<float>  StatPathFindingIterationsInside("ai_stat", "StatPathFindingIterationsInside", "PathFindingIterationsInside", 0.f, 100, false);
CVariable<float>  StatPathFindingHeapSizeInside  ("ai_stat", "StatPathFindingHeapSizeInside",   "PathFindingHeapSizeInside",   0.f, 100, false);
// player visibility
CVariable<uint32> StatGrpFastUpdCtr("ai_stat", "StatGrpFastUpdCtr", "GrpFastUpdCtr", 0, 100, false);
CVariable<uint32> StatGrpSlowUpdCtr("ai_stat", "StatGrpSlowUpdCtr", "GrpSlowUpdCtr", 0, 100, false);
CVariable<uint32> StatVision0Ctr       ("ai_stat", "StatVision0Ctr",        "Player in vision at 0m",             0, 100, false);
CVariable<uint32> StatVision1To64Ctr   ("ai_stat", "StatVision1To64Ctr",    "Player in vision from 1m to 64m",    0, 100, false);
CVariable<uint32> StatVision65To127Ctr ("ai_stat", "StatVision65To127Ctr",  "Player in vision from 65m to 127m",  0, 100, false);
CVariable<uint32> StatVision128To254Ctr("ai_stat", "StatVision128To254Ctr", "Player in vision from 128m to 254m", 0, 100, false);
CVariable<uint32> StatVision255Ctr     ("ai_stat", "StatVision255Ctr",      "Player in vision at 255m",           0, 100, false);

// managers
uint32 MgrTotalUpdCtr;
uint32 MgrFaunaUpdCtr;
uint32 MgrNpcUpdCtr;
uint32 MgrPetUpdCtr;
uint32 MgrPlayerUpdCtr;
// groups
uint32 GrpTotalUpdCtr;
uint32 GrpFaunaUpdCtr;
uint32 GrpNpcUpdCtr;
uint32 GrpPetUpdCtr;
// bots
uint32 BotTotalUpdCtr;
uint32 BotFaunaUpdCtr;
uint32 BotNpcUpdCtr;
// Pathfinding
uint32 PathFindingCalls;
uint32 PathFindingIterations;
uint32 PathFindingHeapSize;
uint32 PathFindingCallsInside;
uint32 PathFindingIterationsInside;
uint32 PathFindingHeapSizeInside;
// player visibility
uint32 GrpFastUpdCtr;
uint32 GrpSlowUpdCtr;
uint32 VisionCtr[256];

}

// Aggro parameters
CVariable<float> AggroReturnDistCheck("ai_aggro", "AggroReturnDistCheck", "en m", 15.f, 0, true);
CVariable<float> AggroReturnDistCheckFauna("ai_aggro", "AggroReturnDistCheckFauna", "en m", 15.f, 0, true);
CVariable<float> AggroReturnDistCheckNpc("ai_aggro", "AggroReturnDistCheckNpc", "en m", 1.5f, 0, true);
CVariable<float> AggroD1Radius("ai_aggro", "AggroD1Radius", "en m", 100.f, 0, true);
CVariable<float> AggroD2Radius("ai_aggro", "AggroD2Radius", "en m", 50.f, 0, true);
CVariable<float> AggroPrimaryGroupDist("ai_aggro", "AggroPrimaryGroupDist", "en m", 0.f, 0, true);
CVariable<float> AggroPrimaryGroupCoef("ai_aggro", "AggroPrimaryGroupCoef", "%age, entre 0 et 1", 0.f, 0, true);
CVariable<float> AggroSecondaryGroupDist("ai_aggro", "AggroSecondaryGroupDist", "en m", 0.f, 0, true);
CVariable<float> AggroSecondaryGroupCoef("ai_aggro", "AggroSecondaryGroupCoef", "%age, entre 0 et 1", 0.f, 0, true);
CVariable<float> AggroPropagationRadius("ai_aggro", "AggroPropagationRadius", "en m", 60.f, 0, true);

CVariable<uint32> HealSpecificDowntime("ai_heal", "HealSpecificDowntime", "", 100, 0, true);
CVariable<uint32> HealSpecificDowntimeSelf("ai_heal", "HealSpecificDowntimeSelf", "", 100, 0, true);



//max dist for npc selection (ring only)
CVariable<double>	RingMaxSelectDist("ai", "RingMaxSelectDist", "Max dist used to allow far selection or not.", 20, 0, true);

