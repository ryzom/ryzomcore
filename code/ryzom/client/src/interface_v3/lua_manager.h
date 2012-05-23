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

#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

#include "nel/misc/smart_ptr.h"

namespace NLGUI
{
	class CLuaState;
}

/// Provides a single global access point to the Lua state, and related stuff. :(
class CLuaManager
{
public:
	~CLuaManager();

	static CLuaManager& getInstance()
	{
		if( instance == NULL )
		{
			instance = new CLuaManager();
		}
		return *instance;
	}

	/// Enables attaching the Lua debugger in the CLuaState instance, only matters on startup.
	static void enableLuaDebugging(){ debugLua = true;	}

	NLGUI::CLuaState* getLuaState() const{ return luaState; }

private:
	CLuaManager();

	static CLuaManager *instance;
	static bool debugLua;

	NLMISC::CSmartPtr< NLGUI::CLuaState > luaState;
};


#endif
