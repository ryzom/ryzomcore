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


#ifndef EGS_PD_H
#define EGS_PD_H

#include <nel/misc/types_nl.h>
#include <pd_lib/pd_lib.h>

namespace EGSPD
{
	

//
// Global Forward Declarations
//

class CFameContainerEntryPD;
class CFameContainerPD;
class CGuildFameContainerPD;
class CGuildMemberPD;
class CGuildPD;
class CGuildContainerPD;
class CActiveStepStatePD;
class CActiveStepPD;
class CDoneStepPD;
class CMissionCompassPD;
class CMissionTeleportPD;
class CMissionInsidePlacePD;
class CMissionOutsidePlacePD;
class CHandledAIGroupPD;
class CMissionPD;
class CMissionGuildPD;
class CMissionTeamPD;
class CMissionSoloPD;
class CMissionContainerPD;

//

/// \name Public API for EGSPD database
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
	
} // End of EGSPD

//
// Includes
//

#include "common_pd.h"
#include "game_share/people_pd.h"
#include "game_share/sp_type.h"
#include "game_share/guild_grade.h"
#include "game_share/season.h"
#include "fame_pd.h"
#include "guild_member_pd.h"
#include "guild_pd.h"
#include "guild_container_pd.h"
#include "mission_pd.h"

#include "fame_pd_inline.h"
#include "guild_member_pd_inline.h"
#include "guild_pd_inline.h"
#include "guild_container_pd_inline.h"
#include "mission_pd_inline.h"

//


#include "egs_pd_inline.h"


#endif
