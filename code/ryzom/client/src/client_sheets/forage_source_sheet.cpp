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



#include "stdpch.h"	// First include for pre-compiled headers.
#include "forage_source_sheet.h"
#include "nel/georges/u_form_elm.h"

using namespace std;
using namespace NLGEORGES;


/*
 * Constructor
 */
//CForageSourceSheet::CForageSourceSheet()
//{
//}


/*
 * Build the sheet from an external script.
 */
void CForageSourceSheet::build(const NLGEORGES::UFormElm &item)
{
	if ( (! item.getValueByName( FxFilename, "FX" )) || (FxFilename.empty()) )
		debug( "FX not found in forage_source sheet" );
	if ( (! item.getValueByName( FxSafeFilename, "FXSafe" )) || (FxSafeFilename.empty()) )
		debug( "FXSafe not found in forage_source sheet" );
	if ( ! item.getValueByName( Knowledge, "Knowledge" ) )
		debug( "Knowledge not found in forage_source sheet" );
	const UFormElm *array;
	if ( ! (item.getNodeByName( &array, "Icons" ) && array) )
		debug( "Icons not found in forage_source sheet" );
	else
	{
		uint nbIcons;
		array->getArraySize( nbIcons );
		for ( uint i=0; i!=nbIcons; ++i )
		{
			string iconFilename;
			array->getArrayValue( iconFilename, i );
			Icons.push_back( iconFilename );
		}
	}
}


/*
 * Serialize character sheet into binary data file.
 */
void CForageSourceSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial( FxFilename );
	f.serial( FxSafeFilename );
	f.serial( Knowledge );
	f.serialCont( Icons );
}
