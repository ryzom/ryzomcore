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




#ifndef CL_CHARACTER_SHEET_H
#define CL_CHARACTER_SHEET_H


/////////////
// INCLUDE //
/////////////
#include "client_sheets.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
//
#include "entity_sheet.h"
#include "ground_fx_sheet.h"
#include "body_to_bone_sheet.h"
// Game Share
#include "game_share/loot_harvest_state.h"
#include "game_share/people.h"
#include <vector>
#include <string>


///////////
// USING //
///////////


///////////
// CLASS //
///////////
namespace NLGEORGES
{
	class UFormElm;
	class UFormLoader;
}

/**
 * Class to manage the character sheet.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CCharacterSheet : public CEntitySheet
{
public:
	enum { NumAttackLists = 8 };
public:
	class CEquipment
	{
	public:
		NLMISC::TSStringId	IdItem;
		NLMISC::TSStringId	IdBindPoint;
		sint8				Texture;
		sint8				Color;

		CEquipment()
		{
			IdItem= NLMISC::CStaticStringMapper::emptyId();
			IdBindPoint= NLMISC::CStaticStringMapper::emptyId();
			Texture= 0;
			Color= 0;
		}

		std::string	getItem() const { return ClientSheetsStrings.get(IdItem); }
		std::string	getBindPoint() const { return ClientSheetsStrings.get(IdBindPoint); }

		/// Serialize character sheet into binary data file.
		virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{

			ClientSheetsStrings.serial(f, IdItem);
			f.serial(Texture);
			f.serial(Color);
			ClientSheetsStrings.serial(f, IdBindPoint);
		}
	};

	// Character Gender.
	uint8						Gender;
	// Character Race
	EGSPD::CPeople::TPeople		Race;
	// Character's skeleton.
	NLMISC::TSStringId			IdSkelFilename;
	// Base Name of the animation set.
	NLMISC::TSStringId			IdAnimSetBaseName;
	// Automaton Type
	NLMISC::TSStringId			IdAutomaton;
	float						Scale;
	// The sound familly (used for sound context var 2)
	uint32						SoundFamily;
	// The sound variation (used for sound context var 3)
	uint32						SoundVariation;
	// Lod Character.
	NLMISC::TSStringId			IdLodCharacterName;
	float						LodCharacterDistance;
	// value to scale the "pos" channel of the animation of the creature.
	float						CharacterScalePos;
	// The name of the faction the creature belongs to
	NLMISC::TSStringId			IdFame;

	// Possible(impossible) Actions.
	bool						Selectable;
	bool						Talkable;
	bool						Attackable;
	bool						Givable;
	bool						Mountable;
	bool						Turn;
	bool						SelectableBySpace;
	//character harvest/loot state
	LHSTATE::TLHState HLState;


	// Equipment worm or creature body.
	CEquipment					Body;
	CEquipment					Legs;
	CEquipment					Arms;
	CEquipment					Hands;
	CEquipment					Feet;
	CEquipment					Head;
	CEquipment					Face;
	CEquipment					ObjectInRightHand;
	CEquipment					ObjectInLeftHand;
	// Yoyo: if you add some, modify getWholeEquipmentList()


	sint8						HairColor;
	sint8						Skin;
	sint8						EyesColor;

//	NLMISC::TSStringId			IdFirstName;
//	NLMISC::TSStringId			IdLastName;

	float						DistToFront;
	float						DistToBack;
	float						DistToSide;

	float						ColRadius;
	float						ColHeight;
	float						ColLength;
	float						ColWidth;

	float						ClipRadius;
	float						ClipHeight;

	float						MaxSpeed;
	bool						DisplayOSD;
	// New flags created for bot objects
	bool						DisplayInRadar;			// display the entity in the radar
	bool						DisplayOSDName;			// name is displayed if (DisplayOSD && DisplayName)
	bool						DisplayOSDBars;			// bars are displayed if (DisplayOSD && DisplayBars)
	bool						DisplayOSDForceOver;	// even if ClientCfg.ShowNameUnderCursor==false, force OSD to display when under cursor (DisplayOSD must be true)
	bool						Traversable;			// the client can traverse this entity after some force time

	// Name positions on Z axis
	float						NamePosZLow;
	float						NamePosZNormal;
	float						NamePosZHigh;

	// Alternative Look
	std::vector<NLMISC::TSStringId>	IdAlternativeClothes;

	// Hair Item List
	std::vector<CEquipment>		HairItemList;

	// list of fx for some ground type (step fxs)
	std::vector<CGroundFXSheet> GroundFX;

	// name of static FX played on entity (empty if none)
	NLMISC::TSStringId			IdStaticFX;

	// body to bone
	CBodyToBoneSheet			BodyToBone;

	// spell casting prefix. This prefix is used to see which sheet contains datas about spell casting
	NLMISC::TSStringId			SpellCastingPrefix;

	// attack lists filenames
	std::vector<NLMISC::TSStringId>	AttackLists;

	// consider
	sint8						RegionForce;	// Force depending on the region the creature belongs
	sint8						ForceLevel;		// Level of creature inside the same region
	uint16						Level;			// Precise level of the creature

	bool						R2Npc;

	class CCastRay
	{
	public:
		NLMISC::CVector Origin;
		NLMISC::CVector Pos;
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(Origin);
			f.serial(Pos);
		}
	};
	// offset for projectile casting (useful for watchtowers as they got no bones)
	std::vector<CCastRay> ProjectileCastRay;

	/// Constructor
	CCharacterSheet();

	std::string getSkelFilename() const { return ClientSheetsStrings.get(IdSkelFilename); }
	std::string getAnimSetBaseName() const { return ClientSheetsStrings.get(IdAnimSetBaseName); }
	std::string getAutomaton() const { return ClientSheetsStrings.get(IdAutomaton); }
	std::string getLodCharacterName() const { return ClientSheetsStrings.get(IdLodCharacterName); }
	std::string getFame() const { return ClientSheetsStrings.get(IdFame); }
//	std::string getFirstName() const { return ClientSheetsStrings.get(IdFirstName); }
//	std::string getLastName() const { return ClientSheetsStrings.get(IdLastName); }
	std::string getAlternativeClothes(uint i) const { return ClientSheetsStrings.get(IdAlternativeClothes[i]); }
	std::string getStaticFX() const { return ClientSheetsStrings.get(IdStaticFX); }

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// Return the list of all equipement possibles (body... + HairList). Pointers should be used localy
	void	getWholeEquipmentList(std::vector<const CEquipment*> &equipList) const;

private:
	/// Read an equipment slot.
	void readEquipment(const NLGEORGES::UFormElm &form, const std::string &key, CEquipment &slot);
};


#endif // CL_CHARACTER_SHEET_H

/* End of character_sheet.h */










































