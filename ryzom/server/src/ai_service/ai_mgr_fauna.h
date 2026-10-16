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

#ifndef RYAI_MGR_FAUNA_H
#define RYAI_MGR_FAUNA_H

#include "ai_mgr.h"
#include "ai_place.h"
#include "event_manager.h"
#include "event_reaction_container.h"

class CFauna;

//////////////////////////////////////////////////////////////////////////////
// CMgrFauna                                                                //
//////////////////////////////////////////////////////////////////////////////

/// This is the manager class for fauna (and fauna groups)
class CMgrFauna
: public CManager
{
public:	
	CMgrFauna(IManagerParent* parent, uint32 alias, std::string const& name, std::string const& filename);
	virtual ~CMgrFauna() NL_OVERRIDE;
	
	CStateMachine* getStateMachine() NL_OVERRIDE { return &_StateMachine; }
	
	virtual std::string getOneLineInfoString() const NL_OVERRIDE;
	
	//////////////////////////////////////////////////////////////////////////
	//	CStateMachine
	
	uint32 getAlias() const { return CManager::getAlias(); }
	
	CAIAliasDescriptionNode* getAliasNode() const { return CManager::getAliasNode(); }
	
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////	
	//	Methods inherited from IManager.
	void init() NL_OVERRIDE;
	void update() NL_OVERRIDE;
	void release() NL_OVERRIDE { CManager::release(); }
	
	AITYPES::TMgrType type() const NL_OVERRIDE { return AITYPES::MgrTypeFauna; }
	// event managers --------------------------------------------------
	CAIEvent EventDestinationReachedFirst;
	CAIEvent EventDestinationReachedAll;
	CAIEvent EventBotKilled;
	CAIEvent EventGrpEliminated;
	
	CStateMachine _StateMachine;
	
	void registerEvents();
	
	//////////////////////////////////////////////////////////////////////////	
	//	Alias Tree related Methods.
	
	IAliasCont*			getAliasCont(AITYPES::TAIType type) NL_OVERRIDE;
	CAliasTreeOwner*	createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree) NL_OVERRIDE;
	
	/// @name CAIEntity public implementation
	//@{
	virtual void display(CStringWriter& stringWriter);
	//@}
	
protected:
	/// @name CAIEntity protected implementation
	//@{
	virtual std::string buildDebugString(uint idx);
	//@}
};

#endif
