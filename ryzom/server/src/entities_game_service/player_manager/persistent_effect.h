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


#ifndef PERSISTENT_EFFECT_H
#define PERSISTENT_EFFECT_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// NeL Misc
#include "nel/misc/time_nl.h"

#include "game_share/persistent_data.h"

#include <vector>

//-------------------------------------------------------------------------------------------------
// forward decalarations
//-------------------------------------------------------------------------------------------------

class CCharacter;
class CSTimedEffect;

//-------------------------------------------------------------------------------------------------
// class CPersistentEffect
//-------------------------------------------------------------------------------------------------

/**
 *	CPersistentEffect
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2006
 */

class CPersistentEffect
{
public:
	DECLARE_VIRTUAL_PERSISTENCE_METHODS
	
	// default ctor
	CPersistentEffect(CCharacter* theCharacter);

	// dtor
	~CPersistentEffect();

	// copy persistent effect class in _PersitentEffects vector
	void collectPersistentEffect();

	// clear _PersistentEffects vector
	void clear();

	// activate all persistent effects
	void activate();

	// read/write specific method for store and apply persistent data
	void writePdr(CSTimedEffect * effect, CPersistentDataRecord& pdr) const;
	void readPdr(CPersistentDataRecord& pdr);

private:
	CCharacter *	_TheCharacter;
	// vector of effects must be persistent
	std::vector< NLMISC::CSmartPtr<CSTimedEffect> >	_PersistentEffects;
};


#endif // PERSISTENT_EFFECT_H
