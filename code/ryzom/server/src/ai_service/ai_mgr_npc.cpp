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
#include "ai_mgr_npc.h"
#include "ai_grp_npc.h"
#include "states.h"
#include "game_share/base_types.h"
#include "npc_description_msg.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace RYAI_MAP_CRUNCH;
using namespace	AITYPES;

// Stuff used for management of log messages
static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else nlinfo

//////////////////////////////////////////////////////////////////////////////
// CMgrNpc                                                                  //
//////////////////////////////////////////////////////////////////////////////

// instantiate the bot population
void CMgrNpc::spawn()
{
	nlinfo("---------------  spawn Npc manager: %s ----------------",	getName().c_str()	);
	CManager::spawn();
	
	// inform the EGS of our existence - simulate connection of EGS
	if (EGSHasMirrorReady)
		serviceEvent(CServiceEvent(TServiceId(0), std::string("EGS"), CServiceEvent::SERVICE_UP));
}

// clear the bot population
void CMgrNpc::despawnMgr()
{
	nlinfo("--------------  despawn manager: %s --------------- ",	getName().c_str());
	CManager::despawnMgr();
}

IAliasCont* CMgrNpc::getAliasCont(TAIType type)
{
	switch (type)
	{
	case AITypeNoGo: // not implemented.
		return NULL;
	case AITypeGrp:
		return &_Groups;
	case AITypeNpcStateRoute:
	case AITypeNpcStateZone:
	case AITypePunctualState:
	case AITypeKamiDeposit:
	case AITypeKaravanState:
	case AITypeState:
		return &getStateMachine()->states();
	case AITypeEvent:
		return	&getStateMachine()->eventReactions();
	case AITypeFolder:
	default:
		return	NULL;
	}
}

CAliasTreeOwner* CMgrNpc::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	CAliasTreeOwner* child = NULL;
	
	switch (aliasTree->getType())
	{
	case AITypeGrp:
		child = new CGroupNpc(this, aliasTree, Nothing);
		break;
	case AITypeNpcStateRoute:
	case AITypeNpcStateZone:
	case AITypeKamiDeposit:
	case AITypeKaravanState:
	case AITypeState:
		child = new CAIStatePositional(getStateMachine(), aliasTree);
		break;
	case AITypePunctualState:
		child = new CAIStatePunctual(getStateMachine(), aliasTree);
		break;
	case AITypeEvent:
		child = new CAIEventReaction(getStateMachine(), aliasTree);
		break;		
	case AITypeNoGo:
	case AITypeFolder:
		break;
	}
	if (child)
		cont->addAliasChild(child);	
	return child;
}

std::string	CMgrNpc::getOneLineInfoString() const
{
	return std::string("NPC manager '") + getName() + "'";
}

CMgrNpc::CMgrNpc(IManagerParent* parent, uint32 alias, std::string const& name, std::string const& filename)
: CManager(parent, alias, name, filename)
{
	registerEvents();
}

CMgrNpc::~CMgrNpc()
{
	_StateMachine.clearEventContainerContent();
	_Groups.clear();
}

void CMgrNpc::update()
{
	++AISStat::MgrTotalUpdCtr;
	++AISStat::MgrNpcUpdCtr;
	CManager::update();
}

void CMgrNpc::registerEvents()
{
	_StateMachine.registerEvents();
	
	_StateMachine.addEvent(	"destination_reached",			EventDestinationReachedFirst	);
	_StateMachine.addEvent(	"destination_reached_first",	EventDestinationReachedFirst	);
	_StateMachine.addEvent(	"destination_reached_all",		EventDestinationReachedAll	);
	_StateMachine.addEvent(	"bot_killed",					EventBotKilled				);
	_StateMachine.addEvent(	"squad_leader_killed",			EventSquadLeaderKilled		);
	_StateMachine.addEvent(	"group_eliminated",				EventGrpEliminated			);
}

std::vector<std::string> CMgrNpc::getMultiLineInfoString() const
{
	using namespace MULTI_LINE_FORMATER;
	std::vector<std::string> container;
	std::vector<std::string> strings;
	
	
	pushTitle(container, "CMgrNpc");
	strings = CManager::getMultiLineInfoString();
	FOREACHC(itString, std::vector<std::string>, strings)
		pushEntry(container, *itString);
//	pushEntry(container, "state machine:");
	strings = _StateMachine.getMultiLineInfoString();
	FOREACHC(itString, std::vector<std::string>, strings)
		pushEntry(container, *itString);
	pushFooter(container);
	
	
	return container;
}

// :KLUDGE: This method should be in event_reaction_container.cpp that doesn't exist
std::vector<std::string> CStateMachine::getMultiLineInfoString() const
{
	using namespace MULTI_LINE_FORMATER;
	std::vector<std::string> container;
	
	
	pushTitle(container, "CStateMachine");
	pushEntry(container, "States:");
	FOREACHC(itState, CCont<CAIState>, _states)
		pushEntry(container, " - "+itState->getName());
	pushFooter(container);
	
	
	return container;
}
