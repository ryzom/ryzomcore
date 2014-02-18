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



#ifndef RYAI_SHEETS_H
#define RYAI_SHEETS_H


// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sheet_id.h"

///Nel Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

#include "ai_share/ai_types.h"
#include "game_share/fame.h"

#include "game_share/visual_slot_manager.h"

#include	"ai_script_comp.h"

#include "game_share/people_pd.h"

/**
 * Singleton containing database on information for actors
 * \author Sadge
 * \author Nevrax France
 * \date 2002
 */


namespace AISHEETS
{

enum TFightCfg
{
	FIGHTCFG_MELEE=0,
		FIGHTCFG_RANGE,
		FIGHTCFG_NUKE,
		FIGHTCFG_HEAL,
		FIGHTCFG_MAX
};

//////////////////////////////////////////////////////////////////////////////
// CAIAction                                                                //
//////////////////////////////////////////////////////////////////////////////

class IAIAction
: public NLMISC::CRefCount
//, public NLMISC::CDbgRefCount<IAIAction>
{
public:
	virtual ~IAIAction() { }
public:
	virtual NLMISC::CSheetId SheetId() const = 0;
	virtual bool SelfAction() const = 0;
};
typedef NLMISC::CSmartPtr<IAIAction> IAIActionPtr;
typedef NLMISC::CSmartPtr<IAIAction const> IAIActionCPtr;

class CAIAction
: public IAIAction
{
public:
	CAIAction();
	virtual NLMISC::CSheetId SheetId() const { return _SheetId; }
	virtual bool SelfAction() const { return _SelfAction; }
	
public:
	void readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId);
	void serial(NLMISC::IStream& s);
	void removed() { }
	static uint getVersion();
	
	virtual std::vector<std::string> getMultiLineInfoString() const;
	
private:
	NLMISC::CSheetId _SheetId;
	bool _SelfAction;
};
typedef NLMISC::CSmartPtr<CAIAction> CAIActionPtr;
typedef NLMISC::CSmartPtr<CAIAction const> CAIActionCPtr;

//////////////////////////////////////////////////////////////////////////////
// CActionList                                                              //
//////////////////////////////////////////////////////////////////////////////

class CActionList
: public NLMISC::CDbgRefCount<CActionList>
{
public:
	virtual ~CActionList() { }
	
	NLMISC::CSheetId			_SheetId;
	std::vector<IAIActionCPtr>	_Actions;

	bool _HasNormalAction;
	bool _HasSelfAction;
	
	void computeAbilities();
	
	void readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId);
	void serial(NLMISC::IStream& s);
	void removed() { }
	static uint getVersion();
	
	void addAction(NLMISC::CSheetId const& sheetId, std::string const& actionName);
	
	virtual std::vector<std::string> getMultiLineInfoString() const;
};

//////////////////////////////////////////////////////////////////////////////
// CGroupProperties                                                         //
//////////////////////////////////////////////////////////////////////////////

class CGroupProperties
{
public:
	CGroupProperties();
	virtual ~CGroupProperties() { }
	virtual void setAssist(bool assist) { _Assist = assist; }
	bool assist() const { return _Assist; }
	virtual void setAttack(bool attack) { _Attack = attack; }
	bool attack() const { return _Attack; }
private:
	bool	_Assist;
	bool	_Attack;
};

typedef std::vector<CGroupProperties> TGroupPropertiesLine;

//////////////////////////////////////////////////////////////////////////////
// CDefaultGroupProperties                                                  //
//////////////////////////////////////////////////////////////////////////////

class CDefaultGroupProperties
: public CGroupProperties
{
public:
	virtual ~CDefaultGroupProperties() { }
	// avoid modification.
	virtual void setAssist(bool	assist) { }	
	virtual void setAttack(bool	attack) { }
};

//////////////////////////////////////////////////////////////////////////////
// ICreature                                                                //
//////////////////////////////////////////////////////////////////////////////

class ICreature
: public NLMISC::CRefCount
{
public:
	typedef	std::vector<NLMISC::CSmartPtr<CFightScriptComp> > TScriptCompList;
	
public:
	virtual ~ICreature() { }
	
public:
	virtual NLMISC::CSheetId const& SheetId() const = 0;
	virtual uint32 Level() const = 0;				// Level of the creature
	
	// colors from sheet
	virtual uint8 ColorHead() const = 0;
	virtual uint8 ColorArms() const = 0;
	virtual uint8 ColorHands() const = 0;
	virtual uint8 ColorBody() const = 0;
	virtual uint8 ColorLegs() const = 0;
	virtual uint8 ColorFeets() const = 0;
	
	virtual float Radius() const = 0;				// pacs primitive 's radius
	virtual float Height() const = 0;				// pacs primitive 's height
	virtual float Width() const = 0;				// pacs primitive 's width 
	virtual float Length() const = 0;				// pacs primitive 's length
	virtual float BoundingRadius() const = 0;		// fighting radius
	
	// the entity is a bot object and cannot be traversed.
	virtual bool NotTraversable() const = 0;
	
	// the entity is a fauna, even is used as npc, it keep it's fauna name
	virtual bool ForceDisplayCreatureName() const = 0;
	
	virtual float BonusAggroHungry() const = 0;
	virtual float BonusAggroVeryHungry() const = 0;
	
	virtual float AssistDist() const = 0;
	
	virtual float AggroRadiusNotHungry() const = 0;
	virtual float AggroRadiusHungry() const = 0;
	virtual float AggroRadiusHunting() const = 0;
	
	virtual float AggroReturnDistCheck() const = 0;
	virtual float AggroRadiusD1() const = 0; // large radius, for first hit place
	virtual float AggroRadiusD2() const = 0; // small radius, for last hit place
	virtual float AggroPrimaryGroupDist() const = 0;
	virtual float AggroPrimaryGroupCoef() const = 0;
	virtual float AggroSecondaryGroupDist() const = 0;
	virtual float AggroSecondaryGroupCoef() const = 0;
	virtual float AggroPropagationRadius() const = 0;
	
	virtual AITYPES::TFaunaType FaunaType() const = 0;
	
	virtual float Scale() const = 0;				// 3d scale.
	
	virtual float DistToFront() const = 0;
	virtual float DistToBack() const = 0;
	virtual float DistToSide() const = 0;
	
	virtual float DistModulator() const = 0;	//		=	0.5f;	// (0) - (1) - (n).
	virtual float TargetModulator() const = 0;	//		=	1.f;	// (0) - (1).
	virtual float ScoreModulator() const = 0;	//		=	0.01f;	// (0) - (-1).
	virtual float FearModulator() const = 0;	//		=	0.01f;	// (0) - (1).
	virtual float LifeLevelModulator() const = 0;	//	=	0.5f;	// (0) - (1).
	virtual float CourageModulator() const = 0;	//	=	2.f;	// (-n) - (0) - (+n)		
	virtual float GroupCohesionModulator() const = 0;
	
	virtual float GroupDispersion() const = 0;	//	0.f 1.f
	
	virtual uint32 XPLevel() const = 0;
	virtual uint32 NbPlayers() const = 0;
	
	virtual uint32 EnergyValue() const = 0;
	
	virtual bool CanTurn() const = 0;
	
	virtual NLMISC::CDbgPtr<CActionList> const& FightConfig(TFightCfg fightCfg) const = 0;
	
	virtual NLMISC::CSheetId const& LeftItem() const = 0;
	virtual NLMISC::CSheetId const& RightItem() const = 0;
	
	virtual uint32 MinFightDist() const = 0;
	
	virtual uint32 FactionIndex() const = 0;
	virtual sint32 FameForGuardAttack() const = 0;
	
	virtual std::string const& AssistGroupIndexStr() const = 0;
	virtual std::string const& AttackGroupIndexStr() const = 0;
	virtual std::string const& GroupIndexStr() const = 0;
	
	virtual uint32 GroupPropertiesIndex() const = 0;
	
	/// the creature sheet can specify a multiplier that modulate the dynmaic groupe size
	virtual uint32 DynamicGroupCountMultiplier() const = 0;
	
	virtual std::string const& BotName() const = 0;
	
	virtual TScriptCompList const& ScriptCompList() const = 0;
	
	virtual TScriptCompList const& UpdateScriptList() const = 0;
	virtual TScriptCompList const& DeathScriptList() const = 0;
	virtual TScriptCompList const& BirthScriptList() const = 0;
	
	// Character Race
	virtual EGSPD::CPeople::TPeople Race() const = 0;
	
	virtual CGroupProperties const& getPropertiesCst(uint32 groupIndex) const = 0;
	
	virtual std::vector<std::string> getMultiLineInfoString() const = 0;
	
	static sint32 InvalidFameForGuardAttack;
};
typedef NLMISC::CSmartPtr<ICreature> ICreaturePtr;
typedef NLMISC::CSmartPtr<ICreature const> ICreatureCPtr;

//////////////////////////////////////////////////////////////////////////////
// CCreatureProxy                                                           //
//////////////////////////////////////////////////////////////////////////////

class CCreatureProxy
: public ICreature
{
protected:
	AISHEETS::ICreatureCPtr _Sheet;
public:
	CCreatureProxy(AISHEETS::ICreatureCPtr const& sheet)
	: _Sheet(sheet)
	{
	}
	virtual void setSheet(AISHEETS::ICreatureCPtr const& sheet) { _Sheet = sheet; }
	virtual bool isValid() const { return _Sheet!=NULL; }
	
public:
	virtual NLMISC::CSheetId const& SheetId() const { if (_Sheet) return _Sheet->SheetId(); else return NLMISC::CSheetId::Unknown; }
	virtual uint32 Level() const { return _Sheet->Level(); }
	
	virtual uint8 ColorHead() const { return _Sheet->ColorHead(); }
	virtual uint8 ColorArms() const { return _Sheet->ColorArms(); }
	virtual uint8 ColorHands() const { return _Sheet->ColorHands(); }
	virtual uint8 ColorBody() const { return _Sheet->ColorBody(); }
	virtual uint8 ColorLegs() const { return _Sheet->ColorLegs(); }
	virtual uint8 ColorFeets() const { return _Sheet->ColorFeets(); }
	
	virtual float Radius() const { return _Sheet->Radius(); }
	virtual float Height() const { return _Sheet->Height(); }
	virtual float Width() const { return _Sheet->Width(); }
	virtual float Length() const { return _Sheet->Length(); }
	virtual float BoundingRadius() const { return _Sheet->BoundingRadius(); }
	
	virtual bool NotTraversable() const { return _Sheet->NotTraversable(); }
	
	virtual bool ForceDisplayCreatureName() const { return _Sheet->ForceDisplayCreatureName(); }
	
	virtual float BonusAggroHungry() const { return _Sheet->BonusAggroHungry(); }
	virtual float BonusAggroVeryHungry() const { return _Sheet->BonusAggroVeryHungry(); }
	
	virtual float AssistDist() const { return _Sheet->AssistDist(); }
	
	virtual float AggroRadiusNotHungry() const { return _Sheet->AggroRadiusNotHungry(); }
	virtual float AggroRadiusHungry() const { return _Sheet->AggroRadiusHungry(); }
	virtual float AggroRadiusHunting() const { return _Sheet->AggroRadiusHunting(); }
	
	virtual float AggroReturnDistCheck() const { return _Sheet->AggroReturnDistCheck(); }
	virtual float AggroRadiusD1() const { return _Sheet->AggroRadiusD1(); }
	virtual float AggroRadiusD2() const { return _Sheet->AggroRadiusD2(); }
	virtual float AggroPrimaryGroupDist() const { return _Sheet->AggroPrimaryGroupDist(); }
	virtual float AggroPrimaryGroupCoef() const { return _Sheet->AggroPrimaryGroupCoef(); }
	virtual float AggroSecondaryGroupDist() const { return _Sheet->AggroSecondaryGroupDist(); }
	virtual float AggroSecondaryGroupCoef() const { return _Sheet->AggroSecondaryGroupCoef(); }
	virtual float AggroPropagationRadius() const { return _Sheet->AggroPropagationRadius(); }
	
	virtual AITYPES::TFaunaType FaunaType() const { return _Sheet->FaunaType(); }
	
	virtual float Scale() const { return _Sheet->Scale(); }
	
	virtual float DistToFront() const { return _Sheet->DistToFront(); }
	virtual float DistToBack() const { return _Sheet->DistToBack(); }
	virtual float DistToSide() const { return _Sheet->DistToSide(); }
	
	virtual float DistModulator() const { return _Sheet->DistModulator(); }
	virtual float TargetModulator() const { return _Sheet->TargetModulator(); }
	virtual float ScoreModulator() const { return _Sheet->ScoreModulator(); }
	virtual float FearModulator() const { return _Sheet->FearModulator(); }
	virtual float LifeLevelModulator() const { return _Sheet->LifeLevelModulator(); }
	virtual float CourageModulator() const { return _Sheet->CourageModulator(); }
	virtual float GroupCohesionModulator() const { return _Sheet->GroupCohesionModulator(); }
	
	virtual float GroupDispersion() const { return _Sheet->GroupDispersion(); }
	
	virtual uint32 XPLevel() const { return _Sheet->XPLevel(); }
	virtual uint32 NbPlayers() const { return _Sheet->NbPlayers(); }
	
	virtual uint32 EnergyValue() const { return _Sheet->EnergyValue(); }
	
	virtual bool CanTurn() const { return _Sheet->CanTurn(); }
	
	virtual NLMISC::CDbgPtr<CActionList> const& FightConfig(TFightCfg fightCfg) const { return _Sheet->FightConfig(fightCfg); }
	
	virtual NLMISC::CSheetId const& LeftItem() const { return _Sheet->LeftItem(); }
	virtual NLMISC::CSheetId const& RightItem() const { return _Sheet->RightItem(); }
	
	virtual uint32 MinFightDist() const { return _Sheet->MinFightDist(); }
	
	virtual uint32 FactionIndex() const { return _Sheet->FactionIndex(); }
	virtual sint32 FameForGuardAttack() const { return _Sheet->FameForGuardAttack(); }
	
	virtual std::string const& AssistGroupIndexStr() const { return _Sheet->AssistGroupIndexStr(); }
	virtual std::string const& AttackGroupIndexStr() const { return _Sheet->AttackGroupIndexStr(); }
	virtual std::string const& GroupIndexStr() const { return _Sheet->GroupIndexStr(); }
	
	virtual uint32 GroupPropertiesIndex() const { return _Sheet->GroupPropertiesIndex(); }
	
	virtual uint32 DynamicGroupCountMultiplier() const { return _Sheet->DynamicGroupCountMultiplier(); }
	
	virtual std::string const& BotName() const { return _Sheet->BotName(); }
	
	virtual TScriptCompList const& ScriptCompList() const { return _Sheet->ScriptCompList(); }
	
	virtual TScriptCompList const& UpdateScriptList() const { return _Sheet->UpdateScriptList(); }
	virtual TScriptCompList const& DeathScriptList() const { return _Sheet->DeathScriptList(); }
	virtual TScriptCompList const& BirthScriptList() const { return _Sheet->BirthScriptList(); }
	
	virtual EGSPD::CPeople::TPeople Race() const { return _Sheet->Race(); }
	
	virtual CGroupProperties const& getPropertiesCst(uint32 groupIndex) const { return _Sheet->getPropertiesCst(groupIndex); }
	
	virtual std::vector<std::string> getMultiLineInfoString() const { return _Sheet->getMultiLineInfoString(); }
};

//////////////////////////////////////////////////////////////////////////////
// CCreature                                                                //
//////////////////////////////////////////////////////////////////////////////

class CCreature
: public ICreature
{
public:
	CCreature();
	
private:
	NLMISC::CSheetId _SheetId;
	uint32 _Level;
	
	// colors from sheet
	uint8 _ColorHead;
	uint8 _ColorArms;
	uint8 _ColorHands;
	uint8 _ColorBody;
	uint8 _ColorLegs;
	uint8 _ColorFeets;
	
	float _Radius;
	float _Height;
	float _Width;
	float _Length;
	float _BoundingRadius;
	
	// the entity is a bot object and cannot be traversed.
	bool _NotTraversable;
	
	// the entity is a fauna, even is used as npc, it keep it's fauna name
	bool _ForceDisplayCreatureName;
	
	float _BonusAggroHungry;
	float _BonusAggroVeryHungry;
	
	float _AssistDist;
	
	float _AggroRadiusNotHungry;
	float _AggroRadiusHungry;
	float _AggroRadiusHunting;
	
	float _AggroReturnDistCheck;
	float _AggroRadiusD1;
	float _AggroRadiusD2;
	float _AggroPrimaryGroupDist;
	float _AggroPrimaryGroupCoef;
	float _AggroSecondaryGroupDist;
	float _AggroSecondaryGroupCoef;
	float _AggroPropagationRadius;
	
	AITYPES::TFaunaType _FaunaType;
	
	float _Scale;
	
	float _DistToFront;
	float _DistToBack;
	float _DistToSide;
	
	float _DistModulator;
	float _TargetModulator;
	float _ScoreModulator;
	float _FearModulator;
	float _LifeLevelModulator;
	float _CourageModulator;
	float _GroupCohesionModulator;
	
	float _GroupDispersion;
	
	uint32 _XPLevel;
	uint32 _NbPlayers;
	
	uint32 _EnergyValue;
	
	bool _CanTurn;
	
	NLMISC::CDbgPtr<CActionList> _FightConfig[FIGHTCFG_MAX];
	
	NLMISC::CSheetId _LeftItem;
	NLMISC::CSheetId _RightItem;
	
	uint32 _MinFightDist;
	
	uint32 _FactionIndex;
	sint32 _FameForGuardAttack;
	
	std::string _AssistGroupIndexStr;
	std::string _AttackGroupIndexStr;
	std::string _GroupIndexStr;
	
	uint32 _GroupPropertiesIndex;
	
	/// the creature sheet can specify a multiplier that modulate the dynmaic groupe size
	uint32 _DynamicGroupCountMultiplier;
	
	std::string _BotName;
	
	TScriptCompList _ScriptCompList;
	
	TScriptCompList _UpdateScriptList;
	TScriptCompList _DeathScriptList;
	TScriptCompList _BirthScriptList;
	
	// Character Race
	EGSPD::CPeople::TPeople _Race;
	
public:
	///@name ICreature implementation
	//@{
	virtual NLMISC::CSheetId const& SheetId() const { return _SheetId; }
	virtual uint32 Level() const { return _Level; }
	
	// colors from sheet
	virtual uint8 ColorHead() const { return _ColorHead; }
	virtual uint8 ColorArms() const { return _ColorArms; }
	virtual uint8 ColorHands() const { return _ColorHands; }
	virtual uint8 ColorBody() const { return _ColorBody; }
	virtual uint8 ColorLegs() const { return _ColorLegs; }
	virtual uint8 ColorFeets() const { return _ColorFeets; }
	
	virtual float Radius() const { return _Radius; }
	virtual float Height() const { return _Height; }
	virtual float Width() const { return _Width; }
	virtual float Length() const { return _Length; }
	virtual float BoundingRadius() const { return _BoundingRadius; }
	
	// the entity is a bot object and cannot be traversed.
	virtual bool NotTraversable() const { return _NotTraversable; }
	
	// the entity is a fauna, even is used as npc, it keep it's fauna name
	virtual bool ForceDisplayCreatureName() const { return _ForceDisplayCreatureName; }
	
	virtual float BonusAggroHungry() const { return _BonusAggroHungry; }
	virtual float BonusAggroVeryHungry() const { return _BonusAggroVeryHungry; }
	
	virtual float AssistDist() const { return _AssistDist; }
	
	virtual float AggroRadiusNotHungry() const { return _AggroRadiusNotHungry; }
	virtual float AggroRadiusHungry() const { return _AggroRadiusHungry; }
	virtual float AggroRadiusHunting() const { return _AggroRadiusHunting; }
	
	virtual float AggroReturnDistCheck() const { return _AggroReturnDistCheck; }
	virtual float AggroRadiusD1() const { return _AggroRadiusD1; }
	virtual float AggroRadiusD2() const { return _AggroRadiusD2; }
	virtual float AggroPrimaryGroupDist() const { return _AggroPrimaryGroupDist; }
	virtual float AggroPrimaryGroupCoef() const { return _AggroPrimaryGroupCoef; }
	virtual float AggroSecondaryGroupDist() const { return _AggroSecondaryGroupDist; }
	virtual float AggroSecondaryGroupCoef() const { return _AggroSecondaryGroupCoef; }
	virtual float AggroPropagationRadius() const { return _AggroPropagationRadius; }
	
	virtual AITYPES::TFaunaType FaunaType() const { return _FaunaType; }
	
	virtual float Scale() const { return _Scale; }
	
	virtual float DistToFront() const { return _DistToFront; }
	virtual float DistToBack() const { return _DistToBack; }
	virtual float DistToSide() const { return _DistToSide; }
	
	virtual float DistModulator() const { return _DistModulator; }
	virtual float TargetModulator() const { return _TargetModulator; }
	virtual float ScoreModulator() const { return _ScoreModulator; }
	virtual float FearModulator() const { return _FearModulator; }
	virtual float LifeLevelModulator() const { return _LifeLevelModulator; }
	virtual float CourageModulator() const { return _CourageModulator; }
	virtual float GroupCohesionModulator() const { return _GroupCohesionModulator; }
	
	virtual float GroupDispersion() const { return _GroupDispersion; }
	
	virtual uint32 XPLevel() const { return _XPLevel; }
	virtual uint32 NbPlayers() const { return _NbPlayers; }
	
	virtual uint32 EnergyValue() const { return _EnergyValue; }
	
	virtual bool CanTurn() const { return _CanTurn; }
	
	virtual NLMISC::CDbgPtr<CActionList> const& FightConfig(TFightCfg fightCfg) const { return _FightConfig[fightCfg]; }
	
	virtual NLMISC::CSheetId const& LeftItem() const { return _LeftItem; }
	virtual NLMISC::CSheetId const& RightItem() const { return _RightItem; }
	
	virtual uint32 MinFightDist() const { return _MinFightDist; }
	
	virtual uint32 FactionIndex() const { return _FactionIndex; }
	virtual sint32 FameForGuardAttack() const { return _FameForGuardAttack; }
	
	virtual std::string const& AssistGroupIndexStr() const { return _AssistGroupIndexStr; }
	virtual std::string const& AttackGroupIndexStr() const { return _AttackGroupIndexStr; }
	virtual std::string const& GroupIndexStr() const { return _GroupIndexStr; }
	
	virtual uint32 GroupPropertiesIndex() const { return _GroupPropertiesIndex; }
	
	/// the creature sheet can specify a multiplier that modulate the dynmaic groupe size
	virtual uint32 DynamicGroupCountMultiplier() const { return _DynamicGroupCountMultiplier; }
	
	virtual std::string const& BotName() const { return _BotName; }
	
	virtual TScriptCompList const& ScriptCompList() const { return _ScriptCompList; }
	
	virtual TScriptCompList const& UpdateScriptList() const { return _UpdateScriptList; }
	virtual TScriptCompList const& DeathScriptList() const { return _DeathScriptList; }
	virtual TScriptCompList const& BirthScriptList() const { return _BirthScriptList; }
	
	virtual EGSPD::CPeople::TPeople Race() const { return _Race; }
	
	virtual CGroupProperties const& getPropertiesCst(uint32 groupIndex) const;

	virtual std::vector<std::string> getMultiLineInfoString() const;
	//@}
	
public:
	void readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId);
	void serial(NLMISC::IStream& s);
	
	uint32 minFightDist() const { return MinFightDist(); }
	void calcFightAndVisualValues(std::string* left = NULL, std::string* right = NULL);
	void parseFightConfig(NLGEORGES::UForm const* form, std::string const& fightConfigString, uint32 actionListIndex, NLMISC::CDbgPtr<CActionList>& fightConfig);
	void readFightConfig(NLMISC::IStream& s, NLMISC::CDbgPtr<CActionList>& fightConfig);
	void saveFightConfig(NLMISC::IStream& s, NLMISC::CDbgPtr<CActionList>& fightConfig);
	bool mustAssist(CCreature const& creature) const;
	void setAssisGroupIndexs();
	void setAttackGroupIndexs();
	void addActionConfig(std::string const& sheetIdName, NLMISC::CDbgPtr<CActionList>& actionConfigList);
	bool addActionConfig(NLMISC::CSheetId const& sheetId, NLMISC::CDbgPtr<CActionList>& actionConfigList);
	void removed() { }
	void registerScriptComp(CFightScriptComp* scriptComp);
	
	TGroupPropertiesLine _GroupPropertiesTbl;
	
private:
	CGroupProperties& getProperties(uint32 groupIndex);
	
public:
	static uint getVersion();
	
	template <class T>
	static void getVarListFromParents(NLGEORGES::UForm const* form, std::set<T>& varList, std::string const& varName);
	
	static void getGroupStr(std::vector<uint32>& groupIndexStrList, std::string const& groupIndexStr);
};
typedef NLMISC::CSmartPtr<CCreature> CCreaturePtr;
typedef NLMISC::CSmartPtr<CCreature const> CCreatureCPtr;

//////////////////////////////////////////////////////////////////////////////
// CRaceStats                                                               //
//////////////////////////////////////////////////////////////////////////////

class IRaceStats
: public NLMISC::CRefCount
{
public:
	virtual ~IRaceStats() {}
	virtual NLMISC::CSheetId SheetId() const = 0;
	virtual std::string Race() const = 0;
};
typedef NLMISC::CSmartPtr<IRaceStats> IRaceStatsPtr;
typedef NLMISC::CSmartPtr<IRaceStats const> IRaceStatsCPtr;

class CRaceStats
: public IRaceStats
{
public:
	virtual NLMISC::CSheetId SheetId() const { return _SheetId; }
	virtual std::string Race() const { return _Race; }
	
public:
	void readGeorges(NLMISC::CSmartPtr<NLGEORGES::UForm> const& form, NLMISC::CSheetId const& sheetId);
	void serial(NLMISC::IStream& s);
	void removed() { }
	
private:
	NLMISC::CSheetId _SheetId;
	std::string _Race;
	
public:
	static uint getVersion();
};
typedef NLMISC::CSmartPtr<CRaceStats> CRaceStatsPtr;
typedef NLMISC::CSmartPtr<CRaceStats const> CRaceStatsCPtr;

//////////////////////////////////////////////////////////////////////////////
// CSheets                                                                  //
//////////////////////////////////////////////////////////////////////////////

class CSheets
{
private:
	static CSheets* _Instance;
	
public:
	static CSheets* getInstance();
	static void destroyInstance();
	
public:
	// load the creature data from the george files
	void init();
	void packSheets(const std::string &writeFilesDirectoryName);
	
	// display the creature data for all known creature types
	void display(NLMISC::CSmartPtr<CStringWriter> stringWriter, uint infoSelect = 0);
	
	//
	void release();
	
	// get a data record from the database
	ICreatureCPtr		lookup			(NLMISC::CSheetId const& id);
	CActionList const*	lookupActionList(NLMISC::CSheetId const& id);
	IAIActionCPtr		lookupAction	(NLMISC::CSheetId const& id);
	IRaceStatsCPtr		lookupRaceStats	(NLMISC::CSheetId const& id);
	
	uint32	getGroupPropertiesIndex(std::string	groupIndexName);
	
	uint32 playerGroupIndex() { return _PlayerGroupIndex; }
	
private:
	friend class CCreature;
	
private:
	// prohibit cnstructor as this is a singleton
	CSheets();
	
private:
	CDefaultGroupProperties _DefaultGroupProp;
	
	std::map<NLMISC::CSheetId, CCreaturePtr>	_Sheets;
	std::map<NLMISC::CSheetId, CActionList>		_ActionListSheets;
	std::map<NLMISC::CSheetId, CAIActionPtr>	_ActionSheets;
	std::map<NLMISC::CSheetId, CRaceStatsPtr>	_RaceStatsSheets;
	
	std::map<std::string, uint32> _NameToGroupIndex;
	
	uint32 _PlayerGroupIndex;
	
	bool _Initialised;
};

}

/****************************************************************************/
/* Inline methods                                                           */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CCreature                                                                //
//////////////////////////////////////////////////////////////////////////////

template <class T>
void AISHEETS::CCreature::getVarListFromParents(NLGEORGES::UForm const* form, std::set<T>& varList, std::string const& varName)
{
	uint32 nbParents = form->getNumParent ();
	while (nbParents>0)
	{
		--nbParents;
		NLGEORGES::UForm const* parentForm = form->getParentForm(nbParents);
		if (parentForm)
		{
			getVarListFromParents(parentForm, varList,	varName);
		}
		NLGEORGES::UFormElm const& item = form->getRootNode();
		T temp;
		if (item.getValueByName(temp, varName.c_str()))
		{
			varList.insert (temp);
		}
	}
}

#endif
