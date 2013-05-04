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
#include "id_to_string_array.h"
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;

// ****************************************************************************************
CIDToStringArraySheet::CIDToStringArraySheet()
{
	Type = ID_TO_STRING_ARRAY;
}

// *******************************************************************************************
void CIDToStringArraySheet::build(const NLGEORGES::UFormElm &item)
{
	const UFormElm *stringArray = NULL;
	if (item.getNodeByName(&stringArray, "Array") && stringArray)
	{
		std::string str;
		uint numStr;
		nlverify(stringArray->getArraySize(numStr));
		Array.reserve(numStr);
		for(uint k = 0; k < numStr; ++k)
		{
			const UFormElm *strNode = NULL;
			if (stringArray->getArrayNode(&strNode, k) && strNode)
			{
				Array.push_back(CIDToString());
				Array.back().build(*strNode);
			}
		}
	}
}

// *******************************************************************************************
void CIDToStringArraySheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(Array);
}

// *******************************************************************************************
void CIDToString::build(const NLGEORGES::UFormElm &item)
{
	item.getValueByName(String, "String");
	item.getValueByName(ID, "ID");
}

// *******************************************************************************************
void CIDToString::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(ID);
	f.serial(String);
}

