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



#ifndef CL_ATTACK_LIST_H
#define CL_ATTACK_LIST_H

#include "animation_fx.h"
#include "animation_fx_id_array.h"
//
#include "client_sheets/attack_sheet.h"
#include "client_sheets/attack_id_sheet.h"
//
#include "nel/misc/string_conversion.h"


namespace NL3D
{
	class UAnimationSet;
}


class CAttackListSheet;

/** Description of a creature attack
  */
class CAttack
{
public:
	// pointer on base sheet
	const CAttackSheet *Sheet;
	CAnimationFXSet		AttackBeginFX;  // .animation_fx_set
	CAnimationFXSet		AttackLoopFX;
	CAnimationFXSet		AttackEndFX;
	CAnimationFXSet		AttackStaticObjectCastFX;
	CAnimationFXSet		AttackFailFX;
	CAnimationFXSet		ProjectileFX;
	CAnimationFXSet		ImpactFX;
public:
	// ctor
	CAttack();
	// Init from parent sheet
	void init(const CAttackSheet *sheet, NL3D::UAnimationSet *as);
};

/// A named attack, to be inserted in an attack list
struct CAttackListEntry
{
	CAttackListEntry() : ID(0) { }
	CAttackListEntry(const CAttackIDSheet *id) : ID(id) { }
	const CAttackIDSheet *ID;
	CAttack				  Attack;
};
//
inline bool operator == (const CAttackListEntry &lhs, const CAttackListEntry &rhs)
{
	nlassert(lhs.ID && rhs.ID);
	return lhs.ID == rhs.ID;
}
//
inline bool operator < (const CAttackListEntry &lhs, const CAttackListEntry &rhs)
{
	nlassert(lhs.ID && rhs.ID);
	return *lhs.ID < *rhs.ID;
}


/** A list of attacks, sorted by their IDs
  */
class CAttackList
{
public:
	// build from a sheet
	void init(const CAttackListSheet *attackList, NL3D::UAnimationSet *as);

	// get an attack from its ID, or NULL if not found
	const CAttack *getAttackFromID(const CAttackIDSheet &id) const;
private:
	std::vector<CAttackListEntry> _Attacks;
};


/** gather all creature attack in a single place
  */
class CAttackListManager
{
public:
	// get the unique instance of this class
	static CAttackListManager &getInstance();
	// release memory
	static void releaseInstance();
	/** This :
	  * - Init an animation set.
	  * - Build all creature attacks datas and build their tracks.
	  */
	void init();
	void release();
	// get an attack list by its name (NULL if not found)
	const CAttackList *getAttackList(const std::string &name) const;
	// get auras fx sorted by their id
	const CAnimationFXIDArray &getAuras() const { return _Auras; }
	// get link fx sorted by their id
	const CAnimationFXIDArray &getLinks() const { return _Links; }
private:
	typedef std::map<std::string,
		             CAttackList,
					 NLMISC::CUnsensitiveStrLessPred> TAttackListMap;
	NL3D::UAnimationSet					*_AnimationSet;
	static	CAttackListManager			*_Instance;
	TAttackListMap						_AttackMap;
	// auras and links
	CAnimationFXIDArray			_Auras;
	CAnimationFXIDArray			_Links;
private:
	// ctor
	CAttackListManager();
	void buildAurasFXs();
	void buildLinkFXs();
};



#endif
