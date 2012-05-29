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
#include "outpost_squad_sheet.h"

#include "nel/georges/u_form_elm.h"


// ****************************************************************************
COutpostSquadSheet::COutpostSquadSheet()
{
	Type = CEntitySheet::OUTPOST_SQUAD;
}

// ****************************************************************************
void COutpostSquadSheet::build(const NLGEORGES::UFormElm &root)
{
	root.getValueByName(BuyPrice, "Buy Price");
}

// ****************************************************************************
void COutpostSquadSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(BuyPrice);
}

