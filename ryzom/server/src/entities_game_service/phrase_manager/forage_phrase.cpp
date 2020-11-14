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
#include "forage_phrase.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "egs_globals.h"
#include "player_manager/character.h"
#include "phrase_manager/s_effect.h"
#include "egs_sheets/egs_static_success_table.h"


using namespace NLMISC;
using namespace std;


/*
 * Linked to ECOSYSTEM::EECosystem
 */
SKILLS::ESkills EcoTypeToUsedSkill [ECOSYSTEM::NUM_ECOSYSTEM+1] =
{
	SKILLS::unknown, // common
	SKILLS::SHFDAEM, // desert
	SKILLS::SHFFAEM, // forest
	SKILLS::SHFLAEM, // lacustre
	SKILLS::SHFJAEM, // jungle
	SKILLS::unknown, // goo
	SKILLS::SHFPAEM, // primary root
	SKILLS::unknown  // unknown
};


/*
 * CForagePhrase constructor
 */
CForagePhrase::CForagePhrase() : CSPhrase()
{
	// Set initial values
	_FocusCost = 0;
	//_SabrinaImbalance = 0;
}


/*
 * Test Sabrina balance and set imbalance
 */
bool	CForagePhrase::testSabrinaBalance( sint32 balance, sint32 cost )
{
	if ( cost > balance )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "PHRASE_NOT_ENOUGH_CREDIT" );
		return false;
	}
	//_SabrinaImbalance = balance + cost;
	return true;
}


/*
 * Return the skill that will be used for the specified terrain type (static)
 */
SKILLS::ESkills CForagePhrase::getForageSkillByEcotype( TEcotype ecotype )
{
	return EcoTypeToUsedSkill[ecotype];
}


/*
 * Return the delta level for the skills corresponding to the phrase
 */
//sint32	CForagePhrase::getDeltaLvl( CCharacter *c, SKILLS::ESkills usedSkill, sint32 actionLevel ) const
//{
	/*// Get average value of progressing skills for the current phrase
	sint32 skillSum = 0;
	for ( CSmallSkillsSet::iterator isk=_ProgressingSkills.begin(); isk!=_ProgressingSkills.end(); ++isk )
	{
		skillSum += c->getSkillValue( *isk );
	}

	// Calculate the average of the skill values of the player for every skill set for the current phrase
	sint32 skillAvg = skillSum / (sint32)_ProgressingSkills.size();*/

	// Calculate the success factor.
	// Previously: averageSkill - (2*cost - credit)
	// Now: averageSkill - cost
	//return c->getSkillValue( usedSkill ) - actionLevel;
	/*return skillAvg - _SabrinaImbalance;*/
//}


/*
 * Return success factor and delta level (static)
 */
float  CForagePhrase::rollSuccessFactor( sint32 deltaLvl )
{
	// Roll success factor
	//uint8 chance = PHRASE_UTILITIES::getSuccessChance( deltaLvl );
	//uint8 chance = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::ForageExtract, deltaLvl);

	uint8 roll = (uint8)RandomGenerator.rand( 99 );
	return CStaticSuccessTable::getSuccessFactor( SUCCESS_TABLE_TYPE::ForageExtract, deltaLvl, roll );
}

