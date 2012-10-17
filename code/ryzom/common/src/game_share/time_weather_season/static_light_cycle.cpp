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
#include "static_light_cycle.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"

using namespace NLGEORGES;
using namespace NLMISC;
using namespace std;

//-----------------------------------------------
// CStaticLightCycle readGeorges
//-----------------------------------------------
void CStaticLightCycle::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();

		vector< string > season;
		season.push_back( string("Spring") );
		season.push_back( string("Summer") );
		season.push_back( string("Autumn") );
		season.push_back( string("Winter") );

		uint NbSeasons = 4;
		LightCycles.resize( NbSeasons );

		for( uint i = 0; i < NbSeasons; ++i )
		{
			UFormElm* SeasonElt = NULL;
			if( ! ( root.getNodeByName( &SeasonElt, season[ i ].c_str() ) && SeasonElt ) )
			{
				nlwarning("<CStaticLightCycle readGeorges> can get node %s in sheet %s", season[ i ].c_str(), sheetId.toString().c_str() );
			}
			else
			{
				// Day hour
				if( ! SeasonElt->getValueByName( LightCycles[ i ].DayHour, "DayHour" ) )
				{
					nlwarning("<CStaticLightCycle readGeorges> can get value DayHour in node %d on SeasonElt structure in sheet %s", i, sheetId.toString().c_str() );
				}

				// Day to dusk hour
				if( ! SeasonElt->getValueByName( LightCycles[ i ].DayToDuskHour, "DayToDuskHour" ) )
				{
					nlwarning("<CStaticLightCycle readGeorges> can get value DayToDuskHour in node %d on SeasonElt structure in sheet %s", i, sheetId.toString().c_str() );
				}

				// Dusk to night hour
				if( ! SeasonElt->getValueByName( LightCycles[ i ].DuskToNightHour, "DuskToNightHour" ) )
				{
					nlwarning("<CStaticLightCycle readGeorges> can get value DuskToNightHour in node %d on SeasonElt structure in sheet %s", i, sheetId.toString().c_str() );
				}

				// Night hour
				if( ! SeasonElt->getValueByName( LightCycles[ i ].NightHour, "NightHour" ) )
				{
					nlwarning("<CStaticLightCycle readGeorges> can get value NightHour in node %d on SeasonElt structure in sheet %s", i, sheetId.toString().c_str() );
				}

				// Night to day hour
				if( ! SeasonElt->getValueByName( LightCycles[ i ].NightToDayHour, "NightToDayHour" ) )
				{
					nlwarning("<CStaticLightCycle readGeorges> can get value NightToDayHour in node %d on SeasonElt structure in sheet %s", i, sheetId.toString().c_str() );
				}
			}
		}
	}
}// CStaticLightCycle readGeorges
