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

#include "di_mouse_device.h"
#include "nel/misc/game_device_events.h"
#include "nel/misc/win_event_emitter.h"


#ifdef NL_OS_WINDOWS

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

//======================================================
CDIMouse::CDIMouse() : _MessageMode(RawMode),
					   _MouseSpeed(1.0f),
					   _MouseAccel(10000),
					   _Mouse(NULL),
					   _XAcc(0),
					   _YAcc(0),
					   _XMousePos(0),
					   _YMousePos(0),
					   _LastMouseButtonClicked(-1),
					   _DoubleClickDelay(300),
					   _XFactor(1.f),
					   _YFactor(1.f),
					   OldDIXPos(0),
					   OldDIYPos(0),
					   OldDIZPos(0),
					   _FirstX(true),
   					   _FirstY(true),
					   _SwapButton(false)

{
	std::fill(_MouseButtons, _MouseButtons + MaxNumMouseButtons, false);
	std::fill(_MouseAxisMode, _MouseAxisMode + NumMouseAxis, Raw);
	_MouseFrame.setWH(0, 0, 640, 480);
}

//======================================================
CDIMouse::~CDIMouse()
{
	if (_Mouse)
	{
		_Mouse->Unacquire();
		_Mouse->Release();
	}
}

//======================================================
void	    CDIMouse::setMouseMode(TAxis axis, TAxisMode axisMode)
{
	nlassert(axisMode < AxisModeLast);
	nlassert(axis < AxisLast);
	_MouseAxisMode[axis] = axisMode;
	clampMouseAxis();
}

//======================================================
CDIMouse::TAxisMode	CDIMouse::getMouseMode(TAxis axis) const
{
	nlassert(axis < NumMouseAxis);
	return _MouseAxisMode[axis];
}

//======================================================
void		CDIMouse::setMouseSpeed(float speed)
{
	nlassert(_MessageMode == NormalMode);
	nlassert(speed > 0);
	_MouseSpeed = speed;
}

//======================================================
void		CDIMouse::setMouseAcceleration(uint accel)
{
	_MouseAccel = accel;
}

//======================================================
uint		CDIMouse::getMouseAcceleration() const
{
	return _MouseAccel;
}

//======================================================
bool	CDIMouse::setBufferSize(uint size)
{
	nlassert(size > 0);
	nlassert(_Mouse);
	_Mouse->Unacquire();
	DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = size;
	HRESULT					r = _Mouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
	if (r != DI_OK)	return false;
	_MouseBufferSize = size;
	return true;
}

//======================================================
uint				CDIMouse::getBufferSize() const { return _MouseBufferSize; }

//======================================================
void	CDIMouse::setMousePos(float x, float y)
{
	nlassert(_MessageMode == NormalMode);
	_XMousePos = (sint64) ((double) x * ((sint64) 1 << 32));
	_YMousePos = (sint64) ((double) y * ((sint64) 1 << 32));
}

//======================================================
CDIMouse *CDIMouse::createMouseDevice(IDirectInput8 *di8, HWND hwnd, CDIEventEmitter *diEventEmitter, bool hardware, CWinEventEmitter *we) throw(EDirectInput)
{
	std::auto_ptr<CDIMouse> mouse(new CDIMouse);
	mouse->_DIEventEmitter = diEventEmitter;
	mouse->_Hardware = hardware;
	HRESULT result = di8->CreateDevice(GUID_SysMouse, &(mouse->_Mouse), NULL);
	if (result != DI_OK) throw EDirectInputNoMouse();
	result = mouse->_Mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | (!hardware ? DISCL_EXCLUSIVE:DISCL_NONEXCLUSIVE));
	if (result != DI_OK) throw EDirectInputCooperativeLevelFailed();
	mouse->_Mouse->SetDataFormat(&c_dfDIMouse2);
	mouse->setBufferSize(64);
	mouse->_WE = we;
	mouse->setDoubleClickDelay(::GetDoubleClickTime());

	/** we want an absolute mouse mode, so that, if the event buffer get full, we can retrieve the right position
	  */
	DIPROPDWORD prop;
	prop.diph.dwSize = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow = DIPH_DEVICE;
	prop.diph.dwObj = 0;
	prop.dwData = DIPROPAXISMODE_ABS;
	HRESULT r = mouse->_Mouse->SetProperty(DIPROP_AXISMODE, &prop.diph);
	nlassert(r == DI_OK); // should always succeed...
	//
	mouse->_Mouse->Acquire();
	mouse->_hWnd = hwnd;

	// Enable win32 mouse message only if hardware mouse in normal mode
	if (mouse->_WE)
		mouse->_WE->enableMouseEvents(mouse->_Hardware && (mouse->_MessageMode == IMouseDevice::NormalMode));

	mouse->_SwapButton = GetSystemMetrics(SM_SWAPBUTTON) != 0;

	return mouse.release();
}

//======================================================
float				CDIMouse::getMouseSpeed() const
{
	nlassert(_MessageMode == NormalMode);
	return _MouseSpeed;
}

//======================================================
const CRect &CDIMouse::getMouseFrame() const
{
	nlassert(_MessageMode == NormalMode);
	return _MouseFrame;
}

//======================================================
uint				CDIMouse::getDoubleClickDelay() const { return _DoubleClickDelay; }

//======================================================
inline void	CDIMouse::clampMouseAxis()
{
	if (_MouseAxisMode[XAxis] == Clamped) clamp(_XMousePos, (sint64) _MouseFrame.X  << 32, (sint64) (_MouseFrame.X + _MouseFrame.Width - 1) << 32);
	if (_MouseAxisMode[YAxis] == Clamped) clamp(_YMousePos, (sint64) _MouseFrame.Y << 32, (sint64) (_MouseFrame.X + _MouseFrame.Height - 1) << 32);
}

//======================================================
void CDIMouse::poll(CInputDeviceServer *dev)
{
	nlassert(_Mouse);
	nlassert(_MouseBufferSize > 0);
	static std::vector<DIDEVICEOBJECTDATA> datas;
	datas.resize(_MouseBufferSize);
	DWORD numElements = _MouseBufferSize;
	HRESULT result = _Mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
	if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST)
	{
		result = _Mouse->Acquire();
		HRESULT result = _Mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
		if (result != DI_OK) return;
	}
	else if (result != DI_OK) return;

	if (::IsWindowEnabled(_hWnd) && ::IsWindowVisible(_hWnd))
	{
		for(uint k = 0; k < numElements; ++k)
		{
			CDIEvent *die = new CDIEvent;
			die->Emitter = this;
			die->Datas = datas[k];
			dev->submitEvent(die);
		}
	}
}


//======================================================================
TMouseButton CDIMouse::buildMouseButtonFlags() const
{
	if (_SwapButton)
		return (TMouseButton) (
						   _DIEventEmitter->buildKeyboardButtonFlags()
						   | (_MouseButtons[0] ? rightButton	: 0)
						   | (_MouseButtons[1] ? leftButton		: 0)
						   | (_MouseButtons[2] ? middleButton	: 0)
						  );
	else
		return (TMouseButton) (
						   _DIEventEmitter->buildKeyboardButtonFlags()
						   | (_MouseButtons[0] ? leftButton		: 0)
						   | (_MouseButtons[1] ? rightButton	: 0)
						   | (_MouseButtons[2] ? middleButton	: 0)
						  );
}

//======================================================
TMouseButton CDIMouse::buildMouseSingleButtonFlags(uint button)
{
	static const TMouseButton mb[] = { leftButton, rightButton, middleButton };
	static const TMouseButton mbswap[] = { rightButton, leftButton, middleButton };
	nlassert(button < MaxNumMouseButtons);
	if (_SwapButton)
		return (TMouseButton) (_DIEventEmitter->buildKeyboardButtonFlags() | mbswap[button]);
	else
		return (TMouseButton) (_DIEventEmitter->buildKeyboardButtonFlags() | mb[button]);
}

//======================================================
void CDIMouse::onButtonClicked(uint button, CEventServer *server, uint32 date)
{
	// check for double click
	if (_LastMouseButtonClicked == (sint) button)
	{
		if (date - _MouseButtonsLastClickDate < _DoubleClickDelay)
		{
			CEventMouseDblClk *emdc
			= new CEventMouseDblClk((float) (_XMousePos >> 32),
										    (float) (_YMousePos >> 32),
										    buildMouseSingleButtonFlags(button),
										    _DIEventEmitter);
			server->postEvent(emdc);
			_LastMouseButtonClicked = -1;
		}
		else
		{
			_MouseButtonsLastClickDate = date;
		}
	}
	else
	{
		_LastMouseButtonClicked = button;
		_MouseButtonsLastClickDate = date;
	}
}

//======================================================
void CDIMouse::processButton(uint button, bool pressed, CEventServer *server, uint32 date)
{
	updateMove(server);
	float mx = (float) (_XFactor * (double) _XMousePos / ((double) 65536 * (double) 65536));
	float my = (float) (_YFactor * (double) _YMousePos / ((double) 65536 * (double) 65536));
	if (pressed)
	{
		CEventMouseDown *emd =
		new CEventMouseDown(mx, my, buildMouseSingleButtonFlags(button),
							_DIEventEmitter);
		server->postEvent(emd);
	}
	else
	{
		CEventMouseUp *emu =
		new CEventMouseUp(mx, my, buildMouseSingleButtonFlags(button), _DIEventEmitter);
		server->postEvent(emu);
		onButtonClicked(button, server, date);
	}
	_MouseButtons[button] = pressed;
}

//======================================================
void CDIMouse::submit(IInputDeviceEvent *deviceEvent, CEventServer *server)
{
	if (!_Hardware || (_MessageMode == RawMode))
	{
		CDIEvent *die = safe_cast<CDIEvent *>(deviceEvent);
		bool	pressed;
		switch(die->Datas.dwOfs)
		{
			case	DIMOFS_X:
			{
				if (!_FirstX)
				{
					sint dep = (sint32) die->Datas.dwData - OldDIXPos;

					// Acceleration
					if (_MouseAccel)
					{
						sint accelFactor = abs (dep) / (sint)_MouseAccel;
						dep <<= accelFactor;
					}

					_XAcc += dep;
				}
				else
				{
					_FirstX = false;
				}
				OldDIXPos = (sint32) die->Datas.dwData;
			}
			break;
			case	DIMOFS_Y:
			{
				if (!_FirstY)
				{
					sint dep = (sint32) die->Datas.dwData - OldDIYPos;

					// Acceleration
					if (_MouseAccel)
					{
						sint accelFactor = abs (dep) / (sint)_MouseAccel;
						dep <<= accelFactor;
					}

					_YAcc -= dep;
				}
				else
				{
					_FirstY = false;
				}
				OldDIYPos = (sint32) die->Datas.dwData;
			}
			break;
			case	DIMOFS_Z:
			{
				updateMove(server);
				sint dep = die->Datas.dwData - OldDIZPos;
				OldDIZPos = (sint32) die->Datas.dwData;
				CEventMouseWheel *emw =
				new CEventMouseWheel((float) (_XMousePos >> 32),
											 (float) (_XMousePos >> 32),
											 buildMouseButtonFlags(),
											 dep > 0,
											 _DIEventEmitter);
				server->postEvent(emw);
			}
			break;
			case	DIMOFS_BUTTON0:	/* left button */
				pressed = (die->Datas.dwData & 0x80) != 0;
				processButton(0, pressed, server, die->Datas.dwTimeStamp);
			break;
			case	DIMOFS_BUTTON1: /* right button */
				pressed = (die->Datas.dwData & 0x80) != 0;
				processButton(1, pressed, server, die->Datas.dwTimeStamp);
			break;
			case	DIMOFS_BUTTON2: /* middle button */
				pressed = (die->Datas.dwData & 0x80) != 0;
				processButton(2, pressed, server, die->Datas.dwTimeStamp);
			break;
			default:
				return;
			break;
		}
	}
}

//======================================================
void	CDIMouse::updateMove(CEventServer *server)
{
	if (_XAcc != 0 || _YAcc != 0)
	{
		if (_MessageMode == NormalMode)
		{
			_XMousePos += (sint64) ((double) _MouseSpeed * (sint64) _XAcc * ((sint64) 1 << 32));
			_YMousePos += (sint64) ((double) _MouseSpeed * (sint64) _YAcc * ((sint64)  1 << 32));
			clampMouseAxis();
			CEventMouseMove *emm = new CEventMouseMove((float) (_XFactor * (double) _XMousePos / ((double) 65536 * (double) 65536)), (float) (_YFactor * (double) _YMousePos / ((double) 65536 * (double) 65536)), buildMouseButtonFlags(), _DIEventEmitter);
			server->postEvent(emm);
		}
		else
		{
			CGDMouseMove *emm = new CGDMouseMove(_DIEventEmitter, this, _XAcc, _YAcc);
			server->postEvent(emm);
		}
		_XAcc = _YAcc = 0;
	}
}


//======================================================
void	CDIMouse::convertStdMouseMoveInMickeys(float &dx, float &dy) const
{
	// get in same scale as _XAcc and _YAcc
	double	xacc= ((double)dx/_XFactor) / _MouseSpeed;
	double	yacc= ((double)dy/_YFactor) / _MouseSpeed;

	dx= float(xacc);
	dy =float(yacc);
}


//======================================================
void CDIMouse::transitionOccured(CEventServer *server, const IInputDeviceEvent *)
{
	updateMove(server);
}

//======================================================
void	CDIMouse::setButton(uint button, bool pushed)
{
	nlassert(button < MaxNumMouseButtons);
	_MouseButtons[button] = pushed;
}

//======================================================
bool	CDIMouse::getButton(uint button) const
{
	nlassert(button < MaxNumMouseButtons);
	return _MouseButtons[button];
}

//======================================================
void	CDIMouse::setDoubleClickDelay(uint ms)
{
	nlassert(ms > 0);
	_DoubleClickDelay = ms;
}

//======================================================
void	CDIMouse::setMouseFrame(const CRect &rect)
{
	nlassert(_MessageMode == NormalMode);
	_MouseFrame = rect;
}

//======================================================
void	CDIMouse::setMessagesMode(TMessageMode mode)
{
	nlassert(mode < MessageModeLast);
	_MessageMode = mode;
	_FirstX = _FirstY = true;

	// Enable win32 mouse message only if hardware mouse in normal mode
	if (_WE)
		_WE->enableMouseEvents(_Hardware && (_MessageMode == NormalMode));
}


} // NLMISC

#endif // NL_OS_WINDOWS























