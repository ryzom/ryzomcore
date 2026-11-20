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

// The interface methods of the control endpoint: ui.windows, ui.tree, ui.get, ui.at, ui.click, ui.hover, ui.drag,
// ui.wheel, ui.set_text. Elements go by their id (ui:interface:...). Rectangles and points are window pixels from the
// top left, the pixels of input.mouse and of screen.grab; the interface's own units are the pixels divided by ui_scale.
// A click by id goes through the mouse, at the centre of the element, after checking that the press there goes to the
// element as the widget manager would give it: to the deepest control that takes captures in the window under the
// point. A click that a modal window keeps, or that another window or control over the element would take, fails with
// what takes it. The password of a password box is never given back, only its length.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/ctrl_base.h"
#include "nel/gui/ctrl_base_button.h"
#include "nel/gui/ctrl_draggable.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/view_text.h"
#include "nel/gui/widget_manager.h"

#include "global.h"

#include <cmath>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

namespace ClientCtrl {

// findElement and isShown, declared in client_ctrl_methods.h: CWidgetManager::getElementFromId would log an id that is not
// there, and keep it

CInterfaceElement *findElement(const std::string &id)
{
	if (id.empty())
		return nullptr;
	std::vector<CWidgetManager::SMasterGroup> &groups = CWidgetManager::getInstance()->getAllMasterGroup();
	for (uint m = 0; m < groups.size(); ++m)
	{
		if (groups[m].Group == nullptr)
			continue;
		CInterfaceElement *e = groups[m].Group->getElement(id);
		if (e != nullptr)
			return e;
	}
	return nullptr;
}

bool isShown(const CInterfaceElement *e)
{
	for (; e != nullptr; e = e->getParent())
		if (!e->getActive())
			return false;
	return true;
}

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

	bool hasArea() const { return WindowWidth > 0 && WindowHeight > 0 && UiWidth > 0 && UiHeight > 0; }

	CJsonValue rect(const CInterfaceElement *e) const
	{
		// the interface counts from the bottom left
		CJsonValue r = CJsonValue::object();
		r.set("x", std::floor(e->getXReal() * Scale + 0.5));
		r.set("y", std::floor(WindowHeight - (e->getYReal() + e->getHReal()) * Scale + 0.5));
		r.set("width", std::floor(e->getWReal() * Scale + 0.5));
		r.set("height", std::floor(e->getHReal() * Scale + 0.5));
		return r;
	}

	/// The interface point the pointer goes to for a window pixel, as the client computes it from the event postMouse
	/// posts: the position in 0..1 as a float, times the interface size, rounded (CWidgetManager::handleMouseMoveEvent)
	void toUi(double px, double py, sint32 &ux, sint32 &uy) const
	{
		float x = (float)(px / (double)WindowWidth);
		float y = (float)(1.0 - py / (double)WindowHeight);
		ux = (sint32)std::floor(x * UiWidth + 0.5f);
		uy = (sint32)std::floor(y * UiHeight + 0.5f);
	}

	/// The window pixel that toUi takes back to an interface point
	void toWindow(sint32 ux, sint32 uy, double &px, double &py) const
	{
		px = (double)ux * WindowWidth / UiWidth;
		py = WindowHeight - (double)uy * WindowHeight / UiHeight;
	}

	/// The interface point at the centre of an element, inside it by the rule of CInterfaceElement::isIn: x in
	/// [x, x + w), y in (y, y + h]
	void centreUi(const CInterfaceElement *e, sint32 &ux, sint32 &uy) const
	{
		ux = e->getXReal() + e->getWReal() / 2;
		uy = e->getYReal() + (e->getHReal() + 1) / 2;
	}

	void centre(const CInterfaceElement *e, double &px, double &py) const
	{
		sint32 ux, uy;
		centreUi(e, ux, uy);
		toWindow(ux, uy, px, py);
	}
};

CJsonValue scaleJson(const CUiScale &s)
{
	CJsonValue r = CJsonValue::object();
	r.set("window", CJsonValue::object().set("width", s.WindowWidth).set("height", s.WindowHeight));
	r.set("interface", CJsonValue::object().set("width", s.UiWidth).set("height", s.UiHeight));
	r.set("ui_scale", std::floor(s.Scale * 1000.0 + 0.5) / 1000.0);
	return r;
}

bool isPasswordBox(const CGroupEditBox *edit)
{
	return edit->getProperty("enter_type") == "password";
}

/// What an edit box holds: its text, or only the length of a password
void setInput(CJsonValue &o, const CGroupEditBox *edit)
{
	if (isPasswordBox(edit))
	{
		o.set("password", true);
		o.set("input_length", (uint32)edit->getInputStringRef().size());
	}
	else
	{
		o.set("input", edit->getInputString());
	}
	if (edit->getFrozen())
		o.set("frozen", true);
}

/// What a tool needs to know of an element, without its children
CJsonValue elementJson(const CInterfaceElement *e, const CUiScale &scale)
{
	CJsonValue o = CJsonValue::object();
	o.set("id", e->getId());
	o.set("type", const_cast<CInterfaceElement *>(e)->getClassName());
	o.set("rect", scale.rect(e));
	o.set("active", e->getActive());
	if (e->getActive() && !isShown(e))
		o.set("hidden_by_parent", true);
	if (const CViewText *text = dynamic_cast<const CViewText *>(e))
		o.set("text", text->getText());
	if (const CCtrlTextButton *button = dynamic_cast<const CCtrlTextButton *>(e))
		o.set("text", button->getText());
	if (const CGroupEditBox *edit = dynamic_cast<const CGroupEditBox *>(e))
		setInput(o, edit);
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
		if (button->getFrozen())
			o.set("frozen", true);
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
		// the slot of this element is taken before its children are walked, and given back when the filter leaves it out
		--Budget;
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
		{
			++Budget;
			return false;
		}
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
	const CJsonValue &args = call.getArgs();
	sint64 depth = args.get("depth").getInt(4);
	sint64 max = args.get("max").getInt(400);
	if (depth < 0 || depth > 64)
	{
		call.fail("depth is 0 to 64");
		return;
	}
	if (max < 1 || max > 5000)
	{
		call.fail("max is 1 to 5000");
		return;
	}
	CUiScale scale;
	CTreeWalk walk;
	walk.Scale = &scale;
	walk.MaxDepth = (uint)depth;
	walk.Budget = (uint)max;
	walk.ActiveOnly = args.get("active_only").getBool(true);
	walk.Contains = toLowerAscii(args.get("contains").getString());
	walk.Truncated = false;
	CJsonValue nodes = CJsonValue::array();
	const std::string &id = args.get("id").getString();
	if (!id.empty())
	{
		CInterfaceElement *e = findElement(id);
		if (e == nullptr)
		{
			call.fail(toString("no interface element '%s'", id.c_str()));
			return;
		}
		walk.walk(e, 0, nodes);
	}
	else
	{
		// the active windows of the active master groups, front to back, each at depth 0 as the root of a tree is
		std::vector<CWidgetManager::SMasterGroup> &groups = CWidgetManager::getInstance()->getAllMasterGroup();
		for (uint m = 0; m < groups.size(); ++m)
		{
			if (groups[m].Group == nullptr || !groups[m].Group->getActive())
				continue;
			for (uint p = WIN_PRIORITY_MAX; p > 0; --p)
			{
				const std::list<CInterfaceGroup *> &list = groups[m].PrioritizedWindows[p - 1];
				for (std::list<CInterfaceGroup *>::const_reverse_iterator it = list.rbegin(); it != list.rend(); ++it)
					walk.walk(*it, 0, nodes);
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
	CInterfaceElement *e = findElement(id);
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

/// Whether one element is the other or inside it
bool within(const CInterfaceElement *e, const CInterfaceElement *ancestor)
{
	for (; e != nullptr; e = e->getParent())
		if (e == ancestor)
			return true;
	return false;
}

/// What a press of the mouse at an interface point goes to, by the rule of CWidgetManager::handleMouseEvent: in the
/// window under the point, the deepest control that takes captures, the last drawn on a tie
struct CHit
{
	CInterfaceGroup *Window;
	CCtrlBase *Control;
	/// A modal window that keeps the press, or one the press closes first (exit_click_out)
	CInterfaceGroup *Modal;
	bool ClosesModal;

	CHit()
	    : Window(nullptr)
	    , Control(nullptr)
	    , Modal(nullptr)
	    , ClosesModal(false)
	{
	}
};

CHit hitAt(sint32 ux, sint32 uy)
{
	CWidgetManager *wm = CWidgetManager::getInstance();
	CHit hit;
	hit.Window = wm->getWindowUnder(ux, uy);
	if (wm->hasModal())
	{
		CWidgetManager::SModalWndInfo &modal = wm->getModal();
		CInterfaceGroup *modalWindow = modal.ModalWindow;
		if (modalWindow != nullptr && hit.Window != modalWindow)
		{
			hit.Modal = modalWindow;
			if (!modal.ModalExitClickOut)
			{
				hit.Window = nullptr;
				return hit;
			}
			// the press closes the modal window, then goes on to what is under the point
			hit.ClosesModal = true;
		}
	}
	// no control takes a press while a sheet is dragged
	if (hit.Window == nullptr || CCtrlDraggable::getDraggedSheet() != nullptr)
		return hit;
	std::vector<CCtrlBase *> ctrls;
	wm->getCtrlsUnder(ux, uy, ctrls);
	uint32 deepest = 0;
	for (sint i = (sint)ctrls.size() - 1; i >= 0; --i)
	{
		CCtrlBase *ctrl = ctrls[i];
		if (ctrl == nullptr || !ctrl->isCapturable() || !ctrl->isInGroup(hit.Window))
			continue;
		uint32 depth = ctrl->getDepth(hit.Window);
		if (depth > deepest)
		{
			deepest = depth;
			hit.Control = ctrl;
		}
	}
	return hit;
}

/// Whether a press goes to an element: to it or to a control inside it; for a view, also to the control that holds it
/// (the text of a button); for a group or a view under the point, also to its window when no control takes the press
bool reaches(const CHit &hit, CInterfaceElement *e, sint32 ux, sint32 uy)
{
	if (hit.Window == nullptr)
		return false;
	bool isControl = dynamic_cast<CCtrlBase *>(e) != nullptr;
	bool isGroup = dynamic_cast<CInterfaceGroup *>(e) != nullptr;
	if (hit.Control != nullptr)
		return within(hit.Control, e) || (!isControl && within(e, hit.Control));
	return (isGroup || !isControl) && within(e, hit.Window) && e->isIn(ux, uy);
}

/// Where the mouse acts on an element
struct CReach
{
	double X, Y;
	/// What takes the press there: a control, or the window
	std::string Target;
	/// The modal window the press closes first, exit_click_out
	std::string ClosesModal;
};

/// The point of an element where the mouse acts on it; false with the call failed when the press would not go to it
bool reachElement(CCtrlCall &call, const std::string &idArg, CReach &reach)
{
	const std::string &id = call.getArgs().get(idArg).getString();
	CInterfaceElement *e = findElement(id);
	if (e == nullptr)
	{
		call.fail(toString("no interface element '%s'", id.c_str()));
		return false;
	}
	if (!isShown(e))
	{
		call.fail(toString("'%s' is not shown (it or a parent is not active)", id.c_str()));
		return false;
	}
	if (!CWidgetManager::getInstance()->isMouseHandlingEnabled())
	{
		call.fail("the interface does not handle the mouse now");
		return false;
	}
	CUiScale scale;
	if (!scale.hasArea())
	{
		call.fail("the window has no area (minimized?)");
		return false;
	}
	double px, py;
	scale.centre(e, px, py);
	// an offset from the centre, in pixels, for an element whose centre is not where it takes clicks
	px += call.getArgs().get("dx").getNumber(0);
	py += call.getArgs().get("dy").getNumber(0);
	if (px < 0 || py < 0 || px >= scale.WindowWidth || py >= scale.WindowHeight)
	{
		call.fail(toString("the point of '%s' is outside the window", id.c_str()));
		return false;
	}
	sint32 ux, uy;
	scale.toUi(px, py, ux, uy);
	CHit hit = hitAt(ux, uy);
	if (!reaches(hit, e, ux, uy))
	{
		CJsonValue details = CJsonValue::object();
		details.set("x", px);
		details.set("y", py);
		if (hit.Window == nullptr && hit.Modal != nullptr)
		{
			details.set("modal", hit.Modal->getId());
			call.fail("a modal window takes the input: " + hit.Modal->getId() + "; close it first (ui.tree shows its buttons, ui.click presses one)", details);
			return false;
		}
		if (hit.Control != nullptr)
			details.set("takes_the_press", hit.Control->getId());
		else if (hit.Window != nullptr)
			details.set("takes_the_press", hit.Window->getId());
		call.fail(toString("the mouse would not reach '%s' at that point: %s takes the press there", id.c_str(),
		              hit.Control != nullptr ? "another control" : (hit.Window != nullptr ? "a window without a control there" : "nothing of the interface")),
		    details);
		return false;
	}
	if (const CCtrlBaseButton *button = dynamic_cast<const CCtrlBaseButton *>(hit.Control))
	{
		if (button->getFrozen())
		{
			call.fail(toString("'%s' is frozen: the button takes no click now", button->getId().c_str()), CJsonValue::object().set("frozen", true));
			return false;
		}
	}
	reach.X = px;
	reach.Y = py;
	reach.Target = hit.Control != nullptr ? hit.Control->getId() : hit.Window->getId();
	reach.ClosesModal = hit.ClosesModal ? hit.Modal->getId() : std::string();
	return true;
}

void setReach(CJsonValue &r, const CReach &reach)
{
	r.set("x", reach.X);
	r.set("y", reach.Y);
	r.set("target", reach.Target);
	if (!reach.ClosesModal.empty())
		r.set("closes_modal", reach.ClosesModal);
}

void methodAt(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	double px = call.getArgs().get("x").getNumber(), py = call.getArgs().get("y").getNumber();
	CUiScale scale;
	if (!scale.hasArea())
	{
		call.fail("the window has no area (minimized?)");
		return;
	}
	if (px < 0 || py < 0 || px >= scale.WindowWidth || py >= scale.WindowHeight)
	{
		call.fail(toString("%.0f, %.0f is outside the client area, which is %ux%u", px, py, scale.WindowWidth, scale.WindowHeight));
		return;
	}
	sint32 ux, uy;
	scale.toUi(px, py, ux, uy);
	std::vector<CCtrlBase *> ctrls;
	std::vector<CInterfaceGroup *> groups;
	CWidgetManager::getInstance()->getCtrlsUnder(ux, uy, ctrls);
	CWidgetManager::getInstance()->getGroupsUnder(ux, uy, groups);
	// the controls come in the order they are drawn: the front-most is the last
	CJsonValue c = CJsonValue::array();
	for (uint i = (uint)ctrls.size(); i > 0; --i)
		c.push(elementJson(ctrls[i - 1], scale));
	CJsonValue g = CJsonValue::array();
	for (uint i = 0; i < groups.size(); ++i)
		g.push(CJsonValue(groups[i]->getId()));
	CJsonValue r = CJsonValue::object();
	r.set("controls", c);
	r.set("groups", g);
	CHit hit = hitAt(ux, uy);
	if (hit.Control != nullptr)
		r.set("press_goes_to", hit.Control->getId());
	else if (hit.Window != nullptr)
		r.set("press_goes_to", hit.Window->getId());
	else
		r.set("press_goes_to", CJsonValue());
	if (hit.Modal != nullptr)
		r.set(hit.ClosesModal ? "closes_modal" : "modal", hit.Modal->getId());
	call.reply(r);
}

void methodClick(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CReach reach;
	if (!reachElement(call, "id", reach))
		return;
	std::string action = call.getArgs().get("double").getBool(false) ? "double_click" : "click";
	std::string error;
	if (!postMouse(action, reach.X, reach.Y, call.getArgs().get("button").getString("left"), call.getArgs(), 0, error))
	{
		discardInput();
		call.fail(error);
		return;
	}
	CJsonValue r = CJsonValue::object();
	r.set("id", call.getArgs().get("id"));
	setReach(r, reach);
	r.set("action", action);
	finishAfterPumpOnElement(call, r);
}

void methodHover(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CReach reach;
	if (!reachElement(call, "id", reach))
		return;
	std::string error;
	if (!postMouse("move", reach.X, reach.Y, "left", call.getArgs(), 0, error))
	{
		discardInput();
		call.fail(error);
		return;
	}
	CJsonValue r = CJsonValue::object();
	r.set("id", call.getArgs().get("id"));
	setReach(r, reach);
	finishAfterPump(call, r);
}

void methodWheel(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	sint64 steps = call.getArgs().get("steps").getInt(-1);
	if (steps == 0 || steps > 100 || steps < -100)
	{
		call.fail("steps is 1 to 100 up, or -1 to -100 down");
		return;
	}
	CReach reach;
	if (!reachElement(call, "id", reach))
		return;
	std::string error;
	if (!postMouse("wheel", reach.X, reach.Y, "left", CJsonValue::object(), (sint)steps, error))
	{
		discardInput();
		call.fail(error);
		return;
	}
	CJsonValue r = CJsonValue::object();
	r.set("id", call.getArgs().get("id"));
	setReach(r, reach);
	r.set("steps", (sint32)steps);
	finishAfterPumpOnElement(call, r);
}

void methodDrag(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	CReach from;
	if (!reachElement(call, "from", from))
		return;
	double tx, ty;
	CUiScale scale;
	if (call.getArgs().get("to").isString())
	{
		if (!call.getArgs().get("x").isNull() || !call.getArgs().get("y").isNull())
		{
			call.fail("to (an element id) or x and y (window pixels): not both");
			return;
		}
		// the target only needs to be where the drop lands; what covers it is the drop's business
		const std::string &to = call.getArgs().get("to").getString();
		CInterfaceElement *e = findElement(to);
		if (e == nullptr)
		{
			call.fail(toString("no interface element '%s'", to.c_str()));
			return;
		}
		if (!isShown(e))
		{
			call.fail(toString("'%s' is not shown (it or a parent is not active)", to.c_str()));
			return;
		}
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
	if (tx < 0 || ty < 0 || tx >= scale.WindowWidth || ty >= scale.WindowHeight)
	{
		call.fail(toString("the drop point %.0f, %.0f is outside the client area, which is %ux%u", tx, ty, scale.WindowWidth, scale.WindowHeight));
		return;
	}
	std::string error;
	// press, move in steps as a hand does (a drag starts after a few pixels), release; the modifiers stay down through it
	bool ok = postMouse("down", from.X, from.Y, "left", call.getArgs(), 0, error);
	for (uint i = 1; i <= 8 && ok; ++i)
		ok = postMouse("move_held", from.X + (tx - from.X) * i / 8.0, from.Y + (ty - from.Y) * i / 8.0, "left", call.getArgs(), 0, error);
	if (ok)
		ok = postMouse("up", tx, ty, "left", call.getArgs(), 0, error);
	if (!ok)
	{
		discardInput();
		call.fail(error);
		return;
	}
	CJsonValue r = CJsonValue::object();
	r.set("from", call.getArgs().get("from"));
	r.set("target", from.Target);
	if (!from.ClosesModal.empty())
		r.set("closes_modal", from.ClosesModal);
	r.set("x", tx);
	r.set("y", ty);
	finishAfterPumpOnElement(call, r);
}

void methodSetText(CCtrlCall &call)
{
	if (!requireInterface(call))
		return;
	const std::string &id = call.getArgs().get("id").getString();
	CGroupEditBox *edit = dynamic_cast<CGroupEditBox *>(findElement(id));
	if (edit == nullptr)
	{
		call.fail(toString("'%s' is not an edit box (the id of a CGroupEditBox, often ending in :eb)", id.c_str()));
		return;
	}
	// the box as a player reaches it: shown, under the mouse, not behind a modal window
	CReach reach;
	if (!reachElement(call, "id", reach))
		return;
	if (edit->getFrozen())
	{
		call.fail(toString("'%s' is frozen: it takes no text now", id.c_str()));
		return;
	}
	const std::string &text = call.getArgs().get("text").getString();
	// as a paste: the box keeps what its entry type, max_num_chars and max_num_bytes let through, then its change
	// handler runs, as for a typed character
	if (CGroupEditBoxBase::getCurrSelection() == edit)
		CGroupEditBoxBase::disableSelection();
	edit->setInputString(std::string());
	edit->writeString(text, true, true);
	std::string onChange = edit->getProperty("onchange");
	if (!onChange.empty())
		CAHManager::getInstance()->runActionHandler(onChange, edit, edit->getProperty("onchange_params"));
	CJsonValue r = CJsonValue::object();
	r.set("id", id);
	setInput(r, edit);
	if (edit->getInputString() != text)
		r.set("filtered", "the box did not take all of the text: its entry type or its length limit left some out");
	if (!call.getArgs().get("submit").getBool(false))
	{
		r.set("submitted", false);
		call.reply(r);
		return;
	}
	// Enter goes to the box that has the keyboard focus, which a click on it gives
	CWidgetManager *wm = CWidgetManager::getInstance();
	if (wm->getCaptureKeyboard() != edit)
		wm->setCaptureKeyboard(edit);
	if (wm->getCaptureKeyboard() != edit)
	{
		call.fail(toString("'%s' did not take the keyboard focus: nothing was submitted", id.c_str()), r);
		return;
	}
	// a top window with an Enter handler of its own takes Enter, unless the box is in it (CWidgetManager::handleKeyboardEvent)
	CInterfaceGroup *top = wm->getTopWindow();
	if (top != nullptr && top->getActive() && !top->getAHOnEnter().empty() && edit->getRootWindow() != top)
	{
		call.fail(toString("Enter would go to the handler of the top window '%s', not to the box: nothing was submitted", top->getId().c_str()), r);
		return;
	}
	std::string error;
	if (!postKey("KeyRETURN", error))
	{
		discardInput();
		call.fail(error);
		return;
	}
	r.set("submitted", true);
	finishAfterPump(call, r);
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
	// four looks a second: a window that opens is known within a quarter of a second, without a walk at every frame
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
	endpoint.addMethod(newMethod("ui.windows",
	    "The open windows, front to back per master group (ui:interface in the game, ui:login and ui:outgame before), with their id, type, rectangle in window pixels and title; "
	    "the modal window, which takes all input while open; and what has the keyboard focus. all includes the hidden ones. A window that opens or closes, and the modal window, are also window events: get_events with kinds window and wait_seconds waits for one.",
	    CCtrlSchema().boolean("all", "Also the windows that are not active."), true, methodWindows, nullptr, nullptr));
	endpoint.addMethod(newMethod("ui.tree",
	    "The element tree of a window or element (id), or of every open window: id, type, rectangle in window pixels, active, text, edit box input (only the length of a password), "
	    "tooltip, the action handler of a button, frozen. depth bounds it below the root, or below each window (4 by default), contains keeps the elements whose id or text "
	    "has the word and the path to them, max caps it (truncated says when).",
	    CCtrlSchema()
	        .str("id", "The root element, ui:interface:... (every open window by default).")
	        .integer("depth", "How many levels below the root or each window, 0 to 64 (4 by default).")
	        .str("contains", "Only the elements whose id or text contains this (case-insensitive), with their parents.")
	        .boolean("active_only", "Only the active elements (the default).")
	        .integer("max", "At most this many elements, 1 to 5000 (400 by default)."),
	    true, methodTree, nullptr, nullptr));
	endpoint.addMethod(newMethod("ui.get",
	    "One element: what ui.tree gives for it, its parent, and the properties asked for (any XML property of its type: text, onclick_l, x, posref, color...); a name its type "
	    "does not have gives an empty value and a warning in the log.",
	    CCtrlSchema().str("id", "The element.", true).strings("properties", "The property names."), true, methodGet, nullptr, nullptr));
	endpoint.addMethod(newMethod("ui.at",
	    "What the mouse would find at a window pixel: the controls there, front-most first, the groups, outermost first, and what a press there goes to (press_goes_to), as "
	    "ui.click checks it.",
	    CCtrlSchema().number("x", "Window pixels from the left.", true).number("y", "Window pixels from the top.", true), true, methodAt, nullptr, nullptr));
	endpoint.addMethod(newMethod("ui.click",
	    "Clicks an element by id with the mouse, at its centre (dx, dy move the point), after checking that the press there goes to it as the interface gives it (to the deepest "
	    "control that takes clicks in the window under the point); it fails with what takes the press otherwise, and for a frozen button. target says what takes it; closes_modal "
	    "a modal window that a click outside closes first. Answers once the client has handled the click; it fails when the interface did not take it (an interface animation "
	    "holds the buttons while a screen changes). Input sent just before a loading screen is dropped there, and the call fails.",
	    CCtrlSchema()
	        .str("id", "The element.", true)
	        .choice("button", "left|right", "The button (left by default); the middle one goes to the game: input.mouse.")
	        .boolean("double", "A double click.")
	        .number("dx", "Pixels right of the centre.")
	        .number("dy", "Pixels below the centre.")
	        .boolean("ctrl", "With Ctrl held.")
	        .boolean("shift", "With Shift held.")
	        .boolean("alt", "With Alt held."),
	    false, methodClick, pollAfterPump, nullptr));
	endpoint.addMethod(newMethod("ui.hover", "Moves the mouse over an element, for its tooltip or its hover state, after the same check as ui.click.",
	    CCtrlSchema().str("id", "The element.", true).number("dx", "Pixels right of the centre.").number("dy", "Pixels below the centre."), false, methodHover, pollAfterPump, nullptr));
	endpoint.addMethod(newMethod("ui.wheel", "Turns the mouse wheel over an element, after the same check as ui.click: a list or a scroll area scrolls.",
	    CCtrlSchema().str("id", "The element.", true).integer("steps", "Notches, 1 to 100 up, -1 to -100 down (-1 by default)."), false, methodWheel, pollAfterPump, nullptr));
	endpoint.addMethod(newMethod("ui.drag",
	    "Drags an element with the left button to another element (to) or to a window pixel (x, y), as a hand does: press, move in steps, release, with ctrl, shift and alt "
	    "held through it (ctrl drops one of a stack, shift all of it). The press goes through the check of ui.click. For items to slots and actions to the bar.",
	    CCtrlSchema()
	        .str("from", "The element to drag.", true)
	        .str("to", "The element to drop on.")
	        .number("x", "Or the window pixel to drop at.")
	        .number("y", "Its y.")
	        .boolean("ctrl", "With Ctrl held.")
	        .boolean("shift", "With Shift held.")
	        .boolean("alt", "With Alt held."),
	    false, methodDrag, pollAfterPump, nullptr));
	endpoint.addMethod(newMethod("ui.set_text",
	    "Sets the text of an edit box as a paste does, not key by key: the box keeps what its entry type and length limits let through (filtered says when it left some out), "
	    "and its change handler runs. The box must be shown and reachable as for ui.click, and not frozen. With submit, the box gets the keyboard focus as a click gives it and "
	    "Enter is pressed in it; submitted says so, and the call answers once the client has handled Enter. A password box gives back only its length.",
	    CCtrlSchema().str("id", "The edit box (often ...:eb).", true).str("text", "The text; empty clears the box.", true).boolean("submit", "Press Enter in it after."), false,
	    methodSetText, pollAfterPump, nullptr));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_ui.cpp */
