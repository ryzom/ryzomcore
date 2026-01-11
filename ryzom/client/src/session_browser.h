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

#ifndef SESSION_BROWSER_H
#define SESSION_BROWSER_H

#include "nel/misc/thread.h"
#include "nel/misc/singleton.h"
#include "nel/misc/mutex.h"
#include "game_share/ring_session_manager_itf.h"

/** Interface class to implement in order to be called back
 *	when events arrive on the session brownser.
 */
//class ISessionBrowserEvent
//{
//public:
//
//	/// The list of session is available
//	virtual void onSessionListReady(const std::vector<RSMGR::TSessionDesc> &sessions) =0;
//
//};

class CCallbackClientAdaptor;

/** Session browser requester.
 *	NB : all request are served asynchronously.
 *	You must derive this class and implement the
 *	virtuals to receive return messages.
 */
class CSessionBrowser : public RSMGR::CSessionBrowserServerWebClientItf,
						public NLMISC::IRunnable
{
	friend class CCallbackClientAdaptor;

	// the comm thread object
	NLMISC::IThread		*_CommThread;
	// the thread termination flag
	bool				_TerminateComm;

	// the server address for connection
	NLNET::CInetAddress	_ServerAddr;

	// a flag to signal a failure in connection attempt
	bool				_SignalConnFail;

	// The auth information
	NLNET::CLoginCookie			_LoginCookie;

	// received message counter
	uint32						_NBReceivedMessage;

	// A list of last received message name, in order of arrival
	std::list<std::string>		_LastReceivedMessageNames;

	// a flag set when we are waiting for a message. This bypass the
	// disconnection timeout
	bool						_WaitingForMessage;



	CCallbackClientAdaptor *getCallbackAdaptor();
private:

	/// clear all message queued in the send queue
	void clearSendQueue();

	// the comm thread entry point
	void run();

	// called before each message is dispatched
	void on_preDispatchMessage(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);


protected:

	CSessionBrowser();
public:

	virtual ~CSessionBrowser();

	/** Set auth info */
	void setAuthInfo(const NLNET::CLoginCookie &cookie);

	/** Send a message (Used to implement direct message to Dss
	*/
	void send(const NLNET::CMessage& msg);

	/** 'connect' the interface to the server, in fact, only store the
	 *	connection information, the connection is made only when the
	 *	client effectively send message.
	 */
	virtual void connectItf(NLNET::CInetAddress address);

	// call update each frame
	void update();


	// the connection attempt to the server has failed
	virtual void on_connectionFailed() =0;

	// The connection has been closed.
//	virtual void on_connectionClosed() =0;

	/** Wait until the client receive a message OR a disconnection is signaled
	 *	Return true if the specified message is received, false in case of
	 *	disconnection or if another message is received.
	 */
	bool waitOneMessage(const std::string &msgName);

	bool connected();

};




#endif //SESSION_BROWSER_H
