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



#ifndef RY_PACT_CLASS_H
#define RY_PACT_CLASS_H

#include "game_share/pact.h"

/**
 * CPact
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
struct CPact
{
	uint8	PactNature;
	uint8	PactType;

	CPact() { clear(); }
	CPact( uint8 Nature, uint8 Type ) { PactNature = Nature; PactType = Type; }

	void clear() { PactNature = GSPACT::Kamique; PactType = GSPACT::Type1; }

	bool operator == (const CPact &a) const
	{
		return (PactNature == a.PactNature && PactType == a.PactType );
	}

	bool operator != (const CPact &a) const
	{
		return (PactNature != a.PactNature || PactType != a.PactType );
	}


	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( PactNature );
		f.serial( PactType );
	}
};

#endif // RY_PACT_CLASS_H
/* pact_class.h */
