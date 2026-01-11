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
#include "egs_static_outpost.h"

#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

//----------------------------------------------------------------------------
// CStaticOutpostBuilding
//----------------------------------------------------------------------------

//--------------------------------------------------------------
CStaticOutpostBuilding::TType CStaticOutpostBuilding::fromString( const string & str )
{
	if (str == "TownHall") return TypeTownHall;
	if (str == "Driller") return TypeDriller;
	return TypeEmpty;
}
//--------------------------------------------------------------
std::string CStaticOutpostBuilding::toString( CStaticOutpostBuilding::TType type )
{
	if (type == TypeTownHall) return "TownHall";
	if (type == TypeDriller) return "Driller";
	return "Empty";
}

//----------------------------------------------------------------------------
uint CStaticOutpostBuilding::getVersion ()
{
	return 3;
}

//----------------------------------------------------------------------------
void CStaticOutpostBuilding::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	string value;
	nlverify (root.getValueByName (value, "type"));
	Type = fromString(value);
	nlverify (root.getValueByName (CanBeDestroyedOrBuilt, "can_be_destroyed_or_built"));
	nlverify (root.getValueByName (Name, "name"));
	nlverify (root.getValueByName (NameWhenConstructing, "name_when_constructing"));
	nlverify (root.getValueByName (value, "shape"));
	Shape = CSheetId( value );
	if (Shape == CSheetId::Unknown)
		nlwarning("<CStaticOutpostBuilding::readGeorges> Shape %s is not valid", value.c_str());

	nlverify (root.getValueByName (value, "shape_when_constructing"));
	ShapeWhenConstructing = CSheetId( value );

	nlverify (root.getValueByName (CostDapper, "cost_dapper"));
	nlverify (root.getValueByName (CostTime, "cost_time"));

	// Upgrades
	UFormElm *arrayUpgradeBuildings = NULL;
	if (root.getNodeByName(&arrayUpgradeBuildings, "upgrade"))
	{
		if (arrayUpgradeBuildings != NULL)
		{
			uint size;
			nlverify(arrayUpgradeBuildings->getArraySize(size));
			Upgrade.resize(size);

			for (uint i = 0; i < size; ++i)
			{
				string value;
				arrayUpgradeBuildings->getArrayValue(value, i);
				CSheetId buildingSheetId = CSheetId(value);
				if (buildingSheetId == CSheetId::Unknown)
					nlwarning("<CStaticOutpostBuilding::readGeorges> Upgrade building '%s' is not valid", value.c_str());

				Upgrade[i] = buildingSheetId;
			}
		}
	}

	// Driller part
	UFormElm *pDrillerPart = NULL;
	if (root.getNodeByName(&pDrillerPart, "driller"))
		if (pDrillerPart != NULL)
			Driller.readGeorges(pDrillerPart);
}

//----------------------------------------------------------------------------
void CStaticOutpostBuilding::serial(NLMISC::IStream &f)
{
	f.serialEnum(Type);
	f.serial(CanBeDestroyedOrBuilt);
	f.serial(Name);
	f.serial(NameWhenConstructing);
	f.serial(Shape);
	f.serial(ShapeWhenConstructing);
	f.serial(CostDapper);
	f.serial(CostTime);
	f.serialCont(Upgrade);
	switch(Type)
	{
		case TypeDriller: Driller.serial(f); break;
		default: break;
	}
}

//----------------------------------------------------------------------------
// CStaticOutpostBuilding::CDriller
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CStaticOutpostBuilding::CDriller::readGeorges (const NLGEORGES::UFormElm *pElt)
{
	static uint32 itemSheetType = CSheetId::typeFromFileExtension("sitem");

	// quality factors
	float totalFactor = 0.0f;
	for (uint i = 0; i < DRILLER_NB_LEVEL; ++i)
	{
		string sPath = NLMISC::toString("quality_%03d",  (i+1)*(250/DRILLER_NB_LEVEL));
		nlverify (pElt->getValueByName (QualityFactor[i], sPath.c_str()));
		totalFactor += QualityFactor[i];
	}

	// Mps and Mp quantities (harcoded to 20 max)
	MPs.clear();
	MPQuantities.clear();
	TotalMP = 0.0f;
	for (uint i = 0; i < 20; ++i)
	{
		string namePath = NLMISC::toString("mp%d.name", i);
		string quantityPath = NLMISC::toString("mp%d.quantity", i);
		string	name;
		float	quantity;
		pElt->getValueByName (name, namePath.c_str());
		pElt->getValueByName (quantity, quantityPath.c_str());
		if(!name.empty() && quantity>0.f)
		{
			CSheetId mpSheetId = CSheetId(name);
			if (mpSheetId != CSheetId::Unknown && mpSheetId.getSheetType() == itemSheetType)
			{
				MPs.push_back(mpSheetId);
				MPQuantities.push_back(quantity);
				TotalMP += quantity;
			}
			else
				nlwarning("<CStaticOutpostBuilding::CDriller::readGeorges> Mp sheet '%s' is not valid", name.c_str());
		}
	}
	TotalMP *= totalFactor;
}

//----------------------------------------------------------------------------
void CStaticOutpostBuilding::CDriller::serial(class NLMISC::IStream &f)
{
	for (uint i = 0; i < DRILLER_NB_LEVEL; ++i)
		f.serial(QualityFactor[i]);
	f.serialCont(MPs);
	f.serialCont(MPQuantities);
	f.serial(TotalMP);
}


//----------------------------------------------------------------------------
// CStaticOutpost
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
uint CStaticOutpost::getVersion ()
{
	return 3;
}

//----------------------------------------------------------------------------
void CStaticOutpost::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	nlverify (root.getValueByName (MaxSpawnSquadCount, "Max Number of Spawned Squads"));
	nlverify (root.getValueByName (ChallengeCost, "Challenge Cost"));
	nlverify (root.getValueByName (Level, "Level"));
	nlverify (root.getValueByName (MinimumTribeRoundLevel, "MinimumTribeRoundLevel"));
	nlverify (root.getValueByName (MinimumGuildRoundLevel, "MinimumGuildRoundLevel"));
}

//----------------------------------------------------------------------------
void CStaticOutpost::serial(NLMISC::IStream &f)
{
	f.serial(MaxSpawnSquadCount);
	f.serial(ChallengeCost);
	f.serial(Level);
	f.serial(MinimumTribeRoundLevel);
	f.serial(MinimumGuildRoundLevel);
}

//----------------------------------------------------------------------------
// CStaticOutpostSquad
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
uint CStaticOutpostSquad::getVersion ()
{
	return 4;
}

//----------------------------------------------------------------------------
void CStaticOutpostSquad::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	nlverify (root.getValueByName (BuyPrice, "Buy Price"));
}

//----------------------------------------------------------------------------
void CStaticOutpostSquad::serial( NLMISC::IStream &f)
{
	f.serial(BuyPrice);
}

