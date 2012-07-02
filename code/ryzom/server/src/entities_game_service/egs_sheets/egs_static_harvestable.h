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



#ifndef RY_EGS_STATIC_HARVESTABLE_H
#define RY_EGS_STATIC_HARVESTABLE_H

//Nel georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

//#include "server_share/creature_size.h"
#include "game_share/skills.h"

#include "egs_sheets/egs_static_raw_material.h"


enum TRMUsage { RMUTotalQuantity, RMUFixedQuantity, NbRMUsages };

enum TRMQuantityVariable { RMQVHerbivore, RMQVCarnivore, RMQVBoss5, RMQVBossBegin=RMQVBoss5, RMQVBoss7, RMQVBossEnd=RMQVBoss7, RMQVInvasion5, RMQVInvasion7, RMQVForceBase, NBRMQuantityVariables=RMQVForceBase+6 };

extern const float *QuarteringQuantityByVariable [NBRMQuantityVariables];


/**
 * CStaticCreatureRawMaterial
 * \author Fleury David
 * \author Nevrax France
 * \date 2002
 */
struct CStaticCreatureRawMaterial
{	
	/**
	 *	Default constructor
	 */
	inline CStaticCreatureRawMaterial()
	{}


	/**
	 * Serial
	 */
	inline void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( MpCommon );
		f.serial( UsageAndQuantity );
		f.serial( ItemId );
//		f.serial( PresenceProbabilities );
	}


	/// struc used by the brick service
	CStaticRawMaterial			MpCommon;
	
	/// the associated item sheet id
	NLMISC::CSheetId			ItemId;

	/// Return the RM usage
	TRMUsage					rmUsage() const
	{
		return (TRMUsage)UsageAndQuantity.Usage;
	}

	/**
	 * Return the RM quantity (if rmUsage() returns RMUFixedQuantity, it's quantity per RM, otherwise total quantity).
	 * After loading, it is ensured that this quantity is > 0.
	 */
	uint16						quantityVariable() const
	{
		return UsageAndQuantity.QuantityVar;
	}

	/// The RM usage of fixed quantity
	union TUsageOrQuantity
	{
		struct
		{
			uint16					Usage		: 8; // TRMUsage
			uint16					QuantityVar	: 8; // TRMQuantityVariable
		};

		uint16						Raw;

		///
		TUsageOrQuantity() : Usage((uint16)RMUFixedQuantity), QuantityVar(0) {}

		///
		void	serial( NLMISC::IStream& s ) { s.serial( Raw ); }

	} UsageAndQuantity;

};


/**
 * CStaticHarvestable
 * Class for describing harvest info for a static creature
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CStaticHarvestable
{
public:

	/// Constructor
	CStaticHarvestable();

	/// Destructor
	~CStaticHarvestable();

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialEnum( _HarvestSkill );
		f.serialCont( _Mps );
		f.serialCont( _ItemsForMissions );
	}

	/**
	 * load the infos from a georges sheet
	 * \param root the root node of the georges sheet
	 */
	void loadFromGeorges( const NLGEORGES::UForm &root, const NLMISC::CSheetId &sheetId );

	/**
	 * get the mp of specified index 
	 * \param index index of the mp to retrieve
	 * \return pointer on the raw material object or NULL if invald index
	 */
	inline const CStaticRawMaterial *getRawMaterial( uint8 index ) const
	{
		if (index >= _Mps.size() )
		{
			nlwarning("<CStaticHarvestable::getRawMaterial> Invalid index %u, max value is %u", index, _Mps.size() );
			return NULL;
		}
		else
			return & _Mps[index].MpCommon;
	}

	/// get the raw materials (mp) vector
	inline const std::vector<CStaticCreatureRawMaterial> &getMps() const { return _Mps; }

	/// get the items for missions
	inline const std::vector<NLMISC::CSheetId> &getItemsForMissions() const { return _ItemsForMissions; }

	/// get skill used to harvest 
	inline SKILLS::ESkills getHarvestSkill() const { return _HarvestSkill; }

protected:
	/// The harvestable Mps (except the ones for missions)
	std::vector <CStaticCreatureRawMaterial>	_Mps;

	/// The items (including raw material) for mission auto-quarter
	std::vector <NLMISC::CSheetId>	_ItemsForMissions;

	/// Skill used to harvest this creature
	SKILLS::ESkills				_HarvestSkill;
};

#endif // NL_HARVESTABLE_H

/* End of harvestable.h */
