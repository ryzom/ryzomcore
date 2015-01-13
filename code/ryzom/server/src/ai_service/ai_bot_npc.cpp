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

#include "game_share/visual_slot_manager.h"
#include "server_share/r2_variables.h"

#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "ai_mgr_npc.h"
#include "ai_player.h"
#include "states.h"
#include "ai_profile_npc.h"
#include "ai_control_npc.h"

#include "ais_user_models.h"
#include "dyn_grp_inline.h"

using namespace MULTI_LINE_FORMATER;

using namespace std;
using namespace RYAI_MAP_CRUNCH;

// Stuff used for management of log messages
static bool VerboseLog = false;
#define LOG if (!VerboseLog) { } else nlinfo

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotNpc                                                             //
//////////////////////////////////////////////////////////////////////////////

CSpawnBotNpc::CSpawnBotNpc(TDataSetRow const& row, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CSpawnBot(row, owner, id, radius, level, denyFlags)
{
	_OldHpPercentage = -1.f;
	_NbCurrentDynChats = 0;
	_FacingTheta = 0;
	_FacingTick = 0;
}

bool CSpawnBotNpc::isBotAttackable() const
{
	return getPersistent().grp().getBotAttackable();
}

void CSpawnBotNpc::sendInfoToEGS() const
{
	if (!EGSHasMirrorReady)
		return;
	
	CSpawnBot::sendInfoToEGS();

	TGenNpcDescMsgImp msg;
	msg.setEntityIndex(dataSetRow());
	
	getPersistent().fillDescriptionMsg(msg);
	
	msg.setChat(_CurrentChatProfile);
	msg.send("EGS");
}

CSpawnGroupNpc& CSpawnBotNpc::spawnGrp() const
{
	return static_cast<CSpawnGroupNpc&>(CSpawnBot::spawnGrp());
}

void CSpawnBotNpc::processEvent(CCombatInterface::CEvent const& event)
{
	// no self aggro.
	if (event._targetRow==event._originatorRow)
		return;
	
	if ((event._nature==ACTNATURE::FIGHT || event._nature==ACTNATURE::OFFENSIVE_MAGIC) && !getPersistent().ignoreOffensiveActions())
	{
		float aggro = event._weight;
		if (aggro > -0.20f)
		{
			aggro = -0.20f;
		}
		if (event._nature==ACTNATURE::OFFENSIVE_MAGIC)
		{
			aggro=(1.f+aggro)*0.5f-1.f; // maximize aggro for magic
			//insure if aggressor is player, player have it's target seted for BOSS assist
			CAIEntityPhysical	*ep=CAIS::instance().getEntityPhysical(event._originatorRow);
			CBotPlayer	*player=dynamic_cast<CBotPlayer*>(ep);
			if(player)
			{
				CAIEntityPhysical	*target=player->getVisualTarget();
				if (target)
					player->setTarget(target);
			}
		}
		addAggroFor(event._originatorRow, aggro, true);
		spawnGrp().addAggroFor(event._originatorRow, aggro, true);
	}
}

 void CSpawnBotNpc::update(uint32 ticks)
{
	++AISStat::BotTotalUpdCtr;
	++AISStat::BotNpcUpdCtr;
	
	{
		H_AUTO(AIHpTrig);
		// Fix for HP triggers
		// :FIXME: Clean that triggering stuff, make it generic
		CGroupNpc& persGrp = spawnGrp().getPersistent();
		if (persGrp.haveHpTriggers())
		{
			float newHpPercentage = getPhysical().hpPercentage();
			if (_OldHpPercentage>=0.f && newHpPercentage!=_OldHpPercentage)
			{
				persGrp.hpTriggerCb(_OldHpPercentage, newHpPercentage);
			}
			_OldHpPercentage = newHpPercentage;
		}
		else
		{
			_OldHpPercentage = getPhysical().hpPercentage();
		}
	}
	
	// Bot chat and dyn chat override AI profile unless fighting
	// The directing adjustment is done client-slide (each player sees the NPC facing him)
	if (!getActiveChats().empty())
		return;
	if (getNbActiveDynChats() > 0)
		return;
	if (isStuned())
		return;

	// position before profile update
	CAIPos const startPos = CAIPos(pos());

	// nullify _PlayerController if it is not valid anymore
	if (_PlayerController != NULL && !_PlayerController->isValid())
	{
		CSpawnBot* sp = _PlayerController->getSpawnBot();
		if (sp)
		{
			sp->getPersistent().notifyStopNpcControl();
		}		

		_PlayerController = NULL;		
		
	}

	if (_PlayerController == NULL)
	{
		if (!getAISpawnProfile().isNull())
		{
			H_AUTO(BotNpcUpdateProfile);
			updateProfile(ticks); // then we can update the current bot profile.
		}
	}
	else
	{
		H_AUTO(BotNpcUpdateControl);
		// if there is a valid player controller, it overrides the current profile
		_PlayerController->updateControl(ticks);
	}

	{
		H_AUTO(BotNpcUpdateAgro);
		// every time we need to update bot aggros.
		this->CBotAggroOwner::update(ticks);
	}

	// If sit and move then stand up

	if (IsRingShard)
	{	
		if (getMode() == MBEHAV::SIT
			&& (x().asInt() != startPos.x().asInt() || y().asInt() != startPos.y().asInt()) )
		{
   			setMode(MBEHAV::NORMAL);
		}

		
		if (_FacingTick != 0)
		{	
			uint32 tick = CTimeInterface::gameCycle();
			if ( (tick - _FacingTick) > 40)
			{
				setTheta(_FacingTheta);
				_FacingTick = 0;
			}
		}
	}

}

void CSpawnBotNpc::setFacing(CAngle theta)
{
	if (_FacingTick == 0)
	{
		_FacingTheta = pos().theta();
	}
	
	setTheta(theta);
	_FacingTick = CTimeInterface::gameCycle();
}

void CSpawnBotNpc::setUserModelId(const std::string &id)
{
	_UserModelId = id;
}

void CSpawnBotNpc::setCustomLootTableId(const std::string &id)
{
	_CustomLootTableId = id;
}


void CSpawnBotNpc::setPrimAlias(uint32 primAlias)
{
	_PrimAlias = primAlias;
}
void CSpawnBotNpc::updateChat(CAIState const* state)
{
	if (!state)
		return;
	CBotNpc const& botNpc = getPersistent();
	
	FOREACHC(itChat, CCont<CAIStateChat>, state->chats())
	{
		if (!itChat->testCompatibility(botNpc))
			continue;
		
		// update chat information if any
		CNpcChatProfileImp const* const chatProfile = botNpc.getChat();
		if (!chatProfile)
		{
			_CurrentChatProfile = CNpcChatProfileImp::combineChatProfile(*chatProfile, itChat->getChat());
			// the chat profile has been combined, send it to EGS
			sendInfoToEGS();
		}
		break;
	}
}

std::vector<std::string> CSpawnBotNpc::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	std::vector<std::string> strings;
	
	
	pushTitle(container, "CSpawnBotNpc");
	strings.clear();
	strings = CSpawnBot::getMultiLineInfoString();
	FOREACHC(itString, std::vector<std::string>, strings)
		pushEntry(container, *itString);
	pushEntry(container, "profile: " + CProfilePtr::getOneLineInfoString());
	pushEntry(container, "state: " + spawnGrp().getPersistent().buidStateInstanceDebugString());
	std::string userModelId = getPersistent().getUserModelId();
	if (!(userModelId.empty()))
	{
		pushEntry(container, "UserModelId: " + userModelId);
	}
	if (_CurrentChatProfile.getMissions().empty())
		pushEntry(container, "no mission");
	else
	{
		vector<uint32> const& missions = _CurrentChatProfile.getMissions();
		pushEntry(container, "missions:");
		for (size_t i=0; i<missions.size(); ++i)
		{
			string name = getAIInstance()->findMissionName(missions[i]);
			pushEntry(container, NLMISC::toString("          %u (%s)", missions[i], name.c_str()));
		}
	}
	pushFooter(container);
	
	
	return container;
}

void CSpawnBotNpc::beginBotChat(CBotPlayer* plr)
{
#ifdef NL_DEBUG
	for (size_t i=0; i<_ActiveChats.size(); ++i)
	{
		if (_ActiveChats[i]==plr)
			nlwarning("Chat pair added more than once!!!");
	}
#endif
	
	// add an entry to the bot chat vector
	_ActiveChats.push_back(plr);
}

void CSpawnBotNpc::endBotChat(CBotPlayer* plr)
{
	// run through the bot chat vector looking for a player to erase
	for (size_t i=0;i<_ActiveChats.size();++i)
	{
		if	(_ActiveChats[i]!=plr)
			continue;
		
		// we've found a match for the player so remove the entry from the _aciveChats vector and return
		_ActiveChats[i]	= _ActiveChats[_ActiveChats.size()-1];
		_ActiveChats.pop_back();
		
#ifdef NL_DEBUG
		for (size_t j=i;j<_ActiveChats.size();++j)
		{
			if (_ActiveChats[i]==plr)
			{
				nlwarning("Chat pair removed but another of the same still exists!!!");
			}
		}
#endif
		return;
	}
	// we should never end up here!
	nlwarning("Chat pair removed but not previously added!!!");
}

void CSpawnBotNpc::propagateAggro() const
{
	if (getAggroPropagationRadius()<0.f)
		return;
	CGroup* pGroup = getPersistent().getOwner();
	CSpawnGroup* spGroup = pGroup->getSpawnObj();
	CDynGrpBase* myDgb = pGroup->getGrpDynBase();
	nlassert(myDgb);
	CFamilyBehavior* myfb = myDgb->getFamilyBehavior();
	
	CAIVision<CPersistentOfPhysical> vision;
	// look for bots around
	vision.updateBotsAndPlayers(pGroup->getAIInstance(), CAIVector(pos()), 0, (uint32)getAggroPropagationRadius());
	
	typedef map<CSpawnBot*, float> TCandidatesCont;
	TCandidatesCont	candidates;
	
	if (myfb!=NULL)
	{
		FOREACH(it, CAIVision<CPersistentOfPhysical>, vision)
		{
			CBot& otherPBot = static_cast<CBot&>(*it);
			CSpawnBot* otherSpBot = otherPBot.getSpawnObj();
			CGroup* otherPGroup = otherPBot.getOwner();
			CSpawnGroup* otherSpGroup = otherPGroup->getSpawnObj();
			
			// If bot is in our group skip it
			if (otherSpGroup==spGroup)
				continue;
			
			// If bot is not in a dynamic system skip it
			CDynGrpBase* otherDGB = otherPGroup->getGrpDynBase();
			if (!otherDGB)
				continue;
			
			// If bot is not in our family skip it
			CFamilyBehavior* otherFB = otherDGB->getFamilyBehavior();
			if (otherFB!=myfb)
				continue;
			
			// ok, this group should help us !
			
			// TODO : take only groups with the 'activity_figth' property
			// filter out if the group is already in combat mode
			
			// If group is not spawned skip it (???) (:TODO: See why this test)
			if (!otherPGroup->isSpawned())
				continue;
			
			CProfilePtr& otherFightProfile = otherSpGroup->fightProfile();
			
			if (!otherFightProfile.getAIProfile() || otherFightProfile.getAIProfileType()!=AITYPES::FIGHT_NORMAL || !(static_cast<CGrpProfileFight*>(otherFightProfile.getAIProfile())->stillHaveEnnemy()))
			{
				float dist = (float)CAIVector(otherSpBot->pos()).quickDistTo(pos());
				if (dist<getAggroPropagationRadius())
					candidates[otherSpBot] = dist;
			}
		}
	}
	
	if (!candidates.empty())
		nldebug("Tick %u, prop aggro from %p to %u bots", CTickEventHandler::getGameCycle(), this, candidates.size());
	
	FOREACH (it, TCandidatesCont, candidates)
	{
		float propFactor = 1.0f - (it->second / getAggroPropagationRadius());
		nldebug("  prop aggro from %p to %p at %f meters, prop factor = %f",
			this,
			it->first,
			it->second,
			propFactor);
		// add aggro list of this group to the other group pondered by distance
		it->first->mergeAggroList(*this, propFactor);
	}
}

float CSpawnBotNpc::getReturnDistCheck() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroReturnDistCheck()>=0.f)
		return getPersistent().getSheet()->AggroReturnDistCheck();
	else
		return AggroReturnDistCheckNpc;
}

void CSpawnBotNpc::setPlayerController(CBotPlayer* player)
{
	if (player != NULL)
	{
		_PlayerController = new CPlayerControlNpc(player, this);
		nlassert(_PlayerController->isValid());

	}
	else
	{
		_PlayerController = NULL;
	}
}

void CSpawnBotNpc::setCurrentChatProfile(CNpcChatProfileImp* chatProfile)
{
	if (chatProfile)
		_CurrentChatProfile = *chatProfile;
	else			
		_CurrentChatProfile.clear();	// clear the chat profile
}

CBotNpc& CSpawnBotNpc::getPersistent() const
{
	return static_cast<CBotNpc&>(CSpawnBot::getPersistent());
}

//////////////////////////////////////////////////////////////////////////////
// CBotNpc                                                                  //
//////////////////////////////////////////////////////////////////////////////

CBotNpc::CBotNpc(CGroup* owner, CAIAliasDescriptionNode* alias)
: CBot(owner, alias)
, _Sheet(NULL)
{
	_Sheet = CBotNpcSheetPtr(new CBotNpcSheet(NULL));
	_Sheet->setSheet(CBot::getSheet());
	init();
}

CBotNpc::CBotNpc(CGroup* owner, uint32 alias, std::string const& name)
: CBot(owner, alias, name)
, _Sheet(NULL)
{
	_Sheet = CBotNpcSheetPtr(new CBotNpcSheet(NULL));
	_Sheet->setSheet(CBot::getSheet());
	init();
}

CBotNpc::~CBotNpc() 
{
	if (isSpawned())
	{
		despawnBot();
	}
}

void CBotNpc::calcSpawnPos(RYAI_MAP_CRUNCH::CWorldMap const& worldMap)
{
	CAIStatePositional const* const state = static_cast<CAIStatePositional*>(grp().getStartState());
	RYAI_MAP_CRUNCH::CWorldPosition	wp;
	uint32 maxTries = 100;
	
	breakable
	{
		if (!state)
			break;
		
		if (state->shape().hasPatat() && state->shape().getRandomPosCount())
		{
			do
			{
				RYAI_MAP_CRUNCH::CWorldPosition wp;
				state->shape().getRandomPos(wp);
				_StartPos.setXY(wp);
				maxTries--;
			} while(!worldMap.setWorldPosition(_VerticalPos, wp, _StartPos) && maxTries);
			_StartPos.setTheta(0); // to initialise among vertices deltas (no?).
			break;
		}
		
		if (!state->shape().hasPoints())
			break;
		
		std::vector<CShape::TPosition> const& posList = state->shape().getGeometry();		
		do
		{
			const	uint32 a=CAIS::rand16((uint32)posList.size());
			const	uint32 b=(a+1)%posList.size();
			const	double weight=CAIS::frand();
			_StartPos.setXY(posList[a].toAIVector()+(posList[b].toAIVector()-posList[a].toAIVector())*weight);
			--maxTries;
		} while	(!worldMap.setWorldPosition(_VerticalPos, wp, _StartPos) && maxTries);
		_StartPos.setTheta(0);	// to initialise among vertices deltas (no?).
	}

	if (!wp.isValid() && !isStuck())
		nlwarning("Cannot generate a valid position for Npc %s", getFullName().c_str());
}

CGroupNpc& CBotNpc::grp() const
{
	return *static_cast<CGroupNpc*>(getOwner());
}

void CBotNpc::setUserModelId(const std::string &userModelId)
{
	if (!userModelId.empty())
	{
		if (!CAIUserModelManager::getInstance()->isUserModel(_PrimAlias, userModelId))
		{
			nlwarning("Error while parsing equipment params: cannot find user model id '%s' associated to primAlias '%u'", userModelId.c_str(), _PrimAlias);
			_UserModelId = "";
			return;
		}
		_UserModelId = userModelId;
	}
}

void CBotNpc::setCustomLootTableId(const std::string &customLootTableId)
{
	if (!customLootTableId.empty())
	{
		if (!CAIUserModelManager::getInstance()->isCustomLootTable(_PrimAlias, customLootTableId))
		{
			nlwarning("Error while parsing equipment params: invalid customLootTableId specified: '%s'", customLootTableId.c_str());
			_CustomLootTableId = "";
			return;
		}
		_CustomLootTableId = customLootTableId;
	}
}

std::string CBotNpc::getUserModelId()
{
	return _UserModelId;
}

std::string CBotNpc::getCustomLootTableId()
{
	return _CustomLootTableId;
}

void CBotNpc::setPrimAlias(uint32 alias) 
{ 
	_PrimAlias = alias;
}


uint32 CBotNpc::getPrimAlias() const 
{ 
	return _PrimAlias; 
}


void CBotNpc::fillDescriptionMsg(RYMSG::TGenNpcDescMsg& msg) const
{
	msg.setPlayerAttackable(grp().getPlayerAttackable());
	msg.setBotAttackable(grp().getBotAttackable());
	msg.setAlias(getAlias());
	msg.setGrpAlias(grp().getAlias());
	
	msg.setSheet(getSheet()->SheetId());
	
	msg.setRightHandItem(getSheet()->RightItem());
	msg.setRightHandItemQuality(1);
	
	msg.setLeftHandItem(getSheet()->LeftItem());
	msg.setLeftHandItemQuality(1);
	
	msg.setDontFollow(isStuck());
	msg.setBuildingBot(isBuildingBot());
	
	for	(size_t i=0; i<_LootList.size(); ++i)
	{
		NLMISC::CSheetId const& sheetRef = _LootList[i];
		if (sheetRef!=NLMISC::CSheetId::Unknown)
			msg.getLootList().push_back(sheetRef);
	}

	CGroupNpc::TFactionAttackableSet const& factionAttackableAbove = grp().getFactionAttackableAbove();
	FOREACHC(itFaction, CGroupNpc::TFactionAttackableSet, factionAttackableAbove)
		msg.getOptionalProperties().push_back("FactionAttackableAbove:" + itFaction->first + ":" + NLMISC::toString(itFaction->second));
	CGroupNpc::TFactionAttackableSet const& factionAttackableBelow = grp().getFactionAttackableBelow();
	FOREACHC(itFaction, CGroupNpc::TFactionAttackableSet, factionAttackableBelow)
		msg.getOptionalProperties().push_back("FactionAttackableBelow:" + itFaction->first + ":" + NLMISC::toString(itFaction->second));

	msg.setMaxHitRangeForPC(_MaxHitRangeForPC);

//	msg.setIsMissionStepIconDisplayable(_MissionIconFlags.IsMissionStepIconDisplayable);
//	msg.setIsMissionGiverIconDisplayable(_MissionIconFlags.IsMissionGiverIconDisplayable);
	msg.setUserModelId(_UserModelId);
	msg.setCustomLootTableId(_CustomLootTableId);
	msg.setPrimAlias(_PrimAlias);
}

// :KLUDGE: This methods is a part of the trick for bot respawn
// :TODO: Clean that mess
bool CBotNpc::finalizeSpawnNpc()
{
	// For squads, setup specific mirror properties
	COutpost* ownerOutpost = dynamic_cast<COutpost*>(getOwner()->getOwner()->getOwner());
	if (ownerOutpost)
	{
		// Propagate the alliance id and outpost alias to the instanciated bot
		// Currently, all squads are assumed to be defender of the outpost (not attacker)
		getSpawn()->setOutpostAlias(ownerOutpost->getAlias());
		getSpawn()->setOutpostSide(_OutpostSide);
	}
	
	CMirrors::initSheetServer(getSpawn()->dataSetRow(), getSheet()->SheetId());
	
	getSpawn()->setCurrentChatProfile(_ChatProfile);
	getSpawn()->sendInfoToEGS();
	
	if (_useVisualProperties)	// use VisualPropertyA, B, C
	{
		sendVisualProperties();
	}
	else	// use alternate VPA
	{
		sendVPA	();
	}
	
	getSpawn()->spawnGrp().botHaveSpawn(this);
	
	return true;
}

void CBotNpc::initAdditionalMirrorValues()
{
	// Write the Mission Alias to mirror now - it must be done before CMirrors::declareEntity()
	// to ensure the FS will receive the Sheet and it at the same time, hence it can't be
	// done in finalizeSpawnNpc(), which is called after that
	EGSPD::CPeople::TPeople race = getSheet()->Race();
	if (race < EGSPD::CPeople::Creature ||
		race == EGSPD::CPeople::Kami ||
		race == EGSPD::CPeople::Unknown) // only for humanoid NPCs and bot objects (beware, even some creatures have the entity type RYZOMID::npc)
	{
		//if ((_ChatProfile != NULL) && (!_ChatProfile->getMissions().empty())) // only if the bot has missions to give: not
		CMirrors::initNPCAlias(getSpawn()->dataSetRow(), getAlias());
	}
}

bool CBotNpc::spawn()
{
	if (!isSheetValid())
	{
		nlwarning("spawn() Aborted for bot '%s' due to bad sheet", getFullName().c_str());
		return false;
	}
	
	if (!CBot::spawn())
		return false;
	
	// :KLUDGE: Last part calls a tricky method also called by sheetChanged
	// :TODO: Clean that mess
	return finalizeSpawnNpc();
}

void CBotNpc::sendVPA()	// alternate VPA
{
	SAltLookProp visProp;
	//	sets weapons information.
	{
		CVisualSlotManager* visualSlotManager = CVisualSlotManager::getInstance();
		NLMISC::CSheetId rightSheet = getSheet()->RightItem();
		NLMISC::CSheetId leftSheet = getSheet()->LeftItem();
		
		visProp.Element.WeaponRightHand = visualSlotManager->rightItem2Index(rightSheet);
		visProp.Element.WeaponLeftHand = visualSlotManager->leftItem2Index(leftSheet);
	}
	
	// setting up the visual property A mirror record	
	visProp.Element.ColorTop   = getSheet()->ColorBody();
	visProp.Element.ColorBot   = getSheet()->ColorLegs();
	visProp.Element.ColorHair  = getSheet()->ColorHead();
	visProp.Element.ColorGlove = getSheet()->ColorHands();
	visProp.Element.ColorBoot  = getSheet()->ColorFeets();
	visProp.Element.ColorArm   = getSheet()->ColorArms();
	visProp.Element.Hat = _Hat;
	visProp.Element.Seed = getAlias()!=0?getAlias():(uint32)(size_t)(void*)this;
	LOG("BOT: %s  L: %d  R: %u  H: %u  CHEAD: %u  CARMS: %u  CHANDS: %u CBODY: %u CLEGS: %u CFEETS: %u SEED: %u",
		getName().c_str(),
		visProp.Element.WeaponLeftHand,
		visProp.Element.WeaponRightHand,
		visProp.Element.Hat,
		visProp.Element.ColorTop,
		visProp.Element.ColorBot,
		visProp.Element.ColorHair,
		visProp.Element.ColorGlove,
		visProp.Element.ColorBoot,
		visProp.Element.ColorArm,
		visProp.Element.Seed);
	
	CMirrors::setVPA(getSpawn()->dataSetRow(), visProp);
}

void CBotNpc::sendVisualProperties()	// VisualPropertyA, B, C
{
	CMirrors::setVisualPropertyA( getSpawn()->dataSetRow(), _VisualPropertyA );
	CMirrors::setVisualPropertyB( getSpawn()->dataSetRow(), _VisualPropertyB );
	CMirrors::setVisualPropertyC( getSpawn()->dataSetRow(), _VisualPropertyC );
}

bool CBotNpc::reSpawn(bool sendMessage)
{
	if (!spawn())
		return false;
	
	getSpawn()->updateChat(grp().getCAIState());
	return true;
}

void CBotNpc::despawnBot()
{
	if (isSpawned())
	{
		getSpawn()->spawnGrp().botHaveDespawn(this);
		CBot::despawnBot();
	}
}

void CBotNpc::equipmentInit()					
{
	_Sheet->reset();
	_Hat = false;
	_useVisualProperties = false;	// default is to use alternate VPA
	_VisualPropertyA = 0;
	_VisualPropertyB = 0;
	_VisualPropertyC = 0;
	_FaunaBotUseBotName = false;
}

void CBotNpc::equipmentAdd(std::string const& input)
{
	// if string is empty just return without making a fuss
	if (input.empty())
		return;
	
	// split string into keyword and tail
	std::string keyword, tail;
	if (!AI_SHARE::stringToKeywordAndTail(input,keyword,tail))
	{
		nlwarning("Bot '%s'%s: failed to parse equipment text: '%s'",
			getAliasFullName().c_str(),
			getAliasString().c_str(),
			input.c_str());
		return;
	}
	
	// do something depending on keyword
	if (NLMISC::nlstricmp(keyword,"ri")==0)
	{
		if (tail.empty())
		{
			nlwarning("No sheet name supplied for equipment slot 'ri': '%s'%s",getAliasFullName().c_str(),getAliasString().c_str());
			_Sheet->setRightItem(NLMISC::CSheetId::Unknown);
		}
		else if (NLMISC::nlstricmp(tail,"none")==0)
		{
			_Sheet->setRightItem(NLMISC::CSheetId::Unknown);
		}
		else
		{
			_Sheet->setRightItem(NLMISC::CSheetId(tail));
		}
	}
	else if (NLMISC::nlstricmp(keyword,"li")==0)
	{
		if (tail.empty())
        {
            nlwarning("No sheet name supplied for equipment slot 'li': '%s'%s",getAliasFullName().c_str(),getAliasString().c_str());
			_Sheet->setLeftItem(NLMISC::CSheetId::Unknown);
        }
		else if (NLMISC::nlstricmp(tail,"none")==0)
		{
			_Sheet->setLeftItem(NLMISC::CSheetId::Unknown);
		}
        else
        {
			_Sheet->setLeftItem(NLMISC::CSheetId(tail));
		}
 	}
	else if (NLMISC::nlstricmp(keyword,"HAT")==0 || NLMISC::nlstricmp(keyword,"IH")==0)
	{
		_Hat = true;
	}
	else if (
		NLMISC::nlstricmp(keyword,"UPPER" )==0 || NLMISC::nlstricmp(keyword,"CU")==0 ||
		NLMISC::nlstricmp(keyword,"LOWER" )==0 || NLMISC::nlstricmp(keyword,"CL")==0 ||
		NLMISC::nlstricmp(keyword,"HAIR"  )==0 || NLMISC::nlstricmp(keyword,"CH")==0 ||
		NLMISC::nlstricmp(keyword,"CHEAD" )==0 ||
		NLMISC::nlstricmp(keyword,"CARMS" )==0 ||
		NLMISC::nlstricmp(keyword,"CHANDS")==0 ||
		NLMISC::nlstricmp(keyword,"CBODY" )==0 ||
		NLMISC::nlstricmp(keyword,"CLEGS" )==0 ||
		NLMISC::nlstricmp(keyword,"CFEETS")==0
		)
	{
		setColours(input);
	}
	else if(
		NLMISC::nlstricmp(keyword,"VPA")==0 || NLMISC::nlstricmp(keyword,"VPB")==0 ||
		NLMISC::nlstricmp(keyword,"VPC")==0
		)
	{

		setVisualProperties(input);
	}
	else if ( NLMISC::nlstricmp(keyword,"CLIENT_SHEET")==0  )
	{
		setClientSheet(tail + ".creature");
	}	
	else if (NLMISC::nlstricmp(keyword,"loot")==0)
	{
        if (tail.empty())
        {
            nlwarning("No sheet name supplied for loot list entry: '%s'%s",getAliasFullName().c_str(),getAliasString().c_str());
        }
        else
        {
			_LootList.push_back(NLMISC::CSheetId(tail));
        }
 	}
	else if (NLMISC::nlstricmp(keyword,"FAUNA_BOT_USE_BOTNAME")==0)
	{
		_FaunaBotUseBotName = true;
	}
	else if (NLMISC::nlstricmp(keyword, "USER_MODEL") == 0)
	{
		
		uint32 primAlias = getAlias() >> LigoConfig.getDynamicAliasSize();
		nldebug("Parsing userModelId '%s' with primAlias: '%u'", tail.c_str(), primAlias);
		setPrimAlias(primAlias);
		setUserModelId(tail);
	}
	else if (NLMISC::nlstricmp(keyword, "CUSTOM_LOOT_TABLE") == 0)
	{	
		uint32 primAlias = getAlias() >> LigoConfig.getDynamicAliasSize();
		nldebug("Parsing customLootTableId '%s' with primAlias: '%u'", tail.c_str(), primAlias);
		setPrimAlias(primAlias);
		setCustomLootTableId(tail);
	}
	else
	{
		nlwarning("Bot '%s'%s:failed to parse equipment argument: '%s'",
			getAliasFullName().c_str(),
			getAliasString().c_str(),
			input.c_str());
	}
}

/*
Colors are something like that:
	3D				INTERFACE			MP 
0:	ROUGE			ROUGE				RED 
1:	BEIGE			ORANGE				BEIGE 
2:	VERT CITRON		VERT CITRON			GREEN 
3:	VERT			VERT				TURQUOISE 
4:	BLEU			BLEU				BLUE 
5:	ROUGE dark		ROUGE (normal)		CRIMSON 
6:	BLANC			JAUNE				WHITE 
7:	NOIR			BLEU very dark		BLACK

3D column is (probably) used for equipment
*/
void CBotNpc::setColour(uint8 colour)
{
	_Sheet->setColorHead(colour);
	_Sheet->setColorArms(colour);
	_Sheet->setColorHands(colour);	
	_Sheet->setColorBody(colour);
	_Sheet->setColorLegs(colour);
	_Sheet->setColorFeets(colour);
}

void CBotNpc::setColours(std::string input)
{
	// stuff for manageing colour names
	int const numColours = 8;
	static std::vector <std::string> colourNames[numColours];
	static bool	init = false;
	
	// if string is empty just return without making a fuss
	if (input.empty())
		return;
	
	if (!init)
	{
		// lookup 'ColourNames' in config file (should be a multi-line field)
		NLMISC::CConfigFile::CVar* varPtr = NLNET::IService::getInstance()->ConfigFile.getVarPtr(std::string("ColourNames"));
		if (varPtr!=NULL)
		{
			// for each line in config file var try to add an alternative name for one of the colour slots
			for (uint i=0; i<varPtr->size(); ++i)
			{
				// split line into name and idx (line example: 'red: 5') 
				std::string name, idxStr;
				if (AI_SHARE::stringToKeywordAndTail(varPtr->asString(i),name,idxStr))
				{
					// split succeeded so verify that idxStr contains anumber in range 0..7 and add
					// the name to the list of valid names for the given slot
					uint32 idx;
					NLMISC::fromString(idxStr, idx);
					if (NLMISC::toString(idx)==idxStr && idx<numColours)
						colourNames[idx].push_back(name);
				}
			}
		}
		init = true;
	}

	// split 'input' string into keyword and tail
	std::string keyword, tail;
	if (!AI_SHARE::stringToKeywordAndTail(input,keyword,tail))
	{
		nlwarning("Failed to parse colour text: '%s' for bot: '%s'",input.c_str(),getAliasNode()->fullName().c_str());
		return;
	}

	// do something depending on keyword
	if (NLMISC::nlstricmp(keyword,"UPPER" )==0 || NLMISC::nlstricmp(keyword,"CU")==0 ||
		NLMISC::nlstricmp(keyword,"LOWER" )==0 || NLMISC::nlstricmp(keyword,"CL")==0 ||
		NLMISC::nlstricmp(keyword,"HAIR"  )==0 || NLMISC::nlstricmp(keyword,"CH")==0 ||
		NLMISC::nlstricmp(keyword,"CHEAD" )==0 ||
		NLMISC::nlstricmp(keyword,"CARMS" )==0 ||
		NLMISC::nlstricmp(keyword,"CHANDS")==0 ||
		NLMISC::nlstricmp(keyword,"CBODY" )==0 ||
		NLMISC::nlstricmp(keyword,"CLEGS" )==0 ||
		NLMISC::nlstricmp(keyword,"CFEETS")==0
		)
	{
		std::vector <uint32> results;
		std::string colour;
		while (!tail.empty())
		{
			// extract the next word from the tail
			AI_SHARE::stringToWordAndTail(tail,colour,tail);
			// if the colour string is a number then treat it directly
			uint32 idx;
			NLMISC::fromString(colour, idx);
			if (NLMISC::toString(idx)==colour)
			{
				// we've got a number so make sue it's in valid range and add to results vector
				if (idx<numColours)
					results.push_back(idx);
				else
					nlwarning("Bot '%s'%s: failed to identify colour: '%s' in line: '%s'",
						getAliasFullName().c_str(),
						getAliasString().c_str(),
						colour.c_str(),
						input.c_str());
			}
			else
			{
				// try to find an entry in the colour list that matches the 'colour' string
				bool done=false;
				size_t i;
				for (i=0; i<numColours && !done; ++i)
				{
					for (size_t j=0; j<colourNames[i].size() && !done; ++j)
					{
						if (NLMISC::nlstricmp(colour,colourNames[i][j])==0)
						{
							// found an entry so add to the results vector
							results.push_back((uint32)i);
							done=true;
						}
					}
				}
				
				if (i==numColours)
					nlwarning("Failed to identify colour: '%s' in line: '%s'",colour.c_str(),input.c_str());
			}
		}
		
		// assuming that we found more than	0 results pick one at random
		if (results.empty())
		{
			nlwarning("Bot '%s'%s: failed to identify any valid input in line: '%s'",
				getAliasFullName().c_str(),
				getAliasString().c_str(),
				input.c_str());
			return;
		}
		NLMISC::CRandom generator;
 		sint32 seed = getAlias();
 		uint8* p = (uint8*)&seed;
 		p[1] ^= p[0];
 		p[2] ^= p[0];
 		p[3] ^= p[0];
		if ( NLMISC::nlstricmp(keyword,"UPPER")==0 || NLMISC::nlstricmp(keyword,"CU")==0 )
		{
			// upper body colour
 			generator.srand(seed+975*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorArms(val);
			_Sheet->setColorHands(val);
			_Sheet->setColorBody(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"LOWER")==0 || NLMISC::nlstricmp(keyword,"CL")==0 )
		{
			// lower body colour
 			generator.srand(seed+977*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorLegs(val);
			_Sheet->setColorFeets(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"HAIR")==0 || NLMISC::nlstricmp(keyword,"CH")==0 )
		{
			// hair colour, mapped to head
 			generator.srand(seed+976*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorHead(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"CHEAD")==0)
		{
			// head color
 			generator.srand(seed+979*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorHead(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"CARMS")==0)
		{
			// arms color
 			generator.srand(seed+981*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorArms(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"CHANDS")==0)
		{
			// arms color
 			generator.srand(seed+983*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorHands(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"CBODY")==0)
		{
			// arms color
 			generator.srand(seed+985*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorBody(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"CLEGS")==0)
		{
			// arms color
 			generator.srand(seed+987*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorLegs(val);
		}
		else if ( NLMISC::nlstricmp(keyword,"CFEETS")==0)
		{
			// arms color
 			generator.srand(seed+989*0x10000);
			uint8 val = (uint8)results[generator.rand((uint16)results.size()-1)];
			_Sheet->setColorFeets(val);
		}
	}
	else
	{
		nlwarning("Bot '%s'%s: failed to parse colours argument: '%s'",
			getAliasFullName().c_str(),
			getAliasString().c_str(),
			input.c_str());
	}
}



void CBotNpc::setVisualProperties(std::string input)	// AJM
{
	// set the VisualPropertyA, VisualPropertyB, VisualPropertyC bitfield values
	//	using a hexadecimal entry format

	// if string is empty just return without making a fuss
	if(input.empty())
		return;

	// split 'input' string into keyword and tail
	std::string keyword, tail;
	if(!AI_SHARE::stringToKeywordAndTail(input,keyword,tail))
	{
		nlwarning("Failed to parse visual property text: '%s' for bot: '%s'",input.c_str(),getAliasNode()->fullName().c_str());
		return;
	}

	// load val from tail
	// accept 64bit hex value
	uint64 val;
	sscanf( tail.c_str(), "%"NL_I64"x", &val );

	// can't set into mirror row until bot is spawned, so save away
	if( NLMISC::nlstricmp( keyword,"VPA")==0 )	// VisualPropertyA
	{
		_VisualPropertyA = val;
		_useVisualProperties = true;
	}
	else if( NLMISC::nlstricmp( keyword,"VPB")==0 )	// VisualPropertyB
	{
		_VisualPropertyB = val;
		_useVisualProperties = true;
	}
	else if( NLMISC::nlstricmp( keyword,"VPC")==0 )	// VisualPropertyC
	{
		_VisualPropertyC = val;
		_useVisualProperties = true;
	}

	else
	{
		nlwarning("Bot '%s'%s: failed to parse visual property argument: '%s'",
			getAliasFullName().c_str(),
			getAliasString().c_str(),
			input.c_str());
	}
}

void CBotNpc::setStartPos(double x, double y, float theta, AITYPES::TVerticalPos verticalPos)
{
	_StartPos = CAIPos(x, y, 0, theta);
	_VerticalPos = verticalPos;
}

void CBotNpc::init()
{
	_ChatProfile = NULL;
	_MaxHitRangeForPC = -1.0f;
//	_MissionIconFlags.IsMissionStepIconDisplayable = true;
//	_MissionIconFlags.IsMissionGiverIconDisplayable = true;
	
	equipmentInit();
}

CSpawnBotNpc* CBotNpc::getSpawn()
{
	return static_cast<CSpawnBotNpc*>(getSpawnObj());
}

CSpawnBotNpc const* CBotNpc::getSpawn() const
{
	return static_cast<CSpawnBotNpc const*>(getSpawnObj());
}

void CBotNpc::getSpawnPos(CAIVector& triedPos, RYAI_MAP_CRUNCH::CWorldPosition& pos, RYAI_MAP_CRUNCH::CWorldMap const& worldMap, CAngle& spawnTheta)
{
	if (_StartPos.isNull())
		calcSpawnPos(worldMap);
	
	if (isStuck() || IsRingShard)
		worldMap.setWorldPosition(_VerticalPos, pos, _StartPos);
	else
		CWorldContainer::calcNearestWPosFromPosAnRadius(_VerticalPos, worldMap, pos, _StartPos, 10, 200, CWorldContainer::CPosValidatorDefault());
	
#ifdef NL_DEBUG
	if (!pos.isValid() && !isStuck())
	{
		nlwarning("Npc Spawn Pos Error %s", pos.toString().c_str());
	}
#endif
	
	spawnTheta = _StartPos.theta();
	triedPos = _StartPos;
}

CSpawnBot* CBotNpc::getSpawnBot(TDataSetRow const& row, NLMISC::CEntityId const& id, float radius)
{
	return new CSpawnBotNpc(row, *this, id, radius, getSheet()->Level(), getGroup().getAStarFlag());
}

void CBotNpc::newChat()
{
	_ChatProfile = new CNpcChatProfileImp();
}

CAIS::CCounter& CBotNpc::getSpawnCounter()
{
	return CAIS::instance()._NpcBotCounter;
}

void CBotNpc::setSheet(AISHEETS::ICreatureCPtr const& sheet)
{
	_Sheet->setSheet(sheet);
	sheetChanged();
}

// :KLUDGE: This method is very tricky. It's a copy'n'paste of the method in CBot (BAD!) with more content
// :TODO: Clean that mess
void CBotNpc::sheetChanged()
{
	if (getSpawnObj())
	{
		// Get bot state
		RYAI_MAP_CRUNCH::CWorldPosition botWPos = getSpawnObj()->wpos();
		CAngle spawnTheta = getSpawnObj()->theta();
		float botMeterSize = getSheet()->Scale()*getSheet()->Radius();
		// :TODO: Save profile info
		
		// If stuck bot position may be outside collision and must be recomputed
		if (isStuck() || IsRingShard)
			getSpawnPos(lastTriedPos, botWPos, CWorldContainer::getWorldMap(), spawnTheta);
		
		// Delete old bot
		CMirrors::removeEntity(getSpawnObj()->getEntityId());
		setSpawn(NULL); // automatic smart pointer deletion
		notifyBotDespawn();
		
		// Finalize spawn object creation
		if (!finalizeSpawn(botWPos, spawnTheta, botMeterSize))
			return;
		
		// :KLUDGE: Both finalizeSpawn and finalizeSpawnNpc are called,
		// sheetChanged has a strange herited meaning and may confuse future
		// coders
		// :TODO: Clean that mess and find a more elegant C++ solution to the
		// problem
		finalizeSpawnNpc();
	}
}

//////////////////////////////////////////////////////////////////////////////
// Commmands                                                                //
//////////////////////////////////////////////////////////////////////////////

// Control over verbose nature of logging
NLMISC_COMMAND(verboseNPCBotProfiles, "Turn on or off or check the state of verbose bot profile change logging","")
{
	if (args.size()>1)
		return false;
	
	if (args.size()==1)
		NLMISC::fromString(args[0], VerboseLog);
	
	nlinfo("VerboseLogging is %s",VerboseLog?"ON":"OFF");
	return	true;
}
// virtual function so do not need to be inlined
bool CBotNpc::getFaunaBotUseBotName() const
{ 
	return _FaunaBotUseBotName; 
}

