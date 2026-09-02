// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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

#ifndef CL_CHAT_LINK_CODEC_H
#define CL_CHAT_LINK_CODEC_H

#include "game_share/item_infos.h"
#include "game_share/sphrase_com.h"

#include <string>

namespace CHAT_LINK
{
	extern const char ItemPrefix[];
	extern const char PhrasePrefix[];
	extern const uint32 MaxMessageCharacters;

	struct CItemSnapshot
	{
		uint32 SheetId;
		uint32 Quality;
		uint32 Quantity;
		uint32 Weight;
		sint32 UserColor;
		uint32 Enchant;
		sint32 RMClassType;
		sint32 RMFaberStatType;
		std::string Name;
		CItemInfos Info;

		CItemSnapshot();
	};

	bool encodeItemSnapshot(CItemSnapshot snapshot, std::string &url);
	bool decodeItemSnapshot(const std::string &url, CItemSnapshot &snapshot);
	bool encodePhrase(CSPhraseCom phrase, std::string &url);
	bool decodePhrase(const std::string &url, CSPhraseCom &phrase);

	std::string makeVisibleLink(std::string title, const std::string &url);
	bool isItemUrl(const std::string &url);
	bool isPhraseUrl(const std::string &url);
}

#endif
