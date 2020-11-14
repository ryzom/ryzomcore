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




#ifndef AI_SPIRE_H
#define AI_SPIRE_H
/*
#include "nel/misc/variable.h"
#include "child_container.h"
#include "alias_tree_root.h"
#include "game_share/zc_shard_common.h"
#include "service_dependencies.h"
#include "state_instance.h"
#include "manager_parent.h"
#include "event_reaction_container.h"
#include "ai_place_patat.h"

class CContinent;
class CCellZone;
class CSpireSquadFamily;
class CSpireManager;
class CSpireSquadManager;
template <class FamilyT>
class CGroupDesc;
template <class FamilyT>
class CBotDesc;
class CSpireGroup;
class CSpireSpawnZone;

extern NLMISC::CVariable<bool>	LogSpireDebug;

//////////////////////////////////////////////////////////////////////////////
// Helpers functions                                                        //
//////////////////////////////////////////////////////////////////////////////

namespace SPIREHELPERS {

extern uint32 getEntitySpire(CAIEntityPhysical const* entity);
extern bool isAttackingFaction(uint32 factionIndex, CAIEntityPhysical const* player);

}

#define RYAI_DEBUG_SPIRES 1

#ifdef RYAI_DEBUG_SPIRES

#define SPIRE_DBG nldebug
#define SPIRE_INF nlinfo
#define SPIRE_WRN nlwarning
#define SPIRE_ERR nlerror

#else

extern NLMISC::CLog SpireDbgLog, SpireInfLog, SpireWrnLog, SpireErrLog;
#define SPIRE_DBG SpireDbgLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), SpireDbgLog.displayNL
#define SPIRE_INF SpireInfLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), SpireInfLog.displayNL
#define SPIRE_WRN SpireWrnLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), SpireWrnLog.displayNL
#define SPIRE_ERR SpireErrLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), SpireErrLog.displayNL

#endif

//////////////////////////////////////////////////////////////////////////////
// CSpire                                                                 //
//////////////////////////////////////////////////////////////////////////////

class CSpire
: public NLMISC::CDbgRefCount<CSpire>
, public CAliasChild<CContinent> 
, public NLMISC::CRefCount
, public CAliasTreeRoot
, public CPlaceOwner
, public IManagerParent
{
public:

	typedef std::map< TAIAlias, NLMISC::CSmartPtr<CGroupDesc<CSpireSquadFamily> > > CSquadLinks;

	/// Return the spire corresponding to the alias, or NULL if not found (with a nlwarning)
	static CSpire* getSpireByAlias(TAIAlias spireAlias);

	CSpire(CContinent* owner, uint32 alias, std::string const& name, std::string const& filename);
	virtual ~CSpire();
	
	/// @name CChild implementation
	//@{
	virtual std::string getIndexString() const;
	virtual std::string	getOneLineInfoString() const;
//	virtual std::vector<std::string> getMultiLineInfoString() const;
	virtual std::string getFullName() const;
	//	virtual std::string getName() const;
	//@}
	
	/// @name IManagerParent implementation
	//@{
	virtual CAIInstance* getAIInstance() const;
	virtual CCellZone* getCellZone() { return NULL; }
//	virtual std::string getIndexString() const = 0;
	virtual std::string getManagerIndexString(CManager const* manager) const;
	virtual void groupDead(CGroup* grp) { }
	//@}
	
	/// @name Children containers accessors
	//@{
	CAliasCont<CSpireSpawnZone>& spawnZones() { return _SpawnZones; }
	CAliasCont<CSpireSpawnZone> const& spawnZones() const { return _SpawnZones; }
	CAliasCont<CSpireManager>& managers() { return _Managers; }
	CAliasCont<CSpireManager> const& managers() const { return _Managers; }
	//@}
	
	/// @name Spire zone
	//@{
	NLMISC::CSmartPtr<CAIPlaceSpire> const& getShape() const { return _Shape; }
	void setShape(NLMISC::CSmartPtr<CAIPlaceSpire> const& shape) { _Shape = shape; }
	//@}
	
	/// Returns the group containing building bots
	CGroup* getBuildingGroup();
	/// Returns the building bot with the specified alias
	CBot* getBuildingBotByAlias(TAIAlias alias);
	
	/// Set the tribe that can own the spire when no guild owns it
	void setTribe(std::string const& tribeName);

	std::string const& getTribe() const { return _Tribe; }

	/// Return the guild defending the spire, or InvalidGuildId
	TAllianceId getOwnerAlliance() const { return _OwnerAllianceId; }

	/// Assign the spire to a guild (defender) or to a tribe with InvalidAllianceId.
	void setOwnerAlliance( TAllianceId ownerAllianceId );

	/// Set the attacker guilds (opponent and its allies) of the spire
	void setAttackerAlliance( TAllianceId attackerAllianceId );
	
	/// Set the EGS state of the spire
	void setState( SPIREENUMS::TSpireState state );
	
	/// Manages service event (typically service ups and downs)
	void serviceEvent(CServiceEvent	const& info);
	
	std::string const& getSpireName() const { return _SpireName; }
	
	/// Called regularly at low frequency by continent update
	void update();
	
	void addZone(CSpireSpawnZone* zone);
	void removeZone(CSpireSpawnZone* zone);
	CSpireSpawnZone* getZone(NLMISC::TStringId zoneName);
	
	void createSquad(CGroupDesc<CSpireSquadFamily> const* groupDesc, CSpireSquadManager* manager, std::string const& initialStateName, CSpireSpawnZone const* spawnZone, uint32 spawnOrder, uint32 respawTimeGC, SPIREENUMS::TPVPSide side);
	void createSquad(std::string const& dynGroupName, std::string const& stateMachineName, std::string const& initialStateName, std::string const& zoneName, uint32 spawnOrder, uint32 respawTimeGC, SPIREENUMS::TPVPSide side);
	void createSquad(uint32 dynGroupAlias, uint32 zoneAlias, uint32 spawnOrder, uint32 respawTimeGC, SPIREENUMS::TPVPSide side);
	void spawnSquad(uint32 groupId);
	void despawnSquad(uint32 groupId);
	void deleteSquad(uint32 groupId);
	void despawnAllSquads();
	void sendSpireSquadStatus(CGroupNpc* group);
	void squadLeaderDied(CGroupNpc* group);
	void squadDied(CGroupNpc* group);
	
	void triggerSpecialEvent(SPIREENUMS::TSpecialSpireEvent eventId);
	
	void setBuildingBotSheet(uint32 buildingAlias, NLMISC::CSheetId sheetId, bool autoSpawnDespawn, const std::string & customName);

	bool spawn();
	bool despawn();
	
	/// returns the state of the spire
	std::string getStateName() const;
	
	/// return true if the spire is owned by a guild, not by a tribe
	bool isBelongingToAGuild() const { return (_OwnerAllianceId!=InvalidAllianceId); }
	
	// add a link to a squad that can be created/spawned in the spire
	void addSquadLink( TAIAlias alias, CGroupDesc<CSpireSquadFamily> *squad )
	{
		_SquadLinks.insert( std::make_pair( alias, squad ) );
	}

	/// get a squad that can be created/spawned in the spire
	CGroupDesc<CSpireSquadFamily> *getSquad( TAIAlias alias )
	{
		CSquadLinks::iterator it=_SquadLinks.find( alias );
		if ( it != _SquadLinks.end() )
			return (*it).second;
		else
			return NULL;
	}

	const CSquadLinks& squadLinks() const { return _SquadLinks; }
	
private:
	IAliasCont		*getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner	*createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree);	
	template <class T>
	void sendSpireMessage(std::string const& msgName, T& paramStruct);
		
private:
	/// @name AI service hierarchy
	//@{
	CAliasCont<CSpireSpawnZone> _SpawnZones;
	CAliasCont<CSpireManager> _Managers;
	//@}

	/// Links to the squad group descs
	CSquadLinks					_SquadLinks;

	/// The guild or the tribe that control the spire. Can be InvalidAllianceId if tribe alliance id not known yet.
	TAllianceId					_OwnerAllianceId;
	
	/// The guild that wants to control the spire (if any). Can be InvalidAllianceId (peace time).
	TAllianceId					_AttackerAllianceId;
	
	/// current state of the spire
	SPIREENUMS::TSpireState	_State;
	
	/// The tribe that controls this spire when no guild owns it
	std::string					_Tribe;
	
	/// Name of the spire
	std::string					_SpireName;
	
	// Zone map to retrieve a spawn zone based on its name
	typedef	std::map<NLMISC::TStringId, NLMISC::CDbgPtr<CSpireSpawnZone> > TZoneList;
	TZoneList	_ZoneList;
	
	/// The polygon surounding the spire
//	CShape	_Shape;
	NLMISC::CSmartPtr<CAIPlaceSpire>	_Shape;
};

//////////////////////////////////////////////////////////////////////////////
// CSpireSquadFamily                                                      //
//////////////////////////////////////////////////////////////////////////////

class CSpireSquadFamily
: public NLMISC::CDbgRefCount<CSpireSquadFamily>
, public NLMISC::CRefCount
, public CAliasChild<CAIInstance>
{
public:
	CSpireSquadFamily(CAIInstance* owner, uint32 alias, std::string const& name)
	: CAliasChild<CAIInstance>(owner, alias, name)
	{
	}
	virtual	~CSpireSquadFamily()
	{
		_GroupDescs.clear();	
	}
	
	CAliasCont<CGroupDesc<CSpireSquadFamily> >& groupDescs() { return _GroupDescs; }
	CAliasCont<CGroupDesc<CSpireSquadFamily> > const& groupDescs() const { return _GroupDescs; }
	
	virtual std::string getFullName() const;
	virtual std::string getIndexString() const;
	
//private:
	IAliasCont		*getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner	*createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree);	
	
private:
	/// These are only references to the group descs that are created when reading the squad template primitive
	CAliasCont< CGroupDesc<CSpireSquadFamily> > _GroupDescs;
};

//////////////////////////////////////////////////////////////////////////////
// CSpireSpawnZone                                                        //
//////////////////////////////////////////////////////////////////////////////

/// Zone for spire activity
class CSpireSpawnZone
: public NLMISC::CDbgRefCount<CSpireSpawnZone>
, public CAIPlaceXYR
{
public:
	CSpireSpawnZone(CSpire* owner, CAIAliasDescriptionNode* adn);
	~CSpireSpawnZone();
	
	virtual std::string getFullName() const;
	virtual std::string getIndexString() const;
};

//////////////////////////////////////////////////////////////////////////////
// CSpireManager                                                          //
// This is a container of bot groups,										//
// This is *not* a singleton manager as it is in the EGS.					//
//////////////////////////////////////////////////////////////////////////////

class CSpireManager
: public CMgrNpc
{
public:
	CSpireManager(CSpire* parent, uint32 alias, std::string const& name, std::string const& filename = std::string());
	virtual ~CSpireManager();
	
	void update();
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	virtual std::string	getOneLineInfoString() const;
	
	/// Change the list of enemies of the squad (attackers of the spire)
	void setEnemies( TAllianceId attackerAllianceId );
	
	CAIEvent EventSpirePeaceStateEnd;
	CAIEvent EventSpirePeaceStateBegin;
	CAIEvent EventSpireTribeOwnershipBegin;
	CAIEvent EventSpireTribeOwnershipEnd;
	CAIEvent EventSpireGuildOwnershipBegin;
	CAIEvent EventSpireGuildOwnershipEnd;
	CAIEvent EventSpireOwnerChanged;
	CAIEvent EventSpireAttackerChanged;
	CAIEvent EventSpireStateChanged;
	
	virtual void registerEvents();
	virtual void unregisterEvents();
	
	void setAutoSpawn(bool autoSpawn) { _AutoSpawn = autoSpawn; }
	void autoSpawnBegin();
	void autoSpawnEnd();
	
private:
	bool _AutoSpawn;
};

//////////////////////////////////////////////////////////////////////////////
// CSpireManager                                                          //
//////////////////////////////////////////////////////////////////////////////
/// This class derives from CSpireManager and adds a state machine for
/// squads.
class CSpireSquadManager
: public CSpireManager
{
public:
	CSpireSquadManager(CSpire* parent, uint32 alias, std::string const& name, std::string const& filename = std::string());
};
*/
#endif
