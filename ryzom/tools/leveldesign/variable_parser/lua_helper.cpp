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

#include "stdafx.h"

#include "lua_helper.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
//#include "interface_manager.h"

extern "C"
{
#include "lualib.h"
}

// to get rid of you_must_not_use_assert___use_nl_assert___read_debug_h_file messages
#include <cassert>
#undef assert
#define assert nlassert
#include <luabind/luabind.hpp>

using namespace std;
using namespace NLMISC;

// ***************************************************************************
const char	*CLuaState::_NELSmallScriptTableName= "NELSmallScriptTable";
uint         CLuaStackChecker::_ExceptionContextCounter = 0;

// ***************************************************************************
void CLuaStackChecker::incrementExceptionContextCounter()
{
	++ _ExceptionContextCounter;
}

// ***************************************************************************
void CLuaStackChecker::decrementExceptionContextCounter()
{
	nlassert(_ExceptionContextCounter > 0);
	-- _ExceptionContextCounter;
}


#ifdef LUA_NEVRAX_VERSION
	ILuaIDEInterface *LuaDebuggerIDE = NULL;
	static bool LuaDebuggerVisible = false;
#endif

#ifdef NL_OS_WINDOWS
	HMODULE		LuaDebuggerModule = 0;
#endif

void luaDebuggerMainLoop()
{
#ifdef LUA_NEVRAX_VERSION
	if (!LuaDebuggerIDE) return;
	if (!LuaDebuggerVisible)
	{
		LuaDebuggerIDE->showDebugger(true);
		LuaDebuggerIDE->expandProjectTree();
		LuaDebuggerIDE->sortFiles();
		LuaDebuggerVisible = true;
	}
	LuaDebuggerIDE->doMainLoop();
#endif
}



static std::allocator<uint8> l_stlAlloc;


static void l_free_func(void *block, int oldSize)
{
	l_stlAlloc.deallocate((uint8 *) block, oldSize);
}

static void *l_realloc_func(void *b, int os, int s)
{
	if (os == s) return b;
	void *newB = l_stlAlloc.allocate(s);
	memcpy(newB, b, std::min(os, s));
	l_free_func(b, os);
	return newB;
}



const int MinGCThreshold = 128; // min value at which garbage collector will be triggered (in kilobytes)
// ***************************************************************************
CLuaState::CLuaState()
{
	_State = NULL;

	#ifdef LUA_NEVRAX_VERSION
		_GCThreshold = MinGCThreshold;
	#endif

	if (!_State)
	{
		#ifdef LUA_NEVRAX_VERSION
			_State = lua_open(l_realloc_func, l_free_func);
		#else
			_State = lua_open();
		#endif
		nlassert(_State);
	}

	// *** Load base libs
	{
		CLuaStackChecker lsc(this);
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
		luaL_openlibs(_State);
#else
		luaopen_base (_State);
		luaopen_table (_State);
		luaopen_io (_State);
		luaopen_string (_State);
		luaopen_math (_State);
		luaopen_debug (_State);
#endif

		// open are buggy????
		clear();
	}

	// *** Register basics
	CLuaStackChecker lsc(this);

	// do: LUA_REGISTRYINDEX.(lightuserdata*)this.classes= {}
	pushLightUserData((void *) this);
	newTable();
	push("classes");
	newTable(); // registry class
	setTable(-3);
	setTable(LUA_REGISTRYINDEX);

	// add pointer from lua state to this CLuaState object
	// do: LUA_REGISTRYINDEX.(lightuserdata*)_State= this
	pushLightUserData((void *) _State); // NB : as creator of the state,   we make the assumption that
	                       // no one will be using this pointer in the registry (cf. ref manual about registry)
	pushLightUserData((void *) this);
	setTable(LUA_REGISTRYINDEX);

	// Create the Table that contains Function cache for small script execution
	push(_NELSmallScriptTableName);			// 1:TableName
	newTable();								// 1:TableName 2:table
	setTable(LUA_REGISTRYINDEX);			// ...
	_SmallScriptPool= 0;

	// *** luabind init
	luabind::open(_State);
}


// ***************************************************************************
CLuaStackRestorer::CLuaStackRestorer(CLuaState *state, int finalSize) : _State(state), _FinalSize(finalSize)
{
}

// ***************************************************************************
CLuaStackRestorer::~CLuaStackRestorer()
{
	nlassert(_State);
	_State->setTop(_FinalSize);
}

#ifdef NL_OS_WINDOWS
	static int NoOpReportHook( int /* reportType */, char * /* message */, int * /* returnValue */ )
	{
		return TRUE;
	}
#endif


// ***************************************************************************
CLuaState::~CLuaState()
{
	nlassert(_State);

	#ifdef LUA_NEVRAX_VERSION
		if (!LuaDebuggerIDE)
	#else
		if (1)
	#endif
	{
		lua_close(_State);
	}
	else
	{
		#ifdef LUA_NEVRAX_VERSION
			LuaDebuggerIDE->stopDebug(); // this will also close the lua state
			LuaDebuggerIDE = NULL;
			LuaDebuggerVisible = false;
			#ifdef NL_OS_WINDOWS
				nlassert(LuaDebuggerModule)
				_CrtSetReportHook(NoOpReportHook); // prevent dump of memory leaks at this point
				//::FreeLibrary(LuaDebuggerModule); // don't free the library now (seems that it destroy, the main window, causing
													// a crash when the app window is destroyed for real...
													// -> FreeLibrary will be called when the application is closed
				LuaDebuggerModule = 0;
			#endif
		#endif
	}

	// Clear Small Script Cache
	_SmallScriptPool= 0;
	_SmallScriptCache.clear();
}

// ***************************************************************************
CLuaState *CLuaState::fromStatePointer(lua_State *state)
{
	nlassert(state);
	int initialStackSize = lua_gettop(state);
	lua_checkstack(state,   initialStackSize + 2);
	lua_pushlightuserdata(state,   (void *) state);
	lua_gettable(state,   LUA_REGISTRYINDEX);
	if (!lua_islightuserdata(state,   -1))
	{
		lua_pop(state,   1);
		return NULL;
	}
	CLuaState *ls = (CLuaState *) lua_touserdata(state,   -1);
	lua_pop(state,   1);
	nlassert(initialStackSize == lua_gettop(state));
	return ls;
}

// ***************************************************************************
struct CLuaReader
{
	const std::string *Str;
	bool               Done;
};

void CLuaState::loadScript(const std::string &code,   const std::string &dbgSrc)
{
	if (code.empty()) return;
	struct CHelper
	{
		static const char *luaChunkReaderFromString(lua_State * /* L */,   void *ud,   size_t *sz)
		{
			CLuaReader *rd = (CLuaReader *) ud;
			if (!rd->Done)
			{
				rd->Done = true;
				*sz = rd->Str->size();
				return rd->Str->c_str();
			}
			else
			{
				*sz = 0;
				return NULL;
			}
		}
	};
	CLuaReader rd;
	rd.Str = &code;
	rd.Done = false;

	int result = lua_load(_State,   CHelper::luaChunkReaderFromString,   (void *) &rd,   dbgSrc.c_str());
	if (result !=0)
	{
		// pop the error code
		string	err= toString();
		pop();
		// throw error
		throw ELuaParseError(err);
	}
}

// ***************************************************************************
void CLuaState::executeScriptInternal(const std::string &code,   const std::string &dbgSrc, int numRet)
{
	CLuaStackChecker lsc(this, numRet);

	// load the script
	loadScript(code,   dbgSrc);

	// execute
	if (pcall(0,   numRet) != 0)
	{
		// pop the error code
		string	err= toString();
		pop();
		// throw error
		throw ELuaExecuteError(err);
	}
}

// ***************************************************************************
void CLuaState::executeScript(const std::string &code, int numRet)
{
	// run the script,   with dbgSrc==script
	executeScriptInternal(code,   code, numRet);
}

// ***************************************************************************
bool CLuaState::executeScriptNoThrow(const std::string &code, int numRet)
{
	try
	{
		executeScript(code, numRet);
	}
	catch (ELuaError &e)
	{
		nlwarning(e.what());
		return false;
	}
	return true;
}

// ***************************************************************************
bool CLuaState::executeFile(const std::string &pathName)
{
	CIFile	inputFile;
	if(!inputFile.open(pathName))
		return false;

	#ifdef LUA_NEVRAX_VERSION
		if (LuaDebuggerIDE)
		{
			std::string path = NLMISC::CPath::getCurrentPath() + "/" + pathName.c_str();
			path = CPath::standardizeDosPath(path);
			LuaDebuggerIDE->addFile(path.c_str());
		}
	#endif

	// load the script text
	string	script;
	/*
	while(!inputFile.eof())
	{
		char	tmpBuff[5000];
		inputFile.getline(tmpBuff,   5000);
		script+= tmpBuff;
		script+= "\n";
	}
	*/
	script.resize(NLMISC::CFile::getFileSize(pathName));
	inputFile.serialBuffer((uint8 *) &script[0],  (uint)script.size());


	// execute the script text,   with dbgSrc==filename (use @ for lua internal purpose)
	executeScriptInternal(script,   string("@") + NLMISC::CFile::getFilename(pathName));

	return true;
}

// ***************************************************************************
void CLuaState::executeSmallScript(const std::string &script)
{
	if (script.empty()) return;
	// *** if the small script has not already been called before,   parse it now
	TSmallScriptCache::iterator it= _SmallScriptCache.find(script);
	if(it==_SmallScriptCache.end())
	{
		CLuaStackChecker lsc(this);

		// add it to a function
		loadScript(script,   script);

		// Assign the method to the NEL table: NELSmallScriptTable[_SmallScriptPool]= function
		push(_NELSmallScriptTableName);		// 1:function 2:NelTableName
		getTable(LUA_REGISTRYINDEX);		// 1:function 2:NelTable
		insert(-2);							// 1:NelTable 2:function
		rawSetI(-2,   _SmallScriptPool);		// 1:NelTable
		pop();

		// bkup in cache map
		it= _SmallScriptCache.insert(make_pair(script,   _SmallScriptPool)).first;

		// next allocated
		_SmallScriptPool++;
	}

	// *** Execute the function associated to the script
	CLuaStackChecker lsc(this);
	push(_NELSmallScriptTableName);		// 1:NelTableName
	getTable(LUA_REGISTRYINDEX);		// 1:NelTable
	// get the function at the given index in the "NELSmallScriptTable" table
	rawGetI(-1,   it->second);			// 1:NelTable 2:function

	// execute
	if (pcall(0,   0) != 0)
	{
		// Stack: 1: NelTable 2:errorcode
		// pop the error code,   and clear stack
		string	err= toString();
		pop();			// 1:NelTable
		pop();			// ....
		// throw error
		throw ELuaExecuteError(err);
	}
	else
	{
		// Stack: 1:NelTable
		pop();			// ....
	}
}

// ***************************************************************************
void		CLuaState::registerFunc(const char *name,   lua_CFunction function)
{
	lua_register(_State,   name,   function);
}

// ***************************************************************************
void		CLuaState::pushCClosure(lua_CFunction function,   int n)
{
	nlassert(function);
	nlassert(getTop() >= n);
	lua_pushcclosure(_State,   function,   n);
}

// ***************************************************************************
void CLuaState::push(TLuaWrappedFunction function)
{
	struct CForwarder
	{
		static int callFunc(lua_State *ls)
		{
			nlassert(ls);
			TLuaWrappedFunction func   = (TLuaWrappedFunction) lua_touserdata(ls,   lua_upvalueindex(1));
			CLuaState           *state = (CLuaState *) lua_touserdata(ls,   lua_upvalueindex(2));
			nlassert(func);
			nlassert(state);
			// get real function pointer from the values in the closure
			int numResults;
			int initialStackSize = state->getTop();
			try
			{
				// call the actual function
				numResults =  func(*state);
			}
			catch(const std::exception &e)
			{
				// restore stack to its initial size
				state->setTop(initialStackSize);
				lua_pushstring(ls,   e.what());
				// TODO : see if this is safe to call lua error there" ... (it does a long jump)
				lua_error(ls);
			}
			return numResults;
		}
	};
	pushLightUserData((void *) function);
	pushLightUserData((void *) this);
	pushCClosure(CForwarder::callFunc,   2);
}

// ***************************************************************************
// Wrapped function
void CLuaState::registerFunc(const char *name,   TLuaWrappedFunction function)
{
	nlassert(function);
	CLuaStackChecker lsc(this);
	push(name);
	push(function);
	setTable(LUA_GLOBALSINDEX);
}


// ***************************************************************************
bool			CLuaState::getTableBooleanValue(const char *name,    bool defaultValue)
{
	nlassert(name);
	push(name);
	getTable(-2);
	if (isNil())
	{
		pop();
		return defaultValue;
	}
	bool result = toBoolean(-1);
	pop();
	return result;
}

// ***************************************************************************
double		CLuaState::getTableNumberValue(const char *name,    double defaultValue)
{
	nlassert(name);
	push(name);
	getTable(-2);
	if (isNil())
	{
		pop();
		return defaultValue;
	}
	double result = toNumber(-1);
	pop();
	return result;
}

// ***************************************************************************
const char   *CLuaState::getTableStringValue(const char *name,   const char *defaultValue)
{
	nlassert(name);
	push(name);
	getTable(-2);
	if (isNil())
	{
		pop();
		return defaultValue;
	}
	const char *result = toString(-1);
	pop();
	return result;
}

// ***************************************************************************
void		CLuaState::getStackContext(string &ret,   uint stackLevel)
{
	nlassert(_State);
	ret.clear();
	lua_Debug	dbg;
	if(lua_getstack (_State,   stackLevel,   &dbg))
	{
		if(lua_getinfo(_State,   "lS",   &dbg))
		{
			ret= NLMISC::toString("%s:%d:",   dbg.short_src,   dbg.currentline);
		}
	}
}

// ***************************************************************************
int CLuaState::pcallByName(const char *functionName,   int nargs,   int nresults,   int funcTableIndex /*=LUA_GLOBALSINDEX*/,  int errfunc /*= 0*/)
{
	int initialStackSize = getTop();
	nlassert(functionName);
	nlassert(isTable(funcTableIndex));
	pushValue(funcTableIndex);
	push(functionName);
	getTable(-2);
	remove(-2); // get rid of the table
	nlassert(getTop() >= nargs); // not enough arguments on the stack
	// insert function before its arguments
	insert(- 1 - nargs);
	int result =  pcall(nargs,  nresults,  errfunc);
	int currSize = getTop();
	if (result == 0)
	{
		nlassert(currSize == initialStackSize - nargs + nresults);
	}
	else
	{
		// errors,  the stack contains a single string
		if (errfunc == 0)
		{
			nlassert(currSize == initialStackSize - nargs + 1);
		}
		// else if there's an error handler,  can't know the size of stack
	}
	return result;
}

// ***************************************************************************
void CLuaState::dumpStack()
{
	nlinfo("LUA STACK CONTENT (size = %d)", getTop());
	nlinfo("=================");
	CLuaStackChecker lsc(this);
	for(int k = 1; k <= getTop(); ++k)
	{
		pushValue(k);
		std::string value = toString(-1) ? toString(-1) : "?";
		nlinfo("Stack entry %d : type = %s, value = %s", k, getTypename(type(-1)), value.c_str());
		pop();
	}
}

// ***************************************************************************
void CLuaState::getStackAsString(std::string &dest)
{
	dest = NLMISC::toString("Stack size = %d\n", getTop());
	CLuaStackChecker lsc(this);
	for(int k = 1; k <= getTop(); ++k)
	{
		pushValue(k);
		std::string value = toString(-1) ? toString(-1) : "?";
		dest += NLMISC::toString("Stack entry %d : type = %s, value = %s\n", k, getTypename(type(-1)), value.c_str());
		pop();
	}
}

//================================================================================
CLuaStackChecker::~CLuaStackChecker()
{
	nlassert(_State);
	if (!_ExceptionContextCounter)
	{
		int currSize = _State->getTop();
		if (currSize != _FinalWantedSize)
		{
			static volatile bool assertWanted = true;
			if (assertWanted)
			{
				nlwarning("Lua stack size error : expected size is %d, current size is %d", _FinalWantedSize, currSize);
				_State->dumpStack();
				nlassert(0);
			}
		}
	}
	else
	{
		// this object dtor was called because an exception was thrown, so let the exception
		// propagate (the stack must be broken, but because of the exception, not because of code error)
		_State->setTop(_FinalWantedSize);
	}
}

// ***************************************************************************
void	ELuaWrappedFunctionException::init(CLuaState *ls, const std::string &reason)
{
	// Print first Lua Stack Context
/*
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ls)
	{
		ls->getStackContext(_Reason, 1);		// 1 because 0 is the current C function => return 1 for script called
		// enclose with cool colors
		pIM->formatLuaStackContext(_Reason);
	}
*/
	// Append the reason
	_Reason+= reason;
}

// ***************************************************************************
ELuaWrappedFunctionException::ELuaWrappedFunctionException(CLuaState *luaState)
{
	init(luaState, "");
}

// ***************************************************************************
ELuaWrappedFunctionException::ELuaWrappedFunctionException(CLuaState *luaState, const std::string &reason)
{
	init(luaState, reason);
}

// ***************************************************************************
ELuaWrappedFunctionException::ELuaWrappedFunctionException(CLuaState *luaState, const char *format, ...)
{
	//H_AUTO(Lua_ELuaWrappedFunctionException_ELuaWrappedFunctionException)
	std::string	reason;
	NLMISC_CONVERT_VARGS (reason, format, NLMISC::MaxCStringSize);
	init(luaState, reason);
}

//================================================================================
void         CLuaState::newTable()
{
	nlverify( lua_checkstack(_State, 1) );
	lua_newtable(_State);
}

//================================================================================
int          CLuaState::getGCCount()
{
	return lua_getgccount(_State);
}

//================================================================================
int          CLuaState::getGCThreshold()
{
	//H_AUTO(Lua_CLuaState_getGCThreshold)
#ifdef LUA_NEVRAX_VERSION
	return _GCThreshold;
#else
#	if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
	return lua_gc(_State, LUA_GCCOUNT, 0);
#	else
	return lua_getgcthreshold(_State);
#	endif
#endif
}

//================================================================================
void          CLuaState::setGCThreshold(int kb)
{
	//H_AUTO(Lua_CLuaState_setGCThreshold)
#ifdef LUA_NEVRAX_VERSION
	_GCThreshold = kb;
	handleGC();
#else
#	if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
	lua_gc(_State, LUA_GCCOLLECT, kb);
#	else
	lua_setgcthreshold(_State, kb);
#	endif
#endif
}

//================================================================================
void CLuaState::handleGC()
{
	//H_AUTO(Lua_CLuaState_handleGC)
	#ifdef LUA_NEVRAX_VERSION
		// must handle gc manually with the refcounted version
		int gcCount = getGCCount();
		if (gcCount >= _GCThreshold)
		{
			nlwarning("Triggering GC : memory in use = %d kb, current threshold = %d kb", gcCount, _GCThreshold);
			lua_setgcthreshold(_State, 0);
			gcCount = getGCCount();
			_GCThreshold = std::max(MinGCThreshold, gcCount * 2);
			nlwarning("After GC : memory in use = %d kb, threshold = %d kb", gcCount, _GCThreshold);
		}
	#endif
}


