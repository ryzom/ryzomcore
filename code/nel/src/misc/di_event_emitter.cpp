// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "stdmisc.h"

#include "nel/misc/di_event_emitter.h"
#include "nel/misc/events.h"
#include "nel/misc/win_event_emitter.h"
//
#include "di_mouse_device.h"
#include "di_keyboard_device.h"
#include "di_game_device.h"


#ifdef NL_OS_WINDOWS

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

static const char DirectInputLibName[] = "dinput8.dll";

/////////////////////////////////
//	CDIEventEmitter statics	   //
/////////////////////////////////

HMODULE	CDIEventEmitter::_DirectInputLibHandle = 0;
CDIEventEmitter::TPDirectInput8Create CDIEventEmitter::_PDirectInput8Create = NULL;
uint CDIEventEmitter::_NumCreatedInterfaces = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////
// CDIEventEmitter implementation //
////////////////////////////////////


//======================================================
CDIEventEmitter::CDIEventEmitter(HWND hwnd, CWinEventEmitter *we)
:
 _hWnd(hwnd),
 _WE(we),
 _DInput8(NULL),
 _Keyboard(NULL),
 _Mouse(NULL),
 _ButtonsFlags(noButton)
{
}
//======================================================
CDIEventEmitter::~CDIEventEmitter()
{
	releaseMouse();
	releaseKeyboard();
	// release all devices
	while (_DeviceServer.getNumDevices() != 0)
	{
		IInputDevice *dev = _DeviceServer.getDevice(0);
		_DeviceServer.removeDevice(dev);
		delete dev;
	}
	if (_DInput8) _DInput8->Release();
	-- _NumCreatedInterfaces;
	if (_NumCreatedInterfaces == 0) unloadLib();
}

//======================================================
CDIEventEmitter *CDIEventEmitter::create(HINSTANCE hinst, HWND hwnd, CWinEventEmitter *we) throw(EDirectInput)
{
	if (!loadLib()) throw EDirectInputLibNotFound();
	std::auto_ptr<CDIEventEmitter> dxee(new CDIEventEmitter(hwnd, we));
	HRESULT result = _PDirectInput8Create(hinst,
										  DIRECTINPUT_VERSION,
										  IID_IDirectInput8A,
										  (void **) &dxee->_DInput8,
										  NULL);
	if (result != DI_OK) throw EDirectInputInitFailed();

	// ok, everything's fine, commit changes
	++_NumCreatedInterfaces;
	return dxee.release();
}


//======================================================
bool CDIEventEmitter::loadLib()
{
	if (_DirectInputLibHandle != 0) return true; // library already loaded ?
	HMODULE handle = ::LoadLibrary(DirectInputLibName);
	if (handle == 0) return false;
	//	try to get the creation function
	TPDirectInput8Create cf = (TPDirectInput8Create) ::GetProcAddress(handle, "DirectInput8Create");
	if (!cf)
	{
		::FreeLibrary(handle);
		return false;
	}
	// commit changes
	_DirectInputLibHandle = handle;
	_PDirectInput8Create  = cf;
	return true;
}


//======================================================
void CDIEventEmitter::unloadLib()
{
	nlassert(_DirectInputLibHandle != 0);
	::FreeLibrary(_DirectInputLibHandle);
	_DirectInputLibHandle = 0;
	_PDirectInput8Create = NULL;
}

//======================================================
void CDIEventEmitter::poll(CEventServer *server)
{
	if (_WE) _ButtonsFlags = _WE->buildFlags();
	if (!server)
		server=&_InternalServer;
	_DeviceServer.poll(server);
}


//======================================================
TMouseButton	CDIEventEmitter::buildButtonsFlags() const
{
	uint mouseFlags;
	uint keybFlags;
	//
	if (_Mouse) // takes the flags from the direct input mouse
	{
		mouseFlags = (_Mouse->getButton(0) ? leftButton		: 0)
					 | (_Mouse->getButton(1) ? rightButton	: 0)
					 | (_Mouse->getButton(2) ? middleButton	: 0);
	}
	else // takes the flags from the system mouse
	{
		mouseFlags = _ButtonsFlags & (leftButton | rightButton | middleButton);
	}
	//
	if (_Keyboard) // takes the flags from the direct input keyboard
	{
		keybFlags = (_Keyboard->ShiftPressed	  ? shiftButton		: 0)
					| (_Keyboard->AltPressed	  ? altButton		: 0)
					| (_Keyboard->CtrlPressed	  ? ctrlButton		: 0);
	}
	else // takes the flags from the system keyboard
	{
		keybFlags = _ButtonsFlags & (shiftButton | altButton | ctrlButton);
	}
	return (TMouseButton) (keybFlags | mouseFlags);
}

//======================================================
IMouseDevice	*CDIEventEmitter::getMouseDevice(bool hardware) throw(EInputDevice)
{
	if (_Mouse) return _Mouse;	// already created ?
	try
	{
		// Create a mouse
		std::auto_ptr<CDIMouse> mouse(CDIMouse::createMouseDevice(_DInput8, _hWnd, this, hardware, _WE));
		// register to the device server
		_DeviceServer.registerDevice(mouse.get());
		_Mouse = mouse.get();
		return mouse.release();
	}
	catch (...)
	{
		if (_WE)	_WE->enableMouseEvents(true);
		throw;
	}
}

//======================================================
void	CDIEventEmitter::releaseMouse()
{
	if (!_Mouse) return;
	// reupdate the system keyboard flags
	if (_WE)
	{
		_WE->resetButtonFlagState();
		_WE->enableMouseEvents(true);
	}
	// remove the device
	_DeviceServer.removeDevice(_Mouse);
	delete _Mouse;
	_Mouse = NULL;
}

//===========================================================================
IKeyboardDevice	*CDIEventEmitter::getKeyboardDevice() throw(EInputDevice)
{
	if (_Keyboard) return _Keyboard;
	try
	{
		// create a keyboard
		std::auto_ptr<CDIKeyboard> keyboard(CDIKeyboard::createKeyboardDevice(_DInput8, _hWnd, this, _WE));
		// register to the device server
		_DeviceServer.registerDevice(keyboard.get());
		_Keyboard = keyboard.get();
		return keyboard.release();
	}
	catch (...)
	{
		if (_WE) _WE->enableKeyboardEvents(true);
		throw;
	}
}

//==========================================================================
void	CDIEventEmitter::releaseKeyboard()
{
	if (!_Keyboard) return;
	// reupdate the system keyboard flags
	if (_WE)
	{
		_WE->resetButtonFlagState();
		_WE->enableKeyboardEvents(true);
	}
	//
	_DeviceServer.removeDevice(_Keyboard);
	delete _Keyboard;
	_Keyboard = NULL;
}


//==========================================================================
void	CDIEventEmitter::submitEvents(CEventServer &server, bool allWindows)
{
	_InternalServer.setServer(&server);
	_InternalServer.pump(allWindows);
}

//==========================================================================
void	CDIEventEmitter::emulateMouseRawMode(bool enable)
{
	nlerror("no raw mode emulation on windows, the CDIMouse has a real raw mode");
}

//==========================================================================
/// Tool fct to retrieve the game devices.
static BOOL CALLBACK DIEnumDevicesDescCallback
(
  LPCDIDEVICEINSTANCE lpddi,
  LPVOID pvRef
)
{
	CGameDeviceDesc	desc;
	desc.InstanceName = lpddi->tszInstanceName;
	desc.ProductName  = lpddi->tszProductName;
	switch (lpddi->wUsage & 0xff)
	{
		case DI8DEVTYPE_JOYSTICK: desc.DevType = CGameDeviceDesc::Joystick;  break;
		case DI8DEVTYPE_GAMEPAD:  desc.DevType = CGameDeviceDesc::GamePad;	 break;
		default: desc.DevType = CGameDeviceDesc::DontKnow; break;
	}
	TDeviceDescVect *dv = (TDeviceDescVect *) pvRef;
	dv->push_back(desc);
	return DIENUM_CONTINUE;
}


//==========================================================================
/// Tool fct to retrieve the game devices GUID
static BOOL CALLBACK DIEnumDevicesGUIDCallback
(
  LPCDIDEVICEINSTANCE lpddi,
  LPVOID pvRef
)
{
	std::vector<GUID> *gv = (std::vector<GUID> *) pvRef;
	gv->push_back(lpddi->guidInstance);
	return DIENUM_CONTINUE;
}


//==========================================================================
void	CDIEventEmitter::enumerateGameDevice(TDeviceDescVect &descs) throw(EInputDevice)
{
	uint k;
	nlassert(_DInput8);
	descs.clear();
	// enum all devices of interest
	_DInput8->EnumDevices(DI8DEVCLASS_GAMECTRL, &DIEnumDevicesDescCallback, (LPVOID) &descs, DIEDFL_ALLDEVICES);
	for (k = 0; k < descs.size(); ++k) descs[k].Connected = false;
	// enum all connected devices
	static TDeviceDescVect connecteds;
	_DInput8->EnumDevices(DI8DEVCLASS_GAMECTRL, &DIEnumDevicesDescCallback, (LPVOID) &connecteds, DIEDFL_ATTACHEDONLY);
	// see which devices are connected
	for (k = 0; k < connecteds.size(); ++k)
	{
		TDeviceDescVect::iterator it = std::find(descs.begin(), descs.end(), connecteds[k]);
		it->Connected = true;
	}
}

//==========================================================================
IGameDevice	*CDIEventEmitter::createGameDevice(const std::string &instanceName) throw(EInputDevice)
{
	static TDeviceDescVect		deviceDescs;
	static std::vector<GUID>	deviceGUID;

	nlassert(_DInput8);
	enumerateGameDevice(deviceDescs);
	// get the ID for each device
	deviceGUID.clear();
	HRESULT r = _DInput8->EnumDevices(DI8DEVCLASS_GAMECTRL, &DIEnumDevicesGUIDCallback, (LPVOID) &deviceGUID, DIEDFL_ALLDEVICES);
	nlassert(r == DI_OK);
	nlassert(deviceDescs.size() == deviceGUID.size());

	// search the device that match the instance name
	for (uint k = 0;  k < deviceDescs.size(); ++k)
	{
		if (deviceDescs[k].InstanceName == instanceName)
		{
			std::auto_ptr<CDIGameDevice> gd(CDIGameDevice::createGameDevice(_DInput8, _hWnd, this, deviceDescs[k], deviceGUID[k]));
			// insert in the device server
			_DeviceServer.registerDevice(gd.get());
			return gd.release();
		}
	}
	return NULL;
}

//==========================================================================
void	CDIEventEmitter::releaseGameDevice(IGameDevice	*gd)
{
	nlassert(gd);
	CDIGameDevice *digd = safe_cast<CDIGameDevice *>(gd);
	_DeviceServer.removeDevice(digd);
	delete gd;
}


} // NLMISC

#endif // NL_OS_WINDOWS
