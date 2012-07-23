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

#ifndef CONTINENT_INLINE_H
#define CONTINENT_INLINE_H

#include "ai_bot.h"
#include "ai_grp_fauna.h"
#include "ai_grp_npc.h"
#include "ai_bot_npc.h"
#include "ais_actions.h"

extern NLMISC::CVariable<bool>	LogAcceptablePos;
extern NLMISC::CVariable<bool>	LogGroupCreationFailure;
extern NLMISC::CVariable<bool>	LogOutpostDebug;

//////////////////////////////////////////////////////////////////////////////
// CPopulationRecord                                                        //
//////////////////////////////////////////////////////////////////////////////

inline
CPopulationRecord::CPopulationRecord(AISHEETS::ICreatureCPtr const& sheetData, uint count)
: _SheetData(sheetData)
, _Count(count)
{
}

inline
bool CPopulationRecord::operator==(CPopulationRecord const& other) const
{
	return (_Count==other._Count && _SheetData==other._SheetData);
}

inline
AISHEETS::ICreatureCPtr CPopulationRecord::getCreatureSheet() const
{
	return _SheetData;
}

inline
uint CPopulationRecord::getBotCount(bool useCreatureMultiplier) const
{
	if (useCreatureMultiplier && _SheetData)
		return _Count * _SheetData->DynamicGroupCountMultiplier();
	else
		return _Count;
}

inline
uint32 CPopulationRecord::getEnergyValue(bool useCreatureMultiplier) const
{
	if (_SheetData)
	{
		return	getBotCount(useCreatureMultiplier)*_SheetData->EnergyValue();
	}
	else
	{
#ifdef NL_DEBUG
		nlassert(_SheetData);
#endif
		return	0;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CPopulation                                                              //
//////////////////////////////////////////////////////////////////////////////

inline
CPopulation::CPopulation(CPopulationOwner* owner, CAIAliasDescriptionNode* aliasDescription)
	: CAliasChild<CPopulationOwner>(owner, aliasDescription)
	, _Weight(0)
	, _SpawnType(AITYPES::SpawnTypeBadType)
{
}

inline
CPopulation::CPopulation(CPopulationOwner* owner, uint32 alias, std::string name)
	: CAliasChild<CPopulationOwner>(owner, alias, name)
	, _Weight(0)
	, _SpawnType(AITYPES::SpawnTypeBadType)
{
}

inline
void CPopulation::addPopRecord(CPopulationRecord popRecord)
{
	_PopRecords.push_back(popRecord);
}

//////////////////////////////////////////////////////////////////////////////
// CAICircle                                                                //
//////////////////////////////////////////////////////////////////////////////
/*
// Template members must be inside class definitions for VC++6
template <class V, class R>
CAICircle::CAICircle(V const& center, R radius)
: _Center(center)
, _Radius(radius)
{
}

// Template members must be inside class definitions for VC++6
template <class V>
bool CAICircle::isInside(V const& pos)
{
	return (pos - _Center).sqrnorm() <= (_Radius+1)*(_Radius+1);
}
*/
//////////////////////////////////////////////////////////////////////////////
// CAabb                                                                    //
//////////////////////////////////////////////////////////////////////////////

inline
CAabb::CAabb()
: _VMin(INT_MAX/CAICoord::UNITS_PER_METER, INT_MAX/CAICoord::UNITS_PER_METER)
, _VMax(INT_MIN/CAICoord::UNITS_PER_METER, INT_MIN/CAICoord::UNITS_PER_METER)
{
}
/*
template <class V>
CAabb::CAabb(std::vector<V> const& coords)
: _VMax(INT_MIN/CAICoord::UNITS_PER_METER, INT_MIN/CAICoord::UNITS_PER_METER)
, _VMin(INT_MAX/CAICoord::UNITS_PER_METER, INT_MAX/CAICoord::UNITS_PER_METER)
{
	for (uint k=0; k<coords.size(); ++k)
		includePoint(coords[k]);
}

template <class V>
void CAabb::includePoint(V const& point)
{
	if (point.x() < _VMin.x())
		_VMin.setX(point.x());
	if (point.x() > _VMax.x())
		_VMax.setX(point.x());
	if (point.y() < _VMin.y())
		_VMin.setY(point.y());
	if (point.y() > _VMax.y())
		_VMax.setY(point.y());
}
*/
inline
void CAabb::includeAabb(CAabb const& box)
{
	if (box._VMin.x() < _VMin.x())
		_VMin.setX(box._VMin.x());
	if (box._VMax.x() > _VMax.x())
		_VMax.setX(box._VMax.x());
	if (box._VMin.y() < _VMin.y())
		_VMin.setY(box._VMin.y());
	if (box._VMax.y() > _VMax.y())
		_VMax.setY(box._VMax.y());
}

inline
void CAabb::init()
{
	*this = CAabb();
}
/*
// Template members must be inside class definitions for VC++6
template<class V>
bool CAabb::isInside(V const& v)
{
	return v.x() >= _VMin.x() && v.y() >= _VMin.y() && v.x() <= _VMax.x() && v.y() <= _VMax.y();
}
*/

//////////////////////////////////////////////////////////////////////////////
// CFaunaZone                                                               //
//////////////////////////////////////////////////////////////////////////////

inline
bool CFaunaZone::haveActivity(AITYPES::CPropertyId const& activity) const
{
	if (!_AdditionalActivities.empty())
		return _AdditionalActivities.have(activity);
	return _InitialActivities.have(activity);
}

inline
bool CFaunaZone::haveActivity(AITYPES::CPropertySet const& activities) const
{
	if (!_AdditionalActivities.empty())
		return _AdditionalActivities.containsPartOfStrict(activities);
	return _InitialActivities.containsPartOfStrict(activities);
}

inline
float CFaunaZone::getFreeAreaScore() const
{
	float radius = getRadius();
	return (float)((radius*radius)/((float)getNbUse()+0.001));
}

inline
uint32 CFaunaZone::getNbUse() const
{
	sint nbUse = getRefCount()-1; // -1 for the container
#ifdef NL_DEBUG
	nlassert(nbUse>=0);
#endif
	return nbUse;
}
	
//////////////////////////////////////////////////////////////////////////////
// CAIRefPlaceXYR                                                           //
//////////////////////////////////////////////////////////////////////////////

inline
CAIRefPlaceXYR::CAIRefPlaceXYR(CPlaceOwner*	owner, CAIPlace const* zone)
: CAIPlace(owner, NULL)
{
#ifdef NL_DEBUG
	nlassert(zone!=NULL);
#endif
	_Zone = zone;
}

inline
CAIRefPlaceXYR::operator CAIPlace const*() const
{
	return _Zone;
}

//////////////////////////////////////////////////////////////////////////////
// CAIRefPlaceXYRFauna                                                      //
//////////////////////////////////////////////////////////////////////////////

inline
CAIRefPlaceXYRFauna::CAIRefPlaceXYRFauna(CPlaceOwner*	owner, CAIPlace const* zone)
: CAIRefPlaceXYR(owner, zone)
{
}

//////////////////////////////////////////////////////////////////////////////
// CRebuildContinentAndOutPost                                              //
//////////////////////////////////////////////////////////////////////////////

inline
CRebuildContinentAndOutPost::CRebuildContinentAndOutPost(CContinent* continent)
: _Continent(continent)
{
}

inline
bool CRebuildContinentAndOutPost::absorb(CLazyProcess const& lazyProcess) const
{
	CRebuildContinentAndOutPost const* other = dynamic_cast<CRebuildContinentAndOutPost const*>(&lazyProcess);
	if (!other)
		return false;
	return *other==*this;
}

inline
bool CRebuildContinentAndOutPost::operator==(CRebuildContinentAndOutPost const& other) const
{
	return other._Continent==_Continent;
}

//////////////////////////////////////////////////////////////////////////////
// CContinent                                                               //
//////////////////////////////////////////////////////////////////////////////

inline
CContinent::CContinent(CAIInstance* owner)
: CChild<CAIInstance>(owner)  
{
}

//////////////////////////////////////////////////////////////////////////////
// CRoad                                                                    //
//////////////////////////////////////////////////////////////////////////////

inline
void CRoad::calcLength()
{
	double length = 0.0f;
	for	(sint j=((sint)_Coords.size())-1; j>0; j--)
		length += (_Coords[j] - _Coords[j-1]).toAIVector().norm();
	_Length = (float)length;
	calCost();
}

inline
void CRoad::setDifficulty(float const& difficulty)
{
	_Difficulty = difficulty;
	calCost();
}

inline
void CRoad::calCost()
{
	_CostCoef = _Length*_Difficulty;
}

inline
float CRoad::getCost() const
{
	return _CostCoef*getRefCount();	//	takes account of use.
}

//////////////////////////////////////////////////////////////////////////////
// CRoadTrigger                                                             //
//////////////////////////////////////////////////////////////////////////////

inline
CRoadTrigger::CRoadTrigger(CRoad* owner, uint32 alias, std::string const& name)
: CAliasChild<CRoad>(owner, alias, name)
{
}

//////////////////////////////////////////////////////////////////////////////
// CCell                                                                    //
//////////////////////////////////////////////////////////////////////////////

inline
size_t CCell::npcZoneCount()
{
	return _NpcZonePlaces.size() + _NpcZoneShapes.size();
}

inline
CNpcZone* CCell::npcZone(size_t index)
{
	if (index<_NpcZonePlaces.size())
		return _NpcZonePlaces[(uint32)index];
	else
		return _NpcZoneShapes[(uint32)index];
}

inline
void CCell::getNeighBourgCellList(std::vector<CCell*>& cells) const
{
	cells.reserve(_NeighbourCells.size()+1);
	// build a list of candidate cell to look for the rest zone : ether the current zone or a beighbour
	std::set<NLMISC::CDbgPtr<CCell> >::const_iterator first(_NeighbourCells.begin()), last(_NeighbourCells.end());
	for (; first != last; ++first)
		cells.push_back(*first);
}

//////////////////////////////////////////////////////////////////////////////
// CGroupFamily                                                             //
//////////////////////////////////////////////////////////////////////////////



inline
CGroupFamily::~CGroupFamily()
{
	_GroupDescs.clear();	
}

inline
void CGroupFamily::setProfileParams(std::string const& str, NLMISC::CVirtualRefCount* objet)
{
#if	!FINAL_VERSION
	nlassert(objet);
#endif
	_Params.insert(std::make_pair(NLMISC::CStringMapper::map(str),objet));
}

inline
NLMISC::CVirtualRefCount const* CGroupFamily::getProfileParams(std::string const& str) const
{
	TParamsList::const_iterator	const it = _Params.find(NLMISC::CStringMapper::map(str));
	if (it==_Params.end())
		return	NULL;
	return it->second;
}

inline
void CGroupFamily::addProfileProperty(std::string const& propertyName, AITYPES::CPropertySet const& property)
{
	_Properties[NLMISC::CStringMapper::map(propertyName)] = property;
}

inline
AITYPES::CPropertySet const& CGroupFamily::getProfileProperty(std::string const& propertyName) const
{
	static AITYPES::CPropertySet emptyActivities;
	
	TActivityList::const_iterator it = _Properties.find(NLMISC::CStringMapper::map(propertyName));
	if (it==_Properties.end())
		return emptyActivities;
	return it->second;
}

//////////////////////////////////////////////////////////////////////////////
// CGroupDesc                                                               //
//////////////////////////////////////////////////////////////////////////////

template <class FamilyT>
CGroupDesc<FamilyT>::CGroupDesc(FamilyT* owner, uint32 alias, std::string const& name)
: CAliasChild<FamilyT>(owner, alias, name)
, _BotCount(0)
, _CountMultiplier(false)
, _GroupEnergyValue((uint32)(0.01*AITYPES::ENERGY_SCALE))
, _SpawnType(AITYPES::SpawnTypeAlways)
, _EnergyCoef(1.f)
, _MultiLevel(false)
, _Sheet(NULL)
, _MultiLevelSheets(_MultiLevelSheetCount)
, _LevelDelta(0)
, _PlayerAttackable(true)
, _BotAttackable(true)
{
	for (size_t j=0; j<_MultiLevelSheetCount; ++j)
		_MultiLevelSheets[j] = NULL;
	for (uint32	i=0; i<4; ++i)
	{
		_SeasonFlags[i] = false;
		_WeightLevel[i] = 0;
	}
}

template <class FamilyT>
bool CGroupDesc<FamilyT>::isValidForDayOrNight(bool const& isDay) const
{
	if (_SpawnType == AITYPES::SpawnTypeAlways)
		return true;
	return isDay?(_SpawnType == AITYPES::SpawnTypeDay):(_SpawnType == AITYPES::SpawnTypeNight);
}

template <class FamilyT>
void CGroupDesc<FamilyT>::setSheet(AISHEETS::ICreatureCPtr const& sheetPtr)
{
#ifdef NL_DEBUG
	nlassert(sheetPtr);
#endif
	_Sheet = sheetPtr;
}

template <class FamilyT>
bool CGroupDesc<FamilyT>::setSheet(std::string const& sheetName)
{
	if (!sheetName.empty())
	{
		if (_MultiLevel)
		{
			for (size_t i=0; i<_MultiLevelSheetCount; ++i)
			{
				char letter = char(i/4) + 'b';
				char number = (i%4) + '1';
				std::string sheetNameLevel = sheetName+letter+number;
				// Compute sheet id
				NLMISC::CSheetId sheetId(sheetNameLevel+".creature");
				// Find the sheet
				AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
				// If the sheet doesn't exist
				if (sheetId==NLMISC::CSheetId::Unknown || !sheet)
				{
					nlwarning("Sheet '%s' for group '%s'%s is unknown !", 
						sheetNameLevel.c_str(), 
						this->getFullName().c_str(),
						this->getAliasString().c_str());
					return false;
				}
				_MultiLevelSheets[i] = sheet;
			}
		}
		else
		{
			// Compute sheet id
			NLMISC::CSheetId sheetId(sheetName+".creature");
			// Find the sheet
			AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
			// If the sheet doesn't exist
			if (sheetId==NLMISC::CSheetId::Unknown || !sheet)
			{
				nlwarning("Sheet '%s' for group '%s'%s is unknown !", 
					sheetName.c_str(), 
					this->getFullName().c_str(),
					this->getAliasString().c_str());
				return false;
			}
			_Sheet=sheet;
		}
	}
	return true;
}

template <class FamilyT>
AISHEETS::ICreatureCPtr CGroupDesc<FamilyT>::sheet(sint32 baseLevel) const
{
	if (_MultiLevel && baseLevel!=-1)
	{
		sint32 level = baseLevel + getLevelDelta();
		// Clamp to [0;_MultiLevelSheetCount]
		level = std::min(level, (sint32)(_MultiLevelSheetCount-1));
		level = std::max(level, (sint32)0);
		return _MultiLevelSheets[level];
	}
	else
		return _Sheet;
}

template <class FamilyT>
uint32 CGroupDesc<FamilyT>::getRealBotCount() const
{
	if (_Sheet && getCountMultiplierFlag() && !_MultiLevel)
		return _BotCount * _Sheet->DynamicGroupCountMultiplier();
	else
		return _BotCount;
}

template <class FamilyT>
void CGroupDesc<FamilyT>::setSeasonFlags(bool const seasonFlags[4])
{
	for (uint32	i=0;i<4;i++)
		_SeasonFlags[i] = seasonFlags[i];
}

template <class FamilyT>
void CGroupDesc<FamilyT>::setWeightLevels(uint32 const weights[4])
{
	for (uint32	i=0;i<4;i++)
		_WeightLevel[i] = weights[i];
}

template <class FamilyT>
CGrpFauna* CGroupDesc<FamilyT>::createFaunaGroup(CFamilyBehavior* familyBehavior) const
{
	H_AUTO(createFaunaGroup)

	uint32 energyLevel = familyBehavior->effectiveLevel();
	AISHEETS::ICreatureCPtr const ls = sheet();

	const	RYAI_MAP_CRUNCH::TAStarFlag	AStarFlag=RYAI_MAP_CRUNCH::WaterAndNogo;

	if	(!ls)
	{
	#if !FINAL_VERSION
		nlwarning("CRegion::createGroup Can't retreive creature info for sheet '%s' from group desc '%s'%s to spawn in region '%s'%s", 
			(sheet()?sheet()->SheetId().toString().c_str():NLMISC::CSheetId::Unknown.toString().c_str()), 
			this->getAliasFullName().c_str(), 
			this->getAliasString().c_str(),	
			this->getOwner()->getAliasFullName().c_str(),	
			this->getOwner()->getAliasString().c_str());	//	CaracSheet
	#endif
		return NULL;
	}

	AITYPES::CPropertySet	food, rest;
	
	familyBehavior->getActivities	(food, rest);
	
	/*
	nlinfo("Num food activities = %d", (int) food.size());
	nlinfo("Num rest activities = %d", (int) rest.size());
	*/

	const	CFaunaZone	*faunaZone=familyBehavior->getOwner()->lookupFaunaZone(rest, AStarFlag, familyBehavior->grpFamily()->getSubstitutionId());
	if	(!faunaZone)
		return	NULL;

	const	CFaunaZone	*fzFood=NULL;
	const	CFaunaZone	*fzRest=NULL;

	// select a random spawn zone into the vector
	{
		std::vector<CCell*>	cells;
		faunaZone->getOwner()->getNeighBourgCellList(cells);
		cells.push_back(faunaZone->getOwner());

		if	(!familyBehavior->getOwner()->findRestAndFoodFaunaZoneInCellList(fzRest, rest, fzFood, food, cells, AStarFlag))
		{
		#if !FINAL_VERSION
			nlwarning("CRegion::createGroup can't find zone pair with properties food: <%s> rest: <%s>, for family %s, group '%s'%s", 
				food.toString().c_str(), 
				rest.toString().c_str(), 
				familyBehavior->getName().c_str(),	
				this->getAliasFullName().c_str(),		
				this->getAliasString().c_str());
		#endif
			return NULL;
		}
		if	(	!fzFood
			||	!fzRest)
		{
		#if !FINAL_VERSION
			nlwarning("CRegion::createGroup can't find zone pair with properties food: <%s> rest: <%s>, for family %s, group '%s'%s", 
				food.toString().c_str(), 
				rest.toString().c_str(), 
				familyBehavior->getName().c_str(),	
				this->getAliasFullName().c_str(),		
				this->getAliasString().c_str());
		#endif
			return	NULL;
		}

	}

	if	(	!fzRest
		&&	!rest.empty()	)
	{
	#if !FINAL_VERSION
		nlwarning("Fauna dynamic: CRegion::createGroup can't find fauna zone to rest '%s' in region '%s'%s", 
			familyBehavior->getName().c_str(),	
			this->getAliasFullName().c_str(),	
			this->getAliasString().c_str());
	#endif
		return NULL;
	}

	CMgrFauna	*mf		=	familyBehavior->mgrFauna();
	CGrpFauna	*grp	=	new CGrpFauna(mf, NULL, AStarFlag);
	mf->groups().addAliasChild(grp);

	grp->initDynGrp	(this, familyBehavior);

	/// Fill the fauna group data
	CAIRefPlaceXYRFauna *refPlace = new CAIRefPlaceXYRFauna(grp,fzRest);
	refPlace->setupFromOldName("spawn");
	grp->places().addChild	(refPlace, CGrpFauna::SPAWN_PLACE	);
	refPlace = new CAIRefPlaceXYRFauna(grp,fzFood);
	refPlace->setupFromOldName("food");
	grp->places().addChild	(refPlace,	CGrpFauna::EAT_PLACE	);
	refPlace = new CAIRefPlaceXYRFauna(grp,fzRest);
	refPlace->setupFromOldName("rest");
	grp->places().addChild	(refPlace, CGrpFauna::REST_PLACE	);

	if (	!grp->places()[CGrpFauna::SPAWN_PLACE]->worldValidPos().isValid()
		||	!grp->places()[CGrpFauna::EAT_PLACE]->worldValidPos().isValid()
		||	!grp->places()[CGrpFauna::REST_PLACE]->worldValidPos().isValid()	)
	{
		if	(LogGroupCreationFailure)
		{
		#if !FINAL_VERSION
			nlwarning("Invalid place to spawn dynamic group '%s'%s", 
				this->getAliasFullName().c_str(),
				this->getAliasString().c_str());
			if (!grp->places()[CGrpFauna::SPAWN_PLACE]->worldValidPos().isValid())
				nlwarning("  Invalid spawn place %s", grp->places()[CGrpFauna::SPAWN_PLACE]->midPos().toString().c_str());
			if (!grp->places()[CGrpFauna::EAT_PLACE]->worldValidPos().isValid())
				nlwarning("  Invalid eat place %s", grp->places()[CGrpFauna::EAT_PLACE]->midPos().toString().c_str());
			if (!grp->places()[CGrpFauna::REST_PLACE]->worldValidPos().isValid())
				nlwarning("  Invalid rest place %s", grp->places()[CGrpFauna::REST_PLACE]->midPos().toString().c_str());
		#endif
		}
		mf->groups().removeChildByIndex(grp->getChildIndex());
		return NULL;
	}

	CPopulation	*pop = new CPopulation(grp);
	{
		pop->setWeight(1);
		pop->setSpawnType(spawnType());
		pop->addPopRecord(CPopulationRecord(ls, getRealBotCount()));

		for	(size_t	i=0; i<_PopulationRecords.size(); ++i)
			pop->addPopRecord(_PopulationRecords[i]);

	}
	grp->populations().addAliasChild(pop);

	grp->setAutoSpawn(false);
	grp->spawn();

	if	(!grp->getSpawnObj())
	{
		// the spawning has failed, delete the useless object
	#if !FINAL_VERSION
		if (!grp->getSpawnCounter().remainToMax())
			nldebug("Cannot spawn the dynamic group: maximum reached");
		else
			nlwarning("Failed to spawn the dynamic group");
	#endif
		mf->groups().removeChildByIndex(grp->getChildIndex());
		return	NULL;
	}
	
	return	grp;
}

inline
static CAIVector randomPos(double dispersionRadius)
{
	if (dispersionRadius<=0.)
	{
		return CAIVector(0., 0.);
	}
	uint32 const maxLimit=((uint32)~0U)>>1;
	double rval = (double)CAIS::rand32(maxLimit)/(double)maxLimit; // [0-1[
	double r = dispersionRadius*sqrt(rval);
	rval = (double)CAIS::rand32(maxLimit)/(double)maxLimit; // [0-1[
	double t = 2.0*NLMISC::Pi*rval;
	double dx = cos(t)*r;
	double dy = sin(t)*r;
	return CAIVector(dx, dy);
}

template <class FamilyT>
CGroupNpc* CGroupDesc<FamilyT>::createNpcGroup(CMgrNpc* mgr, CAIVector const& pos, double dispersionRadius, sint32 baseLevel, bool spawnBots) const
{
	H_AUTO(createNpcGroup)
	// Keep base level positive or -1 (single level)
	baseLevel = std::max(baseLevel, (sint32)-1);
	// Verify we have all the sheets
	// :TODO: Add verification for named bots sheets
	if (!sheet(baseLevel) && getBaseBotCount()>0)
	{
		nlwarning("CGroupDesc::createNpcGroup can't retrieve sheet from group '%s'%s in region '%s'%s", this->getAliasFullName().c_str(), this->getAliasString().c_str(), this->getOwner()->getAliasFullName().c_str(), this->getOwner()->getAliasString().c_str());
		return NULL;
	}
	FOREACHC (itBotDesc, typename CCont<CBotDesc<FamilyT> >, botDescs())
	{
		if (!itBotDesc->sheet(baseLevel))
		{
			nlwarning("CGroupDesc::createNpcGroup can't retrieve sheet from bot '%s'%s in group '%s'%s in region '%s'%s", itBotDesc->getAliasFullName().c_str(), itBotDesc->getAliasString().c_str(), this->getAliasFullName().c_str(), this->getAliasString().c_str(), this->getOwner()->getAliasFullName().c_str(), this->getOwner()->getAliasString().c_str());
			return NULL;
		}
	}
	// Create a group
	CGroupNpc *grp = new CGroupNpc(mgr, NULL, /*AStarFlag*/RYAI_MAP_CRUNCH::Nothing);
	// Register it in the manager
	mgr->groups().addAliasChild(grp);
	// Set the group parameters
	grp->setAutoSpawn(false);
	grp->setName(this->getName());
	grp->clearParameters();
	for (uint i=0; i<grpParameters().size(); ++i)
		grp->addParameter(grpParameters()[i]);
	grp->setPlayerAttackable(_PlayerAttackable);
	grp->setBotAttackable(_BotAttackable);
	
	// Save whether we have named or unnamed bots
	if	(getRealBotCount() == 0)
		grp->setBotsAreNamedFlag();
	else
		grp->clrBotsAreNamedFlag();
	
	
	{
		uint i=0;
		// build the specific bots data
		for	(; i<botDescs().size(); ++i)
		{
			const CBotDesc<FamilyT>	*const	bd = botDescs()[i];

			uint nbClone = 1;
			if (getCountMultiplierFlag())
			{
				// the group use the multiplier from the creature sheet
				nbClone *= bd->sheet(baseLevel)->DynamicGroupCountMultiplier();
			}

			// loop for the requested clones
			for (uint j=0; j<nbClone; ++j)
			{
				grp->bots().addChild(new CBotNpc(grp, 0, bd->getBotName()), i); // Doub: 0 instead of bd->getAlias() otherwise all bots will have the same non-zero alias
				CBotNpc	*const	bot = static_cast<CBotNpc*>(grp->bots()[i]);

				bot->setSheet		(bd->sheet(baseLevel));
				bot->equipmentInit	();
				bot->initEnergy		(groupEnergyCoef());
				CAIVector rpos(pos);
				if (i!=0)
				{
					RYAI_MAP_CRUNCH::CWorldMap const& worldMap = CWorldContainer::getWorldMap();
					RYAI_MAP_CRUNCH::CWorldPosition	wp;
					uint32 maxTries = 100;
					do
					{
						rpos = pos;
						rpos += randomPos(dispersionRadius);
						--maxTries;
					}
					while (!worldMap.setWorldPosition(AITYPES::vp_auto, wp, rpos) && maxTries>0);
					if (maxTries<=0)
						rpos = pos;
				}
				bot->setStartPos	(rpos.x().asDouble(),rpos.y().asDouble(), 0, AITYPES::vp_auto);
			}
		}

		// build un-named bot
		uint nbClone = getRealBotCount();
		for	(uint j=0; j<nbClone; ++i,++j)
		{
			grp->bots().addChild(new CBotNpc(grp, 0, grp->getName()), i); // Doub: 0 instead of getAlias()+i otherwise aliases are wrong

			CBotNpc	*const	bot = NLMISC::safe_cast<CBotNpc*>(grp->bots()[i]);

			bot->setSheet		(sheet(baseLevel));
			bot->equipmentInit	();
			bot->initEnergy		(groupEnergyCoef());
			CAIVector rpos(pos);
			if (i!=0)
			{
				RYAI_MAP_CRUNCH::CWorldMap const& worldMap = CWorldContainer::getWorldMap();
				RYAI_MAP_CRUNCH::CWorldPosition	wp;
				uint32 maxTries = 100;
				do
				{
					rpos = pos;
					rpos += randomPos(dispersionRadius);
					--maxTries;
				}
				while (!worldMap.setWorldPosition(AITYPES::vp_auto, wp, rpos) && maxTries>0);
				if (maxTries<=0)
					rpos = pos;
			}
			bot->setStartPos	(rpos.x().asDouble(),rpos.y().asDouble(), 0, AITYPES::vp_auto);
		}

	}

	grp->spawn();
	if	(!grp->getSpawnObj())
	{
		// the spawning has failed, delete the useless object
		nlwarning("Failed to spawn the dynamic group");
		mgr->groups().removeChildByIndex(grp->getChildIndex());
		return NULL;
	}
	if (spawnBots)
		grp->getSpawnObj()->spawnBots();
	return	grp;
}

template <class FamilyT>
uint32	CGroupDesc<FamilyT>::calcTotalEnergyValue	()	const
{
	uint32	totalEnergyValue=0;
	{
		typename CCont<CBotDesc<FamilyT> >::const_iterator	it=botDescs().begin(), itEnd=botDescs().end();
		//	add specified bots ..
		while (it!=itEnd)
		{
			totalEnergyValue+=it->energyValue();
			++it;
		}
	}

	//	add botcount ..

	if (sheet())
		totalEnergyValue += getRealBotCount()*sheet()->EnergyValue();
	
	{
		std::vector<CPopulationRecord>::const_iterator	it=_PopulationRecords.begin(), itEnd=_PopulationRecords.end();
		while (it!=itEnd)
		{
			const  CPopulationRecord &pr = *it;
			totalEnergyValue += pr.getEnergyValue(getCountMultiplierFlag());
			++it;
		}

	}
	return	totalEnergyValue;
}

template <class FamilyT>
IAliasCont		*CGroupDesc<FamilyT>::getAliasCont(AITYPES::TAIType type)
{
	switch(type)
	{
	case AITYPES::AITypeSquadTemplateMember:
	case AITYPES::AITypeBotTemplate:
	case AITYPES::AITypeBotTemplateMultiLevel:
		return &_BotDescs;
	default:
		return	NULL;
	}
	
}

template <class FamilyT>
sint CGroupDesc<FamilyT>::getNbUse() const
{
	return getRefCount()-1; // less one because its also referenced by aliascont.
}

template <class FamilyT>
CAliasTreeOwner	*CGroupDesc<FamilyT>::createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;
	
	switch(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITYPES::AITypeSquadTemplateMember:
	case AITYPES::AITypeBotTemplate:
	case AITYPES::AITypeBotTemplateMultiLevel:
		child = new CBotDesc<FamilyT>(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	default:
		break;
	}
	
	if	(child)
		cont->addAliasChild(child);
	return	child;
}

template <class FamilyT>
std::string CGroupDesc<FamilyT>::getIndexString() const
{
	return	this->getOwner()->getIndexString()+NLMISC::toString(":%u", this->getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// ContextGroupDesc actions                                                 //
//////////////////////////////////////////////////////////////////////////////

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_SHEE,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	std::string lookSheet;
	
	if (!getArgs(args,name(), lookSheet))
		return;
	
	if (!groupDesc->setSheet(lookSheet))
	{
		groupDesc->getOwner()->groupDescs().removeChildByIndex(groupDesc->getChildIndex());
		CWorkPtr::groupDesc(NULL);
		return;
	}
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_LVLD,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	sint32 levelDelta;
	
	if (!getArgs(args,name(), levelDelta))
		return;
	
	groupDesc->setLevelDelta(levelDelta);
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_SEAS,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	bool seasons[4];
	
	if (!getArgs(args,name(), seasons[0], seasons[1], seasons[2], seasons[3]))
		return;
	
	groupDesc->setSeasonFlags(seasons);
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_ACT,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	uint32 spawnType;
	if (!getArgs(args, name(), spawnType))
		return;
	
	groupDesc->setSpawnType((AITYPES::TSpawnType)spawnType);
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_APRM,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;	
	
	for (size_t i=0; i<args.size(); ++i)
	{
		std::string property;
		args[i].get(property);
		groupDesc->properties().addProperty(AITYPES::CPropertyId::create(property));
	}
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_NRG,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	uint32 weight[4];
	
	if (!getArgs(args,name(), weight[0], weight[1], weight[2], weight[3]))
		return;
	
	groupDesc->setWeightLevels(weight);
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_EQUI,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	groupDesc->botEquipment().clear();
	
	for (size_t i=0; i<args.size(); ++i)
	{
		std::string equip;
		args[i].get(equip);
		groupDesc->botEquipment().push_back(equip);
	}
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_GPRM,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	for (size_t i=0; i<args.size(); ++i)
	{
		std::string param;
		args[i].get(param);
		
		param = NLMISC::toLower(param);
		
		if	(	param == "contact camp"
			||	param == "contact outpost"
			||	param == "contact city"
			||	param == "boss"	)
			groupDesc->properties().addProperty(param);
		else	// unreconized param, leace it for the group instance
			groupDesc->grpParameters().push_back(param);
	}
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,BOTTMPL,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	std::string lookSheet;
	bool multiLevel;
	
	// read the alias tree from the argument list
	CAIAliasDescriptionNode* aliasTree;
	if (!getArgs(args, name(), aliasTree, lookSheet, multiLevel))
		return;
	
	// see whether the region is already loaded
	CBotDesc<FamilyT>* botDesc = groupDesc->botDescs().getChildByAlias(aliasTree->getAlias());
	if (!botDesc)
		return;
	
	botDesc->setMultiLevel(multiLevel);
	botDesc->setSheet(lookSheet);
	
	CWorkPtr::botDesc(botDesc);
	CContextStack::setContext(CAISActionEnums::ContextBotDesc);
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextBotDesc,BT_EQUI,FamilyT)
{
	CBotDesc<FamilyT>* botDesc = static_cast<CBotDesc<FamilyT>*>(CWorkPtr::botDesc());
	if (!botDesc)
		return;
	
	for (size_t i=0; i<args.size(); ++i)
	{
		std::string equip;
		args[i].get(equip);
		botDesc->equipement().push_back(equip);
	}
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextBotDesc,BT_LVLD,FamilyT)
{
	CBotDesc<FamilyT>* botDesc = static_cast<CBotDesc<FamilyT>*>(CWorkPtr::botDesc());
	if (!botDesc)
		return;
	
	sint32 levelDelta;
	
	if (!getArgs(args,name(), levelDelta))
		return;
	
	botDesc->setLevelDelta(levelDelta);
}


/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_GNRJ,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	uint32 energyValue;
	
	if (!getArgs(args,name(), energyValue))
		return;
}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,POPVER,FamilyT)
{
	// add a population version for a group
	// args: uint32 alias, string spawn_type, uint weight, (string sheet, uint32 count)+

	if(!CWorkPtr::groupDesc())
		return;
	
	const uint32 fixedArgsCount = 0;
	if (args.size()<fixedArgsCount+2 || ((args.size()-fixedArgsCount)&1)==1)
	{
		nlwarning("POPVER action FAILED due to bad number of arguments (%d)", args.size());
		return;
	}
	
	// get hold of the parameters and check their validity
	for (size_t i=fixedArgsCount; i+1<args.size(); i+=2)
	{
		std::string sheet;
		uint32 count;
		
		if	(	!args[i].get(sheet)
			||	!args[i+1].get(count))
		{
			nlwarning("POPVER Add Record FAILED due to bad arguments");
			continue;
		}
		
		NLMISC::CSheetId sheetId(sheet);
		if (sheetId==NLMISC::CSheetId::Unknown)
		{
			nlwarning("POPVER Add Record Invalid sheet: %s", sheet.c_str());
			continue;
		}
		
		AISHEETS::ICreatureCPtr sheetPtr = AISHEETS::CSheets::getInstance()->lookup(sheetId);
		if (!sheetPtr)
		{
			nlwarning("POPVER Add Record Invalid sheet: %s", sheet.c_str());
			continue;
		}
		static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc())->populationRecords().push_back(CPopulationRecord(sheetPtr, count));
	}

}

/// :KLUDGE: This code is copied in ai_outpost_actions.h. Update both if you
/// make a modification.
// scales bot energy .. to match with group's one.
DEFINE_ACTION_TEMPLATE1(ContextGroupDesc,GT_END,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	if (!groupDesc->isMultiLevel())
	{
		uint32 totalEnergyValue = groupDesc->calcTotalEnergyValue();
		if (totalEnergyValue)
		{
			double coef = (double)groupDesc->groupEnergyValue()/(double)totalEnergyValue;
			groupDesc->setGroupEnergyCoef((float)coef);
		}
		else
		{
			nlwarning("Retrieved total energy value of 0 for group: %s",groupDesc->getFullName().c_str());
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CBotDesc                                                                 //
//////////////////////////////////////////////////////////////////////////////

template <class FamilyT>
CBotDesc<FamilyT>::CBotDesc(CGroupDesc<FamilyT>* owner, uint32 alias, std::string const& name)
: CAliasChild<CGroupDesc<FamilyT> >(owner, alias, name)
, _MultiLevel(false)
, _Sheet(NULL)
, _MultiLevelSheets(_MultiLevelSheetCount)
, _LevelDelta(0)
, _UseSheetBotName(false)
{
	for (size_t i=0; i<_MultiLevelSheetCount; ++i)
		_MultiLevelSheets[i] = NULL;
}

template <class FamilyT>
std::string CBotDesc<FamilyT>::getIndexString() const
{
	return this->getOwner()->getIndexString() + NLMISC::toString(":%u", this->getChildIndex());
}

template <class FamilyT>
void CBotDesc<FamilyT>::setSheet(std::string const& sheetName)
{
	if (!sheetName.empty())
	{
		if (_MultiLevel)
		{
			for (size_t i=0; i<_MultiLevelSheetCount; ++i)
			{
				char letter = char(i/4) + 'b';
				char number = (i%4) + '1';
				std::string sheetNameLevel = sheetName+letter+number;
				// Compute sheet id
				NLMISC::CSheetId sheetId(sheetNameLevel+".creature");
				// Find the sheet
				AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
				// If the sheet doesn't exist
				if (sheetId==NLMISC::CSheetId::Unknown || !sheet)
				{
					nlwarning("Sheet '%s' for bot '%s' is unknown !", sheetNameLevel.c_str(), this->getAliasFullName().c_str());
				}
				_MultiLevelSheets[i] = sheet;
			}
		}
		else
		{
			// Compute sheet id
			NLMISC::CSheetId sheetId(sheetName+".creature");
			// Find the sheet
			AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
			// If the sheet doesn't exist
			if (sheetId==NLMISC::CSheetId::Unknown || !sheet)
			{
				nlwarning("Sheet '%s' for bot '%s' is unknown !", sheetName.c_str(), this->getAliasFullName().c_str());
			}
			_Sheet = sheet;
		}
	}
}

template <class FamilyT>
AISHEETS::ICreatureCPtr CBotDesc<FamilyT>::sheet(sint32 baseLevel) const
{
	if (_MultiLevel && baseLevel!=-1)
	{
		CGroupDesc<FamilyT>* parent = this->getOwner();
		sint32 level = baseLevel + getLevelDelta() + parent->getLevelDelta();
		// Clamp to [0;_MultiLevelSheetCount]
		level = std::min(level, (sint32)(_MultiLevelSheetCount-1));
		level = std::max(level, (sint32)0);
		return _MultiLevelSheets[level];
	}
	else
		return _Sheet;
}

template <class FamilyT>
uint32 CBotDesc<FamilyT>::energyValue() const
{
	if (!_Sheet && !_MultiLevel)
		nlwarning("Bot descriptor has no sheet and is not multilevel, correct above warnings!");
	
	if (_Sheet)
		return _Sheet->EnergyValue() * _Sheet->DynamicGroupCountMultiplier();
	return	0;
}

template <class FamilyT>
std::string const& CBotDesc<FamilyT>::getBotName() const
{
	if (_UseSheetBotName && _Sheet && !_Sheet->BotName().empty())
		return _Sheet->BotName();
	else
		return this->getName();
}

//////////////////////////////////////////////////////////////////////////////
// CCell                                                                    //
//////////////////////////////////////////////////////////////////////////////

inline
void CCell::unrefZoneInRoads()
{
	FOREACH(it, TAliasZonePlaceList, _NpcZonePlaces)
		it->unrefZoneInRoads	();
	FOREACH(it, TAliasZoneShapeList, _NpcZoneShapes)
		it->unrefZoneInRoads	();
	_NeighbourCells.clear	();
}

//////////////////////////////////////////////////////////////////////////////
// CNpcZone                                                                 //
//////////////////////////////////////////////////////////////////////////////

// :TODO: check if that inlining is necessary
inline
void CNpcZone::unrefZoneInRoads()
{
	while (!_Roads.empty())
		_Roads.back()->unlinkRoad();
}

//////////////////////////////////////////////////////////////////////////////
// CCellZone                                                                //
//////////////////////////////////////////////////////////////////////////////

inline
void CCellZone::unrefZoneInRoads()
{
	FOREACH(it, CCont<CCell>, _Cells)
		it->unrefZoneInRoads();
}


#endif
