// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019-2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "game_share/utils.h"
#include "session_browser.h"

#include "game_share/r2_share_itf.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;

/// The time the client stay connected to the session browser without requests
const uint32 KeepConnectedTime = 3;

/// The longest waitOneMessage waits for its message, in seconds. The wait runs in the main loop of the
/// client, which stands still for as long: this bounds the time a session browser service that is connected
/// and does not answer (or a session manager behind it that does not) keeps the client frozen. An answer
/// takes a fraction of a second; a connection that cannot be made is given up by the comm thread when the
/// system gives up on it, after about 20 seconds, which fits in this.
const uint32 WaitMessageTimeout = 30;

/// When a wait has given up, its answer can still come, in front of the answer to the next request of the
/// same kind (see _OwedAnswers). A later wait for that message looks for this long, in milliseconds, whether
/// another message of the name follows the first one, and takes the last.
const uint32 OwedAnswerGraceTime = 1000;

// a callback adaptor that queue the sent message for later sending.
class CCallbackClientAdaptor : public CNelCallbackClientAdaptor
{
	friend class CSessionBrowser;

	// a queue of message to be sent by the comm thread
	std::queue<CMessage>	_SendQueue;

	// The mutex used by the session browser thread
	NLMISC::CUnfairMutex	_Mutex;

	// A flag to let some message passthrue without enqueuing (for authenticate)
	// Mutex must already be locked during passthrough, as it's also bypassed
	NLMISC::CAtomicBool		_PassThrue;

	CCallbackClientAdaptor(void *containerClass)
		:	CNelCallbackClientAdaptor(containerClass),
			_PassThrue(false)
	{
		_CallbackClient.setPreDispatchCallback(&CCallbackClientAdaptor::cb_preDispatchMessage);
	}

	CCallbackClient &getCallback()
	{
		return _CallbackClient;
	}

	virtual void connect( const NLNET::CInetHost& /* addr */ ) NL_OVERRIDE
	{
		// do not connect now
	}


	virtual void send(const NLNET::CMessage &buffer, NLNET::TSockId hostid = NLNET::InvalidSockId, bool log = true) NL_OVERRIDE
	{
		if (!_PassThrue)
		{
			// queue the message for later sending.
			CAutoMutex<CUnfairMutex> mutex(_Mutex);
			nldebug("SB: Pushing a buffer into SendQueue (from %u elts)", _SendQueue.size());
			_SendQueue.push(buffer);
		}
		else
		{
			// protect socket because update() can be called in the main thread while send() can be called with passthru=true in the session browser thread (by authenticate() for example)
			_CallbackClient.send(buffer, hostid, log);
		}
	}

	// called by the callback client before each message dispatching
	static void cb_preDispatchMessage(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
	{
		CNelCallbackClientAdaptor *adaptor = static_cast<CNelCallbackClientAdaptor*>(netbase.getUserData());
		if (adaptor != nullptr)
		{
			CSessionBrowser *sb = static_cast<CSessionBrowser*>(adaptor->getContainerClass());

			sb->on_preDispatchMessage(msgin, from, netbase);
		}
	}


};



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

CSessionBrowser::CSessionBrowser()
	:	CSessionBrowserServerWebClientItf(new CCallbackClientAdaptor(this)),
		_TerminateComm(false),
		_SignalConnFail(false),
		_NBReceivedMessage(0),
		_NbWaitedMessageReceived(0),
		_WaitingForMessage(false),
		_DropConnection(false)
{
	// start the comm thread
	_CommThread = IThread::create(this);
	_CommThread->start();
}

CSessionBrowser::~CSessionBrowser()
{
	// signal the comm thread to terminate
	_TerminateComm = true;
	// wait the termination of the thread
	_CommThread->wait();

	// ok, we can leave
	delete _CommThread;
}

/** Set auth info */
void CSessionBrowser::setAuthInfo(const NLNET::CLoginCookie &cookie)
{
	_LoginCookie = cookie;
}

CCallbackClientAdaptor *CSessionBrowser::getCallbackAdaptor()
{
	return static_cast<CCallbackClientAdaptor *>(_CallbackClient.get());
}

void CSessionBrowser::connectItf(NLNET::CInetHost address)
{
	// call the interface connectItf
	CSessionBrowserServerWebClientItf::connectItf(address);
	_ServerAddr = address;
}

void CSessionBrowser::clearSendQueue()
{
	CCallbackClientAdaptor	*adaptor = getCallbackAdaptor();
	nldebug("SB: Clearing SendQueue");
	CAutoMutex<CUnfairMutex> mutex(adaptor->_Mutex);
	while (!adaptor->_SendQueue.empty())
		adaptor->_SendQueue.pop();
}

// the comm thread entry point
void CSessionBrowser::run()
{
	CCallbackClientAdaptor	*adaptor = getCallbackAdaptor();
	while(!_TerminateComm)
	{
		// no server address available
		while (!_ServerAddr.isValid())
		{
			// wait a little
			nlSleep(100);

			if (_TerminateComm)
				break;
		}

disconnected:
		// disconnected: there is no connection that a wait could have given up on
		_DropConnection = false;
		// waiting for message to send
		while (adaptor->_SendQueue.empty())
		{
			nlSleep(100);

			if (_TerminateComm)
				break;
		}

retry_connection:
		while (!adaptor->getCallback().connected())
		{
			// connecting...
			try
			{
				nldebug("SB: Connecting...");
				adaptor->getCallback().connect(_ServerAddr);
			}
			catch(...)
			{
				// connection failed !
				// erase all queued messages
				clearSendQueue();
				// put an event in for the main loop
				_SignalConnFail = true;
			}
			if (_TerminateComm)
				break;

		}

		{
			CAutoMutex<CUnfairMutex> mutex(adaptor->_Mutex);
			// authenticate the client
			 adaptor->_PassThrue = true;
			nldebug("SB: Authenticating");
			// nico patch
			{
				if (!connected()) // oddly sometimes it gets disconnected here
				{
					goto retry_connection;
				}
			}
			// a wait gave up while this connection was being made: nothing was sent on it
			if (dropConnectionIfAsked())
			{
				adaptor->_PassThrue = false;
				goto disconnected;
			}
			authenticate(_LoginCookie.getUserId(), _LoginCookie);
			adaptor->_PassThrue = false;
		}
sendMessages:
		// connected, sending messages
		while (!adaptor->_SendQueue.empty())
		{
			bool popped = false;
			{
				CAutoMutex<CUnfairMutex> mutex(adaptor->_Mutex);
				if (!adaptor->getCallback().connected())
				{
					nldebug("SB: Disconnecting when authenticating");
					clearSendQueue(); // TODO: REMOVE?
					goto disconnected;
				}

				// A wait gave up on a request sent on this connection: the next request goes out on a new
				// one, and what still comes in on this one is not received
				if (dropConnectionIfAsked())
					goto disconnected;

				nldebug("SB: Sending next buffer of SendQueue");
				adaptor->getCallback().send(adaptor->_SendQueue.front());
				//if (adaptor->getCallback().connected()) // don't loose msg if it has not been sent (TODO: ADD?)
				{
					adaptor->_SendQueue.pop();
					popped = true;
				}
			}
			if (popped)
				nldebug("SB: Popping from SendQueue (now %u elts)", adaptor->_SendQueue.size());
			else
				nldebug("SB: Disconnected when sending");

			if (_TerminateComm)
				break;
		}

		// idle, waiting new message to send/receive or time out to disconnect
		time_t startWait = CTime::getSecondsSince1970();
		while (_WaitingForMessage || (adaptor->_SendQueue.empty() && CTime::getSecondsSince1970() - startWait < KeepConnectedTime))
		{
			nlSleep(100);

			if (_TerminateComm)
				break;

			{
				CAutoMutex<CUnfairMutex> mutex(adaptor->_Mutex);
				if (dropConnectionIfAsked())
					goto disconnected;

				if (!adaptor->_SendQueue.empty())
					goto sendMessages;

				if (!adaptor->getCallback().connected())
					goto disconnected;
			}
		}

		{
			nldebug("SB: Disconnecting");
			CAutoMutex<CUnfairMutex> mutex(adaptor->_Mutex);
//			if (!adaptor->_SendQueue.empty())
				// timeout, disconnect
				adaptor->getCallback().disconnect();
		}

		if (_TerminateComm)
			break;

		goto disconnected;
	}
}

// True when a message of the name 'received' is the answer a wait for 'waited' waits for
static bool answersWait(const std::string &received, const std::string &waited)
{
	// hardcoded: JSSRE may replace JSSR in some responses
	return received == waited || (received == "JSSRE" && waited == "JSSR");
}

// Called by the comm thread with the mutex of the adaptor held, before it uses the connection
bool CSessionBrowser::dropConnectionIfAsked()
{
	if (!_DropConnection)
		return false;
	nldebug("SB: Dropping the connection of a wait that gave up");
	// (this empties the receive queue as well)
	getCallbackAdaptor()->getCallback().disconnect();
	_DropConnection = false;
	return true;
}

// A message that is the answer to no wait came in: when a wait that gave up is owed one of its name, this is it
void CSessionBrowser::settleOwedAnswer(const std::string &name)
{
	for (std::map<std::string, uint32>::iterator it = _OwedAnswers.begin(); it != _OwedAnswers.end(); ++it)
	{
		if (answersWait(name, it->first))
		{
			nlinfo("SB: the '%s' message a wait gave up on came after all, and is not taken for an answer", name.c_str());
			if (--it->second == 0)
				_OwedAnswers.erase(it);
			return;
		}
	}
}

// (called from update, on the main thread, with the mutex of the adaptor held)
void CSessionBrowser::on_preDispatchMessage(CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */)
{
	++_NBReceivedMessage;
	const std::string name = msgin.getName();
	if (_DropConnection)
	{
		// it came in on a connection that a wait has given up on, and that the comm thread is about to close
		settleOwedAnswer(name);
	}
	else if (_WaitingForMessage && answersWait(name, _WaitedMessageName))
	{
		++_NbWaitedMessageReceived;
	}
	else
	{
		settleOwedAnswer(name);
	}
}


void CSessionBrowser::update()
{
	// check flag to callback a connection failure event
	if (_SignalConnFail)
	{
		on_connectionFailed();
		_SignalConnFail = false;
	}

	//nldebug("SB: Updating module");
	CAutoMutex<CUnfairMutex> mutex(getCallbackAdaptor()->_Mutex);
	// call the underlying interface update
	CSessionBrowserServerWebClientItf::update();
}

bool CSessionBrowser::waitOneMessage(const std::string &msgName)
{
	nldebug("SB: Waiting for message '%s'", msgName.c_str());
	// the messages of this name are counted from here on (on_preDispatchMessage)
	_WaitedMessageName = msgName;
	_NbWaitedMessageReceived = 0;
	_WaitingForMessage = true;

	CCallbackClientAdaptor	*adaptor = getCallbackAdaptor();

	// the answers that earlier waits for this message gave up on: they can still come, in front of this one
	uint32 owed = 0;
	{
		std::map<std::string, uint32>::const_iterator it = _OwedAnswers.find(msgName);
		if (it != _OwedAnswers.end())
			owed = it->second;
	}

	// Loop while the message is not received, still connected or having message to send, for
	// WaitMessageTimeout at most. A message of another name that comes first is no answer to this wait: it is
	// dispatched as it would be in any frame (the list of the player control window, asked for every few
	// seconds), and the wait goes on.
	const TTime start = CTime::getLocalTime();
	TTime firstReceived = 0;
	bool hasReceived = false;
	bool timedOut = false;
	for (;;)
	{
		const TTime now = CTime::getLocalTime();
		if (_NbWaitedMessageReceived > owed)
		{
			// the message of this wait is in, behind every one that was owed
			break;
		}
		if (_NbWaitedMessageReceived != 0)
		{
			// As many messages of the name as are owed, or fewer: they are the owed ones, or the last is this
			// wait's own and owed ones were lost (the service has no one to send an answer to while the client
			// is not connected). Owed answers come from the same queue of the session manager, right in front
			// of this wait's own: the last message of the grace time is taken.
			if (!hasReceived)
			{
				hasReceived = true;
				firstReceived = now;
			}
			if (now - firstReceived >= (TTime)OwedAnswerGraceTime)
				break;
		}
		else if (!adaptor->getCallback().connected() && adaptor->_SendQueue.empty())
		{
			// the connection was lost, or could not be made
			break;
		}
		if (now - start >= (TTime)WaitMessageTimeout * 1000)
		{
			timedOut = _NbWaitedMessageReceived == 0;
			break;
		}

		update();

		// give up the time slice
		nlSleep(0);

	}

	_WaitingForMessage = false;
	_WaitedMessageName.clear();

	if (timedOut)
	{
		// The service is connected, or being connected, and silent. What this wait leaves behind must not be
		// taken for the answer to a later request: the requests that are not sent yet are not sent at all,
		// and the comm thread closes the connection before it sends anything else, so that nothing more is
		// received on it (the service sends an answer to the connection its client has at that time, and
		// drops it when the client has none). An answer can still reach the next connection, when the
		// session manager gets to it after the client has connected again: it is owed, and counted.
		bool sent;
		{
			CAutoMutex<CUnfairMutex> mutex(adaptor->_Mutex);
			sent = adaptor->_SendQueue.empty();
			while (!adaptor->_SendQueue.empty())
				adaptor->_SendQueue.pop();
			_DropConnection = true;
		}
		if (sent)
			++_OwedAnswers[msgName];
		nlwarning("SB: no '%s' message from the session browser in %u seconds: the wait gives up, the connection is dropped (request %s)",
		    msgName.c_str(), WaitMessageTimeout, sent ? "sent, its answer can still come" : "not sent");
		return false;
	}

	// false: the connection was lost, or could not be made, before the message came
	if (_NbWaitedMessageReceived == 0)
		return false;

	// what was owed has come in front of this answer, or is taken as lost
	_OwedAnswers.erase(msgName);
	return true;
}

bool CSessionBrowser::connected()
{
	return getCallbackAdaptor()->connected();
}

void CSessionBrowser::send(const NLNET::CMessage& msg)
{
	getCallbackAdaptor()->send(msg);
}
