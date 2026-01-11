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




#ifndef AI_OUTPOST_H
#define AI_OUTPOST_H

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
class COutpostSquadFamily;
class COutpostManager;
class COutpostSquadManager;
template <class FamilyT>
class CGroupDesc;
template <class FamilyT>
class CBotDesc;
class COutpostGroup;
class COutpostSpawnZone;

extern NLMISC::CVariable<bool>	LogOutpostDebug;

//////////////////////////////////////////////////////////////////////////////
// Helpers functions                                                        //
//////////////////////////////////////////////////////////////////////////////

namespace OUTPOSTHELPERS {

extern uint32 getEntityOutpost(CAIEntityPhysical const* entity);
extern bool isAttackingFaction(uint32 factionIndex, CAIEntityPhysical const* player);

}

#define RYAI_DEBUG_OUTPOSTS 1

#ifdef RYAI_DEBUG_OUTPOSTS

#define OUTPOST_DBG nldebug
#define OUTPOST_INF nlinfo
#define OUTPOST_WRN nlwarning
#define OUTPOST_ERR nlerror

#else

extern NLMISC::CLog OutpostDbgLog, OutpostInfLog, OutpostWrnLog, OutpostErrLog;
#define OUTPOST_DBG OutpostDbgLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostDbgLog.displayNL
#define OUTPOST_INF OutpostInfLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostInfLog.displayNL
#define OUTPOST_WRN OutpostWrnLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostWrnLog.displayNL
#define OUTPOST_ERR OutpostErrLog.setPosition( __LINE__, __FILE__, __FUNCTION__ ), OutpostErrLog.displayNL

#endif

//////////////////////////////////////////////////////////////////////////////
// COutpost                                                                 //
//////////////////////////////////////////////////////////////////////////////

class COutpost
: public NLMISC::CDbgRefCount<COutpost>
, public CAliasChild<CContinent> 
, public NLMISC::CRefCount
, public CAliasTreeRoot
, public CPlaceOwner
, public IManagerParent
{
public:

	typedef std::map< TAIAlias, NLMISC::CSmartPtr<CGroupDesc<COutpostSquadFamily> > > CSquadLinks;

	/// Return the outpost corresponding to the alias, or NULL if not found (with a nlwarning)
	static COutpost* getOutpostByAlias(TAIAlias outpostAlias);

	COutpost(CContinent* owner, uint32 alias, std::string const& name, std::string const& filename);
	virtual ~COutpost();
	
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
	CAliasCont<COutpostSpawnZone>& spawnZones() { return _SpawnZones; }
	CAliasCont<COutpostSpawnZone> const& spawnZones() const { return _SpawnZones; }
	CAliasCont<COutpostManager>& managers() { return _Managers; }
	CAliasCont<COutpostManager> const& managers() const { return _Managers; }
	//@}
	
	/// @name Outpost zone
	//@{
	NLMISC::CSmartPtr<CAIPlaceOutpost> const& getShape() const { return _Shape; }
	void setShape(NLMISC::CSmartPtr<CAIPlaceOutpost> const& shape) { _Shape = shape; }
	//@}
	
	/// Returns the group containing building bots
	CGroup* getBuildingGroup();
	/// Returns the building bot with the specified alias
	CBot* getBuildingBotByAlias(TAIAlias alias);
	
	/// Set the tribe that can own the outpost when no guild owns it
	void setTribe(std::string const& tribeName);

	std::string const& getTribe() const { return _Tribe; }

	/// Return the guild defending the outpost, or InvalidGuildId
	TAllianceId getOwnerAlliance() const { return _OwnerAllianceId; }

	/// Assign the outpost to a guild (defender) or to a tribe with InvalidAllianceId.
	void setOwnerAlliance( TAllianceId ownerAllianceId );

	/// Set the attacker guilds (opponent and its allies) of the outpost
	void setAttackerAlliance( TAllianceId attackerAllianceId );
	
	/// Set the EGS state of the outpost
	void setState( OUTPOSTENUMS::TOutpostState state );
	
	/// Manages service event (typically service ups and downs)
	void serviceEvent(CServiceEvent	const& info);
	
	std::string const& getOutpostName() const { return _OutpostName; }
	
	/// Called regularly at low frequency by continent update
	void update();
	
	void addZone(COutpostSpawnZone* zone);
	void removeZone(COutpostSpawnZone* zone);
	COutpostSpawnZone* getZone(NLMISC::TStringId zoneName);
	
	void createSquad(CGroupDesc<COutpostSquadFamily> const* groupDesc, COutpostSquadManager* manager, std::string const& initialStateName, COutpostSpawnZone const* spawnZone, uint32 spawnOrder, uint32 respawTimeGC, OUTPOSTENUMS::TPVPSide side);
	void createSquad(std::string const& dynGroupName, std::string const& stateMachineName, std::string const& initialStateName, std::string const& zoneName, uint32 spawnOrder, uint32 respawTimeGC, OUTPOSTENUMS::TPVPSide side);
	void createSquad(uint32 dynGroupAlias, uint32 zoneAlias, uint32 spawnOrder, uint32 respawTimeGC, OUTPOSTENUMS::TPVPSide side);
	void spawnSquad(uint32 groupId);
	void despawnSquad(uint32 groupId);
	void deleteSquad(uint32 groupId);
	void despawnAllSquads();
	void sendOutpostSquadStatus(CGroupNpc* group);
	void squadLeaderDied(CGroupNpc* group);
	void squadDied(CGroupNpc* group);
	
	void triggerSpecialEvent(OUTPOSTENUMS::TSpecialOutpostEvent eventId);
	
	void setBuildingBotSheet(uint32 buildingAlias, NLMISC::CSheetId sheetId, bool autoSpawnDespawn, const std::string & customName);

	bool spawn();
	bool despawn();
	
	/// returns the state of the outpost
	std::string getStateName() const;
	
	/// return true if the outpost is owned by a guild, not by a tribe
	bool isBelongingToAGuild() const { return (_OwnerAllianceId!=InvalidAllianceId); }
	
	// add a link to a squad that can be created/spawned in the outpost
	void addSquadLink( TAIAlias alias, CGroupDesc<COutpostSquadFamily> *squad )
	{
		_SquadLinks.insert( std::make_pair( alias, squad ) );
	}

	/// get a squad that can be created/spawned in the outpost
	CGroupDesc<COutpostSquadFamily> *getSquad( TAIAlias alias )
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
	void sendOutpostMessage(std::string const& msgName, T& paramStruct);
		
private:
	/// @name AI service hierarchy
	//@{
	CAliasCont<COutpostSpawnZone> _SpawnZones;
	CAliasCont<COutpostManager> _Managers;
	//@}

	/// Links to the squad group descs
	CSquadLinks					_SquadLinks;

	/// The guild or the tribe that control the outpost. Can be InvalidAllianceId if tribe alliance id not known yet.
	TAllianceId					_OwnerAllianceId;
	
	/// The guild that wants to control the outpost (if any). Can be InvalidAllianceId (peace time).
	TAllianceId					_AttackerAllianceId;
	
	/// current state of the outpost
	OUTPOSTENUMS::TOutpostState	_State;
	
	/// The tribe that controls this outpost when no guild owns it
	std::string					_Tribe;
	
	/// Name of the outpost
	std::string					_OutpostName;
	
	// Zone map to retrieve a spawn zone based on its name
	typedef	std::map<NLMISC::TStringId, NLMISC::CDbgPtr<COutpostSpawnZone> > TZoneList;
	TZoneList	_ZoneList;
	
	/// The polygon surounding the outpost
//	CShape	_Shape;
	NLMISC::CSmartPtr<CAIPlaceOutpost>	_Shape;
};

//////////////////////////////////////////////////////////////////////////////
// COutpostSquadFamily                                                      //
//////////////////////////////////////////////////////////////////////////////

class COutpostSquadFamily
: public NLMISC::CDbgRefCount<COutpostSquadFamily>
, public NLMISC::CRefCount
, public CAliasChild<CAIInstance>
{
public:
	COutpostSquadFamily(CAIInstance* owner, uint32 alias, std::string const& name)
	: CAliasChild<CAIInstance>(owner, alias, name)
	{
	}
	virtual	~COutpostSquadFamily()
	{
		_GroupDescs.clear();	
	}
	
	CAliasCont<CGroupDesc<COutpostSquadFamily> >& groupDescs() { return _GroupDescs; }
	CAliasCont<CGroupDesc<COutpostSquadFamily> > const& groupDescs() const { return _GroupDescs; }
	
	virtual std::string getFullName() const;
	virtual std::string getIndexString() const;
	
//private:
	IAliasCont		*getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner	*createChild(IAliasCont	*cont, CAIAliasDescriptionNode *aliasTree);	
	
private:
	/// These are only references to the group descs that are created when reading the squad template primitive
	CAliasCont< CGroupDesc<COutpostSquadFamily> > _GroupDescs;
};

//////////////////////////////////////////////////////////////////////////////
// COutpostSpawnZone                                                        //
//////////////////////////////////////////////////////////////////////////////

/// Zone for outpost activity
class COutpostSpawnZone
: public NLMISC::CDbgRefCount<COutpostSpawnZone>
, public CAIPlaceXYR
{
public:
	COutpostSpawnZone(COutpost* owner, CAIAliasDescriptionNode* adn);
	~COutpostSpawnZone();
	
	virtual std::string getFullName() const;
	virtual std::string getIndexString() const;
};

//////////////////////////////////////////////////////////////////////////////
// COutpostManager                                                          //
// This is a container of bot groups,										//
// This is *not* a singleton manager as it is in the EGS.					//
//////////////////////////////////////////////////////////////////////////////

class COutpostManager
: public CMgrNpc
{
public:
	COutpostManager(COutpost* parent, uint32 alias, std::string const& name, std::string const& filename = std::string());
	virtual ~COutpostManager();
	
	void update();
	
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);
	virtual std::string	getOneLineInfoString() const;
	
	/// Change the list of enemies of the squad (attackers of the outpost)
	void setEnemies( TAllianceId attackerAllianceId );
	
	CAIEvent EventOutpostPeaceStateEnd;
	CAIEvent EventOutpostPeaceStateBegin;
	CAIEvent EventOutpostTribeOwnershipBegin;
	CAIEvent EventOutpostTribeOwnershipEnd;
	CAIEvent EventOutpostGuildOwnershipBegin;
	CAIEvent EventOutpostGuildOwnershipEnd;
	CAIEvent EventOutpostOwnerChanged;
	CAIEvent EventOutpostAttackerChanged;
	CAIEvent EventOutpostStateChanged;
	
	virtual void registerEvents();
	virtual void unregisterEvents();
	
	void setAutoSpawn(bool autoSpawn) { _AutoSpawn = autoSpawn; }
	void autoSpawnBegin();
	void autoSpawnEnd();
	
private:
	bool _AutoSpawn;
};

//////////////////////////////////////////////////////////////////////////////
// COutpostManager                                                          //
//////////////////////////////////////////////////////////////////////////////
/// This class derives from COutpostManager and adds a state machine for
/// squads.
class COutpostSquadManager
: public COutpostManager
{
public:
	COutpostSquadManager(COutpost* parent, uint32 alias, std::string const& name, std::string const& filename = std::string());
};

#endif
