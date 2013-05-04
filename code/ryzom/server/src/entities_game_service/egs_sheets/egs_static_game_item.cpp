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

#include "egs_sheets/egs_static_game_item.h"
// game share
#include "game_share/visual_slot_manager.h"
#include "game_share/brick_families.h"
#include "game_share/people.h"
// misc
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
// georges
#include "nel/georges/u_type.h"

#include "game_item_manager/weapon_craft_parameters.h"

#include "egs_sheets/egs_sheets.h"

//--------------------
// USING
//--------------------
using namespace std;
using namespace NLGEORGES;
using namespace NLMISC;


NL_INSTANCE_COUNTER_IMPL(CGuildOption);
NL_INSTANCE_COUNTER_IMPL(CCosmetics);
NL_INSTANCE_COUNTER_IMPL(CConsumable);
NL_INSTANCE_COUNTER_IMPL(CXpCatalyser);
NL_INSTANCE_COUNTER_IMPL(SMeleeWeapon);
NL_INSTANCE_COUNTER_IMPL(SRangeWeapon);
NL_INSTANCE_COUNTER_IMPL(SAmmo);
NL_INSTANCE_COUNTER_IMPL(SArmor);
NL_INSTANCE_COUNTER_IMPL(SShield);
NL_INSTANCE_COUNTER_IMPL(CMP);
NL_INSTANCE_COUNTER_IMPL(CTamingTool);
NL_INSTANCE_COUNTER_IMPL(IItemServiceData);
NL_INSTANCE_COUNTER_IMPL(SItemSpecialEffects);
NL_INSTANCE_COUNTER_IMPL(TCommandTicket);

std::vector<RM_GROUP::TRMGroup> CMP::_RMGroupsByFamily;
std::vector<std::string> CMP::_RMGroupNames;
CMP::CRmGroupByName CMP::_RMGroupsByName;

map<string,uint16>	CConsumable::FamiliesFromName;
vector<string>		CConsumable::FamiliesFromIndex;


namespace GUILD_OPTION
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TType)
		NL_STRING_CONVERSION_TABLE_ENTRY(PlayerMainBuilding)
		NL_STRING_CONVERSION_TABLE_ENTRY(GuildMainBuilding)
//		NL_STRING_CONVERSION_TABLE_ENTRY(GuildRmFight)
//		NL_STRING_CONVERSION_TABLE_ENTRY(GuildRmMagic)
//		NL_STRING_CONVERSION_TABLE_ENTRY(GuildRmHarvest)
//		NL_STRING_CONVERSION_TABLE_ENTRY(GuildRmCraft)
		NL_STRING_CONVERSION_TABLE_ENTRY(Unknown)
	NL_END_STRING_CONVERSION_TABLE(TType, Conversion, Unknown)
	
	//--------------------------------------------------------------
	TType fromString( const std::string & str )
	{
		return Conversion.fromString(str);
	}
	//--------------------------------------------------------------
	const std::string & toString( TType type )
	{
		return Conversion.toString(type);
	}
}

//--------------------------------------------------------------
void CCosmetics::serial(class NLMISC::IStream &f)
{
	f.serial( VPValue );
}

//--------------------------------------------------------------
IItemServiceData * IItemServiceData::buildItemServiceData(ITEM_SERVICE_TYPE::TItemServiceType itemServiceType)
{
	switch (itemServiceType)
	{
	case ITEM_SERVICE_TYPE::SpeedUpDPLoss:
		return new CSpeedUpDPLossData;
	}

	return NULL;
}

//--------------------------------------------------------------
void CConsumable::serial(class NLMISC::IStream &f)
{
	f.serial(LoopTimer);
	f.serial(MaxNbLoops);
	f.serial(OverdoseTimer);
	f.serial(ConsumptionTime);
	f.serial(Data);

	f.serialCont( StringParams );

	if (f.isReading() )
	{
		// family
		string familyName;
		f.serial(familyName);
		if (FamiliesFromName.find(familyName) == FamiliesFromName.end())
		{
			Family = (uint16)FamiliesFromIndex.size();
			FamiliesFromName.insert( make_pair(familyName, Family) );
			FamiliesFromIndex.push_back(familyName);
		}
		else
		{
			Family = FamiliesFromName[familyName];
		}

		// Params
		Params.clear();
		uint size = (uint)StringParams.size();
		for (uint i = 0 ; i < size ; ++i)
		{
			addParam(StringParams[i], Params);
		}
	}
	else
	{
		// family
		nlassert(Family < FamiliesFromIndex.size());
		string &familyName = FamiliesFromIndex[Family];
		f.serial(familyName);

		// params
		// nothing to do -> keep params and only serialize the param as strings
	}
}


//--------------------------------------------------------------
void CXpCatalyser::serial(class NLMISC::IStream &f)
{
	f.serial(IsRingCatalyser);
	f.serial(XpBonus);
}

//--------------------------------------------------------------
static std::vector<std::string> strsplit(std::string str, std::string sep)
{
	std::vector<std::string> strings;
	size_t index = 0, index2 = 0;
	while ((index2=str.find(sep, index))!=std::string::npos)
	{
		strings.push_back(str.substr(index, index2-index));
		index = index2+1;
	}
	strings.push_back(str.substr(index, std::string::npos));
	return strings;
}

bool	SItemSpecialEffect::build(std::string const& str)
{
	std::vector<std::string> itemSpecialEffectParams = strsplit(str, ":");
	EffectType = ITEM_SPECIAL_EFFECT::fromString(itemSpecialEffectParams[0]);
	switch (EffectType)
	{
	case ITEM_SPECIAL_EFFECT::ISE_FIGHT_ADD_CRITICAL:
	case ITEM_SPECIAL_EFFECT::ISE_FIGHT_VAMPIRISM:
	case ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION:
	case ITEM_SPECIAL_EFFECT::ISE_FORAGE_NO_RISK:
		if(itemSpecialEffectParams.size()!=2)
		{
			nlwarning("<loadItemFX> EffectType needs 1 arg in '%s'", str.c_str());
			return false;
		}
		EffectArgFloat[0] = atof(itemSpecialEffectParams[1].c_str());
		return true;
		break;
	case ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN:
	case ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_STAT_BONUS:
	case ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_LIMIT:
	case ITEM_SPECIAL_EFFECT::ISE_FORAGE_ADD_RM:
		if(itemSpecialEffectParams.size()!=3)
		{
			nlwarning("<loadItemFX> EffectType needs 2 args in '%s'", str.c_str());
			return false;
		}
		EffectArgFloat[0] = atof(itemSpecialEffectParams[1].c_str());
		EffectArgFloat[1] = atof(itemSpecialEffectParams[2].c_str());
		return true;
		break;
	}

	nlwarning("<loadItemFX> unreconized EffectType in '%s'", str.c_str());
	return false;
}

void SItemSpecialEffect::serial(class NLMISC::IStream &f)
{
	// Don't forget to change the SItem version and the code here if no more 4.
	nlctassert(MaxEffectPerItem==4);
		
	f.serialEnum( EffectType );
//	f.serial( EffectType );
	f.serial( EffectArgFloat[0] );
	f.serial( EffectArgFloat[1] );
	f.serial( EffectArgFloat[2] );
	f.serial( EffectArgFloat[3] );
	f.serial( EffectArgString[0] );
	f.serial( EffectArgString[1] );
	f.serial( EffectArgString[2] );
	f.serial( EffectArgString[3] );
}

void SItemSpecialEffects::serial(class NLMISC::IStream &f)
{
	f.serialCont(Effects);
}

//--------------------------------------------------------------
//						init()  
//--------------------------------------------------------------
void CStaticItem::init()
{
	Family			= ITEMFAMILY::UNDEFINED;
	Type			= ITEM_TYPE::UNDEFINED;
	
	Armor			= NULL;
	MeleeWeapon		= NULL;
	RangeWeapon		= NULL;
	Ammo			= NULL;
	Shield			= NULL;
	TamingTool		= NULL;
	Mp				= NULL;
	GuildOption		= NULL;
	Cosmetics		= NULL;
	ItemServiceData	= NULL;
	ConsumableItem	= NULL;
	XpCatalyser		= NULL;
	CommandTicket	= NULL;

	Skill			= SKILLS::unknown;
	MinSkill		= 0;
	CraftingToolType = TOOL_TYPE::Unknown;

	Origin			= ITEM_ORIGIN::UNKNOWN;
	Sack			= std::string("");
	Stackable		= 1;
	Color			= -2;
	SlotCount		= 0;
	Bulk			= 0;
	Weight			= 0;
	Saleable		= false;
	NoRent			= false;

	ItemIdSheetToModelNumber = 0;
	ItemIdSheetToModelNumberLeftHands = 0;

	RequiredSkill			= SKILLS::unknown;
	MinRequiredSkillLevel	= 0;
	RequiredSkillQualityFactor = 0.0f;
	RequiredSkillQualityOffset = 0;
	RequiredSkill2			= SKILLS::unknown;
	MinRequiredSkillLevel2	= 0;
	RequiredSkillQualityFactor2 = 0.0f;
	RequiredSkillQualityOffset2 = 0;
	
	RequiredCharac			= CHARACTERISTICS::Unknown;
	MinRequiredCharacLevel	= 0;
	RequiredCharacQualityFactor = 0.0f;
	RequiredCharacQualityOffset = 0;

	ItemSpecialEffects = NULL;
} // init //



//--------------------------------------------------------------
// copy constructor
//--------------------------------------------------------------
CStaticItem::CStaticItem( const CStaticItem& itm )
{
	*this = itm;
	if(itm.Armor)
	{
		Armor = new SArmor();
		*Armor = *itm.Armor;
	}
	if(itm.MeleeWeapon)
	{
		MeleeWeapon = new SMeleeWeapon();
		*MeleeWeapon = *itm.MeleeWeapon;
	}
	if(itm.RangeWeapon)
	{
		RangeWeapon = new SRangeWeapon();
		*RangeWeapon = *itm.RangeWeapon;
	}
	if(itm.Ammo)
	{
		Ammo = new SAmmo();
		*Ammo = *itm.Ammo;
	}
	if(itm.Shield)
	{
		Shield = new SShield();
		*Shield = *itm.Shield;
	}
	if(itm.TamingTool)
	{
		TamingTool = new CTamingTool();
		*TamingTool = *itm.TamingTool;
	}
	if(itm.Mp)
	{
		Mp = new CMP();
		*Mp = *itm.Mp;
	}
	if(itm.GuildOption)
	{
		GuildOption = new CGuildOption();
		*GuildOption = *itm.GuildOption;
	}
	if(itm.Cosmetics)
	{
		Cosmetics = new CCosmetics();
		*Cosmetics = *itm.Cosmetics;
	}
	if(itm.ConsumableItem)
	{
		ConsumableItem = new CConsumable();
		*ConsumableItem = *itm.ConsumableItem;
	}
	if(itm.XpCatalyser)
	{
		XpCatalyser = new CXpCatalyser();
		*XpCatalyser = *itm.XpCatalyser;
	}
	if(itm.ItemSpecialEffects)
	{
		ItemSpecialEffects = new SItemSpecialEffects();
		*ItemSpecialEffects = *itm.ItemSpecialEffects;
	}
	if(itm.ItemServiceData)
	{
		ItemServiceData = itm.ItemServiceData->clone();
	}
	if( itm.CommandTicket)
	{
		CommandTicket = new TCommandTicket();
		*CommandTicket = *itm.CommandTicket;
	}
}


//--------------------------------------------------------------
//						~CStaticItem()  
//--------------------------------------------------------------
CStaticItem::~CStaticItem()
{
	clearPtrs(true);
} // Destructor //


//--------------------------------------------------------------
//						lookForEffects()
//--------------------------------------------------------------
std::vector<SItemSpecialEffect> CStaticItem::lookForEffects(ITEM_SPECIAL_EFFECT::TItemSpecialEffect effectType) const
{
	std::vector<SItemSpecialEffect> effects;
	if (ItemSpecialEffects)
	{
		std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
		for (it=ItemSpecialEffects->Effects.begin(), itEnd=ItemSpecialEffects->Effects.end(); it!=itEnd; ++it)
		{
			if (it->EffectType==effectType)
				effects.push_back(*it);
		}
	}
	return effects;
}

//--------------------------------------------------------------
//						loadMeleeWeapon()  
//--------------------------------------------------------------
void loadMeleeWeapon( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if (!item)
		return;

	if (item->MeleeWeapon == NULL)
		item->MeleeWeapon = new SMeleeWeapon();

	const UFormElm *rangeWeapon = NULL;
	
	if ( root.getNodeByName ( &rangeWeapon, "melee weapon") && rangeWeapon )
	{
		string value;
			
		if (root.getValueByName( value, "melee weapon.category" ) )
			item->MeleeWeapon->WeaponType = WEAPONTYPE::stringToWeaponType( value );

		if ( root.getValueByName(value , "melee weapon.skill" ) )
			item->Skill = SKILLS::toSkill( value );

		root.getValueByName( item->MinSkill, "melee weapon.minimum score" );

		root.getValueByName( item->MeleeWeapon->RateOfFire, "melee weapon.rate of fire" );
		root.getValueByName( item->MeleeWeapon->Latency, "melee weapon.latency" );

		root.getValueByName( item->MeleeWeapon->ReachValue, "melee weapon.melee range" );

		if ( root.getValueByName( value, "melee weapon.damage type" ) )
		{
			item->MeleeWeapon->DamageType = DMGTYPE::stringToDamageType( value );			
		}
		else
			nlwarning("<loadMeleeWeapon> DamageType not found for weapon %s", item->Name.c_str() );
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadMeleeWeapon> Can't find the 'melee weapon' structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadMeleeWeapon //

//--------------------------------------------------------------
//						loadRangeWeapon()  
//--------------------------------------------------------------
void loadRangeWeapon( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if (!item)
		return;

	if (item->RangeWeapon == NULL)
		item->RangeWeapon = new SRangeWeapon();

	const UFormElm *rangeWeapon = NULL;
	
	if ( root.getNodeByName ( &rangeWeapon, "range weapon") && rangeWeapon )
	{
		string value;
		
		if ( root.getValueByName( value, "range weapon.category" ) )
			item->RangeWeapon->WeaponType = WEAPONTYPE::stringToWeaponType( value );
		
		if ( root.getValueByName(value , "range weapon.skill" ) )
			item->Skill = SKILLS::toSkill( value );

		root.getValueByName( item->MinSkill, "range weapon.minimum score" );

		root.getValueByName( item->MinSkill, "range weapon.minimum score" );

		root.getValueByName(value ,"range weapon.RangeWeaponType" );
		item->RangeWeapon->AreaType = RANGE_WEAPON_TYPE::stringToRangeWeaponType(value);
		if ( item->RangeWeapon->AreaType == RANGE_WEAPON_TYPE::Unknown )
		{
			nlwarning("<loadRangeWeapon>unknown area type %s", value.c_str());
		}
		else if ( item->RangeWeapon->AreaType == RANGE_WEAPON_TYPE::Missile )
		{
			root.getValueByName( item->RangeWeapon->Missile.Radius ,"range weapon.MissileRadius" );
			root.getValueByName( item->RangeWeapon->Missile.MinFactor ,"range weapon.MissileMinFactor" );
		}
		else if ( item->RangeWeapon->AreaType == RANGE_WEAPON_TYPE::Gatlin )
		{
			root.getValueByName( item->RangeWeapon->Gatling.Height ,"range weapon.GatlingHeight" );
			root.getValueByName( item->RangeWeapon->Gatling.Base ,"range weapon.GatlingBase" );
			root.getValueByName( item->RangeWeapon->Gatling.Angle ,"range weapon.GatlingAngle" );
		}
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadRangeWeapon> Can't find the 'range weapon' structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadRangeWeapon //

//--------------------------------------------------------------
//						loadAmmo()  
//--------------------------------------------------------------
void loadAmmo( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;

	if (item->Ammo == NULL)
		item->Ammo = new SAmmo();

	const UFormElm *ammo = NULL;
	
	if ( root.getNodeByName ( &ammo, "ammo") && ammo )
	{
		string value;
		
		if (root.getValueByName( value, "ammo.damage type" ))
			item->Ammo->DamageType = DMGTYPE::stringToDamageType( value );
		
		if (root.getValueByName(value , "ammo.weapon type" ) )
			item->Skill = SKILLS::toSkill( value );

//		root.getValueByName( item->MinSkill, "minimum score" );

		root.getValueByName( item->Ammo->DamageFactor, "ammo.damage factor" );
		root.getValueByName( item->Ammo->RateOfFire, "ammo.rate of fire" );
		root.getValueByName( item->Ammo->Latency, "ammo.latency" );


		root.getValueByName( item->Ammo->ShortRangeLimit, "ammo.short range limit" );
		root.getValueByName( item->Ammo->MediumRangeLimit, "ammo.medium range limit" );
		root.getValueByName( item->Ammo->LongRangeLimit, "ammo.long range limit" );

		root.getValueByName( item->Ammo->AmmoType, "ammo.type" );
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadAmmo> Can't find the 'ammo' structure for item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadAmmo //



//--------------------------------------------------------------
//						loadArmor()  
//--------------------------------------------------------------
void loadArmor( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;

	if (item->Armor == NULL)
		item->Armor = new SArmor();

	const UFormElm *armor = NULL;
	
	if ( root.getNodeByName ( &armor, "armor") && armor )
	{
		string value;
		if ( root.getValueByName( value, "armor.Armor category" ) )
			item->Armor->ArmorType = ARMORTYPE::toArmorType( value );

//		if (root.getValueByName(value , "armor.Skill" ) )
//			item->Skill = SKILLS::toSkill( value );

		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::PIERCING].Max, "armor.Protections.PiercingMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::PIERCING].Factor, "armor.Protections.PiercingFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::SLASHING].Max, "armor.Protections.SlashingMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::SLASHING].Factor, "armor.Protections.SlashingFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::BLUNT].Max, "armor.Protections.BluntMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::BLUNT].Factor, "armor.Protections.BluntFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::ROT].Max, "armor.Protections.RotMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::ROT].Factor, "armor.Protections.RotFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::ACID].Max, "armor.Protections.AcidMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::ACID].Factor, "armor.Protections.AcidFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::COLD].Max, "armor.Protections.ColdMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::COLD].Factor, "armor.Protections.ColdFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::FIRE].Max, "armor.Protections.FireMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::FIRE].Factor, "armor.Protections.FireFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::POISON].Max, "armor.Protections.PoisonMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::POISON].Factor, "armor.Protections.PoisonFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::ELECTRICITY].Max, "armor.Protections.ElectricityMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::ELECTRICITY].Factor, "armor.Protections.ElectricityFactor" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::SHOCK].Max, "armor.Protections.ShockMax" );
		root.getValueByName( item->Armor->Protections[(uint)DMGTYPE::SHOCK].Factor, "armor.Protections.ShockFactor" );
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadArmor> Can't find the 'armor' structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadArmor //


//--------------------------------------------------------------
//						loadShield()  
//--------------------------------------------------------------
void loadShield( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;

	if (item->Shield == NULL)
		item->Shield = new SShield();

	if( sheetId.toString() == "pre_order.sitem" )
	{
		item->Shield->Unbreakable = true;
	}

	const UFormElm *shield = NULL;
	
	if ( root.getNodeByName ( &shield, "shield") && shield )
	{
		string value;
		if ( root.getValueByName( value, "shield.Category" ) )
			item->Shield->ShieldType = SHIELDTYPE::stringToShieldType( value );

//		if ( root.getValueByName(value , "shield.Skill" ) )
//			item->Skill = SKILLS::toSkill( value );
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadShield> Can't find the 'shield' structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadShield //


//--------------------------------------------------------------
//						loadFaberTool()  
//--------------------------------------------------------------
void loadFaberTool( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;

	const UFormElm *tool = NULL;
	
	if( !root.getValueByName( item->ItemPrice, "basics.Price" ) )
	{
		item->ItemPrice = 0;
		nlwarning("<loadFaberTool> Can't load 'Price' in sheet %s", sheetId.toString().c_str() );
	}

	if ( root.getNodeByName ( &tool, "crafting tool") && tool )
	{
		string value;
		
		//if ( root.getValueByName(value , "crafting tool.skill" ) )
		//	item->Skill = SKILLS::toSkill( value );

		if ( root.getValueByName(value , "crafting tool.type" ) )
			item->CraftingToolType = TOOL_TYPE::toToolType( value );
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadFaberTool> Can't find the 'crafting tool'structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadFaberTool //


//--------------------------------------------------------------
//						loadHarvestTool()  
//--------------------------------------------------------------
void loadHarvestTool( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;

	if( !root.getValueByName( item->ItemPrice, "basics.Price" ) )
	{
		item->ItemPrice = 0;
		nlwarning("<loadFaberTool> Can't load 'Price' in sheet %s", sheetId.toString().c_str() );
	}

	const UFormElm *tool = NULL;
	
	if ( root.getNodeByName ( &tool, "harvest tool") && tool )
	{
		string value;
		
		if ( root.getValueByName(value , "harvest tool.skill" ) )
			item->Skill = SKILLS::toSkill( value );
	}
	else
	{
		nlwarning("<loadHarvestTool> Can't find the 'harvest tool' structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadHarvestTool //


//--------------------------------------------------------------
//						loadTamingTool()  
//--------------------------------------------------------------
void loadTamingTool( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;
	
	if ( item->TamingTool == NULL)
		item->TamingTool = new CTamingTool();
	
	const UFormElm *tool = NULL;
	
	if ( root.getNodeByName ( &tool, "taming tool") && tool )
	{
		string value;
		
		if ( root.getValueByName(value , "taming tool.skill" ) )
			item->Skill = SKILLS::toSkill( value );
		
		if ( root.getValueByName(value , "taming tool.type" ) )
			item->TamingTool->Type = TAMING_TOOL_TYPE::toToolType( value );
		
		root.getValueByName(item->TamingTool->CommandRange , "taming tool.command range" );
		root.getValueByName(item->TamingTool->MaxDonkeys , "taming tool.max donkey" );
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadTamingTool> Can't find the 'taming tool' structure in item %s (sheet %s)", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadTamingTool //


//--------------------------------------------------------------
//						loadRawMaterial()  
//--------------------------------------------------------------
void loadRawMaterial( UFormElm &root, CStaticItem *item, const CSheetId &sheetId )
{
	if ( ! item)
		return;

	//nldebug( "MP %s", sheetId.toString().c_str() );

	if ( item->Mp == NULL)
		item->Mp = new CMP();

	const UFormElm *mp = NULL;
	
	if ( root.getNodeByName ( &mp, "mp") && mp )
	{
		string val;
	
		uint32 uval;
		if (mp->getValueByName( uval, "Family") )
			item->Mp->Family = (RM_FAMILY::TRMFamily)uval;

		if (mp->getValueByName( uval, "Group" )) // see also CMP::loadGroups()
			item->Mp->setGroup( (RM_GROUP::TRMGroup)uval );

		if (mp->getValueByName( val, "Ecosystem"))
			item->Mp->Ecosystem = ECOSYSTEM::stringToEcosystem( val );
		
		if(mp->getValueByName( val, "HarvestSkill"))
			item->Skill = SKILLS::toSkill( val );
		
		if (mp->getValueByName( val, "Category"))
			item->Mp->Category = MP_CATEGORY::stringToMPCategory( val );

		mp->getValueByName( item->Mp->StatEnergy, "StatEnergy" );

		mp->getValueByName( item->Mp->MaxQuality, "MaxQuality" );

		mp->getValueByName( item->Mp->Rarity, "Rarity" );

		mp->getValueByName( item->Mp->MpColor, "MpColor" );
		
		uint size;
		const UFormElm *mpFaberParam = NULL;
		root.getNodeByName( &mpFaberParam, "mp.MpParam" );
		if( mpFaberParam != 0 )
		{
			nlverify( mpFaberParam->getStructSize(size) );

			for( uint i = 0; i < size; ++i )
			{
				const UFormElm *mpFaberParameter = NULL;
				mpFaberParam->getStructNode( i, &mpFaberParameter );
				if( mpFaberParameter )
				{
					CMP::TMpFaberParameters v;
					uint16 value;
					if( mpFaberParameter->getValueByName( value, "Durability" ) )
					{
						if( value > 0 )
						{
							v.Durability = value / 100.0f;
							
							v.MpFaberType = (RM_FABER_TYPE::TRMFType) i;

							mpFaberParameter->getValueByName( value, "Weight" );
							v.Weight = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "SapLoad" );
							v.SapLoad = max( 0.0f, min( value / 100.0f, 1.0f ) );
							
							mpFaberParameter->getValueByName( value, "DMG" );
							v.Dmg = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "Speed" );
							v.Speed = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "Range" );
							v.Range = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "DodgeModifier" );
							v.DodgeModifier = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ParryModifier" );
							v.ParryModifier = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "AdversaryDodgeModifier" );
							v.AdversaryDodgeModifier = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "AdversaryParryModifier" );
							v.AdversaryParryModifier = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ProtectionFactor" );
							v.ProtectionFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "MaxSlashingProtection" );
							v.MaxSlashingProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "MaxBluntProtection" );
							v.MaxBluntProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "MaxPiercingProtection" );
							v.MaxPiercingProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "AcidProtection" );
							v.AcidProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ColdProtection" );
							v.ColdProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "FireProtection" );
							v.FireProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "RotProtection" );
							v.RotProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ShockWaveProtection" );
							v.ShockWaveProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "PoisonProtection" );
							v.PoisonProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ElectricityProtection" );
							v.ElectricityProtection = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "DesertResistance" );
							v.DesertResistance = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ForestResistance" );
							v.ForestResistance = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "LacustreResistance" );
							v.LacustreResistance = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "JungleResistance" );
							v.JungleResistance = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "PrimaryRootResistance" );
							v.PrimaryRootResistance = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ElementalCastingTimeFactor" );
							v.ElementalCastingTimeFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "ElementalPowerFactor" );
							v.ElementalPowerFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "OffensiveAfflictionCastingTimeFactor" );
							v.OffensiveAfflictionCastingTimeFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "OffensiveAfflictionPowerFactor" );
							v.OffensiveAfflictionPowerFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "HealCastingTimeFactor" );
							v.HealCastingTimeFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "HealPowerFactor" );
							v.HealPowerFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "DefensiveAfflictionCastingTimeFactor" );
							v.DefensiveAfflictionCastingTimeFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );
							mpFaberParameter->getValueByName( value, "DefensiveAfflictionPowerFactor" );
							v.DefensiveAfflictionPowerFactor = max( 0.0f, min( value / 100.0f, 1.0f ) );

							string civS;
							mpFaberParameter->getValueByName( civS, "CraftCivSpec" );
							v.CraftCivSpec = ITEM_ORIGIN::stringToEnum( civS );

							mpFaberParameter->getValueByName( v.HpBuff, "HpBuff" );
							mpFaberParameter->getValueByName( v.SapBuff, "SapBuff" );
							mpFaberParameter->getValueByName( v.StaBuff, "StaBuff" );
							mpFaberParameter->getValueByName( v.FocusBuff, "FocusBuff" );
							
							item->Mp->MpFaberParameters.push_back( v );
						}
					}
				}
			}
		}
	}
	else
	{
		// do not show warning for graphicals sheets (starting with fy_, tr_ etc...) or other special sheets with empty name
		if ( !item->Name.empty() )
			nlwarning("<loadRawMaterial> Can't find the 'mp' structure in item %s (sheet %s) ?!?", item->Name.c_str(), sheetId.toString().c_str() );
	}
} // loadRawMaterial //


//--------------------------------------------------------------
//						loadPet()  
//--------------------------------------------------------------
void loadPet(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	if ( ! item )
		return;
	
	string value;
	if( root.getValueByName( value, "pet.Pet Sheet" ) )
	{
		item->PetSheet = CSheetId( value );
	}
	else
	{
		nlwarning("<loadPet> Can't load 'pet.Pet Sheet' in sheet %s", sheetId.toString().c_str() );
	}

	if( !root.getValueByName( item->ItemPrice, "pet.Pet Price" ) )
	{
		item->ItemPrice = 666;
		nlwarning("<loadPet> Can't load 'pet.Pet Price' in sheet %s", sheetId.toString().c_str() );
	}
	if ( ! root.getValueByName( item->PetHungerCount, "pet.Hunger Count" ) )
	{
		item->PetHungerCount = 1000;
		nlwarning("<loadPet> Can't load 'pet.Hunger Count' in sheet %s", sheetId.toString().c_str() );
	}
} //loadPet


//--------------------------------------------------------------
//						loadFood()  
//--------------------------------------------------------------
void loadFood( NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	if ( ! root.getValueByName( item->Calories, "food.Calories" ) )
	{
		item->Calories = 20;
		nlwarning("<loadPet> Can't load 'food.Calories' in sheet %s", sheetId.toString().c_str() );
	}
	if ( item->Calories == 0 )
	{
		nlwarning( "Invalid 0 Calories in %s", sheetId.toString().c_str() );
		item->Calories = 20;
	}
}


//--------------------------------------------------------------
//						loadGuildOption()  
//--------------------------------------------------------------
void loadGuildOption(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	nlassert(item);
	nlassert(item->GuildOption == NULL);
	string value;
	item->GuildOption = new CGuildOption;
	if( !root.getValueByName( value, "guild option.Type" ) )
	{
		nlwarning("<loadGuildOption> Can't load 'guild option.Type' in sheet %s", sheetId.toString().c_str() );
	}
	item->GuildOption->Type = GUILD_OPTION::fromString( value );
	if ( item->GuildOption->Type == GUILD_OPTION::Unknown )
	{
		nlwarning("<loadGuildOption> guild option.Type = '%s' ( invalid ) in sheet %s", value.c_str(), sheetId.toString().c_str() );
	}
//	if( !root.getValueByName( item->GuildOption->XpCost, "guild option.Guild XP Cost" ) )
//	{
//		nlwarning("<loadGuildOption> Can't load 'guild option.Guild XP Cost' in sheet %s", sheetId.toString().c_str() );
//	}
	if( !root.getValueByName( item->GuildOption->MoneyCost, "guild option.Money Cost" ) )
	{
		nlwarning("<loadGuildOption> Can't load 'guild option.Money Cost' in sheet %s", sheetId.toString().c_str() );
	}	
}

//--------------------------------------------------------------
//						loadCosmetics()  
//--------------------------------------------------------------
void loadCosmetics(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	nlassert(item);
	nlassert(item->Cosmetics == NULL);
	item->Cosmetics = new CCosmetics;

	std::string name = sheetId.toString();

	string::size_type pos = name.find('.',0);
	if ( pos == string::npos)
		nlwarning("<loadCosmetics> Can't load the VPValue from sheet name in sheet %s", sheetId.toString().c_str() );
	else
	{
		sint i = (sint)pos - 1;
		for(; i >= 0; i-- )
		{
			if ( !isdigit( name[i] ) )
				break;
		}
		if ( i >= -1 )
		{
			name = name.substr( i+1, pos - i - 1 );
			NLMISC::fromString( name, item->Cosmetics->VPValue );
		}
	}

	
	if( !root.getValueByName( item->Cosmetics->VPValue, "Cosmetics.Visual Property Value" ) )
	{
		nlwarning("<loadCosmetics> Can't load 'Cosmetics.VPValue' in sheet %s", sheetId.toString().c_str() );
	}
	if( !root.getValueByName (item->ItemPrice, "basics.Price") )
	{
		nlwarning("<loadCosmetics> Can't load 'basics.Price' in sheet %s", sheetId.toString().c_str() );
	}
}

//--------------------------------------------------------------
//						loadItemService()
//--------------------------------------------------------------
void loadItemService(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	string strServiceType;
	if (root.getValueByName(strServiceType, "service.ServiceType"))
	{
		item->ItemServiceType = ITEM_SERVICE_TYPE::fromString(strServiceType);
	}
	else
	{
		item->ItemServiceType = ITEM_SERVICE_TYPE::Unknown;
		nlwarning("<loadItemService> Can't load 'service.ServiceType' in sheet %s", sheetId.toString().c_str());
	}

	// build item service data
	item->ItemServiceData = IItemServiceData::buildItemServiceData(item->ItemServiceType);

	if (item->ItemServiceType == ITEM_SERVICE_TYPE::SpeedUpDPLoss)
	{
		CSpeedUpDPLossData * data = dynamic_cast<CSpeedUpDPLossData *>(item->ItemServiceData);
		nlassert(data);
		nlverify( root.getValueByName(data->DurationInDays, "service.DPLossDuration") );
	}
	else
	{
		nlassert(item->ItemServiceData == NULL);
	}

	// error if service is unknown
	if (item->ItemServiceType == ITEM_SERVICE_TYPE::Unknown)
	{
		nlerror("Unknow service type in %s", sheetId.toString().c_str());
	}
}


//--------------------------------------------------------------
//						loadConsumable()
//--------------------------------------------------------------
void loadConsumable(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	nlassert(item);
	nlassert(item->ConsumableItem == NULL);
	item->ConsumableItem = new CConsumable;

	//  family
	string familyName;
	if ( !root.getValueByName(familyName, "Consumable.Family") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.Family'");

	if (CConsumable::FamiliesFromName.find(familyName) == CConsumable::FamiliesFromName.end())
	{
		const uint16 index = (uint16)CConsumable::FamiliesFromIndex.size();
		CConsumable::FamiliesFromName.insert( make_pair(familyName, index) );
		CConsumable::FamiliesFromIndex.push_back(familyName);
		item->ConsumableItem->Family= index;
	}
	else
	{
		item->ConsumableItem->Family = (*CConsumable::FamiliesFromName.find(familyName)).second;
	}

	bool flag;

	if( ! root.getValueByName (flag, "Consumable.BreakWhenHit") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.BreakWhenHit'");
	else
		item->ConsumableItem->Flags.BreakWhenHit = (flag?1:0);

	if( ! root.getValueByName (flag, "Consumable.Sit") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.Sit'");
	else
		item->ConsumableItem->Flags.Sit = (flag?1:0);

	if( ! root.getValueByName (flag, "Consumable.StandUp") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.StandUp'");
	else
		item->ConsumableItem->Flags.StandUp = (flag?1:0);

	if( ! root.getValueByName (flag, "Consumable.Swim") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.Swim'");
	else
		item->ConsumableItem->Flags.Swim = (flag?1:0);

	if( ! root.getValueByName (flag, "Consumable.Mektoub") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.Mektoub'");
	else
		item->ConsumableItem->Flags.Mektoub = (flag?1:0);

	if ( !root.getValueByName(item->ConsumableItem->LoopTimer, "Consumable.LoopTimer") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.LoopTimer'");

	if ( !root.getValueByName(item->ConsumableItem->MaxNbLoops, "Consumable.MaxNbLoops") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.MaxNbLoops'");

	if ( !root.getValueByName(item->ConsumableItem->OverdoseTimer, "Consumable.OverdoseTimer") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.OverdoseTimer'");

	if ( !root.getValueByName(item->ConsumableItem->ConsumptionTime, "Consumable.ConsumptionTime") )
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.ConsumptionTime'");

	// read the params
	//Params
	item->ConsumableItem->StringParams.clear();
	for (uint i=0;i<4;++i)
	{
		string param;
		std::string s=NLMISC::toString("Consumable.Property %i",i);
		if ( root.getValueByName (param, s.c_str()) && !param.empty() )
			item->ConsumableItem->StringParams.push_back(param);
	}
	// Parse Params
	item->ConsumableItem->Params.clear();
	const uint size = (uint)item->ConsumableItem->StringParams.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		addParam(item->ConsumableItem->StringParams[i], item->ConsumableItem->Params);
	}
}

//--------------------------------------------------------------
//						loadXpCatalyser()
//--------------------------------------------------------------
void loadXpCatalyser( NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	nlassert(item);
	nlassert(item->XpCatalyser == NULL);
	item->XpCatalyser = new CXpCatalyser();
	
	// 
	if ( !root.getValueByName(item->XpCatalyser->IsRingCatalyser, "Xp Catalyser.IsRing") )
	{
		nlwarning("<loadXpCatalyser> cannot read the value 'Xp Catalyser.IsRing'");
	}

	// gain factor
	if ( !root.getValueByName(item->XpCatalyser->XpBonus, "Xp Catalyser.Xp Bonus") )
	{
		nlwarning("<loadXpCatalyser> cannot read the value 'Xp Catalyser.Xp Bonus'");
	}
}

//--------------------------------------------------------------
//						loadItemSpecialEffects()
//--------------------------------------------------------------
void loadItemSpecialEffects( NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	nlassert(item);
	nlassert(item->XpCatalyser == NULL);
	item->ItemSpecialEffects = new SItemSpecialEffects();
	
	// Parse effects
	for(uint i=0;i<SItemSpecialEffect::MaxEffectPerItem;i++)
	{
		std::string str;
		char	token[256];
		sprintf(token, "Effects.Effect%d", i+1);
		if ( root.getValueByName(str, token) )
		{
			if (!str.empty())
			{
				SItemSpecialEffect	fx;
				if(fx.build(str))
				{
					item->ItemSpecialEffects->Effects.push_back(fx);
				}
			}
		}
		else
			nlwarning("<loadItemFX> cannot read the value '%s'", token);
	}
}

//--------------------------------------------------------------
//						
//--------------------------------------------------------------
void loadCommandTicket( NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId )
{
	nlassert(item);
	nlassert(item->CommandTicket == NULL);
	item->CommandTicket = new TCommandTicket();

	root.getValueByName(item->CommandTicket->Command, "CommandTicket.Command");
	root.getValueByName(item->CommandTicket->Priviledge, "CommandTicket.Priviledge");
	root.getValueByName(item->CommandTicket->NbRun, "CommandTicket.NbRun");
}

//--------------------------------------------------------------
//						serial()  
//--------------------------------------------------------------
void CStaticItem::serial(class NLMISC::IStream &f)
{
	f.serial( SheetId );
	f.serialEnum( Origin );
	f.serialEnum( Family );
	f.serialEnum( Type );
	f.serial( Name );
	f.serial( Sack );
	f.serial( Stackable );
	f.serial( Color );
	f.serial( DropOrSell );
	f.serial( ShardExchangeable );
	f.serial( CraftPlan );
	f.serial( ItemPrice );
	f.serial( Bulk );
	f.serial( Weight );
	f.serial(TimeToEquip);
	f.serial( Saleable );
	f.serial( NoRent );
	f.serial( Consumable );
	f.serial( EffectWhenConsumed );
	f.serial( EmoteWhenConsumed );

	f.serialEnum(CraftingToolType);
	
	f.serialEnum(Skill);
	f.serial(MinSkill);

	f.serialCont( Slots );
	f.serial( WearEquipmentMalus );

	f.serial(PetSheet);

	f.serial(Destination);
	f.serialEnum( TpType );
	f.serialEnum( TpEcosystem );
	
	f.serial( WeightMax );
	f.serial( BulkMax );
	f.serial( SlotCount );

	f.serial( ItemIdSheetToModelNumber );
	f.serial( ItemIdSheetToModelNumberLeftHands );
	
	f.serialEnum( RequiredSkill );
	f.serial( MinRequiredSkillLevel );
	f.serial( RequiredSkillQualityFactor );
	f.serial( RequiredSkillQualityOffset );
	f.serialEnum( RequiredSkill2 );
	f.serial( MinRequiredSkillLevel2 );
	f.serial( RequiredSkillQualityFactor2 );
	f.serial( RequiredSkillQualityOffset2 );
	
	f.serialEnum( RequiredCharac );
	f.serial( MinRequiredCharacLevel );
	f.serial( RequiredCharacQualityFactor );
	f.serial( RequiredCharacQualityOffset );
	
	f.serialCont( TypeSkillMods );
	
//	f.serial( AmmoWeaponType );
//	f.serial( WeaponType );

	if (f.isReading() )
	{
		ItemSpecialEffects = new SItemSpecialEffects();
		ItemSpecialEffects->serial(f);
		
		switch ( Family )
		{
		case ITEMFAMILY::MELEE_WEAPON:
			MeleeWeapon = new SMeleeWeapon();
			MeleeWeapon->serial(f);
			break;
			
		case ITEMFAMILY::RANGE_WEAPON:
			RangeWeapon = new SRangeWeapon();
			RangeWeapon->serial(f);
			break;

		case ITEMFAMILY::AMMO:
			Ammo = new SAmmo();
			Ammo->serial(f);
			break;

		case ITEMFAMILY::SHIELD:
			Shield = new SShield();
			Shield->serial(f);
			break;

		case ITEMFAMILY::ARMOR:
			Armor = new SArmor();
			Armor->serial(f);
			break;

		case ITEMFAMILY::JEWELRY:
		case ITEMFAMILY::HARVEST_TOOL:		
			break;

		case ITEMFAMILY::TAMING_TOOL:
			TamingTool = new CTamingTool();
			TamingTool->serial(f);
			break;
			
		case ITEMFAMILY::CRAFTING_TOOL:
			break;

		case ITEMFAMILY::RAW_MATERIAL:
			Mp = new CMP();
			Mp->serial(f);
			break;

		case ITEMFAMILY::GUILD_OPTION:
			GuildOption = new CGuildOption;
			GuildOption->serial(f);
			break;

		case ITEMFAMILY::COSMETIC:
			Cosmetics = new CCosmetics;
			Cosmetics->serial(f);
			break;

		case ITEMFAMILY::PET_ANIMAL_TICKET:
			f.serial( PetHungerCount );
			break;

		case ITEMFAMILY::FOOD:
			f.serial( Calories );
			break;

		case ITEMFAMILY::SERVICE:
			f.serialEnum(ItemServiceType);
			ItemServiceData = IItemServiceData::buildItemServiceData(ItemServiceType);
			if (ItemServiceData)
				ItemServiceData->serial(f);
			break;

		case ITEMFAMILY::CONSUMABLE:
			ConsumableItem = new CConsumable;
			ConsumableItem->serial(f);
			break;

		case ITEMFAMILY::XP_CATALYSER:
			XpCatalyser = new CXpCatalyser();
			XpCatalyser->serial(f);
			break;

		case ITEMFAMILY::COMMAND_TICKET:
			CommandTicket = new TCommandTicket;
			CommandTicket->serial(f);
			break;

		default:
			//nlwarning("<serial> For item %s, family type %d is not managed by this loader", Name.c_str(), Family );
			break;
		};
	}
	else
	{
		ItemSpecialEffects->serial(f);
		
		switch ( Family )
		{
		case ITEMFAMILY::MELEE_WEAPON:
			MeleeWeapon->serial(f);
			break;
			
		case ITEMFAMILY::RANGE_WEAPON:
			RangeWeapon->serial(f);
			break;

		case ITEMFAMILY::AMMO:
			Ammo->serial(f);
			break;

		case ITEMFAMILY::SHIELD:
			Shield->serial(f);
			break;

		case ITEMFAMILY::ARMOR:
			Armor->serial(f);
			break;

		case ITEMFAMILY::TAMING_TOOL:
			TamingTool->serial(f);
			break;

		case ITEMFAMILY::JEWELRY:
		case ITEMFAMILY::HARVEST_TOOL:		
			break;

		case ITEMFAMILY::CRAFTING_TOOL:
			break;

		case ITEMFAMILY::RAW_MATERIAL:
			Mp->serial(f);
			break;
		case ITEMFAMILY::GUILD_OPTION:
			GuildOption->serial(f);
			break;
		case ITEMFAMILY::COSMETIC:
			Cosmetics->serial(f);
			break;

		case ITEMFAMILY::PET_ANIMAL_TICKET:
			f.serial( PetHungerCount );
			break;

		case ITEMFAMILY::FOOD:
			f.serial( Calories );
			break;

		case ITEMFAMILY::SERVICE:
			f.serialEnum(ItemServiceType);
			if (ItemServiceData)
				ItemServiceData->serial(f);
			break;

		case ITEMFAMILY::CONSUMABLE:
			ConsumableItem->serial(f);
			break;

		case ITEMFAMILY::XP_CATALYSER:
			XpCatalyser->serial(f);
			break;

		case ITEMFAMILY::COMMAND_TICKET:
			CommandTicket->serial(f);
			break;
			
		default:
			//nlwarning("<CStaticItem::serial> For item %s, family type %d is not managed by this loader",Name.c_str(), Family );
			break;
		};
	}
}


//--------------------------------------------------------------
//						loadItem()  
//--------------------------------------------------------------
void CStaticItem::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const CSheetId &sheetId)
{
	if (form == NULL)
		return;

	// Get the root node, always exist
    UFormElm &root = form->getRootNode ();

	string value;

	// sheet id
	SheetId = sheetId;

	// default price (is overwritten below by other georges properties for some item families)
	root.getValueByName( ItemPrice, "basics.Price" );

	// color
	if( ! root.getValueByName ( Color, "3d.color") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value '3d.color' in sheet %s", sheetId.toString().c_str() );
	}

	// origin
	if( root.getValueByName (value, "basics.origin") )
	{
		Origin = ITEM_ORIGIN::stringToEnum( value );
	}
	else
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.origin' in sheet %s", sheetId.toString().c_str() );
	}

	// family
	if( root.getValueByName (value, "basics.family") )
	{
		Family = ITEMFAMILY::stringToItemFamily( value );
	}
	else
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.family' in sheet %s", sheetId.toString().c_str() );
	}

	// type
	if( root.getValueByName (value, "basics.ItemType") )
	{
		Type = ITEM_TYPE::stringToItemType( value );
	}
	else
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.ItemType' in sheet %s", sheetId.toString().c_str() );
	}

	// name
	if( ! root.getValueByName (Name, "basics.name") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.name' in sheet %s", sheetId.toString().c_str() );
	}

	// Sack type on ground
	if( ! root.getValueByName (Sack, "basics.sack_type") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.sack_type' in sheet %s", sheetId.toString().c_str() );
	}

	// Stackable ( 1 = non stackable, > 1 size max of stack )
	if( ! root.getValueByName (Stackable, "basics.stackable") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.stackable' in sheet %s", sheetId.toString().c_str() );
	}

	// drop sell
	if( ! root.getValueByName ( DropOrSell, "basics.Drop or Sell") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.Drop or Sell' in sheet %s", sheetId.toString().c_str() );
		DropOrSell= false;
	}

	// ShardExchangeable
	if( ! root.getValueByName ( ShardExchangeable, "basics.ShardExchangeable") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.ShardExchangeable' in sheet %s", sheetId.toString().c_str() );
		ShardExchangeable= false;
	}

	// epsilon used when converting floating bulk/weight to avoid round error
	const float epsilon = 0.1f;

	// bulk( in float in sheets, stored in an sint32 to avoid errors while rounding values
	float bulkFloat;
	if( ! root.getValueByName ( bulkFloat, "basics.Bulk") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.Bulk' in sheet %s", sheetId.toString().c_str() );
	}
	else
	{
		Bulk = uint32(bulkFloat * 1000.f + epsilon);
	}

	// weight (in float in sheet, stored in integer as gramme in sheet)
	float weightFloat;
	if(!root.getValueByName( weightFloat, "basics.Weight") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.Weight' in sheet %s", sheetId.toString().c_str() );
	}
	else
	{
		Weight = uint32(weightFloat * 1000.f + epsilon);
	}

	if( ! root.getValueByName ( TimeToEquip, "basics.Time to Equip In Ticks") )
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.Time to Equip In Ticks' in sheet %s", sheetId.toString().c_str() );
	}

	// craft plan
	if( Family == ITEMFAMILY::AMMO || Family == ITEMFAMILY::ARMOR || Family == ITEMFAMILY::JEWELRY || Family == ITEMFAMILY::MELEE_WEAPON
	 || Family == ITEMFAMILY::RANGE_WEAPON || Family == ITEMFAMILY::SHIELD )
	{
		if( ! root.getValueByName ( value, "basics.CraftPlan" ))
		{
			nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.CraftPlan' in sheet %s", sheetId.toString().c_str() );
		}
		else
		{
			if( !value.empty()) // if no craft plan we consider that this item is non craftable
			{
				CraftPlan = CSheetId( value );
				if( CraftPlan == CSheetId::Unknown )
				{
					nlwarning("<CStaticItem::readGeorges> Craftable item %s have no valid craft plan", sheetId.toString().c_str() );
				}
			}
		}
	}
	else CraftPlan = CSheetId::Unknown;

	// Seleable flag
	if( ! root.getValueByName (Saleable, "basics.Saleable") )
	{
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'basics.Saleable'");
	}

	// No rent flag
	if( ! root.getValueByName (NoRent, "basics.No Rent") )
	{
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'basics.No Rent'");
	}

	// Consumable flag
	if( ! root.getValueByName (Consumable, "basics.Consumable") )
	{
		nlwarning("<CStaticItem::readGeorges> cannot read the value 'basics.Consumable'");
	}

	if (Consumable)
	{
		if ( ! root.getValueByName (value, "Consumable.EffectPhrase") )
		{
			nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.EffectPhrase'");
		}
		else
		{
			if ( !value.empty())
			{
				EffectWhenConsumed = CSheetId( value );
				if ( EffectWhenConsumed == CSheetId::Unknown )
				{
					nlwarning("<CStaticItem::readGeorges> Given phrase ID '%s' for consumable effect is unknown", value.c_str() );
				}
			}
		}
		if( ! root.getValueByName (value, "Consumable.EffectEmote") )
		{
			nlwarning("<CStaticItem::readGeorges> cannot read the value 'Consumable.EffectEmote'");
		}
		else
		{
			if (!value.empty())
				EmoteWhenConsumed = value;
		}
	}

	// Teleport destination
	if( Family == ITEMFAMILY::TELEPORT )
	{
		root.getValueByName (Destination, "teleport.SpawnZone");
		
		if ( root.getValueByName (value, "teleport.Type") )
		{
			TpType = TELEPORT_TYPES::getTpTypeFromString(value);
		}
		if( root.getValueByName (value, "teleport.Ecosystem") )
		{
			TpEcosystem = ECOSYSTEM::stringToEcosystem( value );
		}
		if( !root.getValueByName (ItemPrice, "teleport.Tp Price") )
		{
			ItemPrice = 10000;
		}
	}

	// EquipmentSlots
	UFormElm *arrayEquipmentSlot = NULL;
	if( root.getNodeByName( &arrayEquipmentSlot, "basics.EquipmentInfo.EquipmentSlots" ) )
	{
		if( arrayEquipmentSlot )
		{
			uint size;
			nlverify( arrayEquipmentSlot->getArraySize(size) );
			Slots.resize( size );
			map< string, uint16 >::iterator it;

			for( uint i = 0; i < size; ++i )
			{
				arrayEquipmentSlot->getArrayValue( Slots[ i ], i );
				strupr( Slots[ i ] );
				
				if( SLOTTYPE::convertTypeToVisualSlot( SLOTTYPE::stringToSlotType( Slots[ i ] ) ) != SLOTTYPE::HIDDEN_SLOT )
				{
					if( SLOTTYPE::stringToSlotType( Slots[ i ] ) == SLOTTYPE::LEFT_HAND )
					{
						ItemIdSheetToModelNumberLeftHands = (uint16) CVisualSlotManager::getInstance()->sheet2Index( sheetId, SLOTTYPE::convertTypeToVisualSlot( SLOTTYPE::stringToSlotType( Slots[ i ] ) ) );
					}
					else
					{	
						ItemIdSheetToModelNumber = (uint16) CVisualSlotManager::getInstance()->sheet2Index( sheetId, SLOTTYPE::convertTypeToVisualSlot( SLOTTYPE::stringToSlotType( Slots[ i ] ) ) );
					}
				}
			}
		}
	}
	else
	{
		nlwarning( "<CStaticItem::readGeorges> can get the value 'basics.EquipmentInfo.EquipmentSlots' in sheet %s", sheetId.toString().c_str() );
	}

	// Malus for wearing equipment
	if( ! root.getValueByName( WearEquipmentMalus, "basics.EquipmentInfo.WearEquipmentMalus" ) )
	{
		nlwarning("<CStaticItem::readGeorges> can get value 'basics.EquipmentInfo.WearEquipmentMalus' in sheet %s", sheetId.toString().c_str() );
	}

	// ********************* Bag *********************
	// max weight (for animal because player have specific rules)
	// max bulk
	float f = 0.0;
	if( ! root.getValueByName (f, "bag.weight_max") )
	{
		nlwarning("<CStaticItem::readGeorges> can get the value 'bag.weight_max'");
	}
	WeightMax = (uint32)(f * 1000);
	
	if( ! root.getValueByName (f, "bag.bulk_max") )
	{
		nlwarning("<CStaticItem::readGeorges> can get the value 'bag.bulk_max'");
	}
	BulkMax = (uint32)(f * 1000);

	// slot count
	if( ! root.getValueByName (SlotCount, "bag.slot_count") )
	{
		nlwarning("<CStaticItem::readGeorges> can get the value 'bag.slot_count'");
	}

	// ********************* requirements *********************
	if ( root.getValueByName( value, "basics.RequiredSkill" ) )
	{
		RequiredSkill = SKILLS::toSkill(value);
		root.getValueByName( MinRequiredSkillLevel, "basics.MinRequiredSkillLevel" );
		root.getValueByName( RequiredSkillQualityFactor, "basics.RequiredSkillQualityFactor" );
		root.getValueByName( RequiredSkillQualityOffset, "basics.RequiredSkillQualityOffset" );
	}
	
	if ( root.getValueByName( value, "basics.RequiredSkill2" ) )
	{
		RequiredSkill2 = SKILLS::toSkill(value);
		root.getValueByName( MinRequiredSkillLevel2, "basics.MinRequiredSkillLevel2" );
		root.getValueByName( RequiredSkillQualityFactor2, "basics.RequiredSkillQualityFactor2" );
		root.getValueByName( RequiredSkillQualityOffset2, "basics.RequiredSkillQualityOffset2" );
	}
	
	if ( root.getValueByName( value, "basics.RequiredCharac" ) )
	{
		RequiredCharac = CHARACTERISTICS::toCharacteristic(value);
		root.getValueByName( MinRequiredCharacLevel, "basics.MinRequiredCharacLevel" );
		root.getValueByName( RequiredCharacQualityFactor, "basics.RequiredCharacQualityFactor" );
		root.getValueByName( RequiredCharacQualityOffset, "basics.RequiredCharacQualityOffset" );
	}

	// ********************* specific skill bonus *********************
	if ( root.getValueByName( value, "basics.Type 1" ) )
	{
		CTypeSkillMod raceMod;
		raceMod.Type = EGSPD::CClassificationType::fromString(value);
		if ( raceMod.Type != EGSPD::CClassificationType::Unknown )
		{
			root.getValueByName( raceMod.Modifier, "basics.VS Type skill modifier 1" );
			TypeSkillMods.push_back(raceMod);
		}
	}
		
	loadItemSpecialEffects( root, this, sheetId );
	
// load specific elements according to the family type
	switch ( Family )
	{
	case ITEMFAMILY::MELEE_WEAPON:
		MeleeWeapon = new SMeleeWeapon();
		loadMeleeWeapon( root, this, sheetId );
		break;
		
	case ITEMFAMILY::RANGE_WEAPON:
		RangeWeapon = new SRangeWeapon();
		loadRangeWeapon( root, this, sheetId );
		break;

	case ITEMFAMILY::AMMO:
		Ammo = new SAmmo();
		loadAmmo( root, this, sheetId );
		break;

	case ITEMFAMILY::SHIELD:
		Shield = new SShield();
		loadShield( root, this, sheetId );
		break;

	case ITEMFAMILY::ARMOR:
		Armor = new SArmor();
		loadArmor( root, this, sheetId );
		break;

	case ITEMFAMILY::HARVEST_TOOL:
		loadHarvestTool( root, this, sheetId );
		break;

	case ITEMFAMILY::CRAFTING_TOOL:
		loadFaberTool( root, this, sheetId );
		break;

	case ITEMFAMILY::TAMING_TOOL:
		loadTamingTool( root, this, sheetId );
		break;

	case ITEMFAMILY::JEWELRY:
		break;

	case ITEMFAMILY::RAW_MATERIAL:
		loadRawMaterial( root, this, sheetId );
		break;

	case ITEMFAMILY::PET_ANIMAL_TICKET:
		loadPet( root, this, sheetId );
		break;

	case ITEMFAMILY::FOOD:
		loadFood( root, this, sheetId );
		break;

	case ITEMFAMILY::GUILD_OPTION:
		loadGuildOption(root,this,sheetId);
		break;

	case ITEMFAMILY::COSMETIC:
		loadCosmetics(root,this,sheetId);
		break;

	case ITEMFAMILY::SERVICE:
		loadItemService(root,this,sheetId);
		break;

	case ITEMFAMILY::CONSUMABLE:
		loadConsumable(root,this,sheetId);
		break;

	case ITEMFAMILY::XP_CATALYSER:
		loadXpCatalyser(root,this,sheetId);
		break;

	case ITEMFAMILY::COMMAND_TICKET:
		loadCommandTicket(root,this,sheetId);
		break;
		
	default:
		//nlwarning("<CStaticItem::readGeorges> for item %s, the family type %s is not managed by this loader", Name.c_str(), family.c_str() );
		break;
	};
} // readGeorges //


/*
 * Init the 'group <--> string' mapping (static)
 */
void CMP::loadGroups( const char *definitionFile )
{
	UFormLoader *formLoader = UFormLoader::createLoader();
	NLMISC::CSmartPtr<UType> formType;
	formType = formLoader->loadFormType( definitionFile );
	if ( ! formType )
		nlerror( "Can't load %s", definitionFile );
	
	// Browse .typ file. The names are the labels, the unique numbers are the values (not the definition indices)
	string label, value;
	uint nb = formType->getNumDefinition();
	for ( uint i=0; i!=nb; ++i )
	{
		formType->getDefinition( i, label, value );
		uint groupId;
		NLMISC::fromString(value, groupId);

		// Set group -> name
		if ( groupId > 100000 )
			nlerror( "Invalid value %s in %s", value.c_str(), definitionFile );
		if ( _RMGroupNames.size() <= groupId )
			_RMGroupNames.resize( groupId + 1 );
		if ( ! _RMGroupNames[groupId].empty() )
			nlerror( "Multiple value %s found in %s", value.c_str(), definitionFile );
		_RMGroupNames[groupId] = label;

		// Set name -> group
		CRmGroupByName::const_iterator ig = _RMGroupsByName.find( label );
		if ( ig != _RMGroupsByName.end() )
			nlerror( "Multiple label %s found in %s", label.c_str(), definitionFile );
		_RMGroupsByName.insert( make_pair( label, groupId ) );
	}
	UFormLoader::releaseLoader( formLoader );
}

// ***************************************************************************
void CStaticItem::reloadSheet(const CStaticItem &o)
{
	// since Ammo etc are Ptrs, must copy wisely
	clearPtrs(true);

	// copy everything including pointers
	*this= o;

	// set ptrs to NULL, to avoid destruct in dtor
	const_cast<CStaticItem&>(o).clearPtrs(false);
}

// ***************************************************************************
float CStaticItem::getBaseWeight() const
{
	switch( Type )
	{
		// melee weapons
	case ITEM_TYPE::DAGGER:
		return CWeaponCraftParameters::DaggerWeight;
	case ITEM_TYPE::SWORD:
		return CWeaponCraftParameters::SwordWeight;
	case ITEM_TYPE::MACE:
		return CWeaponCraftParameters::MaceWeight;
	case ITEM_TYPE::AXE:
		return CWeaponCraftParameters::AxeWeight;
	case ITEM_TYPE::SPEAR:
		return CWeaponCraftParameters::SpearWeight;
	case ITEM_TYPE::STAFF:
		return CWeaponCraftParameters::StaffWeight;
	case ITEM_TYPE::MAGICIAN_STAFF:
		return CWeaponCraftParameters::MagicianStaffWeight;
	case ITEM_TYPE::TWO_HAND_SWORD:
		return CWeaponCraftParameters::TwoHandSwordWeight;
	case ITEM_TYPE::TWO_HAND_AXE:
		return CWeaponCraftParameters::TwoHandAxeWeight;
	case ITEM_TYPE::PIKE:
		return CWeaponCraftParameters::PikeWeight;
	case ITEM_TYPE::TWO_HAND_MACE:
		return CWeaponCraftParameters::TwoHandMaceWeight;
	
	// range weapon
	case ITEM_TYPE::AUTOLAUCH:
		return CWeaponCraftParameters::AutolauchWeight;
	case ITEM_TYPE::BOWRIFLE:
		return CWeaponCraftParameters::BowrifleWeight;
	case ITEM_TYPE::LAUNCHER:
		return CWeaponCraftParameters::LauncherWeight;
	case ITEM_TYPE::PISTOL:
		return CWeaponCraftParameters::PistolWeight;
	case ITEM_TYPE::BOWPISTOL:
		return CWeaponCraftParameters::BowpistolWeight;
	case ITEM_TYPE::RIFLE:
		return CWeaponCraftParameters::RifleWeight;
	
	// ammo
	case ITEM_TYPE::AUTOLAUNCH_AMMO:
		return CWeaponCraftParameters::AutolaunchAmmoWeight;
	case ITEM_TYPE::BOWRIFLE_AMMO:
		return CWeaponCraftParameters::BowrifleAmmoWeight;
	case ITEM_TYPE::LAUNCHER_AMMO:
		return CWeaponCraftParameters::LauncherAmmoWeight;
	case ITEM_TYPE::PISTOL_AMMO:
		return CWeaponCraftParameters::PistolAmmoWeight;
	case ITEM_TYPE::BOWPISTOL_AMMO:
		return CWeaponCraftParameters::BowpistolAmmoWeight;
	case ITEM_TYPE::RIFLE_AMMO:
		return CWeaponCraftParameters::RifleAmmoWeight;
	
	// armor and shield
	case ITEM_TYPE::SHIELD:
		return CWeaponCraftParameters::ShieldWeight;
	case ITEM_TYPE::BUCKLER:
		return CWeaponCraftParameters::BucklerWeight;
	case ITEM_TYPE::LIGHT_BOOTS:
		return CWeaponCraftParameters::LightBootsWeight;
	case ITEM_TYPE::LIGHT_GLOVES:
		return CWeaponCraftParameters::LightGlovesWeight;
	case ITEM_TYPE::LIGHT_PANTS:
		return CWeaponCraftParameters::LightPantsWeight;
	case ITEM_TYPE::LIGHT_SLEEVES:
		return CWeaponCraftParameters::LightSleevesWeight;
	case ITEM_TYPE::LIGHT_VEST:
		return CWeaponCraftParameters::LightVestWeight;
	case ITEM_TYPE::MEDIUM_BOOTS:
		return CWeaponCraftParameters::MediumBootsWeight;
	case ITEM_TYPE::MEDIUM_GLOVES:
		return CWeaponCraftParameters::MediumGlovesWeight;
	case ITEM_TYPE::MEDIUM_PANTS:
		return CWeaponCraftParameters::MediumPantsWeight;
	case ITEM_TYPE::MEDIUM_SLEEVES:
		return CWeaponCraftParameters::MediumSleevesWeight;
	case ITEM_TYPE::MEDIUM_VEST:
		return CWeaponCraftParameters::MediumVestWeight;
	case ITEM_TYPE::HEAVY_BOOTS:
		return CWeaponCraftParameters::HeavyBootsWeight;
	case ITEM_TYPE::HEAVY_GLOVES:
		return CWeaponCraftParameters::HeavyGlovesWeight;
	case ITEM_TYPE::HEAVY_PANTS:
		return CWeaponCraftParameters::HeavyPantsWeight;
	case ITEM_TYPE::HEAVY_SLEEVES:
		return CWeaponCraftParameters::HeavySleevesWeight;
	case ITEM_TYPE::HEAVY_VEST:
		return CWeaponCraftParameters::HeavyVestWeight;
	case ITEM_TYPE::HEAVY_HELMET:
		return CWeaponCraftParameters::HeavyHelmetWeight;
	
	// jewel
	case ITEM_TYPE::ANKLET:
		return CWeaponCraftParameters::AnkletWeight;
	case ITEM_TYPE::BRACELET:
		return CWeaponCraftParameters::BraceletWeight;
	case ITEM_TYPE::DIADEM:
		return CWeaponCraftParameters::DiademWeight;
	case ITEM_TYPE::EARING:
		return CWeaponCraftParameters::EaringWeight;
	case ITEM_TYPE::PENDANT:
		return CWeaponCraftParameters::PendantWeight;
	case ITEM_TYPE::RING:
		return CWeaponCraftParameters::RingWeight;
	default:
		return 0;
	}
}

// ***************************************************************************
void CStaticItem::clearPtrs(bool doDelete)
{
	if(doDelete)
	{
		if (Ammo != NULL )	delete Ammo;
		if (Armor != NULL )	delete Armor;
		if (MeleeWeapon != NULL )	delete MeleeWeapon;
		if (RangeWeapon != NULL )	delete RangeWeapon;
		if (Cosmetics != NULL )	delete Cosmetics;
		if (Mp != NULL )	delete Mp;
		if (GuildOption != NULL )	delete GuildOption;
		if (Shield != NULL )	delete Shield;
		if (TamingTool != NULL)	delete TamingTool;
		if (ItemServiceData != NULL)	delete ItemServiceData;
		if (CommandTicket != NULL)	delete CommandTicket;
	}

	Ammo = NULL;
	Armor = NULL;
	MeleeWeapon = NULL;
	RangeWeapon = NULL;
	Cosmetics = NULL;
	Mp = NULL;
	GuildOption = NULL;
	Shield = NULL;
	TamingTool = NULL;
	ItemServiceData = NULL;
	CommandTicket = NULL;
}


uint32 CStaticItem::getMaxStackSize() const
{
	if (Stackable == 1)
		return 1;

	return 999; // NEW LIMIT
}
