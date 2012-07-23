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




#ifndef RYAI_BOT_CHAT_INTERFACE_H
#define RYAI_AIDS_INTERFACE_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "game_share/news_types.h"
#include "game_share/bot_chat_types.h"

// the class
class CAIDSInterface
{
public:
	// classic init() and release()
	static void init();
	static void release();

	// send text messages back to the AIDS
	static void info(const std::string &s);
	static void debug(const std::string &s);
	static void warning(const std::string &s);
};

#endif
