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
#include "phrase_manager/available_phrases.h"
#include "egs_sheets/egs_sheets.h"

#include "game_share/fame.h"
#include "nel/misc/hierarchical_timer.h"

#include <nel/misc/command.h> // TEMP

using namespace NLMISC;
using namespace std;

/*
 * Accessors for sheet sphrases (for vector)
 */
#define GET_PHRASE(it) (*(*it).first)
#define GET_SHEET_ID(it) (*it).second.first
#define GET_PHRASE_NAME(it) (*it).second.second


/*
 * Find in oneOfSkillsRequiredList at least one skill that is lower (by value) than the provided limit value
 */
inline bool	skillRequiredAreBelowSkillValueLimit( uint skillValueLimit, const vector<CPlayerSkill>& oneOfSkillsRequiredList )
{
	H_AUTO(skillRequiredAreBelowSkillValueLimit);
	vector<CPlayerSkill>::const_iterator irs;
	for ( irs=oneOfSkillsRequiredList.begin(); irs!=oneOfSkillsRequiredList.end(); ++irs )
	{
		const CPlayerSkill& requiredSkill = *irs;
		if ( requiredSkill.isSkillValueLowerThan( skillValueLimit ) )
		{
			return true;
		}
	}
	return false;
}


/*
 * Find in the player skill list has at least one skill that is higher than the provided skill.
 *
 * Precondition:
 * - playerSkills is indexed by SKILLS::ESkill (<SKILLS::NUM_SKILLS)
 */
inline bool	skillMatchesOneOfList( const CPlayerSkill& skill, const vector<SSkill>& playerSkills )
{
	return (playerSkills[skill.Code].MaxLvlReached >= skill.Value);
}


/*
 * Find in oneOfSkillsRequiredList at least one skill that matches one of the player skills
 *
 * Precondition:
 * - oneOfSkillsRequiredList contains valid skills (not unknown)
 */
inline bool	skillRequiredMatchPlayerSkills( const vector<SSkill>& playerSkills, const vector<CPlayerSkill>& oneOfSkillsRequiredList )
{
	H_AUTO(skillRequiredMatchPlayerSkills);
	vector<CPlayerSkill>::const_iterator irs;
	for ( irs=oneOfSkillsRequiredList.begin(); irs!=oneOfSkillsRequiredList.end(); ++irs )
	{
		const CPlayerSkill& requiredSkill = *irs;
		if ( skillMatchesOneOfList( requiredSkill, playerSkills ) )
			return true;
	}
	return false;
}


/*
 * Check character fame against brick fame
 *
 */
inline bool	fameRequiredMatchPlayerFaction( const CEntityId& eid, const string& faction, sint32 brickFame )
{
	H_AUTO(fameRequiredMatchPlayerFaction);
	
	if( eid.isUnknownId() )
	{
		return false;
	}

	if( faction.empty() )
	{
		return true;
	}

	TStringId fameId = CStringMapper::map(faction);
	sint32 userFame = CFameInterface::getInstance().getFame(eid, fameId);
	if( userFame == NO_FAME )
	{
		return false;
	}
	if( userFame < brickFame )
	{
		return false;
	}

	return true;
}


/*
 * Return false if the phrase contains bricks that require a skill level that the player doesn't have
 * or if it contains bricks that require a brick that the player doesn't have.
 */
bool	isPlayerAllowedToGetAllBricksFromPhrase( const CEntityId& eid,
												 const CStaticRolemasterPhrase& phrase,
												 const std::vector<SSkill>& playerSkills,
												 const set<CSheetId>& bricksAlreadyKnown,
												 uint skillValueLimit,
												 EGSPD::CPeople::TPeople civ,
												 bool bypassBrickRequirements
												 )
{
	H_AUTO(isPlayerAllowedToGetAllBricksFromPhrase);
		
	//nldebug( "Phrase %s", phraseSheetId.toString().c_str() );
	const vector<CSheetId>& bricks = phrase.Bricks;

	bool hasNewBrick = false;

	vector<CSheetId>::const_iterator ibs;
	for ( ibs=bricks.begin(); ibs!=bricks.end(); ++ibs )
	{
		// If this brick is already known, skip it
		if ( bricksAlreadyKnown.find(*ibs) != bricksAlreadyKnown.end() )
		{
			continue;
		}

		hasNewBrick = true;

		const CStaticBrick *staticBrick = CSheets::getSBrickForm( *ibs );
		if ( ! staticBrick )
			return false;
		//nldebug( "  Brick %s", (*ibs).toString().c_str() );

		if ( ! staticBrick->LearnRequiresOneOfSkills.empty() )
		{
			//nldebug( "  -> Requires one of %u skills", staticBrick->LearnRequiresOneOfSkills.size() );

			// Check if the civRestriction of the brick is compatible with the specified civRestriction
//			if ( ! ((civ == EGSPD::CPeople::Common) ||
//				(staticBrick->CivRestriction == EGSPD::CPeople::Common) ||
//				    (staticBrick->CivRestriction == civ)) )
			//if( (staticBrick->CivRestriction != civ) && (staticBrick->CivRestriction != EGSPD::CPeople::Common) )
			//	return false;

			// Check if the brick is below or equal the skill value limit
			if ( ! skillRequiredAreBelowSkillValueLimit( skillValueLimit, staticBrick->LearnRequiresOneOfSkills ) )
				return false;

			// Check if one of the skills match one of the player's
			if ( ! skillRequiredMatchPlayerSkills( playerSkills, staticBrick->LearnRequiresOneOfSkills ) )
				return false;
		}

		// Check if one of the fame required doesn't match with brick min fame value
		if ( ! fameRequiredMatchPlayerFaction( eid, staticBrick->Faction, staticBrick->MinFameValue ) )
			return false;
		
		if ( ! bypassBrickRequirements )
		{
			// Check if all required bricks are known by the player (or are part of the phrase)
			vector<CSheetId>::const_iterator ibl;
			for ( ibl=staticBrick->LearnRequiresBricks.begin(); ibl!=staticBrick->LearnRequiresBricks.end(); ++ibl )
			{
				const CSheetId& brickRequiredSheetId = *ibl;
				if ( bricksAlreadyKnown.find( brickRequiredSheetId ) == bricksAlreadyKnown.end() )
				{
					// The required brick is not known by the player, check if it part of the phrase
					if ( find( bricks.begin(), bricks.end(), brickRequiredSheetId ) == bricks.end() )
						return false;
				}
			}
		}
	}

	// if player known all bricks exclude the phrase
	if (!hasNewBrick)
		return false;

	
	return true;
}


/*
 *
 */
void	buildAvailablePhrasesList( const CEntityId& eid, 
								   const string& brickFilter,
								   const set<CSheetId>& bricksAlreadyKnown,
								   const set<CSheetId>& phrasesAlreadyKnown,
								   const std::vector<SSkill>& playerSkills,
								   vector<CSheetId>& result,
								   uint skillValueLimit,
								   EGSPD::CPeople::TPeople civ,
								   bool bypassBrickRequirements,
								   bool incudeNonRolemasterBricks
								   )
{
	H_AUTO(buildAvailablePhrasesList);

	if( !incudeNonRolemasterBricks )
	{
		// Browse all the rolemaster phrases
		const CAllRolemasterPhrasesLinear& rolemasterPhrases = CSheets::getSRolemasterPhrasesVector();
		CAllRolemasterPhrasesLinear::const_iterator ip;
		for ( ip=rolemasterPhrases.begin(); ip!=rolemasterPhrases.end(); ++ip )
		{
			//nlassert( GET_PHRASE(ip).IsRolemasterPhrase );
			const CSheetId& phraseSheetId = GET_SHEET_ID(ip);

			// Match the brick filter
			if ( GET_PHRASE_NAME(ip).substr( 1, brickFilter.size() ) == brickFilter )
			{
				// Exclude the phrase if the player already knows it
				if ( phrasesAlreadyKnown.find( phraseSheetId ) == phrasesAlreadyKnown.end() )
				{
					// Exclude the phrase if it contains bricks that require a skill level or a brick that the player doesn't have
					if ( isPlayerAllowedToGetAllBricksFromPhrase( eid, GET_PHRASE(ip), playerSkills, bricksAlreadyKnown, skillValueLimit, civ, bypassBrickRequirements ) )
					{
						result.push_back( phraseSheetId );
					}
				}
			}
		}
	}
	else
	{
		// browse all the phrases
		const CAllRolemasterPhrases& rolemasterPhrases = CSheets::getSRolemasterPhrasesMap();
		
		for ( CAllRolemasterPhrases::const_iterator ihm=rolemasterPhrases.begin(); ihm!=rolemasterPhrases.end(); ++ihm )
		{
			if ( (*ihm).first.toString().find( "saiphrase" ) == string::npos )
			{
				const CSheetId& phraseSheetId = (*ihm).first;
				
				// Match the brick filter
				string phraseName = (*ihm).first.toString();
				phraseName = phraseName.substr( 0, phraseName.find( "." ) );
				if ( phraseName.substr( 1, brickFilter.size() ) == brickFilter )
				{
					// Exclude the phrase if the player already knows it
					if ( phrasesAlreadyKnown.find( phraseSheetId ) == phrasesAlreadyKnown.end() )
					{
						// Exclude the phrase if it contains bricks that require a skill level or a brick that the player doesn't have
						if ( isPlayerAllowedToGetAllBricksFromPhrase( eid, (*ihm).second, playerSkills, bricksAlreadyKnown, skillValueLimit, civ, bypassBrickRequirements ) )
						{
							result.push_back( phraseSheetId );
						}
					}	
				}
			}
		}
	}
}


/*
 * Testing command
 */
NLMISC_COMMAND( testRolemaster, "Test rolemaster phrases selection (list separator ':')", "[<skillList> [[<brickList] [skillValueLimit]]]" )
{
	string brickFilter = "b";
	set<CSheetId> knownBricks, knownPhrases;
	uint skillValueLimit = 1000; // should be <= 250 in fact

	// Setup playerSkills (containing all skills in SKILLS::ESkill)
	vector<SSkill> playerSkills( SKILLS::NUM_SKILLS );
	for ( uint i=0; i!=SKILLS::NUM_SKILLS; ++i )
		playerSkills[i].MaxLvlReached = 0;

	// Read argument (including skill list)
	if ( args.size() > 0 )
	{
		const string& skillListS = args[0];
		vector<string> skills;
		explode( skillListS, string(":"), skills, true );
		for ( vector<string>::const_iterator isl=skills.begin(); isl!=skills.end(); ++isl )
		{
			CPlayerSkill ps;
			if ( ps.initFromString( *isl ) )
				playerSkills[ps.Code].MaxLvlReached = ps.Value;
			else
				log.displayNL( "Skill %s malformed", (*isl).c_str() );
		}
		
		if ( args.size() > 1 )
		{
			const string& brickList = args[1];
			vector<string> bricks;
			explode( brickList, string(":"), bricks, true );
			for ( vector<string>::iterator ibl=bricks.begin(); ibl!=bricks.end(); ++ibl )
			{
				if ( (*ibl).find( ".sbrick" ) == string::npos )
					(*ibl) += string(".sbrick");
				 knownBricks.insert( CSheetId( *ibl ) );
			}

			if ( args.size() > 2 )
			{
				NLMISC::fromString(args[2], skillValueLimit);
			}
		}
	}
	else
	{
		playerSkills[SKILLS::SFM].MaxLvlReached = 250;
		playerSkills[SKILLS::SFR].MaxLvlReached = 250;
		playerSkills[SKILLS::SM].MaxLvlReached = 250;
		playerSkills[SKILLS::SC].MaxLvlReached = 250;
		playerSkills[SKILLS::SHF].MaxLvlReached = 250;
	}

	CEntityId eid;

	// Submit
	vector<CSheetId> result;
	buildAvailablePhrasesList( eid, brickFilter, knownBricks, knownPhrases, playerSkills, result, skillValueLimit, EGSPD::CPeople::Common );

	// Display results
	vector<CSheetId>::const_iterator iv;
	for ( iv=result.begin(); iv!=result.end(); ++iv )
	{
		log.displayNL( "%s", (*iv).toString().c_str() );
	}
	log.displayNL( "Found %u phrases on %u", result.size(), CSheets::getSRolemasterPhrasesVector().size() );
	return true;
}
