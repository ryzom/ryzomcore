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

#include "pair_selector.h"
#include "prio_sub.h"
#include "frontend_service.h"

using namespace std;
using namespace CLFECOMMON;


/*
 * Comparison operator by distance
 */
bool TComparePairCE::operator()( const TPairCE& e1, const TPairCE& e2 )
{
	return ( _VisionArray->distanceCE( e1.ClientId, e1.CeId )
		   < _VisionArray->distanceCE( e2.ClientId, e2.CeId ) );
}

  
/*
 * Constructor
 */
CPairSelector::CPairSelector() :
	PseudoSort( false ),
	SelStrategy( Scoring ),
	NbPairsPerRange( 2000 ),
	NbPairsToSelect( 2000 ),
	MinNbPairsToSelect( 2000 ),
	_CurrentRangeIndex( 0 ),
	_BeginIndex( 0 ),
	_NextPairIndex( -1 ),
	_NPSDelta( 0 ),
	_SelGenerator( NULL ),
	_VisionArray( NULL )
{
}


/*
 * Initialization
 */
void				CPairSelector::init( CVisionArray *va )
{
	_VisionArray = va;

	// Create first range
	TPairRange newrange;
	newrange.IndexInRanges = 0;
	newrange.NumberOfElements = 0;
	newrange.DistThreshold = CFrontEndService::instance()->VisibilityDistance;
	_Ranges.push_back( newrange );
}


/*
 * Add to the selector vector (TODO: the distance could be used for insertion)
 */
void				CPairSelector::addPair( TClientId clientid, TCLEntityId ceid, TCoord distance )
{
	// Add to the vector
	_PairsByDistance.push_back( TPairCE( clientid, ceid ) );

	// Update ranges
	if ( (TPairIndex)_PairsByDistance.size() == lastRange().beginIndex( _Ranges ) + NbPairsPerRange )
	{
		// Add one range
		addRange();
	}
	else
	{
		// Update last range (TODO: insert into the good range)
		++lastRange().NumberOfElements;
	}
}


/*
 * Remove from the container
 */
void				CPairSelector::removePair( TClientId clientid, TCLEntityId ceid )
{
	// Find the pair in the vector
	// (TODO: evaluate the need of storing an index in VisionArray, faster find but slower sort)
	TPairVector::iterator ip = find( _PairsByDistance.begin(), _PairsByDistance.end(), TPairCE( clientid, ceid ) );
	nlassert( ip != _PairsByDistance.end() );

	// Remove it
	if ( ip != _PairsByDistance.end()-1)
	{
		*ip = _PairsByDistance.back();
	}
	_PairsByDistance.pop_back();

	// Adjust last range for pseudosorting
	adjustRangesAfterDeleting();
}


/*
 * Remove from the container all pairs with the specified clientid
 */
void				CPairSelector::removeAllPairs( TClientId clientid )
{
	sint nbremoved = 0;
	TPairVector::iterator ip = _PairsByDistance.begin();
	while ( ip < _PairsByDistance.end() )
	{
		// Find matching pair
		if ( (*ip).ClientId == clientid )
		{
			// Remove it (replace it by the last one)
			if ( ip != _PairsByDistance.end()-1 )
			{
				*ip = _PairsByDistance.back();
			}
			_PairsByDistance.pop_back();
			++nbremoved;
		}
		else
		{
			++ip;
		}
	}

	// Adjust ranges
	sint i;
	for ( i=0; i!=nbremoved; ++i )
	{
		adjustRangesAfterDeleting();
	}
}


/*
 * Update ranges after deleting a pair
 */
void				CPairSelector::adjustRangesAfterDeleting()
{
	TPairRange *lastrange = &lastRange();
	if ( nbRanges() == 1 )
	{
		// Decrement size of last range
		nlassert( lastrange->NumberOfElements != 0 );
		--lastrange->NumberOfElements;
	}
	else
	{
		// Remove last range if empty
		while ( (lastrange->NumberOfElements == 0 ) && (nbRanges() != 1) )
		{
			removeLastRange();
			lastrange = &lastRange();
		}

		// Decrement size of last range
		nlassert( lastrange->NumberOfElements != 0 );
		--lastrange->NumberOfElements;
	}
}


/*
 * Sort or pseudosort the pairs, and initialize selection cycle.
 * Must be called before a series of selectNextPair().
 */
void				CPairSelector::sortPairs()
{
	if ( empty() )
		return;
	
	// Sort or pseudosort
	if ( nbPairs() > NbPairsPerRange )
	{
		// Pseudosort
		PseudoSort = true;

		// Get current range
		++_CurrentRangeIndex;
		if ( _CurrentRangeIndex >= nbRanges() )
		{
			_CurrentRangeIndex = 0;
		}
		TPairRange& currentrange = _Ranges[_CurrentRangeIndex];
		nlassert( _CurrentRangeIndex == currentrange.IndexInRanges );

		// Get thresholds and boundaries
		uint32 nbraised = 0;
		uint32 nblowered = 0;
		TCoord minDistThreshold = (currentrange.IndexInRanges != 0) ? _Ranges[_CurrentRangeIndex-1].DistThreshold : 0;
		TCoord maxDistThreshold = (currentrange.IndexInRanges != nbRanges()-1) ? currentrange.DistThreshold : MAX_COORD;
		TPairIndex beginIndex = currentrange.beginIndex( _Ranges );
		TPairIndex endIndex = beginIndex + currentrange.NumberOfElements;
		nlassert( endIndex <= nbPairs() );
		//comment Ben nlinfo( "FEPRIO: Pseudosorting %u elements from %u to %u of range %u (%u couples)...", currentrange.NumberOfElements, beginIndex, endIndex, currentrange.IndexInRanges, nbPairs() );

		// Test every pair with the thresholds to know if they need to be raised or lowered
		TPairIndex index;
		for ( index = beginIndex; index < endIndex; )
		{
			TPairCE &entry = _PairsByDistance[index];
			TCoord d = _VisionArray->distanceCE( entry.ClientId, entry.CeId );

			if (d < minDistThreshold)
			{
				// Raise
				if (index != beginIndex)
					swap( _PairsByDistance[beginIndex], _PairsByDistance[index] );

				++beginIndex;
				++index;
				++nbraised;
			}
			else if (d > maxDistThreshold)
			{
				// Lower
				if (index != endIndex-1)
					swap( _PairsByDistance[endIndex-1], _PairsByDistance[index] );

				--endIndex;
				++nblowered;
			}
			else
			{
				// The pair remains where it is
				++index;
			}
		}

		// Adjust threshold if there are too many elements in the current range
		if ( currentrange.NumberOfElements > NbPairsPerRange + NbPairsPerRange/2 )
		{
			if ( _CurrentRangeIndex == 0 )
			{
				currentrange.DistThreshold /= 2;
			}
			else
			{
				currentrange.DistThreshold = (_Ranges[_CurrentRangeIndex-1].DistThreshold + currentrange.DistThreshold) / 2;
			}
		}


		// The number of removed elements is equal to 
		// the number of elements raised plus the number of elements lowered
		currentrange.NumberOfElements -= (nbraised+nblowered);
		if (currentrange.IndexInRanges != 0)
			_Ranges[_CurrentRangeIndex-1].NumberOfElements += nbraised;
		if (currentrange.IndexInRanges != nbRanges()-1)
			_Ranges[_CurrentRangeIndex+1].NumberOfElements += nblowered;
	}
	else
	{
		// Sort
		PseudoSort = false;
		//comment Ben nlinfo( "FEPRIO: Sorting %u elements...", _PairsByDistance.size() );
		std::sort( _PairsByDistance.begin(), _PairsByDistance.end(), TComparePairCE( _VisionArray ) );
	}

	// Initialize selection cycle
	calcNbPairsToSelect();
}


/*
 * Select the next pair to prioritize, or return NULL if no more pairs for this cycle
 */
const TPairCE		*CPairSelector::selectNextPair()
{
	//nlassert( ! _PairsByDistance.empty() );

	// Set new index
	++_NextPairIndex;
	if ( _NextPairIndex >= nbPairs() )
	{
		// End of table, prepare falling back to beginning for next cycle
		_SelGenerator->init( getNbLevels() );
		selectNextLevel();
		return NULL;
	}
	else if ( _NextPairIndex >= _BeginIndex + NbPairsToSelect )
	{
		// Prepare next level for next cycle
		selectNextLevel();
		return NULL;
	}

	return &_PairsByDistance[_NextPairIndex];
}


/*
 * Add a range for pseudosort, and resample the distance thresholds.
 */
void				CPairSelector::addRange()
{
	// Add one range
	TPairRange& range = lastRange();
	TPairRange newrange;
	newrange.IndexInRanges = range.IndexInRanges + 1;
	newrange.NumberOfElements = 1;
	newrange.DistThreshold = CFrontEndService::instance()->VisibilityDistance;
	nlinfo( "FEPRIO: Adding range %u, index %u, threshold %d", _Ranges.size(), newrange.IndexInRanges, newrange.DistThreshold );
	_Ranges.push_back( newrange );

	// Resample distance thresholds
	uint32 nbranges = nbRanges();
	nlassert( nbranges > 1 ); // size is 1 at initialization
	float resampleratio = (float)(nbranges-1) / (float)nbranges;
	uint i;
	for ( i=0; i!=nbranges-1; ++i )
	{
		_Ranges[i].DistThreshold = (TCoord)((float)_Ranges[i].DistThreshold * resampleratio);
	}

	if ( nbRanges() > 30 )
	{
		nlwarning( "More than 3 seconds will be needed to pseudosort all ranges" );
	}

	printRanges( false );
	nlassertex( lastRange().beginIndex( _Ranges ) + lastRange().NumberOfElements == nbPairs(), ("Total size: %u", nbPairs()) );
}


/*
 * Remove the last range (for pseudosort) when it's empty, and resample the distance thresholds.
 */
void				CPairSelector::removeLastRange()
{
	// Delete last range (if there are more than 1 range)
	nlassert( (nbRanges() > 1) && (lastRange().NumberOfElements == 0) );
	nlinfo( "FEPRIO: Removing last range %u", nbRanges() );
	_Ranges.pop_back();

	// Resample distance thresholds
	uint32 nbranges = nbRanges();
	float resampleratio = (float)(nbranges+1) / (float)(nbranges);
	uint i;
	for ( i=0; i!=nbranges; ++i )
	{
		// TODO: check the cast is good (float precision)
		_Ranges[i].DistThreshold = (TCoord)((float)_Ranges[i].DistThreshold * resampleratio);
	}

	printRanges( false );
}


/*
 * Set the selection strategy
 */
void				CPairSelector::setSelectionStrategy( TSelectionStrategy strat )
{
	SelStrategy = strat;
	setupSelectionGenerator( DEFAULT_P2C_CEILING );
}


/*
 * Setup selection generator, using SelStrategy
 */
void				CPairSelector::setupSelectionGenerator( ISelectionGenerator::TSelectionLevel ceiling )
{
	// Delete existing generator if already created
	if ( _SelGenerator )
	{
		delete _SelGenerator;
	}

	// Create new generator
	switch ( SelStrategy )
	{
	case Power2WithCeiling:
		_SelGenerator = new CP2CGenerator( ceiling );
		break;
	case Scoring:
		_SelGenerator = new CScoringGenerator();
		break;
	default:
		nlstop;
	}

	// Init generator
	_SelGenerator->init( getNbLevels() );
}


/*
 * Select next level. Call selectNextPair() after selectNextLevel().
 */
void				CPairSelector::selectNextLevel()
{
	_BeginIndex = _SelGenerator->getNext() * NbPairsToSelect;
	_NextPairIndex = _BeginIndex - 1; // next increment will advance to the begin index value

	//comment Ben nlinfo( "FEPRIO: Changed selection level to [ %u , %u ]", _BeginIndex, _BeginIndex + NbPairsToSelect );
}


/*
 * Calculate NbPairsToSelect
 */
void				CPairSelector::calcNbPairsToSelect()
{
	/*
	 * Calculate NbPairsToSelect taking into account the user loop time
	 *
	 * Note: instead of taking MinNbPairsToSelect as the minimum value, we should
	 * take into account the number of clients x the number of actions per client, otherwise
	 * some messages may be not entirely filled.
	 */
	uint32 nroldvalue = NbPairsToSelect;
	if ( (CFrontEndService::instance()->UserDurationPAverage > 100) && (NbPairsToSelect > MinNbPairsToSelect) )
	{
		// Decrease the number of pairs to select, to speed up the process
		if ( _NPSDelta > 0 )
		{
			NbPairsToSelect = max( (TPairIndex)(NbPairsToSelect - _NPSDelta/2), (TPairIndex)MinNbPairsToSelect );
		}
		else
		{
			NbPairsToSelect = max( (TPairIndex)(NbPairsToSelect / 2), (TPairIndex)MinNbPairsToSelect );
		}
		//comment Ben nlinfo( "FEPRIO: NbRowsToComputePerCycle is now %u (-), %u", NbPairsToSelect, CFrontEndService::instance()->UserDurationPAverage );
	}
	else if ( (CFrontEndService::instance()->UserDurationPAverage < 10) && (NbPairsToSelect < nbPairs()) )
	{
		// Increase the number of pairs to select, because there is some CPU time available
		if ( _NPSDelta < 0 )
		{
			NbPairsToSelect = min( (TPairIndex)(NbPairsToSelect - _NPSDelta/2), nbPairs() );
		}
		else
		{
			NbPairsToSelect = min( (TPairIndex)(NbPairsToSelect * 2), nbPairs() );
		}
		//comment Ben nlinfo( "FEPRIO: NbRowsToComputePerCycle is now %u (+)", NbPairsToSelect );
	}
	_NPSDelta = NbPairsToSelect - nroldvalue;

	// Update the selection generator
	_SelGenerator->changeNbLevels( getNbLevels() );
}


/*
 * Display the ranges (debugging)
 */
void				CPairSelector::printRanges( bool checkIntegrity ) const
{
	TRanges::const_iterator irange;
	for ( irange=_Ranges.begin(); irange!=_Ranges.end(); ++irange )
	{
		nlinfo( "FEPRIO: Range %u: %u elements, threshold %d", (*irange).IndexInRanges, (*irange).NumberOfElements, (*irange).DistThreshold );
	}
	if ( ! _Ranges.empty() )
	{
		nlinfo( "FEPRIO: Total pairs: %u", nbPairs() );
		if ( checkIntegrity )
		{
			nlassert( lastRange().beginIndex( _Ranges ) + lastRange().NumberOfElements == nbPairs() );
		}
	}
}


/*
 * Display the pairs (debugging)
 */
void				CPairSelector::printPairs() const
{
	TPairVector::const_iterator ipv;
//	stringstream ss;
//	ss << endl;
	string str = "\n";
	sint i = 0;
	for ( ipv=_PairsByDistance.begin(); ipv!=_PairsByDistance.end(); ++ipv )
	{
		//ss << i << ": Client " << (*ipv).ClientId << " - Slot " << (*ipv).CeId << " - Distance " << _VisionArray->distanceCE( (*ipv).ClientId,(*ipv).CeId ) << endl;
		str += NLMISC::toString(i) + ": Client " + NLMISC::toString((*ipv).ClientId) + " - Slot " + NLMISC::toString((*ipv).CeId) + " - Distance " + NLMISC::toString(_VisionArray->distanceCE( (*ipv).ClientId,(*ipv).CeId )) + "\n";
		++i;
	}
	nlinfo( "%s", str.c_str() );
}


/*
 *  Print the scores if there is a scoring selection generator
 */
void				CPairSelector::printScores() const
{
	CScoringGenerator* sg = dynamic_cast<CScoringGenerator*>(_SelGenerator);
	if ( sg )
		sg->printScores();
	else
		nlwarning( "No scoring generator engaged" );
}



NLMISC_COMMAND( displayPairSelectorRanges, "Print the ranges of the pair selector", "[<checkIntegrity>]" )
{
	bool check = false;
	if ( (args.size() > 0) && (atoi(args[0].c_str()) != 0) )
	{
		check = true;
	}

	CFrontEndService::instance()->PrioSub.PairSelector.printRanges( check, &log );
	return true;
}


NLMISC_COMMAND( displayPairSelector, "Print the pairs sorted by distance", "" )
{
	CFrontEndService::instance()->PrioSub.PairSelector.printPairs(&log);
	return true;
}


NLMISC_COMMAND( displayGeneratorScores, "Print the scores of the scoring generator", "" )
{
	CFrontEndService::instance()->PrioSub.PairSelector.printScores(&log);
	return true;
}

NLMISC_COMMAND( setNbpairsPerRange, "Set the number of pairs per range in the selector", "<nb>" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	
	// get the values
	TPairIndex nb = atoi(args[0].c_str());

	CFrontEndService::instance()->PrioSub.PairSelector.NbPairsPerRange = nb;
	return true;
}

NLMISC_COMMAND( getNbpairsPerRange, "Get the number of pairs per range in the selector", "" )
{
	log.displayNL( "%d", CFrontEndService::instance()->PrioSub.PairSelector.NbPairsPerRange );
	return true;
}

NLMISC_COMMAND( setMinNbpairsToSelect, "Set the minimum number of pairs to select", "<nb>" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	
	// get the values
	TPairIndex nb = atoi(args[0].c_str());

	CFrontEndService::instance()->PrioSub.PairSelector.MinNbPairsToSelect = nb;
	return true;
}

NLMISC_COMMAND( getNbpairsToSelect, "Get the number of pairs to select and the minium", "" )
{
	log.displayNL( "min=%d current=%d", CFrontEndService::instance()->PrioSub.PairSelector.MinNbPairsToSelect,
					CFrontEndService::instance()->PrioSub.PairSelector.NbPairsToSelect );
	return true;
}


NLMISC_DYNVARIABLE( uint32, NbPairs, "Number of pairs" )
{
	// We can only read the value
	if ( get )
		*pointer = CFrontEndService::instance()->PrioSub.PairSelector.nbPairs();
}

