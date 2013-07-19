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



//--------------------
// INCLUDE
//--------------------
#include "stdpch.h"
#include "testing_tool_service.h"

// game share
#include "game_share/tick_event_handler.h"
#include "game_share/slot_equipment.h"
#include "game_share/roles.h"
#include "game_share/brick_types.h"

// Nel
#include "nel/misc/sheet_id.h"
#include "nel/misc/command.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_loader.h"

#include "stdio.h"

#include <algorithm>

//--------------------
// USING
//--------------------
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

//--------------------
// GLOBALS
//--------------------
CTestingTool*		pTestingTool = NULL;
uint8				EGSIsUp = 0;

/****************************************************************\
							init() 
\****************************************************************/
void CTestingTool::init()
{
	// Init CSheetId
	CSheetId::init(0);

	//init the global TestingTool pointer
	pTestingTool = this;
	_TestOccurs = false;
	_EngageFightInTick = 0;


	// init the Tick Event Handler
	CTickEventHandler::init( ttsUpdate );

	// Init the pseudo random number generator
	srand( (uint)CTickEventHandler::getGameTime() );

	//set connect/disconnect callbacks
	CUnifiedNetwork::getInstance()->setServiceUpCallback( string("*"), cbConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbDisconnection, 0);
}

/****************************************************************\
							update() 
\****************************************************************/
bool CTestingTool::update()
{
	return true;
} // update //

/****************************************************************\
						ttsUpdate() 
\****************************************************************/
void CTestingTool::ttsUpdate()
{
	if( pTestingTool )
	{
		pTestingTool->testUpdate();
	}
} // ttsUpdate //

/****************************************************************\
							release() 
\****************************************************************/
void CTestingTool::release()
{
}// release //

/****************************************************************\
							cbConnection() 
\****************************************************************/
void cbConnection( const std::string &serviceName, uint16 serviceId, void *arg )
{
	if (serviceName=="EGS")
	{
		EGSIsUp++;
		// set our entities server Id as the same as the EGS
		// we use the CEntityId "bad code" for now
		CEntityId id;
		id.setServiceId((uint8)serviceId);
	}
}

/****************************************************************\
							cbDisconnection() 
\****************************************************************/
void cbDisconnection( const std::string &serviceName, uint16 serviceId, void *arg )
{
	if (serviceName=="EGS")
	{
		EGSIsUp--;
	}
} // cbDisconnection //

//----------------------------------------------------------------
// Start test session
//
//----------------------------------------------------------------
void CTestingTool::startTestSession( const std::string& sheet )
{
	if ( EGSIsUp == 0 )
	{
		nlwarning("EGS should be up to start tests");
		return;
	}
	// Load sheet describe test session
	NLGEORGES::UFormLoader *formLoader = NLGEORGES::UFormLoader::createLoader ();

	NLMISC::CSmartPtr<NLGEORGES::UForm> form = formLoader->loadForm ((sheet+string(".combat_test")).c_str ());
	if (form)
	{
		UFormElm& root = form->getRootNode();

		// Test description
		if( ! root.getValueByName ( _TestDescription, "test description") )
		{
			nlwarning( "<CTestingTool::startTestSession> can get the value 'test description' in sheet %s", sheet.c_str() );
		}

		// Iteration
		if( ! root.getValueByName ( _IterationCount, "iteration count") )
		{
			nlwarning( "<CTestingTool::startTestSession> can get the value 'iteration count' in sheet %s", sheet.c_str() );
		}

		// Level min
		if( ! root.getValueByName ( _LevelMini, "level min") )
		{
			nlwarning( "<CTestingTool::startTestSession> can get the value 'level min' in sheet %s", sheet.c_str() );
		}
		
		// Level max
		if( ! root.getValueByName ( _LevelMaxi, "level max") )
		{
			nlwarning( "<CTestingTool::startTestSession> can get the value 'level max' in sheet %s", sheet.c_str() );
		}

		//////////////////////////////////////////////////////////
		// Actors
		UFormElm* actorNode = NULL;
		char nodeName[128];
		strcpy(nodeName,"actor 0");
		uint i =0;
		while(root.getNodeByName ( &actorNode, nodeName ) && actorNode)
		{
			SActor actor;
			// Actor setting
			if( ! actorNode->getValueByName ( actor.Actor, "Actor Setting") )
				nlwarning( "<CTestingTool::startTestSession> can get the value 'Actor Setting'  for actor %d in sheet %s",i, sheet.c_str() );
			if ( actor.Actor.empty() )
			{
				nlinfo("<CTestingTool::startTestSession> actor count : %d", i);
				break;
			}

			// Level offset
			if ( !actorNode->getValueByName ( actor.LevelOffset, "level offset"))
				nlwarning( "<CTestingTool::startTestSession> can get the value 'level offset' for actor %d in sheet %s",i, sheet.c_str() );
			// Repeat action
			if ( !actorNode->getValueByName ( actor.RepeatAction, "repeat action"))
				nlwarning( "<CTestingTool::startTestSession> can get the value 'repeat action' in sheet %s", sheet.c_str() );
			if ( !actorNode->getValueByName ( actor.RepeatActionType, "repeat action type"))
				nlwarning( "<CTestingTool::startTestSession> can get the value 'repeat action type' in sheet %s", sheet.c_str() );
			
			// Punctual action 1
			if ( !actorNode->getValueByName ( actor.PunctualAction1, "punctual action 1 phrase"))
				nlwarning( "<CTestingTool::startTestSession> can get the value 'punctual action 1 phrase'  for actor %d in sheet %s", i,sheet.c_str() );
			// Punctual action 1 rate
			float f;
			if( ! actorNode->getValueByName ( f, "punctual action 1 repeat rate") )
				nlwarning( "<CTestingTool::startTestSession> can get the value 'punctual action 1 repeat rate'  for actor %d in sheet %s",i,sheet.c_str() );
			else
				actor.PunctualAction1Rate = (uint16) ( f / 0.1f );
			if ( !actorNode->getValueByName ( actor.PunctualAction1Type, "punctual action 1 type"))
				nlwarning( "<CTestingTool::startTestSession> can get the value 'punctual action 1 type'  for actor %d in sheet %s", i,sheet.c_str() );
			
			// Punctual action 2
			if( ! actorNode->getValueByName ( actor.PunctualAction2, "punctual action 2 phrase") )
				nlwarning( "<CTestingTool::startTestSession> can get the value 'punctual action 2 phrase'  for actor %d in sheet %s",i, sheet.c_str() );
			// Punctual action 2 rate
			if( ! actorNode->getValueByName ( f, "punctual action 2 repeat rate") )
				nlwarning( "<CTestingTool::startTestSession> can get the value 'punctual action 2 repeat rate'  for actor %d in sheet %s",i, sheet.c_str() );
			else
				actor.PunctualAction2Rate = (uint16) ( f / 0.1f );
			if ( !actorNode->getValueByName ( actor.PunctualAction2Type, "punctual action 2 type"))
				nlwarning( "<CTestingTool::startTestSession> can get the value 'punctual action 2 type'  for actor %d in sheet %s", i,sheet.c_str() );
			
			// Actor target
			if( ! actorNode->getValueByName ( actor.TargetIndex, "Target") )
				nlwarning( "<CTestingTool::startTestSession> can get the value 'Actor Target'  for actor %d in sheet %s",i, sheet.c_str() );

			// Target distance
			if( ! actorNode->getValueByName ( actor.TargetDistance, "TargetDistance") )
				nlwarning( "<CTestingTool::startTestSession> can get the value 'TargetDistance'  for actor %d in sheet %s",i, sheet.c_str() );

			_Actors.push_back(actor);
			sprintf(nodeName,"actor %d",++i);	
		}
		
		// resize actor property vectors
		_ActorsIds.resize(_Actors.size());
		_ActorsDeathFlags.resize(_Actors.size());
		_ActorsSentence1.resize(_Actors.size());
		_ActorsSentence2.resize(_Actors.size());
		_ActorsSentence3.resize(_Actors.size());
		_ActorsStats.resize(_Actors.size());
		_ActorsBeginStats.resize(_Actors.size());
		_ActionSend.resize(_Actors.size());
		_ActionTime.resize(_Actors.size());
		_ActorsNbWin.resize(_Actors.size());
	
		// check targets validity
		for ( i = 0; i< _Actors.size(); i++ )
		{
			if ( _Actors[i].TargetIndex >= _Actors.size()  || i == _Actors[i].TargetIndex)
			{
				nlwarning("invalid target index %d for actor %d ( there are %d actors )",_Actors[i].TargetIndex,i, _Actors.size() );
				return;
			}
		}
		testSessionProceed();
	}
	else
	{
		nlinfo( "Test Sheet %s not found", sheet.c_str() );
	}
}
	
//----------------------------------------------------------------
// Test session proceed
//
//----------------------------------------------------------------
void CTestingTool::testSessionProceed()
{
	_TestSessionReport = fopen( "testSessionReport.txt", "w+t" );

	fputs( "=====================================================================\n", _TestSessionReport );
	fputs( "===================== NEW TEST SESSION STARTED ======================\n", _TestSessionReport );
	fputs( "=====================================================================\n\n", _TestSessionReport );
	fputs( _TestDescription.c_str(), _TestSessionReport );
	fputs( "\n\n", _TestSessionReport );

	_CurrentLevel = _LevelMini;
	_CurrentIteration = 0;

	// Init statistics members
	for (uint i = 0; i < _ActorsIds.size();i++ )
	{
		_ActorsStats[i].init();
		_ActorsSentence1[i].init();
		_ActorsSentence2[i].init();
		_ActorsSentence3[i].init();
		_ActorsNbWin[i] = 0;
	}
	_NbCombatCycle = 0;
	_TimeTestStart = CTickEventHandler::getGameCycle() + 50;
	// Starting test.. Engage !
	startTest();
}

//----------------------------------------------------------------
// Test occurs: return true until test is occurs
//
//----------------------------------------------------------------
void CTestingTool::startTest()
{
	// Spawn actors in EGS
	// have to be done in two passes to build entity ids
	for (uint i = 0; i< _ActorsIds.size(); i++ )
	{
		uint16 actorLevel = _Actors[i].LevelOffset + _CurrentLevel;
	

		_ActorsIds[i].setType( RYZOMID::player );
		_ActorsIds[i].setShortId( 0x100 + i*0x100 );
		// patch totake new char creation into account
		uint8 creatorId = _ActorsIds[i].getDynamicId();

		uint64 id = 0x20000000;
		uint32 usedId = (uint32) ((_ActorsIds[i].getShortId() >> 8 ) & 0x001fffff);
		id |= uint64(creatorId) << 21;
		id |= ( uint64(usedId) <<8)|0;
		_ActorsIds[i] = CEntityId(RYZOMID::player,id);
		_ActorsIds[i].setCreatorId( creatorId );


		CMessage msgout("SPAWN_TEST_PLAYER");
		msgout.serial( _Actors[i].Actor );
		msgout.serial( _Actors[i].TargetDistance );
		msgout.serial( _ActorsIds[ _Actors[i].TargetIndex ]);
		msgout.serial( actorLevel );				
		msgout.serial( _ActorsIds[i] );
		CUnifiedNetwork::getInstance()->send( "EGS", msgout );
		_ActorsDeathFlags.set(i,false);
	}
	string s = string("\n*********** Starting test iteration ") + toString( _CurrentIteration ) + string("\n");
	fputs( s.c_str(), _TestSessionReport );
	nlinfo( s.c_str() );

	_EngageFightInTick = 50;
	_TimeTestStart = CTickEventHandler::getGameCycle() + 50;
}

//----------------------------------------------------------------
// Test occurs: return true until test is occurs
//
//----------------------------------------------------------------
void CTestingTool::testEnd()
{
	for (uint i = 0; i< _Actors.size(); i++ )
	{
		CMessage msgout("UNSPAWN_ACTOR");
		msgout.serial( _ActorsIds[i] );
		CUnifiedNetwork::getInstance()->send( "EGS", msgout );
		if ( _ActorsDeathFlags[_Actors[i].TargetIndex] )
			_ActorsNbWin[i]++;
	}

	_CurrentIteration++;

	
	if( _CurrentIteration < _IterationCount )
	{
		startTest();
	}
	else
	{
		// Output test session statistics
		string out = string("\n\n/////////////// Test Review, ") + NLMISC::toString( _IterationCount ) + string(" iterations completed \\\\\\\\\\\\\\\\\\\\\\\\\n");
		fputs( out.c_str(), _TestSessionReport );
		out = string(" =*=*=*=*=*= Actors Starting states report =*=*=*=*=*=\n" );
		fputs( out.c_str(), _TestSessionReport );
		uint i =0;
		for ( ; i < _Actors.size(); i++ )
		{
			out = string("===== Start Actor") + toString(i) + string(" State ") + _Actors[i].Actor + string("\n");
			fputs( out.c_str(), _TestSessionReport ); 
//			out = string("Actor profile ") + ROLES::toString( (ROLES::ERole) _ActorsBeginStats[i].Role ) + string("\n");
//			fputs( out.c_str(), _TestSessionReport ); 
//			out = string("Level ") + NLMISC::toString( _ActorsBeginStats[i].Level ) + string("\n");
//			fputs( out.c_str(), _TestSessionReport ); 
			out = string("Hp ") + NLMISC::toString( _ActorsBeginStats[i].Hp ) + string("\n");
			fputs( out.c_str(), _TestSessionReport ); 
			out = string("Stamina ") + NLMISC::toString( _ActorsBeginStats[i].Sta ) + string("\n");
			fputs( out.c_str(), _TestSessionReport ); 
			out = string("Sap ") + NLMISC::toString( _ActorsBeginStats[i].Sap ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string("Armor ") + _ActorsBeginStats[i].Armor.toString() + string(" Quality ") + NLMISC::toString( _ActorsBeginStats[i].ArmorQuality ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string("Item in right hand ") + _ActorsBeginStats[i].RightHand.toString() + string(" Quality ") + NLMISC::toString( _ActorsBeginStats[i].RightHandQuality ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string("Item in left hand ") + _ActorsBeginStats[i].LeftHand.toString() + string(" Quality ") + NLMISC::toString( _ActorsBeginStats[i].LeftHandQuality ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			for( int s = 0; s < 3; ++s )
			{
				vector< NLMISC::CSheetId > &sentence = (s==0)?_ActorsBeginStats[i].Sentence1:( (s==1)?_ActorsBeginStats[i].Sentence2:_ActorsBeginStats[i].Sentence3);
				
				out = string("Sentence  ") + NLMISC::toString( s + 1 ) + string(": ");
				fputs( out.c_str(), _TestSessionReport );
				
				for( vector< NLMISC::CSheetId >::iterator it = sentence.begin(); it != sentence.end(); ++it )
				{
					out = (*it).toString() + string(" ");
					fputs( out.c_str(), _TestSessionReport );
				}
				fputs( "\n", _TestSessionReport );
			}
		}
		fputs( "\n", _TestSessionReport );
		
		// Combat session report
		out = string( " =*=*=*=*=*= Combat session report =*=*=*=*=*=\n" );
		fputs( out.c_str(), _TestSessionReport );
		out = string( "Nb Combat cyle " ) + NLMISC::toString( _NbCombatCycle / ( _IterationCount * _Actors.size() ) ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		out = string( "Combat duration " ) + NLMISC::toString( _NbCombatCycle / ( 0.1f * _IterationCount ) ) + string( " secondes\n");
		fputs( out.c_str(), _TestSessionReport );

		// Actors combat session statistics
		out = string(" =*=*=*=*=*= Actors combat statistics report =*=*=*=*=*=\n");
		fputs( out.c_str(), _TestSessionReport );
		
		for (i = 0; i < _Actors.size(); i++ )
		{
			out = string("==== Actor")+toString(i)+string(":\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string("Actor")+toString(i)+string(" win fight ") + NLMISC::toString( _ActorsNbWin[i] ) + string(" times\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Nb Touchs: " ) + NLMISC::toString( _ActorsStats[i].NbTouch / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Nb Fails: " ) + NLMISC::toString( _ActorsStats[i].NbFailure / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Nb Criticals: " ) + NLMISC::toString( _ActorsStats[i].NbCritical / _IterationCount) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Nb Gived damages: " ) + NLMISC::toString( _ActorsStats[i].GivenDamage / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Armor adsorption: " ) + NLMISC::toString( _ActorsStats[i].ArmorAbsorption / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Shield adsorption: " ) + NLMISC::toString( _ActorsStats[i].ShieldAbsorption / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Effective damages: " ) + NLMISC::toString( _ActorsStats[i].SendDamage / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Final Hp Value: " ) + NLMISC::toString( _ActorsStats[i].FinalHp / _IterationCount ) + string( " stats: " ) + NLMISC::toString( 100.f * _ActorsStats[i].FinalHp / _ActorsBeginStats[i].Hp / _IterationCount ) + string( " %\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Final Stamina Value: " ) + NLMISC::toString( _ActorsStats[i].FinalSta / _IterationCount ) + string( " stats: " ) + NLMISC::toString( 100.f * _ActorsStats[i].FinalSta / _ActorsBeginStats[i].Sta / _IterationCount ) + string( " %\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Final Sap Value: " ) + NLMISC::toString( _ActorsStats[i].FinalSap / _IterationCount ) + string( " stats: " ) + NLMISC::toString( 100.f * _ActorsStats[i].FinalSap / _ActorsBeginStats[i].Sap / _IterationCount ) + string( " %\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Ratio Stamina per gived damage: " ) + NLMISC::toString( 1.0f * ( _ActorsBeginStats[i].Sta - ( _ActorsStats[i].FinalSta / _IterationCount ) ) / ( _ActorsStats[i].GivenDamage / _IterationCount ) ) + string( "\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Ratio Sap per gived damage: " ) + NLMISC::toString( 1.0f * ( _ActorsBeginStats[i].Sap - ( _ActorsStats[i].FinalSap / _IterationCount ) ) / ( _ActorsStats[i].GivenDamage / _IterationCount ) ) + string( "\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Ratio Stamina per effective damage: " ) + NLMISC::toString( 1.0f * ( _ActorsBeginStats[i].Sta - ( _ActorsStats[i].FinalSta / _IterationCount ) ) / ( _ActorsStats[i].SendDamage / _IterationCount ) ) + string( "\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( " Ratio Sap per effective damage: " ) + NLMISC::toString( ( 1.0f * _ActorsBeginStats[i].Sap - ( _ActorsStats[i].FinalSap / _IterationCount ) ) / ( _ActorsStats[i].SendDamage / _IterationCount ) ) + string( "\n" );
			fputs( out.c_str(), _TestSessionReport );
		}
	

		// Sentences combat session statistics
		out = string("\n=*=*=*=*=*= Sentences combat statistics report =*=*=*=*=*=\n");
		fputs( out.c_str(), _TestSessionReport );
		
		for (i = 0; i < _Actors.size(); i++ )
		{
			// sentence 1
			out = string("==== Actor ") + toString(i) + string(" Sentence 1:\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb use: " ) + NLMISC::toString( _ActorsSentence1[i].NbUse / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb failure: " ) + NLMISC::toString( _ActorsSentence1[i].NbFail / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb success: " ) + NLMISC::toString( _ActorsSentence1[i].NbSuccess / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total gived damage: " ) + NLMISC::toString( _ActorsSentence1[i].TotalGivenDamage / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total Stamina consummate: " ) + NLMISC::toString( _ActorsSentence1[i].TotalStaminaConsume / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total Sap consummate: " ) + NLMISC::toString( _ActorsSentence1[i].TotalSapConsum / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			
			if( _ActorsSentence1[i].NbSuccess )
			{
				out = string( "Average gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalGivenDamage / _ActorsSentence1[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average shield absorption: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalShieldAbsorption / _ActorsSentence1[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average armor absorption: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalArmorAbsorption / _ActorsSentence1[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalSendDamage / _ActorsSentence1[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
			
			if( _ActorsSentence1[i].TotalGivenDamage )
			{
				out = string( "Stamina consummate by gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalStaminaConsume / _ActorsSentence1[i].TotalGivenDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Sap consummate by gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalSapConsum / _ActorsSentence1[i].TotalGivenDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
			
			if( _ActorsSentence1[i].TotalSendDamage )
			{
				out = string( "Stamina consummate by effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalStaminaConsume / _ActorsSentence1[i].TotalSendDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Sap consummate by effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence1[i].TotalSapConsum / _ActorsSentence1[i].TotalSendDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}

			// sentence 2
			out = string("==== Actor ") + toString(i) + string(" Sentence 2:\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb use: " ) + NLMISC::toString( _ActorsSentence2[i].NbUse / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb failure: " ) + NLMISC::toString( _ActorsSentence2[i].NbFail / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb success: " ) + NLMISC::toString( _ActorsSentence2[i].NbSuccess / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total gived damage: " ) + NLMISC::toString( _ActorsSentence2[i].TotalGivenDamage / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total Stamina consummate: " ) + NLMISC::toString( _ActorsSentence2[i].TotalStaminaConsume / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total Sap consummate: " ) + NLMISC::toString( _ActorsSentence2[i].TotalSapConsum / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			
			if( _ActorsSentence2[i].NbSuccess )
			{
				out = string( "Average gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalGivenDamage / _ActorsSentence2[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average shield absorption: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalShieldAbsorption / _ActorsSentence2[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average armor absorption: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalArmorAbsorption / _ActorsSentence2[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalSendDamage / _ActorsSentence2[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
			
			if( _ActorsSentence2[i].TotalGivenDamage )
			{
				out = string( "Stamina consummate by gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalStaminaConsume / _ActorsSentence2[i].TotalGivenDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Sap consummate by gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalSapConsum / _ActorsSentence2[i].TotalGivenDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
			
			if( _ActorsSentence2[i].TotalSendDamage )
			{
				out = string( "Stamina consummate by effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalStaminaConsume / _ActorsSentence2[i].TotalSendDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Sap consummate by effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence2[i].TotalSapConsum / _ActorsSentence2[i].TotalSendDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}

			// sentence 3
			out = string("==== Actor ") + toString(i) + string(" Sentence 3:\n" );
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb use: " ) + NLMISC::toString( _ActorsSentence3[i].NbUse / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb failure: " ) + NLMISC::toString( _ActorsSentence3[i].NbFail / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Nb success: " ) + NLMISC::toString( _ActorsSentence3[i].NbSuccess / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total gived damage: " ) + NLMISC::toString( _ActorsSentence3[i].TotalGivenDamage / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total Stamina consummate: " ) + NLMISC::toString( _ActorsSentence3[i].TotalStaminaConsume / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			out = string( "Total Sap consummate: " ) + NLMISC::toString( _ActorsSentence3[i].TotalSapConsum / _IterationCount ) + string("\n");
			fputs( out.c_str(), _TestSessionReport );
			
			if( _ActorsSentence3[i].NbSuccess )
			{
				out = string( "Average gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalGivenDamage / _ActorsSentence3[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average shield absorption: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalShieldAbsorption / _ActorsSentence3[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average armor absorption: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalArmorAbsorption / _ActorsSentence3[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Average effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalSendDamage / _ActorsSentence3[i].NbSuccess ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
			
			if( _ActorsSentence3[i].TotalGivenDamage )
			{
				out = string( "Stamina consummate by gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalStaminaConsume / _ActorsSentence3[i].TotalGivenDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Sap consummate by gived damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalSapConsum / _ActorsSentence3[i].TotalGivenDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
			
			if( _ActorsSentence3[i].TotalSendDamage )
			{
				out = string( "Stamina consummate by effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalStaminaConsume / _ActorsSentence3[i].TotalSendDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
				out = string( "Sap consummate by effective damage: " ) + NLMISC::toString( 1.0f * _ActorsSentence3[i].TotalSapConsum / _ActorsSentence3[i].TotalSendDamage ) + string("\n");
				fputs( out.c_str(), _TestSessionReport );
			}
		}
		// Restart test session at next level if needed
		if( ++_CurrentLevel <= _LevelMaxi )
		{
			_CurrentIteration = 0;
			// Init statistics members
			for (uint i = 0; i < _Actors.size(); i++ )
			{
				_ActorsStats[i].init();
				_ActorsSentence1[i].init();
				_ActorsSentence2[i].init();
				_ActorsSentence3[i].init();
			}
			_NbCombatCycle = 0;
			_TimeTestStart = CTickEventHandler::getGameCycle() + 22;
			startTest();
		}
		else
		{
			_TestOccurs = false;
			_Actors.clear();
			fputs( "*** Test Session End ***\n", _TestSessionReport );
			fclose( _TestSessionReport );
			nlinfo( "************** Test ended **************" );
		}
	}
}

//----------------------------------------------------------------
// Test update: executed at each tick
//
//----------------------------------------------------------------
void CTestingTool::testUpdate()
{
	if( _TestOccurs )
	{
		bool combatEnd = true;
		for (uint i = 0; i < _Actors.size(); i++ )
		{
			if( !_ActorsDeathFlags[i] && !_ActorsDeathFlags[ _Actors[i].TargetIndex ] )
				combatEnd = false;
	
			// Test if is time for punctual action for actor
			if( _ActionSend[i] == 0 && _Actors[i].PunctualAction1Rate != 0 )
			{				
				if( ( ( CTickEventHandler::getGameCycle() - _ActionTime[i] ) % _Actors[i].PunctualAction1Rate ) == 0 )
				{
					// Execute punctual action 1 for actor i
					_ActionTime[i] = CTickEventHandler::getGameCycle();
					_ActionSend[i] = 1;
					executeSentence( _ActorsIds[i], _Actors[i].PunctualAction1, _Actors[i].PunctualAction1Type );
				}
			}
			else if( _Actors[i].PunctualAction2Rate != 0 )
			{
				if( ( ( CTickEventHandler::getGameCycle() - _ActionTime[i] ) % _Actors[i].PunctualAction2Rate ) == 0 )
				{
					// Execute punctual action 2 for actor 1
					_ActionTime[i] = CTickEventHandler::getGameCycle();
					_ActionSend[i] = 0;
					executeSentence( _ActorsIds[i], _Actors[i].PunctualAction2, _Actors[i].PunctualAction2Type);
				}
			}
			else
				_ActionSend[i] = 0;
		}
		if (combatEnd)
		{
			testEnd();
			_TestOccurs = false;
			return;
		}

	}
	else if( _EngageFightInTick > 0 )
	{
		--_EngageFightInTick;

		if( _EngageFightInTick == 0 )
		{
			// Actors combat engagement
			for (uint i = 0; i < _Actors.size(); i++)
			{
				if ( _Actors[i].RepeatAction != -1)
				{
					CMessage msgCombat("TEST_TOOL_ENGAGE_COMBAT");
					msgCombat.serial(_ActorsIds[i]);
					msgCombat.serial( _ActorsIds[_Actors[i].TargetIndex] );
					msgCombat.serial( _Actors[i].RepeatAction );
					msgCombat.serial( _Actors[i].RepeatActionType );
					
					CUnifiedNetwork::getInstance()->send( "EGS", msgCombat );

					// Init test variable for management
				}					
				_ActionSend[i] = 0;
				_ActionTime[i] = CTickEventHandler::getGameCycle();
			}
			_TestOccurs = true;
		}
	}
}

//----------------------------------------------------------------
// set actor start statistics
//
//----------------------------------------------------------------
void CTestingTool::setActorStartState( NLNET::CMessage& msgin )
{
	CEntityId id;
	msgin.serial( id );

	for (uint i = 0; i < _ActorsIds.size(); i++ )
	{
		if (id == _ActorsIds [i] )
		{
			msgin.serial( _ActorsBeginStats[i] );
			_ActorsStats[i].FinalHp += _ActorsBeginStats[i].Hp;
			_ActorsStats[i].FinalSta += _ActorsBeginStats[i].Sta;
			_ActorsStats[i].FinalSap += _ActorsBeginStats[i].Sap;
			break;
		}
	}
}


//----------------------------------------------------------------
// executeSentence
//----------------------------------------------------------------
void CTestingTool::executeSentence( const NLMISC::CEntityId& id, uint8 sentenceIdx, uint8 sentenceType )
{
	nlinfo("Execute sentence %d for entity %s",sentenceIdx, id.toString().c_str() );
	CMessage msgout("EXECUTE_SENTENCE");
	msgout.serial( const_cast<CEntityId &> (id) );
	msgout.serial( sentenceIdx );

	BRICK_TYPE::EBrickType type = BRICK_TYPE::COMBAT;
	if( sentenceType == 1 )
	{
		type = BRICK_TYPE::MAGIC;
	}
	msgout.serialEnum( type );

	CUnifiedNetwork::getInstance()->send( "EGS", msgout );
} // executeSentence


//----------------------------------------------------------------
// Set actor dead
//
//----------------------------------------------------------------
void CTestingTool::setActorDead( NLMISC::CEntityId& id )
{
	for (uint i = 0; i < _ActorsIds.size(); i++ )
	{
		if (id == _ActorsIds [i] )
		{
			_ActorsDeathFlags.set(i,true);
			break;
		}
	}
}

//----------------------------------------------------------------
// Log report for an actor
//
//----------------------------------------------------------------
void CTestingTool::logReport( NLMISC::CEntityId& id, SLogReport& LogReport )
{
	string out;
	SActorStatistics * Actor, * Cible;
	SSentenceStatistics * sentence;

	// Output action
	fputs( "*** Action log report ***\n", _TestSessionReport );
	
	// Who make action
	for (uint i = 0; i < _ActorsIds.size(); i++ )
	{
		if (id == _ActorsIds [i] )
		{
			char out[256];
			sprintf(out,"   Actor%d: %s perform action\n",i, _Actors[i].Actor.c_str() );
			fputs( out, _TestSessionReport );
			Actor = &_ActorsStats[i];
			Cible = &_ActorsStats[_Actors[i].TargetIndex];
			
			if( testSentence( LogReport.UsedBrick, _ActorsBeginStats[i].Sentence1 ) )
				sentence = &_ActorsSentence1[i];
			else if( testSentence( LogReport.UsedBrick, _ActorsBeginStats[i].Sentence2 ) )
				sentence = &_ActorsSentence2[i];
			else if( testSentence( LogReport.UsedBrick, _ActorsBeginStats[i].Sentence3 ) )
				sentence = &_ActorsSentence3[i];
			else
			{
				nlwarning("test willstop because the reported sentence does not belong to entity %s",_ActorsIds [i].toString().c_str());
				nlwarning("sentence received : ");
				uint j = 0;
				for (; j < LogReport.UsedBrick.size(); j++ )
					nlwarning("     -%s",LogReport.UsedBrick[j].toString());
				nlwarning("sentence memorized 1: ");
				for (j=0; j < _ActorsBeginStats[i].Sentence1.size(); j++ )
					nlwarning("     -%s",_ActorsBeginStats[i].Sentence1[j].toString());
				nlwarning("sentence memorized 2: ");
				for (j=0; j < _ActorsBeginStats[i].Sentence2.size(); j++ )
					nlwarning("     -%s",_ActorsBeginStats[i].Sentence2[j].toString());
				nlwarning("sentence memorized 3: ");
				for (j=0; j < _ActorsBeginStats[i].Sentence3.size(); j++ )
					nlwarning("     -%s",_ActorsBeginStats[i].Sentence3[j].toString());
				nlstop;
			}
			break;
		}
	}

	// Sentence used
	fputs( "   Sentence:", _TestSessionReport );
	for( vector< CSheetId >::iterator it = LogReport.UsedBrick.begin(); it != LogReport.UsedBrick.end(); ++it )
	{
		out = string(" ") + (*it).toString();
		fputs( out.c_str(), _TestSessionReport );
	}
	fputs( "\n", _TestSessionReport );

	// Sentence nb use
	sentence->NbUse++;

	// Sentence succes luck
	out = string("   Sentence Success luck: ") + NLMISC::toString( LogReport.SentenceSuccesLuck ) + string(" / 100\n");
	fputs( out.c_str(), _TestSessionReport );

	// Sentence succes or fail
	if( LogReport.SentenceSucces )
	{
		fputs( "   Sentence success !!!\n", _TestSessionReport );
	}
	else
	{
		fputs( "   Sentence fail !!!\n", _TestSessionReport );
	}

	// Sentence Hit luck
	out = string("   Sentence Hit luck: ") + NLMISC::toString( LogReport.HitLuck ) + string(" / 100\n");
	fputs( out.c_str(), _TestSessionReport );

	// Sentence hit or miss
	if( LogReport.Hit )
	{
		fputs( "   Sentence Hit !!!\n", _TestSessionReport );
		Actor->NbTouch++;
	}
	else
	{
		fputs( "   Sentence Miss !!!\n", _TestSessionReport );
		Actor->NbFailure++;
	}

	// Sentence nb success or fail
	if( LogReport.SentenceSucces && LogReport.Hit )
	{
		sentence->NbSuccess++;
	}
	else
	{
		sentence->NbFail++;
	}

	// Combined sentence success luck ( sentence success  + sentence hit )
	uint8 combinedLuck = (uint8) (100 * ( (LogReport.SentenceSuccesLuck / 100.f) * (LogReport.HitLuck / 100.f) ));
	out = string("   Combined Sentence success luck (Sentence succes + sentence hit): ") + NLMISC::toString( combinedLuck ) + string(" / 100\n");
	fputs( out.c_str(), _TestSessionReport );


	if( LogReport.SentenceSucces && LogReport.Hit )
	{
		// Critical hit ?
		if( LogReport.CriticalHit )
		{
			fputs( "   Sentence perform a critcal hit !!!\n", _TestSessionReport );
			Actor->NbCritical++;
		}
		else
		{
			fputs( "   Sentence perform a normal hit\n", _TestSessionReport );
		}

		// Max damage
		out = string("   Max Damage: ") + NLMISC::toString( LogReport.MaxDamage ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );

		// Min Damage
		out = string("   Min Damage: ") + NLMISC::toString( LogReport.MinDamage ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );

		// Sentence damage perform
		out = string("   Sentence Damage given: ") + NLMISC::toString( LogReport.GivenDamage ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		Actor->GivenDamage += LogReport.GivenDamage;
		sentence->TotalGivenDamage += (uint16) LogReport.GivenDamage;

		// Hit localization
		out = string("   Hit Localization: ") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment)LogReport.Localized ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );

		// Shield Absorption
		out = string("   Shield Absorption: ") + NLMISC::toString( LogReport.ShieldAbsorption ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		Actor->ShieldAbsorption += LogReport.ShieldAbsorption;
		sentence->TotalArmorAbsorption += (uint16) LogReport.ShieldAbsorption;

		// Armor Absorption
		out = string("   Armor Absorption: ") + NLMISC::toString( LogReport.ArmorAbsorption ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		Actor->ArmorAbsorption += LogReport.ArmorAbsorption;
		sentence->TotalShieldAbsorption += (uint16)LogReport.ShieldAbsorption;

		// Target Hp lost
		out = string("   Target Hp Lost: ") + NLMISC::toString( LogReport.HpLost ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		Actor->SendDamage += LogReport.HpLost;
		Cible->FinalHp -= LogReport.HpLost;
		sentence->TotalSendDamage += (uint16)LogReport.HpLost;

		// Special effect Resist, Start and Duration
		// ...
	}

	if( LogReport.SentenceSucces )
	{
		// Used Stamina
		out = string("   Used Stamina: ") + NLMISC::toString( LogReport.UsedStamina ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		Actor->FinalSta -= LogReport.UsedStamina;
		sentence->TotalStaminaConsume += (uint16)LogReport.UsedStamina;

		// Used Sap
		out = string("   Used Sap: ") + NLMISC::toString( LogReport.UsedSap ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
		Actor->FinalSap -= LogReport.UsedSap;
		sentence->TotalSapConsum += (uint16)LogReport.UsedSap;

		// Stamina left
		out = string("   Stamina left: ") + NLMISC::toString( LogReport.StaminaLeft ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );

		// Sap left
		out = string("   Sap left: ") + NLMISC::toString( LogReport.SapLeft ) + string("\n");
		fputs( out.c_str(), _TestSessionReport );
	}

	++_NbCombatCycle;
}

//----------------------------------------------------------------
// Sentence comparison
//
//----------------------------------------------------------------
bool CTestingTool::testSentence( const std::vector< NLMISC::CSheetId >& s1, const std::vector< NLMISC::CSheetId >& s2 )
{
	if( s1.size() != s2.size() )
	{
		return false;
	}

	for( std::vector< NLMISC::CSheetId >::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1 )
	{
		bool found = false;
		for( std::vector< NLMISC::CSheetId >::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2 )
		{
			if( *it1 == *it2 ) found = true;
		}
		if( !found ) return false;
	}
	return true;
}

//----------------------------------------------------------------
// Callback for Received actor start statistics
//
//----------------------------------------------------------------
void cbActorStartState( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	pTestingTool->setActorStartState( msgin );
}

//----------------------------------------------------------------
// Callback for Received actor dead report
//
//----------------------------------------------------------------
void cbActorDead( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId Id;
	msgin.serial( Id );

	pTestingTool->setActorDead( Id );
}

//----------------------------------------------------------------
// Callback for Received log report report
//
//----------------------------------------------------------------
void cbLogReport( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId	id;
	SLogReport	LogReport;
	msgin.serial( id );
	msgin.serial( LogReport );

	pTestingTool->logReport( id, LogReport );
}

/****************************************************************\
 ************** callback table for input message ****************
\****************************************************************/
TUnifiedCallbackItem CbArray[] = 
{
	{ "ACTOR_START_STATE",			cbActorStartState	},
	{ "TTS_REPORT_ACTOR_DEAD",		cbActorDead			},
	{ "TTS_LOG_REPORT",				cbLogReport			}
};


NLNET_SERVICE_MAIN( CTestingTool, "TTS", "testing_tool_service", 0, CbArray, "", "" );


//-----------------------------------------------
// start_test :
//
//-----------------------------------------------
NLMISC_COMMAND(startTest,"start test session","<filemame of combat test sheet>")
{
	if( args.size() == 1 )
	{
		if( pTestingTool )
		{
			pTestingTool->startTestSession( args[0] );
		}
		else
		{
			nlinfo("<start_test> Service not ready, retry in few time...");
		}
		return true;
	}
	return false;
} // dump_objects //
