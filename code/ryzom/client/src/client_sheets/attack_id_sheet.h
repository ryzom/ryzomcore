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

#ifndef CL_ATTACK_ID_H
#define CL_ATTACK_ID_H

#include "game_share/magic_fx.h"
#include "game_share/range_weapon_type.h"


/** Identifier of an attack (magic, range, melle, or creature)
  * This helps doing the links between attack behaviour and its graphical representation
  *
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
class CAttackIDSheet
{
public:
	enum TType { Melee = 0, Range, Magic, Creature, DamageShield, Unknown };
	//
	class CSpellInfo
	{
	public:
		MAGICFX::TSpellMode Mode;
		MAGICFX::TMagicFx   ID;
	public:
		void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};
public:
	TType				  Type;
	union
	{
		CSpellInfo		                    SpellInfo;            // valid if type == Magic
		RANGE_WEAPON_TYPE::TRangeWeaponType RangeWeaponType;	  // valid if type == Range
		uint32			                    CreatureAttackIndex;  // valid if type == Creature.  Currently, maybe 0 or 1 2 attack per creature)
		uint32								DamageShieldType;       // valid if type == DamageShield
	};
public:
	virtual void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};

// compares spell infos
bool operator == (const CAttackIDSheet::CSpellInfo &lhs, const CAttackIDSheet::CSpellInfo &rhs);
inline bool operator != (const CAttackIDSheet::CSpellInfo &lhs, const CAttackIDSheet::CSpellInfo &rhs) { return !(lhs == rhs); }
// build an ordering between attack ids
bool operator < (const CAttackIDSheet::CSpellInfo &lhs, const CAttackIDSheet::CSpellInfo &rhs);
// compares 2 attack ids
bool operator == (const CAttackIDSheet &lhs, const CAttackIDSheet &rhs);
inline bool operator != (const CAttackIDSheet &lhs, const CAttackIDSheet &rhs) { return !(lhs == rhs); }
// build an ordering between attack ids
bool operator < (const CAttackIDSheet &lhs, const CAttackIDSheet &rhs);



#endif
