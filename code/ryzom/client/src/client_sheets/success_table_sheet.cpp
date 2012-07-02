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

#include "success_table_sheet.h"
#include "nel/georges/u_form_elm.h"


using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

// ***************************************************************************
void CSuccessTableSheet::build(const NLGEORGES::UFormElm &root)
{
	const UFormElm *array = NULL;
	if (root.getNodeByName (&array, "Chances") && array)
	{
		// Get array size
		uint size;
		array->getArraySize (size);

		SuccessTable.resize( size );

		// Get a array value
		for (uint i=0; i<size; ++i)
		{
			const UFormElm *line = NULL;
			if ( array->getArrayNode( &line, i) && line)
			{
				line->getValueByName( SuccessTable[ i ].RelativeLevel, "RelativeLevel" );
				line->getValueByName( SuccessTable[ i ].SuccessProbability, "SuccessChances" );
				line->getValueByName( SuccessTable[ i ].PartialSuccessProbability, "PartialSuccessMaxDraw" );
			}
		}
	}
}


