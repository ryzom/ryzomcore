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




#ifndef RYAI_ENTITY_PHYSICAL_H
#define RYAI_ENTITY_PHYSICAL_H

#include "ai_entity.h"
#include "ai_pos_mirror.h"
#include "persistent_spawnable.h"
#include "ai_share/world_map.h"
#include "server_share/combat_state.h"
#include "world_container.h"
#include "ai_entity_matrix.h"
#include "server_share/action_flags.h"
#include "combat_interface.h"

#include "world_map_link.h"

#include "knapsack_solver.h"

#include "nel/misc/variable.h"

class CAIEntityPhysical;
class CPetOwner;
class CFightFaunaProfile;

extern NLMISC::CVariable<float>	SpeedFactor;

//////////////////////////////////////////////////////////////////////////////
// CTargetable                                                              //
//////////////////////////////////////////////////////////////////////////////

/**	Targeting system
	There are both the targeter and the targetable in the same class for
	template easier manipulation reasons.
	@note Don't forget to call detachFromTargeting() in T dtor.
*/
template <class T>
class CTargetable
{
#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
	friend class CTargetable<T>;
#	endif
#endif
public:
	typedef NLMISC::CDbgPtr<T> TPtr;
//	typedef T* TPtr;
	enum TTargetType
	{
		TARGET_TYPE_FIGHT = 0,
		TARGET_TYPE_VISUAL,
		TARGET_TYPE_UNREACHABLE,
		TARGET_TYPE_MAX
	};
	
	float _AggroScore;
	uint32 _ChooseLastTime;
	
	/// @name Constructor and destructor
	//@{
	CTargetable();
	virtual	~CTargetable();
	//@}
	
	virtual	void targetDied	() { }
	
	/// @name Accessors
	//@{
	TPtr const& firstTargeter()            const { return _FirstTargeters[TARGET_TYPE_FIGHT]; }
	TPtr const& firstVisualTargeter()      const { return _FirstTargeters[TARGET_TYPE_VISUAL]; }
	TPtr const& firstUnreachableTargeter() const { return _FirstTargeters[TARGET_TYPE_UNREACHABLE]; }
	
	TPtr const& nextTargeter() const { return _NextTargeter; }
	
	uint32 targeterCount()            const { return _TargeterCount[TARGET_TYPE_FIGHT]; }
	uint32 visualTargeterCount()      const { return _TargeterCount[TARGET_TYPE_VISUAL]; }
	uint32 unreachableTargeterCount() const { return _TargeterCount[TARGET_TYPE_UNREACHABLE]; }
	uint32 totalTargeterCount() const;
	//@}
	
	/// @name Target management
	//@{
	virtual void setTarget(T* target);
	virtual void setVisualTarget(T* target);
	virtual void setUnreachableTarget(T* target);
	
	TPtr getTarget() const;
	TPtr getVisualTarget() const;
	TPtr getUnreachableTarget() const;
	
	void detachFromTargeters();
	/// Detach from targeters and target
	void detachFromTargeting();
	//@}
	
	/// @name Fight management
	//@{
	virtual float getFreeFightSpaceRatio() const { return std::max(fightTargetersFreeWeight()/fightTargetersWeightMax(), 0.f); }
	virtual float fightWeight() const { return _DefaultFightWeight; }
	virtual float fightValue() const { return _DefaultFightValue; }
	virtual float fightTargetersWeightMax() const { return _DefaultFightTargetersWeightMax; }
	virtual float fightTargetersWeight() const { return _FightTargetersWeight; }
	virtual float fightTargetersFreeWeight() const { return fightTargetersWeightMax() - fightTargetersWeight(); }
	virtual float fightTargetersValue() const { return _FightTargetersValue; }
	//@}
	
private:
	void setTarget(TTargetType type, TPtr const& target);
	TPtr getTarget(TTargetType type) const;
		
	/// @name Targeter management
	/// These functions are responsible to set/unset the actual _Target pointer in targeters
	//@{
	void linkTargeter    (TTargetType type, TPtr const& targeter, TPtr const& nextTargeter);
	void unlinkTargeter  (TTargetType type, TPtr const& targeter);
	void addTargeter     (TTargetType type, TPtr const& targeter);
	void removeTargeter  (TTargetType type, TPtr const& targeter);
	void tryToAddTargeter(TTargetType type, TPtr const& targeter);
	//@}
	
private:
	// Target stuff
	uint32 _TargeterCount[TARGET_TYPE_MAX];
	TPtr _FirstTargeters[TARGET_TYPE_MAX];
	// Targeter stuff
	TPtr _Target;
	TTargetType _TargetType;
	TPtr _NextTargeter;
	
	/// @name Fight management
	//@{
	float _FightTargetersWeight;
	float _FightTargetersValue;
	//@}
	
public:
	static CKnapsackSolver::Algorithm _TargeterChoiceAlgorithm;
private:
	static float const _DefaultFightTargetersWeightMax;
	static float const _DefaultFightWeight;
	static float const _DefaultFightValue;
};

class CPersistentOfPhysical :
	public NLMISC::CDbgRefCount<CPersistentOfPhysical>,
	public CPersistent<CAIEntityPhysical>,
	public CWorldMapLink<CPersistentOfPhysical>
{
public:
	bool isAt16MetersPos(uint16 x, uint16 y) const;
	
	/// You must overload this access to cast the objet with a custom more proper type. ( you know what StepH meant? )
	CAIEntityPhysical* getSpawnObj() const;

	/// Retrieve an info string on the entity
	virtual std::string	getOneLineInfoString() const =0;
};

typedef std::vector<NLMISC::CDbgPtr<CPersistentOfPhysical> > TPersistentList;
	
//////////////////////////////////////////////////////////////////////////////
// CAIEntityPhysical                                                        //
//////////////////////////////////////////////////////////////////////////////

// :KLUDGE: These should be in game_share
typedef uint32 TAllianceId;
typedef uint32 TAIAlias;

class IAIEntityPhysicalHealer
{
public:
	virtual void healerAdded(CAIEntityPhysical* entity) = 0;
	virtual void healerRemoved(CAIEntityPhysical* entity) = 0;
};

class CAIEntityPhysical;

class CAIEntityPhysicalLocator
{
public:
	static CAIEntityPhysicalLocator* getInstance();
private:
	static CAIEntityPhysicalLocator* _Instance;
	
public:
	CAIEntityPhysical* getEntity(TDataSetRow const& row) const;
	CAIEntityPhysical* getEntity(NLMISC::CEntityId const& id) const;
	void addEntity(TDataSetRow const& row, NLMISC::CEntityId const& id, CAIEntityPhysical* entity);
	void delEntity(TDataSetRow const& row, NLMISC::CEntityId const& id, CAIEntityPhysical* entity);
private:
	std::map<TDataSetRow, CAIEntityPhysical*> _EntitiesByRow;
	std::map<NLMISC::CEntityId, CAIEntityPhysical*> _EntitiesById;
};

/// CAIEntityPhysical is the common parent of bots, players and any other
/// physical objects that have an existence in the world.
class CAIEntityPhysical
: public NLMISC::CDbgRefCount<CAIEntityPhysical>
, public CAIEntity
, public CSpawnable<CPersistentOfPhysical>
, public CTargetable<CAIEntityPhysical>
{
public:
	/// @name Constructor and destructor
	//@{
	CAIEntityPhysical(CPersistentOfPhysical &owner, TDataSetRow const& entityIndex, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag const& AStarFlags);
	virtual ~CAIEntityPhysical();
	//@}
	
	/// @name Accessors
	//@{
	NLMISC::CEntityId	const&	getEntityId()	const { return _id; }
	float						radius()		const { return _radius; }
	CAIPosMirror		const&	pos()			const { return _pos; }
	virtual CAIPos				aipos()			const { return CAIPos(_pos); }
	CAICoord			const&	x()				const { return _pos.x(); }
	CAICoord			const&	y()				const { return _pos.y(); }
	sint32						h()				const { return _pos.h(); }
	CAngle						theta()			const { return _pos.theta(); }
	float						hpPercentage()	const { return (float)currentHitPoints()/(float)maxHitPoints(); }
	TDataSetRow			const&	dataSetRow()	const { return _dataSetRow; }
	uint32						level()			const { return _Level; }
	RYAI_MAP_CRUNCH::CWorldPosition	const& wpos() const { return _wpos; }
	void setWPos(RYAI_MAP_CRUNCH::CWorldPosition const& pos);
	RYAI_MAP_CRUNCH::TAStarFlag const& getAStarFlag() const { return _AStarFlags; }
	//@}
	
	/// @name Mirror accessors
	//@{
	TYPE_CURRENT_HIT_POINTS	currentHitPoints()		const { return _CurrentHitPoint(); }
	TYPE_MAX_HIT_POINTS		maxHitPoints()			const { return _MaxHitPoint(); }	
	TYPE_VISION_COUNTER		currentVisionCounter()	const { return _VisionCounter(); }
	bool					havePlayersAround()		const;
	
	/// Return the alias of the outpost where the bot is, or 0 if outside of an outpost
	TAIAlias				outpostAlias()			const { return _InOutpostAlias.getValue(); }
	uint8					outpostSide()			const { return _InOutpostSide.getValue(); }

	uint32					getInstanceNumber()		const { return _instanceNumber(); };
	MBEHAV::EMode			getMode()				const { return (MBEHAV::EMode)_mode().Mode; }
	MBEHAV::EBehaviour		getBehaviour()			const { return (MBEHAV::EBehaviour)_behaviour().Behaviour; }
	bool					isAlive()				const { return getMode()!=MBEHAV::DEATH; }
	
	RYZOMACTIONFLAGS::TActionFlag getActionFlags()	const { return (RYZOMACTIONFLAGS::TActionFlag)_ActionFlags(); }
	void setActionFlags(RYZOMACTIONFLAGS::TActionFlag const& flag);
	void removeActionFlags(RYZOMACTIONFLAGS::TActionFlag const& flag);
	//@}
	
	/// @name Virtual accessors
	//@{
	virtual	bool isBotAttackable() const = 0;

	/// The returned type can be different from the type in the CEntityId (ex: pack_animal instead of creature for all player's mektoubs)
	virtual	RYZOMID::TTypeId getRyzomType() const = 0;
	virtual float getCollisionDist(float angTo) const;
	//@}
	
	/// @name Fighting
	//@{
	virtual void processEvent(CCombatInterface::CEvent const& event) = 0;
	//@}
	
	/// @name Effects
	//@{
	//	Food
	float& food() { return _food; }
	
	// Stun cast
	sint32& stun() { return _Stuned; }
	bool isStuned() const { return _Stuned!=0; }
	
	sint32& root() { return _Rooted; }
	bool isRooted() const { return _Rooted!=0; }
	
	sint32& blind() { return _Blinded; }
	bool isBlinded() const { return _Blinded!=0; }
	
	sint32& fear() { return _Feared; }
	bool isFeared() const { return _Feared!=0; }
	//@}
	
	/// @name Movement
	//@{
	virtual bool canMove() const;
	float walkSpeed() const;
	float runSpeed() const;
	//@}
	
	/// @name Healer count management
	//@{
	virtual void addHealer(IAIEntityPhysicalHealer* healer) { _Healers.insert(healer); if (healer) healer->healerAdded(this); }
	virtual void delHealer(IAIEntityPhysicalHealer* healer) { _Healers.erase(healer); if (healer) healer->healerRemoved(this); }
	virtual sint getHealerCount() { return (sint)_Healers.size(); }
	//@}
	
	static int _PlayerVisibilityDistance;

	virtual sint32 getFame(std::string const& faction, bool modulated = false, bool returnUnknowValue = false) const;
	virtual sint32 getFameIndexed(uint32 factionIndex, bool modulated = false, bool returnUnknowValue = false) const;

protected:
	virtual float getSpeedFactor() const { return 1.f; }
	
protected:
	friend	class CFightFaunaProfile;
	
	// position and orientation (only changeable by bots).
	CAIPosMirror _pos;

private:
	
	//	done to hide access except for CModEntityPhysical ..
	friend	class	CModEntityPhysical;

	// entity index	- for MIRRORS
	TDataSetRow	_dataSetRow;
	
	RYAI_MAP_CRUNCH::CWorldPosition	_wpos;

	// instance number
	CMirrorPropValue<uint32>				_instanceNumber;

	// generic visual properties
	CMirrorPropValue<MBEHAV::TMode>			_mode;
	CMirrorPropValueRO<MBEHAV::CBehaviour>	_behaviour;
	CMirrorPropValue<TYPE_TARGET_ID>		_targetRow;

	CMirrorPropValueRO<float>	_RunSpeed;
	CMirrorPropValueRO<float>	_WalkSpeed;

	CMirrorPropValueRO<TYPE_CURRENT_HIT_POINTS>	_CurrentHitPoint;
	CMirrorPropValueRO<TYPE_MAX_HIT_POINTS>		_MaxHitPoint;
	CMirrorPropValueRO<TYPE_VISION_COUNTER>		_VisionCounter;
	CMirrorPropValue<TYPE_IN_OUTPOST_ZONE_ALIAS> _InOutpostAlias;
	CMirrorPropValue<TYPE_IN_OUTPOST_ZONE_SIDE> _InOutpostSide;

	/// flags used by AI service to know the state of the entity
	CMirrorPropValue<uint16>	_ActionFlags;
	
	sint32	_Stuned;	//	Is the bot stuned ?
	sint32	_Rooted;	//	Is the bot rooted ?
	sint32	_Blinded;	//	Is the bot blinded ?
	sint32	_Feared;	//	Is the bot Feared ?
	
	NLMISC::CEntityId	_id;
	float	_radius;
	float	_food;

	uint32	_Level;

	RYAI_MAP_CRUNCH::TAStarFlag	_AStarFlags;
	
	std::multiset<IAIEntityPhysicalHealer*>	_Healers;
};

//////////////////////////////////////////////////////////////////////////////
// CModEntityPhysical                                                       //
//////////////////////////////////////////////////////////////////////////////

class CModEntityPhysical
: public NLMISC::CDbgRefCount<CModEntityPhysical>
, public CAIEntityPhysical
{
public:
	/// @name Constructor
	//@{
	CModEntityPhysical(CPersistentOfPhysical& owner, TDataSetRow const& entityIndex, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag const& AStarFlags);
	//@}
	
	/// @name Accessors (setters)
	//@{
	virtual void setTheta(CAngle theta) { _pos.setTheta(theta); }
	void setMode(MBEHAV::EMode m);
	void setInstanceNumber(uint32 instanceNumber) { _instanceNumber = instanceNumber; };
	void setBehaviour(MBEHAV::EBehaviour b) { CMirrors::setBehaviour(dataSetRow(), b); }
	
	/// The AIS may set only the alias of the outpost where a bot is, or a character the EGS sets is
	void setOutpostAlias(TAIAlias alias) { _InOutpostAlias = alias; }
	/// 
	void setOutpostSide(OUTPOSTENUMS::TPVPSide side) { _InOutpostSide = (side==OUTPOSTENUMS::OutpostAttacker); }
	//@}
	
	/// @name Targeting overrides
	//@{
	virtual void targetDied() { _targetRow = TDataSetRow(); }
	virtual void setTarget(CAIEntityPhysical* target);
	virtual void setVisualTarget(CAIEntityPhysical* target);
	virtual void setUnreachableTarget(CAIEntityPhysical* target);
	//@}
	
	/// @name Movement
	//@{
	/// Fast routine (but you must ensure that pos and wpos are related)
	void setPos(CAIPos const& pos, RYAI_MAP_CRUNCH::CWorldPosition const& wpos);
	/// Slow routine
	bool setPos(CAIPos const& pos);
	/// Set position
	bool moveTo(CAIPos const& newPos, RYAI_MAP_CRUNCH::TAStarFlag const& denyFlags);
	/// Change position
	/// If this method is extracted from the class definition VC++ fails to instantiate it.
	template <class W>
	bool moveBy(W vect, RYAI_MAP_CRUNCH::TAStarFlag const& denyFlags)
	{
		if (!wpos().isValid())
			return false;
		
		CAIVector posVect(pos());
		posVect += vect;
		
		//	first we try from the real position (not bound).
		CAIPos destPos(posVect, h(), theta());
		return moveTo(destPos, denyFlags);
	}
	
	/// Calculate the repulsion with other bots and players
	CAIVector calcRepulsion(CAIPos const& pos) const;
	bool calcStraightRepulsion(CAIPos const& pos, CAIVector& repulsion) const;
	
	void setMoveDecalage(CAIVector const& decalage) { _Decalage = decalage; }
	CAIVector const& moveDecalage() const { return _Decalage; }
	void resetDecalage();
	//@}

private:
	CAIVector calcRepulsionFrom(CAIVector const& pos, const std::vector<const CAIEntityPhysical*>& entities) const;
	bool calcStraightRepulsionFrom(CAIVector const& pos, const std::vector<const CAIEntityPhysical*>& entities, CAIVector& repulsion) const;

private:
	CAIVector _Decalage;
};

#endif
