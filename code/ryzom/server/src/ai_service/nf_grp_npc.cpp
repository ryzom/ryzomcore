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

#include "ai_player.h"

#include "ai_script_data_manager.h"
#include "ais_control.h"

using std::string;
using std::vector;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;

// a big bad global var !
extern CAIEntityPhysical	*TempSpeaker;
extern CBotPlayer			*TempPlayer;

/****************************************************************************/
/* CGroupNpc methods                                                        */
/****************************************************************************/

//----------------------------------------------------------------------------
// setfactionProp
/** @page code

@subsection setFactionProp_ss_
Set a property of the faction profile. Valid values for Property are:
- faction
- ennemyFaction
- friendFaction

There are special faction that are automatically attributed to entities.
Special factions are:
- Player
- Predator
- Famous<faction> where faction is the name of a Ryzom faction with uppercase
initials and without underscores (ie: tribe_beachcombers ->
FamousTribeBeachcombers), it represents players with a positive fame with the
specified faction
- outpost:<id>:<side> where id is the outpost alias as an int, and side is
either attacker ou defender (these faction may see their format change soon
and shouldn't be used without prior discussion with the responsible coder), it
represents players and squads fighting in an outpost conflict

Arguments: s(Property),s(Content) ->
@param Property is the property to modify
@param Content is a '|' seperated list of faction names

@code
()setFactionProp("ennemyFaction", "Player");
@endcode

*/
// CGroupNpc
void setFactionProp_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string params = stack.top();	// ToChange
	stack.pop();
	TStringId const factionType = CStringMapper::map(stack.top());
	stack.pop();
	
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("setFactionProp on a non Npc Group, doesnt work");
		return;
	}
	
	static TStringId factionStrId = CStringMapper::map("faction");
	static TStringId ennemyFactionStrId = CStringMapper::map("ennemyFaction");
	static TStringId friendFactionStrId = CStringMapper::map("friendFaction");
	breakable
	{
		CPropertySetWithExtraList<TAllianceId> tmpSet;	
		CStringSeparator const sep(params,"|");
		while (sep.hasNext())
			tmpSet.addProperty(AITYPES::CPropertyId::create(sep.get()));
		
		if (factionType==factionStrId)
		{
			grpNpc->faction() = tmpSet;
			break;
		}
		if (factionType==ennemyFactionStrId)
		{
			grpNpc->ennemyFaction() = tmpSet;
			break;
		}
		if (factionType==friendFactionStrId)
		{
			grpNpc->friendFaction() = tmpSet;
			break;
		}
		nlwarning("'%s' is not a correct faction name", CStringMapper::unmap(factionType).c_str());
		return;
	}
}

void setOupostMode_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string sideStr = stack.top(); stack.pop();
	std::string aliasStr = stack.top();
	CGroupNpc* const npcGroup = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if ( ! npcGroup)
	{
		nlwarning("setOutpostMode on a non Npc Group, doesnt work");
		return;
	}

	OUTPOSTENUMS::TPVPSide side;
	if (sideStr == "attacker")
	{
		side = OUTPOSTENUMS::OutpostAttacker;
	}
	else if (sideStr == "owner")
	{
		side = OUTPOSTENUMS::OutpostOwner;
	}
	else
	{
		nlwarning("setOutpostMode: invalid side");
	}
	
	npcGroup->setOutpostSide(side);
	npcGroup->setOutpostFactions(side);
	FOREACH(botIt, CCont<CBot>, npcGroup->bots())
	{
		CBot* bot = *botIt;
		CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
		if (botNpc)
		{
			CSpawnBotNpc* spawnBotNpc = botNpc->getSpawn();
			if (spawnBotNpc)
			{
				spawnBotNpc->setOutpostSide(side);
				spawnBotNpc->setOutpostAlias(LigoConfig.aliasFromString(aliasStr));
			}
		}
	}
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
/** @page code

@subsection moveToZone_ss_
Moves the current group from one zone to another.

Arguments: s(From), s(To) ->
@param[in] From is a zone name
@param[in] To is a zone name

@code
()moveToZone($zone1, $zone2); // Move the current group from $zone1 to $zone2
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void moveToZone_ss_(CStateInstance* entity, CScriptStack& stack)
{
	TStringId const zoneDest = CStringMapper::map(stack.top());
	stack.pop();
	TStringId const zoneSrc = CStringMapper::map(stack.top());
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (!entity)
	{
		nlwarning("moveToZone failed: no state instance!");
		nlwarning(" - from %s", CStringMapper::unmap(zoneSrc).c_str());
		nlwarning(" - to %s", CStringMapper::unmap(zoneDest).c_str());
		return;
	}

	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{
		nlwarning("moveToZone failed: no NPC group!");
		nlwarning(" - from %s", CStringMapper::unmap(zoneSrc).c_str());
		nlwarning(" - to %s", CStringMapper::unmap(zoneDest).c_str());
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{
		nlwarning("moveToZone failed: no spawned group!");
		nlwarning(" - from %s", CStringMapper::unmap(zoneSrc).c_str());
		nlwarning(" - to %s", CStringMapper::unmap(zoneDest).c_str());
		return;
	}
	
	CNpcZone const* const destZone = aiInstance->getZone(zoneDest);
	CNpcZone const* const srcZone = aiInstance->getZone(zoneSrc);
	
	if (!destZone)
	{
		nlwarning("moveToZone: getZone destination failed!");
		nlwarning(" - from %s", CStringMapper::unmap(zoneSrc).c_str());
		nlwarning(" - to %s", CStringMapper::unmap(zoneDest).c_str());
	}
	if (!srcZone)
	{
		nlwarning("moveToZone: getZone source failed!");
		nlwarning(" - from %s", CStringMapper::unmap(zoneSrc).c_str());
		nlwarning(" - to %s", CStringMapper::unmap(zoneDest).c_str());
	}
	if (!destZone || !srcZone)
		return;
	
	if (destZone==srcZone)
	{
		nlwarning("Trying to find a path between two same zones in %s, aborting moveToZone", entity->getActiveState()->getAliasFullName().c_str());
		nlwarning(" - from %s", CStringMapper::unmap(zoneSrc).c_str());
		nlwarning(" - to %s", CStringMapper::unmap(zoneDest).c_str());
		return;
	}
	
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileDynFollowPath(spawnGroup, srcZone, destZone, AITYPES::CPropertySet()));
}

//----------------------------------------------------------------------------
/** @page code

@subsection setActivity_s_
Changes the activity of the group. Valid activities are:
- "no_change"
- "escorted"
- "guard"
- "guard_escorted"
- "normal"
- "faction"
- "faction_no_assist"
- "bandit"

Arguments: s(Activity) ->
@param[in] Activity is an activity name the group will take

@code
()setActivity("bandit"); // Gives a bandit activity to the group
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void setActivity_s_(CStateInstance* entity, CScriptStack& stack)
{
	string activity = stack.top();
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (!entity) { nlwarning("setActivity failed!"); return; }
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{	nlwarning("setActivity failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{	nlwarning("setActivity failed: no spawned group");
		return;
	}
	
	breakable
	{
		if (activity=="no_change")
			break;
		
		if (activity=="escorted")
		{
			spawnGroup->activityProfile().setAIProfile(new CGrpProfileEscorted(spawnGroup));
			break;
		}
		if (activity=="guard")
		{
			spawnGroup->activityProfile().setAIProfile(new CGrpProfileGuard(spawnGroup));
			break;
		}
		if (activity=="guard_escorted")
		{
			spawnGroup->activityProfile().setAIProfile(new CGrpProfileGuardEscorted(spawnGroup));
			break;
		}

		if (activity=="normal")
		{
			spawnGroup->activityProfile().setAIProfile(new CGrpProfileNormal(spawnGroup));
			break;
		}
		if (activity=="faction")
		{
			spawnGroup->activityProfile().setAIProfile(new CGrpProfileFaction(spawnGroup));
			break;
		}
		if (activity=="faction_no_assist")
		{
			CGrpProfileFaction * grpPro = new CGrpProfileFaction(spawnGroup);
			grpPro->noAssist();
			spawnGroup->activityProfile().setAIProfile(grpPro);
			break;
		}
		if (activity=="bandit")
		{
			spawnGroup->activityProfile().setAIProfile(new CGrpProfileBandit(spawnGroup));
			break;
		}

		nlwarning("trying to set activity profile to an unknown profile name");
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection waitInZone_s_
Makes the group wander in the specified zone. It's useful to prevent the group
from looping previous movement profile).

Arguments: s(Zone) ->
@param[in] Zone is a zone name

@code
()waitInZone($zone); // Makes the group wander in $zone
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void waitInZone_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string zoneName = stack.top();
	TStringId const zoneDest = CStringMapper::map(zoneName);
	stack.pop();
	
	if (!entity)
	{
		nlwarning("waitInZone failed!");
		return;
	}
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
	{
		// :TODO: Check is that warning was a flood
		//nlwarning("waitInZone failed!");
		return;
	}
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{
		nlwarning("waitInZone failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{
		nlwarning("waitInZone failed: no spawned group");
		return;
	}
	
	CNpcZone const* const destZone = aiInstance->getZone(zoneDest);
	
	if (!destZone)
	{
		nlwarning("waitInZone: getZone destination failed! (%s)", zoneName.c_str());
		return;
	}
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileWander(spawnGroup, destZone));
}

//----------------------------------------------------------------------------
/** @page code

@subsection stopMoving__
Makes the group stop moving and stand at its current position.

Arguments: ->

@code
()stopMoving(); // Makes the group stop moving
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void stopMoving__(CStateInstance* entity, CScriptStack& stack)
{
	if (!entity)
	{
		nlwarning("stopMoving failed!");
		return;
	}
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{
		nlwarning("stopMoving failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{
		nlwarning("stopMoving failed: no spawned group");
		return;
	}
	
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileIdle(spawnGroup));
}

//----------------------------------------------------------------------------
/** @page code

@subsection startWander_f_
Set activity to wander in current pos:

Arguments: f(Radius) ->
@param[in] Radius dispersion of wander activity

@code
()startWander(100); // Gives a wander activity to the group with dispersion of 100
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void startWander_f_(CStateInstance* entity, CScriptStack& stack)
{
	uint32 dispersionRadius = (uint32)(float&)stack.top();
	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (!entity) { nlwarning("setActivity failed!"); return; }
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{	nlwarning("startWander failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{	nlwarning("startWander failed: no spawned group");
		return;
	}

	CAIVector centerPos;
	if	(!spawnGroup->calcCenterPos(centerPos))	// true if there's some bots in the group.
	{	nlwarning("startWander failed: no center pos");
		return;
	}

	NLMISC::CSmartPtr<CNpcZonePlaceNoPrim> destZone = NLMISC::CSmartPtr<CNpcZonePlaceNoPrim>(new CNpcZonePlaceNoPrim());
	destZone->setPosAndRadius(AITYPES::vp_auto, CAIPos(centerPos, 0, 0), (uint32)(dispersionRadius*1000.));
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileWanderNoPrim(spawnGroup, destZone));
}

//----------------------------------------------------------------------------
/** @page code

@subsection startMoving_fff_
Set activity to wander in current pos:

Arguments: f(Radius) ->
@param[in] Radius dispersion of wander activity

@code
()startMoving(100,-100,10); // Moves the group to 100,-100 with radius of 10
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void startMoving_fff_(CStateInstance* entity, CScriptStack& stack)
{
	uint32 dispersionRadius = (uint32)(float&)stack.top();
	stack.pop();
	float const y = (float&)stack.top();
	stack.pop();
	float const x = (float&)stack.top();
 	stack.pop();
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (!entity) { nlwarning("setActivity failed!"); return; }
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{	nlwarning("setActivity failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{	nlwarning("setActivity failed: no spawned group");
		return;
	}

	NLMISC::CSmartPtr<CNpcZonePlaceNoPrim> destZone = NLMISC::CSmartPtr<CNpcZonePlaceNoPrim>(new CNpcZonePlaceNoPrim());
	destZone->setPosAndRadius(AITYPES::vp_auto, CAIPos(CAIVector(x, y), 0, 0), (uint32)(dispersionRadius*1000.));
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileWanderNoPrim(spawnGroup, destZone));

	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection followPlayer_sf_
Set activity to follow the given player

Arguments: s(PlayerEid) f(Radius) ->
@param[in] PlayerEid id of player to follow
@param[in] Radius dispersion of wander activity

@code
()followPlayer("(0x0002015bb4:01:88:88)",10);
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void followPlayer_sf_(CStateInstance* entity, CScriptStack& stack)
{
	uint32 dispersionRadius = (uint32)(float&)stack.top(); stack.pop();
	NLMISC::CEntityId playerId = NLMISC::CEntityId((std::string)stack.top());
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	if (!entity) { nlwarning("followPlayer failed!"); return; }
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{	nlwarning("followPlayer failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{	nlwarning("followPlayer failed: no spawned group");
		return;
	}

	if (playerId == CEntityId::Unknown)
	{
		nlwarning("followPlayer failed: unknown player");
		DEBUG_STOP;
		return;
	}

	spawnGroup->movingProfile().setAIProfile(new CGrpProfileFollowPlayer(spawnGroup, TheDataset.getDataSetRow(playerId), dispersionRadius));

	return;
}


//----------------------------------------------------------------------------
/** @page code

@subsection wander__
Makes the group wander in the current npc state zone.

Arguments: ->

@code
()wander(); // Makes the group wander
@endcode

*/
// Spawned CGroupNpc not in a family behaviour
void wander__(CStateInstance* entity, CScriptStack& stack)
{
	if (!entity)
	{
		nlwarning("wander failed!");
		return;
	}
	
	IManagerParent* const managerParent = entity->getGroup()->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CGroupNpc* group = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!group)
	{
		nlwarning("wander failed: no NPC group");
		return;
	}
	CSpawnGroupNpc* spawnGroup = group->getSpawnObj();
	if (!spawnGroup)
	{
		nlwarning("wander failed: no spawned group");
		return;
	}
	
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileWander(spawnGroup));
}

//----------------------------------------------------------------------------
/** @page code

@subsection setAttackable_f_
Sets the group as being attackable (or not) by bots and players. 0 means not
attackable, 1 means attackable.

Arguments: f(Attackable) ->
@param[in] Attackable tells whether the group is attackable

@code
()setAttackable(1); // Make the group attackable by players and bots
@endcode

*/
// CGroupNpc
void setAttackable_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool const attackable = (float&)stack.top()!=0.0f;
	stack.pop();
	
	CGroup* group = entity->getGroup();
	
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	npcGroup->setPlayerAttackable(attackable);
	npcGroup->setBotAttackable(attackable);
	
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
/** @page code

@subsection setPlayerAttackable_f_
Sets the group as being attackable (or not) by players. 0 means not
attackable, 1 means attackable.

Arguments: f(Attackable) ->
@param[in] Attackable tells whether the group is attackable

@code
()setPlayerAttackable(1); // Make the group attackable by players
@endcode

*/
// CGroupNpc
void setPlayerAttackable_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool attackable = (float&)stack.top()!=0.0f;
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	npcGroup->setPlayerAttackable(attackable);
	
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
// setBotAttackable_f_
// Arguments: f(Attackable) ->
/** @page code

@subsection setBotAttackable_f_
Sets the group as being attackable (or not) by bots. 0 means not attackable, 1
means attackable.

Arguments: f(Attackable) ->
@param[in] Attackable tells whether the group is attackable

@code
()setBotAttackable(0); // Make the group not attackable by bots
@endcode

*/
// CGroupNpc
void setBotAttackable_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool attackable = (float&)stack.top()!=0.0f;
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	npcGroup->setBotAttackable(attackable);
	
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
// setFactionAttackableAbove_sff_
// Arguments: s(Faction),f(Threshold),f(Attackable) ->
/** @page code

@subsection setFactionAttackableAbove_sff_
Sets the group as being attackable (or not) by players having their fame
matching the specified condition. If player fame for Faction is above
specified Threshold the bot will be attackable by that player or not depending
on Attackable parameter, 0 meaning not attackable, 1 meaning attackable.

Note: Bots must not be player attackable for the faction to be taken into
account.

Arguments: s(Faction),f(Threshold),f(Attackable) ->
@param[in] Faction tells the faction against which player fame will be tested
@param[in] Threshold is the test threshold above which player will be able to attack
@param[in] Attackable tells whether the group is attackable or not

@code
()setFactionAttackableAbove("TribeNightTurners", 0, 1); // Make the group attackable by players with positive tribe_night_turners fame
@endcode

*/
// CGroupNpc
void setFactionAttackableAbove_sff_(CStateInstance* entity, CScriptStack& stack)
{
	bool attackable = (float&)stack.top()!=0.0f;
	stack.pop();
	sint32 threshold = (sint32)(float&)stack.top();
	stack.pop();
	std::string faction = (std::string&)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	npcGroup->setFactionAttackableAbove(faction, threshold, attackable);
	
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
// setFactionAttackableBelow_sff_
// Arguments: s(Faction),f(Threshold),f(Attackable) ->
/** @page code

@subsection setFactionAttackableBelow_sff_
Sets the group as being attackable (or not) by players having their fame
matching the specified condition. If player fame for Faction is below
specified Threshold the bot will be attackable by that player or not depending
on Attackable parameter, 0 meaning not attackable, 1 meaning attackable.

Note: Bots must not be player attackable for the faction to be taken into
account.

Arguments: s(Faction),f(Threshold),f(Attackable) ->
@param[in] Faction tells the faction against which player fame will be tested
@param[in] Threshold is the test threshold below which player will be able to attack
@param[in] Attackable tells whether the group is attackable or not

@code
()setFactionAttackableBelow("TribeMatisianBorderGuards", 0, 1); // Make the group attackable by players with negative tribe_matisian_border_guards fame
@endcode

*/
// CGroupNpc
void setFactionAttackableBelow_sff_(CStateInstance* entity, CScriptStack& stack)
{
	bool attackable = (float&)stack.top()!=0.0f;
	stack.pop();
	sint32 threshold = (sint32)(float&)stack.top();
	stack.pop();
	std::string faction = (std::string&)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	npcGroup->setFactionAttackableBelow(faction, threshold, attackable);
	
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
/** @page code

@subsection addBotChat_s_
Add an entry in the botchat menu of every bot of the group. Specified text ID
can be created dynamically with setSimplePhrase (@ref setSimplePhrase_ss_).

Arguments: s(BotChat) ->
@param[in] BotChat is a text ID

@code
()addBotChat("menu:QUESTION:REPONSE");
@endcode

*/
// CGroupNpc
void addBotChat_s_(CStateInstance* entity, CScriptStack& stack)
{
	string const botChat = (string)stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	FOREACH(botIt, CCont<CBot>,	group->bots())
	{
		CBot* bot = *botIt;
		CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
		if (botNpc)
		{
			if (!botNpc->getChat())
				botNpc->newChat();
			botNpc->getChat()->add(botNpc->getAIInstance(), botChat);
			CSpawnBotNpc* spawnBotNpc = botNpc->getSpawn();
			if (spawnBotNpc)
			{
				spawnBotNpc->setCurrentChatProfile(botNpc->getChat());
			}
		}
	}
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
/** @page code

@subsection clearBotChat__
Removes all entries from the botchat menu of every bot of the group.

Arguments: ->

@code
()clearBotChat();
@endcode

*/
// CGroupNpc
void clearBotChat__(CStateInstance* entity, CScriptStack& stack)
{
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	FOREACH(botIt, CCont<CBot>,	group->bots())
	{
		CBot* bot = *botIt;
		CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
		if (botNpc)
		{
			if (!botNpc->getChat())
				botNpc->newChat();
			botNpc->getChat()->clear();
			CSpawnBotNpc* spawnBotNpc = botNpc->getSpawn();
			if (spawnBotNpc)
			{
				spawnBotNpc->setCurrentChatProfile(botNpc->getChat());
			}
		}
	}
	if (npcGroup->isSpawned())
		npcGroup->getSpawnObj()->sendInfoToEGS();
}

//----------------------------------------------------------------------------
/** @page code

@subsection ignoreOffensiveActions_f_
Make the bots of the group ignore offensive actions issued on them.

Arguments: f(IgnoreOffensiveActions) ->
@param[in] IgnoreOffensiveActions is tells whethere to ignore offensive actions (1) or not (0)

@code
()ignoreOffensiveActions(1);
@endcode

*/
// CGroup
void ignoreOffensiveActions_f_(CStateInstance* entity, CScriptStack& stack)
{
	bool const ignoreOffensiveActions = ((float)stack.top())!=0.f;
	stack.pop();
	
	CGroup* group = entity->getGroup();
	FOREACH(botIt, CCont<CBot>,	group->bots())
	{
		CBot* bot = *botIt;
		if (bot)
			bot->setIgnoreOffensiveActions(ignoreOffensiveActions);
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection setDespawnTime_f_
Sets the time before current group being despawned.

Arguments: f(DespawnTime) ->
@param[in] DespawnTime is the despawn time in ticks (-1 will set "pseudo-infinite")

@code
()setDespawnTime(80);
()setDespawnTime(-1);
@endcode

*/
// CGroupNpc
void setDespawnTime_f_(CStateInstance* entity, CScriptStack& stack)
{
	float time = stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	uint32 despawntime = (uint32)(sint32)time; // -1 => ~0
	// TODO:kxu: fix CAITimer!
	if (despawntime > 0x7fffffff)
		despawntime = 0x7fffffff; // AI timers do not treat properly delta times greater than the max signed int
	npcGroup->despawnTime() = despawntime;
}


/** @page code

@subsection despawnBotByAlias_s_
Despawn a specific Bot by its alias

Arguments:  f(alias), ->
@param[in] alias is the alias of the bot


@code
()despawnBotByAlias('(A:1000:10560)');
@endcode

*/
// CGroup
void despawnBotByAlias_s_(CStateInstance* entity, CScriptStack& stack)
{
	uint32 alias =  LigoConfig.aliasFromString((string)stack.top()); stack.pop();
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	FOREACH(botIt, CCont<CBot>,	group->bots())
	{

		CBot* bot = *botIt;
		
		if (bot->getAlias() != alias) { continue; }
		if (!bot->isSpawned()) return;	
		if (bot->getRyzomType() == RYZOMID::npc)
		{
			CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
			CSpawnBotNpc* spawnBotNpc = botNpc->getSpawn();
			spawnBotNpc->spawnGrp().addBotToDespawnAndRespawnTime(&spawnBotNpc->getPersistent(), 1, spawnBotNpc->spawnGrp().getPersistent().respawnTime());				
		}

	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection setRespawnTime_f_
Sets the time in game cycles before current group being respawned.

Arguments: f(RespawnTime) ->
@param[in] RespawnTime is the respawn time in ticks (-1 will set "pseudo-infinite")

@code
()setRespawnTime(80);
()setRespawnTime(-1);
@endcode

*/
// CGroupNpc
void setRespawnTime_f_(CStateInstance* entity, CScriptStack& stack)
{
	float const time = stack.top();
	stack.pop();
	
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);

	uint32 respawntime = (uint32)(sint32)time; // -1 => ~0
	// TODO:kxu: fix CAITimer!
	if (respawntime > 0x7fffffff)
		respawntime = 0x7fffffff; // AI timers do not treat properly delta times greater than the max signed int
	npcGroup->respawnTime() = respawntime;
}

//----------------------------------------------------------------------------
/** @page code

@subsection addHpUpTrigger_ff_
Registers a trigger on HP increases. Whenever the HP level of a bot upcross
the threshold it triggers the specified user event. Several triggers can be
registered on the same group, even with the same threshold and event.

Arguments: f(threshold),f(user_event_n) ->
@param[in] threshold is a HP threshold
@param[in] user_event_n is the user event to trigger

@code
()addHpUpTrigger(0.5, 4);
@endcode

*/
// CGroupNpc
void addHpUpTrigger_ff_(CStateInstance* entity, CScriptStack& stack)
{
	int eventId = (int)(float)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to add a hp up trigger (%f) listener (user event %d) in a group which is not an NPC group.", threshold, eventId);
		return;
	}
	grpNpc->addHpUpTrigger(threshold, eventId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delHpUpTrigger_ff_
Unregisters a trigger on HP increases. The same values used when registering
the trigger must be passed. If several triggers were defined with the same
parameters only one is removed.

Arguments: f(threshold),f(user_event_n) ->
@param[in] threshold is a HP threshold
@param[in] user_event_n is the user event to trigger

@code
()delHpUpTrigger(0.5, 4);
@endcode

*/
// CGroupNpc
void delHpUpTrigger_ff_(CStateInstance* entity, CScriptStack& stack)
{
	int eventId = (int)(float)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to delete a hp up trigger (%f) listener (user event %d) in a group which is not an NPC group.", threshold, eventId);
		return;
	}
	grpNpc->delHpUpTrigger(threshold, eventId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addHpDownTrigger_ff_
Registers a trigger on HP decreases. Whenever the HP level of a bot downcross
the threshold it triggers the specified user event. Several triggers can be
registered on the same group, even with the same threshold and event.

Arguments: f(threshold),f(user_event_n) ->
@param[in] threshold is a HP threshold
@param[in] user_event_n is the user event to trigger

@code
()addHpDownTrigger(0.5, 5);
@endcode

*/
// CGroupNpc
void addHpDownTrigger_ff_(CStateInstance* entity, CScriptStack& stack)
{
	int eventId = (int)(float)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to add a hp down trigger (%f) listener (user event %d) in a group which is not an NPC group.", threshold, eventId);
		return;
	}
	grpNpc->addHpDownTrigger(threshold, eventId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delHpDownTrigger_ff_
Unregisters a trigger on HP decreases. The same values used when registering
the trigger must be passed. If several triggers were defined with the same
parameters only one is removed.

Arguments: f(threshold),f(user_event_n) ->
@param[in] threshold is a HP threshold
@param[in] user_event_n is the user event to trigger

@code
()delHpDownTrigger(0.5, 5);
@endcode

*/
// CGroupNpc
void delHpDownTrigger_ff_(CStateInstance* entity, CScriptStack& stack)
{
	int eventId = (int)(float)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to delete a hp down trigger (%f) listener (user event %d) in a group which is not an NPC group.", threshold, eventId);
		return;
	}
	grpNpc->delHpDownTrigger(threshold, eventId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addHpUpTrigger_fs_
@sa @ref addHpUpTrigger_ff_

These triggers call a script function instead of trigger a user event.

Arguments: f(threshold),s(callback) ->
@param[in] threshold is a HP threshold
@param[in] callback is the script callback to trigger

@code
()addHpUpTrigger(0.5, "onHPIncrease");
@endcode

*/
// CGroupNpc
void addHpUpTrigger_fs_(CStateInstance* entity, CScriptStack& stack)
{
	std::string cbFunc = (std::string)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to add a hp up trigger (%f) listener (%s) in a group which is not an NPC group.", threshold, cbFunc.c_str());
		return;
	}
	grpNpc->addHpUpTrigger(threshold, cbFunc);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delHpUpTrigger_fs_
@sa @ref delHpUpTrigger_ff_

This function is used to remove script function triggers.

Arguments: f(threshold),s(callback) ->
@param[in] threshold is a HP threshold
@param[in] callback is the script callback to trigger

@code
()delHpUpTrigger(0.5, "onHPIncrease");
@endcode

*/
// CGroupNpc
void delHpUpTrigger_fs_(CStateInstance* entity, CScriptStack& stack)
{
	std::string cbFunc = (std::string)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to delete a hp up trigger (%f) listener (%s) in a group which is not an NPC group.", threshold, cbFunc.c_str());
		return;
	}
	grpNpc->delHpUpTrigger(threshold, cbFunc);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addHpDownTrigger_fs_
@sa @ref addHpDownTrigger_ff_

These triggers call a script function instead of trigger a user event.

Arguments: f(threshold),s(callback) ->
@param[in] threshold is a HP threshold
@param[in] callback is the script callback to trigger

@code
()addHpDownTrigger(0.5, "onHPDecrease");
@endcode

*/
// CGroupNpc
void addHpDownTrigger_fs_(CStateInstance* entity, CScriptStack& stack)
{
	std::string cbFunc = (std::string)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to add a hp down trigger (%f) listener (%s) in a group which is not an NPC group.", threshold, cbFunc.c_str());
		return;
	}
	grpNpc->addHpDownTrigger(threshold, cbFunc);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delHpDownTrigger_fs_
@sa @ref delHpDownTrigger_ff_

This function is used to remove script function triggers.

Arguments: f(threshold),s(callback) ->
@param[in] threshold is a HP threshold
@param[in] callback is the script callback to trigger

@code
()delHpDownTrigger(0.5, "onHPDecrease");
@endcode

*/
// CGroupNpc
void delHpDownTrigger_fs_(CStateInstance* entity, CScriptStack& stack)
{
	std::string cbFunc = (std::string)stack.top();
	stack.pop();
	float threshold = (float)stack.top();
	stack.pop();
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to delete a hp down trigger (%f) listener (%s) in a group which is not an NPC group.", threshold, cbFunc.c_str());
		return;
	}
	grpNpc->delHpDownTrigger(threshold, cbFunc);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addNamedEntityListener_ssf_
Associates a listeners with a named entity property. Whenever that field
changes the specified user event is triggered. Valid property names are:
- state
- param1
- param2
Name of the entity cannot be listened, because it cannot change. Several
listeners (even with the same parameters) can be associated to each property.

Arguments: s(name),s(prop),f(event) ->
@param[in] name is a the name of the named entity to listen
@param[in] prop is a the property of the named entity to listen
@param[in] event is a the user event to trigger

@code
()addNamedEntityListener("Invasion", "state", 6);
@endcode

*/
// CGroupNpc
void addNamedEntityListener_ssf_(CStateInstance* entity, CScriptStack& stack)
{
	int event = (int)(float)stack.top();
	stack.pop();
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to create a named entity (%s:%s) listener (user event %d) in a group which is not an NPC group.", name.c_str(), prop.c_str(), event);
		return;
	}
	grpNpc->addNamedEntityListener(name, prop, event);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delNamedEntityListener_ssf_
Removes a listener from a named entity property. If several listeners with the
same parameters are registered, only one is removed.

Arguments: s(name),s(prop),f(event) ->
@param[in] name is a the name of the named entity to listen
@param[in] prop is a the property of the named entity to listen
@param[in] event is a the user event to trigger

@code
()delNamedEntityListener("Invasion", "state", 6);
@endcode

*/
// CGroupNpc
void delNamedEntityListener_ssf_(CStateInstance* entity, CScriptStack& stack)
{
	int event = (int)(float)stack.top();
	stack.pop();
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to delete a named entity (%s:%s) listener (user event %d) in a group which is not an NPC group.", name.c_str(), prop.c_str(), event);
		return;
	}
	grpNpc->delNamedEntityListener(name, prop, event);
}

//----------------------------------------------------------------------------
/** @page code

@subsection addNamedEntityListener_sss_
@sa @ref addNamedEntityListener_ssf_

These listeners call a script function instead of triggering a user event.

Arguments: s(name),s(prop),s(cbFunc) ->
@param[in] name is a the name of the named entity to listen
@param[in] prop is a the property of the named entity to listen
@param[in] cbFunc is a the callback to trigger

@code
()addNamedEntityListener("Invasion", "state", "onStateChange");
@endcode

*/
// CGroupNpc
void addNamedEntityListener_sss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string cbFunc = (std::string)stack.top();
	stack.pop();
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to create a named entity (%s:%s) listener (%s) in a group which is not an NPC group.", name.c_str(), prop.c_str(), cbFunc.c_str());
		return;
	}
	grpNpc->addNamedEntityListener(name, prop, cbFunc);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delNamedEntityListener_sss_
@sa @ref delNamedEntityListener_ssf_

This function removes script function listeners.

Arguments: s(name),s(prop),s(cbFunc) ->
@param[in] name is a the name of the named entity to listen
@param[in] prop is a the property of the named entity to listen
@param[in] cbFunc is a the callback to trigger

@code
()delNamedEntityListener("Invasion", "state", "onStateChange");
@endcode

*/
// CGroupNpc
void delNamedEntityListener_sss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string cbFunc = (std::string)stack.top();
	stack.pop();
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	
	CGroupNpc* const grpNpc = dynamic_cast<CGroupNpc*>(entity->getGroup());
	if (!grpNpc)
	{
		nlwarning("Trying to delete a named entity (%s:%s) listener (%s) in a group which is not an NPC group.", name.c_str(), prop.c_str(), cbFunc.c_str());
		return;
	}
	grpNpc->delNamedEntityListener(name, prop, cbFunc);
}

//----------------------------------------------------------------------------
/** @page code

@subsection setPlayerController_ss_

Make a player control a npc.

Arguments: s(botId),s(playerId) ->
@param[in] botId is the entity id of the bot the player will control
@param[in] playerId is the entity id of the player that will control the bot

@code
()setPlayerController("(0x0002015bb4:01:88:88)", "(0x0000004880:00:00:00)");
@endcode

*/
// CSpawnBotNpc
void setPlayerController_ss_(CStateInstance* entity, CScriptStack& stack)
{
	NLMISC::CEntityId playerId = NLMISC::CEntityId((std::string)stack.top());
	stack.pop();
	NLMISC::CEntityId botId = NLMISC::CEntityId((std::string)stack.top());
	stack.pop();
	if (botId!=NLMISC::CEntityId::Unknown)
	{
		CGroup* grp = NULL;
		CSpawnBotNpc* bot = NULL;
		CAIEntityPhysicalLocator *inst = CAIEntityPhysicalLocator::getInstance();
		if (inst)
		{
			CAIEntityPhysical *botEntity = inst->getEntity(botId);
			if (botEntity)
			{
				if ((bot = dynamic_cast<CSpawnBotNpc*>(botEntity))
					&& (&bot->getPersistent().getGroup())==entity->getGroup())
				{
					CBotPlayer* player = NULL;
					if (playerId!=NLMISC::CEntityId::Unknown
						&& (player = dynamic_cast<CBotPlayer*>(CAIEntityPhysicalLocator::getInstance()->getEntity(playerId))))
					{
						bot->setPlayerController(player);
					}
					else
						bot->setPlayerController(NULL);
				}
			}
			else
				nlwarning("Bot entity not found");
		}
		else
			nlwarning("Instance not found");
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection clearPlayerController_s_

Stop the control of a npc by a player.

Arguments: s(botId) ->
@param[in] botId is the entity id of the bot

@code
()clearPlayerController("(0x0002015bb4:01:88:88)");
@endcode

*/
// CSpawnBotNpc
void clearPlayerController_s_(CStateInstance* entity, CScriptStack& stack)
{
	NLMISC::CEntityId botId = NLMISC::CEntityId((std::string)stack.top());
	stack.pop();
	if (botId!=NLMISC::CEntityId::Unknown)
	{
		CGroup* grp = NULL;
		CSpawnBotNpc* bot = NULL;
		if ((bot = dynamic_cast<CSpawnBotNpc*>(CAIEntityPhysicalLocator::getInstance()->getEntity(botId)))
			&& (&bot->getPersistent().getGroup())==entity->getGroup())
		{
			bot->setPlayerController(NULL);
		}
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection activateEasterEgg_fffsffffsss_

Call the EGS function CCharacterControl::activateEasterEgg

Arguments: f(easterEggId), f(scenarioId),  f(actId), s(items), f(x), f(y), f(z),f(heading), f(groupname), f(name), f(clientSheet)->
@param[in] items is the sheet/quantity vector (a string of format "item1:qty1;item2:qty2;...")

@code
()activateEasterEgg(2, 1601, 4, "toto.sitem:2;tata.sitem:1;titi.sitem:3", 1247, 4627, 0, 0);
@endcode

*/
void activateEasterEgg_fffsffffsss_(CStateInstance* si, CScriptStack& stack)
{
	std::string clientSheet = (std::string)stack.top(); stack.pop();
	std::string name = (std::string)stack.top(); stack.pop();
	std::string grpCtrl = (std::string)stack.top(); stack.pop();
	float heading = (float)stack.top(); stack.pop();
	float z = (float)stack.top(); stack.pop();
	float y = (float)stack.top(); stack.pop();
	float x = (float)stack.top(); stack.pop();
	
	string items = (string)stack.top(); stack.pop();
	uint32 actId = static_cast<uint32>( (float)stack.top()); stack.pop();
	TSessionId scenarioId( static_cast<uint32>( (float)stack.top())); stack.pop();
	uint32 easterEgg = static_cast<uint32>( (float)stack.top()); stack.pop();

	IAisControl::getInstance()->activateEasterEgg(easterEgg, scenarioId, actId, items, x, y, z, heading, grpCtrl, name, clientSheet);
}

//----------------------------------------------------------------------------
/** @page code

@subsection deactivateEasterEgg_fff_

Call the EGS function CCharacterControl::deactivateEasterEgg

Arguments: f(easterEggId), f(scenarioId), f(actId) ->

@code
()deactivateEasterEgg(0, 4);
@endcode

*/
void deactivateEasterEgg_fff_(CStateInstance* si, CScriptStack& stack)
{
	uint32 actId = static_cast<uint32>( (float)stack.top()); stack.pop();
	TSessionId scenarioId( static_cast<uint32>( (float)stack.top())); stack.pop();
	uint32 easterEgg = static_cast<uint32>( (float)stack.top()); stack.pop();

	IAisControl::getInstance()->deactivateEasterEgg(easterEgg, scenarioId, actId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection receiveMissionItems_ssc_

The npc will ask mission items to the targeter player.

A new entry of the npc contextual menu will propose to the targeter player
to give mission items to the npc if he has the requested items.

Then user events are triggered on the group to inform it about what happens:
- user_event_1: triggered if the player has the requested mission items.
- user_event_2: triggered if the player hasn't the requested mission items.
- user_event_3: triggered after the player has given the mission items to the npc.

Warning: this function can only be called after the event "player_target_npc".
Warning: only works on an R2 shard for R2 plot items.

Arguments: s(missionItems), s(missionText), c(groupToNotify) ->
@param[in] missionItems is the list of mission items, the string format is "item1:qty1;item2:qty2;...".
@param[in] missionText is the text which will appear in the npc contextual menu.
@param[in] groupToNotify is the npc group which will receive the user events.

@code
(@groupToNotify)group_name.context();
()receiveMissionItems("toto.sitem:2;tata.sitem:1;titi.sitem:3", "Mission text", @groupToNotify);
@endcode

*/
// CSpawnBotNpc
void receiveMissionItems_ssc_(CStateInstance* entity, CScriptStack& stack)
{
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() );
	stack.pop();
	string missionText = (string)stack.top();
	stack.pop();
	string missionItems = (string)stack.top();
	stack.pop();

	if (groupToNotify == NULL)
	{
		nlwarning("receiveMissionItems failed: groupToNotify is NULL");
		DEBUG_STOP;
		return;
	}

	if (missionItems.empty())
	{
		nlwarning("receiveMissionItems failed: missionItems is empty");
		DEBUG_STOP;
		return;
	}

	nlassert(entity != NULL);
	CGroup* const group = entity->getGroup();
	IManagerParent* const managerParent = group->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("receiveMissionItems failed: the AI instance of the entity is NULL");
		DEBUG_STOP;
		return;
	}

	CBotPlayer* const talkingPlayer = TempPlayer;
	CSpawnBotNpc* const talkingNpc = dynamic_cast<CSpawnBotNpc*>(TempSpeaker);
	if	(	talkingPlayer == NULL
		||	talkingNpc == NULL
		||	talkingNpc->getPersistent().getOwner() != group) // check if the talking npc is in this group
	{
		nlwarning("receiveMissionItems failed: invalid interlocutors");
		DEBUG_STOP;
		return;
	}

	// do nothing if one of them is dead
	if (!talkingPlayer->isAlive() || !talkingNpc->isAlive())
	{
		return;
	}

	// turn the npc to face the player
	talkingNpc->setTheta(talkingNpc->pos().angleTo(talkingPlayer->pos()));

	// send the request to the EGS
	CGiveItemRequestMsg msg;
	msg.InstanceId = aiInstance->getInstanceNumber();
	msg.GroupAlias = groupToNotify->getAlias();
	msg.CharacterRowId = talkingPlayer->dataSetRow();
	msg.CreatureRowId = talkingNpc->dataSetRow();
	msg.MissionText = missionText;

	// extract items and quantities from the missionItems string
	std::vector<std::string> itemAndQtyList;
	NLMISC::splitString(missionItems, ";", itemAndQtyList);
	if (itemAndQtyList.empty())
	{
		nlwarning("receiveMissionItems failed: the provided mission items string is invalid");
		DEBUG_STOP;
		return;
	}
	FOREACHC(it, std::vector<std::string>, itemAndQtyList)
	{
		std::vector<std::string> itemAndQty;
		NLMISC::splitString(*it, ":", itemAndQty);
		if (itemAndQty.size() != 2)
		{
			nlwarning("receiveMissionItems failed: the provided mission items string is invalid");
			DEBUG_STOP;
			return;
		}

		const CSheetId sheetId(itemAndQty[0]);
		if (sheetId == CSheetId::Unknown)
		{
			nlwarning("receiveMissionItems failed: invalid mission item sheet '%s'", itemAndQty[0].c_str());
			DEBUG_STOP;
			return;
		}

		uint32 quantity;
		NLMISC::fromString(itemAndQty[1], quantity);
		if (quantity == 0)
		{
			nlwarning("receiveMissionItems failed: invalid quantity '%s'", itemAndQty[1].c_str());
			DEBUG_STOP;
			return;
		}

		msg.Items.push_back(sheetId);
		msg.Quantities.push_back(quantity);
	}
	nlassert(!msg.Items.empty());
	nlassert(msg.Items.size() == msg.Quantities.size());

	msg.send("EGS");
}

//----------------------------------------------------------------------------
/** @page code

@subsection giveMissionItems_ssc_

The npc will give mission items to the targeter player.

A new entry of the npc contextual menu will propose to the targeter player
to receive mission items from the npc.

Then user events are triggered on the group to inform it about what happens:
- user_event_1: triggered after the player has received the mission items from the npc.

Warning: this function can only be called after the event "player_target_npc".
Warning: only works on an R2 shard for R2 plot items.

Arguments: s(missionItems), s(missionText), c(groupToNotify) ->
@param[in] missionItems is the list of mission items, the string format is "item1:qty1;item2:qty2;...".
@param[in] missionText is the text which will appear in the npc contextual menu.
@param[in] groupToNotify is the npc group which will receive the user events.

@code
(@groupToNotify)group_name.context();
()giveMissionItems("toto.sitem:2;tata.sitem:1;titi.sitem:3", "Mission text", @groupToNotify);
@endcode

*/
// CSpawnBotNpc
void giveMissionItems_ssc_(CStateInstance* entity, CScriptStack& stack)
{
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() );
	stack.pop();
	string missionText = (string)stack.top();
	stack.pop();
	string missionItems = (string)stack.top();
	stack.pop();

	if (groupToNotify == NULL)
	{
		nlwarning("giveMissionItems failed: groupToNotify is NULL");
		DEBUG_STOP;
		return;
	}

	if (missionItems.empty())
	{
		nlwarning("giveMissionItems failed: missionItems is empty");
		DEBUG_STOP;
		return;
	}

	nlassert(entity != NULL);
	CGroup* const group = entity->getGroup();
	IManagerParent* const managerParent = group->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("giveMissionItems failed: the AI instance of the entity is NULL");
		DEBUG_STOP;
		return;
	}

	CBotPlayer* const talkingPlayer = TempPlayer;
	CSpawnBotNpc* const talkingNpc = dynamic_cast<CSpawnBotNpc*>(TempSpeaker);
	if	(	talkingPlayer == NULL
		||	talkingNpc == NULL
		||	talkingNpc->getPersistent().getOwner() != group) // check if the talking npc is in this group
	{
		nlwarning("giveMissionItems failed: invalid interlocutors");
		DEBUG_STOP;
		return;
	}

	// do nothing if one of them is dead
	if (!talkingPlayer->isAlive() || !talkingNpc->isAlive())
	{
		return;
	}

	// turn the npc to face the player
	talkingNpc->setTheta(talkingNpc->pos().angleTo(talkingPlayer->pos()));

	// send the request to the EGS
	CReceiveItemRequestMsg msg;
	msg.InstanceId = aiInstance->getInstanceNumber();
	msg.GroupAlias = groupToNotify->getAlias();
	msg.CharacterRowId = talkingPlayer->dataSetRow();
	msg.CreatureRowId = talkingNpc->dataSetRow();
	msg.MissionText = missionText;

	// extract items and quantities from the missionItems string
	std::vector<std::string> itemAndQtyList;
	NLMISC::splitString(missionItems, ";", itemAndQtyList);
	if (itemAndQtyList.empty())
	{
		nlwarning("giveMissionItems failed: the provided mission items string is invalid");
		DEBUG_STOP;
		return;
	}
	FOREACHC(it, std::vector<std::string>, itemAndQtyList)
	{
		std::vector<std::string> itemAndQty;
		NLMISC::splitString(*it, ":", itemAndQty);
		if (itemAndQty.size() != 2)
		{
			nlwarning("giveMissionItems failed: the provided mission items string is invalid");
			DEBUG_STOP;
			return;
		}

		const CSheetId sheetId(itemAndQty[0]);
		if (sheetId == CSheetId::Unknown)
		{
			nlwarning("giveMissionItems failed: invalid mission item sheet '%s'", itemAndQty[0].c_str());
			DEBUG_STOP;
			return;
		}

		uint32 quantity;
		NLMISC::fromString(itemAndQty[1], quantity);
		if (quantity == 0)
		{
			nlwarning("giveMissionItems failed: invalid quantity '%s'", itemAndQty[1].c_str());
			DEBUG_STOP;
			return;
		}

		msg.Items.push_back(sheetId);
		msg.Quantities.push_back(quantity);
	}
	nlassert(!msg.Items.empty());
	nlassert(msg.Items.size() == msg.Quantities.size());

	msg.send("EGS");
}

//----------------------------------------------------------------------------
/** @page code

@subsection talkTo_sc_

A new entry of the npc contextual menu will propose to the targeter player to talk to the npc.

Then user events are triggered on the group to inform it about what happens:
- user_event_1: triggered each time (because of the TRICK).
- user_event_3: triggered after the player has talked to the npc.

Warning: this function can only be called after the event "player_target_npc".

Arguments: s(missionText), c(groupToNotify) ->
@param[in] missionText is the text which will appear in the npc contextual menu.
@param[in] groupToNotify is the npc group which will receive the user events.

@code
(@groupToNotify)group_name.context();
()talkTo("Mission text", @groupToNotify);
@endcode

*/
// CSpawnBotNpc
void talkTo_sc_(CStateInstance* entity, CScriptStack& stack)
{
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() );
	stack.pop();
	string missionText = (string)stack.top();
	stack.pop();

	if (groupToNotify == NULL)
	{
		nlwarning("talkTo failed: groupToNotify is NULL");
		DEBUG_STOP;
		return;
	}

	nlassert(entity != NULL);
	CGroup* const group = entity->getGroup();
	IManagerParent* const managerParent = group->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("talkTo failed: the AI instance of the entity is NULL");
		DEBUG_STOP;
		return;
	}

	CBotPlayer* const talkingPlayer = TempPlayer;
	CSpawnBotNpc* const talkingNpc = dynamic_cast<CSpawnBotNpc*>(TempSpeaker);
	if	(	talkingPlayer == NULL
		||	talkingNpc == NULL
		||	talkingNpc->getPersistent().getOwner() != group) // check if the talking npc is in this group
	{
		nlwarning("talkTo failed: invalid interlocutors");
		DEBUG_STOP;
		return;
	}

	// do nothing if one of them is dead
	if (!talkingPlayer->isAlive() || !talkingNpc->isAlive())
	{
		return;
	}

	// turn the npc to face the player
	talkingNpc->setTheta(talkingNpc->pos().angleTo(talkingPlayer->pos()));

	// send the request to the EGS
	// TRICK: here we use an empty give item request
	CGiveItemRequestMsg msg;
	msg.InstanceId = aiInstance->getInstanceNumber();
	msg.GroupAlias = groupToNotify->getAlias();
	msg.CharacterRowId = talkingPlayer->dataSetRow();
	msg.CreatureRowId = talkingNpc->dataSetRow();
	msg.MissionText = missionText;
	nlassert(msg.Items.empty());
	nlassert(msg.Quantities.empty());

	msg.send("EGS");
}


//give_reward
void giveReward_ssssc_(CStateInstance* entity, CScriptStack& stack)
{
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() );
	stack.pop();
	
	string notEnoughPointsText = (string)stack.top();
	stack.pop();
	
	string inventoryFullText = (string)stack.top();
	stack.pop();
	
	string rareRewardText = (string)stack.top();
	stack.pop();
	
	string rewardText = (string)stack.top();
	stack.pop();
	
	
	if (groupToNotify == NULL)
	{
		nlwarning("giveReward failed: groupToNotify is NULL");
		DEBUG_STOP;
		return;
	}

	nlassert(entity != NULL);
	CGroup* const group = entity->getGroup();
	IManagerParent* const managerParent = group->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("giveReward failed: the AI instance of the entity is NULL");
		DEBUG_STOP;
		return;
	}

	CEntityId charEid(group->getEventParamString(0)); // must be player char EID
	CEntityId npcEid(group->getEventParamString(1)); // must be NPC EID

	if (charEid == CEntityId::Unknown || npcEid == CEntityId::Unknown)
	{
		nlwarning("giveReward failed: the chareter or npc can't be retrreived from the event parameter");
		DEBUG_STOP;
		return;
	}

	// retrieve the CBotPlayer
	CAIEntityPhysical	*charEntity=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(charEid));
	if	(!charEntity)
		return;
	CBotPlayer	*talkingPlayer=NLMISC::safe_cast<CBotPlayer*>(charEntity);
	if	(!talkingPlayer)
		return;

	// retrieve the CSpawnBotNpcBotPlayer
	CAIEntityPhysical	*npcEntity=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(npcEid));
	if	(!npcEntity)
		return;
	CSpawnBotNpc	*talkingNpc=NLMISC::safe_cast<CSpawnBotNpc*>(npcEntity);
	if	(!talkingNpc)
		return;

//	CBotPlayer* const talkingPlayer = TempPlayer;
//	CSpawnBotNpc* const talkingNpc = dynamic_cast<CSpawnBotNpc*>(TempSpeaker);
	if	(	talkingPlayer == NULL
		||	talkingNpc == NULL)
	{
		nlwarning("giveReward failed: invalid interlocutors");
		DEBUG_STOP;
		return;
	}

	// do nothing if one of them is dead
	if (!talkingPlayer->isAlive() || !talkingNpc->isAlive())
	{
		return;
	}

	// turn the npc to face the player
	talkingNpc->setTheta(talkingNpc->pos().angleTo(talkingPlayer->pos()));
	
	TDataSetRow	CharacterRowId = talkingPlayer->dataSetRow();
	TDataSetRow	CreatureRowId = talkingNpc->dataSetRow();
	
	IAisControl::getInstance()->giveRewardMessage(CharacterRowId, CreatureRowId, 
		rewardText, rareRewardText, inventoryFullText, notEnoughPointsText);
	
	
}




//give_reward
void teleportNear_fffc_(CStateInstance* entity, CScriptStack& stack)
{
	CGroupNpc* const groupToNotify = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() );
	stack.pop();
	
	float z = (float)stack.top(); stack.pop();
	float y = (float)stack.top(); stack.pop();
	float x = (float)stack.top(); stack.pop();
	
	
	if (groupToNotify == NULL)
	{
		nlwarning("teleportNear failed: groupToNotify is NULL");
		DEBUG_STOP;
		return;
	}

	nlassert(entity != NULL);
	CGroup* const group = entity->getGroup();
	IManagerParent* const managerParent = group->getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (aiInstance == NULL)
	{
		nlwarning("giveReward failed: the AI instance of the entity is NULL");
		DEBUG_STOP;
		return;
	}

	CEntityId charEid(group->getEventParamString(0)); // must be player char EID
	CEntityId npcEid(group->getEventParamString(1)); // must be NPC EID

	if (charEid == CEntityId::Unknown || npcEid == CEntityId::Unknown)
	{
		nlwarning("giveReward failed: the character or npc can't be retreived from the event parameter");
		DEBUG_STOP;
		return;
	}

	// retrieve the CBotPlayer
	CAIEntityPhysical	*charEntity=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(charEid));
	if	(!charEntity)
		return;
	CBotPlayer	*talkingPlayer=NLMISC::safe_cast<CBotPlayer*>(charEntity);
	if	(!talkingPlayer)
		return;

	// retrieve the CSpawnBotNpcBotPlayer
	CAIEntityPhysical	*npcEntity=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(npcEid));
	if	(!npcEntity)
		return;
	CSpawnBotNpc	*talkingNpc=NLMISC::safe_cast<CSpawnBotNpc*>(npcEntity);
	if	(!talkingNpc)
		return;

//	CBotPlayer* const talkingPlayer = TempPlayer;
//	CSpawnBotNpc* const talkingNpc = dynamic_cast<CSpawnBotNpc*>(TempSpeaker);
	if	(	talkingPlayer == NULL
		||	talkingNpc == NULL)
	{
		nlwarning("giveReward failed: invalid interlocutors");
		DEBUG_STOP;
		return;
	}

	// do nothing if one of them is dead
	if (!talkingPlayer->isAlive() || !talkingNpc->isAlive())
	{
		return;
	}

	// turn the npc to face the player
///	talkingNpc->setTheta(talkingNpc->pos().angleTo(talkingPlayer->pos()));
	
	TDataSetRow	CharacterRowId = talkingPlayer->dataSetRow();
	TDataSetRow	CreatureRowId = talkingNpc->dataSetRow();

	NLMISC::CEntityId player = CMirrors::getEntityId(CharacterRowId);
	if (player != NLMISC::CEntityId::Unknown)
	{		
		IAisControl::getInstance()->teleportNearMessage(player, x, y, z);
	}

	
}




// Return an "alive" bot from a group by the bot name
static CSpawnBot* getSpawnBotFromGroupByName(CGroupNpc* const group, const std::string& botname)
{
	// Group exist
	if (!group) { return 0; }

	// Group is in a valid AIInstance
	{		
		if (!group->getOwner())  { return 0;	}
		IManagerParent* const managerParent = group->getOwner()->getOwner();
		CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
		if (!aiInstance) { return 0; }
	}

	// Bot is spawn and alive
	{
		
		CSpawnBot* spawnBot=0;
		CAliasCont<CBot> const& bots = group->bots();
		CCont<CBot> & cc_bots = group->bots();
		CBot* child;
		if (botname == "")
			child = bots.getFirstChild();
		else
			child = bots.getChildByName(botname);
		if (!child)	{  return 0; }			
		
		spawnBot = child->getSpawnObj();
		if (!spawnBot || !spawnBot->isAlive())	{ return 0; }
		return spawnBot;		
	}

	nlassert(0); //this path is never used
	return 0;
}

//----------------------------------------------------------------------------
/** @page code

@subsection facing_cscs_

The npc1 will turn to npc2


Arguments: c(group1), s(botname1), c(group2), s(botname2),  ->
@param[in] group1 is the npc group of the boot that turn to the other bot
@param[in] botname1 is the name of the bot
@param[in] group2 is the npc group of the boot that is the target
@param[in] botname2 is the name of the bot that is targeted


@code
(@group1)group_name1.context();
(@group1)group_name2.context();
()facing(@group1, "bob", @group2, "bobette");
@endcode

*/
// CSpawnBotNpc



void facing_cscs_(CStateInstance* entity, CScriptStack& stack)
{
	string botname2 = (string)stack.top(); stack.pop();	
	CGroupNpc* const group2 = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() ); stack.pop();	
	string botname1 = (string)stack.top(); stack.pop();	
	CGroupNpc* const group1 = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() ); stack.pop();
		
	CSpawnBot* bot1 = getSpawnBotFromGroupByName(group1, botname1);
	CSpawnBot* bot2 = getSpawnBotFromGroupByName(group2, botname2);

	if (!bot1 || !bot2) { return; }

	CSpawnBotNpc* bot = dynamic_cast<CSpawnBotNpc*>(bot1);
	if (!bot) { return; }
	// Same than setTheta but initial theta is restored few secondes later
	bot->setFacing(bot1->pos().angleTo(bot2->pos()));
	//	bot1->setTheta(bot1->pos().angleTo(bot2->pos()));
}

//----------------------------------------------------------------------------
/** @page code

@subsection npcSay_css_

Make a npc say a text

There are 3 type of text
- Classic  StringId (IOS does traduction)
- RING "complete text (no traduction)
- RING StringId	(DSS does traduction)

Arguments: c(group), s(botname), s(text),  ->
@param[in] group is the npc group of the boot that does the emote
@param[in] botname is the name of the bot
@param[in] text is the name of the emote

@code
(@group)group_name.context();
()npcSay(@group, "bob", "DSS_1601 RtEntryText_6") ;// Send To dss
()npcSay(@group, "bob", "RAW Ca farte?"); // phrase direcly send to IOS as raw (for debug)
()npcSay(@group, "bob", "answer_group_no_m"); //phrase id

@endcode


*/
// CSpawnBotNpc

#include "game_share/chat_group.h"
#include "game_share/send_chat.h"

void execSayHelper(CSpawnBot *spawnBot, NLMISC::CSString text, CChatGroup::TGroupType mode = CChatGroup::say)
{
	if (spawnBot)
	{
		NLMISC::CSString prefix = text.left(4);
		if (prefix=="DSS_")
		{
			
			NLMISC::CSString phrase = text.right(text.length() - 4);
			NLMISC::CSString idStr = phrase.strtok(" ",false,false,false,false);
			uint32 scenarioId = atoi(idStr.c_str());
			forwardToDss(spawnBot->dataSetRow(), mode, phrase, scenarioId);
			return;
		}
		
		if (prefix=="RAW ")
		{
			std::string phrase = text.right(text.length()-4);
			ucstring ucstr = phrase;
			npcChatToChannelSentence(spawnBot->dataSetRow(), mode, ucstr);
			return;
		}

		//Classic phrase ID
		npcChatToChannel(spawnBot->dataSetRow(), mode, text);
	}
}

void npcSay_css_(CStateInstance* entity, CScriptStack& stack)
{
	string text = (string)stack.top(); stack.pop();	
	string botname = (string)stack.top(); stack.pop();	
	CGroupNpc* const group = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() ); stack.pop();	
		
	CSpawnBot* spawnBot = getSpawnBotFromGroupByName(group, botname);

	if (!spawnBot) { return; }

	execSayHelper(spawnBot, text);
}

//----------------------------------------------------------------------------
/** @page code

@subsection npcSay_ss_

Make a npc say a text

Arguments: s(text), s(mode) ->
@param[in] text is the text to say. prefix with ID: to use an id
@param[in] mode is the mode to use (say, shout)

@code
()npcSay("Hello!","say"); // phrase direcly send to IOS as raw
()npcSay("ID:answer_group_no_m","shout"); // phrase id
@endcode

*/

void npcSay_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string sMode = (std::string)stack.top(); stack.pop();
	std::string text = (std::string)stack.top(); stack.pop();

	CChatGroup::TGroupType mode = CChatGroup::say;
	mode = CChatGroup::stringToGroupType(sMode);
	CGroup* group = entity->getGroup();

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
					std::string prefix = NLMISC::CSString(text).left(3);
					if (NLMISC::nlstricmp(prefix.c_str(), "id:") == 0) {
						text = NLMISC::CSString(text).right(text.length()-3);
						execSayHelper(spawnBot, text, mode);
					}
					else {
						execSayHelper(spawnBot, "RAW " + text, mode);
					}
				}
			}
		}
	}
}



//----------------------------------------------------------------------------
/** @page code

@subsection deactivateEasterEgg_fff_

Call the DSS function CAnimationModule::dssMessage

Arguments: f(easterEggId), f(scenarioId), f(actId) ->

@code
()dssMessage(114, "BC", "Bob", "Life is harde");
@endcode

*/

void dssMessage_fsss_(CStateInstance* entity, CScriptStack& stack)
{
	string msg = (string)stack.top(); stack.pop();	
	string who = (string)stack.top(); stack.pop();	
	string mode = (string)stack.top(); stack.pop();	
	float instance = (float)stack.top(); stack.pop();	
	
	IAisControl::getInstance()->dssMessage(TSessionId(uint32(instance)), mode, who, msg);
	return;
}


//----------------------------------------------------------------------------
/** @page code

@subsection setScenarioPoints

Call the DSS function CAnimationModule::setScenarioPoints

Arguments: f(scenarioInstance), f(scenarioPoints) ->

@code
()setScenarioPoints(114, 42);
@endcode

*/

void setScenarioPoints_ff_(CStateInstance* entity, CScriptStack& stack)
{
	float scenarioPoints = (float)stack.top(); stack.pop();	
	float instance = (float)stack.top(); stack.pop();	
	
	IAisControl::getInstance()->setScenarioPoints(TSessionId(uint32(instance)), scenarioPoints);
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection startScenarioTiming

Call the DSS function CAnimationModule::startScenarioTiming

Arguments: f(scenarioInstance),

@code
()startScenarioTiming(114, 42);
@endcode

*/

void startScenarioTiming_f_(CStateInstance* entity, CScriptStack& stack)
{	
	float instance = (float)stack.top(); stack.pop();	
	
	IAisControl::getInstance()->startScenarioTiming(TSessionId(uint32(instance)));
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection endScenarioTiming

Call the DSS function CAnimationModule::endScenarioTiming

Arguments: f(scenarioInstance),

@code
()endScenarioTiming(114, 42);
@endcode

*/

void endScenarioTiming_f_(CStateInstance* entity, CScriptStack& stack)
{	
	float instance = (float)stack.top(); stack.pop();	
	
	IAisControl::getInstance()->endScenarioTiming(TSessionId(uint32(instance)));
	return;
}

//----------------------------------------------------------------------------
/** @page code

@subsection emote_css_

Make a npc launch a emote


Arguments: c(group1), s(botname1), c(group2), s(botname2),  ->
@param[in] group1 is the npc group of the boot that does the emote
@param[in] botname1 is the name of the bot
@param[in] emote is the name of the emote


@code
(@group1)group_name.context();
()emote(@group1, "bob", "sad")
@endcode

*/
// CSpawnBotNpc


/****************************************************************************/

void emote_css_(CStateInstance* entity, CScriptStack& stack)
{
	string emoteName = (string)stack.top(); stack.pop();	
	string botname = (string)stack.top(); stack.pop();	
	CGroupNpc* const group = dynamic_cast<CGroupNpc*>( (IScriptContext*)stack.top() ); stack.pop();	
		
	CSpawnBot* spawnBot = getSpawnBotFromGroupByName(group, botname);

	if (!spawnBot) { return; }

	//CBot& bot = spawnBot->getPersistent();
	
	// The entity Id must be valid (we know that the bot is alive so its entity Id must be ok)
	NLMISC::CEntityId	entityId=spawnBot->getEntityId();
	if (entityId == NLMISC::CEntityId::Unknown)
	{
		return;
	}

	// Is the emote valid
	uint32 emoteId = CAIS::instance().getEmotNumber(emoteName);
	if (emoteId == ~0)
	{
		return;
	}

	// Get the behaviour Id
	MBEHAV::EBehaviour behaviourId = (MBEHAV::EBehaviour)(emoteId + MBEHAV::EMOTE_BEGIN);
	
	// Change the behaviour
	NLNET::CMessage msgout("SET_BEHAVIOUR");
	msgout.serial(entityId);
	MBEHAV::CBehaviour bh(behaviourId);
	bh.Data = (uint16)(CTimeInterface::gameCycle());
	msgout.serial(bh);

	NLNET::CUnifiedNetwork::getInstance()->send( "EGS", msgout );
}


void emote_ss_(CStateInstance* entity, CScriptStack& stack)
{
	string emoteName = (string)stack.top(); stack.pop();	
	NLMISC::CEntityId botId = NLMISC::CEntityId((std::string)stack.top());

	if (botId == NLMISC::CEntityId::Unknown)
	{
		return;
	}

	// Is the emote valid
	uint32 emoteId = CAIS::instance().getEmotNumber(emoteName);
	if (emoteId == ~0)
	{
		return;
	}

	// Get the behaviour Id
	MBEHAV::EBehaviour behaviourId = (MBEHAV::EBehaviour)(emoteId + MBEHAV::EMOTE_BEGIN);
	
	// Change the behaviour
	NLNET::CMessage msgout("SET_BEHAVIOUR");
	msgout.serial(botId);
	MBEHAV::CBehaviour bh(behaviourId);
	bh.Data = (uint16)(CTimeInterface::gameCycle());
	msgout.serial(bh);

	NLNET::CUnifiedNetwork::getInstance()->send( "EGS", msgout );
}



void emote_s_(CStateInstance* entity, CScriptStack& stack)
{
	string emoteName = (string)stack.top(); stack.pop();

	// Is the emote valid
	uint32 emoteId = CAIS::instance().getEmotNumber(emoteName);
	if (emoteId == ~0)
		return;

	// Get the behaviour Id
	MBEHAV::EBehaviour behaviourId = (MBEHAV::EBehaviour)(emoteId + MBEHAV::EMOTE_BEGIN);
	

	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
	{
		FOREACH(itBot, CCont<CBot>, group->bots())
		{
			CBot* bot = *itBot;
			if (bot)
			{
				// Change the behaviour
				if (bot->isSpawned())
				{
					CSpawnBot *spawnBot = bot->getSpawnObj();
					if (spawnBot)
					{
						CEntityId	botId = spawnBot->getEntityId();
						NLNET::CMessage msgout("SET_BEHAVIOUR");
						msgout.serial(botId);
						MBEHAV::CBehaviour bh(behaviourId);
						bh.Data = (uint16)(CTimeInterface::gameCycle());
						msgout.serial(bh);

						NLNET::CUnifiedNetwork::getInstance()->send( "EGS", msgout );
					}
				}
			}
		}
	}
	
}

void rename_s_(CStateInstance* entity, CScriptStack& stack)
{
	string newName = (string)stack.top(); stack.pop();	

	CGroup* group = entity->getGroup();
	
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
					if (spawnBot)
					{
						TDataSetRow	row = spawnBot->dataSetRow();
						ucstring name;
						name.fromUtf8(newName);
						NLNET::CMessage	msgout("CHARACTER_NAME");
						msgout.serial(row);
						msgout.serial(name);
						sendMessageViaMirror("IOS", msgout);
						bot->setCustomName(name);
					}
				}
			}
		}
	}
	
}

void vpx_s_(CStateInstance* entity, CScriptStack& stack)
{
	string vpx = (string)stack.top(); stack.pop();	

	CGroup* group = entity->getGroup();
	
	if (group->isSpawned())
	{
		FOREACH(itBot, CCont<CBot>, group->bots())
		{
			CBotNpc* bot = NLMISC::safe_cast<CBotNpc*>(*itBot);
			if (bot)
			{
				bot->setVisualProperties(vpx);
				bot->sendVisualProperties();
			}
		}
	}	
}

//----------------------------------------------------------------------------
/** @page code

@subsection maxHitRange_f_
Sets the max hit range possible for player, in meters

Arguments: f(MaxHitRange) ->
@param[in] MaxHitRange set the max range for player can hit this npc group

@code
()maxHitRange(50); // Set the max hit range in 50 meters all npc in group
@endcode

*/
// CBotNpc
void maxHitRange_f_(CStateInstance* entity, CScriptStack& stack)
{
	float maxHitRange = (float&)stack.top(); stack.pop();
	CGroup* group = entity->getGroup();
	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
	
	FOREACH(botIt, CCont<CBot>,	group->bots())
	{
		CBot* bot = *botIt;
		
		if (!bot->isSpawned()) return;	
		if (bot->getRyzomType() == RYZOMID::npc)
		{
			CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
			botNpc->setMaxHitRangeForPlayer(maxHitRange);
		}
	}
}

////----------------------------------------------------------------------------
///** @page code
//
//@subsection hideMissionStepIcon_b_
//Allows to hide icons for current missions steps having interactions with this NPC (default: shown)
//
//Arguments: b(hide) ->
//@param[in] hide true to hide the icon for all mission steps relating to this NPC (default: false)
//
//*/
//// CBotNpc
//void hideMissionStepIcon_b_(CStateInstance* entity, CScriptStack& stack)
//{
//	bool b = (bool&)stack.top(); stack.pop();
//	CGroup* group = entity->getGroup();
//	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
//	
//	FOREACH(botIt, CCont<CBot>,	group->bots())
//	{
//		CBot* bot = *botIt;
//		
//		if (!bot->isSpawned()) return;	
//		if (bot->getRyzomType() == RYZOMID::npc)
//		{
//			CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
//			botNpc->setMissionStepIconHidden(b);
//		}
//	}
//}
//
////----------------------------------------------------------------------------
///** @page code
//
//@subsection hideMissionGiverIcon_b_
//Allows to hide icons for missions proposed by this NPC (default: shown)
//
//Arguments: b(hide) ->
//@param[in] hide true to hide the icon for all missions propsed by this NPC (default: false)
//
//*/
//// CBotNpc
//void hideMissionGiverIcon_b_(CStateInstance* entity, CScriptStack& stack)
//{
//	bool b = (bool&)stack.top(); stack.pop();
//	CGroup* group = entity->getGroup();
//	CGroupNpc* npcGroup = NLMISC::safe_cast<CGroupNpc*>(group);
//	
//	FOREACH(botIt, CCont<CBot>,	group->bots())
//	{
//		CBot* bot = *botIt;
//		
//		if (!bot->isSpawned()) return;	
//		if (bot->getRyzomType() == RYZOMID::npc)
//		{
//			CBotNpc* botNpc = NLMISC::safe_cast<CBotNpc*>(bot);
//			botNpc->setMissionGiverIconHidden(b);
//		}
//	}
//}


std::map<std::string, FScrptNativeFunc> nfGetNpcGroupNativeFunctions()
{
	std::map<std::string, FScrptNativeFunc> functions;
	
#define REGISTER_NATIVE_FUNC(cont, func) cont.insert(std::make_pair(std::string(#func), &func))

	REGISTER_NATIVE_FUNC(functions, setFactionProp_ss_);
	REGISTER_NATIVE_FUNC(functions, setOupostMode_ss_);
	REGISTER_NATIVE_FUNC(functions, moveToZone_ss_);
	REGISTER_NATIVE_FUNC(functions, setActivity_s_);
	REGISTER_NATIVE_FUNC(functions, startWander_f_);
	REGISTER_NATIVE_FUNC(functions, startMoving_fff_);
	REGISTER_NATIVE_FUNC(functions, waitInZone_s_);
	REGISTER_NATIVE_FUNC(functions, stopMoving__);
	REGISTER_NATIVE_FUNC(functions, followPlayer_sf_);
	REGISTER_NATIVE_FUNC(functions, wander__);
	REGISTER_NATIVE_FUNC(functions, setAttackable_f_);
	REGISTER_NATIVE_FUNC(functions, setPlayerAttackable_f_);
	REGISTER_NATIVE_FUNC(functions, setBotAttackable_f_);
	REGISTER_NATIVE_FUNC(functions, setFactionAttackableAbove_sff_);
	REGISTER_NATIVE_FUNC(functions, setFactionAttackableBelow_sff_);
	REGISTER_NATIVE_FUNC(functions, addBotChat_s_);
	REGISTER_NATIVE_FUNC(functions, clearBotChat__);
	REGISTER_NATIVE_FUNC(functions, ignoreOffensiveActions_f_);
	REGISTER_NATIVE_FUNC(functions, setDespawnTime_f_);
	REGISTER_NATIVE_FUNC(functions, setRespawnTime_f_);
	REGISTER_NATIVE_FUNC(functions, addHpUpTrigger_ff_);
	REGISTER_NATIVE_FUNC(functions, delHpUpTrigger_ff_);
	REGISTER_NATIVE_FUNC(functions, addHpDownTrigger_ff_);
	REGISTER_NATIVE_FUNC(functions, delHpDownTrigger_ff_);
	REGISTER_NATIVE_FUNC(functions, addHpUpTrigger_fs_);
	REGISTER_NATIVE_FUNC(functions, delHpUpTrigger_fs_);
	REGISTER_NATIVE_FUNC(functions, addHpDownTrigger_fs_);
	REGISTER_NATIVE_FUNC(functions, delHpDownTrigger_fs_);
	REGISTER_NATIVE_FUNC(functions, addNamedEntityListener_ssf_);
	REGISTER_NATIVE_FUNC(functions, delNamedEntityListener_ssf_);
	REGISTER_NATIVE_FUNC(functions, addNamedEntityListener_sss_);
	REGISTER_NATIVE_FUNC(functions, delNamedEntityListener_sss_);
	REGISTER_NATIVE_FUNC(functions, setPlayerController_ss_);
	REGISTER_NATIVE_FUNC(functions, clearPlayerController_s_);
	REGISTER_NATIVE_FUNC(functions, activateEasterEgg_fffsffffsss_);
	REGISTER_NATIVE_FUNC(functions, deactivateEasterEgg_fff_);
	REGISTER_NATIVE_FUNC(functions, receiveMissionItems_ssc_);
	REGISTER_NATIVE_FUNC(functions, giveMissionItems_ssc_);
	REGISTER_NATIVE_FUNC(functions, talkTo_sc_);
	REGISTER_NATIVE_FUNC(functions, facing_cscs_);
	REGISTER_NATIVE_FUNC(functions, emote_css_);
	REGISTER_NATIVE_FUNC(functions, emote_ss_);
	REGISTER_NATIVE_FUNC(functions, emote_s_);
	REGISTER_NATIVE_FUNC(functions, rename_s_);
	REGISTER_NATIVE_FUNC(functions, vpx_s_);
	REGISTER_NATIVE_FUNC(functions, npcSay_css_);
	REGISTER_NATIVE_FUNC(functions, npcSay_ss_);
	REGISTER_NATIVE_FUNC(functions, dssMessage_fsss_);
	REGISTER_NATIVE_FUNC(functions, despawnBotByAlias_s_);	
	REGISTER_NATIVE_FUNC(functions, giveReward_ssssc_);
	REGISTER_NATIVE_FUNC(functions, teleportNear_fffc_);

	REGISTER_NATIVE_FUNC(functions, setScenarioPoints_ff_);
	REGISTER_NATIVE_FUNC(functions, startScenarioTiming_f_);
	REGISTER_NATIVE_FUNC(functions, endScenarioTiming_f_);

	REGISTER_NATIVE_FUNC(functions, maxHitRange_f_);

//	REGISTER_NATIVE_FUNC(functions, hideMissionStepIcon_b_);
//	REGISTER_NATIVE_FUNC(functions, hideMissionGiverIcon_b_);


#undef REGISTER_NATIVE_FUNC
	
	return functions;
}
