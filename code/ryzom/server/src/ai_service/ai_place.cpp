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
#include "ai_place.h"
#include "ai_place_xyr.h"
#include "ai_place_patat.h"
#include "ai_instance.h"
#include "ai_grp_fauna.h" // TODO nico : try to get rid of that dependency

//////////////////////////////////////////////////////////////////////////////
// CAIPlace                                                                 //
//////////////////////////////////////////////////////////////////////////////

std::string CAIPlace::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceXYR                                                              //
//////////////////////////////////////////////////////////////////////////////

bool CAIPlaceXYR::atPlace(CAIEntityPhysical const* entity) const
{
	return atPlace(entity->pos());
}

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceXYRFauna                                                         //
//////////////////////////////////////////////////////////////////////////////
std::string CAIPlaceXYRFauna::getOneLineInfoString() const
{
	std::string result = NLMISC::toString("Name = %s; Active : %s, index = %d ", getFullName().c_str(), getActive() ? "on" : "off", (int) getIndex());
	if (getFlag(FLAG_SPAWN)) result +="spawn ";
	if (getFlag(FLAG_EAT)) result +="food ";
	if (getFlag(FLAG_REST)) result +="rest ";
	if (getTimeDriven()) result += NLMISC::toString(" TIME_DRIVEN : days=%s; time=%s", getDayInterval().c_str(), getTimeInterval().c_str());
	return result;	
}

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceFastXYR                                                          //
//////////////////////////////////////////////////////////////////////////////

bool CAIPlaceFastXYR::atPlace(CAIEntityPhysical const* entity) const
{
	return atPlace(entity->pos());
}

void CAIPlaceFastXYR::display(CStringWriter& stringWriter) const
{
	stringWriter.append("XYR: ("+_Pos.x().toString()
		+" "
		+_Pos.y().toString()
		+" "+NLMISC::toString(_Pos.h())
		+") Radius "
		+NLMISC::toString(_Radius)
		+" "
		+getName());
}

uint CFaunaGenericPlace::setupFromOldName(const std::string &name)
{
	uint32 stayTime;
	uint placeIndex = ~0;
	// depending on place name setup eat/ rest/ sleep pointers
	if (NLMISC::nlstricmp(name,"spawn")==0)
	{
		placeIndex = CGrpFauna::SPAWN_PLACE;
		setIndex(0);
		setReachNext(true);
		setFlag(FLAG_SPAWN, true);
		stayTime = CGrpFauna::refTimer(CGrpFauna::CORPSE_TIME);
	}
	else
	if (NLMISC::nlstricmp(name,"food")==0)
	{
		placeIndex = CGrpFauna::EAT_PLACE;
		setIndex(1);
		setReachNext(true);
		setFlag(FLAG_EAT, true);
		stayTime = CGrpFauna::refTimer(CGrpFauna::EAT_TIME);
	}
	else
	if (NLMISC::nlstricmp(name,"rest")==0)
	{
		placeIndex = CGrpFauna::REST_PLACE;
		setIndex(2);
		std::vector<sint32> arcs(1);
		arcs[0] = 1; // can reach place 1 from place 2
		setArcs(arcs);
		setFlag(FLAG_REST, true);
		stayTime = CGrpFauna::refTimer(CGrpFauna::REST_TIME);
	}
	else
	{
		nlwarning("Unknown fauna place type");
		nlassert(0);
	}
	stayTime *= FAUNA_BEHAVIOR_GLOBAL_SCALE;	
	setMinStayTime(stayTime);
	setMaxStayTime(stayTime);
	return placeIndex;
}

bool  CFaunaGenericPlace::getActive() const
{
	if (!_TimeDriven) return _Active;
	// NB : INDICES FOR DAYS are expected to start at 1!!
	extern bool FAUNA_GRAPH_USES_DEBUG_TIME;
	const CRyzomTime &rt = FAUNA_GRAPH_USES_DEBUG_TIME ? CTimeInterface::getRyzomDebugTime() : CTimeInterface::getRyzomTime();
	std::vector<std::string> dayIntervals;
	NLMISC::explode(_DayInterval, std::string(","), dayIntervals, true);
	std::string season = EGSPD::CSeason::toString(rt.getRyzomSeason());
	std::string month = MONTH::toString((MONTH::EMonth) rt.getRyzomMonth());
	std::string weekday = WEEKDAY::toString((WEEKDAY::EWeekDay) rt.getRyzomDay());	
	bool found = false;
	for (uint k = 0; k < dayIntervals.size(); ++k)
	{		
		bool goodToken = false;
		if (NLMISC::nlstricmp(dayIntervals[k], "always") == 0)
		{
			found = true;
			break;
		}
		if (NLMISC::nlstricmp(dayIntervals[k], season) == 0)
		{
			found = true;
			break;
		}		
		if (EGSPD::CSeason::fromString(season) != EGSPD::CSeason::Unknown)
		{
			goodToken = true;
		}
		if (NLMISC::nlstricmp(dayIntervals[k], month) == 0)
		{
			found = true;
			break;
		}
		if (MONTH::toMonth(dayIntervals[k]) != MONTH::UNKNOWN)
		{
			goodToken = true;
		}
		if (NLMISC::nlstricmp(dayIntervals[k], weekday) == 0)
		{
			found = true;
			break;
		}
		if (WEEKDAY::toWeekDay(dayIntervals[k]) != WEEKDAY::UNKNOWN)
		{
			goodToken = true;
		}
		// see if this is a n interval
		int startDay, endDay;
		if (sscanf(dayIntervals[k].c_str(), "%d-%d", &startDay, &endDay) == 2)
		{			
			goodToken = true;
			if ((int) (rt.getRyzomDay() + 1) >= startDay && (int) (rt.getRyzomDay() + 1) <= endDay)
			{
				found = true;
				break;
			}
		}
		// see if this is a single day
		int day;
		if (sscanf(dayIntervals[k].c_str(), "%d", &day) == 1)
		{
			goodToken = true;
			if ((int) (rt.getRyzomDay() + 1) == day)
			{
				found = true;
				break;
			}
		}
		if (!goodToken)
		{
			nlwarning("Unknwon time interval token : %s", dayIntervals[k].c_str());
		}
	}
	if (!found) return false;
	// test against time interval
	// If no interval is given then assume whole day
	if (_TimeInterval.empty()) return true;
	std::vector<std::string> timeIntervals;
	NLMISC::explode(_TimeInterval, std::string(","), timeIntervals, true);
	for (uint k = 0; k < dayIntervals.size(); ++k)
	{				
		uint startHour, endHour;
		if (sscanf(timeIntervals[k].c_str(), "%d-%d", &startHour, &endHour) == 2)
		{			
			if (startHour > endHour)
			{
				// reversed interval
				if (rt.getRyzomTime() >= startHour || rt.getRyzomTime() <= endHour)
				{
					return true;
				}
			}
			else
			{				
				if (rt.getRyzomTime() >= startHour && rt.getRyzomTime() <= endHour)
				{
					return true;
				}
			}			
		}
		else
		{
			nlwarning("Unknwon time interval token : %s", timeIntervals[k].c_str());
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceShape                                                            //
//////////////////////////////////////////////////////////////////////////////

CAIPlaceShape::CAIPlaceShape(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription, bool warnOnInvalidPosition)
: CAIPlace(owner, aliasDescription), _Shape(!warnOnInvalidPosition)
{
	_Shape.calcRandomPos(_MidPos);
	CWorldContainer::getWorldMap().setWorldPosition(AITYPES::vp_auto, _WorldValidPos, _MidPos);
	_Shape.calcRandomPos(_MidPos);
}

bool CAIPlaceShape::atPlace(CAIVector const& pos) const
{
	return _Shape.contains(pos);
}

bool CAIPlaceShape::atPlace(const CAIVectorMirror &pos) const
{
	return _Shape.contains(pos);
}

bool CAIPlaceShape::atPlace(CAIEntityPhysical const* entity) const
{
	return atPlace(entity->pos());
}

CAIPos const& CAIPlaceShape::midPos() const
{
	return _MidPos;
}

RYAI_MAP_CRUNCH::CWorldPosition const& CAIPlaceShape::worldValidPos() const
{
	return _WorldValidPos;
}

float CAIPlaceShape::getRadius() const
{
	// :TODO: Compute a real radius here.
	return 20.f;
}

AITYPES::TVerticalPos CAIPlaceShape::getVerticalPos() const
{
	return _Shape.getVerticalPos();
}

void CAIPlaceShape::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const
{
	CAIPos	aiPos;
	_Shape.calcRandomPos(aiPos);
	CWorldContainer::getWorldMap().setWorldPosition(AITYPES::vp_auto, pos, aiPos);
}

bool CAIPlaceShape::calcRandomPos(CAIPos& pos) const
{
	return _Shape.calcRandomPos(pos);
}

bool CAIPlaceShape::setPatat(AITYPES::TVerticalPos verticalPos, std::vector<CAIVector> const& points)
{
	return _Shape.setPatat(verticalPos, points);
}

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceIntersect                                                        //
//////////////////////////////////////////////////////////////////////////////

CAIPlaceIntersect::CAIPlaceIntersect(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription)
: CAIPlace(owner, aliasDescription)
{
}

bool CAIPlaceIntersect::atPlace(CAIVector const& pos) const
{
	if (_Place1 && _Place2)
		return _Place1->atPlace(pos) && _Place2->atPlace(pos);
	if (_Place1)
		return _Place1->atPlace(pos);
	if (_Place2)
		return _Place2->atPlace(pos);
	return false;
}

bool CAIPlaceIntersect::atPlace(const CAIVectorMirror &pos) const
{
	if (_Place1 && _Place2)
		return _Place1->atPlace(pos) && _Place2->atPlace(pos);
	if (_Place1)
		return _Place1->atPlace(pos);
	if (_Place2)
		return _Place2->atPlace(pos);
	return true;
}

bool CAIPlaceIntersect::atPlace(CAIEntityPhysical const* pos) const
{
	if (_Place1 && _Place2)
		return _Place1->atPlace(pos) && _Place2->atPlace(pos);
	if (_Place1)
		return _Place1->atPlace(pos);
	if (_Place2)
		return _Place2->atPlace(pos);
	return true;
}

CAIPos const& CAIPlaceIntersect::midPos() const
{
	nlassert(_Place1 || _Place2);
	if (_Place1)
		return _Place1->midPos();
	if (_Place2)
		return _Place2->midPos();
	return _DummyMidPos;
}

RYAI_MAP_CRUNCH::CWorldPosition const& CAIPlaceIntersect::worldValidPos() const
{
	nlassert(_Place1 || _Place2);
	if (_Place1)
		return _Place1->worldValidPos();
	if (_Place2)
		return _Place2->worldValidPos();
	return _DummyValidPos;
}

float CAIPlaceIntersect::getRadius() const
{
	nlassert(_Place1 || _Place2);
	if (_Place1)
		return _Place1->getRadius();
	if (_Place2)
		return _Place2->getRadius();
	return 0.f;
}

AITYPES::TVerticalPos CAIPlaceIntersect::getVerticalPos() const
{
	nlassert(_Place1 || _Place2);
	if (_Place1)
		return _Place1->getVerticalPos();
	if (_Place2)
		return _Place2->getVerticalPos();
	return AITYPES::vp_auto;
}

void CAIPlaceIntersect::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const
{
	nlassert(_Place1 || _Place2);
	if (_Place1)
		_Place1->getRandomPos(pos);
	if (_Place2)
		_Place2->getRandomPos(pos);
}

bool CAIPlaceIntersect::calcRandomPos(CAIPos& pos) const
{
//	if (_Place1 && _Place1->calcRandomPos(pos))
//		return true;
//	if (_Place2 && _Place2->calcRandomPos(pos))
//		return true;
	return false;
}

void CAIPlaceIntersect::setPlace1(NLMISC::CSmartPtr<CAIPlace const> const& place)
{
	_Place1 = place;
}

void CAIPlaceIntersect::setPlace2(NLMISC::CSmartPtr<CAIPlace const> const& place)
{
	_Place2 = place;
}

//////////////////////////////////////////////////////////////////////////////
// CAIPlaceOutpost                                                          //
//////////////////////////////////////////////////////////////////////////////

CAIPlaceOutpost::CAIPlaceOutpost(CPlaceOwner* owner, CAIAliasDescriptionNode* aliasDescription)
: CAIPlaceShape(owner, aliasDescription,false)
, _OutpostAlias(0)
{
}

bool CAIPlaceOutpost::atPlace(CAIEntityPhysical const* entity) const
{
	if (_OutpostAlias!=0)
	{
		CMirrorPropValueRO<TYPE_IN_OUTPOST_ZONE_ALIAS> entityInOutpostAlias(TheDataset, entity->dataSetRow(), DSPropertyIN_OUTPOST_ZONE_ALIAS);
		return (entityInOutpostAlias == _OutpostAlias);
	}
	return CAIPlaceShape::atPlace(entity);
}

void CAIPlaceOutpost::setOutpostAlias(uint32 outpostAlias)
{
	_OutpostAlias = outpostAlias;
}

