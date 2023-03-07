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



#ifndef NL_FORAGE_PHRASE_H
#define NL_FORAGE_PHRASE_H

#include "phrase_manager/s_phrase.h"
#include "harvest_source.h"
#include "game_share/ecosystem.h"

/*
 * Vector of skills (no duplicated items)
 */
class CSmallSkillsSet : public std::vector<SKILLS::ESkills>
{
public:

	/// Insert an item if it is not already contained in the set
	void	insert( SKILLS::ESkills s )
	{
		if ( std::find( begin(), end(), s ) == end() )
			push_back( s );
	}

	typedef std::vector<SKILLS::ESkills>::iterator iterator;
};


class CCharacter;


/**
 * Common class for all forage actions (including prospection and extraction)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CForagePhrase : public CSPhrase
{
public:

	/// Constructor
	CForagePhrase();

protected:

	/// Test Sabrina balance and set imbalance
	bool	testSabrinaBalance( sint32 balance, sint32 cost );

	// Obsolete
	/*void	insertProgressingSkill( SKILLS::ESkills s, const NLMISC::CSheetId& brickSheet )
	{
		if ( s != SKILLS::unknown )
			_ProgressingSkills.insert( s );
	}*/

	/// Return the skill that will be used for the specified terrain type (static)
	static SKILLS::ESkills getForageSkillByEcotype( TEcotype ecotype );

	// Return the delta level for the skills corresponding to the phrase
	//sint32	getDeltaLvl( CCharacter *c, SKILLS::ESkills usedSkill, sint32 actionLevel ) const;

	/// Return a randomized success factor for the specified delta level (static)
	static float	rollSuccessFactor( sint32 deltaLvl );

protected: // because CForagePhrase is a common trunc

	/// Acting entity (set by build())
	TDataSetRow					_ActorRowId;

	// 2*SabrinaCost - SabrinaCredit (set by build()) (not useful anymore)
	//sint32						_SabrinaImbalance;

	/// Skills that the action makes progress (used as a set) (set by build()) (obsolete)
	//CSmallSkillsSet			_ProgressingSkills;

	/// Forage time in ticks (set by build)
	NLMISC::TGameCycle			_ForageTime;

	/// Focus cost of the forage action (set by build())
	sint32						_FocusCost;
};


#endif // NL_FORAGE_PHRASE_H

/* End of forage_phrase.h */
