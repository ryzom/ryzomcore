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

#ifndef RY_POSITIONORENTITYTYPEHELPER_H
#define RY_POSITIONORENTITYTYPEHELPER_H

//#include "pd_support_service\stat_character.h"
#include "mission_manager\ai_alias_translator.h"
#include "nel\misc\entity_id.h"
#include "game_share\position_or_entity_type.h"
#include "mission_manager\mission_parser.h"
#include "nel\misc\string_mapper.h"


//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Class that can contain either an entity id or a position             */
/************************************************************************/ 
class CPositionOrEntityHelper
{
public:
	
	/************************************************************************/
	/* Creates a PositionOrEntity instance from a string                    */
	/************************************************************************/
	static TPositionOrEntity fromString(const std::string& s);

	static const TPositionOrEntity Invalid;

};


#endif /* RY_POSITIONORENTITYTYPEHELPER_H */
