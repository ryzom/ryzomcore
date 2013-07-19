// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include "nel/3d/zone_manager.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ------------------------------------------------------------------------------------------------
// CZoneManager
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
CZoneManager::CZoneManager()
{
	_RemovingZone= false;
	_ZoneTileColorMono = true;
	_ZoneTileColorFactor = 1.0f;
	_LastArea= 0;
	_LastX= _LastY= std::numeric_limits<uint32>::max();
}

// ------------------------------------------------------------------------------------------------
CZoneManager::~CZoneManager()
{
}

// ------------------------------------------------------------------------------------------------
uint CZoneManager::getNumZoneLeftToLoad ()
{
	// Make a set of the loaded zone
	set<uint16> zoneLoaded;
	uint32 i;
	for (i = 0; i < _LoadedZones.size(); ++i)
	{
		zoneLoaded.insert (_LoadedZones[i]);
	}

	// Check for each zone in the list if they are loaded or not
	uint zoneCount = 0;
	for (i = 0; i < _ZoneList.size(); ++i)
	{
		if (zoneLoaded.find (_ZoneList[i]) == zoneLoaded.end ())
			zoneCount++;
	}
	return zoneCount;
}

// ------------------------------------------------------------------------------------------------
void CZoneManager::checkZonesAround (uint x, uint y, uint area, const std::vector<uint16> *validZoneIds)
{
	if (_RemovingZone) return;

	// Obtain the new set of zones around
	if ( (x != _LastX) || (y != _LastY) || (area != _LastArea) )
		getListZoneId (x, y, area, _ZoneList, validZoneIds);
	_LastX = x;
	_LastY = y;
	_LastArea = area;

	// **** Look if we have zone loaded that is not needed anymore
	uint32 i, j;
	for (i = 0; i < _LoadedZones.size(); ++i)
	{
		// If the loadedzone i do not appear in the zone list so we have to remove it
		bool bFound = false;
		uint16 nLoadedZone = _LoadedZones[i];
		for (j = 0; j < _ZoneList.size(); ++j)
		{
			if (_ZoneList[j] == nLoadedZone)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			// Remove the zone nLoadedZone
			_IdZoneToRemove = nLoadedZone;
			_RemovingZone = true;
			return;
		}
	}

	// **** Look if we have zone not already loaded
	for (i = 0; i < _ZoneList.size(); ++i)
	{
		// If the zone requested appear in the zone loaded, we don't have to load it
		bool bFound = false;
		uint16 nZone = _ZoneList[i];
		for (j = 0; j < _LoadedZones.size(); ++j)
		{
			if (_LoadedZones[j] == nZone)
			{
				bFound = true;
				break;
			}
		}

		// if the zone is not already loaded
		if (!bFound)
		{
			// Already loading ?
			std::list<CLoadingZone>::iterator ite = _LoadingZones.begin ();
			while (ite != _LoadingZones.end())
			{
				if (ite->ZoneToAddId == nZone)
					break;

				// Next loading zone
				ite++;
			}

			// Not loading ?
			if (ite == _LoadingZones.end())
			{
				// Add a new zone to load
				_LoadingZones.push_back(CLoadingZone ());
				CLoadingZone &newZone = _LoadingZones.back();
				newZone.ZoneToAddName = getZoneNameFromId(nZone);
				newZone.ZoneToAddId = nZone;
				newZone.Zone = NULL;

				// We have to load this zone. add a load task
				CAsyncFileManager &rAFM = CAsyncFileManager::getInstance();

				// Make a position
				uint x, y;
				getZonePos (newZone.ZoneToAddId, x, y);
				CVector v = CVector ((float)x, -(float)y, 0);
				rAFM.addTask (new CZoneLoadingTask(newZone.ZoneToAddName, &newZone.Zone, v, _ZoneTileColorMono, _ZoneTileColorFactor));
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
bool CZoneManager::isWorkComplete (CZoneManager::SZoneManagerWork &rWork)
{
	// Check if there is someting to add
	std::list<CLoadingZone>::iterator ite = _LoadingZones.begin ();
	while (ite != _LoadingZones.end())
	{
		// Loaded ?
		if (ite->Zone)
		{
			rWork.ZoneAdded = true;
			rWork.NameZoneAdded = ite->ZoneToAddName;
			rWork.ZoneRemoved = false;
			rWork.IdZoneToRemove = 0;
			rWork.NameZoneRemoved = "";
			rWork.Zone = const_cast<CZone*>(ite->Zone);
			_LoadedZones.push_back (ite->ZoneToAddId);

			// Remove from loading zone
			_LoadingZones.erase(ite);
			return true;
		}

		// Next zone
		ite++;
	}

	if (_RemovingZone)
	{
		_RemovingZone = false;
		rWork.ZoneAdded = false;
		rWork.NameZoneAdded = "";
		rWork.ZoneRemoved = true;
		rWork.IdZoneToRemove = _IdZoneToRemove;
		rWork.NameZoneRemoved = getZoneNameFromId(_IdZoneToRemove);
		uint32 i, j;
		for (i = 0 ; i < _LoadedZones.size(); ++i)
			if (_LoadedZones[i] == _IdZoneToRemove)
				break;
		if (i < _LoadedZones.size())
		{
			for (j = i; j < _LoadedZones.size()-1; ++j)
				_LoadedZones[j] = _LoadedZones[j+1];
			_LoadedZones.resize(_LoadedZones.size()-1);
		}
		rWork.Zone = NULL;
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
void CZoneManager::clear()
{
	nlassert(_LoadingZones.size() == 0);
	_LoadedZones.clear();
	_RemovingZone = false;
}


// ------------------------------------------------------------------------------------------------
// CZoneLoadingTask
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
CZoneLoadingTask::CZoneLoadingTask(const std::string &sZoneName, TVolatileZonePtr *ppZone, CVector &pos, bool monochrome, float factor)
{
	*ppZone = NULL;
	_Zone = ppZone;
	_ZoneName = sZoneName;
	Position = pos;
	_Monochrome = monochrome;
	_TileColorFactor = max(0.0f, factor);
}

// ------------------------------------------------------------------------------------------------
void CZoneLoadingTask::run(void)
{
	// Lookup the zone
	string zonePathLookup = CPath::lookup (_ZoneName, false, false, true);
	if (zonePathLookup.empty())
		zonePathLookup = _ZoneName;

	CZone *ZoneTmp = new CZone;
	CIFile file;
	file.setAsyncLoading(true);
	file.setCacheFileOnOpen(true);
	if(file.open(zonePathLookup))
	{
		ZoneTmp->serial(file);
		file.close();
		ZoneTmp->setTileColor(_Monochrome, _TileColorFactor);
		*_Zone = ZoneTmp;
	}
	else
	{
		//nldebug("CZoneLoadingTask::run(): File not found: %s", zonePathLookup.c_str ());
		delete ZoneTmp;
		*_Zone = (CZone*)-1; // Return error
	}

	delete this;
}


// ***************************************************************************
void CZoneLoadingTask::getName (std::string &result) const
{
	result = "LoadZone(" + _ZoneName + ")";
}

// ***************************************************************************
CZoneLoadingTask::~CZoneLoadingTask()
{
}


} // NL3D

