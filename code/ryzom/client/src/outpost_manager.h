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

#ifndef CL_OUTPOST_MANAGER_H
#define CL_OUTPOST_MANAGER_H

#include "nel/misc/types_nl.h"
#include "game_share/outpost.h"


// ***************************************************************************
/**
 * Singleton for managing outpost
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class COutpostManager
{
private:
	// when this tick is reached, abort the proposal, and force player to be neutral
	uint32		_EndTickForPvpJoinProposal;

public:

	/// Constructor
	COutpostManager();

	/// Called when the server ask to join for PVP in a Outpost Zone
	void	startPvpJoinProposal(bool playerGuildInConflict, bool playerGuildIsAttacker,
		uint32 ownerGuildNameId, uint32 attackerGuildNameId, uint32 declTimer);

	/// Called when the client answer to the join for PVP in a Outpost Zone
	void	endPvpJoinProposal(bool bNeutral, OUTPOSTENUMS::TPVPSide pvpSide);

	/// Update the manager
	void	update();
};


// Easy Singleton
extern COutpostManager		OutpostManager;


#endif // NL_OUTPOST_MANAGER_H

/* End of outpost_manager.h */
