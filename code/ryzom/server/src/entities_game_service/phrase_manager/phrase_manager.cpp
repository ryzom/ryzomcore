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



/////////////
// INCLUDE 
/////////////
#include "stdpch.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/phrase_manager_callbacks.h"
#include "combat_phrase.h"
#include "s_phrase_factory.h"
#include "phrase_manager/s_effect.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "phrase_manager/magic_phrase.h"
#include "phrase_manager/special_power_phrase.h"
#include "harvest_phrase.h"
#include "timed_action_phrase.h"
#include "entities_game_service.h"
//
#include "entity_structure/statistic.h"
#include "egs_sheets/egs_static_ai_action.h"
//
#include "nel/misc/algo.h"
#include "server_share/stl_allocator_checker.h"


/////////////
// USING
/////////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;

/////////////
// GLOBALS 
/////////////
//CPhraseManager					*CPhraseManager::_Instance = NULL;

uint32	CSPhrase::NbAllocatedPhrases = 0;
uint32	CSPhrase::NbDesallocatedPhrases = 0;


//--------------------------------------------------------------
//		CEntityPhrases::stopCyclicAction()
//--------------------------------------------------------------
void CEntityPhrases::stopCyclicAction(const TDataSetRow &entityRowId)
{
	H_AUTO(CEntityPhrases_stopCyclicAction);
	if ( _CyclicAction != NULL)
	{
		// if next action is the cyclic action, stop it too
		if (_NextAction == _CyclicAction)
			_NextAction = NULL;
		
		_CyclicAction->cyclic(false);
		_CyclicAction = NULL;
		_CyclicActionInfos.reset();
		
		CCharacter *character = PlayerManager.getChar(entityRowId);
		if (character)
		{
			character->writeCycleCounterInDB();
		}
	}
} // stopCyclicAction //

//--------------------------------------------------------------
//		CEntityPhrases::cancelCombatActions()
//--------------------------------------------------------------
bool CEntityPhrases::cancelCombatActions(const TDataSetRow &entityRowId, bool disengageOnEndOnly)
{
	H_AUTO(CEntityPhrases_cancelCombatActions);
	
	bool returnValue = true;
	if (_CurrentAction != NULL && _CurrentAction->getType() ==  BRICK_TYPE::COMBAT)
	{
		CCombatPhrasePtr combatPhrase = dynamic_cast<CCombatPhrase*> ( static_cast<CSPhrase*> (_CurrentAction) );
		if (combatPhrase != NULL)
		{
			if (disengageOnEndOnly)
			{
				combatPhrase->disengageOnEnd(true);
				returnValue = false;
			}
			else if (!combatPhrase->beingProcessed() && !combatPhrase->disengageOnEnd())
			{
				if (_CurrentAction->state() >= CSPhrase::ExecutionInProgress)
					_CurrentAction->stop();
				else
					_CurrentAction->stopBeforeExecution();

				_CurrentAction = NULL;
				clearAttackFlag();
			}
			
			CCharacter *character = PlayerManager.getChar(entityRowId);
			if (character)
			{
				character->writeExecPhraseInDB(0);
				if ( ! combatPhrase->cyclic() )
					character->writeNextPhraseInDB( combatPhrase->nextCounter() );
			}
		}
	}
	
	if ( _NextAction != NULL && _NextAction->getType() == BRICK_TYPE::COMBAT )
	{
		CCharacter *character = PlayerManager.getChar(entityRowId);
		if (character)
		{
			character->writeExecPhraseInDB(0);
			if ( ! _NextAction->cyclic() )
				character->writeNextPhraseInDB( _NextAction->nextCounter() );
		}
		_NextAction = NULL;
	}
	
	if ( _CyclicAction != NULL && _CyclicAction->getType() == BRICK_TYPE::COMBAT )
	{
		_CyclicAction = NULL;
		_CyclicActionInfos.reset();
		CCharacter *character = PlayerManager.getChar(entityRowId);
		if (character)
		{
			character->writeCycleCounterInDB();
		}
	}
	
	return returnValue;
} // cancelCombatActions //

//--------------------------------------------------------------
//		CEntityPhrases::dumpPhrasesInfos()
//--------------------------------------------------------------
void CEntityPhrases::dumpPhrasesInfos( NLMISC::CLog &log) const
{
	H_AUTO(CEntityPhrases_dumpPhrasesInfos);
	
	if (_CurrentAction != NULL)
	{
		log.displayNL("	Current action :");
		log.displayNL("		Type %s", BRICK_TYPE::toString(_CurrentAction->getType()).c_str() );
		log.displayNL("		Current State %u", _CurrentAction->state() );
		log.displayNL("		Idle : %s", _CurrentAction->idle()?"Yes":"No" );
	}
	else
	{
		log.displayNL("	No current action");
	}
	
	if (_CyclicAction != NULL)
	{
		log.displayNL("	Cyclic action :");
		log.displayNL("		Type %s", BRICK_TYPE::toString(_CyclicAction->getType()).c_str() );
		log.displayNL("		Current State %u", _CyclicAction->state() );
		log.displayNL("		Idle : %s", _CyclicAction->idle()?"Yes":"No" );
	}
	else
	{
		log.displayNL("	No cyclic action");
	}
	
	if (_NextAction != NULL)
	{
		log.displayNL("	Next action :");
		log.displayNL("		Type %s", BRICK_TYPE::toString(_NextAction->getType()).c_str() );
		log.displayNL("		Current State %u", _NextAction->state() );
		log.displayNL("		Idle : %s", _NextAction->idle()?"Yes":"No" );
	}
	else
	{
		log.displayNL("	No next action");
	}
} // dumpPhrasesInfos //


//--------------------------------------------------------------
//		CEntityPhrases::renewCyclicAction()
//--------------------------------------------------------------
CSPhrasePtr &CEntityPhrases::renewCyclicAction()
{
	H_AUTO(CEntityPhrases_renewCyclicAction);
	
	if ( _CyclicAction != NULL )
	{
		// set next action to NULL if next action is the current action, it will be set to the new
		// cylic by setCyclicAction
		if (_NextAction == _CyclicAction)
			_NextAction = NULL;
		
		CSPhrasePtr ptr = CPhraseManager::getInstance().executePhrase( _CyclicActionInfos.ActorRowId, _CyclicActionInfos.TargetRowId, _CyclicActionInfos.CyclicActionBricks, true );
		if (ptr.isNull())
		{
			_CyclicAction = NULL;
			_CyclicActionInfos.reset();
		}
		else
		{
			setCyclicAction( ptr, _CyclicActionInfos );
		}
	}
	
	return _CyclicAction;
} // renewCyclicAction //

//--------------------------------------------------------------
//		CEntityPhrases::clearAttackFlag()
//--------------------------------------------------------------
void CEntityPhrases::clearAttackFlag()
{
	// reset 'attacks' flag to indicate ends of an action
	CEntityBase *actingEntity = CEntityBaseManager::getEntityBasePtr(_EntityId);
	if (actingEntity)
	{
		if (actingEntity->getActionFlag() & RYZOMACTIONFLAGS::Attacks)
		{
			if (ClearAttackFlags)
				actingEntity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		}
	}
}


//--------------------------------------------------------------
//						CPhraseManager()  
//--------------------------------------------------------------
//CPhraseManager::CPhraseManager()
//{
//} // CPhraseManager


//--------------------------------------------------------------
//						removeEntities()  
// remove entities from the manager and clear their phrases, call each tick to 
// remove all the entities dead or despaned since last tick
//--------------------------------------------------------------
void CPhraseManager::removeEntities()
{
	H_AUTO(CPhraseManager_removeEntities);
	
	if (_EntitiesToRemove.empty())
		return;
	
	list<TDataSetRow>::const_iterator itEnd = _EntitiesToRemove.end();
	list<TDataSetRow>::const_iterator it;
	
	for ( it = _EntitiesToRemove.begin() ; it != itEnd ; ++it)
	{
		TDataSetRow entityRowId = *it;
		// disengage entity if he was engaged in combat
		disengage( entityRowId, false, true );
		
		//disengage all the entities which were in melee combat with the removed entity and with no active phrase
		TRowSetRowMap::iterator itAgg = _MapEntityToMeleeAggressors.find( entityRowId );
		if (itAgg != _MapEntityToMeleeAggressors.end() )
		{
			set<TDataSetRow> ids = (*itAgg).second;
			set<TDataSetRow>::iterator itId;
			for ( itId = ids.begin() ; itId != ids.end() ; ++itId )
			{
				bool endCombat = true;
				// check entity has no current phrase
				TMapIdToIndex::iterator itIndex = _PhrasesIndex.find(*itId);
				if ( itIndex != _PhrasesIndex.end())
				{
					const uint32 index = (*itIndex).second;
					BOMB_IF( index >= _EntityPhrases.size(), "Index out of bound", continue);
					if ( _EntityPhrases[index].getCurrentAction() != NULL && _EntityPhrases[index].getCurrentAction()->state() == CSPhrase::Latent)
						endCombat = false;
				}
				
				if (endCombat)
				{
					if (cancelAllCombatSentences( *itId, true))
						disengage(*itId, false, true);
				}
			}
			//_MapEntityToMeleeAggressors.erase( it ); // automatically done by disengaging all aggressors
		}
		
		//disengage all the entities which were in range combat with the removed entity
		itAgg = _MapEntityToRangeAggressors.find( entityRowId );
		if ( itAgg != _MapEntityToRangeAggressors.end() )
		{
			set<TDataSetRow> ids = (*itAgg).second;
			set<TDataSetRow>::iterator itId;
			for ( itId = ids.begin() ; itId != ids.end() ; ++itId )
			{
				bool endCombat = true;
				// check entity has no current phrase
				TMapIdToIndex::iterator itIndex = _PhrasesIndex.find(*itId);
				if ( itIndex != _PhrasesIndex.end())
				{
					const uint32 index = (*itIndex).second;
					BOMB_IF( index >= _EntityPhrases.size(), "Index out of bound", continue);
					
					if (_EntityPhrases[index].getCurrentAction() != NULL && _EntityPhrases[index].getCurrentAction()->state() == CSPhrase::Latent)
						endCombat = false;
				}
				
				if (endCombat)
				{
					if (cancelAllCombatSentences( *itId, true))
						disengage( *itId, false, true);
				}
			}
			//_MapEntityToRangeAggressors.erase( it ); // automatically done by disengaging all aggressors
		}
		
		// find the entity phrases execution list if any
		TMapIdToIndex::const_iterator it = _PhrasesIndex.find(entityRowId);
		if ( it != _PhrasesIndex.end())
		{
			BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", continue);
			// clear sentences
			_EntityPhrases[(*it).second].cancelAllPhrases();
			// reset next and cycle counter in DB
			CCharacter *character = PlayerManager.getChar(entityRowId);
			if (character)
			{
				character->writeExecPhraseInDB(0);
				character->writeNextPhraseInDB(character->nextCounter());
				character->writeCycleCounterInDB();
			}
			
			// NB : do not remove the entry in phrases, will be cleaned in the update loop
		}
	}
	
	_EntitiesToRemove.clear();
} // removeEntities //

//--------------------------------------------------------------
//						updatePhrases()  
//--------------------------------------------------------------
void CPhraseManager::updatePhrases()
{
	H_AUTO(PhraseManagerUpdate);
	
	/************************************************************************/
	/* Display stats on nb phrases and effects every 60s
	/************************************************************************/
#ifdef NL_DEBUG
	/* TEMP!!!
	if (CTickEventHandler::getGameCycle() % 600 == 0 )
	{
		egs_pminfo("Nb allocated phrases = %u", CSPhrase::NbAllocatedPhrases);
		egs_pminfo("Nb desallocated phrases = %u", CSPhrase::NbDesallocatedPhrases);
		
		egs_pminfo("Nb allocated effects = %u", CSEffect::NbDesallocatedEffects);
		egs_pminfo("Nb desallocated effects = %u", CSEffect::NbDesallocatedEffects);		
	}
	*/
#endif
	
	// remove dead or despawned entities
	removeEntities();
	
	string errorString;
	
	// update max nb entities
	if (_MaxNbEntities < _PhrasesIndex.size())
	{
		_MaxNbEntities = (uint32)_PhrasesIndex.size();
#ifdef NL_DEBUG
		nlinfo("New record in nb of processed entities ! %u", _MaxNbEntities);
#endif
	}
	
	// update first sentence in each player sentences Fifo 
	TMapIdToIndex::iterator it;
	for (it = _PhrasesIndex.begin() ; it != _PhrasesIndex.end() ; )
	{
		const uint32 index  = (*it).second;
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", continue);
		
		CEntityPhrases &entityPhrases = _EntityPhrases[index];
		TPhraseList &launchingActions = entityPhrases.getLaunchingActions();
		
		const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
		
		// check every launching actions
		TPhraseList::iterator itLaunching = launchingActions.begin();
		while ( itLaunching != launchingActions.end() )
		{
			CSPhrasePtr &phrase = *itLaunching;
#if !FINAL_VERSION
			nlassert( phrase != NULL );
			nlassert( phrase->state() == CSPhrase::LatencyEnded && !phrase->isApplied() );
#endif
			
			if ( phrase->applyDate() <= time )
			{
				phrase->apply();
				phrase->isApplied(true);
				
				TPhraseList::iterator itDel = itLaunching;
				++itLaunching;
				launchingActions.erase(itDel);
			}
			else
			{
				++itLaunching;
			}
		}
		STL_ALLOC_TEST
		
		CSPhrasePtr phrase = entityPhrases.getCurrentAction();
		if (phrase == NULL)
		{
			// erase the CEntityPhrases object if there is no launching action anymore
			if (launchingActions.size() == 0)
			{
				TMapIdToIndex::iterator itDel = it;
				++it;
				_PhrasesIndex.erase(itDel);
				
				_FreeIndex.push_back(index);
				_EntityPhrases[index].cancelAllPhrases();
			}
			else
			{
				++it;
			}
			continue;
		}
		
		// if the phrase is cyclic and idle, take the next one
		if (phrase->cyclic() && phrase->idle())
		{
			entityPhrases.goToNextAction();
			phrase = entityPhrases.getCurrentAction();
		}
		
		if ( phrase == NULL )
		{
			egs_pminfo("NULL phrase in phrase update" );
			continue;
		}
		
		// update this phrase
		updateEntityCurrentAction( (*it).first, entityPhrases);
		
		// get next entity sentences
		++it;
	}
	
	// send all the waiting event reports
	sendEventReports();
	//
	sendAIEvents();
}

//--------------------------------------------------------------
//						updateEntityCurrentAction()  
//--------------------------------------------------------------
void CPhraseManager::updateEntityCurrentAction(const TDataSetRow &entityId, CEntityPhrases &entityPhrases)
{
	H_AUTO(PhraseManager_updatePhrase);
	
	CSPhrasePtr phrase = entityPhrases.getCurrentAction();
	TPhraseList &launchingActions = entityPhrases.getLaunchingActions();
	
	if (phrase == NULL)
		return;
	
	bool deletePhrase = false; // true if an error occurs
	bool executionEnd = false; // true if sentence execution has ended
	
	/************************************************************************/
	/* flag the phrase as 'being processed'
	/************************************************************************/
	phrase->beingProcessed(true);
	
	// if the phrase is being executed
	if ( phrase->state() == CSPhrase::SecondValidated || phrase->state() == CSPhrase::ExecutionInProgress || phrase->state() == CSPhrase::Latent )
	{
		H_AUTO(CEntityPhrases_updateEntityCurrentAction_processPhraseInExecution);
		
		const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
		if ( phrase->executionEndDate() <= time && phrase->state() == CSPhrase::ExecutionInProgress && !phrase->idle())
		{
			// second validation of phrase if not already done
			if ( ! phrase->validate() )
			{
				deletePhrase = true;
			}
			else
			{
				phrase->setState(CSPhrase::SecondValidated);
			}
		}
		else
		{
			if ( ! phrase->update() )
			{
				deletePhrase = true;
			}
		}
		
		if (!deletePhrase)
		{
			// update has been successful, now do things according to sentence state
			switch(phrase->state())
			{
			case CSPhrase::SecondValidated:
				if ( phrase->executionEndDate() <= time && !phrase->idle())
				{
					if (!phrase->launch())
					{
						deletePhrase = true;
						break;
					}
					else
					{
						phrase->setState(CSPhrase::Latent);
					}
				}
				// Does not waste a tick when became Latent (apply date is 0 (most of time))
				if ( ! ((phrase->state() == CSPhrase::Latent) && (phrase->applyDate() <= time)) )
					break;
				
			case CSPhrase::Latent:
				if ( !phrase->isApplied() && phrase->applyDate() <= time )
				{
					phrase->apply();
					phrase->isApplied(true);
				}
				
				if ( phrase->latencyEndDate() <= time )
				{
					CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(entityId);
					if (entity && entity->getId().getType() == RYZOMID::player)
					{
						CCombatPhrase * combatPhrase = dynamic_cast<CCombatPhrase*>( static_cast<CSPhrase*> (phrase));
						if ( combatPhrase )
						{
							if( phrase->cyclic() == false && entityPhrases.getNextActionConst() == NULL )
							{
								combatPhrase->disengageOnEnd(true);
							}
						}
					}

					phrase->end();
					phrase->setState(CSPhrase::LatencyEnded);
					executionEnd = true;
				}
				break;
				
				/*				case CSPhrase::LatencyEnded:
				{
				executionEnd = true;
				}
				break;
				*/				default:
				// unknown state ?
				break;
			}
		}
	}		
	// the phrase is updated for the first time (or gets out from idle state), validate and execute it
	else
	{
		if ( phrase->idle() )
		{
			H_AUTO(CEntityPhrases_updateEntityCurrentAction_processIdlePhrase);
			// phrase is idle, update to check if phrase must exit from idle state
			if ( ! phrase->update() )
			{
				deletePhrase = true;
			}

			// if out from idle, revalidate (conditions might have changes since validation) and execute
			if (phrase->idle())
				goto afterPhraseProcessing;
		}

		{
			H_AUTO(CEntityPhrases_updateEntityCurrentAction_firstValidateAndExecute);
			// if phrase has never been evaluated, evaluate it
			if ( phrase->state() == CSPhrase::New )
			{
				if (!phrase->evaluate())
				{
					//nlwarning("For player %s", phrase->getPlayerId().toString().c_str() );
					nlwarning("	Invalid phrase tested - should NEVER happens !!!!!!!!");
					deletePhrase = true;
				}
				else
				{
					phrase->setState(CSPhrase::Evaluated);
				}
			}
			
			// validate phrase
			if ( ! phrase->validate() )
			{
				deletePhrase = true;
			}			
			else
			{
				phrase->setState(CSPhrase::Validated);
				//if sentense isn't idle, execute it
				if ( ! phrase->idle() )
				{
					// execute phrase
					phrase->execute();
					phrase->setState(CSPhrase::ExecutionInProgress);
					// if entity is a player, update DB
					CCharacter *character = PlayerManager.getChar(entityId);
					if (character)
					{
						character->writeExecPhraseInDB(phrase->phraseBookIndex());
						if ( ! phrase->cyclic() )
							character->writeNextPhraseInDB( phrase->nextCounter() );
					}
				}
			}
		}
	}
afterPhraseProcessing:
	
	/************************************************************************/
	/* the phrase is no longer in processing
	/************************************************************************/
	phrase->beingProcessed(false);
	
	if (deletePhrase || executionEnd)
	{
		H_AUTO(CEntityPhrases_updateEntityCurrentAction_deleteOrEndPhrase);
		
		// reset 'attacks' flag to indicate ends of an action
		CEntityBase *actingEntity = CEntityBaseManager::getEntityBasePtr(entityId);
		if (actingEntity)
			actingEntity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		
		CCharacter *character = dynamic_cast<CCharacter *> (actingEntity);
		if (character)
		{
			character->writeExecPhraseInDB(0);
			if ( ! phrase->cyclic() )
				character->writeNextPhraseInDB( phrase->nextCounter() );
		}
		
		if (deletePhrase == true)
		{
			// if the phrase is cyclic we must clear it in entityPhrases
			if (phrase->cyclic())
			{
				entityPhrases.stopCyclicAction(entityId);
			}
			phrase->stop();
		}
		else if(executionEnd == true)
		{
			if ( !phrase->isApplied() )
			{
				H_AUTO(CEntityPhrases_updateEntityCurrentAction_pushLaunchingAction);
#if !FINAL_VERSION
				TPhraseList::iterator itExists = std::find(launchingActions.begin(), launchingActions.end(), phrase);
				nlassert(itExists == launchingActions.end());
#endif
				
				launchingActions.push_back(phrase);
				if (phrase->cyclic())
				{
					// warning: renewCyclicAction() can return NULL if there is no cyclic action
					// it is possible if cancelCombatActions() has been called.
					phrase = entityPhrases.renewCyclicAction();
					if (!phrase.isNull())
					{
						phrase->evaluate();
						phrase->setState(CSPhrase::Evaluated);
					}
				}
			}
			else if (phrase->cyclic())
			{
				phrase->isApplied(false);
				phrase->evaluate();
				phrase->setState(CSPhrase::Evaluated);
			}
		}
		
		// go to next action for this entity
		entityPhrases.goToNextAction();
	}
} // updatePhrases //


//-----------------------------------------------
//			executeNoTime()
//-----------------------------------------------
void CPhraseManager::executeNoTime( CSPhrasePtr &phrasePtr, bool needTovalidate)
{
	H_AUTO(CPhraseManager_executeNoTime);
	
	if (phrasePtr == NULL)
		return;
	
	if (!phrasePtr->evaluate())
		return;
	
	if( needTovalidate )
	{
		if (!phrasePtr->validate())
			return;
	}
	
	phrasePtr->execute();
	
	if (!phrasePtr->launch())
		return;
	
	phrasePtr->apply();
	phrasePtr->end();
} // executeNoTime //


//-----------------------------------------------
//			sendEventReports()
//-----------------------------------------------
void CPhraseManager::sendEventReports()
{
	H_AUTO(CPhraseManager_sendEventReports);
	
	if ( _EventReports.empty() && _AIEventReports.empty())
		return;
	
	if ( !_RegisteredServices.empty() )
	{
		// send to registered services
		CMessage msgReport("EVENT_REPORTS");
		msgReport.serialCont(_EventReports);
		
		set<NLNET::TServiceId>::iterator it;
		for (it = _RegisteredServices.begin() ; it != _RegisteredServices.end() ; ++it)
		{
			sendMessageViaMirror (*it, msgReport);
			// 
			INFOLOG("Send EVENT_REPORTS to service %u", it->get() );
		}
	}
	
	if ( !_AIEventReports.empty() && !_AIRegisteredServices.empty() )
	{
		// send to registered services for AI 
		CBSAIEventReportMsg msgAI;
		const uint nbAiReports = (uint)_AIEventReports.size();
		for (uint i = 0 ; i < nbAiReports ; ++i )
		{
			msgAI.pushBack( _AIEventReports[i] );
		}
		
		// it's better to broadcast rather than sending it to each AIS.
		msgAI.send("AIS");
		
		/*		set<uint16>::iterator it;
		for (it = _AIRegisteredServices.begin() ; it != _AIRegisteredServices.end() ; ++it)
		{
		msgAI.send( uint8 (*it) );
		INFOLOG("Send EVENT_REPORTS to AI service %u", (*it) );
		}
		*/	}
		
		_EventReports.clear();
		_AIEventReports.clear();
} // sendEventReports //



//-----------------------------------------------
//			sendAIEvents()
//-----------------------------------------------
void CPhraseManager::sendAIEvents()
{
/*if (_AIEvents.empty())
return;

*/	TAIEventList::iterator it;
	const TAIEventList::iterator itEnd = _AIEvents.end();

	for (it = _AIEvents.begin() ; it != itEnd ; ++it)
	{
//		msgai.serial( *(*it) );
		delete (*it);
		*it = NULL;
	}

	_AIEvents.clear();
	
	/*	set<uint16>::iterator itservice;
	for (itservice = _AIRegisteredServices.begin() ; itservice != _AIRegisteredServices.end() ; ++itservice)
	{
	sendMessageViaMirror (*itservice, msgai);
	INFOLOG("Send AI_EVENTS to AI service %u", (*itservice) );
	}
	*/	
} // sendAIEvents //


//-----------------------------------------------
//			addPhrase()
//-----------------------------------------------
bool CPhraseManager::addPhrase( const TDataSetRow &actorRowId, CSPhrasePtr &phrase, const CCyclicActionInfos &cyclicInfos, bool cyclic )
{
	H_AUTO(CPhraseManager_addPhrase);
	
	if (!phrase) return false;
	
	TMapIdToIndex::iterator it = _PhrasesIndex.find( actorRowId );
	// actor doesn't already have phrases
	if (it == _PhrasesIndex.end() )
	{	
		// new entry
		CEntityPhrases entityPhrases(TheDataset.getEntityId(actorRowId));
//#ifdef NL_DEBUG
//		entityPhrases.EntityId = TheDataset.getEntityId(actorRowId);
//#endif				
		if (cyclic)
		{
			entityPhrases.setCyclicAction(phrase, cyclicInfos);
		}
		else
		{
			entityPhrases.setNextPhrase(phrase);
		}
		
		uint32 index;
		if (_FreeIndex.empty())
		{
			_EntityPhrases.push_back(entityPhrases);
			index = (uint32)_EntityPhrases.size()-1;
		}
		else
		{
			index = _FreeIndex.back();
			_FreeIndex.pop_back();
			_EntityPhrases[index] = entityPhrases;
		}
		
		_PhrasesIndex.insert( make_pair(actorRowId, index) );
	}
	// actor already have phrases in the manager, just add the new one
	else
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return false);
		
		CEntityPhrases &entityPhrases = _EntityPhrases[(*it).second];
		
		// TEMP FIX : FORBID TWO CRAFT PHRASE IN QUEUE
		if (phrase->getType() == BRICK_TYPE::FABER)
		{
			if (entityPhrases.getCurrentAction() && entityPhrases.getCurrentAction()->getType() == BRICK_TYPE::FABER)
				return false;
		}
		//////////////////////////////////////////////////////////////////////////
		
		if (cyclic)
		{
			//egs_pminfo( "Set next cyclic action" );
			entityPhrases.setNextPhrase(phrase); // ensure no time is lost when switching action
			entityPhrases.setCyclicAction(phrase, cyclicInfos);
		}
		else
		{
			entityPhrases.setNextPhrase(phrase);
		}
	}
	
	return true;
} // addPhrase //

//-----------------------------------------------
//			useConsumableItem()
//-----------------------------------------------
CSPhrasePtr CPhraseManager::useConsumableItem( const TDataSetRow &actorRowId, const CStaticItem *itemForm, uint16 quality )
{
	CSPhrasePtr phrase;
	if ( itemForm->EffectWhenConsumed != CSheetId::Unknown )
	{
		phrase = CPhraseManager::getInstance().executePhrase( actorRowId, actorRowId, itemForm->EffectWhenConsumed, false, 0, 0, false, false);
	}
	else
	{
		CSpecialPowerPhrase * powerPhrase = new CSpecialPowerPhrase();
		phrase = powerPhrase;
		if ( !powerPhrase || !powerPhrase->buildFromConsumable(actorRowId, itemForm, quality) )
			phrase = NULL;
		
		if ( phrase != NULL )
		{
			if (!addPhrase(actorRowId, phrase))
				phrase = NULL;
		}
		
		{
			CCharacter* character = PlayerManager.getChar(actorRowId);
			if (character)
				character->checkCharacterStillValide("End CPhraseManager::useConsumableItem");
		}
	}
	if ( !itemForm->EmoteWhenConsumed.empty() )
	{
		CCharacter* character = PlayerManager.getChar(actorRowId);
		if (character)
		{
			uint16 emoteIdx = CSheets::getTextEmoteList().getEmoteIndex(itemForm->EmoteWhenConsumed);
			MBEHAV::EBehaviour behav = CSheets::getTextEmoteList().getEmoteBehav(itemForm->EmoteWhenConsumed);
			character->sendEmote(character->getId(), behav, emoteIdx, false);
		}
	}
	
	return phrase;
}

//-----------------------------------------------
//			executePhrase()
//-----------------------------------------------
CSPhrasePtr CPhraseManager::executePhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const std::vector<NLMISC::CSheetId> &brickIds, bool cyclic, uint16 phraseId, uint8 nextCounter, bool enchant, bool needToValidate )
{
	H_AUTO(CPhraseManager_executePhrase);
	
	if ( brickIds.empty() )
		return NULL;
	const CStaticBrick * brick = CSheets::getSBrickForm( brickIds[0] );
	if ( brick && brick->Family == BRICK_FAMILIES::BEPA )
	{
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (entity && entity->getId().getType() == RYZOMID::player)
		{
			CCharacter * user = (CCharacter*)entity;
			user->procEnchantment();
			user->writeNextPhraseInDB(nextCounter);
			return CSPhrasePtr(NULL);
		}
		else
			nlwarning("<CPhraseManager::executePhrase> actor %u is a bot or an invalid entity and uses a proc item %u", actorRowId.getIndex());
		
	}
	
	CSPhrasePtr phrase = buildSabrinaPhrase(actorRowId, targetRowId, brickIds, phraseId, nextCounter, true);
	if (!phrase)
	{
		// set the action flag to 0 if actor was a bot as the AI set it to 1
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (entity && entity->getId().getType() != RYZOMID::player)
		{
			entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		}
		return NULL;
	}
	
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
	if (entity && entity->getId().getType() == RYZOMID::player)
	{
		CCharacter * user = (CCharacter*)entity;
		if (enchant)
		{
			phrase->setEnchantMode(true);
		}
		else
		{			
			phrase->setEnchantMode(false);
		}
	}
	
	
	phrase->setPrimaryTarget( targetRowId );
	
	// if phrase is a forage extraction phrase, cyclic must be true
	// if phrase isn't a combat phrase, cyclic must be false
	if (phrase->getType() == BRICK_TYPE::FORAGE_EXTRACTION)
		cyclic = true;
	else if (phrase->getType() != BRICK_TYPE::COMBAT)
		cyclic = false;
	
	// if phrase is a power phrase then execute it right now, do not wait update, and returns
	if (phrase->getType() == BRICK_TYPE::SPECIAL_POWER)
	{
		executeNoTime(phrase,needToValidate);

		CCharacter * user = dynamic_cast<CCharacter *>(entity);
		if (user)
			user->writeNextPhraseInDB( phrase->nextCounter() );
		return NULL;
	}
	
	// keep infos if action is cyclic
	CCyclicActionInfos cyclicInfos;
	if (cyclic)
	{
		cyclicInfos.ActorRowId = actorRowId;
		cyclicInfos.TargetRowId = targetRowId;
		cyclicInfos.CyclicActionBricks = brickIds;
	}
	
	if (!addPhrase(actorRowId, phrase, cyclicInfos, cyclic))
	{
		phrase = NULL;
		
		// set the action flag to 0 if actor was a bot as the AI set it to 1
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (entity && entity->getId().getType() != RYZOMID::player)
		{
			entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		}
	}
	
	return phrase;
} // executePhrase //


//-----------------------------------------------
//			executePhrase()
//-----------------------------------------------
CSPhrasePtr CPhraseManager::executePhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, const NLMISC::CSheetId &phraseSheet, bool cyclic, uint16 phraseId , uint8 nextCounter, bool enchant, bool needToValidate )
{
	H_AUTO(CPhraseManager_executePhrase2);
	
	const CStaticRolemasterPhrase *phrase = CSheets::getSRolemasterPhrase(phraseSheet);
	if (!phrase)
	{
		// set the action flag to 0 if actor was a bot as the AI set it to 1
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (entity && entity->getId().getType() != RYZOMID::player)
		{
			entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		}
		return NULL;
	}
	
	return executePhrase(actorRowId, targetRowId, phrase->Bricks, cyclic, phraseId, nextCounter, enchant, needToValidate);
} // executePhrase //

//-----------------------------------------------
//			executeAiAction()
//-----------------------------------------------
void CPhraseManager::executeAiAction( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const NLMISC::CSheetId &actionId, float damageCoeff, float speedCoeff )
{
	H_AUTO(CPhraseManager_executeAiAction);
	
	{
		CCharacter * c = PlayerManager.getChar(targetRowId);
		if( c != 0 )
			c->checkCharacterStillValide("start CPhraseManager::executeAiAction");
	}
	
	// get ai action form
	const CStaticAiAction *aiActionForm = CSheets::getAiActionForm(actionId);
	if (aiActionForm == NULL)
	{
		// set the action flag to 0 if actor was a bot as the AI set it to 1
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (entity && entity->getId().getType() != RYZOMID::player)
		{
			entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		}
		{
			CCharacter * c = PlayerManager.getChar(targetRowId);
			if( c != 0 )
				c->checkCharacterStillValide("Step1 CPhraseManager::executeAiAction");
		}
		return;
	}
	
	CSPhrasePtr phrase = NULL;
	bool instantExecution = false;
	
	// find action type
	AI_ACTION::TAiActionType type = aiActionForm->getType();
	switch(type)
	{
	case AI_ACTION::Melee:
	case AI_ACTION::Range:
		{
			phrase = new CCombatPhrase();
			if (phrase)
			{
				CCombatPhrase * combatPhrase = dynamic_cast<CCombatPhrase*>( static_cast<CSPhrase*> (phrase));
				if ( !combatPhrase || ! ( combatPhrase->initPhraseFromAiAction(actorRowId, aiActionForm, damageCoeff, speedCoeff) ) )
				{
					phrase = NULL;
				}
			}
		}
		break;
		
	case AI_ACTION::HealSpell:
	case AI_ACTION::DamageSpell:
	case AI_ACTION::DoTSpell:
	case AI_ACTION::HoTSpell:
	case AI_ACTION::EffectSpell:
	case AI_ACTION::EoTSpell:
	case AI_ACTION::ToxicCloud:
		{
			phrase = new CMagicPhrase();
			if (phrase)
			{
				CMagicPhrase * magicPhrase = dynamic_cast<CMagicPhrase*>( static_cast<CSPhrase*> (phrase));
				if ( !magicPhrase || !( magicPhrase->initPhraseFromAiAction(actorRowId, aiActionForm) ) )
				{
					phrase = NULL;
				}
				else
				{
					phrase->setPrimaryTarget( targetRowId );
					
					// check if phrase is instant
					if (magicPhrase->getCastingTime() == 0 )
					{
						instantExecution = true;
					}
				}				
			}
		}
		break;
		
	default:
		nlwarning("<CPhraseManager::executeAiAction> Unknown AI_ACTION type %u for ai action %s", type, actionId.toString().c_str());
		break;
	};
	
	if ( phrase != NULL )
	{
		if (instantExecution)
		{
			executeNoTime(phrase);
			{
				CCharacter * c = PlayerManager.getChar(targetRowId);
				if( c != 0 )
					c->checkCharacterStillValide("Step2 CPhraseManager::executeAiAction");
			}
			return;
		}
		else
		{
			if (!addPhrase(actorRowId, phrase))
				phrase = NULL;
		}		
	}
	
	if (phrase == NULL)
	{
		// set the action flag to 0 if actor was a bot as the AI set it to 1
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(actorRowId);
		if (entity && entity->getId().getType() != RYZOMID::player)
		{
			entity->setActionFlag( RYZOMACTIONFLAGS::Attacks, false );
		}
	}
	{
		CCharacter * c = PlayerManager.getChar(targetRowId);
		if( c != 0 )
			c->checkCharacterStillValide("End CPhraseManager::executeAiAction");
	}
	return;
} // executeAiAction //

//-----------------------------------------------
//			init()
//-----------------------------------------------
void CPhraseManager::init()
{
	_MaxNbEntities = 0;
	_EntityPhrases.reserve(500);

	addCallbacks();
	
	PHRASE_UTILITIES::loadLocalisationTable( CPath::lookup("localisation.localisation_table" ) );
} // init //

//-----------------------------------------------
//			addAiEventReport()
//-----------------------------------------------
void CPhraseManager::addAiEventReport( const CAiEventReport &report )
{
	_AIEventReports.push_back(report);
	
	NLMISC::CEntityId actorId = CEntityBaseManager::getEntityId(report.Originator);
	NLMISC::CEntityId targetId = CEntityBaseManager::getEntityId(report.Target);
	
	AGGROLOG("AI event report : actor %s, target %s, type %s, aggro = %f, ", actorId.toString().c_str(), targetId.toString().c_str(), ACTNATURE::toString(report.Type).c_str(), report.AggroAdd );
	for (uint i = 0 ; i < report.DeltaValue.size() ; ++i)
		AGGROLOG("	Delta : value %s : %d", AI_EVENT_REPORT::toString(report.AffectedStats[i]).c_str(), report.DeltaValue[i]);
} // addAiEventReport //

//-----------------------------------------------
//			addCallbacks()
//-----------------------------------------------
void CPhraseManager::addCallbacks()
{
	static bool added = false;
	if (added)
		return;
	
	added = true;
	
	//array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "REGISTER_EVENT_REPORTS",			cbRegisterService		},
		{ "REGISTER_AI_EVENT_REPORTS",		cbRegisterServiceAI		},
		
		{ "UNREGISTER_EVENT_REPORTS",		cbUnregisterService		},
		{ "UNREGISTER_AI_EVENT_REPORTS",	cbUnregisterServiceAI	},
		
		{ "DISENGAGE_NOTIFICATION",			cbDisengageNotification	},
		{ "DISENGAGE",						cbDisengage				},	
	};
	
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
} // addCallbacks //

//-----------------------------------------------
//			clearMeleeEngagedEntities()
//-----------------------------------------------
void CPhraseManager::clearMeleeEngagedEntities()
{ 
	H_AUTO(CPhraseManager_clearMeleeEngagedEntities);
	
	TRowRowMap::const_iterator it;
	const TRowRowMap::const_iterator itEnd = _MapEntityToEngagedEntityInMeleeCombat.end();
	for (it = _MapEntityToEngagedEntityInMeleeCombat.begin() ; it != itEnd ; ++it)
	{
		// cancel all combat sentences for that entity
		cancelAllCombatSentences( (*it).first, false );
	}
	
	_MapEntityToMeleeAggressors.clear();
	
	_MapEntityToEngagedEntityInMeleeCombat.clear();
} // clearMeleeEngagedEntities //

//-----------------------------------------------
//-----------------------------------------------
TDataSetRow CPhraseManager::getEntityEngagedMeleeBy( const TDataSetRow &entityRowId) const
{
	TRowRowMap::const_iterator it = _MapEntityToEngagedEntityInMeleeCombat.find( entityRowId );
	if (it != _MapEntityToEngagedEntityInMeleeCombat.end() )
		return (*it).second;
	else
		return TDataSetRow();
}

//-----------------------------------------------
//-----------------------------------------------
TDataSetRow CPhraseManager::getEntityEngagedRangeBy( const TDataSetRow &entityRowId) const
{
	TRowRowMap::const_iterator it = _MapEntityToEngagedEntityInRangeCombat.find( entityRowId );
	if (it != _MapEntityToEngagedEntityInRangeCombat.end() )
		return (*it).second;
	else
		return TDataSetRow();
}

//-----------------------------------------------
//-----------------------------------------------
const std::set<TDataSetRow> &CPhraseManager::getMeleeAggressors( const TDataSetRow &entityRowId ) const
{
	static std::set<TDataSetRow> emptySet;
	const TRowSetRowMap::const_iterator it = _MapEntityToMeleeAggressors.find(entityRowId) ;
	if (it != _MapEntityToMeleeAggressors.end() )
	{
		return (*it).second;
	}
	else
		return emptySet;
}

//-----------------------------------------------
//-----------------------------------------------
const std::set<TDataSetRow> &CPhraseManager::getRangeAggressors( const TDataSetRow &entityRowId ) const
{
	static std::set<TDataSetRow> emptySet;
	const TRowSetRowMap::const_iterator it = _MapEntityToRangeAggressors.find(entityRowId) ;
	if (it != _MapEntityToRangeAggressors.end() )
	{
		return (*it).second;
	}
	else
		return emptySet;
}

//-----------------------------------------------
//			checkPhraseValidity()
//-----------------------------------------------
bool CPhraseManager::checkPhraseValidity( const std::vector<NLMISC::CSheetId> &brickIds ) const
{
	H_AUTO(CPhraseManager_checkPhraseValidity);
	
	// No check made for the moment, do it later
	vector<CSheetId>::const_iterator it;
	vector<CSheetId>::const_iterator itEnd = brickIds.end();
	
	set<BRICK_FAMILIES::TBrickFamily> mandatoryFamilies;	
	set<BRICK_FAMILIES::TBrickFamily> allowedFamilies;
	
	static vector<string> forbidWords(30);
	static vector<string> defWords(30);
	
	set<BRICK_FAMILIES::TBrickFamily> foundFamilies;
	set<string> forbiddenDef;
	set<string> forbiddenExclude;
	
	sint16 totalCost = 0;
	sint16 totalCredit = 0;
	float totalRelativeCost = 1.0f;
	float totalRelativeCredit = 1.0f;
	
	for (it = brickIds.begin() ; it != itEnd ; ++it)
	{
		const CStaticBrick *brick = CSheets::getSBrickForm(*it);
		if (!brick)
		{
			nlwarning("<CPhraseManager::checkPhraseValidity> Cannot find brick object for brick sheet %s", (*it).toString().c_str() );
			return false;
		}
		
		mandatoryFamilies.insert( brick->MandatoryFamilies.begin(), brick->MandatoryFamilies.end() );
		allowedFamilies.insert( brick->OptionalFamilies.begin(), brick->OptionalFamilies.end() );
		allowedFamilies.insert( brick->CreditFamilies.begin(), brick->CreditFamilies.end() );
		
		forbidWords.clear();
		defWords.clear();
		NLMISC::splitString(brick->ForbiddenExclude, ":", forbidWords);
		NLMISC::splitString(brick->ForbiddenDef, ":", defWords);
		
		// check brick doesn't define a flag forbid by another brick
		for ( uint i = 0 ; i < defWords.size() ; ++i)
		{
			if (forbiddenExclude.find(defWords[i]) != forbiddenExclude.end() )
			{
				DEBUGLOG("<checkPhraseValidity> The flag %s is forbidden, cancel", defWords[i].c_str() );
				return false;
			}
		}
		// check brick doesn't forbid a flag already defined
		for ( uint i = 0 ; i < forbidWords.size() ; ++i)
		{
			if (forbiddenDef.find(forbidWords[i]) != forbiddenDef.end() )
			{
				DEBUGLOG("<checkPhraseValidity> The flag %s is forbidden, cancel", forbidWords[i].c_str() );
				return false;
			}
		}
		
		forbiddenDef.insert( defWords.begin(), defWords.end() );
		forbiddenExclude.insert( forbidWords.begin(), forbidWords.end() );
		
		// skip root brick for found families
		if ( !BRICK_FAMILIES::isRootFamily(brick->Family) )
		{
			foundFamilies.insert( brick->Family );
			/*			if ( foundFamilies.insert( brick->Family ).second == false)
			{
			nlwarning("<checkPhraseValidity> The family %s were already found in phrase, error", BRICK_FAMILIES::toString(brick->Family).c_str() );
			return false;
			}
			*/
		}

		if( brick->SabrinaValue > 0 )
			totalCost += brick->SabrinaValue;
		else
			totalCredit -= brick->SabrinaValue;

		if( brick->SabrinaRelativeValue > 0.0f )
			totalRelativeCost += brick->SabrinaRelativeValue;
		else
			totalRelativeCredit -= brick->SabrinaRelativeValue;
	}
	
	// check cost
	totalCost = (sint16)(totalCost * totalRelativeCost);
	totalCredit = (sint16)(totalCredit * totalRelativeCredit);
	

	if (totalCost > totalCredit)
	{
		DEBUGLOG("<checkPhraseValidity> Credit must be > to cost for a phrase to be valid, cancel return false");
		return false;
	}
	
	// check all mandatory are present
	set<BRICK_FAMILIES::TBrickFamily>::const_iterator itb;
	set<BRICK_FAMILIES::TBrickFamily>::const_iterator itbEnd = mandatoryFamilies.end();
	for (itb = mandatoryFamilies.begin() ; itb != itbEnd ; ++itb)
	{
		set<BRICK_FAMILIES::TBrickFamily>::iterator itbf = foundFamilies.find(*itb);
		if ( itbf == foundFamilies.end() )
		{
			DEBUGLOG("<checkPhraseValidity> The family %s is mandatory but isn't in the phrase, cancel return false", BRICK_FAMILIES::toString(*itb).c_str() );
			return false;
		}
		else
			foundFamilies.erase(itbf);
	}
	
	// check all the other families are allowed
	itbEnd = foundFamilies.end();
	for (itb = foundFamilies.begin() ; itb != itbEnd ; ++itb)
	{
		if (allowedFamilies.find(*itb) == allowedFamilies.end() )
		{
			DEBUGLOG("<checkPhraseValidity> The family %s isn't a valid optional or creadit one, cancel return false", BRICK_FAMILIES::toString(*itb).c_str() );
			return false;
		}
	}
	
	return true;
} // checkPhraseValidity //


//-----------------------------------------------
//			buildSabrinaPhrase()
//-----------------------------------------------
CSPhrasePtr CPhraseManager::buildSabrinaPhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, const vector<CSheetId> &brickIds, uint16 phraseId, uint8 nextCounter, bool execution )
{
	H_AUTO(CPhraseManager_buildSabrinaPhrase);
	
	if (brickIds.empty())
		return NULL;
	
	// if actor is invalid return NULL
	if ( !TheDataset.isAccessible(actorRowId) )
		return NULL;
	
	CSPhrasePtr phrase = ISPhraseFactory::buildPhrase(actorRowId,brickIds, execution);
	if (!phrase)
	{
		nlwarning("<CPhraseManager::buildSabrinaPhrase> For entity %s, factory returns a NULL pointer instead of phrase!. First brick is %s", actorRowId.toString().c_str(),brickIds[0].toString().c_str());
		return NULL;
	}
	
	phrase->nextCounter(nextCounter);
	phrase->phraseBookIndex(phraseId);
	
	return phrase;
} // buildSabrinaPhrase //

//-----------------------------------------------
//			defaultAttackSabrina()
//-----------------------------------------------
void CPhraseManager::defaultAttackSabrina( const TDataSetRow &attackerRowId, const TDataSetRow &targetRowId )
{
	H_AUTO(CPhraseManager_defaultAttackSabrina);
	
	static const CSheetId defaultAttackSheet("bfpa01.sbrick");
	
	// if attacker is in water, he can't attack
	CCharacter *player = PlayerManager.getChar(attackerRowId);
	if (player)
	{
		if (player->isInWater())
		{
			
			PHRASE_UTILITIES::sendDynamicSystemMessage(attackerRowId, "EGS_CANNOT_ATTACK_IN_WATER");	
			return;
		}
	}
	
	vector<CSheetId> bricks;
	bricks.push_back(defaultAttackSheet);
	
	CSPhrasePtr phrase = executePhrase( attackerRowId, targetRowId, bricks, true);
	
	CCombatPhrase *combatPhrase = dynamic_cast<CCombatPhrase*> ( static_cast<CSPhrase*> (phrase) );
	if (combatPhrase != NULL)
	{
		combatPhrase->setRootSheetId(defaultAttackSheet);
		if (player)
		{
			phrase->nextCounter(player->nextCounter());
			
			//Bsi.append( StatPath, NLMISC::toString("[UAA] %s %s %s", player->getId().toString().c_str(), "default attack", defaultAttackSheet.toString().c_str()) );
			//EgsStat.displayNL("[UAA] %s %s %s", player->getId().toString().c_str(), "default attack", defaultAttackSheet.toString().c_str());
//			EGSPD::useActionAchete(player->getId(), "default attack", defaultAttackSheet.toString());
		}
	}
} // defaultAttackSabrina //

//--------------------------------------------------------------
//					removeEntity()  
//
// Precondition: the entity is in mirror
//--------------------------------------------------------------
void CPhraseManager::removeEntity( const TDataSetRow &entityRowId, bool removeRightNow )
{
	H_AUTO(CPhraseManager_removeEntity);
	
	if ( !removeRightNow )
	{
		_EntitiesToRemove.push_back(entityRowId);
	}
	else
	{
		// disengage entity if he was engaged in combat
		disengage( entityRowId, false, true );
		
		//disengage all the entities which were in melee combat with the removed entity and with no active phrase
		TRowSetRowMap::iterator itAgg = _MapEntityToMeleeAggressors.find( entityRowId );
		if (itAgg != _MapEntityToMeleeAggressors.end() )
		{
			set<TDataSetRow> ids = (*itAgg).second;
			set<TDataSetRow>::iterator itId;
			for ( itId = ids.begin() ; itId != ids.end() ; ++itId )
			{
				bool endCombat = true;
				// check entity has no current phrase
				TMapIdToIndex::iterator itIndex = _PhrasesIndex.find(*itId);
				if ( itIndex != _PhrasesIndex.end())
				{
					const uint32 index = (*itIndex).second;
					BOMB_IF( index >= _EntityPhrases.size(), "Index out of bound", continue);
					if ( _EntityPhrases[index].getCurrentAction() != NULL && _EntityPhrases[index].getCurrentAction()->state() == CSPhrase::Latent)
						endCombat = false;
				}
				
				if (endCombat)
				{
					if (cancelAllCombatSentences( *itId, true))
						disengage( *itId, false, true);
				}
			}
			//_MapEntityToMeleeAggressors.erase( it ); // automatically done by disengaging all aggressors
		}
		
		//disengage all the entities which were in range combat with the removed entity
		itAgg = _MapEntityToRangeAggressors.find( entityRowId );
		if (itAgg != _MapEntityToRangeAggressors.end() )
		{
			set<TDataSetRow> ids = (*itAgg).second;
			set<TDataSetRow>::iterator itId;
			for ( itId = ids.begin() ; itId != ids.end() ; ++itId )
			{
				bool endCombat = true;
				// check entity has no current phrase
				TMapIdToIndex::iterator itIndex = _PhrasesIndex.find(*itId);
				if ( itIndex != _PhrasesIndex.end())
				{
					const uint32 index = (*itIndex).second;
					BOMB_IF( index >= _EntityPhrases.size(), "Index out of bound", continue);
					if ( _EntityPhrases[index].getCurrentAction() != NULL && _EntityPhrases[index].getCurrentAction()->state() == CSPhrase::Latent)
						endCombat = false;
				}
				
				if (endCombat)
				{
					if (cancelAllCombatSentences( *itId, true))
						disengage( *itId, false, true);
				}
			}
			//_MapEntityToRangeAggressors.erase( it ); // automatically done by disengaging all aggressors
		}
		
		// find the entity phrases execution list if any
		TMapIdToIndex::iterator itIndex = _PhrasesIndex.find( entityRowId );
		if ( itIndex != _PhrasesIndex.end() )
		{
			const uint32 index = (*itIndex).second;
			BOMB_IF( index >= _EntityPhrases.size(), "Index out of bound", return);
			// clear sentences
			_EntityPhrases[index].cancelAllPhrases();
			// reset next and cycle counter in DB
			CCharacter *character = PlayerManager.getChar(entityRowId);
			if (character)
			{
				character->writeExecPhraseInDB(0);
				character->writeNextPhraseInDB(character->nextCounter());
				character->writeCycleCounterInDB();
			}
			// do not remove the entry in phrases, will be cleaned in the update loop
		}
	}
} // removeEntity //


//-----------------------------------------------
//			engageMelee()
//-----------------------------------------------
void CPhraseManager::engageMelee( const TDataSetRow &entity1, const TDataSetRow &entity2 ) 
{
	H_AUTO(CPhraseManager_engageMelee);
	
	if ( !TheDataset.isAccessible(entity1) || !TheDataset.isAccessible(entity2))
		return;
	
	//disengage from precedent combat if any, without deleting combat phrase
	disengage(entity1, true, true, false);
	
	_MapEntityToEngagedEntityInMeleeCombat.insert( make_pair(entity1,entity2) );
	
	TRowSetRowMap::iterator it = _MapEntityToMeleeAggressors.find( entity2 );
	
	if (it != _MapEntityToMeleeAggressors.end() )
	{
		(*it).second.insert( entity1 );
	}
	else
	{
		set<TDataSetRow> agg;
		agg.insert( entity1 );
		_MapEntityToMeleeAggressors.insert( make_pair( entity2, agg) );
	}
	
	// check mode if player
	if (TheDataset.getEntityId(entity1).getType() == RYZOMID::player)
	{
		CCharacter *character = PlayerManager.getChar(entity1);
		if ( character && character->getMode() != MBEHAV::COMBAT )
			character->setMode(MBEHAV::COMBAT);
	}
	
	// send message to clients to indicate the new combat
	PHRASE_UTILITIES::sendEngageMessages( TheDataset.getEntityId(entity1), TheDataset.getEntityId(entity2) );
} // engageMelee //


//-----------------------------------------------
//			engageMelee()
//-----------------------------------------------
void CPhraseManager::engageRange( const TDataSetRow &entity1, const TDataSetRow &entity2 ) 
{
	H_AUTO(CPhraseManager_engageRange);
	
	if ( !TheDataset.isAccessible(entity1) || !TheDataset.isAccessible(entity2))
		return;
	
	//disengage from precedent combat if any
	disengage(entity1, true, true, false);
	
	_MapEntityToEngagedEntityInRangeCombat.insert( make_pair(entity1,entity2) );
	
	TRowSetRowMap::iterator it = _MapEntityToRangeAggressors.find( entity2 );
	if (it != _MapEntityToRangeAggressors.end() )
	{
		(*it).second.insert( entity1 );
	}
	else
	{
		set<TDataSetRow> agg;
		agg.insert( entity1 );
		_MapEntityToRangeAggressors.insert( make_pair( entity2, agg) );
	}
	
	// change entity mode for COMBAT
	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( entity1 );
	if (entity == NULL)
	{
		nlwarning("<CBrickSentenceManager::engageRange> Invalid entity rowId %u", entity1.getIndex() );
		return;
	}
	
	// check mode if player
	if (TheDataset.getEntityId(entity1).getType() == RYZOMID::player)
	{
		CCharacter *character = PlayerManager.getChar(entity1);
		if ( character && character->getMode() != MBEHAV::COMBAT )
			character->setMode(MBEHAV::COMBAT);
	}
	
	// send message to clients to indicate the new combat
	PHRASE_UTILITIES::sendEngageMessages( TheDataset.getEntityId(entity1), TheDataset.getEntityId(entity2) );
} // engageRange //


//-----------------------------------------------
//			disengage()
//-----------------------------------------------
void CPhraseManager::disengage( const TDataSetRow &entityRowId,  bool sendChatMsg, bool disengageCreature, bool cancelCombatSentence)
{
	H_AUTO(CPhraseManager_disengage);
	
	CEntityId entityId;
	if (TheDataset.isAccessible(entityRowId))
	{
		// Clear the mirror target list
		CMirrorPropValueList<uint32> targetList( TheDataset, entityRowId, DSPropertyTARGET_LIST );
		targetList.clear();
		
		entityId = TheDataset.getEntityId(entityRowId);
		// only disengage players unless specified
		if (entityId.getType() != RYZOMID::player && !disengageCreature )
		{
			nlwarning("<CPhraseManager::disengage> Tried to disengage bot %s, cancel",entityId.toString().c_str() );
			return;
		}
	}
	
	//CEntityId entityTarget;
	TDataSetRow entityTargetRowId;
	
	CEntityBase* entityPtr = CEntityBaseManager::getEntityBasePtr( entityId );
	if (!entityPtr)
	{
		//nlwarning ("<CPhraseManager::disengage> invalid entityId %s",entityId.toString().c_str() );
		//return;
	}
	else
	{
		// if player and in mode combat, change mode to normal
		if (entityId.getType() == RYZOMID::player && entityPtr->getMode() == MBEHAV::COMBAT)
		{
			entityPtr->setMode( MBEHAV::NORMAL, false, false );
		}
	}
	
	//	_MapEntityToInitiatedCombat.erase(entityRowId);
	
	TRowRowMap::iterator it = _MapEntityToEngagedEntityInMeleeCombat.find( entityRowId );
	
	// was in melee combat
	if (it != _MapEntityToEngagedEntityInMeleeCombat.end() )
	{
		entityTargetRowId = (*it).second;
		_MapEntityToEngagedEntityInMeleeCombat.erase( entityRowId );
		
		DEBUGLOG("<CPhraseManager::disengage> Disengage entity Id %s from MELEE combat", entityId.toString().c_str() );
		
		// remove this entity from the aggressors of its previous target entity
		TRowSetRowMap::iterator itAgg = _MapEntityToMeleeAggressors.find( entityTargetRowId );
		if (itAgg != _MapEntityToMeleeAggressors.end() )
		{
			(*itAgg).second.erase( entityRowId );
			
			// if last aggressor, remove entry
			if ((*itAgg).second.empty() )
			{
				_MapEntityToMeleeAggressors.erase( itAgg );
			}
		}
		else
			nlwarning("<CPhraseManager::disengage> Error in _MapEntityToMeleeAggressors, should have found aggressor for entity %s",TheDataset.getEntityId(entityTargetRowId).toString().c_str() );
		
		// remove the aggressor from target aggressors
		CEntityBase* targetEntity = CEntityBaseManager::getEntityBasePtr( entityTargetRowId );
		if ( targetEntity )
		{
			//targetEntity->removeAgressor( entityId );
		}
	}
	// was in range combat
	else
	{
		it = _MapEntityToEngagedEntityInRangeCombat.find( entityRowId );
		if (it != _MapEntityToEngagedEntityInRangeCombat.end() )
		{
			entityTargetRowId = (*it).second;
			_MapEntityToEngagedEntityInRangeCombat.erase( entityRowId );
			DEBUGLOG("<CPhraseManager::disengage> Disengage entity Id %s from RANGE combat", entityId.toString().c_str() );		
		}
		else
			return; // not engaged in combat
		
		// remove this entity from the aggressors of its previous target entity
		TRowSetRowMap::iterator itAgg = _MapEntityToRangeAggressors.find( entityTargetRowId );
		if (itAgg != _MapEntityToRangeAggressors.end() )
		{
			(*itAgg).second.erase( entityRowId );
			
			// if last aggressor, remove entry
			if ((*itAgg).second.empty() )
				_MapEntityToRangeAggressors.erase( itAgg );
		}
		else
			nlwarning("<CPhraseManager::disengage> Error in _MapEntityToRangeAggressors, should have found aggressor for entity %s",TheDataset.getEntityId(entityTargetRowId).toString().c_str() );
		
		// remove the aggressor from target aggressors
		CEntityBase* targetEntity = CEntityBaseManager::getEntityBasePtr( entityTargetRowId );
		if ( targetEntity )
		{
			//			targetEntity->removeAgressor( entityId );
		}
	}
	
	INFOLOG("<CPhraseManager::disengage> Disengaging entity %s, was in combat with %s", entityId.toString().c_str(), TheDataset.getEntityId(entityTargetRowId).toString().c_str());
	
	if (cancelCombatSentence)
	{
		// cancel all combat sentences for that entity
		// disengage at end of the current action because if we break the latency the creatures could hit each tick when changing their target
		cancelAllCombatSentences(entityRowId, true);
	}
	
	// send message to players
	if (sendChatMsg)
		PHRASE_UTILITIES::sendDisengageMessages( entityId, TheDataset.getEntityId(entityTargetRowId));
} // disengage //

//--------------------------------------------------------------
//						cancelAllCombatSentences()  
//--------------------------------------------------------------
bool CPhraseManager::cancelAllCombatSentences( const TDataSetRow &entityRowId, bool disengageOnEndOnly)
{
	H_AUTO(CPhraseManager_cancelAllCombatSentences);
	
	bool returnValue = true;
	
	// find the entity execution list if any
	TMapIdToIndex::iterator itIndex = _PhrasesIndex.find( entityRowId );
	if (itIndex != _PhrasesIndex.end() )
	{
		BOMB_IF( (*itIndex).second >= _EntityPhrases.size(), "Index out of bound", return returnValue);
		
		returnValue = _EntityPhrases[(*itIndex).second].cancelCombatActions(entityRowId,disengageOnEndOnly);
	}
	
	return returnValue;
} // cancelAllCombatSentences //


//--------------------------------------------------------------
//						breakCast()  
//--------------------------------------------------------------
void CPhraseManager::breakCast( sint32 attackSkillValue, CEntityBase * entity, CEntityBase * defender)
{
	H_AUTO(CPhraseManager_breakCast);
	
	if ( EntitiesNoCastBreak )
		return;
	
	nlassert(entity);
	nlassert(defender);
	// try to get a magic phrase being cast (it the phrase at the bginning of the queue
	TMapIdToIndex::iterator it = _PhrasesIndex.find( defender->getEntityRowId() );
	if ( it != _PhrasesIndex.end() )
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return);
		CSPhrase * phrase = static_cast<CSPhrase*>(_EntityPhrases[(*it).second].getCurrentAction());
		
		if (!phrase)
			return;
		
		switch( phrase->getType() )
		{
		case BRICK_TYPE::MAGIC:
			{
				CMagicPhrasePtr magicPhrase = dynamic_cast< CMagicPhrase * > (phrase);
				
				// only break spells not already finished =)
				if ( magicPhrase && magicPhrase->state() != CSPhrase::Latent)
				{
					// compute average skill value of the phrase
					sint32 magicSkillValue = 0;
					
					if ( defender->getId().getType() == RYZOMID::player )
					{
						
						CCharacter *pC = dynamic_cast<CCharacter*> (defender);
						if (!pC)
						{
							nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", defender->getId().toString().c_str());
							return;
						}
						
						// compute average skill value
						for ( uint i = 0; i < magicPhrase->getSkills().size(); i++ )
						{
							magicSkillValue += pC->getSkillValue(magicPhrase->getSkills()[i]);
						}
						if (!magicPhrase->getSkills().empty())
						{
							magicSkillValue /= (sint32)magicPhrase->getSkills().size();
						}

						// boost magic skill for low level characters
						sint32 sb = (sint32)MagicSkillStartValue.get();
						magicSkillValue = max( sb, magicSkillValue ) ;

						// add magic boost from consumable
						magicSkillValue += pC->magicSuccessModifier();
					}
					else
					{
						const CStaticCreatures * form = defender->getForm();
						if ( !form )
						{
							nlwarning( "<MAGIC>invalid creature form %s in entity %s", defender->getType().toString().c_str(), defender->getId().toString().c_str() );
							return;
						}	
						magicSkillValue = form->getAttackLevel();
					}
					
					//test if the spell is broken
					uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::BreakCastResist, magicSkillValue + magicPhrase->getBreakResist() - attackSkillValue);
					uint8 roll = (uint8) RandomGenerator.rand(99);

					// add spire effect ( quantity )
					if ( entity->getId().getType() == RYZOMID::player  )
					{
						const CSEffect* pEffect = entity->lookForActiveEffect( EFFECT_FAMILIES::TotemHarvestQty );
						if ( pEffect != NULL )
						{
							chances = (uint8)( (sint32)chances + pEffect->getParamValue() );
						}
					}
					
					if ( roll >= chances )
					{
						_EntityPhrases[(*it).second].cancelTopPhrase();

						if ( entity->getId().getType() == RYZOMID::player  )
						{
							SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
							params[0].setEIdAIAlias( defender->getId(), CAIAliasTranslator::getInstance()->getAIAlias(defender->getId()) );
							PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(), "MAGIC_YOU_BREAK_ENEMY_CAST", params);
						}
					}
				}
			}
			break;
		case BRICK_TYPE::TIMED_ACTION:
			{
				CTimedActionPhrase *actionPhrase = dynamic_cast< CTimedActionPhrase * > (phrase);
				
				if ( actionPhrase && actionPhrase->state() != CSPhrase::Latent)
				{
					if ( actionPhrase->testCancelOnHit(attackSkillValue, entity, defender) == true )
						_EntityPhrases[(*it).second].cancelTopPhrase();
				}
			}
			break;
		default:;
		}
	}
}// breakCast


//--------------------------------------------------------------
//						breakLaunchingLinks()
//--------------------------------------------------------------
void CPhraseManager::breakLaunchingLinks(CEntityBase * entity)
{
	H_AUTO(CPhraseManager_breakLaunchingLinks);
	
	BOMB_IF( entity == NULL, "<CPhraseManager::breakLaunchingLink> entity should not be NULL", return );
	
	TMapIdToIndex::iterator itFind = _PhrasesIndex.find( entity->getEntityRowId() );
	if (itFind == _PhrasesIndex.end())
		return;
	
	BOMB_IF( (*itFind).second >= _EntityPhrases.size(), "Index out of bound", return);
	
	CEntityPhrases & entityPhrases = _EntityPhrases[(*itFind).second];
	TPhraseList & launchingActions = entityPhrases.getLaunchingActions();
	
	for (TPhraseList::iterator it = launchingActions.begin(); it != launchingActions.end(); ++it)
	{
		CSPhrasePtr & phrase = *it;
		BOMB_IF( phrase == NULL, "<CPhraseManager::breakLaunchingLink> phrase should not be NULL", continue );
		
		CMagicPhrase * magicPhrase = dynamic_cast<CMagicPhrase *>( (CSPhrase *)phrase );
		if (magicPhrase == NULL)
			continue;
		
		BOMB_IF(
			magicPhrase->getActor() != entity->getEntityRowId(),
			NLMISC::toString("<CPhraseManager::breakLaunchingLink> phrase actor %s should be %s", magicPhrase->getActor().toString().c_str(), entity->getEntityRowId().toString().c_str()),
			continue
			);
		
		magicPhrase->breakNewLink(true);
	}
	
} // breakLaunchingLinks

//--------------------------------------------------------------
//--------------------------------------------------------------
bool CPhraseManager::hasActionInProgress(TDataSetRow rowId)
{
	TMapIdToIndex::const_iterator it = _PhrasesIndex.find(rowId);
	if ( it != _PhrasesIndex.end())
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return false);
		
		if ( !_EntityPhrases[(*it).second].getCurrentAction().isNull() )
			return true;
		else
			return false;
	}
	return false;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
const CEntityPhrases *CPhraseManager::getEntityPhrases(TDataSetRow rowId) const 
{
	TMapIdToIndex::const_iterator it = _PhrasesIndex.find(rowId);
	if ( it != _PhrasesIndex.end())
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return NULL);
		return &_EntityPhrases[(*it).second];
	}
	return NULL;
}

//--------------------------------------------------------------
//						harvestDefault()  
//--------------------------------------------------------------
bool CPhraseManager::harvestDefault(const TDataSetRow &actorRowId, const CSheetId &rawMaterialSheet, uint16 minQuality, uint16 maxQuality, uint16 quantity, bool deposit )
{
	H_AUTO(CPhraseManager_harvestDefault);
	
	vector<CSheetId> bricks;
	static const CSheetId quarteringBrick("bhq01.sbrick");
	static const CSheetId foragingBrick("bhf01.sbrick");
	
	if (quarteringBrick == CSheetId::Unknown)
	{
		nlwarning("ERROR : cannot find quartering brick : bhq01.sbrick.");
		return false;
	}
	if (foragingBrick == CSheetId::Unknown)
	{
		nlwarning("ERROR : cannot find foraging brick : bhf01.sbrick.");
		return false;
	}
	
	if (deposit)
		bricks.push_back( foragingBrick );
	else
		bricks.push_back( quarteringBrick );
	
	TDataSetRow nullId;
	
	CSPhrasePtr phrase = buildSabrinaPhrase(actorRowId, nullId, bricks);
	CHarvestPhrase *harvestPhrase = dynamic_cast<CHarvestPhrase*> ( static_cast<CSPhrase*> (phrase) );
	if (!phrase)
	{
		return false;
	}
	if (!harvestPhrase)
	{
		//		delete phrase;
		return false;
	}
	
	harvestPhrase->minQuality(minQuality);
	harvestPhrase->maxQuality(maxQuality);
	harvestPhrase->quantity(quantity);
	harvestPhrase->setRawMaterial(rawMaterialSheet);
	//harvestPhrase->deposit(deposit);
	
	if (!addPhrase(actorRowId, phrase))
	{
		//		delete phrase;
		return false;
	}
	/*
	TMapIdToPhraseStruc::iterator it = _Phrases.find( actorRowId );
	// actor doesn't already have phrases
	if (it == _Phrases.end() )
	{
	// new entry
	CEntityPhrases entityPhrases;
	#ifdef NL_DEBUG
	entityPhrases.EntityId = TheDataset.getEntityId(actorRowId);
	#endif
	if (!entityPhrases.addPhraseFifo(phrase))
	{
	delete phrase;
	return false;
	}
	
	  _Phrases.insert( make_pair(actorRowId, entityPhrases) );
	  }
	  // actor already have phrases in the manager, just add the new one
	  else
	  {
	  CEntityPhrases &entityPhrases = (*it).second;
	  if (!entityPhrases.addPhraseFifo(phrase))
	  {
	  delete phrase;
	  return false;
	  }
	  }
	*/
	
	return true;
} // harvestDefault //


//--------------------------------------------------------------
//				cancelTopPhrase()  
//--------------------------------------------------------------
void CPhraseManager::cancelTopPhrase(const TDataSetRow &entityRowId, bool staticOnly)
{
	H_AUTO(CPhraseManager_cancelTopPhrase);
	
	TMapIdToIndex::iterator it = _PhrasesIndex.find( entityRowId );
	if (it != _PhrasesIndex.end() )
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return);
		_EntityPhrases[(*it).second].cancelTopPhrase(staticOnly);
	}
} // cancelTopPhrase //

//--------------------------------------------------------------
//				cancelAllPhrases()  
//--------------------------------------------------------------
void CPhraseManager::cancelAllPhrases(const TDataSetRow &entityRowId)
{
	H_AUTO(CPhraseManager_cancelAllPhrases);
	
	TMapIdToIndex::iterator it = _PhrasesIndex.find( entityRowId );
	if (it != _PhrasesIndex.end() )
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return);
		
		_EntityPhrases[(*it).second].cancelAllPhrases();
		
		CCharacter *character = PlayerManager.getChar((*it).first);
		if (character)
		{
			character->writeExecPhraseInDB(0);
			character->writeNextPhraseInDB(character->nextCounter());
			character->writeCycleCounterInDB();
		}
	}
} // cancelAllPhrases //

//--------------------------------------------------------------
//				updateNextCounterValue()
//--------------------------------------------------------------
void CPhraseManager::updateNextCounterValue( const TDataSetRow &entityRowId, uint8 counterValue )
{
	TMapIdToIndex::const_iterator it = _PhrasesIndex.find(entityRowId);
	if ( it != _PhrasesIndex.end())
	{
		BOMB_IF( (*it).second >= _EntityPhrases.size(), "Index out of bound", return);
		_EntityPhrases[(*it).second].updateNextCounterValue(counterValue);
	}
} // updateNextCounterValue //


NLMISC_COMMAND(displaySabrinaAllocation,"Display allocated / freed phrases and effects counters","")
{
	log.displayNL("Nb allocated phrases = %u", CSPhrase::NbAllocatedPhrases);
	log.displayNL("Nb freed phrases = %u", CSPhrase::NbDesallocatedPhrases);
	log.displayNL("Nb Phrase still allocated = %u", CSPhrase::NbAllocatedPhrases - CSPhrase::NbDesallocatedPhrases);
	
	log.displayNL("Nb allocated effects = %u", CSEffect::NbDesallocatedEffects);
	log.displayNL("Nb freed effects = %u", CSEffect::NbDesallocatedEffects);
	log.displayNL("Nb effects still allocated = %u", CSEffect::NbDesallocatedEffects - CSEffect::NbDesallocatedEffects);
	return true;
}

#ifdef NL_DEBUG

NLMISC_COMMAND(addBrickDebugParams,"add params to the current debug param list","<param description>")
{
	if( args.empty())
		return false;
	//DebugBrick.addParam(args[0]);
	return true;
}

NLMISC_COMMAND(useDebugBrick,"use debug Brick or not","<0/1>")
{
	if( args.empty())
	{
		//		log.displayNL("UseDebugBrick = %u", UseDebugBrick);
		return true;
	}
	
	//	NLMISC::fromString(args[0], UseDebugBrick);
	return true;
}

NLMISC_COMMAND(clearBrickDebugParams,"clear the parameters","")
{
	if( !args.empty())
		return false;
	//	DebugBrick.Params.clear();
	return true;
}

NLMISC_COMMAND(procItem,"force the use of an item proc","<player><sphrase>")
{
	if( args.size() != 2 )
		return false;
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar( id );
	if (!user)
		return true;
	CSheetId sheet(args[1]);
	const CStaticRolemasterPhrase* phrase = CSheets::getSRolemasterPhrase(sheet);
	if(!phrase)
		return true;
	CMagicPhrase ph;
	if ( ph.buildProc(user->getEntityRowId(),phrase->Bricks) )
		ph.procItem();
	return true;
}

#endif
