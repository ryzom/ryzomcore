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



#ifndef SEND_CHAT_H
#define SEND_CHAT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/stream.h"

#include "nel/net/message.h"
#include "nel/net/unified_network.h"
#include "synchronised_message.h"
#include "chat_group.h"
#include "game_share/string_manager_sender.h"

#include <string>

/**
 * Send a chat line from system to a player that will be displayed as a normal chat sentence
 * Sentence will be formated using "<ServiceName:ServiceId>" as prefix of chat string
 */
void chatToPlayer(const NLMISC::CEntityId &id, const std::string &chatString);

/**
 * Send a chat line from system to a group of player that will be displayed as a normal chat sentence
 * Sentence will be formated using "<ServiceName:ServiceId>" as prefix of chat string
 */
void chatToGroup(const NLMISC::CEntityId &id, const std::string &chatString);

/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	phraseId is a phrase identifier in the phrase translation file.
 *	param are the parameter of the phrase
 */
void npcChatParamToChannel(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam> &params);

/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	phraseId is a phrase identifier in the phrase translation file.
 */
void npcChatToChannel(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, const std::string &phraseId);


/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	phraseId is a phrase identifier in the phrase translation file.
 */
void npcChatToChannelEx(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, uint32 phraseId);

/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	sentence is the sentence to be sent.
 */
void npcChatToChannelSentence(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, ucstring& sentence);

/**
 *	Request to the DSS to send a chat line from a bot in a chat channel
 *	Chat group can be constructed from CChatGroup class.
 *	sentenceId is the id of the sentence that must be sent by the DSS
 */
void forwardToDss(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, std::string& sentenceId,uint32 scenarioId);

/**
 *	Request to the DSS to send a chat line from a bot in a chat channel
 *	Chat group can be constructed from CChatGroup class.
 *	sentenceId is the id of the sentence that must be sent by the DSS
 */
void forwardToDssArg(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, std::string& sentenceId,uint32 scenarioId,std::vector<float>& argValues);

/**
 *	Send a tell line from a bot (mainly NPC) to a player
 *	phraseId is a phrase identifier in the phrase translation file.
 */
void npcTellToPlayer(const TDataSetRow &senderId, const TDataSetRow &receiverId, const std::string &phraseId, bool needSenderNpc=true);


/**
 *	Send a tell line from a bot (mainly NPC) to a player. Accept parametered strings
 *	phraseId is a phrase id obtained through the string manager
 */
void npcTellToPlayerEx(const TDataSetRow &senderId, const TDataSetRow &receiverId, uint32 phraseId);


#endif // SEND_CHAT_H

/* End of send_chat.h */
