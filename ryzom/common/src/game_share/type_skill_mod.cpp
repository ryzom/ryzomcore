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
#include "type_skill_mod.h"


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


// ----------------------------------------------------------------------------
// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of persistent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF

//-----------------------------------------------------------------------------
// Persistent data for CTypeSkillMod
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CTypeSkillMod

#define PERSISTENT_PRE_STORE\
	H_AUTO(CTypeSkillModStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CTypeSkillModApply);\

#define PERSISTENT_DATA\
	PROP(sint32, Modifier)\
	PROP2(Type,	std::string,	EGSPD::CClassificationType::toString(Type),	Type=EGSPD::CClassificationType::fromString(val))\

	//#pragma message( PERSISTENT_GENERATION_MESSAGE )

#include "persistent_data_template.h"