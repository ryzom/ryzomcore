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



#ifndef RY_SABRINA_PHRASE_MANAGER_H
#define RY_SABRINA_PHRASE_MANAGER_H

// stl
#include <vector>
// misc
#include "nel/misc/types_nl.h"
// sabrina
#include "sabrina_phrase_instance.h"
#include "sabrina_pointers.h"


/**
 * The Sabrina phrase manager singleton
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSabrinaPhraseManager
{	
public:
	// singleton init - called in service init
	static void init();

	// update called each tick in service update
	// updates phrases and dispatches events to AI
	static void update();

	// singleton release - called in service release
	static void release();

	// activate a phrase & setup the next event time (time should be >=0 and <256)
	static void setNextPhraseEvent(CSabrinaPhraseInstance* phrase,NLMISC::TGameCycle nextEventTime);

private:
	// this is a singleton so prohibit construction
	CSabrinaPhraseManager();

	typedef std::vector<CSabrinaPhraseInstancePtr> TPhraseVector;
	static TPhraseVector _PhraseVectors[256];
};


#endif 
