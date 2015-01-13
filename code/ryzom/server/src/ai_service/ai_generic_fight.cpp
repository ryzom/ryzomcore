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
#include "ai_generic_fight.h"
#include "ai_player.h"
#include "server_share/msg_brick_service.h"
#include "ai_profile_npc.h"

extern bool simulateBug(int bugId);

RYZOMID::TTypeId ai_generic_fight_debugCheckedType = RYZOMID::unknown;
#define debugCheckedType ai_generic_fight_debugCheckedType

RYAI_REGISTER_PROFILE_FACTORY(CBotProfileFleeFactory, "bot_flee");
RYAI_REGISTER_PROFILE_FACTORY(CBotProfileFightFactory, "bot_fight");

using namespace NLMISC;
//using namespace std;

std::string CBotProfileFlee::getOneLineInfoString() const
{
	return	std::string("flee profile: bot="+_Bot->dataSetRow().toString()+" "+CMirrors::getEntityId(_Bot->dataSetRow()).toString());
}

std::string CBotProfileFight::getOneLineInfoString() const
{
	return	std::string("fight profile: bot="+_Bot->dataSetRow().toString()+" ennemy="+_Ennemy->dataSetRow().toString()+", Hitting: "+toString(isHitting())+" SearchPath: "+toString(_SearchAlternativePath));
}

std::string CBotProfileHeal::getOneLineInfoString() const
{
	return	std::string("heal profile: bot="+_Row.toString()+" "+CMirrors::getEntityId(_Row).toString()+", Hitting: "+toString(isHitting())+" SearchPath: "+toString(_SearchAlternativePath));
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFight                                                         //
//////////////////////////////////////////////////////////////////////////////

void CBotProfileFight::beginProfile()
{
	_Engaged = false;
	_Bot->setTarget(_Ennemy);
	eventBeginFight	();
}

void CBotProfileFight::resumeProfile()
{
	_Engaged = false;
	_Bot->setTarget(_Ennemy);
}

CBotProfileFight::CBotProfileFight(CProfileOwner* owner, CAIEntityPhysical* ennemy)
: CBotProfileFightHeal()
, _Bot(NLMISC::safe_cast<CSpawnBot*>(owner))
, _Ennemy(ennemy)
, _PathPos(NLMISC::safe_cast<CSpawnBot*>(owner)->theta())
, _PathCont(NLMISC::safe_cast<CSpawnBot*>(owner)->getAStarFlag())
, _RangeCalculated(false)
, _SearchAlternativePath(false)
{
#ifdef NL_DEBUG_PTR
	_Bot.setData(this);
#endif
}

CBotProfileFight::~CBotProfileFight()
{
	if	(	_Bot.isNULL()
		||	_Bot->isAlive())
		return;

	AISHEETS::ICreature::TScriptCompList const& scriptList = _Bot->getPersistent().getSheet()->DeathScriptList();
	FOREACHC(it, AISHEETS::ICreature::TScriptCompList, scriptList)
		(*it)->update(*_Bot);
}

float CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_MAX] = { 0.5f, 80.f, 80.f, 80.f };
float CBotProfileFightHeal::fightDefaultMinRange = 0.01f;
float CBotProfileFightHeal::fightDefaultMaxRange = 0.5f;
float CBotProfileFightHeal::fightMeleeMinRange   = 0.01f;
float CBotProfileFightHeal::fightMeleeMaxRange   = 0.5f;
float CBotProfileFightHeal::fightRangeMinRange   = 0.4f;
float CBotProfileFightHeal::fightRangeMaxRange   = 80.f;
float CBotProfileFightHeal::fightMixedMinRange   = 0.01f;
float CBotProfileFightHeal::fightMixedMaxRange   = 45.f;
float CBotProfileFightHeal::giveUpDistance       = 100.f;
bool  CBotProfileFightHeal::fleeUnreachableTargets = true;


NLMISC_COMMAND(fightMeleeDist, "Generic fight melee attack range","")
{
	if(args.size()>1) return false;
	if(args.size()==1) CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_MELEE] = (float)atof(args[0].c_str());
	log.displayNL("Melee fight range is %f", CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_MELEE]);
	return true;
}
NLMISC_COMMAND(fightRangeDist, "Generic fight range attack range","")
{
	if(args.size()>1) return false;
	if(args.size()==1) CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_RANGE] = (float)atof(args[0].c_str());
	log.displayNL("Range fight range is %f", CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_RANGE]);
	return true;
}
NLMISC_COMMAND(fightNukeDist, "Generic fight nuke attack range","")
{
	if(args.size()>1) return false;
	if(args.size()==1) CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_NUKE] = (float)atof(args[0].c_str());
	log.displayNL("Nuke fight range is %f", CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_NUKE]);
	return true;
}
NLMISC_COMMAND(fightHealDist, "Generic fight heal range","")
{
	if(args.size()>1) return false;
	if(args.size()==1) CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_HEAL] = (float)atof(args[0].c_str());
	log.displayNL("Heal fight range is %f", CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_HEAL]);
	return true;
}
NLMISC_COMMAND(fightDefaultRange, "Generic default fight range","")
{
	if (args.size()==0 || args.size()==2)
	{
		if (args.size()==2)
		{
			CBotProfileFightHeal::fightDefaultMinRange = (float)atof(args[0].c_str());
			CBotProfileFightHeal::fightDefaultMaxRange = (float)atof(args[1].c_str());
		}
		log.displayNL("Generic default fight range is [%f;%f]", CBotProfileFightHeal::fightDefaultMinRange, CBotProfileFightHeal::fightDefaultMaxRange);
		return true;
	}
	return false;
}
NLMISC_COMMAND(fightMeleeRange, "Generic melee fight range","")
{
	if (args.size()==0 || args.size()==2)
	{
		if (args.size()==2)
		{
			CBotProfileFightHeal::fightMeleeMinRange = (float)atof(args[0].c_str());
			CBotProfileFightHeal::fightMeleeMaxRange = (float)atof(args[1].c_str());
		}
		log.displayNL("Generic melee fight range is [%f;%f]", CBotProfileFightHeal::fightMeleeMinRange, CBotProfileFightHeal::fightMeleeMaxRange);
		return true;
	}
	return false;
}
NLMISC_COMMAND(fightRangeRange, "Generic range fight range","")
{
	if (args.size()==0 || args.size()==2)
	{
		if (args.size()==2)
		{
			CBotProfileFightHeal::fightRangeMinRange = (float)atof(args[0].c_str());
			CBotProfileFightHeal::fightRangeMaxRange = (float)atof(args[1].c_str());
		}
		log.displayNL("Generic range fight range is [%f;%f]", CBotProfileFightHeal::fightRangeMinRange, CBotProfileFightHeal::fightRangeMaxRange);
		return true;
	}
	return false;
}
NLMISC_COMMAND(fightMixedRange, "Generic mixed fight range","")
{
	if (args.size()==0 || args.size()==2)
	{
		if (args.size()==2)
		{
			CBotProfileFightHeal::fightMixedMinRange = (float)atof(args[0].c_str());
			CBotProfileFightHeal::fightMixedMaxRange = (float)atof(args[1].c_str());
		}
		log.displayNL("Generic mixed fight range is [%f;%f]", CBotProfileFightHeal::fightMixedMinRange, CBotProfileFightHeal::fightMixedMaxRange);
		return true;
	}
	return false;
}
NLMISC_COMMAND(fightGiveUpDistance, "Generic fight give up distance","")
{
	if(args.size()>1) return false;
	if(args.size()==1) CBotProfileFightHeal::giveUpDistance = (float)atof(args[0].c_str());
	log.displayNL("Give up distance is %f", CBotProfileFightHeal::giveUpDistance);
	return true;
}
NLMISC_COMMAND(fleeUnreachableTargets, "Tells if creatures flee an unreachable player","")
{
	if(args.size()>1) return false;
	if(args.size()==1)
	{
		bool b;
		fromString(args[0], b);
		CBotProfileFightHeal::fleeUnreachableTargets = b;
	}
	log.displayNL("Creatures do%s flee unreachable targets", CBotProfileFightHeal::fleeUnreachableTargets?"":" not");
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileHeal                                                          //
//////////////////////////////////////////////////////////////////////////////

void CBotProfileHeal::beginProfile()
{
	_Engaged=false;
	_Bot->setTarget(CAIS::instance().getEntityPhysical(_Row));
	//	eventBeginFight(); // :TODO: Reactivate this
}

void CBotProfileHeal::resumeProfile()
{
	_Engaged=false;
	_Bot->setTarget(CAIS::instance().getEntityPhysical(_Row));
}

CBotProfileHeal::CBotProfileHeal(const TDataSetRow	&row, CProfileOwner *owner)
: CBotProfileFightHeal()
, _Bot(NLMISC::safe_cast<CSpawnBot*>(owner))
, _PathPos(NLMISC::safe_cast<CSpawnBot*>(owner)->theta())
, _PathCont(NLMISC::safe_cast<CSpawnBot*>(owner)->getAStarFlag())
, _Row(row)
, _RangeCalculated(false)
, _SearchAlternativePath(false)
{
}

CBotProfileHeal::~CBotProfileHeal()
{
	if	(	_Bot.isNULL()
		||	_Bot->isAlive())
		return;

	AISHEETS::ICreature::TScriptCompList const& scriptList = _Bot->getPersistent().getSheet()->DeathScriptList();
	for (AISHEETS::ICreature::TScriptCompList::const_iterator it=scriptList.begin(), itEnd=scriptList.end(); it!=itEnd; ++it)
		(*it)->update(*_Bot);
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileFlee                                                          //
//////////////////////////////////////////////////////////////////////////////

CBotProfileFlee::CBotProfileFlee(CProfileOwner *owner)
: CAIBaseProfile()
, _DenyFlags(NLMISC::safe_cast<CSpawnBot*>(owner)->getAStarFlag())
, _PathPos(NLMISC::safe_cast<CSpawnBot*>(owner)->theta())
, _fightFleePathContainer(NLMISC::safe_cast<CSpawnBot*>(owner)->getAStarFlag())
, _Bot(NLMISC::safe_cast<CSpawnBot*>(owner))
{
}

CBotProfileFlee::~CBotProfileFlee()
{
}

void CBotProfileFlee::beginProfile()
{
	_LastDir = RYAI_MAP_CRUNCH::CDirection(RYAI_MAP_CRUNCH::CDirection::UNDEFINED);
}

void CBotProfileFlee::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(BotFleeProfileUpdate);
	CFollowPathContext fpcBotFleeProfileUpdate("BotFleeProfileUpdate");

	if (!_Bot->getUnreachableTarget().isNULL())
	{
		CAIVector delta = CAIVector(_Bot->getUnreachableTarget()->aipos());
		delta -= CAIVector(_Bot->pos());
		if (delta.quickNorm()>giveUpDistanceUnreachable)
			_Bot->setUnreachableTarget((CAIEntityPhysical*)NULL);
	}
	if	(!_Bot->canMove())
		return;

	bool	calcDone=true;

	CAIVector	fleeVector(_Bot->moveDecalage());
	_Bot->resetDecalage();
	if	(fleeVector.isNull())
		fleeVector.setX(1+fleeVector.x());	// hum ..
	RYAI_MAP_CRUNCH::CDirection	startDir(fleeVector.x(), fleeVector.y(), true);
	
	// if we need to change our destination.
	if	(	startDir!=_LastDir
		||	!_LastStartPos.hasSameFullCellId(_Bot->wpos()))
	{
		const	RYAI_MAP_CRUNCH::CWorldMap	&worldMap=CWorldContainer::getWorldMap();
		calcDone=false;
		
		for	(sint nbStep=0;nbStep<8;nbStep++)
		{
			// try to find a direction around startDir.
			RYAI_MAP_CRUNCH::CDirection	dir(startDir);
			dir.addStep((RYAI_MAP_CRUNCH::CDirection::TDeltaDirection)	((nbStep&1)?(nbStep>>1):(-(nbStep>>1))));
			
			const	RYAI_MAP_CRUNCH::CRootCell	*rootCell=worldMap.getRootCellCst(_Bot->wpos().stepCell(dir.dx(),dir.dy()));
			if	(rootCell)
			{
				RYAI_MAP_CRUNCH::CWorldPosition wpos=rootCell->getWorldPosition(_Bot->getPersistent().getChildIndex()&3);
				if	(	wpos.isValid()
					&&	(wpos.getFlags()&_DenyFlags)==0 )	// verify that we got some compatible flags ..
				{
					_LastDir=startDir;
					_LastStartPos=_Bot->wpos();

					calcDone=true;
					_fightFleePathContainer.setDestination(/*AITYPES::vp_auto, */wpos);
					break;
				}
				
			}
			
		}
		
	}
	
	//	if we found somewhere to go, then go there ..
	if	(calcDone)
	{
		float	dist=_Bot->runSpeed()*ticksSinceLastUpdate;
		CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
				_Bot,
				_PathPos,
				_fightFleePathContainer,
				dist,
				dist*.71f,
				.5f);
		if (status==CFollowPath::FOLLOW_NO_PATH)
		{
			// :KLUDGE: Warning has been removed to avoid flooding, without solving the problem
//			nlwarning("Flee problem with destination properties (Water, Nogo)");
			// :TODO: Rework that case
			_LastDir=RYAI_MAP_CRUNCH::CDirection(RYAI_MAP_CRUNCH::CDirection::UNDEFINED);
		}
	}
	else
	{
		_LastDir=RYAI_MAP_CRUNCH::CDirection(RYAI_MAP_CRUNCH::CDirection::UNDEFINED);
	}

}

float CBotProfileFlee::giveUpDistanceUnreachable = 75.f;

NLMISC_COMMAND(fleeGiveUpDistanceUnreachable, "Generic flee give up distance when fleeing an unreachable player","")
{
	if(args.size()>1) return false;
	if(args.size()==1) CBotProfileFlee::giveUpDistanceUnreachable = (float)atof(args[0].c_str());
	log.displayNL("Give up distance is %f", CBotProfileFlee::giveUpDistanceUnreachable);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CFightOrganizer                                                          //
//////////////////////////////////////////////////////////////////////////////

CFightOrganizer::CFightOrganizer()
: _HaveEnnemy(true)
{
}	

bool CFightOrganizer::healIteration(CBot* bot, CBot* otherBot)
{
	if (bot && otherBot)
	{
		CSpawnBot* spBot = bot->getSpawnObj();
		CSpawnBot* otherSpBot = otherBot->getSpawnObj();
		if (spBot && otherSpBot && otherSpBot->isAlive())
		{
			float hp = otherSpBot->hpPercentage();
			int neededHealers = 0;
			if (hp<.90f) ++neededHealers;
			if (hp<.75f) ++neededHealers;
			if (hp<.50f) ++neededHealers;
			if (hp<.25f) ++neededHealers;
			if (neededHealers > otherSpBot->getHealerCount())
			{
				IAIProfile* profile = spBot->getAIProfile();
				AITYPES::TProfiles profileType = profile?profile->getAIProfileType():AITYPES::BAD_TYPE;
				if (profileType!=AITYPES::BOT_HEAL)
					setHeal(spBot, otherSpBot);
				spBot->setTarget(otherSpBot);
				return true;
			}
		}
	}
	return false;
}

bool CFightOrganizer::reorganizeIteration(CBot* bot)
{
	CSpawnBot	*spawnBot=bot->getSpawnObj();
	if	(	!spawnBot
		||	!spawnBot->isAlive	())
		return true;

	IAIProfile	*profile=spawnBot->getAIProfile();
	AITYPES::TProfiles	profileType=profile?profile->getAIProfileType():AITYPES::BAD_TYPE;
	
	//	special comp if feared bypass every other comp .. (panic mode !)
	if	(spawnBot->isFeared())
	{
		CAIVector fleeVect;
		CAIEntityPhysical* aggroer = spawnBot->firstVisualTargeter();
		while (aggroer!=NULL)
		{
			CAIVector delta(spawnBot->aipos());
			delta -= aggroer->aipos();
			fleeVect += delta;
			aggroer = aggroer->nextTargeter();
		}
		
		CAIS& inst = CAIS::instance();
		
		FOREACHC(itEntry, CBotAggroOwner::TBotAggroList, spawnBot->getBotAggroList())
		{
			CAIEntityPhysical* const phys = inst.getEntityPhysical(itEntry->second->getBot());
			if (!phys)
				continue;
			CAIVector delta(spawnBot->aipos());
			delta -= phys->aipos();
			fleeVect += delta;
		}
		
		fleeVect.normalize(1000);
		CAIVector toGroup = spawnBot->spawnGrp().getCenterPos();
		toGroup -= spawnBot->aipos();
		toGroup.normalize(1000);
		fleeVect += toGroup;
		// :TODO: Uncomment following line and test extensively (may reduce flee speed or dist, but is more correct)
	//	toGroup.normalize(1000);
		setFlee(spawnBot, fleeVect);
		return true;
	}
	// special comp if healer bypass every other comp
	if (profileType==AITYPES::BOT_HEAL)
	{
		CBotProfileHeal* healProfile = NLMISC::safe_cast<CBotProfileHeal*>(profile);
		if (healProfile->isHitting())
		{
			_HaveEnnemy = true;
			return true;
		}
	}
	if (bot->isHealer())
	{
		CGroup* group = bot->getOwner();
		CBot* otherBot = NULL;
		CSpawnBot* otherSp = NULL;
		// Heal leader
		if (spawnBot->canHeal() && healIteration(bot, group->getSquadLeader(true)))
			return true;
		// Heal self
		if (spawnBot->canSelfHeal() && healIteration(bot, bot))
			return true;
		// Heal healers
		if (spawnBot->canHeal())
		{
			FOREACH(itBot, CCont<CBot>, group->bots())
			{
				if (*itBot && bot!=*itBot && (*itBot)->isHealer())
					if (healIteration(bot, *itBot))
						return true;
			}
		}
		// Heal others
		if (spawnBot->canHeal())
		{
			FOREACH(itBot, CCont<CBot>, group->bots())
			{
				if (*itBot && bot!=*itBot)
					if (healIteration(bot, *itBot))
						return true;
			}
		}
	}
	
	if	(profileType==AITYPES::BOT_FIGHT)
	{
		CBotProfileFight	*fightProfile=NLMISC::safe_cast<CBotProfileFight*>(profile);
		if	(fightProfile->isHitting())
		{
			_HaveEnnemy=true;
			return true;
		}

	}
	
	std::vector<CAIEntityPhysical*>	botList;
	AISHEETS::ICreatureCPtr botSheet = bot->getSheet();
	
	float grpAggroCoef = 0.5f*botSheet->GroupCohesionModulator();
	
	spawnBot->updateListAndMarkBot(botList, 1.f-grpAggroCoef);
	
	// unmarkBot list and choose target.
	double	BestChooseScore=0;	//botSheet.ScoreModulator;
	
	CAIEntityPhysical *       ennemy = NULL;
	CAIEntityPhysical const * fleeEnnemy = NULL;
	double	BestFleeScore=0;	//botSheet.FearModulator;
	
	CAIVector	movingVector;
	double		fear=1.0f;

	FOREACH(it, std::vector<CAIEntityPhysical*>, botList)
	{
		CAIEntityPhysical	*const	entity=(*it);
		
		if (!entity->isAlive())
		{
			if (ai_profile_npc_VerboseLog)
			{
				nldebug("<FIGHT>Entity %s have aggro for dead entity %s, forgetting it.", spawnBot->getEntityId().toString().c_str(), entity->getEntityId().toString().c_str());
			}

			spawnBot->forgetAggroFor(entity->dataSetRow());
			continue;
		}
		if (!entity->isBotAttackable())
		{
			if (ai_profile_npc_VerboseLog)
			{
				nldebug("<FIGHT>Entity %s have aggro for non bot attackable entity %s, forgetting it.", spawnBot->getEntityId().toString().c_str(), entity->getEntityId().toString().c_str());
			}

			spawnBot->forgetAggroFor(entity->dataSetRow());
			continue;
		}
		
		// is there a problem.
		if	(entity->_AggroScore>0)
		{
			CAIVector targetToPos(spawnBot->aipos());
			targetToPos -= entity->aipos();
			
			double slotCoef;
			{
				// 1 near - 0 far.
				slotCoef = 1.f/(1.f+targetToPos.quickNorm()*botSheet->DistModulator());	// melee consideration. (don't know correct dist for caster or range may be in munition sheet !?).
				
				if	(((CAIEntityPhysical*)entity->getTarget())!=spawnBot)
				{
					int	nbOtherTargeter = entity->targeterCount();
					if (entity==((CAIEntityPhysical*)spawnBot->getTarget()))
						--nbOtherTargeter;
					float targetCoef = 1.f/(1.f+nbOtherTargeter*nbOtherTargeter*botSheet->TargetModulator());
					slotCoef *= targetCoef;
				}
				slotCoef *= entity->getFreeFightSpaceRatio();
			}
			
//////////////////////////////////////////////////////////////////////////////
			
			float score = (float)(entity->_AggroScore*slotCoef);
			
			if	(score>=BestChooseScore) // add distance and bot profile compatibility.
			{
				BestChooseScore=score;
				ennemy=entity;
			}
		}
		entity->_ChooseLastTime = std::numeric_limits<uint32>::max();
	}
	
	if (fleeEnnemy==NULL && !spawnBot->getUnreachableTarget().isNULL())
	{
		fleeEnnemy = spawnBot->getUnreachableTarget();
	}
	
	if (ennemy)
	{
		_HaveEnnemy=true;
		nlassert(ennemy->getRyzomType()!=debugCheckedType);
		if	(((CAIEntityPhysical*)spawnBot->getTarget())==ennemy)
		{
			return true;
		}
		
		// set the correct profile, if its not the case, otherwise, just change the target.
		if	(profileType!=AITYPES::BOT_FIGHT)
		{
			setFight(spawnBot, ennemy);
		}
		spawnBot->setTarget(ennemy);
	}
	else if (fleeEnnemy)
	{
		CAIVector fleeVect(spawnBot->aipos());
		fleeVect -= fleeEnnemy->aipos();
		// :TODO: Check if following block is wanted by game design
		/*
		CAIVector toGroup = spawnBot->spawnGrp().getCenterPos();
		toGroup -= spawnBot->aipos();
		fleeVect += toGroup;
		*/
		fleeVect.normalize(1000);
		setFlee(spawnBot, fleeVect);
	}
	else if (spawnBot->isReturning())
	{
		_HaveEnnemy=true;
		setReturnAfterFight(spawnBot);
	}
	else
	{
		setNoFight(spawnBot);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileReturnAfterFight                                              //
//////////////////////////////////////////////////////////////////////////////

CBotProfileReturnAfterFight::CBotProfileReturnAfterFight(CProfileOwner* owner)
: CAIBaseProfile()
//, _PathCont(NLMISC::safe_cast<CSpawnBot*>(owner)->getPersistent().getOwner()->getAStarFlag())
{
	_Bot = static_cast<CSpawnBot*>(owner);
//	PROFILE_LOG("bot", "return_after_fight", "ctor", "");
//	CSpawnBot* spawnBot = static_cast<CSpawnBot*>(owner);
//	_PathCont.setDestination(spawnBot->getReturnPos());
//	_MoveProfile = new CBotProfileFollowPos(&_PathCont, owner);
}

CBotProfileReturnAfterFight::~CBotProfileReturnAfterFight()
{
//	PROFILE_LOG("bot", "return_after_fight", "dtor", "");
}

void CBotProfileReturnAfterFight::beginProfile()
{
//	PROFILE_LOG("bot", "return_after_fight", "begin", "");
//	_MoveProfile->beginProfile();
	_Bot->ignoreReturnAggro(true);
}

void CBotProfileReturnAfterFight::endProfile()
{
//	PROFILE_LOG("bot", "return_after_fight", "end", "");
//	_MoveProfile->endProfile();
	_Bot->ignoreReturnAggro(false);
}

void CBotProfileReturnAfterFight::stateChangeProfile()
{
//	_MoveProfile->stateChangeProfile();
}

void CBotProfileReturnAfterFight::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(CBotProfileReturnAfterFightUpdate);
//	_MoveProfile->updateProfile(ticksSinceLastUpdate);
}

std::string CBotProfileReturnAfterFight::getOneLineInfoString() const
{
	std::string info = "return_after_fight bot profile";
//	info += " (";
//	info += _MoveProfile?_MoveProfile->getOneLineInfoString():std::string("<no move profile>");
//	info += ")";
	return info;
}
