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
#include "ai_bot.h"
#include "visual_properties_interface.h"
#include "server_share/msg_brick_service.h"
#include "server_share/r2_variables.h"

using namespace MULTI_LINE_FORMATER;

extern NLLIGO::CLigoConfig LigoConfig;

//////////////////////////////////////////////////////////////////////////////
// CSpawnBot                                                                //
//////////////////////////////////////////////////////////////////////////////

CSpawnBot::CSpawnBot(TDataSetRow const& entityIndex, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32	level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CModEntityPhysical(owner, entityIndex, id, radius, level, denyFlags)
, _SpawnGroup(owner.getOwner()->getSpawnObj())
, CProfileOwner()
, CDynSpawnBot(owner)
, _DamageSpeedCoef(1.f)
, _DamageCoef(1.f)
, _LastHealTick(0)
, _LastSelfHealTick(0)
, _SpeedFactor(1.f)
{
	nlassert(owner.getOwner()->getSpawnObj());
	spawnGrp().incSpawnedBot(getPersistent());
	getPersistent().getSpawnCounter().inc();
	setInstanceNumber(getAIInstance()->getInstanceNumber());
}

CSpawnBot::~CSpawnBot()
{
	clearAggroList();
	getPersistent().unlinkFromWorldMap();
	spawnGrp().decSpawnedBot();
	getPersistent().getSpawnCounter().dec();
}

CAIInstance* CSpawnBot::getAIInstance() const
{
	return getPersistent().getAIInstance();
}

void CSpawnBot::setVisualPropertiesName()
{
	CBot& botRef = CSpawnBot::getPersistent();
	ucstring name = botRef.getName();
	
	if (CVisualPropertiesInterface::UseIdForName)
	{
		name = NLMISC::toString("AI:%s", botRef.getIndexString().c_str());
	}
	
	if (name.empty() && CVisualPropertiesInterface::ForceNames)
	{
		name = NLMISC::CFile::getFilenameWithoutExtension(botRef.getSheet()->SheetId().toString().c_str());
	}
	
	if (!botRef.getCustomName().empty())
		name = botRef.getCustomName();

	//	no name the bot will appear without name on the client.
	if (name.empty())
		return;
	
	
	// In ringshard we use npc with fauna sheet but we want to be enable to change theire name
	if (! botRef.getFaunaBotUseBotName()) //false by default
	{
		if (botRef.getSheet()->ForceDisplayCreatureName())
			return;
		// the npc name is displayed as a fauna
	}

	
	CVisualPropertiesInterface::setName(dataSetRow(), name);
}

float CSpawnBot::fightWeight() const
{
	if (getPersistent().isSheetValid())
	{
		AISHEETS::ICreatureCPtr sheet = getPersistent().getSheet();
		if (!sheet->FightConfig(AISHEETS::FIGHTCFG_MELEE).isNULL())
		{
			// Melee fighters use a lot of space
			return .98f;
		}
		else
		{
			if (!sheet->FightConfig(AISHEETS::FIGHTCFG_NUKE).isNULL())
				return .01f;
			if (!sheet->FightConfig(AISHEETS::FIGHTCFG_RANGE).isNULL())
				return .01f;
			// :FIXME: We consider healers are melee, coz they can't heal for the moment and use default attack.
			if (!sheet->FightConfig(AISHEETS::FIGHTCFG_HEAL).isNULL())
				return .98f;
			// All others are like melee (default attack)
			return .98f;
		}
	}
	return this->CAIEntityPhysical::fightValue();
}

float CSpawnBot::fightValue() const
{
	if (getPersistent().isSheetValid())
	{
		AISHEETS::ICreatureCPtr sheet = getPersistent().getSheet();
		return (float)(sheet->XPLevel() * sheet->NbPlayers());
	}
	return this->CAIEntityPhysical::fightValue();
}

CSpawnGroup& CSpawnBot::spawnGrp() const
{
#if !FINAL_VERSION
	nlassert(NLMISC::safe_cast<CSpawnGroup*>(getPersistent().getGroup().getSpawnObj()) == _SpawnGroup);
#endif
	return *_SpawnGroup;
}

std::vector<std::string> CSpawnBot::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CSpawnBot");
	pushEntry(container, "dataSetRow=" + NLMISC::toString("%x", dataSetRow().counter()) + ":" + NLMISC::toString("%X", dataSetRow().getIndex()));
	container.back() += " pos/rot=" + pos().toString();
	if ((CAIEntityPhysical*)getTarget())
		container.back() += NLMISC::toString(" target=%s", getTarget()->getEntityId().toString().c_str());
	if (isBlinded())
		container.back() += NLMISC::toString(" blinded");
	if (isRooted())
		container.back() += NLMISC::toString(" rooted");
	if (isStuned())
		container.back() += NLMISC::toString(" stuned");
	if (isFeared())
		container.back() += NLMISC::toString(" feared");
	pushEntry(container, "outpost: ");
	container.back() += " alias=" + LigoConfig.aliasToString(outpostAlias());
	container.back() += " side=";
	container.back() += outpostSide()?"attacker":"defender";
	pushEntry(container, "haveAggro=" + NLMISC::toString(haveAggro()));
	container.back() += " isReturning=" + NLMISC::toString(isReturning());
	pushFooter(container);
	
	
	return container;
}

NLMISC::CSmartPtr<CAIPlace const> CSpawnBot::buildFirstHitPlace(TDataSetRow const& aggroBot) const
{
	NLMISC::CSmartPtr<CAIPlace const> thisPlace, groupPlace;
	{
		NLMISC::CSmartPtr<CAIPlaceFastXYR> place = NLMISC::CSmartPtr<CAIPlaceFastXYR>(new CAIPlaceFastXYR(NULL));
		place->setPosAndRadius(AITYPES::vp_auto, pos(), (uint32)(getD1Radius()*1000.f));
		thisPlace = place;
	}
	groupPlace = getPersistent().getOwner()->getSpawnObj()->buildFirstHitPlace(aggroBot);
	if (groupPlace)
	{
		NLMISC::CSmartPtr<CAIPlaceIntersect> place = NLMISC::CSmartPtr<CAIPlaceIntersect>(new CAIPlaceIntersect(NULL));
		place->setPlace1(thisPlace);
		place->setPlace2(groupPlace);
		thisPlace = place;
	}
	return thisPlace;
}

std::set<CBotAggroOwner*> CSpawnBot::getAggroGroup(bool primary) const
{
	if (primary)
	{
		if (getPrimaryGroupAggroDist()>0.f)
			return std::set<CBotAggroOwner*>(); /// @TODO Fill this
		else
			return std::set<CBotAggroOwner*>();
	}
	else
	{
		if (getSecondaryGroupAggroDist()>0.f)
			return std::set<CBotAggroOwner*>(); /// @TODO Fill this
		else
			return std::set<CBotAggroOwner*>();
	}
}

float CSpawnBot::getReturnDistCheck() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroReturnDistCheck()>=0.f)
		return getPersistent().getSheet()->AggroReturnDistCheck();
	else
		return CBotAggroOwner::getReturnDistCheck();
}

float CSpawnBot::getD1Radius() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroRadiusD1()>=0.f)
		return getPersistent().getSheet()->AggroRadiusD1();
	else
		return CBotAggroOwner::getD1Radius();
}

float CSpawnBot::getD2Radius() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroRadiusD2()>=0.f)
		return getPersistent().getSheet()->AggroRadiusD2();
	else
		return CBotAggroOwner::getD2Radius();
}

float CSpawnBot::getPrimaryGroupAggroDist() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroPrimaryGroupDist()>=0.f)
		return getPersistent().getSheet()->AggroPrimaryGroupDist();
	else
		return CBotAggroOwner::getPrimaryGroupAggroDist();
}

float CSpawnBot::getPrimaryGroupAggroCoef() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroPrimaryGroupCoef()>=0.f)
		return getPersistent().getSheet()->AggroPrimaryGroupCoef();
	else
		return CBotAggroOwner::getPrimaryGroupAggroCoef();
}

float CSpawnBot::getSecondaryGroupAggroDist() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroSecondaryGroupDist()>=0.f)
		return getPersistent().getSheet()->AggroSecondaryGroupDist();
	else
		return CBotAggroOwner::getSecondaryGroupAggroDist();
}

float CSpawnBot::getSecondaryGroupAggroCoef() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroSecondaryGroupCoef()>=0.f)
		return getPersistent().getSheet()->AggroSecondaryGroupCoef();
	else
		return CBotAggroOwner::getSecondaryGroupAggroCoef();
}

float CSpawnBot::getAggroPropagationRadius() const
{
	if (getPersistent().isSheetValid() && getPersistent().getSheet()->AggroPropagationRadius()>=0.f)
		return getPersistent().getSheet()->AggroPropagationRadius();
	else
		return CBotAggroOwner::getAggroPropagationRadius();
}

bool CSpawnBot::canHeal()
{
	if (!getPersistent().getSheet()->FightConfig(AISHEETS::FIGHTCFG_HEAL)->_HasNormalAction)
		return false;
	if (_LastHealTick==0)
		return true;
	else
		return (CTimeInterface::gameCycle() - _LastHealTick) > HealSpecificDowntime;
}

bool CSpawnBot::canSelfHeal()
{
	if (!getPersistent().getSheet()->FightConfig(AISHEETS::FIGHTCFG_HEAL)->_HasSelfAction)
		return false;
	if (_LastSelfHealTick==0)
		return true;
	else
		return (CTimeInterface::gameCycle() - _LastSelfHealTick) > HealSpecificDowntimeSelf;
}

void CSpawnBot::healTriggered()
{
	_LastHealTick = CTimeInterface::gameCycle();
}

void CSpawnBot::selfHealTriggered()
{
	_LastSelfHealTick = CTimeInterface::gameCycle();
}

void CSpawnBot::aggroLost(TDataSetRow const& aggroBot) const
{
	sAggroLost(aggroBot, dataSetRow());
}

void CSpawnBot::aggroGain(TDataSetRow const& aggroBot) const
{
	sAggroGain(aggroBot, dataSetRow());
}

bool CSpawnBot::getProp(size_t Id, uint32& value) const
{
	TPropList::const_iterator it = _PropList.find(Id);
	if (it==_PropList.end())
		return	false;
	
	value = it->second;
	return true;
}

void CSpawnBot::setProp(size_t Id, uint32 value)
{
	_PropList[Id] = value;
}

CBot& CSpawnBot::getPersistent() const
{
	return static_cast<CBot&>(CSpawnable<CPersistentOfPhysical>::getPersistent());
}

void CSpawnBot::setTheta(CAngle theta)
{
	if (getPersistent().getSheet()->CanTurn())
		CModEntityPhysical::setTheta(theta);
}

// :KLUDGE: This method change the sheet of the bot in the mirror. It appear
// that the client is totally unable to handle a sheet change during an entity
// lifetime. So this method should not be called.
/// @TODO Remove the method from spawn classes since its now longer called
// when sheet changes, and since spawn object is in fact destroyed and
// recreated.
void CSpawnBot::sheetChanged()
{
//	CMirrors::initSheet(dataSetRow(), getPersistent().getSheet()->SheetId());
}

void CSpawnBot::sendInfoToEGS() const
{
	if (!EGSHasMirrorReady)
		return;

	const uint32& maxHp = getPersistent().getCustomMaxHp();
	if (maxHp > 0.f)
	{
		CChangeCreatureMaxHPMsg& msgList = CAIS::instance().getCreatureChangeMaxHP();
		
		msgList.Entities.push_back(dataSetRow());
		msgList.MaxHp.push_back((uint32)(maxHp));
		msgList.SetFull.push_back((uint8)(1));
	}
}


//////////////////////////////////////////////////////////////////////////////
// CBot                                                                     //
//////////////////////////////////////////////////////////////////////////////

CBot::CBot(CGroup* owner, CAIAliasDescriptionNode* alias)
: CAliasChild<CGroup>(owner, alias)
, _VerticalPos(AITYPES::vp_auto)
, _Sheet(NULL)
, _Stuck(false)
, _IgnoreOffensiveActions(false)
, _Healer(false)
, _SetSheetData(NULL)
, _Observers(NULL)
, _ProfileData(NULL)
, _CustomMaxHp(0)
{
}

CBot::CBot(CGroup* owner, uint32 alias, std::string const& name)
: CAliasChild<CGroup>(owner,alias, name)
, _VerticalPos(AITYPES::vp_auto)
, _Sheet(NULL)
, _Stuck(false)
, _IgnoreOffensiveActions(false)
, _Healer(false)
, _SetSheetData(NULL)
, _Observers(NULL)
, _ProfileData(NULL)
, _CustomMaxHp(0.f)
{
}

CBot::~CBot()
{
	// despawn before calling this destructor, because it may will generate a pure virtual call error!
#if !FINAL_VERSION
	nlassert(!isSpawned());			
#endif

	if (_Observers != NULL)
	{
		notifyBotDespawn();
		delete _Observers;
	}
}

std::string	CBot::getOneLineInfoString() const
{
	return std::string("Bot '") + getName() + "'";
}

std::vector<std::string> CBot::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	std::vector<std::string> strings;
	
	
	pushTitle(container, "CBot");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " eid=" + getEntityIdString();
	container.back() += " alias=" + getAliasTreeOwner()->getAliasString() + " raw alias=" + NLMISC::toString(getAliasTreeOwner()->getAlias());
	pushEntry(container, " name=" + getName());
	if (isSheetValid())
		container.back() += " sheet=" + NLMISC::CFile::getFilenameWithoutExtension(getSheet()->SheetId().toString());
	pushEntry(container, "fullname=" + getFullName());
	if (isSheetValid())
	{
		strings = getSheet()->getMultiLineInfoString();
		FOREACHC(it, std::vector<std::string>, strings)
			pushEntry(container, *it);
	}
	else
		pushEntry(container, "<invalid sheet>");
	if (isSpawned())
	{
		strings = getSpawnObj()->getMultiLineInfoString();
		FOREACHC(it, std::vector<std::string>, strings)
			pushEntry(container, *it);
	}
	else
		pushEntry(container, "<not spawned>");
	pushFooter(container);
	
	
	return container;
}

std::string CBot::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":b%u", getChildIndex());
}

std::string CBot::getEntityIdString() const
{
	if (isSpawned())
		return getSpawnObj()->getEntityId().toString() ;
	else
		return NLMISC::CEntityId().toString();
}

CAIInstance* CBot::getAIInstance() const
{
	return getOwner()->getAIInstance();
}


void CBot::addEnergy() const
{
	getOwner()->getOwner()->getOwner()->addEnergy(botEnergyValue());
}

void CBot::removeEnergy() const
{
	getOwner()->getOwner()->getOwner()->removeEnergy(botEnergyValue());
}

std::string	CBot::getFullName()	const
{
	return std::string(getOwner()->getFullName() +":"+ getName());
}	

NLMISC::CEntityId CBot::createEntityId() const
{
	// motif is always 0 as we use automatic id assignement from mirror
	uint64	motif = 0;
	
	//	fake, just waiting real pets implementation in mirror.
	RYZOMID::TTypeId	botType=getRyzomType();
	if (botType==RYZOMID::pack_animal)
		botType=RYZOMID::creature;
	
#ifndef NL_DEBUG
	nlassert(botType!=RYZOMID::player);
#endif
	return NLMISC::CEntityId(botType,motif);
}

bool CBot::isHealer() const
{
	return _Healer && isSheetValid() && !getSheet()->FightConfig(AISHEETS::FIGHTCFG_HEAL).isNULL();
}

void CBot::initEnergy(float energyCoef)
{
	if (isSheetValid())
		setBotEnergyValue((uint32)((double)energyCoef*(double)getSheet()->EnergyValue()));
}

void CBot::serviceEvent(CServiceEvent const& info)
{
	if (!isSpawned())
		return;
	
	CSpawnBot* spawnBot = getSpawnObj();
	
	if (info.getEventType()==CServiceEvent::SERVICE_UP)
	{
		if (info.getServiceName()=="IOS")
		{
			spawnBot->setVisualPropertiesName();
		}
		else if (info.getServiceName()=="EGS")
		{
			spawnBot->sendInfoToEGS();
			spawnBot->removeActionFlags(RYZOMACTIONFLAGS::Attacks);	//	clear Action Flags to avoid stuck problems with EGS.
			spawnBot->stun()=0;
			spawnBot->blind()=0;
			spawnBot->root()=0;
		}
	}
}

CSpawnBot* CBot::getSpawnObj() const
{
	return static_cast<CSpawnBot*>(CPersistentOfPhysical::getSpawnObj());
}

// :KLUDGE: This method is was created to avoid duplication of code. It
// results that it doesn't have a meaning without prior code to be executed
// (see sheetChanged and spawn). It's a trick that should be removed since
// there is code duplication anyway in sheetChanged method and its overrides.
/// @TODO Clean that mess
bool CBot::finalizeSpawn(RYAI_MAP_CRUNCH::CWorldPosition const& botWPos, CAngle const& spawnTheta, float botMeterSize)
{
	// Create eid
	NLMISC::CEntityId eid = createEntityId();
	// Create row
	TDataSetRow	const row = CMirrors::createEntity(eid);
	if (!row.isValid())
	{
		nlwarning("***> Not Enough Mirror Space for Type: %s", RYZOMID::toString(getRyzomType()).c_str());
		return false;
	}

	if (_ClientSheet!=NLMISC::CSheetId::Unknown)
		CMirrors::initSheet(row, _ClientSheet);
	else
		CMirrors::initSheet(row, getSheet()->SheetId());
	
	// get the spawn of this persistent objet.
	setSpawn(getSpawnBot(row, eid, botMeterSize));
	
	CSpawnBot* spawnBot = getSpawnObj();
	nlassert(spawnBot);
	
	spawnBot->setVisualPropertiesName();
	
	CAIPos botPos;
	if (!isStuck() && !IsRingShard)
		botPos = CAIPos(botWPos.toAIVector(),0,0);
	else
		botPos = CAIPos(lastTriedPos, 0, 0);
	spawnBot->setPos(botPos, botWPos);
	// Use base class method to avoid overload in 
	spawnBot->CModEntityPhysical::setTheta(spawnTheta);
	
	this->initAdditionalMirrorValues(); // let derived class do its additional inits before declaring the entity
	CMirrors::declareEntity(row);
	linkToWorldMap(this, spawnBot->pos(), getAIInstance()->botMatrix());
	
	return true;
}

bool CBot::spawn()
{
	nlassert(!isSpawned());
	nlassert(getChildIndex()!=-1);	//	significates that we spawn an unattached bot.
	
	// Check we can spawn
	if (isSpawned())
		return true;
	if (getSheet()->SheetId() == NLMISC::CSheetId::Unknown)
	{
		nlwarning("Bot '%s'%s: invalid sheet id", getAliasFullName().c_str(), getAliasString().c_str());
		return false;
	}
	if (!getSpawnCounter().remainToMax())
		return false;
	
	// Get initial state
	RYAI_MAP_CRUNCH::CWorldPosition botWPos;
	CAngle spawnTheta;
	getSpawnPos(lastTriedPos, botWPos, CWorldContainer::getWorldMap(), spawnTheta);
	float botMeterSize = getSheet()->Scale()*getSheet()->Radius();
	
	// Check the initial position is valid
	if (!botWPos.isValid() && !isStuck())
	{
		nlwarning("Bot '%s'%s: invalid spawn pos and not stuck", getAliasFullName().c_str(), getAliasString().c_str());
		return false;
	}
	
	// Finalize spawn object creation
	return finalizeSpawn(botWPos, spawnTheta, botMeterSize);		
}

void CBot::despawnBot()
{
#if !FINAL_VERSION
	nlassert(isSpawned());
#endif
	if (!isSpawned())
		return;
	
	CMirrors::removeEntity(getSpawnObj()->getEntityId());
	setSpawn(NULL); // automatic smart pointer deletion
	notifyBotDespawn();
}

bool CBot::reSpawn(bool sendMessage)
{
	return spawn();
}

/// @KLUDGE This method is overridden in npc and fauna classes in a strange
/// way. The initial code is copied'n'pasted, and the finalizeSpawn method is
/// called along with a more specific one. This may confuse future coders and
/// be hard to maintain.
/// @TODO Clean that mess
void CBot::sheetChanged()
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
		finalizeSpawn(botWPos, spawnTheta, botMeterSize);
	}
}

class CSetSheetTimerEvent
: public CTimerEvent
{
	NLMISC::CSmartPtr<CBot> _Bot;
	uint32 _Step;
public:
	CSetSheetTimerEvent(CBot* bot, uint32 step) : _Bot(bot), _Step(step) { }
	virtual void timerCallback(CTimer* owner)
	{
		if (!_Bot.isNull())
			_Bot->setSheetDelayed(_Step);
	}
};

void CBot::setSheet(AISHEETS::ICreatureCPtr const& sheet)
{
	_Sheet = sheet;
	sheetChanged();
}

void CBot::setClientSheet(const std::string & clientSheetName)
{
	// Message warning is print if clientSheetName is not in sheet id
	if (!clientSheetName.empty())
	{
		if (!_ClientSheet.buildSheetId(clientSheetName))
		{
			nlwarning("Invalid CLIENT_SHEET %s", clientSheetName.c_str());
			return;
		}
		sheetChanged();
	}
}

// 1: Stop the bot
void CBot::triggerSetSheet(AISHEETS::ICreatureCPtr const& sheet)
{
	if (_SetSheetData)
	{
		nlwarning("Another sheet change is going, this one is canceled");
		return;
	}
	
	NLMISC::CSheetId sheetId= (BotRepopFx.get().empty()? NLMISC::CSheetId::Unknown: NLMISC::CSheetId(BotRepopFx));
	if (getSpawnObj() && sheetId!=NLMISC::CSheetId::Unknown)
	{
		_SetSheetData = new CSetSheetData();
		_SetSheetData->_FxSheetId = sheetId;
		_SetSheetData->_SheetToSet = sheet;
		// Timer 1 is to let time for the bot to stop moving
		_SetSheetTimer.setRemaining(12, new CSetSheetTimerEvent(this, 0));
		
		getSpawnObj()->setSpeedFactor(0.f);
	}
	else
	{
		setSheet(sheet);
	}
}

// 2: Trigger Fx
void CBot::setSheetDelayed0()
{
	nlassert(_SetSheetData);
	// Timer 2 is to let time for the fx to hide the bot despawn
	_SetSheetTimer.setRemaining(8, new CSetSheetTimerEvent(this, 1));

	
	_SetSheetData->_Fx = CFxEntityManager::getInstance()->create(getSpawnObj()->pos(), _SetSheetData->_FxSheetId);
	if (!_SetSheetData->_Fx->spawn())
	{
		nlwarning("Unable to spawn fx entity (mirror range full?)");
		CFxEntityManager::getInstance()->destroy(_SetSheetData->_Fx);
		_SetSheetData->_Fx = NULL;
	}
}

// 3: Change the sheet (depop/repop)
void CBot::setSheetDelayed1()
{
	nlassert(_SetSheetData);
	// Timer 3 is to let time for the fx to hide the bot spawn
	_SetSheetTimer.setRemaining(15, new CSetSheetTimerEvent(this, 2));
	
	setSheet(_SetSheetData->_SheetToSet);
	_SetSheetData->_SheetToSet = NULL;
}

// 4: Stop the Fx
void CBot::setSheetDelayed2()
{
	nlassert(_SetSheetData);
	if (!_SetSheetData->_Fx.isNull())
	{
		_SetSheetData->_Fx->despawn();
		CFxEntityManager::getInstance()->destroy(_SetSheetData->_Fx);
	}
	delete _SetSheetData;
	_SetSheetData = NULL;
}

void CBot::setSheetDelayed(uint32 step)
{
	switch (step)
	{
	case 0: setSheetDelayed0(); break;
	case 1: setSheetDelayed1(); break;
	case 2: setSheetDelayed2(); break;
	default: nlerror("setSheetDelayed called with an invalid step number");
	}
}

void CBot::attachObserver(IObserver* obs)
{
	if (_Observers == NULL)
	{
		_Observers = new std::vector<IObserver*>;
		_Observers->push_back(obs);
		return;
	}

	std::vector<IObserver*>::const_iterator it = std::find(_Observers->begin(), _Observers->end(), obs);
	if (it == _Observers->end())
		_Observers->push_back(obs);
}

void CBot::detachObserver(IObserver* obs)
{
	if (_Observers == NULL)
		return;

	std::vector<IObserver*>::iterator it = std::find(_Observers->begin(), _Observers->end(), obs);
	if (it != _Observers->end())
	{
		*it = _Observers->back();
		_Observers->pop_back();
	}
}

void CBot::notifyBotDespawn()
{
	if (_Observers == NULL)
		return;

	FOREACH(it, std::vector<IObserver*>, (*_Observers))
		(*it)->notifyBotDespawn(this);
}

void CBot::notifyBotDeath()
{
	if (_Observers == NULL)
		return;

	FOREACH(it, std::vector<IObserver*>, (*_Observers))
		(*it)->notifyBotDeath(this);
}

void CBot::notifyStopNpcControl()
{
	if (_Observers == NULL)
		return;

	FOREACH(it, std::vector<IObserver*>, (*_Observers))
		(*it)->notifyStopNpcControl(this);
}
