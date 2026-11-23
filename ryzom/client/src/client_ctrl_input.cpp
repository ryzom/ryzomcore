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

// The raw input methods of the control endpoint: input.key, input.text, input.mouse, and the posting of what the ui
// methods send through the mouse and the keyboard. The events go to the event server of the driver, as the window's
// own events do, so neither the focus of the window nor the operating system is involved.
//
// The input of a call waits in a queue of its own and is posted at the start of the next pump that the interface
// handles (CInputHandlerManager::pumpEvents), never in the pump of a loading screen (pumpEventsNoIM): a loading that
// comes first drops it, and its call fails, so it never acts on the screen that follows the loading. The call answers
// after the pump that delivered it, with whether the interface took it. A release is never dropped: it waits for the
// next pump that has the interface, so nothing stays held.
//
// The modifiers of a call are pressed before its input and let go of after it, as on a keyboard, since the client
// reads them from the keys (the async listener) and not only from the bits of the events. A key or button a connection
// leaves held is let go of when that connection closes, gives the lease back, or another one drives.
//
// This file also has the helpers the method files of the interface share: newMethod, the method whose arguments are
// checked against its schema.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/3d/u_driver.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/common.h"
#include "nel/misc/events.h"
#include "nel/misc/utf_string_view.h"

#include "global.h"
#include "input.h"

#include <cmath>
#include <list>
#include <set>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

namespace {

const uint s_MaxRepeats = 100;
const sint s_MaxWheelSteps = 100;
const uint s_MaxTextLength = 4096;

// *** the arguments ***

/// Whether a value is of the type of its schema property; expected says what it should have been
bool argOfType(const CJsonValue &value, const CJsonValue &property, std::string &expected)
{
	const std::string &type = property.get("type").getString();
	if (type == "string")
	{
		if (!value.isString())
		{
			expected = "a string";
			return false;
		}
		const CJsonValue &choices = property.get("enum");
		if (!choices.isArray())
			return true;
		for (uint i = 0; i < choices.size(); ++i)
			if (choices.at(i).getString() == value.getString())
				return true;
		expected = "one of ";
		for (uint i = 0; i < choices.size(); ++i)
			expected += (i > 0 ? ", " : "") + choices.at(i).getString();
		return false;
	}
	if (type == "number" || type == "integer")
	{
		if (!value.isNumber() || !isValidDouble(value.getNumber()))
		{
			expected = "a finite number";
			return false;
		}
		// integers are exact in a double up to 2^53
		if (type == "integer" && (value.getNumber() != std::floor(value.getNumber()) || std::fabs(value.getNumber()) > 9007199254740992.0))
		{
			expected = "an integer";
			return false;
		}
		return true;
	}
	if (type == "boolean")
	{
		if (!value.isBool())
		{
			expected = "true or false";
			return false;
		}
		return true;
	}
	if (type == "object")
	{
		if (!value.isObject())
		{
			expected = "an object";
			return false;
		}
		return true;
	}
	if (type == "array")
	{
		if (!value.isArray())
		{
			expected = "an array";
			return false;
		}
		const CJsonValue &items = property.get("items");
		if (!items.isObject())
			return true;
		for (uint i = 0; i < value.size(); ++i)
		{
			std::string item;
			if (!argOfType(value.at(i), items, item))
			{
				expected = toString("an array whose items are %s (item %u is not)", item.c_str(), i);
				return false;
			}
		}
		return true;
	}
	// no type: anything
	return true;
}

/// Fails the call and returns false when its arguments do not fit the schema of its method
bool checkArgs(CCtrlCall &call, const CJsonValue &schema)
{
	const CJsonValue &args = call.getArgs();
	if (!args.isNull() && !args.isObject())
	{
		call.fail("the arguments are a JSON object");
		return false;
	}
	const CJsonValue &properties = schema.get("properties");
	const CJsonValue::TMembers &members = args.members();
	for (uint i = 0; i < members.size(); ++i)
	{
		const std::string &name = members[i].first;
		const CJsonValue &property = properties.get(name);
		if (!property.isObject())
		{
			std::string known;
			for (uint k = 0; k < properties.members().size(); ++k)
				known += (k > 0 ? ", " : "") + properties.members()[k].first;
			call.fail(toString("%s has no argument '%s'; its arguments are: %s", call.getMethod().c_str(), name.c_str(), known.empty() ? "none" : known.c_str()));
			return false;
		}
		// null stands for an argument not given
		if (members[i].second.isNull())
			continue;
		std::string expected;
		if (!argOfType(members[i].second, property, expected))
		{
			call.fail(toString("'%s' is %s", name.c_str(), expected.c_str()));
			return false;
		}
	}
	const CJsonValue &required = schema.get("required");
	for (uint i = 0; i < required.size(); ++i)
	{
		if (args.get(required.at(i).getString()).isNull())
		{
			call.fail(toString("'%s' is required", required.at(i).getString().c_str()));
			return false;
		}
	}
	return true;
}

class CCheckedMethod : public CCtrlFunctionMethod
{
public:
	CCheckedMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly, TFunction callFunction, TFunction pollFunction,
	    TFunction cancelFunction)
	    : CCtrlFunctionMethod(name, description, schema, readOnly, callFunction, pollFunction, cancelFunction)
	{
	}

	virtual void call(CCtrlCall &call) NL_OVERRIDE
	{
		if (checkArgs(call, getSchema()))
			CCtrlFunctionMethod::call(call);
	}
};

// *** the input ***

/// What a connection holds down, as the input it sent leaves it
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

	bool empty() const { return Keys.empty() && Buttons == 0; }
};

std::map<uint32, CHeld> s_Held;

/// The modifier keys of a call
struct CModifiers
{
	bool Ctrl, Shift, Alt;

	CModifiers()
	    : Ctrl(false)
	    , Shift(false)
	    , Alt(false)
	{
	}
};

class CPendingInput;

/// The input of one call, or the releases of a connection, until it is posted
struct CInputBatch
{
	uint32 Connection;
	std::vector<CEvent *> Events;
	/// It presses something (a key, a button, a move, a character); one that only lets go is never dropped
	bool Presses;
	/// What it presses that its connection counts as held from then on, given back when it is dropped
	std::vector<TKey> KeysDown;
	uint ButtonsDown;
	/// The modifier keys it pressed for its call, let go of at its end
	CModifiers Modifiers;
	/// The event whose handling the call reports
	CEvent *Main;
	CPendingInput *Pending;

	CInputBatch()
	    : Connection(0)
	    , Presses(false)
	    , ButtonsDown(0)
	    , Main(nullptr)
	    , Pending(nullptr)
	{
	}
};

// the input waiting for a pump that has the interface, in the order it came; a list, as calls point into it
std::list<CInputBatch> s_Queue;
// the input of the call being run, until it is finished or discarded
CInputBatch s_Open;

/// What a call that sent input waits for
class CPendingInput : public ICtrlCallData
{
public:
	enum TState
	{
		Queued,
		Posted,
		Dropped
	};

	CJsonValue Result;
	TState State;
	/// Input aimed at an element: the call fails when the interface did not take its press
	bool ForElement;
	bool HasMain;
	bool MainHandled;
	/// The batch, while it is queued
	CInputBatch *Batch;

	CPendingInput();
	~CPendingInput() NL_OVERRIDE;
};

// the main events posted in the current pump, by the call that waits to hear how they were handled
std::map<const CEvent *, CPendingInput *> s_Posted;

CPendingInput::CPendingInput()
    : State(Queued)
    , ForElement(false)
    , HasMain(false)
    , MainHandled(false)
    , Batch(nullptr)
{
}

CPendingInput::~CPendingInput()
{
	// the call is answered or dropped: its input, if it still waits, goes on without it
	if (Batch != nullptr)
		Batch->Pending = nullptr;
	for (std::map<const CEvent *, CPendingInput *>::iterator it = s_Posted.begin(); it != s_Posted.end(); ++it)
	{
		if (it->second == this)
		{
			s_Posted.erase(it);
			break;
		}
	}
}

void addInput(CEvent *event)
{
	s_Open.Events.push_back(event);
}

/// Lets go of what a connection holds, as a batch of releases behind the input already queued
void queueReleases(uint32 connection, CHeld &held)
{
	if (held.empty())
		return;
	CInputBatch batch;
	batch.Connection = connection;
	for (std::set<TKey>::iterator k = held.Keys.begin(); k != held.Keys.end(); ++k)
		batch.Events.push_back(new CEventKeyUp(*k, noKeyButton, nullptr));
	static const TMouseButton s_Buttons[] = { leftButton, middleButton, rightButton };
	for (uint i = 0; i < 3; ++i)
		if (held.Buttons & s_Buttons[i])
			batch.Events.push_back(new CEventMouseUp(held.X, held.Y, s_Buttons[i], nullptr));
	held.Keys.clear();
	held.Buttons = 0;
	s_Queue.push_back(batch);
}

/// What the calling connection holds; what other connections still hold is let go of first, since the lease has gone
/// from them to this one
CHeld &heldOfCaller()
{
	CCtrlEndpoint *endpoint = getEndpoint();
	uint32 connection = endpoint != nullptr ? endpoint->getCallingConnection() : 0;
	for (std::map<uint32, CHeld>::iterator it = s_Held.begin(); it != s_Held.end(); ++it)
	{
		if (it->first != connection && !it->second.empty())
		{
			nlinfo("CTRL: releasing the keys and buttons connection %u left held, as connection %u drives now", it->first, connection);
			queueReleases(it->first, it->second);
		}
	}
	s_Open.Connection = connection;
	return s_Held[connection];
}

/// A connection closed: what it held is let go of, so the character does not keep running, and what it watches goes
void onConnectionClosed(uint32 connection)
{
	dataConnectionClosed(connection);
	std::map<uint32, CHeld>::iterator it = s_Held.find(connection);
	if (it == s_Held.end())
		return;
	if (!it->second.empty())
	{
		nlinfo("CTRL: releasing the keys and buttons connection %u left held", connection);
		queueReleases(connection, it->second);
	}
	s_Held.erase(it);
}

CModifiers modifiersOf(const CJsonValue &args)
{
	CModifiers m;
	m.Ctrl = args.get("ctrl").getBool(false);
	m.Shift = args.get("shift").getBool(false);
	m.Alt = args.get("alt").getBool(false);
	return m;
}

/// The modifiers down while the input of the call is handled: those of the call, and those the connection holds
CModifiers modifiersDown(const CModifiers &m, const CHeld &held)
{
	CModifiers down = m;
	down.Ctrl = down.Ctrl || s_Open.Modifiers.Ctrl || held.Keys.count(KeyCONTROL) || held.Keys.count(KeyLCONTROL) || held.Keys.count(KeyRCONTROL);
	down.Shift = down.Shift || s_Open.Modifiers.Shift || held.Keys.count(KeySHIFT) || held.Keys.count(KeyLSHIFT) || held.Keys.count(KeyRSHIFT);
	down.Alt = down.Alt || s_Open.Modifiers.Alt || held.Keys.count(KeyMENU) || held.Keys.count(KeyLMENU) || held.Keys.count(KeyRMENU);
	return down;
}

TKeyButton keyButtons(const CModifiers &m)
{
	uint bits = noKeyButton;
	if (m.Ctrl)
		bits |= ctrlKeyButton;
	if (m.Shift)
		bits |= shiftKeyButton;
	if (m.Alt)
		bits |= altKeyButton;
	return (TKeyButton)bits;
}

uint mouseButtons(const CModifiers &m)
{
	uint bits = noButton;
	if (m.Ctrl)
		bits |= ctrlButton;
	if (m.Shift)
		bits |= shiftButton;
	if (m.Alt)
		bits |= altButton;
	return bits;
}

/// Presses the modifier keys of the call that are not down yet, each with the bits of those down with it, as the
/// keyboard reports them; endInput lets go of them
void pressModifiers(const CModifiers &m, const CHeld &held)
{
	CModifiers down = modifiersDown(CModifiers(), held);
	if (m.Ctrl && !down.Ctrl)
	{
		down.Ctrl = s_Open.Modifiers.Ctrl = true;
		addInput(new CEventKeyDown(KeyCONTROL, keyButtons(down), true, nullptr));
	}
	if (m.Shift && !down.Shift)
	{
		down.Shift = s_Open.Modifiers.Shift = true;
		addInput(new CEventKeyDown(KeySHIFT, keyButtons(down), true, nullptr));
	}
	if (m.Alt && !down.Alt)
	{
		down.Alt = s_Open.Modifiers.Alt = true;
		addInput(new CEventKeyDown(KeyMENU, keyButtons(down), true, nullptr));
	}
}

/// Lets go of the modifier keys the call pressed, in the reverse order
void releaseModifiers()
{
	CModifiers down = s_Open.Modifiers;
	if (down.Alt)
	{
		down.Alt = false;
		addInput(new CEventKeyUp(KeyMENU, keyButtons(down), nullptr));
	}
	if (down.Shift)
	{
		down.Shift = false;
		addInput(new CEventKeyUp(KeySHIFT, keyButtons(down), nullptr));
	}
	if (down.Ctrl)
	{
		down.Ctrl = false;
		addInput(new CEventKeyUp(KeyCONTROL, keyButtons(down), nullptr));
	}
	s_Open.Modifiers = CModifiers();
}

/// The character a keyboard gives with the press of a key that types none of its own (Windows and X both give these
/// four), 0 for none: an edit box validates on Enter and lets go of the focus on Escape through it
u32char keyCharacter(TKey key, const CModifiers &down)
{
	if (down.Ctrl || down.Alt)
		return 0;
	switch (key)
	{
	case KeyBACK: return 8;
	case KeyTAB: return 9;
	case KeyRETURN: return 13;
	case KeyESCAPE: return 27;
	default: return 0;
	}
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

/// A key press, held key or release, with its character, into the input of the call
void addKey(TKey key, bool press, bool release, uint repeats, CHeld &held)
{
	CModifiers down = modifiersDown(CModifiers(), held);
	TKeyButton bits = keyButtons(down);
	u32char c = keyCharacter(key, down);
	if (press)
	{
		s_Open.Presses = true;
		bool first = held.Keys.find(key) == held.Keys.end();
		CEventKeyDown *event = new CEventKeyDown(key, bits, first, nullptr);
		addInput(event);
		if (s_Open.Main == nullptr)
			s_Open.Main = event;
		if (c != 0)
			addInput(new CEventChar(c, bits, nullptr));
		for (uint i = 0; i < repeats; ++i)
		{
			addInput(new CEventKeyDown(key, bits, false, nullptr));
			if (c != 0)
				addInput(new CEventChar(c, bits, nullptr));
		}
		if (!release && held.Keys.insert(key).second)
			s_Open.KeysDown.push_back(key);
	}
	if (release)
	{
		CEventKeyUp *event = new CEventKeyUp(key, bits, nullptr);
		addInput(event);
		if (s_Open.Main == nullptr)
			s_Open.Main = event;
		held.Keys.erase(key);
	}
}

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
	sint64 repeat = args.get("repeat").getInt(0);
	if (repeat < 0 || repeat > (sint64)s_MaxRepeats)
	{
		call.fail(toString("repeat is 0 to %u", s_MaxRepeats));
		return;
	}
	if (repeat > 0 && action == "up")
	{
		call.fail("repeat goes with press or down: a release has no repeats");
		return;
	}
	CHeld &held = heldOfCaller();
	pressModifiers(modifiersOf(args), held);
	addKey(key, action != "up", action != "down", (uint)repeat, held);
	CJsonValue r = CJsonValue::object();
	r.set("key", CEventKey::getStringFromKey(key));
	r.set("action", action);
	if (repeat > 0)
		r.set("repeats", (uint32)repeat);
	finishAfterPump(call, r);
}

/// Whether input of the calling connection waits to be posted: what it does may give an edit box the focus
bool inputWaits()
{
	CCtrlEndpoint *endpoint = getEndpoint();
	uint32 connection = endpoint != nullptr ? endpoint->getCallingConnection() : 0;
	for (std::list<CInputBatch>::const_iterator it = s_Queue.begin(); it != s_Queue.end(); ++it)
		if (it->Connection == connection && it->Presses)
			return true;
	return false;
}

void methodText(CCtrlCall &call)
{
	::u32string text = CUtfStringView(call.getArgs().get("text").getString()).toUtf32();
	if (text.empty())
	{
		call.fail("text is empty: there is nothing to type");
		return;
	}
	if (text.size() > s_MaxTextLength)
	{
		call.fail(toString("text has %u characters; input.text types at most %u in one call", (uint)text.size(), s_MaxTextLength));
		return;
	}
	for (uint i = 0; i < text.size(); ++i)
	{
		// an edit box takes 13 as Enter, which validates it: the keys that act are input.key's
		if (text[i] < 32 || text[i] == 127)
		{
			call.fail(toString("text has the control character %u at %u: Enter, Tab, Backspace and the other keys are input.key", (uint)text[i], i));
			return;
		}
	}
	NLGUI::CCtrlBase *focus = NLGUI::CWidgetManager::getInstance()->getCaptureKeyboard();
	if (focus == nullptr && !inputWaits())
	{
		call.fail("nothing has the keyboard focus: the characters would go nowhere; ui.click an edit box first");
		return;
	}
	heldOfCaller();
	s_Open.Presses = true;
	for (uint i = 0; i < text.size(); ++i)
	{
		CEventChar *event = new CEventChar(text[i], noKeyButton, nullptr);
		addInput(event);
		if (s_Open.Main == nullptr)
			s_Open.Main = event;
	}
	CJsonValue r = CJsonValue::object();
	r.set("characters", (uint32)text.size());
	r.set("keyboard_focus", focus != nullptr ? CJsonValue(focus->getId()) : CJsonValue());
	finishAfterPump(call, r);
}

void methodMouse(CCtrlCall &call)
{
	const CJsonValue &args = call.getArgs();
	std::string action = args.get("action").getString("click");
	double px = args.get("x").getNumber();
	double py = args.get("y").getNumber();
	sint64 steps = args.get("steps").getInt(1);
	if (action != "wheel" && !args.get("steps").isNull())
	{
		call.fail("steps goes with the action wheel");
		return;
	}
	if (steps == 0 || steps > s_MaxWheelSteps || steps < -s_MaxWheelSteps)
	{
		call.fail(toString("steps is 1 to %d up, or -1 to -%d down", s_MaxWheelSteps, s_MaxWheelSteps));
		return;
	}
	std::string error;
	if (!postMouse(action, px, py, args.get("button").getString(), args, (sint)steps, error))
	{
		discardInput();
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

/// Queues the input of the call, with the call waiting for its pump; a call without input is answered now
void finishInput(CCtrlCall &call, const CJsonValue &result, bool forElement)
{
	if (s_Open.Events.empty())
	{
		call.reply(result);
		return;
	}
	releaseModifiers();
	CPendingInput *pending = new CPendingInput();
	pending->Result = result;
	pending->ForElement = forElement;
	pending->HasMain = s_Open.Main != nullptr;
	call.setData(pending);
	s_Open.Pending = pending;
	s_Queue.push_back(s_Open);
	pending->Batch = &s_Queue.back();
	s_Open = CInputBatch();
}

/// Takes back what a batch counted as held: none of it was posted
void unhold(const CInputBatch &batch)
{
	std::map<uint32, CHeld>::iterator it = s_Held.find(batch.Connection);
	if (it == s_Held.end())
		return;
	for (uint i = 0; i < batch.KeysDown.size(); ++i)
		it->second.Keys.erase(batch.KeysDown[i]);
	it->second.Buttons &= ~batch.ButtonsDown;
}

} // anonymous namespace

ICtrlMethod *newMethod(const std::string &name, const std::string &description, const CJsonValue &schema, bool readOnly, CCtrlFunctionMethod::TFunction callFunction,
    CCtrlFunctionMethod::TFunction pollFunction, CCtrlFunctionMethod::TFunction cancelFunction)
{
	return new CCheckedMethod(name, description, schema, readOnly, callFunction, pollFunction, cancelFunction);
}

bool postMouse(const std::string &action, double px, double py, const std::string &buttonName, const CJsonValue &modifierArgs, sint steps, std::string &error)
{
	TMouseButton button;
	if (!mouseButton(buttonName, button))
	{
		error = "button is left, right or middle";
		return false;
	}
	if (action != "move" && action != "move_held" && action != "down" && action != "up" && action != "click" && action != "double_click" && action != "wheel")
	{
		error = "action is move, down, up, click, double_click or wheel";
		return false;
	}
	if (action == "wheel" && (steps == 0 || steps > s_MaxWheelSteps || steps < -s_MaxWheelSteps))
	{
		error = toString("steps is 1 to %d up, or -1 to -%d down", s_MaxWheelSteps, s_MaxWheelSteps);
		return false;
	}
	uint32 width = 0, height = 0;
	Driver->getWindowSize(width, height);
	if (width == 0 || height == 0)
	{
		error = "the window has no area (minimized?)";
		return false;
	}
	if (!isValidDouble(px) || !isValidDouble(py) || px < 0 || py < 0 || px >= width || py >= height)
	{
		error = toString("%.0f, %.0f is outside the client area, which is %ux%u", px, py, width, height);
		return false;
	}
	// in free look the interface does not follow the moves, while it takes a press where the pointer was left
	if (IsMouseFreeLook())
	{
		error = "the mouse is in free look (the camera follows it and the pointer is hidden): a press would act where the pointer was left; end free look first "
		        "(input.mouse up of the right button)";
		return false;
	}
	// NeL's mouse events are in 0..1 from the bottom left, as the window's own emitter makes them
	float x = (float)(px / (double)width);
	float y = (float)(1.0 - py / (double)height);
	CHeld &held = heldOfCaller();
	pressModifiers(modifiersOf(modifierArgs), held);
	uint modifiers = mouseButtons(modifiersDown(CModifiers(), held));
	TMouseButton moveButtons = (TMouseButton)(modifiers | held.Buttons);
	TMouseButton withButton = (TMouseButton)(modifiers | button);
	held.X = x;
	held.Y = y;
	if (action != "up")
		s_Open.Presses = true;

	CEvent *main = nullptr;
	if (action == "move" || action == "move_held")
	{
		addInput(new CEventMouseMove(x, y, action == "move_held" ? (TMouseButton)(moveButtons | button) : moveButtons, nullptr));
	}
	else if (action == "down")
	{
		addInput(new CEventMouseMove(x, y, moveButtons, nullptr));
		addInput(main = new CEventMouseDown(x, y, withButton, nullptr));
		if (!(held.Buttons & button))
			s_Open.ButtonsDown |= button;
		held.Buttons |= button;
	}
	else if (action == "up")
	{
		addInput(new CEventMouseMove(x, y, moveButtons, nullptr));
		addInput(main = new CEventMouseUp(x, y, withButton, nullptr));
		held.Buttons &= ~(uint)button;
	}
	else if (action == "click")
	{
		addInput(new CEventMouseMove(x, y, moveButtons, nullptr));
		addInput(main = new CEventMouseDown(x, y, withButton, nullptr));
		addInput(new CEventMouseUp(x, y, withButton, nullptr));
	}
	else if (action == "double_click")
	{
		// as the window system reports one: down, up, double click, up
		addInput(new CEventMouseMove(x, y, moveButtons, nullptr));
		addInput(main = new CEventMouseDown(x, y, withButton, nullptr));
		addInput(new CEventMouseUp(x, y, withButton, nullptr));
		addInput(new CEventMouseDblClk(x, y, withButton, nullptr));
		addInput(new CEventMouseUp(x, y, withButton, nullptr));
	}
	else
	{
		addInput(new CEventMouseMove(x, y, moveButtons, nullptr));
		for (sint i = 0; i < std::abs(steps); ++i)
		{
			CEvent *wheel = new CEventMouseWheel(x, y, moveButtons, steps > 0, nullptr);
			addInput(wheel);
			if (main == nullptr)
				main = wheel;
		}
	}
	if (s_Open.Main == nullptr)
		s_Open.Main = main;
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
	addKey(key, true, true, 0, heldOfCaller());
	return true;
}

void finishAfterPump(CCtrlCall &call, const CJsonValue &result)
{
	finishInput(call, result, false);
}

void finishAfterPumpOnElement(CCtrlCall &call, const CJsonValue &result)
{
	finishInput(call, result, true);
}

void discardInput()
{
	for (uint i = 0; i < s_Open.Events.size(); ++i)
		delete s_Open.Events[i];
	unhold(s_Open);
	s_Open = CInputBatch();
}

void pollAfterPump(CCtrlCall &call)
{
	CPendingInput *pending = dynamic_cast<CPendingInput *>(call.getData());
	if (pending == nullptr)
	{
		call.fail("the call kept no input to wait for");
		return;
	}
	if (pending->State == CPendingInput::Queued)
		return;
	if (pending->State == CPendingInput::Dropped)
	{
		call.fail("a loading screen (a pump without the interface) came before the input was posted: nothing was sent, as it was meant for the screen before it; "
		          "wait for the state to settle, look again and send it again",
		    CJsonValue::object().set("phase", getPhase()));
		return;
	}
	CJsonValue r = pending->Result;
	if (pending->HasMain)
	{
		r.set("interface", pending->MainHandled);
		if (pending->ForElement && !pending->MainHandled)
		{
			call.fail("the interface did not take the input: an interface animation holds its buttons (a screen that changes), or it does not handle the mouse now; "
			          "the input went on to the game",
			    r);
			return;
		}
	}
	call.reply(r);
}

void takeInput(std::vector<CEvent *> &events)
{
	s_Posted.clear();
	if (!s_Open.Events.empty())
	{
		nlwarning("CTRL: a method left %u input events it neither finished nor discarded: they are dropped", (uint)s_Open.Events.size());
		discardInput();
	}
	CCtrlEndpoint *endpoint = getEndpoint();
	if (endpoint == nullptr)
	{
		// the endpoint is gone with its calls: what waited goes too
		for (std::list<CInputBatch>::iterator it = s_Queue.begin(); it != s_Queue.end(); ++it)
			for (uint i = 0; i < it->Events.size(); ++i)
				delete it->Events[i];
		s_Queue.clear();
		s_Held.clear();
		return;
	}
	// the lease was given back: what its connection held is let go of
	if (!endpoint->isLeaseHeld())
	{
		for (std::map<uint32, CHeld>::iterator it = s_Held.begin(); it != s_Held.end(); ++it)
		{
			if (!it->second.empty())
			{
				nlinfo("CTRL: releasing the keys and buttons connection %u held, as nobody holds the lease now", it->first);
				queueReleases(it->first, it->second);
			}
		}
	}
	for (std::list<CInputBatch>::iterator it = s_Queue.begin(); it != s_Queue.end(); ++it)
	{
		events.insert(events.end(), it->Events.begin(), it->Events.end());
		if (it->Pending != nullptr)
		{
			it->Pending->State = CPendingInput::Posted;
			it->Pending->Batch = nullptr;
			if (it->Main != nullptr)
				s_Posted[it->Main] = it->Pending;
		}
	}
	s_Queue.clear();
}

void dropInputForLoading()
{
	uint dropped = 0;
	for (std::list<CInputBatch>::iterator it = s_Queue.begin(); it != s_Queue.end();)
	{
		if (!it->Presses)
		{
			++it;
			continue;
		}
		unhold(*it);
		for (uint i = 0; i < it->Events.size(); ++i)
			delete it->Events[i];
		if (it->Pending != nullptr)
		{
			it->Pending->State = CPendingInput::Dropped;
			it->Pending->Batch = nullptr;
		}
		it = s_Queue.erase(it);
		++dropped;
	}
	if (dropped > 0)
		nlinfo("CTRL: a loading screen came before the input of %u calls was posted: it is dropped", dropped);
}

void inputHandled(const CEvent &event, bool byInterface)
{
	std::map<const CEvent *, CPendingInput *>::iterator it = s_Posted.find(&event);
	if (it == s_Posted.end())
		return;
	it->second->MainHandled = byInterface;
	s_Posted.erase(it);
}

void registerInputMethods(CCtrlEndpoint &endpoint)
{
	endpoint.setConnectionClosedCallback(onConnectionClosed);
	endpoint.addMethod(newMethod("input.key",
	    "Presses, holds or releases a key, as the keyboard does: ctrl, shift and alt are pressed before it and let go of after it, and Enter, Tab, Backspace and Escape give their "
	    "character too, so Enter validates an edit box and Escape lets go of its focus. The names are NeL's: KeyA, KeyRETURN, KeyESCAPE, KeyF1, KeyUP, KeyDELETE... "
	    "It reaches the game as well as the interface, as a real key does: a movement key moves the character. A key held down is released when the connection closes or "
	    "gives the lease back. The input goes to the next frame that has the interface; a loading screen that comes first drops it and the call fails. Answers once the "
	    "client has handled it, with interface true when the interface took it.",
	    CCtrlSchema()
	        .str("key", "The key: KeyA or A, KeyRETURN, KeyESCAPE, KeyBACK, KeyTAB, KeyF1...", true)
	        .choice("action", "press|down|up", "press (down and up, the default), down, or up.")
	        .integer("repeat", "With press or down, this many key repeats after the first down (0 to 100).")
	        .boolean("ctrl", "With Ctrl.")
	        .boolean("shift", "With Shift.")
	        .boolean("alt", "With Alt."),
	    false, methodKey, pollAfterPump, nullptr));
	endpoint.addMethod(newMethod("input.text",
	    "Types text into what has the keyboard focus, as character events (no key presses), at most 4096 characters; it fails when nothing has the focus. Enter, Tab and "
	    "the other keys that act are input.key's. Answers once the client has handled it.",
	    CCtrlSchema().str("text", "The text, printable characters only.", true), false, methodText, pollAfterPump, nullptr));
	endpoint.addMethod(newMethod("input.mouse",
	    "Moves the mouse to x,y and clicks, presses, releases or turns the wheel there, as the mouse does: the interface finds what is under the pointer, and what it does not "
	    "take goes to the game (the middle button always does). x and y are pixels of the client area from its top left, as a screen grab has them. ctrl, shift and alt are "
	    "pressed before and let go of after. It fails in free look (the camera following the mouse): the camera methods set the view. A button held down is released when the "
	    "connection closes or gives the lease back. Answers once the client has handled it, with interface true when the interface took the press.",
	    CCtrlSchema()
	        .choice("action", "click|double_click|down|up|move|wheel", "What to do (click by default).")
	        .number("x", "Pixels from the left of the client area.", true)
	        .number("y", "Pixels from the top of the client area.", true)
	        .choice("button", "left|right|middle", "The button (left by default).")
	        .integer("steps", "For wheel: the notches, 1 to 100 up, -1 to -100 down (1 by default).")
	        .boolean("ctrl", "With Ctrl held.")
	        .boolean("shift", "With Shift held.")
	        .boolean("alt", "With Alt held."),
	    false, methodMouse, pollAfterPump, nullptr));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_input.cpp */
