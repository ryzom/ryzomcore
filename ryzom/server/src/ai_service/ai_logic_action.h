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



#ifndef RYAI_LOGIC_ACTION_H
#define RYAI_LOGIC_ACTION_H

#include "alias_tree_owner.h"
#include "game_share/r2_basic_types.h"

class IAIEvent;

class CAIEventReaction;
class CGroupFamily;
class CGroup;
class CStateInstance;

//---------------------------------------------------------------------------------
// CAIEventReactionAction
//---------------------------------------------------------------------------------
// this is the base class for action types that can be used to process events for
// teams and other entiites

class	IAILogicAction
	:	public NLMISC::CRefCount
{
public:
	typedef	NLMISC::CSmartPtr<IAILogicAction> TSmartPtr;

	// NOTE: ctor takes following form:
	// CLASS(const std::vector<std::string> &args, const std::vector<CAIEventReactionAction *> &subActions) 

	// note that actions are NOT responsible for deleting child actions
	virtual	~IAILogicAction()
	{}

	// this is the execute 'callback' for the action type.
	// NOTE: This code should be fast and compact as it may be called very large numbers of times
	// depending on the whim of the level designers
	// returns true on success, false on failure
	virtual bool executeAction		(CStateInstance *entity,const IAIEvent *event)=0;

	virtual	void addPropertyZone	(CTmpPropertyZone::TSmartPtr	zone)
	{}

	// attach a group family to the action if it supports it (assert otherwise)
	virtual void addGroupFamily(CGroupFamily *gf) { nlassert(0); }
};

// Code use by native functions and LogicAction
class CAILogicActionDssStartActHelper
{
public:	
	static void dssStartAct(TSessionId sessionId, uint32 actId);	
};

class CAILogicActionSitDownHelper
{
public:	
	static void sitDown(CGroup* group);
	static void standUp(CGroup* group);
};


class CAILogicDynamicIfHelper
{
public:
	static void setConditionSuccess(bool value) {_ConditionSuccess = value;}
	static bool getConditionSuccess() { return _ConditionSuccess;}
private:
	static bool _ConditionSuccess;
};
//---------------------------------------------------------------------------------
#endif
