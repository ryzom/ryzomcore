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

#include "script_compiler.h"

#include "ai_grp_npc.h"
#include "group_profile.h"
#include "ai_generic_fight.h"
#include "server_share/msg_brick_service.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

#include "ai_script_data_manager.h"

#include "nf_helpers.h"
#include "ai_aggro.h"
#include "game_share/send_chat.h"
#include "game_share/string_manager_sender.h"
#include <time.h>


using std::string;
using std::vector;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;


//----------------------------------------------------------------------------
/** @page code

@subsection spawn__
Spawns the current group.

Arguments: ->

*/
// CGroup
void spawn__(CStateInstance* entity, CScriptStack& stack)
{
	if (!entity)
	{
		nlwarning("spawnInstance failed");
		return;
	}
	CGroup const* const grp = entity->getGroup();
	if (grp)
	{
		if (grp->isSpawned())
			grp->getSpawnObj()->spawnBots();
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection despawn_f_
Depawns the current group.

Arguments: f(Immediatly) ->
@param Immediatly is whether the groups spawns immediatly (1) or not (0)

@code
()despawn(0);  // despawn
()despawn(1);  // despawn immediatement
@endcode

*/
// CGroup
void despawn_f_(CStateInstance* entity, CScriptStack& stack)
{
	float const immediatly = stack.top();
	stack.pop();
	
	if (!entity)
	{
		nlwarning("despawnInstance failed");
		return;
	}
	
	CGroup* const grp = entity->getGroup();	
	if (!grp)
	{
		nlwarning("despawn : entity '%s'%s is not a group ? ", 
			entity->aliasTreeOwner()->getAliasFullName().c_str(),
			entity->aliasTreeOwner()->getAliasString().c_str());
		return;
	}	
	grp->despawnBots(immediatly!=0);
}


//----------------------------------------------------------------------------
/** @page code

@subsection isAlived__f
Test if the group is alived

Arguments:  -> f(Immediatly)
@return 1 if group is spawned

@code
(alive)isAlived();  
@endcode

*/
// CGroup
void isAlived__f(CStateInstance* entity, CScriptStack& stack)
{
	
	if (!entity)
	{
		stack.push(0.0f);
		return;
	}
	
	CGroup* const grp = entity->getGroup();	
	if (!grp)
	{
		nlwarning("isAlived__f : entity '%s'%s is not a group ? ", 
			entity->aliasTreeOwner()->getAliasFullName().c_str(),
			entity->aliasTreeOwner()->getAliasString().c_str());				
		stack.push(0.0f);
		return;
	}	
	
	if (!grp->isSpawned())
	{
		stack.push(0.0f);
		return;
	}

	
	for (uint i=0; i<grp->bots().size(); ++i)
	{
		const	CBot *const	bot = grp->getBot(i);
		if	(	!bot
			||	!bot->isSpawned())
			continue;


		CAIEntityPhysical *const	ep=bot->getSpawnObj();
		if (ep->isAlive())
		{
			stack.push(1.0f);
			return;
		}
	}

	stack.push(0.0f);
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPos_ssfff_c
Used to create a dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y), f(Dispersion) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[in] Dispersion is the dispersion radius in meters when spawning a group
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroupPos("class_forager", "state_machine_1", x, y, 30);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPos_ssfff_c(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	float const y = stack.top();
	stack.pop();
	float const x = stack.top();
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (dispersionRadius<0.)
		dispersionRadius = 0.;
	stack.push(spawnNewGroup(entity, stack, aiInstance, CAIVector(x, y), -1, dispersionRadius));
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPos_ssfff_
Used to create a dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y), f(Dispersion) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[in] Dispersion is the dispersion radius in meters when spawning a group

@code
()newNpcChildGroupPos("class_forager", "state_machine_1", x, y, 30);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPos_ssfff_(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	float const y = stack.top();
	stack.pop();
	float const x = stack.top();
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (dispersionRadius<0.)
		dispersionRadius = 0.;
	spawnNewGroup(entity, stack, aiInstance, CAIVector(x, y), -1, dispersionRadius);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPos_ssff_c
Used to create a dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroupPos("class_forager", "state_machine_1", x, y);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPos_ssff_c(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroupPos_ssfff_c(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPos_ssff_
Used to create a dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis

@code
()newNpcChildGroupPos("class_forager", "state_machine_1", x, y);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPos_ssff_(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroupPos_ssfff_(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPosMl_ssffff_c
Used to create a multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y), f(BaseLevel), f(Dispersion) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[in] BaseLevel is the base level of the spawned group
@param[in] Dispersion is the dispersion radius in meters when spawning a group
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroupPosMl("class_forager", "state_machine_1", x, y, 13, 7.5);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPosMl_ssffff_c(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	float const y = stack.top();
	stack.pop();
	float const x = stack.top();
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (dispersionRadius<0.)
		dispersionRadius = 0.;
	stack.push(spawnNewGroup(entity, stack, aiInstance, CAIVector(x,y), -1, dispersionRadius));
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPosMl_ssffff_
Used to create a multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y), f(BaseLevel), f(Dispersion) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[in] BaseLevel is the base level of the spawned group
@param[in] Dispersion is the dispersion radius in meters when spawning a group

@code
()newNpcChildGroupPosMl("class_forager", "state_machine_1", x, y, 13, 7.5);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPosMl_ssffff_(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	float const y = stack.top();
	stack.pop();
	float const x = stack.top();
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (dispersionRadius<0.)
		dispersionRadius = 0.;
	spawnNewGroup(entity, stack, aiInstance, CAIVector(x,y), -1, dispersionRadius);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPosMl_ssfff_c
Used to create a multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y), f(BaseLevel) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[in] BaseLevel is the base level of the spawned group
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroupPosMl("class_forager", "state_machine_1", x, y, 13);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPosMl_ssfff_c(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroupPosMl_ssffff_c(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupPosMl_ssfff_
Used to create a multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), f(x), f(y), f(BaseLevel) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] x position of the spawned group on x axis
@param[in] y position of the spawned group on y axis
@param[in] BaseLevel is the base level of the spawned group

@code
()newNpcChildGroupPosMl("class_forager", "state_machine_1", x, y, 13);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupPosMl_ssfff_(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroupPosMl_ssffff_(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getMidPos__ff
Returns the position (x, y) of the current group.

Arguments: -> f(x), f(y)
@param[out] x position of the group on x axis
@param[out] y position of the group on y axis

@code
(x, y)getMidPos();
@endcode

*/
// CGroup
void getMidPos__ff(CStateInstance* entity, CScriptStack& stack)
{
	CGroup* const group = entity->getGroup();
	
	CAIVector vect;
	if (group->isSpawned())
	{
		if (!group->getSpawnObj()->calcCenterPos(vect))
			group->getSpawnObj()->calcCenterPos(vect, true);
	}
	
	float x((float)vect.x().asDouble());
	float y((float)vect.y().asDouble());
	
	stack.push(y);
	stack.push(x);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroup_sssf_c
Used to create dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(Dispersion) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[in] Dispersion is the dispersion radius in meters when spawning a group
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroup("class_forager", "state_machine_1", $zone, 10);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroup_sssf_c(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	TStringId const zoneName = CStringMapper::map(stack.top());
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CNpcZone const* spawnZone = aiInstance->getZone(zoneName);
	if (!spawnZone)
	{
		string StateMachine = stack.top();
		stack.pop();
		stack.pop();
		nlwarning("newNpcChildGroup failed : spawnZone Not Found ! for StateMachine : %s", StateMachine.c_str());
		return;
	}
	if (dispersionRadius<0.)
	{
		dispersionRadius = 0.;
		CNpcZonePlace const* spawnZonePlace = dynamic_cast<CNpcZonePlace const*>(spawnZone);
		if (spawnZonePlace!=NULL)
			dispersionRadius = spawnZonePlace->getRadius();
	}
	stack.push(spawnNewGroup(entity, stack, aiInstance, spawnZone->midPos(), -1, dispersionRadius));
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroup_sssf_
Used to create dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(Dispersion) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[in] Dispersion is the dispersion radius in meters when spawning a group

@code
()newNpcChildGroup("class_forager", "state_machine_1", $zone, 10);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroup_sssf_(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	TStringId const zoneName = CStringMapper::map(stack.top());
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CNpcZone const* spawnZone = aiInstance->getZone(zoneName);
	if (!spawnZone)
	{
		string StateMachine = stack.top();
		stack.pop();
		stack.pop();
		nlwarning("newNpcChildGroup failed : spawnZone Not Found ! for StateMachine : %s", StateMachine.c_str());
		return;
	}
	if (dispersionRadius<0.)
	{
		dispersionRadius = 0.;
		CNpcZonePlace const* spawnZonePlace = dynamic_cast<CNpcZonePlace const*>(spawnZone);
		if (spawnZonePlace!=NULL)
			dispersionRadius = spawnZonePlace->getRadius();
	}
	spawnNewGroup(entity, stack, aiInstance, spawnZone->midPos(), -1, dispersionRadius);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroup_sss_c
Used to create dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(Dispersion) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroup("class_forager", "state_machine_1", $zone);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroup_sss_c(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroup_sssf_c(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroup_sss_
Used to create dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(Dispersion) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods

@code
()newNpcChildGroup("class_forager", "state_machine_1", $zone);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroup_sss_(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroup_sssf_(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupMl_sssff_c
Used to create multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(BaseLevel), f(Dispersion) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[in] BaseLevel is the base level of the spawned group
@param[in] Dispersion is the dispersion radius in meters when spawning a group
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroupMl("class_forager", "state_machine_1", $zone, 2, 50);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupMl_sssff_c(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	int baseLevel = (int)(float)stack.top();
	stack.pop();
	TStringId const zoneName = CStringMapper::map(stack.top());
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CNpcZone const* spawnZone = aiInstance->getZone(zoneName);
	if (!spawnZone)
	{
		string StateMachine = stack.top();
		stack.pop();
		stack.pop();
		nlwarning("newNpcChildGroup failed : spawnZone Not Found ! for StateMachine : %s", StateMachine.c_str());
		return;
	}
	if (dispersionRadius<0.)
	{
		dispersionRadius = 0.;
		CNpcZonePlace const* spawnZonePlace = dynamic_cast<CNpcZonePlace const*>(spawnZone);
		if (spawnZonePlace!=NULL)
			dispersionRadius = spawnZonePlace->getRadius();
	}
	stack.push(spawnNewGroup(entity, stack, aiInstance, spawnZone->midPos(), baseLevel, dispersionRadius));
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupMl_sssff_
Used to create multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(BaseLevel), f(Dispersion) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[in] BaseLevel is the base level of the spawned group
@param[in] Dispersion is the dispersion radius in meters when spawning a group

@code
()newNpcChildGroupMl("class_forager", "state_machine_1", $zone, 2, 50);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupMl_sssff_(CStateInstance* entity, CScriptStack& stack)
{
	double dispersionRadius = (double)(float)stack.top();
	stack.pop();
	int baseLevel = (int)(float)stack.top();
	stack.pop();
	TStringId const zoneName = CStringMapper::map(stack.top());
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CNpcZone const* spawnZone = aiInstance->getZone(zoneName);
	if (!spawnZone)
	{
		string StateMachine = stack.top();
		stack.pop();
		stack.pop();
		nlwarning("newNpcChildGroup failed : spawnZone Not Found ! for StateMachine : %s", StateMachine.c_str());
		return;
	}
	if (dispersionRadius<0.)
	{
		dispersionRadius = 0.;
		CNpcZonePlace const* spawnZonePlace = dynamic_cast<CNpcZonePlace const*>(spawnZone);
		if (spawnZonePlace!=NULL)
			dispersionRadius = spawnZonePlace->getRadius();
	}
	spawnNewGroup	(entity, stack, aiInstance, spawnZone->midPos(), baseLevel, dispersionRadius);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupMl_sssf_c
Used to create multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(BaseLevel) -> c(Group)
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[in] BaseLevel is the base level of the spawned group
@param[out] Group is the newly created group

@code
(@grp)newNpcChildGroupMl("class_forager", "state_machine_1", $zone, 12);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupMl_sssf_c(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroupMl_sssff_c(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection newNpcChildGroupMl_sssf_
Used to create multilevel dynamic npc group (a parent/children relation is defined).

Arguments: s(GroupTemplate), s(StateMachine), s(Zone), f(BaseLevel) ->
@param[in] GroupTemplate is the name of a template npc group defined in the same AIInstance
@param[in] StateMachine is the name of a state machine defined in the same AIInstance
@param[in] Zone is returned by the getZoneWithFlags methods
@param[in] BaseLevel is the base level of the spawned group

@code
()newNpcChildGroupMl("class_forager", "state_machine_1", $zone, 12);
@endcode

*/
// CGroup not in a family behaviour
void newNpcChildGroupMl_sssf_(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)-1.0f);
	newNpcChildGroupMl_sssff_(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection spawnManager_s_
Spawns a manager.

Arguments: s(ManagerName) ->
@param[in] ManagerName is the name of the manager to spawn

@code
()spawnManager("NPC Manager"); // Spawns all the NPCs in the NPC manager called "NPC Manager"
@endcode

*/
// CGroup not in a family behaviour
void spawnManager_s_(CStateInstance* entity, CScriptStack& stack)
{
	string ManagerName = stack.top();
	stack.pop();

	breakable
	{
		if (!entity)
		{
			nlwarning("SpawnManager error entity not spawned");
			break;
		}
		
		IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
		CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
		if	(!aiInstance)
		{
			nlwarning("SpawnManager error AIInstance not Found");
			break;
		}
		/** :TODO: If CStringFilter is available use it here and test the whole */
		string/*CStringFilter*/ managerFilter(ManagerName);
		FOREACH(itManager, CAliasCont<CManager>, aiInstance->managers())
		{
			CManager* manager = *itManager;
			if (manager && managerFilter==manager->getName())
			{
				manager->spawn();
			}
		}
		return;
	}
	nlwarning("SpawnManager error");
}

//----------------------------------------------------------------------------
/** @page code

@subsection despawnManager_s_
Despawns a manager.

Arguments: s(ManagerName) ->
@param[in] ManagerName is the name of the manager to despawn

@code
()despawnManager("NPC Manager"); // Despawns all the NPCs in the NPC manager called "NPC Manager"
@endcode

*/
// CGroup not in a family behaviour
void despawnManager_s_(CStateInstance* entity, CScriptStack& stack)
{
	string ManagerName = stack.top();
	stack.pop();
	
	breakable
	{
		if (!entity)
		{
			nlwarning("DespawnManager error entity not spawned");
			break;
		}
		
		IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
		CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
		if (!aiInstance)
		{
			nlwarning("DespawnManager error AIInstance not Found");
			break;
		}
		// :TODO: If CStringFilter is available use it here and test the whole
		string/*CStringFilter*/ managerFilter(ManagerName);
		FOREACH(itManager, CAliasCont<CManager>, aiInstance->managers())
		{
			CManager	*manager=*itManager;
			if (manager && managerFilter==manager->getName())
			{
				manager->despawnMgr();
			}
		}
		return;
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection getGroupTemplateWithFlags_sss_s
Returns the name of a randomly chosen group template corresponding to specified flags.

Arguments: s(OneOf), s(Mandatory), s(ExceptFlags) -> s(GroupTemplate)
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[in] ExceptFlags is a '|' separated list of flags
@param[out] GroupTemplate is a group template matching at least one of OneOf flags, all Manadatory flags and none of ExceptFlags

@code
($group)getGroupTemplateWithFlags("food|rest", "invasion|outpost"); // Get a group which matches 'invasion', 'outpost' and either 'food' or 'rest' flags.
@endcode

*/
// CGroup not in a family behaviour
void getGroupTemplateWithFlags_sss_s(CStateInstance* entity, CScriptStack& stack)
{
	string exceptProperties = stack.top();
	stack.pop();
	string mandatoryProperties = stack.top();
	stack.pop();
	string oneOfProperties = stack.top();
	stack.pop();
	
	// If no AI instance return
	IManagerParent* managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
	{
		stack.push(string());
		return;
	}
	
	//	Fill the property sets.
	AITYPES::CPropertySet oneOfSet = readSet(oneOfProperties);
	AITYPES::CPropertySet mandatorySet = readSet(mandatoryProperties);
	AITYPES::CPropertySet exceptSet = readSet(exceptProperties);
	
	vector<CGroupDesc<CGroupFamily> const*>	groupDescs;
	FOREACH (itCont, CCont<CContinent>, aiInstance->continents())
	{
		FOREACH (itRegion, CCont<CRegion>, itCont->regions())
		{
			FOREACH (itFamily, CCont<CGroupFamily>, itRegion->groupFamilies())
			{
				FOREACH (itGroupDesc, CCont<CGroupDesc<CGroupFamily> >, itFamily->groupDescs())
				{
					// Skip groups not meeting the criteria
					if (!itGroupDesc->properties().containsPartOfNotStrict(oneOfSet))
						continue;
					if (!itGroupDesc->properties().containsAllOf(mandatorySet))
						continue;
					if (itGroupDesc->properties().containsPartOfStrict(exceptSet))
						continue;
					groupDescs.push_back(*itGroupDesc);
				}
			}
		}
	}
	
	if (groupDescs.size()==0)
	{
		nlwarning("getGroupTemplateWithFlags failed: no group template found that contains all of '%s' and a part of'%s' ", mandatoryProperties.c_str(), oneOfProperties.c_str());
		stack.push(string());
		return;
	}
	
	CGroupDesc<CGroupFamily> const* groupDesc = groupDescs[CAIS::rand16((uint32)groupDescs.size())];
	stack.push(groupDesc->getFullName());
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection getGroupTemplateWithFlags_ss_s
Returns the name of a randomly chosen group template corresponding to specified flags.

Arguments: s(OneOf), s(Mandatory) -> s(GroupTemplate)
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[out] GroupTemplate is a group template matching at least one of OneOf flags and all Manadatory flags

@code
($group)getGroupTemplateWithFlags("food|rest", "invasion|outpost"); // Get a group which matches 'invasion', 'outpost' and either 'food' or 'rest' flags.
@endcode

*/
// CGroup not in a family behaviour
void getGroupTemplateWithFlags_ss_s(CStateInstance* entity, CScriptStack& stack)
{
	stack.push(std::string(""));
	getGroupTemplateWithFlags_sss_s(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getZoneWithFlags_ssss_s
The first parameter is a list of 'one match enough' zone flags. The zones must
have at least one of these flags to be selected. The second parameter is a
list of 'all required' zone flag. The zones must have all these flags to be
selected. The fourth parameter is a list of 'all forbidden' zone flag. The
zones must not have any of these flags to be selected. Additionnaly the
returned zone cannot be ExceptZone. Last argument specifies a list of flags
that the selected zone cannot have.

With the list of selectable zone, the system then select a zone based on
entity density in each selected zone. The system will select the least
populated zone. It then returns the zone name.

Priority is given to current Cell, and after that to all cells.

Arguments: s(OneOf), s(Mandatory), s(ExceptZone), s(ExceptProperties) -> s(Zone)
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[in] ExceptZone is a zone name
@param[in] ExceptProperties is a '|' separated list of flags
@param[out] Zone is a zone matching at least one of OneOf flags, all Manadatory flags and can't be ExceptZone

@code
($zone)getZoneWithFlags("food|rest", "invasion|outpost", $oldzone); // Get a zone which matches invasion, outpost and either food or rest flags with the best score except $oldzone.
($zone)getZoneWithFlags("boss", "", $oldzone); // Get a zone which matches boss flag with the best score except the $oldzone
@endcode

*/
// CGroup not in a family behaviour
void getZoneWithFlags_ssss_s(CStateInstance* entity, CScriptStack& stack)
{
	string exceptProperties = stack.top();
	stack.pop();
	TStringId curZoneId= CStringMapper::map(stack.top());
	stack.pop();
	string mandatoryProperties = stack.top();
	stack.pop();
	string oneOfProperties = stack.top();
	stack.pop();
	
	// If no AI instance return
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
	{
		stack.push(string());
		return;
	}
	
	//	Fill the property sets.
	AITYPES::CPropertySet oneOfSet = readSet(oneOfProperties);
	AITYPES::CPropertySet mandatorySet = readSet(mandatoryProperties);
	AITYPES::CPropertySet exceptSet = readSet(exceptProperties);
	// Get current zone
	CNpcZone const* const curZone = aiInstance->getZone(curZoneId);
	// Create the scorer
	CZoneScorerMandatoryAndOneOfPlusExcept const scorer(oneOfSet, mandatorySet, exceptSet, curZone);

	getZoneWithFlags_helper(entity, stack, aiInstance, scorer);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getZoneWithFlags_sss_s
@sa @ref getZoneWithFlags_ssss_s

Arguments: s(OneOf), s(Mandatory), s(ExceptZone) -> s(Zone)
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[in] ExceptZone is a zone name
@param[out] Zone is a zone matching at least one of OneOf flags, all Manadatory flags and can't be ExceptZone

@code
($zone)getZoneWithFlags("food|rest", "invasion|outpost", $oldzone); // Get a zone which matches invasion, outpost and either food or rest flags with the best score except $oldzone.
($zone)getZoneWithFlags("boss", "", $oldzone); // Get a zone which matches boss flag with the best score except the $oldzone
@endcode

*/
// CGroup not in a family behaviour
void getZoneWithFlags_sss_s(CStateInstance* entity, CScriptStack& stack)
{
	stack.push(std::string(""));
	getZoneWithFlags_ssss_s(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNearestZoneWithFlags_ffsss_s
@sa @ref getZoneWithFlags_ssss_s

The zone returned by this function is the nearest from the specified point and taking into account the zone free space.

Arguments: f(X), f(Y), s(OneOf), s(Mandatory), s(ExceptProperties) -> s(Zone)
@param[in] X is the position of the zone we are looking for on the X axis
@param[in] Y is the position of the zone we are looking for on the X axis
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[in] ExceptProperties is a '|' separated list of flags
@param[out] Zone is a zone matching at least one of OneOf flags, all Manadatory flags and can't be ExceptZone

@code
($zone)getNearestZoneWithFlags(x, y, "food|rest", "invasion|outpost", "stayaway");
@endcode

*/
// CGroup not in a family behaviour
void getNearestZoneWithFlags_ffsss_s(CStateInstance* entity, CScriptStack& stack)
{
	string exceptProperties = stack.top();
	stack.pop();
	string mandatoryProperties = stack.top();
	stack.pop();
	string oneOfProperties = stack.top();
	stack.pop();
	float const y = stack.top();
	stack.pop();
	float const x = stack.top();
	stack.pop();
	
	// If no AI instance return
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
	{
		stack.push(string());
		return;
	}
	
	//	Fill the property sets.
	AITYPES::CPropertySet oneOfSet = readSet(oneOfProperties);
	AITYPES::CPropertySet mandatorySet = readSet(mandatoryProperties);
	AITYPES::CPropertySet exceptSet = readSet(exceptProperties);
	// Create the scorer
	CZoneScorerMandatoryAndOneOfAndDistAndSpace const scorer(oneOfSet, mandatorySet, exceptSet, CAIVector(x, y));
	
	getZoneWithFlags_helper(entity, stack, aiInstance, scorer);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNearestZoneWithFlags_ffss_s
@sa @ref getZoneWithFlags_sss_s

The zone returned by this function is the nearest from the specified point and taking into account the zone free space.

Arguments: f(X), f(Y), s(OneOf), s(Mandatory), s(ExceptProperties) -> s(Zone)
@param[in] X is the position of the zone we are looking for on the X axis
@param[in] Y is the position of the zone we are looking for on the X axis
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[out] Zone is a zone matching at least one of OneOf flags, all Manadatory flags and can't be ExceptZone

@code
($zone)getNearestZoneWithFlags(x, y, "food|rest", "invasion|outpost");
@endcode

*/
// CGroup not in a family behaviour
void getNearestZoneWithFlags_ffss_s(CStateInstance* entity, CScriptStack& stack)
{
	stack.push(std::string(""));
	getNearestZoneWithFlags_ffsss_s(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNearestZoneWithFlagsStrict_ffsss_s
@sa @ref getZoneWithFlags_ssss_s

The zone returned by this function is the nearest from the specified point without taking into account the zone free space.

Arguments: f(X), f(Y), s(OneOf), s(Mandatory), s(ExceptProperties) -> s(Zone)
@param[in] X is the position of the zone we are looking for on the X axis
@param[in] Y is the position of the zone we are looking for on the X axis
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[in] ExceptProperties is a '|' separated list of flags
@param[out] Zone is a zone matching at least one of OneOf flags, all Manadatory flags and can't be ExceptZone

@code
($zone)getNearestZoneWithFlagsStrict(x, y, "food|rest", "invasion|outpost", "stayaway");
@endcode

*/
// CGroup not in a family behaviour
void getNearestZoneWithFlagsStrict_ffsss_s(CStateInstance* entity, CScriptStack& stack)
{
	string exceptProperties = stack.top();
	stack.pop();
	string mandatoryProperties = stack.top();
	stack.pop();
	string oneOfProperties = stack.top();
	stack.pop();
	float const y = stack.top();
	stack.pop();
	float const x = stack.top();
	stack.pop();
	
	// If no AI instance return
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
	{
		stack.push(string());
		return;
	}
	
	//	Fill the property sets.
	AITYPES::CPropertySet oneOfSet = readSet(oneOfProperties);
	AITYPES::CPropertySet mandatorySet = readSet(mandatoryProperties);
	AITYPES::CPropertySet exceptSet = readSet(exceptProperties);
	// Create the scorer
	const	CZoneScorerMandatoryAndOneOfAndDist	scorer(oneOfSet, mandatorySet, exceptSet, CAIVector(x, y));
	
	getZoneWithFlags_helper(entity, stack, aiInstance, scorer);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNearestZoneWithFlagsStrict_ffss_s
@sa @ref getZoneWithFlags_sss_s

The zone returned by this function is the nearest from the specified point without taking into account the zone free space.

Arguments: f(X), f(Y), s(OneOf), s(Mandatory), s(ExceptProperties) -> s(Zone)
@param[in] X is the position of the zone we are looking for on the X axis
@param[in] Y is the position of the zone we are looking for on the X axis
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[out] Zone is a zone matching at least one of OneOf flags, all Manadatory flags and can't be ExceptZone

@code
($zone)getNearestZoneWithFlagsStrict(x, y, "food|rest", "invasion|outpost");
@endcode

*/
// CGroup not in a family behaviour
void getNearestZoneWithFlagsStrict_ffss_s(CStateInstance* entity, CScriptStack& stack)
{
	stack.push(std::string(""));
	getNearestZoneWithFlagsStrict_ffsss_s(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNeighbourZoneWithFlags_ssss_s
@sa @ref getZoneWithFlags_ssss_s

Here priority is given to current Cell, after that to neighbour Cells, and
finally to all cells. CurrentZone cannot be returned.

Arguments: s(CurrentZone), s(OneOf), s(Mandatory), s(ExceptProperties) -> s(Zone)
@param[in] CurrentZone is a zone name
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[in] ExceptProperties is a '|' separated list of flags
@param[out] Zone is a zone matching the specified flags

@code
($zone1)getNeighbourZoneWithFlags($zone, "boss", "", $exceptflag); // Get the zone in the neighbour cells of the one containing $zone which matches specified flags
@endcode

*/
// CGroup not in a family behaviour
void getNeighbourZoneWithFlags_ssss_s(CStateInstance* entity, CScriptStack& stack)
{
	string exceptProperties = stack.top();
	stack.pop();
	string mandatoryProperties = stack.top();
	stack.pop();
	string oneOfProperties = stack.top();
	stack.pop();
	TStringId curZoneId = CStringMapper::map(stack.top());
	stack.pop();
	
	// If no AI instance return
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if	(!aiInstance)
	{
		stack.push(string());
		return;
	}
	
	// If curzone is invalid return
	CNpcZone const* const curZone = aiInstance->getZone(curZoneId);
	if	(!curZone)
	{
		nlwarning("getNeighbourZoneWithFlags failed: current zone is invalid!");
		nlwarning(" - current zone: %s", CStringMapper::unmap(curZoneId).c_str());
		nlwarning(" - oneOfProperties:     %s", oneOfProperties.c_str());
		nlwarning(" - mandatoryProperties: %s", mandatoryProperties.c_str());
		nlwarning(" - exceptProperties:    %s", exceptProperties.c_str());
		stack.push(string());
		return;
	}
	
	//	Fill the property sets.
	AITYPES::CPropertySet oneOfSet = readSet(oneOfProperties);
	AITYPES::CPropertySet mandatorySet = readSet(mandatoryProperties);
	AITYPES::CPropertySet exceptSet = readSet(exceptProperties);
	// Create the scorer
	CZoneScorerMandatoryAndOneOfPlusExcept const scorer(oneOfSet, mandatorySet, exceptSet, curZone);
	
	vector<CCell*> cells;
	curZone->getOwner()->getNeighBourgCellList(cells);
	cells.push_back(curZone->getOwner());
	std::random_shuffle(cells.begin(), cells.end());
	
	CNpcZone const* const newZone = CCellZone::lookupNpcZoneScorer(cells, scorer);
	
	if (newZone)
	{
		stack.push(newZone->getAliasTreeOwner().getAliasFullName());
		return;
	}
	nlwarning("getNeighbourgZoneWithFlags failed: no zone found");
	nlwarning(" - current zone: %s", CStringMapper::unmap(curZoneId).c_str());
	nlwarning(" - oneOfProperties:     %s", oneOfProperties.c_str());
	nlwarning(" - mandatoryProperties: %s", mandatoryProperties.c_str());
	nlwarning(" - exceptProperties:    %s", exceptProperties.c_str());
	
	stack.push(string());
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNeighbourZoneWithFlags_sss_s
@sa @ref getZoneWithFlags_sss_s

Here priority is given to current Cell, after that to neighbour Cells, and
finally to all cells. CurrentZone cannot be returned.

Arguments: s(CurrentZone), s(OneOf), s(Mandatory) -> s(Zone)
@param[in] CurrentZone is a zone name
@param[in] OneOf is a '|' separated list of flags
@param[in] Mandatory is a '|' separated list of flags
@param[out] Zone is a zone matching the specified flags

@code
($zone1)getNeighbourZoneWithFlags($zone, "boss", ""); // Get the zone in the neighbour cells of the one containing $zone which matches specified flags
@endcode

*/
// CGroup not in a family behaviour
void getNeighbourZoneWithFlags_sss_s(CStateInstance* entity, CScriptStack& stack)
{
	stack.push(std::string(""));
	getNeighbourZoneWithFlags_ssss_s(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection setAggro_ff_
Sets aggro parameters of current group.

Arguments: f(Range), f(NbTicks) ->
@param[in] Range is a aggro range in meters
@param[in] NbTicks is a aggro update period in ticks

@code
()setAggro(Range, NbTicks); // Sets the aggro range of the group to Range, updated every NbTicks ticks
@endcode

*/
// CGroup
void setAggro_ff_(CStateInstance* entity, CScriptStack& stack)
{
	sint32 updateNbTicks = (sint32)(float)stack.top();
	stack.pop();
	float aggroRange = stack.top();
	stack.pop();
	
	CGroup* const grp = entity->getGroup();
	if (grp)
	{
		grp->_AggroRange = aggroRange;
		if (updateNbTicks>0)
			grp->_UpdateNbTicks = updateNbTicks;
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection setCanAggro_f_
Let a bot aggro or prevent it from aggroing.

Arguments: f(CanAggro) ->
@param[in] CanAggro tells whether the bot can aggro (!=0) or not (==0)

@code
()setCanAggro(0);
@endcode

*/
// CGroup
void setCanAggro_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool canAggro = ((float)stack.top())!=0.f;
	stack.pop();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		CSpawnBot* spBot = bot->getSpawnObj();
		if (spBot)
			spBot->setCanAggro(canAggro);
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection clearAggroList_f_
Reset the aggrolist of a bot.

Arguments: f(bool don't send lost aggro message to EGS) ->

@code
()clearAggroList(0/1);
@endcode

*/
// CGroup
void clearAggroList_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool sendAggroLostMessage = ((float)stack.top())==0.f;
	stack.pop();

	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		CSpawnBot* spBot = bot->getSpawnObj();
		if (spBot)
			spBot->clearAggroList(sendAggroLostMessage);
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection clearAggroList__
Reset the aggrolist of a bot.

Arguments:  ->

@code
()clearAggroList();
@endcode

*/
// CGroup
void clearAggroList__(CStateInstance* entity, CScriptStack& stack)
{
	stack.push((float)0.f);
	clearAggroList_f_(entity, stack);
}

//----------------------------------------------------------------------------
/** @page code

@subsection setMode_s_
Sets the mode of every bot of the group. Valid modes are:
- Normal
- Sit
- Eat
- Rest
- Alert
- Hungry
- Death

Arguments: s(Mode) ->
@param[in] Mode is the mode name to set

@code
()setMode("Alert");
@endcode

*/
// CGroup
void setMode_s_(CStateInstance* entity, CScriptStack& stack)
{
	string NewMode = stack.top();
	stack.pop();
	
	MBEHAV::EMode mode = MBEHAV::stringToMode(NewMode);
	if (mode==MBEHAV::UNKNOWN_MODE)
		return;
	
	FOREACH(botIt, CCont<CBot>,	entity->getGroup()->bots())
	{
		if (botIt->getSpawnObj())
			botIt->getSpawnObj()->setMode(mode);
	}
}


//----------------------------------------------------------------------------
/** @page code

@subsection setAutoSpawn_f_
Determine if the current group should respawn automatically after despawn.

Arguments: f(AutoSpawn) ->
@param[in] AutoSpawn is whether the group automatically rewpawns (1) or not (0)

@code
()setAutoSpawn(1);
@endcode

*/
// CGroup
void setAutoSpawn_f_(CStateInstance* entity, CScriptStack& stack)
{
	float const autoSpawn = stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	group->setAutoSpawn(autoSpawn!=0.f);
}

//----------------------------------------------------------------------------
// HP related methods
/** @page code

@subsection setMaxHP_ff_
Sets the Max HP level of each bot of the group.

Arguments: f(MaxHp) f(SetFull) ->
@param[in] MaxHP is the new maximum HP for each bot
@param[in] SetFull if not 0, will set the HP to the new maximum

@code
()setMaxHP(50000,1);
@endcode

*/
// CGroup
void setMaxHP_ff_(CStateInstance* entity, CScriptStack& stack)
{
	bool  setFull = ((float)stack.top() != 0.f); stack.pop();
	float maxHp = ((float)stack.top()); stack.pop();
	
	CChangeCreatureMaxHPMsg& msgList = CAIS::instance().getCreatureChangeMaxHP();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		
		if (maxHp > 0)
		{
			CSpawnBot* const sbot = bot->getSpawnObj();
			msgList.Entities.push_back(sbot->dataSetRow());
			msgList.MaxHp.push_back((uint32)(maxHp));
			msgList.SetFull.push_back((uint8)(setFull?1:0));
		}
		bot->setCustomMaxHp((uint32)maxHp);
	}
}

/** @page code

@subsection setHPLevel_f_
Sets the current HP level of each bot of the group.

Arguments: f(Coef) ->
@param[in] Coef is the percentage of its max HP each creature will have

@code
()setHPLevel(0.8);
@endcode

*/
// CGroup
void setHPLevel_f_(CStateInstance* entity, CScriptStack& stack)
{
	float coef = stack.top();
	stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		
		CSpawnBot* const sbot = bot->getSpawnObj();
		
		msgList.Entities.push_back(sbot->dataSetRow());
		msgList.DeltaHp.push_back((sint32)(sbot->maxHitPoints()*coef));
	}
}




//----------------------------------------------------------------------------
// HP related methods
/** @page code

@subsection setHPScale_f__f_
Sets the current HP level of level of each bot of the group. its maxHitPoints
eg:
for a bot HP = 850 and MaxHP = 1000
()setHPScale_f_(0);  HP will be 0 so DeltaHp = 850
()setHPScale_f_(1);  HP will be 100 so DeltaHp = 150
()setHPScale_f_(0.5);  HP will be 500 so DeltaHp = 350
if bot HP = 840 and Max   abd setHpRatio(0)  HP will be 0


Arguments: f(Coef) ->
@param[in] Coef is the percentage of its max HP each creature will *BE*

@code
()setHPLevel(0.8);
@endcode

*/
// CGroup
void setHPScale_f_(CStateInstance* entity, CScriptStack& stack)
{
	float coef = stack.top();
	stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		
		CSpawnBot* const sbot = bot->getSpawnObj();
		
		msgList.Entities.push_back(sbot->dataSetRow());
		msgList.DeltaHp.push_back((sint32)( sbot->maxHitPoints() *coef - sbot->currentHitPoints())  );
	}
}

//----------------------------------------------------------------------------
// Url related method
/** @page code

@subsection setUrl_ss_
Sets the name and url of right-click action

Arguments: s(actionName),s(url) ->
@param[in] actionName of action when player mouse over
@param[in] url of action when player mouse over

@code
()setUrl("Click on Me", "http://www.domain.com/script.php");
@endcode

*/
// CGroup
void setUrl_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string url = (std::string)stack.top();stack.pop();	
	std::string actionName = (std::string)stack.top();stack.pop();	
	
	CCreatureSetUrlMsg msg;
	FOREACH(botIt, CCont<CBot>,	entity->getGroup()->bots())
	{
		CSpawnBot* pbot = botIt->getSpawnObj();
		if (pbot!=NULL)
		{		
			msg.Entities.push_back(pbot->dataSetRow());
		}
	}
	
	msg.ActionName = actionName;
	msg.Url = url;
	msg.send(egsString);
}



/** @page code

@subsection scaleHP_f_
Scales the bots HP.

Arguments: f(Coef) ->
@param[in] Coef is the percentage of its current HP each creature will have

@code
()scaleHP(2);
@endcode

*/
// CGroup
void scaleHP_f_(CStateInstance* entity, CScriptStack& stack)
{
	float coef = stack.top();
	stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		
		CSpawnBot* const sbot = bot->getSpawnObj();
		
		msgList.Entities.push_back(sbot->dataSetRow());
		msgList.DeltaHp.push_back((sint32)(sbot->currentHitPoints()*coef));
	}
}


/** @page code

@subsection setBotHPScaleByAlias_fs_
Same as setHpSacale but only on a specific bot of a groupe from the current group by its bot alias

Arguments:  f(alias),f(Coef), ->
@param[in] alias is the alias of the bot
@param[in] Coef is the percentage of its current HP each creature will have


@code
()scaleHpByAlias(2, '(A:1000:10560)');
@endcode

*/
// CGroup
void setBotHPScaleByAlias_fs_(CStateInstance* entity, CScriptStack& stack)
{
	uint32 alias =  LigoConfig.aliasFromString((string)stack.top()) ; stack.pop();
		
	float coef = stack.top(); stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		
		if (bot->getAlias() != alias) { continue; }
		if (!bot->isSpawned()) return;			
		
		CSpawnBot* const sbot = bot->getSpawnObj();
		
		msgList.Entities.push_back(sbot->dataSetRow());
		msgList.DeltaHp.push_back((sint32)( sbot->maxHitPoints() *coef - sbot->currentHitPoints())  );
	}
}


/** @page code

@subsection downScaleHP_f_	
Scales the bots HP down.

Arguments: f(Coef) ->
@param[in] Coef is a value

@code
()downScaleHP(2);
@endcode

*/
// CGroup
void downScaleHP_f_(CStateInstance* entity, CScriptStack& stack)
{
	float coef = stack.top();
	stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	clamp(coef, 0.f, 1.f);
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		
		CSpawnBot* const sbot = bot->getSpawnObj();
		
		msgList.Entities.push_back(sbot->dataSetRow());
		msgList.DeltaHp.push_back((sint32)(sbot->currentHitPoints()*(coef-1)));
	}
}

/** @page code

@subsection upScaleHP_f_
Scales the bots HP up.

Arguments: f(Coef) ->
@param[in] Coef is a value

@code
()upScaleHP(2);
@endcode

*/
// CGroup
void upScaleHP_f_(CStateInstance* entity, CScriptStack& stack)
{
	float coef = stack.top();
	stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	clamp(coef, 0.f, 1.f);
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		CSpawnBot* spBot = bot->getSpawnObj();
		if (spBot)
		{
			msgList.Entities.push_back(spBot->dataSetRow());
			msgList.DeltaHp.push_back((sint32)((spBot->maxHitPoints()-spBot->currentHitPoints())*coef));
		}
	}
}

/** @page code

@subsection addHP_f_
Add HP to the bots.

Arguments: f(HP) ->
@param[in] HP is the amount of hit points to add to each bot

@code
()addHP(500);
@endcode

*/
// CGroup
void addHP_f_(CStateInstance* entity, CScriptStack& stack)
{
	float addHP = stack.top();
	stack.pop();
	
	CChangeCreatureHPMsg& msgList = CAIS::instance().getCreatureChangeHP();
	
	FOREACH(bot, CCont<CBot>, entity->getGroup()->bots())
	{
		if (!bot->isSpawned())
			continue;
		
		CSpawnBot* const sbot = bot->getSpawnObj();
		
		msgList.Entities.push_back(sbot->dataSetRow());
		msgList.DeltaHp.push_back((sint32)(sbot->currentHitPoints()+addHP));
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection aiAction_s_
Triggers an AI action, defined by its sheet name, on the current target.

Arguments: s(actionSheetId) ->
@param[in] actionSheetId is a the sheet name of the action

@code
()aiAction("kick_his_ass.aiaction");
@endcode

*/
// CGroup
void aiAction_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string actionName = stack.top();
	stack.pop();
	NLMISC::CSheetId sheetId(actionName);
	if (sheetId==NLMISC::CSheetId::Unknown)
	{
		nlwarning("Action SheetId Unknown %s", actionName.c_str());
		return;
	}
	AISHEETS::IAIActionCPtr action = AISHEETS::CSheets::getInstance()->lookupAction(sheetId);
	if (action.isNull())
	{
		nlwarning("Action SheetId Unknown %s", actionName.c_str());
		return;
	}
	
	FOREACH(botIt, CCont<CBot>,	entity->getGroup()->bots())
	{
		CSpawnBot* pbot = botIt->getSpawnObj();
		if (pbot!=NULL)
		{
			CSpawnBot& bot = *pbot;
			if (!bot.getAIProfile())
				continue; // OK
			
			CBotProfileFight* profile = dynamic_cast<CBotProfileFight*>(bot.getAIProfile());
			if (!profile)
				continue; // OK
			
			if (!profile->atAttackDist())
				continue; // NOT OK
			
			TDataSetRow	dataSetRow;
			if ((CAIEntityPhysical*)bot.getTarget())
				dataSetRow = bot.getTarget()->dataSetRow();
			
			CEGSExecuteAiActionMsg msg(bot.dataSetRow(), dataSetRow, action->SheetId(), bot._DamageCoef, bot._DamageSpeedCoef);
			msg.send(egsString);
			bot.setActionFlags(RYZOMACTIONFLAGS::Attacks);
			// OK
		}
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection aiActionSelf_s_
Triggers an AI action, defined by its sheet name, on the bot itself.

Arguments: s(actionSheetId) ->
@param[in] actionSheetId is a the sheet name of the action

@code
()aiActionSelf("defensive_aura.aiaction");
@endcode

*/
// CGroup
void aiActionSelf_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string actionName = stack.top();
	stack.pop();
	NLMISC::CSheetId	sheetId(actionName);
	if (sheetId==NLMISC::CSheetId::Unknown)
	{
		nlwarning("Action SheetId Unknown %s", actionName.c_str());
		return;
	}
	AISHEETS::IAIActionCPtr action = AISHEETS::CSheets::getInstance()->lookupAction(sheetId);
	if (action.isNull())
	{
		nlwarning("Action SheetId Unknown %s", actionName.c_str());
		return;
	}
	
	FOREACH(botIt, CCont<CBot>,	entity->getGroup()->bots())
	{
		CSpawnBot* pbot = botIt->getSpawnObj();
		if (pbot!=NULL)
		{
			CSpawnBot& bot = *pbot;
			CEGSExecuteAiActionMsg	msg(bot.dataSetRow(), bot.dataSetRow(), action->SheetId(), bot._DamageCoef, bot._DamageSpeedCoef);
			msg.send(egsString);
			bot.setActionFlags(RYZOMACTIONFLAGS::Attacks);
			// OK
		}
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection addProfileParameter_s_
Adds a profile parameter to the current group.

Arguments: s(parameterName) ->
@param[in] parameterName is a the id of the parameter to add

@code
()addProfileParameter("running"); // equivalent to "running" parameter in group primitive
@endcode

*/
// CGroup
void addProfileParameter_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
		group->getSpawnObj()->addProfileParameter(name, "", 0.f);
}


//----------------------------------------------------------------------------
/** @page code

@subsection addProfileParameter_ss_
Adds a profile parameter to the current group.

Arguments: s(parameterName),s(parameterContent) ->
@param[in] parameterName is a the id of the parameter to add
@param[in] parameterContent is the value of the parameter

@code
()addProfileParameter("foo", "bar"); // equivalent to "foo:bar" parameter in group primitive
@endcode

*/
// CGroup
void addProfileParameter_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
		group->getSpawnObj()->addProfileParameter(name, value, 0.f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addProfileParameter_sf_
Adds a profile parameter to the current group.

Arguments: s(parameterName),f(parameterContent) ->
@param[in] parameterName is a the id of the parameter to add
@param[in] parameterContent is the value of the parameter

@code
()addProfileParameter("foo", 0.5); // equivalent to "foo:0.5" parameter in group primitive
@endcode

*/
// CGroup
void addProfileParameter_sf_(CStateInstance* entity, CScriptStack& stack)
{
	float value = (float)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
		group->getSpawnObj()->addProfileParameter(name, "", value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection removeProfileParameter_s_
removes a profile parameter from the current group.

Arguments: s(parameterName) ->
@param[in] parameterName is a the id of the parameter to remove

@code
()removeProfileParameter("running"); // remove "running" or "running:<*>" parameter from group
@endcode

*/
// CGroup
void removeProfileParameter_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
		group->getSpawnObj()->removeProfileParameter(name);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addPersistentProfileParameter_s_
Adds a profile parameter to the current group.

Arguments: s(parameterName) ->
@param[in] parameterName is a the id of the parameter to add

@code
()addProfileParameter("running"); // equivalent to "running" parameter in group primitive
@endcode

*/
// CGroup
void addPersistentProfileParameter_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();

	if (group->isSpawned())
		group->getSpawnObj()->addProfileParameter(name, "", 0.f);

	group->addProfileParameter(name, "", 0.f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addPersistentProfileParameter_ss_
Adds a profile parameter to the current group.

Arguments: s(parameterName),s(parameterContent) ->
@param[in] parameterName is a the id of the parameter to add
@param[in] parameterContent is the value of the parameter

@code
()addPersistentProfileParameter("foo", "bar"); // equivalent to "foo:bar" parameter in group primitive
@endcode

*/
// CGroup
void addPersistentProfileParameter_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();	

	if (group->isSpawned())
		group->getSpawnObj()->addProfileParameter(name, value, 0.f);

	group->addProfileParameter(name, value, 0.f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addPersistentProfileParameter_sf_
Adds a profile parameter to the current group.

Arguments: s(parameterName),f(parameterContent) ->
@param[in] parameterName is a the id of the parameter to add
@param[in] parameterContent is the value of the parameter

@code
()addPersistentProfileParameter("foo", 0.5); // equivalent to "foo:0.5" parameter in group primitive
@endcode

*/
// CGroup
void addPersistentProfileParameter_sf_(CStateInstance* entity, CScriptStack& stack)
{
	float value = (float)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	if (group->isSpawned())
		group->getSpawnObj()->addProfileParameter(name, "", value);

	group->addProfileParameter(name, "", value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection removePersistentProfileParameter_s_
removes a profile parameter from the current group.

Arguments: s(parameterName) ->
@param[in] parameterName is a the id of the parameter to remove

@code
()removeProfileParameter("running"); // remove "running" or "running:<*>" parameters from group
@endcode

*/
// CGroup
void removePersistentProfileParameter_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	if (group->isSpawned())
		group->getSpawnObj()->removeProfileParameter(name);

	group->removeProfileParameter(name);
}
//----------------------------------------------------------------------------
/** @page code

@subsection getOutpostState__s
Returns the name of the current outpost state (group must be in an outpost).

Arguments: -> s(StateName)
@param[out] StateName is the name of the current outpost state

@code
($name)getOutpostState();
@endcode

*/
// CGroup
void getOutpostStateName__s(CStateInstance* si, CScriptStack& stack)
{
	string str;
	
	breakable
	{
		if (!si)
			break;
		
		CGroup* group = si->getGroup();
		if (!group)
			break;
		
		COutpostManager* manager = dynamic_cast<COutpostManager*>(group->getOwner());
		if (!manager)
			break;
		
		str = static_cast<COutpost*>(manager->getOwner())->getStateName();
	}
	
	stack.push(str);
}

//----------------------------------------------------------------------------
/** @page code

@subsection isOutpostTribeOwner__f
Returns whether the current outpost owner is a tribe (group must be in an outpost).

Arguments: -> f(TribeOwner)
@param[out] TribeOwner is whether the current outpost owner is a tribe (1) or not (0)

@code
(tribeOwner)isOutpostTribeOwner();
@endcode

*/
// CGroup
void isOutpostTribeOwner__f(CStateInstance* si, CScriptStack& stack)
{
	float tribeOwner = 0.f;
	
	breakable
	{
		if (!si)
			break;
		
		CGroup* group = si->getGroup();
		if (!group)
			break;

		COutpostManager* manager = dynamic_cast<COutpostManager*>(group->getOwner());
		if (!manager)
			break;
		
		if (!static_cast<COutpost*>(manager->getOwner())->isBelongingToAGuild())
			tribeOwner = 1.f;
	}
	
	stack.push(tribeOwner);
}

//----------------------------------------------------------------------------
/** @page code

@subsection isOutpostGuildOwner__f
Returns whether the current outpost owner is a guild (group must be in an outpost).

Arguments: -> f(TribeOwner)
@param[out] TribeOwner is whether the current outpost owner is a guild (1) or not (0)

@code
(guildOwner)isOutpostGuildOwner();
@endcode

*/
// CGroup
void isOutpostGuildOwner__f(CStateInstance* si, CScriptStack& stack)
{
	float guildOwner = 0.f;
	
	breakable
	{
		if (!si)
			break;
		
		CGroup* group = si->getGroup();
		if (!group)
			break;

		COutpostManager* manager = dynamic_cast<COutpostManager*>(group->getOwner());
		if (!manager)
			break;
		
		if (static_cast<COutpost*>(manager->getOwner())->isBelongingToAGuild())
			guildOwner = 1.f;
	}
	
	stack.push(guildOwner);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getEventParam_f_f
Returns the content of a param

Arguments: f(paramIndex) -> f(value)
@param[in] paramIndex is the parameter index passed by EGS ai_event message
@param[out] value is a the value of the parameter

@code
(val)getEventParam(2);
@endcode

*/
// CGroup
void getEventParam_f_f(CStateInstance* entity, CScriptStack& stack)
{
	uint32 varId = (uint32)((float)stack.top());

	CGroup* group = entity->getGroup();

	stack.top() = group->getEventParamFloat(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getEventParam_f_s
Returns the content of a param

Arguments: f(paramIndex) -> s(value)
@param[in] paramIndex is the parameter index passed by EGS ai_event message
@param[out] value is a the value of the parameter

@code
($val)getEventParam(2);
@endcode

*/
// CGroup
void getEventParam_f_s(CStateInstance* entity, CScriptStack& stack)
{
	uint32 varId = (uint32)((float)stack.top());

	CGroup* group = entity->getGroup();

	stack.top() = group->getEventParamString(varId);
}



// -- Boss functions

//----------------------------------------------------------------------------
/** @page code

@subsection getPlayerStat_ss_f
Get some player stat.

A player EntityId is used to identify the player. This EntityId is passed as string as argument. The EntityId can be obtains via getCurrentPlayerAggroListTarget or getRandomPlayerAggroListTarget.
The player must be in the same AI Instance (same continent).
If the player is not in the same Ai Instance or the input string is empty the function return zero and *display* a warning message on the log. You can think of using isPlayerAlived to be sure that the id is still a valid value.
If param is not one of "HP", "MaxHp", "RatioHp" zero is return and a warning message is printed on the log.
- The "Hp" stat is the property CURRENT_HIT_POINTS as seen in the mirror.
- The "MaxHp" stat is the property MAX_HIT_POINTS as seen in the mirror.
- The "RatioHp" stat is (Hp * 100) / MaxHp
Be careful the argument is case sensitive.

Arguments: s(playerEidAsString), s(statName) -> s(result)
@param[in] playerEidAsString is EntityId as string from the player we want infos
@param[in] statName is the name of the property (can be "HP", "MaxHp", "RatioHp")
@param[out] value is a the value of the parameter

@code
($playerEid)getCurrentPlayerEid();
print($playerEid); //log (0x00001fbd50:00:00:81)
(maxHp)getPlayerStat($playerEid, "MaxHp");
@endcode
*/
void getPlayerStat_ss_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getPlayerStat_ss_f";
	// reaed input
	std::string statName = ((std::string)stack.top()); stack.pop();
	std::string playerEidStr = ((std::string)stack.top()); stack.pop();


	// get Dataset of the player to have access to mirror values
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());	
	TDataSetRow playerRow = TheDataset.getDataSetRow( playerEid );
	if (! TheDataset.isAccessible( playerRow  ) )
	{
		nlwarning("Try to call %s with on a player '%s' that is not accessible. The isPlayerAlived function must be called to be sure that the palyer is still alived.", funName.c_str(), playerEidStr.c_str() );
		stack.push((float)0);
		return;
	}

	if (statName == "Hp" )
	{	
		// return DSPropertyCURRENT_HIT_POINTS Mirror value
		CMirrorPropValue<sint32> mirrorSymbol( TheDataset, playerRow, DSPropertyCURRENT_HIT_POINTS );
		stack.push((float)mirrorSymbol.getValue());
		return;
	}
	else if (statName == "MaxHp")
	{
		// return DSPropertyMAX_HIT_POINTS Mirror value
		CMirrorPropValue<sint32> mirrorSymbol( TheDataset, playerRow, DSPropertyMAX_HIT_POINTS );
		stack.push((float)mirrorSymbol.getValue());
		return;
	}
	else if (statName == "RatioHp")
	{
		// return percentage of live (read from mirror values)
		CMirrorPropValue<sint32> mirrorSymbol( TheDataset, playerRow, DSPropertyCURRENT_HIT_POINTS );
		CMirrorPropValue<sint32> mirrorSymbol2( TheDataset, playerRow, DSPropertyMAX_HIT_POINTS );

		stack.push((float)(100.0*mirrorSymbol.getValue() / mirrorSymbol2.getValue()));
		return;
	}
	else
	{
		nlwarning("Try to call %s with wrong state %s", funName.c_str(), statName.c_str() );
	}
	stack.push((float)0);		
	return;
		
}

/** @page code

@subsection getPlayerDistance_fs_f
Get the distance between a player and a bot in meters. 

A player EntityId is used to identify the player. This EntityId is passed as string as argument. The EntityId can be obtains via getCurrentPlayerAggroListTarget or getRandomPlayerAggroListTarget.
The player must be in the same AI Instance (same continent).
If the player is not in the same Ai Instance or the input string is empty the function return -1 and display a warning message on the log. You can think of using isPlayerAlived to be sure that the player id is still a valid value.

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index the function returns -1 and display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.

Arguments: s(playerEidAsString), f(botIndex) -> s(result)
@param[in] playerEidAsString is EntityId as string from the player we want infos
@param[in] botIndex is the index of an bot member of the current group
@param[out] value is a the distance between the player on the bot (or -1 if error)

@code
($playerEid)getCurrentPlayerEid();
(index)getBotIndexByName("toto");
(distance)getPlayerDistance(index, $playerEid);
@endcode
*/
void getPlayerDistance_fs_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getPlayerDistance_is_f";

	// read input params
	std::string playerEidStr = ((std::string)stack.top()); stack.pop();
	sint32 botIndex = (sint32)((float)stack.top()); stack.pop();

	// get the player Eid
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());

	// get the Spawn bot by its index
	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());
		stack.push((float) -1);
		return;
	}	
	if (!group->isSpawned() ||  botIndex <0 ||  group->bots().size() <= static_cast<uint32>(botIndex))
	{
		nlwarning("%s used bad index %d/%d (or group not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push((float)-1);
		return;
	}	
	CBot* bot  = group->getBot(botIndex);
	CSpawnBot* spBot = bot?bot->getSpawnObj():0;
	if (!spBot)
	{
		nlwarning("%s used bad index %d/%d (or bot not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push((float)-1);
		return;
	}

	// get DataSetRow of player to read mirro values
	TDataSetRow playerRow = TheDataset.getDataSetRow( playerEid );
	if (! TheDataset.isAccessible( playerRow ) )
	{
		nlwarning("Try to call %s with on a player '%s' that is not accessible. The isPlayerAlived function must be called to be sure that the player is still alived.", funName.c_str(), playerEidStr.c_str() );
		stack.push((float)-1);
		return;
	}

	// calculate the size between position of the player (from mirro) to position of the spawn bot.
	CMirrorPropValue<sint32> mirrorSymbol( TheDataset, playerRow, DSPropertyPOSX );
	CMirrorPropValue<sint32> mirrorSymbol2( TheDataset, playerRow, DSPropertyPOSY );
	const double dx = double (mirrorSymbol.getValue()) - double(spBot->x().asInt());
	const double dy = double (mirrorSymbol2.getValue()) - double(spBot->y().asInt());
	const double dist2 = (dx*dx + dy*dy) / 1000000.0;
	const double dist = sqrt(dist2);

	stack.push(float(dist));
	
	return;
		
}



/** @page code

@subsection getCurrentPlayerAggroListTarget_f_s
Get the player entity id (as string) that has the most aggro in the aggro list of a bot.

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index (or bot not spawaned) the function returns an empty string and display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.

A player EntityId is used to identify the player. This EntityId is returned as result. If the aggro list is empty an empty string is returned (but *NO* warning messages)


Arguments: f(botIndex) -> s(playerEidAsString)
@param[in] botIndex is the index of an bot member of the current group
@param[out] playerEidAsString is EntityId as string from the player we want infos

@code
($playerId)getCurrentPlayerAggroListTarget(4); 
(distance)getPlayerDistance(4, $playerId); 
@endcode

*/

void getCurrentPlayerAggroListTarget_f_s(CStateInstance* entity, CScriptStack& stack)
{
	
	std::string funName = "getCurrentPlayerAggroListTarget_f_s";

	// get input params
	sint32 botIndex = (sint32)((float)stack.top()); stack.pop();

	// get spawn Bot
	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());
		stack.push(std::string(""));
		return;
	}	
	if (!group->isSpawned() ||  botIndex <0 ||  group->bots().size() <= static_cast<uint32>(botIndex))
	{
		nlwarning("%s used bad index %d/%d (or group not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(std::string(""));

		return;
	}
	CBot* bot  = group->getBot(botIndex);
	CSpawnBot* spBot = bot?bot->getSpawnObj():0;
	if (!spBot)
	{
		nlwarning("%s used bad index %d/%d (or bot not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(std::string(""));
		return;
	}

	// iteratre throug aggro list to have the eid with max aggro
	CBotAggroOwner::TBotAggroList const& aggroList = spBot->getBotAggroList();	
	CBotAggroOwner::TBotAggroList::const_iterator aggroIt(aggroList.begin()), aggroEnd(aggroList.end());
	TDataSetRow foundRow = TDataSetRow();
	float foundAggro = 0;
	for (; aggroIt != aggroEnd; ++aggroIt)
	{		
		float aggro = aggroIt->second->finalAggro();
		if (aggro > foundAggro)
		{
			if (CMirrors::getEntityId(aggroIt->first).getType() != RYZOMID::player)
			{
				continue;
			}
			CAIEntityPhysical* ep = CAIS::instance().getEntityPhysical(aggroIt->first);
			if (!ep)
			{
				continue;
			}
			
			CBotPlayer const* const player = NLMISC::safe_cast<CBotPlayer const*>(ep);
			if (!player)
			{
				continue;
			}
			if (!player->isAggroable())
			{
				continue;
			}
			foundAggro = aggro;
			foundRow = aggroIt->first;
		}
	}

	if ( foundRow == TDataSetRow())
	{	// Empty aggro list so no warning displayed
		stack.push(std::string(""));
		return;
	}
	
	const NLMISC::CEntityId& found = CMirrors::getEntityId(foundRow);
	stack.push(std::string(found.toString()));
}




/** @page code

@subsection getRandomPlayerAggroListTarget_f_s
Get a player entity id (as string) from the aggro list of a bot.

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index (or bot not spawaned) the function returns an empty string and display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.

A player EntityId is used to identify the player. This EntityId is returned as result. If the aggro list is empty an empty string is returned (but *NO* warning messages)


Arguments: f(botIndex) -> s(playerEidAsString)
@param[in] botIndex is the index of an bot member of the current group
@param[out] playerEidAsString is EntityId as string from the player we want infos

@code
($playerId)getRandomPlayerAggroListTarget(4); 
()setAggroListTarget(4, $playerId); 
@endcode
*/


void getRandomPlayerAggroListTarget_f_s(CStateInstance* entity, CScriptStack& stack)
{
	
	std::string funName = "getRandomPlayerAggroListTarget_f_s";

	// test botIndex
	uint32 botIndex = (uint32)((float)stack.top()); stack.pop();

	// get Spawn bot
	CGroup* group = entity->getGroup();
	if (!group || !group->isSpawned() ||  botIndex <0 ||  group->bots().size() <= static_cast<uint32>(botIndex))
	{
		nlwarning("%s used bad index %d/%d (or group not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(std::string(""));
		return;
	}
	CBot* bot  = group->getBot(botIndex);
	CSpawnBot* spBot = bot?bot->getSpawnObj():0;
	if (!spBot)
	{
		nlwarning("%s used bad index %d/%d (or bot not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(std::string(""));
		return;
	}
	
	// make a vector of aggroable player
	CBotAggroOwner::TBotAggroList const& aggroList = spBot->getBotAggroList();
	//typedef std::map<TDataSetRow, TAggroEntryPtr> TBotAggroList;
	CBotAggroOwner::TBotAggroList::const_iterator aggroIt(aggroList.begin()), aggroEnd(aggroList.end());
	TDataSetRow foundRow = TDataSetRow();
	std::vector<TDataSetRow> playerAggroable;
	for (; aggroIt != aggroEnd; ++aggroIt)
	{		
		if (CMirrors::getEntityId(aggroIt->first).getType() != RYZOMID::player)
		{
			continue;
		}
		CAIEntityPhysical* ep = CAIS::instance().getEntityPhysical(aggroIt->first);
		if (!ep)
		{
			continue;
		}
		
		CBotPlayer const* const player = NLMISC::safe_cast<CBotPlayer const*>(ep);
		if (!player)
		{
			continue;
		}
		if (!player->isAggroable())
		{
			continue;
		}					
		playerAggroable.push_back(aggroIt->first);

	}


	if ( playerAggroable.empty())
	{
		stack.push(std::string(""));
		return;
	}
	// chose a randomly a player among the container
	uint32 index = static_cast<uint32>( static_cast<double>(playerAggroable.size())*rand()/(RAND_MAX+1.0) );
	
	const NLMISC::CEntityId& found = CMirrors::getEntityId(playerAggroable[ index ]);
	stack.push(std::string(found.toString()));
}


/** @page code

@subsection getAggroListElement_ff_s
Get a player entity id (as string) from the aggro list of a bot by it aggro list index.

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index (or bot not spawaned) the function returns an empty string and display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.

A aggro list index is used to identified the player in the aggro list. The size of the aggro list is given by getAggroListSize_f_s
)


Arguments: f(botIndex), f(aggroListIndex) -> s(playerEidAsString)
@param[in] botIndex is the index of an bot member of the current group
@param[in] aggroListIndex is the index of a aggroable player in the aggro list of the bot identified by botIndex
@param[out] playerEidAsString is EntityId as string from the player we want infos

@code
(aggroSize)getAggorListSize(0);
// to much player are attacking the boss
if (aggoroSize > 10)
{
($player1)getAggroListElement(0);
($player2)getAggroListElement(1);
($player3)getAggroListElement(2);
// so the boss teleport 3 person from its aggro list at the end of the world
teleportPlayer($player1, 14233, 123123, 0, 0);
teleportPlayer($player2, 14233, 123123, 0, 0);
teleportPlayer($player2, 14233, 123123, 0, 0);
clearAggroList();
}

@endcode
*/
void getAggroListElement_ff_s(CStateInstance* entity, CScriptStack& stack)
{
	
	std::string funName = "getAggroListElement_ff_s";



	// get params
	uint32 elementIndex = (uint32)((float)stack.top()); stack.pop();
	sint32 botIndex = (uint32)((float)stack.top()); stack.pop();
	
	// get spawn bot by its index
	CGroup* group = entity->getGroup();
	if (!group || !group->isSpawned() ||  botIndex <0 ||  group->bots().size() <= static_cast<uint32>(botIndex))
	{
		nlwarning("%s used bad index %d/%d (or group not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(std::string(""));
		return;
	}
	CBot* bot  = group->getBot(botIndex);
	CSpawnBot* spBot = bot?bot->getSpawnObj():0;
	if (!spBot)
	{
		nlwarning("%s used bad index %d/%d (or bot not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(std::string(""));
		return;
	}
	
	// make a vector of aggroable player
	CBotAggroOwner::TBotAggroList const& aggroList = spBot->getBotAggroList();
	//typedef std::map<TDataSetRow, TAggroEntryPtr> TBotAggroList;
	CBotAggroOwner::TBotAggroList::const_iterator aggroIt(aggroList.begin()), aggroEnd(aggroList.end());
	TDataSetRow foundRow = TDataSetRow();

	std::vector<TDataSetRow> playerAggroable;
	for (; aggroIt != aggroEnd; ++aggroIt)
	{		
		if (CMirrors::getEntityId(aggroIt->first).getType() != RYZOMID::player)
		{
			continue;
		}
		CAIEntityPhysical* ep = CAIS::instance().getEntityPhysical(aggroIt->first);
		if (!ep)
		{
			continue;
		}
		
		CBotPlayer const* const player = NLMISC::safe_cast<CBotPlayer const*>(ep);
		if (!player)
		{
			continue;
		}
		if (!player->isAggroable())
		{
			continue;
		}					
		playerAggroable.push_back(aggroIt->first);

	}


	if ( playerAggroable.empty())
	{
		stack.push(std::string(""));
		return;
	}
	// if index outside return "";
	if (0 > elementIndex && elementIndex >= playerAggroable.size())
	{
		stack.push(std::string(""));
		return;
	}
	
	const NLMISC::CEntityId& found = CMirrors::getEntityId(playerAggroable[ uint32(elementIndex) ]);
	stack.push(std::string(found.toString()));
}


/** @page code

@subsection getAggroListSize_f_f
Get a size of the aggro lsit of a bot (list of aggroable PLAYER)

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index (or bot not spawaned) the function returns an empty string and display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.

A number is used to indicate the size of the aggro lsit

Arguments:  f(aggroListIndex) -> f(sizeOfAggroList)
@param[in] botIndex is the index of an bot member of the current group.
@param[out] sizeOfAggroList The size of the aggro list index.

@code
(aggroSize)getAggorListSize(0);
// to much player are attacking the boss
if (aggoroSize > 10)
{
($player1)getAggroListElement(0);
($player2)getAggroListElement(1);
($player3)getAggroListElement(2);
// so the boss teleport 3 person from its aggro list at the end of the world
teleportPlayer($player1, 14233, 123123, 0, 0);
teleportPlayer($player2, 14233, 123123, 0, 0);
teleportPlayer($player2, 14233, 123123, 0, 0);
clearAggroList();
}

@endcode
*/
void getAggroListSize_f_f(CStateInstance* entity, CScriptStack& stack)
{
	
	std::string funName = "getRandomPlayerAggroListTarget_f_s";



	// input params
	uint32 botIndex = (uint32)((float)stack.top()); stack.pop();

	// get spawn bot by index of the bot
	CGroup* group = entity->getGroup();
	if (!group || !group->isSpawned() ||  botIndex <0 ||  group->bots().size() <= static_cast<uint32>(botIndex))
	{
		nlwarning("%s used bad index %d/%d (or group not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(float(0) );
		return;
	}
	CBot* bot  = group->getBot(botIndex);
	CSpawnBot* spBot = bot?bot->getSpawnObj():0;
	if (!spBot)
	{
		nlwarning("%s used bad index %d/%d (or bot not spawn)", funName.c_str(), botIndex, group->bots().size());
		stack.push(float(0) );
		return;
	}
	
	// make a vector of aggroable player
	CBotAggroOwner::TBotAggroList const& aggroList = spBot->getBotAggroList();
	//typedef std::map<TDataSetRow, TAggroEntryPtr> TBotAggroList;
	CBotAggroOwner::TBotAggroList::const_iterator aggroIt(aggroList.begin()), aggroEnd(aggroList.end());
	TDataSetRow foundRow = TDataSetRow();

	std::vector<TDataSetRow> playerAggroable;
	for (; aggroIt != aggroEnd; ++aggroIt)
	{		
		if (CMirrors::getEntityId(aggroIt->first).getType() != RYZOMID::player)
		{
			continue;
		}
		CAIEntityPhysical* ep = CAIS::instance().getEntityPhysical(aggroIt->first);
		if (!ep)
		{
			continue;
		}
		
		CBotPlayer const* const player = NLMISC::safe_cast<CBotPlayer const*>(ep);
		if (!player)
		{
			continue;
		}
		if (!player->isAggroable())
		{
			continue;
		}					
		playerAggroable.push_back(aggroIt->first);

	}


	// return the size of the aggro list		
	stack.push(float(playerAggroable.size()));
}


/** @page code

@subsection setAggroListTarget_fs_

Maximize the Aggro of a target from the Aggro list of one bot (this id can be given by getRandomPlayerAggroListTarget)..

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index (or bot not spawaned) the function display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.

A player EntityId is used to identify the player. If the entityId is not from the aggro list then a warning message is shown in the log. If the entityId is a empty string nothing no message are displayed.

Arguments: f(botIndex) >s(playerEidAsString) > 
@param[in] botIndex is the index of an bot member of the current group
@param[in] playerEidAsString is EntityId of the player that will be attacked

@code
($playerId)getRandomPlayerAggroListTarget(4); 
()setAggroListTarget(4, $playerId); 
@endcode
*/

void setAggroListTarget_fs_(CStateInstance* entity, CScriptStack& stack)
{

	std::string funName = "setAggroListTarget_fs_";
	

	
	std::string playerEidStr = ((std::string)stack.top()); stack.pop();
	sint32 botIndex = (sint32)((float)stack.top());stack.pop();

	// get the entity id
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());

	// get the spawn bot by its indesx
	CGroup* group = entity->getGroup();
	if (!group || !group->isSpawned() ||  botIndex <0 ||  group->bots().size() <= static_cast<uint32>(botIndex))
	{
		nlwarning("%s used bad index %d/%d (or group not spawn)", funName.c_str(), botIndex, group->bots().size());		
		return;
	}	
	CBot* bot  = group->getBot(botIndex);
	CSpawnBot* spBot = bot?bot->getSpawnObj():0;
	if (!spBot)
	{
		nlwarning("%s used bad index %d/%d (or bot not spawn)", funName.c_str(), botIndex, group->bots().size());		
		return;
	}


	// test if player eid is ok
	if (playerEidStr.empty()) { return; }
	TDataSetRow playerRow = TheDataset.getDataSetRow( playerEid );
	bool accessible = TheDataset.isAccessible( playerRow );
	bool pj = playerEid.getType() == RYZOMID::player;
	CAIEntityPhysical* ep = accessible && pj ? CAIS::instance().getEntityPhysical(playerRow):0;		
	CBotPlayer const* const player = ep ? NLMISC::safe_cast<CBotPlayer const*>(ep):0;
	if (!ep || !player ||!player->isAggroable() )
	{
		nlwarning("Try to call %s with on a player '%s' that is not accessible (or not aggroable). The isPlayerAlived function must be called to be sure that the player is still alived.", funName.c_str(), playerEidStr.c_str() );		
		return;
	}

	
	// maximize aggro from the bot for the player
	spBot->maximizeAggroFor(playerRow);
}


/** @page code

@subsection setGroupAggroListTarget_s_

Maximize the Aggro of a target from the Aggro list of one bot to his group (this id can be given by getRandomPlayerAggroListTarget)..

A player EntityId is used to identify the player. If the entityId is not from the aggro list then a warning message is shown in the log. If the entityId is a empty string nothing no message are displayed.

Arguments: s(playerEidAsString) >  
@param[in] playerEidAsString is EntityId of the player that will be attacked by the group

@code
($playerId)getRandomPlayerAggroListTarget(4); 
()setGroupAggroListTarget($playerId); 
@endcode
*/

void setGroupAggroListTarget_s_(CStateInstance* entity, CScriptStack& stack)
{
	
	std::string funName = "setGroupAggroListTarget_s_";

	// read params
	std::string playerEidStr = ((std::string)stack.top()); stack.pop();

	// read eid of the player
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());

	
	// test if player eid is ok
	if (playerEidStr.empty()) { return; }
	TDataSetRow playerRow = TheDataset.getDataSetRow( playerEid );
	bool accessible = TheDataset.isAccessible( playerRow );
	bool pj = playerEid.getType() == RYZOMID::player;
	CAIEntityPhysical* ep = accessible && pj ? CAIS::instance().getEntityPhysical(playerRow):0;		
	CBotPlayer const* const player = ep ? NLMISC::safe_cast<CBotPlayer const*>(ep):0;
	if (!ep || !player ||!player->isAggroable() )
	{
		nlwarning("Try to call %s with on a player '%s' that is not accessible (or not aggroable). The isPlayerAlived function must be called to be sure that the player is still alived.", funName.c_str(), playerEidStr.c_str() );		
		return;
	}
	
	// test if group is spawn
	CGroup* group = entity->getGroup();
	if (!group || !group->isSpawned() || group->bots().isEmpty() )
	{		
		nlwarning("Call %s on a empty/not spawned group", funName.c_str() );
		return;
	}
	
	// apply maximizeAggroFor on all member of the group
	CCont<CBot>::iterator itBot = group->bots().begin();
	CCont<CBot>::iterator itEnd = group->bots().end();

	for (; itBot != itEnd; ++itBot)
	{

		CSpawnBot* spBot = itBot->getSpawnObj();
		if (!spBot) { continue; }
		spBot->maximizeAggroFor(playerRow);
	}
	
}

/** @page code

@subsection setManagerAggroListTarget_ss_

Maximize the Aggro of a target of one bot to some groups of his manage.

A player EntityId is used to identify the player. If the entityId is not from the aggro list then a warning message is shown in the log. If the entityId is a empty string nothing no message are displayed.

A string is used to select groups of the manager (groups name must contains this string)

Arguments: f(botIndex), s(playerEidAsString) >  
@param[in] playerId is EntityId of the player
@param[in] nameElement The element of the name of all group we are interest in.

@code
($playerId)getRandomPlayerAggroListTarget(0); 
()setManagerAggroListTarget($playerId, "group_bandit_");  // all group that have name like "group_bandit_*" will attack the player
@endcode
*/

void setManagerAggroListTarget_ss_(CStateInstance* entity, CScriptStack& stack)
{
	
	std::string funName = "setManagerAggroListTarget_ss_";


	// read params
	std::string playerEidStr = ((std::string)stack.top()); stack.pop();
	std::string groupNameStr = ((std::string)stack.top()); stack.pop();


	// test if player eid is ok
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());

	if (playerEidStr.empty()) { return; }
	TDataSetRow playerRow = TheDataset.getDataSetRow( playerEid );
	bool accessible = TheDataset.isAccessible( playerRow );
	bool pj = playerEid.getType() == RYZOMID::player;
	CAIEntityPhysical* ep = accessible && pj ? CAIS::instance().getEntityPhysical(playerRow):0;		
	CBotPlayer const* const player = ep ? NLMISC::safe_cast<CBotPlayer const*>(ep):0;
	if (!ep || !player ||!player->isAggroable() )
	{
		nlwarning("Try to call %s with on a player '%s' that is not accessible (or not aggroable). The isPlayerAlived function must be called to be sure that the player is still alived.", funName.c_str(), playerEidStr.c_str() );		
		return;
	}
	
	
	CGroup* group = entity->getGroup();
	if (!group )
	{		
		nlwarning("Call %s on a non exisiting group", funName.c_str() );
		return;
	}
	// get the manager of the group
	CManager* manager = group->getOwner();
	if (!manager)
	{		
		nlwarning("Call %s on a non exisiting manager", funName.c_str() );
		return;
	}
	// for all group of the manager maximize aaggro *IF* the group name contains groupNameStr
	group=manager->getNextValidGroupChild();
	while (group!=NULL)
	{
		if ( ! (!group || !group->isSpawned() || group->bots().isEmpty() ) && group->getName().find(groupNameStr) != std::string::npos )
		{		
				// apply maximizeAggroFor on all member of the group
			CCont<CBot>::iterator itBot = group->bots().begin();
			CCont<CBot>::iterator itEnd = group->bots().end();

			for (; itBot != itEnd; ++itBot)
			{

				CSpawnBot* spBot = itBot->getSpawnObj();
				if (!spBot) { continue; }
				spBot->maximizeAggroFor(playerRow);
			}
		}			
		group = manager->getNextValidGroupChild(group);
	}
			
}



/** @page code

@subsection getBotIndexByName_s_f

Get the index of a bot of a group by its name (or return -1). Mainly useful for scripted boss.

botIndex begins at zero for the first member of the group, -1 if the bot is not found.

Arguments:, s(botName) >  f(botIndex)

@param[in] name is the name of the bot
@param[out] botIndex is the index of an bot member of the current group

@code
(botIndex)getBotIndexByName("boss_random_aggro");
($playerId)getRandomPlayerAggroListTarget(botIndex); 
()setAggroListTarget(botIndex, $playerId); 
}
@endcode
*/
void getBotIndexByName_s_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getBotIndexByName_s_f";

	// read params
	std::string botName = (std::string)stack.top(); stack.pop();

	// test if group is spawned
	CGroup* group = entity->getGroup();
	if (!group || group->bots().isEmpty() )
	{	
		nlwarning("Call %s on a empty group", funName.c_str() );
		stack.push((float)-1);
		return;
	}
	

	// Search on all  persistent bot of the groupe the bot that have the searched name
	CCont<CBot>::iterator itBot = group->bots().begin();
	CCont<CBot>::iterator itEnd = group->bots().end();

	sint32 index = -1;
	for (; itBot != itEnd; ++itBot)
	{
		++index;
		const std::string& name =  itBot->getName();
		if (name == botName)
		{
			stack.push((float)index);
			return;
		}
	}
	
	// Bot not found
	stack.push((float)-1);
	return;
}


/** @page code

@subsection isGroupAlived__f

Test if a group is alived (at least one member is alived)

A bot index is used to identify the bot. If there is only one spawned bot in a group then the index value is 0. If we want the bot index of a specific bot the getBotIndexByName function can be used. The function getBotCount can be used to know the number of bot in a group. The function isGroupAlived or isBotAlived can be used to verify if the index is valid.
If no bot are identified by the bot index (or bot not spawaned) the function display a warning message. You can this of using isBotAlived to be sure that the bot index is still a valid value.


Arguments: s(playerid) >  f(success)

@param[out] sucess is 1.0f if there is at least one member of the group alived (0.0f if not alived bot are found)

@code
($alived)isGroupAlived();
if ($alived == 1.0f) {
} 
@endcode

*/
void isGroupAlived__f(CStateInstance* entity, CScriptStack& stack)
{	
	isAlived__f(entity, stack);
}


/** @page code

@subsection isBotAlived_f_f

Test if a bot of the current group is alived. The bot is identified by its index.

Arguments: f(botIndex) >  f(success)

@param[int] botIndex the index of the bot.
@param[out] sucess is 1.0f if there is at least one member of the group alived (0.0f if not alived bot are found)

@code
($botIndex)getBotIndexByName("boss_3");
$alived)isBotAlived($botIndex);
if (alived == 1.0f) {
}
@endcode

*/
void isBotAlived_f_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "isBotAlived_f_f";
	// get input index
	sint32 botIndex = (sint32)((float)stack.top());	stack.pop();

	// get Spawn Bot
	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());
		stack.push((float)0);
		return;
	}	
	if (!group->isSpawned() || group->bots().isEmpty() ||  botIndex < 0 || group->bots().size() < static_cast<uint32>(botIndex))
	{
		stack.push(0.0f);return;
	}
	const CBot *const	bot = group->getBot(botIndex);
	if	( !bot || !bot->isSpawned()) { stack.push(0.0f); return;};
	CAIEntityPhysical *const	ep=bot->getSpawnObj();
	if	( !ep) { stack.push(0.0f); return;};

	// test if spawn bot is alive
	if (ep->isAlive()) { stack.push(1.0f); return;}

	stack.push(0.0f);
	return;
}


/** @page code

@subsection isPlayerAlived_s_f

Test if a a player is is alived.

A player EntityId is used to identify the player. If the player is not connected, not in the same continent, or dead 0 is returned.

Arguments: s(playerId) >  f(success)

@param[in]  palyerId
@param[out] success is 1.0f if the entity id of a player is alived.

@code
($botIndex)getBotIndexByName("boss_3");
($playerId)getCurrentPlayerAggroListTarget($botIndex):
(alived)isPlayerlived($playerId);
if (alived == 1.0f) {
} 
@endcode
*/
void isPlayerAlived_s_f(CStateInstance* entity, CScriptStack& stack)
{
	// get input data
	std::string playerEidStr = (std::string)stack.top(); stack.pop();

	//get CAIEntityPhysical of the player
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());		
	TDataSetRow playeDataSetRow = TheDataset.getDataSetRow( playerEid) ;
	if (playerEidStr.empty() || !TheDataset.isAccessible( playeDataSetRow ) || playerEid.getType() != RYZOMID::player )
	{
		stack.push(0.0f); return;
	}
	CAIEntityPhysical* ep = CAIS::instance().getEntityPhysical(playeDataSetRow);
	if (!ep)
	{
		
		stack.push(0.0f); return;
	}
	// test if the player is alived
	if (ep->isAlive())
	{
		stack.push(1.0f); return;
	}

	stack.push(0.0f); return;
	
}

/** @page code
@subsection getServerTimeStr__s

Gets the server time as string "eg 21:17:14 the x"
This function is useful for stat purpose or debug (to know when a boss is down)

Arguments: > s(serverTime) 


@param[out] The server time as debug string

@code
($serverTime)getServerTimeAsString();
@endcode
*/

void getServerTimeStr__s(CStateInstance* entity, CScriptStack& stack)
{
	//get the server time (as debug string)
	time_t currentTime;
	time( &currentTime );
	std::string str = NLMISC::toString("%s", asctime(localtime(&currentTime)));
	stack.push(str);
}

/** @page code
@subsection getServerTime__s

Gets the server time as number (it is the number of seconde since 1970). This value is useful for saving the server date on a file

Arguments: > s(serverTime) 

@param[out] The server time as string (a float is not sharp enough

@code
($serverTime)getServerTime();
@endcode
*/

void getServerTime__s(CStateInstance* entity, CScriptStack& stack)
{
	//get the server time (as time stamp)
	time_t currentTime;
	time( &currentTime );
	std::string ret = NLMISC::toString("%d", currentTime);
	// convert to string
	stack.push((std::string)ret);
}



/** @page code
@subsection getRyzomDateStr__s

Gets the ryzom date as string
Arguments: > s(ryzomDateStr)

@param[out] ryzomTimeAndDateAsString The time and date of the ryzom univers as debug string.

@code
($ryzomDateStr)getRyzomDateStr);
@endcode
*/
void getRyzomDateStr__s(CStateInstance* entity, CScriptStack& stack)
{
	// Display time using ryzom style
	std::string result;
	const CRyzomTime &rt = CTimeInterface::getRyzomTime();
	result = NLMISC::toString("%d:%d:00", (int) floorf(rt.getRyzomTime()) , (int) floorf(60.f * fmodf(rt.getRyzomTime(), 1.f)));
	
	uint32 month = rt.getRyzomMonth();
	MONTH::EMonth monthInCycle = rt.getRyzomMonthInCurrentCycle();
	std::string monthName = MONTH::toString((MONTH::EMonth) monthInCycle);
	uint32 dayOfMonth = rt.getRyzomDayOfMonth();			
	std::string dayName = WEEKDAY::toString((WEEKDAY::EWeekDay) rt.getRyzomDayOfWeek());
	result += NLMISC::toString(" / %s %d  %s(%d) %d",
								dayName.c_str(),
								(int) (dayOfMonth + 1),
								monthName.c_str(),
		                       (int)  (month + 1),							   
							   (int) rt.getRyzomYear());
				
	stack.push( result );
}

/** @page code
@subsection getRyzomDate__s

Gets the ryzom tick game cycle. Useful for computing difference of time.
The return value is a string and not a float because float is not sharp enought?
Arguments: > s(tickGameCycle)

@param[out] TickGameCycle The time of the ryzom univers (stops when server stops). 10 tick is  1 second.

@code
($tickGameCycle)getRyzomDate();
@endcode
*/
void getRyzomDate__s(CStateInstance* entity, CScriptStack& stack)
{
	//get GameTickCycle
	const NLMISC::TGameCycle ryzomTime= CTickEventHandler::getGameCycle();
	std::string ret = NLMISC::toString("%d", ryzomTime);
	// convert to string
	stack.push((std::string)ret);
}



class CPhraseParameters
{
public:
	enum TMode {NpcMsg, SystemMsg, EmoteMsg};
	TVectorParamCheck Values;
};

CPhraseParameters PhraseParameters;

//----------------------------------------------------------------------------
/** @page code

@subsection phraseBegin__
Clear the parameters stack.

Used when creating a customized msg via phraseEndSystemMsg_fss_, phraseEndNpcMsg_fss_, phraseEndSystemMsg_fss_.
It Must be called at start of phrase if we are not sure that the parameter stack is clean.

Parameter stack is unclean when someone has called a phrasePush* function but *not* a phraseEnd function before (which is *very* bad).

Arguments:  ->

@code
()phraseBegin();
()phrasePushValue("money", 15);
()phrasePushValue("integer", 15);
()phrasePushValue("time", 15);
()phraseEndSystemMsg(0, "say", "PHRASE_FROM_PHRASE_WITH_3_PARAMETERS");
@endcode

*/
// CGroup
void phraseBegin__(CStateInstance* entity, CScriptStack& stack)
{
	PhraseParameters.Values.clear();
}

//----------------------------------------------------------------------------
/** @page code

@subsection phrasePushValue_sf_
This function push a value as number on the parameter stack. This stack is used by phraseEndSystemMsg_fss_, phraseEndNpcMsg_fss_, phraseEndSystemMsg_fss_ functions.
@see phraseBegin__
@see phrasePushString_ss_
@see phraseEndSystemMsg_fss_
@see phraseEndNpcMsg_fss_
@see phraseEndSystemMsg_fss_


The value *Must* be an number and only few type are handled.
- money
- integer
- time

Some value other can be handled *BUT* the value *MUST*  be  the C++ code enum value. So LD must ask to coder to do a scripting fonction that have as input a string that contains enum value as string eg "matis" and that return the C++ enum value as int (eg 4).
- skill
- faction
- power_type
- race
- damage_type		
- characteristic
- score
- body_part

Arguments: s(paramType), f(value) ->
@param[in] paramType the type of the parameter
@param[in] value the value  of the parameter


@code
()phraseBegin();
()phrasePushValue("money", 15);
()phrasePushValue("integer", 15);
()phrasePushValue("time", 15);
()phraseEndSystemMsg(0, "say", "PHRASE_WITH_3_PARAMETERS");
@endcode

*/
// CGroup
void phrasePushValue_sf_(CStateInstance* entity, CScriptStack& stack)
{
	float f = (float)stack.top();stack.pop(); // get the value as float 
	std::string typeStr = (std::string)stack.top(); stack.pop(); // get the param type
	// create the good STRING_MANAGER::TParam in fonction of its type and is value
	STRING_MANAGER::TParamType type = STRING_MANAGER::stringToParamType(typeStr);
	STRING_MANAGER::TParam param;
	param.Type = type;
	switch( type )	
	{	
		
		case STRING_MANAGER::money:
		{
			param.Money = static_cast<uint64>(f);
			break;
		}
		
				
		case STRING_MANAGER::integer:
		{
			param.Int = static_cast<sint32>(f);
			break;
		}

		case STRING_MANAGER::time:
		{
			param.Time = static_cast<uint32>(f);
			break;
		}			

		case STRING_MANAGER::skill:
		case STRING_MANAGER::faction:
		case STRING_MANAGER::power_type:
		case STRING_MANAGER::race:
		case STRING_MANAGER::damage_type:			
		case STRING_MANAGER::characteristic:
		case STRING_MANAGER::score:
		case STRING_MANAGER::body_part:	
		{		
			param.Enum = static_cast<uint32>( f );	
			break;
		}

		default:
			param.Type = STRING_MANAGER::invalid_value;

		break;
	}
	
	PhraseParameters.Values.push_back(param);
}


//----------------------------------------------------------------------------
/** @page code

@subsection phrasePushString_ss_
This function push a value as string on the parameter stack. This stack is used by phraseEndSystemMsg_fss_, phraseEndNpcMsg_fss_, phraseEndSystemMsg_fss_ functions.
@see phraseBegin__
@see phrasePushValue_sf_
@see phraseEndSystemMsg_fss_
@see phraseEndNpcMsg_fss_
@see phraseEndSystemMsg_fss_


The value *Must* be a string.
The string is internal converted to number, sheetId, entityId  when needed.

Input as a number:
- money
- integer
- time

Input as string literal
-  literal

Input as an entityId (obtains via getCurrentPlayerAggroListTarget_f_s, getRandomPlayerAggroListTarget_f_s, getBotEid_f_s, getCurrentSpeakerEid__s, getAggroListElement_ff_s)
- player
- bot
- entity

Input as sheetId name  (example: "toto.sbrick", "toto.sitem", "toto.creature")
- item
- outpost
- creature_model
- creature
- sphrase
- sbrick

Input as string identifier
- place
- event_faction
- title
- bot_name

Input as stringId (number): *WARNING* LD must as coder to do function that return string_id if they want to use that type
- dyn_string_id
- string_id


Input must be a enum value:
Some value other can be handled *BUT* the value *MUST*  be  the C++ code enum value. So LD must ask to coder to do a scripting fonction that have as input a string that contains enum value as string eg "matis" and that return the C++ enum value as int (eg 4).
- skill
- faction
- power_type
- race
- damage_type		
- characteristic
- score
- body_part

Arguments: s(paramType), f(value) ->
@param[in] paramType the type of the parameter
@param[in] value the value  of the parameter


@code
($playerEid)getCurrentPlayerEid();
($botEid)group3.getBotEid(4);
()phraseBegin();
()phrasePushValue("integer", 15);
()phrasePushString("integer", "15");
()phrasePushString("literal", "Test 123");
()phrasePushString("player", $playerEid);
()phrasePushString("bot", $botEid);
()phrasePushString("item", "abc.sitem");
()phrasePushString("sbrick", "toto.sbrick");
()phrasePushString("creature_model", "toto.creature");
()phraseEndSystemMsg(0, "say", "PHRASE_WITH_SOME_PARAMETERS");
@endcode
*/
void phrasePushString_ss_(CStateInstance* entity, CScriptStack& stack )
{
	std::string s = (std::string)stack.top(); stack.pop(); // get the param value
	std::string typeStr = (std::string)stack.top(); stack.pop(); // get the param type

	// construct a STRING_MANAGER::TParam in fonction of its type and its value
	STRING_MANAGER::TParamType type = STRING_MANAGER::stringToParamType(typeStr);
	STRING_MANAGER::TParam param;
	param.Type = type;
	switch( type )	
	{	
		
		case STRING_MANAGER::money:
		{
			NLMISC::fromString(s, param.Money);
			break;
		}
	
		case STRING_MANAGER::player:
		case STRING_MANAGER::bot:
		case STRING_MANAGER::entity:
		{

			NLMISC::CEntityId id;
			uint32 alias = 0;
			id.fromString(s.c_str());
			CAIEntityPhysical* entityPhysical=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(id));
			if (entityPhysical)	
			{
				switch( entityPhysical->getRyzomType())
				{
					case RYZOMID::creature:
					case RYZOMID::npc:
					{
					
						CSpawnBot* sb = NLMISC::safe_cast<CSpawnBot*>(entityPhysical);
						alias = sb->getPersistent().getAlias();
						break;
					}
					default:
						;
						// player or other have no alias
				}
			}
			param.setEIdAIAlias( id, alias );
		}
		break;
				
		case STRING_MANAGER::integer:
		{
			NLMISC::fromString(s, param.Int);
			break;
		}

		case STRING_MANAGER::time:
		{
			NLMISC::fromString(s, param.Time);
			break;
		}
		
				
		case STRING_MANAGER::item:	
		case STRING_MANAGER::outpost:
		case STRING_MANAGER::creature_model:
		case STRING_MANAGER::creature:
		case STRING_MANAGER::sphrase:
		case STRING_MANAGER::sbrick:
		{
			if (s.empty())
			{
				nlwarning("Sheet name expected but not found in script");
				param.SheetId= CSheetId();
			}
			else
			{
				param.SheetId = CSheetId(s);
			}
			break;
		}
		
		
		case STRING_MANAGER::place:
		case STRING_MANAGER::event_faction:		
		case STRING_MANAGER::title:
		case STRING_MANAGER::bot_name:
		{
			param.Identifier = s;
			break;
		}

		case STRING_MANAGER::skill:
		case STRING_MANAGER::faction:
		case STRING_MANAGER::power_type:
		case STRING_MANAGER::race:
		case STRING_MANAGER::damage_type:			
		case STRING_MANAGER::characteristic:
		case STRING_MANAGER::score:
		case STRING_MANAGER::body_part:	
		{		
			NLMISC::fromString(s, param.Enum);
			break;
		}

		case STRING_MANAGER::literal:
			param.Literal.fromUtf8(s);
			break;

		case STRING_MANAGER::dyn_string_id:
		case STRING_MANAGER::string_id:
			NLMISC::fromString(s, param.StringId);
			break;

		case STRING_MANAGER::self:
		case STRING_MANAGER::role:
		case STRING_MANAGER::compass:
		case STRING_MANAGER::guild:
		case STRING_MANAGER::ecosystem:		
		case STRING_MANAGER::classification_type:
		default:
			param.Type = STRING_MANAGER::invalid_value;

		break;
	}
	
	PhraseParameters.Values.push_back(param);
}



static void phraseEnd(CStateInstance* entity, CScriptStack& stack,  CPhraseParameters::TMode mode)
{
	std::string funName = "phraseEnd";
	
	// get Function parameters			
	
	std::string phraseId = (std::string)stack.top();stack.pop();
	std::string sayMode;
	if (mode != CPhraseParameters::EmoteMsg)
	{
		sayMode = (std::string)stack.top();stack.pop();
	}
	sint32 botIndex = (sint32)((float)stack.top());stack.pop();
	

	// Verify is bot is alived


	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());
		PhraseParameters.Values.clear();
		return;
	}	
	if (!group->isSpawned() || group->bots().isEmpty() ||  botIndex < 0 || group->bots().size() < static_cast<uint32>(botIndex))
	{
		PhraseParameters.Values.clear();
		return;
	}	
	const CBot *const	bot = group->getBot(botIndex);
	if	( !bot || !bot->isSpawned()) 
	{
		PhraseParameters.Values.clear();
		return;
	}
	CSpawnBot *const	sp= bot->getSpawnObj();
	if	( !sp || !sp->isAlive() )
	{
		PhraseParameters.Values.clear();
		return;
	}
	
	// parse type of chat
	CChatGroup::TGroupType groupType;
	if (mode != CPhraseParameters::EmoteMsg)
	{
		groupType = CChatGroup::stringToGroupType(sayMode);
	}

	// verify phraseId validity
	if (phraseId.empty())
	{
		nlwarning("%s %d: Phrase identifier is empty", funName.c_str(),static_cast<uint32>(mode));
		return;
	}

	// send chat to client 
	switch (mode)
	{
	case CPhraseParameters::NpcMsg:
		npcChatParamToChannel(sp->dataSetRow(), groupType, phraseId.c_str(), PhraseParameters.Values);	
		break;
	case CPhraseParameters::SystemMsg:
		STRING_MANAGER::sendSystemStringToClientAudience(sp->dataSetRow(),std::vector<NLMISC::CEntityId>(), groupType, phraseId.c_str(), PhraseParameters.Values);
		break;

	case CPhraseParameters::EmoteMsg:
		STRING_MANAGER::sendCustomEmoteTextToClientAudience(sp->dataSetRow(),std::vector<NLMISC::CEntityId>(), phraseId.c_str(), PhraseParameters.Values);	
		break;
	}
	
	PhraseParameters.Values.clear();
}

//----------------------------------------------------------------------------
/** @page code

@subsection phraseEndNpcMsg_fss_
Send a message with parameter through a bot says. 
Parameters are taken from the parameters stack.
@see phrasePushValue_sf_
@see phrasePushString_ss_

Arguments: s(botIndex), s(sayType), s(phraseIdentifier) ->
@param[in] botIndex the Position of the bot in the group ( see getBotIndexByName_s_f)
@param[in] sayType Is the type of the say dialog ("say", "shout", "civilization", "territory", "universe", "arround", "system", "region")
@param[in] phraseIdentifier Is the identifier phrase as seen in phrase_wk.uxt

@code
()phrasePushString("literal", "text non traduit");
()groupOf5Bot.phraseEndNpcMsg(4, "say", "PHRASE_TOTO");
@endcode

WARNING
In this case in the phrase_wk.txt PHRASE_TOTO must be defined as
@code
PHRASE_TOTO(bot b, literal l)
{
	[I am $b$, on this text is not translated $l$ ]
}
@endcode

The first parameter is ALWAYS the bot that says the text (value is automaticaly set)
*/
void phraseEndNpcMsg_fss_(CStateInstance* entity, CScriptStack& stack)
{
	phraseEnd(entity, stack, CPhraseParameters::NpcMsg);
}

//----------------------------------------------------------------------------
/** @page code

@subsection phraseEndSystemMsg_fss_
Send a message with parameter through system braodcast msg.
Parameters are taken from the parameters stack.
@see phrasePushValue_sf_
@see phrasePushString_ss_

Arguments: s(botIndex), s(sayType), s(phraseIdentifier) ->
@param[in] botIndex the Position of the bot in the group ( see getBotIndexByName_s_f)
@param[in] sayType Is the type of the say dialog ("say", "shout", "civilization", "territory", "universe", "arround", "system", "region")
@param[in] phraseIdentifier Is the identifier phrase as seen in phrase_wk.uxt

@code
()phrasePushString("literal", "Test des levels designer");
()groupOf5Bot.phraseEndSystemMsg(4, "say", "PHRASE_TOTO");
@endcode

*WARNING*
In this case in the phrase_wk.txt PHRASE_TOTO must be defined as
@code
PHRASE_TOTO(literal l)
{
	[$l$]
}
@endcode
The first parameter is *NOT* automaticaly set to the bot that send the system msg as phraseEndNpcMsg_fss_.

Because the msg can be send as "around". The broadcas msg must be send by a bot (that have a valid position)
*/
void phraseEndSystemMsg_fss_(CStateInstance* entity, CScriptStack& stack)
{
	phraseEnd(entity, stack, CPhraseParameters::SystemMsg);
}


//----------------------------------------------------------------------------
/** @page code

@subsection phraseEndEmoteMsg_fs_
Send a custom emote message with parameter through system braodcast msg.
Parameters are taken from the parameters stack.
@see phrasePushValue_sf_
@see phrasePushString_ss_

Arguments: s(botIndex), s(phraseIdentifier) ->
@param[in] botIndex the Position of the bot in the group ( see getBotIndexByName_s_f)
@param[in] phraseIdentifier Is the identifier phrase as seen in phrase_wk.uxt

@code
($playerEid)getCurrentPlayerEid();
($botEid)getCurrentSpeakerEid();

()phrasePushString("player", $playerEid);
()groupOf5Bot.phraseEndEmoteMsg(,"PHRASE_TOTO1");

()phrasePushString("bot", $botEid);
()groupOf5Bot.phraseEndEmoteMsg(,"PHRASE_TOTO2");
@endcode

*WARNING*
In this case in the phrase_wk.txt PHRASE_TOTO must be defined as
@code
PHRASE_TOTO1(player p)
{
	[$p$ is laughing ]
}
PHRASE_TOTO2(bot b)
{
	[$b$ is sad ]
}
@endcode
The first parameter is NOT automaticaly the bot that says the text as phraseEndNpcMsg_fss_ because a bot can make a player doing a emote text.

The emote msg must be send by a bot (that have a valid position).
*/
void phraseEndEmoteMsg_fs_(CStateInstance* entity, CScriptStack& stack)
{
	phraseEnd(entity, stack, CPhraseParameters::EmoteMsg);
}


//----------------------------------------------------------------------------
/** @page code

@subsection queryEgs_sscfs_
Send a query msg to egs to know infos on a player. 
Answer is asynchronous so we have to indicates a group and a user event that will be triggered when answer will come back to AIS

Possible info to know are 
- Name
- Hp
- MaxHp
- RatioHp
- Sap
- MaxSap
- RatioSap
- Focus
- MaxFocus
- RatioFocus
- Stamina
- MaxStamina
- RatioStamina


Arguments: s(botIndex), s(query), c(groupThatWillBeTriggered), f(idOfTheUserEvent), s(msgId)
@param[in] botIndex the Position of the bot in the group ( see getBotIndexByName_s_f)
@param[in] query The query we want to send
@param[in] groupThatWillBeTriggered The group that will receive a user_event when the answer will come
@param[in] idOfTheUserEvent The number of the user event that will be triggered
@param[in] msgId The id of the msg

Answer will be given by the getParam

@code
	//Sening msg to EGS
	(@groupToNotify)boss_group.context();
	()queryEgs("Name", $playerEid, @groupToNotify, 4, "MSG_NAME");
	()queryEgs("Hp", $playerEid, @groupToNotify, 4, "msg1");
	()queryEgs("MaxHp", $playerEid, @groupToNotify, 4, "msg2");
	()queryEgs("RatioHp", $playerEid, @groupToNotify, 4, "msg3");
	()queryEgs("Sap", $playerEid, @groupToNotify, 4,  "msg4");
	()queryEgs("MaxSap", $playerEid, @groupToNotify, 4,  "msg5");
	()queryEgs("RatioSap", $playerEid, @groupToNotify, 4,  "msg6");
	()queryEgs("Focus", $playerEid, @groupToNotify, 4,  "msg7");
	()queryEgs("MaxFocus", $playerEid, @groupToNotify, 4,  "msg8");
	()queryEgs("RatioFocus", $playerEid, @groupToNotify, 4,  "msg9");
	()queryEgs("Stamina", $playerEid, @groupToNotify, 4,  "msg10");
	()queryEgs("MaxStamina", $playerEid, @groupToNotify, 4,  "msg11");
	()queryEgs("RatioStamina", $playerEid, @groupToNotify, 4,  "msg12");
	()queryEgs("BestSkillLevel", $playerEid, @groupToNotify, 4, "msg13");
@endcode
Answer of the EGS
@code
	// the user_event 4 of groupToNotify will be trigered 
	($msgName)getEventParam(0); // the msg name
	($ret)getEventParam(1); // the return
	($funName)getEventParam(2); // the name of the function
	($playerEid)getEventParam(3); // the id of the player
	($param1)getEventParam(4); // empty ot item, or sbrick or botEid

	if ($msgName == "MSG_NAME") {
		()phrasePushString("literal", $ret);
		()phrasePushString("player", $playerEid);
		()boss_group.phraseEndNpcMsg(0, "say", "TEST_BOSS_TEST_MSG");
	}
	else
	{
		()phrasePushString("player", $playerEid);
		()phrasePushString("literal", $funName);
		()phrasePushString("literal", $msgName);
		()phrasePushString("integer", $ret);
		()boss_group.phraseEndNpcMsg(0, "say", "TEST_BOSS_TEST_EGS_QUERY");

	}
@endcode

*/
void queryEgs_sscfs_(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "queryEgs_sscfs_";
	// read input params
	string literal = (string)stack.top(); stack.pop();
	float useEventId = (float)stack.top(); stack.pop();	
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() ); stack.pop();	
	string param1 = (string)stack.top(); stack.pop();
	string func = (string)stack.top(); stack.pop();

	// create a CUserEventMsg to send to EGS
	CGroup* const group = entity ? entity->getGroup() : 0;
	IManagerParent* const managerParent = (group&& group->getOwner()) ? group->getOwner()->getOwner():0;
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("%s failed: the AI instance of the entity is NULL", funName.c_str());
		DEBUG_STOP;
		return;
	}
	CQueryEgs  msg;
	msg.InstanceId = aiInstance->getInstanceNumber();
	msg.GroupAlias = groupToNotify->getAlias();
	msg.EventId = (int)useEventId;
	msg.Params.push_back(literal);
	msg.Params.push_back(func);
	msg.Params.push_back(param1);
	msg.send("EGS");

}

/*
@subsection queryEgs_ssscfs_
Send a query msg to egs to know infos on a player. 
Answer is asynchronous so we have to indicates a group and a user event that will be triggered when answer will come back to AIS

Possible info to know are: 
- KnowBrick (to knwo if the player know a specific brick); return value is 0 or 1 (if the player know the brick)
- IsInInventory (to knwo has an item in inventory of in equipement); return value is 0, 1(item in equipment), 2(item in bag) 
- Target (to know if a player target a bot; return value is 0, 1(player target the bot)
	
Arguments: s(botIndex), s(query), s(queryParam), c(groupThatWillBeTriggered), f(idOfTheUserEvent), s(msgId)
@param[in] botIndex the Position of the bot in the group ( see getBotIndexByName_s_f)
@param[in] query The query we want to send
@param[in] queryParam Is the paramter of the query (a .creature of IsInInventory, a .sbrick for KnowBrick, a Bot EntityId for Target
@param[in] groupThatWillBeTriggered The group that will receive a user_event when the answer will come
@param[in] idOfTheUserEvent The number of the user event that will be triggered
@param[in] msgId The id of the msg

Answer will be given by the getParam
@code
	//Sening msg to EGS
	//($playerEid)getCurrentPlayerEid();
	(@groupToNotify)boss_group.context();
	()queryEgs("KnowBrick", $playerEid, "bfma01.sbrick", @groupToNotify, 4, "MSG_BRICK");
	()queryEgs("IsInInventory", $playerEid, "iccm1bm.sitem", @groupToNotify, 4, "MSG_ITEM");
	()queryEgs("Target", $playerEid, $botEid, @groupToNotify, 4, "msg14");
@endcode

Answer of the EGS
@code
	($msgName)getEventParam(0); // the msg name
	($ret)getEventParam(1); // the return
	($funName)getEventParam(2); // the name of the function
	($playerEid)getEventParam(3); // the id of the player
	($param1)getEventParam(4); // empty ot item, or sbrick or botEid

	if ($msgName == "MSG_ITEM")
	{
		()phrasePushString("item", $param1);
		()phrasePushString("integer", $ret);
		()boss_group.phraseEndNpcMsg(0, "say", "TEST_BOSS_TEST_EGS_QUERY_ITEM");
	} else if ($msgName == "MSG_BRICK") {
		()phrasePushString("sbrick", $param1);
		()phrasePushString("integer", $ret);
		()boss_group.phraseEndNpcMsg(0, "say", "TEST_BOSS_TEST_EGS_QUERY_BRICK");

	} else if ($msgName == "MSG_NAME") {
		()phrasePushString("literal", $ret);
		()phrasePushString("player", $playerEid);
		()boss_group.phraseEndNpcMsg(0, "say", "TEST_BOSS_TEST_MSG");
	}
	else
	{
		()phrasePushString("player", $playerEid);
		()phrasePushString("literal", $funName);
		()phrasePushString("literal", $msgName);
		()phrasePushString("integer", $ret);
		()boss_group.phraseEndNpcMsg(0, "say", "TEST_BOSS_TEST_EGS_QUERY");

	}
@endcode
*/
void queryEgs_ssscfs_(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "queryEgs_ssscfs_";
	// get input params
	string literal = (string)stack.top(); stack.pop();
	float useEventId = (float)stack.top(); stack.pop();	
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() ); stack.pop();	
	string param2 = (string)stack.top(); stack.pop();
	string param1 = (string)stack.top(); stack.pop();
	string func = (string)stack.top(); stack.pop();

	// create CQueryEgs to send to egs
	CGroup* const group = entity ? entity->getGroup() : 0;
	IManagerParent* const managerParent = (group&& group->getOwner()) ? group->getOwner()->getOwner():0;
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("%s failed: the AI instance of the entity is NULL", funName.c_str());
		DEBUG_STOP;
		return;
	}
	CQueryEgs  msg;
	msg.InstanceId = aiInstance->getInstanceNumber();
	msg.GroupAlias = groupToNotify->getAlias();
	msg.EventId = (int)useEventId;
	msg.Params.push_back(literal);
	msg.Params.push_back(func);
	msg.Params.push_back(param1);
	msg.Params.push_back(param2);
	msg.send("EGS");

}


/** @page code

@subsection summonPlayer_fs_

Summon a player to a bot.

Can be used by a boss to teleport a player. Or the create a teleporter.

A player EntityId is used to identify the player.
A index is used to identified the bot (index for the current group)


Arguments:  f(botIndex), s(playerEid) > 
@param[in] botIndex is the index of the bot in the current group(static group)
@param[in] playerEid The entityId of the player

@code
($playerId)getRandomPlayerAggroListTarget(0); 
()teleportPlayer(0, $playerEid); // teleport player to the boss.
@endcode
*/
void summonPlayer_fs_(CStateInstance* entity, CScriptStack& stack)
{

	std::string funName = "summonPlayer_fffs_";
	std::string playerEidStr = ((std::string)stack.top()); stack.pop();
	sint32 botIndex = (sint32)((float)stack.top()); stack.pop();
	

	// Verify is bot is alived
	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());		
		return;
	}	
	if (!group->isSpawned() || group->bots().isEmpty() ||  botIndex < 0 || group->bots().size() < static_cast<uint32>(botIndex))
	{		
		return;
	}	
	const CBot *const	bot = group->getBot(botIndex);
	if	( !bot || !bot->isSpawned()) 
	{	
		return;
	}
	CSpawnBot *const	sp= bot->getSpawnObj();
	if	( !sp || !sp->isAlive() )
	{	
		return;
	}


	// Read position for mirror
	TDataSetRow row = sp->dataSetRow();		
	if (! TheDataset.isAccessible( row ) )
	{	
		return;
	}

	CMirrorPropValue<sint32> mirrorSymbolX( TheDataset, row, DSPropertyPOSX );
	CMirrorPropValue<sint32> mirrorSymbolY( TheDataset, row, DSPropertyPOSY );

	// retrieve the CBotPlayer
	NLMISC::CEntityId playerEid;
	playerEid.fromString(playerEidStr.c_str());
	CAIEntityPhysical	*charEntity=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(playerEid));
	if	(!charEntity)
		return;

	// do nothing if one of them is dead
	if (!charEntity->isAlive() )
	{
		return;
	}


	// send teleport MSG
	TDataSetRow	CharacterRowId = charEntity->dataSetRow();

	NLMISC::CEntityId player = CMirrors::getEntityId(CharacterRowId);
	
	if (player != NLMISC::CEntityId::Unknown)
	{	
		sint32 x2 = mirrorSymbolX.getValue();
		sint32 y2 = mirrorSymbolY.getValue();
		sint32 z2 = 0;
		float t = 0; // TODO direction to mob?

		NLNET::CMessage msgout( "TELEPORT_PLAYER" );
		nlWrite(msgout, serial, player );
		msgout.serial( const_cast<sint32 &>(x2) );
		msgout.serial( const_cast<sint32 &>(y2) );		
		msgout.serial( const_cast<sint32 &>(z2) );		
		msgout.serial( const_cast<float &>(t) );		
		sendMessageViaMirror( "EGS", msgout );	
	}

	
}

/** @page code

@subsection teleportPlayer_sffff_

Teleport a player to a position

A player EntityId is used to identify the player.
The position is identified by the value x,y,z and the heading


Arguments: s(playerId), f(x), f(y), f(z), f(heading) >  
@param[in] playerId is EntityId of the player
@param[in] x,y,z,heading is the new position of the player

@code
($playerId)getRandomPlayerAggroListTarget(0); 
()teleportPlayer($playerEid, 1000, 1000, 100, 0);
@endcode
*/
void teleportPlayer_sffff_(CStateInstance* entity, CScriptStack& stack)
{
	// get player and position parameters
	float t = (float)stack.top(); stack.pop(); // heading
	float z = (float)stack.top(); stack.pop();
	float y = (float)stack.top(); stack.pop();
	float x = (float)stack.top(); stack.pop();
	std::string playerEidStr = (std::string)stack.top(); stack.pop();
	NLMISC::CEntityId playerEid; 
	playerEid.fromString(playerEidStr.c_str());

	// retrieve the CBotPlayer
	CAIEntityPhysical	*charEntity=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(playerEid));
	if	(!charEntity)
		return;

	// do nothing if one of them is dead
	if (!charEntity->isAlive() )
	{
		return;
	}

	// teleport player to position
	if (playerEid != NLMISC::CEntityId::Unknown)
	{		
		sint32 x2 = static_cast<sint32>(x*1000);
		sint32 y2 = static_cast<sint32>(y*1000);
		sint32 z2 = static_cast<sint32>(z*1000);		

		NLNET::CMessage msgout( "TELEPORT_PLAYER" );
		msgout.serial( const_cast<CEntityId &>(playerEid) );
		msgout.serial( const_cast<sint32 &>(x2) );
		msgout.serial( const_cast<sint32 &>(y2) );		
		msgout.serial( const_cast<sint32 &>(z2) );		
		msgout.serial( const_cast<float &>(t) );		
		sendMessageViaMirror( "EGS", msgout );	
	}
	
}
//----------------------------------------------------------------------------
/** @page code

@subsection getBotEid_f_s
Get the bot EntityId by its Index.
Arguments:    f(botIndex) -> s(botEid)

@param[in] botIndex the Position of the bot in the group
@param[out] botEid The entity Id given by the bot (or empty) if the indexed bot is not from the group

@code
(index)getBotIndexByName("bot_toto");
($botEid)getBotEid(index);
if (index != -1)
{
()phrasePushValue("entity", $botEid);
()phraseEndEmoteMsg(index, "PHRASE_YOUR_ARE_CLICKING_ON_ME");
}
@endcode
*/

void getBotEid_f_s(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getBotEid_f_s";
	// get spawn Bot by its index
	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());
		stack.push(std::string(CEntityId::Unknown.toString()));
		return;
	}	
	sint32 botIndex = (sint32)((float)stack.top());stack.pop();
	if (!group->isSpawned() || group->bots().isEmpty() ||  botIndex < 0 || group->bots().size() < static_cast<uint32>(botIndex))
	{
		stack.push(std::string(CEntityId::Unknown.toString()));
		return;
	}
	
	const CBot *const	bot = group->getBot(botIndex);
	if	( !bot  || !bot->isSpawned()) { stack.push(std::string(CEntityId::Unknown.toString())); return;}


	CSpawnBot *const	sb= bot->getSpawnObj();
	if	( !sb ||!sb->isAlive()) {stack.push(std::string(CEntityId::Unknown.toString())); return;};

	// return the entity id of the spawn bot
	CEntityId id= sb->getEntityId();
	stack.push(std::string(id.toString()));

}

//----------------------------------------------------------------------------
/** @page code

@subsection getBotIndex_s_f
Get the bot Index by its entityId.
Entity Id of a bot can be given via getCurrentSpeackerEid().
It can be useful to Known the index of the bot in the group with the EntityId.

Arguments:  s(botEid) -> f(botIndex),

@param[in] botEid The entity Id given by the bot
@param[out] botIndex the Position of the bot in the group (or -1 if the entity_id is not from a bot of the group)

@code
($botEid)getCurrentSpeakerEid();
(index)getBotIndex($botEid);
if (index != -1)
{
()phrasePushValue("entity", $botEid);
()phraseEndNpcg(index, "PHRASE_YOUR_ARE_CLICKING_ON_ME");
}
@endcode
*/
void getBotIndex_s_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getBotEid_f_s";
	CGroup* group = entity->getGroup();
	if (!group)
	{	
		nlwarning("%s on a non Npc Group, doesn't work", funName.c_str());
		stack.push(std::string(CEntityId::Unknown.toString()));
		return;
	}	
	std::string botEid = (std::string)stack.top(); stack.pop();
		
	// look in the group if the bot is alived
	uint32 botIndex = 0, last = group->bots().size();
	for ( ;botIndex != last; ++botIndex)
	{
		if (!group->isSpawned() || group->bots().isEmpty() || group->bots().size() < static_cast<uint32>(botIndex))
		{
			continue;			
		}
		
		const CBot *const	bot = group->getBot(botIndex);
		if	( !bot  || !bot->isSpawned()) { continue; }


		CSpawnBot *const	sb= bot->getSpawnObj();
		if	( !sb ||!sb->isAlive()) {continue;};
		CEntityId id= sb->getEntityId();
		// if bot is alived return its index
		if (botEid == id.toString())
		{
			stack.push((float)botIndex);
			return;
		}

	}
	stack.push((float)-1);
	return;
	
}

/** @page code

@subsection getCurrentPlayerEid__s
Get the entity id of the player that is clicking on a bot.

WARNING the function is only valid when called from an player_target_npc event.


Arguments: -> s(playerEidAsString),
@param[out] playerEidAsString is EntityId as string from the player.

@code
($playerEid)getCurrentPlayerEid();
(index)getBotIndexByName("toto");
(distance)getPlayerDistance(index, $playerEid);
phrasePushString("player", $playerEid);
phrasePushValue("interger", distance);
phraseEndNpcMsg(index, "say", "MSG_BOT_B_SAYS_THE_PLAYER_P_IS_AT_DISTANCE_D");
@endcode
*/
void getCurrentPlayerEid__s(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getCurrentPlayerEid__s";
	CEntityId id = CEntityId::Unknown;
	// if we are in player_target_npc event TempPlayer is valid
	if (!TempPlayer)		
	{
		//TempPlayer is invalid so return Unkwn eid
		std::string s = id.toString();
		stack.push(s);
		return;
	}
	// TempPlayer is valid so return eid
	id = TempPlayer->getEntityId();
	std::string s = id.toString();
	stack.push(s);
	return;
}


/** @page code

@subsection getCurrentSpeakerEid__s
Get the entity id of the bot at which the player as that is clicking at.

WARNING the function is only valid when called from an player_target_npc event.


Arguments: -> s(botEidAsString),
@param[out] botEidAsString is EntityId as string from the bot.

@code
($botEid)getCurrentSpeakerEid();
($playerEid)getCurrentSpeakerEid();

phrasePushString("player", $playerEid);
phrasePushValue("bot", $botEid);
phraseEndEmotMsg(index, "EMOT_PLAYER_INSULT_BOT");
@endcode
*/
void getCurrentSpeakerEid__s(CStateInstance* entity, CScriptStack& stack)
{
	std::string funName = "getCurrentSpeakerEid__s";
	CEntityId id = CEntityId::Unknown;
	// if we are in player_target_npc event TempSpeaker is valid
	if (!TempSpeaker)
	{
		//TempSpeaker is invalid so return Unkwn eid
		std::string s = id.toString();
		stack.push(s);
		return;
	}
	//TempSpeaker is valid so return correct eid
	id = TempSpeaker->getEntityId();
	std::string s = id.toString();
	stack.push(s);
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection setSheet_s_
Change the sheet of a creature

Arguments: -> s(sheetName)

@code
()setSheet('ccdeb2');

@endcode

*/
void setSheet_s_(CStateInstance* entity, CScriptStack& stack)
{
	string sheetname = stack.top();
	stack.pop();
	
	CSheetId sheetId(sheetname+".creature");
	if (sheetId==CSheetId::Unknown)
		return;
	
	FOREACH(itBot, CCont<CBot>, entity->getGroup()->bots())
	{
		CBot* bot = *itBot;
		if (bot)
		{
			AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
			if (!sheet.isNull())
				bot->triggerSetSheet(sheet);
		}
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection setClientSheet_s_
Change the client sheet of a creature

Arguments: -> s(sheetName)

@code
()setClientSheet('ccdeb2');

@endcode

*/
void setClientSheet_s_(CStateInstance* entity, CScriptStack& stack)
{
	string sheetname = stack.top();
	stack.pop();
	
	if (sheetname.find(".creature") == string::npos)
		sheetname += ".creature";

	FOREACH(itBot, CCont<CBot>, entity->getGroup()->bots())
	{
		CBot* bot = *itBot;
		if (bot)
		{
			bot->setClientSheet(sheetname);
		}
	}
}

/****************************************************************************/

//----------------------------------------------------------------------------
/** @page code

@subsection setHealer_f_
Make the group healer (need test)

Arguments: -> f(isHealer)

@code
()setHealer(1);

@endcode

*/
void setHealer_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool value = ((float)stack.top())!=0.f;
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
	{
		FOREACH(itBot, CCont<CBot>, group->bots())
		{
			CBot* bot = *itBot;
			if (bot)
			{
				bot->setHealer(value);
				//	break; // :NOTE: Only set first bot as a healer
			}
		}
	}
}

/****************************************************************************/

//----------------------------------------------------------------------------
/** @page code

@subsection sitDown__
Make the group sit Down

Arguments: ->

@code
()sitDown();

@endcode

*/
// CStateInstance
void sitDown__(CStateInstance* entity, CScriptStack& stack)
{

	CGroup* group = entity->getGroup();

	if (!group)
	{
		nlwarning("sitDown__ failed");
		return;
	}
	
	CAILogicActionSitDownHelper::sitDown(group);
}


//----------------------------------------------------------------------------
/** @page code

@subsection standUp
Make the group stand up (if was previously stand down)

Arguments: ->

@code
()standUp(); 

@endcode

*/
// CStateInstance
void standUp__(CStateInstance* entity, CScriptStack& stack)
{
	CGroup* group = entity->getGroup();
	if (!group)
	{
		nlwarning("standUp__ failed");
		return;
	}
	
	CAILogicActionSitDownHelper::standUp(group);
}

//----------------------------------------------------------------------------
/** @page code

@subsection standUp
Use to implement setConditionRet

Arguments: ->

@code
()setConditionRet(1); 

@endcode

*/
// CStateInstance
void setConditionSuccess_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool conditionState = (uint32)((float)stack.top()) != 0;
	CGroup* group = entity->getGroup();
	if (!group)
	{
		nlwarning("setConditionSuccess_f_ failed");
		return;
	}
	
	CAILogicDynamicIfHelper::setConditionSuccess(conditionState);
}

inline
static float randomAngle()
{
	uint32 const maxLimit = CAngle::PI*2;
	float val = (float)CAIS::rand32(maxLimit);
	return val;
}

//----------------------------------------------------------------------------
/** @page code

@subsection facing_f_

The npc will face the given direction


Arguments: f(direction)
@param[in] direction is the new angle of the bot in radians

@code
()facing(3.14);
@endcode

*/

// CStateInstance
void facing_f_(CStateInstance* entity, CScriptStack& stack)
{
	float const theta = (float)stack.top(); stack.pop();
	CGroup* group = entity->getGroup();

	bool bRandomAngle = false;
	if (theta > (NLMISC::Pi * 2.0) || theta < (-NLMISC::Pi * 2.0))
		bRandomAngle = true;

	if (group->isSpawned())
	{
		FOREACH(itBot, CCont<CBot>, group->bots())
		{
			CBot* bot = *itBot;
			if (bot)
			{
				if (bot->isSpawned())
				{
					CSpawnBot *spawnBot = bot->getSpawnObj();

					if (bRandomAngle)
						spawnBot->setTheta(randomAngle());
					else
						spawnBot->setTheta(theta);
					
				}
			}
		}
	}
}

std::map<std::string, FScrptNativeFunc> nfGetGroupNativeFunctions()
{
	std::map<std::string, FScrptNativeFunc> functions;
	
#define REGISTER_NATIVE_FUNC(cont, func) cont.insert(std::make_pair(std::string(#func), &func))
	
	REGISTER_NATIVE_FUNC(functions, spawn__);
	REGISTER_NATIVE_FUNC(functions, despawn_f_);	
	REGISTER_NATIVE_FUNC(functions, isAlived__f);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPos_ssfff_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPos_ssfff_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPos_ssff_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPos_ssff_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPosMl_ssffff_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPosMl_ssffff_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPosMl_ssfff_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupPosMl_ssfff_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroup_sssf_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroup_sssf_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroup_sss_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroup_sss_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupMl_sssff_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupMl_sssff_);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupMl_sssf_c);
	REGISTER_NATIVE_FUNC(functions, newNpcChildGroupMl_sssf_);
	REGISTER_NATIVE_FUNC(functions, spawnManager_s_);
	REGISTER_NATIVE_FUNC(functions, sitDown__);
	REGISTER_NATIVE_FUNC(functions, standUp__);
	REGISTER_NATIVE_FUNC(functions, despawnManager_s_);
	REGISTER_NATIVE_FUNC(functions, getMidPos__ff);
	REGISTER_NATIVE_FUNC(functions, getGroupTemplateWithFlags_sss_s);
	REGISTER_NATIVE_FUNC(functions, getGroupTemplateWithFlags_ss_s);
	REGISTER_NATIVE_FUNC(functions, getZoneWithFlags_ssss_s);
	REGISTER_NATIVE_FUNC(functions, getZoneWithFlags_sss_s);
	REGISTER_NATIVE_FUNC(functions, getNearestZoneWithFlags_ffsss_s);
	REGISTER_NATIVE_FUNC(functions, getNearestZoneWithFlags_ffss_s);
	REGISTER_NATIVE_FUNC(functions, getNearestZoneWithFlagsStrict_ffsss_s);
	REGISTER_NATIVE_FUNC(functions, getNearestZoneWithFlagsStrict_ffss_s);
	REGISTER_NATIVE_FUNC(functions, getNeighbourZoneWithFlags_ssss_s);
	REGISTER_NATIVE_FUNC(functions, getNeighbourZoneWithFlags_sss_s);
	REGISTER_NATIVE_FUNC(functions, setAggro_ff_);
	REGISTER_NATIVE_FUNC(functions, setCanAggro_f_);
	REGISTER_NATIVE_FUNC(functions, clearAggroList_f_);
	REGISTER_NATIVE_FUNC(functions, clearAggroList__);
	REGISTER_NATIVE_FUNC(functions, setMode_s_);
	REGISTER_NATIVE_FUNC(functions, setAutoSpawn_f_);
	REGISTER_NATIVE_FUNC(functions, setMaxHP_ff_);
	REGISTER_NATIVE_FUNC(functions, setHPLevel_f_);
	REGISTER_NATIVE_FUNC(functions, setHPScale_f_);
	REGISTER_NATIVE_FUNC(functions, scaleHP_f_);
	REGISTER_NATIVE_FUNC(functions, setBotHPScaleByAlias_fs_);
	REGISTER_NATIVE_FUNC(functions, downScaleHP_f_);
	REGISTER_NATIVE_FUNC(functions, upScaleHP_f_);
	REGISTER_NATIVE_FUNC(functions, addHP_f_);
	REGISTER_NATIVE_FUNC(functions, aiAction_s_);
	REGISTER_NATIVE_FUNC(functions, aiActionSelf_s_);
	REGISTER_NATIVE_FUNC(functions, addProfileParameter_s_);
	REGISTER_NATIVE_FUNC(functions, addProfileParameter_ss_);
	REGISTER_NATIVE_FUNC(functions, addProfileParameter_sf_);
	REGISTER_NATIVE_FUNC(functions, removeProfileParameter_s_);
	REGISTER_NATIVE_FUNC(functions, addPersistentProfileParameter_s_);
	REGISTER_NATIVE_FUNC(functions, addPersistentProfileParameter_ss_);
	REGISTER_NATIVE_FUNC(functions, addPersistentProfileParameter_sf_);
	REGISTER_NATIVE_FUNC(functions, removePersistentProfileParameter_s_);
	REGISTER_NATIVE_FUNC(functions, getOutpostStateName__s);
	REGISTER_NATIVE_FUNC(functions, isOutpostTribeOwner__f);
	REGISTER_NATIVE_FUNC(functions, isOutpostGuildOwner__f);
	REGISTER_NATIVE_FUNC(functions, getEventParam_f_f);
	REGISTER_NATIVE_FUNC(functions, getEventParam_f_s);
	REGISTER_NATIVE_FUNC(functions, setSheet_s_);
	REGISTER_NATIVE_FUNC(functions, setClientSheet_s_);
	REGISTER_NATIVE_FUNC(functions, setHealer_f_);
	REGISTER_NATIVE_FUNC(functions, setConditionSuccess_f_);
	REGISTER_NATIVE_FUNC(functions, facing_f_);
	REGISTER_NATIVE_FUNC(functions, setUrl_ss_);

	// Boss functions (custom text)
	REGISTER_NATIVE_FUNC(functions, phraseBegin__);
	REGISTER_NATIVE_FUNC(functions, phrasePushValue_sf_);
	REGISTER_NATIVE_FUNC(functions, phrasePushString_ss_);
	REGISTER_NATIVE_FUNC(functions, phraseEndNpcMsg_fss_);
	REGISTER_NATIVE_FUNC(functions, phraseEndSystemMsg_fss_);
	REGISTER_NATIVE_FUNC(functions, phraseEndEmoteMsg_fs_);

	// Boss functions (Bot infos)
	REGISTER_NATIVE_FUNC(functions, getBotIndex_s_f);
	REGISTER_NATIVE_FUNC(functions, getBotEid_f_s);
	REGISTER_NATIVE_FUNC(functions, getCurrentSpeakerEid__s);
	REGISTER_NATIVE_FUNC(functions, getBotIndexByName_s_f);
	REGISTER_NATIVE_FUNC(functions, isGroupAlived__f);
	REGISTER_NATIVE_FUNC(functions, isBotAlived_f_f);

	// Boss functions (Player infos)
	REGISTER_NATIVE_FUNC(functions, isPlayerAlived_s_f);
	REGISTER_NATIVE_FUNC(functions, getPlayerStat_ss_f);
	REGISTER_NATIVE_FUNC(functions, getPlayerDistance_fs_f);
	REGISTER_NATIVE_FUNC(functions, getCurrentPlayerEid__s);
	REGISTER_NATIVE_FUNC(functions, queryEgs_sscfs_);
	REGISTER_NATIVE_FUNC(functions, queryEgs_ssscfs_);

	// Boss functions (Aggro list)
	REGISTER_NATIVE_FUNC(functions, getCurrentPlayerAggroListTarget_f_s);
	REGISTER_NATIVE_FUNC(functions, getRandomPlayerAggroListTarget_f_s);
	REGISTER_NATIVE_FUNC(functions, getAggroListElement_ff_s);
	REGISTER_NATIVE_FUNC(functions, getAggroListSize_f_f);
	REGISTER_NATIVE_FUNC(functions, setAggroListTarget_fs_);
	REGISTER_NATIVE_FUNC(functions, setGroupAggroListTarget_s_);
	REGISTER_NATIVE_FUNC(functions, setManagerAggroListTarget_ss_);
	
	// Boss functions (Time infos)
	REGISTER_NATIVE_FUNC(functions, getServerTimeStr__s);
	REGISTER_NATIVE_FUNC(functions, getServerTime__s);
	REGISTER_NATIVE_FUNC(functions, getRyzomDateStr__s);
	REGISTER_NATIVE_FUNC(functions, getRyzomDate__s);

	// Boss function (teleport actions)
	REGISTER_NATIVE_FUNC(functions, teleportPlayer_sffff_);
	REGISTER_NATIVE_FUNC(functions, summonPlayer_fs_);


#undef REGISTER_NATIVE_FUNC
	
	return functions;
}
