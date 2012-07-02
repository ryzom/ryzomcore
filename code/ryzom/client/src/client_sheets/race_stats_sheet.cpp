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
#include "race_stats_sheet.h"
// Game Share
#include "game_share/characs_build.h"
// Georges
#include "nel/georges/u_form_elm.h"


///////////
// USING //
///////////
using namespace std;


/////////////
// METHODS //
/////////////
const string CGenderInfo::UnknownItemName = "";

// description of a visual slot that must be read by CGenderInfo
struct	CSlotInfo
{
	SLOTTYPE::EVisualSlot  Slot;
	const char			  *Suffix; // suffix in the sheet
};

static const CSlotInfo UsedVisualSlots[] =
{
	{ SLOTTYPE::CHEST_SLOT, "DefaultChest" },
	{ SLOTTYPE::LEGS_SLOT,  "DefaultLegs"  },
	{ SLOTTYPE::HEAD_SLOT,  "DefaultHair"  },
	{ SLOTTYPE::ARMS_SLOT,  "DefaultArms"  },
	{ SLOTTYPE::FACE_SLOT,  "DefaultFace"  },
	{ SLOTTYPE::HANDS_SLOT, "DefaultHands" },
	{ SLOTTYPE::FEET_SLOT,  "DefaultFeet"  }
};
static const uint NumUsedVisualSlots = sizeof(UsedVisualSlots) / sizeof(UsedVisualSlots[0]);


/////////////////
// CGenderInfo //
/////////////////
//===============================================================
CGenderInfo::CGenderInfo()
{
	CharacterScalePos = 1;
	for (uint32 i = 0; i < 8; ++i)
	{
		BlendShapeMin[i] = 0.0f;
		BlendShapeMax[i] = 100.0f;
	}
	NamePosZLow = 0.f;
	NamePosZNormal = 0.f;
	NamePosZHigh = 0.f;
}

//===============================================================
void CGenderInfo::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	// Get equipment items
	// get slots
	for(uint k = 0; k < NumUsedVisualSlots; ++k)
	{
		if (!item.getValueByName(Items[UsedVisualSlots[k].Slot], (prefix + UsedVisualSlots[k].Suffix).c_str()))
		{
			nlwarning("Can't get %s from race_stats sheet", UsedVisualSlots[k].Suffix);
		}
	}
	// Get skeleton
	if (!item.getValueByName(Skelfilename, (prefix + "Skel").c_str()))
	{
		nlwarning("Can't get skeleton");
	}
	// get base animation name
	if (!item.getValueByName(AnimSetBaseName, (prefix + "AnimSetBaseName").c_str()))
	{
		nlwarning("Can't get AnimSetBaseName");
	}


	// Load Lod character name
	if(!item.getValueByName(LodCharacterName, (prefix + "LodCharacterName").c_str()))
		nlwarning("CGenderInfo::build: Key 'LodCharacterName' not found.");

	// Load Lod character apparition distance
	if(!item.getValueByName(LodCharacterDistance, (prefix + "LodCharacterDistance").c_str()))
		nlwarning("CGenderInfo::build: Key 'LodCharacterDistance' not found.");


	// value to scale the "pos" channel of the animation of the player.
	if(!item.getValueByName(CharacterScalePos, (prefix + "CharacterScalePos").c_str()))
		nlwarning("CGenderInfo::build: Key 'CharacterScalePos' not found.");
	for (uint32 i = 0; i < 8; ++i)
	{
		if(!item.getValueByName(BlendShapeMin[i], (prefix+"MorphTargetMin"+NLMISC::toString(i)).c_str()))
			nlwarning("CGenderInfo::build: Key 'MorphTargetMin%d' not found.",i);
		if(!item.getValueByName(BlendShapeMax[i], (prefix+"MorphTargetMax"+NLMISC::toString(i)).c_str()))
			nlwarning("CGenderInfo::build: Key 'MorphTargetMax%d' not found.",i);
	}

	if(!item.getValueByName(NamePosZLow, (prefix + "NamePosZLow").c_str()))
		nlwarning("CGenderInfo::build: Key 'NamePosZLow' not found.");
	if(!item.getValueByName(NamePosZNormal, (prefix + "NamePosZNormal").c_str()))
		nlwarning("CGenderInfo::build: Key 'NamePosZNormal' not found.");
	if(!item.getValueByName(NamePosZHigh, (prefix + "NamePosZHigh").c_str()))
		nlwarning("CGenderInfo::build: Key 'NamePosZHigh' not found.");
}

//===============================================================
void CGenderInfo::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// serial used slots
	for(uint k = 0; k < NumUsedVisualSlots; ++k)
	{
		f.serial(Items[UsedVisualSlots[k].Slot]);
	}
	f.serial(Skelfilename);
	f.serial(AnimSetBaseName);
	f.serial(LodCharacterName);
	f.serial(LodCharacterDistance);
	f.serial(CharacterScalePos);
	f.serialCont(GroundFX);
	for (uint32 i = 0; i < 8; ++i)
	{
		f.serial(BlendShapeMin[i]);
		f.serial(BlendShapeMax[i]);
	}
	f.serial(NamePosZLow);
	f.serial(NamePosZNormal);
	f.serial(NamePosZHigh);
}


//-----------------------------------------------
// getItemName :
// Get the item name for a given slot.
//-----------------------------------------------
const string &CGenderInfo::getItemName(SLOTTYPE::EVisualSlot slot) const
{
	// Check the slot.
	if(slot >= SLOTTYPE::NB_SLOT)
	{
		// Bad Slot.
		nlwarning("CGenderInfo::getItemName: slot '%d' invalid.", slot);
		return UnknownItemName;
	}

	// Return the name of the default item for this slot.
	return Items[slot];
}// getItemName //





/////////////////////
// CRaceStatsSheet //
/////////////////////
//===============================================================
CRaceStatsSheet::CRaceStatsSheet()
{
	std::fill(CharacStartValue, CharacStartValue + CHARACTERISTICS::NUM_CHARACTERISTICS, 0);
	// Initialize the type.
	Type = CEntitySheet::RACE_STATS;
}

//===============================================================
void CRaceStatsSheet::build(const NLGEORGES::UFormElm &item)
{
	GenderInfos[0].build(item, "DefaultEquipment.Male equipment.");
	GenderInfos[1].build(item, "DefaultEquipment.Female equipment.");
	// build base characs
	loadCharacteristicsFromSheet(item, "Characteristics.", CharacStartValue);
	// Get race
	std::string race;
	item.getValueByName(race, "Race");
	People = EGSPD::CPeople::fromString(race);

	// Get the skin to use.
	if(!item.getValueByName(Skin, "Skin"))
		nlwarning("CRaceStatsSheet::build: Cannot find the key 'Skin'.");
	//
	buildGroundFXs(item, "GroundFXMale", GenderInfos[0].GroundFX);
	buildGroundFXs(item, "GroundFXFemale", GenderInfos[1].GroundFX);
	// get Automaton Type
	if(!item.getValueByName(Automaton, "Automaton"))
		nlwarning("CRaceStatsSheet::build: Can't get 'Automaton'.");
	BodyToBone.build(item, "Localisation.");
	// Attack lists
	for(uint k = 0; k < NumAttackLists; ++k)
	{
		std::string attackListName;
		if(item.getValueByName(attackListName, NLMISC::toString("attack_list%d", (int) k).c_str()) && !attackListName.empty())
		{
			AttackLists.push_back(ClientSheetsStrings.add(attackListName));
		}
	}

}

//===============================================================
void CRaceStatsSheet::buildGroundFXs(const NLGEORGES::UFormElm &item, const std::string &name, std::vector<CGroundFXSheet> &dest)
{
	// ground fxs
	const NLGEORGES::UFormElm *elm;
	if(item.getNodeByName(&elm, name.c_str()) && elm)
	{
		// Check array.
		if(elm->isArray())
		{
			// Get Array Size
			uint groundFXArraySize;
			if(elm->getArraySize(groundFXArraySize))
			{
				if(groundFXArraySize > 0)
				{
					// Adjust the array size.
					dest.reserve(groundFXArraySize);

					// Get values.
					for(uint i=0; i< groundFXArraySize; ++i)
					{
						const NLGEORGES::UFormElm *node;
						if (elm->getArrayNode(&node, i) && node != NULL)
						{
							CGroundFXSheet gfs;
							if (!gfs.build(*node))
							{
								nlwarning("Error while building node %d", (int) i);
							}
							else
							{
								uint k;
								for(k = 0; k < dest.size(); ++k)
								{
									if (dest[k].GroundID == gfs.GroundID)
									{
										debug("Duplicated material");
										dest[k] = gfs;
										break;
									}
								}
								if (k == dest.size())
								{
									dest.push_back(gfs);
								}
							}
						}
					}
				}
			}
			else
				debug(NLMISC::toString("'%s' cannot get the array size.", name.c_str()));
		}
		else
			debug(NLMISC::toString("'%s' is not an array.", name.c_str()));
	}
	else
		debug(NLMISC::toString("'%s' key not found.", name.c_str()));
	// sort fxs by ground type
	std::sort(dest.begin(), dest.end());
}

//===============================================================
void CRaceStatsSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	for(uint k = 0; k < CHARACTERISTICS::NUM_CHARACTERISTICS; ++k)
	{
		f.serial(CharacStartValue[k]);
	}
	f.serial(GenderInfos[0], GenderInfos[1]);

	if(f.isReading())
	{
		uint8 people;
		f.serial(people);
		People = (EGSPD::CPeople::TPeople)people;
	}
	else
	{
		uint8 people = (uint8)People;
		f.serial(people);
	}

	f.serial(Skin);	// The skin
	f.serial(Automaton);
	f.serial(BodyToBone);
	// attack list
	uint32 size = (uint32)AttackLists.size();
	f.serial(size);
	AttackLists.resize(size);
	//
	for(uint k = 0; k < size; ++k)
	{
		ClientSheetsStrings.serial(f, AttackLists[k]);
	}
}
