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

#ifndef NL_INPUT_DEVICE_SERVER_H
#define NL_INPUT_DEVICE_SERVER_H

#include "types_nl.h"
#include <vector>


namespace NLMISC
{

class CEventServer;
struct IInputDevice;
struct IInputDeviceEvent;


/** Base class for an input device server. Unlike an event server, it manages several devices, and can sort their events (by date for example).
  * It keeps a list of active devices.
  * It can poll datas from every active device.
  * It can sort devices messages to submit them in correct order to a CEventServer.
  */

class CInputDeviceServer
{
public:
	/// register a device into this device server.
	void	registerDevice(IInputDevice *device);
	/// remove a device from this server (but does not delete it).
	void	removeDevice(IInputDevice *device);
	// returns the number of registered devices
	uint	getNumDevices() const { return (uint)_Devices.size(); }
	// return a device
	IInputDevice *getDevice(uint index) { return _Devices[index]; }
	/// Test whether the given device is handled by this server.
	bool	isDevice(IInputDevice *device) const;
	/// Retrieve datas from the devices, and submit them to the given CEventServer.
	void	poll(CEventServer *server);
	/// Allow an input device to register an event. The event will then be deleted by this server
	void	submitEvent(IInputDeviceEvent *deviceEvent);
	// dtor
	virtual ~CInputDeviceServer() {}
private:
	typedef	std::vector<IInputDevice *>		 TDeviceCont;
	typedef	std::vector<IInputDeviceEvent *> TEventCont;
private:
	TDeviceCont	_Devices;
	TEventCont  _Events;
};




/** An event from an input device.
  */
struct IInputDeviceEvent
{
	IInputDevice    *Emitter;	// the input device that emitted that event
	// Used to sort events by time stamp.
	virtual bool	operator < (const IInputDeviceEvent &IInputDeviceEvent) const = 0;
	virtual ~IInputDeviceEvent() {}
};


} // NLMISC


#endif // NL_INPUT_DEVICE_SERVER_H

/* End of input_device_server.h */
