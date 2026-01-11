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
#error

#ifndef IOS_PD_H
#define IOS_PD_H

#include <nel/misc/types_nl.h>
#include <pd_lib/pd_lib.h>

namespace IOSPD
{
	

//
// Global Forward Declarations
//


//

/// \name Public API for IOSPD database
// @{

/**
 * Initialise the whole database engine.
 * Call this function at service init.
 */
void							init(uint32 overrideDbId);

/**
 * Tells if database engine is ready to work.
 * Engine may not be ready because PDS is down, not yet ready
 * or message queue to PDS is full.
 */
bool							ready();

/**
 * Update the database engine.
 * Call this method once per tick, only if engine is ready (see also ready() above).
 */
void							update();

/**
 * Logs chat sentence with sender and receipiants.
 */
void							logChat(const ucstring& sentence, const NLMISC::CEntityId& from, const std::vector<NLMISC::CEntityId>& to);

/**
 * Logs tell sentence with sender and single recipient (might be player or group).
 */
void							logTell(const ucstring& sentence, const NLMISC::CEntityId& from, const NLMISC::CEntityId& to);

/**
 * Release the whole database engine.
 * Call this function at service release.
 */
void							release();

// @}

extern RY_PDS::CPDSLib	PDSLib;
	
} // End of IOSPD

//
// Includes
//

#include "ios_chat_log.h"

#include "ios_chat_log_inline.h"

//


#include "ios_pd_inline.h"


#endif
