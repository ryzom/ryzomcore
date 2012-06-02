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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Application
#include "emot_list_sheet.h"
// NeL
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_loader.h"

///////////
// USING //
///////////

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;

/////////////
// METHODS //
/////////////

// ***************************************************************************
// CEmotListSheet
// ***************************************************************************

//-----------------------------------------------
// CEmotListSheet :
// Constructor.
//-----------------------------------------------
CEmotListSheet::CEmotListSheet()
{
	Type = CEntitySheet::EMOT;

}// CEmotListSheet //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CEmotListSheet::build(const NLGEORGES::UFormElm &item)
{
	const UFormElm *list = 0;
	item.getNodeByName(&list, "emot_list");
	if(list)
	{
		// Get the array size.
		uint size;
		list->getArraySize(size);
		Emots.clear();
		if(size != 0)
			Emots.resize(size, CAnimationStateSheet::UnknownState);
		nlinfo("There is '%d' emots in the list.", size);
		// Get emots name.
		for(uint i=0; i<size; ++i)
		{
			string result;
			list->getArrayValue(result, i);
			Emots[i] = CAnimationStateSheet::getAnimationStateId(result);
			nlinfo("- '%s'", result.c_str());
		}
	}
	else
		nlwarning("Cannot find the key 'emot_list'.");

}// build //

//-----------------------------------------------
// serial :
// Serialize a CAutomatonStateSheet.
//-----------------------------------------------
void CEmotListSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	uint32 size;
	if (f.isReading ())
	{
		f.serial (size);
		Emots.resize (size);
	}
	else
	{
		size = (uint32)Emots.size();
		f.serial (size);
	}
	for (uint i = 0; i < size; i++)
		f.serialEnum(Emots[i]);

}// serial //

