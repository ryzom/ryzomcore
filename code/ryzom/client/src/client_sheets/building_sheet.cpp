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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Application
#include "building_sheet.h"
// Georges
#include "nel/georges/u_form_elm.h"



///////////
// USING //
///////////
using namespace NLGEORGES;
using namespace std;


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CBuildingSheet :
// Constructor.
//-----------------------------------------------
CBuildingSheet::CBuildingSheet()
{
	// Initialize the type.
	Type = CEntitySheet::BUILDING;
}// CBuildingSheet //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CBuildingSheet::build(const NLGEORGES::UFormElm &item)
{
	// Load the name.
	if(!item.getValueByName(BuildedIg, "builded_ig"))
		debug("builded_ig not found.");
	if(!item.getValueByName(BuildedIcon, "builded_icon"))
		debug("builded_icon not found.");
	if(!item.getValueByName(BuildingIcon, "building_icon"))
		debug("building_icon not found.");
	if(!item.getValueByName(Name, "name"))
		debug("name not found.");
}// build //

//-----------------------------------------------
// serial :
// Serialize character sheet into binary data file.
//-----------------------------------------------
void CBuildingSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serialize class components.
	f.serial(BuildedIg);
	f.serial(BuildedIcon);
	f.serial(BuildingIcon);
	f.serial(Name);
}// serial //

