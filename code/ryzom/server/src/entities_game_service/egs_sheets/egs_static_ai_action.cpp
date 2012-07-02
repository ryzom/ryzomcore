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
#include "egs_static_ai_action.h"
//Nel georges
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
//NeL misc
#include "nel/misc/string_conversion.h"

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


NLMISC::TGameCycle CSpellParams::UseAttackSpeedForCastingTime = ~0;

namespace AI_ACTION
{
	// The conversion table
	NL_BEGIN_STRING_CONVERSION_TABLE (TAiActionType)
		NL_STRING_CONVERSION_TABLE_ENTRY (Melee)
		NL_STRING_CONVERSION_TABLE_ENTRY (Range)
		NL_STRING_CONVERSION_TABLE_ENTRY (HealSpell)
		NL_STRING_CONVERSION_TABLE_ENTRY (DamageSpell)
		NL_STRING_CONVERSION_TABLE_ENTRY (HoTSpell)
		NL_STRING_CONVERSION_TABLE_ENTRY (DoTSpell)
		NL_STRING_CONVERSION_TABLE_ENTRY (EffectSpell)
		NL_STRING_CONVERSION_TABLE_ENTRY (EoTSpell)
		NL_STRING_CONVERSION_TABLE_ENTRY (ToxicCloud)
		NL_STRING_CONVERSION_TABLE_ENTRY (UnknownType)
	NL_END_STRING_CONVERSION_TABLE(TAiActionType, AiActionConversion, UnknownType)

	const std::string	&toString(TAiActionType e)
	{
		return AiActionConversion.toString(e);
	}

	TAiActionType	toAiActionType(const std::string &s)
	{
		return AiActionConversion.fromString(s);
	}

	NL_BEGIN_STRING_CONVERSION_TABLE (TAiEffectType)
		NL_STRING_CONVERSION_TABLE_ENTRY (Stun)
		NL_STRING_CONVERSION_TABLE_ENTRY (Root)
		NL_STRING_CONVERSION_TABLE_ENTRY (Mezz)
		NL_STRING_CONVERSION_TABLE_ENTRY (Blind)
		NL_STRING_CONVERSION_TABLE_ENTRY (Fear)
		NL_STRING_CONVERSION_TABLE_ENTRY (Hatred)		
		NL_STRING_CONVERSION_TABLE_ENTRY (MeleeMadness)
		NL_STRING_CONVERSION_TABLE_ENTRY (RangeMadness)
		NL_STRING_CONVERSION_TABLE_ENTRY (MagicMadness)
		NL_STRING_CONVERSION_TABLE_ENTRY (SlowMove)
		NL_STRING_CONVERSION_TABLE_ENTRY (SlowMelee)
		NL_STRING_CONVERSION_TABLE_ENTRY (SlowRange)
		NL_STRING_CONVERSION_TABLE_ENTRY (SlowMagic)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufAcid)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufCold)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufRot)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufFire)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufPoison)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufElectric)
		NL_STRING_CONVERSION_TABLE_ENTRY (ResistDebufShockwave)
		NL_STRING_CONVERSION_TABLE_ENTRY (SkillDebufMelee)
		NL_STRING_CONVERSION_TABLE_ENTRY (SkillDebufRange)
		NL_STRING_CONVERSION_TABLE_ENTRY (SkillDebufMagic)
		NL_STRING_CONVERSION_TABLE_ENTRY (Dot)
		NL_STRING_CONVERSION_TABLE_ENTRY (Stench)
		NL_STRING_CONVERSION_TABLE_ENTRY (Bounce)
		NL_STRING_CONVERSION_TABLE_ENTRY (RedirectAttacks)
		NL_STRING_CONVERSION_TABLE_ENTRY (ReflectDamage)
		NL_STRING_CONVERSION_TABLE_ENTRY (ReverseDamage)

		NL_STRING_CONVERSION_TABLE_ENTRY (MassDispel)
		NL_STRING_CONVERSION_TABLE_ENTRY (Disarm)

		NL_STRING_CONVERSION_TABLE_ENTRY (UnknownEffect)
		NL_STRING_CONVERSION_TABLE_ENTRY (Normal) // only keep it for compatibility
	NL_END_STRING_CONVERSION_TABLE(TAiEffectType, AiEffectConversion, UnknownEffect)


	const std::string	&toString(TAiEffectType e)
	{
		return AiEffectConversion.toString(e);
	}

	TAiEffectType	toAiEffectType(const std::string &s)
	{
		return AiEffectConversion.fromString(s);
	}

	EFFECT_FAMILIES::TEffectFamily toEffectFamily(TAiEffectType effect, TAiActionType action)
	{
		const bool combat = (action == Melee || action == Range);

		switch(effect)
		{
		case Stun:
			return (combat?EFFECT_FAMILIES::CombatStun : EFFECT_FAMILIES::Stun);
		case Root:
			return EFFECT_FAMILIES::Root;
		case Mezz:
			return EFFECT_FAMILIES::Mezz;
		case Blind:
			return EFFECT_FAMILIES::Blind;
		case Fear:
			return EFFECT_FAMILIES::Fear;
		case Hatred:
			return EFFECT_FAMILIES::AllHatred;			
		case MeleeMadness:
			return EFFECT_FAMILIES::MadnessMelee;
		case RangeMadness:
			return EFFECT_FAMILIES::MadnessRange;
		case MagicMadness:
			return EFFECT_FAMILIES::MadnessMagic;
		case SlowMove:
			return (combat?EFFECT_FAMILIES::CombatMvtSlow : EFFECT_FAMILIES::SlowMove);
		case SlowMelee:
			return EFFECT_FAMILIES::SlowMelee;
		case SlowRange:
			return EFFECT_FAMILIES::SlowRange;
		case SlowMagic:
			return EFFECT_FAMILIES::SlowMagic;
		case ResistDebufAcid:
			return EFFECT_FAMILIES::DebuffResistAcid;
		case ResistDebufCold:
			return EFFECT_FAMILIES::DebuffResistCold;
		case ResistDebufRot:
			return EFFECT_FAMILIES::DebuffResistRot;
		case ResistDebufFire:
			return EFFECT_FAMILIES::DebuffResistFire;
		case ResistDebufPoison:
			return EFFECT_FAMILIES::DebuffResistPoison;
		case ResistDebufElectric:
			return EFFECT_FAMILIES::DebuffResistElectricity;
		case ResistDebufShockwave:
			return EFFECT_FAMILIES::DebuffResistSchock;
		case SkillDebufMelee:
			return EFFECT_FAMILIES::DebuffSkillMelee;
		case SkillDebufRange:
			return EFFECT_FAMILIES::DebuffSkillRange;
		case SkillDebufMagic:
			return EFFECT_FAMILIES::DebuffSkillMagic;
		case Stench:
			return EFFECT_FAMILIES::Stench;
		case Bounce:
			return EFFECT_FAMILIES::Bounce;
		case RedirectAttacks:
			return EFFECT_FAMILIES::RedirectAttacks;
		case ReflectDamage:
			return EFFECT_FAMILIES::ReflectDamage;
		case ReverseDamage:
			return EFFECT_FAMILIES::ReverseDamage;
			
		// punctual effects, no need for an effect family
		case MassDispel:
		case Disarm:
		//
		default:
			return EFFECT_FAMILIES::Unknown;
		};
	};

}; // AI_ACTION


namespace DURATION_TYPE
{
	// The conversion table
	NL_BEGIN_STRING_CONVERSION_TABLE (TDurationType)
		NL_STRING_CONVERSION_TABLE_ENTRY (Normal)
		NL_STRING_CONVERSION_TABLE_ENTRY (Permanent)
		NL_STRING_CONVERSION_TABLE_ENTRY (UntilCasterDeath)
		NL_STRING_CONVERSION_TABLE_ENTRY (Unknown)
	NL_END_STRING_CONVERSION_TABLE(TDurationType, DurationConversion, Unknown)
		
	const std::string	&toString(TDurationType e)
	{
		return DurationConversion.toString(e);
	}
	
	TDurationType	fromString(const std::string &s)
	{
		return DurationConversion.fromString(s);
	}
};


//--------------------------------------------------------------
//					CCombatParams::serial
//--------------------------------------------------------------
void CCombatParams::serial(class NLMISC::IStream &f)
{
	f.serial(Melee);
	f.serial(SpeedFactor);
	f.serial(DamageFactor);
	f.serial(DamageModifier);
	f.serial(Critic);
	f.serial(SpecialDamageFactor);
	f.serial(EffectValue);
	f.serial(EffectTime);
	f.serial(EffectUpdateFrequency);
	f.serial(ArmorFactor);
	
	if (f.isReading())
	{
		string val;
		f.serial(val);
		AimingType = AI_AIMING_TYPE::toAimingType(val);
		f.serial(val);
		SpecialDamageType = DMGTYPE::stringToDamageType(val);
		f.serial(val);
		DamageType = DMGTYPE::stringToDamageType(val);
		f.serial(val);
		EffectFamily = AI_ACTION::toAiEffectType(val);
		f.serial(val);
		Behaviour = MBEHAV::stringToBehaviour(val);
		f.serial(val);
		EffectAffectedScore = SCORES::toScore(val);
		f.serial(val);
		EffectDamageType = DMGTYPE::stringToDamageType(val);
		f.serial(val);
		EffectDurationType = DURATION_TYPE::fromString(val);
	}
	else
	{
		string val = AI_AIMING_TYPE::toString(AimingType);
		f.serial(val);
		val = DMGTYPE::toString(SpecialDamageType);
		f.serial(val);
		val = DMGTYPE::toString(DamageType);
		f.serial(val);		
		val = AI_ACTION::toString(EffectFamily);
		f.serial(val);		
		val = MBEHAV::behaviourToString(Behaviour);
		f.serial(val);
		val = SCORES::toString(EffectAffectedScore);
		f.serial(val);
		val = DMGTYPE::toString(EffectDamageType);
		f.serial(val);
		val = DURATION_TYPE::toString(EffectDurationType);
		f.serial(val);
	}
} // CCombatParams::serial //

//--------------------------------------------------------------
//					CCombatParams::readForm
//--------------------------------------------------------------
void CCombatParams::readForm (const UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type)
{
	string value;

	Melee = (type == AI_ACTION::Melee);

	root.getValueByName( SpeedFactor, "SpeedFactor" );
	root.getValueByName( DamageFactor, "DamageFactor" );	
	root.getValueByName( DamageModifier, "DamageAdd" );
	root.getValueByName( Critic, "Critic" );
	root.getValueByName( SpecialDamageFactor, "SpecialDamageFactor" );
	root.getValueByName( ArmorFactor, "ArmorAbsorptionFactor" );	
		
	if (root.getValueByName( value, "CombatDamageType" ))
		DamageType = DMGTYPE::stringToDamageType(value);
	
	if (root.getValueByName( value, "SpecialDamageType" ))
		SpecialDamageType = DMGTYPE::stringToDamageType(value);

	if (root.getValueByName( value, "AimingType" ))
		AimingType = AI_AIMING_TYPE::toAimingType(value);

	if (root.getValueByName( value, "Behaviour" ))
	{
		Behaviour = MBEHAV::stringToBehaviour(value);
	}

	// effect
	if (root.getValueByName( value, "EffectType" ))
		EffectFamily = AI_ACTION::toAiEffectType(value);

	if (root.getValueByName( value, "EffectDurationType" ))
		EffectDurationType= DURATION_TYPE::fromString(value);
	
	if (EffectFamily == AI_ACTION::Dot)
	{
		root.getValueByName( EffectValue, "DamageValue" );
		
		float time;
		if ( root.getValueByName( time, "Duration" ) )
			EffectTime = (TGameCycle)(time / CTickEventHandler::getGameTimeStep());
		if ( root.getValueByName( time, "UpdateFrequency" ) )
			EffectUpdateFrequency = (TGameCycle)(time / CTickEventHandler::getGameTimeStep());

		if (root.getValueByName( value, "DamageType" ))
			EffectDamageType = DMGTYPE::stringToDamageType(value);
		if ( root.getValueByName( value, "DamageScore" ) )
			EffectAffectedScore = SCORES::toScore(value);
	}
	else
	{
		root.getValueByName( EffectValue, "EffectValue" );
		float time;
		root.getValueByName( time, "EffectDuration" );
		EffectTime = (TGameCycle)(time / CTickEventHandler::getGameTimeStep()); // time is in seconds in sheet, and we keep it in ticks
	}
} // CCombatParams::readForm //


//--------------------------------------------------------------
//					CSpellParams::serial
//--------------------------------------------------------------
void CSpellParams::serial(class NLMISC::IStream &f)
{
	f.serial(CastingTime);
	f.serial(PostActionTime);
	f.serial(Stackable);
	f.serial(SapCost);
	f.serial(HpCost);
	f.serial(SpellParamValue);
	f.serial(SpellParamValue2);
	f.serial(SpellPowerFactor);
	f.serial(SpellLevel);
	if (f.isReading())
	{
		string val;
		f.serial(val);
		Skill = SKILLS::toSkill(val);
		f.serial(val);
		AffectedScore = SCORES::toScore(val);
		f.serial(val);
		DamageType = DMGTYPE::stringToDamageType(val);
		f.serial(val);
		Behaviour = MBEHAV::stringToBehaviour(val);
	}
	else
	{
		string val = SKILLS::toString(Skill);
		f.serial(val);
		val = SCORES::toString(AffectedScore);
		f.serial(val);
		val = DMGTYPE::toString(DamageType);
		f.serial(val);
		val = MBEHAV::behaviourToString(Behaviour);
		f.serial(val);		
	}
} // CSpellParams::serial //

//--------------------------------------------------------------
//					CSpellParams::readForm
//--------------------------------------------------------------
void CSpellParams::readForm (const UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type)
{
	string value;

	float time;
	root.getValueByName( time, "CastingTime" );
	// special value : if casting time is -1 in sheet, set it to UseAttackSpeedForCastingTime
	if (time < 0)
		CastingTime = UseAttackSpeedForCastingTime;
	else
		CastingTime = TGameCycle(time / CTickEventHandler::getGameTimeStep()); // time is in seconds in sheet, and we keep it in ticks

	root.getValueByName( time, "PostActionTime" );
	PostActionTime = TGameCycle(time / CTickEventHandler::getGameTimeStep()); // time is in seconds in sheet, and we keep it in ticks
	
	uint32 stack;
	root.getValueByName( stack, "Stackable" );
	if (stack > 0)
		Stackable = true;
	else
		Stackable = false;
	
	root.getValueByName( SapCost, "SapCost" );	
	root.getValueByName( HpCost, "HpCost" );
	root.getValueByName(SpellLevel, "SpellLevel");

	if (root.getValueByName( value, "Skill" ))
		Skill = SKILLS::toSkill(value);

	if (root.getValueByName( value, "Behaviour" ))
		Behaviour = MBEHAV::stringToBehaviour(value);

	// type specialization
	switch(type)
	{
	case AI_ACTION::DamageSpell:
	case AI_ACTION::DoTSpell:
		root.getValueByName( SpellParamValue2, "DamageVampirismValue" );
	case AI_ACTION::ToxicCloud:
		root.getValueByName( SpellParamValue, "DamageValue" );
		root.getValueByName( SpellPowerFactor, "SpellPowerFactor" );
		if ( root.getValueByName( value, "DamageType" ) )
			DamageType = DMGTYPE::stringToDamageType(value);
		if ( root.getValueByName( value, "DamageScore" ) )
			AffectedScore = SCORES::toScore(value);
		break;

	case AI_ACTION::HealSpell:
	case AI_ACTION::HoTSpell:
		root.getValueByName( SpellParamValue, "HealValue" );
		root.getValueByName( SpellPowerFactor, "SpellPowerFactor" );
		if ( root.getValueByName( value, "HealedScore" ) )
			AffectedScore = SCORES::toScore(value);
		break;

	case AI_ACTION::EffectSpell:
	case AI_ACTION::EoTSpell:
		root.getValueByName( SpellPowerFactor, "SpellPowerFactor" );
		root.getValueByName( SpellParamValue, "EffectValue" );
		root.getValueByName( SpellParamValue2, "EffectValue2" );
		break;

	default:
		break;
	};
	
} // CSpellParams::readForm //

//--------------------------------------------------------------
//					COTSpellParams::readForm
//--------------------------------------------------------------
void COTSpellParams::readForm (const UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type)
{
	CSpellParams::readForm(root,sheetId, type);
	
	float durationInSeconds;
	if ( root.getValueByName( durationInSeconds, "UpdateFrequency" ) )
		UpdateFrequency = (TGameCycle)(durationInSeconds / CTickEventHandler::getGameTimeStep());
	
	if (root.getValueByName( durationInSeconds, "Duration" ) )
		Duration = (TGameCycle)(durationInSeconds / CTickEventHandler::getGameTimeStep());

	string value;
	if (root.getValueByName( value, "DurationType" ))
		EffectDurationType= DURATION_TYPE::fromString(value);
} // COTSpellParams::readForm //

//--------------------------------------------------------------
//					CEffectSpellParams::serial
//--------------------------------------------------------------
void CEffectSpellParams::serial(class NLMISC::IStream &f)
{
	CSpellParams::serial(f);

	f.serial(Duration);

	if (f.isReading())
	{
		string val;
		f.serial(val);
		EffectFamily = AI_ACTION::toAiEffectType(val);
		f.serial(val);
		EffectDurationType = DURATION_TYPE::fromString(val);
	}
	else
	{
		string val = AI_ACTION::toString(EffectFamily);
		f.serial(val);
		val = DURATION_TYPE::toString(EffectDurationType);
		f.serial(val);
	}
} // CEffectSpellParams::serial //

//--------------------------------------------------------------
//					CEffectSpellParams::readForm
//--------------------------------------------------------------
void CEffectSpellParams::readForm (const UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type)
{
	CSpellParams::readForm(root,sheetId, type);

	float durationInSeconds;
	if (root.getValueByName( durationInSeconds, "EffectDuration" ) )
		Duration = (TGameCycle)(durationInSeconds / CTickEventHandler::getGameTimeStep());
	
	string value;
	if (root.getValueByName( value, "EffectType" ))
		EffectFamily = AI_ACTION::toAiEffectType(value);

	if (root.getValueByName( value, "EffectDurationType" ))
		EffectDurationType= DURATION_TYPE::fromString(value);

} // CEffectSpellParams::readForm //

//--------------------------------------------------------------
//					COTEffectSpellParams::serial
//--------------------------------------------------------------
void COTEffectSpellParams::serial(class NLMISC::IStream &f)
{
	COTSpellParams::serial(f);
		
	if (f.isReading())
	{
		string val;
		f.serial(val);
		EffectFamily = AI_ACTION::toAiEffectType(val);
	}
	else
	{
		string val = AI_ACTION::toString(EffectFamily);
		f.serial(val);
	}
} // COTEffectSpellParams::serial //

//--------------------------------------------------------------
//					COTEffectSpellParams::readForm
//--------------------------------------------------------------
void COTEffectSpellParams::readForm (const UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type)
{
	COTSpellParams::readForm(root,sheetId, type);
	
	string value;
	if (root.getValueByName( value, "EffectType" ))
	{
		EffectFamily = AI_ACTION::toAiEffectType(value);
	}
} // COTEffectSpellParams::readForm //

//--------------------------------------------------------------
//				TAiArea::serial
//--------------------------------------------------------------
void TAiArea::serial(class NLMISC::IStream &f)
{		
	if (f.isReading() )
	{
		string val;
		f.serial(val);
		AreaType = MAGICFX::toSpellMode(val);
	}
	else
	{
		string val = MAGICFX::toString(AreaType);
		f.serial(val);
	}
	
	if (AreaType == MAGICFX::UnknownSpellMode)
		return;
		
	if (AreaType == MAGICFX::Chain)
	{
		f.serial(AreaRange);
		f.serial(ChainFadingFactor);
		f.serial(ChainMaxTargets);
	}
	else if (AreaType == MAGICFX::Spray)
	{		
		f.serial(SprayHeight);
		f.serial(SprayBase);
		f.serial(SprayAngle);
		f.serial(SprayMaxTargets);
	}
	else if (AreaType == MAGICFX::Bomb)
	{
		f.serial(AreaRange);
		f.serial(BombMaxTargets);
	}
} // TAiArea::serial //



//--------------------------------------------------------------
//					TAiArea::readForm
//--------------------------------------------------------------
void TAiArea::readForm (const UFormElm &root, const NLMISC::CSheetId &sheetId, AI_ACTION::TAiActionType type)
{
	string value;

	if ( root.getValueByName( value, "AreaType" ) )
	{
		AreaType = MAGICFX::toSpellMode(value);

		switch(AreaType)
		{
		case MAGICFX::Bomb:
			root.getValueByName( AreaRange, "BombRadius" );
			root.getValueByName( BombMaxTargets, "MaxTarget" );
			break;
		case MAGICFX::Spray:
			root.getValueByName( SprayHeight, "SprayDistance" );
			root.getValueByName( SprayBase, "SprayWidth" );
			root.getValueByName( SprayAngle, "SprayAngle" );
			root.getValueByName( SprayMaxTargets, "MaxTarget" );
			break;
		case MAGICFX::Chain:
			root.getValueByName( AreaRange, "ChainDistance" );
			root.getValueByName( ChainMaxTargets, "ChainMaxTarget" );
			root.getValueByName( ChainFadingFactor, "ChainDamageFactor" );
			root.getValueByName( ChainMaxTargets, "MaxTarget" );
			break;
		default:
			break;
		};
	}	
} // TAiArea::readForm //



//--------------------------------------------------------------
//					CStaticAiAction::readGeorges
//--------------------------------------------------------------
void CStaticAiAction::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	if (!form) return;

	_SheetId = sheetId;

	const UFormElm &root = form->getRootNode();

	string value;
	root.getValueByName( value, "type" );
	
	_Type = AI_ACTION::toAiActionType(value);
	if (_Type == AI_ACTION::UnknownType)
	{
		nlwarning("<CStaticAiAction::readGeorges> Unknown Ai Action Type %s found in sheet %s", value.c_str(), sheetId.toString().c_str());
		return;
	}

	_Area.readForm(root,sheetId, _Type);

	switch(_Type)
	{
	case AI_ACTION::Melee:
	case AI_ACTION::Range:
		_Data.Combat.init();
		_Data.Combat.readForm(root,sheetId,_Type);
		break;

	case AI_ACTION::DamageSpell:
	case AI_ACTION::HealSpell:
		_Data.Spell.init();
		_Data.Spell.readForm(root,sheetId,_Type);
		break;

	case AI_ACTION::DoTSpell:
	case AI_ACTION::HoTSpell:
	case AI_ACTION::ToxicCloud:
		_Data.OTSpell.init();
		_Data.OTSpell.readForm(root,sheetId,_Type);
		break;
		
	case AI_ACTION::EffectSpell:
		_Data.EffectSpell.init();
		_Data.EffectSpell.readForm(root,sheetId,_Type);
		break;

	case AI_ACTION::EoTSpell:
		_Data.OTEffectSpell.init();
		_Data.OTEffectSpell.readForm(root,sheetId,_Type);
		break;

	default:
		break;
	};
	
} // CStaticAiAction::readGeorges //


//--------------------------------------------------------------
//					CStaticAiAction::readGeorges
//--------------------------------------------------------------
void CStaticAiAction::serial(class NLMISC::IStream &f)
{
	f.serial(_SheetId);
	
	if (f.isReading())
	{
		string val;
		f.serial(val);
		_Type = AI_ACTION::toAiActionType(val);		
	}
	else
	{
		string val = AI_ACTION::toString(_Type);
		f.serial(val);
	}

	switch(_Type)
	{
	case AI_ACTION::Melee:
	case AI_ACTION::Range:
		_Data.Combat.serial(f);
		break;

	case AI_ACTION::DamageSpell:
	case AI_ACTION::HealSpell:
		_Data.Spell.serial(f);
		break;

	case AI_ACTION::DoTSpell:
	case AI_ACTION::HoTSpell:
	case AI_ACTION::ToxicCloud:
		_Data.OTSpell.serial(f);
		break;

	case AI_ACTION::EffectSpell:
		_Data.EffectSpell.serial(f);
		break;

	case AI_ACTION::EoTSpell:
		_Data.OTEffectSpell.serial(f);
		break;

	default:
		break;
	};

	_Area.serial(f);	
	
} // CStaticAiAction::serial //
