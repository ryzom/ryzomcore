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



#ifndef AVAILABLE_PHRASES_H
#define AVAILABLE_PHRASES_H

#include "egs_sheets/egs_static_brick.h"
#include "egs_sheets/egs_static_rolemaster_phrase.h"
#include "entity_structure/statistic.h"
#include "game_share/people.h"


/**
 * Get the sabrina phrases to make available for a player (who wants to buy phrases from his Rolemaster).
 * \param eid The entity id, used by the fame interface to check fame requirements.
 * \param brickFilter A part of a brick code indicating a skill tree branch. Only option bricks matching
 *        this filter will be returned.
 * \param bricksAlreadyKnown The bricks known by the player. Phrases containing a brick that requires
 *		  a brick that the player doesn't have will be discarded (see CStaticBrick::BrickRequiresBricks).
 * \param phrasesAlreadyKnown The rolemaster phrases that were sold to the player before. These phrases
 *        won't be returned again.
 * \param playerSkills The current skills/levels of the player, indexed by SKILLS::ESkill. Phrases
 *        containing a brick with too high required skill for the player will be discarded
 *        (see CStaticBrick::BrickRequiresOneOfSkill).
 * \param result The vector where the phrases will be inserted (must be empty before the call).
 * \param skillValueLimit A value to be used as a limiter for the bricks in the returned phrases.
 *        If a brick from a phrase has a minimum required skill that is higher that the provided limit
 *        value, the phrase won't be returned.
 * \param civ If not Common, the value will be used to exclude phrases containing a brick
 *        with a different civRestriction that is not 'common'.
 * \param bypassBrickRequirements If set to true then do not check brick requirements (allows to learn increase damage 4
 *		  even if increase damage 3 isn't known for exemple)
 */
void	buildAvailablePhrasesList( const NLMISC::CEntityId& eid,
								   const std::string& brickFilter,
								   const std::set<NLMISC::CSheetId>& bricksAlreadyKnown,
								   const std::set<NLMISC::CSheetId>& phrasesAlreadyKnown,
								   const std::vector<SSkill>& playerSkills,
								   std::vector<NLMISC::CSheetId>& result,
								   uint skillValueLimit=250,
								   EGSPD::CPeople::TPeople civ= EGSPD::CPeople::Common,
								   bool bypassBrickRequirements = false,
								   bool includeNonRolemasterBricks = true
								   );

/*
 * Return false if the phrase contains bricks that require a skill level that the player doesn't have
 * or if it contains bricks that require a brick that the player doesn't have.
 */
bool	isPlayerAllowedToGetAllBricksFromPhrase( const NLMISC::CEntityId& eid,
												 const CStaticRolemasterPhrase& phrase,
												 const std::vector<SSkill>& playerSkills,
												 const std::set<NLMISC::CSheetId>& bricksAlreadyKnown,
												 uint skillValueLimit,
												 EGSPD::CPeople::TPeople civ,
												 bool bypassBrickRequirements
												 );

#endif
