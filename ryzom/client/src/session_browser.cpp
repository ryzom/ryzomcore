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
		_WaitingForMessage(false)
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
		// disconnected, waiting for message to send
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

void CSessionBrowser::on_preDispatchMessage(CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */)
{
	++_NBReceivedMessage;
	if (_WaitingForMessage && answersWait(msgin.getName(), _WaitedMessageName))
		++_NbWaitedMessageReceived;
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


	// Loop while the message is not received, still connected or having message to send. A message of another
	// name that comes first is no answer to this wait: it is dispatched as it would be in any frame (the list
	// of the player control window, asked for every few seconds), and the wait goes on.
	while (_NbWaitedMessageReceived == 0
		&& (adaptor->getCallback().connected() || !adaptor->_SendQueue.empty()))
	{
		update();

		// give up the time slice
		nlSleep(0);

	}

	_WaitingForMessage = false;
	_WaitedMessageName.clear();

	// false: the connection was lost, or could not be made, before the message came
	return _NbWaitedMessageReceived != 0;
}

bool CSessionBrowser::connected()
{
	return getCallbackAdaptor()->connected();
}

void CSessionBrowser::send(const NLNET::CMessage& msg)
{
	getCallbackAdaptor()->send(msg);
}
