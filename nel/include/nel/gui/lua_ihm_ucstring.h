// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_LUA_IHM_UCSTRING_H
#define NL_LUA_IHM_UCSTRING_H

// luabind conversion of CLuaUCString, include after <luabind/luabind.hpp>

#ifdef RYZOM_LUA_UCSTRING

#include "nel/gui/lua_ihm.h"

namespace luabind
{
	/** A CLuaUCString parameter takes a ucstring object, or a string or a number as UTF-8 text.
	  * Strings and numbers score 1, so an overload taking std::string wins for them.
	  * A CLuaUCString result reaches Lua as a new ucstring object.
	  */
	template <>
	struct default_converter<NLGUI::CLuaUCString>
		: native_converter_base<NLGUI::CLuaUCString>
	{
		static int compute_score(lua_State *L, int index)
		{
			if (NLGUI::CLuaIHM::getUCStringObject(L, index)) return 0;
			int type = lua_type(L, index);
			return (type == LUA_TSTRING || type == LUA_TNUMBER) ? 1 : -1;
		}

		NLGUI::CLuaUCString from(lua_State *L, int index)
		{
			const std::string *str = NLGUI::CLuaIHM::getUCStringObject(L, index);
			if (str) return NLGUI::CLuaUCString(*str);
			size_t len;
			const char *s = lua_tolstring(L, index, &len);
			return NLGUI::CLuaUCString(std::string(s, len));
		}

		void to(lua_State *L, const NLGUI::CLuaUCString &value)
		{
			NLGUI::CLuaIHM::pushUCString(L, value.Utf8);
		}
	};

	template <>
	struct default_converter<const NLGUI::CLuaUCString>
		: default_converter<NLGUI::CLuaUCString>
	{
	};

	template <>
	struct default_converter<const NLGUI::CLuaUCString &>
		: default_converter<NLGUI::CLuaUCString>
	{
	};
}

#endif // RYZOM_LUA_UCSTRING

#endif // NL_LUA_IHM_UCSTRING_H

/* End of lua_ihm_ucstring.h */
