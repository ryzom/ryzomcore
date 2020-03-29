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




#ifndef RY_S_PHRASE_H
#define RY_S_PHRASE_H

#include "nel/misc/types_nl.h"
#include "game_item_manager/game_item.h"

class CStaticBrick; 
struct CEvalReturnInfos;

/**
 * Base virtual class for all Sabrina phrases
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSPhrase
{
public:
	enum TPhraseState
	{
		New = 0,
		Evaluated,
		Validated,
		//Idle,
		ExecutionInProgress,
		SecondValidated,
		WaitNextCycle,
		Latent,
		LatencyEnded,

		UnknownState,
	};

public:
	/// Constructor
	CSPhrase() 
	{
		_State = New;
		_Idle = false;
		_ExecutionEndDate = 0;
		_LatencyEndDate = 0;
		_NbWaitingRequests = 0;
		_PhraseBookIndex = 0;
		_NextCounter = 0;
		_IsStatic = false;
	}

	/// destructor
	virtual ~CSPhrase()
	{}

	/// get Id
	//inline const TPhraseId &getId() const { return _Id; }

	/// get internal state
	inline TPhraseState state() const { return _State; }

	/// get idle flag
	inline bool idle() const { return _Idle; }
	
	/// set idle flag
	inline void idle( bool idle ) { _Idle = idle; }

	/// get execution end date
	inline NLMISC::TGameCycle executionEndDate() const { return _ExecutionEndDate; }

	/// get latency end date
	inline NLMISC::TGameCycle latencyEndDate() const { return _LatencyEndDate; }

	/// get nb of waiting request to other services
	inline uint16 getNbWaitingRequests() const { return _NbWaitingRequests; }

	/// add a waiting request
	inline void addRequest() { ++_NbWaitingRequests; }

	/// remove a waiting request
	inline void removeRequest() { if (_NbWaitingRequests) --_NbWaitingRequests;	}

	/**
	 * build the phrase from bricks, actor and main target
	 * \param actorRowId: row of the entity creating the phrase
	 * \param bricks : bricks composing the phrase
	 * \return true on success
	 */
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks ) = 0;

	/**
	 * set the actor
	 */
	virtual void setActor( const TDataSetRow &entityRowId ) = 0;

	/**
	 * set the primary item
	 * \param itemPtr pointer on primary item 
	 */
//	virtual void setPrimaryItem( CGameItemPtr itemPtr ) = 0;

	/**
	 * set the secondary item
	 * \param itemPtr pointer on secondary item 
	 */
//	virtual void setSecondaryItem( CGameItemPtr itemPtr ) = 0;

	/**
	 * add a consumable ressource (object)
	 * \param itemPtr pointer on the consumable item
	 */
//	virtual void addConsumableItem( CGameItemPtr itemPtr ) = 0;

	/**
	 * set the primary target
	 * \param entityId id of the primary target
	 */
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId ) = 0;

	/**
	 * add a target entity
	 * \param entityId id of the target
	 */
	virtual void addTargetEntity( const TDataSetRow &entityRowId ) = 0;

	/**
	 * evaluate phrase
	 * \param evalReturnInfos struct that will receive evaluation results
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate(CEvalReturnInfos *msg = NULL) = 0;

	/**
	 * validate phrase
	 * \return true of the phrase is valid
	 */
	virtual bool validate() = 0;

	/**
	 * update phrase
	 * \return true of the phrase is valid
	 */
	virtual bool update() = 0;

	/**
	 * execute this phrase
	 */
	virtual void execute() = 0;

	/**
	 * at end of execution, apply phrase
	 */ 
	virtual void apply() = 0;

	/**
	 * called at the end of the latency time
	 */
	virtual void end() = 0;

	/**
	 * called for brutal stop of the phrase
	 */
	virtual void stop() { end(); }

	/// get being processed flag
	inline bool beingProcessed() const { return _BeingProcessed; }

	/// 
	inline uint8 nextCounter() const { return _NextCounter; }
	inline void nextCounter(uint8 c) { _NextCounter = c; }
	///
	inline sint16 phraseBookIndex() const { return _PhraseBookIndex; }
	inline void phraseBookIndex(sint16 i) { _PhraseBookIndex = i; }
	/// 
	inline bool isStatic() const { return _IsStatic; }
	inline void isStatic(bool b) { _IsStatic = b; }

protected:
	/// phrase state
	TPhraseState			_State;
	/// idle or not
	bool					_Idle;
	/// execution end date
	NLMISC::TGameCycle		_ExecutionEndDate;
	/// latency end date
	NLMISC::TGameCycle		_LatencyEndDate;
	/// nb of request to other services (GPMS for area effects)
	uint16					_NbWaitingRequests;
	///	index in client phrase book (0 = not in the phrase book)
	uint16					_PhraseBookIndex;
	///	next counter
	uint8					_NextCounter;
	/// is static
	bool					_IsStatic;

	/// being processed flag
	mutable bool			_BeingProcessed;
};


#endif // RY_S_PHRASE_H

/* End of s_phrase.h */
