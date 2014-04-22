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

#include "outpost.h"
#include "zone_util.h"
#include "pacs_client.h"
#include "client_cfg.h"
#include "village.h"

using namespace NLMISC;
using namespace NLPACS;


// ***************************************************************************
COutpost::COutpost()
{
	_OutpostId= -1;
}

// ***************************************************************************
COutpost::~COutpost()
{
	removeOutpost();
}

// ***************************************************************************
COutpost::COutpost(const COutpost &other)
{
	for(uint i=0;i<RZ_MAX_BUILDING_PER_OUTPOST;i++)
		_Buildings[i]= other._Buildings[i];
	// must not have addedprims, else dtor crash
	nlassert(other._AddedPrims.empty());
	_OutpostId= other._OutpostId;
	_Village= other._Village;
}

// ***************************************************************************
bool COutpost::setupOutpost(const CContinentParameters::CZC &zone, sint32 outpostId, CVillage *village)
{
	// Yoyo. legacy code. should no more be needed now. Still let for check (useful?)
	NLMISC::CVector2f zonePos;
	if (!getPosFromZoneName(zone.Name, zonePos))
	{
		nlwarning("Outpost : invalid zone name (%s)", zone.Name.c_str());
		return false;
	}

	// set the outpost id
	_OutpostId = outpostId;

	// set the village for ruins display (if any)
	_Village= village;

	return true;
}


// ***************************************************************************
void COutpost::setBuildingPosition (uint building, const NLMISC::CQuat &rot, const NLMISC::CVector &pos)
{
	// Set the building position and rotation
	if (building<RZ_MAX_BUILDING_PER_OUTPOST)
	{
		_Buildings[building].Rotation = rot;
		_Buildings[building].Position = pos;
	}

	// if a village is setuped for ruins display, setup it
	if(_Village)
		_Village->setBuildingPosition(building, rot, pos);
}

// ***************************************************************************
void COutpost::initOutpost ()
{
	// remove the outpost from col, if any
	removeOutpost();

	// Add collisions, if correclty setuped
	if (_OutpostId > -1)
	{
		// Register RZ_MAX_BUILDING_PER_OUTPOST observers for the RZ_MAX_BUILDING_PER_OUTPOST buildings
		uint i;
		for (i=0; i<RZ_MAX_BUILDING_PER_OUTPOST; i++)
		{
			// Put the ZC pacs_prim
			TPacsPrimMap::iterator pbIt = PacsPrims.find(NLMISC::strlwr(NLMISC::CFile::getFilenameWithoutExtension(ClientCfg.ZCPacsPrim)));
			if (pbIt != PacsPrims.end())
			{
				// Build the building matrix
				NLMISC::CMatrix instanceMatrix;
				instanceMatrix.identity();
				instanceMatrix.setRot(_Buildings[i].Rotation);
				instanceMatrix.setPos(_Buildings[i].Position);

				// Compute orientation and position
				NLMISC::CVector pos;
				float			angle;
				NLPACS::UMoveContainer::getPACSCoordsFromMatrix(pos, angle, instanceMatrix);

				// insert the matching primitive block
				if (PACS)
					PACS->addCollisionnablePrimitiveBlock(pbIt->second, 0, 1, &_AddedPrims, angle, pos, true, CVector(1,1,1));
			}
		}
	}

	// add 3D if needed
	if(_Village)
		_Village->initOutpost();
}

// ***************************************************************************
void COutpost::removeOutpost ()
{
	// remove collisions
	if(PACS)
	{
		for(uint i=0; i<_AddedPrims.size(); i++)
		{
			PACS->removePrimitive(_AddedPrims[i]);
		}
		_AddedPrims.clear();
	}

	// remove 3D
	if(_Village)
		_Village->removeOutpost();
}

