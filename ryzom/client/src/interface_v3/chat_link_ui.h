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

#ifndef CL_CHAT_LINK_UI_H
#define CL_CHAT_LINK_UI_H

#include "chat_link_codec.h"

#include "nel/misc/types_nl.h"

#include <string>

class CDBCtrlSheet;

namespace CHAT_LINK
{
	enum TInsertResult
	{
		InsertOk,
		InsertNoChat,
		InsertInputFull
	};

	// Insert in the current or last chat input, then fall back to main chat.
	// The message limit is counted in Unicode characters.
	TInsertResult insertIntoChat(const std::string &marker);

	bool captureItemSnapshot(CDBCtrlSheet *item, uint32 slotId, CItemSnapshot &snapshot);
	std::string createItemMarker(CItemSnapshot snapshot, bool *customTextOmitted=NULL);
	std::string createPhraseMarker(const CSPhraseCom &phrase);
	void releasePreviewSheets();
}

#endif
