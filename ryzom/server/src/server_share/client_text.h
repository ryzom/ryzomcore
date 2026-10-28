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

#ifndef RY_SERVER_SHARE_CLIENT_TEXT_H
#define RY_SERVER_SHARE_CLIENT_TEXT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/ucstring.h"
#include "nel/net/unified_network.h"

#include "game_share/chat_group.h"

/**
 * Check the length of a text a client sent in a chat line, a tell, an AFK text, a custom emote or a guild message of
 * the day. The stock client cuts what the player types to CChatGroup::MaxClientTextLength UTF-16 units. A longer text
 * is logged, and the frontend that forwarded the message is asked to count it against the client: return true, and
 * the caller drops the message.
 */
inline bool refuseClientTextTooLong(const NLMISC::CEntityId &sender, const char *messageName, const ucstring &text, NLNET::TServiceId frontendId)
{
	if (text.size() <= CChatGroup::MaxClientTextLength)
		return false;

	nlwarning("HACK: %s sent %s with %u UTF-16 units of text, above the limit of %u, refused", sender.toString().c_str(), messageName, (uint)text.size(), (uint)CChatGroup::MaxClientTextLength);
	NLNET::CMessage msgout("CLIENT_TEXT_TOO_LONG");
	NLMISC::CEntityId senderId = sender;
	msgout.serial(senderId);
	NLNET::CUnifiedNetwork::getInstance()->send(frontendId, msgout);
	return true;
}

#endif // RY_SERVER_SHARE_CLIENT_TEXT_H

/* End of client_text.h */
