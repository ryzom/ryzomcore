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

#ifndef RY_EGS_STATIC_BRICK_CPP_H
#define RY_EGS_STATIC_BRICK_CPP_H

#include "nel/misc/smart_ptr.h"

class TBrickParam
{
public:
	enum TValueType
	{
		SAP,
		HP,
		STA,
		STA_WEIGHT_FACTOR,
		FOCUS,
		SET_BEHAVIOUR,
		DEFINE_FLAG,
		BYPASS_CHECK,
		LATENCY_FACTOR,
		STA_LOSS_FACTOR,
		DEBUFF_REGEN,
		SAP_LOSS_FACTOR,
		AIM,
		ATT_SKILL_MOD,
		DEFENSE_MOD,
		THROW_OFF_BALANCE,
		INC_DMG,
		INC_DMG_TYPE_RSTR,
		INC_DMG_RACE_RSTR,
		INC_DMG_ECOS_RSTR,
		INC_DMG_SEASON_RSTR,
		SPECIAL_DAMAGE,
		ARMOR_MOD,
		SLOW_CAST,
		OPENING_1,
		OPENING_2,
		OPENING_3,
		COMBAT_SLOW_ATTACK,
		COMBAT_SLOW,
		BLEED_FACTOR,
		SPECIAL_HIT,
		HIT_ALL_AGGRESSORS,
		WEAPON_WEAR_MOD,
		CRITICAL_HIT_MOD,
		MA,
		MA_END,
		MA_EFFECT,
		MA_STAT,
		MA_EFFECT_MOD,
		MA_EFFECT_MULT,
		MA_CASTING_TIME,
		MA_DMG_TYPE,
		MA_DMG,
		MA_HEAL,
		MA_RANGE,
		MA_LINK_COST,
		MA_LINK_PERIOD,
		MA_CURE,
		MA_LINK_POWER,
		MA_BREAK_RES,
		MA_ARMOR_COMP,
		MA_VAMPIRISE,
		MA_VAMPIRISE_RATIO,
		CR_RECOMMENDED,
		CR_HP,
		CR_SAP,
		CR_STA,
		CR_FOCUS,
		CR_QUALITY,
		CR_DURABILITY,
		CR_DAMAGE,
		CR_HITRATE,
		CR_RANGE,
		CR_DMG_PROTECTION,
		CR_SAPLOAD,
		CR_WEIGHT,
		FG_RANGE,
		FG_LD_RANGE,
		FG_ANGLE,
		FG_MULTI,
		FG_KNOW,
		FG_TIME,
		FG_SRC_TIME,
		FG_STAT_ENERGY,
		FG_STAT_ENERGY_ONLY,
		FG_VIS_DIST,
		FG_VIS_STEALTH,
		FG_SRC_LOCATOR,
		FG_ATTEMPTS,
		FG_ABS_S,
		FG_ABS_A,
		FG_ABS_Q,
		FG_SRC_PRD,
		FG_SRC_APT,
		FG_QUALITY,
		FG_PRES,
		FG_STAB,
		FG_CR_STEALTH,
		FG_ABS_SRC_DMG,
		KAMI_OFFERING,
		KAMI_ANGER_DECREASE,
		FG_REDUCE_DMG,
		FG_ECT_SPC,
		FG_RMGRP_FILT,
		FG_RMFAM_FILT,
		FG_ITEMPART_FILT,
		SP_TAUNT,
		SP_SHIELDING,
		SP_LIFE_AURA,
		SP_LIFE_AURA2,
		SP_STAMINA_AURA,
		SP_STAMINA_AURA2,
		SP_SAP_AURA,
		SP_SAP_AURA2,
		SP_SPEEDING_UP,
		SP_INVULNERABILITY,
		SP_MELEE_PROTECTION_AURA,
		SP_RANGE_PROTECTION_AURA,
		SP_MAGIC_PROTECTION_AURA,
		SP_WAR_CRY_AURA,
		SP_FIRE_WALL_AURA,
		SP_THORN_WALL_AURA,
		SP_WATER_WALL_AURA,
		SP_LIGHTNING_WALL_AURA,
		SP_BERSERK,
		SP_ENCHANT_WEAPON,
		SP_CALM_ANIMAL,
		NEEDED_BRICK_FLAG,
		SP_BALANCE,
		SP_HEAL,
		SP_RECAST_TIME,
		SP_CHG_CHARAC,
		SP_MOD_DEFENSE,
		SP_MOD_CRAFT_SUCCESS,
		SP_MOD_MELEE_SUCCESS,
		SP_MOD_RANGE_SUCCESS,
		SP_MOD_MAGIC_SUCCESS,
		SP_MOD_FORAGE_SUCCESS,
		SP_MOD_MAGIC_PROTECTION,
		BONUS_FG_EXTRACTION_TIME_GC,
		BONUS_CR_DURABILITY,
		BONUS_LANDMARK_NUMBER,
		AREA_BOMB,
		AREA_SPRAY,
		AREA_CHAIN,
		AREA_TARGETS,
		MA_RECHARGE,
		CHARAC_UPGRADE,
		SCORE_UPGRADE,
		TA_TELEPORT,
		TA_DISCONNECT,
		TA_MOUNT,
		TA_UNMOUNT,
		TA_CONSUME,
		NUM_VALUES,
		BAD_VALUE= NUM_VALUES
	};

	class IId : public NLMISC::CRefCount
	{
	public:
		IId() : ParsedOk(false) { }
		TValueType id() const	{ return _Id; }
		void convertInput(std::vector<std::string> &args, const std::string &input)
		{
			unsigned i=0,j=0;
			while (i < input.size())
			{
				j=i;
				while( i<input.size() && input[i]!=':' ) ++i;
				args.push_back(input.substr(j,i-j));
				++i;
			}
		}

		bool ParsedOk;

	protected:
		TValueType _Id;
	};
	typedef NLMISC::CSmartPtr<IId> IIdPtr;

	const TBrickParam& operator =(std::string copyOfStr)
	{
		// convert the string to lower case
		for (unsigned i=0;i<copyOfStr.size();++i)
			if (copyOfStr[i]>='A' && copyOfStr[i]<='Z')
				copyOfStr[i]^=('A'^'a');

		if (copyOfStr=="sap") {_Value=SAP; return *this;}
		if (copyOfStr=="hp") {_Value=HP; return *this;}
		if (copyOfStr=="sta") {_Value=STA; return *this;}
		if (copyOfStr=="sta_weight_factor") {_Value=STA_WEIGHT_FACTOR; return *this;}
		if (copyOfStr=="focus") {_Value=FOCUS; return *this;}
		if (copyOfStr=="set_behaviour") {_Value=SET_BEHAVIOUR; return *this;}
		if (copyOfStr=="define_flag") {_Value=DEFINE_FLAG; return *this;}
		if (copyOfStr=="bypass_check") {_Value=BYPASS_CHECK; return *this;}
		if (copyOfStr=="latency_factor") {_Value=LATENCY_FACTOR; return *this;}
		if (copyOfStr=="sta_loss_factor") {_Value=STA_LOSS_FACTOR; return *this;}
		if (copyOfStr=="debuff_regen") {_Value=DEBUFF_REGEN; return *this;}
		if (copyOfStr=="sap_loss_factor") {_Value=SAP_LOSS_FACTOR; return *this;}
		if (copyOfStr=="aim") {_Value=AIM; return *this;}
		if (copyOfStr=="att_skill_mod") {_Value=ATT_SKILL_MOD; return *this;}
		if (copyOfStr=="defense_mod") {_Value=DEFENSE_MOD; return *this;}
		if (copyOfStr=="throw_off_balance") {_Value=THROW_OFF_BALANCE; return *this;}
		if (copyOfStr=="inc_dmg") {_Value=INC_DMG; return *this;}
		if (copyOfStr=="inc_dmg_type_rstr") {_Value=INC_DMG_TYPE_RSTR; return *this;}
		if (copyOfStr=="inc_dmg_race_rstr") {_Value=INC_DMG_RACE_RSTR; return *this;}
		if (copyOfStr=="inc_dmg_ecos_rstr") {_Value=INC_DMG_ECOS_RSTR; return *this;}
		if (copyOfStr=="inc_dmg_season_rstr") {_Value=INC_DMG_SEASON_RSTR; return *this;}
		if (copyOfStr=="special_damage") {_Value=SPECIAL_DAMAGE; return *this;}
		if (copyOfStr=="armor_mod") {_Value=ARMOR_MOD; return *this;}
		if (copyOfStr=="slow_cast") {_Value=SLOW_CAST; return *this;}
		if (copyOfStr=="opening_1") {_Value=OPENING_1; return *this;}
		if (copyOfStr=="opening_2") {_Value=OPENING_2; return *this;}
		if (copyOfStr=="opening_3") {_Value=OPENING_3; return *this;}
		if (copyOfStr=="combat_slow_attack") {_Value=COMBAT_SLOW_ATTACK; return *this;}
		if (copyOfStr=="combat_slow") {_Value=COMBAT_SLOW; return *this;}
		if (copyOfStr=="bleed_factor") {_Value=BLEED_FACTOR; return *this;}
		if (copyOfStr=="special_hit") {_Value=SPECIAL_HIT; return *this;}
		if (copyOfStr=="hit_all_aggressors") {_Value=HIT_ALL_AGGRESSORS; return *this;}
		if (copyOfStr=="weapon_wear_mod") {_Value=WEAPON_WEAR_MOD; return *this;}
		if (copyOfStr=="critical_hit_mod") {_Value=CRITICAL_HIT_MOD; return *this;}
		if (copyOfStr=="ma") {_Value=MA; return *this;}
		if (copyOfStr=="ma_end") {_Value=MA_END; return *this;}
		if (copyOfStr=="ma_effect") {_Value=MA_EFFECT; return *this;}
		if (copyOfStr=="ma_stat") {_Value=MA_STAT; return *this;}
		if (copyOfStr=="ma_effect_mod") {_Value=MA_EFFECT_MOD; return *this;}
		if (copyOfStr=="ma_effect_mult") {_Value=MA_EFFECT_MULT; return *this;}
		if (copyOfStr=="ma_casting_time") {_Value=MA_CASTING_TIME; return *this;}
		if (copyOfStr=="ma_dmg_type") {_Value=MA_DMG_TYPE; return *this;}
		if (copyOfStr=="ma_dmg") {_Value=MA_DMG; return *this;}
		if (copyOfStr=="ma_heal") {_Value=MA_HEAL; return *this;}
		if (copyOfStr=="ma_range") {_Value=MA_RANGE; return *this;}
		if (copyOfStr=="ma_link_cost") {_Value=MA_LINK_COST; return *this;}
		if (copyOfStr=="ma_link_period") {_Value=MA_LINK_PERIOD; return *this;}
		if (copyOfStr=="ma_cure") {_Value=MA_CURE; return *this;}
		if (copyOfStr=="ma_link_power") {_Value=MA_LINK_POWER; return *this;}
		if (copyOfStr=="ma_break_res") {_Value=MA_BREAK_RES; return *this;}
		if (copyOfStr=="ma_armor_comp") {_Value=MA_ARMOR_COMP; return *this;}
		if (copyOfStr=="ma_vampirise") {_Value=MA_VAMPIRISE; return *this;}
		if (copyOfStr=="ma_vampirise_ratio") {_Value=MA_VAMPIRISE_RATIO; return *this;}
		if (copyOfStr=="cr_recommended") {_Value=CR_RECOMMENDED; return *this;}
		if (copyOfStr=="cr_hp") {_Value=CR_HP; return *this;}
		if (copyOfStr=="cr_sap") {_Value=CR_SAP; return *this;}
		if (copyOfStr=="cr_sta") {_Value=CR_STA; return *this;}
		if (copyOfStr=="cr_focus") {_Value=CR_FOCUS; return *this;}
		if (copyOfStr=="cr_quality") {_Value=CR_QUALITY; return *this;}
		if (copyOfStr=="cr_durability") {_Value=CR_DURABILITY; return *this;}
		if (copyOfStr=="cr_damage") {_Value=CR_DAMAGE; return *this;}
		if (copyOfStr=="cr_hitrate") {_Value=CR_HITRATE; return *this;}
		if (copyOfStr=="cr_range") {_Value=CR_RANGE; return *this;}
		if (copyOfStr=="cr_dmg_protection") {_Value=CR_DMG_PROTECTION; return *this;}
		if (copyOfStr=="cr_sapload") {_Value=CR_SAPLOAD; return *this;}
		if (copyOfStr=="cr_weight") {_Value=CR_WEIGHT; return *this;}
		if (copyOfStr=="fg_range") {_Value=FG_RANGE; return *this;}
		if (copyOfStr=="fg_ld_range") {_Value=FG_LD_RANGE; return *this;}
		if (copyOfStr=="fg_angle") {_Value=FG_ANGLE; return *this;}
		if (copyOfStr=="fg_multi") {_Value=FG_MULTI; return *this;}
		if (copyOfStr=="fg_know") {_Value=FG_KNOW; return *this;}
		if (copyOfStr=="fg_time") {_Value=FG_TIME; return *this;}
		if (copyOfStr=="fg_src_time") {_Value=FG_SRC_TIME; return *this;}
		if (copyOfStr=="fg_stat_energy") {_Value=FG_STAT_ENERGY; return *this;}
		if (copyOfStr=="fg_stat_energy_only") {_Value=FG_STAT_ENERGY_ONLY; return *this;}
		if (copyOfStr=="fg_vis_dist") {_Value=FG_VIS_DIST; return *this;}
		if (copyOfStr=="fg_vis_stealth") {_Value=FG_VIS_STEALTH; return *this;}
		if (copyOfStr=="fg_src_locator") {_Value=FG_SRC_LOCATOR; return *this;}
		if (copyOfStr=="fg_attempts") {_Value=FG_ATTEMPTS; return *this;}
		if (copyOfStr=="fg_abs_s") {_Value=FG_ABS_S; return *this;}
		if (copyOfStr=="fg_abs_a") {_Value=FG_ABS_A; return *this;}
		if (copyOfStr=="fg_abs_q") {_Value=FG_ABS_Q; return *this;}
		if (copyOfStr=="fg_src_prd") {_Value=FG_SRC_PRD; return *this;}
		if (copyOfStr=="fg_src_apt") {_Value=FG_SRC_APT; return *this;}
		if (copyOfStr=="fg_quality") {_Value=FG_QUALITY; return *this;}
		if (copyOfStr=="fg_pres") {_Value=FG_PRES; return *this;}
		if (copyOfStr=="fg_stab") {_Value=FG_STAB; return *this;}
		if (copyOfStr=="fg_cr_stealth") {_Value=FG_CR_STEALTH; return *this;}
		if (copyOfStr=="fg_abs_src_dmg") {_Value=FG_ABS_SRC_DMG; return *this;}
		if (copyOfStr=="kami_offering") {_Value=KAMI_OFFERING; return *this;}
		if (copyOfStr=="kami_anger_decrease") {_Value=KAMI_ANGER_DECREASE; return *this;}
		if (copyOfStr=="fg_reduce_dmg") {_Value=FG_REDUCE_DMG; return *this;}
		if (copyOfStr=="fg_ect_spc") {_Value=FG_ECT_SPC; return *this;}
		if (copyOfStr=="fg_rmgrp_filt") {_Value=FG_RMGRP_FILT; return *this;}
		if (copyOfStr=="fg_rmfam_filt") {_Value=FG_RMFAM_FILT; return *this;}
		if (copyOfStr=="fg_itempart_filt") {_Value=FG_ITEMPART_FILT; return *this;}
		if (copyOfStr=="sp_taunt") {_Value=SP_TAUNT; return *this;}
		if (copyOfStr=="sp_shielding") {_Value=SP_SHIELDING; return *this;}
		if (copyOfStr=="sp_life_aura") {_Value=SP_LIFE_AURA; return *this;}
		if (copyOfStr=="sp_life_aura2") {_Value=SP_LIFE_AURA2; return *this;}
		if (copyOfStr=="sp_stamina_aura") {_Value=SP_STAMINA_AURA; return *this;}
		if (copyOfStr=="sp_stamina_aura2") {_Value=SP_STAMINA_AURA2; return *this;}
		if (copyOfStr=="sp_sap_aura") {_Value=SP_SAP_AURA; return *this;} 
		if (copyOfStr=="sp_sap_aura2") {_Value=SP_SAP_AURA2; return *this;}
		if (copyOfStr=="sp_speeding_up") {_Value=SP_SPEEDING_UP; return *this;}
		if (copyOfStr=="sp_invulnerability") {_Value=SP_INVULNERABILITY; return *this;}
		if (copyOfStr=="sp_melee_protection_aura") {_Value=SP_MELEE_PROTECTION_AURA; return *this;}
		if (copyOfStr=="sp_range_protection_aura") {_Value=SP_RANGE_PROTECTION_AURA; return *this;}
		if (copyOfStr=="sp_magic_protection_aura") {_Value=SP_MAGIC_PROTECTION_AURA; return *this;}
		if (copyOfStr=="sp_war_cry_aura") {_Value=SP_WAR_CRY_AURA; return *this;}
		if (copyOfStr=="sp_fire_wall_aura") {_Value=SP_FIRE_WALL_AURA; return *this;}
		if (copyOfStr=="sp_thorn_wall_aura") {_Value=SP_THORN_WALL_AURA; return *this;}
		if (copyOfStr=="sp_water_wall_aura") {_Value=SP_WATER_WALL_AURA; return *this;}
		if (copyOfStr=="sp_lightning_wall_aura") {_Value=SP_LIGHTNING_WALL_AURA; return *this;}
		if (copyOfStr=="sp_berserk") {_Value=SP_BERSERK; return *this;}
		if (copyOfStr=="sp_enchant_weapon") {_Value=SP_ENCHANT_WEAPON; return *this;}
		if (copyOfStr=="sp_calm_animal") {_Value=SP_CALM_ANIMAL; return *this;}
		if (copyOfStr=="needed_brick_flag") {_Value=NEEDED_BRICK_FLAG; return *this;}
		if (copyOfStr=="sp_balance") {_Value=SP_BALANCE; return *this;}
		if (copyOfStr=="sp_heal") {_Value=SP_HEAL; return *this;}
		if (copyOfStr=="sp_recast_time") {_Value=SP_RECAST_TIME; return *this;}
		if (copyOfStr=="sp_chg_charac") {_Value=SP_CHG_CHARAC; return *this;}
		if (copyOfStr=="sp_mod_defense") {_Value=SP_MOD_DEFENSE; return *this;}
		if (copyOfStr=="sp_mod_craft_success") {_Value=SP_MOD_CRAFT_SUCCESS; return *this;}
		if (copyOfStr=="sp_mod_melee_success") {_Value=SP_MOD_MELEE_SUCCESS; return *this;}
		if (copyOfStr=="sp_mod_range_success") {_Value=SP_MOD_RANGE_SUCCESS; return *this;}
		if (copyOfStr=="sp_mod_magic_success") {_Value=SP_MOD_MAGIC_SUCCESS; return *this;}
		if (copyOfStr=="sp_mod_forage_success") {_Value=SP_MOD_FORAGE_SUCCESS; return *this;}
		if (copyOfStr=="sp_mod_magic_protection") {_Value=SP_MOD_MAGIC_PROTECTION; return *this;}
		if (copyOfStr=="bonus_fg_extraction_time_gc") {_Value=BONUS_FG_EXTRACTION_TIME_GC; return *this;}
		if (copyOfStr=="bonus_cr_durability") {_Value=BONUS_CR_DURABILITY; return *this;}
		if (copyOfStr=="bonus_landmark_number") {_Value=BONUS_LANDMARK_NUMBER; return *this;}
		if (copyOfStr=="area_bomb") {_Value=AREA_BOMB; return *this;}
		if (copyOfStr=="area_spray") {_Value=AREA_SPRAY; return *this;}
		if (copyOfStr=="area_chain") {_Value=AREA_CHAIN; return *this;}
		if (copyOfStr=="area_targets") {_Value=AREA_TARGETS; return *this;}
		if (copyOfStr=="ma_recharge") {_Value=MA_RECHARGE; return *this;}
		if (copyOfStr=="charac_upgrade") {_Value=CHARAC_UPGRADE; return *this;}
		if (copyOfStr=="score_upgrade") {_Value=SCORE_UPGRADE; return *this;}
		if (copyOfStr=="ta_teleport") {_Value=TA_TELEPORT; return *this;}
		if (copyOfStr=="ta_disconnect") {_Value=TA_DISCONNECT; return *this;}
		if (copyOfStr=="ta_mount") {_Value=TA_MOUNT; return *this;}
		if (copyOfStr=="ta_unmount") {_Value=TA_UNMOUNT; return *this;}
		if (copyOfStr=="ta_consume") {_Value=TA_CONSUME; return *this;}

		_Value=BAD_VALUE;
		return *this;
	}

	TBrickParam(const char* value)
	{
		*this=std::string(value);
	}

	TBrickParam(const std::string &value)
	{
		*this=value;
	}

	const TBrickParam& operator =(TValueType value)
	{
		_Value=value;
		return *this;
	}

	operator TValueType() const
	{
		return _Value;
	}

private:
	TValueType _Value;
};

struct CSBrickParamSap : public TBrickParam::IId
{
	// quantity of SAP to use
	unsigned Sap;

	CSBrickParamSap():
		Sap(0)
	{
		_Id = TBrickParam::SAP;
	}

	CSBrickParamSap(const std::string&str)
	{
		*this=CSBrickParamSap();
		*this=str;
	}

	const CSBrickParamSap& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Sap);

		return *this;
	}
};


struct CSBrickParamHp : public TBrickParam::IId
{
	// quantity of HP to use
	unsigned Hp;

	CSBrickParamHp():
		Hp(0)
	{
		_Id = TBrickParam::HP;
	}

	CSBrickParamHp(const std::string&str)
	{
		*this=CSBrickParamHp();
		*this=str;
	}

	const CSBrickParamHp& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Hp);

		return *this;
	}
};


struct CSBrickParamSta : public TBrickParam::IId
{
	// quantity of STA to use
	unsigned Sta;

	CSBrickParamSta():
		Sta(0)
	{
		_Id = TBrickParam::STA;
	}

	CSBrickParamSta(const std::string&str)
	{
		*this=CSBrickParamSta();
		*this=str;
	}

	const CSBrickParamSta& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Sta);

		return *this;
	}
};


struct CSBrickParamStaWeightFactor : public TBrickParam::IId
{
	// STA factor of weight to use
	float StaFactor;

	// STA constante used
	unsigned StaConst;

	CSBrickParamStaWeightFactor():
		StaFactor(0.0f),
		StaConst(0)
	{
		_Id = TBrickParam::STA_WEIGHT_FACTOR;
	}
	
	CSBrickParamStaWeightFactor(const std::string&str)
	{
		*this=CSBrickParamStaWeightFactor();
		*this=str;
	}
	
	const CSBrickParamStaWeightFactor& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()!=2)
			return *this;
		
		ParsedOk=true;
		StaFactor=(float)atof(args[0].c_str());
		NLMISC::fromString(args[1], StaConst);
		
		return *this;
	}
};


struct CSBrickParamFocus : public TBrickParam::IId
{
	// quantity of FOCUS to use
	unsigned Focus;

	CSBrickParamFocus():
		Focus(0)
	{
		_Id = TBrickParam::FOCUS;
	}

	CSBrickParamFocus(const std::string&str)
	{
		*this=CSBrickParamFocus();
		*this=str;
	}

	const CSBrickParamFocus& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Focus);

		return *this;
	}
};


struct CSBrickParamSetBehaviour : public TBrickParam::IId
{
	// the new behaviour to use
	std::string Behaviour;

	CSBrickParamSetBehaviour():
		Behaviour()
	{
		_Id = TBrickParam::SET_BEHAVIOUR;
	}

	CSBrickParamSetBehaviour(const std::string&str)
	{
		*this=CSBrickParamSetBehaviour();
		*this=str;
	}

	const CSBrickParamSetBehaviour& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Behaviour=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamDefineFlag : public TBrickParam::IId
{
	// the defined flag
	std::string Flag;

	CSBrickParamDefineFlag():
		Flag()
	{
		_Id = TBrickParam::DEFINE_FLAG;
	}

	CSBrickParamDefineFlag(const std::string&str)
	{
		*this=CSBrickParamDefineFlag();
		*this=str;
	}

	const CSBrickParamDefineFlag& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Flag=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamBypassCheck : public TBrickParam::IId
{
	// the check flag to bypass
	std::string FlagType;

	CSBrickParamBypassCheck():
		FlagType()
	{
		_Id = TBrickParam::BYPASS_CHECK;
	}

	CSBrickParamBypassCheck(const std::string&str)
	{
		*this=CSBrickParamBypassCheck();
		*this=str;
	}

	const CSBrickParamBypassCheck& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		FlagType=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamLatencyFactor : public TBrickParam::IId
{
	// min factor on weapon latency
	float MinLatencyFactor;
	// max factor on weapon latency
	float MaxLatencyFactor;

	CSBrickParamLatencyFactor():
		MinLatencyFactor(1),
		MaxLatencyFactor(0.5)
	{
		_Id = TBrickParam::LATENCY_FACTOR;
	}

	CSBrickParamLatencyFactor(const std::string&str)
	{
		*this=CSBrickParamLatencyFactor();
		*this=str;
	}

	const CSBrickParamLatencyFactor& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinLatencyFactor=(float)atof(args[0].c_str());
		MaxLatencyFactor=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamStaLossFactor : public TBrickParam::IId
{
	// min factor of damage also applied to stamina
	float MinFactor;
	// max factor of damage also applied to stamina
	float MaxFactor;

	CSBrickParamStaLossFactor():
		MinFactor(0.0),
		MaxFactor(0.0)
	{
		_Id = TBrickParam::STA_LOSS_FACTOR;
	}

	CSBrickParamStaLossFactor(const std::string&str)
	{
		*this=CSBrickParamStaLossFactor();
		*this=str;
	}

	const CSBrickParamStaLossFactor& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinFactor=(float)atof(args[0].c_str());
		MaxFactor=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamDebuffRegen : public TBrickParam::IId
{
	// affected score regen (Sap, Stamina, HitPoints, Focus)
	std::string Score;
	// duration in seconds
	float Duration;
	// min factor of regen debuff
	float MinFactor;
	// max factor of regen debuff
	float MaxFactor;

	CSBrickParamDebuffRegen():
		Score(),
		Duration(0.0),
		MinFactor(0.0),
		MaxFactor(0.0)
	{
		_Id = TBrickParam::DEBUFF_REGEN;
	}

	CSBrickParamDebuffRegen(const std::string&str)
	{
		*this=CSBrickParamDebuffRegen();
		*this=str;
	}

	const CSBrickParamDebuffRegen& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=4)
			return *this;

		ParsedOk=true;
		Score=args[0].c_str();
		Duration=(float)atof(args[1].c_str());
		MinFactor=(float)atof(args[2].c_str());
		MaxFactor=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamSapLossFactor : public TBrickParam::IId
{
	// min factor of damage also applied to sap
	float MinFactor;
	// max factor of damage also applied to sap
	float MaxFactor;

	CSBrickParamSapLossFactor():
		MinFactor(0.0),
		MaxFactor(0.0)
	{
		_Id = TBrickParam::SAP_LOSS_FACTOR;
	}

	CSBrickParamSapLossFactor(const std::string&str)
	{
		*this=CSBrickParamSapLossFactor();
		*this=str;
	}

	const CSBrickParamSapLossFactor& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinFactor=(float)atof(args[0].c_str());
		MaxFactor=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamAim : public TBrickParam::IId
{
	// Homin, kitin (=land kitin), bird, flying_kitin....(see body.h)
	std::string BodyType;
	// head, body, arms... (see slot_equipment.cpp)
	std::string AimedSlot;

	CSBrickParamAim():
		BodyType(),
		AimedSlot()
	{
		_Id = TBrickParam::AIM;
	}

	CSBrickParamAim(const std::string&str)
	{
		*this=CSBrickParamAim();
		*this=str;
	}

	const CSBrickParamAim& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		BodyType=args[0].c_str();
		AimedSlot=args[1].c_str();

		return *this;
	}
};


struct CSBrickParamAttackSkillModifier : public TBrickParam::IId
{
	// min modifier on attacker skills to hit its target
	sint32 MinModifier;
	// max modifier on attacker skills to hit its target
	sint32 MaxModifier;

	CSBrickParamAttackSkillModifier():
		MinModifier(0),
		MaxModifier(20)
	{
		_Id = TBrickParam::ATT_SKILL_MOD;
	}

	CSBrickParamAttackSkillModifier(const std::string&str)
	{
		*this=CSBrickParamAttackSkillModifier();
		*this=str;
	}

	const CSBrickParamAttackSkillModifier& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], MinModifier);
		NLMISC::fromString(args[1], MaxModifier);

		return *this;
	}
};


struct CSBrickParamDefenseModifier : public TBrickParam::IId
{
	// min modifier on attacker defense skills during the attack
	sint32 MinModifier;
	// max modifier on attacker defense skills during the attack
	sint32 MaxModifier;

	CSBrickParamDefenseModifier():
		MinModifier(0),
		MaxModifier(-20)
	{
		_Id = TBrickParam::DEFENSE_MOD;
	}

	CSBrickParamDefenseModifier(const std::string&str)
	{
		*this=CSBrickParamDefenseModifier();
		*this=str;
	}

	const CSBrickParamDefenseModifier& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], MinModifier);
		NLMISC::fromString(args[1], MaxModifier);

		return *this;
	}
};


struct CSBrickParamThrowOffBalance : public TBrickParam::IId
{
	// effect min duration
	float MinDuration;
	// effect max duration 
	float MaxDuration;

	CSBrickParamThrowOffBalance():
		MinDuration(0.0f),
		MaxDuration(5.0f)
	{
		_Id = TBrickParam::THROW_OFF_BALANCE;
	}

	CSBrickParamThrowOffBalance(const std::string&str)
	{
		*this=CSBrickParamThrowOffBalance();
		*this=str;
	}

	const CSBrickParamThrowOffBalance& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinDuration=(float)atof(args[0].c_str());
		MaxDuration=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamIncreaseDamage : public TBrickParam::IId
{
	//min factor on damage
	float MinFactor;
	//max factor on damage
	float MaxFactor;

	CSBrickParamIncreaseDamage():
		MinFactor(1.0f),
		MaxFactor(2.0f)
	{
		_Id = TBrickParam::INC_DMG;
	}

	CSBrickParamIncreaseDamage(const std::string&str)
	{
		*this=CSBrickParamIncreaseDamage();
		*this=str;
	}

	const CSBrickParamIncreaseDamage& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinFactor=(float)atof(args[0].c_str());
		MaxFactor=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamIncDmgTypeRestriction : public TBrickParam::IId
{
	//type restriction
	std::string TypeRestriction;
	//bonus on damage factor
	float FactorModifier;

	CSBrickParamIncDmgTypeRestriction():
		TypeRestriction(),
		FactorModifier(0.0f)
	{
		_Id = TBrickParam::INC_DMG_TYPE_RSTR;
	}

	CSBrickParamIncDmgTypeRestriction(const std::string&str)
	{
		*this=CSBrickParamIncDmgTypeRestriction();
		*this=str;
	}

	const CSBrickParamIncDmgTypeRestriction& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		TypeRestriction=args[0].c_str();
		FactorModifier=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamIncDmgRaceRestriction : public TBrickParam::IId
{
	//race restriction
	std::string RaceRestriction;
	//bonus on damage factor
	float FactorModifier;

	CSBrickParamIncDmgRaceRestriction():
		RaceRestriction(),
		FactorModifier(0.0f)
	{
		_Id = TBrickParam::INC_DMG_RACE_RSTR;
	}

	CSBrickParamIncDmgRaceRestriction(const std::string&str)
	{
		*this=CSBrickParamIncDmgRaceRestriction();
		*this=str;
	}

	const CSBrickParamIncDmgRaceRestriction& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		RaceRestriction=args[0].c_str();
		FactorModifier=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamIncDmgEcosystemRestriction : public TBrickParam::IId
{
	//Ecosystem restriction
	std::string EcosystemRestriction;
	//bonus on damage factor
	float FactorModifier;

	CSBrickParamIncDmgEcosystemRestriction():
		EcosystemRestriction(),
		FactorModifier(0.0f)
	{
		_Id = TBrickParam::INC_DMG_ECOS_RSTR;
	}

	CSBrickParamIncDmgEcosystemRestriction(const std::string&str)
	{
		*this=CSBrickParamIncDmgEcosystemRestriction();
		*this=str;
	}

	const CSBrickParamIncDmgEcosystemRestriction& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		EcosystemRestriction=args[0].c_str();
		FactorModifier=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamIncDmgSeasonRestriction : public TBrickParam::IId
{
	//Season restriction
	std::string SeasonRestriction;
	//bonus on damage factor
	float FactorModifier;

	CSBrickParamIncDmgSeasonRestriction():
		SeasonRestriction(),
		FactorModifier(0.0f)
	{
		_Id = TBrickParam::INC_DMG_SEASON_RSTR;
	}

	CSBrickParamIncDmgSeasonRestriction(const std::string&str)
	{
		*this=CSBrickParamIncDmgSeasonRestriction();
		*this=str;
	}

	const CSBrickParamIncDmgSeasonRestriction& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		SeasonRestriction=args[0].c_str();
		FactorModifier=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamSpecialDamage : public TBrickParam::IId
{
	// damage type
	std::string DamageType;
	//min factor of damage
	float MinFactor;
	//max factor of damage
	float MaxFactor;

	CSBrickParamSpecialDamage():
		DamageType(),
		MinFactor(0.0f),
		MaxFactor(1.0f)
	{
		_Id = TBrickParam::SPECIAL_DAMAGE;
	}

	CSBrickParamSpecialDamage(const std::string&str)
	{
		*this=CSBrickParamSpecialDamage();
		*this=str;
	}

	const CSBrickParamSpecialDamage& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		DamageType=args[0].c_str();
		MinFactor=(float)atof(args[1].c_str());
		MaxFactor=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamArmorMod : public TBrickParam::IId
{
	// affected armor type (light, medium, heavy, kitin etc..)
	std::string ArmorType;
	//max factor applied on armor absorption
	float MinFactor;
	//max factor applied on armor absorption (< min as smaller is better)
	float MaxFactor;

	CSBrickParamArmorMod():
		ArmorType(),
		MinFactor(1.0f),
		MaxFactor(0.5f)
	{
		_Id = TBrickParam::ARMOR_MOD;
	}

	CSBrickParamArmorMod(const std::string&str)
	{
		*this=CSBrickParamArmorMod();
		*this=str;
	}

	const CSBrickParamArmorMod& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		ArmorType=args[0].c_str();
		MinFactor=(float)atof(args[1].c_str());
		MaxFactor=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamSlowCast : public TBrickParam::IId
{
	// duration of the slow in seconds
	float Duration;
	// min factor applied on cast time
	float MinFactor;
	// max factor applied on cast time
	float MaxFactor;

	CSBrickParamSlowCast():
		Duration(),
		MinFactor(1.0f),
		MaxFactor(2.0f)
	{
		_Id = TBrickParam::SLOW_CAST;
	}

	CSBrickParamSlowCast(const std::string&str)
	{
		*this=CSBrickParamSlowCast();
		*this=str;
	}

	const CSBrickParamSlowCast& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		Duration=(float)atof(args[0].c_str());
		MinFactor=(float)atof(args[1].c_str());
		MaxFactor=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamOpening1 : public TBrickParam::IId
{
	
	std::string EventFlag;

	CSBrickParamOpening1():
		EventFlag()
	{
		_Id = TBrickParam::OPENING_1;
	}

	CSBrickParamOpening1(const std::string&str)
	{
		*this=CSBrickParamOpening1();
		*this=str;
	}

	const CSBrickParamOpening1& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		EventFlag=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamOpening2 : public TBrickParam::IId
{
	
	std::string EventFlag1;
	
	std::string EventFlag2;

	CSBrickParamOpening2():
		EventFlag1(),
		EventFlag2()
	{
		_Id = TBrickParam::OPENING_2;
	}

	CSBrickParamOpening2(const std::string&str)
	{
		*this=CSBrickParamOpening2();
		*this=str;
	}

	const CSBrickParamOpening2& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		EventFlag1=args[0].c_str();
		EventFlag2=args[1].c_str();

		return *this;
	}
};


struct CSBrickParamOpening3 : public TBrickParam::IId
{
	
	std::string EventFlag1;
	
	std::string EventFlag2;
	
	std::string EventFlag3;

	CSBrickParamOpening3():
		EventFlag1(),
		EventFlag2(),
		EventFlag3()
	{
		_Id = TBrickParam::OPENING_3;
	}

	CSBrickParamOpening3(const std::string&str)
	{
		*this=CSBrickParamOpening3();
		*this=str;
	}

	const CSBrickParamOpening3& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		EventFlag1=args[0].c_str();
		EventFlag2=args[1].c_str();
		EventFlag3=args[2].c_str();

		return *this;
	}
};


struct CSBrickParamCombatSlowAttack : public TBrickParam::IId
{
	// duration of the effect in seconds
	float Duration;
	// min factor applied on target attack latency (+50 = +50%)
	sint32 MinFactor;
	// max factor applied on target attack latency (+50 = +50%)
	sint32 MaxFactor;

	CSBrickParamCombatSlowAttack():
		Duration(),
		MinFactor(),
		MaxFactor()
	{
		_Id = TBrickParam::COMBAT_SLOW_ATTACK;
	}

	CSBrickParamCombatSlowAttack(const std::string&str)
	{
		*this=CSBrickParamCombatSlowAttack();
		*this=str;
	}

	const CSBrickParamCombatSlowAttack& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		Duration=(float)atof(args[0].c_str());
		NLMISC::fromString(args[1], MinFactor);
		NLMISC::fromString(args[2], MaxFactor);

		return *this;
	}
};


struct CSBrickParamCombatSlow : public TBrickParam::IId
{
	// duration of the effect in seconds
	float Duration;
	// min factor applied on target attack latency or casting time (+50 = +50%)
	sint32 MinFactor;
	// max factor applied on target attack latency or casting time (+50 = +50%)
	sint32 MaxFactor;

	CSBrickParamCombatSlow():
		Duration(),
		MinFactor(),
		MaxFactor()
	{
		_Id = TBrickParam::COMBAT_SLOW;
	}

	CSBrickParamCombatSlow(const std::string&str)
	{
		*this=CSBrickParamCombatSlow();
		*this=str;
	}

	const CSBrickParamCombatSlow& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		Duration=(float)atof(args[0].c_str());
		NLMISC::fromString(args[1], MinFactor);
		NLMISC::fromString(args[2], MaxFactor);

		return *this;
	}
};


struct CSBrickParamBleedFactor : public TBrickParam::IId
{
	// duration of the effect in seconds
	float Duration;
	// min factor of dealt damage also lost in bleed
	float MinFactor;
	// max factor of dealt damage also lost in bleed
	float MaxFactor;

	CSBrickParamBleedFactor():
		Duration(),
		MinFactor(),
		MaxFactor()
	{
		_Id = TBrickParam::BLEED_FACTOR;
	}

	CSBrickParamBleedFactor(const std::string&str)
	{
		*this=CSBrickParamBleedFactor();
		*this=str;
	}

	const CSBrickParamBleedFactor& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		Duration=(float)atof(args[0].c_str());
		MinFactor=(float)atof(args[1].c_str());
		MaxFactor=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamSpecialHit : public TBrickParam::IId
{

	CSBrickParamSpecialHit()
	{
		_Id = TBrickParam::SPECIAL_HIT;
	}

	CSBrickParamSpecialHit(const std::string&str)
	{
		*this=CSBrickParamSpecialHit();
		*this=str;
	}

	const CSBrickParamSpecialHit& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamHitAllAggressors : public TBrickParam::IId
{
	// min factor on dealt damage (total damage divided among targets)
	float MinFactor;
	// max factor on dealt damage (total damage divided among targets)
	float MaxFactor;

	CSBrickParamHitAllAggressors():
		MinFactor(),
		MaxFactor()
	{
		_Id = TBrickParam::HIT_ALL_AGGRESSORS;
	}

	CSBrickParamHitAllAggressors(const std::string&str)
	{
		*this=CSBrickParamHitAllAggressors();
		*this=str;
	}

	const CSBrickParamHitAllAggressors& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinFactor=(float)atof(args[0].c_str());
		MaxFactor=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamWeaponWearMod : public TBrickParam::IId
{
	// min weapon wear modifier
	float MinModifier;
	// max weapon wear modifier
	float MaxModifier;

	CSBrickParamWeaponWearMod():
		MinModifier(),
		MaxModifier()
	{
		_Id = TBrickParam::WEAPON_WEAR_MOD;
	}

	CSBrickParamWeaponWearMod(const std::string&str)
	{
		*this=CSBrickParamWeaponWearMod();
		*this=str;
	}

	const CSBrickParamWeaponWearMod& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		MinModifier=(float)atof(args[0].c_str());
		MaxModifier=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamCriticalHitMod : public TBrickParam::IId
{
	// min critical hit chance modifier
	uint8 MinModifier;
	// max critical hit chance  modifier
	uint8 MaxModifier;

	CSBrickParamCriticalHitMod():
		MinModifier(),
		MaxModifier()
	{
		_Id = TBrickParam::CRITICAL_HIT_MOD;
	}

	CSBrickParamCriticalHitMod(const std::string&str)
	{
		*this=CSBrickParamCriticalHitMod();
		*this=str;
	}

	const CSBrickParamCriticalHitMod& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], MinModifier);
		NLMISC::fromString(args[1], MaxModifier);

		return *this;
	}
};


struct CSBrickParamMaType : public TBrickParam::IId
{
	// type name
	std::string Type;

	CSBrickParamMaType():
		Type()
	{
		_Id = TBrickParam::MA;
	}

	CSBrickParamMaType(const std::string&str)
	{
		*this=CSBrickParamMaType();
		*this=str;
	}

	const CSBrickParamMaType& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Type=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamMaEnd : public TBrickParam::IId
{

	CSBrickParamMaEnd()
	{
		_Id = TBrickParam::MA_END;
	}

	CSBrickParamMaEnd(const std::string&str)
	{
		*this=CSBrickParamMaEnd();
		*this=str;
	}

	const CSBrickParamMaEnd& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamMagicEffect : public TBrickParam::IId
{
	// effect name
	std::string Effect;

	CSBrickParamMagicEffect():
		Effect()
	{
		_Id = TBrickParam::MA_EFFECT;
	}

	CSBrickParamMagicEffect(const std::string&str)
	{
		*this=CSBrickParamMagicEffect();
		*this=str;
	}

	const CSBrickParamMagicEffect& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Effect=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamMagicStat : public TBrickParam::IId
{
	// affected stat
	std::string Stat;
	// affected stat type
	std::string Type;

	CSBrickParamMagicStat():
		Stat(),
		Type()
	{
		_Id = TBrickParam::MA_STAT;
	}

	CSBrickParamMagicStat(const std::string&str)
	{
		*this=CSBrickParamMagicStat();
		*this=str;
	}

	const CSBrickParamMagicStat& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		Stat=args[0].c_str();
		Type=args[1].c_str();

		return *this;
	}
};


struct CSBrickParamMagicEffectMod : public TBrickParam::IId
{
	// effect modifier
	sint32 EffectMod;

	CSBrickParamMagicEffectMod():
		EffectMod()
	{
		_Id = TBrickParam::MA_EFFECT_MOD;
	}

	CSBrickParamMagicEffectMod(const std::string&str)
	{
		*this=CSBrickParamMagicEffectMod();
		*this=str;
	}

	const CSBrickParamMagicEffectMod& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], EffectMod);

		return *this;
	}
};


struct CSBrickParamMagicEffectMult : public TBrickParam::IId
{
	// effect modifier
	float EffectMult;

	CSBrickParamMagicEffectMult():
		EffectMult()
	{
		_Id = TBrickParam::MA_EFFECT_MULT;
	}

	CSBrickParamMagicEffectMult(const std::string&str)
	{
		*this=CSBrickParamMagicEffectMult();
		*this=str;
	}

	const CSBrickParamMagicEffectMult& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		EffectMult=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCastingTime : public TBrickParam::IId
{
	// casting modifier in seconds
	float CastingTime;

	CSBrickParamCastingTime():
		CastingTime(0)
	{
		_Id = TBrickParam::MA_CASTING_TIME;
	}

	CSBrickParamCastingTime(const std::string&str)
	{
		*this=CSBrickParamCastingTime();
		*this=str;
	}

	const CSBrickParamCastingTime& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		CastingTime=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamMagicDmgType : public TBrickParam::IId
{
	// magic damage type
	std::string DmgType;

	CSBrickParamMagicDmgType():
		DmgType()
	{
		_Id = TBrickParam::MA_DMG_TYPE;
	}

	CSBrickParamMagicDmgType(const std::string&str)
	{
		*this=CSBrickParamMagicDmgType();
		*this=str;
	}

	const CSBrickParamMagicDmgType& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		DmgType=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamMagicDmg : public TBrickParam::IId
{
	// fixed modifier on energy
	sint32 Hp;
	// fixed modifier on energy
	sint32 Sap;
	// fixed modifier on energy
	sint32 Sta;

	CSBrickParamMagicDmg():
		Hp(0),
		Sap(0),
		Sta(0)
	{
		_Id = TBrickParam::MA_DMG;
	}

	CSBrickParamMagicDmg(const std::string&str)
	{
		*this=CSBrickParamMagicDmg();
		*this=str;
	}

	const CSBrickParamMagicDmg& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Hp);
		NLMISC::fromString(args[1], Sap);
		NLMISC::fromString(args[2], Sta);

		return *this;
	}
};


struct CSBrickParamMagicHeal : public TBrickParam::IId
{
	// fixed modifier on energy
	sint32 Hp;
	// fixed modifier on energy
	sint32 Sap;
	// fixed modifier on energy
	sint32 Sta;

	CSBrickParamMagicHeal():
		Hp(0),
		Sap(0),
		Sta(0)
	{
		_Id = TBrickParam::MA_HEAL;
	}

	CSBrickParamMagicHeal(const std::string&str)
	{
		*this=CSBrickParamMagicHeal();
		*this=str;
	}

	const CSBrickParamMagicHeal& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Hp);
		NLMISC::fromString(args[1], Sap);
		NLMISC::fromString(args[2], Sta);

		return *this;
	}
};


struct CSBrickParamMagicRanges : public TBrickParam::IId
{
	
	sint8 RangeIndex;

	CSBrickParamMagicRanges():
		RangeIndex()
	{
		_Id = TBrickParam::MA_RANGE;
	}

	CSBrickParamMagicRanges(const std::string&str)
	{
		*this=CSBrickParamMagicRanges();
		*this=str;
	}

	const CSBrickParamMagicRanges& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], RangeIndex);

		return *this;
	}
};


struct CSBrickParamMagicLinkCost : public TBrickParam::IId
{
	
	sint32 Cost;

	CSBrickParamMagicLinkCost():
		Cost()
	{
		_Id = TBrickParam::MA_LINK_COST;
	}

	CSBrickParamMagicLinkCost(const std::string&str)
	{
		*this=CSBrickParamMagicLinkCost();
		*this=str;
	}

	const CSBrickParamMagicLinkCost& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Cost);

		return *this;
	}
};


struct CSBrickParamMagicLinkPeriod : public TBrickParam::IId
{
	
	uint32 Period;

	CSBrickParamMagicLinkPeriod():
		Period()
	{
		_Id = TBrickParam::MA_LINK_PERIOD;
	}

	CSBrickParamMagicLinkPeriod(const std::string&str)
	{
		*this=CSBrickParamMagicLinkPeriod();
		*this=str;
	}

	const CSBrickParamMagicLinkPeriod& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Period);

		return *this;
	}
};


struct CSBrickParamMagicCure : public TBrickParam::IId
{
	
	std::string Cure;

	CSBrickParamMagicCure():
		Cure()
	{
		_Id = TBrickParam::MA_CURE;
	}

	CSBrickParamMagicCure(const std::string&str)
	{
		*this=CSBrickParamMagicCure();
		*this=str;
	}

	const CSBrickParamMagicCure& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Cure=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamMagicLinkPower : public TBrickParam::IId
{
	
	uint16 Power;

	CSBrickParamMagicLinkPower():
		Power()
	{
		_Id = TBrickParam::MA_LINK_POWER;
	}

	CSBrickParamMagicLinkPower(const std::string&str)
	{
		*this=CSBrickParamMagicLinkPower();
		*this=str;
	}

	const CSBrickParamMagicLinkPower& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Power);

		return *this;
	}
};


struct CSBrickParamMagicBreakResist : public TBrickParam::IId
{
	
	uint16 BreakResist;
	
	uint16 BreakResistPower;

	CSBrickParamMagicBreakResist():
		BreakResist(),
		BreakResistPower()
	{
		_Id = TBrickParam::MA_BREAK_RES;
	}

	CSBrickParamMagicBreakResist(const std::string&str)
	{
		*this=CSBrickParamMagicBreakResist();
		*this=str;
	}

	const CSBrickParamMagicBreakResist& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], BreakResist);
		NLMISC::fromString(args[1], BreakResistPower);

		return *this;
	}
};


struct CSBrickParamMagicArmorComp : public TBrickParam::IId
{
	
	uint16 ArmorComp;

	CSBrickParamMagicArmorComp():
		ArmorComp()
	{
		_Id = TBrickParam::MA_ARMOR_COMP;
	}

	CSBrickParamMagicArmorComp(const std::string&str)
	{
		*this=CSBrickParamMagicArmorComp();
		*this=str;
	}

	const CSBrickParamMagicArmorComp& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], ArmorComp);

		return *this;
	}
};


struct CSBrickParamMagicVampirise : public TBrickParam::IId
{
	
	sint32 Vampirise;

	CSBrickParamMagicVampirise():
		Vampirise()
	{
		_Id = TBrickParam::MA_VAMPIRISE;
	}

	CSBrickParamMagicVampirise(const std::string&str)
	{
		*this=CSBrickParamMagicVampirise();
		*this=str;
	}

	const CSBrickParamMagicVampirise& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Vampirise);

		return *this;
	}
};


struct CSBrickParamMagicVampiriseRatio : public TBrickParam::IId
{
	
	float VampiriseRatio;

	CSBrickParamMagicVampiriseRatio():
		VampiriseRatio()
	{
		_Id = TBrickParam::MA_VAMPIRISE_RATIO;
	}

	CSBrickParamMagicVampiriseRatio(const std::string&str)
	{
		*this=CSBrickParamMagicVampiriseRatio();
		*this=str;
	}

	const CSBrickParamMagicVampiriseRatio& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		VampiriseRatio=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftRecommended : public TBrickParam::IId
{
	
	uint32 Recommended;

	CSBrickParamCraftRecommended():
		Recommended()
	{
		_Id = TBrickParam::CR_RECOMMENDED;
	}

	CSBrickParamCraftRecommended(const std::string&str)
	{
		*this=CSBrickParamCraftRecommended();
		*this=str;
	}

	const CSBrickParamCraftRecommended& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Recommended);

		return *this;
	}
};


struct CSBrickParamCraftHP : public TBrickParam::IId
{
	
	sint32 HitPoint;

	CSBrickParamCraftHP():
		HitPoint()
	{
		_Id = TBrickParam::CR_HP;
	}

	CSBrickParamCraftHP(const std::string&str)
	{
		*this=CSBrickParamCraftHP();
		*this=str;
	}

	const CSBrickParamCraftHP& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], HitPoint);

		return *this;
	}
};


struct CSBrickParamCraftSap : public TBrickParam::IId
{
	
	sint32 Sap;

	CSBrickParamCraftSap():
		Sap()
	{
		_Id = TBrickParam::CR_SAP;
	}

	CSBrickParamCraftSap(const std::string&str)
	{
		*this=CSBrickParamCraftSap();
		*this=str;
	}

	const CSBrickParamCraftSap& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Sap);

		return *this;
	}
};


struct CSBrickParamCraftSta : public TBrickParam::IId
{
	
	sint32 Stamina;

	CSBrickParamCraftSta():
		Stamina()
	{
		_Id = TBrickParam::CR_STA;
	}

	CSBrickParamCraftSta(const std::string&str)
	{
		*this=CSBrickParamCraftSta();
		*this=str;
	}

	const CSBrickParamCraftSta& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Stamina);

		return *this;
	}
};


struct CSBrickParamCraftFocus : public TBrickParam::IId
{
	
	uint32 Focus;

	CSBrickParamCraftFocus():
		Focus()
	{
		_Id = TBrickParam::CR_FOCUS;
	}

	CSBrickParamCraftFocus(const std::string&str)
	{
		*this=CSBrickParamCraftFocus();
		*this=str;
	}

	const CSBrickParamCraftFocus& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Focus);

		return *this;
	}
};


struct CSBrickParamCraftQuality : public TBrickParam::IId
{
	
	sint32 Quality;

	CSBrickParamCraftQuality():
		Quality()
	{
		_Id = TBrickParam::CR_QUALITY;
	}

	CSBrickParamCraftQuality(const std::string&str)
	{
		*this=CSBrickParamCraftQuality();
		*this=str;
	}

	const CSBrickParamCraftQuality& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Quality);

		return *this;
	}
};


struct CSBrickParamCraftDurability : public TBrickParam::IId
{
	
	float Durability;

	CSBrickParamCraftDurability():
		Durability()
	{
		_Id = TBrickParam::CR_DURABILITY;
	}

	CSBrickParamCraftDurability(const std::string&str)
	{
		*this=CSBrickParamCraftDurability();
		*this=str;
	}

	const CSBrickParamCraftDurability& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Durability=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftDamage : public TBrickParam::IId
{
	
	float Damage;

	CSBrickParamCraftDamage():
		Damage()
	{
		_Id = TBrickParam::CR_DAMAGE;
	}

	CSBrickParamCraftDamage(const std::string&str)
	{
		*this=CSBrickParamCraftDamage();
		*this=str;
	}

	const CSBrickParamCraftDamage& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Damage=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftHitRate : public TBrickParam::IId
{
	
	float HitRate;

	CSBrickParamCraftHitRate():
		HitRate()
	{
		_Id = TBrickParam::CR_HITRATE;
	}

	CSBrickParamCraftHitRate(const std::string&str)
	{
		*this=CSBrickParamCraftHitRate();
		*this=str;
	}

	const CSBrickParamCraftHitRate& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		HitRate=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftRange : public TBrickParam::IId
{
	
	float Range;

	CSBrickParamCraftRange():
		Range()
	{
		_Id = TBrickParam::CR_RANGE;
	}

	CSBrickParamCraftRange(const std::string&str)
	{
		*this=CSBrickParamCraftRange();
		*this=str;
	}

	const CSBrickParamCraftRange& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Range=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftDmgProtection : public TBrickParam::IId
{
	
	float DmgProtection;

	CSBrickParamCraftDmgProtection():
		DmgProtection()
	{
		_Id = TBrickParam::CR_DMG_PROTECTION;
	}

	CSBrickParamCraftDmgProtection(const std::string&str)
	{
		*this=CSBrickParamCraftDmgProtection();
		*this=str;
	}

	const CSBrickParamCraftDmgProtection& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		DmgProtection=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftSapload : public TBrickParam::IId
{
	
	float Sapload;

	CSBrickParamCraftSapload():
		Sapload()
	{
		_Id = TBrickParam::CR_SAPLOAD;
	}

	CSBrickParamCraftSapload(const std::string&str)
	{
		*this=CSBrickParamCraftSapload();
		*this=str;
	}

	const CSBrickParamCraftSapload& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Sapload=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamCraftWeight : public TBrickParam::IId
{
	
	float Weight;

	CSBrickParamCraftWeight():
		Weight()
	{
		_Id = TBrickParam::CR_WEIGHT;
	}

	CSBrickParamCraftWeight(const std::string&str)
	{
		*this=CSBrickParamCraftWeight();
		*this=str;
	}

	const CSBrickParamCraftWeight& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Weight=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageRange : public TBrickParam::IId
{
	
	float Range;

	CSBrickParamForageRange():
		Range()
	{
		_Id = TBrickParam::FG_RANGE;
	}

	CSBrickParamForageRange(const std::string&str)
	{
		*this=CSBrickParamForageRange();
		*this=str;
	}

	const CSBrickParamForageRange& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Range=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageLocateDepositRange : public TBrickParam::IId
{
	
	float Range;

	CSBrickParamForageLocateDepositRange():
		Range()
	{
		_Id = TBrickParam::FG_LD_RANGE;
	}

	CSBrickParamForageLocateDepositRange(const std::string&str)
	{
		*this=CSBrickParamForageLocateDepositRange();
		*this=str;
	}

	const CSBrickParamForageLocateDepositRange& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Range=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageAngle : public TBrickParam::IId
{
	
	uint32 Angle;

	CSBrickParamForageAngle():
		Angle()
	{
		_Id = TBrickParam::FG_ANGLE;
	}

	CSBrickParamForageAngle(const std::string&str)
	{
		*this=CSBrickParamForageAngle();
		*this=str;
	}

	const CSBrickParamForageAngle& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Angle);

		return *this;
	}
};


struct CSBrickParamForageMulti : public TBrickParam::IId
{
	
	uint32 Limit;

	CSBrickParamForageMulti():
		Limit()
	{
		_Id = TBrickParam::FG_MULTI;
	}

	CSBrickParamForageMulti(const std::string&str)
	{
		*this=CSBrickParamForageMulti();
		*this=str;
	}

	const CSBrickParamForageMulti& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Limit);

		return *this;
	}
};


struct CSBrickParamForageKnowledge : public TBrickParam::IId
{
	
	uint8 Know;

	CSBrickParamForageKnowledge():
		Know()
	{
		_Id = TBrickParam::FG_KNOW;
	}

	CSBrickParamForageKnowledge(const std::string&str)
	{
		*this=CSBrickParamForageKnowledge();
		*this=str;
	}

	const CSBrickParamForageKnowledge& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Know);

		return *this;
	}
};


struct CSBrickParamForageTime : public TBrickParam::IId
{
	
	float Time;

	CSBrickParamForageTime():
		Time()
	{
		_Id = TBrickParam::FG_TIME;
	}

	CSBrickParamForageTime(const std::string&str)
	{
		*this=CSBrickParamForageTime();
		*this=str;
	}

	const CSBrickParamForageTime& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Time=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageSourceTime : public TBrickParam::IId
{
	
	float Time;

	CSBrickParamForageSourceTime():
		Time()
	{
		_Id = TBrickParam::FG_SRC_TIME;
	}

	CSBrickParamForageSourceTime(const std::string&str)
	{
		*this=CSBrickParamForageSourceTime();
		*this=str;
	}

	const CSBrickParamForageSourceTime& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Time=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageStatEnergy : public TBrickParam::IId
{
	
	float StatEnergy;

	CSBrickParamForageStatEnergy():
		StatEnergy()
	{
		_Id = TBrickParam::FG_STAT_ENERGY;
	}

	CSBrickParamForageStatEnergy(const std::string&str)
	{
		*this=CSBrickParamForageStatEnergy();
		*this=str;
	}

	const CSBrickParamForageStatEnergy& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		StatEnergy=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamStatEnergyOnly : public TBrickParam::IId
{
	
	uint8 StatEnergyExact;

	CSBrickParamStatEnergyOnly():
		StatEnergyExact()
	{
		_Id = TBrickParam::FG_STAT_ENERGY_ONLY;
	}

	CSBrickParamStatEnergyOnly(const std::string&str)
	{
		*this=CSBrickParamStatEnergyOnly();
		*this=str;
	}

	const CSBrickParamStatEnergyOnly& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], StatEnergyExact);

		return *this;
	}
};


struct CSBrickParamForageVisDist : public TBrickParam::IId
{
	
	float Dist;

	CSBrickParamForageVisDist():
		Dist()
	{
		_Id = TBrickParam::FG_VIS_DIST;
	}

	CSBrickParamForageVisDist(const std::string&str)
	{
		*this=CSBrickParamForageVisDist();
		*this=str;
	}

	const CSBrickParamForageVisDist& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Dist=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageVisStealth : public TBrickParam::IId
{
	
	uint8 Mode;

	CSBrickParamForageVisStealth():
		Mode()
	{
		_Id = TBrickParam::FG_VIS_STEALTH;
	}

	CSBrickParamForageVisStealth(const std::string&str)
	{
		*this=CSBrickParamForageVisStealth();
		*this=str;
	}

	const CSBrickParamForageVisStealth& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Mode);

		return *this;
	}
};


struct CSBrickParamForageSourceLocator : public TBrickParam::IId
{
	
	uint8 Flag;

	CSBrickParamForageSourceLocator():
		Flag()
	{
		_Id = TBrickParam::FG_SRC_LOCATOR;
	}

	CSBrickParamForageSourceLocator(const std::string&str)
	{
		*this=CSBrickParamForageSourceLocator();
		*this=str;
	}

	const CSBrickParamForageSourceLocator& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Flag);

		return *this;
	}
};


struct CSBrickParamForageAttempts : public TBrickParam::IId
{
	
	uint16 Nb;

	CSBrickParamForageAttempts():
		Nb()
	{
		_Id = TBrickParam::FG_ATTEMPTS;
	}

	CSBrickParamForageAttempts(const std::string&str)
	{
		*this=CSBrickParamForageAttempts();
		*this=str;
	}

	const CSBrickParamForageAttempts& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Nb);

		return *this;
	}
};


struct CSBrickParamForageAbsorptionS : public TBrickParam::IId
{
	
	float Absorption;

	CSBrickParamForageAbsorptionS():
		Absorption()
	{
		_Id = TBrickParam::FG_ABS_S;
	}

	CSBrickParamForageAbsorptionS(const std::string&str)
	{
		*this=CSBrickParamForageAbsorptionS();
		*this=str;
	}

	const CSBrickParamForageAbsorptionS& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Absorption=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageAbsorptionA : public TBrickParam::IId
{
	
	float Absorption;

	CSBrickParamForageAbsorptionA():
		Absorption()
	{
		_Id = TBrickParam::FG_ABS_A;
	}

	CSBrickParamForageAbsorptionA(const std::string&str)
	{
		*this=CSBrickParamForageAbsorptionA();
		*this=str;
	}

	const CSBrickParamForageAbsorptionA& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Absorption=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageAbsorptionQ : public TBrickParam::IId
{
	
	float Absorption;

	CSBrickParamForageAbsorptionQ():
		Absorption()
	{
		_Id = TBrickParam::FG_ABS_Q;
	}

	CSBrickParamForageAbsorptionQ(const std::string&str)
	{
		*this=CSBrickParamForageAbsorptionQ();
		*this=str;
	}

	const CSBrickParamForageAbsorptionQ& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Absorption=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForagePeriod : public TBrickParam::IId
{
	
	float Period;

	CSBrickParamForagePeriod():
		Period()
	{
		_Id = TBrickParam::FG_SRC_PRD;
	}

	CSBrickParamForagePeriod(const std::string&str)
	{
		*this=CSBrickParamForagePeriod();
		*this=str;
	}

	const CSBrickParamForagePeriod& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Period=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageAperture : public TBrickParam::IId
{
	
	float Aperture;

	CSBrickParamForageAperture():
		Aperture()
	{
		_Id = TBrickParam::FG_SRC_APT;
	}

	CSBrickParamForageAperture(const std::string&str)
	{
		*this=CSBrickParamForageAperture();
		*this=str;
	}

	const CSBrickParamForageAperture& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Aperture=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageQuality : public TBrickParam::IId
{
	
	float Quality;

	CSBrickParamForageQuality():
		Quality()
	{
		_Id = TBrickParam::FG_QUALITY;
	}

	CSBrickParamForageQuality(const std::string&str)
	{
		*this=CSBrickParamForageQuality();
		*this=str;
	}

	const CSBrickParamForageQuality& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Quality=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForagePreservation : public TBrickParam::IId
{
	
	float Pres;

	CSBrickParamForagePreservation():
		Pres()
	{
		_Id = TBrickParam::FG_PRES;
	}

	CSBrickParamForagePreservation(const std::string&str)
	{
		*this=CSBrickParamForagePreservation();
		*this=str;
	}

	const CSBrickParamForagePreservation& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Pres=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageStability : public TBrickParam::IId
{
	
	float Stab;

	CSBrickParamForageStability():
		Stab()
	{
		_Id = TBrickParam::FG_STAB;
	}

	CSBrickParamForageStability(const std::string&str)
	{
		*this=CSBrickParamForageStability();
		*this=str;
	}

	const CSBrickParamForageStability& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Stab=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageCreatureStealth : public TBrickParam::IId
{
	
	float Stealth;

	CSBrickParamForageCreatureStealth():
		Stealth()
	{
		_Id = TBrickParam::FG_CR_STEALTH;
	}

	CSBrickParamForageCreatureStealth(const std::string&str)
	{
		*this=CSBrickParamForageCreatureStealth();
		*this=str;
	}

	const CSBrickParamForageCreatureStealth& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Stealth=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageAbsorbSourceDmg : public TBrickParam::IId
{
	
	uint8 Percent;

	CSBrickParamForageAbsorbSourceDmg():
		Percent()
	{
		_Id = TBrickParam::FG_ABS_SRC_DMG;
	}

	CSBrickParamForageAbsorbSourceDmg(const std::string&str)
	{
		*this=CSBrickParamForageAbsorbSourceDmg();
		*this=str;
	}

	const CSBrickParamForageAbsorbSourceDmg& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Percent);

		return *this;
	}
};


struct CSBrickParamKamiOffering : public TBrickParam::IId
{
	
	uint32 Num;

	CSBrickParamKamiOffering():
		Num()
	{
		_Id = TBrickParam::KAMI_OFFERING;
	}

	CSBrickParamKamiOffering(const std::string&str)
	{
		*this=CSBrickParamKamiOffering();
		*this=str;
	}

	const CSBrickParamKamiOffering& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Num);

		return *this;
	}
};


struct CSBrickParamKamiAngerDecrease : public TBrickParam::IId
{
	
	float Delta;

	CSBrickParamKamiAngerDecrease():
		Delta()
	{
		_Id = TBrickParam::KAMI_ANGER_DECREASE;
	}

	CSBrickParamKamiAngerDecrease(const std::string&str)
	{
		*this=CSBrickParamKamiAngerDecrease();
		*this=str;
	}

	const CSBrickParamKamiAngerDecrease& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Delta=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageReduceDamage : public TBrickParam::IId
{
	
	float Ratio;

	CSBrickParamForageReduceDamage():
		Ratio()
	{
		_Id = TBrickParam::FG_REDUCE_DMG;
	}

	CSBrickParamForageReduceDamage(const std::string&str)
	{
		*this=CSBrickParamForageReduceDamage();
		*this=str;
	}

	const CSBrickParamForageReduceDamage& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Ratio=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamForageEcotypeSpec : public TBrickParam::IId
{
	
	std::string Ecotype;

	CSBrickParamForageEcotypeSpec():
		Ecotype()
	{
		_Id = TBrickParam::FG_ECT_SPC;
	}

	CSBrickParamForageEcotypeSpec(const std::string&str)
	{
		*this=CSBrickParamForageEcotypeSpec();
		*this=str;
	}

	const CSBrickParamForageEcotypeSpec& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Ecotype=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamForageRMGroupFilter : public TBrickParam::IId
{
	
	uint32 Value;

	CSBrickParamForageRMGroupFilter():
		Value()
	{
		_Id = TBrickParam::FG_RMGRP_FILT;
	}

	CSBrickParamForageRMGroupFilter(const std::string&str)
	{
		*this=CSBrickParamForageRMGroupFilter();
		*this=str;
	}

	const CSBrickParamForageRMGroupFilter& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Value);

		return *this;
	}
};


struct CSBrickParamForageRMFamilyFilter : public TBrickParam::IId
{
	
	uint32 Value;

	CSBrickParamForageRMFamilyFilter():
		Value()
	{
		_Id = TBrickParam::FG_RMFAM_FILT;
	}

	CSBrickParamForageRMFamilyFilter(const std::string&str)
	{
		*this=CSBrickParamForageRMFamilyFilter();
		*this=str;
	}

	const CSBrickParamForageRMFamilyFilter& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Value);

		return *this;
	}
};


struct CSBrickParamForageItemPartFilter : public TBrickParam::IId
{
	
	uint32 ItemPartIndex;

	CSBrickParamForageItemPartFilter():
		ItemPartIndex()
	{
		_Id = TBrickParam::FG_ITEMPART_FILT;
	}

	CSBrickParamForageItemPartFilter(const std::string&str)
	{
		*this=CSBrickParamForageItemPartFilter();
		*this=str;
	}

	const CSBrickParamForageItemPartFilter& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], ItemPartIndex);

		return *this;
	}
};


struct CSBrickParamPowerTaunt : public TBrickParam::IId
{
	// entities of higher level cannot be taunt
	uint16 TauntPower;
	// effective range in meters
	float Range;
	// disable taunt powers for x seconds
	float DisableTime;

	CSBrickParamPowerTaunt():
		TauntPower(),
		Range(),
		DisableTime()
	{
		_Id = TBrickParam::SP_TAUNT;
	}

	CSBrickParamPowerTaunt(const std::string&str)
	{
		*this=CSBrickParamPowerTaunt();
		*this=str;
	}

	const CSBrickParamPowerTaunt& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], TauntPower);
		Range=(float)atof(args[1].c_str());
		DisableTime=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamShielding : public TBrickParam::IId
{
	// granted protection in % without a shield
	uint8 NoShieldProtectionFactor;
	// max protection without a shield
	uint16 NoShieldProtectionMax;
	// granted protection in % with a buckler
	uint8 BucklerProtectionFactor;
	// max protection with a buckler
	uint16 BucklerProtectionMax;
	// granted protection in % with a shield
	uint8 ShieldProtectionFactor;
	// max protection with a shield
	uint16 ShieldProtectionMax;
	// power duration
	float Duration;
	// disable power for x seconds	
	float DisableTime;

	CSBrickParamShielding():
		NoShieldProtectionFactor(),
		NoShieldProtectionMax(),
		BucklerProtectionFactor(),
		BucklerProtectionMax(),
		ShieldProtectionFactor(),
		ShieldProtectionMax(),
		Duration(),
		DisableTime()
	{
		_Id = TBrickParam::SP_SHIELDING;
	}

	CSBrickParamShielding(const std::string&str)
	{
		*this=CSBrickParamShielding();
		*this=str;
	}

	const CSBrickParamShielding& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=8)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], NoShieldProtectionFactor);
		NLMISC::fromString(args[1], NoShieldProtectionMax);
		NLMISC::fromString(args[2], BucklerProtectionFactor);
		NLMISC::fromString(args[3], BucklerProtectionMax);
		NLMISC::fromString(args[4], ShieldProtectionFactor);
		NLMISC::fromString(args[5], ShieldProtectionMax);
		Duration=(float)atof(args[6].c_str());
		DisableTime=(float)atof(args[7].c_str());

		return *this;
	}
};


struct CSBrickParamLifeAura : public TBrickParam::IId
{
	// regen modifier (in %)
	uint16 RegenMod;
	// duration in seconds
	float Duration;
	// aura radius in meters
	float Radius;
	// disable life aura for x seconds on targets
	float TargetDisableTime;
	// disable life aura for x seconds on user
	float UserDisableTime;

	CSBrickParamLifeAura():
		RegenMod(),
		Duration(),
		Radius(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_LIFE_AURA;
	}

	CSBrickParamLifeAura(const std::string&str)
	{
		*this=CSBrickParamLifeAura();
		*this=str;
	}

	const CSBrickParamLifeAura& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], RegenMod);
		Duration=(float)atof(args[1].c_str());
		Radius=(float)atof(args[2].c_str());
		TargetDisableTime=(float)atof(args[3].c_str());
		UserDisableTime=(float)atof(args[4].c_str());

		return *this;
	}
};


struct CSBrickParamLifeAura2 : public TBrickParam::IId
{
	// regen modifier (in %)
	uint16 RegenMod;
	// duration in seconds
	float Duration;
	// aura radius in meters
	float Radius;
	// disable life aura for x seconds on targets
	float TargetDisableTime;
	// disable life aura for x seconds on user
	float UserDisableTime;

	CSBrickParamLifeAura2():
		RegenMod(),
		Duration(),
		Radius(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_LIFE_AURA2;
	}

	CSBrickParamLifeAura2(const std::string&str)
	{
		*this=CSBrickParamLifeAura2();
		*this=str;
	}

	const CSBrickParamLifeAura2& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		RegenMod=atoi(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		Radius=(float)atof(args[2].c_str());
		TargetDisableTime=(float)atof(args[3].c_str());
		UserDisableTime=(float)atof(args[4].c_str());

		return *this;
	}
};

struct CSBrickParamStaminaAura : public TBrickParam::IId
{
	// regen modifier (in %)
	uint16 RegenMod;
	// duration in seconds
	float Duration;
	// aura radius in meters
	float Radius;
	// disable life aura for x seconds on targets
	float TargetDisableTime;
	// disable life aura for x seconds on user
	float UserDisableTime;

	CSBrickParamStaminaAura():
		RegenMod(),
		Duration(),
		Radius(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_STAMINA_AURA;
	}

	CSBrickParamStaminaAura(const std::string&str)
	{
		*this=CSBrickParamStaminaAura();
		*this=str;
	}

	const CSBrickParamStaminaAura& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], RegenMod);
		Duration=(float)atof(args[1].c_str());
		Radius=(float)atof(args[2].c_str());
		TargetDisableTime=(float)atof(args[3].c_str());
		UserDisableTime=(float)atof(args[4].c_str());

		return *this;
	}
};

struct CSBrickParamStaminaAura2 : public TBrickParam::IId
{
	// regen modifier (in %)
	uint16 RegenMod;
	// duration in seconds
	float Duration;
	// aura radius in meters
	float Radius;
	// disable life aura for x seconds on targets
	float TargetDisableTime;
	// disable life aura for x seconds on user
	float UserDisableTime;

	CSBrickParamStaminaAura2():
		RegenMod(),
		Duration(),
		Radius(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_STAMINA_AURA2;
	}

	CSBrickParamStaminaAura2(const std::string&str)
	{
		*this=CSBrickParamStaminaAura2();
		*this=str;
	}

	const CSBrickParamStaminaAura2& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		RegenMod=atoi(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		Radius=(float)atof(args[2].c_str());
		TargetDisableTime=(float)atof(args[3].c_str());
		UserDisableTime=(float)atof(args[4].c_str());

		return *this;
	}
};
struct CSBrickParamSapAura : public TBrickParam::IId
{
	// regen modifier (in %)
	uint16 RegenMod;
	// duration in seconds
	float Duration;
	// aura radius in meters
	float Radius;
	// disable life aura for x seconds on targets
	float TargetDisableTime;
	// disable life aura for x seconds on user
	float UserDisableTime;

	CSBrickParamSapAura():
		RegenMod(),
		Duration(),
		Radius(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_SAP_AURA;
	}

	CSBrickParamSapAura(const std::string&str)
	{
		*this=CSBrickParamSapAura();
		*this=str;
	}

	const CSBrickParamSapAura& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], RegenMod);
		Duration=(float)atof(args[1].c_str());
		Radius=(float)atof(args[2].c_str());
		TargetDisableTime=(float)atof(args[3].c_str());
		UserDisableTime=(float)atof(args[4].c_str());

		return *this;
	}
};

struct CSBrickParamSapAura2 : public TBrickParam::IId
{
	// regen modifier (in %)
	uint16 RegenMod;
	// duration in seconds
	float Duration;
	// aura radius in meters
	float Radius;
	// disable life aura for x seconds on targets
	float TargetDisableTime;
	// disable life aura for x seconds on user
	float UserDisableTime;

	CSBrickParamSapAura2():
		RegenMod(),
		Duration(),
		Radius(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_SAP_AURA2;
	}

	CSBrickParamSapAura2(const std::string&str)
	{
		*this=CSBrickParamSapAura2();
		*this=str;
	}

	const CSBrickParamSapAura2& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		RegenMod=atoi(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		Radius=(float)atof(args[2].c_str());
		TargetDisableTime=(float)atof(args[3].c_str());
		UserDisableTime=(float)atof(args[4].c_str());

		return *this;
	}
};
struct CSBrickParamSpeedingUp : public TBrickParam::IId
{
	// speed modifier (in %)
	uint16 SpeedMod;
	// duration in seconds
	float Duration;
	// disable power for x seconds	
	float DisableTime;

	CSBrickParamSpeedingUp():
		SpeedMod(),
		Duration(),
		DisableTime()
	{
		_Id = TBrickParam::SP_SPEEDING_UP;
	}

	CSBrickParamSpeedingUp(const std::string&str)
	{
		*this=CSBrickParamSpeedingUp();
		*this=str;
	}

	const CSBrickParamSpeedingUp& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], SpeedMod);
		Duration=(float)atof(args[1].c_str());
		DisableTime=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamInvulnerability : public TBrickParam::IId
{
	// duration in seconds
	float Duration;
	// disable power for x seconds	
	float DisableTime;

	CSBrickParamInvulnerability():
		Duration(),
		DisableTime()
	{
		_Id = TBrickParam::SP_INVULNERABILITY;
	}

	CSBrickParamInvulnerability(const std::string&str)
	{
		*this=CSBrickParamInvulnerability();
		*this=str;
	}

	const CSBrickParamInvulnerability& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		Duration=(float)atof(args[0].c_str());
		DisableTime=(float)atof(args[1].c_str());

		return *this;
	}
};


struct CSBrickParamMeleeProtection : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;

	CSBrickParamMeleeProtection():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_MELEE_PROTECTION_AURA;
	}

	CSBrickParamMeleeProtection(const std::string&str)
	{
		*this=CSBrickParamMeleeProtection();
		*this=str;
	}

	const CSBrickParamMeleeProtection& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=4)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamRangeProtection : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;

	CSBrickParamRangeProtection():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_RANGE_PROTECTION_AURA;
	}

	CSBrickParamRangeProtection(const std::string&str)
	{
		*this=CSBrickParamRangeProtection();
		*this=str;
	}

	const CSBrickParamRangeProtection& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=4)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamMagicProtection : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;

	CSBrickParamMagicProtection():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime()
	{
		_Id = TBrickParam::SP_MAGIC_PROTECTION_AURA;
	}

	CSBrickParamMagicProtection(const std::string&str)
	{
		*this=CSBrickParamMagicProtection();
		*this=str;
	}

	const CSBrickParamMagicProtection& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=4)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamWarCry : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;
	// damage bonus (20 = +20%)
	sint16 DamageBonus;

	CSBrickParamWarCry():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime(),
		DamageBonus()
	{
		_Id = TBrickParam::SP_WAR_CRY_AURA;
	}

	CSBrickParamWarCry(const std::string&str)
	{
		*this=CSBrickParamWarCry();
		*this=str;
	}

	const CSBrickParamWarCry& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], DamageBonus);

		return *this;
	}
};


struct CSBrickParamFireWall : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;
	// damage 
	sint16 Damage;

	CSBrickParamFireWall():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime(),
		Damage()
	{
		_Id = TBrickParam::SP_FIRE_WALL_AURA;
	}

	CSBrickParamFireWall(const std::string&str)
	{
		*this=CSBrickParamFireWall();
		*this=str;
	}

	const CSBrickParamFireWall& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], Damage);

		return *this;
	}
};


struct CSBrickParamThornWall : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;
	// damage 
	sint16 Damage;

	CSBrickParamThornWall():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime(),
		Damage()
	{
		_Id = TBrickParam::SP_THORN_WALL_AURA;
	}

	CSBrickParamThornWall(const std::string&str)
	{
		*this=CSBrickParamThornWall();
		*this=str;
	}

	const CSBrickParamThornWall& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], Damage);

		return *this;
	}
};


struct CSBrickParamWaterWall : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;
	// damage 
	sint16 Damage;

	CSBrickParamWaterWall():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime(),
		Damage()
	{
		_Id = TBrickParam::SP_WATER_WALL_AURA;
	}

	CSBrickParamWaterWall(const std::string&str)
	{
		*this=CSBrickParamWaterWall();
		*this=str;
	}

	const CSBrickParamWaterWall& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], Damage);

		return *this;
	}
};


struct CSBrickParamLightningWall : public TBrickParam::IId
{
	// aura radius in meters
	float Radius;
	// duration in seconds
	float Duration;
	// disable aura for x seconds on targets
	float TargetDisableTime;
	// disable aura for x seconds on user
	float UserDisableTime;
	// damage 
	sint16 Damage;

	CSBrickParamLightningWall():
		Radius(),
		Duration(),
		TargetDisableTime(),
		UserDisableTime(),
		Damage()
	{
		_Id = TBrickParam::SP_LIGHTNING_WALL_AURA;
	}

	CSBrickParamLightningWall(const std::string&str)
	{
		*this=CSBrickParamLightningWall();
		*this=str;
	}

	const CSBrickParamLightningWall& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		Radius=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		TargetDisableTime=(float)atof(args[2].c_str());
		UserDisableTime=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], Damage);

		return *this;
	}
};


struct CSBrickParamBerserk : public TBrickParam::IId
{
	// disable berserker power for x seconds
	float DisableTime;
	// duration in seconds
	float Duration;
	// DoT damage suffered by user, damage per update
	float DamagePerUpdate;
	// DoT update frequency in seconds
	float UpdateFrequency;
	// damage bonus (20 = +20 damage points before success factor)
	uint16 DamageBonus;

	CSBrickParamBerserk():
		DisableTime(),
		Duration(),
		DamagePerUpdate(),
		UpdateFrequency(),
		DamageBonus()
	{
		_Id = TBrickParam::SP_BERSERK;
	}

	CSBrickParamBerserk(const std::string&str)
	{
		*this=CSBrickParamBerserk();
		*this=str;
	}

	const CSBrickParamBerserk& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		DisableTime=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		DamagePerUpdate=(float)atof(args[2].c_str());
		UpdateFrequency=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], DamageBonus);

		return *this;
	}
};


struct CSBrickParamEnchantWeapon : public TBrickParam::IId
{
	// disable berserker power for x seconds
	float DisableTime;
	// duration in seconds
	float Duration;
	// damage type
	std::string DamageType;
	// DoT update frequency in seconds
	float DpsBonus;
	// damage bonus (20 = +20 damage points before success factor)
	uint16 DamageBonus;

	CSBrickParamEnchantWeapon():
		DisableTime(),
		Duration(),
		DamageType(),
		DpsBonus(),
		DamageBonus()
	{
		_Id = TBrickParam::SP_ENCHANT_WEAPON;
	}

	CSBrickParamEnchantWeapon(const std::string&str)
	{
		*this=CSBrickParamEnchantWeapon();
		*this=str;
	}

	const CSBrickParamEnchantWeapon& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		DisableTime=(float)atof(args[0].c_str());
		Duration=(float)atof(args[1].c_str());
		DamageType=args[2].c_str();
		DpsBonus=(float)atof(args[3].c_str());
		NLMISC::fromString(args[4], DamageBonus);

		return *this;
	}
};


struct CSBrickParamCalmAnimal : public TBrickParam::IId
{
	// fauna type restriction (quadruped, land kittin...)
	std::string TypeRestriction;
	// aura radius in meters
	float Radius;
	// duration in seconds
	float DisableTime;

	CSBrickParamCalmAnimal():
		TypeRestriction(),
		Radius(),
		DisableTime()
	{
		_Id = TBrickParam::SP_CALM_ANIMAL;
	}

	CSBrickParamCalmAnimal(const std::string&str)
	{
		*this=CSBrickParamCalmAnimal();
		*this=str;
	}

	const CSBrickParamCalmAnimal& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		TypeRestriction=args[0].c_str();
		Radius=(float)atof(args[1].c_str());
		DisableTime=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamNeededBrickFlag : public TBrickParam::IId
{
	
	std::string Flag;

	CSBrickParamNeededBrickFlag():
		Flag()
	{
		_Id = TBrickParam::NEEDED_BRICK_FLAG;
	}

	CSBrickParamNeededBrickFlag(const std::string&str)
	{
		*this=CSBrickParamNeededBrickFlag();
		*this=str;
	}

	const CSBrickParamNeededBrickFlag& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Flag=args[0].c_str();

		return *this;
	}
};


struct CSBrickParamBalance : public TBrickParam::IId
{
	// disable power for x seconds
	float DisableTime;
	// affected score 
	std::string AffectedScore;
	// power range
	float Range;
	// score loss factor in %
	float LossFactor;

	CSBrickParamBalance():
		DisableTime(),
		AffectedScore(),
		Range(),
		LossFactor()
	{
		_Id = TBrickParam::SP_BALANCE;
	}

	CSBrickParamBalance(const std::string&str)
	{
		*this=CSBrickParamBalance();
		*this=str;
	}

	const CSBrickParamBalance& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=4)
			return *this;

		ParsedOk=true;
		DisableTime=(float)atof(args[0].c_str());
		AffectedScore=args[1].c_str();
		Range=(float)atof(args[2].c_str());
		LossFactor=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamHeal : public TBrickParam::IId
{
	// affected score 
	std::string AffectedScore;
	// value added to affected score
	sint32 HealValue;
	// value added to affected score in % of max target score
	float HealFactorValue;
	// disable power for x seconds
	float DisableTime;
	// type of power (Heal, HealHpC ...)
	std::string PowerType;

	CSBrickParamHeal():
		AffectedScore(),
		HealValue(),
		HealFactorValue(),
		DisableTime(),
		PowerType()
	{
		_Id = TBrickParam::SP_HEAL;
	}

	CSBrickParamHeal(const std::string&str)
	{
		*this=CSBrickParamHeal();
		*this=str;
	}

	const CSBrickParamHeal& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=5)
			return *this;

		ParsedOk=true;
		AffectedScore=args[0].c_str();
		NLMISC::fromString(args[1], HealValue);
		HealFactorValue=(float)atof(args[2].c_str());
		DisableTime=(float)atof(args[3].c_str());
		PowerType=args[4].c_str();

		return *this;
	}
};


struct CSBrickParamRecastTime : public TBrickParam::IId
{
	// disable power for x seconds
	float Time;

	CSBrickParamRecastTime():
		Time()
	{
		_Id = TBrickParam::SP_RECAST_TIME;
	}

	CSBrickParamRecastTime(const std::string&str)
	{
		*this=CSBrickParamRecastTime();
		*this=str;
	}

	const CSBrickParamRecastTime& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Time=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamBonusFgExtractionTimeGC : public TBrickParam::IId
{
	
	float AdditionalTimeGC;

	CSBrickParamBonusFgExtractionTimeGC():
		AdditionalTimeGC()
	{
		_Id = TBrickParam::BONUS_FG_EXTRACTION_TIME_GC;
	}

	CSBrickParamBonusFgExtractionTimeGC(const std::string&str)
	{
		*this=CSBrickParamBonusFgExtractionTimeGC();
		*this=str;
	}

	const CSBrickParamBonusFgExtractionTimeGC& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		AdditionalTimeGC=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamBonusCrDurability : public TBrickParam::IId
{
	
	float Bonus;

	CSBrickParamBonusCrDurability():
		Bonus()
	{
		_Id = TBrickParam::BONUS_CR_DURABILITY;
	}

	CSBrickParamBonusCrDurability(const std::string&str)
	{
		*this=CSBrickParamBonusCrDurability();
		*this=str;
	}

	const CSBrickParamBonusCrDurability& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Bonus=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamBonusLandmarkNumber : public TBrickParam::IId
{
	
	float Nb;

	CSBrickParamBonusLandmarkNumber():
		Nb()
	{
		_Id = TBrickParam::BONUS_LANDMARK_NUMBER;
	}

	CSBrickParamBonusLandmarkNumber(const std::string&str)
	{
		*this=CSBrickParamBonusLandmarkNumber();
		*this=str;
	}

	const CSBrickParamBonusLandmarkNumber& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		Nb=(float)atof(args[0].c_str());

		return *this;
	}
};


struct CSBrickParamAreaBomb : public TBrickParam::IId
{
	// Radius
	float Radius;
	// MinFactor when we are at extreme range
	float MinFactor;
	uint8 MaxTarget;

	CSBrickParamAreaBomb():
		Radius(),
		MinFactor()
	{
		_Id = TBrickParam::AREA_BOMB;
	}

	CSBrickParamAreaBomb(const std::string&str)
	{
		*this=CSBrickParamAreaBomb();
		*this=str;
	}

	const CSBrickParamAreaBomb& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Radius);
		NLMISC::fromString(args[1], MinFactor);
		NLMISC::fromString(args[2], MaxTarget);

		return *this;
	}
};


struct CSBrickParamAreaSpray : public TBrickParam::IId
{
	// angle in degree
	uint8 Angle;
	// height of the trapezoid
	float Height;
	// little base length
	float Base;
	// max target
	uint8 MaxTarget;

	CSBrickParamAreaSpray():
		Angle(),
		Height(),
		Base(),
		MaxTarget()
	{
		_Id = TBrickParam::AREA_SPRAY;
	}

	CSBrickParamAreaSpray(const std::string&str)
	{
		*this=CSBrickParamAreaSpray();
		*this=str;
	}

	const CSBrickParamAreaSpray& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=4)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], Angle);
		Height=(float)atof(args[1].c_str());
		Base=(float)atof(args[2].c_str());
		NLMISC::fromString(args[3], MaxTarget);

		return *this;
	}
};


struct CSBrickParamAreaChain : public TBrickParam::IId
{
	// range between 2 targets
	float Range;
	// max nb targets
	uint8 MaxTargets;
	// damage factor
	float Factor;

	CSBrickParamAreaChain():
		Range(),
		MaxTargets(),
		Factor()
	{
		_Id = TBrickParam::AREA_CHAIN;
	}

	CSBrickParamAreaChain(const std::string&str)
	{
		*this=CSBrickParamAreaChain();
		*this=str;
	}

	const CSBrickParamAreaChain& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=3)
			return *this;

		ParsedOk=true;
		Range=(float)atof(args[0].c_str());
		NLMISC::fromString(args[1], MaxTargets);
		Factor=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamAreaTargets : public TBrickParam::IId
{
	// each target count as 'TargetFactor' for damage or heal division among targets
	float TargetFactor;
	// max nb targets
	uint8 MaxTargets;

	CSBrickParamAreaTargets():
		TargetFactor(),
		MaxTargets()
	{
		_Id = TBrickParam::AREA_TARGETS;
	}

	CSBrickParamAreaTargets(const std::string&str)
	{
		*this=CSBrickParamAreaTargets();
		*this=str;
	}

	const CSBrickParamAreaTargets& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		TargetFactor=(float)atof(args[0].c_str());
		NLMISC::fromString(args[1], MaxTargets);

		return *this;
	}
};


struct CSBrickParamMagicRecharge : public TBrickParam::IId
{
	// sap load
	uint32 SapLoad;

	CSBrickParamMagicRecharge():
		SapLoad()
	{
		_Id = TBrickParam::MA_RECHARGE;
	}

	CSBrickParamMagicRecharge(const std::string&str)
	{
		*this=CSBrickParamMagicRecharge();
		*this=str;
	}

	const CSBrickParamMagicRecharge& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=1)
			return *this;

		ParsedOk=true;
		NLMISC::fromString(args[0], SapLoad);

		return *this;
	}
};


struct CSBrickParamCharacUpgrade : public TBrickParam::IId
{
	// affected characteristic
	std::string Characteristic;
	// bonus on charac
	uint32 Modifier;

	CSBrickParamCharacUpgrade():
		Characteristic(),
		Modifier()
	{
		_Id = TBrickParam::CHARAC_UPGRADE;
	}

	CSBrickParamCharacUpgrade(const std::string&str)
	{
		*this=CSBrickParamCharacUpgrade();
		*this=str;
	}

	const CSBrickParamCharacUpgrade& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		Characteristic=args[0].c_str();
		NLMISC::fromString(args[1], Modifier);

		return *this;
	}
};


struct CSBrickParamScoreUpgrade : public TBrickParam::IId
{
	// affected score
	std::string Score;
	// modifier on score
	sint32 Modifier;

	CSBrickParamScoreUpgrade():
		Score(),
		Modifier()
	{
		_Id = TBrickParam::SCORE_UPGRADE;
	}

	CSBrickParamScoreUpgrade(const std::string&str)
	{
		*this=CSBrickParamScoreUpgrade();
		*this=str;
	}

	const CSBrickParamScoreUpgrade& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=2)
			return *this;

		ParsedOk=true;
		Score=args[0].c_str();
		NLMISC::fromString(args[1], Modifier);

		return *this;
	}
};


struct CSBrickParamTeleport : public TBrickParam::IId
{

	CSBrickParamTeleport()
	{
		_Id = TBrickParam::TA_TELEPORT;
	}

	CSBrickParamTeleport(const std::string&str)
	{
		*this=CSBrickParamTeleport();
		*this=str;
	}

	const CSBrickParamTeleport& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamDisconnect : public TBrickParam::IId
{

	CSBrickParamDisconnect()
	{
		_Id = TBrickParam::TA_DISCONNECT;
	}

	CSBrickParamDisconnect(const std::string&str)
	{
		*this=CSBrickParamDisconnect();
		*this=str;
	}

	const CSBrickParamDisconnect& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamMount : public TBrickParam::IId
{

	CSBrickParamMount()
	{
		_Id = TBrickParam::TA_MOUNT;
	}

	CSBrickParamMount(const std::string&str)
	{
		*this=CSBrickParamMount();
		*this=str;
	}

	const CSBrickParamMount& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamUnmount : public TBrickParam::IId
{

	CSBrickParamUnmount()
	{
		_Id = TBrickParam::TA_UNMOUNT;
	}

	CSBrickParamUnmount(const std::string&str)
	{
		*this=CSBrickParamUnmount();
		*this=str;
	}

	const CSBrickParamUnmount& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamConsumeItem : public TBrickParam::IId
{

	CSBrickParamConsumeItem()
	{
		_Id = TBrickParam::TA_CONSUME;
	}

	CSBrickParamConsumeItem(const std::string&str)
	{
		*this=CSBrickParamConsumeItem();
		*this=str;
	}

	const CSBrickParamConsumeItem& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);

		if (args.size()!=0)
			return *this;

		ParsedOk=true;

		return *this;
	}
};


struct CSBrickParamChgCharac : public TBrickParam::IId
{
	// affected characteristic 
	std::string AffectedCharac;
	// coefficient to modify characteristic proportionally to item level
	float ModifierCoefficient;
	// value added to affected characteristic
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamChgCharac():
		AffectedCharac(),
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_CHG_CHARAC;
	}
	
	CSBrickParamChgCharac(const std::string&str)
	{
		*this=CSBrickParamChgCharac();
		*this=str;
	}
	
	const CSBrickParamChgCharac& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()!=4 && args.size()!=5)
			return *this;
		
		ParsedOk=true;
		AffectedCharac=args[0].c_str();
		ModifierCoefficient=(float)atof(args[1].c_str());
		ModifierConstant=(float)atof(args[2].c_str());
		Duration=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamModDefense : public TBrickParam::IId
{
	// Dodge or Parry ?
	std::string DefenseMode;
	// coefficient to modify defense success chance proportionally to item level
	float ModifierCoefficient;
	// value added to defense success chance
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamModDefense():
		DefenseMode(),
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_DEFENSE;
	}
	
	CSBrickParamModDefense(const std::string&str)
	{
		*this=CSBrickParamModDefense();
		*this=str;
	}
	
	const CSBrickParamModDefense& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()<4)
			return *this;
		
		ParsedOk=true;
		DefenseMode=args[0].c_str();
		ModifierCoefficient=(float)atof(args[1].c_str());
		ModifierConstant=(float)atof(args[2].c_str());
		Duration=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamModCraftSuccess : public TBrickParam::IId
{
	// coefficient to modify craft success chance proportionally to item level
	float ModifierCoefficient;
	// value added to craft success chance
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamModCraftSuccess():
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_CRAFT_SUCCESS;
	}
	
	CSBrickParamModCraftSuccess(const std::string&str)
	{
		*this=CSBrickParamModCraftSuccess();
		*this=str;
	}
	
	const CSBrickParamModCraftSuccess& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()<3)
			return *this;
		
		ParsedOk=true;
		ModifierCoefficient=(float)atof(args[0].c_str());
		ModifierConstant=(float)atof(args[1].c_str());
		Duration=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamModMeleeSuccess : public TBrickParam::IId
{
	// coefficient to modify melee success chance proportionally to item level
	float ModifierCoefficient;
	// value added to melee success chance
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamModMeleeSuccess():
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_MELEE_SUCCESS;
	}
	
	CSBrickParamModMeleeSuccess(const std::string&str)
	{
		*this=CSBrickParamModMeleeSuccess();
		*this=str;
	}
	
	const CSBrickParamModMeleeSuccess& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()<3)
			return *this;
		
		ParsedOk=true;
		ModifierCoefficient=(float)atof(args[0].c_str());
		ModifierConstant=(float)atof(args[1].c_str());
		Duration=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamModRangeSuccess : public TBrickParam::IId
{
	// coefficient to modify range success chance proportionally to item level
	float ModifierCoefficient;
	// value added to range success chance
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamModRangeSuccess():
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_RANGE_SUCCESS;
	}
	
	CSBrickParamModRangeSuccess(const std::string&str)
	{
		*this=CSBrickParamModRangeSuccess();
		*this=str;
	}
	
	const CSBrickParamModRangeSuccess& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()<3)
			return *this;
		
		ParsedOk=true;
		ModifierCoefficient=(float)atof(args[0].c_str());
		ModifierConstant=(float)atof(args[1].c_str());
		Duration=(float)atof(args[2].c_str());

		return *this;
	}
};


struct CSBrickParamModMagicSuccess : public TBrickParam::IId
{
	// coefficient to modify magic success chance proportionally to item level
	float ModifierCoefficient;
	// value added to magic success chance
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamModMagicSuccess():
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_MAGIC_SUCCESS;
	}
	
	CSBrickParamModMagicSuccess(const std::string&str)
	{
		*this=CSBrickParamModMagicSuccess();
		*this=str;
	}
	
	const CSBrickParamModMagicSuccess& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()<3)
			return *this;
		
		ParsedOk=true;
		ModifierCoefficient=(float)atof(args[0].c_str());
		ModifierConstant=(float)atof(args[1].c_str());
		Duration=(float)atof(args[2].c_str());

		return *this;
	}
};



struct CSBrickParamModForageSuccess : public TBrickParam::IId
{
	// ecosystem
	std::string Ecosystem;
	// coefficient to modify forage success chance proportionally to item level
	float ModifierCoefficient;
	// value added to forage success chance
	float ModifierConstant;
	// duration in seconds
	float Duration;
	
	CSBrickParamModForageSuccess():
		Ecosystem(),
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_FORAGE_SUCCESS;
	}
	
	CSBrickParamModForageSuccess(const std::string&str)
	{
		*this=CSBrickParamModForageSuccess();
		*this=str;
	}
	
	const CSBrickParamModForageSuccess& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()<4)
			return *this;
		
		ParsedOk=true;
		Ecosystem=args[0].c_str();
		ModifierCoefficient=(float)atof(args[1].c_str());
		ModifierConstant=(float)atof(args[2].c_str());
		Duration=(float)atof(args[3].c_str());

		return *this;
	}
};


struct CSBrickParamModMagicProtection : public TBrickParam::IId
{
	// affected magic protection 
	std::string AffectedProtection;
	// coefficient to modify protection proportionally to item level
	float ModifierCoefficient;
	// value added to affected protection
	float ModifierConstant;
	// duration in seconds
	float Duration;

	
	CSBrickParamModMagicProtection():
		AffectedProtection(),
		ModifierCoefficient(),
		ModifierConstant(),
		Duration()
	{
		_Id = TBrickParam::SP_MOD_MAGIC_PROTECTION;
	}
	
	CSBrickParamModMagicProtection(const std::string&str)
	{
		*this=CSBrickParamModMagicProtection();
		*this=str;
	}
	
	const CSBrickParamModMagicProtection& operator=(const std::string& input)
	{
		std::vector<std::string> args;
		convertInput(args, input);
		
		if (args.size()!=4)
			return *this;
		
		ParsedOk=true;
		AffectedProtection=args[0].c_str();
		ModifierCoefficient=(float)atof(args[1].c_str());
		ModifierConstant=(float)atof(args[2].c_str());
		Duration=(float)atof(args[3].c_str());
		
		return *this;
	}
};

#endif // RY_EGS_STATIC_BRICK_CPP_H
