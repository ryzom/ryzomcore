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

/*
*
** Lua - An Extensible Extension Language
** Tecgraf: Computer Graphics Technology Group, PUC-Rio, Brazil
** http://www.lua.org	mailto:info@lua.org
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h


extern "C"
{

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>


#define LUA_VERSION	"Lua 5.0.2"
#define LUA_COPYRIGHT	"Copyright (C) 1994-2004 Tecgraf, PUC-Rio"
#define LUA_AUTHORS 	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"



/* option for multiple returns in `lua_pcall' and `lua_call' */
#define LUA_MULTRET	(-1)


/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_GLOBALSINDEX	(-10001)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* error codes for `lua_load' and `lua_pcall' */
#define LUA_ERRRUN	1
#define LUA_ERRFILE	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


typedef struct lua_State lua_State;

typedef int (*lua_CFunction) (lua_State *L);


/*
** functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*lua_Chunkreader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Chunkwriter) (lua_State *L, const void* p,
                                size_t sz, void* ud);


/*
** basic types
*/
#define LUA_TNONE	(-1)

#define LUA_TNIL	0
#define LUA_TBOOLEAN	1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER	3
#define LUA_TSTRING	4
#define LUA_TTABLE	5
#define LUA_TFUNCTION	6
#define LUA_TUSERDATA	7
#define LUA_TTHREAD	8


/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/*
** generic extra include file
*/
#ifdef LUA_USER_H
#include LUA_USER_H
#endif


/* type of numbers in Lua */
#ifndef LUA_NUMBER
typedef double lua_Number;
#else
typedef LUA_NUMBER lua_Number;
#endif


/* mark for all API functions */
#ifndef LUA_API
#define LUA_API		extern
#endif


/*
** state manipulation
*/
LUA_API lua_State *(*lua_open) (void);
LUA_API void       (*lua_close) (lua_State *L);
LUA_API lua_State *(*lua_newthread) (lua_State *L);

LUA_API lua_CFunction (*lua_atpanic) (lua_State *L, lua_CFunction panicf);


/*
** basic stack manipulation
*/
LUA_API int   (*lua_gettop) (lua_State *L);
LUA_API void  (*lua_settop) (lua_State *L, int idx);
LUA_API void  (*lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (*lua_remove) (lua_State *L, int idx);
LUA_API void  (*lua_insert )(lua_State *L, int idx);
LUA_API void  (*lua_replace) (lua_State *L, int idx);
LUA_API int   (*lua_checkstack) (lua_State *L, int sz);

LUA_API void  (*lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (*lua_isnumber) (lua_State *L, int idx);
LUA_API int             (*lua_isstring) (lua_State *L, int idx);
LUA_API int             (*lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (*lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (*lua_type) (lua_State *L, int idx);
LUA_API const char     *(*lua_typename) (lua_State *L, int tp);

LUA_API int            (*lua_equal) (lua_State *L, int idx1, int idx2);
LUA_API int            (*lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int            (*lua_lessthan) (lua_State *L, int idx1, int idx2);

LUA_API lua_Number      (*lua_tonumber) (lua_State *L, int idx);
LUA_API int             (*lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(*lua_tostring) (lua_State *L, int idx);
LUA_API size_t          (*lua_strlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (*lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(*lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(*lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(*lua_topointer) (lua_State *L, int idx);


/*
** push functions (C -> stack)
*/
LUA_API void  (*lua_pushnil) (lua_State *L);
LUA_API void  (*lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void  (*lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (*lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(*lua_pushvfstring) (lua_State *L, const char *fmt, ...);
LUA_API const char *(*lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (*lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (*lua_pushboolean) (lua_State *L, int b);
LUA_API void  (*lua_pushlightuserdata) (lua_State *L, void *p);


/*
** get functions (Lua -> stack)
*/
LUA_API void  (*lua_gettable) (lua_State *L, int idx);
LUA_API void  (*lua_rawget) (lua_State *L, int idx);
LUA_API void  (*lua_rawgeti) (lua_State *L, int idx, int n);
LUA_API void  (*lua_newtable) (lua_State *L);
LUA_API void *(*lua_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (*lua_getmetatable) (lua_State *L, int objindex);
LUA_API void  (*lua_getfenv) (lua_State *L, int idx);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (*lua_settable) (lua_State *L, int idx);
LUA_API void  (*lua_rawset) (lua_State *L, int idx);
LUA_API void  (*lua_rawseti) (lua_State *L, int idx, int n);
LUA_API int   (*lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (*lua_setfenv) (lua_State *L, int idx);


/*
** `load' and `call' functions (load and run Lua code)
*/
LUA_API void  (*lua_call) (lua_State *L, int nargs, int nresults);
LUA_API int   (*lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
LUA_API int (*lua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
LUA_API int   (*lua_load) (lua_State *L, lua_Chunkreader reader, void *dt,
                        const char *chunkname);

LUA_API int (*lua_dump) (lua_State *L, lua_Chunkwriter writer, void *data);


/*
** coroutine functions
*/
LUA_API int  (*lua_yield) (lua_State *L, int nresults);
LUA_API int  (*lua_resume) (lua_State *L, int narg);

/*
** garbage-collection functions
*/
LUA_API int   (*lua_getgcthreshold) (lua_State *L);
LUA_API int   (*lua_getgccount) (lua_State *L);
LUA_API void  (*lua_setgcthreshold) (lua_State *L, int newthreshold);

/*
** miscellaneous functions
*/

LUA_API const char *(*lua_version) (void);

LUA_API int   (*lua_error) (lua_State *L);

LUA_API int   (*lua_next) (lua_State *L, int idx);

LUA_API void  (*lua_concat) (lua_State *L, int n);



/*
** ===============================================================
** some useful macros
** ===============================================================
*/

#define lua_boxpointer(L,u) \
	(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))

#define lua_unboxpointer(L,i)	(*(void **)(lua_touserdata(L, i)))

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_register(L,n,f) \
	(lua_pushstring(L, n), \
	 lua_pushcfunction(L, f), \
	 lua_settable(L, LUA_GLOBALSINDEX))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, f, 0)

#define lua_isfunction(L,n)	(lua_type(L,n) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L,n) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L,n) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L,n) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L,n) == LUA_TBOOLEAN)
#define lua_isnone(L,n)		(lua_type(L,n) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L,n) <= 0)

#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)



/*
** compatibility macros and functions
*/


LUA_API int (*lua_pushupvalues) (lua_State *L);

#define lua_getregistry(L)	lua_pushvalue(L, LUA_REGISTRYINDEX)
#define lua_setglobal(L,s)	\
   (lua_pushstring(L, s), lua_insert(L, -2), lua_settable(L, LUA_GLOBALSINDEX))

#define lua_getglobal(L,s)	\
		(lua_pushstring(L, s), lua_gettable(L, LUA_GLOBALSINDEX))


/* compatibility with ref system */

/* pre-defined references */
#define LUA_NOREF	(-2)
#define LUA_REFNIL	(-1)

#define lua_ref(L,lock)	((lock) ? luaL_ref(L, LUA_REGISTRYINDEX) : \
      (lua_pushstring(L, "unlocked references are obsolete"), lua_error(L), 0))

#define lua_unref(L,ref)	luaL_unref(L, LUA_REGISTRYINDEX, (ref))

#define lua_getref(L,ref)	lua_rawgeti(L, LUA_REGISTRYINDEX, ref)



/*
** {======================================================================
** useful definitions for Lua kernel and libraries
** =======================================================================
*/

/* formats for Lua numbers */
#ifndef LUA_NUMBER_SCAN
#define LUA_NUMBER_SCAN		"%lf"
#endif

#ifndef LUA_NUMBER_FMT
#define LUA_NUMBER_FMT		"%.14g"
#endif

/* }====================================================================== */


/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct lua_Debug lua_Debug;  /* activation record */

typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int (*lua_getstack) (lua_State *L, int level, lua_Debug *ar);
LUA_API int (*lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *(*lua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(*lua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(*lua_getupvalue) (lua_State *L, int funcindex, int n);
LUA_API const char *(*lua_setupvalue) (lua_State *L, int funcindex, int n);

LUA_API int (*lua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook (*lua_gethook) (lua_State *L);
LUA_API int (*lua_gethookmask) (lua_State *L);
LUA_API int (*lua_gethookcount) (lua_State *L);


#define LUA_IDSIZE	60

struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) `global', `local', `field', `method' */
  const char *what;	/* (S) `Lua', `C', `main', `tail' */
  const char *source;	/* (S) */
  int currentline;	/* (l) */
  int nups;		/* (u) number of upvalues */
  int linedefined;	/* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int i_ci;  /* active function */
};

/* }====================================================================== */

// from lauxlib.h


#include "lua.h"


#ifndef LUALIB_API
#define LUALIB_API	LUA_API
#endif



typedef struct luaL_reg {
  const char *name;
  lua_CFunction func;
} luaL_reg;


LUALIB_API void (*luaL_openlib) (lua_State *L, const char *libname,
                               const luaL_reg *l, int nup);
LUALIB_API int (*luaL_getmetafield) (lua_State *L, int obj, const char *e);
LUALIB_API int (*luaL_callmeta) (lua_State *L, int obj, const char *e);
LUALIB_API int (*luaL_typerror) (lua_State *L, int narg, const char *tname);
LUALIB_API int (*luaL_argerror) (lua_State *L, int numarg, const char *extramsg);
LUALIB_API const char *(*luaL_checklstring) (lua_State *L, int numArg, size_t *l);
LUALIB_API const char *(*luaL_optlstring) (lua_State *L, int numArg,
                                           const char *def, size_t *l);
LUALIB_API lua_Number (*luaL_checknumber) (lua_State *L, int numArg);
LUALIB_API lua_Number (*luaL_optnumber) (lua_State *L, int nArg, lua_Number def);

LUALIB_API void (*luaL_checkstack) (lua_State *L, int sz, const char *msg);
LUALIB_API void (*luaL_checktype) (lua_State *L, int narg, int t);
LUALIB_API void (*luaL_checkany) (lua_State *L, int narg);

LUALIB_API int   (*luaL_newmetatable) (lua_State *L, const char *tname);
LUALIB_API void  (*luaL_getmetatable) (lua_State *L, const char *tname);
LUALIB_API void *(*luaL_checkudata) (lua_State *L, int ud, const char *tname);

LUALIB_API void (*luaL_where) (lua_State *L, int lvl);
LUALIB_API int (*luaL_error) (lua_State *L, const char *fmt, ...);

LUALIB_API int (*luaL_findstring) (const char *st, const char *const lst[]);

LUALIB_API int (*luaL_ref) (lua_State *L, int t);
LUALIB_API void (*luaL_unref) (lua_State *L, int t, int ref);

LUALIB_API int (*luaL_getn) (lua_State *L, int t);
LUALIB_API void (*luaL_setn) (lua_State *L, int t, int n);

LUALIB_API int (*luaL_loadfile) (lua_State *L, const char *filename);
LUALIB_API int (*luaL_loadbuffer) (lua_State *L, const char *buff, size_t sz,
                                const char *name);



/*
** ===============================================================
** some useful macros
** ===============================================================
*/

#define luaL_argcheck(L, cond,numarg,extramsg) if (!(cond)) \
                                               luaL_argerror(L, numarg,extramsg)
#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))
#define luaL_checkint(L,n)	((int)luaL_checknumber(L, n))
#define luaL_checklong(L,n)	((long)luaL_checknumber(L, n))
#define luaL_optint(L,n,d)	((int)luaL_optnumber(L, n,(lua_Number)(d)))
#define luaL_optlong(L,n,d)	((long)luaL_optnumber(L, n,(lua_Number)(d)))


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/



#ifndef LUAL_BUFFERSIZE
	#define LUAL_BUFFERSIZE	  BUFSIZ
#endif


typedef struct luaL_Buffer {
  char *p;			/* current position in buffer */
  int lvl;  /* number of strings in the stack (level) */
  lua_State *L;
  char buffer[LUAL_BUFFERSIZE];
} luaL_Buffer;

#define luaL_putchar(B,c) \
  ((void)((B)->p < ((B)->buffer+LUAL_BUFFERSIZE) || luaL_prepbuffer(B)), \
   (*(B)->p++ = (char)(c)))

#define luaL_addsize(B,n)	((B)->p += (n))

LUALIB_API void (*luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API char *(*luaL_prepbuffer) (luaL_Buffer *B);
LUALIB_API void (*luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (*luaL_addstring) (luaL_Buffer *B, const char *s);
LUALIB_API void (*luaL_addvalue) (luaL_Buffer *B);
LUALIB_API void (*luaL_pushresult) (luaL_Buffer *B);


/* }====================================================== */



/*
** Compatibility macros and functions
*/

LUALIB_API int   (*lua_dofile) (lua_State *L, const char *filename);
LUALIB_API int   (*lua_dostring) (lua_State *L, const char *str);
LUALIB_API int   (*lua_dobuffer) (lua_State *L, const char *buff, size_t sz,
                               const char *n);


#define luaL_check_lstr 	luaL_checklstring
#define luaL_opt_lstr 	luaL_optlstring
#define luaL_check_number 	luaL_checknumber
#define luaL_opt_number	luaL_optnumber
#define luaL_arg_check	luaL_argcheck
#define luaL_check_string	luaL_checkstring
#define luaL_opt_string	luaL_optstring
#define luaL_check_int	luaL_checkint
#define luaL_check_long	luaL_checklong
#define luaL_opt_int	luaL_optint
#define luaL_opt_long	luaL_optlong

// from lualob.h

#ifndef LUALIB_API
#define LUALIB_API	LUA_API
#endif


#define LUA_COLIBNAME	"coroutine"
LUALIB_API int (*luaopen_base) (lua_State *L);

#define LUA_TABLIBNAME	"table"
LUALIB_API int (*luaopen_table) (lua_State *L);

#define LUA_IOLIBNAME	"io"
#define LUA_OSLIBNAME	"os"
LUALIB_API int (*luaopen_io) (lua_State *L);

#define LUA_STRLIBNAME	"string"
LUALIB_API int (*luaopen_string) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
LUALIB_API int (*luaopen_math) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
LUALIB_API int (*luaopen_debug) (lua_State *L);


LUALIB_API int (*luaopen_loadlib) (lua_State *L);


/* to help testing the libraries */
#ifndef lua_assert
#define lua_assert(c)		/* empty */
#endif


/* compatibility code */
#define lua_baselibopen	luaopen_base
#define lua_tablibopen	luaopen_table
#define lua_iolibopen	luaopen_io
#define lua_strlibopen	luaopen_string
#define lua_mathlibopen	luaopen_math
#define lua_dblibopen	luaopen_debug

/******************************************************************************
* Copyright (C) 1994-2004 Tecgraf, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

// prototype of functions

// from lua.h
typedef lua_State *(*Tlua_open) (void);
typedef void       (*Tlua_close) (lua_State *L);
typedef lua_State *(*Tlua_newthread) (lua_State *L);
typedef lua_CFunction (*Tlua_atpanic) (lua_State *L, lua_CFunction panicf);
typedef int   (*Tlua_gettop) (lua_State *L);
typedef void  (*Tlua_settop) (lua_State *L, int idx);
typedef void  (*Tlua_pushvalue) (lua_State *L, int idx);
typedef void  (*Tlua_remove) (lua_State *L, int idx);
typedef void  (*Tlua_insert )(lua_State *L, int idx);
typedef void  (*Tlua_replace) (lua_State *L, int idx);
typedef int   (*Tlua_checkstack) (lua_State *L, int sz);
typedef void  (*Tlua_xmove) (lua_State *from, lua_State *to, int n);
typedef int   (*Tlua_isnumber) (lua_State *L, int idx);
typedef int   (*Tlua_isstring) (lua_State *L, int idx);
typedef int   (*Tlua_iscfunction) (lua_State *L, int idx);
typedef int   (*Tlua_isuserdata) (lua_State *L, int idx);
typedef int   (*Tlua_type) (lua_State *L, int idx);
typedef const char     *(*Tlua_typename) (lua_State *L, int tp);
typedef int (*Tlua_equal) (lua_State *L, int idx1, int idx2);
typedef int (*Tlua_rawequal) (lua_State *L, int idx1, int idx2);
typedef int (*Tlua_lessthan) (lua_State *L, int idx1, int idx2);
typedef lua_Number (*Tlua_tonumber) (lua_State *L, int idx);
typedef int (*Tlua_toboolean) (lua_State *L, int idx);
typedef const char *(*Tlua_tostring) (lua_State *L, int idx);
typedef size_t (*Tlua_strlen) (lua_State *L, int idx);
typedef lua_CFunction (*Tlua_tocfunction) (lua_State *L, int idx);
typedef void *(*Tlua_touserdata) (lua_State *L, int idx);
typedef lua_State *(*Tlua_tothread) (lua_State *L, int idx);
typedef const void *(*Tlua_topointer) (lua_State *L, int idx);
typedef void  (*Tlua_pushnil) (lua_State *L);
typedef void  (*Tlua_pushnumber) (lua_State *L, lua_Number n);
typedef void  (*Tlua_pushlstring) (lua_State *L, const char *s, size_t l);
typedef void  (*Tlua_pushstring) (lua_State *L, const char *s);
typedef const char *(*Tlua_pushvfstring) (lua_State *L, const char *fmt, ...);
typedef const char *(*Tlua_pushfstring) (lua_State *L, const char *fmt, ...);
typedef void  (*Tlua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
typedef void  (*Tlua_pushboolean) (lua_State *L, int b);
typedef void  (*Tlua_pushlightuserdata) (lua_State *L, void *p);
typedef void  (*Tlua_gettable) (lua_State *L, int idx);
typedef void  (*Tlua_rawget) (lua_State *L, int idx);
typedef void  (*Tlua_rawgeti) (lua_State *L, int idx, int n);
typedef void  (*Tlua_newtable) (lua_State *L);
typedef void *(*Tlua_newuserdata) (lua_State *L, size_t sz);
typedef int   (*Tlua_getmetatable) (lua_State *L, int objindex);
typedef void  (*Tlua_getfenv) (lua_State *L, int idx);
typedef void  (*Tlua_settable) (lua_State *L, int idx);
typedef void  (*Tlua_rawset) (lua_State *L, int idx);
typedef void  (*Tlua_rawseti) (lua_State *L, int idx, int n);
typedef int   (*Tlua_setmetatable) (lua_State *L, int objindex);
typedef int   (*Tlua_setfenv) (lua_State *L, int idx);
typedef void  (*Tlua_call) (lua_State *L, int nargs, int nresults);
typedef int   (*Tlua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
typedef int (*Tlua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
typedef int   (*Tlua_load) (lua_State *L, lua_Chunkreader reader, void *dt,
							const char *chunkname);
typedef int (*Tlua_dump) (lua_State *L, lua_Chunkwriter writer, void *data);
typedef int  (*Tlua_yield) (lua_State *L, int nresults);
typedef int  (*Tlua_resume) (lua_State *L, int narg);
typedef int   (*Tlua_getgcthreshold) (lua_State *L);
typedef int   (*Tlua_getgccount) (lua_State *L);
typedef void  (*Tlua_setgcthreshold) (lua_State *L, int newthreshold);
typedef const char *(*Tlua_version) (void);
typedef int   (*Tlua_error) (lua_State *L);
typedef int   (*Tlua_next) (lua_State *L, int idx);
typedef void  (*Tlua_concat) (lua_State *L, int n);
typedef int (*Tlua_pushupvalues) (lua_State *L);
typedef void (*Tlua_Hook) (lua_State *L, lua_Debug *ar);
typedef int (*Tlua_getstack) (lua_State *L, int level, lua_Debug *ar);
typedef int (*Tlua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
typedef const char *(*Tlua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
typedef const char *(*Tlua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
typedef const char *(*Tlua_getupvalue) (lua_State *L, int funcindex, int n);
typedef const char *(*Tlua_setupvalue) (lua_State *L, int funcindex, int n);
typedef int (*Tlua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
typedef lua_Hook (*Tlua_gethook) (lua_State *L);
typedef int (*Tlua_gethookmask) (lua_State *L);
typedef int (*Tlua_gethookcount) (lua_State *L);

// from lauxlib.h
typedef void (*TluaL_openlib) (lua_State *L, const char *libname,
                               const luaL_reg *l, int nup);
typedef int (*TluaL_getmetafield) (lua_State *L, int obj, const char *e);
typedef int (*TluaL_callmeta) (lua_State *L, int obj, const char *e);
typedef int (*TluaL_typerror) (lua_State *L, int narg, const char *tname);
typedef int (*TluaL_argerror) (lua_State *L, int numarg, const char *extramsg);
typedef const char *(*TluaL_checklstring) (lua_State *L, int numArg, size_t *l);
typedef const char *(*TluaL_optlstring) (lua_State *L, int numArg,
                                           const char *def, size_t *l);
typedef lua_Number (*TluaL_checknumber) (lua_State *L, int numArg);
typedef lua_Number (*TluaL_optnumber) (lua_State *L, int nArg, lua_Number def);

typedef void (*TluaL_checkstack) (lua_State *L, int sz, const char *msg);
typedef void (*TluaL_checktype) (lua_State *L, int narg, int t);
typedef void (*TluaL_checkany) (lua_State *L, int narg);

typedef int   (*TluaL_newmetatable) (lua_State *L, const char *tname);
typedef void  (*TluaL_getmetatable) (lua_State *L, const char *tname);
typedef void *(*TluaL_checkudata) (lua_State *L, int ud, const char *tname);

typedef void (*TluaL_where) (lua_State *L, int lvl);
typedef int (*TluaL_error) (lua_State *L, const char *fmt, ...);

typedef int (*TluaL_findstring) (const char *st, const char *const lst[]);

typedef int (*TluaL_ref) (lua_State *L, int t);
typedef void (*TluaL_unref) (lua_State *L, int t, int ref);

typedef int (*TluaL_getn) (lua_State *L, int t);
typedef void (*TluaL_setn) (lua_State *L, int t, int n);

typedef int (*TluaL_loadfile) (lua_State *L, const char *filename);
typedef int (*TluaL_loadbuffer) (lua_State *L, const char *buff, size_t sz,
                                const char *name);

typedef void (*TluaL_buffinit) (lua_State *L, luaL_Buffer *B);
typedef char *(*TluaL_prepbuffer) (luaL_Buffer *B);
typedef void (*TluaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);
typedef void (*TluaL_addstring) (luaL_Buffer *B, const char *s);
typedef void (*TluaL_addvalue) (luaL_Buffer *B);
typedef void (*TluaL_pushresult) (luaL_Buffer *B);
typedef int   (*Tlua_dofile) (lua_State *L, const char *filename);
typedef int   (*Tlua_dostring) (lua_State *L, const char *str);
typedef int   (*Tlua_dobuffer) (lua_State *L, const char *buff, size_t sz,
                               const char *n);

// from lualib.h
typedef int (*Tluaopen_base) (lua_State *L);
typedef int (*Tluaopen_table) (lua_State *L);
typedef int (*Tluaopen_io) (lua_State *L);
typedef int (*Tluaopen_string) (lua_State *L);
typedef int (*Tluaopen_math) (lua_State *L);
typedef int (*Tluaopen_debug) (lua_State *L);
typedef int (*Tluaopen_loadlib) (lua_State *L);



} // extern "C"

bool loadLuaDLL();

#endif
