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

/*

  Ring XP rules:

  While playing in the Ring, players receive Ring Reward Points (RRPs) instead of XP
  The model for gaining ring bonus points works as follows:

  - Time is divided into notional "Time Slices"
  
  - A Time Slice start when a player performs a "Qualifying Action" 
  
  - A Time Slice ends when a sufficient number of "Qalifying Actions" have been performed AND a 
    minimum time has passed since the time slice started

  - A new Time Slice can not start until any previous Time Slice has ended

  - At the end of each Time Slice the player gains a fixed number of RRPs

  - When the player hits a target RRP level they gain a fixed RRP BONUS (eg, at 20k RRPs there
    is a 5K bonus). The logic behind this bonus is that it insights players to make scenarios
	of roughly 30 minutes to 1 hour duration for progression purposes
  
  - RRPs are classed in 6 families by level ( <=20, <=50, <=100, <=150, <=200, <=250 )

  - The class of RRPs gained in a scenario depend on the level of the scenario. The definition
    of a 'Qualifying Action' also depends on the level of the scenario.


  Ring Reward Generators:

  - A special component called a 'Reward Generator' can be dropped into ring scenarios.

  - When a player who has a threshold level of RRPs clicks on a 'Reward Generator', a random 'loot'
    is generated and given to them and the given number of RRPs are consumed. The loot is not
	generated if the player inventory is full.

  - The random loot is composed of 'no drop' consulmables. To start with there will only be Ring XPCats.

*/


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdpch.h"

// NeL MISC
#include "nel/misc/variable.h"

// Local
#include "player_manager/cdb.h"
#include "player_manager/cdb_branch.h"
#include "character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "ring_reward_points.h"
#include "modules/animation_session_manager.h"
#include "mission_manager/mission_event.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// NLMISC::CVariables
//-------------------------------------------------------------------------------------------------

// global enable / disable flag for the rrp system
CVariable<bool>		RingRPEnabled				("ringReward", "RingRPEnabled",					"Flag to enable or dissable ring reward point system", true,0,true);
CVariable<uint32>	RingRPMode					("ringReward", "RingRPMode",					"Mode 1: fisrst system implemented, Mode 2: second system implemented", 2,0,true);
CVariable<bool>		VerboseRingRPLog			("ringReward", "VerboseRingRPLog",				"Flag to enable or dissable verbose logging in the ring reward point system", true,0,true);

// time slice management
CVariable<uint32>	RingRPXPRequiredPerAction	("ringReward", "RingRPXPRequiredPerAction",		"Minimum number of XP required for an action to be taken into consideration", 700,0,true);
CVariable<uint32>	RingRPXPRequiredPerTimeSlice("ringReward", "RingRPXPRequiredPerTimeSlice",	"Minimum number of XP required during a time slice in order to gain a ring point", 700,0,true);
CVariable<float>	RingRPMinTimeSliceDuration	("ringReward", "RingRPMinTimeSliceDuration",	"Minimum time between bonus point gain in the Ring (in seconds)", 5*60.0,0,true);
CVariable<uint32>	RingRPPointsPerTimeSlice	("ringReward", "RingRPPointsPerTimeSlice",		"Number of Ring Reward Points gained per time slice", 50,0,true);

// 'minimum number of active player' management
CVariable<uint32>	RingRPMinActivePlayers		("ringReward", "RingRPMinActivePlayers",		"Minimum number of active players required for xp to be gained", 2,0,true);
CVariable<float>	RingRPActivityDuration		("ringReward", "RingRPActivityDuration",		"Duration that player is considered to be 'active' for after validating a time slice", 10*60.0,0,true);

// the super bonus applied once per session
CVariable<uint32>	RingRPSuperBonusRequirement	("ringReward", "RingRPSuperBonusRequirement",	"Score required in a ring session in order to gain a super bonus", 100,0,true);
CVariable<uint32>	RingRPSuperBonusValue		("ringReward", "RingRPSuperBonusValue",			"Value of the super bonus", 100,0,true);

// capping the number of RRP points earned per character in a scenario
CVariable<uint32>	RingRPLimitPerSession		("ringReward", "RingRPLimitPerSession",			"Maximum number of points that can be earned by a single player in a single scenario", 1000,0,true);
CVariable<uint32>	RingRPMaxGainsPerSession	("ringReward", "RingRPMaxGainsPerSession",		"Maximum number of times that points can be earned by a single player in a single scenario", 20,0,true);

// reward generation
CVariable<uint32>	RingRPPointsPerReward		("ringReward", "RingRPPointsPerReward",			"Number of Ring Reward Points required for generation of a reward", 100,0,true);
CVariable<string>	RingRPRewardItemSheet		("ringReward", "RingRPRewardItemSheet",			"Item sheet used for ring reward items that are generated", "ixpca02.sitem",0,true);
CVariable<uint32>	RingRPRewardItemQuantity	("ringReward", "RingRPRewardItemQuantity",		"Quantity of ring reward items to generate per iteration", 200,0,true);


//-------------------------------------------------------------------------------------------------
// Handy debug macros
//-------------------------------------------------------------------------------------------------

#define LOG if (VerboseRingRPLog==false) {} else nldebug


//-------------------------------------------------------------------------------------------------
// class CRingRewardPointsImplTimerEvent
//-------------------------------------------------------------------------------------------------

class CRingRewardPointsImplTimerEvent: public CTimerEvent
{
public:
	CRingRewardPointsImplTimerEvent(CRingRewardPointsImpl* parent);
	void timerCallback(CTimer* owner);
private:
	CRingRewardPointsImpl* _Parent;
};


//-------------------------------------------------------------------------------------------------
// class CRingRewardPointsImpl
//-------------------------------------------------------------------------------------------------

class CRingRewardPointsImpl
{
public:
	// default ctor
	CRingRewardPointsImpl(CCharacter* theCharacter);

	// dtor
	~CRingRewardPointsImpl();

	// Methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	// implementations of methods defined in CRingRewardPoints class
	void initDb();
	void setScenarioLevel(R2::TSessionLevel scenarioLevel);
	void addXp(R2::TSessionLevel xpLevel, uint32 xpValue);
	CRingRewardPoints::TGenerateRewardResult generateReward(R2::TSessionLevel rewardLevel);

	// method called when the time slice timer hits 0 and again each time XP is gained after 
	// time slice hits 0 until time slice is validated
	// this method validates the time slice and performs all necessary related tasks
	void tryToValidateTimeSlice();


private:
	// The character that we belong to
	CCharacter* _ParentCharacter;

	// the level of the scenario that we are currently playing in
	R2::TSessionLevel _ScenarioLevel;

	// time at which last validated time slice began
	NLMISC::TGameCycle _LastTimeSliceStartTime;

	// timer used to allocate points at the end of the time slice
	CTimer _PointAllocatorTimer;

	// the number of ring points accumulated for each level
	uint32 _CumulatedRRP[R2::TSessionLevel::nb_enum_items];

	// the number of ring points accumulated for each level
	uint32 _HistoricRRP[R2::TSessionLevel::nb_enum_items];

	// database reminders
//	ICDBStructNode*	_DB_RRPSLevel_Reminder[R2::TSessionLevel::nb_enum_items];

};


//-------------------------------------------------------------------------------------------------
// methods CRingRewardPoints
//-------------------------------------------------------------------------------------------------

CRingRewardPoints::CRingRewardPoints(CCharacter* theCharacter)
{
	_Implementation= new CRingRewardPointsImpl(theCharacter);
}

CRingRewardPoints::~CRingRewardPoints()
{
	delete _Implementation;
}

void CRingRewardPoints::initDb()
{
	_Implementation->initDb();
}

void CRingRewardPoints::setScenarioLevel(R2::TSessionLevel scenarioLevel)
{
	// if the ring reward system is disabled then drop out
	if (!RingRPEnabled) return;

	_Implementation->setScenarioLevel( scenarioLevel );
}

void CRingRewardPoints::store(CPersistentDataRecord &pdr) const
{
	_Implementation->store( pdr );
}

void CRingRewardPoints::apply(CPersistentDataRecord &pdr)
{
	_Implementation->apply( pdr );
}

void CRingRewardPoints::addXp( R2::TSessionLevel xpLevel, uint32 xpValue )
{
	// if the ring reward system is dissabled then drop out
	if (!RingRPEnabled) return;

	_Implementation->addXp( xpLevel, xpValue );
}

CRingRewardPoints::TGenerateRewardResult CRingRewardPoints::generateReward( R2::TSessionLevel rewardLevel )
{
	// if the ring reward system is dissabled then drop out
	if (!RingRPEnabled) return CRingRewardPoints::grr_invalid;

	return _Implementation->generateReward( rewardLevel );
}


//-------------------------------------------------------------------------------------------------
// methods CRingRewardPointsImplTimerEvent
//-------------------------------------------------------------------------------------------------

CRingRewardPointsImplTimerEvent::CRingRewardPointsImplTimerEvent(CRingRewardPointsImpl* parent)
	: _Parent(parent)
{
}

void CRingRewardPointsImplTimerEvent::timerCallback(CTimer* owner)
{
	_Parent->tryToValidateTimeSlice();
}


//-------------------------------------------------------------------------------------------------
// methods CRingRewardPointsImpl
//-------------------------------------------------------------------------------------------------

CRingRewardPointsImpl::CRingRewardPointsImpl(CCharacter* theCharacter): _ParentCharacter(theCharacter)
{
	LOG("RRP ctor");
	_ScenarioLevel					= R2::TSessionLevel::invalid_val;
	_ParentCharacter				= theCharacter;
	_LastTimeSliceStartTime			= (NLMISC::TGameCycle)0;
	for (uint32 i=0; i<R2::TSessionLevel::nb_enum_items; ++i)
	{
		_CumulatedRRP[i]= 0;
		_HistoricRRP[i]= 0;
//		_DB_RRPSLevel_Reminder[i] = NULL;
	}
}

CRingRewardPointsImpl::~CRingRewardPointsImpl()
{
	LOG("RRP dtor");
}


void CRingRewardPointsImpl::initDb()
{
	LOG("RRP initDb: %s",_ParentCharacter->getId().toString().c_str());

	CCDBStructNodeBranch *rootNode = CCDBStructBanks::instance()->getStructRoot( CDBPlayer );
	nlassert( rootNode );

//	nlverify(_DB_RRPSLevel_Reminder[0] = rootNode->getICDBStructNodeFromNameFromRoot( "USER:RRPS_LEVEL_20"));
//	nlverify(_DB_RRPSLevel_Reminder[1] = rootNode->getICDBStructNodeFromNameFromRoot( "USER:RRPS_LEVEL_50"));
//	nlverify(_DB_RRPSLevel_Reminder[2] = rootNode->getICDBStructNodeFromNameFromRoot( "USER:RRPS_LEVEL_100"));
//	nlverify(_DB_RRPSLevel_Reminder[3] = rootNode->getICDBStructNodeFromNameFromRoot( "USER:RRPS_LEVEL_150"));
//	nlverify(_DB_RRPSLevel_Reminder[4] = rootNode->getICDBStructNodeFromNameFromRoot( "USER:RRPS_LEVEL_200"));
//	nlverify(_DB_RRPSLevel_Reminder[5] = rootNode->getICDBStructNodeFromNameFromRoot( "USER:RRPS_LEVEL_250"));

	for (uint i=0; i<R2::TSessionLevel::nb_enum_items; ++i)
//		_ParentCharacter->_PropertyDatabase.setProp(_DB_RRPSLevel_Reminder[i], _CumulatedRRP[i]);
		CBankAccessor_PLR::getUSER().getRRPS_LEVELS(i).setVALUE(_ParentCharacter->_PropertyDatabase, _CumulatedRRP[i]);
}

void CRingRewardPointsImpl::setScenarioLevel(R2::TSessionLevel scenarioLevel)
{
	LOG("RRP setScenarioLevel(%s): %s",scenarioLevel.toString().c_str(),_ParentCharacter->getId().toString().c_str());

	BOMB_IF(!scenarioLevel.isValid(),"Attempt to set an invalid value for scenarioLevel: clamping to INVALID_LEVEL for character "<<_ParentCharacter->getId().toString(),scenarioLevel= R2::TSessionLevel::invalid_val);

	_ScenarioLevel= scenarioLevel;
	_LastTimeSliceStartTime= (NLMISC::TGameCycle)0;
}

void CRingRewardPointsImpl::tryToValidateTimeSlice()
{
	LOG("RRP tryToValidateTimeSlice(): %s",_ParentCharacter->getId().toString().c_str());

	// give up if the scenario level hasn't been set (must be an edit / test session)
	if (!_ScenarioLevel.isValid())
	{
		BOMB("Invalid scenario level for character: "<<_ParentCharacter->getId().toString(),return);
	}

	// get hold of the session info record
	const TAnimSessionInfo* sessionInfo= IAnimSessionMgr::getInstance()->getAnimSessionForChar(uint32(_ParentCharacter->getId().getShortId()));
	DROP_IF(sessionInfo==NULL,"Failed to add XP for character "<<_ParentCharacter->getId().toString()<<" because session not found",return);

	// get a refference to the participant info for our parent character
	uint32 parentCharId= uint32(_ParentCharacter->getId().getShortId());
	TAnimSessionInfo::TParticipantInfos::const_iterator parentParticipantInfoIt= sessionInfo->ParticipantInfos.find(parentCharId);
	BOMB_IF(parentParticipantInfoIt==sessionInfo->ParticipantInfos.end(),"RRP tryToValidateTimeSlice(): Failed to find character in their session: "<<_ParentCharacter->getId().toString(),return);

	// count the number of active players in the session
	NLMISC::TGameCycle cutoffTime= CTickEventHandler::getGameCycle()- (NLMISC::TGameCycle)(10.0*(float)RingRPActivityDuration);
	uint32 numActivePlayers= 0;
	TAnimSessionInfo::TParticipantInfos::const_iterator it=    sessionInfo->ParticipantInfos.begin();
	TAnimSessionInfo::TParticipantInfos::const_iterator itEnd= sessionInfo->ParticipantInfos.end();
	for (; it!=itEnd; ++it)
	{
		// only count players that have been active recently
		// note that as long as RingRPActivityDuration > RingRPMinTimeSliceDuration, the 'parent' character will necessarily be in the list
		if (it->second.LastActivityTime>= cutoffTime)
			++numActivePlayers;
	}

	if (RingRPMode==1)
	{
		// if there are insufficient active players then drop out
		if (numActivePlayers<(uint32)RingRPMinActivePlayers)
		{
			// display a failure message
			// send a "There are too few active players for you to gain RRP points" message to client
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_TOO_FEW_ACTIVE_PLAYERS");
			LOG("RRP tryToValidateTimeSlice(): %s FAILED because only %u other active characters found in the scenario (%u required)",_ParentCharacter->getId().toString().c_str(),numActivePlayers,(uint32)RingRPMinActivePlayers);
			return;
		}
		LOG("RRP tryToValidateTimeSlice(): %s with %u other active characters in the scenario",_ParentCharacter->getId().toString().c_str(),numActivePlayers);

		// if we're here it means that the time slice is validated...
		uint32 pointsGainedThisSession= parentParticipantInfoIt->second.PointsGainedThisSession;

		// setup the number of points to be gained
		uint32 pointsGainedThisTime= RingRPPointsPerTimeSlice;

		// check to make sure we haven't reached the max points per scenario cap
		if (pointsGainedThisSession >=(uint32)RingRPLimitPerSession)
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_SESSION_LIMIT_REACHED");
			return;
		}

		// display a congratulations message
		// send a "You have gained %d ring reward points making %d this session and %d in all" message to client
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Int = pointsGainedThisTime;
		params[1].Int = _ScenarioLevel.asIndex();
		params[2].Int = _CumulatedRRP[_ScenarioLevel.asIndex()]+pointsGainedThisTime;
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GAIN", params);

		// see if we qualify for the super bonus
		if ( (pointsGainedThisSession+pointsGainedThisTime>= RingRPSuperBonusRequirement) && (pointsGainedThisSession< RingRPSuperBonusRequirement) )
		{
			// apply the bonus
			pointsGainedThisTime+= RingRPSuperBonusValue;
			LOG("RRP tryToValidateTimeSlice(): %s: BONUS: level %s, bonus %u",	_ParentCharacter->getId().toString().c_str(), _ScenarioLevel.toString().c_str(), (uint32)RingRPSuperBonusValue );

			// display a congratulations message
			// send a "You have gained a ring reward point BONUS of %d making %d this session and %d in all" message to client
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = RingRPSuperBonusValue;
			params[1].Int = _ScenarioLevel.asIndex();
			params[2].Int = _CumulatedRRP[_ScenarioLevel.asIndex()]+pointsGainedThisTime;
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_BONUS", params);		
		}

		// gain the points
		pointsGainedThisSession+= pointsGainedThisTime;
		_CumulatedRRP[_ScenarioLevel.asIndex()]+= pointsGainedThisTime;
		_HistoricRRP[_ScenarioLevel.asIndex()]+= pointsGainedThisTime;
//		_ParentCharacter->_PropertyDatabase.setProp(_DB_RRPSLevel_Reminder[_ScenarioLevel.asIndex()], _CumulatedRRP[_ScenarioLevel.asIndex()]);
		CBankAccessor_PLR::getUSER().getRRPS_LEVELS(_ScenarioLevel.asIndex()).setVALUE(_ParentCharacter->_PropertyDatabase, _CumulatedRRP[_ScenarioLevel.asIndex()]);

		LOG("RRP tryToValidateTimeSlice(): %s: point gain: level %s, gain %u, session ttl %u, cumulated ttl %u, historic ttl %u",_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str(),
			pointsGainedThisTime,pointsGainedThisSession,_CumulatedRRP[_ScenarioLevel.asIndex()],_HistoricRRP[_ScenarioLevel.asIndex()]);

		// add this points in the anim session
		IAnimSessionMgr::getInstance()->addRRPForChar(uint32(_ParentCharacter->getId().getShortId()), pointsGainedThisTime);
	}

	if (RingRPMode==2)
	{
		// make sure the number of active players in the session is valid
		BOMB_IF(numActivePlayers<1, "Clamping number of active players to 1 for character: "<<_ParentCharacter->getId().toString(),numActivePlayers=1);
		LOG("RRP tryToValidateTimeSlice(): %s with %u other active characters in the scenario",_ParentCharacter->getId().toString().c_str(),numActivePlayers);

		// setup the number of points to be gained
		uint32 pointsGainedThisTime=
			(numActivePlayers==1)?	25:
			(numActivePlayers==2)?	50:
			(numActivePlayers==3)?	65:
			(numActivePlayers==4)?	75:
			/* default */			80;

		// lookup the level of the best action used by the character
		R2::TSessionLevel bestActionLevel= (R2::TSessionLevel::TValues)parentParticipantInfoIt->second.BestActionLevel;
		BOMB_IF(!bestActionLevel.isValid(),"No RRP gain because best action  level is invalid for character: "<<_ParentCharacter->getId().toString(),return);

		// if we're here it means that the time slice is validated...
		uint32 pointsGainedThisSession= parentParticipantInfoIt->second.PointsGainedThisSession;

		// lookup the number of times that the character has gained RRP this session
		uint32 numRRPGains= parentParticipantInfoIt->second.NumRRPGains;

		// check to make sure we haven't reached the max point gains per scenario cap
		if (numRRPGains >=(uint32)RingRPMaxGainsPerSession)
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_SESSION_LIMIT_REACHED");
			return;
		}

		// apply a factor to the number of points gained in a given time slice
		pointsGainedThisTime=
			(numRRPGains==0)?	(uint32)(pointsGainedThisTime * 0.75):
			(numRRPGains==1)?	(uint32)(pointsGainedThisTime * 1.0):
			(numRRPGains==2)?	(uint32)(pointsGainedThisTime * 1.5):
			(numRRPGains==3)?	(uint32)(pointsGainedThisTime * 2.0):
			(numRRPGains==4)?	(uint32)(pointsGainedThisTime * 1.5):
			(numRRPGains==5)?	(uint32)(pointsGainedThisTime * 1.25):
			/* default */		pointsGainedThisTime;

		// display a congratulations message
		// send a "You have gained %d ring reward points of type %d with %d active players for %d seconds of active time
		SM_STATIC_PARAMS_4(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Int = pointsGainedThisTime;
		params[1].Int = bestActionLevel.asIndex();
		params[2].Int = numActivePlayers;
		params[3].Int = (numRRPGains+1)*(uint32)RingRPMinTimeSliceDuration;
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GAIN2", params);

		// gain the points
		pointsGainedThisSession+= pointsGainedThisTime;
		_CumulatedRRP[bestActionLevel.asIndex()]+= pointsGainedThisTime;
		_HistoricRRP[bestActionLevel.asIndex()]+= pointsGainedThisTime;
//		_ParentCharacter->_PropertyDatabase.setProp(_DB_RRPSLevel_Reminder[bestActionLevel.asIndex()], _CumulatedRRP[bestActionLevel.asIndex()]);
		CBankAccessor_PLR::getUSER().getRRPS_LEVELS(bestActionLevel.asIndex()).setVALUE(_ParentCharacter->_PropertyDatabase, _CumulatedRRP[bestActionLevel.asIndex()]);

		LOG("RRP tryToValidateTimeSlice(): %s: point gain: level %s, gain %u, session ttl %u, cumulated ttl %u, historic ttl %u",_ParentCharacter->getId().toString().c_str(),bestActionLevel.toString().c_str(),
			pointsGainedThisTime,pointsGainedThisSession,_CumulatedRRP[bestActionLevel.asIndex()],_HistoricRRP[bestActionLevel.asIndex()]);

		// add this points in the anim session
		IAnimSessionMgr::getInstance()->addRRPForChar(uint32(_ParentCharacter->getId().getShortId()), pointsGainedThisTime);
	}
}

void CRingRewardPointsImpl::addXp( R2::TSessionLevel xpLevel, uint32 xpValue )
{
	// give up if the scenario level hasn't been set (must be an edit / test session)
	if (!_ScenarioLevel.isValid())
		return;

	// make sure the xp type is valid
	BOMB_IF(!xpLevel.isValid(),"Ignoring attempt to add invalid xp type to character "<<_ParentCharacter->getId().toString(),return);

	// if this is not a qualifying action due to xp equivalence then ignore it
	if (xpValue<RingRPXPRequiredPerAction)
	{
		// display a non-progress message
		// send a "You gain nothing from this action because it is too easy for you (equivalent to %d xp)" message to client
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = xpValue;
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_BAD_ACTION", params);
		LOG("RRP addXp(%s,%u): %s: BAD ACTION for scenario level %s",xpLevel.toString().c_str(), xpValue,_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str());
		return;
	}

	if (RingRPMode==1)
	{
		// if this is not a qualifying action due to the scenario level then ignore it
		if (_ScenarioLevel>xpLevel)
		{
			// display a non-progress message
			// send a "You gain nothing from this action because it is below %s level" message to client
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = _ScenarioLevel.asIndex();
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_BAD_SKILL", params);
			LOG("RRP addXp(%s,%u): %s: BAD ACTION for scenario level %s",xpLevel.toString().c_str(), xpValue,_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str());
			return;
		}

		// flag the player as 'active' in the animation session manager
		IAnimSessionMgr::getInstance()->flagCharAsActive(uint32(_ParentCharacter->getId().getShortId()),xpLevel.getValue());

		// calculate time now and time since last time slice started
		NLMISC::TGameCycle timeNow= CTickEventHandler::getGameCycle();
		NLMISC::TGameCycle deltaTime= timeNow- _LastTimeSliceStartTime;

		// setup a value in ticks for min time slice duration
		NLMISC::TGameCycle minTimeSliceDuration= (NLMISC::TGameCycle)(RingRPMinTimeSliceDuration*10);

		// if we're still in the last time slice then give up
		if (deltaTime < minTimeSliceDuration)
		{
			LOG("RRP addXp(%s,%u): %s: for scenario level %s: DELTA TIME TOO SHORT: timeNow %u, _LastTimeSliceStartTime %u, deltaTime %u, minTimeSliceDuration %u",xpLevel.toString().c_str(), xpValue,_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str(),
				(uint32)timeNow, (uint32) _LastTimeSliceStartTime, (uint32)deltaTime, (uint32)minTimeSliceDuration );
			return;
		}

		// If we're here then we're validating the time slice...
		LOG("RRP addXp(%s,%u): %s: for scenario level %s: VALIDATE TIME SLICE:  timeNow %u, _LastTimeSliceStartTime %u, deltaTime %u, minTimeSliceDuration %u",xpLevel.toString().c_str(), xpValue,_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str(),
				(uint32)timeNow, (uint32) _LastTimeSliceStartTime, (uint32)deltaTime, (uint32)minTimeSliceDuration );

		// setup a new value for the _LastTimeSliceStartTime
		_LastTimeSliceStartTime =
			(deltaTime < (3*minTimeSliceDuration)/2)?	_LastTimeSliceStartTime+ minTimeSliceDuration:
			(deltaTime < 2*minTimeSliceDuration)?		_LastTimeSliceStartTime+ (timeNow- _LastTimeSliceStartTime- minTimeSliceDuration)*2/ minTimeSliceDuration:
			/* default */								timeNow;
		
		// setup the 'point allocator' timer
		_PointAllocatorTimer.set(_LastTimeSliceStartTime+minTimeSliceDuration,new CRingRewardPointsImplTimerEvent(this));
	}

	else if (RingRPMode==2)
	{
		// flag the player as 'active' in the animation session manager
		IAnimSessionMgr::getInstance()->flagCharAsActive(uint32(_ParentCharacter->getId().getShortId()),xpLevel.getValue());

		// calculate time now and time since last time slice started
		NLMISC::TGameCycle timeNow= CTickEventHandler::getGameCycle();
		NLMISC::TGameCycle deltaTime= timeNow- _LastTimeSliceStartTime;

		// setup a value in ticks for min time slice duration
		NLMISC::TGameCycle minTimeSliceDuration= (NLMISC::TGameCycle)(RingRPMinTimeSliceDuration*10);

		// if we're still in the last time slice then give up
		if (deltaTime < minTimeSliceDuration)
		{
			LOG("RRP addXp(%s,%u): %s: for scenario level %s: DELTA TIME TOO SHORT: timeNow %u, _LastTimeSliceStartTime %u, deltaTime %u, minTimeSliceDuration %u",xpLevel.toString().c_str(), xpValue,_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str(),
				(uint32)timeNow, (uint32) _LastTimeSliceStartTime, (uint32)deltaTime, (uint32)minTimeSliceDuration );
			return;
		}

		// If we're here then we're validating the time slice...
		LOG("RRP addXp(%s,%u): %s: for scenario level %s: VALIDATE TIME SLICE:  timeNow %u, _LastTimeSliceStartTime %u, deltaTime %u, minTimeSliceDuration %u",xpLevel.toString().c_str(), xpValue,_ParentCharacter->getId().toString().c_str(),_ScenarioLevel.toString().c_str(),
				(uint32)timeNow, (uint32) _LastTimeSliceStartTime, (uint32)deltaTime, (uint32)minTimeSliceDuration );

		// setup a new value for the _LastTimeSliceStartTime
		_LastTimeSliceStartTime =
			(deltaTime < (3*minTimeSliceDuration)/2)?	_LastTimeSliceStartTime+ minTimeSliceDuration:
			(deltaTime < 2*minTimeSliceDuration)?		_LastTimeSliceStartTime+ (timeNow- _LastTimeSliceStartTime- minTimeSliceDuration)*2/ minTimeSliceDuration:
			/* default */								timeNow;
		
		// setup the 'point allocator' timer
		_PointAllocatorTimer.set(_LastTimeSliceStartTime+minTimeSliceDuration,new CRingRewardPointsImplTimerEvent(this));
	}
}

CRingRewardPoints::TGenerateRewardResult CRingRewardPointsImpl::generateReward(R2::TSessionLevel rewardLevel)
{
	// give up if the scenario level hasn't been set (must be an edit / test session)
	if (!_ScenarioLevel.isValid())
	{
		BOMB("Invalid scenario level for character: "<<_ParentCharacter->getId().toString(),return CRingRewardPoints::grr_invalid);
	}

	// in any case, generate a mission event
//#pragma message (NL_LOC_WRN "give the correct scenario tag")
	// TODO: give the correct scenario tag
	CMissionEventTaggedRingScenarioDone event("blabla");
	_ParentCharacter->processMissionEvent(event);
	
//#pragma message (NL_LOC_WRN "Implements 'rare' item reward generation")
	// TODO: Implements 'rare' item reward generation
	// make sure the reward level is valid
	BOMB_IF(rewardLevel>R2::TSessionLevel::last_enum_item,"Ignoring attempting to generate reward of invalid level for character "<<_ParentCharacter->getId().toString(),return CRingRewardPoints::grr_invalid);

	// if we're in an edit session then send a message to the client and give up
	if (!rewardLevel.isValid())
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GENERATE_IGNORED");
		LOG("RRP generateReward(invalid): %s: ABORTING DUE TO INVALID REWARD LEVEL",_ParentCharacter->getId().toString().c_str());
		return CRingRewardPoints::grr_invalid;
	}

	if (RingRPMode==1)
	{
		/* *** todo *** */
		/*

			This is where we add some code to validate the mission step if player is in newbyland and has the 'complete a ring scenario' mission

		*/
		/* *** todo *** */
//#pragma message (NL_LOC_WRN "Implements mission step validation")
//#pragma message (NL_LOC_WRN "Implements 'rare' item reward generation")
		// TODO: Implements mission step validation and 'rare' item reward generation

		// NB : need to return grr_ok_rare if rare item were gained

		// calculate the reward points of this level that the player has
		uint32 &playerRewardPoints= _CumulatedRRP[rewardLevel.asIndex()];

		// make sure the player has sufficient reward points of this level
		if (playerRewardPoints < RingRPPointsPerReward)
		{
			// send a "You require at least %d reward points to generate a reward but you only have %d" message to client
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = _ScenarioLevel.asIndex();
			params[1].Int = RingRPPointsPerReward;
			params[2].Int = playerRewardPoints;
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GENERATE_LACK_POINTS", params);
			LOG("RRP generateReward(%s): %s: ABORTING DUE TO INSUFFICIENT REWARD POINTS playerRewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);
			return CRingRewardPoints::grr_no_points;
		}

		// setup parameters for the reward items
		NLMISC::CSheetId itemSheetId	= NLMISC::CSheetId(RingRPRewardItemSheet);
		uint16 quantity					= (uint16)RingRPRewardItemQuantity;
		uint16 itemLevel				= (uint16)rewardLevel.asLevel();

		LOG("RRP generateReward(%s): %s: GENERATING REWARDS: rewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);

		// generate as many rewards as we can
		uint32 totalRewardsGenerated= 0;
		while ( playerRewardPoints >= RingRPPointsPerReward && _ParentCharacter->createItemInInventory( INVENTORIES::bag, itemLevel, quantity, itemSheetId ) )
		{
			LOG("RRP generateReward(%s): %s: - GENERATE REWARD: rewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);
			// consume some points
			playerRewardPoints-= RingRPPointsPerReward;
			// add the generated rewards to the rewards counter
			totalRewardsGenerated+= quantity;
		}

		// if we managed to generate any rewards at all then send a success message to the player & update the database to reflact the consumed reward points
		if (totalRewardsGenerated!=0)
		{
			// update the database
//			_ParentCharacter->_PropertyDatabase.setProp(_DB_RRPSLevel_Reminder[rewardLevel.asIndex()], playerRewardPoints);
			CBankAccessor_PLR::getUSER().getRRPS_LEVELS(rewardLevel.asIndex()).setVALUE(_ParentCharacter->_PropertyDatabase, playerRewardPoints);


			// send a "you gain %d %s of quality %d" message to client
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
			params[0].Int = (sint32)totalRewardsGenerated;
			params[1].SheetId = itemSheetId;
			params[2].Int = (sint32)itemLevel;
			PHRASE_UTILITIES::sendDynamicSystemMessage( _ParentCharacter->getEntityRowId(), "RRP_GENERATE_REWARD", params );
		}

		// make sure the player's inventory isn't full
		if ( playerRewardPoints >= RingRPPointsPerReward )
		{
			// send a "Your inventory is too full to add any more ring rewards" to client
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GENERATE_INVENTORY_FULL");
			LOG("RRP generateReward(%s): %s: - INVENTORY FULL: rewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);
			return CRingRewardPoints::grr_no_place;
		}
	}

	if (RingRPMode==2)
	{
		/* *** todo *** */
		/*
			This is where we add some code to validate the mission step if player is in newbyland and has the 'complete a ring scenario' mission
		*/
//#pragma message (NL_LOC_WRN "Implements mission step validation")
		// TODO: Implements mission step validation
		/* *** todo *** */

		// some handy temporaries
		bool insufficientRRP= true;
		bool invFull= false;
		uint32 bestRRP= 0;
		R2::TSessionLevel bestLevel= R2::TSessionLevel::sl_a;

		// for each reward level
		for (R2::TSessionLevel rewardLevel=R2::TSessionLevel::sl_a; rewardLevel.isValid(); rewardLevel=(R2::TSessionLevel::TValues)(rewardLevel.getValue()+1))
		{
			// some handy temporaries
			uint32 totalRewardsGenerated= 0;

			// get a refference to the reward points of this level that the player has, and make sure he has enough for at least 1 reward
			uint32 &playerRewardPoints= _CumulatedRRP[rewardLevel.asIndex()];
			if (playerRewardPoints < RingRPPointsPerReward)
			{
				if (bestRRP>=playerRewardPoints) { bestLevel=rewardLevel; bestRRP=playerRewardPoints; }
				continue;
			}

			// we have sufficuent RRP for this type of reward
			insufficientRRP= false;

			// setup parameters for the reward items
			NLMISC::CSheetId itemSheetId	= NLMISC::CSheetId(RingRPRewardItemSheet);
			uint16 quantity					= (uint16)RingRPRewardItemQuantity;
			uint16 itemLevel				= (uint16)rewardLevel.asLevel();

			LOG("RRP generateReward(%s): %s: GENERATING REWARDS: rewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);

			// generate as many rewards as we can
			while ( playerRewardPoints >= RingRPPointsPerReward )
			{
				// try to create a new reward
				bool createItemResult= _ParentCharacter->createItemInInventory( INVENTORIES::bag, itemLevel, quantity, itemSheetId );

				// see if the player inventory is full
				if (createItemResult==false)
				{
					invFull= true;
					LOG("RRP generateReward(%s): %s: - INVENTORY FULL: rewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);
					break;
				}

				// we succeeded in generating a reaward...
				LOG("RRP generateReward(%s): %s: - GENERATE REWARD: rewardPoints %u", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), playerRewardPoints);
				// consume some points
				playerRewardPoints-= RingRPPointsPerReward;
				// add the generated rewards to the rewards counter
				totalRewardsGenerated+= quantity;
			}

			// if we managed to generate any rewards at all then send a success message to the player & update the database to reflact the consumed reward points
			if (totalRewardsGenerated!=0)
			{
				// send a "you gain %d %s of quality %d" message to client
				SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
				params[0].Int = (sint32)totalRewardsGenerated;
				params[1].SheetId = itemSheetId;
				params[2].Int = (sint32)itemLevel;
				PHRASE_UTILITIES::sendDynamicSystemMessage( _ParentCharacter->getEntityRowId(), "RRP_GENERATE_REWARD", params );
			}

			// update the database
//			_ParentCharacter->_PropertyDatabase.setProp(_DB_RRPSLevel_Reminder[rewardLevel.asIndex()], playerRewardPoints);
			CBankAccessor_PLR::getUSER().getRRPS_LEVELS(rewardLevel.asIndex()).setVALUE(_ParentCharacter->_PropertyDatabase, playerRewardPoints);
		}

		// make sure the player has sufficient reward points of at lease one level
		if (insufficientRRP)
		{
			// send a "You require at least %d reward points to generate a reward but you only have %d" message to client
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = bestLevel.asIndex();
			params[1].Int = RingRPPointsPerReward;
			params[2].Int = bestRRP;
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GENERATE_LACK_POINTS", params);
			LOG("RRP generateReward(%s): %s: ABORTING DUE TO INSUFFICIENT REWARD POINTS playerRewardPoints %u", bestLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str(), bestRRP);
			return CRingRewardPoints::grr_no_points;
		}

		// make sure the player's inventory isn't full
		if ( invFull )
		{
			// send a "Your inventory is too full to add any more ring rewards" to client
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ParentCharacter->getEntityRowId(), "RRP_GENERATE_INVENTORY_FULL");
			return CRingRewardPoints::grr_no_place;
		}

		// NB : need to return grr_ok_rare if rare item were gained
//#pragma message (NL_LOC_WRN "Implements 'rare' item reward generation")
		// TODO: Implements 'rare' item reward generation
	}

	LOG("RRP generateReward(%s): %s: - SUCCESS", rewardLevel.toString().c_str(), _ParentCharacter->getId().toString().c_str());
	return CRingRewardPoints::grr_ok;
}


//-----------------------------------------------------------------------------
// Persistent data for CRingRewardPointsImpl
//-----------------------------------------------------------------------------

#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily
#define PERSISTENT_CLASS CRingRewardPointsImpl

#define PERSISTENT_PRE_APPLY\
	for (uint32 i=0;i<sizeof(_CumulatedRRP)/sizeof(_CumulatedRRP[0]);++i)\
		_CumulatedRRP[i]= 0;\

#define PERSISTENT_DATA\
  LPROP_MAP2(RingRewardPoints,NLMISC::CSString,uint32,\
		ARRAY_LOGIC((uint32)R2::TSessionLevel::nb_enum_items),\
		(NLMISC::CSString("Level")+char('A'+i)),\
		 _CumulatedRRP[i],\
		 if (key.size()==6) { uint32 idx=(key[5]-'A'); if (idx <R2::TSessionLevel::nb_enum_items) _CumulatedRRP[idx]=val; } )\
  LPROP_MAP2(RingRewardPointsHistory,NLMISC::CSString,uint32,\
		ARRAY_LOGIC((uint32)R2::TSessionLevel::nb_enum_items),\
		(NLMISC::CSString("Level")+char('A'+i)),\
		 _HistoricRRP[i],\
		 if (key.size()==6) { uint32 idx=(key[5]-'A'); if (idx <R2::TSessionLevel::nb_enum_items) _HistoricRRP[idx]=val; } )\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
