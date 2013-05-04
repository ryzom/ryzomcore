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

#include "game_item_manager/game_item.h"
#include "game_share/brick_types.h"

class CStaticBrick;

/**
 * Base virtual class for all Sabrina phrases
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSPhrase : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CSPhrase);
public:

	enum TPhraseState
	{
		New = 0,
		Evaluated,
		Validated,
		ExecutionInProgress,
		SecondValidated,
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
		_ApplyDate = ~0; // ensure apply() never called before launch()
		_LatencyEndDate = 0; // by default, time between apply() and end() is immediate
		_PhraseBookIndex = 0;
		_NextCounter = 0;
		_IsStatic = false;
		_IsApplied = false;
		_BeingProcessed = false;
		_PhraseType = BRICK_TYPE::UNKNOWN;

		++NbAllocatedPhrases;
	}

	/// destructor
	virtual ~CSPhrase()
	{
		++NbDesallocatedPhrases;
	}

	/// get Id
	//inline const TPhraseId &getId() const { return _Id; }

	/// get internal state
	inline TPhraseState state() const { return _State; }

	/// set internal state
	inline void setState(TPhraseState state) 
	{ 
		_State = state; 
	}
	
	/// get idle flag
	inline bool idle() const { return _Idle; }
	
	/// set idle flag
	inline void idle( bool idle ) { _Idle = idle; }

	/// get execution end date
	inline NLMISC::TGameCycle executionEndDate() const { return _ExecutionEndDate; }

	/// get apply date
	inline NLMISC::TGameCycle applyDate() const { return _ApplyDate; }

	/// get latency end date
	inline NLMISC::TGameCycle latencyEndDate() const { return _LatencyEndDate; }

	/**
	 * build the phrase from bricks, actor and main target
	 * \param actorRowId: row of the entity creating the phrase
	 * \param bricks : bricks composing the phrase
	 * \param buildToExecute if true then phrase is built to be executed right now, if false it's only for memorisation
	 * \return true on success
	 */
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true ) = 0;

	/**
	 * set the primary target
	 * \param entityId id of the primary target
	 */
	virtual void setPrimaryTarget( const TDataSetRow &/* entityRowId */ ) {}

	/**
	 * evaluate phrase
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate() = 0;

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
	 * at end of the execution, launch phrase
	 * \return true if launched without errors
	 */
	virtual bool launch() = 0;

	/**
	 * apply phrase
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

	/**
	 * called for brutal stop of the phrase befire it's execution
	 */
	virtual void stopBeforeExecution() {}

	/// get being processed flag
	inline bool beingProcessed() const { return _BeingProcessed; }

	/// set the being processed flag
	inline void beingProcessed(bool flag) { _BeingProcessed = flag; }

	/// 
	inline uint8 nextCounter() const { return _NextCounter; }
	inline void nextCounter(uint8 c) { _NextCounter = c; }
	///
	inline uint16 phraseBookIndex() const { return _PhraseBookIndex; }
	inline void phraseBookIndex(uint16 i) { _PhraseBookIndex = i; }
	/// 
	inline bool isStatic() const { return _IsStatic; }
	inline void isStatic(bool b) { _IsStatic = b; }
	///
	inline bool isApplied() const { return _IsApplied; }
	inline void isApplied(bool b) { _IsApplied = b; }

	inline BRICK_TYPE::EBrickType getType() const { return _PhraseType; }

	/** Return true if the phrase if grammatically valid (otherwise, send a msg to the player actorRowId)
	 * TODO: anti-hacking
	 */
	static bool	validateSabrinaGrammar( const std::vector< const CStaticBrick* >& /* bricks */, TDataSetRow /* actorRowId */ ) { return true; }

	virtual void setBrickSheets( const std::vector<NLMISC::CSheetId> & /* bricks */){}
	virtual void setEnchantMode(bool){}

	inline void cyclic(bool b) { _CyclicPhrase = b; }
	inline bool cyclic() const { return _CyclicPhrase; }

	/// Return true if a cancellation of the phrase due to character's movement must be ignored at the moment
	virtual bool mustOverrideCancelStaticAction() const { return false; }

protected:	
	/// idle or not
	bool					_Idle;
	/// execution end date
	NLMISC::TGameCycle		_ExecutionEndDate;
	/// apply date
	NLMISC::TGameCycle		_ApplyDate;
	/// latency end date
	NLMISC::TGameCycle		_LatencyEndDate;
	///	index in client phrase book (0 = not in the phrase book)
	uint16					_PhraseBookIndex;
	///	next counter
	uint8					_NextCounter;
	/// is static
	bool					_IsStatic;
	/// is applied
	bool					_IsApplied;

	/// being processed flag
	mutable bool			_BeingProcessed;

	// phrase type
	BRICK_TYPE::EBrickType	_PhraseType;

	/// repeat mode on/off
	bool					_CyclicPhrase;

private:
	/// phrase state
	TPhraseState			_State;


public:
	static uint32	NbAllocatedPhrases;
	static uint32	NbDesallocatedPhrases;
};

typedef NLMISC::CSmartPtr<CSPhrase> CSPhrasePtr;


#endif // RY_S_PHRASE_H

/* End of s_phrase.h */
