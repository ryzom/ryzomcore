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





//////////////
//	INCLUDE	//
//////////////
#include "stdpch.h"
#include "egs_static_brick.h"
#include "egs_sheets.h"
#include "game_share/fame.h"
// nel georges
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
// ai share
#include "ai_share/ai_share.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;
using namespace AI_SHARE;

//////////////
//	GLOBALS	//
//////////////
const uint	SBRICK_MAX_MANDATORY	= 12;
const uint	SBRICK_MAX_OPTIONAL		= 32;
const uint	SBRICK_MAX_PARAMETER	= 4;
const uint	SBRICK_MAX_CREDIT		= 12;


map< pair<uint16, uint16>, CSheetId > CStaticBrick::_Bricks;

NL_INSTANCE_COUNTER_IMPL(CFaber);

//--------------------------------------------------------------
//						constructor  
//--------------------------------------------------------------
CStaticBrick::CStaticBrick()
{
	ForcedLocalisation = SLOT_EQUIPMENT::UNDEFINED;
	Family = BRICK_FAMILIES::Unknown;
	Nature = ACTNATURE::UNKNOWN;
	IndexInFamily = 0;
	SabrinaValue = 0;
	SabrinaRelativeValue = 0.0f;
	PowerValue = 0;
	SkillPointPrice = 0;
	Faber = 0;
	MinCastTime = 10;	// Default min : 1 sec
	MaxCastTime = 100;	// Default max : 10sec
	MinRange    = 1;	// Default min : 1  meter
	MaxRange    = 100;	// Default max : 100meters
	TargetRestriction	= TARGET::EveryBody;
	ToolType = TOOL_TYPE::Unknown;
	CraftingDuration = 2.f;
	MinFameValue = (sint32)0x80000000;
	UsableWithEmptyHands = true;
} // constructor //


//--------------------------------------------------------------
//						destructor  
//--------------------------------------------------------------
CStaticBrick::~CStaticBrick()
{
	Params.clear();
} // destructor //


//--------------------------------------------------------------
//					serial()  
//--------------------------------------------------------------
void CStaticBrick::serial(class NLMISC::IStream &f)
{
	f.serial( Name );
	f.serial( SheetId );
	f.serialEnum(Nature);
	f.serial(SabrinaValue);
	f.serial(SabrinaRelativeValue);
	f.serial(SkillPointPrice);
	f.serial(PowerValue);
	f.serial(IndexInFamily);
//	f.serial(RangeTable);
	f.serial(MinCastTime);
	f.serial(MaxCastTime);
	f.serial(MinRange);
	f.serial(MaxRange);	
	f.serialCont( StringParams );
	f.serial(ForbiddenDef);
	f.serial(ForbiddenExclude);

	f.serialCont( LearnRequiresOneOfSkills );
	f.serialCont( LearnRequiresBricks );
	f.serialEnum( CivRestriction );
	f.serial( Faction );
	f.serial( MinFameValue );
	f.serial( UsableWithEmptyHands );

	if (f.isReading() )
	{
		uint16 size;
		string str;
	// family
		f.serial( str );
		Family = BRICK_FAMILIES::toSBrickFamily(str);

	// localisation
		f.serial( str );
		ForcedLocalisation = SLOT_EQUIPMENT::stringToSlotEquipment(str);

	// Params
		Params.clear();
		size = (uint16)StringParams.size();
		for (uint i = 0 ; i < size ; ++i)
		{
			addParam(StringParams[i], Params);
		}

	// Grammar		
		// mandatory
		f.serial(size);
		for ( uint16 i = 0 ; i < size ; ++i)
		{			
			f.serial(str);

			BRICK_FAMILIES::TBrickFamily family = BRICK_FAMILIES::toSBrickFamily(str);
			if (family == BRICK_FAMILIES::Unknown)
			{
				nlwarning("Cannot find family associated to string %s", str.c_str() );
			}
			else
				MandatoryFamilies.insert( family );
		}

		// optional
		f.serial(size);
		for ( uint16 i = 0 ; i < size ; ++i)
		{			
			f.serial(str);

			BRICK_FAMILIES::TBrickFamily family = BRICK_FAMILIES::toSBrickFamily(str);
			if (family == BRICK_FAMILIES::Unknown)
			{
				nlwarning("Cannot find family associated to string %s", str.c_str() );
			}
			else
				OptionalFamilies.insert( family );
		}

		// credit
		f.serial(size);
		for ( uint16 i = 0 ; i < size ; ++i)
		{			
			f.serial(str);

			BRICK_FAMILIES::TBrickFamily family = BRICK_FAMILIES::toSBrickFamily(str);
			if (family == BRICK_FAMILIES::Unknown)
			{
				nlwarning("Cannot find family associated to string %s", str.c_str() );
			}
			else
				CreditFamilies.insert( family );
		}

		// if bricks is faber mandatory
		if( Family >= BRICK_FAMILIES::BeginFaberMandatory && Family <= BRICK_FAMILIES::EndFaberMandatory )
		{
			Faber = new CFaber();
			Faber->serial(f);
		}

		// Insert brick (ONLY for player bricks, do not add .saibrick used by AI)
		if (SheetId.toString().find(".saibrick") == string::npos)
		{
			std::map< std::pair<uint16, uint16>, CSheetId >::const_iterator itFound = _Bricks.find( std::make_pair( Family,IndexInFamily) );
			if ( itFound != _Bricks.end() )
			{
				nlwarning("ALREADY ADDED brick %s, familyId = %u, index = %u in sheet %s",SheetId.toString().c_str(), Family, IndexInFamily, (*itFound).second.toString().c_str() );
			}
			else
			{
				pair< map< std::pair<uint16, uint16>, CSheetId >::iterator, bool > ret = _Bricks.insert( make_pair( make_pair(Family,IndexInFamily), SheetId) );
				if (!ret.second == true)
					nlwarning("FAILED TO INSERT brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), Family, IndexInFamily);
			}
		}

		// skills
		f.serial(size);
		for (uint i = 0 ; i < size ; ++i )
		{
			f.serial(str);
			SKILLS::ESkills skill = SKILLS::toSkill(str);
			if (skill != SKILLS::unknown)
				Skills.push_back(skill);
		}
	}
	else
	{		
	// family
		string str = BRICK_FAMILIES::toString(Family);
		f.serial( str );

	// localisation
		str = SLOT_EQUIPMENT::toString(ForcedLocalisation);
		f.serial( str );
		
	// params
		// nothing to do -> keep params and only serialize the param as strings

	// grammar
		// mandatory
		set<BRICK_FAMILIES::TBrickFamily>::const_iterator it = MandatoryFamilies.begin();
		set<BRICK_FAMILIES::TBrickFamily>::const_iterator itEnd = MandatoryFamilies.end();
		uint16 size = (uint16)MandatoryFamilies.size();
		f.serial(size);
		for ( ; it != itEnd ; ++it)
		{
			string str = BRICK_FAMILIES::toString(*it);
			f.serial(str);
		}

		// optional
		it = OptionalFamilies.begin();
		itEnd = OptionalFamilies.end();
		size = (uint16)OptionalFamilies.size();

		f.serial(size);
		for ( ; it != itEnd ; ++it)
		{
			string str = BRICK_FAMILIES::toString(*it);
			f.serial(str);
		}

		// credit
		it = CreditFamilies.begin();
		itEnd = CreditFamilies.end();
		size = (uint16)CreditFamilies.size();

		f.serial(size);
		for ( ; it != itEnd ; ++it)
		{
			string str = BRICK_FAMILIES::toString(*it);
			f.serial(str);
		}

		// if bricks is faber mandatory
		if( Faber != 0 )
		{
			Faber->serial(f);
		}

		// skills
		size = (uint16)Skills.size();
		f.serial(size);
		for (uint i = 0 ; i < size ; ++i )
		{
			string str = SKILLS::toString(Skills[i]);
			f.serial(str);
		}
	}
	// Serialize Target Type.
	f.serialEnum(TargetRestriction);

	// Serialize crafting tool type
	f.serialEnum( ToolType );

	// Serialize crafting duration
	f.serial( CraftingDuration );
} // serial //


//--------------------------------------------------------------
//					readGeorges()  
//--------------------------------------------------------------
void CStaticBrick::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	SheetId = sheetId;

	// Get the root node, always exist
    UFormElm &root = form->getRootNode ();

	readStaticBrick( root, sheetId );
}

// read static bricks from gived root
void CStaticBrick::readStaticBrick( const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId)
{
	string value;
	const sint NB_LETTERS_AFTER_FAMILY = 2;

	// brick name
	Name = sheetId.toString();

	bool isAIBrick = false;
	if (sheetId.toString().find(".saibrick") != string::npos)
	{
		isAIBrick = true;
	}

	/// TODO : convert parameters strings in lowercase when reading

	// FamilyId
	//if( root.getValueByName (value, "Basics.FamilyId") )
	//{
	//Family = BRICK_FAMILIES::toSBrickFamily( sheetName.substr( 0, sheetName.size() - NB_LETTERS_AFTER_FAMILY ) );	
	//}
	if( root.getValueByName (value, "Basics.FamilyId") )
	{
		Family = BRICK_FAMILIES::toSBrickFamily( value );	
		if(Family==BRICK_FAMILIES::Unknown)
			nlwarning("CStaticBrick::readStaticBrick: Unknown Family %s in sheet %s", value.c_str(), sheetId.toString().c_str());
/*
		// Yoyo: patch to read auto generated bricks
		if(Family==BRICK_FAMILIES::Unknown)
		{
			string	sheetName= sheetId.toString();
			string::size_type	end= sheetName.find(".sbrick")-NB_LETTERS_AFTER_FAMILY;
			string sub = sheetName.substr(0,end);
			sub = strupr(sub);
			Family = BRICK_FAMILIES::toSBrickFamily ( sub );
		}
*/
	}
	else
	{		
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'FamilyId' for sheet %s", sheetId.toString().c_str() );
	}

	// nature
	if( root.getValueByName (value, "Basics.Action Nature") )
	{
		Nature = ACTNATURE::toActionNature(value);
	}

	// skill
	if( root.getValueByName (value, "Basics.Skill") )
	{
		vector<string> skills;
		explode( value, string(":"), skills, true );
		vector<string>::const_iterator isv;
		for ( isv=skills.begin(); isv!=skills.end(); ++isv )
		{
			const string& sav = *isv;
			SKILLS::ESkills skill = SKILLS::toSkill(sav);
			if ( skill != SKILLS::unknown ) // discard the skill if unknown
				Skills.push_back( skill );
//			else
//				nlwarning( "<CStaticBrick::readGeorges> Invalid Skill value '%s' in sheet %s", sav.c_str(), sheetId.toString().c_str() );
		}
	}
	else
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'Skill' for sheet %s", sheetId.toString().c_str() );
	}
	

	// SabrinaValue
	root.getValueByName (SabrinaValue, "Basics.SabrinaCost");

	// Sabrina relative Value
	root.getValueByName(SabrinaRelativeValue, "Basics.SabrinaRelativeValue");

	// Skill point price
	root.getValueByName (SkillPointPrice, "Basics.SPCost");

	// power value
	root.getValueByName (PowerValue, "Basics.PowerValue");
	
	// IndexInFamily
	if( ! root.getValueByName (IndexInFamily, "Basics.IndexInFamily") )	
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'IndexInFamily' for sheet %s", sheetId.toString().c_str() );
	}

	// forced localisation
	if( root.getValueByName (value, "Basics.ForcedLocalisation") )
	{
		ForcedLocalisation = SLOT_EQUIPMENT::stringToSlotEquipment(value);
	}
	else
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'ForcedLocalisation' for sheet %s", sheetId.toString().c_str() );
	}
	
	// LearnRequiresOneOfSkills
	if ( ! root.getValueByName (value, "Basics.LearnRequiresOneOfSkills") )
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'LearnRequireOneOfSkills' for sheet %s", sheetId.toString().c_str() );
	}
	vector<string> skillsAndValues;
	explode( value, string(":"), skillsAndValues, true );
	vector<string>::const_iterator isv;
	for ( isv=skillsAndValues.begin(); isv!=skillsAndValues.end(); ++isv )
	{
		const string& sav = *isv;
		CPlayerSkill ps;
		if ( ps.initFromString( sav ) ) // discard the skill if unknown
			LearnRequiresOneOfSkills.push_back( ps );
		else
			nlwarning( "<CStaticBrick::readGeorges> Invalid LearnRequiresOneOfSkills value '%s' in sheet %s", sav.c_str(), sheetId.toString().c_str() );
	}

	// LearnRequiresBricks
	if ( ! root.getValueByName (value, "Basics.LearnRequiresBricks") )
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'LearnRequireBricks' for sheet %s", sheetId.toString().c_str() );
	}
	vector<string> requiredBricks;
	explode( value, string(":"), requiredBricks, true );
	vector<string>::const_iterator irb;
	for ( irb=requiredBricks.begin(); irb!=requiredBricks.end(); ++irb )
	{
		string brickSheet = *irb;
		if ( isAIBrick )
		{
			if (brickSheet.find( ".saibrick" ) == string::npos)
			{
				brickSheet += string(".saibrick");
			}
		}
		else
		{
			if(brickSheet.find( ".sbrick" ) == string::npos)
			{
				brickSheet += string(".sbrick");
			}
		}
		
		CSheetId brickSheetId( NLMISC::strlwr( brickSheet ) );
		if ( brickSheetId == CSheetId::Unknown )
			nlwarning( "<CStaticBrick::readGeorges> LearnRequireBricks: unknown sheet %s in %s", brickSheet.c_str(), sheetId.toString().c_str() );
		else
			LearnRequiresBricks.push_back( brickSheetId );
	}

	// CivRestriction
	if ( ! root.getValueByName (value, "Basics.CivRestriction") )
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'CivRestriction' for sheet %s", sheetId.toString().c_str() );
		CivRestriction = EGSPD::CPeople::Common;
	}
	CivRestriction = EGSPD::CPeople::fromString( value );

	// Insert brick (ONLY for player bricks, do not add .saibrick used by AI)
	if (!isAIBrick)
	{
		std::map< std::pair<uint16, uint16>, CSheetId >::const_iterator itFound = _Bricks.find( std::make_pair( Family,IndexInFamily) );
		if ( itFound != _Bricks.end() )
		{
			nlwarning("ALREADY ADDED brick %s, familyId = %u, index = %u in sheet %s",SheetId.toString().c_str(), Family, IndexInFamily, (*itFound).second.toString().c_str() );
		}
		else
		{
			pair< map< std::pair<uint16, uint16>, CSheetId >::iterator, bool > ret = _Bricks.insert( make_pair( make_pair(Family,IndexInFamily), SheetId) );
			if (!ret.second == true)
				nlwarning("FAILED TO INSERT brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), Family, IndexInFamily);
		}
	}

	// read the params
	//Params
	StringParams.clear();
	for (uint i=0;i<20;++i)
	{
		string param;
		std::string s=NLMISC::toString("Basics.Property %i",i);
		if ( root.getValueByName (param, s.c_str()) && !param.empty() )
			StringParams.push_back(param);
	}
	// Parse Params
	Params.clear();
	const uint size = (uint)StringParams.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		addParam(StringParams[i], Params);
	}
	
	// Load Faber tools
	if( Family >= BRICK_FAMILIES::BeginFaberRoot && Family <= BRICK_FAMILIES::EndFaberRoot )
	{
		if ( root.getValueByName(value , "faber.Tool type" ) ) ToolType = TOOL_TYPE::toToolType( value );
	}

	// crafting duration
	if ( !root.getValueByName(CraftingDuration , "faber.Crafting Duration" ) ) 
	{
		nlwarning("<CStaticBrick::readGeorges> can't get the value 'faber.Crafting Duration' for sheet %s", sheetId.toString().c_str() );
	}

	// Load faber plan if brick is faber mandatory
	if( Family >= BRICK_FAMILIES::BeginFaberMandatory && Family <= BRICK_FAMILIES::EndFaberMandatory )
	{
		loadFaber( root, sheetId );
	}

	// Min Max Range/CastTime
	root.getValueByName (MinCastTime, "Basics.MinCastTime");
	root.getValueByName (MaxCastTime, "Basics.MaxCastTime");
	root.getValueByName (MinRange, "Basics.MinRange");
	root.getValueByName (MaxRange, "Basics.MaxRange");
	// Get the target type.
	std::string targetRestriction;
	if(root.getValueByName(targetRestriction, "Basics.TargetRestriction"))
		TargetRestriction = TARGET::stringToTargetRestriction(targetRestriction);
	else
		nlwarning("CStaticBrick:readStaticBrick: cannot get the value from the key 'Basics.TargetRestriction'.");
	TargetRestriction = TARGET::stringToTargetRestriction(targetRestriction);

	root.getValueByName(Faction, "Basics.Faction");
	root.getValueByName(MinFameValue, "Basics.Minimum fame");

	// mandatory families
	MandatoryFamilies.clear();
	char	tmp[256];
	for( uint i = 0 ; i < SBRICK_MAX_MANDATORY ; ++i )
	{
		sprintf(tmp, "Mandatory.f%d", i);
		root.getValueByName (value, tmp);
		if (!value.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(value);
			if(bf != BRICK_FAMILIES::Unknown)
				MandatoryFamilies.insert( bf );
			else
				nlwarning("Unknown Mandatory family %s",value.c_str());
		}
	}
	
	// Optional families
	OptionalFamilies.clear();
	for( uint i = 0 ; i < SBRICK_MAX_OPTIONAL ; ++i )
	{
		sprintf(tmp, "Optional.f%d", i);
		root.getValueByName (value, tmp);
		if (!value.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(value);
			if(bf != BRICK_FAMILIES::Unknown)
				OptionalFamilies.insert( bf );
			else
				nlwarning("Unknown optional family %s",value.c_str());
		}
	}
	
	// Parameter families
//	ParameterFamilies.clear();
	for( uint i = 0 ; i < SBRICK_MAX_PARAMETER ; ++i )
	{
		sprintf(tmp, "Parameter.f%d", i);
		root.getValueByName (value, tmp);
		if (!value.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(value);
			if(bf != BRICK_FAMILIES::Unknown)
				MandatoryFamilies.insert( bf );
			else
				nlwarning("Unknown Parameter family %s",value.c_str());
		}
	}
	
	// Credit families
	CreditFamilies.clear();
	for( uint i = 0 ; i < SBRICK_MAX_CREDIT ; ++i )
	{
		sprintf(tmp, "Credit.f%d", i);
		root.getValueByName (value, tmp);
		if (!value.empty())
		{
			BRICK_FAMILIES::TBrickFamily	bf= BRICK_FAMILIES::toSBrickFamily(value);
			if(bf != BRICK_FAMILIES::Unknown)
				CreditFamilies.insert( bf );
		}
	}
	
	root.getValueByName (ForbiddenDef, "Basics.ForbiddenDef" );
	root.getValueByName (ForbiddenExclude, "Basics.ForbiddenExclude" );

	root.getValueByName (UsableWithEmptyHands, "Basics.UsableWithEmptyHands");
	
} // readStaticBrick //


/*
 * Init from a string such as "SFM 50". If skill unknown, return false.
 */
bool	CPlayerSkill::initFromString( const string& skillAndValue )
{
	string::size_type p = skillAndValue.find( ' ' );
	if ( (p == string::npos) || (skillAndValue.size() == p+1) )
		return false;
	else
	{
		string codeS = skillAndValue.substr( 0, p );
		if ( (! codeS.empty()) && (codeS[0] != 'S') )
		{
			nlwarning( "Skill code missing S before %s (added)", codeS.c_str() );
			codeS = "S" + codeS;
		}
		Code = SKILLS::toSkill( codeS );
		NLMISC::fromString( skillAndValue.substr( p+1 ), Value );
		return (Code != SKILLS::unknown);
	}
}


//
// When generating egs_static_brick.cpp.h from this method, the
// TBrickParam::TValueType identifier is used as the identifier
// string if the brick properties.
//
//--------------------------------------------------------------
//					addParam()  
//--------------------------------------------------------------
void addParam(const std::string &paramStr, std::vector<TBrickParam::IIdPtr> &Params)
{
	std::string keyword;
	std::string tail;
	stringToKeywordAndTail(paramStr,keyword,tail);

	// $*ENUM TBrickParam
	switch(TBrickParam(keyword))
	{
	/************************************************************************/
	/* General Params														*/
	/************************************************************************/
	case TBrickParam::SAP:
		// $*STRUCT CSBrickParamSap TBrickParam::SAP
		// $*-i unsigned Sap=0		// quantity of SAP to use
		Params.push_back(new CSBrickParamSap(tail)); 
		break;

	case TBrickParam::HP:
		// $*STRUCT CSBrickParamHp TBrickParam::HP
		// $*-i unsigned Hp=0		// quantity of HP to use
		Params.push_back(new CSBrickParamHp(tail)); 
		break;

	case TBrickParam::STA:
		// $*STRUCT CSBrickParamSta TBrickParam::STA
		// $*-i unsigned Sta=0		// quantity of STA to use
		Params.push_back(new CSBrickParamSta(tail)); 
		break;

	case TBrickParam::STA_WEIGHT_FACTOR:
		// $*STRUCT CSBrickParamStaWeightFactor TBrickParam::STA_WEIGHT_FACTOR
		// $*-i float StaFactor=0		// quantity of STA FACTOR of WEIGHT to use
		// $*-i unsigned StaConst=0		// quantity of STA Constante to use, used STA = StaFactor * (weight of equipped weapons) + StaConst
		Params.push_back(new CSBrickParamStaWeightFactor(tail)); 
		break;

	case TBrickParam::FOCUS:
		// $*STRUCT CSBrickParamFocus TBrickParam::FOCUS
		// $*-i unsigned Focus=0		// quantity of FOCUS to use
		Params.push_back(new CSBrickParamFocus(tail)); 
		break;

	case TBrickParam::SET_BEHAVIOUR:
		// $*STRUCT CSBrickParamSetBehaviour TBrickParam::SET_BEHAVIOUR
		// $*-s std::string Behaviour	// the new behaviour to use
		Params.push_back(new CSBrickParamSetBehaviour(tail)); 
		break;

	case TBrickParam::DEFINE_FLAG:
		// $*STRUCT CSBrickParamDefineFlag TBrickParam::DEFINE_FLAG
		// $*-s std::string Flag	// the defined flag
		Params.push_back(new CSBrickParamDefineFlag(tail)); 
		break;

	case TBrickParam::BYPASS_CHECK:
		// $*STRUCT CSBrickParamBypassCheck TBrickParam::BYPASS_CHECK
		// $*-s std::string FlagType	// the check flag to bypass
		Params.push_back(new CSBrickParamBypassCheck(tail)); 
		break;
		
	/************************************************************************/
	/* COMBAT Params														*/
	/************************************************************************/
	case TBrickParam::LATENCY_FACTOR:
		{
		// $*STRUCT CSBrickParamLatencyFactor TBrickParam::LATENCY_FACTOR
		// $*-f float MinLatencyFactor = 1	// min factor on weapon latency
		// $*-f float MaxLatencyFactor = 0.5	// max factor on weapon latency
		Params.push_back(new CSBrickParamLatencyFactor(tail)); 
		}
		break;

	case TBrickParam::STA_LOSS_FACTOR:
		{
		// $*STRUCT CSBrickParamStaLossFactor TBrickParam::STA_LOSS_FACTOR
		// $*-f float MinFactor = 0.0	// min factor of damage also applied to stamina
		// $*-f float MaxFactor = 0.0	// max factor of damage also applied to stamina
		Params.push_back(new CSBrickParamStaLossFactor(tail));
		}
		break;

	case TBrickParam::DEBUFF_REGEN:
		{
		// $*STRUCT CSBrickParamDebuffRegen TBrickParam::DEBUFF_REGEN
		// $*-s std::string Score // affected score regen (Sap, Stamina, HitPoints, Focus)
		// $*-f float Duration = 0.0	// duration in seconds
		// $*-f float MinFactor = 0.0	// min factor of regen debuff
		// $*-f float MaxFactor = 0.0	// max factor of regen debuff
		Params.push_back(new CSBrickParamDebuffRegen(tail)); 
		}
		break;
		
	case TBrickParam::SAP_LOSS_FACTOR:
		{
		// $*STRUCT CSBrickParamSapLossFactor TBrickParam::SAP_LOSS_FACTOR
		// $*-f float MinFactor = 0.0	// min factor of damage also applied to sap
		// $*-f float MaxFactor = 0.0	// max factor of damage also applied to sap
		Params.push_back(new CSBrickParamSapLossFactor(tail)); 
		}
		break;

	case TBrickParam::AIM:
		{
		// $*STRUCT CSBrickParamAim TBrickParam::AIM
		// $*-s std::string BodyType // Homin, kitin (=land kitin), bird, flying_kitin....(see body.h)
		// $*-s std::string AimedSlot // head, body, arms... (see slot_equipment.cpp)
		Params.push_back(new CSBrickParamAim(tail)); 
		}
		break;


	case TBrickParam::ATT_SKILL_MOD:
		{
		// $*STRUCT CSBrickParamAttackSkillModifier TBrickParam::ATT_SKILL_MOD
		// $*-i sint32 MinModifier = 0	// min modifier on attacker skills to hit its target
		// $*-i sint32 MaxModifier = 20	// max modifier on attacker skills to hit its target
		Params.push_back(new CSBrickParamAttackSkillModifier(tail));
		}
		break;

	case TBrickParam::DEFENSE_MOD:
		{
		// $*STRUCT CSBrickParamDefenseModifier TBrickParam::DEFENSE_MOD
		// $*-i sint32	MinModifier = 0	// min modifier on attacker defense skills during the attack
		// $*-i sint32	MaxModifier = -20	// max modifier on attacker defense skills during the attack
		Params.push_back(new CSBrickParamDefenseModifier(tail));
		}
		break;

	case TBrickParam::THROW_OFF_BALANCE:
		{
			// $*STRUCT CSBrickParamThrowOffBalance TBrickParam::THROW_OFF_BALANCE
			// $*-f float	MinDuration = 0.0f	// effect min duration
			// $*-f float	MaxDuration = 5.0f	// effect max duration 
			Params.push_back(new CSBrickParamThrowOffBalance(tail));
		}
		break;

	case TBrickParam::INC_DMG:
		{
		// $*STRUCT CSBrickParamIncreaseDamage TBrickParam::INC_DMG
		// $*-f float	MinFactor = 1.0f //min factor on damage
		// $*-f float	MaxFactor = 2.0f //max factor on damage
		Params.push_back(new CSBrickParamIncreaseDamage(tail)); 
		}
		break;

	case TBrickParam::INC_DMG_TYPE_RSTR:
		{
		// $*STRUCT CSBrickParamIncDmgTypeRestriction TBrickParam::INC_DMG_TYPE_RSTR
		// $*-s std::string	TypeRestriction //type restriction
		// $*-f float		FactorModifier = 0.0f //bonus on damage factor
		Params.push_back(new CSBrickParamIncDmgTypeRestriction(tail)); 
		}
		break;

	case TBrickParam::INC_DMG_RACE_RSTR:
		{
		// $*STRUCT CSBrickParamIncDmgRaceRestriction TBrickParam::INC_DMG_RACE_RSTR
		// $*-s std::string	RaceRestriction //race restriction
		// $*-f float		FactorModifier = 0.0f //bonus on damage factor
		Params.push_back(new CSBrickParamIncDmgRaceRestriction(tail)); 
		}
		break;

	case TBrickParam::INC_DMG_ECOS_RSTR:
		{
		// $*STRUCT CSBrickParamIncDmgEcosystemRestriction TBrickParam::INC_DMG_ECOS_RSTR
		// $*-s std::string	EcosystemRestriction //Ecosystem restriction
		// $*-f float		FactorModifier = 0.0f //bonus on damage factor
		Params.push_back(new CSBrickParamIncDmgEcosystemRestriction(tail)); 
		}
		break;

	case TBrickParam::INC_DMG_SEASON_RSTR:
		{
		// $*STRUCT CSBrickParamIncDmgSeasonRestriction TBrickParam::INC_DMG_SEASON_RSTR
		// $*-s std::string	SeasonRestriction //Season restriction
		// $*-f float		FactorModifier = 0.0f //bonus on damage factor
		Params.push_back(new CSBrickParamIncDmgSeasonRestriction(tail)); 
		}
		break;

	case TBrickParam::SPECIAL_DAMAGE:
		{
			// $*STRUCT CSBrickParamSpecialDamage TBrickParam::SPECIAL_DAMAGE
			// $*-s std::string	DamageType // damage type
			// $*-f float		MinFactor = 0.0f //min factor of damage
			// $*-f float		MaxFactor = 1.0f //max factor of damage
			Params.push_back(new CSBrickParamSpecialDamage(tail)); 
		}
		break;

	case TBrickParam::ARMOR_MOD:
		{
		// $*STRUCT CSBrickParamArmorMod TBrickParam::ARMOR_MOD
		// $*-s std::string	ArmorType // affected armor type (light, medium, heavy, kitin etc..)
		// $*-f float		MinFactor = 1.0f //max factor applied on armor absorption
		// $*-f float		MaxFactor = 0.5f //max factor applied on armor absorption (< min as smaller is better)
		Params.push_back(new CSBrickParamArmorMod(tail)); 
		}
		break;


	case TBrickParam::SLOW_CAST:
		{
		// $*STRUCT CSBrickParamSlowCast TBrickParam::SLOW_CAST
		// $*-f float Duration	// duration of the slow in seconds
		// $*-f float MinFactor = 1.0f // min factor applied on cast time
		// $*-f float MaxFactor = 2.0f // max factor applied on cast time
		Params.push_back(new CSBrickParamSlowCast(tail)); 
		}
		break;
		

	case TBrickParam::OPENING_1:
		{
		// $*STRUCT CSBrickParamOpening1 TBrickParam::OPENING_1
		// $*-s std::string EventFlag
		Params.push_back(new CSBrickParamOpening1(tail)); 
		}
		break;

	case TBrickParam::OPENING_2:
		{
		// $*STRUCT CSBrickParamOpening2 TBrickParam::OPENING_2
		// $*-s std::string EventFlag1
		// $*-s std::string EventFlag2
		Params.push_back(new CSBrickParamOpening2(tail)); 
		}
		break;

	case TBrickParam::OPENING_3:
		{
		// $*STRUCT CSBrickParamOpening3 TBrickParam::OPENING_3
		// $*-s std::string EventFlag1
		// $*-s std::string EventFlag2
		// $*-s std::string EventFlag3
		Params.push_back(new CSBrickParamOpening3(tail)); 
		}
		break;


	case TBrickParam::COMBAT_SLOW_ATTACK:
		// $*STRUCT CSBrickParamCombatSlowAttack TBrickParam::COMBAT_SLOW_ATTACK
		// $*-f float Duration	// duration of the effect in seconds
		// $*-i sint32 MinFactor	// min factor applied on target attack latency (+50 = +50%)
		// $*-i sint32 MaxFactor	// max factor applied on target attack latency (+50 = +50%)
		Params.push_back(new CSBrickParamCombatSlowAttack(tail)); 
		break;

	case TBrickParam::COMBAT_SLOW:
		// $*STRUCT CSBrickParamCombatSlow TBrickParam::COMBAT_SLOW
		// $*-f float Duration	// duration of the effect in seconds
		// $*-i sint32 MinFactor	// min factor applied on target attack latency or casting time (+50 = +50%)
		// $*-i sint32 MaxFactor	// max factor applied on target attack latency or casting time (+50 = +50%)
		Params.push_back(new CSBrickParamCombatSlow(tail)); 
		break;
		
	case TBrickParam::BLEED_FACTOR:
		// $*STRUCT CSBrickParamBleedFactor TBrickParam::BLEED_FACTOR
		// $*-f float Duration		// duration of the effect in seconds
		// $*-f float MinFactor		// min factor of dealt damage also lost in bleed
		// $*-f float MaxFactor		// max factor of dealt damage also lost in bleed
		Params.push_back(new CSBrickParamBleedFactor(tail)); 
		break;

	case TBrickParam::SPECIAL_HIT:
		// $*STRUCT CSBrickParamSpecialHit TBrickParam::SPECIAL_HIT
		// no param
		Params.push_back(new CSBrickParamSpecialHit(tail)); 
		break;

	case TBrickParam::HIT_ALL_AGGRESSORS:
		// $*STRUCT CSBrickParamHitAllAggressors TBrickParam::HIT_ALL_AGGRESSORS
		// $*-f float MinFactor		// min factor on dealt damage (total damage divided among targets)
		// $*-f float MaxFactor		// max factor on dealt damage (total damage divided among targets)
		Params.push_back(new CSBrickParamHitAllAggressors(tail)); 
		break;

	case TBrickParam::WEAPON_WEAR_MOD:
		// $*STRUCT CSBrickParamWeaponWearMod TBrickParam::WEAPON_WEAR_MOD
		// $*-f float MinModifier		// min weapon wear modifier
		// $*-f float MaxModifier		// max weapon wear modifier
		Params.push_back(new CSBrickParamWeaponWearMod(tail)); 
		break;

	case TBrickParam::CRITICAL_HIT_MOD:
		// $*STRUCT CSBrickParamCriticalHitMod TBrickParam::CRITICAL_HIT_MOD
		// $*-i uint8 MinModifier		// min critical hit chance modifier
		// $*-i uint8 MaxModifier		// max critical hit chance  modifier
		Params.push_back(new CSBrickParamCriticalHitMod(tail)); 
		break;
		
		
	
	/************************************************************************/
	/* MAGIC params                                                         */
	/************************************************************************/

	case TBrickParam::MA:
		// $*STRUCT CSBrickParamMaType TBrickParam::MA
		// $*-s std::string Type	// type name
		Params.push_back(new CSBrickParamMaType(tail)); 
		break;

	case TBrickParam::MA_END:
		// $*STRUCT CSBrickParamMaEnd TBrickParam::MA_END
		Params.push_back(new CSBrickParamMaEnd(tail)); 
		break;

	case TBrickParam::MA_EFFECT:
		// $*STRUCT CSBrickParamMagicEffect TBrickParam::MA_EFFECT
		// $*-s std::string Effect	// effect name
		Params.push_back(new CSBrickParamMagicEffect(tail)); 
		break;
		
	case TBrickParam::MA_STAT:
		// $*STRUCT CSBrickParamMagicStat TBrickParam::MA_STAT
		// $*-s std::string Stat // affected stat
		// $*-s std::string Type // affected stat type
		Params.push_back(new CSBrickParamMagicStat(tail)); 
		break;
		

	case TBrickParam::MA_EFFECT_MOD:
		// $*STRUCT CSBrickParamMagicEffectMod TBrickParam::MA_EFFECT_MOD
		// $*-i sint32 EffectMod 	// effect modifier
		Params.push_back(new CSBrickParamMagicEffectMod(tail)); 
		break;

	case TBrickParam::MA_EFFECT_MULT:
		// $*STRUCT CSBrickParamMagicEffectMult TBrickParam::MA_EFFECT_MULT
		// $*-f float EffectMult 	// effect modifier
		Params.push_back(new CSBrickParamMagicEffectMult(tail)); 
		break;
	
	case TBrickParam::MA_CASTING_TIME:
		// $*STRUCT CSBrickParamCastingTime TBrickParam::MA_CASTING_TIME
		// $*-f float CastingTime= 0	// casting modifier in seconds
		Params.push_back(new CSBrickParamCastingTime(tail)); 
		break;

	case TBrickParam::MA_DMG_TYPE:
		// $*STRUCT CSBrickParamMagicDmgType TBrickParam::MA_DMG_TYPE
		// $*-s std::string DmgType	// magic damage type
		Params.push_back(new CSBrickParamMagicDmgType(tail)); 
		break;

	case TBrickParam::MA_DMG:
		// $*STRUCT CSBrickParamMagicDmg TBrickParam::MA_DMG
		// $*-i sint32 Hp = 0	// fixed modifier on energy
		// $*-i sint32 Sap = 0	// fixed modifier on energy
		// $*-i sint32 Sta = 0	// fixed modifier on energy
		Params.push_back(new CSBrickParamMagicDmg(tail)); 
		break;
	case TBrickParam::MA_HEAL:
		// $*STRUCT CSBrickParamMagicHeal TBrickParam::MA_HEAL
		// $*-i sint32 Hp = 0	// fixed modifier on energy
		// $*-i sint32 Sap = 0	// fixed modifier on energy
		// $*-i sint32 Sta = 0	// fixed modifier on energy
		Params.push_back(new CSBrickParamMagicHeal(tail)); 
		break;
	case TBrickParam::MA_RANGE:
		// $*STRUCT CSBrickParamMagicRanges TBrickParam::MA_RANGE
		// $*-i sint8 RangeIndex
		Params.push_back(new CSBrickParamMagicRanges(tail)); 
		break;
	case TBrickParam::MA_LINK_COST:
		// $*STRUCT CSBrickParamMagicLinkCost TBrickParam::MA_LINK_COST
		// $*-i sint32 Cost
		Params.push_back(new CSBrickParamMagicLinkCost(tail)); 
		break;
	case TBrickParam::MA_LINK_PERIOD:
		// $*STRUCT CSBrickParamMagicLinkPeriod TBrickParam::MA_LINK_PERIOD
		// $*-i uint32 Period
		Params.push_back(new CSBrickParamMagicLinkPeriod(tail)); 
		break;

	case TBrickParam::MA_CURE:
		// $*STRUCT CSBrickParamMagicCure TBrickParam::MA_CURE
		// $*-s std::string Cure
		Params.push_back(new CSBrickParamMagicCure(tail)); 
		break;

	case TBrickParam::MA_LINK_POWER:
		// $*STRUCT CSBrickParamMagicLinkPower TBrickParam::MA_LINK_POWER
		// $*-i uint16 Power
		Params.push_back(new CSBrickParamMagicLinkPower(tail)); 
		break;
	case TBrickParam::MA_BREAK_RES:
		// $*STRUCT CSBrickParamMagicBreakResist TBrickParam::MA_BREAK_RES
		// $*-i uint16 BreakResist
		// $*-i uint16 BreakResistPower
		Params.push_back(new CSBrickParamMagicBreakResist(tail)); 
		break;
	case TBrickParam::MA_ARMOR_COMP:
		// $*STRUCT CSBrickParamMagicArmorComp TBrickParam::MA_ARMOR_COMP
		// $*-i uint16 ArmorComp
		Params.push_back(new CSBrickParamMagicArmorComp(tail)); 
		break;
	case TBrickParam::MA_VAMPIRISE:
		// $*STRUCT CSBrickParamMagicVampirise TBrickParam::MA_VAMPIRISE
		// $*-i sint32 Vampirise
		Params.push_back(new CSBrickParamMagicVampirise(tail)); 
		break;
	case TBrickParam::MA_VAMPIRISE_RATIO:
		// $*STRUCT CSBrickParamMagicVampiriseRatio TBrickParam::MA_VAMPIRISE_RATIO
		// $*-f float VampiriseRatio
		Params.push_back(new CSBrickParamMagicVampiriseRatio(tail)); 
		break;

	/************************************************************************/
	/* CRAFT Params															*/
	/************************************************************************/

	case TBrickParam::CR_RECOMMENDED:
		// $*STRUCT CSBrickParamCraftRecommended TBrickParam::CR_RECOMMENDED
		// $*-i uint32 Recommended
		Params.push_back(new CSBrickParamCraftRecommended(tail)); 
		break;
	case TBrickParam::CR_HP:
		// $*STRUCT CSBrickParamCraftHP TBrickParam::CR_HP
		// $*-i sint32 HitPoint
		Params.push_back(new CSBrickParamCraftHP(tail)); 
		break;
	case TBrickParam::CR_SAP:
		// $*STRUCT CSBrickParamCraftSap TBrickParam::CR_SAP
		// $*-i sint32 Sap
		Params.push_back(new CSBrickParamCraftSap(tail)); 
		break;
	case TBrickParam::CR_STA:
		// $*STRUCT CSBrickParamCraftSta TBrickParam::CR_STA
		// $*-i sint32 Stamina
		Params.push_back(new CSBrickParamCraftSta(tail)); 
		break;
	case TBrickParam::CR_FOCUS:
		// $*STRUCT CSBrickParamCraftFocus TBrickParam::CR_FOCUS
		// $*-i uint32 Focus
		Params.push_back(new CSBrickParamCraftFocus(tail)); 
		break;
	case TBrickParam::CR_QUALITY:
		// $*STRUCT CSBrickParamCraftQuality TBrickParam::CR_QUALITY
		// $*-i sint32 Quality
		Params.push_back(new CSBrickParamCraftQuality(tail)); 
		break;
	case TBrickParam::CR_DURABILITY:
		// $*STRUCT CSBrickParamCraftDurability TBrickParam::CR_DURABILITY
		// $*-f float Durability
		Params.push_back(new CSBrickParamCraftDurability(tail)); 
		break;
	case TBrickParam::CR_DAMAGE:
		// $*STRUCT CSBrickParamCraftDamage TBrickParam::CR_DAMAGE
		// $*-f float Damage
		Params.push_back(new CSBrickParamCraftDamage(tail)); 
		break;
	case TBrickParam::CR_HITRATE:
		// $*STRUCT CSBrickParamCraftHitRate TBrickParam::CR_HITRATE
		// $*-f float HitRate
		Params.push_back(new CSBrickParamCraftHitRate(tail)); 
		break;
	case TBrickParam::CR_RANGE:
		// $*STRUCT CSBrickParamCraftRange TBrickParam::CR_RANGE
		// $*-f float Range
		Params.push_back(new CSBrickParamCraftRange(tail)); 
		break;
	case TBrickParam::CR_DMG_PROTECTION:
		// $*STRUCT CSBrickParamCraftDmgProtection TBrickParam::CR_DMG_PROTECTION
		// $*-f float DmgProtection
		Params.push_back(new CSBrickParamCraftDmgProtection(tail)); 
		break;
	case TBrickParam::CR_SAPLOAD:
		// $*STRUCT CSBrickParamCraftSapload TBrickParam::CR_SAPLOAD
		// $*-f float Sapload
		Params.push_back(new CSBrickParamCraftSapload(tail)); 
		break;
	case TBrickParam::CR_WEIGHT:
		// $*STRUCT CSBrickParamCraftWeight TBrickParam::CR_WEIGHT
		// $*-f float Weight
		Params.push_back(new CSBrickParamCraftWeight(tail)); 
		break;

	/************************************************************************/
	/* Forage Params												        */
	/************************************************************************/

	// ****** Prospection ******
	case TBrickParam::FG_RANGE:
		// $*STRUCT CSBrickParamForageRange TBrickParam::FG_RANGE
		// $*-f float Range
		Params.push_back(new CSBrickParamForageRange(tail));
		break;
	case TBrickParam::FG_LD_RANGE:
		// $*STRUCT CSBrickParamForageLocateDepositRange TBrickParam::FG_LD_RANGE
		// $*-f float Range
		Params.push_back(new CSBrickParamForageLocateDepositRange(tail));
		break;
	case TBrickParam::FG_ANGLE:
		// $*STRUCT CSBrickParamForageAngle TBrickParam::FG_ANGLE
		// $*-i uint32 Angle
		Params.push_back(new CSBrickParamForageAngle(tail));
		break;
	case TBrickParam::FG_MULTI:
		// $*STRUCT CSBrickParamForageMulti TBrickParam::FG_MULTI
		// $*-i uint32 Limit
		Params.push_back(new CSBrickParamForageMulti(tail));
		break;
	case TBrickParam::FG_KNOW:
		// $*STRUCT CSBrickParamForageKnowledge TBrickParam::FG_KNOW
		// $*-i uint8 Know
		Params.push_back(new CSBrickParamForageKnowledge(tail));
		break;
	case TBrickParam::FG_TIME:
		// $*STRUCT CSBrickParamForageTime TBrickParam::FG_TIME
		// $*-f float Time
		Params.push_back(new CSBrickParamForageTime(tail));
		break;
	case TBrickParam::FG_SRC_TIME:
		// $*STRUCT CSBrickParamForageSourceTime TBrickParam::FG_SRC_TIME
		// $*-f float Time
		Params.push_back(new CSBrickParamForageSourceTime(tail));
		break;
	case TBrickParam::FG_STAT_ENERGY:
		// $*STRUCT CSBrickParamForageStatEnergy TBrickParam::FG_STAT_ENERGY
		// $*-f float StatEnergy
		Params.push_back(new CSBrickParamForageStatEnergy(tail));
		break;
	case TBrickParam::FG_STAT_ENERGY_ONLY:
		// $*STRUCT CSBrickParamStatEnergyOnly TBrickParam::FG_STAT_ENERGY_ONLY
		// $*-i uint8 StatEnergyExact
		Params.push_back(new CSBrickParamStatEnergyOnly(tail));
		break;
	case TBrickParam::FG_VIS_DIST:
		// $*STRUCT CSBrickParamForageVisDist TBrickParam::FG_VIS_DIST
		// $*-f float Dist
		Params.push_back(new CSBrickParamForageVisDist(tail));
		break;
	case TBrickParam::FG_VIS_STEALTH:
		// $*STRUCT CSBrickParamForageVisStealth TBrickParam::FG_VIS_STEALTH
		// $*-i uint8 Mode
		Params.push_back(new CSBrickParamForageVisStealth(tail));
		break;
	case TBrickParam::FG_SRC_LOCATOR:
		// $*STRUCT CSBrickParamForageSourceLocator TBrickParam::FG_SRC_LOCATOR
		// $*-i uint8 Flag
		Params.push_back(new CSBrickParamForageSourceLocator(tail));
		break;
	case TBrickParam::FG_ATTEMPTS:
		// $*STRUCT CSBrickParamForageAttempts TBrickParam::FG_ATTEMPTS
		// $*-i uint16 Nb
		Params.push_back(new CSBrickParamForageAttempts(tail));
		break;

	// ****** Extraction ******
	case TBrickParam::FG_ABS_S:
		// $*STRUCT CSBrickParamForageAbsorptionS TBrickParam::FG_ABS_S
		// $*-f float Absorption
		Params.push_back(new CSBrickParamForageAbsorptionS(tail));
		break;
	case TBrickParam::FG_ABS_A:
		// $*STRUCT CSBrickParamForageAbsorptionA TBrickParam::FG_ABS_A
		// $*-f float Absorption
		Params.push_back(new CSBrickParamForageAbsorptionA(tail));
		break;
	case TBrickParam::FG_ABS_Q:
		// $*STRUCT CSBrickParamForageAbsorptionQ TBrickParam::FG_ABS_Q
		// $*-f float Absorption
		Params.push_back(new CSBrickParamForageAbsorptionQ(tail));
		break;
	case TBrickParam::FG_SRC_PRD:
		// $*STRUCT CSBrickParamForagePeriod TBrickParam::FG_SRC_PRD
		// $*-f float Period
		Params.push_back(new CSBrickParamForagePeriod(tail));
		break;
	case TBrickParam::FG_SRC_APT:
		// $*STRUCT CSBrickParamForageAperture TBrickParam::FG_SRC_APT
		// $*-f float Aperture
		Params.push_back(new CSBrickParamForageAperture(tail));
		break;
	case TBrickParam::FG_QUALITY:
		// $*STRUCT CSBrickParamForageQuality TBrickParam::FG_QUALITY
		// $*-f float Quality
		Params.push_back(new CSBrickParamForageQuality(tail));
		break;
	case TBrickParam::FG_PRES:
		// $*STRUCT CSBrickParamForagePreservation TBrickParam::FG_PRES
		// $*-f float Pres
		Params.push_back(new CSBrickParamForagePreservation(tail));
		break;
	case TBrickParam::FG_STAB:
		// $*STRUCT CSBrickParamForageStability TBrickParam::FG_STAB
		// $*-f float Stab
		Params.push_back(new CSBrickParamForageStability(tail));
		break;
	case TBrickParam::FG_CR_STEALTH:
		// $*STRUCT CSBrickParamForageCreatureStealth TBrickParam::FG_CR_STEALTH
		// $*-f float Stealth
		Params.push_back(new CSBrickParamForageCreatureStealth(tail));
		break;
	case TBrickParam::FG_ABS_SRC_DMG:
		// $*STRUCT CSBrickParamForageAbsorbSourceDmg TBrickParam::FG_ABS_SRC_DMG
		// $*-i uint8 Percent
		Params.push_back(new CSBrickParamForageAbsorbSourceDmg(tail));
		break;
	case TBrickParam::KAMI_OFFERING:
		// $*STRUCT CSBrickParamKamiOffering TBrickParam::KAMI_OFFERING
		// $*-i uint32 Num
		Params.push_back(new CSBrickParamKamiOffering(tail));
		break;
	case TBrickParam::KAMI_ANGER_DECREASE:
		// $*STRUCT CSBrickParamKamiAngerDecrease TBrickParam::KAMI_ANGER_DECREASE
		// $*-f float Delta
		Params.push_back(new CSBrickParamKamiAngerDecrease(tail));
		break;
	case TBrickParam::FG_REDUCE_DMG:
		// $*STRUCT CSBrickParamForageReduceDamage TBrickParam::FG_REDUCE_DMG
		// $*-f float Ratio
		Params.push_back(new CSBrickParamForageReduceDamage(tail));
		break;

	// ****** Prospection & extraction ******
	case TBrickParam::FG_ECT_SPC:
		// $*STRUCT CSBrickParamForageEcotypeSpec TBrickParam::FG_ECT_SPC
		// $*-s std::string Ecotype
		Params.push_back(new CSBrickParamForageEcotypeSpec(tail));
		break;
	case TBrickParam::FG_RMGRP_FILT:
		// $*STRUCT CSBrickParamForageRMGroupFilter TBrickParam::FG_RMGRP_FILT
		// $*-i uint32 Value
		Params.push_back(new CSBrickParamForageRMGroupFilter(tail));
		break;
	case TBrickParam::FG_RMFAM_FILT:
		// $*STRUCT CSBrickParamForageRMFamilyFilter TBrickParam::FG_RMFAM_FILT
		// $*-i uint32 Value
		Params.push_back(new CSBrickParamForageRMFamilyFilter(tail));
		break;
	case TBrickParam::FG_ITEMPART_FILT:
		// $*STRUCT CSBrickParamForageItemPartFilter TBrickParam::FG_ITEMPART_FILT
		// $*-i uint32 ItemPartIndex
		Params.push_back(new CSBrickParamForageItemPartFilter(tail));
		break;

	/************************************************************************/
	/* Special Powers Params												*/
	/************************************************************************/
	case TBrickParam::SP_TAUNT:
		// $*STRUCT CSBrickParamPowerTaunt TBrickParam::SP_TAUNT
		// $*-i uint16 TauntPower	// entities of higher level cannot be taunt
		// $*-f float Range			// effective range in meters
		// $*-f float DisableTime	// disable taunt powers for x seconds
		Params.push_back(new CSBrickParamPowerTaunt(tail)); 
		break;

	case TBrickParam::SP_SHIELDING:
		// $*STRUCT CSBrickParamShielding TBrickParam::SP_SHIELDING
		// $*-i uint8	NoShieldProtectionFactor	// granted protection in % without a shield
		// $*-i uint16	NoShieldProtectionMax	// max protection without a shield
		// $*-i uint8	BucklerProtectionFactor	// granted protection in % with a buckler
		// $*-i uint16	BucklerProtectionMax		// max protection with a buckler
		// $*-i uint8	ShieldProtectionFactor	// granted protection in % with a shield
		// $*-i uint16	ShieldProtectionMax		// max protection with a shield
		// $*-f float	Duration					// power duration
		// $*-f float	DisableTime				// disable power for x seconds	
		Params.push_back(new CSBrickParamShielding(tail)); 
		break;

	case TBrickParam::SP_LIFE_AURA:
		// $*STRUCT CSBrickParamLifeAura TBrickParam::SP_LIFE_AURA
		// $*-i uint16	RegenMod			// regen modifier (in %)
		// $*-f float	Duration			// duration in seconds
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable life aura for x seconds on user
		Params.push_back(new CSBrickParamLifeAura(tail)); 
		break;

	case TBrickParam::SP_LIFE_AURA2:
		// $*STRUCT CSBrickParamLifeAura2 TBrickParam::SP_LIFE_AURA2
		// $*-i uint16	RegenMod			// regen modifier (in %) proportionally to item level
		// $*-f float	Duration			// duration in seconds
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable life aura for x seconds on user
		Params.push_back(new CSBrickParamLifeAura2(tail));
		break;

	case TBrickParam::SP_STAMINA_AURA:
		// $*STRUCT CSBrickParamStaminaAura TBrickParam::SP_STAMINA_AURA
		// $*-i uint16	RegenMod			// regen modifier (in %)
		// $*-f float	Duration			// duration in seconds
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable life aura for x seconds on user
		Params.push_back(new CSBrickParamStaminaAura(tail)); 
		break;

	case TBrickParam::SP_STAMINA_AURA2:
		// $*STRUCT CSBrickParamStaminaAura2 TBrickParam::SP_STAMINA_AURA2
		// $*-i uint16	RegenMod			// regen modifier (in %) proportionally to item level
		// $*-f float	Duration			// duration in seconds
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable life aura for x seconds on user
		Params.push_back(new CSBrickParamStaminaAura2(tail)); 
		break;

	case TBrickParam::SP_SAP_AURA:
		// $*STRUCT CSBrickParamSapAura TBrickParam::SP_SAP_AURA
		// $*-i uint16	RegenMod			// regen modifier (in %)
		// $*-f float	Duration			// duration in seconds
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable life aura for x seconds on user
		Params.push_back(new CSBrickParamSapAura(tail)); 
		break;

	case TBrickParam::SP_SAP_AURA2:
		// $*STRUCT CSBrickParamSapAura2 TBrickParam::SP_SAP_AURA2
		// $*-i uint16	RegenMod			// regen modifier (in %) proportionally to item level
		// $*-f float	Duration			// duration in seconds
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	TargetDisableTime	// disable life aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable life aura for x seconds on user
		Params.push_back(new CSBrickParamSapAura2(tail)); 
		break;

	case TBrickParam::SP_SPEEDING_UP:
		// $*STRUCT CSBrickParamSpeedingUp TBrickParam::SP_SPEEDING_UP
		// $*-i uint16	SpeedMod			// speed modifier (in %)
		// $*-f float	Duration			// duration in seconds
		// $*-f float	DisableTime			// disable power for x seconds	
		Params.push_back(new CSBrickParamSpeedingUp(tail)); 
		break;

	case TBrickParam::SP_INVULNERABILITY:
		// $*STRUCT CSBrickParamInvulnerability TBrickParam::SP_INVULNERABILITY
		// $*-f float	Duration			// duration in seconds
		// $*-f float	DisableTime			// disable power for x seconds	
		Params.push_back(new CSBrickParamInvulnerability(tail));
		break;

	case TBrickParam::SP_MELEE_PROTECTION_AURA:
		// $*STRUCT CSBrickParamMeleeProtection TBrickParam::SP_MELEE_PROTECTION_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		Params.push_back(new CSBrickParamMeleeProtection(tail));
		break;

	case TBrickParam::SP_RANGE_PROTECTION_AURA:
		// $*STRUCT CSBrickParamRangeProtection TBrickParam::SP_RANGE_PROTECTION_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		Params.push_back(new CSBrickParamRangeProtection(tail));
		break;

	case TBrickParam::SP_MAGIC_PROTECTION_AURA:
		// $*STRUCT CSBrickParamMagicProtection TBrickParam::SP_MAGIC_PROTECTION_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		Params.push_back(new CSBrickParamMagicProtection(tail));
		break;

	case TBrickParam::SP_WAR_CRY_AURA:
		// $*STRUCT CSBrickParamWarCry TBrickParam::SP_WAR_CRY_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		// $*-i sint16	DamageBonus			// damage bonus (20 = +20%)
		Params.push_back(new CSBrickParamWarCry(tail));
		break;

	case TBrickParam::SP_FIRE_WALL_AURA:
		// $*STRUCT CSBrickParamFireWall TBrickParam::SP_FIRE_WALL_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		// $*-i sint16	Damage				// damage 
		Params.push_back(new CSBrickParamFireWall(tail));
		break;

	case TBrickParam::SP_THORN_WALL_AURA:
		// $*STRUCT CSBrickParamThornWall TBrickParam::SP_THORN_WALL_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		// $*-i sint16	Damage				// damage 
		Params.push_back(new CSBrickParamThornWall(tail));
		break;

	case TBrickParam::SP_WATER_WALL_AURA:
		// $*STRUCT CSBrickParamWaterWall TBrickParam::SP_WATER_WALL_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		// $*-i sint16	Damage				// damage 
		Params.push_back(new CSBrickParamWaterWall(tail));
		break;

	case TBrickParam::SP_LIGHTNING_WALL_AURA:
		// $*STRUCT CSBrickParamLightningWall TBrickParam::SP_LIGHTNING_WALL_AURA
		// $*-f float	Radius				// aura radius in meters
		// $*-f float	Duration			// duration in seconds
		// $*-f float	TargetDisableTime	// disable aura for x seconds on targets
		// $*-f float	UserDisableTime		// disable aura for x seconds on user
		// $*-i sint16	Damage				// damage 
		Params.push_back(new CSBrickParamLightningWall(tail));
		break;

	case TBrickParam::SP_BERSERK:
		// $*STRUCT CSBrickParamBerserk TBrickParam::SP_BERSERK
		// $*-f float	DisableTime			// disable berserker power for x seconds
		// $*-f float	Duration			// duration in seconds
		// $*-f float	DamagePerUpdate		// DoT damage suffered by user, damage per update
		// $*-f float	UpdateFrequency		// DoT update frequency in seconds
		// $*-i uint16	DamageBonus			// damage bonus (20 = +20 damage points before success factor)
		Params.push_back(new CSBrickParamBerserk(tail));
		break;
		
	case TBrickParam::SP_ENCHANT_WEAPON:
		// $*STRUCT CSBrickParamEnchantWeapon TBrickParam::SP_ENCHANT_WEAPON
		// $*-f float	DisableTime			// disable berserker power for x seconds
		// $*-f float	Duration			// duration in seconds
		// $*-s std::string DamageType		// damage type
		// $*-f float	DpsBonus			// DoT update frequency in seconds
		// $*-i uint16	DamageBonus			// damage bonus (20 = +20 damage points before success factor)
		Params.push_back(new CSBrickParamEnchantWeapon(tail));
		break;

	case TBrickParam::SP_CALM_ANIMAL:
		// $*STRUCT CSBrickParamCalmAnimal TBrickParam::SP_CALM_ANIMAL
		// $*-s std::string	TypeRestriction		// fauna type restriction (quadruped, land kittin...)
		// $*-u uint32		Power				// power of the effect (to oppose to creature level 0-250+)
		// $*-f float		Radius				// aura radius in meters
		// $*-f float		DisableTime			// duration in seconds
		Params.push_back(new CSBrickParamCalmAnimal(tail));
		break;
			
	case TBrickParam::NEEDED_BRICK_FLAG:
		// $*STRUCT CSBrickParamNeededBrickFlag TBrickParam::NEEDED_BRICK_FLAG
		// $*-s std::string Flag
		Params.push_back(new CSBrickParamNeededBrickFlag(tail)); 
		break;

	case TBrickParam::SP_BALANCE:
		// $*STRUCT CSBrickParamBalance TBrickParam::SP_BALANCE
		// $*-f float		DisableTime			// disable power for x seconds
		// $*-s std::string AffectedScore		// affected score 
		// $*-f float		Range				// power range
		// $*-f float		LossFactor			// score loss factor in %
		Params.push_back(new CSBrickParamBalance(tail));
		break;
		
	case TBrickParam::SP_HEAL:
		// $*STRUCT CSBrickParamHeal TBrickParam::SP_HEAL
		// $*-s std::string AffectedScore		// affected score 
		// $*-i sint32		HealValue			// value added to affected score
		// $*-f float		HealFactorValue		// value added to affected score in % of max target score
		// $*-f float		DisableTime			// disable power for x seconds
		// $*-s std::string	PowerType			// type of power (Heal, HealHpC ...)
		Params.push_back(new CSBrickParamHeal(tail));
		break;

	case TBrickParam::SP_RECAST_TIME:
		// $*STRUCT CSBrickParamRecastTime TBrickParam::SP_RECAST_TIME
		// $*-f float		Time			// disable power for x seconds
		Params.push_back(new CSBrickParamRecastTime(tail));
		break;

	case TBrickParam::SP_CHG_CHARAC:
		// $*STRUCT CSBrickParamChgCharac TBrickParam::SP_CHG_CHARAC
		// $*-s std::string AffectedCharac		// affected characteristic
		// $*-f float		ModifCoef			// coefficient to modify characteristic proportionately to item level
		// $*-i float		ModifConst			// value added to affected characteristic
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamChgCharac(tail));
		break;

	case TBrickParam::SP_MOD_MAGIC_PROTECTION:
		// $*STRUCT CSBrickParamModMagicProtection TBrickParam::SP_MOD_MAGIC_PROTECTION
		// $*-s std::string AffectedProtection	// affected magic protection
		// $*-f float		ModifCoef			// coefficient to modify protection proportionally to item level
		// $*-i float		ModifConst			// value added to affected protection
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModMagicProtection(tail));
		break;

	case TBrickParam::SP_MOD_DEFENSE:
		// $*STRUCT CSBrickParamModDefense TBrickParam::SP_MOD_DEFENSE
		// $*-s std::string DefenseMode			// Dodge or Parry ?
		// $*-f float		ModifCoef			// coefficient to modify defense success chance proportionately to item level
		// $*-i float		ModifConst			// value added to defense success chance
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModDefense(tail));
		break;

	case TBrickParam::SP_MOD_CRAFT_SUCCESS:
		// $*STRUCT CSBrickParamModCraftSuccess TBrickParam::SP_MOD_CRAFT_SUCCESS
		// $*-f float		ModifCoef			// coefficient to modify craft success chance proportionately to item level
		// $*-i float		ModifConst			// value added to craft success chance
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModCraftSuccess(tail));
		break;

	case TBrickParam::SP_MOD_MELEE_SUCCESS:
		// $*STRUCT CSBrickParamModMeleeSuccess TBrickParam::SP_MOD_MELEE_SUCCESS
		// $*-f float		ModifCoef			// coefficient to modify melee success chance proportionately to item level
		// $*-i float		ModifConst			// value added to fight success chance
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModMeleeSuccess(tail));
		break;

	case TBrickParam::SP_MOD_RANGE_SUCCESS:
		// $*STRUCT CSBrickParamModRangeSuccess TBrickParam::SP_MOD_RANGE_SUCCESS
		// $*-f float		ModifCoef			// coefficient to modify range success chance proportionately to item level
		// $*-i float		ModifConst			// value added to fight success chance
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModRangeSuccess(tail));
		break;

	case TBrickParam::SP_MOD_MAGIC_SUCCESS:
		// $*STRUCT CSBrickParamModMagicSuccess TBrickParam::SP_MOD_MAGIC_SUCCESS
		// $*-f float		ModifCoef			// coefficient to modify magic success chance proportionately to item level
		// $*-i float		ModifConst			// value added to fight success chance
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModMagicSuccess(tail));
		break;

	case TBrickParam::SP_MOD_FORAGE_SUCCESS:
		// $*STRUCT CSBrickParamModForageSuccess TBrickParam::SP_MOD_FORAGE_SUCCESS
		// $*-s std::string Ecosystem			// cf ecosystem.h (game_share)
		// $*-f float		ModifCoef			// coefficient to modify forage success chance proportionately to item level
		// $*-i float		ModifConst			// value added to forage success chance
		// $*-f float		Duration			// duration in seconds
		Params.push_back(new CSBrickParamModForageSuccess(tail));
		break;

	/************************************************************************/
	/* Bonus																*/
	/************************************************************************/

	case TBrickParam::BONUS_FG_EXTRACTION_TIME_GC:
		// $*STRUCT CSBrickParamBonusFgExtractionTimeGC TBrickParam::BONUS_FG_EXTRACTION_TIME_GC
		// $*-f float AdditionalTimeGC
		Params.push_back(new CSBrickParamBonusFgExtractionTimeGC(tail));
		break;

	case TBrickParam::BONUS_CR_DURABILITY:
		// $*STRUCT CSBrickParamBonusCrDurability TBrickParam::BONUS_CR_DURABILITY
		// $*-f float Bonus
		Params.push_back(new CSBrickParamBonusCrDurability(tail));
		break;

	case TBrickParam::BONUS_LANDMARK_NUMBER:
		// $*STRUCT CSBrickParamBonusLandmarkNumber TBrickParam::BONUS_LANDMARK_NUMBER
		// $*-f float Nb
		Params.push_back(new CSBrickParamBonusLandmarkNumber(tail));
		break;

	/************************************************************************/
	/* Areas Params															*/
	/************************************************************************/
	case TBrickParam::AREA_BOMB:
		// $*STRUCT CSBrickParamAreaBomb TBrickParam::AREA_BOMB
		// $*-f float Radius		// Radius
		// $*-f float MinFactor		// MinFactor when we are at extreme range
		Params.push_back(new CSBrickParamAreaBomb(tail)); 
		break;
	case TBrickParam::AREA_SPRAY:
		// $*STRUCT CSBrickParamAreaSpray TBrickParam::AREA_SPRAY
		// $*-i uint8 Angle			// angle in degree
		// $*-f float Height		// height of the trapezoid
		// $*-f float Base			// little base length
		Params.push_back(new CSBrickParamAreaSpray(tail)); 
		break;
	case TBrickParam::AREA_CHAIN:
		// $*STRUCT CSBrickParamAreaChain TBrickParam::AREA_CHAIN
		// $*-f float Range			// range between 2 targets
		// $*-i uint8 MaxTargets	// max nb targets
		// $*-f float Factor		// damage factor
		Params.push_back(new CSBrickParamAreaChain(tail)); 
		break;
	case TBrickParam::AREA_TARGETS:
		// $*STRUCT CSBrickParamAreaTargets TBrickParam::AREA_TARGETS
		// $*-f float	TargetFactor	// each target count as 'TargetFactor' for damage or heal division among targets
		// $*-i uint8	MaxTargets		// max nb targets
		Params.push_back(new CSBrickParamAreaTargets(tail)); 
		break;

	/************************************************************************/
	/* Enchantment Params
	/************************************************************************/
	case TBrickParam::MA_RECHARGE:
		// $*STRUCT CSBrickParamMagicRecharge TBrickParam::MA_RECHARGE
		// $*-i uint32 SapLoad		// sap load
		Params.push_back(new CSBrickParamMagicRecharge(tail)); 
		break;

	/************************************************************************/
	/* upgrade params (training bricks)
	/************************************************************************/
	case TBrickParam::CHARAC_UPGRADE:
		// $*STRUCT CSBrickParamCharacUpgrade TBrickParam::CHARAC_UPGRADE
		// $*-s std::string		Characteristic	// affected characteristic
		// $*-i uint32			Modifier		// bonus on charac
		Params.push_back(new CSBrickParamCharacUpgrade(tail)); 
		break;
	
	case TBrickParam::SCORE_UPGRADE:
		// $*STRUCT CSBrickParamScoreUpgrade TBrickParam::SCORE_UPGRADE
		// $*-s std::string		Score		// affected score
		// $*-i sint32			Modifier	// modifier on score
		Params.push_back(new CSBrickParamScoreUpgrade(tail)); 
		break;

	/************************************************************************/
	/* Timed Actions Params
	/************************************************************************/
	case TBrickParam::TA_TELEPORT:
		// $*STRUCT CSBrickParamTeleport TBrickParam::TA_TELEPORT
		Params.push_back(new CSBrickParamTeleport(tail)); 
		break;

	case TBrickParam::TA_DISCONNECT:
		// $*STRUCT CSBrickParamDisconnect TBrickParam::TA_DISCONNECT
		Params.push_back(new CSBrickParamDisconnect(tail)); 
		break;

	case TBrickParam::TA_MOUNT:
		// $*STRUCT CSBrickParamMount TBrickParam::TA_MOUNT
		Params.push_back(new CSBrickParamMount(tail)); 
		break;

	case TBrickParam::TA_UNMOUNT:
		// $*STRUCT CSBrickParamUnmount TBrickParam::TA_UNMOUNT
		Params.push_back(new CSBrickParamUnmount(tail)); 
		break;

	case TBrickParam::TA_CONSUME:
		// $*STRUCT CSBrickParamConsumeItem TBrickParam::TA_CONSUME
		Params.push_back(new CSBrickParamConsumeItem(tail)); 
		break;
	}
} // addParam //

//--------------------------------------------------------------
//						loadFaber()  
//--------------------------------------------------------------
void CStaticBrick::loadFaber( const UFormElm &root, const CSheetId &sheetId )
{
	if ( Faber == NULL) Faber = new CFaber();
	
	const UFormElm *faber = NULL;
	
	string value;
	string groupFamily;
	string propName;
	uint16 quantity = 0;
	
	root.getValueByName( value, "faber.Create.Crafted Item" );
	Faber->CraftedItem = CSheetId(value);

	root.getValueByName( Faber->NbItemsPerUnit, "faber.Create.Nb built items" );

	root.getValueByName( Faber->Durability, "faber.Durability Factor" );
	root.getValueByName( Faber->Weight, "faber.Weight Factor" );
	root.getValueByName( Faber->Dmg, "faber.DMG Factor" );
	root.getValueByName( Faber->Speed, "faber.Speed Factor" );
	root.getValueByName( Faber->SapLoad, "faber.SapLoad Factor" );
	root.getValueByName( Faber->Range, "faber.Range Factor" );
	root.getValueByName( Faber->DodgeModifier, "faber.Dodge Factor" );
	root.getValueByName( Faber->ParryModifier, "faber.Parry Factor" );
	root.getValueByName( Faber->AdversaryDodgeModifier, "faber.Adversary Dodge Modifier Factor" );
	root.getValueByName( Faber->AdversaryParryModifier, "faber.Adversary Parry Modifier Factor" );
	root.getValueByName( Faber->ProtectionFactor, "faber.Protection Factor" );
	root.getValueByName( Faber->MaxSlashingProtection, "faber.Slashing Protection Factor" );
	root.getValueByName( Faber->MaxBluntProtection, "faber.Blunt Protection Factor" );
	root.getValueByName( Faber->MaxPiercingProtection, "faber.Piercing Protection Factor" );

	root.getValueByName( Faber->AcidProtectionFactor, "faber.Acid Protection Factor" );
	root.getValueByName( Faber->ColdProtectionFactor, "faber.Cold Protection Factor" );
	root.getValueByName( Faber->FireProtectionFactor, "faber.Fire Protection Factor" );
	root.getValueByName( Faber->RotProtectionFactor, "faber.Rot Protection Factor" );
	root.getValueByName( Faber->ShockWaveProtectionFactor, "faber.ShockWave Protection Factor" );
	root.getValueByName( Faber->PoisonProtectionFactor, "faber.Poison Protection Factor" );
	root.getValueByName( Faber->ElectricityProtectionFactor, "faber.Electricity Protection Factor" );

	root.getValueByName( Faber->DesertResistanceFactor, "faber.Desert Resistance Factor" );
	root.getValueByName( Faber->ForestResistanceFactor, "faber.Forest Resistance Factor" );
	root.getValueByName( Faber->LacustreResistanceFactor, "faber.Lacustre Resistance Factor" );
	root.getValueByName( Faber->JungleResistanceFactor, "faber.Jungle Resistance Factor" );
	root.getValueByName( Faber->PrimaryRootResistanceFactor, "faber.PrimaryRoot Resistance Factor" );

	root.getValueByName( Faber->ElementalCastingTimeFactor, "faber.Elemental Casting Time Factor" );
	root.getValueByName( Faber->ElementalPowerFactor, "faber.Elemental Power Factor" );
	root.getValueByName( Faber->OffensiveAfflictionCastingTimeFactor, "faber.Offensive Affliction Casting Time Factor" );
	root.getValueByName( Faber->OffensiveAfflictionPowerFactor, "faber.Offensive Affliction Power Factor" );
	root.getValueByName( Faber->HealCastingTimeFactor, "faber.Heal Casting Time Factor" );
	root.getValueByName( Faber->HealPowerFactor, "faber.Heal Power Factor" );
	root.getValueByName( Faber->DefensiveAfflictionCastingTimeFactor, "faber.Defensive Affliction Casting Time Factor" );
	root.getValueByName( Faber->DefensiveAfflictionPowerFactor, "faber.Defensive Affliction Power Factor" );

	root.getValueByName( Faber->HpBonusPerLevel, "faber.HpBonusPerLevel" );
	root.getValueByName( Faber->SapBonusPerLevel, "faber.SapBonusPerLevel" );
	root.getValueByName( Faber->StaBonusPerLevel, "faber.StaBonusPerLevel" );
	root.getValueByName( Faber->FocusBonusPerLevel, "faber.FocusBonusPerLevel" );

	root.getValueByName( Faber->AllowPartialSuccess, "faber.AllowPartialSuccess" );
	
	for (uint i = 1 ; i <= 5 ; ++i)
	{
		propName = "faber.Create.MP "+toString(i);
		if (root.getValueByName( value, propName.c_str() ) && !value.empty() )
		{			
			propName = "faber.Create.Quantity "+toString(i);
			if ( root.getValueByName( quantity, propName.c_str() ) && quantity > 0)
			{
				CFaber::TRawMaterial mp;
				mp.MpType = RM_FABER_TYPE::toFaberType( value );
				mp.Quantity = quantity;
				Faber->NeededMps.push_back( mp );
			}
		}
	}

	for (uint i = 1 ; i <= 5 ; ++i)
	{
		propName = "faber.Create.MP formula "+toString(i);
		if (root.getValueByName( value, propName.c_str() ) && !value.empty() )
		{			
			propName = "faber.Create.Quantity formula "+toString(i);
			if( root.getValueByName( quantity, propName.c_str() ) && quantity > 0) 
			{
				CFaber::TRawMaterialFormula mpFormula;
				mpFormula.MpType = CSheetId(value);
				mpFormula.Quantity = quantity;
				Faber->NeededMpsFormula.push_back( mpFormula );
			}
		}
	}
} // loadFaber //


//--------------------------------------------------------------
//						getBrickFromFamilyIndex()  
//--------------------------------------------------------------
const CStaticBrick *CStaticBrick::getBrickFromFamilyIndex(uint16 family, uint16 index)
{
	std::map< std::pair<uint16, uint16>, NLMISC::CSheetId>::const_iterator it = _Bricks.find( std::make_pair(family,index));
	if (it != _Bricks.end())
	{
		return CSheets::getSBrickForm((*it).second);
	}
	else
	{
		return NULL;
	}
} // getBrickFromFamilyIndex //


//--------------------------------------------------------------
//						reloadSheet
//--------------------------------------------------------------
void CStaticBrick::reloadSheet(const CStaticBrick &o)
{
	// nothing special
	*this= o;
}




