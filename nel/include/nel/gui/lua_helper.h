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

#ifndef RZ_LUA_HELPER_H
#define RZ_LUA_HELPER_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

extern "C"
{
	#include "lua_loadlib.h"
}

namespace NLGUI
{

	class CLuaState;

	namespace LuaHelperStuff
	{
		void formatLuaStackContext( std::string &stackContext );
		std::string	formatLuaErrorSysInfo( const std::string &error );
		std::string formatLuaErrorNlWarn( const std::string &error );
	}


	// ***************************************************************************
	/** Helper class to see if a stack is restored at its initial size (or with n return results).
	  * Check that the stack size remains unchanged when the object goes out of scope
	  */
	class CLuaStackChecker
	{
	public:
		CLuaStackChecker(CLuaState *state, int numWantedResults = 0);
		~CLuaStackChecker();
		/** Increment exception context counter
		  * When an exception is thrown, lua stack checker do any assert bu will
		  * rather restore the lua stack at its original size, and will
		  * let the exception a chance to propagate
		  */
		static void incrementExceptionContextCounter();
		static void decrementExceptionContextCounter();

	private:
		CLuaState    *_State;
		int          _FinalWantedSize;
		static uint  _ExceptionContextCounter;

	};

	// **************************************************************************
	/** Helper class to restore the lua stack to the desired size when this object goes out of scope
	  */
	class CLuaStackRestorer
	{
	public:
		CLuaStackRestorer(CLuaState *state, int finalSize);
		~CLuaStackRestorer();
	private:
		int _FinalSize;
		CLuaState *_State;
	};

	////////////////
	// EXCEPTIONS //
	////////////////

	class ELuaError : public NLMISC::Exception
	{
	public:
		ELuaError() { CLuaStackChecker::incrementExceptionContextCounter(); }
		virtual ~ELuaError() NL_OVERRIDE { CLuaStackChecker::decrementExceptionContextCounter(); }
		ELuaError(const std::string &reason) : Exception(reason) { CLuaStackChecker::incrementExceptionContextCounter(); }
		// what(), plus append the Reason
		virtual std::string luaWhat() const throw() {return NLMISC::toString("LUAError: %s", what());}
	};

	// A parse error occurred
	class ELuaParseError : public ELuaError
	{
	public:
		ELuaParseError() {}
		ELuaParseError(const std::string &reason) : ELuaError(reason) {}
		virtual ~ELuaParseError() NL_OVERRIDE { }
		// what(), plus append the Reason
		virtual std::string luaWhat() const throw() NL_OVERRIDE {return NLMISC::toString("ELuaParseError: %s", what());}
	};

	/** Exception thrown when something went wrong inside a wrapped function called by lua
	  */
	class ELuaWrappedFunctionException : public ELuaError
	{
	public:
		ELuaWrappedFunctionException(CLuaState *luaState);
		ELuaWrappedFunctionException(CLuaState *luaState, const std::string &reason);
		ELuaWrappedFunctionException(CLuaState *luaState, const char *format, ...);
		virtual ~ELuaWrappedFunctionException() NL_OVERRIDE { }
		virtual const char	*what() const throw() NL_OVERRIDE {return _Reason.c_str();}
	protected:
		void	init(CLuaState *ls, const std::string &reason);
	protected:
		std::string	_Reason;
	};

	// A execution error occurred
	class ELuaExecuteError : public ELuaError
	{
	public:
		ELuaExecuteError() {}
		ELuaExecuteError(const std::string &reason) : ELuaError(reason) {}
		virtual ~ELuaExecuteError() NL_OVERRIDE { }
		// what(), plus append the Reason
		virtual std::string luaWhat() const throw() NL_OVERRIDE {return NLMISC::toString("ELuaExecuteError: %s", what());}
	};

	// A bad cast occurred when using lua_checkcast
	class ELuaBadCast : public ELuaError
	{
	public:
		ELuaBadCast() {}
		ELuaBadCast(const std::string &reason) : ELuaError(reason) {}
		// what(), plus append the Reason
		virtual std::string luaWhat() const throw() {return NLMISC::toString("ELuaBadCast: %s", what());}
	};

	// Error when trying to indexate an object that is not a table
	class ELuaNotATable : public ELuaError
	{
	public:
		ELuaNotATable() {}
		ELuaNotATable(const std::string &reason) : ELuaError(reason) {}
		// what(), plus append the Reason
		virtual std::string luaWhat() const throw() {return NLMISC::toString("ELuaNotATable: %s", what());}
	};


	// ***************************************************************************
	// a function to be used with a CLuaState instance
	typedef int (* TLuaWrappedFunction) (CLuaState &ls);


	// ***************************************************************************
	/** C++ version of a lua state
	  */
	class CLuaState : public NLMISC::CRefCount
	{
	public:
		typedef NLMISC::CRefPtr<CLuaState>   TRefPtr;

		// Create a new environement
		CLuaState( bool debugger = false );
		~CLuaState();


		/// \name Registering
		// @{
		// register a wrapped function
		void				registerFunc(const char *name, TLuaWrappedFunction function);
		// @}


		/// \name Script execution
		// @{

		/** Parse a script and push as a function in top of the LUA stack
		 *	\throw ELuaParseError
		 *	\param dbgSrc is a string for debug. Should be a filename (preceded with '@'), or a short script.
		 */
		void				loadScript(const std::string &code, const std::string &dbgSrc);

		/** Execute a script from a string, possibly throwing an exception if there's a parse error
		 *	\throw ELuaParseError, ELuaExecuteError
		 */
		void				executeScript(const std::string &code, int numRet = 0);

		/** Execute a script from a string. If an errors occurs it is printed in the log
		 *	\return true if script execution was successful
		 */
		bool				executeScriptNoThrow(const std::string &code, int numRet = 0);

		/** Load a Script from a File (maybe in a BNP), and execute it
		 *	\return false if file not found
		 *	\throw ELuaParseError, ELuaExecuteError
		 */
		bool				executeFile(const std::string &pathName);

		/** execute a very Small Script (a function call for instance)
		 *	It is different from doString() in such there is a cache (where the key is the script itself)
		 *	so that the second time this script is executed, there is no parsing
		 *	Note: I experienced optim with about 10 times faster than a executeScript() on a simple "a= a+1;" script
		 *	\throw ELuaParseError, ELuaExecuteError
		 */
		void				executeSmallScript(const std::string &script);

		// @}


		/// \name Stack Manipulation
		// @{
		// stack manipulation (indices start at 1)
		void				setTop(int index);     // set new size of stack
		void				clear() { setTop(0); }
		int					getTop();
		bool				empty() { return getTop() == 0; }
		void				pushGlobalTable();		
		void				pushValue(int index);  // copie nth element of stack to the top of the stack
		void				remove(int index);     // remove nth element of stack
		void				insert(int index);     // insert last element of the stack before the given position
		void				replace(int index);    // replace nth element of the stack with the top of the stack
		void				pop(int numElem = 1);  // remove n elements from the top of the stack
		// test the type of an element in the stack
		// return one of the following values :
		// LUA_TNIL
		// LUA_TNUMBER
		// LUA_TBOOLEAN
		// LUA_TSTRING
		// LUA_TTABLE
		// LUA_TFUNCTION
		// LUA_TUSERDATA
		// LUA_TTHREAD
		// LUA_TLIGHTUSERDATA
		int					type(int index = -1);
		const char			*getTypename(int type);
		bool				isNil(int index = -1);
		bool				isBoolean(int index = -1);
		bool				isNumber(int index = -1);
		bool				isInteger(int index = -1);
		bool				isString(int index = -1);
		bool				isTable(int index = -1);
		bool				isFunction(int index = -1);
		bool				isCFunction(int index = -1);
		bool				isUserData(int index = -1);
		bool				isLightUserData(int index = -1);
		// converting then getting a value from the stack
		bool				toBoolean(int index = -1);
		lua_Number			toNumber(int index = -1);
		lua_Integer			toInteger(int index = -1);
		const char			*toString(int index = -1);
		void				toString(int index, std::string &str);		// convert to a std::string, with a NULL check.
		size_t				strlen(int index = -1);
		lua_CFunction		toCFunction(int index = -1);
		void				*toUserData(int index = -1);
		const void			*toPointer(int index = -1);
		/** Helper functions : get value of the wanted type in the top table after conversion
		* A default value is used if the stack entry is NULL.
		* If conversion fails then an exception is thrown (with optional msg)
		*/
		bool				getTableBooleanValue(const char *name, bool        defaultValue= false);
		double				getTableNumberValue(const char *name,  double      defaultValue= 0.0);
		sint64				getTableIntegerValue(const char *name,  sint64     defaultValue= 0);
		const char			*getTableStringValue(const char *name, const char *defaultValue= NULL);
		// pushing value onto the stack
		void				push(bool value);
		void				push(float value);
		void				push(double value);
		void				push(uint8 value);
		void				push(uint16 value);
		void				push(uint32 value);
		void				push(uint64 value);
		void				push(sint8 value);
		void				push(sint16 value);
		void				push(sint32 value);
		void				push(sint64 value);
		void				push(const char *str);
		void				push(const char *str, int length);
		void				push(const std::string &str);
		void				pushNil();
		void				push(lua_CFunction f);
		void                push(TLuaWrappedFunction function);
		void				pushLightUserData(void *);			// push a light user data (use newUserData to push a full userdata)
		// metatables
		bool				getMetaTable(int index = -1);
		bool				setMetaTable(int index = -1);  // set the metatable at top of stack to the object at 'index' (usually -2), then pop the metatable
														   // even if asignment failed
		// comparison
		bool				equal(int index1, int index2);
		bool				rawEqual(int index1, int index2);
		bool				lessThan(int index1, int index2);
		// concatenation of the n element at the top of the stack (using lua semantic)
		void				concat(int numElem);
		// tables
		void				newTable(); // create a new table at top of the stack
		void				getTable(int index); // get value from a table at index 'index' (key is at top)
		void				rawGet(int index);
		void				setTable(int index); // set (key, value) from top of the stack into the given table
												 // both key and value are poped
		void				rawSet(int index);
		bool				next(int index);  // table traversal
		// UserData
		void				*newUserData(uint size);
		// seting value by int index in a table
		void				rawSetI(int index, int n);
		void				rawGetI(int index, int n);
		/** Calling functions (it's up to the caller to clear the results)
		  * The function should have been pushed on the stack
		  */
		void				call(int nargs, int nresults);
		int					pcall(int nargs, int nresults, int errfunc = 0);
		/** Helper : Execute a function by name. Lookup for the function is done in the table at the index 'funcTableIndex'
		  * the behaviour is the same than with call of pcall.
		  */
		int                 pcallByNameGlobal(const char *functionName, int nargs, int nresults, int errfunc = 0);
		int                 pcallByName(const char *functionName, int nargs, int nresults, int funcTableIndex, int errfunc = 0);

		// push a C closure (pop n element from the stack and associate with the function)
		void				pushCClosure(lua_CFunction function, int n);
		// @}


		/// \name Misc
		// @{
		/** Retrieve pointer to a CLuaState environment from its lua_State pointer, or NULL
		  * if there no such environment
		  */
		static CLuaState	*fromStatePointer(lua_State *state);
		// Get state pointer. The state should not be closed (this object has ownership)
		lua_State			*getStatePointer() const {return _State;}
		// check that an index is valid when accessing the stack
		// an assertion is raised if the index is not valid
		void				checkIndex(int index);

		// registering C function to use with a lua state pointer
		void				registerFunc(const char *name, lua_CFunction function);

		// Garbage collector
		int					getGCCount();      // get memory in use in KB
		int					getGCThreshold();  // get max memory in KB
		void				setGCThreshold(int kb);   // set max memory in KB (no-op with ref-counted version)

		// handle garbage collector for ref-counted version of lua (no-op with standard version, in which case gc handling is automatic)
		void				handleGC();

		/** For Debug: get the Stack context of execution (filename / line)
		 *	\param stackLevel: get the context of execution of the given stackLevel.
		 *		0 for the current function
		 *		1 for the function that called 0
		 *		2 ....
		 *	NB: if called from a C function called from LUA, remember that stackLevel 0 is the current function.
		 *		Hence if you want to know what LUA context called you, pass stackLevel=1!
		 *	\param ret string cleared if any error, else filled with formated FileName / LineNumber
		 */
		void				getStackContext(std::string &ret, uint stackLevel);
		// @}

		// for debug : dump the current content of the stack (no recursion)
		void				dumpStack();
		static void			dumpStack(lua_State *ls);
		void				getStackAsString(std::string &dest);


	private:
		lua_State			*_State;

		#ifdef LUA_NEVRAX_VERSION
			int				 _GCThreshold; // if refcounted gc is used, then garbage collector is handled manually
		#endif
		// Small Script Cache
		uint						_SmallScriptPool;
		typedef std::map<std::string, uint>	TSmallScriptCache;
		TSmallScriptCache			_SmallScriptCache;
		static const char *			_NELSmallScriptTableName;

#ifdef _WIN32
		HMODULE m_LuaSocket;
#endif

	private:
		// this object isn't intended to be copied
		CLuaState(const CLuaState &/* other */):NLMISC::CRefCount() { nlassert(0); }
		CLuaState &operator=(const CLuaState &/* other */) { nlassert(0); return *this; }

		void				executeScriptInternal(const std::string &code, const std::string &dbgSrc, int numRet = 0);
		int                 pcallByNameInternal(const char *functionName, int nargs, int nresults, int errfunc, int initialStackSize);

	};


	// Access to lua function
	// one should not include lua.h directly because if a debugger is present, lua
	// function pointer will be taken from a dynamic library.



	//=============================================================================================
	// include implementation
	#define RZ_INCLUDE_LUA_HELPER_INLINE
		#include "lua_helper_inline.h"
	#undef RZ_INCLUDE_LUA_HELPER_INLINE

}

#endif
