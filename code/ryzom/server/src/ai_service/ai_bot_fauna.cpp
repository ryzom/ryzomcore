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
#include "ai_bot_fauna.h"
#include "ai_grp_fauna.h"
#include "ai_mgr_fauna.h"
#include "ai_player.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "ai_profile_fauna.h"

#include "dyn_grp_inline.h"

using namespace NLMISC;
using namespace std;
using namespace RYAI_MAP_CRUNCH;
using namespace	AITYPES;

/****************************************************************************/
/* File configuration                                                       */
/****************************************************************************/

// ---------------------------------------------------------------------------
// Debug defines
// ---------------------------------------------------------------------------
// COMPACT_POS_WARNINGS compress flooding warnings concerning path problems.
// Positions where the problems occures are stored and displayed and cleared
// every minute.
// :TODO: /!\ As it cannot be tested without long-time run with several
// players the following define can be commented to restore previous behavior.
#define COMPACT_POS_WARNINGS 1

// ---------------------------------------------------------------------------
// Some combat constants
static float const CONSIDER_MIN_DIST = 6.f;

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Stuff used for management of log messages
#ifdef NL_DEBUG
static bool VerboseLog = false;
#endif
#ifndef NL_DEBUG
static bool VerboseLog = false;
#endif

#define LOG if (!VerboseLog) {} else nlinfo

// ---------------------------------------------------------------------------
// Control over verbose nature of logging
NLMISC_COMMAND(verboseFaunaLog,"Turn on or off or check the state of verbose fauna activity logging","")
{
	if(args.size()>1)
		return false;
	
	if(args.size()==1)
		StrToBool	(VerboseLog, args[0]);
	
	log.displayNL("verboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}

//----------------------------------------------------------------------------
// Control over verbose nature of logging
NLMISC_COMMAND(verboseFaunaBot,"Turn on or off or check the state of verbose fauna bot","")
{
	if(args.size()>1)
		return false;
	
	if(args.size()==1)
		StrToBool	(VerboseLog, args[0]);
	
	nlinfo("VerboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}

/****************************************************************************/
/* Local classes definition and function declatations                       */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CBotProfileGoAway                                                   //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileGoAway
: public CAIBaseProfile
{
public:
	CBotProfileGoAway(CProfileOwner* owner, RYAI_MAP_CRUNCH::TAStarFlag denyFlags, float speed = 0.f, CAIFaunaActivityBaseSpawnProfile* lastProfile = NULL);
	
	virtual void beginProfile();
	
	virtual void endProfile() { }
	
	// Speed is in the range [0;1]
	void setSpeed(float speed) { _Speed = speed; }
	
	float speed () { return _Speed; }
	
	CAIVector& getDecalageRef() { return _Decalage; }
	
	CAIFaunaActivityBaseSpawnProfile* lastProfile()	const { return _LastProfile; }
	
	virtual void updateProfile(uint ticksSinceLastUpdate);
	
	virtual std::string getOneLineInfoString() const { return std::string("go_away profile"); }
	
	virtual	TProfiles getAIProfileType () const { return BOT_GO_AWAY; }
	
public:
	RYAI_MAP_CRUNCH::CDirection		_LastDir;
	RYAI_MAP_CRUNCH::CMapPosition	_LastStartPos;
	
	CPathPosition	_PathPos;
	CPathCont		_fightGoAwayPathContainer;
	
protected:
	CAIVector		_Decalage;
	CSpawnBot*		_Bot;
	float			_Speed;
	NLMISC::CSmartPtr<CAIFaunaActivityBaseSpawnProfile>	_LastProfile;
};

//////////////////////////////////////////////////////////////////////////////
// CBotProfileGoAway                                                        //
//////////////////////////////////////////////////////////////////////////////

class CBotProfileGoAwayFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner);
};

CBotProfileGoAwayFactory BotProfileGoAway;

//////////////////////////////////////////////////////////////////////////////
// Global functions                                                         //
//////////////////////////////////////////////////////////////////////////////

static const char *cyclesStateName(CFaunaActivity::TCycleState s);

/****************************************************************************/
/* Function definitions                                                     */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotFauna                                                           //
//////////////////////////////////////////////////////////////////////////////

CSpawnBotFauna::CSpawnBotFauna(TDataSetRow const& row, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CSpawnBot(row, owner, id, radius, level, denyFlags)
{
	setCycleState(CFaunaActivity::CycleStateUndefined);
	
	//	we start with a wander activity.
	setAIProfile(this, &WanderFaunaProfileFactory, false);
	
	_NextBestTargetUpdate.set(1);	//	next.
	_Hungry = 1.f;
}

CSpawnBotFauna::~CSpawnBotFauna	()
{
}

CBotFauna& CSpawnBotFauna::getPersistent() const
{
	return static_cast<CBotFauna&>(CSpawnBot::getPersistent());
}

bool CSpawnBotFauna::canMove()	const
{
	return /*!isRooted() && */walkSpeed() != 0 && getPersistent().faunaType()!=AITYPES::FaunaTypePlant;
}

float CSpawnBotFauna::getCollisionDist(float angTo) const
{
	// :TODO: Rehabilitate behaviour based on sheet (see this function history in CVS)
	return radius();
}

void CSpawnBotFauna::sendInfoToEGS()	const
{
	CFaunaBotDescription &fbd = CAIS::instance().getFaunaDescription();
	fbd.Bots.push_back(dataSetRow());
	fbd.GrpAlias.push_back(getPersistent().grp().getAlias());
}

void CSpawnBotFauna::update(TProfiles activity, uint32 ticksSinceLastUpdate)
{
	H_AUTO(CSpawnBotFauna_update);
	
	nlassert(!getAISpawnProfile().isNull());
	//	this piece of code change the current comportment of the fauna in regards of the group comportment.
	
	++AISStat::BotTotalUpdCtr;
	++AISStat::BotFaunaUpdCtr;
	
	TProfiles faunaActivity = getAIProfileType();
	if (faunaActivity==BOT_FLEE && !isFeared() && getUnreachableTarget().isNULL())
	{
		setDefaultComportment();
		faunaActivity=getAIProfileType();
	}
	
	{
		if (	spawnGrp().mustDespawnBots()
			&&	(	spawnGrp().despawnImmediately()
				||	(	faunaActivity!=BOT_FIGHT
					&&	faunaActivity!=BOT_FLEE
					)
				)
			)	// if its group is out of time.
		{
			spawnGrp().addBotToDespawnAndRespawnTime(&getPersistent(), 1, 2);	//	.. set the bot to be despawn and gives the control to its group.
			return;
		}
		
		if (	faunaActivity!=ACTIVITY_CORPSE
			&&	(spawnGrp().getUpdatePriority()<(2<<2))	)	// 2*40 -> 80 meters
		{
			if (	faunaActivity!=BOT_FIGHT
				||	!NLMISC::safe_cast<CBotProfileFight*>(getAIProfile())->isHitting())
			{

				if (	_NextBestTargetUpdate.test()
					||	faunaActivity==BOT_GO_AWAY)
				{
					getBestTarget	();
					faunaActivity=getAIProfileType();	//	activity could have changed.
					_NextBestTargetUpdate.set(15);		// getbesttarget every 1.5 secs ..
				}
			}
		}
		
		if (	faunaActivity==ACTIVITY_GRAZING
			||	faunaActivity==ACTIVITY_RESTING
			||	faunaActivity==ACTIVITY_WANDERING	)
		{			
			if	(faunaActivity!=activity)
			{
				CAIFaunaActivityBaseSpawnProfile	*profile=NLMISC::safe_cast<CAIFaunaActivityBaseSpawnProfile*>(getAIProfile());
				if	(!(profile->getMovementMagnet().isNull()))
				{
					if	(profile->getMovementMagnet()->getMovementType()==CMovementMagnet::Movement_Move)
					{
						switch(activity)
						{
						case ACTIVITY_RESTING:
							setAIProfile(this,&RestFaunaProfileFactory, false);
							break;
						case ACTIVITY_GRAZING:
							setAIProfile(this,&GrazeFaunaProfileFactory, false);
							break;
						case ACTIVITY_WANDERING:
							setAIProfile(this,&WanderFaunaProfileFactory, false);
							break;
						case ACTIVITY_PLANTIDLE:
							setAIProfile(this,&PlanteIdleFaunaProfileFactory, false);
							break;
						default:					
							nlwarning("Unsupported activity for fauna bot");
							break;
						}
					}
				}
			}
		}
	}
	
	//	the behaviour update.
	if (!isStuned())
	{
		updateProfile(ticksSinceLastUpdate);
		
		// if normal activity .. update Aggro.
		if (	faunaActivity!=BOT_FIGHT
			&&	faunaActivity!=BOT_FLEE	)
		{
			this->CBotAggroOwner::update(ticksSinceLastUpdate);
		}
	}
}

float CSpawnBotFauna::getReturnDistCheck() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroReturnDistCheck()>=0.f)
		return getPersistent().getSheet()->AggroReturnDistCheck();
	else
		return AggroReturnDistCheckFauna;
}

void CSpawnBotFauna::eventEngaged(TDataSetRow const& originId)
{
}

void CSpawnBotFauna::processEvent(CCombatInterface::CEvent const& event)
{
	// no self aggro.
	if (event._targetRow==event._originatorRow)
		return;
	
	//	To remove when debug done ..
	CAIEntityPhysical *ep = CAIS::instance().getEntityPhysical(event._originatorRow);
	if (ep==NULL || ep->getAIInstance()->getChildIndex()!=getAIInstance()->getChildIndex())
	{
		nlwarning("AIInstance Problem !!");
		return;
	}
	
	if ((event._nature==ACTNATURE::FIGHT || event._nature==ACTNATURE::OFFENSIVE_MAGIC) && !getPersistent().ignoreOffensiveActions())
	{
		float aggro = event._weight;
		if (aggro>-0.15f)
		{
			aggro = -0.15f;
		}
		if (event._nature==ACTNATURE::OFFENSIVE_MAGIC)
		{
			aggro = (float)((1.f+aggro)*0.5f-1.f);	//	maximize aggro for magic
			//insure if aggressor is player, player have it's target seted for BOSS assist
			CBotPlayer	*player=dynamic_cast<CBotPlayer*>(ep);
			if(player)
			{
				CAIEntityPhysical	*target=player->getVisualTarget();
				if (target)
					player->setTarget(target);
			}
		}
		addAggroFor(event._originatorRow, aggro, true);
	}
}

void CSpawnBotFauna::getBestTarget()
{
	// Get ourself
	CBotFauna& thisBotFauna = getPersistent();
	// Get our group
	CGrpFauna& grp = thisBotFauna.grp();
	// Get our type
	TFaunaType faunaType = grp.getType();
	// Compute an aggro radius
	float const AggroRadius = faunaType==FaunaTypeHerbivore?30.f:aggroRadius();
	
	// used if the bot is too far from its group center to use its vision. ???
	CAIVision<CPersistentOfPhysical>	vision;
	CAIVision<CPersistentOfPhysical>::iterator itVision, itVisionEnd;
	
	//	VISUAL_LOOK_AT_DIST
	//	ASSistDist
	
	float const VISUAL_LOOK_AT_DIST = 10.f;
	
	// Compute a test radius
	float testradius = AggroRadius; // is a minimum for other bot consideration.
	if	(testradius<VISUAL_LOOK_AT_DIST)
		testradius=VISUAL_LOOK_AT_DIST;
	if	(testradius<thisBotFauna.getSheet()->AssistDist())
		testradius=thisBotFauna.getSheet()->AssistDist();
	testradius+=radius();
	
	// Get the vision
	vision.updateBotsAndPlayers(getAIInstance(), CAIVector(pos()), (uint32)(testradius+3.f), (uint32)(testradius+3.f));
	itVision = vision.begin();
	itVisionEnd = vision.end();
	
	// Vars for the most interesting player
	float				bestCuriosityScore = (radius()+1.5f)*2000.f;
	CAIEntityPhysical*	curiosityPlayer = NULL;
	
	// Vars for the most appealing target
	float				bestVisualScore = 0.f;
	CAIEntityPhysical*	visualTarget = NULL;
	
	if (!getVisualTarget().isNULL())
	{
		if (	getTarget().isNULL() // check if we are looking to a too far visual target.
			&&	(	_VisualTargetTimer.test()
				||	getVisualTarget()->pos().quickDistTo(pos())>VISUAL_LOOK_AT_DIST	)		// if the target is too far ..
			)
		{
			_VisualTargetTimer.set(CAIS::rand32(40)+25);
			bestVisualScore=1.f;
			setVisualTarget(NULL);
		}
		
		if (!_VisualTargetTimer.test())
		{
			bestVisualScore=1.f;
		}
		
	}
	
	// (yet) unknown vars
	float bestScore = 0.f;
	bool goAway = false;
	TProfiles thisProfileType = getAIProfile()->getAIProfileType();
	uint32 const thisFaunaGroupIndex = thisBotFauna.getSheet()->GroupPropertiesIndex();
	sint32 const thisHeight = wpos().getMetricHeight	();
	
	// For each entity in the visual range
	for	( ; itVision!=itVisionEnd; ++itVision)
	{
		// The other entity
		CAIEntityPhysical* const entity = itVision->getSpawnObj();
		
		// If the other entity is not real or if it is us, skip it
		if (entity==NULL || entity==this)
			continue;
		
		// We compute the vector to the other entity
		CAIVector delta = pos();
		delta -= entity->pos();
		// We compute the dist to it
		double const dist = delta.quickNorm();
		
		// Check that the entity is in the test radius
		if	(dist <= (testradius+entity->radius()))
		{
			//	inf 10 m .. this second test was only put here for test .. need more tuning before generalization.
			bool const tooHigh = abs(entity->wpos().getMetricHeight()-thisHeight)>10000;
			if	(tooHigh)
				continue;
			
			// Depending on the type of the entity
			switch (entity->getRyzomType())
			{
//////////////////////////////////////////////////////////////////////////////
// The other is a player
			case RYZOMID::player:
				{
					// Depending what we are...
					switch(faunaType)
					{
					// ...but in fact whathever we are
					case FaunaTypePlant:
					case FaunaTypeHerbivore:
					case FaunaTypePredator:
						{
							// Get the root cell of the player (for safe zone stuff)
							CRootCell const* const rootCell = entity->wpos().getRootCell();
							// If player is a valid target and if we dont like him
							if	(	entity->isAlive()
								&&	dist<AggroRadius
								&&	thisBotFauna.getSheet()->getPropertiesCst(AISHEETS::CSheets::getInstance()->playerGroupIndex()).attack()
								&&	rootCell
								&&	rootCell->getFlag()==0)	//	not in safe zone.
							{
								// Set some aggro
								setAggroMinimumFor(entity->dataSetRow(), 0.8f*0.5f, false);
							}
							// If we don't have to aggro, and we're not a plant
							else if (faunaType!=FaunaTypePlant)
							{
								// Cast to player
								CBotPlayer* player = NLMISC::type_cast<CBotPlayer*>(entity);
							//	CBotPlayer* player = NLMISC::safe_cast<CBotPlayer*>(entity);
								
								// If player is aggressive and close enough
								if (player->isAggressive() && dist<std::max(CONSIDER_MIN_DIST, thisBotFauna.getSheet()->AssistDist()))
								{
									// If we are in an interruptible state
									if (	thisProfileType==ACTIVITY_GRAZING
										||	thisProfileType==ACTIVITY_RESTING
										||	thisProfileType==ACTIVITY_WANDERING)
									{
										// Get our profile
										IMouvementMagnetOwner* magnetOwner = dynamic_cast<IMouvementMagnetOwner*>(getAIProfile());
										if (magnetOwner)
										{
											// Get our movement magnet
											CMovementMagnet* movementMagnet = magnetOwner->getMovementMagnet();
											// If we have one and we are not moving finish it
											if	(movementMagnet!=NULL && movementMagnet->getMovementType()!=CMovementMagnet::Movement_Move)
												movementMagnet->stateTimer().set(0);
										}
									}
									// Get out of the switch, next tick we'll aggro him
									break;
								}
								
								// Check for curiosity
								if (	canMove()
									&&	!player->isAggressive()
									&&	entity->wpos().isValid()
									&&	(entity->wpos().getFlags()&entity->getAStarFlag())==0)
								{
									// Suppose we can go to him
									bool canChange = true;
									// If we are doing something not very interesting
									if	(	thisProfileType==ACTIVITY_GRAZING
										||	thisProfileType==ACTIVITY_RESTING
										||	thisProfileType==ACTIVITY_WANDERING)
									{
										// We change only...
										canChange = false;
										IMouvementMagnetOwner* magnetOwner = dynamic_cast<IMouvementMagnetOwner*>(getAIProfile());
										if	(magnetOwner && !(magnetOwner->getMovementMagnet().isNull()))
										{
											// ...if we are just moving
											canChange = magnetOwner->getMovementMagnet()->getMovementType()==CMovementMagnet::Movement_Move;
										}
									}
									
									if	(canChange)
									{
										// Compute a curiosity score, taking into account the time and the dist
										float const targetterNumber = (float)entity->totalTargeterCount();
										float const targetterScore = 1 + targetterNumber*targetterNumber*targetterNumber;
										float const time = (float)_TimeBeforeNextCuriosity.timeRemaining();
										float const curiosityScore = targetterScore * time * (float)dist;
										// If it's the most interesting player
										if	(curiosityScore<bestCuriosityScore)
										{
											// Set it as such
											bestCuriosityScore=curiosityScore;
											curiosityPlayer=entity;
										}
									}
								}
								
								// If we are already looking at something, or the entity is too far
								if	(	(bestVisualScore==1.f)
									||	(dist>=(VISUAL_LOOK_AT_DIST-1.f)) )
									// Skip it
									break;
								
								// If we have no visual target
								if	(	!((CAIEntityPhysical*)getVisualTarget())
									&&	!((CAIEntityPhysical*)getTarget()))
								{
									float const score = (float)(1.f/(1.f+dist+CAIS::rand32(7)));
									if (score>bestVisualScore)
									{
										bestVisualScore=score;
										visualTarget = entity;
									}
								}
							}
						}
						break;
					default:
						break;
					}
				}
				break;
//////////////////////////////////////////////////////////////////////////////
// The other is a npc
			case RYZOMID::npc:
				{
					// Depending on what we are
					switch(faunaType)
					{
					// If we're a plant
					case FaunaTypePlant:
						// Ignore the npc
						break;
					// If we're an herbivore
					case FaunaTypeHerbivore:
						{
							// If we already have a visual target or the entity is too far
							if	(	bestVisualScore==1.f
								||	dist>(VISUAL_LOOK_AT_DIST-1.f)
								||	(	((CAIEntityPhysical*)getVisualTarget())
									||	((CAIEntityPhysical*)getTarget()))	)
								// Skip it
								break;
							
							// If we are doing something not interesting
							if	(	thisProfileType==ACTIVITY_GRAZING
								||	thisProfileType==ACTIVITY_RESTING
								||	thisProfileType==ACTIVITY_WANDERING)
							{
								// We look at the npc if he is appealing
								float const score = (float)(1.f/(1.f+dist+CAIS::rand32(7)));
								if	(score>bestVisualScore)
								{
									bestVisualScore=score;
									visualTarget=entity;
								}
							}
						}
						break;
					// If we are a predator
					case FaunaTypePredator:
						{
							// Get the entity as a npc bot
							CSpawnBotNpc* botNpc = NLMISC::safe_cast<CSpawnBotNpc*>(entity);
							// If it is an escorted entity
							if (botNpc->spawnGrp().activityProfile().getAIProfileType() == ACTIVITY_ESCORTED)
							{
								// Aggro it
								setAggroMinimumFor(entity->dataSetRow(), 0.8f, false);
							}
						}
						break;
					default:
						break;
					}
				}
				break;
//////////////////////////////////////////////////////////////////////////////
// The other is an animal
// :TODO: Finish the doc of that method
			case RYZOMID::creature:
			case RYZOMID::pack_animal:
				{
					CSpawnBot	*botCreat=NLMISC::safe_cast<CSpawnBot*>(entity);
					CGroup	&creatGrp=botCreat->getPersistent().getGroup();
					const	uint32	otherCreatGrpIndex=botCreat->getPersistent().getSheet()->GroupPropertiesIndex();
					const	AISHEETS::CGroupProperties	&groupProp=thisBotFauna.getSheet()->getPropertiesCst(otherCreatGrpIndex);
					
				//////////////////////////////////////////////////////////////
				// Assist it ?
					
					//	if creature is fighting
					if (botCreat->hasBeenHit(20))	//	20 ticks (2 seconds) persistent test ..
					{
						//	if nearest than assist dist ..
						//	and same group
						//		or assist compatibility.
						if	(	dist<thisBotFauna.getSheet()->AssistDist()
							&&	(	&creatGrp==&grp
								||	groupProp.assist())
							)
						{
							const	CAIEntityPhysical	*const	target=botCreat->getTarget();
							if (	target
								&&	target->getRyzomType()==RYZOMID::player)
							{
								setAggroMinimumFor(target->dataSetRow(), 0.2f, false);
							}
							
							// check attackers and give minimum aggro.
							CAIEntityPhysical const* attacker = botCreat->firstTargeter();
							while (attacker!=NULL)
							{
								if (attacker->getRyzomType()==RYZOMID::player)
								{
									setAggroMinimumFor(attacker->dataSetRow(), 0.2f, false);
								}
								attacker = attacker->nextTargeter();
							}
						}
					}
					
				//////////////////////////////////////////////////////////////
				// Attack it ?
					if (groupProp.attack())
					{
						if	(	canMove	()
							&&	hungry	()>0	)
						{
							if	(entity->isAlive())
							{
								if (!((CAIEntityPhysical*)getTarget()))
								{
									if (runSpeed()>entity->runSpeed())
									{
										//	got enought life ? (more than 75%).
										if	((4*currentHitPoints())>(3*maxHitPoints()))
										{
											// check if the herbivore is in the current place 
											const	CAIPlace	*place=spawnGrp().targetPlace();
											float	alpha((float)place->midPos().quickDistTo(CAIVector(entity->wpos())));
											alpha	-=	place->getRadius();
											if (alpha<0)
												alpha=0;
											alpha=(float)(1/(1+alpha*0.1));
											setAggroMinimumFor(entity->dataSetRow(), 0.8f*alpha*0.5f, false);
										}
									}
								}
							}
							else
							{
								if	(entity->food()>0)
								{
									if	(	(	(	thisProfileType!=BOT_FIGHT
												&&	thisProfileType!=BOT_FLEE	)
											||	(((CAIEntityPhysical*)getTarget())==NULL || ((CAIEntityPhysical*)getTarget())==entity)	)
										&&	thisProfileType!=BOT_GO_AWAY
										&&	thisProfileType!=ACTIVITY_EAT_CORPSE	)
									{
										CSpawnBot	*botCreat=NLMISC::safe_cast<CSpawnBot*>(entity);
										// if its a corpse, change our behaviour to eat.
										IAIProfile* profile = botCreat->getAIProfile();
										if (	profile
											&&	profile->getAIProfileType()==ACTIVITY_CORPSE
											&&	botCreat->wpos().isValid()
											&&	!(botCreat->wpos().getFlags()&botCreat->getAStarFlag())
											)
										{
											CCorpseFaunaProfile	*corpseProfile=NLMISC::safe_cast<CCorpseFaunaProfile*>(profile);
											if	(	!corpseProfile->haveEater()
												&&	!corpseProfile->eated()	)
											{	// start a eater comportment.
												corpseProfile->setEater(true);
												setAIProfile(new CEatCorpseFaunaProfile(this,entity->dataSetRow(), getAStarFlag()));
											}
										}
									}
								}
							}
						}
					}
					else
					{
				//////////////////////////////////////////////////////////////
				// Flee from it ?
						const	AISHEETS::CGroupProperties	&groupProp=botCreat->getPersistent().getSheet()->getPropertiesCst(thisFaunaGroupIndex);
						
						//	the other creature may attack us .. :O
						if (groupProp.attack())
						{
							if	(	canMove()
								&&	entity->isAlive()
								&&	runSpeed()<entity->runSpeed())
							{
								TProfiles	creatActivity=getAIProfileType();
								if (dist<(testradius*0.5f))
								{
									if	(	creatActivity==ACTIVITY_GRAZING
										||	creatActivity==ACTIVITY_RESTING
										||	creatActivity==ACTIVITY_WANDERING	)
									{
										CAIFaunaActivityBaseSpawnProfile	*botProfile=NLMISC::safe_cast<CAIFaunaActivityBaseSpawnProfile*>(getAIProfile());
										setAIProfile(new CBotProfileGoAway(this,getAStarFlag(), 1.f, botProfile));
										creatActivity=BOT_GO_AWAY;
									}
									else
									{
										if (creatActivity==ACTIVITY_CURIOSITY)
										{
											setAIProfile(new CBotProfileGoAway(this,getAStarFlag(), 1.f, NULL));
											creatActivity=BOT_GO_AWAY;
										}
									}
									
								}
								
								if	(creatActivity==BOT_GO_AWAY)
								{
									CAIVector	dir=delta;
									const	float	qNorm=(float)dir.quickNorm();
									dir.normalize(1000.f/(qNorm+1.f));
									CBotProfileGoAway	*const	profile=NLMISC::safe_cast<CBotProfileGoAway*>(getAIProfile());
									profile->getDecalageRef()+=dir;
									float	speed=100.f/(qNorm*qNorm+1.f);
									if	(speed>1.f)
										speed=1.f;
									profile->setSpeed(speed);
									goAway=true;
								}
							}
						}
					}
					
					if	(	bestVisualScore==1.f
						||	dist>(VISUAL_LOOK_AT_DIST-1.f))
						break;
					
					//	if no visual target.								
					if	(!((CAIEntityPhysical*)getVisualTarget()) && !((CAIEntityPhysical*)getTarget()))
					{
						if	(	thisProfileType==ACTIVITY_GRAZING
							||	thisProfileType==ACTIVITY_RESTING
							||	thisProfileType==ACTIVITY_WANDERING	)
						{
							const	float	score=(float)(1.f/(1.f+dist+CAIS::rand32(7)));
							if (score>bestVisualScore)
							{
								bestVisualScore=score;
								visualTarget=entity;
							}
						}
					}
				}
				break;
			default:
				break;
			}
		} // if entity is in test radius
	} // for each entity in visual range
	
	TProfiles	faunaActivity=getAIProfileType();
	if	(faunaActivity==BOT_GO_AWAY)
	{
		CBotProfileGoAway	*profile=NLMISC::safe_cast<CBotProfileGoAway*>(getAIProfile());
		if (!goAway)	//profile->decalage().isNull())
		{
			CAIFaunaActivityBaseSpawnProfile* lastProfile = profile->lastProfile();
			if (canMove() && lastProfile)
			{
				IMouvementMagnetOwner* magnetOwner = NLMISC::safe_cast<IMouvementMagnetOwner*>(lastProfile);
				CMovementMagnet* movementMagnet = magnetOwner->getMovementMagnet();
				if (movementMagnet)
				{
					movementMagnet->setState(CMovementMagnet::Movement_Wait_Anim);
					movementMagnet->stateTimer().set(CAIS::rand32(51)+50);	// wait between 5 and 10 seconds.
				}
				setAIProfile(lastProfile, CProfilePtr::START_RESUME);
			}
			else
			{
				setDefaultComportment();
			}
		}
	}
	
	thisProfileType=getAIProfile()->getAIProfileType();
	
	if	(curiosityPlayer)
	{
		if	(	thisProfileType==ACTIVITY_WANDERING
			||	thisProfileType==ACTIVITY_GRAZING
			||	thisProfileType==ACTIVITY_RESTING	)
		{
			setAIProfile(new CCuriosityFaunaProfile(this,curiosityPlayer->dataSetRow(), getAStarFlag()));
		}
	}
	else
	{
		if (_TimeBeforeNextCuriosity.test())
		{
			_TimeBeforeNextCuriosity.set((CAIS::rand32(120)+120)*10);	//	consider every 2 to 4 minutes;
		}
	}

	if (visualTarget)
	{
		thisProfileType=getAIProfile()->getAIProfileType();
		if	(	thisProfileType==ACTIVITY_GRAZING
			||	thisProfileType==ACTIVITY_RESTING
			||	thisProfileType==ACTIVITY_WANDERING	)
		{
			setVisualTarget(visualTarget);
			_VisualTargetTimer.set(CAIS::rand32(60)+10);
		}
	}
}


float CSpawnBotFauna::aggroRadius()
{
	switch	(cycleState())
	{
	case CFaunaActivity::CycleStateVeryHungry:
	case CFaunaActivity::CycleStateStarving:
		if (spawnGrp().getPersistent().places()[CGrpFauna::EAT_PLACE]->atPlace(CAIVector(pos())))
			return getPersistent().getSheet()->AggroRadiusHunting();
		else
			return getPersistent().getSheet()->AggroRadiusHungry();
	default:
		return getPersistent().getSheet()->AggroRadiusNotHungry();
	}
}

//////////////////////////////////////////////////////////////////////////////
// CMovementMagnet                                                          //
//////////////////////////////////////////////////////////////////////////////

CMovementMagnet::CMovementMagnet(CSpawnBotFauna& botFauna, RYAI_MAP_CRUNCH::TAStarFlag flag)
: _BotFauna(botFauna)
, _PathPos(botFauna.theta())
, _PathCont(flag)
, _denyFlags(flag)
{
	//	start by moving during a variable time, just to disperse bot in their place.(must be initialised by constructor)
	_StateTimer.set(0);	//	do not wait a long time .. :)
	_State = Movement_Wait_Anim;
	_PathPos._Angle = _BotFauna.theta();
	_Speed = _BotFauna.walkSpeed();
}

CMovementMagnet::~CMovementMagnet()
{
}

void CMovementMagnet::setBotAngle()
{
	_PathPos._Angle = _BotFauna.theta();
}

CAIVector const& CMovementMagnet::getDestination() const
{
#ifdef NL_DEBUG
	nlassert(isDestinationValid	());
#endif
	return _PathCont.getDestination();
}

bool CMovementMagnet::isDestinationValid() const
{
	return _PathCont.getDestPos().isValid();
}

void CMovementMagnet::getNewDestination(RYAI_MAP_CRUNCH::CWorldPosition const& alternativePos, RYAI_MAP_CRUNCH::TAStarFlag denyFlag)
{
	if	(CAIS::frand()>_BotFauna.getPersistent().getSheet()->GroupDispersion())	//	to make some variety.
	{
		// first, try to take the same way as another bot of the group with the same comportment.		
		CCont<CBot >	&bots	=	_BotFauna.spawnGrp().bots();
		uint32	nbBots=(uint32)bots.size();

		float		bestScore=0.f;
		CAIVector	bestDest;
		
		for (uint32 i=0;i<nbBots;i++)
		{
			CBotFauna	*bot=NLMISC::safe_cast<CBotFauna*>(bots[i]);
			if	(!bot)
				continue;

			CSpawnBotFauna	*faunaBot=bot->getSpawn();
			if	(	!faunaBot
				||	faunaBot==&_BotFauna
				||	faunaBot->getAIProfileType()!=_BotFauna.getAIProfileType()	)
				continue;

			IMouvementMagnetOwner* magnetOwner = dynamic_cast<IMouvementMagnetOwner*>(faunaBot->getAIProfile());
			if	(!magnetOwner)
				continue;

			const	CMovementMagnet	*const	movementMagnet = magnetOwner->getMovementMagnet();
			if	(	!movementMagnet
				||	!movementMagnet->isDestinationValid())
				continue;

			const	CAIVector	&destPos=movementMagnet->getDestination	();
			if	(	destPos==_LastDest
				||	destPos==_PathCont.getDestination())
				continue;

			if	(	!faunaBot->wpos().isValid()
				||	(faunaBot->wpos().getFlags()&denyFlag)!=0)
				continue;

			// can be optimize by in avoid inversion.
			const	float	distToBot=(float)(1.f/(faunaBot->pos().quickDistTo(_BotFauna.pos())+1.f));
			if (distToBot<bestScore)
				continue;

			bestScore = distToBot;
			bestDest = destPos;
		}

		if (bestScore>0)
		{
			_LastDest=_PathCont.getDestination();
			_PathCont.setDestination(vp_auto, bestDest);
			return;
		}
	}
	
	// if failed, then try to take an random destination.
	
	// here, we have to find another place to go (!)
	uint32 nbTries = 64;
	CWorldPosition newPos;
	float bestScore = 0;
	
	CSpawnGroupFauna const& grpFauna = _BotFauna.spawnGrp();
	CAIPlace const* const place = grpFauna.targetPlace();
	
	do
	{
		--nbTries;
		
		CWorldPosition wRndPos;
		_BotFauna.spawnGrp().targetPlace()->getRandomPos(wRndPos);
		
		//	check if its a nogo and water proof position.
		if (	!wRndPos.isValid()
			||	(wRndPos.getTopologyRef().getCstTopologyNode().getFlags()&denyFlag)!=0	)
			continue;
		
	#if !FINAL_VERSION
		nlassertex(wRndPos.isValid(), ("Error: can't find a valid pos in place '%s'", _BotFauna.spawnGrp().targetPlace()->getAliasFullName().c_str()));
	#else
		if (!wRndPos.isValid())
			nlwarning("Error: can't find a valid pos in place '%s'", _BotFauna.spawnGrp().targetPlace()->getAliasFullName().c_str());
	#endif
		
		CAIVector const newPosVector = CAIVector(wRndPos);
		
		double const distToGrp = newPosVector.quickDistTo(grpFauna.getCenterPos());
		double distToBot = newPosVector.quickDistTo(_BotFauna.pos());
		
		// if too near, then make this score not too good, else add some value to minimize the effect of bot dist / group dist.
		distToBot += (distToBot<=1.0)?30.0:4.0;
		
		float const score = (float)(distToGrp/distToBot);
		
		if (score<bestScore)
			continue;
		
		bestScore = score;
		newPos = wRndPos;
	} while (nbTries>0);
	
	if (!newPos.isValid())
	{
		_PathCont.setDestination(vp_auto, alternativePos);	//	the alternative pos given by parameters.
		nlwarning("Error: can't find a valid pos in place '%s' near %s", _BotFauna.spawnGrp().targetPlace()->getAliasFullName().c_str(), _BotFauna.spawnGrp().targetPlace()->worldValidPos().toString().c_str());
	}
	else
		_PathCont.setDestination(vp_auto, newPos);
}

void CMovementMagnet::update(uint32 waitTime, uint32 ticksSinceLastUpdate, bool ignoreBadPos)
{
	H_AUTO(MovementMagnet);
	
	switch (_State)
	{
		BeginAnim:
			_State=Movement_Anim;
			_StateTimer.set((uint32)((waitTime*0.5)+CAIS::rand16(waitTime)));
			// drop through to Wait

		case Movement_Anim:
			// this is a small hack to allow migration code to avoid turning
			if (waitTime==0)
				goto BeginMove;
			// this is the basic wait code - it waits!
			if (_StateTimer.test())
				goto BeginWaitAnim;
			break;

		BeginWaitAnim:
			// select a random angle in range +/-pi, biased towards 0
			// and setup dTheta and dThetaTimer to avoid turning again at start of movement
			_StateTimer.set(100);	//secs (wait for the anim to stop)
			_State=Movement_Wait_Anim;

		case Movement_Wait_Anim:
			if (_StateTimer.test())
				goto BeginMove;
			break;

		BeginMove:
			_State=Movement_Move;
			getNewDestination	(_BotFauna.wpos(), _denyFlags);	// drop through to Move
			_Speed=_BotFauna.walkSpeed();

		case Movement_Move:
		{
			if (!_BotFauna.canMove())
				break;

			float	distToDest=(float)_PathCont.getDestination().quickDistTo(_BotFauna.pos());
			distToDest-=((_BotFauna.getPersistent().getChildIndex()&7)+1.5f);

			float		dist=_Speed*ticksSinceLastUpdate;
			CAIVector	lastPos=_BotFauna.pos();
			{
				CAIVector	deviateVector(CAngle(_BotFauna.theta().asRadians()+(Pi*0.5f)*sin(CTimeInterface::gameCycle()*0.03f+_BotFauna.getPersistent().getChildIndex())).asVector2d());
				_BotFauna.setMoveDecalage(_BotFauna.moveDecalage()+deviateVector);
			}
			CFollowPath::TFollowStatus status = CFollowPath::getInstance()->followPath(
					&_BotFauna,
					_PathPos,
					_PathCont,
					dist,
					dist*.5f,
					.5f);
			
			if (!ignoreBadPos)
			{
				if	(status==CFollowPath::FOLLOW_NO_PATH)	//	No Path Found !
					getNewDestination	(_BotFauna.wpos(), _denyFlags);	// drop through to Move
				
				if (distToDest<=0 || lastPos.quickDistTo(_BotFauna.pos())<(dist*0.5f))	//	too much people.
				{
					goto BeginAnim;
				}
			}
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CReturnMovementMagnet                                                    //
//////////////////////////////////////////////////////////////////////////////

CReturnMovementMagnet::CReturnMovementMagnet(RYAI_MAP_CRUNCH::CWorldPosition const& forcedDest, CSpawnBotFauna& botFauna, RYAI_MAP_CRUNCH::TAStarFlag flag)
: CMovementMagnet(botFauna, flag)
, _ForcedDest(forcedDest)
{
	/*
	RYAI_MAP_CRUNCH::CWorldPosition	wpos;
	if (CWorldContainer::getWorldMap().setWorldPosition(forcedDest.h(), wpos, forcedDest.toAIVector()))
	{
		setWPos(wpos);
	}
	else
	{
		nlerror("Impossible to create a valid world pos for return magnet.")
	}
	*/
//	sint32 z = forcedDest.h();
//	CAIVector const vect = forcedDest.toAIVector();
//	bool res = CWorldContainer::getWorldMap().setWorldPosition(z, _ForcedDest, vect);
//	nlassert(res);
}

void CReturnMovementMagnet::getNewDestination(RYAI_MAP_CRUNCH::CWorldPosition const& alternativePos, RYAI_MAP_CRUNCH::TAStarFlag denyFlag)
{
	if (_ForcedDest.isValid() && (_ForcedDest.getTopologyRef().getCstTopologyNode().getFlags()&denyFlag)==0)
		_PathCont.setDestination(_ForcedDest);
	else
		CMovementMagnet::getNewDestination(alternativePos, denyFlag);
}

//////////////////////////////////////////////////////////////////////////////
// CBotFauna                                                                //
//////////////////////////////////////////////////////////////////////////////

CAIS::CCounter& CBotFauna::getSpawnCounter()
{
	return CAIS::instance()._FaunaBotCounter;
}

CBotFauna::CBotFauna(AITYPES::TFaunaType type, CGroup* owner, CAIAliasDescriptionNode* alias)
: CBot(owner, alias)
, _Type(type)
, _Sheet(NULL)
{
	_Sheet = CBotFaunaSheetPtr(new CBotFaunaSheet(NULL));
	_Sheet->setSheet(CBot::getSheet());
}

CBotFauna::~CBotFauna()
{
	if (isSpawned())
	{
		despawnBot();
	}
}

CSpawnBot* CBotFauna::getSpawnBot(TDataSetRow const& row, NLMISC::CEntityId const& id, float radius)
{
	return new CSpawnBotFauna(row, *this, id, radius, getSheet()->Level(), getGroup().getAStarFlag());
}

bool CBotFauna::reSpawn(bool sendMessage)
{
	// we made some tries because its an random positionned spawn that may failed some times ..
	uint32 maxtries = 100;
	while (!spawn() && maxtries>0)
	{
		--maxtries;
	}
	
	if (!isSpawned() && sendMessage)
	{
		LOG("Cannot spawn a fauna bot %s", getFullName().c_str());
	}
	return isSpawned();
}

// :KLUDGE: This methods is a part of the trick for bot respawn
// :TODO: Clean that mess
bool CBotFauna::finalizeSpawnFauna()
{
	getSpawn()->sendInfoToEGS();
	
	// execute birth script :)
	AISHEETS::ICreature::TScriptCompList const& scriptList = getSheet()->BirthScriptList();
	FOREACHC(it, AISHEETS::ICreature::TScriptCompList, scriptList)
		(*it)->update(*getSpawn());
	return true;
}

//	nothing special is made, its a simple bot spawn.
bool CBotFauna::spawn()
{
	// initialise the energy value.
	initEnergy(NLMISC::safe_cast<CGrpFauna*>(getOwner())->getEnergyCoef());
	
	if (!CBot::spawn())
		return false;
	
	// :KLUDGE: Last part calls a tricky method also called by sheetChanged
	// :TODO: Clean that mess
	return finalizeSpawnFauna();
}

void CBotFauna::despawnBot()
{
	CBot::despawnBot();
}

CGrpFauna& CBotFauna::grp() const
{
	return *static_cast<CGrpFauna*>(getOwner());
}

CMgrFauna& CBotFauna::mgr() const
{
	return *static_cast<CMgrFauna*>(grp().getOwner());
}

void CBotFauna::getSpawnPos(CAIVector& triedPos, CWorldPosition& pos, CWorldMap const& worldMap, CAngle& spawnTheta)
{
	nlassert(grp().getSpawnObj());
	CSpawnGroupFauna* grpFauna = grp().getSpawnObj();
	CBotFauna* leader = grpFauna->leader();
	
	spawnTheta = CAngle(CAIS::frand(2*NLMISC::Pi));
	
	//	if possible, fauna must spawn near its leader.
	if (leader && leader->isSpawned() && getSheet()->FaunaType()!=FaunaTypePlant)
	{
		RYAI_MAP_CRUNCH::CWorldPosition	leaderPos=leader->getSpawn()->wpos();
		//	if the leader is not in the current place ..
		if (!grpFauna->targetPlace()->atPlace(leaderPos))
		{	//	spawn the fauna near the leader (same place for instance).
			pos = leaderPos;
		}
	}
	
	if (!pos.isValid())
	{
		//	if we try to spawn a plant, try to find a position where no plant already are.
		if (getSheet()->FaunaType()==FaunaTypePlant)
		{
			bool rejected = true;
			uint32 nbTry = 64;
			while (nbTry-- > 0 && rejected)
			{
				grpFauna->targetPlace()->getRandomPos(pos);
				
				//	Check if this place is Valid for us ..
				CAIVision<CPersistentOfPhysical> vision;
				vision.updateBotsAndPlayers(getAIInstance(), pos,0,2);
				std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > const& bots = vision.bots();
				
				rejected = false;
				FOREACHC(it, std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> >, bots)
				{
					CAIEntityPhysical const* const phys = (*it)->getSpawnObj();
					if (!phys || phys->getRyzomType()!=RYZOMID::creature)
						continue;
					
					CSpawnBotFauna const* const fauna = NLMISC::safe_cast<const CSpawnBotFauna*>(phys);
					if (	fauna!=NULL
						&&	fauna->getPersistent().getSheet()->FaunaType()==FaunaTypePlant
						&&	fauna->pos().quickDistTo(pos.toAIVector())<2)
					{
						rejected = true;
						break;
					}
				}
			};
		#if !FINAL_VERSION
			if (rejected)
			{
				nlwarning ("Solo Plant Pos Spawn Not Found at: %s", pos.toString().c_str());
			}			
		#endif
		}
		else
		{
			uint32 tries = 100;	//	think we won't be so expensive in the average case.
			while (tries-- > 0)
			{
				RYAI_MAP_CRUNCH::CCompatibleResult res;
				grpFauna->targetPlace()->getRandomPos(pos);
				areCompatiblesWithoutStartRestriction(pos, grpFauna->targetPlace()->worldValidPos(), getGroup().getAStarFlag(), res);
				if (res.isValid()) // Cool!
					break;
			}
		}
	}
	triedPos = pos.toAIVector();
}

std::string	CBotFauna::getOneLineInfoString() const 
{ 
	return std::string("Fauna bot '") + getName() + "'" + "(AliasName : "+getAliasFullName()+")"; 
}


void CBotFauna::sheetChanged()
{
	if (getSpawnObj())
	{
		// Get bot state
		RYAI_MAP_CRUNCH::CWorldPosition botWPos = getSpawnObj()->wpos();
		CAngle spawnTheta = getSpawnObj()->theta();
		float botMeterSize = getSheet()->Scale()*getSheet()->Radius();
		// :TODO: Save profile info
		
		// If stuck bot position may be outside collision and must be recomputed
		if (isStuck() || IsRingShard)
			getSpawnPos(lastTriedPos, botWPos, CWorldContainer::getWorldMap(), spawnTheta);
		
		// Delete old bot
		CMirrors::removeEntity(getSpawnObj()->getEntityId());
		setSpawn(NULL); // automatic smart pointer deletion
		notifyBotDespawn();
		
		// Finalize spawn object creation
		if (!finalizeSpawn(botWPos, spawnTheta, botMeterSize))
			return;
		
		// :KLUDGE: Both finalizeSpawn and finalizeSpawnFauna are called,
		// sheetChanged has a strange herited meaning and may confuse future
		// coders
		// :TODO: Clean that mess and find a more elegant C++ solution to the
		// problem
		finalizeSpawnFauna();
	}
}

void CBotFauna::triggerSetSheet(AISHEETS::ICreatureCPtr const& sheet)
{
	// :KLUDGE: This test is there to mimick the behaviour of the client. This
	// is not the better way to do this, but the client is so... well, you see
	// what I mean.
	if (EGSPD::CPeople::Creature<=sheet->Race() && sheet->Race()<EGSPD::CPeople::EndCreature)
		CBot::triggerSetSheet(sheet);
	else
		nlwarning("Trying to set a NPC sheet to a creature, sheet change canceled.");
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileGoAway                                                        //
//////////////////////////////////////////////////////////////////////////////

CBotProfileGoAway::CBotProfileGoAway(CProfileOwner* owner, RYAI_MAP_CRUNCH::TAStarFlag denyFlags, float speed, CAIFaunaActivityBaseSpawnProfile* lastProfile) 
: CAIBaseProfile()
, _Bot(NLMISC::safe_cast<CSpawnBot*>(owner))
, _PathPos(NLMISC::safe_cast<CSpawnBot*>(owner)->theta())
, _Speed(speed)
, _LastProfile(lastProfile)
, _fightGoAwayPathContainer(denyFlags)
{
}

void CBotProfileGoAway::beginProfile()
{		
	_LastDir = RYAI_MAP_CRUNCH::CDirection(RYAI_MAP_CRUNCH::CDirection::UNDEFINED);
	_fightGoAwayPathContainer.setDestination(vp_auto, CAIVector(_Bot->pos()));
	_Bot->setMode(MBEHAV::NORMAL);
}

void CBotProfileGoAway::updateProfile(uint ticksSinceLastUpdate)
{
	H_AUTO(BotGoAwayProfileUpdate)
	CFollowPathContext fpcBotGoAwayProfileUpdate("BotGoAwayProfileUpdate");

	if (!_Bot->canMove())
		return;
	
	bool	calcDone=true;
	
	if (_Decalage.isNull())
		_Decalage.setX(1+_Decalage.x());	// hum ..
	RYAI_MAP_CRUNCH::CDirection	startDir(_Decalage.x(), _Decalage.y(), true);
	
	// if we need to change our destination.
	if	(	startDir!=_LastDir
		||	_fightGoAwayPathContainer.getDestPos(/*_Bot->size()*/).hasSameFullCellId(_Bot->wpos())	)
	{
		float							BestScore=-1.f;
		RYAI_MAP_CRUNCH::CWorldPosition	BestPos;
		
		const	RYAI_MAP_CRUNCH::CWorldMap	&worldMap=CWorldContainer::getWorldMap(/*_Bot->size()*/);
		calcDone=false;
		
		sint nbStep=0;
		
		while	(nbStep<8)
		{
			// try to find a direction around startDir.
			RYAI_MAP_CRUNCH::CDirection	dir(startDir);
			dir.addStep((RYAI_MAP_CRUNCH::CDirection::TDeltaDirection)	((nbStep&1)?(nbStep>>1):(-(nbStep>>1))));
			
			const	RYAI_MAP_CRUNCH::CRootCell	*rootCell=worldMap.getRootCellCst(_Bot->wpos().stepCell(dir.dx(),dir.dy()));	//	_Bot->wpos()
			if	(rootCell)
			{
				for (uint32	pointIndex=0;pointIndex<4;pointIndex++)
				{
					const	RYAI_MAP_CRUNCH::CWorldPosition &wpos=rootCell->getWorldPosition(pointIndex);
					
					if	(wpos.isValid())
					{
						CCompatibleResult	res;
						areCompatiblesWithoutStartRestriction(_Bot->wpos(), wpos, _fightGoAwayPathContainer.getDenyFlags(), res);
						if	(!res.isValid())
						{
						//	nlwarning("Error case avoided. Please report this warning to jvuarand.");
							continue;
						}
						
						CAIVector	deltaToDest(wpos-_Bot->wpos());
						const	float	score=(float) CAIVector(wpos-_Bot->wpos()).dot(_Decalage);	//*deltaToDest.norm();
						if	(	score>BestScore
							&&	deltaToDest.quickNorm()>2.f) // minimum distance requires.
						{
							BestScore=score;
							BestPos=wpos;
						}
					}
				}					
			}
			nbStep++;
		}
		
		RYAI_MAP_CRUNCH::CCompatibleResult	res;
		if (_Bot->wpos().isValid() && BestPos.isValid())
		{
			areCompatiblesWithoutStartRestriction(_Bot->wpos(), BestPos, _fightGoAwayPathContainer.denyFlags(), res, true);
			if (BestPos.isValid() && res.isValid())
			{
				_LastDir=startDir;
				_LastStartPos=_Bot->wpos();
				calcDone=true;
				_fightGoAwayPathContainer.setDestination(vp_auto, BestPos);
			}
		}
	}
	
	//	if we found somewhere to go, then go there ..
	if (calcDone)
	{
		const	float	dist=	((1.f-_Speed)*_Bot->walkSpeed()+_Speed*_Bot->runSpeed())*ticksSinceLastUpdate;
		
		_Decalage.normalize(100.f*1000.f);
		_Decalage += CAIVector(_Bot->pos());
		CFollowPath::TFollowStatus const status = CFollowPath::getInstance()->followPath(
				_Bot,
				_PathPos,
				_fightGoAwayPathContainer,
				dist,
				dist*.71f,
				.5f,
				true,
				&_Decalage);
		if (status==CFollowPath::FOLLOW_NO_PATH)
		{
	#if !FINAL_VERSION
			nlwarning("GoAway followpath problem (!!)");
	#endif
		}
	}
	else
	{
		_LastDir=RYAI_MAP_CRUNCH::CDirection(RYAI_MAP_CRUNCH::CDirection::UNDEFINED);
	}
	_Decalage=CAIVector(0,0);
}

//////////////////////////////////////////////////////////////////////////////
// CBotProfileGoAway                                                        //
//////////////////////////////////////////////////////////////////////////////

NLMISC::CSmartPtr<IAIProfile> CBotProfileGoAwayFactory::createAIProfile(CProfileOwner* owner)
{
	nlassert(false);
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Global functions                                                         //
//////////////////////////////////////////////////////////////////////////////

static const char *cyclesStateName(CFaunaActivity::TCycleState s)
{
	switch (s)
	{
	case CFaunaActivity::CycleStateHungry:		return "HUNGRY";
	case CFaunaActivity::CycleStateVeryHungry:	return "VERY_HUNGRY";
	case CFaunaActivity::CycleStateStarving:	return "STARVING";
	case CFaunaActivity::CycleStateDigesting:	return "DIGESTING";
	case CFaunaActivity::CycleStateTired:		return "TIRED";
	case CFaunaActivity::CycleStateVeryTired:	return "VERY_TIRED";
	case CFaunaActivity::CycleStateExhausted:	return "EXHAUSTED";
	case CFaunaActivity::CycleStateShaking:		return "SHAKING";
	default:
		break;
	}
	return "UNKNOWN STATE";
}

