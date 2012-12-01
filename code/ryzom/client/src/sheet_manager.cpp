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
// Misc
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sheet_id.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_type.h"
#include "nel/georges/load_form.h"
// Client
#include "sheet_manager.h"
//#include "client_cfg.h"
#include "client_sheets/entity_sheet.h"
#include "client_sheets/faction_sheet.h"
// Game Share
#include "game_share/visual_slot_manager.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;


/////////////
// GLOBALS //
/////////////
// Sheet manager.
CSheetManager	SheetMngr;
UFormLoader		*CSheetManager::FormLoader = NULL;
//COFile			fItemAssoc;
//bool			ItemAssocFileOpen = false;

// there can be several instance of CSheetManager (for reload operations) -> ctruct the loader here rather than in CSheetManager ctor
class CFormLoaderInit
{
public:
	CFormLoaderInit()
	{
		nlassert (CSheetManager::FormLoader == NULL);
		CSheetManager::FormLoader = UFormLoader::createLoader ();
		nlassert (CSheetManager::FormLoader != NULL);
	}
	~CFormLoaderInit()
	{
		if (CSheetManager::FormLoader)
		{
			UFormLoader::releaseLoader (CSheetManager::FormLoader);
			CSheetManager::FormLoader = NULL;
		}
	}
};


static CFormLoaderInit FormLoadInit;

class CTypeVersion
{
public:
	std::string     Type;
	uint            Version;

	CTypeVersion(std::string type, uint version) {Type=type; Version=version;}
};

CTypeVersion TypeVersion [] =
{
	CTypeVersion("creature",                17),
//	CTypeVersion("player",                  0),
	CTypeVersion("fx",                      0),
	CTypeVersion("building",                2),
	CTypeVersion("sitem",                   42),
	CTypeVersion("item",                    42),
	CTypeVersion("plant",                   5),
	CTypeVersion("death_impact",            0),
//	CTypeVersion("mission",                 0),
	CTypeVersion("race_stats",              3),
	CTypeVersion("light_cycle",             0),
	CTypeVersion("weather_setup",           1),
	CTypeVersion("continent",               12),
	CTypeVersion("world",                   1),
	CTypeVersion("weather_function_params", 2),
	CTypeVersion("mission_icon",            0),
	CTypeVersion("sbrick",                  32),
	CTypeVersion("sphrase",                 4),
	CTypeVersion("skill_tree",              5),
	CTypeVersion("titles",					1),
	CTypeVersion("succes_chances_table",    1),
	CTypeVersion("automaton_list",			23),
	CTypeVersion("animset_list",			25),
	CTypeVersion("animation_fx",			4),
	CTypeVersion("id_to_string_array",		1),
	CTypeVersion("emot",					1),
	CTypeVersion("forage_source",			2),
	CTypeVersion("flora",					0),
	CTypeVersion("animation_fx_set",		3),
	CTypeVersion("attack_list",		        9),
	CTypeVersion("text_emotes",				1),
	CTypeVersion("sky",						5),
	CTypeVersion("outpost",					0),
	CTypeVersion("outpost_building",		1),
	CTypeVersion("outpost_squad",			1),
	CTypeVersion("faction",					0),
};
// This is just a value for CVS conflict generation.
const	char	*LastPackedFilenameVersionIncrementer= "nico";



/////////////
// MEMBERS //
/////////////
static uint Version = 2;


////////////////////////
// CSheetManagerEntry //
////////////////////////

// ***************************************************************************
CSheetManagerEntry::CSheetManagerEntry () :	EntitySheet(NULL)
{
}

//-----------------------------------------------
// ~CSheetManagerEntry :
//-----------------------------------------------
CSheetManagerEntry::~CSheetManagerEntry ()
{
	// Release the sheet.
	if(EntitySheet != 0)
	{
		delete EntitySheet;
		EntitySheet = 0;
	}
}// ~CSheetManagerEntry //


// ***************************************************************************
void CSheetManagerEntry::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	// Load the form with given sheet id
	if (form)
	{
//		if (EntitySheet != NULL)
//			delete EntitySheet;

		CEntitySheet *sheet = NULL;

		std::string extension = NLMISC::CSheetId::fileExtensionFromType(sheetId.getSheetType());

		// create the new structure
		if (extension == "creature")
			sheet = new CCharacterSheet;
		else if(extension == "player")
			sheet = new CPlayerSheet;
		else if(extension == "fx")
			sheet = new CFXSheet;
		else if(extension == "building")
			sheet = new CBuildingSheet;
		else if(extension == "sitem" || extension == "item" )
			sheet = new CItemSheet;
		else if(extension == "plant")
			sheet = new CPlantSheet;
		else if(extension == "death_impact")
			sheet = new CPactSheet;
		else if(extension == "mission")
			sheet = new CMissionSheet;
		else if(extension == "race_stats")
			sheet = new CRaceStatsSheet;
		else if(extension == "light_cycle")
			sheet = new CLightCycleSheet;
		else if(extension == "weather_setup")
			sheet = new CWeatherSetupSheet;
		else if(extension == "continent")
			sheet = new CContinentSheet;
		else if(extension == "world")
			sheet = new CWorldSheet;
		else if(extension == "weather_function_params")
			sheet = new CWeatherFunctionParamsSheet;
		else if(extension == "mission_icon")
			sheet = new CMissionIconSheet;
		else if(extension == "sbrick")
			sheet = new CSBrickSheet;
		else if(extension == "sphrase")
			sheet = new CSPhraseSheet;
		else if(extension == "skill_tree")
			sheet = new CSkillsTreeSheet;
		else if(extension == "titles")
			sheet = new CUnblockTitlesSheet;
		else if(extension == "succes_chances_table")
			sheet = new CSuccessTableSheet;
		else if(extension == "automaton_list")
			sheet = new CAutomatonListSheet;
		else if(extension == "animset_list")
			sheet = new CAnimationSetListSheet;
		else if(extension == "animation_fx")
			sheet = new CAnimationFXSheet;
		else if(extension == "id_to_string_array")
			sheet = new CIDToStringArraySheet;
		else if(extension == "emot")
			sheet = new CEmotListSheet;
		else if(extension == "forage_source")
			sheet = new CForageSourceSheet;
		else if(extension == "flora")
			sheet = new CFloraSheet;
		else if(extension == "animation_fx_set")
			sheet = new CAnimationFXSetSheet;
		else if(extension == "attack_list")
			sheet = new CAttackListSheet;
		else if(extension == "text_emotes")
			sheet = new CTextEmotListSheet;
		else if(extension == "sky")
			sheet = new CSkySheet;
		else if(extension == "outpost")
			sheet = new COutpostSheet;
		else if(extension == "outpost_building")
			sheet = new COutpostBuildingSheet;
		else if(extension == "outpost_squad")
			sheet = new COutpostSquadSheet;
		else if(extension == "faction")
			sheet = new CFactionSheet;
		else
		{
			nlwarning("CSheetManager::loadSheet: Do not know how to create the class from the sheet '%s'.", sheetId.toString().c_str());
			return;
		}

		// Build the sheet from an external file.
		sheet->Id = sheetId;
		sheet->build(form->getRootNode());

		EntitySheet = sheet;
		SheetMngr.processSheet(EntitySheet);
	}
	// Error while loading the form.
	else
	{
//		nlwarning("CSheetManager::loadSheet: Cannot load the form '%s'.", filename.c_str());
		EntitySheet = 0;
	}
}

// ***************************************************************************
void CSheetManagerEntry::initSheet(CEntitySheet *pES, NLMISC::IStream &s, CEntitySheet::TType type)
{
	if (pES != NULL)
	{
		pES->Id.serial(s);
		pES->serial(s);
		pES->Type = type;
		SheetMngr.processSheet(pES);
	}
}


// ***************************************************************************
void CSheetManagerEntry::serial (NLMISC::IStream &s)
{
	if (s.isReading())
	{
//		if (EntitySheet != NULL)
//			delete EntitySheet;

		CEntitySheet::TType type = CEntitySheet::TypeCount;
		s.serialEnum(type);

		switch(type)
		{
			case CEntitySheet::FAUNA:
			{
				EntitySheet = new CCharacterSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::FLORA:
			{
				EntitySheet = new CFloraSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::CHAR:
			{
				EntitySheet = new CPlayerSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::FX:
			{
				EntitySheet = new CFXSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::BUILDING:
			{
				EntitySheet = new CBuildingSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::ITEM:
			{
				EntitySheet = new CItemSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::PLANT:
			{
				EntitySheet = new CPlantSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::PACT:
			{
				EntitySheet = new CPactSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::MISSION:
			{
				EntitySheet = new CMissionSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::MISSION_ICON:
			{
				EntitySheet = new CMissionIconSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::RACE_STATS:
			{
				EntitySheet = new CRaceStatsSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::LIGHT_CYCLE:
			{
				EntitySheet = new CLightCycleSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::WEATHER_SETUP:
			{
				EntitySheet = new CWeatherSetupSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::CONTINENT:
			{
				EntitySheet = new CContinentSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::WORLD:
			{
				EntitySheet = new CWorldSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::WEATHER_FUNCTION_PARAMS:
			{
				EntitySheet = new CWeatherFunctionParamsSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::SBRICK:
			{
				EntitySheet = new CSBrickSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::SPHRASE:
			{
				EntitySheet = new CSPhraseSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::SKILLS_TREE:
			{
				EntitySheet = new CSkillsTreeSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::UNBLOCK_TITLES:
			{
				EntitySheet = new CUnblockTitlesSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::SUCCESS_TABLE:
			{
				EntitySheet = new CSuccessTableSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::AUTOMATON_LIST:
			{
				EntitySheet = new CAutomatonListSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::ANIMATION_SET_LIST:
			{
				EntitySheet = new CAnimationSetListSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::ANIMATION_FX:
			{
				EntitySheet = new CAnimationFXSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::ID_TO_STRING_ARRAY:
			{
				EntitySheet = new CIDToStringArraySheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::EMOT:
			{
				EntitySheet = new CEmotListSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::FORAGE_SOURCE:
			{
				EntitySheet = new CForageSourceSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::ANIMATION_FX_SET:
			{
				EntitySheet = new CAnimationFXSetSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::ATTACK_LIST:
			{
				EntitySheet = new CAttackListSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::TEXT_EMOT:
			{
				EntitySheet = new CTextEmotListSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::SKY:
			{
				EntitySheet = new CSkySheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::OUTPOST:
			{
				EntitySheet = new COutpostSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::OUTPOST_BUILDING:
			{
				EntitySheet = new COutpostBuildingSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::OUTPOST_SQUAD:
			{
				EntitySheet = new COutpostSquadSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			case CEntitySheet::FACTION:
			{
				EntitySheet = new CFactionSheet;
				initSheet(EntitySheet, s, type);
			}
			break;
			default:
				nlwarning("CSheetManager::load: Unknown type '%d' in the packed file. Rebuild=true and Ignore this sheet.", type);
				EntitySheet = NULL;
			break;
		}
	}
	else
	{
		if (EntitySheet != NULL)
		{
			s.serialEnum(EntitySheet->Type);
			EntitySheet->Id.serial(s);
			EntitySheet->serial(s);
		}
		else
		{
			// write a speudo entry into the stream
			CEntitySheet::TType tmp  = CEntitySheet::UNKNOWN;
			s.serialEnum(tmp);
		}
	}
}

// ***************************************************************************
void CSheetManagerEntry::removed()
{
	// any action that is needed if the sheet no more exist.
	if (EntitySheet != 0)
	{
		delete EntitySheet;
		EntitySheet = 0;
	}
}

// ***************************************************************************
uint CSheetManagerEntry::getVersion ()
{
	return Version;
}

//-----------------------------------------------
//-----------------------------------------------
void CSheetManagerEntry::setVersion(uint version)
{
	Version = version;
}// setVersion //


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CSheetManager :
// Constructor.
//-----------------------------------------------
CSheetManager::CSheetManager()
{
	_NbEyesColor = 0;	// Default is no color available for the eyes.

	// Slot 0 is invalid.
	for(uint i=0; i<SLOTTYPE::NB_SLOT; ++i)
	{
		TItemVector slotList;
		slotList.push_back(0);
		_VisualSlots.push_back(slotList);
	}
}

//-----------------------------------------------
// CSheetManager :
// Destructor.
//-----------------------------------------------
CSheetManager::~CSheetManager()
{
	release();
}

//-----------------------------------------------
void CSheetManager::release()
{
	_VisualSlots.clear();

	_EntitySheetContainer.clear();

	_SheetToVS.clear();
}

//-----------------------------------------------
// load :
// Load all sheets.
//-----------------------------------------------
void CSheetManager::load(NLMISC::IProgressCallback &callBack, bool updatePackedSheet, bool needComputeVS, bool dumpVSIndex)
{
	// Open The Item Association File
//	if(!fItemAssoc.open(getLogDirectory() + "item_association.dbg", false, true))
//		nlwarning("CSheetManager::load: Cannot Open the 'item_association.txt'.");
//	else
//		ItemAssocFileOpen = true;

	// Initialize the Sheet DB.
	loadAllSheet(callBack, updatePackedSheet, needComputeVS, dumpVSIndex);

	// Close the Item Association File.
//	fItemAssoc.close();
//	ItemAssocFileOpen = false;

	// Optimize memory taken by all strings of all sheets
	ClientSheetsStrings.memoryCompress();

	return;
}// load //


//-----------------------------------------------
// loadAllSheet :
// Load all sheets.
//-----------------------------------------------
void CSheetManager::loadAllSheet(NLMISC::IProgressCallback &callBack, bool updatePackedSheet, bool needComputeVS, bool dumpVSIndex, bool forceRecompute /*= false*/, const std::vector<std::string> *userExtensions /*= NULL*/)
{

	callBack.progress (0);
	callBack.pushCropedValues (0, 0.5f);

	// Get some information from typ files.
	loadTyp();

	// prepare a list of sheets extension to load.
	vector<string>	extensions;

	uint sizeTypeVersion = sizeof(TypeVersion);
	uint sizeCTypeVersion = sizeof(CTypeVersion);
	uint nb = sizeTypeVersion/sizeCTypeVersion;
	{
		if (!userExtensions)
		{
			_EntitySheetContainer.clear();
		}
		TEntitySheetMap entitySheetContainer;
		for(uint i=0; i<nb; ++i)
		{
			// see if extension is wanted
			bool found = false;
			if (userExtensions)
			{
				for(uint l = 0; l < userExtensions->size(); ++l)
				{
					if (stricmp((*userExtensions)[l].c_str(), TypeVersion[i].Type.c_str()) == 0)
					{
						found = true;
					}
				}
			}
			else
			{
				 found = true;
			}
			if (found)
			{
				entitySheetContainer.clear();
				extensions.clear();
				extensions.push_back(TypeVersion[i].Type);
				CSheetManagerEntry::setVersion(TypeVersion[i].Version);
				string path = CPath::lookup(TypeVersion[i].Type + ".packed_sheets", false);
				if (forceRecompute && !path.empty())
				{
					// delete previous packed sheets
					NLMISC::CFile::deleteFile(path);
					path.clear();
				}
				if(path.empty())
					path = CPath::standardizePath(_OutputDataPath) + TypeVersion[i].Type + ".packed_sheets";
				::loadForm(extensions, path, entitySheetContainer, updatePackedSheet);

				TEntitySheetMap::iterator it = entitySheetContainer.begin();
				while(it  != entitySheetContainer.end())
				{
					_EntitySheetContainer[(*it).first] = (*it).second;
					(*it).second.EntitySheet = 0;
					// Next
					++it;
				}
			}
		}
	}


	// Re-compute Visual Slot
	if(needComputeVS)
		computeVS();

	// Compute Visual Slots
	{
		for(uint i=0; i<SLOTTYPE::NB_SLOT; ++i)
			_VisualSlots[i].resize(CVisualSlotManager::getInstance()->getNbIndex((SLOTTYPE::EVisualSlot)i)+1, 0);	// Nb Index +1 because index 0 is reserve for empty.

		//
		TEntitySheetMap::iterator it = _EntitySheetContainer.begin();
		while(it != _EntitySheetContainer.end())
		{
			std::vector<CVisualSlotManager::TIdxbyVS> result;
			CVisualSlotManager::getInstance()->sheet2Index((*it).first, result);

			for(uint i=0; i<result.size(); ++i)
			{
				if(dynamic_cast<CItemSheet *>((*it).second.EntitySheet))
				{
					_SheetToVS[dynamic_cast<CItemSheet *>((*it).second.EntitySheet)].push_back(std::make_pair(result[i].VisualSlot, result[i].Index));
					_VisualSlots[result[i].VisualSlot][result[i].Index] = dynamic_cast<CItemSheet *>((*it).second.EntitySheet);
				}
			}

			++it;
		}
	}

	// Dump visual slots
	// nb : if a new visual_slot.tab has just been generated don't forget
	// to move it in data_common before dump.
	if(dumpVSIndex)
		dumpVisualSlotsIndex();

	//
	callBack.popCropedValues();
}// loadAllSheet //


// ***************************************************************************
void CSheetManager::loadAllSheetNoPackedSheet(NLMISC::IProgressCallback &callBack, const std::vector<std::string> &extensions, const std::string &wildcardFilter)
{

	callBack.progress (0);
	callBack.pushCropedValues (0, 0.5f);

	//  load all forms
	::loadFormNoPackedSheet(extensions, _EntitySheetContainer, wildcardFilter);

	//
	callBack.popCropedValues();
}


//-----------------------------------------------
// computeVS :
// compute Visual Slots for this sheet.
//-----------------------------------------------
void CSheetManager::computeVS()
{
	static std::map< std::string, uint16 > ProcessedItem;
	map< string, uint16 >::iterator it;

	CVisualSlotManager::TVisualSlot vs;
	vs.resize(SLOTTYPE::NB_SLOT);

	//
	TEntitySheetMap::iterator itS = _EntitySheetContainer.begin();
	while(itS != _EntitySheetContainer.end())
	{
		// Visual Slots are only valid for Items.
		CItemSheet *item = dynamic_cast<CItemSheet *>((*itS).second.EntitySheet);
		if (item && (*itS).first.getSheetType()==CSheetId::typeFromFileExtension(std::string("sitem")))
		{
			for(uint j=0; j<SLOTTYPE::NB_SLOT_TYPE; ++j)
			{
				SLOTTYPE::TSlotType	slotType= (SLOTTYPE::TSlotType)j;
				if( item->hasSlot(slotType) )
				{
					SLOTTYPE::EVisualSlot visualSlot = SLOTTYPE::convertTypeToVisualSlot(slotType);
					if(visualSlot != SLOTTYPE::HIDDEN_SLOT)
					{
						string currentSheet = ((*itS).first).toString();

						CVisualSlotManager::TElement vsElmt;

						string sheetName = toString("%s%d", currentSheet.c_str(), visualSlot);

						// Is the sheet already process (could be process by a sheet with a different lvl)
						it = ProcessedItem.find( sheetName );
						// Insert if not found
						if(it == ProcessedItem.end())
						{
							uint itemNumber;
							if(vs[visualSlot].Element.size() == 0)
								itemNumber = 1;
							else
								itemNumber = vs[visualSlot].Element[vs[visualSlot].Element.size()-1].Index+1;

							// Item Processed
							ProcessedItem.insert(make_pair(sheetName, itemNumber));

							vsElmt.Index = itemNumber;
						}
						else
						{
							vsElmt.Index = (*it).second;
						}
						//
						vsElmt.SheetId = (*itS).first;
						vs[visualSlot].Element.push_back(vsElmt);
					}
				}
			}
		}

		// Next Sheet
		++itS;
	}


	// Open the file.
	NLMISC::COFile f;
	if(f.open("visual_slot.tab"))
	{
		// Dump entities.
		f.serialCont(vs);

		// Close the File.
		f.close();
	}
	else
		nlwarning("SheetMngr:load: cannot open/create the file 'visual_slot.tab'.");
}// computeVS //

//-----------------------------------------------
// processSheet :
// Porcessing the sheet.
// \param sheet : sheet to process.
//-----------------------------------------------
void CSheetManager::processSheet (CEntitySheet * /* sheet */)
{
	// For now: no op
}// processSheet //

//=======================================================================
const CSheetManager::TVisualSlotItemArray *CSheetManager::getVSItems(CItemSheet *sheet) const
{
	TItemSheet2SlotItemArray::const_iterator it = _SheetToVS.find(sheet);
	if (it == _SheetToVS.end())
		return NULL;
	return &(it->second);
}

//=======================================================================
sint CSheetManager::getVSIndex(const std::string &itemName, SLOTTYPE::EVisualSlot slot)
{
	NLMISC::CSheetId si;
	if (!si.buildSheetId(itemName))
	{
		nlwarning("<CSheetManager::getVSIndex> : cannot build id from item %s for the slot %d.", itemName.c_str(), slot);
		return -1;
	}

	TEntitySheetMap::iterator it =  _EntitySheetContainer.find(si);;
	if (it == _EntitySheetContainer.end())
	{
		nlwarning("<CSheetManager::getVSIndex> : cannot find %s for the slot %d.", itemName.c_str(), slot);
		return -1;
	}
	if (it->second.EntitySheet == 0 || it->second.EntitySheet->type() != CEntitySheet::ITEM)
	{
		nlwarning("<CSheetManager::getVSIndex> : %s is not an item for the slot %d.", itemName.c_str(), slot);
		return -1;
	}

	CItemSheet *is = static_cast<CItemSheet *>(it->second.EntitySheet);

	const TVisualSlotItemArray *ia = getVSItems(is);
	if (ia == NULL)
	{
		nlwarning("<CSheetManager::getVSIndex> : no items for the slot %d. while looking for %s", slot, itemName.c_str());
		return -1;
	}

	TVisualSlotItemArray::const_iterator first(ia->begin()), last(ia->end());
	for(; first != last; ++first)
	{
		if (first->first == slot)
		{
			return first->second;
		}
	}

	nlwarning("<CSheetManager::getVSIndex> : cannot find %s for the slot %d.", itemName.c_str(), slot);
	return -1;
}

//-----------------------------------------------
// get :
// Get a sheet from its number.
// \param uint32 num : sheet number.
// \return CEntitySheet * : pointer on the sheet according to the param or 0 if any pb.
//-----------------------------------------------
CEntitySheet *CSheetManager::get(CSheetId num)
{
	TEntitySheetMap::iterator it =  _EntitySheetContainer.find(num);
	if(it != _EntitySheetContainer.end())
		return it->second.EntitySheet;
	else
		return NULL;
}// get //


//-----------------------------------------------
uint CSheetManager::getNumItem(SLOTTYPE::EVisualSlot slot)
{
	// The slot is not a visible one.
	if (slot == SLOTTYPE::HIDDEN_SLOT)
		 return 0;
	// Convert into an uint to remove warnings.
	uint s = (uint)slot;

	// Check slot.
	if(s < _VisualSlots.size())
	{
		return (uint)_VisualSlots[s].size();
	}
	else
	{
		nlwarning("CSheetManager::getNumItem : invalid slot %d.", slot);
		return 0;
	}
}

//-----------------------------------------------
// getItem :
// Get the real.
//-----------------------------------------------
CItemSheet *CSheetManager::getItem(SLOTTYPE::EVisualSlot slot, uint index)
{
	// The slot is not a visible one.
	if(slot == SLOTTYPE::HIDDEN_SLOT)
		return 0;

	// Convert into an uint to remove warnings.
	uint s = (uint)slot;

	// Check slot.
	if(s < _VisualSlots.size())
	{
		// Check index.
		if(index < _VisualSlots[s].size())
		{
			// Not the default Item.
			if(index != 0)
				return _VisualSlots[s][index];
			// Default Item.
			else
				return NULL;
		}
		// Bad index.
		else
		{
			//nlwarning("CSheetManager::getItem : invalid index %d for the slot %d.", index, slot);
			return NULL;
		}
	}
	// Bad slot.
	else
	{
		nlwarning("CSheetManager::getItem : invalid slot %d.", slot);
		return NULL;
	}
}// getItem //


//-----------------------------------------------
// loadTyp :
// Get Some information from 'typ' files.
//-----------------------------------------------
void CSheetManager::loadTyp()
{
	// Read the Eyes Color 'typ'
	NLMISC::CSmartPtr<NLGEORGES::UType> smartPtr = FormLoader->loadFormType("_creature_3d_eyes_color.typ");
	if(smartPtr)
	{
		string maxStr = smartPtr->getMax();
		fromString(maxStr, _NbEyesColor);
		if(_NbEyesColor <= 0)
			nlwarning("CSheetManager::loadTyp: There no is Color available for the eyes.");
	}
	else
		nlwarning("CSheetManager::loadTyp: Cannot load the '_creature_3d_eyes_color.typ' file.");

	// Read the Hair Color 'typ'
	smartPtr = FormLoader->loadFormType("_creature_3d_hair_color.typ");
	if(smartPtr)
	{
		string maxStr = smartPtr->getMax();
		fromString(maxStr, _NbHairColor);
		if(_NbHairColor <= 0)
			nlwarning("CSheetManager::loadTyp: There is no Color available for the hair.");
	}
	else
		nlwarning("CSheetManager::loadTyp: Cannot load the '_creature_3d_hair_color.typ' file.");
}// initTyp //


// ***************************************************************************
void CSheetManager::dumpVisualSlots()
{
	for(uint k = 0; k < _VisualSlots.size(); ++k)
	{
		TItemVector &iv = _VisualSlots[k];
		for(uint l = 0; l < iv.size(); ++l)
		{
			if (iv[l])
			{
				nlinfo("Slot %d, item %d = %s", (int) k, (int) l, iv[l]->Id.toString().c_str());
			}
		}
	}
}


// ***************************************************************************
void CSheetManager::dumpVisualSlotsIndex()
{
	FILE * vsIndexFile = fopen(std::string(getLogDirectory() + "vs_index.txt").c_str(),"w");
	if( vsIndexFile )
	{
		for (uint i=0; i < SLOTTYPE::NB_SLOT; ++i)
		{
			fprintf(vsIndexFile,"VISUAL SLOT : %d\n", i);
			TItemVector &rVTmp = _VisualSlots[i];
			for (uint j = 0; j < rVTmp.size(); ++j)
			{
				CItemSheet *pIS = rVTmp[j];
				if (pIS != NULL)
				{
					fprintf(vsIndexFile,"%d : %s\n", j, pIS->Id.toString().c_str());
				}
				//nlSleep(100);
			}
		}
	}
	else
	{
		nlwarning("<CSheetManager::loadAllSheet> Can't open file to dump VS index");
	}
	fclose(vsIndexFile);
}
