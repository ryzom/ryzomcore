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

#ifndef NL_GAME_DEVICE_EVENT_H
#define NL_GAME_DEVICE_EVENT_H

#include "types_nl.h"
#include "events.h"
#include "game_device.h"




namespace NLMISC
{

struct IMouseDevice;
struct IGameDevice;

const CClassId  EventGDMouseMove(0x12142bc4, 0x43c73e74);
const CClassId  EventGDButtonDownId(0x57141957, 0x3efb143a);
const CClassId  EventGDButtonUpId(0x16105e06, 0x302536b2);
const CClassId  EventGDAxisMovedId(0x073306, 0x41173626);
const CClassId  EventGDSliderMovedId(0x68776586, 0x394a6916);
const CClassId  EventGDPOVChanged(0x362851b9, 0x395c4d61);


//==========================================================================================
/// A raw mouse move message, expressed in mickeys (relative values)
class  CGDMouseMove : public CEvent
{
public:
	IMouseDevice *MD;
	sint X, Y;
public:
	CGDMouseMove(IEventEmitter *emitter, IMouseDevice *md, sint x, sint y) : CEvent(emitter, EventGDMouseMove), MD(md), X(x), Y(y)
	{}

	virtual	CEvent			*clone() const {return new CGDMouseMove(*this);}
};


//==========================================================================================
/**
 * An event from a game device (joystick, joypad ...)
 */
class CGameDeviceEvent : public CEvent
{
public:
	/// the game device this event come from
	IGameDevice *GameDevice;
public:
	CGameDeviceEvent(
					 IGameDevice *gameDevice,
					 IEventEmitter *emitter,
					 const CClassId &classId
					)
					: CEvent(emitter, classId),
					  GameDevice(gameDevice)
	{}
};


//==========================================================================================
/** A button state has changed
  */
class CGDButton : public CGameDeviceEvent
{
public:
	// index of the buttons that has been pushed
	uint ButtonIndex;
	bool Pushed;
public:
	///
	CGDButton(
			  uint buttonIndex,
			  bool pushed,
			  IGameDevice *gameDevice,
			  IEventEmitter *emitter,
			  const CClassId &classId
			 )
			 : CGameDeviceEvent(gameDevice, emitter, classId),
			   ButtonIndex(buttonIndex),
			   Pushed(pushed)
	{}
};


//==========================================================================================
/** A button has been pushed
  */
class CGDButtonDown : public CGDButton
{
public:
	///
	CGDButtonDown(uint buttonIndex, IGameDevice *gameDevice, IEventEmitter *emitter)
				 : CGDButton(buttonIndex, true, gameDevice, emitter, EventGDButtonDownId)
	{}

	virtual	CEvent			*clone() const {return new CGDButtonDown(*this);}
};

//==========================================================================================
/** A button has been released
  */
class CGDButtonUp : public CGDButton
{
public:
	///
	CGDButtonUp(uint buttonIndex, IGameDevice *gameDevice, IEventEmitter *emitter)
				 : CGDButton(buttonIndex, false, gameDevice, emitter, EventGDButtonUpId)
	{}

	virtual	CEvent			*clone() const {return new CGDButtonUp(*this);}
};

//==========================================================================================
/// An axis has moved
class CGDAxisMoved : public CGameDeviceEvent
{
public:
	IGameDevice::TAxis	Axis;
	// current position of the axis, ranges from -1.f to 1.f
	float				Value;
public:
	CGDAxisMoved(
				  IGameDevice::TAxis axis,
				  float	value,
				  IGameDevice *gameDevice,
				  IEventEmitter *emitter
				 )
				 : CGameDeviceEvent(gameDevice, emitter, EventGDAxisMovedId),
				   Axis(axis),
				   Value(value)
	{}

	virtual	CEvent			*clone() const {return new CGDAxisMoved(*this);}
};


//==========================================================================================
/// A slider position has changed
class CGDSliderMoved : public CGameDeviceEvent
{
public:
	uint SliderIndex;
	// current position of the slider, ranges from 0.f to 1.f
	float SliderPos;
public:
	CGDSliderMoved(
				  float	sliderPos,
				  uint sliderIndex,
				  IGameDevice *gameDevice,
				  IEventEmitter *emitter
				 )
				 : CGameDeviceEvent(gameDevice, emitter, EventGDSliderMovedId),
				   SliderIndex(sliderIndex),
				   SliderPos(sliderPos)
	{}

	virtual	CEvent			*clone() const {return new CGDSliderMoved(*this);}
};

//==========================================================================================
/// A point of view control changed
class CGDPOVChanged : public CGameDeviceEvent
{
public:
	uint  POVIndex;
	bool  Centered;
	// The POV angle, in degrees (CW)
	float POVAngle;
public:
	CGDPOVChanged(
				  bool  centered,
				  float	povAngle,
				  uint	povIndex,
				  IGameDevice *gameDevice,
				  IEventEmitter *emitter
				 )
				 : CGameDeviceEvent(gameDevice, emitter, EventGDPOVChanged),
				   POVIndex(povIndex),
				   Centered(centered),
				   POVAngle(povAngle)
	{}

	virtual	CEvent			*clone() const {return new CGDPOVChanged(*this);}
};


} // NLMISC


#endif // NL_GAME_DEVICE_EVENT_H

/* End of game_device_event.h */
