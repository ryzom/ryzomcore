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
#include "ai_mgr.h"
#include "ai.h"
#include "event_reaction.h"
#include "ai_grp.h"
#include "ais_actions.h"	//	for CWorkPtr::instance()
#include "continent.h"

using namespace	AITYPES;

std::string CAIEventReaction::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}


void CAIEventReaction::setEvent(const std::string &eventName,CStateMachine *container)
{
	if	(!_eventMgr.isNULL())
		_eventMgr->removeReaction(this);
	
	nlassert(container);
	_eventMgr= container->getEventManager(eventName);
#ifdef NL_DEBUG
	_eventMgr.setData( this );
#endif
	
	if	(!_eventMgr.isNULL())
	{
		_eventMgr->addReaction(this);
	}
	else
	{
		nlwarning( "Failed to find event manager for events of type '%s' ",	/*in Npc Mgr %u (%s)*/
			eventName.c_str());	//	, container->getAlias(), container->getAliasNode()->fullName().c_str());
	}
}

void CAIEventReaction::setState(uint32 alias)
{
	if	(!_eventMgr.isNULL())
		_eventMgr->removeReaction(this);
	
	_states.clear();
	_states.push_back(alias);
	
	if	(!_eventMgr.isNULL())
		_eventMgr->addReaction(this);
}

void CAIEventReaction::setGroup(uint32 alias)
{
	if	(!_eventMgr.isNULL())
		_eventMgr->removeReaction(this);
	
	_groups.clear();
	_groups.push_back(alias);
	
	if	(!_eventMgr.isNULL())
		_eventMgr->addReaction(this);
}

IAILogicAction::TSmartPtr CAIEventReaction::buildAction(CAIEventActionNode *dsc,	const CAIAliasDescriptionNode *eventNode, CStateMachine *container)
{
	std::vector<IAILogicAction::TSmartPtr> subActions;
	
	// build the sub actions into a vector
	for (uint i=0;i<dsc->Children.size();++i)
	{
		subActions.push_back(buildAction(dsc->Children[i],eventNode,container));
	}

	IAILogicAction::TSmartPtr	newAction=newAILogicAction(dsc->Action.c_str(),dsc->Args,subActions,eventNode,container);

	CWorkPtr::addLogicAction(newAction, dsc->Alias);

	for (uint i=0;i<dsc->_PropertyZones.size();++i)
	{
		newAction->addPropertyZone(dsc->_PropertyZones[i]);
//		subActions.push_back(buildAction(dsc->Children[i],eventNode,container));
	}
	
	// call the factory to build the logic action object from the description record
	return	newAction;
}

void CAIEventReaction::processEventDescription(CAIEventDescription *dsc,CStateMachine *container)
{
	// if we're updating remove event from manager before update to allow for sensible classification
	// on re-insertion
	if	(!_eventMgr.isNULL())
		_eventMgr->removeReaction(this);
	
	nlassert(container);
	// the event manager
	_eventMgr= container->getEventManager(dsc->EventType);
#ifdef NL_DEBUG
	_eventMgr.setData( this );
#endif
	
	// if this isn't a fixed state event deal with state parameters
	if	(_type!=FixedState)
	{
		// the named state list
		_states.clear();
		uint32 i;
		for (i=0;i<dsc->NamedStates.size();++i)
		{
			uint32 state=getAliasNode()->findAliasByNameAndType(dsc->NamedStates[i],AITypeNpcStateRoute);
			
			if (state==0)
				state=getAliasNode()->findAliasByNameAndType(dsc->NamedStates[i],AITypeNpcStateZone);
			
			if (state==0)
			{
//				nlwarning("Warning: Dodgy event setup in '%s' because named state '%s' not found", container->getAliasNode()->fullName().c_str(), dsc->NamedStates[i].c_str());
				nlwarning("Warning: Dodgy event setup because named state '%s' not found", dsc->NamedStates[i].c_str());
				continue;
			}
			_states.push_back(state);
		}
		
		// state keyword filter
		_stateFilter.clear();
		for (i=0;i<dsc->StateKeywords.size();++i)
		{
			CKeywordFilter filter;
			if (!CAIKeywords::stateFilter(dsc->StateKeywords[i], filter))
			{		
				nlwarning("There are some keyword error in '%s'", getAliasNode()->fullName().c_str());
				continue;
			}
			_stateFilter+=filter;
		}
		
	}
	
	// if this isn't a fixed group event deal with group parameters
	if	(_type!=FixedGroup)
	{
		uint32 i;
		// the named group list
		_groups.clear();
		for (i=0;i<dsc->NamedGroups.size();++i)
		{
			std::vector<CGroup*>	grps;
			// get all groups with the this name
			CWorkPtr::aiInstance()->findGroup(grps, dsc->NamedGroups[i]);
			const CAIAliasDescriptionNode *parent = getAliasNode();
			// retrieve the manager.
			while	(	parent
					&&	parent->getType() != AITypeManager)
			{
				parent = parent->getParent();
			}

			// remove any group that belong to another manager
			if (parent)
			{
				uint nbToRemove = 0;
				for (uint i=0; i<grps.size()-nbToRemove; ++i)
				{
					if (grps[i]->getManager().getAlias() != parent->getAlias())
					{
						std::swap(grps[i], grps.back());
					}
				}
				grps.erase(grps.end()-nbToRemove, grps.end());
				//grps.erase(std::remove_if(grps.begin(), grps.end(), CAliasTreeOwner::CAliasDiff(parent->getAlias())), grps.end());
			}
			else
			{
				nlwarning("In '%s': Can't find manager for this event!", getAliasFullName().c_str());
			}
			
			if (grps.empty())
			{
				nlwarning("In '%s': Dodgy event setup because named group '%s' not found",
					getAliasFullName().c_str(),
					dsc->NamedGroups[i].c_str());
				continue;
			}

			for (uint j=0; j<grps.size(); ++j)
			{
				_groups.push_back(grps[j]->aliasTreeOwner()->getAlias());
			}
		}
		
		// group keyword filter
		_groupFilter.clear();
		for (i=0;i<dsc->GroupKeywords.size();++i)
		{
			CKeywordFilter filter;
			if (!CAIKeywords::groupFilter(dsc->GroupKeywords[i], filter))
			{
				nlwarning("There are some keyword error in '%s'", getAliasNode()->fullName().c_str());
				continue;
			}
			_groupFilter+=filter;
		}
	}
	
	// the action
	_action=buildAction(dsc->Action,getAliasNode(),container);

	if (_action.isNull())
	{
		nlwarning( "In '%s': failed to find action under the event",
			getAliasFullName().c_str(),
			dsc->EventType.c_str());
	}
	
	// if all went well link the event reaction to the reaction manager
	if (!_eventMgr.isNULL())
	{
		_eventMgr->addReaction(this);
	}
	else
	{
//		nlwarning( "Failed to find event manager for events of type '%s' in Npc Mgr %u (%s)",
//			dsc->EventType.c_str(), container->getAlias(), container->getAliasNode()->fullName().c_str());
		nlwarning( "In '%s': failed to find event manager for events of type '%s'",
			getAliasFullName().c_str(),
			dsc->EventType.c_str());
	}
	
}

std::string		CStateInstance::buidStateInstanceDebugString	() const
{
	if (	!_state
		&&	!_PunctualState)
		return	std::string("NO STATE");

	CStateInstance	*const	statInstancePt=const_cast<CStateInstance*>(this);
	
	std::string	s=NLMISC::toString("STATE: %s (%s)[%s] PUNCTUAL: %s (%s)[%s]",
		(!_state)? "NULL": _state->getName().c_str(),
		statInstancePt->timerStateTimeout().toString().c_str(),
		(!_NextState)? "NULL": _NextState->getName().c_str(),
		(!_PunctualState)? "NULL": _PunctualState->getName().c_str(), statInstancePt->timerPunctTimeout().toString().c_str(),
		_CancelPunctualState? "CANCEL":	(!_NextPunctualState)?	"NULL": 	_NextPunctualState->getName().c_str()
		);
	
	if (statInstancePt->timerUser(0).isEnabled())
		s+= NLMISC::toString(" TO: %s",statInstancePt->timerUser(0).toString().c_str());
	if (statInstancePt->timerUser(1).isEnabled())
		s+= NLMISC::toString(" T1: %s",statInstancePt->timerUser(1).toString().c_str());
	if (statInstancePt->timerUser(2).isEnabled())
		s+= NLMISC::toString(" T2: %s",statInstancePt->timerUser(2).toString().c_str());
	if (statInstancePt->timerUser(3).isEnabled())
		s+= NLMISC::toString(" T3: %s",statInstancePt->timerUser(3).toString().c_str());	
	return	s;
}
