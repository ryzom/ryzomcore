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



#ifndef RY_TYPE_SKILL_MOD_H
#define RY_TYPE_SKILL_MOD_H

#include "nel/misc/types_nl.h"
#include "people.h"
#include "characteristics.h"
#include "persistent_data.h"

struct CTypeSkillMod
{
	CTypeSkillMod() : Modifier(0),Type(EGSPD::CClassificationType::Unknown)
	{}

	void serial(class NLMISC::IStream &f)
	{
		f.serialEnum( Type );
		f.serial( Modifier );
	}

	sint32 Modifier;
	EGSPD::CClassificationType::TClassificationType Type;

	DECLARE_PERSISTENCE_METHODS
};


#endif // RY_TYPE_SKILL_MOD_H

/* End of type_skill_mod.h */
