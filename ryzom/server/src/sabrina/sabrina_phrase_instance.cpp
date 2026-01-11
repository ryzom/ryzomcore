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




#include "game_share/tick_event_handler.h"
#include "sabrina_phrase_instance.h"
#include "sabrina_phrase_manager.h"
#include "sabrina_actor.h"

// Constructor
CSabrinaPhraseInstance::CSabrinaPhraseInstance(const ISabrinaActor* actor)
{
	_State			= Inactive;
	_NextEventTime	= 0;
	_ThePhrase		= NULL;
	_Actor			= const_cast<ISabrinaActor*>(actor);
	_Target			= NULL;
}

CSabrinaPhraseInstance::CSabrinaPhraseInstance(const CSabrinaPhraseInstance& other)
{
	_State			= other._State;
	_NextEventTime	= other._NextEventTime;
	_ThePhrase		= other._ThePhrase;
	_Actor			= other._Actor;
	_Target			= other._Target;

	// note that this line will cause an assert if you try to duplicate a phrase which has established
	// item locks - this is correct behaviour as it is not legal to have two lock objects refferencing
	// the same item.
	_Items			= other._Items;
}

// Destructor
CSabrinaPhraseInstance::~CSabrinaPhraseInstance()
{
	#ifdef NL_DEBUG
		nlassert(_ThePhrase==NULL);
		nlassert(_Target==NULL);
		nlassert(_State==Inactive);
	#endif
}

// method used to initialise a new phrase
void CSabrinaPhraseInstance::beginPhrase(const ISabrinaPhraseModel* thePhrase)
{
	#ifdef NL_DEBUG
		nlassert(_ThePhrase==NULL);
		nlassert(_State==Inactive);
		nlassert(_Target==NULL);
	#endif

	// setup basic data
	_ThePhrase=const_cast<ISabrinaPhraseModel*>(thePhrase);
	_NextEventTime=0;

	// setup the target
	_Target= _ThePhrase->requiresTarget()? _Actor->getTarget(): NULL;
	if (_Target!=NULL)
		_Target->addTargeter(this);

	// call the actor's init() callback
	_Actor->cbSabrinaActionBegin(this);

	// ask the model whether the phrase is valid (target's accessible, etc)
	// if the validation fails then close the phrase down and exit cleanly
	SABRINA::TEventCode validationResult=_ThePhrase->validate(this);
	if (!SABRINA::isSuccess(validationResult))
	{
		abortPhrase(validationResult);
		return;
	}

	// ask the phrase model to calculate the pre-execution delay
	uint32 ticks= _ThePhrase->calculatePreExecutionDelay(this);

	// setup the state information
	_State= PreActionDelay;
	_NextEventTime= CTickEventHandler::getGameCycle()+ticks;

	// check whether the action time is '0' - requiring immediate action
	if (ticks==0)
	{
		updatePhrase();
	}
	else
	{
		CSabrinaPhraseManager::setNextPhraseEvent(this,_NextEventTime);
	}
}

// update routine called on event triggers
// returns true if _NextEventTime == current time (otherwise false)
bool CSabrinaPhraseInstance::updatePhrase()
{
	// if the phrase' timer's been changed then this event is not valid so ignore it...
	NLMISC::TGameCycle curTime=CTickEventHandler::getGameCycle();
	if (_NextEventTime != curTime)
		return false;

	// change the value of '_NextEventTime' to avoid multiple executions of the action phrase
	// when the phrase end date is set to the same time more than once...
	--_NextEventTime;

	// depending on state do something...
	switch (_State)
	{
	case PreActionDelay:
		{
			// ask the model whether the phrase is valid (target's accessible, etc)
			// if the validation fails then close the phrase down and exit cleanly
			SABRINA::TEventCode validationResult=_ThePhrase->validate(this);
			if (!SABRINA::isSuccess(validationResult))
			{
				_Actor->cbSabrinaActionFailure(this,validationResult);
				terminate();
				break;
			}

			// ask the model to execute the phrase action and apply results to whoever necessary
			// if execution failed then exit cleanly
			SABRINA::TEventCode executionResult= _ThePhrase->executeAndApplyResults(this);
			if (SABRINA::isSuccess(executionResult))
			{
				_Actor->cbSabrinaActionSuccess(this,executionResult);
			}
			else
			{
				_Actor->cbSabrinaActionFailure(this,executionResult);
				terminate();
				break;
			}

			// ask the phrase model to calculate the post-execution delay
			uint32 ticks=_ThePhrase->calculatePostExecutionDelay(this);

			// setup the state information
			_State= PostActionDelay;
			_NextEventTime= CTickEventHandler::getGameCycle()+ticks;

			// check whether the action time is '0' - requiring immediate action
			if (ticks==0)
			{
				updatePhrase();
			}
			else
			{
				CSabrinaPhraseManager::setNextPhraseEvent(this,_NextEventTime);
			}

			break;
		}

	case PostActionDelay:
		terminate();
		break;

	case Inactive:
		break;
	}
	return true;
}

// method used to prematurely abort execution of phrase
void CSabrinaPhraseInstance::abortPhrase(SABRINA::TEventCode reason)
{
	switch (_State)
	{
	case Inactive:
		return;

	case PostActionDelay:
		return;

	case PreActionDelay:
		// call the actor's phrase cancelation callback
		_Actor->cbSabrinaActionCancel(this,reason);
		// terminate the phrase
		terminate();
		return;

	default:
		#ifdef NL_DEBUG
			nlerror("CSabrinaPhraseInstance::abortPhrase(): Bad '_State' value: %d",(uint32)_State);
		#endif
	}
}

// method used to prolong a sabrina action (eg prolong cast...)
void CSabrinaPhraseInstance::addTimeBeforeNextEvent(uint32 ticks)
{
	if (!isActive())
	{
		#ifdef NL_DEBUG
			nlerror("CSabrinaPhraseInstance::addTimeBeforeNextEvent(): phrase instance is inactive");
		#endif
		return;
	}
	_NextEventTime+= ticks;
	CSabrinaPhraseManager::setNextPhraseEvent(this,_NextEventTime);
}

// method for interrogating the state of the phrase instance - returns false if 'Inactive' otherwise true
bool CSabrinaPhraseInstance::isActive()
{
	return _State!=Inactive;
}

// method used to prematurely abort execution of phrase
void CSabrinaPhraseInstance::terminate()
{
	// make sure the actor is aware that their current action is terminated
	if (_State!=Inactive)
		_Actor->cbSabrinaActionEnd(this);

	// let target know that they are no longer targeted by this phrase
	if (_Target!=NULL)
		_Target->removeTargeter(this);

	// housekeeping
	_Target=NULL;
	_ThePhrase=NULL;
	_State=Inactive;
	return;
}
