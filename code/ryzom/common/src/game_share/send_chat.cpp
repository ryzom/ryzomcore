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

#include "nel/misc/types_nl.h"
#include "send_chat.h"

/**
 * Send a chat line from system to a player that will be displayed as a normal chat sentence
 * Sentence will be formated using "<ServiceName:ServiceId>" as prefix of chat string
 */
void chatToPlayer(const NLMISC::CEntityId &id, const std::string &chatString)
{
	NLNET::CMessage	msgout("CHAT");
	bool	talkToPlayer = true;
	msgout.serial(talkToPlayer, const_cast<NLMISC::CEntityId&>(id), const_cast<std::string&>(chatString));
	sendMessageViaMirror("IOS", msgout);
}

/**
 * Send a chat line from system to a group of player that will be displayed as a normal chat sentence
 * Sentence will be formated using "<ServiceName:ServiceId>" as prefix of chat string
 */
void chatToGroup(const NLMISC::CEntityId &id, const std::string &chatString)
{
	NLNET::CMessage	msgout("CHAT");
	bool	talkToPlayer = false;
	msgout.serial(talkToPlayer, const_cast<NLMISC::CEntityId&>(id), const_cast<std::string&>(chatString));
	sendMessageViaMirror("IOS", msgout);
}

/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	phraseId is a phrase identifier in the phrase translation file.
 *	param are the parameter of the phrase
 */
void npcChatParamToChannel(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam> &params)
{
	NLNET::CMessage	msgout("NPC_CHAT_PARAM");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serialEnum(groupType);
	msgout.serial(const_cast<std::string&>(phraseId));

	uint32 size = (uint32)params.size();
	msgout.serial(size);
//	params.resize(size);
	for ( uint i = 0; i < size; i++ )
	{
		uint8 type8 = params[i].Type;
		msgout.serial( type8 );
		const_cast<STRING_MANAGER::TParam&>(params[i]).serialParam( false, msgout, (STRING_MANAGER::TParamType) type8 );
	}

	sendMessageViaMirror("IOS", msgout);
}

/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	phraseId is a phrase identifier in the phrase translation file.
 */
void npcChatToChannel(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, const std::string &phraseId)
{
	NLNET::CMessage	msgout("NPC_CHAT");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serialEnum(groupType);
	msgout.serial(const_cast<std::string&>(phraseId));
	sendMessageViaMirror("IOS", msgout);
}


/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	phraseId is a phrase identifier in the phrase translation file.
 */
void npcChatToChannelEx(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, uint32 phraseId)
{
	NLNET::CMessage	msgout("NPC_CHAT_EX");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serialEnum(groupType);
	msgout.serial(phraseId);
	sendMessageViaMirror("IOS", msgout);
}

/**
 *	Send a chat line from a bot (mainly NPC) in a chat channel (know as chat group).
 *	Chat group can be constructed from CChatGroup class.
 *	sentence is the sentence to be sent.
 */
void npcChatToChannelSentence(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, ucstring& sentence)
{
	NLNET::CMessage	msgout("NPC_CHAT_SENTENCE");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serialEnum(groupType);
	msgout.serial(sentence);
	sendMessageViaMirror("IOS", msgout);
}

/**
 *	Request to the DSS to send a chat line from a bot in a chat channel
 *	Chat group can be constructed from CChatGroup class.
 *	sentenceId is the id of the sentence that must be sent by the DSS
 */
void forwardToDss(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, std::string& sentenceId,uint32 scenarioId)
{
	nlinfo( ("forwarding to DSS : id: "+sentenceId).c_str());
	NLNET::CMessage	msgout("translateAndForward");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serialEnum(groupType);
	msgout.serial(sentenceId);
	msgout.serial(scenarioId);
	NLNET::CUnifiedNetwork::getInstance()->send("DSS",msgout);
}

/**
 *	Request to the DSS to send a chat line from a bot in a chat channel
 *	Chat group can be constructed from CChatGroup class.
 *	sentenceId is the id of the sentence that must be sent by the DSS
 */
void forwardToDssArg(const TDataSetRow &senderId, CChatGroup::TGroupType groupType, std::string& sentenceId,uint32 scenarioId,std::vector<float>& argValues)
{
	nlinfo( ("forwarding to DSS : id: "+sentenceId).c_str());
	NLNET::CMessage	msgout("translateAndForwardArg");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serialEnum(groupType);
	msgout.serial(sentenceId);
	msgout.serial(scenarioId);
	uint32 size=(uint32)argValues.size(),i=0;
	msgout.serial(size);
	for(;i<size;++i)
	{
		msgout.serial(argValues[i]);
	}
	NLNET::CUnifiedNetwork::getInstance()->send("DSS",msgout);
}

/**
 *	Send a tell line from a bot (mainly NPC) to a player
 *	phraseId is a phrase identifier in the phrase translation file.
 */
void npcTellToPlayer(const TDataSetRow &senderId, const TDataSetRow &receiverId, const std::string &phraseId, bool needSenderNpc)
{
	NLNET::CMessage	msgout;
	if ( needSenderNpc )
	{
		msgout.setType("NPC_TELL");
		msgout.serial(const_cast<TDataSetRow&>(senderId));
	}
	else
	{
		msgout.setType("GHOST_TELL");
	}
	msgout.serial(const_cast<TDataSetRow&>(receiverId));
	msgout.serial(const_cast<std::string&>(phraseId));
	sendMessageViaMirror("IOS", msgout);
}


/**
 *	Send a tell line from a bot (mainly NPC) to a player. Accept parametered strings
 *	phraseId is a phrase id obtained through the string manager
 */
void npcTellToPlayerEx(const TDataSetRow &senderId, const TDataSetRow &receiverId, uint32 phraseId)
{
	NLNET::CMessage	msgout("NPC_TELL_EX");
	msgout.serial(const_cast<TDataSetRow&>(senderId));
	msgout.serial(const_cast<TDataSetRow&>(receiverId));
	msgout.serial(phraseId);
	sendMessageViaMirror("IOS", msgout);
}

/* End of send_chat.cpp */
