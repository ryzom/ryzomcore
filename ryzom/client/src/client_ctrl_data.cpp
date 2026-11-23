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

// The data methods of the control endpoint: db.get, db.list, db.set, db.watch, strings.get,
// strings.request, strings.ui, client.window, client.desktop, client.perf. The database is the
// client's (SERVER: what the server sends, UI: and LOCAL: the interface's own); db.set writes what
// Lua may write: not SERVER:, and of LOCAL: only LOCAL:R2.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/common.h"
#include "nel/misc/i18n.h"
#include "nel/misc/system_utils.h"

#include "client_cfg.h"
#include "global.h"
#include "interface_v3/input_handler_manager.h"
#include "net_manager.h"
#include "string_manager_client.h"
#include "window_desktop.h"

#include <set>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

// the integers a double holds exactly: the 64-bit values beyond them (sheet ids, entity ids) travel as text
const sint64 s_MaxExactInteger = SINT64_CONSTANT(9007199254740992);

/// Reads a number of the arguments into value: absent or null leaves value as it is; false with the call failed when
/// it is not a number, not finite, or beyond limit either way
bool readNumber(CCtrlCall &call, const char *name, double limit, double &value)
{
	const CJsonValue &arg = call.getArgs().get(name);
	if (arg.isNull())
		return true;
	if (!arg.isNumber() || !isValidDouble(arg.getNumber()) || fabs(arg.getNumber()) > limit)
	{
		call.fail(toString("%s is a number from %g to %g", name, -limit, limit));
		return false;
	}
	value = arg.getNumber();
	return true;
}

/// An integer: a whole number a double holds exactly, or the decimal text of any 64-bit value. The text must read
/// back as it was written, so that trailing characters, fractions and overflows do not pass.
bool readInteger(const CJsonValue &value, sint64 &result)
{
	if (value.isNumber())
	{
		double v = value.getNumber();
		if (!isValidDouble(v) || floor(v) != v || fabs(v) > (double)s_MaxExactInteger)
			return false;
		result = (sint64)v;
		return true;
	}
	if (value.isString())
		return fromString(value.getString(), result) && toString("%" NL_I64 "d", result) == value.getString();
	return false;
}

ICDBNode *findNode(const std::string &path)
{
	CCDBNodeBranch *root = NLGUI::CDBManager::getInstance()->getDB();
	if (root == nullptr)
		return nullptr;
	if (path.empty())
		return root;
	ICDBNode::CTextId id(path);
	return root->getNode(id, false);
}

/// The value of a leaf into o, and as text when a double would not hold it exactly
void setLeafValue(CJsonValue &o, const CCDBNodeLeaf *leaf)
{
	sint64 v = leaf->getValue64();
	o.set("value", v);
	if (v > s_MaxExactInteger || v < -s_MaxExactInteger)
		o.set("value_text", toString("%" NL_I64 "d", v));
}

CJsonValue leafJson(const CCDBNodeLeaf *leaf)
{
	CJsonValue o = CJsonValue::object();
	setLeafValue(o, leaf);
	return o;
}

void listBranch(CCDBNodeBranch *branch, const std::string &path, uint depth, uint maxDepth, uint &budget, bool &truncated, CJsonValue &out)
{
	uint count = branch->getNbNodes();
	for (uint i = 0; i < count; ++i)
	{
		if (budget == 0)
		{
			truncated = true;
			return;
		}
		ICDBNode *node = branch->getNode((uint16)i);
		if (node == nullptr)
			continue;
		std::string name = *node->getName();
		std::string child = path.empty() ? name : path + ":" + name;
		--budget;
		if (node->isLeaf())
		{
			CJsonValue o = leafJson(static_cast<CCDBNodeLeaf *>(node));
			o.set("path", child);
			out.push(o);
		}
		else
		{
			CCDBNodeBranch *sub = static_cast<CCDBNodeBranch *>(node);
			out.push(CJsonValue::object().set("path", child).set("branch", (uint32)sub->getNbNodes()));
			if (depth + 1 < maxDepth)
				listBranch(sub, child, depth + 1, maxDepth, budget, truncated, out);
		}
	}
}

void methodGet(CCtrlCall &call)
{
	const std::string &path = call.getArgs().get("path").getString();
	ICDBNode *node = findNode(path);
	if (node == nullptr)
	{
		call.fail(toString("no database node '%s'", path.c_str()));
		return;
	}
	if (!node->isLeaf())
	{
		call.fail(toString("'%s' is a branch of %u nodes: db.list lists it", path.c_str(), (uint) static_cast<CCDBNodeBranch *>(node)->getNbNodes()));
		return;
	}
	CJsonValue r = leafJson(static_cast<CCDBNodeLeaf *>(node));
	r.set("path", path);
	call.reply(r);
}

void methodList(CCtrlCall &call)
{
	const std::string &path = call.getArgs().get("path").getString();
	ICDBNode *node = findNode(path);
	if (node == nullptr || node->isLeaf())
	{
		call.fail(toString(node == nullptr ? "no database node '%s'" : "'%s' is a leaf: db.get reads it", path.c_str()));
		return;
	}
	uint maxDepth = (uint)std::max((sint64)1, std::min((sint64)16, call.getArgs().get("depth").getInt(1)));
	uint budget = (uint)std::max((sint64)1, std::min((sint64)20000, call.getArgs().get("max").getInt(500)));
	bool truncated = false;
	CJsonValue nodes = CJsonValue::array();
	listBranch(static_cast<CCDBNodeBranch *>(node), path, 0, maxDepth, budget, truncated, nodes);
	CJsonValue r = CJsonValue::object();
	r.set("path", path);
	r.set("nodes", nodes);
	r.set("truncated", truncated);
	call.reply(r);
}

void methodSet(CCtrlCall &call)
{
	const std::string &path = call.getArgs().get("path").getString();
	// Lua's rule (CLuaIHMRyzom::setDbProp): the server's branches are the server's, and LOCAL: the client's own code's,
	// but for LOCAL:R2, the scenario editor's
	if (startsWithCaseInsensitiveAscii(path, "SERVER:") || (startsWithCaseInsensitiveAscii(path, "LOCAL:") && !startsWithCaseInsensitiveAscii(path, "LOCAL:R2")))
	{
		call.fail("SERVER: and LOCAL: are not written, as for Lua: the server and the client's own code own them (LOCAL:R2 is written)");
		return;
	}
	ICDBNode *node = findNode(path);
	if (node == nullptr || !node->isLeaf())
	{
		call.fail(toString(node == nullptr ? "no database leaf '%s'" : "'%s' is a branch", path.c_str()));
		return;
	}
	sint64 v;
	if (!readInteger(call.getArgs().get("value"), v))
	{
		call.fail("value is an integer: a whole number, or its decimal text (for a value beyond 2^53)");
		return;
	}
	static_cast<CCDBNodeLeaf *>(node)->setValue64(v);
	CJsonValue r = leafJson(static_cast<CCDBNodeLeaf *>(node));
	r.set("path", path);
	call.reply(r);
}

// the watches there can be at once, the leaves one can cover, and the db events a second: the events go to the
// ring that all events share, and a busy branch must not push the chat and the state out of it
const uint s_MaxWatches = 64;
const uint s_MaxWatchedLeaves = 4096;
const uint s_MaxDbEventsPerSecond = 100;

TTime s_DbEventSecond = 0;
uint s_DbEventCount = 0;
uint s_DbEventsDropped = 0;

/// A watched node: the changes of its leaves are events, each with the leaf that changed, while a connection watches it
class CWatch : public ICDBNode::IPropertyObserver
{
public:
	std::string Path;
	ICDBNode *Node;
	// the connections that watch it; with none, the observer is off its node, or on a node that no path reaches
	std::set<uint32> Owners;

	CWatch()
	    : Node(nullptr)
	{
	}

	virtual void update(ICDBNode *node) NL_OVERRIDE
	{
		if (Owners.empty())
			return;
		TTime now = CTime::getLocalTime();
		if (now - s_DbEventSecond >= 1000)
		{
			s_DbEventSecond = now;
			s_DbEventCount = 0;
		}
		if (s_DbEventCount >= s_MaxDbEventsPerSecond)
		{
			++s_DbEventsDropped;
			return;
		}
		++s_DbEventCount;
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf *>(node);
		CJsonValue e = CJsonValue::object();
		e.set("path", node != nullptr ? node->getFullName() : Path);
		if (node != nullptr && node != Node)
			e.set("watch", Path);
		if (leaf != nullptr)
			setLeafValue(e, leaf);
		if (s_DbEventsDropped > 0)
		{
			e.set("dropped_before", s_DbEventsDropped);
			s_DbEventsDropped = 0;
		}
		addEvent("db", e);
	}
};

typedef NLMISC::CSmartPtr<CWatch> TWatchPtr;
// the observers by path, kept as long as the client runs: the leaves hold plain pointers to their observers, and the
// leaves below a watched branch are not followed one by one, so none is left pointing at a deleted observer. A path
// watched again takes its observer back.
std::map<std::string, TWatchPtr> s_Watches;

/// Takes an observer off a node and everything below it: a branch's own removal stops at the first child without it
void removeWatch(ICDBNode *node, CWatch *watch)
{
	ICDBNode::CTextId id;
	if (node->isLeaf())
	{
		node->removeObserver(watch, id);
		return;
	}
	CCDBNodeBranch *branch = static_cast<CCDBNodeBranch *>(node);
	for (uint i = 0; i < branch->getNbNodes(); ++i)
		if (branch->getNode((uint16)i) != nullptr)
			removeWatch(branch->getNode((uint16)i), watch);
}

/// The leaves below a node, counted up to limit + 1
uint countLeaves(ICDBNode *node, uint limit)
{
	if (node->isLeaf())
		return 1;
	CCDBNodeBranch *branch = static_cast<CCDBNodeBranch *>(node);
	uint count = 0;
	for (uint i = 0; i < branch->getNbNodes() && count <= limit; ++i)
		if (branch->getNode((uint16)i) != nullptr)
			count += countLeaves(branch->getNode((uint16)i), limit - count);
	return count;
}

/// The last connection of a watch is gone: the observer comes off the node of its path, if there is one (a node
/// taken down took its observers with it)
void unwatch(CWatch *watch)
{
	ICDBNode *node = findNode(watch->Path);
	if (node != nullptr)
		removeWatch(node, watch);
	watch->Node = nullptr;
}

void methodWatch(CCtrlCall &call)
{
	const std::string &path = call.getArgs().get("path").getString();
	uint32 connection = call.getConnection();
	std::map<std::string, TWatchPtr>::iterator it = s_Watches.find(path);
	if (call.getArgs().get("stop").getBool(false))
	{
		if (it == s_Watches.end() || it->second->Owners.erase(connection) == 0)
		{
			call.fail(toString("this connection does not watch '%s'", path.c_str()));
			return;
		}
		if (it->second->Owners.empty())
			unwatch(it->second);
		call.reply(CJsonValue::object().set("path", path).set("watched", false));
		return;
	}
	ICDBNode *node = findNode(path);
	if (node == nullptr)
	{
		call.fail(toString("no database node '%s'", path.c_str()));
		return;
	}
	if (path.empty())
	{
		call.fail("the root: every change of the database would be an event; watch a branch or a leaf");
		return;
	}
	if (countLeaves(node, s_MaxWatchedLeaves) > s_MaxWatchedLeaves)
	{
		call.fail(toString("'%s' has more than %u leaves, the most a watch covers: watch a branch below it", path.c_str(), s_MaxWatchedLeaves));
		return;
	}
	bool active = it != s_Watches.end() && !it->second->Owners.empty();
	if (!active)
	{
		uint watched = 0;
		for (std::map<std::string, TWatchPtr>::const_iterator w = s_Watches.begin(); w != s_Watches.end(); ++w)
			if (!w->second->Owners.empty())
				++watched;
		if (watched >= s_MaxWatches)
		{
			call.fail(toString("%u nodes are watched, the most there can be: stop a watch first", watched));
			return;
		}
	}
	CWatch *watch;
	if (it != s_Watches.end())
	{
		watch = it->second;
	}
	else
	{
		watch = new CWatch();
		watch->Path = path;
		s_Watches[path] = watch;
	}
	// a node of that path made again since (the UI branch at each login), maybe at the same address, needs the
	// observer again: it is taken off first, so a node that still has it does not get it twice
	removeWatch(node, watch);
	ICDBNode::CTextId id;
	node->addObserver(watch, id);
	watch->Node = node;
	watch->Owners.insert(connection);
	call.reply(CJsonValue::object()
	        .set("path", path)
	        .set("watched", true)
	        .set("events", "db events with the path and value of the leaf that changed, through ctrl.events (kinds db), until this connection stops the watch or closes"));
}

/// The id of the string methods: false with the call failed unless it is an integer that a string id can be
bool readStringId(CCtrlCall &call, uint32 &id)
{
	sint64 v;
	if (!readInteger(call.getArgs().get("id"), v) || v < 0 || v > 0xFFFFFFFF)
	{
		call.fail("id is a string id: an integer from 0 to 4294967295");
		return false;
	}
	id = (uint32)v;
	return true;
}

/// The strings come from the shard: in local mode none does, and the string manager makes up placeholders
bool requireShardStrings(CCtrlCall &call)
{
	if (!ClientCfg.Local)
		return true;
	call.fail("the client runs in local mode, where no shard sends strings: no string id has a text");
	return false;
}

void methodString(CCtrlCall &call)
{
	uint32 id;
	if (!readStringId(call, id) || !requireShardStrings(call))
		return;
	bool dynamic = call.getArgs().get("dynamic").getBool(false);
	std::string text;
	// what the client has, and nothing asked of the shard: a static string the client never asked for stays unknown
	// (strings.request asks). A dynamic string comes from the shard with what uses it, and the client asks for its
	// parts itself as it arrives, so getDynString asks for nothing the client has not asked for already.
	bool arrived = dynamic ? STRING_MANAGER::CStringManagerClient::instance()->getDynString(id, text) : STRING_MANAGER::CStringManagerClient::instance()->getReceivedString(id, text);
	CJsonValue r = CJsonValue::object();
	r.set("id", id);
	r.set("dynamic", dynamic);
	r.set("arrived", arrived);
	r.set("text", text);
	if (!arrived)
		r.set("note", dynamic ? "not complete yet, or no dynamic string the shard sent: the shard sends one with what uses it" : "not received: the client has not asked for it, or the answer is on its way; strings.request asks the shard");
	call.reply(r);
}

void methodStringRequest(CCtrlCall &call)
{
	uint32 id;
	if (!readStringId(call, id) || !requireShardStrings(call))
		return;
	// the request goes through the connection to the shard, there from the character selection on
	if (NetMngr.getConnectionState() != CNetworkConnection::Connected)
	{
		call.fail(toString("the client is not connected to a shard (the connection is %s): strings can be asked for from the character selection on", NetMngr.getConnectionStateCStr()));
		return;
	}
	std::string text;
	bool arrived = STRING_MANAGER::CStringManagerClient::instance()->getString(id, text);
	CJsonValue r = CJsonValue::object();
	r.set("id", id);
	r.set("arrived", arrived);
	if (arrived)
		r.set("text", text);
	else
		r.set("next", "asked of the shard, once (an id asked for already is not asked again): strings.get gives the text when it has arrived");
	call.reply(r);
}

void methodUiString(CCtrlCall &call)
{
	const std::string &key = call.getArgs().get("key").getString();
	CJsonValue r = CJsonValue::object();
	r.set("key", key);
	r.set("exists", CI18N::hasTranslation(key));
	r.set("text", CI18N::get(key));
	r.set("language", CI18N::getCurrentLanguageCode());
	call.reply(r);
}

/// The virtual desktops of the session, the one shown and the one the window is on (Windows)
void reportDesktop(CJsonValue &r)
{
#ifdef NL_OS_WINDOWS
	std::vector<std::string> names;
	uint shown = 0;
	std::string error;
	if (!CSystemUtils::getVirtualDesktops(names, shown, error))
	{
		r.set("desktops_error", error);
		return;
	}
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < names.size(); ++i)
		list.push(CJsonValue::object().set("number", i + 1).set("name", names[i]));
	r.set("desktops", list);
	r.set("shown_desktop", shown);
	uint number = 0;
	if (CSystemUtils::getWindowDesktop(Driver->getDisplay(), number, error))
		r.set("desktop", number);
	else
		r.set("desktop_error", error);
	r.set("on_shown_desktop", number != 0 && number == shown);
	if (WindowDesktop::isRequested())
		r.set("desktop_option", WindowDesktop::getRequested());
#else
	(void)r;
#endif
}

void methodWindow(CCtrlCall &call)
{
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	uint32 uiWidth = 0, uiHeight = 0;
	NLGUI::CViewRenderer::getInstance()->getScreenSize(uiWidth, uiHeight);
	CJsonValue r = CJsonValue::object();
	r.set("width", width);
	r.set("height", height);
	r.set("ui_scale", uiWidth > 0 ? floor((double)width / uiWidth * 1000.0 + 0.5) / 1000.0 : 1.0);
	r.set("interface_scale_setting", ClientCfg.InterfaceScale);
	// the mode of the window, and the one of the configuration (a client on another desktop stays in a window)
	NL3D::UDriver::CMode screenMode;
	if (Driver->getCurrentScreenMode(screenMode))
		r.set("fullscreen", !screenMode.Windowed);
	r.set("fullscreen_setting", !ClientCfg.Windowed);
	r.set("minimized", Driver->isWindowMinimized());
	r.set("focus", CInputHandlerManager::getInstance()->hasFocus());
	r.set("driven", isDriven());
	static const char *const s_Drivers[] = { "auto", "opengl", "direct3d", "opengles", "opengl3" };
	uint driver = (uint)ClientCfg.Driver3D;
	r.set("driver", driver < sizeof(s_Drivers) / sizeof(s_Drivers[0]) ? s_Drivers[driver] : "unknown");
	reportDesktop(r);
	call.reply(r);
}

void methodDesktop(CCtrlCall &call)
{
	const CJsonValue &value = call.getArgs().get("desktop");
	sint64 asked = 0;
	if (value.isNumber() && (!readInteger(value, asked) || asked < 1 || asked > 1000))
	{
		call.fail("desktop: a number counts from 1, for the first desktop");
		return;
	}
	std::string desktop = value.isString() ? value.getString() : (value.isNumber() ? toString("%d", (sint)asked) : std::string());
	if (desktop.empty())
	{
		call.fail("desktop: a virtual desktop by its number (1 for the first one) or the name it was given in the task view");
		return;
	}
	NL3D::UDriver::CMode mode;
	if (Driver->getCurrentScreenMode(mode) && !mode.Windowed)
	{
		call.fail("the client is in a full screen mode, which would take the monitor of the desktop shown: put it in a window first");
		return;
	}
	std::string error;
	uint number = 0;
	if (!CSystemUtils::moveWindowToDesktop(Driver->getDisplay(), desktop, number, error))
	{
		call.fail(error);
		return;
	}
	// never activated from then on, as activating it on a desktop that is not shown would switch the screen there,
	// and kept out of a full screen mode
	Driver->setWindowActivation(false);
	WindowDesktop::setRequested(desktop);
	CJsonValue r = CJsonValue::object();
	r.set("moved", true);
	reportDesktop(r);
	call.reply(r);
}

/// Frames counted over a period
class CPerfData : public ICtrlCallData
{
public:
	uint64 FirstFrame;
	TTime Start;
	TTime Duration;
};

void methodPerf(CCtrlCall &call)
{
	double seconds = 2.0;
	if (!readNumber(call, "seconds", 1e6, seconds))
		return;
	CPerfData *data = new CPerfData();
	data->FirstFrame = getEndpoint() != nullptr ? getEndpoint()->getFrame() : 0;
	data->Start = CTime::getLocalTime();
	data->Duration = (TTime)(std::max(0.2, std::min(60.0, seconds)) * 1000.0);
	call.setData(data);
}

void pollPerf(CCtrlCall &call)
{
	CPerfData *data = static_cast<CPerfData *>(call.getData());
	TTime elapsed = CTime::getLocalTime() - data->Start;
	if (elapsed < data->Duration)
		return;
	uint64 frames = (getEndpoint() != nullptr ? getEndpoint()->getFrame() : 0) - data->FirstFrame;
	CJsonValue r = CJsonValue::object();
	r.set("frames", frames);
	r.set("ms", (sint64)elapsed);
	r.set("fps", floor((double)frames * 1000.0 / (double)elapsed * 10.0 + 0.5) / 10.0);
	r.set("frame_ms", frames > 0 ? floor((double)elapsed / (double)frames * 100.0 + 0.5) / 100.0 : 0.0);
	call.reply(r);
}

} // anonymous namespace

void dataConnectionClosed(uint32 connection)
{
	for (std::map<std::string, TWatchPtr>::iterator it = s_Watches.begin(); it != s_Watches.end(); ++it)
		if (it->second->Owners.erase(connection) != 0 && it->second->Owners.empty())
			unwatch(it->second);
}

void releaseDataMethods()
{
	// the observers stay on their nodes, and alive, until the client exits: the database may be gone already, and with
	// no connection left they add nothing
	for (std::map<std::string, TWatchPtr>::iterator it = s_Watches.begin(); it != s_Watches.end(); ++it)
		it->second->Owners.clear();
}

void registerDataMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("db.get", "A leaf of the client database by path (SERVER:..., UI:..., LOCAL:...): its value.",
	    CCtrlSchema().str("path", "The leaf, SERVER:USER:MONEY for example.", true), true, methodGet));
	endpoint.addMethod(new CCtrlFunctionMethod("db.list", "The nodes of a branch of the client database: leaves with their values, branches with their size, to a depth.",
	    CCtrlSchema().str("path", "The branch (the root when empty).").integer("depth", "Levels to go down (1 by default).").integer("max", "At most this many nodes (500 by default)."), true,
	    methodList));
	// a raw write, past whatever the interface checks before it writes: privileged, so it runs only in a client whose
	// content is trusted
	ICtrlMethod *dbSet = new CCtrlFunctionMethod("db.set", "Sets a leaf of the database where Lua may: not SERVER:, and of LOCAL: only LOCAL:R2; UI: and the others are written.",
	    CCtrlSchema().str("path", "The leaf.", true).any("value", "An integer: a whole number, or its decimal text for a value beyond 2^53.", true), false, methodSet);
	dbSet->setPrivileged(true);
	endpoint.addMethod(dbSet);
	endpoint.addMethod(new CCtrlFunctionMethod("db.watch",
	    "Watches a leaf or branch for this connection: each change is a db event with its path and value (ctrl.events, kinds db), until stop or until the connection closes. "
	    "Not the root, at most 4096 leaves a watch and 64 watches; at most 100 db events a second, and dropped_before counts those left out. A branch the client makes again "
	    "(the UI branch at each login) needs db.watch again.",
	    CCtrlSchema().str("path", "The node.", true).boolean("stop", "Stop watching it."), false, methodWatch));
	endpoint.addMethod(new CCtrlFunctionMethod("strings.get",
	    "A string the client has, by id (a static string, or a dynamic one with dynamic): its text and whether it has arrived from the shard. It asks the shard for nothing: "
	    "strings.request asks for a static string.",
	    CCtrlSchema().integer("id", "The string id.", true).boolean("dynamic", "A dynamic string id."), true, methodString));
	endpoint.addMethod(new CCtrlFunctionMethod("strings.request",
	    "Asks the shard for a static string by id, as the client does for a string it shows, and gives its text if the client has it already; strings.get reads it once it has arrived. "
	    "A dynamic string cannot be asked for: the shard sends it with what uses it.",
	    CCtrlSchema().integer("id", "The string id.", true), false, methodStringRequest));
	endpoint.addMethod(new CCtrlFunctionMethod("strings.ui", "A text of the interface by its key (uiXxx) in the current language, and whether the key has a translation.",
	    CCtrlSchema().str("key", "The key.", true), true, methodUiString));
	endpoint.addMethod(new CCtrlFunctionMethod("client.window",
	    "The window: size in pixels, UI scale, fullscreen, minimized, focus, whether a tool drives it, the driver, and on Windows the virtual desktops (numbered from 1), "
	    "the one shown and the one the window is on.",
	    CCtrlSchema(), true, methodWindow));
	endpoint.addMethod(new CCtrlFunctionMethod("client.desktop",
	    "Moves the window to another virtual desktop (Windows), by its number (1 for the first one) or the name it was given in the task view. It is not activated, "
	    "and the desktop shown stays as it is; a client on a desktop that is not shown goes on running, and a tool that drives it counts as its focus.",
	    CCtrlSchema().any("desktop", "The desktop: a number, or a name.", true), false, methodDesktop));
	endpoint.addMethod(new CCtrlFunctionMethod("client.perf", "The frame rate over a few seconds: frames, fps, mean frame time.",
	    CCtrlSchema().number("seconds", "How long to count (2 by default, 60 at most)."), true, methodPerf, pollPerf));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_data.cpp */
