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




#ifndef RYAI_STATE_CHAT_H
#define RYAI_STATE_CHAT_H

#include "child_container.h"
#include "ai_keywords.h"
#include "npc_description_msg.h"

class CAIState;
class CBotNpc;

class CAIStateChat
	:public	CAliasChild<CAIState>
	,public	NLMISC::CRefCount
{
public:
	// ctor & dtor ------------------------------------------------------
	CAIStateChat(CAIState*	state,	CAIAliasDescriptionNode *aliasDescription): CAliasChild<CAIState>(state,aliasDescription)
	{}
	virtual ~CAIStateChat()
	{}

	virtual	std::string	getIndexString	()	const;

	// Keyword management ----------------------------------------------
	void	botKeywordFilterClear()							{ _botKeywordFilter.clear(); }
	void	botKeywordFilterAdd(const std::string &keyword);

	bool	botKeywordTest(CKeywordMask mask)			{ return _botKeywordFilter.test(mask); }

	// name management -------------------------------------------------
	void	botNameFilterClear()						{ _namedBots.clear(); }
	void	botNameFilterAdd(const std::string &name)	{ _namedBots.push_back(name); }						

	bool	botNameTest(const std::string &name) const
	{
		std::vector<std::string>::const_iterator it(std::find(_namedBots.begin(), _namedBots.end(), name));
		return it != _namedBots.end();
	}

	// compatibility test ----------------------------------------------
	bool	testCompatibility(const CBotNpc &bot) const;
	
	//	Chat management
	const	CNpcChatProfileImp	&getChat()	const
	{
		return	_chatProfile;
	}
	CNpcChatProfileImp	&chatProfile	()
	{
		return	_chatProfile;
	}
	
private:
	// protected data ---------------------------------------------------
	CKeywordFilter				_botKeywordFilter;		// keyword filter for identifying bot types to whom to apply profile
	std::vector<std::string>	_namedBots;	// list of named bots to whom this profile applies

	CNpcChatProfileImp			_chatProfile;			// the chat profile buffer
};

#endif
