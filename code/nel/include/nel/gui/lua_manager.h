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

	/**
	 Lua Manager

	 Provides a single global access point to the Lua state, and related stuff. :(
	 */
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

		/// Returns the Lua state.
		NLGUI::CLuaState* getLuaState() const{ return luaState; }

		/**
		 Executes a Lua script
		 @param luaScript   -  the script we want to execute ( the actual script, not the filename! )
		 @param smallScript -  true if the script is very small, so it can be cached for the possible next execution.
		 */
		bool executeLuaScript( const std::string &luaScript, bool smallScript = false );

		/// Resets the Lua state, that is deallocates it and allocates a new one.
		void ResetLuaState();

		/// Forces the Garbage Collector to run.
		void forceGarbageCollect();

		static void setEditorMode( bool b ){ editorMode = b; }

	private:
		CLuaManager();

		static CLuaManager *instance;
		static bool debugLua;
		static bool editorMode;

		NLGUI::CLuaState *luaState;
	};

}

#endif
