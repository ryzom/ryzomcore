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
#include "attack_list_sheet.h"
//
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;

// *******************************************************************************************
CAttackListSheet::CAttackListSheet()
{
	Type = ATTACK_LIST;
}

// *******************************************************************************************
void CAttackListSheetEntry::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	ID.build(item, prefix + "ID."	);
	Attack.build(item, prefix + "Attack.");
}

// *******************************************************************************************
void CAttackListSheetEntry::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(ID);
	f.serial(Attack);
}

// *******************************************************************************************
void CAttackListSheet::build(const NLGEORGES::UFormElm &item)
{
	const UFormElm *attacks = NULL;
	if (item.getNodeByName(&attacks, "Attacks") && attacks)
	{
		uint numAttacks;
		nlverify(attacks->getArraySize(numAttacks));
		Attacks.reserve(numAttacks);
		for(uint k = 0; k < numAttacks; ++k)
		{
			const UFormElm *attackNode = NULL;
			if (attacks->getArrayNode(&attackNode, k) && attackNode)
			{
				Attacks.push_back(CAttackListSheetEntry());
				Attacks.back().build(*attackNode, "");
			}
		}
	}
}

// *******************************************************************************************
void CAttackListSheet::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(Attacks);
}


