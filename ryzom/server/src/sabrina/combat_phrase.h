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

// misc
#include "nel/misc/types_nl.h"
// game share
#include "game_share/ai_event_report.h"
//
#include "s_phrase.h"
#include "phrase_utilities_functions.h"
#include "combat_attacker.h"
#include "combat_defender.h"
#include "combat_action.h"

//------------------------------------------------------------
/*
	Brick parameters
	
	cr_sta:int_value		- stamina cost to attacker
	cr_hp:int_value			- hp cost to attacker
	cr_t0:int_value			- execution length modifier (in ticks)
	cr_t1:int_value			- latency length modifier (in ticks)
	cr_open:str_name		- constraint meaning action can only be performed when named condition is met

	dmgr:float_value		- damage ratio on targets
	dmgm:int_value			- damage modifier on targets

	aim:str_value			- target specified slot

	skmat:int_value			- attacker skill modifier (on attack)
	skmde:int_value			- defender skill modifier (on defense)
	
	absstar:float_value		- ratio of absorbed stamina on target(s) (relative to damage on hps)
	absstam:int_value		- modifier on absorbed stamina on target(s)
	abssapr:float_value		- ratio of absorbed sap on target(s) (relative to damage on hps)
	abssapm:int_value		- modifier on absorbed sap on target(s)

	lapr:float_value		- light armor protection ratio (after all computing) (1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	mapr:float_value		- medium armor protection ratio (after all computing) (1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	hapr:float_value		- heavy armor protection ratio (after all computing) (1.0 = default; 1.2 = +20% ; 0.6 = -40%)

	lawr:float_value		- light armor wear ratio (1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	mawr:float_value		- medium armor wear ratio (1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	hawr:float_value		- heavy armor wear ratio (1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	
	agrr:float_value		- aggro ratio (default 1.0)

	rdpbr:float_value		- range damage point blank ratio
	rdsrr:float_value		- range damage short range ratio
	rdmrr:float_value		- range damage medium range ratio
	rdlrr:float_value		- range damage long range ratio

	bev_exec:int_value		- value of the behaviour for execution
	bev_ok:int_value		- value of the behaviour for success
	bev_crit:int_value		- value of the behaviour for critical success
	bev_fail:int_value		- value of the behaviour for failure
	bev_fumb:int_value		- value of the behaviour for fumble
			
// following are the special effecs
	ef:stun
	 :int_value				- duration of the stun
	 :int_value				- duration of the stun when resisted

	ef:slow
	 :float_value			- ratio of the slow effect (0.8 = -20%)
	 :float_value			- ratio of the slow effect when resisted (0.8 = -20%)
	 :int_value				- duration of the slow (in ticks)
	 :int_value				- duration of the slow when resisted (in ticks)

	ef:vamphp
	 :float_value			- ratio of the hp lost by the target that are given to the attacker (vampire effect)
	ef:vampsta
	 :float_value			- ratio of the stamina lost by the target that are given to the attacker (vampire effect)
	ef:vampsap
	 :float_value			- ratio of the sap lost by the target that are given to the attacker (vampire effect)

	ef:hrmde
	 :int_value				- Hit Rate Modifier for the defender (in ticks)
	 :int_value				- Hit Rate Modifier for the defender when resist (in ticks)
	 :int_value				- Duration of the Hit Rate modification for the defender (in ticks)
	 :int_value				- Duration of Hit Rate modification for the defender when resist (in ticks)

	ef:fear
	 :int_value				- duration of the fear effect 
	 :int_value				- duration of the fear effect if resisted

	ef:bleed
	 :int_value				- duration of the bleed effect
	 :int_value				- duration of the bleed effect if resisted
	 :int_value				- nb of hp lost each repeat by the target
	 :int_value				- nb of hp lost each repeat by the target if resisted
	 :int_value				- repeat rate of the bleed effect (every x ticks)
	 :int_value				- repeat rate of the bleed effect when resisted (every x ticks)

	ef:poison
	 :int_value				- duration of the poison effect
	 :int_value				- duration of the poison effect if resisted
	 :int_value				- nb of hp lost each repeat by the target
	 :int_value				- nb of hp lost each repeat by the target if resisted
	 :int_value				- repeat rate of the poison effect (every x ticks)
	 :int_value				- repeat rate of the poison effect when resisted (every x ticks)

	ef:disease
	 :int_value				- duration of the disease effect
	 :int_value				- duration of the disease effect if resisted
	 :int_value				- nb of hp lost each repeat by the target
	 :int_value				- nb of hp lost each repeat by the target if resisted
	 :int_value				- repeat rate of the disease effect (every x ticks)
	 :int_value				- repeat rate of the disease effect when resisted (every x ticks)

  	ef:firedmg
	 :int_value				- amount of fire damage added to the attack
	 :int_value				- amount of fire damage added to the attack if target resists

	ef:bcast				- break target casting
*/
//------------------------------------------------------------

/**
 * Specialized phrase for combat actions
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatPhrase : public CSPhrase
{
public:
	/// default Constructor
	CCombatPhrase() { init(); }

	/// Constructor
	CCombatPhrase(const CStaticBrick &rootBrick);
	
	/// destructor
	virtual ~CCombatPhrase();

	/// init method
	void init();
	
	/// build the phrase
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks );

	/**
	 * set the actor
	 */
	virtual void setActor( const TDataSetRow &entityRowId ) 
	{

	}

	/**
	 * set the primary item
	 * \param itemPtr pointer on primary item 
	 */
	//virtual void setPrimaryItem( CGameItemPtr itemPtr ) {}

	/**
	 * set the secondary item
	 * \param itemPtr pointer on secondary item 
	 */
	//virtual void setSecondaryItem( CGameItemPtr itemPtr ){}

	/**
	 * add a consumable ressource (object)
	 * \param itemPtr pointer on the consumable item
	 */
	//virtual void addConsumableItem( CGameItemPtr itemPtr ){}	

	/**
	 * set the primary target
	 * \param entityId id of the primary target
	 */
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId )
	{
//		_TargetRowId = entityRowId;
	}

	/**
	 * add a target entity
	 * \param entityId id of the target
	 */
	virtual void addTargetEntity( const TDataSetRow &entityRowId )
	{}

	/**
	 * evaluate phrase
	 * \param evalReturnInfos struct that will receive evaluation results
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate(CEvalReturnInfos *msg = NULL);

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
	 * apply method, called at the end of the execution, starts latency
	 */
	virtual void apply();

	/**
	 * called at the end of the latency time
	 */
	virtual void end();

	/**
	 * set attacker
	 * \param attacker 
	 */
	inline void setAttacker( CCombatAttacker *attacker) { _Attacker = attacker; }

	/**
	 * set primary defender
	 * \param defender
	 */
	inline void setPrimaryDefender( CCombatDefender *defender) { _Defender = defender; }

	/// set the disengageOnEnd flag
	inline void disengageOnEnd(bool flag) { _DisengageOnEnd = flag; }
	/// get the disengageOnEnd flag
	inline bool disengageOnEnd() const { return _DisengageOnEnd; }

protected:
	/**
	 * add a brick to the phrase
	 * \param brick the added brick
	 */
	void addBrick( const CStaticBrick &brick );

	/**
	 * test attacker skill against opponent defense
	 * \return true if the oppent dodged the attack
	 */
	bool testOpponentDefense(const TDataSetRow &targetRowId, const PHRASE_UTILITIES::TPairSlotShield &localisation);

	/**
	 * test phrase success
	 * \return true if success, false if failed (partially or totally)
	 */
	bool testPhraseSuccess();

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
	 * create the defender structure from given row id
	 */
	void createDefender( const TDataSetRow &targetRowId );

	/**
	 * validate combat actions
	 */
	bool validateCombatActions(std::string &errorCode);

	/**
	 * apply combat actions
	 */
	void applyCombatActions();

protected:
	// total cost (sabrina system)
	uint16					_SabrinaCost;

	// total credit (sabrina system)
	uint16					_SabrinaCredit;
	
	/// stamina cost of the attack
	sint32					_StaminaCost;

	// hp cost
	sint32					_HPCost;

	/// execution length modifier (in ticks)
	sint32					_ExecutionLengthModifier;

	/// hit rate modifier (in ticks)
	sint32					_HitRateModifier;
	
	/// modifier on dealt damage
	sint32					_DamageModifier;

	/// factor on dealt damage
	float					_DamageFactor;

	/// forced localisation if any
	SLOT_EQUIPMENT::TSlotEquipment	_ForcedLocalisation;

	/// skill used by the root brick
	SKILLS::ESkills			_RootSkill;

	/// skill modifier for attack
	sint32					_AttackSkillModifier;

	/// repeat mode on/off
	bool					_CyclicPhrase;

	/// stamina loss factor
	float					_StaminaLossFactor;
	/// stamina loss modifier
	uint32					_StaminaLossModifier;
	/// sap loss factor
	float					_SapLossFactor;
	/// sap loss modifier
	uint32					_SapLossModifier;

	/// opening needed
	std::string				_Opening;

	/// 
	bool					_DisengageOnEnd;
	

	//\name the behaviours for the actor
	//@{
	MBEHAV::CBehaviour		_ExecutionBehaviour;
	MBEHAV::CBehaviour		_SuccessBehaviour;
	MBEHAV::CBehaviour		_CriticalSuccessBehaviour;
	MBEHAV::CBehaviour		_FailureBehaviour;
	MBEHAV::CBehaviour		_FumbleBehaviour;
	MBEHAV::CBehaviour		_EndBehaviour;
	MBEHAV::CBehaviour		_StopBehaviour;	
	//@}

	/// Multiplier on light armor damage absorption (after all computing)(1.0 = default; 1.2 = +20% ; 0.6 = -40%)
	float					_LightArmorAbsorptionMultiplier;
	/// Modifier on light armor damage absorption
	sint32					_LightArmorAbsorptionModifier;
	/// Multiplier on light armor wear ( armor hp lost)
	float					_LightArmorWearMultiplier;

	/// Multiplier on Medium armor damage absorption
	float					_MediumArmorAbsorptionMultiplier;
	/// Modifier on medium armor damage absorption
	sint32					_MediumArmorAbsorptionModifier;
	/// Multiplier on Medium armor wear ( armor hp lost)
	float					_MediumArmorWearMultiplier;

	/// Multiplier on Heavy armor damage absorption
	float					_HeavyArmorAbsorptionMultiplier;
	/// Modifier on heavy armor damage absorption
	sint32					_HeavyArmorAbsorptionModifier;
	/// Multiplier on Heavy armor wear ( armor hp lost)
	float					_HeavyArmorWearMultiplier;
	
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

	// following will be used later on
// secondary targets
//	std::vector<NLMISC::CEntityId>	_SecondaryTargets;

	/// \name temp vars
	//@{
	/// success damage factor (1.0 if real success, < 1 if partial success, 0 = total failure)
	float					_PhraseSuccessDamageFactor;
	/// skill used to attack
	SKILLS::ESkills			_AttackSkill;	
	/// validated flag
	bool					_Validated;
	/// already sent a target too far message since last strike
	bool					_TargetTooFarMsg;
	/// 
	bool					_NotEnoughHpMsg;
	/// 
	bool					_NotEnoughStaminaMsg;
	/// is current target valid ?
	bool					_CurrentTargetIsValid;
	/// melee or range combat
	bool					_MeleeCombat;
	/// delta level between attacker and defender
	sint32					_DeltaLevel;
	//}@

	/// combat special actions (such as stun, bleed, slow etc)
	std::vector< CCombatAction * >	_CombatActions;
	
	/// attacker
	CCombatAttacker			*_Attacker;
	/// attacker
	CCombatDefender			*_Defender;
};


#endif // RY_COMBAT_PHRASE_H

/* End of combat_phrase.h */



