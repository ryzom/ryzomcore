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
#include "attack_list.h"
#include "sheet_manager.h"
#include "global.h"
//
#include "client_sheets/attack_list_sheet.h"
//
#include "nel/3d/u_scene.h"
//
#include "nel/misc/sheet_id.h"
//
#include <utility>


H_AUTO_DECL(RZ_AttackList)

using namespace NLMISC;
using namespace std::rel_ops;

extern NL3D::UScene *Scene;

CAttackListManager *CAttackListManager::_Instance = NULL;


// get a .animation_fx sheet from its name (NULL if not found)
static CAnimationFXSetSheet *getAnimFXSetSheetFromName(const std::string &name)
{
	CEntitySheet *sheet = SheetMngr.get(NLMISC::CSheetId(name));
	if (!sheet) return NULL;
	if (sheet->Type == CEntitySheet::ANIMATION_FX_SET) return static_cast<CAnimationFXSetSheet *>(sheet);
	return NULL;
}

// build an attack part
static void buildAttackPart(const std::string &sheetName, CAnimationFXSet &fxSet, NL3D::UAnimationSet *as)
{
	if (!sheetName.empty())
	{
		CAnimationFXSetSheet *sheet = getAnimFXSetSheetFromName(sheetName);
		if (sheet)
		{
			fxSet.init(sheet, as);
		}
		else
		{
			nlwarning("Sheet %s not found", sheetName.c_str());
		}
	}
}

// ***********************************************************************************************
CAttack::CAttack()
{
	Sheet = NULL;
}

// ***********************************************************************************************
void CAttack::init(const CAttackSheet *sheet, NL3D::UAnimationSet *as)
{
	nlassert(!Sheet); // init already done
	if (!sheet) return;
	Sheet = sheet;
	//
	buildAttackPart(Sheet->AttackBeginFX, AttackBeginFX, as);
	buildAttackPart(Sheet->AttackLoopFX, AttackLoopFX, as);
	buildAttackPart(Sheet->AttackEndFX, AttackEndFX, as);
	buildAttackPart(Sheet->AttackStaticObjectCastFX, AttackStaticObjectCastFX, as);
	buildAttackPart(Sheet->AttackFailFX, AttackFailFX, as);
	buildAttackPart(Sheet->ProjectileFX, ProjectileFX, as);
	buildAttackPart(Sheet->ImpactFX, ImpactFX, as);
}

// ***********************************************************************************************
void CAttackList::init(const CAttackListSheet *attackList, NL3D::UAnimationSet *as)
{
	nlassert(_Attacks.empty());
	if (!attackList) return;
	_Attacks.resize(attackList->Attacks.size());
	for(uint k = 0; k < _Attacks.size(); ++k)
	{
		_Attacks[k].ID = &attackList->Attacks[k].ID;
		_Attacks[k].Attack.init(&(attackList->Attacks[k].Attack), as);
	}
	// sort all attacks for fast retrieval
	std::sort(_Attacks.begin(), _Attacks.end());
}


/*
struct CAttackEntryComp
{
	bool operator()(const CAttackListEntry &lhs, const CAttackIDSheet &rhs) const
	{
		nlassert(lhs.ID);
		return *lhs.ID < rhs;
	}
};
*/

struct CAttackEntryComp2
{
	bool operator()(const CAttackListEntry &lhs, const CAttackListEntry &rhs) const
	{
		nlassert(lhs.ID);
		nlassert(rhs.ID);
		return *lhs.ID < *rhs.ID;
	}
};

// ***********************************************************************************************
const CAttack *CAttackList::getAttackFromID(const CAttackIDSheet &id) const
{
	H_AUTO_USE(RZ_AttackList);
// vl: changed, this line only work with stlport
//	std::vector<CAttackListEntry>::const_iterator it = std::lower_bound(_Attacks.begin(), _Attacks.end(), id, CAttackEntryComp());

	CAttackListEntry ale(&id);
	std::vector<CAttackListEntry>::const_iterator it = std::lower_bound(_Attacks.begin(), _Attacks.end(), ale, CAttackEntryComp2());
	if (it == _Attacks.end()) return NULL;
	if (*(it->ID) != id) return NULL;
	return &(it->Attack);
}

// ***********************************************************************************************
CAttackListManager &CAttackListManager::getInstance()
{
	H_AUTO_USE(RZ_AttackList)
	if (_Instance) return *_Instance;
	_Instance = new CAttackListManager();
	return *_Instance;
}

// ***********************************************************************************************
void CAttackListManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

// ***********************************************************************************************
void CAttackListManager::init()
{
	if (_AnimationSet) return; // init already done
	if (!Scene) return; // no scene, can't build anything
	_AnimationSet = Driver->createAnimationSet();
	if (!_AnimationSet) return;
	std::vector<CSheetId> result;
	std::vector<std::string> filenames;
	NLMISC::CSheetId::buildIdVector(result, filenames, "attack_list");
	for(uint k = 0; k < result.size(); ++k)
	{
		const CAttackListSheet *sheet = dynamic_cast<const CAttackListSheet *>(SheetMngr.get(result[k]));
		if (sheet)
		{
			TAttackListMap::iterator it = _AttackMap.find(filenames[k]);
			if (it != _AttackMap.end())
			{
				nlwarning("attack list duplicated : %s", filenames[k].c_str());
			}
			CAttackList &al = _AttackMap[filenames[k]];
			al.init(sheet, _AnimationSet);
		}
	}
	// auras and links
	buildAurasFXs();
	buildLinkFXs();
}

// ***********************************************************************************************
void CAttackListManager::release()
{
	if (!_AnimationSet) return;
	_AttackMap.clear();
	if (Driver) Driver->deleteAnimationSet(_AnimationSet);
	_Auras.release();
	_Links.release();
	delete _Instance;
	_Instance = NULL;
}

// ***********************************************************************************************
const CAttackList *CAttackListManager::getAttackList(const std::string &name) const
{
	H_AUTO_USE(RZ_AttackList)
	TAttackListMap::const_iterator it = _AttackMap.find(name);
	if (it != _AttackMap.end()) return &(it->second);
	return NULL;
}

// ***********************************************************************************************
CAttackListManager::CAttackListManager()
{
	_AnimationSet = NULL;
}

// *******************************************************************************************
void CAttackListManager::buildAurasFXs()
{
	_Auras.init("auras.id_to_string_array", _AnimationSet, false /* must not delete animset, owned by this object */);
}

// *******************************************************************************************
void CAttackListManager::buildLinkFXs()
{
	_Links.init("links.id_to_string_array", _AnimationSet, false /* must not delete animset, owned by this object */);
}
