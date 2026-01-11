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

#include "track.h"
#include "reynolds_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;


// ------------------------------------
// Attraction/Repulsion functions

// strong repulsion limited
inline double	softPointing(double dist, double strength, double amp = 1.0)
{
	return amp*atan(strength*dist)/1.5707963268;
}

// strong repulsion limited
inline double	softTargetting(double dist, double moderateDist, double strength, double amp = 1.0)
{
	return amp*( atan(strength*(dist-moderateDist-0.2)) + atan(strength*(dist-moderateDist+0.2)) )/3.1415927;
}

// strong repulsion limited
inline double	softRepulse(double dist, double moderateDist, double strength, double amp = 1.0)
{
	return amp*(0.5 - atan(strength*(dist-moderateDist)-1.0)/3.1415927);
}



// Required target spacing
double			CTrack::TargetSpacing = 0.5;

// Target attraction strength
double			CTrack::TargetAttraction = 3.0;

// Target attraction amplification
double			CTrack::TargetAmp = 2.0;

// Fast obstacle exclusion distance
double			CTrack::ObstacleExcludeDistance = 6.0;

// Required obstacle spacing
double			CTrack::ObstacleSpacing = 0.5;

// Obstacle repulsion strength
double			CTrack::ObstacleRepulsion = 3.0;

// Obstacle repulsion amplification
double			CTrack::ObstacleAmp = 2.0;


// Minimum motion distance
double			CTrack::MinimumMotion = 0.2;

// Lock distance threshold
double			CTrack::LockThreshold = 0.1;

// Lose distance threshold
double			CTrack::LoseThreshold = 2.0;

// Stabilise cycle
uint32			CTrack::StabiliseCycle = 5;


// The default sheet
CTrack::CSheet	CTrack::CSheet::DefaultSheet;





// ------------------------------------
// Destructor
CTrack::~CTrack()
{
	nldebug("ReynoldsLib:CTrack:~CTrack(): Delete Track %s", _Id.toString().c_str());

	// remove this track from the manager map
	CReynoldsManager::removeTrackFromMap(_Id);

	// delete move primitive
	deleteMovePrimitive();

	nldebug("ReynoldsLib:CTrack:~CTrack(): Track %s deleted", _Id.toString().c_str());
}

// ------------------------------------
// Init track
void	CTrack::setId(const NLMISC::CEntityId &id, const NLMISC::CSheetId &sheet)
{
	_Id = id;
	_SheetId = sheet;
	_HasId = true;
	_IdRequested = false;

	_Sheet = CReynoldsManager::lookup(_SheetId);
	if (_Sheet == NULL)
		_Sheet = &(CSheet::DefaultSheet);
}


// ------------------------------------
// Follow
void	CTrack::follow(CTrack *followed)
{
	acquireControl();
	acquireVision();

	_Followed = followed;
}

// ------------------------------------
// Leave
void	CTrack::leave()
{
	releaseControl();
	releaseVision();

	_Followed = NULL;

	// warn reynolds manager the track left its target
	CReynoldsManager::trackStop(this);
}




// ------------------------------------
// Update
void	CTrack::update(double dt)
{
	// do not update not owned tracks
	if (!_OwnControl)
		return;

	// check if target is forced to leave
	CTrack	*target = (CTrack*)_Followed;
	if (target == NULL || target->_ForceRelease)
	{
		leave();
		return;
	}

	// check if entity has id
	if (!hasId() || !hasPosition())
		return;

	// check if has a move primitive
	if (_MovePrimitive == NULL)
	{
		createMovePrimitive();
		// if failed, just leave
		if (_MovePrimitive == NULL)
			leave();
		return;
	}

	// if target hasn't position and is not owned by manager, request for position updates
	if (!target->isValid())
		return;

	// motion
	CVectorD	motion(CVectorD::Null);
	CVectorD	heading;


	// --------------------------
	// move toward target
	double	targetObjective;
	if (target->isStatic())
	{
		CVectorD	vdist;
		double		ddist;

		ddist = rawDistance(target, vdist);
		heading = vdist;

		targetObjective = ddist;

		double		strength = softPointing(ddist, TargetAttraction, TargetAmp);

		motion += (vdist * (_Sheet->RunSpeed*dt*strength/(ddist+0.01)));
	}
	else
	{
		CVectorD	vdist;
		double		ddist;

		double		cdist = contactDistance(target, vdist, ddist);

		targetObjective = cdist-TargetSpacing;

		double		strength = softTargetting(cdist, TargetSpacing, TargetAttraction, TargetAmp);

		motion += (vdist * (_Sheet->RunSpeed*dt*strength/(ddist+0.01)));
	}


	// --------------------------
	// check target not lost
	if (_State == TargetLocked &&
		targetObjective > LoseThreshold)
	{
		_State = TargetLost;
		CReynoldsManager::stateChanged(_Id, _State);
		leave();
		return;
	}


	// --------------------------
	// check target reachable
	const double	SmoothFactor = 0.7;
	double			tddelta = (_LastTargetDistance<0) ? _SmoothedTargetDistanceDelta : _LastTargetDistance-targetObjective;
	_SmoothedTargetDistanceDelta = _SmoothedTargetDistanceDelta*SmoothFactor + tddelta*(1.0-SmoothFactor);
	_LastTargetDistance = targetObjective;

	// if actor seems not to move fast enough, leave
	if (_State == MovingTowardsTarget &&
		_SmoothedTargetDistanceDelta < 0.1 &&
		targetObjective > 1.0)
	{
		_State = TargetUnreachable;
		CReynoldsManager::stateChanged(_Id, _State);
		leave();
		return;
	}



	// --------------------------
	// avoid obstacles in vision
	TVision::iterator	itv;
	for (itv=_Vision.begin(); itv!=_Vision.end(); )
	{
		CTrack	*obstacle = (CTrack*)((*itv).second);

		// if obstacle is forced to be release, delete it
		if (obstacle->_ForceRelease)
		{
			TVision::iterator	itr = itv++;
			_Vision.erase(itr);
			continue;
		}

		// if obstacle not yet ready, don't avoid it
		if (!obstacle->isValid())
			continue;

		// don't avoid static obstacles (virtual tracks)
		if (!obstacle->isStatic())
		{
			CVectorD	vdist;
			double		ddist;

			ddist = rawDistance(obstacle, vdist);

			if (ddist > ObstacleExcludeDistance)
				continue;

			double		cdist = contactDistanceWithRawDistance(obstacle, vdist, ddist);
			double		strength = softRepulse(cdist, ObstacleSpacing, ObstacleRepulsion, ObstacleAmp);

			motion -= (vdist * (_Sheet->WalkSpeed*dt*strength/(ddist+0.01)));
		}

		++itv;
	}

	// --------------------------
	// avoid walls
	CVectorD	front = motion.normed();
	CVectorD	lateral(-front.y, front.x, 0.0);
	CVectorD	normal;
	float		thetaProbe = frand(3.1415926535f) - 1.570796f;

	if (!_MoveContainer->testMove(_MovePrimitive, front*cos(thetaProbe)*2.0 + lateral*sin(thetaProbe)*1.0, 1, 0, &normal))
		motion += normal*_Sheet->WalkSpeed*dt;





	// --------------------------
	// other motion applied by user
	CReynoldsManager::applyUserMotion(this, motion);





	// --------------------------
	// Do move
	double	motionNorm = motion.norm();

	// check if enough motion
	if (motionNorm < MinimumMotion)
	{
		if (targetObjective < LockThreshold &&
			_State == MovingTowardsTarget &&
			CReynoldsManager::getCycle()-_LastMoveCycle > StabiliseCycle)
		{
			_State = TargetLocked;
			CReynoldsManager::stateChanged(_Id, _State);
		}
		return;
	}

	// reset last moving cycle
	_LastMoveCycle = CReynoldsManager::getCycle();

	// renorm motion if needed
	if (motionNorm > dt*_Sheet->RunSpeed)
		motion *= dt*_Sheet->RunSpeed/motionNorm;

	// eval move
	_MovePrimitive->move(motion, 0);
	_MoveContainer->evalNCPrimitiveCollision(1, _MovePrimitive, 0);

	// store new position/heading
	_Heading = (float)atan2(heading.y, heading.x);
	_Position = _MovePrimitive->getFinalPosition(0);
}



// ------------------------------------
// Update vision
void	CTrack::updateVision(const std::vector<NLMISC::CEntityId> &in, const std::vector<NLMISC::CEntityId> &out)
{
	uint	i;

	// add new in vision
	for (i=0; i<in.size(); ++i)
	{
		CTrack	*newin = CReynoldsManager::createTrack(in[i]);
		if (newin == NULL)
			continue;

		_Vision.insert(make_pair<CEntityId, CSmartPtr<CTrack> >(in[i], newin));
	}

	// remove old of vision
	for (i=0; i<out.size(); ++i)
	{
		_Vision.erase(out[i]);
	}
}

// ------------------------------------
// Update vision
void	CTrack::updateVision(const std::vector<NLMISC::CEntityId> &vision)
{
	uint	i;

	// add new in vision
	TVision	copy = _Vision;
	_Vision.clear();
	for (i=0; i<vision.size(); ++i)
	{
		CTrack	*newin = CReynoldsManager::createTrack(vision[i]);
		if (newin == NULL)
			continue;

		_Vision.insert(make_pair<CEntityId, CSmartPtr<CTrack> >(vision[i], newin));
	}
}






// ------------------------------------
// Acquire Control
void	CTrack::acquireControl()
{
	// set state to moving
	_State = MovingTowardsTarget;
	CReynoldsManager::stateChanged(_Id, _State);

	if (_OwnControl)
		return;

	// unrequest for position updates if necessary
	if (_PositionUpdatesRequested)
	{
		CReynoldsManager::unrequestPositionUpdates(_Id);
		_PositionUpdatesRequested = false;
	}

	// invalidate position
	invalidPosition();

	// and request for valid position
	CReynoldsManager::requestPosition(_Id);

	// init user motion
	CReynoldsManager::initUserMotion(this);

	_OwnControl = true;

	// init last move cycle
	_LastMoveCycle = CReynoldsManager::getCycle();
}

// ------------------------------------
// Release Control
void	CTrack::releaseControl()
{
	// invalidate position
	invalidPosition();

	// remove primitive
	deleteMovePrimitive();

	// release user motion
	CReynoldsManager::releaseUserMotion(this);

	// set state to moving
	_State = Idle;
	CReynoldsManager::stateChanged(_Id, _State);

	_OwnControl = false;
}


// ------------------------------------
// Acquire vision
void	CTrack::acquireVision()
{
	if (_ReceiveVision)
		return;

	_ReceiveVision = true;
	CReynoldsManager::requestVision(_Id);
}

// ------------------------------------
// Release vision
void	CTrack::releaseVision()
{
	if (!_ReceiveVision)
		return;

	_ReceiveVision = false;
	CReynoldsManager::unrequestVision(_Id);
}




// ------------------------------------
// Create Move primitive
void	CTrack::createMovePrimitive()
{
	if (!hasPosition())
	{
		nlwarning("ReynoldsLib:CTrack:createMovePrimitive(): Can't create move primitive, Track %s has no valid position", _Id.toString().c_str());
		return;
	}

	if (!hasId())
	{
		nlwarning("ReynoldsLib:CTrack:createMovePrimitive(): Can't create move primitive, Track %s has no valid id", _Id.toString().c_str());
		return;
	}

	deleteMovePrimitive();

	CReynoldsManager::createMovePrimitive(_Position, _MovePrimitive, _MoveContainer);

	_MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
	_MovePrimitive->setReactionType(UMovePrimitive::Slide);
	_MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
	_MovePrimitive->setCollisionMask(0);
	_MovePrimitive->setOcclusionMask(0);
	_MovePrimitive->setObstacle(false);
	_MovePrimitive->setDontSnapToGround(false);
	_MovePrimitive->setRadius(_Sheet->Radius);
	_MovePrimitive->setHeight(_Sheet->Height);

	_MovePrimitive->insertInWorldImage(0);
	_MovePrimitive->setGlobalPosition(_Position, 0);
	_MovePrimitive->setOrientation(_Heading, 0);
	_MoveContainer->evalCollision(1, 0);
}

// ------------------------------------
// Delete Move primitive
void	CTrack::deleteMovePrimitive()
{
	if (_MovePrimitive != NULL)
	{
		if (_MoveContainer == NULL)
		{
			nlwarning("ReynoldsLib:CTrack:deleteMovePrimitive(): Track %s has a MovePrimitive, but MoveContainer not set", _Id.toString().c_str());
		}
		else
		{
			_MoveContainer->removePrimitive(_MovePrimitive);
		}
	}

	_MovePrimitive = NULL;
	_MoveContainer = NULL;
}

// ------------------------------------
// Request Id
void	CTrack::requestId()
{
	_IdRequested = true;
	CReynoldsManager::requestSheet(_Id);
}

// ------------------------------------
// Request Position
void	CTrack::requestPositionUpdates()
{
	_PositionUpdatesRequested = true;
	CReynoldsManager::requestPositionUpdates(_Id);
}
