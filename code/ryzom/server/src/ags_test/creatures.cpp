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




// Misc
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
// Georges
//#include "nel/georges/u_form.h"
//#include "nel/georges/u_form_elm.h"
//#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"
// Local
#include "creatures.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;

namespace AGS_TEST
{

//-------------------------------------------------------------------------
// the singleton data

std::map<CSheetId,CCreatures::CCreatureRecord> CCreatures::_creatures;
bool CCreatures::_initialised=false;


//-------------------------------------------------------------------------
// init

void CCreatures::init()
{
	if (_initialised)
		return;

	std::vector<std::string> filters;
	filters.push_back("creature");
	filters.push_back("player");

	loadForm(filters, "ags.packed_sheets", _creatures);

	_initialised=true;
}


//-------------------------------------------------------------------------
// display

void CCreatures::display()
{
	nlassert(_initialised);

	std::map<CSheetId,CCreatures::CCreatureRecord>::iterator it;
	for(it=_creatures.begin();it!=_creatures.end();++it)
	{
		nlinfo("CREATURE: %s Walk: %f Run: %f",(*it).first.toString().c_str(),
			(*it).second._walkSpeed,(*it).second._runSpeed);
	}
}


//-------------------------------------------------------------------------
// lookup

const CCreatures::CCreatureRecord *CCreatures::lookup( CSheetId id )
{
	nlassert(_initialised);

	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId,CCreatures::CCreatureRecord>::iterator it;
	it=_creatures.find(id);

	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it!=_creatures.end())
		return &((*it).second);
	else
		return 0;
}



} // end of namespace AGS_TEST
