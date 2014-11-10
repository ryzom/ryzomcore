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



#ifndef RY_HARVESTABLE_H
#define RY_HARVESTABLE_H

// 
#include "mp.h"
#include "player_manager/cdb_synchronised.h"
//Nel georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "game_share/skills.h"
#include "egs_sheets/egs_static_harvestable.h"
//

class CCharacter;

/**
 * CCreatureRawMaterial
 * \author Fleury David
 * \author Nevrax France
 * \date 2002
 */
struct CCreatureRawMaterial
{	
	/**
	 *	Default constructor
	 */
	inline CCreatureRawMaterial() : Quantity(0)
	{}


	/**
	 * Serial
	 */
	inline void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( MpCommon );
		f.serial( Quantity );
		f.serial( ItemId );
	}


	/// struc used by the br
	CRawMaterial				MpCommon;
	
	/// the associated item sheet id
	NLMISC::CSheetId			ItemId;
	/// the quantity
	uint16						Quantity;
};



/**
 * Base class for harvestable creature
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CHarvestable
{
public:

	/// Constructor
	CHarvestable();

	/// Destructor
	~CHarvestable();

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialCont( _Mps );
		f.serialEnum( _HarvestSkill );
	}

	/**
	 * get the quantity available of an MP
	 */
	uint16 getQuantity( uint8 mpIndex )
	{
		if ( mpIndex >= _Mps.size() )
		{
			nlwarning("<CHarvestable::removeMp> Invalid mp index %u, there is only %u mps", mpIndex, _Mps.size() );
			return 0;
		}
		
		return _Mps[ mpIndex ].Quantity;
	}

	/**
	 * write Mp infos in the harvester database
	 */
	bool writeMpInfos();

	/**
	 * remove 'quantity' units of the specified mp
	 * \param mpIndex the mp index (1..NbRawMaterial)
	 * \param quantity the quanity of units removed
	 * \param player pointer on the CCharacter object of the harvester, tu update his databse with the new quantity
	 */
	void removeMp( uint32 mpIndex, uint16 quantity );

	/**
	 * get the mp of specified index 
	 * \param index index of the mp to retrieve
	 * \return pointer on the raw material object or NULL if invalid index
	 */
	inline const CRawMaterial *getRawMaterial( uint8 index ) const
	{
		if (index >= _Mps.size() )
		{
			nlwarning("<CHarvestable::getRawMaterial> Invalid index %u, max value is %u", index, _Mps.size() );
			return NULL;
		}
		else
			return & _Mps[index].MpCommon;
	}

	/**
	 * get the creature mp of specified index 
	 * \param index index of the mp to retrieve
	 * \return pointer on the raw material object or NULL if invalid index
	 */
	inline const CCreatureRawMaterial *getCreatureRawMaterial( uint32 index ) const
	{
		if (index >= _Mps.size() )
		{
			nlwarning("<CHarvestable::getRawMaterial> Invalid index %u, max value is %u", index, _Mps.size() );
			return NULL;
		}
		else
			return & _Mps[index];
	}

	/// set the harvester
	inline void harvesterRowId( const TDataSetRow &row) { _HarvesterRowId = row; }

	/// reset harvester row id
	inline void resetHarvesterRowId() { _HarvesterRowId = TDataSetRow(); }

	/// get the harvester
	//inline CCharacter *harvester() { return _Harvester; }
	inline const TDataSetRow &harvesterRowId() const { return _HarvesterRowId; }

	/// get the raw materials (mp) vector
	inline const std::vector<CCreatureRawMaterial> &getMps() const { return _Mps; }

	/// set harvestable flag
	inline void harvestable( bool b) { _Harvestable = b; }

	/// get harvestable flag
	inline bool harvestable() const { return _Harvestable; }

	/// get skill used to harvest 
	inline SKILLS::ESkills getHarvestSkill() const { return _HarvestSkill; }

	/// set Harvestable Mp with static information
	void setMps( const std::vector< CStaticCreatureRawMaterial>& mps );

	/// get the slot count of loot inventory at its creation
	uint getLootSlotCount() { return _LootSlotCount; }

	/// set the slot count of loot inventory at its creation
	void setLootSlotCount( uint count ) { _LootSlotCount = count; }

protected:
	/// the harvestable Mps (4 per entity)
	std::vector< CCreatureRawMaterial>	_Mps;

	/// pointer on the harvesting character
	TDataSetRow					_HarvesterRowId;

	/// skill used to harvest this creature
	SKILLS::ESkills				_HarvestSkill;

	/// flag indicating if the entity is harvestable
	bool						_Harvestable;

	/// lootable items can be quartered along with rm
	uint						_LootSlotCount;
};

#endif // RY_HARVESTABLE_H

/* End of harvestable.h */
