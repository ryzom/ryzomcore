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




#ifndef RYAI_EVENT_REACTION_H
#define RYAI_EVENT_REACTION_H

#include "ai_logic_action.h"
#include "keyword.h"
#include "child_container.h"


template <class T> class CAliasCont;
class CGroupFamily;


//	This is the base class for event handlers for NPC groups

class CAIEvent;
class CStateMachine;
class CAIEventDescription;
class CAIEventActionNode;

class CAIEventReaction :
	public	NLMISC::CDbgRefCount<CAIEventReaction>,
	public	NLMISC::CRefCount,
	public	CAliasChild<CStateMachine>
#ifdef NL_DEBUG
	, public NLMISC::IDbgPtrData
#endif
{
public:
	enum EType { Generic, FixedGroup, FixedState };


	// ctor & dtor ------------------------------------------------------
	CAIEventReaction(CStateMachine*	container,CAIAliasDescriptionNode *aliasDescription):
		CAliasChild<CStateMachine>(container,aliasDescription),	_eventMgr(), _action(NULL), _type(Generic)
	{
	}

	CAIEventReaction(CStateMachine*	container, uint32 alias, std::string const& name)
	: CAliasChild<CStateMachine>(container, alias, name)
	, _eventMgr()
	, _action(NULL)
	, _type(Generic)
	{
	}
	

	std::string getIndexString() const;

	void setType(EType type) 
	{
		_type=type;
	}

	// NOTE if the event mgr is set then we need to remove self from event mgr befor deletion
	virtual ~CAIEventReaction()
	{
	}

	// basic accessors --------------------------------------------------
	IAILogicAction::TSmartPtr getAction() const	{ return _action; }

	// write accesoors used during parse operations
	void processEventDescription(CAIEventDescription *dsc,CStateMachine *container);	// process a complete new event description record
	
	IAILogicAction	*newAILogicAction(	const char *name,const std::vector<std::string>	&args, const std::vector<IAILogicAction::TSmartPtr>	&subActions, const CAIAliasDescriptionNode *eventNode, CStateMachine	*container);	

	void setEvent	(const std::string &eventName,CStateMachine *container);// this causes the event reaction to be linked to an event manager
	void setState	(uint32 alias);						// used for events that apply to a specific state
	void setGroup	(uint32 alias);						// used for events that apply to a specific group

	// the following routine shouldn't be needed it should be superceded by a better alternative
	// in the event manager
	template <class TState>
	  bool	testCompatibility(CStateInstance	*const	stateInstance, const	TState	*const	state) const;

protected:
	// protected data ---------------------------------------------------
	NLMISC::CDbgPtr<CAIEvent>	_eventMgr;			// my parent

	IAILogicAction::TSmartPtr	_action;				// my child actions (managed by me)

	std::vector<uint32> _states;	// vector of aliases of specific states
	CKeywordFilter _stateFilter;	// keyword filter describing states that reation applies to

	std::vector<uint32> _groups;	// vector of aliases of specific groups
	CKeywordFilter _groupFilter;	// keyword filter for describing groups that reaction applies to

	EType _type;

	

	IAILogicAction::TSmartPtr buildAction(CAIEventActionNode *dsc, const CAIAliasDescriptionNode *eventNode, CStateMachine *container);
};

#endif
