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

// Misc
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"

#include "nel/net/service.h"
// Local
#include "sheets.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace NLGEORGES;

//-------------------------------------------------------------------------
// the singleton data

std::map<CSheetId,CGpmSheets::CSheet> CGpmSheets::_sheets;
bool CGpmSheets::_initialised=false;


//-------------------------------------------------------------------------
// init

void CGpmSheets::init()
{
	if (_initialised)
		return;

	CSheetId::init(0);

	std::vector<std::string> filters;
	filters.push_back("creature");
	filters.push_back("player");

	loadForm(filters, IService::getInstance()->WriteFilesDirectory.toString()+"gpms.packed_sheets", _sheets);

	_initialised=true;
}


//-------------------------------------------------------------------------
// display

void CGpmSheets::display()
{
	nlassert(_initialised);

	std::map<CSheetId,CGpmSheets::CSheet>::iterator it;
	for(it=_sheets.begin();it!=_sheets.end();++it)
	{
		nlinfo("SHEET:%s Walk:%f Run:%f Radius:%f Height:%f Bounding:%f Scale:%f",(*it).first.toString().c_str(),
			(*it).second.WalkSpeed, (*it).second.RunSpeed, (*it).second.Radius, (*it).second.Height, (*it).second.BoundingRadius, (*it).second.Scale);
	}
}


//-------------------------------------------------------------------------
// lookup

const CGpmSheets::CSheet *CGpmSheets::lookup( CSheetId id )
{
	nlassert(_initialised);

	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId,CGpmSheets::CSheet>::iterator it;
	it=_sheets.find(id);

	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it!=_sheets.end())
		return &((*it).second);
	else
		return 0;
}


