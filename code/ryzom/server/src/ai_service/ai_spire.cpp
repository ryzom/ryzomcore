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
#include "ai_spire.h"

#include "ai_instance.h"
#include "ais_actions.h"

using namespace	AITYPES;
using namespace std;
using namespace CAISActionEnums;


extern CAIInstance* currentInstance;

DEFINE_ACTION(ContextGlobal, SPIREMGR)
{
	nlassertex(currentInstance != NULL, ("No AIInstance created !"));
	CAIInstance* aiInstance = currentInstance ;
	CWorkPtr::aiInstance(aiInstance);	// set the current AIInstance.
	
	// get hold of the manager's slot id - note that managers are identified by slot and not by alias!
	uint32 alias;
	string spireName;
	string mapName;
	string filename;
	if (!getArgs(args, name(), alias, spireName, mapName, filename))
		return;
	
	string mgrName = "spire_manager_"+spireName;
	
	// see whether the manager is already loaded
	CManager* mgr = aiInstance->managers().getChildByName(mgrName);
	
	// not found so look for a free slot
	if (!mgr)
		aiInstance->newMgr(MgrTypeNpc, 0, mgrName, mapName, filename);
	else
		mgr->registerForFile(filename);		
	
	mgr = aiInstance->managers().getChildByName(mgrName);
	CMgrNpc* mgrNpc = dynamic_cast<CMgrNpc*>(mgr);
	if (!mgrNpc)
		return;
	
	// setup the working manager pointer and exit
	CWorkPtr::mgr(mgrNpc);
	CWorkPtr::eventReactionContainer(mgrNpc->getStateMachine());
	
	// set workptr state to this state
	CContextStack::setContext(ContextNpcMgr);
}

DEFINE_ACTION(ContextEventContainer, SPIRSTAT) // spire state
{
	CStateMachine* container = CWorkPtr::eventReactionContainer();
	if (!container)
		return;
	
	uint32 alias;
	string spireName;
	if (!getArgs(args, name(), alias, spireName))
		return;
	
	string stateName = "spire_state_"+spireName;
	
	CAIStatePositional* state = new CAIStatePositional(container, alias, stateName);
	container->states().addAliasChild(state);
	// set workptr::state to this state
	CWorkPtr::stateState(container->states()[0]);
	if (!CWorkPtr::stateState())
	{
		nlwarning("Failed to select state %s", LigoConfig.aliasToString(alias).c_str());
		return;
	}
	
	// set workptr state to this state
	CContextStack::setContext(ContextPositionalState);
}

DEFINE_ACTION(ContextNpcMgr, SPIREGRP)
{
	CMgrNpc* mgr = CWorkPtr::mgrNpc();
	if (!mgr)
		return;
	
	uint32 alias;
	string spireName;
	if (!getArgs(args, name(), alias, spireName))
		return;
	
	string grpName = "spire_group_"+spireName;
	
	CGroupNpc* grp = new CGroupNpc(mgr, alias, grpName, RYAI_MAP_CRUNCH::Nothing);
	grp->setAutoSpawn(false);
	mgr->groups().addChild(grp);
	CWorkPtr::grp(grp);
	if (!CWorkPtr::grpNpc())
	{
		nlwarning("Failed to select spire group %s as not found in manager: %s",
			grpName.c_str(),
			CWorkPtr::mgrNpc()->getName().c_str());
		return;
	}
	CStateMachine* stateMachine = CWorkPtr::eventReactionContainer();
	if (stateMachine)
		grp->setStartState(stateMachine->cstStates()[0]);
	
	CContextStack::setContext(ContextNpcGrp);
}

DEFINE_ACTION(ContextNpcGrp, SPIREBOT)
{
	CGroupNpc* grp = CWorkPtr::grpNpc();
	if (!grp)
		return;
	
	uint32 alias;
	string spireName;
	if (!getArgs(args, name(), alias, spireName))
		return;
	
	string botName = "spire_bot_"+spireName;
	
	CBotNpc* bot = new CBotNpc(grp, alias, botName);
	grp->bots().addChild(bot);
	CWorkPtr::bot(bot);
	if (!CWorkPtr::botNpc())
	{
		nlwarning("Failed to select spire bot %s as not found in group: %s",
			botName.c_str(),
			CWorkPtr::grpNpc()->getName().c_str());
		return;
	}
	
	if (bot->getChat().isNull())
		bot->newChat();
	bot->getChat()->add(bot->getAIInstance(), "op:spire");

	bot->setStuck(true);
	
	// set workptr state to this state
	CContextStack::setContext(ContextNpcBot);
}

DEFINE_ACTION(ContextNpcBot, SPIRSHTS)
{
	CGroupNpc* grp = CWorkPtr::grpNpc();
	if (!grp)
		return;
	
	CBotNpc* bot = CWorkPtr::botNpc();
	if (!bot)
		return;
	
	FOREACHC(itArg, std::vector<CAIActions::CArg>, args)
	{
		string str, name, sheet;
		itArg->get(str);
		AI_SHARE::stringToKeywordAndTail(str, name, sheet);
		grp->setStrLogicVar(NLMISC::CStringMapper::map("$sheet_spire_"+name), sheet);
	}
}





#if 0
#include "continent.h"
#include "ai_grp_npc.h"
#include "game_share/people.h"
#include "ai_profile_npc.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

using namespace std;
using namespace NLMISC;



CFileDisplayer SpireDisplayer("spires.log");
CLog SpireDbgLog(CLog::LOG_DEBUG), SpireInfLog(CLog::LOG_INFO), SpireWrnLog(CLog::LOG_WARNING), SpireErrLog(CLog::LOG_ERROR);


namespace SPIREHELPERS {

static std::map<uint32, uint32> spireFactions;

bool isAttackingFaction(uint32 factionIndex, CAIEntityPhysical const* player)
{
	std::map<uint32, uint32>::const_iterator it = spireFactions.find(player->spireAlias());
	return it!=spireFactions.end() && it->second==factionIndex;
}

}

//////////////////////////////////////////////////////////////////////////////
// CSpire                                                                 //
//////////////////////////////////////////////////////////////////////////////

//std::set<CContinent*>		ChangedSpires;
//CSpire::TSpireIndex CSpire::_Spires;

CSpire::CSpire(CContinent* owner, uint32 alias, std::string const& name, std::string const& filename)
: CAliasChild<CContinent>(owner, alias, name)
, CAliasTreeRoot(filename)
, _OwnerAllianceId(InvalidAllianceId)
, _AttackerAllianceId(InvalidAllianceId)
, _State(SPIREENUMS::UnknownSpireState)

{
	static bool logInitDone = false;
	if ( ! logInitDone )
	{
		SpireDbgLog.addDisplayer( &SpireDisplayer );
		SpireInfLog.addDisplayer( &SpireDisplayer );
		SpireWrnLog.addDisplayer( &SpireDisplayer );
		logInitDone = true;
	}

	SPIRE_DBG("Creating spire %s' (%s)", name.c_str(), getAliasFullName().c_str());
	if (LogSpireDebug)
		SPIRE_DBG("Creating spire '%s'", getAliasFullName().c_str());
	
	_SpireName = getName();
	CSpireManager* manager = NULL;
	// Create default squad manager
	manager = new CSpireSquadManager(this, 0, "default_squad_manager", filename);
	if (manager)
	{
		_Managers.addChild(manager);
		manager->init();
	}
	// Create default building manager
	manager = new CSpireManager(this, 0, "default_building_manager", filename);
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

CSpire::~CSpire()
{
	if (LogSpireDebug)
		SPIRE_DBG("Deleting spire '%s'", getAliasFullName().c_str());

	_SpawnZones.clear();
}

std::string	CSpire::getOneLineInfoString() const
{
	return std::string("Spire '") + getName() + "' " + getAliasString() + ", State=" + SPIREENUMS::toString(_State);
}

std::string CSpire::getFullName() const
{
	return std::string(getOwner()->getFullName() +":"+ getName());
}

std::string CSpire::getIndexString() const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":o%u", getChildIndex());
}
/*
std::string	CSpire::getPlaceOwnerFullName() const
{
	return getFullName();
}

std::string	CSpire::getPlaceOwnerIndexString() const
{
	return getIndexString();
}
*/
CAIInstance* CSpire::getAIInstance() const
{
	return getOwner()->getAIInstance();
}

std::string	CSpire::getManagerIndexString(CManager const* manager) const
{
	return getIndexString()+NLMISC::toString(":m%u", manager->getChildIndex());
}

IAliasCont* CSpire::getAliasCont(TAIType type)
{
	switch (type)
	{
	case AITypeSpireSquadFamily:
		SPIRE_WRN( "Obsolete squad family node under spire %s", getName().c_str() );
		return NULL;
	case AITypeSpireSpawnZone:
		return &_SpawnZones;
	case AITypeManager:
		return &_Managers;
	case AITypeSpireManager:
		return &_Managers;
	case AITypeSpireBuilding:
		return getBuildingGroup()->getAliasCont(AITypeBot);
	default:
		return NULL;
	}

}

CAliasTreeOwner* CSpire::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner*	child	=	NULL;

	switch(aliasTree->getType())
	{
		//	create the child and adds it to the corresponding position.
	case AITypeSpireSpawnZone:
		child = new CSpireSpawnZone(this, aliasTree);
		break;
	case AITypeManager:
		child = new CSpireSquadManager(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	case AITypeSpireManager:
		child = new CSpireSquadManager(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	case AITypeSpireBuilding:
		return getBuildingGroup()->createChild(cont, aliasTree);
	}

	if (child)
		cont->addAliasChild(child);
	return	child;
}

CGroup* CSpire::getBuildingGroup()
{
	CSpireManager* manager = _Managers.getChildByName("default_building_manager");
	return manager->groups().getChildByName("default_building_group");
}

void CSpire::setTribe(const std::string &tribeName)
{
	// The tribe is only populated by the NPCs of the squads
	_Tribe = tribeName;
	if (LogSpireDebug)
		SPIRE_DBG("setting tribe '%s' in '%s' as owner for spire '%s'",
			tribeName.c_str(),
			getOwner()->getName().c_str(),
			getAliasFullName().c_str());
}

void CSpire::setOwnerAlliance( TAllianceId ownerAllianceId )
{
	if (_OwnerAllianceId!=ownerAllianceId)
	{
		// if the old owner was a tribe
		if (_OwnerAllianceId==InvalidAllianceId)
		{
			triggerSpecialEvent(SPIREENUMS::TribeOwnershipEnd);
			SPIREHELPERS::spireFactions.erase(getAlias());
		}
		else
			triggerSpecialEvent(SPIREENUMS::GuildOwnershipEnd);
	}
	std::swap(_OwnerAllianceId, ownerAllianceId); // 'ownerAllianceId' contains old owner alliance id
	if (_OwnerAllianceId!=ownerAllianceId)
	{
		triggerSpecialEvent(SPIREENUMS::OwnerChanged);
		// if the new owner is a tribe
		if (_OwnerAllianceId==InvalidAllianceId)
		{
			SPIREHELPERS::spireFactions.insert(make_pair(getAlias(), CStaticFames::getInstance().getFactionIndex(_Tribe)));
			triggerSpecialEvent(SPIREENUMS::TribeOwnershipBegin);
		}
		else
			triggerSpecialEvent(SPIREENUMS::GuildOwnershipBegin);
	}
}

void CSpire::setAttackerAlliance( TAllianceId attackerAllianceId )
{
	std::swap(_AttackerAllianceId, attackerAllianceId); // 'attackerAllianceId' contains old attacker alliance id
	if (_AttackerAllianceId!=attackerAllianceId)
	{
		triggerSpecialEvent(SPIREENUMS::AttackerChanged);
	}
	
	// Update the enemies of the current squads of the spire
	FOREACH( im, CAliasCont<CSpireManager>, _Managers )
		im->setEnemies( _AttackerAllianceId );
}

void CSpire::setState( SPIREENUMS::TSpireState state )
{
	if (_State!=state)
	{
		if (_State==SPIREENUMS::Peace)
			triggerSpecialEvent(SPIREENUMS::PeaceStateEnd);
	}
	std::swap(_State, state); // 'state' contains old _State
	if (_State!=state)
	{
		triggerSpecialEvent(SPIREENUMS::StateChanged);
		if (_State==SPIREENUMS::Peace)
			triggerSpecialEvent(SPIREENUMS::PeaceStateBegin);
	}
}

void CSpire::update()
{
//	FOREACH(it, CAliasCont<CSpireSquadFamily>, _SquadFamilies) it->update();
//	FOREACH(it, CAliasCont<CSpireSpawnZone>,   _SpawnZones)    it->update();
	FOREACH(it, CAliasCont<CSpireManager>,     _Managers)      it->update();
}

void CSpire::addZone(CSpireSpawnZone* zone)
{
#if !FINAL_VERSION
	if (_ZoneList.find(NLMISC::CStringMapper::map(zone->getName()))!=_ZoneList.end())
		SPIRE_WRN("this SpireSpawnZone have the same name than another: %s", zone->getName().c_str());
#endif
	_ZoneList[NLMISC::CStringMapper::map(zone->getName())] = zone;
}

void CSpire::removeZone(CSpireSpawnZone* zone)
{
	_ZoneList.erase(NLMISC::CStringMapper::map(zone->getName()));
}

CSpireSpawnZone* CSpire::getZone(NLMISC::TStringId zoneName)
{
	return _ZoneList[zoneName];
}

void CSpire::serviceEvent(CServiceEvent const& info)
{
	if (info.getServiceName()=="EGS" && info.getEventType()==CServiceEvent::SERVICE_DOWN)
	{
		// Delete all the squad groups
		CSpireManager* manager = _Managers.getChildByName("default_squad_manager");
		if (manager)
			manager->groups().clear();
	}
	FOREACH(itManager, CAliasCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		manager->serviceEvent(info);
	}
}

bool CSpire::spawn()
{
	// Spawn spire managers
	CCont<CSpireManager>::iterator itManager, itManagerEnd(_Managers.end());
	for (itManager=_Managers.begin(); itManager!=itManagerEnd; ++itManager)
	{
		CSpireManager* manager = *itManager;
		manager->spawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

bool CSpire::despawn()
{
	// Despawn instance managers
	CCont<CSpireManager>::iterator itManager, itManagerEnd(_Managers.end());
	for (itManager=_Managers.begin(); itManager!=itManagerEnd; ++itManager)
	{
		CSpireManager* manager = *itManager;
		manager->despawnMgr();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

//-----------------------------------------------------------------------------
std::string CSpire::getStateName() const
{
	return SPIREENUMS::toString(_State);
}

//////////////////////////////////////////////////////////////////////////////
// CSpireSquadFamily                                                      //
//////////////////////////////////////////////////////////////////////////////

IAliasCont* CSpireSquadFamily::getAliasCont(TAIType type)
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

CAliasTreeOwner* CSpireSquadFamily::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
{
	if (!cont)
		return	NULL;
	
	CAliasTreeOwner* child = NULL;
	
	switch (aliasTree->getType())
	{
	case AITypeSquadTemplateVariant:
		child = new CGroupDesc<CSpireSquadFamily>(this, aliasTree->getAlias(), aliasTree->getParent()->getName()+":"+aliasTree->getName());
		break;
	case AITypeGroupTemplate:
	case AITypeGroupTemplateMultiLevel:
		child = new CGroupDesc<CSpireSquadFamily>(this, aliasTree->getAlias(), aliasTree->getName());
		break;
	}
	
	if (child)
		cont->addAliasChild(child);
	return child;
}

std::string CSpireSquadFamily::getFullName() const
{
//	return std::string(getOwner()->getFullName() +":"+ getName());
	return getName();
}

std::string CSpireSquadFamily::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":sf%u", getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// CGroupDesc                                                               //
//////////////////////////////////////////////////////////////////////////////

template <> size_t const CGroupDesc<CSpireSquadFamily>::_MultiLevelSheetCount = 20;

//////////////////////////////////////////////////////////////////////////////
// CBotDesc                                                                 //
//////////////////////////////////////////////////////////////////////////////

template <> size_t const CBotDesc<CSpireSquadFamily>::_MultiLevelSheetCount = 20;

//////////////////////////////////////////////////////////////////////////////
// CSpireSpawnZone                                                        //
//////////////////////////////////////////////////////////////////////////////

CSpireSpawnZone::CSpireSpawnZone(CSpire* owner, CAIAliasDescriptionNode* adn)
: CAIPlaceXYR(owner, adn)
{
	owner->addZone(this);
}

CSpireSpawnZone::~CSpireSpawnZone()
{
	static_cast<CSpire*>(getOwner())->removeZone(this);
}

std::string CSpireSpawnZone::getFullName() const
{
	return std::string(getOwner()->getFullName() +":"+ getName());
}

std::string CSpireSpawnZone::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}

//////////////////////////////////////////////////////////////////////////////
// CSpireManager                                                          //
//////////////////////////////////////////////////////////////////////////////

CSpireManager::CSpireManager(CSpire* parent, uint32 alias, std::string const& name, std::string const& filename)
: CMgrNpc(parent, alias, name, filename)
, _AutoSpawn(false)
{
	registerEvents();
}

CSpireManager::~CSpireManager()
{
	unregisterEvents(); // need be called otherwise the state manager will be unhappy when the CAIEvent objects are destroyed
}

std::string	CSpireManager::getOneLineInfoString() const
{
	return std::string("Spire manager '") + getName() + "'";
}

void CSpireManager::update()
{
	CMgrNpc::update();
}

IAliasCont* CSpireManager::getAliasCont(TAIType type)
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

CAliasTreeOwner* CSpireManager::createChild(IAliasCont* cont, CAIAliasDescriptionNode* aliasTree)
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

void CSpireManager::registerEvents()
{
	_StateMachine.addEvent(	"spire_peace_state_begin",		EventSpirePeaceStateBegin		);
	_StateMachine.addEvent(	"spire_peace_state_end",			EventSpirePeaceStateEnd		);
	_StateMachine.addEvent(	"spire_tribe_ownership_begin",	EventSpireTribeOwnershipBegin	);
	_StateMachine.addEvent(	"spire_tribe_ownership_end",		EventSpireTribeOwnershipEnd	);
	_StateMachine.addEvent(	"spire_guild_ownership_begin",	EventSpireGuildOwnershipBegin	);
	_StateMachine.addEvent(	"spire_guild_ownership_end",		EventSpireGuildOwnershipEnd	);
	_StateMachine.addEvent(	"spire_owner_changed",			EventSpireOwnerChanged		);
	_StateMachine.addEvent(	"spire_attacker_changed",			EventSpireAttackerChanged		);
	_StateMachine.addEvent(	"spire_state_changed",			EventSpireStateChanged		);
}

void CSpireManager::unregisterEvents()
{
	_StateMachine.delEvent(	"spire_peace_state_begin" );
	_StateMachine.delEvent(	"spire_peace_state_end" );
	_StateMachine.delEvent(	"spire_tribe_ownership_begin" );
	_StateMachine.delEvent(	"spire_tribe_ownership_end" );
	_StateMachine.delEvent(	"spire_guild_ownership_begin" );
	_StateMachine.delEvent(	"spire_guild_ownership_end" );
	_StateMachine.delEvent(	"spire_owner_changed" );
	_StateMachine.delEvent(	"spire_attacker_changed" );
	_StateMachine.delEvent(	"spire_state_changed"	);
}

void CSpireManager::autoSpawnBegin()
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

void CSpireManager::autoSpawnEnd()
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

void CSpireManager::setEnemies( TAllianceId attackerAllianceId )
{
	FOREACH( ig, CAliasCont<CGroup>, _Groups )
	{
		// Attack only the declared ennemies of the spire
		CGroupNpc *grp = static_cast<CGroupNpc*>(*ig);
		grp->ennemyFaction().clearExtra();
		grp->ennemyFaction().addExtraProperty(attackerAllianceId);
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSpireSquadManager                                                     //
//////////////////////////////////////////////////////////////////////////////

CSpireSquadManager::CSpireSquadManager(CSpire* parent, uint32 alias, std::string const& name, std::string const& filename)
: CSpireManager(parent, alias, name, filename)
{
	// Create init state
	CAIState* state = new CAIStatePositional(getStateMachine(), 0, "spire_squad_init");
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
	eventAction->Action = "spire_report_squad_death";
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
	eventAction->Action = "spire_send_squad_status";
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
	eventAction->Children[0]->Action = "spire_report_squad_leader_death";
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

void CSpire::createSquad(string const& dynGroupName, string const& stateMachineName, string const& initialStateName, string const& zoneName, uint32 spawnOrder, uint32 respawTimeGC, SPIREENUMS::TPVPSide side)
{
	IManagerParent* const managerParent = getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CSpireSpawnZone const* spawnZone = getZone(CStringMapper::map(zoneName));
	if (!spawnZone)
	{
		SPIRE_WRN("newNpcChildGroup failed : spawnZone Not Found ! for StateMachine : %s", stateMachineName.c_str());
		return;
	}
	
	CGroupDesc<CSpireSquadFamily> const* groupDesc = NULL;
	
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
		SPIRE_WRN("createSquad failed: No Group Found");
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
		FOREACH(itCont, CCont<CSpireManager>, managers())
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
		SPIRE_WRN("createSquad failed : Unknown stateMachine %s", stateMachineName.c_str());
		return;
	}
	// Find the state machine as a npc manager
	CSpireSquadManager* spireManager = dynamic_cast<CSpireSquadManager*>(manager);
	if (!spireManager)
	{
		SPIRE_WRN("createSquad failed : Not an spire state machine !: ", stateMachineName.c_str());
		return;
	}
	createSquad(groupDesc, spireManager, initialStateName, spawnZone, spawnOrder, respawTimeGC, side);
}

void CSpire::createSquad(uint32 dynGroupAlias, uint32 zoneAlias, uint32 spawnOrder, uint32 respawTimeGC, SPIREENUMS::TPVPSide side)
{
	IManagerParent* const managerParent = getOwner()->getOwner();
	CAIInstance* const aiInstance = dynamic_cast<CAIInstance*>(managerParent);
	if (!aiInstance)
		return;
	
	CSpireSpawnZone const* spawnZone = spawnZones().getChildByAlias(zoneAlias);
	if (!spawnZone)
	{
		SPIRE_WRN("createSquad failed: spawn zone %s not found", LigoConfig.aliasToString(zoneAlias).c_str());
		return;
	}
	
	CGroupDesc<CSpireSquadFamily> const* groupDesc = NULL;
	
	//	Find the group template.
	// :TODO: Replace it with a faster map access.
	groupDesc = getSquad( dynGroupAlias );
	if (!groupDesc)
	{
		SPIRE_WRN("createSquad failed: group %s not found", LigoConfig.aliasToString(dynGroupAlias).c_str());
		return;
	}
	
	// Find the state machine as a manager
	CSpireSquadManager* manager = dynamic_cast<CSpireSquadManager*>(managers().getChildByName("default_squad_manager"));
	if (!manager)
	{
		SPIRE_WRN("createSquad failed: default state machine not found! (<- this is a code bug)");
		return;
	}
	createSquad(groupDesc, manager, "", spawnZone, spawnOrder, respawTimeGC, side);
}

void CSpire::createSquad(CGroupDesc<CSpireSquadFamily> const* groupDesc, CSpireSquadManager* manager, string const& initialStateName, CSpireSpawnZone const* spawnZone, uint32 createOrder, uint32 respawTimeGC, SPIREENUMS::TPVPSide side)
{
	SPIRE_DBG("Creating a squad");
	CAIVector const& spawnPosition = spawnZone->midPos();
	sint32 baseLevel = -1;
	double dispersionRadius = spawnZone->getRadius();
	// Save the creator state
	bool const savePlayerAttackable = groupDesc->getGDPlayerAttackable();
	bool const saveBotAttackable = groupDesc->getGDBotAttackable();
	// PlayerAttackable must be false as it it sent to the EGS in BotDescriptionMessage to set the
	// property 'Attackable'. Squads are not attackable unless a player is in war with the owner of
	// the spire (using botchatprogram instead of the property 'Attackable').
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
		SPIRE_WRN("createSquad failed: group %s cannot spawn", LigoConfig.aliasToString(groupDesc->getAlias()).c_str());
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
			CAIState* state = stateMachine->cstStates()[i];
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
		// A tribe owns the spire and all players are ennemies
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
		bot->setOwnerSpire(this);
	}*/
	
	grp->setSpireSide(side);
	// Attack only the declared ennemies of the spire
	if (side==SPIREENUMS::SpireOwner)
	{
	//	grp->faction      ().addProperty(NLMISC::toString("spire:%s:defender", getAliasString().c_str()));
	//	grp->friendFaction().addProperty(NLMISC::toString("spire:%s:defender", getAliasString().c_str()));
		grp->ennemyFaction().addProperty(NLMISC::toString("spire:%s:attacker", getAliasString().c_str()));
	}
	if (side==SPIREENUMS::SpireAttacker)
	{
	//	grp->faction      ().addProperty(NLMISC::toString("spire:%s:attacker", getAliasString().c_str()));
	//	grp->friendFaction().addProperty(NLMISC::toString("spire:%s:attacker", getAliasString().c_str()));
		grp->ennemyFaction().addProperty(NLMISC::toString("spire:%s:defender", getAliasString().c_str()));
	}
	grp->_AggroRange = 25;
	grp->_UpdateNbTicks = 10;
	grp->respawnTime() = respawTimeGC;
	
	grp->updateStateInstance();	// Directly call his first state (to retrieve associated params).
	
	if (createOrder!=0)
	{
		CSpireSquadCreatedMsg params;
		params.Spire = this->getAlias();
		params.CreateOrder = createOrder;
		params.GroupId = reinterpret_cast<uint32>(grp);
		sendSpireMessage("SPIRE_SQUAD_CREATED", params);
	}

	SPIRE_DBG( "Spire %s: squad created defending 0x%x against 0x%x, respawnTime=%u gc", getName().c_str(), _OwnerAllianceId, _AttackerAllianceId, respawTimeGC );
}

void CSpire::spawnSquad(uint32 groupId)
{
	SPIRE_DBG( "Spire %s: Spawning squad 0x%08x", getName().c_str(), groupId );
	FOREACH(itManager, CAliasCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			if (groupId==thisGroupId)
			{
				group->getSpawnObj()->spawnBots();
				CSpireSquadSpawnedMsg params;
				params.Spire = this->getAlias();
				params.GroupId = groupId;
				sendSpireMessage("SPIRE_SQUAD_SPAWNED", params);
				SPIRE_DBG( "\t\tSpawned" );
				return;
			}
		}
	}
	SPIRE_WRN("No squad to spawn with group id 0x%08x. Valid group ids are:", groupId);
	FOREACH(itManager, CAliasCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			SPIRE_WRN("- 0x%08x", thisGroupId);
		}
	}
}

void CSpire::despawnSquad(uint32 groupId)
{
	SPIRE_DBG( "Spire %s: Despawning squad 0x%08x", getName().c_str(), groupId );
	FOREACH(itManager, CAliasCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			if (groupId==thisGroupId)
			{
				group->despawnBots();
				CSpireSquadDespawnedMsg params;
				params.Spire = this->getAlias();
				params.GroupId = groupId;
				sendSpireMessage("SPIRE_SQUAD_DESPAWNED", params);
				return;
			}
		}
	}
	SPIRE_WRN("No squad to despawn");
}

void CSpire::deleteSquad(uint32 groupId)
{
	SPIRE_DBG( "Spire %s: deleting squad 0x%08x", getName().c_str(), groupId );
	FOREACH(itManager, CAliasCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		FOREACH(itGroup, CAliasCont<CGroup>, manager->groups())
		{
			CGroup* group = *itGroup;
			CGroupNpc* groupNpc = static_cast<CGroupNpc*>(group);
			uint32 thisGroupId = reinterpret_cast<uint32>(groupNpc);
			if (groupId==thisGroupId)
			{
				manager->groups().removeChildByIndex(group->getChildIndex());
				// No message here since this is called by squad destructor in EGS
				return;
			}
		}
	}
	SPIRE_WRN("No squad to delete");
}

void CSpire::sendSpireSquadStatus(CGroupNpc* group)
{
	uint32 alias = this->getAlias();
	uint32 groupId = reinterpret_cast<uint32>(group);
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
	NLNET::CMessage msgout("SPIRE_SQUAD_STATUS");
	msgout.serial(alias);
	msgout.serial(groupId);
	msgout.serial(groupAlive);
	msgout.serial(leaderAlive);
	msgout.serial(botCount);
	sendMessageViaMirror("EGS", msgout);
}

void CSpire::squadLeaderDied(CGroupNpc* group)
{
	uint32 alias = this->getAlias();
	uint32 groupId = reinterpret_cast<uint32>(group);
	NLNET::CMessage msgout("SPIRE_SQUAD_LEADER_DIED");
	msgout.serial(alias);
	msgout.serial(groupId);
	sendMessageViaMirror("EGS", msgout);
}

void CSpire::squadDied(CGroupNpc* group)
{
	uint32 alias = this->getAlias();
	uint32 groupId = reinterpret_cast<uint32>(group);
	NLNET::CMessage msgout("SPIRE_SQUAD_DIED");
	msgout.serial(alias);
	msgout.serial(groupId);
	sendMessageViaMirror("EGS", msgout);
}

void CSpire::triggerSpecialEvent(SPIREENUMS::TSpecialSpireEvent eventId)
{
	// Managers handlers
	FOREACH(itManager, CCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		if (manager)
		{
			CAIEvent* event = NULL;
			switch (eventId)
			{
			case SPIREENUMS::PeaceStateBegin: event = &manager->EventSpirePeaceStateBegin; break;
			case SPIREENUMS::PeaceStateEnd: event = &manager->EventSpirePeaceStateEnd; break;
			case SPIREENUMS::TribeOwnershipBegin: event = &manager->EventSpireTribeOwnershipBegin; break;
			case SPIREENUMS::TribeOwnershipEnd: event = &manager->EventSpireTribeOwnershipEnd; break;
			case SPIREENUMS::GuildOwnershipBegin: event = &manager->EventSpireGuildOwnershipBegin; break;
			case SPIREENUMS::GuildOwnershipEnd: event = &manager->EventSpireGuildOwnershipEnd; break;
			case SPIREENUMS::OwnerChanged: event = &manager->EventSpireOwnerChanged; break;
			case SPIREENUMS::AttackerChanged: event = &manager->EventSpireAttackerChanged; break;
			case SPIREENUMS::StateChanged: event = &manager->EventSpireStateChanged; break;
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
	case SPIREENUMS::TribeOwnershipBegin:
	{
		FOREACH(itManager, CCont<CSpireManager>, _Managers)
		{
			CSpireManager* manager = *itManager;
			if (manager)
				manager->autoSpawnBegin();
		}
		break;
	}
	case SPIREENUMS::TribeOwnershipEnd:
	{
		FOREACH(itManager, CCont<CSpireManager>, _Managers)
		{
			CSpireManager* manager = *itManager;
			if (manager)
				manager->autoSpawnEnd();
		}
		break;
	}
	}
}

void CSpire::setBuildingBotSheet(uint32 buildingAlias, NLMISC::CSheetId sheetId, bool autoSpawnDespawn, const std::string & customName)
{
	CBot * bot = getBuildingBotByAlias(buildingAlias);
	if (bot == NULL)
	{
		SPIRE_WRN( "cannot find building bot %s", LigoConfig.aliasToString( buildingAlias ).c_str() );
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

void CSpire::despawnAllSquads()
{
	FOREACH(itManager, CAliasCont<CSpireManager>, _Managers)
	{
		CSpireManager* manager = *itManager;
		CSpireSquadManager* sqManager = dynamic_cast<CSpireSquadManager*>(manager);
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
void CSpire::sendSpireMessage(std::string const& msgName, T& paramStruct)
{
	NLNET::CMessage msgout(msgName);
	msgout.serial(paramStruct);
	sendMessageViaMirror(std::string("EGS"), msgout);
}

//////////////////////////////////////////////////////////////////////////////
// Commands                                                                 //
//////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND(displaySpires, "list the available spire", "")
{
	if (args.size() > 0)
		return false;
	
	uint32 instanceNumber = ~0;
	for (uint i=0; i<CAIS::instance().AIList().size(); ++i)
	{
		CAIInstance	*const	aii = CAIS::instance().AIList()[i];
		if (!aii)
		{
			log.displayNL("No current ai instance, can't look for spire");
			return true;
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
			
			log.displayNL("Spires in continent '%s':", continent->getName().c_str());
			for	(uint j=0; j<continent->spires().size(); ++j)
			{
				CSpire const* spire = continent->spires()[j];
				if (!spire)
					continue;
				
				log.displayNL("  %s", spire->getOneLineInfoString().c_str());
				log.displayNL("    - %d group descs", spire->squadLinks().size());
				for	(CSpire::CSquadLinks::const_iterator isl=spire->squadLinks().begin(); isl!=spire->squadLinks().end(); ++isl )
				{
					CGroupDesc<CSpireSquadFamily> const* gd = (*isl).second;
					if (!gd)
						continue;
					
					log.displayNL("      Group desc '%s' %s", gd->getName().c_str(), gd->getAliasString().c_str());
				}

				log.displayNL("  - %d spawn zones", spire->spawnZones().size());
				for	(uint j=0; j<spire->spawnZones().size(); ++j)
				{
					CSpireSpawnZone const* sz = spire->spawnZones()[j];
					if (!sz)
						continue;
					
					log.displayNL("    SpawnZone '%s' %s (%s)", sz->getName().c_str(), sz->getAliasString().c_str(), sz->midPos().toString().c_str());
				}
				log.displayNL("  - %d state machines", spire->managers().size());
				for	(uint j=0; j<spire->managers().size(); ++j)
				{
					CSpireManager* manager = spire->managers()[j];
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

// spireSpawnSquad 1 fyros spire group machine zone

NLMISC_COMMAND(spireSpawnSquad, "Spawns a squad in an spire", "<instance_number> <continent> <spire> <group_template> <state_machine> <spawn_zone> [<respawnTimeInSec>=5*60]")
{
	if (args.size() != 6)
		return false;
	
	uint32 instanceNumber = ~0;
	fromString(args[0], instanceNumber);
	string continentName = args[1];
	string spireName = args[2];
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
	bool spireFound = false;
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
				for	(uint j=0; j<continent->spires().size() && !done; ++j)
				{
					CSpire* const spire = continent->spires()[j];
					if (!spire || spireName!=spire->getName())
						continue;
					spireFound = true;
					spire->createSquad(dynGroupName, stateMachineName, "", zoneName, 0, respawnTimeGC, SPIREENUMS::UnknownPVPSide);
					done = true;
				}
			}
		}
	}
	if (!done)
	{
		SPIRE_WRN("Could not spawn a new squad");
		log.displayNL("Could not spawn a new squad");
		if (!instanceFound)
			log.displayNL( "Instance not found" );
		if (!continentFound)
			log.displayNL( "Continent not found" );
		if (!spireFound)
			log.displayNL( "Spire not found" );
	}
	return true;
}


/*
 * Return the spire corresponding to the alias, or NULL if not found (with a nlwarning)
 */
CSpire* CSpire::getSpireByAlias( TAIAlias spireAlias )
{
	bool instanceFound = false;
	bool continentFound = false;
	bool spireFound = false;
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
				for	(uint j=0; j<continent->spires().size(); ++j)
				{
					CSpire* spire = continent->spires()[j];
					if (!spire || spireAlias!=spire->getAlias())
						continue;
					return spire;
				}
			}
		}
	}
	if (!instanceFound)
		SPIRE_WRN( "Instance not found" );
	if (!continentFound)
		SPIRE_WRN( "Continent not found" );
	if (!spireFound)
		SPIRE_WRN( "Spire %s not found", LigoConfig.aliasToString( spireAlias ).c_str() );
	return NULL;
}

/*
void spireCreateSquad(uint32 spireAlias, uint32 dynGroupAlias, uint32 zoneAlias, uint32 spawnOrder)
{
	CSpire *spire = CSpire::getSpireByAlias( spireAlias );
	if ( spire )
		spire->createSquad(dynGroupAlias, zoneAlias, spawnOrder);
}

void spireSpawnSquad(uint32 spireAlias, uint32 groupId)
{
	CSpire *spire = CSpire::getSpireByAlias( spireAlias );
	if ( spire )
		spire->spawnSquad(groupId);
}

void spireDespawnSquad(uint32 spireAlias, uint32 groupId)
{
	CSpire *spire = CSpire::getSpireByAlias( spireAlias );
	if ( spire )
		spire->despawnSquad(groupId);
}

void spireDeleteSquad(uint32 spireAlias, uint32 groupId)
{
	CSpire *spire = CSpire::getSpireByAlias( spireAlias );
	if ( spire )
		spire->deleteSquad(groupId);
}

void spireDespawnAllSquads(uint32 spireAlias)
{
	CSpire *spire = CSpire::getSpireByAlias( spireAlias );
	if ( spire )
		spire->despawnAllSquads();
}
*/

NLMISC_COMMAND(spireAliasSpawnSquad, "Spawns a squad in an spire (respawn time = 5 min)", "<spire_alias> <group_alias> <spawn_zone_alias>")
{
	if (args.size()!=3)
		return false;
	
	for (size_t i=0; i<3; ++i)
		if (args[i]=="")
			return false;
	
	uint32 spireAlias = LigoConfig.aliasFromString(args[0]);
	uint32 dynGroupAlias = LigoConfig.aliasFromString(args[1]);
	uint32 zoneAlias = LigoConfig.aliasFromString(args[2]);
	string stateMachineName = "default_squad_manager";
	
	CSpire *spire = CSpire::getSpireByAlias(spireAlias);
	if (spire)
		spire->createSquad(dynGroupAlias, zoneAlias, 0, 5*60*10, SPIREENUMS::UnknownPVPSide);
	
	return true;
}

CBot* CSpire::getBuildingBotByAlias(TAIAlias alias)
{
	CGroup* group = getBuildingGroup();
	if (group)
		return group->bots().getChildByAlias(alias);
	else
		return NULL;
}

NLMISC_COMMAND(spireSetBuildingBotSheet, "Set the sheet of an spire building", "<spire_alias> <building_alias> <bot_sheet>")
{
	if (args.size()!=3)
		return false;
	
	for (size_t i=0; i<3; ++i)
		if (args[i]=="")
			return false;
	
	uint32 spireAlias = LigoConfig.aliasFromString(args[0]);
	uint32 buildingAlias = LigoConfig.aliasFromString(args[1]);
	NLMISC::CSheetId buildingBotSheet = NLMISC::CSheetId(args[2]);;
	
	CSpire* spire = CSpire::getSpireByAlias(spireAlias);
	if (spire)
		spire->setBuildingBotSheet(buildingAlias, buildingBotSheet, true, "");
	
	return true;
}


// Call this macro, then do the code that will must done only if the spire is found in this instance
// The data you have access to after this macro:
// - CSpire *spire
// - msgStruct params;
#define IF_GET_SPIRE_FOR_MSG( msgStruct ) \
	msgStruct params; \
	msgin.serial( params ); \
	CSpire *spire = CSpire::getSpireByAlias( params.Spire ); \
if ( spire )


void cbSpireCreateSquad( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireCreateSquadMsg )
		spire->createSquad(params.Group, params.Zone, params.CreateOrder, params.RespawnTimeS*10, params.Side);
}

void cbSpireSpawnSquad( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireSpawnSquadMsg )
		spire->spawnSquad(params.GroupId);
}

void cbSpireDespawnSquad( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireDespawnSquadMsg )
		spire->despawnSquad(params.GroupId);
}

void cbSpireDeleteSquad( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireDeleteSquadMsg )
		spire->deleteSquad(params.GroupId);
}

void cbSpireDespawnAllSquads( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireDespawnAllSquadsMsg )
		spire->despawnAllSquads();
}

void cbSpireSetOwner( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSetSpireOwner )
		spire->setOwnerAlliance( params.Owner );
}

void cbSpireSetAttacker( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSetSpireAttacker )
		spire->setAttackerAlliance( params.Attacker );
}

void cbSpireSetState( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireSetStateMsg )
		spire->setState( params.State );
}

void cbSpireEvent( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireEventMsg )
		spire->triggerSpecialEvent(params.Event);
}

void cbSpireSetBuildingBotSheet( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	IF_GET_SPIRE_FOR_MSG( CSpireSetBuildingBotSheetMsg )
		spire->setBuildingBotSheet(params.Building, params.SheetId, params.AutoSpawnDespawn, params.CustomName);
}

#include "event_reaction_include.h"
#endif
