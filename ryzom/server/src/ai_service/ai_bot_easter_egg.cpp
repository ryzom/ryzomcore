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
#include "ai_bot_easter_egg.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// CBotEasterEgg                                                            //
//////////////////////////////////////////////////////////////////////////////

CBotEasterEgg::CBotEasterEgg(CGroup* owner, uint32 alias, std::string const& name, uint32 easterEggId)
: CBotNpc(owner, alias, name)
, _EasterEggId(easterEggId)
{
}

CBotEasterEgg::~CBotEasterEgg()
{
}

std::string	CBotEasterEgg::getOneLineInfoString() const
{
	string desc = NLMISC::toString("Easter Egg bot %u '%s'", getEasterEggId(), getName().c_str());
	return desc;
}
