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
#include "npc_icon.h"
#include "ingame_database_manager.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "entities.h"
#include "net_manager.h"

using namespace std;
using namespace NLMISC;

CNPCIconCache* CNPCIconCache::_Instance = NULL;

// Time after which the state of a NPC is considered obsolete and must be refreshed (because it's in gamecycle, actual time increases if server slows down, to avoid server congestion)
NLMISC::TGameCycle CNPCIconCache::_CacheRefreshTimerDelay = NPC_ICON::DefaultClientNPCIconRefreshTimerDelayGC;

// Time between updates of the "catchall timer"
NLMISC::TGameCycle CNPCIconCache::_CatchallTimerPeriod = NPC_ICON::DefaultClientNPCIconRefreshTimerDelayGC;

extern CEntityManager EntitiesMngr;
extern CGenericXmlMsgHeaderManager GenericMsgHeaderMngr;
extern CNetManager NetMngr;


// #pragma optimize ("", off)


CNPCIconCache::CNPCIconCache() : _LastRequestTimestamp(0), _LastTimerUpdateTimestamp(0), _Enabled(true)
{
	_Icons[NPC_ICON::IconNone].init("", "");
	_Icons[NPC_ICON::IconNotAMissionGiver].init("", "");
	_Icons[NPC_ICON::IconListHasOutOfReachMissions].init("mission_available.tga", ""); //"MP_Blood.tga"
	_Icons[NPC_ICON::IconListHasAlreadyTakenMissions].init("ICO_Task_Generic.tga", "r2ed_tool_redo");
	_Icons[NPC_ICON::IconListHasAvailableMission].init("mission_available.tga", "", CViewRadar::MissionList); //"MP_Wood.tga"
	_Icons[NPC_ICON::IconAutoHasUnavailableMissions].init("spe_com.tga", "");
	_Icons[NPC_ICON::IconAutoHasAvailableMission].init("spe_com.tga", "", CViewRadar::MissionAuto); //"MP_Oil.tga"
	_Icons[NPC_ICON::IconStepMission].init("mission_step.tga", "", CViewRadar::MissionStep); //"MP_Shell.tga"

	_DescriptionsToRequest.reserve(256);
}

void CNPCIconCache::release()
{
	if (_Instance)
	{
		delete _Instance;
		_Instance = NULL;
	}
}

const CNPCIconCache::CNPCIconDesc& CNPCIconCache::getNPCIcon(const CEntityCL *entity, bool	bypassEnabled)
{
	// Not applicable? Most entities (creatures, characters) have a null key here.
	BOMB_IF(!entity, "NULL entity in getNPCIcon", return _Icons[NPC_ICON::IconNone]);
	TNPCIconCacheKey npcIconCacheKey = CNPCIconCache::entityToKey(entity);
	if (npcIconCacheKey == 0)
		return _Icons[NPC_ICON::IconNone];

	// Is system disabled?
	if ((!enabled()) && !bypassEnabled)
		return _Icons[NPC_ICON::IconNone];

	// This method must be reasonably fast, because it constantly gets called by the radar view
	H_AUTO(GetNPCIconWithKey);

	// Not applicable (more checks)?
	if (!entity->canHaveMissionIcon())
		return _Icons[NPC_ICON::IconNone];
	if (!entity->isFriend()) // to display icons in the radar, we need the Contextual property to be received as soon as possible
		return _Icons[NPC_ICON::IconNone];

	// Temporarily not shown if the player is in interaction with the NPC
	if (UserEntity->interlocutor() != CLFECOMMON::INVALID_SLOT)
	{
		CEntityCL *interlocutorEntity = EntitiesMngr.entity(UserEntity->interlocutor());
		if (interlocutorEntity && (entityToKey(interlocutorEntity) == npcIconCacheKey))
			return _Icons[NPC_ICON::IconNone];
	}
	if (UserEntity->trader() != CLFECOMMON::INVALID_SLOT)
	{
		CEntityCL *traderEntity = EntitiesMngr.entity(UserEntity->trader());
		if (traderEntity && (entityToKey(traderEntity) == npcIconCacheKey))
			return _Icons[NPC_ICON::IconNone];
	}

	// 1. Test if the NPC is involved in a current goal
	if (isNPCaCurrentGoal(npcIconCacheKey))
		return _Icons[NPC_ICON::IconStepMission];

	// 2. Compute "has mission to take": take from cache, or query the server
	H_AUTO(GetNPCIcon_GIVER);
	CMissionGiverMap::iterator img = _MissionGivers.find(npcIconCacheKey);
	if (img != _MissionGivers.end())
	{
		CNPCMissionGiverDesc& giver = (*img).second;
		if (giver.getState() != NPC_ICON::AwaitingFirstData)
		{
			// Ask the server to refresh the state if the information is old
			// but only known mission givers that have a chance to propose new missions
			if ((giver.getState() != NPC_ICON::NotAMissionGiver) &&
//				(giver.getState() != NPC_ICON::ListHasAlreadyTakenMissions) && // commented out because it would not refresh in case an auto mission become available
				(!giver.isDescTransient()))
			{
				NLMISC::TGameCycle informationAge = NetMngr.getCurrentServerTick() - giver.getLastUpdateTimestamp();
				if (informationAge > _CacheRefreshTimerDelay)
				{
					queryMissionGiverData(npcIconCacheKey);
					giver.setDescTransient();
				}
			}

			// Return the icon depending on the state in the cache
			return _Icons[giver.getState()]; // TNPCIconId maps TNPCMissionGiverState
		}
	}
	else
	{
		// Create mission giver entry and query the server
		CNPCMissionGiverDesc giver;
		CMissionGiverMap::iterator itg = _MissionGivers.insert(make_pair(npcIconCacheKey, giver)).first;
		queryMissionGiverData(npcIconCacheKey);
		//(*itg).second.setDescTransient(); // already made transient by constructor
	}

	return _Icons[NPC_ICON::IconNone];
}

#define getArraySize(a) (sizeof(a)/sizeof(a[0]))

void		CNPCIconCache::addObservers()
{
	// Disabled?
	if (!enabled())
		return;

	// Mission Journal
	static const char *missionStartStopLeavesToMonitor [2] = {"TITLE", "FINISHED"};
	IngameDbMngr.addBranchObserver( IngameDbMngr.getNodePtr(), "MISSIONS", MissionStartStopObserver, missionStartStopLeavesToMonitor, getArraySize(missionStartStopLeavesToMonitor));
	static const char *missionNpcAliasLeavesToMonitor [1] = {"NPC_ALIAS"};
	IngameDbMngr.addBranchObserver( IngameDbMngr.getNodePtr(), "MISSIONS", MissionNpcAliasObserver, missionNpcAliasLeavesToMonitor, getArraySize(missionNpcAliasLeavesToMonitor));

	// Skills
	static const char *skillLeavesToMonitor [2] = {"SKILL", "BaseSKILL"};
	IngameDbMngr.addBranchObserver( IngameDbMngr.getNodePtr(), "CHARACTER_INFO:SKILLS", MissionPrerequisitEventObserver, skillLeavesToMonitor, getArraySize(skillLeavesToMonitor));

	// Owned Items
	static const char *bagLeavesToMonitor [1] = {"SHEET"}; // just saves 2000 bytes or so (500 * observer pointer entry in vector) compared to one observer per bag slot
	IngameDbMngr.addBranchObserver( IngameDbMngr.getNodePtr(), "INVENTORY:BAG", MissionPrerequisitEventObserver, bagLeavesToMonitor, getArraySize(bagLeavesToMonitor));

	// Worn Items
	IngameDbMngr.addBranchObserver( "INVENTORY:HAND", &MissionPrerequisitEventObserver);
	IngameDbMngr.addBranchObserver( "INVENTORY:EQUIP", &MissionPrerequisitEventObserver);

	// Known Bricks
	IngameDbMngr.addBranchObserver( "BRICK_FAMILY", &MissionPrerequisitEventObserver);

	// For other events, search for calls of onEventForMissionAvailabilityForThisChar()
}

void		CNPCIconCache::removeObservers()
{
	// Disabled?
	if (!enabled())
		return;

	// Mission Journal
	IngameDbMngr.getNodePtr()->removeBranchObserver("MISSIONS", MissionStartStopObserver);
	IngameDbMngr.getNodePtr()->removeBranchObserver("MISSIONS", MissionNpcAliasObserver);

	// Skills
	IngameDbMngr.getNodePtr()->removeBranchObserver("CHARACTER_INFO:SKILLS", MissionPrerequisitEventObserver);

	// Owned Items
	IngameDbMngr.getNodePtr()->removeBranchObserver("INVENTORY:BAG", MissionPrerequisitEventObserver);

	// Worn Items
	IngameDbMngr.getNodePtr()->removeBranchObserver("INVENTORY:HAND", MissionPrerequisitEventObserver);
	IngameDbMngr.getNodePtr()->removeBranchObserver("INVENTORY:EQUIP", MissionPrerequisitEventObserver);

	// Known Bricks
	IngameDbMngr.getNodePtr()->removeBranchObserver("BRICK_FAMILY", MissionPrerequisitEventObserver);
}

void		CNPCIconCache::CMissionStartStopObserver::update(ICDBNode* node)
{
	// Every time a mission in progress is started or stopped, refresh the icon for visible NPCs (including mission giver information)
	CNPCIconCache::getInstance().onEventForMissionInProgress();
}

void		CNPCIconCache::CMissionNpcAliasObserver::update(ICDBNode* node)
{
	CNPCIconCache::getInstance().onNpcAliasChangedInMissionGoals();
}

void		CNPCIconCache::CMissionPrerequisitEventObserver::update(ICDBNode* node)
{
	// Every time a mission in progress changes, refresh the icon for the related npc
	CNPCIconCache::getInstance().onEventForMissionAvailabilityForThisChar();
}

void	CNPCIconCache::onEventForMissionAvailabilityForThisChar()
{
	// Disabled?
	if (!enabled())
		return;

	queryAllVisibleMissionGiverData(0);
}

void		CNPCIconCache::queryMissionGiverData(TNPCIconCacheKey npcIconCacheKey)
{
	_DescriptionsToRequest.push_back(npcIconCacheKey);
	//static set<TNPCIconCacheKey> requests1;
	//requests1.insert(npcIconCacheKey);
	//nldebug("%u: queryMissionGiverData           %u (total %u)", NetMngr.getCurrentServerTick(), npcIconCacheKey, requests1.size());

}

void		CNPCIconCache::queryAllVisibleMissionGiverData(NLMISC::TGameCycle olderThan)
{
	// Request an update for all npcs (qualifying, i.e. that have missions) in vision
	for (uint i=0; i<EntitiesMngr.entities().size(); ++i)
	{
		CEntityCL *entity = EntitiesMngr.entity(i);
		if (!entity || !(entity->canHaveMissionIcon() && entity->isFriend()))
			continue;
		TNPCIconCacheKey npcIconCacheKey = CNPCIconCache::entityToKey(entity);
		CMissionGiverMap::iterator img = _MissionGivers.find(npcIconCacheKey);
		if (img == _MissionGivers.end())
			continue; // if the NPC does not have an entry yet, it will be created by getNPCIcon()

		// Refresh only known mission givers that have a chance to propose new missions
		CNPCMissionGiverDesc& giver = (*img).second;
		if (giver.getState() == NPC_ICON::NotAMissionGiver)
			continue;
//		if (giver.getState() == NPC_ICON::ListHasAlreadyTakenMissions)
//			continue; // commented out because it would not refresh in case an auto mission becomes available

		if (olderThan != 0)
		{
			// Don't refresh desscriptions already awaiting an update
			if (giver.isDescTransient())
				continue;

			// Don't refresh NPCs having data more recent than specified
			NLMISC::TGameCycle informationAge = NetMngr.getCurrentServerTick() - giver.getLastUpdateTimestamp();
			if (informationAge <= olderThan)
				continue;

			// Don't refresh NPC being involved in a mission goal (the step icon has higher priority over the giver icon)
			// If later the NPC is no more involved before the information is considered old, it will show
			// the same giver state until the information is considered old. That's why we let refresh
			// the NPC when triggered by an event (olderThan == 0).
			if (isNPCaCurrentGoal(npcIconCacheKey))
				continue;
		}

		_DescriptionsToRequest.push_back(npcIconCacheKey);	
		giver.setDescTransient();

		//static set<TNPCIconCacheKey> requests2;
		//requests2.insert(npcIconCacheKey);
		//nldebug("%u: queryAllVisibleMissionGiverData %u (total %u)", NetMngr.getCurrentServerTick(), npcIconCacheKey, requests2.size());
	}
}

void		CNPCIconCache::update()
{
	// Every CatchallTimerPeriod, browse visible entities and refresh the ones with outdated state
	// (e.g. the ones not displayed in radar).
	if (NetMngr.getCurrentServerTick() > _LastTimerUpdateTimestamp + _CatchallTimerPeriod)
	{
		_LastTimerUpdateTimestamp = NetMngr.getCurrentServerTick();

		// Disabled?
		if (!enabled())
			return;

		queryAllVisibleMissionGiverData(_CacheRefreshTimerDelay);
	}

	// Every tick update at most (2 cycles actually, cf. server<->client communication frequency),
	// send all pending requests in a single message.
	if (NetMngr.getCurrentServerTick() > _LastRequestTimestamp)
	{
		if (!_DescriptionsToRequest.empty())
		{
			CBitMemStream out;
			GenericMsgHeaderMngr.pushNameToStream("NPC_ICON:GET_DESC", out);
			uint8 nb8 = uint8(_DescriptionsToRequest.size() & 0xff); // up to vision size (255 i.e. 256 minus user)
			out.serial(nb8);
			for (CSmallKeyList::const_iterator ikl=_DescriptionsToRequest.begin(); ikl!=_DescriptionsToRequest.end(); ++ikl)
			{
				TNPCIconCacheKey key = *ikl;
				out.serial(key);
			}
			NetMngr.push(out);
			//nldebug("%u: Pushing %hu NPC desc requests", NetMngr.getCurrentServerTick(), nb8);
			_DescriptionsToRequest.clear();
		}
		_LastRequestTimestamp = NetMngr.getCurrentServerTick();
	}
}

void		CNPCIconCache::onEventForMissionInProgress()
{
	// Disabled?
	if (!enabled())
		return;

	// Immediately reflect the mission journal (Step icons)
	refreshIconsOfScene(true);

	// Ask the server to update availability status (will refresh icons if there is at least one change)
	onEventForMissionAvailabilityForThisChar();
}

void		CNPCIconCache::onNpcAliasChangedInMissionGoals()
{
	// Disabled?
	if (!enabled())
		return;

	// Update the storage of keys having a current mission goal.
	storeKeysOfCurrentGoals();

	// Immediately reflect the mission journal (Step icons)
	refreshIconsOfScene(true);
}

bool		CNPCIconCache::isNPCaCurrentGoal(TNPCIconCacheKey npcIconCacheKey) const
{
	// There aren't many simultaneous goals, we can safely browse the vector
	for (CSmallKeyList::const_iterator ikl=_KeysOfCurrentGoals.begin(); ikl!=_KeysOfCurrentGoals.end(); ++ikl)
	{
		if ((*ikl) == npcIconCacheKey)
			return true;
	}
	return false;
}

void		CNPCIconCache::storeKeysOfCurrentGoals()
{
	// This event is very unfrequent, and the number of elements of _KeysOfCurrentGoals is usually very small
	// (typically 0 to 3, while theoretical max is 15*20) so we don't mind rebuilding the list.
	_KeysOfCurrentGoals.clear();
	CCDBNodeBranch *missionNode = safe_cast<CCDBNodeBranch*>(IngameDbMngr.getNodePtr()->getNode(ICDBNode::CTextId("MISSIONS")));
	BOMB_IF (!missionNode, "MISSIONS node missing in DB", return);
	uint nbCurrentMissionSlots = missionNode->getNbNodes();
	for (uint i=0; i!=nbCurrentMissionSlots; ++i)
	{
		ICDBNode *missionEntry = missionNode->getNode((uint16)i);
		ICDBNode::CTextId titleNode("TITLE");
		if (missionEntry->getProp(titleNode) == 0)
			continue;

		CCDBNodeBranch *stepsToDoNode = safe_cast<CCDBNodeBranch*>(missionEntry->getNode(ICDBNode::CTextId("GOALS")));
		BOMB_IF(!stepsToDoNode, "GOALS node missing in MISSIONS DB", return);
		uint nbGoals = stepsToDoNode->getNbNodes();
		for (uint j=0; j!=nbGoals; ++j)
		{
			ICDBNode *stepNode = stepsToDoNode->getNode((uint16)j);
			CCDBNodeLeaf *aliasNode = safe_cast<CCDBNodeLeaf*>(stepNode->getNode(ICDBNode::CTextId("NPC_ALIAS")));
			BOMB_IF(!aliasNode, "NPC_ALIAS node missing in MISSIONS DB", return);
			TNPCIconCacheKey npcIconCacheKey = (TNPCIconCacheKey)aliasNode->getValue32();
			if (npcIconCacheKey != 0)
				_KeysOfCurrentGoals.push_back(npcIconCacheKey);
		}
	}	
}

void		CNPCIconCache::refreshIconsOfScene(bool force)
{
	// Browse all NPCs in vision, and refresh their inscene interface
	for (uint i=0; i<EntitiesMngr.entities().size(); ++i)
	{
		CEntityCL *entity = EntitiesMngr.entity(i);
		if (!entity) continue;

		CMissionGiverMap::iterator it = _MissionGivers.find(CNPCIconCache::entityToKey(entity));
		if ((it!=_MissionGivers.end()) && ((*it).second.hasChanged() || force))
		{
			EntitiesMngr.refreshInsceneInterfaceOfFriendNPC(i);
			(*it).second.setChanged(false);
		}
	}
}

bool		CNPCIconCache::onReceiveMissionAvailabilityForThisChar(TNPCIconCacheKey npcIconCacheKey, NPC_ICON::TNPCMissionGiverState state)
{
	CMissionGiverMap::iterator img = _MissionGivers.find(npcIconCacheKey);
	BOMB_IF(img == _MissionGivers.end(), "Mission Giver " << npcIconCacheKey << "not found", return false);

	//if (state != NPC_ICON::NotAMissionGiver)
	//{
	//	static set<TNPCIconCacheKey> qualifs;
	//	qualifs.insert(npcIconCacheKey);
	//	nldebug("NPC %u qualifies (total=%u)", npcIconCacheKey, qualifs.size());
	//}

	return (*img).second.updateMissionAvailabilityForThisChar(state);
}

bool	CNPCMissionGiverDesc::updateMissionAvailabilityForThisChar(NPC_ICON::TNPCMissionGiverState state)
{
	_HasChanged = (state != _MissionGiverState);
	_MissionGiverState = state;
	_LastUpdateTimestamp = NetMngr.getCurrentServerTick();
	_IsDescTransient = false;
	return _HasChanged;
}

void	CNPCIconCache::setMissionGiverTimer(NLMISC::TGameCycle delay)
{
	_CacheRefreshTimerDelay = delay;
	_CatchallTimerPeriod = delay;
}

std::string CNPCIconCache::getDump() const
{
	string s = toString("System %s\nCurrent timers: %u %u\n", _Enabled?"enabled":"disabled", _CacheRefreshTimerDelay, _CatchallTimerPeriod);
	s += toString("%u NPCs in mission giver map:\n", _MissionGivers.size());
	for (CMissionGiverMap::const_iterator img=_MissionGivers.begin(); img!=_MissionGivers.end(); ++img)
	{
		const CNPCMissionGiverDesc& giver = (*img).second;
		s += toString("NPC %u: ", (*img).first) + giver.getDump() + "\n";
	}
	s += "Current NPC goals:\n";
	for (CSmallKeyList::const_iterator ikl=_KeysOfCurrentGoals.begin(); ikl!=_KeysOfCurrentGoals.end(); ++ikl)
	{
		s += toString("NPC %u", (*ikl));
	}
	return s;
}

std::string CNPCMissionGiverDesc::getDump() const
{
	return toString("%u [%u s ago]", _MissionGiverState, (NetMngr.getCurrentServerTick()-_LastUpdateTimestamp)/10);
}

void CNPCIconCache::setEnabled(bool b)
{
	if (!_Enabled && b)
	{
		_Enabled = b;
		addObservers(); // with _Enabled true
		storeKeysOfCurrentGoals(); // import from the DB
		refreshIconsOfScene(true);
	}
	else if (_Enabled && !b)
	{
		removeObservers(); // with _Enabled true
		_Enabled = b;
		refreshIconsOfScene(true);
	}
}

#ifndef FINAL_VERSION
#error FINAL_VERSION should be defined (0 or 1)
#endif

#if !FINAL_VERSION

NLMISC_COMMAND(dumpNPCIconCache, "Display descriptions of NPCs", "")
{
	log.displayNL(CNPCIconCache::getInstance().getDump().c_str());
	return true;
}

NLMISC_COMMAND(queryMissionGiverData, "Query mission giver data for the specified alias", "<alias>")
{
	if (args.size() == 0)
		return false;
	uint32 alias;
	NLMISC::fromString(args[0], alias);

	CNPCIconCache::getInstance().queryMissionGiverData(alias);
	//giver.setDescTransient();
	return true;
}

#endif

//#pragma optimize ("", on)
