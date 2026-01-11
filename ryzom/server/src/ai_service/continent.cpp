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
#include "nel/misc/string_mapper.h"
#include "nel/net/service.h"
#include "algorithm"
#include "continent.h"
#include "ai_instance.h"
#include "ai_mgr.h"
#include "ai_grp_fauna.h"
#include "ai_mgr_fauna.h"
#include "ai_grp_npc.h"
#include "ai_mgr_npc.h"
#include "ai_bot_npc.h"
#include "group_profile.h"
//#include "family_behavior.h"
#include "family_profile_tribe.h"

#include "continent_inline.h"

using namespace MULTI_LINE_FORMATER;

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RYAI_MAP_CRUNCH;
using namespace AITYPES;

CVariable<bool> LogScorerScores("ai", "LogScorerScores", "", false, 0, true);


//////////////////////////////////////////////////////////////////////////////
// CFaunaZone                                                               //
//////////////////////////////////////////////////////////////////////////////

CFaunaZone::CFaunaZone(CCell *owner, CAIAliasDescriptionNode *adn)
	: CAIPlaceXYR(owner, adn)
{
//	nldebug("Creating fauna zone '%s', alias %u", getName().c_str(), getAlias());
}
CFaunaZone::~CFaunaZone()
{
//	nldebug("Deleting fauna zone '%s', alias %u", getName().c_str(), getAlias());
}

CCell* CFaunaZone::getOwner()	const
{
	return static_cast<CCell*>(CAIPlaceXYR::getOwner());
}

//////////////////////////////////////////////////////////////////////////////
// CNpcZone                                                                 //
//////////////////////////////////////////////////////////////////////////////

CNpcZone::~CNpcZone()
{
	unlinkNpcZone();
}


void CNpcZone::unlinkNpcZone()
{
	while (!_Roads.empty())
	{
		CRoad *road = _Roads.back();
		if (road->startZone().ptr() == this)
		{
			road->setStartZone(0);
		}
		else if (road->endZone().ptr() == this)
		{
			road->setEndZone(0);
		}
		else
		{
			nlassert(false);
		}

		_Roads.pop_back();
	}
}


float CNpcZone::getFreeAreaScore() const
{
	const float area = getArea();
	return area/((float)getNbUse()+0.001f);
}	

std::string	CNpcZone::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":nz%d", getIndex());
}

AITYPES::CPropertySet& CNpcZone::properties()
{
	return _Properties;
}	
const AITYPES::CPropertySet& CNpcZone::properties() const
{
	return _Properties;
}
std::vector<NLMISC::CDbgPtr<CRoad> >& CNpcZone::roads()
{
	return	_Roads;
}

uint32 CNpcZone::getNbUse() const
{
	const sint nbUse = getRefCount() - 1; // -1 for the container
#ifdef NL_DEBUG
	nlassert(nbUse>=0);
#endif
	return	nbUse;
}

//////////////////////////////////////////////////////////////////////////////
// CNpcZonePlace                                                            //
//////////////////////////////////////////////////////////////////////////////

CNpcZonePlace::CNpcZonePlace(CCell *owner, CAIAliasDescriptionNode *adn)
	: CAIPlace(owner, adn)
{
	owner->getOwner()->getAIInstance()->addZone(getAliasFullName(), this);	
//	nldebug("Creating npc zone '%s', alias %u", getName().c_str(), getAlias());
}

CNpcZonePlace::~CNpcZonePlace()
{
	getOwner()->getOwner()->getAIInstance()->removeZone(getAliasFullName(), this);
	
//	nldebug("Deleting npc zone '%s', alias %u", getName().c_str(), getAlias());
}

CCell* CNpcZonePlace::getOwner() const
{ 
	return static_cast<CCell*>(CAIPlace::getOwner());
}

const CAliasTreeOwner& CNpcZonePlace::getAliasTreeOwner() const
{
	return *this;
}

uint32 CNpcZonePlace::getIndex() const
{
	return getIndex();
}

CPlaceRandomPos& CNpcZonePlace::getPlaceRandomPos()
{
	return *this;
}

const CPlaceRandomPos& CNpcZonePlace::getPlaceRandomPos() const
{
	return *this;
}

bool CNpcZonePlace::atPlace(const CAIVector& pos) const 
{
	return (pos-_pos).sqrnorm() <= ((double)_radius*(double)_radius);
}

bool CNpcZonePlace::atPlace(const CAIVectorMirror& pos) const 
{
	return (pos-_pos).sqrnorm() <= ((double)_radius*(double)_radius);
}

bool CNpcZonePlace::atPlace(CAIEntityPhysical const* entity) const
{
	return atPlace(entity->pos());
}

const CAIPos& CNpcZonePlace::midPos() const
{
	return	_pos;
}

float CNpcZonePlace::getArea() const
{
	const float	radius = getRadius();
	return (float)(radius*radius*NLMISC::Pi);
}	

const RYAI_MAP_CRUNCH::CWorldPosition& CNpcZonePlace::worldValidPos() const
{
	return _worldValidPos;
}

float CNpcZonePlace::getRadius() const
{
	return _radius;
}

void CNpcZonePlace::display(CStringWriter& stringWriter) const
{
	// :TODO: Implement that method
	nlassert(false && "not yet implemented!");
}

AITYPES::TVerticalPos CNpcZonePlace::getVerticalPos() const
{
	return CPlaceRandomPos::getVerticalPos();
}
void CNpcZonePlace::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const
{
	CPlaceRandomPos::getRandomPos(pos);
}

void CNpcZonePlace::setPosAndRadius(AITYPES::TVerticalPos verticalPos, const CAIPos& pos, uint32 radius)
{
	_VerticalPos = verticalPos;
	_pos = pos;
	_radius = float(radius)/1000.0f;
#ifdef NL_DEBUG
	nlassert(_radius > 0);
	nlassert(pos.x()!=0||pos.y()!=0);
#endif
	if (	pos.x()==0
		&&	pos.y()==0)
	{
		nlwarning("Null place Position for %s", getAliasFullName().c_str());
	}
	
	if (!CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, _worldValidPos, _pos, _radius, 1000, CWorldContainer::CPosValidatorDefault()))
	{
		if (LogAcceptablePos)
			nlwarning("Unvalid place (no collision free position found) at %d %d ", _pos.x().asInt(), _pos.y().asInt());
	}
	
	buildRandomPos(_worldValidPos, _radius);
}

bool CNpcZonePlace::calcRandomPos(CAIPos& pos) const
{
	double dx, dy;
	const double r = (double)_radius;
	const double rSquare = r*r;
	// :TODO: Replace that while with a theta/r rand and a space conversion
	do
	{
		dx = CAIS::frandPlusMinus(r);
		dy = CAIS::frandPlusMinus(r);
	}
	while (dx*dx+dy*dy>rSquare);
	
	pos.setX(_pos.x()+dx);
	pos.setY(_pos.y()+dy);
	pos.setH(_pos.h());
	pos.setTheta(pos.angleTo(_pos)+CAngle(NLMISC::Pi/2));
	
	return true;
}	

//////////////////////////////////////////////////////////////////////////////
// CNpcZonePlaceNoPrim                                                      //
//////////////////////////////////////////////////////////////////////////////

CNpcZonePlaceNoPrim::CNpcZonePlaceNoPrim()
{
}

CNpcZonePlaceNoPrim::~CNpcZonePlaceNoPrim()
{
}

bool CNpcZonePlaceNoPrim::atPlace(const CAIVector& pos) const 
{
	return (pos-_Pos).sqrnorm() <= ((double)_Radius*(double)_Radius);
}

bool CNpcZonePlaceNoPrim::atPlace(const CAIVectorMirror& pos) const 
{
	return (pos-_Pos).sqrnorm() <= ((double)_Radius*(double)_Radius);
}

bool CNpcZonePlaceNoPrim::atPlace(CAIEntityPhysical const* entity) const
{
	return atPlace(entity->pos());
}

const CAIPos& CNpcZonePlaceNoPrim::midPos() const
{
	return _Pos;
}

float CNpcZonePlaceNoPrim::getArea() const
{
	return (float)(_Radius*_Radius*NLMISC::Pi);
}	

const RYAI_MAP_CRUNCH::CWorldPosition& CNpcZonePlaceNoPrim::worldValidPos() const
{
	return _WorldValidPos;
}

float CNpcZonePlaceNoPrim::getRadius() const
{
	return _Radius;
}

void CNpcZonePlaceNoPrim::display(CStringWriter& stringWriter) const
{
	// :TODO: Implement that method
	nlassert(false && "not yet implemented!");
}

AITYPES::TVerticalPos CNpcZonePlaceNoPrim::getVerticalPos() const
{
	return AITYPES::vp_auto;
}
void CNpcZonePlaceNoPrim::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const
{
	uint maxTries = RandomPosMaxRetry;
	CAIPos dummyPos;
	bool foundRandomPos = false;
	while (!foundRandomPos)
	{
		if (calcRandomPos(dummyPos))
			foundRandomPos = CWorldContainer::getWorldMap().setWorldPosition(_VerticalPos, pos, dummyPos);
		--maxTries;
		if (maxTries<=0)
			break;
	}
	if (!foundRandomPos)
	{
		if (!CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, pos, _WorldValidPos, _Radius, 1000, CWorldContainer::CPosValidatorDefault()))
		{
			if (LogAcceptablePos)
				nlwarning("Unvalid place (no collision free position found) at %d %d", _WorldValidPos.toAIVector().x().asInt(), _WorldValidPos.toAIVector().y().asInt());
		}
	}
}

void CNpcZonePlaceNoPrim::setPosAndRadius(AITYPES::TVerticalPos verticalPos, const CAIPos& pos, uint32 radius)
{
#ifdef NL_DEBUG
	nlassert(radius > 0);
	nlassert(pos.x()!=0||pos.y()!=0);
#endif
	
	_VerticalPos = verticalPos;
	_Pos = pos;
	_Radius = float(radius)/1000.0f;
	
	if (!CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, _WorldValidPos, _Pos, _Radius, 1000, CWorldContainer::CPosValidatorDefault()))
	{
		if (LogAcceptablePos)
			nlwarning("Unvalid place (no collision free position found) at %d %d ", _Pos.x().asInt(), _Pos.y().asInt());
	}
}

bool CNpcZonePlaceNoPrim::calcRandomPos(CAIPos& pos) const
{
	double dx, dy;
	const double r = (double)_Radius;
	const double rSquare = r*r;
	// :TODO: Replace that while with a theta/r rand and a space conversion
	do
	{
		dx = CAIS::frandPlusMinus(r);
		dy = CAIS::frandPlusMinus(r);
	}
	while (dx*dx+dy*dy>rSquare);
	
	pos.setX(_Pos.x()+dx);
	pos.setY(_Pos.y()+dy);
	pos.setH(_Pos.h());
	pos.setTheta(pos.angleTo(_Pos)+CAngle(NLMISC::Pi/2));
	
	return true;
}	

//////////////////////////////////////////////////////////////////////////////
// CNpcZoneShape                                                            //
//////////////////////////////////////////////////////////////////////////////

CNpcZoneShape::CNpcZoneShape(CCell* owner, CAIAliasDescriptionNode* adn)
	: CAIPlace(owner, adn)
{
	owner->getOwner()->getAIInstance()->addZone(getAliasFullName(), this);	
//	nldebug("Creating npc zone '%s', alias %u", getName().c_str(), getAlias());
}

CNpcZoneShape::~CNpcZoneShape()
{
	getOwner()->getOwner()->getAIInstance()->removeZone(getAliasFullName(), this);
//	nldebug("Deleting npc zone '%s', alias %u", getName().c_str(), getAlias());
}

CCell* CNpcZoneShape::getOwner() const
{ 
	return static_cast<CCell*>(CAIPlace::getOwner());
}
const CAliasTreeOwner& CNpcZoneShape::getAliasTreeOwner() const
{
	return *this;
}
uint32 CNpcZoneShape::getIndex() const
{
	return getIndex();
}

CPlaceRandomPos& CNpcZoneShape::getPlaceRandomPos()
{
	return this->_shape;
}
const CPlaceRandomPos& CNpcZoneShape::getPlaceRandomPos() const
{
	return this->_shape;
}

bool CNpcZoneShape::atPlace(const CAIVector& pos) const 
{
	return _shape.contains(pos);
}
bool CNpcZoneShape::atPlace(const CAIVectorMirror &pos) const 
{
	return _shape.contains(pos);
}
bool CNpcZoneShape::atPlace(CAIEntityPhysical const* entity) const
{
	return atPlace(entity->pos());
}

const CAIPos& CNpcZoneShape::midPos() const
{
	return _midPos;
}

float CNpcZoneShape::getArea() const
{
	const float area = 20.0f;
	nlwarning("CNpcZoneShape area asked although it's a fake value.");
	return area;
}	

const RYAI_MAP_CRUNCH::CWorldPosition& CNpcZoneShape::worldValidPos() const
{
	return _worldValidPos;
}

float CNpcZoneShape::getRadius() const
{
	const float radius = 1.0f;
	return radius;
}

void CNpcZoneShape::display(CStringWriter& stringWriter) const
{
	// :TODO: Implement that method
	nlassert(false && "not yet implemented!");
}

AITYPES::TVerticalPos CNpcZoneShape::getVerticalPos() const
{
	return	_shape.getVerticalPos();
}
void CNpcZoneShape::getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const
{
	_shape.getRandomPos(pos);
}

void CNpcZoneShape::setPatat(AITYPES::TVerticalPos verticalPos, const std::vector<CAIVector>& points)
{
	if (!_shape.setPatat(verticalPos, points))
	{
		nlwarning("CNpcZoneShape::setPatat: error while placing the points of '%s'",
			getAliasFullName().c_str());
	}
	buildMidPos();
}
void CNpcZoneShape::buildMidPos()
{
	//nlassert(false && "build appropriate _midPos");
	int count = 0;
	bool succeeded = _shape.calcRandomPos(_midPos);
	while (!succeeded && count<1000)
	{
		succeeded = _shape.calcRandomPos(_midPos);
		++count;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CCell                                                                    //
//////////////////////////////////////////////////////////////////////////////

std::string	CCell::getIndexString()	const
{
	return getOwner()->getIndexString()+NLMISC::toString(":ce%d", getChildIndex());
}

void	CCell::connectRoads()
{
	CContinent	&continent=*getOwner()->getOwner()->getOwner();
	vector<CCell*>	neighBourgCellList;
	getNeighBourgCellList(neighBourgCellList);
	
	for (uint i=0; i<_Roads.size(); ++i)
	{
		CRoad *road = roads()[i];
		if (!road)
			continue;
		
		road->calcLength	();
		// clear existing link;
		road->unlinkRoad();
		
		if	(road->coords().size() > 1)
		{
			// ok, there are 2 points, try to found the zone they belong to
			
			// starting zone try first in the same CCell, after in the neighbourg, and last in the whole continent.
			CNpcZone *nz = findNpcZone(road->getLogicStart());
			if	(!nz)
			{
				FOREACH(itCell, vector<CCell*>, neighBourgCellList)
				{
					nz=(*itCell)->findNpcZone(road->getLogicStart());
					if (nz)
						break;
				}
			}
			if (!nz)
				nz = continent.findNpcZone(road->getLogicStart());
			
			if	(nz)
			{
				// ok, we found the first zone
				road->setStartZone(nz);
				nz->roads().push_back(road);
			}
			else
			{
				//#if	!FINAL_VERSION
				nlwarning("Road '%s'%s end don't reach a zone at %s", 
					road->getAliasFullName().c_str(),
					road->getAliasString().c_str(),
					road->getLogicStart().toString().c_str());
				//#endif
			}
			// ending zone
			nz = findNpcZone(road->getLogicEnd());
			if	(!nz)
			{
				FOREACH(itCell, vector<CCell*>, neighBourgCellList)
				{
					nz=(*itCell)->findNpcZone(road->getLogicEnd());
					if (nz)
						break;
				}
			}
			if (!nz)
				nz = continent.findNpcZone(road->getLogicEnd());
			
			if	(nz)
			{
				// ok, we found the first zone
				road->setEndZone(nz);
				nz->roads().push_back(road);
			}
			else
			{
				//#ifndef FINAL_VERSION
				nlwarning("Road '%s'%s: end don't reach a zone at %s", 
					road->getAliasFullName().c_str(),
					road->getAliasString().c_str(),
					road->getLogicEnd().toString().c_str());
				//#endif
			}
			
		}
		else
		{
			//#ifndef FINAL_VERSION
			if	(road->coords().size()==1)
			{
				nlwarning("Road '%s'%s: only one point at %s", 
					road->getAliasFullName().c_str(),
					road->getAliasString().c_str(),
					road->getLogicStart().toString().c_str());
			}
			//#endif
		}
		
	}
	
}

//////////////////////////////////////////////////////////////////////////////
// CContinent                                                               //
//////////////////////////////////////////////////////////////////////////////

CContinent::~CContinent()
{
	// first, despawn all groups in familyBehaviors managers
	FOREACH(itRegion, CAliasCont<CRegion>, _Regions)
	{
		FOREACH(itCZ, CAliasCont<CCellZone>, itRegion->cellZones())
		{
			FOREACH(itFB, CCont<CFamilyBehavior>, itCZ->familyBehaviors())
			{
				// unspawn in the npc manager
				CManager *mgrNpc = itFB->mgrNpc();
				mgrNpc->groups().setChildSize(0);
				CManager *mgrFauna = itFB->mgrFauna();
				mgrFauna->groups().setChildSize(0);
			}
		}
	}
}

IAliasCont* CContinent::getAliasCont(TAIType type)
{
	switch(type)
	{
	case AITypeDynamicRegion:
		return &_Regions;
	case AITypeOutpost:
		return &_Outposts;
	default:
		return	NULL;
	}
	
}

std::string CContinent::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":c%u", getChildIndex());
}

std::string	CContinent::getOneLineInfoString() const
{
	return std::string("Continent '") + getName() + "'";
}

std::vector<std::string> CContinent::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CContinent");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " name=" + getName();
	pushEntry(container, "fullname=" + getFullName());
	pushFooter(container);
	
	
	return container;
}

CNpcZone* CContinent::findNpcZone(CAIVector const& posInside)
{
	for (CCont<CRegion>::iterator	itRegion=_Regions.begin(), itEndRegion=_Regions.end(); itRegion!=itEndRegion; ++itRegion)
	{
		for (CCont<CCellZone>::iterator	itCellZone=itRegion->cellZones().begin(), itEndCellZone=itRegion->cellZones().end(); itCellZone!=itEndCellZone; ++itCellZone)
		{
			for (CCont<CCell>::iterator	itCell=itCellZone->cells().begin(), itEndCell=itCellZone->cells().end(); itCell!=itEndCell; ++itCell)
			{
				CNpcZone* npcZone = itCell->findNpcZone(posInside);
				if (npcZone)
					return npcZone;
			}
		}
	}
	// no zone match the position !
	return NULL;
}

bool CContinent::markTagForDelete(NLMISC::TStringId fileId)
{
	CAliasTreeRoot::CMarkTagForDelete const deleteMarker(fileId);
	for_each(_Regions.begin(),_Regions.end(),deleteMarker);
	for_each(_Outposts.begin(),_Outposts.end(),deleteMarker);
	return	true;
}


bool	CContinent::deleteTaggedAlias(NLMISC::TStringId fileId)
{
	for_each(_Regions.begin(),_Regions.end(),	CAliasTreeRoot::CDeleteTagged<CRegion>	(_Regions));
	for_each(_Outposts.begin(),_Outposts.end(),	CAliasTreeRoot::CDeleteTagged<COutpost>	(_Outposts));
	return	true;
}

void	CContinent::updateLazyProcess()
{
	if (_LazyProcess.size()<=0)
		return;

	FOREACH(lazy, TLazyProcessList, _LazyProcess)
		(*lazy)->update();

	_LazyProcess.clear();
}

void	CContinent::pushLazyProcess(CSmartPtr<CLazyProcess>	lazyProcess)
{
	FOREACH(lazy, TLazyProcessList, _LazyProcess)
		if ((*lazy)->absorb(*lazyProcess))
			return;

	_LazyProcess.push_back(lazyProcess);
}

void	CRebuildContinentAndOutPost::update()	const
{
	nlinfo	("Build Continent %s dependencies ..", _Continent->getFullName().c_str());
	// rebuild the bounding box
	_Continent->rebuildBoundingBox();
	// rebuild the connectivity graph

	FOREACH(region,CAliasCont<CRegion>,_Continent->regions())
		region->rebuildConnectivity();
	/*
	// relocate the outpost into the correct cellZone
	FOREACH(outpost, CAliasCont<COutpost>, _Continent->outposts())
	{
		CCellZone	*const	czone = _Continent->getOwner()->locateCellZoneForPos(outpost->getPosition());

		outpost->setCellZone(czone);

		// relink outpost and tribe
		_Continent->relinkOutpost();
	}
	*/
}

void	CContinent::update()
{
	updateLazyProcess();

	{
		H_AUTO(RegionsUpdate)
		// update all the regions
		FOREACH(region,CAliasCont<CRegion>,_Regions)
			(*region)->update();
	}
	{
		H_AUTO(OutpostsUpdate)
		// update all the outpost
		FOREACH(outpost, CAliasCont<COutpost>, _Outposts)
			(*outpost)->update();
	}

}

void CContinent::rebuildBoundingBox()
{
	// build the AABB to speedup the task
	_BoundingBox.init();
	for (uint i=0; i<_Regions.size(); ++i)
	{
		CRegion *region = _Regions[i];
		if (!region)
			continue;

		region->rebuildBoundingBox();

		_BoundingBox.includeAabb(region->_BoundingBox);
	}
}

void	CContinent::serviceEvent	(const	CServiceEvent	&info)
{
	CCont<CRegion>::iterator	itRegion=_Regions.begin(), itRegionEnd=_Regions.end();
	while	(itRegion!=itRegionEnd)
	{
		itRegion->serviceEvent(info);
		++itRegion;
	}
	CCont<COutpost>::iterator	itOutpost=_Outposts.begin(), itOutpostEnd=_Outposts.end();
	while	(itOutpost!=itOutpostEnd)
	{
		itOutpost->serviceEvent(info);
		++itOutpost;
	}
}

bool CContinent::spawn()
{
	// Spawn regions
	for (size_t i=0; i<_Regions.size(); ++i)
	{
		CRegion* region = _Regions[(uint32)i];
		if (!region)
			continue;
		region->spawn();
	}
	// Spawn outposts
	for (size_t i=0; i<_Outposts.size(); ++i)
	{
		COutpost* outpost = _Outposts[(uint32)i];
		if (!outpost)
			continue;
		outpost->spawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

bool CContinent::despawn()
{
	// Despawn regions
	for (size_t i=0; i<_Regions.size(); ++i)
	{
		CRegion* region = _Regions[(uint32)i];
		if (!region)
			continue;
		region->despawn();
	}
	// Despawn outposts
	for (size_t i=0; i<_Outposts.size(); ++i)
	{
		COutpost* outpost = _Outposts[(uint32)i];
		if (!outpost)
			continue;
		outpost->despawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CRegion                                                                  //
//////////////////////////////////////////////////////////////////////////////

CRegion::CRegion(CContinent* owner, uint32 alias, std::string const& name, std::string const& filename)
: CAliasChild<CContinent>(owner, alias, name)
, CAliasTreeRoot(filename)
{
}

CRegion::~CRegion()
{
	_CellZones.clear();

	_GroupFamilies.clear();

	nldebug("Deleting region '%s'%s", getName().c_str(), getAliasString().c_str());
}

IAliasCont* CRegion::getAliasCont(TAIType type)
{
	switch	(type)
	{
	case AITypeCellZone:
		return	&_CellZones;
	case AITypeGroupFamilyProfileFauna:
	case AITypeGroupFamilyProfileTribe:
		return	&_GroupFamilies;
	case AITypeGroupFamilyProfileNpc:
		return	&_GroupFamilies;
	default:
		return	NULL;
	}
	
}

CAliasTreeOwner* CRegion::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	if (!cont)
		return NULL;
	
	CAliasTreeOwner* child = NULL;
	
	switch(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITypeCellZone:
		child = new CCellZone(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	case AITypeGroupFamilyProfileFauna:
	case AITypeGroupFamilyProfileTribe:
		child = new CGroupFamily(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	case AITypeGroupFamilyProfileNpc:
		child = new CGroupFamily(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	default:
		break;
	}
	
	if (child)
		cont->addAliasChild(child);
	return child;
}

std::string CRegion::getIndexString() const
{
	return getOwner()->getIndexString()+toString(":r%u", getChildIndex());
}

std::string	CRegion::getOneLineInfoString() const
{
	return std::string("Region '") + getName() + "'";
}

std::vector<std::string> CRegion::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CRegion");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " alias=" + getAliasString();
	container.back() += " name=" + getName();
	pushEntry(container, "fullname=" + getFullName());
	pushFooter(container);
	
	
	return container;
}

std::string CRegion::getFullName() const
{
	return std::string(getOwner()->getFullName() +":"+ getName());
}

void CRegion::rebuildBoundingBox()
{
	// build the AABB to speedup the task
	_BoundingBox.init();
	for (uint i=0; i<_CellZones.size(); ++i)
	{
		CCellZone *czone = _CellZones[i];
		if (!czone)
			continue;
		
		//		CAIVector vmaxZone(INT_MIN/CAICoord::UNITS_PER_METER, INT_MIN/CAICoord::UNITS_PER_METER);
		//		CAIVector vminZone(INT_MAX/CAICoord::UNITS_PER_METER, INT_MAX/CAICoord::UNITS_PER_METER);
		
		czone->_BoundingBox.init();
		for (uint j=0; j<czone->cells().size(); ++j)
		{
			CCell *cell = czone->cells()[j];
			if (!cell)
				continue;
			cell->_BoundingBox = CAabb(cell->_Coords);
			czone->_BoundingBox.includeAabb(cell->_BoundingBox);
		}
		_BoundingBox.includeAabb(czone->_BoundingBox);
	}
}

void CRegion::rebuildConnectivity()
{
	nlinfo	("Build Region '%s' dependencies ..", getFullName().c_str());

	// prestep : clear the road and cell connectivity
	FOREACH(czone, CAliasCont<CCellZone>, _CellZones)
	{
		czone->rebuildEnergyLevels();
		FOREACH(itCell, CCont<CCell>, czone->cells())
		{
			itCell->_NeighbourCells.clear();
			for (TAliasZonePlaceList::iterator	itZone=itCell->npcZonePlaces().begin(),itEndZone=itCell->npcZonePlaces().end();itZone!=itEndZone;++itZone)
				itZone->roads().clear();			
			for (TAliasZoneShapeList::iterator	itZone=itCell->npcZoneShapes().begin(),itEndZone=itCell->npcZoneShapes().end();itZone!=itEndZone;++itZone)
				itZone->roads().clear();			
		}
		
	}
	
	// First pass : build the connectivity between cells
	const double SCALE_DIAG = 1/10.0;
	for (uint i=0; i<_CellZones.size(); ++i)
	{
		CCellZone *czone = _CellZones[i];
		if (!czone)
			continue;
		for (uint j=0; j<czone->cells().size(); ++j)
		{
			CCell *cell1 = czone->cells()[j];
			if (!cell1)
				continue;
			// maximum dist for aproximation of contact
			double epsilon1 = (cell1->_BoundingBox.vmax() - cell1->_BoundingBox.vmin()).norm()*SCALE_DIAG ;

//			for (uint k=i; k<_CellZones.size(); ++k)
			{
				CCellZone *czone2 = czone;	//_CellZones[k];
				if (!czone2)
					continue;
				for (uint l=0; l<czone2->cells().size(); ++l)
				{
					CCell *cell2 = czone->cells()[l];
					
					if	(	!cell2
						||	cell2 == cell1
						||	cell1->_NeighbourCells.find(cell2) != cell1->_NeighbourCells.end())
						continue;

					if	(cell2->_NeighbourCells.find(cell1) != cell2->_NeighbourCells.end())
					{
						nlassert(false);
						continue;
					}

					double epsilon2 = (cell2->_BoundingBox.vmax() - cell2->_BoundingBox.vmin()).norm()*SCALE_DIAG;
					double epsilon = max(epsilon1, epsilon2);

					// check distance between AABB to reduce vertex checking
					// check on X
					if (cell1->_BoundingBox.vmin().x() < cell2->_BoundingBox.vmin().x())
					{
						// cell 1 on left of cell 2
						if ((cell2->_BoundingBox.vmin().x() - cell1->_BoundingBox.vmax().x()) > epsilon)
							continue;
					}
					else
					{
						// cell 1 on right of cell 2
						if ((cell1->_BoundingBox.vmin().x() - cell2->_BoundingBox.vmax().x()) > epsilon)
							continue;
					}
					// check on Y
					if (cell1->_BoundingBox.vmin().y() < cell2->_BoundingBox.vmin().y())
					{
						// cell 1 on left of cell 2
						if ((cell2->_BoundingBox.vmin().y() - cell1->_BoundingBox.vmax().y()) > epsilon)
							continue;
					}
					else
					{
						// cell 1 on right of cell 2
						if ((cell1->_BoundingBox.vmin().y() - cell2->_BoundingBox.vmax().y()) > epsilon)
							continue;
					}
					// ok, if we are there, the bouding box overlaps or are close enough
					// run through the points of each cell
					{
						// square the espilon to check distance faster
						epsilon = std::min(epsilon, 4.0);
						epsilon = epsilon*epsilon;
						uint32 nbCnx = 0;
						for (uint i=0; i<cell1->_Coords.size(); ++i)
						{
							for (uint j=0; j<cell2->_Coords.size(); ++j)
							{
								if ((cell1->_Coords[i] - cell2->_Coords[j]).sqrnorm() < epsilon)
									nbCnx ++;
							}
						}

						if (nbCnx > 1)
						{
							// ok, the two cells are connected.
							bool ok = cell1->_NeighbourCells.insert(cell2).second;
							ok = cell2->_NeighbourCells.insert(cell1).second;

						//	nldebug("Connecting cell '%s' with '%s'", cell1->getName().c_str(), cell2->getName().c_str());
						}
					}
				}
			}
		}
	}

	// second pass, link road with cells
	
	FOREACH(czone, CAliasCont<CCellZone>, _CellZones)
	{
		FOREACH(itCell, CCont<CCell>, czone->cells())
		{
			itCell->connectRoads();
		}
	}
}

void CRegion::update()
{
	for (uint j=0; j<_CellZones.size(); ++j)
	{
		CCellZone *czone = _CellZones[j];
		if	(!czone)
			continue;

		czone->update();
	}
}

void	CRegion::serviceEvent	(const	CServiceEvent	&info)
{
	CCont<CCellZone>::iterator first(_CellZones.begin()), last(_CellZones.end());
	for(; first != last; ++first)
	{
		(*first)->serviceEvent	(info);
	}
	
}

bool CRegion::spawn()
{
	// Spawn cellzones
	for (size_t j=0; j<_CellZones.size(); ++j)
	{
		CCellZone* cellZone = _CellZones[(uint32)j];
		if (!cellZone)
			continue;
		cellZone->spawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

bool CRegion::despawn()
{
	// Despawn cellzones
	for (size_t j=0; j<_CellZones.size(); ++j)
	{
		CCellZone* cellZone = _CellZones[(uint32)j];
		if (!cellZone)
			continue;
		cellZone->despawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CGroupFamily                                                             //
//////////////////////////////////////////////////////////////////////////////

IAliasCont		*CGroupFamily::getAliasCont(TAIType type)
{
	switch	(type)
	{
	case AITypeGroupTemplate:
	case AITypeGroupTemplateMultiLevel:
	case AITypeGroupTemplateFauna:
		return &_GroupDescs;
	break;
	case AITypeGroupTemplateNpc:
		return &_GroupDescs;
	default:
		return	NULL;
	}

}

CAliasTreeOwner	*CGroupFamily::createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree)
{
	if	(!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;
	
	switch(aliasTree->getType())
	{
	case AITypeGroupTemplate:
	case AITypeGroupTemplateMultiLevel:
	case AITypeGroupTemplateFauna:
	case AITypeGroupTemplateNpc:
		child = new CGroupDesc<CGroupFamily>(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	default:
		break;
	}
	
	if	(child)
		cont->addAliasChild(child);
	return	child;
}


const CGroupDesc<CGroupFamily> *CGroupFamily::getProportionalGroupDesc(const	CFamilyBehavior *const	familyBehavior, const	CPropertySet &needActFlag, const	CPropertySet &maskActFlag)
{
	H_AUTO(getProportionalGroupDesc)
	
	// first, build a list of group that match 
	// the family, energy level and spawn type
	
	//	TODO
	//	const	TPopulationFamily &family	=familyBehavior->getFamily();
	const	uint32	level	=	familyBehavior->getLevelIndex();

	vector<const	CGroupDesc<CGroupFamily>*>	groups;

	const	bool	&isDay	=	CTimeInterface::isDay();
	uint32	totalWeight=0;
	uint32	totalSpawnGroup=0;

	for (uint i=0; i<_GroupDescs.size(); ++i)
	{
		const	CGroupDesc<CGroupFamily> *const	gd = _GroupDescs[i];
		if	(!gd)
			continue;

		const	uint32	weight=gd->getWeightForEnergy	(level);
		if (weight<=0)
			continue;

		if	(!gd->isValidForSeason(CTimeInterface::season()))
			continue;

		if	(!gd->isValidForDayOrNight(isDay))
			continue;

		if	(gd->properties().containsPartOfStrict(maskActFlag))
			continue;

		if	(!gd->properties().containsAllOf(needActFlag))
			continue;
			
		totalWeight		+=	weight;
		totalSpawnGroup	+=	gd->getNbUse();
		groups.push_back(gd);

	}

	if	(groups.empty())
	{
		if	(!LogGroupCreationFailure)
			return	NULL;

		return NULL;
	}

	float	maxScore=0;
	const	CGroupDesc<CGroupFamily>	*choosenGroup=NULL;

	for	(std::vector<const CGroupDesc<CGroupFamily>*>::iterator	it=groups.begin(), itEnd=groups.end(); it!=itEnd;	++it)
	{
		//	score to reach theorical rate.
		const	float	score	=	((*it)->getWeightForEnergy(level)*totalSpawnGroup)
								-	((float)(*it)->getNbUse()*totalWeight);
		if	(score<maxScore)
			continue;
		
		choosenGroup=*it;
		maxScore=score;
	}
	return	choosenGroup;
}


//////////////////////////////////////////////////////////////////////////////
// CGroupDesc                                                               //
//////////////////////////////////////////////////////////////////////////////

template <> size_t const CGroupDesc<CGroupFamily>::_MultiLevelSheetCount = 20;

//////////////////////////////////////////////////////////////////////////////
// CBotDesc                                                                 //
//////////////////////////////////////////////////////////////////////////////

template <> size_t const CBotDesc<CGroupFamily>::_MultiLevelSheetCount = 20;

//////////////////////////////////////////////////////////////////////////////
// CRoad                                                                    //
//////////////////////////////////////////////////////////////////////////////

CRoad::CRoad(CCell *owner, uint32 alias, const std::string &name)
	: CAliasChild<CCell>(owner, alias, name),
	_StartZone(), _StartExternal(false), _EndZone(), _EndExternal(false)
{
//	nldebug("Creating road '%s', alias %u", getName().c_str(), getAlias());
}

CRoad::~CRoad()
{
//	nldebug("Deleting road '%s', alias %u", getName().c_str(), getAlias());

	// need to unlink the road from the start and end zones
	unlinkRoad();
}

void CRoad::unlinkRoad()
{
	// unlink the road from the start and end zones
	if (!_StartZone.isNULL())
	{
		std::vector<NLMISC::CDbgPtr<CRoad> > &roads = _StartZone->roads();

		roads.erase(std::remove(roads.begin(), roads.end(), CDbgPtr<CRoad>(this)), roads.end());
	}
	_StartZone = (CNpcZone*)NULL;
	if (!_EndZone.isNULL())
	{
		std::vector<NLMISC::CDbgPtr<CRoad> > &roads = _EndZone->roads();

		roads.erase(std::remove(roads.begin(), roads.end(), CDbgPtr<CRoad>(this)), roads.end());
	}
	_EndZone = (CNpcZone*)NULL;
}

IAliasCont		*CRoad::getAliasCont(TAIType type)
{
	switch(type)
	{
	case AITypeRoadTrigger:
		return &_RoadTriggers;
	default:
		return	NULL;
	}
}
CAliasTreeOwner	*CRoad::createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;

	switch(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITypeRoadTrigger:
		child = new CRoadTrigger(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	}

	if (child)
		cont->addAliasChild(child);
	return	child;
}

std::string CRoad::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":ro%u", getChildIndex());
}


void	CRoad::setPathPoints(TVerticalPos	verticalPos, const std::vector<CAIVector> &points)
{
	_VerticalPos = verticalPos;
	for (uint i=0; i<points.size(); ++i)
	{
		RYAI_MAP_CRUNCH::CWorldPosition newpos;
		CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, newpos, points[i], 0, 1, CWorldContainer::CPosValidatorDefault());
		if (!newpos.isValid())
		{
			CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, newpos, points[i], 6, 100, CWorldContainer::CPosValidatorDefault());
			if (newpos.isValid())
			{
				if (LogAcceptablePos)
					nlinfo("DynRoad '%s'%s: Path pos Error at position %s for size %d, an acceptable position could be %s",
						getAliasFullName().c_str(),
						getAliasString().c_str(),
						points[i].toString().c_str(), 
						0, 
						newpos.toString().c_str() );
			}
			else
				nlwarning("Dynroad '%s'%s: Path pos Error at position %s for size %d, no acceptable position found around", 
					getAliasFullName().c_str(),
					getAliasString().c_str(),
					points[i].toString().c_str(),
					0);
		}
	 	_Coords.push_back(newpos);
	}

	if (points.size()>0)
		_Start=points.front();

	if (points.size()>1)
		_End=points.back();
}

void	CRoad::setStartZone(const	CNpcZone	*const	npcZone)
{
	_StartZone=(CNpcZone*)npcZone;
	if (!npcZone)
	{
		_StartExternal=false;
	}
	else
	{
		_StartExternal=npcZone->getOwner()->getOwner()->getOwner()!=getOwner()->getOwner()->getOwner();
	}

}
void	CRoad::setEndZone(const	CNpcZone	*const	npcZone)
{
	_EndZone=npcZone;
	if (!npcZone)
	{
		_EndExternal=false;
	}
	else
	{
		_EndExternal=npcZone->getOwner()->getOwner()->getOwner()!=getOwner()->getOwner()->getOwner();
	}

}

//////////////////////////////////////////////////////////////////////////////
// CCellZone                                                                //
//////////////////////////////////////////////////////////////////////////////

struct CCellChoice
{
	struct	CZoneScore
	{
		float		score;
		const	CFaunaZone*	zone;
	};
	CCellChoice()
	{
		for (uint32 i=0;i<MAX_ZONE_SCORE;i++)
		{
			zones[i].score=0;
			zones[i].zone=NULL;
		}
	}

	float	getTotalScore()	const
	{
		float	totalScore=1;
		for (uint32 i=0;i<MAX_ZONE_SCORE;i++)
			totalScore*=zones[i].score;
		return	totalScore;
	}

	enum	TZoneScoreType
	{
		FOOD_ZONE_SCORE=0,
		REST_ZONE_SCORE=1,
		MAX_ZONE_SCORE=2
	};
	CZoneScore	zones[MAX_ZONE_SCORE];
};

CCellZone::CCellZone(CRegion *owner, uint32 alias, const std::string &name)
: CAliasChild<CRegion>(owner, alias, name)
{
	nldebug("Creating cell zone '%s'%s", 
		getName().c_str(), 
		getAliasString().c_str());
}

CCellZone::~CCellZone()
{
	// clear the families first to despawn all the groups (managers are in the families)
	_Families.clear();
	_Cells.clear();
	nldebug("Deleting cell zone '%s'%s", 
		getName().c_str(), 
		getAliasString().c_str());
}


std::string	CCellZone::getOneLineInfoString() const
{
	return std::string("Cell zone '") + getName() + "'";
}

std::vector<std::string> CCellZone::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CCellZone");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " name=" + getName();
	pushEntry(container, "fullname=" + getFullName());
	pushFooter(container);
	
	
	return container;
}
std::string CCellZone::getFullName() const
{
	return std::string(getOwner()->getFullName()+":"+getName());
}

IAliasCont* CCellZone::getAliasCont(TAIType type)
{
	switch(type)
	{
	case AITypeCell:
		return &_Cells;
	default:
		return	NULL;
	}
	
}

CAliasTreeOwner	*CCellZone::createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree)
{
	if	(!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;
	
	switch	(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITypeCell:
		child = new CCell(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	}
	
	if (child)
		cont->addAliasChild(child);
	return	child;
}

bool CCellZone::findRestAndFoodFaunaZoneInCellList(CFaunaZone const*& rest, CPropertySet const& restActivity, CFaunaZone const*& food, CPropertySet const& foodActivity, std::vector<CCell*> const& cells, TAStarFlag const denyflags)
{
	static bool extensiveDebug = false; // That extensive debug has code 0001
	if (extensiveDebug) nldebug("ED0001.01: restActivity=%s foodActivity=%s", restActivity.toString().c_str(), foodActivity.toString().c_str());
	
	// Flags topology
	typedef CHashMap<uint,CCellChoice> TSearchMapCellChoice;
	typedef CHashMap<uint,TSearchMapCellChoice > TSearchMap;
	TSearchMap	searchMap;
	const	float	minimumScore=/*28*28*/24*24; // :FIXME: Put that in a config thing (like a config file)
	
	// Property sets for all activities
	CPropertySet activities[CCellChoice::MAX_ZONE_SCORE];
	activities[CCellChoice::FOOD_ZONE_SCORE].merge(foodActivity);
	activities[CCellChoice::REST_ZONE_SCORE].merge(restActivity);
	
	// Look for a conveninent zone in a convenient cell.
	if (extensiveDebug) nldebug("ED0001.02: cells.size()=%d", cells.size());
	// For each cell
	FOREACHC(itCell, std::vector<CCell*>, cells)
	{
		CCell const* const cell = *itCell;
		if	(!cell)
			continue;
		
		if (extensiveDebug) nldebug("ED0001.03: cell->faunaZonesCst().size()=%d", cell->faunaZonesCst().size());
		// For each zone
		FOREACHC(itFaunaZone, CCont<CFaunaZone>, cell->faunaZonesCst())
		{
			CFaunaZone const* const faunaZone = *itFaunaZone;
			#ifdef NL_DEBUG
			nlassert(faunaZone);
			#endif
			// For each activity
			for (uint32	typeZone=0; typeZone<CCellChoice::MAX_ZONE_SCORE; ++typeZone)
			{			
				// Get zone position
				CWorldPosition const& wpos = faunaZone->worldValidPos();
				// Check it's valid
				if	(!wpos.isValid())
				{
					if (extensiveDebug) nldebug("ED0001.04: !wpos.isValid()");
					continue;
				}
				// tmp nico for debug
				/*
				static bool displayActivities = false;
				if (displayActivities)
				{
					nlinfo("Zone activities");
					const AITYPES::CPropertySet &ps = faunaZone->additionalActivities();
					const std::set<NLMISC::TStringId> &props = ps.properties();
					std::set<NLMISC::TStringId>::const_iterator it;
					for (it = props.begin(); it != props.end(); ++it)
					{
						nlinfo("Prop = %s", NLMISC::CStringMapper::unmap(*it).c_str());
					}
					nlinfo("Wanted activities");
					for (it = activities[typeZone].properties().begin(); it != activities[typeZone].properties().end(); ++it)
					{
						nlinfo("Prop = %s", NLMISC::CStringMapper::unmap(*it).c_str());
					}
				}
				*/
				// Check that zone activity match
				if	(!faunaZone->haveActivity(activities[typeZone]))
				{
					if (extensiveDebug) nldebug("ED0001.05: !faunaZone->haveActivity(activities[typeZone])");
					continue;
				}
				
				TAStarFlag const flags = (TAStarFlag)(wpos.getFlags()&GroundFlags);	//	Erase unused flags.
				float const score = faunaZone->getFreeAreaScore();
				
				for	(TAStarFlag	possibleFlag=Nothing;possibleFlag<=GroundFlags;possibleFlag=(TAStarFlag)(possibleFlag+2))	//	tricky !! -> to replace with a defined list of flags to checks.
				{
					const	uint32	incompatibilityFlags=possibleFlag&denyflags&GroundFlags;	//	Erase unused flags.
					if	(incompatibilityFlags)
					{
						if (extensiveDebug) nldebug("ED0001.06: incompatibilityFlags");
						continue;
					}
					
					const	uint32	masterTopo=wpos.getTopologyRef().getCstTopologyNode().getMasterTopo(possibleFlag);
					if	(masterTopo==~0)
					{
						if (extensiveDebug) nldebug("ED0001.07: masterTopo==~0");
						continue;
					}
					
					if	(score<minimumScore)
					{
						if (extensiveDebug) nldebug("ED0001.08: score<minimumScore");
						continue;
					}
					
					CCellChoice::CZoneScore	&zoneScore=searchMap[possibleFlag][masterTopo].zones[typeZone];
					
					if	(score<zoneScore.score)
					{
						if (extensiveDebug) nldebug("ED0001.09: score<zoneScore.score");
						continue;
					}
					
					if	(score==zoneScore.score)
					{
						if (extensiveDebug) nldebug("ED0001.10: score==zoneScore.score");
						if	(CAIS::rand16(1)==0)
							zoneScore.zone=faunaZone;
					}
					else
					{
						if (extensiveDebug) nldebug("ED0001.11: score!=zoneScore.score");
						zoneScore.score=score;
						zoneScore.zone=faunaZone;
					}
				}
			}
		}
	}
	
	//	find now the best score :)
	{
		CCellChoice	*selectedCell=NULL;
		float	minScore=minimumScore;
		for	(TSearchMap::iterator	flagIt=searchMap.begin(), flagItEnd=searchMap.end();flagIt!=flagItEnd;++flagIt)
		{
			for	(TSearchMapCellChoice::iterator	topoIt=flagIt->second.begin(), topoItEnd=flagIt->second.end();topoIt!=topoItEnd;++topoIt)
			{
				if (extensiveDebug) nldebug("ED0001.12: topoIt->second.getTotalScore()=%g minScore=%g", topoIt->second.getTotalScore(), minScore);
				float theScore = topoIt->second.getTotalScore();
				if	(theScore > minScore)
				{
					selectedCell=&topoIt->second;
					minScore=selectedCell->getTotalScore();
				}
			}
		}

		if (selectedCell)
		{
			rest=selectedCell->zones[CCellChoice::REST_ZONE_SCORE].zone;
			food=selectedCell->zones[CCellChoice::FOOD_ZONE_SCORE].zone;

#if !FINAL_VERSION
			const	RYAI_MAP_CRUNCH::TAStarFlag	restFlags=rest->worldValidPos().getTopologyRef().getCstTopologyNode().getFlags();
			const	RYAI_MAP_CRUNCH::TAStarFlag	foodFlags=food->worldValidPos().getTopologyRef().getCstTopologyNode().getFlags();
			nlassert((restFlags&denyflags)==0);
			nlassert((foodFlags&denyflags)==0);
#endif
			return	true;
		}
	}
	return	false;
}


const	CFaunaZone	*CCellZone::lookupFaunaZone(const	CPropertySet &activity,	TAStarFlag	denyflags, size_t replacementGroupFamilyId)
{
	float	totalScore=28*28;

	vector<const CFaunaZone*>	candidates;
	
	//	prepare a randomly ordered list of cell.
	vector<CCell*>	cells;
	for	(CCont<CCell>::iterator	it=_Cells.begin(), itEnd=_Cells.end();it!=itEnd;++it)
		cells.push_back(*it);
	random_shuffle(cells.begin(), cells.end());
	

	// look for a conveninent zone in a convenient cell.
	for (uint c = 0; c < cells.size(); ++c)
	{
		const	CCell *const	cell = cells[c];
		
		if	(!cell)
			continue;

		// look for a zone that support activity.
		for	(CCont<CFaunaZone>::const_iterator	it=cell->faunaZonesCst().begin(), itEnd=cell->faunaZonesCst().end(); it!=itEnd;++it)
		{
			const	CFaunaZone	*const	faunaZone=*it;
#ifdef NL_DEBUG
			nlassert(faunaZone);
#endif
			// tmp nico for debug
			/*
			static bool displayActivities = false;
			if (displayActivities)
			{
				nlinfo("Zone activities");
				const AITYPES::CPropertySet &ps = faunaZone->additionalActivities();
				const std::set<NLMISC::TStringId> &props = ps.properties();
				std::set<NLMISC::TStringId>::const_iterator it;
				for (it = props.begin(); it != props.end(); ++it)
				{
					nlinfo("Prop = %s", NLMISC::CStringMapper::unmap(*it).c_str());
				}
				nlinfo("Wanted activities num = %d ", (int) activity.properties().size());
				if (activity.properties().size() != 0)
				{
					for (it = activity.properties().begin(); it != activity.properties().end(); ++it)
					{
						nlinfo("Prop = %s", NLMISC::CStringMapper::unmap(*it).c_str());
					}
				}
			}
			*/

			if	(	!faunaZone->worldValidPos().isValid()
				||	!faunaZone->haveActivity(activity)	)
				continue;

			// if a replacement GroupFamily was asked, it must be in the zone
			if (replacementGroupFamilyId != 0)
			{
				if (!faunaZone->isSubstituted()) continue;
				if (!faunaZone->isSubsitutedForGroupFamily(replacementGroupFamilyId)) continue;
			}
			else
			{
				if (faunaZone->isSubstituted()) continue; // zone wants a substitution group
			}

			const	RYAI_MAP_CRUNCH::TAStarFlag	flags=faunaZone->worldValidPos().getTopologyRef().getCstTopologyNode().getFlags();
			if	(flags&denyflags)
				continue;

			const	float	score=faunaZone->getFreeAreaScore();
			if	(score>=totalScore)
			{
				if	(score==totalScore)
				{
					candidates.push_back(faunaZone);
				}
				else
				{
					candidates.clear();
					candidates.push_back(faunaZone);
					totalScore=score;
				}
			}
		}
	}
	if	(candidates.size()>0)
		return	candidates[CAIS::rand16((uint32)candidates.size())];
	return NULL;
}

struct TLookupLogFilter
{
	CPropertySet	Properties;
	uint32			Alias;

	bool operator ==(const TLookupLogFilter &other) const
	{
		if (Alias != other.Alias)
			return false;

		return Properties == other.Properties;

	}

	bool operator <(const TLookupLogFilter &other) const
	{
		if (Alias != other.Alias)
			return Alias < other.Alias;

		return Properties < other.Properties;
	}
};

const CNpcZone	*CCellZone::lookupNpcZone(const	CPropertySet &activity, size_t replacementGroupFamilyId)
{
	float	totalScore=0;

	//	prepare a randomly ordered list of cell.
	vector<CCell*>	cells;
	for	(CCont<CCell>::iterator	it=_Cells.begin(), itEnd=_Cells.end();it!=itEnd;++it)
		cells.push_back(*it);
	random_shuffle(cells.begin(), cells.end());
	
	vector<const CNpcZone*>	candidates;

	// look for a convenient zone in a convenient cell.
	for (uint c=0; c < cells.size(); ++c)
	{
		const	CCell *const	cell = cells[c];

		if	(!cell)
			continue;


		FOREACHC(it, TAliasZonePlaceList, cell->npcZonePlacesCst())
		{
			const CNpcZone	*npcZone=*it;
#ifdef NL_DEBUG
			nlassert(npcZone);
#endif
			if	(	!activity.empty()
				&&	!npcZone->properties().containsPartOfNotStrict(activity))
				continue;

			// if a replacement GroupFamily was asked, it must be in the zone
			if (replacementGroupFamilyId != 0)
			{
				if (!npcZone->isSubstituted()) continue;
				if (!npcZone->isSubsitutedForGroupFamily(replacementGroupFamilyId)) continue;
			}
			else
			{
				if (npcZone->isSubstituted()) continue; // zone wants a substitution group
			}

			const	float	score=npcZone->getFreeAreaScore();
			if	(score>totalScore)
			{
				totalScore=score;
				candidates.clear();
				candidates.push_back(npcZone);
				continue;
			}

			if	(score==totalScore)
				candidates.push_back(npcZone);
		}
		FOREACHC(it, TAliasZoneShapeList, cell->npcZoneShapesCst())
		{
			// :FIXME: Same code than above, cut n paste
			const	CNpcZone	*const	npcZone=*it;
#ifdef NL_DEBUG
			nlassert(npcZone);
#endif
			if	(	!activity.empty()
				&&	!npcZone->properties().containsPartOfNotStrict(activity))
				continue;

			const	float	score=npcZone->getFreeAreaScore();
			if	(score>totalScore)
			{
				totalScore=score;
				candidates.clear();
				candidates.push_back(npcZone);
				continue;
			}

			if	(score==totalScore)
				candidates.push_back(npcZone);
		}

	}
				
	if	(candidates.size()>0)
		return	candidates[CAIS::rand16((uint32)candidates.size())];

	// warning only once
	{
		static set<TLookupLogFilter> filter;

		TLookupLogFilter lf;
		lf.Properties = activity;
		lf.Alias = getAlias();

		if (filter.find(lf) == filter.end())
		{
			nlwarning("CCellZone::lookupNpcZone can't find zone for activity '%s' in cellZone '%s'%s (display only ONCE)",
				activity.toString().c_str(),
				getAliasFullName().c_str(),
				getAliasString().c_str());

			filter.insert(lf);
		}
	}
	return NULL;
}




const CNpcZone	*CCellZone::lookupNpcZoneByName(/*const TPopulationFamily &family, */const std::string &zoneName)
{
	for	(uint i=0; i<_Cells.size(); ++i)
	{
		CCell *const	cell = _Cells[i];
		if	(!cell)
			continue;

//		if	(!cell->_FamilyFlags.containsFamily(family))
//			continue;

		for (uint j=0; j<cell->npcZoneCount(); ++j)
		{
			CNpcZone *const		zone = cell->npcZone(j);
			if	(!zone)
				continue;

			if	(zone->getAliasTreeOwner().getName() == zoneName)
			{	// ok, we found it !
				return zone;
			}

		}
		
	}
	return	NULL;
}

std::string CCellZone::getIndexString() const
{
	return getOwner()->getIndexString()+toString(":cz%u", getChildIndex());
}


void	CCellZone::rebuildEnergyLevels()
{
	CCont<CGroupFamily>	&groupFamilies=getOwner()->groupFamilies();

	{
		CPropertySet	groupFamiliesNames;
		
		for (CCont<CGroupFamily>::iterator	it=groupFamilies.begin(), itEnd=groupFamilies.end();it!=itEnd;++it)
			groupFamiliesNames.addProperty(it->getName());
		
		for	(CCont<CFamilyBehavior>::iterator	it=_Families.begin(), itEnd=_Families.end();it!=itEnd;)
		{	
			if (!groupFamiliesNames.have(it->getName()))
			{
				CCont<CFamilyBehavior>::iterator	last=it;
				++it;
				_Families.removeChildByIndex(last->getChildIndex());
				continue;
			}
			++it;
		}

	}

	
	for (CCont<CGroupFamily>::iterator	it=groupFamilies.begin(), itEnd=groupFamilies.end();it!=itEnd;++it)
	{
		CCont<CFamilyBehavior>::iterator	itBehav=_Families.begin(),	itBehavEnd=_Families.end();
		for	(;itBehav!=itBehavEnd;++itBehav)
		{	
			if	(itBehav->getName()==it->getName())
				break;
		}
		if	(itBehav!=itBehavEnd)	//	present ?
			continue;

		//	else create the missing Behavior.
		CSmartPtr<CFamilyBehavior>	fb=new CFamilyBehavior(this, *it);
		if	(!fb->isFamilyProfileValid())
		{
			fb=NULL;
			continue;
		}
		
		fb->setBaseLevel		((uint32)(0.45*(double)ENERGY_SCALE));	//	forced to 0 at start instead of 0.5 ..
		fb->setEffectiveLevel	(fb->baseLevel());
		
		_Families.addChild		(fb);

	}
		
}

// predicate to remove old family manager
class CObsoleteFamilyManagerRemover
{
public:
	bool operator()(CFamilyBehavior *fb) const
	{
		nlassert(fb);

		if (fb->grpFamily()->getSubstitutionId() != 0) // do the state only dynamically added groups
		{
			// if all managers are empty, then can remove
			if (fb->mgrNpc()->isEmpty() && fb->mgrFauna()->isEmpty())
			{
				return true;
			}
		}
		return false;
	}
};

void	CCellZone::update()
{
	H_AUTO(CellZoneUpdate);

	// update all family managers

	for	(CCont<CFamilyBehavior>::iterator	it=_Families.begin(), itEnd=_Families.end(); it!=itEnd;++it)
	{

		{
			H_AUTO(CellZonesManagersUpdate);
			it->updateManagers();
		}
		{
			H_AUTO(FamiliesUpdate);
			if	(it->needUpdate())
				it->update(it->getDt());
		}
	}

	// remove old substitution family manager if they are not referenced any more
	_Families.getInternalCont().erase(std::remove_if(_Families.getInternalCont().begin(), _Families.getInternalCont().end(), CObsoleteFamilyManagerRemover()), _Families.getInternalCont().end());
}

void CCellZone::serviceEvent	(const	CServiceEvent	&info)
{
	CCont<CFamilyBehavior>::iterator first(_Families.begin()), last(_Families.end());
	for(; first != last; ++first)
	{
		(*first)->serviceEvent	(info);
	}
}

const	CNpcZone	*CCellZone::lookupNpcZoneScorer	(std::vector<CCell*>	cells,	const	CZoneScorer		&scorer)
{
	float	totalScore=0;
	
	vector<const CNpcZone*>	candidates;
	
	// look for a convenient zone in a convenient cell.
	for (uint c=0; c < cells.size(); ++c)
	{
		const	CCell *const	cell = cells[c];
		
		if	(!cell)
			continue;
		
		FOREACHC(it, TAliasZonePlaceList, cell->npcZonePlacesCst())
		{
			const	CNpcZone	*const	npcZone=*it;
			
#ifdef NL_DEBUG
			nlassert(npcZone);
#endif			
			const	float	score=scorer.getScore(*npcZone);
			float const distance = scorer.getParam(*npcZone);
			if (LogScorerScores && score>=0.f)
				nldebug("Zone: %s - Score: %f - Distance: %f", npcZone->getAliasTreeOwner().getAliasFullName().c_str(), score, distance);
			if	(score<totalScore)
				continue;
			
			if	(score>totalScore)
			{
				totalScore=score;
				candidates.clear();
				candidates.push_back(npcZone);
				continue;
			}
			
			candidates.push_back(npcZone);
		}
		FOREACHC(it, TAliasZoneShapeList, cell->npcZoneShapesCst())
		{
			// :FIXME: Same code than above, cut n paste
			const	CNpcZone	*const	npcZone=*it;
			
#ifdef NL_DEBUG
			nlassert(npcZone);
#endif			
			const	float	score=scorer.getScore(*npcZone);
			float const distance = scorer.getParam(*npcZone);
			if (LogScorerScores && score>=0.f)
				nldebug("Zone: %s - Score: %f - Distance: %f", npcZone->getAliasTreeOwner().getAliasFullName().c_str(), score, distance);
			if	(score<totalScore)
				continue;
			
			if	(score>totalScore)
			{
				totalScore=score;
				candidates.clear();
				candidates.push_back(npcZone);
				continue;
			}			
			candidates.push_back(npcZone);
		}
		
	}
	if	(candidates.size()>0)
		return	candidates[CAIS::rand16((uint32)candidates.size())];
	return NULL;
}

bool CCellZone::spawn()
{
	// Spawn families
	for (size_t k=0; k<_Families.size(); ++k)
	{
		CFamilyBehavior* familyBehavior = _Families[(uint32)k];
		if (!familyBehavior)
			continue;
		familyBehavior->spawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

bool CCellZone::despawn()
{
	// Despawn families
	for (size_t k=0; k<_Families.size(); ++k)
	{
		CFamilyBehavior* familyBehavior = _Families[(uint32)k];
		if (!familyBehavior)
			continue;
		familyBehavior->despawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CCell                                                                    //
//////////////////////////////////////////////////////////////////////////////

CCell::CCell(CCellZone *owner, uint32 alias, const std::string &name)
	: CAliasChild<CCellZone>(owner, alias, name)
{
}


CCell::~CCell()
{ 
	unlinkCell();
}

std::string CCell::getFullName() const
{
	return std::string(getOwner()->getFullName()+":"+getName());
}

void CCell::unlinkCell()
{
	while (!_NeighbourCells.empty())
	{
		CCell *c = *(_NeighbourCells.begin());
		c->_NeighbourCells.erase(this);
		_NeighbourCells.erase(_NeighbourCells.begin());
	}
}

IAliasCont		*CCell::getAliasCont(TAIType type)
{
	switch(type)
	{
	case AITypeDynFaunaZone:
		return &_FaunaZones;
	case AITypeDynNpcZonePlace:
		return &_NpcZonePlaces;
	case AITypeDynNpcZoneShape:
		return &_NpcZoneShapes;
	case AITypeDynRoad:
		return	&_Roads;
	default:
		return	NULL;
	}

}

CNpcZone	*CCell::findNpcZone(const CAIVector &posInside)
{
//	for (CCont<CNpcZone>::iterator	itNpcZone=npcZones().begin(), itEndNpcZone=npcZones().end(); itNpcZone!=itEndNpcZone; ++itNpcZone)
	FOREACH(itNpcZone, TAliasZonePlaceList, npcZonePlaces())
	{
		if	(!itNpcZone->atPlace(posInside))
			continue;		
		// This zone match the position
		return	*itNpcZone;
	}
	FOREACH(itNpcZone, TAliasZoneShapeList, npcZoneShapes())
	{
		// :FIXME: Same code than above, cut n paste
		if	(!itNpcZone->atPlace(posInside))
			continue;		
		// This zone match the position
		return	*itNpcZone;
	}
	return	NULL;
}


CAliasTreeOwner	*CCell::createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;

	switch(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITypeDynFaunaZone:
	{
		CFaunaZone *fzc = new CFaunaZone(this, aliasTree);
		child = fzc;
	}
	break;
	case AITypeDynNpcZonePlace:
		child = new CNpcZonePlace(this, aliasTree);
		break;
	case AITypeDynNpcZoneShape:
		child = new CNpcZoneShape(this, aliasTree);
		break;
	case AITypeDynRoad:
		child = new CRoad(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	}

	if (child)
		cont->addAliasChild(child);
	return	child;
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// data structure for path finding
//****************************************
class	TListItem
{
public:
	//	only for search purpose.
	TListItem(const	CNpcZone	*const	zone)
		: _Zone(zone)
		,_MvtCost(0)
		,_TargetDist(0)
		,_Value(0)
		,_Parent(NULL)
		,_Road(NULL)
	{}

	TListItem	(const	CNpcZone	*const	zone,	const	float	&mvtCost, const	float	&targetDist, const	float	&value, const	CNpcZone	*const	parent, const	CRoad	*const	road)
		:_Zone(zone)
		,_MvtCost(mvtCost)
		,_TargetDist(targetDist)
		,_Value(value)
		,_Parent(parent)
		,_Road(road)
	{}

	const	float	&value	()	const
	{
		return	_Value;
	}

	const	CNpcZone	*zone	()	const
	{
		return	_Zone;
	}

	const	CNpcZone	*parent	()	const
	{
		return	_Parent;
	}

	const	CRoad		*road	()	const
	{
		return	_Road;
	}

	const	float	&mvtCost	()	const
	{
		return	_MvtCost;
	}

private:
	const	CNpcZone	*const	_Zone;
	const	CNpcZone	*const	_Parent;
	const	CRoad		*const	_Road;
	float	_MvtCost;
	float	_TargetDist;
	float	_Value;
};

struct	TOrderItemOnValue	:	unary_function<TListItem, bool>
{
	bool operator () (const TListItem &item1, const TListItem &item2) const
	{
		return item1.value() < item2.value();
	}

};

struct TOrderItemOnZone : unary_function<TListItem, bool>
{
	bool operator () (const TListItem &item1, const TListItem &item2) const
	{
		return item1.zone()< item2.zone();
	}

};

class	TFindItemOnZone
{
public:
	TFindItemOnZone(const	CNpcZone *zone)
		: _Zone(zone)
	{}
	bool operator () (const TListItem &item) const
	{
		return item.zone()== _Zone;
	}
private:
	const	CNpcZone	*_Zone;
};

bool	pathFind(const	CNpcZone	*const	start, const	CNpcZone	*const	end, const	CPropertySet	&zoneFilter, std::vector<NLMISC::CSmartPtr<CRoad> > &path, bool logError)
{
	/// check that the start and end are in the same continent.
	const	CContinent	*cont = start->getOwner()->getOwner()->getOwner()->getOwner();
	if (end->getOwner()->getOwner()->getOwner()->getOwner() != cont)
	{
		nlwarning("Searching path for zone in different continent !");
		return	false;
	}

#if	!FINAL_VERSION
	if (start==end)
	{
		nlwarning("trying to find a path between same zone %s", start->getAliasTreeOwner().getAliasFullName().c_str());
		path.clear();	//	->leads to an assert in the caller ?
		return	true;
	}
#endif


	multiset<TListItem, TOrderItemOnValue>	openList;
	set<TListItem, TOrderItemOnZone>	closedList;
	
	const	float	localDist=(float)(end->midPos() - start->midPos()).norm();
	TListItem	li(start, 0, localDist, localDist, NULL, NULL);
	openList.insert(li);

	do
	{
		const TListItem bestItem = *openList.begin();
		openList.erase(openList.begin());
		closedList.insert(bestItem);
		nlassert(closedList.find(bestItem) != closedList.end());
	
		// check if we are at destination
		if (bestItem.zone()== end)
		{
			// yeee!  we found the path! build the resulting path
			const TListItem *pli = &bestItem;
			while	(	pli->parent()
					&&	pli->road())
			{
				path.push_back(const_cast<CRoad*>(pli->road()));
				set<TListItem, TOrderItemOnZone>::iterator it(closedList.find(pli->parent()));
				if (it != closedList.end())
					pli = &(*it);
				else
				{
					// look in the open list
					multiset<TListItem, TOrderItemOnValue>::iterator it(find_if(openList.begin(), openList.end(), TFindItemOnZone(pli->parent())));
					nlassert(it != openList.end());
					pli = &(*it);
				}

			}
			// make the path in correct order
			std::reverse(path.begin(), path.end());
			return	true;
		}


		CNpcZone*const	z1 = const_cast<CNpcZone*>(bestItem.zone());

		FOREACH(road, vector<CDbgPtr<CRoad> >, z1->roads())
		{
			CNpcZone *z2 = (*road)->startZone();

			if	(z2 == z1)
			{
				z2 = (*road)->endZone();
				if	(z2==z1)
					continue;
			}

			if	(	!z2
				||	closedList.find(TListItem(z2)) != closedList.end()
				||	(	z2!=end
					&&	z2->properties().containsPartOfStrict(zoneFilter)))
			{
				continue;
			}

			const	float	localDist=(float)(end->midPos() - z2->midPos()).norm();

			//	_Length in meters
			//	_Difficulty between 0 and ??
			TListItem	li(z2,	bestItem.mvtCost()+(*road)->getCost(),	localDist, localDist+li.mvtCost(), z1, *road);

			// first, check if the zone is already in the open list
			multiset<TListItem, TOrderItemOnValue>::iterator it(find_if(openList.begin(), openList.end(), TFindItemOnZone(z2)));

			if (it == openList.end())
			{
				// If it isn't on the open list, add it to the open list. 
				// Make the current square the parent of this square. 
				// Record the F, G, and H costs of the square. 

				openList.insert(li);
			}
			else
			{
				// If it is on the open list already, check to see if this path 
				// to that square is better, using G cost as the measure. A lower 
				// G cost means that this is a better path. If so, change the parent 
				// of the square to the current square, and recalculate the G and F 
				// scores of the square. If you are keeping your open list sorted by F 
				// score, you may need to resort the list to account for the change.

				if (li.mvtCost() < it->mvtCost())
				{
					openList.erase(it);
					openList.insert(li);
				}

			}

		}

	} while(!openList.empty());

	if (logError)
		nlwarning("Could't find path from '%s'%s to '%s'%s", 
			start->getAliasTreeOwner().getAliasFullName().c_str(),	
			start->getAliasTreeOwner().getAliasString().c_str(),
			end->getAliasTreeOwner().getAliasFullName().c_str(),	
			end->getAliasTreeOwner().getAliasString().c_str());
	return	false;
}


//////////////////////////////////////////////////////////////////////////////
// CPopulation                                                              //
//////////////////////////////////////////////////////////////////////////////

std::string CPopulation::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":po%u", getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// CRoadTrigger                                                             //
//////////////////////////////////////////////////////////////////////////////

std::string CRoadTrigger::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":rt%u", getChildIndex());
}



CGroupFamily::CGroupFamily(CAliasTreeOwner *owner, uint32 alias, std::string const& name)
: CAliasTreeOwner(alias, name),
  _Owner(owner),
  _ProfileName(0),
  _SubstitutionId(0),
  _Index(~0)
{
}


bool CAIRefPlaceXYR::atPlace(CAIEntityPhysical const* entity) const
{
	return _Zone->atPlace(entity);
}
