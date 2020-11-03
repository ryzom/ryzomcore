// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/interface_group.h"

#include <algorithm>

// to get rid of you_must_not_use_assert___use_nl_assert___read_debug_h_file messages
#include <cassert>
#ifdef assert
	#undef assert
#endif

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

// Warning: cannot use namespace std,    when using luabind
#ifdef NL_OS_WINDOWS
#  ifndef NL_EXTENDED_FOR_SCOPE
#    undef for
#  endif
#endif

#ifdef NL_DEBUG
#	define assert(x) nlassert(x)
#else
#	define assert(x)
#endif

#include <luabind/luabind.hpp>
// in luabind > 0.6, LUABIND_MAX_ARITY is set to 10
#if LUABIND_MAX_ARITY == 10
#	include <luabind/operator.hpp>
// only luabind > 0.7 have version.hpp (file checked with build system)
#	ifdef HAVE_LUABIND_VERSION
#		include <luabind/version.hpp>
#	endif
#	ifndef LUABIND_VERSION
// luabind 0.7 doesn't define LUABIND_VERSION
#		define LUABIND_VERSION 700
#	endif
// luabind 0.6 doesn't define LUABIND_VERSION but LUABIND_MAX_ARITY is set to 5
#elif LUABIND_MAX_ARITY == 5
#	define LUABIND_VERSION 600
#else
#	pragma error("luabind version not recognized")
#endif


#include "nel/gui/lua_ihm.h"
#include "nel/gui/reflect.h"
#include "nel/misc/algo.h"
#include "nel/misc/file.h"
#include "nel/misc/i18n.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/sstring.h"
#include "nel/misc/command.h"
#include "nel/gui/lua_object.h"
#include "nel/misc/polygon.h"
#include "nel/gui/lua_manager.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/interface_expr.h"
#include "nel/misc/debug.h"

// ***************************************************************************
/*
IMPORTANT NOTE: we do this heavy double registration in this file because we DON'T want
to include luabind.hpp in every file.
Compilation is VERY SLOW
*/
// ***************************************************************************

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#ifdef RYZOM_LUA_UCSTRING
// declare ostream << operator for ucstring -> registration of ucstring iin luabind will build a 'tostring' function from it
std::ostream &operator<<(std::ostream &str, const ucstring &value)
{
	return str << value.toString();
}
#endif

namespace NLGUI
{

	struct CMiscFunctions
	{
		static std::string fileLookup(const std::string &fileName)
		{
			return NLMISC::CPath::lookup(fileName,   false);
		}
		static void shellExecute(const std::string &operation, const std::string &fileName, const std::string &parameters)
		{
		#if !FINAL_VERSION
			#ifdef NL_OS_WINDOWS
				ShellExecuteW(NULL, nlUtf8ToWide(operation), nlUtf8ToWide(fileName), nlUtf8ToWide(parameters), NULL, SW_SHOWDEFAULT);
			#endif
		#endif
		}
	};


	// ***************************************************************************
	bool CLuaIHM::pop(CLuaState &ls,   NLMISC::CRGBA &dest)
	{
		//H_AUTO(Lua_CLuaIHM_pop)
		try
		{
			if (ls.isNil(-1)) return false;
	#if LUABIND_VERSION > 600
			luabind::object obj(luabind::from_stack(ls.getStatePointer(), -1));
			ls.pop();
	#else
			luabind::object obj(ls.getStatePointer());
			obj.set();
	#endif
			dest = luabind::object_cast<NLMISC::CRGBA>(obj);
		}
		catch(const luabind::cast_failed &)
		{
			return false;
		}
		return true;
	}

	// ***************************************************************************
	bool CLuaIHM::pop(CLuaState &ls,NLMISC::CVector2f &dest)
	{
		//H_AUTO(Lua_CLuaIHM_pop)
		try
		{
			if (ls.isNil(-1)) return false;
	#if LUABIND_VERSION > 600
			luabind::object obj(luabind::from_stack(ls.getStatePointer(), -1));
			ls.pop();
	#else
			luabind::object obj(ls.getStatePointer());
			obj.set();
	#endif
			dest = luabind::object_cast<NLMISC::CVector2f>(obj);
		}
		catch(const luabind::cast_failed &)
		{
			return false;
		}
		return true;
	}

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	bool CLuaIHM::pop(CLuaState &ls,   ucstring &dest)
	{
		//H_AUTO(Lua_CLuaIHM_pop)
		try
		{
			if (ls.isNil(-1)) return false;
	#if LUABIND_VERSION > 600
			luabind::object obj(luabind::from_stack(ls.getStatePointer(), -1));
			ls.pop();
	#else
			luabind::object obj(ls.getStatePointer());
			obj.set();
	#endif
			dest = luabind::object_cast<ucstring>(obj);
		}
		catch(const luabind::cast_failed &)
		{
			return false;
		}
		return true;
	}

	// ***************************************************************************
	bool CLuaIHM::isUCStringOnStack(CLuaState &ls, sint index)
	{
		//H_AUTO(Lua_CLuaIHM_isUCStringOnStack)
		ls.pushValue(index);
		ucstring dummy;
		return pop(ls, dummy);
	}

	// ***************************************************************************
	bool CLuaIHM::getUCStringOnStack(CLuaState &ls, sint index, ucstring &dest)
	{
		//H_AUTO(Lua_CLuaIHM_getUCStringOnStack)
		ls.pushValue(index);
		return pop(ls, dest);
	}


	// ***************************************************************************
	void CLuaIHM::push(CLuaState &ls, const ucstring &value)
	{
		//H_AUTO(Lua_CLuaIHM_push)
	#if LUABIND_VERSION > 600
		luabind::detail::push(ls.getStatePointer(), value);
	#else
		luabind::object obj(ls.getStatePointer(), value);
		obj.pushvalue();
	#endif
	}
#endif

	// ***************************************************************************
	// ***************************************************************************
	// CInterface To LUA Registry
	// ***************************************************************************
	// ***************************************************************************


	CLuaState * ELuaIHMException::getLuaState()
	{
		return CLuaManager::getInstance().getLuaState();
	}

}

	// ***************************************************************************
	#define LUA_REGISTER_BASIC(_type_)															\
	luabind::detail::yes_t is_user_defined(luabind::detail::by_value<_type_>);					\
	_type_ convert_lua_to_cpp(lua_State* L, luabind::detail::by_value<_type_>, int index)		\
	{																							\
		return (_type_)lua_tointeger(L, index);													\
	}																							\
	int match_lua_to_cpp(lua_State* L, luabind::detail::by_value<_type_>, int index)			\
	{																							\
		return lua_isnumber(L, index) ? 0:-1;													\
	}																							\
	void convert_cpp_to_lua(lua_State* L, const  _type_& v)										\
	{																							\
		lua_pushinteger(L, (lua_Integer)v);														\
	}

	// Basic LUA types
	namespace luabind
	{
		namespace converters
		{
			LUA_REGISTER_BASIC(sint8)
			LUA_REGISTER_BASIC(uint8)
			LUA_REGISTER_BASIC(sint16)
			LUA_REGISTER_BASIC(uint16)
			LUA_REGISTER_BASIC(sint32)
			LUA_REGISTER_BASIC(uint32)
		}
	}

namespace NLGUI
{
	static CLuaString lstr_Env("Env");
	static CLuaString lstr_isNil("isNil");

	// ***************************************************************************
	int CLuaIHM::luaUIIndex(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_luaUIIndex)
		nlassert(ls.getTop()==2);
		// get the userdata and key
		CReflectableLuaRef *pRefElm = (CReflectableLuaRef *) ls.toUserData(1);

		const char *propName = ls.toString(2);
		CReflectableRefPtrTarget	*pRPT= (CReflectableRefPtrTarget*)(pRefElm->Ptr);
		// ** try to get the Env Table (interface group only)
		if(propName==lstr_isNil)
		{
			ls.push(pRPT==NULL);
			return 1;
		}

		// Check the object is not NULL or freed
		if(pRPT==NULL)
		{
			return 0;
		}

		// ** try to get the Env Table (interface group only)
		if(propName==lstr_Env)
		{
			// Env can be bound to a CInterfaceGroup only
			CInterfaceGroup		*group= dynamic_cast<CInterfaceGroup*>(pRPT);
			if(group==NULL)
			{
				ls.pushNil();
				return 1;
			}
			else
			{
				group->pushLUAEnvTable();
				return 1;
			}
		}

		// ** try to get the property
		const CReflectedProperty *prop = pRefElm->getProp(propName);
		if (prop)
		{
			CLuaIHM::luaValueFromReflectedProperty(ls, *pRPT, *prop);
			return 1;
		}

		// ** try to get a UI relative
		CInterfaceElement	*uiRelative= getUIRelative(dynamic_cast<CInterfaceElement *>(pRPT),    propName);
		if(uiRelative)
		{
			// push the UI onto the stack
			pushUIOnStack(ls,    uiRelative);
			return 1;
		}


		// Fail to find any Attributes or elements
		// Yoyo: don't write any message or warning because this may be a feature (if user want to test that something exit in the ui)
		ls.pushNil();
		return 1;
	}

	// ***************************************************************************
	int CLuaIHM::luaUINewIndex(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_luaUINewIndex)
		nlassert(ls.getTop()==3);
		// get the userdata and key
		CReflectableLuaRef	*pRefElm = (CReflectableLuaRef *) ls.toUserData(1);
		nlassert(pRefElm);
		CReflectableRefPtrTarget	*pRPT= (CReflectableRefPtrTarget*)(pRefElm->Ptr);
		// Check the UI is not NULL or freed
		if(pRPT == NULL)
		{
			return 0;
		}

		const char *propName = ls.toString(2);
		// ** try to set the Env Table (interface group only)
		if(propName == lstr_Env)
		{
			CInterfaceElement *pIE = dynamic_cast<CInterfaceElement *>(pRPT);
			std::string name ;
			if (pIE)
			{
				name = pIE->getId();
			}
			else
			{
				name = "<reflectable element>";
			}
			// Exception!!! not allowed
			throw ELuaIHMException("You cannot change the Env Table of '%s'",    name.c_str());
		}


		// ** try to set the property
		const CReflectedProperty *prop = pRefElm->getProp(propName);
		if (prop)
		{
			CLuaIHM::luaValueToReflectedProperty(ls, 3, *pRPT, *prop);
			return 0;
		}

		CInterfaceElement	*pIE = dynamic_cast<CInterfaceElement *>(pRPT);
		// ** try to get another UI (child or parent)
		CInterfaceElement	*uiRelative= getUIRelative(pIE,    propName);
		if(uiRelative)
		{
			// Exception!!! not allowed
			throw ELuaIHMException("You cannot write into the UI '%s' of '%s'",    propName,    pIE->getId().c_str());
		}

		// ** Prop Not Found
		throw ELuaIHMException("Property '%s' not found in '%s' of type %s",    propName,    pIE ? pIE->getId().c_str() : "<reflectable element>", typeid(*pRPT).name());

		// Fail to find any Attributes or elements
		return 0;
	}

	// ***************************************************************************
	int CLuaIHM::luaUIEq(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_luaUIEq)
		nlassert(ls.getTop() == 2);
		// read lhs & rhs
		// get the userdata and key
		CReflectableLuaRef	*lhs = (CReflectableLuaRef *) ls.toUserData(1);
		CReflectableLuaRef	*rhs = (CReflectableLuaRef *) ls.toUserData(2);
		nlassert(lhs);
		nlassert(rhs);
		ls.push(lhs->Ptr == rhs->Ptr);
		return 1;
	}


	// ***************************************************************************
	int CLuaIHM::luaUIDtor(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_luaUIDtor)
		nlassert(ls.getTop()==1);
		// get the userdata
		CReflectableLuaRef	*pRefElm = (CReflectableLuaRef *) ls.toUserData(1);
		nlassert(pRefElm);

		// call dtor
		pRefElm->~CReflectableLuaRef();

		return 0;
	}

	// ***************************************************************************
	int CLuaIHM::luaUINext(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_luaUINext)
		// Code below allow enumeration of properties of a reflectable object
		// From lua standpoint, the object is seen as a table with (key, value) pairs
		// If object is a CInterfaceGroup, iteration is also done on sons (groups, controls & view).

		if (ls.getTop() != 2)
		{
			CLuaIHM::fails(ls, "__next metamethod require 2 arguments (table & key)");
		}
		CLuaIHM::check(ls, CLuaIHM::isReflectableOnStack(ls, 1), "__next :  require ui element as first arg");
		CReflectableRefPtrTarget *reflectedObject = CLuaIHM::getReflectableOnStack(ls, 1);
		// To traverse all properties / field of the object, we must be able to determine the next key from a previous key
		// (keys are ordered)
		// We use the 'TValueType' enum to know which kind of property we are traversing, and an index in this group of properties
		// The key which uniquely identify an element / property in the reflectable object
		struct CKey
		{
			enum TValueType
			{
				VTGroup = 0, // children groups    (If the object is a CInterfaceGroup)
				VTView,      // children views	   (If the object is a CInterfaceView)
				VTCtrl, 	 // children controls  (If the object is a CInterfaceCtrl)
				VTProp       // List of exported proeprties (For all relfectable objects)
			};
			TValueType		  ValueType;
			sint			  Index;
			const CClassInfo  *ClassInfo; // if ValueType is "VTProp" -> give the class for which property are currently enumerated
			//
			static int tostring(CLuaState &ls) // '__print' metamathod
			{
				CLuaIHM::checkArgCount(ls, "reflected object metatable:__print", 1);
				CKey key;
				key.pop(ls);
				switch(key.ValueType)
				{
					case VTGroup: ls.push(toString("_Group %d", key.Index)); break;
					case VTView:  ls.push(toString("_View %d", key.Index)); break;
					case VTCtrl:  ls.push(toString("_Ctrl %d", key.Index)); break;
					case VTProp:  ls.push(key.ClassInfo->Properties[key.Index].Name); break;
				}
				return 1;
			}
			// push the key on the lua stack
			void push(CLuaState &ls)
			{
				void *ud = ls.newUserData(sizeof(*this));
				*(CKey *) ud = *this;
				getMetaTable(ls).push();
				ls.setMetaTable(-2);
			}
			// pop the key from the lua stack
			void pop(CLuaState &ls)
			{
				CLuaStackChecker lsc(&ls, -1);
				if (!ls.isUserData(-1))
				{
					CLuaIHM::fails(ls, "Can't pop object, not a user data");
				}
				// check that metatable is good (it is share between all keys)
				ls.getMetaTable(-1);
				getMetaTable(ls).push();
				if (!ls.rawEqual(-1, -2))
				{
					CLuaIHM::fails(ls, "Bad metatable for reflectable object key");
				}
				ls.pop(2);
				// retrieve key
				*this = *(CKey *) ls.toUserData(-1);
				ls.pop();
			}
			// get the metatable for a CKey
			CLuaObject &getMetaTable(CLuaState &ls)
			{
				static CLuaObject metatable;
				if (!metatable.isValid())
				{
					// first build
					CLuaStackChecker lsc(&ls);
					ls.newTable();
					ls.push("__tostring");
					ls.push(CKey::tostring);
					ls.setTable(-3);
					metatable.pop(ls);
				}
				return metatable;
			}
		};
		// Pop the current key to continue enumeration
		CKey key;
		if (ls.isNil(2))
		{
			// no key -> start of table
			key.ValueType = CKey::VTGroup;
			key.Index = -1;
		}
		else
		{
			key.pop(ls);
		}
		//
		CInterfaceGroup *group = dynamic_cast<CInterfaceGroup *>(reflectedObject);
		bool enumerate = true;
		while (enumerate)
		{
			switch(key.ValueType)
			{
				case CKey::VTGroup:
					if (!group || (key.Index + 1) == (sint) group->getGroups().size())
					{
						key.Index     = -1;
						key.ValueType = CKey::VTView; // continue enumeration with views
					}
					else
					{
						++ key.Index;
						key.push(ls);
						pushUIOnStack(ls, group->getGroups()[key.Index]);
						return 2;
					}
				break;
				case CKey::VTView:
					if (!group || (key.Index + 1) == (sint) group->getViews().size())
					{
						key.Index     = -1;
						key.ValueType = CKey::VTCtrl; // continue enumeration with controls
					}
					else
					{
						++ key.Index;
						key.push(ls);
						pushUIOnStack(ls, group->getViews()[key.Index]);
						return 2;
					}
				break;
				case CKey::VTCtrl:
					if (!group || (key.Index + 1) == (sint) group->getControls().size())
					{
						key.Index     = -1;
						key.ValueType = CKey::VTProp; // continue enumeration with properties
						key.ClassInfo = reflectedObject->getClassInfo();
					}
					else
					{
						++ key.Index;
						key.push(ls);
						pushUIOnStack(ls, group->getControls()[key.Index]);
						return 2;
					}
				break;
				case CKey::VTProp:
					if (!key.ClassInfo)
					{
						enumerate = false;
						break;
					}
					if ((sint) key.ClassInfo->Properties.size() == (key.Index + 1))
					{
						key.ClassInfo = key.ClassInfo->ParentClass; // continue enumeration in parent class
						key.Index = -1;
					}
					else
					{
						++ key.Index;
						key.push(ls);
						CLuaIHM::luaValueFromReflectedProperty(ls, *reflectedObject, key.ClassInfo->Properties[key.Index]);
						return 2;
					}
				break;
				default:
					nlassert(0);
				break;
			}
		}
		ls.pushNil();
		return 0;
	}

	// ***************************************************************************
	void CLuaIHM::pushUIOnStack(CLuaState &ls, CInterfaceElement *pIE)
	{
		//H_AUTO(Lua_CLuaIHM_pushUIOnStack)
		CLuaIHM::pushReflectableOnStack(ls,    pIE);
	}

	// ***************************************************************************
	bool CLuaIHM::isUIOnStack(CLuaState &ls,    sint index)
	{
		//H_AUTO(Lua_CLuaIHM_isUIOnStack)
		return getUIOnStack(ls,    index) != NULL;
	}

	// ***************************************************************************
	CInterfaceElement *CLuaIHM::getUIOnStack(CLuaState &ls,    sint index)
	{
		//H_AUTO(Lua_CLuaIHM_getUIOnStack)
		return dynamic_cast<CInterfaceElement *>(CLuaIHM::getReflectableOnStack(ls,    index));
	}

	// ***************************************************************************
	void CLuaIHM::checkArgTypeUIElement(CLuaState &ls, const char *funcName, uint index)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgTypeUIElement)
		nlassert(index > 0);
		if (ls.getTop() < (int) index)
		{
			CLuaIHM::fails(ls, "%s : argument %d of expected type ui element was not defined",   funcName,   index);
		}
		if (!isUIOnStack(ls, index))
		{
			CLuaIHM::fails(ls, "%s : argument %d of expected type ui element has bad type : %s",   funcName,   index, ls.getTypename(ls.type(index)),   ls.type(index));
		}
	}


	// ***************************************************************************
	CInterfaceElement *CLuaIHM::getUIRelative(CInterfaceElement *pIE,    const std::string &propName)
	{
		//H_AUTO(Lua_CLuaIHM_getUIRelative)
		if (pIE == NULL) return NULL;
		// If the prop is "parent",    then return the parent of the ui
		if(propName=="parent")
		{
			return pIE->getParent();
		}
		// else try to get a child (if group/exist)
		else
		{
			CInterfaceGroup		*group= dynamic_cast<CInterfaceGroup*>(pIE);
			if(group)
			{
				return group->getElement(group->getId()+":"+propName);
			}
		}

		return NULL;
	}


	// ***************************************************************************
	void	CLuaIHM::registerBasics(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_registerBasics)
		using namespace luabind;
		lua_State	*L= ls.getStatePointer();

		// RGBA
		module(L)
		[
			class_<NLMISC::CRGBA>("CRGBA")
				.def(constructor<>())
				.def(constructor<const NLMISC::CRGBA &>())
				.def(constructor<uint8,    uint8,    uint8>())
				.def(constructor<uint8,    uint8,    uint8,    uint8>())
				.def_readwrite("R",    &NLMISC::CRGBA::R)
				.def_readwrite("G",    &NLMISC::CRGBA::G)
				.def_readwrite("B",    &NLMISC::CRGBA::B)
				.def_readwrite("A",    &NLMISC::CRGBA::A)
		];

#ifdef RYZOM_LUA_UCSTRING
		// ucstring
		module(L)
		[
			class_<ucstring>("ucstring")
				.def(constructor<>())
				.def(constructor<const ucstring &>())
				.def(constructor<const std::string &>())
				.def(const_self + other<const std::string>())
				.def(other<const std::string>() + const_self)
				// NB nico : luabind crash not solved here -> use concatUCString as a replacement
	//			.def(const_self + other<const ucstring &>())
				.def(const_self < other<const ucstring&>())
				.def(const_self == other<const ucstring&>())
				.def("toUtf8",   &ucstring::toUtf8)
				.def("fromUtf8",   &ucstring::fromUtf8)
				.def("substr",  &ucstring::luabind_substr)
				.def(luabind::tostring(const_self)) // __string metamethod
				.def("toString",   (std::string(ucstring::*)()const)&ucstring::toString)
				//.def(self + other<ucstring>())
		];
#endif

		// CVector2f
		module(L)
		[
			class_<NLMISC::CVector2f>("CVector2f")
				.def(constructor<float ,float>())
				.def_readwrite("x",    &NLMISC::CVector2f::x)
				.def_readwrite("y",    &NLMISC::CVector2f::y)
		];

	}


	// ***************************************************************************
	int CLuaIHM::luaMethodCall(lua_State *ls)
	{
		//H_AUTO(Lua_CLuaIHM_luaMethodCall)
		nlassert(ls);
		const CReflectedProperty *prop   = (const CReflectedProperty *) lua_touserdata(ls,      lua_upvalueindex(1));
		CLuaState           *state = (CLuaState *) lua_touserdata(ls,      lua_upvalueindex(2));
		nlassert(prop);
		nlassert(prop->Type == CReflectedProperty::LuaMethod);
		nlassert(state);
		if (state->empty())
		{
			state->push(NLMISC::toString("Error while calling lua method %s:%s : no 'self' reference provided,    did you you function call '.' instead of method call ':' ?",
									prop->ParentClass->ClassName.c_str(),    prop->Name.c_str())
								   );
			lua_error(ls);
		}
		// because this is a method,    first parameter is the 'this'
		CReflectableRefPtrTarget *pRPT = getReflectableOnStack(*state,    1);
		if (!pRPT)
		{
			state->push(NLMISC::toString("Error while calling lua method %s:%s : 'self' pointer is nil or of bad type,    can't make the call.",
									prop->ParentClass->ClassName.c_str(),    prop->Name.c_str())
								   );
			lua_error(ls);
		}
		//
		state->remove(1); // remove 'self' reference from parameters stack
		//
		sint numResults = 0;
		if (pRPT)
		{
			sint initialStackSize = state->getTop();
			try
			{
				// call the actual method
				numResults = (pRPT->*(prop->GetMethod.GetLuaMethod))(*state);
			}
			catch (const std::exception & e)
			{
				// restore stack to its initial size
				state->setTop(initialStackSize);
				lua_pushstring(ls, e.what());
				// TODO : see if this is safe to call lua error there" ... (it does a long jump)
				lua_error(ls);
			}
		}
		return numResults;
	}



	// ***************************************************************************
	int	CLuaIHM::setOnDraw(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_setOnDraw)
		CLuaStackChecker lsc(&ls,    0);

		// params: CInterfaceGroup*,    "script".
		// return: none
		CLuaIHM::checkArgCount(ls,    "setOnDraw",    2);
		CLuaIHM::check(ls,   CLuaIHM::isUIOnStack(ls,    1),    "setOnDraw() requires a UI object in param 1");
		CLuaIHM::check(ls,   ls.isString(2),    "setOnDraw() requires a string in param 2");

		// retrieve args
		CInterfaceElement	*pIE= CLuaIHM::getUIOnStack(ls,    1);
		std::string			script;
		ls.toString(2,    script);
		nlassert(pIE);

		// must be a group
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
		if(!group)
			throw ELuaIHMException("setOnDraw(): '%s' is not a group",    pIE->getId().c_str());
		// Set the script to be executed at each draw
		group->setLuaScriptOnDraw(script);

		return 0;
	}

	// ***************************************************************************
	int CLuaIHM::getOnDraw(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_getOnDraw
		CLuaStackChecker lsc(&ls, 1);

		// params: CInterfaceElement*.
		// return: "script" (nil if empty)
		CLuaIHM::checkArgCount(ls, "getOnDraw", 1);
		CLuaIHM::check(ls, CLuaIHM::isUIOnStack(ls, 1), "getOnDraw() requires a UI object in param 1");

		// retrieve arguments
		CInterfaceElement *pIE = CLuaIHM::getUIOnStack(ls, 1);
		if (pIE)
		{
			// must be a group
			CInterfaceGroup *group = dynamic_cast<CInterfaceGroup*>(pIE);
			if (group)
			{
				if (!group->getLuaScriptOnDraw().empty()) {
					ls.push(group->getLuaScriptOnDraw());
					return 1;
				}
			}
		}
		ls.pushNil();
		return 1;
	}

	// ***************************************************************************
	int	CLuaIHM::addOnDbChange(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_addOnDbChange)
		CLuaStackChecker lsc(&ls,    0);

		// params: CInterfaceGroup*,    "dblist",    "script".
		// return: none
		CLuaIHM::checkArgCount(ls,    "addOnDbChange",    3);
		CLuaIHM::check(ls,   CLuaIHM::isUIOnStack(ls,    1),    "addOnDbChange() requires a UI object in param 1");
		CLuaIHM::check(ls,   ls.isString(2),    "addOnDbChange() requires a string in param 2");
		CLuaIHM::check(ls,   ls.isString(3),    "addOnDbChange() requires a string in param 3");

		// retrieve args
		CInterfaceElement	*pIE= CLuaIHM::getUIOnStack(ls,    1);
		std::string			dbList,    script;
		ls.toString(2,    dbList);
		ls.toString(3,    script);
		nlassert(pIE);

		// must be a group
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
		if(!group)
			throw ELuaIHMException("addOnDbChange(): '%s' is not a group",    pIE->getId().c_str());
		// Set the script to be executed when the given DB change
		group->addLuaScriptOnDBChange(dbList,    script);

		return 0;
	}


	// ***************************************************************************
	int	CLuaIHM::removeOnDbChange(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_removeOnDbChange)
		CLuaStackChecker lsc(&ls,    0);

		// params: CInterfaceGroup*,    "dbList"
		// return: none
		CLuaIHM::checkArgCount(ls,    "removeOnDbChange",    2);
		CLuaIHM::check(ls,   CLuaIHM::isUIOnStack(ls,    1),    "removeOnDbChange() requires a UI object in param 1");
		CLuaIHM::check(ls,   ls.isString(2),    "removeOnDbChange() requires a string in param 2");

		// retrieve args
		CInterfaceElement	*pIE= CLuaIHM::getUIOnStack(ls,    1);
		std::string			dbList;
		ls.toString(2,    dbList);
		nlassert(pIE);

		// must be a group
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
		if(!group)
			throw ELuaIHMException("removeOnDbChange(): '%s' is not a group",    pIE->getId().c_str());
		// Remove the script to be executed when the given DB change
		group->removeLuaScriptOnDBChange(dbList);

		return 0;
	}



	// ***************************************************************************
	int CLuaIHM::setCaptureKeyboard(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_setCaptureKeyboard)
		const char *funcName = "setCaptureKeyboard";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgTypeUIElement(ls, funcName, 1);
		CCtrlBase *ctrl = dynamic_cast<CCtrlBase *>( CLuaIHM::getUIOnStack(ls, 1));
		if (!ctrl)
		{
			CLuaIHM::fails(ls, "%s waits a ui control as arg 1", funcName);
		}
		CWidgetManager::getInstance()->setCaptureKeyboard(ctrl);
		return 0;
	}

	// ***************************************************************************
	int CLuaIHM::resetCaptureKeyboard(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_resetCaptureKeyboard)
		const char *funcName = "resetCaptureKeyboard";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		CWidgetManager::getInstance()->resetCaptureKeyboard();
		return 0;
	}

	// ***************************************************************************
	int	CLuaIHM::getUIId(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_getUIId)
		CLuaStackChecker lsc(&ls,    1);

		// params: CInterfaceElement*
		// return: "ui:interface:...". (empty if error)
		CLuaIHM::checkArgCount(ls,    "getUIId",    1);
		CLuaIHM::check(ls,   CLuaIHM::isUIOnStack(ls,   1),    "getUIId() requires a UI object in param 1");

		// retrieve args
		CInterfaceElement	*pIE= CLuaIHM::getUIOnStack(ls,    1);

		// convert to id
		if(pIE)
			ls.push(pIE->getId());
		else
			ls.push("");

		return 1;
	}



	// ***************************************************************************
	int	CLuaIHM::runAH(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_runAH)
		CLuaStackChecker lsc(&ls,    0);

		// params: CInterfaceElement *,    "ah",    "params".
		// return: none
		CLuaIHM::checkArgCount(ls,    "runAH",    3);
		CLuaIHM::check(ls,   CLuaIHM::isUIOnStack(ls,    1) || ls.isNil(1),    "runAH() requires a UI object in param 1 (or Nil)");
		CLuaIHM::check(ls,   ls.isString(2),    "runAH() requires a string in param 2");
		CLuaIHM::check(ls,   ls.isString(3),    "runAH() requires a string in param 3");

		// retrieve args
		CInterfaceElement	*pIE= CLuaIHM::getUIOnStack(ls,    1);
		std::string			ah,    params;
		ls.toString(2,    ah);
		ls.toString(3,    params);

		// run AH
		// The element must be ctrl (or NULL)
		CCtrlBase	*ctrl= NULL;
		if(pIE)
		{
			ctrl= dynamic_cast<CCtrlBase*>(pIE);
			if(!ctrl)
				throw ELuaIHMException("runAH(): '%s' is not a ctrl",    pIE->getId().c_str());
		}
		CAHManager::getInstance()->runActionHandler(ah,    ctrl,    params);

		return 0;
	}

	// ***************************************************************************
	int CLuaIHM::getWindowSize(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_getWindowSize)
		CLuaIHM::checkArgCount(ls,   "getWindowSize",   0);
		uint32 w,   h;
		CViewRenderer::getInstance()->getScreenSize(w,   h);
		ls.push(w);
		ls.push(h);
		return 2;
	}

	// ***************************************************************************
	int CLuaIHM::setTopWindow(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_setTopWindow)
		const char *funcName = "setTopWindow";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CInterfaceGroup *wnd = dynamic_cast<CInterfaceGroup *>( CLuaIHM::getUIOnStack(ls, 1));
		if (!wnd)
		{
			CLuaIHM::fails(ls, "%s : interface group expected as arg 1", funcName);
		}
		CWidgetManager::getInstance()->setTopWindow(wnd);
		return 0;
	}

	int CLuaIHM::getTextureSize(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_getTextureSize)
		const char *funcName = "getTextureSize";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		std::string textureName = ls.toString(1);

		CBitmap bitmap;
		CIFile fs(CPath::lookup(textureName).c_str());
		bitmap.load(fs);

		ls.push(bitmap.getWidth());
		ls.push(bitmap.getHeight());

		return 2;
	}



	// ***************************************************************************
	int	CLuaIHM::disableModalWindow(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_disableModalWindow)
		CLuaIHM::checkArgCount(ls, "disableModalWindow", 0);
		CWidgetManager::getInstance()->disableModalWindow();
		return 0;
	}

	// ***************************************************************************
	int	CLuaIHM::deleteUI(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_deleteUI)
		CLuaStackChecker lsc(&ls,    0);

		// params: CInterfaceElement *
		// return: none
		CLuaIHM::checkArgCount(ls,    "deleteUI",    1);
		CLuaIHM::check(ls,   CLuaIHM::isUIOnStack(ls,    1),    "deleteUI() requires a UI object in param 1");

		// retrieve args
		CInterfaceElement	*pIE= CLuaIHM::getUIOnStack(ls,    1);
		if(!pIE)
			return 0;

		// has a parent?
		CInterfaceGroup	*parent= pIE->getParent();
		if(parent)
		{
			// correctly remove from parent
			parent->delElement(pIE);
		}
		else
		{
			// just delete
			delete pIE;
		}

		return 0;
	}

	// ***************************************************************************
	int CLuaIHM::deleteReflectable(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_deleteReflectable)
		CLuaStackChecker lsc(&ls,    0);

		// params: CInterfaceElement *
		// return: none
		CLuaIHM::checkArgCount(ls,    "deleteReflectable",    1);
		CLuaIHM::check(ls,   CLuaIHM::isReflectableOnStack(ls,    1),    "deleteReflectable() requires a reflectable C++ object in param 1");

		// retrieve args
		CReflectableRefPtrTarget	*pRPT= CLuaIHM::getReflectableOnStack(ls,    1);
		if(!pRPT)
			return 0;


		CInterfaceElement *pIE = dynamic_cast<CInterfaceElement *>(pRPT);

		if (pIE)
		{
			// has a parent?
			CInterfaceGroup	*parent= pIE->getParent();
			if(parent)
			{
				// correctly remove from parent
				parent->delElement(pIE);
			}
		}

		// just delete
		delete pIE;

		return 0;
	}


	int CLuaIHM::getCurrentWindowUnder(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_getCurrentWindowUnder)
		CLuaStackChecker lsc(&ls,    1);
		CInterfaceElement	*pIE= CWidgetManager::getInstance()->getCurrentWindowUnder();
		if(!pIE)
		{
			ls.pushNil();
			nlerror("getCurrentWindowUnder(): No UICaller found. return Nil");
		}
		else
		{
			CLuaIHM::pushUIOnStack(ls,    pIE);
		}
		return 1;
	}

	// ***************************************************************************
	bool CLuaIHM::fileExists(const std::string &fileName)
	{
		//H_AUTO(Lua_CLuaIHM_fileExists)
		return CPath::exists(fileName);
	}

	// ***************************************************************************
	int	CLuaIHM::runExprAndPushResult(CLuaState &ls,    const std::string &expr)
	{
		//H_AUTO(Lua_CLuaIHM_runExprAndPushResult)
		// Execute expression
		CInterfaceExprValue value;
		if (CInterfaceExpr::eval(expr,    value,    NULL))
		{
			switch(value.getType())
			{
			case CInterfaceExprValue::Boolean:
				ls.push(value.getBool());
				break;
			case CInterfaceExprValue::Integer:
				ls.push(value.getInteger());
				break;
			case CInterfaceExprValue::Double:
				ls.push(value.getDouble());
				break;
			case CInterfaceExprValue::String:
				ls.push(value.getString());
				break;
			case CInterfaceExprValue::RGBA:
				{
					CRGBA color = value.getRGBA();
	#if LUABIND_VERSION > 600
					luabind::detail::push(ls.getStatePointer(), color);
	#else
					luabind::object obj(ls.getStatePointer(), color);
					obj.pushvalue();
	#endif
					break;
				}
				break;
			case CInterfaceExprValue::UserType: // Yoyo: don't care UserType...
			default:
				ls.pushNil();
				break;
			}
		}
		else
			ls.pushNil();

		return 1;
	}

	// ***************************************************************************
	int	CLuaIHM::runExpr(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_runExpr)
		CLuaStackChecker lsc(&ls,    1);

		// params: "expr".
		// return: any of: nil,   bool,   string,   number,    RGBA,    UCString
		CLuaIHM::checkArgCount(ls,    "runExpr",    1);
		CLuaIHM::check(ls,   ls.isString(1),    "runExpr() requires a string in param 1");

		// retrieve args
		std::string expr;
		ls.toString(1,    expr);

		// run expression and push result
		return runExprAndPushResult(ls,    expr);
	}

	// ***************************************************************************
	int CLuaIHM::runFct(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_runFct)
		CLuaStackChecker lsc(&ls,    1);

		// params: "expr",    param1,    param2...
		// return: any of: nil,   bool,   string,   number,    RGBA,    UCString
		CLuaIHM::checkArgMin(ls,    "runFct",    1);
		CLuaIHM::check(ls,   ls.isString(1),    "runExpr() requires a string in param 1");

		// retrieve fct
		std::string expr;
		ls.toString(1,    expr);
		expr+= "(";

		// retrieve params
		uint	top= ls.getTop();
		for(uint i=2;i<=top;i++)
		{
			if(i>2)
				expr+= ",   ";

			// If it is a number
			if(ls.type(i)==LUA_TNUMBER)
			{
				std::string	paramValue;
				ls.toString(i,    paramValue);		// nb: transformed to a string in the stack
				expr+= paramValue;
			}
			// else suppose a string
			else
			{
				// must enclose with "'"
				std::string	paramValue;
				ls.toString(i,    paramValue);
				expr+= std::string("'") + paramValue + std::string("'") ;
			}
		}

		// end fct call
		expr+= ")";


		// run expression and push result
		return runExprAndPushResult(ls,    expr);
	}

	// ***************************************************************************
	int CLuaIHM::runCommand(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_runCommand)
		CLuaStackChecker lsc(&ls,    1);
		if (ls.empty())
		{
			nlwarning("'runCommand' : Command name expected");
			ls.push(false);
			return 1;
		}
		const char *commandName = ls.toString(1);
		if (!commandName)
		{
			nlwarning("'runCommand' : Bad command name");
			ls.push(false);
			return 1;
		}
		if (!NLMISC::ICommand::LocalCommands || !NLMISC::ICommand::LocalCommands->count(ls.toString(1)))
		{
			nlwarning("'runCommand' : Command %s not found",    ls.toString(1));
			ls.push(false);
			return 1;
		}
		std::string rawCommandString = ls.toString(1);
		NLMISC::ICommand *command = (*NLMISC::ICommand::LocalCommands)[ls.toString(1)];
		nlassert(command);
		std::vector<std::string> args(ls.getTop() - 1);
		for(uint k = 2; k <= (uint) ls.getTop(); ++k)
		{
			if (ls.toString(k))
			{
				args[k - 2] = ls.toString(k);
				rawCommandString += " " + std::string(ls.toString(k));
			}
		}

		ls.push(command->execute(rawCommandString,   args,    NLMISC::ErrorLog(),    false,    true));
		return 1;
	}

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	int CLuaIHM::isUCString(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_isUCString)
		const char *funcName = "isUCString";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		ls.push(CLuaIHM::isUCStringOnStack(ls, 1));
		return 1;
	}

	// ***************************************************************************
	int CLuaIHM::concatUCString(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_concatUCString)
		const char *funcName = "concatUCString";
		ucstring result;
		for (uint k = 1; k <= (uint) ls.getTop(); ++k)
		{
			//nlwarning("arg %d = %s", k, ls.getTypename(ls.type(k)));
			ucstring part;
			if (ls.isString(k))
			{
				part.fromUtf8(ls.toString(k));
			}
			else
			{
				CLuaIHM::checkArgTypeUCString(ls, funcName, k);
				nlverify(CLuaIHM::getUCStringOnStack(ls, k, part));
			}
			result += part;
		}
		CLuaIHM::push(ls, result);
		return 1;
	}
#endif

	// ***************************************************************************
	int CLuaIHM::concatString(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_concatUCString)
		const char *funcName = "concatString";
		std::string result;
		uint stackSize = ls.getTop();
		for (uint k = 1; k <= stackSize; ++k)
		{
			CLuaIHM::checkArgType(ls, funcName, k, LUA_TSTRING);
			result += ls.toString(k);
		}
		ls.push(result);
		return 1;
	}

	// ***************************************************************************
	int CLuaIHM::tableToString(CLuaState &ls)
	{
		const char *funcName = "tableToString";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TTABLE);
		uint length = 0;
		// compute size
		ls.pushNil();
		while (ls.next(-2))
		{
			ls.toString(-1);
			length += (uint)ls.strlen(-1);
			ls.pop(2);
		}
		std::string result;
		result.resize(length);
		char *dest = &result[0];
		// concatenate
		ls.pushNil();
		while (ls.next(-2))
		{
			uint length = (uint)ls.strlen(-1);
			if (length)
			{
				memcpy(dest, ls.toString(-1), length);
			}
			dest += length;
			ls.pop(2);
		}
		ls.push(result);
		return 1;
	}


	int CLuaIHM::getPathContent(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_getPathContent)
		const char *funcName = "getPathContent";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		std::vector<std::string> files;
		NLMISC::CPath::getPathContent(ls.toString(1), false, false, true, files);
		ls.newTable();
		for(uint k = 0; k < files.size(); ++k)
		{
			ls.push(k);
			ls.push(files[k]);
			ls.setTable(-3);
		}
		return 1;
	}




	// ***************************************************************************
	void CLuaIHM::luaValueFromReflectedProperty(CLuaState &ls, CReflectable &reflectedObject, const CReflectedProperty &property)
	{
		//H_AUTO(Lua_CLuaIHM_luaValueFromReflectedProperty)
		switch(property.Type)
		{
			case CReflectedProperty::Boolean:
				ls.push( (reflectedObject.*(property.GetMethod.GetBool))() );
			break;
			case CReflectedProperty::SInt32:
				ls.push( (reflectedObject.*(property.GetMethod.GetSInt32))() );
			break;
			case CReflectedProperty::Float:
				ls.push( (reflectedObject.*(property.GetMethod.GetFloat))() );
			break;
			case CReflectedProperty::String:
				ls.push( (reflectedObject.*(property.GetMethod.GetString))() );
			break;
#ifdef RYZOM_LUA_UCSTRING
			case CReflectedProperty::UCString:
			{
				ucstring str = (reflectedObject.*(property.GetMethod.GetUCString))();
	#if LUABIND_VERSION > 600
				luabind::detail::push(ls.getStatePointer(), str);
	#else
				luabind::object obj(ls.getStatePointer(), str);
				obj.pushvalue();
	#endif
			}
			break;
			case CReflectedProperty::UCStringRef:
			{
				ucstring str = (reflectedObject.*(property.GetMethod.GetUCStringRef))();
	#if LUABIND_VERSION > 600
				luabind::detail::push(ls.getStatePointer(), str);
	#else
				luabind::object obj(ls.getStatePointer(), str);
				obj.pushvalue();
	#endif
			}
			break;
#endif
			case CReflectedProperty::StringRef:
				ls.push( (reflectedObject.*(property.GetMethod.GetStringRef))() );
			break;
			case CReflectedProperty::RGBA:
			{
				CRGBA color = (reflectedObject.*(property.GetMethod.GetRGBA))();
	#if LUABIND_VERSION > 600
				luabind::detail::push(ls.getStatePointer(), color);
	#else
				luabind::object obj(ls.getStatePointer(), color);
				obj.pushvalue();
	#endif
			}
			break;
			case CReflectedProperty::LuaMethod:
			{
				// must create a closure that will forward the call to the real method
				if (!property.LuaMethodRef.isValid())
				{
					ls.pushLightUserData((void *) &property);
					ls.pushLightUserData((void *) &ls);
					ls.pushCClosure(luaMethodCall, 2);
					property.LuaMethodRef.pop(ls);
				}
				nlassert(property.LuaMethodRef.getLuaState() == &ls); // only one single lua state supported for now
				property.LuaMethodRef.push();
			}
			break;
			default:
				nlstop;
			break;
		}
	}

	// ***************************************************************************
	void CLuaIHM::luaValueToReflectedProperty(CLuaState &ls, int stackIndex, CReflectable &target, const CReflectedProperty &property)
	{
		//H_AUTO(Lua_property_throw)
		if(ls.isNil(stackIndex))
			throw ELuaIHMException("Trying to set nil to UI property '%s'",    property.Name.c_str());
		switch(property.Type)
		{
			case CReflectedProperty::Boolean:
				{
					bool	val= ls.toBoolean(stackIndex);
					(target.*(property.SetMethod.SetBool))(val);
					return;
				}
			case CReflectedProperty::SInt32:
				{
					sint32	val= (sint32)ls.toInteger(stackIndex);
					(target.*(property.SetMethod.SetSInt32))(val);
					return;
				}
			case CReflectedProperty::UInt32:
				{
					uint32	val= (uint32)ls.toInteger(stackIndex);
					(target.*(property.SetMethod.SetUInt32))(val);
					return;
				}
			case CReflectedProperty::Float:
				{
					float	val= (float)ls.toNumber(stackIndex);
					(target.*(property.SetMethod.SetFloat))(val);
					return;
				}
			case CReflectedProperty::String:
			case CReflectedProperty::StringRef:
				{
					std::string val;
					ls.toString(stackIndex,    val);
					(target.*(property.SetMethod.SetString))(val);
					return;
				}
#ifdef RYZOM_LUA_UCSTRING
			case CReflectedProperty::UCString:
			case CReflectedProperty::UCStringRef:
				{
					ucstring val;
					// Additionaly return of CInterfaceExpr may be std::string... test std string too
					if(ls.isString() || ls.isNumber() || ls.isInteger())
					{
						std::string	str;
						ls.toString(stackIndex,    str);
						val= str;
					}
#ifdef RYZOM_LUA_UCSTRING
					else
					{
						// else this should be a ucstring
						if (!pop(ls, val))
						{
							throw ELuaIHMException("You must set a string,    number or ucstring to UI property '%s'",    property.Name.c_str());
						}
					}
#endif
					(target.*(property.SetMethod.SetUCString))(val);
					return;
				}
#endif
			case CReflectedProperty::RGBA:
				{
					CRGBA color;
					if (pop(ls,   color))
					{
						(target.*(property.SetMethod.SetRGBA))(color);
					}
					else
					{
						throw ELuaIHMException("You must set a CRGBA to UI property '%s'",    property.Name.c_str());
					}
					return;
				}
			default:
				nlstop;
		}
	}


	// ***************************************************************************
	void CLuaIHM::createLuaEnumTable(CLuaState &ls, const std::string &str)
	{
		//H_AUTO(Lua_CLuaIHM_createLuaEnumTable)
		std::string path = "", script, p;
		CSString s = str;
		// Create table recursively (ex: 'game.TPVPClan' will check/create the table 'game' and 'game.TPVPClan')
		p = s.splitTo('.', true);
		while (!p.empty())
		{
			if (path.empty() )
				path = p;
			else
				path += "." + p;
			script = "if (" + path + " == nil) then " + path + " = {}; end";
			ls.executeScript(script);
			p = s.splitTo('.', true);
		}
	}

	#define LUABIND_ENUM(__enum__, __name__, __num__, __toStringFunc__) \
		createLuaEnumTable(ls, __name__); \
		for (uint e=0 ; e<__num__ ; e++) \
		{ \
			std::string str = __toStringFunc__((__enum__)e); \
			std::string temp = __name__ + toString(".") + __toStringFunc__((__enum__)e) + " = " + toString("%u;", e); \
			ls.executeScript(temp); \
		} \

	// ***************************************************************************
	#define LUABIND_FUNC(__func__) luabind::def(#__func__,    &__func__)

	void	CLuaIHM::registerIHM(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_registerIHM)
		CLuaStackChecker lsc(&ls);

		// *** Register a Table for ui env.
		ls.push(IHM_LUA_ENVTABLE);			// "__ui_envtable"
		ls.newTable();						// "__ui_envtable"  {}
		ls.setTable(LUA_REGISTRYINDEX);


		// *** Register the MetaTable for UI userdata
		ls.push(IHM_LUA_METATABLE);			// "__ui_metatable"
		ls.newTable();						// "__ui_metatable"  {}
		// set the '__index' method
		ls.push("__index");
		ls.push(luaUIIndex);
		nlassert(ls.isCFunction());
		ls.setTable(-3);					// "__ui_metatable"  {"__index"= CFunc_luaUIIndex}
		// set the '__newindex' method
		ls.push("__newindex");
		ls.push(luaUINewIndex);
		nlassert(ls.isCFunction());
		ls.setTable(-3);
		// set the '__newindex' method
		ls.push("__gc");
		ls.push(luaUIDtor);
		nlassert(ls.isCFunction());
		ls.setTable(-3);
		// set the '__eq' method
		ls.push("__eq");
		ls.push(luaUIEq);
		nlassert(ls.isCFunction());
		ls.setTable(-3);
		// set the custom '__next' method
		ls.push("__next");
		ls.push(luaUINext);
		nlassert(ls.isCFunction());
		ls.setTable(-3);
		// set registry
		ls.setTable(LUA_REGISTRYINDEX);


		// *** Register Functions
		ls.registerFunc("setOnDraw",    setOnDraw);
		ls.registerFunc("getOnDraw", getOnDraw);
		ls.registerFunc("setCaptureKeyboard", setCaptureKeyboard);
		ls.registerFunc("resetCaptureKeyboard", resetCaptureKeyboard);
		ls.registerFunc("setTopWindow", setTopWindow);
		ls.registerFunc("addOnDbChange",    addOnDbChange);
		ls.registerFunc("removeOnDbChange",    removeOnDbChange);
		ls.registerFunc("getUIId",    getUIId);
		ls.registerFunc("runAH",    runAH);
		ls.registerFunc("deleteUI",    deleteUI);
		ls.registerFunc("deleteReflectable",    deleteReflectable);
		ls.registerFunc("getWindowSize",    getWindowSize);
		ls.registerFunc("getTextureSize", getTextureSize);
		ls.registerFunc("disableModalWindow", disableModalWindow);
#ifdef RYZOM_LUA_UCSTRING
		ls.registerFunc("isUCString", isUCString);
		ls.registerFunc("concatUCString", concatUCString);
#endif
		ls.registerFunc("concatString", concatString);
		ls.registerFunc("tableToString", tableToString);
		ls.registerFunc("getCurrentWindowUnder", getCurrentWindowUnder);
		ls.registerFunc("runExpr",    runExpr);
		ls.registerFunc("runFct",    runFct);
		ls.registerFunc("runCommand",    runCommand);
		ls.registerFunc("getPathContent", getPathContent);
		
		// Through LUABind API
		lua_State	*L= ls.getStatePointer();

		luabind::module(L)
		[
			luabind::def("findReplaceAll",    (std::string(*)(const std::string &,  const std::string &,  const std::string &)) &findReplaceAll),
			luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const ucstring &,  const ucstring &)) &findReplaceAll),
			luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const std::string &,  const std::string &)) &findReplaceAll),
			luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const ucstring &,  const std::string &)) &findReplaceAll),
			luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const std::string &,  const ucstring &)) &findReplaceAll),

		#if !FINAL_VERSION
			LUABIND_FUNC(openDoc),
			LUABIND_FUNC(launchProgram),
		#endif

			luabind::def("fileLookup",    CMiscFunctions::fileLookup),
			luabind::def("shellExecute",  CMiscFunctions::shellExecute),
			LUABIND_FUNC(fileExists)
		];

		// inside i18n table
		luabind::module(L, "i18n")
		[
#ifdef RYZOM_LUA_UCSTRING
			luabind::def("get", &CI18N::getAsUtf16), // Compatibility
#else
			luabind::def("get", &CI18N::get),
#endif
			luabind::def("hasTranslation", &CI18N::hasTranslation)
		];
		// inside 'nlfile' table
		luabind::module(L, "nlfile")
		[
			luabind::def("getFilename", NLMISC::CFile::getFilename),
			luabind::def("getExtension", NLMISC::CFile::getExtension),
			luabind::def("getFilenameWithoutExtension", NLMISC::CFile::getFilenameWithoutExtension)
		];
		// inside 'nltime' table
		luabind::module(L, "nltime")
		[
			luabind::def("getPreciseLocalTime", getPreciseLocalTime),
			luabind::def("getSecondsSince1970", NLMISC::CTime::getSecondsSince1970),
			luabind::def("getLocalTime", getLocalTime)	// NB : use CLuaIHM::getLocalTime instead of NLMISC::CTime::getLocalTime, because the NLMISC
														// version returns a uint64, which can't be casted into lua numbers (doubles ...)
		];
	}


	// ***************************************************************************
	double CLuaIHM::getPreciseLocalTime()
	{
		//H_AUTO(Lua_CLuaIHM_getPreciseLocalTime)
		// don't export these 2 function to lua directly here, because all uint64 can't be represented with lua 'numbers'
		return NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	}

	// ***************************************************************************
	void	CLuaIHM::registerAll(CLuaState &ls)
	{
		//H_AUTO(Lua_CLuaIHM_registerAll)
		registerBasics(ls);
		registerIHM(ls);
	}



	//#define CHECK_REFLECTABLE_MT


	// ***************************************************************************
	void CLuaIHM::pushReflectableOnStack(CLuaState &ls,   class CReflectableRefPtrTarget *pRPT)
	{
		//H_AUTO(Lua_CLuaIHM_pushReflectableOnStack)
		nlassert(pRPT);
		CLuaStackChecker lsc(&ls,    1);

		if (!pRPT)
		{
			ls.pushNil();
			return;
		}

		//ls.dumpStack();
		/** if there's already a ref ptr for this object in the registry, then use it,
		  * else create a new one to avoid costly allocations
		  */
		ls.pushLightUserData(pRPT);
		ls.getTable(LUA_REGISTRYINDEX);
		//ls.dumpStack();
		if (ls.isNil())
		{
			ls.pop(1);
			//ls.dumpStack();
			// allocate the user data where to put the ref ptr
			void	*ptr= ls.newUserData(sizeof(CReflectableLuaRef));
			nlassert(ptr);
			//ls.dumpStack();

// disable memory leaks detection for placement new
#ifdef new
	#undef new
#endif

			// initialize it, and copy the given element
			new (ptr) CReflectableLuaRef(pRPT);

// reenable memory leaks detection for placement new
#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

			// Assign to this user data the __ui_metatable
			//ls.dumpStack();
			ls.push(IHM_LUA_METATABLE);			// userdata   "__ui_metatable"
			//ls.dumpStack();
			ls.getTable(LUA_REGISTRYINDEX);		// userdata   __ui_metatable
			//ls.dumpStack();
			nlverify(ls.setMetaTable(-2));		// userdata
			//ls.dumpStack();

			// cache in registry
			ls.pushLightUserData(pRPT);
			ls.pushValue(-2); // copy for table insertion
			//ls.dumpStack();
			ls.setTable(LUA_REGISTRYINDEX);
			//ls.dumpStack();
		}

		// Check that the metatable is correct
	#ifdef CHECK_REFLECTABLE_MT
		nlverify(ls.getMetaTable(-1));		// userdata   __ui_metatable
		ls.push("__index");
		ls.getTable(-2);
		ls.push("__newindex");
		ls.getTable(-3);
		ls.push("__gc");
		ls.getTable(-4);
		nlassert(ls.isCFunction(-1));
		nlassert(ls.isCFunction(-2));
		nlassert(ls.isCFunction(-3));
		ls.pop(4);
	#endif
		//ls.dumpStack();
	}

	// ***************************************************************************
	bool CLuaIHM::isReflectableOnStack(CLuaState &ls,   sint index)
	{
		//H_AUTO(Lua_CLuaIHM_isReflectableOnStack)
		CLuaStackChecker lsc(&ls);

		if(!ls.isUserData(index))
			return false;
		// verify that it is a UI with its metatable
		if(!ls.getMetaTable(index))			// ???  object_metatable
			return false;
		ls.push(IHM_LUA_METATABLE);			// ???  object_metatable  "__ui_metatable"
		ls.getTable(LUA_REGISTRYINDEX);		// ???  object_metatable  __ui_metatable
		// equal test
		bool	ok= ls.rawEqual(-2,    -1);
		// Also must not be nil (maybe nil in case of LuaIHM still not registered)
		ok= ok && !ls.isNil(-1);
		ls.pop();
		ls.pop();

		return ok;
	}

	// ***************************************************************************
	CReflectableRefPtrTarget *CLuaIHM::getReflectableOnStack(CLuaState &ls,    sint index)
	{
		//H_AUTO(Lua_CLuaIHM_getReflectableOnStack)
		if(!isReflectableOnStack(ls,    index))
			return NULL;

		CReflectableLuaRef	*p= (CReflectableLuaRef *) ls.toUserData(index);
		nlassert(p->Ptr);
		return p->Ptr;
	}


	// ***************************************************************************
	// ***************************************************************************
	// LUA IHM Functions
	// ***************************************************************************
	// ***************************************************************************


	// ***************************************************************************
	uint32 CLuaIHM::getLocalTime()
	{
		//H_AUTO(Lua_CLuaIHM_getLocalTime)
		return (uint32) NLMISC::CTime::getLocalTime();
	}


	// ***************************************************************************
	std::string		CLuaIHM::findReplaceAll(const std::string &str,   const std::string &search,   const std::string &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		std::string ret= str;
		while(strFindReplace(ret,   search,   replace));
		return ret;
	}

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const ucstring &search,   const ucstring &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return strFindReplaceAll(str, search, replace);
	}

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const std::string &search,   const std::string &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return findReplaceAll(str,   ucstring::makeFromUtf8(search),   ucstring::makeFromUtf8(replace));
	}

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const std::string &search,   const ucstring &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return findReplaceAll(str,   ucstring::makeFromUtf8(search),  replace);
	}

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const ucstring &search,   const std::string &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return findReplaceAll(str,   search,   ucstring::makeFromUtf8(replace));
	}
#endif

	// ***************************************************************************
	void CLuaIHM::fails(CLuaState &ls, const char *format, ...)
	{
		//H_AUTO(Lua_CLuaIHM_fails)
		std::string	reason;
		NLMISC_CONVERT_VARGS (reason,    format,    NLMISC::MaxCStringSize);
		std::string stack;
		ls.getStackAsString(stack);
		// use a std::exception,    to avoid Nel Exception warning
		throw ELuaIHMException("%s. Lua stack = \n %s", reason.c_str(), stack.c_str());
	}


	// ***************************************************************************
	void	CLuaIHM::checkArgCount(CLuaState &ls,    const char* funcName,    uint nArgs)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgCount)
		if(ls.getTop()!=(sint)nArgs)
		{
			fails(ls, "%s() need exactly %d arguments (tips : check between method & function call)",    funcName,    nArgs);
		}
	}

	// ***************************************************************************
	void	CLuaIHM::checkArgMin(CLuaState &ls,    const char* funcName,    uint nArgs)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgMin)
		if(ls.getTop()<(sint)nArgs)
		{
			fails(ls, "%s() need at least %d arguments (tips : check between method & function call)",    funcName,    nArgs);
		}
	}

	// ***************************************************************************
	void CLuaIHM::checkArgMax(CLuaState &ls,const char* funcName,uint nArgs)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgMax)
		if(ls.getTop()>(sint)nArgs)
		{
			fails(ls, "%s() need at most %d arguments.",    funcName,    nArgs);
		}
	}

	// ***************************************************************************
	void	CLuaIHM::check(CLuaState &ls,   bool ok,    const std::string &failReason)
	{
		//H_AUTO(Lua_CLuaIHM_check)
		if(!ok)
		{
			fails(ls, failReason.c_str());
		}
	}

	// ***************************************************************************
	void CLuaIHM::checkArgType(CLuaState &ls,   const char *funcName,   uint index,   int argType)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgType)
		nlassert(index > 0);
		if (ls.getTop() < (int) index)
		{
			fails(ls, "%s : argument %d of expected type %s was not defined",   funcName,   index,   ls.getTypename(argType));
		}
		if (ls.type(index) != argType)
		{
			fails(ls, "%s : argument %d of expected type %s has bad type : %s",   funcName,   index,   ls.getTypename(argType),   ls.getTypename(ls.type(index)),   ls.type(index));
		}
	}

	// ***************************************************************************
	void CLuaIHM::checkArgTypeRGBA(CLuaState &ls, const char *funcName, uint index)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgTypeRGBA)
		nlassert(index > 0);
		if (ls.getTop() < (int) index)
		{
			fails(ls, "%s : argument %d of expected type RGBA was not defined",   funcName,   index);
		}
		ls.pushValue(index);
		CRGBA dummy;
		if (!pop(ls, dummy))
		{
			fails(ls, "%s : argument %d of expected type RGBA has bad type : %s",   funcName,   index, ls.getTypename(ls.type(index)),   ls.type(index));
		}
	}

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	void CLuaIHM::checkArgTypeUCString(CLuaState &ls, const char *funcName, uint index)
	{
		//H_AUTO(Lua_CLuaIHM_checkArgTypeUCString)
		nlassert(index > 0);
		if (ls.getTop() < (int) index)
		{
			fails(ls, "%s : argument %d of expected type ucstring was not defined",   funcName,   index);
		}
		ls.pushValue(index);
		ucstring dummy;
		if (!pop(ls, dummy))
		{
			fails(ls, "%s : argument %d of expected type ucstring has bad type : %s",   funcName,   index,   ls.getTypename(ls.type(index)),   ls.type(index));
		}
	}
#endif


	// ***************************************************************************
	bool CLuaIHM::popString(CLuaState &ls, std::string & dest)
	{
		//H_AUTO(Lua_CLuaIHM_popString)
		try
		{
	#if LUABIND_VERSION > 600
			luabind::object obj(luabind::from_stack(ls.getStatePointer(), -1));
			ls.pop();
	#else
			luabind::object obj(ls.getStatePointer());
			obj.set();
	#endif
			dest = luabind::object_cast<std::string>(obj);
		}
		catch(const luabind::cast_failed &)
		{
			return false;
		}
		return true;
	}

	// ***************************************************************************
	bool CLuaIHM::popSINT32(CLuaState &ls, sint32 & dest)
	{
		//H_AUTO(Lua_CLuaIHM_popSINT32)
		try
		{
	#if LUABIND_VERSION > 600
			luabind::object obj(luabind::from_stack(ls.getStatePointer(), -1));
			ls.pop();
	#else
			luabind::object obj(ls.getStatePointer());
			obj.set();
	#endif
			dest = luabind::object_cast<sint32>(obj);
		}
		catch(const luabind::cast_failed &)
		{
			return false;
		}
		return true;
	}

	// ***************************************************************************
	void CLuaIHM::getPoly2DOnStack(CLuaState &ls, sint index, NLMISC::CPolygon2D &dest)
	{
		//H_AUTO(Lua_CLuaIHM_getPoly2DOnStack)
		ls.pushValue(index);
		CLuaObject poly;
		poly.pop(ls);
		dest.Vertices.clear();
		ENUM_LUA_TABLE(poly, it)
		{
			it.nextValue().push();
			NLMISC::CVector2f pos;
			if (!pop(ls, pos))
			{
				fails(ls, "2D polygon expects CVector2f for poly coordinates");
			}
			dest.Vertices.push_back(pos);
		}
	}

}
