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
// strings.ui, client.window, client.perf. The database is the client's (SERVER: what the server
// sends, UI: and LOCAL: the interface's own); as for Lua, the server branches are read-only.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/i18n.h"
#include "nel/misc/system_utils.h"

#include "client_cfg.h"
#include "global.h"
#include "interface_v3/input_handler_manager.h"
#include "string_manager_client.h"
#include "window_desktop.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

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

CJsonValue leafJson(const CCDBNodeLeaf *leaf)
{
	CJsonValue o = CJsonValue::object();
	sint64 v = leaf->getValue64();
	// the 64-bit values (sheet ids, entity ids) are exact as text
	o.set("value", v);
	if (v > 9007199254740992LL || v < -9007199254740992LL)
		o.set("value_text", toString("%" NL_I64 "d", v));
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
			if (depth + 1 >= maxDepth)
			{
				out.push(CJsonValue::object().set("path", child).set("branch", (uint32)sub->getNbNodes()));
			}
			else
			{
				out.push(CJsonValue::object().set("path", child).set("branch", (uint32)sub->getNbNodes()));
				listBranch(sub, child, depth + 1, maxDepth, budget, truncated, out);
			}
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
	if (startsWithCaseInsensitiveAscii(path, "SERVER:"))
	{
		call.fail("the server branches are read-only, as they are for Lua: the server owns them");
		return;
	}
	ICDBNode *node = findNode(path);
	if (node == nullptr || !node->isLeaf())
	{
		call.fail(toString(node == nullptr ? "no database leaf '%s'" : "'%s' is a branch", path.c_str()));
		return;
	}
	const CJsonValue &value = call.getArgs().get("value");
	sint64 v;
	if (value.isString())
	{
		if (!fromString(value.getString(), v))
		{
			call.fail("value is a number, or its text");
			return;
		}
	}
	else
	{
		v = value.getInt(0);
	}
	static_cast<CCDBNodeLeaf *>(node)->setValue64(v);
	CJsonValue r = leafJson(static_cast<CCDBNodeLeaf *>(node));
	r.set("path", path);
	call.reply(r);
}

/// A watched node: the changes of its leaves are events, each with the leaf that changed
class CWatch : public ICDBNode::IPropertyObserver
{
public:
	std::string Path;
	ICDBNode *Node;
	virtual void update(ICDBNode *node) NL_OVERRIDE
	{
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf *>(node);
		CJsonValue e = CJsonValue::object();
		e.set("path", node != nullptr ? node->getFullName() : Path);
		if (node != nullptr && node != Node)
			e.set("watch", Path);
		if (leaf != nullptr)
			e.set("value", leaf->getValue64());
		addEvent("db", e);
	}
};

std::map<std::string, CWatch *> s_Watches;
// the observers are reference counted by the nodes too: these keep them alive whatever the nodes do
std::vector<NLMISC::CSmartPtr<CWatch>> s_WatchObjects;

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

void methodWatch(CCtrlCall &call)
{
	const std::string &path = call.getArgs().get("path").getString();
	bool stop = call.getArgs().get("stop").getBool(false);
	ICDBNode *node = findNode(path);
	std::map<std::string, CWatch *>::iterator it = s_Watches.find(path);
	if (stop)
	{
		// the node of the watch, if it is still there: a node taken down took its observers with it
		if (it != s_Watches.end() && node != nullptr && node == it->second->Node)
			removeWatch(node, it->second);
		if (it != s_Watches.end())
			s_Watches.erase(it);
		call.reply(CJsonValue::object().set("path", path).set("watched", false));
		return;
	}
	if (node == nullptr)
	{
		call.fail(toString("no database node '%s'", path.c_str()));
		return;
	}
	// a node of that path made again since (the UI branch at each login), maybe at the same address, needs the
	// observer again: it is taken off first, so a node that still has it does not get it twice
	if (it != s_Watches.end() && it->second->Node == node)
	{
		removeWatch(node, it->second);
		ICDBNode::CTextId id;
		node->addObserver(it->second, id);
	}
	else
	{
		CWatch *watch = new CWatch();
		watch->Path = path;
		watch->Node = node;
		s_WatchObjects.push_back(watch);
		ICDBNode::CTextId id;
		node->addObserver(watch, id);
		s_Watches[path] = watch;
	}
	call.reply(CJsonValue::object().set("path", path).set("watched", true).set("events", "db events with the path and value of the leaf that changed, through ctrl.events (kinds db)"));
}

void methodString(CCtrlCall &call)
{
	sint64 id = call.getArgs().get("id").getInt(-1);
	if (id < 0)
	{
		call.fail("id is required");
		return;
	}
	bool dynamic = call.getArgs().get("dynamic").getBool(false);
	std::string text;
	bool arrived = dynamic ? STRING_MANAGER::CStringManagerClient::instance()->getDynString((uint32)id, text) : STRING_MANAGER::CStringManagerClient::instance()->getString((uint32)id, text);
	CJsonValue r = CJsonValue::object();
	r.set("id", id);
	r.set("dynamic", dynamic);
	r.set("arrived", arrived);
	r.set("text", text);
	if (!arrived)
		r.set("note", "asked of the server now; ask again in a moment");
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
	std::string desktop = value.isString() ? value.getString() : (value.isNumber() ? toString("%d", (sint)value.getInt(0)) : std::string());
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
	CPerfData *data = new CPerfData();
	data->FirstFrame = getEndpoint() != nullptr ? getEndpoint()->getFrame() : 0;
	data->Start = CTime::getLocalTime();
	data->Duration = (TTime)(std::max(0.2, std::min(60.0, call.getArgs().get("seconds").getNumber(2.0))) * 1000.0);
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

void releaseDataMethods()
{
	// the observers stay on their nodes until the database goes; they only add events, and the endpoint is gone
	s_Watches.clear();
}

void registerDataMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("db.get", "A leaf of the client database by path (SERVER:..., UI:..., LOCAL:...): its value.",
	    CCtrlSchema().str("path", "The leaf, SERVER:USER:MONEY for example.", true), true, methodGet));
	endpoint.addMethod(new CCtrlFunctionMethod("db.list", "The nodes of a branch of the client database: leaves with their values, branches with their size, to a depth.",
	    CCtrlSchema().str("path", "The branch (the root when empty).").integer("depth", "Levels to go down (1 by default).").integer("max", "At most this many nodes (500 by default)."), true,
	    methodList));
	endpoint.addMethod(new CCtrlFunctionMethod("db.set", "Sets a leaf of the UI or LOCAL branches; the server branches are read-only, as for Lua.",
	    CCtrlSchema().str("path", "The leaf.", true).any("value", "A number, or its text for 64-bit values.", true), false, methodSet));
	endpoint.addMethod(new CCtrlFunctionMethod("db.watch", "Watches a leaf or branch: each change is a db event with its path and value (ctrl.events, kinds db). stop ends it.",
	    CCtrlSchema().str("path", "The node.", true).boolean("stop", "Stop watching it."), true, methodWatch));
	endpoint.addMethod(new CCtrlFunctionMethod("strings.get",
	    "A string of the string manager by id (a static string, or a dynamic one with dynamic): its text and whether it has arrived from the server yet.",
	    CCtrlSchema().integer("id", "The string id.", true).boolean("dynamic", "A dynamic string id."), true, methodString));
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
