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
#include "phrase_manager.h"
#include "phrase_manager_callbacks.h"
#include "combat_phrase.h"
#include "s_phrase_factory.h"
#include "player_manager.h"
#include "character.h"
#include "phrase_utilities_functions.h"
#include "magic_phrase.h"
#include "harvest_phrase.h"
#include "game_share/entity_structure/statistic.h"

/////////////
// USING
/////////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RY_GAME_SHARE;

/////////////
// GLOBALS 
/////////////
CPhraseManager					*CPhraseManager::_Instance = NULL;
uint8							CEntityPhrases::_QueueMaxSize = 2;

bool							FIFOFullReplaceOrDiscard = false;

/////////////
// EXTERN 
/////////////
extern CPhraseManager			*PhraseManager;

// FIFOFullReplaceOrDiscard 
NLMISC_COMMAND(FIFOFullReplaceOrDiscard,"toggle the behaviour when phrase fifo is full( replace old one or discrad new one)","")
{
	FIFOFullReplaceOrDiscard = !FIFOFullReplaceOrDiscard;

	log.displayNL("FIFOFullReplaceOrDiscard is %s",FIFOFullReplaceOrDiscard?"replace":"discard");
	return true;
}


//--------------------------------------------------------------
//		CEntityPhrases : clear
//--------------------------------------------------------------
void CEntityPhrases::clear()
{
	if (CyclicAction != NULL)
	{
		delete CyclicAction;
		CyclicAction = NULL;
	}

	const TPhraseList::iterator itEnd = Fifo.end();
	for (TPhraseList::iterator it = Fifo.begin() ; it != itEnd ; ++it)
	{
		if (*it != NULL)
		{
			delete (*it);
			(*it) = NULL;
		}
	}
	Fifo.clear();
	CyclicInProgress = false;
	DefaultAttackUsed = false;
} // clear //

//--------------------------------------------------------------
//		CEntityPhrases::setCyclicAction()
//--------------------------------------------------------------
void CEntityPhrases::setCyclicAction( CSPhrase *phrase)
{
	if (!phrase)
		return;

	// test new sentence validity (if not valid, keep old sentence)
	string errorCode;
	if (phrase->evaluate(NULL) == false || phrase->validate() == false)
	{
		DEBUGLOG("<CEntityPhrases::setCyclicAction> Invalid sentence tested, error code = ");
		/*// if error code begins with '(' do not send it !
		if ( !errorCode.empty() && errorCode[0] != '(' )
		{
			// inform player
			PHRASE_UTILITIES::sendSimpleMessage( sentence->getPlayerId(), errorCode );
		}
		*/
		// delete the sentence
		delete phrase;
		return;
	}

	if ( CyclicAction != NULL)
	{
		// if in progress, set repeat mode to false and insert this sentence in front of the sentence fifo
		if ( CyclicInProgress )
		{
			Fifo.push_front(CyclicAction);
			CyclicInProgress = false;
		}
		// not in progress, simply delete it
		else
		{
			delete CyclicAction;
		}
	}

	CyclicAction = phrase;
	
	//const CBrick *rootBrick = sentence->getRootBrick();
	//if ( rootBrick != NULL && rootBrick->name() == "Default attack")
	//	DefaultAttackUsed = true;
	//else
		DefaultAttackUsed = false;
} // setCyclicAction //

//--------------------------------------------------------------
//		CEntityPhrases::stopCyclicAction()
//--------------------------------------------------------------
void CEntityPhrases::stopCyclicAction(const TDataSetRow &entityRowId)
{
	if ( CyclicAction != NULL)
	{
		CCharacter *character = PlayerManager.getChar(entityRowId);
		// if in progress, set repeat mode to false and insert this sentence in front of the sentence fifo
		if ( CyclicInProgress )
		{
			Fifo.push_front(CyclicAction);
			CyclicInProgress = false;
			DefaultAttackUsed = false;			
		}
		// not in progress, simply delete it
		else
		{
			delete CyclicAction;
		}

		CyclicAction = NULL;

		if (character)
		{
			character->writeCycleCounterInDB();
		}
	}
} // stopCyclicAction //


//--------------------------------------------------------------
//		CEntityPhrases::createDefaultAttackIfCombat()
//--------------------------------------------------------------
void CEntityPhrases::createDefaultAttackIfCombat( const TDataSetRow &actingEntityRowId )
{
	const CEntityId &actingEntityId = TheDataset.getEntityId(actingEntityRowId);

	if (CyclicAction != NULL)
		return;

	if (DefaultAttackUsed)
		return;

	// check entity has engaged a combat
	TDataSetRow targetRowId = PhraseManager->getEntityEngagedMeleeBy(actingEntityRowId);
	if ( !targetRowId.isValid() || !TheDataset.isDataSetRowStillValid(targetRowId) )
	{
		targetRowId = PhraseManager->getEntityEngagedRangeBy(actingEntityRowId);
	}

	if ( !targetRowId.isValid() || !TheDataset.isDataSetRowStillValid(targetRowId))
		return;
	
	// create new sentence
	DEBUGLOG("Create default attack for entity %s", actingEntityId.toString().c_str() );

	vector<CSheetId>	bricks;
	bricks.push_back( CSheetId("test_default_attack.sbrick") );
	CSPhrase *phrase = PhraseManager->buildSabrinaPhrase( actingEntityRowId, targetRowId, bricks );
	if (phrase)
	{
		CyclicAction = phrase;
	}
	else
		nlwarning("<CEntityPhrases::createDefaultAttackIfCombat> Failed to create default attack phrase for entity %s",actingEntityId.toString().c_str() );

	DefaultAttackUsed = true;
} // createDefaultAttackIfCombat //


//--------------------------------------------------------------
//		CEntityPhrases::cancelAllPhrasesButFirstOne()
//--------------------------------------------------------------
void CEntityPhrases::cancelAllPhrasesButFirstOne()
{
	if (CyclicInProgress)
	{
		TPhraseList::iterator it;
		for (it = Fifo.begin() ; it != Fifo.end() ; ++it)
		{
			// delete the sentence
			if ( (*it) != NULL )
				delete (*it);
		}
		
		// clear every sentences
		Fifo.clear();
	}
	else
	{
		if (CyclicAction)
		{
			delete CyclicAction;
			CyclicAction = NULL;				
		}

		// skip the first sentence if any
		TPhraseList::iterator it = Fifo.begin();
		TPhraseList::iterator itSec;
		if (it != Fifo.end())
			itSec = ++it;
		else
			return;

		// clear every other sentences
		for ( ; it != Fifo.end() ; ++it)
		{
			// remove the sentence
			if ( (*it) != NULL )
				delete (*it);
		}
		
		// clear every sentences
		Fifo.erase(itSec, Fifo.end());
	}
} // CEntityPhrases::cancelAllPhrasesButFirstOne //

//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
void CEntityPhrases::cancelTopSentence(bool staticOnly)
{
	if (CyclicInProgress && CyclicAction && (!staticOnly || CyclicAction->isStatic()) )
	{
		CyclicAction->stop();
		delete CyclicAction;
		CyclicAction = NULL;
	}
	else
	{
		TPhraseList::iterator it = Fifo.begin();
		if (it != Fifo.end())
		{
			if ( ((*it) != NULL) && (!staticOnly || (*it)->isStatic()))
			{
				(*it)->stop();
				delete (*it);
				Fifo.pop_front();
			}			
		}
	}
} // CEntityPhrases::cancelTopSentence //

//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
bool CEntityPhrases::addPhraseFifo( CSPhrase *phrase)
{
	if (!phrase)
		return false;

	// check the fifo queue isn't already at max size
	if ( Fifo.size() >= _QueueMaxSize )
	{			
		if ( FIFOFullReplaceOrDiscard == true)
		{
			if ( (*Fifo.begin()) != 0)
			{
				DEBUGLOG("<CEntityPhrases::addPhraseFifo> FIFO is full (contains %u elts), delete 1st phrase and push new one",_QueueMaxSize );
				delete (*Fifo.begin());
			}
			else
			{
				DEBUGLOG("<CEntityPhrases::addPhraseFifo> FIFO is full (contains %u elts), 1st phrase is NULL push new one",_QueueMaxSize );
			}

			Fifo.pop_front();
		}
		else
		{
			DEBUGLOG("<CEntityPhrases::addPhraseFifo> FIFO is full (contains %u elts), do not add new phrase",_QueueMaxSize );
			delete phrase;
			return false;
		}
	}

	Fifo.push_back(phrase);
	
	return true;
} // CEntityPhrases::cancelTopSentence //

//--------------------------------------------------------------
//						CPhraseManager()  
//--------------------------------------------------------------
CPhraseManager::CPhraseManager()
{
} // CPhraseManager

//--------------------------------------------------------------
//						updatePhrases()  
//--------------------------------------------------------------
void CPhraseManager::updatePhrases()
{
	string errorString;

	// update first sentence in each player sentences Fifo 
	TMapIdToPhraseStruc::iterator it;
	for (it = _Phrases.begin() ; it != _Phrases.end() ; )
	{
		bool deletePhrase = false; // true if an error occurs
		bool executionEnd = false; // true if sentence execution has ended

		CSPhrase *phrase = NULL;
		CEntityPhrases &entityPhrases = (*it).second;

		if ( entityPhrases.CyclicAction == NULL )
		{
//			entityPhrases.createDefaultAttackIfCombat( (*it).first );
		}

		if ( entityPhrases.CyclicAction != NULL && (entityPhrases.CyclicInProgress == true || entityPhrases.Fifo.empty()) )
		{
			phrase = entityPhrases.CyclicAction;
			if (entityPhrases.DefaultAttackUsed == true && entityPhrases.CyclicInProgress == false)
			{
				// send message to client
				PHRASE_UTILITIES::sendSimpleMessage( (*it).first, "EGS_START_DEFAULT_ATTACK" );
			}
			entityPhrases.CyclicInProgress = true;
		}
		else
		{
			// get the first sentence if any
			if ( ! entityPhrases.Fifo.empty() )
				phrase = * entityPhrases.Fifo.begin();
		}
		
		if (phrase == NULL)
		{
			TMapIdToPhraseStruc::iterator itDel = it;
			++it;
			_Phrases.erase(itDel);
			continue;
		}

		// if the phrase is being executed
		if ( phrase->state() == CSPhrase::SecondValidated || phrase->state() == CSPhrase::ExecutionInProgress || phrase->state() == CSPhrase::Latent || phrase->state() == CSPhrase::WaitNextCycle )
		{
			// if phrase execution time ended, make a second validation test
			// if the phrase execution delay time has ended, apply phrase effects
			const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
			if ( phrase->executionEndDate() <= time && phrase->state() == CSPhrase::ExecutionInProgress && phrase->getNbWaitingRequests() == 0 && !phrase->idle())
			{
				// second validation of phrase if not already done
				//nlwarning("Second Validate of phrase for entity %s",(*it).first.toString().c_str() );
				if ( ! phrase->validate(/* errorString*/) )
				{
				//	DEBUGLOG("<CPhraseManager::update> For entity %s error validating a phrase during second validation, error code = %s",phrase->getPlayerId().toString().c_str(), errorString.c_str() );

					// set player behaviour to failed (if he was casting for exemple)
 				/*	PHRASE_UTILITIES::sendUpdateBehaviour( phrase->getPlayerId(), phrase->failureBehaviour() );

					// inform the player of the problem
					// if error code begins with '(' do not send it !
					if ( !errorString.empty() && errorString[0] != '(' )
					{						
						PHRASE_UTILITIES::sendSimpleMessage( phrase->getPlayerId(), errorString );
					}
				*/	
					deletePhrase = true;
				}
			}

			if ( !deletePhrase)
			{
				if ( ! phrase->update(/*errorString*/) )
				{
				/*	DEBUGLOG("<CPhraseManager::update> For entity %s, error updating a phrase, error code = %s", phrase->getPlayerId().toString().c_str(),errorString.c_str() );
					
					// inform the player of the problem
					// if error code begins with '(' do not send it !
					if ( !errorString.empty() && errorString[0] != '(' )
					{
						PHRASE_UTILITIES::sendSimpleMessage( phrase->getPlayerId(), errorString );
					}
*/
					phrase->stop();
					deletePhrase = true;
				}
				// test the end of the execution
				//else if ( phrase->executionHasEnded() )
				else if ( phrase->state() == CSPhrase::LatencyEnded )
				{
					executionEnd = true;
				}
			}
		}		
		else if ( phrase->idle() )
		{
			// phrase is idle, update and execute if no longer idle
			if ( ! phrase->update(/*errorString*/) )
			{
			/*	DEBUGLOG("<CPhraseManager::update> For entity %s, error updating a phrase, error code = %s", phrase->getPlayerId().toString().c_str(),errorString.c_str() );
					// inform the player of the problem
				// if error code begins with '(' do not send it !
				if ( !errorString.empty() && errorString[0] != '(' )
				{
					PHRASE_UTILITIES::sendSimpleMessage( phrase->getPlayerId(), errorString );
				}
*/
				phrase->stop();
				deletePhrase = true;
			}

			if (!phrase->idle() && phrase->state() == CSPhrase::Validated )
			{
				phrase->execute();
				// if entity is a player, update DB
				CCharacter *character = PlayerManager.getChar((*it).first);
				if (character)
				{
					character->writeExecPhraseInDB(phrase->phraseBookIndex(), phrase->nextCounter());
				}
			}
		}
		// the phrase is updated for the first time validate and execute it
		else
		{
			// if phrase has never been evaluated, evaluate it
			if ( phrase->state() == CSPhrase::New )
			{
				bool eval;
				
/*				if (phrase->getId() != PhraseNullId)
				{
					CEvalReturnInfos infos;
					eval = phrase->evaluate(&infos);

					CCharacter *player = PlayerManager.getChar( phrase->getPlayerId() );
					if (player != NULL)
					{
						player->sentenceEvaluationResult( infos.Valid, infos.Appraisal, infos.Cost, phrase->sentenceId().first, phrase->sentenceId().second);
					}
				}
				else
				*/
					eval = phrase->evaluate();

				if (!eval)				
				{
					//nlwarning("For player %s", phrase->getPlayerId().toString().c_str() );
					nlwarning("	Invalid phrase tested - should NEVER happens !!!!!!!!");

					deletePhrase = true;
				}
			}

			// validate phrase
			if ( ! phrase->validate(/*errorString*/) )
			{
/*				DEBUGLOG("<CPhraseManager::update> For entity %s error validating a phrase, error code = %s",phrase->getPlayerId().toString().c_str(), errorString.c_str() );

				// inform the player of the problem
				// if error code begins with '(' do not send it !
				if ( !errorString.empty() && errorString[0] != '(' )
				{
					PHRASE_UTILITIES::sendSimpleMessage( phrase->getPlayerId(), errorString );
				}
*/
				deletePhrase = true;
			}
			//if sentense isn't idle, execute it
			else if ( ! phrase->idle() )
			{
				// determine success
		//		phrase->determineSuccess( NULL );
				
				// test dodge and resists
		//		if (phrase->success())
		//			phrase->testDodgeAndResist( NULL );

				// if all target have not dodge, we consider the phrase as a success for the mission system
				/// TODO : there could be a better way to determine mission success
			/*	CCharacter *player = PlayerManager.getChar( phrase->getPlayerId() );
				if (player != NULL)
				{
					list<CSentenceElement*>::const_iterator itEl = phrase->getElements().begin();
					for (;itEl != phrase->getElements().end(); ++itEl)
					{
						const CBrick *brick = dynamic_cast<CBrick*> (*itEl);
						if (brick != NULL)
						{
							if ( !phrase->targetDodgeFlags(0) )
							{
								CMissionEventUseBrick event( phrase->getPlayerId(),phrase->getTarget(),brick->getSheetId(),brick->getCareer(),brick->getJob() );
								player->processMissionEvent(event);
							}
							const std::vector<NLMISC::CEntityId> targets = phrase->getSecondaryTargets();
							for ( uint i = 0; i < phrase->getSecondaryTargets().size(); ++i )
							{
								if ( !phrase->targetDodgeFlags(i+1) == 0)
								{
									CMissionEventUseBrick event( phrase->getPlayerId(),phrase->getSecondaryTargets()[i],brick->getSheetId(),brick->getCareer(),brick->getJob() );
									player->processMissionEvent(event);
								}
							}
						}
					}
				}
			*/	// execute phrase
				phrase->execute();
				// if entity is a player, update DB
				CCharacter *character = PlayerManager.getChar((*it).first);
				if (character)
				{
					character->writeExecPhraseInDB(phrase->phraseBookIndex(), phrase->nextCounter());
				}
			}
		}

		if (deletePhrase == true)
		{
			/*CCharacter *character = PlayerManager.getChar((*it).first);
			if (character)
			{
				character->writeExecPhraseInDB(0, phrase->nextCounter());
			}
			*/
			phrase->stop();
			if (entityPhrases.CyclicInProgress == true)
			{
				entityPhrases.CyclicInProgress = false;
				delete entityPhrases.CyclicAction;
				entityPhrases.CyclicAction = NULL;

				// if the deleted phrase wasn't already the default attack, create a default attack as the cyclic phrase
				if ( !entityPhrases.DefaultAttackUsed)
				{
//					entityPhrases.createDefaultAttackIfCombat( (*it).first );
				}
				// if the deleted phrase was the default attack -> disengage from combat
				else
				{
					//DEBUGLOG("Disengage entity %s as the default attack validation has failed", ((*it).first).toString().c_str() );
					disengage((*it).first, true, true);
				}
			}
			else
			{
				delete phrase;
				entityPhrases.Fifo.pop_front();
			}
		}
		else if(executionEnd == true)
		{
			// if entity is a player, update DB
		/*	CCharacter *character = PlayerManager.getChar((*it).first);
			if (character)
			{
				character->writeExecPhraseInDB(0, phrase->nextCounter());
			}
			*/

			if (entityPhrases.CyclicInProgress == true )
			{
				entityPhrases.CyclicAction->evaluate();
				if ( !entityPhrases.Fifo.empty() )
					entityPhrases.CyclicInProgress = false;
			}
			else
			{
				delete phrase;
				entityPhrases.Fifo.pop_front();
			}
		}

		// get next entity sentences
		++it;
	}


	// send all the waiting event reports
	sendEventReports();
	//
	sendAIEvents();

} // updatePhrases //


//-----------------------------------------------
//			sendEventReports()
//-----------------------------------------------
void CPhraseManager::sendEventReports()
{
	if ( _EventReports.empty() && _AIEventReports.empty())
		return;

	if ( !_RegisteredServices.empty() )
	{
		// send to registered services
		CMessage msgReport("EVENT_REPORTS");
		msgReport.serialCont(_EventReports);

		set<string>::iterator it;
		for (it = _RegisteredServices.begin() ; it != _RegisteredServices.end() ; ++it)
		{
			sendMessageViaMirror (*it, msgReport);
			// 
			INFOLOG("Send EVENT_REPORTS to service %s", (*it).c_str() );
		}
	}

	if ( !_AIEventReports.empty() && !_AIRegisteredServices.empty() )
	{
		// send to registered services for AI 
		CBSAIEventReportMsg msgAI;
		const uint nbAiReports = _AIEventReports.size();
		for (uint i = 0 ; i < nbAiReports ; ++i )
		{
			msgAI.pushBack( _AIEventReports[i] );
		}

		set<string>::iterator it;
		for (it = _AIRegisteredServices.begin() ; it != _AIRegisteredServices.end() ; ++it)
		{
			msgAI.send (*it );
			INFOLOG("Send EVENT_REPORTS to AI service %s", (*it).c_str() );
		}
	}

	_EventReports.clear();
	_AIEventReports.clear();
//	_AggroWeights.clear();
} // sendEventReports //



//-----------------------------------------------
//			sendAIEvents()
//-----------------------------------------------
void CPhraseManager::sendAIEvents()
{
	if (_AIEvents.empty())
		return;

	// send to registered services
	CMessage msgai("AI_EVENTS");
	uint16 size = _AIEvents.size();
	msgai.serial( size );

	TAIEventList::iterator it;
	const TAIEventList::iterator itEnd = _AIEvents.end();

	for (it = _AIEvents.begin() ; it != itEnd ; ++it)
	{
		msgai.serial( *(*it) );
		delete (*it);
		*it = NULL;
	}

	_AIEvents.clear();

	set<string>::iterator itservice;
	for (itservice = _AIRegisteredServices.begin() ; itservice != _AIRegisteredServices.end() ; ++itservice)
	{
		sendMessageViaMirror (*itservice, msgai);
		INFOLOG("Send AI_EVENTS to AI service %s", (*itservice).c_str() );
	}
	
} // sendAIEvents //


//-----------------------------------------------
//			executePhrase()
//-----------------------------------------------
void CPhraseManager::executePhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const std::vector<NLMISC::CSheetId> &brickIds, bool cyclic, uint16 phraseId, uint8 nextCounter )
{
	CSPhrase *phrase = buildSabrinaPhrase(actorRowId, targetRowId, brickIds, phraseId, nextCounter);
	if (!phrase)
	{
		nlwarning("<CPhraseManager::executePhrase> Failed to build sabrina phrase for actor %u", actorRowId.getIndex());
		return;
	}
	phrase->setPrimaryTarget( targetRowId );

	TMapIdToPhraseStruc::iterator it = _Phrases.find( actorRowId );
	// actor doesn't already have phrases
	if (it == _Phrases.end() )
	{	
		// new entry
		CEntityPhrases entityPhrases;
		
		if (cyclic)
		{
			entityPhrases.setCyclicAction(phrase);
			entityPhrases.CyclicInProgress = true;
		}
		else
		{
			entityPhrases.addPhraseFifo(phrase);
		}
		
		_Phrases.insert( make_pair(actorRowId, entityPhrases) );
	}
	// actor already have phrases in the manager, just add the new one
	else
	{
		CEntityPhrases &entityPhrases = (*it).second;
		if (cyclic)
		{
			entityPhrases.setCyclicAction(phrase);
		}
		else
		{
			entityPhrases.addPhraseFifo(phrase);
		}
	}
} // executePhrase //



//-----------------------------------------------
//			init()
//-----------------------------------------------
void CPhraseManager::init()
{
	addCallbacks();

	PHRASE_UTILITIES::loadLocalisationTable( CPath::lookup("localisation.localisation_table" ) );
} // init //

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
//			checkPhraseValidity()
//-----------------------------------------------
bool CPhraseManager::checkPhraseValidity( const std::vector<NLMISC::CSheetId> &brickIds ) const
{
	/*
	vector<CSheetId>::const_iterator it;
	vector<CSheetId>::const_iterator itEnd = brickIds.end();

	set<BRICK_FAMILIES::TBrickFamily> mandatoryFamilies;	
	set<BRICK_FAMILIES::TBrickFamily> allowedFamilies;
	set<BRICK_FAMILIES::TBrickFamily> forbiddenFamilies;

	set<BRICK_FAMILIES::TBrickFamily> foundFamilies;

	sint16 totalCost = 0;

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
		forbiddenFamilies.insert( brick->ForbiddenFamilies.begin(), brick->ForbiddenFamilies.end() );

		if ( foundFamilies.insert( brick->Family ).second == false)
		{
			nlwarning("<checkPhraseValidity> The family %s were already found in phrase, error", BRICK_FAMILIES::toString(brick->Family).c_str() );
			return false;
		}

		totalCost += brick->SabrinaValue;
	}

	// check cost
	if (totalCost > 0)
	{
		DEBUGLOG("<checkPhraseValidity> Creadit must be > to cost for a phrase to be valid, cancel return false");
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

	// check no forbidden is present
	itbEnd = forbiddenFamilies.end();
	for (itb = forbiddenFamilies.begin() ; itb != itbEnd ; ++itb)
	{
		if (foundFamilies.find(*itb) != foundFamilies.end() )
		{
			DEBUGLOG("<checkPhraseValidity> The family %s is forbidden but is in the phrase, cancel return false", BRICK_FAMILIES::toString(*itb).c_str() );
			return false;
		}
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
	*/

	return true;
} // checkPhraseValidity //


//-----------------------------------------------
//			checkPhraseValidity()
//-----------------------------------------------
CSPhrase *CPhraseManager::buildSabrinaPhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, const vector<CSheetId> &brickIds, uint16 phraseId, uint8 nextCounter )
{
	if (brickIds.empty())
		return NULL;

	if (!checkPhraseValidity(brickIds))
		return NULL;

	CSPhrase *phrase = ISPhraseFactory::buildPhrase(actorRowId,brickIds);
	if (!phrase)
	{
		nlwarning("<CPhraseManager::buildSabrinaPhrase> For entity %u, factory returns a NULL pointer instead of phrase!. First brick is %s", actorRowId.getIndex(),brickIds[0].toString().c_str());
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
	CEntityId attackerId = TheDataset.getEntityId(attackerRowId);
	CEntityId targetId = TheDataset.getEntityId(targetRowId);

	DEBUGLOG("<CPhraseManager::attacks> entity %d attacks entity %d", attackerId.toString().c_str(), targetId.toString().c_str() );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( attackerId );
	
	if (entity == NULL)
	{
		nlwarning("<CPhraseManager::attacks> Invalid entity Id %s", attackerId.toString().c_str() );
		return;
	}

	// cancel entity static action
	if ( entity->getId().getType() == RYZOMID::player ) 
	{
		( (CCharacter*)entity)->cancelStaticActionInProgress();
	}
	entity->cancelStaticEffects();

	// check if attacker is already in combat
	CEntityId id ;// = getEntityEngagedMeleeBy( attackerId );
	if (id == CEntityId::Unknown )
	{
		//id = getEntityEngagedRangeBy( attackerId );
	}

	if (id != CEntityId::Unknown )
	{
		TMapIdToPhraseStruc::iterator it = _Phrases.find( attackerRowId );
		if (it == _Phrases.end() )
		{
			// error should not happens
			nlwarning("<CPhraseManager::defaultAttackSabrina> ERROR Cannot find entity sentences for entity Id %s but entity is engaged !, should never occurs (serious)", attackerId.toString().c_str() );
			return;
		}
		CEntityPhrases &entityPhrases = (*it).second;

		// check if the engaged target and the new one are different
		if (id != targetId)
		{
			// close first combat, program the action for disengage on end //
			entityPhrases.cancelAllPhrasesButFirstOne();
			
			if (entityPhrases.CyclicInProgress)
			{
			//	entityPhrases.CyclicAction->disengageOnEnd(true);
			}
			else
			{
				TPhraseList::iterator it = entityPhrases.Fifo.begin();
				if (it != entityPhrases.Fifo.end() && (*it) != NULL);
				{
			//		(*it)->disengageOnEnd(true);
				}
			}
		}

		vector<CSheetId>	bricks;
		bricks.push_back( CSheetId("test_default_attack.sbrick") );
		CSPhrase *phrase = CPhraseManager::buildSabrinaPhrase( attackerRowId, targetRowId, bricks );
		if (phrase)
		{
			(*it).second.setCyclicAction( phrase );
		}
		else
		{
			nlwarning("<CPhraseManager::attacks> ERROR while creating default attack sentence for entity Id %s", attackerId.toString().c_str() );
			return;
		}
	}
	else
	{
		// attacks
		vector<CSheetId>	bricks;
		bricks.push_back( CSheetId("test_default_attack.sbrick") );
		//executePhrase(attackerId, targetId, bricks);
		CSPhrase *phrase = CPhraseManager::buildSabrinaPhrase( attackerRowId, targetRowId, bricks );
		if (!phrase)
		{
			nlwarning("Error when creating default sabrina attack, cancel");
			return;
		}

		TMapIdToPhraseStruc::iterator it = _Phrases.find( attackerRowId );
		if (it == _Phrases.end() )
		{	
			// new entry
			CEntityPhrases entityPhrases;
			
			entityPhrases.setCyclicAction(phrase);
			entityPhrases.CyclicInProgress = true;
			
			_Phrases.insert( make_pair(attackerRowId, entityPhrases) );
		}
		else
		{
			CEntityPhrases &entityPhrases = (*it).second;
			entityPhrases.setCyclicAction(phrase);
		}
	}		
} // defaultAttackSabrina //

//--------------------------------------------------------------
//					removeEntity()  
//
// Precondition: the entity is in mirror
//--------------------------------------------------------------
void CPhraseManager::removeEntity( const TDataSetRow &entityRowId)
{
	// disengage entity if he was engaged in combat
	disengage( entityRowId, false, true );

	//disengage all the entities which were in melee combat with the removed entity
	TRowSetRowMap::iterator it = _MapEntityToMeleeAggressors.find( entityRowId );
	if (it != _MapEntityToMeleeAggressors.end() )
	{
		set<TDataSetRow> ids = (*it).second;
		set<TDataSetRow>::iterator itId;
		for ( itId = ids.begin() ; itId != ids.end() ; ++itId )
		{
			if (cancelAllCombatSentences( *itId, true))
				disengage( *itId, false, true);
		}
		//_MapEntityToMeleeAggressors.erase( it ); // automatically done by disengaging all aggressors
	}

	//disengage all the entities which were in range combat with the removed entity
	it = _MapEntityToRangeAggressors.find( entityRowId );
	if (it != _MapEntityToRangeAggressors.end() )
	{
		set<TDataSetRow> ids = (*it).second;
		set<TDataSetRow>::iterator itId;
		for ( itId = ids.begin() ; itId != ids.end() ; ++itId )
		{
			if (cancelAllCombatSentences( *itId, true))
				disengage( *itId, false, true );
		}
		//_MapEntityToRangeAggressors.erase( it ); // automatically done by disengaging all aggressors
	}

	// find the entity phrases execution list if any
	TMapIdToPhraseStruc::iterator itEntityPhrase = _Phrases.find( entityRowId );
	if ( itEntityPhrase != _Phrases.end() )
	{
		// remove the entry
		(*itEntityPhrase).second.clear();
		_Phrases.erase( itEntityPhrase );
		INFOLOG("<CPhraseManager::removeEntity> Removed entity (row %u)", entityRowId.getIndex() );
	}
} // removeEntity //


//-----------------------------------------------
//			engageMelee()
//-----------------------------------------------
void CPhraseManager::engageMelee( const TDataSetRow &entity1, const TDataSetRow &entity2 ) 
{
	//disengage from precedent combat if any, without deleting combat phrase
	disengage(entity1, true, true, false);

	_MapEntityToInitiatedCombat.insert( make_pair(entity1, CCombat(entity1, entity2, true) ) );

	_MapEntityToEngagedEntityInMeleeCombat.insert( make_pair(entity1,entity2) );

	TRowSetRowMap::iterator it = _MapEntityToMeleeAggressors.find( entity2 );


	// add the aggressor to target aggressors
	CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entity2 );
	if ( targetEntity )
	{
//		targetEntity->addAgressor( entity1 );
	}

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
	//disengage from precedent combat if any
	disengage(entity1, true, true, false);

	_MapEntityToInitiatedCombat.insert( make_pair(entity1, CCombat(entity1, entity2, false) ) );
	
	_MapEntityToEngagedEntityInRangeCombat.insert( make_pair(entity1,entity2) );

	// add the aggressor to target aggressors
	CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entity2 );
	if ( targetEntity )
	{
//		targetEntity->addAgressor( entity1 );
	}

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
	CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( entity1 );
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
	//entity->setMode( MBEHAV::COMBAT );
	
	// send message to clients to indicate the new combat
	PHRASE_UTILITIES::sendEngageMessages( TheDataset.getEntityId(entity1), TheDataset.getEntityId(entity2) );
} // engageRange //


//-----------------------------------------------
//			disengage()
//-----------------------------------------------
void CPhraseManager::disengage( const TDataSetRow &entityRowId,  bool sendChatMsg, bool disengageCreature, bool cancelCombatSentence)
{
	CEntityId entityId = TheDataset.getEntityId(entityRowId);
	// only disengage players unless specified
	if (entityId.getType() != RYZOMID::player && !disengageCreature )
	{
		nlwarning("<CPhraseManager::disengage> Tried to disengage bot %s, cancel",entityId.toString().c_str() );
		return;
	}
	
	//CEntityId entityTarget;
	TDataSetRow entityTargetRowId;

	CEntityBase* entityPtr = PHRASE_UTILITIES::entityPtrFromId( entityId );
	if (!entityPtr)
	{
		//nlwarning ("<CPhraseManager::disengage> WARNING invalid entityId %s",entityId.toString().c_str() );
		return;
	}

	// if player and in mode combat, change mode to normal
	if (entityId.getType() == RYZOMID::player && entityPtr->getMode() == MBEHAV::COMBAT)
	{
		entityPtr->setMode( MBEHAV::NORMAL, false, false );
	}

	_MapEntityToInitiatedCombat.erase(entityRowId);
	
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
		CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entityTargetRowId );
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

			CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( entityRowId );
			if (entity == NULL)
			{
				nlwarning("<CPhraseManager::disengage> Invalid entity Id %s", entityId.toString().c_str() );		
			}
			else
			{
				// change entity mode for Normal mode
				entity->setMode( MBEHAV::NORMAL, true );
			}
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
		CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entityTargetRowId );
		if ( targetEntity )
		{
//			targetEntity->removeAgressor( entityId );
		}
	}
	
	INFOLOG("<CPhraseManager::disengage> Disengaging entity %s, was in combat with %s", entityId.toString().c_str(), TheDataset.getEntityId(entityTargetRowId).toString().c_str());

	if (cancelCombatSentence)
	{
		// cancel all combat sentences for that entity
		cancelAllCombatSentences( entityRowId, false);
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
	bool returnValue = true;
	
	// find the player execution list if any
	TMapIdToPhraseStruc::iterator itEntityPhrase = _Phrases.find( entityRowId );
	if ( itEntityPhrase != _Phrases.end() )
	{
		CEntityPhrases &entityPhrases = (*itEntityPhrase).second;

		// manage cyclic sentence
		if ( entityPhrases.CyclicAction != NULL /*&& entityPhrases.CyclicAction->getType() == BRICK_TYPE::COMBAT*/)
		{
			//if (entityPhrases.CyclicInProgress)
			{
				entityPhrases.stopCyclicAction(entityRowId);
			}
			/*else
			{
				entityPhrases.CyclicAction->stop();
				delete entityPhrases.CyclicAction;
				entityPhrases.CyclicAction = NULL;
				entityPhrases.CyclicInProgress = false;
				// if entity is a player, write the cyclic counter in DB
				CCharacter *character = PlayerManager.getChar(entityRowId);
				if (character)
				{
					character-writeCycleCounterInDB();
				}
			}
			*/
		}

		// non-cyclic sentence
		vector<TPhraseList::iterator> delVect;
		// get the first sentence if any
		TPhraseList &phrases = entityPhrases.Fifo;
				
		TPhraseList::iterator it = phrases.begin();
		if (it != phrases.end() /*&& (*it)->getType() == BRICK_TYPE::COMBAT*/)
		{
			CCombatPhrase *combatPhrase = dynamic_cast<CCombatPhrase*> (*it);
			if (combatPhrase != 0)
			{
				// if the sentence is currently executed, do nothing, else, erase it like all the others
				if ( combatPhrase->beingProcessed() == false && !combatPhrase->disengageOnEnd() )
				{
					combatPhrase->stop();

					// remove the sentence
					delVect.push_back(it);
					if ( combatPhrase != NULL )
						delete combatPhrase;
				}
				else if (disengageOnEndOnly)
				{
					combatPhrase->disengageOnEnd( true );
					returnValue = false;
				}

				if ( it == phrases.begin() )
				{
					CCharacter *character = PlayerManager.getChar(entityRowId);
					if (character)
					{
						character->writeExecPhraseInDB(0, combatPhrase->nextCounter());
					}
				}
				
				++it;
			}
		}

		for ( ; it != phrases.end() ; ++it)
		{
			if ( dynamic_cast<CCombatPhrase*> (*it) != 0)
			{
				// remove the sentence
				delVect.push_back(it);
				if ( (*it) != NULL )
					delete (*it);
			}
		}
		
		// clear every sentences
		vector<TPhraseList::iterator>::iterator itdel;
		for ( itdel = delVect.begin() ; itdel != delVect.end() ; ++itdel)
		{
			phrases.erase( *itdel );
		}
	}

	// CODE
	/////////------------------------------------- TEMP PATCH
	// find the entity phrases execution list if any
	/*TMapIdToPhraseStruc::iterator itEntityPhrase = _Phrases.find( entityRowId );
	if ( itEntityPhrase != _Phrases.end() )
	{
		// remove the entry
		(*itEntityPhrase).second.clear();
		_Phrases.erase( itEntityPhrase );
	}
	/////////------------------------------------- TEMP PATCH
	*/
	return returnValue;
} // cancelAllCombatSentences //


//--------------------------------------------------------------
//						breakCast()  
//--------------------------------------------------------------
void CPhraseManager::breakCast( sint32 attackSkillValue, CEntityBase * entity, CEntityBase * defender)
{
	nlassert(entity);
	nlassert(defender);
	// try to get a magic phrase being cast (it the phrase at the bginning of the queue
	TMapIdToPhraseStruc::iterator it = _Phrases.find( defender->getEntityRowId() );
	if ( it != _Phrases.end() )
	{
		if ( (*it).second.Fifo.begin() != (*it).second.Fifo.end() )
		{
			CMagicPhrase * magicPhrase = dynamic_cast< CMagicPhrase * > ( *( (*it).second.Fifo.begin() ) );
			if ( magicPhrase )
			{
				// compute average skill value of the phrase
				sint skillValue = 0;
				for ( uint i = 0; i < magicPhrase->getSkills().size(); i++ )
				{
					SSkill * skill = entity->getSkills().getSkillStruct( magicPhrase->getSkills()[i] );
					if ( skill )
					{
						skillValue+= skill->Current;
					}
					else
					{
						nlwarning("<CMagicPhrase apply> invalid skill %d",magicPhrase->getSkills()[i] );
						return;
					}
				}
				//test if the spell is broken
				const uint8 chances = PHRASE_UTILITIES::getSuccessChance( (attackSkillValue - skillValue - magicPhrase->getBreakResist() )/10 );
				const uint8 roll = (uint8) RandomGenerator.rand(99);
				float successFactor = PHRASE_UTILITIES::getSucessFactor(chances, roll);
				if ( successFactor >= 1 )
					(*it).second.cancelTopSentence();
			}
		}
	}
}// breakCast


//--------------------------------------------------------------
//						breakCast()  
//--------------------------------------------------------------
bool CPhraseManager::harvestDefault(const TDataSetRow &actorRowId, const CSheetId &rawMaterialSheet, uint16 minQuality, uint16 maxQuality, uint16 quantity, bool deposit )
{
	vector<CSheetId> bricks;
	const CSheetId quarteringBrick("root_harvest_default.sbrick");
	const CSheetId foragingBrick("root_harvest_default.sbrick");

	if (quarteringBrick == CSheetId::Unknown)
	{
		nlwarning("ERROR : cannot find quartering brick : root_harvest_default.sbrick.");
		return false;
	}
	if (foragingBrick == CSheetId::Unknown)
	{
		nlwarning("ERROR : cannot find foraging brick : root_harvest_default.sbrick.");
		return false;
	}

	if (deposit)
		bricks.push_back( foragingBrick );
	else
		bricks.push_back( quarteringBrick );
			
	TDataSetRow nullId;
	
	CSPhrase *phrase = buildSabrinaPhrase(actorRowId, nullId, bricks);
	CHarvestPhrase *harvestPhrase = dynamic_cast<CHarvestPhrase*> (phrase);
	if (!phrase)
	{
		return false;
	}
	if (!harvestPhrase)
	{
		delete phrase;
		return false;
	}

	harvestPhrase->minQuality(minQuality);
	harvestPhrase->maxQuality(maxQuality);
	harvestPhrase->quantity(quantity);
	harvestPhrase->setRawMaterial(rawMaterialSheet);
	harvestPhrase->deposit(deposit);

	TMapIdToPhraseStruc::iterator it = _Phrases.find( actorRowId );
	// actor doesn't already have phrases
	if (it == _Phrases.end() )
	{	
		// new entry
		CEntityPhrases entityPhrases;
		
		entityPhrases.addPhraseFifo(phrase);
		
		_Phrases.insert( make_pair(actorRowId, entityPhrases) );
	}
	// actor already have phrases in the manager, just add the new one
	else
	{
		CEntityPhrases &entityPhrases = (*it).second;
		entityPhrases.addPhraseFifo(phrase);
	}

	return true;
} // harvestDefault //


//--------------------------------------------------------------
//				cancelStaticActionInProgress()  
//--------------------------------------------------------------
void CPhraseManager::cancelStaticActionInProgress(const TDataSetRow &actorRowId)
{
	TMapIdToPhraseStruc::iterator it = _Phrases.find( actorRowId );
	if (it != _Phrases.end() )
	{
		(*it).second.cancelTopSentence(true);
	}
} // cancelStaticActionInProgress //


