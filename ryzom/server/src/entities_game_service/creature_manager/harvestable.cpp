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

#include "harvestable.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "game_item_manager/player_inv_temp.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

extern CPlayerManager	PlayerManager;
extern NLMISC::CRandom	RandomGenerator;


extern CVariable<bool> VerboseQuartering;
std::string CurrentCreatureSpawningDebug;

//--------------------------------------------------------------
//						Constructor
//--------------------------------------------------------------
CHarvestable::CHarvestable()
{
	_HarvestSkill = SKILLS::unknown;
	_LootSlotCount = 0;
} // Constructor //



//--------------------------------------------------------------
//						Destructor
//--------------------------------------------------------------
CHarvestable::~CHarvestable()
{
	if ( TheDataset.isAccessible(_HarvesterRowId) )
	{
		CEntityId harvesterId = TheDataset.getEntityId(_HarvesterRowId);

	/*	string msgName = "OPS_HARVEST_INTERRUPTED";

		CMessage msg("STATIC_STRING");
		msg.serial( harvesterId );
		set<CEntityId> excluded;
		msg.serialCont( excluded );
		msg.serial( msgName );
		sendMessageViaMirror ("IOS", msg);
*/
		CCharacter *harvester = PlayerManager.getChar(_HarvesterRowId);
		if (harvester)
			harvester->endHarvest();
		else
		{
			nlwarning("CHarvestable::~CHarvestable> WARNING harvester row id not reset but can't find corresponding entity.");
#ifdef NL_DEBUG
			nlstop;
#endif
		}
	}
} // Destructor //


//--------------------------------------------------------------
//						removeMp()  
//--------------------------------------------------------------
void CHarvestable::removeMp( uint32 mpIndex, uint16 quantity )
{
	if ( mpIndex >= _Mps.size() )
	{
		nlwarning("<CHarvestable::removeMp> Invalid mp index %u, there is only %u mps", mpIndex, _Mps.size() );
		return;
	}
	
	_Mps[ mpIndex ].Quantity = ( _Mps[ mpIndex ].Quantity > quantity) ? (_Mps[ mpIndex ].Quantity-quantity) : 0;

	// if a player has been specified, update his database with the new quantity
	if ( TheDataset.isAccessible(_HarvesterRowId) )
	{
		CCharacter *harvester = PlayerManager.getChar( _HarvesterRowId );
		if (harvester)
		{
			CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)harvester->getInventory(INVENTORIES::temporary);
			nlassert( invTemp );
			invTemp->setDispQuantity(_LootSlotCount+mpIndex, _Mps[ mpIndex ].Quantity);
		}
	}
} // removeMp //


//--------------------------------------------------------------
//						writeMpInfos()  
//--------------------------------------------------------------
bool CHarvestable::writeMpInfos()
{
	if ( !TheDataset.isAccessible(_HarvesterRowId) )
		return false;

	CCharacter *harvester = PlayerManager.getChar( _HarvesterRowId );
	if (harvester == NULL)
		return false;

	// If the player didn't close the temp inventory for a forage, end forage session
	harvester->endForageSession();

	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)harvester->getInventory(INVENTORIES::temporary);
	invTemp->enterMode(TEMP_INV_MODE::Quarter);

	uint validMps = 0;
	uint nbMp = (uint)_Mps.size();

	// Count number of valid mps
	if (nbMp > INVENTORIES::NbTempInvSlots)
		nbMp = INVENTORIES::NbTempInvSlots;

	for (uint i = 0 ; i < nbMp ; ++i)
	{
		const CCreatureRawMaterial &mp = _Mps[i];
		
		if (mp.ItemId != CSheetId::Unknown)
			if (mp.Quantity > 0)
				++validMps;
	}	

	// Display quantity and sheet id of the mps
	for (uint i = 0; i < invTemp->getSlotCount()-_LootSlotCount; ++i)
	{
		if (i >= nbMp )
		{
			invTemp->setDispQuantity(_LootSlotCount+i, 0);
			continue;
		}
		else
		{
			const CCreatureRawMaterial &mp = _Mps[ i ];

			if ( mp.ItemId != CSheetId::Unknown )
			{
				invTemp->setDispQuantity(_LootSlotCount+i, mp.Quantity);
#if !FINAL_VERSION
				nldebug("Write MP %u, Sheet = %s, qty = %u",i, mp.ItemId.toString().c_str(),mp.Quantity);
#endif
				if (mp.Quantity)
					++validMps;
			}
			else
			{
				invTemp->setDispQuantity(_LootSlotCount+i, 0);
#if !FINAL_VERSION
				nldebug("Write MP %u, Sheet = Unknown, qty = 0",i );
#endif
			}

			if( mp.Quantity > 0 )
				invTemp->setDispSheetId(_LootSlotCount+i, mp.ItemId);
		}
	}

	return (validMps!=0);
} // writeMpInfos //


//--------------------------------------------------------------
//	setMps()   FOR QUARTERING ONLY (NOT FORAGE)
//
// 12/08/2004: New algorithm, replacing the odd 'decreasing quantity' previous one.
//--------------------------------------------------------------
void CHarvestable::setMps( const vector<CStaticCreatureRawMaterial>& mps )
{
	_Mps.clear();

	if( IsRingShard ) 
	{
		if (VerboseQuartering)
			nldebug("QRTR: %s: No quartering in Ring mode", CurrentCreatureSpawningDebug.c_str());
		return;
	}

	if ( mps.empty() )
	{
		if (VerboseQuartering)
			nldebug("QRTR: %s: This creature has no MP", CurrentCreatureSpawningDebug.c_str());
		return;
	}

	// Identify craft, fixed-quantity and total-quantity raw materials
	vector<uint> rmIndices [NbRMUsages];
	for ( uint i=0; i!=mps.size(); ++i )
	{
		rmIndices[mps[i].rmUsage()].push_back( i );
	}

	// First, assign fixed-quantity raw materials
	for ( vector<uint>::const_iterator irm=rmIndices[RMUFixedQuantity].begin(); irm!=rmIndices[RMUFixedQuantity].end(); ++irm )
	{
		uint iSlot = (*irm);

		// Add the RM
		CCreatureRawMaterial rm;
		rm.MpCommon.Name = mps[iSlot].MpCommon.Name;
		rm.MpCommon.AssociatedItemName = mps[iSlot].MpCommon.AssociatedItemName;
		rm.MpCommon.MinQuality = mps[iSlot].MpCommon.MinQuality;
		rm.MpCommon.MaxQuality = mps[iSlot].MpCommon.MaxQuality;
		rm.ItemId = mps[iSlot].ItemId;
		rm.Quantity = (uint16)*QuarteringQuantityByVariable[mps[iSlot].quantityVariable()];
		_Mps.push_back( rm );
	}
	if (VerboseQuartering)
	{
		if (!rmIndices[RMUFixedQuantity].empty())
			nldebug("QRTR: %s: %u fixed quantity", CurrentCreatureSpawningDebug.c_str(), rmIndices[RMUFixedQuantity].size());
	}

	// Then, assign raw materials. The size of the arrays rmIndices[rmUsage] will decrease.
	//for ( uint rmUsage=RMUCraft; rmUsage<=RMUMission; ++rmUsage )
	//{
	if ( ! rmIndices[RMUTotalQuantity].empty() )
	{
		if (VerboseQuartering)
			nldebug("QRTR: %s: %u potential total quantity", CurrentCreatureSpawningDebug.c_str(), rmIndices[RMUTotalQuantity].size());

		// Randomize number of slots and compute each slot average (supports any float average)
		TRMQuantityVariable quantityVariable = (TRMQuantityVariable)mps[rmIndices[RMUTotalQuantity][0]].quantityVariable();
		const float totalQuarteringQuantityAverage = *QuarteringQuantityByVariable[quantityVariable]; // if a creature is in RMTotalQuantity mode, all the RMs have the same quantityVariable() shared for all
		const uint MinFilledSlots = 1;
		uint rndNbSlotsToFill = MinFilledSlots + RandomGenerator.rand( (uint16)rmIndices[RMUTotalQuantity].size() - MinFilledSlots );
		float quantityAveragePerFilledSlot = totalQuarteringQuantityAverage / ((float)rndNbSlotsToFill);
		bool limitTo10PctFromAverage = ((quantityVariable >= RMQVBossBegin) && (quantityVariable <= RMQVBossEnd));
		uint intQuantityAveragePerFilledSlot = (uint)quantityAveragePerFilledSlot;
		float decimalPartQuantityAveragePerFilledSlot = quantityAveragePerFilledSlot - (float)floor( quantityAveragePerFilledSlot );

		// Randomize slots to fill and quantities
		for ( uint i=0; i!=rndNbSlotsToFill; ++i )
		{
			// Select a random slot
			uint iLastSlotIndex = (uint)rmIndices[RMUTotalQuantity].size() - 1;
			uint iSlotIndex = RandomGenerator.rand( iLastSlotIndex ); // index in rmIndices[RMUCraft]
			uint iSlot = rmIndices[RMUTotalQuantity][iSlotIndex]; // index in mps

			// Remove used slot from the rmIndices[RMUCraft] list for subsequent slot randomizations
			if ( iSlotIndex < iLastSlotIndex )
				rmIndices[RMUTotalQuantity][iSlotIndex] = rmIndices[RMUTotalQuantity].back();
			rmIndices[RMUTotalQuantity].pop_back();

			// Calculate the quantity for the selected slot
			uint slotQuantity;
			if ( limitTo10PctFromAverage )
			{
				uint tenth = intQuantityAveragePerFilledSlot / 10;
				slotQuantity = (intQuantityAveragePerFilledSlot-tenth) + RandomGenerator.rand( tenth * 2 );
			}
			else
			{
				slotQuantity = RandomGenerator.rand( intQuantityAveragePerFilledSlot * 2 );
			}
			if ( (decimalPartQuantityAveragePerFilledSlot != 0) &&
				 (RandomGenerator.frand( 1.0f ) < decimalPartQuantityAveragePerFilledSlot) )
				++slotQuantity;

			// Add the RM (except if the calculated quantity is 0)
			if ( slotQuantity != 0 )
			{
				CCreatureRawMaterial rm;
				rm.MpCommon.Name = mps[iSlot].MpCommon.Name;
				rm.MpCommon.AssociatedItemName = mps[iSlot].MpCommon.AssociatedItemName;
				rm.MpCommon.MinQuality = mps[iSlot].MpCommon.MinQuality;
				rm.MpCommon.MaxQuality = mps[iSlot].MpCommon.MaxQuality;
				rm.ItemId = mps[iSlot].ItemId;
				rm.Quantity = (uint16)slotQuantity;
				_Mps.push_back( rm );
				if (VerboseQuartering)
					nldebug("QRTR: %s: %u of %s", CurrentCreatureSpawningDebug.c_str(), slotQuantity, rm.ItemId.toString().c_str());
			}
		}
	}
	//}
}
