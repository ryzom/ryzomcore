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

#ifndef RYAI_BOT_EASTER_EGG_H
#define RYAI_BOT_EASTER_EGG_H

#include "ai_bot_npc.h"

//////////////////////////////////////////////////////////////////////////////
// CBotEasterEgg                                                            //
//////////////////////////////////////////////////////////////////////////////

class CBotEasterEgg : public CBotNpc
{
public:
	CBotEasterEgg(CGroup* owner, uint32 alias, std::string const& name, uint32 easterEggId);
	virtual ~CBotEasterEgg();

	/// get the easter egg ID
	uint32	getEasterEggId() const { return _EasterEggId; }

	virtual std::string	getOneLineInfoString() const;

private:
	uint32	_EasterEggId;
};

#endif
