// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2016  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


#include "stdpch.h"

#include <sstream>

#include <nel/misc/command.h>
#include <nel/misc/base64.h>

//#include "game_share/generic_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/synchronised_message.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/properties.h"
#include "game_share/backup_service_interface.h"

#include "server_share/r2_variables.h"
#include "server_share/memc_wrapper.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "chat_manager.h"
#include "string_manager.h"
#include "input_output_service.h"
#include "chat_unifier_client.h"

#include "server_share/log_chat_gen.h"

//#include "ios_pd.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

#ifdef LOG_INFO
#undef LOG_INFO
#endif

void	logChatDirChanged(IVariable &var)
{
	// LogChatDirectory variable changed, reset it!
	//IOS->getChatManager().resetChatLog();
}

CVariable<bool>			VerboseChatManagement("ios","VerboseChatManagement", "Set verbosity for chat management", false, 0, true);
CVariable<std::string>	LogChatDirectory("ios", "LogChatDirectory", "Log Chat directory (default, unset is SaveFiles service directory", "", 0, true, logChatDirChanged);
CVariable<bool>			ForceFarChat("ios","ForceFarChat", "Force the use of SU to dispatch chat", false, 0, true);
CVariable<bool>			EnableDeepL("ios","EnableDeepL", "Enable DeepL auto-translation system", false, 0, true);

typedef NLMISC::CTwinMap<TChanID, string> TChanTwinMap;
TChanTwinMap 	_ChanNames;

static const uint MaxChatHistoryEntries = 10000;
static const uint ChatHistoryLifetimeSeconds = 24 * 60 * 60;

//-----------------------------------------------
//	quoteParticipant
//
//-----------------------------------------------
static std::string quoteParticipant(std::string name)
{
	// Titles follow the shard suffix; the suffix is part of the Tell identity.
	const std::string::size_type title = name.find('$');
	if (title != std::string::npos && title > 0 && name[title - 1] == ')')
		name.erase(title);
	return toLowerAscii(name);
}

//-----------------------------------------------
//	newMessageId
//
//-----------------------------------------------
static std::string newMessageId()
{
	static boost::uuids::random_generator generator;
	return boost::uuids::to_string(generator());
}

namespace
{
	// Pass metadata through chat(), tell() and sendChat(); restore it on return or exception.
	class CSharedMessageScope
	{
	public:
		CSharedMessageScope(const CChatMessage *&target, const CChatMessage &message)
		: _Target(target), _Previous(target)
		{
			_Target = &message;
		}

		CSharedMessageScope(const CChatMessage *&target, const CChatMessage *message)
		: _Target(target), _Previous(target)
		{
			if (message)
				_Target = message;
		}

		~CSharedMessageScope()
		{
			_Target = _Previous;
		}

	private:
		const CChatMessage *&_Target;
		const CChatMessage *_Previous;
	};
}


//-----------------------------------------------
//	prepareMessageItems
//
//-----------------------------------------------
static void prepareMessageItems(std::vector<CChatMessagePart> &parts, const TDataSetRow &receiver)
{
	// Item names are localized for each receiver through the string manager.
	for (std::vector<CChatMessagePart>::iterator it = parts.begin(); it != parts.end(); ++it)
	{
		if (it->Type != CChatMessagePart::Item)
			continue;
		CChatMessageItem &item = it->ItemValue;
		if (!item.NamePhraseId.empty())
		{
			item.NameId = STRING_MANAGER::sendStringToClient(receiver, item.NamePhraseId,
				TVectorParamCheck(), &IosLocalSender);
		}
		else if (!item.Name.empty())
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal = item.Name;
			item.NameId = STRING_MANAGER::sendStringToClient(receiver, "LITERAL", params, &IosLocalSender);
		}
		if (!item.CreatorName.empty())
			item.Info.CreatorName = SM->storeString(item.CreatorName);
		item.NamePhraseId.clear();
		item.Name.clear();
		item.CreatorName.clear();
	}
}

//-----------------------------------------------
//	appendMessageTrailer
//
//-----------------------------------------------
static void appendMessageTrailer(CBitMemStream &stream, const CChatMessage &source,
	const ucstring &displayText, const CCharacterInfos &receiver, bool ownTell = false,
	const ucstring &tellTarget = ucstring(), bool noBubble = false,
	const ucstring &visibleSender = ucstring())
{
	CChatMessageTrailer trailer;
	trailer.OwnTell = ownTell;
	trailer.TellTarget = tellTarget;
	trailer.Message = source;
	trailer.Message.NoBubble = source.NoBubble || noBubble;
	if (!source.Quote.MessageId.empty() &&
		!IOS->getChatManager().canReceiveQuote(source.Quote.MessageId, receiver.EntityId))
		trailer.Message.Quote = CChatMessageQuote();
	if (!visibleSender.empty())
		trailer.Message.SenderName = visibleSender;
	ucstring cleanDisplayText = displayText;
	if (CHAT_MESSAGE::stripNoBubble(cleanDisplayText))
		trailer.Message.NoBubble = true;
	CHAT_MESSAGE::stripNoBubble(trailer.Message);
	prepareMessageItems(trailer.Message.Parts, receiver.DataSetIndex);
	prepareMessageItems(trailer.Message.TranslatedParts, receiver.DataSetIndex);
	prepareMessageItems(trailer.Message.Quote.Parts, receiver.DataSetIndex);
	// Keep Parts as the quote source; TranslatedParts carries recipient text
	// changed by legacy control prefixes when no translation payload exists.
	if (trailer.Message.TranslationLanguage.empty() && trailer.Message.TranslatedParts.empty() &&
		trailer.Message.Parts.size() == 1 && trailer.Message.Parts[0].Type == CChatMessagePart::Text &&
		!cleanDisplayText.empty() && trailer.Message.Parts[0].TextValue != cleanDisplayText)
	{
		CChatMessagePart displayPart = trailer.Message.Parts[0];
		displayPart.TextValue = cleanDisplayText;
		trailer.Message.TranslatedParts.push_back(displayPart);
	}
	stream.serial(trailer);
}

//-----------------------------------------------
//	sharedMessageLogText
//
//-----------------------------------------------
static ucstring sharedMessageLogText(const std::vector<CChatMessagePart> &parts, const std::string &sourceLanguage)
{
	ucstring text;
	const CStringManager::TLanguages language = SM->checkLanguageCode(sourceLanguage);
	const CStringManager::CEntityWords &places = SM->getEntityWords(language, STRING_MANAGER::place);
	for (std::vector<CChatMessagePart>::const_iterator it = parts.begin(); it != parts.end(); ++it)
	{
		if (it->Type == CChatMessagePart::Text)
			text += it->TextValue;
		else if (it->Type == CChatMessagePart::Item)
		{
			const CChatMessageItem &item = it->ItemValue;
			if (!item.Name.empty())
				text += item.Name;
			else
				text += ucstring::makeFromUtf8("[item:" + (item.NamePhraseId.empty() ?
					item.SheetId.toString() : item.NamePhraseId) + "]");
		}
		else if (it->Type == CChatMessagePart::Phrase)
		{
			const CChatMessagePhrase &phrase = it->PhraseValue;
			if (!phrase.Phrase.Name.empty())
				text += phrase.Phrase.Name;
			else
				text += ucstring::makeFromUtf8("[action:" + phrase.SheetId.toString() + "]");
		}
		else if (it->Type == CChatMessagePart::Position)
		{
			const CChatMessagePosition &position = it->PositionValue;
			text += position.Kind == CChatMessagePosition::UserLandMark ? position.FlagName :
				SM->getString(places.getStringId(toLowerAscii(position.Place), "name"));
		}
		else if (it->Type == CChatMessagePart::Macro)
			text += ucstring::makeFromUtf8(it->MacroValue.Name);
	}
	return text;
}

//-----------------------------------------------
//	sharedMessageLogText
//
//-----------------------------------------------
static ucstring sharedMessageLogText(const CChatMessage &message)
{
	return sharedMessageLogText(message.Parts, message.SourceLanguage);
}

//-----------------------------------------------
//	sharedMessageDisplayText
//
//-----------------------------------------------
static ucstring sharedMessageDisplayText(const CChatMessage &message)
{
	if (message.TranslatedParts.empty())
		return sharedMessageLogText(message);
	ucstring translated = sharedMessageLogText(message.TranslatedParts,
		message.TranslationLanguage.empty() ? message.SourceLanguage : message.TranslationLanguage);
	if (message.TranslationLanguage.empty() || message.SourceLanguage.size() != 2)
		return translated;
	return ucstring("{:") + ucstring(message.SourceLanguage) + ucstring(":") +
		sharedMessageLogText(message) + ucstring("}@{ ") + translated;
}

//-----------------------------------------------
//	sharedMessageDeliveryText
//
//-----------------------------------------------
static ucstring sharedMessageDeliveryText(const CChatMessage &message)
{
	ucstring text = sharedMessageDisplayText(message);
	if (message.NoBubble)
	{
		const ucstring tag("{no_bubble}");
		if (!text.empty() && text[0] == '&')
		{
			ucstring::size_type closing = text.find((ucchar)'&', 1);
			if (closing != ucstring::npos)
				text.insert(closing + 1, tag);
			else
				text = tag + text;
		}
		else
			text = tag + text;
	}
	return text;
}

//-----------------------------------------------
//	forwardedSharedMessage
//
//-----------------------------------------------
static CChatMessage forwardedSharedMessage(const CChatMessage &source, ucstring visibleText,
	const std::string &deliveryLanguage, bool noBubble = false)
{
	CChatMessage forwarded = source;
	CHAT_MESSAGE::stripNoBubble(forwarded);
	if (CHAT_MESSAGE::stripNoBubble(visibleText) || noBubble)
		forwarded.NoBubble = true;
	if (forwarded.Parts.size() == 1 && forwarded.Parts[0].Type == CChatMessagePart::Text &&
		forwarded.TranslatedParts.empty())
	{
		ucstring::size_type end = visibleText.find(ucstring("}@{"));
		if (visibleText.size() >= 8 && visibleText.substr(0, 2) == ucstring("{:") &&
			visibleText[4] == ':' && end != ucstring::npos && end >= 5)
		{
			forwarded.SourceLanguage = visibleText.substr(2, 2).toString();
			forwarded.Parts[0].TextValue = visibleText.substr(5, end - 5);
			CChatMessagePart translation;
			translation.TextValue = visibleText.substr(end + 4);
			forwarded.TranslatedParts.push_back(translation);
		}
		else
			forwarded.Parts[0].TextValue = visibleText;
	}
	if (!deliveryLanguage.empty())
		forwarded.TranslationLanguage = deliveryLanguage;
	return forwarded;
}

//-----------------------------------------------
//	getMentionCanonicalName
//
//-----------------------------------------------
static bool getMentionCanonicalName(const std::string &token, uint32 homeSessionId,
	std::string &canonical, TSessionId &session)
{
	CShardNames &shards = CShardNames::getInstance();
	const std::string::size_type dot = token.find('.'), open = token.find('(');
	if (dot != std::string::npos || open != std::string::npos)
	{
		const bool fullName = open != std::string::npos;
		if ((fullName && (token[token.size() - 1] != ')' || token.find('(', open + 1) != std::string::npos)) ||
			(!fullName && (dot == 0 || dot + 1 == token.size() || token.find('.', dot + 1) != std::string::npos)))
			return false;
		const std::string shard = fullName ? token.substr(open + 1, token.size() - open - 2) : token.substr(0, dot);
		uint matches = 0;
		const CShardNames::TSessionNames &names = shards.getSessionNames();
		for (uint i = 0; i < names.size(); ++i)
			if (compareCaseInsensitive(shard, fullName ? names[i].DisplayName : names[i].ShortName) == 0)
				++matches;
		if (matches != 1)
			return false;
	}
	std::string shortName;
	shards.parseRelativeName(TSessionId(homeSessionId), token, shortName, session);
	if (shortName.empty())
		return false;
	canonical = shards.makeFullName(shortName, session);
	return !canonical.empty();
}

//-----------------------------------------------
//	findMentionPlayer
//
//-----------------------------------------------
static CCharacterInfos *findMentionPlayer(const std::string &token, uint32 homeSessionId)
{
	std::string canonical;
	TSessionId session;
	if (!getMentionCanonicalName(token, homeSessionId, canonical, session))
		return NULL;
	CCharacterInfos *found = NULL;
	CInputOutputService::TIdToInfos &players = IOS->getCharInfosCont();
	for (CInputOutputService::TIdToInfos::iterator it = players.begin(); it != players.end(); ++it)
	{
		CCharacterInfos *player = it->second;
		if (player && player->EntityId.getType() == RYZOMID::player && player->HomeSessionId == session &&
			compareCaseInsensitive(player->ShortName.toUtf8(), canonical) == 0)
		{
			if (found)
				return NULL;
			found = player;
		}
	}
	return found;
}

//-----------------------------------------------
//	resolveChatMentions
//
//-----------------------------------------------
static void resolveChatMentions(CChatMessage &message)
{
	if (!message.ResolveMentions)
		return;
	std::vector<CChatMessageMention> mentions;
	for (uint part = 0; part < message.Parts.size(); ++part)
	{
		if (message.Parts[part].Type != CChatMessagePart::Text)
			continue;
		const ucstring &text = message.Parts[part].TextValue;
		for (uint start = 0; start < text.size(); ++start)
		{
			const uint end = CHAT_MESSAGE::getMentionEnd(text, start);
			if (end == start)
				continue;
			ucstring name = text.substr(start + 1, end - start - 1);
			CCharacterInfos *player = findMentionPlayer(name.toUtf8(), message.MentionHomeSessionId);
			CChatMessageMention mention;
			if (player)
			{
				mention.PlayerId = player->EntityId;
				mention.Name = player->ShortName;
			}
			else
			{
				bool found = false;
				for (uint i = 0; i < message.Mentions.size(); ++i)
					if (message.Mentions[i].Part == part && message.Mentions[i].Start == start && message.Mentions[i].Length == end - start)
					{
						mention = message.Mentions[i];
						found = true;
						break;
					}
				if (!found)
					continue;
			}
			mention.Part = (uint8)part;
			mention.Start = (uint16)start;
			mention.Length = (uint16)(end - start);
			mentions.push_back(mention);
			start = end - 1;
		}
	}
	message.Mentions.swap(mentions);
}

//-----------------------------------------------
//	prepareSharedMessage
//
//-----------------------------------------------
static bool prepareSharedMessage(const TDataSetRow &sender, CChatMessage &message, bool bypassTranslation = false,
	CChatGroup::TGroupType group = CChatGroup::nbChatMode)
{
	CCharacterInfos *infos = IOS->getCharInfos(TheDataset.getEntityId(sender));
	if (!infos || !message.isValid())
		return false;
	CHAT_MESSAGE::stripNoBubble(message);
	message.MessageId = newMessageId();
	message.SenderId = infos->EntityId;
	message.SenderName = infos->Name;
	message.Timestamp = CTime::getSecondsSince1970();
	message.SourceLanguage = SM->getLanguageCodeString(infos->Language);
	message.TranslationLanguage.clear();
	for (std::vector<CChatMessagePart>::iterator it = message.Parts.begin(); it != message.Parts.end(); ++it)
	{
		if (it->Type == CChatMessagePart::Position)
		{
			it->PositionValue.SenderName = infos->Name;
			it->PositionValue.Timestamp = message.Timestamp;
		}
	}
	message.AllowTranslation = EnableDeepL && !bypassTranslation &&
		!IOS->getChatManager().getClient(sender).dontSendTranslation(message.SourceLanguage);
	message.TranslatedParts.clear();
	message.MentionHomeSessionId = infos->HomeSessionId.asInt();
	message.ResolveMentions = true;
	// Players are resolved here; group mentions only in their own channel.
	std::vector<CChatMessageMention>::iterator mention = message.Mentions.begin();
	while (mention != message.Mentions.end())
	{
		const bool allowed = (mention->Scope == CChatMessageMention::Team && group == CChatGroup::team) ||
			(mention->Scope == CChatMessageMention::Guild && group == CChatGroup::guild) ||
			(mention->Scope == CChatMessageMention::All &&
				group != CChatGroup::tell && group != CChatGroup::nbChatMode);
		mention = allowed ? mention + 1 : message.Mentions.erase(mention);
	}
	resolveChatMentions(message);
	return message.isValid();
}

//-----------------------------------------------
//	prepareLegacyMessage
//
//-----------------------------------------------
static bool prepareLegacyMessage(const TDataSetRow &sender, const ucstring &text, CChatMessage &message, bool stripControlPrefix = false)
{
	if (text.empty() || text.size() > CHAT_MESSAGE::MaxTextLength)
		return false;
	// Ordinary chat and Tell messages need an ID too, so clients can quote them.
	CChatMessagePart part;
	part.TextValue = stripControlPrefix ? text.substr(1) : text;
	message.Parts.push_back(part);
	return prepareSharedMessage(sender, message, text[0] == '>');
}

//-----------------------------------------------
//	readTranslatedPartLength
//
//-----------------------------------------------
static bool readTranslatedPartLength(const std::string &translatedParts, std::string::size_type &offset, uint32 &length)
{
	length = 0;
	const std::string::size_type first = offset;
	while (offset < translatedParts.size() && translatedParts[offset] >= '0' &&
		translatedParts[offset] <= '9')
	{
		const uint32 digit = translatedParts[offset++] - '0';
		if (length > (CHAT_MESSAGE::MaxSerializedSize - digit) / 10)
			return false;
		length = length * 10 + digit;
	}
	if (offset == first || offset == translatedParts.size() || translatedParts[offset] != ':')
		return false;
	++offset;
	return true;
}

CChatManager::CChatManager () : _Log(CLog::LOG_INFO), _SharedMessage(NULL)
{
	_Log.addDisplayer(&_Displayer);
}

//-----------------------------------------------
//	findMessage
//
//-----------------------------------------------
CChatManager::CMessageHistoryEntry *CChatManager::findMessage(const std::string &messageId)
{
	std::map<std::string, CMessageHistoryEntry>::iterator it = _MessageHistory.find(messageId);
	return it == _MessageHistory.end() ? NULL : &it->second;
}

//-----------------------------------------------
//	rememberMessage
//
//-----------------------------------------------
CChatManager::CMessageHistoryEntry *CChatManager::rememberMessage(const CChatMessage &message)
{
	if (message.MessageId.empty())
		return NULL;
	CMessageHistoryEntry *existing = findMessage(message.MessageId);
	if (existing)
		return existing;
	const uint32 now = CTime::getSecondsSince1970();
	while (!_MessageHistoryOrder.empty())
	{
		const std::string &oldestId = _MessageHistoryOrder.front();
		CMessageHistoryEntry *oldest = findMessage(oldestId);
		if (oldest && _MessageHistoryOrder.size() < MaxChatHistoryEntries &&
			(now <= oldest->Message.Timestamp || now - oldest->Message.Timestamp <= ChatHistoryLifetimeSeconds))
			break;
		if (oldest && !oldest->ExternalId.empty())
		{
			std::map<std::string, std::string>::iterator mapped = _ExternalMessageIds.find(oldest->ExternalId);
			if (mapped != _ExternalMessageIds.end() && mapped->second == oldestId)
				_ExternalMessageIds.erase(mapped);
		}
		_MessageHistory.erase(oldestId);
		_MessageHistoryOrder.pop_front();
	}
	_MessageHistoryOrder.push_back(message.MessageId);
	CMessageHistoryEntry &entry = _MessageHistory[message.MessageId];
	entry.Message = message;
	return &entry;
}

//-----------------------------------------------
//	resolveQuote
//
//-----------------------------------------------
bool CChatManager::resolveQuote(CChatMessage &message, const std::string &channel,
	const CEntityId &sender, const std::string &receiver)
{
	const std::string quotedId = message.Quote.MessageId;
	message.Quote = CChatMessageQuote();
	if (quotedId.empty())
		return true;
	CMessageHistoryEntry *original = findMessage(quotedId);
	if (!original)
		return false;

	// Public channels need no recipient history; private quotes stay with their sender and receivers.
	if (sender != CEntityId::Unknown && !canReceiveQuote(quotedId, sender))
		return false;
	if (channel.compare(0, 5, "tell:") == 0)
	{
		// Tell quotes must stay between the original participants.
		if (original->Channel.compare(0, 5, "tell:") != 0 || receiver.empty())
			return false;
		CCharacterInfos *target = IOS->getCharInfos(ucstring::makeFromUtf8(receiver));
		if (target && original->Message.SenderId != target->EntityId &&
			original->Receivers.find(target->EntityId) == original->Receivers.end())
			return false;
		if (sender == CEntityId::Unknown)
		{
			if (!target)
				return false;
			const std::string currentSender = quoteParticipant(message.SenderName.toUtf8());
			const std::string currentTarget = quoteParticipant(receiver);
			const std::string sourceSender = quoteParticipant(original->Message.SenderName.toUtf8());
			const std::string sourceTarget = quoteParticipant(original->Channel.substr(5));
			if (currentSender.empty() || currentTarget.empty() ||
				!((currentSender == sourceSender && currentTarget == sourceTarget) ||
					(currentSender == sourceTarget && currentTarget == sourceSender)))
				return false;
		}
		else
		{
			// Remote characters have no local infos, so require a server-resolved full name.
			const bool qualifiedRemoteTarget = !target && receiver.find('(') != std::string::npos &&
				receiver[receiver.size() - 1] == ')';
			const std::string targetName = quoteParticipant(receiver);
			if (!target && (!qualifiedRemoteTarget ||
				(targetName != quoteParticipant(original->Message.SenderName.toUtf8()) &&
				 targetName != quoteParticipant(original->Channel.substr(5)))))
				return false;
		}
	}
	else if (original->Channel != channel && !(channel == "say" && original->Channel == "arround"))
		return false;

	message.Quote.MessageId = original->Message.MessageId;
	message.Quote.SenderId = original->Message.SenderId;
	message.Quote.SenderName = original->Message.SenderName;
	if (original->Message.SenderId == CEntityId::Unknown &&
		original->ExternalId.compare(0, 6, "zulip:") == 0 &&
		!message.Quote.SenderName.empty() && message.Quote.SenderName[0] != '~')
		message.Quote.SenderName = ucstring("~") + message.Quote.SenderName;
	message.Quote.Timestamp = original->Message.Timestamp;
	message.Quote.Parts = original->Message.Parts;
	return message.Quote.isValid();
}

//-----------------------------------------------
//	canReceiveQuote
//
//-----------------------------------------------
bool CChatManager::canReceiveQuote(const std::string &messageId, const CEntityId &receiver)
{
	CMessageHistoryEntry *original = findMessage(messageId);
	if (!original)
		return false;
	if (original->Message.SenderId == receiver ||
		original->Receivers.find(receiver) != original->Receivers.end())
		return true;
	return original->Channel == "universe" || original->Channel == "say" ||
		original->Channel == "shout" || original->Channel == "arround";
}

//-----------------------------------------------
//	recordSharedReceiver
//
//-----------------------------------------------
void CChatManager::recordSharedReceiver(const CEntityId &receiver, CChatGroup::TGroupType chatMode)
{
	if (chatMode == CChatGroup::universe || chatMode == CChatGroup::say ||
		chatMode == CChatGroup::shout || chatMode == CChatGroup::arround)
		return;
	if (_SharedMessage && receiver.getType() == RYZOMID::player)
	{
		CMessageHistoryEntry *entry = rememberMessage(*_SharedMessage);
		if (entry)
			entry->Receivers.insert(receiver);
	}
}

//-----------------------------------------------
//	logSharedMessage
//
//-----------------------------------------------
void CChatManager::logSharedMessage(const CChatMessage &message, const std::string &channel,
	const std::string &language)
{
	CMessageHistoryEntry *entry = rememberMessage(message);
	if (!entry || entry->Exported)
		return;
	entry->Channel = channel;
	entry->Exported = true;
	const std::string text = sharedMessageLogText(message).toUtf8();
	const std::string quoteText = sharedMessageLogText(message.Quote.Parts, message.SourceLanguage).toUtf8();
	std::string parts;
	const bool exportParts = message.Parts.size() != 1 || message.Parts[0].Type != CChatMessagePart::Text;
	for (uint i = 0; i < message.Parts.size(); ++i)
	{
		const CChatMessagePart &part = message.Parts[i];
		const ucstring partText = sharedMessageLogText(std::vector<CChatMessagePart>(1, part), message.SourceLanguage);
		if (exportParts)
			parts += toString("part|%u|%s|%s\n", i,
				part.Type == CChatMessagePart::Text ? "text" : "reference",
				base64::encode(partText.toUtf8()).c_str());
	}
	_Log.displayNL("chat_meta|1|%s|%s|%s|%s|%s|%u|%s|%s|%s|%s|%s|%s",
		base64::encode(channel).c_str(), base64::encode(IOS->getRocketName(message.SenderName)).c_str(),
		language.c_str(), message.AllowTranslation ? "*" : language.c_str(), message.MessageId.c_str(),
		message.Timestamp, message.SenderId.toString().c_str(),
		message.Quote.MessageId.c_str(), base64::encode(message.Quote.SenderName.toUtf8()).c_str(),
		base64::encode(quoteText).c_str(), base64::encode(text).c_str(),
		base64::encode(parts).c_str());
}


//-----------------------------------------------
//	init
//
//-----------------------------------------------
void CChatManager::init( /*const string& staticDBFileName, const string& dynDBFileName*/ )
{
//	if (!staticDBFileName.empty())
//		_StaticDB.load( staticDBFileName );
//
//	if (!dynDBFileName.empty())
//		_DynDB.load( dynDBFileName );

#ifdef HAVE_MEMCACHED
	CMemC::init();
#endif

	// create a chat group 'universe'
	addGroup(CEntityId(RYZOMID::chatGroup,0), CChatGroup::universe, "");

	// reset chat log system (at least to init it once!)
	resetChatLog();
} // init //

void CChatManager::onServiceDown(const std::string &serviceShortName)
{
	// if service is EGS, remove all chat groups
	if (serviceShortName == "EGS")
	{
		vector<TGroupId>	groupToRemove;

		// parse all group, selecting the one to remove when ESG is down
		std::map< TGroupId, CChatGroup >::iterator first(_Groups.begin()), last(_Groups.end());
		for (; first != last; ++first)
		{
			const TGroupId &gid = first->first;
			const CChatGroup &cg = first->second;

			switch (cg.Type)
			{
				case CChatGroup::universe:
				case CChatGroup::say:
				case CChatGroup::shout:
				case CChatGroup::player:
				case CChatGroup::nbChatMode:
					continue;
				case CChatGroup::team:
				case CChatGroup::guild:
				case CChatGroup::civilization:
				case CChatGroup::territory:
				case CChatGroup::tell:
				case CChatGroup::arround:
				case CChatGroup::system:
				case CChatGroup::region:
				case CChatGroup::dyn_chat:
					groupToRemove.push_back(gid);
					break;
			}
		}

		// remove all chat groups that belong to EGS or players
		for (uint i=0; i<groupToRemove.size(); ++i)
		{
			removeGroup(groupToRemove[i]);

		}

		// clear muted players table
		_MutedUsers.clear();

		// clear the dyn chats
		_DynChat.removeAllChannels();
	}
}

/*
 * Reset ChatLog management
 */
void CChatManager::resetChatLog()
{
	std::string	logPath = (LogChatDirectory.get().empty() ? Bsi.getLocalPath() : LogChatDirectory.get());
	_Displayer.setParam(CPath::standardizePath(logPath) + "chat.log");
}


bool CChatManager::checkClient( const TDataSetRow& id )
{
	TClientInfoCont::iterator itCl = _Clients.find( id );

	return itCl != _Clients.end();
}



void CChatManager::addMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MEMCACHED
	CMemC::setWithIndex("Shard-Command", toString("addMutedUser:%d", eid.getShortId()));
#endif
	_MutedUsers.insert( eid );
}

void CChatManager::removeMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MEMCACHED
	CMemC::setWithIndex("Shard-Command", toString("removeMutedUser:%d", eid.getShortId()));
#endif
	_MutedUsers.erase( eid );
}

void CChatManager::addUniverseMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MEMCACHED
	CMemC::setWithIndex("Shard-Command", toString("addUniverseMutedUser:%d", eid.getShortId()));
#endif

	_MutedUniverseUsers.insert( eid );
}

void CChatManager::removeUniverseMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MEMCACHED
	CMemC::setWithIndex("Shard-Command", toString("removeUniverseMutedUser:%d", eid.getShortId()));
#endif

	_MutedUniverseUsers.erase( eid );
}


//-----------------------------------------------
//	addClient
//
//-----------------------------------------------
void CChatManager::addClient( const TDataSetRow& id )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: addClient : adding client %s:%x into chat manager and universe group.",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}

	if(id.getIndex() == 0xffffff)
	{
		nlwarning("id.getIndex() == 0xffffff");
		return;
	}

	CEntityId eid = TheDataset.getEntityId(id);

	TClientInfoCont::iterator itCl = _Clients.find( id );
	if( itCl == _Clients.end() )
	{
		CChatClient *client = new CChatClient(id);
		_Clients.insert( make_pair(id,client) );

		if (eid.getType() == RYZOMID::player/* && !IsRingShard*/)
		{

			// add player in the group universe
			TGroupId grpUniverse = CEntityId(RYZOMID::chatGroup,0);
			addToGroup(grpUniverse, id);

			client->setChatMode(CChatGroup::say);
			client->updateAudience();

		}
	}
	else
	{
		nlwarning("CChatManager::addClient :  the client %s:%x is already in the manager !",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}
	// add in the dyn chat
	_DynChat.addClient(id);

} // addClient //



//-----------------------------------------------
//	removeClient
//
//-----------------------------------------------
void CChatManager::removeClient( const TDataSetRow& id )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: removeClient : removing the client %s:%x from chat manager !",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}

	TClientInfoCont::iterator itCl = _Clients.find( id );
	if( itCl != _Clients.end() )
	{
		// remove the client from any chat group that it subscribed.
		itCl->second->unsubscribeAllChatGroup();

		delete itCl->second;
		_Clients.erase( itCl );

	}
	else
	{
		nlwarning("CChatManager::removeClient : The client %s:%x is unknown !",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}
	// remove from the dyn chat
	_DynChat.removeClient(id);
} // removeClient //



//-----------------------------------------------
//	getClient
//
//-----------------------------------------------
CChatClient& CChatManager::getClient( const TDataSetRow& id )
{
	TClientInfoCont::iterator itCl = _Clients.find( id );
	if( itCl != _Clients.end() )
	{
		return *(itCl->second);
	}
	else
	{
		throw CChatManager::EChatClient(TheDataset.getEntityId(id));
	}

} // getClient //



//-----------------------------------------------
//	addGroup
//
//-----------------------------------------------
void CChatManager::addGroup( TGroupId gId, CChatGroup::TGroupType gType, const std::string &groupName )
{
	if ( gId == CEntityId::Unknown )
	{
		nlwarning("<CHAT> Cannot add chat group CEntityId::Unknown. group name = '%s'",groupName.c_str());
		return;
	}
	if (VerboseChatManagement)
	{
		if (!groupName.empty())
			nldebug("IOSCM: addGroup : adding %s named chat group %s as '%s'",
				CChatGroup::groupTypeToString(gType).c_str(),
				gId.toString().c_str(),
				groupName.c_str());
		else
			nldebug("IOSCM: addGroup : adding %s anonymous chat group %s",
				CChatGroup::groupTypeToString(gType).c_str(),
				gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp == _Groups.end() )
	{
		TStringId nameId = CStringMapper::map(groupName);
		_Groups.insert( make_pair(gId, CChatGroup(gType, nameId)) );

		if (!groupName.empty())
		{
			pair<map<TStringId, TGroupId>::iterator, bool> ret;
			ret = _GroupNames.insert(make_pair(nameId, gId));
			if (!ret.second)
			{
				nlwarning("CChatManager::addGroup : will adding group %s, a chat group with the same name '%s' already exist !",
					gId.toString().c_str(),
					groupName.c_str());
			}
		}
	}
	else
	{
		nlwarning("CChatManager::addGroup : the group %s already exists", gId.toString().c_str());
	}

} // addGroup //



//-----------------------------------------------
//	removeGroup
//
//-----------------------------------------------
void CChatManager::removeGroup( TGroupId gId )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: removeGroup : removing group %s",
			gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		if (itGrp->second.GroupName != CStringMapper::emptyId())
		{
			std::map<TStringId, TGroupId>::iterator it(_GroupNames.find(itGrp->second.GroupName));
			if (it == _GroupNames.end() || it->second != gId)
			{
				nlwarning("CChatManager::removeGroup : can't remove the group %s named '%s' from named group index",
					gId.toString().c_str(),
					CStringMapper::unmap(itGrp->second.GroupName).c_str());
			}
			else
				_GroupNames.erase(itGrp->second.GroupName);
		}
		_Groups.erase( itGrp );
	}
	else
	{
		nlwarning("CChatManager::removeGroup : the group %s is unknown", gId.toString().c_str());
	}

} // removeGroup //



//-----------------------------------------------
//	addToGroup
//
//-----------------------------------------------
void CChatManager::addToGroup( TGroupId gId, const TDataSetRow &charId )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: addtoGroup : adding player %s:%x to group %s",
		TheDataset.getEntityId(charId).toString().c_str(),
		charId.getIndex(),
		gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		// add player in the group
		pair<CChatGroup::TMemberCont::iterator, bool> ret;
		ret = itGrp->second.Members.insert( charId );
		if (!ret.second)
		{
			nlwarning("CChatManager::addToGroup : can't add player %s:%x into group %s, already inside !",
				TheDataset.getEntityId(charId).toString().c_str(),
				charId.getIndex(),
				gId.toString().c_str());
		}
		else
		{
			TClientInfoCont::iterator itCl = _Clients.find( charId );
			if( itCl != _Clients.end() )
			{
				itCl->second->subscribeInChatGroup(gId);

				if (itGrp->second.Type == CChatGroup::team)
					itCl->second->setTeamChatGroup( gId );
				else if (itGrp->second.Type == CChatGroup::guild)
					itCl->second->setGuildChatGroup( gId );
				else if (itGrp->second.Type == CChatGroup::region)
					itCl->second->setRegionChatGroup( gId );
			}
			else
			{
				nlwarning("CChatManager::addToGroup : client %s:%x is unknown",
					TheDataset.getEntityId(charId).toString().c_str(),
					charId.getIndex());
				// remove it from the group (don't leave bad client...)
				itGrp->second.Members.erase(charId);
			}
		}
	}
	else
	{
		nlwarning("CChatManager::addToGroup : the group %s is unknown",gId.toString().c_str());
	}

} // addToGroup //



//-----------------------------------------------
//	removeFromGroup
//
//-----------------------------------------------
void CChatManager::removeFromGroup( TGroupId gId, const TDataSetRow &charId )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: removeFromGroup : removing player %s:%x from group %s",
			TheDataset.getEntityId(charId).toString().c_str(),
			charId.getIndex(),
			gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup::TMemberCont::iterator itM = itGrp->second.Members.find(charId );
		if( itM != itGrp->second.Members.end() )
		{
			itGrp->second.Members.erase( itM );
			TClientInfoCont::iterator itCl = _Clients.find( charId );
			if( itCl != _Clients.end() )
			{
				itCl->second->unsubscribeInChatGroup(gId);

				if (itGrp->second.Type == CChatGroup::team)
					itCl->second->setTeamChatGroup( CEntityId::Unknown);
				else if (itGrp->second.Type == CChatGroup::guild)
					itCl->second->setGuildChatGroup( CEntityId::Unknown );
				else if (itGrp->second.Type == CChatGroup::region)
					itCl->second->setRegionChatGroup( CEntityId::Unknown );
			}
			else
			{
				nlwarning("CChatManager::removeFromGroup : player %s:%x is unknown",
					TheDataset.getEntityId(charId).toString().c_str(),
					charId.getIndex());
			}
		}
		else
		{
			nlwarning("CChatManager::removeFromGroup : player %s:%x is not in the group %s",
				TheDataset.getEntityId(charId).toString().c_str(),
				charId.getIndex(),
				gId.toString().c_str());
		}
	}
	else
	{
		nlwarning("CChatManager::removeFromGroup : the group %s is unknown",
			gId.toString().c_str());
	}

} // removeFromGroup //


//-----------------------------------------------
//	getGroup
//
//-----------------------------------------------
CChatGroup& CChatManager::getGroup( const TGroupId& gId )
{
	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		return (*itGrp).second;
	}
	else
	{
		throw EChatGroup(gId);
	}

} // getGroup //


//-----------------------------------------------
//	checkNeedDeeplize
//
//-----------------------------------------------
void CChatManager::checkNeedDeeplize( const TDataSetRow& sender, const ucstring& ucstr, const string& senderLang,
	string &langs, uint &nbrReceivers, bool controlPrefix, TGroupId grpId)
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	CChatManager &cm = IOS->getChatManager();
	CChatClient &senderClient = cm.getClient(sender);

	bool have_fr = false;
	bool have_de = false;
	bool have_en = false;
	bool have_ru = false;
	bool have_es = false;
	CChatGroup::TMemberCont::iterator itA;
	CChatGroup::TMemberCont::iterator itEnd;

	nbrReceivers = 0;

	if (grpId == CEntityId::Unknown)
	{
		itA = itCl->second->getAudience().Members.begin();
		itEnd = itCl->second->getAudience().Members.end();
	}
	else
	{
		map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
		if( itGrp != _Groups.end() )
		{
			CChatGroup &chatGrp = itGrp->second;
			itA = chatGrp.Members.begin();
			itEnd = chatGrp.Members.end();
		}
		else
		{
			itA = itCl->second->getAudience().Members.end();
			itEnd = itCl->second->getAudience().Members.end();
		}
	}


	for( ; itA != itEnd; ++itA )
	{
		ucstring message;
		string receiverName;
		NLMISC::CEntityId receiverId = TheDataset.getEntityId(*itA);
		CCharacterInfos* co = IOS->getCharInfos(receiverId);

		_DestUsers.push_back(receiverId);
		string receiverLang;
		if (co == NULL)
		{
			receiverName = receiverId.toString();
		}
		else
		{
			receiverName = co->Name.toString();
			receiverLang = SM->getLanguageCodeString(co->Language);
		}

		if (EnableDeepL && !senderClient.dontSendTranslation(senderLang))
		{
			CChatClient &client = getClient(*itA);

			if (senderLang == "wk")
				receiverLang = senderLang;

			if (controlPrefix) // Sent directly when prefixed by '>', it's the anti-translation code
			{
				if (grpId == CEntityId::Unknown && ucstr.length() > 5 && ucstr[1] == ':' && ucstr[4] == ':') // check lang prefix only for chat()
				{
					string usedLang = ucstr.toString().substr(2, 2);
					if (usedLang == receiverLang && !client.dontReceiveTranslation(usedLang))
						message = ucstr.substr(5);
				}
				else
				{
					message = ucstr.substr(1);
				}
			}
			else if (senderLang == receiverLang || client.dontReceiveTranslation(senderLang)) // Sent directly if sender and receiver uses same lang
			{
				message = ucstr;
			}
			else
			{
				if (!have_fr && receiverLang == "fr")
					have_fr = true;
				if (!have_de && receiverLang == "de")
					have_de = true;
				if (!have_en && receiverLang == "en")
					have_en = true;
				if (!have_ru && receiverLang == "ru")
					have_ru = true;
				if (!have_es && receiverLang == "es")
					have_es = true;
				nbrReceivers++;
			}
		}
		else
		{
			message = controlPrefix ? ucstr.substr(1) : ucstr;
		}

		if (!message.empty())
		{
			if (grpId == CEntityId::Unknown)
				sendChat( itCl->second->getChatMode(), *itA, message, sender);
		}
	}

	if (grpId != CEntityId::Unknown) // Chat in group must be sent only one time
	{
		if (controlPrefix) // direct, no translation
			chatInGroup( grpId, ucstr.substr(1), sender );
		else if (ucstr.length() > 5 && ucstr[1] == ':' && ucstr[4] == ':' &&
			(!EnableDeepL || !_SharedMessage || _SharedMessage->Parts[0].Type == CChatMessagePart::Text)) // Already have filter
			chatInGroup( grpId, ucstr, sender );
		else
			chatInGroup( grpId, ucstring(":"+senderLang+": ")+ucstr, sender ); // Need filter
	}


	langs = senderLang;
	if (have_fr)
		langs += "-fr";
	if (have_de)
		langs += "-de";
	if (have_en)
		langs += "-en";
	if (have_ru)
		langs += "-ru";
	if (have_es)
		langs += "-es";

}




//-----------------------------------------------
//	chat
//
//-----------------------------------------------
void CChatManager::chat( const TDataSetRow& sender, const ucstring& ucstr, bool sharedControlPrefix)
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	const bool nativeSharedMessage = _SharedMessage != NULL;
	const bool controlPrefix = nativeSharedMessage ? sharedControlPrefix : (!ucstr.empty() && ucstr[0] == '>');
	if (!nativeSharedMessage && controlPrefix && ucstr.size() == 1)
		return;
	CChatMessage legacyMessage;
	const CChatMessage *legacyMetadata = NULL;
	if (!_SharedMessage && itCl != _Clients.end() && prepareLegacyMessage(sender, ucstr, legacyMessage, controlPrefix))
		legacyMetadata = &legacyMessage;
	CSharedMessageScope legacyScope(_SharedMessage, legacyMetadata);

	if( itCl != _Clients.end() )
	{
		CChatManager &cm = IOS->getChatManager();
		CChatClient &senderClient = cm.getClient(sender);

//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM:  chat The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}

//		CEntityId eid = TheDataset.getEntityId(sender);
		// Get the char info
		//WARNING: can be NULL
		CCharacterInfos *ci = IOS->getCharInfos(eid);

		// info for log the chat message
		string senderName;
		string fullName;

		if (ci == NULL)
		{
			senderName = TheDataset.getEntityId(sender).toString();
			fullName = senderName;
		}
		else
		{
			fullName = IOS->getRocketName(ci->Name);
			senderName = ci->Name.toString();
		}

		static const char*	groupNames[]=
		{
			"say",
			"shout",
			"team",
			"guild",
			"civilization",
			"territory",
			"universe",
			"tell",
			"player",
			"arround",
			"system",
			"region",
			"dyn_chat",
			"nbChatMode"
		};

		// clean up container
		_DestUsers.clear();

		string senderLang = SM->getLanguageCodeString(ci->Language);

		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::shout :
		case CChatGroup::say :
		case CChatGroup::arround :
			{
				string langs;
				uint nbrReceivers;
				checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, controlPrefix);

				if (_SharedMessage || nbrReceivers > 0)
				{
					if (_SharedMessage)
						logSharedMessage(*_SharedMessage, groupNames[itCl->second->getChatMode()], senderLang);
					else
						_Log.displayNL("%s|%s|%s|%s|%s", groupNames[itCl->second->getChatMode()], fullName.c_str(), senderLang.c_str(), langs.c_str(), ucstr.toUtf8().c_str() );
				}
			}
			break;
		case CChatGroup::region :
			{
				// Previously, the msgs were sent to the current audience as well, to avoid characters around but
				// in an adjoining region not receiving the region msg. But the neighbouring was not tested,
				// and the audience was not updated after the chat mode became region (see CChatClient::updateAudience())
				// so even after teleporting in a remote region the previous around people were still receiving
				// the messages.

				TGroupId grpId = itCl->second->getRegionChatGroup();
				_DestUsers.push_back(grpId);
				string langs;
				uint nbrReceivers;
				checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, controlPrefix, grpId);
				if (_SharedMessage || nbrReceivers > 0)
				{
					if (_SharedMessage)
						logSharedMessage(*_SharedMessage, "region:" + grpId.toString(), senderLang);
					else
						_Log.displayNL("region:%s|%s|%s|%s|%s", grpId.toString().c_str(), fullName.c_str(), senderLang.c_str(), langs.c_str(), ucstr.toUtf8().c_str() );
				}
			}
			break;


		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup, 0);
				_DestUsers.push_back(grpId);

				double date = 1000.0*(double)CTime::getSecondsSince1970();

				bool isTranslation = false;
				string autoSub = "1";
				uint8 startPos = 0;
				string rtzText = ucstr.toUtf8();
				string chatId = "all";
				string chatType = "univers";


				string usedlang = senderLang;
				if (_SharedMessage && !EnableDeepL)
					logSharedMessage(*_SharedMessage, "universe", senderLang);

				if (EnableDeepL)
				{
					chatType = "dynamic";
					if (controlPrefix) // Sent directly when prefixed by '>', it's the anti-translation code
					{
						startPos = 1;
						rtzText = rtzText.substr(1);
						string::size_type endOfOriginal = rtzText.find("}@{");
						if (rtzText.size() > 4 && rtzText[0] == ':' && rtzText[3] == ':')
						{
							if (rtzText[4] == '{')
								isTranslation = true;
							startPos = 5;
							usedlang = rtzText.substr(1, 2);
							string source_lang = usedlang;

							if (endOfOriginal != string::npos)
							{
								if (rtzText.size() > 9)
									source_lang = rtzText.substr(6, 2);
								string sourceText = rtzText.substr(9, endOfOriginal-9);
								strFindReplace(sourceText, ")", "}");
								rtzText = rtzText.substr(endOfOriginal+4, rtzText.size()-endOfOriginal-4);
								}
							else
							{
								rtzText = rtzText.substr(4, rtzText.size()-4);
							}

							if (source_lang == "en") // in RC the icon are :gb:
								rtzText = ":gb: "+rtzText;
							else
								rtzText = ":"+source_lang+": "+rtzText;

							chatId = "FACTION_EN";
							if (usedlang != SM->getLanguageCodeString(ci->Language))
								autoSub = "0";
						}
						chatInGroup( grpId, ucstr.substr(1), sender );
						if (nativeSharedMessage)
							logSharedMessage(*_SharedMessage, "universe", senderLang);
					}
					else if (senderClient.dontSendTranslation(senderLang))
					{
						chatInGroup( grpId, ucstr, sender );
						if (nativeSharedMessage)
							logSharedMessage(*_SharedMessage, "universe", senderLang);
					}
					else
					{
						string langs;
						uint nbrReceivers;
						checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, controlPrefix, grpId);
						if (_SharedMessage)
							logSharedMessage(*_SharedMessage, "universe", senderLang);
						else
							_Log.displayNL("universe|%s|%s|*|%s", fullName.c_str(), senderLang.c_str(), ucstr.toUtf8().c_str());
					}
				}
				else
					chatInGroup( grpId, controlPrefix ? ucstr.substr(1) : ucstr, sender );
			}
			break;

		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				_DestUsers.push_back(grpId);
				string langs;
				uint nbrReceivers;
				checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, controlPrefix, grpId);
				if (_SharedMessage)
					logSharedMessage(*_SharedMessage, "team:" + grpId.toString(), senderLang);
				else
					_Log.displayNL("team:%s|%s|%s|%s|%s", grpId.toString().c_str(), fullName.c_str(), senderLang.c_str(), langs.c_str(), ucstr.toUtf8().c_str() );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				_DestUsers.push_back(grpId);

				uint32 guildId = grpId.getShortId() - 0x10000000;
				ostringstream sGuildId;
				sGuildId << guildId;

				double date = 1000.0*(double)CTime::getSecondsSince1970();

				bool isTranslation = false;
				uint8 startPos = 0;
				string rtzText = ucstr.toUtf8();
				string usedlang = senderLang;
				if (_SharedMessage && !EnableDeepL)
					logSharedMessage(*_SharedMessage, "guild:" + grpId.toString(), senderLang);

				if (EnableDeepL)
				{
					if (controlPrefix)
					{
						startPos = 1;
						rtzText = rtzText.substr(1);
						string::size_type endOfOriginal = rtzText.find("}@{");
						if (rtzText.size() > 4 && rtzText[0] == ':' && rtzText[3] == ':')
						{
							if (rtzText[4] == '{')
								isTranslation = true;
							startPos = 5;
							usedlang = rtzText.substr(1, 2);
							string source_lang = usedlang;

							if (endOfOriginal != string::npos)
							{
								if (rtzText.size() > 9)
									source_lang = rtzText.substr(6, 2);
								string sourceText = rtzText.substr(9, endOfOriginal-9);
								strFindReplace(sourceText, ")", "}");
								rtzText = rtzText.substr(endOfOriginal+4, rtzText.size()-endOfOriginal-4);
							}
							else
							{
								rtzText = rtzText.substr(4, rtzText.size()-4);
							}

							if (source_lang == "en") // in RC the icon are :gb:
								rtzText = ":gb: "+rtzText;
							else
								rtzText = ":"+source_lang+": "+rtzText;
						}
						chatInGroup( grpId, ucstr.substr(1), sender );
						if (nativeSharedMessage)
							logSharedMessage(*_SharedMessage, "guild:" + grpId.toString(), senderLang);
					}
					else if (senderClient.dontSendTranslation(senderLang))
					{
						chatInGroup( grpId, ucstr, sender );
						if (nativeSharedMessage)
							logSharedMessage(*_SharedMessage, "guild:" + grpId.toString(), senderLang);
					}
					else
					{
						string langs;
						uint nbrReceivers;
						checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, controlPrefix, grpId);
						if (_SharedMessage)
							logSharedMessage(*_SharedMessage, "guild:" + grpId.toString(), senderLang);
						else
							_Log.displayNL("guild:%s|%s|%s|%s|%s", grpId.toString().c_str(), fullName.c_str(), senderLang.c_str(), langs.c_str(), ucstr.toUtf8().c_str() );
					}
				}
				else
					chatInGroup(grpId, controlPrefix ? ucstr.substr(1) : ucstr, sender);
			}
			break;



		case CChatGroup::dyn_chat:
		{
			TChanID chanId = itCl->second->getDynChatChan();

			CDynChatSession *session = _DynChat.getSession(chanId, sender);
			if (session) // player must have a session in that channel
			{
				if (session->WriteRight) // player must have the right to speak in the channel
				{
					// If universal channel check if player muted
					if (session->getChan()->UniversalChannel)
					{
						if(_MutedUsers.find( eid ) != _MutedUsers.end())
						{
							nldebug("IOSCM:  chat The player %s:%x is muted",
								TheDataset.getEntityId(sender).toString().c_str(),
								sender.getIndex());
							return;
						}
					}

					const std::string *tmpChatId = _ChanNames.getB(chanId);
					string chatId;
					if (tmpChatId)
						chatId = *tmpChatId;

					double date = 1000.0*(double)CTime::getSecondsSince1970();


					string rtzText = ucstr.toUtf8();

					bool sendMessages = true;
					bool haveOriginMessage = false;
					bool isTranslation = false;
					uint8 startPos = controlPrefix ? 1 : 0;

					string usedlang = senderLang;
					string autoSub = "0";

					if (EnableDeepL)
					{
						if (senderClient.dontSendTranslation(senderLang) || controlPrefix) // Sent directly when prefixed by '>', it's the anti-translation code
						{
							startPos = controlPrefix ? 1 : 0;
							rtzText = rtzText.substr(startPos);
							string::size_type endOfOriginal = rtzText.find("}@{");
							if (rtzText.size() > 4 && rtzText[0] == ':' && rtzText[3] == ':')
							{
								if (rtzText[4] == '{')
									isTranslation = true;
								startPos = 5;
								usedlang = rtzText.substr(1, 2);
								string source_lang = usedlang;

								if (endOfOriginal != string::npos)
								{
									haveOriginMessage = true;
									if (rtzText.size() > 9)
										source_lang = rtzText.substr(6, 2);
									string sourceText = rtzText.substr(9, endOfOriginal-9);
									strFindReplace(sourceText, ")", "}");
									rtzText = rtzText.substr(endOfOriginal+4, rtzText.size()-endOfOriginal-4);
								}
								else
								{
									rtzText = rtzText.substr(4, rtzText.size()-4);
								}

								if (source_lang == "en") // in RC the icon are :gb:
									rtzText = ":gb: "+rtzText;
								else
									rtzText = ":"+source_lang+": "+rtzText;
							}
							if (nativeSharedMessage && !chatId.empty())
								logSharedMessage(*_SharedMessage, "dyn:" + chatId, senderLang);
						}
						// Send for translation
						else
						{
							bool have_fr = false;
							bool have_de = false;
							bool have_en = false;
							bool have_ru = false;
							bool have_es = false;
							CDynChatSession *dcc = session->getChan()->getFirstSession();
							while (dcc)
							{
								NLMISC::CEntityId receiverId = TheDataset.getEntityId(dcc->getClient()->getID());
								CCharacterInfos* co = IOS->getCharInfos(receiverId);
								string receiverLang = SM->getLanguageCodeString(co->Language);
								if (!have_fr && receiverLang == "fr")
									have_fr = true;
								if (!have_de && receiverLang == "de")
									have_de = true;
								if (!have_en && receiverLang == "en")
									have_en = true;
								if (!have_ru && receiverLang == "ru")
									have_ru = true;
								if (!have_es && receiverLang == "es")
									have_es = true;
								dcc = dcc->getNextChannelSession(); // next session in this channel
							}
							string langs = senderLang;
							if (have_fr)
								langs += "-fr";
							if (have_de)
								langs += "-de";
							if (have_en)
								langs += "-en";
							if (have_ru)
								langs += "-ru";
							if (have_es)
								langs += "-es";

							if (_SharedMessage)
								logSharedMessage(*_SharedMessage, "dyn:" + chatId, senderLang);
							else
								_Log.displayNL("dyn:%s|%s|%s|%s|%s", chatId.c_str(), fullName.c_str(), senderLang.c_str(), langs.c_str(), ucstr.toUtf8().c_str());
							sendMessages = _SharedMessage != NULL;
						}
					}

					if (sendMessages)
					{
						if (!session->getChan()->getDontBroadcastPlayerInputs())
						{
							if (_SharedMessage && !EnableDeepL && !chatId.empty())
								logSharedMessage(*_SharedMessage, "dyn:" + chatId, senderLang);
							// add msg to the historic
							CDynChatChan::CHistoricEntry entry;
							entry.String = ucstr;
							if (_SharedMessage)
							{
								entry.Shared = true;
								entry.Message = *_SharedMessage;
							}
							if (ci != NULL)
								entry.SenderString = ci->Name;
							else
								entry.SenderString = "";

							ucstring content = ucstr.substr(startPos);
							if (session->getChan()->HideBubble)
							{
								// true for control channel (Ring)
								ucstring tmp("{no_bubble}");
								if (content.find(tmp) == ucstring::npos)
								{
									tmp += content;
									content.swap(tmp);
								}
							}
							if (nativeSharedMessage || controlPrefix)
								entry.String = content;
							session->getChan()->Historic.push(entry);

							// broadcast to other client in the channel
							CDynChatSession *dcc = session->getChan()->getFirstSession();
							while (dcc)
							{
								NLMISC::CEntityId receiverId = TheDataset.getEntityId(dcc->getClient()->getID());
								CCharacterInfos* co = IOS->getCharInfos(receiverId);
								CChatClient &receiverClient = getClient(dcc->getClient()->getID());

								bool canSendChat = true;

								if (EnableDeepL)
								{
									if (receiverClient.dontReceiveTranslation(senderLang))
									{
										if (haveOriginMessage) // Only send untranslated message
											canSendChat = false;
									}
									else if (!usedlang.empty() && co && usedlang != SM->getLanguageCodeString(co->Language))
										canSendChat = false;
								}

								if (canSendChat)
									sendChat(itCl->second->getChatMode(), dcc->getClient()->getID(), content, sender, chanId);
								dcc = dcc->getNextChannelSession(); // next session in this channel
							}
						}
						else
							sendChat(itCl->second->getChatMode(), itCl->first, ucstr.substr(startPos), sender, chanId);

						if (session->getChan()->getForwardPlayerIntputToOwnerService())
						{
							// send player input to service owner
							NLNET::TServiceId serviceId(chanId.getCreatorId());

							TPlayerInputForward	pif;
							pif.ChanID = chanId;
							pif.Sender = sender;
							pif.Content = controlPrefix ? ucstr.substr(startPos) : ucstr;

							CMessage msgout( "DYN_CHAT:FORWARD");
							msgout.serial(pif);

							CUnifiedNetwork::getInstance()->send(serviceId, msgout);
						}

						if (session->getChan()->getUnifiedChannel())
						{
							// send the text to other shards
							if (IChatUnifierClient::getInstance())
							{
								if (_SharedMessage)
								{
									CChatMessage forwarded = forwardedSharedMessage(*_SharedMessage, ucstr.substr(startPos),
										EnableDeepL ? usedlang : std::string(), session->getChan()->HideBubble);
									IChatUnifierClient::getInstance()->sendUnifiedDynChatShared(session->getChan()->getID(), senderName, forwarded);
								}
								else
									IChatUnifierClient::getInstance()->sendUnifiedDynChat(session->getChan()->getID(), senderName,
										controlPrefix ? ucstr.substr(startPos) : ucstr);
							}
						}
					}
				}
			}
		}
		break;
			// static group

		default :
			nlwarning("<CChatManager::chat> client %u chat in %s ! don't know how to handle it.",
				sender.getIndex(),
				groupNames[itCl->second->getChatMode()]);
		}

		// log chat to PDS system
//		IOSPD::logChat(ucstr, itCl->second->getId(), _DestUsers);
		log_Chat_Chat(CChatGroup::groupTypeToString(itCl->second->getChatMode()),
			TheDataset.getEntityId(sender),
			ucstr.toUtf8(),
			_DestUsers);

	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

} // chat //

//-----------------------------------------------
//	chatShared
//
//-----------------------------------------------
bool CChatManager::chatShared(const TDataSetRow &sender, const CChatMessage &message)
{
	if (_Clients.find(sender) == _Clients.end())
		return false;
	CChatMessage prepared = message;
	CChatClient &client = getClient(sender);
	if (!prepareSharedMessage(sender, prepared, false,
		client.getChatMode() == CChatGroup::arround ? CChatGroup::say : client.getChatMode()))
		return false;
	if (_MutedUsers.find(prepared.SenderId) != _MutedUsers.end())
		return false;
	std::string channel;
	TGroupId groupId = CEntityId::Unknown;
	switch (client.getChatMode())
	{
	case CChatGroup::say:
		channel = "say";
		groupId = client.getSayAudienceId();
		break;
	case CChatGroup::shout:
		channel = "shout";
		groupId = client.getShoutAudienceId();
		break;
	case CChatGroup::arround:
		channel = "arround";
		groupId = client.getSayAudienceId();
		break;
	case CChatGroup::universe:
		if (_MutedUniverseUsers.find(prepared.SenderId) != _MutedUniverseUsers.end())
			return false;
		channel = "universe";
		groupId = CEntityId(RYZOMID::chatGroup, 0);
		break;
	case CChatGroup::team:
		groupId = client.getTeamChatGroup();
		channel = "team:" + groupId.toString();
		break;
	case CChatGroup::guild:
		groupId = client.getGuildChatGroup();
		channel = "guild:" + groupId.toString();
		break;
	case CChatGroup::region:
		groupId = client.getRegionChatGroup();
		channel = "region:" + groupId.toString();
		break;
	case CChatGroup::dyn_chat:
		{
			const TChanID chanId = client.getDynChatChan();
			CDynChatSession *session = _DynChat.getSession(chanId, sender);
			if (!session || !session->WriteRight || !session->getChan())
				return false;
			const std::string *name = _ChanNames.getB(chanId);
			if (name)
				channel = "dyn:" + *name;
		}
		break;
	default:
		break;
	}
	if (client.getChatMode() != CChatGroup::dyn_chat && !channel.empty())
	{
		std::map<TGroupId, CChatGroup>::const_iterator group = _Groups.find(groupId);
		if (group == _Groups.end() || group->second.Members.empty() ||
			group->second.Type != (client.getChatMode() == CChatGroup::arround ? CChatGroup::say : client.getChatMode()))
			return false;
	}
	if (channel.empty() || !resolveQuote(prepared, channel, prepared.SenderId) || !prepared.isValid())
		return false;
	const bool sharedControlPrefix = !prepared.Parts.empty() &&
		prepared.Parts[0].Type == CChatMessagePart::Text &&
		!prepared.Parts[0].TextValue.empty() && prepared.Parts[0].TextValue[0] == '>';
	if (sharedControlPrefix)
	{
		prepared.Parts[0].TextValue.erase(0, 1);
		prepared.AllowTranslation = false;
		std::vector<CChatMessageMention> groupMentions;
		for (uint i = 0; i < prepared.Mentions.size(); ++i)
		{
			CChatMessageMention mention = prepared.Mentions[i];
			if (mention.Scope == CChatMessageMention::Player)
				continue;
			if (mention.Part == 0)
				--mention.Start;
			groupMentions.push_back(mention);
		}
		prepared.Mentions.swap(groupMentions);
		resolveChatMentions(prepared);
		if (!prepared.isValid())
			return false;
	}
	CSharedMessageScope scope(_SharedMessage, prepared);
	ucstring text = sharedMessageLogText(prepared);
	if (prepared.NoBubble)
		text = ucstring("{no_bubble}") + text;
	if (sharedControlPrefix)
		text = ucstring(">") + text;
	chat(sender, text, sharedControlPrefix);
	return true;
}

//-----------------------------------------------
//	sendQuoteResult
//
//-----------------------------------------------
void CChatManager::sendQuoteResult(const CEntityId &sender, uint32 requestId, bool accepted)
{
	if (requestId == 0)
		return;
	CMessage msgout("IMPULS_CH_ID");
	uint8 channel = 1;
	msgout.serial(const_cast<CEntityId&>(sender));
	msgout.serial(channel);
	CBitMemStream stream;
	if (!GenericXmlMsgHeaderMngr.pushNameToStream("STRING:CHAT_SHARE_RESULT", stream))
		return;
	stream.serial(requestId);
	stream.serial(accepted);
	msgout.serialBufferWithSize((uint8*)stream.buffer(), stream.length());
	sendMessageViaMirror(TServiceId(sender.getDynamicId()), msgout);
}


//-----------------------------------------------
//	chatInGroup
//
//-----------------------------------------------
void CChatManager::chatInGroup( TGroupId& grpId, const ucstring& ucstr, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		list<CEntityId>	logDest;

		uint8 startPos = 0;
		string usedlang = "";
		string str = ucstr.toString();
		if (EnableDeepL && str.length() > 4 && str[0] == ':' && str[3] == ':') // check lang prefix
		{
			usedlang = str.substr(1, 2);
			startPos = 4;
		}

		bool areOriginal = true;
		string originLang = usedlang;
		if (EnableDeepL && str.length() > 8 && str[4] == '{' && str[5] == ':' && str[8] == ':') // check lang origin
		{
			areOriginal = false;
			originLang = str.substr(6, 2);
		}

		for( itM = chatGrp.Members.begin(); itM != chatGrp.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			if (EnableDeepL && !usedlang.empty())
			{
				NLMISC::CEntityId receiverId = TheDataset.getEntityId(*itM);
				CCharacterInfos* co = IOS->getCharInfos(receiverId);
				CChatClient &client = getClient(*itM);

				if (client.dontReceiveTranslation(originLang))
				{
					if (!areOriginal)
						continue;
				}
				else if (co == NULL || usedlang != SM->getLanguageCodeString(co->Language))
					continue;
			}

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				// check homeSessionId for universe
				if (/*IsRingShard && */chatGrp.Type == CChatGroup::universe)
				{
					CCharacterInfos *senderChar = IOS->getCharInfos(TheDataset.getEntityId(sender));
					CCharacterInfos *receiverChar = IOS->getCharInfos(TheDataset.getEntityId(*itM));

					if (senderChar == NULL || receiverChar == NULL)
						continue;

					// set GM mode if either speaker of listener is a GM
					bool isGM= senderChar->HavePrivilege || receiverChar->HavePrivilege;

					// for normal players don't send chat to them if their home session id doesn't match the speaker's
					if (!isGM && senderChar->HomeSessionId != receiverChar->HomeSessionId)
					{
						continue;
					}
				}
				// check the exclude list
				if ( std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
				{
					sendChat( itGrp->second.Type, *itM, ucstr.substr(startPos), sender );
					_DestUsers.push_back(TheDataset.getEntityId(*itM));
				}
			}
		}

		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					if (_SharedMessage)
					{
						CChatMessage forwarded = forwardedSharedMessage(*_SharedMessage, ucstr.substr(startPos), usedlang);
						IChatUnifierClient::getInstance()->sendFarGuildChatShared(charInfos->Name, uint32(grpId.getShortId()), forwarded);
					}
					else
						IChatUnifierClient::getInstance()->sendFarGuildChat(charInfos->Name, uint32(grpId.getShortId()), ucstr.substr(startPos));
				}
			}
		}
		else if (chatGrp.Type == CChatGroup::universe /*&& IsRingShard*/)
		{
			// forward universe chat to other shard with home session id
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					// determine the session id as the home session id for normal players and the current session id for GMs
					uint32 sessionId= (charInfos->HavePrivilege && !IsRingShard)? IService::getInstance()->getShardId(): (uint32)charInfos->HomeSessionId;
					if (_SharedMessage)
					{
						CChatMessage forwarded = forwardedSharedMessage(*_SharedMessage, ucstr.substr(startPos), usedlang);
						IChatUnifierClient::getInstance()->sendUniverseChatShared(charInfos->Name, sessionId, forwarded);
					}
					else
						IChatUnifierClient::getInstance()->sendUniverseChat(charInfos->Name, sessionId, ucstr.substr(startPos));
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}

} // chatInGroup //

void CChatManager::farChatInGroup(TGroupId &grpId, uint32 homeSessionId, const ucstring &text, const ucstring &senderName, uint32 senderCid)
{
	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{


		uint8 startPos = 0;
		string usedlang;
		if (EnableDeepL && text.length() > 4 && text[0] == ':' && text[3] == ':') // check lang prefix
		{
			usedlang = text.toString().substr(1, 2);
			startPos = 4;
		}

		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = chatGrp.Members.begin(); itM != chatGrp.Members.end(); ++itM )
		{
			CCharacterInfos *charInfo = IOS->getCharInfos(TheDataset.getEntityId(*itM));
			if (charInfo==NULL)
				continue;

			if (homeSessionId != 0)
			{
				// determine the session id as the home session id for normal players and the current session id for GMs
				uint32 sessionId= (charInfo->HavePrivilege && !IsRingShard)? IService::getInstance()->getShardId(): (uint32)charInfo->HomeSessionId;

				// check that the dest has the same home as sender
				if (sessionId != homeSessionId)
					continue;
			}

			if (EnableDeepL && !usedlang.empty())
			{
				TClientInfoCont::iterator itCl = _Clients.find(*itM);
				if (itCl == _Clients.end())
					continue;
				if (_SharedMessage && itCl->second->dontReceiveTranslation(_SharedMessage->SourceLanguage))
				{
					if (!_SharedMessage->TranslatedParts.empty() && !_SharedMessage->TranslationLanguage.empty())
						continue;
				}
				else if (usedlang != SM->getLanguageCodeString(charInfo->Language))
					continue;
			}

			sendFarChat(itGrp->second.Type, *itM, text.substr(startPos), senderName, CEntityId::Unknown, senderCid);
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}

//-----------------------------------------------
//	farChatInGroupShared
//
//-----------------------------------------------
void CChatManager::farChatInGroupShared(TGroupId &grpId, uint32 homeSessionId, const CChatMessage &message,
	const ucstring &senderName, uint32 senderCid)
{
	std::map<TGroupId, CChatGroup>::const_iterator group = _Groups.find(grpId);
	CMessageHistoryEntry *entry = group == _Groups.end() ? NULL : rememberMessage(message);
	if (entry)
	{
		if (group->second.Type == CChatGroup::universe)
			entry->Channel = "universe";
		else if (group->second.Type == CChatGroup::guild)
			entry->Channel = "guild:" + grpId.toString();
		else if (group->second.Type == CChatGroup::team)
			entry->Channel = "team:" + grpId.toString();
		else if (group->second.Type == CChatGroup::region)
			entry->Channel = "region:" + grpId.toString();
	}
	CChatMessage resolved = message;
	resolveChatMentions(resolved);
	CSharedMessageScope scope(_SharedMessage, resolved);
	ucstring text = sharedMessageDeliveryText(resolved);
	if (EnableDeepL && !message.TranslationLanguage.empty())
		text = ucstring(":" + message.TranslationLanguage + ":") + text;
	farChatInGroup(grpId, homeSessionId, text, senderName, senderCid);
}

//-----------------------------------------------
//	farDynChatShared
//
//-----------------------------------------------
void CChatManager::farDynChatShared(TChanID chanId, const ucstring &senderName, const CChatMessage &message)
{
	CDynChatChan *chan = _DynChat.getChan(chanId);
	if (chan == NULL)
	{
		nldebug("IOSCU : dynChanBroadcastShared : cannot find dynamic channel %s to broadcast chat", chanId.toString().c_str());
		return;
	}
	const std::string *name = _ChanNames.getB(chanId);
	CMessageHistoryEntry *entry = name ? rememberMessage(message) : NULL;
	if (entry)
		entry->Channel = "dyn:" + *name;

	CChatMessage resolved = message;
	resolveChatMentions(resolved);
	CSharedMessageScope scope(_SharedMessage, resolved);
	const ucstring text = sharedMessageDeliveryText(resolved);
	CDynChatSession *dcc = chan->getFirstSession();
	while (dcc)
	{
		NLMISC::CEntityId receiverId = TheDataset.getEntityId(dcc->getClient()->getID());
		CCharacterInfos *receiver = IOS->getCharInfos(receiverId);
		TClientInfoCont::iterator itCl = _Clients.find(dcc->getClient()->getID());
		if (itCl == _Clients.end())
		{
			dcc = dcc->getNextChannelSession();
			continue;
		}
		CChatClient &client = *itCl->second;
		bool canSend = true;
		if (EnableDeepL && !message.TranslationLanguage.empty())
		{
			if (client.dontReceiveTranslation(message.SourceLanguage))
				canSend = message.TranslatedParts.empty();
			else if (receiver && message.TranslationLanguage != SM->getLanguageCodeString(receiver->Language))
				canSend = false;
		}
		if (canSend)
			sendChat(CChatGroup::dyn_chat, dcc->getClient()->getID(), text, TDataSetRow(), chanId, senderName);
		dcc = dcc->getNextChannelSession();
	}
}


void CChatManager::chat2( const TDataSetRow& sender, const std::string &phraseId )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM: chat2 The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::say :
		case CChatGroup::shout :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
						itA != itCl->second->getAudience().Members.end();
							++itA )
				{
					sendChat2( itCl->second->getChatMode(), *itA, phraseId, sender );
				}
			}
			break;
		case CChatGroup::region :
			{
				// See comment in chat()
				TGroupId grpId = itCl->second->getRegionChatGroup();
				chatInGroup2( grpId, phraseId, sender );
			}
			break;


		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup,0);
				chatInGroup2( grpId, phraseId, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				chatInGroup2( grpId, phraseId, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				chatInGroup2( grpId, phraseId, sender );
			}
			break;


			// static group
		default :
			{
			nlwarning("<CChatManager::chat> client %s:%x chat in mode %u ! don't know how to handle it.",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				itCl->second->getChatMode());
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

}


void CChatManager::chatParam( const TDataSetRow& sender, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam>& params )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM: chat2 The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::say :
		case CChatGroup::shout :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
						itA != itCl->second->getAudience().Members.end();
							++itA )
				{
					sendChatParam( itCl->second->getChatMode(), *itA, phraseId, params, sender );
				}
			}
			break;
		case CChatGroup::region :
			{
				// See comment in chat()
				TGroupId grpId = itCl->second->getRegionChatGroup();
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;


		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup,0);
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;


			// static group
		default :
			{
			nlwarning("<CChatManager::chat> client %s:%x chat in mode %u ! don't know how to handle it.",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				itCl->second->getChatMode());
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

}


void CChatManager::chat2Ex( const TDataSetRow& sender, uint32 phraseId)
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM: chat2Ex The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::say :
		case CChatGroup::shout :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
				itA != itCl->second->getAudience().Members.end();
				++itA )
				{
						sendChat2Ex( itCl->second->getChatMode(), *itA, phraseId,sender );
				}
			}
			break;
		case CChatGroup::region :
			{
				// See comment in chat()
				TGroupId grpId = itCl->second->getRegionChatGroup();
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;

		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup,0);
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;


			// static group
		default :
			{
				nlwarning("<CChatManager::chat> client %s:%x chat in mode %u ! don't know how to handle it.",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					itCl->second->getChatMode());
				//				TGroupId grpId = (*itCl).second.getChatGroup();
				//				chatInGroup2( grpId, phraseId, sender );
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}
}


void CChatManager::chatInGroup2Ex( TGroupId& grpId, uint32 phraseId, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = itGrp->second.Members.begin(); itM != itGrp->second.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				const CEntityId &eid = TheDataset.getEntityId(*itM);
				// check the ai instance for region chat
				if (eid.getType() == RYZOMID::player && std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
					sendChat2Ex( itGrp->second.Type, *itM, phraseId, sender );
			}
		}

		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat2Ex(charInfos->Name, uint32(grpId.getShortId()), phraseId);
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}

void CChatManager::chatInGroup2( TGroupId& grpId, const std::string & phraseId, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = itGrp->second.Members.begin(); itM != itGrp->second.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				const CEntityId &eid = TheDataset.getEntityId(*itM);
				if (eid.getType() == RYZOMID::player && std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
					sendChat2( (*itGrp ).second.Type, *itM, phraseId, sender );
			}
		}
		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat2(charInfos->Name, uint32(grpId.getShortId()), phraseId);
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}

void CChatManager::chatParamInGroup( TGroupId& grpId, const std::string & phraseId, const std::vector<STRING_MANAGER::TParam>& params, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = itGrp->second.Members.begin(); itM != itGrp->second.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				const CEntityId &eid = TheDataset.getEntityId(*itM);
				if (eid.getType() == RYZOMID::player && std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
					sendChat2( (*itGrp ).second.Type, *itM, phraseId, sender );
			}
		}
		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat2(charInfos->Name, uint32(grpId.getShortId()), phraseId);
					if (!params.empty())
					{
						nlerror("Guild chat with params is not implemented yet");
					}
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}



void CChatManager::sendEmoteTextToAudience(  const TDataSetRow& sender,const std::string & phraseId, const TVectorParamCheck & params , const std::vector<TDataSetRow> & excluded )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
		// muted players can't do emotes (text)
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM:<CChatManager::sendEmoteTextToAudience> The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}

		// set the player chat mode and update its audience
		CChatGroup::TGroupType oldMode = itCl->second->getChatMode();
		TChanID	oldChan = itCl->second->getDynChatChan();
		itCl->second->setChatMode(CChatGroup::say);
		itCl->second->updateAudience();

		// get audience around the emoting player
		CChatGroup::TMemberCont::iterator itA;
		for( itA = itCl->second->getAudience().Members.begin();
		itA != itCl->second->getAudience().Members.end();
		++itA )
		{
			// ignore users in the excluded vector
			if ( std::find( excluded.begin(),excluded.end(), (*itA) ) == excluded.end() )
			{
				static ucstring ucstr = ucstring("");
				// the phrase
				uint32 sentId = STRING_MANAGER::sendStringToClient( *itA,phraseId.c_str(),params,&IosLocalSender );
				sendChat2Ex( CChatGroup::say, *itA, sentId, sender, ucstr, true );
			}
		}
		// restore old chat mode
		itCl->second->setChatMode( oldMode, oldChan );
	}
	else
	{
		nlwarning("<sendEmoteText> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}
}


//-----------------------------------------------
//		sendEmoteCustomTextToAll
//-----------------------------------------------
void CChatManager::sendEmoteCustomTextToAll( const TDataSetRow& sender, const ucstring & ustr )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
		// muted players can't do custom emotes
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM:<CChatManager::sendEmoteCustomTextToAll> The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}

		// set the player chat mode and update its audience
		CChatGroup::TGroupType oldMode = itCl->second->getChatMode();
		TChanID	oldChan = itCl->second->getDynChatChan();
		itCl->second->setChatMode(CChatGroup::say);
		itCl->second->updateAudience(); // Use the say audience to get the correct members
		itCl->second->setChatMode(CChatGroup::arround);

		// get audience around the emoting player
		CChatGroup::TMemberCont::iterator itA;
/*		for( itA = itCl->second->getAudience().Members.begin();
		itA != itCl->second->getAudience().Members.end();
		++itA )
		{
			sendChatCustomEmote( sender, *itA, ustr );
		}
		*/
		chat(sender, ustr);
		// restore old chat mode
		itCl->second->setChatMode( oldMode, oldChan );
	}
	else
	{
		nlwarning("<sendEmoteCustomTextToAll> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

}


//-----------------------------------------------
//	addDynStr
//
//-----------------------------------------------
//void CChatManager::addDynStr( const CEntityId& receiver, uint32 index, TServiceId frontendId )
//{
//	CDynamicStringInfos * infos = _DynDB.getInfos( index );
//	if( infos )
//	{
//		CMessage msgout( "IMPULS_CH_ID" );
//		CEntityId destId = receiver;
//		uint8 channel = 1;
//		msgout.serial( destId );
//		msgout.serial( channel );
//		CBitMemStream bms;
//
//		GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:ADD_DYN_STR", bms);
//
//		if( infos->IsHuffman )
//		{
//			bool huff = true;
//			bms.serialBit(huff);
//			bms.serial( index );
//			bms.serial( infos->Str );
//			vector<bool> code;
//			_DynDB.getHuffCode( infos->Str, code );
//			bms.serialCont( code );
//		}
//		else
//		{
//			bool huff = false;
//			bms.serialBit(huff);
//			bms.serial( index );
//			bms.serial( infos->Str );
//		}
//
////		nldebug("<CChatManager::addDynStr> sending association [%s,%d] to %s",infos->Str.c_str(),index,receiver.toString().c_str());
//		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
//		sendMessageViaMirror(frontendId, msgout);
//	}
//	else
//	{
//		nlwarning("<CChatManager::addDynStr> Can't find infos for string %d",index);
//	}
//
//
//} // addDynStr //



//-----------------------------------------------
//	sendChat
//
//-----------------------------------------------
void CChatManager::sendChat( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const ucstring& ucstr, const TDataSetRow &sender, TChanID chanID, const ucstring &senderName)
{


	if (senderChatMode == CChatGroup::arround)
	{
		sendChatCustomEmote(sender, receiver, ucstr );
		return;
	}

	//if( receiver != sender )
	{
		CCharacterInfos * charInfos = NULL;
		if( sender.isValid() /* != CEntityId::Unknown*/ )
		{
			charInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
			if( charInfos == NULL )
			{
				nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex());
				return;
			}
		}
		CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
		if( receiverInfos )
		{
			TClientInfoCont::iterator itCl = _Clients.find( receiver );
			if( itCl != _Clients.end() )
			{
				if (itCl->second->getId().getType() == RYZOMID::player)
				{
					bool havePriv = false;
					if (charInfos && charInfos->HavePrivilege)
					{
						havePriv = true;
					}
					if ( ! havePriv && itCl->second->isInIgnoreList(sender))
					{
						return;
					}

					uint32 senderNameIndex;
					// if the sender exists
					if( charInfos )
					{
						senderNameIndex = charInfos->NameIndex;
					}
					else
					{
						// if no sender, we use a special name
						ucstring senderName("<BROADCAST MESSAGE>");
						senderNameIndex = SM->storeString( senderName );
					}

					if (!senderName.empty())
					{
						// the sender overloaded the name
						senderNameIndex = SM->storeString( senderName );
					}

					// send the string to FE
					CMessage msgout( "IMPULS_CH_ID" );
//					CEntityId& destId = receiver;
					uint8 channel = 1;
					CEntityId eid = TheDataset.getEntityId(receiver);
					msgout.serial( eid );
					msgout.serial( channel );
					CBitMemStream bms;
					GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT", bms );

					CChatMsg chatMsg;
					chatMsg.CompressedIndex = sender.getCompressedIndex();
					chatMsg.SenderNameId = senderNameIndex;
					chatMsg.ChatMode = (uint8) senderChatMode;
					if (senderChatMode == CChatGroup::dyn_chat)
					{
						chatMsg.DynChatChanID = chanID;
					}
					chatMsg.Content = ucstr;
					bms.serial( chatMsg );
					CDynChatChan *dynChannel = senderChatMode == CChatGroup::dyn_chat ? _DynChat.getChan(chanID) : NULL;
					if (_SharedMessage)
					{
						appendMessageTrailer(bms, *_SharedMessage, ucstr, *receiverInfos,
							false, ucstring(), dynChannel && dynChannel->HideBubble);
					}

	/*				nldebug("<CChatManager::sendChat> Sending dynamic chat '%s' from client %d to client %s with chat mode %d",
						chatMsg.Content.toString().c_str(),
						chatMsg.Sender,
						receiver.toString().c_str(),
						chatMsg.ChatMode);
	*/
					msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
					sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
					recordSharedReceiver(receiverInfos->EntityId, senderChatMode);
				}
			}
			else
			{
				nlwarning("<CChatManager::sendChat> client %s:%x is unknown",
					TheDataset.getEntityId(receiver).toString().c_str(),
					receiver.getIndex());
			}
		}
		else
		{
			nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}

} // sendChat //

void CChatManager::sendFarChat(const string &name, const ucstring& ucstr, const string &chan, uint32 senderCid)
{
	const TChanID *chanId = _ChanNames.getA(chan);

	if (chan == "universe")
	{
		TGroupId grpId = CEntityId(RYZOMID::chatGroup, 0);
		farChatInGroup(grpId, 0, ucstr, ucstring("~"+name), senderCid);
	}
	else if (chan.substr(0, 6) == "guild:")
	{
		TGroupId grpId = CEntityId::Unknown;
		grpId.fromString(chan.substr(6).c_str());
		farChatInGroup(grpId, 0, ucstr, ucstring("~"+name), senderCid);
	}
	else if (chan.substr(0, 5) == "tell:")
	{
		nlinfo("here:[%s]", ucstr.toUtf8().c_str());
		farTell(CEntityId::Unknown, ucstring("~"+name), false, ucstring(chan.substr(5)), ucstr);
	}
	else if (chanId)
	{

		uint8 startPos = 0;
		string usedlang;
		ucstring text = ucstr;
		if (EnableDeepL && ucstr.length() > 4 && ucstr[0] == ':' && ucstr[3] == ':') // check lang prefix
		{
			usedlang = ucstr.toString().substr(1, 2);
			text = ucstr.substr(4);
		}

		CDynChatSession *dcc = _DynChat.getChan(*chanId)->getFirstSession();
		while (dcc)
		{
			NLMISC::CEntityId receiverId = TheDataset.getEntityId(dcc->getClient()->getID());
			CCharacterInfos* charInfo = IOS->getCharInfos(receiverId);
			nlinfo("%s vs %s", usedlang.c_str(), SM->getLanguageCodeString(charInfo->Language).c_str());
			TClientInfoCont::iterator itCl = _Clients.find(dcc->getClient()->getID());
			if (itCl == _Clients.end())
			{
				dcc = dcc->getNextChannelSession();
				continue;
			}
			bool canSend = !EnableDeepL || usedlang.empty() || usedlang == SM->getLanguageCodeString(charInfo->Language);
			if (_SharedMessage && EnableDeepL && !usedlang.empty() &&
				itCl->second->dontReceiveTranslation(_SharedMessage->SourceLanguage))
				canSend = _SharedMessage->TranslatedParts.empty();
			if (canSend)
				sendFarChat((CChatGroup::TGroupType)12, dcc->getClient()->getID(), text, ucstring("~"+name), *chanId, senderCid);
			dcc = dcc->getNextChannelSession();
		}
	}
}

//-----------------------------------------------
//	bridgeChat
//
//-----------------------------------------------
bool CChatManager::bridgeChat(const std::string &sender, const std::string &channel,
	const std::string &externalId, const std::string &messageId, const std::string &quoteId,
	const std::string &sourceLanguage, const std::string &targetLanguage, const ucstring &text,
	const std::string &translatedParts)
{
	// Reject malformed payloads, unknown channels and mismatched message references.
	if (sender.empty() || sender.size() > CHAT_MESSAGE::MaxReceiverLength ||
		channel.empty() || channel.size() > CHAT_MESSAGE::MaxReceiverLength + 5 ||
		externalId.size() > CHAT_MESSAGE::MaxSerializedSize ||
		sourceLanguage.size() != 2 ||
		(!targetLanguage.empty() && targetLanguage.size() != 2) ||
		text.empty() || text.size() > CHAT_MESSAGE::MaxSerializedSize / sizeof(ucchar))
		return false;

	const std::string::size_type separator = channel.find(':');
	const std::string group = channel.substr(0, separator);
	CChatGroup::TGroupType mode = CChatGroup::stringToGroupType(group);
	TChanID dynamicChannel = CEntityId::Unknown;
	if (group == "dyn")
	{
		if (separator == std::string::npos || separator + 1 == channel.size())
			return false;
		dynamicChannel = getChanId(channel.substr(separator + 1));
		if (_DynChat.getChan(dynamicChannel) == NULL)
			return false;
		mode = CChatGroup::dyn_chat;
	}
	else if (mode == CChatGroup::universe || mode == CChatGroup::guild ||
		mode == CChatGroup::team || mode == CChatGroup::region)
	{
		TGroupId groupId = CEntityId(RYZOMID::chatGroup, 0);
		if (mode == CChatGroup::universe)
		{
			if (channel != "universe")
				return false;
		}
		else
		{
			if (separator == std::string::npos || separator + 1 == channel.size())
				return false;
			groupId.fromString(channel.substr(separator + 1).c_str());
			if (groupId.toString() != channel.substr(separator + 1))
				return false;
		}
		std::map<TGroupId, CChatGroup>::const_iterator found = _Groups.find(groupId);
		if (found == _Groups.end() || found->second.Type != mode)
			return false;
	}
	else if (mode == CChatGroup::tell)
	{
		if (group != "tell" || separator == std::string::npos || separator + 1 == channel.size() ||
			IOS->getCharInfos(ucstring::makeFromUtf8(channel.substr(separator + 1))) == NULL)
			return false;
	}
	else if ((mode != CChatGroup::say && mode != CChatGroup::shout) ||
		separator != std::string::npos || channel != group)
		return false;

	CChatMessage message;
	CMessageHistoryEntry *entry = NULL;
	if (!messageId.empty())
	{
		entry = findMessage(messageId);
		if (!entry || entry->Channel != channel || entry->ExternalId != externalId ||
			entry->Message.Quote.MessageId != quoteId ||
			entry->Message.SourceLanguage != sourceLanguage || !entry->Message.AllowTranslation ||
			targetLanguage.empty() || targetLanguage == sourceLanguage)
			return false;
		message = entry->Message;
		message.TranslationLanguage = targetLanguage;
		message.TranslatedParts.clear();
		if (!translatedParts.empty())
		{
			// count: followed by length:UTF-8 bytes per part; references carry length 0.
			std::string::size_type offset = 0;
			uint32 count;
			if (!readTranslatedPartLength(translatedParts, offset, count) || count != message.Parts.size())
				return false;
			message.TranslatedParts = message.Parts;
			for (uint i = 0; i < message.TranslatedParts.size(); ++i)
			{
				uint32 length;
				if (!readTranslatedPartLength(translatedParts, offset, length) || length > translatedParts.size() - offset)
					return false;
				CChatMessagePart &part = message.TranslatedParts[i];
				if (part.Type == CChatMessagePart::Text)
				{
					const std::string value = translatedParts.substr(offset, length);
					part.TextValue.fromUtf8(value);
					if (part.TextValue.toUtf8() != value)
						return false;
				}
				else if (length != 0)
					return false;
				offset += length;
			}
			if (offset != translatedParts.size())
				return false;
			CChatMessage visible = message;
			visible.Parts = message.TranslatedParts;
			if (sharedMessageLogText(visible) != text)
				return false;
		}
		else
		{
			if (message.Parts.size() != 1 || message.Parts[0].Type != CChatMessagePart::Text)
				return false;
			CChatMessagePart translated;
			translated.TextValue = text;
			message.TranslatedParts.push_back(translated);
		}
		if (!message.isValid())
			return false;
	}
	else
	{
		if (externalId.empty() || !targetLanguage.empty() || !translatedParts.empty())
			return false;
		std::map<std::string, std::string>::const_iterator duplicate = _ExternalMessageIds.find(externalId);
		if (duplicate != _ExternalMessageIds.end())
		{
			entry = findMessage(duplicate->second);
			if (!entry || entry->Channel != channel || entry->Message.SenderName.toUtf8() != sender ||
				entry->Message.Quote.MessageId != quoteId)
				return false;
			_Log.displayNL("chat_bridge|1|%s|%s", base64::encode(externalId).c_str(), duplicate->second.c_str());
			return true;
		}
		if (mode == CChatGroup::say || mode == CChatGroup::shout ||
			mode == CChatGroup::team || mode == CChatGroup::region)
			return false;
		message.MessageId = newMessageId();
		message.SenderName.fromUtf8(sender);
		message.Timestamp = CTime::getSecondsSince1970();
		message.SourceLanguage = sourceLanguage;
		message.AllowTranslation = EnableDeepL && sourceLanguage != "wk";
		CChatMessagePart original;
		original.TextValue = text;
		message.Parts.push_back(original);
		message.Quote.MessageId = quoteId;
		if (!resolveQuote(message, channel, CEntityId::Unknown,
			mode == CChatGroup::tell ? channel.substr(separator + 1) : std::string()))
			return false;
		if (!message.isValid())
			return false;
	}
	CMemStream serialized;
	serialized.serial(message);
	if (serialized.length() > CHAT_MESSAGE::MaxSerializedSize)
		return false;

	ucstring routed;
	if (!targetLanguage.empty())
		routed.fromUtf8(":" + targetLanguage + ":{:" + sourceLanguage + ": " +
			sharedMessageLogText(message).toUtf8() + "}@{ " + text.toUtf8());
	else if (message.AllowTranslation)
		routed.fromUtf8(":" + sourceLanguage + ": " + text.toUtf8());
	else
		routed = text;

	CSharedMessageScope scope(_SharedMessage, message);
	CCharacterInfos *infos = message.SenderId != CEntityId::Unknown ? IOS->getCharInfos(message.SenderId) : NULL;
	if (!infos && (mode == CChatGroup::say || mode == CChatGroup::shout ||
		mode == CChatGroup::team || mode == CChatGroup::region))
		return false;
	if (infos)
	{
		if (_MutedUsers.find(message.SenderId) != _MutedUsers.end())
			return false;
		CChatClient &client = getClient(infos->DataSetIndex);
		if ((mode == CChatGroup::team && channel != "team:" + client.getTeamChatGroup().toString()) ||
			(mode == CChatGroup::guild && channel != "guild:" + client.getGuildChatGroup().toString()) ||
			(mode == CChatGroup::region && channel != "region:" + client.getRegionChatGroup().toString()))
			return false;
		if (mode == CChatGroup::dyn_chat)
		{
			CDynChatSession *session = _DynChat.getSession(dynamicChannel, infos->DataSetIndex);
			if (!session || !session->WriteRight)
				return false;
		}
		if (mode == CChatGroup::tell)
			tell(infos->DataSetIndex, channel.substr(separator + 1), routed);
		else
		{
			const CChatGroup::TGroupType previousMode = client.getChatMode();
			const TChanID previousChannel = client.getDynChatChan();
			client.setChatMode(mode, dynamicChannel);
			client.updateAudience();
			chat(infos->DataSetIndex, ucstring(">") + routed, true);
			client.setChatMode(previousMode, previousChannel);
			client.updateAudience();
		}
	}
	else
		sendFarChat(sender, routed, mode == CChatGroup::dyn_chat ? channel.substr(separator + 1) : channel);

	if (messageId.empty())
	{
		entry = rememberMessage(message);
		if (!entry)
			return false;
		entry->Channel = channel;
		entry->ExternalId = externalId;
		_ExternalMessageIds[externalId] = message.MessageId;
		_Log.displayNL("chat_bridge|1|%s|%s", base64::encode(externalId).c_str(), message.MessageId.c_str());
	}
	return true;
}

void CChatManager::sendFarChat( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const ucstring& ucstr, const ucstring &senderName, TChanID chanID, uint32 senderCid)
{
	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiver );
		if( itCl != _Clients.end() )
		{
			if (itCl->second->getId().getType() == RYZOMID::player)
			{
				if (senderCid > 0 && itCl->second->isInIgnoreList(senderCid))
					return;

				uint32 senderNameIndex = SM->storeString( senderName );

				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
//					CEntityId& destId = receiver;
				uint8 channel = 1;
				CEntityId eid = TheDataset.getEntityId(receiver);
				msgout.serial( eid );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT", bms );

				CChatMsg chatMsg;
				chatMsg.CompressedIndex = 0xFFFFF;
				chatMsg.SenderNameId = senderNameIndex;
				chatMsg.ChatMode = (uint8) senderChatMode;
				if (senderChatMode == CChatGroup::dyn_chat)
				{
					chatMsg.DynChatChanID = chanID;
				}
				chatMsg.Content = ucstr;
				bms.serial( chatMsg );
				CDynChatChan *dynChannel = senderChatMode == CChatGroup::dyn_chat ? _DynChat.getChan(chanID) : NULL;
				if (_SharedMessage)
				{
					appendMessageTrailer(bms, *_SharedMessage, ucstr, *receiverInfos,
						false, ucstring(), dynChannel && dynChannel->HideBubble, senderName);
				}

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
				recordSharedReceiver(receiverInfos->EntityId, senderChatMode);
			}
		}
		else
		{
			nlwarning("<CChatManager::sendChat> client %s:%x is unknown",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}

}


//-----------------------------------------------
//	sendChat2
//
//-----------------------------------------------
void CChatManager::sendChat2( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const std::string &phraseId, const TDataSetRow &sender )
{
	// send the chat phrase to the client
	TVectorParamCheck params;
	params.resize(1);
	params.back().Type = STRING_MANAGER::bot;
	params.back().setEId(  TheDataset.getEntityId(sender) );

	uint32 id = STRING_MANAGER::sendStringToClient(receiver, phraseId, params, &IosLocalSender);
	sendChat2Ex( senderChatMode, receiver, id, sender );
}


//-----------------------------------------------
//	sendChatParam
//
//-----------------------------------------------
void CChatManager::sendChatParam( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam>& params, const TDataSetRow &sender )
{
	TVectorParamCheck params2;
	params2.resize( params.size() + 1);
	// send the chat phrase to the client
	params2[0].Type = STRING_MANAGER::bot;
	params2[0].setEId(  TheDataset.getEntityId(sender) );
	uint32 first = 0, last = (uint32)params.size();
	for ( ; first != last ; ++first)
	{
		params2[first + 1] = params[first];
	}

	uint32 id = STRING_MANAGER::sendStringToClient(receiver, phraseId, params2, &IosLocalSender);
	sendChat2Ex( senderChatMode, receiver, id, sender );
}


//-----------------------------------------------
//	sendChat2Ex
//
//-----------------------------------------------
void CChatManager::sendChat2Ex( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, uint32 phraseId, const TDataSetRow &sender, ucstring customTxt, bool isEmote )
{
	CCharacterInfos * charInfos = NULL;
	if( sender.isValid() /* != CEntityId::Unknown*/ )
	{
		charInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
		if( charInfos == NULL )
		{
			nlwarning("<CChatManager::sendChat2Ex> The character %s:%x is unknown, no chat msg sent",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
	}
	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiver );
		if( itCl != _Clients.end() )
		{
			if (itCl->second->getId().getType() == RYZOMID::player)
			{
				bool havePriv = false;
				if (charInfos && charInfos->HavePrivilege)
				{
					havePriv = true;
				}
				if ( ! havePriv && itCl->second->isInIgnoreList(sender))
				{
					return;
				}

				// send the chat phrase to the client
				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				CEntityId destId = receiverInfos->EntityId;
				uint8 channel = 1;
				msgout.serial( destId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT2", bms );

				CChatMsg2 chatMsg;
				if (isEmote)
				{
					TDataSetRow senderFake = TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW );
					chatMsg.CompressedIndex = senderFake.getCompressedIndex();
					chatMsg.SenderNameId = 0;
				}
				else
				{
					chatMsg.CompressedIndex = sender.getCompressedIndex();
					chatMsg.SenderNameId = charInfos ? charInfos->NameIndex : 0; // empty string if there is no sender
				}
				chatMsg.ChatMode = (uint8) senderChatMode;
				chatMsg.PhraseId = phraseId;
				chatMsg.CustomTxt = customTxt;
				bms.serial( chatMsg );

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::sendChat2Ex> client %s:%x is unknown",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::sendChat2Ex> The character %s:%x is unknown, no chat msg sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}
}//	sendChat2Ex


//-----------------------------------------------
//	sendChatCustomEmote
//
//-----------------------------------------------
void CChatManager::sendChatCustomEmote( const TDataSetRow &sender, const TDataSetRow &receiver, const ucstring& ucstr )
{
	TDataSetRow senderFake = TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW );

	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	CCharacterInfos * senderInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiver );
		if( itCl != _Clients.end() )
		{
			if (itCl->second->getId().getType() == RYZOMID::player)
			{
				bool havePriv = false;
				if (senderInfos && senderInfos->HavePrivilege)
				{
					havePriv = true;
				}
				if ( ! havePriv && itCl->second->isInIgnoreList(sender))
				{
					return;
				}

				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				CEntityId eid = TheDataset.getEntityId(receiver);
				msgout.serial( eid );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT", bms );

				CChatMsg chatMsg;
				chatMsg.CompressedIndex = senderFake.getCompressedIndex();
				chatMsg.SenderNameId = 0;
				chatMsg.ChatMode = (uint8) CChatGroup::say;
				chatMsg.Content = ucstr;
				bms.serial( chatMsg );

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::sendChatCustomEmote> client %s:%x is unknown",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}

} // sendChatCustomEmote //


//-----------------------------------------------
//	tell
//
//-----------------------------------------------
void CChatManager::tell2( const TDataSetRow& sender, const TDataSetRow& receiver, const string& phraseId )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl == _Clients.end() )
	{
		nlwarning("<CChatManager::tell> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return;
	}
	CCharacterInfos * senderInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
	if( senderInfos == 0 )
	{
		nlwarning("<CChatManager::tell> The sender %s:%x is unknown, no tell message sent",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return;
	}

//	bool senderMuted = itCl->second->isMuted();
	bool senderMuted = _MutedUsers.find(TheDataset.getEntityId(sender)) != _MutedUsers.end();
	bool receiverMuted = _MutedUsers.find(TheDataset.getEntityId(receiver)) != _MutedUsers.end();

	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	if( receiverInfos )
	{
		if(	senderMuted && receiverInfos->HavePrivilege == false )
		{
			nldebug("IOSCM: tell2 The player %s:%x is muted and %s have no privilege",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				TheDataset.getEntityId(receiver).toString().c_str() );
			return;
		}
		itCl = _Clients.find( receiverInfos->DataSetIndex );
		if( itCl != _Clients.end() )
		{
			if( receiverMuted && senderInfos->HavePrivilege == false )
			{
				nldebug("IOSCM: tell2 The player %s:%x have no privilege and %s is muted",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					TheDataset.getEntityId(receiver).toString().c_str() );
				return;
			}

			// check if the sender is CSR or is not in the ignore list of the receiver
			if(senderInfos->HavePrivilege || !itCl->second->isInIgnoreList(sender) )
			{
				// send the chat phrase to the client
				TVectorParamCheck params;
				params.resize(1);
				params.back().Type = STRING_MANAGER::bot;
				params.back().setEId( TheDataset.getEntityId(sender));
				uint32 id = STRING_MANAGER::sendStringToClient(receiver, phraseId, params, &IosLocalSender);

				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				msgout.serial( receiverInfos->EntityId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:TELL2", bms);

				bms.serial( senderInfos->NameIndex );
				bms.serial( id);

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::tell> client %s:%x is unknown",
				TheDataset.getEntityId(itCl->first).toString().c_str(),
				itCl->first.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::tell> The receiver %s:%x is unknown, no tell message sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}
} // tell2 //


//-----------------------------------------------
//	tell
//
//-----------------------------------------------
bool CChatManager::tell( const TDataSetRow& sender, const string& receiverIn, const ucstring& ucstr )
{
	const bool structuredMessage = _SharedMessage != NULL;
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	CChatMessage legacyMessage;
	const CChatMessage *legacyMetadata = NULL;
	if (!_SharedMessage && itCl != _Clients.end() && prepareLegacyMessage(sender, ucstr, legacyMessage))
		legacyMetadata = &legacyMessage;
	CSharedMessageScope legacyScope(_SharedMessage, legacyMetadata);
	if( itCl == _Clients.end() )
	{
		nlwarning("<CChatManager::tell> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return false;
	}
	CCharacterInfos * senderInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
	if( senderInfos == 0 )
	{
		nlwarning("<CChatManager::tell> The sender %s:%x is unknown, no tell message sent",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return false;
	}
//	bool senderMuted = itCl->second->isMuted();
	bool senderMuted = _MutedUsers.find(TheDataset.getEntityId(sender)) != _MutedUsers.end();

	// manage domain wide addressing
	string receiver;
	TSessionId receiverSessionId;

	CShardNames::getInstance().parseRelativeName(senderInfos->HomeSessionId, receiverIn, receiver, receiverSessionId);

	receiver = CShardNames::getInstance().makeFullName(receiver, receiverSessionId);
	CCharacterInfos * receiverInfos = IOS->getCharInfos( receiver );

	if( receiverInfos && !ForceFarChat)
	{
		bool receiverMuted = _MutedUsers.find(TheDataset.getEntityId(receiverInfos->DataSetIndex)) != _MutedUsers.end();
		if(	senderMuted && receiverInfos->HavePrivilege == false )
		{
			nldebug("IOSCM: tell The player %s:%x is muted and %s have no privilege",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				receiver.c_str());
			return false;
		}
		itCl = _Clients.find( receiverInfos->DataSetIndex );
		if( itCl != _Clients.end() )
		{
			if( receiverMuted && senderInfos->HavePrivilege == false )
			{
				nldebug("IOSCM: tell The player %s:%x have no privilege and %s is muted",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					receiver.c_str());
				return false;
			}

			// check if the sender is not in the ignore list of the receiver
			if(senderInfos->HavePrivilege || !itCl->second->isInIgnoreList(sender) )
			{
				// check if user is afk
				if ( receiverInfos->DataSetIndex.isValid() && TheDataset.isDataSetRowStillValid( receiverInfos->DataSetIndex ) )
				{
					CMirrorPropValue<uint16> mirrorValue( TheDataset, receiverInfos->DataSetIndex, DSPropertyCONTEXTUAL );
					CProperties prop(mirrorValue);
					if ( prop.afk() )
					{
						// send special message to user
						SM_STATIC_PARAMS_1( vect, STRING_MANAGER::player );
						vect[0].setEId( receiverInfos->EntityId );
						uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_AFK", vect, &IosLocalSender );
						sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId, TDataSetRow(), receiverInfos->AfkCustomTxt );
					}
					if ( _UsersIgnoringTells.find( receiverInfos->EntityId ) != _UsersIgnoringTells.end() )
					{
						// send special message to user (same message as if the receiver was offline)
						SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
						vect[0].Literal = ucstring( receiver );
						uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
						sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
						return false;
					}
				}

				// if the client doesn't know this dynamic string(name of sender), we send it to him
				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				msgout.serial( receiverInfos->EntityId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:TELL", bms);

				TDataSetIndex dsi = senderInfos->DataSetIndex.getCompressedIndex();
				bms.serial( dsi );
				bms.serial( senderInfos->NameIndex );
				bms.serial( const_cast<ucstring&>(ucstr) );
				if (_SharedMessage)
					appendMessageTrailer(bms, *_SharedMessage, ucstr, *receiverInfos);

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
				if (_SharedMessage)
				{
					CMessageHistoryEntry *entry = rememberMessage(*_SharedMessage);
					if (entry)
						entry->Channel = "tell:" + receiver;
				}
				recordSharedReceiver(receiverInfos->EntityId, CChatGroup::tell);

				if (structuredMessage)
					echoTellShared(senderInfos->EntityId, receiverInfos->Name, *_SharedMessage);

				// log tell to PDS
//				IOSPD::logTell(ucstr, senderInfos->EntityId, receiverInfos->EntityId);
				log_Chat_Tell(senderInfos->EntityId, receiverInfos->EntityId, ucstr.toUtf8());
				return true;
			}
			return false;
		}
		else
		{
			nlwarning("<CChatManager::tell> client %s:%x is unknown",
				TheDataset.getEntityId(receiverInfos->DataSetIndex).toString().c_str(),
				receiverInfos->DataSetIndex.getIndex());
			return false;
		}
	}
	else
	{
		// look for named group chat
		std::map<TStringId, TGroupId>::iterator it(_GroupNames.find(CStringMapper::map(receiver)));
		if (it != _GroupNames.end())
		{
			// we found one
			if(	senderMuted )
			{
				nldebug("IOSCM: tell The player %s:%x is muted, can't tell a group chat",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex());
					return false;
			}

			CChatGroup &chatGroup = _Groups[it->second];

			//chatInGroup(it->second, str, sender);	// removed, checked after

			/// check that sender is in this group
			if (chatGroup.Members.find(sender) != chatGroup.Members.end())
			{
				chatInGroup(it->second, ucstr, sender);

				// log tell to PDS
//				IOSPD::logTell(ucstr, senderInfos->EntityId, it->second);
				log_Chat_Tell(senderInfos->EntityId, it->second, ucstr.toUtf8());
				return true;
			}
			else
			{
				// ERROR : not in this chat group !!!!!
				nlwarning("<CChatManager::tell> The receiver %s is unknown, no tell message sent",receiver.c_str());

				SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
				vect[0].Literal = ucstring( receiver );
				uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
				sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
				return false;
			}
		}
		else if (IChatUnifierClient::getInstance() != NULL)
		{
			if (senderMuted && receiverInfos && receiverInfos->HavePrivilege == false)
			{
				nldebug("IOSCM: tell The player %s:%x is muted and %s have no privilege",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					receiver.c_str());
				return false;
			}
			string senderName = senderInfos->Name.toString();
			string::size_type p0 = senderName.find('(');
			if (p0 != string::npos)
				senderName = senderName.substr(0, p0);

			if (structuredMessage)
			{
				if (!IChatUnifierClient::getInstance()->sendFarTellShared(senderInfos->EntityId, senderInfos->HavePrivilege, ucstring(receiver), *_SharedMessage))
					return false;
				echoTellShared(senderInfos->EntityId, ucstring(receiver), *_SharedMessage);
			}
			else if (_SharedMessage)
				logSharedMessage(*_SharedMessage, "tell:" + receiver, _SharedMessage->SourceLanguage);
			else
				_Log.displayNL("tell:%s|%s|%d|%s|%s", receiverIn.c_str(), senderName.c_str(), 1, "*", ucstr.toUtf8().c_str() );
			return true;
		}
		else
		{
			SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
			vect[0].Literal = ucstring( receiver );
			uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
			sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
		}
	}
	return false;
} // tell //

//-----------------------------------------------
//	tellShared
//
//-----------------------------------------------
bool CChatManager::tellShared(const TDataSetRow &sender, const std::string &receiver, const CChatMessage &message)
{
	CChatMessage prepared = message;
	if (!prepareSharedMessage(sender, prepared, false, CChatGroup::tell))
		return false;
	CCharacterInfos *senderInfos = IOS->getCharInfos(prepared.SenderId);
	if (!senderInfos)
		return false;
	const std::string target = CShardNames::getInstance().makeFullNameFromRelative(senderInfos->HomeSessionId, receiver);
	if (!resolveQuote(prepared, "tell:" + target, prepared.SenderId, target) || !prepared.isValid())
		return false;
	CSharedMessageScope scope(_SharedMessage, prepared);
	return tell(sender, receiver, sharedMessageLogText(prepared));
}

//-----------------------------------------------
//	echoTellShared
//
//-----------------------------------------------
void CChatManager::echoTellShared(const NLMISC::CEntityId &senderCharId, const ucstring &receiver, const CChatMessage &message)
{
	CCharacterInfos *senderInfos = IOS->getCharInfos(senderCharId);
	if (senderInfos == NULL)
		return;

	CMessage echo("IMPULS_CH_ID");
	uint8 channel = 1;
	echo.serial(senderInfos->EntityId);
	echo.serial(channel);
	CBitMemStream stream;
	GenericXmlMsgHeaderMngr.pushNameToStream("STRING:TELL", stream);
	TDataSetIndex compressedIndex = senderInfos->DataSetIndex.getCompressedIndex();
	uint32 receiverNameIndex = SM->storeString(receiver);
	stream.serial(compressedIndex);
	stream.serial(receiverNameIndex);
	ucstring text = sharedMessageLogText(message);
	stream.serial(text);
	appendMessageTrailer(stream, message, text, *senderInfos, true, receiver);
	echo.serialBufferWithSize((uint8*)stream.buffer(), stream.length());
	sendMessageViaMirror(TServiceId(senderInfos->EntityId.getDynamicId()), echo);
	CMessageHistoryEntry *entry = rememberMessage(message);
	if (entry)
	{
		entry->Channel = "tell:" + receiver.toUtf8();
		entry->Receivers.insert(senderInfos->EntityId);
	}
}


void CChatManager::farTell( const NLMISC::CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring& receiver, const ucstring& ucstr  )
{
	CCharacterInfos * receiverInfos = IOS->getCharInfos( receiver );
	if( receiverInfos )
	{
		nlinfo("found receiver");
		TClientInfoCont::iterator itCl = _Clients.find( receiverInfos->DataSetIndex );
		if( itCl != _Clients.end() )
		{
			nlinfo("found client");
			bool receiverMuted = _MutedUsers.find(receiverInfos->EntityId) != _MutedUsers.end();
			if( receiverMuted && havePrivilege == false )
			{
				nldebug("IOSCM: tell The player %s have no privilege and %s is muted",
					senderName.toUtf8().c_str(),
					receiver.toUtf8().c_str());
				return;
			}

			CCharacterInfos * senderInfos = IOS->getCharInfos(senderCharId);
			// check if the sender is CSR is not in the ignore list of the receiver
			if (!senderInfos || senderInfos->HavePrivilege || !itCl->second->isInIgnoreList(senderCharId))
			{
				nlinfo("found senderInfos");
				// info for log the chat message
				string receiverName = receiverInfos->Name.toString();

				// if the client doesn't know this dynamic string(name of sender), we send it to him
				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				msgout.serial( receiverInfos->EntityId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:FAR_TELL", bms);

				CFarTellMsg ftm;
				ftm.SenderName = senderName;
				ftm.Text = ucstr;
				ftm.serial(bms);
				if (_SharedMessage)
					appendMessageTrailer(bms, *_SharedMessage, ucstr, *receiverInfos,
						false, ucstring(), false, senderName);

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
				recordSharedReceiver(receiverInfos->EntityId, CChatGroup::tell);

				// log tell to PDS
//				IOSPD::logTell(ucstr, senderCharId, receiverInfos->EntityId);
				log_Chat_Tell(senderCharId, receiverInfos->EntityId, ucstr.toUtf8());
			}
		}
		else
		{
			// no chat unifier, so we can't dispatch
			nlwarning("<CChatManager::tell> client %s:%x is unknown",
				senderCharId.toString().c_str(),
				itCl->first.getIndex());
		}
	}
} // tell //

//-----------------------------------------------
//	farTellShared
//
//-----------------------------------------------
void CChatManager::farTellShared(const NLMISC::CEntityId &senderCharId, const ucstring &senderName,
	bool havePrivilege, const ucstring &receiver, const CChatMessage &message)
{
	CMessageHistoryEntry *entry = rememberMessage(message);
	if (entry)
		entry->Channel = "tell:" + receiver.toUtf8();
	CChatMessage resolved = message;
	resolveChatMentions(resolved);
	CSharedMessageScope scope(_SharedMessage, resolved);
	farTell(senderCharId, senderName, havePrivilege, receiver, sharedMessageLogText(resolved));
}

/*
 * Display the list of clients
 */
void CChatManager::displayChatClients(NLMISC::CLog &log)
{
	TClientInfoCont::iterator im;
	for ( im=_Clients.begin(); im!=_Clients.end(); ++im )
	{
		CCharacterInfos *ci = IOS->getCharInfos(im->second->getId());
		if (ci != NULL)
		{
			if (ci->EntityId.getType() == RYZOMID::player)
				log.displayNL("'%s' %s:%x %s mode '%s'",
					ci->Name.toString().c_str(),
					ci->EntityId.toString().c_str(),
					im->first.getIndex(),
					im->second->isMuted()?"(muted)":"",
					CChatGroup::groupTypeToString(im->second->getChatMode()).c_str() );
		}
		else
		{
			log.displayNL("*no name* %s:%x %s mode '%s'",
				ci->EntityId.toString().c_str(),
				im->first.getIndex(),
				im->second->isMuted()?"(muted)":"",
				CChatGroup::groupTypeToString(im->second->getChatMode()).c_str() );
		}
	}
}


void CChatManager::displayChatGroup(NLMISC::CLog &log, TGroupId gid, CChatGroup &chatGroup)
{
	if (chatGroup.GroupName == CStringMapper::emptyId())
	{
		log.displayNL("Group : anonym (%s), %s : %u clients :",
				gid.toString().c_str(),
				CChatGroup::groupTypeToString(chatGroup.Type).c_str(),
				chatGroup.Members.size());
	}
	else
	{
		log.displayNL("Group : '%s' (%s), %s : %u clients :",
				CStringMapper::unmap(chatGroup.GroupName).c_str(),
				gid.toString().c_str(),
				CChatGroup::groupTypeToString(chatGroup.Type).c_str(),
				chatGroup.Members.size());
	}

	CChatGroup::TMemberCont::iterator first(chatGroup.Members.begin()), last(chatGroup.Members.end());
	for (; first != last; ++first)
	{
		CEntityId eid = TheDataset.getEntityId(*first);
		if (eid.getType() == RYZOMID::player)
		{
			CCharacterInfos *ci = IOS->getCharInfos(TheDataset.getEntityId(*first));
			if (ci != NULL)
				log.displayNL(" - '%s' %s:%x",
					ci->Name.toUtf8().c_str(),
					ci->EntityId.toString().c_str(),
					first->getIndex());
			else
				log.displayNL("   *unknow* %s:%x",
					eid.toString().c_str(),
					first->getIndex());
		}
	}
}

void CChatManager::displayChatGroups(NLMISC::CLog &log, bool displayUniverse, bool displayPlayerAudience)
{
	std::map< TGroupId, CChatGroup >::iterator first(_Groups.begin()), last(_Groups.end());

	for (; first != last; ++first)
	{
		CChatGroup &cg = first->second;

		if ((displayUniverse || cg.Type != CChatGroup::universe)
			&& (cg.Type != CChatGroup::say)
			&& (cg.Type != CChatGroup::shout)
			)
		{
			displayChatGroup(log, first->first, cg);
		}
	}

	if (displayPlayerAudience)
	{
		TClientInfoCont::iterator first(_Clients.begin()), last(_Clients.end());
		for (; first != last; ++first)
		{
			CChatClient *cc = first->second;
			if (cc->getId().getType() == RYZOMID::player)
			{
				log.displayNL ("Chat group for player %s :", cc->getId().toString().c_str());
				displayChatGroup(log, cc->getSayAudienceId(), cc->getSayAudience());
				displayChatGroup(log, cc->getShoutAudienceId(), cc->getShoutAudience());
			}
		}
	}
}

void CChatManager::displayChatAudience(NLMISC::CLog &log, const CEntityId &eid, bool updateAudience)
{
	CCharacterInfos *ci = IOS->getCharInfos(eid);
	if (ci == NULL)
	{
		log.displayNL("Unknown client id '%s'", eid.toString().c_str());
		return;
	}

	TClientInfoCont::iterator it(_Clients.find(ci->DataSetIndex));
	if (it != _Clients.end())
	{
		CChatClient &cc = *(it->second);

		if (updateAudience)
			cc.updateAudience();

		CChatGroup &cg = cc.getAudience();

		if (cg.Type == CChatGroup::say)
			log.displayNL("Client '%s' (%u) say audience:", eid.toString().c_str(), ci->DataSetIndex.getIndex());
		else if (cg.Type == CChatGroup::shout)
			log.displayNL("Client '%s' (%u) shout audience:", eid.toString().c_str(), ci->DataSetIndex.getIndex());
		else
		{
			log.displayNL("Client '%s' (%u) invalide local chat mode !", eid.toString().c_str(), ci->DataSetIndex.getIndex());
			return;
		}


		CChatGroup::TMemberCont::iterator first(cg.Members.begin()), last(cg.Members.end());
		for (; first != last; ++first)
		{
			CEntityId eid = TheDataset.getEntityId(*first);
			if (eid.getType() == RYZOMID::player)
			{
				CCharacterInfos *ci = IOS->getCharInfos(TheDataset.getEntityId(*first));
				if (ci != NULL)
					log.displayNL("  '%s' %s:%x",
						ci->Name.toUtf8().c_str(),
						TheDataset.getEntityId(*first).toString().c_str(),
						first->getIndex());

				else
					log.displayNL("   *unknow* %s:%x",
						TheDataset.getEntityId(*first).toString().c_str(),
						first->getIndex());
			}
		}
	}
	else
	{
		log.displayNL("The client '%s' is not in the chat manager client list", eid.toString().c_str());
	}
}

void CChatManager::sendHistoric(const TDataSetRow &receiver, TChanID chanID)
{
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan)
	{
		nlwarning("Unknown chan");
		return;
	}
	for(uint k = 0; k < chan->Historic.getSize(); ++k)
	{
//		sendChat(CChatGroup::dyn_chat, receiver, chan->Historic[k].String, chan->Historic[k].Sender, chanID);
		if (chan->Historic[k].Shared)
		{
			CSharedMessageScope scope(_SharedMessage, chan->Historic[k].Message);
			sendChat(CChatGroup::dyn_chat, receiver, chan->Historic[k].String, TDataSetRow(), chanID, chan->Historic[k].SenderString);
		}
		else
			sendChat(CChatGroup::dyn_chat, receiver, chan->Historic[k].String, TDataSetRow(), chanID, chan->Historic[k].SenderString);
	}
}



ucstring CChatManager::filterClientInputColorCode(ucstring &text)
{
	ucstring result;
	result.reserve(text.size());

	ucstring::size_type pos = 0;

	for (; pos < text.size(); ++pos)
	{
		if (text[pos] == '@' && pos < text.size()-1 && text[pos+1] == '{')
		{
			continue;
		}
		else
		{
			// authorized char
			result += text[pos];
		}
	}

	return result;
}

ucstring CChatManager::filterClientInput(ucstring &text, bool trimBeginning, bool trimEnd)
{
	ucstring result;
	result.reserve(text.size());
	// 1st, remove any beginning or ending white space
	ucstring::size_type pos = 0;

	// skip begin white spaces or : (used by deepl)
	while (trimBeginning && pos < text.size() && (text[pos] == ' ' || text[pos] == '\t'))
		++pos;

	// remove ending white space
	while (trimEnd && text.size() > 0 && (*(text.rbegin()) == ' ' || *(text.rbegin()) == '\t'))
		text.resize(text.size()-1);

	if (trimBeginning && pos+3 < text.size() && text[pos] == ':' && text[pos+3] == ':')
		++pos;

	if (trimBeginning && pos+1 < text.size() && text[pos] == '>' && text[pos+1] == ':')
		pos += 2;

	// copy string, removing multi white space between words
	// filter out color code
	bool lastIsWhite = false;
	for (; pos < text.size(); ++pos)
	{
		bool currentIsWhite = (text[pos] == ' ' || text[pos] == '\t');
		if (!(lastIsWhite && currentIsWhite))
		{
			// any double white skipped
			if (text[pos] == '&')
			{
				// Special case if there is <NEW> or <CHG> at the beginning
				bool hasBrackets = false;
				if (pos >= 5)
				{
					hasBrackets = (text[pos-1] == '>') &&
						(text[pos-5] == '<');
				}
				// Filter out '&' at the first non-whitespace position to remove
				// system color code (like '&SYS&' )
				bool disallowAmpersand = (trimBeginning && result.empty()) || hasBrackets;
				if (disallowAmpersand)
				{
					result += '.';
				}
				else
				{
					// authorized ampersand
					result += '&';
				}
			}
			else if (text[pos] == '@' && pos < text.size()-1 && text[pos+1] == '{')
			{
				// filter out any match of '@{' to remove color tag (like '@{rgba}')
				result += '.';
			}
			else
			{
				// authorized char
				result += text[pos];
			}
		}
		lastIsWhite = currentIsWhite;
	}

	return result;
}


/// Subscribe special ring users in the ring universe chat
void CChatManager::subscribeCharacterInRingUniverse(const NLMISC::CEntityId &charEId)
{
	// create a fake eid
	TDataSetRow dsr = TheDataset.getDataSetRow(charEId);
	BOMB_IF (!dsr.isValid(), "CChatManager::subscribeCharacterInRingUniverse : the char "<<charEId.toString()<<" is not in the mirror", return);

	// add player in the group universe
	TGroupId grpUniverse = CEntityId(RYZOMID::chatGroup,0);
	addToGroup(grpUniverse, dsr);
}

/// Unsubscribe special ring users in the ring universe chat
void CChatManager::unsubscribeCharacterInRingUniverse(const NLMISC::CEntityId &charEId)
{
	// create a fake eid
	TDataSetRow dsr = TheDataset.getDataSetRow(charEId);
	if (dsr.isValid())
	{
		// remove player of the group universe
		TGroupId grpUniverse = CEntityId(RYZOMID::chatGroup,0);
		removeFromGroup(grpUniverse, dsr);
	}
}

TChanID CChatManager::getChanId(const string name) {
	const TChanID *chanid = _ChanNames.getA(name);
	if (chanid)
		return *chanid;
	return DYN_CHAT_INVALID_CHAN;
}
