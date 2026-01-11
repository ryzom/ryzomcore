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

// stdlib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "../libapi/libapi.h"
#include "stdlib.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
    }
    return TRUE;
}

static int lua_trace(lua_State *L)
{
	Trace(luaL_check_string(L, 1));
	return 0;
}

STDLIB_API int LuaRegister(lua_State* L, HWND hWnd)
{
	InitLibAPI(hWnd);

	AttachConsole();

	lua_baselibopen(L);
	lua_tablibopen(L);
	lua_iolibopen(L);
	lua_strlibopen(L);
	lua_mathlibopen(L);

	lua_register(L, "Trace", lua_trace);

	return 0;
}


