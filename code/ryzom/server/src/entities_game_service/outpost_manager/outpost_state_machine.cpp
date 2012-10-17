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

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include "stdpch.h"

#include "player_manager/character.h"

#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild_char_proxy.h"

#include "outpost.h"
#include "outpost_manager.h"
#include "pvp_manager/pvp.h"
#include "pvp_manager/pvp_manager.h"
#include "egs_globals.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_outpost.h"
#include "server_share/used_continent.h"
#include "primitives_parser.h"
#include "world_instances.h"
#include "entity_manager/entity_manager.h"
#include "outpost_version_adapter.h"

//----------------------------------------------------------------------------
// namespaces
//----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

extern CVariable<uint32> OutpostStateTimeOverride;

static uint32 const seconds = 1;
static uint32 const minutes = 60*seconds;
static uint32 const hours = 60*minutes;
static uint32 const days = 24*hours;

CVariable<uint32> OutpostRangeForCancelOnReset("egs", "OutpostRangeForCancelOnReset", "Time range before next attack period in which a service reboot will cancel the challenge", true, 3*hours, true );
CVariable<bool> UseOutpostExploitFix("egs", "UseOutpostExploitFix", "set true to enable outpost exploit fix", true, false, true );

//----------------------------------------------------------------------------
/// All transition between states are made in that method. This is the
/// centralized place for state management to ease understanding of states and
/// transitions of the outpost. If states or transitions are added or modified
/// these changes should be done here. Each transition call basic action
/// methods. That way each transition is easy to understand.
///
/// More complex methods used bay several transitions would avoid some bugs
/// but make the code more complex to understand, and that would make any
/// extension attempt more error prone.
///
/// NOTE about the guild database updates:
/// by default an update is always done at the end for OUTPOST_PROPERTIES
/// if you add an action that needs another thing to be updated you must do it yourself
/// by calling askGuildDBUpdate(COutpostGuildDBUpdater::PROP_YOU_NEED_TO_UPDATE);
///
void COutpost::eventTriggered(OUTPOSTENUMS::TOutpostEvent event, void* eventParams)
{
	using namespace OUTPOSTENUMS;
	switch (_State)
	{
/****************************************************************************/
	case Peace: { switch (event) {
		case StartOfState:
		{
			OUTPOST_INF("Outpost %s: Starting a peace period", _Name.c_str());
			actionSetPVPActive(false);
			_FightData._MaxAttackLevel = 0;
			_FightData._MaxDefenseLevel = 0;
			_CurrentSquadsAQueue.clear();
			_CurrentSquadsBQueue.clear();
			// Clean attacker
			setAttackerGuild(0);
			resetDefaultDefenseSquads();
			if (isBelongingToAGuild())
			{
				if (_CurrentOutpostLevel>computeGuildMinimumOutpostLevel())
					actionSetTimer0(computeLevelDecrementTime());
			} else
				_CurrentOutpostLevel = computeTribeOutpostLevel();
		} break;
		case Challenged:
		{
			if (_DefaultSquads.size()==0 || _TribeSquadsA.size()==0)
			{
				OUTPOST_WRN("Outpost %s challenged although it has no default squads: challenge discarded", getName().c_str());
				setAttackerGuild(0);
				break;
			}
			actionStopTimer0();
			_RealChallengeTime = CTime::getSecondsSince1970();
			_ChallengeTime = (_RealChallengeTime/hours + 1)*hours; // Aligned on next hour
			_ChallengeHour = (_ChallengeTime%days)/hours; // Aligned on next hour
			nlassert(_ChallengeHour<24);
			actionResetAttackerExpenseLimit();
			actionResetMoneySpent();
			_MoneySpentByAttacker += getChallengeCost();
			OUTPOST_INF("Outpost %s challenged:", getName().c_str());
			OUTPOST_INF("- real challenge time: %dd%02dh%02dm%02ds", _RealChallengeTime/days, (_RealChallengeTime%days)/hours, (_RealChallengeTime%hours)/minutes, (_RealChallengeTime%minutes)/seconds);
			OUTPOST_INF("- challenge time:      %dd%02dh%02dm%02ds", _ChallengeTime/days, (_ChallengeTime%days)/hours, (_ChallengeTime%hours)/minutes, (_ChallengeTime%minutes)/seconds);
			OUTPOST_INF("- challenge hour:      %02dh", _ChallengeHour);
			actionPostNextState(WarDeclaration);
		//	tellEventState(event, _State);
		} break;
		case Timer0End:
		{
			if (isBelongingToAGuild())
			{
				if (_CurrentOutpostLevel>computeGuildMinimumOutpostLevel())
				{
					--_CurrentOutpostLevel;
					OUTPOST_INF("Outpost %s: outpost level is now %d", _Name.c_str(), _CurrentOutpostLevel+1);
				}
				if (_CurrentOutpostLevel>computeGuildMinimumOutpostLevel())
					actionSetTimer0(computeLevelDecrementTime());
			}
		} break;
		case EndOfState:
		{
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case WarDeclaration: { switch (event) {
		case StartOfState:
		{
			OUTPOST_INF("Outpost %s: Entering war preparation phase (war declaration)", _Name.c_str());
			if (OutpostStateTimeOverride>(uint32)0)
				actionSetTimer0(OutpostStateTimeOverride);
			else
				actionSetTimer0(computeChallengeTime());
			actionSetPVPActive(true);
		} break;
		case Timer0End:
		{
			actionPostNextState(AttackBefore);
		//	tellEventState(event, _State);
		} break;
		case EndOfState:
		{
			// Fake the challenge time in case we advanced the timers
			_ChallengeTime = CTime::getSecondsSince1970()-24*hours;
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case AttackBefore: { switch (event) {
		case StartOfState:
		{
			OUTPOST_INF("Outpost %s: Starting attack phase (before fights)", _Name.c_str());
			_CrashHappened = false;
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			if (OutpostStateTimeOverride>(uint32)0)
				actionSetTimer0(OutpostStateTimeOverride);
			else
				actionSetTimer0(computeTimeBeforeAttack());
		} break;
		case Timer0End:
		{
			actionPostNextState(AttackRound);
		//	tellEventState(event, _State);
		} break;
		case EndOfState:
		{
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case AttackRound: { switch (event) {
		case StartOfState: // Init the round
		{
			OUTPOST_INF("Outpost %s: Starting attack round %d, level %d", _Name.c_str(), _FightData._CurrentCombatRound+1, _FightData._CurrentCombatLevel+1);
			actionSetTimer0(computeRoundTime());						//< Round timer
			if (computeSquadCountB(_FightData._CurrentCombatLevel) > 0)
				actionSetTimer1(computeSpawnDelay(_FightData._CurrentCombatLevel));	//< SpawnTimer
			_FightData._SpawnedSquadsA = 0;
			_FightData._SpawnedSquadsB = 0;
			_FightData._KilledSquads = 0;
			_CurrentSquadsAQueue = _NextAttackSquadsA;
			_CurrentSquadsBQueue = _NextAttackSquadsB;

			// Number of squads to spawn
			uint32 squadCount = computeSquadCountA(_FightData._CurrentCombatLevel);
			actionBuySquadsA(squadCount, OUTPOSTENUMS::OutpostOwner);
			_CurrentSquadsA.resize(_CurrentSquadsAQueue.size());
			_CurrentSquadsB.resize(_CurrentSquadsBQueue.size());
			actionSpawnSquadsA(squadCount, OUTPOSTENUMS::OutpostOwner);
		} break;
		case Timer0End: // Check the round result
		{
			// Finish round
			actionStopTimer1(); //< This is just to prevent timer 1 to expire in next round if we artificially trigger timer 0
			bool won;
			if (_FightData._KilledSquads < (_FightData._SpawnedSquadsA+_FightData._SpawnedSquadsB))
			{
				if (_FightData._CurrentCombatLevel>0)
					--_FightData._CurrentCombatLevel;
				won = false;
			}
			else
			{
				++_FightData._CurrentCombatLevel;
				if (_FightData._CurrentCombatLevel > _FightData._MaxAttackLevel)
					_FightData._MaxAttackLevel = _FightData._CurrentCombatLevel;
				won = true;
			}
			actionPayBackAliveSquads(OUTPOSTENUMS::OutpostOwner);
			actionDespawnAllSquads();
			++_FightData._CurrentCombatRound;
			// If that was last round finish fight
			if (_FightData._CurrentCombatRound>=computeRoundCount())
			{
				_FightData._CurrentCombatLevel = 0;
				_FightData._CurrentCombatRound = 0;
				if (won)
				{
					broadcastMessage(OwnerFighters, LastRoundLost);
					broadcastMessage(AttackerFighters, LastRoundWon);
				}
				else
				{
					broadcastMessage(OwnerFighters, LastRoundWon);
					broadcastMessage(AttackerFighters, LastRoundLost);
				}
				if (_FightData._MaxAttackLevel > _CurrentOutpostLevel)
				{
					broadcastMessage(OwnerGuild, AttackSucceeded);
					broadcastMessage(AttackerGuild, AttackSucceeded);
				}
				else
				{
					broadcastMessage(OwnerGuild, AttackFailed);
					broadcastMessage(AttackerGuild, AttackFailed);
				}
				actionPostNextState(AttackAfter);
			}
			else // start another round
			{
				if (won)
				{
					broadcastMessage(OwnerFighters, RoundLost);
					broadcastMessage(AttackerFighters, RoundWon);
				}
				else
				{
					broadcastMessage(OwnerFighters, RoundWon);
					broadcastMessage(AttackerFighters, RoundLost);
				}
				actionPostNextState(AttackRound);
			}
		} break;
		case Timer1End: // Spawn a new squad
		{
			// Index of the squad to spawn
			if (_FightData._SpawnedSquadsB < computeSquadCountB(_FightData._CurrentCombatLevel))
			{
				uint32 squadIndex = _FightData._SpawnedSquadsB;
				actionBuySquadsB(squadIndex, OUTPOSTENUMS::OutpostOwner);
				actionSpawnSquadsB(squadIndex, OUTPOSTENUMS::OutpostOwner);
//				++_FightData._SpawnedSquadsB;
			}
			if (_FightData._SpawnedSquadsB < computeSquadCountB(_FightData._CurrentCombatLevel))
				actionSetTimer1(computeSpawnDelay(_FightData._CurrentCombatLevel));
		} break;
//		case SquadKilled: // Increment dead squad count
//		{
//			++_FightData._KilledSquads;
//		} break;
		case EndOfState:
		{
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case AttackAfter: { switch (event) {
		case StartOfState:
		{
			OUTPOST_INF("Outpost %s: Continuing attack phase (after fight)", _Name.c_str());
			// Here we may shorten the delay if _MaxAttackLevel<=_CurrentOutpostLevel
			if (OutpostStateTimeOverride>(uint32)0)
				actionSetTimer0(OutpostStateTimeOverride);
			else
				actionSetTimer0(computeTimeAfterAttack());
		} break;
		case Timer0End:
		{
			if (_FightData._MaxAttackLevel > _CurrentOutpostLevel)
			{
				if (!isBelongingToAGuild())
				{
					_CurrentOutpostLevel = _FightData._MaxAttackLevel;
					actionChangeOwner();
					actionPostNextState(Peace);
				}
				else
					actionPostNextState(DefenseBefore);
			}
			else
				actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		case EndOfState:
		{
			// Fake the challenge time in case we advanced the timers
			_ChallengeTime = CTime::getSecondsSince1970()-48*hours;
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case DefenseBefore: { switch (event) {
		case StartOfState:
		{
			OUTPOST_INF("Outpost %s: Starting defense phase (before fight)", _Name.c_str());
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			if (OutpostStateTimeOverride>(uint32)0)
				actionSetTimer0(OutpostStateTimeOverride);
			else
				actionSetTimer0(computeTimeBeforeDefense());
		} break;
		case Timer0End:
		{
			actionPostNextState(DefenseRound);
		//	tellEventState(event, _State);
		} break;
		case EndOfState:
		{
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case DefenseRound: { switch (event) {
		case StartOfState: // Init the round
		{
			OUTPOST_INF("Outpost %s: Starting defense round %d, level %d", _Name.c_str(), _FightData._CurrentCombatRound+1, _FightData._CurrentCombatLevel+1);
			actionSetTimer0(computeRoundTime());						//< Round timer
			actionSetTimer1(computeSpawnDelay(_FightData._CurrentCombatLevel));	//< SpawnTimer
			_FightData._SpawnedSquadsA = 0;
			_FightData._SpawnedSquadsB = 0;
			_FightData._KilledSquads = 0;
			_CurrentSquadsAQueue = _NextDefenseSquadsA;
			_CurrentSquadsBQueue = _NextDefenseSquadsB;

			// Number of squads to spawn
			uint32 squadCount = computeSquadCountA(_FightData._CurrentCombatLevel);
			actionBuySquadsA(squadCount, OUTPOSTENUMS::OutpostAttacker);
			_CurrentSquadsA.resize(_CurrentSquadsAQueue.size());
			_CurrentSquadsB.resize(_CurrentSquadsBQueue.size());
			actionSpawnSquadsA(squadCount, OUTPOSTENUMS::OutpostAttacker);
		} break;
		case Timer0End: // Check the round result
		{
			// End round and start another
			bool won;
			if ((_FightData._SpawnedSquadsA+_FightData._SpawnedSquadsB - _FightData._KilledSquads)!=0)
			{
				if (_FightData._CurrentCombatLevel>0)
					--_FightData._CurrentCombatLevel;
				won = false;
			}
			else
			{
				++_FightData._CurrentCombatLevel;
				if (_FightData._CurrentCombatLevel > _FightData._MaxDefenseLevel)
					_FightData._MaxDefenseLevel = _FightData._CurrentCombatLevel;
				won = true;
			}
			actionPayBackAliveSquads(OUTPOSTENUMS::OutpostAttacker);
			actionDespawnAllSquads();
			++_FightData._CurrentCombatRound;
			if (_FightData._CurrentCombatRound>=computeRoundCount())
			{
				_FightData._CurrentCombatLevel = 0;
				_FightData._CurrentCombatRound = 0;
				if (won)
				{
					broadcastMessage(OwnerFighters, LastRoundWon);
					broadcastMessage(AttackerFighters, LastRoundLost);
				}
				else
				{
					broadcastMessage(OwnerFighters, LastRoundLost);
					broadcastMessage(AttackerFighters, LastRoundWon);
				}
				if (_FightData._MaxDefenseLevel >= _FightData._MaxAttackLevel)
				{
					broadcastMessage(OwnerGuild, DefenseSucceeded);
					broadcastMessage(AttackerGuild, DefenseSucceeded);
				}
				else
				{
					broadcastMessage(OwnerGuild, DefenseFailed);
					broadcastMessage(AttackerGuild, DefenseFailed);
				}
				actionPostNextState(DefenseAfter);
			}
			else
			{
				if (won)
				{
					broadcastMessage(OwnerFighters, RoundWon);
					broadcastMessage(AttackerFighters, RoundLost);
				}
				else
				{
					broadcastMessage(OwnerFighters, RoundLost);
					broadcastMessage(AttackerFighters, RoundWon);
				}
				actionPostNextState(DefenseRound);
			}
		} break;
		case Timer1End: // Spawn a new squad
		{
			// Index of the squad to spawn
			if (_FightData._SpawnedSquadsB < computeSquadCountB(_FightData._CurrentCombatLevel))
			{
				uint32 squadIndex = _FightData._SpawnedSquadsB;
				actionBuySquadsB(squadIndex, OUTPOSTENUMS::OutpostAttacker);
				actionSpawnSquadsB(squadIndex, OUTPOSTENUMS::OutpostAttacker);
//				++_FightData._SpawnedSquadsB;
			}
			if (_FightData._SpawnedSquadsB < computeSquadCountB(_FightData._CurrentCombatLevel))
				actionSetTimer1(computeSpawnDelay(_FightData._CurrentCombatLevel));
		} break;
//		case SquadKilled: // Increment dead squad count
//		{
//			++_FightData._KilledSquads;
//		} break;
		case EndOfState:
		{
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	case DefenseAfter: { switch (event) {
		case StartOfState:
		{
			OUTPOST_INF("Outpost %s: Starting defense phase (after fight)", _Name.c_str());
			// Here we may shorten the delay
			if (OutpostStateTimeOverride>(uint32)0)
				actionSetTimer0(OutpostStateTimeOverride);
			else
				actionSetTimer0(computeTimeAfterAttack());
		} break;
		case Timer0End:
		{
			if (_FightData._MaxAttackLevel > _FightData._MaxDefenseLevel)
			{
				_CurrentOutpostLevel = _FightData._MaxAttackLevel;
				actionChangeOwner();
			}
			actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		case EndOfState:
		{
		} break;
		default:
			eventException(event, eventParams);
	} } break;
/****************************************************************************/
	default:
		eventException(event, eventParams);
	}
	
	// update the main outpost properties in client database
	askOutpostDBUpdate();
	askGuildDBUpdate(COutpostGuildDBUpdater::OUTPOST_PROPERTIES);
}

//----------------------------------------------------------------------------
// :NOTE: I saved some copy and paste to ease maintaining, but duplicated case
// should be separated without worry if needed.
void COutpost::eventException(OUTPOSTENUMS::TOutpostEvent event, void* eventParams)
{
	using namespace OUTPOSTENUMS;
	switch (_State)
	{
/****************************************************************************/
	case Peace: { switch (event) {
		case OwnerVanished:
		case OwnerGiveUp:
		{
			actionStopTimer0();
			setOwnerGuild(0);
			resetDefaultAttackSquads();
			_CurrentOutpostLevel = computeTribeOutpostLevel();
		} break;
		case EventAisUp:
		case EventAisDown:
		{
		} break;
		default:
			OUTPOST_WRN("Undhandled event %s in outpost state %s", OUTPOSTENUMS::toString(event).c_str(), OUTPOSTENUMS::toString(_State).c_str());
		}
	} break;
/****************************************************************************/
	case WarDeclaration:
	case AttackBefore: { switch (event) {
		case OwnerVanished:
		case OwnerGiveUp:
		{
			actionStopTimer0();
			actionChangeOwner();
		//	resetDefaultAttackSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State, true, false);
		} break;
		case EventAisUp:
		case EventAisDown:
		{
			if (computeMinimumTimeToNextFight()<OutpostRangeForCancelOnReset)
			{
				_CrashHappened = true;
				actionPayBackMoneySpent();
				actionStopTimer0();
				actionPostNextState(Peace);
				actionCancelOutpostChallenge();
			}
		//	tellEventState(event, _State);
		} break;
		case AttackerVanished:
		case AttackerGiveUp:
		{
			actionStopTimer0();
			actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		default:
			OUTPOST_WRN("Undhandled event %s in outpost state %s", OUTPOSTENUMS::toString(event).c_str(), OUTPOSTENUMS::toString(_State).c_str());
		}
	} break;
/****************************************************************************/
	case AttackRound: { switch (event) {
		case OwnerVanished:
		case OwnerGiveUp:
		{
			actionStopTimer1();
			actionStopTimer0();
			// End round (here we only care if max level changes
			if ((_FightData._SpawnedSquadsA+_FightData._SpawnedSquadsB - _FightData._KilledSquads)==0)
			{
				++_FightData._CurrentCombatLevel;
				if (_FightData._CurrentCombatLevel > _FightData._MaxAttackLevel)
					_FightData._MaxAttackLevel = _FightData._CurrentCombatLevel;
			}
			if (event != OwnerVanished)
				actionPayBackAliveSquads(OUTPOSTENUMS::OutpostOwner);
			actionDespawnAllSquads();
			_CurrentOutpostLevel = _FightData._MaxAttackLevel;
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			actionChangeOwner();
		//	resetDefaultAttackSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State, true, false);
		} break;
		case EventAisUp:
		case EventAisDown:
		{
			_CrashHappened = true;
			actionPayBackMoneySpent();
			actionStopTimer1();
			actionStopTimer0();
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			actionDespawnAllSquads();
			actionPostNextState(Peace);
			actionCancelOutpostChallenge();
		//	tellEventState(event, _State);
		} break;
		case AttackerVanished:
		case AttackerGiveUp:
		{
			actionStopTimer1();
			actionStopTimer0();
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			actionPayBackAliveSquads(OUTPOSTENUMS::OutpostOwner);
			actionDespawnAllSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		default:
			OUTPOST_WRN("Undhandled event %s in outpost state %s", OUTPOSTENUMS::toString(event).c_str(), OUTPOSTENUMS::toString(_State).c_str());
		}
	} break;
/****************************************************************************/
	case AttackAfter:
	case DefenseBefore: { switch (event) {
		case OwnerVanished:
		case OwnerGiveUp:
		{
			actionStopTimer0();
			_CurrentOutpostLevel = _FightData._MaxAttackLevel;
			actionChangeOwner();
		//	resetDefaultAttackSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State, true, false);
		} break;
		case EventAisUp:
		case EventAisDown:
		{
			if (computeMinimumTimeToNextFight()<OutpostRangeForCancelOnReset)
			{
				_CrashHappened = true;
				actionPayBackMoneySpent();
				actionStopTimer0();
				actionPostNextState(Peace);
				actionCancelOutpostChallenge();
			}
		} break;
		case AttackerVanished:
		case AttackerGiveUp:
		{
			actionStopTimer0();
			actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		default:
			OUTPOST_WRN("Undhandled event %s in outpost state %s", OUTPOSTENUMS::toString(event).c_str(), OUTPOSTENUMS::toString(_State).c_str());
		}
	} break;
/****************************************************************************/
	case DefenseRound: { switch (event) {
		case OwnerVanished:
		case OwnerGiveUp:
		{
			actionStopTimer1();
			actionStopTimer0();
			actionPayBackAliveSquads(OUTPOSTENUMS::OutpostAttacker);
			actionDespawnAllSquads();
			_CurrentOutpostLevel = _FightData._MaxAttackLevel;
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			actionChangeOwner();
		//	resetDefaultAttackSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State, true, false);
		} break;
		case EventAisUp:
		case EventAisDown:
		{
			_CrashHappened = true;
			actionPayBackMoneySpent();
			actionStopTimer1();
			actionStopTimer0();
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			actionDespawnAllSquads();
			actionPostNextState(Peace);
			actionCancelOutpostChallenge();
		} break;
		case AttackerVanished:
		case AttackerGiveUp:
		{
			actionStopTimer1();
			actionStopTimer0();
			_FightData._CurrentCombatLevel = 0;
			_FightData._CurrentCombatRound = 0;
			if (event != AttackerVanished)
				actionPayBackAliveSquads(OUTPOSTENUMS::OutpostAttacker);
			actionDespawnAllSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		default:
			OUTPOST_WRN("Undhandled event %s in outpost state %s", OUTPOSTENUMS::toString(event).c_str(), OUTPOSTENUMS::toString(_State).c_str());
		}
	} break;
/****************************************************************************/
	case DefenseAfter: { switch (event) {
		case OwnerVanished:
		case OwnerGiveUp:
		{
			actionStopTimer0();
			_CurrentOutpostLevel = _FightData._MaxAttackLevel;
			actionChangeOwner();
		//	resetDefaultAttackSquads();
			actionPostNextState(Peace);
		//	tellEventState(event, _State, true, false);
		} break;
		case EventAisUp:
		case EventAisDown:
		{
		} break;
		case AttackerVanished:
		case AttackerGiveUp:
		{
			actionStopTimer0();
			actionPostNextState(Peace);
		//	tellEventState(event, _State);
		} break;
		default:
			OUTPOST_WRN("Undhandled event %s in outpost state %s", OUTPOSTENUMS::toString(event).c_str(), OUTPOSTENUMS::toString(_State).c_str());
		}
	} break;
/****************************************************************************/
	default:
		OUTPOST_WRN("Undefined state %s in outpost", OUTPOSTENUMS::toString(event).c_str());
	}

	// update the main outpost properties in client database
	askOutpostDBUpdate();
	askGuildDBUpdate(COutpostGuildDBUpdater::OUTPOST_PROPERTIES);
}
