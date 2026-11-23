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

#include "states.h"
#include "event_reaction_container.h"
#include "state_profil.h"
#include "world_container.h"
#include "ai.h"
#include "ai_instance.h"
#include "state_instance.h"

extern NLMISC::CVariable<bool>	LogAcceptablePos;

using namespace	AITYPES;

std::string	CAIState::getIndexString	()	const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}

void	CAIState::updateDependencies	(const	CAIAliasDescriptionNode	&aliasTree, CAliasTreeOwner *aliasTreeOwner)
{	
	switch(aliasTree.getType())
	{
	case AITypeGrp:
		{
			CGroup	*const	group=NLMISC::safe_cast<CGroup*>(aliasTreeOwner);
			nlassert(group);			
			group->getPersistentStateInstance()->setStartState(getOwner()->states().getChildByAlias(getAlias()));
		}
		break;

	case AITypeEvent:
		{
			CAIEventReaction*const	eventPtr=NLMISC::safe_cast<CAIEventReaction*>(getOwner()->eventReactions().getAliasChildByAlias(aliasTree.getAlias()));
			nlassert(eventPtr);
			if	(!eventPtr)
				break;
			eventPtr->setState	(getAlias());
			eventPtr->setType	(CAIEventReaction::FixedState);
		}
		break;		
	}
	
}

IAliasCont*		CAIState::getAliasCont(TAIType	type)
{
	switch(type)
	{
	case AITypeNpcStateProfile:
		return	&_Profiles;
	case AITypeNpcStateChat:
		return	&_Chats;
	default:
		return	NULL;
	}

}

CAliasTreeOwner*	CAIState::createChild(IAliasCont *cont, CAIAliasDescriptionNode *aliasTree)
{
	CAliasTreeOwner*	child	=	NULL;
	
	switch(aliasTree->getType())
	{
	case AITypeNpcStateProfile:
		child	=	new CAIStateProfile(this, aliasTree);
		break;
	case AITypeNpcStateChat:
		child	=	new CAIStateChat(this, aliasTree);
		break;
	}
	
	if (child)
		cont->addAliasChild(child);	
	return	child;
}


CAIState	*CStateInstance::getCAIState	()
{
	return	_state;
}

bool	CShape::setPath(TVerticalPos verticalPos, const std::vector <CAIVector> &points)
{
	bool ret = true;

	_VerticalPos = verticalPos;
	takeHeights(points);

	_Geometry.clear	();
	_Geometry.reserve(points.size());

	for	(uint32 ind=0;ind<points.size();ind++)
	{
		RYAI_MAP_CRUNCH::CWorldPosition newpos;
		placePoint(newpos, ind, points[ind], 0, 1);

		if	(	!newpos.isValid()
			&&	!_AcceptInvalidPos	)
		{
			placePoint(newpos, ind, points[ind], 6, 100);
//#ifdef NL_DEBUG
			if (newpos.isValid())
			{
				if (LogAcceptablePos)
					nlinfo("CShape '%s': Path pos Error at position %s, an acceptable position could be %s",
						ownerName(), points[ind].toString().c_str(), newpos.toString().c_str());
			}
			else
			{
				nlwarning("CShape '%s': Path pos Error at position %s, no acceptable position found around",
					ownerName(), points[ind].toString().c_str());
				ret = false;
			}
//#endif
		}
		_Geometry.push_back(newpos);
	}
	_GeometryType=PATH;
	warnHeightsOffLayers();

	return ret;
}

bool	CShape::contains	(const	CAIVector	&pos)	const
{
	// Point or line can't contains !
	if	(_Geometry.size() < 3)
		return false;
	
	// Check with the bounding rectangle of the zone
	if	(	(pos.x() < _VMin.x())
		||	(pos.y() < _VMin.y())
		||	(pos.x() > _VMax.x())
		||	(pos.y() > _VMax.y())	)
		return false;
	
	uint32	nNbIntersection = 0;
	for (uint32	i = 0; i<_Geometry.size(); ++i)
	{
		const CAIVector	p1 = _Geometry[i];
		const CAIVector	p2 = _Geometry[(i+1)%_Geometry.size()];
		
		if	(	(p1.y() <= pos.y())
			&&	(p2.y() <= pos.y())	)
			continue;
		if	(	(p1.y() > pos.y())
			&&	(p2.y() > pos.y())	)
			continue;

		const	double	deltaX=p2.x()-p1.x();
		const	double	deltaY=p2.y()-p1.y();
		const	double	deltaYPos=pos.y()-p1.y();

		const	double	xinter = (double)p1.x() + deltaX*(deltaYPos/deltaY);
		if	(xinter > pos.x())
			++nNbIntersection;
	}
	return	((nNbIntersection&1)==1);	// odd intersections so the vertex is inside
}

bool	CShape::setPatat(TVerticalPos verticalPos, const std::vector <CAIVector> &points)
{
	bool ret = true;
	_VerticalPos = verticalPos;
	takeHeights(points);
//	_Geometry=points;
	_GeometryType=PATAT;
	_Geometry.clear();
	_Geometry.reserve(points.size());

	// create a list of valid position inside the patat

	CAIVector vMin, vMax;

	// Point or line can't contains !
	if (points.size() < 3)
		return true;
	
	// Get the bounding rectangle of the zone
	vMax = vMin = points[0];
	for (uint i = 0; i < points.size(); ++i)
	{
		if (vMin.x() > points[i].x())
			vMin.setX(points[i].x());
		if (vMin.y() > points[i].y())
			vMin.setY(points[i].y());

		if (vMax.x() < points[i].x())
			vMax.setX(points[i].x());
		if (vMax.y() < points[i].y())
			vMax.setY(points[i].y());
	}

	_VMin = vMin;
	_VMax = vMax;

	// fill the geometry vector with invalid word pos
	for (uint i=0; i<points.size(); ++i)
	{
		_Geometry.push_back(TPosition(sint(points[i].x().asDouble()), sint(points[i].y().asDouble())));
	}
	// find a valid position for every geometric 

	RYAI_MAP_CRUNCH::CWorldPosition worldPos;
	if (!placePoint(worldPos, 0, points[0], float((vMax - vMin).quickNorm()), 1000))
	{
		if	(!_AcceptInvalidPos)
		{
			nlwarning("CShape '%s': can't find a valid position at %s", ownerName(), points[0].toString().c_str());
			ret = false;
		}
	}
	else
	{
		buildRandomPos(worldPos, float((vMax-vMin).quickNorm()));
	}

	// build the valid pos for each patat point
	_Geometry.clear();
	_Geometry.reserve(points.size());
	for (uint32 ind=0;ind<points.size();ind++)
	{
		RYAI_MAP_CRUNCH::CWorldPosition newpos;
		placePoint(newpos, ind, points[ind], 0, 1);
		if	(	!newpos.isValid()
			&&	!_AcceptInvalidPos)
				
		{
			placePoint(newpos, ind, points[ind], 6, 100);
//#ifdef NL_DEBUG
			if (newpos.isValid())
			{
				if (LogAcceptablePos)
					nlinfo("CShape '%s': Path pos Error at position %s, an acceptable position could be %s",
						ownerName(), points[ind].toString().c_str(), newpos.toString().c_str());
			}
			else
			{
				nlwarning("CShape '%s': Path pos Error at position %s, no acceptable position found around",
					ownerName(), points[ind].toString().c_str());
				ret = false;
			}
//#endif
		}
		_Geometry.push_back(newpos);
	}
	warnHeightsOffLayers();

	return ret;
}

bool CShape::calcRandomPos(CAIPos &pos)	const
{
	CAIVector v(
		(double(_VMin.x()) + CAIS::rand32(_VMax.x() - _VMin.x()))/CAICoord::UNITS_PER_METER,
		(double(_VMin.y()) + CAIS::rand32(_VMax.y() - _VMin.y()))/CAICoord::UNITS_PER_METER);
	if ((v.x() < _VMin.x()) || (v.y() < _VMin.y()) || (v.x() > _VMax.x()) || (v.y() > _VMax.y()))
		return false;
	
	uint32 nNbIntersection = 0;
	for (uint k = 0; k < _Geometry.size(); ++k)
	{
		const CAIVector &p1 = _Geometry[k];
		const CAIVector &p2 = _Geometry[(k+1)%_Geometry.size()];
		
		if (((p1.y()-v.y()) < 0.0)&&((p2.y()-v.y()) < 0.0))
			continue;
		if (((p1.y()-v.y()) > 0.0)&&((p2.y()-v.y()) > 0.0))
			continue;
		if ((p2.y()-p1.y()) == 0)
			continue;

		const	float delta = (v.y()-p1.y()).asInt()/float(p2.y()-p1.y());
		const	float xinter = (float)(p1.x().asInt() + (p2.x()-p1.x()).asInt() * delta);
		if (xinter > v.x())
			++nNbIntersection;
	}
	if ((nNbIntersection&1) == 1) // odd intersections so the vertex is inside
	{
		pos = CAIPos(v, 0, 0);
		return true;
	};

	return false;
}

void CShape::takeHeights(const std::vector<CAIVector> &points)
{
	m_Heights.clear();
	m_HeightPoints.clear();
	if (m_NextHeights.empty())
		return;

	if (m_NextHeights.size() != points.size())
		nlwarning("CShape: '%s' has %u heights for %u points, its points are placed by their vertical pos instead", ownerName(), (uint)m_NextHeights.size(), (uint)points.size());
	else
	{
		m_Heights = m_NextHeights;
		m_HeightPoints = points;
	}
	m_NextHeights.clear();
}

void CShape::warnHeightsOffLayers() const
{
	for (uint32 i = 0; i < m_HeightPoints.size(); ++i)
	{
		RYAI_MAP_CRUNCH::CWorldPosition wpos;
		bool atHeight;
		if (CWorldContainer::getWorldMap().setWorldPositionNearestHeight(m_Heights[i], 0.0, _VerticalPos, wpos, m_HeightPoints[i], NULL, &atHeight) && !atHeight)
			nlwarning("CShape: point %u of '%s' at %s has a height of %.1f m that no layer there is near, it stands on the layer of its vertical pos",
			    i, ownerName(), m_HeightPoints[i].toString().c_str(), m_Heights[i] / 1000.0);
	}
}

bool CShape::placePoint(RYAI_MAP_CRUNCH::CWorldPosition &wpos, uint32 index, const CAIVector &pos, float radius, uint32 tries) const
{
	if (index < m_Heights.size())
		return CWorldContainer::calcNearestWPosAtHeight(m_Heights[index], 0.0, _VerticalPos, wpos, pos, radius, tries, CWorldContainer::CPosValidatorDefault());
	return CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, wpos, pos, radius, tries, CWorldContainer::CPosValidatorDefault());
}

sint32 CShape::heightAt(const CAIVector &pos, double &distance) const
{
	// the heights of the points, weighted by the inverse of their squared distance
	double sum = 0.0, weights = 0.0, minDistSq = 0.0;
	for (uint32 i = 0; i < m_HeightPoints.size(); ++i)
	{
		const double distSq = pos.distSqTo(m_HeightPoints[i]);
		if (distSq < 0.01)
		{
			distance = 0.0;
			return m_Heights[i];
		}
		if (i == 0 || distSq < minDistSq)
			minDistSq = distSq;
		sum += m_Heights[i] / distSq;
		weights += 1.0 / distSq;
	}
	// how far it is from a height that was measured
	distance = sqrt(minDistSq);
	return (sint32)(sum / weights);
}

bool CShape::placePos(RYAI_MAP_CRUNCH::CWorldPosition &wpos, const CAIVector &pos) const
{
	if (m_Heights.empty())
		return CPlaceRandomPos::placePos(wpos, pos);
	double distance;
	const sint32 height = heightAt(pos, distance);
	// inside the shape its points around the position measured the ground there: the height chooses however far the
	// nearest point is (a large zone in a cave or under an arch), with the widest tolerance
	if (distance > (double)RYAI_MAP_CRUNCH::CWorldMap::HeightReach && contains(pos))
		distance = (double)RYAI_MAP_CRUNCH::CWorldMap::HeightReach;
	return CWorldContainer::getWorldMap().setWorldPositionNearestHeight(height, distance, _VerticalPos, wpos, pos);
}
