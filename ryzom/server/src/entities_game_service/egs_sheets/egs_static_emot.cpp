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
#include "egs_sheets/egs_static_emot.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"



using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;



void CStaticEmot::readGeorges(const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	const UFormElm *pElt;
	nlverify (root.getNodeByName (&pElt, "emot_list"));
	uint size;
	nlverify (pElt->getArraySize (size));
	string value;
	_Anims.reserve(size);
	for (uint32 i = 0; i < size; ++i)
	{
	//	const UFormElm *pEltOfList;
	//	// Get the continent
	//	if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
	//	{
	//	}
		_Anims.push_back( string() );
		nlverify( pElt->getArrayValue( _Anims.back(), i ) );
	}
	buildAnimIdMap();
}

void CStaticEmot::buildAnimIdMap()
{
	size_t size = _Anims.size();
	for (size_t i=0; i<size; ++i)
		_AnimIdMap.insert(make_pair(_Anims[i], i));
}

