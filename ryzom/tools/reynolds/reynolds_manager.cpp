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

#include "reynolds_manager.h"

#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;
using namespace NLGEORGES;


/*
 * Manager variables
 */

// Continents
CContinentContainer							CReynoldsManager::_Continents;

// Track Map, all tracks referenced, held by a simple pointer so when no one references a track, it is deleted
CReynoldsManager::TTrackMap					CReynoldsManager::_TrackMap;

// Controlled Track Map, only controlled tracks, held by a smart pointer
CReynoldsManager::TControlledTrackMap		CReynoldsManager::_ControlledTrackMap;

// Command interface
CReynoldsManager::ICommandInterface			*CReynoldsManager::_CommandInterface = NULL;

// User init callback
CReynoldsManager::TUserCallback				CReynoldsManager::_UserInitCallback = NULL;

// User motion callback
CReynoldsManager::TUserMotionCallback		CReynoldsManager::_UserMotionCallback = NULL;

// User release callback
CReynoldsManager::TUserCallback				CReynoldsManager::_UserReleaseCallback = NULL;

// Georges sheets
map<CSheetId, CTrack::CSheet>				CReynoldsManager::_Sheets;

// Sheets initialised ?
bool										CReynoldsManager::_Initialised;

// Current internal cycle
uint32										CReynoldsManager::_Cycle;

/*
 * Manager methods
 */


// ------------------------------------
// Constructor
CReynoldsManager::CReynoldsManager()
{
	nlerror("CReynoldsManager: static library, must not be instanciated !");
}



// ------------------------------------
// Init
void	CReynoldsManager::init(const std::string &packSheetFile)
{
	_Continents.init(100, 100, 8.0, 1);

	initSheets(packSheetFile);

	_Cycle = 0;
}

// ------------------------------------
// Update
void	CReynoldsManager::update(double dt)
{
	TControlledTrackMap::iterator	it;
	for (it=_ControlledTrackMap.begin(); it!=_ControlledTrackMap.end(); ++it)
	{
		CTrack	*track = (CTrack*)((*it).second);
		track->update(dt);
	}

	++_Cycle;
}

// ------------------------------------
// Release
void	CReynoldsManager::release()
{
	_Continents.clear();

	_ControlledTrackMap.clear();

	if (!_TrackMap.empty())
	{
		nlwarning("ReynoldsLib:CReynoldsManager:release(): TrackMap not empty at release !!");

		TTrackMap::iterator	it;
		for (it=_TrackMap.begin(); it!=_TrackMap.end(); ++it)
			delete (*it).second;

		_TrackMap.clear();
	}
}



// ------------------------------------
// Load continent
void	CReynoldsManager::loadContinent(const std::string &name, const std::string &file, sint index)
{
	_Continents.loadContinent(name, file, index);
}





// ------------------------------------
// Create Track
CTrack	*CReynoldsManager::createTrack(const CEntityId &entity)
{
	// look for the target, and create a new track if not found
	TTrackMap::iterator	itt = _TrackMap.find(entity);
	if (itt == _TrackMap.end())
	{
		CTrack	*track = new CTrack();
		pair<TTrackMap::iterator, bool>	res = _TrackMap.insert(TTrackMap::value_type(entity, track));
		itt = res.first;

		requestSheet(entity);
	}
	return (*itt).second;
}

// ------------------------------------
// Remove Track from map
void	CReynoldsManager::removeTrackFromMap(const CEntityId &entity)
{
	_TrackMap.erase(entity);
}








// ------------------------------------
// Follow
void	CReynoldsManager::follow(const NLMISC::CEntityId &entity, const NLMISC::CEntityId &target)
{
	// look for the track, and create a new one if not found -- in both maps
	CTrack	*etrack = createTrack(entity);

	TControlledTrackMap::iterator	ite = _ControlledTrackMap.find(entity);
	if (ite == _ControlledTrackMap.end())
		_ControlledTrackMap.insert(TControlledTrackMap::value_type(entity, etrack));

	// look for the target, and create a new track if not found
	CTrack	*ttrack = createTrack(target);

	nldebug("ReynoldsLib:CReynoldsManager:follow(): %s now follows %s", entity.toString().c_str(), target.toString().c_str());

	// let entity follow the target
	etrack->follow(ttrack);
}

// ------------------------------------
// Go to
void	CReynoldsManager::goTo(const NLMISC::CEntityId &entity, const NLMISC::CVectorD &position)
{
	// look for the track, and create a new one if not found -- in both maps
	CTrack	*etrack = createTrack(entity);

	TControlledTrackMap::iterator	ite = _ControlledTrackMap.find(entity);
	if (ite == _ControlledTrackMap.end())
		_ControlledTrackMap.insert(TControlledTrackMap::value_type(entity, etrack));

	// create a fake track for point position
	static uint	i = 0;
	CEntityId	id(0, i++, 0, 0);
	CTrack	*pointTo = createTrack(id);
	pointTo->setStatic();
	pointTo->setId(id, CSheetId::Unknown);
	pointTo->setPosition(position, 0.0f);

	nldebug("ReynoldsLib:CReynoldsManager:goTo(): %s now goes to (%.1f,%.1f)", entity.toString().c_str(), position.x, position.y);

	// let entity follow the target
	etrack->follow(pointTo);
}

// ------------------------------------
// Stop
void	CReynoldsManager::leaveMove(const NLMISC::CEntityId &entity)
{
	TControlledTrackMap::iterator	ite = _ControlledTrackMap.find(entity);
	if (ite == _ControlledTrackMap.end())
	{
		nlwarning("ReynoldsLib:CReynoldsManager:leaveMove(): undefined entity %s", entity.toString().c_str());
		return;
	}

	nldebug("ReynoldsLib:CReynoldsManager:leaveMove(): %s control left", entity.toString().c_str());

	(*ite).second->leave();
	_ControlledTrackMap.erase(ite);
}

// ------------------------------------
// Destroy
void	CReynoldsManager::destroy(const NLMISC::CEntityId &entity)
{
	CTrack	*track = getTrack(entity);
	if (track != NULL)
		track->forceRelease();

	if (track != NULL && track->hasControlOwned())
		track->leave();

	_ControlledTrackMap.erase(entity);
}






// ------------------------------------
// Request Sheet
void	CReynoldsManager::requestSheet(const NLMISC::CEntityId &entity)
{
	if (!checkInterface())
		return;

	_CommandInterface->requestSheet(entity);
}

// ------------------------------------
// Request Position
void	CReynoldsManager::requestPosition(const NLMISC::CEntityId &entity)
{
	if (!checkInterface())
		return;

	_CommandInterface->requestPosition(entity);
}

// ------------------------------------
// Request Position Updates
void	CReynoldsManager::requestPositionUpdates(const NLMISC::CEntityId &entity)
{
	if (!checkInterface())
		return;

	_CommandInterface->requestPositionUpdates(entity);
}

// ------------------------------------
// Unrequest Position Updates
void	CReynoldsManager::unrequestPositionUpdates(const NLMISC::CEntityId &entity)
{
	if (!checkInterface())
		return;

	_CommandInterface->unrequestPositionUpdates(entity);
}

// ------------------------------------
// Request Vision
void	CReynoldsManager::requestVision(const NLMISC::CEntityId &entity)
{
	if (!checkInterface())
		return;

	_CommandInterface->requestVision(entity);
}

// ------------------------------------
// Unrequest Vision
void	CReynoldsManager::unrequestVision(const NLMISC::CEntityId &entity)
{
	if (!checkInterface())
		return;

	_CommandInterface->unrequestVision(entity);
}


// ------------------------------------
// Updated position
void	CReynoldsManager::updatedPosition(const CEntityId &entity, const CVectorD &position, float heading)
{
	if (_CommandInterface != NULL)
		_CommandInterface->updatePosition(entity, position, heading);
}

// ------------------------------------
// Updated state
void	CReynoldsManager::stateChanged(const CEntityId &entity, CTrackBase::TTrackState state)
{
	if (_CommandInterface != NULL)
		_CommandInterface->stateChanged(entity, state);
}


// ------------------------------------
// TrackStop
void	CReynoldsManager::trackStop(CTrack *track)
{
	if (checkInterface())
		_CommandInterface->stopTrack(track->getId());

	// remove track from controlled tracks
	_ControlledTrackMap.erase(track->getId());
}


// ------------------------------------
// Create Move primitive
void	CReynoldsManager::createMovePrimitive(const NLMISC::CVectorD &pos, NLPACS::UMovePrimitive *&primitive, NLPACS::UMoveContainer *&container)
{
	primitive = NULL;
	container = NULL;

	uint8	continent = _Continents.findContinent(pos);
	if (continent == -1)
	{
		nlwarning("ReynoldsLib:CReynoldsManager:createMovePrimitive(): unable to create move primitive");
		return;
	}

	container = _Continents.getMoveContainer(continent);
	primitive = container->addNonCollisionablePrimitive();
}





// ------------------------------------
// Set Sheet
void	CReynoldsManager::setSheet(const NLMISC::CEntityId &id, const NLMISC::CSheetId &sheet)
{
	CTrack	*track = getTrack(id);
	if (track == NULL)
	{
		nlwarning("ReynoldsLib:CReynoldsManager:setSheet(): Track %s not found", id.toString().c_str());
		return;
	}

	if (track->hasId())
	{
		nlwarning("ReynoldsLib:CReynoldsManager:setSheet(): Track %s already has an Id", id.toString().c_str());
		return;
	}

	track->setId(id, sheet);
}

// ------------------------------------
// Set Position
void	CReynoldsManager::setPosition(const NLMISC::CEntityId &id, const NLMISC::CVectorD &position, float heading)
{
	CTrack	*track = getTrack(id);
	if (track == NULL)
	{
		nlwarning("ReynoldsLib:CReynoldsManager:setPosition(): Track %s not found", id.toString().c_str());
		return;
	}

	track->setPosition(position, heading);
}

// ------------------------------------
// Set Vision
void	CReynoldsManager::setVision(const NLMISC::CEntityId &id, const std::vector<NLMISC::CEntityId> &in, const std::vector<NLMISC::CEntityId> &out)
{
	CTrack	*track = getTrack(id);
	if (track == NULL)
	{
		nlwarning("ReynoldsLib:CReynoldsManager:setVision(): Track %s not found", id.toString().c_str());
		return;
	}

	track->updateVision(in, out);
}

// ------------------------------------
// Set Vision
void	CReynoldsManager::setVision(const NLMISC::CEntityId &id, const std::vector<NLMISC::CEntityId> &vision)
{
	CTrack	*track = getTrack(id);
	if (track == NULL)
	{
		nlwarning("ReynoldsLib:CReynoldsManager:setVision(): Track %s not found", id.toString().c_str());
		return;
	}

	track->updateVision(vision);
}




//-------------------------------------------------------------------------
// Init Sheets
void	CReynoldsManager::initSheets(const std::string &packSheetFile)
{
	if (_Initialised)
		return;

	std::vector<std::string> filters;
	filters.push_back("creature");
	filters.push_back("player");

	loadForm(filters, packSheetFile, _Sheets);

	_Initialised=true;
}


//-------------------------------------------------------------------------
// Lookup Sheet
const CTrack::CSheet *CReynoldsManager::lookup(const CSheetId &id)
{
	nlassert(_Initialised);

	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId, CTrack::CSheet>::iterator it;
	it=_Sheets.find(id);

	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it != _Sheets.end())
		return &((*it).second);
	else
		return NULL;
}


