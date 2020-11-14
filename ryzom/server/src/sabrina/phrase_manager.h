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
#include "nel/misc/time_nl.h"
#include "nel/misc/sheet_id.h"
// std
#include <map>
// game_share
#include "game_share/shield_types.h"
#include "game_share/brick_types.h"
//#include "game_share/slot_equipment.h"
//#include "game_share/mirror_equipment.h"
#include "game_share/event_report.h"
#include "game_share/skills.h"
#include "game_share/tick_event_handler.h"
#include "game_share/ai_event.h"
#include "game_share/sentence_appraisal.h"
//#include "game_share/egs_sheets/egs_sheets.h"
#include "game_share/egs_sheets/egs_static_brick.h"
#include "game_share/game_item_manager/game_item_manager.h"
#include "s_phrase.h"
#include "combat.h"


extern CGameItemManager GameItemManager;

typedef std::vector<NLMISC::CSheetId>					TVectorSheetId;
typedef std::list<SEventReport>							TEventReportList;
typedef std::list<IAIEvent*>							TAIEventList;
typedef std::list<CSPhrase *>							TPhraseList;
typedef std::map<TDataSetRow, CCombat>					TMapRowidToCombat;



struct CEvalReturnInfos
{
	CEvalReturnInfos()
		:Cost(0),Valid(false), Appraisal(SENTENCE_APPRAISAL::Undefined){}
	uint32			Cost;
	//std::string		ReturnString;
	SENTENCE_APPRAISAL::ESentenceAppraisal Appraisal;
	bool			Valid;
};

/**
 * class used for all the sentences and sentence management for an entity
 */
class CEntityPhrases
{
public:
	/// default constructor
	CEntityPhrases() : CyclicInProgress(false), DefaultAttackUsed(false)
	{ CyclicAction = NULL; }

	/// destructor
	~CEntityPhrases()
	{}

	/// get max size of the FIFO
	inline static uint8  queueMaxSize() { return _QueueMaxSize; }
	/// set max size of the FIFO
	inline static void queueMaxSize(uint8 size) { _QueueMaxSize = size; }

	/// clear : delete the sentences
	void clear();

	/// set the cyclic action
	void setCyclicAction( CSPhrase *phrase);

	/// stop the cyclic action
	void stopCyclicAction(const TDataSetRow &entityRowId);

	/// create the default attack cyclic sentence if entity in combat without cyclic sentence
	void createDefaultAttackIfCombat( const TDataSetRow &actingEntityRowId );

	///
	void cancelAllPhrasesButFirstOne();

	/// cancel the top sentence 
	void cancelTopSentence(bool staticOnly = false);

	/// add a phrase in the FIFO
	bool addPhraseFifo( CSPhrase *phrase);

public:
	/// the entity current cyclic action (for combat)
	CSPhrase *		CyclicAction;
	/// set to true if the current sentence is the cyclic one, false otherwise
	bool			CyclicInProgress;
	/// flag set to true if the current cyclic action is the default_attack
	bool			DefaultAttackUsed;
	/// the entity sentence fifo
	TPhraseList		Fifo;

private:
	static uint8	_QueueMaxSize;

};

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CPhraseManager
{	
public:
	typedef std::map<TDataSetRow, CEntityPhrases>			TMapIdToPhraseStruc;
	typedef std::map<TDataSetRow, TDataSetRow>				TRowRowMap;
	typedef std::map<TDataSetRow, std::set<TDataSetRow> >	TRowSetRowMap;
public:
	/// getInstance
	static inline CPhraseManager *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CPhraseManager();
	
		return _Instance;
	}

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
	 * \param serviceName name of the registered service
	 */
	inline void registerService( const std::string &serviceName ) { _RegisteredServices.insert( serviceName ); }


	/**
	 * register a service to the event broadcast for AI
	 * \param serviceName name of the registered service
	 */
	inline void registerServiceForAI( const std::string &serviceName ) { _AIRegisteredServices.insert( serviceName ); }

	/**
	 * get the set of registered servcies for AI event reports
	 */
	inline const std::set<std::string> &registeredServiceForAI() { return _AIRegisteredServices; }

	/**
	 * get the set of registered services for event reports
	 */
	inline const std::set<std::string> &registeredService() { return _RegisteredServices; }

	
	/**
	 * unregister a service to the event broadcast
	 * \param serviceName name of the service to remove
	 */
	inline void unregisterService( const std::string &serviceName ) { _RegisteredServices.erase( serviceName ); }


	/**
	 * unregister a service to the event broadcast
	 * \param serviceName name of the service to remove
	 */
	inline void unregisterServiceForAI( const std::string &serviceName ) { _AIRegisteredServices.insert( serviceName ); }

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
	inline void addAiEventReport( const CAiEventReport &report )
	{
		_AIEventReports.push_back(report);
	}

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
	 */
	void removeEntity( const TDataSetRow &entityRowId);

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
	 * get the combat object for given entity, return NULL if no combat engaged
	 * \param attackerRowId attacker rowid
	 * \return adress of the Combat object or NULL if not found
	 */
	inline CCombat *getCombatInitiatedBy( const TDataSetRow &attackerRowId )
	{
		TMapRowidToCombat::iterator it = _MapEntityToInitiatedCombat.find( attackerRowId );
		if (it != _MapEntityToInitiatedCombat.end() )
			return &((*it).second);
		else
			return NULL;
	}

	/**
	 * if the acting entity has engaged another entity in Melee, get the Entity engaged, return  TDataSetRow() if no entity engaged
	 * \param entityRowId acting entity row id
	 * \return the entity of default value if no entity engaged
	 */
	inline TDataSetRow getEntityEngagedMeleeBy( const TDataSetRow &entityRowId) const
	{
		TRowRowMap::const_iterator it = _MapEntityToEngagedEntityInMeleeCombat.find( entityRowId );
		if (it != _MapEntityToEngagedEntityInMeleeCombat.end() )
			return (*it).second;
		else
			return TDataSetRow();
	}

	/**
	 * if the acting entity has engaged another entity in Range combat, get the Entity engaged, return  TDataSetRow() if no entity engaged
	 * \param entityRowId acting entity row id
	 * \return the entity of default value if no entity engaged
	 */
	inline TDataSetRow getEntityEngagedRangeBy( const TDataSetRow &entityRowId) const
	{
		TRowRowMap::const_iterator it = _MapEntityToEngagedEntityInRangeCombat.find( entityRowId );
		if (it != _MapEntityToEngagedEntityInRangeCombat.end() )
			return (*it).second;
		else
			return TDataSetRow();
	}

	
	/**
	 * get all the melee aggressors for specified entity
	 * \param entityRowId acting entity row id
	 * \return the set of aggressors
	 */
	inline const std::set<TDataSetRow> &getMeleeAggressors( const TDataSetRow &entityRowId ) const
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

	
	/**
	 * get all the range aggressors for specified entity
	 * \param entityId acting entity CEntityId
	 * \return the set of aggressors
	 */
	inline const std::set<TDataSetRow> &getRangeAggressors( const TDataSetRow &entityRowId ) const
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
	 * cancel Static action in progress
	 * \param entityRowId row id of the entity
	 */
	void cancelStaticActionInProgress( const TDataSetRow &actorRowId );

	/**
	 * check the validity (grammar and cost) of given phrase
	 * \param vector of bricks sheet ids
	 * \return true if the phrase is valid
	 */
	bool checkPhraseValidity( const std::vector<NLMISC::CSheetId> &brickIds ) const;

	/**
	 * execute a sabrina phrase
	 * \param actorRowId the rowid of the actor
	 * \param targetRowId rowid of the target
	 * \param brickIds ids of the bricks composing the phrase
	 * \param cyclic true if the sentence is cyclic (combat), false otherwise
	 */
	void executePhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId,  const std::vector<NLMISC::CSheetId> &brickIds, bool cyclic = false, uint16 phraseId = 0, uint8 nextCounter = 0 );

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
	 CSPhrase *buildSabrinaPhrase( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, const std::vector<NLMISC::CSheetId> &brickIds, uint16 phraseId = 0, uint8 nextCounter = 0);
//	 CSPhrase *buildSabrinaPhrase( const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, const std::vector<NLMISC::CSheetId> &brickIds )
//	 {
//		 return buildSabrinaPhrase( TheDataset.getDataSetRow(actorId), TheDataset.getDataSetRow(targetId), brickIds );
//	 }

	 /**
	  * An entity attempts to break a spell being cast
	  * \param attackSkillLevel: value of the skill of the atacker ( = skill /10)
	  * \param attacker: the attacking entity
	  * \param defender : the defender
	  */
	 void breakCast( sint32 attackSkillLevel, CEntityBase * entity, CEntityBase * defender);

private:
	/// private Constructor
	CPhraseManager();

	/**
	 * send the event reports to the registered services
	 */
	void sendEventReports();

	/**
	 * flush ai events
	 */
	void sendAIEvents();


private:
	/// unique instance
	static CPhraseManager*			_Instance;

	/// the map giving the phrase execution FIFO and the current cyclic action for a given entity
	TMapIdToPhraseStruc				_Phrases;

	/// list of registered services for Event Broadcast
	std::set<std::string>			_RegisteredServices;

	/// list of registered services for Event Broadcast for AI
	std::set<std::string>			_AIRegisteredServices;

	/// the list of the events to report
	TEventReportList				_EventReports;
	
	/// the list of ai events
	TAIEventList					_AIEvents;

	/// the list of ai event reports
	std::vector<CAiEventReport>		_AIEventReports;

	/// map entity row id to the related combat object if any
	TMapRowidToCombat				_MapEntityToInitiatedCombat;

	/// map entity ID with the entity they engaged in combat
	TRowRowMap						_MapEntityToEngagedEntityInMeleeCombat;

	/// map entities with the set of entities which have engaged them in melee combat
	TRowSetRowMap					_MapEntityToMeleeAggressors;
		
	/// map entity ID with the entity they have engaged in range combat
	TRowRowMap						_MapEntityToEngagedEntityInRangeCombat;

	/// map entities with the set of entities which have engaged them in range combat
	TRowSetRowMap					_MapEntityToRangeAggressors;
};


#endif // RY_PHRASE_MANAGER_H

/* End of phrase_manager.h */
