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



#ifndef CL_VISUAL_SLOT_MANAGER_H
#define CL_VISUAL_SLOT_MANAGER_H

/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
// Game Share
#include "slot_types.h"


///////////
// CLASS //
///////////
/**
 * Manage visual slots.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2003
 */
class CVisualSlotManager
{
////////////
// PUBLIC //
public:
	/**
	 * Instanciate CVisualSlotManager. There can be only one instance (singleton)
	 * \return CVisualSlotManager * : Pointer on CVisualSlotManager.
	 */
	static CVisualSlotManager *getInstance();

	// release memory
	static void releaseInstance();

public:
	typedef struct
	{
		uint32				Index;
		NLMISC::CSheetId	SheetId;

		/// Load/Save the values using the serial system
		void serial(class NLMISC::IStream &s) throw(NLMISC::EStream)
		{
			s.serial(Index);
			s.serial(SheetId);
		}
	} TElement;

	typedef struct
	{
		// elements list for a visual slot.
		std::vector<TElement> Element;

		/// Load/Save the values using the serial system
		void serial(class NLMISC::IStream &s) throw(NLMISC::EStream)
		{
			s.serialCont(Element);
		}
	} TElementList;

	typedef std::vector <TElementList> TVisualSlot;

	typedef struct
	{
		uint32					Index;
		SLOTTYPE::EVisualSlot	VisualSlot;
	} TIdxbyVS;

public:
	/// Return the visual slot index from a sheet Id for items in right hand.
	uint32 rightItem2Index(const NLMISC::CSheetId &id);
	/// Return the visual slot index from a sheet Id for items in left hand.
	uint32 leftItem2Index(const NLMISC::CSheetId &id);
	/// Return the visual index from a sheet Id and the visual slot.
	uint32 sheet2Index(const NLMISC::CSheetId &id, SLOTTYPE::EVisualSlot slot);
	// Return the sheet Id from visual index and the visual slot.
	NLMISC::CSheetId * index2Sheet(uint32 index, SLOTTYPE::EVisualSlot slot);
	/// ...
	void sheet2Index(const NLMISC::CSheetId &id, std::vector<TIdxbyVS> &result);
	/// Return the number of different index for a visual slot.
	uint32 getNbIndex(SLOTTYPE::EVisualSlot slot) const;


///////////////
// PROTECTED //
protected:
	/// Constructor
	CVisualSlotManager();
	/// Initialize the class
	void init();


/////////////
// PRIVATE //
private:
	// The only one instance of the class
	static CVisualSlotManager	*_Instance;
private:
	//
	TVisualSlot					_VisualSlot;
};


#endif // CL_VISUAL_SLOT_MANAGER_H

/* End of visual_slot_manager.h */
