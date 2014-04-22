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



#ifndef TESTING_TOOL_H
#define TESTING_TOOL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/entity_id.h"
#include "nel/net/service.h"
#include "nel/misc/bit_set.h"

#include "game_share/testing_tool_structures.h"

#include <string>
#include <stdio.h>

// Callback connection / disconection management
void cbConnection( const std::string &serviceName, uint16 sid, void *arg );
void cbDisconnection( const std::string &serviceName, uint16 sid, void *arg );


/**
 * CTestingTool
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CTestingTool : public NLNET::IService
{
public:

	struct SActor
	{
		uint8	TargetIndex;
		float	TargetDistance;
		uint16	LevelOffset;
		uint8	RepeatAction;
		uint8	RepeatActionType;
		uint8	PunctualAction1;
		uint16	PunctualAction1Rate;
		uint8	PunctualAction1Type;
		uint8	PunctualAction2;
		uint16	PunctualAction2Rate;
		uint8	PunctualAction2Type;
		std::string Actor;

		SActor()
		{
			TargetDistance = 0;
			LevelOffset = 0;
			RepeatAction = 0;
			RepeatActionType = 0;
			PunctualAction1 = 0;
			PunctualAction1Rate = 0;
			PunctualAction1Type = 0;
			PunctualAction2 = 0;
			PunctualAction2Rate = 0;
			PunctualAction2Type = 0;
		}
	};

	struct SActorStatistics
	{
		uint16 NbTouch;
		uint16 NbFailure;
		uint16 NbCritical;
		uint32 SendDamage;
		uint32 ShieldAbsorption;
		uint32 ArmorAbsorption;
		uint32 GivenDamage;
		sint32 FinalHp;
		sint32 FinalSta;
		sint32 FinalSap;

		void init()
		{
			NbTouch = 0;
			NbFailure = 0;
			NbCritical = 0;
			SendDamage = 0;
			ShieldAbsorption = 0;
			ArmorAbsorption = 0;			
			GivenDamage = 0;			
			FinalHp = 0;
			FinalSta = 0;
			FinalSap = 0;
		}

		const SActorStatistics &operator + ( SActorStatistics& s )
		{
			NbTouch += s.NbTouch;
			NbFailure += s.NbFailure;
			NbCritical += s.NbCritical;
			SendDamage = s.SendDamage;
			ShieldAbsorption += s.ShieldAbsorption;
			ArmorAbsorption = s.ArmorAbsorption;	
			GivenDamage = s.GivenDamage;
			FinalHp += s.FinalHp;
			FinalSta += s.FinalSta;
			FinalSap += s.FinalSap;
			return *this;
		}

	};

	struct SSentenceStatistics
	{
		uint16 NbUse;
		uint16 NbFail;
		uint16 NbSuccess;
		uint16 TotalSendDamage;
		uint16 TotalSapConsum;
		uint16 TotalStaminaConsume;
		uint16 TotalShieldAbsorption;
		uint16 TotalArmorAbsorption;
		uint16 TotalGivenDamage;

		void init()
		{
			NbUse = 0;
			NbFail = 0;
			NbSuccess = 0;
			TotalSendDamage = 0;
			TotalSapConsum = 0;
			TotalStaminaConsume = 0;
			TotalShieldAbsorption = 0;
			TotalArmorAbsorption = 0;
			TotalGivenDamage = 0;
		}

		const SSentenceStatistics &operator + ( SSentenceStatistics& s )
		{
			NbUse += s.NbUse;
			NbFail += s.NbFail;
			NbSuccess += s.NbSuccess;
			TotalSendDamage += s.TotalSendDamage;
			TotalSapConsum += s.TotalSapConsum;
			TotalStaminaConsume += s.TotalStaminaConsume;
			TotalShieldAbsorption += s.TotalShieldAbsorption;
			TotalArmorAbsorption += s.TotalArmorAbsorption;
			TotalGivenDamage += s.TotalGivenDamage;
			return *this;
		}
	};

	/** 
	 * init the service
	 */
	void init(void);

	/**
	 * load the testing script command
	 */
	void initConfigFileVars();

	/**
	 * main loop
	 */
	bool update(void);

	/**
	 * update, called at each tick
	 */
	static void ttsUpdate();
	
	/**
	 * release
	 */
	void release(void);

	// Start test session: somes init and go
	void startTestSession( const std::string& sheet );

	// Test session proceed
	void testSessionProceed();

	// startTest: start one test
	void startTest();

	// Test end: an Actor is dead, the current test is end
	void testEnd();

	// Test update: executed at each tick
	void testUpdate();

	/// execute speciefied sentence
	void executeSentence( const NLMISC::CEntityId& id, uint8 sentenceIdx, uint8 sentenceType );

	// set actor start statistics
	void setActorStartState( NLNET::CMessage& msgin );

	// Actro dead
	void setActorDead( NLMISC::CEntityId& id );

	// Log report for an actor
	void logReport( NLMISC::CEntityId& id, SLogReport& LogReport );

	// Sentence comparison
	bool CTestingTool::testSentence( const std::vector< NLMISC::CSheetId >& s1, const std::vector< NLMISC::CSheetId >& s2 );
		
private:
	// Sheet information
	std::string		_TestDescription;
	uint16			_IterationCount;
	uint16			_LevelMini;
	uint16			_LevelMaxi;

	std::vector<SActor>					_Actors;
	std::vector<NLMISC::CEntityId>		_ActorsIds;
	NLMISC::CBitSet						_ActorsDeathFlags;
	std::vector<SSentenceStatistics>	_ActorsSentence1;
	std::vector<SSentenceStatistics>	_ActorsSentence2;
	std::vector<SSentenceStatistics>	_ActorsSentence3;
	std::vector<SActorStatistics>		_ActorsStats;
	std::vector<SActorBeginTest>		_ActorsBeginStats;

	std::vector<uint16>					_ActionSend;
	std::vector<NLMISC::TGameCycle>		_ActionTime;
	std::vector<int>					_ActorsNbWin;

	

	// Test variable management
	uint16				_CurrentLevel;
	uint16				_CurrentIteration;
	bool				_TestOccurs;

	uint16				_NbCombatCycle;
	NLMISC::TGameCycle	_TimeTestStart;
	int					_EngageFightInTick;

	FILE*				_TestSessionReport;


	/*
	uint16	_ActionSendActor1;
	uint16	_ActionSendActor2;
	FILE*	_TestSessionReport;

	bool	_Actor1Dead;
	bool	_Actor2Dead;

	int	_Actor1NbWin;
	int	_Actor2NbWin;

	

	// Actor start statictics
	SActorBeginTest	_Actor1BeginStats;
	SActorBeginTest	_Actor2BeginStats;

  NLMISC::TGameCycle _ActionTime1;
	NLMISC::TGameCycle _ActionTime2;
*/
	// Statistics

	
	
};

#endif //TESTING_TOOL_H

