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
#include "egs_static_xp_factor_table.h"
#include "egs_sheets.h"
//Nel georges
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

//--------------------------------------------------------------
// serial
//--------------------------------------------------------------
void CStaticXpFactorTable::serial(class NLMISC::IStream &f)
{
	f.serialCont(_XpFactorTable);
} // serial //


//--------------------------------------------------------------
// readGeorges
//--------------------------------------------------------------
void CStaticXpFactorTable::readGeorges (const CSmartPtr<UForm> &form, const CSheetId &sheetId)
{
	if (!form)
	{
		nlwarning("Error reading sheet %s, form == NULL", sheetId.toString().c_str());
		return;
	}

	UFormElm& root = form->getRootNode();		
	
	const UFormElm *array = NULL;
	if (root.getNodeByName (&array, "XpFactor") && array)
    {
		// Get array size
        uint size;
		array->getArraySize (size);
		
		_XpFactorTable.resize( size );
		
        // Get an array value
        for (uint i=0; i<size; ++i)
        {
			array->getArrayValue(_XpFactorTable[i], i);
        }
	}
} // readGeorges //

