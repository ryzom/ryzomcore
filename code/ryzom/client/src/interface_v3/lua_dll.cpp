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

#include "../stdpch.h"

extern "C"
{
//	#include "lua_loadlib.h"
}

#include "nel/misc/dynloadlib.h"
#include "lua_dll.h"

// from lua.h
Tlua_open lua_open;
Tlua_close lua_close;
Tlua_newthread lua_newthread;
Tlua_atpanic lua_atpanic;
Tlua_gettop lua_gettop;
Tlua_settop lua_settop;
Tlua_pushvalue lua_pushvalue;
Tlua_remove lua_remove;
Tlua_insert  lua_insert ;
Tlua_replace lua_replace;
Tlua_checkstack lua_checkstack;
Tlua_xmove lua_xmove;
Tlua_isnumber lua_isnumber;
Tlua_isstring lua_isstring;
Tlua_iscfunction lua_iscfunction;
Tlua_isuserdata lua_isuserdata;
Tlua_type lua_type;
Tlua_typename lua_typename;
Tlua_equal lua_equal;
Tlua_rawequal lua_rawequal;
Tlua_lessthan lua_lessthan;
Tlua_tonumber lua_tonumber;
Tlua_toboolean lua_toboolean;
Tlua_tostring lua_tostring;
Tlua_strlen lua_strlen;
Tlua_tocfunction lua_tocfunction;
Tlua_touserdata lua_touserdata;
Tlua_tothread lua_tothread;
Tlua_topointer lua_topointer;
Tlua_pushnil lua_pushnil;
Tlua_pushnumber lua_pushnumber;
Tlua_pushlstring lua_pushlstring;
Tlua_pushstring lua_pushstring;
Tlua_pushvfstring lua_pushvfstring;
Tlua_pushfstring lua_pushfstring;
Tlua_pushcclosure lua_pushcclosure;
Tlua_pushboolean lua_pushboolean;
Tlua_pushlightuserdata lua_pushlightuserdata;
Tlua_gettable lua_gettable;
Tlua_rawget lua_rawget;
Tlua_rawgeti lua_rawgeti;
Tlua_newtable lua_newtable;
Tlua_newuserdata lua_newuserdata;
Tlua_getmetatable lua_getmetatable;
Tlua_getfenv lua_getfenv;
Tlua_settable lua_settable;
Tlua_rawset lua_rawset;
Tlua_rawseti lua_rawseti;
Tlua_setmetatable lua_setmetatable;
Tlua_setfenv lua_setfenv;
Tlua_call lua_call;
Tlua_pcall lua_pcall;
Tlua_cpcall lua_cpcall;
Tlua_load lua_load;
Tlua_dump lua_dump;
Tlua_yield lua_yield;
Tlua_resume lua_resume;
Tlua_getgcthreshold lua_getgcthreshold;
Tlua_getgccount lua_getgccount;
Tlua_setgcthreshold lua_setgcthreshold;
Tlua_version lua_version;
Tlua_error lua_error;
Tlua_next lua_next;
Tlua_concat lua_concat;
Tlua_pushupvalues lua_pushupvalues;
Tlua_getstack lua_getstack;
Tlua_getinfo lua_getinfo;
Tlua_getlocal lua_getlocal;
Tlua_setlocal lua_setlocal;
Tlua_getupvalue lua_getupvalue;
Tlua_setupvalue lua_setupvalue;
Tlua_sethook lua_sethook;
Tlua_gethook lua_gethook;
Tlua_gethookmask lua_gethookmask;
Tlua_gethookcount lua_gethookcount;
// from lauxlib.h
TluaL_openlib luaL_openlib;
TluaL_getmetafield luaL_getmetafield;
TluaL_callmeta luaL_callmeta;
TluaL_typerror luaL_typerror;
TluaL_argerror luaL_argerror;
TluaL_checklstring luaL_checklstring;
TluaL_optlstring luaL_optlstring;
TluaL_checknumber luaL_checknumber;
TluaL_optnumber luaL_optnumber;
TluaL_checkstack luaL_checkstack;
TluaL_checktype luaL_checktype;
TluaL_checkany luaL_checkany;
TluaL_newmetatable luaL_newmetatable;
TluaL_getmetatable luaL_getmetatable;
TluaL_checkudata luaL_checkudata;
TluaL_where luaL_where;
TluaL_error luaL_error;
TluaL_findstring luaL_findstring;
TluaL_ref luaL_ref;
TluaL_unref luaL_unref;
TluaL_getn luaL_getn;
TluaL_setn luaL_setn;
TluaL_loadfile luaL_loadfile;
TluaL_loadbuffer luaL_loadbuffer;
TluaL_buffinit luaL_buffinit;
TluaL_prepbuffer luaL_prepbuffer;
TluaL_addlstring luaL_addlstring;
TluaL_addstring luaL_addstring;
TluaL_addvalue luaL_addvalue;
TluaL_pushresult luaL_pushresult;
Tlua_dofile lua_dofile;
Tlua_dostring lua_dostring;
Tlua_dobuffer lua_dobuffer;
// from lua lib.h
Tluaopen_base luaopen_base;
Tluaopen_table luaopen_table;
Tluaopen_io luaopen_io;
Tluaopen_string luaopen_string;
Tluaopen_math luaopen_math;
Tluaopen_debug luaopen_debug;
Tluaopen_loadlib luaopen_loadlib;

bool loadLuaDLL()
{
#ifdef NL_OS_WINDOWS
	HMODULE libHandle = ::LoadLibrary("lua.dll");
	if (!libHandle) return false;
	#define GET_LUA_PROC(name) name = (T##name) ::GetProcAddress(libHandle, #name); if (!name) return false;
	// from lua.h
	GET_LUA_PROC(lua_open)
	GET_LUA_PROC(lua_close)
	GET_LUA_PROC(lua_newthread)
	GET_LUA_PROC(lua_atpanic)
	GET_LUA_PROC(lua_gettop)
	GET_LUA_PROC(lua_settop)
	GET_LUA_PROC(lua_pushvalue)
	GET_LUA_PROC(lua_remove)
	GET_LUA_PROC(lua_insert )
	GET_LUA_PROC(lua_replace)
	GET_LUA_PROC(lua_checkstack)
	GET_LUA_PROC(lua_xmove)
	GET_LUA_PROC(lua_isnumber)
	GET_LUA_PROC(lua_isstring)
	GET_LUA_PROC(lua_iscfunction)
	GET_LUA_PROC(lua_isuserdata)
	GET_LUA_PROC(lua_type)
	GET_LUA_PROC(lua_typename)
	GET_LUA_PROC(lua_equal)
	GET_LUA_PROC(lua_rawequal)
	GET_LUA_PROC(lua_lessthan)
	GET_LUA_PROC(lua_tonumber)
	GET_LUA_PROC(lua_toboolean)
	GET_LUA_PROC(lua_tostring)
	GET_LUA_PROC(lua_strlen)
	GET_LUA_PROC(lua_tocfunction)
	GET_LUA_PROC(lua_touserdata)
	GET_LUA_PROC(lua_tothread)
	GET_LUA_PROC(lua_topointer)
	GET_LUA_PROC(lua_pushnil)
	GET_LUA_PROC(lua_pushnumber)
	GET_LUA_PROC(lua_pushlstring)
	GET_LUA_PROC(lua_pushstring)
	GET_LUA_PROC(lua_pushcclosure)
	GET_LUA_PROC(lua_pushboolean)
	GET_LUA_PROC(lua_pushlightuserdata)
	GET_LUA_PROC(lua_gettable)
	GET_LUA_PROC(lua_rawget)
	GET_LUA_PROC(lua_rawgeti)
	GET_LUA_PROC(lua_newtable)
	GET_LUA_PROC(lua_newuserdata)
	GET_LUA_PROC(lua_getmetatable)
	GET_LUA_PROC(lua_getfenv)
	GET_LUA_PROC(lua_settable)
	GET_LUA_PROC(lua_rawset)
	GET_LUA_PROC(lua_rawseti)
	GET_LUA_PROC(lua_setmetatable)
	GET_LUA_PROC(lua_setfenv)
	GET_LUA_PROC(lua_call)
	GET_LUA_PROC(lua_pcall)
	GET_LUA_PROC(lua_cpcall)
	GET_LUA_PROC(lua_load)
	GET_LUA_PROC(lua_dump)
	GET_LUA_PROC(lua_yield)
	GET_LUA_PROC(lua_resume)
	GET_LUA_PROC(lua_getgcthreshold)
	GET_LUA_PROC(lua_getgccount)
	GET_LUA_PROC(lua_setgcthreshold)
	GET_LUA_PROC(lua_version)
	GET_LUA_PROC(lua_error)
	GET_LUA_PROC(lua_next)
	GET_LUA_PROC(lua_concat)
	GET_LUA_PROC(lua_pushupvalues)
	GET_LUA_PROC(lua_getstack)
	GET_LUA_PROC(lua_getinfo)
	GET_LUA_PROC(lua_getlocal)
	GET_LUA_PROC(lua_setlocal)
	GET_LUA_PROC(lua_getupvalue)
	GET_LUA_PROC(lua_setupvalue)
	GET_LUA_PROC(lua_sethook)
	GET_LUA_PROC(lua_gethook)
	GET_LUA_PROC(lua_gethookmask)
	GET_LUA_PROC(lua_gethookcount)
	// from lauxlib.h
	GET_LUA_PROC(luaL_openlib)
	GET_LUA_PROC(luaL_getmetafield)
	GET_LUA_PROC(luaL_callmeta)
	GET_LUA_PROC(luaL_typerror)
	GET_LUA_PROC(luaL_argerror)
	GET_LUA_PROC(luaL_checklstring)
	GET_LUA_PROC(luaL_optlstring)
	GET_LUA_PROC(luaL_checknumber)
	GET_LUA_PROC(luaL_optnumber)
	GET_LUA_PROC(luaL_checkstack)
	GET_LUA_PROC(luaL_checktype)
	GET_LUA_PROC(luaL_checkany)
	GET_LUA_PROC(luaL_newmetatable)
	GET_LUA_PROC(luaL_getmetatable)
	GET_LUA_PROC(luaL_checkudata)
	GET_LUA_PROC(luaL_where)
	GET_LUA_PROC(luaL_error)
	GET_LUA_PROC(luaL_findstring)
	GET_LUA_PROC(luaL_ref)
	GET_LUA_PROC(luaL_unref)
	GET_LUA_PROC(luaL_getn)
	GET_LUA_PROC(luaL_setn)
	GET_LUA_PROC(luaL_loadfile)
	GET_LUA_PROC(luaL_loadbuffer)
	GET_LUA_PROC(luaL_buffinit)
	GET_LUA_PROC(luaL_prepbuffer)
	GET_LUA_PROC(luaL_addlstring)
	GET_LUA_PROC(luaL_addstring)
	GET_LUA_PROC(luaL_addvalue)
	GET_LUA_PROC(luaL_pushresult)
	GET_LUA_PROC(lua_dofile)
	GET_LUA_PROC(lua_dostring)
	GET_LUA_PROC(lua_dobuffer)
	// from lua lib.h
	GET_LUA_PROC(luaopen_base)
	GET_LUA_PROC(luaopen_table)
	GET_LUA_PROC(luaopen_io)
	GET_LUA_PROC(luaopen_string)
	GET_LUA_PROC(luaopen_math)
	GET_LUA_PROC(luaopen_debug)
	GET_LUA_PROC(luaopen_loadlib)

	return true;
#else
	return false;
#endif
}
