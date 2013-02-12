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
#include "di_game_device.h"
#include "nel/misc/game_device_events.h"

#ifdef NL_OS_WINDOWS

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

//============================================================================
CDIGameDevice::CDIGameDevice() : _Device(NULL)
{
	::memset(&_CurrentState, 0, sizeof(_CurrentState));
}

//============================================================================
CDIGameDevice::~CDIGameDevice()
{
	if (_Device)
	{
		_Device->Unacquire();
		_Device->Release();
	}
}

//============================================================================
CDIGameDevice *CDIGameDevice::createGameDevice(IDirectInput8 *di8,
											   HWND hwnd,
											   CDIEventEmitter *diEventEmitter,
											   const CGameDeviceDesc &desc,
											   REFGUID rguid) throw(EDirectInput)
{
	nlassert(diEventEmitter);
	nlassert(di8);
	std::auto_ptr<CDIGameDevice> dev(new CDIGameDevice);
	//

	HRESULT r = di8->CreateDevice(rguid, &dev->_Device, NULL);
	if (r != DI_OK) throw EDirectInputGameDeviceNotCreated();

	r = dev->_Device->SetDataFormat(pJoyDataFormat);
	nlassert(r == DI_OK);
	//
	r = dev->_Device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (r != DI_OK) throw EDirectInputCooperativeLevelFailed();
	//
	//
	dev->_Desc = desc;
	dev->_EventEmitter = diEventEmitter;
	dev->querryControls();
	return dev.release();
}

//============================================================================
void CDIGameDevice::begin(CEventServer *server)
{
	nlassert(_Device);
	HRESULT r;
	r = _Device->Poll();
	if (r == DIERR_INPUTLOST  || r == DIERR_NOTACQUIRED)
	{
        r = _Device->Acquire();
		if (r != DI_OK) return;
		r = _Device->Poll();
		if (r != DI_OK) return;
	}

	CDIJoyState	newState;
	r = _Device->GetDeviceState(sizeof(CDIJoyState), &newState);
	if (r != DI_OK) return;

	uint k;
	//////////
	// Axis //
	//////////
	for (k = 0; k < MaxNumAxis; ++k)
	{
		CAxis &axis = _Axis[k];
		if (axis.Present)
		{

			if (((LONG *) &newState)[k] != ((LONG *) &_CurrentState)[k]) // state changed ?
			{
				// update position
				axis.Value		= 2.f * (((LONG *) &newState)[k] - axis.Min) / (float) (axis.Max - axis.Min) - 1.f;
				// create event
				CGDAxisMoved	*event = new CGDAxisMoved((IGameDevice::TAxis) k, axis.Value, this, _EventEmitter);
				// update state
				((LONG *) &_CurrentState)[k] = ((LONG *) &newState)[k];
				//
				server->postEvent(event);
				//
			}
		}
	}


	/////////////
	// Buttons //
	/////////////
	for (k = 0; k < _Buttons.size(); ++k)
	{
		CButton &bt = _Buttons[k];
		if ((newState.rgbButtons[k] & 0x80) != (_CurrentState.rgbButtons[k] & 0x80))
		{
			bool pushed = (newState.rgbButtons[k] & 0x80) != 0;
			// update the state of the button
			bt.Pushed = pushed;
			CGDButton *event;
			if (pushed) event = new CGDButtonDown(k, this, _EventEmitter);
			else event = new CGDButtonUp(k, this, _EventEmitter);
			// update state
			_CurrentState.rgbButtons[k] = newState.rgbButtons[k];
			server->postEvent(event);
		}
	}

	/////////////
	// Sliders //
	/////////////
	for (k = 0; k < _Sliders.size(); ++k)
	{
		CSlider &sl = _Sliders[k];
		if (newState.rglSlider[k] != _CurrentState.rglSlider[k]) // state changed ?
		{
			// update position
			sl.Pos		= ( newState.rglSlider[k] - sl.Min) / (float) (sl.Max - sl.Min);
			// create event
			CGDSliderMoved	*event = new CGDSliderMoved(sl.Pos, k, this, _EventEmitter);
			// update state
			_CurrentState.rglSlider[k] = newState.rglSlider[k];
			//
			server->postEvent(event);
		}
	}

	//////////
	// POVs //
	//////////
	for (k = 0; k < _POVs.size(); ++k)
	{
		CPOV &pov = _POVs[k];
		if (newState.rgdwPOV[k] != _CurrentState.rgdwPOV[k]) // state changed ?
		{
			DWORD value = newState.rgdwPOV[k];

			pov.Centered = (LOWORD(value) == 0xFFFF);
			if (!pov.Centered)
			{
				// update position
				pov.Angle		= value / 100.f;
			}
			// create event
			CGDPOVChanged	*event = new CGDPOVChanged(pov.Centered, pov.Angle, k, this, _EventEmitter);
			// update state
			_CurrentState.rgdwPOV[k] = newState.rgdwPOV[k];
			//
			server->postEvent(event);
		}
	}

}

//============================================================================
void CDIGameDevice::poll(CInputDeviceServer *dev)
{
	// buffered datas not supported
}

//============================================================================
void CDIGameDevice::submit(IInputDeviceEvent *deviceEvent, CEventServer *server)
{
	// should never be called, buffered datas not supported
	nlassert(0);
}


//============================================================================
/** Tool fct : tests whether a DIDEVICEOBJECTINSTANCE contains a controls name and return it,
  * or build a default one
  */
static void BuildCtrlName(LPCDIDEVICEOBJECTINSTANCE lpddoi,
						  std::string &destName,
						  const char *defaultName)
{
	if (lpddoi->dwSize >= offsetof(DIDEVICEOBJECTINSTANCE, tszName) + sizeof(TCHAR[MAX_PATH]))
	{
		destName = (::strcmp("N/A", lpddoi->tszName) == 0) ? defaultName
														  : lpddoi->tszName;
	}
	else
	{
		destName = defaultName;
	}
}

//============================================================================
// A callback to enumerate the controls of a device
static BOOL CALLBACK DIEnumDeviceObjectsCallback
(
  LPCDIDEVICEOBJECTINSTANCE lpddoi,
  LPVOID pvRef
)
{

	CDIGameDevice *gd = (CDIGameDevice *) pvRef;
	return gd->processEnumObject(lpddoi);
}



//=======================================================================
// get range for an axis
static HRESULT	GetDIAxisRange(LPDIRECTINPUTDEVICE8 device, uint offset, DWORD type, sint &min, sint &max)
{
	DIPROPRANGE diprg;
    diprg.diph.dwSize       = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow        = DIPH_BYOFFSET;
    diprg.diph.dwObj        = offset;

	// Set the range for the axis
	HRESULT r = device->GetProperty(DIPROP_RANGE, &diprg.diph);

	if (r == DIERR_OBJECTNOTFOUND)
	{
		// try from its ID
		diprg.diph.dwHow        = DIPH_BYID;
		diprg.diph.dwObj        = type;

		// Set the range for the axis
		HRESULT r = device->GetProperty(DIPROP_RANGE, &diprg.diph);
		if (r !=  DI_OK)
		{
			// setup default values ...
			min = 0;
			max = 65535;
			return r;
		}
	}
	else if (r != DI_OK)
	{
		min = 0;
		max = 65535;
		return r;
	}


/*	switch (r)
	{
		default:
			nlinfo("ok");
		break;
		case DIERR_INVALIDPARAM:
			nlinfo("invalid param");
		break;
		case DIERR_NOTEXCLUSIVEACQUIRED:
			nlinfo("DIERR_NOTEXCLUSIVEACQUIRED");
		break;
		case DIERR_NOTINITIALIZED:
			nlinfo("DIERR_NOTINITIALIZED");
		break;
		case DIERR_OBJECTNOTFOUND:
			nlinfo("DIERR_OBJECTNOTFOUND");
		break;
		case DIERR_UNSUPPORTED:
			nlinfo("DIERR_UNSUPPORTED");
		break;
	}*/


	min = (sint) diprg.lMin;
	max = (sint) diprg.lMax;

	return r;
}

//============================================================================
BOOL CDIGameDevice::processEnumObject(LPCDIDEVICEOBJECTINSTANCE lpddoi)
{
	// the dwSize field gives us the size of the objects, and the available fields
	// has this object the field guidType and dwOfs ?
	if (lpddoi->dwSize < offsetof(DIDEVICEOBJECTINSTANCE, dwOfs) + sizeof(DWORD)) return DIENUM_CONTINUE;

	uint ctrlType = (uint) lpddoi->dwType;

	///////////////////////////////////////////
	//	axis, we only support absolute ones  //
	///////////////////////////////////////////

	if (lpddoi->guidType == GUID_XAxis && (ctrlType & DIDFT_ABSAXIS) )
	{
		GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Axis[XAxis].Min, _Axis[XAxis].Max);
		BuildCtrlName(lpddoi, _Axis[XAxis].Name, "X Axis");
		_Axis[XAxis].Present = true;
		return DIENUM_CONTINUE;
	}
	if (lpddoi->guidType == GUID_YAxis && (ctrlType & DIDFT_ABSAXIS))
	{
		GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Axis[YAxis].Min, _Axis[YAxis].Max);
		BuildCtrlName(lpddoi, _Axis[YAxis].Name, "Y Axis");
		_Axis[YAxis].Present = true;
		return DIENUM_CONTINUE;
	}

	if (lpddoi->guidType == GUID_ZAxis && (ctrlType & DIDFT_ABSAXIS))
	{
		GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Axis[ZAxis].Min, _Axis[ZAxis].Max);
		BuildCtrlName(lpddoi, _Axis[ZAxis].Name, "Z Axis");
		_Axis[ZAxis].Present = true;
		return DIENUM_CONTINUE;
	}
	if (lpddoi->guidType == GUID_RxAxis && (ctrlType & DIDFT_ABSAXIS))
	{
		GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Axis[RXAxis].Min, _Axis[RXAxis].Max);
		BuildCtrlName(lpddoi, _Axis[RXAxis].Name, "RX Axis");
		_Axis[RXAxis].Present = true;
		return DIENUM_CONTINUE;
	}
	if (lpddoi->guidType == GUID_RyAxis && (ctrlType & DIDFT_ABSAXIS))
	{
		GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Axis[RYAxis].Min, _Axis[RYAxis].Max);
		BuildCtrlName(lpddoi, _Axis[RYAxis].Name, "RY Axis");
		_Axis[RYAxis].Present = true;
		return DIENUM_CONTINUE;
	}
	if (lpddoi->guidType == GUID_RzAxis && (ctrlType & DIDFT_ABSAXIS))
	{
		GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Axis[RZAxis].Min, _Axis[RZAxis].Max);
		BuildCtrlName(lpddoi, _Axis[RZAxis].Name, "RZ Axis");
		_Axis[RZAxis].Present = true;
		return DIENUM_CONTINUE;
	}


	// has this object the field dwType ?
	if (lpddoi->dwSize < offsetof(DIDEVICEOBJECTINSTANCE, dwType) + sizeof(DWORD)) return DIENUM_CONTINUE;


	uint type = lpddoi->dwType;
	/////////////
	// Buttons //
	/////////////
	if (type & DIDFT_BUTTON)
	{
		if (_Buttons.size() < MaxNumButtons)
		{
			_Buttons.push_back(CButton());
			uint buttonIndex = (uint)_Buttons.size() - 1;
			char defaultButtonName[32];
			smprintf(defaultButtonName, 32, "BUTTON %d", buttonIndex + 1);
			BuildCtrlName(lpddoi, _Buttons[buttonIndex].Name, defaultButtonName);
			return DIENUM_CONTINUE;
		}
	}

	/////////////
	// Sliders //
	/////////////
	if (type & DIDFT_ABSAXIS)
	{
		if (_Sliders.size() < MaxNumSliders)
		{
			_Sliders.push_back(CSlider());
			uint sliderIndex = (uint)_Sliders.size() - 1;
			GetDIAxisRange(_Device, lpddoi->dwOfs, lpddoi->dwType, _Sliders[sliderIndex].Min, _Sliders[sliderIndex].Max);
			char defaultSliderName[32];
			smprintf(defaultSliderName, 32, "SLIDER %d", sliderIndex + 1);
			BuildCtrlName(lpddoi, _Sliders[sliderIndex].Name, defaultSliderName);
		}
		return DIENUM_CONTINUE;
	}


	//////////
	// POVs //
	//////////
	if (type & DIDFT_POV)
	{
		if (_POVs.size() < MaxNumPOVs)
		{
			_POVs.push_back(CPOV());
			uint povIndex = (uint)_POVs.size() - 1;
			char defaultPOVName[16];
			smprintf(defaultPOVName, 16, "POV %d", povIndex + 1);
			BuildCtrlName(lpddoi, _POVs[povIndex].Name, defaultPOVName);
		}
		return DIENUM_CONTINUE;
	}

	return DIENUM_CONTINUE;
}

//============================================================================
void	CDIGameDevice::querryControls()
{
	HRESULT r = _Device->EnumObjects(&DIEnumDeviceObjectsCallback, (LPVOID) this, DIDFT_ALL);
	nlassert(r == DI_OK);
}

//============================================================================
bool  CDIGameDevice::setBufferSize(uint size)
{
	// uisually not supported by this kind of devices
	return false;
}

//============================================================================
uint  CDIGameDevice::getBufferSize() const
{
	// uisually not supported by this kind of devices
	return 0;
}

//============================================================================
uint  CDIGameDevice::getNumButtons() const
{
	return (uint)_Buttons.size();
}

//============================================================================
bool		CDIGameDevice::hasAxis(TAxis axis) const
{
	nlassert(axis < MaxNumAxis);
	return _Axis[axis].Present;
}

//============================================================================
uint		CDIGameDevice::getNumSliders() const
{
	return (uint)_Sliders.size();
}

//============================================================================
uint		CDIGameDevice::getNumPOV() const
{
	return (uint)_POVs.size();
}
//============================================================================
const char *CDIGameDevice::getButtonName(uint index) const
{
	nlassert(index < _Buttons.size());
	return _Buttons[index].Name.c_str();
}

//============================================================================
const char *CDIGameDevice::getAxisName(TAxis axis) const
{
	nlassert(axis < MaxNumAxis);
	nlassert(hasAxis(axis)); // ! Not an axis of this device
	return  _Axis[axis].Name.c_str();
}

//============================================================================
const char *CDIGameDevice::getSliderName(uint index) const
{
	nlassert(index < _Sliders.size());
	return _Sliders[index].Name.c_str();
}

//============================================================================
const char *CDIGameDevice::getPOVName(uint index) const
{
	nlassert(index < _POVs.size());
	return _POVs[index].Name.c_str();
}

//============================================================================
bool		CDIGameDevice::getButtonState(uint index) const
{
	nlassert(index < _Buttons.size());
	return _Buttons[index].Pushed;
}

//============================================================================
float		CDIGameDevice::getAxisValue(TAxis axis) const
{
	nlassert(axis < MaxNumAxis);
	nlassert(hasAxis(axis)); // ! Not an axis of this device
	return  _Axis[axis].Value;
}

//============================================================================
float		CDIGameDevice::getSliderPos(uint index) const
{
	nlassert(index < _Sliders.size());
	return _Sliders[index].Pos;
}

//============================================================================
float		CDIGameDevice::getPOVAngle(uint index) const
{
	nlassert(index < _POVs.size());
	return _POVs[index].Angle;
}



} // NLMISC


#endif // NL_OS_WINDOWS
