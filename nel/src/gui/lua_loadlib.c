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
#include <windows.h>
#include <stdarg.h>
#include "lua_loadlib.h"

typedef   lua_State * (*Tlua_open) (void);
typedef  void       (*Tlua_close) (lua_State *L);
typedef  lua_State *(*Tlua_newthread) (lua_State *L);
typedef  lua_CFunction (*Tlua_atpanic) (lua_State *L, lua_CFunction panicf);
typedef  int   (*Tlua_gettop) (lua_State *L);
typedef  void  (*Tlua_settop) (lua_State *L, int idx);
typedef  void  (*Tlua_pushvalue) (lua_State *L, int idx);
typedef  void  (*Tlua_remove) (lua_State *L, int idx);
typedef  void  (*Tlua_insert )(lua_State *L, int idx);
typedef  void  (*Tlua_replace) (lua_State *L, int idx);
typedef  int   (*Tlua_checkstack) (lua_State *L, int sz);
typedef  void  (*Tlua_xmove) (lua_State *from, lua_State *to, int n);
typedef  int   (*Tlua_isnumber) (lua_State *L, int idx);
typedef  int   (*Tlua_isstring) (lua_State *L, int idx);
typedef  int   (*Tlua_iscfunction) (lua_State *L, int idx);
typedef  int   (*Tlua_isuserdata) (lua_State *L, int idx);
typedef  int   (*Tlua_type) (lua_State *L, int idx);
typedef  const char     *(*Tlua_typename) (lua_State *L, int tp);
typedef  int (*Tlua_equal) (lua_State *L, int idx1, int idx2);
typedef  int (*Tlua_rawequal) (lua_State *L, int idx1, int idx2);
typedef  int (*Tlua_lessthan) (lua_State *L, int idx1, int idx2);
typedef  lua_Number (*Tlua_tonumber) (lua_State *L, int idx);
typedef  int (*Tlua_toboolean) (lua_State *L, int idx);
typedef  const char *(*Tlua_tostring) (lua_State *L, int idx);
typedef  size_t (*Tlua_strlen) (lua_State *L, int idx);
typedef  lua_CFunction (*Tlua_tocfunction) (lua_State *L, int idx);
typedef  void *(*Tlua_touserdata) (lua_State *L, int idx);
typedef  lua_State *(*Tlua_tothread) (lua_State *L, int idx);
typedef  const void *(*Tlua_topointer) (lua_State *L, int idx);
typedef  void  (*Tlua_pushnil) (lua_State *L);
typedef  void  (*Tlua_pushnumber) (lua_State *L, lua_Number n);
typedef  void  (*Tlua_pushlstring) (lua_State *L, const char *s, size_t l);
typedef  void  (*Tlua_pushstring) (lua_State *L, const char *s);
typedef  const char *(*Tlua_pushvfstring) (lua_State *L, const char *fmt, va_list argp);
typedef  const char *(*Tlua_pushfstring) (lua_State *L, const char *fmt, ...);
typedef  void  (*Tlua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
typedef  void  (*Tlua_pushboolean) (lua_State *L, int b);
typedef  void  (*Tlua_pushlightuserdata) (lua_State *L, void *p);
typedef  void  (*Tlua_gettable) (lua_State *L, int idx);
typedef  void  (*Tlua_rawget) (lua_State *L, int idx);
typedef  void  (*Tlua_rawgeti) (lua_State *L, int idx, int n);
typedef  void  (*Tlua_newtable) (lua_State *L);
typedef  void *(*Tlua_newuserdata) (lua_State *L, size_t sz);
typedef  int   (*Tlua_getmetatable) (lua_State *L, int objindex);
typedef  void  (*Tlua_getfenv) (lua_State *L, int idx);
typedef  void  (*Tlua_settable) (lua_State *L, int idx);
typedef  void  (*Tlua_rawset) (lua_State *L, int idx);
typedef  void  (*Tlua_rawseti) (lua_State *L, int idx, int n);
typedef  int   (*Tlua_setmetatable) (lua_State *L, int objindex);
typedef  int   (*Tlua_setfenv) (lua_State *L, int idx);
typedef  void  (*Tlua_call) (lua_State *L, int nargs, int nresults);
typedef  int   (*Tlua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
typedef  int (*Tlua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
typedef  int   (*Tlua_load) (lua_State *L, lua_Chunkreader reader, void *dt,
							const char *chunkname);
typedef  int (*Tlua_dump) (lua_State *L, lua_Chunkwriter writer, void *data);
typedef  int  (*Tlua_yield) (lua_State *L, int nresults);
typedef  int  (*Tlua_resume) (lua_State *L, int narg);
typedef  int   (*Tlua_getgcthreshold) (lua_State *L);
typedef  int   (*Tlua_getgccount) (lua_State *L);
typedef  void  (*Tlua_setgcthreshold) (lua_State *L, int newthreshold);
typedef  const char *(*Tlua_version) (void);
typedef  int   (*Tlua_error) (lua_State *L);
typedef  int   (*Tlua_next) (lua_State *L, int idx);
typedef  void  (*Tlua_concat) (lua_State *L, int n);
typedef  int (*Tlua_pushupvalues) (lua_State *L);
typedef  void (*Tlua_Hook) (lua_State *L, lua_Debug *ar);
typedef  int (*Tlua_getstack) (lua_State *L, int level, lua_Debug *ar);
typedef  int (*Tlua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
typedef  const char *(*Tlua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
typedef  const char *(*Tlua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
typedef  const char *(*Tlua_getupvalue) (lua_State *L, int funcindex, int n);
typedef  const char *(*Tlua_setupvalue) (lua_State *L, int funcindex, int n);
typedef  int (*Tlua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
typedef  lua_Hook (*Tlua_gethook) (lua_State *L);
typedef  int (*Tlua_gethookmask) (lua_State *L);
typedef  int (*Tlua_gethookcount) (lua_State *L);

// from lauxlib.h
typedef  void (*TluaL_openlib) (lua_State *L, const char *libname,
                               const luaL_reg *l, int nup);
typedef  int (*TluaL_getmetafield) (lua_State *L, int obj, const char *e);
typedef  int (*TluaL_callmeta) (lua_State *L, int obj, const char *e);
typedef  int (*TluaL_typerror) (lua_State *L, int narg, const char *tname);
typedef  int (*TluaL_argerror) (lua_State *L, int numarg, const char *extramsg);
typedef  const char *(*TluaL_checklstring) (lua_State *L, int numArg, size_t *l);
typedef  const char *(*TluaL_optlstring) (lua_State *L, int numArg,
                                           const char *def, size_t *l);
typedef  lua_Number (*TluaL_checknumber) (lua_State *L, int numArg);
typedef  lua_Number (*TluaL_optnumber) (lua_State *L, int nArg, lua_Number def);

typedef  void (*TluaL_checkstack) (lua_State *L, int sz, const char *msg);
typedef  void (*TluaL_checktype) (lua_State *L, int narg, int t);
typedef  void (*TluaL_checkany) (lua_State *L, int narg);

typedef  int   (*TluaL_newmetatable) (lua_State *L, const char *tname);
typedef  void  (*TluaL_getmetatable) (lua_State *L, const char *tname);
typedef  void *(*TluaL_checkudata) (lua_State *L, int ud, const char *tname);

typedef  void (*TluaL_where) (lua_State *L, int lvl);
typedef  int (*TluaL_error) (lua_State *L, const char *fmt, ...);

typedef  int (*TluaL_findstring) (const char *st, const char *const lst[]);

typedef  int (*TluaL_ref) (lua_State *L, int t);
typedef  void (*TluaL_unref) (lua_State *L, int t, int ref);

typedef  int (*TluaL_getn) (lua_State *L, int t);
typedef  void (*TluaL_setn) (lua_State *L, int t, int n);

typedef  int (*TluaL_loadfile) (lua_State *L, const char *filename);
typedef  int (*TluaL_loadbuffer) (lua_State *L, const char *buff, size_t sz,
                                const char *name);

typedef  void (*TluaL_buffinit) (lua_State *L, luaL_Buffer *B);
typedef  char *(*TluaL_prepbuffer) (luaL_Buffer *B);
typedef  void (*TluaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);
typedef  void (*TluaL_addstring) (luaL_Buffer *B, const char *s);
typedef  void (*TluaL_addvalue) (luaL_Buffer *B);
typedef  void (*TluaL_pushresult) (luaL_Buffer *B);
typedef  int   (*Tlua_dofile) (lua_State *L, const char *filename);
typedef  int   (*Tlua_dostring) (lua_State *L, const char *str);
typedef  int   (*Tlua_dobuffer) (lua_State *L, const char *buff, size_t sz,
                               const char *n);

// from lualib.h
typedef  int (*Tluaopen_base) (lua_State *L);
typedef  int (*Tluaopen_table) (lua_State *L);
typedef  int (*Tluaopen_io) (lua_State *L);
typedef  int (*Tluaopen_string) (lua_State *L);
typedef  int (*Tluaopen_math) (lua_State *L);
typedef  int (*Tluaopen_debug) (lua_State *L);
typedef  int (*Tluaopen_loadlib) (lua_State *L);

// from lua.h
Tlua_open dlllua_open;
Tlua_close dlllua_close;
Tlua_newthread dlllua_newthread;
Tlua_atpanic dlllua_atpanic;
Tlua_gettop dlllua_gettop;
Tlua_settop dlllua_settop;
Tlua_pushvalue dlllua_pushvalue;
Tlua_remove dlllua_remove;
Tlua_insert  dlllua_insert ;
Tlua_replace dlllua_replace;
Tlua_checkstack dlllua_checkstack;
Tlua_xmove dlllua_xmove;
Tlua_isnumber dlllua_isnumber;
Tlua_isstring dlllua_isstring;
Tlua_iscfunction dlllua_iscfunction;
Tlua_isuserdata dlllua_isuserdata;
Tlua_type dlllua_type;
Tlua_typename dlllua_typename;
Tlua_equal dlllua_equal;
Tlua_rawequal dlllua_rawequal;
Tlua_lessthan dlllua_lessthan;
Tlua_tonumber dlllua_tonumber;
Tlua_toboolean dlllua_toboolean;
Tlua_tostring dlllua_tostring;
Tlua_strlen dlllua_strlen;
Tlua_tocfunction dlllua_tocfunction;
Tlua_touserdata dlllua_touserdata;
Tlua_tothread dlllua_tothread;
Tlua_topointer dlllua_topointer;
Tlua_pushnil dlllua_pushnil;
Tlua_pushnumber dlllua_pushnumber;
Tlua_pushlstring dlllua_pushlstring;
Tlua_pushstring dlllua_pushstring;
Tlua_pushvfstring dlllua_pushvfstring;
Tlua_pushfstring dlllua_pushfstring;
Tlua_pushcclosure dlllua_pushcclosure;
Tlua_pushboolean dlllua_pushboolean;
Tlua_pushlightuserdata dlllua_pushlightuserdata;
Tlua_gettable dlllua_gettable;
Tlua_rawget dlllua_rawget;
Tlua_rawgeti dlllua_rawgeti;
Tlua_newtable dlllua_newtable;
Tlua_newuserdata dlllua_newuserdata;
Tlua_getmetatable dlllua_getmetatable;
Tlua_getfenv dlllua_getfenv;
Tlua_settable dlllua_settable;
Tlua_rawset dlllua_rawset;
Tlua_rawseti dlllua_rawseti;
Tlua_setmetatable dlllua_setmetatable;
Tlua_setfenv dlllua_setfenv;
Tlua_call dlllua_call;
Tlua_pcall dlllua_pcall;
Tlua_cpcall dlllua_cpcall;
Tlua_load dlllua_load;
Tlua_dump dlllua_dump;
Tlua_yield dlllua_yield;
Tlua_resume dlllua_resume;
Tlua_getgcthreshold dlllua_getgcthreshold;
Tlua_getgccount dlllua_getgccount;
Tlua_setgcthreshold dlllua_setgcthreshold;
Tlua_version dlllua_version;
Tlua_error dlllua_error;
Tlua_next dlllua_next;
Tlua_concat dlllua_concat;
Tlua_pushupvalues dlllua_pushupvalues;
Tlua_getstack dlllua_getstack;
Tlua_getinfo dlllua_getinfo;
Tlua_getlocal dlllua_getlocal;
Tlua_setlocal dlllua_setlocal;
Tlua_getupvalue dlllua_getupvalue;
Tlua_setupvalue dlllua_setupvalue;
Tlua_sethook dlllua_sethook;
Tlua_gethook dlllua_gethook;
Tlua_gethookmask dlllua_gethookmask;
Tlua_gethookcount dlllua_gethookcount;
// from lauxlib.h
TluaL_openlib dllluaL_openlib;
TluaL_getmetafield dllluaL_getmetafield;
TluaL_callmeta dllluaL_callmeta;
TluaL_typerror dllluaL_typerror;
TluaL_argerror dllluaL_argerror;
TluaL_checklstring dllluaL_checklstring;
TluaL_optlstring dllluaL_optlstring;
TluaL_checknumber dllluaL_checknumber;
TluaL_optnumber dllluaL_optnumber;
TluaL_checkstack dllluaL_checkstack;
TluaL_checktype dllluaL_checktype;
TluaL_checkany dllluaL_checkany;
TluaL_newmetatable dllluaL_newmetatable;
TluaL_getmetatable dllluaL_getmetatable;
TluaL_checkudata dllluaL_checkudata;
TluaL_where dllluaL_where;
TluaL_error dllluaL_error;
TluaL_findstring dllluaL_findstring;
TluaL_ref dllluaL_ref;
TluaL_unref dllluaL_unref;
TluaL_getn dllluaL_getn;
TluaL_setn dllluaL_setn;
TluaL_loadfile dllluaL_loadfile;
TluaL_loadbuffer dllluaL_loadbuffer;
TluaL_buffinit dllluaL_buffinit;
TluaL_prepbuffer dllluaL_prepbuffer;
TluaL_addlstring dllluaL_addlstring;
TluaL_addstring dllluaL_addstring;
TluaL_addvalue dllluaL_addvalue;
TluaL_pushresult dllluaL_pushresult;
Tlua_dofile dlllua_dofile;
Tlua_dostring dlllua_dostring;
Tlua_dobuffer dlllua_dobuffer;
// from lua lib.h
Tluaopen_base dllluaopen_base;
Tluaopen_table dllluaopen_table;
Tluaopen_io dllluaopen_io;
Tluaopen_string dllluaopen_string;
Tluaopen_math dllluaopen_math;
Tluaopen_debug dllluaopen_debug;
Tluaopen_loadlib dllluaopen_loadlib;

// call to actual dll function
lua_State *lua_open (void) { return dlllua_open(); }
void       lua_close (lua_State *L) { dlllua_close(L); }
lua_State *lua_newthread (lua_State *L) { return dlllua_newthread(L); }
lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) { return dlllua_atpanic(L, panicf); }
int   lua_gettop (lua_State *L) { return dlllua_gettop(L); }
void  lua_settop (lua_State *L, int idx) { dlllua_settop(L, idx); }
void  lua_pushvalue (lua_State *L, int idx) { dlllua_pushvalue(L, idx); }
void  lua_remove (lua_State *L, int idx) { dlllua_remove(L, idx); }
void  lua_insert (lua_State *L, int idx) { dlllua_insert (L, idx); }
void  lua_replace (lua_State *L, int idx) { dlllua_replace(L, idx); }
int   lua_checkstack (lua_State *L, int sz) { return dlllua_checkstack(L, sz); }
void  lua_xmove (lua_State *from, lua_State *to, int n) { dlllua_xmove(from, to, n); }
int   lua_isnumber (lua_State *L, int idx) { return dlllua_isnumber(L, idx); }
int   lua_isstring (lua_State *L, int idx) { return dlllua_isstring(L, idx); }
int   lua_iscfunction (lua_State *L, int idx) { return dlllua_iscfunction(L, idx); }
int   lua_isuserdata (lua_State *L, int idx) { return dlllua_isuserdata(L, idx); }
int   lua_type (lua_State *L, int idx) { return dlllua_type(L, idx); }
const char     *lua_typename (lua_State *L, int tp) { return dlllua_typename(L, tp); }
int lua_equal (lua_State *L, int idx1, int idx2) { return dlllua_equal(L, idx1, idx2); }
int lua_rawequal (lua_State *L, int idx1, int idx2) { return dlllua_rawequal(L, idx1, idx2); }
int lua_lessthan (lua_State *L, int idx1, int idx2) { return dlllua_lessthan(L, idx1, idx2); }
lua_Number lua_tonumber (lua_State *L, int idx) { return dlllua_tonumber(L, idx); }
int lua_toboolean (lua_State *L, int idx) { return dlllua_toboolean(L, idx); }
const char *lua_tostring (lua_State *L, int idx) { return dlllua_tostring(L, idx); }
size_t lua_strlen (lua_State *L, int idx) { return dlllua_strlen(L, idx); }
lua_CFunction lua_tocfunction (lua_State *L, int idx) { return dlllua_tocfunction(L, idx); }
void *lua_touserdata (lua_State *L, int idx) { return dlllua_touserdata(L, idx); }
lua_State *lua_tothread (lua_State *L, int idx) { return dlllua_tothread(L, idx); }
const void *lua_topointer (lua_State *L, int idx) { return dlllua_topointer(L, idx); }
void  lua_pushnil (lua_State *L) { dlllua_pushnil(L); }
void  lua_pushnumber (lua_State *L, lua_Number n) { dlllua_pushnumber(L, n); }
void  lua_pushlstring (lua_State *L, const char *s, size_t l) { dlllua_pushlstring(L, s, l); }
void  lua_pushstring (lua_State *L, const char *s) { dlllua_pushstring(L, s); }

const char *lua_pushvfstring (lua_State *L, const char *fmt, va_list argp)
{
	const char *result;
	va_list _args;
	va_start (_args, fmt);
	result = dlllua_pushvfstring(L, fmt, _args);
	va_end(_args);
	return result;
}

const char *lua_pushfstring (lua_State *L, const char *fmt, ...)
{
	const char *result;
	va_list _args;
	va_start (_args, fmt);
	result = dlllua_pushfstring(L, fmt, _args);
	va_end(_args);
	return result;
}

void  lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) { dlllua_pushcclosure(L, fn, n); }
void  lua_pushboolean (lua_State *L, int b) { dlllua_pushboolean(L, b); }
void  lua_pushlightuserdata (lua_State *L, void *p) { dlllua_pushlightuserdata(L, p); }
void  lua_gettable (lua_State *L, int idx) { dlllua_gettable(L, idx); }
void  lua_rawget (lua_State *L, int idx) { dlllua_rawget(L, idx); }
void  lua_rawgeti (lua_State *L, int idx, int n) { dlllua_rawgeti(L, idx, n); }
void  lua_newtable (lua_State *L) { dlllua_newtable(L); }
void *lua_newuserdata (lua_State *L, size_t sz) { return dlllua_newuserdata(L, sz); }
int   lua_getmetatable (lua_State *L, int objindex) { return dlllua_getmetatable(L, objindex); }
void  lua_getfenv (lua_State *L, int idx) { dlllua_getfenv(L, idx); }
void  lua_settable (lua_State *L, int idx) { dlllua_settable(L, idx); }
void  lua_rawset (lua_State *L, int idx) { dlllua_rawset(L, idx); }
void  lua_rawseti (lua_State *L, int idx, int n) { dlllua_rawseti(L, idx, n); }
int   lua_setmetatable (lua_State *L, int objindex) { return dlllua_setmetatable(L, objindex); }
int   lua_setfenv (lua_State *L, int idx) { return dlllua_setfenv(L, idx); }
void  lua_call (lua_State *L, int nargs, int nresults) { dlllua_call(L, nargs, nresults); }
int   lua_pcall (lua_State *L, int nargs, int nresults, int errfunc) { return dlllua_pcall(L, nargs, nresults, errfunc); }
int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) { return dlllua_cpcall(L, func, ud); }
int   lua_load (lua_State *L, lua_Chunkreader reader, void *dt, const char *chunkname)
{
	return dlllua_load(L, reader, dt, chunkname);
}
int lua_dump (lua_State *L, lua_Chunkwriter writer, void *data) { return dlllua_dump(L, writer, data); }
int  lua_yield (lua_State *L, int nresults) { return dlllua_yield(L, nresults); }
int  lua_resume (lua_State *L, int narg) { return dlllua_resume(L, narg); }
int   lua_getgcthreshold (lua_State *L) { return dlllua_getgcthreshold(L); }
int   lua_getgccount (lua_State *L) { return dlllua_getgccount(L); }
void  lua_setgcthreshold (lua_State *L, int newthreshold) { dlllua_setgcthreshold(L, newthreshold); }
const char *lua_version (void) { return dlllua_version(); }
int   lua_error (lua_State *L) { return dlllua_error(L); }
int   lua_next (lua_State *L, int idx) { return dlllua_next(L, idx); }
void  lua_concat (lua_State *L, int n) { dlllua_concat(L, n); }
int lua_pushupvalues (lua_State *L) { return dlllua_pushupvalues(L); }
int lua_getstack (lua_State *L, int level, lua_Debug *ar) { return dlllua_getstack(L, level, ar); }
int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) { return dlllua_getinfo(L, what, ar); }
const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n) { return dlllua_getlocal(L, ar, n); }
const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n) { return dlllua_setlocal(L, ar, n); }
const char *lua_getupvalue (lua_State *L, int funcindex, int n) { return dlllua_getupvalue(L, funcindex, n); }
const char *lua_setupvalue (lua_State *L, int funcindex, int n) { return dlllua_setupvalue(L, funcindex, n); }
int lua_sethook (lua_State *L, lua_Hook func, int mask, int count) { return dlllua_sethook(L, func, mask, count); }
lua_Hook lua_gethook (lua_State *L) { return dlllua_gethook(L); }
int lua_gethookmask (lua_State *L) { return dlllua_gethookmask(L); }
int lua_gethookcount (lua_State *L) { return dlllua_gethookcount(L); }

// from lauxlib.h
void luaL_openlib (lua_State *L, const char *libname, const luaL_reg *l, int nup) { dllluaL_openlib(L, libname, l, nup); }
int luaL_getmetafield (lua_State *L, int obj, const char *e) { return dllluaL_getmetafield(L, obj, e); }
int luaL_callmeta (lua_State *L, int obj, const char *e) { return dllluaL_callmeta(L, obj, e); }
int luaL_typerror (lua_State *L, int narg, const char *tname) { return dllluaL_typerror(L, narg, tname); }
int luaL_argerror (lua_State *L, int numarg, const char *extramsg) { return dllluaL_argerror(L, numarg, extramsg); }
const char *luaL_checklstring (lua_State *L, int numArg, size_t *l) { return dllluaL_checklstring(L, numArg, l); }
const char *luaL_optlstring (lua_State *L, int numArg, const char *def, size_t *l) { return dllluaL_optlstring(L, numArg, def, l); }
lua_Number luaL_checknumber (lua_State *L, int numArg) { return dllluaL_checknumber(L, numArg); }
lua_Number luaL_optnumber (lua_State *L, int nArg, lua_Number def) { return dllluaL_optnumber(L, nArg, def); }
void luaL_checkstack (lua_State *L, int sz, const char *msg) { dllluaL_checkstack(L, sz, msg); }
void luaL_checktype (lua_State *L, int narg, int t) { dllluaL_checktype(L, narg, t); }
void luaL_checkany (lua_State *L, int narg) { dllluaL_checkany(L, narg); }
int   luaL_newmetatable (lua_State *L, const char *tname) { return dllluaL_newmetatable(L, tname); }
void  luaL_getmetatable (lua_State *L, const char *tname) { dllluaL_getmetatable(L, tname); }
void *luaL_checkudata (lua_State *L, int ud, const char *tname) { return dllluaL_checkudata(L, ud, tname); }
void luaL_where (lua_State *L, int lvl) { dllluaL_where(L, lvl); }

int luaL_error (lua_State *L, const char *fmt, ...)
{
	int result;
	va_list _args;
	va_start (_args, fmt);
	result = dllluaL_error(L, fmt, _args);
	va_end(_args);
	return result;
}
int luaL_findstring (const char *st, const char *const lst[]) { return dllluaL_findstring(st, lst); }
int luaL_ref (lua_State *L, int t) { return dllluaL_ref(L, t); }
void luaL_unref (lua_State *L, int t, int ref) { dllluaL_unref(L, t, ref); }
int luaL_getn (lua_State *L, int t) { return dllluaL_getn(L, t); }
void luaL_setn (lua_State *L, int t, int n) { dllluaL_setn(L, t, n); }
int luaL_loadfile (lua_State *L, const char *filename) { return dllluaL_loadfile(L, filename); }
int luaL_loadbuffer (lua_State *L, const char *buff, size_t sz, const char *name) { return dllluaL_loadbuffer(L, buff, sz, name); }

void luaL_buffinit (lua_State *L, luaL_Buffer *B) { dllluaL_buffinit(L, B); }
char *luaL_prepbuffer (luaL_Buffer *B) { return dllluaL_prepbuffer(B); }

void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) { dllluaL_addlstring(B, s, l); }
void luaL_addstring (luaL_Buffer *B, const char *s) { dllluaL_addstring(B, s); }
void luaL_addvalue (luaL_Buffer *B) { dllluaL_addvalue(B); }
void luaL_pushresult (luaL_Buffer *B) { dllluaL_pushresult(B); }
int   lua_dofile (lua_State *L, const char *filename) { return dlllua_dofile(L, filename); }
int   lua_dostring (lua_State *L, const char *str) { return dlllua_dostring(L, str); }
int   lua_dobuffer (lua_State *L, const char *buff, size_t sz, const char *n) { return dlllua_dobuffer(L, buff, sz, n); }

// from lualib.h
int luaopen_base (lua_State *L) { return dllluaopen_base(L); }
int luaopen_table (lua_State *L) { return dllluaopen_table(L); }
int luaopen_io (lua_State *L) { return dllluaopen_io(L); }
int luaopen_string (lua_State *L) { return dllluaopen_string(L); }
int luaopen_math (lua_State *L) { return dllluaopen_math(L); }
int luaopen_debug (lua_State *L) { return dllluaopen_debug(L); }
int luaopen_loadlib (lua_State *L) { return dllluaopen_loadlib(L); }


int loadLuaDLL()
{
	HMODULE libHandle = LoadLibrary("lua.dll");
	if (!libHandle) return 0;
#define GET_LUA_PROC(name) dll##name = (T##name) GetProcAddress(libHandle, #name); if (!dll##name) return 0;
	// from lua.h
	GET_LUA_PROC(lua_close)
	GET_LUA_PROC(lua_newthread)
	GET_LUA_PROC(lua_atpanic)
	GET_LUA_PROC(lua_gettop)
	GET_LUA_PROC(lua_settop)
	GET_LUA_PROC(lua_pushvalue)
	GET_LUA_PROC(lua_remove)
	GET_LUA_PROC(lua_insert)
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

	return 1;
}
