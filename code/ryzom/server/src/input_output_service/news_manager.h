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



#ifndef NEWS_MANAGER_H
#define NEWS_MANAGER_H


// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"

// game share
#include "game_share/ryzom_entity_id.h"
//#include "game_share/chat_static_database.h"
//#include "game_share/chat_dynamic_database.h"
#include "game_share/news_types.h"

// std
#include <map>
#include <string>


/**
 * CNewsManager
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2002
 */
class CNewsManager 
{
public :

	/**
	 * Init the manager.
	 */
	static void init ();

	/// return a news of a given type
	static void getNews (NEWSTYPE::TNewsType type, NLMISC::CBitMemStream &bms);
};


#endif // NEWS_MANAGER_H

/* End of news_manager.h */
