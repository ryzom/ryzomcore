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
#include "server_share/r2_vision.h"
#include "ai_instance.h"

#include "ai_player.h"
#include "ai_grp_npc.h"
#include "ai_bot_fauna.h"
#include "ai_grp_fauna.h"
#include "ai_bot_easter_egg.h"

#include "ai_instance_inline.h"

#include "commands.h"
#include "messages.h"

using namespace std;
using namespace NLMISC;
using namespace MULTI_LINE_FORMATER;

CAIInstance::CAIInstance(CAIS* owner)
: CChild<CAIS>(owner)
{
	_PlayerManager = new CManagerPlayer(this);
	_PetManager = static_cast<CMgrPet*>(newMgr(AITYPES::MgrTypePet, 0, "Pet Manager", "MapName: Pet Manager", ""));
	_EventNpcManager = static_cast<CMgrNpc*>(newMgr(AITYPES::MgrTypeNpc, 0, "Event NPCs Manager", "MapName: Event NPCs Manager", ""));
	_EasterEggManager = static_cast<CMgrNpc*>(newMgr(AITYPES::MgrTypeNpc, 0, "Easter Eggs Manager", "MapName: Easter Eggs Manager", ""));
	_SquadFamily = new COutpostSquadFamily(this, 0, "Squads");
}

CAIInstance::~CAIInstance()
{
	// delete managers
	if (_PlayerManager != NULL)
	{
		delete _PlayerManager;
		_PlayerManager = NULL;
	}
	_PetManager = NULL;
	_EventNpcManager = NULL;
	_Managers.clear();

	// check if ref pointers were notified of the destruction
	nlassert(_EasterEggGroup == NULL);
	nlassert(_EasterEggManager == NULL);

	_SquadFamily = NULL;
	_SquadVariantNameToGroupDesc.clear();

	// delete continents
	_Continents.clear();
}

static bool zoneHaveError = false;

/// map of lastCreatedNpcGroup by player id
static	std::map<CEntityId, string> _PlayersLastCreatedNpcGroup;

void CAIInstance::addZone(string const& zoneName, CNpcZone* zone)
{
#if !FINAL_VERSION
	TZoneList::iterator	it = zoneList.find(CStringMapper::map(zoneName));
	if (it!=zoneList.end())
	{
		nlwarning("this NpcZone have the same name than another: %s", zoneName.c_str());
		zoneHaveError = true;
	}
#endif
	zoneList[CStringMapper::map(zoneName)] = zone;
}

void CAIInstance::removeZone(string const& zoneName, CNpcZone* zone)
{
	TZoneList::iterator	it = zoneList.find(CStringMapper::map(zoneName));
	zoneList.erase(it);
}

void CAIInstance::updateZoneTrigger(CBotPlayer* player)
{
	std::list<CMgrNpc*> zoneTriggerManager;

	FOREACH(it, CCont<CManager>, _Managers)
	{
		std::string name = it->getName();
		uint32 size = (uint32)name.size();
		const uint32 extensionSize   = 13; // strlen(".zone_trigger");
		if (size >= 13 && name.substr(size - extensionSize, extensionSize) == ".zone_trigger" )
		{
			CMgrNpc* npcManager = dynamic_cast<CMgrNpc*>(*it);
			if (npcManager && npcManager->getStateMachine())
			{
				zoneTriggerManager.push_back(npcManager);
			}
		}
	}
	if ( zoneTriggerManager.empty() )
	{
		return;
	}

	uint64 whoSeesMe = CMirrors::whoSeesMe(player->dataSetRow());
	if (!R2_VISION::isEntityVisibleToPlayers(whoSeesMe))
	{
		// The player is invisible, its a Dm / Gm: no zone event must be triggered
		return;
	}

	std::set<uint32> insideZone;
	std::vector<uint32> enterZone;
	std::vector<uint32> leaveZone;

	FOREACH(it, std::list<CMgrNpc*>, zoneTriggerManager)
	{
		CMgrNpc* npcManager = *it;
		CAliasCont<CGroup>& groups = npcManager->groups();

		CAliasCont<CGroup>::iterator first2(groups.begin()), last2(groups.end());
		for ( ; first2 != last2 ; ++first2)
		{
			CAIState*  startState = first2->getPersistentStateInstance()->getStartState();
			if (startState->isPositional())
			{
				const CAIStatePositional* posit = dynamic_cast<const CAIStatePositional*>(startState);
				if (posit)
				{
					CAIVector pos( player->pos() );
					bool inside = posit->contains(pos);
					if (inside)
					{
						insideZone.insert(first2->getAlias());
					}
				}
			}
		}
	}

	player->updateInsideTriggerZones(insideZone, enterZone, leaveZone);


	static NLMISC::TStringId  nbPlayer = CStringMapper::map("NbPlayer");

	FOREACH(zoneIt, std::vector<uint32>, enterZone)
	{
		FOREACH(it, std::list<CMgrNpc*>, zoneTriggerManager)
		{
			CGroup* grp = (*it)->groups().getChildByAlias(*zoneIt);
			if (grp)
			{

				CGroupNpc* grpNpc = dynamic_cast<CGroupNpc*>(grp);
				CPersistentStateInstance* sa = grpNpc->getPersistentStateInstance();
				if (sa)
				{
					sa->setLogicVar(nbPlayer, sa->getLogicVar(nbPlayer) + 1);
				}
				grpNpc->processStateEvent(grpNpc->getEventContainer().EventPlayerEnterTriggerZone);
			}
		}
	}

	FOREACH(zoneIt, std::vector<uint32>, leaveZone)
	{
		FOREACH(it, std::list<CMgrNpc*>, zoneTriggerManager)
		{
			CGroup* grp = (*it)->groups().getChildByAlias(*zoneIt);
			if (grp)
			{
				CGroupNpc* grpNpc = dynamic_cast<CGroupNpc*>(grp);
				CPersistentStateInstance* sa = grpNpc->getPersistentStateInstance();
				if (sa)
				{
					sa->setLogicVar(nbPlayer, sa->getLogicVar(nbPlayer) - 1);
				}
				grpNpc->processStateEvent(grpNpc->getEventContainer().EventPlayerLeaveTriggerZone);
			}
		}
	}

}

CNpcZone* CAIInstance::getZone(TStringId zoneName)
{
	return zoneList[zoneName];
}

std::string CAIInstance::getManagerIndexString(CManager const* manager) const
{
	return getIndexString()+NLMISC::toString(":m%u", manager->getChildIndex());
}

void CAIInstance::initInstance(string const& continentName, uint32 instanceNumber)
{
	_ContinentName = continentName;
	_InstanceNumber = instanceNumber;

	sendInstanceInfoToEGS();

	if	(!EGSHasMirrorReady)
		return;

	// check all player in mirror to insert them in this instance.
	TEntityIdToEntityIndexMap::const_iterator it, itEnd(TheDataset.entityEnd());
	for	(it = TheDataset.entityBegin(); it!=itEnd; ++it)
	{
		if (it->first.getType()!=RYZOMID::player)
			continue;

		TDataSetRow const row = TheDataset.getCurrentDataSetRow(it->second);
		if (!row.isValid())
			continue;

		// this is a player, check the instance number
		CMirrorPropValueRO<uint32> playerInstance(TheDataset, row, DSPropertyAI_INSTANCE);
		if (playerInstance != instanceNumber)
			continue;

		// ok, add this player
		getPlayerMgr()->addSpawnedPlayer(row, it->first);
	}
}

void CAIInstance::serviceEvent(CServiceEvent const& info)
{
	if (info.getServiceName() == "EGS")
		sendInstanceInfoToEGS();

	FOREACH(it, CCont<CManager>, managers())
		it->serviceEvent(info);

	if (CMgrPet* petManager = getPetMgr())
		petManager->serviceEvent(info);

	if (_EventNpcManager)
		_EventNpcManager->serviceEvent(info);

	FOREACH(first, CCont<CContinent>, continents())
		(*first)->serviceEvent(info);
}

void CAIInstance::sendInstanceInfoToEGS()
{
	// send message to the EGS about this new instance
	if (EGSHasMirrorReady)
	{
		CReportStaticAIInstanceMsg msg;
		msg.InstanceNumber = _InstanceNumber;
		msg.InstanceContinent = _ContinentName;

		msg.send("EGS");
	}
}

bool CAIInstance::advanceUserTimer(uint32 nbTicks)
{
	// for each manager, look for a timer event
	FOREACH(it, CCont<CManager>, _Managers)
	{
		CGroup* grp = (*it)->getNextValidGroupChild();
		while (grp)
		{
			grp->getPersistentStateInstance()->advanceUserTimer(nbTicks);
			grp=(*it)->getNextValidGroupChild(grp);	// next group
		}
	}
	return true;
}

void CAIInstance::addGroupInfo(CGroup* grp)
{
	string const& name = grp->aliasTreeOwner()->getName();
	uint32 alias = grp->aliasTreeOwner()->getAlias();

	if (!name.empty())
		_GroupFromNames[name].push_back(grp);
	if (alias)
		_GroupFromAlias[alias] = grp;
}

void CAIInstance::removeGroupInfo(CGroup* grp, CAliasTreeOwner* grpAliasTreeOwner)
{
	string const& name = grpAliasTreeOwner->getName();
	uint32 alias = grpAliasTreeOwner->getAlias();

	if (!name.empty())
	{
		std::map< std::string, std::vector<NLMISC::CDbgPtr<CGroup> > >::iterator it=_GroupFromNames.find(name);

		// remove from name
		if (it!=_GroupFromNames.end())
		{
			std::vector<NLMISC::CDbgPtr<CGroup> > &v = it->second;	//	_GroupFromNames[name];
			std::vector<NLMISC::CDbgPtr<CGroup> >::iterator itGrp(find(v.begin(), v.end(), NLMISC::CDbgPtr<CGroup>(grp) ));
			if (itGrp != v.end())
				v.erase(itGrp);
		}
	}
	// remove from alias
	if (alias)
		_GroupFromAlias.erase(alias);
}

CGroup* CAIInstance::findGroup(uint32 alias)
{
	std::map<uint32, NLMISC::CDbgPtr<CGroup> >::iterator it(_GroupFromAlias.find(alias));
	if (it != _GroupFromAlias.end())
		return it->second;
	return NULL;
}

void CAIInstance::findGroup(std::vector<CGroup*>& result, std::string const& name)
{
	std::map<std::string, std::vector<NLMISC::CDbgPtr<CGroup> > >::iterator it(_GroupFromNames.find(name));
	if (it != _GroupFromNames.end())
		result.insert(result.end(), it->second.begin(), it->second.end());
}

void CAIInstance::addMissionInfo(std::string const& missionName, uint32 alias)
{
	std::string const* otherName;

	while (true)
	{
		otherName = &findMissionName(alias);
		if (otherName->empty())
			break;
		nlwarning("Replacing mission name for alias %u from '%s' to '%s'", alias, otherName->c_str(), missionName.c_str());
		vector<uint32> &aliases = _MissionToAlias[*otherName];
		aliases.erase(find(aliases.begin(), aliases.end(), alias));
	}

	vector<uint32>& aliases = _MissionToAlias[missionName];
	if (std::find(aliases.begin(), aliases.end(), alias) == aliases.end())
		aliases.push_back(alias);
}

std::string const& CAIInstance::findMissionName(uint32 alias)
{
	map<string, vector<uint32> >::iterator it, itEnd(_MissionToAlias.end());
	for (it=_MissionToAlias.begin(); it!=itEnd; ++it)
	{
		vector<uint32>::iterator it2, itEnd2(it->second.end());
		for (it2=it->second.begin(); it2!=itEnd2; ++it2)
		{
			if (*it2 == alias)
				return it->first;
		}
	}
	static string emptyString;
	return emptyString;
}

void CAIInstance::update()
{
	// Call the player manager's updates
	{
		H_AUTO(PlayerMgrUpdate)
		getPlayerMgr()->update();
	}

	// Call the managers' updates
	{
		H_AUTO(ManagersUpdate)
		FOREACH(it, CCont<CManager>, _Managers)
			it->CManager::update();
	}

	// call continent's update
	{
		H_AUTO(ContinentsUpdate)
		FOREACH(it, CCont<CContinent>, _Continents)
			it->CContinent::update();
	}
}

CManager* CAIInstance::tryToGetManager(char const* str)
{
	return dynamic_cast<CManager*>(tryToGetEntity(str, CAIS::AI_MANAGER));
}

CGroup* CAIInstance::tryToGetGroup(char const* str)
{
	return dynamic_cast<CGroup*>(tryToGetEntity(str, CAIS::AI_GROUP));
}

// :TODO: Document that function
CAIEntity* CAIInstance::tryToGetEntity(char const* str, CAIS::TSearchType searchType)
{
	char	ident[512];
	char*	id	=	ident;

	strcpy(ident,str);

	char		*mgr=NULL;
	char		*grp=NULL;
	char		*bot=NULL;
	char		lastChar;
	CManager	*mgrPtr=NULL;
	CGroup		*grpPtr=NULL;
	CBot		*botPtr=NULL;
	uint32		localIndex=~0;

	mgr	=	id;
	while((*id!=':')&&(*id!=0))		id++;
	lastChar =	*id;
	*id=0;
	id++;

	localIndex=getInt64FromStr(mgr);
	if (localIndex<managers().size())
		mgrPtr	=	managers()[localIndex];
	if	(!mgrPtr)
		goto tryWithEntityId;
	if	(	lastChar==0
		||	searchType==CAIS::AI_MANAGER)
		return	dynamic_cast<CAIEntity*>	(mgrPtr);

	grp	=	id;
	while((*id!=':')&&(*id!=0))		id++;
	lastChar =	*id;
	*id=0;
	id++;

	grpPtr	=	mgrPtr->getGroup(getInt64FromStr(grp));
	if	(!grpPtr)
		goto tryWithEntityId;
	if	(	lastChar==0
		||	searchType==CAIS::AI_GROUP)
		return	dynamic_cast<CAIEntity*>	(grpPtr);

	bot	=	id;
	while((*id!=':')&&(*id!=0))		id++;
	lastChar =	*id;
	*id=0;

	botPtr	=	grpPtr->getBot(getInt64FromStr(bot));
	if	(!botPtr)
		goto tryWithEntityId;
	if	(	lastChar==0
		||	searchType==CAIS::AI_BOT)
		return	dynamic_cast<CAIEntity*>	(botPtr);

	return	NULL;

tryWithEntityId:

	CEntityId	entityId;
	entityId.fromString(str);

	if (entityId.isUnknownId())
		return	NULL;

	CCont<CAIInstance>::iterator instanceIt=CAIS::instance().AIList().begin(), instanceItEnd=CAIS::instance().AIList().end();

	while (instanceIt!=instanceItEnd)
	{
		CAIInstance	*instancePtr=*instanceIt;

		CCont<CManager>::iterator it=instancePtr->managers().begin(), itEnd=instancePtr->managers().end();

		while (it!=itEnd)
		{
			mgrPtr	=	*it;

			grpPtr	=	mgrPtr->getNextValidGroupChild	();
			while (grpPtr)
			{
				botPtr	=	grpPtr->getNextValidBotChild();
				while (botPtr)
				{
					if (botPtr->isSpawned() && botPtr->getSpawnObj()->getEntityId() == entityId)
//					if	(botPtr->createEntityId()==entityId)
						return	dynamic_cast<CAIEntity*>	(botPtr);
					botPtr	=	grpPtr->getNextValidBotChild	(botPtr);
				}
				grpPtr=mgrPtr->getNextValidGroupChild	(grpPtr);
			}
			++it;
		}
		++instanceIt;
	}

	return	NULL;
}

//--------------------------------------------------------------------------
// factory method newMgr()
//--------------------------------------------------------------------------

// factory for creating new managers and for adding them to the _Managers map
// asserts if the id is invalid (<0 or >1023)
// asserts if the id is already in use
CManager* CAIInstance::newMgr(AITYPES::TMgrType type, uint32 alias, std::string const& name, std::string const& mapName, std::string const& filename)
{

	CManager* mgr = CManager::createManager(type, this, alias, name, "", filename);
	// add the manager into the singleton's managers vector
	if (mgr)
	{
		_Managers.addChild(mgr);
		mgr->init	();
	}
	return mgr;
}

bool CAIInstance::spawn()
{
	// Spawn instance managers
	CCont<CManager>::iterator itManager, itManagerEnd(_Managers.end());
	for (itManager=_Managers.begin(); itManager!=itManagerEnd; ++itManager)
	{
		CManager* manager = *itManager;
		manager->spawn();
	}
	// Spawn continents
	CCont<CContinent>::iterator itContinent, itContinentEnd(continents().end());
	for (itContinent=continents().begin(); itContinent!=itContinentEnd; ++itContinent)
	{
		CContinent* continent = *itContinent;
		continent->spawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

bool CAIInstance::despawn()
{
	// Despawn instance managers
	CCont<CManager>::iterator itManager, itManagerEnd(_Managers.end());
	for (itManager=_Managers.begin(); itManager!=itManagerEnd; ++itManager)
	{
		CManager* manager = *itManager;
		manager->despawnMgr();
	}
	// Despawn continents
	CCont<CContinent>::iterator itContinent, itContinentEnd(_Continents.end());
	for (itContinent=_Continents.begin(); itContinent!=itContinentEnd; ++itContinent)
	{
		CContinent* continent = *itContinent;
		continent->despawn();
	}
	// We should check individual errors, but fake success here :)
	return true;
}

//----------------------------------------------------------------------------
// deleteMgr()
//----------------------------------------------------------------------------

// erase a manager (free resources, free id, etc, etc
// asserts if the id is invalid (<0 or >1023)
// displays a warning and returns cleanly if the id is unused
void CAIInstance::deleteMgr(sint mgrId)
{
	_Managers.removeChildByIndex(mgrId);
}

//----------------------------------------------------------------------------
// Interface to kami management
//----------------------------------------------------------------------------

void CAIInstance::registerKamiDeposit(uint32 alias, CGroupNpc* grp)
{
	if (_KamiDeposits.find(alias) !=_KamiDeposits.end() && _KamiDeposits[alias]!=grp)
	{
		nlwarning("Failed to register Kami deposit '%s' with alias '%u' because alias already assigned to deposit '%s'",
			(grp==NULL || grp->getAliasNode()==NULL) ? "NULL" : grp->getAliasNode()->fullName().c_str(),
			alias,
			(_KamiDeposits[alias]==NULL || _KamiDeposits[alias]->getAliasNode()==NULL)? "NULL": _KamiDeposits[alias]->getAliasNode()->fullName().c_str()
			);
		return;
	}
	if (grp!=NULL)
		_KamiDeposits[alias] = grp;
	else
		nlwarning("Attempt to assign Kami deposit alias '%u' to an inexistant group",alias);
}

void CAIInstance::unregisterKamiDeposit(uint32 alias)
{
	if (_KamiDeposits.find(alias) !=_KamiDeposits.end())
		_KamiDeposits.erase(_KamiDeposits.find(alias));
}

//----------------------------------------------------------------------------

std::string	CAIInstance::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":i%u", getChildIndex());
}

std::string	CAIInstance::getOneLineInfoString() const
{
	return "AI instance number " + NLMISC::toString("%u", getInstanceNumber()) + ", continent '" + getContinentName() + "'";
}

std::vector<std::string> CAIInstance::getMultiLineInfoString() const
{
	std::vector<std::string> container;


	pushTitle(container, "CAIInstance");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " num=" + NLMISC::toString("%u", getInstanceNumber());
	container.back() += " continent=" + getContinentName();
	pushFooter(container);


	return container;
}

//----------------------------------------------------------------------------

#include "ai_bot_npc.h"
#include "ai_profile_npc.h"

inline
static CAIVector randomPos(double dispersionRadius)
{
	if (dispersionRadius<=0.)
	{
		return CAIVector(0., 0.);
	}
	uint32 const maxLimit=((uint32)~0U)>>1;
	double rval = (double)CAIS::rand32(maxLimit)/(double)maxLimit; // [0-1[
	double r = dispersionRadius*sqrt(rval);
	rval = (double)CAIS::rand32(maxLimit)/(double)maxLimit; // [0-1[
	double t = 2.0*NLMISC::Pi*rval;
	double dx = cos(t)*r;
	double dy = sin(t)*r;
	return CAIVector(dx, dy);
}

inline
static float randomAngle()
{
	uint32 const maxLimit = CAngle::PI*2;
	float val = (float)CAIS::rand32(maxLimit);
	return val;
}

CGroupNpc* CAIInstance::eventCreateNpcGroup(uint nbBots, NLMISC::CSheetId const& sheetId, CAIVector const& pos, double dispersionRadius, bool spawnBots, double orientation, const std::string &botsName, const std::string &look)
{
	if (!_EventNpcManager)
		return NULL;

	AISHEETS::ICreatureCPtr sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
	if (!sheet)
	{
		nlwarning("invalid sheet while creating event npc group");
		return NULL;
	}

	// Create a group
	CGroupNpc* grp = new CGroupNpc(_EventNpcManager, NULL, RYAI_MAP_CRUNCH::Nothing);
	// Register it in the manager
	_EventNpcManager->groups().addAliasChild(grp);
	// Set the group parameters
	grp->setAutoSpawn(false);
	grp->setName(NLMISC::toString("event_group_%u", grp->getChildIndex()));
	grp->clearParameters();
	grp->setPlayerAttackable(true);
	grp->setBotAttackable(true);

	grp->autoDestroy(true);

	grp->clrBotsAreNamedFlag();

	{
		// build unnamed bot
		for	(uint i=0; i<nbBots; ++i)
		{
			grp->bots().addChild(new CBotNpc(grp, 0, botsName.empty() ? grp->getName():botsName), i); // Doub: 0 instead of getAlias()+i otherwise aliases are wrong

			CBotNpc* const bot = NLMISC::safe_cast<CBotNpc*>(grp->bots()[i]);

			bot->setSheet(sheet);
			if (!look.empty())
				bot->setClientSheet(look);
			bot->equipmentInit();
			bot->initEnergy(/*groupEnergyCoef()*/0);
			CAIVector rpos(pos);
			// Spawn all randomly except if only 1 bot
			if (nbBots > 1)
			{
				RYAI_MAP_CRUNCH::CWorldMap const& worldMap = CWorldContainer::getWorldMap();
				RYAI_MAP_CRUNCH::CWorldPosition	wp;
				uint32 maxTries = 100;
				do
				{
					rpos = pos;
					rpos += randomPos(dispersionRadius);
					--maxTries;
				}
				while (!worldMap.setWorldPosition(AITYPES::vp_auto, wp, rpos) && maxTries>0);
				if (maxTries<=0)
					rpos = pos;
			}

			float angle = 0.f;
			if (orientation < (NLMISC::Pi * 2.0) && orientation > (-NLMISC::Pi * 2.0))
				angle = (float)orientation;
			else
				angle = randomAngle();

			bot->setStartPos(rpos.x().asDouble(),rpos.y().asDouble(), angle, AITYPES::vp_auto);
		}
	}

	grp->spawn();
	CSpawnGroupNpc* spawnGroup = grp->getSpawnObj();
	if	(!spawnGroup)
	{
		// the spawning has failed, delete the useless object
		nlwarning("Failed to spawn the event group");
		_EventNpcManager->groups().removeChildByIndex(grp->getChildIndex());
		return NULL;
	}

	NLMISC::CSmartPtr<CNpcZonePlaceNoPrim> destZone = NLMISC::CSmartPtr<CNpcZonePlaceNoPrim>(new CNpcZonePlaceNoPrim());
	destZone->setPosAndRadius(AITYPES::vp_auto, CAIPos(pos, 0, 0), (uint32)(dispersionRadius*1000.));
	spawnGroup->movingProfile().setAIProfile(new CGrpProfileWanderNoPrim(spawnGroup, destZone));

	if (spawnBots)
		grp->getSpawnObj()->spawnBots();

	return grp;
}

CBotEasterEgg* CAIInstance::createEasterEgg(uint32 easterEggId, NLMISC::CSheetId const& sheetId, std::string const& botName, double x, double y, double z, double heading, const std::string& look)
{
	if (_EasterEggManager == NULL)
		return NULL;

	if (_EasterEggGroup == NULL)
	{
		// create the easter egg group
		CGroupNpc* grp = new CGroupNpc(_EasterEggManager, NULL, RYAI_MAP_CRUNCH::Nothing);
		_EasterEggManager->groups().addAliasChild(grp);

		// set the group parameters
		grp->setAutoSpawn(false);
		grp->setName("easter_egg_group");
		grp->clearParameters();
		grp->setPlayerAttackable(true);
		grp->setBotAttackable(true); // if false the bot will not be attackable by players...
		grp->autoDestroy(false);

		// spawn the group
		grp->spawn();

		CSpawnGroupNpc* spawnGroup = grp->getSpawnObj();
		if (spawnGroup == NULL)
		{
			// the spawning has failed, delete the useless object
			nlwarning("Failed to spawn the easter egg group");
			_EasterEggManager->groups().removeChildByIndex(grp->getChildIndex());
			return NULL;
		}

		// easter eggs do not move
		spawnGroup->movingProfile().setAIProfile(new CGrpProfileIdle(spawnGroup));

		// keep a pointer on the easter egg group
		_EasterEggGroup = grp;
	}
	nlassert(_EasterEggGroup != NULL);

	if (getEasterEgg(easterEggId) != NULL)
	{
		nlwarning("An easter egg with ID %u already exists in instance %u", easterEggId, getInstanceNumber());
		return NULL;
	}

	AISHEETS::ICreatureCPtr sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
	if (!sheet)
	{
		nlwarning("Invalid sheet '%s' while creating an easter egg", sheetId.toString().c_str());
		return NULL;
	}

	// build the easter egg bot
	CBotEasterEgg* bot = new CBotEasterEgg(_EasterEggGroup, 0, botName, easterEggId);
	_EasterEggGroup->bots().addChild(bot);

	bot->setSheet(sheet);
	if (!look.empty()) { bot->setClientSheet(look); }
	bot->equipmentInit();
	bot->initEnergy(0);
	bot->setStartPos(x, y, float(heading), AITYPES::vp_auto);

	bot->spawn();
	if (!bot->isSpawned())
	{
		// the spawning has failed, delete the useless object
		nlwarning("Failed to spawn the easter egg bot");
		_EasterEggGroup->bots().removeChildByIndex(bot->getChildIndex());
		return NULL;
	}

	return bot;
}

void CAIInstance::destroyEasterEgg(uint32 easterEggId)
{
	CBotEasterEgg* bot = getEasterEgg(easterEggId);
	if (bot != NULL)
	{
		bot->despawnBot();
		_EasterEggGroup->bots().removeChildByIndex(bot->getChildIndex());
	}
	else
	{
		nlwarning("Cannot destroy easter egg %u because not found", easterEggId);
	}
}

CBotEasterEgg* CAIInstance::getEasterEgg(uint32 easterEggId)
{
	if (_EasterEggGroup == NULL)
		return NULL;

	FOREACH(itBot, CCont<CBot>, _EasterEggGroup->bots())
	{
		CBotEasterEgg* bot = dynamic_cast<CBotEasterEgg*>(*itBot);
#if !FINAL_VERSION
		nlassert(bot != NULL);
#endif
		if (bot != NULL && bot->getEasterEggId() == easterEggId)
			return bot;
	}

	return NULL;
}

void cbEventCreateNpcGroup( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 messageVersion;
	uint32 instanceNumber;
	sint32 x;
	sint32 y;
	sint32 orientation;
	uint32 nbBots;
	NLMISC::CSheetId sheetId;
	CEntityId playerId;
	double dispersionRadius;
	bool spawnBots;
	std::string botsName;
	std::string look;
	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(instanceNumber);
	msgin.serial(playerId);
	msgin.serial(x);
	msgin.serial(y);
	msgin.serial(orientation);
	msgin.serial(nbBots);
	msgin.serial(sheetId);
	msgin.serial(dispersionRadius);
	msgin.serial(spawnBots);
	msgin.serial(botsName);
	msgin.serial(look);
	CAIInstance* instance = CAIS::instance().getAIInstance(instanceNumber);
	if (instance)
	{
		CGroupNpc* npcGroup = instance->eventCreateNpcGroup(nbBots, sheetId, CAIVector((double)x/1000., (double)y/1000.), dispersionRadius, spawnBots, (double)orientation/1000., botsName, look);
		if (npcGroup != NULL)
		{
			_PlayersLastCreatedNpcGroup[playerId] = npcGroup->getName();
		}
	}
}

extern vector<vector<string> > scriptCommands2;

void cbEventNpcGroupScript( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 messageVersion;
	string botId;
	uint32 nbString;
	vector<string> strings;
	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(nbString);
	
	string eid;
	string firstCommand;
	msgin.serial(eid); // Player or boteid
	msgin.serial(firstCommand); // Player or boteid

	if (firstCommand[0] == '(') // Old eventNpcGroupScript command : (boteid, commands...)
	{
		nlinfo("Event group script with %d strings :", nbString);
		strings.resize(nbString);
		strings[0] = eid;
		nlinfo("  %d '%s'", 0, strings[0].c_str());
		strings[1] = firstCommand;
		nlinfo("  %d '%s'", 1, strings[1].c_str());
		for (uint32 i=2; i<nbString-2; ++i)
		{
			msgin.serial(strings[i]);
			nlinfo("  %d '%s'", i, strings[i].c_str());
		}
	}
	else
	{
		nlinfo("Event group script with %d strings :", nbString-1);
		CEntityId playerId(eid);
		strings.resize(nbString-1);
		NLMISC::CSString groupname = CSString(firstCommand);
		if (firstCommand[0] == '#' && firstCommand[1] == '(')
		{
			NLMISC::CEntityId botEId = NLMISC::CEntityId(firstCommand.substr(1));
			if (botEId==NLMISC::CEntityId::Unknown)
				return;
			CAIEntityPhysical* entity = CAIEntityPhysicalLocator::getInstance()->getEntity(botEId);
			CSpawnBotNpc* bot = dynamic_cast<CSpawnBotNpc*>(entity);
			if (!bot)
				return;
			if (!bot->getPersistent().getOwner())
				return;

			strings[0] = bot->getPersistent().getOwner()->getName();
		}
		else
		{
			strings[0] =  (string)groupname.replace("#last", _PlayersLastCreatedNpcGroup[playerId].c_str());
		}
		nlinfo("  %d '%s'", 0, strings[0].c_str());
		for (uint32 i=1; i<nbString-1; ++i)
		{
			msgin.serial(strings[i]);
			nlinfo("  %d '%s'", i, strings[i].c_str());
		}
	}
	scriptCommands2.push_back(strings);
}

void cbEventFaunaBotSetRadii( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 messageVersion;
	string botName;
	float notHungryRadius;
	float hungryRadius;
	float huntingRadius;

	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(botName);
	msgin.serial(notHungryRadius);
	msgin.serial(hungryRadius);
	msgin.serial(huntingRadius);

	vector<CBot*> bots;
	/// try to find the bot name
	buildFilteredBotList(bots, botName);
	if (bots.empty())
	{
		nlwarning("No bot correspond to name %s", botName.c_str());
		return;
	}
	else
	{
		FOREACH(itBot, vector<CBot*>, bots)
		{
			CBot* bot = *itBot;
			CBotFauna* botFauna = dynamic_cast<CBotFauna*>(bot);
			if (botFauna)
			{
				if (notHungryRadius>=0.f)
					botFauna->setAggroRadiusNotHungry(notHungryRadius);
				if (hungryRadius>=0.f)
					botFauna->setAggroRadiusHungry(hungryRadius);
				if (huntingRadius>=0.f)
					botFauna->setAggroRadiusHunting(huntingRadius);
			}
		}
	}
}

void cbEventFaunaBotResetRadii( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 messageVersion;
	string botName;

	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(botName);

	vector<CBot*> bots;
	/// try to find the bot name
	buildFilteredBotList(bots, botName);
	if (bots.empty())
	{
		nlwarning("No bot correspond to name %s", botName.c_str());
		return;
	}
	else
	{
		FOREACH(itBot, vector<CBot*>, bots)
		{
			CBot* bot = *itBot;
			CBotFauna* botFauna = dynamic_cast<CBotFauna*>(bot);
			if (botFauna)
				botFauna->resetAggroRadius();
		}
	}
}

void cbEventBotCanAggro( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 messageVersion;
	string botName;
	bool canAggro;

	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(botName);
	msgin.serial(canAggro);

	vector<CBot*> bots;
	/// try to find the bot name
	buildFilteredBotList(bots, botName);
	if (bots.empty())
	{
		nlwarning("No bot correspond to name %s", botName.c_str());
		return;
	}
	else
	{
		FOREACH(itBot, vector<CBot*>, bots)
		{
			CBot* bot = *itBot;
			if (bot)
			{
				CSpawnBot* spBot = bot->getSpawnObj();
				if (spBot)
					spBot->setCanAggro(canAggro);
			}
		}
	}
}

void cbEventBotSheet( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 messageVersion;
	string botName;
	NLMISC::CSheetId sheetId;
	bool autoSpawnDespawn;
	string customName;

	msgin.serial(messageVersion);
	nlassert(messageVersion==3);
	msgin.serial(botName);
	msgin.serial(sheetId);
	msgin.serial(autoSpawnDespawn);
	msgin.serial(customName);

	vector<CBot*> bots;
	/// try to find the bot name
	buildFilteredBotList(bots, botName);
	if (bots.empty())
	{
		nlwarning("No bot correspond to name %s", botName.c_str());
		return;
	}
	else
	{
		AISHEETS::ICreatureCPtr const sheet = AISHEETS::CSheets::getInstance()->lookup(sheetId);
		if (sheetId!=NLMISC::CSheetId::Unknown && !sheet.isNull())
		{
			FOREACH(itBot, vector<CBot*>, bots)
			{
				CBot* bot = *itBot;
				if (bot)
				{
					bot->setCustomName(customName);
					bot->triggerSetSheet(sheet);
					if (autoSpawnDespawn && !bot->isSpawned())
						bot->spawn();
				}
			}
		}
		else if (autoSpawnDespawn)
		{
			FOREACH(itBot, vector<CBot*>, bots)
			{
				CBot* bot = *itBot;
				if (bot && bot->isSpawned())
					bot->despawnBot();
			}
		}
	}
}

extern vector<vector<string> > scriptCommandsBotById;

void cbR2NpcBotScriptById(NLNET::CMessage& msgin, const std::string& serviceName, NLNET::TServiceId serviceId)
{
	uint32 messageVersion;
	string botId;
	uint32 nbString;
	vector<string> strings;
	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(nbString);
	strings.resize(nbString);
	for (uint32 i=0; i<nbString; ++i)
		msgin.serial(strings[i]);
	scriptCommandsBotById.push_back(strings);
}

extern vector<vector<string> > scriptCommandsGroupByName;

void cbR2NpcGroupScriptByName(NLNET::CMessage& msgin, const std::string& serviceName, NLNET::TServiceId serviceId)
{
	uint32 messageVersion;
	string botId;
	uint32 nbString;
	vector<string> strings;
	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(nbString);
	strings.resize(nbString);
	for (uint32 i=0; i<nbString; ++i)
		msgin.serial(strings[i]);
	scriptCommandsGroupByName.push_back(strings);
}

//////////////////////////////////////////////////////////////////////////////
// CBotDespawnNotification                                                  //
//////////////////////////////////////////////////////////////////////////////

class CBotDespawnNotification : private CBot::IObserver
{
public:
	/// get the singleton instance
	static CBotDespawnNotification* getInstance();

	/// register a service to be notified when a bot will despawn
	void registerService(NLNET::TServiceId serviceId, CBot* bot, const NLMISC::CEntityId& botId);

	/// dump in a log
	void dump(CLog& log) const;

private:
	/// private ctor
	CBotDespawnNotification();

	/// callback of the observer
	void notifyBotDespawn(CBot* bot);
	void notifyBotDeath(CBot* bot);
	void notifyStopNpcControl(CBot* bot);

	/// the singleton instance
	static CBotDespawnNotification*	_Instance;

	class CRegisteredService
	{
	public:
		CRegisteredService(NLNET::TServiceId serviceId, const CEntityId& botId)
			: _ServiceId(serviceId)
			, _BotId(botId)
		{
		}

		NLNET::TServiceId getServiceId() const { return _ServiceId; }
		const CEntityId& getBotId() const { return _BotId; }

		bool operator==(const CRegisteredService& other) const
		{
			return (_ServiceId == other._ServiceId && _BotId == other._BotId);
		}

	private:
		NLNET::TServiceId		_ServiceId;
		CEntityId	_BotId;
	};

	/// registered services by bot alias
	typedef std::multimap<uint32, CRegisteredService> TRegisteredServiceMap;
	TRegisteredServiceMap	_RegisteredServices;
};

CBotDespawnNotification* CBotDespawnNotification::_Instance = NULL;

CBotDespawnNotification::CBotDespawnNotification()
{
}

CBotDespawnNotification* CBotDespawnNotification::getInstance()
{
	if (_Instance == NULL)
		_Instance = new CBotDespawnNotification;
	return _Instance;
}

void CBotDespawnNotification::registerService(NLNET::TServiceId serviceId, CBot* bot, const NLMISC::CEntityId& botId)
{
	nlassert(bot != NULL);

	TRegisteredServiceMap::const_iterator first	= _RegisteredServices.lower_bound(bot->getAlias());
	TRegisteredServiceMap::const_iterator last	= _RegisteredServices.upper_bound(bot->getAlias());

	if (first == last)
		bot->attachObserver(this);

	CRegisteredService rs(serviceId, botId);

	for (; first != last; ++first)
	{
		if ((*first).second == rs)
			break;
	}
	if (first == last)
		_RegisteredServices.insert( make_pair(bot->getAlias(), rs) );
}

void CBotDespawnNotification::dump(CLog& log) const
{
	FOREACHC(it, TRegisteredServiceMap, _RegisteredServices)
	{
		const CRegisteredService& rs = (*it).second;
		log.displayNL("BotAlias %s => ServiceId %hu, BotId %s",
			LigoConfig.aliasToString((*it).first).c_str(),
			rs.getServiceId().get(),
			rs.getBotId().toString().c_str()
			);
	}
}

void CBotDespawnNotification::notifyBotDespawn(CBot* bot)
{
	nlassert(bot != NULL);

	// catch this event only once for current services registered for this bot
	bot->detachObserver(this);

	TRegisteredServiceMap::iterator first	= _RegisteredServices.lower_bound(bot->getAlias());
	TRegisteredServiceMap::iterator last	= _RegisteredServices.upper_bound(bot->getAlias());
	if (first == last)
	{
		nlwarning("No service requested despawn notification for bot %s",
			LigoConfig.aliasToString(bot->getAlias()).c_str()
			);
		DEBUG_STOP;
		return;
	}

	// notify all services registered for this bot
	while (first != last)
	{
		const CRegisteredService& rs = (*first).second;
		CMessages::notifyBotDespawn(rs.getServiceId(), bot->getAlias(), rs.getBotId());

		TRegisteredServiceMap::iterator tmp = first;
		++first;
		_RegisteredServices.erase(tmp);
	}
}

void CBotDespawnNotification::notifyStopNpcControl(CBot* bot)
{
	nlassert(bot != NULL);

	// catch this event only once for current services registered for this bot
	//bot->detachObserver(this);

	TRegisteredServiceMap::iterator first	= _RegisteredServices.lower_bound(bot->getAlias());
	TRegisteredServiceMap::iterator last	= _RegisteredServices.upper_bound(bot->getAlias());


	// notify all services registered for this bot
	while (first != last)
	{
		const CRegisteredService& rs = (*first).second;
		CMessages::notifyBotStopNpcControl(rs.getServiceId(), bot->getAlias(), rs.getBotId());

		TRegisteredServiceMap::iterator tmp = first;
		++first;
	//	_RegisteredServices.erase(tmp);
	}
}
void CBotDespawnNotification::notifyBotDeath(CBot* bot)
{
		nlassert(bot != NULL);

	// catch this event only once for current services registered for this bot
	bot->detachObserver(this);

	TRegisteredServiceMap::iterator first	= _RegisteredServices.lower_bound(bot->getAlias());
	TRegisteredServiceMap::iterator last	= _RegisteredServices.upper_bound(bot->getAlias());
	if (first == last)
	{
		nlwarning("No service requested death notification for bot %s",
			LigoConfig.aliasToString(bot->getAlias()).c_str()
			);
		DEBUG_STOP;
		return;
	}

	// notify all services registered for this bot
	while (first != last)
	{
		const CRegisteredService& rs = (*first).second;
		CMessages::notifyBotDeath(rs.getServiceId(), bot->getAlias(), rs.getBotId());

		TRegisteredServiceMap::iterator tmp = first;
		++first;
		_RegisteredServices.erase(tmp);
	}
}


void cbAskBotDespawnNotification(NLNET::CMessage& msgin, const std::string& serviceName, NLNET::TServiceId serviceId)
{
	uint32 messageVersion;
	uint32 botAlias;
	CEntityId botId;

	msgin.serial(messageVersion);
	nlassert(messageVersion == 1);
	msgin.serial(botAlias);
	msgin.serial(botId);

	TDataSetRow botRowId = CMirrors::getDataSetRow(botId);
	if (botRowId.isNull())
	{
		CMessages::notifyBotDespawn(serviceId, botAlias, botId);
		return;
	}

	CAIEntityPhysical* entity = CAIS::instance().getEntityPhysical(botRowId);
	if	(entity == NULL)
	{
		CMessages::notifyBotDespawn(serviceId, botAlias, botId);
		return;
	}

#if !FINAL_VERSION
	nlassert(entity->getEntityId() == botId);
#endif

	CSpawnBot* bot = NULL;
	switch (entity->getRyzomType())
	{
	case RYZOMID::creature:
	case RYZOMID::npc:
		bot = NLMISC::safe_cast<CSpawnBot*>(entity);
		break;
	default:
		break;
	}

	// check if the bot has been replaced by another entity in mirror
	if (bot == NULL || bot->getPersistent().getAlias() != botAlias)
	{
		CMessages::notifyBotDespawn(serviceId, botAlias, botId);
		return;
	}

	// register the service to be notified when the bot will despawn
	CBotDespawnNotification::getInstance()->registerService(serviceId, &bot->getPersistent(), botId);
}

void cbSpawnEasterEgg(NLNET::CMessage& msgin, const std::string& serviceName, NLNET::TServiceId serviceId)
{
	uint32 messageVersion;
	uint32 instanceNumber;
	uint32 easterEggId;
	CSheetId sheetId;
	string botName;
	string look;

	sint32 x, y, z;
	float heading;

	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(instanceNumber);
	msgin.serial(easterEggId);
	msgin.serial(sheetId);
	msgin.serial(botName);
	msgin.serial(look);
	msgin.serial(x, y, z, heading);

	CAIInstance* instance = CAIS::instance().getAIInstance(instanceNumber);
	if (instance != NULL)
	{
		CBotEasterEgg* bot = instance->createEasterEgg(easterEggId, sheetId, botName, float(x)/1000., float(y)/1000., float(z)/1000., heading, look );
		if (bot != NULL)
		{
			nlassert(bot->getSpawn() != NULL);
			CEntityId botId = bot->getSpawn()->getEntityId();

			NLNET::CMessage msgout("EASTER_EGG_SPAWNED");
			msgout.serial(botId);
			msgout.serial(easterEggId);
			sendMessageViaMirror(serviceId, msgout);
		}
		else
		{
			nlwarning("Cannot spawn easter egg %u!", easterEggId);
		}
	}
	else
	{
		nlwarning("Cannot spawn easter egg %u, AI instance %u not found!", easterEggId, instanceNumber);
	}
}

void cbDespawnEasterEgg(NLNET::CMessage& msgin, const std::string& serviceName, NLNET::TServiceId serviceId)
{
	uint32 messageVersion;
	uint32 instanceNumber;
	uint32 easterEggId;

	msgin.serial(messageVersion);
	nlassert(messageVersion==1);
	msgin.serial(instanceNumber);
	msgin.serial(easterEggId);

	CAIInstance* instance = CAIS::instance().getAIInstance(instanceNumber);
	if (instance != NULL)
	{
		instance->destroyEasterEgg(easterEggId);
	}
}

NLMISC_COMMAND(simulateMsgAskBotDespawnNotification, "", "<service_id> <bot_alias> <bot_id>")
{
	if (args.size() != 3)
		return false;

	uint16 id;
	NLMISC::fromString(args[0], id);
	NLNET::TServiceId serviceId(id);
	uint32 botAlias = LigoConfig.aliasFromString(args[1]);
	CEntityId botId(args[2].c_str());

	NLNET::CMessage msg("ASK_BOT_DESPAWN_NOTIFICATION");
	uint32 messageVersion = 1;
	msg.serial(messageVersion, botAlias, botId);
	msg.invert();
	cbAskBotDespawnNotification(msg, "FAKE", serviceId);

	return true;
}

NLMISC_COMMAND(dumpBotDespawnNotification, "", "")
{
	if (args.size() != 0)
		return false;

	CBotDespawnNotification::getInstance()->dump(log);
	return true;
}

NLMISC_COMMAND(simulateMsgSpawnEasterEgg, "", "<service_id> <ai_instance> <easter_egg_id> <sheet> <bot_name> <x> <y>")
{
	if (args.size() != 7)
		return false;

	uint16 id;
	NLMISC::fromString(args[0], id);
	NLNET::TServiceId serviceId(id);
	uint32 instanceNumber;
	NLMISC::fromString(args[1], instanceNumber);
	uint32 easterEggId;
	NLMISC::fromString(args[2], easterEggId);
	CSheetId sheetId(args[3]);
	string botName = args[4];
	sint32 x;
	NLMISC::fromString(args[5], x);
	sint32 y;
	NLMISC::fromString(args[6], y);
	sint32 z = 0;
	sint32 heading = 0;
	std::string look = "";

	NLNET::CMessage msg("SPAWN_EASTER_EGG");
	uint32 messageVersion = 1;
	msg.serial(messageVersion);
	msg.serial(instanceNumber);
	msg.serial(easterEggId);
	msg.serial(sheetId);
	msg.serial(botName);
	msg.serial(look);

	msg.serial(x, y, z, heading);
	msg.invert();
	cbSpawnEasterEgg(msg, "FAKE", serviceId);

	return true;
}

NLMISC_COMMAND(simulateMsgDespawnEasterEgg, "", "<service_id> <ai_instance> <easter_egg_id>")
{
	if (args.size() != 3)
		return false;

	uint16 id;
	NLMISC::fromString(args[0], id);
	NLNET::TServiceId serviceId(id);
	uint32 instanceNumber;
	NLMISC::fromString(args[1], instanceNumber);
	uint32 easterEggId;
	NLMISC::fromString(args[2], easterEggId);

	NLNET::CMessage msg("DESPAWN_EASTER_EGG");
	uint32 messageVersion = 1;
	msg.serial(messageVersion);
	msg.serial(instanceNumber);
	msg.serial(easterEggId);
	msg.invert();
	cbDespawnEasterEgg(msg, "FAKE", serviceId);

	return true;
}
