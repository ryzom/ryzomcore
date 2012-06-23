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
#include "mission_sheet.h"
// Georges
#include "nel/georges/u_form_elm.h"


/////////////
// DEFINES //
/////////////
#define NB_STEPS_PER_MISSION 20


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
void CMissionSheet::build(const NLGEORGES::UFormElm &item)
{
	// Load the descriptors.
	if(!item.getValueByName(Name, "Name"))
		debug("key 'Name' not found.");

	if(!item.getValueByName(Description, "Description"))
		debug("key 'Description' not found.");

	if(!item.getValueByName(RewardDescription, "RewardDescription"))
		debug("key 'RewardDescription' not found.");

	// load mission steps description
	for (uint i =1; i< NB_STEPS_PER_MISSION + 1;i++)
	{
		const UFormElm * stepStruct;
		string varName = string("step") + NLMISC::toString(i);
		item.getNodeByName (&stepStruct, varName.c_str());

		if (stepStruct)
		{
			string stepDesc;
			stepStruct->getValueByName(stepDesc,"Description");
			if ( !stepDesc.empty() )
				StepsDescription.push_back(stepDesc);
		}

	}
}// build //


//-----------------------------------------------
// serial :
// Serialize character sheet into binary data file.
//-----------------------------------------------
void CMissionSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serialize class components.
	f.serial(Name);
	f.serial(Description);
	f.serial(RewardDescription);
	f.serialCont(StepsDescription);
}// serial //

