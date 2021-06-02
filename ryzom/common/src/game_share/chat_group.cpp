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
#include "chat_group.h"
#include "nel/misc/string_conversion.h"



static const NLMISC::CStringConversion<CChatGroup::TGroupType>::CPair CChatGroupStringTable [] =
{
	{ "say",		CChatGroup::say },
	{ "shout",		CChatGroup::shout },
	{ "team",		CChatGroup::team },
	{ "guild",		CChatGroup::guild },
	{ "civilization", CChatGroup::civilization },
	{ "territory",	CChatGroup::territory },
	{ "universe",	CChatGroup::universe },
	{ "tell",		CChatGroup::tell },
	{ "player",		CChatGroup::player },
	{ "arround",	CChatGroup::arround },
	{ "system",		CChatGroup::system },
	{ "region",		CChatGroup::region },
};
static const NLMISC::CStringConversion<CChatGroup::TGroupType> CChatGroupConversion(CChatGroupStringTable, sizeof(CChatGroupStringTable) / sizeof(CChatGroupStringTable[0]),  CChatGroup::nbChatMode);

CChatGroup::TGroupType CChatGroup::stringToGroupType( const std::string & str )
{
	return CChatGroupConversion.fromString( str );
}

const std::string &CChatGroup::groupTypeToString(CChatGroup::TGroupType type)
{
	return CChatGroupConversion.toString(type);
}

