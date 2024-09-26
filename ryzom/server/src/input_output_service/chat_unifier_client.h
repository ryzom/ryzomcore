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

#ifndef CHAT_UNIFIER_CLIENT_H
#define CHAT_UNIFIER_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"

class IChatUnifierClient : public NLMISC::CManualSingleton<IChatUnifierClient>
{
public:

	virtual void sendFarTell(const NLMISC::CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text) =0;
	virtual void sendFarGuildChat(const ucstring &senderName, uint32 guildId, const ucstring &text) =0;
	virtual void sendFarGuildChat2(const ucstring &senderName, uint32 guildId, const std::string &phraseName) =0;
	virtual void sendFarGuildChat2Ex(const ucstring &senderName, uint32 guildId, uint32 phraseId) =0;
	virtual void sendUniverseChat(const ucstring &senderName, uint32 homeSessionId, const ucstring &text) = 0;
	virtual void sendUnifiedDynChat(const NLMISC::CEntityId &dynCharId, const ucstring &senderName, const ucstring &text) = 0;
};

#endif // CHAT_UNIFIER_CLIENT_H
