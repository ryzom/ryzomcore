// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef RY_SERVER_SHARE_CLIENT_REFUSAL_H
#define RY_SERVER_SHARE_CLIENT_REFUSAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/net/unified_network.h"

/**
 * A service refused a message of a character in game that no stock client sends, after logging it with a "HACK:"
 * warning. Ask the frontend serving the character, whose id is the dynamic id of the character's entity id, to count
 * it against the client (FEHACK reason RefusedClientMessage, toward RejectedMessagesLimit).
 */
inline void reportRefusedClientMessage(const NLMISC::CEntityId &sender)
{
	NLNET::CMessage msgout("CLIENT_MESSAGE_REFUSED");
	NLMISC::CEntityId senderId = sender;
	msgout.serial(senderId);
	NLNET::CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(sender.getDynamicId()), msgout);
}

#endif // RY_SERVER_SHARE_CLIENT_REFUSAL_H

/* End of client_refusal.h */
