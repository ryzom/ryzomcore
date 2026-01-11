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

#include <nel/misc/types_nl.h>
#include "game_share/egs_sheets/egs_static_brick.h"
#include <set>
#include <vector>


/**
 * Get the sabrina phrases to make available for a player (who wants to buy phrases from his Rolemaster).
 * \param brickFilter A part of a brick code indicating a skill tree branch. Only option bricks matching
 *        this filter will be returned.
 * \param bricksAlreadyKnown The bricks known by the player. Phrases containing a brick that requires
 *		  a brick that the player doesn't have will be discarded (see CStaticBrick::BrickRequiresBricks).
 * \param phrasesAlreadyKnown The rolemaster phrases that were sold to the player before. These phrases
 *        won't be returned again.
 * \param playerSkills The current skills/levels of the player. Phrases containing a brick with too high
 *        required skill for the player will be discarded (see CStaticBrick::BrickRequiresOneOfSkill).
 * \param result The vector where the phrases will be inserted (must be empty before the call).
 */
void	buildAvailablePhrasesList( const std::string& brickFilter,
								   const std::set<NLMISC::CSheetId>& bricksAlreadyKnown,
								   const std::set<NLMISC::CSheetId>& phrasesAlreadyKnown,
								   const std::vector<CPlayerSkill>& playerSkills,
								   std::vector<NLMISC::CSheetId>& result );


#endif
