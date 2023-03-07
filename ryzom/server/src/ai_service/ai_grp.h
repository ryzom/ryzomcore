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



#ifndef RYAI_GRP_H
#define RYAI_GRP_H

#include "persistent_spawnable.h"
#include "ai_mgr.h"
#include "ai_bot.h"
#include "keyword_owner.h"
#include "debug_history.h"
#include "game_share/misc_const.h"
#include "profile_in_state.h"
#include "ai_aggro.h"
#include "dyn_grp.h"
#include "service_dependencies.h"

// forward decl
class CDebugHistory;
class CGroup;
class CBot;
class CSpawnBot;
class CPersistentStateInstance;

extern bool GrpHistoryRecordLog;

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroup                                                              //
//////////////////////////////////////////////////////////////////////////////

/// This is a common parent for all grp classes (bot group classes)
/// Group contains a container of Bots.
class CSpawnGroup 
: public NLMISC::CDbgRefCount<CSpawnGroup>
, public CSpawnable<CPersistent<CSpawnGroup> >
, public CProfileOwner
, public CProfileParameters
{
public:
	CSpawnGroup(CPersistent<CSpawnGroup>& owner);
	
	virtual	~CSpawnGroup();
	
	virtual void spawnBotOfGroup();
	
	void aggroLost(TDataSetRow const& aggroBot) const { }
	
	void aggroGain(TDataSetRow const& aggroBot) const { }
	
	virtual void spawnBots() = 0;
	virtual void despawnBots(bool immediately) = 0;
	
	virtual void update() = 0;
	
	//	Update Rate feature.
	virtual int getUpdatePriority() const { return 0; }
	
	virtual void recalcUpdatePriorityDelta() { }
	
	CGroup& getPersistent() const;
	
	CAliasCont<CBot> const& bots() const;
	CAliasCont<CBot>& bots();
	
	CBot* findLeader();
	
	CProfilePtr& movingProfile() { return _MovingProfile; }
	CProfilePtr& activityProfile() { return _ActivityProfile; }
	CProfilePtr const& activityProfile() const { return _ActivityProfile; }
	CProfilePtr& fightProfile() { return _FightProfile; }
	
	/// only for use by State Machine (or problems will occurs -> callStateChanged if same )
	void setMoveProfileFromStateMachine(IAIProfileFactory* staticProfile);
	/// only for use by State Machine (or problems will occurs -> callStateChanged if same )
	void setActivityProfileFromStateMachine(IAIProfileFactory* staticProfile);
	
	
	bool calcCenterPos(CAIVector& grp_pos, bool allowDeadBot = false);
	
	// Respawn bot list
	class CBotToSpawn
	{
	private:
		friend class CSpawnGroup;
		CBotToSpawn(uint32 botIndex, uint32 despawnTime, uint32 respawnTime)
		: _botIndex(botIndex)
		{
			_despawnTimer.set(despawnTime);
			_respawnTimer.set(respawnTime);
		}
		bool waitingDespawnTimeOver() const
		{
			return _despawnTimer.test();
		}
		bool waitingRespawnTimeOver() const
		{
			return _respawnTimer.test();
		}
		uint32 getBotIndex() const
		{
			return _botIndex;
		}
		CAITimer& respawnTimer()
		{
			return _respawnTimer;
		}
		uint32 _botIndex;
		CAITimer _despawnTimer;
		CAITimer _respawnTimer;
	};
	
	void incSpawnedBot(CBot& spawnBot);
	
	void decSpawnedBot();
	
	void addBotToDespawnAndRespawnTime(CBot* faunaBot, uint32 despawnTime, uint32 respawnTime);	
	
	void checkDespawn();	
	void checkRespawn();
	
	uint32 nbSpawnedBot() const { return _NbSpawnedBot; }
	
	uint32 nbBotToRespawn() const { return (uint32)_BotsToRespawn.size(); }
	
	uint32 nbBotToDespawn()	const { return (uint32)_BotsToDespawn.size(); }
	
	bool isGroupAlive(uint32 const nbMoreKilledBot = 0) const;
	
	CAIVector const& getCenterPos() const { return _CenterPos; }
	
	void setCenterPos(CAIVector const& pos) { _CenterPos = pos; }
	
	std::vector<std::string> getMultiLineInfoString() const;
	
	virtual NLMISC::CSmartPtr<CAIPlace const> buildFirstHitPlace(TDataSetRow const& aggroBot) const;
	
	void addAggroFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL));
	void setAggroMinimumFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL));
	bool haveAggro() const;
	bool haveAggroOrReturnPlace() const;
	
protected:
	CProfilePtr		_PunctualHoldMovingProfile;
	CProfilePtr		_PunctualHoldActivityProfile;

	// The group center pos (not always updated)
	CAIVector		_CenterPos;

private:
	uint32			_NbSpawnedBot;

	std::vector<CBotToSpawn>	_BotsToRespawn;
	std::vector<CBotToSpawn>	_BotsToDespawn;
	
	CProfilePtr		_MovingProfile;
	CProfilePtr		_ActivityProfile;
	CProfilePtr		_FightProfile;
};

//////////////////////////////////////////////////////////////////////////////
// CGroup                                                                   //
//////////////////////////////////////////////////////////////////////////////

class CGroup
: public NLMISC::CDbgRefCount<CGroup>
, public CPersistent<CSpawnGroup>
, public CAliasChild<CManager>
, public CAIEntity
, public CDebugHistory
, public NLMISC::CRefCount
, public CProfileParameters
, public CServiceEvent::CHandler
{
public:
	friend class CSpawnGroup;
	
	CGroup(CManager* owner, RYAI_MAP_CRUNCH::TAStarFlag denyFlag, CAIAliasDescriptionNode* aliasTree = NULL);
	CGroup(CManager* owner, RYAI_MAP_CRUNCH::TAStarFlag denyFlag, uint32 alias, std::string const& name);
	virtual ~CGroup();
	
	void serviceEvent(CServiceEvent const& info);
	
	CBot* getLeader();
	CBot* getSquadLeader(bool checkAliveStatus = true);
		
	void despawnBots();
	
	virtual CDynGrpBase* getGrpDynBase() = 0;
	
	CAliasTreeOwner* aliasTreeOwner() { return this; }
	
	bool _AutoDestroy;
	void autoDestroy(bool ad) { _AutoDestroy = ad; }
	
	/// @name CChild implementation
	//@{
	virtual std::string getIndexString() const;
	virtual std::string getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	virtual std::string getFullName() const;
	//@}
	
	virtual void lastBotDespawned();
	virtual void firstBotSpawned();
	
	virtual CPersistentStateInstance* getPersistentStateInstance() = 0;
	
	RYAI_MAP_CRUNCH::TAStarFlag getAStarFlag() const { return _DenyFlags; }
	
	virtual	CAIS::CCounter& getSpawnCounter() = 0;
	virtual RYZOMID::TTypeId getRyzomType() = 0;
	virtual void setEvent(uint eventId) = 0;
	
	virtual bool spawn();
	
	virtual NLMISC::CSmartPtr<CSpawnGroup> createSpawnGroup() = 0;
	
	virtual void despawnGrp();
	
	void despawnBots(bool immediately);
	
	CAliasCont<CBot> const& bots() const { return _Bots; }
	CAliasCont<CBot>& bots() { return _Bots; }
	
	void display(CStringWriter& stringWriter);
	
	CBot* getBot(uint32 index);
	
	// debugging stuff
	CDebugHistory* getDebugHistory() { return this; }
	
	CBot* getNextValidBotChild(CBot* child = NULL) { return _Bots.getNextValidChild(child); }
	
	CManager& getManager() { return *getOwner(); }
	
	void setEscortTeamId(uint16 teamId) { _EscortTeamId = teamId; }
	uint16 getEscortTeamId() const { return _EscortTeamId; }
	
	void setEscortRange(float range) { _EscortRange = range; }
	float getEscortRange() const { return _EscortRange; }
	
	virtual void setAutoSpawn(bool autoSpawn) { _AutoSpawn = autoSpawn; }
	bool isAutoSpawn() const { return _AutoSpawn; }
	
	CAIInstance* getAIInstance() const { return getOwner()->getAIInstance(); }
	
	void setEventParams(const std::vector<std::string> &a) { _EventParams = a; }
	std::string getEventParamString(uint32 i) { if (i >= _EventParams.size()) return ""; return _EventParams[i]; }
	float getEventParamFloat(uint32 i) { if (i >= _EventParams.size()) return 0.0f; return (float)atof(_EventParams[i].c_str()); }

	float				_AggroRange;
	uint32				_UpdateNbTicks;
	
protected:
	CAliasCont<CBot>	_Bots;
	/// Team Id of the escort (if any).
	uint16				_EscortTeamId;
	/// The range of the escort, ie the maximal distance of any escorter player that alow the group to be escorted
	float				_EscortRange;
	/// The bots automaticaly spawn when the group is spawned.
	bool				_AutoSpawn;
	
	RYAI_MAP_CRUNCH::TAStarFlag _DenyFlags;

	std::vector<std::string> _EventParams;
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroup                                                              //
//////////////////////////////////////////////////////////////////////////////

inline
CSpawnGroup::CSpawnGroup(CPersistent<CSpawnGroup>& owner)	
: CSpawnable<CPersistent<CSpawnGroup> >(owner)
, CProfileOwner()
, _NbSpawnedBot(0)
{
}

inline
void CSpawnGroup::setMoveProfileFromStateMachine(IAIProfileFactory* staticProfile)
{
	_MovingProfile.setAIProfile(this, staticProfile, true);
}

inline
void CSpawnGroup::setActivityProfileFromStateMachine(IAIProfileFactory* staticProfile)
{
	_ActivityProfile.setAIProfile(this, staticProfile, true);
}

inline
bool CSpawnGroup::isGroupAlive(uint32 const nbMoreKilledBot) const
{
	return ((sint32)_NbSpawnedBot-(sint32)_BotsToDespawn.size()-(sint32)nbMoreKilledBot)>0;
}

inline
CGroup& CSpawnGroup::getPersistent() const
{
	return static_cast<CGroup&>(CSpawnable<CPersistent<CSpawnGroup> >::getPersistent());
}

inline
CAliasCont<CBot> const& CSpawnGroup::bots() const
{
	return getPersistent()._Bots;
}

inline
CAliasCont<CBot>& CSpawnGroup::bots()
{
	return getPersistent()._Bots;
}

//////////////////////////////////////////////////////////////////////////////
// CGroup                                                                   //
//////////////////////////////////////////////////////////////////////////////

inline
bool CGroup::spawn()
{
	if (isSpawned())
		return true;
	
	if (!getSpawnCounter().remainToMax())
		return false;
	
	setSpawn(createSpawnGroup());
	return true;
}

inline
void CGroup::despawnGrp()
{
	if (!isSpawned())
		return;
	setSpawn(NULL);
	if (_AutoDestroy)
		getOwner()->groups().removeChildByIndex(getChildIndex());
}

inline
void CGroup::despawnBots(bool immediately)
{
	if (!isSpawned())
		return;		
	getSpawnObj()->despawnBots(immediately);
}

inline
CBot* CGroup::getBot(uint32 index)
{
	if (index>=_Bots.size())
		return NULL;
	return _Bots[index];
}

#endif





