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



#ifndef CL_ATTACK_LIST_SHEET_H
#define CL_ATTACK_LIST_SHEET_H

#include "entity_sheet.h"
#include "attack_sheet.h"
#include "attack_id_sheet.h"


// an entry in an attack list
class CAttackListSheetEntry
{
public:
	CAttackIDSheet ID;
	CAttackSheet   Attack;
public:
	void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};


/** A list of attack (range, melee, magic, or creature)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
class CAttackListSheet : public CEntitySheet
{
public:
	std::vector<CAttackListSheetEntry> Attacks; // sorted list of attack
public:
	CAttackListSheet();
	virtual void build(const NLGEORGES::UFormElm &item);
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};



#endif
