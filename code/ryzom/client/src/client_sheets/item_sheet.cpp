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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Georges
#include "nel/georges/u_form_elm.h"
// Client.
#include "item_sheet.h"

#include "game_share/characteristics.h"
#include "game_share/scores.h"
#include "game_share/skills.h"
#include "game_share/people.h"
#include "game_share/protection_type.h"

#include "nel/misc/i18n.h"

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


// ***************************************************************************
// Easy Macro to translate .typ enum
#define	TRANSLATE_ENUM( _Var_, _unknown_, _func_, _key_)	\
	_Var_ = _unknown_;										\
	if( !item.getValueByName(val, _key_))					\
		debug( toString("Key '%s' not found.", _key_) );	\
	else if( (_Var_ = _func_(val)) == _unknown_ )			\
		debug(#_Var_ " Unknown: " + val);

// Same but no error if the result of enum is _unknown_
#define	TRANSLATE_ENUM_NODB( _Var_, _unknown_, _func_, _key_)	\
	_Var_ = _unknown_;										\
	if( !item.getValueByName(val, _key_))					\
		debug( toString("Key '%s' not found.", _key_) );	\
	else													\
		_Var_ = _func_(val);


// Easy macro to translate value from georges
#define TRANSLATE_VAL( _Var_, _key_ )						\
	if(!item.getValueByName(_Var_, _key_))					\
		debug( toString("Key '%s' not found.", _key_) );	\


// ***************************************************************************
//-----------------------------------------------
// CItemSheet :
// Constructor.
//-----------------------------------------------
CItemSheet::CItemSheet()
{
	IdShape = 0;
	IdShapeFemale = 0;
	MapVariant = 0;
	ItemType = ITEM_TYPE::UNDEFINED;
	Family = ITEMFAMILY::UNDEFINED;
	SlotBF= 0;
	IdIconBack = 0;
	IdIconMain = 0;
	IdIconOver = 0;
	IdIconOver2 = 0;
	IdIconText = 0;
	IdAnimSet = 0;
	Color = 0;
	HasFx = false;
	DropOrSell = false;
	IsItemNoRent = false;
	Stackable = 0;
	IsConsumable = false;

	IdEffect1 = 0;
	IdEffect2 = 0;
	IdEffect3 = 0;
	IdEffect4 = 0;

	Type = CEntitySheet::ITEM;
	Bulk= 0.f;
	EquipTime= 0;
	NeverHideWhenEquipped = false;
	RequiredCharac = CHARACTERISTICS::Unknown;
	RequiredCharacLevel = 0;
	RequiredSkill = SKILLS::unknown;
	RequiredSkillLevel = 0;
	IconColor= NLMISC::CRGBA::White;
	IconBackColor= NLMISC::CRGBA::White;
	IconOverColor= NLMISC::CRGBA::White;
	IconOver2Color= NLMISC::CRGBA::White;

	ItemOrigin = ITEM_ORIGIN::UNKNOWN;

	Cosmetic.VPValue = 0;
	Cosmetic.Gender = GSGENDER::unknown;

	Armor.ArmorType = ARMORTYPE::UNKNOWN;

	MeleeWeapon.WeaponType = WEAPONTYPE::UNKNOWN;
	MeleeWeapon.Skill = SKILLS::unknown;
	MeleeWeapon.DamageType = DMGTYPE::UNDEFINED;
	MeleeWeapon.MeleeRange = 0;

	RangeWeapon.WeaponType = WEAPONTYPE::UNKNOWN;
	RangeWeapon.RangeWeaponType = RANGE_WEAPON_TYPE::Unknown;
	RangeWeapon.Skill = SKILLS::unknown;

	Ammo.Skill = SKILLS::unknown;
	Ammo.DamageType = DMGTYPE::UNDEFINED;
	Ammo.Magazine = 0;

	Mp.Ecosystem = ECOSYSTEM::unknown;
	Mp.MpCategory = MP_CATEGORY::Undefined;
	Mp.HarvestSkill = SKILLS::unknown;
	Mp.Family = RM_FAMILY::Unknown;
	Mp.UsedAsCraftRequirement = false;
	Mp.MpColor = 0;
	Mp.StatEnergy = 0;
	Mp.ItemPartBF = 0;

	Shield.ShieldType = SHIELDTYPE::NONE;

	Tool.Skill = SKILLS::unknown;
	Tool.CraftingToolType = TOOL_TYPE::Unknown;
	Tool.CommandRange = 0;
	Tool.MaxDonkey = 0;

	GuildOption.MoneyCost = 0;
	GuildOption.XPCost = 0;

	Pet.Slot = 0;

	Teleport.Type = TELEPORT_TYPES::NONE;
}// CItemSheet //


//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CItemSheet::build(const NLGEORGES::UFormElm &item)
{
	// Load the name.
	string Shape;
	if(!item.getValueByName(Shape, "3d.shape"))
		debug("key '3d.shape' not found.");
	IdShape = ClientSheetsStrings.add(Shape);

	// Load the name.
	string ShapeFemale;
	if(!item.getValueByName(ShapeFemale, "3d.shape_female"))
		debug("key '3d.shape_female' not found.");
	IdShapeFemale = ClientSheetsStrings.add(ShapeFemale);

	// Get the icon associated.
	string IconMain;
	if(!item.getValueByName (IconMain, "3d.icon"))
		debug("key '3d.icon' not found.");
	IconMain = strlwr (IconMain);
	IdIconMain = ClientSheetsStrings.add(IconMain);

	// Get the icon associated.
	string IconBack;
	if(!item.getValueByName (IconBack, "3d.icon background"))
		debug("key '3d.icon background' not found.");
	IconBack = strlwr (IconBack);
	IdIconBack = ClientSheetsStrings.add(IconBack);

	// Get the icon associated.
	string IconOver;
	if(!item.getValueByName (IconOver, "3d.icon overlay"))
		debug("key '3d.icon overlay' not found.");
	IconOver = strlwr (IconOver);
	IdIconOver = ClientSheetsStrings.add(IconOver);

	// Get the icon associated.
	string IconOver2;
	if(!item.getValueByName (IconOver2, "3d.icon overlay2"))
		debug("key '3d.icon overlay2' not found.");
	IconOver2 = strlwr (IconOver2);
	IdIconOver2 = ClientSheetsStrings.add(IconOver2);

	// Get Special modulate colors
	item.getValueByName (IconColor, "3d.IconColor" );
	item.getValueByName (IconBackColor, "3d.IconBackColor");
	item.getValueByName (IconOverColor, "3d.IconOverColor");
	item.getValueByName (IconOver2Color, "3d.IconOver2Color");

	// Get the icon text associated.
	string IconText;
	if(!item.getValueByName (IconText, "3d.text overlay"))
		debug("key '3d.text overlay' not found.");
	IconText = strlwr (IconText);
	IdIconText = ClientSheetsStrings.add(IconText);

	// See if this item can be hiden when equipped
	if(!item.getValueByName (NeverHideWhenEquipped, "3d.never hide when equiped"))
		debug("key '3d.never hide when equiped.");

	// Load the different slot in wicth the item can be equipped.
	const UFormElm *pElt = 0;
	// check uint32 is OK!
	nlassert( SLOTTYPE::NB_SLOT_TYPE <= 32 );
	SlotBF= 0;
	if(item.getNodeByName(&pElt, "basics.EquipmentInfo.EquipmentSlots") && pElt)
	{
		// Get all slots.
		uint size;
		if(pElt->getArraySize(size))
		{
			for(uint i = 0; i < size; ++i)
			{
				string slotName;
				if(pElt->getArrayValue(slotName, i))
				{
					// Check name.
					if(slotName.empty())
						debug(toString("The slot name %d is Empty.", i));

					// Push the possible slots for the item in the list.
					SlotBF|= SINT64_CONSTANT(1)<< (SLOTTYPE::stringToSlotType(NLMISC::toUpper(slotName)));
				}
			}
		}
		else
			debug("The element 'basics.Equipment Slot' is not an array.");
	}
	else
		debug("Cannot create the element from the name 'basics.Equipment Slot'.");

	// Get the Item Family.
	string family;
	if(!item.getValueByName(family, "basics.family"))
	{
		debug("Key 'basics.family' not found.");
		Family = ITEMFAMILY::UNDEFINED;
	}
	else
	{
		Family = (ITEMFAMILY::EItemFamily) ITEMFAMILY::stringToItemFamily(NLMISC::toUpper( family) );
		if(Family == ITEMFAMILY::UNDEFINED)
			debug("Item Family Undefined.");
	}

	// Get the Item Type.
	string itemtype;
	if(!item.getValueByName(itemtype, "basics.ItemType"))
	{
		debug("Key 'basics.ItemType' not found.");
		ItemType = ITEM_TYPE::UNDEFINED;
	}
	else
	{
		ItemType = (ITEM_TYPE::TItemType) ITEM_TYPE::stringToItemType(NLMISC::toUpper(itemtype) );
		if (ItemType == ITEM_TYPE::UNDEFINED)
			debug("Item Type Undefined.");
	}

	// Get the DropOrSell property
	if(!item.getValueByName (DropOrSell, "basics.Drop or Sell"))
		debug("key 'basics.Drop or Sell' not found.");

	// Get the IsItemNoRent property
	if(!item.getValueByName (IsItemNoRent, "basics.No Rent"))
		debug("key 'basics.No Rent' not found.");

	// Get the stackable property
	if(!item.getValueByName (Stackable, "basics.stackable"))
		debug("key 'basics.stackable' not found.");

	// Get the Consumable property
	if(!item.getValueByName (IsConsumable, "basics.Consumable"))
		debug("key 'basics.Consumable' not found.");

	// Get the texture variante.
	if(!item.getValueByName(MapVariant, "3d.map_variant"))
		debug("Key '3d.map_variant' not found.");

	// Load the name.
	string AnimSet;
	if(!item.getValueByName(AnimSet, "3d.anim_set"))
		debug("key '3d.anim_set' not found.");
	// Force the CASE in UPPER to not be CASE SENSITIVE.
	else
		NLMISC::strlwr(AnimSet);
	IdAnimSet = ClientSheetsStrings.add(AnimSet);

	// Get the Trail Shape
	if(!item.getValueByName(Color, "3d.color"))
		debug("key '3d.color' not found.");

	// Get the Fx flag
	if(!item.getValueByName(HasFx, "3d.has_fx"))
		debug("key '3d.has_fx' not found.");

	// Get special Effect1
	string Effect1;
	if(!item.getValueByName(Effect1, "Effects.Effect1"))
		debug("key 'Effects.Effect1' not found.");
	Effect1 = strlwr(Effect1);
	IdEffect1 = ClientSheetsStrings.add(Effect1);

	// Get special Effect2
	string Effect2;
	if(!item.getValueByName(Effect2, "Effects.Effect2"))
		debug("key 'Effects.Effect2' not found.");
	Effect2 = strlwr(Effect2);
	IdEffect2 = ClientSheetsStrings.add(Effect2);

	// Get special Effect3
	string Effect3;
	if(!item.getValueByName(Effect3, "Effects.Effect3"))
		debug("key 'Effects.Effect3' not found.");
	Effect3 = strlwr(Effect3);
	IdEffect3 = ClientSheetsStrings.add(Effect3);

	// Get special Effect4
	string Effect4;
	if(!item.getValueByName(Effect4, "Effects.Effect4"))
		debug("key 'Effects.Effect4' not found.");
	Effect4 = strlwr(Effect4);
	IdEffect4 = ClientSheetsStrings.add(Effect4);

	// Get its bulk
	TRANSLATE_VAL( Bulk, "basics.Bulk" );

	// Get its equip time
	TRANSLATE_VAL( EquipTime, "basics.Time to Equip In Ticks" );

	// build fx part
	FX.build(item, "3d.fx.");

	// **** Build Help Infos
	string	val;

	TRANSLATE_ENUM( RequiredCharac, CHARACTERISTICS::Unknown, CHARACTERISTICS::toCharacteristic, "basics.RequiredCharac");
	TRANSLATE_VAL( RequiredCharacLevel, "basics.MinRequiredCharacLevel");
	TRANSLATE_ENUM( RequiredSkill, SKILLS::unknown, SKILLS::toSkill, "basics.RequiredSkill");
	TRANSLATE_VAL( RequiredSkillLevel, "basics.MinRequiredSkillLevel");

	// item Origin
	TRANSLATE_ENUM ( ItemOrigin, ITEM_ORIGIN::UNKNOWN, ITEM_ORIGIN::stringToEnum, "basics.origin");

	/// item craft plan
	TRANSLATE_VAL( val, "basics.CraftPlan" );
	if (!val.empty())
		CraftPlan = CSheetId(val);

	// Special according to Family;
	switch(Family)
	{
	// COSMETIC
	case ITEMFAMILY::COSMETIC :
		{
			string sheetName = Id.toString();
			string::size_type pos = sheetName.find('.',0);
			if (pos == string::npos)
				nlwarning("<loadCosmetics> Can't load the VPValue from sheet name in sheet %s", Id.toString().c_str() );
			else
			{
				sint i = (sint)pos - 1;
				for(; i >= 0; i-- )
				{
					if ( !isdigit( sheetName[i] ) )
						break;
				}
				if ( i >= -1 )
				{
					string val = sheetName.substr( i + 1, pos - i - 1);
					NLMISC::fromString( val, Cosmetic.VPValue );
				}
			}

			if ( sheetName.find( "hof" ) != string::npos )
				Cosmetic.Gender = GSGENDER::female;
			else
				Cosmetic.Gender = GSGENDER::male;



		}
		break;
	// ARMOR
	case ITEMFAMILY::ARMOR :
		{
		// ArmorType
		TRANSLATE_ENUM ( Armor.ArmorType, ARMORTYPE::UNKNOWN, ARMORTYPE::toArmorType, "armor.Armor category" );
		}
		break;
	// MELEE_WEAPON
	case ITEMFAMILY::MELEE_WEAPON :
		{
		// WeaponType
		TRANSLATE_ENUM ( MeleeWeapon.WeaponType, WEAPONTYPE::UNKNOWN, WEAPONTYPE::stringToWeaponType, "melee weapon.category" );

		// Skill
		TRANSLATE_ENUM ( MeleeWeapon.Skill, SKILLS::unknown, SKILLS::toSkill, "melee weapon.skill" );

		// DamageType
		TRANSLATE_ENUM ( MeleeWeapon.DamageType, DMGTYPE::UNDEFINED, DMGTYPE::stringToDamageType, "melee weapon.damage type" );

		// DamageType
		TRANSLATE_VAL ( MeleeWeapon.MeleeRange, "melee weapon.melee range" );

		}
		break;
	// RANGE_WEAPON
	case ITEMFAMILY::RANGE_WEAPON :
		{
		// WeaponType
		TRANSLATE_ENUM ( RangeWeapon.WeaponType, WEAPONTYPE::UNKNOWN, WEAPONTYPE::stringToWeaponType, "range weapon.category" );

		// Range weapon type
		TRANSLATE_ENUM ( RangeWeapon.RangeWeaponType, RANGE_WEAPON_TYPE::Generic, RANGE_WEAPON_TYPE::stringToRangeWeaponType, "range weapon.RangeWeaponType" );

		// Skill
		TRANSLATE_ENUM ( RangeWeapon.Skill, SKILLS::unknown, SKILLS::toSkill, "range weapon.skill" );

		}
		break;
	// AMMO
	case ITEMFAMILY::AMMO :
		{
		// Skill
		TRANSLATE_ENUM ( Ammo.Skill, SKILLS::unknown, SKILLS::toSkill, "ammo.weapon type" );

		// DamageType
		TRANSLATE_ENUM ( Ammo.DamageType, DMGTYPE::UNDEFINED, DMGTYPE::stringToDamageType, "ammo.damage type" );

		// Magazine
		TRANSLATE_VAL( Ammo.Magazine, "ammo.magazine" );

		}
		break;
	// RAW_MATERIAL
	case ITEMFAMILY::RAW_MATERIAL :
		{
		// Ecosystem
		TRANSLATE_ENUM( Mp.Ecosystem, ECOSYSTEM::unknown, ECOSYSTEM::stringToEcosystem, "mp.Ecosystem" );

		// MpCategory
		TRANSLATE_ENUM( Mp.MpCategory, MP_CATEGORY::Undefined, MP_CATEGORY::stringToMPCategory, "mp.Category" );

		// Skill
		TRANSLATE_ENUM( Mp.HarvestSkill, SKILLS::unknown, SKILLS::toSkill, "mp.HarvestSkill" );

		// MP Family
		TRANSLATE_VAL( Mp.Family, "mp.Family" );


		// Faber Item Part
		uint	i;
		char	keyTmp[256];
		// ensure that if you modify RM_FABER_TYPE, you have to rebuild the item sheets.
		nlctassert(RM_FABER_TYPE::NUM_FABER_TYPE == 26);
		// ensure that the bitfields are enough (nb: unknown can be stored)
		nlctassert(ITEM_ORIGIN::NUM_ITEM_ORIGIN < 256);
		// ensure that the bitfield for item part buildable for this MP is possible
		nlctassert(RM_FABER_TYPE::NUM_FABER_TYPE <= 32);
		// reset
		Mp.ItemPartBF= 0;
		MpItemParts.clear();
		// check if ok for each
		for(i=0;i<RM_FABER_TYPE::NUM_FABER_TYPE ;i++)
		{
			uint32	durability= 0;
			string	sheetEntry= RM_FABER_TYPE::faberTypeToSheetEntry((RM_FABER_TYPE::TRMFType)i);

			// read the associated durablity  of the MP faberType
			sprintf(keyTmp, "mp.MpParam.%s.Durability", sheetEntry.c_str());
			TRANSLATE_VAL(durability, keyTmp);

			// If not null, ok this MP is associated to this faberType
			if(durability)
			{
				Mp.ItemPartBF |= SINT64_CONSTANT(1)<<i;
				MpItemParts.push_back(CMpItemPart());
				CMpItemPart		&itemPart= MpItemParts.back();

				// read origin filter
				sprintf(keyTmp, "mp.MpParam.%s.CraftCivSpec", sheetEntry.c_str());
				TRANSLATE_ENUM( itemPart.OriginFilter, ITEM_ORIGIN::UNKNOWN, ITEM_ORIGIN::stringToEnum, keyTmp);

				// read each stat
				for(uint j=0;j<RM_FABER_STAT_TYPE::NumRMStatType;j++)
				{
					sprintf(keyTmp, "mp.MpParam.%s.%s", sheetEntry.c_str(), RM_FABER_STAT_TYPE::toString((RM_FABER_STAT_TYPE::TRMStatType)j).c_str());
					TRANSLATE_VAL( itemPart.Stats[j], keyTmp);
				}
			}
		}

		// UsedAsCraftRequirement
		TRANSLATE_VAL( Mp.UsedAsCraftRequirement, "mp.UsedAsCraftRequirement" );

		// MpColor
		TRANSLATE_VAL( Mp.MpColor, "mp.MpColor" );

		// Mp Stat Energy
		TRANSLATE_VAL( Mp.StatEnergy, "mp.StatEnergy");

		}
		break;
	// SHIELD
	case ITEMFAMILY::SHIELD :
		{
		// ShieldType
		TRANSLATE_ENUM( Shield.ShieldType, SHIELDTYPE::NONE, SHIELDTYPE::stringToShieldType, "shield.Category" );
		}
		break;
	// TOOL: different for any tool
	case ITEMFAMILY::CRAFTING_TOOL :
		{
		// CraftingToolType
		TRANSLATE_ENUM( Tool.CraftingToolType, TOOL_TYPE::Unknown, TOOL_TYPE::toToolType, "crafting tool.type");
		}
		break;
	case ITEMFAMILY::HARVEST_TOOL :
		{
		// Skill
		TRANSLATE_ENUM( Tool.Skill, SKILLS::unknown, SKILLS::toSkill, "harvest tool.skill" );
		}
		break;
	case ITEMFAMILY::TAMING_TOOL :
		{
		// Skill
		TRANSLATE_ENUM( Tool.Skill, SKILLS::unknown, SKILLS::toSkill, "taming tool.skill" );

		// CommandRange
		TRANSLATE_VAL( Tool.CommandRange, "taming tool.command range" );

		// MaxDonkey
		TRANSLATE_VAL( Tool.MaxDonkey, "taming tool.max donkey" );

		}
		break;
	case ITEMFAMILY::GUILD_OPTION :
		{
		// Cost in money of the tool
		TRANSLATE_VAL( GuildOption.MoneyCost, "guild option.Money Cost" );

		// Cost in guild XP
		TRANSLATE_VAL( GuildOption.XPCost, "guild option.Guild XP Cost" );
		}
		break;
	case ITEMFAMILY::PET_ANIMAL_TICKET :
		{
		// Cost in money of the tool
		TRANSLATE_VAL( Pet.Slot, "pet.Pet Slot" );
		}
		break;
	case ITEMFAMILY::TELEPORT:
		{
		// Type of teleport
		TRANSLATE_ENUM( Teleport.Type, TELEPORT_TYPES::NONE, TELEPORT_TYPES::getTpTypeFromString, "teleport.Type" );
		}
		break;
	case ITEMFAMILY::SCROLL:
		{
		// Scroll texture
		TRANSLATE_VAL( Scroll.Texture, "Scroll.Texture");
		}
		break;
	case ITEMFAMILY::CONSUMABLE:
		{
			TRANSLATE_VAL( Consumable.ConsumptionTime, "Consumable.ConsumptionTime");
			TRANSLATE_VAL( Consumable.OverdoseTimer, "Consumable.OverdoseTimer");

			Consumable.Properties.clear();
			for(uint i=0;i<4;i++)
			{
				string	val;
				item.getValueByName(val, toString("Consumable.Property %d", i).c_str() );
				if(!val.empty() && val!="NULL")
				{
					Consumable.Properties.push_back(val);
				}
			}
		}
		break;

	default:
		break;
	};

}// build //


//-----------------------------------------------
// serial :
// Serialize character sheet into binary data file.
//-----------------------------------------------
void CItemSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	ClientSheetsStrings.serial(f, IdShape);
	ClientSheetsStrings.serial(f, IdShapeFemale);
	f.serial(SlotBF);			// Serialize Slots used.
	f.serial(MapVariant);		// Serialize Map Variant.
	f.serialEnum(Family);		// Serialize Family.
	f.serialEnum(ItemType);		// Serialize ItemType.
	ClientSheetsStrings.serial(f, IdIconMain);
	ClientSheetsStrings.serial(f, IdIconBack);
	ClientSheetsStrings.serial(f, IdIconOver);
	ClientSheetsStrings.serial(f, IdIconOver2);
	f.serial (IconColor);
	f.serial (IconBackColor);
	f.serial (IconOverColor);
	f.serial (IconOver2Color);
	ClientSheetsStrings.serial(f, IdIconText);
	ClientSheetsStrings.serial(f, IdAnimSet);
	f.serial(Color);			// Serialize the item color.
	f.serial(HasFx);			// Serialize the has fx.
	f.serial(DropOrSell);
	f.serial(IsItemNoRent);
	f.serial(NeverHideWhenEquipped);
	f.serial(Stackable);
	f.serial(IsConsumable);
	f.serial(Bulk);
	f.serial(EquipTime);

	f.serial(FX);

	ClientSheetsStrings.serial(f, IdEffect1);
	ClientSheetsStrings.serial(f, IdEffect2);
	ClientSheetsStrings.serial(f, IdEffect3);
	ClientSheetsStrings.serial(f, IdEffect4);

	f.serialCont(MpItemParts);

	f.serial(CraftPlan);

	f.serialEnum(RequiredCharac);
	f.serial(RequiredCharacLevel);
	f.serialEnum(RequiredSkill);
	f.serial(RequiredSkillLevel);

	// **** Serial Help Infos
	f.serialEnum(ItemOrigin);

	// Different Serial according to family
	switch(Family)
	{
	case ITEMFAMILY::COSMETIC :
		f.serial(Cosmetic);
		break;
	case ITEMFAMILY::ARMOR :
		f.serial(Armor);
		break;
	case ITEMFAMILY::MELEE_WEAPON :
		f.serial(MeleeWeapon);
		break;
	case ITEMFAMILY::RANGE_WEAPON :
		f.serial(RangeWeapon);
		break;
	case ITEMFAMILY::AMMO :
		f.serial(Ammo);
		break;
	case ITEMFAMILY::RAW_MATERIAL :
		f.serial(Mp);
		break;
	case ITEMFAMILY::SHIELD :
		f.serial(Shield);
		break;
	// Same for any tool
	case ITEMFAMILY::CRAFTING_TOOL :
	case ITEMFAMILY::HARVEST_TOOL :
	case ITEMFAMILY::TAMING_TOOL :
		f.serial(Tool);
		break;
	case ITEMFAMILY::GUILD_OPTION :
		f.serial(GuildOption);
		break;
	case ITEMFAMILY::PET_ANIMAL_TICKET :
		f.serial(Pet);
		break;
	case ITEMFAMILY::TELEPORT:
		f.serial(Teleport);
		break;
	case ITEMFAMILY::SCROLL:
		f.serial(Scroll);
		break;
	case ITEMFAMILY::CONSUMABLE:
		f.serial(Consumable);
		break;
	default:
		break;
	};

}// serial //


// ***************************************************************************
bool		CItemSheet::isFaberisable() const
{
	// Only those family of item can be repaired/faber/refined.
	return  Family==ITEMFAMILY::AMMO ||
			Family==ITEMFAMILY::ARMOR ||
			Family==ITEMFAMILY::MELEE_WEAPON ||
			Family==ITEMFAMILY::RANGE_WEAPON ||
			Family==ITEMFAMILY::JEWELRY ||
			Family==ITEMFAMILY::CRAFTING_TOOL ||
			Family==ITEMFAMILY::HARVEST_TOOL ||
			Family==ITEMFAMILY::TAMING_TOOL ||
			Family==ITEMFAMILY::SHIELD;
}

// ***************************************************************************
SKILLS::ESkills CItemSheet::getRequiredSkill() const
{
	switch(Family)
	{
//		case ITEMFAMILY::ARMOR:			return Armor.Skill;
		case ITEMFAMILY::MELEE_WEAPON:	return MeleeWeapon.Skill;
		case ITEMFAMILY::RANGE_WEAPON:  return RangeWeapon.Skill;
		case ITEMFAMILY::AMMO:			return Ammo.Skill;
//		case ITEMFAMILY::SHIELD:		return SHIELDTYPE::shieldTypeToSkill(Shield.ShieldType);
		case ITEMFAMILY::RAW_MATERIAL:  return Mp.HarvestSkill;
		//
		case ITEMFAMILY::HARVEST_TOOL:
		case ITEMFAMILY::TAMING_TOOL:
			return Tool.Skill;
		case ITEMFAMILY::CRAFTING_TOOL:
			return SKILLS::SC;
		default: return SKILLS::unknown;
	}
}

// ***************************************************************************
bool	CItemSheet::isUsedAsCraftRequirement() const
{
	if(Family!=ITEMFAMILY::RAW_MATERIAL)
		return false;

	return Mp.UsedAsCraftRequirement;
}

// ***************************************************************************
bool	CItemSheet::canBuildSomeItemPart() const
{
	if(Family!=ITEMFAMILY::RAW_MATERIAL)
		return false;

	return Mp.ItemPartBF!=0;
}

// ***************************************************************************
bool	CItemSheet::canBuildItemPart(RM_FABER_TYPE::TRMFType e) const
{
	if(e<RM_FABER_TYPE::NUM_FABER_TYPE)
	{
		if(Mp.ItemPartBF&(SINT64_CONSTANT(1)<<e))
			return true;
	}

	// all other cases: false
	return false;
}

// ***************************************************************************
bool	CItemSheet::canBuildItemPart(RM_FABER_TYPE::TRMFType e, ITEM_ORIGIN::EItemOrigin origin) const
{
	if(e<RM_FABER_TYPE::NUM_FABER_TYPE)
	{
		if(Mp.ItemPartBF&(SINT64_CONSTANT(1)<<e))
		{
			const CMpItemPart		&itemPart= getItemPart(e);
			// If this MP can build all origin items, or if origin matchs
			if( itemPart.OriginFilter == ITEM_ORIGIN::COMMON ||
				itemPart.OriginFilter == origin ||
				origin == ITEM_ORIGIN::COMMON )
				return true;
		}
	}

	// all other cases: false
	return false;
}

// ***************************************************************************
const CItemSheet::CMpItemPart	&CItemSheet::getItemPart(RM_FABER_TYPE::TRMFType e) const
{
	nlassert(Mp.ItemPartBF&(SINT64_CONSTANT(1)<<e));

	// count the number of bits set before reaching this item part
	uint	index= 0;
	for(uint i=0;i<(uint)e;i++)
	{
		if(Mp.ItemPartBF&(SINT64_CONSTANT(1)<<i))
			index++;
	}

	nlassert(index<MpItemParts.size());
	return MpItemParts[index];
}

// ***************************************************************************
bool	CItemSheet::hasCharacRequirement(uint itemLevel, CHARACTERISTICS::TCharacteristics	&caracType, float &caracValue) const
{

	switch( Family )
	{
	// **** ARMORS / BUCKLERS
	case ITEMFAMILY::ARMOR:
	case ITEMFAMILY::SHIELD:
		switch( ItemType )
		{
		case ITEM_TYPE::LIGHT_BOOTS:
		case ITEM_TYPE::LIGHT_GLOVES:
		case ITEM_TYPE::LIGHT_PANTS:
		case ITEM_TYPE::LIGHT_SLEEVES:
		case ITEM_TYPE::LIGHT_VEST:
			// No carac requirement
			return false;
		case ITEM_TYPE::MEDIUM_BOOTS:
		case ITEM_TYPE::MEDIUM_GLOVES:
		case ITEM_TYPE::MEDIUM_PANTS:
		case ITEM_TYPE::MEDIUM_SLEEVES:
		case ITEM_TYPE::MEDIUM_VEST:
		case ITEM_TYPE::BUCKLER:
			// Constitution requirement
			caracType= CHARACTERISTICS::constitution;
			caracValue= itemLevel / 1.5f;
			return true;
		case ITEM_TYPE::HEAVY_BOOTS:
		case ITEM_TYPE::HEAVY_GLOVES:
		case ITEM_TYPE::HEAVY_PANTS:
		case ITEM_TYPE::HEAVY_SLEEVES:
		case ITEM_TYPE::HEAVY_VEST:
		case ITEM_TYPE::HEAVY_HELMET:
		case ITEM_TYPE::SHIELD:
			// Constitution requirement
			caracType= CHARACTERISTICS::constitution;
			caracValue= float((sint)itemLevel - 10);
			caracValue= max(caracValue, 0.f);
			return true;
		default:
			// No carac requirement
			return false;
		}
		break;

	// **** MELEE_WEAPONS
	case ITEMFAMILY::MELEE_WEAPON:
		switch( ItemType )
		{
		case ITEM_TYPE::MAGICIAN_STAFF:
			// Intelligence requirement
			caracType= CHARACTERISTICS::intelligence;
			caracValue= float((sint)itemLevel - 10);
			caracValue= max(caracValue, 0.f);
			return true;
		default:
			// Strength requirement
			caracType= CHARACTERISTICS::strength;
			caracValue= float((sint)itemLevel - 10);
			caracValue= max(caracValue, 0.f);
			return true;
		}
		break;

	// **** RANGE_WEAPON
	case ITEMFAMILY::RANGE_WEAPON:
		caracType= CHARACTERISTICS::well_balanced;
		caracValue= float((sint)itemLevel - 10);
		caracValue= max(caracValue, 0.f);
		return true;

	// **** OTHERS
	default:
		// No carac requirement
		return false;
	}
}


// ***************************************************************************
bool	CItemSheet::canExchangeOrGive(bool botChatGift) const
{
	// DropOrSell => ok
	if(DropOrSell)
		return true;

	// still can give any item to bot chat
	return botChatGift;
}

// ***************************************************************************
void	CItemSheet::getItemPartListAsText(ucstring &ipList) const
{
	bool	all= true;
	for(uint i=0;i<RM_FABER_TYPE::NUM_FABER_TYPE;i++)
	{
		RM_FABER_TYPE::TRMFType		faberType= RM_FABER_TYPE::TRMFType(i);

		if(canBuildItemPart(faberType))
		{
			if(!ipList.empty())
				ipList+= ", ";
			ipList+= RM_FABER_TYPE::toLocalString(faberType);
		}
		else
		{
			// Ignore Tools and CampFire
			if(	faberType!=RM_FABER_TYPE::MPBT &&
				faberType!=RM_FABER_TYPE::MPPES &&
				faberType!=RM_FABER_TYPE::MPSH &&
				faberType!=RM_FABER_TYPE::MPTK &&
				faberType!=RM_FABER_TYPE::MPJH &&
				faberType!=RM_FABER_TYPE::MPCF )
				all= false;
		}
	}
	if(all)
	{
		ipList= CI18N::get("uihelpItemMPAllCraft");
	}
}
