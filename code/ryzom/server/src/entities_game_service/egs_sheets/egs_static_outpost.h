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


#ifndef RY_EGS_STATIC_OUTPOST_H
#define RY_EGS_STATIC_OUTPOST_H

#include "nel/misc/types_nl.h"
//Nel georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"


const uint DRILLER_NB_LEVEL = 5; // 50, 100, 150, 200, 250

/**
 * Georges form class for outpost
 * \author Matthieu Besson
 * \author Nevrax France
 * \date September 2005
 */
class CStaticOutpostBuilding
{
public:

	enum TType
	{
		TypeEmpty,
		TypeTownHall,
		TypeDriller
	};
	static TType fromString( const std::string & str );
	static std::string toString( TType type );

public:
	
	CStaticOutpostBuilding() { Type = TypeEmpty; }
	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Serial
	void serial(NLMISC::IStream &f);
	/// Return the version of this class, increments this value when the content of this class has changed
	static uint getVersion ();
	
	/// Removed
	void removed() {}

	/// Type of the building
	TType				Type;

	/// is the building can be destroyed ?
	bool				CanBeDestroyedOrBuilt;

	/// name of the building (to be displayed in the client)
	std::string			Name;

	/// name when constructing
	std::string			NameWhenConstructing;
	
	/// .creature associated
	NLMISC::CSheetId	Shape;

	/// .creature used when the building is under construction
	NLMISC::CSheetId	ShapeWhenConstructing;

	/// dapper cost
	uint32				CostDapper;
	
	/// time cost in seconds
	uint32				CostTime;

	/// upgrades of this building
	std::vector<NLMISC::CSheetId>	Upgrade;

	/// Driller structure for managing mp production 
	/// There is TotalMP = QualityFactor[i]*MPQuantities[j] mp of type MPs[j] produced by OutpostDrillerTimeUnit seconds
	struct CDriller
	{
		float							QualityFactor[DRILLER_NB_LEVEL];
		std::vector<NLMISC::CSheetId>	MPs;
		std::vector<float>				MPQuantities;
		float							TotalMP; ///< Total mp produced by the driller in 'OutpostDrillerTimeUnit' seconds
		/// Read georges sheet (driller subsection)
		void readGeorges(const NLGEORGES::UFormElm *pElt);
		/// Serialize
		void serial(NLMISC::IStream &f);
	};

	CDriller	Driller;
};

/**
 * Georges form class for outpost
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2005
 */
class CStaticOutpost
{
public:

	CStaticOutpost(){}
	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Serial
	void serial(class NLMISC::IStream &f);
	/// Return the version of this class, increments this value when the content of this class has changed
	static uint getVersion ();
	
	/// Removed
	void removed() {}

	/// cost to challenge the outpost in dapper
	uint32								ChallengeCost;
	
	/// maximum number of standard squad spawned
	uint16								MaxSpawnSquadCount;

	/// level of the outpost
	uint8								Level;
	
	uint8								MinimumTribeRoundLevel;
	uint8								MinimumGuildRoundLevel;
};

/**
 * Georges form class for outpost squad
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2005
 */
class CStaticOutpostSquad
{
public:

	CStaticOutpostSquad(){}
	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Serial
	void serial(class NLMISC::IStream &f);
	/// Return the version of this class, increments this value when the content of this class has changed
	static uint getVersion ();
	
	/// Removed
	void removed() {}

	/// buy price of the squad
	uint32	BuyPrice;
};


#endif // RY_EGS_STATIC_OUTPOST_H

/* End of egs_static_outpost.h */
