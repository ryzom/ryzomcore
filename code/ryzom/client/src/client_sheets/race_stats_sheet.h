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


#ifndef CL_RACE_STATS_SHEET_H
#define CL_RACE_STATS_SHEET_H

// Application
#include "entity_sheet.h"
#include "ground_fx_sheet.h"
#include "body_to_bone_sheet.h"
// Game Share
#include "game_share/people.h"
#include "game_share/slot_types.h"
#include "game_share/characteristics.h"


//=======================================================================================================
namespace NLGEORGES
{
	class UFormElm;
}

/** Per Gender Infos. Used by RaceStats sheets
  */
struct CGenderInfo
{
	static const std::string UnknownItemName;


	// skel
	std::string Skelfilename;
	// Anim set base name
	std::string	AnimSetBaseName;
	// default equipment, sheath are not included
	std::string Items[SLOTTYPE::NB_SLOT];

	std::string	LodCharacterName;
	float		LodCharacterDistance;
	// value to scale the "pos" channel of the animation of the player.
	float		CharacterScalePos;
	// ground fxs
	std::vector<CGroundFXSheet> GroundFX;

	// Blendshapes minimum and maximum values
	float BlendShapeMin[8], BlendShapeMax[8];

	// Name positions on Z axis
	float NamePosZLow;
	float NamePosZNormal;
	float NamePosZHigh;

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
	// ctor
	CGenderInfo();
	/// Build
	void build(const NLGEORGES::UFormElm &form, const std::string &prefix);
	/// Serialize
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// Get the item name for a given slot.
	const std::string &getItemName(SLOTTYPE::EVisualSlot slot) const;
};


//=======================================================================================================

/** Class to manage .race_stats sheets
  */
class CRaceStatsSheet : public CEntitySheet
{
public:
	enum { NumAttackLists = 8 };
	EGSPD::CPeople::TPeople	People;
	// Start characteristics values for this race
	sint8				CharacStartValue[CHARACTERISTICS::NUM_CHARACTERISTICS];
	// Per gender infos. 0 is for male and 1 is for female
	CGenderInfo			GenderInfos[2];
	/// Skin to use for this race.
	uint8				Skin;
	// Automaton Type
	std::string			Automaton;
	// Body to bone. For meleeImpact localisation
	CBodyToBoneSheet	BodyToBone;
	// attack lists filenames
	std::vector<NLMISC::TSStringId>	AttackLists;
public:
	// ctor
	CRaceStatsSheet();
	//
	virtual void build(const NLGEORGES::UFormElm &item);
	/// Serialize rce_stats sheet into binary data file.
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	void buildGroundFXs(const NLGEORGES::UFormElm &item, const std::string &name, std::vector<CGroundFXSheet> &dest);
};



#endif





















