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

/** @page outpost

The new outpost system is defined by the document outpost-v2.doc.

*/

#ifndef RY_OUTPOSTENUMS_H
#define RY_OUTPOSTENUMS_H

#include "nel/misc/types_nl.h"

namespace OUTPOSTENUMS
{
	const uint32 MAX_OUTPOST = 16;						// Same as (#x) in the database SERVER:GUILD:OUTPOST:O#x:S see (database.xml)

	const uint32 OUTPOST_MAX_SPAWN_ZONE = 16;			// Same as (#y) in the database SERVER:GUILD:OUTPOST:O#x:SPAWN_ZONE:#y see (database.xml)
	const uint32 OUTPOST_MAX_SQUAD_SHOP = 16;			// Same as (#y) in the database SERVER:GUILD:OUTPOST:O#x:SQUAD_SHOP:#y see (database.xml)

	const uint32 OUTPOST_NB_SQUAD_SLOTS = 12;
	const uint32 OUTPOST_MAX_SQUAD_TRAINING = OUTPOST_NB_SQUAD_SLOTS * 2;	// Same as (#y) in the database SERVER:GUILD:OUTPOST:O#x:S:T#y see (database.xml)
	const uint32 OUTPOST_MAX_SQUAD_SPAWNED = OUTPOST_MAX_SQUAD_TRAINING;	// Same as (#y) in the database SERVER:GUILD:OUTPOST:O#x:S:S#y see (database.xml)

	const uint32 OUTPOST_MAX_BUILDINGS = 4;				// Same as (#y) in the database SERVER:GUILD:OUTPOST:O#x:BUILDINGS:#y see (database.xml)


	// :NOTE: All enums fisrt entry is the undefined state. Uninitialized
	// enums would be created in that state, which is invalid. It will
	// consistently take the value 0.

	/// PVP type of the outpost
	enum TPVPType
	{
		UnknownPVPType,
		PVE,	// can only be attacked if the outpost is held by a tribe and if the attacking guild comes from the same continent as the outpost
		PVP,	// can only be attacked if the attacking guild comes from the same continent as the outpost
		RVR,	// can only be attacked if the attacking guild comes from another continent as the outpost
		Full	// same as RVR but cant be set by the high council
	};

	enum TPVPSide
	{
		OutpostOwner = 0,
		OutpostAttacker = 1,
		UnknownPVPSide
	};

	// :NOTE: The outpost state machine is defined by a list of possible
	// states for the outpost, a list of possible events, a list of possible
	// actions (which can change the state of the outpost), and some event
	// handlers. It is possible to associate an event handler to a state/event
	// pair. An event handler can call some actions, which may affect the
	// current state of the outpost or some internal variables of it.

	/// current state of the outpost
	enum TOutpostState
	{
		UnknownOutpostState,
		Peace,			// nothing happens
		WarDeclaration,	// a guild declared war on the outpost
		AttackBefore,	// the delay between war declaration and war has elapsed. Attacker tries to take the outpost
		AttackRound,	// ...
		AttackAfter,
		DefenseBefore,	// the attack was successful. The attacker must now defend the outpost
		DefenseRound,	// ...
		DefenseAfter
	};

	/// events that can affect outpost state
	enum TOutpostEvent
	{
		UnknownOutpostEvent,
		StartOfState,		// this is when the state starts
		EndOfState,			// this is when the state ends
		Challenged,			// the outpost was challenged
		OwnerVanished,		// the owner guild vanished
		AttackerVanished,	// the attacker guild vanished
		Timer0End,			// the current state timer reached its end, there may
		Timer1End,			//   be 3 concurrent timers, one for the phase, one
		Timer2End,			//   for the combat round, and one for the squad spawn
		AttackerGiveUp,		// the attack guild gave up the attack
		OwnerGiveUp,		// the owner guild gave up the outpost
		SquadKilled,		// a defending squad was killed
		EventAisUp,			// the ais of the outpost started (or we did)
		EventAisDown,		// the ais of the outpost stopped
//		AllSquadsKilled,	// all defending squads were killed
	};

	/// type of the squad
	enum TSquadType
	{
		UnknownSquadType,
		Default,	// squad recruited automatically when the outpost is taken
		Recruited,	// squad recruited by the guild
		Mercenary	// a special mercenary squad
	};

	/// current state of a building used for database
	enum TOutpostBuildingState
	{
		UnknownOutpostBuildingState,
		BuildingInPlace,		// In place (constructed)
		BuildingConstructing	// Constructing
	};

	/// current state of a state
	enum TSquadState
	{
		UnknownSquadState,
		NotCreated,	// squad has not been created in AIS
		NotReady,	// squad is not ready to be spawned
		NotSpawned,	// squad is ready and waiting to be spawned
		Spawning,	// spawn order has been issued but not yet confirmed
		Spawned,	// squad is spawned
		Dead		// squad is dead
	};

	/// events that can affect outpost state
	enum TSpecialOutpostEvent
	{
		UnknownSpecialOutpostEvent,
		PeaceStateBegin,		// State of outpost is now peace
		PeaceStateEnd,			// State of outpost is no more peace
		TribeOwnershipBegin,	// Owner of the outpost is now a tribe (even if not in peace)
		TribeOwnershipEnd,		// Owner of the outpost is no more a tribe
		GuildOwnershipBegin,	// Owner of the outpost is now a guild (even if not in peace)
		GuildOwnershipEnd,		// Owner of the outpost is no more a guild
		StateChanged,			// State of outpost changed
		OwnerChanged,			// Owner of outpost changed
		AttackerChanged			// Attacker of outpost changed
	};

	std::string const& toString(TPVPType val);
	TPVPType toPVPType(std::string const& val);
	std::string const& toString(TPVPSide val);
	TPVPSide toPVPSide(std::string const& val);
	std::string const& toString(TOutpostState val);
	TOutpostState toOutpostState(std::string const& val);
	std::string const& toString(TOutpostEvent val);
	TOutpostEvent toOutpostEvent(std::string const& val);
	std::string const& toString(TSquadType val);
	TSquadType toSquadType(std::string const& val);
	std::string const& toString(TSquadState val);
	TSquadState toSquadState(std::string const& val);
	std::string const& toString(TSpecialOutpostEvent val);
	TSpecialOutpostEvent toSpecialOutpostEvent(std::string const& val);

}

#endif
