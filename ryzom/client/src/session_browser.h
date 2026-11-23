// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/atomic.h"

#include <map>
#include <string>
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
	NLNET::CInetHost	_ServerAddr;

	// a flag to signal a failure in connection attempt
	bool				_SignalConnFail;

	// The auth information
	NLNET::CLoginCookie			_LoginCookie;

	// received message counter
	uint32						_NBReceivedMessage;

	// The name of the message waitOneMessage waits for (empty outside of it), and how many messages of
	// that name were received since the wait began
	std::string					_WaitedMessageName;
	uint32						_NbWaitedMessageReceived;

	// a flag set when we are waiting for a message. This bypass the
	// disconnection timeout
	bool						_WaitingForMessage;

	// Set by a wait that gave up (waitOneMessage), read by the comm thread under the mutex of the adaptor:
	// it closes the connection before it sends anything else
	NLMISC::CAtomicBool			_DropConnection;

	// The answers that waits which gave up are still owed, [message name] = how many. The messages carry
	// nothing that tells which request they answer, and the service sends an answer to the connection its
	// client has when the answer comes: one that comes late is not the answer to a later request.
	std::map<std::string, uint32>	_OwedAnswers;



	CCallbackClientAdaptor *getCallbackAdaptor();
private:

	/// clear all message queued in the send queue
	void clearSendQueue();

	// the comm thread entry point
	void run() NL_OVERRIDE;

	// called before each message is dispatched
	void on_preDispatchMessage(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);

	// closes the connection when a wait that gave up asked for it; true when it did
	bool dropConnectionIfAsked();

	// counts a received message against the answers owed to waits that gave up
	void settleOwedAnswer(const std::string &name);


protected:

	CSessionBrowser();
public:

	virtual ~CSessionBrowser() NL_OVERRIDE;

	/** Set auth info */
	void setAuthInfo(const NLNET::CLoginCookie &cookie);

	/** Send a message (Used to implement direct message to Dss
	*/
	void send(const NLNET::CMessage& msg);

	/** 'connect' the interface to the server, in fact, only store the
	 *	connection information, the connection is made only when the
	 *	client effectively send message.
	 */
	virtual void connectItf(NLNET::CInetHost address);

	// call update each frame
	void update() NL_OVERRIDE;


	// the connection attempt to the server has failed
	virtual void on_connectionFailed() =0;

	// The connection has been closed.
//	virtual void on_connectionClosed() =0;

	/** Wait until the client receive the specified message OR a disconnection is signaled, for 30 seconds
	 *	at most (WaitMessageTimeout).
	 *	The messages of other names that come during the wait are dispatched to their callbacks.
	 *	Return true if the specified message is received, false in case of disconnection or when the
	 *	time is over: the caller tells the player that the request got no answer. The values the
	 *	callback of the message stores are those of an earlier request then.
	 */
	bool waitOneMessage(const std::string &msgName);

	bool connected();

};




#endif //SESSION_BROWSER_H
