// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_LUA_IHM_H
#define NL_LUA_IHM_H

#include "nel/misc/types_nl.h"
#include "nel/gui/lua_helper.h"
#include "nel/gui/interface_element.h"

#define IHM_LUA_METATABLE		"__ui_metatable"
#define IHM_LUA_ENVTABLE		"__ui_envtable"

namespace NLMISC
{
	class CPolygon2D;
	class CVector2f;
	class CRGBA;
	class CLog;
}

namespace NLGUI
{

	class CReflectable;
	class CReflectedProperty;

#ifdef RYZOM_LUA_UCSTRING
	/** Text of a Lua 'ucstring' object, for functions bound through luabind (see lua_ihm_ucstring.h).
	  * Scripts written when ucstring held UTF-16 text still create, compare and modify these objects.
	  * They hold UTF-8 now, so passing text between those scripts and the interface converts nothing.
	  */
	struct CLuaUCString
	{
		CLuaUCString() { }
		CLuaUCString(const std::string &utf8) : Utf8(utf8) { }
		bool empty() const { return Utf8.empty(); }
		std::string Utf8;
	};
#endif

	// ***************************************************************************
	/* 	Use this Exception for all LUA Error (eg: scripted passes bad number of paramters).
	 *	Does not herit from Exception because avoid nlinfo,    because sent twice (catch then resent)
	 *	This is special to lua and IHM since it works with CLuaStackChecker,    and also append to the error msg
	 *	the FileName/LineNumber
	 */
	class ELuaIHMException : public ELuaWrappedFunctionException
	{
	private:
		static CLuaState *getLuaState();
	public:
		ELuaIHMException() : ELuaWrappedFunctionException(getLuaState())
		{
		}
		ELuaIHMException(const std::string &reason) :   ELuaWrappedFunctionException(getLuaState(),    reason)
		{
		}
		ELuaIHMException(const char *format,    ...) : ELuaWrappedFunctionException(getLuaState())
		{
			std::string	reason;
			NLMISC_CONVERT_VARGS (reason,    format,    NLMISC::MaxCStringSize);
			init(getLuaState(),    reason);
		}
	};

	// ***************************************************************************
	/**
	 * Define Functions to export from C to LUA
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2004
	 */
	class CLuaIHM
	{
	public:
		static void	registerAll(CLuaState &ls);

		/** Developer trace sink of the Lua bindings.
		  * The library has no notion of a system info window or of a user setting that turns Lua tracing on, so
		  * the application installs one; without a sink the traces are dropped. These traces report conditions a
		  * UI script author wants to know about (an unknown element, a call that returns nil, a format used on the
		  * wrong kind of text); they are never errors of the library itself.
		  */
		class IDebugOutput
		{
		public:
			virtual ~IDebugOutput() {}
			/// print a trace, typically with the location of the calling Lua line
			virtual void debugInfo(const std::string &dbg) = 0;
			/// print the current Lua call stack
			virtual void dumpCallStack(int startStackLevel) = 0;
		};
		static void			setDebugOutput(IDebugOutput *output);
		static IDebugOutput	*getDebugOutput();
		/// trace through the installed sink (nothing happens without one)
		static void			debugInfo(const std::string &dbg);
		/// dump the Lua call stack through the installed sink (nothing happens without one)
		static void			dumpCallStack(int startStackLevel = 0);

		/// log that receives the output of console commands run from Lua with runCommand(), NLMISC::InfoLog by default
		static void			setCommandLog(NLMISC::CLog *log);
		static NLMISC::CLog	*getCommandLog();

		/** CReflectableInterfaceElement management on stack, stored by a CRefPtr.
		  * May be called as well for ui element, because they derive from CReflectableRefPtrTarget
		  */
		static void pushReflectableOnStack(CLuaState &ls, class CReflectableRefPtrTarget *pRPT);
		static bool	isReflectableOnStack(CLuaState &ls, sint index);
		static CReflectableRefPtrTarget	*getReflectableOnStack(CLuaState &ls, sint index);


		// ucstring, the text object of older scripts (it holds UTF-8)
#ifdef RYZOM_LUA_UCSTRING
		/// push a new ucstring object
		static void	pushUCString(CLuaState &ls, const std::string &utf8);
		static void	pushUCString(lua_State *L, const std::string &utf8);
		/// true for a ucstring object only, as the Lua function isUCString()
		static bool	isUCStringOnStack(CLuaState &ls, sint index);
		/// get the UTF-8 text of a ucstring object, a string or a number; false for any other value
		static bool getUCStringOnStack(CLuaState &ls, sint index, std::string &utf8);
		/// the text held by the ucstring object at 'index', NULL if the value is not a ucstring object
		static std::string *getUCStringObject(lua_State *L, int index);
#endif


		// RGBA
		static bool pop(CLuaState &ls, NLMISC::CRGBA &dest);

		// CVector2f
		static bool pop(CLuaState &ls, NLMISC::CVector2f &dest);

		// helper : get a 2D poly (a table of cvector2f) from a lua table (throw on fail)
		static void getPoly2DOnStack(CLuaState &ls, sint index, NLMISC::CPolygon2D &dest);

		// argument checkin helpers
		static void	checkArgCount(CLuaState &ls, const char* funcName, uint nArgs);		// check that number of argument is exactly the one required
		static void	checkArgMin(CLuaState &ls, const char* funcName, uint nArgs);		// check that number of argument is at least the one required
		static void	checkArgMax(CLuaState &ls, const char* funcName, uint nArgs);		// check that number of argument is at most the one required
		static void	check(CLuaState &ls, bool ok, const std::string &failReason);
		static void	checkArgType(CLuaState &ls, const char *funcName, uint index, int argType);
		static void	checkArgTypeRGBA(CLuaState &ls, const char *funcName, uint index);
#ifdef RYZOM_LUA_UCSTRING
		static void	checkArgTypeUCString(CLuaState &ls, const char *funcName, uint index); // a ucstring object, a string or a number
#endif
		/** throw a lua expection (inside a C function called from lua) with the given reason, and the current call stack
		  * The various check... function call this function when their test fails
		  */
		static void fails(CLuaState &ls, const char *format, ...);

		// pop a sint32 from a lua stack, throw an exception on fail
		static bool popSINT32(CLuaState &ls, sint32 & dest);
		bool popString(CLuaState &ls, std::string & dest);

		/** read/write between values on a lua stack & a property exported from a 'CReflectable' derived object
		  * (throws on error)
		  */
		static void luaValueToReflectedProperty(CLuaState &ls, int stackIndex, CReflectable &target, const CReflectedProperty &property);

		// push a reflected property on the stack
		// NB : no check is done that 'property' is part of the class info of 'reflectedObject'
		static void luaValueFromReflectedProperty(CLuaState &ls, CReflectable &reflectedObject, const CReflectedProperty &property);
		
	private:
		// Functions for the ui metatable
		static class CInterfaceElement* getUIRelative( CInterfaceElement *pIE, const std::string &propName );
		static int luaUIIndex( CLuaState &ls );
		static int luaUINewIndex( CLuaState &ls );
		static int luaUIEq( CLuaState &ls );
		static int luaUINext( CLuaState &ls );
		static int luaUIDtor( CLuaState &ls );


		static void	registerBasics(CLuaState &ls);
		static void	registerIHM(CLuaState &ls);
		static void createLuaEnumTable(CLuaState &ls, const std::string &str);

	public:
		static void pushUIOnStack(CLuaState &ls, CInterfaceElement *pIE);
		static bool	isUIOnStack(CLuaState &ls, sint index);
		static CInterfaceElement	*getUIOnStack(CLuaState &ls, sint index);
		static void	checkArgTypeUIElement(CLuaState &ls, const char *funcName, uint index);

	private:

		//////////////////////////////////////////// Exported functions //////////////////////////////////////////////////////

		static uint32		getLocalTime();
		static double		getPreciseLocalTime();
		static std::string	findReplaceAll(const std::string &str, const std::string &search, const std::string &replace);
#ifdef RYZOM_LUA_UCSTRING
		static CLuaUCString	findReplaceAll(const CLuaUCString &str, const CLuaUCString &search, const CLuaUCString &replace);
#endif

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		static int luaMethodCall(lua_State *ls);
		
		static int	setOnDraw(CLuaState &ls);		// params: CInterfaceGroup*, "script". return: none
		static int	getOnDraw(CLuaState &ls);		// params: CInterfaceGroup*. return: "script" (nil if none)
		static int	addOnDbChange(CLuaState &ls);	// params: CInterfaceGroup*, "dblist", "script". return: none
		static int	removeOnDbChange(CLuaState &ls);// params: CInterfaceGroup*. return: none
		static int  setCaptureKeyboard(CLuaState &ls);
		static int  resetCaptureKeyboard(CLuaState &ls);
		static int	getUIId(CLuaState &ls);			// params: CInterfaceElement*. return: ui id (empty if error)
		static int	runAH(CLuaState &ls);			// params: CInterfaceElement *, "ah", "params". return: none
		static int  getWindowSize(CLuaState &ls);
		static int	setTopWindow(CLuaState &ls); // set the top window
		static int  getTextureSize(CLuaState &ls);
		static int	disableModalWindow(CLuaState &ls);
		static int	deleteUI(CLuaState &ls);		// params: CInterfaceElement*.... return: none
		static int	deleteReflectable(CLuaState &ls);		// params: CInterfaceElement*.... return: none
		static int	getCurrentWindowUnder(CLuaState &ls);		// params: none. return: CInterfaceElement*  (nil if none)
		static bool	fileExists(const std::string &fileName);
		static int	runExprAndPushResult(CLuaState &ls, const std::string &expr);		// Used by runExpr and runFct
		static int	runExpr(CLuaState &ls);			// params: "expr". return: any of: nil,bool,string,number, RGBA, UCString
		static int	runFct(CLuaState &ls);			// params: "expr", param1, param2.... return: any of: nil,bool,string,number, RGBA, UCString
		static int  runCommand(CLuaState &ls);      // params: "command name", param1, param2 ... return true or false
#ifdef RYZOM_LUA_UCSTRING
		static int  isUCString(CLuaState &ls);
		static int	concatUCString(CLuaState &ls); // workaround for + operator that don't work in luabind for ucstrings ...
#endif
		static int	concatString(CLuaState &ls); // speedup concatenation of several strings
		static int	tableToString(CLuaState &ls); // concat element of a table to build a string
		static int	getPathContent(CLuaState &ls);
	};

}

#endif // NL_LUA_IHM_H

/* End of lua_ihm.h */
