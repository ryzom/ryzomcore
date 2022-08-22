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




#ifndef RY_COMBAT_PHRASE_H
#define RY_COMBAT_PHRASE_H

// game share
#include "ai_share/ai_event_report.h"
#include "game_share/body.h"
#include "game_share/ecosystem.h"
#include "game_share/season.h"
#include "game_share/people.h"
#include "game_share/brick_flags.h"
#include "ai_share/ai_aiming_type.h"
//
#include "phrase_manager/s_phrase.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "combat_attacker.h"
#include "combat_defender.h"
#include "combat_action.h"
#include "phrase_manager/area_effect.h"

class CStaticAiAction;

// struct used to store the aimed slot
struct CAimedSlot
{
	CAimedSlot() : BodyType(BODY::UnknownBodyType), Slot(SLOT_EQUIPMENT::UNDEFINED)
	{}

	/// get factor according to refValue
	inline float applyFactor(uint16 refValue)
	{
		if (PowerValue == 0) return 0.0f;
		if (PowerValue>=refValue) return 1.0f; // so refValue cannot be = 0 as PowerValue is uint
		return (1.0f * float(PowerValue)/refValue  );
	}

	uint16							PowerValue;
	BODY::TBodyType					BodyType;
	SLOT_EQUIPMENT::TSlotEquipment	Slot;
};

// struct used to store the aimed slot for AIActions
struct CAIAimedSlot
{
	CAIAimedSlot() : AiAimingType(AI_AIMING_TYPE::Random)
	{}

	AI_AIMING_TYPE::TAiAimingType	AiAimingType;
};


// dynamic modifier
template <class T, sint32 initValue> class CDynValue
{
public:
	CDynValue(): PowerValue(0),
					MinValue( (T)initValue),
					MaxValue( (T)initValue)
	{}

	/// get apply value according to refValue
	inline T applyValue(uint16 refValue)
	{
		if (PowerValue == 0) return MinValue;
		if (PowerValue>=refValue) return MaxValue; // so refValue cannot be = 0 as PowerValue is uint
		return T( MinValue + (MaxValue - MinValue) * float(PowerValue)/refValue  );
	}

	uint16	PowerValue;
	T		MinValue;
	T		MaxValue;
};

typedef CDynValue<float, 1> CDynFactor;

// struct used for specific additional damage
struct CDamageFactor : public CDynFactor
{
	/// ctor
	CDamageFactor() : CDynFactor (),
						Classification(EGSPD::CClassificationType::EndClassificationType),
						Race(EGSPD::CPeople::EndPeople),
						Season(EGSPD::CSeason::EndSeason),
						Ecosystem(ECOSYSTEM::unknown)
	{}

	/// test compatiblity with entity
	bool entityMatchRequirements(CEntityBase *entity);


	EGSPD::CSeason::TSeason				Season;
	ECOSYSTEM::EECosystem			Ecosystem;
	EGSPD::CClassificationType::TClassificationType	Classification;
	EGSPD::CPeople::TPeople				Race;
};

// struct used for armor absorption modifier
struct CArmorAborption : public CDynFactor
{
	/// ctor
	CArmorAborption() : CDynFactor(), ArmorType(ARMORTYPE::UNKNOWN), CreatureType(EGSPD::CClassificationType::EndClassificationType)
	{}

	// homins armor type
	ARMORTYPE::EArmorType			ArmorType;
	// creature armor
	EGSPD::CClassificationType::TClassificationType	CreatureType;
};


/**
 * Specialized phrase for combat actions
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatPhrase : public CSPhrase
{
public:
	struct TTargetInfos
	{
		TTargetInfos()
			: Target(NULL), Distance(0.f), DamageFactor(1.f), DodgeFactor(1.f), InflictedNaturalDamage(0), InflictedDamage(0), NbParryDodgeFlyingTextRequired(0)
		{
		}

		CCombatDefenderPtr	Target;
		float				Distance;
		float				DamageFactor;
		float				DodgeFactor;
		sint32				InflictedNaturalDamage;
		sint32				InflictedDamage;
		sint32				NbParryDodgeFlyingTextRequired;
	};

public:
	/// default Constructor
	CCombatPhrase() { init(); _Attacker = NULL; }

	/// destructor
	virtual ~CCombatPhrase();

	/// build the phrase
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true );

	/// build the phrase from an ai action
	virtual bool initPhraseFromAiAction( const TDataSetRow & actorRowId, const CStaticAiAction *aiAction, float damageCoeff, float speedCoeff );

	/**
	 * set the primary target
	 * \param entityId id of the primary target
	 */
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId )
	{
//		_TargetRowId = entityRowId;
	}

	/**
	 * evaluate phrase
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate();

	/**
	 * validate phrase
	 * \return true of the phrase is valid
	 */
	virtual bool validate();

	/**
	 * update method
	 */
	virtual bool update();

	/**
	 * execute this phrase
	 */
	virtual void execute();

	/**
	 * launch method, called at the end of the execution
	 */
	virtual bool launch();

	/**
	 * apply method, called at the end of the execution, starts latency
	 */
	virtual void apply();

	/**
	 * called at the end of the latency time
	 */
	virtual void end();

	/**
	 * called when brutally ends the action (before latency ends)
	 */
	virtual void stop();

	/**
	 * set attacker
	 * \param attacker
	 */
//	inline void setAttacker( CCombatAttacker* attacker) { _Attacker = attacker; }


	/// set the disengageOnEnd flag
	inline void disengageOnEnd(bool flag) { _DisengageOnEnd = flag; }
	/// get the disengageOnEnd flag
	inline bool disengageOnEnd() const { return _DisengageOnEnd; }

	/// get PhraseSuccessDamageFactor
//	inline float getPhraseSuccessDamageFactor() const { return _PhraseSuccessDamageFactor; }

	/// set root sheet id for database entry
	inline void setRootSheetId( const NLMISC::CSheetId &id) { _RootSheetId = id; }

	/// get attacker weapon sabrina value
	inline uint16 weaponSabrinaValue() const { return _WeaponSabrinaValue; }

	/// get damage
	inline sint32 damage(uint8 index) const
	{
		if (index < _Targets.size())
			return _Targets[index].InflictedDamage;
		else
			return 0;
	}

	/// get entity base damage after armor (damage without bonus modifiers)
	inline sint32 getInflictedNaturalDamage(uint8 index)
	{
		if (index < _Targets.size())
			return _Targets[index].InflictedNaturalDamage;
		else
			return 0;
	}

	/// get attacker
	inline const CCombatAttacker* getAttacker() const { return _Attacker; }

	/// get targets
	inline const std::vector<TTargetInfos> & getTargets()
	{
		return _Targets;
	}

	/// get target
	inline const CCombatDefenderPtr &getTarget(uint8 index)
	{
		static CCombatDefenderPtr nullCombatDefender;

		if (index < _Targets.size())
			return _Targets[index].Target;
		else
			return nullCombatDefender;
	}

	/// get target dodge success factor, >=1 means a full success
	inline float getTargetDodgeFactor(uint8 index)
	{
		if (index < _Targets.size())
			return _Targets[index].DodgeFactor;
		else
			return true;
	}

	/// get actor execution behaviour
	inline MBEHAV::CBehaviour &getExecutionBehaviour() { return _Behaviour; }

	/// get hit localisation (used by some actions)
	inline SLOT_EQUIPMENT::TSlotEquipment getHitLocalisation() const { return _HitLocalisation; }

	// BRIANCODE Added to support mission evaluation of combat bricks
	virtual void setBrickSheets( const std::vector<NLMISC::CSheetId> & bricks)
	{
		_BrickSheets = bricks;
	}

protected:
	struct TApplyAction
	{
		CCombatDefenderPtr					Target;
		bool								MainTarget;
		sint16								DeltaLevel;
		float								DodgeFactor;
		SLOT_EQUIPMENT::TSlotEquipment		HitLocalisation;
		sint32								InflictedDamageBeforeArmor;
		sint32								InflictedNaturalDamage;
		sint32								InflictedDamage;
	};

protected:
	/// init method
	void init();

	/**
	 * add a brick to the phrase
	 * \param brick the added brick
	 */
	void addBrick( const CStaticBrick &brick );

	/**
	 * test attacker skill against opponent defense
	 * \return dodge factor (0 = no dodge,  1 = full dodge, >0&<1 partail dodge)
	 */
	float testOpponentDefense(CCombatDefenderPtr &combatDefender, bool useRightHand, SLOT_EQUIPMENT::TSlotEquipment slot, sint16 deltaLevel, sint32 &nbParryDodgeFlyingTextRequired);

	/**
	 * test attacker skill against opponent shield skill
	 * \return true if the opponent manage to use it's shield to prevent damage
	 */
	bool testShieldEfficiency(CCombatDefenderPtr &combatDefender, CCombatShield &shield);

	/**
	 * test phrase success
	 * \param rightHand true if attack uses the right hand, false if it uses left hand
	 * \return true if success, false if failed (partially or totally)
	 */
	bool testPhraseSuccess(bool useRightHand);

	/**
	 * launch hand attack
	 * \param actingEntity the attacker
	 * \param rightHand true if attack uses the right hand, false if it uses left hand
	 * \param isMad true if attacker is mad and hit himself
	 * \param madnessCaster if isMad == true then this is the caster of the madness spell
	 * \return true if the attack is successful
	 */
	bool launchAttack(CEntityBase * actingEntity, bool rightHand, bool isMad, TDataSetRow madnessCaster);

	/**
	 * launch attack on given target
	 */
	void launchAttackOnTarget(uint8 targetIndex, bool rightHand, bool isMad, bool needComputeBaseDamage);

	/**
	 * check target validity
	 * \param targetRowId rowid of the target to check
	 * \param errorCode the string that will receive the error code if any
	 * \return true if the target is valid
	 */
	bool checkTargetValidity( const TDataSetRow &targetRowId, std::string &errorCode );

	/**
	 * check the attacker can pay phrase's costs
	 */
	bool checkPhraseCost( std::string &errorCode );

	/**
	 * check target distance and orientation
	 */
	bool checkOrientation( const CEntityBase *actor, const CEntityBase *target );

	/**
	 * create the defender structure from given row id
	 */
	CCombatDefenderPtr createDefender( const TDataSetRow &targetRowId );

	/**
	 * validate combat actions
	 */
	bool validateCombatActions(std::string &errorCode);

	/**
	 * apply hand attack
	 * \param actingEntity the attacker
	 * \param rightHand true if attack uses the right hand, false if it uses left hand
	 */
	void applyAttack(CEntityBase * actingEntity, bool rightHand);

	/**
	 * apply an action
	 */
	void applyAction(TApplyAction & action, std::vector<TReportAction> & actionReports, bool rightHand);

	/**
	 * apply combat special actions
	 */
	void applyCombatActions();

	/**
	 * get hit localisation
	 */
	PHRASE_UTILITIES::TPairSlotShield getHitLocalisation(CCombatDefenderPtr &defender, const CCombatShield &shield, DMGTYPE::EDamageType dmgType);

	/**
	 * apply special effect due to localisation of the hit
	 */
	void applyLocalisationSpecialEffect( CCombatDefenderPtr &defender, SLOT_EQUIPMENT::TSlotEquipment slot, sint32 damage, sint32 &lostStamina);

	/**
	 * apply defender armor damage reduction
	 */
	sint32 applyArmorProtections(CCombatDefenderPtr &defender, sint32 damage, DMGTYPE::EDamageType dmgType, SLOT_EQUIPMENT::TSlotEquipment slot, bool shieldIsEffective);

	/// delete all targets objects
	void clearTargets()
	{
		_Targets.clear();
	}

	/// check opening can be used
	bool checkOpening();

	/// build target list
	void buildTargetList(bool rightHand, bool isMad);

	/// apply damage on bodyguard, return remaining damage
	sint32 applyDamageOnBodyguard(CEntityBase *actingEntity, TDataSetRow defenderRowId, sint32 attackerLevel, CSEffectPtr effect, sint32 damage, DMGTYPE::EDamageType dmgType);

	/// apply bounce effect if any exists
	void applyBounceEffect(CEntityBase *actingEntity, sint32 attackerLevel, CSEffectPtr effect, sint32 damage, DMGTYPE::EDamageType dmgType);

	/// compute and send aggro
	void computeAggro(CAiEventReport &aiEventReport, CEntityBase *target, sint32 hpDamage, sint32 staDamage = 0, sint32 sapDamage = 0);

	/// compute base damage
	void computeBaseDamage(bool rightHand, EGSPD::CPeople::TPeople targetRace);

	/// is a feint
	inline bool isFeint()
	{
		if (_BrickDefinedFlags.empty())
			return false;
		else
		{
			for (uint i = 0 ; i < _BrickDefinedFlags.size() ; ++i)
			{
				if (_BrickDefinedFlags[i] == BRICK_FLAGS::Feint)
					return true;
			}
			return false;
		}
	}

	/// compute delta level with given target
	sint16 computeDeltaLevel(CCombatDefenderPtr &combatDefender, bool rightHand);

	/// return sabrina cost with relative cost added
	uint16 sabrinaCost() const { return (uint16) ( _SabrinaCost * _SabrinaRelativeCost ); }

	/// return sabrina cost with relative cost added
	uint16 sabrinaCredit() const { return (uint16) ( _SabrinaCredit * _SabrinaRelativeCredit ); }

protected:
	// total cost (sabrina system)
	uint16					_SabrinaCost;

	// Relative cost must be added to total cost
	float					_SabrinaRelativeCost;

	/// brick max sabrina cost
	uint16					_BrickMaxSabrinaCost;

	// total credit (sabrina system)
	uint16					_SabrinaCredit;

	// Relative credit must be added to total credit
	float					_SabrinaRelativeCredit;

	/// stamina cost of the attack
	sint32					_StaminaCost;

	/// Stamina weapon weight cost factor
	float					_StaminaWeightFactorCost;

	// hp cost
	sint32					_HPCost;

	/// execution length modifier (in ticks)
	sint32					_ExecutionLengthModifier;

	/// hit rate modifier (in ticks)
	sint32					_HitRateModifier;

	/// latency factor
	float					_LatencyFactor;

	/// latency factor
	CDynValue<float,1>		_LatencyFactorDyn;

	/// modifier on dealt damage (whatever the success factor (the factor is applied on damage + damage mod afterwards))
	sint32					_DamageModifier;

	/// factor on dealt damage
	float					_DamageFactor;

	/// modifier on dealt damage
	CDamageFactor			_DamageFactorOnSuccess;

	/// base damage, independent from target
	float					_BaseDamage;

	/// natural damage, amount of damage without any modifiers
	sint32					_NaturalDamage;

	// aimed slot if any (for players)
	CAimedSlot						_AimedSlot;
	// aimed slot if any (for AI)
	AI_AIMING_TYPE::TAiAimingType	_AiAimingType;

	/// skill used by the root brick
	SKILLS::ESkills			_RootSkill;

	/// skill modifier for attack
	CDynValue<sint32,0>		_AttackSkillModifier;

	/// special hit, set to true to use special fx
	bool					_SpecialHit;

	/// stamina loss factor
	CDynValue<float,1>		_StaminaLossDynFactor;
	/// sap loss factor
	CDynValue<float,1>		_SapLossDynFactor;

	/// opening needed
	std::vector<BRICK_FLAGS::TBrickFlag> _OpeningNeededFlags;

	/// flag indicating if the attacker is disengaged at the end of this phrase
	bool					_DisengageOnEnd;

	/// If the specified sheetId is != CSheetId::Unknown, init player database entry "EXECUTE_PHRASE:SHEET" with it
	NLMISC::CSheetId		_RootSheetId;

	/// Critical Hit Chances Modifier (-100 +100)
	CDynValue<sint8,0>		_CriticalHitChancesModifier;

	/// weapon wear modifier
	CDynValue<float,0>		_WeaponWearModifier;

	/// hit all melee aggressors ?
	bool					_HitAllMeleeAggressors;
	CDynValue<float,1>		_MultiTargetGlobalDamageFactor;

	// actor behaviour
	MBEHAV::CBehaviour		_Behaviour;
	/// behaviour weight (sabrina cost of associated brick or 0)
	uint16					_BehaviourWeight;

	/// armor absorption factor
	CArmorAborption			_ArmorAbsorptionFactor;

	/// factor on target aggressivity (aggro) (1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	float					_AggroMultiplier;
	/// modifier on target aggressivity (aggro)
	sint32					_AggroModifier;

	//\name damage multipliers for ranged attacks
	//@{
	float			 		_DamagePointBlank;
	float			 		_DamageShortRange;
	float			 		_DamageMediumRange;
	float			 		_DamageLongRange;
	//@}

	/// vector of ai event report structures if target is managed by ai (ie. not a player)
	//std::vector<CAiEventReport*>	_AiEventReports;
	CAiEventReport			_AiEventReport;

	/// \name temp vars
	//@{
	/// success damage factor (>1 if real success, < 1 if partial success, 0 = total failure)
	float					_PhraseSuccessDamageFactor;
	/// critical hit
	bool					_CriticalHit;
	/// skill used to attack
	SKILLS::ESkills			_AttackSkill;
	/// validated flag
	bool					_Validated;
	/// \name flags indicated if an error message has been sent already (when in idle mode)
	//@{
	bool					_TargetTooFarMsg;
	bool					_NotEnoughHpMsg;
	bool					_NotEnoughStaminaMsg;
	bool					_NoAmmoMsg;
	bool					_BadOrientationMsg;
	//}@
	/// is current target valid ?
	bool					_CurrentTargetIsValid;
	/// melee or range combat
	bool					_MeleeCombat;
	/// total stamina cost
	sint32					_TotalStaminaCost;
	/// total hp cost
	sint32					_TotalHPCost;
	/// the total sabrina cost
	sint32					_TotalSabrinaCost;
	/// right weapon sabrina value
	uint16					_WeaponSabrinaValue;
	/// left weapon sabrina value
	uint16					_LeftWeaponSabrinaValue;
	/// hit localisation on main target
	SLOT_EQUIPMENT::TSlotEquipment _HitLocalisation;
	/// madness caster
	TDataSetRow				_MadnessCaster;
	/// is mad ?
	bool					_IsMad;
	/// if attacker is mad, keep here the skill used by caster to cast the mad effect (if applicable)
	SKILLS::ESkills			_MadSkill;
	//}@

	/// combat special actions (such as stun, bleed, slow etc)
	std::vector<CCombatAction*>	_CombatActions;

	/// targets
	std::vector<TTargetInfos> _Targets;

	/// right hand actions to apply
	std::vector<TApplyAction> _RightApplyActions;

	/// left hand actions to apply
	std::vector<TApplyAction> _LeftApplyActions;

	/// attacker
	CCombatAttacker*		_Attacker;
	/// used ammo
	CCombatWeapon			_Ammo;
	// used right weapon
	CCombatWeapon			_RightWeapon;
	// used left weapon
	CCombatWeapon			_LeftWeapon;

	/// flags defined by used bricks (needed for openings)
	std::vector<BRICK_FLAGS::TBrickFlag> _BrickDefinedFlags;

	// BRIANCODE Added to support mission evaluation of combat bricks
	// bricks composing the phrase
	std::vector<NLMISC::CSheetId> _BrickSheets;

	/// \name Delayed Events
	// @{
	// List of event to be processed during the apply() stage of the combat phrase.
	enum TDelayedEventType
	{
		EventEvade=0,			// the attacker miss (=> defender "evade")
		EventDodge,				// the defender dodged
		EventParry,				// the defender parried
		EventMeleeDodgeOpening,	// the defender is a player and he dodged a melee attack
		EventMeleeParryOpening,	// the defender is a player and he parried a melee attack
	};
	struct	CDelayedEvent
	{
		TDataSetRow			DefenderRowId;
		TDelayedEventType	EventType;
		bool				SendFlyingText;	// true if must apply the flying text
	};
	std::vector<CDelayedEvent>	_DelayedEvents;
	bool						_MissFlyingTextTriggered;

	// add a special event that will resolved in the apply() step
	void		addDelayedEvent(TDataSetRow defenderId, TDelayedEventType eventType, bool sendFlyingText= true);

	// apply the events and clear the list
	void		flushDelayedEvents();
	// @}

};

typedef NLMISC::CSmartPtr<CCombatPhrase> CCombatPhrasePtr;

#endif // RY_COMBAT_PHRASE_H

/* End of combat_phrase.h */



