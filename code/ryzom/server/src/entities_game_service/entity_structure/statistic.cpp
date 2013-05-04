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

#include "game_share/characteristics.h"
#include "game_share/scores.h"
#include "game_share/skills.h"
#include "egs_sheets/egs_static_game_sheet.h"
#include "egs_sheets/egs_sheets.h"

#include "statistic.h"
#include "nel/misc/string_conversion.h"


///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

namespace STAT_TYPES
{
	
	NL_BEGIN_STRING_CONVERSION_TABLE (TStatType)
		NL_STRING_CONVERSION_TABLE_ENTRY(Score)
		NL_STRING_CONVERSION_TABLE_ENTRY(Skill)
		NL_STRING_CONVERSION_TABLE_ENTRY(Speed)
		NL_STRING_CONVERSION_TABLE_ENTRY(Unknown)
	NL_END_STRING_CONVERSION_TABLE(TStatType, StatTypeConversion, Unknown)
		
	//-----------------------------------------------
	// toStatType :
	//-----------------------------------------------
	TStatType toStatType(const std::string &str)
	{
		return StatTypeConversion.fromString(str);
	} // toBrickFamily //
	
	
	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string &toString(TStatType type)
	{
		return StatTypeConversion.toString(type);
	} // toString //
}

	
//-----------------------------------------------
// SCharacteristicsAndScores constructor
//-----------------------------------------------
SCharacteristicsAndScores::SCharacteristicsAndScores()
{
	clear();
}


//-----------------------------------------------
// SCharacteristicsAndScores clear()
//-----------------------------------------------
void SCharacteristicsAndScores::clear()
{
	Base = 0;
	Max = OldMax = 0;
	Modifier = 0;
	Current = OldCurrent = 0;
	BaseRegenerateAction = BaseRegenerateRepos = 0;
	RegenerateModifier = 0;
	CurrentRegenerate = 0;
	RegenerateTickUpdate = CTickEventHandler::getGameCycle();
	KeepRegenerateDecimal = 0;
}


//-----------------------------------------------
// SCharacteristicsAndScores destructor
//-----------------------------------------------
SCharacteristicsAndScores::~SCharacteristicsAndScores()
{
}


//-----------------------------------------------
// SCharacteristicsAndScores serial
//-----------------------------------------------
void SCharacteristicsAndScores::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial( Base );
	f.serial( Max );
	f.serial( Modifier );
	f.serial( Current );
	
	f.serial( BaseRegenerateRepos );
	f.serial( BaseRegenerateAction );
	f.serial( RegenerateModifier );
	f.serial( CurrentRegenerate );
}


//-----------------------------------------------
// CPhysicalCharacteristics constructor
//-----------------------------------------------
CPhysicalCharacteristics::CPhysicalCharacteristics()
{
	// Initialize properties
	_PhysicalCharacteristics.reserve( CHARACTERISTICS::NUM_CHARACTERISTICS );
	_PhysicalCharacteristics.resize( CHARACTERISTICS::NUM_CHARACTERISTICS );
	clear();
}


//-----------------------------------------------
// clear :
//-----------------------------------------------
void CPhysicalCharacteristics::clear() 
{
	for (uint32 i=0;i<_PhysicalCharacteristics.size();++i)
		_PhysicalCharacteristics[i].clear();
}


//-----------------------------------------------
// serial :
//-----------------------------------------------
void CPhysicalCharacteristics::serial( NLMISC::IStream &f ) throw(NLMISC::EStream)
{
	for(int i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		_PhysicalCharacteristics[ i ].serial( f );
	}
} // serial //


//-----------------------------------------------
// CPhysicalScores default constructor
//-----------------------------------------------
CPhysicalScores::CPhysicalScores()
{
	_PhysicalScores.reserve(SCORES::NUM_SCORES ); 
	_PhysicalScores.resize( SCORES::NUM_SCORES );

	clear();
}


//-----------------------------------------------
// clear :
//-----------------------------------------------
void CPhysicalScores::clear() 
{
	/// speed modifier (%)
	SpeedVariationModifier=0;

	// speed in meters/second
	CurrentWalkSpeed= BaseWalkSpeed= 1.3f;
	CurrentRunSpeed= BaseRunSpeed= 6.0f;

	for (uint32 i=0;i<_PhysicalScores.size();++i)
		_PhysicalScores[i].clear();
}


//-----------------------------------------------
// serial :
//-----------------------------------------------
void CPhysicalScores::serial( NLMISC::IStream &f ) throw(NLMISC::EStream)
{
	for( int i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		_PhysicalScores[ i ].serial( f );
	}
	
	f.serial( SpeedVariationModifier );
	f.serial( BaseWalkSpeed );
	f.serial( BaseRunSpeed );
	CurrentWalkSpeed.serialRTWM( f );
	CurrentRunSpeed.serialRTWM( f );
} // serial //


//-----------------------------------------------
// SSkill constructor
//-----------------------------------------------
SSkill::SSkill()
{
	clear();
}


//-----------------------------------------------
// clear :
//-----------------------------------------------
void SSkill::clear()
{
	Base = 0;
	Modifier = 0;
	Current = OldCurrent = 0;

	MaxLvlReached = 0;
	Xp = 0.0f;
	XpNextLvl = 0.0f;
}


//-----------------------------------------------
// SSkill destructor
//-----------------------------------------------
SSkill::~SSkill()
{
}


//-----------------------------------------------
// CSkills constructor :
//-----------------------------------------------
CSkills::CSkills(bool noSkills)
{
	if (!noSkills)
		_Skills.resize( SKILLS::NUM_SKILLS );
	_Sp = 0;
	// Initialize properties
/*	for(int i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		_Skills[ i ].Base = 0;
		_Skills[ i ].Modifier = 0;
		_Skills[ i ].Current = 0;
		
		_Skills[ i ].MaxLvlReached = 0;
		_Skills[ i ].Xp = 0.0f;
		_Skills[ i ].XpNextLvl = 0.0f;
	}
*/
}


//-----------------------------------------------
// clear :
//-----------------------------------------------
void CSkills::clear()
{
	_Sp = 0;
	for(uint i = 0; i < _Skills.size(); ++i )
	{
		_Skills[i].clear();
	}
}


//-----------------------------------------------
// serial :
//
//-----------------------------------------------
void CSkills::serial( NLMISC::IStream &f ) throw(NLMISC::EStream)
{
	uint32 size = SKILLS::NUM_SKILLS;
	f.serial( size );
	for (uint i = 0; i < size; ++i )
	{
		if (f.isReading())
		{
			string skillName;
			f.serial( skillName );

			uint num = (uint)SKILLS::toSkill(skillName);
			if ( num <  _Skills.size() )
			{
				f.serial( _Skills[ num ] );
			}
			else
			{
				SSkill skill;
				f.serial( skill );
			}
		}
		else if( _Skills[ i ].Base > 0 || _Skills[ i ].xp > 0 )
		{
			string skillName = SKILLS::toString( i );
			f.serial( skillName );
			f.serial( _Skills[ i ] );
		}
	}
	f.serial( _Sp );
} // serial //


//-----------------------------------------------
// get skill structure for skill name
//-----------------------------------------------
SSkill* CSkills::getSkillStruct( const string& skillName )
{
	const SKILLS::ESkills s = SKILLS::toSkill( skillName );
	if( s != SKILLS::unknown )
	{
		return & _Skills[ s ];
	}
	return NULL;
}

//-----------------------------------------------
// get skill structure for skill name (const version)
//-----------------------------------------------
const SSkill* CSkills::getSkillStruct( const string& skillName ) const
{
	const SKILLS::ESkills s = SKILLS::toSkill( skillName );
	if( s != SKILLS::unknown )
	{
		return & _Skills[ s ];
	}
	return NULL;
}

//-----------------------------------------------
// get skill structure for skill name
//-----------------------------------------------
SSkill* CSkills::getSkillStruct( SKILLS::ESkills skill )
{
	if( (uint)skill <  _Skills.size() )
	{
		return & _Skills[ skill ];
	}
	return NULL;
}

//-----------------------------------------------
// get skill structure for skill name (const version)
//-----------------------------------------------
const SSkill* CSkills::getSkillStruct( SKILLS::ESkills skill ) const
{
	if( (uint)skill <  _Skills.size() )
	{
		return & _Skills[ skill ];
	}
	return NULL;
}

//-----------------------------------------------
//
//-----------------------------------------------
double CSkills::getMaxXPToGain(SKILLS::ESkills &skill)
{
	static CSheetId sheet("skills.skill_tree");
	const CStaticSkillsTree * skillsTree = CSheets::getSkillsTreeForm( sheet );
	nlassert( skillsTree );
	const uint size = (uint)_Skills.size();

	double xp = 0.0f;
	for ( uint i = 0; i < size; ++i )
	{
		if ( _Skills[i].Base != 0 && _Skills[i].XpNextLvl > xp )
		{
			xp = _Skills[i].XpNextLvl;
			skill = SKILLS::ESkills(i);
		}
	}
	return xp;
}
 



