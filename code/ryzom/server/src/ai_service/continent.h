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




#ifndef CONTINENT_H
#define CONTINENT_H

#include "nel/misc/string_mapper.h"
#include "child_container.h"
#include "alias_tree_root.h"
#include "manager_parent.h"
#include "family_profile.h"
#include "ai_place_xyr.h"
#include "server_share/mission_messages.h"
#include <set>
#include "family_behavior.h"
#include "ai_outpost.h"
#include "service_dependencies.h"
#include "sheets.h"

class CContinent;
class CRegion;
class COutpost;
class CCellZone;
class CGroupFamily;
class IGroupDesc;
template <class FamilyT>
class CGroupDesc;
template <class FamilyT>
class CBotDesc;
class CCell;
class CFaunaZone;
class CNpcZone;
class CRoad;
class CRoadTrigger;
class CAIInstance;
class CMgrFauna;
class CMgrNpc;
class CGrpFauna;
class CGroupNpc;

//////////////////////////////////////////////////////////////////////////////
// CPopulationRecord                                                        //
//////////////////////////////////////////////////////////////////////////////

// the description of the sets of alternative populations for the group
class CPopulationRecord
{
public:
	CPopulationRecord(AISHEETS::ICreatureCPtr const& sheetData, uint count);
	
	bool operator==(CPopulationRecord const& other) const;
	
	AISHEETS::ICreatureCPtr getCreatureSheet() const;
	uint getBotCount(bool useCreatureMultiplier) const;
	
	uint32 getEnergyValue(bool useCreatureMultiplier) const;
	
private:
	AISHEETS::ICreatureCPtr	_SheetData;
	uint					_Count;
};

//////////////////////////////////////////////////////////////////////////////
// CPopulationOwner                                                         //
//////////////////////////////////////////////////////////////////////////////

class CPopulationOwner
{
public:
	virtual ~CPopulationOwner() { }
//	virtual	std::string	getPopulationOwnerIndexString() const = 0;
	
	virtual	std::string	getIndexString() const = 0;
//	{
//		return getPopulationOwnerIndexString();
//	}
};

//////////////////////////////////////////////////////////////////////////////
// CPopulation                                                              //
//////////////////////////////////////////////////////////////////////////////

class CPopulation
: public CAliasChild<CPopulationOwner>
, public NLMISC::CRefCount
{
public:
	CPopulation(CPopulationOwner* owner, CAIAliasDescriptionNode* aliasDescription = NULL);
	CPopulation(CPopulationOwner* owner, uint32 alias, std::string name);
	
	std::string getIndexString() const;
	
	void setSpawnType(AITYPES::TSpawnType spawnType) { _SpawnType = spawnType; }
	AITYPES::TSpawnType const& getSpawnType() { return _SpawnType; };	// always/ day/ night/ etc
	
	void setWeight(uint32 weight) {	_Weight=weight; }
	uint32 const& getWeight() const { return _Weight; }	// random weighting for randomizer
	
	void addPopRecord(CPopulationRecord popRecord);
	
	// passing self off as a vector of CPopulationRecord (to avoid re-writing code)
	CPopulationRecord& operator[](size_t idx) const { return const_cast<CPopulationRecord &>(_PopRecords[idx]); }
	size_t size() const { return _PopRecords.size(); }
	
private:
	std::vector<CPopulationRecord> _PopRecords;
	uint32				_Weight;		// random weighting for randomizer
	AITYPES::TSpawnType	_SpawnType;		// always/ day/ night/ etc
};

//////////////////////////////////////////////////////////////////////////////
// CAICircle                                                                //
//////////////////////////////////////////////////////////////////////////////

/// Description of a circle with coordinate and radius
class CAICircle
{
public:
	CAICircle() { }
	template <class V, class R>
	CAICircle(V const& center, R radius)
	: _Center(center)
	, _Radius(radius)
	{
	}
	
	template <class V>
	bool isInside(V const& pos)
	{
		return (pos - _Center).sqrnorm() <= (_Radius+1)*(_Radius+1);
	}
	
	CAIVector const& center() const { return _Center; }
	double const& radius() const { return _Radius; }
	
private:
	CAIVector	_Center;
	double		_Radius;
};

//////////////////////////////////////////////////////////////////////////////
// CAabb                                                                    //
//////////////////////////////////////////////////////////////////////////////

/// A simple AABB class
class CAabb
{
public:
	CAabb();
	
	template <class V>
	CAabb(std::vector<V> const& coords)
	: _VMax(INT_MIN/CAICoord::UNITS_PER_METER, INT_MIN/CAICoord::UNITS_PER_METER)
	, _VMin(INT_MAX/CAICoord::UNITS_PER_METER, INT_MAX/CAICoord::UNITS_PER_METER)
	{
		for (uint k=0; k<coords.size(); ++k)
			includePoint(coords[k]);
	}
	
	template <class V>
	void includePoint(V const& point)
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
	void includeAabb(CAabb const& box);
	
	void init();
	
	template<class V>
	bool isInside(V const& v)
	{
		return v.x() >= _VMin.x() && v.y() >= _VMin.y() && v.x() <= _VMax.x() && v.y() <= _VMax.y();
	}
	
	CAIVector const& vmin() const { return _VMin; }
	CAIVector const& vmax() const { return _VMax; }
	
private:
	CAIVector _VMin;
	CAIVector _VMax;
};


//////////////////////////////////////////////////////////////////////////////
// CBaseZone                                                                //
//////////////////////////////////////////////////////////////////////////////

/* Contains properties common to all zones 
 * (fauna zones, nps zones ...)
 */
class CBaseZone
{
public:
	typedef CTmpPropertyZone::TTarget TZoneType;
	// Add a new Id identifying a substitution CGroupFamily that should be use instead
	// of standard CGroupFamily (those defined in CRegion), when spawning fauna on that zone
	// Substitution occurs when one or more subtitution id have been added
	void addSubstitutionGroupFamilyId(size_t id) { nlassert(id != 0); _SubstitutionGroupFamilyIds.insert(id); }
	void removeSubstitutionGroupFamilyId(size_t id) { nlassert(id != 0); _SubstitutionGroupFamilyIds.erase(id); }
	bool isSubstituted() const { return !_SubstitutionGroupFamilyIds.empty(); }
	bool isSubsitutedForGroupFamily(size_t id) const { return _SubstitutionGroupFamilyIds.count(id) != 0; }	
	virtual TZoneType getZoneType() const = 0;
	virtual AITYPES::CPropertySet& additionalActivities() = 0;
	virtual AITYPES::CPropertySet const& additionalActivities () const = 0;
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const = 0;
private:
	std::set<size_t>      _SubstitutionGroupFamilyIds;
};


//////////////////////////////////////////////////////////////////////////////
// CFaunaZone                                                               //
//////////////////////////////////////////////////////////////////////////////

/// Zone for fauna activity
class CFaunaZone
: public CBaseZone
, public NLMISC::CDbgRefCount<CFaunaZone>
, public CAIPlaceXYR
{
public:
	CFaunaZone(CCell *owner, CAIAliasDescriptionNode *adn);
	~CFaunaZone();
	
	CCell* getOwner() const;

	bool haveActivity(AITYPES::CPropertyId const& activity) const;
	bool haveActivity(AITYPES::CPropertySet const& activities) const;
	float getFreeAreaScore() const;
	
	AITYPES::CPropertySet& initialActivities() { return _InitialActivities; }
	AITYPES::CPropertySet const& initialActivities() const { return _InitialActivities; }
	
	virtual AITYPES::CPropertySet& additionalActivities() { return _AdditionalActivities; }
	virtual AITYPES::CPropertySet const& additionalActivities () const { return _AdditionalActivities; }
		

	virtual TZoneType getZoneType() const { return CTmpPropertyZone::Fauna; }

	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const { return CAIPlaceXYR::worldValidPos(); }

private:
	uint32 getNbUse() const;
private:	
	AITYPES::CPropertySet _InitialActivities;
	AITYPES::CPropertySet _AdditionalActivities;
};

//////////////////////////////////////////////////////////////////////////////
// CAIRefPlaceXYR                                                           //
//////////////////////////////////////////////////////////////////////////////
class CAIRefPlaceXYR
: public CAIPlace	
{
public:
	CAIRefPlaceXYR(CPlaceOwner*	owner, CAIPlace const* zone);
	
	operator CAIPlace const*() const;
	
	NLMISC::CSmartPtr<CAIPlace const> const& getZone() const { return _Zone; }
	CAIPos const& midPos() const { return _Zone->midPos(); }
	RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const { return _Zone->worldValidPos(); }
	float getRadius() const { return _Zone->getRadius(); }
	
	bool atPlace(CAIVectorMirror const& pos) const { return _Zone->atPlace(pos); }
	bool atPlace(CAIVector const& pos) const { return _Zone->atPlace(pos); }
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition& pos) const { _Zone->getRandomPos(pos); }
	AITYPES::TVerticalPos getVerticalPos() const { return _Zone->getVerticalPos(); }
	void display(CStringWriter& stringWriter) const { _Zone->display(stringWriter); }
	
private:
	NLMISC::CSmartPtr<CAIPlace const> _Zone;
};

//////////////////////////////////////////////////////////////////////////////
// CAIRefPlaceXYRFauna                                                      //
//////////////////////////////////////////////////////////////////////////////
// Just made to track ref count CFaunaZone ..
class CAIRefPlaceXYRFauna
: public CAIRefPlaceXYR,
  public CFaunaGenericPlace
{
public:
	CAIRefPlaceXYRFauna(CPlaceOwner*	owner, CAIPlace const* zone);	
};




//////////////////////////////////////////////////////////////////////////////
// CNpcZone                                                                 //
//////////////////////////////////////////////////////////////////////////////

/// Zone for npc activity                                                    //
class CNpcZone
: public CBaseZone
, public NLMISC::CDbgRefCount<CNpcZone>
, public virtual NLMISC::CVirtualRefCount
{
public:
	virtual ~CNpcZone();
	void unlinkNpcZone();

	/// @name Abstract interface
	//@{
	virtual CCell* getOwner() const = 0;
	virtual const CAliasTreeOwner& getAliasTreeOwner() const = 0;
	virtual uint32 getIndex() const = 0;
	
	virtual CPlaceRandomPos& getPlaceRandomPos() = 0;
	virtual const CPlaceRandomPos& getPlaceRandomPos() const = 0;
	
	/// Test whether a position is inside the zone
	virtual bool atPlace(const CAIVector &pos) const = 0;
	virtual bool atPlace(const CAIVectorMirror &pos) const = 0;
	/// Return the position of the middle of the zone
	virtual const CAIPos &midPos() const = 0;
	
	/// Returns the area of the zone
	virtual float getArea() const = 0;
	//@}

	/// Returns the number of times this zone is occupied
	uint32 getNbUse() const;
	
	/// Returns a score corresponding to the empty space
	virtual float getFreeAreaScore() const;

	std::string	getIndexString() const;
		
	void unrefZoneInRoads();
	
	/// @name Accessors
	//@{
	AITYPES::CPropertySet& properties();
	const AITYPES::CPropertySet& properties() const;
	std::vector<NLMISC::CDbgPtr<CRoad> >& roads();
	//@}

	virtual TZoneType getZoneType() const { return CTmpPropertyZone::Npc; }

	virtual AITYPES::CPropertySet& additionalActivities() { return _Properties; }
	virtual AITYPES::CPropertySet const& additionalActivities () const { return _Properties; }
	
private:
	AITYPES::CPropertySet _Properties;
	/// The list of road starting from this zone.
	std::vector<NLMISC::CDbgPtr<CRoad> > _Roads;
};

// :TODO: check if that inlining is necessary
//inline void CNpcZone::unrefZoneInRoads(); // See at end of this file

//////////////////////////////////////////////////////////////////////////////
// CNpcZonePlace                                                            //
//////////////////////////////////////////////////////////////////////////////

class CNpcZonePlace
: public CNpcZone
, public CPlaceRandomPos
, public CAIPlace
{
public:
	CNpcZonePlace(CCell* owner, CAIAliasDescriptionNode* adn);
	~CNpcZonePlace();

	/// @name CNpcZone implementation
	//@{
	CCell* getOwner() const;
	CAliasTreeOwner const& getAliasTreeOwner() const;
	uint32	getIndex()	const;

	CPlaceRandomPos& getPlaceRandomPos();
	CPlaceRandomPos const& getPlaceRandomPos() const;
	
	virtual bool atPlace(const CAIVector &pos) const;
	virtual bool atPlace(const CAIVectorMirror &pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	
	virtual CAIPos const& midPos() const;
	
	virtual float getArea() const;
	
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const;
	
	virtual float getRadius() const;
	
	virtual void display(CStringWriter	&stringWriter) const;
	
	virtual AITYPES::TVerticalPos getVerticalPos() const;
	virtual void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition &pos) const;
	//@}
	
	void setPosAndRadius(AITYPES::TVerticalPos verticalPos, const CAIPos& pos, uint32 radius);
	
private:
	
	bool calcRandomPos(CAIPos& pos) const;
	
private:
	RYAI_MAP_CRUNCH::CWorldPosition	_worldValidPos;
	CAIPos _pos;
	float _radius;	
};

//////////////////////////////////////////////////////////////////////////////
// CNpcZonePlaceNoPrim                                                      //
//////////////////////////////////////////////////////////////////////////////

class CNpcZonePlaceNoPrim
: public virtual NLMISC::CVirtualRefCount
{
public:
	CNpcZonePlaceNoPrim();
	~CNpcZonePlaceNoPrim();
	
	/// @name CNpcZone implementation
	//@{
	virtual bool atPlace(const CAIVector &pos) const;
	virtual bool atPlace(const CAIVectorMirror &pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	
	virtual CAIPos const& midPos() const;
	
	virtual float getArea() const;
	
	virtual RYAI_MAP_CRUNCH::CWorldPosition const& worldValidPos() const;
	
	virtual float getRadius() const;
	
	virtual void display(CStringWriter	&stringWriter) const;
	
	virtual AITYPES::TVerticalPos getVerticalPos() const;
	virtual void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition &pos) const;
	virtual uint getRandomPosCount() const { return 1; }
	//@}
	
	void setPosAndRadius(AITYPES::TVerticalPos verticalPos, const CAIPos& pos, uint32 radius);
	
private:
	
	bool calcRandomPos(CAIPos& pos) const;
	
private:
	RYAI_MAP_CRUNCH::CWorldPosition	_WorldValidPos;
	CAIPos					_Pos;
	float					_Radius;
	AITYPES::TVerticalPos	_VerticalPos;
};

//////////////////////////////////////////////////////////////////////////////
// CNpcZoneShape                                                            //
//////////////////////////////////////////////////////////////////////////////

class CNpcZoneShape
: public CNpcZone
, public CAIPlace
{
public:
	CNpcZoneShape(CCell* owner, CAIAliasDescriptionNode* adn);
	~CNpcZoneShape();
	
	/// @name CNpcZone implementation
	//@{
	virtual CCell* getOwner() const;
	virtual CAliasTreeOwner const& getAliasTreeOwner() const;
	virtual uint32 getIndex() const;
	
	virtual CPlaceRandomPos& getPlaceRandomPos();
	virtual CPlaceRandomPos const& getPlaceRandomPos() const;
	
	virtual bool atPlace(const CAIVector& pos) const;
	virtual bool atPlace(const CAIVectorMirror &pos) const;
	virtual bool atPlace(CAIEntityPhysical const* entity) const;
	virtual CAIPos const& midPos() const;
	
	virtual float getArea() const;
	
	virtual const RYAI_MAP_CRUNCH::CWorldPosition& worldValidPos() const;
	
	virtual float getRadius() const;
	
	virtual void display(CStringWriter	&stringWriter) const;
	
	virtual AITYPES::TVerticalPos getVerticalPos() const;
	virtual void getRandomPos(RYAI_MAP_CRUNCH::CWorldPosition &pos) const;
	//@}
	
	void setPatat(AITYPES::TVerticalPos verticalPos, const std::vector<CAIVector>& points);
	
private:
	void buildMidPos();
	
private:
	RYAI_MAP_CRUNCH::CWorldPosition	_worldValidPos;
	CShape _shape;
	CAIPos _midPos;
};

//////////////////////////////////////////////////////////////////////////////
// CLazyProcess                                                             //
//////////////////////////////////////////////////////////////////////////////

class CLazyProcess
: public NLMISC::CRefCount
{
public:
	virtual ~CLazyProcess() {}
	/// Used to update some dependencies for instance road connections when the first next update of CContinent is called.
	virtual void update() const = 0;
	/// Used to check if there's no need to add another lazyprocess.
	virtual bool absorb(CLazyProcess const& lazyProcess) const = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CRebuildContinentAndOutPost                                              //
//////////////////////////////////////////////////////////////////////////////

class CRebuildContinentAndOutPost
: public CLazyProcess
{
public:
	CRebuildContinentAndOutPost(CContinent* continent);
	
protected:
	void update () const;
	bool absorb(CLazyProcess const& lazyProcess) const;
	bool operator==(CRebuildContinentAndOutPost const& other) const;
	
private:
	CContinent* _Continent;
};

//////////////////////////////////////////////////////////////////////////////
// CContinent                                                               //
//////////////////////////////////////////////////////////////////////////////

/// Store regions and spead energy level.
class CContinent
: public NLMISC::CDbgRefCount<CContinent>
, public CChild<CAIInstance>
, public NLMISC::CRefCount
, public CServiceEvent::CHandler
, public CAIEntity
{
public:
	CContinent(CAIInstance* owner);
	
	virtual ~CContinent();
	
	CAIInstance* getAIInstance() const { return getOwner(); }

	CAliasCont<CRegion>& regions() { return _Regions;}
	CAliasCont<COutpost>& outposts() { return _Outposts;}
	CAliasCont<COutpost> const& outposts() const { return _Outposts;}

	CNpcZone* findNpcZone(CAIVector const& posInside);
	
	/// Tag all the regions loaded from fileId to delete
	bool markTagForDelete(NLMISC::TStringId fileId);
	/// Delete all region that are taged
	bool deleteTaggedAlias(NLMISC::TStringId fileId);
	
	void rebuildBoundingBox();
	/// Update the continent
	void	update();
	
	void pushLazyProcess(NLMISC::CSmartPtr<CLazyProcess> lazyProcess);
	void updateLazyProcess();
	
	// Some service are up, need to send them data?
	void serviceEvent(CServiceEvent	const& info);
	
	bool spawn();
	bool despawn();
	
	std::string getFullName() const { return getName(); }	
	std::string getIndexString() const;
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	
	void setName(std::string const& name) { _ContinentName = name; }
	
	std::string getName() const { return _ContinentName; }

	/// Bounding box info.
	CAabb _BoundingBox;
	typedef	std::vector<NLMISC::CSmartPtr<CLazyProcess> > TLazyProcessList;
	TLazyProcessList _LazyProcess;
	
private:
	/// @name AI service hierarchy
	//@{
	/// The set of regions stored in this continent
	CAliasCont<CRegion> _Regions;
	/// The set of outpost stored in this continent
	CAliasCont<COutpost> _Outposts;
	//@}
	
	/// Name of the continent
	std::string _ContinentName;
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
};

//////////////////////////////////////////////////////////////////////////////
// CRegion                                                                  //
//////////////////////////////////////////////////////////////////////////////

/// Store cell zones, groups templates and roads.
class CRegion
: public CAliasChild<CContinent>
, public NLMISC::CRefCount
, public CAliasTreeRoot
, public CServiceEvent::CHandler
, public CAIEntity
{
public:
	CRegion(CContinent* owner, uint32 alias, std::string const& name, std::string const& filename);
	virtual ~CRegion();
	
	CAIInstance* getAIInstance() const { return getOwner()->getAIInstance(); }
	
	CAliasCont<CCellZone>& cellZones() { return _CellZones; }
	
	CAliasCont<CGroupFamily>& groupFamilies() { return _GroupFamilies; }
	
	virtual std::string getIndexString() const;
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	virtual std::string getFullName() const;
	
	/// Rebuild the bounding box
	void rebuildBoundingBox();
	/// Rebuild the link road->zone and conectivity graph for cells
	void rebuildConnectivity();
	
	/// Update the region
	void update();
	void serviceEvent(CServiceEvent	const& info);
	
	bool spawn();
	bool despawn();
	
	/// Bounding box info.
	CAabb _BoundingBox;
	
private:
	/// @name AI service hierarchy
	//@{
	/// Container for owned cellzones
	CAliasCont<CCellZone> _CellZones;
	/// Container for owner group descs
	CAliasCont<CGroupFamily> _GroupFamilies;
	//@}
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
};

//////////////////////////////////////////////////////////////////////////////
// CRoad                                                                    //
//////////////////////////////////////////////////////////////////////////////

/// Road for npc moving.
class CRoad
: public	NLMISC::CDbgRefCount<CRoad>
, public	NLMISC::CRefCount
, public	CAliasChild<CCell>
{
public:
	CRoad(CCell* owner, uint32 alias, std::string const& name);
	~CRoad();
	
	std::string getIndexString() const;
	
	CAIVector const& getLogicStart() const { return _Start; }
	CAIVector const& getLogicEnd() const { return	_End; }
	AITYPES::TVerticalPos const& verticalPos() const { return _VerticalPos; }
	void setVerticalPos(AITYPES::TVerticalPos const& vertPos) { _VerticalPos = vertPos; }
	std::vector<RYAI_MAP_CRUNCH::CWorldPosition> const& coords() const { return _Coords; }
	void clearCoords() { _Coords.clear(); }
	
	NLMISC::CDbgPtr<CNpcZone> const& startZone() const { return _StartZone; }
	NLMISC::CDbgPtr<CNpcZone> const& endZone() const { return _EndZone; }
	
	bool const& startExternal()	const { return _StartExternal; }
	
	bool const& endExternal() const { return _EndExternal; }
	
	void setStartZone(CNpcZone const* npcZone);
	void setEndZone(CNpcZone const* npcZone);
	
	void unlinkRoad();
	
	void calcLength();
	
	void setDifficulty(float const& difficulty);
	
	void calCost();
	
	// Pathfinding property.
	float getCost() const;
	
	CAliasCont<CRoadTrigger>& triggers() { return _RoadTriggers; }
	
	void setPathPoints(AITYPES::TVerticalPos verticalPos, std::vector<CAIVector> const& points);
	
	std::set<CNpcZone*>	 AllZones;
	
private:
	/// @name AI service hierarchy
	//@{
	CAliasCont<CRoadTrigger> _RoadTriggers;
	//@}
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	
	/// The vertical pos
	AITYPES::TVerticalPos _VerticalPos;
	/// The path of the road
	std::vector<RYAI_MAP_CRUNCH::CWorldPosition> _Coords;
	/// Pointer to the zone at start of road 
	NLMISC::CDbgPtr<CNpcZone> _StartZone;
	/// Flag that indicate the zone is outside the region
	bool _StartExternal;
	/// Pointer to the zone at end of road 
	NLMISC::CDbgPtr<CNpcZone> _EndZone;
	/// Flag that indicate the zone is outside the region
	bool _EndExternal;
	
	// Data for path finding
	/// length of the road segment
	float _Length;
	/// Difficulty of the road
	float _Difficulty;
	
	float _CostCoef;
	
	CAIVector _Start;
	CAIVector _End;
};

//////////////////////////////////////////////////////////////////////////////
// CRoadTrigger                                                             //
//////////////////////////////////////////////////////////////////////////////

class CRoadTrigger
: public CAliasChild<CRoad>
, public NLMISC::CRefCount
{
public:
	CRoadTrigger(CRoad* owner, uint32 alias, std::string const& name);
	
	std::string getIndexString() const;
	
	CAICircle _Trigger1;
	CAICircle _Trigger2;
	CAICircle _Spawn;
};

//////////////////////////////////////////////////////////////////////////////
// CZoneScorer                                                              //
//////////////////////////////////////////////////////////////////////////////

class CZoneScorer
{
public:
	virtual ~CZoneScorer() { }	
	virtual float getScore(CNpcZone	const& zone) const = 0;
	virtual float getParam(CNpcZone	const& zone) const { return 0.f; };
};

//////////////////////////////////////////////////////////////////////////////
// CCellZone                                                                //
//////////////////////////////////////////////////////////////////////////////

/// Container for cell and energy levels.
class CCellZone
: public CAliasChild<CRegion>
, public NLMISC::CRefCount
, public CServiceEvent::CHandler
, public CAIEntity
{
public:
	CCellZone(CRegion* owner, uint32 alias, std::string const& name);
	~CCellZone();
	
	CAIInstance* getAIInstance() const { return getOwner()->getAIInstance(); }
	
	CAliasCont<CCell>& cells() { return _Cells; }
	
	void unrefZoneInRoads();
	
	std::string getIndexString() const;
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	virtual std::string getFullName() const;
	
	void rebuildEnergyLevels();
	void update();
	void serviceEvent(CServiceEvent const& info);
	
	bool findRestAndFoodFaunaZoneInCellList(CFaunaZone const*& zoneRest, AITYPES::CPropertySet const& rest, CFaunaZone const*& zoneFood, AITYPES::CPropertySet const& food, std::vector<CCell*> const& cells, RYAI_MAP_CRUNCH::TAStarFlag denyflags);
		
	 
	CFaunaZone const* lookupFaunaZone(AITYPES::CPropertySet const& activity, RYAI_MAP_CRUNCH::TAStarFlag denyflags, size_t replacementGroupFamilyId = 0);
	const CNpcZone *lookupNpcZone(AITYPES::CPropertySet const& activity, size_t replacementGroupFamilyId = 0);
	static CNpcZone const* lookupNpcZoneScorer(std::vector<CCell*> cells, CZoneScorer const& scorer);
	
	const CNpcZone* lookupNpcZoneByName(std::string const& zoneName);
	
	CCont<CFamilyBehavior>& familyBehaviors() { return _Families; }
	
	bool spawn();
	bool despawn();
	
	/// Bouding box surrounding all the contained cells.
	CAabb _BoundingBox;
	
	/// @name AI service hierarchy
	//@{
public:
	/// The family behavior container
	CCont<CFamilyBehavior> _Families;
private:
	CAliasCont<CCell> _Cells;
	//@}
	
private:
	// overloads for CAliasChild virtuals
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
};

//////////////////////////////////////////////////////////////////////////////
// CCell                                                                    //
//////////////////////////////////////////////////////////////////////////////

/** Cell definition.
 *	Contains npc and faune zones.
 */
typedef CAliasCont<CNpcZonePlace> TAliasZonePlaceList;
typedef CAliasCont<CNpcZoneShape> TAliasZoneShapeList;

class CCell
: public NLMISC::CDbgRefCount<CCell>
, public NLMISC::CRefCount
, public CAliasChild<CCellZone>
, public CPlaceOwner
{
public:
	CCell(CCellZone* owner, uint32 alias, std::string const& name);
	~CCell();
	
	void unlinkCell();
	
	CAliasCont<CRoad>& roads() { return _Roads; }
	
	void connectRoads();
	
	CAliasCont<CFaunaZone>& faunaZones() { return _FaunaZones; }	
	CAliasCont<CFaunaZone> const& faunaZonesCst() const { return _FaunaZones; }
	
	CNpcZone* findNpcZone(CAIVector const& posInside);	
		
	size_t npcZoneCount();
	CNpcZone* npcZone(size_t index);
	TAliasZonePlaceList& npcZonePlaces() { return _NpcZonePlaces; }
	TAliasZoneShapeList& npcZoneShapes() { return _NpcZoneShapes; }
	TAliasZonePlaceList	const& npcZonePlacesCst() const { return _NpcZonePlaces; }
	TAliasZoneShapeList const& npcZoneShapesCst() const { return _NpcZoneShapes; }

	AITYPES::CPropertySet const& properties() const { return _Properties; }
	AITYPES::CPropertySet& properties() { return _Properties; }
	
	void getNeighBourgCellList(std::vector<CCell*>& cells) const;
	
	void unrefZoneInRoads();

	virtual std::string getIndexString() const;
	virtual std::string getFullName() const;
	
	/// The coordinate of the polygon surounding the cell
	std::vector<CAIVector> _Coords;
	/// The liste of flag for family that can come in this cell
	
	AITYPES::CPropertySet _Properties;
	
	/// The list of neighbour cells
	std::set<NLMISC::CDbgPtr<CCell> > _NeighbourCells;
	
	CAabb _BoundingBox;
	
private:
	/// @name AI service hierarchy
	//@{
	CAliasCont<CFaunaZone> _FaunaZones;
	
	/// Container for owned roads
	CAliasCont<CRoad> _Roads;
	//@}
	
	TAliasZonePlaceList _NpcZonePlaces;
	TAliasZoneShapeList _NpcZoneShapes;
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	
//	std::string	getPlaceOwnerFullName()	const { return getFullName(); }
//	std::string	getPlaceOwnerIndexString() const { return getIndexString(); }
};

//////////////////////////////////////////////////////////////////////////////
// CAggroGroupContainer                                                     //
//////////////////////////////////////////////////////////////////////////////

class CAggroGroupContainer
: public NLMISC::CVirtualRefCount
{
public:
	std::vector<uint32> aggroGroupIds;
};
	

class CZoneMarker;

//////////////////////////////////////////////////////////////////////////////
// CGroupFamily                                                             //
//////////////////////////////////////////////////////////////////////////////

class CGroupFamily
: public NLMISC::CDbgRefCount<CGroupFamily>
, public NLMISC::CRefCount
, public CAliasTreeOwner
, public CLevelEnergy
{
public:	    	
	CGroupFamily(CAliasTreeOwner *owner, uint32 alias, std::string const& name);
	virtual	~CGroupFamily();

	CAliasTreeOwner *getOwner() const { return _Owner; } 
	
	void setFamilyTag(std::string const& familyTag) { _FamilyTag = AITYPES::CPropertyId(familyTag); }
	AITYPES::CPropertyId const& getFamilyTag() const { return _FamilyTag; }
	
	void setProfileName(std::string const& profileName) { _ProfileName = NLMISC::CStringMapper::map(profileName); }
	
	typedef	std::map<NLMISC::TStringId, NLMISC::CSmartPtr<NLMISC::CVirtualRefCount> > TParamsList;
	TParamsList	_Params;

	void setProfileParams(std::string const& str, NLMISC::CVirtualRefCount* objet);
	
	NLMISC::CVirtualRefCount const* getProfileParams(std::string const& str) const;
	
	CAliasCont<CGroupDesc<CGroupFamily> >& groupDescs() { return _GroupDescs; }

	/*
	std::string getFullName() const { return std::string(getOwner()->getFullName() +":"+ getName()); }
	
	std::string getIndexString() const { return getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex()); }
	*/

	
	std::string getFullName() const { return "???:" + getName(); }	
	std::string getIndexString() const { return "???" + NLMISC::toString(":%u", getChildIndex()); }

	// mimic CChild behaviour
	void setChildIndex(uint32 index)
	{
		_Index = index;
	}
	
	uint32 getChildIndex() const
	{
		return _Index;
	}
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	
	CGroupDesc<CGroupFamily> const* getProportionalGroupDesc(CFamilyBehavior const* const familyBehavior, AITYPES::CPropertySet const& needActFlag, AITYPES::CPropertySet const& maskActFlag);
	
	NLMISC::TStringId const& profileName() const { return _ProfileName; }
	
	typedef	std::map<NLMISC::TStringId, AITYPES::CPropertySet> TActivityList;
	
	void addProfileProperty(std::string const& propertyName, AITYPES::CPropertySet const& property);
	AITYPES::CPropertySet const& getProfileProperty(std::string const& propertyName) const;
	

	// mark this GroupFamily as a substitution group. An id of 0 means this group is not a substitution group (which is the default)
	// subtitution GroupFamily will shadow standard GroupFamily group at update 
	// for the fauna zones that have the same id than this group in their subtitution id list.
	// (if the subtitution list is empty then the fauna zone GroupFamilies are not substituted)
	void	setSubstitutionId(size_t substitutionId) { _SubstitutionId = substitutionId; }	
	size_t  getSubstitutionId() const { return _SubstitutionId; }

	void    addZoneMarkerRef(CZoneMarker *zm) { _ZoneMarkerRef.insert(zm); }
	void    removeZoneMarkerRef(CZoneMarker *zm) { _ZoneMarkerRef.erase(zm); }
	bool    hasZoneMarkerRef() const { return !_ZoneMarkerRef.empty(); }

	virtual std::string	getOneLineInfoString() const { return std::string("Group family '") + getName() + "'"; }
	
private:
	/// @name AI service hierarchy
	//@{
	CAliasCont<CGroupDesc<CGroupFamily> >	_GroupDescs;
	//@}
	
	TActivityList			_Properties;
	NLMISC::TStringId		_ProfileName;
	// If the family is a tribe, the property describe the name of the family (witch is the tribe name for tribe family)
	AITYPES::CPropertyId	_FamilyTag;	
	size_t					_SubstitutionId;	
	uint32					_Index;
	CAliasTreeOwner			*_Owner;
	std::set<CZoneMarker *> _ZoneMarkerRef;
};



//////////////////////////////////////////////////////////////////////////////
// CGroupDesc                                                               //
//////////////////////////////////////////////////////////////////////////////

class IGroupDesc
: public NLMISC::CRefCount
{
public:
	virtual ~IGroupDesc() {}
	virtual uint32 groupEnergyValue() const = 0;
	virtual float groupEnergyCoef() const = 0;
	virtual bool getCountMultiplierFlag() const = 0;
	virtual uint32 getWeightForEnergy(size_t levelIndex) const = 0;
	virtual bool isValidForSeason(EGSPD::CSeason::TSeason const& season) const = 0;
	virtual AITYPES::CPropertySet& properties() = 0;
	virtual AITYPES::CPropertySet const& properties() const = 0;
	virtual std::string getFullName() const = 0;
};

/// Description of a group template.
template <class FamilyT>
class CGroupDesc
: public NLMISC::CDbgRefCount<CGroupDesc<FamilyT> >
, public CAliasChild<FamilyT>
, public IGroupDesc
{
public:
	CGroupDesc(FamilyT* owner, uint32 alias, std::string const& name);
	
	std::string getIndexString() const;
	
	CAliasCont<CBotDesc<FamilyT> >& botDescs() { return _BotDescs; }
	CAliasCont<CBotDesc<FamilyT> > const& botDescs() const { return	_BotDescs; }
	
	virtual std::string getFullName() const { return std::string(this->getOwner()->getFullName() +":"+ this->getName()); }
	
	virtual bool isValidForSeason(EGSPD::CSeason::TSeason const& season) const { return	_SeasonFlags[season]; }
	
	bool isValidForDayOrNight(bool const& isDay) const;
	
	virtual AITYPES::CPropertySet& properties() { return _Properties; }
	virtual AITYPES::CPropertySet const& properties() const { return _Properties; }
	
	void setSheet(AISHEETS::ICreatureCPtr const& sheetPtr);
	
	bool setSheet(std::string const& sheetName);
	
	AISHEETS::ICreatureCPtr sheet(sint32 baseLevel = -1) const;
	
	virtual uint32 groupEnergyValue() const { return _GroupEnergyValue; }
	
	uint32 calcTotalEnergyValue() const;
	
	AITYPES::TSpawnType	const& spawnType() const { return _SpawnType; }
	
	// Return the bot count as specified in the primitives data
	uint32 getBaseBotCount() const { return _BotCount; }
	
	// Set the bot count as specified in the primitives data
	void setBaseBotCount(uint32 const& botCount) { _BotCount = botCount; }
	
	// Return the 'real' bot count, ie the bot count multiplied by the creature sheet multiplier.
	// This is the number of bot we realy want to spawn.
	uint32 getRealBotCount() const;
	
	void setCountMultiplierFlag(bool countMultiplier) { _CountMultiplier = countMultiplier; }
	virtual bool getCountMultiplierFlag() const { return _CountMultiplier; }
	std::vector<std::string> const& grpParameters() const { return _GrpParameters; }
	std::vector<std::string>& grpParameters() { return _GrpParameters; }
	std::vector<std::string> const& botEquipment() const { return _BotEquipment; }
	std::vector<std::string>& botEquipment() { return _BotEquipment; }
	
	void setSeasonFlags(bool const seasonFlags[4]);
	void setWeightLevels(uint32 const weights[4]);
	
	void setSpawnType(AITYPES::TSpawnType const& spawnType) { _SpawnType = spawnType; }
	
	void setGroupEnergyCoef(float coef) { _EnergyCoef = coef; }
	
	virtual float groupEnergyCoef() const { return _EnergyCoef; }
	
	CGrpFauna* createFaunaGroup(CFamilyBehavior* familyBehavior) const;
	
	CGroupNpc* createNpcGroup(CMgrNpc* mgr, CAIVector const& pos, double dispersionRadius = 0., sint32 baseLevel = -1, bool spawnBots = true) const;
	
	virtual uint32 getWeightForEnergy(size_t levelIndex) const { return _WeightLevel[levelIndex]; }

	std::vector<CPopulationRecord>& populationRecords() { return _PopulationRecords; }

	// Don't really like this but have no time to do it on a member.
	sint getNbUse() const;
	
	bool getGDPlayerAttackable() const { return _PlayerAttackable; };
	void setGDPlayerAttackable(bool playerAttackable) const { _PlayerAttackable = playerAttackable; }
	
	bool getGDBotAttackable() const { return _BotAttackable; };
	void setGDBotAttackable(bool botAttackable) const { _BotAttackable = botAttackable; }
	void setMultiLevel(bool multiLevel) { _MultiLevel = multiLevel; }
	void setLevelDelta(sint32 levelDelta) { _LevelDelta = levelDelta; }
	sint32 getLevelDelta() const { return _LevelDelta; }
	bool isMultiLevel() const { return _MultiLevel; }

private:
	/// @name AI service hierarchy
	//@{
	/// Container for named bot.
	CAliasCont<CBotDesc<FamilyT> > _BotDescs;
	//@}
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	
	/// The number of bot (for unamed bots)
	uint32 _BotCount;
	// If true, the _BotCount is multiplied by the quantity specified in the creature sheet.
	bool _CountMultiplier;

	// If the group is multilevel it has 20 sheets, else only one
	static size_t const _MultiLevelSheetCount;
	bool _MultiLevel;
	AISHEETS::ICreatureCPtr _Sheet;
	std::vector<AISHEETS::ICreatureCPtr> _MultiLevelSheets;
	sint32 _LevelDelta;
	
	/// The raw groups parameters
	std::vector<std::string> _GrpParameters;
	/// The raw bot equipment.
	std::vector<std::string> _BotEquipment;
	/// The season flags
	bool _SeasonFlags[4];

	uint32 _WeightLevel[4];

	/// The energy value of the group
	uint32 _GroupEnergyValue;

	/// Activity flags (for npcs)
	AITYPES::CPropertySet _Properties;

	/// Spawn type (for fauna)
	AITYPES::TSpawnType _SpawnType;
	// Energy Coef for bot of this group.
	float _EnergyCoef;
	
	std::vector<CPopulationRecord> _PopulationRecords;
	
	/** Flag to set the group attackable by players.
	 *	NB : this flag is set from the family code, not from the
	 *	primitives datas
	 */
	mutable bool _PlayerAttackable;
	// Flag to set the group attackable by other npcs
	mutable bool _BotAttackable;
};

//////////////////////////////////////////////////////////////////////////////
// CBotDesc                                                                 //
//////////////////////////////////////////////////////////////////////////////

class IBotDesc {
};

/// Description of a bot
template <class FamilyT>
class CBotDesc
: public CAliasChild<CGroupDesc<FamilyT> >
, public NLMISC::CRefCount
, public IBotDesc
{
public:
	CBotDesc(CGroupDesc<FamilyT>* owner, uint32 alias, std::string const& name);
	
	std::string getIndexString() const;
	
	void setSheet(AISHEETS::ICreatureCPtr const& sheetPtr)
	{
	#ifdef NL_DEBUG
		nlassert(sheetPtr);
	#endif
		_Sheet = sheetPtr;
	}

	void setSheet(std::string const& sheetName);
	
	AISHEETS::ICreatureCPtr sheet(sint32 baseLevel = -1) const;
	
	std::vector<std::string>& equipement() { return _Equipment; }
	
	uint32 energyValue() const;
	void setMultiLevel(bool multiLevel) { _MultiLevel = multiLevel; }
	void setLevelDelta(sint32 levelDelta) { _LevelDelta = levelDelta; }
	sint32 getLevelDelta() const { return _LevelDelta; }
	void setUseSheetBotName(bool useSheetBotName) { _UseSheetBotName = useSheetBotName; }
	
	std::string const& getBotName() const;
	
private:
	/// Raw equipement
	std::vector<std::string> _Equipment;
	// If the bot is multilevel it has 20 sheets, else only one
	static size_t const _MultiLevelSheetCount;
	bool _MultiLevel;
	AISHEETS::ICreatureCPtr _Sheet;
	std::vector<AISHEETS::ICreatureCPtr> _MultiLevelSheets;
	sint32 _LevelDelta;
	bool _UseSheetBotName;
};

//////////////////////////////////////////////////////////////////////////////
// Free functions                                                           //
//////////////////////////////////////////////////////////////////////////////

bool pathFind(const	CNpcZone	*const	start, const	CNpcZone	*const	end, const	AITYPES::CPropertySet	&zoneFilter, std::vector<NLMISC::CSmartPtr<CRoad> > &path, bool logError=true);

#endif // CONTINENT_H

