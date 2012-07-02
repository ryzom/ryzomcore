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
#include "faction_sheet.h"
//
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;


//=====================================================================================================================
/**Tool fct to extract a value from a sheet, and to display a warning if it failed.
  */
template <class T> static void GetFactionFormValue(const NLGEORGES::UFormElm &item, T &destItem, const char *name)
{
	nlassert(name);
	if (!item.getValueByName(destItem, name)) nlwarning("FactionSheet : can't get %s value.", name);
}

//=======================================================================
void CFactionSheet::build(const NLGEORGES::UFormElm &item)
{
	GetFactionFormValue(item, Icon, "Icon");
}
//=======================================================================
void CFactionSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Icon);
}

