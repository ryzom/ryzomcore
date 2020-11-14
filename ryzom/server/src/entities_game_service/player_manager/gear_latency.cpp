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
#include "player_manager/gear_latency.h"
#include "player_manager/character.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CGearLatency);


void CGearLatency::update(CCharacter * user)
{
	nlassert(user);
	bool update = false;
	while( !_GearLatencies.empty() && (*_GearLatencies.begin()).LatencyEndDate < CTickEventHandler::getGameCycle() )
	{
		_GearLatencies.erase( _GearLatencies.begin() );
		update = true;
	}
	if ( update )
	{
		// update client
		if( !_GearLatencies.empty() )
		{
//			user->_PropertyDatabase.setProp( "USER:ACT_TEND", _GearLatencies.back().LatencyEndDate );
			CBankAccessor_PLR::getUSER().setACT_TEND(user->_PropertyDatabase, _GearLatencies.back().LatencyEndDate );
		}
		else
		{
//			user->_PropertyDatabase.setProp( "USER:ACT_TSTART", 0 );
			CBankAccessor_PLR::getUSER().setACT_TSTART(user->_PropertyDatabase, 0 );
//			user->_PropertyDatabase.setProp( "USER:ACT_TEND", 0 );
			CBankAccessor_PLR::getUSER().setACT_TEND(user->_PropertyDatabase, 0 );
//			user->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", 0 );
			CBankAccessor_PLR::getEXECUTE_PHRASE().setSHEET(user->_PropertyDatabase, CSheetId::Unknown );
//			user->_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0 );
			CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(user->_PropertyDatabase, 0 );
		}
	}
}// CGearLatency::update


CVariable<uint32> EquipTimeFactor("egs", "EquipTimeFactor", "", 1, 0, true);

void CGearLatency::setSlot( INVENTORIES::TInventory inventory, uint32 slot, const CStaticItem * form, CCharacter * user )
{
	static NLMISC::CSheetId equipSheet("big_equip_item.sbrick");
	
	// checks must be done by the caller
	nlassert(form);
	nlassert(user);
	// ignore instant equip items
	if ( form->TimeToEquip == 0)
		return;
	
	// build a new entry
	CGearSlot gear;
	if (inventory == INVENTORIES::equipment)
		gear.InHand = false;
	else if (inventory == INVENTORIES::handling)
		gear.InHand = true;
	else
		nlerror("setSlot : Invalid inventory %u ('%s') : must be handling or equipment ",inventory,INVENTORIES::toString(inventory).c_str() );
	gear.Slot = slot;
	gear.LatencyEndDate = (form->TimeToEquip * EquipTimeFactor.get()) + CTickEventHandler::getGameCycle();
	
	// add it in our sorted list
	std::list<CGearSlot>::iterator it = _GearLatencies.begin();
	for (; it != _GearLatencies.end(); ++it)
	{
		if ( (*it).LatencyEndDate >= gear.LatencyEndDate )
			break;
	}
	if ( it == _GearLatencies.end() )
	{
//		user->_PropertyDatabase.setProp( "USER:ACT_TSTART", CTickEventHandler::getGameCycle() );
		CBankAccessor_PLR::getUSER().setACT_TSTART(user->_PropertyDatabase, CTickEventHandler::getGameCycle() );
//		user->_PropertyDatabase.setProp( "USER:ACT_TEND", gear.LatencyEndDate );
		CBankAccessor_PLR::getUSER().setACT_TEND(user->_PropertyDatabase, gear.LatencyEndDate );
//		sint64 tmp = (sint64)user->actionCounter();
//		user->_PropertyDatabase.setProp( "USER:ACT_NUMBER", tmp );
//		user->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", sint64(equipSheet.asInt()) );
		CBankAccessor_PLR::getEXECUTE_PHRASE().setSHEET(user->_PropertyDatabase, equipSheet);
//		user->_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0 );
		CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(user->_PropertyDatabase, 0 );
	}
	_GearLatencies.insert( it, gear );
}// CGearLatency::setSlot


void CGearLatency::unsetSlot( INVENTORIES::TInventory inventory, uint32 slot, CCharacter * user)
{
	nlassert(user);
	bool inHand = false;
	if (inventory == INVENTORIES::handling)
		inHand = true;
	else
		nlassert(inventory == INVENTORIES::equipment);
	
	std::list<CGearSlot>::iterator it = _GearLatencies.begin();
	for (; it != _GearLatencies.end(); ++it)
	{
		if ( (*it).InHand == inHand && (*it).Slot == slot )
		{
			_GearLatencies.erase(it);
			if ( !_GearLatencies.empty() )
			{
//				user->_PropertyDatabase.setProp( "USER:ACT_TEND", _GearLatencies.back().LatencyEndDate );
				CBankAccessor_PLR::getUSER().setACT_TEND(user->_PropertyDatabase, _GearLatencies.back().LatencyEndDate );
			}
			else
			{
//				user->_PropertyDatabase.setProp( "USER:ACT_TSTART", 0 );
				CBankAccessor_PLR::getUSER().setACT_TSTART(user->_PropertyDatabase, 0 );
//				user->_PropertyDatabase.setProp( "USER:ACT_TEND", 0 );
				CBankAccessor_PLR::getUSER().setACT_TEND(user->_PropertyDatabase, 0 );
//				user->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", 0 );
				CBankAccessor_PLR::getEXECUTE_PHRASE().setSHEET(user->_PropertyDatabase, CSheetId::Unknown );
//				user->_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0 );
				CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(user->_PropertyDatabase, 0 );
			}
			break;
		}
	}
}// CGearLatency::unsetSlot



