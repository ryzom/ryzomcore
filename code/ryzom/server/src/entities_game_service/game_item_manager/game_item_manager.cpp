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

// misc
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/command.h"

// game share
#include "game_share/mirror.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/synchronised_message.h"

// egs
#include "egs_sheets/egs_sheets.h"
#include "game_item_manager/game_item_manager.h"
#include "shop_type/static_items.h"
#include "server_share/log_item_gen.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

extern CMirror			Mirror;
extern CMirroredDataSet	*FeTempDataset;
#ifdef TheDataset
#else 
#define TheDataset		(*FeTempDataset)
#endif

CGameItemManager GameItemManager;

std::vector< CGameItemPtr > CGameItemManager::_NpcSpecificItems;
extern CVariable<bool> EGSLight;



//---------------------------------------------------
// CGameItemManager :
//
//---------------------------------------------------
CGameItemManager::CGameItemManager()
{
//	_CreatedItemCount = 0; // note : must be save and reload

	// delay in ticks ( if tick time step is 100ms --> 5 min )
//	DecayDelay = 5 * 60 * 10; 
//	CorpseToCarrionDelay = 5 * 60 * 10; 
//	CarrionDecayDelay = 5 * 60 * 10; 
	
//	CorpseMaxCount = 2;
	
} // CGameItemManager //


//---------------------------------------------------
// init :
//
//---------------------------------------------------
//void CGameItemManager::init()
//{
//} // init //


//---------------------------------------------------
// getItemsOnTheGround :
//
//---------------------------------------------------
//void CGameItemManager::getItemsOnTheGround( list<CGameItemPtr>& itemsOnGround )
//{
//	map<CEntityId,CGameItemPtr>::const_iterator itItem;
//	for( itItem = _Items.begin(); itItem != _Items.end(); ++itItem )
//	{
//		if( (*itItem).second()->isOnTheGround() )
//		{
//			itemsOnGround.push_back( (*itItem).second );
//		}
//	}
//} // getItemsOnTheGround //


//---------------------------------------------------
// getItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::getItem( const CEntityId& id )
//{
//	map<CEntityId,CGameItemPtr>::iterator itItem = _Items.find( id );
//	if( itItem != _Items.end() )
//	{
//		return (*itItem).second;
//	}
//	else
//	{
//		return NULL;
//	}
//
//} // getItem //


//---------------------------------------------------
// getNewItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::getNewItem( CEntityId& id, CSheetId& sheetId, uint16 quality, bool destroyable , bool dropable)
CGameItemPtr CGameItemManager::getNewItem( const CSheetId& sheetId, uint16 quality, bool destroyable , bool dropable)
{
	CAllStaticItems::iterator itForm = CSheets::getItemMapFormNoConst().find( sheetId );
	if( itForm != CSheets::getItemMapFormNoConst().end() )
	{
		// get the slot count
//		sint16 slotCount = 0;
		//nldebug("<CGameItemManager::getNewItem> Family type: %s", ITEMFAMILY::toString( (*itForm).second.Family ).c_str() );
//		if( ( (*itForm).second.Family == ITEMFAMILY::BAG ) || ( (*itForm).second.Family == ITEMFAMILY::STACK ) )
//		{
//			slotCount = (*itForm).second.SlotCount;
//		}
		
		// create the item
		CGameItemPtr item;
//		item.newItem( id, sheetId, quality, slotCount, destroyable, dropable );
		item.newItem( sheetId, quality, destroyable, dropable );

//		if( item!=NULL )
//		{
			// init the dynamic values from sheet -> DONE in CGameItemCreator
			//(*item).Quality = (*itForm).second.Quality;
			//(*item).HP = (*itForm).second.HitPoints;
			
			// if this item contains sub items we create them
//			if( (*itForm).second.Family == ITEMFAMILY::CORPSE || (*itForm).second.Family == ITEMFAMILY::CARRION )
//			{
				// create the content
/*				for (uint i=0; i < (*itForm).second.Content.size(); i++)
				{
					CSheetId itemSheetId( (*itForm).second.Content[i] );
					createItem( itemSheetId, quality, id, -1, destroyable,dropable );
				}
*/
				// if the item is a corpse, we push it to list and erase the olds one if we reached max corpse count
//				if( (*itForm).second.Family == ITEMFAMILY::CORPSE )
//				{
//					_Corpses.push( item );
//					if( _Corpses.size() > CorpseMaxCount )
//					{
//						CGameItemPtr itemTmp = _Corpses.front();
//						destroyItem( itemTmp );
//						_Corpses.pop();
//					}
//				}
//			}
//		}

		return item;
	}
	return NULL;

} // getNewItem //




//---------------------------------------------------
// createItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::createItem( CEntityId& id, CSheetId& sheetId, uint16 quality, sint16 slot, bool destroyable, bool dropable , const CEntityId &creatorId )
CGameItemPtr CGameItemManager::createItem( const CSheetId& sheetId, uint16 quality, bool destroyable, bool dropable , const CEntityId &creatorId )
{
	H_AUTO(GIM_createItem1);
	
	// test if the item already exists
//	map<CEntityId,CGameItemPtr>::iterator itIt = _Items.find( id );
//	if( itIt != _Items.end() )
//	{
//		nlwarning("<CGameItemManager::createItem> The item %s already exists",id.toString().c_str());
//		return NULL;
//	}

// MALKAV 22/01/03 :  get owner, if owner not found, returns before creating the item and display a simple warning
//	CGameItemPtr ownerItem = NULL;
//	if( owner != CEntityId::Unknown )
//	{
//		ownerItem = getItem( owner );
//		BOMB_IF(ownerItem == NULL ,"Bad owner found for item",return NULL);
//	}
//
	// create a new item
//	CGameItemPtr item = getNewItem( id, sheetId, quality, destroyable, dropable );
	CGameItemPtr item = getNewItem(sheetId, quality, destroyable, dropable );
	if( item != NULL )
	{
//		nldebug("<CGameItemManager::createItem> create item %s with owner %s",id.toString().c_str(), owner.toString().c_str());

//		(*item)->Owner = owner;
		(*item)->setCreator( creatorId );
		
		// insert the item in the map
//		_Items.insert( make_pair(id,item) );
//		_Items.insert( item );

		// insert the item in the children of the owner
// MALKAV 22/01/03 : test the owner existence sooner and use a warning instead of an nlerror to keep going
//		if( owner != CEntityId::Unknown )
//		{
//		 	CGameItemPtr ownerItem = getItem( owner );
//			if( ownerItem!=NULL )
//			{
//				(*ownerItem)->addChild( item, slot );
//			}
//			else
//			{				
//				nlerror("<CGameItemManager::createItem> Can't find the owner item %s",owner.toString().c_str());
//			}
//		}
	}
	else
	{
//		nlwarning("<CGameItemManager::createItem> Can't create the item %s with invalid sheet '%s'", id.toString().c_str(), sheetId.toString().c_str());
		nlwarning("<CGameItemManager::createItem> Can't create an item with invalid sheet '%s'", sheetId.toString().c_str());
	}

	log_Item_Create(item->getItemId(), item->getSheetId(), item->getStackSize(), item->quality());

	return item;

} // createItem //


//---------------------------------------------------
// createRootItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::createRootItem( CEntityId& id, CSheetId& sheetId, uint16 quality, const CEntityId& ownerPlayer, uint16 inventory , sint16 slot, bool destroyable, bool dropable, const CEntityId &creatorId )
//{
//	// test if the item already exists
//	map<CEntityId,CGameItemPtr>::iterator itIt = _Items.find( id );
//	if( itIt != _Items.end() )
//	{
//		nlwarning("<CGameItemManager::createRootItem> The item %s already exists",id.toString().c_str());
//		return NULL;
//	}
//
//	// create a new item
//	CGameItemPtr item = getNewItem( id, sheetId, quality, destroyable, dropable );
//	if( item != NULL )
//	{
//		CEntityId ownerId = ownerPlayer;
//		ownerId.setType( (uint8)inventory );
//
//		(*item)->Owner = ownerId;
//		(*item)->setCreator( creatorId );
//		
//		// insert the item in the map
//		_Items.insert( make_pair(id,item) );
//	}
//	else
//	{
//		nlwarning("<CGameItemManager::createRootItem> Can't create the item %s with type %s",id.toString().c_str(),sheetId.toString().c_str());
//	}
//
//	return item;
//} // createRootItem //

//---------------------------------------------------
// createRootItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::createRootItem( CSheetId& sheetId, uint16 quality, const CEntityId& ownerPlayer, uint16 inventory, sint16 slot, bool destroyable, bool dropable, const CEntityId &creatorId )
//{
//	CEntityId itemId(RYZOMID::object,_CreatedItemCount++);
//	return createRootItem( itemId, sheetId, quality, ownerPlayer, inventory, slot, destroyable, dropable, creatorId );
//} // createRootItem //


//---------------------------------------------------
// createItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::createItem( CEntityId& id, CSheetId& sheetId, uint16 quality, sint32 x, sint32 y, sint32 z, bool destroyable, bool dropable, const NLMISC::CEntityId &creatorId )
//{
//	H_AUTO(GIM_createItem2);
//
//	// test if the item already exists
//	if( getItem(id)!=NULL )
//	{
//		nlwarning("<CGameItemManager::createItem> The item %s already exists",id.toString().c_str());
//		return NULL;
//	}
//
//	// create a new item
//	CGameItemPtr item = getNewItem( id, sheetId, quality, destroyable, dropable );
//	if( item != NULL )
//	{
//		(*item)->Loc.Pos.X = x;
//		(*item)->Loc.Pos.Y = y;
//		(*item)->Loc.Pos.Z = z;
//
//		(*item)->setCreator( creatorId );
//		
//		// insert the item in the map
//		_Items.insert( make_pair(id,item) );
//
////	Obsolete code, keep for not forget ADD_IA_OBJECT GPMS functionality but at this time AI creature don't care about loot on corps
////  So at this time, we don't insert in GPMS a loot of creature 
///*		if( id.getType() == RYZOMID::creature_loot )
//		{
///*			CMessage msgout2("ADD_IA_OBJECT");
//			msgout2.serial( id );
//			msgout2.serial( x );
//			msgout2.serial( y );
//			msgout2.serial( z );
//			float heading = 0;
//			msgout2.serial( heading );
//			uint32 sheet = sheetId.asInt();
//			msgout2.serial( sheet );
//			sendMessageViaMirror( "GPMS", msgout2 );
//		}
//		else
//*/		{
//			// add the item in the GPMS *** +++ Make this in EGS +++ *** 
//			Mirror.addEntity( false, id );
//			TDataSetRow entityIndex = TheDataset.getDataSetRow( id );
//			if ( TheDataset.isAccessible( entityIndex ))
//			{
//				CMirrorPropValue<TYPE_SHEET> valueS( TheDataset, entityIndex, DSPropertySHEET );
//				valueS = sheetId.asInt();
//				CMirrorPropValue<TYPE_POSX> valueX( TheDataset, entityIndex, DSPropertyPOSX );
//				valueX = x;
//				CMirrorPropValue<TYPE_POSY> valueY( TheDataset, entityIndex, DSPropertyPOSY );
//				valueY = y;
//				CMirrorPropValue<TYPE_POSZ> valueZ( TheDataset, entityIndex, DSPropertyPOSZ );
//				valueZ = z;
//				CMirrorPropValue<TYPE_ORIENTATION> valueT( TheDataset, entityIndex, DSPropertyORIENTATION );
//				valueT = 0.0f;
//				const uint64 bitfield = (uint64(0xffffffff) << 32) + uint64(0xffffffff);
//				CMirrorPropValue<TYPE_WHO_SEES_ME> whoSeesMe(TheDataset, entityIndex, DSPropertyWHO_SEES_ME );
//				whoSeesMe = bitfield;
//			}
//		}
//	}
//	else
//	{
//		nlwarning("<CGameItemManager::createItem> Can't create the item %s with type %s",id.toString().c_str(),sheetId.toString().c_str());
//	}
//
//	return item;
//
//} // createItem //



//---------------------------------------------------
// createItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::createItem( CSheetId& sheetId, uint16 quality, const CEntityId& owner, sint16 slot, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId )
//CGameItemPtr CGameItemManager::createItem( CSheetId& sheetId, uint16 quality, bool destroyable, bool dropable, const NLMISC::CEntityId &creatorId )
//{
//	CEntityId itemId(RYZOMID::object,_CreatedItemCount++);
////	return createItem( itemId, sheetId, quality, owner, slot, destroyable,dropable, creatorId );
//	return createItem( sheetId, quality, slot, destroyable,dropable, creatorId );
//} // createItem //
	


//---------------------------------------------------
// createItem :
//
//---------------------------------------------------
//CGameItemPtr CGameItemManager::createItem( CSheetId& sheetId, uint16 quality, sint32 x, sint32 y, sint32 z, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId )
//{
//	CEntityId itemId(RYZOMID::object,_CreatedItemCount++);
////	return createItem( itemId, sheetId, quality, x ,y ,z, destroyable,dropable, creatorId);
//	return createItem( sheetId, quality, x ,y ,z, destroyable,dropable, creatorId);
//} // createItem //
//

//-----------------------------------------------
// createInGameItem
//-----------------------------------------------
CGameItemPtr CGameItemManager::createInGameItem( uint16 quality, uint32 quantity, const NLMISC::CSheetId &sheet, const CEntityId &creatorId , const std::string * phraseId)
{
	H_AUTO(GIM_createInGameItem);

	static const CSheetId preorderSheetId("pre_order.sitem");
	
	if ( quantity == 0 || quality ==0 )
		return NULL;

//	static const CSheetId idSheetStack("stack.sitem");
	const CStaticItem* form = CSheets::getForm( sheet );
	if (!form)
	{
		nlwarning("<CCharacter::createInGameItem> Cannot find form of item %s", sheet.toString().c_str());
		return NULL;
	}

	CGameItemPtr item;
	CGameItemPtr sellingItem;
	// if item can be sold, get it in the sold items list
	if (	form->Family != ITEMFAMILY::RAW_MATERIAL 
		&&	form->Family != ITEMFAMILY::HARVEST_TOOL
		&&	form->Family != ITEMFAMILY::CRAFTING_TOOL
		&&	form->Family != ITEMFAMILY::CRYSTALLIZED_SPELL
		&&	form->Family != ITEMFAMILY::ITEM_SAP_RECHARGE
		&&	form->Family != ITEMFAMILY::FOOD
		)
	{
		vector< CGameItemPtr >::const_iterator it;
		const vector< CGameItemPtr >::const_iterator itEnd = CStaticItems::getStaticItems().end();
		for( it = CStaticItems::getStaticItems().begin(); it != itEnd; ++it )
		{
			if( (*it)->getSheetId() == sheet )
			{
				sellingItem = *it;
				break;
			}
		}
	}

	switch( form->Family )
	{
		case ITEMFAMILY::CRAFTING_TOOL:
		case ITEMFAMILY::HARVEST_TOOL:
		case ITEMFAMILY::RAW_MATERIAL:
		case ITEMFAMILY::TELEPORT:
		case ITEMFAMILY::CRYSTALLIZED_SPELL:
		case ITEMFAMILY::ITEM_SAP_RECHARGE:
		case ITEMFAMILY::MISSION_ITEM:
		case ITEMFAMILY::PET_ANIMAL_TICKET:
		case ITEMFAMILY::HANDLED_ITEM:
		case ITEMFAMILY::CONSUMABLE:
		case ITEMFAMILY::XP_CATALYSER:
		case ITEMFAMILY::SCROLL:
		case ITEMFAMILY::FOOD:
		case ITEMFAMILY::SCROLL_R2:
		case ITEMFAMILY::GENERIC_ITEM:
		{
			item = GameItemManager.createItem( const_cast< CSheetId& > ( sheet ), quality, true, true, creatorId);
		}
		break;
	default:
		{
			if( sellingItem != NULL )
			{
				item = sellingItem->getItemCopy();
				item->quality( quality );
				if ( phraseId )
					item->setPhraseId(*phraseId);
			}
			else if (sheet == preorderSheetId)
			{
				item = GameItemManager.createItem(sheet, quality, true, form->DropOrSell, creatorId);
			}
		}
		if( item == NULL)
		{
			nlwarning("<CCharacter::createInGameItem> Error while creating item : NULL pointer");
			return NULL;
		}
	}
	quantity = min(quantity, item->getMaxStackSize());
	item->setStackSize(quantity);
	return item;
} // createInGameItem //

//---------------------------------------------------
// destroyItem :
//
//---------------------------------------------------
void CGameItemManager::destroyItem( CGameItemPtr &ptr )
{
	H_AUTO(GIM_destroyItem);
	
	if (ptr == NULL)
		return;

	// used to know if we modified the content of a bag on the ground
	CGameItemPtr modifiedBag = NULL;

	// Remove the item from it's inventory (if any)
	if (ptr->getInventory() != NULL)
	{
		ptr->getInventory()->removeItem(ptr->getInventorySlot());
	}
	if (ptr->getRefInventory() != NULL)
	{
		ptr->getRefInventory()->removeItem(ptr->getRefInventorySlot());
	}

	ptr.deleteItem();

} // destroyItem //



//---------------------------------------------------
// dropOnTheGround :
//
//---------------------------------------------------
//void CGameItemManager::dropOnTheGround( const CEntityId& id, sint32 x, sint32 y, sint32 z ) 
//{
//	// until debugged
//	return;
//	
//	// get the item to drop
//	CGameItemPtr item = getItem( id );
//	nlassert( item!=NULL );
//
//	CGameItemPtr bagItem = NULL;
//
//	// try to find a bag on the ground
//	map<CEntityId,CGameItemPtr>::iterator itItem;
//	for( itItem = _Items.begin(); itItem != _Items.end(); ++itItem )
//	{
//		if( (*(*itItem).second)->Owner == CEntityId::Unknown )
//		{
//			uint32 distX = abs(x-(*(*itItem).second)->Loc.Pos.X);
////			nldebug("DistX : %d",distX);
//			if( distX < 10000 )
//			{
//				uint32 distY = abs(y-(*(*itItem).second)->Loc.Pos.Y);
////				nldebug("DistY : %d",distY);
//				if( distY < 10000 )
//				{
//					// check if there is room in this bag
//					uint i = 0;
//					const vector<CGameItemPtr>& children = (*( (*itItem).second) )->getChildren();
//					for (; i < children.size(); ++i )
//					{
//						if (  children[i] == NULL )
//						{
//							bagItem = (*itItem).second;
//							break;
//						}
//					}
//					// if there was a break in the previous iteration, we have found a suitable bag
//					if ( i != children.size() )
//						break;
//					/*
//					//if( abs(z-(*(*itItem).second).Loc.Pos.Z) < 1 )
//					{
//						try
//						{
//							const CStaticItem* bagForm = getForm((*(*itItem).second).getSheetId());
//							const CStaticItem* itemForm = getForm( (*item).getSheetId() );
//							if( bagForm != 0 && itemForm != 0 )
//							{
//								// check if the item fits in the bag
//								bool fit = false;
//								uint i;
//								for( i = 0; i < bagForm->ContentType.size(); i++ )
//								{
//									if( ITEMFAMILY::toString( itemForm->Family ) == bagForm->ContentType[i] )
//									{
//										fit = true;
//										break;
//									}
//								}
//								// if the item fits, we keep this bag
//								if( fit )
//								{
//									nldebug("CGameItemManager::dropOnTheGround> bag found on the ground");
//									bagItem = (*itItem).second;
//									break;
//								}
//							}
//						}
//						catch(const Exception &e)
//						{
//							nlwarning("<CGameItemManager::dropOnTheGround> %s",e.what());
//							return;
//						}
//					}*/
//				}
//			}
//		}
//	}
//	
//	// if no close bag exist or if the bag is full, we create one
//	if( bagItem == NULL  )
//	{
//		try
//		{
////			nldebug("CGameItemManager::dropOnTheGround> Creating bag on the ground at pos %d %d %d",x,y,z);
//			
//			const CStaticItem* itemForm = CSheets::getForm( (*item)->getSheetId() );
//			if( itemForm )
//			{
////				if( !itemForm->Sack.empty() )
////				{
//					CSheetId sheetId("bag.item"/*itemForm->Sack*/);
//					if (sheetId == CSheetId::Unknown)
//					{
//						nlwarning("<CGameItemManager::dropOnTheGround> sheet it bag.item not found");
//						return;
//					}
//
//					bagItem = createItem( sheetId, 10/*itemForm->MaxQuality*/, x, y, z, true,true );
//					bagItem->setAsOnTheGround();
//					/*				}
//				else
//				{
//					nlwarning("<CGameItemManager::dropOnTheGround> Missing sack_type information for the item '%s'",(*item).getSheetId().toString().c_str() );
//					return;
//					}*/
//			}
//		}
//		catch(const Exception &e)
//		{
//			nlwarning("<CGameItemManager::dropOnTheGround> %s",e.what());
//			return;
//		}
//	}
//	if (bagItem==NULL)
//	{
//		nlwarning("<CGameItemManager::dropOnTheGround> could not create a bag item with sheet %s",(*item)->getSheetId().toString().c_str());
//		return;
//	}
//	
//	// detach item from its owner
//	CGameItemPtr ownerItem = getItem( (*item)->Owner );
//	if ( ownerItem!=NULL )
//		ownerItem()->detachChild( (*item)->Loc.Slot );
//	
//	// add the item in the bag
//	(*bagItem)->addChild( item );
//
//	// reset the time on the ground
//	(*bagItem)->TimeOnTheGround = 0;
//
//	// send a message to the GPMS with bag content
//	CMessage msgout("BAG");
//	vector<CEntitySheetId> content;
//	getContent( (*bagItem)->getId(), content );
//	msgout.serial((*bagItem)->getId());
//	msgout.serialCont(content);
//	sendMessageViaMirror("GPMS", msgout);
//
//} // dropOnTheGround //



//---------------------------------------------------
// getContent :
//
//---------------------------------------------------
//void CGameItemManager::getContent( CEntityId& itemId, list<CSheetId>& content )
//{
//	map<CEntityId,CGameItemPtr>::iterator itItem = _Items.find( itemId );
//	if( itItem != _Items.end() )
//	{
//		if( (*(*itItem).second)->getChildren().empty() )
//		{
//			content.push_back( (*(*itItem).second)->getSheetId() );
//		}
//		else
//		{
//			vector<CGameItemPtr>::const_iterator itChild;
//			for( itChild = (*(*itItem).second)->getChildren().begin(); itChild != (*(*itItem).second)->getChildren().end(); ++itChild )
//			{
//				if ( *itChild != NULL )
//					getContent( (*itChild)()->getId(), content );
//			}
//		}
//	}
//	else
//	{
//		nlwarning("<CGameItemManager::getContent> the item %s is unknown",itemId.toString().c_str());
//	}
//
//} // getContent //

//---------------------------------------------------
// getContent :
//
//---------------------------------------------------
//void CGameItemManager::getContent( CEntityId& itemId, vector<CSheetId>& content )
//{
//	map<CEntityId,CGameItemPtr>::iterator itItem = _Items.find( itemId );
//	if( itItem != _Items.end() )
//	{
//		if( (*(*itItem).second)->getChildren().empty() )
//		{
//			content.push_back( (*(*itItem).second)->getSheetId() );
//		}
//		else
//		{
//			vector<CGameItemPtr>::const_iterator itChild;
//			for( itChild = (*(*itItem).second)->getChildren().begin(); itChild != (*(*itItem).second)->getChildren().end(); ++itChild )
//			{
//				if ( *itChild != NULL )
//					getContent( (*itChild)()->getId(), content );
//			}
//		}
//	}
//	else
//	{
//		nlwarning("<CGameItemManager::getContent> the item %s is unknown",itemId.toString().c_str());
//	}
//
//} // getContent //


//---------------------------------------------------
// getContent :
//
//---------------------------------------------------
//void CGameItemManager::getContent( CEntityId& itemId, list<CEntitySheetId>& content )
//{
//	map<CEntityId,CGameItemPtr>::iterator itItem = _Items.find( itemId );
//	if( itItem != _Items.end() )
//	{
//		if( (*(*itItem).second)->getChildren().empty() )
//		{
//			content.push_back( CEntitySheetId((*(*itItem).second)->getId(), (*(*itItem).second)->getSheetId()) );
//		}
//		else
//		{
//			vector<CGameItemPtr>::const_iterator itChild;
//			for( itChild = (*(*itItem).second)->getChildren().begin(); itChild != (*(*itItem).second)->getChildren().end(); ++itChild )
//			{
//				if ( *itChild != NULL )
//					getContent( (*itChild)()->getId(), content );
//			}
//		}
//	}
//	else
//	{
//		nlwarning("<CGameItemManager::getContent> the item %s is unknown",itemId.toString().c_str());
//	}
//
//} // getContent //

//---------------------------------------------------
// getContent :
//
//---------------------------------------------------
//void CGameItemManager::getContent( CEntityId& itemId, vector<CEntitySheetId>& content )
//{
//	map<CEntityId,CGameItemPtr>::iterator itItem = _Items.find( itemId );
//	if( itItem != _Items.end() )
//	{
//		if( (*(*itItem).second)->getChildren().empty() )
//		{
//			content.push_back( CEntitySheetId((*(*itItem).second)->getId(), (*(*itItem).second)->getSheetId()) );
//		}
//		else
//		{
//			vector<CGameItemPtr>::const_iterator itChild;
//			for( itChild = (*(*itItem).second)->getChildren().begin(); itChild != (*(*itItem).second)->getChildren().end(); ++itChild )
//			{
//				if ( *itChild != NULL )
//					getContent( (*itChild)()->getId(), content );
//			}
//		}
//	}
//	else
//	{
//		nlwarning("<CGameItemManager::getContent> the item %s is unknown",itemId.toString().c_str());
//	}
//
//} // getContent //


//---------------------------------------------------
// dumpGameItemList :
//
//---------------------------------------------------
//void CGameItemManager::dumpGameItemList( const string& fileName )
//{
//	FILE * f;
//	f = fopen(fileName.c_str(),"w");
//	
//	if(f)
//	{
//		map<CEntityId,CGameItemPtr>::const_iterator itItem;
//		for( itItem = _Items.begin(); itItem != _Items.end(); ++itItem )
//		{
//			fprintf(f,"%s ; ",(*itItem).first.toString().c_str());
//		}
//
//		fclose(f);
//	}
//	else
//	{
//		nlwarning("(EGS)<CGameItemManager::dumpGameItemList> Can't open the file %s",fileName.c_str());
//	}
//
//} // dumpGameItemList //
//


//----------------------------------------------------------------------------
// CGameItemManager::buildNpcSpecificItems
//----------------------------------------------------------------------------
void CGameItemManager::buildNpcSpecificItems()
{
	if (EGSLight)
		return;

	for( CAllStaticItems::const_iterator it = CSheets::getItemMapForm().begin(); it != CSheets::getItemMapForm().end(); ++it )
	{
		// only keep npc items
//		if( (*it).first.toString().substr(0,4) != "npc_" ) //obsolete, now we want be able to have all non craftable item
		if( (*it).first.toString().substr(0,2) == "ic" )
		{
			continue;
		}
		
		CGameItemPtr item;
//		item.newItem( CEntityId::Unknown, (*it).first, 1, 0, false,false );
		item.newItem( it->first, 1, false,false );
		
		_NpcSpecificItems.push_back(item);
		
#ifdef NL_DEBUG
		nldebug("built npc item item %s", (*it).first.toString().c_str());
#endif
	}
} // CGameItemManager //


//-----------------------------------------------
// createItem :
//-----------------------------------------------
NLMISC_COMMAND(createItem,"create a new item","<sheet id><quality><owner id>")
{
	if( args.size() < 3 )
	{
		return false;
	}

	uint32 itemId;
	NLMISC::fromString(args[0], itemId);
	CSheetId sheetId(itemId);

	uint16 quality;
	NLMISC::fromString(args[1], quality);

	uint32 ownerIdTmp;
	NLMISC::fromString(args[2], ownerIdTmp);
	CEntityId ownerId(RYZOMID::object,ownerIdTmp);	

//	CGameItemPtr item = GameItemManager.createItem( sheetId, quality, ownerId, -1, true,true );
	CGameItemPtr item = GameItemManager.createItem( sheetId, quality, true, true );
	if (item != NULL)
	{
//		log.displayNL("<createItem> Creating item %d",(*item).getItemId().getRawId());
		log.displayNL("<createItem> Item created");
	}
	else
	{
		log.displayNL("<createItem> : failed to create the item.");
		return false;
	}

	return true;

} // createItem //

//-----------------------------------------------
// displayItem :
//-----------------------------------------------
//NLMISC_COMMAND(displayItem,"display item characteristics","<item id>")
//{
//	if( args.size() != 1 )
//	{
//		return false;
//	}
//
//	uint32 itemId;
//	NLMISC::fromString(args[0], itemId)
//
//	CEntityId itemId(RYZOMID::object, itemId);
//	
//	CGameItemPtr item = GameItemManager.getItem(itemId);
//	if (item != NULL)
//	{
//		item->displayInLog(log);
//	}
//	else
//	{
//		log.displayNL("<displayItem> Cannot find item %s",itemId.toString().c_str());
//	}
//
//	return true;
//} // displayItem //

//-----------------------------------------------
// drop :
//-----------------------------------------------
//NLMISC_COMMAND(drop,"drop an item to the ground","<item id><quality><x><y><z>")
//{
//	if( args.size() < 5 )
//	{
//		return false;
//	}
//
//	CEntityId itemId( RYZOMID::object, NLMISC::fromString(args[0].c_str()) );
//	//uint16 quality = (uint16)NLMISC::fromString(args[1].c_str());
//	
//	sint32 x;
//	NLMISC::fromString(args[2], x);
//	x *= 1000;
//	sint32 y;
//	NLMISC::fromString(args[3], y);
//	y *= 1000;
//	sint32 z;
//	NLMISC::fromString(args[4], z);
//	z *= 1000;
//
//	GameItemManager.dropOnTheGround( itemId, x, y, z );
//
//	return true;
//} // drop //
//
//-----------------------------------------------
// spawn :
//-----------------------------------------------
//NLMISC_COMMAND(spawn,"create an item on the ground","[<item id>]<sheet id><quality><x><y><z>")
//{
//	if( args.size() < 5 )
//	{
//		return false;
//	}
//
//	if( args.size() < 6 )
//	{
//		CSheetId sheetId(args[0]);
//		uint16 quality = (uint16)NLMISC::fromString(args[1].c_str());
//		sint32 x = (sint32)NLMISC::fromString(args[2].c_str()) * 1000;
//		sint32 y = (sint32)NLMISC::fromString(args[3].c_str()) * 1000;
//		sint32 z = (sint32)NLMISC::fromString(args[4].c_str()) * 1000;
//		GameItemManager.createItem( sheetId, quality, x, y, z, true , true);
//	}
//	else
//	{
//		CEntityId itemId( RYZOMID::object, NLMISC::fromString(args[0].c_str()) );
//		CSheetId sheetId(args[1]);
//		uint16 quality = (uint16)NLMISC::fromString(args[2].c_str());
//		sint32 x = (sint32)NLMISC::fromString(args[3].c_str()) * 1000;
//		sint32 y = (sint32)NLMISC::fromString(args[4].c_str()) * 1000;
//		sint32 z = (sint32)NLMISC::fromString(args[5].c_str()) * 1000;
//		GameItemManager.createItem( itemId, sheetId, quality, x, y, z, true, true );
//	}
//
//	return true;
//
//} // spawn //

//-----------------------------------------------
// destroy_item :
//-----------------------------------------------
//NLMISC_COMMAND(destroyItem,"destroy the specified item","<item id(id:type:crea:dyn)>")
//{
//	if( args.size() < 1 )
//	{
//		return false;
//	}
//	
//	CEntityId itemId( RYZOMID::object, NLMISC::fromString(args[0].c_str()) );
//	CGameItemPtr ptr = GameItemManager.getItem(itemId);
//	GameItemManager.destroyItem( ptr );
//	
//	return true;
//} // destroy_item //
