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
#include "ai_profile_fauna.h"

#include "ai_bot_fauna.h"
#include "ai_grp_fauna.h"
#include "ai_mgr_fauna.h"

//- Compact logging facilities -----------------------------------------------

CFaunaProfileFloodLogger CWanderFaunaProfile::_FloodLogger(600);
CFaunaProfileFloodLogger CGrazeFaunaProfile::_FloodLogger(600);
CFaunaProfileFloodLogger CRestFaunaProfile::_FloodLogger(600);

//- Profile factories singletons ---------------------------------------------

CPlanteIdleFaunaProfileFactory PlanteIdleFaunaProfileFactory;
CWanderFaunaProfileFactory WanderFaunaProfileFactory;
CGrazeFaunaProfileFactory GrazeFaunaProfileFactory;
CRestFaunaProfileFactory RestFaunaProfileFactory;
CFightFaunaProfileFactory FightFaunaProfileFactory;
CCorpseFaunaProfileFactory CorpseFaunaProfileFactory;
CEatCorpseFaunaProfileFactory EatCorpseFaunaProfileFactory;
CCuriosityFaunaProfileFactory CuriosityFaunaProfileFactory;

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFightFauna                                                    //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileFightFauna
: public CBotProfileFight
{
public:
	CBotProfileFightFauna(CProfileOwner* owner, CAIEntityPhysical* ennemy);
	
	virtual std::string getOneLineInfoString() const { return NLMISC::toString("fight fauna bot profile"); }
	
	void noMoreTarget();
	
	void eventBeginFight();
	void eventTargetKilled();
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileHealFauna                                                     //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileHealFauna
: public CBotProfileHeal
{
public:
	CBotProfileHealFauna(TDataSetRow const& row, CProfileOwner* owner);
	
	virtual std::string getOneLineInfoString() const { return NLMISC::toString("heal fauna bot profile"); }
	
	void noMoreTarget();
	
//	void eventBeginFight();
//	void eventTargetKilled();
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileReturnAfterFightFauna                                         //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileReturnAfterFightFauna
: public CBotProfileReturnAfterFight
, public IMouvementMagnetOwner
{
public:
	CBotProfileReturnAfterFightFauna(CProfileOwner* owner);
	~CBotProfileReturnAfterFightFauna();
	virtual void beginProfile();
	virtual void endProfile();
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual std::string getOneLineInfoString() const { return NLMISC::toString("return_after_fight fauna bot profile"); }
	
	virtual NLMISC::CSmartPtr<CMovementMagnet> const& getMovementMagnet() const { return _MovementMagnet; }
	
private:
	NLMISC::CSmartPtr<CMovementMagnet>	_MovementMagnet;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFightFauna                                                    //
//////////////////////////////////////////////////////////////////////////////

CBotProfileFightFauna::CBotProfileFightFauna(CProfileOwner* owner, CAIEntityPhysical* ennemy)
: CBotProfileFight(owner, ennemy)
{
}

void CBotProfileFightFauna::noMoreTarget()
{
	static_cast<CSpawnBotFauna*>(_Bot.ptr())->setDefaultComportment();
}

void CBotProfileFightFauna::eventBeginFight()
{
	CSpawnBotFauna* spawnable= NLMISC::safe_cast<CSpawnBotFauna*>(_Bot.ptr());
	CGrpFauna &grpFauna = static_cast<CGrpFauna&>(*spawnable->getPersistent().getOwner());
	grpFauna.processStateEvent(grpFauna.getEventContainer().EventBotBeginFight);
}

void CBotProfileFightFauna::eventTargetKilled()
{
	CSpawnBotFauna* spawnable= NLMISC::safe_cast<CSpawnBotFauna*>(_Bot.ptr());
	CGrpFauna *grpFauna = static_cast<CGrpFauna*>(spawnable->getPersistent().getOwner());
	grpFauna->processStateEvent(grpFauna->getEventContainer().EventBotTargetKilled);
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileHealFauna                                                     //
//////////////////////////////////////////////////////////////////////////////

CBotProfileHealFauna::CBotProfileHealFauna(TDataSetRow const& row, CProfileOwner* owner)
: CBotProfileHeal(row, owner)
{
}

void CBotProfileHealFauna::noMoreTarget()
{
	static_cast<CSpawnBotFauna*>(_Bot.ptr())->setDefaultComportment();
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileReturnAfterFightFauna                                         //
//////////////////////////////////////////////////////////////////////////////

CBotProfileReturnAfterFightFauna::CBotProfileReturnAfterFightFauna(CProfileOwner* owner)
: CBotProfileReturnAfterFight(owner)
{
	CSpawnBotFauna* bot = NLMISC::safe_cast<CSpawnBotFauna*>(owner);
	RYAI_MAP_CRUNCH::TAStarFlag denyFlags = bot->getAStarFlag();
	_MovementMagnet = NLMISC::CSmartPtr<CMovementMagnet>(new CReturnMovementMagnet(bot->getReturnPos(), *bot, bot->getAStarFlag()));
}

CBotProfileReturnAfterFightFauna::~CBotProfileReturnAfterFightFauna()
{
}

void CBotProfileReturnAfterFightFauna::beginProfile()
{
//	PROFILE_LOG("bot_fauna", "return_after_fight", "begin", "");
	CBotProfileReturnAfterFight::beginProfile();
	_Bot->ignoreReturnAggro(true);
}

void CBotProfileReturnAfterFightFauna::endProfile()
{
//	PROFILE_LOG("bot_fauna", "return_after_fight", "end", "");
	_Bot->ignoreReturnAggro(false);
	CBotProfileReturnAfterFight::endProfile();
}

void CBotProfileReturnAfterFightFauna::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(CBotProfileReturnAfterFightFaunaUpdate);
	CBotProfileReturnAfterFight::updateProfile(ticksSinceLastUpdate);
	_MovementMagnet->update(0, ticksSinceLastUpdate);
}

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotFauna                                                           //
//////////////////////////////////////////////////////////////////////////////

void CSpawnBotFauna::doFight(CAIEntityPhysical* ennemy)
{
#ifdef NL_DEBUG
	nlassert(ennemy);
#endif
	setAIProfile(new CBotProfileFightFauna(this, ennemy));
}

void CSpawnBotFauna::setDefaultComportment()
{	
	
	if (getPersistent().getRyzomType()==RYZOMID::flora)
		setAIProfile(this, &PlanteIdleFaunaProfileFactory, false);
	else
		setAIProfile(this, &WanderFaunaProfileFactory, false);
}

void CSpawnGroupFauna::setFight(CSpawnBot* bot, CAIEntityPhysical* ennemy)
{
	bot->setAIProfile(new CBotProfileFightFauna(bot, ennemy));
}

void CSpawnGroupFauna::setHeal(CSpawnBot* bot, CAIEntityPhysical* target)
{
	bot->setAIProfile(new CBotProfileHealFauna(target->dataSetRow(), bot));
}

void CSpawnGroupFauna::setNoFight(CSpawnBot* bot)
{
	if (!bot->getTarget().isNULL())
		bot->setTarget(NULL);
	if (	bot->getAIProfileType()==AITYPES::BOT_FLEE
		||	bot->getAIProfileType()==AITYPES::BOT_FIGHT
		||	bot->getAIProfileType()==AITYPES::BOT_HEAL
		||	bot->getAIProfileType()==AITYPES::BOT_RETURN_AFTER_FIGHT	)
	{
		static_cast<CSpawnBotFauna*>(bot)->setDefaultComportment();
	}
}

void CSpawnGroupFauna::setFlee(CSpawnBot* bot, CAIVector& fleeVect)
{
	bot->setMoveDecalage(fleeVect);	
	if (bot->getAIProfileType()!=AITYPES::BOT_FLEE)
	{
		bot->setAIProfile(new CBotProfileFlee(bot));
	}
}

void CSpawnGroupFauna::setReturnAfterFight(CSpawnBot* bot)
{
	if (bot->getAIProfileType()!=AITYPES::BOT_RETURN_AFTER_FIGHT)
	{
		bot->setAIProfile(new CBotProfileReturnAfterFightFauna(bot));
	}
}

//////////////////////////////////////////////////////////////////////////////
// CPlanteIdleFaunaProfile                                                  //
//////////////////////////////////////////////////////////////////////////////

CPlanteIdleFaunaProfile::CPlanteIdleFaunaProfile(CProfileOwner* owner)
: CAIBaseProfile()
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
{
}

void CPlanteIdleFaunaProfile::beginProfile()
{
	_Bot->setMode(MBEHAV::NORMAL);
}

void CPlanteIdleFaunaProfile::endProfile()
{
}

void CPlanteIdleFaunaProfile::updateProfile(uint ticksSinceLastUpdate)
{
}

std::string CPlanteIdleFaunaProfile::getOneLineInfoString() const
{
	return NLMISC::toString("plante_idle fauna profile");
}

//////////////////////////////////////////////////////////////////////////////
// CAIFaunaActivityBaseSpawnProfile                                         //
//////////////////////////////////////////////////////////////////////////////

CAIFaunaActivityBaseSpawnProfile::CAIFaunaActivityBaseSpawnProfile(CProfileOwner* owner)
: CAIBaseProfile()
, _PathPos(NLMISC::safe_cast<CSpawnBotFauna*>(owner)->theta())
, _OutOfMagnet(false)		
{
}

NLMISC::CSmartPtr<CMovementMagnet> const& CAIFaunaActivityBaseSpawnProfile::getMovementMagnet() const
{
	return _MovementMagnet;
}

//////////////////////////////////////////////////////////////////////////////
// CWanderFaunaProfile                                                      //
//////////////////////////////////////////////////////////////////////////////

CWanderFaunaProfile::CWanderFaunaProfile(CProfileOwner* owner)
: CAIFaunaActivityBaseSpawnProfile(owner)
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
{
	_DenyFlags = _Bot->getAStarFlag();
	_MovementMagnet = new CMovementMagnet(*_Bot, _DenyFlags);
}

void CWanderFaunaProfile::beginProfile()
{
	_Bot->setMode(MBEHAV::NORMAL);
	_OutOfMagnet=false;
}

void CWanderFaunaProfile::endProfile()
{
}

void CWanderFaunaProfile::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(WanderFaunaProfileUpdate)
	CFollowPathContext fpcWanderFaunaProfileUpdate("WanderFaunaProfileUpdate");
	
	// calculate distance from bot position to magnet point (used in all the different processes)
	_magnetDistSq=_Bot->pos().distSqTo(_Bot->spawnGrp().magnetPos());
	double grpMagnetRadiusFar=_Bot->spawnGrp().magnetRadiusFar();
	if (_magnetDistSq>(grpMagnetRadiusFar*grpMagnetRadiusFar))
	{
		_Bot->setMode( MBEHAV::NORMAL );
		
		if	(!_OutOfMagnet)
		{
			_PathPos._Angle=_Bot->theta();
			_OutOfMagnet=true;
			_MovementMagnet=NULL;
		}
		
		if	(_Bot->canMove())
		{
			const	float	dist=_Bot->walkSpeed()*ticksSinceLastUpdate;
			CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
					_Bot,
					_PathPos,
					_Bot->spawnGrp().getPathCont(),
					dist,
					dist*0.71f,
					0.5f);
			if (status==CFollowPath::FOLLOW_NO_PATH)
			{
				CFollowPath::TFollowReason lastReason = CFollowPath::getInstance()->lastReason();
				RYAI_MAP_CRUNCH::CWorldMap::TFindInsideAStarPathReason lastFIASPReason = CFollowPath::getInstance()->lastFIASPReason();
#ifdef COMPACT_POS_WARNINGS
				int logTick = CTimeInterface::gameCycle();
				std::string error = _Bot->spawnGrp().getPathCont().getDestPos().toString() + " " + CFollowPath::toString(lastReason, lastFIASPReason);
				++_FloodLogger.logPositions[error];
				if ((logTick-_FloodLogger.logLastTick)>_FloodLogger.logPeriod) // Log errors every minute // :TODO: Put that in a config thing
				{
					if (!_FloodLogger.logPositions.empty())
					{
						nlwarning("bots cannot reach their groups while wandering, positions are:");
						FOREACH(it, CFaunaProfileFloodLogger::TLogPositions, _FloodLogger.logPositions)
						{
							nlwarning(" - %s: %d times", it->first.c_str(), it->second);
						}
						_FloodLogger.logPositions.clear();
					}
					_FloodLogger.logLastTick = logTick;
				}
#else
				nlwarning("Problem with a bot that cannot reach its group from %s to %s %s", _Bot->pos().toString().c_str(), _Bot->spawnGrp().getPathCont().getDestination().toString().c_str(), CFollowPath::toString(lastReason, lastFIASPReason).c_str());
#endif
			}
			
		}
		return;
	}
	
	_OutOfMagnet=false;
	if	(_MovementMagnet.isNull())
		_MovementMagnet=new	CMovementMagnet(*_Bot,_DenyFlags);
	_MovementMagnet->update(30, ticksSinceLastUpdate);
}

std::string CWanderFaunaProfile::getOneLineInfoString() const
{
	return NLMISC::toString("wander fauna profile");
}

//////////////////////////////////////////////////////////////////////////////
// CGrazeFaunaProfile                                                       //
//////////////////////////////////////////////////////////////////////////////

CGrazeFaunaProfile::CGrazeFaunaProfile(CProfileOwner* owner)
: CAIFaunaActivityBaseSpawnProfile(owner)
, _ArrivedInZone(false)
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
{
	_DenyFlags=_Bot->getAStarFlag();
}

void CGrazeFaunaProfile::beginProfile()
{
	_ArrivedInZone=false;
	_OutOfMagnet=false;
	_Bot->setCycleState(CFaunaActivity::CycleStateHungry);
	_CycleTimerBaseTime=_Bot->spawnGrp().getCurrentCycleTime()>>2;	//	4 steps. (sorry, its hard coded)
	_CycleTimer.set(_CycleTimerBaseTime+CAIS::rand16(_CycleTimerBaseTime/4));
}

void CGrazeFaunaProfile::endProfile()
{
}

void CGrazeFaunaProfile::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(GrazeFaunaProfileUpdate)
	CFollowPathContext fpcGrazeFaunaProfileUpdate("GrazeFaunaProfileUpdate");

	// calculate distance from bot position to magnet point (used in all the different processes)
	_magnetDistSq=_Bot->pos().distSqTo(_Bot->spawnGrp().magnetPos());
	
	if	(!_ArrivedInZone)
	{
		float grpMagnetRadiusFar=_Bot->spawnGrp().magnetRadiusFar();
		if	(_magnetDistSq>(grpMagnetRadiusFar*grpMagnetRadiusFar))
		{
			_Bot->setMode( MBEHAV::NORMAL );
			
			if (!_OutOfMagnet)
			{
				_OutOfMagnet=true;
				_PathPos._Angle=_Bot->theta();
			}
			
			if (_Bot->canMove())
			{
				float dist = _Bot->walkSpeed()*ticksSinceLastUpdate;
				CFollowPath::TFollowStatus status = CFollowPath::getInstance()->followPath(
						_Bot,
						_PathPos,
						_Bot->spawnGrp().getPathCont(),
						dist,
						dist*.71f,
						.5f);
				if (status==CFollowPath::FOLLOW_NO_PATH)
				{
					CFollowPath::TFollowReason lastReason = CFollowPath::getInstance()->lastReason();
					RYAI_MAP_CRUNCH::CWorldMap::TFindInsideAStarPathReason lastFIASPReason = CFollowPath::getInstance()->lastFIASPReason();
#ifdef COMPACT_POS_WARNINGS
					int logTick = CTimeInterface::gameCycle();
					std::string error = _Bot->spawnGrp().getPathCont().getDestPos().toString() + " " + CFollowPath::toString(lastReason, lastFIASPReason);
					++_FloodLogger.logPositions[error];
					if ((logTick-_FloodLogger.logLastTick)>_FloodLogger.logPeriod) // Log errors every minute // :TODO: Put that in a config thing
					{
						if (!_FloodLogger.logPositions.empty())
						{
							nlwarning("bots cannot reach their groups while grazing, positions are:");
							FOREACH(it, CFaunaProfileFloodLogger::TLogPositions, _FloodLogger.logPositions)
							{
								nlwarning(" - %s: %d times", it->first.c_str(), it->second);
							}
							_FloodLogger.logPositions.clear();
						}
						_FloodLogger.logLastTick = logTick;
					}
#else
					nlwarning("Follow No Path from %s to %s %s", _Bot->pos().toString().c_str(), _Bot->spawnGrp().getPathCont().getDestPos().toString().c_str(), CFollowPath::toString(lastReason, lastFIASPReason).c_str());
//						_Bot->spawnGrp().addBotToDespawnAndRespawnTime(&_Bot->getPersistent(),1,3000);
#endif
				}
				return;
			}
		}
		else
		{
			_Bot->VisualTargetTimer().set(0);
			_ArrivedInZone=true;
			_MovementMagnet=new	CMovementMagnet(*_Bot,_DenyFlags);
		}
	}
	
	if	(_MovementMagnet.isNull())
		_MovementMagnet=new	CMovementMagnet(*_Bot,_DenyFlags);
	
	if	(_OutOfMagnet)
	{
		_MovementMagnet->setBotAngle();
		_OutOfMagnet=false;
	}
	
	//	increase cycle state when needed.
	if	(_CycleTimer.test())
	{
		_Bot->setCycleState((CFaunaActivity::TCycleState)(_Bot->cycleState()+1));
		if	(_Bot->cycleState()>CFaunaActivity::CycleStateDigesting)
			_Bot->setCycleState(CFaunaActivity::CycleStateDigesting);
		_CycleTimer.set(_CycleTimerBaseTime+CAIS::rand16(_CycleTimerBaseTime/4));
	}
	
	switch	(_Bot->cycleState())
	{
	case CFaunaActivity::CycleStateHungry:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*30, ticksSinceLastUpdate);
		break;
	case CFaunaActivity::CycleStateVeryHungry:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*60, ticksSinceLastUpdate);
		break;
	case CFaunaActivity::CycleStateStarving:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*120, ticksSinceLastUpdate);
		break;
	case CFaunaActivity::CycleStateDigesting:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*30, ticksSinceLastUpdate);
		break;
	}
	
	//	&&	_state==0	)	//	means wait in movementmagnet.
	const float grpMagnetRadiusNear=_Bot->spawnGrp().magnetRadiusNear();
	if	(	_magnetDistSq<=(grpMagnetRadiusNear*grpMagnetRadiusNear)
		&&	_MovementMagnet->getMovementType()==CMovementMagnet::Movement_Anim)
	{
		if	(_Bot->getPersistent().grp().getType()==AITYPES::FaunaTypePredator)
		{
			_Bot->setMode(MBEHAV::ALERT);
		}
		else
		{
			_Bot->setMode(MBEHAV::EAT);
		}
	}
	else
	{
		if (	_Bot->cycleState()==CFaunaActivity::CycleStateHungry
			||	_Bot->cycleState()==CFaunaActivity::CycleStateDigesting)
		{
			_Bot->setMode(MBEHAV::NORMAL);
		}
		else
		{
			_Bot->setMode(MBEHAV::HUNGRY);
		}
	}
}

std::string CGrazeFaunaProfile::getOneLineInfoString() const
{
	return NLMISC::toString("graze fauna profile");
}

//////////////////////////////////////////////////////////////////////////////
// CRestFaunaProfile                                                        //
//////////////////////////////////////////////////////////////////////////////

CRestFaunaProfile::CRestFaunaProfile(CProfileOwner* owner)
: CAIFaunaActivityBaseSpawnProfile(owner)
, _ArrivedInZone(false)
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
{
	_DenyFlags=_Bot->getAStarFlag();
}

void CRestFaunaProfile::beginProfile()
{
	_ArrivedInZone=false;
	_Bot->setCycleState(CFaunaActivity::CycleStateTired);
	_CycleTimerBaseTime=_Bot->spawnGrp().getCurrentCycleTime()>>2;	//	4 steps. (sorry, its hard coded)
	_CycleTimer.set(_CycleTimerBaseTime+CAIS::rand16(_CycleTimerBaseTime/4));
	_OutOfMagnet=false;
}

void CRestFaunaProfile::endProfile()
{
	_Bot->setHungry	();
}

void CRestFaunaProfile::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(RestFaunaProfileUpdate)
	CFollowPathContext fpcRestFaunaProfileUpdate("RestFaunaProfileUpdate");

	// calculate distance from bot position to magnet point (used in all the different processes)
	_magnetDistSq=_Bot->pos().distSqTo(_Bot->spawnGrp().magnetPos());
	
	if (!_ArrivedInZone)
	{
		float grpMagnetRadiusFar=_Bot->spawnGrp().magnetRadiusFar();
		if	(_magnetDistSq>(grpMagnetRadiusFar*grpMagnetRadiusFar))
		{
			if (!_OutOfMagnet)
			{
				_PathPos._Angle=_Bot->theta();
				_OutOfMagnet=true;
			}
			_Bot->setMode( MBEHAV::NORMAL );
			if (_Bot->canMove())
			{				
				const	float	dist=_Bot->walkSpeed()*ticksSinceLastUpdate;
				CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
						_Bot,
						_PathPos,
						_Bot->spawnGrp().getPathCont(),
						dist,
						dist*.71f,
						.5f);
				if (status==CFollowPath::FOLLOW_NO_PATH)
				{
					CFollowPath::TFollowReason lastReason = CFollowPath::getInstance()->lastReason();
					RYAI_MAP_CRUNCH::CWorldMap::TFindInsideAStarPathReason lastFIASPReason = CFollowPath::getInstance()->lastFIASPReason();
#ifdef COMPACT_POS_WARNINGS
					int logTick = CTimeInterface::gameCycle();
					std::string error = _Bot->spawnGrp().getPathCont().getDestPos().toString() + " " + CFollowPath::toString(lastReason, lastFIASPReason);
					++_FloodLogger.logPositions[error];
					if ((logTick-_FloodLogger.logLastTick)>_FloodLogger.logPeriod) // Log errors every minute // :TODO: Put that in a config thing
					{
						if (!_FloodLogger.logPositions.empty())
						{
							nlwarning("bots cannot reach their groups, positions are:");
							FOREACH(it, CFaunaProfileFloodLogger::TLogPositions, _FloodLogger.logPositions)
							{
								nlwarning(" - %s: %d times", it->first.c_str(), it->second);
							}
							_FloodLogger.logPositions.clear();
						}
						_FloodLogger.logLastTick = logTick;
					}
#else
					nlwarning("bot cannot reach its group from %s to %s %s", _Bot->pos().toString().c_str(), _Bot->spawnGrp().getPathCont().getDestPos().toString().c_str(), CFollowPath::toString(lastReason, lastFIASPReason).c_str());
#endif
				}
			}
			return;
		}
		else
		{
			_Bot->VisualTargetTimer().set(0);
			_ArrivedInZone=true;
			_MovementMagnet=new	CMovementMagnet(*_Bot, _DenyFlags);
		}
	}

	if (_MovementMagnet.isNull())
		_MovementMagnet=new	CMovementMagnet(*_Bot, _DenyFlags);

	if (_OutOfMagnet)
	{
		_MovementMagnet->setBotAngle();
		_OutOfMagnet=false;
	}
	
	//	increase cycle state when needed.
	if (_CycleTimer.test())
	{
		_Bot->setCycleState((CFaunaActivity::TCycleState)(_Bot->cycleState()+1));
		if (_Bot->cycleState()>CFaunaActivity::CycleStateShaking)
			_Bot->setCycleState(CFaunaActivity::CycleStateShaking);
		_CycleTimer.set(_CycleTimerBaseTime+CAIS::rand16(_CycleTimerBaseTime/4));
	}
	
	switch(_Bot->cycleState())
	{
	case CFaunaActivity::CycleStateTired:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*80, ticksSinceLastUpdate);
		break;
	case CFaunaActivity::CycleStateVeryTired:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*120, ticksSinceLastUpdate);
		break;
	case CFaunaActivity::CycleStateExhausted:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*200, ticksSinceLastUpdate);
		break;
	case CFaunaActivity::CycleStateShaking:
		_MovementMagnet->update(FAUNA_BEHAVIOR_GLOBAL_SCALE*80, ticksSinceLastUpdate);
		break;
	}
	
	const float grpMagnetRadiusNear=_Bot->spawnGrp().magnetRadiusNear();
	if (	_magnetDistSq<=(grpMagnetRadiusNear*grpMagnetRadiusNear)
		&&	_MovementMagnet->getMovementType()==CMovementMagnet::Movement_Anim)
	{
		_Bot->setMode(MBEHAV::REST);
	}
	else
	{
		_Bot->setMode(MBEHAV::NORMAL);
	}
}

std::string CRestFaunaProfile::getOneLineInfoString() const
{
	return NLMISC::toString("rest fauna profile");
}

//////////////////////////////////////////////////////////////////////////////
// CEatCorpseFaunaProfile                                                   //
//////////////////////////////////////////////////////////////////////////////

CEatCorpseFaunaProfile::CEatCorpseFaunaProfile(CProfileOwner* owner, TDataSetRow const& corpse, RYAI_MAP_CRUNCH::TAStarFlag flag)
: CAIBaseProfile()
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
, _PathPos(NLMISC::safe_cast<CSpawnBotFauna*>(owner)->theta())
, _eated(corpse)
, _atGoodDist(false)
, _PathCont(flag)
{
	CAIEntityPhysical const* const entity = CAIS::instance().getEntityPhysical(_eated);
	if (entity)
		_PathCont.setDestination(AITYPES::vp_auto, CAIVector(entity->pos()));
}

void CEatCorpseFaunaProfile::beginProfile()
{
#ifdef NL_DEBUG
	nlassert(_Bot->canMove	());
#endif
}

void CEatCorpseFaunaProfile::endProfile()
{
	CAIEntityPhysical	*const	entity=CAIS::instance().getEntityPhysical(_eated);

	if (entity)
	{
		CSpawnBotFauna const* const botFauna = NLMISC::safe_cast<CSpawnBotFauna*>(entity);
		IAIProfile* const profile = botFauna->getAIProfile();

		if (_Bot)
		{
			float	pieceOffood=_Bot->radius()*(1.f/3.f);

			if	(pieceOffood>=entity->food())
			{
				pieceOffood=entity->food();
				entity->food()=0;
			}
			else
			{
				entity->food()-=pieceOffood;
			}

			_Bot->hungry()-=pieceOffood;
			if	(_Bot->food()<0)
				_Bot->food()=0;
		}
		
		if	(	profile
			&&	profile->getAIProfileType()==AITYPES::ACTIVITY_CORPSE)
		{
			CCorpseFaunaProfile	*const	corpseProfile=NLMISC::safe_cast<CCorpseFaunaProfile*>(profile);
			corpseProfile->setEater(false);
			corpseProfile->setEated(_Bot->food()==0);
		}
	}
}

void CEatCorpseFaunaProfile::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(EatCorpseFaunaProfileUpdate)
	CFollowPathContext fpcEatCorpseFaunaProfileUpdate("EatCorpseFaunaProfileUpdate");

	CAIEntityPhysical	*const	entity=CAIS::instance().getEntityPhysical(_eated);

	if (entity)
	{
		_Bot->setTarget(entity);

		if (	!_atGoodDist
			&&	_Bot->canMove())
		{
			float	dist=(float)	entity->pos().quickDistTo(_Bot->pos());
			if	(dist>0.5f)	//	0.5f far
			{
				if (!_Bot->canMove())
				{
					return;
				}

				float	moveDist=_Bot->runSpeed()*ticksSinceLastUpdate;
				if	(moveDist>dist*0.25f)	// do go too far ..
				{
					moveDist=dist*0.25f;
				}

				CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
						_Bot,
						_PathPos,
						_PathCont,
						moveDist,
						moveDist*.5f,
						.3f);

				if (status==CFollowPath::FOLLOW_NO_PATH)
				{
					_Bot->setDefaultComportment();	//	.. gives the control to its group.
					return;
				}
				
				if	(status!=CFollowPath::FOLLOW_ARRIVED)
				{
					return;
				}
			}

			_atGoodDist=true;
			_Bot->setTheta(_Bot->pos().angleTo(entity->pos()));
			_Bot->setMode(MBEHAV::EAT);
			_eatTimer.set(100);	//	ten seconds to eat a corpse (vorace!).


			const	CSpawnBotFauna	*const	botFauna=NLMISC::safe_cast<CSpawnBotFauna*>(entity);
			IAIProfile* const profile = botFauna->getAIProfile();
			if	(	profile
				&&	profile->getAIProfileType()==AITYPES::ACTIVITY_CORPSE)
			{
				CCorpseFaunaProfile	*corpseProfile=NLMISC::safe_cast<CCorpseFaunaProfile*>(profile);
				corpseProfile->setEater(true);
			}
			return;					
		}
		else
		{
			if (!_eatTimer.test())	//	did he finished to eat the corpse ?
				return;
		}
	}
	_Bot->setDefaultComportment();	//	.. gives the control to its group.
}

std::string CEatCorpseFaunaProfile::getOneLineInfoString() const
{
	return NLMISC::toString("eat_corpse fauna profile");
}

//////////////////////////////////////////////////////////////////////////////
// CCuriosityFaunaProfile                                                   //
//////////////////////////////////////////////////////////////////////////////

CCuriosityFaunaProfile::CCuriosityFaunaProfile(CProfileOwner* owner, TDataSetRow const& player, RYAI_MAP_CRUNCH::TAStarFlag flag)
: CAIBaseProfile()
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
, _PathPos(NLMISC::safe_cast<CSpawnBotFauna*>(owner)->theta())
, _player(player)
, _atGoodDist(false)
, _Flag(flag)
, _PathCont(flag)
{
#ifdef	NL_DEBUG
	nlassert(owner);
#endif
}

void CCuriosityFaunaProfile::beginProfile()
{
#ifdef	NL_DEBUG
	nlassert(_Bot->canMove	());
#endif
	_curiosityTimer.set(100+CAIS::rand32(100));	// i give you 10 to 20 sec of my time
	_addCuriosityTime=30+CAIS::rand32(30);		//	3 to 6 seconds to say hello.
	_Bot->setMode(MBEHAV::NORMAL);
	_TooFar=true;
}

void CCuriosityFaunaProfile::endProfile()
{
	_Bot->timeBeforeNextCuriosity().set((CAIS::rand32(120)+120)*10*3);	//	consider every 2 to 4 minutes;
	_Bot->setVisualTarget(NULL);
}

void CCuriosityFaunaProfile::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(CuriosityFaunaProfileUpdate)
	CFollowPathContext fpcCuriosityFaunaProfileUpdate("CuriosityFaunaProfileUpdate");

	if	(_curiosityTimer.test())
	{
		_Bot->setDefaultComportment();	//	.. gives the control to its group.
		return;
	}

	CAIEntityPhysical	*const	entity=CAIS::instance().getEntityPhysical(_player);

	if	(entity)
	{

		if	(	!entity->wpos().isValid()
			||	(entity->wpos().getFlags()&_Flag)!=0	//	we don't want to follow player there ..
			||	!entity->isAlive())
		{
			_Bot->setDefaultComportment();	//	.. gives the control to its group.
			return;				
		}
		
		_Bot->setVisualTarget(entity);

		float	dist=(float)	entity->pos().quickDistTo(_Bot->pos());
		if	(dist>(1.f+getDistBetWeen	(*_Bot,	*entity)) )	//	1.f too far
		{
			_TooFar=true;

			if (!_Bot->canMove())
			{
				return;
			}

			_PathCont.setDestination(AITYPES::vp_auto, CAIVector(entity->pos()));

			float	speedCoef=1.f/(dist*0.1f+1.f);
			float	moveDist=(speedCoef*_Bot->walkSpeed()+(1.f-speedCoef)*_Bot->runSpeed())*ticksSinceLastUpdate;
			if (moveDist>dist*0.5f)	// do go too far ..
			{
				moveDist=dist*0.5f;
			}
			CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
					_Bot,
					_PathPos,
					_PathCont,
					moveDist,
					moveDist*.5f,
					.3f);
			if (status==CFollowPath::FOLLOW_NO_PATH)
			{
				_Bot->setDefaultComportment();
			}
			return;
		}

		if (_TooFar)	// just arrived ?
		{
			_Bot->setTheta(_Bot->pos().angleTo(entity->pos()));
			_Bot->setMode(MBEHAV::ALERT);
			
			_curiosityTimer.set((uint32)(_curiosityTimer.timeRemaining()*0.5f+_addCuriosityTime));	//	ten seconds to say hello (cute!).
			_TooFar=false;
		}

	}
}

std::string CCuriosityFaunaProfile::getOneLineInfoString() const
{
	return NLMISC::toString("curiosity fauna profile");
}

//////////////////////////////////////////////////////////////////////////////
// CCorpseFaunaProfile                                                      //
//////////////////////////////////////////////////////////////////////////////

CCorpseFaunaProfile::CCorpseFaunaProfile(CProfileOwner* owner)
: CAIBaseProfile()
, _Bot(NLMISC::safe_cast<CSpawnBotFauna*>(owner))
, _HaveEater(false)
, _Eated(false)
{
}	

inline
void CCorpseFaunaProfile::beginProfile()
{
	_Bot->setTarget(NULL);
	_Bot->setMode(MBEHAV::DEATH);
}

#include "event_reaction_include.h"
