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
#include "ai_mgr_fauna.h"
#include "ai_mgr_npc.h"
#include "ai_mgr_pet.h"
#include "owners.h"

using namespace MULTI_LINE_FORMATER;

CManager::CManager(IManagerParent* parent, uint32 alias, std::string const& name, std::string const& filename) 
: CAliasChild<IManagerParent>(parent, alias, name)
, CAliasTreeRoot(filename)
{
	_CurSpawnRing = 0;
}

CManager::~CManager()
{
	release();
	_Groups.setChildSize(0);
}

CManager* CManager::createManager(AITYPES::TMgrType type, IManagerParent* managerParent, uint32 alias, std::string const& name, std::string const& mapName, std::string const& filename)
{
	CManager* manager = NULL;
	switch	(type)
	{
		case AITYPES::MgrTypeFauna:   manager = new CMgrFauna(managerParent, alias,name, filename); break;
		case AITYPES::MgrTypeKami:    manager = new CMgrNpc(managerParent, alias, name, filename);  break;
		case AITYPES::MgrTypeNpc:     manager = new CMgrNpc(managerParent, alias, name, filename);  break;
		case AITYPES::MgrTypeKaravan: manager = new CMgrNpc(managerParent, alias,name, filename);   break;
		case AITYPES::MgrTypePet:     manager = new CMgrPet(managerParent, alias,name, filename);   break;
		default:
			nlwarning("CManager::createManager(): FAILED to instantiate manager due unmanaged type: %s", AITYPES::getName(type));
			break;
	}
	return manager;
}

void CManager::update()
{
	if (_SpawnList.size()>0)
	{
		H_AUTO(MgrUpdateSpawnList)
		// deal with the spawn list
		// if there are bot groups waiting to be spawned then treat a few of them
		for	(uint count=0;!_SpawnList.empty() && count<_MaxSpawns;count++)
		{
			while (_SpawnList.size()>0)
			{
				_CurSpawnRing++;
				if (_CurSpawnRing>=_SpawnList.size())
					_CurSpawnRing=0;
				
				if (!_SpawnList[_CurSpawnRing]->isSpawned())
					break;
				
				_SpawnList.erase(_SpawnList.begin()+_CurSpawnRing);	// remove the group.
			}
			
			if (_SpawnList.size()>0)
			{
				if (_SpawnList[_CurSpawnRing]->spawn())
					_SpawnList.erase(_SpawnList.begin()+_CurSpawnRing);	// remove the group.
			}
		}
	}
	
	if (groups().size()==0)
		return;
	
	// we're going to run through the groups updating each one once every n ticks where n is
	// determined by the 'getUpdatePriority()' mask
	// note that 'getUpdatePriority()' needs to be renamed but it's late, I'm tired and I just need
	// an unused variable to work with that already exists so that I don't have to go through a complete
	// project recompile.
	uint32 timeOffset = CTimeInterface::gameCycle();
	
	{
		H_AUTO(MgrUpdateSpawned);
		int index = -1;
		FOREACH(it, CCont<CGroup>, groups())
		{
			++index;
			NLMISC::CSmartPtr<CGroup> persistent = *it; // To avoid destruction while executing ..
			if (!persistent->isSpawned())
				continue;
			
			CSpawnGroup* const spawned = persistent->getSpawnObj();
			if (spawned->getUpdatePriority()==1)
				++AISStat::GrpFastUpdCtr;
			if (spawned->getUpdatePriority()==31)
				++AISStat::GrpSlowUpdCtr;
			if (((index+timeOffset)&spawned->getUpdatePriority())!=0)
				continue;
			
			spawned->update();
		}
	}
}

CGroup*	CManager::getNextValidGroupChild(CGroup* group)
{
	return _Groups.getNextValidChild(static_cast<CGroup*>(group));
}

void CManager::serviceEvent(CServiceEvent const& info)
{
	FOREACH(it, CCont<CGroup>, groups())
		it->serviceEvent(info);
}
		
void CManager::spawn()
{
	FOREACH(it, CCont<CGroup>, groups())
		_SpawnList.push_back(*it);
	_MaxSpawns = (groups().size()+127)/(127+1);	// the 127 and 128 are arbitrary ( D M ).
}

void CManager::despawnMgr()
{
	FOREACH(it, CCont<CGroup>, groups())
		it->despawnGrp();
}

CGroup* CManager::getGroup(uint32 index)
{
	if (index>=groups().size())
		return NULL;
	return groups()[index];
}

bool CManager::isEmpty() const
{
	for(uint k = 0; k < _Groups.size(); ++k)
	{
		if (_Groups[k] && _Groups[k]->getNextValidBotChild() != NULL) return false;
	}
	return true;
}

std::string CManager::getIndexString() const
{
	return getOwner()->getManagerIndexString(this);
}

std::string	CManager::getOneLineInfoString() const
{
	return std::string("Manager '") + getName() + "'";
}

std::string CManager::getFullName() const
{
	return getName();
}

std::vector<std::string> CManager::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CManager");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " type=" + std::string(AITYPES::getName(type()));
	container.back() += " alias=" + getAliasString();
	container.back() += " name=" + getName();
	pushFooter(container);
	
	
	return container;
}

void CManager::release()
{
}

void CManager::addToSpawn(CGroup* group)
{
	for (size_t i=0; i<_SpawnList.size(); ++i)
	{
		if (_SpawnList[i].ptr()==group)
			return;
	}
	_SpawnList.push_back(group);
}

void CManager::removeFromSpawnList(CGroup const* group)
{
	for	(size_t i=0; i<_SpawnList.size(); ++i)
	{
		if (_SpawnList[i]==group)
		{
			_SpawnList.erase(_SpawnList.begin()+i);	// remove the group.
			return;
		}
	}
}

CAIInstance* CManager::getAIInstance() const
{
	return getOwner()->getAIInstance();
}
