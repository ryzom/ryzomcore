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
#include "group_profile.h"
#include "ai.h"
#include "family_behavior.h"

using namespace std;
using namespace NLMISC;
using namespace AITYPES;

int CGrpProfileDynFollowPath::_InstanceCounter = 0;

/// Overload for IDynFollowPath interface
void CGrpProfileDynFollowPath::setPath(CNpcZone const* const start, CNpcZone const* const end, CPropertySet const& zoneFilter)
{
	_StartZone = start;
	_EndZone = end;
	_ZoneFilter = zoneFilter;
	_PathCursor = 0;
	_Path.clear();
	_hasChanged=true;
	calcPath();
}

void CGrpProfileDynFollowPath::calcPath()
{
	if	(	_StartZone.isNull()
		||	_EndZone.isNull()
		||	!_hasChanged)
		return;

	_hasChanged=false;
	if	(!::pathFind(_StartZone, _EndZone, _ZoneFilter, _Path, false))
	{
		::pathFind(_StartZone, _EndZone, CPropertySet(), _Path);	//	search without flags.
	}

	_PathCursor = 0;

	if (_Path.empty())
	{
	//	nlwarning("calcPath failed: for group '%s', can't find path between zones:", _Grp->getPersistent().getAliasFullName().c_str());
	//	nlwarning(" - from %s", _StartZone->getAliasTreeOwner().getAliasFullName().c_str());
	//	nlwarning(" - to %s", _EndZone->getAliasTreeOwner().getAliasFullName().c_str());
		RYAI_MAP_CRUNCH::CWorldPosition	startPos;
		CWorldContainer::getWorldMap().setWorldPosition(vp_auto, startPos, _StartZone->midPos());
		RYAI_MAP_CRUNCH::CWorldPosition	endPos;
		CWorldContainer::getWorldMap().setWorldPosition(vp_auto, endPos, _EndZone->midPos());
		_FollowRoute.setAIProfile(new CGrpProfileGoToPoint(_Grp, startPos, endPos, true));
		return;
	}
	nlassert(_PathCursor < _Path.size());
	_CurrentRoad=_Path[_PathCursor];
	_CurrentZone=_StartZone;
	
	_FollowRoute.setAIProfile(new	CGrpProfileFollowRoute(_Grp,	_CurrentRoad->coords(),	_CurrentRoad->verticalPos(), true));
	CGrpProfileFollowRoute*const	fr = static_cast<CGrpProfileFollowRoute*>(_FollowRoute.getAIProfile());
	
	nlassert(_CurrentZone==_CurrentRoad->startZone() || _CurrentZone==_CurrentRoad->endZone());
	fr->setDirection(_CurrentRoad->startZone()==_CurrentZone);
}

void CGrpProfileDynFollowPath::beginProfile()
{
	calcPath();
	CMoveProfile::beginProfile();
}

void CGrpProfileDynFollowPath::endProfile()
{
	_Path.clear();
	_StartZone = _EndZone = NULL;
	if( _FollowRoute.getAIProfile() != NULL)
		_FollowRoute.getAIProfile()->endProfile();
}


bool CGrpProfileDynFollowPath::destinationReach() const
{
	return _CurrentZone==_EndZone;
}

// routine called every time the bot is updated (frequency depends on player proximity, etc)
void CGrpProfileDynFollowPath::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpDynFollowProfileUpdate)
	CFollowPathContext fpcGrpDynFollowProfileUpdate("GrpDynFollowProfileUpdate");

	if	(destinationReach())
		return;

	if (_FollowRoute.getAIProfile() == NULL)
		return;

	CGroupNpc	&persGrp=*safe_cast<CGroupNpc*>(&_Grp->getPersistent());

	if	(_FollowRoute.getAIProfileType() == AITYPES::MOVE_FOLLOW_ROUTE)
	{
		if	(_CurrentZone.isNULL())
			return;
		
		// update the follow route until all bots reach the end
		_FollowRoute.updateProfile(ticksSinceLastUpdate);
		CGrpProfileFollowRoute *fr = safe_cast<CGrpProfileFollowRoute*>(_FollowRoute.getAIProfile());

		if	(fr->profileTerminated())
		{
			// next segment.
			_CurrentZone=fr->getDirection()?_CurrentRoad->endZone():_CurrentRoad->startZone();
			_PathCursor++;
			if	(_PathCursor < _Path.size())
			{
				_CurrentRoad=_Path[_PathCursor];
				

				fr = new	CGrpProfileFollowRoute(_Grp,	_CurrentRoad->coords(),	_CurrentRoad->verticalPos(), true);

				_FollowRoute.setAIProfile(fr);

				// pay attention to CGrpProfileFollowRouteSpawn init in static case .. :\ (to adapt ?)
#ifdef NL_DEBUG
				nlassert(_CurrentZone==_CurrentRoad->startZone() || _CurrentZone==_CurrentRoad->endZone());
#endif
				fr->setDirection(_CurrentRoad->startZone()==_CurrentZone);	//	CurrentZone);
			}
			else
			{
				_CurrentZone=_EndZone;
				//	Message Event for StateMachine.
				persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedFirst);
				persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedAll);
			}

		}

	}
	else if (_FollowRoute.getAIProfileType() == AITYPES::MOVE_GOTO_POINT)
	{
		// update the follow route until all bots reach the end
		_FollowRoute.updateProfile(ticksSinceLastUpdate);
		CGrpProfileGoToPoint* fr = safe_cast<CGrpProfileGoToPoint*>(_FollowRoute.getAIProfile());
		
		if	(fr->profileTerminated())
		{
			_CurrentZone=_EndZone;
			//	Message Event for StateMachine.
			persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedFirst);
			persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedAll);
		}
	}
	else
	{
#ifdef NL_DEBUG
		nlassert(false);
#endif
	}

}


// routine used to build a debug string for detailed information on a bot's status (with respect to their aiProfile)
std::string CGrpProfileDynFollowPath::getOneLineInfoString() const
{
	return NLMISC::toString("");
	//		//		CAIStatePositional *grpState=(CAIStatePositional *)bot->spawnGrp().getCAIState();
	//		return NLMISC::toString("stand_on_start_point(%s)%s",
	//			bot->getPersistent().getStartPos().toString().c_str(),
	//			(bot->getPersistent().getStartPos() != bot->posxy())? " EN ROUTE": " ARRIVED"
	//			);
}
	
void CGrpProfileDynCamping::beginProfile()
{
	// choose a random point in the camp to group around
	CGroup	&persGrp=_Grp->getPersistent();
	if	(!_CurrentZone.isNull())
	{
		RYAI_MAP_CRUNCH::CWorldPosition wp;
		_CurrentZone->getPlaceRandomPos().getRandomPos(wp);

		_CampPos = wp;
		// try to place all the npc around the camp pos.
		const	uint32	count			=	persGrp.bots().size();
		const	float	requiredSpace	=	2.f;
		const	float	radius			=	float(count*(requiredSpace/(2.f*NLMISC::Pi)));
		
		for (uint i=0; i<count; ++i)
		{
			const	CBot *const	bot = persGrp.bots()[i];
			if	(!bot)
				continue;

			CSpawnBot	*const	spawnBot=bot->getSpawnObj();
			if	(!spawnBot)
				continue;
			
			CAIVector pos(_CampPos.toAIVector());
			pos.setX(pos.x() + radius * cos(2.0f*NLMISC::Pi*(float(i)/count)));
			pos.setY(pos.y() + radius * sin(2.0f*NLMISC::Pi*(float(i)/count)));
			
			RYAI_MAP_CRUNCH::CWorldPosition newPos;
			TVerticalPos	vertPos=_CurrentZone->getPlaceRandomPos().getVerticalPos();	//	persGrp.getCurrentNpcZone()->getVerticalPos();
			
			if (!CWorldContainer::calcNearestWPosFromPosAnRadius(vertPos, newPos, pos, 6, 100, CWorldContainer::CPosValidatorDefault()))
				newPos=_CampPos;
			
			spawnBot->setAIProfile(new CBotProfileMoveTo(vertPos, newPos, spawnBot));
		}

	}
	// set the timer for end of camping to 5 à 10 mn
	_EndOfCamping.set(5*60*10 + CAIS::rand32(5*60*10));
}

void CGrpProfileDynCamping::endProfile()
{
	const	CCont<CBot >	&bots=_Grp->getPersistent().bots();
	
	for	(uint i=0; i<bots.size(); ++i)
	{
		const	CBot *const	bot = /*dynamic_cast<CBotNpc*>(*/bots[i]/*)*/;
		if	(!bot)
			continue;

		CSpawnBot *const	spawnBot = bot->getSpawnObj();
		if	(!spawnBot)
			continue;

		if (spawnBot->getAIProfileType() == BOT_MOVE_TO)
		{
			// set an idle activity
			spawnBot->setAIProfile(new CBotProfileStandAtPos(spawnBot));
		}
		else
		{
			// bot already at destination, stand up the bot
			spawnBot->setMode(MBEHAV::NORMAL);
		}

	}

}

void CGrpProfileDynCamping::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpCampingProfileUpdate)
	CFollowPathContext fpcGrpCampingProfileUpdate("GrpCampingProfileUpdate");

	const	CCont<CBot >	&bots=_Grp->getPersistent().bots();
	
	// turn the bot to the group center then sit them
	for	(uint i=0; i<bots.size(); ++i)
	{
		const	CBot	*const	bot = bots[i];
		if	(!bot)
			continue;
		
		CSpawnBot	*const	spawnBot = bot->getSpawnObj();
		if	(!spawnBot)
			continue;
		
		if	(spawnBot->getAIProfileType()!=BOT_MOVE_TO)
			continue;

		CBotProfileMoveTo const* const bp = safe_cast<CBotProfileMoveTo*>(spawnBot->getAIProfile());

		if	(!bp->destinationReach())
			continue;

		// ok, turn the bot to face the group center
		spawnBot->setTheta(CAngle(bot->getSpawnObj()->pos().angleTo(_CampPos)));
		// set an idle activity
		spawnBot->setAIProfile(new CBotProfileStandAtPos(spawnBot));
		// sit down the bot
		spawnBot->setMode(MBEHAV::SIT);
	}

}

std::string CGrpProfileDynCamping::getOneLineInfoString() const
{
	return string("dyn_camping");
}


///////////////////////////////////////////////////////////////////////////
void CGrpProfileDynWaitInZone::beginProfile()
{
	CGroupNpc	&persGrp=_Grp->getPersistent();

	// choose a random point in the camp to group around
	breakable
	{
		if	(_CurrentZone.isNull())
			break;
		if	(_CurrentZone->getPlaceRandomPos().getRandomPosCount()==0)
		{
			nlwarning("No Random Pos in _CurrentZone %s", _CurrentZone->getAliasTreeOwner().getAliasFullName().c_str());
			break;
		}

		RYAI_MAP_CRUNCH::CWorldPosition wp;
		_CurrentZone->getPlaceRandomPos().getRandomPos(wp);
		if	(!wp.isValid())
			break;

		WaitPos = wp;
		// try to place all the npc around the camp pos.
		const	uint32	count			=	persGrp.bots().size();
		const	float	requiredSpace	=	2.f;
		const	float	radius			=	float(count*(requiredSpace/(2.f*NLMISC::Pi)));

		for (uint i=0; i<count; ++i)
		{
			const	CBot	*const	bot = persGrp.bots()[i];
			if	(!bot)
				continue;
			
			CSpawnBot	*const	spawnBot = bot->getSpawnObj();
			if (!spawnBot)
				continue;
			
			CAIVector pos = WaitPos.toAIVector();
			pos.setX(pos.x() + radius * cos(2.0f*NLMISC::Pi*(float(i)/count)));
			pos.setY(pos.y() + radius * sin(2.0f*NLMISC::Pi*(float(i)/count)));

			RYAI_MAP_CRUNCH::CWorldPosition newPos;
			TVerticalPos	vertPos=_CurrentZone->getPlaceRandomPos().getVerticalPos();

			if	(!CWorldContainer::calcNearestWPosFromPosAnRadius(vertPos, newPos, pos, 6, 100, CWorldContainer::CPosValidatorDefault()))
				newPos = WaitPos;

			spawnBot->setAIProfile(new CBotProfileMoveTo(vertPos, newPos, spawnBot));
		}

	}
	StartOfWait = CTimeInterface::gameCycle();
}

void CGrpProfileDynWaitInZone::endProfile()
{
}

void CGrpProfileDynWaitInZone::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpDynWaitProfileUpdate)
	CFollowPathContext fpcGrpDynWaitProfileUpdate("GrpDynWaitProfileUpdate");

	const	CCont<CBot >	&bots=_Grp->getPersistent().bots();
	
	// turn the bot to the group center
	for (uint i=0; i<bots.size(); ++i)
	{
		const	CBot *const	bot = bots[i];
		if	(!bot)
			continue;
		
		CSpawnBot *const	spawnBot = bot->getSpawnObj();
		if	(!spawnBot)
			continue;

		if	(spawnBot->getAIProfileType() != BOT_MOVE_TO)
			continue;

		CBotProfileMoveTo const* const bp = safe_cast<CBotProfileMoveTo*>(spawnBot->getAIProfile());
		if	(!bp->destinationReach())
			continue;

		// ok, turn the bot to face the group center
		spawnBot->setTheta(CAngle(spawnBot->pos().angleTo(WaitPos)));
		// set an idle activity
		spawnBot->setAIProfile(new CBotProfileStandAtPos(spawnBot));
	}

}

std::string CGrpProfileDynWaitInZone::getOneLineInfoString() const
{
	return string("dyn_wait_in_zone profile");
}

#include "event_reaction_include.h"
