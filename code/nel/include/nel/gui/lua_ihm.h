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

#ifndef NL_LUA_IHM_H
#define NL_LUA_IHM_H

#include "nel/misc/types_nl.h"
#include "nel/gui/lua_helper.h"

#define IHM_LUA_METATABLE		"__ui_metatable"
#define IHM_LUA_ENVTABLE		"__ui_envtable"

namespace NLMISC
{
	class CPolygon2D;
	class CVector2f;
	class CRGBA;
}

namespace NLGUI
{

	class CReflectable;
	class CReflectedProperty;

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

		/** CReflectableInterfaceElement management on stack, stored by a CRefPtr.
		  * May be called as well for ui element, because they derive from CReflectableRefPtrTarget
		  */
		static void pushReflectableOnStack(CLuaState &ls, class CReflectableRefPtrTarget *pRPT);
		static bool	isReflectableOnStack(CLuaState &ls, sint index);
		static CReflectableRefPtrTarget	*getReflectableOnStack(CLuaState &ls, sint index);


		// ucstring
		static bool pop(CLuaState &ls, ucstring &dest);
		static void push(CLuaState &ls, const ucstring &value);
		static bool	isUCStringOnStack(CLuaState &ls, sint index);
		static bool getUCStringOnStack(CLuaState &ls, sint index, ucstring &dest);


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
		static void	checkArgTypeUCString(CLuaState &ls, const char *funcName, uint index);
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
		static void luaValueToReflectedProperty(CLuaState &ls, int stackIndex, CReflectable &target, const CReflectedProperty &property) throw(ELuaIHMException);

		// push a reflected property on the stack
		// NB : no check is done that 'property' is part of the class info of 'reflectedObject'
		static void luaValueFromReflectedProperty(CLuaState &ls, CReflectable &reflectedObject, const CReflectedProperty &property);



	private:
		static void	registerBasics(CLuaState &ls);
		static void	registerIHM(CLuaState &ls);
		static void createLuaEnumTable(CLuaState &ls, const std::string &str);

		//////////////////////////////////////////// Exported functions //////////////////////////////////////////////////////

		static uint32		getLocalTime();
		static double		getPreciseLocalTime();
		static std::string	findReplaceAll(const std::string &str, const std::string &search, const std::string &replace);
		static ucstring		findReplaceAll(const ucstring &str, const ucstring &search, const ucstring &replace);
		static ucstring		findReplaceAll(const ucstring &str, const std::string &search, const std::string &replace);
		static ucstring		findReplaceAll(const ucstring &str, const std::string &search, const ucstring &replace);
		static ucstring		findReplaceAll(const ucstring &str, const ucstring &search, const std::string &replace);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		static int luaMethodCall(lua_State *ls);
	};

}

#endif // NL_LUA_IHM_H

/* End of lua_ihm.h */
