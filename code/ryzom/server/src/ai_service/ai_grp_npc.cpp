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
#include "server_share/r2_variables.h"
#include "ai_grp_npc.h"
#include "ai_mgr_npc.h"
#include "ai_bot_npc.h"
#include "game_share/base_types.h"
#include "npc_description_msg.h"
#include "ai_player.h"
#include "ai_profile_npc.h"
#include "server_share/r2_variables.h"

extern bool simulateBug(int bugId);

#include "dyn_grp_inline.h"

using namespace MULTI_LINE_FORMATER;

using namespace NLMISC;
using namespace std;
using namespace	AITYPES;

extern CVariable<uint32> DefaultNpcAggroDist;
extern CVariable<float> DefaultEscortRange;

static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else nlinfo

// 5 minute in ticks (10 Hz)
uint32 DEFAULT_RESPAWN_DELAY = 5*60*10;

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroupNpc                                                           //
//////////////////////////////////////////////////////////////////////////////

uint32 CSpawnGroupNpc::_SlowUpdatePeriod = 10;
vector<uint32> CSpawnGroupNpc::_SlowUpdateBuckets(_SlowUpdatePeriod);

CSpawnGroupNpc::CSpawnGroupNpc(CPersistent<CSpawnGroup>& owner)
: CSpawnGroup(owner)
, _GroupInVision(false)
{
	sint32 const randomVal = (sint32)CTimeInterface::gameCycle()-CAIS::rand32(20);
	_LastUpdate = (randomVal>=0)?randomVal:CTimeInterface::gameCycle();
	_LastBotUpdate = CTimeInterface::gameCycle();
	activityProfile().setAIProfile(new CGrpProfileNormal(this));
	_BotUpdateTimer.set((CAIS::rand32(40)+((long)this>>2))%20); // start with a random value.
	resetSlowUpdateCycle();
	_DespawnBotsWhenNoMoreHandleTimerActive = false;
}

void CSpawnGroupNpc::setSlowUpdatePeriod(uint32 ticks)
{
	// Check args
	nlassert(ticks>0);
	if (ticks==0) return;
	// Save ticks number
	_SlowUpdatePeriod = ticks;
	// Reset cycles buckets
	_SlowUpdateBuckets.clear();
	_SlowUpdateBuckets.resize(_SlowUpdatePeriod);
	// For each group
	FOREACH (itInstance, CCont<CAIInstance>, CAIS::instance().AIList())
	{
		FOREACH (itManager, CCont<CManager>, itInstance->managers())
		{
			FOREACH (itGroup, CCont<CGroup>, itManager->groups())
			{
				// Find the spawn
				CGroup* group = *itGroup;
				CSpawnGroup* spawnGroup = group->getSpawnObj();
				CSpawnGroupNpc* spawnGroupNpc = dynamic_cast<CSpawnGroupNpc*>(spawnGroup);
				if (spawnGroupNpc!=NULL)
					spawnGroupNpc->resetSlowUpdateCycle();
			}
		}
	}
}

uint32 CSpawnGroupNpc::getSlowUpdatePeriod()
{
	return _SlowUpdatePeriod;
}

void CSpawnGroupNpc::resetSlowUpdateCycle()
{
	nlassert(_SlowUpdateBuckets.size() == _SlowUpdatePeriod);
	// Find the lowest bucket
	vector<uint32>::iterator it = std::min_element(_SlowUpdateBuckets.begin(), _SlowUpdateBuckets.end());
	// Assign it to the group
	_SlowUpdateCycle = (uint32)(it - _SlowUpdateBuckets.begin());
	// Fill the bucket with the group
	*it += bots().size();
}

void CSpawnGroupNpc::displaySlowUpdateBuckets()
{
	std::vector<uint32>::iterator it, end=_SlowUpdateBuckets.end();
	for (it = _SlowUpdateBuckets.begin(); it!=end; ++it)
	{
		nlinfo("Bucket %d: %d", it-_SlowUpdateBuckets.begin(), *it);
	}
}

void CSpawnGroupNpc::noMoreHandle(uint32 nNbTickBeforeDespawn)
{
	_DespawnBotsWhenNoMoreHandleTimerActive = true;
	_DespawnBotsWhenNoMoreHandleTimer.set(nNbTickBeforeDespawn);
}

void CSpawnGroupNpc::handlePresent()
{
	_DespawnBotsWhenNoMoreHandleTimerActive = false;
}

void CSpawnGroupNpc::sendInfoToEGS() const
{
	FOREACHC(itBot, CCont<CBot>, bots())
	{
		CBot const* bot = *itBot;
		if (!bot->isSpawned())
			continue;
		// :NOTE: As it can be called during CSpawnBotNpc dtor rely on dynamic cast to verify object is valid
		CSpawnBotNpc const* spawn = dynamic_cast<CSpawnBotNpc const*>(bot->getSpawnObj());
		if (spawn)
			spawn->sendInfoToEGS();
	}
}

void CSpawnGroupNpc::update()
{
	H_AUTO(GrpNpcUpdate);
	
	++AISStat::GrpTotalUpdCtr;
	++AISStat::GrpNpcUpdCtr;
	
	uint32 const Dt = CTimeInterface::gameCycle()-_LastUpdate;
	bool const inFight = activityProfile().getAIProfileType()==FIGHT_NORMAL;

	uint32 updateTrigger; // in tick

	// use the lowest update time available
	if (inFight)
	{
		updateTrigger = 0;
	}
	else if ( getPersistent().isRingGrp() )
	{
		updateTrigger = 4;
	}
	else if (_GroupInVision)
	{
		updateTrigger = 10;
	}
	else
	{
		updateTrigger = 30;
	}
	
	bool const haveToUpdateGroupBehaviour = (Dt>=updateTrigger); // every second.
	
	if (haveToUpdateGroupBehaviour)
	{
		H_AUTO(GrpNpcUpdateBehaviour);
		// record the tick at which we ran this update (for future refference)
		_LastUpdate = CTimeInterface::gameCycle();
		
		checkDespawn();
		checkRespawn();
		getPersistent().updateStateInstance();
	}
	
	// bot update()s --------------------------------------------------
	{
		if (haveToUpdateGroupBehaviour)
		{
			H_AUTO(GrpNpcUpdateBots);
			_GroupInVision = false;
			
			FOREACH(first,CCont<CBot>, bots())
			{
				CSpawnBotNpc const* const bot = static_cast<CBotNpc*>(*first)->getSpawn();
				if (!bot)
					continue;
				if (simulateBug(3))
				{
					if (bot->isAlive())
						continue;
				}
				else // Normal behaviour
				{
					if (!bot->isAlive())
						continue;
				}
				if (!bot->havePlayersAround())
					continue;
				
				_GroupInVision = true;
				break;
			}
		}
		
		// TODO : hack : remove this when the "can't reach and turn arround" debility is corrected
		bool fastUpdate = _GroupInVision || inFight;
		bool slowUpdate = (CTimeInterface::gameCycle()%_SlowUpdatePeriod)==_SlowUpdateCycle;
		
		if (fastUpdate || slowUpdate)
		{
			uint32 const botDt = CTimeInterface::gameCycle()-_LastBotUpdate;
			_LastBotUpdate = CTimeInterface::gameCycle();
			
			FOREACH(first, CCont<CBot>, bots())
			{
				CSpawnBotNpc* const bot = static_cast<CBotNpc*>(*first)->getSpawn();
				if (bot && bot->isAlive())
				{
					H_AUTO(GrpNpcUpdateBotsUpdate);
					bot->update(botDt);
				}
			}
			_BotUpdateTimer.set(10);
		}
	}
	
	if (haveToUpdateGroupBehaviour)
	{			
		H_AUTO(GrpNpcUpdateGrpBehaviour);
		if (!activityProfile().getAISpawnProfile().isNull()) // Check if we have a behaviour.
			activityProfile().updateProfile(Dt); // If so, then update it !
//		this->CBotAggroOwner::update(Dt);
	}

	if (_DespawnBotsWhenNoMoreHandleTimerActive)
	{
		if (_DespawnBotsWhenNoMoreHandleTimer.test())
		{
			_DespawnBotsWhenNoMoreHandleTimerActive = false;
			despawnBots(true);
		}
	}
}

void CSpawnGroupNpc::stateChange(CAIState const* oldState, CAIState const* newState)
{
	// Find changing group profiles.
	{
		setProfileParameters(getPersistent().profileParameters());
		
		IAIProfileFactory* moveProfile = newState->moveProfile();
		IAIProfileFactory* actProfile = newState->activityProfile();
		
		mergeProfileParameters(newState->profileParameters());
		
		FOREACHC(it, CCont<CAIStateProfile>, newState->profiles())
		{
			if (!it->testCompatibility(getPersistent()))
				continue;
			
			if (it->moveProfile()!= RYAI_GET_FACTORY(CGrpProfileNoChangeFactory))
				moveProfile = it->moveProfile();
			
			if (it->activityProfile()!=RYAI_GET_FACTORY(CGrpProfileNoChangeFactory))
				actProfile = it->activityProfile();
			
			mergeProfileParameters(it->profileParameters());
			break;
		}
		
		breakable
		{
			if (oldState)
			{
				if (!newState->isPositional() && oldState->isPositional())
				{
					// begin of punctual state
					// need to backup the current profiles
					_PunctualHoldActivityProfile = activityProfile();
					_PunctualHoldMovingProfile = movingProfile();
					
					activityProfile() = CProfilePtr();
					movingProfile() = CProfilePtr();
					break;
				}
				
				if (newState->isPositional() && !oldState->isPositional())
				{
					// end of punctual state
					// need to restore the backuped profile
					activityProfile() = _PunctualHoldActivityProfile;
					movingProfile() = _PunctualHoldMovingProfile;
					
					_PunctualHoldActivityProfile = CProfilePtr();
					_PunctualHoldMovingProfile = CProfilePtr();
					
					// resume the profiles
					activityProfile().getAISpawnProfile()->resumeProfile();
					movingProfile().getAISpawnProfile()->resumeProfile();
					break;
				}
			}
			
			// normal behavior, transition from positionnal to positionnal state
			if	(moveProfile!=RYAI_GET_FACTORY(CGrpProfileNoChangeFactory))
				setMoveProfileFromStateMachine(moveProfile);
			
			if	(actProfile!=RYAI_GET_FACTORY(CGrpProfileNoChangeFactory))
				setActivityProfileFromStateMachine(actProfile);
			
			break;
		}
	}

	// iterate through bots	changing their chat.
	for (CCont<CBot>::iterator it=bots().begin(), itEnd=bots().end();it!=itEnd;++it)
	{
		CBotNpc	*const	botNpc=static_cast<CBotNpc *>(*it);
		if (!botNpc->isSpawned())
			continue;
		botNpc->getSpawn()->updateChat(newState);
	}
}

void CSpawnGroupNpc::spawnBots()
{
	FOREACH(itBot, CCont<CBot>, bots())
	{
		CBot* bot = *itBot;
		if (!bot->isSpawned())
			bot->spawn();
	}
}

void CSpawnGroupNpc::despawnBots(bool immediately)
{
	FOREACH(itBot, CCont<CBot>, bots())
	{
		CBot* const bot = *itBot;
		if (!bot->isSpawned())
			continue;
		
		if (TheDataset.getOnlineTimestamp( bot->getSpawnObj()->dataSetRow()) >= CTickEventHandler::getGameCycle())
			nlwarning("Bots %s:%s spawn/despawn in the same tick ! despawn ignored", getPersistent().getName().c_str(), bot->getName().c_str());
		else
			bot->despawnBot();
	}
	if (getPersistent()._AutoDestroy)
		getPersistent().getOwner()->groups().removeChildByIndex(getPersistent().getChildIndex());
}

CGroupNpc& CSpawnGroupNpc::getPersistent() const
{
	return static_cast<CGroupNpc&>(CSpawnGroup::getPersistent());
}

//////////////////////////////////////////////////////////////////////////////
// CGroupNpc                                                                //
//////////////////////////////////////////////////////////////////////////////

CGroupNpc::CGroupNpc(CMgrNpc* mgr, CAIAliasDescriptionNode* aliasTree, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CGroup(mgr, denyFlags, aliasTree)
, CDynGrpBase()
, CPersistentStateInstance(*mgr->getStateMachine())
{
	_BotsAreNamed = true;
	
	_PlayerAttackable = false;
	_BotAttackable = false;
	_AggroDist = 0;
	_RingGrp = false;
}

CGroupNpc::CGroupNpc(CMgrNpc* mgr, uint32 alias, std::string const& name, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CGroup(mgr, denyFlags, alias, name)
, CDynGrpBase()
, CPersistentStateInstance(*mgr->getStateMachine())
{
	_BotsAreNamed = true;
	
	_PlayerAttackable = false;
	_BotAttackable = false;
	_AggroDist = 0;
}

CGroupNpc::~CGroupNpc() 
{
	// avoid re-deletion by despawn
	_AutoDestroy = false;
	if (isSpawned()) // to avoid bad CDbgPtr link interpretation
		despawnGrp();
	
	// clear all persistent state instance
	while (!_PSIChilds.empty())
	{
		_PSIChilds.back()->setParentStateInstance(NULL);
	}
	
	_PSIChilds.clear();
}

std::string CGroupNpc::getOneLineInfoString() const
{
	return std::string("NPC Group '") + getName() + "'";
}

std::vector<std::string> CGroupNpc::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	std::vector<std::string> strings;
	
	pushTitle(container, "CGroupNpc");
	strings = CGroup::getMultiLineInfoString();
	FOREACHC(itString, std::vector<std::string>, strings)
		pushEntry(container, *itString);
	pushEntry(container, "faction=" + (faction().empty()?string("<none>"):faction().toString()));
	pushEntry(container, "friends=" + (friendFaction().empty()?string("<none>"):friendFaction().toString()));
	pushEntry(container, "ennemies=" + (ennemyFaction().empty()?string("<none>"):ennemyFaction().toString()));
	pushEntry(container, NLMISC::toString("attackable: player=%s bot=%s", _PlayerAttackable?"yes":"no", _BotAttackable?"yes":"no"));
	pushEntry(container, "state=" + (getState()?getState()->getName():string("<null>")));
	pushFooter(container);
	
	return container;
}

CMgrNpc& CGroupNpc::mgr() const
{
	return *static_cast<CMgrNpc*>(getOwner());
}

void CGroupNpc::updateDependencies(CAIAliasDescriptionNode const& aliasTree, CAliasTreeOwner* aliasTreeOwner)
{	
	switch(aliasTree.getType())
	{		
		case AITypeEvent:
		{
			CAIEventReaction* const eventPtr = NLMISC::safe_cast<CAIEventReaction*>(mgr().getStateMachine()->eventReactions().getChildByAlias(aliasTree.getAlias()));
			nlassert(eventPtr);
			if (!eventPtr)
				break;
			eventPtr->setType(CAIEventReaction::FixedGroup);
			eventPtr->setGroup(getAlias());
			break;
		}
	}
}

IAliasCont* CGroupNpc::getAliasCont(TAIType type)
{
	switch(type)
	{
	case AITypeBot:
		return &_Bots;

	case AITypeFolder:		//	Nothing ..
	default:
		return NULL;
	}
}

CAliasTreeOwner* CGroupNpc::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	CAliasTreeOwner* child = NULL;
	
	switch (aliasTree->getType())
	{
	case AITypeOutpostBuilding:
	case AITypeBot:
		child = new CBotNpc(this, aliasTree);
		break;
		
	case AITypeFolder:
	default:
		break;
	}
	
	if (child!=NULL)
		cont->addAliasChild(child);
	return (child);
}

CSmartPtr<CSpawnGroup> CGroupNpc::createSpawnGroup()
{
	return new CSpawnGroupNpc(*this);
}

void CGroupNpc::serviceEvent (const CServiceEvent &info)
{
	CGroup::serviceEvent(info);
	
	// If the EGS crash
	if ( (info.getServiceName() == "EGS") && (info.getEventType() == CServiceEvent::SERVICE_DOWN) )
	{
		// If the Group of NPC has handles on itself
		if (!_Handles.empty())
		{
			// Remove all handles
			_Handles.clear();
			// and despawn bots
			if (getSpawnObj() != NULL)
			{
				if (getSpawnObj()->isGroupAlive()) // is there some bots spawned
				{
					getSpawnObj()->noMoreHandle(_DespawnTimeWhenNoMoreHandle);
				}
			}
		}
	}

	if ((info.getServiceName() == "EGS") && (info.getEventType() == CServiceEvent::SERVICE_UP))
	{
		processStateEvent(getEventContainer().EventEGSUp);
	}
	
}


bool	CGroupNpc::spawn	()
{		
	if (CGroup::spawn())
	{
		setStartState(getStartState());	//	stateInstance.
		
		// inform the EGS of our existence - simulate connection of EGS
		serviceEvent	(CServiceEvent(NLNET::TServiceId(0),std::string("EGS"),CServiceEvent::SERVICE_UP));
		
		if (isAutoSpawn())
		{
			CCont<CBot >::iterator first(bots().begin()), last(bots().end());
			for (; first != last; ++first)
			{
				CBotNpc *bot = static_cast<CBotNpc*>(*first);
				// ok, we can spawn this bot
				bot->spawn();
			}
		}
		return	true; 		
	}
	return	false;
}

void CGroupNpc::despawnGrp()
{
	CGroup::despawnGrp();
}

void CGroupNpc::clearParameters()
{
	_PlayerAttackable = false;
	_BotAttackable = false;
	_RingGrp = false;
	_AggroDist = DefaultNpcAggroDist;
	setEscortTeamId(CTEAM::InvalidTeamId);
	setEscortRange(DefaultEscortRange);
	respawnTime() = (uint32) DEFAULT_RESPAWN_DELAY;
	despawnTime() = (uint32) DEFAULT_RESPAWN_DELAY / 4;
}

// Parse a paremeter for this group.
void CGroupNpc::addParameter(std::string const& parameter)
{
	static std::string ATTACKABLE("attackable");
	static std::string PLAYER_ATTACKABLE("player_attackable");
	static std::string BOT_ATTACKABLE("bot_attackable");
	static std::string BADGUY("bad guy");
	static std::string AGGRO_RANGE("aggro range");
	static std::string ESCORT_RANGE("escort range");
	static std::string RESPAWN_TIME("respawn time");
	static std::string DESPAWN_TIME("despawn time");
	static std::string RING("ring");
	static std::string DENIED_ASTAR_FLAGS("denied_astar_flags");
	
	std::string key, tail;
	
	// force lowercase
	std::string p = NLMISC::toLower(parameter);
	AI_SHARE::stringToKeywordAndTail(p, key, tail);
	
	breakable
	{
		if (key == RING)
		{
			_RingGrp = true;
			break;
		}
		if (key == BOT_ATTACKABLE)
		{
			_BotAttackable = true;
			break;
		}
		
		if (key == ATTACKABLE || key == PLAYER_ATTACKABLE)
		{
			// the bots are attackable ! 
			_PlayerAttackable = true;
			if (!IsRingShard) // In Ring shard, BotAttackable means attackable by bot not vulnerable 
			{			
				// attackable implie vulnerable!
				_BotAttackable = true;
			}
			break;
		}
		
		if (key == BADGUY)
		{
			// the bots are bad guys! they will attack players in their aggro range.
			if (!tail.empty())
			{
				NLMISC::fromString(tail, _AggroDist);
				// bad guy imply attackable!
				_PlayerAttackable = true;
				// bad guy imply vulnerable!
				_BotAttackable = true;
			}
			else
			{
				nlwarning("GrpNpc::addParameter : in parameter '%s', missing agrodist !", parameter.c_str());
			}
			break;
		}

		if (key == AGGRO_RANGE)
		{
			if (!tail.empty())
			{
				NLMISC::fromString(tail, _AggroDist);

			}
			else
			{
				nlwarning("GrpNpc::addParameter : in parameter '%s', missing agrodist !", parameter.c_str());
			}
			break;
		}
		
		if (key == ESCORT_RANGE)
		{
			if (!tail.empty())
			{
				float range = float(atof(tail.c_str()));
				LOG("Setting range to %f", range);
				setEscortRange(range);
			}
			else
			{
				nlwarning("GrpNpc::addParameter : in parameter '%s', missing escort range !", parameter.c_str());
			}
			break;
		}
		
		if (key == RESPAWN_TIME)
		{
			if (!tail.empty())
			{
				uint32	respawntime = NLMISC::atoui(tail.c_str());
				// TODO:kxu: fix CAITimer!
				if (respawntime > 0x7fffffff)
					respawntime = 0x7fffffff; // AI timers do not treat properly delta times greater than the max signed int
				LOG("Setting respawn time to %u", respawntime);
				respawnTime()=respawntime;
			}
			else
			{
				nlwarning("GrpNpc::addParameter : in parameter '%s', missing respawn time !", parameter.c_str());
			}
			break;
		}
		
		if (key == DESPAWN_TIME)
		{
			if (!tail.empty())
			{
				uint32 despawntime = NLMISC::atoui(tail.c_str());
				// TODO:kxu: fix CAITimer!
				if (despawntime > 0x7fffffff)
					despawntime = 0x7fffffff; // AI timers do not treat properly delta times greater than the max signed int
				LOG("Setting despawn time to %u", despawntime);
				despawnTime()=despawntime;
			}
			else
			{
				nlwarning("GrpNpc::addParameter : in parameter '%s', missing despawn time !", parameter.c_str());
			}
			break;
		}
		
		if (key == DENIED_ASTAR_FLAGS)
		{
			if (!tail.empty())
			{
				RYAI_MAP_CRUNCH::TAStarFlag flags = RYAI_MAP_CRUNCH::Nothing;
				vector<string> sFlags;
				NLMISC::splitString(tail, "|", sFlags);
				FOREACHC(it, vector<string>, sFlags)
				{
					flags = RYAI_MAP_CRUNCH::TAStarFlag( flags | RYAI_MAP_CRUNCH::toAStarFlag(*it) );
				}

				LOG("Setting denied AStar flags to (%s) = %u", tail.c_str(), flags);
				_DenyFlags = flags;
			}
			else
			{
				nlwarning("GrpNpc::addParameter : in parameter '%s', missing denied AStar flags !", parameter.c_str());
			}
			break;
		}

		if	(parameter.empty())
			break;
		
		nlwarning("CAIBotNpc::addParameter unknown parameter '%s'", parameter.c_str());
	}
}

void CGroupNpc::addHpUpTrigger(float threshold, int eventId)
{
	_hpUpTriggers.insert(std::make_pair(threshold, eventId));
}

void CGroupNpc::delHpUpTrigger(float threshold, int eventId)
{
	CGroupNpc::THpTriggerList& hpTriggers = _hpUpTriggers;
	CGroupNpc::THpTriggerList::iterator first, last, trigger;
	first = hpTriggers.lower_bound(threshold);
	last = hpTriggers.upper_bound(threshold);
	
	for (; first!=last; ++first)
	{
		if (first->second==eventId)
			break;
	}
	if (first!=last)
		hpTriggers.erase(first);
}

void CGroupNpc::addHpDownTrigger(float threshold, int eventId)
{
	_hpDownTriggers.insert(std::make_pair(threshold, eventId));
}

void CGroupNpc::delHpDownTrigger(float threshold, int eventId)
{
	CGroupNpc::THpTriggerList& hpTriggers = _hpDownTriggers;
	CGroupNpc::THpTriggerList::iterator first, last, trigger;
	first = hpTriggers.lower_bound(threshold);
	last = hpTriggers.upper_bound(threshold);
	
	for (; first!=last; ++first)
	{
		if (first->second==eventId)
			break;
	}
	if (first!=last)
		hpTriggers.erase(first);
}

void CGroupNpc::addHpUpTrigger(float threshold, std::string cbFunc)
{
	_hpUpTriggers2.insert(std::make_pair(threshold, cbFunc));
}

void CGroupNpc::delHpUpTrigger(float threshold, std::string cbFunc)
{
	CGroupNpc::THpTriggerList2& hpTriggers = _hpUpTriggers2;
	CGroupNpc::THpTriggerList2::iterator first, last, trigger;
	first = hpTriggers.lower_bound(threshold);
	last = hpTriggers.upper_bound(threshold);
	
	for (; first!=last; ++first)
	{
		if (first->second==cbFunc)
			break;
	}
	if (first!=last)
		hpTriggers.erase(first);
}

void CGroupNpc::addHpDownTrigger(float threshold, std::string cbFunc)
{
	_hpDownTriggers2.insert(std::make_pair(threshold, cbFunc));
}

void CGroupNpc::delHpDownTrigger(float threshold, std::string cbFunc)
{
	CGroupNpc::THpTriggerList2& hpTriggers = _hpDownTriggers2;
	CGroupNpc::THpTriggerList2::iterator first, last, trigger;
	first = hpTriggers.lower_bound(threshold);
	last = hpTriggers.upper_bound(threshold);
	
	for (; first!=last; ++first)
	{
		if (first->second==cbFunc)
			break;
	}
	if (first!=last)
		hpTriggers.erase(first);
}

bool CGroupNpc::haveHpTriggers()
{
	return (_hpUpTriggers.size()+_hpDownTriggers.size()+_hpUpTriggers2.size()+_hpDownTriggers2.size())>0;
}

void CGroupNpc::hpTriggerCb(float oldVal, float newVal)
{
	if (newVal>oldVal)
	{
		CGroupNpc::THpTriggerList::const_iterator first, last, trigger, triggerProcessed;
		first = _hpUpTriggers.upper_bound(oldVal);
		last = _hpUpTriggers.upper_bound(newVal);
		for (trigger=first; trigger!=last;)
		{
			triggerProcessed = trigger;
			++trigger;
			processStateEvent(getEventContainer().EventUserEvent[triggerProcessed->second]);
		}
		CGroupNpc::THpTriggerList2::const_iterator first2, last2, trigger2, triggerProcessed2;
		first2 = _hpUpTriggers2.upper_bound(oldVal);
		last2 = _hpUpTriggers2.upper_bound(newVal);
		for (trigger2=first2; trigger2!=last2;)
		{
			triggerProcessed2 = trigger2;
			++trigger2;
			callScriptCallBack(this, NLMISC::CStringMapper::map(triggerProcessed2->second));
		}
	}
	if (newVal<oldVal)
	{
		CGroupNpc::THpTriggerList::const_iterator first, last, trigger, triggerProcessed;
		first = _hpDownTriggers.lower_bound(newVal);
		last = _hpDownTriggers.lower_bound(oldVal);
		for (trigger=first; trigger!=last;)
		{
			triggerProcessed = trigger;
			++trigger;
			processStateEvent(getEventContainer().EventUserEvent[triggerProcessed->second]);
		}
		CGroupNpc::THpTriggerList2::const_iterator first2, last2, trigger2, triggerProcessed2;
		first2 = _hpDownTriggers2.lower_bound(newVal);
		last2 = _hpDownTriggers2.lower_bound(oldVal);
		for (trigger2=first2; trigger2!=last2;)
		{
			triggerProcessed2 = trigger2;
			++trigger2;
			callScriptCallBack(this, NLMISC::CStringMapper::map(triggerProcessed2->second));
		}
	}
}

void CGroupNpc::addNamedEntityListener(std::string const& name, std::string const& prop, int event)
{
	_namedEntityListeners.insert(std::make_pair(std::make_pair(name, prop), event));
	CNamedEntityManager::getInstance()->get(name).addListenerGroup(prop, this);
}

void CGroupNpc::delNamedEntityListener(std::string const& name, std::string const& prop, int event)
{
	TNamedEntityListenerList::iterator first, last, listener;
	listener = _namedEntityListeners.lower_bound(std::make_pair(name, prop));
	last = _namedEntityListeners.upper_bound(std::make_pair(name, prop));
	while (listener!=last)
	{
		if (listener->second==event)
		{
			_namedEntityListeners.erase(listener);
			CNamedEntityManager::getInstance()->get(name).delListenerGroup(prop, this);
			break;
		}
		++listener;
	}
}

void CGroupNpc::addNamedEntityListener(std::string const& name, std::string const& prop, std::string functionName)
{
	_namedEntityListeners2.insert(std::make_pair(std::make_pair(name, prop), functionName));
	CNamedEntityManager::getInstance()->get(name).addListenerGroup(prop, this);
}

void CGroupNpc::delNamedEntityListener(std::string const& name, std::string const& prop, std::string functionName)
{
	TNamedEntityListenerList2::iterator first, last, listener;
	listener = _namedEntityListeners2.lower_bound(std::make_pair(name, prop));
	last = _namedEntityListeners2.upper_bound(std::make_pair(name, prop));
	while (listener!=last)
	{
		if (listener->second==functionName)
		{
			_namedEntityListeners2.erase(listener);
			CNamedEntityManager::getInstance()->get(name).delListenerGroup(prop, this);
			break;
		}
		++listener;
	}
}

void CGroupNpc::namedEntityListenerCb(std::string const& name, std::string const& prop)
{
	TNamedEntityListenerList::iterator first, last, listener;
	first = _namedEntityListeners.lower_bound(std::make_pair(name, prop));
	last = _namedEntityListeners.upper_bound(std::make_pair(name, prop));
	for (listener=first; listener!=last; ++listener)
	{
		processStateEvent(getEventContainer().EventUserEvent[listener->second]);
	}
	TNamedEntityListenerList2::iterator first2, last2, listener2;
	first2 = _namedEntityListeners2.lower_bound(std::make_pair(name, prop));
	last2 = _namedEntityListeners2.upper_bound(std::make_pair(name, prop));
	std::queue<NLMISC::TStringId> listeners;
	for (listener2=first2; listener2!=last2; ++listener2)
		listeners.push(NLMISC::CStringMapper::map(listener2->second));
	while(!listeners.empty())
	{
		callScriptCallBack(this, listeners.front());
		listeners.pop();
	}
}

//--------------------------------------------------------------------
// addHandle
//--------------------------------------------------------------------
void CGroupNpc::addHandle(TDataSetRow playerRowId, uint32 missionAlias, uint32 DespawnTimeInTick)
{
	// If the group is not spawned -> spawn it

	// Save some states when adding first handle
	if (_Handles.empty())
	{
		_AutoSpawnWhenNoMoreHandle = isAutoSpawn();
	}
	
	setAutoSpawn(true);
	// There is always a spawned object for group
	if (getSpawnObj() != NULL)
	{
		if (! getSpawnObj()->isGroupAlive() ) // if no bots spawned
		{
			getSpawnObj()->spawnBots();
		}
		getSpawnObj()->handlePresent();
	}
	// Register the handle
	SHandle h;
	h.MissionAlias = missionAlias;
	h.PlayerRowId = playerRowId;
	
	set<SHandle>::const_iterator it = _Handles.find(h);

	if (it != _Handles.end())
	{
		nlwarning("CGroupNpc::addHandle handle already added player:%d missionAlias:%d", playerRowId.getIndex(), missionAlias);
	}
	else
	{
		_Handles.insert(h);
	}

	_DespawnTimeWhenNoMoreHandle = DespawnTimeInTick;

	CHandledAIGroupSpawnedMsg msg;
	msg.PlayerRowId = playerRowId;
	msg.MissionAlias = missionAlias;
	msg.GroupAlias = getAlias();
	msg.send("EGS");
}

//--------------------------------------------------------------------
// delHandle
//--------------------------------------------------------------------
void CGroupNpc::delHandle(TDataSetRow playerRowId, uint32 missionAlias)
{
	// Unregister the handle
	SHandle h;
	h.MissionAlias = missionAlias;
	h.PlayerRowId = playerRowId;

	set<SHandle>::iterator it = _Handles.find(h);
	if (it != _Handles.end())
	{
		_Handles.erase(it);
	}
	else
	{
		// With the new queue_* generation system multiple handle_release can be done -> do not flood with this message
		// nlwarning("CGroupNpc::delHandle handle not found player:%d missionAlias:%d", playerRowId.getIndex(), missionAlias);
	}

	// If no more handle despawn the group and restore saved variables
	if (_Handles.empty())
	{
		if (getSpawnObj() != NULL)
		{
			if (getSpawnObj()->isGroupAlive()) // is there some bots spawned
			{
				getSpawnObj()->noMoreHandle(_DespawnTimeWhenNoMoreHandle);
			}
		}
		setAutoSpawn(_AutoSpawnWhenNoMoreHandle);
	}

	CHandledAIGroupDespawnedMsg msg;
	msg.PlayerRowId = playerRowId;
	msg.MissionAlias = missionAlias;
	msg.GroupAlias = getAlias();
	msg.send("EGS");
}

//--------------------------------------------------------------------
// the default display()
//--------------------------------------------------------------------
std::string	CSpawnGroupNpc::buildDebugString(uint idx) const
{
	if (idx == 0)
		return "No debug info";
	return std::string();
}

std::string	CGroupNpc::buildDebugString(uint idx) const
{
	
	switch(idx)
	{
	case 0: return "-- CGroupNpc -----------------------------";
	case 1: {
		std::string s;
		if (!isSpawned())
		{
			s += "id=" + getIndexString();
			s += " alias=" + getAliasString();
			s += " info=<not spawned>";
			s += " name=" + getName();
		}
		else
		{
			s += "id=" + getIndexString();
			s += " alias="+getAliasString();
			s += " info='" + getSpawnObj()->buildDebugString(0) + "'";
			s += " name="+getName();
		}
		return s;
	}
	case 2:
		return NLMISC::toString(": %s :", getAliasNode()->getName().c_str()) + buidStateInstanceDebugString();
	case 3:
		return	NLMISC::toString(" User Timers: %s %s %s %s",
			userTimer(0).toString().c_str(),
			userTimer(1).toString().c_str(),
			userTimer(2).toString().c_str(),
			userTimer(3).toString().c_str());
	case 4:
		if (getEscortTeamId() != CTEAM::InvalidTeamId)
			return	NLMISC::toString(" Escorted by team %u", getEscortTeamId());
		else
			return	std::string(" Not escorted");
	case 5: return "----------------------------------------";
	}
	return	std::string();
}

void CGroupNpc::display(CStringWriter	&stringWriter) const
{
#ifdef NL_DEBUG
	nlstopex(("not implemented"));
#endif
}

CAIS::CCounter& CGroupNpc::getSpawnCounter()
{
	return CAIS::instance()._NpcBotCounter;
}

void CGroupNpc::lastBotDespawned()
{
	// send message
	processStateEvent(getEventContainer().EventLastBotDespawned);
}

void CGroupNpc::firstBotSpawned()
{
	setFirstBotSpawned();
}

void CGroupNpc::setColour(uint8 colour)
{
	FOREACH(itBot, CCont<CBot>, bots())
	{
		CBotNpc* bot = static_cast<CBotNpc*>(*itBot);
		if (bot)
			bot->setColour(colour);
	}
}

void CGroupNpc::setOutpostSide(OUTPOSTENUMS::TPVPSide side)
{
	FOREACH(itBot, CCont<CBot>, bots())
	{
		CBotNpc* bot = static_cast<CBotNpc*>(*itBot);
		if (bot)
			bot->setOutpostSide(side);
	}
}

void CGroupNpc::setOutpostFactions(OUTPOSTENUMS::TPVPSide side)
{
	// Attack only the declared ennemies of the outpost
	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		// Bots factions
		faction      ().addProperty(NLMISC::toString("outpost:%s:bot_defender", getAliasString().c_str()));
		friendFaction().addProperty(NLMISC::toString("outpost:%s:bot_defender", getAliasString().c_str()));
		ennemyFaction().addProperty(NLMISC::toString("outpost:%s:bot_attacker", getAliasString().c_str()));
		// Players faction
		ennemyFaction().addProperty(NLMISC::toString("outpost:%s:attacker", getAliasString().c_str()));
	}
	if (side == OUTPOSTENUMS::OutpostAttacker)
	{
		// Bots factions
		faction      ().addProperty(NLMISC::toString("outpost:%s:bot_attacker", getAliasString().c_str()));
		friendFaction().addProperty(NLMISC::toString("outpost:%s:bot_attacker", getAliasString().c_str()));
		ennemyFaction().addProperty(NLMISC::toString("outpost:%s:bot_defender", getAliasString().c_str()));
		// Players faction
		ennemyFaction().addProperty(NLMISC::toString("outpost:%s:defender", getAliasString().c_str()));
	}
}

void CGroupNpc::setFactionAttackableAbove(std::string faction, sint32 threshold, bool botAttackable)
{
	if (botAttackable)
		_FactionAttackableAbove.insert(std::make_pair(faction, threshold));
	else
		_FactionAttackableAbove.erase(std::make_pair(faction, threshold));
}

void CGroupNpc::setFactionAttackableBelow(std::string faction, sint32 threshold, bool botAttackable)
{
	if (botAttackable)
		_FactionAttackableBelow.insert(std::make_pair(faction, threshold));
	else
		_FactionAttackableBelow.erase(std::make_pair(faction, threshold));
}

bool CGroupNpc::isFactionAttackable(std::string faction, sint32 fame)
{
	TFactionAttackableSet::const_iterator it, itEnd;
	for (it=_FactionAttackableAbove.begin(), itEnd=_FactionAttackableAbove.end(); it!=itEnd; ++it)
		if (faction==it->first && fame>it->second)
			return true;
	for (it=_FactionAttackableBelow.begin(), itEnd=_FactionAttackableBelow.end(); it!=itEnd; ++it)
		if (faction==it->first && fame<it->second)
			return true;
	return false;
}

void CGroupNpc::stateChange(CAIState const* oldState, CAIState const* newState)
{
	if (isSpawned())
		getSpawnObj()->stateChange(oldState, newState);
}

void CGroupNpc::setEvent(uint eventId)
{
	nlassert(eventId<10);
	if (eventId >= 10) return;
	processStateEvent(getEventContainer().EventUserEvent[eventId]);
}

//////////////////////////////////////////////////////////////////////////////
// Commands                                                                 //
//////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseNPCGrp,"Turn on or off or check the state of verbose npc group logging","")
{
	if(args.size()>1)
		return false;
	
	if(args.size()==1)
		StrToBool	(VerboseLog, args[0]);
	
	nlinfo("VerboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}

NLMISC_COMMAND(NpcGroupSlowUpdatePeriod, "Slow update period of the NPC groups","[<period_ticks>]")
{
	if (args.size()==0 || args.size()==1)
	{
		if (args.size()==1)
		{
			uint32 ticks;
			NLMISC::fromString(args[0], ticks);
			if (ticks>0)
				CSpawnGroupNpc::setSlowUpdatePeriod(ticks);
			else
				return false;
		}
		log.displayNL("Slow update period of the NPC groups is %d ticks", CSpawnGroupNpc::getSlowUpdatePeriod());
		return true;
	}
	return false;
}

#include "event_reaction_include.h"
