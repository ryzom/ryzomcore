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

using namespace NLMISC;

extern RYZOMID::TTypeId ai_generic_fight_debugCheckedType;
#define debugCheckedType ai_generic_fight_debugCheckedType

static bool s_attack(CAIEntityPhysical const& target, float const& dist,
	NLMISC::CDbgPtr<CSpawnBot> const& _Bot,
	bool const _UseFightConfig,
	bool const heal)
{
#ifdef NL_DEBUG
	nlassert(EGSHasMirrorReady);
#endif
	if (_UseFightConfig)	//	coz we don't want to lose time for faunas ..
	{
		AISHEETS::ICreatureCPtr sheet = _Bot->getPersistent().getSheet();
		
		uint32 nbchoice = 0;
		AISHEETS::TFightCfg	choices[AISHEETS::FIGHTCFG_MAX];
		
		bool useFightMelee = !heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_MELEE).isNULL();
		bool useFightRange = !heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_RANGE).isNULL();
		bool useFightNuke  = !heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_NUKE).isNULL();
		bool useFightHeal  = heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_HEAL).isNULL();
		
		if (useFightMelee && dist<(CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_MELEE] + _Bot->getPersistent().getSheet()->Radius() * _Bot->getPersistent().getSheet()->Scale()))
		{
			choices[nbchoice] = AISHEETS::FIGHTCFG_MELEE;
			++nbchoice;
		}
		if (useFightRange && dist<CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_RANGE])
		{
			choices[nbchoice] = AISHEETS::FIGHTCFG_RANGE;
			++nbchoice;
		}
		if (useFightNuke && dist<CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_NUKE])
		{
			choices[nbchoice] = AISHEETS::FIGHTCFG_NUKE;
			++nbchoice;
		}
		if (useFightHeal && dist<CBotProfileFightHeal::fightDists[AISHEETS::FIGHTCFG_HEAL])
		{
			choices[nbchoice] = AISHEETS::FIGHTCFG_HEAL;
			++nbchoice;
		}
		
		// :TODO: Remember to fix CSpawnBot::fightWeight when enabling heal
		
		if (nbchoice>0)
		{
			AISHEETS::TFightCfg const choice = choices[CAIS::rand16(nbchoice)];
			AISHEETS::CActionList const* fightConfig = sheet->FightConfig(choice);
			if (fightConfig)
			{
				bool self = _Bot->dataSetRow()==target.dataSetRow();
				std::vector<AISHEETS::IAIActionCPtr> phraseList;
				FOREACHC(itAction, std::vector<AISHEETS::IAIActionCPtr>, fightConfig->_Actions)
				{
					AISHEETS::IAIActionCPtr const& action = *itAction;
					if (action->SelfAction() == self)
						phraseList.push_back(action);
				}
				uint32 const size = (uint32)phraseList.size();
				if (size>0)
				{
					nlassert(target.getRyzomType()!=debugCheckedType);
					CEGSExecuteAiActionMsg msg(_Bot->dataSetRow(), target.dataSetRow(), phraseList[CAIS::rand16(size)]->SheetId(), _Bot->_DamageCoef, _Bot->_DamageSpeedCoef);
					msg.send(egsString);
					return true;
				}
				return false;
			}
		}
	}
	
	if (heal)
	{
		return false;
	}
	else
	{
		nlassert(target.getRyzomType()!=debugCheckedType);
		static NLMISC::CSheetId const defaultAttack("abf01.sphrase");
		
		CEGSExecutePhraseMsg msg(_Bot->dataSetRow(), target.dataSetRow(), defaultAttack);
		msg.send(egsString);
		return true;
	}
}

static void s_updatePosInFloatMode(CAIEntityPhysical* target,
								   NLMISC::CDbgPtr<CSpawnBot> const& _Bot)
{
#ifdef NL_DEBUG
	nlassert(_Bot->getMode()==MBEHAV::COMBAT_FLOAT);
	nlassert(target);
#endif
	// determine the position from the angle.
	CAngle theta = CAIVector(_Bot->pos()).angleTo(CAIVector(target->aipos()));
	_Bot->setTheta(theta);
	float thetaAngle = theta.asRadians();
	
	// get the which distance.
	float dist = (float)(_Bot->getCollisionDist(thetaAngle) + target->getCollisionDist(-thetaAngle) + 0.5);
	
	CAIVector delta(-theta.asVector2d()*dist);
	// update the current position.
	_Bot->setPos(CAIPos(CAIVector(target->aipos())+delta, _Bot->h(), theta));
}

static void s_disengage(CModEntityPhysical& originator,
	bool& _RangeCalculated,
	bool const heal)
{
	NLNET::CMessage msgout(disengageString);
	msgout.serial(const_cast<TDataSetRow&>(originator.dataSetRow()));
	sendMessageViaMirror(egsString, msgout);
	_RangeCalculated = false;
}

static void s_disengage(
	NLMISC::CDbgPtr<CSpawnBot> const& _Bot,
	bool& _RangeCalculated,
	bool& _Engaged,
	bool const heal)
{
	if (_Bot->getMode()!=MBEHAV::NORMAL)
	{
		_Bot->setMode(MBEHAV::NORMAL);
	}
	
	if (_Bot->getMode()==MBEHAV::COMBAT_FLOAT)
	{
		CAIEntityPhysical* target = _Bot->getTarget();
		if (target)
			s_updatePosInFloatMode(target, _Bot);
	}
	
	if (_Engaged)
	{
		s_disengage(*_Bot, _RangeCalculated, heal);
		_Engaged = false;
	}
	_RangeCalculated = false;
}

// Returns true if we attacked
static bool s_hit(
	bool& _RangeCalculated,
	bool& _Engaged,
	CBotProfileFightHeal* profile,
	NLMISC::CDbgPtr<CSpawnBot> const& _Bot,
	bool const _UseFightConfig,
	float const norm,
	float const _RangeMax,
	CAIEntityPhysical* const target,
	bool const heal)
{
	// Updates scripts
	FOREACHC(it, AISHEETS::ICreature::TScriptCompList, _Bot->getPersistent().getSheet()->UpdateScriptList())
	{
		(*it)->update(*_Bot);
	}
	// If hit distance is correct
	if (norm<=_RangeMax)
	{
		float const dist = norm;
		// If we are not yet hitting and attack succeeds
		if (!_Bot->isHitting())
		{
			if (s_attack(*target, dist, _Bot, _UseFightConfig, heal))
			{
				// Set action flags
				_Bot->setActionFlags(RYZOMACTIONFLAGS::Attacks);
				if (heal)
				{
					if (_Bot->dataSetRow()==target->dataSetRow())
						_Bot->selfHealTriggered();
					else
						_Bot->healTriggered();
					s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
					_Bot->setTarget(NULL);
					profile->noMoreTarget();
				}
				return true;
			}
		}
	}
	// If hit distance is out of range...
	else
	{
		float const dist = 0.f;
		// (Check if we have a problem between real and theorical position for player)
		if (target->getRyzomType()==RYZOMID::player && NLMISC::safe_cast<CBotPlayer*>(target)->isUnReachable())
		{
			// Attack anyway
			if (s_attack(*target, dist, _Bot, _UseFightConfig, heal))
			{
				// Set action flags
				_Bot->setActionFlags(RYZOMACTIONFLAGS::Attacks);
				if (heal)
				{
					if (_Bot->dataSetRow()==target->dataSetRow())
						_Bot->selfHealTriggered();
					else
						_Bot->healTriggered();
					s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
					_Bot->setTarget(NULL);
					profile->noMoreTarget();
				}
				return true;
			}
		}
	}
	return false;
}

static void s_calcRanges(
	NLMISC::CDbgPtr<CSpawnBot> const& _Bot,
	bool& _RangeCalculated,
	float& _RangeMax,
	float& _RangeMin,
	bool& _UseFightConfig,
	bool const heal)
{
	if (_Bot.isNULL())
		return;
	
	AISHEETS::ICreatureCPtr sheet = _Bot->getPersistent().getSheet();
	
	bool useFightMelee = !heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_MELEE).isNULL();
	bool useFightRange = !heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_RANGE).isNULL();
	bool useFightNuke  = !heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_NUKE).isNULL();
	bool useFightHeal  = heal && !sheet->FightConfig(AISHEETS::FIGHTCFG_HEAL).isNULL();
	
	_UseFightConfig	= useFightMelee || useFightRange || useFightNuke || useFightHeal;
	
	// Default values
	_RangeMin = CBotProfileFightHeal::fightDefaultMinRange;
	_RangeMax = CBotProfileFightHeal::fightDefaultMaxRange;
	if (_UseFightConfig)
	{
		if (useFightMelee && (useFightRange || useFightNuke || useFightHeal))
		{
			_RangeMin = CBotProfileFightHeal::fightMixedMinRange;
			_RangeMax = CBotProfileFightHeal::fightMixedMaxRange;
		}
		else if (useFightMelee)
		{
			_RangeMin = CBotProfileFightHeal::fightMeleeMinRange;
			_RangeMax = CBotProfileFightHeal::fightMeleeMaxRange;
		}
		else
		{
			_RangeMin = CBotProfileFightHeal::fightRangeMinRange;
			_RangeMax = CBotProfileFightHeal::fightRangeMaxRange;
		}
	}
	_RangeMax += _Bot->getPersistent().getSheet()->Radius() * _Bot->getPersistent().getSheet()->Scale();
	_RangeCalculated = true;
}	

// Returns true if we moved properly
bool s_move(
	// Output (and i/o)
	NLMISC::CDbgPtr<CSpawnBot> const& _Bot,
	CPathPosition& _PathPos,
	bool& _SearchAlternativePath,
	CAIEntityPhysical* target,
	float& norm,
	CAIVector& decalage,
	bool& _RangeCalculated,
	bool& _Engaged,
	CPathCont& _PathCont,
	// Input
	float const contactDist,
	float const _RangeMax,
	float const _RangeMin,
	bool const targetInForbiddenZone,
	uint const ticksSinceLastUpdate,
	bool const heal)
{
	// If we are not looking for an alternative path
	if (!_SearchAlternativePath)
	{
		// Look at the target
		_PathPos._Angle = CAIVector(_Bot->pos()).angleTo(CAIVector(target->aipos()));
		// If we are not in combat mode, change mode
		if (_Bot->getMode()!=MBEHAV::COMBAT)
		{
			// If we were in COMBAT_FLOAT (???), update position a last time
			if (_Bot->getMode()==MBEHAV::COMBAT_FLOAT)
			{
				s_updatePosInFloatMode(target, _Bot);
				// The position has changed, so we have to recalculate everything about it.
				norm = buildDecalage(_Bot->pos(), target->aipos(), &decalage) - contactDist;
			}
			// Set mode to COMBAT
			_Bot->setMode(MBEHAV::COMBAT);
		}
	}
	
	// If we cannot move
	if (!_Bot->canMove())
	{
		// Remember that we cannot search for an alternative path
		_SearchAlternativePath = false;
	}
	// :KLUDGE: useless variable
	bool canResetSearchingPath = !_SearchAlternativePath;
	// If we can move and the target is out of range or we have to look for an alternative path
	if (	_Bot->canMove	()
		&&	(	norm<_RangeMin
			||	norm>_RangeMax
			||	_SearchAlternativePath)
		&& !_Bot->isHitting())
	{
//////////////////////////////////////////////////////////////////////////////
// Compute distance to go ////////////////////////////////////////////////////
		// rescale is the distance between real target distance and the mean range
		float rescale = norm - (_RangeMin+_RangeMax)*0.5f;
//////////////////////////////////////////////////////////////////////////////
// Try to go straight to the target //////////////////////////////////////////
		// We are on the main path and the target is reachable
		if (!_SearchAlternativePath && !targetInForbiddenZone)
		{
			// Save our current pos
			CAIPos const lastPos = _Bot->pos();
			// ???
			_Bot->setMoveDecalage(decalage);
			// Let's run
			float speed = _Bot->runSpeed()*ticksSinceLastUpdate;
			// If we got to go back move -speed or +rescale
			if (rescale<0)
				rescale = std::max(-speed, rescale);
			// If we go forward move +speed or +rescale
			else
				rescale = std::min(speed, rescale);
			
			// Compute a new intermediate position along the path
			CPathPosition localPathPos(_PathPos._Angle);
			localPathPos._PathState = CPathPosition::NO_PATH;
			CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
					_Bot,
					localPathPos,
					_PathCont,
					speed,
					rescale,
					0.5f);
			// If we can't follow the path or the computed pos is invalid
			if (	status==CFollowPath::FOLLOW_NO_PATH
				||	(_Bot->wpos().isValid() && (_Bot->wpos().getFlags()&_Bot->getAStarFlag())!=0))
			{
				// Restore position
				_Bot->setPos(lastPos);
				// Find another path
				_SearchAlternativePath = true;
				if (heal)
					canResetSearchingPath = false;
				else
					canResetSearchingPath = !_SearchAlternativePath;
			}
			
			// If we didn't move enough
			if (	rescale>0
				&&	lastPos.quickDistTo(CAIVector(_Bot->pos()))<(rescale*0.5))
			{
				// Find another path
				_SearchAlternativePath = true;
				canResetSearchingPath = false;
			}
		}
//////////////////////////////////////////////////////////////////////////////
// Compute a more complex path to the target /////////////////////////////////
		// We are looking for an alternative path and the target is reachable
		if (	_SearchAlternativePath
			&&	!targetInForbiddenZone	)
		{
			// Compute a vector to the target
			CAIVector delta = CAIVector(target->aipos()) - CAIVector(_Bot->pos());
			// Keep the distance to the target
			double const lastDist = delta.norm();
			// Save our position
			CAIPos const lastPos = _Bot->pos();
			// Set our destination to the target pos
			_PathCont.setDestination(target->wpos());
			// Let's run
			float speed = _Bot->runSpeed()*ticksSinceLastUpdate;
			// If we got to go back move -speed or +rescale
			if (rescale<0)
				rescale = std::max(-speed, rescale);
			// If we go forward move +speed or +rescale
			else
				rescale = std::min(speed, rescale);
			// Move to the target
			CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
					_Bot,
					_PathPos,
					_PathCont,
					speed,
					rescale,
					0.5f);
			// If we cannot reach the target...
			if (status==CFollowPath::FOLLOW_NO_PATH)
			{
				// ...forget it
				// (exploit?, seems to be a short term memory problem).
				_Bot->forgetAggroFor(target->dataSetRow(), true);
				s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
				if (!heal && CBotProfileFightHeal::fleeUnreachableTargets)
					_Bot->setUnreachableTarget(target);
				else
					_Bot->setTarget(NULL);
				return false;
			}
			// Recompute distance to the target
			delta = CAIVector(target->aipos()) - CAIVector(_Bot->pos());
			// If we didn't come closer recompute a path
			double const epsilon = 0.0000001; // This is to prevent a gcc optimization problem
			_SearchAlternativePath = (delta.norm()+epsilon) > lastDist;
		}
		else
		{
			// Force a correct angle (coz of the repulsion prob)
			_Bot->setTheta(_PathPos._Angle);
		}
		// Recompute vector and dist to target
		norm = buildDecalage(_Bot->pos(), target->aipos(), &decalage) - contactDist;
		// (???)
		if (canResetSearchingPath)
		{
			_PathPos._PathState = CPathPosition::NOT_INITIALIZED;
		}
	}
	// If we cannot move or the target is in range
	else
	{
		// Force a correct angle (coz of the repulsion prob)
		_Bot->setTheta(_PathPos._Angle);
		CPathPosition localPathPos(_PathPos._Angle);
		localPathPos._PathState = CPathPosition::NO_PATH;
		// Speed is half the walking speed.
		float const speed = _Bot->walkSpeed()*ticksSinceLastUpdate*0.5f;
		CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
				_Bot,
				localPathPos,
				_PathCont,
				0.f,
				0.f,
				0.5f,
				true,
				NULL,
				speed,
				false);
	}
	return true;
}

// 33 if, 6+ levels
static void s_updateProfile(
	// Output (or input/output)
	NLMISC::CRefPtr<CAIEntityPhysical>& _Ennemy,
	CBotProfileFightHeal* profile,
	bool& _RangeCalculated,
	bool& _AtAttackDist,
	float& _RangeMax,
	CPathCont& _PathCont,
	bool& _Engaged,
	bool& _SearchAlternativePath,
	CPathPosition& _PathPos,
	float& _RangeMin,
	bool& _UseFightConfig,
	NLMISC::CDbgPtr<CSpawnBot> const& _Bot,
	// Input
	uint const ticksSinceLastUpdate,
	bool const heal)
{
	H_AUTO(BotFightHealProfileUpdate);
	CFollowPathContext fpcBotFightHealProfileUpdate("BotFightHealProfileUpdate");

#ifdef NL_DEBUG
	nlassert(_Bot->isAlive());
#endif
	
/****************************************************************************/
/* Early exits                                                              */
/****************************************************************************/
	
	// If we are paralysed due to a spell or special effect
	if (_Bot->isFeared())
		return;
	
	//////////////////////////////////////////////////////////////////////////////
	// X. VALIDATE TARGET ////////////////////////////////////////////////////////
	CAIEntityPhysical* target = NULL;
	// Get the current target
	target = _Bot->getTarget();
	
	// Assert target is valid and if it has changed take its row
	while (!_Bot->isHitting() && target && target!=_Ennemy)
	{
		// Disengage from fight
		s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
		// Change the target row reference
		_Ennemy = target;
		// Synchronize target row refs (shouldn't it be asserted, since _Row is extracted from getTarget)
		_Bot->setTarget(_Ennemy);
		// Get the current target
		target = _Bot->getTarget();
	}
	// Assert target is valid and alive
	if (!target || !target->isAlive() || (heal && target->hpPercentage()>.9f))
	{
		// If we are hitting an invalid or dead target just wait we finish hitting, else disengage
		if (!_Bot->isHitting())
		{
			// If the target died signal it to whoever is interested
			if (target && !heal)
			{
				profile->eventTargetKilled();
				s_disengage(_Bot, _RangeCalculated, _Engaged, heal); // Disengage from fight
				_Bot->forgetAggroFor(target->dataSetRow());
			}
			else
			{
				s_disengage(_Bot, _RangeCalculated, _Engaged, heal); // Disengage from fight
			}
			if (heal)
				_Bot->setTarget(NULL);
			profile->noMoreTarget();
		}
		return;
	}
	// X. VALIDATE TARGET ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	
	// Compute a vector from us to the target
	CAIVector decalage;
	// contactDist is the distance between our center and our target's when we are touching
	float const contactDist = getDistBetWeen(*_Bot, *target) + 0.5f;
	// norm is the distance between our skin and our target's
	float norm = buildDecalage(_Bot->pos(), target->aipos(), &decalage) - contactDist;
	
	// If target is more than giveUpDistance meters far, quit fight and forget aggro.
	if (norm>CBotProfileFightHeal::giveUpDistance)
	{
		s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
		_Bot->forgetAggroFor(target->dataSetRow(), true);
		return;
	}
	
	// Forget aggro on players in water
	// :TODO: Replace in water test with a cleaner prop
	// :KLUDGE: bit manipulation
	if (target->getRyzomType()==RYZOMID::player && (target->getActionFlags() & RYZOMACTIONFLAGS::InWater))
	{
		// Disengage from fight
		s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
		// Forget the aggro on target
		_Bot->forgetAggroFor(target->dataSetRow(), true);
		return;
	}
	// Ignore players that go out of authorized zones
	if (!heal && !_Bot->isAggroValid(target->dataSetRow()))
	{
		_Bot->forgetAggroFor(target->dataSetRow(), true);
		s_disengage(_Bot, _RangeCalculated, _Engaged, heal);
		_Bot->setTarget(NULL);
		return;
	}
	
/****************************************************************************/
/* Data extraction                                                          */
/****************************************************************************/
	
	// Compute attack ranges
	if (!_RangeCalculated)
		s_calcRanges(_Bot, _RangeCalculated, _RangeMax, _RangeMin, _UseFightConfig, heal);
	// Check if we can attack our target
	_AtAttackDist = norm < _RangeMax;
	
	// Check if target can be attacked
	bool const targetInForbiddenZone = ((!target->wpos().isValid())||(target->wpos().getFlags()&_Bot->getAStarFlag())!=0);
	
/****************************************************************************/
/* Profile main processing                                                  */
/****************************************************************************/
	
	// If the target is further than our max range
	if (norm>(_RangeMax+5.f) && targetInForbiddenZone)
	{
		// If we can move
		if (_Bot->canMove())
		{
			H_AUTO(AntiExploit1_BotFightProfileUpdate);			
			CFollowPathContext fpcAntiExploit1_BotFightProfileUpdate("AntiExploit1_BotFightProfileUpdate");

			// Run...
			float botSpeed = _Bot->runSpeed();
			// ...to the target
			_PathCont.setDestination(target->wpos());
			// If we are in COMBAT_FLOAT (???) update the pos accordingly
			if (_Bot->getMode()==MBEHAV::COMBAT_FLOAT)
				s_updatePosInFloatMode(target, _Bot);
			// Anyway set mode to (normal) combat
			_Bot->setMode(MBEHAV::COMBAT);
		}
	}
	// If the target is near enough
	else
	{
		// Set the engaged state
		_Engaged = true;
		
//////////////////////////////////////////////////////////////////////////////
// A. MOVE TO THE BOT ////////////////////////////////////////////////////////
		// Let's move (if return false movement failed and then stop there)
		if (!s_move(
			/* Output (and i/o): */_Bot, _PathPos, _SearchAlternativePath, target, norm, decalage, _RangeCalculated, _Engaged, _PathCont,
			/* Input: */ contactDist, _RangeMax, _RangeMin, targetInForbiddenZone, ticksSinceLastUpdate, heal))
			return;
// A. MOVE TO THE BOT ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
		
//////////////////////////////////////////////////////////////////////////////
// B. HIT THE TARGET /////////////////////////////////////////////////////////
		// If we are not searching for another path and we are not already hitting, then it's time to hit
		if (	!_Bot->isHitting()
			&&	!_SearchAlternativePath)
		{
			// Let's hit (if return true we attacked and then stop there)
			if (s_hit(_RangeCalculated, _Engaged, profile, _Bot, _UseFightConfig, norm, _RangeMax, target, heal))
				return;
		}
// B. HIT THE TARGET /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
	}
	// If we are not hitting although we could
	if (!_Bot->isHitting() && !_Bot->isRooted())
	{
		// Change target
		_Bot->CBotAggroOwner::update(ticksSinceLastUpdate);
	}
}

void CBotProfileFight::endProfile()
{
	s_disengage(_Bot, _RangeCalculated, _Engaged, /*heal*/false);
	if (((CAIEntityPhysical*)_Bot->getTarget())!=NULL)
		_Bot->setTarget(NULL);	// this bot doesn't have any more target now.
}

void CBotProfileFight::updateProfile(uint ticksSinceLastUpdate)
{
	s_updateProfile(
		// Output (or input/output)
		_Ennemy,
		this,
		_RangeCalculated,
		_AtAttackDist,
		_RangeMax,
		_PathCont,
		_Engaged,
		_SearchAlternativePath,
		_PathPos,
		_RangeMin,
		_UseFightConfig,
		_Bot,
		// Input
		ticksSinceLastUpdate,
		/*heal?*/false);
}

void CBotProfileHeal::endProfile()
{
	s_disengage(_Bot, _RangeCalculated, _Engaged, /*heal*/true);
	if (((CAIEntityPhysical*)_Bot->getTarget())!=NULL)
		_Bot->setTarget(NULL);	// this bot doesn't have any more target now.
}

void CBotProfileHeal::updateProfile(uint ticksSinceLastUpdate)
{
	NLMISC::CRefPtr<CAIEntityPhysical> ennemy = CAIS::instance().getEntityPhysical(_Row);
	s_updateProfile(
		// Output (or input/output)
		ennemy,
		this,
		_RangeCalculated,
		_AtAttackDist,
		_RangeMax,
		_PathCont,
		_Engaged,
		_SearchAlternativePath,
		_PathPos,
		_RangeMin,
		_UseFightConfig,
		_Bot,
		// Input
		ticksSinceLastUpdate,
		/*heal?*/true);
	if (ennemy)
		_Row = ennemy->dataSetRow();
	else
		_Row = TDataSetRow();
}
