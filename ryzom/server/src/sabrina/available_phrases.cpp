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
#include "available_phrases.h"
#include "game_share/egs_sheets/egs_sheets.h"
#include "game_share/egs_sheets/egs_static_rolemaster_phrase.h"

#include <nel/misc/command.h> // TEMP

using namespace NLMISC;
using namespace std;


/*
 * Accessors for sheet sphrases
 */
#define GET_SHEET_ID(it) (*it).first
#define GET_PHRASE(it) (*it).second


/*
 *
 */
bool	skillMatchesOneOfList( const CPlayerSkill& skill, const vector<CPlayerSkill>& skillList )
{
	vector<CPlayerSkill>::const_iterator ips;
	for ( ips=skillList.begin(); ips!=skillList.end(); ++ips )
	{
		const CPlayerSkill& playerSkill = *ips;
		//nldebug ( "Player: %s %u Required: %s %u", playerSkill.Code.c_str(), playerSkill.Value, skill.Code.c_str(), skill.Value );
		if ( playerSkill.isAsSkilledAs( skill ) )
		{
			//nldebug( "OK" );
			return true;
		}
	}
	return false;
}


/*
 * Return false if the phrase contains bricks that require a skill level that the player doesn't have
 * or if it contains bricks that require a brick that the player doesn't have.
 */
bool	isPlayerAllowedToGetAllBricksFromPhrase( const CSheetId& phraseSheetId,
												 const vector<CPlayerSkill>& playerSkills,
												 const set<CSheetId>& bricksAlreadyKnown )
{
	const CStaticRolemasterPhrase *phrase = CSheets::getSRolemasterPhrase( phraseSheetId );
	if ( ! phrase )
		return false;

	//nldebug( "Phrase %s", phraseSheetId.toString().c_str() );
	const vector<CSheetId>& bricks = phrase->Bricks;
	vector<CSheetId>::const_iterator ibs;
	for ( ibs=bricks.begin(); ibs!=bricks.end(); ++ibs )
	{
		const CStaticBrick *staticBrick = CSheets::getSBrickForm( *ibs );
		if ( ! staticBrick )
			return false;
		//nldebug( "  Brick %s", (*ibs).toString().c_str() );

		// Check if one of the skills match one of the player's
		if ( ! staticBrick->LearnRequiresOneOfSkills.empty() )
		{
			//nldebug( "  -> Requires one of %u skills", staticBrick->LearnRequiresOneOfSkills.size() );
			vector<CPlayerSkill>::const_iterator irs;
			for ( irs=staticBrick->LearnRequiresOneOfSkills.begin(); irs!=staticBrick->LearnRequiresOneOfSkills.end(); ++irs )
			{
				const CPlayerSkill& requiredSkill = *irs;
				if ( skillMatchesOneOfList( requiredSkill, playerSkills ) )
					break;
			}
			if ( irs == staticBrick->LearnRequiresOneOfSkills.end() )
				return false; // not found
		}

		// Check if all required bricks are known by the player
		vector<CSheetId>::const_iterator ibl;
		for ( ibl=staticBrick->LearnRequiresBricks.begin(); ibl!=staticBrick->LearnRequiresBricks.end(); ++ibl )
		{
			const CSheetId& brickRequiredSheetId = *ibl;
			if ( bricksAlreadyKnown.find( brickRequiredSheetId ) == bricksAlreadyKnown.end() )
				return false;
		}
	}
	return true;
}


/*
 *
 */
void	buildAvailablePhrasesList( const string& brickFilter,
								   const set<CSheetId>& bricksAlreadyKnown,
								   const set<CSheetId>& phrasesAlreadyKnown,
								   const vector<CPlayerSkill>& playerSkills,
								   vector<CSheetId>& result )
{
	// Browse all the phrases
	const CAllRolemasterPhrases& phrasesMap = CSheets::getSRolemasterPhrases();
	CAllRolemasterPhrases::const_iterator ip;
	for ( ip=phrasesMap.begin(); ip!=phrasesMap.end(); ++ip )
	{
		const CSheetId& phraseSheetId = GET_SHEET_ID(ip);
		const string& phraseCode = phraseSheetId.toString();

		// Match the brick filter
		if ( phraseCode.substr( 1, brickFilter.size() ) == brickFilter )
		{
			// Exclude the phrase if the player already knows it
			if ( phrasesAlreadyKnown.find( phraseSheetId ) == phrasesAlreadyKnown.end() )
			{
				// Exclude the phrase if it contains bricks that require a skill level or a brick that the player doesn't have
				if ( isPlayerAllowedToGetAllBricksFromPhrase( phraseSheetId, playerSkills, bricksAlreadyKnown ) )
				{
					result.push_back( phraseSheetId );
				}
			}
		}
	}
}


NLMISC_COMMAND( testRolemaster, "Test rolemaster phrases selection", "" )
{
	string brickFilter = "bf";
	set<CSheetId> knownBricks, knownPhrases;
	vector<CPlayerSkill> playerSkills;
	CPlayerSkill playerSkill;
	playerSkill.Code = "FM1B";
	playerSkill.Value = 200;
	playerSkills.push_back( playerSkill );
	vector<CSheetId> result;
	buildAvailablePhrasesList( brickFilter, knownBricks, knownPhrases, playerSkills, result );
	vector<CSheetId>::const_iterator iv;
	for ( iv=result.begin(); iv!=result.end(); ++iv )
	{
		nlinfo( "%s", (*iv).toString().c_str() );
	}
	nlinfo( "Found %u phrases", result.size() );
	return true;
}
