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
// Application
#include "character_sheet.h"
// Game Share
#include "game_share/georges_helper.h"


///////////
// USING //
///////////
using namespace std;
using namespace NLGEORGES;
using namespace NLMISC;


// ***************************************************************************
// Easy macro to translate value from georges
#define TRANSLATE_VAL( _Var_, _key_ )						\
	if(!item.getValueByName(_Var_, _key_))					\
		debug( string("key '") + string(_key_) + string("' not found.") );



/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CCharacterSheet :
// Constructor.
//-----------------------------------------------
CCharacterSheet::CCharacterSheet()
{
	CharacterScalePos	= 1;
	Scale				= 1.f;
	SoundFamily			= 0;
	SoundVariation		= 0;
	Type				= CEntitySheet::FAUNA;
	Gender				= 0;
	Race				= EGSPD::CPeople::EndPeople;
	IdSkelFilename		= 0;
	IdAnimSetBaseName	= 0;
	IdAutomaton			= 0;
	IdLodCharacterName	= 0;
	LodCharacterDistance	= 0.0f;
	IdFame				= 0;
	DisplayOSD			= true;
	DisplayInRadar		= true;
	DisplayOSDName		= true;
	DisplayOSDBars		= true;
	DisplayOSDForceOver	= false;
	Traversable			= true;
	ClipRadius= 0.f;
	ClipHeight= 0.f;
	R2Npc = false;

	Selectable			= false;
	Talkable			= false;
	Attackable			= false;
	Givable				= false;
	Mountable			= false;
	Turn				= false;
	SelectableBySpace	= false;

	HLState				= LHSTATE::NONE;

	HairColor			= 0;
	Skin				= 0;
	EyesColor			= 0;

	DistToFront			= 0.0f;
	DistToBack			= 0.0f;
	DistToSide			= 0.0f;

	ColRadius			= 0.0f;
	ColHeight			= 0.0f;
	ColLength			= 0.0f;
	ColWidth			= 0.0f;

	MaxSpeed			= 0.0f;

	NamePosZLow			= 0.0f;
	NamePosZNormal		= 0.0f;
	NamePosZHigh		= 0.0f;

	IdStaticFX			= 0;

	SpellCastingPrefix	= 0;

	RegionForce			= 0;
	ForceLevel			= 0;
	Level               = 0;

}// CCharacterSheet //

//-----------------------------------------------
// readEquipment :
// Read an equipment slot.
//-----------------------------------------------
void CCharacterSheet::readEquipment(const NLGEORGES::UFormElm &form, const string &key, CEquipment &slot)
{
	// Get the item (or shape) name.
	string itemName;
	if(!form.getValueByName(itemName, string(key + ".Item").c_str() ))
		debug(NLMISC::toString("Key '%s.Item' not found.", key.c_str()));
	slot.IdItem = ClientSheetsStrings.add(NLMISC::strlwr(itemName));

	// Get the texture.
	if(!form.getValueByName(slot.Texture, string(key + ".Texture").c_str() ))
		debug(NLMISC::toString("Key '%s.Texture' not found.", key.c_str()));

	// Get the color.
	if(!form.getValueByName(slot.Color, string(key + ".Color").c_str() ))
		debug(NLMISC::toString("Key '%s.Color' not found.", key.c_str()));

	// Get the Bind point.
	string bindPointName;
	if(!form.getValueByName(bindPointName, string(key + ".Bind Point").c_str() ))
		debug(NLMISC::toString("Key '%s.Bind Point' not found.", key.c_str()));
	slot.IdBindPoint = ClientSheetsStrings.add(bindPointName);
}// readEquipment //

//-----------------------------------------------
// build :
// Build the sheet from an external script.
//-----------------------------------------------
void CCharacterSheet::build(const NLGEORGES::UFormElm &item)
{
	// First Name
//	string FirstName;
//	if(!item.getValueByName(FirstName, "Basics.First Name"))
//		debug("Key 'Basics.First Name' not found.");
//	IdFirstName = ClientSheetsStrings.add(FirstName);
//	// Character Name
//	string LastName;
//	if(!item.getValueByName(LastName, "Basics.CharacterName"))
//		debug("Key 'Basics.CharacterName' not found.");
//	IdLastName = ClientSheetsStrings.add(LastName);

	// Fame Name
	string FameName;
	if(!item.getValueByName(FameName, "Basics.Fame"))
		debug("Key 'Basics.Fame' not found.");
	IdFame = ClientSheetsStrings.add(FameName);

	// Character Race
	string raceStr;
	if(!item.getValueByName(raceStr, "Basics.Race"))
		debug("Key 'Basics.Race' not found.");
	else if (!raceStr.empty())
	{
		Race = EGSPD::CPeople::fromString(raceStr);

		if (EGSPD::CPeople::toString(Race) != raceStr)
		{
			debug(toString("In sheet '%s': invalid race '%s', race is set to unknow",
				Id.toString().c_str(),
				raceStr.c_str()));
		}
	}
	else
		Race = EGSPD::CPeople::Unknown;

	// Player Gender.
	if(!item.getValueByName(Gender, "Basics.Gender"))
		debug("Key 'Gender' not found.");

	// Load the skel.
	string SkelFilename;
	if(!item.getValueByName(SkelFilename, "3d data.Skel"))
		debug("Key '3d data.Skel' not found.");
	IdSkelFilename = ClientSheetsStrings.add(SkelFilename);

	// BODY
	readEquipment(item, "Basics.Equipment.Body", Body);
	// LEGS
	readEquipment(item, "Basics.Equipment.Legs", Legs);
	// ARMS
	readEquipment(item, "Basics.Equipment.Arms", Arms);
	// HANDS
	readEquipment(item, "Basics.Equipment.Hands", Hands);
	// FEET
	readEquipment(item, "Basics.Equipment.Feet", Feet);
	// HEAD
	readEquipment(item, "Basics.Equipment.Head", Head);
	// FACE
	readEquipment(item, "Basics.Equipment.Face", Face);
	// IN RIGHT HAND
	readEquipment(item, "Basics.Equipment.HandR", ObjectInRightHand);
	// IN LEFT HAND
	readEquipment(item, "Basics.Equipment.HandL", ObjectInLeftHand);


	// Get the animation set Base Name.
	string AnimSetBaseName;
	if(item.getValueByName(AnimSetBaseName, "3d data.AnimSetBaseName"))
	{
		if(AnimSetBaseName.empty())
			debug("AnimSetBaseName is Empty.");
		else
			NLMISC::strlwr(AnimSetBaseName); // Force the CASE in UPPER to not be CASE SENSITIVE.
	}
	else
		debug("Key '3d data.AnimSetBaseName' not found.");
	IdAnimSetBaseName = ClientSheetsStrings.add(AnimSetBaseName);
	// AUTOMATON TYPE
	string Automaton;
	if(item.getValueByName(Automaton, "3d data.Automaton"))
	{
		// Check there is an automaton
		if(Automaton.empty())
			debug("Automaton is Empty.");
		// Lower Case
		else
			NLMISC::strlwr(Automaton);
	}
	// Key not Found
	else
		debug("Key '3d data.Automaton' not found.");
	IdAutomaton = ClientSheetsStrings.add(Automaton);
	// Display OSD
	if(!item.getValueByName(DisplayOSD, "3d data.DisplayOSD"))
	{
		debug("Key '3d data.DisplayOSD' not found -> set 'true'.");
		DisplayOSD = true;
	}

	// New Bot Object flags
	TRANSLATE_VAL(DisplayInRadar, "3d data.DisplayInRadar");
	TRANSLATE_VAL(DisplayOSDName,"3d data.DisplayName");
	TRANSLATE_VAL(DisplayOSDBars,"3d data.DisplayBars");
	TRANSLATE_VAL(DisplayOSDForceOver, "3d data.DisplayAlwaysNameOver");
	TRANSLATE_VAL(Traversable, "Collision.NotTraversable");
	// invert
	Traversable= !Traversable;

	// CREATURE PROPERTIES (Possible Actions)
	// Is the character selectable ?
	if(!item.getValueByName(Selectable, "Properties.Selectable"))
	{
		debug("Key 'Properties.Selectable' not found -> set 'false'.");
		Selectable = false;
	}
	// Is the character Talkable ?
	if(!item.getValueByName(Talkable, "Properties.Talkable"))
	{
		debug("Key 'Properties.Talkable' not found -> set 'false'.");
		Talkable = false;
	}
	// Is the character Attackable ?
	if(!item.getValueByName(Attackable, "Properties.Attackable"))
	{
		debug("Key 'Properties.Attackable' not found -> set 'false'.");
		Attackable = false;
	}
	// Is the character Givable ?
	if(!item.getValueByName(Givable, "Properties.Givable"))
	{
		debug("Key 'Properties.Givable' not found -> set 'false'.");
		Givable = false;
	}
	// Is the character Mountable ?
	if(!item.getValueByName(Mountable, "Properties.Mountable"))
	{
		debug("Key 'Properties.Mountable' not found -> set 'false'.");
		Mountable = false;
	}
	// Is the character allowed to turn ?
	if(!item.getValueByName(Turn, "Properties.Turn"))
	{
		debug("Key 'Properties.Turn' not found -> set 'true'.");
		Turn = true;
	}
	// Is the character selectable by pressing space (default) ?
	if(!item.getValueByName(SelectableBySpace, "Properties.SelectableBySpace"))
	{
		debug("Key 'Properties.SelectableBySpace' not found -> set 'true'.");
		SelectableBySpace = true;
	}
	//Get the harvest/loot state
	string harvestLootStr;
	if( !item.getValueByName(harvestLootStr, "Properties.LootHarvestState") )
		debug("Key 'roperties.LootHarvestState' not found.");
	else
		HLState = LHSTATE::stringToLHState(harvestLootStr);



	// Get the Hair Color.
	if(!item.getValueByName(HairColor, "3d data.HairColor"))
		debug("Key '3d data.HairColor' not found.");

	// Get the Skin Texture.
	if(!item.getValueByName(Skin, "3d data.Skin"))
		debug("Key '3d data.Skin' not found.");

	// Get the Eyes Color.
	if(!item.getValueByName(EyesColor, "3d data.EyesColor"))
		debug("Key '3d data.EyesColor' not found.");

	// Load Lod character name
	string LodCharacterName;
	if(!item.getValueByName(LodCharacterName, "3d data.LodCharacterName"))
		debug("Key '3d data.LodCharacterName' not found.");
	IdLodCharacterName = ClientSheetsStrings.add(LodCharacterName);

	// Load Lod character apparition distance
	if(!item.getValueByName(LodCharacterDistance, "3d data.LodCharacterDistance"))
		debug("Key '3d data.LodCharacterDistance' not found.");

	// value to scale the "pos" channel of the animation of the player.
	if(!item.getValueByName(CharacterScalePos, "3d data.CharacterScalePos"))
		debug("Key '3d data.CharacterScalePos' not found.");

	// value to scale the "pos" channel of the animation of the player.
	if(!item.getValueByName(Scale, "3d data.Scale"))
		debug("Key '3d data.Scale' not found.");
	else
	{
		if(Scale <= 0.0f)
		{
			nlwarning("CCharacterSheet:build: Scale(%f) <= 0.0 so fix scale to 1.0", Scale);
			Scale = 1.0f;
		}
	}

	// Load name positions on Z axis
	if(!item.getValueByName(NamePosZLow, "3d data.NamePosZLow"))
	{
		NamePosZLow = 0.f;
		debug("Key '3d data.NamePosZLow' not found.");
	}
	if(!item.getValueByName(NamePosZNormal, "3d data.NamePosZNormal"))
	{
		NamePosZNormal = 0.f;
		debug("Key '3d data.NamePosZNormal' not found.");
	}
	if(!item.getValueByName(NamePosZHigh, "3d data.NamePosZHigh"))
	{
		NamePosZHigh = 0.f;
		debug("Key '3d data.NamePosZHigh' not found.");
	}

	// value to change sound familly
	if(!item.getValueByName(SoundFamily, "3d data.SoundFamily"))
		debug("Key '3d data.SoundFamily' not found.");
	// value to change sound variation
	if(!item.getValueByName(SoundVariation, "3d data.SoundVariation"))
		debug("Key '3d data.SoundVariation' not found.");


	// Get the dist fromm Bip to Mid
	float tmpBip01ToMid;
	if(!item.getValueByName(tmpBip01ToMid, "Collision.Dist Bip01 to mid"))
	{
		tmpBip01ToMid = 0.f;
		debug("Key 'Collision.Dist Bip01 to mid' not found.");
	}
	// Get the distance from the bip01 to the front.
	if(!item.getValueByName(DistToFront, "Collision.Dist Bip01 to front"))
	{
		DistToFront = 1.f;
		debug("Key 'Collision.Dist Bip01 to front' not found.");
	}
	// Get the distance from the bip01 to the front.
	if(!item.getValueByName(DistToBack, "Collision.Dist Bip01 to back"))
	{
		DistToBack = 1.f;
		debug("Key 'Collision.Dist Bip01 to back' not found.");
	}
	// Get the creature Width.
	if(!item.getValueByName(ColWidth, "Collision.Width"))
	{
		ColWidth = 1.f;
		debug("Key 'Collision.Width' not found.");
	}
	DistToSide = ColWidth;

	DistToFront	= DistToFront-tmpBip01ToMid;
	DistToBack	= tmpBip01ToMid-DistToBack;
	DistToSide	= DistToSide/2.f;

	// Get the creature collision Radius.
	if(!item.getValueByName(ColRadius, "Collision.CollisionRadius"))
	{
		ColRadius = 0.5f;
		debug("Key 'Collision.CollisionRadius' not found.");
	}
	// Get the creature collision Height.
	if(!item.getValueByName(ColHeight, "Collision.Height"))
	{
		ColHeight = 2.f;
		debug("Key 'Collision.Height' not found.");
	}
	// Get the creature collision Length.
	if(!item.getValueByName(ColLength, "Collision.Length"))
	{
		ColLength = 1.f;
		debug("Key 'Collision.Length' not found.");
	}


	// CLIP
	if(!item.getValueByName(ClipRadius, "Collision.ClipRadius"))
	{
		ClipRadius = 0.f;
		debug("Key 'Collision.ClipRadius' not found.");
	}
	if(!item.getValueByName(ClipHeight, "Collision.ClipHeight"))
	{
		ClipHeight = 0.f;
		debug("Key 'Collision.ClipHeight' not found.");
	}


	// SPEED //
	// Get the creature Max Speed (Run).
	if(!item.getValueByName(MaxSpeed, "Basics.MovementSpeeds.RunSpeed"))
	{
		MaxSpeed = 10.f;
		debug("Key 'Basics.MovementSpeeds.RunSpeed' not found.");
	}

	const UFormElm *elm = NULL;
	// Get all alternative Clothes.
	static const char alternativeClothesKey[] = "Basics.Alternative Clothes";
	if(item.getNodeByName(&elm, alternativeClothesKey) && elm)
	{
		// Check array.
		if(elm->isArray())
		{
			// Get Array Size
			uint altClothesArraySize;
			if(elm->getArraySize(altClothesArraySize))
			{
				// Get values.
				string altClothes;
				for(uint i=0; i<altClothesArraySize; ++i)
				{
					if(elm->getArrayValue(altClothes, i))
					{
						if(!altClothes.empty())
						{
							TSStringId IdAltClothes = ClientSheetsStrings.add(altClothes);
							IdAlternativeClothes.push_back(IdAltClothes);
						}
						else
							debug(toString("'%s' field empty for the index '%d'.", alternativeClothesKey, i));
					}
					else
						debug(toString("'%s' cannot get the array value for the index '%d'.", alternativeClothesKey, i));
				}
			}
			else
				debug(toString("'%s' cannot get the array size.", alternativeClothesKey));
		}
		else
			debug(toString("'%s' is not an array.", alternativeClothesKey));
	}
	else
		debug(toString("'%s' key not found.", alternativeClothesKey));

	// Hair item list
	static const char hairItemList[] = "3d data.HairItem";
	if(item.getNodeByName(&elm, hairItemList) && elm)
	{
		// Check array.
		if(elm->isArray())
		{
			// Get Array Size
			uint hairItemArraySize;
			if(elm->getArraySize(hairItemArraySize))
			{
				if(hairItemArraySize > 0)
				{
					// Adjust the array size.
					HairItemList.resize(hairItemArraySize);

					// Get values.
					for(uint i=0; i<hairItemArraySize; ++i)
					{
						string arrayNodeName;
						if(elm->getArrayNodeName(arrayNodeName, i))
							readEquipment(item, string(hairItemList)+"["+toString(i)+"]", HairItemList[i]);
					}
				}
			}
			else
				debug(toString("'%s' cannot get the array size.", hairItemList));
		}
		else
			debug(toString("'%s' is not an array.", hairItemList));
	}
	else
		debug(toString("'%s' key not found.", hairItemList));

	// ground fxs
	static const char groundFXList[] = "3d data.GroundFX";
	if(item.getNodeByName(&elm, groundFXList) && elm)
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
					GroundFX.reserve(groundFXArraySize);

					// Get values.
					for(uint i=0; i< groundFXArraySize; ++i)
					{
						const UFormElm *node;
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
								for(k = 0; k < GroundFX.size(); ++k)
								{
									if (GroundFX[k].GroundID == gfs.GroundID)
									{
										debug("Duplicated material");
										GroundFX[k] = gfs;
										break;
									}
								}
								if (k == GroundFX.size())
								{
									GroundFX.push_back(gfs);
								}
							}
						}
					}
				}
			}
			else
				debug(toString("'%s' cannot get the array size.", groundFXList));
		}
		else
			debug(toString("'%s' is not an array.", groundFXList));
	}
	else
		debug(toString("'%s' key not found.", groundFXList));
	// sort fxs by ground type
	std::sort(GroundFX.begin(), GroundFX.end());

	// Load Ground FX
	string staticFx;
	if(!item.getValueByName(staticFx, "3d data.FX"))
		debug("Key '3d data.FX' not found.");
	IdStaticFX = ClientSheetsStrings.add(staticFx);

	BodyToBone.build(item, "Localisation.");

	// Attack lists
	for(uint k = 0; k < NumAttackLists; ++k)
	{
		std::string attackListName;
		if(item.getValueByName(attackListName, toString("attack_list%d", (int) k).c_str()) && !attackListName.empty())
		{
			AttackLists.push_back(ClientSheetsStrings.add(attackListName));
		}
	}

	if(!item.getValueByName(RegionForce, "Basics.RegionForce"))
	{
		RegionForce = 0;
		debug("Key 'Basics.RegionForce' not found.");
	}

	if(!item.getValueByName(ForceLevel, "Basics.ForceLevel"))
	{
		ForceLevel = 0;
		debug("Key 'Basics.Regio Force' not found.");
	}

	if(!item.getValueByName(Level, "Basics.XPLevel"))
	{
		Level = 0;
		debug("Key 'Basics.XPLevel' not found.");
	}

	// offset for projectiles
	const UFormElm *pElt;
	nlverify (item.getNodeByName (&pElt, "3d data.ProjectileCastRay"));
	uint arraySize;
	if (pElt != NULL)
	{
		nlverify (pElt->getArraySize (arraySize));
		ProjectileCastRay.reserve(arraySize);
		for (uint32 i = 0; i < arraySize; ++i)
		{
			const UFormElm *pEltOfList;
			if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
			{
				const UFormElm *pEltPos;
				const UFormElm *pEltOrigin;
				if (pEltOfList->getNodeByName(&pEltPos, "Pos") &&
					pEltOfList->getNodeByName(&pEltOrigin, "Origin"))
				{
					CCharacterSheet::CCastRay cr;
					if (pEltPos->getValueByName(cr.Pos.x, "X") &&
						pEltPos->getValueByName(cr.Pos.y, "Y") &&
						pEltPos->getValueByName(cr.Pos.z, "Z") &&
						pEltOrigin->getValueByName(cr.Origin.x, "X") &&
						pEltOrigin->getValueByName(cr.Origin.y, "Y") &&
						pEltOrigin->getValueByName(cr.Origin.z, "Z")
					   )
					{
						ProjectileCastRay.push_back(cr);
					}
				}
			}
		}
	}

	if(!item.getValueByName(R2Npc, "r2_npc"))
	{
		R2Npc = false;
		debug("Key 'R2Npc' not found.");
	}
}// build //


//-----------------------------------------------
// serial :
// Serialize character sheet into binary data file.
//-----------------------------------------------
void CCharacterSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serialize class components.
//	ClientSheetsStrings.serial(f, IdFirstName);
//	ClientSheetsStrings.serial(f, IdLastName);
	f.serial(Gender);
	f.serialEnum(Race);
	ClientSheetsStrings.serial(f, IdSkelFilename);
	ClientSheetsStrings.serial(f, IdAnimSetBaseName);
	ClientSheetsStrings.serial(f, IdAutomaton);
	f.serial(Scale);
	f.serial(SoundFamily);
	f.serial(SoundVariation);
	ClientSheetsStrings.serial(f, IdLodCharacterName);

	f.serial(LodCharacterDistance);
	f.serial(Selectable);
	f.serial(Talkable);
	f.serial(Attackable);
	f.serial(Givable);
	f.serial(Mountable);
	f.serial(Turn);
	f.serial(SelectableBySpace);
	f.serialEnum(HLState);
	f.serial(CharacterScalePos);
	f.serial(NamePosZLow);
	f.serial(NamePosZNormal);
	f.serial(NamePosZHigh);
	ClientSheetsStrings.serial(f, IdFame);

	f.serial(Body);
	f.serial(Legs);
	f.serial(Arms);
	f.serial(Hands);
	f.serial(Feet);
	f.serial(Head);
	f.serial(Face);
	f.serial(ObjectInRightHand);
	f.serial(ObjectInLeftHand);

	f.serial(HairColor);
	f.serial(Skin);
	f.serial(EyesColor);

	f.serial(DistToFront);
	f.serial(DistToBack);
	f.serial(DistToSide);

	// Collisions
	f.serial(ColRadius);
	f.serial(ColHeight);
	f.serial(ColLength);
	f.serial(ColWidth);
	f.serial(MaxSpeed);

	// Clip
	f.serial(ClipRadius);
	f.serial(ClipHeight);

	// Alternative Look
	ClientSheetsStrings.serial(f, IdAlternativeClothes);

	// Hair Item List
	f.serialCont(HairItemList);
	// Ground fxs
	f.serialCont(GroundFX);
	// Display OSD
	f.serial(DisplayOSD);
	// static FX
	ClientSheetsStrings.serial(f, IdStaticFX);
	// body to bone
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

	// bot object flags
	f.serial(DisplayInRadar);
	f.serial(DisplayOSDName);
	f.serial(DisplayOSDBars);
	f.serial(DisplayOSDForceOver);
	f.serial(Traversable);

	f.serial(RegionForce);
	f.serial(ForceLevel);
	f.serial(Level);

	f.serialCont(ProjectileCastRay);

	f.serial(R2Npc);

}// serial //

// ***************************************************************************
void CCharacterSheet::getWholeEquipmentList(std::vector<const CEquipment*> &equipList) const
{
	equipList.clear();
	equipList.push_back(&Body);
	equipList.push_back(&Legs);
	equipList.push_back(&Arms);
	equipList.push_back(&Hands);
	equipList.push_back(&Feet);
	equipList.push_back(&Head);
	equipList.push_back(&Face);
	equipList.push_back(&ObjectInRightHand);
	equipList.push_back(&ObjectInLeftHand);
	for(uint i=0;i<HairItemList.size();i++)
	{
		equipList.push_back(&HairItemList[i]);
	}
}

