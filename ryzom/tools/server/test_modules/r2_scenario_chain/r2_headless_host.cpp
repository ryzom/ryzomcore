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

#include "r2_headless_host.h"

#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/string_common.h"
#include "nel/misc/time_nl.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

#include "game_share/people_pd.h"
#include "game_share/scenario_entry_points.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#if LUA_VERSION_NUM < 503
#error "The headless Ring host needs Lua 5.3 or later"
#endif
#if LUA_VERSION_NUM < 504
#define lua_newuserdatauv(L, size, nuvalue) lua_newuserdata(L, size)
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>

using namespace NLMISC;

namespace R2 {

namespace {

CHeadlessHost *s_Host = nullptr;

// registry keys, by address
char s_ProjectionMetatableKey = 0;
char s_ProjectionCacheKey = 0;
char s_UserTablesKey = 0;

typedef CObject::TRefPtr TProjectionRef;

#if LUA_VERSION_NUM >= 505
void *luaAllocate(void * /* userData */, void *block, size_t /* oldSize */, size_t newSize)
{
	if (newSize == 0)
	{
		free(block);
		return nullptr;
	}
	return realloc(block, newSize);
}
#endif

// Pushes the message and returns the marker the R2H_FUNCTION wrapper turns into a Lua error, once the
// C++ objects of the function body are gone (lua_error does not unwind them)
int fail(lua_State *state, const std::string &message)
{
	lua_pushlstring(state, message.data(), message.size());
	return -1;
}

#define R2H_FUNCTION(name)                       \
	int CHeadlessHost::name(lua_State *state)    \
	{                                            \
		int result = name##Body(state);          \
		if (result < 0) return lua_error(state); \
		return result;                           \
	}                                            \
	int CHeadlessHost::name##Body(lua_State *state)

std::string luaArgString(lua_State *state, int idx)
{
	size_t len = 0;
	const char *text = lua_tolstring(state, idx, &len);
	if (!text) return std::string();
	return std::string(text, len);
}

// Text of any value, without changing it on the stack
std::string luaValueText(lua_State *state, int idx)
{
	size_t len = 0;
	const char *text = luaL_tolstring(state, idx, &len);
	std::string result(text ? text : "", text ? len : 0);
	lua_pop(state, 1);
	return result;
}

// "file:line" of the script line that called the running C function
std::string callerLocation(lua_State *state)
{
	luaL_where(state, 1);
	std::string where = lua_tostring(state, -1) ? lua_tostring(state, -1) : "";
	lua_pop(state, 1);
	while (!where.empty() && (where[where.size() - 1] == ' ' || where[where.size() - 1] == ':'))
		where.resize(where.size() - 1);
	return where;
}

// Open a standard library and publish it as a global, as luaL_openlibs does
void openStandardLibrary(lua_State *state, const char *name, lua_CFunction open)
{
	luaL_requiref(state, name, open, 1);
	lua_pop(state, 1);
}

// Leaves a library with the listed functions only: a fresh table takes them and becomes the global and
// the entry of the loaded table
void reduceStandardLibrary(lua_State *state, const char *name, const char *const *functions)
{
	lua_getglobal(state, name);
	lua_newtable(state);
	for (; *functions; ++functions)
	{
		lua_getfield(state, -2, *functions);
		lua_setfield(state, -2, *functions);
	}
	lua_pushvalue(state, -1);
	lua_setglobal(state, name);
	lua_getfield(state, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
	if (lua_istable(state, -1))
	{
		lua_pushvalue(state, -2);
		lua_setfield(state, -2, name);
	}
	lua_pop(state, 3);
}

// Interface scripts the translator needs functions from. r2_core.lua leaves the interface scripts out
// when r2.InClient is not set.
// r2_ui_property_sheet.lua: the r2:canPick* target filters, called by Translator.CheckPickedEntity
// r2_ui_displayer_npc.lua: r2:randomNPC and what it needs (the equipment of the player races, the hair
//   cuts), which the bandit camp makes its bandits with when it is translated
// r2_events.lua, r2_dialogs.lua, r2_activities.lua, r2_logic_comp.lua: the tables r2.events, r2.dialogs,
//   r2.activities and r2.logicComponents, which the getName functions of r2_logic.lua read; the translator
//   calls getName to word its error messages
const char *const s_InterfaceScripts[] = {
	"r2_ui_property_sheet.lua",
	"r2_ui_displayer_npc.lua",
	"r2_events.lua",
	"r2_dialogs.lua",
	"r2_activities.lua",
	"r2_logic_comp.lua",
};

// Stubs: client functions the scripts call on this path and the host has nothing behind. Each gives a
// fixed value and is recorded when called. A client function that is not here and has no implementation
// below does not exist for the scripts, so that a call fails.
enum TStubResult
{
	StubNothing,
	StubFalse,
	StubEmptyTable,
	StubVisualProperties,
};

struct CStubDef
{
	const char *Name; // "name" for a global, "r2.name" for a field of r2
	TStubResult Result;
};

const CStubDef s_Stubs[] = {
	// writes the scenario to a file; the version update saves the scenario it starts from
	{ "r2.save", StubNothing },
	// R2EDMustVerifyRingAccessWhileLoadingAnimation of the client configuration, off by default
	{ "r2.getMustVerifyRingAccessWhileLoadingAnimation", StubFalse },
	// The visual properties of a custom NPC (CEditor::getVisualPropertiesFromObject) come from the item and
	// race sheets and the visual slot table of the client. The run-time NPC gets an empty set here.
	{ "r2.getVisualPropertiesFromInstanceId", StubVisualProperties },
	// nothing is selected
	{ "r2.getSelectedInstance", StubNothing },
	// functions of the interface scripts (r2_ui_*.lua), which are not loaded without r2.InClient
	{ "r2.propertySheetDisplayer", StubEmptyTable },
	// called by the assert of r2_debug.lua
	{ "debugBreak", StubNothing },
};

// Defines of the interface the scripts read (ryzom/client/data/gamedev/interfaces_v3/r2ed_scenario.xml)
const char *const s_Defines[][2] = {
	{ "r2ed_max_num_additionnal_acts", "10" },
	{ "r2ed_max_num_plot_items", "32" },
	{ "r2ed_max_num_plot_item_sheets", "200" },
};

// Variables of the client configuration the scripts read, with the values of client_default.cfg
const char *const s_ConfigValues[][2] = {
	{ "R2EDExtendedDebug", "0" },
	{ "R2EDLightPalette", "0" },
	{ "R2EDLoadDynamicFeatures", "0" },
};

// Variables of the client configuration the scripts read through getClientCfgVar, with the values of
// client_default.cfg
const char *const s_ClientCfgVars[][2] = {
	{ "LevelDesignEnabled", "0" },
};

const char *requestKindName(int kind, bool ghost)
{
	switch (kind)
	{
	case 0: return ghost ? "insertGhostNode" : "insertNode";
	case 1: return ghost ? "setGhostNode" : "setNode";
	case 2: return "eraseNode";
	default: return "moveNode";
	}
}

} // namespace

// ***************************************************************************
CHeadlessHost::CHeadlessHost()
    : m_Lua(nullptr)
    , m_Verbose(false)
    , m_TranslationLoaded(false)
    , m_FormLoader(nullptr)
    , m_Factory(nullptr)
    , m_Scenario(nullptr)
    , m_ServerScenario(nullptr)
    , m_Eid("Client1")
    , m_StartingActIndex(0)
    , m_HasOpenAction(false)
    , m_OpenActionIsPending(false)
    , m_SubActionCount(0)
    , m_HistoricHasScenario(true)
{
	nlassert(!s_Host);
	s_Host = this;
	m_Phase = "init";
}

// ***************************************************************************
CHeadlessHost::~CHeadlessHost()
{
	if (m_Lua)
	{
		lua_close(m_Lua);
		m_Lua = nullptr;
	}
	m_OpenAction.clear();
	m_FrameRequests.clear();
	m_UploadedScenario = nullptr;
	delete m_Scenario;
	m_Scenario = nullptr;
	delete m_ServerScenario;
	m_ServerScenario = nullptr;
	m_Palette.clear();
	delete m_Factory;
	m_Factory = nullptr;
	if (m_FormLoader)
	{
		NLGEORGES::UFormLoader::releaseLoader(m_FormLoader);
		m_FormLoader = nullptr;
	}
	s_Host = nullptr;
}

// ***************************************************************************
void CHeadlessHost::log(const std::string &kind, const std::string &text)
{
	CLogLine line;
	line.Phase = m_Phase;
	line.Kind = kind;
	line.Text = text;
	m_Log.push_back(line);
	if (m_Verbose)
	{
		printf("[%s] %s: %s\n", m_Phase.c_str(), kind.c_str(), text.c_str());
		fflush(stdout);
	}
}

// ***************************************************************************
void CHeadlessHost::loadTranslation(const std::string &directory, const std::string &language)
{
	// the interface strings and, as CEditor::loadLanguageFile, those of the editor
	CPath::addSearchPath(directory, false, false);
	CI18N::load(language);
	if (CPath::exists("r2_" + language + ".uxt"))
		CI18N::loadFromFilename("r2_" + language + ".uxt", false);
	m_TranslationLoaded = true;
}

// ***************************************************************************
// Object trees to Lua values and back, as CComLuaModule::setObjectToLua and getObjectFromLua
// ***************************************************************************
static void setObjectToLua(lua_State *state, const CObject *object, bool withGhosts = true)
{
	luaL_checkstack(state, 8, "the object is nested too deep for the Lua stack");
	if (!object)
	{
		lua_pushnil(state);
		return;
	}
	if (object->isInteger())
	{
		lua_pushinteger(state, (lua_Integer)object->toInteger());
		return;
	}
	if (object->isNumber())
	{
		lua_pushnumber(state, object->toNumber());
		return;
	}
	if (object->isRefId())
	{
		// { Value = id } with r2.RefIdMetatable
		lua_newtable(state);
		lua_pushstring(state, object->toString().c_str());
		lua_setfield(state, -2, "Value");
		lua_getglobal(state, "r2");
		lua_getfield(state, -1, "RefIdMetatable");
		lua_remove(state, -2);
		lua_setmetatable(state, -2);
		return;
	}
	if (object->isString())
	{
		std::string text = object->toString();
		lua_pushlstring(state, text.data(), text.size());
		return;
	}
	if (object->isTable())
	{
		lua_newtable(state);
		uint32 size = object->getSize();
		uint32 arrayIndex = 0;
		for (uint32 i = 0; i < size; ++i)
		{
			std::string key = object->getKey(i);
			const CObject *value = object->getValueAtPos(i);
			if (!withGhosts && value && value->getGhost()) continue;
			if (!key.empty())
			{
				lua_pushstring(state, key.c_str());
				setObjectToLua(state, value, withGhosts);
				lua_settable(state, -3);
			}
			else
			{
				++arrayIndex;
				setObjectToLua(state, value, withGhosts);
				lua_rawseti(state, -2, arrayIndex);
			}
		}
		return;
	}
	lua_pushnil(state);
}

// ***************************************************************************
CObject::TSmartPtr CHeadlessHost::getObjectFromLua(lua_State *state, int idx)
{
	lua_pushvalue(state, idx);

	if (lua_type(state, -1) == LUA_TTABLE && lua_getmetatable(state, -1))
	{
		// obj, mt
		lua_getglobal(state, "r2");
		lua_getfield(state, -1, "RefIdMetatable"); // obj, mt, r2, RefIdMetatable
		bool isRefId = lua_rawequal(state, -1, -3) != 0;
		lua_pop(state, 3); // obj
		if (isRefId)
		{
			lua_getfield(state, -1, "Value");
			CObject::TSmartPtr result;
			if (lua_type(state, -1) == LUA_TSTRING)
				result = new CObjectRefId(luaArgString(state, -1));
			lua_pop(state, 2);
			return result;
		}
	}

	switch (lua_type(state, -1))
	{
	case LUA_TNUMBER: {
		CObject::TSmartPtr result;
		if (lua_isinteger(state, -1))
			result = new CObjectNumber((sint64)lua_tointeger(state, -1));
		else
			result = new CObjectNumber((double)lua_tonumber(state, -1));
		lua_pop(state, 1);
		return result;
	}
	case LUA_TBOOLEAN: {
		sint64 value = lua_toboolean(state, -1);
		lua_pop(state, 1);
		return new CObjectNumber(value);
	}
	case LUA_TSTRING: {
		std::string value = luaArgString(state, -1);
		lua_pop(state, 1);
		return new CObjectString(value);
	}
	case LUA_TTABLE: {
		CObjectTable *table = new CObjectTable();
		CObject::TSmartPtr keep(table);
		lua_pushnil(state);
		while (lua_next(state, -2) != 0)
		{
			std::string key;
			if (lua_type(state, -2) == LUA_TSTRING)
				key = luaArgString(state, -2);
			CObject::TSmartPtr value = getObjectFromLua(state, -1);
			if (value)
				table->add(key, value);
			lua_pop(state, 1);
		}
		lua_pop(state, 1);
		table->sort();
		return keep;
	}
	default:
		// functions, userdata and nil are left out
		lua_pop(state, 1);
		return nullptr;
	}
}

// ***************************************************************************
// Projection of the scenario tree, as CEditor::initObjectProjectionMetatable
// ***************************************************************************
const CObject *CHeadlessHost::getPropertyValue(const CObject *component, const std::string &attrName) const
{
	// as CPropertyAccessor::getPropertyValue: the object, then its palette bases, then the defaults of its
	// class and of the base classes
	const CObject *result = component->getAttr(attrName);
	const CObject *baseElement = component;
	while (!result)
	{
		const CObject *base = baseElement->getAttr("Base");
		if (!base || !base->isString()) break;
		std::map<std::string, CObject::TSmartPtr>::const_iterator it = m_Palette.find(base->toString());
		if (it == m_Palette.end())
		{
			nlwarning("Can't find base palette element : name = %s", base->toString().c_str());
			return nullptr;
		}
		baseElement = it->second;
		result = baseElement->getAttr(attrName);
	}
	if (!result)
	{
		const CObject *propClass = component->getAttr("Class");
		if (propClass && propClass->isString())
		{
			std::string className = propClass->toString();
			while (!result && !className.empty())
			{
				CObjectGenerator *generator = m_Factory->getGenerator(className);
				if (!generator) return nullptr;
				result = generator->getDefaultValue(attrName);
				if (!result)
					className = generator->getBaseClass();
			}
		}
	}
	return result;
}

// ***************************************************************************
void CHeadlessHost::pushProjection(lua_State *state, const CObject *object)
{
	// one userdata per table for as long as Lua refers to it, so that two reads of a node are the same value
	lua_rawgetp(state, LUA_REGISTRYINDEX, &s_ProjectionCacheKey); // cache
	lua_rawgetp(state, -1, object); // cache, ud?
	if (lua_type(state, -1) == LUA_TUSERDATA)
	{
		TProjectionRef *ref = (TProjectionRef *)lua_touserdata(state, -1);
		if ((const CObject *)(*ref) == object)
		{
			lua_remove(state, -2);
			return;
		}
	}
	lua_pop(state, 1); // cache
	void *block = lua_newuserdatauv(state, sizeof(TProjectionRef), 0); // cache, ud
	new (block) TProjectionRef(const_cast<CObject *>(object));
	lua_rawgetp(state, LUA_REGISTRYINDEX, &s_ProjectionMetatableKey);
	lua_setmetatable(state, -2);
	lua_pushvalue(state, -1); // cache, ud, ud
	lua_rawsetp(state, -3, object); // cache, ud
	lua_remove(state, -2);
}

// ***************************************************************************
void CHeadlessHost::pushProjectedValue(lua_State *state, const CObject *object)
{
	if (!object)
		lua_pushnil(state);
	else if (object->isString())
	{
		std::string text = object->toString();
		lua_pushlstring(state, text.data(), text.size());
	}
	else if (object->isInteger())
		lua_pushinteger(state, (lua_Integer)object->toInteger());
	else if (object->isNumber())
		lua_pushnumber(state, object->toNumber());
	else if (object->isTable())
		pushProjection(state, object);
	else
		lua_pushnil(state);
}

// ***************************************************************************
// The client makes a CInstance, which has the Ghost and Selectable properties, for a table with an InstanceId
static bool isInstance(const CObject *obj)
{
	const CObject *instanceId = obj->getAttr("InstanceId");
	return instanceId && instanceId->isString() && !instanceId->toString().empty();
}

R2H_FUNCTION(luaProjectionIndex)
{
	TProjectionRef *ref = (TProjectionRef *)lua_touserdata(state, 1);
	const CObject *obj = *ref;
	if (!obj)
	{
		if (lua_type(state, 2) == LUA_TSTRING && strcmp(lua_tostring(state, 2), "isNil") == 0)
		{
			lua_pushboolean(state, 1);
			return 1;
		}
		return fail(state, "Attempt to access an erased object");
	}
	// an integer is a position; any other number is read as the name its text makes, as in the client
	if (lua_isinteger(state, 2))
	{
		lua_Integer index = lua_tointeger(state, 2);
		if (index >= 0 && index < (lua_Integer)obj->getSize())
		{
			s_Host->pushProjectedValue(state, obj->getValueAtPos((uint32)index));
			return 1;
		}
	}
	if (!lua_isstring(state, 2))
	{
		lua_pushnil(state);
		return 1;
	}
	lua_pushvalue(state, 2);
	std::string name = luaArgString(state, -1);
	lua_pop(state, 1);
	if (name == "isNil")
	{
		lua_pushboolean(state, 0);
		return 1;
	}
	if (name == "Parent")
	{
		s_Host->pushProjectedValue(state, obj->getParent());
		return 1;
	}
	if (name == "ParentInstance")
	{
		const CObject *parent = obj->getParent();
		while (parent)
		{
			if (parent->findAttr("InstanceId"))
			{
				s_Host->pushProjection(state, parent);
				return 1;
			}
			parent = parent->getParent();
		}
		lua_pushnil(state);
		return 1;
	}
	if (name == "IndexInParent")
	{
		if (!obj->getParent())
		{
			lua_pushnil(state);
			return 1;
		}
		sint32 index = obj->getParent()->findIndex(obj);
		if (index >= 0 && obj->getParent()->getKey(index).empty())
			lua_pushinteger(state, index);
		else
			lua_pushinteger(state, -1);
		return 1;
	}
	if (name == "User")
	{
		// a table the scripts are free to write in, one per object
		lua_rawgetp(state, LUA_REGISTRYINDEX, &s_UserTablesKey);
		lua_rawgetp(state, -1, obj);
		if (lua_isnil(state, -1))
		{
			lua_pop(state, 1);
			lua_newtable(state);
			lua_pushvalue(state, -1);
			lua_rawsetp(state, -3, obj);
		}
		return 1;
	}
	if (name == "Size")
	{
		lua_pushinteger(state, obj->getSize());
		return 1;
	}
	if (name == "DisplayerUI" || name == "DisplayerVisual" || name == "DisplayerProperties")
	{
		// no displayers without the interface
		lua_pushnil(state);
		return 1;
	}
	const CObject *value = s_Host->getPropertyValue(obj, name);
	if (value)
	{
		s_Host->pushProjectedValue(state, value);
		return 1;
	}
	// not an attribute: a method of the class
	const CObject *className = obj->findAttr("Class");
	if (className)
	{
		lua_getglobal(state, "r2");
		lua_getfield(state, -1, "Classes");
		if (lua_istable(state, -1))
		{
			lua_getfield(state, -1, className->toString().c_str());
			if (lua_istable(state, -1))
			{
				lua_getfield(state, -1, name.c_str());
				if (lua_isfunction(state, -1))
					return 1;
			}
		}
	}
	// the properties CInstance exports
	if (isInstance(obj))
	{
		if (name == "Ghost")
		{
			lua_pushboolean(state, obj->getGhost() ? 1 : 0);
			return 1;
		}
		if (name == "Selectable" || name == "SelectableFromRoot")
		{
			s_Host->recordStubCall(state, "instance." + name + " (always true)");
			lua_pushboolean(state, 1);
			return 1;
		}
	}
	lua_pushnil(state);
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaProjectionNewIndex)
{
	TProjectionRef *ref = (TProjectionRef *)lua_touserdata(state, 1);
	CObject *obj = *ref;
	if (!obj)
		return fail(state, "Trying to set a property in an erased object");
	std::string name = luaValueText(state, 2);
	// the client lets a script write the properties CInstance exports, on an instance, and nothing else
	if (isInstance(obj))
	{
		if (name == "Ghost")
		{
			obj->setGhost(lua_toboolean(state, 3) != 0);
			return 0;
		}
		if (name == "Selectable" || name == "SelectableFromRoot")
		{
			s_Host->recordStubCall(state, "instance." + name + " (write ignored)");
			return 0;
		}
	}
	return fail(state, "Property " + name + " of editor object is read-only. You must use 'r2.requestSetNode' function to change it");
}

// ***************************************************************************
void CHeadlessHost::pushProjectionKey(lua_State *state, const CObject *obj, sint32 index)
{
	std::string key = obj->getKey(index);
	if (key.empty())
		lua_pushinteger(state, index);
	else
		lua_pushstring(state, key.c_str());
}

R2H_FUNCTION(luaProjectionNext)
{
	TProjectionRef *ref = (TProjectionRef *)lua_touserdata(state, 1);
	const CObject *obj = *ref;
	if (!obj)
		return fail(state, "editor object '__next' metatmethod : Attempt to access an erased object");
	sint32 index;
	if (lua_isnoneornil(state, 2))
	{
		// The walk goes on from a key by looking the key up, so a key that is there twice would bring it
		// back to the first one for ever. The client refuses to start such a walk.
		std::set<std::string> keys;
		for (uint32 k = 0; k < obj->getSize(); ++k)
		{
			std::string key = obj->getKey(k);
			if (!key.empty() && !keys.insert(key).second)
				return fail(state, "Duplicated key of type string found while attempting to enumerate an instance content (key is " + key + "). Aborting to avoid infinite loop.");
		}
		index = -1;
	}
	else if (lua_isinteger(state, 2))
	{
		index = (sint32)lua_tointeger(state, 2);
	}
	else if (lua_isstring(state, 2))
	{
		lua_pushvalue(state, 2);
		std::string key = luaArgString(state, -1);
		lua_pop(state, 1);
		index = obj->findIndex(key);
		if (index == -1)
		{
			lua_pushnil(state);
			lua_pushnil(state);
			return 2;
		}
	}
	else
	{
		lua_pushnil(state);
		lua_pushnil(state);
		return 2;
	}
	sint32 newIndex = index + 1;
	if (newIndex < 0 || newIndex >= (sint32)obj->getSize())
	{
		lua_pushnil(state);
		lua_pushnil(state);
		return 2;
	}
	pushProjectionKey(state, obj, newIndex);
	s_Host->pushProjectedValue(state, obj->getValueAtPos((uint32)newIndex));
	return 2;
}

// ***************************************************************************
int CHeadlessHost::luaProjectionEqual(lua_State *state)
{
	TProjectionRef *lhs = (TProjectionRef *)lua_touserdata(state, 1);
	TProjectionRef *rhs = (TProjectionRef *)lua_touserdata(state, 2);
	bool equal = lhs && rhs && (const CObject *)(*lhs) == (const CObject *)(*rhs);
	lua_pushboolean(state, equal ? 1 : 0);
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaProjectionGc(lua_State *state)
{
	TProjectionRef *ref = (TProjectionRef *)lua_touserdata(state, 1);
	ref->~TProjectionRef();
	return 0;
}

// ***************************************************************************
int CHeadlessHost::luaProjectionToString(lua_State *state)
{
	TProjectionRef *ref = (TProjectionRef *)lua_touserdata(state, 1);
	const CObject *obj = *ref;
	lua_pushfstring(state, "editor object: %p", (const void *)obj);
	return 1;
}

// ***************************************************************************
// Scripts
// ***************************************************************************
bool CHeadlessHost::readScript(const std::string &filename, std::string &chunk, std::string &error) const
{
	// a plain file name of the editor's directory: the client finds its scripts by name through its search
	// paths, which cover the sub directory too
	if (filename.empty() || filename.find_first_of("/\\:") != std::string::npos || filename.find("..") != std::string::npos)
	{
		error = "not a plain script name: " + filename;
		return false;
	}
	std::string path = m_R2Dir + "/" + filename;
	if (!CFile::fileExists(path))
		path = m_R2Dir + "/unit_test/" + filename;
	if (!CFile::fileExists(path))
	{
		error = "Can't find " + filename;
		return false;
	}
	CIFile file;
	if (!file.open(path))
	{
		error = "Can't open " + path;
		return false;
	}
	chunk.resize(file.getFileSize());
	if (!chunk.empty())
		file.serialBuffer((uint8 *)&chunk[0], (uint)chunk.size());
	return true;
}

// ***************************************************************************
bool CHeadlessHost::doFile(const std::string &filename, std::string &error)
{
	std::string chunk;
	if (!readScript(filename, chunk, error))
		return false;
	return runChunk(chunk, "@" + filename, 0, error);
}

// ***************************************************************************
bool CHeadlessHost::runChunk(const std::string &code, const std::string &chunkName, int resultCount, std::string &error)
{
	lua_pushcfunction(m_Lua, luaTraceback);
	int handler = lua_gettop(m_Lua);
	int status = luaL_loadbufferx(m_Lua, code.data(), code.size(), chunkName.c_str(), "t");
	if (status == LUA_OK)
		status = lua_pcall(m_Lua, 0, resultCount, handler);
	if (status != LUA_OK)
	{
		error = luaValueText(m_Lua, -1);
		lua_settop(m_Lua, handler - 1);
		return false;
	}
	lua_remove(m_Lua, handler);
	return true;
}

// ***************************************************************************
bool CHeadlessHost::callDriver(const std::string &function, const std::vector<std::string> &arguments, int resultCount, std::string &error)
{
	// the arguments are values: text of a scenario never becomes Lua source
	lua_pushcfunction(m_Lua, luaTraceback);
	int handler = lua_gettop(m_Lua);
	lua_getglobal(m_Lua, "r2Headless");
	if (!lua_istable(m_Lua, -1))
	{
		lua_settop(m_Lua, handler - 1);
		error = "the driver script did not define r2Headless";
		return false;
	}
	lua_getfield(m_Lua, -1, function.c_str());
	lua_remove(m_Lua, -2);
	for (size_t i = 0; i < arguments.size(); ++i)
		lua_pushlstring(m_Lua, arguments[i].data(), arguments[i].size());
	if (lua_pcall(m_Lua, (int)arguments.size(), resultCount, handler) != LUA_OK)
	{
		error = luaValueText(m_Lua, -1);
		lua_settop(m_Lua, handler - 1);
		return false;
	}
	lua_remove(m_Lua, handler);
	return true;
}

// ***************************************************************************
int CHeadlessHost::luaTraceback(lua_State *state)
{
	// message handler: the message with the stack below it
	const char *message = lua_tostring(state, 1);
	if (!message)
		message = luaL_tolstring(state, 1, nullptr);
	luaL_traceback(state, state, message, 1);
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaPanic(lua_State *state)
{
	// an error outside of a protected call: Lua ends the process after this function
	const char *message = lua_tostring(state, -1);
	printf("panic: %s\nexit: 11\n", message ? message : "(no message)");
	fflush(stdout);
	exit(11);
}

// ***************************************************************************
R2H_FUNCTION(luaRecordSwallowedError)
{
	// called by the pcall replacement with the traceback of an error the script is about to swallow
	s_Host->log("luaError", luaValueText(state, 1));
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaDoFile)
{
	// The client logs an error in a script and goes on (CEditor::doLuaScript). Here it is an error for the
	// caller as well, and the load of the editor fails as a whole.
	std::string filename = luaValueText(state, 1);
	std::string error;
	if (!s_Host->doFile(filename, error))
	{
		s_Host->log("loadError", filename + ": " + error);
		return fail(state, "r2.doFile " + filename + ": " + error);
	}
	return 0;
}

// ***************************************************************************
// load(chunk [, chunkname [, mode [, env]]]), also the loadstring of Lua 5.1: text chunks only, as the client
R2H_FUNCTION(luaSafeLoad)
{
	std::string chunk;
	if (lua_type(state, 1) == LUA_TSTRING)
	{
		chunk = luaArgString(state, 1);
	}
	else if (lua_type(state, 1) == LUA_TFUNCTION)
	{
		for (;;)
		{
			lua_pushvalue(state, 1);
			if (lua_pcall(state, 0, 1, 0) != LUA_OK)
				return fail(state, "load: " + luaValueText(state, -1));
			if (lua_isnil(state, -1))
			{
				lua_pop(state, 1);
				break;
			}
			if (lua_type(state, -1) != LUA_TSTRING)
				return fail(state, "load: the reader function must return a string");
			std::string piece = luaArgString(state, -1);
			lua_pop(state, 1);
			if (piece.empty()) break;
			chunk += piece;
		}
	}
	else
	{
		return fail(state, "load: string or function expected");
	}
	std::string chunkName = lua_type(state, 2) == LUA_TSTRING ? luaArgString(state, 2) : (lua_type(state, 1) == LUA_TSTRING ? chunk : std::string("=(load)"));
	if (!chunk.empty() && chunk[0] == LUA_SIGNATURE[0])
	{
		lua_pushnil(state);
		lua_pushstring(state, "attempt to load a binary chunk (only text chunks are allowed)");
		return 2;
	}
	if (luaL_loadbufferx(state, chunk.data(), chunk.size(), chunkName.c_str(), "t") != LUA_OK)
	{
		lua_pushnil(state);
		lua_insert(state, -2);
		return 2;
	}
	if (!lua_isnone(state, 4))
	{
		lua_pushvalue(state, 4);
		if (!lua_setupvalue(state, -2, 1))
			lua_pop(state, 1);
	}
	return 1;
}

// ***************************************************************************
// loadfile(filename [, mode [, env]]): a script of the editor's directory, text only
R2H_FUNCTION(luaSafeLoadFile)
{
	std::string filename = luaValueText(state, 1);
	std::string chunk, error;
	if (!s_Host->readScript(filename, chunk, error))
	{
		lua_pushnil(state);
		lua_pushlstring(state, error.data(), error.size());
		return 2;
	}
	if (!chunk.empty() && chunk[0] == LUA_SIGNATURE[0])
	{
		lua_pushnil(state);
		lua_pushstring(state, "attempt to load a binary chunk (only text chunks are allowed)");
		return 2;
	}
	std::string chunkName = "@" + filename;
	if (luaL_loadbufferx(state, chunk.data(), chunk.size(), chunkName.c_str(), "t") != LUA_OK)
	{
		lua_pushnil(state);
		lua_insert(state, -2);
		return 2;
	}
	if (!lua_isnone(state, 3))
	{
		lua_pushvalue(state, 3);
		if (!lua_setupvalue(state, -2, 1))
			lua_pop(state, 1);
	}
	return 1;
}

// ***************************************************************************
// r2 functions
// ***************************************************************************
R2H_FUNCTION(luaRegisterGenerator)
{
	if (lua_type(state, 1) != LUA_TTABLE)
		return fail(state, "registerGenerator: table expected");
	CObject::TSmartPtr object = CHeadlessHost::getObjectFromLua(state, 1);
	if (!object || !object->isString("Name"))
		return fail(state, "registerGenerator: class without a Name");
	// the generator holds the class description
	s_Host->m_Factory->registerGenerator(object);
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaNewComponent)
{
	if (lua_type(state, 1) != LUA_TSTRING)
		return fail(state, "newComponent: string expected");
	CObject::TSmartPtr object = s_Host->m_Factory->newComponent(luaArgString(state, 1));
	setObjectToLua(state, object);
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaAddPaletteElement)
{
	if (lua_type(state, 1) != LUA_TSTRING || lua_type(state, 2) != LUA_TTABLE)
		return fail(state, "addPaletteElement: string and table expected");
	std::string key = luaArgString(state, 1);
	CObject::TSmartPtr object = CHeadlessHost::getObjectFromLua(state, 2);
	// the client warns and keeps the first one
	if (!s_Host->m_Palette.insert(std::make_pair(key, object)).second)
		s_Host->log("host", "Palette element added twice : " + key);
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaGetPaletteElement)
{
	if (lua_type(state, 1) != LUA_TSTRING)
		return fail(state, "getPaletteElement: string expected");
	std::map<std::string, CObject::TSmartPtr>::const_iterator it = s_Host->m_Palette.find(luaArgString(state, 1));
	setObjectToLua(state, it == s_Host->m_Palette.end() ? nullptr : it->second.getPtr());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaIsInPalette)
{
	if (lua_type(state, 1) != LUA_TSTRING)
		return fail(state, "isInPalette: string expected");
	lua_pushboolean(state, s_Host->m_Palette.count(luaArgString(state, 1)) ? 1 : 0);
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaGetPropertyValue)
{
	if (lua_type(state, 1) != LUA_TTABLE || lua_type(state, 2) != LUA_TSTRING)
		return fail(state, "getPropertyValue: table and string expected");
	CObject::TSmartPtr object = CHeadlessHost::getObjectFromLua(state, 1);
	const CObject *value = object ? s_Host->getPropertyValue(object, luaArgString(state, 2)) : nullptr;
	setObjectToLua(state, value);
	return 1;
}

// ***************************************************************************
// Node requests and the action historic, as CActionHistoric (ryzom/client/src/r2/dmc/action_historic.cpp).
//
// A request on a ghost node is applied at once ("direct effect, assumed to be local display only"), when
// the historic holds a scenario. Any other request joins the action being recorded. When that action
// ends its requests are sent on, and the client sees their effect at the end of the frame
// (CFrameActionsRecorder). An action that has not ended when the historic is cleared is dropped with its
// requests: r2.clearActionHistoric does that, and so does the end of CEditor::scenarioUpdated.
//
// A request that is sent on goes to the server, which applies it to the scenario it has and answers; the
// client applies it to its own when the answer comes. Both are done here at the end of the frame. A
// request that is applied at once is the client's alone.
// ***************************************************************************
void CHeadlessHost::applyRequest(CScenario *scenario, const CNodeRequest &request, const char *whose)
{
	bool ok = false;
	switch (request.Kind)
	{
	case CNodeRequest::Insert:
		ok = scenario->insertNode(request.InstanceId, request.AttrName, request.Position, request.Key, request.Value->clone());
		break;
	case CNodeRequest::Set: {
		CObject::TSmartPtr value = request.Value->clone();
		ok = scenario->setNode(request.InstanceId, request.AttrName, value);
		break;
	}
	case CNodeRequest::Erase:
		ok = scenario->eraseNode(request.InstanceId, request.AttrName, request.Position);
		break;
	case CNodeRequest::Move:
		ok = scenario->moveNode(request.InstanceId, request.AttrName, request.Position, request.DestInstanceId, request.DestAttrName, request.DestPosition);
		break;
	}
	if (!ok)
		log("host", toString("%s refused by the scenario%s: attribute '%s'%s", requestKindName(request.Kind, false), whose, request.AttrName.c_str(), request.Where.empty() ? "" : (" (" + request.Where + ")").c_str()));
}

// ***************************************************************************
void CHeadlessHost::submitRequest(lua_State *state, const CNodeRequest &request, bool direct)
{
	// named after the function the script called
	std::string kind = requestKindName(request.Kind, request.GhostCall);
	if (direct)
	{
		++m_RequestCounts[kind + " applied at once"];
		applyRequest(m_Scenario, request, "");
		return;
	}
	++m_RequestCounts[kind + " recorded"];
	m_HasOpenAction = true;
	m_OpenAction.push_back(request);
	m_OpenAction.back().Where = callerLocation(state);
}

// ***************************************************************************
void CHeadlessHost::flushOpenAction()
{
	for (size_t i = 0; i < m_OpenAction.size(); ++i)
	{
		++m_RequestCounts[std::string(requestKindName(m_OpenAction[i].Kind, false)) + " sent on at the end of its action"];
		m_FrameRequests.push_back(m_OpenAction[i]);
	}
	m_OpenAction.clear();
}

// ***************************************************************************
void CHeadlessHost::dropOpenAction(const char *reason, bool late)
{
	for (size_t i = 0; i < m_OpenAction.size(); ++i)
	{
		const CNodeRequest &request = m_OpenAction[i];
		std::string kind = requestKindName(request.Kind, false);
		++m_RequestCounts[kind + " " + reason];
		// what was lost: the class of the instance and the attribute, both names of the editor
		std::string className = "?";
		CObject *instance = m_Scenario->getHighLevel() ? m_Scenario->find(request.InstanceId) : nullptr;
		if (instance && instance->isString("Class"))
			className = instance->toString("Class");
		std::string what = kind + " " + className + "." + (request.Kind == CNodeRequest::Insert && !request.Key.empty() ? request.Key : request.AttrName);
		TUseMap &dropped = late ? m_LateRequests[m_Phase] : m_DroppedRequests[m_Phase];
		if (dropped.find(what) == dropped.end())
			dropped[what] = request.Where;
	}
	m_OpenAction.clear();
	m_HasOpenAction = false;
	m_OpenActionIsPending = false;
}

// ***************************************************************************
void CHeadlessHost::endAction()
{
	// as CActionHistoric::endAction
	if (!m_HasOpenAction) return;
	if (m_SubActionCount == 0) return;
	--m_SubActionCount;
	if (m_SubActionCount > 0) return; // several actions expected before the merge
	// the client sends the requests of a pending action on as they come (flushPendingAction, each frame)
	flushOpenAction();
	m_HasOpenAction = false;
	m_OpenActionIsPending = false;
}

// ***************************************************************************
void CHeadlessHost::endFrame()
{
	// as CFrameActionsRecorder::flush with CClientInstantActionFeedBack
	std::vector<CNodeRequest> requests;
	requests.swap(m_FrameRequests);
	for (size_t i = 0; i < requests.size(); ++i)
	{
		if (requests[i].Kind == CNodeRequest::Set && !requests[i].AttrName.empty())
		{
			// CClientEditionModule::requestSetNode sends nothing for a value the editor has already
			CObject *instance = m_Scenario->getHighLevel() ? m_Scenario->find(requests[i].InstanceId) : nullptr;
			const CObject *current = instance ? getPropertyValue(instance, requests[i].AttrName) : nullptr;
			if (current && current->equal(requests[i].Value))
			{
				++m_RequestCounts["setNode not sent: the editor has the value"];
				continue;
			}
		}
		++m_RequestCounts[std::string(requestKindName(requests[i].Kind, false)) + " applied at the end of the frame"];
		if (m_ServerScenario->getHighLevel())
			applyRequest(m_ServerScenario, requests[i], " of the server");
		applyRequest(m_Scenario, requests[i], "");
	}
}

// ***************************************************************************
int CHeadlessHost::requestInsertNode(lua_State *state, bool ghost)
{
	if (lua_gettop(state) != 5 || lua_type(state, 1) != LUA_TSTRING || lua_type(state, 2) != LUA_TSTRING
	    || lua_type(state, 3) != LUA_TNUMBER || lua_type(state, 4) != LUA_TSTRING)
		return fail(state, "requestInsertNode: (instanceId, attrName, position, key, value) expected");
	if (!s_Host->m_Scenario->getHighLevel())
		return fail(state, "requestInsertNode: no scenario");
	CNodeRequest request;
	request.Kind = CNodeRequest::Insert;
	request.InstanceId = luaArgString(state, 1);
	request.AttrName = luaArgString(state, 2);
	request.Position = (sint32)lua_tointeger(state, 3);
	request.Key = luaArgString(state, 4);
	request.Value = getObjectFromLua(state, 5);
	if (!request.Value)
		return fail(state, "requestInsertNode: the value is nil or of a type that gives no object (the client uses it without a test)");
	request.Value->setGhost(ghost);
	request.GhostCall = ghost;
	bool direct = false;
	if (s_Host->m_HistoricHasScenario)
	{
		CObject *target = s_Host->m_Scenario->find(request.InstanceId, request.AttrName);
		direct = ghost || (target && target->getGhost());
	}
	s_Host->submitRequest(state, request, direct);
	return 0;
}

R2H_FUNCTION(luaRequestInsertNode)
{
	return CHeadlessHost::requestInsertNode(state, false);
}

R2H_FUNCTION(luaRequestInsertGhostNode)
{
	return CHeadlessHost::requestInsertNode(state, true);
}

// ***************************************************************************
int CHeadlessHost::requestSetNode(lua_State *state, bool ghost)
{
	if (lua_gettop(state) != 3 || lua_type(state, 1) != LUA_TSTRING || lua_type(state, 2) != LUA_TSTRING)
		return fail(state, "requestSetNode: (instanceId, attrName, value) expected");
	if (!s_Host->m_Scenario->getHighLevel())
		return fail(state, "requestSetNode: no scenario");
	CNodeRequest request;
	request.Kind = CNodeRequest::Set;
	request.InstanceId = luaArgString(state, 1);
	request.AttrName = luaArgString(state, 2);
	request.Value = getObjectFromLua(state, 3);
	if (!request.Value)
	{
		// the client warns and ignores the request
		s_Host->log("host", "requestSetNode : bad type for argument 3 (attribute '" + request.AttrName + "', " + callerLocation(state) + ")");
		return 0;
	}
	request.Value->setGhost(ghost);
	request.GhostCall = ghost;
	bool direct = false;
	if (s_Host->m_HistoricHasScenario)
	{
		// the value takes the ghost flag of the attribute it replaces; an attribute the instance does not
		// have is no ghost, whatever the instance is
		CObject *current = s_Host->m_Scenario->find(request.InstanceId, request.AttrName);
		if (current && current->getGhost())
			request.Value->setGhost(true);
		direct = request.Value->getGhost();
	}
	s_Host->submitRequest(state, request, direct);
	return 0;
}

R2H_FUNCTION(luaRequestSetNode)
{
	return CHeadlessHost::requestSetNode(state, false);
}

R2H_FUNCTION(luaRequestSetGhostNode)
{
	return CHeadlessHost::requestSetNode(state, true);
}

// ***************************************************************************
R2H_FUNCTION(luaRequestEraseNode)
{
	int args = lua_gettop(state);
	if (args < 1 || args > 3 || lua_type(state, 1) != LUA_TSTRING
	    || (args > 1 && lua_type(state, 2) != LUA_TSTRING)
	    || (args > 2 && lua_type(state, 3) != LUA_TNUMBER))
		return fail(state, "requestEraseNode: (instanceId [, attrName [, position]]) expected");
	if (!s_Host->m_Scenario->getHighLevel())
		return fail(state, "requestEraseNode: no scenario");
	CNodeRequest request;
	request.Kind = CNodeRequest::Erase;
	request.InstanceId = luaArgString(state, 1);
	if (args > 1) request.AttrName = luaArgString(state, 2);
	if (args > 2) request.Position = (sint32)lua_tointeger(state, 3);
	bool direct = false;
	if (s_Host->m_HistoricHasScenario)
	{
		CObject *target = s_Host->m_Scenario->find(request.InstanceId, request.AttrName, request.Position);
		direct = target && target->getGhost();
	}
	s_Host->submitRequest(state, request, direct);
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaRequestMoveNode)
{
	if (lua_gettop(state) != 6 || lua_type(state, 1) != LUA_TSTRING || lua_type(state, 2) != LUA_TSTRING
	    || lua_type(state, 3) != LUA_TNUMBER || lua_type(state, 4) != LUA_TSTRING
	    || lua_type(state, 5) != LUA_TSTRING || lua_type(state, 6) != LUA_TNUMBER)
		return fail(state, "requestMoveNode: (instanceId, attrName, position, destInstanceId, destAttrName, destPosition) expected");
	if (!s_Host->m_Scenario->getHighLevel())
		return fail(state, "requestMoveNode: no scenario");
	CNodeRequest request;
	request.Kind = CNodeRequest::Move;
	request.InstanceId = luaArgString(state, 1);
	request.AttrName = luaArgString(state, 2);
	request.Position = (sint32)lua_tointeger(state, 3);
	request.DestInstanceId = luaArgString(state, 4);
	request.DestAttrName = luaArgString(state, 5);
	request.DestPosition = (sint32)lua_tointeger(state, 6);
	bool direct = false;
	if (s_Host->m_HistoricHasScenario)
	{
		CObject *source = s_Host->m_Scenario->find(request.InstanceId, request.AttrName, request.Position);
		CObject *destination = s_Host->m_Scenario->find(request.DestInstanceId, request.DestAttrName);
		direct = source && destination && source->getGhost();
	}
	s_Host->submitRequest(state, request, direct);
	return 0;
}

// ***************************************************************************
int CHeadlessHost::requestNewAction(lua_State * /* state */, bool pending, uint32 count)
{
	// as CComLuaModule::luaRequestNewAction and the CActionHistoric functions it calls
	if (count == 0) return 0;
	if (count > 1)
	{
		s_Host->m_SubActionCount = 1; // force previous multi action to finish
		s_Host->endAction();
		s_Host->m_OpenActionIsPending = pending;
		s_Host->m_SubActionCount = count;
	}
	else if (pending)
	{
		s_Host->endAction();
		s_Host->m_OpenActionIsPending = true;
	}
	else
	{
		// CDynamicMapClient::newAction: the frame's requests are applied, then newSingleAction
		s_Host->endFrame();
		s_Host->endAction();
		if (s_Host->m_SubActionCount == 0)
		{
			s_Host->m_SubActionCount = 1;
			s_Host->m_OpenActionIsPending = false;
		}
	}
	return 0;
}

int CHeadlessHost::luaRequestNewAction(lua_State *state)
{
	return requestNewAction(state, false, 1);
}

int CHeadlessHost::luaRequestNewPendingAction(lua_State *state)
{
	return requestNewAction(state, true, 1);
}

R2H_FUNCTION(luaRequestNewMultiAction)
{
	if (lua_gettop(state) != 2 || lua_type(state, 2) != LUA_TNUMBER)
		return fail(state, "requestNewMultiAction: (name, count) expected");
	return CHeadlessHost::requestNewAction(state, false, (uint32)lua_tointeger(state, 2));
}

R2H_FUNCTION(luaRequestNewPendingMultiAction)
{
	if (lua_gettop(state) != 2 || lua_type(state, 2) != LUA_TNUMBER)
		return fail(state, "requestNewPendingMultiAction: (name, count) expected");
	return CHeadlessHost::requestNewAction(state, true, (uint32)lua_tointeger(state, 2));
}

int CHeadlessHost::luaRequestEndAction(lua_State * /* state */)
{
	s_Host->endAction();
	return 0;
}

int CHeadlessHost::luaRequestForceEndMultiAction(lua_State * /* state */)
{
	// as CActionHistoric::forceEndMultiAction
	if (s_Host->m_SubActionCount > 1) s_Host->m_SubActionCount = 1;
	s_Host->endAction();
	return 0;
}

int CHeadlessHost::luaRequestCancelAction(lua_State * /* state */)
{
	// as CActionHistoric::cancelAction
	s_Host->dropOpenAction("cancelled", false);
	s_Host->m_SubActionCount = 1;
	return 0;
}

int CHeadlessHost::luaClearActionHistoric(lua_State * /* state */)
{
	// as CActionHistoric::clear without a scenario: the action being recorded goes, and the historic holds no
	// scenario until the end of the scenario update
	s_Host->dropOpenAction("dropped by r2.clearActionHistoric", false);
	s_Host->m_HistoricHasScenario = false;
	return 0;
}

int CHeadlessHost::luaRequestUploadCurrentScenario(lua_State * /* state */)
{
	// The client sends the scenario to the service and keeps a copy; when the service acknowledges, the
	// copy comes back as a scenario update (CServerAnswerMsgScenarioUploaded, onScenarioUploaded).
	CObject *highLevel = s_Host->m_Scenario->getHighLevel();
	if (highLevel)
		s_Host->m_UploadedScenario = highLevel->clone();
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaRequestCreateScenario)
{
	// as CComLuaModule::luaRequestCreateScenario: the action historic is cleared, and the scenario of the
	// table is sent to the service, from which it comes back as a scenario update
	if (lua_type(state, 1) != LUA_TTABLE)
		return fail(state, "requestCreateScenario: table expected");
	CObject::TSmartPtr object = CHeadlessHost::getObjectFromLua(state, 1);
	if (!object)
		return fail(state, "requestCreateScenario: the table gives no scenario");
	s_Host->dropOpenAction("dropped by r2.requestCreateScenario", false);
	s_Host->m_HistoricHasScenario = false;
	s_Host->m_UploadedScenario = object;
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaGetScenarioObj)
{
	// as CComLuaModule::luaGetScenarioObj: the scenario of the editor as a table; getScenarioObj(true)
	// leaves out what the editor adds to the scenario for itself, as a save does
	bool withGhosts = !(lua_gettop(state) >= 1 && lua_toboolean(state, 1));
	setObjectToLua(state, s_Host->m_Scenario->getHighLevel(), withGhosts);
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaRequestSetScenarioVersions)
{
	// as CComLuaModule::luaRequestSetScenarioVersions: the list goes to the server outside of the action
	// historic (CClientEditionModule::requestSetNodeNoTest), and the editor sets it on its own scenario
	if (lua_type(state, 1) != LUA_TTABLE)
		return fail(state, "requestSetScenarioVersions: table expected");
	CObject *highLevel = s_Host->m_Scenario->getHighLevel();
	if (!highLevel || !highLevel->isString("InstanceId"))
		return 0;
	CNodeRequest request;
	request.Kind = CNodeRequest::Set;
	request.InstanceId = highLevel->toString("InstanceId");
	request.AttrName = "Versions";
	request.Value = getObjectFromLua(state, 1);
	if (!request.Value)
		return 0;
	request.Where = callerLocation(state);
	if (s_Host->m_ServerScenario->getHighLevel())
	{
		++s_Host->m_RequestCounts["setScenarioVersions sent to the server"];
		s_Host->applyRequest(s_Host->m_ServerScenario, request, " of the server");
	}
	++s_Host->m_RequestCounts["setScenarioVersions set in the editor"];
	s_Host->applyRequest(s_Host->m_Scenario, request, "");
	return 0;
}

// ***************************************************************************
bool CHeadlessHost::addFeature(const CObject *component, std::string &error)
{
	lua_pushcfunction(m_Lua, luaTraceback);
	int handler = lua_gettop(m_Lua);
	lua_getglobal(m_Lua, "r2Headless");
	lua_getfield(m_Lua, -1, "addFeature");
	lua_remove(m_Lua, -2);
	setObjectToLua(m_Lua, component);
	bool ok = lua_pcall(m_Lua, 1, 0, handler) == LUA_OK;
	if (!ok) error = luaValueText(m_Lua, -1);
	lua_settop(m_Lua, handler - 1);
	return ok;
}

// ***************************************************************************
bool CHeadlessHost::updateLoadedScenario(CObject::TSmartPtr &scenario, const std::string &place, bool &untouched, std::string &error)
{
	// As the client does with a scenario it has read from a file (CComLuaModule::updateLoadedScenario):
	// r2.Version.updateLoadedScenario gets the scenario as a table and returns nil (the scenario is used as
	// it is), false (the update failed) or the table to use in its place.
	// The table also goes back through the driver, whatever the function returned: when the scenario is
	// to be used as it is, the table must still give the scenario as it was read.
	untouched = true;
	std::string before;
	scenario->serialize(before);
	lua_pushcfunction(m_Lua, luaTraceback);
	int handler = lua_gettop(m_Lua);
	lua_getglobal(m_Lua, "r2Headless");
	lua_getfield(m_Lua, -1, "updateLoadedScenario");
	lua_remove(m_Lua, -2);
	setObjectToLua(m_Lua, scenario);
	lua_pushlstring(m_Lua, place.data(), place.size());
	if (lua_pcall(m_Lua, 2, 2, handler) != LUA_OK)
	{
		error = luaValueText(m_Lua, -1);
		lua_settop(m_Lua, handler - 1);
		return false;
	}
	// the result of r2.Version.updateLoadedScenario, then the table it was given
	bool failed = lua_type(m_Lua, -2) == LUA_TBOOLEAN && !lua_toboolean(m_Lua, -2);
	bool replaced = lua_type(m_Lua, -2) == LUA_TTABLE;
	CObject::TSmartPtr after = getObjectFromLua(m_Lua, -1);
	lua_settop(m_Lua, handler - 1);
	if (failed)
	{
		error = "r2.Version.updateLoadedScenario returned false";
		return false;
	}
	if (!after)
	{
		error = "the table of the scenario gives no scenario after the update";
		return false;
	}
	std::string afterText;
	after->serialize(afterText);
	untouched = afterText == before;
	if (replaced) scenario = after;
	return true;
}

// ***************************************************************************
// The largest number an instance of the tree has after baseName in its name ("<baseName> <number>"), as
// CEditor::getGeneratedNameIndex over the instances of the editor
static void largestNameIndex(const CObject *node, const std::string &baseName, sint &largest)
{
	if (!node || !node->isTable()) return;
	if (node->isString("InstanceId") && node->isString("Name"))
	{
		std::string name = node->toString("Name");
		if (name.size() >= baseName.size() + 2 && name.compare(0, baseName.size(), baseName) == 0 && name[baseName.size()] == ' ')
		{
			size_t pos = baseName.size() + 1;
			while (pos < name.size() && name[pos] >= '0' && name[pos] <= '9') ++pos;
			if (pos == name.size())
			{
				sint index = 0;
				fromString(name.substr(baseName.size() + 1), index);
				largest = std::max(largest, index);
			}
		}
	}
	for (uint32 i = 0; i < node->getSize(); ++i)
		largestNameIndex(node->getValueAtPos(i), baseName, largest);
}

R2H_FUNCTION(luaGenInstanceName)
{
	// r2:genInstanceName(baseName), as CEditor::genInstanceName: the name without the number and the
	// spaces at its end, with the number after the largest one an instance of the scenario has for it
	if (lua_gettop(state) != 2)
		return fail(state, "genInstanceName: (baseName) expected");
	std::string baseName = luaValueText(state, 2);
	sint last = (sint)baseName.size() - 1;
	while (last > 0)
	{
		char c = baseName[last];
		if (!(c >= '0' && c <= '9') && c != ' ' && c != '\t') break;
		--last;
	}
	baseName.resize(last + 1);
	sint largest = 0;
	largestNameIndex(s_Host->m_Scenario->getHighLevel(), baseName, largest);
	std::string name = baseName + " " + toString(largest + 1);
	lua_pushlstring(state, name.data(), name.size());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaGetInstanceFromId)
{
	// r2:getInstanceFromId(id)
	if (lua_gettop(state) != 2 || lua_type(state, 2) != LUA_TSTRING)
		return fail(state, std::string("getInstanceFromId: string expected, got ") + luaL_typename(state, 2));
	CObject *found = nullptr;
	if (s_Host->m_Scenario->getHighLevel())
		found = s_Host->m_Scenario->find(luaArgString(state, 2));
	if (found && found->isTable())
		s_Host->pushProjection(state, found);
	else
		lua_pushnil(state);
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaGetNamespace(lua_State *state)
{
	// "r2_%04d_" with the number of the edition session
	s_Host->recordStubCall(state, "r2.getNamespace = 'r2_0001_' (the session number of the client)");
	lua_pushstring(state, "r2_0001_");
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaSetStartingActIndex(lua_State *state)
{
	s_Host->m_StartingActIndex = (uint32)lua_tointeger(state, 1);
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaGetSheetIdName)
{
	uint32 value = (uint32)lua_tointeger(state, 1);
	std::string name;
	CSheetId sheetId(value);
	if (sheetId != CSheetId::Unknown)
		name = sheetId.toString();
	// without sheet_id.bin the name is a placeholder, which ends up in the run-time tree
	if (!CPath::exists("sheet_id.bin"))
		s_Host->recordStubCall(state, "r2.getSheetIdName (no sheet_id.bin: placeholder name)");
	lua_pushstring(state, name.c_str());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaPrint)
{
	s_Host->log("print", luaValueText(state, 1));
	return 0;
}

// ***************************************************************************
// Globals
// ***************************************************************************
R2H_FUNCTION(luaDebugInfo)
{
	s_Host->log("debugInfo", luaValueText(state, 1));
	return 0;
}

R2H_FUNCTION(luaMessageBox)
{
	s_Host->log("messageBox", luaValueText(state, 1));
	return 0;
}

R2H_FUNCTION(luaDisplaySystemInfo)
{
	s_Host->log("systemInfo", luaValueText(state, 1));
	return 0;
}

R2H_FUNCTION(luaDumpCallStack)
{
	luaL_traceback(state, state, "call stack", 1);
	s_Host->log("debugInfo", luaValueText(state, -1));
	return 0;
}

// ***************************************************************************
R2H_FUNCTION(luaI18nGet)
{
	// the client gives a ucstring object, which the scripts on this path only concatenate
	std::string label = luaValueText(state, 1);
	if (s_Host->m_TranslationLoaded)
	{
		const std::string &text = CI18N::get(label);
		lua_pushlstring(state, text.data(), text.size());
	}
	else
	{
		s_Host->recordStubCall(state, "i18n.get (no translations: the label itself)");
		lua_pushlstring(state, label.data(), label.size());
	}
	return 1;
}

R2H_FUNCTION(luaI18nHasTranslation)
{
	std::string label = luaValueText(state, 1);
	if (!s_Host->m_TranslationLoaded)
		s_Host->recordStubCall(state, "i18n.hasTranslation (no translations: false)");
	lua_pushboolean(state, (s_Host->m_TranslationLoaded && CI18N::hasTranslation(label)) ? 1 : 0);
	return 1;
}

// ***************************************************************************
// Sheets. The client answers these from its packed sheets; here the creature sheets come from the
// leveldesign directory through Georges when one is given.
// ***************************************************************************
void CHeadlessHost::setSheetDirectory(const std::string &leveldesign)
{
	CPath::addSearchPath(leveldesign + "/DFN", true, false);
	CPath::addSearchPath(leveldesign + "/game_elem", true, false);
	CPath::addSearchPath(leveldesign + "/game_element", true, false);
	m_FormLoader = NLGEORGES::UFormLoader::createLoader();
}

// ***************************************************************************
bool CHeadlessHost::getCreatureSkeleton(const std::string &sheet, std::string &skeleton)
{
	std::map<std::string, std::string>::const_iterator it = m_SkeletonCache.find(sheet);
	if (it != m_SkeletonCache.end())
	{
		skeleton = it->second;
		return true;
	}
	if (!m_FormLoader) return false;
	skeleton.clear();
	if (CPath::exists(sheet))
	{
		CSmartPtr<NLGEORGES::UForm> form = m_FormLoader->loadForm(sheet.c_str());
		if (form)
			form->getRootNode().getValueByName(skeleton, "3d data.Skel");
	}
	else
	{
		log("host", "getCharacterSheetSkel: no sheet " + sheet);
	}
	m_SkeletonCache[sheet] = skeleton;
	return true;
}

// ***************************************************************************
R2H_FUNCTION(luaGetCharacterSheetSkel)
{
	// getCharacterSheetSkel(sheet, isMale), used by r2:isBotObject: a creature without a skeleton is an object
	std::string sheet = luaValueText(state, 1);
	std::string skeleton;
	if (!s_Host->getCreatureSkeleton(sheet, skeleton))
	{
		// no sheets: every creature is taken to have a skeleton. r2:isBotObject also tests the name, so
		// the object_*.creature sheets are still found.
		s_Host->recordStubCall(state, "getCharacterSheetSkel (no sheets: approximated)");
		skeleton = "unknown.skel";
	}
	lua_pushlstring(state, skeleton.data(), skeleton.size());
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaSnapPosToGround(lua_State *state)
{
	// r2:snapPosToGround(x, y): the client intersects the island's height map and collisions. Without the
	// world the answer is the one CEditor::luaSnapPosToGround gives when nothing is hit: x, y and a z of 0.
	s_Host->recordStubCall(state, "r2.snapPosToGround (no world: z = 0)");
	lua_settop(state, 3);
	lua_pushnumber(state, 0.0);
	return 3;
}

// ***************************************************************************
bool CHeadlessHost::getCreatureField(const std::string &sheet, const char *field, std::string &value)
{
	// the fields of a creature sheet the client keeps in CCharacterSheet
	value.clear();
	if (!m_FormLoader || !CPath::exists(sheet)) return false;
	CSmartPtr<NLGEORGES::UForm> form = m_FormLoader->loadForm(sheet.c_str());
	if (!form) return false;
	return form->getRootNode().getValueByName(value, field);
}

// ***************************************************************************
R2H_FUNCTION(luaGetR2PlayerRace)
{
	// getR2PlayerRace(sheet), as CLuaIHMRyzom::getR2PlayerRace: the race of the creature sheet
	std::string sheet = luaValueText(state, 1);
	std::string race;
	if (s_Host->getCreatureField(sheet, "Basics.Race", race))
	{
		race = EGSPD::CPeople::toString(EGSPD::CPeople::fromString(race));
	}
	else
	{
		// no sheets: the sheets of the player races have the race in their name
		s_Host->recordStubCall(state, "getR2PlayerRace (no sheets: from the name of the sheet)");
		static const char *const s_Races[][2] = { { "fyros", "Fyros" }, { "matis", "Matis" }, { "tryker", "Tryker" }, { "zorai", "Zorai" } };
		for (size_t i = 0; i < sizeof(s_Races) / sizeof(s_Races[0]); ++i)
		{
			if (sheet.find(s_Races[i][0]) != std::string::npos) race = s_Races[i][1];
		}
	}
	lua_pushlstring(state, race.data(), race.size());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaIsR2PlayerMale)
{
	// isR2PlayerMale(sheet), as CLuaIHMRyzom::isR2PlayerMale: a sheet the client does not have is male
	std::string sheet = luaValueText(state, 1);
	std::string gender;
	bool male = true;
	if (s_Host->getCreatureField(sheet, "Basics.Gender", gender))
	{
		uint8 value = 0;
		fromString(gender, value);
		male = value == 0; // GSGENDER::male
	}
	else
	{
		s_Host->recordStubCall(state, "isR2PlayerMale (no sheets: from the name of the sheet)");
		male = sheet.find("female") == std::string::npos;
	}
	lua_pushboolean(state, male ? 1 : 0);
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaFileLookup)
{
	// fileLookup(name), as CLuaIHM::fileLookup: the path of a file of the search paths, "" when there is none
	std::string path = CPath::lookup(luaValueText(state, 1), false);
	lua_pushlstring(state, path.data(), path.size());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaGetSheetId)
{
	std::string name = luaValueText(state, 1);
	if (!CPath::exists("sheet_id.bin"))
		s_Host->recordStubCall(state, "getSheetId (no sheet_id.bin: unknown sheet)");
	lua_pushinteger(state, (lua_Integer)(sint32)CSheetId(name).asInt());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaGetEmoteBehaviorFromEmoteId)
{
	static CEmoteBehavior s_EmoteBehavior;
	std::string behavior = s_EmoteBehavior.get(luaValueText(state, 1));
	lua_pushlstring(state, behavior.data(), behavior.size());
	return 1;
}

// ***************************************************************************
R2H_FUNCTION(luaGetDefine)
{
	// a define of the interface; the client reads them from the interface files it has loaded
	std::string name = luaValueText(state, 1);
	for (size_t i = 0; i < sizeof(s_Defines) / sizeof(s_Defines[0]); ++i)
	{
		if (name == s_Defines[i][0])
		{
			s_Host->recordStubCall(state, "getDefine('" + name + "') = '" + s_Defines[i][1] + "' (the value of r2ed_scenario.xml, built in)");
			lua_pushstring(state, s_Defines[i][1]);
			return 1;
		}
	}
	return fail(state, "getDefine: the host has no value for '" + name + "'");
}

// ***************************************************************************
R2H_FUNCTION(luaGetClientCfgVar)
{
	std::string name = luaValueText(state, 1);
	if (name == "BuildName")
	{
		// the version name the editor gives a scenario it saves
		s_Host->recordStubCall(state, "getClientCfgVar('BuildName') = 'headless'");
		lua_pushstring(state, "headless");
		return 1;
	}
	for (size_t i = 0; i < sizeof(s_ClientCfgVars) / sizeof(s_ClientCfgVars[0]); ++i)
	{
		if (name == s_ClientCfgVars[i][0])
		{
			s_Host->recordStubCall(state, toString("getClientCfgVar('%s') = %s (the value of client_default.cfg, built in)", s_ClientCfgVars[i][0], s_ClientCfgVars[i][1]));
			lua_pushstring(state, s_ClientCfgVars[i][1]);
			return 1;
		}
	}
	return fail(state, "getClientCfgVar: the host has no value for '" + name + "'");
}

// ***************************************************************************
R2H_FUNCTION(luaGetCompleteIslands)
{
	// getCompleteIslands(): island name to { continent, xmin, ymin, xmax, ymax, entrypoints }, as
	// CLuaIHMRyzom::getCompleteIslands. The islands come from the island files of the client data, when they
	// are on the search path.
	lua_newtable(state);
	bool haveIslands = CPath::exists("r2_entry_points.txt");
	static const char *const s_IslandFiles[] = { "r2_desert_islands.xml", "r2_lakes_islands.xml", "r2_jungle_islands.xml", "r2_forest_islands.xml", "r2_roots_islands.xml" };
	for (size_t i = 0; i < sizeof(s_IslandFiles) / sizeof(s_IslandFiles[0]); ++i)
		haveIslands = haveIslands && CPath::exists(s_IslandFiles[i]);
	if (!haveIslands)
	{
		s_Host->recordStubCall(state, "getCompleteIslands (no island files: no island)");
		return 1;
	}
	const CScenarioEntryPoints::TCompleteIslands &islands = CScenarioEntryPoints::getInstance().getCompleteIslands();
	for (size_t i = 0; i < islands.size(); ++i)
	{
		const CScenarioEntryPoints::CCompleteIsland &island = islands[i];
		lua_newtable(state);
		lua_pushstring(state, island.Continent.c_str());
		lua_setfield(state, -2, "continent");
		lua_pushinteger(state, island.XMin);
		lua_setfield(state, -2, "xmin");
		lua_pushinteger(state, island.YMin);
		lua_setfield(state, -2, "ymin");
		lua_pushinteger(state, island.XMax);
		lua_setfield(state, -2, "xmax");
		lua_pushinteger(state, island.YMax);
		lua_setfield(state, -2, "ymax");
		lua_newtable(state);
		for (size_t e = 0; e < island.EntryPoints.size(); ++e)
		{
			lua_newtable(state);
			lua_pushinteger(state, island.EntryPoints[e].X);
			lua_setfield(state, -2, "x");
			lua_pushinteger(state, island.EntryPoints[e].Y);
			lua_setfield(state, -2, "y");
			lua_setfield(state, -2, island.EntryPoints[e].Location.c_str());
		}
		lua_setfield(state, -2, "entrypoints");
		lua_setfield(state, -2, island.Island.c_str());
	}
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaColor(lua_State *state)
{
	// CRGBA(r, g, b, a): the client gives a colour object; the scripts on this path only store it
	s_Host->recordStubCall(state, "CRGBA (a table in place of a colour)");
	lua_settop(state, 4);
	lua_newtable(state);
	static const char *const s_Fields[] = { "R", "G", "B", "A" };
	for (int i = 0; i < 4; ++i)
	{
		if (lua_isnil(state, i + 1))
			lua_pushinteger(state, i == 3 ? 255 : 0);
		else
			lua_pushvalue(state, i + 1);
		lua_setfield(state, -2, s_Fields[i]);
	}
	return 1;
}

// ***************************************************************************
int CHeadlessHost::luaTimeMs(lua_State *state)
{
	lua_pushinteger(state, (lua_Integer)CTime::getLocalTime());
	return 1;
}

// ***************************************************************************
void CHeadlessHost::recordStubCall(lua_State *state, const std::string &name)
{
	TUseMap &uses = m_CalledStubs[m_Phase];
	if (uses.find(name) != uses.end()) return;
	uses[name] = callerLocation(state);
	log("stub", name);
}

// ***************************************************************************
int CHeadlessHost::luaStub(lua_State *state)
{
	const char *name = lua_tostring(state, lua_upvalueindex(1));
	TStubResult result = (TStubResult)lua_tointeger(state, lua_upvalueindex(2));
	s_Host->recordStubCall(state, name);
	switch (result)
	{
	case StubFalse:
		lua_pushboolean(state, 0);
		return 1;
	case StubEmptyTable:
		lua_newtable(state);
		return 1;
	case StubVisualProperties:
		lua_pushstring(state, "VPA:0000000000000000\nVPB:0000000000000000\nVPC:0000000000000000");
		return 1;
	default:
		return 0;
	}
}

// ***************************************************************************
int CHeadlessHost::luaTracked(lua_State *state)
{
	// counts the call, then runs the host function on the same stack
	++s_Host->m_HostCalls[s_Host->m_Phase][lua_tostring(state, lua_upvalueindex(1))];
	lua_CFunction function = lua_tocfunction(state, lua_upvalueindex(2));
	return function(state);
}

// ***************************************************************************
void CHeadlessHost::pushTracked(lua_State *state, const char *name, int (*function)(lua_State *))
{
	lua_pushstring(state, name);
	lua_pushcfunction(state, function);
	lua_pushcclosure(state, luaTracked, 2);
}

// ***************************************************************************
int CHeadlessHost::luaUndefinedRead(lua_State *state)
{
	// __index of the globals, of r2 and of config: the name with the place of the first read, and nil as
	// without it. config gives the values the host has built in, and records that.
	if (lua_type(state, 2) == LUA_TSTRING)
	{
		std::string prefix = lua_tostring(state, lua_upvalueindex(1));
		std::string field = lua_tostring(state, 2);
		if (prefix == "config.")
		{
			for (size_t i = 0; i < sizeof(s_ConfigValues) / sizeof(s_ConfigValues[0]); ++i)
			{
				if (field == s_ConfigValues[i][0])
				{
					s_Host->recordStubCall(state, "config." + field + " = " + s_ConfigValues[i][1] + " (the value of client_default.cfg, built in)");
					lua_pushinteger(state, atoi(s_ConfigValues[i][1]));
					return 1;
				}
			}
		}
		TUseMap &uses = s_Host->m_UndefinedReads[s_Host->m_Phase];
		std::string name = prefix + field;
		if (uses.find(name) == uses.end())
			uses[name] = callerLocation(state);
	}
	lua_pushnil(state);
	return 1;
}

// ***************************************************************************
void CHeadlessHost::setProbe(lua_State *state, const char *prefix)
{
	// table on the top of the stack gets a metatable that records the reads of undefined fields
	lua_newtable(state);
	lua_pushstring(state, prefix);
	lua_pushcclosure(state, luaUndefinedRead, 1);
	lua_setfield(state, -2, "__index");
	lua_setmetatable(state, -2);
}

// ***************************************************************************
void CHeadlessHost::openLibraries()
{
	// The libraries NLGUI::CLuaState gives the scripts of a release client (openStandardLibraries in
	// nel/src/gui/lua_helper.cpp): nothing that reaches the file system, the process or the internals of the
	// VM. No io, no package; os has its clock functions; dofile is removed; load and loadfile take text only.
	// debug is opened whole here for the driver script, and reduceLibraries() leaves traceback.
	lua_State *state = m_Lua;
	openStandardLibrary(state, "_G", luaopen_base);
	openStandardLibrary(state, LUA_COLIBNAME, luaopen_coroutine);
	openStandardLibrary(state, LUA_TABLIBNAME, luaopen_table);
	openStandardLibrary(state, LUA_STRLIBNAME, luaopen_string);
	openStandardLibrary(state, LUA_MATHLIBNAME, luaopen_math);
	openStandardLibrary(state, LUA_UTF8LIBNAME, luaopen_utf8);
#if LUA_VERSION_NUM == 503 && defined(LUA_COMPAT_BITLIB)
	openStandardLibrary(state, LUA_BITLIBNAME, luaopen_bit32);
#endif
	openStandardLibrary(state, LUA_OSLIBNAME, luaopen_os);
	static const char *const s_OsSubset[] = { "time", "date", "clock", "difftime", nullptr };
	reduceStandardLibrary(state, LUA_OSLIBNAME, s_OsSubset);
	openStandardLibrary(state, LUA_DBLIBNAME, luaopen_debug);

	lua_pushnil(state);
	lua_setglobal(state, "dofile");
	lua_pushcfunction(state, luaSafeLoadFile);
	lua_setglobal(state, "loadfile");
	lua_pushcfunction(state, luaSafeLoad);
	lua_setglobal(state, "load");
	lua_pushcfunction(state, luaSafeLoad);
	lua_setglobal(state, "loadstring"); // the 5.1 name, which the scripts use

	// unpack moved into the table library in 5.2: both names
	lua_getglobal(state, LUA_TABLIBNAME);
	lua_getfield(state, -1, "unpack");
	lua_setglobal(state, "unpack");
	lua_pop(state, 1);
}

// ***************************************************************************
void CHeadlessHost::reduceLibraries()
{
	static const char *const s_DebugSubset[] = { "traceback", nullptr };
	reduceStandardLibrary(m_Lua, LUA_DBLIBNAME, s_DebugSubset);
}

// ***************************************************************************
void CHeadlessHost::registerFunctions()
{
	lua_State *state = m_Lua;

	// projection metatable, cache of the projections (weak values) and user tables
	lua_newtable(state);
	lua_pushcfunction(state, luaProjectionIndex);
	lua_setfield(state, -2, "__index");
	lua_pushcfunction(state, luaProjectionNewIndex);
	lua_setfield(state, -2, "__newindex");
	lua_pushcfunction(state, luaProjectionNext);
	lua_setfield(state, -2, "__next");
	lua_pushcfunction(state, luaProjectionEqual);
	lua_setfield(state, -2, "__eq");
	lua_pushcfunction(state, luaProjectionGc);
	lua_setfield(state, -2, "__gc");
	lua_pushcfunction(state, luaProjectionToString);
	lua_setfield(state, -2, "__tostring");
	lua_pushstring(state, "CObjectTable");
	lua_setfield(state, -2, "tag");
	lua_rawsetp(state, LUA_REGISTRYINDEX, &s_ProjectionMetatableKey);

	lua_newtable(state);
	lua_newtable(state);
	lua_pushstring(state, "v");
	lua_setfield(state, -2, "__mode");
	lua_setmetatable(state, -2);
	lua_rawsetp(state, LUA_REGISTRYINDEX, &s_ProjectionCacheKey);

	lua_newtable(state);
	lua_rawsetp(state, LUA_REGISTRYINDEX, &s_UserTablesKey);

	// r2
	static const luaL_Reg s_R2Functions[] = {
		{ "doFile", luaDoFile },
		{ "registerGenerator", luaRegisterGenerator },
		{ "newComponent", luaNewComponent },
		{ "addPaletteElement", luaAddPaletteElement },
		{ "getPaletteElement", luaGetPaletteElement },
		{ "isInPalette", luaIsInPalette },
		{ "getPropertyValue", luaGetPropertyValue },
		{ "requestInsertNode", luaRequestInsertNode },
		{ "requestInsertGhostNode", luaRequestInsertGhostNode },
		{ "requestSetNode", luaRequestSetNode },
		{ "requestSetGhostNode", luaRequestSetGhostNode },
		{ "requestEraseNode", luaRequestEraseNode },
		{ "requestMoveNode", luaRequestMoveNode },
		{ "requestNewAction", luaRequestNewAction },
		{ "requestNewPendingAction", luaRequestNewPendingAction },
		{ "requestNewMultiAction", luaRequestNewMultiAction },
		{ "requestNewPendingMultiAction", luaRequestNewPendingMultiAction },
		{ "requestEndAction", luaRequestEndAction },
		{ "requestForceEndMultiAction", luaRequestForceEndMultiAction },
		{ "requestCancelAction", luaRequestCancelAction },
		{ "clearActionHistoric", luaClearActionHistoric },
		{ "requestUploadCurrentScenario", luaRequestUploadCurrentScenario },
		{ "requestCreateScenario", luaRequestCreateScenario },
		{ "getScenarioObj", luaGetScenarioObj },
		{ "requestSetScenarioVersions", luaRequestSetScenarioVersions },
		{ "getInstanceFromId", luaGetInstanceFromId },
		{ "genInstanceName", luaGenInstanceName },
		{ "getNamespace", luaGetNamespace },
		{ "setStartingActIndex", luaSetStartingActIndex },
		{ "getSheetIdName", luaGetSheetIdName },
		{ "print", luaPrint },
		{ "getEmoteBehaviorFromEmoteId", luaGetEmoteBehaviorFromEmoteId },
		{ "snapPosToGround", luaSnapPosToGround },
		{ nullptr, nullptr }
	};
	lua_newtable(state);
	for (const luaL_Reg *reg = s_R2Functions; reg->name; ++reg)
	{
		std::string name = std::string("r2.") + reg->name;
		pushTracked(state, name.c_str(), reg->func);
		lua_setfield(state, -2, reg->name);
	}
	lua_newtable(state);
	lua_setfield(state, -2, "Config");
	lua_setglobal(state, "r2");

	// globals
	static const luaL_Reg s_GlobalFunctions[] = {
		{ "messageBox", luaMessageBox },
		{ "displaySystemInfo", luaDisplaySystemInfo },
		{ "dumpCallStack", luaDumpCallStack },
		{ "getCharacterSheetSkel", luaGetCharacterSheetSkel },
		{ "getSheetId", luaGetSheetId },
		{ "getR2PlayerRace", luaGetR2PlayerRace },
		{ "isR2PlayerMale", luaIsR2PlayerMale },
		{ "fileLookup", luaFileLookup },
		{ "getDefine", luaGetDefine },
		{ "getClientCfgVar", luaGetClientCfgVar },
		{ "getCompleteIslands", luaGetCompleteIslands },
		{ "CRGBA", luaColor },
		{ nullptr, nullptr }
	};
	for (const luaL_Reg *reg = s_GlobalFunctions; reg->name; ++reg)
	{
		pushTracked(state, reg->name, reg->func);
		lua_setglobal(state, reg->name);
	}
	lua_pushcfunction(state, luaDebugInfo);
	lua_setglobal(state, "debugInfo");
	lua_pushcfunction(state, luaDebugInfo);
	lua_setglobal(state, "rawDebugInfo");
	lua_pushcfunction(state, luaRecordSwallowedError);
	lua_setglobal(state, "r2HeadlessRecordError");

	lua_newtable(state);
	pushTracked(state, "i18n.get", luaI18nGet);
	lua_setfield(state, -2, "get");
	pushTracked(state, "i18n.hasTranslation", luaI18nHasTranslation);
	lua_setfield(state, -2, "hasTranslation");
	lua_setglobal(state, "i18n");

	lua_newtable(state);
	lua_pushcfunction(state, luaTimeMs);
	lua_setfield(state, -2, "getLocalTime");
	lua_pushcfunction(state, luaTimeMs);
	lua_setfield(state, -2, "getPreciseLocalTime");
	lua_setglobal(state, "nltime");

	// the client configuration, as the scripts read it
	lua_newtable(state);
	setProbe(state, "config.");
	lua_setglobal(state, "config");

	// stubs
	for (size_t i = 0; i < sizeof(s_Stubs) / sizeof(s_Stubs[0]); ++i)
	{
		const char *name = s_Stubs[i].Name;
		lua_pushstring(state, name);
		lua_pushinteger(state, s_Stubs[i].Result);
		lua_pushcclosure(state, luaStub, 2);
		if (strncmp(name, "r2.", 3) == 0)
		{
			lua_getglobal(state, "r2");
			lua_insert(state, -2);
			lua_setfield(state, -2, name + 3);
			lua_pop(state, 1);
		}
		else
		{
			lua_setglobal(state, name);
		}
	}

	// probes for what is read and not there
	lua_pushglobaltable(state);
	setProbe(state, "");
	lua_pop(state, 1);
	lua_getglobal(state, "r2");
	setProbe(state, "r2.");
	lua_pop(state, 1);
}

// ***************************************************************************
bool CHeadlessHost::init(const std::string &r2Dir, const std::string &driverFile, std::string &error)
{
	m_R2Dir = r2Dir;
	m_Factory = new CObjectFactory(m_Eid);
	m_Scenario = new CScenario(nullptr);
	m_ServerScenario = new CScenario(nullptr);

	// The scripts walk tables with string keys, and the order of such a walk follows the hash seed of the
	// state. A fixed seed gives the same run-time tree on every run; Lua 5.5 takes it here, an earlier
	// version draws its own.
#if LUA_VERSION_NUM >= 505
	m_Lua = lua_newstate(luaAllocate, nullptr, 0);
#else
	m_Lua = luaL_newstate();
#endif
	if (!m_Lua)
	{
		error = "lua_newstate failed";
		return false;
	}
	lua_atpanic(m_Lua, luaPanic);
	openLibraries();

	// math.pow, which 5.5 dropped and the client keeps for its scripts
	if (!runChunk("math.pow = math.pow or function(x, y) return x ^ y end\n", "=compat", 0, error))
		return false;

	// The host's own script. It runs first so that it can keep the debug functions its error handler
	// needs, which the editor's scripts do not get.
	{
		std::string driver;
		CIFile file;
		if (!file.open(driverFile))
		{
			error = "can't read the driver script " + driverFile;
			return false;
		}
		driver.resize(file.getFileSize());
		if (!driver.empty())
			file.serialBuffer((uint8 *)&driver[0], (uint)driver.size());
		if (!runChunk(driver, "@" + CFile::getFilename(driverFile), 0, error))
			return false;
	}
	reduceLibraries();

	registerFunctions();

	// pcall keeps its results but records the traceback of the errors it catches: the scripts swallow
	// errors in several places and print the message at most
	static const char *const s_PcallWrap = "local rawXpcall, traceback, record = xpcall, debug.traceback, r2HeadlessRecordError\n"
	                                       "r2HeadlessRecordError = nil\n"
	                                       "function pcall(f, ...)\n"
	                                       "	return rawXpcall(f, function(msg)\n"
	                                       "		record(traceback(tostring(msg), 2))\n"
	                                       "		return msg\n"
	                                       "	end, ...)\n"
	                                       "end\n";
	if (!runChunk(s_PcallWrap, "=pcallWrap", 0, error))
		return false;

	m_Phase = "load";
	std::vector<std::string> none;
	if (!callDriver("beforeCore", none, 0, error))
	{
		error = "r2Headless.beforeCore: " + error;
		return false;
	}
	if (!doFile("r2_core.lua", error))
		return false;
	for (size_t i = 0; i < sizeof(s_InterfaceScripts) / sizeof(s_InterfaceScripts[0]); ++i)
	{
		if (!doFile(s_InterfaceScripts[i], error))
		{
			error = std::string(s_InterfaceScripts[i]) + ": " + error;
			return false;
		}
	}
	// What the client does for these scripts when it sets up its interface, and the path needs:
	// r2.logicComponents:initLogicEditors calls r2.events:initEditor, which fills r2.classLogicAttributes
	// (the event, action and condition names of each class, read by the getName functions);
	// r2:buildPaletteUI fills r2.PaletteIdToTranslation (the name the palette gives each entry, which the
	// translator compares the name of a creature with).
	if (!runChunk("r2.events:mergeLogicTranslations()", "=mergeLogicTranslations", 0, error))
	{
		error = "r2.events:mergeLogicTranslations: " + error;
		return false;
	}
	if (!callDriver("fillPaletteTranslations", none, 0, error))
	{
		error = "r2Headless.fillPaletteTranslations: " + error;
		return false;
	}
	if (!callDriver("afterInterfaceScripts", none, 0, error))
	{
		error = "r2Headless.afterInterfaceScripts: " + error;
		return false;
	}
	// a script that failed and whose caller went on (r2.doFileProtected, r2.init) fails the load as well
	for (size_t i = 0; i < m_Log.size(); ++i)
	{
		if (m_Log[i].Kind == "luaError" || m_Log[i].Kind == "loadError")
		{
			error = "a script of the editor failed while loading (see the errors above)";
			return false;
		}
	}
	return true;
}

// ***************************************************************************
void CHeadlessHost::beginScenarioUpdated(CObject *highLevel, bool fromServer)
{
	// CClientEditionModule::onScenarioUploaded and updateScenario
	m_Factory->clear();
	if (fromServer)
	{
		CObject::TSmartPtr serverCopy = highLevel->clone();
		m_ServerScenario->setHighLevel(serverCopy);
	}
	m_Scenario->setHighLevel(highLevel);
	m_Factory->setMaxId(m_Eid, m_Scenario->getMaxId(m_Eid));
}

// ***************************************************************************
void CHeadlessHost::endScenarioUpdated()
{
	// CEditor::scenarioUpdated ends with CActionHistoric::clear(highLevel)
	dropOpenAction("dropped at the end of the scenario update", false);
	m_HistoricHasScenario = m_Scenario->getHighLevel() != nullptr;
}

// ***************************************************************************
bool CHeadlessHost::takeUploadedScenario(CObject::TSmartPtr &scenario)
{
	if (!m_UploadedScenario) return false;
	scenario = m_UploadedScenario;
	m_UploadedScenario = nullptr;
	return true;
}

// ***************************************************************************
void CHeadlessHost::endStepWithLateRequests()
{
	dropOpenAction("applied a frame later", true);
	m_FrameRequests.clear();
}

// ***************************************************************************
void CHeadlessHost::resetRtIds()
{
	// as CClientEditionModule::startingScenario
	static const char *const s_RtClasses[] = { "RtAct", "RtAiState", "RtNpcGrp", "RtNpcEventHandlerAction", "RtNpcEventHandler",
		"RtLocation", "RtTextManager", "RtScenario", "RtUserTrigger", "RtEntryText", "RtPlotItem" };
	for (size_t i = 0; i < sizeof(s_RtClasses) / sizeof(s_RtClasses[0]); ++i)
		m_Factory->setMaxId(s_RtClasses[i], 0);
}

// ***************************************************************************
CObject *CHeadlessHost::getHighLevel() const
{
	return m_Scenario->getHighLevel();
}

// ***************************************************************************
CObject *CHeadlessHost::getServerHighLevel() const
{
	return m_ServerScenario ? m_ServerScenario->getHighLevel() : nullptr;
}

// ***************************************************************************
std::string CHeadlessHost::getScenarioInstanceId() const
{
	CObject *highLevel = m_Scenario->getHighLevel();
	if (!highLevel || !highLevel->isString("InstanceId")) return std::string();
	return highLevel->toString("InstanceId");
}

} // namespace R2
