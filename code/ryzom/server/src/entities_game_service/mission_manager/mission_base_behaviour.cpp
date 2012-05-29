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
#include "mission_manager/mission_base_behaviour.h"
#include "mission_manager/mission_manager.h"
#include "mission_manager/mission_queue_manager.h"
#include "egs_pd.h"
#include "egs_utils.h"
#include "creature_manager/creature_manager.h"
#include "zone_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "mission_log.h"
#include "mission_manager/mission_guild.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_team.h"
#include "player_manager/character.h"
#include "player_manager/character_encyclopedia.h"
#include "primitives_parser.h"
#include "game_share/time_weather_season/time_date_season_manager.h"

using namespace std;
using namespace NLMISC;


CVariable<sint32> MissionForcedSeason( "egs", "MissionForcedSeason", "-1=Use real season, 0=Spring, ..., 3=Winter", -1, 0, true );
CVariable<float> MissionForcedTime( "egs", "MissionForcedTime", "-1=Use real ryzom time, [0, 24[=fake time for missions (e.g. 16.5 for 16:30)", -1.0f, 0, true );


//----------------------------------------------------------------------------
void CMissionBaseBehaviour::onCreation( TAIAlias giver)
{
	_ClientIndex = 0xFF; // Must be initialized with setClientIndex later (for instance in looking in the DB)
	_Mission->setFailureIndex(0xFFFFFFFF);
	_Mission->setDescIndex(0xFFFFFFFF);
	_Mission->setGiver(giver);
	_Mission->setSeason(EGSPD::CSeason::Invalid);


	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	_ProcessingState = Normal;

	// if the mission has steps, init the active steps
	if ( !templ->Steps.empty() )
	{
		// get the highest index of among all active steps
		// First try to get the highest index among out of order steps
		// if the index is 0xFFFFFFFF, we have only 1 step
		uint32 stepCount = templ->getOutOfOrderSteps(0) + 1;
		if (  stepCount== 0 )
			stepCount = 1;

		nlassert( stepCount <= templ->Steps.size() );
		for ( uint32 i = 0; i < stepCount; i++ )
		{
			EGSPD::CActiveStepPD* step = _Mission->addToSteps(i + 1);
			EGS_PD_AST(step);
			std::vector<uint32> ret;
			templ->Steps[i]->getInitState( ret );
			const uint32 stepSize = (uint32)ret.size();
			for ( uint32 j = 0; j < stepSize; j++ )
			{
				EGSPD::CActiveStepStatePD * state = step->addToStates( j + 1 );
				EGS_PD_AST(state);
				state->setState( ret[j] );
			}
		}
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::onLoad()
{
	_ProcessingState = Normal;
	_Mission = dynamic_cast<EGSPD::CMissionPD*> ( this );
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::putInGame()
{
	nlassert( _Mission );
	if ( _Mission->getEndDate() != 0 )
	{
		CMissionManager::getInstance()->addTimedMission(_Mission);
	}
	if ( _Mission->getCrashHandlerIndex() != 0xFFFFFFFF )
	{
		CMissionManager::getInstance()->addCrashHandlingMissions(*_Mission);
	}
	if ( _Mission->getPlayerReconnectHandlerIndex() != 0xFFFFFFFF )
	{
		CMissionManager::getInstance()->addPlayerReconnectHandlingMissions(*_Mission);
	}

	// Call onActivation() on the mission template of each active step
	CMissionTemplate *missionTemplate = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	for ( std::map<uint32, EGSPD::CActiveStepPD>::iterator itStep=_Mission->getStepsBegin(); itStep!=_Mission->getStepsEnd(); ++itStep )
	{
		EGSPD::CActiveStepPD& step = (*itStep).second;
		uint32 indexInTemplate = step.getIndexInTemplate(); // or (*itStep).first?
		if ( (! missionTemplate) || (indexInTemplate > missionTemplate->Steps.size()) )
		{
			nlwarning( "Invalid active mission step %u/%hu", _Mission->getTemplateId(), (uint16)indexInTemplate );
			continue;
		}
		uint32 idx = indexInTemplate - 1;
		IMissionStepTemplate *missionStepTemplate = missionTemplate->Steps[idx];
		if ( missionStepTemplate )
		{
			list<CMissionEvent*> eventList;
			missionStepTemplate->onActivation( _Mission, idx, eventList );
		}
	}

	CMissionManager::getInstance()->checkPlaceConstraints( _Mission );
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::sendContextTexts(const TDataSetRow& user, const TDataSetRow& interlocutor, std::vector< std::pair<bool,uint32> >& textInfos)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	const NLMISC::CEntityId & giver = CAIAliasTranslator::getInstance()->getEntityId( _Mission->getGiver() );
	// send mission progress context
	for ( map<uint32, EGSPD::CActiveStepPD>::const_iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
	{
		bool gift = false;
		nlassert( uint( (*it).second.getIndexInTemplate() - 1) < templ->Steps.size() );
		uint32 txt = templ->Steps[ (*it).second.getIndexInTemplate() -1 ]->sendContextText( user,interlocutor, _Mission , gift, giver );
		if ( txt )
			textInfos.push_back(std::make_pair(gift,txt));
	}
	// send mission tp description
	for ( map<uint32,EGSPD::CMissionTeleportPD>::const_iterator it = _Mission->getTeleportsBegin(); it !=  _Mission->getTeleportsEnd(); ++it )
	{
		nlassert( uint32((*it).second.getIndex() - 1 ) < (uint32)templ->Teleports.size() );
		CMissionActionSetTeleport * tp = templ->Teleports[(*it).second.getIndex() - 1];
		nlassert( tp );

		CCreature * bot = CreatureManager.getCreature(interlocutor);
		if (!bot)
		{
			nlwarning( "<MISSIONS> invalid bot %s",interlocutor.toString().c_str() );
			continue;
		}
		if ( getTeleportBot( (*it).second.getIndex() ) != bot->getAlias() )
		{
			if ( bot->getAlias() != _Mission->getGiver() )
				continue;
		}

		TVectorParamCheck params = tp->Params;
		CMissionParser::solveEntitiesNames( params,user,giver );
		uint32 txt = STRING_MANAGER::sendStringToClient(user,tp->PhraseId,params);
		if ( txt )
			textInfos.push_back(std::make_pair(false,txt));
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::getBotChatOptions(const TDataSetRow& interlocutor, std::vector<CBotChat> & botChats)
// "gift" is a bool, as in.. "true, there is a gift" or "false, no gift".
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	for ( map<uint32, EGSPD::CActiveStepPD>::const_iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
	{
		bool gift = false;
		nlassert( (*it).second.getIndexInTemplate() - 1 < templ->Steps.size() );
		if( templ->Steps[  (*it).second.getIndexInTemplate() - 1  ]->hasBotChatOption(interlocutor, _Mission , gift ) )
		{
			CBotChat botChat;
			botChat.Gift = gift;
			botChat.StepIndex =  (*it).second.getIndexInTemplate();
			botChats.push_back(botChat);
		}
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::overrideDesc(uint32 descIndex)
{
	_Mission->setDescIndex(descIndex);
	_Mission->updateUsersJournalEntry();
}

//----------------------------------------------------------------------------
uint32 CMissionBaseBehaviour::sendDesc( const TDataSetRow & user )
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	TDataSetRow row = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( _Mission->getGiver() ) );
	return templ->sendDescText( user,row, _Mission->getDescIndex() );
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::addCompassTarget( uint32 targetId, bool isBot )
{
	CCreature * c = NULL;
	CPlace * place = NULL;
	if (isBot)
	{
		const CEntityId & id = CAIAliasTranslator::getInstance()->getEntityId( targetId );
		if (id != CEntityId::Unknown)
		{
			c = CreatureManager.getCreature( id );
			if (c == NULL)
			{
				nlwarning("<CMissionInstance addCompassTarget> Invalid entity %s", id.toString().c_str());
				return;
			}
		}
		else
		{
			nlwarning("<CMissionInstance addCompassTarget> Invalid entity alias %s", CPrimitivesParser::aliasToString(targetId).c_str());
			return;
		}
	}
	else
	{
		place = CZoneManager::getInstance().getPlaceFromId( (uint16)targetId );
		if (place == NULL)
		{
			nlwarning("<MISSIONS> Invalid place %u", targetId);
			return;
		}
	}

	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	// get a free compass index
	uint32 freeIdx = 1;
	if (templ->Tags.NoList)
	{
		bool found = false;
		for ( map<uint32,EGSPD::CMissionCompassPD>::const_iterator it = _Mission->getCompassBegin(); it != _Mission->getCompassEnd(); ++it )
		{
			const EGSPD::CMissionCompassPD & currentCompass = (*it).second;
			const EGSPD::CMissionPD * currentMission = currentCompass.getMission();
			nlassert(currentMission != NULL);
			CMissionTemplate * currentTempl = CMissionManager::getInstance()->getTemplate(currentMission->getTemplateId());
			nlassert(currentTempl != NULL);

			// return if the target is already present (for dialogs submenu only)
			if (currentTempl->Tags.NoList)
			{
				if (c != NULL && currentCompass.getBotId() == c->getAlias())
					return;
				if (place != NULL && currentCompass.getPlace() == place->getAlias())
					return;
			}

			if ( (*it).first != freeIdx )
				found = true;

			if (!found)
				freeIdx++;
		}
	}
	else
	{
		for ( map<uint32,EGSPD::CMissionCompassPD>::const_iterator it = _Mission->getCompassBegin(); it != _Mission->getCompassEnd(); ++it )
		{
			if ( (*it).first != freeIdx )
				break;
			freeIdx++;
		}
	}

	nlassert(_Mission->getCompass(freeIdx) == NULL);
	EGSPD::CMissionCompassPD * compass = _Mission->addToCompass(freeIdx);
	nlassert(compass != NULL);
	if (c != NULL)
	{
		compass->setBotId(c->getAlias());
		compass->setPlace(CAIAliasTranslator::Invalid);
	}
	else if (place != NULL)
	{
		compass->setBotId(CAIAliasTranslator::Invalid);
		compass->setPlace(place->getAlias());
	}
	else
	{
		nlstop;
	}

	std::vector<TDataSetRow> entities;
	getEntities(entities);
	if ( templ->Tags.NoList )
	{
		TVectorParamCheck params(1);
		sint32 x = 0;
		sint32 y = 0;
		string msg;

		if ( c )
		{
			x = c->getState().X();
			y = c->getState().Y();

			// Send the bot name to the client if not already done (or if the name has changed)
			//CMirrorPropValueRO<TYPE_NAME_STRING_ID> botNameId( TheDataset, c->getEntityRowId(), DSPropertyNAME_STRING_ID );
			params[0].Type = STRING_MANAGER::bot;
			params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias( c->getId()) );
			msg = "COMPASS_BOT";
		}
		else if ( place )
		{
			x = place->getCenterX();
			y = place->getCenterY();
			params[0].Identifier = place->getName();
			params[0].Type = STRING_MANAGER::place;
			msg = "COMPASS_PLACE";
		}
		for ( uint i  = 0; i < entities.size(); i++)
		{
			uint32 txt = STRING_MANAGER::sendStringToClient( entities[i],msg,params );
			PlayerManager.sendImpulseToClient( getEntityIdFromRow(entities[i]), "JOURNAL:ADD_COMPASS", x,y,txt );
		}
	}
	else
	{
		_Mission->updateUsersJournalEntry();
	}
	for ( uint i  = 0; i < entities.size(); i++)
	{
		CCharacter::sendDynamicSystemMessage(entities[i], "NEW_COMPASS");
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::removeCompassBot( TAIAlias bot )
{
	for ( map<uint32,EGSPD::CMissionCompassPD>::const_iterator it = _Mission->getCompassBegin(); it != _Mission->getCompassEnd(); ++it )
	{
		if ( (*it).second.getBotId() == bot )
		{
			_Mission->deleteFromCompass( (*it).first );
			break;
		}
	}

	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	std::vector<TDataSetRow> entities;
	getEntities(entities);
	if ( templ->Tags.NoList )
	{
		CCreature * c = CreatureManager.getCreature( CAIAliasTranslator::getInstance()->getEntityId( bot ) );
		if ( c )
		{
			TVectorParamCheck params(1);
			CMirrorPropValueRO<TYPE_NAME_STRING_ID> botNameId( TheDataset, c->getEntityRowId(), DSPropertyNAME_STRING_ID );
			params[0].Type = STRING_MANAGER::bot;
			params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias( c->getId() ) );
			for ( uint i  = 0; i < entities.size(); i++)
			{
				uint32 txt = STRING_MANAGER::sendStringToClient( entities[i],"COMPASS_BOT",params );
				PlayerManager.sendImpulseToClient( getEntityIdFromRow(entities[i]), "JOURNAL:REMOVE_COMPASS", txt );
			}
		}
	}
	else
		_Mission->updateUsersJournalEntry();
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::removeCompassPlace( uint16 placeId )
{
	CPlace *place = CZoneManager::getInstance().getPlaceFromId( placeId );
	if (place == NULL)
		return;

	for ( map<uint32,EGSPD::CMissionCompassPD>::const_iterator it = _Mission->getCompassBegin(); it != _Mission->getCompassEnd(); ++it )
	{
		if ( (*it).second.getPlace() == place->getAlias())
		{
			_Mission->deleteFromCompass( (*it).first );
			break;
		}
	}
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	std::vector<TDataSetRow> entities;
	getEntities(entities);
	if ( templ->Tags.NoList )
	{
		TVectorParamCheck params(1);
		params[0].Type = STRING_MANAGER::place;
		params[0].Identifier = place->getName();
		for ( uint i  = 0; i < entities.size(); i++)
		{
			uint32 txt = STRING_MANAGER::sendStringToClient( entities[i],"COMPASS_PLACE",params );
			PlayerManager.sendImpulseToClient( getEntityIdFromRow(entities[i]), "JOURNAL:REMOVE_COMPASS", txt );
		}
	}
	else
	{
		_Mission->updateUsersJournalEntry();
	}
}



//----------------------------------------------------------------------------
CMissionEvent::TResult CMissionBaseBehaviour::processEventForStep( const TDataSetRow & userRow, EGSPD::CActiveStepPD & step, CMissionEvent & event )
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	nlassert( uint(step.getIndexInTemplate() - 1) < templ->Steps.size() );
	IMissionStepTemplate * stepTempl = templ->Steps[ step.getIndexInTemplate() - 1 ];
	//let's test all the substeps of the step we are testing
	uint32 stateIndex = 0;
	for ( map<uint32,EGSPD::CActiveStepStatePD>::iterator it = step.getStatesBegin(); it != step.getStatesEnd(); ++it )
	{
		// we only test uncomplete steps
		if ( (*it).second.getState() != 0 )
		{
			CEntityId id = CAIAliasTranslator::getInstance()->getEntityId( _Mission->getGiver() );
			uint resultInt = stepTempl->processEvent( userRow, event, stateIndex, TheDataset.getDataSetRow(id) );

			if ( event.Type == CMissionEvent::Debug )
				resultInt = 1;
			if ( resultInt )
			{
				// the event was processed : update the step state
				if ( (*it).second.getState() > resultInt )
					(*it).second.setState( (*it).second.getState() - resultInt );
				else
					(*it).second.setState( 0 );

				// if the step is not finished, we bail out
				map<uint32,EGSPD::CActiveStepStatePD>::iterator itState = step.getStatesBegin();
				for ( ; itState != step.getStatesEnd(); ++itState )
				{
					if ( (*itState).second.getState() != 0 )
					{
						break;
					}
				}
				if ( itState != step.getStatesEnd() )
					return CMissionEvent::Modified;

				return CMissionEvent::StepEnds;
			}
		}
		stateIndex++;
	}
	return CMissionEvent::Nothing;
}


//----------------------------------------------------------------------------
bool CMissionBaseBehaviour::checkConstraints( bool logForProcessingEvent, const std::string dbgPrefix ) const
{
	const CRyzomTime& ryzomTime = CTimeDateSeasonManager::getRyzomTimeReference();
	float time = (MissionForcedTime.get()==-1.0f) ? ryzomTime.getRyzomTime() : MissionForcedTime.get();
	if ( _Mission->getHourLowerBound() != 0.0f || _Mission->getHourUpperBound() != 0.0f )
	{
		bool timeOk = true;
		if (_Mission->getHourUpperBound() < _Mission->getHourLowerBound())
		{
			if( time > _Mission->getHourUpperBound() && time < _Mission->getHourLowerBound() )
				timeOk = false;
		}
		else
		{
			if( time > _Mission->getHourUpperBound() || time < _Mission->getHourLowerBound() )
				timeOk = false;
		}
		if (!timeOk)
		{
			if ( logForProcessingEvent )
				MISDBG("%s failed : bad time bounds", dbgPrefix.c_str() ); // can flood if verbose is on
			return false;
		}
	}
	CRyzomTime::ESeason season = (MissionForcedSeason.get()==-1) ? ryzomTime.getRyzomSeason() : (CRyzomTime::ESeason)(uint)MissionForcedSeason.get();
	if ( _Mission->getSeason() != EGSPD::CSeason::Invalid && season != _Mission->getSeason() )
	{
		if ( logForProcessingEvent )
			MISDBG("%s failed: bad season", dbgPrefix.c_str() ); // can flood if verbose is on
		return false;
	}
	return true;
}


//----------------------------------------------------------------------------
CMissionEvent::TResult CMissionBaseBehaviour::processEvent( const TDataSetRow & userRow, std::list< CMissionEvent* > & eventList,uint32 stepIndex )
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	CCharacter *user = PlayerManager.getChar(userRow);
	string sDebugPrefix;
	if (user != NULL)
		sDebugPrefix += "user:" + user->getId().toString() + " ";
	sDebugPrefix += "miss:" + CPrimitivesParser::aliasToString(_Mission->getTemplateId());
	sDebugPrefix += ",'" + templ->getMissionName() + "' processEvent :";

	if ( _Mission->getFinished() )
		return CMissionEvent::Nothing;
	//ignore events for empty missions
	if ( templ->Steps.empty() )
		return CMissionEvent::Nothing;
	// check constraints
	if ( ! checkConstraints( true, sDebugPrefix ) )
		CMissionEvent::Nothing;
	// get the event to test ( first of the list )
	nlassert( !eventList.empty() );
	CMissionEvent & event = *( *(eventList.begin()) );
//	MISDBG("mission %s receiving event %u", CPrimitivesParser::aliasToString(templ->Alias).c_str(), event.Type );

	EGSPD::CActiveStepPD * currentStep = NULL;
	CMissionEvent::TResult resultEnum = CMissionEvent::Nothing;
	if ( stepIndex != 0xFFFFFFFF )
	{
		EGSPD::CActiveStepPD * step = _Mission->getSteps( stepIndex );
		nlassert( step ); // TODO: fails if two members of a team with a 'give item' team mission give an item at the exact same time - from CTeam::processTeamMissionStepEvent() from CCharacter::acceptExchange() from cbClientValidateMissionGift()
		resultEnum = processEventForStep(userRow, *step, event);
		if ( resultEnum != CMissionEvent::StepEnds )
			return resultEnum;
		currentStep = step;
	}
	else
	{
		for ( map<uint32, EGSPD::CActiveStepPD>::iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
		{
			CMissionEvent::TResult resultTmp = processEventForStep( userRow, (*it).second, event );
			if ( resultTmp != CMissionEvent::Nothing )
			{
				if ( resultTmp != CMissionEvent::StepEnds )
					return resultTmp;
				else
					resultEnum = CMissionEvent::StepEnds;
				currentStep = &( (*it).second );
				break;
			}
		}
	}
	if ( resultEnum == CMissionEvent::Nothing )
		return resultEnum;

	nlassert( currentStep );
	uint32 currentStepIdx = currentStep->getIndexInTemplate() - 1;
	nlassert(currentStepIdx < templ->Steps.size() );

	// if the step is an "any" step, we end all the other "any" active steps
	if ( templ->Steps[ currentStepIdx ]->isAny() )
	{
		while( _Mission->getStepsBegin() != _Mission->getStepsEnd() )
		{
			uint32 indexInTemplate = ( *_Mission->getStepsBegin() ).second.getIndexInTemplate();
			if ( indexInTemplate - 1 != currentStepIdx )
			{
				// Call the cancel handler on the other 'any' steps that not need be successfully processed
				templ->Steps[ currentStepIdx ]->onCancelStepAny( _Mission, indexInTemplate - 1 );
			}
			_Mission->addToStepsDone( indexInTemplate );
			_Mission->deleteFromSteps( ( *_Mission->getStepsBegin() ).first );
		}
	}
	// otherwise, we only end the current step. We must keep the step sorted for the mission journal
	else
	{
		_Mission->addToStepsDone( currentStep->getIndexInTemplate() );
		_Mission->deleteFromSteps( currentStep->getIndexInTemplate() );
	}

	// as the step is ended, we can launch its end actions
	const std::vector<IMissionAction*>&	actions = templ->Steps[ currentStepIdx ]->getActions();
	for ( uint it = 0; _ProcessingState == Normal && it < actions.size(); ++it )
		actions[it]->launch(_Mission, eventList);

	// if there is no more active steps, we can process out of order actions and get the next active steps
	// if we encountered a jump/fail/end previously, we dont launch those actions
	if ( _Mission->getStepsBegin() == _Mission->getStepsEnd() && _ProcessingState == Normal)
	{
		// launch OOO actions if any
		uint32 oooIndex = templ->Steps[ currentStepIdx ]->getOOOStepIndex();
		if ( oooIndex < templ->OOOActions.size() )
		{
			for ( uint it = 0; _ProcessingState == Normal && it < templ->OOOActions[oooIndex].size(); ++it )
				templ->OOOActions[oooIndex][it]->launch(_Mission,eventList);
		}

		// build the active steps ( only if we did not reach an end/fail/jump )
		if ( _ProcessingState == Normal )
		{
			uint32 firstActiveStep = 0xFFFFFFFF;
			if ( oooIndex != 0xFFFFFFFF )
			{
				for ( sint i = (sint)currentStepIdx; i >= 0; i-- )
				{
					if ( templ->getOutOfOrderSteps( i ) != 0xFFFFFFFF )
					{
						firstActiveStep = templ->getOutOfOrderSteps( i ) + 1;
						break;
					}
				}
			}
			else
				firstActiveStep = currentStepIdx + 1;

			nlassert( firstActiveStep < templ->Steps.size() );
			uint32 lastActiveStep = templ->getOutOfOrderSteps( firstActiveStep );
			uint32 stepCount = ( lastActiveStep == 0xFFFFFFFF )? 1 : lastActiveStep -  firstActiveStep + 1;

			for ( uint i = 0; i < stepCount; i++ )
			{
				EGSPD::CActiveStepPD * step = _Mission->addToSteps(firstActiveStep +  i + 1);
				EGS_PD_AST(step);

				std::vector<uint32> ret;
				templ->Steps[step->getIndexInTemplate()-1]->getInitState( ret );
				const uint32 stepSize = (uint32)ret.size();
				for ( uint j = 0; j < stepSize; j++ )
				{
					EGSPD::CActiveStepStatePD * state = step->addToStates( j + 1 );
					EGS_PD_AST(state);
					state->setState( ret[j] );
				}
				templ->Steps[step->getIndexInTemplate()-1]->onActivation( _Mission,step->getIndexInTemplate() - 1 ,eventList );
			}
		}
	}

	// check if we reached the end of the mission
	if ( _ProcessingState == Complete )
	{
		MISDBG("%s ok, step %u done -> mission completed", sDebugPrefix.c_str(), currentStep->getIndexInTemplate() );
		templ->AlreadyDone = true;
		if(!user->isShopingListInProgress())
			user->endBotChat();
		return CMissionEvent::MissionEnds;
	}

	if ( _ProcessingState == Failed )
	{
		MISDBG("%s ok, step %u done -> mission failed", sDebugPrefix.c_str(), currentStep->getIndexInTemplate() );
		_ProcessingState = Normal;
		onFailure( true );
		if(!user->isShopingListInProgress())
			user->endBotChat();
		return CMissionEvent::MissionFailed;
	}
	else
	if ( _ProcessingState == ActionFailed )
	{
		MISDBG("%s ok, step %u done -> mission action failed", sDebugPrefix.c_str(), currentStep->getIndexInTemplate() );
		_ProcessingState = Normal;
		onFailure( false );
		return CMissionEvent::Nothing;
	}
	else
	{
		MISDBG("%s ok, step %u done -> mission step ended, mission still there", sDebugPrefix.c_str(), currentStep->getIndexInTemplate() );
		_ProcessingState = Normal;
		return CMissionEvent::StepEnds;
	}
	return resultEnum;
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::jump( uint32 step, uint32 action,std::list< CMissionEvent * > & eventList)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	// if we jump before first step
	if ( step == 0xFFFFFFFF )
	{
		// launch the actions
		for ( uint i = action; i < templ->InitialActions.size(); i++ )
		{
			templ->InitialActions[i]->launch(_Mission,eventList);
			if ( _ProcessingState  != Normal && _ProcessingState  != Init )
				return;
		}
	}
	// if we are during a step
	else
	{
		if ( step >= templ->Steps.size() )
		{
			nlwarning("<CMissionInstance::jump> Serious error : jump step = %u >= step count (%u)",step, templ->Steps.size() );
			return;
		}

		// if we are in out of order actions, launch the appropriate ones
		if ( action >= templ->Steps[step]->getActions().size() )
		{
			// get the index of the targeted bloc
			uint32 oooIndex = templ->Steps[step]->getOOOStepIndex();
			// if they are actions in this out of order bloc, we launch them
			if( oooIndex < templ->OOOActions.size() )
			{
				action -= (uint32)templ->Steps[step]->getActions().size();
				for ( uint i = action; i < templ->OOOActions[oooIndex].size(); i++ )
				{
					templ->OOOActions[oooIndex][i]->launch(_Mission,eventList);
					if ( _ProcessingState  != Normal && _ProcessingState != Init )
					{
						return;
					}
				}
			}
		}
		// we are in a specific step action. Launch them
		else for ( uint i = action; i < templ->Steps[step]->getActions().size(); i++ )
		{
			templ->Steps[step]->getActions()[i]->launch( _Mission, eventList );
			if ( _ProcessingState  != Normal && _ProcessingState != Init )
				return;
		}
	}

	// Here, event are launched and no nested jumps encountered, so we have to set the active steps
	while( _Mission->getStepsBegin() != _Mission->getStepsEnd() )
		_Mission->deleteFromSteps( ( *_Mission->getStepsBegin() ).first );

	uint32 firstActiveStep = step + 1;
	nlassert( firstActiveStep < templ->Steps.size() );
	uint32 lastActiveStep = templ->getOutOfOrderSteps( firstActiveStep );
	uint32 stepCount = ( lastActiveStep == 0xFFFFFFFF )? 1 : lastActiveStep -  firstActiveStep + 1;

	for ( uint i = 0; i < stepCount; i++ )
	{
		EGSPD::CActiveStepPD * step = _Mission->addToSteps(firstActiveStep + i + 1);
		EGS_PD_AST(step);

		std::vector<uint32> ret;
		templ->Steps[step->getIndexInTemplate()-1]->getInitState( ret );
		const uint32 stepSize = (uint32)ret.size();
		for ( uint j = 0; j < stepSize; j++ )
		{
			EGSPD::CActiveStepStatePD * state = step->addToStates( j + 1 );
			EGS_PD_AST(state);
			state->setState( ret[j] );
		}
		if( _ProcessingState != Init )
			templ->Steps[step->getIndexInTemplate()-1]->onActivation( _Mission,step->getIndexInTemplate() - 1 ,eventList);
	}

	_ProcessingState = InJump;
	_Mission->updateUsersJournalEntry();
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::setTimer( NLMISC::TGameCycle cycle )
{
	// cycle = 0 -> remove the timer
	CMissionManager::getInstance()->removeTimedMission(_Mission);
	if ( cycle == 0 )
	{
		_Mission->setEndDate(0);
		_Mission->setBeginDate(0);
	}
	else
	{
		_Mission->setEndDate ( cycle + CTickEventHandler::getGameCycle() );
		_Mission->setBeginDate( CTickEventHandler::getGameCycle() );
		CMissionManager::getInstance()->addTimedMission(_Mission);
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::activateInitialSteps(std::list< CMissionEvent * > & eventList)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	for ( map<uint32, EGSPD::CActiveStepPD>::const_iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
	{
		const uint32 idx = (*it).second.getIndexInTemplate()-1;
		if ( idx < templ->Steps.size() && templ->Steps[ idx ] )
			templ->Steps[ idx ]->onActivation( _Mission,idx,eventList );
	}
	_ProcessingState = Normal;
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::setSuccessFlag()
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	_Mission->setFinished(true);
	_Mission->setMissionSuccess(true);
	while( _Mission->getStepsBegin() != _Mission->getStepsEnd() )
		_Mission->deleteFromSteps( ( *_Mission->getStepsBegin() ).first );

	templ->LastSuccessDate = CTickEventHandler::getGameCycle();
	CMissionManager::getInstance()->deInstanciateMission(_Mission);
	if (templ->Tags.NoList == false) // If the mission is in the journal, update it
		_Mission->updateUsersJournalEntry();

	// update the encyclopedia if needed
	updateEncyclopedia();
}


//----------------------------------------------------------------------------
void CMissionBaseBehaviour::setFailureFlag()
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	_Mission->setFinished(true);
	_Mission->setMissionSuccess(false);
	while( _Mission->getStepsBegin() != _Mission->getStepsEnd() )
		_Mission->deleteFromSteps( ( *_Mission->getStepsBegin() ).first );

	CMissionManager::getInstance()->deInstanciateMission(_Mission);
	if (templ->Tags.NoList == false) // If the mission is in the journal, update it
		_Mission->updateUsersJournalEntry();
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::onFailure(bool ignoreJumps,bool sendMessage)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	// if mission was in a waiting queue, removed player from queue manager
	if (_Mission->getWaitingQueueId() != 0)
	{
		CCharacter * user = getMainEntity();
		if (user)
		{
			CMissionQueueManager::getInstance()->removePlayerFromQueue(user->getId(), _Mission->getWaitingQueueId());
		}
	}

	// remove the mission from the timed mission container
	// we have to do that because jumps can prevent the mission from being deleted
	if ( _Mission->getEndDate() )
	{
		CMissionManager::getInstance()->removeTimedMission( _Mission );
		_Mission->setEndDate(0);
		_Mission->setBeginDate(0);
	}
	if ( _Mission->getCrashHandlerIndex() != 0xFFFFFFFF )
	{
		CMissionManager::getInstance()->removeCrashHandlingMissions(*_Mission);
	}
	if ( _Mission->getPlayerReconnectHandlerIndex() != 0xFFFFFFFF )
	{
		CMissionManager::getInstance()->removePlayerReconnectHandlingMissions(*_Mission);
	}

	// Call onFailure on template steps
	for ( std::map<uint32, EGSPD::CActiveStepPD>::iterator itStep=_Mission->getStepsBegin(); itStep!=_Mission->getStepsEnd(); ++itStep )
	{
		EGSPD::CActiveStepPD& step = (*itStep).second;
		uint32 indexInTemplate = step.getIndexInTemplate(); // or (*itStep).first?
		if ( indexInTemplate > templ->Steps.size() )
		{
			nlwarning( "Invalid active mission step %u/%hu", _Mission->getTemplateId(), (uint16)indexInTemplate );
			continue;
		}
		uint32 idx = indexInTemplate - 1;
		IMissionStepTemplate *missionStepTemplate = templ->Steps[idx];
		if ( missionStepTemplate )
		{
			missionStepTemplate->onFailure( _Mission, idx );
		}
	}

	CMissionManager::getInstance()->removeFromPlaceConstraints(_Mission);

	vector<TAIAlias> groups;
	MISDBG("miss:%s onFailure : mission failed", CPrimitivesParser::aliasToString(templ->Alias).c_str());
	uint32 index = _Mission->getFailureIndex();

	std::list< CMissionEvent* > eventList;
	if( index < templ->FailureActions.size() )
	{
		for ( uint i = 0; getProcessingState() == CMissionBaseBehaviour::Normal && i< templ->FailureActions[index].size() ; i++ )
		{
			if ( !ignoreJumps || dynamic_cast<CMissionActionJump*>( templ->FailureActions[index][i] ) == NULL )
				templ->FailureActions[index][i]->launch(_Mission,eventList);
		}
	}
	if ( getProcessingState() == CMissionBaseBehaviour::Normal )
	{
		if ( templ->Type == MISSION_DESC::Guild )
		{
			CMissionGuild * mission = dynamic_cast<CMissionGuild *>(_Mission);
			if ( mission )
			{
				while( !eventList.empty() )
				{
					/// todo guild missions
					//guild->processGuildSpecificEvent( *(eventList.front()) );
					delete eventList.front();
					eventList.pop_front();
				}
			}
		}
		else
		{
			CCharacter * user = getMainEntity();
			if (user)
			{
				while( !eventList.empty() )
				{
					user->processMissionEvent( *eventList.front() );
					delete eventList.front();
					eventList.pop_front();
				}
			}
		}
	}

	_Mission->setFailureIndex( 0xFFFFFFFF );
	// special cleanup before jumping
	if ( getProcessingState() != CMissionBaseBehaviour::Normal )
	{
		/*
		if ( getEndDate() )
		{
		CMissionManager::getInstance()->removeTimedMission(this);
		}
		if ( getMonoEndDate() )
		{
		CMissionManager::getInstance()->removeMonoMission(this);
		}
			*/

		templ->getEscortGroups(groups);
		MISDBG("miss:%s onFailure : clean up escort group", CPrimitivesParser::aliasToString(templ->Alias).c_str());
		if ( !groups.empty() )
		{
			for ( uint i = 0; i <groups.size(); i++)
				CMissionManager::getInstance()->unregisterEscort( groups[i], getMainEntity()->getId() );
		}
	}
	else
	{
		// stop children missions
		stopChildren();
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::addTeleport( uint32 tpIdx )
{
	_Mission->addToTeleports( tpIdx );
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::removeTeleport( uint32 tpIdx )
{
	_Mission->deleteFromTeleports( tpIdx );
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::teleport(CCharacter * user,uint tpIndex)
{
	nlassert(user);
	EGSPD::CMissionTeleportPD * tp = _Mission->getTeleports( tpIndex );
	if ( tp == NULL )
	{
		nlwarning("Invalid TP index %u",tpIndex);
		return;
	}
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	nlassert( tpIndex - 1 < templ->Teleports.size() );
	nlassert( templ->Teleports[tpIndex - 1] );

	const CTpSpawnZone* tpZone = CZoneManager::getInstance().getTpSpawnZone( templ->Teleports[tpIndex-1]->DestinationIdx );
	if ( ! tpZone )
	{
		nlwarning("invalid tp index %u for user %s",tpIndex,user->getId().toString().c_str() );
		return;
	}
	sint32 x,y,z;
	float heading;
	tpZone->getRandomPoint(x,y,z,heading);
	user->forbidNearPetTp();
	user->tpWanted(x,y,z,true,heading);
	if ( templ->Teleports[tpIndex - 1]->Once )
		removeTeleport(tpIndex);
}

//----------------------------------------------------------------------------
TAIAlias CMissionBaseBehaviour::getTeleportBot(uint32 tpIndex)
{
	EGSPD::CMissionTeleportPD * tp = _Mission->getTeleports( tpIndex );
	if ( tp == NULL )
	{
		nlwarning("Invalid TP index %u",tpIndex);
		return CAIAliasTranslator::Invalid;
	}
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	nlassert( uint( tpIndex - 1 ) < templ->Teleports.size() );
	nlassert( templ->Teleports[tpIndex - 1] );
	if ( templ->Teleports[tpIndex -1]->Bot != CAIAliasTranslator::Invalid )
		return templ->Teleports[tpIndex -1]->Bot;
	return _Mission->getGiver();
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::checkEscortFailure(bool groupWiped)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	for ( map<uint32, EGSPD::CActiveStepPD>::const_iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
	{
		nlassert( uint( (*it).second.getIndexInTemplate() - 1 ) < templ->Steps.size() );
		if ( templ->Steps[ (*it).second.getIndexInTemplate() - 1 ]->checkEscortFailure( groupWiped ) )
		{
			onFailure(false);
			return;
		}
	}
}

//----------------------------------------------------------------------------
bool CMissionBaseBehaviour::itemGiftDone( CCharacter * user,const std::vector< CGameItemPtr > & itemsGiven,uint32 stepIndex, std::vector<uint32> & result )
{
	nlassert(user);
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	EGSPD::CActiveStepPD* step = _Mission->getSteps(stepIndex);
	if ( step )
	{
		return templ->Steps[step->getIndexInTemplate() - 1]->itemGiftDone( *user, itemsGiven, *step ,result );
	}
	return false;
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::updateUserJournalEntry( CCharacter & user, const std::string & dbPrefix )
{
	if (dbPrefix == "GROUP:")
		_updateUserJournalEntry( user, CBankAccessor_PLR::getGROUP().getMISSIONS());
	else
		_updateUserJournalEntry( user, CBankAccessor_PLR::getMISSIONS());
}

template <class DBType>
void CMissionBaseBehaviour::_updateUserJournalEntry( CCharacter & user, DBType &missionDb)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	if( !templ )
	{
		nlwarning("<MISSIONS>invalid template %u",_Mission->getTemplateId());
		return;
	}
	if ( templ->Tags.NoList )
		return;
	if (_ClientIndex == 0xFF) // There must be a valid journal entry !!!
		return;

	user.updateTarget();

	const CEntityId & giverId = CAIAliasTranslator::getInstance()->getEntityId(_Mission->getGiver());
	if ( giverId == CEntityId::Unknown )
	{
		nlwarning("<MISSIONS> Invalid alias %s ( not spawned? )", CPrimitivesParser::aliasToString(_Mission->getGiver()).c_str());
		return;
	}
	TDataSetRow giverRow = TheDataset.getDataSetRow( giverId );

	typename DBType::TArray	&missionEntry = missionDb.getArray(_ClientIndex);

	missionEntry.setTYPE(user._PropertyDatabase, templ->Type);
	missionEntry.setICON(user._PropertyDatabase, templ->Icon);

	if (missionEntry.getTITLE(user._PropertyDatabase) == 0) // new missions are always written in a free slot
	{
		// Make TITLE never change for a mission in progress, only write it once (sendTitleText() generates a new string id every time it is called)
		missionEntry.setTITLE(user._PropertyDatabase, templ->sendTitleText( user.getEntityRowId(),giverRow));
	}
	missionEntry.setDETAIL_TEXT(user._PropertyDatabase, _Mission->sendDesc( user.getEntityRowId() ));

	if (_Mission->getCriticalPartEndDate() != 0 && (_Mission->getEndDate() == 0 || _Mission->getCriticalPartEndDate() < _Mission->getEndDate()) )
		missionEntry.setEND_DATE(user._PropertyDatabase, _Mission->getCriticalPartEndDate());
	else
		missionEntry.setEND_DATE(user._PropertyDatabase, _Mission->getEndDate());

	missionEntry.setBEGIN_DATE(user._PropertyDatabase, _Mission->getBeginDate());
	if (!_Mission->getFinished())
	{
		missionEntry.setFINISHED(user._PropertyDatabase, 0);
	}
	else // Mission is finished
	{
		if (_Mission->getMissionSuccess())
			missionEntry.setFINISHED(user._PropertyDatabase, 1);
		else // Mission is finished but failed
			missionEntry.setFINISHED(user._PropertyDatabase, 2);
	}
	missionEntry.setABANDONNABLE(user._PropertyDatabase, !templ->Tags.NonAbandonnable);

	if ( _Mission->getStepsBegin() == _Mission->getStepsEnd() )
		missionEntry.setOR_STEPS(user._PropertyDatabase, 0);
	else
		missionEntry.setOR_STEPS(user._PropertyDatabase, templ->Steps[ (*_Mission->getStepsBegin()).second.getIndexInTemplate() - 1]->isAny());

	uint stepIdx = 0;
	uint32 currentOOO = 0xFFFFFFFF;
	bool RPTxtOOOWritten = false;
	// write active steps
	for ( map<uint32,EGSPD::CActiveStepPD>::iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
	{
		if( uint( (*it).second.getIndexInTemplate() - 1 ) >= templ->Steps.size() )
		{
			nlwarning("<MISSIONS> '%s' Invalid step index %u count is %u", user.getId().toString().c_str(), uint( (*it).second.getIndexInTemplate() - 1 ) , templ->Steps.size() );
			return;
		}
		IMissionStepTemplate * step = templ->Steps[ (*it).second.getIndexInTemplate() - 1 ];

		if ( step == NULL )
		{
			nlwarning("<MISSIONS> '%s' Invalid step index %u count is %u. The step is NULL", user.getId().toString().c_str(), uint( (*it).second.getIndexInTemplate() - 1 ) , templ->Steps.size() );
			return;
		}

		bool display = step->isDisplayed();
		// if we are in an overriden OOO,
		if ( step->isInOverridenOOO() )
		{
			// only display the first active step
			if ( it != _Mission->getStepsBegin() )
				display = false;
			// if step is not the first step of the OOO in the mission template, the step to be display is in fact the first one
			else if ( templ->getOutOfOrderSteps((*it).first - 1) == 0xFFFFFFFF )
			{
				uint firstOOOStepIndex = ~0;
				const uint size = (uint)templ->OutOfOrderSteps.size();
				for ( uint i = 0; i < size; i++ )
				{
					if ( templ->OutOfOrderSteps[i].first <= (*it).first - 1 &&  (*it).first - 1 <= templ->OutOfOrderSteps[i].second  )
					{
						firstOOOStepIndex = templ->OutOfOrderSteps[i].first;
						break;
					}
				}
				if( firstOOOStepIndex >= templ->Steps.size() )
				{
					nlwarning("<MISSIONS> '%s' Invalid first OOO Step Index %u count is %u", user.getId().toString().c_str(), firstOOOStepIndex , templ->Steps.size() );
					return;
				}
				step = templ->Steps[ firstOOOStepIndex ];
				if ( step == NULL )
				{
					nlwarning("<MISSIONS> '%s' Invalid first OOO Step Index %u count is %u. The step is NULL", user.getId().toString().c_str(), firstOOOStepIndex, templ->Steps.size() );
					return;
				}
				display = true;
			}
		}
		else
			currentOOO = 0xFFFFFFFF;

		// Send NPCs related to the goals (for icon)
		TAIAlias involvedBot = CAIAliasTranslator::Invalid;
		if ( display && step->isIconDisplayedOnStepNPC() )
		{
//			CCreature *bot = CreatureManager.getCreature(giverRow);
//			if (bot && bot->isMissionStepIconDisplayable())
//			{
				bool invalidIsGiver;
				involvedBot = step->getInvolvedBot(invalidIsGiver);
				if ((involvedBot == CAIAliasTranslator::Invalid) && invalidIsGiver)
					involvedBot = _Mission->getGiver();
//			}
		}
#ifdef NL_DEBUG
		nlassert(CAIAliasTranslator::Invalid == 0); // nlctassert would not compile (VC6)
#endif

		missionEntry.getGOALS().getArray(stepIdx).setNPC_ALIAS(user._PropertyDatabase, involvedBot);

		// do not display step that have the display flag set to false
		// if the step is in an overridden OOO. Only display the first step
		if ( display )
		{
			vector<uint32> states;
			for ( map<uint32,EGSPD::CActiveStepStatePD>::iterator itState = (*it).second.getStatesBegin(); itState != (*it).second.getStatesEnd(); ++itState )
			{
				states.push_back( (*itState).second.getState() );
			}


			// in case of OOO, RP text is displayed only one time
			if( step->getOOOStepIndex() == 0xFFFFFFFF )
			{
				RPTxtOOOWritten = false;
			}
			// Send RP text first
			uint32 stepTxt = step->sendRpStepText( &user,states, giverId );
			if( stepTxt!=0 && !RPTxtOOOWritten )
			{
				missionEntry.getGOALS().getArray(stepIdx).setTEXT(user._PropertyDatabase, stepTxt);
				stepIdx++;
				if( step->getOOOStepIndex() != 0xFFFFFFFF )
				{
					RPTxtOOOWritten = true;
				}
			}

			// Send standard (or overriden) step texts
			stepTxt = step->sendStepText( &user,states, giverId );
			if( stepTxt!=0 )
			{
				missionEntry.getGOALS().getArray(stepIdx).setTEXT(user._PropertyDatabase, stepTxt);
				stepIdx++;
			}
		}
		currentOOO = step->getOOOStepIndex();
	}


	// write done steps
	bool DontDisplayStepAnyInHisto = false;
	uint stepDoneIdx = 0;
	uint32 doneOOO = 0xFFFFFFFF;
	for ( map<uint32,EGSPD::CDoneStepPD>::iterator it = _Mission->getStepsDoneBegin(); it != _Mission->getStepsDoneEnd(); ++it )
	{
		if ( ( uint( (*it).second.getIndex() - 1) >=  templ->Steps.size() ) )
		{
			nlwarning("<MISSIONS> '%s' Invalid step index %u count is %u", user.getId().toString().c_str(), uint( (*it).second.getIndex() - 1 ) , templ->Steps.size() );
			return;
		}
		IMissionStepTemplate * step = templ->Steps[ (*it).second.getIndex() -1];

		if ( step == NULL )
		{
			nlwarning("<MISSIONS> '%s' Invalid step index %u count is %u. step is NULL", user.getId().toString().c_str(), uint( (*it).second.getIndex() - 1 ) , templ->Steps.size() );
			return;
		}
		bool display = false;
		// special case if the step is in an OOO block
		if ( step->getOOOStepIndex() != 0xFFFFFFFF )
		{
			/// if the step is in an overridden block
			if ( step->isInOverridenOOO() || step->isThereRoleplayText() )
			{
				// if the step is part of an active OOO block, do no display
				if ( step->getOOOStepIndex() != currentOOO )
				{
					// otherwise, only display the first step of the ooo
					display =  ( doneOOO != step->getOOOStepIndex() );
				}
			}
			// follow standard rules otherwise
			else
				display = step->isDisplayed();
			doneOOO = step->getOOOStepIndex();
		}
		else
			display = step->isDisplayed();

		if ( display && stepDoneIdx < NB_HISTO_PER_MISSION )
		{
			vector<uint32> states;
			step->getInitState( states );

			// Send RP text first
			uint32 stepTxt = step->sendRpStepText( &user,states, giverId );
			if(stepTxt!=0)
			{
				missionEntry.getHISTO().getArray(stepDoneIdx).setTEXT(user._PropertyDatabase, stepTxt);
				stepDoneIdx++;
				// roleplay text replaces all step text of step any
				if( step->isAny() )
					DontDisplayStepAnyInHisto = true;
			}
			else
			{
				if( !(step->isAny() && DontDisplayStepAnyInHisto) )
				{
					if( !step->isAny() )
						DontDisplayStepAnyInHisto = false;
					// Send standard (or overriden) step texts
					stepTxt = step->sendStepText(&user,states, giverId );
					missionEntry.getHISTO().getArray(stepDoneIdx).setTEXT(user._PropertyDatabase, stepTxt);
					stepDoneIdx++;
				}
			}
		}
	}

	for ( uint i = stepIdx; i < NB_STEP_PER_MISSION; i++ )
	{
		missionEntry.getGOALS().getArray(i).setTEXT(user._PropertyDatabase, 0);
		missionEntry.getGOALS().getArray(i).setNPC_ALIAS(user._PropertyDatabase, CAIAliasTranslator::Invalid);
	}
	for ( uint i = stepDoneIdx; i < NB_HISTO_PER_MISSION; i++ )
	{
		missionEntry.getHISTO().getArray(i).setTEXT(user._PropertyDatabase, 0);
	}

//	uint nbEntry = updateCompass(user,dbPrefix);
	uint nbEntry = _updateCompass(user, missionDb);
	for ( uint i = nbEntry ; i < NB_JOURNAL_COORDS; i++ )
	{
		missionEntry.getTARGET(i).setX(user._PropertyDatabase, 0);
		missionEntry.getTARGET(i).setY(user._PropertyDatabase, 0);
		missionEntry.getTARGET(i).setTITLE(user._PropertyDatabase, 0);
	}
}

//----------------------------------------------------------------------------
void CMissionBaseBehaviour::updateEncyclopedia()
{
	// update encylopedia when a mission ended successfully
	CMissionManager *pMM = CMissionManager::getInstance();
	CMissionTemplate *pMT = pMM->getTemplate(_Mission->getTemplateId());
	if (pMT != NULL)
	{
		if ((pMT->EncycloAlbum != -1) && (pMT->EncycloThema != -1) && (pMT->EncycloTask != -1))
		{
			vector<TDataSetRow> entities;
			_Mission->getEntities(entities);
			for (uint32 i = 0; i < entities.size(); ++i)
			{
				CCharacter *pChar = PlayerManager.getChar(entities[i]);
				if (pChar != NULL)
					pChar->getEncyclopedia().updateTask(pMT->EncycloAlbum, pMT->EncycloThema, pMT->EncycloTask, 2);
			}
		}
	}
}

//----------------------------------------------------------------------------
uint CMissionBaseBehaviour::updateCompass(CCharacter & user, const std::string & dbPrefix)
{
	if (dbPrefix == "GROUP:")
		return _updateCompass(user, CBankAccessor_PLR::getGROUP().getMISSIONS());
	else
		return _updateCompass(user, CBankAccessor_PLR::getMISSIONS());
}


//----------------------------------------------------------------------------
template <class DBType>
uint CMissionBaseBehaviour::_updateCompass(CCharacter & user, DBType &missionDb)
{
	uint compassIdx = 0;
	for ( map<uint32,EGSPD::CMissionCompassPD>::iterator it = _Mission->getCompassBegin(); it != _Mission->getCompassEnd(); ++it )
	{
		TVectorParamCheck params(1);
		sint32 x = 0;
		sint32 y = 0;
		bool updateTitle = false;
		uint32 newTitleId;

		if ( (*it).second.getPlace() == CAIAliasTranslator::Invalid )
		{
			CCreature * c = CreatureManager.getCreature( CAIAliasTranslator::getInstance()->getEntityId( (*it).second.getBotId() ) );
			if ( c )
			{
				x = c->getState().X();
				y = c->getState().Y();

				// Send the bot name to the client if not already done (or if the name has changed)
				CMirrorPropValueRO<TYPE_NAME_STRING_ID> botNameId( TheDataset, c->getEntityRowId(), DSPropertyNAME_STRING_ID );
				if ( botNameId() != (*it).second.NameStringId )
				{
					params[0].Type = STRING_MANAGER::bot;
					params[0].setEIdAIAlias( CAIAliasTranslator::getInstance()->getEntityId( (*it).second.getBotId() ),  (*it).second.getBotId()  );
					updateTitle = true;
					newTitleId = STRING_MANAGER::sendStringToClient( user.getEntityRowId(),"COMPASS_BOT",params );
					(*it).second.NameStringId = botNameId();
					(*it).second.NameString.clear();
				}
			}
		}
		else
		{
			CPlace * place = CZoneManager::getInstance().getPlaceFromAlias( (*it).second.getPlace() );
			if ( place )
			{
				x = place->getCenterX();
				y = place->getCenterY();

				// Send the place name to the client if not already done (or if the name has changed)
				if ( place->getName() != (*it).second.NameString )
				{
					params[0].Identifier = place->getName();
					params[0].Type = STRING_MANAGER::place;
					updateTitle = true;
					newTitleId = STRING_MANAGER::sendStringToClient( user.getEntityRowId(),"COMPASS_PLACE",params );
					(*it).second.NameString = place->getName();
					(*it).second.NameStringId = 0;
				}
			}
		}
		/// ANTIBUG TO AVOID FLOODS
		uint maxClientIndex = 0;
		if ( dynamic_cast<CMissionSolo*>( _Mission ) )
			maxClientIndex = MaxSoloMissionCount;
		else if ( dynamic_cast<CMissionTeam*>( _Mission ) )
			maxClientIndex = MaxGroupMissionCount;
		if ( _ClientIndex < maxClientIndex )
		{
//			if ( node )
//			{
			if (compassIdx >= 8)
			{
				CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
				if(templ)
					nlwarning("In mission '%s': invalid access to CBD entry MISSIONS:%u:TARGET%u : TARGET is out of 8 range", templ->getMissionName().c_str(), _ClientIndex, compassIdx);
				else
					nlwarning("In mission template id '%u': (template doesn't have name!) invalid access to CBD entry MISSIONS:%u:TARGET%u : TARGET is out of 8 range", _Mission->getTemplateId(), _ClientIndex, compassIdx);
			}
			else
			{
				//				user._PropertyDatabase.x_setProp( node, "X", x );
				missionDb.getArray(_ClientIndex).getTARGET(compassIdx).setX(user._PropertyDatabase, x);
				//				user._PropertyDatabase.x_setProp( node, "Y", y );
				missionDb.getArray(_ClientIndex).getTARGET(compassIdx).setY(user._PropertyDatabase, y);
				if ( updateTitle )
					//					user._PropertyDatabase.x_setProp( node, "TITLE", newTitleId );
					missionDb.getArray(_ClientIndex).getTARGET(compassIdx).setTITLE(user._PropertyDatabase, newTitleId);
				//			}
			}
			compassIdx++;
		}
	}
	return compassIdx;
}

// ****************************************************************************
void CMissionBaseBehaviour::applyPlayerReconnectHandler()
{
	if ( _Mission->getFinished() )
		return;
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	uint32 index = _Mission->getPlayerReconnectHandlerIndex();
	std::list< CMissionEvent* > eventList;
	if( index < templ->PlayerReconnectHandlers.size() )
	{
		for (uint i = 0; getProcessingState() == CMissionBaseBehaviour::Normal &&
						i < templ->PlayerReconnectHandlers[index].Actions.size(); i++ )
			templ->PlayerReconnectHandlers[index].Actions[i]->launch(_Mission,eventList);
	}
	if (getProcessingState() == CMissionBaseBehaviour::Normal)
	{
		if (templ->Type == MISSION_DESC::Guild)
		{
			CMissionGuild * mission = dynamic_cast<CMissionGuild *>(_Mission);
			if (mission != NULL)
			{
				while( !eventList.empty() )
				{
					/// todo guild missions
					//guild->processGuildSpecificEvent( *(eventList.front()) );
					delete eventList.front();
					eventList.pop_front();
				}
			}
		}
		else
		{
			CCharacter * user = getMainEntity();
			if (user != NULL)
			{
				while (!eventList.empty())
				{
					user->processMissionEvent(*eventList.front());
					delete eventList.front();
					eventList.pop_front();
				}
			}
		}
	}

	// set processing state to normal except if the state is complete
	if (_ProcessingState == Complete)
	{
		templ->AlreadyDone = true;
	}
	else if (_ProcessingState == Failed)
	{
		_ProcessingState = Normal;
		onFailure( true );
	}
	else
	{
		_ProcessingState = Normal;
	}
}

// ****************************************************************************
void CMissionBaseBehaviour::applyCrashHandler(bool EGSCrash, const std::string & AIInstanceName )
{
	if ( _Mission->getFinished() )
		return;
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);

	uint32 index = _Mission->getCrashHandlerIndex();
	std::list< CMissionEvent* > eventList;
	if( index < templ->CrashHandlers.size() )
	{
		if ( EGSCrash || ( std::find( templ->CrashHandlers[index].AIInstances.begin(), templ->CrashHandlers[index].AIInstances.end(), AIInstanceName ) != templ->CrashHandlers[index].AIInstances.end() ) )
		{
			for ( uint i = 0; getProcessingState() == CMissionBaseBehaviour::Normal && i< templ->CrashHandlers[index].Actions.size() ; i++ )
				templ->CrashHandlers[index].Actions[i]->launch(_Mission,eventList);
		}
	}
	if ( getProcessingState() == CMissionBaseBehaviour::Normal )
	{
		if ( templ->Type == MISSION_DESC::Guild )
		{
			CMissionGuild * mission = dynamic_cast<CMissionGuild *>(_Mission);
			if ( mission )
			{
				while( !eventList.empty() )
				{
					/// todo guild missions
					//guild->processGuildSpecificEvent( *(eventList.front()) );
					delete eventList.front();
					eventList.pop_front();
				}
			}
		}
		else
		{
			CCharacter * user = getMainEntity();
			if (user)
			{
				while( !eventList.empty() )
				{
					user->processMissionEvent( *eventList.front() );
					delete eventList.front();
					eventList.pop_front();
				}
			}
		}
	}

	// set processing state to normal except if the state is complete
	if ( _ProcessingState == Complete )
	{
		templ->AlreadyDone = true;
	}
	else if ( _ProcessingState == Failed )
	{
		_ProcessingState = Normal;
		onFailure( true );
	}
	else
	{
		_ProcessingState = Normal;
	}
}

bool CMissionBaseBehaviour::checkConsistencyWithTemplate()
{
	const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	if ( templ )
	{
		uint count = 0;
		for ( std::map<uint32, EGSPD::CActiveStepPD>::iterator it = _Mission->getStepsBegin(); it != _Mission->getStepsEnd(); ++it )
		{
			count++;
			uint states = 0;
			for ( std::map<uint32, EGSPD::CActiveStepStatePD>::iterator itState = (*it).second.getStatesBegin(); itState != (*it).second.getStatesEnd(); ++itState )
				states++;

			uint idx = (*it).first-1;
			if ( idx >= templ->Steps.size() )
				return false;
			vector<uint32> ret;
			if ( templ->Steps[idx] == NULL )
				return false;
			templ->Steps[idx]->getInitState( ret );
			if ( ret.size() != states )
				return false;
		}
		if ( count > templ->Steps.size() )
			return false;
		return true;
	}
	return false;
}
