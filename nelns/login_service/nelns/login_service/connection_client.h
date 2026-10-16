// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_CONNECTION_CLIENT_H
#define NL_CONNECTION_CLIENT_H

#include <map>
#include <memory>

#include <nel/net/buf_net_base.h>
#include <nel/net/callback_net_base.h>
#include <nel/net/callback_server.h>
#include <nel/net/unified_network.h>
#include <nel/net/message.h>

#include <nelns/login_service/persistence.h>

class ConnectionClient
{
public:
	explicit ConnectionClient(IPersistence& persistence);

	~ConnectionClient() = default;

	void update();
	void sendToClient(NLNET::CMessage &msgout, NLNET::TSockId sockId);

private:
	void cbClientVerifyLoginPassword(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	void cbClientChooseShard(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);
	void cbClientConnection(NLNET::TSockId from);
	void cbClientDisconnection(NLNET::TSockId from);

	void cbWSShardChooseShard(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);

	// The cookie's address field carries a per-connection id, and the reply
	// routing looks the socket up again through these maps. Storing the
	// TSockId pointer itself in the 32 bit field truncated it on 64 bit
	// builds, and a stale value could match another client's connection.
	uint32 allocateConnectionId(NLNET::TSockId from);
	uint32 connectionId(NLNET::TSockId from) const;
	NLNET::TSockId connectionById(uint32 id) const;
	void releaseConnectionId(NLNET::TSockId from);

	IPersistence& persistence;
	NLNET::CCallbackServer ClientsServer;

	uint32 m_ConnectionIdCounter = 0;
	std::map<uint32, NLNET::TSockId> m_IdToSock;
	std::map<NLNET::TSockId, uint32> m_SockToId;
};

#endif // NL_CONNECTION_CLIENT_H
