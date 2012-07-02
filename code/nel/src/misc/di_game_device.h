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

#ifndef NL_DI_GAME_DEVICE_H
#define NL_DI_GAME_DEVICE_H

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/di_event_emitter.h"
#include "nel/misc/game_device.h"

namespace NLMISC
{

//
typedef DIJOYSTATE2 CDIJoyState;
const   DIDATAFORMAT * const pJoyDataFormat = &c_dfDIJoystick2;
const	uint MaxNumSliders = 2;
const	uint MaxNumPOVs = 4;
const   uint MaxNumButtons = 128;
//

struct EDirectInputGameDeviceNotCreated : EDirectInput
{
	EDirectInputGameDeviceNotCreated() : EDirectInput("Unable to create a game device") {}
};


/**
 * Direct input implementation of a game device.
 */

class CDIGameDevice : public IGameDevice
{
public:
	/// Create a direct input game device from the given RGUID.	Destroy it with delete
	static CDIGameDevice *createGameDevice(IDirectInput8 *di8,
										   HWND hwnd,
										   CDIEventEmitter *diEventEmitter,
										   const CGameDeviceDesc &desc,
										   REFGUID rguid) throw(EDirectInput);
	~CDIGameDevice();

	///\name From IInputDevice
	//@{
		virtual bool		setBufferSize(uint size);
		virtual uint		getBufferSize() const;
	//@}

	///\name From IGameDevice
	//@{
		virtual const		CGameDeviceDesc &getDescription()  const { return _Desc; }
		//
		virtual	uint		getNumButtons() const;
		virtual bool		hasAxis(TAxis axis) const;
		virtual uint		getNumSliders() const;
		virtual	uint		getNumPOV() const;
		//
		virtual const char *getButtonName(uint index) const;
		virtual const char *getAxisName(TAxis axis) const;
		virtual const char *getSliderName(uint index) const;
		virtual const char *getPOVName(uint index) const;
		//
		virtual bool		getButtonState(uint index) const;
		virtual float		getAxisValue(TAxis axis) const;
		virtual float		getSliderPos(uint index) const;
		virtual float		getPOVAngle(uint index) const;
	//@}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
private:

	// base class for controls
	struct CCtrl
	{
		std::string Name;
	};

	// a button
	struct CButton	: public CCtrl
	{
		bool		Pushed;
		CButton() : Pushed(false) {}
	};

	// an axis. Its value either gives its position (-1 .. 1) or its angle (CCW in radians)
	struct CAxis	: public CCtrl
	{
		bool		Present; // is this axis used ?
		// min and max values from Direct Input
		sint        Min, Max;
		float		Value;
		CAxis() : Value(0.f), Present(false) {}
	};

	// a slider
	struct CSlider	: public CCtrl
	{
		sint        Min, Max;
		float Pos;
		CSlider() : Pos(0.f) {}
	};

	// a POV
	struct CPOV	: public CCtrl
	{
		bool  Centered;
		float Angle;
		CPOV() : Angle(0.f), Centered(true) {}
	};


private:
	// ctor
	CDIGameDevice();
	///\name From IInputDevice
	//@{
		virtual void begin(CEventServer *server);
		virtual void poll(CInputDeviceServer *dev);
		virtual void submit(IInputDeviceEvent *deviceEvent, CEventServer *server);
	//@}
	/**	 Get the controls (buttons, slider..) of this device from the Direct Input interface to build this object infos.
	  */
	void	querryControls();

	/// Called during EnumObject
	BOOL	processEnumObject(LPCDIDEVICEOBJECTINSTANCE lpddoi);
	friend  BOOL CALLBACK DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi,  LPVOID pvRef);
private:
	LPDIRECTINPUTDEVICE8	_Device;
	CGameDeviceDesc			_Desc;
	CDIEventEmitter			*_EventEmitter;

	///\name Device infos
	//@{
		CAxis					_Axis[MaxNumAxis];
		std::vector<CButton>	_Buttons;
		std::vector<CSlider>	_Sliders;
		std::vector<CPOV>		_POVs;
	//@}
	CDIJoyState					_CurrentState;

};


} // NLMISC

#endif // NL_OS_WINDOWS


#endif // NL_DI_GAME_DEVICE_H

/* End of di_play_device.h */
