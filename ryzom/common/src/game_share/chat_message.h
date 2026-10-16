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

#ifndef RY_CHAT_MESSAGE_H
#define RY_CHAT_MESSAGE_H

#include "chat_group.h"
#include "item_infos.h"
#include "sphrase_com.h"
#include "nel/misc/rgba.h"
#include "nel/misc/stream.h"
#include <string>
#include <vector>

namespace CHAT_MESSAGE
{
	enum
	{
		MaxTextLength = 255,
		MaxReceiverLength = 255,
		MaxReferences = 8, // Bounds attachment fan-out and item-info payload size.
		MaxParts = MaxReferences * 2 + 1,
		MaxMentions = MaxTextLength / 2,
		MaxMacroCommands = 32,
		MaxEmojiNameLength = 64, // Longest name the client emoji table accepts.
		MessageIdLength = 36,
		MaxSerializedSize = 64 * 1024 // Bounds authoritative data before server fan-out.
	};

	inline bool stripNoBubble(ucstring &text)
	{
		const ucstring tag("{no_bubble}");
		bool removed = false;
		ucstring::size_type position = text.find(tag);
		while (position != ucstring::npos)
		{
			text.erase(position, tag.size());
			removed = true;
			position = text.find(tag, position);
		}
		return removed;
	}

	inline bool isMentionNameChar(uint32 c)
	{
		return c > 127 || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == '(' || c == ')';
	}

	inline bool isMentionBoundary(uint32 c)
	{
		return c == ' ' || c == '\t' || c == '\n' || c == '"' || c == '\'' ||
			c == '[' || c == '(' || c == ',' || c == ':' || c == ';' || c == '!' || c == '?';
	}

	// Ignore code spans, links, URLs and /$$...$$/ tokens; their @ characters
	// are not player mentions.
	bool isMentionProtectedText(const ucstring &text, uint32 start);

	uint32 getMentionEnd(const ucstring &text, uint32 start);

	bool isValidTarget(CChatGroup::TGroupType group, const NLMISC::CEntityId &dynamicChannelId,
		const std::string &receiver);

	template <class T>
	void serialBounded(NLMISC::IStream &stream, std::vector<T> &values, uint max)
	{
		nlassert(stream.isReading() || values.size() <= max);
		uint8 count = stream.isReading() ? 0 : (uint8)values.size();
		stream.serial(count);
		if (count > max)
			throw NLMISC::EInvalidDataStream(stream);
		if (stream.isReading())
			values.resize(count);
		for (uint i = 0; i < values.size(); ++i)
			stream.serial(values[i]);
	}
}

class CChatMessagePosition
{
public:
	enum TKind
	{
		PlayerPosition,
		MapPosition,
		UserLandMark
	};

	CChatMessagePosition() : Kind(PlayerPosition), X(0), Y(0), FlagColor(NLMISC::CRGBA::White),
		Timestamp(0) {}

	void serial(NLMISC::IStream &stream)
	{
		stream.serialEnum(Kind);
		stream.serial(X);
		stream.serial(Y);
		stream.serial(FlagName);
		stream.serial(FlagColor);
		stream.serial(Place);
		stream.serial(Region);
		stream.serial(Continent);
		stream.serial(SenderName);
		stream.serial(Timestamp);
	}

	bool isValid() const;

	// Mirror and map coordinates use millimetres.
	TKind Kind;
	sint32 X;
	sint32 Y;
	ucstring FlagName;
	NLMISC::CRGBA FlagColor;
	std::string Place;
	std::string Region;
	std::string Continent;
	ucstring SenderName;
	uint32 Timestamp;
};

class CChatMessageItem
{
public:
	CChatMessageItem()
	: Quality(0), Quantity(0), Weight(0), UserColor(0), NameId(0), Enchant(0),
	  RMClassType(0), RMFaberStatType(0)
	{
	}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(SheetId);
		stream.serial(Quality);
		stream.serial(Quantity);
		stream.serial(Weight);
		stream.serial(UserColor);
		stream.serial(NameId);
		stream.serial(NamePhraseId);
		stream.serial(Name);
		stream.serial(CreatorName);
		stream.serial(Enchant);
		stream.serial(RMClassType);
		stream.serial(RMFaberStatType);
		stream.serial(Info);
	}

	NLMISC::CSheetId SheetId;
	uint32 Quality;
	uint32 Quantity;
	uint32 Weight;
	sint32 UserColor;
	// Receiver string id; the IOS resolves it per receiver before sending.
	uint32 NameId;
	std::string NamePhraseId;
	ucstring Name;
	ucstring CreatorName;
	uint32 Enchant;
	sint32 RMClassType;
	sint32 RMFaberStatType;
	CItemInfos Info;
};

class CChatMessagePhrase
{
public:
	void serial(NLMISC::IStream &stream)
	{
		stream.serial(SheetId);
		stream.serial(Phrase);
	}

	// Known rolemaster phrases only carry their sheet; custom phrases carry the bricks.
	NLMISC::CSheetId SheetId;
	CSPhraseCom Phrase;
};

class CChatMessageMacro
{
public:
	class CCommand
	{
	public:
		void serial(NLMISC::IStream &stream)
		{
			stream.serial(Name);
			stream.serial(Params);
		}

		std::string Name;
		std::string Params;
	};

	CChatMessageMacro() : BitmapBack(0), BitmapIcon(0), BitmapOver(0) {}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(Name);
		stream.serial(DispText);
		stream.serial(BitmapBack);
		stream.serial(BitmapIcon);
		stream.serial(BitmapOver);
		CHAT_MESSAGE::serialBounded(stream, Commands, CHAT_MESSAGE::MaxMacroCommands);
	}

	bool isValid() const;

	// Same fields as the client macro; the receiver reviews them in the macro editor.
	std::string Name;
	std::string DispText;
	uint8 BitmapBack;
	uint8 BitmapIcon;
	uint8 BitmapOver;
	std::vector<CCommand> Commands;
};

class CChatMessageReaction
{
public:
	CChatMessageReaction() : Remove(false) {}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(MessageId);
		stream.serial(Emoji);
		stream.serial(Remove);
	}

	bool isValid() const;

	// Reacted message and the emoji name written between colons.
	std::string MessageId;
	std::string Emoji;
	bool Remove;
};

class CChatMessagePart
{
public:
	enum TType
	{
		Text,
		Item,
		Phrase,
		Position,
		Macro,
		Reaction
	};

	CChatMessagePart() : Type(Text) {}

	void serial(NLMISC::IStream &stream)
	{
		stream.serialEnum(Type);
		switch (Type)
		{
		case Text:
			stream.serial(TextValue);
			break;
		case Item:
			stream.serial(ItemValue);
			break;
		case Phrase:
			stream.serial(PhraseValue);
			break;
		case Position:
			stream.serial(PositionValue);
			break;
		case Macro:
			stream.serial(MacroValue);
			break;
		case Reaction:
			stream.serial(ReactionValue);
			break;
		default:
			throw NLMISC::EInvalidDataStream(stream);
		}
	}

	bool isValid() const;

	TType Type;
	ucstring TextValue;
	CChatMessageItem ItemValue;
	CChatMessagePhrase PhraseValue;
	CChatMessagePosition PositionValue;
	CChatMessageMacro MacroValue;
	CChatMessageReaction ReactionValue;
};

class CChatMessageQuote
{
public:
	CChatMessageQuote() : SenderId(NLMISC::CEntityId::Unknown), Timestamp(0) {}

	bool isValid() const;

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(MessageId);
		stream.serial(SenderId);
		stream.serial(SenderName);
		stream.serial(Timestamp);
		CHAT_MESSAGE::serialBounded(stream, Parts, CHAT_MESSAGE::MaxParts);
	}

	std::string MessageId;
	NLMISC::CEntityId SenderId;
	ucstring SenderName;
	uint32 Timestamp;
	std::vector<CChatMessagePart> Parts;
};

class CChatMessageMention
{
public:
	enum TScope
	{
		Player,
		Team,
		Guild,
		All
	};

	CChatMessageMention() : Scope(Player), Part(0), Start(0), Length(0) {}
	void serial(NLMISC::IStream &stream)
	{
		stream.serialEnum(Scope);
		stream.serial(PlayerId);
		stream.serial(Name);
		stream.serial(Part);
		stream.serial(Start);
		stream.serial(Length);
	}
	TScope Scope;
	// Mentioned player, only for the Player scope.
	NLMISC::CEntityId PlayerId;
	ucstring Name;
	uint8 Part;
	uint16 Start;
	uint16 Length;
};

class CChatMessage
{
public:
	CChatMessage() : NoBubble(false), Timestamp(0), AllowTranslation(false),
		MentionHomeSessionId(0), ResolveMentions(false) {}

	bool isValid() const;

	void serial(NLMISC::IStream &stream)
	{
		uint8 version = 1;
		stream.serial(version);
		if (version != 1)
			throw NLMISC::EInvalidDataStream(stream);
		stream.serial(NoBubble);
		stream.serial(MessageId);
		stream.serial(SenderId);
		stream.serial(SenderName);
		stream.serial(Timestamp);
		CHAT_MESSAGE::serialBounded(stream, Parts, CHAT_MESSAGE::MaxParts);
		stream.serial(SourceLanguage);
		stream.serial(TranslationLanguage);
		stream.serial(AllowTranslation);
		CHAT_MESSAGE::serialBounded(stream, TranslatedParts, CHAT_MESSAGE::MaxParts);
		stream.serial(Quote);
		stream.serial(MentionHomeSessionId);
		stream.serial(ResolveMentions);
		CHAT_MESSAGE::serialBounded(stream, Mentions, CHAT_MESSAGE::MaxMentions);
	}

	bool NoBubble;
	std::string MessageId;
	NLMISC::CEntityId SenderId;
	ucstring SenderName;
	uint32 Timestamp;
	std::vector<CChatMessagePart> Parts;
	std::string SourceLanguage;
	std::string TranslationLanguage;
	bool AllowTranslation;
	std::vector<CChatMessagePart> TranslatedParts;
	CChatMessageQuote Quote;
	// Home shard used to resolve unqualified player names.
	uint32 MentionHomeSessionId;
	// Allow the receiving IOS to resolve @ names in Parts.
	bool ResolveMentions;
	std::vector<CChatMessageMention> Mentions;
};

namespace CHAT_MESSAGE
{
	// A reaction is a message of its own, made of a single reaction part.
	inline bool isReaction(const CChatMessage &message)
	{
		return message.Parts.size() == 1 && message.Parts[0].Type == CChatMessagePart::Reaction;
	}

	inline void stripNoBubble(CChatMessage &message)
	{
		std::vector<CChatMessagePart> *partLists[] = { &message.Parts, &message.TranslatedParts };
		for (uint list = 0; list < 2; ++list)
		{
			for (std::vector<CChatMessagePart>::iterator part = partLists[list]->begin(); part != partLists[list]->end(); ++part)
			{
				if (part->Type == CChatMessagePart::Text && stripNoBubble(part->TextValue))
					message.NoBubble = true;
			}
		}
	}
}

class CChatMessageReference
{
public:
	enum TType
	{
		Item,
		KnownPhrase,
		PhraseSheet,
		Position,
		MapPosition,
		Mention,
		Macro,
		Reaction
	};

	CChatMessageReference() : Start(0), Length(0), Type(Item), Value(0) {}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(Start);
		stream.serial(Length);
		stream.serialEnum(Type);
		switch (Type)
		{
		case Item:
		case KnownPhrase:
		case PhraseSheet:
		case Mention:
			stream.serial(Value);
			break;
		case MapPosition:
			stream.serial(PositionValue);
			break;
		case Macro:
			stream.serial(MacroValue);
			break;
		case Reaction:
			stream.serial(ReactionValue);
			break;
		case Position:
			break;
		default:
			throw NLMISC::EInvalidDataStream(stream);
		}
	}

	uint16 Start;
	uint16 Length;
	TType Type;
	// Inventory slot, known phrase index, phrase sheet id or group mention scope.
	uint32 Value;
	CChatMessagePosition PositionValue;
	CChatMessageMacro MacroValue;
	CChatMessageReaction ReactionValue;
};

class CChatMessageRequest
{
public:
	CChatMessageRequest() : ClientRequestId(0) {}

	bool isValid() const;

	void serial(NLMISC::IStream &stream)
	{
		uint8 version = 1;
		stream.serial(version);
		if (version != 1)
			throw NLMISC::EInvalidDataStream(stream);
		stream.serial(Text);
		stream.serial(QuoteMessageId);
		stream.serial(ClientRequestId);
		CHAT_MESSAGE::serialBounded(stream, References, CHAT_MESSAGE::MaxReferences);
	}

	ucstring Text;
	std::string QuoteMessageId;
	uint32 ClientRequestId;
	std::vector<CChatMessageReference> References;
};

// Appended to the existing chat impulse; older clients ignore the remaining bits.
class CChatMessageTrailer
{
public:
	CChatMessageTrailer() : OwnTell(false) {}

	void serial(NLMISC::IStream &stream)
	{
		uint8 version = 1;
		stream.serial(version);
		if (version != 1)
			throw NLMISC::EInvalidDataStream(stream);
		stream.serial(OwnTell);
		stream.serial(TellTarget);
		stream.serial(Message);
	}

	bool OwnTell;
	ucstring TellTarget;
	CChatMessage Message;
};

#endif
