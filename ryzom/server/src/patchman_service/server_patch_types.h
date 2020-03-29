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

#ifndef SERVER_PATCH_TYPES_H
#define SERVER_PATCH_TYPES_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"


namespace PATCHMAN
{
	//-----------------------------------------------------------------------------
	// handy type definitions
	//-----------------------------------------------------------------------------

	// A map of module ids to module states
	typedef std::map<NLMISC::CSString,NLMISC::CSString> TModuleStates;

	// A map of domain names to domain info records (containing version numbers for the domain)
	struct SDomainInfo
	{
		uint32 LaunchVersion;
		uint32 InstallVersion;

		SDomainInfo()
		{
			LaunchVersion= ~0u;
			InstallVersion= ~0u;
		}
	};
	typedef std::map<NLMISC::CSString,SDomainInfo> TDomains;

	// A map of version names to version info records (containing paired version numbers for the server and client)
	struct SNamedVersionInfo
	{
		uint32 ServerVersion;
		uint32 ClientVersion;

		SNamedVersionInfo()
		{
			ServerVersion= ~0u;
			ClientVersion= ~0u;
		}
	};
	typedef std::map<NLMISC::CSString,SNamedVersionInfo> TNamedVersions;
}

//-----------------------------------------------------------------------------
#endif
