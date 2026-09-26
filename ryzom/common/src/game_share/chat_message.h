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

namespace CHAT_MESSAGE
{
	enum
	{
		MaxTextLength = 255,
		MaxReceiverLength = 255,
		MaxReferences = 8, // Bounds attachment fan-out and item-info payload size.
		MaxParts = MaxReferences * 2 + 1,
		MaxSerializedSize = 64 * 1024 // Bounds authoritative data before server fan-out.
	};

	inline bool isValidTarget(CChatGroup::TGroupType group, const NLMISC::CEntityId &dynamicChannelId,
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

	NLMISC::CSheetId SheetId;
	CSPhraseCom Phrase;
};

class CChatMessagePart
{
public:
	enum TType
	{
		Text,
		Item,
		Phrase
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
		}
	}

	TType Type;
	ucstring TextValue;
	CChatMessageItem ItemValue;
	CChatMessagePhrase PhraseValue;
};

class CChatMessage
{
public:
	CChatMessage() : NoBubble(false) {}

	bool isValid() const
	{
		if (Parts.empty() || Parts.size() > CHAT_MESSAGE::MaxParts)
			return false;

		uint32 textLength = 0;
		uint32 referenceCount = 0;
		for (std::vector<CChatMessagePart>::const_iterator it = Parts.begin(); it != Parts.end(); ++it)
		{
			if (it->Type == CChatMessagePart::Text)
			{
				if (it->TextValue.size() > CHAT_MESSAGE::MaxTextLength - textLength)
					return false;
				textLength += (uint32)it->TextValue.size();
			}
			else
			{
				if (it->Type != CChatMessagePart::Item && it->Type != CChatMessagePart::Phrase)
					return false;
				if (++referenceCount > CHAT_MESSAGE::MaxReferences)
					return false;
			}
		}
		return true;
	}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(NoBubble);
		nlassert(stream.isReading() || Parts.size() <= CHAT_MESSAGE::MaxParts);
		uint8 count = stream.isReading() ? 0 : (uint8)Parts.size();
		stream.serial(count);
		if (stream.isReading())
		{
			if (count > CHAT_MESSAGE::MaxParts)
				throw NLMISC::EInvalidDataStream(stream);
			Parts.resize(count);
		}
		for (uint i = 0; i < Parts.size(); ++i)
			stream.serial(Parts[i]);
	}

	bool NoBubble;
	std::vector<CChatMessagePart> Parts;
};

class CChatMessageReference
{
public:
	enum TType
	{
		Item,
		KnownPhrase,
		PhraseSheet
	};

	CChatMessageReference() : Start(0), Length(0), Type(Item), Value(0) {}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(Start);
		stream.serial(Length);
		stream.serialEnum(Type);
		stream.serial(Value);
	}

	uint16 Start;
	uint16 Length;
	TType Type;
	uint32 Value;
};

class CChatMessageRequest
{
public:
	bool isValid() const
	{
		if (Text.size() > CHAT_MESSAGE::MaxTextLength || References.empty() ||
			References.size() > CHAT_MESSAGE::MaxReferences)
			return false;

		uint32 textPosition = 0;
		for (std::vector<CChatMessageReference>::const_iterator it = References.begin();
			it != References.end(); ++it)
		{
			if (it->Length == 0 || it->Start < textPosition || it->Start > Text.size() ||
				it->Length > Text.size() - it->Start ||
				(it->Type != CChatMessageReference::Item &&
				 it->Type != CChatMessageReference::KnownPhrase &&
				 it->Type != CChatMessageReference::PhraseSheet))
				return false;
			textPosition = it->Start + it->Length;
		}
		return true;
	}

	void serial(NLMISC::IStream &stream)
	{
		stream.serial(Text);
		nlassert(stream.isReading() || References.size() <= CHAT_MESSAGE::MaxReferences);
		uint8 count = stream.isReading() ? 0 : (uint8)References.size();
		stream.serial(count);
		if (stream.isReading())
		{
			if (count > CHAT_MESSAGE::MaxReferences)
				throw NLMISC::EInvalidDataStream(stream);
			References.resize(count);
		}
		for (uint i = 0; i < References.size(); ++i)
			stream.serial(References[i]);
	}

	ucstring Text;
	std::vector<CChatMessageReference> References;
};

#endif
