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
// Georges
#include "nel/georges/u_form_elm.h"
// Client.
#include "player_sheet.h"
// Game share
#include "game_share/skills.h"
#include "game_share/skills_build.h"


///////////
// USING //
///////////
using namespace NLGEORGES;
using namespace std;


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CPlayerSheet :
// Constructor.
//-----------------------------------------------
CPlayerSheet::CPlayerSheet()
{
	CharacterScalePos= 1;
	Gender=0;
	// Initialize the type.
	Type	= CEntitySheet::CHAR;
}// CPlayerSheet //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CPlayerSheet::build(const NLGEORGES::UFormElm &item)
{
	// Initialize the scale.
	Scale	= 1.f;

	// Player Gender.
	if(!item.getValueByName(Gender, "Gender"))
		debug("Key 'Gender' not found.");

	// Get the skel.
	if(!item.getValueByName(SkelFilename, "Skel"))
		debug("Key 'Skel' not found.");

	// Load the Animation Set Base Name.
	if(!item.getValueByName(AnimSetBaseName, "AnimSetBaseName"))
		debug("Key 'AnimSetBaseName' not found.");
	// Force the CASE in UPPER to not be CASE SENSITIVE.
	else
		NLMISC::strlwr(AnimSetBaseName);

	// Load Lod character name
	if(!item.getValueByName(LodCharacterName, "LodCharacterName"))
		debug("Key 'LodCharacterName' not found.");

	// Load Lod character apparition distance
	if(!item.getValueByName(LodCharacterDistance, "LodCharacterDistance"))
		debug("Key 'LodCharacterDistance' not found.");

	// Get the people of the player.
	string people;
	if(!item.getValueByName(people, "Peuple"))
	{
		debug("Key 'Peuple' not found.");
		People = EGSPD::CPeople::EndPeople;
	}
	else
		People = EGSPD::CPeople::fromString(people);

	// Get the Default Player Face filename.
	if(!item.getValueByName(DefaultFace, "DefaultFace"))
		debug("Key 'DefaultFace' not found.");
	// Get the Default Player Chest filename.
	if(!item.getValueByName(DefaultChest, "DefaultChest"))
		debug("Key 'DefaultChest' not found.");
	// Get the Default Player Legs filename.
	if(!item.getValueByName(DefaultLegs, "DefaultLegs"))
		debug("Key 'DefaultLegs' not found.");
	// Get the Default Player Arms filename.
	if(!item.getValueByName(DefaultArms, "DefaultArms"))
		debug("Key 'DefaultArms' not found.");
	// Get the Default Player Hands filename.
	if(!item.getValueByName(DefaultHands, "DefaultHands"))
		debug("Key 'DefaultHands' not found.");
	// Get the Default Player Feet filename.
	if(!item.getValueByName(DefaultFeet, "DefaultFeet"))
		debug("Key 'DefaultFeet' not found.");
	// Get the Default Player Hair filename.
	if(!item.getValueByName(DefaultHair, "DefaultHair"))
		debug("Key 'DefaultHair' not found.");

	// get carac
	if(!item.getValueByName(DefaultHair, "DefaultHair"))
		debug("Key 'DefaultHair' not found.");


	// NB: lod colors are given with panoply colors for players


	// value to scale the "pos" channel of the animation of the player.
	if(!item.getValueByName(CharacterScalePos, "CharacterScalePos"))
		debug("Key 'CharacterScalePos' not found.");

	#define PLAYER_SHEET_EQUIP "Basics.Equipment."
	Body.build(PLAYER_SHEET_EQUIP "Body", item);
	Legs.build(PLAYER_SHEET_EQUIP "Legs", item);
	Arms.build(PLAYER_SHEET_EQUIP "Arms", item);
	Hands.build(PLAYER_SHEET_EQUIP "Hands", item);
	Feet.build(PLAYER_SHEET_EQUIP "Feet", item);
	Head.build(PLAYER_SHEET_EQUIP "Head", item);
	Face.build(PLAYER_SHEET_EQUIP "Face", item);
	ObjectInRightHand.build(PLAYER_SHEET_EQUIP "ObjectInRightHand", item);
	ObjectInLeftHand.build(PLAYER_SHEET_EQUIP "ObjectInLeftHand", item);
	Headdress.build(PLAYER_SHEET_EQUIP "Headdress", item);
	EarL.build(PLAYER_SHEET_EQUIP "EarL", item);
	EarR.build(PLAYER_SHEET_EQUIP "EarR", item);
	Neck.build(PLAYER_SHEET_EQUIP "Neck", item);
	Shoulders.build(PLAYER_SHEET_EQUIP "Shoulders", item);
	Back.build(PLAYER_SHEET_EQUIP "Back", item);
	WristL.build(PLAYER_SHEET_EQUIP "WristL", item);
	WristR.build(PLAYER_SHEET_EQUIP "WristR", item);
	FingerL.build(PLAYER_SHEET_EQUIP "FingerL", item);
	FingerR.build(PLAYER_SHEET_EQUIP "FingerR", item);
	AnkleR.build(PLAYER_SHEET_EQUIP "AnkleR", item);
	AnkleL.build(PLAYER_SHEET_EQUIP "AnkleL", item);
	Ammo.build(PLAYER_SHEET_EQUIP "Ammo", item);

	// build charac
	buildCharac(item);

	// load skills
	loadSkillsSummaryFromSheet(item, "Basics.", Skills);
}// build //


//-----------------------------------------------
void CPlayerSheet::buildCharac(const NLGEORGES::UFormElm &item)
{
	static const char characsBasePath[] = "Basics.Characteristics.";
	for(uint k = 0; k < CHARACTERISTICS::NUM_CHARACTERISTICS; ++k)
	{
		const std::string &characName = CHARACTERISTICS::toString((CHARACTERISTICS::TCharacteristics)k);
		std::string characPath = characsBasePath + characName;
		if(!item.getValueByName(Characteristics[k], characPath.c_str()))
		{
			debug("Key " + characName + "not found.");
			Characteristics[k] = 0;
		}
	}
}


//-----------------------------------------------
// serial :
// Serialize player sheet into binary data file.
//-----------------------------------------------
void CPlayerSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serialize class components.
	f.serial(SkelFilename);
	f.serial(AnimSetBaseName);
	f.serial(Scale);
	f.serial(LodCharacterName);
	f.serial(LodCharacterDistance);
	f.serial(CharacterScalePos);
	f.serialEnum(People);
	f.serial(DefaultFace);
	f.serial(DefaultChest);
	f.serial(DefaultLegs);
	f.serial(DefaultArms);
	f.serial(DefaultHands);
	f.serial(DefaultFeet);
	f.serial(DefaultHair);
	f.serial(Gender);
	// equipment
	f.serial(Body);
	f.serial(Legs);
	f.serial(Arms);
	f.serial(Hands);
	f.serial(Feet);
	f.serial(Head);
	f.serial(Face);
	f.serial(ObjectInRightHand);
	f.serial(ObjectInLeftHand);
	f.serial(Headdress);
	f.serial(EarL);
	f.serial(EarR);
	f.serial(Neck);
	f.serial(Shoulders);
	f.serial(Back);
	f.serial(WristL);
	f.serial(WristR);
	f.serial(FingerL);
	f.serial(FingerR);
	f.serial(AnkleL);
	f.serial(AnkleR);
	f.serial(Ammo);

	for(uint k = 0; k < CHARACTERISTICS::NUM_CHARACTERISTICS; ++k)
	{
		f.serial(Characteristics[k]);
	}

	f.serialCont(Skills);
}// serial //


//=========================================================================================
void CPlayerSheet::CEquipment::build(const std::string &key,const NLGEORGES::UFormElm &item)
{
	// Get the item (or shape) name.
	string itemName;
	if(!item.getValueByName(itemName, string(key + ".Item").c_str() ))
		debug(NLMISC::toString("Key '%s.Item' not found.", key.c_str()));
	else
		Item = NLMISC::strlwr(itemName);

	// Get the color.
	if(!item.getValueByName(Color, string(key + ".Color").c_str() ))
		debug(NLMISC::toString("Key '%s.Color' not found.", key.c_str()));
}
