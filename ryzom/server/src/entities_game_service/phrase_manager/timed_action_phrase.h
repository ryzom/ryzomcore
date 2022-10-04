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



#ifndef RYZOM_TIMED_ACTION_PHRASE_H
#define RYZOM_TIMED_ACTION_PHRASE_H

#include "phrase_manager/s_phrase.h"
#include "timed_actions.h"
//
#include "game_share/client_action_type.h"


/**
 * This class represents a sabrina phrase used for special powers such as auras
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CTimedActionPhrase : public CSPhrase
{
public:

	/// ctor
	CTimedActionPhrase();

	/// dtor
	virtual ~CTimedActionPhrase();

	/// \name Override methods from CSPhrase
	//@{
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true );

	/**
	 * evaluate phrase
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate();
	
	/**
	 * validate phrase
	 * \return true if phrase is valid
	 */
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual bool launch();
	virtual void apply();
	
	/**
	 * called at the end of the latency time
	 */
	virtual void end();

	/**
	 * called when brutally stop the phrase
	 */
	virtual void stop();

	/**
	 * called for brutal stop of the phrase before it's execution
	 */
	virtual void stopBeforeExecution();
	//@}

	/// test if action is canceled when actor is being hit, return true if canceled
	bool testCancelOnHit( sint32 attackSkillValue, CEntityBase * entity, CEntityBase * defender);

	/**
	 * set the primary target
	 * \param entityId id of the primary target
	 */
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId ) { _TargetRowId = entityRowId; }

	/// get target
	inline const TDataSetRow &getTarget() const { return _TargetRowId; }

private:
	/// acting entity
	TDataSetRow				_ActorRowId;

	/// target id
	TDataSetRow				_TargetRowId;

	/// a pointer to the action to execute
	CTimedAction *			_TimedAction;

	/// execution duration
	NLMISC::TGameCycle		_ExecutionDuration;

	/// client action type
	CLIENT_ACTION_TYPE::TClientActionType _ActionType;

	/// root sheet id
	NLMISC::CSheetId		_RootSheetId;
};

#endif // RYZOM_TIMED_ACTION_PHRASE_H

/* End of timed_action_phrase.h */
