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
#include "ai_grp_fauna.h"
#include "ai_mgr_fauna.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

using namespace MULTI_LINE_FORMATER;

using namespace NLMISC;
using namespace RYAI_MAP_CRUNCH;
using namespace	AITYPES;

static char const* stateName(CSpawnGroupFauna::TState s)
{
	switch (s)
	{
	case CSpawnGroupFauna::StateDespawned:	return "DESPAWNED";
	case CSpawnGroupFauna::StateSpawning:	return "SPAWNING";
	case CSpawnGroupFauna::StateGrazing:	return "GRAZING";
	case CSpawnGroupFauna::StateWandering:	return "WANDERING";
	case CSpawnGroupFauna::StateResting:	return "RESTING";
	default:
		break;
	}
	return "UNKNOWN STATE";
}


// helper : get a fauna xyr zone from a base zone or a zone reference
static inline const CFaunaGenericPlace *getFaunaGenericPlace(const CAIPlace *place)
{	
	const CFaunaGenericPlace *faunaPlace = dynamic_cast<const CFaunaGenericPlace *>(place);
	nlassert(faunaPlace);
	return faunaPlace;
}

//////////////////////////////////////////////////////////////////////////////
// CGrpFauna static data                                                    //
//////////////////////////////////////////////////////////////////////////////

CGrpFauna::CCycleDef const CGrpFauna::cycles[] =
{
	{ CSpawnGroupFauna::StateSpawning, CGrpFauna::SPAWN_TIME, CGrpFauna::SPAWN_PLACE },
	{ CSpawnGroupFauna::StateGrazing,  CGrpFauna::EAT_TIME,   CGrpFauna::EAT_PLACE },
	{ CSpawnGroupFauna::StateResting,  CGrpFauna::REST_TIME,  CGrpFauna::REST_PLACE }
};

uint32 const CGrpFauna::nbCycle = sizeof(CGrpFauna::CCycleDef) / sizeof(CGrpFauna::cycles);

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroupFauna                                                         //
//////////////////////////////////////////////////////////////////////////////

CSpawnGroupFauna::CSpawnGroupFauna(CPersistent<CSpawnGroup>& owner, RYAI_MAP_CRUNCH::TAStarFlag denyFlag)
: CSpawnGroup(owner)
, _DespawnImmediately(false)
, _PathCont(denyFlag)
{
	// variables for CAIMgrFauna's update prioritisation system
	_UpdatePriority = 0;	// 0..15 - priority class (distance based - 0 is highest priority)
	
	_Leader = (CBotFauna*)NULL;
	
	_Timer.set(0);
	
	// pick a spawn place
	sint spawnPlace = getPersistent().getNextPlace(NULL, CAIPlaceXYRFauna::FLAG_SPAWN);
	if (spawnPlace == CGrpFauna::INVALID_PLACE)
	{
		std::vector<uint> candidates;
		// seek place with the smallest index
		sint minIndex = INT_MAX;
		for (uint k  = 0; k < getPersistent().places().size(); ++k)
		{	
			const CFaunaGenericPlace *place = getFaunaGenericPlace(getPersistent().places()[k]);
			minIndex = std::min((sint) place->getIndex(), minIndex);
		}
		for (uint k  = 0; k < getPersistent().places().size(); ++k)
		{	
			const CFaunaGenericPlace *place = getFaunaGenericPlace(getPersistent().places()[k]);
			if ((sint) place->getIndex() == minIndex)
			{
				candidates.push_back(k);
			}
		}
		spawnPlace = (sint) candidates[rand() % candidates.size()];		
		//nlwarning("No spawn place found for group %s, using place with smallest index", getPersistent().getName().c_str());		
	}
	setPlace(spawnPlace);
	_CenterPos = _TargetPlace->midPos();
	
	// make sure memory's been allocated for the bots
	if (bots().size()==0)
		getPersistent().allocateBots();
	
	if (bots().size()!=0)
	{
		uint32 const spawnTimer = getPersistent().timer(CGrpFauna::SPAWN_TIME);
		_Timer.set(spawnTimer);
	}
	
	// setup the update priority system variables
	_LastUpdate = CTimeInterface::gameCycle();
	_DeltaTime = 1;
	
	_CurrentCycle = 0;
	
	setMustDespawnBots(false);
}

CSpawnGroupFauna& CSpawnBotFauna::spawnGrp()
{
	return static_cast<CSpawnGroupFauna&>(CSpawnBot::spawnGrp());
}

void CSpawnGroupFauna::despawnGrp()				//	critical code (despawn 'this' object).
{
	CGrpFauna* faunaGrpPtr = &getPersistent();
	faunaGrpPtr->mgr().addToSpawn(faunaGrpPtr);
	faunaGrpPtr->despawnGrp();
}

void CSpawnGroupFauna::recalcUpdatePriorityDeltaAndGroupPos()
{	
	// recalculate the priority delta score for the group based on players in vision
	bool speedUpdate = false;
	
	sint32 grpPosx = 0;
	sint32 grpPosy = 0;
	sint32 nbBots = 0;
	
	FOREACH(it, CCont<CBot>, bots())
	{
		CBotFauna	*bot=NLMISC::safe_cast<CBotFauna*>(*it);
		CSpawnBotFauna	*botFauna=bot->getSpawn();
		if (botFauna)
		{
			if (botFauna->isAlive())
			{
				const	CAIPosMirror	&pos=botFauna->pos();
				grpPosx+=(sint32)(pos.x().asInt()*(1.0/1000.0));
				grpPosy+=(sint32)(pos.y().asInt()*(1.0/1000.0));
				nbBots++;
			}
			
			if (botFauna->havePlayersAround())
			{
				speedUpdate=true;
				break;
			}			
			
		}
		
	}

	if (nbBots>0)
	{
		_CenterPos = CAIVector(grpPosx/nbBots,grpPosy/nbBots);
	}
	
	if (speedUpdate)
		_UpdatePriority = 1;
	else
		_UpdatePriority = 31;
	
	// if players are approaching then crop our move time
	uint32	curTime	= CTimeInterface::gameCycle	();
	if (((sint32)(curTime-_LastUpdate))>(_UpdatePriority+1))
		_LastUpdate	=	curTime-(_UpdatePriority+1);
	
	_DeltaTime = curTime-_LastUpdate;
}

CBotFauna* CSpawnGroupFauna::findLeader()
{
	CBotFauna* possibleLeader = NULL;
	
	CCont<CBot >::iterator it = bots().begin();
	CCont<CBot >::iterator itEnd = bots().end();
	while (it!=itEnd)
	{
		CBotFauna* botPtr = static_cast<CBotFauna*>(*it);
		if	(botPtr->isSpawned())	//	if bot is spawned
		{
			if (botPtr->getSpawnObj()->isAlive())	// is alive
			{
				possibleLeader=botPtr;
				if (_TargetPlace->atPlace(possibleLeader->getSpawn()->pos()))	//	and eventually in place
					return	possibleLeader;
			}
		}
		++it;
	}
	return possibleLeader;
}

void CSpawnGroupFauna::update()
{
	H_AUTO(GrpFaunaUpdate);
	
	++AISStat::GrpTotalUpdCtr;
	++AISStat::GrpFaunaUpdCtr;
	
	getPersistent().updateStateInstance();
	
	if (_CurrentCycle==~0)
		return;
	
	// Respawn
//	breakable
	{
		H_AUTO(GrpFaunaUpdateDealWithDead);
		checkDespawn ();
		
		if	(nbBotToRespawn()>0)
		{
			if	(nbSpawnedBot()>0)	//	(getPersistent().bots().size()/2))
			{
				if	(getPersistent().timeAllowSpawn	())
				{
					checkRespawn();
				}
			}
			else
			{
				//	critical code (despawn 'this' object).
				despawnGrp();
				return;	//	forced becoz the group is despawn and respawned (will be updated next time).
			}
		}
	}
	
	// recalculate our priority rating in function of distance from player
	// if players are approaching then crop our move time
	recalcUpdatePriorityDeltaAndGroupPos();
	
	// identify the leader, call type-dependent update and calculate group position and radius
	{
		H_AUTO(GrpFaunaUpdateByType);
		
		H_TIME(GrpFaunaUpdateFindLeader, _Leader = findLeader(););
		// locate the first live bot and treat them as the group leader
		
		// update the state variable (think about changing state)
		H_TIME(GrpFaunaUpdateCheckTimer, checkTimers(););
		
		H_TIME(GrpFaunaUpdateGeneralUpdate, generalUpdate(););
		
		//	re-find leader as it could have been despawn ..
		_Leader = findLeader();
	}
	// record the current tick as last update time for the group
	_LastUpdate = CTimeInterface::gameCycle();
}

void CSpawnGroupFauna::generalUpdate(TState	state)
{
	H_TIME(GrpFaunaReorganize, reorganize(bots().begin(), bots().end()););
	
	{
		H_AUTO(GrpFaunaUpdDespawnTest);
		
		if (	!mustDespawnBots()
			&&	getUpdatePriority	()>(2<<3)
			&&	!getPersistent().timeAllowSpawn()	)	//	40*3 -> more than 120 meters far from players
		{
			setMustDespawnBots	(true);
		}
	}
	
	if (state==StateUndefined)
		state=CGrpFauna::cycles[_CurrentCycle]._Activity;
	
	// call a type-dependent update
	switch	(getPersistent().getType())
	{
	case FaunaTypeHerbivore:
	case FaunaTypePredator:
		{
			H_AUTO(GrpFaunaUpdNonPlant);
			switch (state)	//	updateAnimals.
			{
			case StateDespawned:	break;
			case StateSpawning:		updateSpawning();	break;
			case StateGrazing:		updateActivity(ACTIVITY_GRAZING);	break;
			case StateWandering:	updateActivity(ACTIVITY_WANDERING);	break;
			case StateResting:		updateActivity(ACTIVITY_RESTING);		break;
			default:	nlwarning("CSpawnGroupFauna::updateAnimals FAILED because state not valid: %d",state);
			}
		}
		break;
		
	case FaunaTypePlant:
		{
			H_AUTO(GrpFaunaUpdPlant);
			// run the state behaviour code
			switch (state)	//	updatePlants.
			{
			case StateDespawned:	break;
			case StateSpawning:		updateSpawning();	break;
			case StateGrazing:		updateActivity(ACTIVITY_PLANTIDLE);	break;
			case StateWandering:	updateActivity(ACTIVITY_PLANTIDLE);	break;
			case StateResting:		updateActivity(ACTIVITY_PLANTIDLE);	break;
			default:	nlwarning("CSpawnGroupFauna::updatePlants FAILED because state not valid: %d",state);
			}
		}
		break;
		
	default:	nlwarning("CSpawnGroupFauna::update() FAILED because group type not valid: %d",getPersistent().getType());
	}
}

bool CSpawnGroupFauna::isSpawning()
{
	return CGrpFauna::cycles[_CurrentCycle]._Activity == StateSpawning;
}

void CSpawnGroupFauna::updateSpawning()
{
	//	check if we have some bots to spawn to reach timer (proportionnaly).
	if	(	(!mustDespawnBots())
			&&	(	(	_Timer.timeRemaining()==0
					&&	nbSpawnedBot()<bots().size())
				||	(	_Timer.totalTime()!=0
					&&	nbSpawnedBot()<((bots().size()*_Timer.timeSinceStart())/_Timer.totalTime()))
					)	)
	{
		uint	i,j;
		uint	targetCount=0;
#ifdef NL_DEBUG
		nlassert(getPersistent().populations()[getPersistent()._CurPopulation]);
#endif
		CPopulation&	curPop=*getPersistent().populations()[getPersistent()._CurPopulation];

		for (i=0;i<curPop.size();++i)
			targetCount+=curPop[i].getBotCount(getPersistent().getCountMultiplierFlag());
		
		// if no more bots to spawn then change state...
		if	(bots()[targetCount-1]->isSpawned())
		{
			incCurrentCycle();
			generalUpdate();
			return;
		}

		// identify the bot to spawn
		uint	count=0;
		{
			CBot*	faunaPt=NULL;
			for (i=0;i<curPop.size();++i)
			{
				CPopulationRecord	&popRecord=curPop[i];
				for (j=0;j<popRecord.getBotCount(getPersistent().getCountMultiplierFlag());++j,++count)
				{
					if	(!bots()[count]->isSpawned())
						break;
				}
				if (j<popRecord.getBotCount(getPersistent().getCountMultiplierFlag()))
					break;
			}

		}

	
		
		// by definition there must be a bot type
		nlassert(i<curPop.size());

		// spawn the bot
		{
			CBotFauna	*faunaPt=NLMISC::safe_cast<CBotFauna*>(bots()[count]);

			faunaPt->setSheet	(/*const_cast<AISHEETS::ICreature *>(*/curPop[i].getCreatureSheet()/*)*/);
			faunaPt->reSpawn	();
		}

		// if this is the first bot to be spawned in the group then st up the leader pointer to point to it
		// and set the group type
		if (count==0)
		{
			_Leader = findLeader();
		}

	}
	// to all intent and purpose consider that our bots are wandering
	generalUpdate(StateWandering);
}

void CSpawnGroupFauna::incCurrentCycle()
{
	setCurrentCycle(_CurrentCycle + 1);
}

void CSpawnGroupFauna::setCurrentCycle(uint32 cycle)
{	
	if (getPersistent().places().isEmpty())
	{
		nlwarning("No places in fauna group %s", getPersistent().getName().c_str());
		return;
	}
	//	did we start a new cycle ?
	if (cycle>=(sizeof(CGrpFauna::cycles)/sizeof(CGrpFauna::CCycleDef)))
		cycle=1;

	const CFaunaGenericPlace *targetPlacePtr = getFaunaGenericPlace(targetPlace());

	sint nextPlace = CGrpFauna::INVALID_PLACE;
	// search a place that match current cycle
	// First we search a neighbour place that has wanted activity
	// otherwise we change activity to the one we found
	switch(CGrpFauna::cycles[cycle]._Place)
	{		
		case CGrpFauna::EAT_PLACE:
			nextPlace = getPersistent().getNextPlace(targetPlacePtr, CAIPlaceXYRFauna::FLAG_EAT); 
			if (nextPlace == CGrpFauna::INVALID_PLACE)
			{				
				nextPlace = getPersistent().getNextPlace(targetPlacePtr, CAIPlaceXYRFauna::FLAG_REST);
				if (nextPlace != CGrpFauna::INVALID_PLACE)
				{
					cycle = CGrpFauna::REST_PLACE; // force to rest
				}
				else
				{
					nextPlace = targetPlace()->getChildIndex();
					// remains in the same place
					if (!targetPlacePtr->getFlag(CAIPlaceXYRFauna::FLAG_EAT))
					{
						// can't eat there, so force to rest
						cycle = CGrpFauna::REST_PLACE; // force to rest
					}
				}
			}			
		break;
		case CGrpFauna::REST_PLACE:
			nextPlace = getPersistent().getNextPlace(targetPlacePtr, CAIPlaceXYRFauna::FLAG_REST); 
			if (nextPlace == CGrpFauna::INVALID_PLACE)
			{
				nextPlace = getPersistent().getNextPlace(targetPlacePtr, CAIPlaceXYRFauna::FLAG_EAT);
				if (nextPlace != CGrpFauna::INVALID_PLACE)
				{
					cycle = CGrpFauna::EAT_PLACE; // force to eat
				}
				else
				{
					nextPlace = targetPlace()->getChildIndex();
					// remains in the same place
					if (!targetPlacePtr->getFlag(CAIPlaceXYRFauna::FLAG_REST))
					{
						// can't rest there, so force to eat
						cycle = CGrpFauna::EAT_PLACE; // force to rest
					}
				}
			}			
		break;
	}		
	_CurrentCycle = cycle;
	setPlace(nextPlace);

	if (_CurrentCycle==CGrpFauna::EAT_PLACE)
	{
		for (CCont<CBot >::iterator it=bots().begin(), itEnd=bots().end();it!=itEnd;++it)
		{
			CBotFauna		*const	faunaBot=NLMISC::safe_cast<CBotFauna*>(*it);
			CSpawnBotFauna	*const	faunaSpawnBot=	faunaBot->getSpawn();
			if	(!faunaSpawnBot)
				continue;

			faunaSpawnBot->hungry()=faunaSpawnBot->radius();
		}
	}
}

void CSpawnGroupFauna::updateActivity(TProfiles activity)
{
	FOREACH(it, CCont<CBot>, bots())
	{
		CBotFauna*	bot=NLMISC::safe_cast<CBotFauna*>(*it);
		if (bot->isSpawned())
			bot->getSpawn()->update(activity,getDt());
	}
}

void CSpawnGroupFauna::checkTimers()
{
	if (CGrpFauna::cycles[_CurrentCycle]._Activity==StateSpawning)
		return;
	
	if (!_ArrivedInZone)	//	 if we are changing the current activity zone.
	{
		if	(	!_Leader.isNULL()
			&&	_Leader->isSpawned())
		{
			const	CAIPos	leaderPos(_Leader->getSpawn()->pos());	
			const	CAIPos	midPos=_TargetPlace->midPos();	//	better is very possible.
			
			if	(leaderPos.distTo(midPos)<_TargetPlace->getRadius())	//	si leader dans la zone.
			{
				_ArrivedInZone=true;	//	we desactivate the boolean.				
				const CFaunaGenericPlace *faunaPlace = getFaunaGenericPlace(targetPlace());
				NLMISC::CRandom rnd;
				uint32 stayTime = faunaPlace->getMinStayTime() + (sint32) (rnd.frand() * ((sint32) faunaPlace->getMaxStayTime() - (sint32) faunaPlace->getMinStayTime()));
				_Timer.set(stayTime);
				/*
				nlwarning("Group %s : Setting stay time to %d in place %s with index %d", 
					      getPersistent().getName().c_str(), 
						  (int) stayTime,
						  faunaPlace->getName().c_str(),
						  (int) faunaPlace->getIndex());
						  */

			}

		}
		
	}
	else
	{
		// nlwarning("Group %s : Timer = %d", getPersistent().getName().c_str(), (int) (100 * _Timer.timeRemaining() / _Timer.totalTime()));
		if	(_Timer.test())	//	si fin de timer.
			incCurrentCycle();
	}
}

void CSpawnGroupFauna::spawnBots()
{
	setMustDespawnBots(false);
	_CurrentCycle = 0; // first activity == spawning.
	nlassert(bots().size()>0);
	_Timer.set(getPersistent().timer(CGrpFauna::SPAWN_TIME));
}

void CSpawnGroupFauna::despawnBots(bool immediately)
{
	setDespawnImmediately(immediately);
	setMustDespawnBots();
}

CAIPos const& CSpawnGroupFauna::magnetPos() const
{ 
	return targetPlace()->midPos();
}

float CSpawnGroupFauna::magnetRadiusNear() const
{		
	return targetPlace()->getRadius()*(7.0f/8.0f);
}

float CSpawnGroupFauna::magnetRadiusFar() const
{		
	return targetPlace()->getRadius()*(9.0f/8.0f);
}

CGrpFauna& CSpawnGroupFauna::getPersistent()
{
	return static_cast<CGrpFauna&>(CSpawnGroup::getPersistent());
}

uint32 CSpawnGroupFauna::getCurrentCycleTime()
{
	return getPersistent().timer(CGrpFauna::cycles[_CurrentCycle]._Time);
}

//////////////////////////////////////////////////////////////////////////////
// CGrpFauna                                                                //
//////////////////////////////////////////////////////////////////////////////


uint32 CGrpFauna::refTimer(TTime time)
{
	switch(time)
	{
		case EAT_TIME:    return  250;
		case REST_TIME:   return  250;
		case SPAWN_TIME:  return  30;
		case CORPSE_TIME: return  120;
		case RESPAWN_TIME: return  45;	// "Backward" compatibility: 45 seconds after the corpse is despawned (after corpse time, or when looted)
		default:
			nlassert(0);
		break;
	}
	return 0;
}

CGrpFauna::CGrpFauna(CMgrFauna* mgr, CAIAliasDescriptionNode* aliasTree, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CGroup(mgr, denyFlags, aliasTree)
, CDynGrpBase()
, CPersistentStateInstance(*mgr->getStateMachine())
{
	
	// state
	
	_CurPopulation = ~0u;
	
	_CurrentCycle = ~0;
	
	// default values.	
	setTimer(EAT_TIME, refTimer(EAT_TIME));
	setTimer(REST_TIME, refTimer(REST_TIME));
	setTimer(SPAWN_TIME, refTimer(SPAWN_TIME));
	setTimer(CORPSE_TIME, refTimer(CORPSE_TIME));		
	setTimer(RESPAWN_TIME, refTimer(RESPAWN_TIME));		
}

void CGrpFauna::stateChange(CAIState const* oldState, CAIState const* newState)
{
}

std::string	CGrpFauna::getOneLineInfoString() const
{
	return std::string("Fauna group '") + getName() + "'";
}

std::vector<std::string> CGrpFauna::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CGrpFauna");
	pushEntry(container, "id=" + CGroup::getIndexString());
	container.back() += " alias=" + getAliasString();
	container.back() += " name=" + getName();
	pushEntry(container, "fullname=" + CGroup::getFullName());
	FOREACHC(it, CCont<CPopulation>, _Populations)
	{
		CPopulation const* pop = *it;
		uint32 index = pop->getChildIndex();
		pushEntry(container, "- population["+toString(index)+"]: "+((_CurPopulation==index)? "* ACTIVE *": ""));
		
		for (uint j=0; j<pop->size(); ++j)
		{
			CPopulationRecord& popRecord = (*pop)[j];
			pushEntry(container, "bots:");
			container.back() += " count="+toString(popRecord.getBotCount(getCountMultiplierFlag()));
			if (popRecord.getCreatureSheet()==NULL)
				container.back() += " <no sheet>";
			else
				container.back() += " sheet='"+popRecord.getCreatureSheet()->SheetId().toString()+"'";
		}
	}
	FOREACHC(it, CCont<CBot>, bots())
	{
		std::vector<std::string> strings = it->getMultiLineInfoString();
		FOREACHC(itString, std::vector<std::string>, strings)
			container.push_back("  " + *itString);
	}
	pushFooter(container);
	
	
	return container;
}

IAliasCont* CGrpFauna::getAliasCont(TAIType type)
{
	switch(type)
	{
	case AITypePlaceFauna:
	case AITypePlace:
		return &_Places;
	case AITypeGrpFaunaPop:
		return &_Populations;
	default:
		return NULL;
	}
}

CAliasTreeOwner* CGrpFauna::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner* child = NULL;
	
	switch (aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
		case AITypePlaceFauna:
			child = new CAIPlaceXYRFauna(this, aliasTree);			
		break;
		case AITypePlace:
			{
				std::string const& name = aliasTree->getName();				
				CAIPlaceXYRFauna *faunaPlace = new CAIPlaceXYRFauna(this, aliasTree);
				child = faunaPlace;	
				uint placeIndex = faunaPlace->setupFromOldName(name);
				nlassert(placeIndex!=~0);				

				if (placeIndex!=~0)
					cont->addAliasChild(child, placeIndex);
				
				return child;
			}
		break;
		case AITypeGrpFaunaPop:
			child = new CPopulation(this, aliasTree);
			break;
	}
	
	if (child)
		cont->addAliasChild(child);
	return	child;
}

void CGrpFauna::displayPlaces(CStringWriter& stringWriter) const
{
	FOREACHC(it, CCont<CAIPlace>, _Places)
	{
		it->display(stringWriter);
	}
}


CGrpFauna::~CGrpFauna() 
{
	if (isSpawned())	// to avoid bad CDbgPtr link interpretation
	{
		despawnGrp();
	}
	
	// unlink all child persistent state instance
	while (!_PSIChilds.empty())
	{
		_PSIChilds.back()->setParentStateInstance(NULL);
	}
	_PSIChilds.clear();
}

void CGrpFauna::setEvent(uint eventId)
{
	nlassert(eventId<10);
	processStateEvent(getEventContainer().EventUserEvent[eventId]);
}

void CGrpFauna::serviceEvent (const CServiceEvent &info)
{
	CGroup::serviceEvent(info);
	
	if ((info.getServiceName() == "EGS") && (info.getEventType() == CServiceEvent::SERVICE_UP))
	{
		processStateEvent(getEventContainer().EventEGSUp);
	}
	
}

NLMISC::CSmartPtr<CSpawnGroup> CGrpFauna::createSpawnGroup()
{
	return new CSpawnGroupFauna(*this, getAStarFlag());
}

bool CGrpFauna::spawn()
{
	if (!getSpawnCounter().remainToMax())
		return false;
	
	setStartState(getStartState());	//	stateInstance.
	return spawnPop(~0);
}

bool CGrpFauna::timeAllowSpawn(uint32 popVersion) const
{
	if (popVersion==12345)
	{
		popVersion = _CurPopulation;
	}
	
	CPopulation* popPtr = _Populations[popVersion];
#ifdef NL_DEBUG
	nlassert(popPtr);
#endif
	if (!popPtr)
	{
		return false;
	}
	TSpawnType st = popPtr->getSpawnType();
	
	bool const& isDay = CTimeInterface::isDay();

	return (st==SpawnTypeAlways) || (isDay&&st==SpawnTypeDay) || (!isDay&&st==SpawnTypeNight);
}

bool CGrpFauna::spawnPop(uint popVersion)
{
	if (places().isEmpty()) return false;
	for (uint k = 0; k < places().size(); ++k)
	{
		if (!places()[k]->worldValidPos().isValid()) return false;
	}
	/*
	if (	!places()[SPAWN_PLACE]->worldValidPos().isValid()
		||	!places()[EAT_PLACE]->worldValidPos().isValid()
		||	!places()[REST_PLACE]->worldValidPos().isValid())	// coz time is not initialized yet ..
		return	false;*/
	
	//	check compatibility.
	/*
	{
		RYAI_MAP_CRUNCH::CCompatibleResult res;
		areCompatiblesWithoutStartRestriction(places()[SPAWN_PLACE]->worldValidPos(), places()[EAT_PLACE]->worldValidPos(), getAStarFlag(), res);
		if	(!res.isValid())
			return	false;
		areCompatiblesWithoutStartRestriction(places()[SPAWN_PLACE]->worldValidPos(), places()[REST_PLACE]->worldValidPos(), getAStarFlag(), res);
		if	(!res.isValid())
			return	false;
		areCompatiblesWithoutStartRestriction(places()[EAT_PLACE]->worldValidPos(), places()[REST_PLACE]->worldValidPos(), getAStarFlag(), res);
		if	(!res.isValid())
			return	false;
	}
	*/
	// check each arc of the graph
	for (uint k = 0; k < places().size(); ++k)
	{
		nlassert(_Places[k]);
		checkArcs(*_Places[k]);
	}

	
	//	check flags ..
	for (uint32 i=0;i<places().size();i++)
	{
		if (!places()[i]->worldValidPos().isValid())
			return	false;
		const	RYAI_MAP_CRUNCH::TAStarFlag	flags=places()[i]->worldValidPos().getTopologyRef().getCstTopologyNode().getFlags();
		if	((flags&getAStarFlag())!=0)
			return	false;
	}
	
	// check the validity of the input parameter
	if (popVersion!=~0 && popVersion>=_Populations.size())
	{
		nlwarning("CGrpFauna::spawn(idx) FAILED for group %s because idx (%d) >= _Populations.size() (%d)",this->CGroup::getFullName().c_str(),popVersion,_Populations.size());
		return	false;
	}
	
	popVersion = ~0;
	
	//	if we are in a cycle.
	if (_CurrentCycle!=~0)
	{
		Cycle const& cycle = _Cycles[_CurrentCycle];
		
		// this to avoid bug dues to bad data initialization.
		do
		{
			++_CurrentCycleIndex;
		} while	(	_CurrentCycleIndex<(sint32)cycle._PopList.size()
				&&	!_Populations[cycle._PopList[_CurrentCycleIndex]]);
		
		if (_CurrentCycleIndex<(sint32)cycle._PopList.size())
		{
			popVersion=cycle._PopList[_CurrentCycleIndex];
			if (!timeAllowSpawn(popVersion))
			{
				popVersion=~0;
			}
		}
		
		if (popVersion==~0)
		{
			_CurrentCycle = ~0;
		}
	}
	
	// if the population version has not been specified then select one at weighted random with day/night difference.
	if (popVersion==~0)
	{
		uint32 totalWeight = 0;
		
		//	we can precalculate this, but it won't appears so much to be called.
		FOREACH(it, CCont<CPopulation>, _Populations)
		{
			CPopulation const& pop = *(*it);
			if (!timeAllowSpawn(pop.getChildIndex()))
				continue;
			totalWeight += pop.getWeight();
		}
		
		if (totalWeight==0)
			return false;
		
		{
			sint32 rnd = CAIS::rand32(totalWeight);
			FOREACH(it, CCont<CPopulation>, _Populations)
			{
				CPopulation	const& pop = *(*it);
				if (!timeAllowSpawn(pop.getChildIndex()))
					continue;
				
				rnd -= pop.getWeight();
				if (rnd>0)	// we found the population to spawn. :)
					continue;
				
				popVersion=pop.getChildIndex();
				break;
			}
		}
		
#if !FINAL_VERSION
		nlassert(popVersion!=~0);
#endif
		if	(popVersion==~0)
			return	false;
		
		//	find if we are starting a new cycle ..
		for	(uint32 i=0;i<_Cycles.size();i++)
		{
			nlassert(_Cycles[i]._PopList.size()>0);
			if (_Cycles[i]._PopList[0]!=popVersion)
				continue;
			
			_CurrentCycle = i;
			_CurrentCycleIndex = 0;
		}
	}
	
	if	(popVersion >= _Populations.size())
	{
		nlwarning("Problem with pop size for group id %s, NAME = %s", this->CGroup::getFullName().c_str(), getName().c_str() );
		return	false;
	}
	
	// setup the pointer to the current population
	_CurPopulation = popVersion;
	
	// check that we have a defined spawn location
	if (!_Places[SPAWN_PLACE])
	{
		nlwarning("CGrpFauna::spawn(idx) FAILED for group %s because _spawnPlace==NULL",this->CGroup::getFullName().c_str());
		return false;
	}
	
	// if the group is already spawned despawn it
	if (isSpawned())
	{
		despawnGrp();
	}
	
	nlassert(_CurPopulation!=~0);
	
	//////////////////////////////////////////////////////////////////////////
	// Init the group type.	
	setType	((*_Populations[_CurPopulation])[0].getCreatureSheet()->FaunaType()); //	gets the first population record of the population to spawn.	
	
	{
		uint32 botCount=0;
		uint32 i;
		CPopulation& curPop = *populations()[_CurPopulation];
		for (i=0; i<curPop.size(); ++i)
		{
			botCount += curPop[i].getBotCount(getCountMultiplierFlag());
			if (	curPop[i].getBotCount(getCountMultiplierFlag()) == 0
				||	curPop[i].getCreatureSheet()->FaunaType() == getType())
				continue;
			
			if (getGroupDesc()) // Dyn system.
			{
				nlwarning("******  WARNING: Different Fauna Type in Template Group %s", getGroupDesc()->getFullName().c_str());
			}
			else
			{
				nlwarning("******  WARNING: Different Fauna Type in group %s", this->CGroup::getFullName().c_str());
			}
		}
		bots().setChildSize(botCount); // set the good size for bots vector.
		
		for (i=0;i<botCount;i++)
			_Bots.addChild(new CBotFauna(getType(), this), i);
	}
	
	return CGroup::spawn();
}

void CGrpFauna::despawnGrp()
{
	CGroup::despawnGrp();
	_CurPopulation = ~0u;
}

//	reads cycle from primitive (string representation).
void CGrpFauna::setCyles(std::string const& cycles)
{
	uint32 strIndex = 0;
	uint32 curCycle = ~0;
	
	while (strIndex<cycles.size())
	{
		char carac = cycles[++strIndex];
		
		if (carac>='A' && carac<='Z')
			carac += 'a'-'A';
		
		if (carac>='a' && carac<='z')
		{
			if (curCycle==~0)
			{
				curCycle = (uint32)_Cycles.size();
				_Cycles.push_back(Cycle());
			}
			Cycle& CycleRef = _Cycles[curCycle];
			CycleRef._PopList.push_back((uint16)(carac-'a'));
		}
		else
		{
			curCycle = ~0;
		}
	}
}

void CGrpFauna::setPopulation(CPopulation* pop) 
{
	CPopulation* sameAliasPop = NULL;
	uint32 index = ~0;
	
	if (pop)
		sameAliasPop = _Populations.getChildByAlias(pop->getAlias());
	
	if (pop && pop->size()==0)	// no population record :(
		pop=NULL;
	
	if (sameAliasPop)	//	Alias already present ?
	{
		index = sameAliasPop->getChildIndex();
		_Populations.addChild(pop, index);	//	automatic deletion with smart pointers
	}
	else
	{
		_Populations.addChild(pop);	//	else simply add it to the populations container
	}
	
	//	if it was the current population, respawn it. (to check with designers?)
	if (index==_CurPopulation)
	{
		if (isSpawned())	// if spawned, despawn.
			getSpawnObj()->despawnGrp();
	}
}

//----------------------------------------------------------------------------
// private utilities
//----------------------------------------------------------------------------

void CGrpFauna::allocateBots()
{
	uint maxPopulation = 0;
	
	// work out how much space we need
	CCont<CPopulation>::iterator it = populations().begin();
	CCont<CPopulation>::iterator itEnd = populations().end();
	
	while (it!=itEnd)
	{
		CPopulation* pop = *(it);
		uint count=0;
		
		for (sint j=(sint)pop->size()-1;j>=0;j--)
			count+=(*pop)[j].getBotCount(getCountMultiplierFlag());
		
		if (count>maxPopulation)
			maxPopulation=count;		
		++it;
	}
	
	_Bots.setChildSize(maxPopulation);
	for	(uint32 i=0;i<maxPopulation;i++)
		_Bots.addChild(new	CBotFauna(getType(),this),i);
}

// Methods for setting up static data ----------------------------------------
void CGrpFauna::setType(TFaunaType type)
{
	faction().removeProperties();
	if (type==AITYPES::FaunaTypePredator)
		faction().addProperty(AITYPES::CPropertyId("Predator"));	
	_Type = type;
}

CMgrFauna& CGrpFauna::mgr() const
{	
	return *static_cast<CMgrFauna*>(getOwner());	
}

CAIS::CCounter& CGrpFauna::getSpawnCounter()
{
	return	CAIS::instance()._FaunaBotCounter;
}

void CGrpFauna::lastBotDespawned()
{
	// send message
	processStateEvent(getEventContainer().EventLastBotDespawned);
}

void CGrpFauna::firstBotSpawned()
{
	setFirstBotSpawned();
}

sint CGrpFauna::getNextPlace(const CFaunaGenericPlace *startPlace, CAIPlaceXYRFauna::TFlag wantedFlag) const
{
	nlassert(wantedFlag < CAIPlaceXYRFauna::FLAG_COUNT);
	std::vector<uint> candidates;
	std::vector<uint> activeCandidates;
	if (!startPlace)
	{
		for (uint k = 0; k < _Places.size(); ++k)
		{
			const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
			if (place->getFlag(wantedFlag))
			{
				if (place->getActive())
				{
					activeCandidates.push_back(_Places[k]->getChildIndex());
				}
				else
				{
					candidates.push_back(_Places[k]->getChildIndex());
				}
			}
		}
	}
	else
	{	
		sint minIndex = INT_MAX;
		sint firstIndex = INT_MAX;
		if (startPlace->getReachNext())
		{
			for (uint k  = 0; k < _Places.size(); ++k)
			{	
				const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
				firstIndex = std::min(firstIndex, (sint) place->getIndex());
				if (place->getIndex() < minIndex && place->getIndex() > startPlace->getIndex())
				{
					minIndex = place->getIndex();
				}
			}
			minIndex = std::max(minIndex, firstIndex);
			for (uint k  = 0; k < _Places.size(); ++k)
			{
				const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
				if ((sint) place->getIndex() == minIndex)
				{
					if (place->getActive())
					{
						activeCandidates.push_back(_Places[k]->getChildIndex());
					}
					else
					{
						candidates.push_back(_Places[k]->getChildIndex());
					}
				}
			}
		}
		// includes all places reachable from the arcs list		
		for (uint k  = 0; k < _Places.size(); ++k)
		{					
			const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
			if (place != startPlace && place->getFlag(wantedFlag))
			{
				// if this place is reachable from current place arcs ...
				if (std::find(startPlace->getArcs().begin(), startPlace->getArcs().end(), place->getIndex()) != startPlace->getArcs().end())
				{
					// ... then it is a candidate.
					if (place->getActive())
					{
						activeCandidates.push_back(_Places[k]->getChildIndex());
					}
					else
					{
						candidates.push_back(_Places[k]->getChildIndex());
					}
				}
			}
		}
	}
	// active vertices are taken in priority
	// nlwarning("%d active place, %d unactive places", (int) activeCandidates.size(), (int) candidates.size());
	if (!activeCandidates.empty())
	{
		return (sint) activeCandidates[rand() % activeCandidates.size()];
	}	
	// if current place is valid then don't move
	if (startPlace && startPlace->getActive()) return CAIPlaceXYRFauna::INVALID_PLACE;
	// otherwise select a place in unactive places
	if (candidates.empty()) return CAIPlaceXYRFauna::INVALID_PLACE;
	return (sint) candidates[rand() % candidates.size()];
}

bool CGrpFauna::checkArcs(const CAIPlace &startPlace) const
{
	const CFaunaGenericPlace *startPlaceGeneric = getFaunaGenericPlace(&startPlace);
	// TODO nico : this function has a lot of similarities with CGrpFauna::getNextPlace
	// adding  a getArcs function would be nice
	sint minIndex = INT_MAX;
	sint firstIndex = INT_MAX;
	if (startPlaceGeneric->getReachNext())
	{
		for (uint k  = 0; k < _Places.size(); ++k)
		{	
			const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
			firstIndex = std::min(firstIndex, (sint) place->getIndex());
			if (place->getIndex() < minIndex && place->getIndex() > startPlaceGeneric->getIndex())
			{
				minIndex = place->getIndex();
			}
		}
		minIndex = std::max(minIndex, firstIndex);
		for (uint k  = 0; k < _Places.size(); ++k)
		{
			const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
			if ((sint) place->getIndex() == minIndex)
			{
				RYAI_MAP_CRUNCH::CCompatibleResult res;
				areCompatiblesWithoutStartRestriction(startPlace.worldValidPos(), _Places[k]->worldValidPos(), getAStarFlag(), res);
				if (!res.isValid()) return false; 
			}
		}
	}
	// includes all places reachable from the arcs list		
	for (uint k  = 0; k < _Places.size(); ++k)
	{					
		const CFaunaGenericPlace *place = getFaunaGenericPlace(_Places[k]);
		if (place != startPlaceGeneric)
		{			
			if (std::find(startPlaceGeneric->getArcs().begin(), startPlaceGeneric->getArcs().end(), place->getIndex()) != startPlaceGeneric->getArcs().end())
			{
				// this place is in current arc list
				RYAI_MAP_CRUNCH::CCompatibleResult res;
				areCompatiblesWithoutStartRestriction(startPlace.worldValidPos(), _Places[k]->worldValidPos(), getAStarFlag(), res);
				if (!res.isValid()) return false; 
			}
		}
	}
	return true;
}

void CSpawnGroupFauna::setPlace(int placeIndex)
{	
	const CFaunaGenericPlace *place = getFaunaGenericPlace(getPersistent().places()[placeIndex]);
	//nlwarning("Going to place %s with index %d", getPersistent().places()[placeIndex]->getName().c_str(), place->getIndex());

	if ((int) getPersistent().places().size() <= placeIndex)
	{
		nlwarning("Bad place index for fauna group %s", getPersistent().getName().c_str());
	}
	
	// const CFaunaGenericPlace *faunaPlace = getFaunaGenericPlace(getPersistent().places()[placeIndex]);
	// nlwarning("Group %s : Chosing place %s (%d) with graph index %d", getPersistent().getName().c_str(), faunaPlace->getName().c_str(), placeIndex, (int) faunaPlace->getIndex());

	_TargetPlace = getPersistent().places()[placeIndex];
#if !FINAL_VERSION
	const	RYAI_MAP_CRUNCH::TAStarFlag	flags=targetPlace()->worldValidPos().getTopologyRef().getCstTopologyNode().getFlags();
	nlassert((flags&getPersistent().getAStarFlag())==0);
#endif
	_PathCont.setDestination(targetPlace()->getVerticalPos(), targetPlace()->worldValidPos());
	_ArrivedInZone = false;
}

#include "event_reaction_include.h"
