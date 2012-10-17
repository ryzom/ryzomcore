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
#include "modifiers_in_db.h"
#include "player_manager/character.h"

const uint8 NbBonusModifiers				= 12;
const uint8 NbMalusModifiers				= 12;

using namespace std;
using namespace NLMISC;

CModifierInDB::CModifierInDB()
{
	init();
}

void CModifierInDB::clear()
{
	ActivationDate= 0;
	Disabled = false;
	SheetId = NLMISC::CSheetId::Unknown;
}

void CModifierInDB::init()
{
	clear();
}

void CModifierInDB::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Disabled);
	if(Disabled) 
	{
		f.serial(ActivationDate);
		f.serial(SheetId);
	}		
}


CModifiersInDB::CModifiersInDB()
{
	Bonus.resize(NbBonusModifiers);
	Malus.resize(NbMalusModifiers);
	clear();
}

void CModifiersInDB::clear()
{
	for (uint32 i=0;i<Bonus.size();++i)
		Bonus[i].clear();

	for (uint32 i=0;i<Malus.size();++i)
		Malus[i].clear();
}	

void CModifiersInDB::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(Bonus);
	f.serialCont(Malus);
	// let s be sure that the vector have the right size
	if ( f.isReading() )
	{
		if ( Bonus.size() != NbBonusModifiers )
		{
			nlwarning("BUG: number of bonus modifier is not %d but is %d", NbBonusModifiers, Bonus.size());
			Bonus.resize(NbBonusModifiers);
		}
		if ( Malus.size() != NbMalusModifiers )
		{	
			nlwarning("BUG: number of Malus modifier is not %d but is %d", NbMalusModifiers, Malus.size());
			Malus.resize(NbMalusModifiers);
		}
	}
}

void CModifiersInDB::writeInDatabase(CCDBSynchronised &database)
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	for (uint i = 0 ; i < NbBonusModifiers ; ++i)
	{
		if (Bonus[i].Disabled && Bonus[i].SheetId != NLMISC::CSheetId::Unknown && Bonus[i].ActivationDate > time)
		{
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Sheet[i], Bonus[i].SheetId.asInt());
			CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(i).setSHEET(database, Bonus[i].SheetId);
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Disable[i], 1);
			CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(i).setDISABLED(database, true);
		}
	}
	for (uint i = 0 ; i < NbMalusModifiers ; ++i)
	{
		if (Malus[i].Disabled && Malus[i].SheetId != NLMISC::CSheetId::Unknown && Malus[i].ActivationDate > time)
		{
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Sheet[i], Malus[i].SheetId.asInt());
			CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(i).setSHEET(database, Malus[i].SheetId);
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Disable[i], 1);
			CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(i).setDISABLED(database, true);
		}
	}
}

void CModifiersInDB::update(CCDBSynchronised &database)
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	for (uint i = 0 ; i < NbBonusModifiers ; ++i)
	{
		if (Bonus[i].Disabled && Bonus[i].SheetId != NLMISC::CSheetId::Unknown && Bonus[i].ActivationDate <= time)
		{
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Sheet[i], 0);
			CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(i).setSHEET(database, CSheetId::Unknown);
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Disable[i], 0);
			CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(i).setDISABLED(database, false);
			Bonus[i].init();
		}
	}
	for (uint i = 0 ; i < NbMalusModifiers ; ++i)
	{
		if (Malus[i].Disabled && Malus[i].SheetId != NLMISC::CSheetId::Unknown && Malus[i].ActivationDate <= time)
		{
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Sheet[i], 0);
			CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(i).setSHEET(database, CSheetId::Unknown);
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Disable[i], 0);
			CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(i).setDISABLED(database, false);
			Malus[i].init();
		}
	}
}

sint8 CModifiersInDB::addEffect(const NLMISC::CSheetId &sheetId, bool bonus, CCDBSynchronised &database)
{
	if (sheetId == NLMISC::CSheetId::Unknown)
		return (sint8)-1;
	
	std::vector<CModifierInDB> &modifiers = bonus ? Bonus : Malus;

	// if same effect sheetid is found disabled, enable it, otherwise take first empty slot
	sint8 freeSlot = -1;
	for (uint i = 0 ; i < 12 ; ++i)
	{
		if (modifiers[i].SheetId == sheetId)
		{
			freeSlot = i;
			break;
		}
		else if ( freeSlot == -1 && modifiers[i].SheetId == NLMISC::CSheetId::Unknown)
		{
			freeSlot = i;
		}
	}

	if (freeSlot != -1)
	{
		modifiers[freeSlot].SheetId = sheetId;
		if(bonus)
		{
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Sheet[freeSlot], sheetId.asInt() );
			CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(freeSlot).setSHEET(database, sheetId );
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Disable[freeSlot], 0);
			CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(freeSlot).setDISABLED(database, false);
		}
		else
		{
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Sheet[freeSlot], sheetId.asInt() );
			CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(freeSlot).setSHEET(database, sheetId );
//			database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Disable[freeSlot], 0);
			CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(freeSlot).setDISABLED(database, false);
		}
	}

	return freeSlot;
}

void CModifiersInDB::removeEffect(uint8 index, bool bonus, CCDBSynchronised &database)
{
	if ( (bonus && index >= NbBonusModifiers) || (!bonus && index >= NbMalusModifiers) )
		return;
	
	std::vector<CModifierInDB> &modifiers = bonus ? Bonus : Malus;
	modifiers[index].init();

	if(bonus)
	{
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Sheet[index], 0 );
		CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(index).setSHEET(database, CSheetId::Unknown );
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Disable[index], 0);
		CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(index).setDISABLED(database, false);
	}
	else
	{
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Sheet[index], 0 );
		CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(index).setSHEET(database, CSheetId::Unknown);
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Disable[index], 0);
		CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(index).setDISABLED(database, false);
	}
}

void CModifiersInDB::disableEffect(uint8 index, bool bonus, NLMISC::TGameCycle activationDate, CCDBSynchronised &database)
{
	if ( (bonus && index >= NbBonusModifiers) || (!bonus && index >= NbMalusModifiers) )
		return;
	
	std::vector<CModifierInDB> &modifiers = bonus ? Bonus : Malus;
	const std::string type = bonus ? "BONUS:" : "MALUS:";

	modifiers[index].Disabled = true;
	modifiers[index].ActivationDate = activationDate;
	
	if(bonus)
	{
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.Disable[index], 1);
		CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(index).setDISABLED(database, true);
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Bonus.DisableTime[index], activationDate);
		CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(index).setDISABLED_TIME(database, activationDate);
	}
	else
	{
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.Disable[index], 1);
		CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(index).setDISABLED(database, true);
//		database.setProp( CCharacter::getDataIndexReminder()->Modifiers.Malus.DisableTime[index], activationDate);
		CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(index).setDISABLED_TIME(database, activationDate);
	}
}

void CModifiersInDB::_addBonus(const CModifierInDB& bonus)
{
	// check whether the bonus already exists in the bonus vector
	for (uint i = 0 ; i < NbBonusModifiers ; ++i)
	{
		if (Bonus[i].ActivationDate==bonus.ActivationDate && Bonus[i].SheetId==bonus.SheetId)
		{
			return;
		}
	}

	// look for a free slot to store this bonus in
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	for (uint i = 0 ; i < NbBonusModifiers ; ++i)
	{
		if (Bonus[i].ActivationDate==0)
		{
			Bonus[i].ActivationDate= bonus.ActivationDate;
			Bonus[i].SheetId= bonus.SheetId;
			return;
		}
	}

	// if we're here it's cos there were no free slots
	STOP("No free slots found to store bonus effect in")
}

void CModifiersInDB::_addMalus(const CModifierInDB& malus)
{
	// check whether the penalty already exists in the penalty vector
	for (uint i = 0 ; i < NbMalusModifiers ; ++i)
	{
		if (Malus[i].ActivationDate==malus.ActivationDate && Malus[i].SheetId==malus.SheetId)
		{
			return;
		}
	}

	// look for a free slot to store this penalty in
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	for (uint i = 0 ; i < NbMalusModifiers ; ++i)
	{
		if (Malus[i].ActivationDate==0)
		{
			Malus[i].ActivationDate= malus.ActivationDate;
			Malus[i].SheetId= malus.SheetId;
			return;
		}
	}

	// if we're here it's cos there were no free slots
	STOP("No free slots found to store penalty effect in")
}

