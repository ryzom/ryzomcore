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

#ifndef RYAI_EVENT_REACTION_CONTAINER_H
#define RYAI_EVENT_REACTION_CONTAINER_H

#include "event_manager.h"

class CPersistentStateInstance;

class CStateMachine
#ifdef NL_DEBUG
: public NLMISC::IDbgPtrData
#endif
{
public:
	CStateMachine() { }
	virtual ~CStateMachine()
	{
		clearEventContainerContent ();
	}
	
	virtual std::vector<std::string> getMultiLineInfoString() const;
	
	CAIEvent EventPositionalStateTimeout;
	CAIEvent EventPunctualStateTimeout;
	CAIEvent EventStartOfState;
	CAIEvent EventEndOfState;
	CAIEvent EventUserTimer[4];
	CAIEvent EventUserEvent[10];
	CAIEvent EventEscortAway;
	CAIEvent EventEscortBack;
	
	CAIEvent EventVariableChanged;
	CAIEvent EventVariableChangedTab[4];
	CAIEvent EventPlayerTargetNpc;
	CAIEvent EventPlayerFollowNpc;
	CAIEvent EventBotBeginFight;
	CAIEvent EventBotTargetKilled;
	CAIEvent EventGroupBeginFight;
	CAIEvent EventGroupEndFight;
	
	CAIEvent EventLastBotDespawned;
	CAIEvent EventFirstBotSpawned;
	
	CAIEvent EventEGSUp;
	CAIEvent EventPlayerEnterTriggerZone;
	CAIEvent EventPlayerLeaveTriggerZone;

	
	virtual void registerEvents()
	{
		addEvent("punctual_state_timeout",	EventPunctualStateTimeout	);
		addEvent("start_of_state",			EventStartOfState			);
		addEvent("end_of_state",			EventEndOfState				);
		addEvent("state_timeout",			EventPositionalStateTimeout	);
		addEvent("timer_t0_triggered",		EventUserTimer[0]			);
		addEvent("timer_t1_triggered",		EventUserTimer[1]			);
		addEvent("timer_t2_triggered",		EventUserTimer[2]			);
		addEvent("timer_t3_triggered",		EventUserTimer[3]			);
		addEvent("user_event_0",			EventUserEvent[0]			);
		addEvent("user_event_1",			EventUserEvent[1]			);
		addEvent("user_event_2",			EventUserEvent[2]			);
		addEvent("user_event_3",			EventUserEvent[3]			);
		addEvent("user_event_4",			EventUserEvent[4]			);
		addEvent("user_event_5",			EventUserEvent[5]			);
		addEvent("user_event_6",			EventUserEvent[6]			);
		addEvent("user_event_7",			EventUserEvent[7]			);
		addEvent("user_event_8",			EventUserEvent[8]			);
		addEvent("user_event_9",			EventUserEvent[9]			);
		addEvent("escort_away",				EventEscortAway				);
		addEvent("escort_back",				EventEscortBack				);
		
		addEvent("variable_changed",		EventVariableChanged		);
		addEvent("variable_v0_changed",		EventVariableChangedTab[0]	);
		addEvent("variable_v1_changed",		EventVariableChangedTab[1]	);
		addEvent("variable_v2_changed",		EventVariableChangedTab[2]	);
		addEvent("variable_v3_changed",		EventVariableChangedTab[3]	);
		
		addEvent("player_target_npc",		EventPlayerTargetNpc		);
		addEvent("player_follow_npc",		EventPlayerFollowNpc		);
		addEvent("bot_begin_fight",			EventBotBeginFight			);
		addEvent("bot_target_killed",		EventBotTargetKilled		);
		addEvent("group_under_attack",		EventGroupBeginFight		);
		addEvent("group_attack_end",		EventGroupEndFight			);
		
		addEvent("group_despawned",			EventLastBotDespawned		);
		addEvent("group_spawned",			EventFirstBotSpawned		);
		addEvent("egs_up",					EventEGSUp					);
		addEvent("player_arrived_trigger_zone", EventPlayerEnterTriggerZone);
		addEvent("player_left_trigger_zone", EventPlayerLeaveTriggerZone);
	}	
	
	void addEvent(std::string const& name, CAIEvent& event)
	{
		_eventNameMap[name] = &event;
#ifdef NL_DEBUG
		_eventNameMap[name].setData(this);
#endif
		event.setName(name);
	}
	
	void delEvent(std::string const& name)
	{
		_eventNameMap.erase(name);
	}
	
	void clearEventContainerContent()
	{
		std::map<std::string, NLMISC::CDbgPtr<CAIEvent> >::iterator it=_eventNameMap.begin(), itEnd=_eventNameMap.end();
		while (it!=itEnd)
		{
			it->second->removeAllReaction();
			++it;
		}
		
		_eventReactions.clear();
		_eventNameMap.clear();
	}
	
	std::map<std::string,NLMISC::CDbgPtr<CAIEvent> > const& GetEventManagerMap() const
	{
		return _eventNameMap;
	}
	
	//--------------------------------------------------------------------------
	// CAIMgr METHOD Encapsulating user event trigger system 
	//--------------------------------------------------------------------------
	
	CAIEvent* getEventManager(std::string const& name)
	{
		std::map<std::string,NLMISC::CDbgPtr<CAIEvent> >::const_iterator it = GetEventManagerMap().find(name);
		if (it!=GetEventManagerMap().end())
			return it->second;
		return NULL;
	}
	
	std::string const& getEventManagerName(CAIEvent const* evtMgr)
	{
		static std::string const unknown("UNKNOWN MANAGER");
		typedef std::map<std::string, NLMISC::CDbgPtr<CAIEvent> > TTmpCont;
		FOREACH(it, TTmpCont, _eventNameMap)
		{
			if (it->second!=evtMgr)
				continue;
			return it->first;
		}
		return unknown;
	}
	
	virtual	std::string	getIndexString()
	{ 
		return std::string("0");
	}
	
	inline CAliasCont<CAIEventReaction>& eventReactions() { return _eventReactions; }
	
	CAliasCont<CAIState>& states() { return _states; }
	CAliasCont<CAIState> const& cstStates() const { return _states; }
	
protected:
	CAliasCont<CAIState> _states;
	CAliasCont<CAIEventReaction> _eventReactions;
	std::map<std::string,NLMISC::CDbgPtr<CAIEvent> > _eventNameMap;
};

#endif
