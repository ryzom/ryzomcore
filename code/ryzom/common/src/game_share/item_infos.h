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



#ifndef RY_ITEM_INFOS_H
#define RY_ITEM_INFOS_H

#include "nel/misc/types_nl.h"
#include "damage_types.h"
#include "protection_type.h"
#include "people.h"
#include "skills.h"
#include "characteristics.h"
#include "sphrase_com.h"
#include "type_skill_mod.h"


///\structure defining the protection of an item
struct SProtection
{
	inline SProtection():Factor(0.0f),Max(0){}
	void serial( NLMISC::IStream & f )
	{
		f.serial( Factor );
		f.serial( Max );
	}
	/// protection factor ( must be in [0..1] range 0 means no protection)
	float Factor;
	/// max absorbed damage
	uint16 Max;
};


/**
 * class used to store the item advanced infos and to send it to the client
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CItemInfos
{
public:
	enum
	{
		// The number of bits of the lower bits part (the index part)
		// 10 because the trading list can have 1024 entries
		SlotIdIndexBitSize= 11,
		SlotIdIndexBitMask= (1<<SlotIdIndexBitSize)-1,
	};

	enum
	{
		MaxMagicProtectionByJewel= 3,
		MaxMagicResistanceByJewel= 3,
	};

public:
	/// ctor
	CItemInfos();

	///\serial
	void serial(NLMISC::IStream & s);

	/// inventoryId(6 higher order bits )/slotId(10 lowers bits)
	uint16	slotId;
	/// version of the inventory slot
	uint16	versionInfo;
	///\name item properties
	//{@
	uint32 Hp;				// Current Durability of item
	uint32 HpMax;			// Max Durability of item (when is new)
	uint32 CreatorName;		// player character crafted item (if is a item crafted by PC)
	uint32 CurrentDamage;	// if is weapons or ammo, damage made by item whith current PC skill, if item is an range weapon, it is a modifier (+/- and base is ammo) (can be modified by action)
	uint32 MaxDamage;		// if is weapons or ammo, maximum damage can be made by item (due to his recommended skill (= old quality for weapon and ammos), this is reached when skill = recommended (can be modified by action)
	float HitRate;			// nb hits per minute item made (can be modified by action)
	uint32 SapLoadCurrent;	// sap load of item (for enchantment)
	uint32 SapLoadMax;		// sap load of item (for enchantment)
	float Range;			// range of weapon in milimeter, if item is an ammo, it is a modifier (+/-, base is range weapon)
	sint32 ParryModifier;	// parry modifier
	sint32 DodgeModifier;	// dodge modifier
	sint32 AdversaryParryModifier; // target parry modifier
	sint32 AdversaryDodgeModifier; // target dodge modifier
	float ProtectionFactor;	// protection in % gived by item
	uint32 MaxSlashingProtection; // max damage absorbed by protection
	uint32 MaxBluntProtection;    // max damage absorbed by protection
	uint32 MaxPiercingProtection; // max damage absorbed by protection
	PROTECTION_TYPE::TProtectionType MagicProtection[MaxMagicProtectionByJewel]; //magic protection, only gived by jewel
	uint32 MagicProtectionFactor[MaxMagicProtectionByJewel];
	uint32 DesertMagicResistance;	// Magic resistances
	uint32 ForestMagicResistance;
	uint32 LacustreMagicResistance;
	uint32 JungleMagicResistance;
	uint32 PrimaryRootMagicResistance;
	sint32 HpBuff;	// hp buff / debuff gived when item is equipped
	sint32 SapBuff; // sap buff / debuff gived when item is equipped
	sint32 StaBuff; // sta buff / debuff gived when item is equipped
	sint32 FocusBuff; // focus buff / debuff gived when item is equipped

	SKILLS::ESkills	RequiredSkill; // required skill
	uint16 RequiredSkillLevel; // required skill level
	SKILLS::ESkills	RequiredSkill2; // required skill
	uint16 RequiredSkillLevel2; // required skill level
	CHARACTERISTICS::TCharacteristics RequiredCharac; // required stat
	uint16 RequiredCharacLevel; // min required stat level

	// type specific skill bonus (applied against given race ennemies)
	std::vector<CTypeSkillMod>	TypeSkillMods;

	// magic focus factor
	enum	TMagicFactorType
	{
		OffensiveElemental=0,	// elemental spells (DD acid, fire...)
		OffensiveAffliction,	// offensive affliction spells
		DefensiveHeal,			// heal spells
		DefensiveAffliction,	// defensive affliction spells

		NumMagicFactorType
	};
	float	CastingSpeedFactor[NumMagicFactorType];
	float	MagicPowerFactor[NumMagicFactorType];

	CSPhraseCom	Enchantment;
	float  WearEquipmentMalus; // Malus for wearing this equipment (malus is used when execute an magic, forage action, craft action...), malus is only applicable for weapon and armor pieces

	ucstring	CustomText;
	ucstring	R2ItemDescription;
	ucstring	R2ItemComment;
	uint8		PetNumber; // 1 based pet index
	//@}
};


#endif // RY_ITEM_INFOS_H

/* End of item_infos.h */
