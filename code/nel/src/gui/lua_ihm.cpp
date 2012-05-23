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


#include "nel/gui/lua_helper.h"

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
#include "nel/gui/lua_object.h"
#include "nel/misc/polygon.h"
#include "nel/gui/lua_manager.h"


// ***************************************************************************
/*
IMPORTANT NOTE: we do this heavy double registration in this file because we DON'T want
to include luabind.hpp in every file.
Compilation is VERY SLOW
*/
// ***************************************************************************

using namespace NLMISC;

// declare ostream << operator for ucstring -> registration of ucstring iin luabind will build a 'tostring' function from it
std::ostream &operator<<(std::ostream &str, const ucstring &value)
{
	return str << value.toString();
}

namespace NLGUI
{

	struct CMiscFunctions
	{
		static std::string fileLookup(const std::string &fileName)
		{
			return NLMISC::CPath::lookup(fileName,   false);
		}
		static void shellExecute(const char *operation,   const char *fileName,   const char *parameters)
		{
		#if !FINAL_VERSION
			#ifdef NL_OS_WINDOWS
				ShellExecute(NULL,   operation,   fileName,   parameters,  NULL,   SW_SHOWDEFAULT);
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
	luabind::detail::yes_t is_user_defined(luabind::detail::by_value<_type_>);													\
	_type_ convert_lua_to_cpp(lua_State* L,    luabind::detail::by_value<_type_>,    int index)						\
	{																							\
		return (_type_)lua_tonumber(L,    index);													\
	}																							\
	int match_lua_to_cpp(lua_State* L,    luabind::detail::by_value<_type_>,    int index)								\
	{																							\
		if (lua_isnumber(L,    index)) return 0; else return -1;									\
	}																							\
	void convert_cpp_to_lua(lua_State* L,    const  _type_& v)										\
	{																							\
		lua_pushnumber(L,    (double)v);															\
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
	//		LUA_REGISTER_BASIC(sint)
	//		LUA_REGISTER_BASIC(uint)
		}
	}

namespace NLGUI
{

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
		if (pRPT == NULL)
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
		sint initialStackSize = state->getTop();
		try
		{
			// call the actual method
			numResults = (pRPT->*(prop->GetMethod.GetLuaMethod))(*state);
		}
		catch(const std::exception &e)
		{
			// restore stack to its initial size
			state->setTop(initialStackSize);
			lua_pushstring(ls,      e.what());
			// TODO : see if this is safe to call lua error there" ... (it does a long jump)
			lua_error(ls);
		}
		return numResults;
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
				ls.push( (lua_Number)(reflectedObject.*(property.GetMethod.GetSInt32))() );
			break;
			case CReflectedProperty::Float:
				ls.push( (lua_Number)(reflectedObject.*(property.GetMethod.GetFloat))() );
			break;
			case CReflectedProperty::String:
				ls.push( (reflectedObject.*(property.GetMethod.GetString))() );
			break;
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

	static CLuaString lstr_Env("Env");
	static CLuaString lstr_isNil("isNil");

	// ***************************************************************************
	void CLuaIHM::luaValueToReflectedProperty(CLuaState &ls, int stackIndex, CReflectable &target, const CReflectedProperty &property) throw(ELuaIHMException)
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
					sint32	val= (sint32)ls.toNumber(stackIndex);
					(target.*(property.SetMethod.SetSInt32))(val);
					return;
				}
			case CReflectedProperty::UInt32:
				{
					uint32	val= (uint32)ls.toNumber(stackIndex);
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
				{
					std::string val;
					ls.toString(stackIndex,    val);
					(target.*(property.SetMethod.SetString))(val);
					return;
				}
			case CReflectedProperty::UCString:
				{
					ucstring val;
					// Additionaly return of CInterfaceExpr may be std::string... test std string too
					if(ls.isString() || ls.isNumber())
					{
						std::string	str;
						ls.toString(stackIndex,    str);
						val= str;
					}
					else
					{
						// else this should be a ucstring
						if (!pop(ls, val))
						{
							throw ELuaIHMException("You must set a string,    number or ucstring to UI property '%s'",    property.Name.c_str());
						}
					}
					(target.*(property.SetMethod.SetUCString))(val);
					return;
				}
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
		while (p.size() > 0)
		{
			if (path == "")
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
			std::string temp = __name__ + toString(".") + __toStringFunc__((__enum__)e) + " = " + toString("%d;", e); \
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


		// *** Register Functions
		
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
			luabind::def("shellExecute",  CMiscFunctions::shellExecute)
		];

		// inside i18n table
		luabind::module(L, "i18n")
		[
			luabind::def("get", &CI18N::get),
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
			// initialize it,    and copy the given element
			new (ptr) CReflectableLuaRef(pRPT);
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

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const ucstring &search,   const ucstring &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		ucstring	ret= str;
		while(strFindReplace(ret,   search,   replace));
		return ret;
	}

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const std::string &search,   const std::string &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return findReplaceAll(str,   ucstring(search),   ucstring(replace));
	}

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const std::string &search,   const ucstring &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return findReplaceAll(str,   ucstring(search),   ucstring(replace));
	}

	// ***************************************************************************
	ucstring		CLuaIHM::findReplaceAll(const ucstring &str,   const ucstring &search,   const std::string &replace)
	{
		//H_AUTO(Lua_CLuaIHM_findReplaceAll)
		return findReplaceAll(str,   ucstring(search),   ucstring(replace));
	}


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
