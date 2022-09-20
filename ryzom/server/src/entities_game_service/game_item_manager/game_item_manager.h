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



#ifndef GAME_ITEM_MANAGER_H
#define GAME_ITEM_MANAGER_H

// georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_loader.h"

// std
#include <queue>

// game_share
#include "server_share/msg_gpm_service.h"
#include "game_share/inventories.h"

#include "game_item_manager/game_item.h"




/**
 * CGameItemManager
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CGameItemManager
{
public :

	/// decay delay of an item on the ground (tick count)
//	NLMISC::TGameCycle DecayDelay;

	/// delay before a corpse becomes a carrion (tick count)
//	NLMISC::TGameCycle CorpseToCarrionDelay;

	/// carrion decay delay(tick count)
//	NLMISC::TGameCycle CarrionDecayDelay;

	/// corpse max count
//	uint32 CorpseMaxCount;

	/// exception thrown when a sheet is unknown
	struct ESheet : public NLMISC::Exception
	{
		ESheet( const NLMISC::CSheetId& sheetId ) : Exception ("The sheet "+sheetId.toString()+" is unknown") {}
	};

public:

	/**
	 * Default constructor
	 */
	CGameItemManager();

	/**
	 * Init the manager (register callbacks)
	 */
//	void init();

	/**
	 *	Get the list of items on the ground
	 */
//	void getItemsOnTheGround( std::list<CGameItemPtr>& itemsOnGround );

	/**
	 *	Get the item from its id
	 * \param id is the item's id
	 * \return reference on the item
	 */
//	CGameItemPtr getItem( const NLMISC::CEntityId& id );


	/**
	 * get the max number of HP for the speciefied quality level
	 * \param quality the quality level
	 * \return the number of hp
	 */
//	inline uint32 getMaxHPForQualityLevel( uint16 quality ) const
//	{
//		return (uint32) pow(1.2f, float(quality) -1) * 35;
//	};

	/**
	 * get the max number of HP for the speciefied quality level
	 * \param quality the quality level
	 * \return the number of hp
	 */
//	inline uint32 getMaxHPForQualityLevel( uint16 quality, const CStaticItem *form) const
//	{
///*		static CGameItem tmpItem;
//
//		if (!form || form->HitPoints.empty() )
//			return (uint32) pow(1.2f, float(quality) -1) * 35;
//		else
//		{
//			//tmpItem.setStandardQuality(quality);
//			tmpItem._Quality = quality;
//
//			double result;
//			int errorIndex;
//			if( tmpItem.evalExpression ( form->HitPoints.c_str(), result, &errorIndex ) != NLMISC::CEvalNumExpr::NoError )
//			{
//				nlwarning("<CGameItemManager::getMaxHPForQualityLevel> Can't eval %s", form->HitPoints.c_str() );
//				return 0;
//			}
//			else
//			{
//				return (uint32) result;
//			}
//		}
//*/
//		return quality * 100;
//	}
	
	/**
	 *	Create a new item
	 * \param id the item's id
	 * \param sheetId sheet id
	 * \quality is quality of item (level...)
	 * \param owner id of the owner
	 * \param slot slot index
	 * \param destroyable true is the object is destroyable
	 */
//	CGameItemPtr createItem( NLMISC::CEntityId& id, NLMISC::CSheetId& sheetId, uint16 quality, const NLMISC::CEntityId& owner, sint16 slot, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );
	CGameItemPtr createItem( const NLMISC::CSheetId& sheetId, uint16 quality, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );

	/// create a real in game item
	CGameItemPtr createInGameItem( uint16 quality, uint32 quantity, const NLMISC::CSheetId &sheet, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown , const std::string * phraseId = NULL);

	/**
	 *	Create a new item
	 * \param id the item's id
	 * \param sheetId sheet id
	 * \quality is quality of item (level...)
	 * \param owner id of the owner entity (a player !)
	 * \param inventory id of the inventiory to which belongs this item
	 * \param slot slot index
	 * \param destroyable true is the object is destroyable
	 */
//	CGameItemPtr createRootItem( NLMISC::CEntityId& id, NLMISC::CSheetId& sheetId, uint16 quality, const NLMISC::CEntityId& ownerPlayer, uint16 inventory , sint16 slot, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );

	/**
	 *	Create a new item
	 * \param id the item's id
	 * \param sheetId sheet id
	 * \quality is quality of item (level...)
	 * \param x x position
	 * \param y y position
	 * \param z z position
	 * \param destroyable true is the object is destroyable
	 */
//	CGameItemPtr createItem( NLMISC::CEntityId& id, NLMISC::CSheetId& sheetId, uint16 quality, sint32 x, sint32 y, sint32 z, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );
//	CGameItemPtr createItem( NLMISC::CSheetId& sheetId, uint16 quality, sint32 x, sint32 y, sint32 z, bool destroyable,bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );
	
	/**
	 *	Create a new item (its id is created by the manager)
	 * \param sheetId sheet id
	 * \quality is quality of item (level...)
	 * \param owner id of the owner
	 * \param slot slot index
	 * \param destroyable true is the object is destroyable
	 */
//	CGameItemPtr createItem( NLMISC::CSheetId& sheetId, uint16 quality, const NLMISC::CEntityId& owner, sint16 slot, bool destroyable, bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );
//	CGameItemPtr createItem( NLMISC::CSheetId& sheetId, uint16 quality, bool destroyable, bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );

	/**
	 *	Create a new item (its id is created by the manager)
	 * \param sheetId sheet id
	 * \quality is quality of item (level...)
	 * \param owner id of the owner entity (a player !)
	 * \param inventory id of the inventiory to which belongs this item
	 * \param slot slot index
	 * \param destroyable true is the object is destroyable
	 */
//	CGameItemPtr createRootItem( NLMISC::CSheetId& sheetId, uint16 quality, const NLMISC::CEntityId& ownerPlayer, uint16 inventory, sint16 slot, bool destroyable, bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );
	
	/**
	 *	Create a new item (its id is created by the manager)
	 * \param sheetId sheet id
	 * \quality is quality of item (level...)
	 * \param x x position
	 * \param y y position
	 * \param z z position
	 * \param destroyable true is the object is destroyable
	 */
//	CGameItemPtr createItem( NLMISC::CSheetId& sheetId, uint16 quality, sint32 x, sint32 y, sint32 z, bool destroyable, bool dropable, const NLMISC::CEntityId &creatorId = NLMISC::CEntityId::Unknown );

	/**
	 *	Destroy an item
	 * \param id is the id of the object
	 */
	void destroyItem( CGameItemPtr &itemToDestroy );

	/**
	 *	remove an item from the item map
	 * \param id is the id of the object
	 */
//	inline void removeFromMap( NLMISC::CEntityId& id )	{ _Items.erase( id ); }
//	inline void removeFromMap( const CGameItemPtr& id )	{ _Items.erase( id ); }

	/**
	 * Insert item
	 * \param item the item
	 */
//	void insertItem( CGameItemPtr item  )	{ _Items.insert( std::make_pair(item->getItemId(),item) ); }
//	void insertItem( CGameItemPtr item  )	{ _Items.insert( item) ; }

	/**
	 * Drop an item on the ground
	 * \param id the item's id
	 * \param x x coordinate
	 * \param y y coordinate
	 * \param z z coordinate
	 */
//	void dropOnTheGround( const NLMISC::CEntityId& id, sint32 x, sint32 y, sint32 z ); 

	/**
	 *	Save all the player items to a file, and remove them from the base
	 */
//	void removeAndSavePlayerItems( NLMISC::IStream &f, NLMISC::CEntityId& inventory );

	/**
	 *	Get the list of item(sheet id) contained in the item
	 * \param itemId is the id of the container
	 * \param content will contains the list of item(their sheet id)
	 */
//	void getContent( NLMISC::CEntityId& itemId, std::list<NLMISC::CSheetId>& content );

	/**
	 *	Get the list of item(sheet id) contained in the item
	 * \param itemId is the id of the container
	 * \param content will contains the list of item(their sheet id)
	 */
//	void getContent( NLMISC::CEntityId& itemId, std::vector<NLMISC::CSheetId>& content );

	/**
	 *	Get the list of item(entity id and sheet id) contained in the item
	 * \param itemId is the id of the container
	 * \param content will contains the list of item(their sheet id)
	 */
//	void getContent( NLMISC::CEntityId& itemId, std::list<CEntitySheetId>& content );

	/**
	 *	Get the list of item(entity id and sheet id) contained in the item
	 * \param itemId is the id of the container
	 * \param content will contains the list of item(their sheet id)
	 */
//	void getContent( NLMISC::CEntityId& itemId, std::vector<CEntitySheetId>& content );

	/**
	 *	Dump the item list in a text file
	 * \param fileName is the name of the file
	 */
//	void dumpGameItemList( const std::string& fileName );

	/**
	 * Get current item indice and increment it for next item created
	 * \return new item indice
	 */
//	uint64 getFreeItemIndice() { return _CreatedItemCount++; }

	// build models of npc specific items
	static void buildNpcSpecificItems();

	/// get models of npc specific items
	static inline const std::vector< CGameItemPtr > & getNpcSpecificItems() {return _NpcSpecificItems;}

private :

	/**
	 *	Creat a new item and inits it with its sheet
	 * \param id the item's id
	 * \param sheetId sheet id
	 * \param destroyable true is the object is destroyable 
	 */
//	CGameItemPtr getNewItem( NLMISC::CEntityId& id, NLMISC::CSheetId& sheetId, uint16 quality, bool destroyable , bool dropable );
	CGameItemPtr getNewItem( const NLMISC::CSheetId& sheetId, uint16 quality, bool destroyable , bool dropable );

private :

	/// number of created items (used to generate new id)
//	uint64 _CreatedItemCount;

	/// All the items
//	std::map< NLMISC::CEntityId,CGameItemPtr > _Items;
//	std::set< CGameItemPtr >	_Items;
	
	/// corpses in the world
//	std::queue< CGameItemPtr > _Corpses;

	/// keep a model of each npc specific item (non craftable item)
	static std::vector< CGameItemPtr > _NpcSpecificItems;
};

extern CGameItemManager GameItemManager;

#endif // GAME_ITEM_MANAGER_H

/* End of game_item_manager.h */

