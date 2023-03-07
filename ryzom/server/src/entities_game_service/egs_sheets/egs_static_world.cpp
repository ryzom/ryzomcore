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
#include "egs_sheets/egs_static_world.h"

#include "game_share/continent.h"

#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"



using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


void CStaticWorld::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	const UFormElm *pElt;
	nlverify (root.getNodeByName (&pElt, "continents list"));
	uint size;
	nlverify (pElt->getArraySize (size));
	string value;
	
	Continents.reserve(size);
	for (uint32 i = 0; i <size; ++i)
	{
		const UFormElm *pEltOfList;
		// Get the continent
		if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
		{
			nlverify( pEltOfList->getValueByName( value, "continent_name" ) );
			CSheetId sheet( value + ".continent" );
			if ( sheet == CSheetId::Unknown )
				nlerror("CStaticWorld sheet '%s' is invalid. ryzom.world is broken",value.c_str() );

			uint contId = ( CONTINENT::TContinent ) CONTINENT::toContinent( value );
			if ( contId == CONTINENT::UNKNOWN )
			{
				nlverify( pEltOfList->getValueByName( value, "selection_name" ) );
				contId = ( CONTINENT::TContinent ) CONTINENT::toContinent( value );
			}
			if ( contId != CONTINENT::UNKNOWN )
			{
				if ( contId >= Continents.size() )
					Continents.resize( contId + 1 );
				Continents[contId] = sheet;
			}
			else
				nlwarning("CStaticWorld continent '%s' is invalid. ryzom.world is broken",value.c_str() );
		}
	}		
}

///////////////////////////////////////////////////////////////////////////////////////////

void CStaticContinent::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	const UFormElm *elm;
	string value;
	if(root.getNodeByName(&elm, "ZCs") && elm)
	{
		uint nbZC;
		nlverify(elm->getArraySize(nbZC));
		Outposts.reserve( nbZC );
		for(uint i = 0; i < nbZC; ++i)
		{
			// Village pointer
			const UFormElm *zc;
			if(elm->getArrayNode(&zc, i) && zc)
				nlverify(zc->getValueByName(value, "outpost_number"));
			uint index;
			NLMISC::fromString(value, index);
			Outposts.push_back( index );
		}
	}
}



