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



#ifndef CL_SHEET_MANAGER_H
#define CL_SHEET_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/progress_callback.h"
// Application
#include "client_sheets/entity_sheet.h"
#include "client_sheets/character_sheet.h"
#include "client_sheets/player_sheet.h"
#include "client_sheets/fx_sheet.h"
#include "client_sheets/forage_source_sheet.h"
#include "client_sheets/building_sheet.h"
#include "client_sheets/item_sheet.h"
#include "client_sheets/pact_sheet.h"
#include "client_sheets/plant_sheet.h"
#include "client_sheets/mission_sheet.h"
#include "client_sheets/mission_icon_sheet.h"
#include "client_sheets/race_stats_sheet.h"
#include "client_sheets/light_cycle_sheet.h"
#include "client_sheets/weather_setup_sheet.h"
#include "client_sheets/continent_sheet.h"
#include "client_sheets/world_sheet.h"
#include "client_sheets/weather_function_params_sheet.h"
#include "client_sheets/sbrick_sheet.h"
#include "client_sheets/sphrase_sheet.h"
#include "client_sheets/skills_tree_sheet.h"
#include "client_sheets/unblock_titles_sheet.h"
#include "client_sheets/success_table_sheet.h"
#include "client_sheets/automaton_list_sheet.h"
#include "client_sheets/animation_fx_sheet.h"
#include "client_sheets/id_to_string_array.h"
#include "client_sheets/emot_list_sheet.h"
#include "client_sheets/flora_sheet.h"
#include "client_sheets/attack_list_sheet.h"
#include "client_sheets/sky_sheet.h"
#include "client_sheets/text_emot_list_sheet.h"
#include "client_sheets/outpost_sheet.h"
#include "client_sheets/outpost_building_sheet.h"
#include "client_sheets/outpost_squad_sheet.h"


// game share
#include "game_share/slot_types.h"
#include "game_share/skills.h"
// Std
#include <map>
#include <string>


///////////
// CLASS //
///////////
namespace NLMISC
{
	class CSheetId;
}

namespace NLGEORGES
{
	class UFormLoader;
	class UForm;
}

/**
 * Class to manage all sheets.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CSheetManagerEntry
{

public:
	/// The data which will be filled in readGeorges and serial
	CEntitySheet	*EntitySheet;

public:

	/// Constructor
	CSheetManagerEntry ();
	virtual ~CSheetManagerEntry ();

	/// Load the values using the george sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

	/// Load/Save the values using the serial system
	void serial (NLMISC::IStream &s);
	// Useful for serial
	void initSheet (CEntitySheet *pES, NLMISC::IStream &s, CEntitySheet::TType type);


	/**
	 * Event to implement any action when the sheet is no longer existent.
	 * This method is called when a sheet have been read from the packed sheet
	 * and the associated sheet file no more exist in the directories.
	 */
	void removed();

	static uint getVersion();
	static void setVersion(uint version);
};

/**
 * Class to manage all sheets.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CSheetManager
{
	friend class CSheetManagerEntry;

protected:
	sint	_NbEyesColor;
	sint	_NbHairColor;

public:

	/// this is the structure that hold all the sheets needed
	typedef std::map<NLMISC::CSheetId, CSheetManagerEntry>	TEntitySheetMap;

	typedef std::pair<SLOTTYPE::EVisualSlot, uint>			TVisualSlotItem;
	typedef std::vector<TVisualSlotItem>					TVisualSlotItemArray;

public:
	/// A ptr on a form loader
	static NLGEORGES::UFormLoader		*FormLoader;

public:
	/// Constructor
	CSheetManager();
	/// Destructor
	~CSheetManager();

	// release memory
	void release();

	/// Load all sheets.
	/// \param needComputeVS : if 'true' this will compute the visual slot file too.
	void load        (NLMISC::IProgressCallback &callBack, bool updatePackedSheet, bool needComputeVS = false, bool dumpVSIndex = false);
	/// Load all sheet or a subset of them (if 'extensions' is not NULL, them the extensions list is taken from that vector)
	void loadAllSheet(NLMISC::IProgressCallback &callBack, bool updatePackedSheet, bool needComputeVS, bool dumpVSIndex, bool forceRecompute = false, const std::vector<std::string> *extensions = NULL);
	/// Load all sheet, without packed sheet, and with a wildcard (for reload feature)
	void loadAllSheetNoPackedSheet(NLMISC::IProgressCallback &callBack, const std::vector<std::string> &extensions, const std::string &wildcardFilter);

	/**
	 * Get a sheet from is number.
	 * \param CSheetId num : sheet number.
	 * \return CEntitySheet * : pointer on the sheet according to the param or 0 if any pb.
	 */
	CEntitySheet *get(NLMISC::CSheetId num);

	/// Get the number of available items for the given visual slot
	uint		getNumItem(SLOTTYPE::EVisualSlot slot);

	/// Get the pointer on an item form from an index.
	CItemSheet *getItem(SLOTTYPE::EVisualSlot slot, uint index);

	// Get a pair of visual slots / index from a CItemSheet pointer.
	const TVisualSlotItemArray *getVSItems(CItemSheet *sheet) const;

	// From an item name and a slot, get its item, or -1 if not found
	sint						getVSIndex(const std::string &itemName, SLOTTYPE::EVisualSlot slot);

	/// Return the number of color available for the eyes.
	sint nbEyesColor() {return _NbEyesColor;}
	/// Return the number of color available for the hair.
	sint nbHairColor() {return _NbHairColor;}


	/// Get all sheets (useful for other managers (skill, brick, ...))
	// @{
	const TEntitySheetMap & getSheets() { return _EntitySheetContainer; }
	// @}

	// tmp : dump the visual slots
	void dumpVisualSlots();

	/// dump all visual slots indexes in a file
	void dumpVisualSlotsIndex();

	/// Set output data path
	void setOutputDataPath(const std::string &dataPath) { _OutputDataPath = dataPath; }

	/// Return output data path
	const std::string& getOutputDataPath() const { return _OutputDataPath; }

private:
	typedef std::vector<CItemSheet *>	TItemVector;
	typedef std::vector<TItemVector>	TSlots;
	TSlots								_VisualSlots;

	// directory where to create .packed_sheets
	std::string							_OutputDataPath;


protected:

	/// this structure is fill by the loadForm() function and will contain all the sheets needed
	TEntitySheetMap _EntitySheetContainer;


	typedef std::map<CItemSheet *, TVisualSlotItemArray> TItemSheet2SlotItemArray;

	// Associate sheet to visual slots
	TItemSheet2SlotItemArray							_SheetToVS;

protected:
	/** Processing the sheet.
	 * \param sheet : sheet to process.
	 */
	void processSheet (CEntitySheet *sheet);
	/// compute Visual Slots for this sheet.
	void computeVS();
	///
	void loadTyp();
};


////////////
// EXTERN //
////////////
// Sheet manager.
extern CSheetManager	SheetMngr;


#endif // CL_SHEET_MANAGER_H

/* End of sheet_manager.h */
