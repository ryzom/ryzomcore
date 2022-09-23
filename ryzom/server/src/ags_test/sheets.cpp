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
#include "nel/misc/path.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"
// Local
#include "sheets.h"


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

std::map<CSheetId,CSheets::CSheet> CSheets::_Sheets;
bool CSheets::_Initialised=false;

//-------------------------------------------------------------------------
// init

void CSheets::init()
{
	if (_Initialised)
		return;

	std::vector<std::string> filters;
	filters.push_back("creature");
	filters.push_back("player");

	loadForm(filters, "data_shard/ags_test.packed_sheets", _Sheets);

	_Initialised=true;
}


//-------------------------------------------------------------------------
// display

void CSheets::display(NLMISC::CLog *log)
{
	nlassert(_Initialised);

	std::map<CSheetId,CSheets::CSheet>::iterator it;
	for(it=_Sheets.begin();it!=_Sheets.end();++it)
	{
		log->displayNL("SHEET:%s Walk:%f Run:%f Radius:%f Height:%f Bounding:%f Type:%s ChatType: %s",(*it).first.toString().c_str(),
			(*it).second.WalkSpeed, 
			(*it).second.RunSpeed, 
			(*it).second.Radius, 
			(*it).second.Height, 
			(*it).second.BoundingRadius, 
//			(*it).second.Name.c_str(),
			(*it).second.isNpc?"NPC": "CREATURE",
			(std::string()+((*it).second.CanChatTP?"TP ":"")+((*it).second.CanChatTrade?"MERCHANT ":"")+((*it).second.CanChatMission?"MISSION ":"")).c_str()
			);
	}
}


//-------------------------------------------------------------------------
// lookup

const CSheets::CSheet *CSheets::lookup( CSheetId id )
{
	nlassert(_Initialised);

	// setup an iterator and lookup the sheet id in the map
	std::map<CSheetId,CSheets::CSheet>::iterator it=_Sheets.find(id);

	// if we found a valid entry return a pointer to the creature record otherwise 0
	if (it!=_Sheets.end())
		return &((*it).second);
	else
		return NULL;
}

} //namespace AGS_TEST

