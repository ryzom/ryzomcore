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



#include "stdpch.h"
#include "sbrick_sheet.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/common.h"
#include "nel/misc/algo.h"
#include "item_sheet.h"
#include "game_share/brick_flags.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


// ***************************************************************************
// Easy Macro to translate .typ enum
#define	TRANSLATE_ENUM( _Var_, _unknown_, _func_, _key_)	\
	_Var_ = _unknown_;										\
	if( !root.getValueByName(val, _key_))					\
		debug("Key '" _key_ "' not found.");				\
	else if( (_Var_ = _func_(val)) == _unknown_ )			\
		debug(#_Var_ " Unknown: " + val);

// Same but no error if the result of enum is _unknown_
#define	TRANSLATE_ENUM_NODB( _Var_, _unknown_, _func_, _key_)	\
	_Var_ = _unknown_;										\
	if( !root.getValueByName(val, _key_))					\
		debug( toString("Key '%s' not found.", _key_) );				\
	else													\
		_Var_ = _func_(val);


// Easy macro to translate value from georges
#define TRANSLATE_VAL( _Var_, _key_ )						\
	if(!root.getValueByName(_Var_, _key_))					\
		debug( string("key '") + string(_key_) + string("' not found.") );


// ***************************************************************************
static void	strRemoveChar(string &str, char c)
{
	str.erase( remove(str.begin(), str.end(), c), str.end() );
}

// ***************************************************************************
void CSBrickSheet::build (const NLGEORGES::UFormElm &root)
{
	string		sTmp;
	uint		i;

	// read the array of skills
	string	skillUseStr;
	TRANSLATE_VAL(skillUseStr, "Basics.Skill");
	while(strFindReplace(skillUseStr, ":", " "));
	std::vector<string>		listSkill;
	splitString(skillUseStr," ",listSkill);
	// build the req skill array
	UsedSkills.clear();
	UsedSkills.reserve(listSkill.size());
	for(i=0;i<listSkill.size();i++)
	{
		SKILLS::ESkills	skill= SKILLS::toSkill(listSkill[i]);
		// Yoyo: patch to read auto generated bricks
		if(skill==SKILLS::unknown)
		{
			skill = (SKILLS::ESkills)SKILLS::toSkill("S" + listSkill[i]);
		}
		// keep only whats work
		if(skill!=SKILLS::unknown)
		{
			UsedSkills.push_back(skill);
		}
	}
	// if empty, fill at least with unknown
	if(UsedSkills.empty())
		UsedSkills.push_back(SKILLS::unknown);

	// get family id
	root.getValueByName (sTmp, "Basics.FamilyId" );
	BrickFamily = BRICK_FAMILIES::toSBrickFamily (sTmp);
	if(BrickFamily==BRICK_FAMILIES::Unknown)
		nlwarning("CSBrickSheet:build: BrickFamily Unknown '%s'.", sTmp.c_str());
/*
	// Yoyo: patch to read auto generated bricks
	if(BrickFamily==BRICK_FAMILIES::Unknown)
	{
		string	sheetName= Id.toString();
		std::string::size_type	end= sheetName.find(".sbrick")-2;
		BrickFamily = BRICK_FAMILIES::toSBrickFamily ( NLMISC::toUpper(sheetName.substr(0,end)) );
		if(BrickFamily==BRICK_FAMILIES::Unknown)
			nlwarning("Unknown Family for SBrick: %s", sheetName.c_str());
	}
*/
	root.getValueByName (IndexInFamily, "Basics.IndexInFamily" );
	root.getValueByName (Level, "Basics.Level" );
	// read icons
	string Icon;
	root.getValueByName (Icon, "Client.Icon" );
	Icon = strlwr(Icon);
	IdIcon = ClientSheetsStrings.add(Icon);

	string IconBack;
	root.getValueByName (IconBack, "Client.IconBack" );
	IconBack = strlwr(IconBack);
	IdIconBack = ClientSheetsStrings.add(IconBack);

	string IconOver;
	root.getValueByName (IconOver, "Client.IconOver" );
	IconOver = strlwr(IconOver);
	IdIconOver = ClientSheetsStrings.add(IconOver);

	string IconOver2;
	root.getValueByName (IconOver2, "Client.IconOver2" );
	IconOver2 = strlwr(IconOver2);
	IdIconOver2 = ClientSheetsStrings.add(IconOver2);

	root.getValueByName (IconColor, "Client.IconColor" );
	root.getValueByName (IconBackColor, "Client.IconBackColor");
	root.getValueByName (IconOverColor, "Client.IconOverColor");
	root.getValueByName (IconOver2Color, "Client.IconOver2Color");
	root.getValueByName (SabrinaCost, "Basics.SabrinaCost" );
	root.getValueByName (SabrinaRelativeCost, "Basics.SabrinaRelativeValue" );

	// mandatory families
	MandatoryFamilies.clear();
	char	tmp[256];
	for(i=0;i<SBRICK_MAX_MANDATORY;i++)
	{
		sprintf(tmp, "Mandatory.f%d", i);
		root.getValueByName (sTmp, tmp);
		if (!sTmp.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(sTmp);
			if(bf != BRICK_FAMILIES::Unknown)
				MandatoryFamilies.push_back( bf );
			else
				nlwarning("Unknown Mandatory family %s",sTmp.c_str());
		}
	}

	// Optional families
	OptionalFamilies.clear();
	for(i=0;i<SBRICK_MAX_OPTIONAL;i++)
	{
		sprintf(tmp, "Optional.f%d", i);
		root.getValueByName (sTmp, tmp);
		if (!sTmp.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(sTmp);
			if(bf != BRICK_FAMILIES::Unknown)
				OptionalFamilies.push_back( bf );
			else
				nlwarning("Unknown optional family %s",sTmp.c_str());
		}
	}

	// Parameter families
	ParameterFamilies.clear();
	for(i=0;i<SBRICK_MAX_PARAMETER;i++)
	{
		sprintf(tmp, "Parameter.f%d", i);
		root.getValueByName (sTmp, tmp);
		if (!sTmp.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(sTmp);
			if(bf != BRICK_FAMILIES::Unknown)
				ParameterFamilies.push_back( bf );
			else
				nlwarning("Unknown Parameter family %s",sTmp.c_str());
		}
	}

	// Credit families
	CreditFamilies.clear();
	for(i=0;i<SBRICK_MAX_CREDIT;i++)
	{
		sprintf(tmp, "Credit.f%d", i);
		root.getValueByName (sTmp, tmp);
		BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(sTmp);
		if(bf != BRICK_FAMILIES::Unknown)
			CreditFamilies.push_back( bf );
	}

	string ForbiddenDef;
	root.getValueByName (ForbiddenDef, "Basics.ForbiddenDef" );
	IdForbiddenDef = ClientSheetsStrings.add(ForbiddenDef);

	string ForbiddenExclude;
	root.getValueByName (ForbiddenExclude, "Basics.ForbiddenExclude" );
	IdForbiddenExclude = ClientSheetsStrings.add(ForbiddenExclude);

	// **** Properties
	Properties.clear();
	for(i=0;i<MaxProperties;i++)
	{
		string	val;
		root.getValueByName(val, toString("Basics.Property %d", i).c_str() );
		if(!val.empty() && val!="NULL")
		{
			CProperty	prop;
			prop.Text= val;
			Properties.push_back(prop);
		}
	}

	// **** Faber
	// The FaberPlan are stored in Mandatory only, but the tool filter is in root
	if( isFaber() && (isMandatory() || isRoot()) )
	{
		string	val;

		FaberPlan.ItemPartMps.clear();
		FaberPlan.FormulaMps.clear();

		// Get Item Built
		TRANSLATE_VAL(val, "faber.Create.Crafted Item");
		FaberPlan.ItemBuilt.buildSheetId(val);

		// Get Skill Filters
		TRANSLATE_ENUM ( FaberPlan.ToolType, TOOL_TYPE::Unknown, TOOL_TYPE::toToolType, "faber.Tool type");

		// Get NB item built (for ammo)
		TRANSLATE_VAL( FaberPlan.NbItemBuilt, "faber.Create.Nb built items");

		// MPs. Try all MP1 .. 5 slots. Stop when not valid
		for (uint k=0; k< MAX_FABER_REQ_MP; ++k)
		{
			sint32	mpQuantity= 0;
			root.getValueByName(mpQuantity, toString("faber.Create.Quantity %d", k+1).c_str() );
			// if the req quantity is not 0
			if ( mpQuantity>0 )
			{
				CFaberPlan::CItemPartMP	mpVal;
				mpVal.Quantity= mpQuantity;
				// No error if unknown: filter not used (all MPs match)
				TRANSLATE_ENUM_NODB( mpVal.FaberTypeFilter, RM_FABER_TYPE::Unknown, RM_FABER_TYPE::toFaberType,
					toString("faber.Create.MP %d", k+1).c_str() );

				// Add this req MP.
				FaberPlan.ItemPartMps.push_back(mpVal);
			}
			// else subsequents MP slots are not used
			else
				break;
		}

		// Formula MPs. Try all MP1 .. 5 slots. Stop when not valid
		for (uint k=0; k< MAX_FABER_REQ_MP; ++k)
		{
			sint32	mpQuantity= 0;
			root.getValueByName(mpQuantity, toString("faber.Create.Quantity formula %d", k+1).c_str() );
			// if the req quantity is not 0
			if ( mpQuantity>0 )
			{
				CFaberPlan::CFormulaMP	mpVal;
				mpVal.Quantity= mpQuantity;
				// No error if unknown: filter not used (all MPs match)
				TRANSLATE_VAL(val, toString("faber.Create.MP formula %d", k+1).c_str());
				mpVal.ItemRequired.buildSheetId(val);

				// Add this req MP.
				FaberPlan.FormulaMps.push_back(mpVal);
			}
			// else subsequents MP slots are not used
			else
				break;
		}
	}


	// read minmax range/cast time when guigui ready
	TRANSLATE_VAL(MinCastTime, "Basics.MinCastTime");
	TRANSLATE_VAL(MaxCastTime, "Basics.MaxCastTime");
	TRANSLATE_VAL(MinRange, "Basics.MinRange");
	TRANSLATE_VAL(MaxRange, "Basics.MaxRange");

	// Read SPCost
	TRANSLATE_VAL(SPCost, "Basics.SPCost");

	// Read AvoidCyclic
	TRANSLATE_VAL(AvoidCyclic, "Basics.AvoidCyclic");

	// Read UsableWithEmptyHands
	TRANSLATE_VAL(UsableWithEmptyHands, "Basics.UsableWithEmptyHands");

	// Read Action Nature
	string	val;
	TRANSLATE_ENUM(ActionNature, ACTNATURE::UNKNOWN, ACTNATURE::toActionNature, "Basics.Action Nature");

	// Read CivRestriction
	TRANSLATE_ENUM(CivRestriction, EGSPD::CPeople::Common, EGSPD::CPeople::fromString, "Basics.CivRestriction");


	// **** parse properties, to precompute the Bricks Flags.
	BrickRequiredFlags= 0;
	for(i=0;i<Properties.size();i++)
	{
		string text= NLMISC::toLower(Properties[i].Text);

		// If the property is an opening property
		const	string	openingProp[]= { "opening_1:", "opening_2:", "opening_3:" };
		// or if the property is a general brick flag
		const	string	neededBrickFlag= "needed_brick_flag";
		const	uint	nOpeningProp= sizeof(openingProp) / sizeof(openingProp[0]);
		for(uint j=0;j<nOpeningProp;j++)
		{
			const string &prop= openingProp[j];
			// if found this property
			if( text.compare(0, prop.size(), prop)==0 ||
				(j==0 && text.compare(0, neededBrickFlag.size(), neededBrickFlag)==0)
			  )
			{
				// get all the opening requirement
				vector<string>	strList;
				strList.reserve(10);
				splitString(text, ":", strList);
				for(uint k=1;k<strList.size();k++)
				{
					// remove empty space, and convert to a bit
					strRemoveChar(strList[k], ' ');
					BRICK_FLAGS::TBrickFlag	evFlag;
					evFlag= BRICK_FLAGS::toBrickFlag(strList[k]);
					if(evFlag!=BRICK_FLAGS::UnknownFlag)
						BrickRequiredFlags|= (uint64(1)<<(uint)evFlag);
				}
				break;
			}
		}
	}

	// **** parse required skills
	// parse the sheet str
	string	skillReqStr;
	TRANSLATE_VAL(skillReqStr, "Basics.LearnRequiresOneOfSkills");
	while(strFindReplace(skillReqStr, ":", " "));
	listSkill.clear();
	splitString(skillReqStr," ",listSkill);
	// build the req skill array
	RequiredSkills.clear();
	RequiredSkills.reserve(listSkill.size()/2);
	for(i=0;i<listSkill.size()/2;i++)
	{
		CSkillValue		sv;
		sv.Skill= SKILLS::toSkill(listSkill[i*2]);
		fromString(listSkill[i*2+1], sv.Value);
		// keep only whats work
		if(sv.Skill!=SKILLS::unknown)
		{
			RequiredSkills.push_back(sv);
		}
	}

	// **** parse required bricks
	// parse the sheet str
	string	brickReqStr;
	TRANSLATE_VAL(brickReqStr, "Basics.LearnRequiresBricks");
	while(strFindReplace(brickReqStr, ":", " "));
	std::vector<string>		listBrick;
	listBrick.clear();
	splitString(brickReqStr," ",listBrick);
	// build the req skill array
	RequiredBricks.clear();
	RequiredBricks.reserve(listBrick.size());
	for(i=0;i<listBrick.size();i++)
	{
		CSheetId	sheetId;
		string	str= listBrick[i];
		strlwr(str);
		if(str.find(".sbrick")==string::npos)
			str+= ".sbrick";
		sheetId.buildSheetId(str);
		if(sheetId!=CSheetId::Unknown)
		{
			RequiredBricks.push_back(sheetId);
		}
	}

	// faction index
	string faction;
	root.getValueByName (faction, "Basics.Faction" );
	FactionIndex = CStaticFames::getInstance().getFactionIndex( faction );

	// min fame value
	TRANSLATE_VAL(MinFameValue, "Basics.Minimum fame");

	// **** Magic only: try to get a ResistType against this brick
	for(i=0;i<Properties.size();i++)
	{
		string text= toLower(Properties[i].Text);

		// *** If the property is a DamageType
		const string	dmgTypeProp= "ma_dmg_type:";
		if( text.compare(0, dmgTypeProp.size(), dmgTypeProp)==0 )
		{
			// extract the dmg type
			string	dtStr= text.substr(dmgTypeProp.size());
			strRemoveChar(dtStr, ' ');
			DMGTYPE::EDamageType	dt= DMGTYPE::stringToDamageType(dtStr);
			if(dt!=DMGTYPE::UNDEFINED)
			{
				// Convert to a resist type
				RESISTANCE_TYPE::TResistanceType	rt= DMGTYPE::getAssociatedResistanceType(dt);
				if(rt!=RESISTANCE_TYPE::None)
					MagicResistType= rt;
			}
		}

		// *** Do the same if the property is an effect family (affliction spells)
		const string	effectFamProp= "ma_effect:";
		if( text.compare(0, effectFamProp.size(), effectFamProp)==0 )
		{
			// extract the effect family
			string	efStr= text.substr(effectFamProp.size());
			strRemoveChar(efStr, ' ');
			EFFECT_FAMILIES::TEffectFamily	ef= EFFECT_FAMILIES::toEffectFamily(efStr);
			if(ef!=EFFECT_FAMILIES::Unknown)
			{
				// Convert to a resist type
				RESISTANCE_TYPE::TResistanceType	rt= EFFECT_FAMILIES::getAssociatedResistanceType(ef);
				if(rt!=RESISTANCE_TYPE::None)
					MagicResistType= rt;
			}
		}

	}

}


// ***************************************************************************
bool							CSBrickSheet::isRoot() const
{
	return BRICK_FAMILIES::isRootFamily(BrickFamily);
}

// ***************************************************************************
bool							CSBrickSheet::isCredit() const
{
	return BRICK_FAMILIES::isCreditFamily(BrickFamily);
}

// ***************************************************************************
bool							CSBrickSheet::isMandatory() const
{
	return BRICK_FAMILIES::isMandatoryFamily(BrickFamily);
}

// ***************************************************************************
bool							CSBrickSheet::isOptional() const
{
	return BRICK_FAMILIES::isOptionFamily(BrickFamily);
}

// ***************************************************************************
bool							CSBrickSheet::isParameter() const
{
	return BRICK_FAMILIES::isParameterFamily(BrickFamily);
}

// ***************************************************************************
bool							CSBrickSheet::isCombat() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::COMBAT;
}

// ***************************************************************************
bool							CSBrickSheet::isMagic() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::MAGIC;
}

// ***************************************************************************
bool							CSBrickSheet::isFaber() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::FABER;
}

// ***************************************************************************
bool							CSBrickSheet::isHarvest() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::HARVEST;
}

// ***************************************************************************
bool							CSBrickSheet::isForageProspection() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::FORAGE_PROSPECTION;
}

// ***************************************************************************
bool							CSBrickSheet::isForageExtraction() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::FORAGE_EXTRACTION;
}

// ***************************************************************************
bool							CSBrickSheet::isSpecialPower() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::SPECIAL_POWER;
}

// ***************************************************************************
bool							CSBrickSheet::isProcEnchantment() const
{
	return BRICK_FAMILIES::brickType(BrickFamily) == BRICK_TYPE::PROC_ENCHANTEMENT;
}

// ***************************************************************************
bool							CSBrickSheet::mustDisplayLevel() const
{
	// always but if root or mandatory, special interface, or if level is 0
	// NB: Yoyo Hack. special interface with indexInFamily==63 means "want to display the level"
	return !(	isMandatory() ||
				isRoot() ||
				(BrickFamily>= BRICK_FAMILIES::BeginInterface && BrickFamily<= BRICK_FAMILIES::EndInterface && IndexInFamily!=63) ||
				Level==0 );
}
