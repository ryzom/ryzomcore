// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
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

// The Lua and action handler methods of the control endpoint: lua.run, ah.run, ah.list. A chunk runs on the main
// thread with a bound on its time: a count hook raises an error once its timeout_ms has passed, so a loop that never
// ends stops the chunk, not the client.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/gui/action_handler.h"
#include "nel/gui/ctrl_base.h"
#include "nel/gui/lua_helper.h"
#include "nel/gui/lua_manager.h"
#include "nel/gui/widget_manager.h"

#include <set>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

namespace ClientCtrl {

namespace {

// the bound of a run: 10 s by default, 10 minutes at most
const sint64 s_DefaultTimeoutMs = 10000;
const sint64 s_MaxTimeoutMs = 600000;
// the instructions between two looks at the clock
const int s_HookCount = 10000;
// the nesting of the tables made of args
const uint s_MaxArgsDepth = 32;

// the run the count hook bounds
bool s_LuaBounded = false;
TTime s_LuaDeadline = 0;
uint s_LuaTimeoutMs = 0;
bool s_LuaTimedOut = false;

// *** Lua values as JSON ***

struct CLuaToJson
{
	lua_State *L;
	uint MaxDepth;
	uint Budget;
	bool Truncated;
	std::set<const void *> Path;

	CJsonValue convert(int index, uint depth)
	{
		if (index < 0)
			index = lua_gettop(L) + index + 1;
		int type = lua_type(L, index);
		switch (type)
		{
		case LUA_TNIL:
			return CJsonValue();
		case LUA_TBOOLEAN:
			return CJsonValue(lua_toboolean(L, index) != 0);
		case LUA_TNUMBER:
#if LUA_VERSION_NUM >= 503
			if (lua_isinteger(L, index))
				return CJsonValue((sint64)lua_tointeger(L, index));
#endif
			return CJsonValue((double)lua_tonumber(L, index));
		case LUA_TSTRING: {
			size_t len = 0;
			const char *s = lua_tolstring(L, index, &len);
			return CJsonValue(std::string(s, len));
		}
		case LUA_TTABLE:
			return convertTable(index, depth);
		default:
			return CJsonValue(toString("<%s: %p>", lua_typename(L, type), lua_topointer(L, index)));
		}
	}

	CJsonValue convertTable(int index, uint depth)
	{
		const void *p = lua_topointer(L, index);
		if (Path.find(p) != Path.end())
		{
			Truncated = true;
			return CJsonValue(toString("<cycle: table %p>", p));
		}
		if (depth >= MaxDepth)
		{
			Truncated = true;
			return CJsonValue(toString("<table %p: below max_depth>", p));
		}
		Path.insert(p);

		// a sequence 1..n is an array, anything else an object
#if LUA_VERSION_NUM >= 502
		size_t length = lua_rawlen(L, index);
#else
		size_t length = lua_objlen(L, index);
#endif
		size_t count = 0;
		bool sequence = true;
		lua_pushnil(L);
		while (lua_next(L, index) != 0)
		{
			++count;
			if (lua_type(L, -2) != LUA_TNUMBER)
				sequence = false;
			lua_pop(L, 1);
		}
		sequence = sequence && count == length;

		CJsonValue result = sequence ? CJsonValue::array() : CJsonValue::object();
		if (sequence)
		{
			for (size_t i = 1; i <= length; ++i)
			{
				if (Budget == 0)
				{
					Truncated = true;
					break;
				}
				--Budget;
				lua_rawgeti(L, index, (int)i);
				result.push(convert(-1, depth + 1));
				lua_pop(L, 1);
			}
		}
		else
		{
			lua_pushnil(L);
			while (lua_next(L, index) != 0)
			{
				if (Budget == 0)
				{
					Truncated = true;
					lua_pop(L, 2);
					break;
				}
				--Budget;
				// a copy of the key: lua_tostring on the key itself would break lua_next
				lua_pushvalue(L, -2);
				std::string key;
				int keyType = lua_type(L, -1);
				if (keyType == LUA_TSTRING)
				{
					size_t len = 0;
					const char *s = lua_tolstring(L, -1, &len);
					key.assign(s, len);
				}
				else if (keyType == LUA_TNUMBER)
				{
					// as Lua writes a key that is not a string, apart from the string "1"
					size_t len = 0;
					const char *s = lua_tolstring(L, -1, &len);
					key = "[" + std::string(s, len) + "]";
				}
				else if (keyType == LUA_TBOOLEAN)
				{
					key = lua_toboolean(L, -1) != 0 ? "[true]" : "[false]";
				}
				else
				{
					key = toString("<%s: %p>", lua_typename(L, keyType), lua_topointer(L, -1));
				}
				lua_pop(L, 1);
				result.set(key, convert(-1, depth + 1));
				lua_pop(L, 1);
			}
		}
		Path.erase(p);
		return result;
	}
};

/// The deepest nesting of a JSON value that starts at depth
uint jsonDepth(const CJsonValue &value, uint depth)
{
	uint deepest = depth;
	if (value.isArray())
	{
		for (uint i = 0; i < value.size(); ++i)
			deepest = std::max(deepest, jsonDepth(value.at(i), depth + 1));
	}
	else if (value.isObject())
	{
		const CJsonValue::TMembers &members = value.members();
		for (uint i = 0; i < members.size(); ++i)
			deepest = std::max(deepest, jsonDepth(members[i].second, depth + 1));
	}
	return deepest;
}

/// A JSON value as a Lua value; runChunk checks the nesting first
void pushJson(lua_State *L, const CJsonValue &value)
{
	switch (value.getType())
	{
	case CJsonValue::Bool:
		lua_pushboolean(L, value.getBool() ? 1 : 0);
		break;
	case CJsonValue::Number:
#if LUA_VERSION_NUM >= 503
		if (value.getNumber() == (double)value.getInt() && value.getNumber() > -9e15 && value.getNumber() < 9e15)
		{
			lua_pushinteger(L, (lua_Integer)value.getInt());
			break;
		}
#endif
		lua_pushnumber(L, (lua_Number)value.getNumber());
		break;
	case CJsonValue::String:
		lua_pushlstring(L, value.getString().data(), value.getString().size());
		break;
	case CJsonValue::Array:
		lua_newtable(L);
		for (uint i = 0; i < value.size(); ++i)
		{
			pushJson(L, value.at(i));
			lua_rawseti(L, -2, (int)i + 1);
		}
		break;
	case CJsonValue::Object: {
		lua_newtable(L);
		const CJsonValue::TMembers &members = value.members();
		for (uint i = 0; i < members.size(); ++i)
		{
			lua_pushlstring(L, members[i].first.data(), members[i].first.size());
			pushJson(L, members[i].second);
			lua_rawset(L, -3);
		}
		break;
	}
	default:
		lua_pushnil(L);
		break;
	}
}

/// Loads a chunk of Lua text: a precompiled chunk can crash the virtual machine, so it is refused
int loadText(lua_State *L, const std::string &code)
{
#if LUA_VERSION_NUM >= 502
	return luaL_loadbufferx(L, code.data(), code.size(), "=ctrl", "t");
#else
	if (!code.empty() && code[0] == LUA_SIGNATURE[0])
	{
		lua_pushstring(L, "a precompiled chunk is not loaded, only Lua text");
		return LUA_ERRSYNTAX;
	}
	return luaL_loadbuffer(L, code.data(), code.size(), "=ctrl");
#endif
}

/// The error with where it happened, as the message handler of the call
int luaTraceback(lua_State *L)
{
	const char *message = lua_tostring(L, 1);
	std::string text = message != nullptr ? message : toString("(an error object of type %s)", luaL_typename(L, 1));
	text += "\nstack traceback:";
	lua_Debug ar;
	for (int level = 1; lua_getstack(L, level, &ar) != 0 && level < 32; ++level)
	{
		lua_getinfo(L, "Sln", &ar);
		text += toString("\n\t%s:%d: in %s", ar.short_src, ar.currentline, ar.name != nullptr ? ar.name : (*ar.what == 'm' ? "main chunk" : "?"));
	}
	lua_pushlstring(L, text.data(), text.size());
	return 1;
}

/// The count hook of a run: once the deadline has passed it raises an error, again at every count, so a chunk that
/// catches it stops at the next count outside its pcall. A coroutine that outlives its run keeps the hook (a thread
/// gets the hook of the one that made it) and drops it at its next count.
void luaDeadlineHook(lua_State *L, lua_Debug * /* ar */)
{
	if (!s_LuaBounded)
	{
		lua_sethook(L, nullptr, 0, 0);
		return;
	}
	if (CTime::getLocalTime() < s_LuaDeadline)
		return;
	s_LuaTimedOut = true;
	luaL_error(L, "the chunk ran longer than timeout_ms (%d ms) and was stopped", (int)s_LuaTimeoutMs);
}

/// What a run of a chunk gives
struct CLuaRun
{
	CJsonValue Results;
	std::string Error;
	bool Truncated;
	bool TimedOut;

	CLuaRun()
	    : Truncated(false)
	    , TimedOut(false)
	{
	}
};

/// Runs a chunk in the interface's Lua state, with the items of args as its ..., at most timeoutMs, and converts what it
/// returns: tables followed to maxDepth, maxItems values in all, the returns themselves counted
bool runChunk(const std::string &code, const CJsonValue &args, uint maxDepth, uint maxItems, uint timeoutMs, CLuaRun &run)
{
	run.Results = CJsonValue::array();
	CLuaState *state = CLuaManager::getInstance().getLuaState();
	if (state == nullptr)
	{
		run.Error = "there is no interface Lua state yet";
		return false;
	}
	if (jsonDepth(args, 0) > s_MaxArgsDepth + 1)
	{
		run.Error = toString("args nest deeper than %u levels", s_MaxArgsDepth);
		return false;
	}
	lua_State *L = state->getStatePointer();
	// room for the handler, the chunk, the arguments and their nested tables (two slots a level)
	if (!lua_checkstack(L, (int)args.size() + 2 * (int)s_MaxArgsDepth + 8))
	{
		run.Error = "the Lua stack has no room for the call";
		return false;
	}
	int top = lua_gettop(L);
	lua_pushcfunction(L, luaTraceback);
	int handler = top + 1;
	if (loadText(L, code) != 0)
	{
		run.Error = std::string("the chunk does not compile: ") + (lua_tostring(L, -1) != nullptr ? lua_tostring(L, -1) : "?");
		lua_settop(L, top);
		return false;
	}
	for (uint i = 0; i < args.size(); ++i)
		pushJson(L, args.at(i));

	// the count hook in the place of the state's own for the run, which gets it back after
	lua_Hook previousHook = lua_gethook(L);
	int previousMask = lua_gethookmask(L);
	int previousCount = lua_gethookcount(L);
	bool previousBounded = s_LuaBounded;
	TTime previousDeadline = s_LuaDeadline;
	uint previousTimeout = s_LuaTimeoutMs;
	s_LuaBounded = true;
	s_LuaDeadline = CTime::getLocalTime() + timeoutMs;
	s_LuaTimeoutMs = timeoutMs;
	s_LuaTimedOut = false;
	lua_sethook(L, luaDeadlineHook, LUA_MASKCOUNT, s_HookCount);
	int status = lua_pcall(L, (int)args.size(), LUA_MULTRET, handler);
	lua_sethook(L, previousHook, previousMask, previousCount);
	run.TimedOut = s_LuaTimedOut;
	s_LuaBounded = previousBounded;
	s_LuaDeadline = previousDeadline;
	s_LuaTimeoutMs = previousTimeout;
	if (status != 0)
	{
		run.Error = lua_tostring(L, -1) != nullptr ? lua_tostring(L, -1) : "?";
		lua_settop(L, top);
		return false;
	}

	CLuaToJson converter;
	converter.L = L;
	converter.MaxDepth = maxDepth;
	converter.Budget = maxItems;
	converter.Truncated = false;
	int resultTop = lua_gettop(L);
	// the conversion walks the tables with a few slots a level
	if (!lua_checkstack(L, (int)maxDepth * 4 + 8))
	{
		lua_settop(L, top);
		run.Error = "the Lua stack has no room to read the results";
		return false;
	}
	for (int i = handler + 1; i <= resultTop; ++i)
	{
		if (converter.Budget == 0)
		{
			converter.Truncated = true;
			break;
		}
		--converter.Budget;
		run.Results.push(converter.convert(i, 0));
	}
	lua_settop(L, top);
	run.Truncated = converter.Truncated;
	return true;
}

CJsonValue shownToJson(const std::vector<std::string> &shown)
{
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < shown.size(); ++i)
		list.push(CJsonValue(shown[i]));
	return list;
}

void methodLuaRun(CCtrlCall &call)
{
	const CJsonValue &a = call.getArgs();
	const std::string &code = a.get("code").getString();
	if (code.empty())
	{
		call.fail("code is empty");
		return;
	}
	const CJsonValue &args = a.get("args");
	if (!args.isNull() && !args.isArray())
	{
		call.fail("args is a JSON array: its items are the chunk's ...");
		return;
	}
	sint64 maxDepth = a.get("max_depth").getInt(6);
	sint64 maxItems = a.get("max_items").getInt(2000);
	sint64 timeoutMs = a.get("timeout_ms").getInt(s_DefaultTimeoutMs);
	if (maxDepth < 1 || maxDepth > 32)
	{
		call.fail("max_depth is 1 to 32");
		return;
	}
	if (maxItems < 1 || maxItems > 100000)
	{
		call.fail("max_items is 1 to 100000");
		return;
	}
	if (timeoutMs < 1 || timeoutMs > s_MaxTimeoutMs)
	{
		call.fail(toString("timeout_ms is 1 to %u", (uint)s_MaxTimeoutMs));
		return;
	}
	std::vector<std::string> shown;
	CLuaRun run;
	bool ok;
	{
		CShownScope scope(shown);
		ok = runChunk(code, args, (uint)maxDepth, (uint)maxItems, (uint)timeoutMs, run);
	}
	if (!ok)
	{
		CJsonValue details = CJsonValue::object();
		details.set("shown", shownToJson(shown));
		if (run.TimedOut)
			details.set("timed_out", true);
		call.fail(run.Error, details);
		return;
	}
	CJsonValue r = CJsonValue::object();
	r.set("results", run.Results);
	r.set("truncated", run.Truncated);
	if (run.Truncated)
		r.set("note", "tables deeper than max_depth, cycles, or more values than max_items are left out");
	r.set("shown", shownToJson(shown));
	call.reply(r);
}

void methodAhRun(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	const std::string &name = call.getArgs().get("name").getString();
	const std::string &params = call.getArgs().get("params").getString();
	CAHManager *manager = CAHManager::getInstance();
	if (manager->getActionHandler(name) == nullptr)
	{
		call.fail(toString("no action handler '%s'; ah.list lists them", name.c_str()));
		return;
	}
	// some handlers act on the control that ran them
	CCtrlBase *caller = nullptr;
	const std::string &callerId = call.getArgs().get("caller").getString();
	if (!callerId.empty())
	{
		CInterfaceElement *element = findElement(callerId);
		caller = dynamic_cast<CCtrlBase *>(element);
		if (element == nullptr)
		{
			call.fail(toString("no interface element '%s'", callerId.c_str()));
			return;
		}
		if (caller == nullptr)
		{
			call.fail(toString("'%s' is not a control", callerId.c_str()));
			return;
		}
	}
	std::vector<std::string> shown;
	{
		CShownScope scope(shown);
		manager->runActionHandler(name, caller, params);
	}
	CJsonValue r = CJsonValue::object();
	r.set("handler", name);
	r.set("params", params);
	if (caller != nullptr)
		r.set("caller", callerId);
	r.set("shown", shownToJson(shown));
	call.reply(r);
}

void methodAhList(CCtrlCall &call)
{
	std::vector<std::string> handlers;
	CAHManager::getInstance()->getActionHandlers(handlers);
	std::string prefix = call.getArgs().get("prefix").getString();
	std::string contains = toLowerAscii(call.getArgs().get("contains").getString());
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < handlers.size(); ++i)
	{
		if (!prefix.empty() && handlers[i].compare(0, prefix.size(), prefix) != 0)
			continue;
		if (!contains.empty() && toLowerAscii(handlers[i]).find(contains) == std::string::npos)
			continue;
		list.push(CJsonValue(handlers[i]));
	}
	call.reply(CJsonValue::object().set("handlers", list).set("count", list.size()));
}

} // anonymous namespace

bool runLua(const std::string &code, const CJsonValue &args, uint maxDepth, uint maxItems, CJsonValue &results, std::string &error, bool *truncated)
{
	CLuaRun run;
	if (!runChunk(code, args, maxDepth, maxItems, (uint)s_DefaultTimeoutMs, run))
	{
		error = run.Error;
		return false;
	}
	results = run.Results;
	if (truncated != nullptr)
		*truncated = run.Truncated;
	return true;
}

void registerScriptMethods(CCtrlEndpoint &endpoint)
{
	// any script and any action handler: privileged, so they run only in a client whose content is trusted
	ICtrlMethod *luaRun = newMethod("lua.run",
	    "Runs a Lua chunk of any length in the interface's Lua state, with args as its ..., and returns what it returns (tables as JSON, max_depth and max_items bound them, "
	    "the returns themselves counted; a key that is not a string is written as Lua writes it, [1]), or the error with a traceback; also the system messages it showed. "
	    "It has what the interface's Lua has: getUI, getDbProp, runAH, r2, and with AllowDebugLua the host libraries. It stops with an error once timeout_ms has passed "
	    "(10 s by default); a call into C++ that does not return is not stopped.",
	    CCtrlSchema()
	        .str("code", "The Lua chunk; 'return x' gives x back.", true)
	        .any("args", "A JSON array; its items are the chunk's ... (objects become tables), nested 32 levels at most.")
	        .integer("max_depth", "How deep tables are followed, 1 to 32 (6 by default).")
	        .integer("max_items", "How many values at most in all, 1 to 100000 (2000 by default).")
	        .integer("timeout_ms", "The longest the chunk may run, 1 to 600000 ms (10000 by default)."),
	    false, methodLuaRun, nullptr, nullptr);
	luaRun->setPrivileged(true);
	endpoint.addMethod(luaRun);
	ICtrlMethod *ahRun = newMethod("ah.run",
	    "Runs an action handler by name with its parameters, as the interface does when a control fires it; caller names the control it acts on when it needs one. Returns the system messages it showed. "
	    "This runs the handler directly, in whatever screen the client shows: it passes over whatever covers the control, unlike a click, and a handler of another screen acts on "
	    "elements that are not shown.",
	    CCtrlSchema()
	        .str("name", "The action handler, as ah.list names it.", true)
	        .str("params", "Its parameters, in the interface's syntax (name=value|name=value).")
	        .str("caller", "The id of the control that fires it (ui:interface:...)."),
	    false, methodAhRun, nullptr, nullptr);
	ahRun->setPrivileged(true);
	endpoint.addMethod(ahRun);
	endpoint.addMethod(newMethod("ah.list", "The action handlers of the interface, filtered by prefix or a word of the name.",
	    CCtrlSchema().str("prefix", "Only names starting with this.").str("contains", "Only names containing this (case-insensitive)."), true, methodAhList, nullptr, nullptr));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_script.cpp */
