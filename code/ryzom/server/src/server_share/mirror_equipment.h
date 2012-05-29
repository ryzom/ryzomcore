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



#ifndef RY_MIRROR_EQUIPMENT_H
#define RY_MIRROR_EQUIPMENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/sheet_id.h"
#include "game_share/persistent_data.h"
//


/**
 * SMirrorEquipment
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class SMirrorEquipment
{
public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	NLMISC::CSheetId	IdSheet;
	uint16				Quality;

	// Default contructor
	SMirrorEquipment() { IdSheet = 0; Quality = 0; }
	SMirrorEquipment( const NLMISC::CSheetId& id, uint16 Q ) { IdSheet = id; Quality = Q; }

	// operators
	const SMirrorEquipment &operator = (const SMirrorEquipment &e)
	{
		IdSheet = e.IdSheet;
		Quality = e.Quality;
		return *this;
	}

	virtual bool operator == (const SMirrorEquipment &e) const
	{
		return ( IdSheet == e.IdSheet && Quality == e.Quality);
	}

	virtual bool operator != (const SMirrorEquipment &e) const
	{
		return ( IdSheet != e.IdSheet || Quality != e.Quality);
	}

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( IdSheet );
		f.serial( Quality );
	}
};

namespace NLMISC
{
	std::string toString( const SMirrorEquipment& );
};

#endif // RY_MIRROR_EQUIPMENT_H
/* End of mirror_equipment.h */

