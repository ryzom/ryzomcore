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

#ifndef RYAI_AI_MGR_H
#define RYAI_AI_MGR_H

#include "ai_entity.h"
#include "alias_tree_root.h"
#include "manager_parent.h"
#include "service_dependencies.h"

extern NLLIGO::CLigoConfig	LigoConfig;
class CSpawnGroup;
class CStateMachine;

//////////////////////////////////////////////////////////////////////////////
// CManager                                                                 //
//////////////////////////////////////////////////////////////////////////////

class CManager
: public CAliasChild<IManagerParent>
, public NLMISC::CRefCount
, public CAliasTreeRoot
, public CServiceEvent::CHandler
, public CAIEntity
{
public:
	CManager(IManagerParent* parent, uint32 alias, std::string const& name, std::string const& filename);
	virtual ~CManager();
	
	/// @name CChild implementation
	//@{
	virtual std::string getIndexString() const;
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	virtual std::string getFullName() const;
//	virtual std::string getName() const;
	//@}
	
	/// @name CServiceEvent::CHandler implementation
	//@{
	virtual void serviceEvent(CServiceEvent const& info);
	//@}
	
	/// @name CAIEntity public implementation
	//@{
	virtual CAIInstance* getAIInstance() const;
	//@}
	
public:
	/// @name Virtual interface
	//@{
	virtual void release();
	virtual void update();
	virtual AITYPES::TMgrType type() const = 0;
	virtual void init() = 0;
	virtual CStateMachine* getStateMachine() = 0;
	virtual void spawn();		
	virtual void despawnMgr();
	//@}
	
	/// @name Accessors
	//@{
	CAliasCont<CGroup>& groups() { return _Groups; }
	CAliasCont<CGroup> const& groups() const { return _Groups; }
	CGroup*	getGroup(uint32 index);
	
	bool isEmpty() const;

	CAliasTreeOwner* getAliasTreeOwner() { return this; }
	
	uint32 getAlias() const { return CAliasTreeOwner::getAlias(); }
	std::string getMap() const { return _map; }
	NLMISC::CEntityId getEntityId() { return _EntityId; }
	//@}
	
	CGroup* getNextValidGroupChild(CGroup* group = NULL);
	
	void addToSpawn(CGroup* group);
	void removeFromSpawnList(CGroup const* group);
	
	static CManager* createManager(AITYPES::TMgrType type, IManagerParent* managerParent, uint32 alias, std::string const& name, std::string const& mapName, std::string const& filename);
	
protected:
	/// @name AI service hierarchy
	//@{
	CAliasCont<CGroup> _Groups;
	//@}
	
	std::string _map;
	
private:
	NLMISC::CEntityId _EntityId;
	uint32 _MaxSpawns;
	std::vector<NLMISC::CDbgPtr<CGroup> > _SpawnList;
	uint32 _CurSpawnRing;
};

#endif
