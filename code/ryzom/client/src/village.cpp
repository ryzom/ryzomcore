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


#include "village.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/vector.h"
#include "nel/misc/vector_2f.h"
#include "client_sheets/village_sheet.h"
#include "client_sheets/entity_sheet.h"
#include "zone_util.h"
#include "ig_enum.h"
#include "ingame_database_manager.h"
#include "sheet_manager.h"
#include "pacs_client.h"
#include "client_cfg.h"

using namespace NLMISC;
using namespace std;

//===================================================================================
CVillage::CVillage() : _Scene(NULL)
{
	// Not an output
	_IsOutpost = false;

	// Init buildings
	for (uint i=0; i<RZ_MAX_BUILDING_PER_OUTPOST; i++)
	{
		_Buildings[i].Id = i;
	}
}

//===================================================================================
CVillage::~CVillage()
{
}

//===================================================================================
/*virtual*/ bool CVillage::needCompleteLoading(const NLMISC::CVector &pos) const
{
	nlassert(_Scene);	// should call built
	return _IG.needCompleteLoading(pos);
}

//===================================================================================
/*virtual*/ void CVillage::update(const NLMISC::CVector &pos)
{
	nlassert(_Scene);	// should call built
	_IG.update(pos);
}

//===================================================================================
/*virtual*/ void CVillage::forceUpdate(const NLMISC::CVector &pos, NLMISC::IProgressCallback &progress)
{
	nlassert(_Scene);	// should call built
	_IG.forceUpdate(pos, progress);
}

//===================================================================================
bool CVillage::setupFromSheet(NL3D::UScene *scene, const CVillageSheet &sheet, CStreamableIG::TString2IG *loadedIGMap /*=NULL*/)
{
	nlassert(!_Scene); // should be built only once
	nlassert(scene);




	NLMISC::CVector pos;
	pos.z = sheet.Altitude;
	std::string zoneName;

	NLMISC::CVector2f zonePos;
	if (!getPosFromZoneName(sheet.Zone, zonePos))
	{
		nlwarning("Village : invalid zone name (%s)", sheet.Zone.c_str());
		return false;
	}
	pos.x = zonePos.x + sheet.CenterX;
	pos.y = zonePos.y + sheet.CenterY;

	_IG.init(scene, pos, sheet.ForceLoadDist, sheet.LoadDist, sheet.UnloadDist);
	_IG.setLoadedIGMap(loadedIGMap);


	_IG.reserve((uint)sheet.IGs.size());

	_IG.Name = sheet.Name;

	// For each children
	for(uint k = 0; k < sheet.IGs.size(); ++k)
	{
		_IG.addIG(sheet.IGs[k].IgName, sheet.IGs[k].ParentName, CVector::Null, CQuat::Identity);
	}
	_Scene = scene;
	_IsOutpost= false;
	return true;
}

//===================================================================================
void CVillage::forceUnload()
{
	_IG.forceUnload();
}

//===================================================================================
void CVillage::registerObserver(IIGObserver *obs)
{
	_IG.registerObserver(obs);
}


//===================================================================================
void CVillage::removeObserver(IIGObserver *obs)
{
	_IG.removeObserver(obs);
}


//===================================================================================
bool CVillage::isObserver(IIGObserver *obs) const
{
	return _IG.isObserver(obs);
}

//===================================================================================
bool CVillage::enumIGs(IIGEnum *callback)
{
	return _IG.enumIGs(callback);
}


// ***************************************************************************
// ***************************************************************************
// OUTPOST
// ***************************************************************************
// ***************************************************************************


//===================================================================================
bool CVillage::setupOutpost(NL3D::UScene *scene, const CContinentParameters::CZC &zone, sint32 /* outpost */, CStreamableIG::TString2IG *loadedIGMap)
{
	nlassert(!_Scene); // should be built only once
	nlassert(scene);

	// Get the zone position (for async loading)
	NLMISC::CVector pos;
	NLMISC::CVector2f zonePos;
	if (!getPosFromZoneName(zone.Name, zonePos))
	{
		nlwarning("Outpost : invalid zone name (%s)", zone.Name.c_str());
		return false;
	}
	pos.x = zonePos.x;
	pos.y = zonePos.y;
	pos.z = 0;

	// Setup the IG
	_IG.init(scene, pos, zone.ForceLoadDist, zone.LoadDist, zone.UnloadDist);
	_IG.setLoadedIGMap(loadedIGMap);
	_IG.reserve(RZ_MAX_BUILDING_PER_OUTPOST);

	_Scene = scene;
	_IsOutpost = true;
	return true;
}

//===================================================================================
void CVillage::setBuildingPosition (uint building, const NLMISC::CQuat &rot, const NLMISC::CVector &pos)
{
	// Set the building position and rotation
	if (building<RZ_MAX_BUILDING_PER_OUTPOST)
	{
		_Buildings[building].Rotation = rot;
		_Buildings[building].Position = pos;
	}
}

//===================================================================================
void CVillage::initOutpost ()
{
	if (_IsOutpost)
	{
		// Init the ZC with ruins
		CSheetId ruins ("ruins.building");
		for (uint i=0; i<RZ_MAX_BUILDING_PER_OUTPOST; i++)
		{
			updateBuilding(i, ruins);
		}
	}
}

//===================================================================================
void CVillage::removeOutpost ()
{
	if (_IsOutpost)
	{
		// Force form to be reseted so next time outpost is reloaded
		for (uint i=0; i<RZ_MAX_BUILDING_PER_OUTPOST; i++)
		{
			_Buildings[i].CurrentSheetId = CSheetId::Unknown;
		}
	}
}

//===================================================================================
void CVillage::updateBuilding(uint buildingId, NLMISC::CSheetId newForm)
{
	if(buildingId>=RZ_MAX_BUILDING_PER_OUTPOST)
		return;
	CBuilding	&building= _Buildings[buildingId];

	// Form has changed ?
	if (building.CurrentSheetId != newForm)
	{
		// Resize the streamable ig, if needed
		for (uint i=_IG.getIGNum(); i<RZ_MAX_BUILDING_PER_OUTPOST; i++)
		{
			// todo hulud add rotation
			_IG.addIG("", "", _Buildings[i].Position, _Buildings[i].Rotation);
		}

		// Ig name for this building
		string igName;

		// If the Sheet id is unknown, just remove the old building if there was one.
		if(newForm != CSheetId::Unknown)
		{
			// Get a pointer on the converted sheet into the client.
			CEntitySheet *entitySheet = SheetMngr.get(newForm);
			const CBuildingSheet *bldSh = dynamic_cast<const CBuildingSheet *>(entitySheet);
			if (bldSh)
			{
				// todo hulud here, manage multi igs for multi building state
				igName = bldSh->BuildedIg;
			}
		}

		// Set the building IG
		_IG.setIG(building.Id, igName, "");
		building.CurrentSheetId = newForm;
	}
}

