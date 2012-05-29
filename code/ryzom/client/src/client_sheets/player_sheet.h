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




#ifndef CL_PLAYER_SHEET_H
#define CL_PLAYER_SHEET_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
// Application
#include "entity_sheet.h"
// Game share
#include "game_share/people.h"
#include "game_share/characteristics.h"
#include "game_share/slot_types.h"
#include "game_share/skills_build.h"
// std
#include <vector>
#include <string>


///////////
// CLASS //
///////////
//struct CSkillSummary;

namespace NLGEORGES
{
	class UFormElm;
	class UFormLoader;
}

/**
 * Class to manage the player sheet.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2001
 */
class CPlayerSheet : public CEntitySheet
{
public:
	struct CEquipment
	{
		std::string Item; // name of the '.item'
		sint8       Color;

		/// Build the sheet from an external script.
		virtual void build(const std::string &key, const NLGEORGES::UFormElm &item);
		/// Serialize
		virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(Item, Color);
		}
	};

public:
	std::string					SkelFilename;
	std::string					AnimSetBaseName;
	float						Scale;
	// Lod Character.
	std::string					LodCharacterName;
	float						LodCharacterDistance;
	// value to scale the "pos" channel of the animation of the player.
	float						CharacterScalePos;
	// People of the player (FYROS, MATIS, ...)
	EGSPD::CPeople::TPeople			People;
	// Default player look.
	std::string					DefaultFace;
	std::string					DefaultChest;
	std::string					DefaultLegs;
	std::string					DefaultArms;
	std::string					DefaultHands;
	std::string					DefaultFeet;
	std::string					DefaultHair;
	// Player Gender.
	uint8						Gender;

	// Equipment for player
	CEquipment					Body;
	CEquipment					Legs;
	CEquipment					Arms;
	CEquipment					Hands;
	CEquipment					Feet;
	CEquipment					Head;
	CEquipment					Face;
	CEquipment					ObjectInRightHand;
	CEquipment					ObjectInLeftHand;
	CEquipment					Headdress;
	CEquipment					EarL;
	CEquipment					EarR;
	CEquipment					Neck;
	CEquipment					Shoulders;
	CEquipment					Back;
	CEquipment					WristL;
	CEquipment					WristR;
	CEquipment					FingerL;
	CEquipment					FingerR;
	CEquipment					AnkleL;
	CEquipment					AnkleR;
	CEquipment					Ammo;

	// caracs
	uint16						Characteristics[CHARACTERISTICS::NUM_CHARACTERISTICS];
	// skills
	std::vector<CSkillSummary>	Skills;


public:
	/// Constructor
	CPlayerSheet();

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);


private:
	void buildCharac(const NLGEORGES::UFormElm &item);
};

#endif // CL_PLAYER_SHEET_H

/* End of player_sheet.h */
