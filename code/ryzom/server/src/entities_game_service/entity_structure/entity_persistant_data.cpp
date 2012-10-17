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

#include "entity_persistant_data.h"


///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

//---------------------------------------------------
// serial: serial persistant part of CEntityBase for player character
//---------------------------------------------------
void CEntityBasePersistantData::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
}

//---------------------------------------------------
// serialXml: serial persistant part of CEntityBase for player character
//---------------------------------------------------
void CEntityBasePersistantData::serialXml(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.xmlPush("CEntityBasePart");
	f.xmlPush("CEntityBasePartVersion");
	// serial version
	uint16 version;
	if ( !f.isReading() )
		version = getCurrentVersion();
	f.serial(version);
	f.xmlPop();

	f.xmlPush("PositionsAndDirection");
	f.serial( _EntityState );
	f.xmlPop();

	f.xmlPush("SheetId");
	_SheetId.serialRTWM( f );
	f.xmlPop();

	if (version <= 5)
	{
		f.xmlPush("NameAndSurname");
		f.serial( _Name );
		ucstring tmp;
		f.serial( tmp);
	}
	else
	{
		f.xmlPush("Name");
		f.serial( _Name );
	}
	f.xmlPop();

	f.xmlPush("Race_gender_size");
	if (f.isReading())
	{
		if ( version < 3)
			f.serialEnum( _Race );
		else
		{
			string race;
			f.serial(race);
			_Race = EGSPD::CPeople::fromString(race);
		}
	}
	else
	{
		string race = EGSPD::CPeople::toString(_Race);
		f.serial(race);
	}
	f.serial( _Gender );
	f.serial( _Size );
	f.xmlPop();

	f.xmlPush("Characteristics");
	f.serial( _PhysCharacs );
	f.xmlPop();

	f.xmlPush("Scores");
	f.serial( _PhysScores );
	f.xmlPop();

	f.xmlPush("Skills");
	f.serial( _Skills );
	f.xmlPop();
/*
	f.xmlPush("Items");
	f.serial( _Items );
	f.xmlPop();
*/
	f.xmlPush("ActiveSheath_NBSheath");
	uint8 nbSheath;	f.serial( nbSheath );
	uint8 activeSheath;	f.serial( activeSheath );
	f.xmlPop();

//Deprecated
/*
	f.xmlPush("Role");
	f.serial( Role );
	f.xmlPop();

	f.xmlPush("ActiveRoleLevel_ActiveRole_BestRole_BestRoleLevel");
	f.serial( _ActiveRoleLevel );
	f.serial( _ActiveRole );
	_BestRole.serialRTWM( f );
	_BestRoleLevel.serialRTWM( f );
	f.xmlPop();
*/
	f.xmlPush("SpecialModifier_GlobalSkillModifier");
	f.serial( _SpecialModifiers );
	if ( version < 4 )
	{
		sint32 dummy;
		f.serial( dummy );
	}
	f.xmlPop();

	if ( f.isReading() )
	{
		if (version >= 2)
		{
			f.xmlPush("Defense");
			f.serial( _DodgeAsDefense );
			string slot;
			f.serial( slot );
			_ProtectedSlot = SLOT_EQUIPMENT::stringToSlotEquipment(slot);
			f.xmlPop();
		}
		else
		{
			_DodgeAsDefense = true;
			_ProtectedSlot = SLOT_EQUIPMENT::UNDEFINED;
		}
	}
	else
	{
		f.xmlPush("Defense");
		f.serial( _DodgeAsDefense );
		string slot = SLOT_EQUIPMENT::toString(_ProtectedSlot);
		f.serial( slot );
		f.xmlPop();
	}

	f.xmlPop();
}

