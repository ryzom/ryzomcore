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



#ifndef NL_PROCESSING_SPREADER_H
#define NL_PROCESSING_SPREADER_H

#include "nel/misc/types_nl.h"
#include "fe_types.h"

// Test with the least frequent period even at low load (define=yes/undef=no)
#undef TEST_WORSE_EXECUTION_PERIOD


/**
 * This class is aimed to spread an iterating process onto several game cycles,
 * when it is too long to be executed in one game cycle.
 * When mustProcessNow() returns true, call getProcessingBounds() to get the
 * range, in the client map, of clients to process.
 * After processing them, call endProcessing().
 * After any cycle, call incCycle().
 * When a client leaves the front-end, call notifyClientRemoval().
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CProcessingSpreader
{
public:

	/// Constructor
	CProcessingSpreader();

	/// Initialization
	void					init();

	/// Execution period in game cycle
	sint32					ExecutionPeriod;

	/// Return true if the process must be executed at the current cycle
	bool					mustProcessNow()
	{
#ifdef TEST_WORSE_EXECUTION_PERIOD
		return (_Count == 0);
#else
		return true;
#endif
	}

	/// Get the range (beginning iterator, index and outer bound) for the current processing
	void					getProcessingBounds( THostMap::iterator& firstit, sint& firstindex, sint& outerboundindex );

	/// Method to call at the end of a processing
	void					endProcessing( THostMap::iterator icm )
	{
		_ClientMapIterator = icm;
#ifdef TEST_WORSE_EXECUTION_PERIOD
		_Count = ExecutionPeriod;
#endif
	}

	/// Method to call every cycle, possibly after processing
	void					incCycle()
	{
#ifdef TEST_WORSE_EXECUTION_PERIOD
		--_Count;
#endif
	}

	/// Method to call after a new client is added into the clientmap (give it clientmap.end() before the addition!)
	void					notifyClientAddition( THostMap::iterator endBeforeAddition );

	/// Method to call when a client leaves
	void					notifyClientRemoval( THostMap::iterator icm );
	
private:

#ifdef TEST_WORSE_EXECUTION_PERIOD
	/// Execution counter
	sint32					_Count;
#endif

	/// Current map index
	sint32					_ClientMapIndex;

	/// Current map iterator
	THostMap::iterator		_ClientMapIterator;

	/// True if ClientMapIterator is invalid
	bool					_Invalidated;
};


#endif // NL_PROCESSING_SPREADER_H

/* End of processing_spreader.h */
