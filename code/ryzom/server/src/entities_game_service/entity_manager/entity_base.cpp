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


/////////////
// INCLUDE //
/////////////
//Nel misc
#include "nel/misc/vector_2d.h"
#include "nel/misc/vectord.h"
#include "nel/misc/hierarchical_timer.h"

//Nel georges
#include "nel/georges/u_form.h"

//Game share
#include "game_share/slot_equipment.h"
#include "ai_share/ai_event.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/chat_group.h"
#include "server_share/effect_manager.h"
#include "game_share/visual_fx.h"

#include "server_share/r2_vision.h"

#include "entity_manager/entity_base.h"
#include "entity_manager/entity_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "creature_manager/creature.h"
#include "phrase_manager/s_effect.h"
#include "phrase_manager/s_link_effect.h"
#include "entity_matrix.h"
#include "building_manager/building_manager.h"
#include "player_manager/gm_tp_pending_command.h"
#include "zone_manager.h"
#include "world_instances.h"
#include "outpost_manager/outpost_manager.h"
#include "modules/shard_unifier_client.h"

#include "egs_sheets/egs_sheets.h"


// TEMP
#include "phrase_manager/combat_phrase.h"

extern CChangeActionFlagMsg ChangeActionFlagMsg; // Defined in entities_game_service.cpp

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CVariable<string>	NeverAggroPriv;

CEntityMatrix EntityMatrix;

CVariable<bool> EGSLight("egs","EGSLight", "Load EGS with a minimal set of feature loaded", false, 0, true);

//-----------------------------------------------
// CEquipmentSlots :
// Default constructor.
//-----------------------------------------------
CEquipmentSlots::CEquipmentSlots()
{
	Headdress	= SMirrorEquipment();
	Head		= SMirrorEquipment();
	Face		= SMirrorEquipment();
	EarL		= SMirrorEquipment();
	EarR		= SMirrorEquipment();
	Neck		= SMirrorEquipment();
	Shoulders	= SMirrorEquipment();
	Back		= SMirrorEquipment();
	Chest		= SMirrorEquipment();
	Arms		= SMirrorEquipment();
	WristL		= SMirrorEquipment();
	WristR		= SMirrorEquipment();
	Hands		= SMirrorEquipment();
	FingerL		= SMirrorEquipment();
	FingerR		= SMirrorEquipment();
	Legs		= SMirrorEquipment();
	AnkleL		= SMirrorEquipment();
	AnkleR		= SMirrorEquipment();
	Feet		= SMirrorEquipment();

	for( int i = 0; i < NB_SHEATH; ++i )
	{
		Sheath[ i ].HandL = SMirrorEquipment();
		Sheath[ i ].HandR = SMirrorEquipment();
		Sheath[ i ].Ammo = SMirrorEquipment();
	}
}


//-----------------------------------------------
// serial CEquipmentSlots properties:
//
//-----------------------------------------------
void CEquipmentSlots::serial( NLMISC::IStream &f ) throw(NLMISC::EStream)
{
	f.serial( Headdress );
	f.serial( Head );
	f.serial( Face );
	f.serial( EarL );
	f.serial( EarR );
	f.serial( Neck );
	f.serial( Shoulders );
	f.serial( Back );
	f.serial( Chest );
	f.serial( Arms );
	f.serial( WristL );
	f.serial( WristR );
	f.serial( Hands );
	f.serial( FingerL );
	f.serial( FingerR );
	f.serial( Legs );
	f.serial( AnkleL );
	f.serial( AnkleR );
	f.serial( Feet );

	for( int i = 0; i < NB_SHEATH; ++i )
	{
		f.serial( Sheath[ i ].HandL );
		f.serial( Sheath[ i ].HandR );
		f.serial( Sheath[ i ].Ammo );
	}
} // CEquipmentSlots::serial //


//-----------------------------------------------
// CEntityBase :
// Default constructor.
//-----------------------------------------------
CEntityBase::CEntityBase(bool noSkills)
:CEntityBasePersistantData(noSkills)
{
	_ListLink.setEntity(this);
	_SheetId= CSheetId().asInt();

	_Name = "";
//	_Surname = "";

	_Race = EGSPD::CPeople::EndPeople;
	_Gender = 0;
	
	_Behaviour= MBEHAV::IDLE;
	_Mode= MBEHAV::TMode( MBEHAV::NORMAL, 0.0f );
	_InstanceNumber= INVALID_AI_INSTANCE;

	_EntityMounted=TDataSetRow();

	_Stunned= false;

	// range 0..1023
	_StatusBars= 1023;

	// action flags
	_ActionFlags= 0;

	_ProtectedSlot = SLOT_EQUIPMENT::UNDEFINED;
	_DodgeAsDefense = true;

	_IsDead = false;
	_PreventEntityMoves = 0;

	_DamageShieldDamage = 0;
	_DamageShieldHpDrain = 0;

	_MezzCount = 0;
	// god mod inactive
	_GodMode = false;

	// invulnerable mode inactive
	_Invulnerable = false;

	_EventSpeedVariationModifier = 1.f;
}


// clear() method used by apply() method
void CEntityBase::clear()
{
	_InstanceNumber= INVALID_AI_INSTANCE;
	_EntityMounted= TDataSetRow();
	_ProtectedSlot= SLOT_EQUIPMENT::UNDEFINED;
	_PreventEntityMoves= 0;
	_DamageShieldDamage= 0;
	_DamageShieldHpDrain= 0;
	_MezzCount= 0;
	_Stunned= false;	
	_IsDead= false;
	_Behaviour= MBEHAV::IDLE;
	_Mode= MBEHAV::TMode( MBEHAV::NORMAL, 0.0f );

	_SheetId=CSheetId().asInt();
	_EntityState.clear();
	_Name="";
	_Race = EGSPD::CPeople::EndPeople;
	_Gender = 0;
	_Size= 0;
	_DodgeAsDefense= false;
	_PhysCharacs.clear();
	_PhysScores.clear();
	_Skills.clear();

	_SpecialModifiers.MeleeAttackModifierOnEnemy= 0;
	_SpecialModifiers.MeleeAttackModifierOnSelf= 0;
	_SpecialModifiers.MagicCastingModifierOnSelf= 0;
	_SpecialModifiers.MagicCastingModifierOnEnemy= 0;
	_SpecialModifiers.RangeAttackModifierOnEnemy= 0;
	_SpecialModifiers.RangeAttackModifierOnSelf= 0;
	_SpecialModifiers.AttackModifierOnSelf= 0;
	_SpecialModifiers.ChanceToFailStrategy= 0;
	_SpecialModifiers.ChanceToFailSpell= 0;
	_SpecialModifiers.ChanceToFailFaber= 0;
	_SpecialModifiers.ChanceToFailHarvest= 0;
	_SpecialModifiers.ChanceToFailTracking= 0;
	_SpecialModifiers.MeleeAttackSlow= 0;
	_SpecialModifiers.MeleeSlashingDamageArmor= 0;
	_SpecialModifiers.MeleeBluntDamageArmor= 0;
	_SpecialModifiers.MeleePiercingDamageArmor= 0;
	_SpecialModifiers.MeleeDamageModifierFactor= 0;
	_SpecialModifiers.RangeDamageModifierFactor= 0;
	_SpecialModifiers.CreatureMeleeTakenDamageFactor= 0;
	_SpecialModifiers.CreatureRangeTakenDamageFactor= 0;
	_SpecialModifiers.CombatBrickLatencyMultiplier= 0;
	_SpecialModifiers.MagicBrickLatencyMultiplier= 0;
	_SpecialModifiers.ArmorQualityModifier= 0;
	_SpecialModifiers.WeaponQualityModifier= 0;
	_SpecialModifiers.ArmorAbsorbtionMultiplier= 0;
}


void CEntityBase::setName( const ucstring& name ) 
{ 
	_Name = name; 
}



//-----------------------------------------------
// addPropertiesToMirror :
// 
//-----------------------------------------------
void CEntityBase::addPropertiesToMirror( const TDataSetRow& entityIndex, bool keepSheetId )
{
//	nldebug( "Adding properties of E%u (%s) to mirror", entityIndex.getIndex(), _Id.toString().c_str() );

	nlassert( _Id != CEntityId::Unknown ); // must fail when "transport class msg" is received before adding on the physical machine
	_EntityRowId = entityIndex;

	// The entity state is set here but in mirrorizeEntityState()

	if ( keepSheetId )
		_SheetId.tempMirrorize( TheDataset, entityIndex, DSPropertySHEET ); // put the value in the mirror
	else
	{
		_SheetId.init( TheDataset, entityIndex, DSPropertySHEET ); // use the value already in the mirror or set it later
	}

	/// physical characteristics
	_PhysScores._PhysicalScores[ SCORES::hit_points ].Max.tempMirrorize( TheDataset, entityIndex, DSPropertyMAX_HIT_POINTS );
	_PhysScores._PhysicalScores[ SCORES::hit_points ].Current.tempMirrorize( TheDataset, entityIndex, DSPropertyCURRENT_HIT_POINTS );
	
	/// current run speed
	_PhysScores.CurrentRunSpeed.tempMirrorize( TheDataset, entityIndex, DSPropertyCURRENT_RUN_SPEED );
	_PhysScores.CurrentWalkSpeed.tempMirrorize( TheDataset, entityIndex, DSPropertyCURRENT_WALK_SPEED );

	/// stunned
	_Stunned.tempMirrorize( TheDataset, entityIndex, DSPropertySTUNNED );

	// Mode
	if( _Id.getType() == RYZOMID::player )
	{
		MBEHAV::TMode mode = _Mode;
		mode.setModeAndPos( (MBEHAV::EMode)mode.Mode, entityIndex );
		_Mode= mode;
		_Mode.tempMirrorize( TheDataset, entityIndex, DSPropertyMODE );

		_InstanceNumber.tempMirrorize( TheDataset, entityIndex, DSPropertyAI_INSTANCE );
	}
	else
	{
		_Mode.init( TheDataset, entityIndex, DSPropertyMODE );

		_InstanceNumber.init( TheDataset, entityIndex, DSPropertyAI_INSTANCE );
	}

	// Behaviour
	_Behaviour.tempMirrorize( TheDataset, entityIndex, DSPropertyBEHAVIOUR );

	
	// Current target
	_Target.init( TheDataset, entityIndex, DSPropertyTARGET_ID );

	//	Steph modification, only do this on players!
	if (getId().getType()==RYZOMID::player)
		_Target=TDataSetRow();
	
	// Current mounted
	_EntityMounted.tempMirrorize( TheDataset, entityIndex, DSPropertyENTITY_MOUNTED_ID );
	
	// Current rider
	_RiderEntity.init( TheDataset, entityIndex, DSPropertyRIDER_ENTITY_ID );
	_RiderEntity=TDataSetRow();

	// initialize the WhoSeesMe bitfield (every bit set to 1)
	_WhoSeesMe.init( TheDataset, entityIndex, DSPropertyWHO_SEES_ME );
	const uint64 bitfield = IsRingShard? R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_PLAYER,true): UINT64_CONSTANT(0xffffffffffffffff);
	_WhoSeesMe = bitfield;

	// the bars on entity head (life bar as a visual property)
	_StatusBars.tempMirrorize( TheDataset, entityIndex, DSPropertyBARS );

	_ActionFlags.tempMirrorize( TheDataset, entityIndex, DSPropertyACTION_FLAGS );

	_OwnerPeople.init( TheDataset, entityIndex, DSPropertyOWNER_PEOPLE );
	_OwnerPeople = MOUNT_PEOPLE::Unknown;

} // addPropertiesToMirror //


/*
 * Add the current entity state into the mirror
 */
void CEntityBase::mirrorizeEntityState( bool copyValueToMirror, TDataSetRow entityIndex ) // can't be by ref
{
	if( !TheDataset.isAccessible(entityIndex) )
	{
		entityIndex = TheDataset.getDataSetRow( _Id );
	}

	if ( copyValueToMirror )
	{
		 _EntityState.X.tempMirrorize( TheDataset, entityIndex, DSPropertyPOSX );
		 _EntityState.Y.tempMirrorize( TheDataset, entityIndex, DSPropertyPOSY );
		 _EntityState.Z.tempMirrorize( TheDataset, entityIndex, DSPropertyPOSZ );
		 _EntityState.Heading.tempMirrorize( TheDataset, entityIndex, DSPropertyORIENTATION );
	}
	else
	{
		_EntityState.X.init( TheDataset, entityIndex, DSPropertyPOSX );
		_EntityState.Y.init( TheDataset, entityIndex, DSPropertyPOSY );
		_EntityState.Z.init( TheDataset, entityIndex, DSPropertyPOSZ );
		_EntityState.Heading.init( TheDataset, entityIndex, DSPropertyORIENTATION );
	}
}


//-----------------------------------------------
// loadSheetCharacter: Load George Sheet
// 
//-----------------------------------------------
void CEntityBase::loadSheetEntity( const CSheetId& sheetId )
{
	_SheetId= sheetId.asInt(); // why??
}

//-----------------------------------------------
// setTarget
//
//-----------------------------------------------
void CEntityBase::setTarget( const NLMISC::CEntityId& targetId, bool sendMessage )
{ 
	nlassert(getId().getType()==RYZOMID::player);
	_Target = TheDataset.getDataSetRow( targetId );
}

//---------------------------------------------------
// lookupStat :
//
//---------------------------------------------------
sint32& CEntityBase::lookupStat( const string& var ) throw (CEntityBase::EInvalidStat)
{
	// TODO Alain: optimize this...
	uint i;
	for(i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		if( var == string("Base") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics) i ) )
		{
			return _PhysCharacs._PhysicalCharacteristics[ i ].Base;
		}
		else if( var == string("Max") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics) i ) )
		{
			_PhysCharacs._PhysicalCharacteristics[ i ].Max.setChanged();
			return _PhysCharacs._PhysicalCharacteristics[ i ].Max.directAccessForStructMembers();
		}
		else if( var == string("Modifier") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics) i ) )
		{
			return _PhysCharacs._PhysicalCharacteristics[ i ].Modifier;
		}
		else if( var == string("Current") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics) i ) )
		{
			_PhysCharacs._PhysicalCharacteristics[ i ].Current.setChanged();
			return _PhysCharacs._PhysicalCharacteristics[ i ].Current.directAccessForStructMembers();
		}
/*		else if( var == string("BaseRegenerate") + CHARACTERISTICS::toString( i ) )
		{
			return _PhysCharacs._PhysicalCharacteristics[ i ].BaseRegenerate;
		}
		else if( var == string("RegenerateModifier") + CHARACTERISTICS::toString( i ) )
		{
			return _PhysCharacs._PhysicalCharacteristics[ i ].RegenerateModifier;
		}
		else if( var == string("CurrentRegenerate") + CHARACTERISTICS::toString( i ) )
		{
			return _PhysCharacs._PhysicalCharacteristics[ i ].CurrentRegenerate;
		}
*/	}

	for(i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		if( var == string("Base") + SCORES::toString( i ) )
		{
			return _PhysScores._PhysicalScores[ i ].Base;
		}
		else if( var == string("Max") + SCORES::toString( i ) )
		{
			_PhysScores._PhysicalScores[ i ].Max.setChanged();
			return _PhysScores._PhysicalScores[ i ].Max.directAccessForStructMembers();
		}
		else if( var == string("Modifier") + SCORES::toString( i ) )
		{
			return _PhysScores._PhysicalScores[ i ].Modifier;
		}
		else if( var == string("Current") + SCORES::toString( i ) )
		{
			_PhysScores._PhysicalScores[ i ].Current.setChanged();
			return _PhysScores._PhysicalScores[ i ].Current.directAccessForStructMembers();
		}
/*		else if( var == string("BaseRegenerate") + SCORES::toString( i ) )
		{
			return _PhysScores._PhysicalScores[ i ].BaseRegenerate;
		}
		else if( var == string("RegenerateModifier") + SCORES::toString( i ) )
		{
			return _PhysScores._PhysicalScores[ i ].RegenerateModifier;
		}
		else if( var == string("CurrentRegenerate") + SCORES::toString( i ) )
		{
			return _PhysScores._PhysicalScores[ i ].CurrentRegenerate;
		}
*/	}

	for(i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		if( var == string("Current") + SKILLS::toString( i ) )
		{
			return _Skills._Skills[ i ].Current;
		}
		else if( var == string("Base") + SKILLS::toString( i ) )
		{
			return _Skills._Skills[ i ].Base;
		}
		else if( var == string("Modifier") + SKILLS::toString( i ) )
		{
			return _Skills._Skills[ i ].Modifier;
		}
	}

	if (var == string("SpeedVariationModifier") )
	{
		return _PhysScores.SpeedVariationModifier;
	}

	if (var == string("MeleeAttackModifierOnSelf") )
		return _SpecialModifiers.MeleeAttackModifierOnSelf;

	if (var == string("MeleeAttackModifierOnEnemy") )
		return _SpecialModifiers.MeleeAttackModifierOnEnemy;
	
	if (var == string("RangeAttackModifierOnSelf") )
		return _SpecialModifiers.RangeAttackModifierOnSelf;

	if (var == string("AttackModifierOnSelf") )
		return _SpecialModifiers.AttackModifierOnSelf;
	
	if (var == string("RangeAttackModifierOnEnemy") )
		return _SpecialModifiers.RangeAttackModifierOnEnemy;

	if (var == string("MagicCastingModifierOnSelf") )
		return _SpecialModifiers.MagicCastingModifierOnSelf;

	if (var == string("MagicCastingModifierOnEnemy") )
		return _SpecialModifiers.MagicCastingModifierOnEnemy;

	if (var == string("ChanceToFailStrategy") )
		return _SpecialModifiers.ChanceToFailStrategy;
	if (var == string("ChanceToFailSpell") )
		return _SpecialModifiers.ChanceToFailSpell;
	if (var == string("ChanceToFailFaber") )
		return _SpecialModifiers.ChanceToFailFaber;
	if (var == string("ChanceToFailHarvest") )
		return _SpecialModifiers.ChanceToFailHarvest;
	if (var == string("ChanceToFailTracking") )
		return _SpecialModifiers.ChanceToFailTracking;
	if (var == string("MeleeAttackSlow") )
		return _SpecialModifiers.MeleeAttackSlow;

	if (var == string("MeleeSlashingDamageArmor") )
		return _SpecialModifiers.MeleeSlashingDamageArmor;
	if (var == string("MeleeBluntDamageArmor") )
		return _SpecialModifiers.MeleeBluntDamageArmor;
	if (var == string("MeleePiercingDamageArmor") )
		return _SpecialModifiers.MeleePiercingDamageArmor;

	if (var == string("MeleeDamageModifierFactor") )
		return _SpecialModifiers.MeleeDamageModifierFactor;
	if (var == string("RangeDamageModifierFactor") )
		return _SpecialModifiers.RangeDamageModifierFactor;

	if (var == string("CreatureMeleeTakenDamageFactor") )
		return _SpecialModifiers.CreatureMeleeTakenDamageFactor;
	if (var == string("CreatureRangeTakenDamageFactor") )
		return _SpecialModifiers.CreatureRangeTakenDamageFactor;

	if (var == string("CombatBrickLatencyMultiplier") )
		return _SpecialModifiers.CombatBrickLatencyMultiplier;	
	if (var == string("MagicBrickLatencyMultiplier") )
		return _SpecialModifiers.MagicBrickLatencyMultiplier;	

	if (var == string("ArmorQualityModifier") )
		return _SpecialModifiers.ArmorQualityModifier;		

	if (var == string("WeaponQualityModifier") )
		return _SpecialModifiers.WeaponQualityModifier;	

	if (var == string("ArmorAbsorbtionMultiplier") )
		return _SpecialModifiers.ArmorAbsorbtionMultiplier;	
	
	
	// ERROR -> throw exception
	throw CEntityBase::EInvalidStat() ;
} // lookupStat //


//---------------------------------------------------
// lookupStat for Characteristics:
//
//---------------------------------------------------
sint32& CEntityBase::lookupStat( CHARACTERISTICS::TCharacteristics c, SCharacteristicsAndScores::TCharacteristicsAndScoreSubType st ) throw (CEntityBase::EInvalidStat)
{
	if( c < CHARACTERISTICS::NUM_CHARACTERISTICS )
	{
		switch( st )
		{
			case SCharacteristicsAndScores::base:
				return _PhysCharacs._PhysicalCharacteristics[ c ].Base;
			case SCharacteristicsAndScores::max:
				{
					_PhysCharacs._PhysicalCharacteristics[ c ].Max.setChanged();
					return _PhysCharacs._PhysicalCharacteristics[ c ].Max.directAccessForStructMembers();
				}
			case SCharacteristicsAndScores::modifier:
				return _PhysCharacs._PhysicalCharacteristics[ c ].Modifier;
			case SCharacteristicsAndScores::current:
				{
					_PhysCharacs._PhysicalCharacteristics[ c ].Current.setChanged();
					return _PhysCharacs._PhysicalCharacteristics[ c ].Current.directAccessForStructMembers();
				}
/*			case SCharacteristicsAndScores::base_regenerate:
				return _PhysCharacs._PhysicalCharacteristics[ c ].BaseRegenerate;
			case SCharacteristicsAndScores::regenerate_modifier:
				return _PhysCharacs._PhysicalCharacteristics[ c ].RegenerateModifier;
			case SCharacteristicsAndScores::current_regenerate:
				return _PhysCharacs._PhysicalCharacteristics[ c ].CurrentRegenerate;
*/			default:
				break;
		}
	}
	// ERROR -> throw exception
	throw CEntityBase::EInvalidStat() ;
}


//---------------------------------------------------
// lookupStat for Scores:
//
//---------------------------------------------------
sint32& CEntityBase::lookupStat( SCORES::TScores score, SCharacteristicsAndScores::TCharacteristicsAndScoreSubType st ) throw (CEntityBase::EInvalidStat)
{
	if( score < SCORES::NUM_SCORES )
	{
		switch( st )
		{
			case SCharacteristicsAndScores::base:
				return _PhysScores._PhysicalScores[ score ].Base;
			case SCharacteristicsAndScores::max:
				{
					_PhysScores._PhysicalScores[ score ].Max.setChanged();
					return _PhysScores._PhysicalScores[ score ].Max.directAccessForStructMembers();
				}
			case SCharacteristicsAndScores::modifier:
				return _PhysScores._PhysicalScores[ score ].Modifier;
			case SCharacteristicsAndScores::current:
				{
					_PhysScores._PhysicalScores[ score ].Current.setChanged();
					return _PhysScores._PhysicalScores[ score ].Current.directAccessForStructMembers();
				}
/*			case SCharacteristicsAndScores::base_regenerate:
				return _PhysScores._PhysicalScores[ score ].BaseRegenerate;
			case SCharacteristicsAndScores::regenerate_modifier:
				return _PhysScores._PhysicalScores[ score ].RegenerateModifier;
			case SCharacteristicsAndScores::current_regenerate:
				return _PhysScores._PhysicalScores[ score ].CurrentRegenerate;
*/			default:
				break;
		}
	}
	// ERROR -> throw exception
	throw CEntityBase::EInvalidStat() ;
}


//---------------------------------------------------
// lookupStat for Skills:
//
//---------------------------------------------------
sint32& CEntityBase::lookupStat( SKILLS::ESkills skill, SSkill::ESkillSubType st ) throw (CEntityBase::EInvalidStat)
{
/*	enum ESkillSubType 
	{ 
		base = 0, 
		modifier,
		current,
		maxLvlReached,
	};
*/	
	if( skill < SKILLS::NUM_SKILLS )
	{
		switch( st )
		{
			case SSkill::base: 
				return _Skills._Skills[ skill ].Base;
			case SSkill::modifier:
				return _Skills._Skills[ skill ].Modifier;
			case SSkill::current:
				return _Skills._Skills[ skill ].Current;
			case SSkill::maxLvlReached:
				return _Skills._Skills[ skill ].MaxLvlReached;
			default:
				break;
		}
	}
	// ERROR -> throw exception
	throw CEntityBase::EInvalidStat() ;
}


//---------------------------------------------------
// lookupStat for SpecialModifiers:
//
//---------------------------------------------------
sint32& CEntityBase::lookupStat( CSpecialModifiers::ESpecialModifiers sm ) throw (CEntityBase::EInvalidStat)
{
	switch( sm )
	{
		case CSpecialModifiers::speed_variation_modifier:
			return _PhysScores.SpeedVariationModifier;
		case CSpecialModifiers::melee_attack_modifier_on_self:
			return _SpecialModifiers.MeleeAttackModifierOnSelf;
		case CSpecialModifiers::melee_attack_modifier_on_enemy:
			return _SpecialModifiers.MeleeAttackModifierOnEnemy;
		case CSpecialModifiers::range_attack_modifier_on_self:
			return _SpecialModifiers.RangeAttackModifierOnSelf;
		case CSpecialModifiers::attack_modifier_on_self:
			return _SpecialModifiers.AttackModifierOnSelf;
		case CSpecialModifiers::range_attack_modifier_on_enemy:
			return _SpecialModifiers.RangeAttackModifierOnEnemy;
		case CSpecialModifiers::magic_casting_modifier_on_self:
			return _SpecialModifiers.MagicCastingModifierOnSelf;
		case CSpecialModifiers::magic_casting_modifier_on_enemy:
			return _SpecialModifiers.MagicCastingModifierOnEnemy;
		case CSpecialModifiers::chance_to_fail_strategy:
			return _SpecialModifiers.ChanceToFailStrategy;
		case CSpecialModifiers::chance_to_fail_spell:
			return _SpecialModifiers.ChanceToFailSpell;
		case CSpecialModifiers::chance_to_fail_faber:
			return _SpecialModifiers.ChanceToFailFaber;
		case CSpecialModifiers::chance_to_fail_harvest:
			return _SpecialModifiers.ChanceToFailHarvest;
		case CSpecialModifiers::chance_to_fail_tracking:
			return _SpecialModifiers.ChanceToFailTracking;
		case CSpecialModifiers::melee_attack_slow:
			return _SpecialModifiers.MeleeAttackSlow;
		case CSpecialModifiers::melee_slashing_damage_armor:
			return _SpecialModifiers.MeleeSlashingDamageArmor;
		case CSpecialModifiers::melee_blunt_damage_armor:
			return _SpecialModifiers.MeleeBluntDamageArmor;
		case CSpecialModifiers::melee_piercing_damage_armor:
			return _SpecialModifiers.MeleePiercingDamageArmor;
		case CSpecialModifiers::melee_damage_modifier_factor:
			return _SpecialModifiers.MeleeDamageModifierFactor;
		case CSpecialModifiers::range_damage_modifier_factor:
			return _SpecialModifiers.RangeDamageModifierFactor;
		case CSpecialModifiers::creature_melee_taken_damage_factor:
			return _SpecialModifiers.CreatureMeleeTakenDamageFactor;
		case CSpecialModifiers::creature_range_taken_damage_factor:
			return _SpecialModifiers.CreatureRangeTakenDamageFactor;
		case CSpecialModifiers::combat_brick_latency_multiplier:
			return _SpecialModifiers.CombatBrickLatencyMultiplier;	
		case CSpecialModifiers::magic_brick_latency_multiplier:
			return _SpecialModifiers.MagicBrickLatencyMultiplier;	
		case CSpecialModifiers::armor_quality_modifier:
			return _SpecialModifiers.ArmorQualityModifier;		
		case CSpecialModifiers::weapon_quality_modifier:
			return _SpecialModifiers.WeaponQualityModifier;	
		case CSpecialModifiers::armor_absorbtion_multiplier:
			return _SpecialModifiers.ArmorAbsorbtionMultiplier;	
		default:
			break;
	};
	// ERROR -> throw exception
	throw CEntityBase::EInvalidStat() ;
} // lookupStat //

const sint32& CEntityBase::lookupStat( CSpecialModifiers::ESpecialModifiers sm ) const throw (CEntityBase::EInvalidStat)
{
	return const_cast<CEntityBase*>(this)->lookupStat(sm);
}


//---------------------------------------------------
// changeCurrentHp :
//
//---------------------------------------------------

// the fact that this returns "true" to indicate that the entity dies is really, really dumb.
bool CEntityBase::changeCurrentHp(sint32 deltaValue, TDataSetRow responsibleEntity)
{
	// test entity isn't dead already (unless it's a player)
	if	(isDead())
	{
		return false;
	}

	// entity is taking damage
	if (deltaValue < 0)
	{
		// if entity is invincible, damage are reduced to 0
		const CSEffect *invincible = lookForActiveEffect( EFFECT_FAMILIES::Invincibility);
		if (invincible)
		{
			deltaValue = 0;
			return false;
		}

		// if a reverse damage effect is on this entity, inverse damage
		const CSEffect *effect = lookForActiveEffect( EFFECT_FAMILIES::ReverseDamage);
		if (effect)
		{
			deltaValue = -deltaValue;
		}
	}

	_PhysScores._PhysicalScores[SCORES::hit_points].Current = _PhysScores._PhysicalScores[SCORES::hit_points].Current + deltaValue;

	// if entity is mezzed and delta is != 0 unmezz it
	if (_MezzCount && deltaValue != 0)
	{
		unmezz();
	}
	
	if (_PhysScores._PhysicalScores[SCORES::hit_points].Current <= 0)
	{
		// for god mode
		if (!_GodMode && !_Invulnerable)
		{				
			kill(responsibleEntity);
			
			CCreature *c = dynamic_cast<CCreature *>(this);
			if (c!=NULL)
				c->setUpdateNextTick();
			
			return true;
		}
		else
		{
			_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Base;
			setHpBar( 1023 );
			return false;
		}
	}
	else
	{
		if( _PhysScores._PhysicalScores[SCORES::hit_points].Max > 0)
			setHpBar( (uint32) ( (1023 * _PhysScores._PhysicalScores[SCORES::hit_points].Current) / _PhysScores._PhysicalScores[SCORES::hit_points].Max) );
		else
			setHpBar(0);

		return false;
	}
}

//---------------------------------------------------
// setActionFlag :
// 
//---------------------------------------------------
void CEntityBase::setActionFlag( RYZOMACTIONFLAGS::TActionFlag flag, bool value ) 
{
	if ( !_ActionFlags.isInitialized()) return;

//		nldebug("Entity %s, set ActionFlag %s to %d", _Id.toString().c_str(), RYZOMACTIONFLAGS::toString(flag).c_str(), value);
	/*
	if	(value)
		_ActionFlags = (_ActionFlags.getValue() | flag );
	else
		_ActionFlags = (_ActionFlags.getValue() & (~flag) );
		*/

	ChangeActionFlagMsg.push(_EntityRowId, flag,value);
}

//---------------------------------------------------
// setValue :
//
//---------------------------------------------------
bool CEntityBase::setValue( const string& var, const string& value )
{
	//		 _Behaviour.getValue().Behaviour < RESURECTED
	//		||	_Behaviour.getValue().Behaviour > MBEHAV::PERMANENT_DEATH )

	//	not dead.
	if	(!_IsDead)
	{
		try
		{
			nldebug( "CEntityBase::setValue setting value %s to %s", var.c_str(),value.c_str() );
			sint32 &temp = lookupStat(var);
			sint32 v;
			NLMISC::fromString(value, v);
			if( v < 0 )
			{
				v = 0;
			}
			temp = v;
		}
		catch(const CEntityBase::EInvalidStat &e)
		{
			nlwarning("<CEntityBase::setValue> Exception : %s",e.what( var ) );
			return false;
		}
	}
	return true;
} // setValue //


//---------------------------------------------------
// modifyValue :
//
//---------------------------------------------------
bool CEntityBase::modifyValue( const string& var, const string& value )
{
//	if( _Behaviour.getValue().Behaviour < MBEHAV::RESURECTED || _Behaviour.getValue().Behaviour > MBEHAV::PERMANENT_DEATH )
		
	if(!_IsDead )
	{
		try
		{
			sint32 &temp = lookupStat(var);
			sint32 v;
			NLMISC::fromString(value, v);
			sint32 oldValue = temp;
			temp = temp + v;
			//nlinfo(" Modify value %s of %s for entity %s, old value %d, new value %d", var.c_str(), value.c_str(), _Id.toString().c_str(), oldValue, temp.getValue() );
		}
		catch(const CEntityBase::EInvalidStat &e)
		{
			nlwarning("<CEntityBase::modifyValue> Exception : %s",e.what( var ) );
			return false;
		}
	}
	else
	{
		//nlinfo(" modify value for entity %s but it's dead", _Id.toString().c_str() );
	}
	return true;
} // setValue //


//---------------------------------------------------
// getValue :
//
//---------------------------------------------------
bool CEntityBase::getValue( const string& var, string& value )
{
	try
	{
		sint32 val = lookupStat(var);
		value = toString(val);
	}
	catch(const CEntityBase::EInvalidStat &e)
	{
		if( var == string("Behaviour") )
		{
			value = toString( _Behaviour.getValue() );
			return true;
		}
		else if( var == string("Mode") )
		{
			value = toString( _Mode.getValue().Mode );
			return true;
		}
		else if( var == string("Target") )
		{
			value = toString( _Target.getValue() );
			return true;
		}
		nlwarning("<CEntityBase::getValue> Exception : %s",e.what( var ) );
		return false;
	}
	return true;
} // getValue //

//---------------------------------------------------
// manage effects, remove effects with no time duration left, apply effects...
//
//---------------------------------------------------
void CEntityBase::tickEffectUpdate()
{
	// now use CTimer to manage events for effects
/*	if (_SEffects.empty())
		return;

	{
		H_AUTO(EntityBaseTickEffectUpdate);
		
		uint32 effectFlag = 0;
		for (uint i = 0; i < _SEffects.size();  )
		{
			if ( _SEffects[i] )
			{
				// prevent effect to be deleted during process by keeping a smart pointer on it
				CSEffectPtr effect = _SEffects[i];

				if ( effect->isTimeToUpdate() )
				{
					if ( effect->update( effectFlag ) )
					{
						// check entity hasn't been killed by this effect, in such a case stop processing effects
						if (_IsDead)
							return;

						// remove the effect (dont increment i because the effect vector is modified)
						removeSabrinaEffect(effect);
						continue;
					}
				}
				i++;
			}
			else
			{
				nlwarning("<CEntityBase tickEffectUpdate effect %u is NULL. Debug",i);
				i++;
			}
		}
	}
*/
} // tickEffectUpdate //


//---------------------------------------------------
// Set all entity stats modifiers to initial states
//
//---------------------------------------------------
void CEntityBase::resetEntityModifier()
{
	H_AUTO(EntityBaseResetEntityModifier);

	_PhysScores.SpeedVariationModifier = 0;
	float Score2 = 1.2f;
	float Regen2 = 1.2f;

	if( _Id.getType() == RYZOMID::player )
	{
		const CStaticRaceStats* raceStat = CSheets::getRaceStats( CSheetId(_SheetId) );
		if( raceStat )
		{
			Score2 = raceStat->ProgressionScore2;
			Regen2 = raceStat->ProgressionRegen2;
		}
	}

	// init special modifier
	_SpecialModifiers.init();

	int i;
	// Set characteristics modifier to zero
	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		_PhysCharacs._PhysicalCharacteristics[ i ].Modifier = 0;
		_PhysCharacs._PhysicalCharacteristics[ i ].RegenerateModifier = 0;
	}

	// Set scores modifier to zero
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		_PhysScores._PhysicalScores[ i ].Modifier = 0;
		_PhysScores._PhysicalScores[ i ].RegenerateModifier = 0;
	}

	// Set Skill modifier to zero
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		_Skills._Skills[ i ].Modifier = 0;
	}

	// Apply effect product by equipped items on local variables if race is homin
	if( _Race < EGSPD::CPeople::EndHumanoid )
	{
		for( i = 0; i < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++i )
		{
			CSheetId id;
			uint16 quality = 0;
			if( _Id.getType() == RYZOMID::player )
			{
/*				CGameItemPtr item = ( ( CCharacter * ) this )->getInventory()[ INVENTORIES::equipment ]()->getChildren()[ i ];
				if( item!=NULL )
				{
					id = const_cast< CSheetId& >((*item).getSheetId());
					quality = (*item).quality();
				}
*/			}
			else
			{
				switch( i )
				{
					case SLOT_EQUIPMENT::HEADDRESS:
						id = _Items.Headdress.IdSheet;
						quality = _Items.Headdress.Quality;
						break;
					case SLOT_EQUIPMENT::HEAD:
						id = _Items.Head.IdSheet;
						quality = _Items.Head.Quality;
						break;
					case SLOT_EQUIPMENT::EARL:
						id = _Items.EarL.IdSheet;
						quality = _Items.EarL.Quality;
						break;
					case SLOT_EQUIPMENT::EARR:
						id = _Items.EarR.IdSheet;
						quality = _Items.EarR.Quality;
						break;
					case SLOT_EQUIPMENT::NECKLACE:
						id = _Items.Neck.IdSheet;
						quality = _Items.Neck.Quality;
						break;
					case SLOT_EQUIPMENT::CHEST:
						id = _Items.Chest.IdSheet;
						quality = _Items.Chest.Quality;
						break;
					case SLOT_EQUIPMENT::ARMS:
						id = _Items.Arms.IdSheet;
						quality = _Items.Arms.Quality;
						break;
					case SLOT_EQUIPMENT::WRISTL:
						id = _Items.WristL.IdSheet;
						quality = _Items.WristL.Quality;
						break;
					case SLOT_EQUIPMENT::WRISTR:
						id = _Items.WristR.IdSheet;
						quality = _Items.WristR.Quality;
						break;
					case SLOT_EQUIPMENT::HANDS:
						id = _Items.Hands.IdSheet;
						quality = _Items.Hands.Quality;
						break;
					case SLOT_EQUIPMENT::FINGERL:
						id = _Items.FingerL.IdSheet;
						quality = _Items.FingerL.Quality;
						break;
					case SLOT_EQUIPMENT::FINGERR:
						id = _Items.FingerR.IdSheet;
						quality = _Items.FingerR.Quality;
						break;
					case SLOT_EQUIPMENT::LEGS:
						id = _Items.Legs.IdSheet;
						quality = _Items.Legs.Quality;
						break;
					case SLOT_EQUIPMENT::ANKLEL:
						id = _Items.AnkleL.IdSheet;
						quality = _Items.AnkleL.Quality;
						break;
					case SLOT_EQUIPMENT::ANKLER:
						id = _Items.AnkleR.IdSheet;
						quality = _Items.AnkleR.Quality;
						break;
					case SLOT_EQUIPMENT::FEET:
						id = _Items.Feet.IdSheet;
						quality = _Items.Feet.Quality;
						break;
					default:;
				}
			}

			// if current slot is equipped, get a pointer of form corresponding to item
			if( id != CSheetId() )
			{
				const CStaticItem* itemForm = CSheets::getForm( id );

				if( itemForm )
				{
//					if( quality >= _BestRoleLevel && itemForm->LevelMini <= _ActiveRoleLevel )
					{
/*						uint i;
						
						// Characteristics modifers
						for( i = 0; i < itemForm->CharacteristicsModifier.size(); ++i )
						{
							CHARACTERISTICS::ECharacteristics c = CHARACTERISTICS::toCharacteristic( itemForm->CharacteristicsModifier[ i ] );
							if( c != CHARACTERISTICS::unknown )
							{
								_PhysCharacs._PhysicalCharacteristics[ c ].Modifier ++;
							}
						}
						
						// Regen characteristics modifier
						for( i = 0; i < itemForm->RegenCharacteristicsModifier.size(); ++i )
						{
							CHARACTERISTICS::ECharacteristics c = CHARACTERISTICS::toCharacteristic( itemForm->RegenCharacteristicsModifier[ i ].first );
							if( c != CHARACTERISTICS::unknown )
							{
								_PhysCharacs._PhysicalCharacteristics[ c ].RegenerateModifier = _PhysCharacs._PhysicalCharacteristics[ c ].RegenerateModifier + (sint32) ( _PhysCharacs._PhysicalCharacteristics[ c ].BaseRegenerateRepos * Regen2 * itemForm->RegenCharacteristicsModifier[ i ].second );
							}
						}
						
						// Score Modifier
						for( i = 0; i < itemForm->ScoresModifier.size(); ++i )
						{
							SCORES::EScores s = SCORES::toScore( itemForm->ScoresModifier[ i ].first );
							if( s != SCORES::unknown )
							{
								_PhysScores._PhysicalScores[ s ].Modifier = _PhysScores._PhysicalScores[ s ].Modifier + (sint32) ( _PhysScores._PhysicalScores[ s ].Base * Score2 * itemForm->ScoresModifier[ i ].second );
							}
						}
						
						// Regen ScoreModifier
						for( i = 0; i < itemForm->RegenScoresModifier.size(); ++i )
						{
							SCORES::EScores s = SCORES::toScore( itemForm->RegenScoresModifier[ i ].first );
							if( s != SCORES::unknown )
							{
								_PhysScores._PhysicalScores[ s ].RegenerateModifier = _PhysScores._PhysicalScores[ s ].RegenerateModifier + (sint32) ( _PhysScores._PhysicalScores[ s ].BaseRegenerateRepos * Regen2 * itemForm->RegenScoresModifier[ i ].second );
							}
						}
						
						// Skill modifier
						for( i = 0; i < itemForm->SkillsModifier.size(); ++i )
						{
							SKILLS::ESkills s = SKILLS::toSkill( itemForm->SkillsModifier[ i ].first );
							if( s != SKILLS::unknown )
							{
								_Skills._Skills[ s ].Modifier = _Skills._Skills[ s ].Modifier + itemForm->SkillsModifier[ i ].second;
							}
						}
		*/			}
				}
			}
		}
	}
} // resetEntityModifier //

//---------------------------------------------------
// recompute all Max value
//
//---------------------------------------------------
void CEntityBase::computeMaxValue()
{
	H_AUTO(EntityBaseComputeMaxValue);

	float Score2 = 1.2f;
	float Regen2 = 1.2f;

	if( _Id.getType() == RYZOMID::player )
	{
		const CStaticRaceStats* raceStat = CSheets::getRaceStats( CSheetId(_SheetId) );
		if( raceStat )
		{
			Score2 = raceStat->ProgressionScore2;
			Regen2 = raceStat->ProgressionRegen2;
		}
	}

	int i;
	float maxf;
	sint32 max;
	// Characteristics
	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		if( _PhysCharacs._PhysicalCharacteristics[ i ].Modifier > 1 ) _PhysCharacs._PhysicalCharacteristics[ i ].Modifier = 1;
		_PhysCharacs._PhysicalCharacteristics[ i ].Max = _PhysCharacs._PhysicalCharacteristics[ i ].Modifier + _PhysCharacs._PhysicalCharacteristics[ i ].Base;
		
		maxf = _PhysCharacs._PhysicalCharacteristics[ i ].BaseRegenerateRepos * Regen2 - _PhysCharacs._PhysicalCharacteristics[ i ].BaseRegenerateRepos;
		if( _PhysCharacs._PhysicalCharacteristics[ i ].RegenerateModifier > maxf ) _PhysCharacs._PhysicalCharacteristics[ i ].RegenerateModifier = maxf;
	}
	
	// Scores
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		max = ( (sint32) ( _PhysScores._PhysicalScores[ i ].Base * Score2 ) ) - _PhysScores._PhysicalScores[ i ].Base;
		if( _PhysScores._PhysicalScores[ i ].Modifier > max ) 
			_PhysScores._PhysicalScores[ i ].Modifier = max;

		_PhysScores._PhysicalScores[ i ].Max = _PhysScores._PhysicalScores[ i ].Modifier + _PhysScores._PhysicalScores[ i ].Base;
		
		maxf = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos * Regen2 - _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos;
		if( _PhysScores._PhysicalScores[ i ].RegenerateModifier > maxf ) 
			_PhysScores._PhysicalScores[ i ].RegenerateModifier = maxf;
	}
	
	// Skills
	for( i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		if( _Skills._Skills[ i ].Modifier > 10 ) _Skills._Skills[ i ].Modifier = 10;
		_Skills._Skills[ i ].Current = _Skills._Skills[ i ].Modifier + _Skills._Skills[ i ].Base;
	}
} // computeMaxValue //

//---------------------------------------------------
// apply regenerate and clip currents value
//
//---------------------------------------------------
void CEntityBase::applyRegenAndClipCurrentValue()
{
	H_AUTO(EntityBaseApplyRegenAndClipCurrentValue);

	// First compute all current regen
	int i;
/*	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		_PhysCharacs._PhysicalCharacteristics[ i ].CurrentRegenerate = _PhysCharacs._PhysicalCharacteristics[ i ].RegenerateModifier + _PhysCharacs._PhysicalCharacteristics[ i ].BaseRegenerateRepos;
		if ( _PhysCharacs._PhysicalCharacteristics[ i ].CurrentRegenerate < 0 ) _PhysCharacs._PhysicalCharacteristics[ i ].CurrentRegenerate = 0;
	}
*/
//	for( i = 0; i < SCORES::NUM_SCORES; ++i )
//	{
//		_PhysScores._PhysicalScores[ i ].CurrentRegenerate = _PhysScores._PhysicalScores[ i ].RegenerateModifier + _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos;
//		if ( _PhysScores._PhysicalScores[ i ].CurrentRegenerate < 0 ) _PhysScores._PhysicalScores[ i ].CurrentRegenerate = 0;
//	}
//
	_PhysScores._PhysicalScores[ SCORES::hit_points ].CurrentRegenerate = _PhysScores._PhysicalScores[ SCORES::hit_points ].RegenerateModifier + _PhysScores._PhysicalScores[ SCORES::hit_points ].BaseRegenerateRepos;
	if ( _PhysScores._PhysicalScores[ SCORES::hit_points ].CurrentRegenerate < 0 ) 
		_PhysScores._PhysicalScores[ SCORES::hit_points ].CurrentRegenerate = 0;



	// Compute current Skills
/*	for( i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		_Skills._Skills[ i ].Current = _Skills._Skills[ i ].Modifier + _Skills._Skills[ i ].Base;
		if( _Skills._Skills[ i ].Current < 1 ) _Skills._Skills[ i ].Current = 1;
	}
*/
/*	// regenerate and clip to max value
	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		if( _PhysCharacs._PhysicalCharacteristics[ i ].Current < _PhysCharacs._PhysicalCharacteristics[ i ].Max ) 
		{ 
			_PhysCharacs._PhysicalCharacteristics[ i ].KeepRegenerateDecimal += _PhysCharacs._PhysicalCharacteristics[ i ].CurrentRegenerate * ( CTickEventHandler::getGameCycle() - _PhysCharacs._PhysicalCharacteristics[ i ].RegenerateTickUpdate ) / 10.0f ;
			_PhysCharacs._PhysicalCharacteristics[ i ].Current = _PhysCharacs._PhysicalCharacteristics[ i ].Current + (sint32) _PhysCharacs._PhysicalCharacteristics[ i ].KeepRegenerateDecimal; 
			_PhysCharacs._PhysicalCharacteristics[ i ].KeepRegenerateDecimal -= (sint32) _PhysCharacs._PhysicalCharacteristics[ i ].KeepRegenerateDecimal;
		}
		if( _PhysCharacs._PhysicalCharacteristics[ i ].Current >  _PhysCharacs._PhysicalCharacteristics[ i ].Max )
		{
			_PhysCharacs._PhysicalCharacteristics[ i ].Current = _PhysCharacs._PhysicalCharacteristics[ i ].Max;
		}
		else if( _PhysCharacs._PhysicalCharacteristics[ i ].Current < 0 ) _PhysCharacs._PhysicalCharacteristics[ i ].Current = 0;
		_PhysCharacs._PhysicalCharacteristics[ i ].RegenerateTickUpdate = CTickEventHandler::getGameCycle();
	}
*/

	
//	for( i = 0; i < SCORES::NUM_SCORES; ++i )
//	{
//		if( _PhysScores._PhysicalScores[ i ].Current < _PhysScores._PhysicalScores[ i ].Max ) 
//		{ 
//			_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal += _PhysScores._PhysicalScores[ i ].CurrentRegenerate * ( CTickEventHandler::getGameCycle() - _PhysScores._PhysicalScores[ i ].RegenerateTickUpdate ) / 10.0f;
//			_PhysScores._PhysicalScores[ i ].Current = (sint32) ( _PhysScores._PhysicalScores[ i ].Current + (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal );
//			_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal -= (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal;
//		}
//		if( _PhysScores._PhysicalScores[ i ].Current > _PhysScores._PhysicalScores[ i ].Max )
//		{
//			_PhysScores._PhysicalScores[ i ].Current = _PhysScores._PhysicalScores[ i ].Max;
//		}
//		else if( _PhysScores._PhysicalScores[ i ].Current < 0 ) _PhysScores._PhysicalScores[ i ].Current = 0;
//		_PhysScores._PhysicalScores[ i ].RegenerateTickUpdate = CTickEventHandler::getGameCycle();
//	}
//
	i= SCORES::hit_points;
	if( _PhysScores._PhysicalScores[ i ].Current < _PhysScores._PhysicalScores[ i ].Max ) 
	{ 
		_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal += _PhysScores._PhysicalScores[ i ].CurrentRegenerate * ( CTickEventHandler::getGameCycle() - _PhysScores._PhysicalScores[ i ].RegenerateTickUpdate ) * 0.10f;

		const sint32 sintPart = (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal;
		if (sintPart >= 1)
		{
			_PhysScores._PhysicalScores[ i ].Current = (sint32) ( _PhysScores._PhysicalScores[ i ].Current + sintPart );
			_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal -= sintPart;
			PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->applyRegenHP(_EntityRowId, sintPart);
		}
	}
	if( _PhysScores._PhysicalScores[ i ].Current > _PhysScores._PhysicalScores[ i ].Max )
	{
		_PhysScores._PhysicalScores[ i ].Current = _PhysScores._PhysicalScores[ i ].Max;
	}
	else if( _PhysScores._PhysicalScores[ i ].Current < 0 ) 
		_PhysScores._PhysicalScores[ i ].Current = 0;
	_PhysScores._PhysicalScores[ i ].RegenerateTickUpdate = CTickEventHandler::getGameCycle();

	// Speed
	// while stunned/root/mezzed etc speed is forced to 0
	if ( canEntityMove() )
	{
		float pcSpeed = (1.0f + float(_PhysScores.SpeedVariationModifier) * 0.01f) * _EventSpeedVariationModifier;
		_PhysScores.CurrentWalkSpeed = pcSpeed * _PhysScores.BaseWalkSpeed;
		_PhysScores.CurrentRunSpeed = pcSpeed * _PhysScores.BaseRunSpeed;
	}
	else
	{
		_PhysScores.CurrentWalkSpeed = 0.0f;
		_PhysScores.CurrentRunSpeed = 0.0f;
	}
	
} // applyRegenAndClipCurrentValue //

//---------------------------------------------------
// set who sees me
//---------------------------------------------------
void CEntityBase::setWhoSeesMe( TYPE_WHO_SEES_ME val ) 
{ 
	_WhoSeesMe = val; 
	
	CCharacter* c = dynamic_cast<CCharacter*>(this);
//	CCharacter* c = CharacterManager.getCharacter(CharacterManager.getCharacterId(getId()));
	if (c!=NULL)
		c->setAggroable(R2_VISION::isEntityVisibleToMobs(_WhoSeesMe()));
}


//---------------------------------------------------
// remove all spells on entity
//---------------------------------------------------
void CEntityBase::removeAllSpells()
{
	stopAllLinks(1.0f);
	//destroy all effects
	for ( uint i = 0; i < _SEffects.size();)
	{
		for (uint i = 0; i < _SEffects.size();  )
		{
			if ( _SEffects[i] )
			{
				// stop the effect (remove sabrina effect doesn't stop it)
				CSEffectPtr effect = _SEffects[i];
				if (effect)
					effect->stopEffect();

				// remove the effect (dont increment i because the effect vector is modified), without activating sleeping effects
				removeSabrinaEffect( _SEffects[i], false );
/*
 ALREADY DONE in the effect removed() method, as it's called in removeSabrinaEffect
				CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( effect->getCreatorRowId() );
				if( caster )
				{
					caster->removeLink(effect);
				}
*/
			}
			else
			{
				nlwarning("<CEntityBase tickEffectUpdate effect %u is NULL. Debug",i);
				i++;
			}
		}
	}
}

//---------------------------------------------------
// destructor :
//
//---------------------------------------------------
CEntityBase::~CEntityBase()
{
	_ListLink.unlink();
} // destructor //


//---------------------------------------------------
// set the behaviour
//---------------------------------------------------
void CEntityBase::setBehaviour( MBEHAV::CBehaviour behaviour, bool forceUpdate )
{
	H_AUTO(CEntityBaseSetBehaviour);
	
//	if	(	_Behaviour.getValue().Behaviour < MBEHAV::RESURECTED
//		||	_Behaviour.getValue().Behaviour > MBEHAV::PERMANENT_DEATH)
	if( !_IsDead || forceUpdate == true )
	{
		// cannot change behaviour while stunned (except Death)
		if ( _Stunned )
		{
//			if (behaviour.Behaviour < MBEHAV::RESURECTED || behaviour.Behaviour > MBEHAV::PERMANENT_DEATH)
			if( !_IsDead || forceUpdate == true )
			{
				//nlinfo("<CEntityBase::setBehaviour> %d behaviour %s not setted because entity %s is stunned", CTickEventHandler::getGameCycle(), behaviour.toString().c_str(), _Id.toString().c_str() );
			}

			else
			{
				_Stunned = false;
				_Behaviour = behaviour;
				_ContextualProperty.directAccessForStructMembers().talkableTo( false );
				_ContextualProperty.setChanged();
				//nlinfo("<CEntityBase::setBehaviour> %d change behaviour to %s for dead entity %s, stuned flag is reseted (dead)", CTickEventHandler::getGameCycle(), behaviour.toString().c_str(), _Id.toString().c_str() );
			}
		}
		else
		{
			_Behaviour = behaviour;
			//nlinfo("<CEntityBase::setBehaviour> %d change behaviour to %s for entity %s", CTickEventHandler::getGameCycle(), behaviour.toString().c_str(), _Id.toString().c_str() );
		}
	}
} // setBehaviour //


//---------------------------------------------------
// serial: reading off-mirror, writing from mirror
//---------------------------------------------------
void CEntityBase::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.xmlPush("CEntityBasePart");
	f.xmlPush("CEntityBasePartVersion");
	// serial version
	uint16 version;
	if ( !f.isReading() )
		version = getCurrentVersion();
	f.serial(version);
	f.xmlPop();

	f.xmlPush("PositionsAndDirection");
	if(f.isReading())
	{
		f.serial( _EntityState );
	}
	else
	{
        COfflineEntityState state;
        state.X = _EntityState.X;
        state.Y = _EntityState.Y;
        state.Z = _EntityState.Z;
        state.Heading = _EntityState.Heading;

		sint32 cell = 0;
		CCharacter * c = dynamic_cast<CCharacter*>(this);
		if ( _Id.getType() == RYZOMID::player && c!=NULL)
		{

			if ( c->getEnterFlag() )
			{
				TDataSetRow dsr = c->getEntityRowId();
				if ( dsr.isNull() )
				{
					nlwarning( "ANTIBUG: Character %s EnterFlag but row not set", c->getId().toString().c_str() );
				}
				else
				{
					CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, dsr, DSPropertyCELL );
					cell = mirrorCell;			
					if ( cell <= -2 )
					{
						/// todo pvp
						// the user is inside : save the corresponding position outdoor
						const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( c->getBuildingExitZone() );
						if ( zone )
						{
							zone->getRandomPoint( state.X, state.Y,state.Z,state.Heading );
						}
						else
						{
							nlwarning("user %s is not found in a room but cell is %d)", _Id.toString().c_str(), cell );
						}
					}
				}
			}
		}
	
    
		if(state.X <= 0 || state.Y >= 0)
        {
			// saving a guy that is in a teleportation
            if ( _Id.getType() == RYZOMID::player )
            {
                CCharacter * c = (CCharacter*) this;
                if ( c->getEnterFlag() )
                {
                    CCharacter * c = (CCharacter*) this;
                    if ( c->getEnterFlag() )
                    {
                        state.X = c->getTpCoordinate().X;
                        state.Y = c->getTpCoordinate().Y;
                        state.Z = c->getTpCoordinate().Z;
                        state.Heading = c->getTpCoordinate().Heading;
                    }
                }
            }
		}
        f.serial( state );
	}
	f.xmlPop();

/*
	if( version < 5 )
	{
		f.xmlPush("SheetId");
		_SheetId.serialRTWM( f );
		f.xmlPop();
	}
*/
	if (version <= 5)
	{
		f.xmlPush("NameAndSurname");
		f.serial( _Name );
		ucstring surname;
		f.serial( surname );
		f.xmlPop();
	}
	else
	{
		f.xmlPush("Name");
		f.serial( _Name );
		f.xmlPop();
	}

	// process pending Tp command for this character if exist
	if(f.isReading())
	{
		COfflineEntityState state;
		if( CGmTpPendingCommand::getInstance()->getTpPendingforCharacter( _Name.toString(), state, (CCharacter&)*this ) )
		{
			_EntityState= state;
		}
	}

	f.xmlPush("Race_gender_size");
	if (f.isReading())
	{
		string race;
		f.serial(race);
		_Race = EGSPD::CPeople::fromString(race);

		// check coordinate, if not valid, init position to newbie land start point depending to race
		if( _EntityState.X <= 0 || _EntityState.Y >= 0 )
		{
			// select starting point
			RYZOM_STARTING_POINT::TStartPoint sp;
//			if(UseNewNewbieLandStartingPoint)
			{
				sp= RYZOM_STARTING_POINT::starting_city;
			}
/*			else
			{
				switch( _Race )
				{
					case EGSPD::CPeople::Fyros:
						sp = RYZOM_STARTING_POINT::aegus;
						break;
					case EGSPD::CPeople::Tryker:
						sp = RYZOM_STARTING_POINT::aubermouth;
						break;
					case EGSPD::CPeople::Matis:
						sp = RYZOM_STARTING_POINT::stalli;
						break;
					case EGSPD::CPeople::Zorai:
						sp = RYZOM_STARTING_POINT::qai_lo;
						break;
					default:
						sp = RYZOM_STARTING_POINT::aegus;
				}
			}
*/
			// set the character initial state
			TAIAlias bot,mission;
			const CTpSpawnZone * zone = CZoneManager::getInstance().getStartPoint( (uint16)sp,bot,mission );
			if ( !zone )
			{
				nlwarning( "Invalid start point %d", sp );
			}
			else
			{
				COfflineEntityState state;
				zone->getRandomPoint( state.X,state.Y,state.Z,state.Heading );
				_EntityState= state;
			}
			
		}
		

	}
	else
	{
		string race = EGSPD::CPeople::toString(_Race);
		f.serial(race);
	}

	if( f.isReading() )
	{
		for( map< CSheetId, CStaticRaceStats >::const_iterator it = CSheets::getRaceStatsContainer().begin(); it != CSheets::getRaceStatsContainer().end(); ++it )
		{
			if( (*it).second.Race == _Race )
			{
				_SheetId= it->first.asInt();
				break;
			}
		}
		nlassert( CSheetId(_SheetId()) != CSheetId::Unknown );
	}

	f.serial( _Gender );
	f.serial( _Size );
	f.xmlPop();

	f.xmlPush("Characteristics");
	f.serial( _PhysCharacs );
	f.xmlPop();

	f.xmlPush("Scores");
	f.serial( _PhysScores );
	f.xmlPop();

	f.xmlPush("Skills");
	f.serial( _Skills );
	f.xmlPop();

	f.xmlPush("Items");
	f.serial( _Items );
	f.xmlPop();

	f.xmlPush("ActiveSheath_NBSheath");
	uint8 nbSheath;	f.serial( nbSheath );
	uint8 activeSheath;	f.serial( activeSheath );
	f.xmlPop();

	f.xmlPush("SpecialModifier_GlobalSkillModifier");
	f.serial( _SpecialModifiers );
/*
	if ( version < 4)
	{
		// serial former param _GlobalSkillModifier
		sint32 dummy;
		f.serial(dummy);
	}
*/	f.xmlPop();

	if ( f.isReading() )
	{
//		if (version >= 2)
		{
			f.xmlPush("Defense");
			f.serial( _DodgeAsDefense );
			string slot;
			f.serial( slot );
			_ProtectedSlot = SLOT_EQUIPMENT::stringToSlotEquipment(slot);
			f.xmlPop();
		}
/*		else
		{
			_DodgeAsDefense = true;
			_ProtectedSlot = SLOT_EQUIPMENT::UNDEFINED;
		}
*/	}
	else
	{
		f.xmlPush("Defense");
		f.serial( _DodgeAsDefense );
		string slot = SLOT_EQUIPMENT::toString(_ProtectedSlot);
		f.serial( slot );
		f.xmlPop();
	}

	f.xmlPop();
}

//---------------------------------------------------
// getRegen2
//---------------------------------------------------
float CEntityBase::getRegen2()
{	
	if( _Id.getType() == RYZOMID::player )
	{
		const CStaticRaceStats* raceStat = CSheets::getRaceStats( CSheetId(_SheetId) );
		if( raceStat )
		{
			return raceStat->ProgressionRegen2;
		}
	}
	return 1.2f;
}

//---------------------------------------------------
// getScore2
//---------------------------------------------------
float CEntityBase::getScore2()
{	
	if( _Id.getType() == RYZOMID::player )
	{
		const CStaticRaceStats* raceStat = CSheets::getRaceStats( CSheetId(_SheetId) );
		if( raceStat )
		{
			return raceStat->ProgressionScore2;
		}
	}
	return 1.2f;
}



//---------------------------------------------------
// stun
//---------------------------------------------------
void CEntityBase::stun()
{
	_Stunned = true;
	_Behaviour = MBEHAV::CBehaviour( MBEHAV::STUNNED );

	// cancel entity static action
	//cancelStaticActionInProgress();
	cancelStaticEffects();

	if ( _Id.getType() != RYZOMID::player)
	{
		CAIStunEvent *event = new CAIStunEvent();
		if (!event)
		{
			nlwarning("<CEntityBase::stun> FAILED to allocate new object CAIStunEvent !!!!!!");
			return;
		}
		event->CreatureId = _Id;		
		CPhraseManager::getInstance().addAIEvent(event);
	}
} // stun //



//---------------------------------------------------
// wake
//---------------------------------------------------
void CEntityBase::wake()
{
	if (! _Stunned.getValue() ) return;

	_Stunned = false;

	_Behaviour = MBEHAV::CBehaviour( MBEHAV::STUN_END );

	if ( _Id.getType() != RYZOMID::player)
	{
		CAIStunEndEvent *event = new CAIStunEndEvent();
		if (!event)
		{
			nlwarning("<CEntityBase::wake> FAILED to allocate new object CAIStunEndEvent !!!!!!");
			return;
		}
		event->CreatureId = _Id;		
		CPhraseManager::getInstance().addAIEvent( event );
	}
} // wake //

//-----------------------------------------------
// CEntityBase::addSabrinaEffect
//-----------------------------------------------
bool CEntityBase::addSabrinaEffect( CSEffect *effect )
{
	if( _Invulnerable )
		return false;

	if (!effect)
	{
		nlwarning("<CEntityBase::addSabrinaEffect> tried to add a NULL effect for entity %s", _Id.toString().c_str());
		return false;
	}

	// Special case  :if added effect is invulnerability, clear all effects !
	if (effect->getFamily() == EFFECT_FAMILIES::PowerInvulnerability)
	{
		removeAllSpells();
	}

	bool newActive = false;

	// if effect isn't stackable
	if ( !effect->isStackable() )
	{
		CSEffect *activeEffect = NULL;
		uint index;
		// get the effect of the same type that is active if any
		for (uint i = 0; i < _SActiveEffects.size(); i++ )
		{
			if ( _SActiveEffects[i] )
			{
				if ( _SActiveEffects[i]->getFamily() == effect->getFamily() )
				{
					activeEffect = _SActiveEffects[i];
					index = i;
					break;
				}
			}
			else
				nlwarning("<CEntityBase addSabrinaEffect> NULL active effect #%u found in an entity. Debug needed",i);
		}
		
		if (activeEffect != NULL)
		{
			// if new effect is more powerful than active effect, remove active effect and apply new one
			if ( effect->getPower() > activeEffect->getPower() || effect->automaticallyReplaceFamily() ) 
			{
				if (activeEffect->canBeInactive())
				{
					activeEffect->removed();
					_SActiveEffects[index] = effect;
				}
				else
				{
					removeSabrinaEffect(activeEffect);
					_SActiveEffects.push_back(effect);
					newActive = true;
				}
			}
			else
			{
				if (!effect->canBeInactive())
				{
					// Since new effect can't replace active one and can't be inactive don't add it at all
					return false; //< and don't add it in DB
				}
			}
		}
		else
		{
			_SActiveEffects.push_back(effect);
			newActive = true;
		}
	}
	else
	{
		_SActiveEffects.push_back(effect);
		newActive = true;
	}

	_SEffects.push_back(effect);

	CBasicEffect basicEffect(_SEffects.back()->getFamily(),effect->getCreatorRowId(),_EntityRowId);
	CEffectManager::addEffect(_EntityRowId,basicEffect);
	effect->setEffectId( basicEffect.effectId() );

	if (newActive)
	{
		// if the effect is one of the prevent move type, inc the value
		switch(effect->getFamily()) 
		{	
		case EFFECT_FAMILIES::Mezz:
			++_MezzCount;
			incPreventEntityMove();
			break;
		case EFFECT_FAMILIES::Stun:
		case EFFECT_FAMILIES::CombatStun:
			stun();
			incPreventEntityMove();
			break;
		case EFFECT_FAMILIES::Root:
			incPreventEntityMove();
			break;
		default:;
		}

		// force first update of effect !!warning!! can sometimes kill current entity or clear spells... so last thing to do
		effect->forceUpdate();
	}

	return true;
} // addSabrinaEffect //

//-----------------------------------------------
// CEntityBase::addSabrinaEffect
//-----------------------------------------------
bool CEntityBase::removeSabrinaEffect( CSEffect *effect, bool activateSleepingEffect )
{
	bool removeEffectInDb = true;

	if (!effect)
	{
		nlwarning("<CEntityBase::removeSabrinaEffect> tried to remove a NULL effect for entity %s", _Id.toString().c_str());
		return removeEffectInDb;
	}

	EFFECT_FAMILIES::TEffectFamily effectFamily = effect->getFamily();
	bool effectRemoved = false;
	bool activeEffectRemoved = false;

	// check if removed effect is active
	for (uint i = 0; i < _SActiveEffects.size(); i++ )
	{
		if ( _SActiveEffects[i] )
		{
			if ( _SActiveEffects[i] == effect )
			{
				effect->removed();
				effect->isRemoved(true);
				_SActiveEffects[i] = _SActiveEffects.back();
				_SActiveEffects.pop_back();
				activeEffectRemoved = true;
				break;
			}
		}
		else
			nlwarning("<CEntityBase removeSabrinaEffect> NULL active effect #%u found in an entity. Debug needed",i);
	}
	
	for (uint i = 0; i < _SEffects.size(); i++ )
	{
		if ( _SEffects[i] == effect )
		{
			effect->isRemoved(true);
// removed now called when effect is active
//			effect->removed();
			CEffectManager::removeEffect( _EntityRowId, effect->getEffectId() );
			_SEffects[i] = _SEffects.back();
			_SEffects.pop_back();
			effectRemoved = true;
			break;
		}
	}

	// effect is not longer valid from here
	effect = 0;

	if (activeEffectRemoved)
	{
		if (activateSleepingEffect)
		{
			// find the most powerful effect of the same family as the removed one and activate it,
			// unless the same effect family already exists in active effects
			const CSEffectPtr &activeEffect = lookForActiveEffect(effectFamily);
			if (activeEffect == NULL)
			{
				const CSEffectPtr &effect = lookForSEffect(effectFamily);
				if (effect != NULL)
				{
					_SActiveEffects.push_back(effect);
					// warning this method may kill current entity
					effect->forceUpdate();
					removeEffectInDb = false;
				}
			}
			else
			{
				removeEffectInDb = false;
			}
		}

		// if no more similar effect process special trigger from effect family
		if (removeEffectInDb)
		{
			switch(effectFamily) 
			{
			case EFFECT_FAMILIES::Mezz:
				if ( _MezzCount > 0 ) // TEMP workaround
					--_MezzCount;
				decPreventEntityMove();
				break;

			case EFFECT_FAMILIES::Stun:
			case EFFECT_FAMILIES::CombatStun:
				if (removeEffectInDb)
					wake();
				decPreventEntityMove();
				break;

			case EFFECT_FAMILIES::Root:
				decPreventEntityMove();
				break;

			default:
				break;
			}
		}
	}
	else
	{
		removeEffectInDb = false;
	}

	if(!effectRemoved)
		nlwarning( "<CEntityBase removeSabrinaEffect> effect not found on entity %s",_Id.toString().c_str() );

	return removeEffectInDb;
} // removeSabrinaEffect //


//-----------------------------------------------
// CEntityBase::addLink
//-----------------------------------------------
void CEntityBase::addLink( CSLinkEffect *effect )
{
	if (!effect || !TheDataset.isAccessible(effect->getCreatorRowId()) )
		return;

	_SEffectLinks.push_back(effect);

	// set the visual FX to link type
	CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
	if (visualFx.isInitialized())
	{
		CVisualFX fx;
		fx.unpack(visualFx.getValue());
		fx.Link = 1 + effect->getMagicFxType();
		sint64 prop;
		fx.pack(prop);
		visualFx = (sint16)prop;
	}

	// send message to chat
	CEntityId targetId = CEntityBaseManager::getEntityId(effect->getTargetRowId());

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
	if ( targetId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
		PHRASE_UTILITIES::sendDynamicSystemMessage( TheDataset.getDataSetRow(targetId), "MAGIC_NEW_LINK_TARGET", params);
	}
	if ( _Id.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "MAGIC_NEW_LINK_CASTER", params);
	}
} // addLink //

//-----------------------------------------------
// CEntityBase::removeLink
//-----------------------------------------------
void CEntityBase::removeLink( CSLinkEffect *effect, float factorOnSurvivalTime )
{
	if (!effect)
	{
		nlwarning("<CEntityBase::removeLink> tried to remove a NULL link effect for entity %s", _Id.toString().c_str());
		return;
	}

	for (uint i = 0; i < _SEffectLinks.size(); i++ )
	{
		if ( _SEffectLinks[i] == effect )
		{
			CSLinkEffectPtr effectPtr = effect; // keep a ref

			_SEffectLinks[i] = _SEffectLinks.back();
			_SEffectLinks.pop_back();
			CEntityId targetId = CEntityBaseManager::getEntityId(effect->getTargetRowId());

			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
			if ( targetId.getType() == RYZOMID::player )
			{
				params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
				PHRASE_UTILITIES::sendDynamicSystemMessage( effect->getTargetRowId(), "MAGIC_DESTROY_LINK_TARGET", params);
			}
			if ( _Id.getType() == RYZOMID::player )
			{
				params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
				PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "MAGIC_DESTROY_LINK_CASTER", params);
			}

			// break the link, it will last a few seconds depending on its type (see _NoLinkSurvivalTime)
			effect->breakLink(factorOnSurvivalTime);

			// change visual property
			if (TheDataset.isAccessible(_EntityRowId))
			{
				CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
				if (visualFx.isInitialized())
				{
					CVisualFX fx;
					fx.unpack(visualFx.getValue());
					fx.Link = 0;
					sint64 prop;
					fx.pack(prop);
					visualFx = (sint16)prop;
				}
			}

			return;
		}
	}
//	nlwarning( "<CEntityBase removeLink> effect not found on entity %s",_Id.toString().c_str() );
} // removeLink //

//-----------------------------------------------
// CEntityBase::stopAllLinks
//-----------------------------------------------
void CEntityBase::stopAllLinks(float factorOnSurvivalTime)
{
	CPhraseManager::getInstance().breakLaunchingLinks(this);

	if (_SEffectLinks.empty())
		return;

	// change visual property
	if (TheDataset.isAccessible(_EntityRowId))
	{
		CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
		if (visualFx.isInitialized())
		{
			CVisualFX fx;
			fx.unpack(visualFx.getValue());
			fx.Link = 0;
			sint64 prop;
			fx.pack(prop);
			visualFx = (sint16)prop;
		}
	}

	// stop links, but do not delete them
	for (uint i = 0; i < _SEffectLinks.size(); ++i)
	{
		if ( _SEffectLinks[i] )
		{
			CSLinkEffect * effect = _SEffectLinks[i];			

			const CEntityId targetId = CEntityBaseManager::getEntityId(effect->getTargetRowId());

			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
			if ( targetId.getType() == RYZOMID::player )
			{
				params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
				PHRASE_UTILITIES::sendDynamicSystemMessage( effect->getTargetRowId(), "MAGIC_DESTROY_LINK_TARGET", params);
			}
			if ( _Id.getType() == RYZOMID::player )
			{
				params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
				PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "MAGIC_DESTROY_LINK_CASTER", params);
			}

			// break the link, it will last a few seconds depending on its type (see _NoLinkSurvivalTime)
			effect->breakLink(factorOnSurvivalTime);
		}
		else
		{
			nlwarning( "<CEntityBase stopAllLinks> link %u is NULL on entity %s",i,_Id.toString().c_str() );
		}
	}
	_SEffectLinks.clear();
} // stopAllLinks //

//-----------------------------------------------
// CEntityBase::lookForActiveEffect
//-----------------------------------------------
const CSEffectPtr &CEntityBase::lookForActiveEffect( EFFECT_FAMILIES::TEffectFamily effectType )
{
	static CSEffectPtr nullRet;
	
	for (uint i = 0; i < _SActiveEffects.size(); i++ )
	{
		if ( _SActiveEffects[i] )
		{
			if ( _SActiveEffects[i]->getFamily() == effectType )
			{
				return _SActiveEffects[i];
			}
		}
		else
			nlwarning("<CEntityBase lookForActiveEffect> NULL effect #%u found in an entity. Debug needed",i);
	}

	return nullRet;
}// CEntityBase::lookForActiveEffect

//-----------------------------------------------
// CEntityBase::lookForSEffect
//-----------------------------------------------
const CSEffectPtr &CEntityBase::lookForSEffect( EFFECT_FAMILIES::TEffectFamily effectType, bool getHigherValue )
{
	static CSEffectPtr nullRet;
	sint32 ret = -1;
	for (uint i = 0; i < _SEffects.size(); i++ )
	{
		if ( _SEffects[i] )
		{
			if ( _SEffects[i]->getFamily() == effectType )
			{
				if ( getHigherValue )
				{
					if (ret == -1 || abs( _SEffects[ret]->getParamValue() ) <  abs( _SEffects[i]->getParamValue() ) )
						ret = i;
				}
				else
					return _SEffects[i];
			}
		}
		else
			nlwarning("<CEntityBase lookForSEffect> NULL effect #%u found in an entity. Debug needed",i);
	}
	
	if (ret >= 0 )
		return _SEffects[ret];
	else
		return nullRet;
}// CEntityBase::lookForSEffect


//-----------------------------------------------
// CEntityBase::addAgressorXp: memorize xp gain per agressor for offensive action
//-----------------------------------------------
/*void CEntityBase::addAgressorXp( const CEntityId& agressor, double xp, const std::string& Skill )
{
	SKILLS::ESkills skill;
	skill = SKILLS::toSkill( Skill );
	if( skill == SKILLS::unknown )
	{
		nlwarning("<CEntityBase::addAgressorXp> Skill %s is unknown", Skill.c_str() );
		return;
	}

	TDelayedXpGainContainer::iterator it = _AgressorXp.find( agressor );
	if( it == _AgressorXp.end() )
	{
		TDelayedXpGain gain;
		gain.skill = skill;
		gain.xp = xp;
		vector< TDelayedXpGain > tmp;
		tmp.push_back( gain );
		_AgressorXp.insert( make_pair( agressor, tmp ) );
	}
	else
	{
		const uint size = (*it).second.size();
		for( uint i = 0; i < size; ++ i )
		{
			if( (*it).second [ i ].skill == skill )
			{
				(*it).second [ i ].xp = (*it).second [ i ].xp + xp;
				return;
			}
		}
		TDelayedXpGain gain;
		gain.skill = skill;
		gain.xp = xp;
		(*it).second.push_back( gain );
	}
}
*/

//-----------------------------------------------
// CEntityBase::giveAgressorXp: give to agressor memorized xp gain
//-----------------------------------------------
/*void CEntityBase::giveAgressorXp()
{
	for( TDelayedXpGainContainer::iterator it = _AgressorXp.begin(); it != _AgressorXp.end(); ++it )
	{
		const uint size = (*it).second.size();
		for( uint i = 0; i < size; ++i )
		{
			CCharacter * player = PlayerManager.getChar( (*it).first );
			if( player )
			{
				player->addXpToSkill( (*it).second[ i ].xp, SKILLS::toString( (*it).second[ i ].skill ) );
			}
		}
	}
	_AgressorXp.clear();
}
*/

//-----------------------------------------------
// CEntityBase::canEntityUseAction
//-----------------------------------------------
bool CEntityBase::canEntityUseAction(CBypassCheckFlags bypassCheckFlags, bool sendMessage) const
{
	static EFFECT_FAMILIES::TEffectFamily ForbidActionsEffects[2]=
	{
		EFFECT_FAMILIES::Fear,
		EFFECT_FAMILIES::PowerInvulnerability,
	};

	if ( _IsDead )
		return false;

	if (_Stunned.getValue() && !bypassCheckFlags.Flags.Stun)
		return false;
	
	if (_MezzCount > 0 && !bypassCheckFlags.Flags.Sleep)
		return false;

	const uint8 size = sizeof(ForbidActionsEffects) / sizeof(EFFECT_FAMILIES::TEffectFamily);
	// look for forbidden effects
	for (uint i = 0; i < _SEffects.size(); i++ )
	{
		if ( _SEffects[i] )
		{
			for ( uint j = 0 ;j < size ; ++j )
			{
				// check this family need to be checked
				switch(ForbidActionsEffects[j] ) 
				{
				case EFFECT_FAMILIES::Fear:
					if (bypassCheckFlags.Flags.Fear)
						continue;
				case EFFECT_FAMILIES::PowerInvulnerability:
					if (bypassCheckFlags.Flags.Invulnerability)
						continue;
				default:;
				}

				if(ForbidActionsEffects[j] == _SEffects[i]->getFamily())
					return false;
			}
		}
		else
			nlwarning("<CEntityBase canEntityUseAction> NULL effect #%u found in an entity. Debug needed",i);
	}

	return true;
} // canEntityUseAction //

//-----------------------------------------------
// CEntityBase::canEntityDefend
//-----------------------------------------------
bool CEntityBase::canEntityDefend()
{
	static EFFECT_FAMILIES::TEffectFamily ForbidDefenseEffects[2]=
	{
		EFFECT_FAMILIES::Fear,
		EFFECT_FAMILIES::Blind,
	};

	if (_Stunned.getValue() || _IsDead || _MezzCount > 0)
		return false;

	if (_Mode().Mode == MBEHAV::SIT)
		return false;

	const uint8 size = sizeof(ForbidDefenseEffects) / sizeof(EFFECT_FAMILIES::TEffectFamily);
	const uint8 nbEffects = (uint8)_SEffects.size();

	// look for forbidden effects
	for (uint i = 0; i < nbEffects; ++i )
	{
		if ( _SEffects[i] )
		{
			for ( uint j = 0 ;j < size ; ++j )
			{
				if(ForbidDefenseEffects[j] == _SEffects[i]->getFamily())
					return false;
			}
		}
		else
			nlwarning("<CEntityBase::canEntityDefend> NULL effect #%u found in an entity. Debug needed",i);
	}
	
	return true;

} // canEntityDefend //

//-----------------------------------------------
// CEntityBase::unmezz
//-----------------------------------------------
void CEntityBase::unmezz()
{
	for (uint i = 0; i < _SEffects.size();  )
	{
		if ( _SEffects[i] )
		{
			if ( _SEffects[i]->getFamily() == EFFECT_FAMILIES::Mezz )
			{
				removeSabrinaEffect( _SEffects[i]);
				continue;
			}
			++i;
		}
		else
		{
			nlwarning("<CEntityBase unmezz effect %u is NULL. Debug",i);
			++i;
		}
	}
} // unmezz //

/// change the outpost id
void CEntityBase::setOutpostAlias( uint32 id )
{
	_OutpostAlias = id;
	
	updateOutpostAliasClientVP();
}

/// get the outpost alias
uint32 CEntityBase::getOutpostAlias() const
{
	if( _OutpostAlias.isReadable() )
		return _OutpostAlias;
	else
		return 0;
}

/// change the outpost side
void CEntityBase::setOutpostSide( OUTPOSTENUMS::TPVPSide side )
{
	_OutpostSide = side;

	updateOutpostSideClientVP();
}

/// get the outpost side
OUTPOSTENUMS::TPVPSide CEntityBase::getOutpostSide() const
{
	if( _OutpostSide.isReadable() )
		return (OUTPOSTENUMS::TPVPSide)_OutpostSide.getValue();
	else
		return OUTPOSTENUMS::UnknownPVPSide;
}

///----------------------------------------------------------------------------
void CEntityBase::updateOutpostAliasClientVP()
{
	uint32 id = getOutpostAlias();
	
	if( id == 0 )
	{
		// special value : reset all
		_OutpostSide = OUTPOSTENUMS::UnknownPVPSide;
		_OutpostInfos = 0;
	}
	else
	{
		uint16 outpostVPId = COutpostManager::getInstance().getOutpostShortId( id );
		uint16 outpostVPSide = (getOutpostSide() == OUTPOSTENUMS::OutpostAttacker )?1:0;
		uint16 vpValue = (outpostVPSide<<15) | outpostVPId;
		_OutpostInfos = vpValue;
	}
}

///----------------------------------------------------------------------------
void CEntityBase::updateOutpostSideClientVP()
{
	OUTPOSTENUMS::TPVPSide side = (OUTPOSTENUMS::TPVPSide) getOutpostSide();

	if( side != OUTPOSTENUMS::UnknownPVPSide )
	{
		uint16 outpostVPId = COutpostManager::getInstance().getOutpostShortId( getOutpostAlias() );
		uint16 vpValue = ((uint16)side<<15) | outpostVPId;
		_OutpostInfos = vpValue;
	}
}


//-----------------------------------------------------------------------------
bool CEntityBase::isEntityAnOutpostEnemy( const CEntityId& id )
{
	if( getOutpostAlias() != 0 )
	{
		CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( id );
		if (entity)
		{
			if( entity->getOutpostAlias() == getOutpostAlias() )
			{
				if( entity->getOutpostSide() != getOutpostSide() )
				{
					return true;
				}
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CEntityBase::isSpire() const
{
	NLMISC::CSString sheetIdName = ((NLMISC::CSheetId)_SheetId).toString();
	return (sheetIdName.left(6)=="spire_") || (sheetIdName.left(13)=="object_spire_");
}

