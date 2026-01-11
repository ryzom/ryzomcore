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



#ifndef SYNCHRONISED_MESSAGE_H
#define SYNCHRONISED_MESSAGE_H

#include <nel/net/transport_class.h>


void	sendMessageViaMirror( const std::string& destServiceName, NLNET::CMessage& msgout );
void	sendMessageViaMirror( NLNET::TServiceId destServiceId, NLNET::CMessage& msgout );
void	sendMessageViaMirrorToAll( NLNET::CMessage& msgout );


/**
 * Transport class synchronised with the mirror system
 */
class CMirrorTransportClass : public NLNET::CTransportClass
{
public:

	/// Send the transport class to a specified service using the service id
	void send( NLNET::TServiceId sid )
	{
		sendMessageViaMirror( sid, write() );
		//nldebug( "%u: Sending MTC to service %hu", CTickEventHandler::getGameCycle(), (uint16)sid );
	}

	/// Send the transport class to a specified service using the service name
	void send( const std::string &serviceName )
	{
		sendMessageViaMirror( serviceName, write() );
		//nldebug( "%u: Sending MTC to %s", CTickEventHandler::getGameCycle(), serviceName.c_str() );
	}

	// Send the transport class directly (with no synchronisation, using CTransportClass::send())
	//void sendAsync( uint8 sid ) { ((NLNET::CTransportClass*)(this))->send( sid ); }

	// Send the transport class directly (with no synchronisation, using CTransportClass::send())
	//void sendAsync( std::string serviceName ) { ((NLNET::CTransportClass*)(this))->send( serviceName ); }
};

#endif
