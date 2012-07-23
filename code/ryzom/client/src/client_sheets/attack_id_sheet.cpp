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
#include "attack_id_sheet.h"
//
#include "nel/georges/u_form_elm.h"


// *****************************************************************************************
void CAttackIDSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	uint32 attackType = 0;
	item.getValueByName(attackType, (prefix + "AttackType").c_str());
	Type = (TType) attackType;
	switch(Type)
	{
		case Melee:
			// currently no additionnal infos for melee
		break;
		case Range:
		{
			std::string rangeWeaponType;
			if (item.getValueByName(rangeWeaponType, (prefix + "RangeWeaponType").c_str()))
			{
				RangeWeaponType = RANGE_WEAPON_TYPE::stringToRangeWeaponType(rangeWeaponType);
			}
		}
		break;
		case Magic:
			SpellInfo.build(item, prefix);
		break;
		case Creature:
			item.getValueByName(CreatureAttackIndex, (prefix + "CreatureAttackIndex").c_str());
		break;
		case DamageShield:
			item.getValueByName(DamageShieldType, (prefix + "DamageShieldType").c_str());
		break;
		default:
			nlwarning("Bad attack type");
		break;
	}
}

// *****************************************************************************************
void CAttackIDSheet::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialEnum(Type);
	switch(Type)
	{
		case Melee:
			// currently no additionnal infos for melee
		break;
		case Range:
			f.serialEnum(RangeWeaponType);
		break;
		case Magic:
			f.serial(SpellInfo);
		break;
		case Creature:
			f.serial(CreatureAttackIndex);
		break;
		case DamageShield:
			f.serial(DamageShieldType);
		break;
		default:
			nlwarning("Bad attack type");
			break;
	}
}

// *****************************************************************************************
void CAttackIDSheet::CSpellInfo::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	uint32 spellID = 0;
	item.getValueByName(spellID, (prefix + ".SpellID").c_str());
	ID = (MAGICFX::TMagicFx) spellID;
	uint32 spellMode = 0;
	item.getValueByName(spellMode, (prefix + ".SpellMode").c_str());
	Mode = (MAGICFX::TSpellMode) spellMode;
}

// *****************************************************************************************
void CAttackIDSheet::CSpellInfo::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialEnum(Mode);
	f.serialEnum(ID);
}

// *****************************************************************************************
bool operator == (const CAttackIDSheet &lhs, const CAttackIDSheet &rhs)
{
	if (lhs.Type != rhs.Type) return false;
	switch(lhs.Type)
	{
		case CAttackIDSheet::Melee: return true;
		case CAttackIDSheet::Range: return lhs.RangeWeaponType == rhs.RangeWeaponType;
		case CAttackIDSheet::Magic: return lhs.SpellInfo == rhs.SpellInfo;
		case CAttackIDSheet::Creature: return lhs.CreatureAttackIndex == rhs.CreatureAttackIndex;
		case CAttackIDSheet::DamageShield: return lhs.DamageShieldType == rhs.DamageShieldType;
		default: return false;
	}
}

// *****************************************************************************************
bool operator < (const CAttackIDSheet &lhs, const CAttackIDSheet &rhs)
{
	if (lhs.Type != rhs.Type) return lhs.Type < rhs.Type;
	switch(lhs.Type)
	{
		case CAttackIDSheet::Melee: return false;
		case CAttackIDSheet::Range: return lhs.RangeWeaponType < rhs.RangeWeaponType;
		case CAttackIDSheet::Magic: return lhs.SpellInfo < rhs.SpellInfo;
		case CAttackIDSheet::Creature: return lhs.CreatureAttackIndex < rhs.CreatureAttackIndex;
		case CAttackIDSheet::DamageShield: return lhs.DamageShieldType < rhs.DamageShieldType;
		default: return false;
	}
}

// *****************************************************************************************
bool operator == (const CAttackIDSheet::CSpellInfo &lhs, const CAttackIDSheet::CSpellInfo &rhs)
{
	return lhs.Mode == rhs.Mode && lhs.ID == rhs.ID;
}

// *****************************************************************************************
bool operator < (const CAttackIDSheet::CSpellInfo &lhs, const CAttackIDSheet::CSpellInfo &rhs)
{
	if (lhs.Mode != rhs.Mode) return lhs.Mode < rhs.Mode;
	if (lhs.ID != rhs.ID) return lhs.ID < rhs.ID;
	return false;
}
