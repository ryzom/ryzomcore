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



#ifndef NL_PRIORITIZER_H
#define NL_PRIORITIZER_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "fe_types.h"
#include "processing_spreader.h"


/*
 * Forward declarations
 */
class CPairSelector;
class CVisionArray;
class CPropertyDispatcher;
class CPropertyIdTranslator;
class CHistory;
class TPairCE;


/**
 * Prioritizer.
 * It knows how to calculate the priority of pairs <Client, Entity>.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPrioritizer
{
public:

	/// Type of priority strategy
	enum TPrioStrategy { DistanceOnly = 0, DistanceDelta = 1 };

	/// Constructor
	CPrioritizer();

	/// Initialization
	void					init( CVisionArray *va,
								  CPropertyIdTranslator* pt, CHistory *h );

	/// Calculate the priorities
	void					calculatePriorities();

	CLFECOMMON::TCoord		getDeltaPos( TEntityIndex entityindex, TClientId clientid, CLFECOMMON::TCLEntityId ceid );

	CLFECOMMON::TCoord		getDeltaOrientation( TEntityIndex entityindex, TClientId clientid, CLFECOMMON::TCLEntityId ceid );

	/// Return the priority strategy
	TPrioStrategy			prioStrategy() const { return _PrioStrategy; }
	
	// Set the priority strategy
	//void					setPrioStrategy( TPrioStrategy ps );

	// Constants (obsolete)
	/*
	static const TCoord		DeltaMinThreshold;
	static const TCoord		DeltaMaxThreshold;
	static const TCoord		DistMinThreshold;
	static const TCoord		DistMaxThreshold;
	*/
	static const CLFECOMMON::TCoord	MaxDelta;

	CProcessingSpreader		PositionSpreader, OrientationSpreader, DiscreetSpreader;

protected:

	/// Calculate the priorities of position for the current cycle
	void					calculatePriorityOfPosition();

	/// Calculate the priorities of orientation for the current cycle
	void					calculatePriorityOfOrientation();

	/// Calculate the priorities of discreet props for the current cycle
	void					calculatePriorityOfDiscreet();

private:

	/// Vision Array
	CVisionArray			*_VisionArray;

	/// Property Id Translator
	CPropertyIdTranslator	*_PropTranslator;

	/// History
	CHistory				*_History;

	/// Strategy
	TPrioStrategy			_PrioStrategy;

};


#endif // NL_PRIORITIZER_H

/* End of prioritizer.h */
