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

#ifndef RZ_INCLUDE_LUA_HELPER_INLINE
	#error "don't include directly, include lua_helper.h instead"
#endif


/////////////////////
// CLuaStackChecker //
//////////////////////

//================================================================================
inline CLuaStackChecker::CLuaStackChecker(CLuaState *state, int numWantedResults)
{
	nlassert(state);
	_State = state;
	_FinalWantedSize = state->getTop() + numWantedResults;
}


///////////////
// CLuaState //
///////////////

//================================================================================
inline void CLuaState::checkIndex(int index)
{
	//H_AUTO(Lua_CLuaState_checkIndex)
	// NB : more restrictive test that in the documentation there, because
	// we don't expose the check stack function
#if LUA_VERSION_NUM >= 502
	nlassert( (index!=0 && abs(index) <= getTop())
		|| index == LUA_REGISTRYINDEX
		);
#else
	nlassert( (index!=0 && abs(index) <= getTop())
		|| index == LUA_REGISTRYINDEX
		|| index == LUA_GLOBALSINDEX
		);
#endif
}

//================================================================================
inline int  CLuaState::getTop()
{
	//H_AUTO(Lua_CLuaState_getTop)
	return lua_gettop(_State);
}

//================================================================================
inline void CLuaState::setTop(int index)
{
	//H_AUTO(Lua_CLuaState_setTop)
	// if index is strictly negative, this is a "pop". ensure that the user don't pop more than allowed
	if (index < 0)
	{
		checkIndex(index);
	}
	// if index is positive and is more than the current top
	else if ( index>getTop() )
	{
		// must ensure that we have enough stack space
		nlverify( lua_checkstack(_State, index-getTop()) );
	}

	// set top
	lua_settop(_State, index);
}

//================================================================================
inline void CLuaState::pushGlobalTable()
{
	//H_AUTO(Lua_CLuaState_pushGlobalTable)
#if LUA_VERSION_NUM >= 502
	lua_pushglobaltable(_State);
#else
	checkIndex(LUA_GLOBALSINDEX);
	lua_pushvalue(_State, LUA_GLOBALSINDEX);
#endif
}

//================================================================================
inline void CLuaState::pushValue(int index)
{
	//H_AUTO(Lua_CLuaState_pushValue)
	checkIndex(index);
	lua_pushvalue(_State, index);
}

//================================================================================
inline void CLuaState::remove(int index)
{
	//H_AUTO(Lua_CLuaState_remove)
	checkIndex(index);
	lua_remove(_State, index);
}

//================================================================================
inline void CLuaState::insert(int index)
{
	//H_AUTO(Lua_CLuaState_insert)
	checkIndex(index);
	lua_insert(_State, index);
}

//================================================================================
inline void CLuaState::replace(int index)
{
	//H_AUTO(Lua_CLuaState_replace)
	checkIndex(index);
	lua_replace(_State, index);
}

//================================================================================
inline void CLuaState::pop(int numElem /* = 1 */)
{
	//H_AUTO(Lua_CLuaState_pop)
	nlassert(numElem <= getTop());
	lua_pop(_State, numElem);
}

//================================================================================
inline int CLuaState::type(int index /* = -1 */)
{
	//H_AUTO(Lua_CLuaState_type)
	checkIndex(index);
	return lua_type(_State, index);
}

//================================================================================
inline const char *CLuaState::getTypename(int type)
{
	//H_AUTO(Lua_CLuaState_getTypename)
	return lua_typename(_State, type);
}

//================================================================================
inline bool CLuaState::isNil(int index)
{
	//H_AUTO(Lua_CLuaState_isNil)
	checkIndex(index);
	return lua_isnil(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isBoolean(int index)
{
	//H_AUTO(Lua_CLuaState_isBoolean)
	checkIndex(index);
	return lua_isboolean(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isNumber(int index)
{
	//H_AUTO(Lua_CLuaState_isNumber)
	checkIndex(index);
	return lua_isnumber(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isString(int index)
{
	//H_AUTO(Lua_CLuaState_isString)
	checkIndex(index);
	return lua_isstring(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isTable(int index)
{
	//H_AUTO(Lua_CLuaState_isTable)
	checkIndex(index);
	return lua_istable(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isFunction(int index)
{
	//H_AUTO(Lua_CLuaState_isFunction)
	checkIndex(index);
	return lua_isfunction(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isCFunction(int index)
{
	//H_AUTO(Lua_CLuaState_isCFunction)
	checkIndex(index);
	return lua_iscfunction(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isUserData(int index)
{
	//H_AUTO(Lua_CLuaState_isUserData)
	checkIndex(index);
	return lua_isuserdata(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::isLightUserData(int index)
{
	//H_AUTO(Lua_CLuaState_isLightUserData)
	checkIndex(index);
	return lua_islightuserdata(_State, index) != 0;
}

//================================================================================
inline bool CLuaState::toBoolean(int index)
{
	//H_AUTO(Lua_CLuaState_toBoolean)
	checkIndex(index);
	return lua_toboolean(_State, index) != 0;
}

//================================================================================
inline lua_Number CLuaState::toNumber(int index)
{
	//H_AUTO(Lua_CLuaState_toNumber)
	checkIndex(index);
	return lua_tonumber(_State, index);
}

//================================================================================
inline const char *CLuaState::toString(int index)
{
	//H_AUTO(Lua_CLuaState_toString)
	checkIndex(index);
	return lua_tostring(_State, index);
}

//================================================================================
inline void			CLuaState::toString(int index, std::string &str)
{
	//H_AUTO(Lua_CLuaState_toString)
	checkIndex(index);
	const char *pc= lua_tostring(_State, index);
	if(pc)
		str= pc;
	else
		str.clear();
}

//================================================================================
inline size_t CLuaState::strlen(int index)
{
	//H_AUTO(Lua_CLuaState_strlen)
	checkIndex(index);
#if LUA_VERSION_NUM >= 502
	return lua_rawlen(_State, index);
#else
	return lua_strlen(_State, index);
#endif
}

//================================================================================
inline lua_CFunction CLuaState::toCFunction(int index)
{
	//H_AUTO(Lua_CLuaState_toCFunction)
	checkIndex(index);
	return lua_tocfunction(_State, index);
}

//================================================================================
inline void *CLuaState::toUserData(int index)
{
	//H_AUTO(Lua_CLuaState_toUserData)
	checkIndex(index);
	return lua_touserdata(_State, index);
}

//================================================================================
inline const void *CLuaState::toPointer(int index)
{
	//H_AUTO(Lua_CLuaState_toPointer)
	checkIndex(index);
	return lua_topointer(_State, index);
}


//================================================================================
inline void CLuaState::push(bool value)
{
	//H_AUTO(Lua_CLuaState_push)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushboolean(_State, (int) value);
}

//================================================================================
inline void CLuaState::push(lua_Number value)
{
	//H_AUTO(Lua_CLuaState_push)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushnumber(_State, value);
}

//================================================================================
inline void CLuaState::push(const char *str)
{
	//H_AUTO(Lua_CLuaState_push)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushstring(_State, str);
}


//================================================================================
inline void CLuaState::push(const char *str, int length)
{
	//H_AUTO(Lua_CLuaState_push)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushlstring(_State, str, length);
}

//================================================================================
inline void         CLuaState::push(const std::string &str)
{
	//H_AUTO(Lua_CLuaState_push)
	nlverify( lua_checkstack(_State, 1) );
	push(str.c_str());
}

//================================================================================
inline void         CLuaState::pushNil()
{
	//H_AUTO(Lua_CLuaState_pushNil)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushnil(_State);
}

//================================================================================
inline void         CLuaState::push(lua_CFunction f)
{
	//H_AUTO(Lua_CLuaState_push)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushcfunction(_State, f);
}

//================================================================================
inline void         CLuaState::pushLightUserData(void *ptr)
{
	//H_AUTO(Lua_CLuaState_pushLightUserData)
	nlverify( lua_checkstack(_State, 1) );
	lua_pushlightuserdata(_State, ptr);
}

//================================================================================
inline bool         CLuaState::equal(int index1, int index2)
{
	//H_AUTO(Lua_CLuaState_equal)
	checkIndex(index1);
	checkIndex(index2);
#if LUA_VERSION_NUM >= 502
	return lua_compare(_State, index1, index2, LUA_OPEQ) != 0;
#else
	return lua_equal(_State, index1, index2) != 0;
#endif
}

//================================================================================
inline bool         CLuaState::getMetaTable(int index)
{
	//H_AUTO(Lua_CLuaState_getMetaTable)
	checkIndex(index);
	return lua_getmetatable(_State, index) != 0;
}

//================================================================================
inline bool         CLuaState::setMetaTable(int index)
{
	//H_AUTO(Lua_CLuaState_setMetaTable)
	checkIndex(index);
	return lua_setmetatable(_State, index) != 0;
}

//================================================================================
inline bool         CLuaState::rawEqual(int index1, int index2)
{
	//H_AUTO(Lua_CLuaState_rawEqual)
	checkIndex(index1);
	checkIndex(index2);
	return lua_rawequal(_State, index1, index2) != 0;
}

//================================================================================
inline bool         CLuaState::lessThan(int index1, int index2)
{
	//H_AUTO(Lua_CLuaState_lessThan)
	checkIndex(index1);
	checkIndex(index2);
#if LUA_VERSION_NUM >= 502
	return lua_compare(_State, index1, index2, LUA_OPLT) != 0;
#else
	return lua_lessthan(_State, index1, index2) != 0;
#endif
}


//================================================================================
inline void         CLuaState::concat(int numElem)
{
	//H_AUTO(Lua_CLuaState_concat)
	nlassert(numElem <= getTop());
	lua_concat(_State, numElem);
}

//================================================================================
inline void         CLuaState::getTable(int index)
{
	//H_AUTO(Lua_CLuaState_getTable)
	checkIndex(index);
	nlassert(isTable(index) || isUserData(index));
	lua_gettable(_State, index);
}

//================================================================================
inline void                CLuaState::rawGet(int index)
{
	//H_AUTO(Lua_CLuaState_rawGet)
	checkIndex(index);
	lua_rawget(_State, index);
}

//================================================================================
inline void                CLuaState::setTable(int index)
{
	//H_AUTO(Lua_CLuaState_setTable)
	checkIndex(index);
	nlassert(getTop() >= 2);
	nlassert(isTable(index));
	lua_settable(_State, index);
}

//================================================================================
inline void                CLuaState::rawSet(int index)
{
	//H_AUTO(Lua_CLuaState_rawSet)
	checkIndex(index);
	lua_rawset(_State, index);
}

//================================================================================
inline bool                CLuaState::next(int index)
{
	//H_AUTO(Lua_CLuaState_next)
	checkIndex(index);
	return lua_next(_State, index) != 0;
}

//================================================================================
inline void                CLuaState::rawSetI(int index, int n)
{
	//H_AUTO(Lua_CLuaState_rawSetI)
	checkIndex(index);
	lua_rawseti(_State, index, n);
}

//================================================================================
inline void                CLuaState::rawGetI(int index, int n)
{
	//H_AUTO(Lua_CLuaState_rawGetI)
	checkIndex(index);
	lua_rawgeti(_State, index, n);
}

//================================================================================
inline void                CLuaState::call(int nargs, int nresults)
{
	//H_AUTO(Lua_CLuaState_call)
	nlassert(getTop() >= nargs);
	lua_call(_State, nargs, nresults);
}

//================================================================================
inline int                 CLuaState::pcall(int nargs, int nresults, int errfunc)
{
	//H_AUTO(Lua_CLuaState_pcall)
	return lua_pcall(_State, nargs, nresults, errfunc);
}

//================================================================================
inline void					*CLuaState::newUserData(uint size)
{
	//H_AUTO(Lua_CLuaState_newUserData)
	nlassert(size>0);
	return lua_newuserdata(_State, size);
}
