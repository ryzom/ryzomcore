// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/lua_helper.h"
#include "nel/misc/file.h"

#ifdef LUA_NEVRAX_VERSION
	#include "lua_ide_dll_nevrax/include/lua_ide_dll/ide_interface.h" // external debugger
#endif


#include "nel/gui/lua_loadlib.h"

// to get rid of you_must_not_use_assert___use_nl_assert___read_debug_h_file messages
#include <cassert>
#ifdef assert
	#undef assert
#endif

#ifdef NL_DEBUG
	#define assert(x) nlassert(x)
#else
	#define assert(x)
#endif

#include <luabind/luabind.hpp>
#include <nel/misc/algo.h>
#include <nel/misc/path.h>

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	namespace LuaHelperStuff
	{
		void formatLuaStackContext( std::string &stackContext )
		{
			stackContext = std::string( "@{FC8A}" ).append( stackContext ).append( "@{FC8F} " );
		}

		std::string formatLuaErrorSysInfo( const std::string &error )
		{
			return std::string( "@{FC8F}" ).append( error );
		}
		
		std::string formatLuaErrorNlWarn( const std::string &error )
		{
			// Remove color tags (see formatLuaErrorSC())
			std::string ret = error;
			strFindReplace( ret, "@{FC8A}", "" );
			strFindReplace( ret, "@{FC8F}", "" );
			return ret;
		}
	}

	// ***************************************************************************
	const char	*CLuaState::_NELSmallScriptTableName= "NELSmallScriptTable";
	uint         CLuaStackChecker::_ExceptionContextCounter = 0;

	// ***************************************************************************
	void CLuaStackChecker::incrementExceptionContextCounter()
	{
		//H_AUTO(Lua_CLuaStackChecker_incrementExceptionContextCounter)
		++ _ExceptionContextCounter;
	}

	// ***************************************************************************
	void CLuaStackChecker::decrementExceptionContextCounter()
	{
		//H_AUTO(Lua_CLuaStackChecker_decrementExceptionContextCounter)
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
	CLuaState::CLuaState( bool debugger )
	{
		_State = NULL;

		#ifdef LUA_NEVRAX_VERSION
			_GCThreshold = MinGCThreshold;
		#endif

		#ifdef NL_OS_WINDOWS
		if( debugger )
		{
			#ifndef LUA_NEVRAX_VERSION
				static bool warningShown = false;
				if (!warningShown)
				{
					nldebug( "Lua debugger was asked, but the static lua library against which the client was linked is too old. Please update to lua-5.0.2_nevrax. Debugging won't be available!" );
					//MessageBox (NULL, "Lua debugger was asked, but the static lua library against which the client was linked is too old. Please update to lua-5.0.2_nevrax. Debugging won't be available!", "Lua support", MB_OK);
					warningShown = true;
				}
			#else
				nlassert(LuaDebuggerIDE == NULL); // for now, only one debugger supported...
				#ifdef NL_DEBUG
					LuaDebuggerModule = ::LoadLibrary("lua_ide2_dll_d.dll");
				#else
					LuaDebuggerModule = ::LoadLibrary("lua_ide2_dll_r.dll");
				#endif
				if (LuaDebuggerModule)
				{
					TGetLuaIDEInterfaceVersion getVersion = (TGetLuaIDEInterfaceVersion) GetProcAddress(LuaDebuggerModule, "GetLuaIDEInterfaceVersion");
					nlassert(getVersion);
					int dllInterfaceVersion = getVersion();
					if (dllInterfaceVersion > LUA_IDE_INTERFACE_VERSION)
					{
						MessageBox (NULL, "Lua debugger interface is newer than the application. Debugging will be disabled. Please update your client", "Lua support", MB_OK);
					}
					else if (dllInterfaceVersion < LUA_IDE_INTERFACE_VERSION)
					{
						MessageBox (NULL, "Lua debugger interface is too old. Lua debugging will be disabled. Please ask for a more recent dll.", "Lua support", MB_OK);
					}
					else
					{
						TGetLuaIDEInterface getter = (TGetLuaIDEInterface) GetProcAddress(LuaDebuggerModule, "GetLuaIDEInterface");
						nlassert(getter);
						LuaDebuggerIDE = getter();
						LuaDebuggerIDE->prepareDebug("save\\___external_debug.lpr", l_realloc_func, l_free_func, Driver->getDisplay());
						_State = LuaDebuggerIDE->getLuaState();
					}
				}
			#endif
		}
		#endif

		if (!_State)
		{
			#ifdef LUA_NEVRAX_VERSION
				_State = lua_open(l_realloc_func, l_free_func);
			#elif defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
				_State = luaL_newstate();
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

#ifdef _WIN32
			// Lua socket library for MobDebug, optional
			if (NLMISC::CFile::fileExists("socket\\core.dll"))
			{
				// Load socket\core.dll dynamically
				m_LuaSocket = LoadLibraryW(L"socket\\core.dll");
				if (!m_LuaSocket)
				{
					nlwarning("Lua socket library found, but failed to load");
				}
				else
				{
					void *luaopen_socket_core = (void *)GetProcAddress(m_LuaSocket, "luaopen_socket_core");
					if (!luaopen_socket_core)
					{
						nlwarning("Lua socket library loaded, but `luaopen_socket_core` not found");
						FreeLibrary(m_LuaSocket);
						m_LuaSocket = NULL;
					}
					else
					{
						// preload['socket.core'] = luaopen_socket_core
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
						lua_getglobal(_State, "package");
						lua_getfield(_State, -1, "preload");
						lua_pushcfunction(_State, (lua_CFunction)luaopen_socket_core);
						lua_setfield(_State, -2, "socket.core");
						lua_pop(_State, 2);
						nlinfo("Lua socket library preloaded");
#endif
					}
				}
			}
			else
			{
				m_LuaSocket = NULL;
			}
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
	CLuaStackRestorer::CLuaStackRestorer(CLuaState *state, int finalSize) : _FinalSize(finalSize), _State(state)
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

#ifdef _WIN32
		if (m_LuaSocket)
		{
			FreeLibrary(m_LuaSocket);
			m_LuaSocket = NULL;
		}
#endif
	}

	// ***************************************************************************
	CLuaState *CLuaState::fromStatePointer(lua_State *state)
	{
		//H_AUTO(Lua_CLuaState_fromStatePointer)
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
		//H_AUTO(Lua_CLuaState_loadScript)
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

		int result = lua_load(_State,   CHelper::luaChunkReaderFromString,   (void *) &rd,   dbgSrc.c_str()
#if LUA_VERSION_NUM >= 502
			, NULL
#endif
			);
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
		//H_AUTO(Lua_CLuaState_executeScriptInternal)
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
		//H_AUTO(Lua_CLuaState_executeScript)
		// run the script,   with dbgSrc==script
		executeScriptInternal(code,   code, numRet);
	}

	// ***************************************************************************
	bool CLuaState::executeScriptNoThrow(const std::string &code, int numRet)
	{
		//H_AUTO(Lua_CLuaState_executeScriptNoThrow)
		try
		{
			executeScript(code, numRet);
		}
		catch (const ELuaError &e)
		{
			nlwarning(e.what());
			return false;
		}
		return true;
	}

	// ***************************************************************************
	bool CLuaState::executeFile(const std::string &pathName)
	{
		//H_AUTO(Lua_CLuaState_executeFile)

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
		script.resize(CFile::getFileSize(pathName));
		inputFile.serialBuffer((uint8 *) &script[0],  (uint)script.size());


		// execute the script text,   with dbgSrc==filename (use @ for lua internal purpose)
#ifdef _WIN32
		// Paths need to be correct for debugging to work
		std::string pathNameStandardized = pathName;
		if (pathNameStandardized.size() > 1)
		{
			if (pathNameStandardized[1] == ':' && pathNameStandardized[0] >= 'a' && pathNameStandardized[0] <= 'z')
				pathNameStandardized[0] -= 'a' - 'A';
			for (ptrdiff_t i = 0; i < (ptrdiff_t)pathNameStandardized.size(); ++i)
			{
				if (pathNameStandardized[i] == '/')
					pathNameStandardized[i] = '\\';
			}
		}
#else
		const std::string &pathNameStandardized = pathName;
#endif
		executeScriptInternal(script, string("@") + pathNameStandardized);

		return true;
	}

	// ***************************************************************************
	void CLuaState::executeSmallScript(const std::string &script)
	{
		//H_AUTO(Lua_CLuaState_executeSmallScript)
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
		//H_AUTO(Lua_CLuaState_registerFunc)
		lua_register(_State,   name,   function);
	}

	// ***************************************************************************
	void		CLuaState::pushCClosure(lua_CFunction function,   int n)
	{
		//H_AUTO(Lua_CLuaState_pushCClosure)
		nlassert(function);
		nlassert(getTop() >= n);
		lua_pushcclosure(_State,   function,   n);
	}

	// ***************************************************************************
	void CLuaState::push(TLuaWrappedFunction function)
	{
		//H_AUTO(Lua_CLuaState_push)
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
				int numResults = 0;
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
		//H_AUTO(Lua_CLuaState_registerFunc)
		nlassert(function);
		CLuaStackChecker lsc(this);
#if LUA_VERSION_NUM >= 502
		pushGlobalTable();
#endif
		push(name);
		push(function);
#if LUA_VERSION_NUM >= 502
		setTable(-3); // -3 is the pushGlobalTable
		pop(1); // pop the pushGlobalTable value (setTable popped the 2 pushes)
#else
		setTable(LUA_GLOBALSINDEX);
#endif
	}


	// ***************************************************************************
	bool			CLuaState::getTableBooleanValue(const char *name,    bool defaultValue)
	{
		//H_AUTO(Lua_CLuaState_getTableBooleanValue)
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
		//H_AUTO(Lua_CLuaState_getTableNumberValue)
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
	sint64		CLuaState::getTableIntegerValue(const char *name,    sint64 defaultValue)
	{
		//H_AUTO(Lua_CLuaState_getTableIntegerValue)
		nlassert(name);
		push(name);
		getTable(-2);
		if (isNil())
		{
			pop();
			return defaultValue;
		}
		sint64 result = toInteger(-1);
		pop();
		return result;
	}

	// ***************************************************************************
	const char   *CLuaState::getTableStringValue(const char *name,   const char *defaultValue)
	{
		//H_AUTO(Lua_CLuaState_getTableStringValue)
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
		//H_AUTO(Lua_CLuaState_getStackContext)
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
	int CLuaState::pcallByNameGlobal(const char *functionName, int nargs, int nresults, int errfunc /*= 0*/)
	{
		int initialStackSize = getTop();
		nlassert(functionName);
#if LUA_VERSION_NUM >= 502
		pushGlobalTable();
#else
		nlassert(isTable(LUA_GLOBALSINDEX));
		pushValue(LUA_GLOBALSINDEX);
#endif
		return pcallByNameInternal(functionName, nargs, nresults, errfunc, initialStackSize);
	}

	int CLuaState::pcallByName(const char *functionName,   int nargs,   int nresults,   int funcTableIndex,  int errfunc /*= 0*/)
	{
		int initialStackSize = getTop();
		nlassert(functionName);
		nlassert(isTable(funcTableIndex));
		pushValue(funcTableIndex);
		return pcallByNameInternal(functionName, nargs, nresults, errfunc, initialStackSize);
	}

	int CLuaState::pcallByNameInternal(const char *functionName,   int nargs,   int nresults,  int errfunc /*= 0*/, int initialStackSize)
	{
		//H_AUTO(Lua_CLuaState_pcallByName)
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
		//H_AUTO(Lua_CLuaState_dumpStack)
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
		//H_AUTO(Lua_CLuaState_getStackAsString)
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
		//H_AUTO(Lua_ELuaWrappedFunctionException_init)
		// Print first Lua Stack Context
		if(ls)
		{
			ls->getStackContext(_Reason, 1);		// 1 because 0 is the current C function => return 1 for script called
			// enclose with cool colors
			LuaHelperStuff::formatLuaStackContext(_Reason);
		}

		// Append the reason
		_Reason+= reason;
	}

	// ***************************************************************************
	ELuaWrappedFunctionException::ELuaWrappedFunctionException(CLuaState *luaState)
	{
		//H_AUTO(Lua_ELuaWrappedFunctionException_ELuaWrappedFunctionException)
		init(luaState, "");
	}

	// ***************************************************************************
	ELuaWrappedFunctionException::ELuaWrappedFunctionException(CLuaState *luaState, const std::string &reason)
	{
		//H_AUTO(Lua_ELuaWrappedFunctionException_ELuaWrappedFunctionException)
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
		//H_AUTO(Lua_CLuaState_newTable)
		nlverify( lua_checkstack(_State, 1) );
		lua_newtable(_State);
	}

	//================================================================================
	int          CLuaState::getGCCount()
	{
		//H_AUTO(Lua_CLuaState_getGCCount)
#if LUA_VERSION_NUM >= 502
		// deprecated
		return 0;
#else
		return lua_getgccount(_State);
#endif
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

}
