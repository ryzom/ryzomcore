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



#ifndef NL_PRIO_SUB_H
#define NL_PRIO_SUB_H

#include <nel/misc/types_nl.h>
#include "vision_array.h"
#include "vision_provider.h"
#include "distance_prioritizer.h"

#include <nel/misc/hierarchical_timer.h>

/**
 * Priority Subsystem.
 * You can call update for a whole cycle or call each task independantly
 * (for example, to allow multithreading).
 * One cycle consists of the following tasks:
 * #1. processVision()
 * #2. preparePairSelection()
 * #3. calculatePriorities()
 * #4. initDispatcherCycle()
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPrioSub
{
public:

	/// Constructor
	CPrioSub();

	/// Main initialization
	void					init( CHistory *h, CClientIdLookup *cl )
							{
								VisionProvider.init( &VisionArray, h, cl );
								Prioritizer.init( &VisionArray, &VisionProvider, h );
							}

	/// Perform one cycle (PropDispatcher becomes ready for getNextParcel())
	void					update()
							{
								H_TIME(ProcessVision, processVision(););
								H_TIME(calculatePriorities, calculatePriorities(););
							}

	/// Process Vision Differences
	void					processVision()
							{ VisionProvider.processVision(); }

	/// Calculate the priorities corresponding to the selected pairs
	void					calculatePriorities()
							{ Prioritizer.calculatePriorities(); }

	/// Unit testing (TEMP)
	void					testVisionProvider();

public:

	/// Entities seen by the clients and the priorities of their properties
	CVisionArray			VisionArray;

	/// Manager of the vision (who sees who)
	CVisionProvider			VisionProvider;

	/// Priority calculation
	CDistancePrioritizer	Prioritizer;

private:

	/// Counter for adjustHPThreshold
	//uint32					_AHPTCounter;

};


#endif // NL_PRIO_SUB_H

/* End of prio_sub.h */
