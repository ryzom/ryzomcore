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
#include "text_emot_list_sheet.h"
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
// CTextEmotListSheet :
// Constructor.
//-----------------------------------------------
CTextEmotListSheet::CTextEmotListSheet()
{
	Type = CEntitySheet::TEXT_EMOT;

}// CEmotListSheet //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CTextEmotListSheet::build(const NLGEORGES::UFormElm &item)
{
	const UFormElm *list = 0;
	item.getNodeByName(&list, "Array");
	if(list)
	{
		// Get the array size.
		uint size;
		list->getArraySize(size);
		TextEmotList.clear();
		if(size != 0)
			TextEmotList.resize(size);

		// Get emots name.
		for(uint i=0; i<size; ++i)
		{
			const UFormElm *node = 0;
			list->getArrayNode(&node, i);
			if (node != NULL)
			{
				string path, anim, emoteId, fxToSpawn;
				float fxSpawnDelay = 0.f;
				float fxSpawnDist  = 0.5f;
				bool usableFromClientUI = true;
				node->getValueByName(path, "Access Path");
				node->getValueByName(anim, "Animation");
				node-> getValueByName(emoteId, "EmoteId");
				node->getValueByName(fxToSpawn, "FxToSpawn");
				node->getValueByName(fxSpawnDelay, "FxSpawnDelay");
				node->getValueByName(fxSpawnDist, "FxSpawnDist");
				node->getValueByName(usableFromClientUI, "Usable From Client UI");
				TextEmotList[i].Path = path;
				TextEmotList[i].Anim = anim;
				TextEmotList[i].EmoteId = emoteId;
				TextEmotList[i].FXToSpawn = fxToSpawn;
				TextEmotList[i].FXSpawnDelay = fxSpawnDelay;
				TextEmotList[i].FXSpawnDist = fxSpawnDist;
				TextEmotList[i].UsableFromClientUI = usableFromClientUI;
			}
		}
	}

}// build //

//-----------------------------------------------
// serial :
// Serialize a CAutomatonStateSheet.
//-----------------------------------------------
void CTextEmotListSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(TextEmotList);

}// serial //

