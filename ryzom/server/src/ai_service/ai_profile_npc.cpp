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
#include "profile.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "ai_mgr_npc.h"
#include "ai_grp_fauna.h"

#include "ai_bot_fauna.h"

#include "states.h"
#include "path_behaviors.h"
#include "ai_aggro.h"
#include "ai_player.h"
#include "ai_generic_fight.h"
#include "ai_profile_npc.h"
#include "group_profile.h"

extern bool simulateBug(int bugId);

#include "dyn_grp_inline.h"

using namespace std;
using namespace RYAI_MAP_CRUNCH;
using namespace NLMISC;
using namespace NLNET;
using namespace	AITYPES;

// Global configuration variables
extern CVariable<int> DefaultWanderMinTimer;
extern CVariable<int> DefaultWanderMaxTimer;
extern CVariable<sint32> FameForGuardAttack;
extern CVariable<sint32> FameForGuardHelp;

// Stuff used for management of log messages
bool ai_profile_npc_VerboseLog = false;

void ai_profile_npc_LOG(std::string const& type, std::string const& profile, std::string const& step, std::string const& object)
{
	static size_t maxType    = 0;
	static size_t maxProfile = 0;
	static size_t maxStep    = 0;
	static size_t maxObject  = 0;
	if (ai_profile_npc_VerboseLog)
	{
		maxType    = std::max(maxType, type.length());
		maxProfile = std::max(maxProfile, profile.length());
		maxStep    = std::max(maxStep, step.length());
		maxObject  = std::max(maxObject, object.length());
		std::string log = "profile";
		log += " " + type    + std::string(maxType    - type.length(),    ' ');
		log += " " + profile + std::string(maxProfile - profile.length(), ' ');
		log += " " + step    + std::string(maxStep    - step.length(),    ' ');
		log += " " + object  + std::string(maxObject  - object.length(),  ' ');
		nlinfo("%s", log.c_str());
	}
}
#define PROFILE_LOG(type,profile,step,object) ai_profile_npc_LOG(type,profile,step,object)

#define LOG if (!ai_profile_npc_VerboseLog) {} else nlinfo

NLMISC_COMMAND(verboseAIProfiles,"Turn on or off or check the state of verbose ai profile info logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(ai_profile_npc_VerboseLog, args[0]);

	nlinfo("VerboseLogging is %s",ai_profile_npc_VerboseLog?"ON":"OFF");
	return true;
}

/****************************************************************************/
/* Local classes                                                            */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFightNpc                                                 //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFightNpc
: public CBotProfileFight
{
public:
	CBotProfileFightNpc(CProfileOwner* owner, CAIEntityPhysical* ennemy);
	virtual ~CBotProfileFightNpc();
	
	virtual std::string getOneLineInfoString() const { return "fight npc bot profile"; }
	
	void noMoreTarget();
	
	void eventBeginFight();
	void eventTargetKilled();
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileHealNpc                                                       //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileHealNpc
: public CBotProfileHeal
, public IAIEntityPhysicalHealer
{
public:
	CBotProfileHealNpc(CAIEntityPhysical* target, CProfileOwner* owner);
	virtual ~CBotProfileHealNpc();
	
	virtual std::string getOneLineInfoString() const { return "heal npc bot profile"; }
	
	void noMoreTarget();
	
	virtual void healerAdded(CAIEntityPhysical* entity);
	virtual void healerRemoved(CAIEntityPhysical* entity);
	
//	void eventBeginFight();
//	void eventTargetKilled();
private:
	CAIEntityPhysical* _Target;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileReturnAfterFightNpc                                           //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileReturnAfterFightNpc
: public CBotProfileReturnAfterFight
{
public:
	CBotProfileReturnAfterFightNpc(CSpawnBotNpc* owner);
	~CBotProfileReturnAfterFightNpc();
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual std::string getOneLineInfoString() const;
	
//	virtual NLMISC::CSmartPtr<CMovementMagnet> const& getMovementMagnet() const { return _MovementMagnet; }
	
private:
	CPathCont	_PathCont;
	NLMISC::CSmartPtr<CBotProfileFollowPos>	_MoveProfile;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileTribu                                                         //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileTribu : public CGrpProfileNormal
{
public:
	CGrpProfileTribu(CProfileOwner *owner);
	virtual ~CGrpProfileTribu();
	
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	
	virtual std::string getOneLineInfoString() const;
	
	virtual TProfiles getAIProfileType() const { return ACTIVITY_TRIBU; }
	
private:
	CAIVector _CenterPos;
};

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileStandOnVertices                                               //
//////////////////////////////////////////////////////////////////////////////

class CGrpProfileStandAtStartPoint
: public CMoveProfile
{
public:
	CGrpProfileStandAtStartPoint(CProfileOwner* owner);
	virtual ~CGrpProfileStandAtStartPoint();
	
	class CBotPositionner : public CRefCount
	{
	public:
		CBotPositionner(RYAI_MAP_CRUNCH::TAStarFlag flags);
		CBotPositionner(TVerticalPos verticalPos, CAIPos position, RYAI_MAP_CRUNCH::TAStarFlag flag);
		virtual ~CBotPositionner();
		void setBotAtDest(bool atDest = true);
		bool isBotAtDest() const;
		
		CPathCont		_PathCont;
		CAIPos			_Position;
		TVerticalPos	_VerticalPos;
	private:
		bool			_BotAtDest;
	};
	
	CPathCont* getPathCont(CBot const* bot);
	
	virtual void beginProfile();
	void resumeProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	
	void addBot(CBot* bot);
	void removeBot(CBot* bot);
	
	void setCurrentValidPos();
	
	virtual	TProfiles getAIProfileType() const { return MOVE_STAND_ON_VERTICES; }
	virtual std::string getOneLineInfoString() const;
	
private:
	typedef std::map<CBot const*, CSmartPtr<CBotPositionner> > TNpcBotPositionnerMap;
	
	TNpcBotPositionnerMap	_NpcList;
	bool					_Finished;
};

/****************************************************************************/
/* Local profile factories                                                  */
/****************************************************************************/

//- Simple profile factories (based on generic factory) ----------------------

// CGrpProfileFightFactory
typedef CAIGenericProfileFactory<CGrpProfileFight> CGrpProfileFightFactory;

// CGrpProfileTribuFactory
typedef CAIGenericProfileFactory<CGrpProfileTribu> CGrpProfileTribuFactory;

// CGrpProfileIdleFactory
typedef CAIGenericProfileFactory<CGrpProfileIdle> CGrpProfileIdleFactory;

// CGrpProfileStandAtStartPointFactory
typedef CAIGenericProfileFactory<CGrpProfileStandAtStartPoint> CGrpProfileStandAtStartPointFactory;

//- Singleton profiles (stateless ones) --------------------------------------
extern CGrpProfileFightFactory GrpProfileFightFactory;

/****************************************************************************/
/* Implementations                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroupNpc                                                           //
//////////////////////////////////////////////////////////////////////////////

void CSpawnGroupNpc::botHaveDied(CBotNpc* bot)
{
	// check bot profile type and update group profile bot list.
	if (fightProfile().getAIProfile())
	{
		CSlaveProfile* const profile = NLMISC::type_cast<CSlaveProfile*>(fightProfile().getAIProfile());
		if (profile)
			profile->removeBot(bot);
	}
	if (movingProfile().getAIProfile())
	{
		CSlaveProfile* const profile = NLMISC::type_cast<CSlaveProfile*>(movingProfile().getAIProfile());
		if (profile)
			profile->removeBot(bot);
	}
	
	{
		CSpawnBotNpc* const spawn = bot->getSpawn();
		if (spawn)
			spawn->setAIProfile(BotProfileStandAtPosFactory.createAIProfile(spawn));
	}
}

void CSpawnGroupNpc::botHaveDespawn(CBotNpc* bot)
{
	CSpawnGroupNpc::botHaveDied(bot);
}

void CSpawnGroupNpc::botHaveSpawn(CBotNpc* bot)
{
	if (movingProfile().getAIProfile())
	{
		NLMISC::safe_cast<CSlaveProfile*>(movingProfile().getAIProfile())->addBot(bot);
	}
	if (fightProfile().getAIProfile())
	{
		NLMISC::safe_cast<CSlaveProfile*>(fightProfile().getAIProfile())->addBot(bot);
	}
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFightNpc                                                      //
//////////////////////////////////////////////////////////////////////////////

CBotProfileFightNpc::CBotProfileFightNpc(CProfileOwner* owner, CAIEntityPhysical* ennemy)
: CBotProfileFight(owner, ennemy)
{
	PROFILE_LOG("bot", "fight_npc", "ctor", "");
}

CBotProfileFightNpc::~CBotProfileFightNpc()
{
	PROFILE_LOG("bot", "fight_npc", "dtor", "");
}

void CBotProfileFightNpc::noMoreTarget()
{		
	_Bot->setAIProfile(BotProfileStandAtPosFactory.createAIProfile(_Bot.ptr()));
}

void CBotProfileFightNpc::eventBeginFight()
{
	TempSpeaker = _Bot;
	CSpawnBotNpc* spawnable = NLMISC::safe_cast<CSpawnBotNpc*>(_Bot.ptr());
	CGroupNpc* grpNpc = static_cast<CGroupNpc*>(spawnable->getPersistent().getOwner());
	grpNpc->processStateEvent(grpNpc->getEventContainer().EventBotBeginFight);
	TempSpeaker = NULL;
}
void CBotProfileFightNpc::eventTargetKilled()
{
	TempSpeaker = _Bot;
	CSpawnBotNpc* spawnable= NLMISC::safe_cast<CSpawnBotNpc*>(_Bot.ptr());
	CGroupNpc* grpNpc = static_cast<CGroupNpc*>(spawnable->getPersistent().getOwner());
	grpNpc->processStateEvent(grpNpc->getEventContainer().EventBotTargetKilled);
	TempSpeaker = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileHealNpc                                                       //
//////////////////////////////////////////////////////////////////////////////

CBotProfileHealNpc::CBotProfileHealNpc(CAIEntityPhysical* target, CProfileOwner* owner)
: CBotProfileHeal(target->dataSetRow(), owner)
, _Target(target)
{
	PROFILE_LOG("bot", "heal_npc", "ctor", "");
	if (_Target)
		_Target->addHealer(this);
}

CBotProfileHealNpc::~CBotProfileHealNpc()
{
	PROFILE_LOG("bot", "heal_npc", "dtor", "");
	if (_Target)
		_Target->delHealer(this);
}

void CBotProfileHealNpc::healerAdded(CAIEntityPhysical* entity)
{
}

void CBotProfileHealNpc::healerRemoved(CAIEntityPhysical* entity)
{
	if (_Target == entity)
		_Target = NULL;
}

void CBotProfileHealNpc::noMoreTarget()
{		
	_Bot->setAIProfile(BotProfileStandAtPosFactory.createAIProfile(_Bot.ptr()));
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileReturnAfterFightNpc                                           //
//////////////////////////////////////////////////////////////////////////////

CBotProfileReturnAfterFightNpc::CBotProfileReturnAfterFightNpc(CSpawnBotNpc* owner)
: CBotProfileReturnAfterFight(owner)
, _PathCont(owner->getPersistent().getOwner()->getAStarFlag())
{
	PROFILE_LOG("bot_npc", "return_after_fight", "ctor", "");
//	CSpawnBotNpc* bot = NLMISC::safe_cast<CSpawnBotNpc*>(owner);
//	RYAI_MAP_CRUNCH::TAStarFlag denyFlags = bot->getAStarFlag();
	_PathCont.setDestination(owner->getReturnPos());
	_MoveProfile = NLMISC::CSmartPtr<CBotProfileFollowPos>(new CBotProfileFollowPos(&_PathCont, owner));
}

CBotProfileReturnAfterFightNpc::~CBotProfileReturnAfterFightNpc()
{
	PROFILE_LOG("bot_npc", "return_after_fight", "dtor", "");
}

void CBotProfileReturnAfterFightNpc::beginProfile()
{
	PROFILE_LOG("bot_npc", "return_after_fight", "begin", "");
	CBotProfileReturnAfterFight::beginProfile();
	_Bot->ignoreReturnAggro(true);
	_MoveProfile->beginProfile();
}

void CBotProfileReturnAfterFightNpc::endProfile()
{
	PROFILE_LOG("bot_npc", "return_after_fight", "end", "");
	_MoveProfile->endProfile();
	_Bot->ignoreReturnAggro(false);
	CBotProfileReturnAfterFight::endProfile();
}

void CBotProfileReturnAfterFightNpc::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(CBotProfileReturnAfterFightFaunaUpdate);
	CBotProfileReturnAfterFight::updateProfile(ticksSinceLastUpdate);
	_MoveProfile->updateProfile(ticksSinceLastUpdate);
}

std::string CBotProfileReturnAfterFightNpc::getOneLineInfoString() const
{
	std::string info = "return_after_fight npc bot profile";
	info += " (";
	info += _MoveProfile?_MoveProfile->getOneLineInfoString():std::string("<no move profile>");
	info += ")";
	return info;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFight                                                    //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileFight::CGrpProfileFight(CProfileOwner *owner)
: CFightProfile(owner)
, CFightOrganizer()
, _WasRunning(false)
{		
	PROFILE_LOG("group", "fight", "ctor", "");
}

CGrpProfileFight::~CGrpProfileFight()
{
	PROFILE_LOG("group", "fight", "dtor", "");
	for (CCont<CBot >::iterator it=_Grp->bots().begin(), itEnd=_Grp->bots().end();it!=itEnd;++it)
	{
		CBot	*bot=*it;
		removeBot	(bot);
	}		
}

void CGrpProfileFight::beginProfile()
{
	PROFILE_LOG("group", "fight", "begin", "");
	if (_Grp->getPersistent().getGrpDynBase() != /*(CDynGrpBase*)*/NULL
		&& !_Grp->getPersistent().getGrpDynBase()->getFamilyBehavior().isNULL())
	{
		// this is a dynamic bots groups, activate assist between groups
		_CheckAround.set(CHECK_AROUND_PERIOD);
	}

	_HaveEnnemy=true;
	for (CCont<CBot >::iterator it=_Grp->getPersistent().bots().begin(), itEnd=_Grp->getPersistent().bots().end();it!=itEnd;++it)
	{
		addBot	(*it);
	}
	_WasRunning = _Grp->checkProfileParameter("running");
	if (!_WasRunning)
		_Grp->addProfileParameter("running", "", 0.f);
}

void CGrpProfileFight::endProfile()
{
	PROFILE_LOG("group", "fight", "end", "");
	if (!_WasRunning)
		_Grp->removeProfileParameter("running");
}

void CGrpProfileFight::addBot(CBot* bot)
{
	vector<CBot*>::iterator it = find(_NpcList.begin(), _NpcList.end(), bot);
	if (it==_NpcList.end())
		_NpcList.push_back(bot);
}

void CGrpProfileFight::removeBot(CBot* bot)
{
	vector<CBot*>::iterator it = find(_NpcList.begin(), _NpcList.end(), bot);
	if (it!=_NpcList.end())
		_NpcList.erase(it);
}

void CGrpProfileFight::setFight(CSpawnBot* bot, CAIEntityPhysical* ennemy)
{
	bot->setAIProfile(new CBotProfileFightNpc(bot, ennemy));
}

void CGrpProfileFight::setHeal(CSpawnBot* bot, CAIEntityPhysical* target)
{
	bot->setAIProfile(new CBotProfileHealNpc(target, bot));
}

void CGrpProfileFight::setNoFight(CSpawnBot* bot)
{
	if (!bot->getTarget().isNULL())
		bot->setTarget(NULL);
	if (	bot->getAIProfileType()==BOT_FLEE
		||	bot->getAIProfileType()==BOT_FIGHT
		||	bot->getAIProfileType()==BOT_HEAL
		||	bot->getAIProfileType()==BOT_RETURN_AFTER_FIGHT	)
	{			
		bot->setAIProfile(new CBotProfileStandAtPos(bot));
	}
}

void CGrpProfileFight::setFlee(CSpawnBot* bot, CAIVector& fleeVect)
{
	bot->setMoveDecalage(fleeVect);		
	if (bot->getAIProfileType()!=BOT_FLEE)
	{
		bot->setAIProfile(new CBotProfileFlee(bot));
	}
}

void CGrpProfileFight::setReturnAfterFight(CSpawnBot* bot)
{
	if (bot->getAIProfileType()!=BOT_RETURN_AFTER_FIGHT)
	{
		bot->setAIProfile(new CBotProfileReturnAfterFightNpc(NLMISC::safe_cast<CSpawnBotNpc*>(bot)));
	}
}

bool CGrpProfileFight::stillHaveEnnemy() const
{
	return _HaveEnnemy;
}

void CGrpProfileFight::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpFightProfileUpdate);
	CFollowPathContext fpcGrpFightProfileUpdate("GrpFightProfileUpdate");

	// check if some bots died or are despawned.

	for(uint i = 0; i < _NpcList.size();)
	{
		CSpawnBot	*spawnBot=_NpcList[i]->getSpawnObj();
		if (	!spawnBot
			||	!spawnBot->isAlive())
		{
			_NpcList.erase(_NpcList.begin()+i);
			continue;
		}
		i++;
	}
	reorganize(_NpcList.begin(), _NpcList.end());
	
	// check groups around
	if (_CheckAround.test())
	{
		_CheckAround.set(CHECK_AROUND_PERIOD);
		
		FOREACH(itBot, vector<CBot*>, _NpcList)
		{
			CBot* pBot = *itBot;
			if (pBot)
			{
				CSpawnBot* bot = pBot->getSpawnObj();
				if (bot)
					bot->propagateAggro();
			}
		}
	}
}
	
std::string	CGrpProfileFight::getOneLineInfoString() const
{
	return "fight profile";
}

std::vector<CBot*>& CGrpProfileFight::npcList()
{
	return _NpcList;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileNormal                                                        //
//////////////////////////////////////////////////////////////////////////////

void CGrpProfileNormal::beginProfile()
{
	PROFILE_LOG("group", "normal", "begin", "");
	_GroupFighting=false;
}
	
void CGrpProfileNormal::endProfile()
{
	PROFILE_LOG("group", "normal", "end", "");
	setGroupFighting(false);
}
		
void CGrpProfileNormal::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpNormalProfileUpdate);
	CFollowPathContext fpcGrpNormalProfileUpdate("GrpNormalProfileUpdate");

	if (_GroupFighting)
	{
		if (!_Grp->fightProfile().getAIProfile())
			_Grp->fightProfile().setAIProfile(new	CGrpProfileFight(_Grp));
		
		_Grp->fightProfile().mayUpdateProfile(ticksSinceLastUpdate);
		
		CFightProfile* profile = NLMISC::safe_cast<CFightProfile*>(_Grp->fightProfile().getAIProfile());
		if	(!profile->stillHaveEnnemy	())
		{
			// :TODO: Verify if it's needed to erase bots aggro too/instead
//			_Grp->clearAggroList();	// this erase all agro.
			
			setGroupFighting(false);
			_Grp->fightProfile().setAIProfile(NULL);
			
			CSlaveProfile* moveProfile = NLMISC::type_cast<CSlaveProfile*>(_Grp->movingProfile().getAIProfile());
			if (moveProfile)
				moveProfile->resumeProfile();
		}
	}
	else
	{
		if (_Grp->haveAggroOrReturnPlace())
		{
			if(_Grp->isGroupAlive())
			{
				//	set the fighting comportment.
				if (!_Grp->fightProfile().getAIProfile())
	//				_Grp->fightProfile().setAIProfile(new CGrpProfileFight(_Grp));
					_Grp->fightProfile().setAIProfile(_Grp.ptr(), &GrpProfileFightFactory, false);
				
				setGroupFighting(true);
			}
		}
		else
		{
			_Grp->movingProfile().mayUpdateProfile(ticksSinceLastUpdate);
		}
	}
}
	
std::string CGrpProfileNormal::getOneLineInfoString() const
{
	std::string info = "normal profile";
	info += " group_fighting=" + NLMISC::toString(_GroupFighting);
	return info;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileBandit                                                   //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileBandit::CGrpProfileBandit(CProfileOwner *owner)
: CGrpProfileNormal(owner)
{
	PROFILE_LOG("group", "bandit", "ctor", "");
}

CGrpProfileBandit::~CGrpProfileBandit()
{
	PROFILE_LOG("group", "bandit", "dtor", "");
}

void CGrpProfileBandit::beginProfile()
{
	PROFILE_LOG("group", "bandit", "begin", "");
	CGrpProfileNormal::beginProfile();
	

	CGroupNpc	&persGrp=_Grp->getPersistent();
	
	if (persGrp.isRingGrp())
	{
		_AggroRange = persGrp.getAggroDist();
	}
	else
	{
		// look for aggro range parameter or set a default value
		float aggroRangeFloat = 0.f;

		if (!_Grp->getProfileParameter("aggro range", aggroRangeFloat))
			_AggroRange =static_cast<uint32>( CGrpProfileBanditFactory::getDefaultBanditAggroRange() );
		else
			_AggroRange = static_cast<uint32>(aggroRangeFloat);
	
		bool resendInfo = false;
		

		if (!persGrp.getPlayerAttackable	())
		{
			persGrp.setPlayerAttackable	(true);
			resendInfo = true;
		}
		if (!persGrp.getBotAttackable	())
		{
			persGrp.setBotAttackable	(true);
			resendInfo = true;
		}
		if (resendInfo)
			_Grp->sendInfoToEGS		();
	}
	

}

void CGrpProfileBandit::endProfile()
{
	PROFILE_LOG("group", "bandit", "end", "");
	CGrpProfileNormal::endProfile();
}
	
void CGrpProfileBandit::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpBanditProfileUpdate);
	CFollowPathContext fpcGrpBanditProfileUpdate("GrpBanditProfileUpdate");

	CAIVision<CPersistentOfPhysical>	BanditVision;

	breakable
	{
		CAIVector	centerPos;
		if	(!_Grp->calcCenterPos(centerPos))	// true if there's some bots in the group.
			break;
		_Grp->setCenterPos(centerPos);
			
		uint32	playerRadius=uint(_AggroRange);
		uint32	botRadius=uint(_AggroRange);
		uint32	groupPlayerRadius=playerRadius*2;
		uint32	groupBotRadius=botRadius*2;
		
		uint32	minRadius=playerRadius>botRadius?botRadius:playerRadius;
		
		CFightProfile*	fightProfile=static_cast<CFightProfile*>(_Grp->fightProfile().getAIProfile());

		if (fightProfile)
		{					
			CAIVision<CPersistentOfPhysical>	localBanditVision;
			
			for (vector<CBot*>::iterator it=fightProfile->npcList().begin(), itEnd=fightProfile->npcList().end();it!=itEnd;++it)
			{
				CBot			*bot=(*it);
				CSpawnBot	*spawnBot=bot->getSpawnObj();
				
				if (!spawnBot)
					continue;
				
				double	distToCenter=centerPos.quickDistTo(spawnBot->pos());
				if (distToCenter>minRadius) // (minRadius*2) - minRadius
				{
					const CAIVector spawnBotPos(spawnBot->pos());
					//	bot vision update.
					localBanditVision.updateBotsAndPlayers(spawnBot->getAIInstance(), spawnBotPos, playerRadius, botRadius);
					
					//	bandits don't like guards nor escorted people
					{
						const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &bots = localBanditVision.bots();
						std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(bots.begin()), last(bots.end());
						for (; first != last; ++first)
						{
							CAIEntityPhysical *ep = (*first)->getSpawnObj();
							if (	ep
								&&	ep->getRyzomType()==RYZOMID::npc
								&&	ep->isAlive())
							{
								CSpawnBotNpc *botNpc = NLMISC::safe_cast<CSpawnBotNpc *>(ep);
								
								if	(	botNpc->spawnGrp().activityProfile().getAIProfileType() == ACTIVITY_GUARD
									||	botNpc->spawnGrp().activityProfile().getAIProfileType() == ACTIVITY_GUARD_ESCORTED
									||	botNpc->spawnGrp().activityProfile().getAIProfileType() == ACTIVITY_ESCORTED	)
								{
									spawnBot->setAggroMinimumFor(ep->dataSetRow(), 0.8f, false);
								}
							}
						}
					}
					
					// bandits don't like players.
					{			
						const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &players = localBanditVision.players();
						std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(players.begin()), last(players.end());
						for (; first != last; ++first)
						{
							CPersistentOfPhysical *player = (*first);
							CAIEntityPhysical *ep = player->getSpawnObj();
							if (	ep
								&&	ep->isAlive()
								&&	ep->currentHitPoints()>0.f)
							{
								const	CRootCell	*const	rootCell=ep->wpos().getRootCell();
								if	(	rootCell
									&&	rootCell->getFlag()!=0	)	//	Safe Zone ?
									continue;

								spawnBot->setAggroMinimumFor(ep->dataSetRow(), 0.5f, false);
							}
						}
					}											
				}
			}
		}
		// group vision update.
		BanditVision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), centerPos, playerRadius, botRadius);
	}

	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);

	// check if we are in war and if some bot are waiting for a bus.
	if	(_GroupFighting)
	{
		// check if some bots are not fighting.
		for (CCont<CBot >::iterator it=_Grp->getPersistent().bots().begin(), itEnd=_Grp->getPersistent().bots().end();it!=itEnd;++it)
		{
			CBot*			bot=*it;
			CSpawnBot	*spawnBot=bot->getSpawnObj();
			if (	spawnBot
				&&	spawnBot->isAlive()
				&&	spawnBot->getAIProfileType()==BOT_STAND_AT_POS)
			{
				// :KLUDGE: We verify here that we have a moving profile, to prevent some crashes
				// :TODO: Remove that check and make sure a group always have a moving profile (even if none is defined in primitives)
				if (_Grp->movingProfile().getAIProfile())
				{
					CMoveProfile	*moveProf=NLMISC::safe_cast<CMoveProfile*>(_Grp->movingProfile().getAIProfile());
					moveProf->resumeBot(bot);
				}
			}
		}
	}
	
	//	bandits don't like guards nor escorted people
	{
		const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &bots = BanditVision.bots();
		std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(bots.begin()), last(bots.end());
		for (; first != last; ++first)
		{
			CAIEntityPhysical *const ep = (*first)->getSpawnObj();
			if (	!ep
				||	ep->getRyzomType()!=RYZOMID::npc
				||	!ep->isAlive())
				continue;

			const	CSpawnBot	*const bot = NLMISC::safe_cast<const CSpawnBot *>(ep);
			
			const	TProfiles	profileType=bot->spawnGrp().activityProfile().getAIProfileType();
			if	(	profileType != ACTIVITY_GUARD
				&&	profileType != ACTIVITY_GUARD_ESCORTED
				&&	profileType != ACTIVITY_ESCORTED	)
				continue;

			_Grp->setAggroMinimumFor(ep->dataSetRow(), 0.8f, false);
		}
	}
	
	// bandits don't like players.
	{
		const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &players = BanditVision.players();
		
		std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(players.begin()), last(players.end());
		for (; first != last; ++first)
		{
			CPersistentOfPhysical*const	player = (*first);
			CAIEntityPhysical*const		ep = player->getSpawnObj();

			if (	ep
				&&	ep->isAlive()
				&&	ep->currentHitPoints()>0.f)	//	not in safe zone.
			{
				const	CRootCell	*const	rootCell=ep->wpos().getRootCell();
				if	(	rootCell
					&&	rootCell->getFlag()!=0	)	//	Safe Zone ?
					continue;
				
				_Grp->setAggroMinimumFor(ep->dataSetRow(), 0.5f, false);
			}
		}
	}
}

std::string CGrpProfileBandit::getOneLineInfoString() const
{
	std::string info = "bandit profile";
	info += " aggro_range=" + NLMISC::toString(_AggroRange);
	return info;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileGuard                                                    //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileGuard::CGrpProfileGuard(CProfileOwner *owner)
: CGrpProfileNormal(owner)
{
	PROFILE_LOG("group", "guard", "ctor", "");
}

CGrpProfileGuard::~CGrpProfileGuard()
{
	PROFILE_LOG("group", "guard", "dtor", "");
}
		
void CGrpProfileGuard::beginProfile()
{
	PROFILE_LOG("group", "guard", "begin", "");
	CGrpProfileNormal::beginProfile();
	CGroupNpc	&persGrp=_Grp->getPersistent();
	
	if (persGrp.isRingGrp())
	{
		_AggroRange = persGrp.getAggroDist();
	}
	else
	{
		_AggroRange = 25;
	}
}

void CGrpProfileGuard::endProfile()
{
	PROFILE_LOG("group", "guard", "end", "");
	CGrpProfileNormal::endProfile();
}
	
void CGrpProfileGuard::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpGuardProfileUpdate);
	CFollowPathContext fpcGrpGuardProfileUpdate("GrpGuardProfileUpdate");

	CAIVision<CPersistentOfPhysical>	GuardVision;		
	const	uint32	aggroSize=uint32(_AggroRange);

	TTicks startVisionTime = CTime::getPerformanceTime();

	{
		H_AUTO(GrpGuardProfileVision);
		CAIVector	centerPos;
		if	(_Grp->calcCenterPos(centerPos))	// true if there's some bots in the group.
		{
			_Grp->setCenterPos(centerPos);
			if	(!_GroupFighting)
				_CenterPos=_Grp->getCenterPos();
			GuardVision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), _CenterPos, aggroSize, aggroSize);
		}
	}

	TTicks endVisionTime = CTime::getPerformanceTime();

	static uint32 s_maxBotsVisible = 0;
	static double s_maxBotsVisionTime = 0.0;
	
	uint32 numBotsVisible = (uint32)GuardVision.bots().size();
	double deltaVisionTime = CTime::ticksToSecond(endVisionTime-startVisionTime);
	bool bTellUs = false;
	if( s_maxBotsVisible < numBotsVisible )
	{
		s_maxBotsVisible = numBotsVisible;
		bTellUs = true;
	}

	if( s_maxBotsVisionTime < deltaVisionTime )
	{
		s_maxBotsVisionTime = deltaVisionTime;
		bTellUs = true;
	}

	if( bTellUs )
	{
		nldebug( "==> max bots visible is now %u", s_maxBotsVisible );
		nldebug( "vision time: %.2f", (float)(deltaVisionTime * 1000.0) );
	}
	
	uint32	factionIndex=CStaticFames::INVALID_FACTION_INDEX;
	sint32 fameForGuardAttack = FameForGuardAttack;
	{
		H_AUTO(GrpGuardProfileFaction);
		CAliasCont<CBot> &bots = _Grp->getPersistent().bots();
		if (!bots.size() != 0 && bots.begin() != bots.end())
		{
			CBot* bot = *(bots.begin());
			if (bot != NULL)
			{
				factionIndex=bot->getSheet()->FactionIndex();
				if (bot->getSheet()->FameForGuardAttack() != AISHEETS::ICreature::InvalidFameForGuardAttack)
					fameForGuardAttack = bot->getSheet()->FameForGuardAttack();
				
/*				if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
				{
					nlwarning("Bot sheet '%s' have invalid faction index (guard profile)", bot->getSheet()->SheetId().toString().c_str());
				}
*/
			}
		}
	}
	
	string s;
	float f = 0.f;
	if (_Grp->getProfileParameter("faction", s) && !s.empty())
	{
		factionIndex = CStaticFames::getInstance().getFactionIndex(s);
	}
	if (_Grp->getProfileParameter("fame_for_guard_attack", f))
	{
		fameForGuardAttack = (sint32)f;
	}
	
	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);

	// check if we are in war and if some bot are waiting for a bus.
	if	(_GroupFighting)
	{
		H_AUTO(GrpGuardProfileFighting);
		const	CAIVector	centerPos(_Grp->getCenterPos());

		// check if some bots are not fighting.
		for (CCont<CBot >::iterator it=_Grp->getPersistent().bots().begin(), itEnd=_Grp->getPersistent().bots().end();it!=itEnd;++it)
		{
			const	CBot*const	bot=*it;
			CSpawnBot	*const	spawnBot=bot->getSpawnObj();
			if (	spawnBot
				&&	spawnBot->isAlive())
			{
				switch(spawnBot->getAIProfileType())
				{
				case BOT_STAND_AT_POS:
					{
						CMoveProfile*const	moveProf=NLMISC::type_cast<CMoveProfile*>(_Grp->movingProfile().getAIProfile());
						if	(moveProf)
							moveProf->resumeBot(bot);
					}
					break;
				case BOT_FIGHT:
					{
						// This system is now managed by CBotAggroOwner itself
						/*
						const	CAIEntityPhysical*const	target=spawnBot->getTarget();
						if (target)
						{
							// if target is out of range, then forget the aggro.
							if (centerPos.quickDistTo(target->pos())>50)
							{
								spawnBot->forgetAggroFor(target->dataSetRow());
								_Grp->forgetAggroFor(target->dataSetRow());
							}
						}
						*/
					}
					break;
				default:
					break;
				}
			}
		}
	}
	
	//	guards don't like bandits.
	{
		H_AUTO(GrpGuardProfileBandits);
		const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &bots = GuardVision.bots();
		std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(bots.begin()), last(bots.end());
		for (; first != last; ++first)
		{
			const	CAIEntityPhysical *const	ep = (*first)->getSpawnObj();
			if	(ep->isAlive())
			{
				switch	(ep->getRyzomType())
				{
				case RYZOMID::npc:
					{
						const	CSpawnBotNpc *const	botNpc = NLMISC::safe_cast<const	CSpawnBotNpc*>(ep);

						if	(botNpc->spawnGrp().activityProfile().getAIProfileType() == ACTIVITY_BANDIT)
						{
							if (_CenterPos.quickDistTo(botNpc->pos())<aggroSize)
							{
								_Grp->setAggroMinimumFor(ep->dataSetRow(), 0.5f, false);
							}
						}
					}
					break;
				case RYZOMID::creature:
					{
						const	CSpawnBotFauna *const	botFauna = NLMISC::safe_cast<const	CSpawnBotFauna*>(ep);

						if (botFauna->getPersistent().faunaType()==FaunaTypePredator)
						{
							if (_CenterPos.quickDistTo(botFauna->pos())<aggroSize)
							{
								_Grp->setAggroMinimumFor(ep->dataSetRow(), 0.5f, false);
							}
						}
					}
					break;
				default:
					break;
				}
			}
		}
	}
	
	//	guards don't like bots that attack players.
	{
		H_AUTO(GrpGuardProfileAttack);
		const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &players = GuardVision.players();		
		LOG("%u players in group vision with aggrosize %d", players.size(), aggroSize);
		
		const	CAIVector	&centerPos=_Grp->getCenterPos();

		std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(players.begin()), last(players.end());
		for (; first != last; ++first)
		{
			const	CPersistentOfPhysical *const	player = (*first);
			CAIEntityPhysical *const	ep = player->getSpawnObj();
			if	(	!ep
				||	!ep->isAlive()
				||	ep->currentHitPoints()<=0.f
				||	ep->wpos().toAIVector().quickDistTo(centerPos)>aggroSize)
				continue;

			//	check Fame before choosing what to do ..
			sint32 const fame = ep->getFameIndexed(factionIndex);
			
			// if player is kos attack him (only if bot is attackable by player)
			if	((_Grp->getPersistent().getPlayerAttackable() || (factionIndex!=CStaticFames::INVALID_FACTION_INDEX && _Grp->getPersistent().isFactionAttackable(CStaticFames::getInstance().getFactionName(factionIndex), fame))) && ((factionIndex!=CStaticFames::INVALID_FACTION_INDEX && fame<fameForGuardAttack) || OUTPOSTHELPERS::isAttackingFaction(factionIndex, ep)))
			{
				// the guard attack the player !
				_Grp->setAggroMinimumFor(ep->dataSetRow(), 1.f, false);
				continue;
			}

			//	check if player is attacked and assist him).
			CAIEntityPhysical const* phys = ep->firstTargeter();
			while (phys)
			{
				switch(phys->getRyzomType())
				{
				case	RYZOMID::player:
					break;
				case	RYZOMID::npc:
					{
						const	CSpawnBotNpc	*const	botNpc=dynamic_cast<const	CSpawnBotNpc*>(phys);
						if	(	botNpc
							&&	botNpc->getPersistent().getGroup().getSpawnObj()->activityProfile().getAIProfileType() == ACTIVITY_GUARD)
						{
							break;
						}

						// guard don't attack npc of the same faction, rather, they attack the player !
						if (factionIndex != CStaticFames::INVALID_FACTION_INDEX && botNpc->getPersistent().getSheet()->FactionIndex() == factionIndex)
						{
							// the guard attack the player !
							_Grp->setAggroMinimumFor(ep->dataSetRow(), 1.f, false);
							break;
						}
					}
				default:
					// guard defend only player with a not too bad fame
					if (fame >= FameForGuardHelp && !OUTPOSTHELPERS::isAttackingFaction(factionIndex, ep))
						_Grp->setAggroMinimumFor(phys->dataSetRow(), 1.f, false);
					break;
				}
				phys = phys->nextTargeter();
			}
		}
	}
}
	
std::string CGrpProfileGuard::getOneLineInfoString() const
{
	return "guard profile";
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileTribu                                                    //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileTribu::CGrpProfileTribu(CProfileOwner *owner)
: CGrpProfileNormal(owner)
{
	PROFILE_LOG("group", "tribe", "ctor", "");
}

CGrpProfileTribu::~CGrpProfileTribu()
{
	PROFILE_LOG("group", "tribe", "dtor", "");
}
		
void CGrpProfileTribu::beginProfile()
{
	PROFILE_LOG("group", "tribe", "begin", "");
	CGrpProfileNormal::beginProfile();
}

void CGrpProfileTribu::endProfile()
{
	PROFILE_LOG("group", "tribe", "end", "");
	CGrpProfileNormal::endProfile();
}
	
void CGrpProfileTribu::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpTribuProfileUpdate);
	CFollowPathContext fpcGrpTribuProfileUpdate("GrpTribuProfileUpdate");

	CAIVision<CPersistentOfPhysical>	TribuVision;		
	const	uint32	aggroSize=40;

	breakable
	{
		CAIVector	centerPos;
		if	(!_Grp->calcCenterPos(centerPos))	// true if there's some bots in the group.
			break;

		_Grp->setCenterPos(centerPos);
		if	(!_GroupFighting)
			_CenterPos=_Grp->getCenterPos();
		TribuVision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), _CenterPos, aggroSize, aggroSize);
	}
	
	uint32	factionIndex=CStaticFames::INVALID_FACTION_INDEX;
	{
		CBotNpc* bot = static_cast<CBotNpc*>(*_Grp->getPersistent().bots().begin());
		factionIndex = bot->getSheet()->FactionIndex();
	}

	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);

	// check if we are in war and if some bot are waiting for a bus.
	if	(_GroupFighting)
	{
		CAIVector	centerPos(_Grp->getCenterPos());

		// check if some bots are not fighting.
		for (CCont<CBot >::iterator it=_Grp->getPersistent().bots().begin(), itEnd=_Grp->getPersistent().bots().end();it!=itEnd;++it)
		{
			const	CBot*const	bot=*it;
			CSpawnBot	*const	spawnBot=bot->getSpawnObj();
			if	(	!spawnBot
				||	!spawnBot->isAlive())
				continue;

			switch(spawnBot->getAIProfileType())
			{
			case BOT_STAND_AT_POS:
				{
					CMoveProfile	*moveProf=NLMISC::safe_cast<CMoveProfile*>(_Grp->movingProfile().getAIProfile());
					moveProf->resumeBot(bot);
				}							
				break;
			case BOT_FIGHT:
				{
					// This system is managed by CBotAggroOwner now
					/*
					const	CAIEntityPhysical	*const	target=spawnBot->getTarget();
					if	(!target)
						break;

					// if target is out of range, then forget the aggro.
					if (centerPos.quickDistTo(target->pos())<=50)
						break;

					spawnBot->forgetAggroFor(target->dataSetRow());
					_Grp->forgetAggroFor(target->dataSetRow());
					*/
				}
				break;
			default:
				break;
			}
		}
	}		

	//	Tribus don't like players with bad fame.
	{
		const std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > &players = TribuVision.players();
		
		LOG("%u players in group vision", players.size());
		
		std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator	first(players.begin()), last(players.end());
		for (; first != last; ++first)
		{
			CPersistentOfPhysical *const player = (*first);
			CAIEntityPhysical *const ep = player->getSpawnObj();
			if	(	!ep
				||	!ep->isAlive()
				||	ep->currentHitPoints()<=0.f)
				continue;

			const	CRootCell	*const	rootCell=ep->wpos().getRootCell();
			if	(	rootCell
				&&	rootCell->getFlag()!=0	)	//	Safe Zone ?
				continue;

			//	check Fame before choosing what to do ..
			sint32 const fame = ep->getFameIndexed(factionIndex);
			
			//	check if player is attacked.
			if	(fame<-10000 || OUTPOSTHELPERS::isAttackingFaction(factionIndex, ep))
			{
				_Grp->setAggroMinimumFor(ep->dataSetRow(), 1.f, false);
			}
		}
	}
}
	
std::string CGrpProfileTribu::getOneLineInfoString() const
{
	return "tribu profile";
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFollowRoute                                              //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileGoToPoint::CGrpProfileGoToPoint(CProfileOwner *owner, RYAI_MAP_CRUNCH::CWorldPosition const& startPos, RYAI_MAP_CRUNCH::CWorldPosition const& endPos, bool dontSendEvent)
: CMoveProfile(owner)
, _StartPos(startPos)
, _EndPos(endPos)
, _PathCont(NLMISC::safe_cast<CSpawnGroup*>(owner)->getPersistent().getAStarFlag())
, _DontSendEvent(dontSendEvent)
{
	PROFILE_LOG("group", "go_to_point", "ctor", "");
	_GlobalOrient.setX(1);
	_GlobalOrient.setY(0);
	_FollowForward=true;
	_ValidPosInit=false;
	_StopNpc = false;
}
	
void CGrpProfileGoToPoint::setDirection(bool forward)
{
	if	(	_FollowForward==forward
		&&	_ValidPosInit	)
		return;

	_ValidPosInit=true;
	_FollowForward=forward;
}

void CGrpProfileGoToPoint::beginProfile()
{
	PROFILE_LOG("group", "go_to_point", "begin", "");
	_ProfileTerminated = false;
	CMoveProfile::beginProfile();
	
	setCurrentDestination(_EndPos); // *
}

void	CGrpProfileGoToPoint::stateChangeProfile()
{
	setCurrentDestination(_EndPos); // *
	
	// set a stand at pos profile on every bots
	FOREACH(it, CAliasCont<CBot>, _Grp->bots())
	{
		CSpawnBot *sb = (*it)->getSpawnObj();
		if (sb)
			sb->setAIProfile(new CBotProfileStandAtPos(sb));
	}
	
	resumeProfile();
}

void CGrpProfileGoToPoint::endProfile()
{
	PROFILE_LOG("group", "go_to_point", "end", "");
}

void CGrpProfileGoToPoint::resumeProfile()
{
	PROFILE_LOG("group", "go_to_point", "resume", "");
	FOREACH(it, TBotFollowerMap, _NpcList)
	{
		const	CBot	*const	bot=it->first;
		CSpawnBot	*const	sbot=bot->getSpawnObj();
		if	(!sbot)
			continue;

		switch (sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			break;
		default:	//	push the correct comportment.
				sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
			break;
		}
		
	}
	setCurrentDestination(_EndPos);
}

void CGrpProfileGoToPoint::addBot(CBot* bot)
{
	_NpcList[bot] = CBotFollower();

	CSpawnBot	*sbot=bot->getSpawnObj();
	if (sbot)
		sbot->setAIProfile(new CBotProfileStandAtPos(sbot));

	_MustCalcRatios = true;
}

void CGrpProfileGoToPoint::removeBot(CBot* bot)
{
	TBotFollowerMap::iterator it=_NpcList.find(bot);

	if (it==_NpcList.end())
		return;

	CSpawnBotNpc	*const	spawnBot=NLMISC::safe_cast<CBotNpc*>(bot)->getSpawn();
	if (spawnBot)
		spawnBot->setAIProfile(BotProfileStandAtPosFactory.createAIProfile(spawnBot));
	
	_NpcList.erase	(it);
	_MustCalcRatios = true;
}

void CGrpProfileGoToPoint::setCurrentDestination(RYAI_MAP_CRUNCH::CWorldPosition const& dest)
{
	_PathCont.setDestination(dest);
	
	FOREACH(it, TBotFollowerMap, _NpcList)
		it->second.setBotAtDest(false);
}

void CGrpProfileGoToPoint::calcRatios()
{
	_MustCalcRatios = false;

	// loop to compute max speeds
	_MaxRunSpeed  = FLT_MAX;
	_MaxWalkSpeed = FLT_MAX;
	FOREACH(it, TBotFollowerMap, _NpcList)
	{
		CBot			*bot = it->first;	//static_cast<CBotNpc*>(_Grp->bots()[botFollower.getIndex()]);
		CSpawnBot	*sbot = bot->getSpawnObj();
		if	(	!sbot
			||	!sbot->isAlive())
			continue;

		_MaxRunSpeed  = std::min(sbot->runSpeed(), _MaxRunSpeed);
		_MaxWalkSpeed = std::min(sbot->walkSpeed(), _MaxWalkSpeed);
	}


	if (_Shape!=SHAPE_RECTANGLE)
		return;
	
	const	uint32	nbbots=(uint32)_NpcList.size();
	
	_NbRange	= (uint32)	sqrt(_Ratio*nbbots);
	if (_NbRange==0)
		_NbRange=1;
	_NbLines = nbbots/_NbRange;
	_NbBotInNormalShape = _NbLines*_NbRange;
	_Rest = nbbots-_NbBotInNormalShape;
			
	_Cx=(double(_NbRange)-1.0)*0.5;
	_Cy=(double(_NbLines)-1.0)*0.5;
	_Cy=(_Cy*_NbBotInNormalShape+double(_NbLines)*_Rest)/double(nbbots);
}	

void CGrpProfileGoToPoint::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(CGrpGoToPointProfileUpdate);
	CFollowPathContext fpcCGrpGoToPointProfileUpdate("CGrpGoToPointProfileUpdate");

	CMoveProfile::updateProfile(ticksSinceLastUpdate);
	
	CSpawnGroupNpc	*NpcGrp=NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp.ptr());
	CGroupNpc			&persGrp=NpcGrp->getPersistent();
		
	CAIVector	groupPosition	=	NpcGrp->getCenterPos();
	CAIVector	perpGlobalOrient;
	NpcGrp->calcCenterPos(groupPosition);
			
	uint32	nbAtDest=0;
	uint32	nbNewAtDest=0;

	uint32	botIndex=0;
	uint32	xIndex=0;
	uint32	yIndex=0;

	double	dx=0;
	double	dy=0;


	if	(_Shape==SHAPE_RECTANGLE)
	{
		perpGlobalOrient.setX(-_GlobalOrient.y());
		perpGlobalOrient.setY(_GlobalOrient.x());
	}

	//////////////////////////////////////////////////////////////////////////
	//	Calcs the correct gravity grid position (must be done only when bot are removed or add to the group.
	if (_MustCalcRatios)
		calcRatios	();
	

	FOREACH(it, TBotFollowerMap, _NpcList)
	{
		CBotFollower	&botFollower=it->second;
		if	(botFollower.isBotAtDest())
		{
			nbAtDest++;
			continue;
		}

		CBot			*bot=it->first;
		CSpawnBot	*sbot=bot->getSpawnObj();
		if	(	!sbot
			||	!sbot->isAlive())
			continue;

		// verify if the bot has a correct profile and if he reached the destination position.
		switch	(sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			{
				if (_ProfileTerminated)
				{
					// remove the profile
					sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
				}
				else
				{
					CBotProfileFollowPos* prof = NLMISC::safe_cast<CBotProfileFollowPos*>(sbot->getAIProfile());
					
					// flag the sub profile to stop the npc
					prof->setStop(_StopNpc);
					if (!_StopNpc)
					{
						if (prof->_Status==CFollowPath::FOLLOW_ARRIVED)
						{
							botFollower.setBotAtDest();
							nbNewAtDest++;
							nbAtDest++;
						}
						else
						{
							// update speeds
							prof->setMaxSpeeds(_MaxWalkSpeed, _MaxRunSpeed);
						}
					}
				}
			}
			break;
		default:	//	push the correct comportment.
			{
				if (!_ProfileTerminated)
				{
					sbot->setAIProfile(new CBotProfileFollowPos(&_PathCont, sbot));
					CBotProfileFollowPos* prof = NLMISC::safe_cast<CBotProfileFollowPos*>(sbot->getAIProfile());
					// update speeds
					prof->setMaxSpeeds(_MaxWalkSpeed, _MaxRunSpeed);
				}
			}
			break;
		}					

		if	(_Shape==SHAPE_RECTANGLE)
		{
			NLMISC::CVector2d	dir=sbot->theta().asVector2d();
			dx+=dir.x;
			dy+=dir.y;
			
			// 4 rows
			CAIVector	idealPos=groupPosition;
			if (botIndex>=_NbBotInNormalShape)
			{
				idealPos += perpGlobalOrient * (_XSize*(_Cx-double(xIndex)-(_NbRange-_Rest)*0.5));
			}
			else
			{
				idealPos += perpGlobalOrient * (_XSize*(_Cx-double(xIndex)));
			}
			
			idealPos+=_GlobalOrient*(_YSize*(_Cy-(double)yIndex));
			idealPos-=CAIVector(sbot->pos());
			
			botIndex++;
			xIndex++;
			if (xIndex>=_NbRange)
			{
				xIndex=0;
				yIndex++;
			}
			sbot->setMoveDecalage(idealPos);
		}
	}

	if	(_Shape==SHAPE_RECTANGLE)
	{
		_GlobalOrient.setX(dx/botIndex);
		_GlobalOrient.setY(dy/botIndex);
	}

	// first to arrived ?
	if	(nbAtDest>0 && !_ProfileTerminated)
	{
		if (!_DontSendEvent)
		{
			persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedFirst);
			persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedAll);
		}
		_ProfileTerminated = true;
	}
}
	
CGrpProfileGoToPoint::~CGrpProfileGoToPoint()
{
	for (CCont<CBot >::iterator it=_Grp->bots().begin(), itEnd=_Grp->bots().end();it!=itEnd;++it)
		removeBot(*it);
}

CGrpProfileGoToPoint::CBotFollower::CBotFollower()
: _BotAtDest(false)
{
}

CGrpProfileGoToPoint::CBotFollower::~CBotFollower()
{
}

void CGrpProfileGoToPoint::CBotFollower::setBotAtDest(bool atDest)
{
	_BotAtDest = atDest;
}

const bool& CGrpProfileGoToPoint::CBotFollower::isBotAtDest() const
{
	return _BotAtDest;
}

bool CGrpProfileGoToPoint::getDirection()
{
	return _FollowForward;
}


CPathCont* CGrpProfileGoToPoint::getPathCont(CBot const* bot)
{
	return &_PathCont;
}

std::string CGrpProfileGoToPoint::getOneLineInfoString() const
{
	std::string info = "go_to_point profile";
	info += " stop_npc=" + NLMISC::toString(_StopNpc);
	return info;
}

bool CGrpProfileGoToPoint::profileTerminated() const
{
	return _ProfileTerminated;
}

void CGrpProfileGoToPoint::stopNpc(bool stop)
{
	_StopNpc = stop;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileGoToPoint                                                   //
//////////////////////////////////////////////////////////////////////////////

//RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileGoToPoint, "goto_point");

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFollowRoute                                              //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileFollowRoute::CGrpProfileFollowRoute(CProfileOwner *owner)
: CMoveProfile(owner)
, _PathCont(NLMISC::safe_cast<CSpawnGroup*>(owner)->getPersistent().getAStarFlag())
, _DontSendEvent(false)
{
	PROFILE_LOG("group", "follow_route", "ctor", "");
	_GlobalOrient.setX(1);
	_GlobalOrient.setY(0);
	_FollowForward = true;
	_StopNpc = false;
	
	CGroupNpc	&grp=*safe_cast<CGroupNpc*>(&_Grp->getPersistent());
	if	(grp.getState())
	{
		const	CAIState *const	state = grp.getCAIState();
		if	(state->isPositional())
		{
			const	CAIStatePositional	*const	statePositionnal=static_cast<const	CAIStatePositional*>(state);
			_Geometry=&statePositionnal->shape().getGeometry();
			_GeometryComeFromState = true;
			_VerticalPos=statePositionnal->shape().getVerticalPos();
			return;
		}
	}
	_GeometryComeFromState = false;
	_Geometry = NULL;
	_VerticalPos = TVerticalPos();
	_GeomIndex = 0;
#ifdef NL_DEBUG
	nlassert(true==false);	//	Cannot use this constructor outside machine state context.
#endif
}


CGrpProfileFollowRoute::CGrpProfileFollowRoute(CProfileOwner *owner,const std::vector<CShape::TPosition>	&geometry,const	TVerticalPos	&verticalPos, bool dontSendEvent)
: CMoveProfile(owner)
, _PathCont(NLMISC::safe_cast<CSpawnGroup*>(owner)->getPersistent().getAStarFlag())
, _GeometryComeFromState(false)
, _Geometry(&geometry)
, _VerticalPos(verticalPos)
, _DontSendEvent(dontSendEvent)
{
	PROFILE_LOG("group", "follow_route", "ctor2", "");
	_GlobalOrient.setX(1);
	_GlobalOrient.setY(0);
	_FollowForward=true;
	_ValidPosInit=false;
	_GeomIndex=0;
	_StopNpc = false;
}
	
void CGrpProfileFollowRoute::setDirection(bool forward)
{
	if	(	_FollowForward==forward
		&&	_ValidPosInit	)
		return;

	_ValidPosInit=true;
	_FollowForward=forward;
#ifdef NL_DEBUG
	nlassert(_Geometry);
#endif
	setCurrentValidPos(_VerticalPos);
}

void CGrpProfileFollowRoute::beginProfile()
{
	PROFILE_LOG("group", "follow_route", "begin", "");
	_ProfileTerminated = false;
	CMoveProfile::beginProfile();

	if (_GeometryComeFromState)
		assignGeometryFromState();
}

void	CGrpProfileFollowRoute::assignGeometryFromState()
{
	_ProfileTerminated = false;

	//	default value initialization.
	std::string		shape;
	_Shape	=	SHAPE_NOTHING;
	_XSize	=	1;
	_YSize	=	1;
	_Ratio	=	1;
	
	if	(_Grp->getProfileParameter("shape", shape)
		&&	shape=="rectangle")
	{
		_Shape = SHAPE_RECTANGLE;
	}
	
	_Grp->getProfileParameter("ratio", _Ratio);
	_Grp->getProfileParameter("xsize", _XSize);
	_Grp->getProfileParameter("ysize", _YSize);

	_GeomIndex=0;
	{
		CGroup	&persGrp=NLMISC::safe_cast<CSpawnGroup*>(_Grp.ptr())->getPersistent();
		// R2_PRIMITIVE_LAXITY
		if (IsRingShard.get())
		{
			nlassertex(_Geometry, ("CGrpProfileFollowRoute : NULL geometry data for group '%s'", persGrp.getFullName().c_str()));
			if (_Geometry->empty())
			{
				nlwarning("CGrpProfileFollowRoute : missing geometry data for group '%s'", persGrp.getFullName().c_str());
			}
		}
		else
		{
			nlassertex(_Geometry && _Geometry->size()>0, ("CGrpProfileFollowRoute : missing geometry data for group '%s'", persGrp.getFullName().c_str()));
		}
	}

	CSpawnGroupNpc *grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup *>(_Grp));
	CGroupNpc &pgrp = grp->getPersistent();
	const	CAIState	*const	state = pgrp.getActiveState();

	if	(	!state
		||	!state->isPositional())
		return;
	
	const	CAIStatePositional	*const	sp = static_cast<const	CAIStatePositional	*const>(state);
	if	(!sp->shape().hasPoints() && (_GeometryComeFromState))
	{
		nlwarning("Error, no position in state '%s'%s", 
			sp->getAliasFullName().c_str(),
			sp->getAliasString().c_str());
	}
	else
	{
		_Geometry = &(sp->shape().getGeometry());
		_GeometryComeFromState = true;
		_MustCalcRatios = true;
	}

	setCurrentValidPos	(_VerticalPos);	//	static_cast<CSpawnGroupNpc*>(_Grp.ptr())->getPersistent().getGeometryVerticalPos(), geometry);
}

void CGrpProfileFollowRoute::stateChangeProfile()
{
	assignGeometryFromState();
	
	// set a stand at pos profile on every bots
	FOREACH(it, CAliasCont<CBot>, _Grp->bots())
	{
		CSpawnBot *sb = (*it)->getSpawnObj();
		if (sb)
			sb->setAIProfile(new CBotProfileStandAtPos(sb));
	}
	// Reset stop flag to false
	stopNpc(false);
	
	resumeProfile();
}

void CGrpProfileFollowRoute::endProfile()
{
	PROFILE_LOG("group", "follow_route", "end", "");
}

void CGrpProfileFollowRoute::resumeProfile()
{
	PROFILE_LOG("group", "follow_route", "resume", "");
	FOREACH(it, TBotFollowerMap, _NpcList)
	{
		const	CBot	*const	bot=it->first;
		CSpawnBot	*const	sbot=bot->getSpawnObj();
		if	(!sbot)
			continue;

		switch	(sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			break;
		default:	//	push the correct comportment.
				sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
			break;
		}
		
	}
//	setCurrentValidPos	(NLMISC::safe_cast<CAIStatePositional *>(NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp)->getPersistent().getCAIState()));
#ifdef NL_DEBUG
	nlassert(_Geometry);
#endif
	setCurrentValidPos	(_VerticalPos);
}

void	CGrpProfileFollowRoute::addBot	(CBot	*bot)
{
	_NpcList[bot]=CBotFollower();

	CSpawnBot	*sbot=bot->getSpawnObj();
	if (sbot)
		sbot->setAIProfile(new CBotProfileStandAtPos(sbot));

	_MustCalcRatios = true;
}

void	CGrpProfileFollowRoute::removeBot	(CBot	*bot)
{
	TBotFollowerMap::iterator it=_NpcList.find(bot);

	if (it==_NpcList.end())
		return;

	CSpawnBotNpc	*const	spawnBot=NLMISC::safe_cast<CBotNpc*>(bot)->getSpawn();
	if (spawnBot)
		spawnBot->setAIProfile(BotProfileStandAtPosFactory.createAIProfile(spawnBot));
	
	_NpcList.erase	(it);
	_MustCalcRatios = true;
}

void	CGrpProfileFollowRoute::setCurrentValidPos	(TVerticalPos verticalPos)
{
	if (_Geometry->size()>0)
	{
#if !FINAL_VERSION
		nlassert(_Geometry!=NULL);
#endif
		size_t index = getDirection()?_GeomIndex:(_Geometry->size()-_GeomIndex-1);
#if !FINAL_VERSION
		nlassert(index < _Geometry->size());
#endif
		_PathCont.setDestination	(verticalPos, (*_Geometry)[index]);
	}
	
	FOREACH(it, TBotFollowerMap, _NpcList)
		it->second.setBotAtDest(false);
}

void	CGrpProfileFollowRoute::calcRatios	()
{
	_MustCalcRatios = false;

	// loop to compute max speeds
	_MaxRunSpeed  = FLT_MAX;
	_MaxWalkSpeed = FLT_MAX;
	FOREACH(it, TBotFollowerMap, _NpcList)
	{
		CBot			*bot = it->first;	//static_cast<CBotNpc*>(_Grp->bots()[botFollower.getIndex()]);
		CSpawnBot	*sbot = bot->getSpawnObj();
		if	(	!sbot
			||	!sbot->isAlive())
			continue;

		_MaxRunSpeed  = std::min(sbot->runSpeed(), _MaxRunSpeed);
		_MaxWalkSpeed = std::min(sbot->walkSpeed(), _MaxWalkSpeed);
	}


	if	(_Shape!=SHAPE_RECTANGLE)
		return;

	const	uint32	nbbots=(uint32)_NpcList.size();

	_NbRange	= (uint32)	sqrt(_Ratio*nbbots);
	if (_NbRange==0)
		_NbRange=1;
	_NbLines = nbbots/_NbRange;
	_NbBotInNormalShape = _NbLines*_NbRange;
	_Rest = nbbots-_NbBotInNormalShape;
			
	_Cx=(double(_NbRange)-1.0)*0.5;
	_Cy=(double(_NbLines)-1.0)*0.5;
	_Cy=(_Cy*_NbBotInNormalShape+double(_NbLines)*_Rest)/double(nbbots);
}	

void CGrpProfileFollowRoute::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpFollowRouteProfileUpdate);
	CFollowPathContext fpcGrpFollowRouteProfileUpdate("GrpFollowRouteProfileUpdate");

	CMoveProfile::updateProfile(ticksSinceLastUpdate);
	
	CSpawnGroupNpc	*NpcGrp=NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp.ptr());
	CGroupNpc			&persGrp=NpcGrp->getPersistent();
		
	CAIVector	groupPosition	=	NpcGrp->getCenterPos();
	CAIVector	perpGlobalOrient;
	NpcGrp->calcCenterPos(groupPosition);
			
	uint32	nbAtDest=0;
	uint32	nbNewAtDest=0;

	uint32	botIndex=0;
	uint32	xIndex=0;
	uint32	yIndex=0;

	double	dx=0;
	double	dy=0;

	// R2_PRIMITIVE_LAXITY
	if (IsRingShard.get())
	{
		if (!_ProfileTerminated && _Geometry->empty())
			_ProfileTerminated = true;
	}

	if	(_Shape==SHAPE_RECTANGLE)
	{
		perpGlobalOrient.setX(-_GlobalOrient.y());
		perpGlobalOrient.setY(_GlobalOrient.x());
	}

	//////////////////////////////////////////////////////////////////////////
	//	Calcs the correct gravity grid position (must be done only when bot are removed or add to the group.
	if (_MustCalcRatios)
		calcRatios	();
	

	FOREACH(it, TBotFollowerMap, _NpcList)
	{
		CBotFollower	&botFollower=it->second;
		if	(botFollower.isBotAtDest())
		{
			nbAtDest++;
			continue;
		}

		CBot			*bot=it->first;
		CSpawnBot	*sbot=bot->getSpawnObj();
		if	(	!sbot
			||	!sbot->isAlive())
			continue;

		// verify if the bot has a correct profile and if he reached the destination position.
		switch	(sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			{
				if (_ProfileTerminated)
				{
					// remove the profile
					sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
				}
				else
				{
					CBotProfileFollowPos* prof = NLMISC::safe_cast<CBotProfileFollowPos*>(sbot->getAIProfile());
					
					// flag the sub profile to stop the npc
					prof->setStop(_StopNpc);
					if (!_StopNpc)
					{
						if (prof->_Status==CFollowPath::FOLLOW_ARRIVED)
						{
							botFollower.setBotAtDest();
							nbNewAtDest++;
							if (simulateBug(2))
							{
								/* Following statement was missing */
							}
							else
							{
								nbAtDest++;
							}
						}
						else
						{
							// update speeds
							prof->setMaxSpeeds(_MaxWalkSpeed, _MaxRunSpeed);
						}
					}
				}
			}
			break;
		default:	//	push the correct comportment.
			{
				if (!_ProfileTerminated)
				{
					sbot->setAIProfile(new CBotProfileFollowPos(&_PathCont, sbot));
					CBotProfileFollowPos* prof = NLMISC::safe_cast<CBotProfileFollowPos*>(sbot->getAIProfile());
					// update speeds
					prof->setMaxSpeeds(_MaxWalkSpeed, _MaxRunSpeed);
				}
			}
			break;
		}					

		if	(_Shape==SHAPE_RECTANGLE)
		{
			NLMISC::CVector2d	dir=sbot->theta().asVector2d();
			dx+=dir.x;
			dy+=dir.y;
			
			// 4 rows
			CAIVector	idealPos=groupPosition;
			if (botIndex>=_NbBotInNormalShape)
			{
				idealPos += perpGlobalOrient * (_XSize*(_Cx-double(xIndex)-(_NbRange-_Rest)*0.5));
			}
			else
			{
				idealPos += perpGlobalOrient * (_XSize*(_Cx-double(xIndex)));
			}
			
			idealPos+=_GlobalOrient*(_YSize*(_Cy-(double)yIndex));
			idealPos-=CAIVector(sbot->pos());
			
			botIndex++;
			xIndex++;
			if (xIndex>=_NbRange)
			{
				xIndex=0;
				yIndex++;
			}
			sbot->setMoveDecalage(idealPos);
		}
	}

	if	(_Shape==SHAPE_RECTANGLE)
	{
		_GlobalOrient.setX(dx/botIndex);
		_GlobalOrient.setY(dy/botIndex);
	}

	// first to arrived ?
	if	(nbAtDest>0 && !_ProfileTerminated)
	{
		// oh la la (la, let 's go dancing)..
		_GeomIndex++;			
#ifdef NL_DEBUG
		nlassert(_Geometry);
#endif

		if	(_GeomIndex>=_Geometry->size())	//	we reach the end.
		{
			_GeomIndex=0;

			if (!_DontSendEvent)
			{
				persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedFirst);
				persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedAll);
			}
			_ProfileTerminated = true;
		}
		else
		{
			setCurrentValidPos(_VerticalPos);
		}
	}
}
	
//////////////////////////////////////////////////////////////////////////////
// CGrpProfileStandOnVertices                                          //
//////////////////////////////////////////////////////////////////////////////

CPathCont* CGrpProfileStandOnVertices::getPathCont(CBot const* bot)
{
	TNpcBotPositionnerMap::const_iterator it=_NpcList.find(bot);
	if	(it==_NpcList.end())
		return	NULL;
	return	&it->second->_PathCont;
}
	
void CGrpProfileStandOnVertices::beginProfile()
{
	PROFILE_LOG("group", "stand_on_vertices", "begin", "");
	CMoveProfile::beginProfile();
	CSpawnGroupNpc	*NpcGrp=NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp.ptr());
	
	CAIStatePositional *grpState=NLMISC::safe_cast<CAIStatePositional *>(NpcGrp->getPersistent().getCAIState());		

	if	(	!grpState
		||	grpState->shape().numPoints() == 0
		||	!grpState->isPositional())
	{
		if (grpState)
			nlwarning("CGrpProfileStandOnVertices::beginProfile : grpState without valid points %s for group %s", grpState->getAliasFullName().c_str(), NpcGrp->getPersistent().getFullName().c_str());
		else
			nlwarning("CGrpProfileStandOnVertices::beginProfile : invalid no grpState for group %s", NpcGrp->getPersistent().getFullName().c_str());
	}
	setCurrentValidPos	(grpState);
	_Finished=false;
}

void CGrpProfileStandOnVertices::setCurrentValidPos(CAIStatePositional *grpState)
{
	for (TNpcBotPositionnerMap::iterator it=_NpcList.begin(), itEnd=_NpcList.end();it!=itEnd;++it)
	{
		CBotPositionner	*botPos=(*it).second;
		botPos->setBotAtDest(false);
		botPos->_PathCont.setDestination(grpState->shape().getVerticalPos(), *grpState->shape().point(botPos->_GeomIndex));
	}
}

void	CGrpProfileStandOnVertices::resumeProfile()
{
	PROFILE_LOG("group", "stand_on_vertices", "resume", "");
	for (TNpcBotPositionnerMap::iterator it=_NpcList.begin(), itEnd=_NpcList.end();it!=itEnd;++it)
	{
		const	CBot*const	bot=(*it).first;
		CSpawnBot	*sbot=bot->getSpawnObj();
		if	(!sbot)
			continue;

		switch	(sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			break;
		default:	//	push the correct behaviour
			{					
				sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
			}
			break;
		}
		
	}

	CSpawnGroupNpc	*NpcGrp=NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp.ptr());
	CAIStatePositional *grpState=NLMISC::safe_cast<CAIStatePositional *>(NpcGrp->getPersistent().getCAIState());
	nlassert(	grpState->shape().numPoints()>0
		&&	grpState
		&&	grpState->isPositional());
	setCurrentValidPos	(grpState);		
	_Finished=false;
}
	
void	CGrpProfileStandOnVertices::addBot	(CBot	*bot)
{
	CGroupNpc&	grp=NLMISC::safe_cast<CBotNpc*>(bot)->grp();
#ifdef NL_DEBUG
	nlassert(grp.getSpawnObj());
#endif
	
	CAIStatePositional *grpState=NLMISC::safe_cast<CAIStatePositional *>(grp.getCAIState());

	CSpawnBot	*const	sbot=bot->getSpawnObj();
	
	if ( grpState->shape().numPoints() < 1 )
	{
		nlwarning("CGrpProfileStandOnVertices : group state '%s'%s: no vertice !", grpState->getAliasFullName().c_str(),  grpState->getAliasString().c_str());
		return;
	}
	
	if	(sbot)
		sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
	
	_NpcList[bot]=new	CBotPositionner	(bot->getChildIndex()%grpState->shape().numPoints(), bot->getGroup().getAStarFlag());
}
	
void	CGrpProfileStandOnVertices::removeBot	(CBot	*bot)
{
	TNpcBotPositionnerMap::iterator it=_NpcList.find(bot);
	if (it!=_NpcList.end())
	{
		CSpawnBotNpc	*spawnBot=NLMISC::safe_cast<CBotNpc*>(bot)->getSpawn();
		if	(spawnBot)
			spawnBot->setAIProfile(new CBotProfileStandAtPos(spawnBot));

		_NpcList.erase	(it);
	}

}

void CGrpProfileStandOnVertices::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpStandProfileUpdate);
	CFollowPathContext fpcGrpStandProfileUpdate("GrpStandProfileUpdate");

	CMoveProfile::updateProfile(ticksSinceLastUpdate);
	
	CGroupNpc				&persGrp=NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp.ptr())->getPersistent();
	CAIStatePositional	*grpState=static_cast<CAIStatePositional*>(persGrp.getCAIState());
	
	uint32	nbAtDest=0;
	uint32	nbNewAtDest=0;
	
	for (TNpcBotPositionnerMap::iterator it=_NpcList.begin(), itEnd=_NpcList.end();it!=itEnd;++it)
	{
		CBotPositionner	*botPos=(*it).second;
		if	(!botPos->isBotAtDest())
		{
			const	CBot*const	bot=(*it).first;
			CSpawnBot	*sbot=bot->getSpawnObj();
			if	(	sbot
				&&	sbot->isAlive())
			{
				switch	(sbot->getAIProfileType())
				{
				case BOT_FOLLOW_POS:
					{
						CBotProfileFollowPos* prof = NLMISC::safe_cast<CBotProfileFollowPos*>(sbot->getAIProfile());
						
						if (prof->_Status==CFollowPath::FOLLOW_ARRIVED)
						{
							sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
							botPos->setBotAtDest();
							nbNewAtDest++;
							if (simulateBug(2))
							{
								/* Following statement was missing */
							}
							else
							{
								nbAtDest++;
							}
						}
					}
					break;
				default:	//	push the correct comportment.
					{
						sbot->setAIProfile(new CBotProfileFollowPos(&botPos->_PathCont, sbot));
					}
					break;
				}
			}
		}
		else
		{
			nbAtDest++;
		}
	}
	
	// first to arrived ?
	if	(	nbNewAtDest==nbAtDest
		&&	nbAtDest>0	)
	{
		persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedFirst);
	}
	
	// all arrived ?
	if	(	nbAtDest==_NpcList.size()
		&&	!_Finished	)
	{
		persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedAll);
		_Finished=true;
	}
}
	
//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFollowPlayer                                                //
//////////////////////////////////////////////////////////////////////////////
CGrpProfileFollowPlayer::CGrpProfileFollowPlayer(CProfileOwner* owner, TDataSetRow const& playerRow, uint32 dispersionRadius)
: CMoveProfile(owner)
, _PlayerRow(playerRow)
, _DispersionRadius(dispersionRadius)
, _PathPos(CAngle(0))
, _PathCont(NLMISC::safe_cast<CSpawnBotNpc*>(owner)->getAStarFlag())
{
	PROFILE_LOG("group", "follow player", "ctor", "");
	_Status = CFollowPath::FOLLOWING;
}

bool CGrpProfileFollowPlayer::destinationReach()	const
{
	return	_Status == CFollowPath::FOLLOW_ARRIVED
		||	_Status==CFollowPath::FOLLOW_NO_PATH;
}

void CGrpProfileFollowPlayer::beginProfile()
{
	_Status = CFollowPath::FOLLOWING;
}

// TODO: this doesn't work very well at all...
void CGrpProfileFollowPlayer::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(CGrpProfileFollowPlayerUpdate);
	CFollowPathContext fpcGrpFollowPlayerUpdate("CGrpProfileFollowPlayerUpdate");

	// check all bot to see if there need to move 
	CSpawnGroupNpc* grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup*>(_Grp));
	CGroupNpc &pgrp = grp->getPersistent();
	
	CBotPlayer*	plrPtr	=	dynamic_cast<CBotPlayer*>(CAIS::instance().getEntityPhysical(_PlayerRow));

	if ( ! plrPtr) {
		nlwarning("CGrpProfileFollowPlayer: No valid player position to follow");
		return;
	}

	_PathCont.setDestination(plrPtr->wpos());
	_PathPos._Angle = plrPtr->theta();

	for (uint i = 0; i < pgrp.bots().size(); ++i)
	{
		CBotNpc* bot = static_cast<CBotNpc*>(pgrp.bots()[i]);
		if (!bot)
			continue;

		// check current bot state
		CSpawnBotNpc *sbot = bot->getSpawn();
		if (!sbot)
			continue;

		// Need to wait for a correct position before moving?
		CAIVector const& dest = _PathCont.getDestination();
		if (dest.x()==0 || dest.y()==0)
			return;
		
		static const std::string runParameter("running");
		float	dist;
		if (sbot->getPersistent().getOwner()->getSpawnObj()->checkProfileParameter(runParameter))
			dist = sbot->runSpeed()*ticksSinceLastUpdate;		
		else
			dist = sbot->walkSpeed()*ticksSinceLastUpdate;

		// Move
		CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
			sbot,
			_PathPos,
			_PathCont,
			dist,
			0.f,
			0.5f);

		if (status==CFollowPath::FOLLOW_NO_PATH)
		{
			nlwarning("Problem with following player");
		}

		
	}	
}



//////////////////////////////////////////////////////////////////////////////
// CGrpProfileIdle                                                     //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileIdle::CGrpProfileIdle(CProfileOwner* owner)
: CMoveProfile(owner)
{
	PROFILE_LOG("group", "idle", "ctor", "");
}

CGrpProfileIdle::~CGrpProfileIdle()
{
	PROFILE_LOG("group", "idle", "dtor", "");
	FOREACH(it, CCont<CBot>, _Grp->bots())
	{
		CBot* bot = *it;
		removeBot(bot);
	}
}

CGrpProfileIdle::CBotPositionner::CBotPositionner()
{
}

CGrpProfileIdle::CBotPositionner::~CBotPositionner()
{
}

void CGrpProfileIdle::beginProfile()
{
	PROFILE_LOG("group", "idle", "begin", "");
	CMoveProfile::beginProfile();
}

CPathCont* CGrpProfileIdle::getPathCont(CBot const* bot)
{
	return NULL;
}

void CGrpProfileIdle::resumeProfile()
{
	PROFILE_LOG("group", "idle", "resume", "");
	typedef std::map<CBot*,CBotPositionner> TCont;
	FOREACH(it, TCont, _NpcList)
	{
		CBot* bot = (*it).first;
		CSpawnBot* sbot = bot->getSpawnObj();
		if (sbot)
		{
			switch (sbot->getAIProfileType())
			{
			case BOT_STAND_AT_POS:
				break;
			default:	//	push the correct comportment.
				sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
				break;
			}
		}
	}
}

void CGrpProfileIdle::addBot(CBot* bot)
{
#ifdef NL_DEBUG
	CGroupNpc&	grp=NLMISC::safe_cast<CBotNpc*>(bot)->grp();
	nlassert(grp.getSpawnObj());
#endif
	_NpcList[bot]=CBotPositionner	();
	CSpawnBot	*sbot=bot->getSpawnObj();
	if (sbot)
		sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
}

void CGrpProfileIdle::removeBot(CBot* bot)
{
	std::map<CBot*, CBotPositionner>::iterator it=_NpcList.find(bot);
	if (it!=_NpcList.end())
	{
		_NpcList.erase	(it);
	}

}

void CGrpProfileIdle::endProfile()
{
	PROFILE_LOG("group", "idle", "end", "");
}

void CGrpProfileIdle::updateProfile(uint ticksSinceLastUpdate)
{
	CMoveProfile::updateProfile(ticksSinceLastUpdate);
}

std::string CGrpProfileIdle::getOneLineInfoString() const
{
	return "idle profile";
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileWander                                                   //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileWander::CGrpProfileWander(CProfileOwner* owner, CNpcZone const* npcZone)
: CMoveProfile(owner)
, _Social(false)
, _NpcZone(npcZone)
{
	PROFILE_LOG("group", "wander", "ctor", "");
	_BotStandProfileType = BOT_STAND_AT_POS;
	_BotStandProfileFactory = &BotProfileStandAtPosFactory;
	_RandomPos=&npcZone->getPlaceRandomPos();
}

CGrpProfileWander::CGrpProfileWander(CProfileOwner* owner)
: CMoveProfile(owner)
, _Social(false)
{
	PROFILE_LOG("group", "wander", "ctor2", "");
	affectZoneFromStateMachine();
#if !FINAL_VERSION
	// R2_PRIMITIVE_LAXITY
	if (!IsRingShard.get())
	{
		nlassert(!_RandomPos.isNULL());
	}
#endif

	// default to stand apt pos profile
	_BotStandProfileType = BOT_STAND_AT_POS;
	_BotStandProfileFactory = &BotProfileStandAtPosFactory;
}

CGrpProfileWander::~CGrpProfileWander()
{
	PROFILE_LOG("group", "wander", "dtor", "");
}

void CGrpProfileWander::stateChangeProfile()
{
	affectZoneFromStateMachine();
	resetDestinationReachedData();
}

void CGrpProfileWander::affectZoneFromStateMachine()
{	
	CSpawnGroupNpc* grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup *>(_Grp));
	CGroupNpc& pgrp = grp->getPersistent();
	CAIState const* const state = pgrp.getActiveState();
	
	if (	!state
		||	!state->isPositional())
		return;
	
	CAIStatePositional const* const sp = static_cast<CAIStatePositional const* const>(state);
	if	(!sp->shape().hasPoints())
	{
		if (sp->getAliasNode())
			nlwarning("Error, no position in state %s", sp->getAliasNode()->fullName().c_str());
		else
			nlwarning("Error, no position in state %s", sp->getName().c_str());
	}
	if (sp->shape().hasPatat())
		_RandomPos=&(sp->shape());
}

void CGrpProfileWander::resetDestinationReachedData()
{
	_DestinationReachedFirst = false;
	_DestinationReachedAll = false;
	std::fill(_NpcDestinationReached.begin(), _NpcDestinationReached.end(), false);
}

void CGrpProfileWander::setBotStandProfile(TProfiles	botStandProfileType, IAIProfileFactory *botStandProfileFactory)
{
	_BotStandProfileType = botStandProfileType;
	_BotStandProfileFactory = botStandProfileFactory;
}

void CGrpProfileWander::beginProfile()
{
	PROFILE_LOG("group", "wander", "begin", "");
	CMoveProfile::beginProfile();

	CSpawnGroupNpc *grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup *>(_Grp));

	if	(grp->checkProfileParameter("forage"))
	{
		_BotStandProfileType = BOT_FORAGE;
		_BotStandProfileFactory = &BotProfileForageFactory;
	}
	else if	(grp->checkProfileParameter("social"))
	{
		_Social = true;
	}

	_NpcDestinationReached.resize( grp->getPersistent().bots().size());
	resetDestinationReachedData();
	
}

void CGrpProfileWander::addBot(CBot* bot) 
{
	CSpawnBot	*const	spawnBot=bot->getSpawnObj();
	if (!spawnBot)
		return;

	if (_BotStandProfileType == BOT_FORAGE)
	{
		// special case, only set the forage activity and let the bot live there life
		spawnBot->setAIProfile(_BotStandProfileFactory->createAIProfile(spawnBot));
		return;
	}

	CBotProfileStandAtPos* const profile = new CBotProfileStandAtPos(spawnBot);
#ifdef NL_DEBUG
	nlassert(profile!=NULL);
#endif
	spawnBot->setAIProfile(profile);
}
void CGrpProfileWander::removeBot(CBot* bot) 
{
}
CPathCont* CGrpProfileWander::getPathCont(CBot const* bot)
{
	return NULL;
}

void CGrpProfileWander::endProfile()
{
	PROFILE_LOG("group", "wander", "end", "");
}

void CGrpProfileWander::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpWanderProfileUpdate);
	CFollowPathContext fpcGrpWanderProfileUpdate("GrpWanderProfileUpdate");

	if (_BotStandProfileType == BOT_FORAGE)
	{
		// special case, only set the forage activity and let the bot live there life
		return;
	}

	// check all bot to see if there need to move 
	CSpawnGroupNpc* grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup*>(_Grp));
	CGroupNpc &pgrp = grp->getPersistent();
	bool aNpcHasReachDestination = false;
	for (uint i=0; i<pgrp.bots().size(); ++i)
	{
		CBotNpc* bot = static_cast<CBotNpc*>(pgrp.bots()[i]);
		if (!bot)
			continue;

		// check current bot state
		CSpawnBotNpc *sbot = bot->getSpawn();
		if (!sbot)
			continue;

		IAIProfile *profile = sbot->getAIProfile();

		if (!profile)
		{
			// init a profile on the bot
			sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
			continue;
		}			

		if (profile->getAIProfileType() == BOT_MOVE_TO)
		{
			// check if we are arrived
			CBotProfileMoveTo* mt = static_cast<CBotProfileMoveTo*>(profile);
			if (mt->destinationReach())
			{
				if (!_DestinationReachedAll)
				{
					
					uint32 npcSize =  (uint32)pgrp.bots().size();
					uint32 reachedSize = (uint32)_NpcDestinationReached.size();
					if (reachedSize!= npcSize)
					{
						_NpcDestinationReached.resize(npcSize);
						// invalid the vector a new bot has arrived						
						if (npcSize>reachedSize){ std::fill(_NpcDestinationReached.begin(), _NpcDestinationReached.end(), false); }
					}
					
					if ( !_NpcDestinationReached[i])
					{
						_NpcDestinationReached[i] = true;
						aNpcHasReachDestination = true;
					}
					
				}
				
				// look arround for interesting target
				CAIVision<CPersistentOfPhysical> vision;
				vision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), CAIVector(sbot->pos()), 10, 10);

				CPersistentOfPhysical *target = NULL;

				if (!vision.players().empty())
				{
					// there are some player near, look at one if it is not behin us
					uint index = CAIS::rand16((uint32)vision.players().size());
					CAngle angle(CAngle::pi());

					while (index < vision.players().size() && !target)
					{
						CPersistentOfPhysical *pop = vision.players()[index++];

						angle = sbot->pos().angleTo(pop->getSpawnObj()->pos());

						if (angle < CAngle::pi()/2 && angle > CAngle::pi()/-2)
						{
							target = pop;
							break;
						}
					}
				}
				if (!target && !vision.bots().empty())
				{
					// there are some bots near, look at one if it is not behin us
					uint index = CAIS::rand16((uint32)vision.bots().size());
					CAngle angle(CAngle::pi());

					while (index < vision.bots().size() && !target)
					{
						CPersistentOfPhysical *pop = vision.bots()[index++];

						angle = sbot->pos().angleTo(pop->getSpawnObj()->pos());

						if (angle < CAngle::pi()/2 && angle > CAngle::pi()/-2)
						{
							target = pop;
							break;
						}
					}
				}
				// set the visual target
				if (target && target->isSpawned())
					sbot->setVisualTarget(target->getSpawnObj());
				else
					sbot->setVisualTarget(NULL);

				// now, set the idle activity with a random timer
				
				sbot->setAIProfile(_BotStandProfileFactory->createAIProfile(sbot));
				CBotProfileWanderBase *wbs = safe_cast<CBotProfileWanderBase*>(sbot->getAIProfile());
				if	(wbs)
				{
					float	waitMin;
					float	waitMax;
					static	string	waitMinStr("wait min");
					static	string	waitMaxStr("wait max");
					if	(grp->getProfileParameter(waitMinStr, waitMin))
					{
						if	(!grp->getProfileParameter(waitMaxStr, waitMax))
							waitMax = waitMin;
						else
							waitMax = waitMax > waitMin ? waitMax : waitMin;
					}	
					else
					{
						waitMin = float(DefaultWanderMinTimer);
						waitMax = float(DefaultWanderMaxTimer);
					}
					wbs->setTimer(uint32(waitMin+CAIS::rand32(uint32(waitMax-waitMin))));
				}
			}
			continue;
		}
		
		if	(profile->getAIProfileType()==_BotStandProfileType)
		{
			const CBotProfileWanderBase*const wbs = static_cast<CBotProfileStandAtPos *>(sbot->getAIProfile());
#ifdef NL_DEBUG
		nlassert(wbs);
#endif
			if (!wbs->testTimer())
				continue;
		}
		
#ifdef	NL_DEBUG
		nlassert(_RandomPos != NULL && _RandomPos->getRandomPosCount() != 0);
#endif
		// R2_PRIMITIVE_LAXITY
		// for a Ring shard stop here and only do a warning to avoid asserting later
		// primitives generated by players may be incorrect and should not crash the service
		if (IsRingShard.get())
		{
			if	(	_RandomPos == NULL
				||	_RandomPos->getRandomPosCount() == 0)
			{
				string stateName = "NULL state!";
				CAIState const* const state = pgrp.getActiveState();
				if (state != NULL)
					stateName = state->getAliasFullName();
				nlwarning("No valid wander position for state '%s'", stateName.c_str());
				return;
			}
		}

		// time out, move to another point in the geometry
		RYAI_MAP_CRUNCH::CWorldPosition wp;
		
		if (_Social && CAIS::rand32(3) == 0)
		{
			// this time, we try to reach an npc in the neighbour
			//TODO : implemente this behavior
		}
		else
		{
			// standard random move
			_RandomPos->getRandomPos(wp);
		}
		CBotProfileMoveTo* mts = new CBotProfileMoveTo(_RandomPos->getVerticalPos(), wp, sbot);
		sbot->setAIProfile(mts);

	}

	if (aNpcHasReachDestination && !_DestinationReachedAll)
	{

		if (!_DestinationReachedFirst)
		{
			_DestinationReachedFirst = true;
			pgrp.processStateEvent(pgrp.mgr().EventDestinationReachedFirst);
		}

		uint32 first=0, last=(uint32)_NpcDestinationReached.size();
		for ( ; first != last && _NpcDestinationReached[first]; ++first) {}
		
		if (first == last)
		{
			_DestinationReachedAll = true;
			pgrp.processStateEvent(pgrp.mgr().EventDestinationReachedAll);
		}
	}
}

std::string CGrpProfileWander::getOneLineInfoString() const
{
	return "wander group profile";
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileWanderNoPrim                                                  //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileWanderNoPrim::CGrpProfileWanderNoPrim(CProfileOwner* owner, NLMISC::CSmartPtr<CNpcZonePlaceNoPrim> const& npcZone)
: CMoveProfile(owner)
, _Social(false)
, _NpcZone(npcZone)
{
	PROFILE_LOG("group", "wander", "ctor", "");
	_BotStandProfileType = BOT_STAND_AT_POS;
	_BotStandProfileFactory = &BotProfileStandAtPosFactory;
//	_RandomPos = &npcZone->getPlaceRandomPos();
}
/*
CGrpProfileWanderNoPrim::CGrpProfileWanderNoPrim(CProfileOwner* owner)
: CMoveProfile(owner)
, _Social(false)
{
	PROFILE_LOG("group", "wander", "ctor2", "");
	affectZoneFromStateMachine();
#if !FINAL_VERSION
	nlassert(!_RandomPos.isNULL());
#endif

	// default to stand apt pos profile
	_BotStandProfileType = BOT_STAND_AT_POS;
	_BotStandProfileFactory = &BotProfileStandAtPosFactory;
}
*/
void CGrpProfileWanderNoPrim::stateChangeProfile()
{
//	affectZoneFromStateMachine();
//	resetDestinationReachedData();
}
/*
void CGrpProfileWanderNoPrim::affectZoneFromStateMachine()
{	
	CSpawnGroupNpc* grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup *>(_Grp));
	CGroupNpc& pgrp = grp->getPersistent();
	CAIState const* const state = pgrp.getActiveState();
	
	if (	!state
		||	!state->isPositional())
		return;
	
	CAIStatePositional const* const sp = static_cast<CAIStatePositional const* const>(state);
	if	(!sp->shape().hasPoints())
	{
		if (sp->getAliasNode())
			nlwarning("Error, no position in state %s", sp->getAliasNode()->fullName().c_str());
		else
			nlwarning("Error, no position in state %s", sp->getName().c_str());
	}
	if (sp->shape().hasPatat())
		_RandomPos=&(sp->shape());
}
*/

CGrpProfileWanderNoPrim::~CGrpProfileWanderNoPrim()
{
	PROFILE_LOG("group", "wander", "dtor", "");
}

void CGrpProfileWanderNoPrim::setBotStandProfile(TProfiles	botStandProfileType, IAIProfileFactory *botStandProfileFactory)
{
	_BotStandProfileType = botStandProfileType;
	_BotStandProfileFactory = botStandProfileFactory;
}

void CGrpProfileWanderNoPrim::beginProfile()
{
	PROFILE_LOG("group", "wander", "begin", "");
	CMoveProfile::beginProfile();

	CSpawnGroupNpc *grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup *>(_Grp));

	if	(grp->checkProfileParameter("forage"))
	{
		_BotStandProfileType = BOT_FORAGE;
		_BotStandProfileFactory = &BotProfileForageFactory;
	}
	else if	(grp->checkProfileParameter("social"))
	{
		_Social = true;
	}
}

void CGrpProfileWanderNoPrim::addBot(CBot* bot) 
{
	CSpawnBot	*const	spawnBot=bot->getSpawnObj();
	if (!spawnBot)
		return;

	if (_BotStandProfileType == BOT_FORAGE)
	{
		// special case, only set the forage activity and let the bot live there life
		spawnBot->setAIProfile(_BotStandProfileFactory->createAIProfile(spawnBot));
		return;
	}

	CBotProfileStandAtPos* const profile = new CBotProfileStandAtPos(spawnBot);
#ifdef NL_DEBUG
	nlassert(profile!=NULL);
#endif
	spawnBot->setAIProfile(profile);
}
void CGrpProfileWanderNoPrim::removeBot(CBot* bot) 
{
}
CPathCont* CGrpProfileWanderNoPrim::getPathCont(CBot const* bot)
{
	return NULL;
}

void CGrpProfileWanderNoPrim::endProfile()
{
	PROFILE_LOG("group", "wander", "end", "");
}

void CGrpProfileWanderNoPrim::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpWanderProfileUpdate);
	CFollowPathContext fpcGrpWanderProfileUpdate("GrpWanderProfileUpdate");

	if (_BotStandProfileType == BOT_FORAGE)
	{
		// special case, only set the forage activity and let the bot live there life
		return;
	}

	// check all bot to see if there need to move 
	CSpawnGroupNpc* grp = static_cast<CSpawnGroupNpc*>(static_cast<CSpawnGroup*>(_Grp));
	CGroupNpc &pgrp = grp->getPersistent();
	
	for (uint i=0; i<pgrp.bots().size(); ++i)
	{
		CBotNpc* bot = static_cast<CBotNpc*>(pgrp.bots()[i]);
		if (!bot)
			continue;

		// check current bot state
		CSpawnBotNpc *sbot = bot->getSpawn();
		if (!sbot)
			continue;

		IAIProfile *profile = sbot->getAIProfile();

		if (!profile)
		{
			// init a profile on the bot
			sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
			continue;
		}			

		if (profile->getAIProfileType() == BOT_MOVE_TO)
		{
			// check if we are arrived
			CBotProfileMoveTo* mt = static_cast<CBotProfileMoveTo*>(profile);
			if (mt->destinationReach())
			{
				// look arround for interesting target
				CAIVision<CPersistentOfPhysical> vision;
				vision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), CAIVector(sbot->pos()), 10, 10);

				CPersistentOfPhysical *target = NULL;

				if (!vision.players().empty())
				{
					// there are some player near, look at one if it is not behin us
					uint index = CAIS::rand16((uint32)vision.players().size());
					CAngle angle(CAngle::pi());

					while (index < vision.players().size() && !target)
					{
						CPersistentOfPhysical *pop = vision.players()[index++];

						angle = sbot->pos().angleTo(pop->getSpawnObj()->pos());

						if (angle < CAngle::pi()/2 && angle > CAngle::pi()/-2)
						{
							target = pop;
							break;
						}
					}
				}
				if (!target && !vision.bots().empty())
				{
					// there are some bots near, look at one if it is not behin us
					uint index = CAIS::rand16((uint32)vision.bots().size());
					CAngle angle(CAngle::pi());

					while (index < vision.bots().size() && !target)
					{
						CPersistentOfPhysical *pop = vision.bots()[index++];

						angle = sbot->pos().angleTo(pop->getSpawnObj()->pos());

						if (angle < CAngle::pi()/2 && angle > CAngle::pi()/-2)
						{
							target = pop;
							break;
						}
					}
				}
				// set the visual target
				if (target && target->isSpawned())
					sbot->setVisualTarget(target->getSpawnObj());
				else
					sbot->setVisualTarget(NULL);

				// now, set the idle activity with a random timer
				
				sbot->setAIProfile(_BotStandProfileFactory->createAIProfile(sbot));
				CBotProfileWanderBase *wbs = safe_cast<CBotProfileWanderBase*>(sbot->getAIProfile());
				if	(wbs)
				{
					float	waitMin;
					float	waitMax;
					static	string	waitMinStr("wait min");
					static	string	waitMaxStr("wait max");
					if	(grp->getProfileParameter(waitMinStr, waitMin))
					{
						if	(!grp->getProfileParameter(waitMaxStr, waitMax))
							waitMax = waitMin;
						else
							waitMax = waitMax > waitMin ? waitMax : waitMin;
					}	
					else
					{
						waitMin = float(DefaultWanderMinTimer);
						waitMax = float(DefaultWanderMaxTimer);
					}
					wbs->setTimer(uint32(waitMin+CAIS::rand32(uint32(waitMax-waitMin))));
				}
			}
			continue;
		}
		
		if	(profile->getAIProfileType()==_BotStandProfileType)
		{
			const CBotProfileWanderBase*const wbs = static_cast<CBotProfileStandAtPos *>(sbot->getAIProfile());
#ifdef NL_DEBUG
		nlassert(wbs);
#endif
			if (!wbs->testTimer())
				continue;
		}
		
#ifdef	NL_DEBUG
		nlassert(_NpcZone->getRandomPosCount());
#endif
		// time out, move to another point in the geometry
		RYAI_MAP_CRUNCH::CWorldPosition wp;
		
		if (_Social && CAIS::rand32(3) == 0)
		{
			// this time, we try to reach an npc in the neighbour
			//TODO : implemente this behavior
		}
		else
		{
			// standard random move
			_NpcZone->getRandomPos(wp);
		}
		CBotProfileMoveTo* mts = new CBotProfileMoveTo(_NpcZone->getVerticalPos(), wp, sbot);
		sbot->setAIProfile(mts);
	}	
}

std::string CGrpProfileWanderNoPrim::getOneLineInfoString() const
{
	return "wander group profile (without primitive)";
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileStandOnVertices                                          //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileStandAtStartPoint::CGrpProfileStandAtStartPoint(CProfileOwner* owner)
: CMoveProfile(owner)
{
	PROFILE_LOG("group", "stand_at_start_point", "ctor", "");
}

CGrpProfileStandAtStartPoint::~CGrpProfileStandAtStartPoint()
{
	PROFILE_LOG("group", "stand_at_start_point", "dtor", "");
	for (CCont<CBot >::iterator it=_Grp->bots().begin(), itEnd=_Grp->bots().end();it!=itEnd;++it)
	{
		CBot	*bot=*it;
		removeBot	(bot);
	}		
}

CGrpProfileStandAtStartPoint::CBotPositionner::CBotPositionner(RYAI_MAP_CRUNCH::TAStarFlag	flags)
: _PathCont(flags)
{
}

CGrpProfileStandAtStartPoint::CBotPositionner::CBotPositionner(TVerticalPos verticalPos, CAIPos position, RYAI_MAP_CRUNCH::TAStarFlag	flag) 
: _PathCont(flag)
, _Position(position)
, _VerticalPos(verticalPos)
, _BotAtDest(false)
{
	_PathCont.setDestination(verticalPos, position);
}
		
CGrpProfileStandAtStartPoint::CBotPositionner::~CBotPositionner()
{
}

inline
void CGrpProfileStandAtStartPoint::CBotPositionner::setBotAtDest(bool atDest)
{
	_BotAtDest = atDest;
}

inline
bool CGrpProfileStandAtStartPoint::CBotPositionner::isBotAtDest() const
{
	return _BotAtDest;
}

CPathCont* CGrpProfileStandAtStartPoint::getPathCont(CBot const* bot)
{
	TNpcBotPositionnerMap::const_iterator it = _NpcList.find(bot);
	if (it==_NpcList.end())
		return	NULL;
	return &it->second->_PathCont;
}

void CGrpProfileStandAtStartPoint::beginProfile()
{
	PROFILE_LOG("group", "stand_at_start_point", "begin", "");
	CMoveProfile::beginProfile();
	
	setCurrentValidPos();
	_Finished = false;
}

void CGrpProfileStandAtStartPoint::resumeProfile()
{
	PROFILE_LOG("group", "stand_at_start_point", "resume", "");
	for (TNpcBotPositionnerMap::iterator it=_NpcList.begin(), itEnd=_NpcList.end();it!=itEnd;++it)
	{
		const	CBot*const	bot=(*it).first;
		CSpawnBot	*sbot=bot->getSpawnObj();
		if	(!sbot)
			continue;

		switch	(sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			break;
		default:	//	push the correct comportment.
			sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
			break;
		}
		
	}

	setCurrentValidPos();
	_Finished = false;
}

void CGrpProfileStandAtStartPoint::addBot(CBot* bot)
{
	CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
	
	CGroupNpc& grp = botNpc->grp();
#ifdef NL_DEBUG
	nlassert(grp.getSpawnObj());
#endif		
	CAIStatePositional *grpState=NLMISC::safe_cast<CAIStatePositional *>(grp.getCAIState());		
	_NpcList[bot]=new	CBotPositionner	(botNpc->getStartVerticalPos(), botNpc->getStartPos(), botNpc->getGroup().getAStarFlag());

	CSpawnBot	*const	sbot=bot->getSpawnObj();
	if	(sbot)
		sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
}

void CGrpProfileStandAtStartPoint::removeBot(CBot* bot)
{
	TNpcBotPositionnerMap::iterator it=_NpcList.find(bot);

	if (it==_NpcList.end())
		return;
	
	CSpawnBotNpc* const spawnBot = NLMISC::safe_cast<CBotNpc*>(bot)->getSpawn();
	if (spawnBot)
		spawnBot->setAIProfile(new CBotProfileStandAtPos(spawnBot));
	_NpcList.erase	(it);
}

void CGrpProfileStandAtStartPoint::setCurrentValidPos()
{
	FOREACH(it, TNpcBotPositionnerMap, _NpcList)
	{
		CBotPositionner	*botPos=(*it).second;
		botPos->setBotAtDest(false);
		botPos->_PathCont.setDestination(botPos->_VerticalPos, botPos->_Position);
	}

}

void CGrpProfileStandAtStartPoint::endProfile()
{
	PROFILE_LOG("group", "stand_at_start_point", "end", "");
}

void CGrpProfileStandAtStartPoint::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpStandStartPointProfileUpdate);
	CFollowPathContext fpcGrpStandStartPointProfileUpdate("GrpStandStartPointProfileUpdate");

	CMoveProfile::updateProfile(ticksSinceLastUpdate);

	CGroupNpc	&persGrp=NLMISC::safe_cast<CSpawnGroupNpc*>(_Grp.ptr())->getPersistent();
	CAIStatePositional	*grpState=static_cast<CAIStatePositional*>(persGrp.getCAIState());
	
	uint32	nbAtDest=0;
	uint32	nbNewAtDest=0;

	for (TNpcBotPositionnerMap::iterator it=_NpcList.begin(), itEnd=_NpcList.end();it!=itEnd;++it)
	{
		CBotPositionner*const	botPos=(*it).second;
		if	(botPos->isBotAtDest())
		{
			nbAtDest++;
			continue;
		}

		const	CBot*const	bot=(*it).first;
		CSpawnBot	*sbot=bot->getSpawnObj();
		if	(	!sbot
			||	!sbot->isAlive())
			continue;

		switch	(sbot->getAIProfileType())
		{
		case BOT_FOLLOW_POS:
			{
				CBotProfileFollowPos* prof = NLMISC::safe_cast<CBotProfileFollowPos*>(sbot->getAIProfile());
				
				if (prof->_Status==CFollowPath::FOLLOW_ARRIVED)
				{
					sbot->setTheta(botPos->_Position.theta());
					sbot->setAIProfile(new CBotProfileStandAtPos(sbot));
					botPos->setBotAtDest();
					nbNewAtDest++;
					if (simulateBug(2))
					{
						/* Following statement was missing */
					}
					else
					{
						nbAtDest++;
					}
				}
				
			}
			break;
		default:	//	push the correct comportment.
			{
				sbot->setAIProfile(new CBotProfileFollowPos(&botPos->_PathCont, sbot));
			}
			break;
		}
		
	}

	// first to arrived ?
	if	(	nbNewAtDest==nbAtDest
		&&	nbAtDest>0	)
	{
		persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedFirst);
	}

	// all arrived ?
	if	(	nbAtDest==_NpcList.size()
		&&	!_Finished	)
	{
		persGrp.processStateEvent(persGrp.mgr().EventDestinationReachedAll);
		_Finished=true;
	}
}

std::string CGrpProfileStandAtStartPoint::getOneLineInfoString() const
{
	std::string info = "stand_at_start_point group profile";
	info += " finished=" + NLMISC::toString(_Finished);
	return info;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileEscorted                                                 //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileEscorted::CGrpProfileEscorted(CProfileOwner *owner)
: CGrpProfileNormal(owner)
{
	PROFILE_LOG("group", "escorted", "ctor", "");
}

CGrpProfileEscorted::~CGrpProfileEscorted()
{
	PROFILE_LOG("group", "escorted", "dtor", "");
}

void CGrpProfileEscorted::beginProfile()
{
	PROFILE_LOG("group", "escorted", "begin", "");
	CGrpProfileNormal::beginProfile();
	_EscortTeamInRange = false;
}

void CGrpProfileEscorted::endProfile()
{
	PROFILE_LOG("group", "escorted", "end", "");
	if (_Grp->movingProfile().getAIProfileType() == AITYPES::MOVE_FOLLOW_ROUTE)
	{
		CGrpProfileFollowRoute* prof = NLMISC::safe_cast<CGrpProfileFollowRoute*>(_Grp->movingProfile().getAIProfile());
		if (prof)
			prof->stopNpc(false);
	}
}

void CGrpProfileEscorted::stateChangeProfile() 
{
	CGrpProfileNormal::beginProfile();
}
	
void CGrpProfileEscorted::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpEscortedProfileUpdate);
	CFollowPathContext fpcGrpEscortedProfileUpdate("GrpEscortedProfileUpdate");

	const	uint32	ESCORT_RANGE_DELTA = 3;
	CGroupNpc	&GrpRef	=	_Grp->getPersistent();
	const	uint16	teamId	=	GrpRef.getEscortTeamId();

	if (teamId == CTEAM::InvalidTeamId)
	{
		// no escort team assigned, can't move! 
		_EscortTeamInRange = false;
		return;
	}

	// we need to have some member of the escort team in range to allow movement
	// look arround for an escort member
	CAIVector	centerPos;
	bool		escortAway = true;
	bool		escortBack = false;
	float		escortRange=GrpRef.getEscortRange();
	double		distAway = escortRange+ESCORT_RANGE_DELTA;
	double		distBack = escortRange-ESCORT_RANGE_DELTA;
	// square the dist to speedup test
	distAway *= distAway;
	distBack *= distBack;
	if	(_Grp->calcCenterPos(centerPos))	// true if there's some bots in the group.
	{
		CAIVision<CPersistentOfPhysical>	vision;

		// group vision update.
		vision.updateBotsAndPlayers(_Grp->getPersistent().getAIInstance(), centerPos, uint(escortRange+ESCORT_RANGE_DELTA), 0);

		if (vision.players().empty())
		{
			// no player, no need to check, the escort is away !
			escortAway = true;
		}
		else
		{
			// loop on each player until we know if escort is away or back
			vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >::const_iterator first(vision.players().begin()), last(vision.players().end());
			for (;first != last && (escortAway || !escortBack); ++first)
			{
				CBotPlayer *player = safe_cast<CBotPlayer*>((*first).ptr());	//	(CPersistentOfPhysical*)
				if (player->isAlive())
				{
					if (CMirrors::getTeamId(player->getSpawnObj()->dataSetRow()) == teamId)
					{
						// on of our escort girl ;)
						// check the distance
						double sqrDist = (centerPos - player->getSpawnObj()->pos()).sqrnorm();
						if (sqrDist < distBack)
						{
							escortBack = true;
							// the escort is back
						}
						if (sqrDist < distAway)
						{
							escortAway = false;
							// the escort is not away
						}
					}
				}
			}
		}
	}
	
	if	(_EscortTeamInRange)
	{
		if (escortAway)
		{
			// switch to 'wait for escort' mode and send event 'escort away'
			_EscortTeamInRange = false;
			GrpRef.processStateEvent(GrpRef.mgr().getStateMachine()->EventEscortAway);
		}
	}
	else
	{
		if (escortBack)
		{
			// switch to 'escort in range' mode and send event 'escort back'
			_EscortTeamInRange = true;
			GrpRef.processStateEvent(GrpRef.mgr().getStateMachine()->EventEscortBack);
		}
	}
	
	if	(_GroupFighting)
	{
		if (!_Grp->fightProfile().getAIProfile())
			_Grp->fightProfile().setAIProfile(_Grp.ptr(), &GrpProfileFightFactory, false);
		
		_Grp->fightProfile().mayUpdateProfile(ticksSinceLastUpdate);

		CFightProfile* profile = NLMISC::safe_cast<CFightProfile*>(_Grp->fightProfile().getAIProfile());
		if	(!profile->stillHaveEnnemy())
		{
			// :TODO: Verify if it's needed to erase bots aggro too/instead
//			_Grp->clearAggroList();	// this erase all aggro.
			setGroupFighting	(false);

			_Grp->fightProfile().setAIProfile(NULL);
			(NLMISC::safe_cast<CMoveProfile*>(_Grp->movingProfile().getAIProfile()))->resumeProfile	();
		}
	}
	else
	{
		if (!_Grp->checkProfileParameter("defenceless"))
		{
			if (_Grp->haveAggroOrReturnPlace())
			{
				if(_Grp->isGroupAlive())
				{
					//	set the fighting comportment.
					if (!_Grp->fightProfile().getAIProfile())
						_Grp->fightProfile().setAIProfile(_Grp.ptr(), &GrpProfileFightFactory, false);
					
					setGroupFighting(true);
				}
			}
		}
	}
	
	if (_Grp->movingProfile().getAIProfileType() == AITYPES::MOVE_FOLLOW_ROUTE && !_GroupFighting)
	{
		CGrpProfileFollowRoute* prof = NLMISC::safe_cast<CGrpProfileFollowRoute*>(_Grp->movingProfile().getAIProfile());
		
		if (prof)
		{
			prof->stopNpc(!_EscortTeamInRange);
			_Grp->movingProfile().mayUpdateProfile(ticksSinceLastUpdate);
		}
	}
}

std::string CGrpProfileEscorted::getOneLineInfoString() const
{
	std::string info = "escorted profile";
	info += " escort_team_in_range=" + NLMISC::toString(_EscortTeamInRange);
	uint16 teamId = _Grp->getPersistent().getEscortTeamId();
	info += " team_id=" + (teamId==CTEAM::InvalidTeamId)?"InvalidTeamId":NLMISC::toString(teamId);
	return info;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileGuardEscorted                                            //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileGuardEscorted::CGrpProfileGuardEscorted(CProfileOwner *owner)
: CGrpProfileNormal(owner)
{
	PROFILE_LOG("group", "guard_escorted", "ctor", "");
	_GuardProfile = new CGrpProfileGuard(owner);
	_EscortedProfile = new CGrpProfileEscorted(owner);
}

CGrpProfileGuardEscorted::~CGrpProfileGuardEscorted()
{
	PROFILE_LOG("group", "guard_escorted", "dtor", "");
}

void CGrpProfileGuardEscorted::beginProfile()
{
	PROFILE_LOG("group", "guard_escorted", "begin", "");
	CGrpProfileNormal::beginProfile();

	_GuardProfile->beginProfile();
	_EscortedProfile->beginProfile();
}

void CGrpProfileGuardEscorted::endProfile()
{
	PROFILE_LOG("group", "guard_escorted", "end", "");
	_EscortedProfile->endProfile();
	_GuardProfile->endProfile();
}

void CGrpProfileGuardEscorted::stateChangeProfile() 
{
	CGrpProfileNormal::beginProfile();
	_EscortedProfile->stateChangeProfile();
	_GuardProfile->stateChangeProfile();
}
	
void CGrpProfileGuardEscorted::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrpEscortedGuardProfileUpdate);
	CFollowPathContext fpcGrpEscortedGuardProfileUpdate("GrpEscortedGuardProfileUpdate");

	_GuardProfile->updateProfile(ticksSinceLastUpdate);
	if (!_EscortedProfile->isGroupFighting())
		_EscortedProfile->updateProfile(ticksSinceLastUpdate);
}

std::string CGrpProfileGuardEscorted::getOneLineInfoString() const
{
	std::string info = "guard_escorted profile";
	info += " (";
	info += _GuardProfile?_GuardProfile->getOneLineInfoString():std::string("<no guard profile>");
	info += ") (";
	info += _EscortedProfile?_EscortedProfile->getOneLineInfoString():std::string("<no escorted profile>");
	info += ")";
	return info;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileSquad                                                         //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileSquad::CGrpProfileSquad(CProfileOwner* const owner)
: CGrpProfileFaction(owner)
{
	PROFILE_LOG("group", "squad", "ctor", "");
}

CGrpProfileSquad::~CGrpProfileSquad()
{
	PROFILE_LOG("group", "squad", "dtor", "");
}

void CGrpProfileSquad::beginProfile()
{
	PROFILE_LOG("group", "squad", "begin", "");
	CGrpProfileFaction::beginProfile();
	CGroupNpc& thisGrpNpc = _Grp->getPersistent();	

	// Set aggro parameters
	thisGrpNpc._AggroRange = 25;
	thisGrpNpc._UpdateNbTicks = 10;
}

string CGrpProfileSquad::getOneLineInfoString()	const
{
	return "squad profile";
}

void CGrpProfileSquad::aggroEntity(CAIEntityPhysical const* entity)
{
//	COutpost* outpost = getDefendedOutpost();
//	if (!outpost || outpost->getShape()->atPlace(CAIVector(entity->pos())))
		_Grp->setAggroMinimumFor(entity->dataSetRow(), 0.5f, false);
}

NLMISC::CSmartPtr<CAIPlace const> CGrpProfileSquad::buildFirstHitPlace(TDataSetRow const& aggroBot)
{
	COutpost* outpost = getDefendedOutpost();
	if (outpost)
		return &*(outpost->getShape());
	else
		return NULL;
}
/*
void CGrpProfileSquad::aggroEntity(CAIEntityPhysical const* entity)
{
	COutpost* outpost = getDefendedOutpost();
	if (outpost)
	{
		// Check that the (player) entity is in the outpost zone (according to the rules of the EGS with timers)
		CMirrorPropValueRO<TYPE_IN_OUTPOST_ZONE_ALIAS> entityInOutpostAlias(TheDataset, entity->dataSetRow(), DSPropertyIN_OUTPOST_ZONE_ALIAS);
		if (entityInOutpostAlias != outpost->getAlias())
			return;
	}
	_Grp->setBotAggroMinimum(entity->dataSetRow(), -0.5f);
}
*/
//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFaction                                                       //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileFaction::CGrpProfileFaction(CProfileOwner* const owner)
: CGrpProfileNormal(owner)
{
	PROFILE_LOG("group", "faction", "ctor", "");
	bNoAssist = false;
}

CGrpProfileFaction::~CGrpProfileFaction()
{
	PROFILE_LOG("group", "faction", "dtor", "");
}

void CGrpProfileFaction::beginProfile()
{
	PROFILE_LOG("group", "faction", "begin", "");
}

void CGrpProfileFaction::endProfile()
{
	PROFILE_LOG("group", "faction", "end", "");
}

AITYPES::CPropertySet CGrpProfileFaction::_FameFactions;

void CGrpProfileFaction::initFameFactions()
{
	static bool inited = false;
	if (!inited)
	{
		inited = true;
		std::map<std::string, NLMISC::TStringId> factionTranslator;
		std::vector<NLMISC::TStringId> const& factionNames = CStaticFames::getInstance().getFactionNames();
		std::vector<NLMISC::TStringId>::const_iterator it, end=factionNames.end();
		for (it=factionNames.begin(); it!=end; ++it)
		{
			std::string fameFaction = CStringMapper::unmap(*it);
			std::string scriptFaction = fameFactionToScriptFaction(fameFaction);
			factionTranslator.insert(std::make_pair(scriptFaction, *it));
			_FameFactions.addProperty(scriptFaction);
		}
		_FameFactions.addProperty(CPropertyId("Player"));
#ifdef NL_DEBUG_FACTION_NAME_CONVERTER
		nldebug("Testing script faction name converters");
		std::string names[][2] = {
			{ "matis", "FamousMatis" },
			{ "fyros", "FamousFyros" },
			{ "tryker", "FamousTryker" },
			{ "zorai", "FamousZorai" },
			{ "kami", "FamousKami" },
			{ "black_kami", "FamousBlackKami" },
			{ "karavan", "FamousKaravan" },
			{ "white_karavan", "FamousWhiteKaravan" },
			{ "tribe_ancient_dryads", "FamousTribeAncientDryads" },
			{ "tribe_antikamis", "FamousTribeAntikamis" },
			{ "tribe_barkers", "FamousTribeBarkers" },
			{ "tribe_beachcombers", "FamousTribeBeachcombers" },
			{ "tribe_black_circle", "FamousTribeBlackCircle" },
			{ "tribe_cholorogoos", "FamousTribeCholorogoos" },
			{ "tribe_cockroaches", "FamousTribeCockroaches" },
			{ "tribe_company_of_the_eternal_tree", "FamousTribeCompanyOfTheEternalTree" },
			{ "tribe_corsair", "FamousTribeCorsair" },
			{ "tribe_cute", "FamousTribeCute" },
			{ "tribe_darkening_sap", "FamousTribeDarkeningSap" },
			{ "tribe_dune_riders", "FamousTribeDuneRiders" },
			{ "tribe_ecowarriors", "FamousTribeEcowarriors" },
			{ "tribe_firebrands", "FamousTribeFirebrands" },
			{ "tribe_first_deserter", "FamousTribeFirstDeserter" },
			{ "tribe_frahar", "FamousTribeFrahar" },
			{ "tribe_frahar_hunters", "FamousTribeFraharHunters" },
			{ "tribe_gibbay", "FamousTribeGibbay" },
			{ "tribe_goo_heads", "FamousTribeGooHeads" },
			{ "tribe_green_seed", "FamousTribeGreenSeed" },
			{ "tribe_hamazans_of_the_dead_seed", "FamousTribeHamazansOfTheDeadSeed" },
			{ "tribe_icon_workshipers", "FamousTribeIconWorkshipers" },
			{ "tribe_keepers", "FamousTribeKeepers" },
			{ "tribe_kitin_gatheres", "FamousTribeKitinGatheres" },
			{ "tribe_lagoon_brothers", "FamousTribeLagoonBrothers" },
			{ "tribe_lawless", "FamousTribeLawless" },
			{ "tribe_leviers", "FamousTribeLeviers" },
			{ "tribe_master_of_the_goo", "FamousTribeMasterOfTheGoo" },
			{ "tribe_matisian_border_guards", "FamousTribeMatisianBorderGuards" },
			{ "tribe_night_turners", "FamousTribeNightTurners" },
			{ "tribe_oasis_diggers", "FamousTribeOasisDiggers" },
			{ "tribe_pyromancers", "FamousTribePyromancers" },
			{ "tribe_recoverers", "FamousTribeRecoverers" },
			{ "tribe_renegades", "FamousTribeRenegades" },
			{ "tribe_restorers", "FamousTribeRestorers" },
			{ "tribe_root_tappers", "FamousTribeRootTappers" },
			{ "tribe_sacred_sap", "FamousTribeSacredSap" },
			{ "tribe_sap_gleaners", "FamousTribeSapGleaners" },
			{ "tribe_sap_slaves", "FamousTribeSapSlaves" },
			{ "tribe_scorchers", "FamousTribeScorchers" },
			{ "tribe_shadow_runners", "FamousTribeShadowRunners" },
			{ "tribe_siblings_of_the_weeds", "FamousTribeSiblingsOfTheWeeds" },
			{ "tribe_silt_sculptors", "FamousTribeSiltSculptors" },
			{ "tribe_slavers", "FamousTribeSlavers" },
			{ "tribe_smuglers", "FamousTribeSmuglers" },
			{ "tribe_the_arid_matis", "FamousTribeTheAridMatis" },
			{ "tribe_the_kuilde", "FamousTribeTheKuilde" },
			{ "tribe_the_slash_and_burn", "FamousTribeTheSlashAndBurn" },
			{ "tribe_tutors", "FamousTribeTutors" },
			{ "tribe_water_breakers", "FamousTribeWaterBreakers" },
			{ "tribe_woven_bridles", "FamousTribeWovenBridles" }
		};
		size_t size = sizeof(names)/sizeof(names[0]);
		for (size_t i=0; i<size; ++i)
		{
			nldebug("%s -> %s", names[i][0].c_str(), fameFactionToScriptFaction(names[i][0]).c_str());
			nlassert( names[i][1] == fameFactionToScriptFaction(names[i][0]) );
			nldebug("%s -> %s", names[i][1].c_str(), scriptFactionToFameFaction(names[i][1]).c_str());
			nlassert( names[i][0] == scriptFactionToFameFaction(names[i][1]) );
		}
#endif
	}
}

bool CGrpProfileFaction::entityHavePartOfFactions(CAIEntityPhysical const* entity, AITYPES::CPropertySetWithExtraList<TAllianceId> const& factions)
{
	H_AUTO(CGrpProfileFaction_entityHavePartOfFactions);
	switch (entity->getRyzomType())
	{
		case RYZOMID::player:
		{
			// Test Player
			if (factions.have(AITYPES::CPropertyId("Player")))
			{
				//	nldebug("Entity has faction Player");
				return true;
			}
			// Test if the entity is involved in an outpost war
			if (entity->outpostAlias()!=0)
			{
				if (factions.have(NLMISC::toString("outpost:%s:%s", LigoConfig.aliasToString(entity->outpostAlias()).c_str(), entity->outpostSide()?"attacker":"defender")))
					return true;
			}
			// Test entity fame value
			std::set<TStringId> factionsSet = factions.properties();
			std::set<TStringId>::const_iterator it, end = factionsSet.end();
			for (it=factionsSet.begin(); it!=end; ++it)
			{
				string factionInfos = CStringMapper::unmap(*it);
				string fameFaction = scriptFactionToFameFaction(factionInfos);
			//	sint32 fame = CFameInterface::getInstance().getFameOrCivilisationFame(entity->getEntityId(), CStringMapper::map(fameFaction));
				sint32 const fame = entity->getFame(fameFaction);
				sint32 const value = scriptFactionToFameFactionValue(factionInfos);
				bool gt = scriptFactionToFameFactionGreaterThan(factionInfos);
				if ((fame != NO_FAME && gt && fame > value) ||
					(fame != NO_FAME && !gt && fame < value))
				{
					//	nldebug("Entity has faction %s", CStringMapper::unmap(*it).c_str());
					return true;
				}
			}
			return false;
		}
		case RYZOMID::npc:
		{
			CSpawnBotNpc const* const sbnEntity = NLMISC::safe_cast<CSpawnBotNpc const*>(entity);
			CGroupNpc *const gnEntityGroup = NLMISC::safe_cast<CGroupNpc*>(&sbnEntity->getPersistent().getGroup());
			return gnEntityGroup->faction().containsPartOfStrict(factions);
		}
		case RYZOMID::creature:
		{
			CSpawnBotFauna const* const sbnEntity = NLMISC::safe_cast<CSpawnBotFauna const*>(entity);
			CGrpFauna* const gnEntityGroup = NLMISC::safe_cast<CGrpFauna*>(&sbnEntity->getPersistent().getGroup());
			return gnEntityGroup->faction().containsPartOfStrict(factions);
		}
		default:
		{
			return false;
		}
	}
}

std::string CGrpProfileFaction::scriptFactionToFameFaction(std::string name)
{
	if (name.find("Famous")!=0)
		return name;
	std::string ret = "";
	for (size_t i=6; i<name.length(); ++i)
	{
		if (i==6 && name[i]>='A' && name[i]<='Z')
			ret += name[i]-'A'+'a';
		else if (name[i]>='A' && name[i]<='Z')
		{
			ret += "_";
			ret += name[i]-'A'+'a';
		}
		else if  (name[i] == '>' || name[i] == '<')
		{
			return ret;
		}
		else
		{
			ret += name[i];
		}
	}
	return ret;
}

bool CGrpProfileFaction::scriptFactionToFameFactionGreaterThan(string name)
{	
	if (name.find("<") != string::npos)
		return false;
		
	return true;
}

sint32 CGrpProfileFaction::scriptFactionToFameFactionValue(string name)
{
	size_t start = name.find(">");
	if (start == string::npos)
	{
		start = name.find("<");
		if (start == string::npos)
			return 0;
	}

	sint32 value;
	NLMISC::fromString(name.substr(start+1), value);
	return value*6000;
}

std::string CGrpProfileFaction::fameFactionToScriptFaction(std::string name)
{
	std::string ret = "Famous";
	for (size_t i=0; i<name.length(); ++i)
	{
		if (i==0 && name[i]>='a' && name[i]<='z')
			ret += name[i]-'a'+'A';
		else if (name[i]=='_' && name[i+1]>='a' && name[i+1]<='z')
			ret += name[++i]-'a'+'A';
		else
			ret += name[i];
	}
	return ret;
}

void CGrpProfileFaction::aggroEntity(CAIEntityPhysical const* entity)
{
	_Grp->setAggroMinimumFor(entity->dataSetRow(), 0.5f, false);
}

void CGrpProfileFaction::checkTargetsAround()
{
	if (!_checkTargetTimer.test())
		return;
	
	CGroupNpc& thisGrpNpc = _Grp->getPersistent();	
	
	_checkTargetTimer.set(thisGrpNpc._UpdateNbTicks+CAIS::rand16(2)); // every _UpdateNbTicks+1 seconds.	
	
	initFameFactions();
	CPropertySetWithExtraList<TAllianceId> const& thisFaction = thisGrpNpc.faction();
	CPropertySetWithExtraList<TAllianceId> const& thisFriendFactions = thisGrpNpc.friendFaction();
	CPropertySetWithExtraList<TAllianceId> const& thisEnnemyFactions = thisGrpNpc.ennemyFaction();

	// We don't assist or attack players if our friends/ennemies are not in factions
	bool const assistPlayers = (thisFriendFactions.containsPartOfStrictFilter("Famous*") || thisFriendFactions.have(AITYPES::CPropertyId("Player")));
	bool const assistBots    = !thisFriendFactions.empty() && !bNoAssist;
	bool const attackPlayers = (!thisEnnemyFactions.extraSetEmpty()) || thisEnnemyFactions.containsPartOfStrictFilter("Famous*") || thisEnnemyFactions.have(AITYPES::CPropertyId("Player")) || thisEnnemyFactions.containsPartOfStrictFilter("outpost:*");
	bool const attackBots    = !thisEnnemyFactions.empty();
	
	CAIVision<CPersistentOfPhysical>	Vision;
	breakable
	{
		CAIVector	centerPos;
		if	(!_Grp->calcCenterPos(centerPos))	// true if there's some bots in the group.
			break;
		
		const	uint32	playerRadius=(uint32)(assistPlayers||attackPlayers?thisGrpNpc._AggroRange:0);
		const	uint32	botRadius=(uint32)(assistBots||attackBots?thisGrpNpc._AggroRange:0);
		
		Vision.updateBotsAndPlayers(thisGrpNpc.getAIInstance(), centerPos, playerRadius, botRadius);
	}
	
	// Assist players
	if (assistPlayers)
	{
		// For each player
		FOREACHC (itAssisted, std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >, Vision.players())
		{
			// Get assisted entity
			CPersistentOfPhysical const* const popAssisted = (*itAssisted);
			CAIEntityPhysical* const epAssisted = popAssisted->getSpawnObj();
			// If entity is not alive skip it
			if (!epAssisted || !epAssisted->isAlive() || epAssisted->currentHitPoints()<=0.f)
				continue;
			// If entity is not a friend skip it
			if (!entityHavePartOfFactions(epAssisted, thisFriendFactions))
				continue;
			// For each targeter of the assisted entity
			CAIEntityPhysical const* epAttacker = epAssisted->firstTargeter();
			while (epAttacker)
			{
				// If attacker is not in our faction attack him
				if (!entityHavePartOfFactions(epAttacker, thisFaction))
					aggroEntity(epAttacker);
				epAttacker = epAttacker->nextTargeter();
			}
		}
	}
	// Assist bots
	if (assistBots)
	{
		FOREACHC (itAssisted, std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >, Vision.bots())
		{
			// Get assisted entity
			const CPersistentOfPhysical *const popAssisted = (*itAssisted);
			CAIEntityPhysical *const epAssisted = popAssisted->getSpawnObj();
			// If entity is not alive skip it
			if (!epAssisted || !epAssisted->isAlive() || epAssisted->currentHitPoints()<=0.f)
				continue;
			// If entity is not a npc skip it
			if (epAssisted->getRyzomType()!=RYZOMID::npc)
				continue;
			// If entity is not a friend skip it
			if (!entityHavePartOfFactions(epAssisted, thisFriendFactions))
				continue;
			// For each targeter of the assisted entity
			CAIEntityPhysical const* epAttacker = epAssisted->firstTargeter();
			while (epAttacker)
			{
				// If attacker is not in our faction attack him
				if (!entityHavePartOfFactions(epAttacker, thisFaction))
				{
					const CSpawnBot * spwBot = dynamic_cast<const CSpawnBot *>(epAssisted);
					if(spwBot)
					{
						if (spwBot->haveAggroWithEntity(epAttacker->dataSetRow()))
							aggroEntity(epAttacker);
					}
				}
				epAttacker = epAttacker->nextTargeter();
			}
		}
	}
	// Attack players
	if (attackPlayers)
	{
		FOREACHC (itAttacked, TPersistentList, Vision.players())
		{
			// Get attacked entity
			CPersistentOfPhysical const* const popAttacked = (*itAttacked);
			CAIEntityPhysical const* const epAttacked = popAttacked->getSpawnObj();
			// If entity is not alive skip it
			if (!epAttacked || !epAttacked->isAlive() || epAttacked->currentHitPoints()<=0.f)
				continue;
			// If entity is not an ennemy skip it
			if (!entityHavePartOfFactions(epAttacked, thisEnnemyFactions))
				continue;
			// If entity is a friend skip it
			if (entityHavePartOfFactions(epAttacked, thisFriendFactions))
				continue;
			// If entity is in our faction skip it
			if (entityHavePartOfFactions(epAttacked, thisFaction))
				continue;
			// If player is in safe zone skip it
			CRootCell const* const rootCell = epAttacked->wpos().getRootCell();
			if (rootCell && rootCell->getFlag()!=0)	//	Safe Zone ?
				continue;
			// Attack the rest
			aggroEntity(epAttacked);
		}
	}
	// Attack bots
	if (attackBots)
	{
		FOREACHC (itAttacked, TPersistentList, Vision.bots())
		{
			// Get attacked entity
			CPersistentOfPhysical const* const popAttacked = (*itAttacked);
			CAIEntityPhysical const* const epAttacked = popAttacked->getSpawnObj();
			// If entity is not alive skip it
			if (!epAttacked || !epAttacked->isAlive() || epAttacked->currentHitPoints()<=0.f)
				continue;
			// If entity is not an ennemy skip it
			if (!entityHavePartOfFactions(epAttacked, thisEnnemyFactions))
				continue;
			// If entity is a friend skip it
			if (entityHavePartOfFactions(epAttacked, thisFriendFactions))
				continue;
			// If entity is in our faction skip it
			if (entityHavePartOfFactions(epAttacked, thisFaction))
				continue;
			// Attack the rest
			aggroEntity(epAttacked);
		}
	}
}

void CGrpProfileFaction::updateProfile(uint ticksSinceLastUpdate)
{
	checkTargetsAround	();		
	CGrpProfileNormal::updateProfile(ticksSinceLastUpdate);
}

string CGrpProfileFaction::getOneLineInfoString() const
{
	return "faction profile";
}

//////////////////////////////////////////////////////////////////////////////
// Profile factories implementation                                         //
//////////////////////////////////////////////////////////////////////////////

//- Singleton profiles (stateless ones) --------------------------------------

CBotProfileStandAtPosFactory BotProfileStandAtPosFactory;
CBotProfileForageFactory BotProfileForageFactory;
CGrpProfileFightFactory GrpProfileFightFactory;

//- Factory registering ------------------------------------------------------

// CGrpProfileGuardFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileGuardFactory, "guard");

// CGrpProfileTribuFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileTribuFactory, "tribu");

// CGrpProfileIdleFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileIdleFactory, "idle");

// CGrpProfileStandAtStartPointFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileStandAtStartPointFactory, "stand_on_start_point");

// CGrpProfileEscortedFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileEscortedFactory, "escorted");

// CGrpProfileGuardEscorted Factory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileGuardEscortedFactory, "guard_escorted");

// CGrpProfileSquadFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileSquadFactory, "squad");

// CGrpProfileFactionFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileFactionFactory, "faction");

// CGrpProfileStandOnVerticesFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileStandOnVerticesFactory, "stand_on_vertices");

// CGrpProfileWanderFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileWanderFactory, "wander");

// CGrpProfileNormalFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileNormalFactory, "normal");

// CGrpProfileFollowRouteFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileFollowRouteFactory, "follow_route");

// CGrpProfileNoChangeFactory
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileNoChangeFactory, "no_change");

// CGrpProfileBanditFactory
NLMISC::CSmartPtr<IAIProfile> CGrpProfileBanditFactory::createAIProfile(CProfileOwner* owner)
{
	LOG("bandit group profile factory create");
	static bool firstCall = true;
	if (firstCall)
	{
		CConfigFile::CVar *v = IService::getInstance()->ConfigFile.getVarPtr("DefaultBanditAggroRange");
		if (v)
			_DefaultAggroRange = v->asFloat();
		else
			_DefaultAggroRange = 15.0f;
	}
	
	return new CGrpProfileBandit(owner);
}
float CGrpProfileBanditFactory::getDefaultBanditAggroRange()
{
	return _DefaultAggroRange;
}
float CGrpProfileBanditFactory::_DefaultAggroRange;
RYAI_REGISTER_PROFILE_FACTORY(CGrpProfileBanditFactory, "bandit");

// Global profile factory stuff. This should be put in some profile.cpp since it's common with fauna profiles.
IAIProfileFactory* lookupAIGrpProfile(const char *name)
{
	IAIProfileFactory *ret =  CAiFactoryIndirect<IAIProfileFactory, std::string>::instance().getFactory(std::string(name));
	if (!ret)
		nlwarning("Can't find activity '%s', returning NULL", name);
	return ret;
}
RYAI_IMPLEMENT_FACTORY_INDIRECT(IAIProfileFactory, std::string);

//***************************************************************************/
/* Below that line is magical ununderstandable stuff                        */
//***************************************************************************/

float getDistBetWeen(CAIEntityPhysical& creat1, CAIEntityPhysical& creat2)
{
//	coz player position is not updated really 'goodly' as it can be in a invalid ai map position.
	float angTo = (creat1.pos().angleTo(creat2.pos())).asRadians();
	
	return creat1.getCollisionDist(angTo) + creat2.getCollisionDist(-angTo);
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileMoveTo                                                        //
//////////////////////////////////////////////////////////////////////////////

CBotProfileMoveTo::CBotProfileMoveTo(AITYPES::TVerticalPos verticalPos, RYAI_MAP_CRUNCH::CWorldPosition const& dest, CProfileOwner* owner) 
: CAIBaseProfile()
, _VerticalPos(verticalPos)
, _Dest(dest)
, _PathCont(NLMISC::safe_cast<CSpawnBotNpc*>(owner)->getAStarFlag())
, _PathPos(NLMISC::safe_cast<CSpawnBotNpc*>(owner)->theta())
, _Bot(NLMISC::safe_cast<CSpawnBotNpc*>(owner))
{
	PROFILE_LOG("bot", "move_to", "ctor", "");
#ifdef NL_DEBUG_PTR
	_Bot.setData(this);
#endif
#if !FINAL_VERSION
	nlassert(dest.x()!=0 || dest.y()!=0);
#endif
}

void CBotProfileMoveTo::beginProfile()
{
	PROFILE_LOG("bot", "move_to", "begin", "");
	_LastPos = CAIVector(_Bot->pos());
	_PathCont.setDestination(_VerticalPos, _Dest);
	_Status = CFollowPath::FOLLOWING;
}

void CBotProfileMoveTo::endProfile()
{
	PROFILE_LOG("bot", "move_to", "end", "");
}

void CBotProfileMoveTo::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(BotMoveToProfileUpdate);
	CFollowPathContext fpcBotMoveToProfileUpdate("BotMoveToProfileUpdate");

	if (!_Bot->canMove())
		return;
	
	if (_Status != CFollowPath::FOLLOW_ARRIVED)
	{
		static const std::string runParameter("running");
		float	dist;
		if (_Bot->getPersistent().getOwner()->getSpawnObj()->checkProfileParameter(runParameter))
			dist =_Bot->runSpeed()*ticksSinceLastUpdate;		
		else
			dist =_Bot->walkSpeed()*ticksSinceLastUpdate;
		
		_Status = CFollowPath::getInstance()->followPath(
				_Bot,
				_PathPos,
				_PathCont,
				dist,
				0.f,
				.5f);
		if (_Status==CFollowPath::FOLLOW_NO_PATH)
		{
			// get a base pointer to allow virtual call to work
			
			nlwarning("Follow No Path : %s", _Bot->getPersistent().getOneLineInfoString().c_str());
		}
	}
}

bool CBotProfileMoveTo::destinationReach()	const
{
	return	_Status == CFollowPath::FOLLOW_ARRIVED
		||	_Status==CFollowPath::FOLLOW_NO_PATH;
}

std::string CBotProfileMoveTo::getOneLineInfoString() const
{
	return "move_to bot profile";
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFollowPos                                                     //
//////////////////////////////////////////////////////////////////////////////

CBotProfileFollowPos::CBotProfileFollowPos(CBotProfileFollowPos const& other)
: CAIBaseProfile()
, _PathPos(const_cast<CBotProfileFollowPos&>(other)._PathPos._Angle) // Just to debug...
, _Bot(const_cast<CBotProfileFollowPos&>(other)._Bot)
, _PathCont(const_cast<CBotProfileFollowPos&>(other)._PathCont)
, _MaxWalkSpeed(FLT_MAX)
, _MaxRunSpeed(FLT_MAX)
, _Stop(false)
{
	PROFILE_LOG("bot", "follow_pos", "ctor", "");
#ifdef NL_DEBUG_PTR
	_Bot.setData(this);
#endif
}

CBotProfileFollowPos::CBotProfileFollowPos(CPathCont* pathCont, CProfileOwner* owner)
: CAIBaseProfile()
, _PathPos(NLMISC::safe_cast<CSpawnBotNpc*>(owner)->theta())
, _Bot(NLMISC::safe_cast<CSpawnBotNpc*>(owner))
, _PathCont(pathCont)
, _MaxWalkSpeed(FLT_MAX)
, _MaxRunSpeed(FLT_MAX)
, _Stop(false)
{
	PROFILE_LOG("bot", "follow_pos", "ctor", "");
#ifdef NL_DEBUG
	nlassert(pathCont);
#endif
}

void CBotProfileFollowPos::beginProfile()
{
	PROFILE_LOG("bot", "follow_pos", "begin", "");
	_Status = CFollowPath::FOLLOWING;
}

void CBotProfileFollowPos::endProfile()
{
	PROFILE_LOG("bot", "follow_pos", "end", "");
}

void CBotProfileFollowPos::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(BotFollowPosProfileUpdate);
	CFollowPathContext fpcBotFollowPosProfileUpdate("BotFollowPosProfileUpdate");

	if (!_Bot->canMove() || _Stop)
		return;

	static const std::string runParameter("running");
	float	dist;
	float	speed;
	if (_Bot->getPersistent().getOwner()->getSpawnObj()->checkProfileParameter(runParameter))
		speed = std::min(_Bot->runSpeed(), _MaxRunSpeed);
	else
		speed = std::min(_Bot->walkSpeed(), _MaxWalkSpeed);
	
	dist = speed*ticksSinceLastUpdate;		
	
	CPathCont	&pathContRef=*_PathCont;
	if (_Status!=CFollowPath::FOLLOW_NO_PATH)
	{
		_Status = CFollowPath::getInstance()->followPath(
				_Bot,
				_PathPos,
				pathContRef,
				dist,
				0,
				.5f);
	}
	if (_Status==CFollowPath::FOLLOW_NO_PATH)
	{
		// R2_PRIMITIVE_LAXITY
		if (!IsRingShard.get())
		{
			nlwarning("Follow No Path for '%s'%s",
				_Bot->getPersistent().getAliasFullName().c_str(),
				_Bot->getPersistent().getAliasString().c_str());
		}
	}
}

std::string CBotProfileFollowPos::getOneLineInfoString() const
{
	std::string info = "follow_pos bot profile";
	info += " stop=" + NLMISC::toString(_Stop);
	info += " max_walk_speed=" + NLMISC::toString(_MaxWalkSpeed);
	info += " max_run_speed=" + NLMISC::toString(_MaxRunSpeed);
	return info;
}

void CBotProfileFollowPos::setMaxSpeeds(float walkSpeed, float runSpeed)
{
	_MaxWalkSpeed = walkSpeed;
	_MaxRunSpeed = runSpeed;
}

void CBotProfileFollowPos::setStop(bool stop)
{
	_Stop = stop;
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileWanderBase                                                    //
//////////////////////////////////////////////////////////////////////////////

CBotProfileWanderBase::CBotProfileWanderBase()
: CAIBaseProfile()
{
	PROFILE_LOG("bot", "wander_base", "ctor", "");
}

void CBotProfileWanderBase::setTimer(uint32 ticks)
{
	_Timer.set(ticks);
}

bool CBotProfileWanderBase::testTimer() const
{
	return _Timer.test();
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileStandAtPos                                                    //
//////////////////////////////////////////////////////////////////////////////

CBotProfileStandAtPos::CBotProfileStandAtPos(CProfileOwner* owner)
: CBotProfileWanderBase()
, _Bot(NLMISC::safe_cast<CSpawnBotNpc*>(owner))
{
	PROFILE_LOG("bot", "stand_at_pos", "ctor", "");
#ifdef NL_DEBUG_PTR
	_Bot.setData(this);
#endif
}

void CBotProfileStandAtPos::beginProfile()
{
	PROFILE_LOG("bot", "stand_at_pos", "begin", "");
}

void CBotProfileStandAtPos::endProfile()
{
	PROFILE_LOG("bot", "stand_at_pos", "end", "");
}

void CBotProfileStandAtPos::updateProfile(uint ticksSinceLastUpdate)
{
}

std::string CBotProfileStandAtPos::getOneLineInfoString() const
{
	return "stand_at_pos bot profile";
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileForage                                                        //
//////////////////////////////////////////////////////////////////////////////

CBotProfileForage::CBotProfileForage(CProfileOwner* owner)
: CBotProfileWanderBase()
, _Bot(NLMISC::safe_cast<CSpawnBotNpc*>(owner))
, _TemporarySheetUsed(false)
{
	PROFILE_LOG("bot", "forage", "ctor", "");
#ifdef NL_DEBUG_PTR
	_Bot.setData(this);
#endif
	_OldSheet = 0;
}

CBotProfileForage::~CBotProfileForage()
{
	PROFILE_LOG("bot", "forage", "dtor", "");
#ifdef NL_DEBUG
	nlassert(_TemporarySheetUsed==false);
#endif
	if (_TemporarySheetUsed)
	{
		setOldSheet();	//	if this leads to a virtual pure call somewhere,
						//	try to ask profileOwner to flush its profile in Bot dtor,
						//	which is a proper way to delete objects.
	}
}

void CBotProfileForage::beginProfile()
{
	PROFILE_LOG("bot", "forage", "begin", "");
	// begin first forage in 3-10 second
	_ForageTimer.set(30+CAIS::rand32(70));
	
	static const NLMISC::CSheetId forageTool("itforage.sitem");
	/*
	AISHEETS::ICreature *cs = new AISHEETS::CCreature(*_Bot->getPersistent().getSheet());
	_OldSheet = _Bot->getPersistent().getSheet();
	cs->LeftItem = NLMISC::CSheetId();
	cs->RightItem = forageTool;
	_Bot->getPersistent().setSheet(cs);
	_Bot->getPersistent().sendVPA();
	_TemporarySheetUsed	=	true;
	*/
	nlerror("This profile has been broken (above block commented), it shouldn't be used");
	
	// begin propecting
	_Bot->setBehaviour(MBEHAV::PROSPECTING);
}

void CBotProfileForage::endProfile()
{
	PROFILE_LOG("bot", "forage", "end", "");
	// stop foraging if needed
	if	(_Bot->getBehaviour() == MBEHAV::EXTRACTING)
		_Bot->setBehaviour(MBEHAV::EXTRACTING_END);
	else if	(_Bot->getBehaviour() == MBEHAV::PROSPECTING)
		_Bot->setBehaviour(MBEHAV::PROSPECTING_END);
	/*
	setOldSheet();
	*/
	nlerror("This profile has been broken (above block commented), it shouldn't be used");
}

void CBotProfileForage::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(BotForageProfileUpdate);
	CFollowPathContext fpcBotForageProfileUpdate("BotForageProfileUpdate");

	if (_ForageTimer.test())
	{
		// somethink to do :)
		if	(_Bot->getBehaviour() != MBEHAV::EXTRACTING)
		{
			// do a little turn
			CAngle a = _Bot->theta();
			CAngle newAngle(CAIS::randPlusMinus(CAngle::PI/4));
			_Bot->setTheta(a+newAngle);
			
			// begin foraging
			_Bot->setBehaviour(MBEHAV::EXTRACTING);
			
			// forage for 10 to 15 sec
			_ForageTimer.set(100+CAIS::rand32(50));
		}
		else if (_Bot->getBehaviour() == MBEHAV::EXTRACTING)
		{
			// end foraging
			_Bot->setBehaviour(MBEHAV::PROSPECTING);
			
			// wait 20-30s before next forage
			_ForageTimer.set(250+CAIS::rand32(50));
		}
	}
}

void CBotProfileForage::setOldSheet()
{
	AISHEETS::ICreatureCPtr cs = _Bot->getPersistent().getSheet();
	_Bot->getPersistent().setSheet(_OldSheet);
	_Bot->getPersistent().sendVPA();
	_TemporarySheetUsed = false;
//	delete const_cast<AISHEETS::ICreature*>(cs);
	nlerror("This profile has been broken (above line commented), it shouldn't be used");
}

std::string CBotProfileForage::getOneLineInfoString() const
{
	return "forage bot profile";
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileNormal                                                        //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileNormal::CGrpProfileNormal(CProfileOwner* owner)
: _Grp(NLMISC::safe_cast<CSpawnGroupNpc*>(owner))
//, _GroupFighting(false) // :KLUDGE: Replaced by a bug simulation
{
	PROFILE_LOG("group", "normal", "ctor", "");
	// This should be part of init list, but can't do conditionnal init
	if (simulateBug(7))
	{
		/* Following statement was missing in initialization list. */
	}
	else
	{
		_GroupFighting = false;
	}
}

bool CGrpProfileNormal::isGroupFighting() const
{
	return _GroupFighting;
}

void CGrpProfileNormal::setGroupFighting(bool groupFighting)
{
	if (_GroupFighting!=groupFighting)
	{
		_GroupFighting=groupFighting;
		
		CGroupNpc	&grp=_Grp->getPersistent();
		if (groupFighting)
			grp.processStateEvent(grp.getEventContainer().EventGroupBeginFight);
		else
			grp.processStateEvent(grp.getEventContainer().EventGroupEndFight);
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSlaveProfile                                                            //
//////////////////////////////////////////////////////////////////////////////

CSlaveProfile::CSlaveProfile(CProfileOwner* owner)
: _Grp(NLMISC::safe_cast<CSpawnGroupNpc*>(owner))
{
	PROFILE_LOG("base", "slave", "ctor", "");
}

void CSlaveProfile::beginProfile()
{
	PROFILE_LOG("base", "slave", "begin", "");
	FOREACH(itBot, CCont<CBot>, _Grp->bots())
	{
		CBot* bot = *itBot;
		CSpawnBot* spawnBot = bot->getSpawnObj();
		if (!spawnBot || !spawnBot->isAlive())
			continue;
		addBot(bot);
	}
}		

void CSlaveProfile::updateProfile(uint ticksSinceLastUpdate)
{
}

//////////////////////////////////////////////////////////////////////////////
// CMoveProfile                                                             //
//////////////////////////////////////////////////////////////////////////////

CMoveProfile::CMoveProfile(CProfileOwner* owner) 
: CSlaveProfile(owner)
, _MaxRunSpeed(FLT_MAX)
, _MaxWalkSpeed(FLT_MAX)
{
	PROFILE_LOG("base", "move", "ctor", "");
}

void CMoveProfile::resumeBot(CBot const* bot)
{
	CSpawnBot* spawnBot = bot->getSpawnObj();
	if (!spawnBot)
		return;
	
	CPathCont* pathCont = getPathCont(bot);
	if (!pathCont)
		return;
	
	spawnBot->setAIProfile(new CBotProfileFollowPos(pathCont, spawnBot));
}

//////////////////////////////////////////////////////////////////////////////
// CFightProfile                                                            //
//////////////////////////////////////////////////////////////////////////////

CFightProfile::CFightProfile(CProfileOwner* owner)
: CSlaveProfile(owner)
{
	PROFILE_LOG("base", "fight", "ctor", "");
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileFollowRoute                                                   //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileFollowRoute::~CGrpProfileFollowRoute()
{
	PROFILE_LOG("group", "follow_route", "dtor", "");
	FOREACH(itBot, CCont<CBot>, _Grp->bots())
		removeBot(*itBot);
}

CGrpProfileFollowRoute::CBotFollower::CBotFollower()
: _BotAtDest(false)
{
}

CGrpProfileFollowRoute::CBotFollower::~CBotFollower()
{
}

void CGrpProfileFollowRoute::CBotFollower::setBotAtDest(bool atDest)
{
	_BotAtDest = atDest;
}

const bool& CGrpProfileFollowRoute::CBotFollower::isBotAtDest() const
{
	return _BotAtDest;
}

bool CGrpProfileFollowRoute::getDirection()
{
	return _FollowForward;
}


CPathCont* CGrpProfileFollowRoute::getPathCont(CBot const* bot)
{
	return &_PathCont;
}

std::string CGrpProfileFollowRoute::getOneLineInfoString() const
{
	std::string info = "follow_route group profile";
	info += " stop_npc=" + NLMISC::toString(_StopNpc);
	return info;
}

bool CGrpProfileFollowRoute::profileTerminated() const
{
	return _ProfileTerminated;
}

void CGrpProfileFollowRoute::stopNpc(bool stop)
{
	_StopNpc = stop;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpProfileStandOnVertices                                               //
//////////////////////////////////////////////////////////////////////////////

CGrpProfileStandOnVertices::CBotPositionner::CBotPositionner(RYAI_MAP_CRUNCH::TAStarFlag	flags)
: _PathCont(flags)
{
}

CGrpProfileStandOnVertices::CBotPositionner::CBotPositionner(uint32 geomIndex, RYAI_MAP_CRUNCH::TAStarFlag flags)
: _PathCont(flags)
, _GeomIndex(geomIndex)
, _BotAtDest(false)
{
}

void CGrpProfileStandOnVertices::CBotPositionner::setBotAtDest(bool atDest)
{
	_BotAtDest=atDest;
}

bool CGrpProfileStandOnVertices::CBotPositionner::isBotAtDest() const
{
	return _BotAtDest;
}

CGrpProfileStandOnVertices::CGrpProfileStandOnVertices(CProfileOwner* owner)
: CMoveProfile(owner)
, _DenyFlags(NLMISC::safe_cast<CSpawnGroup*>(owner)->getPersistent().getAStarFlag())
{
	PROFILE_LOG("group", "stand_on_vertices", "ctor", "");
}

CGrpProfileStandOnVertices::~CGrpProfileStandOnVertices()
{
	PROFILE_LOG("group", "stand_on_vertices", "dtor", "");
	for (CCont<CBot >::iterator it=_Grp->bots().begin(), itEnd=_Grp->bots().end();it!=itEnd;++it)
	{
		removeBot(*it);
	}
}

void CGrpProfileStandOnVertices::endProfile()
{
	PROFILE_LOG("group", "stand_on_vertices", "end", "");
}

std::string CGrpProfileStandOnVertices::getOneLineInfoString() const
{
	std::string info = "stand_on_vertices group profile";
	info += " finished=" + NLMISC::toString(_Finished);
	return info;
}


//- Complex profile factories ------------------------------------------------

// CBotProfileMoveToFactory (with specialization)
typedef CAIGenericProfileFactory<CBotProfileMoveTo> CBotProfileMoveToFactory;
template <>
NLMISC::CSmartPtr<IAIProfile> CAIGenericProfileFactory<CBotProfileMoveTo>::createAIProfile(CProfileOwner* owner)
{
	nlerror("This profile factory (CBotProfileMoveToFactory) can't be used");
	return NULL;
}

// CBotProfileFollowPosFactory (with specialization)
typedef CAIGenericProfileFactory<CBotProfileFollowPos> CBotProfileFollowPosFactory;
template <>
NLMISC::CSmartPtr<IAIProfile> CAIGenericProfileFactory<CBotProfileFollowPos>::createAIProfile(CProfileOwner* owner)
{
	nlerror("This profile factory (CBotProfileFollowPosFactory) can't be used");
	return NULL;
}

#include "event_reaction_include.h"
