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

#ifndef RYAI_BOT_H
#define RYAI_BOT_H

#include "nel/misc/path.h"
#include "ai_entity_physical.h"
#include "ai_entity_physical_inline.h"
#include "ai_instance.h"
#include "ai_pos.h"
#include "states.h"
#include "debug_history.h"
#include "sheets.h"
#include "profile.h"
#include "ai_aggro.h"
#include "dyn_grp.h"
#include "nel/misc/variable.h"
#include "service_dependencies.h"
#include "game_share/timer.h"
#include "fx_entity_manager.h"

class CGroup;

template <class T> class CSpawnable;
template <class T> class CPersistent;

void sAggroLost(TDataSetRow playerBot, TDataSetRow targetBot);
void sAggroGain(TDataSetRow playerBot, TDataSetRow targetBot);

class CSpawnGroup;

class CSpawnGroup;

extern CAIVector lastTriedPos;

//////////////////////////////////////////////////////////////////////////////
// CSpawnBot                                                                //
//////////////////////////////////////////////////////////////////////////////

class CSpawnBot
: public NLMISC::CDbgRefCount<CSpawnBot>
, public CModEntityPhysical
, public CProfileOwner
, public CProfilePtr
, public CBotAggroOwner
, public CDynSpawnBot
{
public:
	typedef CHashMap<size_t, uint32> TPropList;
	
public:
	
	CSpawnBot(TDataSetRow const& entityIndex, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	virtual ~CSpawnBot();
	
	virtual void setTheta(CAngle theta);
	
	virtual void sendInfoToEGS() const;
	
	CBot& getPersistent() const;
	
	CSpawnGroup& spawnGrp() const;
	
	/// @name CBotAggroOwner implementation
	//@{
	virtual void aggroLost(TDataSetRow const& aggroBot) const;
	virtual void aggroGain(TDataSetRow const& aggroBot) const;
	
	virtual NLMISC::CEntityId getAggroOwnerEid() const { return getEntityId(); }
	virtual RYAI_MAP_CRUNCH::CWorldPosition getAggroPos() const { return wpos(); }
	virtual NLMISC::CSmartPtr<CAIPlace const> buildFirstHitPlace(TDataSetRow const& aggroBot) const;
	virtual std::set<CBotAggroOwner*> getAggroGroup(bool primary) const;
	
	virtual float getReturnDistCheck() const;
	virtual float getD1Radius() const;
	virtual float getD2Radius() const;
	virtual float getPrimaryGroupAggroDist() const;
	virtual float getPrimaryGroupAggroCoef() const;
	virtual float getSecondaryGroupAggroDist() const;
	virtual float getSecondaryGroupAggroCoef() const;
	virtual float getAggroPropagationRadius() const;
	//@}
	
	virtual void setVisualPropertiesName();
	
	// as there not a lot of prop (1 or 2, maybe 3) stores in this comportment, we don't need hash.
	bool getProp(size_t Id, uint32& value) const;
	void setProp(size_t Id, uint32 value);
	
	virtual float fightWeight() const;
	virtual float fightValue() const;
	
	CAIInstance* getAIInstance() const;
	std::vector<std::string> getMultiLineInfoString() const;
		
	void setSpawnGroup(CSpawnGroup* spawnGroup) { _SpawnGroup = spawnGroup; }
	
	bool isHitting() const { return (getActionFlags()&RYZOMACTIONFLAGS::Attacks)!=0; }

	virtual void sheetChanged();
	
	bool canHeal();
	bool canSelfHeal();
	void healTriggered();
	void selfHealTriggered();
	
	virtual float getSpeedFactor() const { return _SpeedFactor; }
	virtual void setSpeedFactor(float value) { _SpeedFactor = value; }
	
public:
	TPropList _PropList;
	
	float _DamageSpeedCoef;
	float _DamageCoef;
	
private:
	CSpawnGroup* _SpawnGroup;
	
private:
	uint32	_LastHealTick;
	uint32	_LastSelfHealTick;
	
	float	_SpeedFactor;
};

//////////////////////////////////////////////////////////////////////////////
// CBot                                                                     //
//////////////////////////////////////////////////////////////////////////////

class CBot
: public CAliasChild<CGroup>
, public CPersistentOfPhysical
, public CAIEntity
, public CDebugHistory
, public NLMISC::CRefCount
, public CDynBot
, public CServiceEvent::CHandler
{
	friend class CGroup;

public:
	/// @name Ctors and dtor
	//@{
	CBot(CGroup* owner, CAIAliasDescriptionNode* alias = NULL);
	CBot(CGroup* owner, uint32 alias, std::string const& name);
	virtual ~CBot();
	//@}
	
	/// @name CChild implementation
	//@{
	virtual std::string getIndexString() const;
	virtual std::string getEntityIdString() const;
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	virtual std::string getFullName() const;
	//@}
	
	/// @name Sheet management
	//@{
	virtual AISHEETS::ICreatureCPtr getSheet() const { return _Sheet; }
	virtual bool isSheetValid() const { return !_Sheet.isNull(); }
	virtual void setSheet(AISHEETS::ICreatureCPtr const& sheet);
	
	virtual void triggerSetSheet(AISHEETS::ICreatureCPtr const& sheet);
	//@}
	
	/// @name AI objects hierarchy access
	//@{
	CAIInstance* getAIInstance() const;
	CGroup& getGroup() { return *getOwner(); }
	CAliasTreeOwner const* getAliasTreeOwner() const { return this; }
	CSpawnBot* getSpawnObj() const;
	//@}
	
	/// @name CDynBot stuff
	//@{
	void addEnergy() const;
	void removeEnergy() const;
	CDynBot const& getDynBot() const { return *this; };
	void initEnergy(float energyCoef);
	//@}

	/// @name Observers management
	//@{
	class IObserver
	{
	public:
		virtual void notifyBotDespawn(CBot* bot) {}
		virtual void notifyBotDeath(CBot* bot) {}
		virtual void notifyStopNpcControl(CBot* bot) {}
	};
	void attachObserver(IObserver* obs);
	void detachObserver(IObserver* obs);

	void notifyBotDespawn();
	void notifyBotDeath();
	void notifyStopNpcControl();
	//@}

	/// Debugging stuff
	CDebugHistory* getDebugHistory() { return this; }
	
	void serviceEvent(CServiceEvent const& info);
	
	virtual AITYPES::TFaunaType type() const { return AITYPES::FaunaTypeBadType; }
	virtual	RYZOMID::TTypeId getRyzomType() const = 0;
	
	virtual void getSpawnPos(CAIVector& triedPos, RYAI_MAP_CRUNCH::CWorldPosition& pos, RYAI_MAP_CRUNCH::CWorldMap const& worldMap, CAngle& spawnTheta) = 0;
	
	virtual CAIS::CCounter& getSpawnCounter() = 0;
	
	virtual bool spawn();
	virtual void despawnBot();
	virtual bool reSpawn(bool sendMessage = true);
	
	bool isStuck() const { return _Stuck; }
	void setStuck(bool value) { _Stuck = value; }
	
	bool isBuildingBot() const { return _BuildingBot; }
	void setBuildingBot(bool value) { _BuildingBot = value; }
	
	bool ignoreOffensiveActions() const { return _IgnoreOffensiveActions; }
	void setIgnoreOffensiveActions(bool value) { _IgnoreOffensiveActions = value; }
	
	bool isHealer() const;
	void setHealer(bool value) { _Healer = value; }
	
	NLMISC::CEntityId createEntityId() const;
	
	const ucstring& getCustomName() const { return _CustomName; }
	void setCustomName(const ucstring &name) { _CustomName = name; }

	const uint32& getCustomMaxHp() const { return _CustomMaxHp; }
	void setCustomMaxHp(const uint32 &maxHp) { _CustomMaxHp = maxHp; }

	virtual void setClientSheet(const std::string & clientSheetName);  

	// Can be redefine by NpcGroup in case of a BotNpc with a fauna sheet but that we don't want the name to ignore
	// Especialy for Ring Creatures
	virtual bool getFaunaBotUseBotName() const { return false;} 

protected:
	virtual CSpawnBot* getSpawnBot(TDataSetRow const& row, NLMISC::CEntityId const& id, float radius) = 0;
	
	virtual void sheetChanged();
	bool finalizeSpawn(RYAI_MAP_CRUNCH::CWorldPosition const& botWPos, CAngle const& spawnTheta, float botMeterSize);
	virtual void initAdditionalMirrorValues() {}
	void setSheetDelayed(uint32 step);
	void setSheetDelayed0();
	void setSheetDelayed1();
	void setSheetDelayed2();
	friend class CSetSheetTimerEvent;		
	
protected:
	AITYPES::TVerticalPos _VerticalPos;
	
private:
	AISHEETS::ICreatureCPtr _Sheet;
	NLMISC::CSheetId		_ClientSheet;
	bool _Stuck;
	bool _IgnoreOffensiveActions;
	bool _Healer;
	bool _BuildingBot;
	ucstring _CustomName;
	uint32 _CustomMaxHp;
	CTimer					_SetSheetTimer;
	struct CSetSheetData
	{
		AISHEETS::ICreatureCPtr	_SheetToSet;
		NLMISC::CSheetId		_FxSheetId;
		CFxEntityPtr			_Fx;
	};
	CSetSheetData*				_SetSheetData;
	std::vector<IObserver*>*	_Observers;
	
public:
	void* _ProfileData;
};

#endif
