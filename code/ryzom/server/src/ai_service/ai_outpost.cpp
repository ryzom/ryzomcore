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
#include "ai_outpost.h"
#include "continent.h"
#include "ai_grp_npc.h"
#include "game_share/people.h"
#include "ai_profile_npc.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

using namespace std;
using namespace NLMISC;
using namespace	AITYPES;



CFileDisplayer OutpostDisplayer("outposts.log");
CLog OutpostDbgLog(CLog::LOG_DEBUG), OutpostInfLog(CLog::LOG_INFO), OutpostWrnLog(CLog::LOG_WARNING), OutpostErrLog(CLog::LOG_ERROR);


namespace OUTPOSTHELPERS {

static std::map<uint32, uint32> outpostFactions;

bool isAttackingFaction(uint32 factionIndex, CAIEntityPhysical const* player)
{
	std::map<uint32, uint32>::const_iterator it = outpostFactions.find(player->outpostAlias());
	return it!=outpostFactions.end() && it->second==factionIndex;
}

}

//////////////////////////////////////////////////////////////////////////////
// COutpost                                                                 //
//////////////////////////////////////////////////////////////////////////////

//std::set<CContinent*>		ChangedOutposts;
//COutpost::TOutpostIndex COutpost::_Outposts;

COutpost::COutpost(CContinent* owner, uint32 alias, std::string const& name, std::string const& filename)
: CAliasChild<CContinent>(owner, alias, name)
, CAliasTreeRoot(filename)
, _OwnerAllianceId(InvalidAllianceId)
, _AttackerAllianceId(InvalidAllianceId)
, _State(OUTPOSTENUMS::UnknownOutpostState)

{
	static bool logInitDone = false;
	if ( ! logInitDone )
	{
		OutpostDbgLog.addDisplayer( &OutpostDisplayer );
		OutpostInfLog.addDisplayer( &OutpostDisplayer );
		OutpostWrnLog.addDisplayer( &OutpostDisplayer );
		logInitDone = true;
	}

	OUTPOST_DBG("Creating outpost %s' (%s)", name.c_str(), getAliasFullName().c_str());
	if (LogOutpostDebug)
		OUTPOST_DBG("Creating outpost '%s'", getAliasFullName().c_str());
	
	_OutpostName = getName();
	COutpostManager* manager = NULL;
	// Create default squad manager
	manager = new COutpostSquadManager(this, 0, "default_squad_manager", filename);
	if (manager)
	{
		_Managers.addChild(manager);
		manager->init();
	}
	// Create default building manager
	manager = new COutpostManager(this, 0, "default_building_manager", filename);
	if (manager)
	{
		_Managers.addChild(manager);
		manager->init();
		CGroupNpc* group = new CGroupNpc(manager, NULL, /*AStarFlag*/RYAI_MAP_CRUNCH::Nothing);
		if (group)
		{
			manager->groups().addAliasChild(group);
			group->setAutoSpawn(false);
			group->setName("default_building_group");
			group->clearParameters();
			group->setPlayerAttackable(false);
			group->setBotAttackable(false);
			group->setBotsAreNamedFlag();
		}
	}
}

COutpost::~COutpost()
{
	if (LogOutpostDebug)
		OUTPOST_DBG("Deleting outpost '%s'", getAliasFullName().c_str());

	_SpawnZones.clear();
}

std::string	COutpost::getOneLineInfoString() const
{
	return std::string("Outpost '") + getName() + "' " + getAliasString() + ", State=" + OUTPOSTENUMS::toString(_State);
}

std::string COutpost::getFullName() const
{
	return std::string(getOwner()->getFullName() +":"+ getName());
}

std::string COutpost::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":o%u", getChildIndex());
}
/*
std::string	COutpost::getPlaceOwnerFullName() const
{
	return getFullName();
}

std::string	COutpost::getPlaceOwnerIndexString() const
{
	return getIndexString();
}
*/
CAIInstance* COutpost::getAIInstance() const
{
	return getOwner()->getAIInstance();
}

std::string	COutpost::getManagerIndexString(CManager const* manager) const
{
	return getIndexString()+NLMISC::toString(":m%u", manager->getChildIndex());
}

IAliasCont* COutpost::getAliasCont(TAIType type)
{
	switch (type)
	{
	case AITypeOutpostSquadFamily:
		OUTPOST_WRN( "Obsolete squad family node under outpost %s", getName().c_str() );
		return NULL;
	case AITypeOutpostSpawnZone:
		return &_SpawnZones;
	case AITypeManager:
		return &_Managers;
	case AITypeOutpostManager:
		return &_Managers;
	case AITypeOutpostBuilding:
		return getBuildingGroup()->getAliasCont(AITypeBot);
	default:
		return NULL;
	}

}

CAliasTreeOwner* COutpost::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;

	switch(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITypeOutpostSpawnZone:
		child = new COutpostSpawnZone(this, aliasTree);
		break;
	case AITypeManager:
		child = new COutpostSquadManager(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	case AITypeOutpostManager:
		child = new COutpostSquadManager(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	case AITypeOutpostBuilding:
		return getBuildingGroup()->createChild(cont, aliasTree);
	}

	if (child)
		cont->addAliasChild(child);
	return	child;
}

CGroup* COutpost::getBuildingGroup()
{
	COutpostManager* manager = _Managers.getChildByName("default_building_manager");
	return manager->groups().getChildByName("default_building_group");
}

void COutpost::setTribe(const std::string &tribeName)
{
	// The tribe is only populated by the NPCs of the squads
	_Tribe = tribeName;
	if (LogOutpostDebug)
		OUTPOST_DBG("setting tribe '%s' in '%s' as owner for outpost '%s'",
			tribeName.c_str(),
			getOwner()->getName().c_str(),
			getAliasFullName().c_str());
}

void COutpost::setOwnerAlliance( TAllianceId ownerAllianceId )
{
	if (_OwnerAllianceId!=ownerAllianceId)
	{
		// if the old owner was a tribe
		if (_OwnerAllianceId==InvalidAllianceId)
		{
			triggerSpecialEvent(OUTPOSTENUMS::TribeOwnershipEnd);
			OUTPOSTHELPERS::outpostFactions.erase(getAlias());
		}
		else
			triggerSpecialEvent(OUTPOSTENUMS::GuildOwnershipEnd);
	}
	std::swap(_OwnerAllianceId, ownerAllianceId); // 'ownerAllianceId' contains old owner alliance id
	if (_OwnerAllianceId!=ownerAllianceId)
	{
		triggerSpecialEvent(OUTPOSTENUMS::OwnerChanged);
		// if the new owner is a tribe
		if (_OwnerAllianceId==InvalidAllianceId)
		{
			OUTPOSTHELPERS::outpostFactions.insert(make_pair(getAlias(), CStaticFames::getInstance().getFactionIndex(_Tribe)));
			triggerSpecialEvent(OUTPOSTENUMS::TribeOwnershipBegin);
		}
		else
			triggerSpecialEvent(OUTPOSTENUMS::GuildOwnershipBegin);
	}
}

void COutpost::setAttackerAlliance( TAllianceId attackerAllianceId )
{
	std::swap(_AttackerAllianceId, attackerAllianceId); // 'attackerAllianceId' contains old attacker alliance id
	if (_AttackerAllianceId!=attackerAllianceId)
	{
		triggerSpecialEvent(OUTPOSTENUMS::AttackerChanged);
	}
	
	// Update the enemies of the current squads of the outpost
	FOREACH( im, CAliasCont<COutpostManager>, _Managers )
		im->setEnemies( _AttackerAllianceId );
}

void COutpost::setState( OUTPOSTENUMS::TOutpostState state )
{
	if (_State!=state)
	{
		if (_State==OUTPOSTENUMS::Peace)
			triggerSpecialEvent(OUTPOSTENUMS::PeaceStateEnd);
	}
	std::swap(_State, state); // 'state' contains old _State
	if (_State!=state)
	{
		triggerSpecialEvent(OUTPOSTENUMS::StateChanged);
		if (_State==OUTPOSTENUMS::Peace)
			triggerSpecialEvent(OUTPOSTENUMS::PeaceStateBegin);
	}
}

void COutpost::update()
{
//	FOREACH(it, CAliasCont<COutpostSquadFamily>, _SquadFamilies) it->update();
//	FOREACH(it, CAliasCont<COutpostSpawnZone>,   _SpawnZones)    it->update();
	FOREACH(it, CAliasCont<COutpostManager>,     _Managers)      it->update();
}

void COutpost::addZone(COutpostSpawnZone* zone)
{
#if !FINAL_VERSION
	if (_ZoneList.find(NLMISC::CStringMapper::map(zone->getName()))!=_ZoneList.end())
		OUTPOST_WRN("this OutpostSpawnZone have the same name than another: %s", zone->getName().c_str());
#endif
	_ZoneList[NLMISC::CStringMapper::map(zone->getName())] = zone;
}

void COutpost::removeZone(COutpostSpawnZone* zone)
{
	_ZoneList.erase(NLMISC::CStringMapper::map(zone->getName()));
}

COutpostSpawnZone* COutpost::getZone(NLMISC::TStringId zoneName)
{
	return _ZoneList[zoneName];
}

void COutpost::serviceEvent(CServiceEvent const& info)
{
	if (info.getServiceName()=="EGS" && info.getEventType()==CServiceEvent::SERVICE_DOWN)
	{
		// Delete all the squad groups
		COutpostManager* manager = _Managers.getChildByName("default_squad_manager");
		if (manager)
			manager->groups().clear();
	}
	FOREACH(itManager, CAliasCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		manager->serviceEvent(info);
	}
}

bool COutpost::spawn()
{
	// Spawn outpost managers
	CCont<COutpostManager>::iterator itManager, itManagerEnd(_Managers.end());
	for (itManager=_Managers.begin(); itManager!=itManagerEnd; ++itManager)
	{
		COutpostManager* manager = *itManager;
		manager->spawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

bool COutpost::despawn()
{
	// Despawn instance managers
	CCont<COutpostManager>::iterator itManager, itManagerEnd(_Managers.end());
	for (itManager=_Managers.begin(); itManager!=itManagerEnd; ++itManager)
	{
		COutpostManager* manager = *itManager;
		manager->despawnMgr();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

//-----------------------------------------------------------------------------
std::string COutpost::getStateName() const
{
	return OUTPOSTENUMS::toString(_State);
}

//////////////////////////////////////////////////////////////////////////////
// COutpostSquadFamily                                                      //
//////////////////////////////////////////////////////////////////////////////

IAliasCont* COutpostSquadFamily::getAliasCont(TAIType type)
{
	switch (type)
	{
	case AITypeGroupTemplate:
	case AITypeGroupTemplateMultiLevel:
		return &_GroupDescs;
	default:
		return NULL;
	}
	
}

CAliasTreeOwner* COutpostSquadFamily::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner* child = NULL;
	
	switch (aliasTree->getType())
	{
	case AITypeSquadTemplateVariant:
		child = new CGroupDesc<COutpostSquadFamily>(this, aliasTree->getAlias(), aliasTree->getParent()->getName()+":"+aliasTree->getName());
		break;
	case AITypeGroupTemplate:
	case AITypeGroupTemplateMultiLevel:
		child = new CGroupDesc<COutpostSquadFamily>(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	}
	
	if (child)
		cont->addAliasChild(child);
	return child;
}

std::string COutpostSquadFamily::getFullName() const
{
//	return std::string(getOwner()->getFullName() +":"+ getName());
	return getName();
}

std::string COutpostSquadFamily::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":sf%u", getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// CGroupDesc                                                               //
//////////////////////////////////////////////////////////////////////////////

template <> size_t const CGroupDesc<COutpostSquadFamily>::_MultiLevelSheetCount = 20;

//////////////////////////////////////////////////////////////////////////////
// CBotDesc                                                                 //
//////////////////////////////////////////////////////////////////////////////

template <> size_t const CBotDesc<COutpostSquadFamily>::_MultiLevelSheetCount = 20;

//////////////////////////////////////////////////////////////////////////////
// COutpostSpawnZone                                                        //
//////////////////////////////////////////////////////////////////////////////

COutpostSpawnZone::COutpostSpawnZone(COutpost* owner, CAIAliasDescriptionNode* adn)
: CAIPlaceXYR(owner, adn)
{
	owner->addZone(this);
}

COutpostSpawnZone::~COutpostSpawnZone()
{
	static_cast<COutpost*>(getOwner())->removeZone(this);
}

std::string COutpostSpawnZone::getFullName() const
{
	return std::string(getOwner()->getFullName() +":"+ getName());
}

std::string COutpostSpawnZone::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// COutpostManager                                                          //
//////////////////////////////////////////////////////////////////////////////

COutpostManager::COutpostManager(COutpost* parent, uint32 alias, std::string const& name, std::string const& filename)
: CMgrNpc(parent, alias, name, filename)
, _AutoSpawn(false)
{
	registerEvents();
}

COutpostManager::~COutpostManager()
{
	unregisterEvents(); // need be called otherwise the state manager will be unhappy when the CAIEvent objects are destroyed
}

std::string	COutpostManager::getOneLineInfoString() const
{
	return std::string("Outpost manager '") + getName() + "'";
}

void COutpostManager::update()
{
	CMgrNpc::update();
}

IAliasCont* COutpostManager::getAliasCont(TAIType type)
{
//	IAliasCont* cont = NULL;
//	switch (type)
//	{
//	default:
//		break;
//	}
//	if (cont)
//		return cont;
//	else
		return CMgrNpc::getAliasCont(type);
}

CAliasTreeOwner* COutpostManager::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
//	CAliasTreeOwner* child = NULL;
//	switch (aliasTree->getType())
//	{
//	default:
//		break;
//	}
//	if (child)
//		cont->addAliasChild(child);	
//	if (child)
//		return child;
//	else
		return CMgrNpc::createChild(cont, aliasTree);
}

void COutpostManager::registerEvents()
{
	_StateMachine.addEvent(	"outpost_peace_state_begin",		EventOutpostPeaceStateBegin		);
	_StateMachine.addEvent(	"outpost_peace_state_end",			EventOutpostPeaceStateEnd		);
	_StateMachine.addEvent(	"outpost_tribe_ownership_begin",	EventOutpostTribeOwnershipBegin	);
	_StateMachine.addEvent(	"outpost_tribe_ownership_end",		EventOutpostTribeOwnershipEnd	);
	_StateMachine.addEvent(	"outpost_guild_ownership_begin",	EventOutpostGuildOwnershipBegin	);
	_StateMachine.addEvent(	"outpost_guild_ownership_end",		EventOutpostGuildOwnershipEnd	);
	_StateMachine.addEvent(	"outpost_owner_changed",			EventOutpostOwnerChanged		);
	_StateMachine.addEvent(	"outpost_attacker_changed",			EventOutpostAttackerChanged		);
	_StateMachine.addEvent(	"outpost_state_changed",			EventOutpostStateChanged		);
}

void COutpostManager::unregisterEvents()
{
	_StateMachine.delEvent(	"outpost_peace_state_begin" );
	_StateMachine.delEvent(	"outpost_peace_state_end" );
	_StateMachine.delEvent(	"outpost_tribe_ownership_begin" );
	_StateMachine.delEvent(	"outpost_tribe_ownership_end" );
	_StateMachine.delEvent(	"outpost_guild_ownership_begin" );
	_StateMachine.delEvent(	"outpost_guild_ownership_end" );
	_StateMachine.delEvent(	"outpost_owner_changed" );
	_StateMachine.delEvent(	"outpost_attacker_changed" );
	_StateMachine.delEvent(	"outpost_state_changed"	);
}

void COutpostManager::autoSpawnBegin()
{
	if (!_AutoSpawn)
		return;
	FOREACH(itGroup, CCont<CGroup>, groups())
	{
		CGroupNpc* group = static_cast<CGroupNpc*>(*itGroup);
		if (group)
		{
			CSpawnGroupNpc* spawn = group->getSpawnObj();
			if (spawn)
				spawn->spawnBots();
		}
	}
}

void COutpostManager::autoSpawnEnd()
{
	if (!_AutoSpawn)
		return;
	FOREACH(itGroup, CCont<CGroup>, groups())
	{
		CGroupNpc* group = static_cast<CGroupNpc*>(*itGroup);
		if (group)
		{
			CSpawnGroupNpc* spawn = group->getSpawnObj();
			if (spawn)
				spawn->despawnBots(1);
		}
	}
}

void COutpostManager::setEnemies( TAllianceId attackerAllianceId )
{
	FOREACH( ig, CAliasCont<CGroup>, _Groups )
	{
		// Attack only the declared ennemies of the outpost
		CGroupNpc *grp = static_cast<CGroupNpc*>(*ig);
		grp->ennemyFaction().clearExtra();
		grp->ennemyFaction().addExtraProperty(attackerAllianceId);
	}
}

//////////////////////////////////////////////////////////////////////////////
// COutpostSquadManager                                                     //
//////////////////////////////////////////////////////////////////////////////

COutpostSquadManager::COutpostSquadManager(COutpost* parent, uint32 alias, std::string const& name, std::string const& filename)
: COutpostManager(parent, alias, name, filename)
{
	// Create init state
	CAIState* state = new CAIStatePositional(getStateMachine(), 0, "outpost_squad_init");
	IAIProfileFactory* aiProfile = lookupAIGrpProfile("squad");
	state->setActivityProfile(aiProfile);
	getStateMachine()->states().addChild(state);
	
	CAIEventDescription eventDescription;
	CAIEventActionNode::TSmartPtr eventAction;	
	CAIEventReaction* event;
	
	// Create event handler for start of state
	eventDescription.EventType = "start_of_state";
//	eventDescription.StateKeywords.push_back("");
//	eventDescription.NamedStates.push_back("");
//	eventDescription.GroupKeywords.push_back("");
//	eventDescription.NamedGroups.push_back("");
	
	// Create event action
	eventAction = new CAIEventActionNode;
	eventAction->Action = "code";
	eventAction->Weight = 1;
	eventAction->Args.push_back("print(\"CREATING A SQUAD\");");
//	eventAction->Args.push_back("()setFactionProp(\"faction\", \"foo\");");
//	eventAction->Args.push_back("()setFactionProp(\"ennemyFaction\", \"bar\");");
//	eventAction->Args.push_back("()setFactionProp(\"friendFaction\", \"baf\");");
	eventAction->Args.push_back("()setHealer(1);");
	
	// Register event action
	eventDescription.Action = eventAction;
	eventAction = NULL;
	
	// Register event handler
	event = new CAIEventReaction(getStateMachine(), 0, eventDescription.EventType);
	event->processEventDescription(&eventDescription, getStateMachine());
	getStateMachine()->eventReactions().addChild(event);
	event = NULL;
	
	// Create event handler for group death
	eventDescription.EventType = "group_eliminated";
//	eventDescription.StateKeywords.push_back("");
//	eventDescription.NamedStates.push_back("");
//	eventDescription.GroupKeywords.push_back("");
//	eventDescription.NamedGroups.push_back("");
	
	// Create event action
	eventAction = new CAIEventActionNode;	
	eventAction->Action = "outpost_report_squad_death";
	eventAction->Weight = 1;
//	eventAction->Args.push_back("");
	
	// Register event action
	eventDescription.Action = eventAction;
	eventAction = NULL;
	
	// Register event handler
	event = new CAIEventReaction(getStateMachine(), 0, eventDescription.EventType);
	event->processEventDescription(&eventDescription, getStateMachine());
	getStateMachine()->eventReactions().addChild(event);
	event = NULL;
	
	// Create event handler for bot death
	eventDescription.EventType = "bot_killed";
//	eventDescription.StateKeywords.push_back("");
//	eventDescription.NamedStates.push_back("");
//	eventDescription.GroupKeywords.push_back("");
//	eventDescription.NamedGroups.push_back("");
	
	// Create event action
	eventAction = new CAIEventActionNode;	
	eventAction->Action = "outpost_send_squad_status";
	eventAction->Weight = 1;
//	eventAction->Args.push_back("");
	
	// Register event action
	eventDescription.Action = eventAction;
	eventAction = NULL;
	
	// Register event handler
	event = new CAIEventReaction(getStateMachine(), 0, eventDescription.EventType);
	event->processEventDescription(&eventDescription, getStateMachine());
	getStateMachine()->eventReactions().addChild(event);
	event = NULL;
	
	// Create event handler for bot death
	eventDescription.EventType = "squad_leader_killed";
//	eventDescription.StateKeywords.push_back("");
//	eventDescription.NamedStates.push_back("");
//	eventDescription.GroupKeywords.push_back("");
//	eventDescription.NamedGroups.push_back("");
	
	// Create event action
	eventAction = new CAIEventActionNode;
	eventAction->Action = "multi_actions";
	eventAction->Weight = 1;
	eventAction->Children.resize(2);
	eventAction->Children[0] = new CAIEventActionNode;
	eventAction->Children[0]->Action = "outpost_report_squad_leader_death";
	eventAction->Children[0]->Weight = 1;
//	eventAction->Children[0]->Args.push_back("");
	eventAction->Children[1] = new CAIEventActionNode;
	eventAction->Children[1]->Action = "code";
	eventAction->Children[1]->Weight = 1;
	eventAction->Children[1]->Args.push_back("()setAutoSpawn(0);");
#ifdef NL_DEBUG
	eventAction->Children[1]->Args.push_back("print(\"INFINITE RESPAWN TIME FOR SQUAD\");");
#endif
	
	// Register event action
	eventDescription.Action = eventAction;
	eventAction = NULL;
	
	// Register event handler
	event = new CAIEventReaction(getStateMachine(), 0, eventDescription.EventType);
	event->processEventDescription(&eventDescription, getStateMachine());
	getStateMachine()->eventReactions().addChild(event);
	event = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void COutpost::createSquad(string const& dynGroupName, string const& stateMachineName, string const& initialStateName, string const& zoneName, uint32 spawnOrder, uint32 respawTimeGC, OUTPOSTENUMS::TPVPSide side)
{
	IManagerParent* const managerParent = getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	COutpostSpawnZone const* spawnZone = getZone(CStringMapper::map(zoneName));
	if (!spawnZone)
	{
		OUTPOST_WRN("newNpcChildGroup failed : spawnZone Not Found ! for StateMachine : %s", stateMachineName.c_str());
		return;
	}
	
	CGroupDesc<COutpostSquadFamily> const* groupDesc = NULL;
	
	FOREACH(itGroupDesc, CSquadLinks, _SquadLinks)
	{
		if ((*itGroupDesc).second->getName()==dynGroupName)
		{
			groupDesc = (*itGroupDesc).second;
			goto groupFound;
		}
	}
groupFound:
	if (!groupDesc)
	{
		OUTPOST_WRN("createSquad failed: No Group Found");
		return;
	}
	
	// Find the state machine as a manager
	CManager* manager = NULL;
	FOREACH(itCont, CCont<CManager>, aiInstance->managers())
	{
		if (itCont->getName()==stateMachineName)
		{
			manager = *itCont;
			break;
		}
	}
	if (!manager)
	{
		FOREACH(itCont, CCont<COutpostManager>, managers())
		{
			if (itCont->getName()==stateMachineName)
			{
				manager = *itCont;
				break;
			}
		}
	}
	if (!manager)
	{
		OUTPOST_WRN("createSquad failed : Unknown stateMachine %s", stateMachineName.c_str());
		return;
	}
	// Find the state machine as a npc manager
	COutpostSquadManager* outpostManager = dynamic_cast<COutpostSquadManager*>(manager);
	if (!outpostManager)
	{
		OUTPOST_WRN("createSquad failed : Not an outpost state machine !: ", stateMachineName.c_str());
		return;
	}
	createSquad(groupDesc, outpostManager, initialStateName, spawnZone, spawnOrder, respawTimeGC, side);
}

void COutpost::createSquad(uint32 dynGroupAlias, uint32 zoneAlias, uint32 spawnOrder, uint32 respawTimeGC, OUTPOSTENUMS::TPVPSide side)
{
	IManagerParent* const managerParent = getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	COutpostSpawnZone const* spawnZone = spawnZones().getChildByAlias(zoneAlias);
	if (!spawnZone)
	{
		OUTPOST_WRN("createSquad failed: spawn zone %s not found", LigoConfig.aliasToString(zoneAlias).c_str());
		return;
	}
	
	CGroupDesc<COutpostSquadFamily> const* groupDesc = NULL;
	
	//	Find the group template.
	// :TODO: Replace it with a faster map access.
	groupDesc = getSquad( dynGroupAlias );
	if (!groupDesc)
	{
		OUTPOST_WRN("createSquad failed: group %s not found", LigoConfig.aliasToString(dynGroupAlias).c_str());
		return;
	}
	
	// Find the state machine as a manager
	COutpostSquadManager* manager = dynamic_cast<COutpostSquadManager*>(managers().getChildByName("default_squad_manager"));
	if (!manager)
	{
		OUTPOST_WRN("createSquad failed: default state machine not found! (<- this is a code bug)");
		return;
	}
	createSquad(groupDesc, manager, "", spawnZone, spawnOrder, respawTimeGC, side);
}

void COutpost::createSquad(CGroupDesc<COutpostSquadFamily> const* groupDesc, COutpostSquadManager* manager, string const& initialStateName, COutpostSpawnZone const* spawnZone, uint32 createOrder, uint32 respawTimeGC, OUTPOSTENUMS::TPVPSide side)
{
	OUTPOST_DBG("Creating a squad");
	CAIVector const& spawnPosition = spawnZone->midPos();
	sint32 baseLevel = -1;
	double dispersionRadius = spawnZone->getRadius();
	// Save the creator state
	bool const savePlayerAttackable = groupDesc->getGDPlayerAttackable();
	bool const saveBotAttackable = groupDesc->getGDBotAttackable();
	// PlayerAttackable must be false as it it sent to the EGS in BotDescriptionMessage to set the
	// property 'Attackable'. Squads are not attackable unless a player is in war with the owner of
	// the outpost (using botchatprogram instead of the property 'Attackable').
	// BotAttackable must be true, otherwise the EGS sets it as invulnerable (not attackable).
	groupDesc->setGDPlayerAttackable(false);
	groupDesc->setGDBotAttackable(true);
	// Create the group
	CGroupNpc* const grp = groupDesc->createNpcGroup(manager, spawnPosition, dispersionRadius, baseLevel, false);
	// Restore the creator state
	groupDesc->setGDPlayerAttackable(savePlayerAttackable);
	groupDesc->setGDBotAttackable(saveBotAttackable);
	// Verify that the group was created
	if (!grp)
	{
		OUTPOST_WRN("createSquad failed: group %s cannot spawn", LigoConfig.aliasToString(groupDesc->getAlias()).c_str());
		return;
	}
	// Set the new group parameters
	// Let the EGS destroys squads explicitely because when a group is destroyed in the AIS its groupId can be reused
	// and 2 squads would have the same groupId in the EGS (it asserts)
	grp->autoDestroy(false);
	grp->setAutoSpawn(true);
	//	grp->getPersistentStateInstance()->setParentStateInstance(entity->getPersistentStateInstance());
	grp->initDynGrp(groupDesc, NULL);
	// Set color
	uint8 color = (uint8)(grp->getChildIndex() % 8);
	grp->setColour(color);
	// Get the state machine
	CStateMachine const* stateMachine = manager->getStateMachine();
#if !FINAL_VERSION
	// Verify that we have a state in the state machine
	if (stateMachine->cstStates().size()==0)
		nlerror("no state defined for StateMachine in Manager %s", manager->getFullName().c_str());
#endif
	// Set the group in that state
	CAIState* initialState = NULL;
	if (!initialStateName.empty())
	{
	//	FOREACH(itState, CCont<CAIState>, stateMachine->states())
		for (size_t i=0; i<stateMachine->cstStates().size(); ++i)
		{
			CAIState* state = stateMachine->cstStates()[(uint32)i];
			if (state->getName()==initialStateName)
				initialState = state;
		}
	}
	if (initialState==NULL)
		initialState = stateMachine->cstStates()[0];	//	sets the first state (must exist!).
	grp->setStartState(initialState);
	
	// Do not assist friends because it would lead to exploits
	/*if ( _OwnerGuild != InvalidGuildId )
	{
		//grp->faction().addExtraProperty(_OwnerGuild); // TODO: set the GuildId for the squad npcs?
		//grp->friendFaction().addExtraProperty(_OwnerGuild); // will be defended by the squad
	}
	else
	{
		// A tribe owns the outpost and all players are ennemies
		// CPropertyId tribeFaction(CGrpProfileFaction::fameFactionToScriptFaction(_OwnerTribe));
		//grp->faction().addProperty(tribeFaction);
		//grp->friendFaction().addProperty(tribeFaction); // will be defended by the squad
		//grp->ennemyFaction().addProperty(CPropertyId("Player"));
	}*/

	// Set guild id of members of the squad (for the tribe case, these *are* the tribe as well)
	// This was removed because CBotNpc::spawn() now uses getOwner()->getOwner()->getOwner() to access the outpsot
	/*FOREACH(ib, CAliasCont<CBot>, grp->bots())
	{
		CBotNpc *bot = static_cast<CBotNpc*>(*ib);
		bot->setOwnerOutpost(this);
	}*/
	
	grp->setOutpostSide(side);
	grp->setOutpostFactions(side);

	grp->_AggroRange = 25;
	grp->_UpdateNbTicks = 10;
	grp->respawnTime() = respawTimeGC;
	
	grp->updateStateInstance();	// Directly call his first state (to retrieve associated params).
	
	if (createOrder!=0)
	{
		COutpostSquadCreatedMsg params;
		params.Outpost = this->getAlias();
		params.CreateOrder = createOrder;
		// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
		//params.GroupId = reinterpret_cast<uint32>(grp);
		params.GroupId = (uint32)(size_t)(void*)grp;
		sendOutpostMessage("OUTPOST_SQUAD_CREATED", params);
	}

	OUTPOST_DBG( "Outpost %s: squad created defending 0x%x against 0x%x, respawnTime=%u gc", getName().c_str(), _OwnerAllianceId, _AttackerAllianceId, respawTimeGC );
}

void COutpost::spawnSquad(uint32 groupId)
{
	OUTPOST_DBG( "Outpost %s: Spawning squad 0x%08x", getName().c_str(), groupId );
	FOREACH(itManager, CAliasCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
			//uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			uint32 thisGroupId = (uint32)(size_t)(void*)groupNpc;
			if (groupId==thisGroupId)
			{
				group->getSpawnObj()->spawnBots();
				COutpostSquadSpawnedMsg params;
				params.Outpost = this->getAlias();
				params.GroupId = groupId;
				sendOutpostMessage("OUTPOST_SQUAD_SPAWNED", params);
				OUTPOST_DBG( "\t\tSpawned" );
				return;
			}
		}
	}
	OUTPOST_WRN("No squad to spawn with group id 0x%08x. Valid group ids are:", groupId);
	FOREACH(itManager, CAliasCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
			//uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			uint32 thisGroupId = (uint32)(size_t)(void*)groupNpc;
			OUTPOST_WRN("- 0x%08x", thisGroupId);
		}
	}
}

void COutpost::despawnSquad(uint32 groupId)
{
	OUTPOST_DBG( "Outpost %s: Despawning squad 0x%08x", getName().c_str(), groupId );
	FOREACH(itManager, CAliasCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
			//uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			uint32 thisGroupId = (uint32)(size_t)(void*)groupNpc;
			if (groupId==thisGroupId)
			{
				group->despawnBots();
				COutpostSquadDespawnedMsg params;
				params.Outpost = this->getAlias();
				params.GroupId = groupId;
				sendOutpostMessage("OUTPOST_SQUAD_DESPAWNED", params);
				return;
			}
		}
	}
	OUTPOST_WRN("No squad to despawn");
}

void COutpost::deleteSquad(uint32 groupId)
{
	OUTPOST_DBG( "Outpost %s: deleting squad 0x%08x", getName().c_str(), groupId );
	FOREACH(itManager, CAliasCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
			//uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			uint32 thisGroupId = (uint32)(size_t)(void*)groupNpc;
			if (groupId==thisGroupId)
			{
				manager->groups().removeChildByIndex(group->getChildIndex());
				// No message here since this is called by squad destructor in EGS
				return;
			}
		}
	}
	OUTPOST_WRN("No squad to delete");
}

void COutpost::sendOutpostSquadStatus(CGroupNpc* group)
{
	uint32 alias = this->getAlias();
	// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
	//uint32 groupId = reinterpret_cast<uint32>(group);
	uint32 groupId = (uint32)(size_t)(void*)group;
	bool groupAlive = false;
	bool leaderAlive = group->getSquadLeader()!=NULL;
	uint32 botCount = 0;
	// Persistent status
	// Spawn dependent status
	CSpawnGroup* spawnGroup = group->getSpawnObj();
	if (spawnGroup)
	{
		groupAlive = spawnGroup->isGroupAlive();
	}
	NLNET::CMessage msgout("OUTPOST_SQUAD_STATUS");
	msgout.serial(alias);
	msgout.serial(groupId);
	msgout.serial(groupAlive);
	msgout.serial(leaderAlive);
	msgout.serial(botCount);
	sendMessageViaMirror("EGS", msgout);
}

void COutpost::squadLeaderDied(CGroupNpc* group)
{
	uint32 alias = this->getAlias();
	// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
	//uint32 groupId = reinterpret_cast<uint32>(group);
	uint32 groupId = (uint32)(size_t)(void*)group;
	NLNET::CMessage msgout("OUTPOST_SQUAD_LEADER_DIED");
	msgout.serial(alias);
	msgout.serial(groupId);
	sendMessageViaMirror("EGS", msgout);
}

void COutpost::squadDied(CGroupNpc* group)
{
	uint32 alias = this->getAlias();
	// Bug #847: Just downcast the pointer to make the groupId, the collision in 64b is negligible
	//uint32 groupId = reinterpret_cast<uint32>(group);
	uint32 groupId = (uint32)(size_t)(void*)group;
	NLNET::CMessage msgout("OUTPOST_SQUAD_DIED");
	msgout.serial(alias);
	msgout.serial(groupId);
	sendMessageViaMirror("EGS", msgout);
}

void COutpost::triggerSpecialEvent(OUTPOSTENUMS::TSpecialOutpostEvent eventId)
{
	// Managers handlers
	FOREACH(itManager, CCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		if (manager)
		{
			CAIEvent* event = NULL;
			switch (eventId)
			{
			case OUTPOSTENUMS::PeaceStateBegin: event = &manager->EventOutpostPeaceStateBegin; break;
			case OUTPOSTENUMS::PeaceStateEnd: event = &manager->EventOutpostPeaceStateEnd; break;
			case OUTPOSTENUMS::TribeOwnershipBegin: event = &manager->EventOutpostTribeOwnershipBegin; break;
			case OUTPOSTENUMS::TribeOwnershipEnd: event = &manager->EventOutpostTribeOwnershipEnd; break;
			case OUTPOSTENUMS::GuildOwnershipBegin: event = &manager->EventOutpostGuildOwnershipBegin; break;
			case OUTPOSTENUMS::GuildOwnershipEnd: event = &manager->EventOutpostGuildOwnershipEnd; break;
			case OUTPOSTENUMS::OwnerChanged: event = &manager->EventOutpostOwnerChanged; break;
			case OUTPOSTENUMS::AttackerChanged: event = &manager->EventOutpostAttackerChanged; break;
			case OUTPOSTENUMS::StateChanged: event = &manager->EventOutpostStateChanged; break;
			}
			if (event)
			{
				FOREACH(itGroup, CCont<CGroup>, manager->groups())
				{
					CGroupNpc* group = static_cast<CGroupNpc*>(*itGroup);
					if (group)
						group->processStateEvent(*event);
				}
			}
		}
	}
	// Special handlers
	switch (eventId)
	{
	case OUTPOSTENUMS::TribeOwnershipBegin:
	{
		FOREACH(itManager, CCont<COutpostManager>, _Managers)
		{
			COutpostManager* manager = *itManager;
			if (manager)
				manager->autoSpawnBegin();
		}
		break;
	}
	case OUTPOSTENUMS::TribeOwnershipEnd:
	{
		FOREACH(itManager, CCont<COutpostManager>, _Managers)
		{
			COutpostManager* manager = *itManager;
			if (manager)
				manager->autoSpawnEnd();
		}
		break;
	}
	}
}

void COutpost::setBuildingBotSheet(uint32 buildingAlias, NLMISC::CSheetId sheetId, bool autoSpawnDespawn, const std::string & customName)
{
	CBot * bot = getBuildingBotByAlias(buildingAlias);
	if (bot == NULL)
	{
		OUTPOST_WRN( "cannot find building bot %s", LigoConfig.aliasToString( buildingAlias ).c_str() );
		return;
	}

	AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
	if (sheetId!=NLMISC::CSheetId::Unknown && !sheet.isNull())
	{
		bot->setCustomName(customName);
		bot->triggerSetSheet(sheet);
		if (autoSpawnDespawn && !bot->isSpawned())
			bot->spawn();
	}
	else
	{
		if (autoSpawnDespawn && bot->isSpawned())
			bot->despawnBot();
	}
}

void COutpost::despawnAllSquads()
{
	FOREACH(itManager, CAliasCont<COutpostManager>, _Managers)
	{
		COutpostManager* manager = *itManager;
		COutpostSquadManager* sqManager = dynamic_cast<COutpostSquadManager*>(manager);
		if (sqManager)
		{
			FOREACH(itGroup, CAliasCont<CGroup>, sqManager->groups())
			{
				CGroup* group = *itGroup;
				group->despawnBots();
			}
		}
	}
}

template <class T>
void COutpost::sendOutpostMessage(std::string const& msgName, T& paramStruct)
{
	NLNET::CMessage msgout(msgName);
	msgout.serial(paramStruct);
	sendMessageViaMirror(std::string("EGS"), msgout);
}

//////////////////////////////////////////////////////////////////////////////
// Commands                                                                 //
//////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND(displayOutposts, "list the available outpost", "")
{
	if (args.size() > 0)
		return false;
	
	uint32 instanceNumber = ~0;
	for (uint i=0; i<CAIS::instance().AIList().size(); ++i)
	{
		CAIInstance	*const	aii = CAIS::instance().AIList()[i];
		if (!aii)
		{
			log.displayNL("No current ai instance, can't look for outpost");
			continue;
		}
		
		string continentName;
		if (!args.empty())
			continentName = args[0];
		
		// retreive the oupost
		for (uint i=0; i<aii->continents().size(); ++i)
		{
			CContinent *const	continent = aii->continents()[i];
			if	(	!continent
				||	(	!continentName.empty()
				&&	continentName!=continent->getName()))
				continue;
			
			log.displayNL("Outposts in continent '%s':", continent->getName().c_str());
			for	(uint j=0; j<continent->outposts().size(); ++j)
			{
				COutpost const* outpost = continent->outposts()[j];
				if (!outpost)
					continue;
				
				log.displayNL("  %s", outpost->getOneLineInfoString().c_str());
				log.displayNL("    - %d group descs", outpost->squadLinks().size());
				for	(COutpost::CSquadLinks::const_iterator isl=outpost->squadLinks().begin(); isl!=outpost->squadLinks().end(); ++isl )
				{
					CGroupDesc<COutpostSquadFamily> const* gd = (*isl).second;
					if (!gd)
						continue;
					
					log.displayNL("      Group desc '%s' %s", gd->getName().c_str(), gd->getAliasString().c_str());
				}

				log.displayNL("  - %d spawn zones", outpost->spawnZones().size());
				for	(uint j=0; j<outpost->spawnZones().size(); ++j)
				{
					COutpostSpawnZone const* sz = outpost->spawnZones()[j];
					if (!sz)
						continue;
					
					log.displayNL("    SpawnZone '%s' %s (%s)", sz->getName().c_str(), sz->getAliasString().c_str(), sz->midPos().toString().c_str());
				}
				log.displayNL("  - %d state machines", outpost->managers().size());
				for	(uint j=0; j<outpost->managers().size(); ++j)
				{
					COutpostManager* manager = outpost->managers()[j];
					if (!manager)
						continue;
					
					log.displayNL("    State machine '%s' %s", manager->getName().c_str(), manager->getAliasString().c_str());
					log.displayNL("    - %d states", manager->getStateMachine()->states().size());
					for	(uint j=0; j<manager->getStateMachine()->states().size(); ++j)
					{
						CAIState const* state = manager->getStateMachine()->states()[j];
						if (!state)
							continue;
						
						log.displayNL("      State '%s' %s", state->getName().c_str(), state->getAliasString().c_str());
					}
					log.displayNL("    - %d groups", manager->groups().size());
					for	(uint j=0; j<manager->groups().size(); ++j)
					{
						CGroup const* group = manager->groups()[j];
						if (group)
						{
							log.displayNL("      Group '%s' %s", group->getName().c_str(), group->getAliasString().c_str());
							log.displayNL("      - %d bots", group->bots().size());
							for	(uint k=0; k<group->bots().size(); ++k)
							{
								CBot const* bot = group->bots()[k];
								if (bot)
									log.displayNL("        Bot '%s' %s", bot->getName().c_str(), bot->getAliasString().c_str());
								else
									log.displayNL("        Bot <NULL>");
							}
						}
						else
							log.displayNL("      Group <NULL>");
					}
				}
			}
			
		}
	}
	return true;
}

// outpostSpawnSquad 1 fyros outpost group machine zone

NLMISC_COMMAND(outpostSpawnSquad, "Spawns a squad in an outpost", "<instance_number> <continent> <outpost> <group_template> <state_machine> <spawn_zone> [<respawnTimeInSec>=5*60]")
{
	if (args.size() != 6)
		return false;
	
	uint32 instanceNumber = ~0;
	fromString(args[0], instanceNumber);
	string continentName = args[1];
	string outpostName = args[2];
	string dynGroupName = args[3];
	string stateMachineName = args[4];
	string zoneName = args[5];
	uint32 respawnTimeGC = 5*60*10;
	if ( args.size() > 6 )
	{
		NLMISC::fromString(args[6], respawnTimeGC);
		respawnTimeGC *= 10;
	}
	
	for (size_t i=0; i<5; ++i)
		if (args[i]=="")
			return false;
	
	bool done = false;
	bool instanceFound = false;
	bool continentFound = false;
	bool outpostFound = false;
	if (instanceNumber<CAIS::instance().AIList().size())
	{
		CAIInstance* const aiinstance = CAIS::instance().AIList()[instanceNumber];
		if (aiinstance)
		{
			instanceFound = true;
			for (uint i=0; i<aiinstance->continents().size() && !done; ++i)
			{
				CContinent* const continent = aiinstance->continents()[i];
				if (!continent || continentName!=continent->getName())
					continue;
				continentFound = true;
				for	(uint j=0; j<continent->outposts().size() && !done; ++j)
				{
					COutpost* const outpost = continent->outposts()[j];
					if (!outpost || outpostName!=outpost->getName())
						continue;
					outpostFound = true;
					outpost->createSquad(dynGroupName, stateMachineName, "", zoneName, 0, respawnTimeGC, OUTPOSTENUMS::UnknownPVPSide);
					done = true;
				}
			}
		}
	}
	if (!done)
	{
		OUTPOST_WRN("Could not spawn a new squad");
		log.displayNL("Could not spawn a new squad");
		if (!instanceFound)
			log.displayNL( "Instance not found" );
		if (!continentFound)
			log.displayNL( "Continent not found" );
		if (!outpostFound)
			log.displayNL( "Outpost not found" );
	}
	return true;
}


/*
 * Return the outpost corresponding to the alias, or NULL if not found (with a nlwarning)
 */
COutpost* COutpost::getOutpostByAlias( TAIAlias outpostAlias )
{
	bool instanceFound = false;
	bool continentFound = false;
	bool outpostFound = false;
	for (uint i=0; i<CAIS::instance().AIList().size(); ++i)
	{
		CAIInstance* const aiinstance = CAIS::instance().AIList()[i];
		if (aiinstance)
		{
			instanceFound = true;
			for (uint i=0; i<aiinstance->continents().size(); ++i)
			{
				CContinent* const continent = aiinstance->continents()[i];
				if (!continent)
					continue;
				continentFound = true;
				for	(uint j=0; j<continent->outposts().size(); ++j)
				{
					COutpost* outpost = continent->outposts()[j];
					if (!outpost || outpostAlias!=outpost->getAlias())
						continue;
					return outpost;
				}
			}
		}
	}
	if (!instanceFound)
		OUTPOST_WRN( "Instance not found" );
	if (!continentFound)
		OUTPOST_WRN( "Continent not found" );
	if (!outpostFound)
		OUTPOST_WRN( "Outpost %s not found", LigoConfig.aliasToString( outpostAlias ).c_str() );
	return NULL;
}

/*
void outpostCreateSquad(uint32 outpostAlias, uint32 dynGroupAlias, uint32 zoneAlias, uint32 spawnOrder)
{
	COutpost *outpost = COutpost::getOutpostByAlias( outpostAlias );
	if ( outpost )
		outpost->createSquad(dynGroupAlias, zoneAlias, spawnOrder);
}

void outpostSpawnSquad(uint32 outpostAlias, uint32 groupId)
{
	COutpost *outpost = COutpost::getOutpostByAlias( outpostAlias );
	if ( outpost )
		outpost->spawnSquad(groupId);
}

void outpostDespawnSquad(uint32 outpostAlias, uint32 groupId)
{
	COutpost *outpost = COutpost::getOutpostByAlias( outpostAlias );
	if ( outpost )
		outpost->despawnSquad(groupId);
}

void outpostDeleteSquad(uint32 outpostAlias, uint32 groupId)
{
	COutpost *outpost = COutpost::getOutpostByAlias( outpostAlias );
	if ( outpost )
		outpost->deleteSquad(groupId);
}

void outpostDespawnAllSquads(uint32 outpostAlias)
{
	COutpost *outpost = COutpost::getOutpostByAlias( outpostAlias );
	if ( outpost )
		outpost->despawnAllSquads();
}
*/

NLMISC_COMMAND(outpostAliasSpawnSquad, "Spawns a squad in an outpost (respawn time = 5 min)", "<outpost_alias> <group_alias> <spawn_zone_alias>")
{
	if (args.size()!=3)
		return false;
	
	for (size_t i=0; i<3; ++i)
		if (args[i]=="")
			return false;
	
	uint32 outpostAlias = LigoConfig.aliasFromString(args[0]);
	uint32 dynGroupAlias = LigoConfig.aliasFromString(args[1]);
	uint32 zoneAlias = LigoConfig.aliasFromString(args[2]);
	string stateMachineName = "default_squad_manager";
	
	COutpost *outpost = COutpost::getOutpostByAlias(outpostAlias);
	if (outpost)
		outpost->createSquad(dynGroupAlias, zoneAlias, 0, 5*60*10, OUTPOSTENUMS::UnknownPVPSide);
	
	return true;
}

CBot* COutpost::getBuildingBotByAlias(TAIAlias alias)
{
	CGroup* group = getBuildingGroup();
	if (group)
		return group->bots().getChildByAlias(alias);
	else
		return NULL;
}

NLMISC_COMMAND(outpostSetBuildingBotSheet, "Set the sheet of an outpost building", "<outpost_alias> <building_alias> <bot_sheet>")
{
	if (args.size()!=3)
		return false;
	
	for (size_t i=0; i<3; ++i)
		if (args[i]=="")
			return false;
	
	uint32 outpostAlias = LigoConfig.aliasFromString(args[0]);
	uint32 buildingAlias = LigoConfig.aliasFromString(args[1]);
	NLMISC::CSheetId buildingBotSheet = NLMISC::CSheetId(args[2]);;
	
	COutpost* outpost = COutpost::getOutpostByAlias(outpostAlias);
	if (outpost)
		outpost->setBuildingBotSheet(buildingAlias, buildingBotSheet, true, "");
	
	return true;
}


// Call this macro, then do the code that will must done only if the outpost is found in this instance
// The data you have access to after this macro:
// - COutpost *outpost
// - msgStruct params;
#define IF_GET_OUTPOST_FOR_MSG( msgStruct ) \
	msgStruct params; \
	msgin.serial( params ); \
	COutpost *outpost = COutpost::getOutpostByAlias( params.Outpost ); \
if ( outpost )


void cbOutpostCreateSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostCreateSquadMsg )
		outpost->createSquad(params.Group, params.Zone, params.CreateOrder, params.RespawnTimeS*10, params.Side);
}

void cbOutpostSpawnSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostSpawnSquadMsg )
		outpost->spawnSquad(params.GroupId);
}

void cbOutpostDespawnSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostDespawnSquadMsg )
		outpost->despawnSquad(params.GroupId);
}

void cbOutpostDeleteSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostDeleteSquadMsg )
		outpost->deleteSquad(params.GroupId);
}

void cbOutpostDespawnAllSquads( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostDespawnAllSquadsMsg )
		outpost->despawnAllSquads();
}

void cbOutpostSetOwner( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( CSetOutpostOwner )
		outpost->setOwnerAlliance( params.Owner );
}

void cbOutpostSetAttacker( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( CSetOutpostAttacker )
		outpost->setAttackerAlliance( params.Attacker );
}

void cbOutpostSetState( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostSetStateMsg )
		outpost->setState( params.State );
}

void cbOutpostEvent( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostEventMsg )
		outpost->triggerSpecialEvent(params.Event);
}

void cbOutpostSetBuildingBotSheet( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	IF_GET_OUTPOST_FOR_MSG( COutpostSetBuildingBotSheetMsg )
		outpost->setBuildingBotSheet(params.Building, params.SheetId, params.AutoSpawnDespawn, params.CustomName);
}


#include "event_reaction_include.h"
