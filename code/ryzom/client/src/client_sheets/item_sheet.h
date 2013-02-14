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




#ifndef CL_ITEM_SHEET_H
#define CL_ITEM_SHEET_H


/////////////
// INCLUDE //
/////////////
#include "client_sheets.h"
// misc
#include "nel/misc/types_nl.h"
// client
#include "entity_sheet.h"
#include "item_fx_sheet.h"
// Game share
#include "game_share/slot_types.h"
#include "game_share/item_family.h"
#include "game_share/item_type.h"
#include "game_share/skills.h"
#include "game_share/armor_types.h"
#include "game_share/weapon_types.h"
#include "game_share/damage_types.h"
#include "game_share/ecosystem.h"
#include "game_share/mp_category.h"
#include "game_share/item_origin.h"
#include "game_share/shield_types.h"
#include "game_share/crafting_tool_type.h"
#include "game_share/rm_family.h"
#include "game_share/range_weapon_type.h"
#include "game_share/characteristics.h"
#include "game_share/teleport_types.h"
#include "game_share/gender.h"
#include "game_share/characteristics.h"
// std
#include <string>


///////////
// USING //
///////////


///////////
// CLASS //
///////////
namespace NLGEORGES
{
	class UFormElm;
	class UFormLoader;
}


// ***************************************************************************
/**
 * Class to manage an item sheet.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CItemSheet : public CEntitySheet
{
public:
	struct	CCosmetic
	{
		uint32	VPValue;
		GSGENDER::EGender Gender;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(VPValue);
			f.serialEnum(Gender);
		}
	};
	struct	CArmor
	{
		ARMORTYPE::EArmorType		ArmorType;

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(ArmorType);
		}
	};
	struct	CMeleeWeapon
	{
		WEAPONTYPE::EWeaponType		WeaponType;
		SKILLS::ESkills				Skill;
		DMGTYPE::EDamageType		DamageType;
		sint32						MeleeRange;

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(WeaponType);
			f.serialEnum(Skill);
			f.serialEnum(DamageType);
			f.serial(MeleeRange);
		}
	};
	struct	CRangeWeapon
	{
		WEAPONTYPE::EWeaponType				WeaponType;
		RANGE_WEAPON_TYPE::TRangeWeaponType RangeWeaponType;
		SKILLS::ESkills						Skill;

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(WeaponType);
			f.serialEnum(Skill);
			f.serialEnum(RangeWeaponType);
		}
	};
	struct	CAmmo
	{
		SKILLS::ESkills				Skill;
		DMGTYPE::EDamageType		DamageType;
		sint32						Magazine;

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(Skill);
			f.serialEnum(DamageType);
			f.serial(Magazine);
		}
	};
	// Info on a itemPart which can be build by this MP
	struct CMpItemPart
	{
		// The origin filter. It is actually a ITEM_ORIGIN::EItemOrigin
		uint8			OriginFilter;

		// The differents stats for this itemPart
		uint8			Stats[RM_FABER_STAT_TYPE::NumRMStatType];

		CMpItemPart()
		{
			OriginFilter = 0;
			std::fill( Stats, Stats + RM_FABER_STAT_TYPE::NumRMStatType, 0 );
		}

		void	serial(NLMISC::IStream &f)
		{
			f.serial(OriginFilter);
			// must change sheet version
			nlctassert(RM_FABER_STAT_TYPE::NumRMStatType==34);
			for(uint i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
			{
				f.serial(Stats[i]);
			}
		}
	};
	struct	CMp
	{
	public:
		ECOSYSTEM::EECosystem		Ecosystem;
		MP_CATEGORY::TMPCategory	MpCategory;
		SKILLS::ESkills				HarvestSkill;
		// The MP Family
		RM_FAMILY::TRMFamily				Family;
		// If the MP is used as a special Craft Component requirement
		bool								UsedAsCraftRequirement;
		// The Mp color
		sint8								MpColor;
		// The mean Stat Energy of this MP
		uint16								StatEnergy;
		// The ItemParts this MP can craft
		uint64								ItemPartBF;

	public:
		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(Ecosystem);
			f.serialEnum(MpCategory);
			f.serialEnum(HarvestSkill);
			f.serialEnum(Family);
			f.serial(ItemPartBF);
			f.serial(UsedAsCraftRequirement);
			f.serial(MpColor);
			f.serial(StatEnergy);
		}
	};
	struct	CShield
	{
		SHIELDTYPE::EShieldType		ShieldType;

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(ShieldType);
		}
	};
	struct	CTool
	{
		SKILLS::ESkills					Skill;				// Used by HARVEST, TAMING, TRAINING tools
		TOOL_TYPE::TCraftingToolType	CraftingToolType;	// Used by CRAFTING tool
		sint32							CommandRange;		// Used by TAMING tool
		sint32							MaxDonkey;			// Used by TAMING tool

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(Skill);
			f.serialEnum(CraftingToolType);
			f.serial(CommandRange);
			f.serial(MaxDonkey);
		}
	};

	struct CGuildOption
	{
		uint32		MoneyCost;
		sint32		XPCost;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(MoneyCost);
			f.serial(XPCost);
		}
	};

	struct CPet
	{
		sint32		Slot;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(Slot);
		}
	};

	struct CTeleport
	{
		TELEPORT_TYPES::TTeleportType Type;

		void	serial(NLMISC::IStream &f)
		{
			f.serialEnum(Type);
		}
	};

	struct CScroll
	{
		std::string Texture;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(Texture);
		}
	};

	struct CConsumable
	{
		uint16 OverdoseTimer;
		uint16 ConsumptionTime;

		CConsumable()
		{
			OverdoseTimer = 0;
			ConsumptionTime = 0;
		}

		std::vector<std::string> Properties;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(OverdoseTimer);
			f.serial(ConsumptionTime);
			f.serialCont(Properties);
		}
	};

	public:
	/// shape file name
	NLMISC::TSStringId					IdShape;
	/// Female shape file name
	NLMISC::TSStringId					IdShapeFemale;
	/// Equipment slot. This is a bitField matching each bit to SLOTTYPE::TSlotType
	uint64								SlotBF;
	/// texture variant.
	uint32								MapVariant;
	/// Item Family
	ITEMFAMILY::EItemFamily				Family;
	/// Item Type
	ITEM_TYPE::TItemType				ItemType;
	/// icon file name for race type
	NLMISC::TSStringId					IdIconBack;
	/// icon file name for main icon type
	NLMISC::TSStringId					IdIconMain;
	/// icon file name for overlay
	NLMISC::TSStringId					IdIconOver;
	/// icon file name for overlay2
	NLMISC::TSStringId					IdIconOver2;
	// Special Color to modulate with
	NLMISC::CRGBA						IconColor;
	NLMISC::CRGBA						IconBackColor;
	NLMISC::CRGBA						IconOverColor;
	NLMISC::CRGBA						IconOver2Color;
	/// icon Special Text (raw materials)
	NLMISC::TSStringId					IdIconText;
	/// Part of the animation set ot use with this item.
	NLMISC::TSStringId					IdAnimSet;
	/// Item Color. Special Enum for armours
	sint8								Color;
	/// has fx
	bool								HasFx;
	// Does the player can sell the item ?
	bool								DropOrSell;
	// Item is not persistent to a disconnection ?
	bool								IsItemNoRent;
	/// item max stack size
	uint32								Stackable;
	/// is item consumable
	bool								IsConsumable;
	/// Bulk.
	float								Bulk;
	/// Equip Time
	uint32								EquipTime;
	/// true if this item can be hidden when equipped
	bool								NeverHideWhenEquipped;

	// FX
	CItemFXSheet						FX;

	// item special effects
	NLMISC::TSStringId					IdEffect1;
	NLMISC::TSStringId					IdEffect2;
	NLMISC::TSStringId					IdEffect3;
	NLMISC::TSStringId					IdEffect4;

	// Only used for Mp
	std::vector<CMpItemPart>			MpItemParts;

	// item requirements
	CHARACTERISTICS::TCharacteristics	RequiredCharac;
	uint16								RequiredCharacLevel;
	SKILLS::ESkills						RequiredSkill;
	uint16								RequiredSkillLevel;

	/// if craftable, the craft plan
	NLMISC::CSheetId					CraftPlan;

	/// \name Help Infos
	// @{
	// Basics
	ITEM_ORIGIN::EItemOrigin			ItemOrigin;
	// Different according to Family
	union
	{
		CCosmetic						Cosmetic;
		CArmor							Armor;
		CMeleeWeapon					MeleeWeapon;
		CRangeWeapon					RangeWeapon;
		CAmmo							Ammo;
		CMp								Mp;
		CShield							Shield;
		CTool							Tool;
		CGuildOption					GuildOption;
		CPet							Pet;
		CTeleport						Teleport;
	};

	CScroll							Scroll;
	CConsumable						Consumable;
	// @}


	/**
	 * Constructor
	 */
	CItemSheet();

	std::string getShape() const { return ClientSheetsStrings.get(IdShape); }
	std::string getShapeFemale() const { return ClientSheetsStrings.get(IdShapeFemale); }
	std::string getIconBack() const { return ClientSheetsStrings.get(IdIconBack); }
	std::string getIconMain() const { return ClientSheetsStrings.get(IdIconMain); }
	std::string getIconOver() const { return ClientSheetsStrings.get(IdIconOver); }
	std::string getIconOver2() const { return ClientSheetsStrings.get(IdIconOver2); }
	std::string getIconText() const { return ClientSheetsStrings.get(IdIconText); }
	std::string getAnimSet() const { return ClientSheetsStrings.get(IdAnimSet); }

	std::string getEffect1() const { return ClientSheetsStrings.get(IdEffect1); }
	std::string getEffect2() const { return ClientSheetsStrings.get(IdEffect2); }
	std::string getEffect3() const { return ClientSheetsStrings.get(IdEffect3); }
	std::string getEffect4() const { return ClientSheetsStrings.get(IdEffect4); }

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// true if the item can put in the slot e
	bool		hasSlot(SLOTTYPE::TSlotType e) const {return (SlotBF&(SINT64_CONSTANT(1)<<e))!=0;}

	/// true if the item is faberisable according to family
	bool		isFaberisable() const;

	// Get the skill required by the item depending on its type (or unknown_skill_type if no specific skill is required)
	SKILLS::ESkills		getRequiredSkill() const;

	// MP only. true if durability ok
	bool	canBuildItemPart(RM_FABER_TYPE::TRMFType e) const;

	// MP only. true if durability ok, and if origin ok
	bool	canBuildItemPart(RM_FABER_TYPE::TRMFType e, ITEM_ORIGIN::EItemOrigin origin) const;

	// MP only. get a ref on the item part. assert if cannot build for any race
	const CMpItemPart	&getItemPart(RM_FABER_TYPE::TRMFType e) const;

	// true if this is an MP that can build some item part
	bool	canBuildSomeItemPart() const;

	// true if this is an MP that is required for some special item crafting
	bool	isUsedAsCraftRequirement() const;

	// Weapon/Armor/Buckler only: check if some requirement on this item.
	// if true, caracType/caracValue is filled according to itemType and itemLevel
	// the current carac must be >= caracValue
	bool	hasCharacRequirement(uint itemLevel, CHARACTERISTICS::TCharacteristics	&caracType, float &caracValue) const;

	// return if canDrop (player exchange or BotChat Gift)
	bool	canExchangeOrGive(bool botChatGift) const;

	// MP only. return translated text of all item part this MP can build. empty, if can't build anything
	void	getItemPartListAsText(ucstring &ipList) const;

	// get craft plan
	const NLMISC::CSheetId &getCraftPlan() const { return CraftPlan; }
};


#endif // CL_ITEM_SHEET_H

/* End of item_sheet.h */
