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

#ifndef RYAI_GRP_NPC_H
#define RYAI_GRP_NPC_H

#include "ai_grp.h"
#include "timer.h"
#include "ai_vision.h"
#include "ai_keywords.h"
#include "state_instance.h"
#include "named_entity_manager.h"
#include "nel/misc/string_mapper.h"

class CMgrNpc;
class CBotNpc;
class CSpawnBotNpc;

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroupNpc                                                           //
//////////////////////////////////////////////////////////////////////////////

class CSpawnGroupNpc
: public NLMISC::CDbgRefCount<CSpawnGroupNpc>
, public CSpawnGroup
{
public:
	CSpawnGroupNpc(CPersistent<CSpawnGroup>& owner);
	
	virtual ~CSpawnGroupNpc() { }
	
	CGroupNpc& getPersistent() const;
	
	virtual void spawnBots();
	virtual void despawnBots(bool immediately);
	
	void update();
	
	void sendInfoToEGS() const;
	
	std::string buildDebugString(uint idx) const;
	
	void stateChange(CAIState const* oldState, CAIState const* newState);
	
	void botHaveDied(CBotNpc* bot);
	void botHaveDespawn(CBotNpc* bot);
	void botHaveSpawn(CBotNpc* bot);
	
public:
	void resetSlowUpdateCycle();
	static void setSlowUpdatePeriod(uint32 ticks);
	static uint32 getSlowUpdatePeriod();
	static void displaySlowUpdateBuckets();
	
	void noMoreHandle(uint32 nNbTickBeforeDespawn);
	void handlePresent();

private:
	bool _GroupInVision;
	CAITimer _BotUpdateTimer;
	uint32 _LastUpdate;	// gamecycle at which update() last called
	uint32 _LastBotUpdate;
	uint32 _SlowUpdateCycle;
	static uint32 _SlowUpdatePeriod;
	static std::vector<uint32> _SlowUpdateBuckets;	

	bool _DespawnBotsWhenNoMoreHandleTimerActive;
	CAITimer	_DespawnBotsWhenNoMoreHandleTimer;
};

//////////////////////////////////////////////////////////////////////////////
// CGroupNpc                                                                //
//////////////////////////////////////////////////////////////////////////////

class CGroupNpc
: public NLMISC::CDbgRefCount<class CGroupNpc>
, public CGroup
, public CDynGrpBase
, public CPersistentStateInstance
{
public:
	typedef std::set<std::pair<std::string, sint32> > TFactionAttackableSet;
	
public:
	CGroupNpc(CMgrNpc* mgr, CAIAliasDescriptionNode* aliasTree, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	CGroupNpc(CMgrNpc* mgr, uint32 alias, std::string const& name, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	
	virtual ~CGroupNpc();
	
	/// @name CChild implementation
	//@{
//	virtual std::string getIndexString() const;
	virtual std::string getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
//	virtual std::string getFullName() const;
	//@}
	
	CDynGrpBase* getGrpDynBase() { return this; }
	
	//////////////////////////////////////////////////////////////////////////	
	//	PersistentStateInstance
	
	CAliasTreeOwner* aliasTreeOwner() { return this; }
	
	void stateChange(CAIState const* oldState, CAIState const* newState);
	
	CGroup* getGroup() { return this; }
	
	//////////////////////////////////////////////////////////////////////////
	
	virtual void lastBotDespawned();
	virtual void firstBotSpawned();
	
	// debugging stuff
	CDebugHistory* getDebugHistory() { return this; }
	
	CAIS::CCounter& getSpawnCounter();
	
	RYZOMID::TTypeId getRyzomType() { return RYZOMID::npc; }
	
	NLMISC::CSmartPtr<CSpawnGroup> createSpawnGroup();
	
	CSpawnGroupNpc*	getSpawnObj() const { return NLMISC::type_cast<CSpawnGroupNpc*>(CGroup::getSpawnObj()); }
	
	CPersistentStateInstance* getPersistentStateInstance() { return this; }
	
	void setEvent(uint eventId);

	virtual void serviceEvent (const CServiceEvent &info);
	
	void init() { }
	
	void release() { }

	
	// inheritted virtual interface ------------------------------------
	virtual bool spawn();
	virtual void despawnGrp();
	
	virtual std::string buildDebugString(uint idx) const;
	virtual void display(CStringWriter& stringWriter) const;
	
	void updateDependencies(CAIAliasDescriptionNode const& aliasTree, CAliasTreeOwner* aliasTreeOwner);
	IAliasCont* getAliasCont(AITYPES::TAIType type);
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree);			
	
	// basic utilities -------------------------------------------------
	CMgrNpc& mgr() const;
	
	// management of the bot population --------------------------------
	// allocator for allocating new bot objects
	CGroupNpc* newBot(); 
	
	bool botsAreNamed() { return _BotsAreNamed; }
	void setBotsAreNamedFlag() { _BotsAreNamed = true; }
	void clrBotsAreNamedFlag() { _BotsAreNamed = false; }
	
	// Parameter management -------------------------------------
 	void clearParameters();
	// Parse a paremeter for this group.
	void addParameter(const std::string &parameter);

	// set if the bots of the group are attackable by players
	void setPlayerAttackable(bool playerAttackable) { _PlayerAttackable = playerAttackable; }
	bool getPlayerAttackable() { return _PlayerAttackable; }
	
	// set if the bots of the group are attackable by other bots
	void setBotAttackable(bool botAttackable) { _BotAttackable = botAttackable; }
	bool getBotAttackable() { return _BotAttackable; }
	
	void setFactionAttackableAbove(std::string faction, sint32 threshold, bool botAttackable);
	TFactionAttackableSet const& getFactionAttackableAbove() const { return _FactionAttackableAbove; }
	void setFactionAttackableBelow(std::string faction, sint32 threshold, bool botAttackable);
	TFactionAttackableSet const& getFactionAttackableBelow() const { return _FactionAttackableBelow; }
	bool isFactionAttackable(std::string faction, sint32 fame);
	
	uint32 getAggroDist() { return _AggroDist; }
	
	uint32& despawnTime() { return _DespawnTime; }
	uint32& respawnTime() { return _RespawnTime; }		
	
	AITYPES::CPropertySetWithExtraList<TAllianceId>& faction() { return _faction; }
	AITYPES::CPropertySetWithExtraList<TAllianceId>& ennemyFaction() { return _ennemyFaction; }
	AITYPES::CPropertySetWithExtraList<TAllianceId>& friendFaction() { return _friendFaction; }
	
	AITYPES::CPropertySetWithExtraList<TAllianceId> const& faction() const { return _faction; }
	AITYPES::CPropertySetWithExtraList<TAllianceId> const& ennemyFaction() const { return _ennemyFaction; }
	AITYPES::CPropertySetWithExtraList<TAllianceId> const& friendFaction() const { return _friendFaction; }
	
public:
	void addHpUpTrigger(float threshold, int eventId);
	void delHpUpTrigger(float threshold, int eventId);
	void addHpUpTrigger(float threshold, std::string cbFunc);
	void delHpUpTrigger(float threshold, std::string cbFunc);
	
	void addHpDownTrigger(float threshold, int eventId);
	void delHpDownTrigger(float threshold, int eventId);
	void addHpDownTrigger(float threshold, std::string cbFunc);
	void delHpDownTrigger(float threshold, std::string cbFunc);
	
	bool haveHpTriggers();
	void hpTriggerCb(float oldVal, float newVal);
	
	void addNamedEntityListener(std::string const& name, std::string const& prop, int event);
	void delNamedEntityListener(std::string const& name, std::string const& prop, int event);
	void addNamedEntityListener(std::string const& name, std::string const& prop, std::string functionName);
	void delNamedEntityListener(std::string const& name, std::string const& prop, std::string functionName);
	void namedEntityListenerCb(std::string const& name, std::string const& prop);
	
	void addHandle(TDataSetRow playerRowId, uint32 missionAlias, uint32 DespawnTimeInTick);
	void delHandle(TDataSetRow playerRowId, uint32 missionAlias);
	
	uint32 getTimerWhenNoMoreHandle();

	void setSpawnZone(const CNpcZone *zone) { _SpawnZone = zone; }
	const CNpcZone *getSpawnZone() const { return _SpawnZone; }
	
	void setColour(uint8 colour);
	
	void setOutpostSide(OUTPOSTENUMS::TPVPSide side);
	void setOutpostFactions(OUTPOSTENUMS::TPVPSide side);
	bool isRingGrp() const { return _RingGrp;}
	
private:
	/// group basics
	bool _BotsAreNamed;	// true if the bots in the group are explicitly placed in level editor tool - false otherwise
	/// NPCs are attackable by player ?
	bool _PlayerAttackable;
	/// NPCs are attackable by npcs
	bool _BotAttackable;
	/// NPCs are attackable by players with fame for faction (string) above threshold (float)
	TFactionAttackableSet _FactionAttackableAbove;
	/// NPCs are attackable by players with fame for faction (string) below threshold (float)
	TFactionAttackableSet _FactionAttackableBelow;
	/// Aggro distance : any player passing at less this distance will be attacked
	uint32 _AggroDist;
	/// Respawn time in ticks
	uint32 _RespawnTime;
	/// Despawn time in ticks
	uint32 _DespawnTime;

	
	
	AITYPES::CPropertySetWithExtraList<TAllianceId> _faction;
	AITYPES::CPropertySetWithExtraList<TAllianceId> _ennemyFaction;
	AITYPES::CPropertySetWithExtraList<TAllianceId> _friendFaction;
	
	typedef std::multimap<float, int> THpTriggerList;
	typedef std::multimap<float, std::string> THpTriggerList2;
	THpTriggerList _hpUpTriggers;
	THpTriggerList _hpDownTriggers;
	THpTriggerList2 _hpUpTriggers2;
	THpTriggerList2 _hpDownTriggers2;
	
	typedef std::multimap<std::pair<std::string, std::string>, int> TNamedEntityListenerList;
	TNamedEntityListenerList _namedEntityListeners;
	typedef std::multimap<std::pair<std::string, std::string>, std::string> TNamedEntityListenerList2;
	TNamedEntityListenerList2 _namedEntityListeners2;

	struct SHandle
	{
		TDataSetRow	PlayerRowId;
		uint32		MissionAlias;
		
		bool operator < (const SHandle &h) const
		{
			if (PlayerRowId < h.PlayerRowId) return true;
			
			if (PlayerRowId == h.PlayerRowId)
				if (MissionAlias < h.MissionAlias)
					return true;

			return false;
		}
	};

	std::set<SHandle>	_Handles;
	uint32				_DespawnTimeWhenNoMoreHandle;
	bool				_AutoSpawnWhenNoMoreHandle;
	NLMISC::CVirtualRefPtr<const CNpcZone> _SpawnZone;
	bool				_RingGrp;//Ring rulez: like a override bandit profile

};

#endif
