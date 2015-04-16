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
#include "ai_logic_action.h"
#include "event_reaction_container.h"
#include "state_instance.h"
#include "ai_grp.h"
#include "child_container.h"
#include "server_share/msg_ai_service.h"
#include "ai_share/ai_share.h"
#include "ai_grp_npc.h"
#include "ai_bot_npc.h"
#include "game_share/send_chat.h"
#include "game_share/chat_group.h"
#include "ai_player.h"
#include "nel/misc/debug.h"
#include "nel/misc/sstring.h"
#include "ais_actions.h"
#include "states.h"
#include "continent.h"
#include "continent_inline.h"

#include "script_vm.h"
#include "script_compiler.h"

extern bool simulateBug(int bugId);

#include "dyn_grp_inline.h"

using namespace std;
using namespace NLMISC;
using namespace AITYPES;
using namespace	AICOMP;

static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else nlinfo


//////////////////////////////////////////////////////////////////////////
//	Generic Actions



//-------------------------------------------------------------------------------------------
// 	random_select_state
//-------------------------------------------------------------------------------------------

class	CAILogicActionRandomSelectState:public	IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionRandomSelectState(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		_weightSum=0;

		if (!subActions.empty())
			nlwarning("sub-actions of 'random_select_state' are ignored");

		for (uint i=0;i<args.size();++i)
		{
			std::string weightStr, stateStr;
			AI_SHARE::stringToWordAndTail(args[i],weightStr, stateStr);
			sint16 weight;
			NLMISC::fromString(weightStr, weight);
			if	(	weight<=0
				||	NLMISC::toString(weight)!=weightStr)
			{
//				nlinfo("no weight found for state in line '%s' - setting weight to 1",args[i].c_str());
				stateStr=args[i];
				weight=1;
			}
			
			CAIState	*state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(stateStr,AITypeNpcStateRoute));
			if	(!state) 
				state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(stateStr,AITypeNpcStateZone));

			if	(!state)
			{
				nlwarning("DATA_BUG: EVENT: %s: Failed to identify state: %s",eventNode->fullName().c_str(),args[i].c_str());
				continue;
			}

			_states.push_back(state);
			_weights.push_back(weight);
			_weightSum+=weight;
		}

		// make sure there's at least one state
		if (_weightSum==0)
			nlwarning("DATA_BUG: random_select_state: State list is unweighted!");
	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		// make sure something was found
		if (_weightSum==0)
		{
			nlwarning("random_select_state: State list is unweighted!");
			return false;
		}

		// generate a random number in range [0.._weightSum) and find a corresponding doofer
		sint32 randVal=CAIS::rand32(_weightSum);
		uint i;
		for (i=0;i<_states.size() && randVal>=_weights[i];++i)
			randVal-=_weights[i];

		// quick debug test... would be an assert...
#ifdef NL_DEBUG
		nlassert(randVal<=_weights[i]);	// "BUG: Random value outside random range!"
#endif

		entity->getDebugHistory()->addHistory("GRP State Change: %s => %s",
			entity->getState()->getAliasNode()->fullName().c_str(),
			_states[i]->getAliasNode()->fullName().c_str());

		entity->setNextState(_states[i]);
		return true;
	}

private:
	std::vector<CDbgPtr<CAIState> > _states;
	std::vector<sint16> _weights;
	uint16 _weightSum;
};


//-------------------------------------------------------------------------------------------
// 	begin_state
//-------------------------------------------------------------------------------------------

class CAILogicActionBeginState: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionBeginState(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if	(!subActions.empty())
			nlwarning("sub-actions of 'begin_state' are ignored");

		for (uint i=0;i<args.size();++i)
		{
			CAIState	*state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(args[i],AITypeNpcStateRoute));
			breakable
			{
				if (state)
					break;
				state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(args[i],AITypeNpcStateZone));
				if (state)
					break;
				state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(args[i],AITypeState));
				if (state)
					break;
				state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(args[i],AITypePunctualState));
				if (state)
					break;

				nlwarning("Action begin_state: failed to identify state: '%s' in '%s'",
					args[i].c_str(),
					eventNode->fullName().c_str());
				continue;
			}
			_states.push_back(state);
		}

	}

//	CPersistentStateInstance
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{

		if (_states.empty())
		{
			nlwarning("begin_state failed because state list is empty");
			return false;
		}
		uint i=CAIS::rand16((uint32)_states.size());

		entity->getDebugHistory()->addHistory("GRP State Change: %s => %s",
			entity->getState()->getAliasNode()->fullName().c_str(),
			_states[i]->getAliasNode()->fullName().c_str());
		
		entity->setNextState(_states[i]);
		return true;
	}
	
private:
	std::vector<CDbgPtr<CAIState> > _states;
};


//-------------------------------------------------------------------------------------------
// 	user_event
//-------------------------------------------------------------------------------------------

class IAILogicActionUserEvent: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	IAILogicActionUserEvent(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (!subActions.empty())
			nlwarning("sub-actions of 'user_event' are ignored");
		_groupsName = args;
	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		if	(	_groups.empty()
			&&	!_groupsName.empty())
		{
			// try to retreive the groups
			for (uint i=0; i<_groupsName.size(); ++i)
			{
				std::vector<CGroup*>	grps;
				if (simulateBug(4))
				{
					CAIS::instance().AIList()[0]->findGroup(grps, _groupsName[i]);
				}
				else
				{
					entity->getGroup()->getAIInstance()->findGroup(grps, _groupsName[i]);
				}
				if (grps.empty())
				{
					nlwarning("Can't find group(s) for the name '%s'", _groupsName[i].c_str());
					continue;
				}
				
				// retreive persistent state instance pointer.
				for (uint j=0; j<grps.size(); ++j)
					_groups.push_back(grps[j]->getPersistentStateInstance());
			}
			// clear the groupName vector to not redo the job
			_groupsName.clear();
		}

		if (_groups.empty())
		{
			nlwarning("user_event failed because group list is empty");
			return false;
		}

		for (uint i=0;i<_groups.size();++i)
		{
			CPersistentStateInstance *grp=_groups[i];
			if	(!grp)
				continue;
			
			grp->getDebugHistory()->addHistory("GRP User Event: %u",getIndex());

			nlassert(getIndex()<10);
			grp->processStateEvent(grp->getPersistentStateInstance()->getEventContainer().EventUserEvent[getIndex()]);
		}
		return true;
	}

protected:
	virtual uint32 getIndex()=0;

private:	
	std::vector<CPersistentStateInstance*>	_groups;
	std::vector<std::string>				_groupsName;
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent0 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent0(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex();
};

uint32 CAILogicActionUserEvent0::getIndex()
{
	return 0;
}

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent1 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent1(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 1; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent2 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent2(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 2; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent3 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent3(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 3; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent4 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent4(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 4; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent5 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent5(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 5; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent6 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent6(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 6; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent7 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent7(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 7; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent8 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent8(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 8; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionUserEvent9 : public IAILogicActionUserEvent
{
public:
	CAILogicActionUserEvent9(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionUserEvent(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 9; }
};


//-------------------------------------------------------------------------------------------
// 	multi_action
//-------------------------------------------------------------------------------------------

class CAILogicActionMultiAction: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionMultiAction(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (!args.empty())
			nlwarning("arguments of 'multi_action' are ignored");

		if (subActions.empty())
			nlwarning("no sub-actions found for multi_action action");

		_subActions=subActions;
	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
//	virtual bool executeAction(CAIEntity *entity,const IAIEvent *event)
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		if (_subActions.empty())
		{
			nlwarning("multi_action failed because sub-action list is empty");
			return false;
		}

		bool result=true;

		const	uint32 nbActions=(uint32)_subActions.size();
		for (uint32 i=0;i<nbActions;i++)
		{
			if(_subActions[i]==NULL)
			{
				nlwarning("multi actions : _subActions[%d] NULL ! ",i);
			}
			if (!_subActions[i]->executeAction(entity,event))
				result=false;
		}
		return result;
	}

private:
	std::vector<IAILogicAction::TSmartPtr> _subActions;
};


//-------------------------------------------------------------------------------------------
// 	punctual_state
//-------------------------------------------------------------------------------------------

class CAILogicActionPunctualState: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionPunctualState(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if	(!subActions.empty())
			nlwarning("sub-actions of 'punctual_state' are ignored");

		for (uint i=0;i<args.size();++i)
		{
			CAIState	*const	state=container->states().getChildByAlias(eventNode->findAliasByNameAndType(args[i],AITypePunctualState));
			if (!state)
			{
				nlwarning("Failed to identify punctual state: %s",args[i].c_str());
				continue;
			}
			_states.push_back(state);
		}

	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		if (_states.empty())
		{
			nlwarning("begin_punctual_state failed because state list is empty");
			return false;
		}
		entity->setNextPunctualState(_states[CAIS::rand16((uint32)_states.size())]);
		entity->getDebugHistory()->addHistory("GRP BeginPunctual State: %s",
					entity->getNextPunctualState()->getAliasNode()->fullName().c_str());
		return true;
	}

private:
	std::vector<CDbgPtr<CAIState> > _states;
};


//-------------------------------------------------------------------------------------------
// 	punctual_state_end
//-------------------------------------------------------------------------------------------

class CAILogicActionPunctualStateEnd: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionPunctualStateEnd(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (!subActions.empty())
			nlwarning("sub-actions of 'punctual_state_end' are ignored");

		if (!args.empty())
			nlwarning("args of 'punctual_state_end' are ignored");
	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		entity->getDebugHistory()->addHistory("GRP End Punctual State: %s",
						entity->getPunctualState()->getAliasNode()->fullName().c_str());
		
		entity->cancelPunctualState();
		return true;
	}

};


//-------------------------------------------------------------------------------------------
// 	random_select
//-------------------------------------------------------------------------------------------

class CAILogicActionRandomSelect: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionRandomSelect(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (!args.empty())
			nlwarning("arguments of 'random_select' are ignored");

		if (subActions.empty())
			nlwarning("no sub-actinos found for random action");

		_subActions=subActions;
	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		if (_subActions.empty())
		{
			nlwarning("random_select failed because sub-action list is empty");
			return false;
		}
		_subActions[CAIS::rand16((uint32)_subActions.size())]->executeAction(entity,event);

		return true;
	}

private:
	std::vector<IAILogicAction::TSmartPtr> _subActions;
};


//-------------------------------------------------------------------------------------------
// 	set_state_timeout
//-------------------------------------------------------------------------------------------

class CAILogicActionSetStateTimeout: public IAILogicAction
{
public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	CAILogicActionSetStateTimeout(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> &subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (!subActions.empty())
			nlwarning("sub-actions of 'set state timeout' are ignored");

		switch (args.size())
		{
		case 2:	NLMISC::fromString(args[0], _min); if (args[0]!=NLMISC::toString(_min)) goto BadArgs;
				NLMISC::fromString(args[1], _max); if (args[1]!=NLMISC::toString(_max)) goto BadArgs;
				break;
		case 1:	NLMISC::fromString(args[0], _min); if (args[0]!=NLMISC::toString(_min)) goto BadArgs;
				_max=_min;
				break;
		default: 
		BadArgs:
			nlwarning("Invalid arguments for 'set state timeout'");
			_min=0;
			_max=0;
		}

	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		uint t=_min;
		if (_min != _max)
			t += CAIS::rand32(_max-_min);
		entity->timerStateTimeout().set(t);
		entity->getDebugHistory()->addHistory("GRP Set State Timeout: %u", t);
		return true;
	}

private:
	uint32 _min,_max;
};


//-------------------------------------------------------------------------------------------
// 	set_timer_t*
//-------------------------------------------------------------------------------------------

// this class is not instantiated directly. It is derived to refference timers t0,t1,... etc

class IAILogicActionSetTimer: public IAILogicAction
{
	enum TTimerMode
	{
		tm_invalid,
		tm_timer,
		tm_daytime,
		tm_weekday,
		tm_monthday,
		tm_seasonday,
		tm_yearday
	};

public:
	// init is called just after instantiation to give class a chance to parse arguments and
	// deal with sub actions
	IAILogicActionSetTimer(	const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		_Mode = tm_invalid;
		_DayTime = 0;
		_DayNumber = 0;
		_Min = _Max = 0;

		if (!subActions.empty())
			nlwarning("sub-actions of 'set timer' are ignored");
		if(args.empty())
		{
			nlwarning("Invalid arguments 'for set_timer'");
			return;
		}
		std::vector<std::string> strs;
		NLMISC::explode(args[0], string(" "), strs, true);

		if (strs[0] == "day_time" || strs[0] == "daytime")
		{
			// this is a day time timer

			if (strs.size() != 2)
			{
				nlwarning("Invalid argument for set timer with daytime(%s) : %s", args[0].c_str(), eventNode->fullName().c_str());
				_Min = _Max = 0;
			}
			else 
			{
				_Mode = tm_daytime;
				_DayTime = parseDayTime(strs[1]);
			}
		}
		else if (strs[0] == "week_day")
		{
			if (strs.size() < 2)
			{
				nlwarning("Missing weekday argument for set timer '%s' : '%s'", eventNode->fullName().c_str(), args[0].c_str());
			}
			_Mode = tm_weekday;

			_DayNumber = atoui(strs[1].c_str());

			if (strs.size() == 3)
				_DayTime = parseDayTime(strs[2]);
		}
		else if (strs[0] == "month_day")
		{
			if (strs.size() < 2)
			{
				nlwarning("Missing monthday argument for set timer '%s' : '%s'", eventNode->fullName().c_str(), args[0].c_str());
			}
			_Mode = tm_monthday;

			_DayNumber = atoui(strs[1].c_str());

			if (strs.size() == 3)
				_DayTime = parseDayTime(strs[2]);
		}
		else if (strs[0] == "season_day")
		{
			if (strs.size() < 2)
			{
				nlwarning("Missing seasonday argument for set timer '%s' : '%s'", eventNode->fullName().c_str(), args[0].c_str());
			}
			_Mode = tm_seasonday;

			_DayNumber = atoui(strs[1].c_str());

			if (strs.size() == 3)
				_DayTime = parseDayTime(strs[2]);
		}
		else if (strs[0] == "year_day")
		{
			if (strs.size() < 2)
			{
				nlwarning("Missing yearday argument for set timer '%s' : '%s'", eventNode->fullName().c_str(), args[0].c_str());
			}
			_Mode = tm_yearday;

			_DayNumber = atoui(strs[1].c_str());

			if (strs.size() == 3)
				_DayTime = parseDayTime(strs[2]);
		}
		else
		{
			_Mode = tm_timer;
			switch (args.size())
			{
			case 2:	NLMISC::fromString(args[0], _Min); if (args[0]!=NLMISC::toString(_Min)) goto BadArgs;
					NLMISC::fromString(args[1], _Max); if (args[1]!=NLMISC::toString(_Max)) goto BadArgs;
					break;
			case 1:	NLMISC::fromString(args[0], _Min); if (args[0]!=NLMISC::toString(_Min)) goto BadArgs;
					_Max=_Min;
					break;
			default: 
			BadArgs:
				nlwarning("Invalid arguments for 'set timer'(%s,%s) : %s",
					(args.size()==1||args.size()==2)? args[0].c_str(): "BAD ARG COUNT",
					(args.size()==2)? args[1].c_str(): "" ,
					eventNode->fullName().c_str());
				_Min=0;
				_Max=0;
				_Mode = tm_invalid;
			}
		}

	}

	float parseDayTime(const string &param)
	{
		uint hour;
		uint minute = 0;

		vector<string>	parts;
		explode(param, string(":"), parts, false);

		if (parts.size() == 2)
			minute = NLMISC::atoui(parts[1].c_str());

		hour = NLMISC::atoui(parts[0].c_str());

		// convert minute to humdreds of hour to obtain time of day
		return hour + (minute * 100.f)/(60.f*100.f);
	}

	// this is the executeAction 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	virtual bool executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		// precompute the timer ticks
		float currentTime = CTimeInterface::getRyzomTime().getRyzomTime();
		if (_DayTime < currentTime)
		{
			// advance to next day
			currentTime -= 24;
		}
		float deltaTime = _DayTime - currentTime;
		// convert to ticks
		uint32 timeTicks = uint32(deltaTime * RYZOM_HOURS_IN_TICKS);

		switch (_Mode)
		{
		case tm_daytime:
			{
				_Min = _Max = timeTicks;
			}
			break;
		case tm_weekday:
			{
				uint32 dow = (uint32)CTimeInterface::getRyzomTime().getRyzomDayOfWeek();
				while (dow < _DayNumber)
					dow += RYZOM_WEEK_IN_DAY;

				_Min = _Max = timeTicks + dow * RYZOM_DAY_IN_TICKS;
			}
			break;
		case tm_monthday:
			{
				uint32 dom = CTimeInterface::getRyzomTime().getRyzomDayOfMonth();
				while (dom < _DayNumber)
					dom += RYZOM_MONTH_IN_DAY;

				_Min = _Max = timeTicks + dom * RYZOM_DAY_IN_TICKS;
			}
			break;
		case tm_seasonday:
			{
				uint32 dos = CTimeInterface::getRyzomTime().getRyzomDayOfSeason();
				while (dos < _DayNumber)
					dos += RYZOM_SEASON_IN_DAY;

				_Min = _Max = timeTicks + dos * RYZOM_DAY_IN_TICKS;
			}
			break;
		case tm_yearday:
			{
				uint32 doy = CTimeInterface::getRyzomTime().getRyzomDayOfYear();
				while (doy < _DayNumber)
					doy += RYZOM_YEAR_IN_DAY;

				_Min = _Max = timeTicks + doy * RYZOM_DAY_IN_TICKS;
			}
			break;
		}

		uint t=_Min;
		if (_Max != _Min)
			t+=CAIS::rand32(_Max-_Min);
		entity->timerUser(getIndex()).set(t);
		entity->getDebugHistory()->addHistory("GRP Set  Timer t%i: %u", getIndex(), t);
		return true;
	}
 
protected:
	virtual uint32 getIndex()=0;
	TTimerMode	_Mode;
	uint32		_Min;
	uint32		_Max;
	float		_DayTime;
	uint32		_DayNumber;
};

//-------------------------------------------------------------------------------------------

class CAILogicActionSetTimerT0 : public IAILogicActionSetTimer
{
public:
	CAILogicActionSetTimerT0(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionSetTimer(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 0; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionSetTimerT1 : public IAILogicActionSetTimer
{
public:
	CAILogicActionSetTimerT1(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionSetTimer(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 1; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionSetTimerT2 : public IAILogicActionSetTimer
{
public:
	CAILogicActionSetTimerT2(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionSetTimer(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 2; }
};

//-------------------------------------------------------------------------------------------

class CAILogicActionSetTimerT3 : public IAILogicActionSetTimer
{
public:
	CAILogicActionSetTimerT3(const std::vector<std::string> &args, 
								const std::vector<IAILogicAction::TSmartPtr> &subActions, 
								const CAIAliasDescriptionNode *eventNode, CStateMachine *container):
								IAILogicActionSetTimer(args,subActions,eventNode,container){}
protected:
	virtual uint32 getIndex()	{ return 3; }
};


//-------------------------------------------------------------------------------------------
class CAILogicActionSpawn : public IAILogicAction
{
public:
	CAILogicActionSpawn	(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		CGroup *grp = entity->getGroup();
		if (grp)
		{
			if	(grp->isSpawned())
				grp->getSpawnObj()->spawnBots();
			return true;
		}
		else
		{
			nlwarning("CAILogicActionSpawn : entity %s is not a group ? ", 
				entity->aliasTreeOwner()->getAliasString().c_str());
			return false;
		}

	}

};
//-------------------------------------------------------------------------------------------
class CAILogicActionDespawn : public IAILogicAction
{
public:
	CAILogicActionDespawn (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		_immediately = false;

		// look in args to see if the 'immediately' options is here
		if	(args.empty())
			return;

		_immediately = (args[0] == "immediately");
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		CGroup *const	grp = entity->getGroup();
		
		if (!grp)
		{
			nlwarning("CAILogicActionDespawn : entity %s is not a group ? ", 
				entity->aliasTreeOwner()->getAliasString().c_str());
			return false;			
		}

		grp->despawnBots(_immediately);
		return true;
	}
private:
	bool	_immediately;
};

//-------------------------------------------------------------------------------------------
class CAILogicActionSendMessage : public IAILogicAction
{
public:
	CAILogicActionSendMessage (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (args.empty())
		{
			nlwarning("CAILogicActionSendMessage need at least a service name !");
			return;
		}

		_destService = args[0];

		std::vector<std::string> temp(args.begin()+1, args.end());

		_content.swap(temp);
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if (_destService.empty())
			return false;
		
		CCAisActionMsg msg;
		msg.Alias = entity->aliasTreeOwner()->getAlias();
		msg.Content = _content;

		msg.send(_destService);

		return true;
	}
private:
	std::string					_destService;
	std::vector<std::string>	_content;
};

// a big bad global var !
CAIEntityPhysical	*TempSpeaker = NULL;
CBotPlayer			*TempPlayer = NULL;

//-------------------------------------------------------------------------------------------
class CAILogicActionSay : public IAILogicAction
{
public:
	CAILogicActionSay (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		_tellToEscort = false;
		if (args.empty())
		{
			nlwarning("CAILogicActionSay need a phrase identifier !");
			return;
		}

		std::string key, tail;
		AI_SHARE::stringToKeywordAndTail(args[0], key, tail);
		
		breakable
		{
			if (key == "tell")
			{
				_type = CChatGroup::tell;
				break;
			}
			if (key == "say")
			{
				_type = CChatGroup::say;
				break;
			}
			if (key == "shout")
			{
				_type = CChatGroup::shout;
				break;
			}
			if (key == "universe")
			{
				_type = CChatGroup::universe;
				break;
			}
			if (key == "escort")
			{
				_type = CChatGroup::tell;
				_tellToEscort = true;
				break;
			}
			nlwarning("Unknow chat mode '%s' ! default to 'say'", key.c_str());
			_type = CChatGroup::say;
			// look in the first arg for a channel tag
		}
		if (tail.empty())
		{
			nlwarning("Phrase identifier is empty in action '%s'!", eventNode->fullName().c_str());
			return;
		}
		_phraseId = tail;
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if	(_phraseId.empty())
			return false;

		// we must first find the group leader, then send a chat message a 'say' mode
		const CAIAliasDescriptionNode	*const aliasDesc=entity->aliasTreeOwner()->getAliasNode();

		if	(	(aliasDesc && aliasDesc->getType() != AITypeGrp)
			||	!entity->getGroup()	)
		{
			nlwarning("group can't have 'say' action !");
			return	true;
		}

		{
			const	CSpawnGroup	*const	grp = entity->getGroup()->getSpawnObj();
#if !FINAL_VERSION
			nlassert(grp);
#endif
			CSpawnBot	*bot = NULL;

			if	(	TempSpeaker
				&&	TempSpeaker->isAlive())
				bot = NLMISC::safe_cast<CSpawnBot*>(TempSpeaker);

			if	(!bot)
			{
				CBot *b = grp->getPersistent().getLeader();
				if (b)
					bot = b->getSpawnObj();
			}

			if	(	!bot
				||	!bot->isAlive())
			{
				LOG("CAILogicActionSay::executeAction : no bots in the group, ignoring action.");
				return true;
			}
						
			// ok, we have the leader, send the chat message.
			if (!_tellToEscort)
			{
				if (_type == CChatGroup::tell && TempPlayer != NULL)
				{
					// tell to the player
					LOG("CAILogicActionSay::executeAction : sending phrase '%s' to player %s", _phraseId.c_str(), TempPlayer->getEntityId().toString().c_str());
					npcTellToPlayer(bot->dataSetRow(), TempPlayer->dataSetRow(), _phraseId);
				}
				else
				{
					LOG("CAILogicActionSay::executeAction : sending phrase '%s' to chat group 'say'", _phraseId.c_str());
					npcChatToChannel(bot->dataSetRow(), _type, _phraseId);
				}
			}
			else
			{
				// retrieve the team id of the player, then tell to each member of the team.
				const uint16 teamId = grp->getPersistent().getEscortTeamId();

				if	(teamId == CTEAM::InvalidTeamId)
				{
					nlwarning("CAILogicActionSay::executeAction : can't tell to escort team coz escort is invalid");
					return false;
				}

				LOG("CAILogicActionSay::executeAction : telling phrase '%s' to escort team %u", _phraseId.c_str(), teamId);

				{
					CManagerPlayer	*mgr=grp->getPersistent().getAIInstance()->getPlayerMgr();
					const std::set<TDataSetRow> team = mgr->getPlayerTeam(teamId);
					std::set<TDataSetRow>::const_iterator first(team.begin()), last(team.end());
					for (; first != last; ++first)
						npcTellToPlayer(bot->dataSetRow(), *first, _phraseId);
				}
			}
		}
		return true;
	}
private:
	std::string				_phraseId;
	CChatGroup::TGroupType	_type;
	bool					_tellToEscort;
};

//a class to find a group by its name in a state machine, 
class CAIGroupFinder
{
public:
static	CGroup* findGroup(const std::string& groupName,CStateMachine *stateMachine)
	{
		vector<CGroup*> grps;
		CWorkPtr::aiInstance()->findGroup(grps, groupName);
		if (grps.empty())
			{
				nlwarning("Can't find group named '%s'", groupName.c_str());
				return NULL;
			}
			
			if (grps.size() == 1)
			{
				return grps.front();
			}
			else
			{
				CGroup	*igroup=NULL;
				//	Check if theres a group with the good name in the same stateMachine (and only one).
				for		(sint32	grpIndex=(sint32)grps.size()-1;grpIndex>=0;grpIndex--)
				{
					if	(grps[grpIndex]->getManager().getStateMachine()!=stateMachine)
						continue;

					if	(igroup)
					{
						nlwarning("More than one group has the name '%s' in the same manager", groupName.c_str());
						return NULL;
					}
					igroup=grps[grpIndex];
				}
				if (!igroup)
				{
					nlwarning("No group has the name '%s'", groupName.c_str());
					return NULL;
				}
				return igroup;
			}
		return NULL;
	}	
private:
	CAIGroupFinder();
};

class CAIVariableParser
{
public:

	enum TVarType
	{
		local_variable,
		foreign_variable,
		constant,
		invalid_var_type
	};

	enum TOperator
	{
		less_than,
		greater_than,
		less_equal,
		greater_equal,
		not_equal,
		equal,			
		assign,			// WARNING : don't change the relative order of this enumerated constante
		add,			// they must remain grouped from equal up to div
		sub,			//
		mult,			//
		div,			//
		invalid_operator
		
	};

	struct TVariable
	{
		TVarType	Type;
		CGroup		*Group;
		NLMISC::TStringId	VarId;
		//	uint32		VarId;
		float		Value;

		TVariable() :
			Type(invalid_var_type),
			Group(NULL),
//			VarId(0),
			Value(0)
		{}
	};

	float retreiveValue(CStateInstance *stateInstance, const TVariable &var)
	{
		switch	(var.Type)
		{
		case local_variable:
			return stateInstance->getLogicVar(var.VarId);
		case foreign_variable:
			{
				CStateInstance	*stateInstance=var.Group->getPersistentStateInstance();
				return	stateInstance->getLogicVar(var.VarId);
			}
		case constant:
			return var.Value;
		default:
			nlwarning("retreiveValue from invalid var type ! returning 0");
			return 0;
		}

	}

	bool evalCondition(CStateInstance *stateInstance, const TVariable &var1, TOperator op, const TVariable &var2)
	{
		switch (op)
		{
		case less_than:
			return retreiveValue(stateInstance, var1) < retreiveValue(stateInstance, var2);
		case greater_than:
			return retreiveValue(stateInstance, var1) > retreiveValue(stateInstance, var2);
		case less_equal:
			return retreiveValue(stateInstance, var1) <= retreiveValue(stateInstance, var2);
		case greater_equal:
			return retreiveValue(stateInstance, var1) >= retreiveValue(stateInstance, var2);
		case equal:
		case assign:
			return retreiveValue(stateInstance, var1) == retreiveValue(stateInstance, var2);
		case not_equal:
			return retreiveValue(stateInstance, var1) != retreiveValue(stateInstance, var2);
		}
		nlwarning("evalCondition invalid operator %u", op);
		return false;
	}

	bool parseOperator(TOperator &op, const string &str)
	{
		struct TOpName
		{
			const char	*name;
			TOperator	op;
		};

		static TOpName	opNames[] =
		{
			{"<", less_than},
			{">", greater_than},
			{"<=", less_equal},
			{">=", greater_equal},
			{"=", assign},
			{"!=", not_equal},
			{"<>", not_equal},
			{"==", equal},		
			{"+", add},
			{"-", sub},
			{"*", mult},
			{"/", div}
		};
		const uint32 TABLE_SIZE = sizeof(opNames) / sizeof(TOpName);

		for (uint i=0; i<TABLE_SIZE; ++i)
		{
			if (str!=opNames[i].name)
				continue;

			op = opNames[i].op;
			return true;
		}
		op = invalid_operator;
		return false;
	}
	
	

	// NB : the second parameter in not passed as const reference because it is modified internaly
	bool	parseVar(TVariable &var, std::string str,CStateMachine *stateMachine)
	{
		var.Type = invalid_var_type;
		string::size_type pos = str.find(":");
		if (pos == string::npos)
		{
			pos = str.find(".");
		}

		if (pos != string::npos)
		{
			// there is a group name, extract it and retreive the group.
			var.Type = foreign_variable;
			const	string groupName = str.substr(0, pos);
			vector<CGroup*> grps;
			CWorkPtr::aiInstance()->findGroup(grps, groupName);
			if (grps.empty())
			{
				nlwarning("CAIVariableParser can't find group named '%s'", groupName.c_str());
				var.Type = invalid_var_type;
				return false;
			}
			
			if (grps.size() == 1)
			{
				var.Group = grps.front();
			}
			else
			{
				CGroup	*igroup=NULL;
				//	Check if theres a group with the good name in the same stateMachine (and only one).
				for		(sint32	grpIndex=(sint32)grps.size()-1;grpIndex>=0;grpIndex--)
				{
					if	(grps[grpIndex]->getManager().getStateMachine()!=stateMachine)
						continue;

					if	(igroup)
					{
						nlwarning("CAIVariableParser more than one group has the name '%s' in the same manager", groupName.c_str());
						var.Type = invalid_var_type;
						return false;
					}
					igroup=grps[grpIndex];
				}
				if (!igroup)
				{
					nlwarning("CAIVariableParser more than one group has the name '%s'", groupName.c_str());
					var.Type = invalid_var_type;
					return false;
				}
				var.Group=igroup;
			}
			str = str.substr(pos+1);
		}
		if	(str.empty())
		{
			nlwarning("CAIVariableParser no char left to read variable id or constante value");
			var.Type = invalid_var_type;
			return false;
		}

		// check if it's a number or a var.
		//TODO: and negative values ?
		if ((str[0]<='9' && str[0]>='0')||(str[0]=='-'))	//	its a number
		{
			var.Type = constant;
			NLMISC::fromString(str, var.Value);
			return	true;
		}

//		if (str[0] == 'v' || str[0] == 'V')
		//	its a variable.
		{
//			if (str.size() != 2)
//			{
//				nlwarning("CAIVariableParser bad string format to read variable index in '%s'. Should be 'vX' (with X in [0-3])", str.c_str());
//				var.Type = invalid_var_type;
//				return false;
//			}

			// this is a variable index
			if	(var.Type != foreign_variable)
				var.Type = local_variable;

			var.VarId = CStringMapper::map(str);
//			var.VarId = str[1] - '0';
//			NLMISC::clamp(var.VarId, (uint32)local_variable, (uint32)invalid_var_type);
			return true;
		}

		// try to parse a constant value
//		var.Type = constant;
//		double val = atof(str.c_str());
//		var.Value = float(val);
//		return true;
	}

};
//-------------------------------------------------------------------------------------------
// Same as CAILogicActionConditionIf but the condition is evaluated dynamicaly not at loading of primitive(eg primitive 1 modify primitive 2)
class CAILogicActionDynamicIf : public IAILogicAction, public CAIVariableParser
{
public:
	CAILogicActionDynamicIf (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (args.empty())
		{
			nlwarning("condition_if (%s) need arguments !", eventNode->fullName().c_str());
			return;
		}
		if (subActions.empty())
		{
			nlwarning("condition_if (%s) need sub action !", eventNode->fullName().c_str());
			return;
		}
		_SubActions = subActions;
		_Args = args;
		//_EventFullName = eventNode->fullName();
		if (!_Args.empty())
		{
			std::string oldValue = _Args[_Args.size()-1];
			_Args[_Args.size()-1] = std::string("if (") + oldValue + std::string("){()setConditionSuccess(1);} else { ()setConditionSuccess(0); }");
		}

	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
				// parse the argument string.

		if (_Args.empty()) { return false; }
		if (_SubActions.empty()){ return false;}
		
		
		NLMISC::CSmartPtr<const  AIVM::CByteCode> codePtr =  AICOMP::CCompiler::getInstance().compileCode(_Args, _EventFullName + std::string(":dynamic if"));
		
		CAILogicDynamicIfHelper::setConditionSuccess(false);
		if (!codePtr.isNull())
		{
			entity->getPersistentStateInstance()->interpretCode(entity, codePtr);
		}
		
		if ( CAILogicDynamicIfHelper::getConditionSuccess() )
		{
			_SubActions[0]->executeAction(entity, event);
		}
		else if (_SubActions.size() == 2)
		{

			_SubActions[1]->executeAction(entity, event);
		}
		return true;
	}
protected:
	CAIVariableParser::TVariable	_Var1;
	CAIVariableParser::TOperator	_Op;
	CAIVariableParser::TVariable	_Var2;

	std::vector<IAILogicAction::TSmartPtr> _SubActions;
	std::vector<std::string> _Args;

	std::string _EventFullName;
	static bool _Condition;
};

//-------------------------------------------------------------------------------------------
class CAILogicActionConditionIf : public IAILogicAction, public CAIVariableParser
{
public:
	CAILogicActionConditionIf (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (args.empty())
		{
			nlwarning("condition_if (%s) need arguments !", eventNode->fullName().c_str());
			return;
		}
		if (subActions.empty())
		{
			nlwarning("condition_if (%s) need sub action !", eventNode->fullName().c_str());
			return;
		}
		_subActions = subActions;

		// parse the argument string.
		vector<string>	strs;
		NLMISC::explode(args[0], string(" "), strs, true);

		if (strs.size() != 3)
		{
			nlwarning("condition_if (%s) invalid condition format '%s'. Need 3 parts, found %u", eventNode->fullName().c_str(), args[0].c_str(), strs.size());
		}

		if (!parseVar(_Var1, strs[0],container))
		{
			nlwarning("condition_if (%s) error parsing var 1 in action '%s'", strs[0].c_str(), eventNode->fullName().c_str());
			return;
		}
		if (!parseVar(_Var2, strs[2],container))
		{
			nlwarning("condition_if (%s) error parsing var 2 in action '%s'", strs[2].c_str(), eventNode->fullName().c_str());
			return;
		}
		if (!parseOperator(_Op, strs[1]))
		{
			nlwarning("condition_if (%s) error parsing operator in action '%s'", strs[1].c_str(), eventNode->fullName().c_str());
			return;
		}

	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if (evalCondition(entity, _Var1, _Op, _Var2))
		{
			if (!_subActions.empty())
				_subActions[0]->executeAction(entity, event);
		}
		return true;
	}
protected:
	CAIVariableParser::TVariable	_Var1;
	CAIVariableParser::TOperator	_Op;
	CAIVariableParser::TVariable	_Var2;

	std::vector<IAILogicAction::TSmartPtr> _subActions;
};
//-------------------------------------------------------------------------------------------
class CAILogicActionConditionIfElse : public CAILogicActionConditionIf
{
public:
	CAILogicActionConditionIfElse (const std::vector<std::string> &args, 
									const std::vector<IAILogicAction::TSmartPtr> &subActions, 
									const CAIAliasDescriptionNode *eventNode, 
									CStateMachine *container)
	:CAILogicActionConditionIf(args, subActions, eventNode, container)
	{
		if (subActions.size() != 2)
		{
			nlwarning("condition_if_else (%s) should have 2 sub action, %u found", eventNode->fullName().c_str(), subActions.size());
		}

	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if (evalCondition(entity, _Var1, _Op, _Var2))
		{
			if (!_subActions.empty())
				_subActions[0]->executeAction(entity, event);
		}
		else if (_subActions.size() > 1)
		{
			_subActions[1]->executeAction(entity, event);
		}
		return true;
	}
private:
};
//-------------------------------------------------------------------------------------------
class CAILogicActionModifyVariable : public IAILogicAction, CAIVariableParser
{
public:
	CAILogicActionModifyVariable (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		_Op = invalid_operator;
		if (args.empty())
		{
			nlwarning("modify_variable (%s) need arguments !", eventNode->fullName().c_str());
			return;
		}
		if (!subActions.empty())
		{
			nlwarning("modify_variable (%s) sub action ignored !", eventNode->fullName().c_str());
		}

		// parse the argument string.
		vector<string>	strs;
		NLMISC::explode(args[0], string(" "), strs, true);

		if (strs.size() != 3)
		{
			nlwarning("modify_variable (%s) : need 3 parts in arg '%s', only found %u", eventNode->fullName().c_str(), args[0].c_str(), strs.size());
			return;
		}

		if (!parseVar(_Var1, strs[0], container))
		{
			nlwarning("modify_variable (%s) error parsing var 1 in action '%s'", strs[0].c_str(), eventNode->fullName().c_str());
			return;
		}
		if (_Var1.Type == constant)
		{
			nlwarning("modify_variable (%s) left variable can't be a constant.", eventNode->fullName().c_str(), eventNode->fullName().c_str());
			return;

		}
		if (!parseVar(_Var2, strs[2], container))
		{
			nlwarning("modify_variable (%s) error parsing var 2 in action '%s'", strs[2].c_str(), eventNode->fullName().c_str());
			return;
		}
		if (!parseOperator(_Op, strs[1]))
		{
			nlwarning("modify_variable (%s) error parsing operator in action '%s'", strs[1].c_str(), eventNode->fullName().c_str());
			return;
		}

		if (_Op < CAIVariableParser::assign || _Op > CAIVariableParser::div)
		{
			nlwarning("modify_variable (%s) invalid operator %u", eventNode->fullName().c_str(), _Op);
			_Op = CAIVariableParser::invalid_operator;
		}

	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if (_Op == invalid_operator)
			return false;

		float value = retreiveValue(entity, _Var1);

		switch(_Op)
		{
		case assign:
			value = retreiveValue(entity, _Var2);
			//nlinfo("retreived value: %f",value);
			break;
		case add:
			value += retreiveValue(entity, _Var2);
			break;
		case sub:
			value -= retreiveValue(entity, _Var2);
			break;
		case mult:
			value *= retreiveValue(entity, _Var2);
			break;
		case div:
			value /= retreiveValue(entity, _Var2);
			break;
		}

		if (_Var1.Type == local_variable)
		{
			entity->setLogicVar(_Var1.VarId, value);
		}
		else if (_Var1.Type == foreign_variable)
		{
			if (_Var1.Group != NULL)
				_Var1.Group->getPersistentStateInstance()->setLogicVar(_Var1.VarId, value);
		}
		else
		{
			nlwarning("modify_variable Inconsistent variable 1 type : %u", _Var1.Type);
		}
		return true;
	}
private:
	CAIVariableParser::TVariable	_Var1;
	CAIVariableParser::TOperator	_Op;
	CAIVariableParser::TVariable	_Var2;
};


//-------------------------------------------------------------------------------------------
class CAILogicActionEmot : public IAILogicAction
{
public:
	CAILogicActionEmot (const std::vector<std::string> &args, 
									const std::vector<IAILogicAction::TSmartPtr> &subActions, 
									const CAIAliasDescriptionNode *eventNode, 
									CStateMachine *container)
	{
		uint32 emot = ~0;

		if (!subActions.empty())
		{
			nlwarning("emot (%s) should not have sub action", eventNode->fullName().c_str());
		}
		if (args.empty())
		{
			nlwarning("emot (%s) must containts the emot name !", eventNode->fullName().c_str());
			return;
		}

		emot = CAIS::instance().getEmotNumber(args[0]);

		if (emot == ~0)
		{
			nlwarning("emot (%s) unknow emot name : '%s' !", eventNode->fullName().c_str(), args[0].c_str());
			_EmotNumber = MBEHAV::NUMBER_OF_BEHAVIOURS;
		}
		else
		{
			// offset to emot behavior
			_EmotNumber = (MBEHAV::EBehaviour)(emot + MBEHAV::EMOTE_BEGIN);
		}
		_R2 = false;
		// parse the bots filter (if any)
		for (uint i=1; i<args.size(); ++i)
		{
			const string &s = args[i];
			if	(s.empty())
				continue;

			if (isdigit(s[0]))
			{
				// extract a bot number
				_BotNumber.push_back(NLMISC::atoui(s.c_str()));
			}
			else
			{

				std::string str = s;
				string::size_type pos = str.find(":");
				if (pos != string::npos)
				{
					const string groupName = str.substr(0, pos);
					CGroup* grp = CAIGroupFinder::findGroup(groupName,container);
					if(grp == NULL)
					{
						nlwarning("npc_say (%s) bad group name (%s) !", eventNode->fullName().c_str(), groupName.c_str());
						return;
					}
					_Groups.push_back(grp);
					str = str.substr(pos+1);
					_BotNames.push_back(str);
					_R2 = true;
				}
				else
				{
					_BotNames.push_back(s);
					_Groups.push_back(0);
				}
		
			}

		}
		if	(	!_BotNames.empty()
			&&	!_BotNumber.empty())
		{
			nlwarning("emot (%) : mixing bot name and bot number is dangerous !", eventNode->fullName().c_str());
		}

	}

	bool executeAction(CStateInstance* entity, IAIEvent const* event)
	{
		CGroup	*group = entity->getGroup();
		if (_EmotNumber==MBEHAV::NUMBER_OF_BEHAVIOURS)
			return	false;

		vector<CAIEntityPhysical *>	eps;

		if	(	TempSpeaker
			&&	TempSpeaker->getEntityId().getType()==group->getRyzomType()
			&&	safe_cast<CSpawnBot*>(TempSpeaker)->getPersistent().getOwner() == group)	// check that the tempSpeaker entitiy is in this group
		{
			
			// ok, this is a good 'speaker', it will play the emot
			eps.push_back(TempSpeaker);

			// turn it to face the first targeter
			CAIEntityPhysical *targeter = TempSpeaker->firstTargeter();
			if (targeter)
				safe_cast<CSpawnBot*>(TempSpeaker)->setTheta(TempSpeaker->pos().angleTo(targeter->pos()));
		}
		else
		{

			// r2 mode groupename:botname
			if (_R2)
			{
				uint first = 0, last = (uint)_Groups.size();
				for (; first != last; ++first)
				{
					CGroup	*grp = _Groups[first];
					if (grp == 0 ) { grp = group; }
					CAliasCont<CBot> const& bots = grp->bots();
					if(bots.size()!=0)
					{
							
						CBot* chld = bots.getChildByName(_BotNames[first]);
						if (chld )
						{
							CAIEntityPhysical *const ep = chld->getSpawnObj();
							if (ep)
							{
								eps.push_back(ep);
							}
						}					
					}
				}
			}
			// normal behavior botname or id
			else
			{

				for (uint i=0; i<group->bots().size(); ++i)
				{
					const	CBot *const	bot = group->getBot(i);
					if	(	!bot
						||	!bot->isSpawned())
						continue;


					CAIEntityPhysical *const	ep=bot->getSpawnObj();

					if (!_BotNames.empty())
					{
						const string *name;
						if (group->getRyzomType() == RYZOMID::npc)
						{
							CGroupNpc *const grpNpc = safe_cast<CGroupNpc*>(group);
							if	(	!grpNpc->botsAreNamed()
								||	bot->getAliasTreeOwner() == NULL)
							{
								name = &grpNpc->aliasTreeOwner()->getName();
							}
							else
							{
								name = &bot->getAliasTreeOwner()->getName();
							}

						}
						else
						{
							name = &group->aliasTreeOwner()->getName();
						}

						// if not found.
						if (find(_BotNames.begin(), _BotNames.end(), *name)==_BotNames.end())
							continue;
					}

					if	(	!_BotNumber.empty()
						&&	find(_BotNumber.begin(), _BotNumber.end(), i)==_BotNumber.end())
					{
						continue;
					}
					eps.push_back(ep);
				}

			}
		}

					
		// send message
		if (!eps.empty())
		{
			for (uint i=0; i<eps.size(); ++i)
			{
				NLMISC::CEntityId	entityId=eps[i]->getEntityId();
				
				LOG("action emot : setting emot %u on bot %s", _EmotNumber, entityId.toString().c_str());
				// the filter test passed, apply the emot
				NLNET::CMessage msgout("SET_BEHAVIOUR");
				msgout.serial(entityId);
				MBEHAV::CBehaviour bh(_EmotNumber);
				bh.Data = (uint16)(CTimeInterface::gameCycle());
				msgout.serial(bh);

				NLNET::CUnifiedNetwork::getInstance()->send( "EGS", msgout );
			}

		}
		else
		{
			if(group)
				nlwarning("action emote for GroupAlias %s Group name %s : no bots can play the emot ! ", group->getAliasString().c_str(), group->getFullName().c_str());
			else
				nlwarning("action emote: no bots can play the emot !");
		}
			
		return true;
	}
private:
	MBEHAV::EBehaviour	_EmotNumber;
	// a list of bot names that will play the emot
	vector<string>		_BotNames;
	vector<CGroup*>     _Groups;
	// a list of bot order number that will play the emot
	vector<uint32>		_BotNumber;
	bool				_R2;
};


bool CAILogicDynamicIfHelper::_ConditionSuccess = false;

void CAILogicActionSitDownHelper::sitDown(CGroup* group)
{
	// stand up each bots		

	for (CCont<CBot>::iterator it=group->bots().begin(), itEnd=group->bots().end(); it!=itEnd; ++it)
	{
		CSpawnBot *const sb = it->getSpawnObj();
		if	(	!sb
			||	sb->getMode() == MBEHAV::SIT)
			continue;
		
		sb->setMode(MBEHAV::SIT);
	}
}

void CAILogicActionSitDownHelper::standUp(CGroup* group)
{
			// stand up each bots		
		for (CCont<CBot>::iterator it=group->bots().begin(), itEnd=group->bots().end(); it!=itEnd; ++it)
		{
			CSpawnBot *const sb = it->getSpawnObj();
			if	(	!sb
				||	sb->getMode() != MBEHAV::SIT)
				continue;

			sb->setMode(MBEHAV::NORMAL);
		}
}




class CAILogicActionSitDown : public IAILogicAction
{
public:
	CAILogicActionSitDown (const std::vector<std::string> &args, 
									const std::vector<IAILogicAction::TSmartPtr> &subActions, 
									const CAIAliasDescriptionNode *eventNode, 
									CStateMachine *container)
	{
		if (!subActions.empty())
		{
			nlwarning("sit_down (%s) should not have sub action", eventNode->fullName().c_str());
		}
		if (!args.empty())
		{
			nlwarning("sit_down (%s) doesn have parameters, ignoring!", eventNode->fullName().c_str());
		}
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		CGroup	*const	group = entity->getGroup();
		 CAILogicActionSitDownHelper::sitDown(group);
		return true;
	}
};

class CAILogicActionStandUp : public IAILogicAction
{
public:
	CAILogicActionStandUp (const std::vector<std::string> &args, 
									const std::vector<IAILogicAction::TSmartPtr> &subActions, 
									const CAIAliasDescriptionNode *eventNode, 
									CStateMachine *container)
	{
		if (!subActions.empty())
		{
			nlwarning("stand_up (%s) should not have sub action", eventNode->fullName().c_str());
		}
		if (!args.empty())
		{
			nlwarning("stand_up (%s) doesn have parameters, ignoring!", eventNode->fullName().c_str());
		}
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		CGroup	*const	group = entity->getGroup();
		CAILogicActionSitDownHelper::standUp(group);
		return true;
	}

};


class CAILogicActionSetFaunaActivity : public IAILogicAction
{
	enum TFaunaActivity
	{
		fa_rest,
		fa_food,
		fa_invalid
	};

	//	dont seems to be ok, Boris, check this code.

	TFaunaActivity	_Activity;
public:
	CAILogicActionSetFaunaActivity (const std::vector<std::string> &args, 
									const std::vector<IAILogicAction::TSmartPtr> &subActions, 
									const CAIAliasDescriptionNode *eventNode, 
									CStateMachine *container)
	{
		if (args.empty())
		{
			nlwarning("set_fauna_activity (%s) : missing activity parameter", eventNode->fullName().c_str());
			_Activity = fa_invalid;
		}
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if (_Activity == fa_invalid)
			return false;

		CGroup	*group = entity->getGroup();

		if (group->getRyzomType()!=RYZOMID::creature)
		{
			nlwarning("set_fauna_activity : the group '%s' is not a fauna group !",
				group->getAliasFullName().c_str());
			return	true;
		}

		CGrpFauna *grpFauna = safe_cast<CGrpFauna*>(group);

		CGrpFauna::TPlaces place;
		_Activity == fa_rest ? place = CGrpFauna::REST_PLACE : CGrpFauna::EAT_PLACE;
		// change the state of the fauna group if needed
		// lookup for the cycle we want
		uint32 i;
		for (i=0; i<CGrpFauna::nbCycle; ++i)
		{
			if (grpFauna->cycles[i]._Place == place)
				break;
		}

		if (i == CGrpFauna::nbCycle)
		{
			nlwarning("set_fauna_activity : Can't find fauna activity '%s' in fauna group '%s'",
				place == CGrpFauna::REST_PLACE ? "REST_PLACE" : "EAT_PLACE",
				grpFauna->getAliasFullName().c_str());
			return false;
		}

		grpFauna->getSpawnObj()->setCurrentCycle(i);
		return true;
	}
};
/*
class CAILogicActionOutpostGiverReady : public IAILogicAction
{
public:
	CAILogicActionOutpostGiverReady (const std::vector<std::string> &args, 
									const std::vector<IAILogicAction::TSmartPtr> &subActions, 
									const CAIAliasDescriptionNode *eventNode, 
									CStateMachine *container)
	{
		if (!args.empty())
		{
			nlwarning("CAILogicActionOutpostGiverReady : should not have parameters");
		}
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		// just find the outpost name in the group name.
		CGroup	*const	group = entity->getGroup();
		CSpawnGroupNpc *const	spawnGroup = dynamic_cast<CSpawnGroupNpc *>(static_cast<CSpawnGroup*>(group->createSpawnGroup()));

		if (!spawnGroup)
		{
			nlwarning("CAILogicActionOutpostGiverReady : group '%s'%s is not an npc group",
				group->getAliasFullName().c_str(),
				group->getAliasString().c_str());
			return false;
		}

		const std::string &groupName = group->getName();

		// the group name should be in the format "outpost_<continent>_<outpost_number>_<group_civ>_steward_<continent_short>1_group"
		// and we must keep "outpost_<continent>_<number>" for the outpost name

		vector<string> parts;
		explode(groupName, "_", parts, false);
		if (parts.size() != 7)
		{
			nlwarning("CAILogicActionOutpostGiverReady: Invalid outpost mission giver '%s', need 'outpost_<continent>_<outpost_number>_<group_civ>_steward_<continent_short>1_group' for ", groupName.c_str());
			nlwarning("CAILogicActionOutpostGiverReady: can't process action for group '%s'%s", 
				group->getAliasFullName().c_str(), 
				group->getAliasString().c_str());
			return false;
		}

		string outpostName = parts[0]+"_"+parts[1]+"_"+parts[2];

		// retreive the outpost
		COutpost *const	outpost = COutpost::getOutpostByName(outpostName);
		if	(!outpost)
		{
			nlwarning("CAILogicActionOutpostGiverReady : can't find outpost '%s'", outpostName.c_str());
			return false;
		}

		// just call new event, this will spawn the corect npc
		outpost->newEvent();

		return true;
	}
};
*/
//-------------------------------------------------------------------------------------------
//a switch action. 
//Parameters : a variable ([group_name:]v{0,1,2,3})
//Optional a list of label '0', '1', '2', 'default'

//execute the subAction at the index equal to the variable value

class CAILogicActionSwitchActions : public IAILogicAction, public CAIVariableParser
{
public:
	CAILogicActionSwitchActions (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		// this line treated first ... in case we bomb out in one of the if(...) { ... return; } cases

		uint32 nbArgs = (uint32)args.size();
		if (nbArgs==0)
		{
			nlwarning("switch_actions (%s) need an argument !", eventNode->fullName().c_str());
			return;
		}
		if (subActions.empty())
		{
			nlwarning("switch_actions (%s) need sub actions !", eventNode->fullName().c_str());
			return;
		}
		_SubActions = subActions;
		// parse the argument string.

		if (!parseVar(_Var, args[0],container))
		{
			nlwarning("switch_actions (%s) error parsing var in action '%s'", args[0].c_str(), eventNode->fullName().c_str());
			return;
		}

		// if size > 1 the other params are labels  eg '0' '1' '2' 'default'
		if(nbArgs>1)
		{
			if (nbArgs<subActions.size()+1)
			{
				nlwarning("CAILogicActionSwitchActions : error! not enough weights in parameters !");
			}
			else
			{
				_Labels.resize(nbArgs);

				for(uint32 i=0; i<nbArgs-1; ++i)
				{
					std::string label = args[i+1];
					if(label =="default")
					{
						_DefaultAction = subActions[i];
						_Labels[i] = -1; // the default case eg "default:" -> _Label[?] = -1
					}
					else
					{
						sint32 value;
						NLMISC::fromString(label, value);
						_Labels[i] = value ; // the other case eg "case 4:" -> _Label[?] = 4;
					}
				}
			}
		}
	}

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if (_SubActions.empty()) { return true; }
			
		uint32 val = static_cast<uint32>(retreiveValue(entity,_Var));
		IAILogicAction::TSmartPtr action;
		if(_Labels.empty())
		{
			if(val>=_SubActions.size())
			{
				return false;
			}
			action = _SubActions[val];
		}
		else
		{
			//look in the labels the action corresponding the Value 
			action = 0;
			for (uint i=0; i<_Labels.size() && !action ;++i)
			{
				if (_Labels[i] != -1 && static_cast<uint32>(_Labels[i]) == val )
				{
					action = _SubActions[i];
				}
			}
			
			//not found so use the default value
			if (!action) { action = _DefaultAction; }
		}
		BOMB_IF(!action,"ERROR: Bad action pointer!",return false);

		return action->executeAction(entity, event);
	}
protected:
	CAIVariableParser::TVariable	_Var;
	std::vector<IAILogicAction::TSmartPtr> _SubActions;
	std::vector<sint32> _Labels;
	IAILogicAction::TSmartPtr _DefaultAction;
};
class	CPropertyZone
:public	CRefCount
{
public:	
	typedef	NLMISC::CSmartPtr<CPropertyZone>	TSmartPtr;
	CPropertyZone	()
		:_Shape(true),
		_Target(CTmpPropertyZone::All)
	{
	}
	virtual	~CPropertyZone	()
	{
	}
	CShape	_Shape;
	CPropertySet	_Properties;
	CTmpPropertyZone::TTarget _Target;
};

class	CZoneMarker
:public	NLMISC::CVirtualRefCount
{
public:	
	
	CZoneMarker(CAIInstance	*aiInstance, 
		        const	std::vector<CPropertyZone::TSmartPtr>	&zones,
				CAliasCont<CGroupFamily> *substitutionGroupFamily,
				size_t	substitutionId
			   )
		:_AiInstance(aiInstance)
		,_Zones(zones)
		,_SubstitutionId(substitutionId)
	{
		//	add activities flags.
#if !FINAL_VERSION
		nlassert(!_AiInstance.isNULL());
#endif
		if (substitutionGroupFamily)
		{
			_SubstitutionGroupFamilies = *substitutionGroupFamily;
			if (!_SubstitutionGroupFamilies.isEmpty())
			{
				std::sort(_SubstitutionGroupFamilies.getInternalCont().begin(), _SubstitutionGroupFamilies.getInternalCont().end()/*, std::less<CSmartPtr<CGroupFamily> >*/);
				// must sort because we're gonna use set_union later
				for (uint k = 0; k < _SubstitutionGroupFamilies.size(); ++k)
				{
					nlassert(_SubstitutionGroupFamilies[k] != NULL);
					nlassert(substitutionId != 0);
					_SubstitutionGroupFamilies[k]->setSubstitutionId(substitutionId);
				}
			}
		}
		parseCell	(true);
	}
	virtual		~CZoneMarker()
	{
		//	remove activities flags.
		parseCell	(false);
		_AiInstance=(CAIInstance*)NULL;
	}


private:
	void	parseCell	(bool setFlag);
	

	size_t	getId()	const
	{
		return	(size_t)this;
	}
	
	CDbgPtr<CAIInstance>	_AiInstance;
	const	std::vector<CPropertyZone::TSmartPtr>	&_Zones;
	CAliasCont<CGroupFamily>						_SubstitutionGroupFamilies;
	size_t											_SubstitutionId;

};



void	CZoneMarker::parseCell	(bool setFlag)
{		
	for (uint k = 0; k < _SubstitutionGroupFamilies.size(); ++k)
	{
		nlassert(_SubstitutionGroupFamilies[k]);
		if (setFlag)
		{
			_SubstitutionGroupFamilies[k]->addZoneMarkerRef(this);
		}
		else
		{
			_SubstitutionGroupFamilies[k]->removeZoneMarkerRef(this);
		}
	}
	std::vector<CBaseZone *> bz;
	for (CCont<CContinent>::iterator itCont(_AiInstance->continents().begin()), itEndCont(_AiInstance->continents().end()); itCont != itEndCont; ++itCont)
	{
		for (CCont<CRegion>::iterator	itRegion=itCont->regions().begin(), itEndRegion=itCont->regions().end(); itRegion!=itEndRegion; ++itRegion)
		{
			for (CCont<CCellZone>::iterator	itCellZone=itRegion->cellZones().begin(), itEndCellZone=itRegion->cellZones().end(); itCellZone!=itEndCellZone; ++itCellZone)
			{
				for (CCont<CCell>::iterator	itCell=itCellZone->cells().begin(), itEndCell=itCellZone->cells().end(); itCell!=itEndCell; ++itCell)
				{
					bz.clear();
					for (CCont<CFaunaZone>::iterator	itFaunaZone=itCell->faunaZones().begin(), itEndFaunaZone=itCell->faunaZones().end(); itFaunaZone!=itEndFaunaZone; ++itFaunaZone)
					{
						if	(!itFaunaZone->worldValidPos().isValid())
							continue;
						bz.push_back(*itFaunaZone);
					}
					for(uint k = 0; k < itCell->npcZoneCount(); ++k)					
					{
						if	(!itCell->npcZone(k)->worldValidPos().isValid())
							continue;
						bz.push_back(itCell->npcZone(k));
					}
					for(std::vector<CBaseZone *>::iterator itBaseZone = bz.begin(); itBaseZone != bz.end(); ++ itBaseZone)
					{
						for (size_t zoneInd=0;zoneInd<_Zones.size();zoneInd++)
						{
							CPropertyZone::TSmartPtr	zone=_Zones[zoneInd];
							
							// see if zone type is ok
							if (zone->_Target != CTmpPropertyZone::All)
							{
								if (zone->_Target != (*itBaseZone)->getZoneType()) continue;
							}

							if	(!zone->_Shape.contains	((*itBaseZone)->worldValidPos()))
								continue;
							
							//	set or unset flags ..
							if	(setFlag)
							{
								(*itBaseZone)->additionalActivities().merge(zone->_Properties,getId());
								if (_SubstitutionId != 0)
								{									
									// merge with the familyGroup of the region
									CAliasCont<CGroupFamily> &grf = itRegion->groupFamilies();
									std::sort(grf.getInternalCont().begin(), grf.getInternalCont().end());
									CAliasCont<CGroupFamily> result;
									std::set_union(grf.getInternalCont().begin(), grf.getInternalCont().end(), 
												   _SubstitutionGroupFamilies.getInternalCont().begin(), _SubstitutionGroupFamilies.getInternalCont().end(),
												   std::back_inserter(result.getInternalCont()));
									grf.swap(result);
									// mark fauna zone to accept this substitution group
									(*itBaseZone)->addSubstitutionGroupFamilyId(_SubstitutionId);
								}
							}
							else
							{								
								(*itBaseZone)->additionalActivities().removeProperties(getId());
								if (_SubstitutionId != 0)
								{
									// mark fauna zone to reject this substitution group
									(*itBaseZone)->removeSubstitutionGroupFamilyId(_SubstitutionId);
								}
							}							
						}

					}
					
				}
				// rebuild cell zones energy levels if substitution groups where added 
				// (in order to create missing family behaviour objects)
				if (_SubstitutionId != 0 && setFlag)
				{
					itCellZone->rebuildEnergyLevels();
				}
			}
			
		}
		
	}
		
}


class CAILogicActionSetFlagsOnDynZones: public IAILogicAction
{
public:
	CAILogicActionSetFlagsOnDynZones (const std::vector<std::string> &args, 
		const std::vector<IAILogicAction::TSmartPtr> &subActions, 
		const CAIAliasDescriptionNode *eventNode, 
		CStateMachine *container)		
	{

		for (uint32 i=0;i<args.size();++i)
		{
			_Activities.addProperty(CPropertyId::create(args[i]));	//	it->first));
		}

	}
	
	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		if	(!entity)
			return	true;

		const	CAIState	*state=entity->getActiveState();
		if	(	!state
			||	!state->isPositional())
			return	true;

		const CAIStatePositional	*positionalState=NLMISC::safe_cast<const CAIStatePositional*>(state);	//	good coz of previous check.

//		if	(!positionalState->shape().hasPatat())	//	must have a pattat.
//			return	true;

		IManagerParent	*const	managerParent=entity->getGroup()->getOwner()->getOwner();

		CAIInstance	*const	aiInstance=dynamic_cast<CAIInstance*>(managerParent);
		if	(!aiInstance)
			return	true;

		//	now, we have to add an objet that marks flag to zones at construction and remove them at destruction.
		//	and let StateChange take care of removing it when necessary.

		const	bool	haveAPattat=positionalState->shape().hasPatat();
		if	(haveAPattat)	//	must have a pattat.
		{
			const	CPropertyZone::TSmartPtr	newZone=new CPropertyZone();
			newZone->_Shape=positionalState->shape();
			newZone->_Properties=_Activities;
			_Zones.push_back(newZone);
		}
		// retrieve substitution groups from parent
		if (_GroupFamilies.isEmpty())
		{
			entity->addStatePersistentObj(state, new CZoneMarker(aiInstance, _Zones, NULL, 0 ));	//	remove for instance
		}
		else
		{
			entity->addStatePersistentObj(state, new CZoneMarker(aiInstance, _Zones, &_GroupFamilies, (size_t) this) );	//	remove for instance
		}
		
		if	(haveAPattat)
			_Zones.pop_back();
		return	true;
	}

	void addPropertyZone	(CTmpPropertyZone::TSmartPtr	zone)
	{
		CPropertyZone::TSmartPtr	newZone=new CPropertyZone();
		if (!newZone->_Shape.setPatat(zone->verticalPos, zone->points))
		{
			
		}
		newZone->_Properties=zone->properties;
		newZone->_Target = zone->Target;
		_Zones.push_back(newZone);
	}

	
	virtual void addGroupFamily(CGroupFamily *grpFam) { _GroupFamilies.addChild(grpFam); }
	
private:
	std::vector<CPropertyZone::TSmartPtr>	_Zones;
	CPropertySet	  _Activities;	
	CAliasCont<CGroupFamily> _GroupFamilies;
};


class CAILogicActionSpawnDynGroup: public IAILogicAction
{
public:
	CAILogicActionSpawnDynGroup (const std::vector<std::string> &args, 
		const std::vector<IAILogicAction::TSmartPtr> &subActions, 
		const CAIAliasDescriptionNode *eventNode, 
		CStateMachine *container)
	{
		nlwarning("loadActionSpawnDynGroup");
		_StateMachine=findParam	("state_machine",args);
		_DynGroup=findParam	("dyn_group",args);
		_Where=findParam	("where",args);
		_Count=findParam	("count",args);
	}

	std::string	findParam	(const std::string &str, const std::vector<std::string> &args)
	{
		for (uint32 i=0;i<args.size();++i)
		{
			const string &argStr=args[i];

			if (argStr.find(str)!=string::npos)
			{				
				size_t index=argStr.find_last_of(" ");
				if	(	index==string::npos
					||	(index+1)>=argStr.size())
					continue;

				index++;
				return	argStr.substr(index, argStr.size()-index);
			}

		}
		return	string();
	}
	
	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		nlwarning("executeActionSpawnDynGroup");
		if	(!entity)
			return	true;
				
		IManagerParent	*const	managerParent=entity->getGroup()->getOwner()->getOwner();
		
		CAIInstance	*const	aiInstance=dynamic_cast<CAIInstance*>(managerParent);
		if	(!aiInstance)
			return	true;

		const	CGroupDesc<CGroupFamily>	*groupDesc=NULL;
		
		const CGroupFamily *groupFamily = NULL;

		//	Find Group.

		FOREACH (itCont, CCont<CContinent>, aiInstance->continents())
		{
			FOREACH (itRegion, CCont<CRegion>, itCont->regions())
			{
				FOREACH (itFamily, CCont<CGroupFamily>, itRegion->groupFamilies())
				{
					FOREACH (itGroupDesc, CCont<CGroupDesc<CGroupFamily> >, itFamily->groupDescs())
					{
						if	(itGroupDesc->getName()==_DynGroup)
						{
							groupFamily = &(**itFamily);
							groupDesc=*itGroupDesc;
							goto	groupFound;
						}

					}

				}

			}

		}
groupFound:

		const	CStateMachine *stateMachine=NULL;
		CManager	*manager;
		// Find State_Machine.
		FOREACH(itCont,CCont<CManager>,aiInstance->managers())
		{
			if	(itCont->getName()==_StateMachine)
			{
				manager=*itCont;
				break;
			}

		}
		if (!manager)
			return	true;
		CMgrNpc	*const	npcManager=dynamic_cast<CMgrNpc*>(manager);
		if (!npcManager)
			return	true;
			
		stateMachine=manager->getStateMachine();
		if	(stateMachine->cstStates().size()==0)
			stateMachine=NULL;

		std::vector<CCellZone*>	cellZones;

		FOREACH (itCont, CCont<CContinent>, aiInstance->continents())
		{
			FOREACH (itRegion, CCont<CRegion>, itCont->regions())
			{
				FOREACH (itCellZone, CCont<CCellZone>, itRegion->cellZones())
				{
					cellZones.push_back(*itCellZone);
				}

			}

		}
		std::random_shuffle(cellZones.begin(), cellZones.end());

		const	CNpcZone	*spawnZone;
		FOREACH(itCellZone, vector<CCellZone*>, cellZones)
		{
			spawnZone=(*itCellZone)->lookupNpcZone(AITYPES::CPropertySet(), groupFamily->getSubstitutionId());
			if	(spawnZone)
				break;
		}
				
		// set the npc of the group attackable
//		groupDesc->setAttackable(true);
		
		CGroupNpc	*grp=groupDesc->createNpcGroup	(npcManager, spawnZone->midPos());
		
		if	(grp)
		{
			grp->initDynGrp		(groupDesc, NULL);
			grp->setStartState	(stateMachine->cstStates()[0]);	//	sets the first state.
			CSpawnGroupNpc	*const	spawnGrp=grp->getSpawnObj();
//			spawnGrp->activityProfile().setAIProfile(new CGrpProfileDynHarvest(spawnGrp,this,spawnZone,spawnZone));
		}
		return	true;
	}
	
private:
	std::string	_StateMachine;
	std::string	_DynGroup;
	std::string	_Where;
	std::string	_Count;	
};



//////////////////////////////////////////////////////////////////////////
//	Special Code Action

class CAIAliasDescriptionNode;
class CStateMachine;

class CAILogicActionCode : public IAILogicAction
{
public:
	CAILogicActionCode(
		const std::vector<std::string> &args,
		const std::vector<IAILogicAction::TSmartPtr> &subActions, 
		const CAIAliasDescriptionNode *eventNode,
		CStateMachine *container);	
	bool executeAction(CStateInstance *entity, const IAIEvent *event);
	NLMISC::CSmartPtr<const AIVM::CByteCode> _byteCode;
};


CAILogicActionCode::CAILogicActionCode (const std::vector<std::string> &args, 	const std::vector<IAILogicAction::TSmartPtr> &subActions, 
	const CAIAliasDescriptionNode *eventNode, 	CStateMachine *container)
{
	nldebug("loadActionCode");
	_byteCode=CCompiler::getInstance().compileCode	(args, eventNode->fullName());
}
	
bool	CAILogicActionCode::executeAction(CStateInstance	*entity,const IAIEvent *event)
{
	entity->interpretCode(NULL,	_byteCode);
	return	true;
}


//
//////////////////////////////////////////////////////////////////////////
// Special outpost action

//class CAIAliasDescriptionNode;
//class CStateMachine;

class CAILogicActionOutpostSendSquadStatus
: public IAILogicAction
{
public:
	CAILogicActionOutpostSendSquadStatus(
		std::vector<std::string> const& args,
		std::vector<IAILogicAction::TSmartPtr> const& subActions,
		CAIAliasDescriptionNode const* eventNode,
		CStateMachine* container);	
	bool executeAction(CStateInstance* entity, IAIEvent const* event);
};


CAILogicActionOutpostSendSquadStatus::CAILogicActionOutpostSendSquadStatus(
   std::vector<std::string> const& args,
   std::vector<IAILogicAction::TSmartPtr> const& subActions,
   CAIAliasDescriptionNode const* eventNode,
   CStateMachine* container)
{
}

bool CAILogicActionOutpostSendSquadStatus::executeAction(CStateInstance* entity, IAIEvent const* event)
{
	CGroup* group = entity->getGroup();
	if (!group)
	{
		nlwarning("send_outpost_squad_status failed because state instance is not a group");
		return false;
	}
	CGroupNpc* npcGrp = dynamic_cast<CGroupNpc*>(group);
	if (!npcGrp)
	{
		nlwarning("send_outpost_squad_status failed because group is not a NPC group");
		return false;
	}
	CManager* manager = npcGrp->getOwner();
	IManagerParent* managerParent = manager->getOwner();
	COutpost* outpost = dynamic_cast<COutpost*>(managerParent);
	if (!outpost)
	{
		nlwarning("send_outpost_squad_status failed because group manager parent is not an outpost");
		return false;
	}
	outpost->sendOutpostSquadStatus(npcGrp);
	return true;
}

//
//////////////////////////////////////////////////////////////////////////
// Special outpost action

class CAILogicActionOutpostReportSquadLeaderDeath
: public IAILogicAction
{
public:
	CAILogicActionOutpostReportSquadLeaderDeath(
		std::vector<std::string> const& args,
		std::vector<IAILogicAction::TSmartPtr> const& subActions,
		CAIAliasDescriptionNode const* eventNode,
		CStateMachine* container);	
	bool executeAction(CStateInstance* entity, IAIEvent const* event);
};


CAILogicActionOutpostReportSquadLeaderDeath::CAILogicActionOutpostReportSquadLeaderDeath(
	std::vector<std::string> const& args,
	std::vector<IAILogicAction::TSmartPtr> const& subActions,
	CAIAliasDescriptionNode const* eventNode,
	CStateMachine* container)
{
}

bool CAILogicActionOutpostReportSquadLeaderDeath::executeAction(CStateInstance* entity, IAIEvent const* event)
{
	CGroup* group = entity->getGroup();
	if (!group)
	{
		nlwarning("outpost_report_squad_leader_death failed because state instance is not a group");
		return false;
	}
	CGroupNpc* npcGrp = dynamic_cast<CGroupNpc*>(group);
	if (!npcGrp)
	{
		nlwarning("outpost_report_squad_leader_death failed because group is not a NPC group");
		return false;
	}
	CManager* manager = npcGrp->getOwner();
	IManagerParent* managerParent = manager->getOwner();
	COutpost* outpost = dynamic_cast<COutpost*>(managerParent);
	if (!outpost)
	{
		nlwarning("outpost_report_squad_leader_death failed because group manager parent is not an outpost");
		return false;
	}
	outpost->squadLeaderDied(npcGrp);
	return true;
}


//
//////////////////////////////////////////////////////////////////////////
// Special outpost action

class CAILogicActionOutpostReportSquadDeath
: public IAILogicAction
{
public:
	CAILogicActionOutpostReportSquadDeath(
		std::vector<std::string> const& args,
		std::vector<IAILogicAction::TSmartPtr> const& subActions,
		CAIAliasDescriptionNode const* eventNode,
		CStateMachine* container);	
	bool executeAction(CStateInstance* entity, IAIEvent const* event);
};


CAILogicActionOutpostReportSquadDeath::CAILogicActionOutpostReportSquadDeath(
	std::vector<std::string> const& args,
	std::vector<IAILogicAction::TSmartPtr> const& subActions,
	CAIAliasDescriptionNode const* eventNode,
	CStateMachine* container)
{
}

bool CAILogicActionOutpostReportSquadDeath::executeAction(CStateInstance* entity, IAIEvent const* event)
{
	CGroup* group = entity->getGroup();
	if (!group)
	{
		nlwarning("outpost_report_squad_death failed because state instance is not a group");
		return false;
	}
	CGroupNpc* npcGrp = dynamic_cast<CGroupNpc*>(group);
	if (!npcGrp)
	{
		nlwarning("outpost_report_squad_death failed because group is not a NPC group");
		return false;
	}
	CManager* manager = npcGrp->getOwner();
	IManagerParent* managerParent = manager->getOwner();
	COutpost* outpost = dynamic_cast<COutpost*>(managerParent);
	if (!outpost)
	{
		nlwarning("outpost_report_squad_death failed because group manager parent is not an outpost");
		return false;
	}
	outpost->squadDied(npcGrp);
	return true;
}

//------------------------------------------------------------------
//A say action where it is possible to specify which npc is talking.
//------------------------------------------------------------------
class CAILogicActionNpcSay : public IAILogicAction, public CAIVariableParser
{
public:
	CAILogicActionNpcSay (const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		_Arg = false;
		if (args.empty())
		{
			nlwarning("CAILogicActionNpcSay need a sentence !");
			return;
		}
		
		
		if (!subActions.empty())
		{
			nlwarning("npc_say (%s) should not have sub action", eventNode->fullName().c_str());
			return;
		}

		//no npc specified, (use leader)
		if(args.size()==1)
		{
			_Npc = "";
			_Group = NULL;
			_Sentence = args[0];
		}
		else 
			//npc specified + string
		if (args.size()>=2)
		{
			std::string str = args[0];

			string::size_type pos = str.find(":");

			if (pos == string::npos)
			{
				pos = str.find(".");
			}
			if (pos != string::npos)
			{
				const string groupName = str.substr(0, pos);
				CGroup* grp = CAIGroupFinder::findGroup(groupName,container);
				if(grp == NULL)
				{
					nlwarning("npc_say (%s) bad group name (%s) !", eventNode->fullName().c_str(),groupName.c_str());
					return;
				}
				_Group = grp;
				str = str.substr(pos+1);
				_Npc = str;
			}
			else
			{
				_Group = NULL;
				_Npc=args[0];
			}
			_Sentence=args[1];
		}
		/*
		//string with variables
		else if (args.size()>=3)
		{
			uint32 i =2;
			CAIVariableParser::TVariable var;
			for(;i<args.size();++i)
			{
				if (!parseVar(var, args[i],container))
				{
					nlwarning("Error while parsing variable %s !",args[i].c_str());
					return;
				}
				nlinfo("npc_say use variable <%s> !",args[i].c_str());
				_Vars.push_back(var);
			}
			_Arg=true;
		}*/
		else
		{
			nlwarning("<npc_say> error parameters count!");
		}

		std::string cstring =NLMISC::CSString (_Sentence).left(4);

		if(cstring=="DSS_")
		{
			_Id=true;
			NLMISC::CSString tmp = NLMISC::CSString (_Sentence).right((uint)_Sentence.length()-4);
			NLMISC::CSString tmp2 = tmp.strtok(" ",false,false,false,false);
			_ScenarioId = tmp2.atoui();
			_Sentence = tmp;
			nlwarning("<npc_say> scenario id : %d string id : %s ",_ScenarioId,_Sentence.c_str());
		}
		else
		{
			_Id=false;
		}
	}

	bool	executeAction(CStateInstance *entity,const IAIEvent *event)
	{
		if(_Sentence == "")
		{
			nlwarning("npc has nothing to say!");
			return false;
		}

		const CAIAliasDescriptionNode	*const aliasDesc=entity->aliasTreeOwner()->getAliasNode();

		if	(	aliasDesc->getType() != AITypeGrp
			||	!entity->getGroup()	)
		{
			nlwarning("group can't have 'npc_say' action !");
			return	true;
		}

		CGroup	*	grp ;
		if(_Group==NULL)
			grp= entity->getGroup();
		else
			grp = _Group;
		CSpawnBot* bot=NULL;
		if(!_Npc.empty())
		{
			CAliasCont<CBot> const& bots = grp->bots();
			if(bots.size()==0)
			{
				nlwarning("group empty! say nothing! ");
				return true;
			}

			CBot* chld=bots.getChildByName(_Npc);
			if(!chld)
			{
				std::string tmp= "no npc named "+_Npc+"! say nothing!";
				nlwarning(tmp.c_str());
				return true;
			}
			bot= chld->getSpawnObj();
		}
		else
		{
			CBot *b = grp->getSpawnObj()->getPersistent().getLeader();
			if (b && b->isSpawned())
			{
				bot = b->getSpawnObj();				
			}
		}
		
		if (!bot  || !bot->isAlive())
		{
				return true;
		}	

		if(!_Id)
		{
			ucstring ucstr = _Sentence;
			npcChatToChannelSentence(bot->dataSetRow(),CChatGroup::say, ucstr);
		}
		else
		{
			if(!_Arg)
			{
				forwardToDss(bot->dataSetRow(),CChatGroup::say,_Sentence,_ScenarioId);
			}
			else
			{
				float val;
				uint32 size=(uint32)_Vars.size(),i=0;
				std::vector<float> values;
				for(;i<size;++i)
				{
					val = retreiveValue(entity,_Vars[i]);
					values.push_back(val);
				}
				forwardToDssArg(bot->dataSetRow(),CChatGroup::say,_Sentence,_ScenarioId,values);
			}
		}
		return true;
	}
private:

	std::string _Sentence;
	uint32 _ScenarioId;
	std::string _Npc;
	CGroup* _Group;
	bool _Id;
	bool _Arg;
	vector<CAIVariableParser::TVariable> _Vars;
};
class CAILogicActionNull : public IAILogicAction
{
	public:
	CAILogicActionNull(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{}
	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{return true;}
	
};
//----------------------------------------------------------------------

void CAILogicActionDssStartActHelper::dssStartAct(TSessionId sessionId, uint32 actId)
{
	NLNET::CMessage	msgout("DSS_START_ACT");		
	msgout.serial(sessionId);
	msgout.serial(actId);
	NLNET::CUnifiedNetwork::getInstance()->send("DSS",msgout);
}

//----------------------------------------------------------------------



class CAILogicActionFacing : public IAILogicAction
{
public:
	CAILogicActionFacing(const std::vector<std::string> &args, const std::vector<IAILogicAction::TSmartPtr> 
								&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
	{
		if (args.empty()||args.size()!=2)
		{
			nlwarning("CAILogicActionFacing need two Npc name !");
			return;
		}
		
		
		if (subActions.size()!=0)
		{
			nlwarning("CAILogicActionFacing (%s) should not have any sub action !", eventNode->fullName().c_str());
			return;
		}
	//	_facedName = args[1];
		for(uint32 i=0;i<args.size();++i)
		{
			std::string name = args[i];
			std::string str = args[i];
			string::size_type pos = str.find(":");
			if (pos != string::npos)
			{
				const string groupName = str.substr(0, pos);
				CGroup* grp = CAIGroupFinder::findGroup(groupName,container);
				if(grp == NULL)
				{
					nlwarning("npc_say (%s) bad group name (%s) !", eventNode->fullName().c_str(),groupName.c_str());
					return;
				}
				_FacerGroups.push_back(grp);
				str = str.substr(pos+1);
				_FacerNames.push_back(str);
			}
			else
			{
				_FacerGroups.push_back(NULL);
				_FacerNames.push_back(name);
			}
		}
	}
	

	bool	executeAction(CStateInstance	*entity,const IAIEvent *event)
	{
		CGroup	*defaultGrp = entity->getGroup();
		
		CSpawnBot* botFacer=NULL;
		CSpawnBot* botFaced=NULL;


		if (_FacerNames.size()  !=2 ) 
		{
			nlwarning("Warning CAILogicActionFacing: need two Npc name !");
			return false;
		}

	
		{

			CGroup	*grp = _FacerGroups[0];
			if (grp == 0 ) { grp = defaultGrp; }
			
			CAliasCont<CBot> const& bots = grp->bots();
			if(bots.size()==0)
			{
				nlwarning("Warning CAILogicActionFacing: group empty! unable to face !");
				return false;
			}


			CBot* chld;

			chld=bots.getChildByName(_FacerNames[0]);
			if(!chld)
			{
				nlwarning("Warning CAILogicActionFacing: no npc named %s ! unable to face !",_FacerNames[0].c_str());
				return false;
			}
			botFaced= chld->getSpawnObj();
			if (!botFaced)
			{
				return true;
			}
		}

		{

			CGroup	*grp = _FacerGroups[1];
			if (grp == 0 ) { grp = defaultGrp; }
			
			CAliasCont<CBot> const& bots = grp->bots();
			if(bots.size()==0)
			{
				nlwarning("Warning CAILogicActionFacing: group empty! unable to face !");
				return false;
			}


			CBot* chld;

			chld=bots.getChildByName(_FacerNames[1]);
			if(!chld)
			{
				nlwarning("Warning CAILogicActionFacing: no npc named %s ! unable to face !",_FacerNames[1].c_str());
				return false;
			}
			botFacer= chld->getSpawnObj();
			if (!botFaced)
			{				
				return true;
			}
		}


		if(botFacer && botFaced)
		{

			botFacer->setTheta(botFacer->pos().angleTo(botFaced->pos()));
	
		}
		else
		{
			return false;
		}
			
		

		return true;
	}
private:
	std::vector<std::string> _FacerNames;
	std::vector<CGroup*> _FacerGroups;
};

//
//////////////////////////////////////////////////////////////////////////
//	Generic Actions
//////////////////////////////////////////////////////////////////////////

IAILogicAction	*CAIEventReaction::newAILogicAction(const char *name,
													const std::vector<std::string> &args, 
													const std::vector<IAILogicAction::TSmartPtr> &subActions, 
													const CAIAliasDescriptionNode *eventNode, 
													CStateMachine	*container)
{
#define BUILD(theName,theClass) if (NLMISC::nlstricmp(theName,name)==0) return new theClass(args,subActions,eventNode,container);
	BUILD(	"begin_state",			CAILogicActionBeginState		)
	BUILD(	"random_select_state",	CAILogicActionRandomSelectState	)
	BUILD(	"multi_actions",		CAILogicActionMultiAction		)
	BUILD(	"punctual_state",		CAILogicActionPunctualState		)
	BUILD(	"punctual_state_end",	CAILogicActionPunctualStateEnd	)
	BUILD(	"random_select",		CAILogicActionRandomSelect		)
	BUILD(	"set_state_timeout",	CAILogicActionSetStateTimeout	)
	BUILD(	"set_timer_t0",			CAILogicActionSetTimerT0		)
	BUILD(	"set_timer_t1",			CAILogicActionSetTimerT1		)
	BUILD(	"set_timer_t2",			CAILogicActionSetTimerT2		)
	BUILD(	"set_timer_t3",			CAILogicActionSetTimerT3		)
	BUILD(	"trigger_event_0",		CAILogicActionUserEvent0		)
	BUILD(	"trigger_event_1",		CAILogicActionUserEvent1		)
	BUILD(	"trigger_event_2",		CAILogicActionUserEvent2		)
	BUILD(	"trigger_event_3",		CAILogicActionUserEvent3		)
	BUILD(	"trigger_event_4",		CAILogicActionUserEvent4		)
	BUILD(	"trigger_event_5",		CAILogicActionUserEvent5		)
	BUILD(	"trigger_event_6",		CAILogicActionUserEvent6		)
	BUILD(	"trigger_event_7",		CAILogicActionUserEvent7		)
	BUILD(	"trigger_event_8",		CAILogicActionUserEvent8		)
	BUILD(	"trigger_event_9",		CAILogicActionUserEvent9		)
	BUILD(	"spawn",				CAILogicActionSpawn				)
	BUILD(	"despawn",				CAILogicActionDespawn			)
	BUILD(	"send_message",			CAILogicActionSendMessage		)
	BUILD(	"say",					CAILogicActionSay				)
	BUILD(  "npc_say",				CAILogicActionNpcSay			)
	BUILD(  "facing",				CAILogicActionFacing			)
	BUILD(	"dynamic_if",			CAILogicActionDynamicIf		)
	BUILD(	"condition_if",			CAILogicActionConditionIf		)
	BUILD(	"condition_if_else",	CAILogicActionConditionIfElse	)
	BUILD(	"modify_variable",		CAILogicActionModifyVariable	)
	BUILD(	"emot",					CAILogicActionEmot				)
	BUILD(	"sit_down",				CAILogicActionSitDown			)
	BUILD(	"stand_up",				CAILogicActionStandUp			)
	BUILD(	"set_fauna_activity",	CAILogicActionSetFaunaActivity	)
//	BUILD(	"outpost_giver_ready",	CAILogicActionOutpostGiverReady	)
	BUILD(	"set_flags_on_dyn_zones",	CAILogicActionSetFlagsOnDynZones	)
	BUILD(	"spawn_dyn_group",		CAILogicActionSpawnDynGroup		)
	BUILD(	"code",					CAILogicActionCode				)
	BUILD(	"outpost_send_squad_status",	CAILogicActionOutpostSendSquadStatus	)
	BUILD(	"outpost_report_squad_leader_death",	CAILogicActionOutpostReportSquadLeaderDeath	)
	BUILD(	"outpost_report_squad_death",	CAILogicActionOutpostReportSquadDeath	)
	BUILD(	"switch_actions", CAILogicActionSwitchActions			)
	BUILD(	"null_action", CAILogicActionNull						)

#undef BUILD
	return NULL;
}

//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseLogicAction,"Turn on or off or check the state of verbose logic action logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(VerboseLog, args[0]);

	nlinfo("VerboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}

#include "event_reaction_include.h"
