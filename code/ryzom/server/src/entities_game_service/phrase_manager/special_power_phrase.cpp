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
// game share
#include "game_share/brick_families.h"
#include "game_share/string_manager_sender.h"
//
#include "special_power_phrase.h"
#include "s_phrase_factory.h"
#include "phrase_manager/phrase_utilities_functions.h"
//
#include "special_power_basic.h"
#include "special_power_taunt.h"
#include "special_power_shielding.h"
#include "special_power_speeding_up.h"
#include "special_power_basic_aura.h"
#include "special_power_dot.h"
#include "special_power_enchant_weapon.h"
#include "special_power_balance.h"
#include "special_power_heal.h"
#include "special_power_chg_charac.h"
#include "special_power_mod_defense.h"
#include "special_power_mod_craft_success.h"
#include "special_power_mod_melee_success.h"
#include "special_power_mod_range_success.h"
#include "special_power_mod_magic_success.h"
#include "special_power_mod_forage_success.h"
#include "special_power_mod_magic_protection.h"

#include "entity_manager/entity_manager.h"
#include "game_item_manager/game_item.h"
#include "egs_sheets/egs_static_game_item.h"

DEFAULT_SPHRASE_FACTORY( CSpecialPowerPhrase, BRICK_TYPE::SPECIAL_POWER );

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//-----------------------------------------------
// ctor
//-----------------------------------------------
CSpecialPowerPhrase::CSpecialPowerPhrase()
{
	_IsStatic = false;
	_AddRecastTime = 0;
	_PhraseType = BRICK_TYPE::SPECIAL_POWER;
	_ConsumableFamilyId = (uint16)~0;
}

//-----------------------------------------------
// dtor
//-----------------------------------------------
CSpecialPowerPhrase::~CSpecialPowerPhrase()
{
	// clear special powers
	for (uint i = 0 ; i <_Powers.size() ; ++i)
	{
		if (_Powers[i] != NULL)
			delete _Powers[i];
	}
}

//-----------------------------------------------
// CSpecialPowerPhrase build
//-----------------------------------------------
bool CSpecialPowerPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{

	_ActorRowId = actorRowId;

	for (uint i = 0 ; i < bricks.size() ; ++i)
	{
		processParams(bricks[i]->Params, false, 0);
	}

	return true;
}// CSpecialPowerPhrase build

//-----------------------------------------------
// CSpecialPowerPhrase buildFromConsumable
//-----------------------------------------------
bool CSpecialPowerPhrase::buildFromConsumable(const TDataSetRow & actorRowId, const CStaticItem *itemForm, uint16 quality)
{
	_ActorRowId = actorRowId;

	nlassert(itemForm);
	nlassert(itemForm->ConsumableItem);
	
	// valid all checks of phrase, checks are done on item
	_BypassCheckFlags.setFlag(CHECK_FLAG_TYPE::InWater, true);
	_BypassCheckFlags.setFlag(CHECK_FLAG_TYPE::OnMount, true);
	_BypassCheckFlags.setFlag(CHECK_FLAG_TYPE::WhileSitting, true);
	
	processParams(itemForm->ConsumableItem->Params, true, quality);
	
	_ConsumableFamilyId = itemForm->ConsumableItem->Family;

	return true;
}// CSpecialPowerPhrase buildFromConsumable

//-----------------------------------------------
// CSpecialPowerPhrase processParams
//-----------------------------------------------
void CSpecialPowerPhrase::processParams(const vector<TBrickParam::IIdPtr> &params, bool isConsumable, uint16 quality)
{
	// process params
	for ( uint j = 0 ; j < params.size() ; ++j)
	{
		const TBrickParam::IId	*param = params[j];
		if (!param) continue;

		switch(param->id())
		{
		case TBrickParam::SP_RECAST_TIME:
			{
				// $*STRUCT CSBrickParamRecastTime: public TBrickParam::CId <TBrickParam::SP_RECAST_TIME>
				// $*-f float		Time			// disable power for x seconds
				// get time in ticks
				_AddRecastTime += (NLMISC::TGameCycle)( ((CSBrickParamRecastTime *)param)->Time / CTickEventHandler::getGameTimeStep() );
				break;
			}


		case TBrickParam::SP_TAUNT:
			{
			// $*STRUCT CSBrickParamPowerTaunt: public TBrickParam::CId <TBrickParam::SP_TAUNT>
			// $*-i uint16 TauntPower	// entities of higher level cannot be taunt
			// $*-f float Range			// effective range in meters
			// $*-f float DisableTime	// disable taunt powers for x seconds
			CSpecialPowerTaunt *power = new CSpecialPowerTaunt(_ActorRowId, this, 
																((CSBrickParamPowerTaunt *)param)->TauntPower, 
																((CSBrickParamPowerTaunt *)param)->Range,
																((CSBrickParamPowerTaunt *)param)->DisableTime
																);
			if (power)
				_Powers.push_back(power);

			break;
			}

		case TBrickParam::SP_SHIELDING:
			{
			// $*STRUCT CSBrickParamShielding: public TBrickParam::CId <TBrickParam::SP_SHIELDING>
			// $*-i uint8	NoShieldProtectionFactor	// granted protection in % without a shield
			// $*-i uint16	NoShieldProtectionMax	// max protection without a shield
			// $*-i uint8	BucklerProtectionFactor	// granted protection in % with a buckler
			// $*-i uint16	BucklerProtectionMax		// max protection with a buckler
			// $*-i uint8	ShieldProtectionFactor	// granted protection in % with a shield
			// $*-i uint16	ShieldProtectionMax		// max protection with a shield
			// $*-f float	Duration					// power duration
			// $*-f float	DisableTime				// disable power for x seconds	
			CSpecialPowerShielding *power = new CSpecialPowerShielding(_ActorRowId, this, ((CSBrickParamShielding *)param)->Duration, ((CSBrickParamShielding *)param)->DisableTime);
			if (power)
			{
				power->setNoShieldProtection( ((CSBrickParamShielding *)param)->NoShieldProtectionFactor / 100.0f, ((CSBrickParamShielding *)param)->NoShieldProtectionMax);
				power->setBucklerProtection( ((CSBrickParamShielding *)param)->BucklerProtectionFactor / 100.0f, ((CSBrickParamShielding *)param)->BucklerProtectionMax);
				power->setShieldProtection( ((CSBrickParamShielding *)param)->ShieldProtectionFactor / 100.0f, ((CSBrickParamShielding *)param)->ShieldProtectionMax);
				power->setByPass(isConsumable);
				_Powers.push_back(power);
			}
			break;
			}

		case TBrickParam::SP_LIFE_AURA:
			{
			// $*STRUCT CSBrickParamLifeAura: public TBrickParam::CId <TBrickParam::SP_LIFE_AURA>
			// $*-i uint16	RegenMod			// regen modifier
			// $*-f float	Duration			// duration in seconds
			// $*-f float	Radius				// aura radius in meters
			// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
			// $*-f float	UserDisableTime		// disable life aura for x seconds on user
				CSpecialPowerBasicAura *lifeAura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamLifeAura *)param)->Duration, ((CSBrickParamLifeAura *)param)->UserDisableTime, ((CSBrickParamLifeAura *)param)->TargetDisableTime, POWERS::LifeAura);
				if (lifeAura)
				{
					lifeAura->setRadius(((CSBrickParamLifeAura *)param)->Radius);
					lifeAura->setFamilies(EFFECT_FAMILIES::PowerRootLifeAura,EFFECT_FAMILIES::PowerLifeAura );
					lifeAura->setParamValue(((CSBrickParamLifeAura *)param)->RegenMod);
					lifeAura->setByPass(isConsumable);
					_Powers.push_back(lifeAura);
				}
			}
			break;

		case TBrickParam::SP_LIFE_AURA2:
			{
			// $*STRUCT CSBrickParamLifeAura: public TBrickParam::CId <TBrickParam::SP_LIFE_AURA2>
			// $*-i uint16	RegenMod			// regen modifier proportionally to item level
			// $*-f float	Duration			// duration in seconds
			// $*-f float	Radius				// aura radius in meters
			// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
			// $*-f float	UserDisableTime		// disable life aura for x seconds on user
				CSpecialPowerBasicAura *lifeAura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamLifeAura *)param)->Duration, ((CSBrickParamLifeAura *)param)->UserDisableTime, ((CSBrickParamLifeAura *)param)->TargetDisableTime, POWERS::LifeAura);
				if (lifeAura)
				{
					lifeAura->setRadius(((CSBrickParamLifeAura *)param)->Radius);
					lifeAura->setFamilies(EFFECT_FAMILIES::PowerRootLifeAura,EFFECT_FAMILIES::PowerLifeAura );
					lifeAura->setParamValue(((CSBrickParamLifeAura *)param)->RegenMod*quality);
					lifeAura->setByPass(isConsumable);
					_Powers.push_back(lifeAura);
				}
			}
			break;

		case TBrickParam::SP_STAMINA_AURA:
			{
				CSpecialPowerBasicAura *staminaAura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamStaminaAura*)param)->Duration, ((CSBrickParamLifeAura *)param)->UserDisableTime, ((CSBrickParamStaminaAura *)param)->TargetDisableTime, POWERS::StaminaAura);
				if (staminaAura)
				{
					staminaAura->setRadius(((CSBrickParamStaminaAura *)param)->Radius);
					staminaAura->setFamilies(EFFECT_FAMILIES::PowerRootStaminaAura,EFFECT_FAMILIES::PowerStaminaAura );
					staminaAura->setParamValue(((CSBrickParamStaminaAura *)param)->RegenMod);
					staminaAura->setByPass(isConsumable);
					_Powers.push_back(staminaAura);
				}
			}
			break;

		case TBrickParam::SP_STAMINA_AURA2:
			{
				CSpecialPowerBasicAura *staminaAura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamStaminaAura*)param)->Duration, ((CSBrickParamLifeAura *)param)->UserDisableTime, ((CSBrickParamStaminaAura *)param)->TargetDisableTime, POWERS::StaminaAura);
				if (staminaAura)
				{
					staminaAura->setRadius(((CSBrickParamStaminaAura *)param)->Radius);
					staminaAura->setFamilies(EFFECT_FAMILIES::PowerRootStaminaAura,EFFECT_FAMILIES::PowerStaminaAura );
					staminaAura->setParamValue(((CSBrickParamStaminaAura *)param)->RegenMod*quality);
					staminaAura->setByPass(isConsumable);
					_Powers.push_back(staminaAura);
				}
			}
			break;

		case TBrickParam::SP_SAP_AURA:
			{
				CSpecialPowerBasicAura *sapAura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamSapAura*)param)->Duration, ((CSBrickParamLifeAura *)param)->UserDisableTime, ((CSBrickParamSapAura *)param)->TargetDisableTime, POWERS::SapAura);
				if (sapAura)
				{
					sapAura->setRadius(((CSBrickParamSapAura *)param)->Radius);
					sapAura->setFamilies(EFFECT_FAMILIES::PowerRootSapAura,EFFECT_FAMILIES::PowerSapAura );
					sapAura->setParamValue(((CSBrickParamSapAura *)param)->RegenMod);
					sapAura->setByPass(isConsumable);
					_Powers.push_back(sapAura);
				}
			}
			break;

		case TBrickParam::SP_SAP_AURA2:
			{
				CSpecialPowerBasicAura *sapAura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamSapAura*)param)->Duration, ((CSBrickParamLifeAura *)param)->UserDisableTime, ((CSBrickParamSapAura *)param)->TargetDisableTime, POWERS::SapAura);
				if (sapAura)
				{
					sapAura->setRadius(((CSBrickParamSapAura *)param)->Radius);
					sapAura->setFamilies(EFFECT_FAMILIES::PowerRootSapAura,EFFECT_FAMILIES::PowerSapAura );
					sapAura->setParamValue(((CSBrickParamSapAura *)param)->RegenMod*quality);
					sapAura->setByPass(isConsumable);
					_Powers.push_back(sapAura);
				}
			}
			break;

		case TBrickParam::SP_SPEEDING_UP:
			{
				// $*STRUCT CSBrickParamSpeedingUp: public TBrickParam::CId <TBrickParam::SP_SPEEDING_UP>
				// $*-i uint16	SpeedMod			// speed modifier (in %)
				// $*-f float	Duration			// duration in seconds
				// $*-f float	DisableTime			// disable power for x seconds
				CSpecialPowerSpeedingUp *power = new CSpecialPowerSpeedingUp(_ActorRowId, this, (uint8)((CSBrickParamSpeedingUp *)param)->SpeedMod, ((CSBrickParamSpeedingUp *)param)->Duration, ((CSBrickParamSpeedingUp *)param)->DisableTime);
				if (power)
				{
					power->setByPass(isConsumable);
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_INVULNERABILITY:
			{
				// $*STRUCT CSBrickParamInvulnerability: public TBrickParam::CId <TBrickParam::SP_INVULNERABILITY>
				// $*-f float	Duration			// duration in seconds
				// $*-f float	DisableTime			// disable power for x seconds	
				CSpecialPowerBasic *power = new CSpecialPowerBasic(_ActorRowId, this, ((CSBrickParamInvulnerability *)param)->Duration, ((CSBrickParamInvulnerability *)param)->DisableTime, POWERS::Invulnerability);
				if (power)
				{
					power->setEffectFamily( EFFECT_FAMILIES::PowerInvulnerability );
					power->setByPass(isConsumable);
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MELEE_PROTECTION_AURA:
			{
				// $*STRUCT CSBrickParamMeleeProtection: public TBrickParam::CId <TBrickParam::SP_MELEE_PROTECTION_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamMeleeProtection*)param)->Duration, ((CSBrickParamMeleeProtection *)param)->UserDisableTime, ((CSBrickParamMeleeProtection *)param)->TargetDisableTime, POWERS::MeleeProtection);
				if (aura)
				{
					aura->setRadius(((CSBrickParamMeleeProtection *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootProtection,EFFECT_FAMILIES::PowerProtection );
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
			}
			break;

		case TBrickParam::SP_RANGE_PROTECTION_AURA:
			{
				// $*STRUCT CSBrickParamRangeProtection: public TBrickParam::CId <TBrickParam::SP_RANGE_PROTECTION_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamRangeProtection*)param)->Duration, ((CSBrickParamRangeProtection *)param)->UserDisableTime, ((CSBrickParamRangeProtection *)param)->TargetDisableTime, POWERS::Umbrella);
				if (aura)
				{
					aura->setRadius(((CSBrickParamRangeProtection *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootUmbrella,EFFECT_FAMILIES::PowerUmbrella);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
			}
			break;

		case TBrickParam::SP_MAGIC_PROTECTION_AURA:
			{
				// $*STRUCT CSBrickParamMagicProtection: public TBrickParam::CId <TBrickParam::SP_MAGIC_PROTECTION_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamMagicProtection*)param)->Duration, ((CSBrickParamMagicProtection *)param)->UserDisableTime, ((CSBrickParamMagicProtection *)param)->TargetDisableTime, POWERS::AntiMagicShield);
				if (aura)
				{
					aura->setRadius(((CSBrickParamMagicProtection *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootAntiMagicShield,EFFECT_FAMILIES::PowerAntiMagicShield);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
			}
			break;

		case TBrickParam::SP_WAR_CRY_AURA:
			{
				// $*STRUCT CSBrickParamWarCry: public TBrickParam::CId <TBrickParam::SP_WAR_CRY_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				// $*-f sint16	DamageBonus			// damage bonus (20 = +20%)
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamWarCry*)param)->Duration, ((CSBrickParamWarCry *)param)->UserDisableTime, ((CSBrickParamWarCry *)param)->TargetDisableTime, POWERS::WarCry);
				if (aura)
				{
					aura->setParamValue(((CSBrickParamWarCry *)param)->DamageBonus);
					aura->setRadius(((CSBrickParamWarCry *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootWarCry,EFFECT_FAMILIES::PowerWarCry);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
				break;
			}

		case TBrickParam::SP_FIRE_WALL_AURA:
			{
				// $*STRUCT CSBrickParamFireWall: public TBrickParam::CId <TBrickParam::SP_FIRE_WALL_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				// $*-f sint16	Damage				// damage
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamFireWall*)param)->Duration, ((CSBrickParamFireWall *)param)->UserDisableTime, ((CSBrickParamFireWall *)param)->TargetDisableTime, POWERS::FireWall);
				if (aura)
				{
					aura->setParamValue(((CSBrickParamFireWall *)param)->Damage);
					aura->setRadius(((CSBrickParamFireWall *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootFireWall,EFFECT_FAMILIES::PowerFireWall);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
				break;
			}

		case TBrickParam::SP_THORN_WALL_AURA:
			{
				// $*STRUCT CSBrickParamThornWall: public TBrickParam::CId <TBrickParam::SP_THORN_WALL_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				// $*-f sint16	Damage				// damage
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamThornWall*)param)->Duration, ((CSBrickParamThornWall *)param)->UserDisableTime, ((CSBrickParamThornWall *)param)->TargetDisableTime, POWERS::ThornWall);
				if (aura)
				{
					aura->setParamValue(((CSBrickParamThornWall *)param)->Damage);
					aura->setRadius(((CSBrickParamThornWall *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootThornWall,EFFECT_FAMILIES::PowerThornWall);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
				break;
			}

		case TBrickParam::SP_WATER_WALL_AURA:
			{
				// $*STRUCT CSBrickParamWaterWall: public TBrickParam::CId <TBrickParam::SP_WATER_WALL_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				// $*-f sint16	Damage				// damage
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamWaterWall*)param)->Duration, ((CSBrickParamWaterWall *)param)->UserDisableTime, ((CSBrickParamWaterWall *)param)->TargetDisableTime, POWERS::WaterWall);
				if (aura)
				{
					aura->setParamValue(((CSBrickParamWaterWall *)param)->Damage);
					aura->setRadius(((CSBrickParamWaterWall *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootWaterWall,EFFECT_FAMILIES::PowerWaterWall);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
				break;
			}

		case TBrickParam::SP_LIGHTNING_WALL_AURA:
			{
				// $*STRUCT CSBrickParamLightningWall: public TBrickParam::CId <TBrickParam::SP_LIGHTNING_WALL_AURA>
				// $*-f float	Radius				// aura radius in meters
				// $*-f float	Duration			// duration in seconds
				// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
				// $*-f float	UserDisableTime		// disable aura for x seconds on user
				// $*-f sint16	Damage				// damage
				CSpecialPowerBasicAura *aura = new CSpecialPowerBasicAura(_ActorRowId, this, ((CSBrickParamLightningWall*)param)->Duration, ((CSBrickParamLightningWall *)param)->UserDisableTime, ((CSBrickParamLightningWall *)param)->TargetDisableTime, POWERS::LightningWall);
				if (aura)
				{
					aura->setParamValue(((CSBrickParamLightningWall *)param)->Damage);
					aura->setRadius(((CSBrickParamLightningWall *)param)->Radius);
					aura->setFamilies(EFFECT_FAMILIES::PowerRootLightningWall,EFFECT_FAMILIES::PowerLightningWall);
					aura->setByPass(isConsumable);
					_Powers.push_back(aura);
				}
				break;
			}

		case TBrickParam::SP_BERSERK:
			{
				// $*STRUCT CSBrickParamBerserk: public TBrickParam::CId <TBrickParam::SP_BERSERK>
				// $*-f float	DisableTime			// disable berserker power for x seconds
				// $*-f float	Duration			// duration in seconds
				// $*-f float	DamagePerUpdate		// DoT damage suffered by user, damage per update
				// $*-f float	UpdateFrequency		// DoT update frequency in seconds
				// $*-i uint16	DamageBonus			// damage bonus (20 = +20 damage points before success factor)
				CSpecialPowerDoT*power = new CSpecialPowerDoT(_ActorRowId, this, ((CSBrickParamBerserk *)param)->Duration, ((CSBrickParamBerserk *)param)->DisableTime, POWERS::Berserk);
				if (power)
				{
					power->setUpdateFrequency(((CSBrickParamBerserk *)param)->UpdateFrequency);
					power->setDamagePerUpdate(((CSBrickParamBerserk *)param)->DamagePerUpdate);
					power->setParamValue(((CSBrickParamBerserk *)param)->DamageBonus);
					power->setEffectFamily( EFFECT_FAMILIES::PowerBerserker );
					power->setByPass(isConsumable);
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_ENCHANT_WEAPON:
			{
				// $*STRUCT CSBrickParamEnchantWeapon TBrickParam::SP_ENCHANT_WEAPON
				// $*-f float	DisableTime			// disable berserker power for x seconds
				// $*-f float	Duration			// duration in seconds
				// $*-s std::string DamageType		// damage type
				// $*-f float	DpsBonus			// DoT update frequency in seconds
				// $*-i uint16	DamageBonus			// damage bonus (20 = +20 damage points before success factor)
				CSpecialPowerEnchantWeapon* power = new CSpecialPowerEnchantWeapon(_ActorRowId, this, ((CSBrickParamEnchantWeapon*)param)->Duration, ((CSBrickParamEnchantWeapon*)param)->DisableTime, POWERS::EnchantWeapon);
				if (power)
				{
					power->setDamageType(DMGTYPE::stringToDamageType(((CSBrickParamEnchantWeapon*)param)->DamageType));
					power->setDpsBonus(((CSBrickParamEnchantWeapon*)param)->DpsBonus);
					power->setParamValue(((CSBrickParamEnchantWeapon*)param)->DamageBonus);
					power->setEffectFamily( EFFECT_FAMILIES::PowerEnchantWeapon );
					_Powers.push_back(power);
				}
			}
			break;
			
		case TBrickParam::SP_BALANCE:
			{
				// $*STRUCT CSBrickParamBalance: public TBrickParam::CId <TBrickParam::SP_BALANCE>
				// $*-f float		DisableTime			// disable power for x seconds
				// $*-s std::string AffectedScore		// affected score 
				// $*-f float		Range				// power range
				// $*-f float		LossFactor			// score loss factor (0.5 = 50%, 0.3 = 30% loss..)
				const CSBrickParamBalance *cp = (CSBrickParamBalance*) (param);
				CSpecialPowerBalance *power = new CSpecialPowerBalance( _ActorRowId, this, cp->DisableTime, cp->LossFactor/100.0f, cp->Range,  POWERS::BalanceHp );
				if (power)
				{
					power->setAffectedScore( SCORES::toScore(cp->AffectedScore) );
					power->setByPass(isConsumable);
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_HEAL:
			{
				// $*STRUCT CSBrickParamHeal: public TBrickParam::CId <TBrickParam::SP_HEAL>
				// $*-s std::string AffectedScore		// affected score 
				// $*-i sint32		HealValue			// value added to affected score
				// $*-f float		HealFactorValue		// value added to affected score in % of max target score
				// $*-f float		DisableTime			// disable power for x seconds
				// $*-s std::string	PowerType			// type of power (Heal, HealHpC ...)
				const CSBrickParamHeal *cp = (CSBrickParamHeal*) (param);
				if ( cp->HealValue > 0 || cp->HealFactorValue > 0.0f)
				{
					CSpecialPowerHeal *power = new CSpecialPowerHeal( _ActorRowId, this, cp->DisableTime, cp->HealValue, cp->HealFactorValue / 100.0f, POWERS::toPowerType(cp->PowerType) );
					if (power)
					{
						power->setAffectedScore( SCORES::toScore(cp->AffectedScore) );
						power->setByPass(isConsumable);
						_Powers.push_back(power);
					}
				}
			}
			break;

		case TBrickParam::SP_CHG_CHARAC:
			{
				// $*STRUCT CSBrickParamChgCharac: public TBrickParam::CId <TBrickParam::SP_CHG_CHARAC>
				// $*-s std::string AffectedCharac	// affected characteristic
				// $*-f float	ModifCoef			// coefficient to modify characteristic proportionally to item level
				// $*-i float	ModifConst			// characteristic modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamChgCharac *cp = (CSBrickParamChgCharac*) (param);
				CSpecialPowerChgCharac *power = new CSpecialPowerChgCharac(_ActorRowId, this, cp->Duration, POWERS::ChgCharac, cp->AffectedCharac, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;
			

		case TBrickParam::SP_MOD_DEFENSE:
			{
				// $*STRUCT CSBrickParamModDefense: public TBrickParam::CId <TBrickParam::SP_MOD_DEFENSE>
				// $*-s std::string DefenseMode		// dodge or parry ?
				// $*-f float	ModifCoef			// coefficient to modify defense success chance proportionally to item level
				// $*-i float	ModifConst			// constant modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModDefense *cp = (CSBrickParamModDefense*) (param);
				CSpecialPowerModDefense *power = new CSpecialPowerModDefense(_ActorRowId, this, cp->Duration, cp->DefenseMode, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MOD_CRAFT_SUCCESS:
			{
				// $*STRUCT CSBrickParamModCraftSuccess: public TBrickParam::CId <TBrickParam::SP_MOD_CRAFT_SUCCESS>
				// $*-f float	ModifCoef			// coefficient to modify craft success chance proportionally to item level
				// $*-i float	ModifConst			// constant modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModCraftSuccess *cp = (CSBrickParamModCraftSuccess*) (param);
				CSpecialPowerModCraftSuccess *power = new CSpecialPowerModCraftSuccess(_ActorRowId, this, cp->Duration, POWERS::ModCraftSkill, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MOD_MELEE_SUCCESS:
			{
				// $*STRUCT CSBrickParamModMeleeSuccess: public TBrickParam::CId <TBrickParam::SP_MOD_MELEE_SUCCESS>
				// $*-f float	ModifCoef			// coefficient to modify melee success chance proportionally to item level
				// $*-i float	ModifConst			// constant modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModMeleeSuccess *cp = (CSBrickParamModMeleeSuccess*) (param);
				CSpecialPowerModMeleeSuccess *power = new CSpecialPowerModMeleeSuccess(_ActorRowId, this, cp->Duration, POWERS::ModMeleeSkill, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MOD_RANGE_SUCCESS:
			{
				// $*STRUCT CSBrickParamModRangeSuccess: public TBrickParam::CId <TBrickParam::SP_MOD_RANGE_SUCCESS>
				// $*-f float	ModifCoef			// coefficient to modify range success chance proportionally to item level
				// $*-i float	ModifConst			// constant modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModRangeSuccess *cp = (CSBrickParamModRangeSuccess*) (param);
				CSpecialPowerModRangeSuccess *power = new CSpecialPowerModRangeSuccess(_ActorRowId, this, cp->Duration, POWERS::ModRangeSkill, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MOD_MAGIC_SUCCESS:
			{
				// $*STRUCT CSBrickParamModMagicSuccess: public TBrickParam::CId <TBrickParam::SP_MOD_MAGIC_SUCCESS>
				// $*-f float	ModifCoef			// coefficient to modify magic success chance proportionally to item level
				// $*-i float	ModifConst			// constant modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModMagicSuccess *cp = (CSBrickParamModMagicSuccess*) (param);
				CSpecialPowerModMagicSuccess *power = new CSpecialPowerModMagicSuccess(_ActorRowId, this, cp->Duration, POWERS::ModMagicSkill, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MOD_FORAGE_SUCCESS:
			{
				// $*STRUCT CSBrickParamModForageSuccess: public TBrickParam::CId <TBrickParam::SP_MOD_FORAGE_SUCCESS>
				// $*-s std::string Ecosystem		// cf ecosystem.h
				// $*-f float	ModifCoef			// coefficient to modify forage success chance proportionally to item level
				// $*-i float	ModifConst			// constant modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModForageSuccess *cp = (CSBrickParamModForageSuccess*) (param);
				CSpecialPowerModForageSuccess *power = new CSpecialPowerModForageSuccess(_ActorRowId, this, cp->Duration, cp->Ecosystem, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::SP_MOD_MAGIC_PROTECTION:
			{
				// $*STRUCT CSBrickParamModMagicProtection: public TBrickParam::CId <TBrickParam::SP_MOD_MAGIC_PROTECTION>
				// $*-s std::string AffectedProtection	// affected magic protection
				// $*-f float	ModifCoef			// coefficient to modify characteristic proportionally to item level
				// $*-i float	ModifConst			// characteristic modifier
				// $*-f float	Duration			// duration in seconds
				const CSBrickParamModMagicProtection *cp = (CSBrickParamModMagicProtection*) (param);
				CSpecialPowerModMagicProtection *power = new CSpecialPowerModMagicProtection(_ActorRowId, this, cp->Duration, POWERS::ModMagicProtection, cp->AffectedProtection, cp->ModifierCoefficient*quality, cp->ModifierConstant );
				if (power)
				{
					_Powers.push_back(power);
				}
			}
			break;

		case TBrickParam::BYPASS_CHECK:
			{
				const CSBrickParamBypassCheck *cp = (CSBrickParamBypassCheck*) (param);
				_BypassCheckFlags.setFlag(CHECK_FLAG_TYPE::fromString(cp->FlagType), true);
			}

		default:
			break;
		};
	}
}// CSpecialPowerPhrase processParams


//-----------------------------------------------
// CSpecialPowerPhrase evaluate
//-----------------------------------------------
bool CSpecialPowerPhrase::evaluate()
{
	return true;
}// CSpecialPowerPhrase evaluate


//-----------------------------------------------
// CSpecialPowerPhrase validate
//-----------------------------------------------
bool CSpecialPowerPhrase::validate()
{
	string errorCode;
	_Targets.clear();
	
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
	if (!entity)
		return false;

	// test entity can use action
	if (entity->canEntityUseAction(_BypassCheckFlags) == false)
	{
		return false;
	}

	// set targets
	_Targets.push_back(entity->getTargetDataSetRow());

	// validate all powers
	const uint nbPowers = (uint)_Powers.size();
	for (uint i = 0 ; i < nbPowers ; ++i)
	{
#ifdef NL_DEBUG
		nlassert(_Powers[i] != NULL);
#endif
		if ( !_Powers[i]->validate(errorCode) )
			return false;
	}	

	return true;
}// CSpecialPowerPhrase validate


//-----------------------------------------------
// CSpecialPowerPhrase update
//-----------------------------------------------
bool  CSpecialPowerPhrase::update()
{
	// nothing to do
	return true;
}// CSpecialPowerPhrase update


//-----------------------------------------------
// CSpecialPowerPhrase execute
//-----------------------------------------------
void  CSpecialPowerPhrase::execute()
{
	_ExecutionEndDate = 0;
	// nothing to do
}// CSpecialPowerPhrase execute


//-----------------------------------------------
// CSpecialPowerPhrase launch
//-----------------------------------------------
bool CSpecialPowerPhrase::launch()
{
	// apply immediatly
	_ApplyDate = 0;
	return true;
}//CSpecialPowerPhrase launch


//-----------------------------------------------
// CSpecialPowerPhrase apply
//-----------------------------------------------
void CSpecialPowerPhrase::apply()
{
	_LatencyEndDate = 0;

	// TODO
	// apply effect

	

	applyPowers();
	
}//CSpecialPowerPhrase apply


//-----------------------------------------------
// CSpecialPowerPhrase applyPowers
//-----------------------------------------------
void CSpecialPowerPhrase::applyPowers()
{
	// apply powers
	const uint nbPowers = (uint)_Powers.size();
	for (uint i = 0 ; i < nbPowers ; ++i)
	{
	#ifdef NL_DEBUG
		nlassert(_Powers[i] != NULL);
	#endif
		_Powers[i]->apply();
	}
} // applyPowers //

//-----------------------------------------------
// CSpecialPowerPhrase end
//-----------------------------------------------
void CSpecialPowerPhrase::end()
{
	// nothing to do
} // end //

//-----------------------------------------------
// CSpecialPowerPhrase stop
//-----------------------------------------------
void CSpecialPowerPhrase::stop()
{
	// nothing to do
} // stop //

