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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/utils.h"

// local
#include "gus_net_remote_module.h"
#include "gus_net_connection.h"
#include "gus_net_messages.h"
#include "gus_net_hub.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// methods IRemoteModule
	//-----------------------------------------------------------------------------

	IRemoteModule::IRemoteModule()
	{
		static uint32 nextId=0;

		_UniqueId= nextId++;
	}

	IRemoteModule::~IRemoteModule()
	{
	}


	//-----------------------------------------------------------------------------
	// methods CRemoteModuleViaConnection
	//-----------------------------------------------------------------------------

	CRemoteModuleViaConnection::CRemoteModuleViaConnection()
	{
		_RemoteId= InvalidRemoteModuleId;
	}

	bool CRemoteModuleViaConnection::init(TConnectionModulePtr connection,CMsgRegisterModule& msg)
	{
		_Connection= connection;

		_RemoteId= msg.getModuleId();
		_Name= msg.getModuleName();
		_Parameters= msg.getParameters();

		return true;
	}

	const NLMISC::CSString CRemoteModuleViaConnection::getInfoString() const
	{
		return NLMISC::toString("%-32s Remote Module: %4u (%4u): %s %s",
			_Connection->getConnectionAddress().c_str(),_UniqueId,_RemoteId,_Name.c_str(),_Parameters.c_str());
	}

	const TConnectionModulePtr CRemoteModuleViaConnection::getConnection() const
	{
		return _Connection;
	}

	NLNET::TSockId CRemoteModuleViaConnection::getSockId() const
	{
		return _Connection->getCbClient()->getSockId();
	}

	uint32 CRemoteModuleViaConnection::getRemoteId() const
	{
		return _RemoteId;
	}

	uint32 CRemoteModuleViaConnection::getUniqueId() const
	{
		return _UniqueId;
	}

	const CSString& CRemoteModuleViaConnection::getName() const
	{
		return _Name;
	}	

	const CSString& CRemoteModuleViaConnection::getParameters() const
	{
		return _Parameters;
	}	

	void CRemoteModuleViaConnection::sendMessage(const CSString& msgName, TRawMsgBodyPtr msgBody,uint32 senderModuleId)
	{
		TModuleIdVector ids;
		ids.push_back(_RemoteId);
		_Connection->sendMessage(msgName,msgBody,ids,senderModuleId);
	}


	//-----------------------------------------------------------------------------
	// methods CRemoteModuleOnHub
	//-----------------------------------------------------------------------------

	CRemoteModuleOnHub::CRemoteModuleOnHub()
	{
		_SockId= NLNET::InvalidSockId;
		_RemoteId= InvalidRemoteModuleId;
	}

	bool CRemoteModuleOnHub::init(THubModulePtr hub,TSockId tsid,CMsgRegisterModule& msg)
	{
		_Hub= hub;
		_SockId= tsid;

		_RemoteId= msg.getModuleId();
		_Name= msg.getModuleName();
		_Parameters= msg.getParameters();

		return true;
	}

	void CRemoteModuleOnHub::buildDescriptionMsg(CMsgRegisterModule& msg)
	{
		msg.setup(_UniqueId,_Name,_Parameters);
	}

	const NLMISC::CSString CRemoteModuleOnHub::getInfoString() const
	{
		return NLMISC::toString("HUB: %-27s Remote Module: %4u (%4u): %s %s",
			_SockId->asString().c_str(),_UniqueId,_RemoteId,_Name.c_str(),_Parameters.c_str());
	}

	const THubModulePtr CRemoteModuleOnHub::getHub() const
	{
		return _Hub;
	}

	NLNET::TSockId CRemoteModuleOnHub::getSockId() const
	{
		return _SockId;
	}

	uint32 CRemoteModuleOnHub::getRemoteId() const
	{
		return _RemoteId;
	}

	uint32 CRemoteModuleOnHub::getUniqueId() const
	{
		return _UniqueId;
	}

	const CSString& CRemoteModuleOnHub::getName() const
	{
		return _Name;
	}	

	const CSString& CRemoteModuleOnHub::getParameters() const
	{
		return _Parameters;
	}	

	void CRemoteModuleOnHub::sendMessage(const CSString& msgName, TRawMsgBodyPtr msgBody,uint32 senderModuleId)
	{
		TModuleIdVector ids;
		ids.push_back(_RemoteId);
		_Hub->sendMessage(msgName,msgBody,_SockId,ids,senderModuleId);
	}
}

