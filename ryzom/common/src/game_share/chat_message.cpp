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

#include "stdpch.h"
#include "chat_message.h"

namespace CHAT_MESSAGE
{
	bool isMentionProtectedText(const ucstring &text, uint32 start)
	{
		uint32 lineStart = 0;
		uint32 codeLength = 0;
		ucchar codeDelimiter = 0;
		for (uint32 pos = 0; pos < start; ++pos)
		{
			if (text[pos] == '\n')
			{
				lineStart = pos + 1;
				continue;
			}
			if (!codeLength && text[pos] == '\\' && pos + 1 < start && text[pos + 1] != '\n')
			{
				++pos;
				continue;
			}
			if (text[pos] != '`' && text[pos] != '~')
				continue;
			uint32 length = 1;
			while (pos + length < text.size() && text[pos + length] == text[pos])
				++length;
			const bool atLineStart = pos - lineStart <= 3 &&
				text.substr(lineStart, pos - lineStart).find_first_not_of(' ') == ucstring::npos;
			if (codeLength)
			{
				if (text[pos] == codeDelimiter && atLineStart && length >= codeLength)
				{
					uint32 tail = pos + length;
					while (tail < text.size() && (text[tail] == ' ' || text[tail] == '\t' || text[tail] == '\r'))
						++tail;
					if (tail == text.size() || text[tail] == '\n')
						codeLength = 0;
				}
			}
			else if (atLineStart && length >= 3)
			{
				codeDelimiter = text[pos];
				codeLength = length;
			}
			else if (text[pos] == '`')
			{
				ucstring delimiter;
				delimiter.assign(length, '`');
				ucstring::size_type close = text.find(delimiter, pos + length);
				while (close != ucstring::npos &&
					(text[close - 1] == '`' || (close + length < text.size() && text[close + length] == '`')))
					close = text.find(delimiter, close + length);
				if (close != ucstring::npos)
				{
					if (start < close)
						return true;
					const ucstring::size_type newline = text.rfind('\n', close);
					if (newline != ucstring::npos)
						lineStart = (uint32)newline + 1;
					pos = (uint32)close + length - 1;
					continue;
				}
			}
			pos += length - 1;
		}
		if (codeLength)
			return true;
		for (uint32 pos = 0; pos < start; ++pos)
		{
			if (text[pos] == '\\')
			{
				++pos;
				continue;
			}
			if (text[pos] != '[')
				continue;
			uint32 close = pos + 1;
			uint32 depth = 1;
			for (; close < text.size(); ++close)
			{
				if (text[close] == '\\')
					++close;
				else if (text[close] == '[')
					++depth;
				else if (text[close] == ']' && --depth == 0)
					break;
			}
			if (depth || close + 1 >= text.size() || (text[close + 1] != '(' && text[close + 1] != '['))
				continue;
			const ucchar open = text[close + 1];
			const ucchar end = open == '(' ? ')' : ']';
			uint32 target = close + 2;
			depth = 1;
			for (; target < text.size(); ++target)
			{
				if (text[target] == '\\')
					++target;
				else if (text[target] == open)
					++depth;
				else if (text[target] == end && --depth == 0)
					break;
			}
			if (!depth && start < target)
				return true;
		}
		uint32 word = start;
		while (word && text[word - 1] != ' ' && text[word - 1] != '\t' && text[word - 1] != '\n')
			--word;
		const std::string preceding = NLMISC::toLowerAscii(text.substr(word, start - word).toUtf8());
		if (preceding.find("http://") != std::string::npos || preceding.find("https://") != std::string::npos)
			return true;
		const ucstring::size_type command = text.rfind(ucstring("/$$"), start);
		if (command != ucstring::npos)
		{
			const ucstring::size_type close = text.find(ucstring("$$/"), command + 3);
			if (close != ucstring::npos && start < close)
				return true;
		}
		const ucstring::size_type label = text.rfind('(', start);
		if (label != ucstring::npos)
		{
			const ucstring::size_type close = text.find(ucstring(")["), label + 1);
			if (close != ucstring::npos && start < close)
			{
				const std::string target = NLMISC::toLowerAscii(text.substr(close + 2, 8).toUtf8());
				if (target.compare(0, 7, "http://") == 0 || target.compare(0, 8, "https://") == 0)
					return true;
			}
		}
		return false;
	}

	uint32 getMentionEnd(const ucstring &text, uint32 start)
	{
		if (start >= text.size() || text[start] != '@' ||
			(start && !isMentionBoundary(text[start - 1])) || isMentionProtectedText(text, start))
			return start;
		uint32 end = start + 1;
		while (end < text.size() && isMentionNameChar(text[end]))
			++end;
		if (end > start + 1 && text[end - 1] == '.')
			--end;
		const ucstring name = text.substr(start + 1, end - start - 1);
		if (!name.empty() && name[name.size() - 1] == ')' && name.find('(') == ucstring::npos)
			--end;
		return end > start + 1 ? end : start;
	}

	bool isValidTarget(CChatGroup::TGroupType group, const NLMISC::CEntityId &dynamicChannelId,
		const std::string &receiver)
	{
		if (group == CChatGroup::tell)
			return dynamicChannelId == NLMISC::CEntityId::Unknown &&
				!receiver.empty() && receiver.size() <= MaxReceiverLength;
		if (!receiver.empty())
			return false;
		if (group == CChatGroup::dyn_chat)
			return dynamicChannelId.getType() == RYZOMID::dynChatGroup;
		if (dynamicChannelId != NLMISC::CEntityId::Unknown)
			return false;
		return group == CChatGroup::say || group == CChatGroup::shout ||
			group == CChatGroup::team || group == CChatGroup::guild ||
			group == CChatGroup::region || group == CChatGroup::universe;
	}
}

bool CChatMessagePosition::isValid() const
{
	if (Kind != PlayerPosition && Kind != MapPosition && Kind != UserLandMark)
		return false;
	if (FlagName.size() > CHAT_MESSAGE::MaxTextLength ||
		(Kind == UserLandMark ? FlagName.empty() : !FlagName.empty()))
		return false;
	for (uint i = 0; i < FlagName.size(); ++i)
		if (FlagName[i] < 32 || FlagName[i] == 127)
			return false;
	return Place.size() <= CHAT_MESSAGE::MaxTextLength &&
		Region.size() <= CHAT_MESSAGE::MaxTextLength &&
		Continent.size() <= CHAT_MESSAGE::MaxTextLength &&
		SenderName.size() <= CHAT_MESSAGE::MaxReceiverLength;
}

bool CChatMessageMacro::isValid() const
{
	if (Name.empty() || Name.size() > CHAT_MESSAGE::MaxTextLength ||
		DispText.size() > CHAT_MESSAGE::MaxTextLength ||
		Commands.empty() || Commands.size() > CHAT_MESSAGE::MaxMacroCommands)
		return false;
	for (uint i = 0; i < Commands.size(); ++i)
	{
		if (Commands[i].Name.empty() || Commands[i].Name.size() > CHAT_MESSAGE::MaxTextLength ||
			Commands[i].Params.size() > CHAT_MESSAGE::MaxTextLength)
			return false;
	}
	return true;
}

bool CChatMessagePart::isValid() const
{
	switch (Type)
	{
	case Text:
		return TextValue.size() <= CHAT_MESSAGE::MaxSerializedSize / sizeof(ucchar);
	case Item:
		return ItemValue.SheetId != NLMISC::CSheetId::Unknown &&
			ItemValue.NamePhraseId.size() <= CHAT_MESSAGE::MaxTextLength &&
			ItemValue.Name.size() <= CHAT_MESSAGE::MaxTextLength &&
			ItemValue.CreatorName.size() <= CHAT_MESSAGE::MaxReceiverLength;
	case Phrase:
		return PhraseValue.SheetId != NLMISC::CSheetId::Unknown || !PhraseValue.Phrase.Bricks.empty();
	case Position:
		return PositionValue.isValid();
	case Macro:
		return MacroValue.isValid();
	}
	return false;
}

bool CChatMessageQuote::isValid() const
{
	if (MessageId.empty())
		return SenderId == NLMISC::CEntityId::Unknown && SenderName.empty() &&
			Timestamp == 0 && Parts.empty();
	if (MessageId.size() != CHAT_MESSAGE::MessageIdLength ||
		SenderName.size() > CHAT_MESSAGE::MaxReceiverLength ||
		Parts.size() > CHAT_MESSAGE::MaxParts)
		return false;
	bool visible = false;
	for (uint i = 0; i < Parts.size(); ++i)
	{
		if (!Parts[i].isValid())
			return false;
		visible |= Parts[i].Type != CChatMessagePart::Text || !Parts[i].TextValue.empty();
	}
	return Parts.empty() || visible;
}

bool CChatMessage::isValid() const
{
	if (Parts.empty() || Parts.size() > CHAT_MESSAGE::MaxParts ||
		TranslatedParts.size() > CHAT_MESSAGE::MaxParts ||
		(!TranslatedParts.empty() && TranslatedParts.size() != Parts.size()))
		return false;
	bool visible = false;
	uint32 referenceCount = 0;
	for (uint i = 0; i < Parts.size(); ++i)
	{
		const CChatMessagePart &part = Parts[i];
		if (!part.isValid())
			return false;
		if (part.Type == CChatMessagePart::Text)
		{
			visible |= !part.TextValue.empty();
			continue;
		}
		if (part.Type == CChatMessagePart::Position &&
			(part.PositionValue.Place.empty() || part.PositionValue.Continent.empty()))
			return false;
		if (++referenceCount > CHAT_MESSAGE::MaxReferences)
			return false;
		visible = true;
	}
	if (!visible)
		return false;
	for (uint i = 0; i < TranslatedParts.size(); ++i)
	{
		if (TranslatedParts[i].Type != Parts[i].Type || !TranslatedParts[i].isValid())
			return false;
	}
	if (Mentions.size() > CHAT_MESSAGE::MaxMentions)
		return false;
	uint32 previousEnd = 0;
	uint8 previousPart = 0;
	for (uint i = 0; i < Mentions.size(); ++i)
	{
		const CChatMessageMention &mention = Mentions[i];
		if (mention.Part >= Parts.size() || Parts[mention.Part].Type != CChatMessagePart::Text)
			return false;
		const ucstring &text = Parts[mention.Part].TextValue;
		const bool player = mention.Scope == CChatMessageMention::Player;
		if (mention.Scope > CChatMessageMention::All ||
			(player && (mention.PlayerId.getType() != RYZOMID::player || mention.Name.empty() ||
				mention.Name.size() > CHAT_MESSAGE::MaxReceiverLength)) ||
			(!player && (mention.PlayerId != NLMISC::CEntityId::Unknown || !mention.Name.empty())) ||
			mention.Length < 2 || mention.Start >= text.size() ||
			mention.Length > text.size() - mention.Start ||
			text[mention.Start] != '@' ||
			(i && (mention.Part < previousPart ||
				(mention.Part == previousPart && mention.Start < previousEnd))))
			return false;
		previousPart = mention.Part;
		previousEnd = mention.Start + mention.Length;
	}
	return Quote.isValid() &&
		(MessageId.empty() || MessageId.size() == CHAT_MESSAGE::MessageIdLength) &&
		SenderName.size() <= CHAT_MESSAGE::MaxReceiverLength &&
		(SourceLanguage.empty() || SourceLanguage.size() == 2) &&
		(TranslationLanguage.empty() || TranslationLanguage.size() == 2);
}

bool CChatMessageRequest::isValid() const
{
	if (Text.empty() || Text.size() > CHAT_MESSAGE::MaxTextLength ||
		References.size() > CHAT_MESSAGE::MaxReferences ||
		!(QuoteMessageId.empty() ||
			(QuoteMessageId.size() == CHAT_MESSAGE::MessageIdLength && ClientRequestId != 0)))
		return false;
	uint32 textPosition = 0;
	for (std::vector<CChatMessageReference>::const_iterator it = References.begin();
		it != References.end(); ++it)
	{
		if (it->Length == 0 || it->Start < textPosition || it->Start > Text.size() ||
			it->Length > Text.size() - it->Start || it->Type > CChatMessageReference::Macro ||
			(it->Type == CChatMessageReference::Mention &&
				(it->Value == CChatMessageMention::Player || it->Value > CChatMessageMention::All)) ||
			(it->Type == CChatMessageReference::MapPosition &&
				(!it->PositionValue.isValid() || it->PositionValue.Kind == CChatMessagePosition::PlayerPosition)) ||
			(it->Type == CChatMessageReference::Macro && !it->MacroValue.isValid()))
			return false;
		textPosition = it->Start + it->Length;
	}
	return true;
}
