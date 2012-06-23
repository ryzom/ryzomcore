// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef __SERVICE_DEPENDENCIES__
#define __SERVICE_DEPENDENCIES__


class CServiceEvent
{
public:
	enum TType
	{
		SERVICE_UP = 0,
		SERVICE_DOWN
	};
	
	class CHandler
	{
	public:
		virtual ~CHandler() { }
		virtual void serviceEvent(CServiceEvent const& info) = 0;
	};
	
public:
	CServiceEvent(NLNET::TServiceId serviceId, std::string const& serviceName, TType const& eventType);
	virtual ~CServiceEvent() { }
	
	NLNET::TServiceId getServiceId() const { return _serviceId; }
	std::string const& getServiceName() const { return _serviceName; }
	TType const& getEventType()	const { return _eventType; }
	
private:
	NLNET::TServiceId const _serviceId;
	std::string		const _serviceName;
	TType		const _eventType;
};

inline
CServiceEvent::CServiceEvent(NLNET::TServiceId serviceId, std::string const& serviceName, TType const& eventType)
	:	_serviceId(serviceId),
		_serviceName(serviceName),
		_eventType(eventType)
{
}

#endif
