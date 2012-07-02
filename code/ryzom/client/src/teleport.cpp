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
#include "stdpch.h"
// Client
#include "teleport.h"
// Misc
#include "nel/misc/vectord.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


////////////
// METHOD //
////////////
const NLMISC::CVectorD CTeleport::Unknown = NLMISC::CVectorD(-1.0, -1.0, -1.0);
CTeleport::TDestinations CTeleport::_Destinations;


//-----------------------------------------------
// load :
// Load Destinations.
//-----------------------------------------------
void CTeleport::load(const std::string &/* filename */)
{
/*
// TRAP : no more teleport list to load from georges file if you want this feature again ask me

	// Clear old destinations.
	_Destinations.clear();

	// Load the Form.
	CSmartPtr<UForm> form = 0;
	NLGEORGES::UFormLoader *formLoader = UFormLoader::createLoader();
	if(formLoader)
	{
		form = formLoader->loadForm(filename.c_str());
		if(!form)
		{
			nlwarning("CTeleport::load: can't load form '%s'.", filename.c_str());
			return;
		}
	}
	else
	{
		nlwarning("CTeleport::load: cannot create an UFormLoader *.");
		return;
	}

	// Get the root.
	const UFormElm& rootElmt = form->getRootNode();


	// Get animations.
	const UFormElm *elmt = 0;
	rootElmt.getNodeByName(&elmt, "list");
	if(elmt)
	{
		uint arrawSize;
		elmt->getArraySize(arrawSize);
		// Get all animation for the State.
		for(uint i = 0; i<arrawSize; ++i)
		{
			const UFormElm *tpElmt;
			elmt->getArrayNode(&tpElmt, i);
			if(tpElmt)
			{
				// Destination name.
				string destName;
				if(tpElmt->getValueByName(destName, "name"))
				{
					// All in UPPER CASE to not be CASE SENSITIVE.
					NLMISC::strlwr(destName);

					// Get the position
					CVector pos;
					if(tpElmt->getValueByName(pos.x, "position.X")
					&& tpElmt->getValueByName(pos.y, "position.Y")
					&& tpElmt->getValueByName(pos.z, "position.Z"))
					{
						_Destinations.insert(make_pair(destName, pos));
					}
					else
						nlwarning("CTeleport::load: Cannot find the one of the key 'position.X or Y or Z' for the element %d.", i);
				}
				else
					nlwarning("CTeleport::load: Cannot find the key 'name' for the element %d.", i);
			}
			else
				nlwarning("CTeleport::load: element (%d) should be here in 'list'.", i);
		}
	}
	else
		nlwarning("CTeleport::load: there is no element 'list'.");

	// Release the loader.
	if(formLoader)
	{
		UFormLoader::releaseLoader(formLoader);
		formLoader = 0;
	}
*/
}// load //

//-----------------------------------------------
// getPos :
// Get the destination position or CTeleport::Unknown.
//-----------------------------------------------
const NLMISC::CVectorD &CTeleport::getPos(const std::string &/* dest */)
{
	return Unknown;
/*
	TDestinations::const_iterator it = _Destinations.find(dest);
	if(it == _Destinations.end())
		return Unknown;
	else
		return (*it).second;
*/
}// getPos //
