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
#include "roles.h"
#include "nel/misc/i18n.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NLMISC;

namespace ROLES
{

// for convert enum to part of role definition sheet name
static const string StringSheetArray[NB_ROLES]=
{
	"Unknown",
	"Fighter",
	"Caster",
	"Crafter",
	"Harvester",
};

// for convert enum to string role name
static const string StringArray[NB_ROLES]=
{
	"Unknown",
	"Fighter",
	"Caster",
	"Crafter",
	"Harvester",
};

// convert role id to sheet string
const std::string& toSheetString( ERole RoleId )
{
	nlassert((sint)RoleId<NB_ROLES);
	return StringSheetArray[RoleId];
}

// convert role id to role name string
const std::string& toString( ERole RoleId )
{
	nlassert((sint)RoleId<NB_ROLES);
	return StringArray[RoleId];
}

// convert role name string to role
ERole toRoleId( const std::string& Role )
{
	for(uint i=0;i<NB_ROLES;i++)
	{
		if(nlstricmp(StringArray[i], Role)==0)
			return (ERole)i;
	}
	return unknown;
}

// Return the Translated name of the Job
const ucstring &roleToUCString (ERole r)
{
	return NLMISC::CI18N::get( toString( r ) );
}

} // ROLES
