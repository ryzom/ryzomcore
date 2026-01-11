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


#ifndef CALLBACK_ADAPTER_H
#define CALLBACK_ADAPTER_H

#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"

class ICallbackServerAdaptor
{
	void					*_ContainerClass;
public:
	ICallbackServerAdaptor(void *containerClass)
		:	_ContainerClass(containerClass)
	{
	}

	virtual ~ICallbackServerAdaptor()
	{
	}

	void *getContainerClass()
	{
		return _ContainerClass;
	}

	virtual void addCallbackArray(const NLNET::TCallbackItem *callbackarray, sint arraysize) =0;
	virtual void setConnectionCallback(NLNET::TNetCallback cb, void *arg) =0;
	virtual void setDisconnectionCallback(NLNET::TNetCallback cb, void *arg) =0;
	virtual void init(uint16 port) =0;
	virtual void disconnect( NLNET::TSockId hostid) =0;
	virtual void send(const NLNET::CMessage &buffer, NLNET::TSockId hostid, bool log = true) =0;
	virtual void update() =0;
};

class ICallbackClientAdaptor
{
	void					*_ContainerClass;
public:
	ICallbackClientAdaptor(void *containerClass)
		:	_ContainerClass(containerClass)
	{
	}

	virtual ~ICallbackClientAdaptor()
	{
	}

	void *getContainerClass()
	{
		return _ContainerClass;
	}
	virtual void addCallbackArray(const NLNET::TCallbackItem *callbackarray, sint arraysize) =0;
	virtual void setDisconnectionCallback(NLNET::TNetCallback cb, void *arg) =0;
	virtual void connect( const NLNET::CInetAddress& addr ) =0;
	virtual bool connected(  ) =0;
	virtual void send(const NLNET::CMessage &buffer, NLNET::TSockId hostid = NLNET::InvalidSockId, bool log = true) =0;
	virtual void update() =0;
};

/** this is the default adaptor that make use of the NeL callback server
 */
class CNelCallbackServerAdaptor : public ICallbackServerAdaptor
{
protected:
	NLNET::CCallbackServer	_CallbackServer;

public:
	CNelCallbackServerAdaptor(void *containerClass)
		:	ICallbackServerAdaptor(containerClass)
	{
		_CallbackServer.setUserData(this);
	}

protected:
	virtual void addCallbackArray(const NLNET::TCallbackItem *callbackarray, sint arraysize)
	{
		_CallbackServer.addCallbackArray(callbackarray, arraysize);
	}
	virtual void setConnectionCallback(NLNET::TNetCallback cb, void *arg)
	{
		_CallbackServer.setConnectionCallback(cb, arg);
	}
	virtual void setDisconnectionCallback(NLNET::TNetCallback cb, void *arg)
	{
		_CallbackServer.setDisconnectionCallback(cb, arg);
	}
	virtual void init(uint16 port)
	{
		_CallbackServer.init(port);
	}
	virtual void disconnect( NLNET::TSockId hostid)
	{
		_CallbackServer.disconnect(hostid);
	}

	virtual void send(const NLNET::CMessage &buffer, NLNET::TSockId hostid, bool log = true)
	{
		_CallbackServer.send(buffer, hostid, log);
	}
	virtual void update()
	{
		_CallbackServer.update();
	}
};

/** this is the default adaptor that make use of the NeL callback client
 */
class CNelCallbackClientAdaptor : public ICallbackClientAdaptor
{
protected:
	NLNET::CCallbackClient	_CallbackClient;

public:
	CNelCallbackClientAdaptor(void *containerClass)
		:	ICallbackClientAdaptor(containerClass)
	{
		_CallbackClient.setUserData(this);
	}

protected:
	virtual void addCallbackArray(const NLNET::TCallbackItem *callbackarray, sint arraysize)
	{
		_CallbackClient.addCallbackArray(callbackarray, arraysize);
	}
	virtual void setDisconnectionCallback(NLNET::TNetCallback cb, void *arg)
	{
		_CallbackClient.setDisconnectionCallback(cb, arg);
	}
	virtual void connect( const NLNET::CInetAddress& addr )
	{
		_CallbackClient.connect(addr);
	}
	virtual bool connected()
	{
		return _CallbackClient.connected();
	}

	virtual void send(const NLNET::CMessage &buffer, NLNET::TSockId hostid = NLNET::InvalidSockId, bool log = true)
	{
		_CallbackClient.send(buffer, hostid, log);
	}
	virtual void update()
	{
		_CallbackClient.update();
	}
};


#endif // CALLBACK_ADAPTER_H
