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

#ifndef LUA_IDE_INTERFACE_H
#define LUA_IDE_INTERFACE_H

extern "C"
{
	#include "lua.h"
}

struct lua_State;

// this files requires inclusion of lua

const int LUA_IDE_INTERFACE_VERSION = 5; // please increment this version number each time 
                                          // this interface change


// called by the debugger in its event loop when the application is breaked
struct IDebuggedAppMainLoop
{
	// 
	virtual void breakEventLoop() = 0;
};

struct ILuaIDEInterface
{	
	virtual void	   prepareDebug(const char *tmpProjectFile, lua_realloc_t reallocfunc, lua_free_t freefunc, HWND mainWnd) = 0;
	virtual void	   stopDebug() = 0;
	virtual void       showDebugger(bool visible = true) = 0;
	virtual bool       isBreaked() = 0;
	virtual void       doMainLoop() = 0;		
	virtual void	   addFile(const char *filename) = 0;
	virtual void	   expandProjectTree() = 0;
	virtual void	   sortFiles() = 0;
	virtual void	   setBreakPoint(const char *name, int line) = 0;
	virtual void					setDebuggedAppMainLoop(IDebuggedAppMainLoop *mainAppLoop) = 0;
	virtual IDebuggedAppMainLoop	*setDebuggedAppMainLoop() const = 0;
	virtual lua_State *getLuaState() = 0;
	// write in the log
	virtual void	   debugInfo(const char *msg) = 0;
};

#ifdef LUA_IDE_DLL_EXPORTS
	#define LUA_IDE_API __declspec(dllexport)
#else
	#define LUA_IDE_API __declspec(dllimport)
#endif

typedef  int (* TGetLuaIDEInterfaceVersion) ();

// return the current version of the interface for the dll
typedef  ILuaIDEInterface *(* TGetLuaIDEInterface) ();

#endif