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
#include "hair_set.h"
#include "sheet_manager.h"
// Client Sheets
#include "client_sheets/item_sheet.h"
// Game Share
#include "game_share/slot_types.h"

#include "nel/misc/common.h"
#include "nel/misc/progress_callback.h"

// hair set manager
CHairSet HairSet;

H_AUTO_DECL(RZ_HairSet)

//=========================================================================
void CHairSet::init (NLMISC::IProgressCallback &progress)
{
	H_AUTO_USE(RZ_HairSet)
	clear();
	uint numHairItem = SheetMngr.getNumItem(SLOTTYPE::HEAD_SLOT);
	for(uint k = 0; k < numHairItem; ++k)
	{
		// Progress bar
		progress.progress ((float)k/(float)numHairItem);

		const CItemSheet *item = SheetMngr.getItem(SLOTTYPE::HEAD_SLOT, k);
		if (item && !item->getShape().empty())
		{
			if (item->getShape().find("cheveux", 0) != std::string::npos)
			{
				// get race
				std::string itemName = NLMISC::toLower(item->getShape());

				// fortunately, first character of each race is distinct
				switch(itemName[0])
				{
					case 'm': _Hairs[Matis].push_back(k); break;
					case 't': _Hairs[Tryker].push_back(k); break;
					case 'z': _Hairs[Zorai].push_back(k); break;
					case 'f': _Hairs[Fyros].push_back(k); break;
				}
			}
		}
	}
}

//=========================================================================
void CHairSet::clear()
{
	H_AUTO_USE(RZ_HairSet)
	for(uint k = 0; k < NumPeople; ++k)
	{
		NLMISC::contReset(_Hairs[k]);
	}
}

//=========================================================================
uint CHairSet::getNumHairItem(EGSPD::CPeople::TPeople gspeople) const
{
	H_AUTO_USE(RZ_HairSet)
	EPeople people = convPeople(gspeople);
	return people != DontKnow ? (uint)_Hairs[people].size() : 0;
}


//=========================================================================
sint CHairSet::getHairItemId(EGSPD::CPeople::TPeople gspeople, uint index) const
{
	H_AUTO_USE(RZ_HairSet)
	EPeople people = convPeople(gspeople);
	if (people == DontKnow) return -1;
	if (index > _Hairs[people].size()) return -1;
	return (sint) _Hairs[people][index];
}


//=========================================================================
sint CHairSet::getHairItemFromId(EGSPD::CPeople::TPeople gspeople, uint Id) const
{
	H_AUTO_USE(RZ_HairSet)
	EPeople people = convPeople(gspeople);
	if (people == DontKnow) return -1;
	TIntArray::const_iterator it = std::find(_Hairs[people].begin(), _Hairs[people].end(), Id);
	if (it != _Hairs[people].end()) return (sint)(it - _Hairs[people].begin());
	else return -1;
}

//=========================================================================
CHairSet::EPeople CHairSet::convPeople(EGSPD::CPeople::TPeople people)
{
	H_AUTO_USE(RZ_HairSet)
	switch(people)
	{
		case EGSPD::CPeople::Fyros: return Fyros;
		case EGSPD::CPeople::Matis: return Matis;
		case EGSPD::CPeople::Tryker: return Tryker;
		case EGSPD::CPeople::Zorai: return Zorai;
		default: return DontKnow;
	}
}

//=========================================================================
EGSPD::CPeople::TPeople CHairSet::convPeople(CHairSet::EPeople people)
{
	H_AUTO_USE(RZ_HairSet)
	if (people > NumPeople) return EGSPD::CPeople::EndPeople;
	static const EGSPD::CPeople::TPeople peopleConv[] =
	{
		EGSPD::CPeople::Fyros,
		EGSPD::CPeople::Matis,
		EGSPD::CPeople::Tryker,
		EGSPD::CPeople::Zorai,
	};
	return peopleConv[people];
}


//=========================================================================
bool CHairSet::isHairItemId(uint id) const
{
	H_AUTO_USE(RZ_HairSet)
	for(uint k = 0; k < NumPeople; ++k)
	{
		if (getHairItemFromId(convPeople((CHairSet::EPeople) k), id) != -1) return true;
	}
	return false;
}

//=========================================================================
EGSPD::CPeople::TPeople CHairSet::getPeopleFromHairItemID(uint id) const
{
	H_AUTO_USE(RZ_HairSet)
	for(uint k = 0; k < NumPeople; ++k)
	{
		if (getHairItemFromId(convPeople((CHairSet::EPeople) k), id) != -1) return convPeople((CHairSet::EPeople) k);
	}
	return EGSPD::CPeople::EndPeople;
}
