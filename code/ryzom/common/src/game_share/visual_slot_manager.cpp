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
#include "stdpch.h"
// Misc
#include "nel/misc/file.h"
#include "nel/misc/stream.h"
#include "nel/misc/path.h"
// Game Share
#include "slot_types.h"
#include "visual_slot_manager.h"


////////////////////
// STATIC MEMBERS //
////////////////////
CVisualSlotManager	*CVisualSlotManager::_Instance = 0;


////////////////////
// STATIC METHODS //
////////////////////
//---------------------------------------------------
// getInstance :
// Instanciate CVisualSlotManager. There can be only one instance (singleton)
// \return CVisualSlotManager * : Pointer on CVisualSlotManager.
//---------------------------------------------------
CVisualSlotManager * CVisualSlotManager::getInstance()
{
	if(_Instance == 0)
	{
		_Instance = new CVisualSlotManager();
		if(_Instance)
			_Instance->init();
	}
	return _Instance;
}// getInstance //

// release memory
void CVisualSlotManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CVisualSlotManager :
// Constructor
//-----------------------------------------------
CVisualSlotManager::CVisualSlotManager()
{
}// CVisualSlotManager //


//-----------------------------------------------
// init :
// Load a visual slot file.
//-----------------------------------------------
void CVisualSlotManager::init()
{
	const std::string filename = "visual_slot.tab";

	// Open the file.
	NLMISC::CIFile f;
	if(f.open(NLMISC::CPath::lookup(filename, false, false)))
	{
		// Dump entities.
		f.serialCont(_VisualSlot);

		// Close the File.
		f.close();

		// Display elements read.
		for(uint i=0, len = _VisualSlot.size(); i<len; ++i)
		{
			_VisualSlot[i].updateMaps();
//			for(uint j=0; j<_VisualSlot[i].Element.size(); ++j)
//				nlinfo("Visu: %d Num: %d Id: %d sheet: %s", i, _VisualSlot[i].Element[j].Index, _VisualSlot[i].Element[j].SheetId.asInt(), _VisualSlot[i].Element[j].SheetId.toString().c_str());
		}
	}
	else
		nlwarning("VSMngr:load: cannot open the file '%s'.", filename.c_str());
}// init //

void CVisualSlotManager::TElementList::updateMaps()
{
	SheetIdToIndexMap.clear();

	for(uint i=0, len = Element.size(); i<len; ++i)
	{
		const TElement &e = Element[i];
		SheetIdToIndexMap[e.SheetId] = e.Index;
	}
}

//-----------------------------------------------
// rightItem2Index :
// Return the visual slot index from a sheet Id for items in right hand.
//-----------------------------------------------
uint32 CVisualSlotManager::rightItem2Index(const NLMISC::CSheetId &id)
{
	return sheet2Index(id, SLOTTYPE::RIGHT_HAND_SLOT);
}// rightItem2Index //


//-----------------------------------------------
// leftItem2Index :
// Return the visual slot index from a sheet Id for items in left hand.
//-----------------------------------------------
uint32 CVisualSlotManager::leftItem2Index(const NLMISC::CSheetId &id)
{
	return sheet2Index(id, SLOTTYPE::LEFT_HAND_SLOT);
}// leftItem2Index //


//-----------------------------------------------
// sheet2Index :
// Return the visual index from a sheet Id and the visual slot.
//-----------------------------------------------
uint32 CVisualSlotManager::sheet2Index(const NLMISC::CSheetId &id, SLOTTYPE::EVisualSlot slot)
{
	if((uint)slot < _VisualSlot.size())
	{
		const TElementList &el = _VisualSlot[slot];
		TElementList::SheetIdToIndexMapType::const_iterator it = el.SheetIdToIndexMap.find(id);
		if (it != el.SheetIdToIndexMap.end()) return it->second;
	}
	else
		nlwarning("VSMngr:sheet2Index: Bad slot '%d' -> you probably need to rebuild the tab.", (sint)slot);

	// No Item
	return 0;
}// sheet2Index //

//-----------------------------------------------
// index2Sheet :
// Return the sheet Id from visual index and the visual slot.
//-----------------------------------------------
NLMISC::CSheetId * CVisualSlotManager::index2Sheet(uint32 index, SLOTTYPE::EVisualSlot slot)
{
	if((uint)slot < _VisualSlot.size())
	{
		for(uint i=0; i<_VisualSlot[slot].Element.size(); ++i)
			if(_VisualSlot[slot].Element[i].Index == index)
				return &_VisualSlot[slot].Element[i].SheetId;
	}
	else
		nlwarning("VSMngr:index2Sheet: Bad slot '%d' -> you probably need to rebuild the tab.", (sint)slot);

	// No SheetId
	return NULL;
}

//-----------------------------------------------
// sheet2Index :
// \warning The result vector is not cleared before beeing filled.
//-----------------------------------------------
void CVisualSlotManager::sheet2Index(const NLMISC::CSheetId &id, std::vector<TIdxbyVS> &result)
{
	for(uint i=0; i<_VisualSlot.size(); ++i)
	{
		for(uint j=0; j<_VisualSlot[i].Element.size(); ++j)
		{
			if(_VisualSlot[i].Element[j].SheetId == id)
			{
				TIdxbyVS idxbyVS;
				idxbyVS.Index      = _VisualSlot[i].Element[j].Index;
				idxbyVS.VisualSlot = (SLOTTYPE::EVisualSlot)i;
				result.push_back(idxbyVS);
			}
		}
	}
}// sheet2Index //


//-----------------------------------------------
// getNbIndex :
// Return the number of different index for a visual slot.
//-----------------------------------------------
uint32 CVisualSlotManager::getNbIndex(SLOTTYPE::EVisualSlot slot) const
{
	uint32 count = 0;

	if((uint)slot < _VisualSlot.size())
	{
		for(uint i=0; i<_VisualSlot[slot].Element.size(); ++i)
			if(_VisualSlot[slot].Element[i].Index > count)
				count = _VisualSlot[slot].Element[i].Index;
	}
	else
		nlwarning("VSMngr:getNbIndex: Bad slot '%d' -> you probably need to rebuild the tab.", (sint)slot);

	// No Item
	return count;
}// getNbIndex //
