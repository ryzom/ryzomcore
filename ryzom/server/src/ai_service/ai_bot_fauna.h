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

#ifndef RYAI_BOT_FAUNA_H
#define RYAI_BOT_FAUNA_H

#define FAUNA_BEHAVIOR_GLOBAL_SCALE (4)

#include "ai_bot.h"
#include "timer.h"
#include "path_behaviors.h"

class CFauna;
class CSpawnGroupFauna;
class CBotPlayer;
class CBotFauna;
class CGrpFauna;
class CMgrFauna;

class CGeneralFaunaUpdate;

//////////////////////////////////////////////////////////////////////////////
// CFaunaActivity                                                           //
//////////////////////////////////////////////////////////////////////////////

class CFaunaActivity
{
public:
	virtual ~CFaunaActivity() { }
	
	enum TCycleState
	{
		// eat behaviour cycles.
		CycleStateHungry = 0,
		CycleStateVeryHungry,
		CycleStateStarving,
		CycleStateDigesting,
		
		// sleep behaviour cycles.
		CycleStateTired,
		CycleStateVeryTired,
		CycleStateExhausted,
		CycleStateShaking,
		
		CycleStateUndefined
	};
	
	virtual void update() = 0;
	virtual AITYPES::TProfiles getActivityType() = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotFauna                                                           //
//////////////////////////////////////////////////////////////////////////////

class CSpawnBotFauna
: public CSpawnBot
{
public:
	CSpawnBotFauna(TDataSetRow const& row, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	virtual	~CSpawnBotFauna();
	
	CSpawnGroupFauna& spawnGrp();
	
	void update(AITYPES::TProfiles	activity, uint32 ticksSinceLastUpdate);
	
	bool canMove() const;
	
	//////////////////////////////////////////////////////////////////////////
	//	Specific
	//////////////////////////////////////////////////////////////////////////	
	
	CFaunaActivity::TCycleState const& cycleState() const { return _CycleState; }
	void setCycleState(CFaunaActivity::TCycleState const& cycleState) { _CycleState=cycleState; }
	
	//////////////////////////////////////////////////////////////////////////
	//	Fight Features.
	//////////////////////////////////////////////////////////////////////////
	
	void getBestTarget();
	
	void processEvent(CCombatInterface::CEvent const& event);
	void eventEngaged(TDataSetRow const& originId);
	
	/// @name Profiles transition
	//@{
	void doFight(CAIEntityPhysical* ennemy);
	void setDefaultComportment();
	//@}
	
	
	float aggroRadius();
	
	float getCollisionDist(float angTo)	const;
	
	void setHungry() { _Hungry = 1.f; }
	float& hungry() { return _Hungry; }
	
	// fauna are always attackable by other bots
	virtual	bool isBotAttackable() const  { return true; }
	
	//////////////////////////////////////////////////////////////////////////
	//	Specific ( Comportement )
	//////////////////////////////////////////////////////////////////////////	
	
	//----------------------------------------------------------------------------------------------
	// Dispatching message to EGS to describe chat possibilities
	//----------------------------------------------------------------------------------------------
	
	void sendInfoToEGS() const;
	
	CBotFauna& getPersistent() const;
	
	// replace by	'getRYZOMType'
	virtual RYZOMID::TTypeId getRyzomType() const { return RYZOMID::creature; }
	
	//////////////////////////////////////////////////////////////////////////
	//	Specific ( Comportement )
	//////////////////////////////////////////////////////////////////////////
		
	CFaunaActivity::TCycleState	_CycleState;
	
	CAITimer& timeBeforeNextCuriosity() { return _TimeBeforeNextCuriosity; }
	
	CAITimer& VisualTargetTimer() { return _VisualTargetTimer; }

	float getReturnDistCheck() const;
	
private:
	float	_Hungry;
	float	_Food;
	CAITimer	_TimeBeforeNextCuriosity;
	CAITimer	_VisualTargetTimer;
	CAITimer	_NextBestTargetUpdate;
};

//////////////////////////////////////////////////////////////////////////////
// CMovementMagnet                                                           //
//////////////////////////////////////////////////////////////////////////////

class CMovementMagnet
: public NLMISC::CRefCount
{
public:
	enum TMovementType
	{
		Movement_Anim = 0,
		Movement_Wait_Anim,
		Movement_Move
	};
	
	CMovementMagnet(CSpawnBotFauna& botFauna, RYAI_MAP_CRUNCH::TAStarFlag flag);
	virtual ~CMovementMagnet();
	virtual void update(uint32 waitTime, uint32 ticksSinceLastUpdate) { update(waitTime, ticksSinceLastUpdate, false); }
	void setBotAngle();
	
	virtual void getNewDestination(RYAI_MAP_CRUNCH::CWorldPosition const& alternativePos, RYAI_MAP_CRUNCH::TAStarFlag denyFlag);
	CAIVector const& getDestination() const;
	bool isDestinationValid() const;
	TMovementType getMovementType() const { return _State; }
	
	CAITimer& stateTimer() { return _StateTimer; }
	
	void setState(TMovementType state) { _State = state; }
	
protected:
	void update(uint32 waitTime, uint32 ticksSinceLastUpdate, bool ignoreBadPos);
	CPathCont		_PathCont;
private:
	float	_Speed;
	CAngle	_dTheta;		// the change in orientation at last update
	CAITimer	_dThetaTimer;	// the time for which the current rotation will be applied	
	
	CAITimer	_StateTimer;
	TMovementType	_State;

	RYAI_MAP_CRUNCH::TAStarFlag _denyFlags;
	
	CSpawnBotFauna&	_BotFauna;
	CPathPosition	_PathPos;
	CAIVector		_LastDest;
};

//////////////////////////////////////////////////////////////////////////////
// CReturnMovementMagnet                                                    //
//////////////////////////////////////////////////////////////////////////////

class CReturnMovementMagnet
: public CMovementMagnet
{
public:
	CReturnMovementMagnet(RYAI_MAP_CRUNCH::CWorldPosition const& forcedDest, CSpawnBotFauna& botFauna, RYAI_MAP_CRUNCH::TAStarFlag flag);
	virtual void getNewDestination(RYAI_MAP_CRUNCH::CWorldPosition const& alternativePos, RYAI_MAP_CRUNCH::TAStarFlag denyFlag);
	virtual void update(uint32 waitTime, uint32 ticksSinceLastUpdate) { CMovementMagnet::update(waitTime, ticksSinceLastUpdate, true); }
	
private:
	RYAI_MAP_CRUNCH::CWorldPosition	_ForcedDest;
};

//////////////////////////////////////////////////////////////////////////////
// IMouvementMagnetOwner                                                    //
//////////////////////////////////////////////////////////////////////////////

class IMouvementMagnetOwner
{
public:
	virtual NLMISC::CSmartPtr<CMovementMagnet> const& getMovementMagnet() const = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CBotFaunaSheet                                                           //
//////////////////////////////////////////////////////////////////////////////

class CBotFaunaSheet
: public AISHEETS::CCreatureProxy
{
public:
	CBotFaunaSheet(AISHEETS::ICreatureCPtr const& sheet)
	: AISHEETS::CCreatureProxy(sheet)
	, _AggroRadiusNotHungry(0.f)
	, _AggroRadiusHungry(0.f)
	, _AggroRadiusHunting(0.f)
	{
		reset();
	}
	
	virtual void setSheet(AISHEETS::ICreatureCPtr const& sheet)
	{
		CCreatureProxy::setSheet(sheet);
		reset();
	}
	
	///@name ICreature overloads
	//@{
	virtual float AggroRadiusNotHungry() const { return _AggroRadiusNotHungry; }
	virtual float AggroRadiusHungry() const { return _AggroRadiusHungry; }
	virtual float AggroRadiusHunting() const { return _AggroRadiusHunting; }
	//@}
	
	///@name Setters
	//@{
	void setAggroRadiusNotHungry(float val) { _AggroRadiusNotHungry = val; }
	void setAggroRadiusHungry(float val) { _AggroRadiusHungry = val; }
	void setAggroRadiusHunting(float val) { _AggroRadiusHunting = val; }
	//@}
	
	void reset()
	{
		if (_Sheet)
		{
			_AggroRadiusNotHungry = _Sheet->AggroRadiusNotHungry();
			_AggroRadiusHungry = _Sheet->AggroRadiusHungry();
			_AggroRadiusHunting = _Sheet->AggroRadiusHunting();
		}
	}
	
private:
	float	_AggroRadiusNotHungry;
	float	_AggroRadiusHungry;
	float	_AggroRadiusHunting;
};
typedef NLMISC::CSmartPtr<CBotFaunaSheet> CBotFaunaSheetPtr;
typedef NLMISC::CSmartPtr<CBotFaunaSheet const> CBotFaunaSheetCPtr;

//////////////////////////////////////////////////////////////////////////////
// CBotFauna                                                                //
//////////////////////////////////////////////////////////////////////////////

class CBotFauna
: public NLMISC::CDbgRefCount<CBotFauna>
, public CBot
{
public:
	CBotFauna(AITYPES::TFaunaType type, CGroup* owner, CAIAliasDescriptionNode* alias = NULL);
	
	virtual ~CBotFauna();
	
	AITYPES::TFaunaType faunaType()	const { return _Type; }
	
	CSpawnBot* getSpawnBot(TDataSetRow const& row, NLMISC::CEntityId const& id, float radius);
	
	CAIS::CCounter& getSpawnCounter();
	
	virtual bool spawn();
	void despawnBot();
	bool reSpawn(bool sendMessage = true);
		
	CSpawnBotFauna*	getSpawn() { return static_cast<CSpawnBotFauna*>(getSpawnObj()); }
	
	RYZOMID::TTypeId getRyzomType() const { return RYZOMID::creature; }
		
	CGrpFauna& grp() const;
	CMgrFauna& mgr() const;
	
	//	(assuming targetPlace is always valid ..)
	void getSpawnPos(CAIVector& triedPos, RYAI_MAP_CRUNCH::CWorldPosition& pos, RYAI_MAP_CRUNCH::CWorldMap const& worldMap, CAngle& spawnTheta);
	
	virtual std::string	getOneLineInfoString() const;
	
	virtual AISHEETS::ICreatureCPtr getSheet() const { return _Sheet.getPtr(); }
	virtual void setSheet(AISHEETS::ICreatureCPtr const& sheet)
	{
		_Sheet->setSheet(sheet);
		sheetChanged();
	}
	virtual bool isSheetValid() const
	{
		return _Sheet!=NULL && _Sheet->isValid();
	}
	
	void setAggroRadiusNotHungry(float val) { _Sheet->setAggroRadiusNotHungry(val); }
	void setAggroRadiusHungry(float val) { _Sheet->setAggroRadiusHungry(val); }
	void setAggroRadiusHunting(float val) { _Sheet->setAggroRadiusHunting(val); }
	void resetAggroRadius() { _Sheet->reset(); }
	
	void triggerSetSheet(AISHEETS::ICreatureCPtr const& sheet);
	
protected:
	virtual void sheetChanged();
	bool finalizeSpawnFauna();
	
private:
	AITYPES::TFaunaType	_Type;
	CAIPos				_SpawnPos;
	CBotFaunaSheetPtr	_Sheet;
};

#endif
