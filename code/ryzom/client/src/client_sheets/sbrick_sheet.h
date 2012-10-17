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



#ifndef RY_SBRICK_SHEET_H
#define RY_SBRICK_SHEET_H

#include "client_sheets.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "game_share/brick_types.h"
#include "game_share/skills.h"
#include "game_share/brick_families.h"
#include "game_share/rm_family.h"
#include "game_share/crafting_tool_type.h"
#include "entity_sheet.h"
#include "game_share/action_nature.h"
#include "game_share/people.h"
#include "game_share/fame.h"
#include "game_share/resistance_type.h"

// ***************************************************************************
#define	SBRICK_MAX_MANDATORY		12
#define	SBRICK_MAX_OPTIONAL			32
#define	SBRICK_MAX_PARAMETER		4
#define	SBRICK_MAX_CREDIT			12


// ***************************************************************************
// The max number of different MP required for a faber sentence. This applies for ItemPart MPs, and Formula MPs
#define	MAX_FABER_REQ_MP		5


// ***************************************************************************
/**
 * New Sabrina Brick Sheet def.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CSBrickSheet : public CEntitySheet
{
public:
	// MaxProperties in Sheet
	enum	{MaxProperties= 20};

	// A property of a brick
	struct CProperty
	{
		// The whole param in georges.
		std::string		Text;
		// translated at runTime with CSBrickManager to PropId/Value (not serialised)
		uint			PropId;
		float			Value;
		float			Value2;

		CProperty()
		{
			PropId=0;
			Value= 0.f;
			Value2=0.f;
		}

		void	serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(Text);
		}
	};

	// The Faber plan For Craft mandatories.
	struct	CFaberPlan
	{
		CFaberPlan()
		{
			NbItemBuilt= 0;
			ToolType = TOOL_TYPE::Unknown;
		}

		// ItemPartMP: a request for MPs that can build required item part
		struct	CItemPartMP
		{
			// If a filter is Unknown, then all Mps matchs
			RM_FABER_TYPE::TRMFType				FaberTypeFilter;
			uint32								Quantity;

			CItemPartMP()
			{
				FaberTypeFilter = RM_FABER_TYPE::Unknown;
				Quantity = 0;
			}

			void serial (NLMISC::IStream &s) throw(NLMISC::EStream)
			{
				s.serialEnum(FaberTypeFilter);
				s.serial(Quantity);
			}

		};

		// FormulaMP: a specific MP (or can be any item) that the faber requires in addition to CItemPartMPs
		struct CFormulaMP
		{
			NLMISC::CSheetId			ItemRequired;
			uint32						Quantity;
			void serial (NLMISC::IStream &s) throw(NLMISC::EStream)
			{
				s.serial(ItemRequired);
				s.serial(Quantity);
			}

			CFormulaMP()
			{
				Quantity = 0;
			}
		};

		// Faber
		NLMISC::CSheetId				ItemBuilt;
		std::vector<CItemPartMP>		ItemPartMps;
		std::vector<CFormulaMP>			FormulaMps;
		TOOL_TYPE::TCraftingToolType	ToolType;
		uint32							NbItemBuilt;


		void serial (NLMISC::IStream &s) throw(NLMISC::EStream)
		{
			s.serial(ItemBuilt);
			s.serialCont(ItemPartMps);
			s.serialCont(FormulaMps);
			s.serialEnum(ToolType);
			s.serial(NbItemBuilt);
		}
	};

	// a serializable Skill (special for serial)
	class CSkillSerial
	{
	public:
		SKILLS::ESkills		Skill;

		void	serial(NLMISC::IStream	&f)
		{
			f.serialEnum(Skill);
		};

		CSkillSerial() : Skill(SKILLS::unknown) {}
		CSkillSerial(SKILLS::ESkills e) : Skill(e) {}
		CSkillSerial	&operator=(SKILLS::ESkills e)
		{
			Skill= e;
			return *this;
		}
		operator SKILLS::ESkills() const {return Skill;}
	};

	// For progression
	class CSkillValue
	{
	public:
		SKILLS::ESkills		Skill;
		uint32				Value;

		CSkillValue()
		{
			Skill = SKILLS::unknown;
			Value = 0;
		}

		void	serial(NLMISC::IStream	&f)
		{
			f.serialEnum(Skill);
			f.serial(Value);
		}
	};

public:
	// For Combat only, this array may be >1. One of the skills match for combat
	// guaranteed to be at least of size 1, with Unknown skill
	std::vector<CSkillSerial>		UsedSkills;
	BRICK_FAMILIES::TBrickFamily	BrickFamily;
	uint8							IndexInFamily;
	uint8							Level;
	NLMISC::TSStringId				IdIcon;
	NLMISC::TSStringId				IdIconBack;
	NLMISC::TSStringId				IdIconOver;
	NLMISC::TSStringId				IdIconOver2;
	NLMISC::CRGBA					IconColor;
	NLMISC::CRGBA					IconBackColor;
	NLMISC::CRGBA					IconOverColor;
	NLMISC::CRGBA					IconOver2Color;
	sint32							SabrinaCost;
	float							SabrinaRelativeCost;
	// Brick Properties
	std::vector<CProperty>			Properties;

	std::vector<uint16>				MandatoryFamilies;
	std::vector<uint16>				OptionalFamilies;
	std::vector<uint16>				ParameterFamilies;
	std::vector<uint16>				CreditFamilies;

	// Forbidden Brick KeyWords.
	NLMISC::TSStringId				IdForbiddenDef;
	NLMISC::TSStringId				IdForbiddenExclude;

	// The Faber Repair/Refine/Create plan
	CFaberPlan						FaberPlan;

	/// Min/Max Casting Time (1/10sec), and range
	uint8							MinCastTime;
	uint8							MaxCastTime;
	/// Min/Max Range (in meters)
	uint8							MinRange;
	uint8							MaxRange;

	// Brick flags required (0 by default)
	uint64							BrickRequiredFlags;

	// The SkillPoint Cost
	uint32							SPCost;

	// The Action Nature
	ACTNATURE::TActionNature		ActionNature;

	// For Progression, the required skill
	std::vector<CSkillValue>		RequiredSkills;

	// For Progression, the required Bricks
	std::vector<NLMISC::CSheetId>	RequiredBricks;

	// true if a phrase containing this brick can be cyclic
	bool							AvoidCyclic;

	// if true this brick can't be used when no item in hands
	bool							UsableWithEmptyHands;

	// Brick civilisation restriction
	EGSPD::CPeople::TPeople			CivRestriction;

	//	faction to check if min fame value != 0
	uint32							FactionIndex;

	// min fame value to learn this brick
	sint32							MinFameValue;

	// Magic Only: The Domain of this spell (ie The resistance type)
	RESISTANCE_TYPE::TResistanceType	MagicResistType;

public:
	// true if the brick is a Root, according to BrickFamily
	bool							isRoot() const;
	// true if the brick is a Credit, according to BrickFamily
	bool							isCredit() const;
	// true if the brick is a Mandatory, according to BrickFamily
	bool							isMandatory() const;
	// true if the brick is a Optional, according to BrickFamily
	bool							isOptional() const;
	// true if the brick is a Parameter, according to BrickFamily
	bool							isParameter() const;

	// return the brick type, according to BrickFamily
	BRICK_TYPE::EBrickType			getBrickType() const {return BRICK_FAMILIES::brickType(BrickFamily);}
	// true if the brick is Combat Brick, according to BrickFamily
	bool							isCombat() const;
	// true if the brick is Magic Brick, according to BrickFamily
	bool							isMagic() const;
	// true if the brick is Faber Brick, according to BrickFamily
	bool							isFaber() const;
	// true if the brick is Harvest Brick, according to BrickFamily
	bool							isHarvest() const;
	// true if the brick is Forage Prospection Brick, according to BrickFamily
	bool							isForageProspection() const;
	// true if the brick is Forage Extraction Brick, according to BrickFamily
	bool							isForageExtraction() const;
	// true if the brick is SpecialPower Brick, according to BrickFamily
	bool							isSpecialPower() const;
	// true if the brick is Enchantment Brick, according to BrickFamily
	bool							isProcEnchantment() const;

	// true if must display the level index
	bool							mustDisplayLevel() const;

public:

	CSBrickSheet ()
	{
		UsedSkills.push_back(SKILLS::unknown);
		Type = SBRICK;
		BrickFamily = BRICK_FAMILIES::Unknown;
		IndexInFamily = 0;
		Level = 0;
		IdIcon = 0;
		IdIconBack = 0;
		IdIconOver = 0;
		IdIconOver2 =0;
		IconColor= NLMISC::CRGBA::White;
		IconBackColor= NLMISC::CRGBA::White;
		IconOverColor= NLMISC::CRGBA::White;
		IconOver2Color= NLMISC::CRGBA::White;
		SabrinaCost= 0;
		SabrinaRelativeCost = 0.0f;
		IdForbiddenDef = 0;
		IdForbiddenExclude = 0;
		MinCastTime= 10;
		MaxCastTime= 100;
		MinRange= 1;
		MaxRange= 10;
		BrickRequiredFlags= 0;
		SPCost= 0;
		AvoidCyclic= false;
		UsableWithEmptyHands = true;
		FactionIndex = CStaticFames::INVALID_FACTION_INDEX;
		MinFameValue = NO_FAME;
		ActionNature= ACTNATURE::UNKNOWN;
		CivRestriction= EGSPD::CPeople::Common;
		MagicResistType= RESISTANCE_TYPE::None;
	}


	std::string getIcon() const { return ClientSheetsStrings.get(IdIcon); }
	std::string getIconBack() const { return ClientSheetsStrings.get(IdIconBack); }
	std::string getIconOver() const { return ClientSheetsStrings.get(IdIconOver); }
	std::string getIconOver2() const { return ClientSheetsStrings.get(IdIconOver2); }
	std::string getForbiddenDef() const { return ClientSheetsStrings.get(IdForbiddenDef); }
	std::string getForbiddenExclude() const { return ClientSheetsStrings.get(IdForbiddenExclude); }

	// Works For all brick but combat. combat may have more than one Skill
	SKILLS::ESkills		getSkill() const {return UsedSkills[0];}

	// Georges Std build implementation
	virtual void build (const NLGEORGES::UFormElm &root);

	virtual void serial (NLMISC::IStream &s) throw(NLMISC::EStream)
	{
		std::string sTmp;
		s.serialCont(UsedSkills);
		s.serialEnum(BrickFamily);
		s.serial (IndexInFamily);
		s.serial (Level);
		s.serial (sTmp);
		ClientSheetsStrings.serial(s, IdIcon);
		ClientSheetsStrings.serial(s, IdIconBack);
		ClientSheetsStrings.serial(s, IdIconOver);
		ClientSheetsStrings.serial(s, IdIconOver2);

		s.serial (IconColor);
		s.serial (IconBackColor);
		s.serial (IconOverColor);
		s.serial (IconOver2Color);
		s.serial (SabrinaCost);
		s.serial (SabrinaRelativeCost);
		s.serialCont (MandatoryFamilies);
		s.serialCont (OptionalFamilies);
		s.serialCont (ParameterFamilies);
		s.serialCont (CreditFamilies);

		ClientSheetsStrings.serial(s, IdForbiddenDef);
		ClientSheetsStrings.serial(s, IdForbiddenExclude);

		s.serial (FaberPlan);
		s.serialCont(Properties);
		s.serial(MinCastTime);
		s.serial(MaxCastTime);
		s.serial(MinRange);
		s.serial(MaxRange);

		s.serial(BrickRequiredFlags);

		s.serial(SPCost);

		s.serialEnum(ActionNature);

		s.serialCont(RequiredSkills);

		s.serialCont(RequiredBricks);

		s.serial(AvoidCyclic);

		s.serial(UsableWithEmptyHands);

		s.serialEnum(CivRestriction);

		s.serial(FactionIndex);

		s.serial(MinFameValue);

		s.serialEnum(MagicResistType);
	}

};


#endif // NL_SBRICK_SHEET_H

/* End of sbrick_sheet.h */
