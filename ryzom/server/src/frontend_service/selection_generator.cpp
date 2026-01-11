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


#include "selection_generator.h"
//#include <sstream>

using namespace std;


/*
 * Recursive function that generate the levels for CP2CGenerator
 * The output looks like the following:
 * 0 1 0 2 0 1 0 3 0 1 0 2 0 1 0 4 0 1 0 2 0 1 0 3 0 1 0 2 0 1 0
 * (every level has twice occurences as its lower level)
 */
void				CP2CGenerator::generateLevels( vector<TSelectionLevel>::iterator& iter, TSelectionLevel level )
{
	if ( level == 0 )
	{
		*iter= 0;
		++iter;
	}
	else
	{
		generateLevels( iter, level-1 );
		*iter = level;
		++iter;
		generateLevels( iter, level-1 );
	}

}


/*
 * Power of 2
 */
uint32 pow2( uint32 n )
{
	uint32 res = 1;
	res <<=  n;
	return res;
}



/*
 * Constructor
 */
CP2CGenerator::CP2CGenerator( TSelectionLevel ceiling ) :
	_CurrentLevelIndex(0), _Ceiling(ceiling), _NbLevels(0), _CurrentLowRank(0)
{
	// Generate levels
	_Ceiling = std::max( ceiling, (TSelectionLevel)1 );
	nldebug( "P2CGenerator: Generating levels..." );
	_LevelSequence.resize( pow2(_Ceiling+1)-1 );
	vector<TSelectionLevel>::iterator iter = _LevelSequence.begin();
	generateLevels( iter, _Ceiling );
	_LevelSequence.pop_back(); // avoid the last zero
}


/*
 * Initialization of a selection cycle
 */
void				CP2CGenerator::init( TSelectionLevel nblevels )
{
	_CurrentLevelIndex = 0;
	_NbLevels = nblevels;
	_CurrentLowRank = 0;
}


/*
 * Change the number of levels without restarting the cycle
 */
void				CP2CGenerator::changeNbLevels( TSelectionLevel nblevels )
{
	if ( nblevels < _LevelSequence[_CurrentLevelIndex] )
	{
		// The number of levels has been reduced to lower than the current level
		init( nblevels );
	}
	else
	{
		_NbLevels = nblevels;
	}
}


/*
 * Return the next level to select
 */
ISelectionGenerator::TSelectionLevel	CP2CGenerator::getNext()
{
	// Get next generated level
	TSelectionLevel level = _LevelSequence[_CurrentLevelIndex];
	_CurrentLevelIndex = (_CurrentLevelIndex + 1) % _LevelSequence.size();

	if ( level < _Ceiling )
	{
		// Case of power of 2
		return level;
	}
	else
	{
		// Equitable case beyond ceiling
		nlassertex( _NbLevels >= _Ceiling, ("%u %u", _NbLevels, _Ceiling) );
		uint32 rank = _CurrentLowRank;
		_CurrentLowRank = (_CurrentLowRank + 1 ) % ( _NbLevels - _Ceiling + 1 );
		return level + rank;
	}
}



/*
 * Initialization of a selection cycle
 */
void					CScoringGenerator::init( TSelectionLevel nblevels )
{
	// Do not set the scores to 0, they remain the same from one cycle to the next one
	changeNbLevels( nblevels );

	// TODO: handle possibility of score overflow
}


/*
 * Change the number of levels without restarting the cycle
 */
void					CScoringGenerator::changeNbLevels( TSelectionLevel nblevels )
{
	// Adjust vector size, and reset all scores to 0 if one level was added
	if ( nblevels > _LevelScores.size() )
	{
		fill( _LevelScores.begin(), _LevelScores.end(), 0 );
	}
	_LevelScores.resize( nblevels, 0 );
}


/*
 * Return the next level to select
 */
ISelectionGenerator::TSelectionLevel	CScoringGenerator::getNext()
{
	nlassert( ! _LevelScores.empty() );

	// Return the index of the smallest score and add index+1 to the min score
	TLevelScores::iterator iminscore = min_element( _LevelScores.begin(), _LevelScores.end() );
	uint32 index = (uint32)(iminscore - _LevelScores.begin());
	(*iminscore) += (index + 1);
	//printScores();
	return index;
}


/*
 * Display the scores (debugging)
 */
void					CScoringGenerator::printScores()
{
//	stringstream ss;
	string str;
	vector<uint32>::iterator ils;
	for ( ils=_LevelScores.begin(); ils!=_LevelScores.end(); ++ils )
	{
		//ss << " " << (int)(*ils);
		str += " " + NLMISC::toString((*ils));
	}
	nlinfo( "SCOG: Scores: %s", str.c_str() );
}

