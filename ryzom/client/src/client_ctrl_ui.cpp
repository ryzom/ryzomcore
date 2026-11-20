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

// The interface methods of the control endpoint: ui.windows, ui.tree, ui.get, ui.at, ui.click,
// ui.hover, ui.drag, ui.wheel, ui.set_text. Elements go by their id (ui:interface:...). Rectangles and
// points are window pixels from the top left, the pixels of input.mouse and of screen.grab; the
// interface's own units are the pixels divided by ui_scale. A click by id goes through the mouse,
// at the centre of the element, after checking that the element is what the mouse would find there:
// a click that passes a modal window, or another window over the element, fails with what covers it.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/gui/ctrl_base.h"
#include "nel/gui/ctrl_base_button.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/view_text.h"
#include "nel/gui/widget_manager.h"

#include "global.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

namespace ClientCtrl {

namespace {

/// The window size in pixels and the interface size in its units
struct CUiScale
{
	uint32 WindowWidth, WindowHeight;
	uint32 UiWidth, UiHeight;
	double Scale;

	CUiScale()
	{
		Driver->getWindowSize(WindowWidth, WindowHeight);
		CViewRenderer::getInstance()->getScreenSize(UiWidth, UiHeight);
		// the scale the renderer draws with: the interface size is it rounded down
		Scale = CViewRenderer::getInstance()->getInterfaceScale();
		if (Scale <= 0.0)
			Scale = 1.0;
	}

	CJsonValue rect(const CInterfaceElement *e) const
	{
		// the interface counts from the bottom left
		CJsonValue r = CJsonValue::object();
		r.set("x", floor(e->getXReal() * Scale + 0.5));
		r.set("y", floor(WindowHeight - (e->getYReal() + e->getHReal()) * Scale + 0.5));
		r.set("width", floor(e->getWReal() * Scale + 0.5));
		r.set("height", floor(e->getHReal() * Scale + 0.5));
		return r;
	}

	void toUi(double px, double py, sint32 &ux, sint32 &uy) const
	{
		ux = (sint32)floor(px / Scale);
		uy = (sint32)floor((WindowHeight - py) / Scale);
	}

	void centre(const CInterfaceElement *e, double &px, double &py) const
	{
		px = (e->getXReal() + e->getWReal() * 0.5) * Scale;
		py = WindowHeight - (e->getYReal() + e->getHReal() * 0.5) * Scale;
	}
};

CJsonValue scaleJson(const CUiScale &s)
{
	CJsonValue r = CJsonValue::object();
	r.set("window", CJsonValue::object().set("width", s.WindowWidth).set("height", s.WindowHeight));
	r.set("interface", CJsonValue::object().set("width", s.UiWidth).set("height", s.UiHeight));
	r.set("ui_scale", floor(s.Scale * 1000.0 + 0.5) / 1000.0);
	return r;
}

/// What a tool needs to know of an element, without its children
CJsonValue elementJson(const CInterfaceElement *e, const CUiScale &scale)
{
	CJsonValue o = CJsonValue::object();
	o.set("id", e->getId());
	o.set("type", const_cast<CInterfaceElement *>(e)->getClassName());
	o.set("rect", scale.rect(e));
	o.set("active", e->getActive());
	if (!e->isActiveThroughParents())
		o.set("hidden_by_parent", true);
	if (const CViewText *text = dynamic_cast<const CViewText *>(e))
		o.set("text", text->getText());
	if (const CCtrlTextButton *button = dynamic_cast<const CCtrlTextButton *>(e))
		o.set("text", button->getText());
	if (const CGroupEditBox *edit = dynamic_cast<const CGroupEditBox *>(e))
		o.set("input", edit->getInputString());
	if (const CGroupContainer *container = dynamic_cast<const CGroupContainer *>(e))
		o.set("title", container->getTitle());
	if (const CCtrlBase *ctrl = dynamic_cast<const CCtrlBase *>(e))
	{
		std::string help = ctrl->getDefaultContextHelp();
		if (!help.empty())
			o.set("tooltip", help);
	}
	if (const CCtrlBaseButton *button = dynamic_cast<const CCtrlBaseButton *>(e))
	{
		std::string left = e->getProperty("onclick_l");
		if (!left.empty())
		{
			o.set("onclick_l", left);
			std::string params = e->getProperty("params_l");
			if (!params.empty())
				o.set("params_l", params);
		}
		std::string right = e->getProperty("onclick_r");
		if (!right.empty())
			o.set("onclick_r", right);
		o.set("pushed", button->getPushed());
	}
	return o;
}

struct CTreeWalk
{
	const CUiScale *Scale;
	uint MaxDepth;
	uint Budget;
	bool ActiveOnly;
	std::string Contains;
	bool Truncated;

	/// The element and what is below it; a filter keeps the path to what it matches
	bool walk(CInterfaceElement *e, uint depth, CJsonValue &out)
	{
		if (ActiveOnly && !e->getActive())
			return false;
		if (Budget == 0)
		{
			Truncated = true;
			return false;
		}
		CJsonValue node = elementJson(e, *Scale);
		bool matches = Contains.empty() || toLowerAscii(e->getId()).find(Contains) != std::string::npos
		    || toLowerAscii(node.get("text").getString()).find(Contains) != std::string::npos;
		CInterfaceGroup *group = dynamic_cast<CInterfaceGroup *>(e);
		if (group != nullptr)
		{
			uint children = (uint)(group->getGroups().size() + group->getControls().size() + group->getViews().size());
			if (depth >= MaxDepth)
			{
				if (children > 0)
					node.set("children_not_shown", children);
			}
			else
			{
				CJsonValue list = CJsonValue::array();
				bool any = false;
				const std::vector<CInterfaceGroup *> &groups = group->getGroups();
				for (uint i = 0; i < groups.size(); ++i)
					any = walkChild(groups[i], depth, list) || any;
				const std::vector<CCtrlBase *> &ctrls = group->getControls();
				for (uint i = 0; i < ctrls.size(); ++i)
					any = walkChild(ctrls[i], depth, list) || any;
				const std::vector<CViewBase *> &views = group->getViews();
				for (uint i = 0; i < views.size(); ++i)
					any = walkChild(views[i], depth, list) || any;
				if (list.size() > 0)
					node.set("children", list);
				matches = matches || any;
			}
		}
		if (!matches)
			return false;
		--Budget;
		out.push(node);
		return true;
	}

	bool walkChild(CInterfaceElement *e, uint depth, CJsonValue &list)
	{
		return e != nullptr && walk(e, depth + 1, list);
	}
};

void methodWindows(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CUiScale scale;
	CWidgetManager *wm = CWidgetManager::getInstance();
	bool all = call.getArgs().get("all").getBool(false);
	CJsonValue masters = CJsonValue::array();
	std::vector<CWidgetManager::SMasterGroup> &groups = wm->getAllMasterGroup();
	for (uint m = 0; m < groups.size(); ++m)
	{
		CWidgetManager::SMasterGroup &mg = groups[m];
		if (mg.Group == nullptr || (!all && !mg.Group->getActive()))
			continue;
		CJsonValue windows = CJsonValue::array();
		// front to back: the highest priority first, the last of a list in front
		for (uint p = WIN_PRIORITY_MAX; p > 0; --p)
		{
			const std::list<CInterfaceGroup *> &list = mg.PrioritizedWindows[p - 1];
			for (std::list<CInterfaceGroup *>::const_reverse_iterator it = list.rbegin(); it != list.rend(); ++it)
			{
				if (!all && !(*it)->getActive())
					continue;
				CJsonValue w = elementJson(*it, scale);
				w.set("priority", p - 1);
				windows.push(w);
			}
		}
		masters.push(CJsonValue::object().set("id", mg.Group->getId()).set("active", mg.Group->getActive()).set("windows", windows));
	}
	CJsonValue r = scaleJson(scale);
	r.set("master_groups", masters);
	CInterfaceGroup *modal = wm->getModalWindow();
	r.set("modal", modal != nullptr ? CJsonValue(modal->getId()) : CJsonValue());
	CCtrlBase *keyboard = wm->getCaptureKeyboard();
	r.set("keyboard_focus", keyboard != nullptr ? CJsonValue(keyboard->getId()) : CJsonValue());
	call.reply(r);
}

void methodTree(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CUiScale scale;
	CTreeWalk walk;
	walk.Scale = &scale;
	walk.MaxDepth = (uint)std::max((sint64)0, std::min((sint64)64, call.getArgs().get("depth").getInt(4)));
	walk.Budget = (uint)std::max((sint64)1, std::min((sint64)5000, call.getArgs().get("max").getInt(400)));
	walk.ActiveOnly = call.getArgs().get("active_only").getBool(true);
	walk.Contains = toLowerAscii(call.getArgs().get("contains").getString());
	walk.Truncated = false;
	CJsonValue nodes = CJsonValue::array();
	const std::string &id = call.getArgs().get("id").getString();
	if (!id.empty())
	{
		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId(id);
		if (e == nullptr)
		{
			call.fail(toString("no interface element '%s'", id.c_str()));
			return;
		}
		walk.walk(e, 0, nodes);
	}
	else
	{
		// the active windows of the active master groups, front to back
		std::vector<CWidgetManager::SMasterGroup> &groups = CWidgetManager::getInstance()->getAllMasterGroup();
		for (uint m = 0; m < groups.size(); ++m)
		{
			if (groups[m].Group == nullptr || !groups[m].Group->getActive())
				continue;
			for (uint p = WIN_PRIORITY_MAX; p > 0; --p)
			{
				const std::list<CInterfaceGroup *> &list = groups[m].PrioritizedWindows[p - 1];
				for (std::list<CInterfaceGroup *>::const_reverse_iterator it = list.rbegin(); it != list.rend(); ++it)
					walk.walk(*it, 1, nodes);
			}
		}
	}
	CJsonValue r = scaleJson(scale);
	r.set("elements", nodes);
	r.set("truncated", walk.Truncated);
	if (walk.Truncated)
		r.set("note", "more elements than max: narrow with id, depth or contains");
	call.reply(r);
}

void methodGet(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	const std::string &id = call.getArgs().get("id").getString();
	CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId(id);
	if (e == nullptr)
	{
		call.fail(toString("no interface element '%s'", id.c_str()));
		return;
	}
	CUiScale scale;
	CJsonValue r = elementJson(e, scale);
	const CJsonValue &names = call.getArgs().get("properties");
	if (names.size() > 0)
	{
		CJsonValue properties = CJsonValue::object();
		for (uint i = 0; i < names.size(); ++i)
			properties.set(names.at(i).getString(), e->getProperty(names.at(i).getString()));
		r.set("properties", properties);
	}
	if (e->getParent() != nullptr)
		r.set("parent", e->getParent()->getId());
	call.reply(r);
}

/// The front-most controls and groups at a window pixel, as the mouse would find them
void underPoint(double px, double py, std::vector<CCtrlBase *> &ctrls, std::vector<CInterfaceGroup *> &groups)
{
	CUiScale scale;
	sint32 ux, uy;
	scale.toUi(px, py, ux, uy);
	CWidgetManager::getInstance()->getCtrlsUnder(ux, uy, ctrls);
	CWidgetManager::getInstance()->getGroupsUnder(ux, uy, groups);
}

void methodAt(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	double px = call.getArgs().get("x").getNumber(-1), py = call.getArgs().get("y").getNumber(-1);
	if (px < 0 || py < 0)
	{
		call.fail("x and y are required, in window pixels");
		return;
	}
	std::vector<CCtrlBase *> ctrls;
	std::vector<CInterfaceGroup *> groups;
	underPoint(px, py, ctrls, groups);
	CUiScale scale;
	CJsonValue c = CJsonValue::array();
	for (uint i = 0; i < ctrls.size(); ++i)
		c.push(elementJson(ctrls[i], scale));
	CJsonValue g = CJsonValue::array();
	for (uint i = 0; i < groups.size(); ++i)
		g.push(CJsonValue(groups[i]->getId()));
	CJsonValue r = CJsonValue::object();
	r.set("controls", c);
	r.set("groups", g);
	CInterfaceGroup *modal = CWidgetManager::getInstance()->getModalWindow();
	if (modal != nullptr)
		r.set("modal", modal->getId());
	call.reply(r);
}

/// Whether one element is the other or inside it
bool within(const CInterfaceElement *e, const CInterfaceElement *ancestor)
{
	for (; e != nullptr; e = e->getParent())
		if (e == ancestor)
			return true;
	return false;
}

/// The point of an element where the mouse acts on it; false with the call failed when the mouse would not reach it
bool reachElement(CCtrlCall &call, const std::string &idArg, double &px, double &py)
{
	const std::string &id = call.getArgs().get(idArg).getString();
	CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId(id);
	if (e == nullptr)
	{
		call.fail(toString("no interface element '%s'", id.c_str()));
		return false;
	}
	if (!e->isActiveThroughParents())
	{
		call.fail(toString("'%s' is not shown (it or a parent is not active)", id.c_str()));
		return false;
	}
	CUiScale scale;
	scale.centre(e, px, py);
	// an offset from the centre, in pixels, for an element whose centre is not where it takes clicks
	px += call.getArgs().get("dx").getNumber(0);
	py += call.getArgs().get("dy").getNumber(0);
	if (px < 0 || py < 0 || px >= scale.WindowWidth || py >= scale.WindowHeight)
	{
		call.fail(toString("the centre of '%s' is outside the window", id.c_str()));
		return false;
	}
	std::vector<CCtrlBase *> ctrls;
	std::vector<CInterfaceGroup *> groups;
	underPoint(px, py, ctrls, groups);
	// the mouse finds the control under it; a group (a list, a window) takes what is in it
	bool reached = false;
	for (uint i = 0; i < ctrls.size() && !reached; ++i)
		reached = within(ctrls[i], e) || within(e, ctrls[i]);
	if (!reached && dynamic_cast<CCtrlBase *>(e) == nullptr)
		for (uint i = 0; i < groups.size() && !reached; ++i)
			reached = within(groups[i], e);
	if (!reached)
	{
		CJsonValue covering = CJsonValue::array();
		for (uint i = 0; i < ctrls.size(); ++i)
			covering.push(CJsonValue(ctrls[i]->getId()));
		if (ctrls.empty())
			for (uint i = 0; i < groups.size() && i < 3; ++i)
				covering.push(CJsonValue(groups[i]->getId()));
		CJsonValue details = CJsonValue::object();
		details.set("under_the_point", covering);
		details.set("x", px);
		details.set("y", py);
		CInterfaceGroup *modal = CWidgetManager::getInstance()->getModalWindow();
		if (modal != nullptr)
			details.set("modal", modal->getId());
		call.fail(toString("the mouse would not reach '%s' at its centre: something else is there (a modal window, another window, or nothing that takes clicks)", id.c_str()), details);
		return false;
	}
	return true;
}

void methodClick(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	double px, py;
	if (!reachElement(call, "id", px, py))
		return;
	std::string action = call.getArgs().get("double").getBool(false) ? "double_click" : "click";
	std::string error;
	if (!postMouse(action, px, py, call.getArgs().get("button").getString("left"), call.getArgs(), 0, error))
	{
		call.fail(error);
		return;
	}
	CJsonValue r = CJsonValue::object();
	r.set("id", call.getArgs().get("id"));
	r.set("x", px);
	r.set("y", py);
	r.set("action", action);
	finishAfterPump(call, r);
}

void methodHover(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	double px, py;
	if (!reachElement(call, "id", px, py))
		return;
	std::string error;
	postMouse("move", px, py, "left", call.getArgs(), 0, error);
	finishAfterPump(call, CJsonValue::object().set("id", call.getArgs().get("id")).set("x", px).set("y", py));
}

void methodWheel(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	double px, py;
	if (!reachElement(call, "id", px, py))
		return;
	sint steps = (sint)call.getArgs().get("steps").getInt(-1);
	std::string error;
	if (steps == 0 || !postMouse("wheel", px, py, "left", call.getArgs(), steps, error))
	{
		call.fail(error.empty() ? "steps is not 0: positive up, negative down" : error);
		return;
	}
	finishAfterPump(call, CJsonValue::object().set("id", call.getArgs().get("id")).set("steps", steps));
}

void methodDrag(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	double fx, fy;
	if (!reachElement(call, "from", fx, fy))
		return;
	double tx, ty;
	if (call.getArgs().get("to").isString())
	{
		// the target only needs to be where the drop lands; what covers it is the drop's business
		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId(call.getArgs().get("to").getString());
		if (e == nullptr)
		{
			call.fail("no interface element '" + call.getArgs().get("to").getString() + "'");
			return;
		}
		CUiScale scale;
		scale.centre(e, tx, ty);
	}
	else if (call.getArgs().get("x").isNumber() && call.getArgs().get("y").isNumber())
	{
		tx = call.getArgs().get("x").getNumber();
		ty = call.getArgs().get("y").getNumber();
	}
	else
	{
		call.fail("to (an element id) or x and y (window pixels) are required");
		return;
	}
	// the drop must be in the window, or the button would stay held
	CUiScale scale;
	if (tx < 0 || ty < 0 || tx >= scale.WindowWidth || ty >= scale.WindowHeight)
	{
		call.fail(toString("the drop point %.0f, %.0f is outside the client area, which is %ux%u", tx, ty, scale.WindowWidth, scale.WindowHeight));
		return;
	}
	std::string error;
	// press, move in steps as a hand does (a drag starts after a few pixels), release
	bool ok = postMouse("down", fx, fy, "left", call.getArgs(), 0, error);
	for (uint i = 1; i <= 8 && ok; ++i)
		ok = postMouse("move_held", fx + (tx - fx) * i / 8.0, fy + (ty - fy) * i / 8.0, "left", call.getArgs(), 0, error);
	if (!postMouse("up", ok ? tx : fx, ok ? ty : fy, "left", call.getArgs(), 0, error) || !ok)
	{
		call.fail(error);
		return;
	}
	finishAfterPump(call, CJsonValue::object().set("from", call.getArgs().get("from")).set("x", tx).set("y", ty));
}

void methodSetText(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	const std::string &id = call.getArgs().get("id").getString();
	CGroupEditBox *edit = dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId(id));
	if (edit == nullptr)
	{
		call.fail(toString("'%s' is not an edit box (the id of a CGroupEditBox, often ending in :eb)", id.c_str()));
		return;
	}
	edit->setInputString(call.getArgs().get("text").getString());
	bool submit = call.getArgs().get("submit").getBool(false);
	if (submit)
	{
		// Enter in the box, as a player would press it there
		CWidgetManager::getInstance()->setCaptureKeyboard(edit);
		std::string error;
		postKey("KeyRETURN", error);
	}
	finishAfterPump(call, CJsonValue::object().set("id", id).set("input", edit->getInputString()).set("submitted", submit));
}

} // anonymous namespace

namespace {

// the windows shown at the last look, by id, and the modal window then
std::map<std::string, bool> s_ShownWindows;
std::string s_ShownModal;
TTime s_LastWindowLook = 0;

} // anonymous namespace

void trackWindows()
{
	// four looks a second: a window that opens is known within a frame or so, without a walk at every frame
	TTime now = CTime::getLocalTime();
	if (now - s_LastWindowLook < 250)
		return;
	s_LastWindowLook = now;
	CWidgetManager *wm = CWidgetManager::getInstance();
	std::map<std::string, bool> shown;
	std::map<std::string, std::string> titles;
	std::vector<CWidgetManager::SMasterGroup> &masters = wm->getAllMasterGroup();
	for (uint m = 0; m < masters.size(); ++m)
	{
		if (masters[m].Group == nullptr || !masters[m].Group->getActive())
			continue;
		// the labels over the entities live in the world space priority: they come and go with the entities
		for (uint p = WIN_PRIORITY_WORLD_SPACE + 1; p < WIN_PRIORITY_MAX; ++p)
		{
			const std::list<CInterfaceGroup *> &list = masters[m].PrioritizedWindows[p];
			for (std::list<CInterfaceGroup *>::const_iterator it = list.begin(); it != list.end(); ++it)
			{
				if (!(*it)->getActive())
					continue;
				shown[(*it)->getId()] = true;
				if (const CGroupContainer *container = dynamic_cast<const CGroupContainer *>(*it))
					titles[(*it)->getId()] = container->getTitle();
			}
		}
	}
	for (std::map<std::string, bool>::const_iterator it = shown.begin(); it != shown.end(); ++it)
	{
		if (s_ShownWindows.find(it->first) != s_ShownWindows.end())
			continue;
		CJsonValue e = CJsonValue::object();
		e.set("id", it->first);
		e.set("action", "opened");
		std::map<std::string, std::string>::const_iterator title = titles.find(it->first);
		if (title != titles.end() && !title->second.empty())
			e.set("title", title->second);
		addEvent("window", e);
	}
	for (std::map<std::string, bool>::const_iterator it = s_ShownWindows.begin(); it != s_ShownWindows.end(); ++it)
		if (shown.find(it->first) == shown.end())
			addEvent("window", CJsonValue::object().set("id", it->first).set("action", "closed"));
	s_ShownWindows.swap(shown);
	CInterfaceGroup *modal = wm->getModalWindow();
	std::string modalId = modal != nullptr ? modal->getId() : std::string();
	if (modalId != s_ShownModal)
	{
		// one modal window in the place of another: the first closed
		if (!s_ShownModal.empty())
			addEvent("window", CJsonValue::object().set("id", s_ShownModal).set("action", "modal_closed"));
		if (!modalId.empty())
			addEvent("window", CJsonValue::object().set("id", modalId).set("action", "modal_opened"));
		s_ShownModal = modalId;
	}
}

void registerUiMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("ui.windows",
	    "The open windows, front to back per master group (ui:interface in the game, ui:login and ui:outgame before), with their id, type, rectangle in window pixels and title; "
	    "the modal window, which takes all input while open; and what has the keyboard focus. all includes the hidden ones. A window that opens or closes, and the modal window, are also window events: get_events with kinds window and wait_seconds waits for one.",
	    CCtrlSchema().boolean("all", "Also the windows that are not active."), true, methodWindows));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.tree",
	    "The element tree of a window or element (id), or of every open window: id, type, rectangle in window pixels, active, text, edit box input, tooltip, the action handler of a button. "
	    "depth bounds it (4 by default), contains keeps the elements whose id or text has the word and the path to them, max caps it (truncated says when).",
	    CCtrlSchema()
	        .str("id", "The root element, ui:interface:... (every open window by default).")
	        .integer("depth", "How many levels below the root (4 by default).")
	        .str("contains", "Only the elements whose id or text contains this (case-insensitive), with their parents.")
	        .boolean("active_only", "Only the active elements (the default).")
	        .integer("max", "At most this many elements (400 by default)."),
	    true, methodTree));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.get", "One element: what ui.tree gives for it, its parent, and the properties asked for (any XML property of its type: text, onclick_l, x, posref, color...).",
	    CCtrlSchema().str("id", "The element.", true).strings("properties", "The property names."), true, methodGet));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.at", "What the mouse would find at a window pixel: the controls, front-most first, and the groups.",
	    CCtrlSchema().number("x", "Window pixels from the left.", true).number("y", "Window pixels from the top.", true), true, methodAt));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.click",
	    "Clicks an element by id with the mouse, at its centre (dx, dy move the point), after checking that the mouse would reach it there; it fails with what covers it otherwise. "
	    "Answers once the client has handled the click.",
	    CCtrlSchema()
	        .str("id", "The element.", true)
	        .choice("button", "left|right|middle", "The button (left by default).")
	        .boolean("double", "A double click.")
	        .number("dx", "Pixels right of the centre.")
	        .number("dy", "Pixels below the centre.")
	        .boolean("ctrl", "With Ctrl held.")
	        .boolean("shift", "With Shift held."),
	    false, methodClick, pollAfterPump));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.hover", "Moves the mouse over an element, for its tooltip or its hover state.",
	    CCtrlSchema().str("id", "The element.", true).number("dx", "Pixels right of the centre.").number("dy", "Pixels below the centre."), false, methodHover, pollAfterPump));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.wheel", "Turns the mouse wheel over an element: a list or a scroll area scrolls.",
	    CCtrlSchema().str("id", "The element.", true).integer("steps", "Notches, positive up, negative down (-1 by default)."), false, methodWheel, pollAfterPump));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.drag",
	    "Drags an element with the left button to another element (to) or to a window pixel (x, y), as a hand does: press, move in steps, release. For items to slots and actions to the bar.",
	    CCtrlSchema().str("from", "The element to drag.", true).str("to", "The element to drop on.").number("x", "Or the window pixel to drop at.").number("y", "Its y."), false, methodDrag,
	    pollAfterPump));
	endpoint.addMethod(new CCtrlFunctionMethod("ui.set_text",
	    "Sets the text of an edit box, and with submit presses Enter in it, as typing and Enter would. Answers once the client has handled it.",
	    CCtrlSchema().str("id", "The edit box (often ...:eb).", true).str("text", "The text.", true).boolean("submit", "Press Enter in it after."), false, methodSetText, pollAfterPump));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_ui.cpp */
