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



#ifndef NL_EGS_STATIC_AI_ACTION_H
#define NL_EGS_STATIC_AI_ACTION_H

//Nel georges
#include "nel/georges/u_form.h"
//
#include "ai_share/ai_aiming_type.h"
#include "game_share/damage_types.h"
#include "game_share/skills.h"
#include "game_share/scores.h"
#include "game_share/effect_families.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/magic_fx.h"

namespace AI_ACTION
{
	enum TAiActionType
	{
		Melee = 0,
		Range,
		HealSpell,
		DamageSpell,
		HoTSpell,
		DoTSpell,
		EffectSpell,
		EoTSpell,
		ToxicCloud,

		UnknownType,
	};

	const std::string	&toString(TAiActionType e);
	TAiActionType	toAiActionType(const std::string &s);

	enum TAiEffectType
	{
		Stun = 0,
		Root,
		Mezz,
		Blind,
		Fear,
		Hatred,
		MeleeMadness,
		RangeMadness,
		MagicMadness,
		SlowMove,
		SlowMelee,
		SlowRange,
		SlowMagic,
		ResistDebufAcid,
		ResistDebufCold,
		ResistDebufRot,
		ResistDebufFire,
		ResistDebufPoison,
		ResistDebufElectric,
		ResistDebufShockwave,
		SkillDebufMelee,
		SkillDebufRange,
		SkillDebufMagic,
		Dot,
		
		Stench,
		Bounce,
		RedirectAttacks,
		ReflectDamage,
		ReverseDamage,

		MassDispel,
		Disarm,

		UnknownEffect,
		Normal = UnknownEffect, // only keep it for compatibility
	};

	const std::string	&toString(TAiEffectType e);
	TAiEffectType	toAiEffectType(const std::string &s);
	EFFECT_FAMILIES::TEffectFamily toEffectFamily(TAiEffectType effect, TAiActionType action);

};

namespace DURATION_TYPE
{
	enum TDurationType
	{
		Normal = 0, // seconds
		Permanent,
		UntilCasterDeath,

		Unknown,
	};
	const std::string	&toString(TDurationType e);
	TDurationType	fromString(const std::string &s);


}; // DURATION_TYPE 

/**
 * struct used for melee and range combat action
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct CCombatParams
{
	// no ctor as this class is used in an union

	// init method
	void init()
	{
		Melee = true;
		SpeedFactor = 0;
		DamageFactor = 0;
		DamageModifier = 0;
		DamageType = DMGTYPE::UNDEFINED;
		AimingType = AI_AIMING_TYPE::Random;
		SpecialDamageFactor = 0;
		SpecialDamageType = DMGTYPE::UNDEFINED;
		EffectFamily = AI_ACTION::UnknownEffect;
		EffectValue = 0.0f;
		EffectTime = 0;
		EffectUpdateFrequency = 10;
		EffectDamageType = DMGTYPE::UNDEFINED;
		EffectAffectedScore = SCORES::hit_points;
		Critic = 0.0f;
		Behaviour = MBEHAV::UNKNOWN_BEHAVIOUR;
		ArmorFactor = 1.0f;
	}

	/// Serial
	void serial(class NLMISC::IStream &f);

	/// read params from georges
	void readForm (const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type);

	bool		Melee;
	float		SpeedFactor;
	float		DamageFactor;	
	sint32		DamageModifier;
	/// factor on defender armor protection (0 : ignore defender armor, 1 : normal armor) 
	float		ArmorFactor; 
	/// [0,1], if >0 a random test is made to force a critical hit (1 = always a critical hit)
	float		Critic; 
	DMGTYPE::EDamageType	DamageType;
	AI_AIMING_TYPE::TAiAimingType	AimingType;

	// if special dmg
	float					SpecialDamageFactor;
	DMGTYPE::EDamageType	SpecialDamageType;

	// if special effect
	AI_ACTION::TAiEffectType	EffectFamily;
	float						EffectValue;
	NLMISC::TGameCycle			EffectTime;
	NLMISC::TGameCycle			EffectUpdateFrequency;
	DMGTYPE::EDamageType		EffectDamageType;
	SCORES::TScores				EffectAffectedScore;
	DURATION_TYPE::TDurationType EffectDurationType;

	// force a behaviour
	MBEHAV::EBehaviour			Behaviour;
};

/**
 * struct used for spell action
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct CSpellParams
{
	// no ctor as this class is used in an union

	// init method
	void init()
	{
		CastingTime = 0;
		PostActionTime = 0;
		SapCost = 0;
		HpCost = 0;
		Skill = SKILLS::unknown;
		AffectedScore = SCORES::hit_points;
		DamageType = DMGTYPE::UNDEFINED;
		SpellParamValue = 0;
		SpellParamValue2 = 0;
		SpellPowerFactor = 0.0f;
		Behaviour = MBEHAV::UNKNOWN_BEHAVIOUR;
		Stackable = false;
		SpellLevel = 0.0f;
	}

	/// Serial
	void serial(class NLMISC::IStream &f);

	/// read params from georges
	void readForm (const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type);

	NLMISC::TGameCycle	CastingTime; // in ticks
	NLMISC::TGameCycle	PostActionTime; // in ticks
	SKILLS::ESkills		Skill;
	uint16				SapCost;
	uint16				HpCost;
	SCORES::TScores		AffectedScore;
	float				SpellParamValue;
	float				SpellParamValue2;
	float				SpellPowerFactor;
	bool				Stackable;
	float				SpellLevel;
	
	// dmg spell
	DMGTYPE::EDamageType	DamageType;

	// force a behaviour
	MBEHAV::EBehaviour	Behaviour;

	// static value indicating oif casting time is set to a special value. In this case, use creature attack speed for casting time
	static NLMISC::TGameCycle UseAttackSpeedForCastingTime;
};

/**
 * struct used for 'Over Time' spell aiaction
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct COTSpellParams : public CSpellParams
{
	// no ctor as this class is used in an union

	// init method
	void init()
	{
		CSpellParams::init();
		UpdateFrequency = 10;
		Duration = 0;
	}

	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		CSpellParams::serial(f);
		f.serial(UpdateFrequency);
		f.serial(Duration);

		if (f.isReading())
		{
			std::string val;
			f.serial(val);
			EffectDurationType = DURATION_TYPE::fromString(val);
		}
		else
		{
			std::string val = DURATION_TYPE::toString(EffectDurationType);
			f.serial(val);
		}
	}

	/// read params from georges
	void readForm (const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type);

	NLMISC::TGameCycle				UpdateFrequency; // in ticks
	NLMISC::TGameCycle				Duration; // in ticks
	DURATION_TYPE::TDurationType	EffectDurationType;
};


/**
 * struct used for 'Effect' spell aiaction
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct CEffectSpellParams : public CSpellParams
{
	// no ctor as this class is used in an union

	// init method
	void init()
	{
		CSpellParams::init();
		EffectFamily = AI_ACTION::UnknownEffect;
		Duration = 0;
	}

	/// Serial
	void serial(class NLMISC::IStream &f);

	/// read params from georges
	void readForm (const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type);


	AI_ACTION::TAiEffectType		EffectFamily;
	NLMISC::TGameCycle				Duration; // in ticks
	DURATION_TYPE::TDurationType	EffectDurationType;
};

/**
 * struct used for 'Over Time Effect' spell aiaction
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct COTEffectSpellParams : public COTSpellParams
{
	// no ctor as this class is used in an union

	// init method
	void init()
	{
		COTSpellParams::init();
		EffectFamily = AI_ACTION::UnknownEffect;
	}

	/// Serial
	void serial(class NLMISC::IStream &f);

	/// read params from georges
	void readForm (const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type);

	AI_ACTION::TAiEffectType	EffectFamily;
};


// union containing all params struct
union TAiActionParams
{
	CCombatParams		 Combat;
	CSpellParams		 Spell;
	COTSpellParams		 OTSpell;
	CEffectSpellParams	 EffectSpell;
	COTEffectSpellParams OTEffectSpell;
};

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct TAiArea
{
	TAiArea() : AreaType(MAGICFX::UnknownSpellMode), AreaRange(0.0f),
				ChainFadingFactor(0.0f), ChainMaxTargets(0),
				SprayHeight(0.0f),SprayBase(0.0f),SprayAngle(0)
	{}

	/// Serial
	void serial(class NLMISC::IStream &f);
	
	/// read params from georges
	void readForm (const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type);

	// Area type (none/chain/spray/bomb)
	MAGICFX::TSpellMode	AreaType;
	// area range in meters
	float				AreaRange;
	// if bomb, max nb targets
	uint8				BombMaxTargets;
	// if chain, max nb targets
	uint8				ChainMaxTargets;
	// if chain, fading factor (]0;1])
	float				ChainFadingFactor;
	// if spray, max height in meters
	float				SprayHeight;
	// if spray, base width in meters
	float				SprayBase;
	// if spray, angle in degrees
	uint8				SprayAngle;
	// if spray, max nb targets
	uint8				SprayMaxTargets;
};


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CStaticAiAction
{
public:

	/// Constructor
	CStaticAiAction() {}

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 21; }

	/// Serial
	void serial(class NLMISC::IStream &f);

	/// Removed
	void removed() {}

	/// get ai action type
	inline AI_ACTION::TAiActionType getType() const { return _Type; }

	/// get data params
	inline const TAiActionParams &getData() const { return _Data; }

	/// get area params
	inline const TAiArea &getAreaData() const { return _Area; }

	/// get sheetId
	inline const NLMISC::CSheetId &getSheetId() const { return _SheetId; }

protected:
	/// ai action type
	AI_ACTION::TAiActionType	_Type;

	/// union containing all the params according to type
	TAiActionParams				_Data;

	/// data to describe the area affected by the action
	TAiArea						_Area;

	/// associated sheetId
	NLMISC::CSheetId			_SheetId;
};


#endif // NL_EGS_STATIC_AI_ACTION_H

/* End of egs_static_ai_action.h */
