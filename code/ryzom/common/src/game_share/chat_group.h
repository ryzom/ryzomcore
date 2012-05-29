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



#ifndef CHAT_GROUP_H
#define CHAT_GROUP_H

// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"

// game share
#include "ryzom_entity_id.h"
#include "base_types.h"

// std
#include <set>


// group id
//typedef uint32 TGroupId;
typedef NLMISC::CEntityId TGroupId;






/**
 * CChatGroup
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct CChatGroup
{
	enum { MaxDynChanPerPlayer = 8 };
	// group type
	enum TGroupType
	{
		say = 0,
		shout,
		team,
		guild,
		civilization,
		territory,
		universe,
		tell,
		player,
		arround,
		system,
		region,
		dyn_chat,
		nbChatMode,
		// Following mode are client side only. Thus, after 'nbChatMode'
		yubo_chat	// (special telnet chat for Game Masters, same channel as the Yubo Klient)
	};

	/// group type
	TGroupType Type;
	/// Group name (for player chat channel)
//	std::string GroupName;
	NLMISC::TStringId	GroupName;

	typedef std::set<TDataSetRow>	TMemberCont;
	/// group members
//	std::set<NLMISC::CEntityId> Members;
	TMemberCont Members;

	/**
	 * Default constructor
	 */
	CChatGroup()
		: Type(nbChatMode)
	{}

	/**
	 * Constructor
	 */
//	CChatGroup( TGroupType type, const std::string &groupName )
	CChatGroup( TGroupType type, NLMISC::TStringId groupName)
		: Type(type),
		GroupName(groupName)
	{ }

	/// convert a string to a group type
	static TGroupType stringToGroupType( const std::string & str );
	/// Convert a chat group to string
	static const std::string &groupTypeToString(CChatGroup::TGroupType type);

};


#endif // CHAT_GROUP_H

/* End of chat_group.h */
