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

#ifndef RYAI_GRP_FAUNA_H
#define RYAI_GRP_FAUNA_H

#include "server_share/r2_variables.h"
#include "ai_vision.h"
#include "timer.h"
#include "ai_place_xyr.h"
#include "ai_grp.h"
#include "ai_bot_fauna.h"
#include "owners.h"
#include "state_instance.h"

#include "ai_generic_fight.h"

class CGrpFauna; 
class CSpawnGroupFauna;

#define GROUP_VISION_UPDATE_RADIUS	(80)

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroupFauna                                                         //
//////////////////////////////////////////////////////////////////////////////

/// This class represents a fauna group - it may be specialised at a later date
class CSpawnGroupFauna
: public CSpawnGroup
, public CFightOrganizer
{
public:
	// public enums ----------------------------------------------------
	enum TState
	{
		StateUndefined,
			StateDespawned,
			StateSpawning,
			StateGrazing,
			StateWandering,
			StateResting
	};
	
public:
	CSpawnGroupFauna(CPersistent<CSpawnGroup>& owner, RYAI_MAP_CRUNCH::TAStarFlag denyFlag);
	
	bool isSpawning();
	
	void update();
	
	/// @name CFightOrganizer implementation
	//@{
	virtual void setFight(CSpawnBot* bot, CAIEntityPhysical* ennemy);
	virtual void setHeal(CSpawnBot* bot, CAIEntityPhysical* target);
	virtual void setNoFight(CSpawnBot* bot);
	virtual void setFlee(CSpawnBot* bot, CAIVector& fleeVect);
	virtual void setReturnAfterFight(CSpawnBot* bot);
	//@}
	
	virtual void spawnBots();
	virtual void despawnBots(bool immediately);
	
	//	overrides the init to avoid automatic bot spawn ..
	void spawnBotOfGroup() { }
	
	virtual void despawnGrp(); // critical code (despawn 'this' object).
	
	uint32 getLastUpdate() const { return _LastUpdate; }
	
	//	Update Rate feature.
	int getUpdatePriority() const { return IsRingShard? (_UpdatePriority&7): _UpdatePriority; }
	//	overloading to recalc the priority update rate.
	void recalcUpdatePriorityDeltaAndGroupPos();
	
	CAIPos const& magnetPos() const;
	float magnetRadiusNear() const;
	float magnetRadiusFar() const;
	
	void resetTimer() { _Timer.set(0); }
	
	CGrpFauna& getPersistent();
	
//	std::string buildDebugString(uint idx) const;
	
	void setPlace(int placeIndex);
	
	void incCurrentCycle();
	
	void setCurrentCycle(uint32 cycle);
	
	uint32 getCurrentCycleTime();
	
	CAIPlace* targetPlace() const { return _TargetPlace; }
	
	CPathCont& getPathCont() { return _PathCont; }
	
	CBotFauna* leader() const { return _Leader; }
	
	void setDespawnImmediately(bool immediately) { _DespawnImmediately = immediately; }
	
	bool despawnImmediately() { return _DespawnImmediately; }
	
	void setMustDespawnBots(bool toDespawnBots = true) { _MustDespawnBots=toDespawnBots; }
	
	bool mustDespawnBots() { return _MustDespawnBots; }
	
private:
	uint32 getDt() const { return _DeltaTime; }
	
	CBotFauna* findLeader();	
	
	// Behavior update method.
	void generalUpdate(TState state = StateUndefined);
	
	// Update method for different states.
	void updateSpawning();
	void updateActivity(AITYPES::TProfiles activity);
	
	void checkTimers(); // called at each update to see if the last timer delay has expired (if true calls updateCycles)
	
private:
	CPathCont _PathCont;
	
	NLMISC::CDbgPtr<CAIPlace>	_TargetPlace;	// a dynamic pointer to one of the above places
	
	bool		_ArrivedInZone;	// indicates that we have not reach the new zone
	
	NLMISC::CDbgPtr<CBotFauna>	_Leader;		// the individual in the group who is treated as the leader
	CAITimer		_Timer;
	
	uint32		_CurrentCycle;
	
	int			_UpdatePriority;
	uint32		_LastUpdate;	// gamecycle at which update() last called
	uint32		_DeltaTime;		//	Dt, simply
	
	bool		_MustDespawnBots;		// if its a day group and its night for instance.
	bool		_DespawnImmediately;	// when MustdespawnBots, indicate if we must done it immedialtely or at best when no player can see it
};

//////////////////////////////////////////////////////////////////////////////
// CGrpFauna                                                                //
//////////////////////////////////////////////////////////////////////////////

class CGrpFauna
: public CPlaceOwner
, public CPersistentStateInstance
, public CGroup
, public CDynGrpBase
, public CPopulationOwner
{
public:
	enum
	{
		INVALID_PLACE = CFaunaGenericPlace::INVALID_PLACE
	};
	enum TPlaces
	{
		SPAWN_PLACE = 0,
		EAT_PLACE,
		REST_PLACE
	};
	enum TTime
	{
		SPAWN_TIME = 0,
		EAT_TIME,
		REST_TIME,
		CORPSE_TIME,
		RESPAWN_TIME,
		LAST_TIME
	};
	struct CCycleDef
	{
		CSpawnGroupFauna::TState	_Activity;
		TTime	_Time;
		TPlaces	_Place;
		uint32	_NextCycle;
	};
	
	class Cycle
	{
	public:
		virtual ~Cycle() { }
		friend class CGrpFauna;
		uint16 getPopIndex(int ind)
		{
			return _PopList[ind];
		}
		std::vector<uint16> _PopList;
	};
	
public:
	CGrpFauna(CMgrFauna* mgr, CAIAliasDescriptionNode* aliasTree, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	
	virtual ~CGrpFauna();
	
	RYZOMID::TTypeId getRyzomType() { return RYZOMID::creature; }
	
	CDynGrpBase* getGrpDynBase() { return this; }
	
	//////////////////////////////////////////////////////////////////////////	
	// PersistentStateInstance
	
	CAliasTreeOwner* aliasTreeOwner() { return this; }
	
	CAIS::CCounter& getSpawnCounter();
	
	bool timeAllowSpawn(uint32 popVersion = 12345)	const;
	
	void stateChange(CAIState const* oldState, CAIState const* newState);
	
	CGroup* getGroup() { return this; }
	
	//////////////////////////////////////////////////////////////////////////
	
	virtual void lastBotDespawned();
	virtual void firstBotSpawned();
	
	// debugging stuff
	CDebugHistory* getDebugHistory() { return this; }
	
	CPersistentStateInstance* getPersistentStateInstance() { return this; }
	
	virtual void setEvent(uint eventId);
	virtual void serviceEvent (const CServiceEvent &info);
	
	virtual std::string getFullName() const { return CGroup::getFullName(); }
	virtual std::string getIndexString() const { return CGroup::getIndexString(); }
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	
	NLMISC::CSmartPtr<CSpawnGroup> createSpawnGroup();
	
	CSpawnGroupFauna* getSpawnObj() const { return NLMISC::type_cast<CSpawnGroupFauna*>(CGroup::getSpawnObj()); }
	
	void allocateBots();
	
	// This struct is used to manage dead creatures and their corresponding respawned creature.
	
	// inheritted virtual interface ------------------------------------
	virtual bool spawn();
	bool spawnPop(uint popVersion);
	virtual void despawnGrp();
	
	// Methods for setting up static data ------------------------------
	void setType(AITYPES::TFaunaType type);
	void setCyles(std::string const& cycles);
	
	// Methods for manageing population descriptions
	void setPopulation(CPopulation *pop);
	
	// Methods for managing places
	void setPlace(CAIPlace*& variable, CAIPlace* place);
	void setupWanderPlace();
	
	// Miscelaneous accessors ------------------------------------------
	CMgrFauna& mgr() const;
	
	// reference timer value
	static uint32 refTimer(TTime time);
	uint32 timer(TTime time) const { return _Timer[time]; }
	void setTimer(TTime timer, uint32 time) { _Timer[timer] = time*10; } ///< we set time in seconds.
	
	AITYPES::TFaunaType getType() const { return _Type; }
	
	void setSpawnType(AITYPES::TSpawnType sp_type) { _SpawnType = sp_type; }
	
	void displayPlaces(CStringWriter& stringWriter) const;
	
	CAliasCont<CAIPlace>& places() { return _Places; }
	CAliasCont<CPopulation>& populations() { return _Populations; }
	
	AITYPES::CPropertySet& faction() { return _Faction; }
	
	static const CCycleDef cycles[];
	static const uint32 nbCycle;
	
	void setAutoSpawn(bool autoSpawn) { CGroup::setAutoSpawn(autoSpawn); if (!isAutoSpawn()) _CurrentCycle=-1; }
	
	/// @name CChild implementation
	//@{
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	//@}
	
	/** Find next valid place with the wanted flags.
	  * If several places are valid, then one is picked randomly	  
	  * \return INVALID_PLACE if no such place exist
	  */
	sint getNextPlace(const CFaunaGenericPlace *startPlace, CAIPlaceXYRFauna::TFlag wantedFlag) const;

	// check that all places designated by the arcs of a given place are reachable
	bool checkArcs(const CAIPlace &startPlace) const;

public:
	// Dynamic data modified during normal activity --------------------
	uint32 _CurPopulation; // which alternative bot population is active
	
protected:
		
	// Static data initilised at init time -----------------------------
	
	// basic data (name, id, pointer to manager, etc)
	AITYPES::TFaunaType		_Type;	// FaunaTypeHerbivore, FaunaTypePredator, ... etc
	
	// Definition of a vector of vectors of classes for populations ----
	
	CAliasCont<CPopulation>	_Populations;
	CAliasCont<CAIPlace>	_Places;
	
	AITYPES::TSpawnType		_SpawnType;
	
	CAITimer					_priorityRecalcTimer;	// timer used to determine time of next recalc of _updateMask
	
	uint32					_Timer[LAST_TIME];
	
	float					_Aggro;
	
	/// Animat Addon
	double					_MotivationGroupProtection; // A value between 0 an 1 giving the importance of the group protection.
	double					_MotivationHarvestSap;		// Motivation for the sap harvest.
	
	std::vector<Cycle>		_Cycles;	//	Cycles List.
	sint32					_CurrentCycle;			//	current Cycle.
	sint32					_CurrentCycleIndex;		// the pop index in the cycle
	
	AITYPES::CPropertySet	_Faction;
};

#endif
