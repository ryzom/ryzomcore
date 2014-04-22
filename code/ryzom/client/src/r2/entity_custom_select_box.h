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

#ifndef R2_ENTITY_CUSTOM_SELECT_BOX_H
#define R2_ENTITY_CUSTOM_SELECT_BOX_H

#include "nel/misc/aabbox.h"
#include "nel/gui/lua_object.h"
#include <map>

namespace NLGUI
{
	class CLuaObject;
}

using namespace NLGUI;

namespace R2
{

class CEntityCustomSelectBox
{
public:
	bool			Enabled;
	NLMISC::CAABBox Box;
public:
	CEntityCustomSelectBox() : Enabled(false) {}
	void toTable(CLuaObject &table);
	void fromTable(CLuaObject &table);
};

typedef std::map<std::string, CEntityCustomSelectBox> TEntityCustomSelectBoxMap;

}

#endif
