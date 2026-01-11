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

#ifndef RYZOM_DYNAMIC_ITEM_H
#define RYZOM_DYNAMIC_ITEM_H

// Misc

#include "game_share/item_type.h"
#include "game_share/backup_service_interface.h"

#include "item_for_sale.h"

#include <vector>

/**
 * CDynamicItems singleton
 * list of items selling by players characters
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 */
class CDynamicItems
{
	NL_INSTANCE_COUNTER_DECL(CDynamicItems);
public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);
	
	DECLARE_PERSISTENCE_METHODS_WITH_ARG(uint index)

	// return an instance on singleton
	static CDynamicItems * getInstance();

	// add dynamic item for sell
	bool addDynamicItemForSell( TItemTradePtr& item );

	// remove dynamic item for sell
	void removeDynamicItemForSell( TItemTradePtr& item );

	// init CDynamicItems, load save file
	void init();

	// save all sub vector
	void saveAll();

	// tickupdate
	void tickUpdate();
	
	// return item in _DynamicItems for character
	void getItemsOfCharacter( const NLMISC::CEntityId& charId, std::vector< TItemTradePtr >& itemsForSaleOfCharacter );

	// return true if dynamic items store initialized (= loaded)
	inline bool isInit() { return _InitOk; }

	// backup service callback
	void fileAvailable(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

private:

	CDynamicItems();

	// typedef for base sub vectors
	typedef std::vector<TItemTradePtr>		TItemTradeVector;

	// get the save directory path (does not include SaveFilesDirectory, only relative path)
	void getSavePath(std::string &path);

	// Build the file name for the sub vector (does not include SaveFilesDirectory, only relative path)
	void makeFileName(uint subIndex, std::string &fileName);

	// return the sub index according to owner
	uint32 getSubVectorIndex(const NLMISC::CEntityId& characterId);

	// return the sub index according to item owner
	uint32 getSubVectorIndex(const TItemTradePtr& item);

	TItemTradeVector &getSubVector(const TItemTradePtr& item);

	// save dynamic items content sub vector
	void save(uint subIndex );



	enum // for speudo contantes
	{	
		SALE_STORE_VERSION = 1,
		// 1024 vectors 
		NUM_SUB_VECTORS = 1<<10,
		// mask for sub vector index
		SUB_VECTOR_MASK = NUM_SUB_VECTORS-1,
	};

	static CDynamicItems *	_Instance;

	TItemTradeVector		_DynamicItems[NUM_SUB_VECTORS];
//	std::vector< TItemTradePtr >	_DynamicItems;
	NLMISC::TGameCycle		_LastSaveTick;
	uint32					_NextVectorToSave;
	uint32					_NextVectorToCheck;
	uint32					_NextItemToCheck;
	uint32					_TotalNbItemsForSale;

	bool					_InitOk;

	uint32					_CurrentLoadIndex;
};

#endif // RYZOM_STATIC_ITEM_H

/* dynamic_items.h */

