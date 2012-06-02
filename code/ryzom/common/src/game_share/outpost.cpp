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
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
#include "outpost.h"

using namespace std;
using namespace NLMISC;

namespace OUTPOSTENUMS
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TPVPType)
		NL_STRING_CONVERSION_TABLE_ENTRY(PVE)
		NL_STRING_CONVERSION_TABLE_ENTRY(PVP)
		NL_STRING_CONVERSION_TABLE_ENTRY(RVR)
		NL_STRING_CONVERSION_TABLE_ENTRY(Full)
		NL_STRING_CONVERSION_TABLE_ENTRY(UnknownPVPType)
	NL_END_STRING_CONVERSION_TABLE(TPVPType, StaticCOutpostTPVPTypeConversion, UnknownPVPType)

	NL_BEGIN_STRING_CONVERSION_TABLE (TPVPSide)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostOwner)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostAttacker)
		NL_STRING_CONVERSION_TABLE_ENTRY(UnknownPVPSide)
	NL_END_STRING_CONVERSION_TABLE(TPVPSide, StaticCOutpostTPVPSideConversion, UnknownPVPSide)

	NL_BEGIN_STRING_CONVERSION_TABLE (TOutpostState)
		NL_STRING_CONVERSION_TABLE_ENTRY(Peace)
		NL_STRING_CONVERSION_TABLE_ENTRY(WarDeclaration)
		NL_STRING_CONVERSION_TABLE_ENTRY(AttackBefore)
		NL_STRING_CONVERSION_TABLE_ENTRY(AttackRound)
		NL_STRING_CONVERSION_TABLE_ENTRY(AttackAfter)
		NL_STRING_CONVERSION_TABLE_ENTRY(DefenseBefore)
		NL_STRING_CONVERSION_TABLE_ENTRY(DefenseRound)
		NL_STRING_CONVERSION_TABLE_ENTRY(DefenseAfter)
		NL_STRING_CONVERSION_TABLE_ENTRY(UnknownOutpostState)
	NL_END_STRING_CONVERSION_TABLE(TOutpostState, StaticCOutpostStateConversion, UnknownOutpostState)

	NL_BEGIN_STRING_CONVERSION_TABLE (TOutpostEvent)
		NL_STRING_CONVERSION_TABLE_ENTRY(StartOfState)
		NL_STRING_CONVERSION_TABLE_ENTRY(EndOfState)
		NL_STRING_CONVERSION_TABLE_ENTRY(Challenged)
		NL_STRING_CONVERSION_TABLE_ENTRY(OwnerVanished)
		NL_STRING_CONVERSION_TABLE_ENTRY(AttackerVanished)
		NL_STRING_CONVERSION_TABLE_ENTRY(Timer0End)
		NL_STRING_CONVERSION_TABLE_ENTRY(Timer1End)
		NL_STRING_CONVERSION_TABLE_ENTRY(Timer2End)
		NL_STRING_CONVERSION_TABLE_ENTRY(AttackerGiveUp)
		NL_STRING_CONVERSION_TABLE_ENTRY(OwnerGiveUp)
		NL_STRING_CONVERSION_TABLE_ENTRY(SquadKilled)
		NL_STRING_CONVERSION_TABLE_ENTRY(EventAisUp)
		NL_STRING_CONVERSION_TABLE_ENTRY(EventAisDown)
		NL_STRING_CONVERSION_TABLE_ENTRY(UnknownOutpostEvent)
	NL_END_STRING_CONVERSION_TABLE(TOutpostEvent, StaticOutpostEventConversion, UnknownOutpostEvent)

	NL_BEGIN_STRING_CONVERSION_TABLE (TSquadType)
		NL_STRING_CONVERSION_TABLE_ENTRY(Default)
		NL_STRING_CONVERSION_TABLE_ENTRY(Recruited)
		NL_STRING_CONVERSION_TABLE_ENTRY(Mercenary)
		NL_STRING_CONVERSION_TABLE_ENTRY(UnknownSquadType)
	NL_END_STRING_CONVERSION_TABLE(TSquadType, StaticCOutpostSquadTypeConversion, UnknownSquadType)

	NL_BEGIN_STRING_CONVERSION_TABLE (TSquadState)
		NL_STRING_CONVERSION_TABLE_ENTRY(NotCreated)
		NL_STRING_CONVERSION_TABLE_ENTRY(NotReady)
		NL_STRING_CONVERSION_TABLE_ENTRY(NotSpawned)
		NL_STRING_CONVERSION_TABLE_ENTRY(Spawning)
		NL_STRING_CONVERSION_TABLE_ENTRY(Spawned)
		NL_STRING_CONVERSION_TABLE_ENTRY(Dead)
		NL_STRING_CONVERSION_TABLE_ENTRY(UnknownSquadState)
	NL_END_STRING_CONVERSION_TABLE(TSquadState, StaticCOutpostSquadStateConversion, UnknownSquadState)

	NL_BEGIN_STRING_CONVERSION_TABLE (TSpecialOutpostEvent)
		NL_STRING_CONVERSION_TABLE_ENTRY(PeaceStateBegin)
		NL_STRING_CONVERSION_TABLE_ENTRY(PeaceStateEnd)
		NL_STRING_CONVERSION_TABLE_ENTRY(TribeOwnershipBegin)
		NL_STRING_CONVERSION_TABLE_ENTRY(TribeOwnershipEnd)
		NL_STRING_CONVERSION_TABLE_ENTRY(GuildOwnershipBegin)
		NL_STRING_CONVERSION_TABLE_ENTRY(GuildOwnershipEnd)
		NL_STRING_CONVERSION_TABLE_ENTRY(StateChanged)
		NL_STRING_CONVERSION_TABLE_ENTRY(OwnerChanged)
		NL_STRING_CONVERSION_TABLE_ENTRY(AttackerChanged)
	NL_END_STRING_CONVERSION_TABLE(TSpecialOutpostEvent, StaticSpecialOutpostEventConversion, UnknownSpecialOutpostEvent)

	std::string const& toString(TPVPType val) { return StaticCOutpostTPVPTypeConversion.toString(val); }
	TPVPType toPVPType(std::string const& val) { return StaticCOutpostTPVPTypeConversion.fromString(val); }
	std::string const& toString(TPVPSide val) { return StaticCOutpostTPVPSideConversion.toString(val); }
	TPVPSide toPVPSide(std::string const& val) { return StaticCOutpostTPVPSideConversion.fromString(val); }
	std::string const& toString(TOutpostState val) { return StaticCOutpostStateConversion.toString(val); }
	TOutpostState toOutpostState(std::string const& val) { return StaticCOutpostStateConversion.fromString(val); }
	std::string const& toString(TOutpostEvent val) { return StaticOutpostEventConversion.toString(val); }
	TOutpostEvent toOutpostEvent(std::string const& val) { return StaticOutpostEventConversion.fromString(val); }
	std::string const& toString(TSquadType val) { return StaticCOutpostSquadTypeConversion.toString(val); }
	TSquadType toSquadType(std::string const& val) { return StaticCOutpostSquadTypeConversion.fromString(val); }
	std::string const& toString(TSquadState val) { return StaticCOutpostSquadStateConversion.toString(val); }
	TSquadState toSquadState(std::string const& val) { return StaticCOutpostSquadStateConversion.fromString(val); }
	std::string const& toString(TSpecialOutpostEvent val) { return StaticSpecialOutpostEventConversion.toString(val); }
	TSpecialOutpostEvent toSpecialOutpostEvent(std::string const& val) { return StaticSpecialOutpostEventConversion.fromString(val); }

}
