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

// Net
#include "nel/net/service.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"

#include "sheets.h"

#include "nel/misc/o_xml.h"

using namespace MULTI_LINE_FORMATER;

///////////
// USING //
///////////
using namespace NLGEORGES;
using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace	AITYPES;

//////////////////////////////////////////////////////////////////////////////
// Constants                                                                //
//////////////////////////////////////////////////////////////////////////////

#ifdef NL_DEBUG
	CVariable<string> debugSheet("ai", "debugSheet", "The sheet to break onto", "", 0, true);
#endif

char const* AISPackedSheetsFilename="ais.packed_sheets";
char const* AISPackedFightConfigSheetsFilename="ais_fight_config.packed_sheets";
char const* AISPackedActionSheetsFilename="ais_action.packed_sheets";
char const* AISPackedRaceStatsSheetsFilename="ais_race_stats.packed_sheets";

static AISHEETS::CCreature EmptySheet;

sint32 AISHEETS::ICreature::InvalidFameForGuardAttack = 0x7FFFFFFF;


//////////////////////////////////////////////////////////////////////////////
// CAIAction                                                                //
//////////////////////////////////////////////////////////////////////////////

AISHEETS::CAIAction::CAIAction()
: _SelfAction(false)
{
}

void AISHEETS::CAIAction::readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId)
{			
	NLGEORGES::UFormElm const& item = form->getRootNode();
	// the form was found so read the true values from George
	_SheetId = sheetId;
	item.getValueByName(_SelfAction, "SelfAction");
}

uint AISHEETS::CAIAction::getVersion()
{
	return 2;
}

void AISHEETS::CAIAction::serial(NLMISC::IStream& s)
{
	s.serial(_SheetId);
	s.serial(_SelfAction);
}

std::vector<std::string> AISHEETS::CAIAction::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	pushTitle(container, "AISHEETS::CAIAction");
	pushEntry(container, "ai_action sheet");
	pushFooter(container);
	
	return container;
}

//////////////////////////////////////////////////////////////////////////////
// CActionList                                                              //
//////////////////////////////////////////////////////////////////////////////

void AISHEETS::CActionList::computeAbilities()
{
	_HasNormalAction = false;
	_HasSelfAction = false;
	FOREACH(itAction, std::vector<IAIActionCPtr>, _Actions)
	{
		IAIActionCPtr const& action = *itAction;
		if (!action.isNull() && action->SelfAction())
			_HasSelfAction = true;
		else
			_HasNormalAction = true;
	}
}

void AISHEETS::CActionList::readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId)
{
	NLGEORGES::UFormElm const& item = form->getRootNode();
	// the form was found so read the true values from George
	_SheetId = sheetId;
	
	{
		NLGEORGES::UFormElm const* actionListNode = NULL;
		item.getNodeByName(&actionListNode, "actions");
		
		if (actionListNode)
		{
			uint arraySize = 0;
			actionListNode->getArraySize(arraySize);
			for	(uint i=0; i<arraySize; ++i)
			{
				std::string action;
				actionListNode->getArrayValue(action, i);
				if (!action.empty())
				{
					addAction(NLMISC::CSheetId(action), action);
				}
			}
		}
	}
	computeAbilities();
}

uint AISHEETS::CActionList::getVersion()
{
	return 4;
}

void AISHEETS::CActionList::serial(NLMISC::IStream& s)
{
	s.serial(_SheetId);
	
	if (s.isReading())
	{
		uint32 nbSheet;
		s.serial(nbSheet);
		for (uint32 i=0; i<nbSheet; ++i)
		{
			NLMISC::CSheetId sheetId;
			s.serial(sheetId);
			addAction(sheetId, std::string());
		}
	}
	else
	{
		uint32 nbSheet = (uint32)_Actions.size();
		s.serial(nbSheet);
		for (uint32 i=0; i<nbSheet; ++i)
		{
			NLMISC::CSheetId sheetId = _Actions[i]->SheetId();
			s.serial(sheetId);
		}
	}
	computeAbilities();
}

void AISHEETS::CActionList::addAction(NLMISC::CSheetId const& sheetId, std::string const& actionName)
{
	IAIActionCPtr action = CSheets::getInstance()->lookupAction(sheetId);
	
	if (!action.isNull())
	{
		_Actions.push_back(action);
	}
	else
	{
		if (!actionName.empty())				
			nlwarning("action %s doesnt exist", actionName.c_str());
	}
}

std::vector<std::string> AISHEETS::CActionList::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	pushTitle(container, "AISHEETS::CActionList");
	pushEntry(container, "action_list sheet");
	pushFooter(container);
	
	return container;
}

//////////////////////////////////////////////////////////////////////////////
// CGroupProperties                                                         //
//////////////////////////////////////////////////////////////////////////////

AISHEETS::CGroupProperties::CGroupProperties()
: _Assist(false)
, _Attack(false)
{
}

//////////////////////////////////////////////////////////////////////////////
// CCreature                                                                //
//////////////////////////////////////////////////////////////////////////////

AISHEETS::CCreature::CCreature()
: _Level(1)
, _Radius(0.5f), _Height(2.0f), _Width(1.0f), _Length(1.0f)
, _BoundingRadius(0.5)
, _BonusAggroHungry(0.0), _BonusAggroVeryHungry(0.0)
, _AssistDist(10)
, _MinFightDist(0)
, _FactionIndex(CStaticFames::INVALID_FACTION_INDEX)
, _FameForGuardAttack(ICreature::InvalidFameForGuardAttack)
, _GroupPropertiesIndex(0)
, _DynamicGroupCountMultiplier(1)
{
}

void AISHEETS::CCreature::calcFightAndVisualValues(std::string* left, std::string* right)
{
	CVisualSlotManager* visualSlotManager = CVisualSlotManager::getInstance();
#ifdef NL_DEBUG
	nlassert(visualSlotManager);
#endif
	uint32 leftAsInt = visualSlotManager->leftItem2Index(LeftItem());
	uint32 rightAsInt = visualSlotManager->rightItem2Index(RightItem());
	
	if (LeftItem()!=NLMISC::CSheetId::Unknown && leftAsInt==0 && left!=NULL)
	{
		if (left->size() < 3 || left->at(3) != 'p')
			nlwarning("Left item '%s' not allowed, if ammo, this is normal", left->c_str());
	}
	
	if (RightItem()!=NLMISC::CSheetId::Unknown && rightAsInt==0 && right!=NULL)
	{
		if (right->size() < 3 || right->at(3) != 'p')
			nlwarning("Right item '%s' not allowed, if ammo, this is normal", right->c_str());
	}
	_MinFightDist = (!FightConfig(FIGHTCFG_RANGE).isNULL()||!FightConfig(FIGHTCFG_NUKE).isNULL())?20:0;
}

void AISHEETS::CCreature::parseFightConfig(NLGEORGES::UForm const* form, std::string const& fightConfigString, uint32 actionListIndex, NLMISC::CDbgPtr<CActionList>& fightConfig)
{
	NLGEORGES::UFormElm const* actionListNode = NULL;
	const_cast<NLGEORGES::UFormElm&>(form->getRootNode()).getNodeByName(&actionListNode, fightConfigString.c_str());
	
	if (actionListNode)
	{
		uint arraySize = 0;
		actionListNode->getArraySize(arraySize);
		
		if (actionListIndex<arraySize)
		{
			std::string	actionListFileName;
			actionListNode->getArrayValue(actionListFileName,actionListIndex);
			addActionConfig	(actionListFileName, fightConfig);
		}
	}
}

void AISHEETS::CCreature::readFightConfig(NLMISC::IStream& s, NLMISC::CDbgPtr<CActionList>& fightConfig)
{
	NLMISC::CSheetId sheetId;
	s.serial(sheetId);
	if (sheetId!=NLMISC::CSheetId::Unknown)
		addActionConfig(sheetId, fightConfig);
}

void AISHEETS::CCreature::saveFightConfig(NLMISC::IStream& s, NLMISC::CDbgPtr<CActionList>& fightConfig)
{
	if (!fightConfig.isNULL())
	{
		s.serial(fightConfig->_SheetId);
	}
	else
	{
		NLMISC::CSheetId id;
		s.serial(id);
	}
}

bool AISHEETS::CCreature::mustAssist(CCreature const& creature) const
{
	return getPropertiesCst(creature.GroupPropertiesIndex()).assist();
}

void AISHEETS::CCreature::setAssisGroupIndexs()
{
	_GroupPropertiesIndex = CSheets::getInstance()->getGroupPropertiesIndex(GroupIndexStr());
	if (_GroupPropertiesIndex==~0)
		return;
	
	std::vector<uint32> groupList;
	getGroupStr(groupList, AssistGroupIndexStr());
	
	FOREACH(it, std::vector<uint32>, groupList)
		getProperties(*it).setAssist(true);
}

void AISHEETS::CCreature::setAttackGroupIndexs()
{
	_GroupPropertiesIndex = CSheets::getInstance()->getGroupPropertiesIndex(GroupIndexStr());
	if (_GroupPropertiesIndex==~0)
		return;
	
	std::vector<uint32>	groupList;
	getGroupStr(groupList, AttackGroupIndexStr());
	
	FOREACH(it, std::vector<uint32>, groupList)
		getProperties(*it).setAttack(true);
}

void AISHEETS::CCreature::addActionConfig(std::string const& sheetIdName, NLMISC::CDbgPtr<CActionList>& actionConfigList)
{
	if (sheetIdName.empty())
	{
		nlwarning("sheetIdName is empty");
		return;
	}

	if (!addActionConfig(NLMISC::CSheetId(sheetIdName), actionConfigList))
	{
#ifdef NL_DEBUG
		nlwarning("Error in actionConfig Reference for %s", sheetIdName.c_str());
#endif
	}
}

bool AISHEETS::CCreature::addActionConfig(NLMISC::CSheetId const& sheetId, NLMISC::CDbgPtr<CActionList>& actionConfigList)
{
	CActionList const* actionConfig = CSheets::getInstance()->lookupActionList(sheetId);
	if (actionConfig)
	{
		actionConfigList = actionConfig;	//	fightConfigList.push_back(actionConfig);
		return true;
	}
	return false;
}

AISHEETS::CGroupProperties& AISHEETS::CCreature::getProperties(uint32 groupIndex)
{
#if !FINAL_VERSION
	nlassert(groupIndex!=~0);
#endif
	if (_GroupPropertiesTbl.size()<=groupIndex && groupIndex!=~0)
	{
		uint32 const resizeSize = std::max((uint32)CSheets::getInstance()->_NameToGroupIndex.size(), (uint32)(groupIndex+1));
		_GroupPropertiesTbl.resize(resizeSize);
	}
	return _GroupPropertiesTbl[groupIndex];
}

AISHEETS::CGroupProperties const& AISHEETS::CCreature::getPropertiesCst(uint32 groupIndex) const
{
	if (groupIndex<_GroupPropertiesTbl.size())
		return _GroupPropertiesTbl[groupIndex];
	else
		return CSheets::getInstance()->_DefaultGroupProp;
}

std::vector<std::string> AISHEETS::CCreature::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	pushTitle(container, "AISHEETS::CCreature");
	pushEntry(container, "sheet=" + SheetId().toString());
	pushEntry(container, "level=" + toString("%d", Level()));
	container.back() += "radii=" + toString("%4.1f,%4.1f", Radius(), BoundingRadius());
	container.back() += "height=" + toString("%4.1f", Height());
	pushFooter(container);
	
	return container;
}

void AISHEETS::CCreature::readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId)
{
	NLGEORGES::UFormElm const& item = form->getRootNode();
	
	// the form was found so read the true values from George
	_SheetId = sheetId;
#ifdef NL_DEBUG
	nlassert(debugSheet.get().empty() || _SheetId!=NLMISC::CSheetId(debugSheet));
#endif
	
	item.getValueByName(_Level,"Basics.Level");
	
	if (!item.getValueByName(_DynamicGroupCountMultiplier, "Basics.Characteristics.DynGroupCountMultiplier"))
		_DynamicGroupCountMultiplier = 1;
	
	item.getValueByName(_ColorHead,"Basics.Equipment.Head.Color");
	item.getValueByName(_ColorArms,"Basics.Equipment.Arms.Color");
	item.getValueByName(_ColorHands,"Basics.Equipment.Hands.Color");
	item.getValueByName(_ColorBody,"Basics.Equipment.Body.Color");
	item.getValueByName(_ColorLegs,"Basics.Equipment.Legs.Color");
	item.getValueByName(_ColorFeets,"Basics.Equipment.Feet.Color");
	
	item.getValueByName(_Radius,"Collision.CollisionRadius");
	item.getValueByName(_Height,"Collision.Height");
	item.getValueByName(_Width,"Collision.Width");
	item.getValueByName(_Length,"Collision.Length");
	item.getValueByName(_BoundingRadius,"Collision.BoundingRadius");
	
	item.getValueByName(_NotTraversable, "Collision.NotTraversable");
	
	item.getValueByName(_BonusAggroHungry,"Combat.BonusAggroHungry");
	item.getValueByName(_BonusAggroVeryHungry,"Combat.BonusAggroVeryHungry");
	
	item.getValueByName(_AggroRadiusNotHungry,"Combat.AggroRadiusNotHungry");
	item.getValueByName(_AggroRadiusHungry,"Combat.AggroRadiusHungry");
	item.getValueByName(_AggroRadiusHunting,"Combat.AggroRadiusHunting");
	
	if (!item.getValueByName(_AggroReturnDistCheck,"Combat.AggroReturnDistCheck"))
		_AggroReturnDistCheck = -1.f;
	if (!item.getValueByName(_AggroRadiusD1,"Combat.AggroRadiusD1"))
		_AggroRadiusD1 = -1.f;
	if (!item.getValueByName(_AggroRadiusD2,"Combat.AggroRadiusD2"))
		_AggroRadiusD2 = -1.f;
	if (!item.getValueByName(_AggroPrimaryGroupDist,"Combat.AggroPrimaryGroupDist"))
		_AggroPrimaryGroupDist = -1.f;
	if (!item.getValueByName(_AggroPrimaryGroupCoef,"Combat.AggroPrimaryGroupCoef"))
		_AggroPrimaryGroupCoef = -1.f;
	if (!item.getValueByName(_AggroSecondaryGroupDist,"Combat.AggroSecondaryGroupDist"))
		_AggroSecondaryGroupDist = -1.f;
	if (!item.getValueByName(_AggroSecondaryGroupCoef,"Combat.AggroSecondaryGroupCoef"))
		_AggroSecondaryGroupCoef = -1.f;
	if (!item.getValueByName(_AggroPropagationRadius,"Combat.AggroPropagationRadius"))
		_AggroPropagationRadius = -1.f;
	
	item.getValueByName(_AssistDist,"Combat.AssistDist");
	
	item.getValueByName(_Scale, "3d data.Scale");
	{
		std::string faunaTypeStr;
		item.getValueByName(faunaTypeStr, "Basics.type");
		_FaunaType = getType<TFaunaType>(faunaTypeStr.c_str());
	}
	
	item.getValueByName(_ForceDisplayCreatureName, "3d data.ForceDisplayCreatureName");
	
	// Get the dist fromm Bip to Mid
	float tmpBip01ToMid;
	if (!item.getValueByName(tmpBip01ToMid, "Collision.Dist Bip01 to mid"))
		tmpBip01ToMid = 0.f;
	// Get the distance from the bip01 to the front.
	if (!item.getValueByName(_DistToFront, "Collision.Dist Bip01 to front"))
		_DistToFront = 1.f;
	// Get the distance from the bip01 to the front.
	if (!item.getValueByName(_DistToBack, "Collision.Dist Bip01 to back"))
		_DistToBack = 1.f;
	// Get the creature Width.
	if (!item.getValueByName(_DistToSide, "Collision.Width"))
		_DistToSide = 1.f;
	
	_DistToFront = _DistToFront-tmpBip01ToMid;
	_DistToBack = tmpBip01ToMid-_DistToBack;
	_DistToSide = _DistToSide/2.f;
	
	_DistToFront *= _Scale;
	_DistToBack *= _Scale;
	_DistToSide *= _Scale;
	
	if (!item.getValueByName(_DistModulator, "Combat.DistModulator"))
		_DistModulator = 0.5f;		// (0) - (1) - (n).
	
	if (!item.getValueByName(_TargetModulator, "Combat.TargetModulator"))
		_TargetModulator = 1.f;		// (0) - (1).
	
	if (!item.getValueByName(_ScoreModulator, "Combat.ScoreModulator"))
		_ScoreModulator = 0.01f;		// (0) - (1).
	
	if (!item.getValueByName(_FearModulator, "Combat.FearModulator"))
		_FearModulator = 0.01f;		// (0) - (1).
	
	if (!item.getValueByName(_LifeLevelModulator, "Combat.LifeLevelModulator"))
		_LifeLevelModulator = 0.5f;	// (0) - (1).
	
	if (!item.getValueByName(_CourageModulator, "Combat.CourageModulator"))
		_CourageModulator = 2.f;	// (-n) - (0) - (+n).
	
	if (!item.getValueByName(_GroupCohesionModulator, "Combat.GroupCohesionModulator"))
		_GroupCohesionModulator = 0.5f;	// (0) - (1)
	
	if (!item.getValueByName(_GroupDispersion, "Basics.MovementSpeeds.GroupDispersion"))
		_GroupDispersion = 0.5f;	// (0) - (1).
	
	if (!item.getValueByName(_XPLevel, "Basics.XPLevel"))
		_XPLevel = 1;
	
	if (!item.getValueByName(_NbPlayers, "Basics.NbPlayers"))
		_NbPlayers = 1;
	
	nlassert(_DistModulator>=0);
	nlassert(_TargetModulator>=0);
	nlassert(_ScoreModulator>=0 && _ScoreModulator<=1);
	nlassert(_FearModulator>=0 && _FearModulator<=1);
	nlassert(_LifeLevelModulator>=0 && _LifeLevelModulator<=1);
	nlassert(_GroupCohesionModulator>=0 && _GroupCohesionModulator<=1);			
	nlassert(_GroupDispersion>=0 && _GroupDispersion<=1);
	
	_EnergyValue = uint32(0.01f * ENERGY_SCALE);
	float v;
	if (item.getValueByName(v, "Basics.Characteristics.DynamicEnergyValue") && v!=0)
		_EnergyValue = uint32(v * ENERGY_SCALE);
	
	if (!item.getValueByName(_CanTurn, "Properties.Turn"))
		_CanTurn = true;
	
	uint32 meleeConfigChoice = 0;
	uint32 rangeConfigChoice = 0;
	uint32 nukeConfigChoice = 0;
	uint32 healConfigChoice = 0;
	
	breakable
	{
		std::string	actionConfigStr;
		item.getValueByName(actionConfigStr, "action_cfg");
		
		if (actionConfigStr.length()!=5) // 4numbers + "f".
			break;
		
		char a[2] = "0";
		a[0] = actionConfigStr[0];
		meleeConfigChoice = atol(a);
		a[0] = actionConfigStr[1];
		rangeConfigChoice = atol(a);
		a[0] = actionConfigStr[2];
		nukeConfigChoice = atol(a);
		a[0] = actionConfigStr[3];
		healConfigChoice = atol(a);
	}
	
	static std::string meleeFightConfigString("melee_cfg");
	static std::string rangeFightConfigString("range_cfg");
	static std::string nukeFightConfigString("nuke_cfg");
	static std::string healFightConfigString("heal_cfg");

	if (meleeConfigChoice>0)
		parseFightConfig(form, meleeFightConfigString, meleeConfigChoice-1, _FightConfig[FIGHTCFG_MELEE]);
	if (rangeConfigChoice>0)
		parseFightConfig(form, rangeFightConfigString, rangeConfigChoice-1, _FightConfig[FIGHTCFG_RANGE]);
	if (nukeConfigChoice>0)
		parseFightConfig(form, nukeFightConfigString, nukeConfigChoice-1, _FightConfig[FIGHTCFG_NUKE]);
	if (healConfigChoice>0)
		parseFightConfig(form, healFightConfigString, healConfigChoice-1, _FightConfig[FIGHTCFG_HEAL]);
	
	//	reads left & right item.
	{
		std::string	left;
		item.getValueByName(left, "item_left");
		if (!left.empty())
			_LeftItem = NLMISC::CSheetId(left);
		
		std::string	right;
		item.getValueByName(right, "item_right");
		if (!right.empty())
			_RightItem = NLMISC::CSheetId(right);
		
		calcFightAndVisualValues(&left, &right);
	}
	
	std::string s;
	if (item.getValueByName(s, "Basics.Fame"))
	{
		_FactionIndex = CStaticFames::getInstance().getFactionIndex(s);
	}
	if (item.getValueByName(s, "Basics.FameForGuardAttack") && !s.empty())
	{
		double tmp;
		sscanf(s.c_str(), "%f", &tmp);
		_FameForGuardAttack = (sint32)tmp;
	}
	else
		_FameForGuardAttack = ICreature::InvalidFameForGuardAttack;
	
	//	Assist Group Indexs.
	{
		item.getValueByName(_GroupIndexStr,"group_id");
		if (_GroupIndexStr.empty())
		{
			std::string	cat;
			std::string	raceCode;
			if (item.getValueByName(cat, "category") && item.getValueByName(raceCode, "race_code"))
				_GroupIndexStr = cat + raceCode;
		}
		
		item.getValueByName(_AssistGroupIndexStr, "group_assist");
		setAssisGroupIndexs();
		item.getValueByName(_AttackGroupIndexStr, "group_attack");
		setAttackGroupIndexs();
	}
	
	// Bot name
	item.getValueByName(_BotName, "Basics.BotName");
	
	//////////////////////////////////////////////////////////////////////////
	//	Reads Script Comps.
	breakable
	{
		NLGEORGES::UFormElm const* scriptCompNode = NULL;
		const_cast<NLGEORGES::UFormElm&>(form->getRootNode()).getNodeByName(&scriptCompNode, "special_comp");
		
		if (!scriptCompNode)
			break;
		
		uint arraySize = 0;
		scriptCompNode->getArraySize(arraySize);
		
		for	(uint arrayIndex=0; arrayIndex<arraySize; ++arrayIndex)
		{
			std::string	scriptCompStr;
			scriptCompNode->getArrayValue(scriptCompStr, arrayIndex);
#ifndef NO_AI_COMP
			CFightScriptComp* scriptComp;
			try
			{
				scriptComp = CFightScriptCompReader::createScriptComp(scriptCompStr);
				registerScriptComp(scriptComp);
			}
			catch (const ReadFightActionException& ex)
			{
				nlwarning("script read error (ignored): %s", ex.what());
			}
#endif
		}
	}
	// Creature race
	breakable
	{
		string raceStr;
		if(item.getValueByName(raceStr, "Basics.Race") && !raceStr.empty())
			_Race = EGSPD::CPeople::fromString(raceStr);
		else
			_Race = EGSPD::CPeople::Unknown;
	}
}

void AISHEETS::CCreature::registerScriptComp(CFightScriptComp* scriptComp)
{
	_ScriptCompList.push_back(scriptComp);
	
	CFightSelectFilter* filter = dynamic_cast<CFightSelectFilter*>(scriptComp);
	if (!filter)
		return;
	
	std::string const& param = filter->getParam();
	if (param=="ON_UPDATE")
		_UpdateScriptList.push_back(scriptComp);
	if (param=="ON_DEATH")
		_DeathScriptList.push_back(scriptComp);
	if (param=="ON_BIRTH")
		_BirthScriptList.push_back(scriptComp);
}


uint AISHEETS::CCreature::getVersion()
{ 
	return 45;
}

void AISHEETS::CCreature::serial(NLMISC::IStream &s)
{
	s.serial(_SheetId, _Level);
#ifdef NL_DEBUG
	nlassert(debugSheet.get().empty() || _SheetId!=NLMISC::CSheetId(debugSheet));
#endif
	
	s.serial(_DynamicGroupCountMultiplier);
	s.serial(_ColorHead, _ColorArms, _ColorHands),
	s.serial(_ColorBody, _ColorLegs, _ColorFeets);
	
	s.serial(_Radius, _Height, _Width, _Length);
	s.serial(_BoundingRadius);
	s.serial(_BonusAggroHungry, _BonusAggroVeryHungry);
	s.serial(_AggroRadiusNotHungry, _AggroRadiusHungry, _AggroRadiusHunting);
	s.serial(_AggroReturnDistCheck);
	s.serial(_AggroRadiusD1, _AggroRadiusD2);
	s.serial(_AggroPrimaryGroupDist);
	s.serial(_AggroPrimaryGroupCoef);
	s.serial(_AggroSecondaryGroupDist);
	s.serial(_AggroSecondaryGroupCoef);
	s.serial(_AggroPropagationRadius);
	s.serial(_Scale);
	s.serial(_ForceDisplayCreatureName);
	s.serialEnum(_FaunaType);
	
	s.serial(_DistToFront);
	s.serial(_DistToBack);
	s.serial(_DistToSide);
	
	s.serial(_DistModulator);
	s.serial(_TargetModulator);
	s.serial(_ScoreModulator);
	s.serial(_FearModulator);
	s.serial(_LifeLevelModulator);
	s.serial(_CourageModulator);
	s.serial(_GroupCohesionModulator);
	
	s.serial(_GroupDispersion);
	
	s.serial(_XPLevel);
	s.serial(_NbPlayers);
	
	s.serial(_EnergyValue);
	
	s.serial(_CanTurn);
	
	if (s.isReading())
	{
		readFightConfig(s, _FightConfig[FIGHTCFG_MELEE]);
		readFightConfig(s, _FightConfig[FIGHTCFG_RANGE]);
		readFightConfig(s, _FightConfig[FIGHTCFG_NUKE]);
		readFightConfig(s, _FightConfig[FIGHTCFG_HEAL]);
	}
	else
	{
		saveFightConfig(s, _FightConfig[FIGHTCFG_MELEE]);
		saveFightConfig(s, _FightConfig[FIGHTCFG_RANGE]);
		saveFightConfig(s, _FightConfig[FIGHTCFG_NUKE]);
		saveFightConfig(s, _FightConfig[FIGHTCFG_HEAL]);
	}
	
	s.serial(_AssistDist);
	
	//	serialize left & right item.
	s.serial(_LeftItem, _RightItem);
	s.serial(_FactionIndex);
	s.serial(_FameForGuardAttack);
	
	s.serial(_GroupIndexStr);
	s.serial(_AssistGroupIndexStr);
	s.serial(_AttackGroupIndexStr);
	
	s.serial(_BotName);
	s.serialEnum(_Race);
	
	if (s.isReading())
	{
		setAssisGroupIndexs();
		setAttackGroupIndexs();
	}
	
	if (s.isReading())
	{
		uint32 nbScript;
		s.serial(nbScript);
		for	(uint32	index=0; index<nbScript; ++index)
		{
			string scriptCompStr;
			s.serial(scriptCompStr);
			
#ifndef NO_AI_COMP
			CFightScriptComp* scriptComp;
			try
			{
				scriptComp = CFightScriptCompReader::createScriptComp(scriptCompStr);
				registerScriptComp(scriptComp);
			}
			catch (const ReadFightActionException& ex)
			{
				nlwarning("script read error (ignored): %s", ex.what());
			}
#endif
		}
	}
	else
	{
		uint32 nbScript = (uint32)_ScriptCompList.size();
		s.serial(nbScript);
		for (uint32 index=0; index<nbScript; ++index)
		{
			string str = _ScriptCompList[index]->toString();
			s.serial(str);
		}
	}
	
	calcFightAndVisualValues();
}

void AISHEETS::CCreature::getGroupStr(std::vector<uint32>& groupIndexStrList, std::string const& groupIndexStr)
{
	if (groupIndexStr.empty())
		return;
	
	size_t firstIndex = 0;
	size_t lastIndex = firstIndex - 1;
	
	do
	{
		firstIndex = lastIndex + 1;
		lastIndex = groupIndexStr.find_first_of(',',firstIndex);
		
		std::string str;
		if (lastIndex==std::string::npos)
			str = groupIndexStr.substr(firstIndex, groupIndexStr.size()-firstIndex);
		else
			str = groupIndexStr.substr(firstIndex, lastIndex-firstIndex);
		
		uint32 const otherGroupIndex = CSheets::getInstance()->getGroupPropertiesIndex(str);
		if (otherGroupIndex!=~0)
			groupIndexStrList.push_back(otherGroupIndex);
	} while (lastIndex!=std::string::npos);
}

//////////////////////////////////////////////////////////////////////////////
// CRaceStats                                                               //
//////////////////////////////////////////////////////////////////////////////

void AISHEETS::CRaceStats::readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId)
{
	NLGEORGES::UFormElm const& item = form->getRootNode();
	
	// the form was found so read the true values from George
	_SheetId = sheetId;
#ifdef NL_DEBUG
	nlassert(debugSheet.get().empty() || _SheetId!=NLMISC::CSheetId(debugSheet));
#endif
	
	item.getValueByName(_Race, "Race");
}

uint AISHEETS::CRaceStats::getVersion()
{ 
	return 1;
}

void AISHEETS::CRaceStats::serial(NLMISC::IStream &s)
{
	s.serial(_SheetId);
#ifdef NL_DEBUG
	nlassert(debugSheet.get().empty() || _SheetId!=NLMISC::CSheetId(debugSheet));
#endif
	
	s.serial(_SheetId);
	s.serial(_Race);
}

//////////////////////////////////////////////////////////////////////////////
// CSheets                                                                  //
//////////////////////////////////////////////////////////////////////////////

AISHEETS::CSheets* AISHEETS::CSheets::_Instance = NULL;

AISHEETS::CSheets* AISHEETS::CSheets::getInstance()
{
	if (!_Instance)
		_Instance = new AISHEETS::CSheets;
	return _Instance;
}

void AISHEETS::CSheets::destroyInstance()
{
	delete _Instance;
	_Instance = NULL;
}

AISHEETS::CSheets::CSheets()
: _Initialised(false)
{
}

void AISHEETS::CSheets::init()
{
	if (_Initialised)
		return;
	
	_PlayerGroupIndex=getGroupPropertiesIndex("zp");
#if !FINAL_VERSION
	nlassert(_PlayerGroupIndex!=~0);
#endif
	
	packSheets(IService::getInstance()->WriteFilesDirectory.toString());
	
	_Initialised=true;
}

void AISHEETS::CSheets::packSheets(const std::string &writeFilesDirectoryName)
{
	CConfigFile::CVar *varPtr = IService::isServiceInitialized() ? IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths")) : NULL;
	
	// if config file variable 'GeorgePaths' exists then only do a minimal loadForms otherwise do the full works
	if (varPtr!=NULL)
	{
		
		bool	addSearchPath=false;
		
		loadForm2("aiaction",	writeFilesDirectoryName+AISPackedActionSheetsFilename, _ActionSheets, false, false);
		if (_ActionSheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath=true;
				for (uint32 i=0;i<varPtr->size();++i)
					CPath::addSearchPath(varPtr->asString(i).c_str(), true, false);
			}
			loadForm2("aiaction",	writeFilesDirectoryName+AISPackedActionSheetsFilename, _ActionSheets, true);
		}
		
		loadForm("actionlist",	writeFilesDirectoryName+AISPackedFightConfigSheetsFilename, _ActionListSheets, false, false);
		if (_ActionListSheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath=true;
				for (uint32 i=0;i<varPtr->size();++i)
					CPath::addSearchPath(varPtr->asString(i).c_str(), true, false);
			}
			loadForm("actionlist", writeFilesDirectoryName+AISPackedFightConfigSheetsFilename, _ActionListSheets, true);
		}
		
		
		loadForm2("creature",	writeFilesDirectoryName+AISPackedSheetsFilename, _Sheets, false, false);
		if (_Sheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath=true;
				for (uint32 i=0;i<varPtr->size();++i)
					CPath::addSearchPath(varPtr->asString(i).c_str(), true, false);
			}
			loadForm2("creature", writeFilesDirectoryName+AISPackedSheetsFilename, _Sheets, true);
		}
		
		loadForm2("race_stats",	writeFilesDirectoryName+AISPackedRaceStatsSheetsFilename, _RaceStatsSheets, false, false);
		if (_RaceStatsSheets.empty())
		{
			if (!addSearchPath)
			{
				addSearchPath=true;
				for (uint32 i=0;i<varPtr->size();++i)
					CPath::addSearchPath(varPtr->asString(i).c_str(), true, false);
			}
			loadForm2("race_stats", writeFilesDirectoryName+AISPackedRaceStatsSheetsFilename, _RaceStatsSheets, true);
		}
		
	}
	else
	{
		loadForm2("aiaction",	writeFilesDirectoryName+AISPackedActionSheetsFilename, _ActionSheets, true);
		loadForm("actionlist",	writeFilesDirectoryName+AISPackedFightConfigSheetsFilename, _ActionListSheets, true);
		loadForm2("creature",	writeFilesDirectoryName+AISPackedSheetsFilename, _Sheets, true);
		loadForm2("race_stats",	writeFilesDirectoryName+AISPackedRaceStatsSheetsFilename, _RaceStatsSheets, true);
	}
}

void AISHEETS::CSheets::release()
{
	_Sheets.clear();
	_ActionListSheets.clear();
	_ActionSheets.clear();
	_RaceStatsSheets.clear();
}

uint32 AISHEETS::CSheets::getGroupPropertiesIndex(std::string groupIndexName)
{
	if (groupIndexName.empty())
		return	~0;
	
	NLMISC::strupr(groupIndexName);
	std::map<string, uint32>::iterator it = _NameToGroupIndex.find(groupIndexName);
	if (it==_NameToGroupIndex.end())
	{
		uint32 groupIndex = (uint32)_NameToGroupIndex.size();
		_NameToGroupIndex.insert(make_pair(groupIndexName, groupIndex));
#if !FINAL_VERSION
		nldebug("GroupIndex Entry: %s %d", groupIndexName.c_str(), groupIndex);
#endif
		
		it = _NameToGroupIndex.find(groupIndexName);
#ifdef NL_DEBUG
		nlassert(it!=_NameToGroupIndex.end());
#endif
		// Resize other group table. Better imp should be done with listeners.
	}
	return it->second;
}

void AISHEETS::CSheets::display(CSmartPtr<CStringWriter>	stringWriter, uint infoSelect)
{
	nlassert(_Initialised);
	
	std::map<CSheetId, AISHEETS::CCreaturePtr>::iterator it;
	for(it=_Sheets.begin(); it!=_Sheets.end(); ++it)
	{
		std::vector<std::string> strings;
		strings = it->second->getMultiLineInfoString();
		FOREACHC(itString, std::vector<std::string>, strings)
			stringWriter->append(toString("%04x", it->second->SheetId().asInt()) + " " + *itString);
	}
}

AISHEETS::ICreatureCPtr AISHEETS::CSheets::lookup(CSheetId const& id)
{
	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId, AISHEETS::CCreaturePtr>::iterator it=_Sheets.find(id);
	
	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it!=_Sheets.end())
		return (AISHEETS::CCreature*)it->second;
	else
	{
		nlwarning("Unknow creature sheet '%s'", id.toString().c_str());
		return NULL;
	}
}

AISHEETS::IAIActionCPtr AISHEETS::CSheets::lookupAction(CSheetId const& id)
{
	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId, CAIActionPtr>::iterator it = _ActionSheets.find(id);
	
	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it!=_ActionSheets.end())
		return (AISHEETS::CAIAction*)it->second;
	else
		return NULL;
}

AISHEETS::CActionList const* AISHEETS::CSheets::lookupActionList(CSheetId const& id)
{
	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId, AISHEETS::CActionList>::iterator it=_ActionListSheets.find(id);
	
	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it!=_ActionListSheets.end())
		return &((*it).second);
	else
		return NULL;
}

AISHEETS::IRaceStatsCPtr AISHEETS::CSheets::lookupRaceStats(CSheetId const& id)
{
	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId, AISHEETS::CRaceStatsPtr>::iterator it=_RaceStatsSheets.find(id);
	
	if (it!=_RaceStatsSheets.end())
		return (AISHEETS::CRaceStats*)it->second;
	else
	{
		nlwarning("Unknow race_stats sheet '%s'", id.toString().c_str());
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Console commands                                                         //
//////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND(displaySheetNames,"display sheet data for all sheets","")
{
	if(args.size() !=0) return false;

	AISHEETS::CSheets::getInstance()->display(new CLogStringWriter(&log),0);

	return true;
}

NLMISC_COMMAND(displaySheetBasics,"display sheet data for all sheets","")
{
	if(args.size() !=0) return false;

	AISHEETS::CSheets::getInstance()->display(new CLogStringWriter(&log),1);

	return true;
}

NLMISC_COMMAND(displaySheetCombat,"display sheet data for all sheets","")
{
	if(args.size() !=0) return false;

	AISHEETS::CSheets::getInstance()->display(new CLogStringWriter(&log),2);

	return true;
}

NLMISC_COMMAND(displaySheetByName,"display sheet data for given sheets","<sheet> [<sheet>...]")
{
	if	(args.size() <1)
		return	false;
	
	for	(uint i=0;i<args.size();++i)
	{
		// lookup the sheet id
		AISHEETS::ICreatureCPtr sheet = AISHEETS::CSheets::getInstance()->lookup(NLMISC::CSheetId(args[i]));
		if (!sheet)
		{
			log.displayNL("Failed to find sheet: %s",args[0].c_str());
			continue;
		}
		std::vector<std::string> strings = sheet->getMultiLineInfoString();
		FOREACHC(it, std::vector<std::string>, strings)
			log.displayNL("%s", it->c_str());
	}
	return true;
}
/*
NLMISC_COMMAND(setSheetProperty,"change a value read from a sheet","<sheet> level|walk|run|radius|bounding|height|aggro|attack|danger|flight|survive|initSurv|crit <value>")
{
	if (args.size() !=3)
		return false;
	
	// lookup the sheet id
	AISHEETS::CCreature* sheet = const_cast<AISHEETS::CCreature*>(dynamic_cast<AISHEETS::CCreature const*>(AISHEETS::CSheets::getInstance()->lookup(NLMISC::CSheetId(args[0]))));
	if (!sheet)
	{
		log.displayNL("Failed to find sheet: %s",args[0].c_str());
		return false;
	}

	// get the value
	float val = (float)atof(args[2].c_str());
	if (val==0 && args[2]!="0" && args[2]!="0.0")
	{
		log.displayNL("'%s' is not a valid value",args[2].c_str());
		return false;
	}
	
	breakable
	{
		if (nlstricmp(args[1].c_str(),"level")==0)		{ sheet->_Level=(uint32)val;		break; }
		if (nlstricmp(args[1].c_str(),"radius")==0)		{ sheet->_Radius=val;			break; }
		if (nlstricmp(args[1].c_str(),"bounding")==0)	{ sheet->_BoundingRadius=val;	break; }
		if (nlstricmp(args[1].c_str(),"height")==0)		{ sheet->_Height=val;			break; }
		
		log.displayNL("variable name not recognised: %s", args[1].c_str());
		return false;
	}
	
	std::vector<std::string> strings = sheet->getMultiLineInfoString();
	FOREACHC(it, std::vector<std::string>, strings)
		log.displayNL("%s", it->c_str());
	return true;
}
*/
