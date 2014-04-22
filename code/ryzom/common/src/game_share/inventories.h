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



#ifndef RY_INVENTORIES_H
#define RY_INVENTORIES_H

#include "nel/misc/types_nl.h"
#include <stdio.h>
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/enum_bitset.h"
#include "nel/net/service.h"

/* ******
define for pet animal
WARNING!!!!! If you change MAX_INVENTORY_ANIMAL value, you'll have to:
- change code around all nlctassert that should raise because of change
- search in file MAX_INVENTORY_ANIMAL in all XML (including database.xml AND local_database.xml) to do correct changes
* ******/
#define MAX_PACK_ANIMAL					3
#define MAX_MEKTOUB_MOUNT				1
#define MAX_OTHER_PET					0
#define MAX_INVENTORY_ANIMAL			( MAX_PACK_ANIMAL + MAX_MEKTOUB_MOUNT )

// This give by which value the WEIGHT in database must be divided to get the value in Kg
// valueKg= valueDb / DB_WEIGHT_SCALE
#define DB_WEIGHT_SCALE		100


namespace INVENTORIES
{
	/** Domain wide unique id for item (on 64 bits)
	 *	Use a combination of unix time, shardId and current second serial number.
	 */
	struct TItemId
	{
		/// create a new item id
		TItemId()
		{
			uint32 now = NLMISC::CTime::getSecondsSince1970();
			if (now != _LastCreation)
			{
				_LastCreation = now;
				_SerialNumber = 0;
			}

			_CreateTime = now;
			_SerialNumber = _NextSerialNumber++;
			_ShardId = NLNET::IService::getInstance()->getShardId();
		}

		/// Build from a raw id value
		TItemId(uint64 rawId)
		{
			_RawId = rawId;
		}

		/// build an item id from a string
		TItemId(const std::string &idStr)
		{
			_RawId = 0;

			uint32 shardId = 0;
			uint32 serialNum = 0;
			if (sscanf(idStr.c_str(), "[%u:%x:%u]", &shardId, &_CreateTime, &serialNum) == 3)
			{
				_ShardId = shardId;
				_SerialNumber = serialNum;
			}
		}

		/// Get the raw id value
		uint64 getRawId() const
		{
			return _RawId;
		}

		/// hum... serial
		void serial(NLMISC::IStream &s)
		{
			s.serial(_RawId);
		}

		/// convert to a standard string representation (that can be parsed back as itemId)
		std::string toString() const
		{
			return NLMISC::toString("[%u:0x%x:%u]", _ShardId, _CreateTime, _SerialNumber);
		}

		bool operator == (const TItemId &other) const
		{
			return _RawId == other._RawId;
		}

		bool operator < (const TItemId &other) const
		{
			return _RawId < other._RawId;
		}

	private:
		union
		{
			// the 64 bits row ID
			uint64 _RawId;
			struct
			{
				// creation date in unix time
				uint32	_CreateTime;
				// the serial number (during the creation second)
				uint32	_SerialNumber	: 24;
				// the shard id
				uint32	_ShardId		: 8;
			};
		};
		/// Static counter to remember the date of the current serial
		static uint32 _LastCreation;
		/// The serial number in the current second
		static uint32 _NextSerialNumber;
	};


	enum THandSlot
	{
		right = 0,
			left,
			NB_HAND_SLOT
	};

	enum TInventory
	{
		// TODO : remove handling, merge it with equipement
		handling = 0,
			temporary,						// 1
			equipment,						// 2
			bag,							// 3
			pet_animal,						// 4 Character can have 5 pack animal
			pet_animal1 = pet_animal,	// for toString => TInventory convertion
			pet_animal2,
			pet_animal3,
			pet_animal4,
			max_pet_animal,					// 8
			NUM_INVENTORY = max_pet_animal,	// 8
			UNDEFINED = NUM_INVENTORY,		// 8

			exchange,						// 9  This is not a bug : exchange is a fake inventory
			exchange_proposition,			// 10  and should not count in the number of inventory
			// same for botChat trading.
			trading,						// 11
			reward_sharing,					// 12 fake inventory, not in database.xml. Used by the item info protocol only
			guild,							// 13 (warning: number stored in guild saved file)
			player_room,					// 14
			NUM_ALL_INVENTORY				// warning: distinct from NUM_INVENTORY
	};

	/**
	* Convert an inventory name to a TInventory enum
	* \param str the input string
	* \return the TInventory associated to this string (UNDEFINED if the string cannot be interpreted)
	*/
	TInventory toInventory(const std::string &str);

	/**
	* Return inventory name
	* \param inv TInventory identify
	* \return name of inventory (or unknown if nit exist )
	*/
	const std::string& toString( TInventory inv );

	/**
	* get the name of an inventory in the data base. Use this API to fill the user database
	* \return if the specified inventory is supported
	* \param inventory : the inventory enum
	* \param retValue ; the returned string
	*/
	//bool getDatabaseString( std::string & retValue,TInventory inventory );



	extern const char *DatabaseStringFromEInventory [NUM_ALL_INVENTORY];

	const uint MaxNbPackers = MAX_INVENTORY_ANIMAL;

	class CInventoryCategoryForCharacter
	{
	public:
		enum TInventoryId
		{
			Bag,
				Packers,
				Room=Packers+MaxNbPackers,
				InvalidInvId,
				NbInventoryIds=InvalidInvId
		};
		static const char *InventoryStr [NbInventoryIds];
		static const uint InventoryNbSlots [NbInventoryIds];
		enum TSlotBitSize
		{
			SlotBitSize = 10
		};

		static std::string getDbStr( TInventoryId invId );
		static bool needPlainInfoVersionTransfer() { return false; } // incrementation is sufficient
	};

	class CInventoryCategoryForGuild
	{
	public:
		enum TInventoryId
		{
			GuildInvId,
				InvalidInvId,
				NbInventoryIds=InvalidInvId
		};
		static const char *InventoryStr [NbInventoryIds];
		static const uint InventoryNbSlots [NbInventoryIds];
		enum TSlotBitSize
		{
			SlotBitSize = 10
		};

		static std::string getDbStr( TInventoryId invId );
		static bool needPlainInfoVersionTransfer() { return true; } // incrementation is not sufficient because can incremented when player offline, and some values are skipped
	};

	//const uint NbBitsForInventoryId = 3; // must include InvalidInvId
	/* ******
	WARNING!!!!! If you change those value, you'll have to:
	- change database.xml AND local_database.xml
	- change slotids server<->client sizes
	* *******/
	const uint NbBagSlots = 500;
	const uint NbPackerSlots = 500;
	const uint NbRoomSlots = 1000;
	const uint NbGuildSlots = 1000;
	const uint NbTempInvSlots = 16;

	enum TItemPropId
	{
		Sheet,
			Quality,
			Quantity,
			UserColor,
			Locked,
			Weight,
			NameId,
			Enchant,
			ItemClass,
			ItemBestStat,
			Price,
			ResaleFlag,
			PrerequisitValid,
			Worned,
			NbItemPropId
	};

	const uint NbBitsForItemPropId = 4; // TODO: replace this constant by an inline function using NbItemPropId

	const uint LowNumberBound = 7;
	const uint LowNumberBits = 3;
	const uint InfoVersionBitSize = 8;
	extern const char *InfoVersionStr;


	/// toString(), alternate version for TInventoryId
	inline const std::string toString( CInventoryCategoryForCharacter::TInventoryId invId )
	{
		return std::string(CInventoryCategoryForCharacter::InventoryStr[invId]);
	}

	/// toString(), alternate version for TInventoryId
	inline const std::string toString( CInventoryCategoryForGuild::TInventoryId invId )
	{
		return std::string(CInventoryCategoryForGuild::InventoryStr[invId]);
	}

	/// Return the inventory db root string
	inline std::string CInventoryCategoryForCharacter::getDbStr( TInventoryId invId )
	{
		return std::string("INVENTORY:") + INVENTORIES::toString( invId );
	}

	/// Return the inventory db root string
	inline std::string CInventoryCategoryForGuild::getDbStr( TInventoryId invId )
	{
		return INVENTORIES::toString( invId ) + ":INVENTORY";
	}


	/**
	* Slot in inventory (bag, room, etc.)
	*
	* TODO: Check version at client startup
	*/
	class CItemSlot
	{
	public:

		static const uint DataBitSize [NbItemPropId];
		static const char *ItemPropStr [NbItemPropId];

		/// Return the version
		static uint16	getVersion() { return 0; }

		/// Default constructor
		CItemSlot() {}

		/// Constructor. Warning: does not reset the values!
		explicit CItemSlot( uint32 slotIndex ) : _SlotIndex(slotIndex) {}

		/// Change the slot index
		void	setSlotIndex( uint32 slotIndex ) { _SlotIndex = slotIndex; }

		/// Return the slot index
		uint32	getSlotIndex() const { return _SlotIndex; }

		/// Set all properties to 0
		void	reset()
		{
			for ( uint i=0; i!=NbItemPropId; ++i )
				_ItemProp[i] = 0;
		}

		/// Set a property
		void	setItemProp( TItemPropId id, sint32 value )
		{
			_ItemProp[id] = value;
		}

		/// Get a property
		sint32	getItemProp( TItemPropId id ) const
		{
			return _ItemProp[id];
		}

		/// Serial from/to bit stream
		template <class CInventoryCategoryTemplate>
			void	serialAll( NLMISC::CBitMemStream& bms, const CInventoryCategoryTemplate * /* templ */ =0 )
		{
			bms.serial( _SlotIndex, CInventoryCategoryTemplate::SlotBitSize );

			uint i;
			// SHEET, QUALITY, QUANTITY and USER_COLOR never require compression
			for (i = 0; i < 4; ++i)
				bms.serial((uint32&)_ItemProp[i], DataBitSize[i]);

			// For all other the compression is simple the first bit indicates if the value is zero
			if (bms.isReading())
			{
				for (; i < NbItemPropId; ++i)
				{
					bool b;
					bms.serialBit(b);
					if (b)
						_ItemProp[i] = 0;
					else
						bms.serial((uint32&)_ItemProp[i], DataBitSize[i]);
				}
			}
			else
			{
				for (; i != NbItemPropId; ++i)
				{
					bool b = (_ItemProp[i] == 0);
					bms.serialBit(b);
					if (!b)
						bms.serial((uint32&)_ItemProp[i], DataBitSize[i]);
				}
			}
		}

		/// Serial from/to bit stream
		template <class CInventoryCategoryTemplate>
			void	serialOneProp( NLMISC::CBitMemStream& bms, const CInventoryCategoryTemplate * /* templ */ =0 )
		{
			bms.serial( _SlotIndex, CInventoryCategoryTemplate::SlotBitSize );
			bms.serial( _OneProp );
		}

		/// Set all properties from another object
		void	copyFrom( const CItemSlot& src )
		{
			for ( uint i=0; i!=NbItemPropId; ++i )
			{
				_ItemProp[i] = src._ItemProp[i];
			}
		}

		// Assignment operator
		CItemSlot&	operator=( const CItemSlot& src )
		{
			copyFrom( src );
			_SlotIndex = src._SlotIndex;
			return *this;
		}

private:

	struct COneProp
	{
		TItemPropId				ItemPropId;
		sint32					ItemPropValue;

		void serial( NLMISC::CBitMemStream& bms )
		{
			bms.serial( (uint32&)ItemPropId, NbBitsForItemPropId );
			bms.serial( (uint32&)ItemPropValue, CItemSlot::DataBitSize[ItemPropId] );
		}
	};

	union
	{
		/// All item properties
		sint32		_ItemProp [NbItemPropId];

		/// Only one prop (needs to be here because a CItemSlot can't be stored in an union because of the constructors)
		COneProp	_OneProp;
	};

	/// Slot number
	uint32			_SlotIndex;

	public:

		// Accessors (for internal use only)
		COneProp&		getOneProp() { return _OneProp; }

};


// Define constants for inventory modification flag (TODO: add ic_slot_count)
enum TInventoryChange
{
	ic_total_bulk		= 1<<0,
	ic_total_weight		= 1<<1,
	ic_item_list		= 1<<2,
	ic_other			= 1<<3,
};

typedef NLMISC::CEnumBitset<TInventoryChange>	TInventoryChangeFlags;

// Define constants for item modification callback flag
enum TItemChange
{
	itc_bulk			= 1<<0,
	itc_weight			= 1<<1,
	itc_enchant			= 1<<2,
	itc_hp				= 1<<3,
	itc_inserted		= 1<<4,
	itc_removed			= 1<<5,
	itc_lock_state		= 1<<6,
	itc_info_version	= 1<<7,
	itc_worned			= 1<<8,
};

typedef NLMISC::CEnumBitset<TItemChange>	TItemChangeFlags;

enum	// for pseudo constants
{
	INSERT_IN_FIRST_FREE_SLOT = 0xFFFFFFFF,
	REMOVE_MAX_STACK_QUANTITY = 0xFFFFFFFF,
	INVALID_INVENTORY_SLOT = INSERT_IN_FIRST_FREE_SLOT,
};


} // namespace INVENTORIES

#endif // RY_INVENTORIES_H

/* End of inventories.h */





















