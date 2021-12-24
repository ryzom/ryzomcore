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



#ifndef NL_SELECTION_GENERATOR_H
#define NL_SELECTION_GENERATOR_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <vector>


/**
 * Interface ISelectionGenerator for class that generates a series of numbers that are "selection levels".
 * Note1: this is an "adaptable generator" as defined in the STL.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class ISelectionGenerator
{
public:

	/// Type of level
	typedef uint32 TSelectionLevel;

	/// Type for STL compatibility
	typedef TSelectionLevel result_type;

	/// Initialization of a selection cycle
	virtual void					init( TSelectionLevel nblevels ) {}

	/// Change the number of levels without restarting the cycle
	virtual void					changeNbLevels( TSelectionLevel nblevels ) {}

	/// Return the next level to select
	virtual TSelectionLevel			getNext() = 0;

	/// Return the next level to select (STL style)
	result_type						operator() () { return getNext(); }
};


/*
 * CP2CGenerator: selection generator for strategy Power2WithCeiling
 */
class CP2CGenerator : public ISelectionGenerator
{
public:

	/// Constructor
	CP2CGenerator( TSelectionLevel ceiling );

	/// Initialization of a selection cycle
	virtual void					init( TSelectionLevel nblevels );

	/// Change the number of levels without restarting the cycle
	virtual void					changeNbLevels( TSelectionLevel nblevels );

	/// Return the next level to select
	virtual TSelectionLevel			getNext();

	/// Resursive function that fills _LevelSequence
	static void						generateLevels( std::vector<TSelectionLevel>::iterator& iter, TSelectionLevel level );

private:

	/// Stored sequence
	std::vector<TSelectionLevel>	_LevelSequence;

	/// Index of current level, pointing to _LevelSequence
	uint32							_CurrentLevelIndex;

	/// Max level for power of 2. All greater levels (which I call "low ranks") are equitable
	TSelectionLevel					_Ceiling;

	/// Total number of levels
	TSelectionLevel					_NbLevels;

	/// Current index in the low ranks (equitable levels)
	uint32							_CurrentLowRank;

};


/// Default ceiling
const ISelectionGenerator::TSelectionLevel DEFAULT_P2C_CEILING = 2;


/*
 * CScoringGenerator: selection generator for strategy Scoring
 */
class CScoringGenerator : public ISelectionGenerator
{
public:

	/// Initialization of a selection cycle
	virtual void					init( TSelectionLevel nblevels );

	/// Change the number of levels without restarting the cycle
	virtual void					changeNbLevels( TSelectionLevel nblevels );

	/// Return the next level to select
	virtual TSelectionLevel			getNext();

	/// Display the scores (debugging)
	void							printScores();

private:

	/// Type of vector of level scores
	typedef std::vector<uint32> TLevelScores;

	/// Vector of level scores
	TLevelScores					_LevelScores;
};



#endif // NL_SELECTION_GENERATOR_H

/* End of selection_generator.h */
