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




//////////////
// INCLUDES //
//////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Application
#include "mission_icon_sheet.h"
// Georges
#include "nel/georges/u_form_elm.h"



///////////
// USING //
///////////
using namespace NLGEORGES;
using namespace NLMISC;
using namespace std;


//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CMissionIconSheet::build(const NLGEORGES::UFormElm &item)
{
	// Load the descriptors.
	if(!item.getValueByName(MainIconBg, "MainIconBg"))
		debug("key 'MainIconBg' not found.");

	if(!item.getValueByName(MainIconFg, "MainIconFg"))
		debug("key 'MainIconFg' not found.");

	if(!item.getValueByName(SmallIcon, "SmallIcon"))
		debug("key 'SmallIcon' not found.");

}// build //


//-----------------------------------------------
// serial :
// Serialize character sheet into binary data file.
//-----------------------------------------------
void CMissionIconSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(MainIconBg);
	f.serial(MainIconFg);
	f.serial(SmallIcon);
}// serial //

