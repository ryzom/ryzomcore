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

// The raw input methods of the control endpoint: input.key, input.text, input.mouse. The events go to
// the event server of the driver, as the window's own events do, so neither the focus of the window
// nor the operating system is involved, and the modifier keys are part of each event. A key or button
// a connection leaves held is let go of when that connection closes.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/misc/events.h"
#include "nel/misc/utf_string_view.h"

#include "global.h"

#include <set>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

/// What a connection holds down
struct CHeld
{
	std::set<TKey> Keys;
	uint Buttons;
	float X, Y;

	CHeld()
	    : Buttons(0)
	    , X(0.5f)
	    , Y(0.5f)
	{
	}
};

std::map<uint32, CHeld> s_Held;

CHeld &heldOfCaller()
{
	CCtrlEndpoint *endpoint = getEndpoint();
	return s_Held[endpoint != nullptr ? endpoint->getCallingConnection() : 0];
}

/// A connection closed: what it held is released, so the character does not keep running
void onConnectionClosed(uint32 connection)
{
	std::map<uint32, CHeld>::iterator it = s_Held.find(connection);
	if (it == s_Held.end())
		return;
	CHeld &held = it->second;
	for (std::set<TKey>::iterator k = held.Keys.begin(); k != held.Keys.end(); ++k)
		Driver->EventServer.postEvent(new CEventKeyUp(*k, noKeyButton, nullptr));
	static const TMouseButton s_Buttons[] = { leftButton, middleButton, rightButton };
	for (uint i = 0; i < 3; ++i)
		if (held.Buttons & s_Buttons[i])
			Driver->EventServer.postEvent(new CEventMouseUp(held.X, held.Y, s_Buttons[i], nullptr));
	if (!held.Keys.empty() || held.Buttons != 0)
		nlinfo("CTRL: released the keys and buttons connection %u left held", connection);
	s_Held.erase(it);
}

TKeyButton keyModifiers(const CJsonValue &args)
{
	uint m = noKeyButton;
	if (args.get("ctrl").getBool())
		m |= ctrlKeyButton;
	if (args.get("shift").getBool())
		m |= shiftKeyButton;
	if (args.get("alt").getBool())
		m |= altKeyButton;
	return (TKeyButton)m;
}

uint mouseModifiers(const CJsonValue &args)
{
	uint m = noButton;
	if (args.get("ctrl").getBool())
		m |= ctrlButton;
	if (args.get("shift").getBool())
		m |= shiftButton;
	if (args.get("alt").getBool())
		m |= altButton;
	return m;
}

bool parseKey(const std::string &name, TKey &key)
{
	key = CEventKey::getKeyFromString(name);
	if (key == KeyCount)
		key = CEventKey::getKeyFromString("Key" + name);
	return key != KeyCount;
}

bool mouseButton(const std::string &name, TMouseButton &button)
{
	if (name.empty() || name == "left")
		button = leftButton;
	else if (name == "right")
		button = rightButton;
	else if (name == "middle")
		button = middleButton;
	else
		return false;
	return true;
}

/// The input reaches the interface at the next pump: the call answers at the next update, which comes after it
class CPendingResult : public ICtrlCallData
{
public:
	CJsonValue Result;
};

void methodKey(CCtrlCall &call)
{
	const CJsonValue &args = call.getArgs();
	TKey key;
	if (!parseKey(args.get("key").getString(), key))
	{
		call.fail(toString("no key '%s': the names are those of NeL, KeyA, KeyRETURN, KeyESCAPE, KeyF1, KeyUP...", args.get("key").getString().c_str()));
		return;
	}
	std::string action = args.get("action").getString("press");
	if (action != "press" && action != "down" && action != "up")
	{
		call.fail("action is press, down or up");
		return;
	}
	TKeyButton modifiers = keyModifiers(args);
	uint repeats = (uint)std::max((sint64)0, std::min((sint64)100, args.get("repeat").getInt(0)));
	CHeld &held = heldOfCaller();
	if (action == "press" || action == "down")
	{
		Driver->EventServer.postEvent(new CEventKeyDown(key, modifiers, true, nullptr));
		for (uint i = 0; i < repeats; ++i)
			Driver->EventServer.postEvent(new CEventKeyDown(key, modifiers, false, nullptr));
		if (action == "down")
			held.Keys.insert(key);
	}
	if (action == "press" || action == "up")
	{
		Driver->EventServer.postEvent(new CEventKeyUp(key, modifiers, nullptr));
		held.Keys.erase(key);
	}
	CJsonValue r = CJsonValue::object();
	r.set("key", CEventKey::getStringFromKey(key));
	r.set("action", action);
	if (repeats > 0)
		r.set("repeats", repeats);
	finishAfterPump(call, r);
}

void methodText(CCtrlCall &call)
{
	const std::string &text = call.getArgs().get("text").getString();
	TKeyButton modifiers = keyModifiers(call.getArgs());
	uint count = 0;
	CUtfStringView view(text);
	for (CUtfStringView::iterator it = view.begin(); it != view.end(); ++it)
	{
		Driver->EventServer.postEvent(new CEventChar(*it, modifiers, nullptr));
		++count;
	}
	finishAfterPump(call, CJsonValue::object().set("characters", count));
}

void methodMouse(CCtrlCall &call)
{
	const CJsonValue &args = call.getArgs();
	std::string action = args.get("action").getString("click");
	if (!args.get("x").isNumber() || !args.get("y").isNumber())
	{
		call.fail("x and y are required: pixels of the client area from its top left, as a screen grab has them");
		return;
	}
	double px = args.get("x").getNumber();
	double py = args.get("y").getNumber();
	std::string error;
	if (!postMouse(action, px, py, args.get("button").getString(), args, (sint)args.get("steps").getInt(1), error))
	{
		call.fail(error);
		return;
	}
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	CJsonValue r = CJsonValue::object();
	r.set("action", action);
	r.set("x", px);
	r.set("y", py);
	r.set("window", CJsonValue::object().set("width", width).set("height", height));
	finishAfterPump(call, r);
}

} // anonymous namespace

bool postMouse(const std::string &action, double px, double py, const std::string &buttonName, const CJsonValue &modifierArgs, sint steps, std::string &error)
{
	TMouseButton button;
	if (!mouseButton(buttonName, button))
	{
		error = "button is left, right or middle";
		return false;
	}
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	if (width == 0 || height == 0)
	{
		error = "the window has no area (minimized?)";
		return false;
	}
	if (px < 0 || py < 0 || px >= width || py >= height)
	{
		error = toString("%.0f, %.0f is outside the client area, which is %ux%u", px, py, width, height);
		return false;
	}
	// NeL's mouse events are in 0..1 from the bottom left, as the window's own emitter makes them
	float x = (float)(px / (double)width);
	float y = (float)(1.0 - py / (double)height);
	uint modifiers = mouseModifiers(modifierArgs);
	CHeld &held = heldOfCaller();
	TMouseButton moveButtons = (TMouseButton)(modifiers | held.Buttons);
	TMouseButton withButton = (TMouseButton)(modifiers | button);
	CEventServer &server = Driver->EventServer;
	held.X = x;
	held.Y = y;

	if (action == "move" || action == "move_held")
	{
		server.postEvent(new CEventMouseMove(x, y, action == "move_held" ? (TMouseButton)(moveButtons | button) : moveButtons, nullptr));
	}
	else if (action == "down")
	{
		server.postEvent(new CEventMouseMove(x, y, moveButtons, nullptr));
		server.postEvent(new CEventMouseDown(x, y, withButton, nullptr));
		held.Buttons |= button;
	}
	else if (action == "up")
	{
		server.postEvent(new CEventMouseMove(x, y, moveButtons, nullptr));
		server.postEvent(new CEventMouseUp(x, y, withButton, nullptr));
		held.Buttons &= ~(uint)button;
	}
	else if (action == "click")
	{
		server.postEvent(new CEventMouseMove(x, y, moveButtons, nullptr));
		server.postEvent(new CEventMouseDown(x, y, withButton, nullptr));
		server.postEvent(new CEventMouseUp(x, y, withButton, nullptr));
	}
	else if (action == "double_click")
	{
		// as the window system reports one: down, up, double click, up
		server.postEvent(new CEventMouseMove(x, y, moveButtons, nullptr));
		server.postEvent(new CEventMouseDown(x, y, withButton, nullptr));
		server.postEvent(new CEventMouseUp(x, y, withButton, nullptr));
		server.postEvent(new CEventMouseDblClk(x, y, withButton, nullptr));
		server.postEvent(new CEventMouseUp(x, y, withButton, nullptr));
	}
	else if (action == "wheel")
	{
		if (steps == 0)
		{
			error = "steps is not 0: positive turns the wheel up, negative down";
			return false;
		}
		server.postEvent(new CEventMouseMove(x, y, moveButtons, nullptr));
		for (sint i = 0; i < std::abs(steps); ++i)
			server.postEvent(new CEventMouseWheel(x, y, moveButtons, steps > 0, nullptr));
	}
	else
	{
		error = "action is move, down, up, click, double_click or wheel";
		return false;
	}
	return true;
}

bool postKey(const std::string &name, std::string &error)
{
	TKey key;
	if (!parseKey(name, key))
	{
		error = "no key " + name;
		return false;
	}
	Driver->EventServer.postEvent(new CEventKeyDown(key, noKeyButton, true, nullptr));
	Driver->EventServer.postEvent(new CEventKeyUp(key, noKeyButton, nullptr));
	return true;
}

void finishAfterPump(CCtrlCall &call, const CJsonValue &result)
{
	CPendingResult *pending = new CPendingResult();
	pending->Result = result;
	call.setData(pending);
}

void pollAfterPump(CCtrlCall &call)
{
	CPendingResult *pending = static_cast<CPendingResult *>(call.getData());
	call.reply(pending != nullptr ? pending->Result : CJsonValue::object());
}

void registerInputMethods(CCtrlEndpoint &endpoint)
{
	endpoint.setConnectionClosedCallback(onConnectionClosed);
	endpoint.addMethod(new CCtrlFunctionMethod("input.key",
	    "Presses, holds or releases a key, as the keyboard does, with modifiers in the event (so Ctrl and Alt work). The names are NeL's: KeyA, KeyRETURN, KeyESCAPE, KeyF1, KeyUP, KeyDELETE... "
	    "It reaches the game as well as the interface, as a real key does: a movement key moves the character. A key held down is released when the connection closes. Answers once the client has handled it.",
	    CCtrlSchema()
	        .str("key", "The key: KeyA or A, KeyRETURN, KeyESCAPE, KeyBACK, KeyTAB, KeyF1...", true)
	        .choice("action", "press|down|up", "press (down and up, the default), down, or up.")
	        .integer("repeat", "With press or down, this many key repeats after the first down.")
	        .boolean("ctrl", "With Ctrl.")
	        .boolean("shift", "With Shift.")
	        .boolean("alt", "With Alt."),
	    false, methodKey, pollAfterPump));
	endpoint.addMethod(new CCtrlFunctionMethod("input.text",
	    "Types text into whatever has the keyboard focus, as character events (no key presses). Answers once the client has handled it.", CCtrlSchema().str("text", "The text.", true),
	    false, methodText, pollAfterPump));
	endpoint.addMethod(new CCtrlFunctionMethod("input.mouse",
	    "Moves the mouse to x,y and clicks, presses, releases or turns the wheel there, as the mouse does: the interface finds what is under the pointer. "
	    "x and y are pixels of the client area from its top left, as a screen grab has them. Free look (the camera following the mouse) moves the system cursor and is not covered: the camera methods set the view. "
	    "A button held down is released when the connection closes. Answers once the client has handled it.",
	    CCtrlSchema()
	        .choice("action", "click|double_click|down|up|move|wheel", "What to do (click by default).")
	        .number("x", "Pixels from the left of the client area.", true)
	        .number("y", "Pixels from the top of the client area.", true)
	        .choice("button", "left|right|middle", "The button (left by default).")
	        .integer("steps", "For wheel: the notches, positive up, negative down.")
	        .boolean("ctrl", "With Ctrl held.")
	        .boolean("shift", "With Shift held.")
	        .boolean("alt", "With Alt held."),
	    false, methodMouse, pollAfterPump));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_input.cpp */
