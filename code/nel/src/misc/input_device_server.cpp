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

#include "nel/misc/input_device_server.h"
#include "nel/misc/input_device.h"
#include "nel/misc/debug.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{
//=======================================================================
void	CInputDeviceServer::registerDevice(IInputDevice *device)
{
	nlassert(!isDevice(device));
	_Devices.push_back(device);
}

//=======================================================================
void	CInputDeviceServer::removeDevice(IInputDevice *device)
{
	TDeviceCont::iterator it = std::find(_Devices.begin(), _Devices.end(), device);
	nlassert(it != _Devices.end());
	_Devices.erase(it);
}

//=======================================================================
bool	CInputDeviceServer::isDevice(IInputDevice *device) const
{
	TDeviceCont::const_iterator it = std::find(_Devices.begin(), _Devices.end(), device);
	return it != _Devices.end();
}

//=======================================================================
// Predicate to compare vents dates
struct CInputDeviceEventLess
{
	bool operator()(const IInputDeviceEvent *lhs, const IInputDeviceEvent *rhs) const
	{
		return *lhs < *rhs;
	}
};

//=======================================================================
void	CInputDeviceServer::poll(CEventServer *server)
{
	nlassert(_Events.empty());
	TDeviceCont::iterator deviceIt;
	for (deviceIt = _Devices.begin(); deviceIt != _Devices.end(); ++deviceIt)
	{
		(*deviceIt)->begin(server);
		(*deviceIt)->poll(this);
	}
	// Sort the messages to get the right dates.
	std::sort(_Events.begin(), _Events.end(), CInputDeviceEventLess());
	// submit the result to the server
	IInputDevice *lastVisitedDevice = NULL;
	TEventCont::iterator eventIt;
	for (eventIt = _Events.begin(); eventIt != _Events.end(); ++eventIt)
	{
		// see if this message is from a previous device then the last we visited.
		if (lastVisitedDevice && (*eventIt)->Emitter != lastVisitedDevice)
		{
			// yes, tells that a transition occured
			lastVisitedDevice->transitionOccured(server, *eventIt);
			lastVisitedDevice = (*eventIt)->Emitter;
		}
		nlassert((*eventIt)->Emitter != NULL);
		(*eventIt)->Emitter->submit(*eventIt, server);
	}
	//
	for (deviceIt = _Devices.begin(); deviceIt != _Devices.end(); ++deviceIt)
	{
		(*deviceIt)->transitionOccured(server, NULL);
	}
	// delete the messages
	for (eventIt = _Events.begin(); eventIt != _Events.end(); ++eventIt)
	{
		delete *eventIt;
	}
	//
	_Events.clear();
}

//=======================================================================
void	CInputDeviceServer::submitEvent(IInputDeviceEvent *deviceEvent)
{
	_Events.push_back(deviceEvent);
}


} // NLMISC
