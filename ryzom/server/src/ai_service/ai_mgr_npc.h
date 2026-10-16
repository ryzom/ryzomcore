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

#ifndef RYAI_MGR_NPC_H
#define RYAI_MGR_NPC_H

#include "ai_mgr.h"
#include "event_manager.h"
#include "states.h"
#include "ai_variables.h"
#include "event_reaction_container.h"

//////////////////////////////////////////////////////////////////////////////
// CMgrNpc                                                                  //
//////////////////////////////////////////////////////////////////////////////

/// This is the manager class for npc (and npc groups)
class CMgrNpc
: public CManager
{
public:
	CMgrNpc(IManagerParent* parent, uint32 alias, std::string const& name, std::string const& filename);
	virtual ~CMgrNpc() NL_OVERRIDE;
	
	void update() NL_OVERRIDE;
	
	CStateMachine* getStateMachine() NL_OVERRIDE { return &_StateMachine; }
	
	uint32 getAlias() const { return CManager::getAlias(); }
	CAIAliasDescriptionNode* getAliasNode() const { return CManager::getAliasNode(); }
	
	virtual std::string	getOneLineInfoString() const NL_OVERRIDE;
	virtual std::vector<std::string> getMultiLineInfoString() const NL_OVERRIDE;
	
	//////////////////////////////////////////////////////////////////////////
	
	//	Methods inherited from IManager.
	void init() NL_OVERRIDE { }
	void release() NL_OVERRIDE { }
	
	void serviceDown(uint32	sid, std::string const& name) { }
	
	AITYPES::TMgrType type() const NL_OVERRIDE { return AITYPES::MgrTypeNpc; }
	
	// instantiate the bot population
	virtual void spawn() NL_OVERRIDE;
	// clear the bot population
	virtual void despawnMgr() NL_OVERRIDE;
	
	// event managers --------------------------------------------------
	CAIEvent EventDestinationReachedFirst;
	CAIEvent EventDestinationReachedAll;
	CAIEvent EventBotKilled;
	CAIEvent EventSquadLeaderKilled;
	CAIEvent EventGrpEliminated;
	
	virtual void registerEvents();
	
	//////////////////////////////////////////////////////////////////////////	
	//	Alias Tree related Methods.
	
	IAliasCont* getAliasCont(AITYPES::TAIType type) NL_OVERRIDE;
	CAliasTreeOwner* createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree) NL_OVERRIDE;
	
	//////////////////////////////////////////////////////////////////////////
	CStateMachine _StateMachine;
};

#endif
