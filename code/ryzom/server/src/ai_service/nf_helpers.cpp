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

#include	"stdpch.h"

#include "script_compiler.h"

#include "ai_grp_npc.h"
#include "group_profile.h"
#include "ai_generic_fight.h"
#include "server_share/msg_brick_service.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

#include "ai_script_data_manager.h"

using std::string;
using std::vector;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;

AITYPES::CPropertySet readSet(std::string strings, std::string separator = "|")
{
	AITYPES::CPropertySet properties;
	CStringSeparator const sep(strings, separator);
	while (sep.hasNext())
		properties.addProperty(AITYPES::CPropertyId::create(sep.get()));
	return properties;
}

// CStateInstance
IScriptContext* spawnNewGroup(CStateInstance* entity, CScriptStack& stack, CAIInstance* aiInstance, CAIVector const& spawnPosition, sint32 baseLevel, double dispersionRadius)
{
	string stateMachineName = stack.top();
	stack.pop();
	string dynGroupName = stack.top();
	stack.pop();
	
	if (!entity)
	{
		nlwarning("spawnNewGroup failed because entity==NULL");
		return NULL;	//	return a normal stack.
	}
	
	CGroupDesc<CGroupFamily> const* groupDesc = NULL;
	
	//	Find the group template.
	// :TODO: Replace it with a faster map access.
	FOREACH (itCont, CCont<CContinent>, aiInstance->continents())
	{
		FOREACH (itRegion, CCont<CRegion>, itCont->regions())
		{
			FOREACH (itFamily, CCont<CGroupFamily>, itRegion->groupFamilies())
			{
				FOREACH (itGroupDesc, CCont<CGroupDesc<CGroupFamily> >, itFamily->groupDescs())
				{
					if (itGroupDesc->getFullName()==dynGroupName || itGroupDesc->getName()==dynGroupName)
					{
						groupDesc = *itGroupDesc;
						goto groupFound;
					}
				}
			}
		}
	}
groupFound:
	if (groupDesc==NULL)
	{
		nlwarning("spawnNewGroup failed: No Group Template Found: '%s'",dynGroupName.c_str());
		return NULL;
	}
	
	// Find the state machine as a manager
	CManager* manager=NULL;
	FOREACH(itCont, CCont<CManager>, aiInstance->managers())
	{
		if (itCont->getFullName()==stateMachineName || itCont->getName()==stateMachineName)
		{
			manager = *itCont;
			break;
		}
	}
	if (!manager)
	{
		nlwarning("spawnNpcGroup failed : Unknown stateMachine: '%s'", stateMachineName.c_str());
		return NULL;
	}
	// Find the state machine as a npc manager
	CMgrNpc* npcManager = dynamic_cast<CMgrNpc*>(manager);
	if (!npcManager)
	{
		nlwarning("spawnNpcGroup failed : Not a npc state machine !: '%s'", stateMachineName.c_str());
		return NULL;
	}
	// Get the state machine
	CStateMachine const* stateMachine = manager->getStateMachine();
	if (stateMachine->cstStates().size()==0)
		stateMachine = NULL;
	// Save the creator state
	bool const savePlayerAttackable = groupDesc->getGDPlayerAttackable();
	bool const saveBotAttackable = groupDesc->getGDBotAttackable();
	// Set it to a correct value (:TODO: see why)
	groupDesc->setGDPlayerAttackable(true);
	groupDesc->setGDBotAttackable(true);
	// Create the group
	CGroupNpc* const grp = groupDesc->createNpcGroup(npcManager, spawnPosition, dispersionRadius, baseLevel);
	// Restore the creator state
	groupDesc->setGDPlayerAttackable(savePlayerAttackable);
	groupDesc->setGDBotAttackable(saveBotAttackable);
	// Verify that the group was created
	if (!grp)
	{
		nlwarning("spawnNpcGroup failed : group cannot spawn !: %s", stateMachineName.c_str());
		return NULL;
	}
	// Set the new group parameters
	grp->autoDestroy(true);
	grp->getPersistentStateInstance()->setParentStateInstance(entity->getPersistentStateInstance());
	grp->initDynGrp(groupDesc, NULL);
	// Verify that we have a state in the state machine
#if !FINAL_VERSION
	if (!stateMachine || stateMachine->cstStates().size()==0)
		nlwarning("no state defined for StateMachine in Manager %s", manager->getFullName().c_str());
#endif
	// Set the group in that state
	if(stateMachine)
		grp->setStartState(stateMachine->cstStates()[0]);	//	sets the first state (must exist!).
	grp->updateStateInstance();	//	directly call his first state (to retrieve associated params).
	
	return grp;
}

void getZoneWithFlags_helper(CStateInstance* entity, CScriptStack& stack, CAIInstance* const aiInstance, CZoneScorer const& scorer)
{
	// :FIXME: Copy n past from getZoneWithFlags2 begin
	// Get all the cell-zones
	vector<CCellZone*> cellZones;
	FOREACH(itCont, CCont<CContinent>, aiInstance->continents())
	{
		FOREACH(itRegion, CCont<CRegion>, itCont->regions())
		{
			FOREACH(itCellZone, CCont<CCellZone>, itRegion->cellZones())
			{
				cellZones.push_back(*itCellZone);
			}
		}
	}
	// Shuffle 'em
	std::random_shuffle(cellZones.begin(), cellZones.end());
	// While no zone found
	FOREACH(itCellZone, std::vector<CCellZone*>, cellZones)
	{
		// Get all cells
		vector<CCell*> cells;
		FOREACH(it, CCont<CCell>, (*itCellZone)->cells())
			cells.push_back(*it);
		// Shuffle 'em
		std::random_shuffle(cells.begin(), cells.end());
		// Get a zone with a good score
		CNpcZone const* spawnZone = CCellZone::lookupNpcZoneScorer(cells, scorer);
		if (spawnZone)
		{
			stack.push(spawnZone->getAliasTreeOwner().getAliasFullName());
			return;
		}
	}
	
	nlwarning("getZoneWithFlags/getNearestZoneWithFlags No Zone Found");
	stack.push(string());
	return;
}

