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



#ifndef RY_PHRASE_MANAGER_H
#define RY_PHRASE_MANAGER_H

// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/variable.h"
#include "nel/misc/singleton.h"

// game_share
#include "game_share/shield_types.h"
#include "game_share/brick_types.h"
#include "server_share/event_report.h"
#include "ai_share/ai_event.h"
#include "server_share/msg_brick_service.h"
//
#include "phrase_manager/s_phrase.h"
#include "entity_manager/entity_base.h"
#include "game_item_manager/game_item_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"


extern CGameItemManager GameItemManager;

typedef std::vector<NLMISC::CSheetId>					TVectorSheetId;
typedef std::list<SEventReport>							TEventReportList;
typedef std::list<IAIEvent*>							TAIEventList;
typedef std::list<CSPhrasePtr>							TPhraseList;

static const std::vector<NLMISC::CSheetId> EmptySheetVect;

struct CCyclicActionInfos
{
	TDataSetRow						ActorRowId;
	TDataSetRow						TargetRowId;
	std::vector<NLMISC::CSheetId>	CyclicActionBricks;

	CCyclicActionInfos() { }
	inline void reset() { CyclicActionBricks.clear(); }
};

static const CCyclicActionInfos NoCyclicInfo = CCyclicActionInfos();

class CPhraseManager;

/**
 * class used for all the phrase management for an entity
 */
class CEntityPhrases
{
public:
	/// default constructor
	CEntityPhrases(const NLMISC::CEntityId &entityId) : _EntityId(entityId)
	{}

	/// destructor
	~CEntityPhrases()
	{}

	/// set the cyclic action
	inline void setCyclicAction( CSPhrasePtr &phrase, const CCyclicActionInfos &infos)
	{
#ifdef NL_DEBUG
		nlassert(phrase != NULL);
#endif
		phrase->cyclic(true);
		_CyclicAction = phrase;
		_CyclicActionInfos = infos;

		// if no next action, set it
		if (_NextAction.isNull())
		{
			_NextAction = phrase;
			if (_CurrentAction.isNull())
				_CurrentAction = phrase;
		}
	}

	/**
	 * replace the current cyclic action with a newly created copy,
	 * the old cyclic action is discarded.
	 * \return the new cyclic action (the copy) or a NULL pointer if the cyclic action was NULL.
	 */
	CSPhrasePtr &renewCyclicAction();

	/// stop the cyclic action
	void stopCyclicAction(const TDataSetRow &entityRowId);
	
	/// cancel the top phrase
	inline void cancelTopPhrase(bool staticOnly = false)
	{
		if (_CurrentAction && _CurrentAction->beingProcessed())
			return;

		if (_CurrentAction && ( !staticOnly || _CurrentAction->isStatic()) )
		{
			if (_CurrentAction->state() >= CSPhrase::ExecutionInProgress)
				_CurrentAction->stop();
			else
				_CurrentAction->stopBeforeExecution();

			clearAttackFlag();

			// if current action is the cyclic one, re init phrase parameters
			if (_CurrentAction == _CyclicAction)
			{
			   _CurrentAction->evaluate();
			   _CurrentAction->setState(CSPhrase::Evaluated);
			}

			_CurrentAction = NULL;
			goToNextAction();
		}
	} 

	/// cancel all phrases
	inline void cancelAllPhrases()
	{
		if (_CurrentAction && _CurrentAction->beingProcessed())
			return;

		if (_CyclicAction && _CyclicAction != _NextAction && _CyclicAction != _CurrentAction)
		{
			_CyclicAction->stopBeforeExecution();
		}

		if (_NextAction)
		{
			if (_NextAction != _CurrentAction)
				_NextAction->stopBeforeExecution();

			_NextAction = NULL;
		}
		
		if (_CurrentAction)
		{
			if (_CurrentAction->state() >= CSPhrase::ExecutionInProgress)
				_CurrentAction->stop();
			else
				_CurrentAction->stopBeforeExecution();

			_CurrentAction = NULL;
		}
		
		_CyclicAction = NULL;
		_CyclicActionInfos.reset();

		clearAttackFlag();
	}

	/// set next phrase
	inline void setNextPhrase( CSPhrasePtr &phrase )
	{
#ifdef NL_DEBUG
		nlassert(phrase != NULL);
#endif
		phrase->cyclic(false);
		_NextAction = phrase;

		// if there is no current action, set current to next and next to cyclic
		if (_CurrentAction == NULL)
		{
			goToNextAction();
		}
	}

	/// get current action phrase
	inline CSPhrasePtr &getCurrentAction() 
	{ 
		if (_CurrentAction == NULL)
		{
			goToNextAction();
		}
		return _CurrentAction; 
	}

	// return the current action, or NULL if there is no current action (different from getCurrentAction())
	const CSPhrasePtr& getCurrentActionConst() const
	{
		return _CurrentAction;
	}

	// return the next action, or NULL if there is no next action
	const CSPhrasePtr& getNextActionConst() const
	{
		return _NextAction;
	}

	/// go to next action
	inline void goToNextAction() 
	{
		if (!_NextAction)
			_NextAction = _CyclicAction;
		_CurrentAction = _NextAction;
		_NextAction = _CyclicAction;
	}

	/// get launching actions
	inline TPhraseList &getLaunchingActions()
	{
		return _LaunchingActions;
	}

	/** 
	 * cancel combat actions (usually after a disengage...)
	 * \param playerId rowId of the entity
	 * \param disengageOnEndOnly if set to true, set the related flag if a combat action if found active, and return true
	 */
	bool cancelCombatActions(const TDataSetRow &entityRowId, bool disengageOnEndOnly);
	
	// Debug : dump entity phrases infos
	void dumpPhrasesInfos( NLMISC::CLog &log ) const;

	/**
	 * called when the 'nest counter' value of the nextAction must be updated
	 * \param counterValue the new counter value
	 */
	inline void updateNextCounterValue(uint8 counterValue)
	{
		if (_NextAction)
			_NextAction->nextCounter(counterValue);
	}

	// free actor (clear attack actor)
	void clearAttackFlag();

private:
	/// the entity current cyclic action if any
	CSPhrasePtr			_CyclicAction;

	/// if any cyclic action, keep the infos to build it (needed for 'missile' actions added to _LaunchingActions)
	CCyclicActionInfos	_CyclicActionInfos;

	/// the current action in progress
	CSPhrasePtr			_CurrentAction;

	/// the next action
	CSPhrasePtr			_NextAction;

	/// launching actions
	TPhraseList			_LaunchingActions;

	/// associated entityId
	NLMISC::CEntityId	_EntityId;
};

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CPhraseManager : public NLMISC::CSingleton<CPhraseManager>
{	
public:
	//typedef std::map<TDataSetRow, CEntityPhrases>				TMapIdToPhraseStruc;
	typedef CHashMap<TDataSetRow, TDataSetRow, TDataSetRow::CHashCode>				TRowRowMap;
	typedef CHashMap<TDataSetRow, std::set<TDataSetRow>, TDataSetRow::CHashCode>	TRowSetRowMap;
	typedef CHashMap<TDataSetRow, uint32, TDataSetRow::CHashCode>					TMapIdToIndex;
public:
/*	/// getInstance
	static inline CPhraseManager *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CPhraseManager();
	
		return _Instance;
	}
*/
	/// Destructor
	virtual ~CPhraseManager() {}

	/// updatePhrases
	void updatePhrases();

	/// add the callbacks to the service callback array
	void addCallbacks();

	/// init method
	void init();

	/**
	 * register a service to the event broadcast
	 * \param serviceId sid of the registered service
	 */
	inline void registerService(NLNET::TServiceId serviceId ) { _RegisteredServices.insert( serviceId ); }


	/**
	 * register a service to the event broadcast for AI
	 * \param serviceName name of the registered service
	 */
	inline void registerServiceForAI( NLNET::TServiceId serviceId ) { _AIRegisteredServices.insert( serviceId ); }

	/**
	 * get the set of registered servcies for AI event reports
	 */
	inline const std::set<NLNET::TServiceId> &registeredServiceForAI() { return _AIRegisteredServices; }

	/**
	 * get the set of registered services for event reports
	 */
	inline const std::set<NLNET::TServiceId> &registeredService() { return _RegisteredServices; }

	
	/**
	 * unregister a service to the event broadcast
	 * \param serviceName name of the service to remove
	 */
	inline void unregisterService( NLNET::TServiceId serviceId ) { _RegisteredServices.erase( serviceId ); }


	/**
	 * unregister a service to the event broadcast
	 * \param serviceName name of the service to remove
	 */
	inline void unregisterServiceForAI( NLNET::TServiceId serviceId ) { _AIRegisteredServices.erase( serviceId ); }

	/**
	 * add an event report for the current tick
	 * \param report the event report to add
	 */
	inline void addEventReport( const SEventReport &report)
	{
		_EventReports.push_back( report);		
	}

	/**
	 * add an AI event report for the current tick
	 * \param report the AI event report to add
	 */
	void addAiEventReport( const CAiEventReport &report );

	/**
	 * add an AI event for the current tick
	 * \param report the event to add
	 */
	inline void addAIEvent( IAIEvent *event )
	{
		if (event != NULL)
			_AIEvents.push_back( event );
	}

	/**
	 * called when an entity dies or a player is disconnected
	 * \param entityRowId rowId of the entity to remove
	 * \param removeRightNow true if entity must be removed without waiting (player disconnection for instance)
	 */
	void removeEntity( const TDataSetRow &entityRowId, bool removeRightNow = false);

	/**
	 * engage entity 1 with entity 2 in melee combat
	 * \param entity1 the entity engaging entity2
	 * \param entity2 the entity being engaged by entity1
	 */
	void engageMelee( const TDataSetRow &entity1, const TDataSetRow &entity2 );

	/**
	 * engage entity 1 with entity 2 in range combat
	 * \param entity1 the entity engaging entity2
	 * \param entity2 the entity being engaged by entity1
	 */
	void engageRange( const TDataSetRow &entity1, const TDataSetRow &entity2 );

	/**
	 * clear engaged entities map for melee combat
	 */
	void clearMeleeEngagedEntities();

	/**
	 * if the acting entity has engaged another entity in Melee, get the Entity engaged, return  TDataSetRow() if no entity engaged
	 * \param entityRowId acting entity row id
	 * \return the entity of default value if no entity engaged
	 */
	TDataSetRow getEntityEngagedMeleeBy( const TDataSetRow &entityRowId) const;

	/**
	 * if the acting entity has engaged another entity in Range combat, get the Entity engaged, return  TDataSetRow() if no entity engaged
	 * \param entityRowId acting entity row id
	 * \return the entity of default value if no entity engaged
	 */
	TDataSetRow getEntityEngagedRangeBy( const TDataSetRow &entityRowId) const;

	/**
	 * get all the melee aggressors for specified entity
	 * \param entityRowId acting entity row id
	 * \return the set of aggressors
	 */
	const std::set<TDataSetRow> &getMeleeAggressors( const TDataSetRow &entityRowId ) const;

	/**
	 * get all the range aggressors for specified entity
	 * \param entityId acting entity CEntityId
	 * \return the set of aggressors
	 */
	const std::set<TDataSetRow> &getRangeAggressors( const TDataSetRow &entityRowId ) const;

	/*
	 * disengage an entity from combat
	 * \param entityRowId the entity disengaging from combat
	 * \param sendChatMsg true if must send message to the clients
	 * \param cancelCombatSentence true if combat sentence should be canceled, false otherwise
	 * \param 
	 */
	void disengage( const TDataSetRow &entityRowId, bool sendChatMsg, bool disengageCreature = false, bool cancelCombatSentence = true );

	/**
	 * cancel all combat sentences of that player
	 * \param playerId rowId of the entity
	 * \param disengageOnEndOnly if set to true, set the related flag if a combat sentrence if found active, and return true
	 */
	bool cancelAllCombatSentences( const TDataSetRow &playerRowId, bool disengageOnEndOnly );

	/**
	 * check the validity (grammar and cost) of given phrase
	 * \param vector of bricks sheet ids
	 * \return true if the phrase is valid
	 */
	bool checkPhraseValidity( const std::vector<NLMISC::CSheetId> &brickIds ) const;

	/**
	 * use a consumable item, apply it's effect
	 * \param actorRowId the rowid of the actor
	 * \param item the item to consume
	 * \param quality quality of the item consumed
	 * \return pointer on the phrase
	 */
	CSPhrasePtr useConsumableItem( const TDataSetRow &actorRowId, const CStaticItem *itemForm, uint16 quality );

	/**
	 * execute a sabrina phrase
	 * \param actorRowId the rowid of the actor
	 * \param targetRowId rowid of the target
	 * \param brickIds ids of the bricks composing the phrase
	 * \param cyclic true if the sentence is cyclic (combat), false otherwise
	 * \return pointer on the phrase
	 */
	CSPhrasePtr executePhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const std::vector<NLMISC::CSheetId> &brickIds, bool cyclic = false, uint16 phraseId = 0, uint8 nextCounter = 0, bool enchant = false, bool needToValidate = true );

	/**
	 * execute a sabrina phrase
	 * \param actorRowId the rowid of the actor
	 * \param targetRowId rowid of the target
	 * \param phraseSheet sheetid of the phrase to execute
	 * \param cyclic true if the sentence is cyclic (combat), false otherwise
	 * \return pointer on the phrase
	 */
	CSPhrasePtr executePhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const NLMISC::CSheetId &phraseSheet, bool cyclic = false, uint16 phraseId = 0, uint8 nextCounter = 0 , bool enchant = false, bool needToValidate = true);

	/**
	 * execute an AI action
	 * \param actorRowId the rowid of the actor
	 * \param targetRowId rowid of the target
	 * \param actionId sheetid of the ai action
	 */
	void executeAiAction( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const NLMISC::CSheetId &actionId, float damageCoeff = 1.0f, float speedCoeff = 1.0f );

	/**
	 * cancel the top phrase of given entity
	 * \param entityRowId row id of the entity doing the action
	 * \param flag, set to true if the actoin must only be canceled if it's a static action
	 */
	void cancelTopPhrase(const TDataSetRow &entityRowId, bool staticOnly = false);

	/**
	 * cancel all the phrases of given entity
	 * \param entityRowId row id of the entity
	 */
	void cancelAllPhrases(const TDataSetRow &entityRowId);

	/**
	 * called when the 'nest counter' value of the nextAction must be updated
	 * \param entityRowId row id of the entity doing the action
	 * \param counterValue the new counter value
	 */
	void updateNextCounterValue( const TDataSetRow &entityRowId, uint8 counterValue );

	/**
	 * create a default harvest phrase
	 * \param actorRowId the rowid of the actor
	 * \param rawMaterialSheet harvested raw material
	 * \param minQuality harvested raw material min quality
	 * \param maxQuality harvested raw material max quality
	 * \param quantity harvested raw material quantity
	 * \param deposit true if the entity harvest a deposit
	 * \return false if the phrase cannot be built
	 */
	bool harvestDefault(const TDataSetRow &actorRowId, const NLMISC::CSheetId &rawMaterialSheet, uint16 minQuality, uint16 maxQuality, uint16 quantity, bool deposit = false );
		
	/**
	 * entity1 attacks entity2 using a default attack or any other attacks
	 * \param attackerId id of the attacker
	 * \param targetId id of the attacked entity
	 */
	void defaultAttackSabrina( const TDataSetRow &attackerRowId, const TDataSetRow &targetRowId );
	inline void defaultAttackSabrina( const NLMISC::CEntityId &attackerId, const NLMISC::CEntityId &targetId )
	{
		defaultAttackSabrina( TheDataset.getDataSetRow(attackerId), TheDataset.getDataSetRow(targetId) );
	}
	
	/**
	  * build a sabrina phrase from the given set of bricks
	  * \param actorRowId actor row id
	  * \param targetRowId primary target row id
	  * \param brickIds the brick sheet ids
	  * \return pointer on the built phrase, NULL if failed
	  */
	 CSPhrasePtr buildSabrinaPhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, const std::vector<NLMISC::CSheetId> &brickIds, uint16 phraseId = 0, uint8 nextCounter = 0, bool execution = true);

	 /**
	  * An entity attempts to break a spell being cast
	  * \param attackSkillLevel: value of the skill of the atacker ( = skill /10)
	  * \param attacker: the attacking entity
	  * \param defender : the defender
	  */
	 void breakCast( sint32 attackSkillLevel, CEntityBase * entity, CEntityBase * defender);


	 /**
	  * Break links launched by entity (links that have not been created yet).
	  * Links will be created when missile will reach the target, but will immediately be broken.
	  */
	 void breakLaunchingLinks(CEntityBase * entity);

	 /**
	  * return true if given entity has an action in progress, false otherwise
	  */
	 bool hasActionInProgress(TDataSetRow rowId);

	 /**
	  * get entity phrases (for debug)
	  */
	 const CEntityPhrases *getEntityPhrases(TDataSetRow rowId) const;

	 /// get current nb of entities in manager
	 uint32 getNbEntitiesInManager() const { return (uint32)_PhrasesIndex.size(); }

	 /// get max nb of entities in manager
	 uint32 getMaxNbEntitiesInManager() const { return _MaxNbEntities; }

private:
	/// private Constructor (singleton)
//PhraseManager();

	/** 
	 * update entity current action, called by updatePhrases for each entity
	 */
	void updateEntityCurrentAction( const TDataSetRow &entityId, CEntityPhrases &entityPhrases);

	/**
	 * send the event reports to the registered services
	 */
	void sendEventReports();

	/**
	 * flush ai events
	 */
	void sendAIEvents();

	/**
	 * add a sabrina phrase to actor phrases
	 * \param actorRowId the rowid of the actor
	 * \param phrase pointer on the phrase to execute
	 * \return true if phrase can be added to structure
	 */
	bool addPhrase( const TDataSetRow &actorRowId, CSPhrasePtr &phrase, const CCyclicActionInfos &cyclicInfos = NoCyclicInfo, bool cyclic = false);

	/**
     * Execute an instant action (like a power), bypass queue, immediate execution taking no time
	 * \param phrasePtr pointer on the phrase to execute
	 * \param needTovalidate false to bypass validation tests(when launched by consumable items)
	 */
	void executeNoTime( CSPhrasePtr &phrasePtr, bool needTovalidate = true);

	/**
	 * remove dead or disconnected entities
	 */
	void removeEntities();

private:
	/// unique instance
//	static CPhraseManager*			_Instance;

	/// map giving entity phrases index in vector for given entity row id
	TMapIdToIndex					_PhrasesIndex;

	/// vector of entity Phrases
	std::vector<CEntityPhrases>		_EntityPhrases;

	/// list of free index in vector
	std::vector<uint32>				_FreeIndex;

	/// list of registered services for Event Broadcast
	std::set<NLNET::TServiceId>		_RegisteredServices;

	/// list of registered services for Event Broadcast for AI
	std::set<NLNET::TServiceId>		_AIRegisteredServices;

	/// the list of the events to report
	TEventReportList				_EventReports;
	
	/// the list of ai events
	TAIEventList					_AIEvents;

	/// the list of ai event reports
	std::vector<CAiEventReport>		_AIEventReports;

	/// map entity ID with the entity they engaged in combat
	TRowRowMap						_MapEntityToEngagedEntityInMeleeCombat;

	/// map entities with the set of entities which have engaged them in melee combat
	TRowSetRowMap					_MapEntityToMeleeAggressors;
		
	/// map entity ID with the entity they have engaged in range combat
	TRowRowMap						_MapEntityToEngagedEntityInRangeCombat;

	/// map entities with the set of entities which have engaged them in range combat
	TRowSetRowMap					_MapEntityToRangeAggressors;

	/// list of entities to remove from manager
	std::list<TDataSetRow>			_EntitiesToRemove;

	/// max number of entities in manager
	uint32							_MaxNbEntities;
};


#endif // RY_PHRASE_MANAGER_H

/* End of phrase_manager.h */
