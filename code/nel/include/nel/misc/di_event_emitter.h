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

#ifndef NL_DI_EVENT_EMITTER_H
#define NL_DI_EVENT_EMITTER_H



#include "types_nl.h"


#ifdef NL_OS_WINDOWS


#define DIRECTINPUT_VERSION 0x0800

#include "input_device_server.h"
#include "input_device_manager.h"
#include "event_emitter.h"
#include "smart_ptr.h"
#include "events.h"
#include "rect.h"
#include "game_device.h"
#define NOMINMAX
#include <windows.h>
#include <dinput.h>



namespace NLMISC
{


class CWinEventEmitter;
class CDIKeyboard;
class CDIMouse;
struct IMouseDevice;
struct IKeyboardDevice;

//
struct EDirectInput : public EInputDevice
{
	EDirectInput(const char *reason) : EInputDevice(reason) {}
};
//
struct EDirectInputLibNotFound : public  EDirectInput
{
	EDirectInputLibNotFound() : EDirectInput("can't found the direct input dll") {}
};
//
struct EDirectInputInitFailed : public  EDirectInput
{
	EDirectInputInitFailed() : EDirectInput("Direct input initialization failed") {}
};
//
struct EDirectInputCooperativeLevelFailed : public  EDirectInput
{
	EDirectInputCooperativeLevelFailed() : EDirectInput("Direct Input Device Cooperative level couldn't be set") {}
};


// Class to represent Direct Inputs events
struct CDIEvent : public IInputDeviceEvent
{
	virtual bool	operator < (const IInputDeviceEvent &ide) const
	{
		// just compare the dates
		return Datas.dwTimeStamp < (safe_cast<const CDIEvent *>(&ide))->Datas.dwTimeStamp;
	}
	DIDEVICEOBJECTDATA	Datas;
};

/**
 * This manage events by using DirectInput8.
 * This should be polled regularly.
 * This can be mixed with a CWinEmitter (for example, you may have mouse using direct input, and keyboard using standard messages)
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CDIEventEmitter : public IEventEmitter, public IInputDeviceManager
{
public:
	/** Build a Direct Input Event Emitter object. An exception containing the reason is thrown if the initialization failed.
	  * The obtained object must be released by deleting it.
	  * \param hinst the instance of the application.
	  * \param hwnd  the main window of the application.
	  * \param we A windows eventsemitter. Can be NULL. Needed if you want to mix WIN32 events and Direct Input events
	  *			  (for example, a Direct Input Mouse and a Win32 Keyboard)
	  */
	static CDIEventEmitter *create(HINSTANCE hinst, HWND hwnd, CWinEventEmitter *we);
	~CDIEventEmitter();
public:

	/// This poll the direct input state, directly storing the result in the given server, or keeping the result in internal server if NULL.
	void					poll(CEventServer *server = NULL);

	///\name From IDeviceManager, access to devices
	//@{
		// Test if a mouse has been created (by a call to getMouseDeivce)
		virtual	bool			isMouseCreated() { return _Mouse != NULL; }
		/** Create the mouse device if needed (one active at a time for that object, repeated calls returns the same pointer) and get an interface on it. An exception if thrown if it couldn't be obtained.
		  * If this object has a pointer on a win32 emiter, Win32 mouse messages are replaced by this mouse messages.
		  */
		virtual IMouseDevice	*getMouseDevice(bool hardware) throw(EInputDevice);
		/// remove the direct input mouse
		virtual void	releaseMouse();
		/** Create the keyboard device if needed (one active at a time for that object, repeated calls returns the same pointer)  and get an interface on it.
		  * If this object has a pointer on a win32 emiter, Win32 keyboard messages are replaced by this keyboard messages.
		  * NB: A direct input has no notion of localization or key combinations. See keyboard_device.h for more infos
		  */
		virtual IKeyboardDevice	*getKeyboardDevice() throw(EInputDevice);
		/// remove the direct input keyboard
		virtual void	releaseKeyboard();
		// Enumerates current game devices (gamepads, joystick etc.). The result is stored in the given vector
		virtual void	enumerateGameDevice(TDeviceDescVect &descs) throw(EInputDevice);
		// Create the given game device from its instance name. It also means that it will begin to sends inputs
		virtual IGameDevice	*createGameDevice(const std::string &instanceName) throw(EInputDevice);
		// Release the given game device
		virtual void		 releaseGameDevice(IGameDevice	*);
	//@}

	/// from IEventEmitter
	virtual void			submitEvents(CEventServer &server, bool allWindows);
	virtual void			emulateMouseRawMode(bool enable);

	// Build a TMouseButton value from the current buttons state
	TMouseButton	buildButtonsFlags() const;
	// Build a TMouseButton value (but with no mouse values)
	TMouseButton	buildKeyboardButtonFlags() const
	{
		return (TMouseButton) (buildButtonsFlags() & (ctrlButton|shiftButton|altButton));
	}

//================================================================
//================================================================
//================================================================
private:
	typedef HRESULT (WINAPI * TPDirectInput8Create) (HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
	// Private internal server message, used to stored all messages internally before to dispatch them, when no server is supplied to poll(...
	class CDIEventServer : CEventServer
	{
		friend class CDIEventEmitter;
	public:
		void setServer (CEventServer *server)
		{
			_Server = server;
		}
	private:
		bool pumpEvent(CEvent *event)
		{
			CEventServer::pumpEvent(event);
			_Server->postEvent (event);
			return false;
		}
	private:
		CEventServer *_Server;
	};
private:
	HWND								_hWnd;
	TMouseButton						_ButtonsFlags;
	NLMISC::CRefPtr<CWinEventEmitter>	_WE;
	static HMODULE				_DirectInputLibHandle;
	static TPDirectInput8Create _PDirectInput8Create;
	static uint					_NumCreatedInterfaces;
private:
	static bool loadLib();
	static void unloadLib();
//====
private:
	CDIEventServer							_InternalServer;
	CInputDeviceServer						_DeviceServer;
	IDirectInput8							*_DInput8;
	CDIMouse								*_Mouse;
	CDIKeyboard								*_Keyboard;
private:
	CDIEventEmitter(HWND hwnd, CWinEventEmitter *we);
};



} // NLMISC

#endif // NL_WINDOWS


#endif // NL_DX_EVENT_EMITTER_H

/* End of dx_event_emitter.h */
