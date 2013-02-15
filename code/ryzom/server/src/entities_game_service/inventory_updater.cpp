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
#include "nel/net/unified_network.h"
#include "inventory_updater.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "player_manager/cdb_synchronised.h"
#include "database_plr.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace INVENTORIES;

extern CGenericXmlMsgHeaderManager	GenericMsgManager;


const CInventoryUpdaterForCharacter::TInventoryId CInventoryUpdaterForCharacter::InvIdFromEInventory [INVENTORIES::NUM_ALL_INVENTORY] =
{
	CInventoryCategoryForCharacter::InvalidInvId, //handling
	CInventoryCategoryForCharacter::InvalidInvId, //temporary,					
//	CInventoryCategoryForCharacter::InvalidInvId, //pickup,						
	CInventoryCategoryForCharacter::InvalidInvId, //equipment,					
	CInventoryCategoryForCharacter::Bag,						
	(CInventoryCategoryForCharacter::TInventoryId)(CInventoryCategoryForCharacter::Packers+0),
	(CInventoryCategoryForCharacter::TInventoryId)(CInventoryCategoryForCharacter::Packers+1),
	(CInventoryCategoryForCharacter::TInventoryId)(CInventoryCategoryForCharacter::Packers+2),
	(CInventoryCategoryForCharacter::TInventoryId)(CInventoryCategoryForCharacter::Packers+3),
//	CInventoryCategoryForCharacter::InvalidInvId, //harvest,					
//	CInventoryCategoryForCharacter::InvalidInvId, //bot_gift,					
	CInventoryCategoryForCharacter::InvalidInvId, //NUM_INVENTORY,				
	CInventoryCategoryForCharacter::InvalidInvId, //exchange,					
	CInventoryCategoryForCharacter::InvalidInvId, //exchange_proposition,		
	CInventoryCategoryForCharacter::InvalidInvId, //trading,					
	CInventoryCategoryForCharacter::InvalidInvId, //reward_sharing,				
	CInventoryCategoryForCharacter::InvalidInvId, //guild,						
	CInventoryCategoryForCharacter::Room
};


//CInventoryUpdater *TestInvUpdater = NULL;


/*
 * Constructor
 */
template <class CInventoryCategoryTemplate>
CInventoryUpdater<CInventoryCategoryTemplate>::CInventoryUpdater NL_TMPL_PARAM_ON_METHOD_1(CInventoryCategoryTemplate)()
{
	for ( uint invId=0; invId!=CInvCat::NbInventoryIds; ++invId )
	{
		_ItemInfoVersions[invId].resize( CInvCat::InventoryNbSlots[invId], 0 );
	}
}


/*
 * Constructor
 */
CInventoryUpdaterForCharacter::CInventoryUpdaterForCharacter( CCDBSynchronised *propertyDatabase ) :
	_PropertyDatabasePt(propertyDatabase)
{
	for ( uint invId=0; invId!=CInvCat::NbInventoryIds; ++invId )
	{
		_LastItemInfoVersionsSent[invId].resize( CInvCat::InventoryNbSlots[invId], 0 );
	}
}


/*
 * Constructor
 */
CInventoryUpdaterForGuild::CInventoryUpdaterForGuild()
{}


/*
 * Send all inventory updates to the client.
 *
 * The interface counter need not be sent, it's neither a problem whenever the message arrives before the
 * database counter nor after (if packet is lost).
 */
void		CInventoryUpdaterForCharacter::sendAllUpdates( const NLMISC::CEntityId& destEntityId )
{
	H_AUTO(IU_sendAllUpdates);
	
	// Push message header
	// The first message has a different name, because the client must know that it's the
	// first message to inhibit its oberver callbacks, although it is not garanteed that
	// it's the first message to arrive on the client (see impulsion channels on the FS).
	bool isInitialUpdate = false;
	CBitMemStream bms;
	if ( _PropertyDatabasePt->notSentYet() )
	{
		GenericMsgManager.pushNameToStream( "DB_INIT:INV", bms );
		isInitialUpdate = true;
		//TestInvUpdater = this;
	}
	else
	{
		GenericMsgManager.pushNameToStream( "DB_UPD_INV", bms );
	}

	bool hasContentToSend = fillAllUpdates( bms, isInitialUpdate );

	if ( hasContentToSend || isInitialUpdate ) // the DB_INIT:INV is required by the client for init msg detection
	{
		// Send message
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( const_cast<CEntityId&>(destEntityId) );
		msgout.serialBufferWithSize( (uint8*)bms.buffer(), bms.length() );
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(destEntityId.getDynamicId()), msgout );
	}
}


/*
 * Increment the stored "Info Version" number, only if it's useful.
 */
void		CInventoryUpdaterForCharacter::incInfoVersion( TInventoryId invId, uint slotIndex )
{
	// Increment the info version number only if equal to the last info version sent to the client
	// else it's not necessary as the client has not still asked for a new item info for this slot
	// This corrects a possible bug if the EGS does 256 changes while the client doesn't ask for info
	if ( _ItemInfoVersions[invId][slotIndex] == _LastItemInfoVersionsSent[invId][slotIndex] )
	{
		CInventoryUpdater<CInvCat>::incInfoVersion( invId, slotIndex );
		pushInfoVersion( invId, slotIndex );
	}
}


/*
 * Set the last sent info version, when the client requested to receive info sync.
 */
void		CInventoryUpdaterForCharacter::syncInfoVersion( TInventoryId invId, uint slotIndex )
{
	_LastItemInfoVersionsSent[invId][slotIndex] = _ItemInfoVersions[invId][slotIndex];
}


/*
 * Return the "Info Version" number, from classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
 */
uint8		CInventoryUpdaterForCharacter::getInfoVersionFromClassicDatabase( INVENTORIES::TInventory inventory, uint slotIndex )
{
//	return (uint8)_PropertyDatabasePt->getProp( string("INVENTORY:") + DatabaseStringFromEInventory[inventory] + NLMISC::toString(":%u:INFO_VERSION", slotIndex) );
	switch (inventory)
	{
	case INVENTORIES::temporary:
		return CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slotIndex).getINFO_VERSION(*_PropertyDatabasePt);
	default:
		return 0;
	}
}


/*
 * Reset an item slot, into classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
 */
void		CInventoryUpdaterForCharacter::resetItemIntoClassicDatabase( INVENTORIES::TInventory inventory, uint slotIndex )
{
	ICDBStructNode *node = _PropertyDatabasePt->getICDBStructNodeFromName( string("INVENTORY:") + DatabaseStringFromEInventory[inventory] + NLMISC::toString(":%d", slotIndex) );
	switch (inventory)
	{
	case INVENTORIES::handling:
		// nothing to do
		break;
	case INVENTORIES::temporary:
		{
			CBankAccessor_PLR::TINVENTORY::TTEMP::TArray &arrayItem = CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slotIndex);
			for ( uint i=0; i!=NbItemPropId; ++i )
			{
				arrayItem.setSHEET(*_PropertyDatabasePt, CSheetId::Unknown);
				arrayItem.setQUALITY(*_PropertyDatabasePt, 0);
				arrayItem.setQUANTITY(*_PropertyDatabasePt, 0);
				arrayItem.setUSER_COLOR(*_PropertyDatabasePt, 0);
	//			arrayItem.setLOCKED(_PropertyDatabasePt, 0);
				arrayItem.setWEIGHT(*_PropertyDatabasePt, 0);
				arrayItem.setNAMEID(*_PropertyDatabasePt, 0);
				arrayItem.setENCHANT(*_PropertyDatabasePt, 0);
				arrayItem.setRM_CLASS_TYPE(*_PropertyDatabasePt, 0);
				arrayItem.setRM_FABER_STAT_TYPE(*_PropertyDatabasePt, 0);
	//			arrayItem.setPRICE(*_PropertyDatabasePt, 0);
	//			arrayItem.setRESALE_FLAG(_PropertyDatabasePt, 0);
				arrayItem.setPREREQUISIT_VALID(*_PropertyDatabasePt, 0);
	//			arrayItem.setWORNED(_PropertyDatabasePt, 0);
			}
		}
		break;
	case INVENTORIES::equipment:
		// nothing to do
		break;
	}
}


/*
 * Set an item slot, into classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
 */
void		CInventoryUpdaterForCharacter::setItemPropsToClassicDatabase( INVENTORIES::TInventory inventory, const INVENTORIES::CItemSlot& itemSlot )
{
//	ICDBStructNode *node = NULL;
//	switch (inventory)
//	{
//	case INVENTORIES::handling:
//		{
//			CBankAccessor_PLR::TINVENTORY::THAND::TArray &arrayItem = CBankAccessor_PLR::getINVENTORY().getHAND().getArray(*_PropertyDatabasePt, itemSlot);
//			node = arrayItem.getCDBNode();
//		}
//		break;
//	case INVENTORIES::temporary:
//		{
//			CBankAccessor_PLR::TINVENTORY::THAND::TArray &arrayItem = CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(*_PropertyDatabasePt, itemSlot);
//			node = arrayItem.getCDBNode();
//		}
//		break;
//	case INVENTORIES::equipment:
//		{
//			CBankAccessor_PLR::TINVENTORY::THAND::TArray &arrayItem = CBankAccessor_PLR::getINVENTORY().getEQUIP().getArray(*_PropertyDatabasePt, itemSlot);
//			node = arrayItem.getCDBNode();
//		}
//		break;
//
//	}
//	if (node != NULL)
//	{
//		for ( uint i=0; i!=NbItemPropId; ++i )
//		{
//			// It does not matter if a property is lacking for a certain inventory, because this method does not assert/warning
//			_PropertyDatabasePt->setProp( node, CItemSlot::ItemPropStr[i], (sint64)itemSlot.getItemProp( (INVENTORIES::TItemPropId)i ) );
//		}
//	}
	ICDBStructNode *node = _PropertyDatabasePt->getICDBStructNodeFromName( string("INVENTORY:") + DatabaseStringFromEInventory[inventory] + NLMISC::toString(":%d", itemSlot.getSlotIndex()) );
	for ( uint i=0; i!=NbItemPropId; ++i )
	{
		// It does not matter if a property is lacking for a certain inventory, because this method does not assert/warning
		_PropertyDatabasePt->x_setProp( node, CItemSlot::ItemPropStr[i], (sint64)itemSlot.getItemProp( (INVENTORIES::TItemPropId)i ) );
	}
}


/*
 * Set only one property of an item slot, into classic database only (will not work for inventories for which isCompatibleWithInventory() is true)
 */
void		CInventoryUpdaterForCharacter::setOneItemPropToClassicDatabase( INVENTORIES::TInventory inventory, uint slotIndex, INVENTORIES::TItemPropId propId, sint32 value )
{
	ICDBStructNode *leaf = _PropertyDatabasePt->getICDBStructNodeFromName( string("INVENTORY:") + DatabaseStringFromEInventory[inventory] + NLMISC::toString(":%d:", slotIndex) + CItemSlot::ItemPropStr[propId] );
	_PropertyDatabasePt->x_setProp( leaf, (sint64)value );
}


/*#include "nel/misc/command.h"
NLMISC_COMMAND( testInventoryUpdater, "", "" )
{
	// setItemProps
	INVENTORIES::CItemSlot itemSlot( 0 );
	itemSlot.setItemProp( INVENTORIES::Sheet, CSheetId("itforage.sitem").asInt() );
	itemSlot.setItemProp( INVENTORIES::Quality, 6 );
	itemSlot.setItemProp( INVENTORIES::Quantity, 1 );
	itemSlot.setItemProp( INVENTORIES::UserColor, 0 );
	itemSlot.setItemProp( INVENTORIES::Locked, 0 );
	itemSlot.setItemProp( INVENTORIES::Weight, 0 );
	itemSlot.setItemProp( INVENTORIES::NameId, 0 );
	itemSlot.setItemProp( INVENTORIES::Enchant, 0 );
	itemSlot.setItemProp( INVENTORIES::Pos, 0 );
	itemSlot.setItemProp( INVENTORIES::Price, 0 );
	itemSlot.setItemProp( INVENTORIES::ResaleFlag, 0 );
	TestInvUpdater->setItemProps( INVENTORIES::Bag, itemSlot );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 0, INVENTORIES::Weight, 2 );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 1, INVENTORIES::Weight, 2 );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 1, INVENTORIES::Weight, 3 );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 1, INVENTORIES::Quantity, 2 );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 1, INVENTORIES::Quantity, 4 );
	itemSlot.setSlotIndex( 1 );
	TestInvUpdater->setItemProps( INVENTORIES::Bag, itemSlot ); // tests "cancel" case in setItemProp()
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 1, INVENTORIES::Quantity, 5 );
	TestInvUpdater->resetItem( INVENTORIES::Bag, 0 );
	TestInvUpdater->setItemProps( INVENTORIES::Bag, itemSlot );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 0, INVENTORIES::Weight, 2 );
	TestInvUpdater->setOneItemProp( INVENTORIES::Bag, 0, INVENTORIES::Weight, 3 );
	TestInvUpdater->resetItem( INVENTORIES::Bag, 0 ); // tests "cancel" case in resetItem()

	TestInvUpdater->_PropertyDatabasePt->setProp( "INVENTORY:BAG:0:SHEET", 0 ); // should produce an error at runtime

	return true;
}*/
