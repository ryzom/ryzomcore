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



#include "stdpch.h"
#include "common_shard_callbacks.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "nel/net/unified_network.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager				PlayerManager;

/// Received an string-id association from IOS
void cbStoreStringResult( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	/*
	ucstring str;
	uint32 stringId;
	
	msgin.serial( str );
	msgin.serial( stringId );
	
	// put here code for other managers
	if (PlayerManager.setStringId(str, stringId))
		return;

	nlwarning("<cbStoreStringResult> unused string '%s'", str.toString().c_str() ); 
	*/
}

// received stall order form backup service due to server problem on writing player saves
void cbStallShard( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	string filename;
	msgin.serial( filename );

	nlwarning("Backup service send Stall order when trying write %s file", filename.c_str() );
	PlayerManager.broadcastMessage( 2, 0, 5, "Technical problem occured on the server,");
	PlayerManager.broadcastMessage( 2, 0, 5, "All non administrator accounts are disconnected immediately.");
	PlayerManager.broadcastMessage( 2, 0, 5, "Customer Support is already working on it.");
	PlayerManager.broadcastMessage( 2, 0, 5, "Sorry for any inconveniences.");
	PlayerManager.broadcastMessage( 2, 0, 5, "...");
	PlayerManager.setStallMode( true );
}

// received resume order from backup service
void cbResumeShard( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	PlayerManager.setStallMode( false );
	PlayerManager.broadcastMessage( 1, 0, 0, "Server resumed");
}



void	cbStallMode( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		bool		stalled;
		std::string	reason;

		msgin.serial(stalled, reason);

		if (stalled)
		{
			CMessage	msgout("HALT_TICK");
			msgout.serial(reason);

			nlwarning("Shard stalled by %s: %s", serviceName.c_str(), reason.c_str());

			CUnifiedNetwork::getInstance()->send("TICKS", msgout);
		}
		else
		{
			CMessage	msgout("RESUME_TICK");

			nlwarning("Shard resumed by %s: %s", serviceName.c_str(), reason.c_str());

			CUnifiedNetwork::getInstance()->send("TICKS", msgout);
		}
	}
	catch (const Exception&)
	{
	}

}



void CCommonShardCallbacks::init()
{
/*	/// the array of callbacks
	TUnifiedCallbackItem array[]=
	{
		{ "STORE_STRING_RESULT",					cbStoreStringResult },
		{ "STALL_SHARD",							cbStallShard		},
		{ "RESUME_SHARD",							cbResumeShard		},
	}; 
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( array, sizeof(array)/sizeof(array[0]) );
	*/

	/// the array of callbacks
	TUnifiedCallbackItem array[]=
	{
		{ "STALL_MODE",								cbStallMode			},
	}; 
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( array, sizeof(array)/sizeof(array[0]) );
}

void CCommonShardCallbacks::release()
{
}
