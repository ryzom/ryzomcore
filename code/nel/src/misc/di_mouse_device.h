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

#ifndef NL_DI_MOUSE_DEVICE_H
#define NL_DI_MOUSE_DEVICE_H

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS


#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/input_device_server.h"
#include "nel/misc/mouse_device.h"
#include <dinput.h>


namespace NLMISC
{


//
struct EDirectInputNoMouse : public EDirectInput
{
	EDirectInputNoMouse() : EDirectInput("No mouse found") {}
};


class CDXEventEmitter;


/**
 * Direct Input implementation of a mouse
 * \see CDIEventEmitter
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CDIMouse : public IMouseDevice
{
public:
	enum { MaxNumMouseButtons = 3, NumMouseAxis = 3};
public:
	///\name Object
	//@{
		virtual ~CDIMouse();
		/** Create a mouse device from a valid DirectInput8 pointer. This must then be deleted by the caller.
		  * \return the interface or throw an exception if the creation failed
		  */
		static CDIMouse *createMouseDevice(IDirectInput8 *di8, HWND hwnd, CDIEventEmitter *diEventEmitter, bool hardware, class CWinEventEmitter *we) throw(EDirectInput);
	//@}

	///\name Mouse params, inherited from IMouseDevice
	//@{
		void				setMessagesMode(TMessageMode mode);
		TMessageMode		getMessagesMode() const { return _MessageMode; }
		void				setMouseMode(TAxis axis, TAxisMode axisMode);
		TAxisMode			getMouseMode(TAxis axis) const;
		void				setMouseSpeed(float speed);
		float				getMouseSpeed() const;
		void				setMouseAcceleration(uint speed);
		uint				getMouseAcceleration() const;
		void				setMouseFrame(const CRect &rect);
		const CRect			&getMouseFrame() const;
		void				setDoubleClickDelay(uint ms);
		uint				getDoubleClickDelay() const;
		void				setMousePos(float x, float y);
		void				setFactors(float xFactor, float yFactor)
		{
			nlassert(_MessageMode == NormalMode);
			_XFactor = xFactor;
			_YFactor = yFactor;
		}
		float				getXFactor() const { nlassert(_MessageMode == NormalMode); return _XFactor; }
		float				getYFactor() const { 	nlassert(_MessageMode == NormalMode); return _YFactor; }
		void				convertStdMouseMoveInMickeys(float &dx, float &dy) const;
	//@}

	///\name From IInputDevice
	//@{

		virtual bool setBufferSize(uint size);
		virtual uint getBufferSize() const;
	//@}

	///\name From IInputDevice
	//@{
		void	setButton(uint button, bool pushed);
		bool	getButton(uint button) const;
	//@}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	LPDIRECTINPUTDEVICE8		_Mouse;
	//
	bool						_Hardware;
	//
	TMessageMode				_MessageMode;
	//
	TAxisMode					_MouseAxisMode[NumMouseAxis];
	sint64						_XMousePos, _YMousePos; // position encoded in fixed point 32 : 32. This allow wrapping and no loss of precision, when not in clamped mode
	                                                    // NB: this is sint64 because of max range reached with 16:16 when looping around x with great mouse speed
	bool						_FirstX, _FirstY;
	float						_MouseSpeed;
	uint						_MouseAccel;
	CRect						_MouseFrame;
	//
	bool						_MouseButtons[MaxNumMouseButtons];
	uint32						_MouseButtonsLastClickDate;
	sint						_LastMouseButtonClicked;
	uint						_DoubleClickDelay;
	uint						_MouseBufferSize;
	HWND						_hWnd;
	//
	sint32						OldDIXPos, OldDIYPos, OldDIZPos; // old positions reported by direct input
	sint						_XAcc, _YAcc; // accumulate move (needed because they are generated on a single axis for each DI event)
	float						_XFactor, _YFactor;
	//
	CDIEventEmitter				*_DIEventEmitter;
	// The windows emitter to enable / disble win32 mouse messages
	NLMISC::CRefPtr<CWinEventEmitter>	_WE;
	// Does the button left and right are swapped ?
	bool						_SwapButton;
private:
	/// ctor
	CDIMouse();
	/// Clamp the mouse axis that need to be.
	void					clampMouseAxis();
	///  Sum the mouse move and produce an event
	void					updateMove(CEventServer *server);
	void					processButton(uint button, bool pressed, CEventServer *server, uint32 date);
	TMouseButton			buildMouseButtonFlags() const;
	TMouseButton			buildMouseSingleButtonFlags(uint button);
	void					onButtonClicked(uint button, CEventServer *server, uint32 date);
	///\name From IInputDevice
	//@{
		virtual void poll(CInputDeviceServer *dev);
		virtual void submit(IInputDeviceEvent *deviceEvent, CEventServer *server);
		virtual void transitionOccured(CEventServer *server, const IInputDeviceEvent *nextMessage);
	//@}
};


} // NL3D


#endif // NL_OS_WINDOWS

#endif // NL_DI_MOUSE_H

/* End of di_mouse.h */
