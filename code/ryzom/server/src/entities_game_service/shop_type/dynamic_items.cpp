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

#include "nel/misc/command.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/net/service.h"

#include "game_share/item_family.h"
#include "game_item_manager/game_item.h"

#include "egs_sheets/egs_sheets.h"

#include "../egs_variables.h"
#include "shop_type/shop_type_manager.h"
#include "dynamic_items.h"
#include "offline_character_command.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


NL_INSTANCE_COUNTER_IMPL(CDynamicItems);

CVariable<uint32> TotalNbItemsForSale("egs","TotalNbItemsForSale","Total number of items for sale (read-only)", 0, 0, false);
extern CVariable<bool> EGSLight;


//-----------------------------------------------------------------------------
// Persistent data for CItemsForSale
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CDynamicItems
#define PERSISTENT_STORE_ARGS uint index
#define PERSISTENT_APPLY_ARGS uint index

#define PERSISTENT_DATA\
	LSTRUCT_VECT(_DynamicItems,\
					VECT_LOGIC(_DynamicItems[index]), \
					((CItemForSale*)&*(_DynamicItems[index][ i ]))->store(pdr), \
					{\
						CItemForSale *item = new CItemForSale; \
						item->apply(pdr); \
						/* Determine the correct slot */ \
						uint32 slot = getSubVectorIndex(item->getOwner()); \
						_DynamicItems[slot].push_back(item);\
						TotalNbItemsForSale = ++_TotalNbItemsForSale;\
					}\
				)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

#undef PERSISTENT_DATA
#undef PERSISTENT_APPLY_ARGS
#undef PERSISTENT_STORE_ARGS
#undef PERSISTENT_CLASS

// CDynamicItems  implementation

CDynamicItems * CDynamicItems::_Instance = 0;


//-----------------------------------------------------------------------------
CDynamicItems::CDynamicItems()
{
	_NextVectorToCheck = 0;
	_NextItemToCheck = 0;
	_NextVectorToSave = 0;
	_TotalNbItemsForSale = 0;
	_InitOk = false;
}


//-----------------------------------------------------------------------------
CDynamicItems * CDynamicItems::getInstance()
{
	if( _Instance == 0 )
	{
		_Instance = new CDynamicItems();
		nlassert( _Instance != 0 );
	}
	return _Instance;	
}


//-----------------------------------------------------------------------------
uint32 CDynamicItems::getSubVectorIndex(const NLMISC::CEntityId& characterId)
{
	return PlayerManager.getPlayerId(characterId)&SUB_VECTOR_MASK;
}

//-----------------------------------------------------------------------------
uint32 CDynamicItems::getSubVectorIndex(const TItemTradePtr& item)
{
	return PlayerManager.getPlayerId(item->getOwner())&SUB_VECTOR_MASK;
}

//-----------------------------------------------------------------------------
CDynamicItems::TItemTradeVector &CDynamicItems::getSubVector(const TItemTradePtr& item)
{
	return _DynamicItems[getSubVectorIndex(item)];
}

//-----------------------------------------------------------------------------
bool CDynamicItems::addDynamicItemForSell( TItemTradePtr& item )
{
	H_AUTO(CDynamicItems_addDynamicItemForSell);

	//add item in right shop unit
	if(	CShopTypeManager::addItemInShopUnitDynamic( item ) )
	{
		// select the sub vector according to owner id
		getSubVector(item).push_back( item );
		TotalNbItemsForSale = ++_TotalNbItemsForSale;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CDynamicItems::removeDynamicItemForSell( TItemTradePtr& item )
{
	H_AUTO(CDynamicItems_removeDynamicItemForSell);

	TItemTradeVector &subVec = getSubVector(item);
	for (uint i=0; i<subVec.size(); ++i)
	{
		if( subVec[i] == item )
		{	
			//remove item from right shop unit
			if( CShopTypeManager::removeItemFromShopUnitDynamic( item ) )
			{
				subVec[ i ] = subVec.back();
				subVec.pop_back();
				TotalNbItemsForSale = --_TotalNbItemsForSale;
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void CDynamicItems::getSavePath(std::string &path)
{
	path = toString("sale_store/");
}
//-----------------------------------------------------------------------------

void CDynamicItems::makeFileName(uint subIndex, std::string &fileName)
{
	string path;
	getSavePath(path);
	fileName = path+toString("sale_store_%04u_pdr", subIndex);
	if (XMLSave)
		fileName += ".xml";
	else
		fileName += ".bin";
}


struct CItemsFileCb : public IBackupFileReceiveCallback
{
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		CDynamicItems::getInstance()->fileAvailable(fileDescription, dataStream);
	}
};


void CDynamicItems::fileAvailable(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	static CPersistentDataRecord	pdr;
	pdr.clear();

	pdr.fromBuffer(dataStream);
	apply(pdr, 0);
}

//-----------------------------------------------------------------------------
void CDynamicItems::init()
{
	nlinfo("Loading all sale store items...");
	TTime start = NLMISC::CTime::getLocalTime();

	// Check save directory
//	string path;
//	getSavePath(path);
//	path = Bsi.getLocalPath() + path;
//	if (!CFile::isExists(path))
//	{
//		// create the save directory
//		CFile::createDirectoryTree(path);
//	}

	_NextVectorToCheck = 0;
	_NextItemToCheck = 0;
	_NextVectorToSave = 0;
	_LastSaveTick = CTickEventHandler::getGameCycle();

	if (EGSLight)
		return;

	CItemsFileCb *cb = new CItemsFileCb;

	// build the file list
	vector<string> fileNames;
	for (uint i=0; i<NUM_SUB_VECTORS; ++i)
	{
		string fileName;
		makeFileName(i, fileName);


//		_CurrentLoadIndex = i;
		fileNames.push_back(fileName);
	}

	/// load the files
	Bsi.syncLoadFiles(fileNames, cb);

	// post load treatment

	for (uint i=0; i<NUM_SUB_VECTORS; ++i)
	{
		TItemTradeVector &subVec = _DynamicItems[i];
		
		for( uint32 j = 0; j < subVec.size(); ++j )
		{
			//add item in right shop unit
			CShopTypeManager::addItemInShopUnitDynamic( subVec[ j ] );
		}
	}


//		Bsi.syncLoadFile(fileName, cb);

//		fileName = Bsi.getLocalPath() + fileName;
//
//		if (CFile::isExists(fileName))
//		{
//			static CPersistentDataRecord	pdr;
//			pdr.clear();
//			
//			pdr.readFromFile(fileName.c_str());
//			apply(pdr, i);
//
//			for( uint32 j = 0; j < subVec.size(); ++j )
//			{
//				//add item in right shop unit
//				CShopTypeManager::addItemInShopUnitDynamic( subVec[ j ] );
//			}
//		}
//	}

//	///////////////////////////////////////////////////////////
//	/// Conversion/compatibility code 
//	/// This code initially write because the first storage mode 
//	/// concentrate item in only 5/16th of the 1024 storage file.
//	///////////////////////////////////////////////////////////
	

//	string versionFile = path+"sale_store_version.bin";
	// check version file for conversion
//	if (!CFile::isExists(versionFile))
//	{
//		nlinfo("Converting sale store item from initial format to format 1");
//		// no version file, this is the initial storage version
//		vector<TItemTradePtr>	allItems;
//		for (uint i=0; i<NUM_SUB_VECTORS; ++i)
//		{
//			TItemTradeVector &tv = _DynamicItems[i];
//
//			for (uint j=0; j<tv.size(); ++j)
//			{
//				allItems.push_back(tv[j]);
//			}
//			// cleanup vector
//			tv.clear();
//		}
//
//		TotalNbItemsForSale = 0;
//		_TotalNbItemsForSale = 0;
//
//		// rebuild container
//		for (uint i=0; i<allItems.size(); ++i)
//		{
//			// inset the item
//			addDynamicItemForSell(allItems[i]);	
//		}
//
//		// save the new container
//		saveAll();
//
//		// write the version file
////		COFile version(versionFile);
////		version.serialVersion(SALE_STORE_VERSION);
//	}
//	else
//	{
//		// version file exist, check correctness
//	CIFile version(versionFile);
//	uint v;
//	v = version.serialVersion(SALE_STORE_VERSION);
//
//	nlassert(v == SALE_STORE_VERSION);
//	}

	TTime end = NLMISC::CTime::getLocalTime();
	nlinfo("All items loaded in %u ms", uint32(end-start) );

	_InitOk = true;
}

//-----------------------------------------------------------------------------
void CDynamicItems::saveAll()
{
	if( _InitOk )
	{
		nlinfo("Saving all sale store items...");
		TTime start = NLMISC::CTime::getLocalTime();
		for (uint i=0; i<NUM_SUB_VECTORS; ++i)
		{
			save(i);
		}

		TTime end = NLMISC::CTime::getLocalTime();
		nlinfo("Save all items in %u ms", uint32(end-start) );
	}
	else
	{
		nlinfo("Sale store not saved because Dynamic item sale store not loaded" );
	}
}

//-----------------------------------------------------------------------------
void CDynamicItems::save(uint subIndex)
{
	H_AUTO(CDynamicItems_save);
	
	static CPersistentDataRecordRyzomStore	pdr;
	pdr.clear();
	store(pdr, subIndex);

	string fileName;
	makeFileName(subIndex, fileName);

	CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, Bsi );

	if (XMLSave)
	{
		string s;
		pdr.toString(s);
		msg.DataMsg.serialBuffer((uint8*)&s[0], (uint)s.size());
	}
	else
	{
		uint size = pdr.totalDataSize();
		vector<char> buffer(size);
		pdr.toBuffer(&buffer[0], size);
		msg.DataMsg.serialBuffer((uint8*)&buffer[0], size);
	}
	
	Bsi.sendFile( msg );
}	

//-----------------------------------------------------------------------------
void CDynamicItems::tickUpdate()
{
	H_AUTO(CDynamicItems_tickUpdate);
	// check if item are reach maximum time in sale store
	TItemTradeVector &subVec = _DynamicItems[_NextVectorToCheck];

	enum TTradeItemStatus { ok = 0, time_expired = 1, not_available = 2 };

	if( _NextItemToCheck < subVec.size() )
	{
		TTradeItemStatus status = ok;

		if( CTickEventHandler::getGameCycle() - subVec[ _NextItemToCheck]->getStartSaleCycle() + subVec[ _NextItemToCheck]->getItemPtr()->getTotalSaleCycle() > MaxGameCycleSaleStore )
		{
			status = time_expired;
		}
		else if( subVec[ _NextItemToCheck]->isAvailable( 1 ) == false )
		{
			status = not_available;
		}

		if( status != ok )
		{
			// store quantity before remove item, else it's lost
			uint32 quantity = subVec[ _NextItemToCheck]->getQuantity();

			// must be removed before send offline command, because CMaximumShopStoreTimeReached class a method in CCharacter that checking quantity left
			while ( ! CShopTypeManager::removeItemFromShopUnitDynamic( subVec[ _NextItemToCheck] )  );

			// send offline command to character for inform it his item are reached maximum sell store time
			if( status == time_expired )
			{
				string command;
				CMaximumShopStoreTimeReached::makeStringCommande( command, subVec[ _NextItemToCheck]->getOwner(), subVec[ _NextItemToCheck]->getSheetId(), quantity, subVec[ _NextItemToCheck]->getIdentifier() );
				COfflineCharacterCommand::getInstance()->addOfflineCommand( command );
			}
			
			subVec[_NextItemToCheck] = subVec.back();
			subVec.pop_back();
			TotalNbItemsForSale = --_TotalNbItemsForSale;
		}
		else
		{
			// advance to next item in current vector
			++_NextItemToCheck;
		}
	}
	else
	{
		// advance to next vector
		_NextVectorToCheck++;
		_NextVectorToCheck &= SUB_VECTOR_MASK;
		_NextItemToCheck = 0;
	}

	// +5 is used to desync guild and store save
	if( ( (CTickEventHandler::getGameCycle()+5) - _LastSaveTick ) > StoreSavePeriod	)
	{
		_LastSaveTick = CTickEventHandler::getGameCycle();
		save(_NextVectorToSave++);

		_NextVectorToSave &= SUB_VECTOR_MASK;
	}
}

//-----------------------------------------------------------------------------
void CDynamicItems::getItemsOfCharacter( const NLMISC::CEntityId& charId, std::vector< TItemTradePtr >& itemsForSaleOfCharacter )
{
	TItemTradeVector &subVec = _DynamicItems[getSubVectorIndex(charId)];
	for( uint i = 0; i < subVec.size(); ++i )
	{
		if( subVec[ i ]->getOwner() == charId )
		{
			itemsForSaleOfCharacter.push_back( subVec[ i ] );
		}
	}
}


