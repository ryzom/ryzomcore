/** \file network.h
 * Network interface between the game and NeL
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NETWORK_H
#define NETWORK_H

//
// Includes
//

#include <string>

#include <nel/misc/time_nl.h>

#include "entities.h"

//
// External definitions
//

namespace NLMISC
{
	class CVector;
}

namespace NLNET
{
	class CCallbackClient;
}

namespace SBCLIENT {

//
// External variables
//

// Pointer to the connection to the server
extern NLNET::CCallbackClient	*Connection;

//
// External functions
//

// Return true if the client is online
bool	isOnline ();

// Send the new entity (the player)
void	sendAddEntity (uint32 id, std::string &name, uint8 race);

// Send a chat line to the server
void	sendChatLine (std::string Line);

// Send the user entity position to the server
void	sendEntityPos (CEntity &entity);

// Send a new snowball to the server
void	sendSnowBall (uint32 eid, const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, float deflagRadius);

void	initNetwork (const std::string &lc, const std::string &addr);
void	updateNetwork ();
void	releaseNetwork ();

} /* namespace SBCLIENT */

#endif // NETWORK_H

/* End of network.h */
