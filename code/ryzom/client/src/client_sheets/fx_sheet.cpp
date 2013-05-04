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
// Georges
#include "nel/georges/u_form_elm.h"
// Client.
#include "fx_sheet.h"


///////////
// USING //
///////////
using namespace NLGEORGES;
using namespace std;


uint CFXSheet::CPSStruct::StandardIndex = 2;


//-----------------------------------------------
// CFXSheet :
// Constructor.
//-----------------------------------------------
CFXSheet::CFXSheet()
{
	// Initialize the type.
	Type	= CEntitySheet::FX;
}// CFXSheet //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CFXSheet::build(const NLGEORGES::UFormElm &item)
{

	// Trail List
	const UFormElm *elmt = 0;
	bool result = item.getNodeByName(&elmt, "Trail_List");
	if(result)
	{
		if (elmt)
		{
			uint arrawSize;
			elmt->getArraySize(arrawSize);
			// Get all animation for the State.
			for(uint i = 0; i<arrawSize; ++i)
			{
				const UFormElm *trailElmt;
				result = elmt->getArrayNode(&trailElmt, i);
				if (result)
				{
					if(trailElmt)
					{
						// Get the shape
						string shape;
						if(!trailElmt->getValueByName(shape, "Shape"))
							debug("Key 'Shape' not found.");

						nlinfo("trail %s", shape.c_str());
						if(!shape.empty())
							TrailList.push_back(shape);
					}
				}
				else
				{
					nlwarning("CAnimationState::build : element (%d) should be here in 'animations'. Sheet is %s.", Id.toString().c_str());
				}
			}
		}
	}
	else
	{
		nlwarning("CFXSheet::build: there is no element 'Trail_List' in the form %s.", Id.toString().c_str());
	}

	// PS List
	elmt = 0;
	item.getNodeByName(&elmt, "PS_List");
	if(elmt)
	{
		uint arrawSize;
		elmt->getArraySize(arrawSize);
		// Get all animation for the State.
		for(uint i = 0; i<arrawSize; ++i)
		{
			const UFormElm *psElmt;
			elmt->getArrayNode(&psElmt, i);
			if(psElmt)
			{
				// Get the shape
				CPSStruct ps;
				if(!psElmt->getValueByName(ps.PSName,	"PS"))
					debug("Key 'PS' not found.");
				if(!psElmt->getValueByName(ps.Anim,		"Anim"))
					debug("Key 'Anim' not found.");

				CPSStruct::CPSUserParamStruct params;

				// Power Null
				ps.Power.push_back(params);

				// Weak
				if(!psElmt->getValueByName(params.UserParam0, "weak.User Param 0"))
					debug("Key 'weak.User Param 0' not found.");
				if(!psElmt->getValueByName(params.UserParam1, "weak.User Param 1"))
					debug("Key 'weak.User Param 1' not found.");
				if(!psElmt->getValueByName(params.UserParam2, "weak.User Param 2"))
					debug("Key 'weak.User Param 2' not found.");
				if(!psElmt->getValueByName(params.UserParam3, "weak.User Param 3"))
					debug("Key 'weak.User Param 3' not found.");
				ps.Power.push_back(params);

				// Standard
				nlassert( CPSStruct::StandardIndex == ps.Power.size() );
				if(!psElmt->getValueByName(params.UserParam0, "standard.User Param 0"))
					debug("Key 'standard.User Param 0' not found.");
				if(!psElmt->getValueByName(params.UserParam1, "standard.User Param 1"))
					debug("Key 'standard.User Param 1' not found.");
				if(!psElmt->getValueByName(params.UserParam2, "standard.User Param 2"))
					debug("Key 'standard.User Param 2' not found.");
				if(!psElmt->getValueByName(params.UserParam3, "standard.User Param 3"))
					debug("Key 'standard.User Param 3' not found.");
				ps.Power.push_back(params);

				// Strong
				if(!psElmt->getValueByName(params.UserParam0, "strong.User Param 0"))
					debug("Key 'strong.User Param 0' not found.");
				if(!psElmt->getValueByName(params.UserParam1, "strong.User Param 1"))
					debug("Key 'strong.User Param 1' not found.");
				if(!psElmt->getValueByName(params.UserParam2, "strong.User Param 2"))
					debug("Key 'strong.User Param 2' not found.");
				if(!psElmt->getValueByName(params.UserParam3, "strong.User Param 3"))
					debug("Key 'strong.User Param 3' not found.");
				ps.Power.push_back(params);

				// Critical
				if(!psElmt->getValueByName(params.UserParam0, "critical.User Param 0"))
					debug("Key 'critical.User Param 0' not found.");
				if(!psElmt->getValueByName(params.UserParam1, "critical.User Param 1"))
					debug("Key 'critical.User Param 1' not found.");
				if(!psElmt->getValueByName(params.UserParam2, "critical.User Param 2"))
					debug("Key 'critical.User Param 2' not found.");
				if(!psElmt->getValueByName(params.UserParam3, "critical.User Param 3"))
					debug("Key 'critical.User Param 3' not found.");
				ps.Power.push_back(params);

				// Weak Combo
				if(!psElmt->getValueByName(params.UserParam0, "weak combo.User Param 0"))
					debug("Key 'weak combo.User Param 0' not found.");
				if(!psElmt->getValueByName(params.UserParam1, "weak combo.User Param 1"))
					debug("Key 'weak combo.User Param 1' not found.");
				if(!psElmt->getValueByName(params.UserParam2, "weak combo.User Param 2"))
					debug("Key 'weak combo.User Param 2' not found.");
				if(!psElmt->getValueByName(params.UserParam3, "weak combo.User Param 3"))
					debug("Key 'weak combo.User Param 3' not found.");
				ps.Power.push_back(params);

				// Strng Combo
				if(!psElmt->getValueByName(params.UserParam0, "strong combo.User Param 0"))
					debug("Key 'strong combo.User Param 0' not found.");
				if(!psElmt->getValueByName(params.UserParam1, "strong combo.User Param 1"))
					debug("Key 'strong combo.User Param 1' not found.");
				if(!psElmt->getValueByName(params.UserParam2, "strong combo.User Param 2"))
					debug("Key 'strong combo.User Param 2' not found.");
				if(!psElmt->getValueByName(params.UserParam3, "strong combo.User Param 3"))
					debug("Key 'strong combo.User Param 3' not found.");
				ps.Power.push_back(params);

				if(!ps.PSName.empty())
					PSList.push_back(ps);
			}
			else
				nlwarning("CAnimationState::build : element (%d) should be here in 'animations'.");
		}
	}
	else
		nlwarning("CFXSheet::build: there is no element 'PS_List' in the form.");
}// build //


//-----------------------------------------------
// serial :
// Serialize FX sheet into binary data file.
//-----------------------------------------------
void CFXSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(TrailList);
	f.serialCont(PSList);
}// serial //
