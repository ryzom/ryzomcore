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


#include "stdpch.h"
#include <algorithm>

#include "lua_helper.h"
// Warning: cannot use namespace std,    when using luabind
#ifdef NL_OS_WINDOWS
#  ifndef NL_EXTENDED_FOR_SCOPE
#    undef for
#  endif
#endif

// to get rid of you_must_not_use_assert___use_nl_assert___read_debug_h_file messages
#include <cassert>
#undef assert
#define assert nlassert
#include <luabind/luabind.hpp>
#if LUABIND_MAX_ARITY == 10
# include <luabind/version.hpp>
# include <luabind/operator.hpp>
#elif LUABIND_MAX_ARITY == 5
# define LUABIND_VERSION 600
#else
# pragma error("luabind version not recognized")
#endif


#include "lua_ihm.h"
#include "reflect.h"
#include "action_handler.h"
#include "action_handler_tools.h"
#include "interface_manager.h"
#include "interface_group.h"
#include "view_text.h"
#include "game_share/people_pd.h"
#include "group_tree.h"
#include "interface_link.h"
#include "interface_expr.h"
#include "people_interraction.h"
#include "nel/misc/algo.h"
#include "nel/misc/file.h"
#include "nel/misc/i18n.h"
#include "nel/misc/time_nl.h"
#include "skill_manager.h"
#include "group_html.h"
#include "../net_manager.h"
#include "../user_entity.h"
#include "sphrase_manager.h"
#include "guild_manager.h"
#include "../client_cfg.h"
#include "../sheet_manager.h"
#include "lua_object.h"
#include "game_share/emote_list_parser.h"
#include "game_share/pvp_clan.h"
#include "../weather.h"
#include "../continent_manager.h"
#include "../zone_util.h"
#include "../motion/user_controls.h"
#include "group_html_cs.h"
#include "bonus_malus.h"
#include "group_editbox.h"
#include "../entities.h"
#include "../sheet_manager.h"				// for emotes
#include "../global.h"						// for emotes
#include "../entity_animation_manager.h"	// for emotes
#include "../net_manager.h"				// for emotes
#include "../client_chat_manager.h"		// for emotes
#include "../login.h"
#include "lua_object.h"
#include "../actions.h"
#include "../bg_downloader_access.h"
#include "../connection.h"
#include "../login_patch.h"

#include "bot_chat_page_all.h"
#include "bot_chat_page_ring_sessions.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/polygon.h"
#include "game_share/scenario_entry_points.h"
#include "game_share/bg_downloader_msg.h"
#include "game_share/constants.h"

#ifdef LUA_NEVRAX_VERSION
	#include "lua_ide_dll_nevrax/include/lua_ide_dll/ide_interface.h" // external debugger
#endif


#ifdef LUA_NEVRAX_VERSION
	extern ILuaIDEInterface *LuaDebuggerIDE;
#endif

// ***************************************************************************
/*
IMPORTANT NOTE: we do this heavy double registration in this file because we DON'T want
to include luabind.hpp in every file.
Compilation is VERY SLOW
*/
// ***************************************************************************

using namespace NLMISC;
using namespace NLGEORGES;
using namespace R2;


extern NLMISC::CLog	g_log;
extern CContinentManager ContinentMngr;
extern uint8 PlayerSelectedSlot;
extern CClientChatManager		ChatMngr;
extern void addWebIGParams (string &url);

// declare ostream << operator for ucstring -> registration of ucstring iin luabind will build a 'tostring' function from it
std::ostream &operator<<(std::ostream &str, const ucstring &value)
{
	return str << value.toString();
}

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


/**
  * Data structure pushed in lua (a userdata) to access CReflectableRefPtrTarget derived objects
  * These includes element of the GUI.
  * if holds a pointer to the reflectable object, and
  * a cache to its CClassInfo for fast access to exported properties
  * \see reflect.h
  */

//
class CReflectableLuaRef
{
public:
	CReflectableLuaRef(CReflectableRefPtrTarget *ptr = NULL) : Ptr(ptr), _ClassInfo(NULL) {}
	NLMISC::CRefPtr<CReflectableRefPtrTarget> Ptr;
	const CClassInfo						  &getClassInfo() const;
	// IMPORTANT : luaStringPtr should have been obtained from lua, see remark in CClassInfo
	const CReflectedProperty				  *getProp(const char *luaStringPtr) const;
private:
	// cache to class definition of the pointee object (once a CReflectableLuaRef created in lua, it remains a *const* pointer)
	mutable const CClassInfo							  *_ClassInfo;
};

inline const CClassInfo &CReflectableLuaRef::getClassInfo() const
{
	nlassert(Ptr); // class info should not be accessed for a null ptr
	if (_ClassInfo) return *_ClassInfo;
	_ClassInfo = Ptr->getClassInfo();
	return *_ClassInfo;
}

const CReflectedProperty *CReflectableLuaRef::getProp(const char *luaStringPtr) const
{
	const CClassInfo &ci = getClassInfo();
	CClassInfo::TLuaStrToPropMap::const_iterator it = ci.LuaStrToProp.find(luaStringPtr);
	if (it != ci.LuaStrToProp.end())
	{
		return it->second.Prop;
	}
	// slowly retrieve property, and store in cache
	// NB nico : this could also be done at startup...
	const CReflectedProperty *prop = CReflectSystem::getProperty(ci.ClassName, luaStringPtr, false);
	if (!prop) return NULL;
	CLuaIndexedProperty lip;
	lip.Id = CLuaString(luaStringPtr); // keep a ref on the lua string to ensure that its pointer always remains valid
	lip.Prop = prop;
	ci.LuaStrToProp[luaStringPtr] = lip;
	return prop;
}

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
	catch(luabind::cast_failed &)
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
	catch(luabind::cast_failed &)
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
	catch(luabind::cast_failed &)
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
// Special Action Handlers for Lua
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
class CHandlerLUA : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller,    const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// For getUI() LUA function,    push the UI caller
		if(pCaller)
			_UICallerStack.push_back(pCaller);

		// execute a small script. NB: use a small script here because
		// most often action handlers are called from xml files => lot of redundant script
		pIM->executeLuaScript(sParams,   true);

		// pop UI caller
		if(pCaller)
			_UICallerStack.pop_back();
	}

	// get the top of stack Caller to this LUA script
	static CCtrlBase	*getUICaller();

private:
	static	std::deque<CRefPtr<CCtrlBase> >		_UICallerStack;
};
REGISTER_ACTION_HANDLER( CHandlerLUA,    "lua");
std::deque<CRefPtr<CCtrlBase> >		CHandlerLUA::_UICallerStack;

CCtrlBase	*CHandlerLUA::getUICaller()
{
	if(_UICallerStack.empty())
		return NULL;
	else
		return _UICallerStack.back();
}


// ***************************************************************************
// Allow also to call script from expression
static DECLARE_INTERFACE_USER_FCT(lua)
{
	if(args.size()!=1 || !args[0].toString())
	{
		nlwarning("<lua> requires 1 arg (string=script)");
		return false;
	}

	// Retrieve lua state
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CLuaState	*state= pIM->getLuaState();
	if(!state)
		return false;
	CLuaState	&ls= *state;

	// *** clear return value
	const	std::string		retId= "__ui_internal_ret_";
	CLuaStackChecker	lsc(&ls);
	ls.push(retId);
	ls.pushNil();
	ls.setTable(LUA_GLOBALSINDEX);


	// *** execute script
	std::string	script= args[0].getString();
	// assign return value in retId.
	script= retId + "= " + script;
	// execute a small script here,   because most often exprs are called from xml files => lot of redundant script
	pIM->executeLuaScript(script,   true);


	// *** retrieve and convert return value
	ls.push(retId);
	ls.getTable(LUA_GLOBALSINDEX);
	bool	ok= false;
	sint	type= ls.type();
	if (type==LUA_TBOOLEAN)
	{
		// get and pop
		bool	val= ls.toBoolean();
		ls.pop();
		// set result
		result.setBool(val);
		ok= true;
	}
	else if(type==LUA_TNUMBER)
	{
		// get and pop
		double	val= ls.toNumber();
		ls.pop();
		// set double or integer?
		if(val==floor(val))
			result.setInteger(sint64(floor(val)));
		else
			result.setDouble(val);
		ok= true;
	}
	else if(type==LUA_TSTRING)
	{
		// get and pop
		std::string	val;
		ls.toString(-1,    val);
		ls.pop();
		// set result
		result.setString(val);
		ok= true;
	}
	else if(type==LUA_TUSERDATA)
	{
		// NB: the value is poped in obj.set() (no need to do ls.pop());

		// try with ucstring
		ucstring ucstrVal;
		if (CLuaIHM::pop(ls, ucstrVal))
		{
			result.setUCString(ucstrVal);
			ok= true;
		}

		// try with RGBA
		if(!ok)
		{
			NLMISC::CRGBA rgbaVal;
			if (CLuaIHM::pop(ls, rgbaVal))
			{
				result.setRGBA(rgbaVal);
				ok= true;
			}
		}
	}
	else
	{
		// error (nil for instance)
		ls.pop();
	}

	return ok;
}
REGISTER_INTERFACE_USER_FCT("lua",    lua)



// ***************************************************************************
// ***************************************************************************
// CInterface To LUA Registry
// ***************************************************************************
// ***************************************************************************


CLuaState * ELuaIHMException::getLuaState()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	return im->getLuaState();
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

	// MISC ui ctors
	struct CUICtor
	{
		// CGroupTree::SNode
		static int SNode(CLuaState &ls)
		{
			CLuaIHM::checkArgCount(ls,    "SNode",    0);
			CLuaIHM::pushReflectableOnStack(ls,    new CGroupTree::SNode);
			return 1;
		}
	};

	ls.registerFunc("SNode",    CUICtor::SNode);
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
	int numResults;
	int initialStackSize = state->getTop();
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
		luaValueFromReflectedProperty(ls, *pRPT, *prop);
		return 1;
	}

	// ** try to get a UI relative
	CInterfaceElement	*uiRelative= getUIRelative(dynamic_cast<CInterfaceElement *>(pRPT),    propName);
	if(uiRelative)
	{
		// push the UI onto the stack
		CLuaIHM::pushUIOnStack(ls,    uiRelative);
		return 1;
	}


	// Fail to find any Attributes or elements
	// Yoyo: don't write any message or warning because this may be a feature (if user want to test that something exit in the ui)
	ls.pushNil();
	return 1;
}


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
		luaValueToReflectedProperty(ls, 3, *pRPT, *prop);
		return 0;
	}

	CInterfaceElement	*pIE = dynamic_cast<CInterfaceElement *>(pRPT);
	// ** try to get an other UI (child or parent)
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
	CLuaIHM::check(ls, isReflectableOnStack(ls, 1), "__next :  require ui element as first arg");
	CReflectableRefPtrTarget *reflectedObject = getReflectableOnStack(ls, 1);
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
					CLuaIHM::pushUIOnStack(ls, group->getGroups()[key.Index]);
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
					CLuaIHM::pushUIOnStack(ls, group->getViews()[key.Index]);
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
					CLuaIHM::pushUIOnStack(ls, group->getControls()[key.Index]);
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
					luaValueFromReflectedProperty(ls, *reflectedObject, key.ClassInfo->Properties[key.Index]);
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

int CLuaIHM::getClientCfgVar(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getClientCfgVar)
	const char *funcName = "getClientCfgVar";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	std::string varName = ls.toString(1);

	CConfigFile::CVar *v = ClientCfg.ConfigFile.getVarPtr(varName);
	if (!v) return 0;
	if(v->size()==1)
	{
		switch(v->Type)
		{
			case CConfigFile::CVar::T_REAL:
				ls.push((double) v->asDouble());
				return 1;
			break;
			case CConfigFile::CVar::T_STRING:
				ls.push(v->asString());
				return 1;
			break;
			default: // handle both T_INT && T_BOOL
			case CConfigFile::CVar::T_INT:
				ls.push((double) v->asInt());
				return 1;
			break;
		}
	}
	else
	{
		ls.newTable();
		CLuaObject result(ls);
		uint count = 0;
		for(uint i = 0; i<v->StrValues.size(); i++)
		{
			result.setValue(toString(count).c_str(), v->StrValues[i]);
			count++;
		}
		for(uint i = 0; i<v->IntValues.size(); i++)
		{
			result.setValue(toString(count).c_str(), (double)v->IntValues[i]);
			count++;
		}
		for(uint i = 0; i<v->RealValues.size(); i++)
		{
			result.setValue(toString(count).c_str(), (double)v->RealValues[i]);
			count++;
		}
		result.push();
		return 1;
	}

	return 0;
}


// ***************************************************************************
int CLuaIHM::luaClientCfgIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaClientCfgIndex)
	CConfigFile::CVar *v = ClientCfg.ConfigFile.getVarPtr(ls.toString(2));
	if (!v) return 0;
	if (v->size() != 1)
	{
		// arrays not implemented (would require a second metatable)....
		throw ELuaWrappedFunctionException(&ls, "Access to array inside client.cfg not supported.");
	}
	switch(v->Type)
	{
		case CConfigFile::CVar::T_REAL:
			ls.push((double) v->asDouble());
			return 1;
		break;
		case CConfigFile::CVar::T_STRING:
			ls.push(v->asString());
			return 1;
		break;
		default: // handle both T_INT && T_BOOL
		case CConfigFile::CVar::T_INT:
			ls.push((double) v->asInt());
			return 1;
		break;
	}
	return 0;
}

// ***************************************************************************
int CLuaIHM::luaClientCfgNewIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_luaClientCfgNewIndex)
	throw ELuaWrappedFunctionException(&ls, "Can't write into config file from lua.");
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

	// *** Register the metatable for access to client.cfg (nb nico this may be more general later -> access to any config file ...)
	{
		ls.pushValue(LUA_GLOBALSINDEX);
		CLuaObject globals(ls);
		CLuaObject clientCfg = globals.newTable("config");
		CLuaObject mt = globals.newTable("__cfmt");
		nlverify(clientCfg.setMetaTable(mt));
		mt.setValue("__index", luaClientCfgIndex);
		mt.setValue("__newindex", luaClientCfgNewIndex);
		globals.setNil("__cfmt"); // remove temp metatable
	}

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


	// *** Register a Table for ui env.
	ls.push(IHM_LUA_ENVTABLE);			// "__ui_envtable"
	ls.newTable();						// "__ui_envtable"  {}
	ls.setTable(LUA_REGISTRYINDEX);


	// *** Register Functions
	// Through std LUA API
	ls.registerFunc("setOnDraw",    setOnDraw);
	ls.registerFunc("setCaptureKeyboard", setCaptureKeyboard);
	ls.registerFunc("resetCaptureKeyboard", resetCaptureKeyboard);
	ls.registerFunc("validMessageBox",    validMessageBox);
	ls.registerFunc("setTopWindow", setTopWindow);
	ls.registerFunc("concatUCString", concatUCString);
	ls.registerFunc("concatString", concatString);
	ls.registerFunc("tableToString", tableToString);
	ls.registerFunc("addOnDbChange",    addOnDbChange);
	ls.registerFunc("removeOnDbChange",    removeOnDbChange);
	ls.registerFunc("getUICaller",    getUICaller);
	ls.registerFunc("getCurrentWindowUnder", getCurrentWindowUnder);
	ls.registerFunc("getUI",    getUI);
	ls.registerFunc("getIndexInDB", getIndexInDB);
	ls.registerFunc("getUIId",    getUIId);
	ls.registerFunc("createGroupInstance", createGroupInstance);
	ls.registerFunc("createUIElement", createUIElement);
	ls.registerFunc("launchContextMenuInGame",    launchContextMenuInGame);
	ls.registerFunc("parseInterfaceFromString",    parseInterfaceFromString);
	ls.registerFunc("updateAllLocalisedElements",    updateAllLocalisedElements);
	ls.registerFunc("runAH",    runAH);
	ls.registerFunc("runExpr",    runExpr);
	ls.registerFunc("runFct",    runFct);
	ls.registerFunc("runCommand",    runCommand);
	ls.registerFunc("formatUI",    formatUI);
	ls.registerFunc("formatDB",    formatDB);
	ls.registerFunc("deleteUI",    deleteUI);
	ls.registerFunc("deleteReflectable",    deleteReflectable);
	ls.registerFunc("dumpUI",    dumpUI);
	ls.registerFunc("setKeyboardContext",    setKeyboardContext);
	ls.registerFunc("breakPoint",    breakPoint);
	ls.registerFunc("getWindowSize",    getWindowSize);
	ls.registerFunc("setTextFormatTaged",    setTextFormatTaged);
	ls.registerFunc("initEmotesMenu", initEmotesMenu);
	ls.registerFunc("isUCString", isUCString);
	ls.registerFunc("hideAllWindows", hideAllWindows);
	ls.registerFunc("hideAllNonSavableWindows", hideAllNonSavableWindows);
	ls.registerFunc("getDesktopIndex", getDesktopIndex);
	ls.registerFunc("setLuaBreakPoint", setLuaBreakPoint);
	ls.registerFunc("getMainPageURL", getMainPageURL);
	ls.registerFunc("getCharSlot", getCharSlot);
	ls.registerFunc("getPathContent", getPathContent);
	ls.registerFunc("getServerSeason", getServerSeason);
	ls.registerFunc("computeCurrSeason", computeCurrSeason);
	ls.registerFunc("getAutoSeason", getAutoSeason);
	ls.registerFunc("getTextureSize", getTextureSize);
	ls.registerFunc("enableModalWindow", enableModalWindow);
	ls.registerFunc("disableModalWindow", disableModalWindow);
	ls.registerFunc("getPlayerPos", getPlayerPos);
	ls.registerFunc("displaySystemInfo", displaySystemInfo);
	ls.registerFunc("disableContextHelpForControl", disableContextHelpForControl);
	ls.registerFunc("disableContextHelp", disableContextHelp);
	ls.registerFunc("setWeatherValue", setWeatherValue);
	ls.registerFunc("getWeatherValue", getWeatherValue);
	ls.registerFunc("getCompleteIslands", getCompleteIslands);
	ls.registerFunc("getIslandId", getIslandId);
	ls.registerFunc("getClientCfgVar", getClientCfgVar);
	ls.registerFunc("isPlayerFreeTrial", isPlayerFreeTrial);
	ls.registerFunc("isPlayerNewbie", isPlayerNewbie);
	ls.registerFunc("isInRingMode", isInRingMode);
	ls.registerFunc("getUserRace",  getUserRace);
	// Through LUABind API
	lua_State	*L= ls.getStatePointer();

	luabind::module(L)
	[
		LUABIND_FUNC(getDbProp),
		LUABIND_FUNC(setDbProp),
		LUABIND_FUNC(debugInfo),
		LUABIND_FUNC(rawDebugInfo),
		LUABIND_FUNC(dumpCallStack),
		LUABIND_FUNC(getDefine),
		LUABIND_FUNC(setContextHelpText),
		luabind::def("messageBox",    (void(*)(const ucstring &)) &messageBox),
		luabind::def("messageBox",    (void(*)(const ucstring &, const std::string &)) &messageBox),
		luabind::def("messageBox",    (void(*)(const ucstring &, const std::string &, int caseMode)) &messageBox),
		luabind::def("messageBox",    (void(*)(const std::string &)) &messageBox),
		luabind::def("messageBoxWithHelp",    (void(*)(const ucstring &)) &messageBoxWithHelp),
		luabind::def("messageBoxWithHelp",    (void(*)(const ucstring &, const std::string &)) &messageBoxWithHelp),
		luabind::def("messageBoxWithHelp",    (void(*)(const ucstring &, const std::string &, int caseMode)) &messageBoxWithHelp),
		luabind::def("messageBoxWithHelp",    (void(*)(const std::string &)) &messageBoxWithHelp),
		luabind::def("findReplaceAll",    (std::string(*)(const std::string &,  const std::string &,  const std::string &)) &findReplaceAll),
		luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const ucstring &,  const ucstring &)) &findReplaceAll),
		luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const std::string &,  const std::string &)) &findReplaceAll),
		luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const ucstring &,  const std::string &)) &findReplaceAll),
		luabind::def("findReplaceAll",    (ucstring(*)(const ucstring &,  const std::string &,  const ucstring &)) &findReplaceAll),
		LUABIND_FUNC(getPlayerSelectedSlot),
		LUABIND_FUNC(isInGame),
		LUABIND_FUNC(pauseBGDownloader),
		LUABIND_FUNC(unpauseBGDownloader),
		LUABIND_FUNC(requestBGDownloaderPriority),
		LUABIND_FUNC(getBGDownloaderPriority),
		LUABIND_FUNC(getPatchLastErrorMessage),
		LUABIND_FUNC(isPlayerSlotNewbieLand),
		LUABIND_FUNC(getSkillIdFromName),
		LUABIND_FUNC(getSkillLocalizedName),
		LUABIND_FUNC(getMaxSkillValue),
		LUABIND_FUNC(getBaseSkillValueMaxChildren),
		LUABIND_FUNC(getMagicResistChance),
		LUABIND_FUNC(getDodgeParryChance),
		LUABIND_FUNC(browseNpcWebPage),
		LUABIND_FUNC(clearHtmlUndoRedo),
		LUABIND_FUNC(getDynString),
		LUABIND_FUNC(isDynStringAvailable),
		LUABIND_FUNC(isFullyPatched),
		LUABIND_FUNC(getSheetType),
		LUABIND_FUNC(getSheetName),
		LUABIND_FUNC(getFameIndex),
		LUABIND_FUNC(getFameName),
		LUABIND_FUNC(getFameDBIndex),
		LUABIND_FUNC(getFirstTribeFameIndex),
		LUABIND_FUNC(getNbTribeFameIndex),
		LUABIND_FUNC(getClientCfg),
		LUABIND_FUNC(fileExists),
		LUABIND_FUNC(sendMsgToServer),
		LUABIND_FUNC(sendMsgToServerPvpTag),
		LUABIND_FUNC(isGuildQuitAvailable),
		LUABIND_FUNC(sortGuildMembers),
		LUABIND_FUNC(getNbGuildMembers),
		LUABIND_FUNC(getGuildMemberName),
		LUABIND_FUNC(getGuildMemberGrade),
		LUABIND_FUNC(isR2Player),
		LUABIND_FUNC(getR2PlayerRace),
		LUABIND_FUNC(isR2PlayerMale),
		LUABIND_FUNC(getCharacterSheetSkel),
		LUABIND_FUNC(getSheetId),
		LUABIND_FUNC(getCharacterSheetRegionForce),
		LUABIND_FUNC(getCharacterSheetRegionLevel),
		LUABIND_FUNC(replacePvpEffectParam),
		LUABIND_FUNC(getRegionByAlias),
		LUABIND_FUNC(tell),
		LUABIND_FUNC(isRingAccessPointInReach),
		LUABIND_FUNC(updateTooltipCoords),
		LUABIND_FUNC(secondsSince1970ToHour),
		LUABIND_FUNC(isCtrlKeyDown),
		LUABIND_FUNC(encodeURLUnicodeParam),

	#if !FINAL_VERSION
		LUABIND_FUNC(openDoc),
		LUABIND_FUNC(launchProgram),
	#endif

		luabind::def("fileLookup",    CMiscFunctions::fileLookup),
		luabind::def("shellExecute",  CMiscFunctions::shellExecute),

		LUABIND_FUNC(getPlayerLevel),
		LUABIND_FUNC(getTargetLevel),
		LUABIND_FUNC(getTargetForceRegion),
		LUABIND_FUNC(getTargetLevelForce),
		LUABIND_FUNC(isTargetNPC),
		LUABIND_FUNC(isTargetPlayer), // return 'true' if the target is an npc
		LUABIND_FUNC(isTargetUser),
		LUABIND_FUNC(isPlayerInPVPMode),
		LUABIND_FUNC(isTargetInPVPMode)
	];

	LUABIND_ENUM(PVP_CLAN::TPVPClan, "game.TPVPClan", PVP_CLAN::NbClans, PVP_CLAN::toString);
	LUABIND_ENUM(BONUS_MALUS::TBonusMalusSpecialTT, "game.TBonusMalusSpecialTT", BONUS_MALUS::NbSpecialTT, BONUS_MALUS::toString);

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
void	CLuaIHM::pushUIOnStack(CLuaState &ls,    class CInterfaceElement *pIE)
{
	//H_AUTO(Lua_CLuaIHM_pushUIOnStack)
	pushReflectableOnStack(ls,    pIE);
}

// ***************************************************************************
bool	CLuaIHM::isUIOnStack(CLuaState &ls,    sint index)
{
	//H_AUTO(Lua_CLuaIHM_isUIOnStack)
	return getUIOnStack(ls,    index) != NULL;
}

// ***************************************************************************
CInterfaceElement	*CLuaIHM::getUIOnStack(CLuaState &ls,    sint index)
{
	//H_AUTO(Lua_CLuaIHM_getUIOnStack)
	return dynamic_cast<CInterfaceElement *>(getReflectableOnStack(ls,    index));
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
sint32	CLuaIHM::getDbProp(const std::string &dbProp)
{
	//H_AUTO(Lua_CLuaIHM_getDbProp)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= pIM->getDbProp(dbProp,    false);
	if(node)
		return node->getValue32();
	else
	{
		debugInfo(toString("getDbProp(): '%s' dbProp Not found",    dbProp.c_str()));
		return 0;
	}
}

void	CLuaIHM::setDbProp(const std::string &dbProp,    sint32 value)
{
	//H_AUTO(Lua_CLuaIHM_setDbProp)
	// Do not allow Write on SERVER: or LOCAL:
	static const std::string	dbServer= "SERVER:";
	static const std::string	dbLocal= "LOCAL:";
	static const std::string	dbLocalR2= "LOCAL:R2";
	if( (0==dbProp.compare(0,    dbServer.size(),    dbServer)) ||
		(0==dbProp.compare(0,    dbLocal.size(),    dbLocal))
		)
	{
		if (0!=dbProp.compare(0,    dbLocalR2.size(),    dbLocalR2))
		{
			nlstop;
			throw ELuaIHMException("setDbProp(): You are not allowed to write on 'SERVER:...' or 'LOCAL:...' database");
		}
	}

	// Write to the DB if found
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= pIM->getDbProp(dbProp,    false);
	if(node)
		node->setValue32(value);
	else
		debugInfo(toString("setDbProp(): '%s' dbProp Not found",    dbProp.c_str()));
}


// ***************************************************************************
void		CLuaIHM::debugInfo(const std::string &cstDbg)
{
	//H_AUTO(Lua_CLuaIHM_debugInfo)
	if(ClientCfg.DisplayLuaDebugInfo)
	{
		std::string dbg = cstDbg;
		if (ClientCfg.LuaDebugInfoGotoButtonEnabled)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			lua_State *ls = pIM->getLuaState()->getStatePointer();
			lua_Debug	luaDbg;
			if(lua_getstack (ls,     1,     &luaDbg))
			{
				if(lua_getinfo(ls,     "lS",     &luaDbg))
				{
					// add a command button to jump to the wanted file
					dbg = createGotoFileButtonTag(luaDbg.short_src, luaDbg.currentline) + dbg;
				}
			}
		}
		rawDebugInfo(dbg);
	}
}

// ***************************************************************************
void CLuaIHM::rawDebugInfo(const std::string &dbg)
{
	//H_AUTO(Lua_CLuaIHM_rawDebugInfo)
	if(ClientCfg.DisplayLuaDebugInfo)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		if (!dbg.empty() && dbg[0] == '@')
		{
			// if color is already given use the message as it
			NLMISC::InfoLog->displayRawNL(dbg.c_str());
		}
		else
		{
			NLMISC::InfoLog->displayRawNL(pIM->formatLuaErrorSysInfo(dbg).c_str());
		}
		#ifdef LUA_NEVRAX_VERSION
			if (LuaDebuggerIDE)
			{
				LuaDebuggerIDE->debugInfo(dbg.c_str());
			}
		#endif
		pIM->displaySystemInfo(pIM->formatLuaErrorSysInfo(dbg));
	}
}

// ***************************************************************************
int CLuaIHM::displaySystemInfo(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_displaySystemInfo)
	const char *funcName = "displaySystemInfo";
	checkArgCount(ls, funcName, 2);
	checkArgTypeUCString(ls, funcName, 1);
	checkArgType(ls, funcName, 2, LUA_TSTRING);
	ucstring msg;
	nlverify(getUCStringOnStack(ls, 1, msg));
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->displaySystemInfo(msg, ls.toString(2));
	return 0;
}

// ***************************************************************************
int CLuaIHM::setWeatherValue(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setWeatherValue)
	const char *funcName = "setWeatherValue";
	checkArgMin(ls, funcName, 1);
	checkArgMax(ls, funcName, 2);
	checkArgType(ls, funcName, 1, LUA_TBOOLEAN);
//	bool autoWeather = ls.toBoolean(1);
	ClientCfg.ManualWeatherSetup = !ls.toBoolean(1);
	if (ls.getTop() == 2)
	{
		checkArgType(ls, funcName, 2, LUA_TNUMBER);
		ManualWeatherValue = (float) ls.toNumber(2);
	}
	return 0;
}

// ***************************************************************************
int CLuaIHM::getWeatherValue(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getWeatherValue)
	const char *funcName = "getWeatherValue";
	checkArgCount(ls, funcName, 0);
	uint64 currDay = RT.getRyzomDay();
	float currHour = (float) RT.getRyzomTime();
	ls.push(::getBlendedWeather(currDay, currHour, *WeatherFunctionParams, ContinentMngr.cur()->WeatherFunction));
	return 1;
}

void CLuaIHM::dumpCallStack(int startStackLevel)
{
	//H_AUTO(Lua_CLuaIHM_dumpCallStack)
	if(ClientCfg.DisplayLuaDebugInfo)
	{
		lua_Debug	dbg;
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		lua_State *ls = pIM->getLuaState()->getStatePointer();
		int stackLevel = startStackLevel;
		rawDebugInfo("Call stack : ");
		rawDebugInfo("-------------");
		while (lua_getstack (ls,   stackLevel,   &dbg))
		{
			if(lua_getinfo(ls,   "lS",   &dbg))
			{
				std::string result = createGotoFileButtonTag(dbg.short_src,   dbg.currentline) + NLMISC::toString("%s:%d:",   dbg.short_src,   dbg.currentline);
				rawDebugInfo(result);
			}
			++ stackLevel;
		}
	}
}

// ***************************************************************************
void CLuaIHM::getCallStackAsString(int startStackLevel /*=0*/,std::string &result)
{
	//H_AUTO(Lua_CLuaIHM_getCallStackAsString)
	result.clear();
	lua_Debug	dbg;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	lua_State *ls = pIM->getLuaState()->getStatePointer();
	int stackLevel = startStackLevel;
	result += "Call stack : \n";
	result += "-------------";
	while (lua_getstack (ls,   stackLevel,   &dbg))
	{
		if(lua_getinfo(ls,   "lS",   &dbg))
		{
			result += NLMISC::toString("%s:%d:",   dbg.short_src,   dbg.currentline);
		}
		++ stackLevel;
	}
}

// ***************************************************************************
std::string	CLuaIHM::createGotoFileButtonTag(const char *fileName, uint line)
{
	//H_AUTO(Lua_CLuaIHM_createGotoFileButtonTag)
	if (ClientCfg.LuaDebugInfoGotoButtonEnabled)
	{
		// TODO nico : put this in the interface
		// add a command button to jump to the wanted file
		return toString("/$$%s|%s|lua|%s('%s',   %d)$$/",
					   ClientCfg.LuaDebugInfoGotoButtonTemplate.c_str(),
					   ClientCfg.LuaDebugInfoGotoButtonCaption.c_str(),
					   ClientCfg.LuaDebugInfoGotoButtonFunction.c_str(),
					   fileName,
					   line
					  );
	}
	return "";
}

// ***************************************************************************
std::string	CLuaIHM::getDefine(const std::string &def)
{
	//H_AUTO(Lua_CLuaIHM_getDefine)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ClientCfg.DisplayLuaDebugInfo && !pIM->isDefineExist(def))
		debugInfo(toString("getDefine(): '%s' not found",    def.c_str()));
	return pIM->getDefine(def);
}



// ***************************************************************************
static CEntityCL *getTargetSlot()
{
	const char *dbPath = "UI:VARIABLES:TARGET:SLOT";
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCDBNodeLeaf *node = im->getDbProp(dbPath, false);
	if (!node) return NULL;
	if ((uint8) node->getValue32() == (uint8) CLFECOMMON::INVALID_SLOT)
	{
		return NULL;
	}
	return EntitiesMngr.entity((uint) node->getValue32());		
}

// ***************************************************************************
sint32 CLuaIHM::getPlayerLevel()
{
	if (!UserEntity) return -1;
	CSkillManager	*pSM= CSkillManager::getInstance();
	uint32 maxskill = pSM->getBestSkillValue(SKILLS::SC);
	maxskill = std::max(maxskill, pSM->getBestSkillValue(SKILLS::SF));
	maxskill = std::max(maxskill, pSM->getBestSkillValue(SKILLS::SH));
	maxskill = std::max(maxskill, pSM->getBestSkillValue(SKILLS::SM));
	return sint32(maxskill);
}

// ***************************************************************************
sint32 CLuaIHM::getTargetLevel()
{
	CEntityCL *target = getTargetSlot();
	if (!target) return -1;
	if ( target->isPlayer() )
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDbPlayerLevel = pIM->getDbProp( pIM->getDefine("target_player_level") );
		return pDbPlayerLevel ? pDbPlayerLevel->getValue32() : -1;
	}
	else
	{
		CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(target->sheetId()));
		if(!pCS) return -1;
		// only display the consider if the target is attackable #523
		if(!pCS->Attackable) return -1;
		if(!target->properties().attackable()) return -1;
		return sint32(pCS->Level);
	}
	return -1;
}

// ***************************************************************************
sint32 CLuaIHM::getTargetForceRegion()
{	
	CEntityCL *target = getTargetSlot();
	if (!target) return -1;
	if ( target->isPlayer() )
	{			
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDbPlayerLevel = pIM->getDbProp( pIM->getDefine("target_player_level") );			
		if (!pDbPlayerLevel) return -1;
		sint nLevel = pDbPlayerLevel->getValue32();
		if ( nLevel < 250 )
		{				
			return (sint32) ((nLevel < 20) ? 1 : (nLevel / 50) + 2);
		}
		else
		{				
			return 8;
		}
	}
	else
	{			
		CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(target->sheetId()));
		return pCS ? (sint32) pCS->RegionForce : -1;
	}		
	return 0;
}

// ***************************************************************************
sint32 CLuaIHM::getTargetLevelForce()
{
	CEntityCL *target = getTargetSlot();
	if (!target) return -1;
	if ( target->isPlayer() )
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDbPlayerLevel = pIM->getDbProp( pIM->getDefine("target_player_level") );
		if (!pDbPlayerLevel) return -1;
		sint nLevel = pDbPlayerLevel->getValue32();
		if ( nLevel < 250 )
		{
			return (sint32) (((nLevel % 50) * 5 / 50) + 1);
		}
		else
		{
			return 6;
		}
	}
	else
	{
		CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(target->sheetId()));
		return pCS ? (sint32) pCS->ForceLevel : -1;
	}
	return 0;
}

// ***************************************************************************
bool CLuaIHM::isTargetNPC()
{
	CEntityCL *target = getTargetSlot();
	if (!target) return false;
	return target->isNPC();
}

// ***************************************************************************
bool CLuaIHM::isTargetPlayer()
{
	CEntityCL *target = getTargetSlot();
	if (!target) return false;
	return target->isPlayer();
}


// ***************************************************************************
bool CLuaIHM::isTargetUser()
{
	CEntityCL *target = getTargetSlot();
	if (!target) return false;
	return target->isUser();
}

// ***************************************************************************
bool CLuaIHM::isPlayerInPVPMode()
{
	if (!UserEntity) return false;
	return (UserEntity->getPvpMode() & PVP_MODE::PvpZoneFaction)  != 0;
}

// ***************************************************************************
bool CLuaIHM::isTargetInPVPMode()
{
	CEntityCL *target = getTargetSlot();
	if (!target) return false;
	return (target->getPvpMode() & PVP_MODE::PvpZoneFaction)  != 0;
}

// ***************************************************************************
void CLuaIHM::pauseBGDownloader()
{
	::pauseBGDownloader();
}

// ***************************************************************************
void CLuaIHM::unpauseBGDownloader()
{
	::unpauseBGDownloader();
}

// ***************************************************************************
void CLuaIHM::requestBGDownloaderPriority(uint priority)
{
	if (priority >= BGDownloader::ThreadPriority_Count)
	{
		throw NLMISC::Exception("requestBGDownloaderPriority() : invalid priority");
	}
	CBGDownloaderAccess::getInstance().requestDownloadThreadPriority((BGDownloader::TThreadPriority) priority, false);
}

// ***************************************************************************
sint CLuaIHM::getBGDownloaderPriority()
{
	return CBGDownloaderAccess::getInstance().getDownloadThreadPriority();
}

// ***************************************************************************
ucstring CLuaIHM::getPatchLastErrorMessage()
{
	if (isBGDownloadEnabled())
	{
		return CBGDownloaderAccess::getInstance().getLastErrorMessage();
	}
	else
	{
		CPatchManager *pPM = CPatchManager::getInstance();
		return pPM->getLastErrorMessage();
	}
}

// ***************************************************************************
bool CLuaIHM::isInGame()
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	return pIM->isInGame();
}

// ***************************************************************************
uint32 CLuaIHM::getPlayerSelectedSlot()
{
	return (uint32) PlayerSelectedSlot;
}

// ***************************************************************************
bool CLuaIHM::isPlayerSlotNewbieLand(uint32 slot)
{
	if (slot > CharacterSummaries.size())
	{
		throw ELuaIHMException("isPlayerSlotNewbieLand(): Invalid slot %d",  (int) slot);
	}
	return CharacterSummaries[slot].InNewbieland;
}


// ***************************************************************************
void		CLuaIHM::messageBox(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text);
}

// ***************************************************************************
void		CLuaIHM::messageBox(const ucstring &text, const std::string &masterGroup)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text, masterGroup);
}

// ***************************************************************************
void		CLuaIHM::messageBox(const ucstring &text, const std::string &masterGroup, int caseMode)
{
	if (caseMode < 0 || caseMode >= CaseCount)
	{
		throw ELuaIHMException("messageBox: case mode value is invalid.");
	}
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text, masterGroup, (TCaseMode) caseMode);
}

// ***************************************************************************
void		CLuaIHM::messageBox(const std::string &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	static volatile bool dumpCallStack = false;
	if (dumpCallStack)
	{
		CLuaIHM::dumpCallStack(0);
	}
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBox(text);
}

// ***************************************************************************
void		CLuaIHM::messageBoxWithHelp(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text);
}

// ***************************************************************************
void		CLuaIHM::messageBoxWithHelp(const ucstring &text, const std::string &masterGroup)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text, masterGroup);
}

// ***************************************************************************
void		CLuaIHM::messageBoxWithHelp(const ucstring &text, const std::string &masterGroup, int caseMode)
{
	if (caseMode < 0 || caseMode >= CaseCount)
	{
		throw ELuaIHMException("messageBoxWithHelp: case mode value is invalid.");
	}
	//H_AUTO(Lua_CLuaIHM_messageBox)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text, masterGroup, "" ,"", (TCaseMode) caseMode);
}

// ***************************************************************************
void		CLuaIHM::messageBoxWithHelp(const std::string &text)
{
	//H_AUTO(Lua_CLuaIHM_messageBox)
	static volatile bool dumpCallStack = false;
	if (dumpCallStack)
	{
		CLuaIHM::dumpCallStack(0);
	}
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->messageBoxWithHelp(text);
}

// ***************************************************************************
void		CLuaIHM::setContextHelpText(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_setContextHelpText)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->setContextHelpText(text);
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
int	CLuaIHM::getUICaller(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getUICaller)
	CLuaStackChecker lsc(&ls,    1);

	// params: none.
	// return: CInterfaceElement*  (nil if error)
	CInterfaceElement	*pIE= CHandlerLUA::getUICaller();
	if(!pIE)
	{
		ls.pushNil();
		debugInfo(toString("getUICaller(): No UICaller found. return Nil"));
	}
	else
	{
		pushUIOnStack(ls,    pIE);
	}
	return 1;
}

// ***************************************************************************
int CLuaIHM::getCurrentWindowUnder(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getCurrentWindowUnder)
	CLuaStackChecker lsc(&ls,    1);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceElement	*pIE= im->getCurrentWindowUnder();
	if(!pIE)
	{
		ls.pushNil();
		debugInfo(toString("getCurrentWindowUnder(): No UICaller found. return Nil"));
	}
	else
	{
		pushUIOnStack(ls,    pIE);
	}
	return 1;
}
int	CLuaIHM::getUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getUI)
	// params: "ui:interface:...".
	// return: CInterfaceElement*  (nil if error)
	const char *funcName = "getUI";
	check(ls,  ls.getTop() == 1 || ls.getTop() == 2, funcName);
	checkArgType(ls,   funcName, 1, LUA_TSTRING);
	bool verbose = true;
	if (ls.getTop() > 1)
	{
		checkArgType(ls,   funcName, 2, LUA_TBOOLEAN);
		verbose = ls.toBoolean(2);
	}

	// get the string
	std::string	eltStr;
	ls.toString(1,    eltStr);

	// return the element
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CInterfaceElement	*pIE= pIM->getElementFromId(eltStr);
	if(!pIE)
	{
		ls.pushNil();
		if (verbose)
		{
			std::string stackContext;
			ls.getStackContext(stackContext,   1);
			debugInfo(toString("%s : getUI(): '%s' not found",    stackContext.c_str(),   eltStr.c_str()));
		}
	}
	else
	{
		pushUIOnStack(ls,    pIE);
	}
	return 1;
}

// ***************************************************************************
int	CLuaIHM::getUIId(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getUIId)
	CLuaStackChecker lsc(&ls,    1);

	// params: CInterfaceElement*
	// return: "ui:interface:...". (empty if error)
	checkArgCount(ls,    "getUIId",    1);
	check(ls,   isUIOnStack(ls,   1),    "getUIId() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);

	// convert to id
	if(pIE)
		ls.push(pIE->getId());
	else
		ls.push("");

	return 1;
}

// ***************************************************************************
int	CLuaIHM::getIndexInDB(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getIndexInDB)
	CLuaStackChecker lsc(&ls,    1);

	// params: CDBCtrlSheet*
	// return: index in DB of a dbctrlsheet  (empty if error)
	checkArgCount(ls,    "getIndexInDB",    1);
	check(ls,   isUIOnStack(ls,   1),    "getIndexInDB() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	CDBCtrlSheet		*pCS= dynamic_cast<CDBCtrlSheet*>(pIE);

	// get the index in db
	if(pCS)
		ls.push((double)pCS->getIndexInDB());
	else
		ls.push(0.0);

	return 1;
}

// ***************************************************************************
int CLuaIHM::createGroupInstance(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_createGroupInstance)
	const char *funcName = "createGroupInstance";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);
	std::vector<std::pair<std::string, std::string> > templateParams;
	CLuaObject params;
	params.pop(ls);
	ENUM_LUA_TABLE(params, it)
	{
		if (!it.nextKey().isString())
		{
			nlwarning("%s : bad key encountered with type %s, string expected.", funcName, it.nextKey().getTypename());
			continue;
		}
		if (!it.nextValue().isString())
		{
			nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
			continue;
		}
		templateParams.push_back(std::pair<std::string, std::string>(it.nextKey().toString(), it.nextValue().toString())); // strange compilation bug here when I use std::make_pair ... :(
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *result = im->createGroupInstance(ls.toString(1), ls.toString(2), templateParams);
	if (!result)
	{
		ls.pushNil();
	}
	else
	{
		CLuaIHM::pushUIOnStack(ls, result);
	}
	return 1;
}

// ***************************************************************************
int CLuaIHM::createUIElement(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_createUIElement)
	const char *funcName = "addUIElement";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);
	std::vector<std::pair<std::string, std::string> > templateParams;
	CLuaObject params;
	params.pop(ls);
	ENUM_LUA_TABLE(params, it)
	{
		if (!it.nextKey().isString())
		{
			nlwarning("%s : bad key encountered with type %s, string expected.", funcName, it.nextKey().getTypename());
			continue;
		}
		if (!it.nextValue().isString())
		{
			nlwarning("%s : bad value encountered with type %s for key %s, string expected.", funcName, it.nextValue().getTypename(), it.nextKey().toString().c_str());
			continue;
		}
		templateParams.push_back(std::pair<std::string, std::string>(it.nextKey().toString(), it.nextValue().toString())); // strange compilation bug here when I use std::make_pair ... :(
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceElement *result = im->createUIElement(ls.toString(1), ls.toString(2), templateParams);
	if (!result)
	{
		ls.pushNil();
	}
	else
	{
		CLuaIHM::pushUIOnStack(ls, result);
	}
	return 1;
}

// ***************************************************************************
int CLuaIHM::getCompleteIslands(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getCompleteIslands)
	const char *funcName = "getCompleteIslands";
	CLuaIHM::checkArgCount(ls, funcName, 0);

	ls.newTable();
	CLuaObject result(ls);

	// load entryPoints
	CScenarioEntryPoints scenarioEntryPoints = CScenarioEntryPoints::getInstance();
	const CScenarioEntryPoints::TCompleteIslands& islands =  scenarioEntryPoints.getCompleteIslands();

	CScenarioEntryPoints::TCompleteIslands::const_iterator island(islands.begin()), lastIsland(islands.end());
	for( ; island != lastIsland ; ++island)
	{
		ls.newTable();
		CLuaObject islandTable(ls);
		islandTable.setValue("continent", island->Continent);
		islandTable.setValue("xmin", (double)island->XMin);
		islandTable.setValue("ymin", (double)island->YMin);
		islandTable.setValue("xmax", (double)island->XMax);
		islandTable.setValue("ymax", (double)island->YMax);

		ls.newTable();
		CLuaObject entrypointsTable(ls);

		for(uint e=0; e<island->EntryPoints.size(); e++)
		{
			const CScenarioEntryPoints::CShortEntryPoint & entryPoint = island->EntryPoints[e];
			ls.newTable();
			CLuaObject entrypointTable(ls);
			entrypointTable.setValue("x", (double)entryPoint.X);
			entrypointTable.setValue("y", (double)entryPoint.Y);

			entrypointsTable.setValue(entryPoint.Location, entrypointTable);
		}
		islandTable.setValue("entrypoints", entrypointsTable);

		result.setValue(island->Island, islandTable);
	}

	result.push();

	return 1;
}

// ***************************************************************************
int CLuaIHM::getIslandId(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getIslandId)
	const char *funcName = "getIslandId";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	check(ls,   ls.isString(1),    "getIslandId() requires a string in param 1");


	CScenarioEntryPoints scenarioEntryPoints = CScenarioEntryPoints::getInstance();
	uint32 id = scenarioEntryPoints.getIslandId(ls.toString(1));
	ls.push((double)id);

	return 1;
}

// ***************************************************************************
int CLuaIHM::launchContextMenuInGame(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_launchContextMenuInGame)
	CLuaStackChecker lsc(&ls);
	checkArgCount(ls,    "launchContextMenuInGame",    1);
	check(ls,   ls.isString(1),    "launchContextMenuInGame() requires a string in param 1");
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->launchContextMenuInGame(ls.toString(1));
	return 0;
}

// ***************************************************************************
int CLuaIHM::parseInterfaceFromString(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_parseInterfaceFromString)
	CLuaStackChecker lsc(&ls,    1);
	checkArgCount(ls,    "parseInterfaceFromString",    1);
	check(ls,   ls.isString(1),    "parseInterfaceFromString() requires a string in param 1");
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::vector<std::string> script(1);
	script[0] = ls.toString(1);
	ls.push(pIM->parseInterface(script,    true,    false));
	return 1;
}

// ***************************************************************************
int CLuaIHM::updateAllLocalisedElements(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_updateAllLocalisedElements)
	TTime startTime = CTime::getLocalTime();
	//
	CLuaStackChecker lsc(&ls);
	checkArgCount(ls,    "updateAllLocalisedElements",    0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->updateAllLocalisedElements();
	//
	TTime endTime = CTime::getLocalTime();
	if (ClientCfg.R2EDVerboseParseTime)
	{
		nlinfo("%.2f seconds for 'updateAllLocalisedElements'", (endTime - startTime) / 1000.f);
	}
	return 0;
}

// ***************************************************************************
int CLuaIHM::setCaptureKeyboard(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setCaptureKeyboard)
	const char *funcName = "setCaptureKeyboard";
	checkArgCount(ls, funcName, 1);
	checkArgTypeUIElement(ls, funcName, 1);
	CCtrlBase *ctrl = dynamic_cast<CCtrlBase *>(getUIOnStack(ls, 1));
	if (!ctrl)
	{
		fails(ls, "%s waits a ui control as arg 1", funcName);
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->setCaptureKeyboard(ctrl);
	return 0;
}

// ***************************************************************************
int CLuaIHM::resetCaptureKeyboard(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_resetCaptureKeyboard)
	const char *funcName = "resetCaptureKeyboard";
	checkArgCount(ls, funcName, 0);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->resetCaptureKeyboard();
	return 0;
}

// ***************************************************************************
int	CLuaIHM::setOnDraw(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setOnDraw)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceGroup*,    "script".
	// return: none
	checkArgCount(ls,    "setOnDraw",    2);
	check(ls,   isUIOnStack(ls,    1),    "setOnDraw() requires a UI object in param 1");
	check(ls,   ls.isString(2),    "setOnDraw() requires a string in param 2");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			script;
	ls.toString(2,    script);

	// must be a group
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
	if(!group)
		throw ELuaIHMException("setOnDraw(): '%s' is not a group",    pIE->getId().c_str());
	// Set the script to be executed at each draw
	group->setLuaScriptOnDraw(script);

	return 0;
}

// ***************************************************************************
int	CLuaIHM::addOnDbChange(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_addOnDbChange)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceGroup*,    "dblist",    "script".
	// return: none
	checkArgCount(ls,    "addOnDbChange",    3);
	check(ls,   isUIOnStack(ls,    1),    "addOnDbChange() requires a UI object in param 1");
	check(ls,   ls.isString(2),    "addOnDbChange() requires a string in param 2");
	check(ls,   ls.isString(3),    "addOnDbChange() requires a string in param 3");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			dbList,    script;
	ls.toString(2,    dbList);
	ls.toString(3,    script);

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
	checkArgCount(ls,    "removeOnDbChange",    2);
	check(ls,   isUIOnStack(ls,    1),    "removeOnDbChange() requires a UI object in param 1");
	check(ls,   ls.isString(2),    "removeOnDbChange() requires a string in param 2");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			dbList;
	ls.toString(2,    dbList);

	// must be a group
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(pIE);
	if(!group)
		throw ELuaIHMException("removeOnDbChange(): '%s' is not a group",    pIE->getId().c_str());
	// Remove the script to be executed when the given DB change
	group->removeLuaScriptOnDBChange(dbList);

	return 0;
}


// ***************************************************************************
int	CLuaIHM::runAH(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_runAH)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *,    "ah",    "params".
	// return: none
	checkArgCount(ls,    "runAH",    3);
	check(ls,   isUIOnStack(ls,    1) || ls.isNil(1),    "runAH() requires a UI object in param 1 (or Nil)");
	check(ls,   ls.isString(2),    "runAH() requires a string in param 2");
	check(ls,   ls.isString(3),    "runAH() requires a string in param 3");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	std::string			ah,    params;
	ls.toString(2,    ah);
	ls.toString(3,    params);

	// run AH
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// The element must be ctrl (or NULL)
	CCtrlBase	*ctrl= NULL;
	if(pIE)
	{
		ctrl= dynamic_cast<CCtrlBase*>(pIE);
		if(!ctrl)
			throw ELuaIHMException("runAH(): '%s' is not a ctrl",    pIE->getId().c_str());
	}
	pIM->runActionHandler(ah,    ctrl,    params);

	return 0;
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
			ls.push((double)value.getInteger());
			break;
		case CInterfaceExprValue::Double:
			ls.push(value.getDouble());
			break;
		case CInterfaceExprValue::String:
			{
				ucstring	ucstr= value.getUCString();
				// Yoyo: dynamically decide wether must return a string or a ucstring
				bool	mustUseUCString= false;
				for (uint i = 0; i < ucstr.size (); i++)
				{
					if (ucstr[i] > 255)
					{
						mustUseUCString= true;
						break;
					}
				}
				// push a ucstring?
				if(mustUseUCString)
				{
#if LUABIND_VERSION > 600
					luabind::detail::push(ls.getStatePointer(), ucstr);
#else
					luabind::object obj(ls.getStatePointer(), ucstr);
					obj.pushvalue();
#endif
				}
				else
				{
					ls.push(ucstr.toString());
				}
				break;
			}
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
	checkArgCount(ls,    "runExpr",    1);
	check(ls,   ls.isString(1),    "runExpr() requires a string in param 1");

	// retrieve args
	std::string expr;
	ls.toString(1,    expr);

	// run expression and push result
	return runExprAndPushResult(ls,    expr);
}

// ***************************************************************************
int		CLuaIHM::runFct(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_runFct)
	CLuaStackChecker lsc(&ls,    1);

	// params: "expr",    param1,    param2...
	// return: any of: nil,   bool,   string,   number,    RGBA,    UCString
	checkArgMin(ls,    "runFct",    1);
	check(ls,   ls.isString(1),    "runExpr() requires a string in param 1");

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

	ls.push(command->execute(rawCommandString,   args,    g_log,    false,    true));
	return 1;
}

// ***************************************************************************
int		CLuaIHM::formatUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_formatUI)
	CLuaStackChecker lsc(&ls,    1);

	// params: "expr",    param1,    param2....
	// return: string with # and % parsed
	checkArgMin(ls,    "formatUI",    1);
	check(ls,   ls.isString(1),    "formatUI() require a string in param1");

	// get the string to format
	std::string	propVal;
	ls.toString(1,    propVal);

	// *** format with %
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::string	newPropVal,    defError;
	if(!pIM->solveDefine(propVal,    newPropVal,    defError))
	{
		throw ELuaIHMException("formatUI(): Can't find define: '%s'",    defError.c_str());
	}

	// *** format with any additional parameter and #1,    #2,    #3 etc...
	// search backward,    starting from bigger param to replace (thus avoid to replace #1 before #13 for instance...)
	sint	stackIndex= ls.getTop();
	while(stackIndex>1)
	{
		std::string	paramValue;
		ls.toString(stackIndex,    paramValue);

		// For stack param 4,    the param index is 3 (because stack param 2 is the param No 1)
		sint	paramIndex= stackIndex-1;
		while(NLMISC::strFindReplace(newPropVal,    toString("#%d",    paramIndex),    paramValue));

		// next
		stackIndex--;
	}

	// return result
	ls.push(newPropVal);
	return 1;
}

// ***************************************************************************
int		CLuaIHM::formatDB(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_formatDB)
	CLuaStackChecker lsc(&ls,    1);

	// params: param1,    param2....
	// return: string with @ and ,    added
	checkArgMin(ls,    "formatDB",    1);
	uint	top= ls.getTop();

	std::string	dbRes;
	for(uint i=1;i<=top;i++)
	{
		if(i==1)
			dbRes= "@";
		else
			dbRes+= ",   @";

		std::string	paramValue;
		ls.toString(i,    paramValue);
		dbRes+= paramValue;
	}

	// return result
	ls.push(dbRes);
	return 1;
}

// ***************************************************************************
int	CLuaIHM::deleteUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_deleteUI)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *
	// return: none
	checkArgCount(ls,    "deleteUI",    1);
	check(ls,   isUIOnStack(ls,    1),    "deleteUI() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
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
	checkArgCount(ls,    "deleteReflectable",    1);
	check(ls,   isReflectableOnStack(ls,    1),    "deleteReflectable() requires a reflectable C++ object in param 1");

	// retrieve args
	CReflectableRefPtrTarget	*pRPT= getReflectableOnStack(ls,    1);
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

// ***************************************************************************
int	CLuaIHM::dumpUI(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_dumpUI)
	CLuaStackChecker lsc(&ls,    0);

	// params: CInterfaceElement *
	// return: none
	checkArgCount(ls,    "dumpUI",    1);
	check(ls,   isUIOnStack(ls,    1),    "dumpUI() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);
	if(!pIE)
		debugInfo("UI: NULL");
	else
	{
		// Display also Information on RefPtr (warning: don't modify pinfo!!!)
		nlassert(pIE->pinfo);
		debugInfo(NLMISC::toString("UI: %x. %s. RefPtrCount: %d",    pIE,    pIE->getId().c_str(),
			pIE->pinfo->IsNullPtrInfo?0:pIE->pinfo->RefCount));
	}

	return 0;
}

// ***************************************************************************
int	CLuaIHM::setKeyboardContext(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setKeyboardContext)
	const char *funcName = "setKeyboardContext";
	checkArgMin(ls, funcName, 1);
	checkArgType(ls, funcName, 1, LUA_TSTRING);

	ActionsContext.setContext(ls.toString(1));

	return 0;
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
void CLuaIHM::fails(CLuaState &ls, const std::string &format,...)
{
	//H_AUTO(Lua_CLuaIHM_fails)
	std::string	reason;
	const char *formatPtr = format.c_str();
	NLMISC_CONVERT_VARGS (reason, formatPtr, NLMISC::MaxCStringSize);
	fails(ls, reason.c_str());
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
		fails(ls, failReason);
	}
}

// ***************************************************************************
void CLuaIHM::checkArgType(CLuaState &ls,   const char *funcName,   uint index,   int argType)
{
	//H_AUTO(Lua_CLuaIHM_checkArgType)
	nlassert(index > 0);
	if (ls.getTop() < (int) index)
	{
		fails(ls, toString("%s : argument %d of expected type %s was not defined",   funcName,   index,   ls.getTypename(argType)));
	}
	if (ls.type(index) != argType)
	{
		fails(ls, toString("%s : argument %d of expected type %s has bad type : %s",   funcName,   index,   ls.getTypename(argType),   ls.getTypename(ls.type(index)),   ls.type(index)));
	}
}

// ***************************************************************************
void CLuaIHM::checkArgTypeRGBA(CLuaState &ls, const char *funcName, uint index)
{
	//H_AUTO(Lua_CLuaIHM_checkArgTypeRGBA)
	nlassert(index > 0);
	if (ls.getTop() < (int) index)
	{
		fails(ls, toString("%s : argument %d of expected type RGBA was not defined",   funcName,   index));
	}
	ls.pushValue(index);
	CRGBA dummy;
	if (!pop(ls, dummy))
	{
		fails(ls, toString("%s : argument %d of expected type RGBA has bad type : %s",   funcName,   index, ls.getTypename(ls.type(index)),   ls.type(index)));
	}
}

// ***************************************************************************
void CLuaIHM::checkArgTypeUIElement(CLuaState &ls, const char *funcName, uint index)
{
	//H_AUTO(Lua_CLuaIHM_checkArgTypeUIElement)
	nlassert(index > 0);
	if (ls.getTop() < (int) index)
	{
		fails(ls, toString("%s : argument %d of expected type ui element was not defined",   funcName,   index));
	}
	if (!isUIOnStack(ls, index))
	{
		fails(ls, toString("%s : argument %d of expected type ui element has bad type : %s",   funcName,   index, ls.getTypename(ls.type(index)),   ls.type(index)));
	}
}

// ***************************************************************************
void CLuaIHM::checkArgTypeUCString(CLuaState &ls, const char *funcName, uint index)
{
	//H_AUTO(Lua_CLuaIHM_checkArgTypeUCString)
	nlassert(index > 0);
	if (ls.getTop() < (int) index)
	{
		fails(ls, toString("%s : argument %d of expected type ucstring was not defined",   funcName,   index));
	}
	ls.pushValue(index);
	ucstring dummy;
	if (!pop(ls, dummy))
	{
		fails(ls, toString("%s : argument %d of expected type ucstring has bad type : %s",   funcName,   index,   ls.getTypename(ls.type(index)),   ls.type(index)));
	}
}


// ***************************************************************************
int CLuaIHM::validMessageBox(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_validMessageBox)
	const char *funcName = "validMessageBox";
	checkArgCount(ls, funcName, 6);
	ucstring msg;
	ls.pushValue(1); // copy ucstring at the end of stack to pop it
	check(ls, pop(ls, msg), "validMessageBox : ucstring wanted as first parameter");
	checkArgType(ls, funcName, 2, LUA_TSTRING);
	checkArgType(ls, funcName, 3, LUA_TSTRING);
	checkArgType(ls, funcName, 4, LUA_TSTRING);
	checkArgType(ls, funcName, 5, LUA_TSTRING);
	checkArgType(ls, funcName, 6, LUA_TSTRING);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->validMessageBox(CInterfaceManager::QuestionIconMsg, msg, ls.toString(2), ls.toString(3), ls.toString(4), ls.toString(5), ls.toString(6));
	return 0;
}

// ***************************************************************************
int CLuaIHM::setTopWindow(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setTopWindow)
	const char *funcName = "setTopWindow";
	checkArgCount(ls, funcName, 1);
	CInterfaceGroup *wnd = dynamic_cast<CInterfaceGroup *>(getUIOnStack(ls, 1));
	if (!wnd)
	{
		fails(ls, "%s : interface group expected as arg 1", funcName);
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->setTopWindow(wnd);
	return 0;
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
			nlverify(getUCStringOnStack(ls, k, part));
		}
		result += part;
	}
	push(ls, result);
	return 1;
}

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

// ***************************************************************************
sint32	CLuaIHM::getSkillIdFromName(const std::string &def)
{
	//H_AUTO(Lua_CLuaIHM_getSkillIdFromName)
	SKILLS::ESkills	e= SKILLS::toSkill(def);
	// Avoid any bug,    return SF if not found
	if(e>=SKILLS::unknown)
		e= SKILLS::SF;
	return e;
}

// ***************************************************************************
ucstring	CLuaIHM::getSkillLocalizedName(sint32 skillId)
{
	//H_AUTO(Lua_CLuaIHM_getSkillLocalizedName)
	return ucstring(STRING_MANAGER::CStringManagerClient::getSkillLocalizedName((SKILLS::ESkills)skillId));
}

// ***************************************************************************
sint32	CLuaIHM::getMaxSkillValue(sint32 skillId)
{
	//H_AUTO(Lua_CLuaIHM_getMaxSkillValue)
	CSkillManager	*pSM= CSkillManager::getInstance();
	return pSM->getMaxSkillValue((SKILLS::ESkills)skillId);
}

// ***************************************************************************
sint32	CLuaIHM::getBaseSkillValueMaxChildren(sint32 skillId)
{
	//H_AUTO(Lua_CLuaIHM_getBaseSkillValueMaxChildren)
	CSkillManager	*pSM= CSkillManager::getInstance();
	return pSM->getBaseSkillValueMaxChildren((SKILLS::ESkills)skillId);
}

// ***************************************************************************
bool CLuaIHM::executeFunctionOnStack(CLuaState &ls,   int numArgs,   int numRet)
{
	//H_AUTO(Lua_CLuaIHM_executeFunctionOnStack)
	static volatile bool dumpFunction = false;
	if (dumpFunction)
	{
		CLuaStackRestorer lsr(&ls, ls.getTop());
		lua_Debug ar;
		ls.pushValue(-1 - numArgs);
		lua_getinfo (ls.getStatePointer(), ">lS", &ar);
		nlwarning((std::string(ar.what) + ", at line " + toString(ar.linedefined) + " in " + std::string(ar.source)).c_str());
	}
	int result = ls.pcall(numArgs,   numRet);
	switch (result)
	{
		case LUA_ERRRUN:
		case LUA_ERRMEM:
		case LUA_ERRERR:
		{
			CLuaIHM::debugInfo(ls.toString(-1));
			ls.pop();
			return false;
		}
		break;
		case 0:
			return true;
		break;
		default:
			nlassert(0);
		break;
	}
	return false;
}


// ***************************************************************************
int CLuaIHM::breakPoint(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_breakPoint)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::string reason;
	ls.getStackContext(reason,   1);		// 1 because 0 is the current C function => return 1 for script called
	pIM->formatLuaStackContext(reason);
	NLMISC::InfoLog->displayRawNL(reason.c_str());
	static volatile bool doAssert = true;
	if (doAssert) // breakPoint can be discarded in case of looping assert
	{
		NLMISC_BREAKPOINT;
	}
	return 0;
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
	catch(luabind::cast_failed &)
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
	catch(luabind::cast_failed &)
	{
		return false;
	}
	return true;
}


// ***************************************************************************
int CLuaIHM::getWindowSize(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getWindowSize)
	checkArgCount(ls,   "getWindowSize",   0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint32 w,   h;
	pIM->getViewRenderer().getScreenSize(w,   h);
	ls.push((double) w);
	ls.push((double) h);
	return 2;
}


// ***************************************************************************
int	CLuaIHM::setTextFormatTaged(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setTextFormatTaged)
	// params: CViewText*,    "text" (or ucstring)
	// return: none
	checkArgCount(ls,    "setTextFormatTaged",    2);

	// *** check and retrieve param 1
	check(ls,   isUIOnStack(ls,    1),    "setTextFormatTaged() requires a UI object in param 1");
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);

	// *** check and retrieve param 2. must be a string or a ucstring
	ucstring	text;
	if(ls.isString(2))
	{
		std::string			str;
		ls.toString(2,    str);
		text= str;
	}
	else
	{
		// try to pop a ucstring from the stack
		// fail?
		if(!pop(ls, text))
		{
			check(ls,   false,    "setTextFormatTaged() requires a string or a ucstring in param 2");
		}
	}

	// must be a view text
	CViewText	*vt= dynamic_cast<CViewText*>(pIE);
	if(!vt)
		throw ELuaIHMException("setTextFormatTaged(): '%s' is not a CViewText",    pIE->getId().c_str());

	// Set the text as format
	vt->setTextFormatTaged(text);

	return 0;
}

// ***************************************************************************
sint32	CLuaIHM::getMagicResistChance(bool elementalSpell,   sint32 casterSpellLvl,   sint32 victimResistLvl)
{
	//H_AUTO(Lua_CLuaIHM_getMagicResistChance)
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	casterSpellLvl= std::max(casterSpellLvl,   sint32(0));
	victimResistLvl= std::max(victimResistLvl,   sint32(0));
	/*  The success rate in the table is actually the "Casting Success Chance".
		Thus,   the relativeLevel is casterSpellLvl - victimResistLvl
		Moreover,   must take the "PartialSuccessMaxDraw" line because the spell is not resisted if success>0
	*/
	sint32	chanceToHit= pPM->getSuccessRate(elementalSpell?CSPhraseManager::STResistMagic:CSPhraseManager::STResistMagicLink,
		casterSpellLvl-victimResistLvl,   true);
	clamp(chanceToHit,   0,   100);

	// Thus,   the resist chance is 100 - hit chance.
	return 100 - chanceToHit;
}

// ***************************************************************************
sint32	CLuaIHM::getDodgeParryChance(sint32 attLvl, sint32 defLvl)
{
	//H_AUTO(Lua_CLuaIHM_getDodgeParryChance)
	CSPhraseManager	*pPM = CSPhraseManager::getInstance();
	attLvl= std::max(attLvl, sint32(0));
	defLvl= std::max(defLvl, sint32(0));

	sint32 chance = pPM->getSuccessRate(CSPhraseManager::STDodgeParry, defLvl-attLvl, false);
	clamp(chance, 0, 100);

	return chance;
}

// ***************************************************************************
void	CLuaIHM::browseNpcWebPage(const std::string &htmlId, const std::string &urlIn, bool addParameters, double timeout)
{
	//H_AUTO(Lua_CLuaIHM_browseNpcWebPage)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CGroupHTML	*groupHtml= dynamic_cast<CGroupHTML*>(pIM->getElementFromId(htmlId));
	if(groupHtml)
	{
		// if true, it means that we want to display a web page that use webig auth
		bool webig = urlIn.find("http://") == 0;

		string	url;
		// append the WebServer to the url
		if (urlIn.find("ring_access_point=1") != std::string::npos)
		{
			url = RingMainURL + "?" + urlIn;
		}
		else if(webig)
		{
			url = urlIn;
		}
		else
		{
			url = WebServer + urlIn;
		}

		if (addParameters && !webig)
		{
			// append shardid, playername and language code
			string userName;
			string guildName;
			if(UserEntity)
			{
				userName = UserEntity->getDisplayName ().toString();
				STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
				ucstring ucsTmp;
				pSMC->getString (UserEntity->getGuildNameID(), ucsTmp);
				guildName = ucsTmp.toString();

				while (guildName.find(' ') != string::npos)
				{
					guildName[guildName.find(' ')] = '_';
				}
			}

			url += ((url.find('?') != string::npos) ? "&" : "?") +
					string("shard=") + toString(ShardId) +
					string("&user_login=") + userName +
					string("&lang=") + ClientCfg.getHtmlLanguageCode() +
					string("&guild_name=") + guildName;
		}
/*
		if(webig)
		{
			// append special webig auth params
			addWebIGParams(url);
		}
*/
		// set the wanted timeout
		groupHtml->setTimeout((float)std::max(0.0, timeout));

		// Browse the url
		groupHtml->clean();
		groupHtml->browse(url.c_str());
		// Set top of the page
		CCtrlScroll *pScroll = groupHtml->getScrollBar();
		if (pScroll != NULL)
			pScroll->moveTrackY(10000);
	}
}

// ***************************************************************************
void		CLuaIHM::clearHtmlUndoRedo(const std::string &htmlId)
{
	//H_AUTO(Lua_CLuaIHM_clearHtmlUndoRedo)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CGroupHTML	*groupHtml= dynamic_cast<CGroupHTML*>(pIM->getElementFromId(htmlId));
	if(groupHtml)
		groupHtml->clearUndoRedo();
}

// ***************************************************************************
ucstring	CLuaIHM::getDynString(sint32 dynStringId)
{
	//H_AUTO(Lua_CLuaIHM_getDynString)
	ucstring result;
	STRING_MANAGER::CStringManagerClient::instance()->getDynString(dynStringId,   result);
	return result;
}

// ***************************************************************************
bool		CLuaIHM::isDynStringAvailable(sint32 dynStringId)
{
	//H_AUTO(Lua_CLuaIHM_isDynStringAvailable)
	ucstring result;
	bool res = STRING_MANAGER::CStringManagerClient::instance()->getDynString(dynStringId,   result);
	return res;
}

// ***************************************************************************
bool CLuaIHM::isFullyPatched()
{
	return AvailablePatchs == 0;
}

// ***************************************************************************
std::string CLuaIHM::getSheetType(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getSheetType)
	const CEntitySheet *sheetPtr = SheetMngr.get(CSheetId(sheet));
	if (!sheetPtr) return "";
	return CEntitySheet::typeToString(sheetPtr->Type);
}


// ***************************************************************************
std::string CLuaIHM::getSheetName(uint32 sheetId)
{
	return CSheetId(sheetId).toString();
}

// ***************************************************************************
sint32	CLuaIHM::getFameIndex(const std::string &factionName)
{
	//H_AUTO(Lua_CLuaIHM_getFameIndex)
	return CStaticFames::getInstance().getFactionIndex(factionName);
}

// ***************************************************************************
std::string	CLuaIHM::getFameName(sint32 fameIndex)
{
	//H_AUTO(Lua_CLuaIHM_getFameName)
	return CStaticFames::getInstance().getFactionName(fameIndex);
}

// ***************************************************************************
sint32	CLuaIHM::getFameDBIndex(sint32 fameIndex)
{
	//H_AUTO(Lua_CLuaIHM_getFameDBIndex)
	// Yoyo: avoid crash if fames not initialized
	if(CStaticFames::getInstance().getNbFame()==0)
		return 0;
	else
		return CStaticFames::getInstance().getDatabaseIndex(fameIndex);
}

// ***************************************************************************
sint32	CLuaIHM::getFirstTribeFameIndex()
{
	//H_AUTO(Lua_CLuaIHM_getFirstTribeFameIndex)
	return CStaticFames::getInstance().getFirstTribeFameIndex();
}

// ***************************************************************************
sint32	CLuaIHM::getNbTribeFameIndex()
{
	//H_AUTO(Lua_CLuaIHM_getNbTribeFameIndex)
	// Yoyo: avoid crash if fames not initialized. at leasst one tribe
	return std::max(1U, CStaticFames::getInstance().getNbTribeFameIndex());
}

// ***************************************************************************
bool CLuaIHM::fileExists(const string &fileName)
{
	//H_AUTO(Lua_CLuaIHM_fileExists)
	return CPath::exists(fileName);
}

// ***************************************************************************
string CLuaIHM::getClientCfg(const string &varName)
{
	//H_AUTO(Lua_CLuaIHM_getClientCfg)
	return ClientCfg.readString(varName);
}

// ***************************************************************************
void CLuaIHM::sendMsgToServer(const std::string &sMsg)
{
	//H_AUTO(Lua_CLuaIHM_sendMsgToServer)
	::sendMsgToServer(sMsg);
}

// ***************************************************************************
void CLuaIHM::sendMsgToServerPvpTag(bool pvpTag)
{
	//H_AUTO(Lua_CLuaIHM_sendMsgToServerPvpTag)
	uint8 tag = (uint8)pvpTag;
	::sendMsgToServer("PVP:PVP_TAG", tag);
}


// ***************************************************************************
bool CLuaIHM::isGuildQuitAvailable()
{
	//H_AUTO(Lua_CLuaIHM_isGuildQuitAvailable)
	return CGuildManager::getInstance()->getGuild().QuitGuildAvailable;
}

// ***************************************************************************
void CLuaIHM::sortGuildMembers()
{
	//H_AUTO(Lua_CLuaIHM_sortGuildMembers)
	CGuildManager::getInstance()->sortGuildMembers();
}

// ***************************************************************************
sint32 CLuaIHM::getNbGuildMembers()
{
	//H_AUTO(Lua_CLuaIHM_getNbGuildMembers)
	return (sint32)CGuildManager::getInstance()->getGuildMembers().size();
}

// ***************************************************************************
string CLuaIHM::getGuildMemberName(sint32 nMemberId)
{
	//H_AUTO(Lua_CLuaIHM_getGuildMemberName)
	if ((nMemberId < 0) || (nMemberId >= getNbGuildMembers()))
		return "";
	return CGuildManager::getInstance()->getGuildMembers()[nMemberId].Name.toString();
}

// ***************************************************************************
string CLuaIHM::getGuildMemberGrade(sint32 nMemberId)
{
	//H_AUTO(Lua_CLuaIHM_getGuildMemberGrade)
	if ((nMemberId < 0) || (nMemberId >= getNbGuildMembers()))
		return "";
	return EGSPD::CGuildGrade::toString(CGuildManager::getInstance()->getGuildMembers()[nMemberId].Grade);
}

// ***************************************************************************
bool CLuaIHM::isR2Player(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_isR2Player)
	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheet));
	if (!entitySheet) return false;
	const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
	if(!chSheet) return false;
	return chSheet->R2Npc;
}

// ***************************************************************************
std::string CLuaIHM::getR2PlayerRace(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getR2PlayerRace)
	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheet));
	if (!entitySheet) return "";
	const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
	if(!chSheet) return "";
	return EGSPD::CPeople::toString(chSheet->Race);
}

// ***************************************************************************
bool CLuaIHM::isR2PlayerMale(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_isR2PlayerMale)
	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheet));
	if (!entitySheet) return true;
	const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
	if(!chSheet) return true;

	return (chSheet->Gender == GSGENDER::male);
}

// ***************************************************************************
std::string CLuaIHM::getCharacterSheetSkel(const std::string &sheet, bool isMale)
{
	//H_AUTO(Lua_CLuaIHM_getCharacterSheetSkel)
	const CEntitySheet *sheetPtr = SheetMngr.get(CSheetId(sheet));
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(sheetPtr);
	if (charSheet) return charSheet->getSkelFilename();
	const CRaceStatsSheet *raceStatSheet = dynamic_cast<const CRaceStatsSheet *>(sheetPtr);
	if (raceStatSheet) return raceStatSheet->GenderInfos[isMale ? 0 : 1].Skelfilename;
	return "";
}

// ***************************************************************************
sint32	CLuaIHM::getSheetId(const std::string &itemName)
{
	//H_AUTO(Lua_CLuaIHM_getSheetId)
	return (sint32)CSheetId(itemName).asInt();
}

// ***************************************************************************
sint CLuaIHM::getCharacterSheetRegionForce(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getCharacterSheetRegionForce)
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(CSheetId(sheet)));
	if (!charSheet) return 0;
	return charSheet->RegionForce;
}

// ***************************************************************************
sint CLuaIHM::getCharacterSheetRegionLevel(const std::string &sheet)
{
	//H_AUTO(Lua_CLuaIHM_getCharacterSheetRegionLevel)
	const CCharacterSheet *charSheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(CSheetId(sheet)));
	if (!charSheet) return 0;
	return charSheet->RegionForce;
}

// ***************************************************************************
bool CLuaIHM::isCtrlKeyDown()
{
	//H_AUTO(Lua_CLuaIHM_isCtrlKeyDown)
	bool ctrlDown = Driver->AsyncListener.isKeyDown(KeyLCONTROL) ||
					Driver->AsyncListener.isKeyDown(KeyRCONTROL);
	if (ctrlDown) nlwarning("ctrl down");
	else nlwarning("ctrl up");
	return ctrlDown;
}

// ***************************************************************************
std::string CLuaIHM::encodeURLUnicodeParam(const ucstring &text)
{
	//H_AUTO(Lua_CLuaIHM_encodeURLUnicodeParam)
	return convertToHTML(text.toUtf8());
}

// ***************************************************************************
ucstring CLuaIHM::replacePvpEffectParam(const ucstring &str, sint32 parameter)
{
	//H_AUTO(Lua_CLuaIHM_replacePvpEffectParam)
	ucstring result = str;
	CSString s = str.toString();
	std::string p, paramString;

	// Locate parameter and store it
	p = s.splitTo('%', true);
	while (p.size() > 0 && s.size() > 0)
	{
		if (s[0] == 'p' || s[0] == 'n' || s[0] == 'r')
		{
			paramString = "%";
			paramString += s[0];
			break;
		}
		p = s.splitTo('%', true);
	}

	// Return original string if param isn't found
	if (paramString.size() < 2)
		return str;

	// Replace parameter based on its type
	switch (paramString[1])
	{
	case 'p':
		p = toString("%.1f %%", parameter/100.0);
		break;
	case 'n':
		p = toString(parameter);
		break;
	case 'r':
		p = toString("%.1f", parameter/100.0);
		break;
	default:
		debugInfo("Bad arguments in " + str.toString() + " : " + paramString);
	}

	strFindReplace(result, paramString.c_str(), p);

	return result;
}

// ***************************************************************************
string CLuaIHM::getRegionByAlias(uint32 alias)
{
	//H_AUTO(Lua_CLuaIHM_getRegionByAlias)
	return ContinentMngr.getRegionNameByAlias(alias);
}

struct CEmoteStruct
{
	string EmoteId;
	string Path;
	string Anim;
	bool   UsableFromClientUI;

	bool operator< (const CEmoteStruct & entry) const
	{
		string path1 = Path;
		string path2 = entry.Path;

		for(;;)
		{
			string::size_type pos1 = path1.find('|');
			string::size_type pos2 = path2.find('|');

			ucstring s1 = toUpper(CI18N::get(path1.substr(0, pos1)));
			ucstring s2 = toUpper(CI18N::get(path2.substr(0, pos2)));

			sint result = s1.compare(s2);
			if (result != 0)
				return (result < 0);

			if (pos1 == string::npos)
				return (pos2 != string::npos);
			if (pos2 == string::npos)
				return false;

			path1 = path1.substr(pos1 + 1);
			path2 = path2.substr(pos2 + 1);
		}
		return false;
	}
};

// ***************************************************************************
int CLuaIHM::initEmotesMenu(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_initEmotesMenu)
	CLuaIHM::checkArgCount(ls, "initEmotesMenu", 2);
	CLuaIHM::checkArgType(ls, "initEmotesMenu", 2, LUA_TSTRING);
	const std::string & emoteMenu = ls.toString(1);
	const std::string & luaParams = ls.toString(2);

	ls.newTable();
	CLuaObject result(ls);
	std::map<std::string, std::string> emoteList;
	uint maxVisibleLine=10;

	CTextEmotListSheet *pTELS = dynamic_cast<CTextEmotListSheet*>(SheetMngr.get(CSheetId("list.text_emotes")));
	if (pTELS == NULL)
		return 0;

	std::list<CEmoteStruct> entries;
	if (entries.empty())
	{
		for (uint i = 0; i < pTELS->TextEmotList.size(); i++)
		{
			CEmoteStruct entry;
			entry.EmoteId = pTELS->TextEmotList[i].EmoteId;
			entry.Path = pTELS->TextEmotList[i].Path;
			entry.Anim = pTELS->TextEmotList[i].Anim;
			entry.UsableFromClientUI = pTELS->TextEmotList[i].UsableFromClientUI;
			entries.push_back(entry);
		}
		entries.sort();
	}

	// The list of behaviour missnames emotList
	CEmotListSheet *pEmotList = dynamic_cast<CEmotListSheet*>(SheetMngr.get(CSheetId("list.emot")));
	nlassert (pEmotList != NULL);
	nlassert (pEmotList->Emots.size() <= 255);

	// Get the focus beta tester flag
	bool betaTester = false;

	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	CSkillManager		*pSM = CSkillManager::getInstance();

	betaTester = pSM->isTitleUnblocked(CHARACTER_TITLE::FBT);

	CGroupMenu *pInitRootMenu = dynamic_cast<CGroupMenu*>(pIM->getElementFromId(emoteMenu));
	pInitRootMenu->reset();

	for (std::list<CEmoteStruct>::const_iterator it = entries.begin(); it != entries.end(); it++)
	{
		std::string sEmoteId = (*it).EmoteId;
		std::string sState = (*it).Anim;
		std::string sName = (*it).Path;

		// Check that the emote can be added to UI
		// ---------------------------------------
		if( (*it).UsableFromClientUI == false )
		{
			continue;
		}

		// Check the emote reserved for FBT (hardcoded)
		// --------------------------------------------
		if (sState == "FBT" && !betaTester)
			continue;

		uint32 i, j;
		// Add to the game context menu
		// ----------------------------
		uint32 nbToken = 1;
		for (i = 0; i < sName.size(); ++i)
			if (sName[i] == '|')
				nbToken++;

		CGroupMenu *pRootMenu = dynamic_cast<CGroupMenu*>(pIM->getElementFromId(emoteMenu));
		CGroupSubMenu *pMenu = pRootMenu->getRootMenu();

		for (i = 0; i < nbToken; ++i)
		{
			if(i==0)
			{
				sName = sName.substr(sName.find('|')+1,sName.size());
			}
			else
			{
				string sTmp;
				if (i != (nbToken-1))
					sTmp = sName.substr(0,sName.find('|'));
				else
					sTmp = sName;



				// Look if this part of the path is already present
				bool bFound = false;
				for (j = 0; j < pMenu->getNumLine(); ++j)
				{
					if (sTmp == pMenu->getLineId(j))
					{
						bFound = true;
						break;
					}
				}

				if (!bFound) // Create it
				{
					if (i != (nbToken-1))
					{
						pMenu->addLine (CI18N::get(sTmp), "", "", sTmp);
						// Create a sub menu
						CGroupSubMenu *pNewSubMenu = new CGroupSubMenu(CViewBase::TCtorParam());
						pMenu->setSubMenu(j, pNewSubMenu);
					}
					else
					{
						// Create a line
						pMenu->addLine (CI18N::get(sTmp), "lua",
							luaParams+"('"+sEmoteId+"', '"+toString(CI18N::get(sTmp))+"')", sTmp);
						emoteList[sEmoteId] = (toLower(CI18N::get(sTmp))).toUtf8();
					}
				}

				// Jump to sub menu
				if (i != (nbToken-1))
				{
					pMenu = pMenu->getSubMenu(j);
					sName = sName.substr(sName.find('|')+1,sName.size());
				}
			}
		}
		pMenu->setMaxVisibleLine(maxVisibleLine);
	}
	pInitRootMenu->setMaxVisibleLine(maxVisibleLine);

	std::map<std::string, std::string>::iterator it;
	for(it=emoteList.begin(); it!=emoteList.end(); it++)
	{
		result.setValue(it->first, it->second);
	}
	result.push();

	return 1;
}

// ***************************************************************************
int CLuaIHM::isUCString(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_isUCString)
	const char *funcName = "isUCString";
	checkArgCount(ls, funcName, 1);
	ls.push(isUCStringOnStack(ls, 1));
	return 1;
}

// ***************************************************************************
int CLuaIHM::hideAllWindows(CLuaState &/* ls */)
{
	//H_AUTO(Lua_CLuaIHM_hideAllWindows)
	CInterfaceManager::getInstance()->hideAllWindows();
	return 0;
}

// ***************************************************************************
int CLuaIHM::hideAllNonSavableWindows(CLuaState &/* ls */)
{
	//H_AUTO(Lua_CLuaIHM_hideAllNonSavableWindows)
	CInterfaceManager::getInstance()->hideAllNonSavableWindows();
	return 0;
}

// ***************************************************************************
int CLuaIHM::getDesktopIndex(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getDesktopIndex)
	ls.push((double) CInterfaceManager::getInstance()->getMode());
	return 1;
}

// ***************************************************************************
int CLuaIHM::setLuaBreakPoint(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_setLuaBreakPoint)
	const char *funcName = "setLuaBreakPoint";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);


	#ifdef LUA_NEVRAX_VERSION
		if (LuaDebuggerIDE)
		{
			LuaDebuggerIDE->setBreakPoint(ls.toString(1), (int) ls.toNumber(2));
		}
	#endif

	return 0;
}

// ***************************************************************************
int CLuaIHM::getMainPageURL(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getMainPageURL)
	const char *funcName = "getMainPageURL";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push(RingMainURL);
	return 1;
}

// ***************************************************************************
int	CLuaIHM::getCharSlot(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getCharSlot)
	const char *funcName = "getCharSlot";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push(double(PlayerSelectedSlot));
	return 1;
}

// ***************************************************************************
sint32		CLuaIHM::secondsSince1970ToHour(sint32 seconds)
{
	//H_AUTO(Lua_CLuaIHM_secondsSince1970ToHour)
	// convert to readable form
	struct tm	*tstruct;
	time_t		tval= seconds;
	tstruct= gmtime(&tval);
	if(!tstruct)
	{
		debugInfo(toString("Bad Date Received: %d", seconds));
		return 0;
	}

	return tstruct->tm_hour;	// 0-23
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
		ls.push((double) k);
		ls.push(files[k]);
		ls.setTable(-3);
	}
	return 1;
}

int CLuaIHM::getServerSeason(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getServerSeason)
	const char *funcName = "getServerSeason";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	extern uint8 ServerSeasonValue;
	ls.push((double) ServerSeasonValue);
	return 1;
}

int CLuaIHM::computeCurrSeason(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_computeCurrSeason)
	const char *funcName = "computeCurrSeason";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push((double) (::computeCurrSeason() + 1));
	return 1;
}

int CLuaIHM::getAutoSeason(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getAutoSeason)
	const char *funcName = "getAutoSeason";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	ls.push((double) (StartupSeason + 1));
	return 1;
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

	ls.push((double) bitmap.getWidth());
	ls.push((double) bitmap.getHeight());

	return 2;
}


int CLuaIHM::enableModalWindow(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_enableModalWindow)
	const char *funcName = "enableModalWindow";
	CLuaIHM::checkArgCount(ls, funcName, 2);

	check(ls,   isUIOnStack(ls, 1), "enableModalWindow() requires a UI object in param 1");
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);

	CInterfaceElement	*pIE= getUIOnStack(ls, 1);
	std::string modalId = ls.toString(2);

	// convert to id
	if(pIE)
	{
		CCtrlBase * ctrl = dynamic_cast<CCtrlBase*>(pIE);
		if(ctrl)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( pIM->getElementFromId(modalId) );
			if(group)
			{
				UserControls.stopFreeLook();

				// enable the modal
				pIM->enableModalWindow(ctrl, group);
			}
			else
			{
				nlwarning("<CLuaIHM::enableModalWindow> Couldn't find group %s", modalId.c_str());
			}

		}
	}

	return 0;
}

// ***************************************************************************
int		CLuaIHM::disableModalWindow(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_disableModalWindow)
	checkArgCount(ls, "disableModalWindow", 0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->disableModalWindow();
	return 0;
}

// ***************************************************************************
int CLuaIHM::getPlayerPos(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_getPlayerPos)
	checkArgCount(ls, "getPlayerPos", 0);
	ls.push(UserEntity->pos().x);
	ls.push(UserEntity->pos().y);
	ls.push(UserEntity->pos().z);
	return 3;
}

// ***************************************************************************
int CLuaIHM::isPlayerFreeTrial(CLuaState &ls)
{
	checkArgCount(ls, "isPlayerFreeTrial", 0);
	ls.push(FreeTrial);
	return 1;
}



// ***************************************************************************
int CLuaIHM::disableContextHelp(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_disableContextHelp)
	CLuaStackChecker lsc(&ls,    0);
	checkArgCount(ls,    "disableContextHelp",    0);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->disableContextHelp();
	return 0;
}

// ***************************************************************************
int			CLuaIHM::disableContextHelpForControl(CLuaState &ls)
{
	//H_AUTO(Lua_CLuaIHM_disableContextHelpForControl)
	CLuaStackChecker lsc(&ls,    0);

	// params: CCtrlBase*
	// return: none
	checkArgCount(ls,    "disableContextHelpForControl",    1);
	check(ls,   isUIOnStack(ls,   1),    "disableContextHelpForControl() requires a UI object in param 1");

	// retrieve args
	CInterfaceElement	*pIE= getUIOnStack(ls,    1);

	// go
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->disableContextHelpForControl(dynamic_cast<CCtrlBase*>(pIE));

	return 0;
}

// ***************************************************************************
void CLuaIHM::tell(const ucstring &player, const ucstring &msg)
{
	//H_AUTO(Lua_CLuaIHM_tell)
	// display a /tell command in the main chat
	if (!player.empty())
	{
		if (!msg.empty())
		{
			ChatMngr.tell(player.toUtf8(), msg);
		}
		else
		{
			CChatWindow *w = PeopleInterraction.ChatGroup.Window;
			if (w)
			{
				CInterfaceManager *im = CInterfaceManager::getInstance();
				w->setKeyboardFocus();
				w->enableBlink(1);
				w->setCommand(ucstring("tell ") + CEntityCL::removeTitleFromName(player) + ucstring(" "), false);
				CGroupEditBox *eb = w->getEditBox();
				if (eb != NULL)
				{
					eb->bypassNextKey();
				}
				if (w->getContainer())
				{
					w->getContainer()->setActive(true);
					im->setTopWindow(w->getContainer());
				}
			}
		}
	}
}

// ***************************************************************************
bool CLuaIHM::isRingAccessPointInReach()
{
	//H_AUTO(Lua_CLuaIHM_isRingAccessPointInReach)
	if (BotChatPageAll->RingSessions->RingAccessPointPos == CVector::Null) return false;
	const CVectorD &vect1 = BotChatPageAll->RingSessions->RingAccessPointPos;
	CVectorD vect2 = UserEntity->pos();
	double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);
	return distanceSquare <= MaxTalkingDistSquare;
}

// ***************************************************************************
void CLuaIHM::updateTooltipCoords()
{
	CInterfaceManager::getInstance()->updateTooltipCoords();
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
		if (!CLuaIHM::pop(ls, pos))
		{
			CLuaIHM::fails(ls, "2D polygon expects CVector2f for poly coordinates");
		}
		dest.Vertices.push_back(pos);
	}
}


// ***************************************************************************
int CLuaIHM::isPlayerNewbie(CLuaState &ls)
{
	checkArgCount(ls, "isPlayerNewbie", 0);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	ls.push(im->getDbProp("SERVER:USER:IS_NEWBIE")->getValueBool());
	return 1;
}


// ***************************************************************************
int CLuaIHM::isInRingMode(CLuaState &ls)
{
	checkArgCount(ls, "isInRingMode", 0);
	extern bool IsInRingMode();
	ls.push(IsInRingMode());
	return 1;
}

// ***************************************************************************
int CLuaIHM::getUserRace(CLuaState &ls)
{
	checkArgCount(ls, "getUserRace", 0);
	if (!UserEntity || !UserEntity->playerSheet())
	{
		ls.push("Unknwown");
	}
	else
	{
		ls.push(EGSPD::CPeople::toString(UserEntity->playerSheet()->People));
	}
	return 1;
}

