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



#ifndef NL_PAIR_SELECTOR_H
#define NL_PAIR_SELECTOR_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "fe_types.h"
#include "selection_generator.h"
#include <vector>
#include <functional>


/*
 * Forward declarations
 */
class CVisionArray;


/*
 * Items of TPairVector
 */
class TPairCE
{
public:
	// Constructor
	TPairCE( TClientId c, CLFECOMMON::TCLEntityId e ) : ClientId(c), CeId(e) {}

	// Copy constructor
	TPairCE( const TPairCE& src )
	{
		operator=( src );
	}

	// Assignment operator
	TPairCE& operator=( const TPairCE& src )
	{
		// no need to check ( &src != this )
		ClientId = src.ClientId;
		CeId = src.CeId;
		return *this;
	}

	// Comparison operator ==
	friend bool operator==( const TPairCE& e1, const TPairCE& e2 )
	{
		return (e1.ClientId == e2.ClientId) && (e1.CeId == e2.CeId );
	}

	// Classical comparison operator < (not needed: we compare by distance instead)
	/*friend bool operator<( const TPairCE& e1, const TPairCE& e2 )
	{
		if (e1.ClientId < e2.ClientId)
			return true;
		else if ( e1.ClientId == e2.ClientId )
			return (e1.CeId < e2.CeId );
		else
			return false;
	}*/

	TClientId				ClientId;
	CLFECOMMON::TCLEntityId CeId;
};


/*
 * Container for pairs sorted by distance
 */
typedef std::vector<TPairCE> TPairVector;


/*
 * Comparison function object for TPairCE
 */
struct TComparePairCE : public std::binary_function < const TPairCE&, const TPairCE&, bool >
{
	/// Constructor
	TComparePairCE( CVisionArray *visionarray ) :
		_VisionArray(visionarray)
		{}

	/// Comparison operator
	bool operator()( const TPairCE& e1, const TPairCE& e2 );

private:

	/// Access to vision array to know distances
	CVisionArray	*_VisionArray;
};


/*
 * Type of index to pair
 */
typedef sint32 TPairIndex;


/*
 * TPairRange: ranges in PairsByDistance, used by pseudosort algorithm
 */
struct TPairRange
{
	uint				IndexInRanges;
	TPairIndex			NumberOfElements;
	CLFECOMMON::TCoord	DistThreshold;

	// The "begin index" of the range depends on the previous ranges
	// (prevents from updating following ranges every time an index boundary changes)
	TPairIndex beginIndex( const std::vector<TPairRange>& vect ) const
	{
		uint i;
		TPairIndex sum = 0;
		for ( i=0; i!=IndexInRanges; ++i )
		{
			sum += vect[i].NumberOfElements;
		}
		return sum;
	}
};


/*
 * Vector of ranges
 */
typedef std::vector<TPairRange> TRanges;



/**
 * Pair Selector.
 * It selects the pairs <Client,Entity> for which the priorities of the properties
 * will be calculated. The pairs are stored in a vector and this one is sorted or
 * pseudosorted (depending on its size) by distance.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPairSelector
{
public:

	/// Type of selection strategy
	enum TSelectionStrategy { Power2WithCeiling = 0, Scoring = 1 };

	/// Constructor
	CPairSelector();

	/// Initialization
	void				init( CVisionArray *va );

						// _______________________
						// For the vision provider

	/// Add to the container
	void				addPair( TClientId clientid, CLFECOMMON::TCLEntityId ceid, CLFECOMMON::TCoord distance );

	/// Remove from the container
	void				removePair( TClientId clientid, CLFECOMMON::TCLEntityId ceid );

	/// Remove from the container all pairs with the specified clientid
	void				removeAllPairs( TClientId clientid );

						// _______________________________
						// For the priority subsystem root

	/// Return true if there is no pair
	bool				empty() { return _PairsByDistance.empty(); }

	/// Sort or pseudosort the pairs, and initialize selection cycle. Must be called before a series of selectNextPair().
	void				sortPairs();

	/// Set the selection strategy
	void				setSelectionStrategy( TSelectionStrategy strat );

						// ___________________
						// For the prioritizer

	/// Select the next pair to prioritize, or return NULL if there is no more pairs for this cycle
	const TPairCE		*selectNextPair();

						// ______________
						// For statistics

	/// Return the number of pairs in the container
	TPairIndex			nbPairs() const { return _PairsByDistance.size(); }

	/// Print the scores if there is a scoring selection generator
	void				printScores() const;

	/// Display the ranges (debugging)
	void				printRanges( bool checkIntegrity ) const;

	/// Display the pairs (debugging)
	void				printPairs() const;

	/// Pseudosort or sort (read)
	bool				PseudoSort;

	/// Selection strategy (read)
	TSelectionStrategy	SelStrategy;

	/// Average number of pairs per range (read/write)
	TPairIndex			NbPairsPerRange;

	/// Number of pairs to select per cycle (read)
	TPairIndex			NbPairsToSelect;

	/// Min number of pairs to select per cycle (read/write)
	TPairIndex			MinNbPairsToSelect;

protected:

	/// Easy access to the last pair
	TPairCE&			lastPair() { nlassert( ! _PairsByDistance.empty() ); return _PairsByDistance.back(); }

	/// Return the number of ranges for pseudosort
	uint32				nbRanges() const { return _Ranges.size(); }

	/// Easy access to the last range for pseudosort
	TPairRange&			lastRange() { nlassert( ! _Ranges.empty() ); return _Ranges.back(); }

	/// Easy access to the last range for pseudosort (const version)
	const TPairRange&	lastRange() const { nlassert( ! _Ranges.empty() ); return _Ranges.back(); }

	// Update ranges after deleting a pair
	void				adjustRangesAfterDeleting();

	/// Add a range for pseudosort, and resample the distance thresholds.
	void				addRange();

	/// Remove the last range (for pseudosort) when it's empty, and resample the distance thresholds.
	void				removeLastRange();

	/// Setup selection generator, using SelStrategy (the argument is meaningful only in strategy P2C)
	void				setupSelectionGenerator( ISelectionGenerator::TSelectionLevel ceiling );

	/// Select next level. Call selectNextPair() after selectNextLevel().
	void				selectNextLevel();

	/// Calculate NbPairsToSelect taking into account the user loop time
	void				calcNbPairsToSelect();

	/// Return the number of levels corresponding to the current size of _PairsByDistance
	uint				getNbLevels() { return (_PairsByDistance.size() / NbPairsToSelect) + 1; }

private:

	/// Pairs sorted by distance
	TPairVector			_PairsByDistance;

	/// Ranges used by pseudosort algorithm
	TRanges				_Ranges;

	/// Index of the current range for pseudosorting
	uint32				_CurrentRangeIndex;

	/// Selection generator (either CP2CGenerator or CScoringGenerator)
	ISelectionGenerator	*_SelGenerator;

	/// Beginning index of the current level given by the selection generator
	TPairIndex			_BeginIndex;

	/// Index of currently selected pair
	TPairIndex			_NextPairIndex;

	/// Difference between previous and current NbPairsToSelect;
	sint32				_NPSDelta;

	/// Vision Array
	CVisionArray		*_VisionArray;
};


#endif // NL_PAIR_SELECTOR_H

/* End of pair_selector.h */
