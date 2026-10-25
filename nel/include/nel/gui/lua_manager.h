// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

		/// Get or create singleton
		static CLuaManager& getInstance()
		{
			if( instance == nullptr)
			{
				instance = new CLuaManager();
			}
			return *instance;
		}

		/// Release singleton
		static void releaseInstance();

		/// Enables attaching the Lua debugger in the CLuaState instance, only matters on startup.
		static void enableLuaDebugging(){ debugLua = true;	}
		/// the next Lua state gets the full standard libraries (io, package, os, debug); development builds only
		static void enableDevelopmentLibraries(){ developmentLibraries = true; }

		/** Reporter of script errors.
		  * The library only has the log. The application installs a reporter that can show the error to the UI
		  * script author (system info window, goto-line button, the script lines around the error); without one
		  * the error is written to the log with nlwarning.
		  */
		class IScriptErrorReporter
		{
		public:
			virtual ~IScriptErrorReporter() {}
			/// error is the Lua error text; luaScript is the script that failed when known, empty for a file that failed to load
			virtual void onScriptError(const std::string &error, const std::string &luaScript) = 0;
		};
		static void setScriptErrorReporter(IScriptErrorReporter *reporter);
		static IScriptErrorReporter *getScriptErrorReporter();
		/// Reports a script error through the installed reporter, or with nlwarning when there is none.
		static void reportScriptError(const std::string &error, const std::string &luaScript = std::string());

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

		/// Forces the Garbage Collector to run. Returns the memory in use afterwards, in Kb.
		uint forceGarbageCollect();

		static void setEditorMode( bool b ){ editorMode = b; }

	private:
		CLuaManager();
		~CLuaManager();

		static CLuaManager *instance;
		static bool debugLua;
		static bool developmentLibraries;
		static bool editorMode;

		NLGUI::CLuaState *luaState;
	};

}

#endif
