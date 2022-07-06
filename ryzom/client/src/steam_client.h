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


#ifndef CL_STEAM_CLIENT_H
#define CL_STEAM_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/dynloadlib.h"

/**
 * Steam API helper to be able to call Steam functions/methods without linking to any library.
 * The library is dynamically loaded and is optional.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CSteamClient
{
public:
	CSteamClient();
	~CSteamClient();

	/**
	 *	Dynamically load Steam client library and functions pointers.
	 *	Also retrieve authentication session ticket if available.
	 *	If no authentication session ticket retrieved, returns false.
	 */
	bool init();

	/**
	 *	Shutdown Steam client and unload library.
	 */
	bool release();

	/**
	 *	Return the authentication session ticket if available.
	 */
	std::string getAuthSessionTicket() const { return _AuthSessionTicket; }

private:
	// handle on Steam DLL
	NLMISC::NL_LIB_HANDLE _Handle;

	// true if succeeded to initialize (must call shutdown)
	bool _Initialized;

	// the retrieved authentication session ticket
	std::string _AuthSessionTicket;
};

#endif
