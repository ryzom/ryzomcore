// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdnet.h"

#include "nel/misc/hierarchical_timer.h"

#include "nel/net/buf_sock.h"
#include "nel/net/callback_net_base.h"
#include "nel/net/net_log.h"



#ifdef USE_MESSAGE_RECORDER
#ifdef NL_OS_WINDOWS
#pragma message ( "NeL Net layer 3: message recorder enabled" )
#endif // NL_OS_WINDOWS
#include "nel/net/message_recorder.h"
#else
#ifdef NL_OS_WINDOWS
#pragma message ( "NeL Net layer 3: message recorder disabled" )
#endif // NL_OS_WINDOWS
#endif


using namespace std;
using namespace NLMISC;

namespace NLNET {


/*
 * Disconnection callback
 */
void cbnbNewDisconnection (TSockId from, void *data)
{
	nlassert (data != NULL);
	CCallbackNetBase *base = (CCallbackNetBase *)data;

	LNETL3_DEBUG("LNETL3NB: cbnbNewDisconnection()");

#ifdef USE_MESSAGE_RECORDER
	// Record or replay disconnection
	base->noticeDisconnection( from );
#endif

	// Call the client callback if necessary
	if (base->_DisconnectionCallback != NULL)
		base->_DisconnectionCallback (from, base->_DisconnectionCbArg);
}


/*
 * Constructor
 */
CCallbackNetBase::CCallbackNetBase(  TRecordingState /* rec */, const string& /* recfilename */, bool /* recordall */ )
	:	_BytesSent(0),
		_BytesReceived(0),
		_NewDisconnectionCallback(cbnbNewDisconnection),
		_DefaultCallback(NULL),
		_PreDispatchCallback(NULL),
		_FirstUpdate (true),
		_UserData(NULL),
		_DisconnectionCallback(NULL),
		_DisconnectionCbArg(NULL)
#ifdef USE_MESSAGE_RECORDER
		, _MR_RecordingState(rec), _MR_UpdateCounter(0)
#endif
{
	createDebug(); // for addNegativeFilter to work even in release and releasedebug modes

#ifdef USE_MESSAGE_RECORDER
	switch ( _MR_RecordingState )
	{
	case Record :
		_MR_Recorder.startRecord( recfilename, recordall );
		break;
	case Replay :
		_MR_Recorder.startReplay( recfilename );
		break;
	default:;
		// No recording
	}
#endif
}

/** Set the user data */
void CCallbackNetBase::setUserData(void *userData)
{
	_UserData = userData;
}

/** Get the user data */
void *CCallbackNetBase::getUserData()
{
	return _UserData;
}

/*
 *	Append callback array with the specified array
 */
void CCallbackNetBase::addCallbackArray (const TCallbackItem *callbackarray, sint arraysize)
{
	if (arraysize == 1 && callbackarray[0].Callback == NULL && strlen(callbackarray[0].Key) == 0)
	{
		// it's an empty array, ignore it
		return;
	}

	// resize the array
	sint oldsize = (sint)_CallbackArray.size();

	_CallbackArray.resize (oldsize + arraysize);

//TOO MUCH MESSAGE	nldebug ("L3NB_CB: Adding %d callback to the array", arraysize);

	for (sint i = 0; i < arraysize; i++)
	{
		sint ni = oldsize + i;
//TOO MUCH MESSAGE		nldebug ("L3NB_CB: Adding callback to message '%s', id '%d'", callbackarray[i].Key, ni);
		// copy callback value

		_CallbackArray[ni] = callbackarray[i];

	}


//	LNETL3_DEBUG ("LNETL3NB_CB: Added %d callback Now, there're %d callback associated with message type", arraysize, _CallbackArray.size ());
}


/*
 * processOneMessage()
 */
void CCallbackNetBase::processOneMessage ()
{
	// slow down the layer H_AUTO (CCallbackNetBase_processOneMessage);

	CMessage msgin ("", true);
	TSockId tsid;
	try
	{
		receive (msgin, &tsid);
	}
	catch (const Exception &e)
	{
		nlwarning(e.what());
		return;
	}

	_BytesReceived += msgin.length ();

	// now, we have to call the good callback
	sint pos = -1;
	std::string name = msgin.getName ();
	sint i;
	for (i = 0; i < (sint)_CallbackArray.size (); i++)
	{
		if (name == _CallbackArray[i].Key)
		{
			pos = i;
			break;
		}
	}

	TMsgCallback	cb = NULL;
	if (pos < 0 || pos >= (sint16) _CallbackArray.size ())
	{
		if (_DefaultCallback == NULL)
		{
			nlwarning ("LNETL3NB_CB: Callback %s not found in _CallbackArray", msgin.toString().c_str());
		}
		else
		{
			cb = _DefaultCallback;
		}
	}
	else
	{
		cb = _CallbackArray[pos].Callback;
	}

	TSockId realid = getSockId (tsid);

	if (!realid->AuthorizedCallback.empty() && msgin.getName() != realid->AuthorizedCallback)
	{
		nlwarning ("LNETL3NB_CB: %s try to call the callback %s but only %s is authorized. Disconnect him!", tsid->asString().c_str(), msgin.toString().c_str(), tsid->AuthorizedCallback.c_str());
		disconnect (tsid);
	}
	else if (cb == NULL)
	{
		nlwarning ("LNETL3NB_CB: Callback %s is NULL, can't call it", msgin.toString().c_str());
	}
	else
	{
		LNETL3_DEBUG ("LNETL3NB_CB: Calling callback (%s)%s", msgin.getName().c_str(), (cb==_DefaultCallback)?" DEFAULT_CB":"");

		if (_PreDispatchCallback != NULL)
		{
			// call the pre dispatch callback
			_PreDispatchCallback(msgin, realid, *this);
		}
		cb(msgin, realid, *this);
	}

/*
	if (pos < 0 || pos >= (sint16) _CallbackArray.size ())
	{
		if (_DefaultCallback == NULL)
			nlwarning ("LNETL3NB_CB: Callback %s not found in _CallbackArray", msgin.toString().c_str());
		else
		{
			// ...
		}
	}
	else
	{
		TSockId realid = getSockId (tsid);

		if (!realid->AuthorizedCallback.empty() && msgin.getName() != realid->AuthorizedCallback)
		{
			nlwarning ("LNETL3NB_CB: %s try to call the callback %s but only %s is authorized. Disconnect him!", tsid->asString().c_str(), msgin.toString().c_str(), tsid->AuthorizedCallback.c_str());
			disconnect (tsid);
		}
		else if (_CallbackArray[pos].Callback == NULL)
		{
			nlwarning ("LNETL3NB_CB: Callback %s is NULL, can't call it", msgin.toString().c_str());
		}
		else
		{
			LNETL3_DEBUG ("LNETL3NB_CB: Calling callback (%s)", _CallbackArray[pos].Key);
			_CallbackArray[pos].Callback (msgin, realid, *this);
		}
	}
*/
}


/*
 * baseUpdate
 * Recorded : YES
 * Replayed : YES
 */
void CCallbackNetBase::baseUpdate (sint32 timeout)
{
	H_AUTO(L3UpdateCallbackNetBase);
#ifdef NL_DEBUG
	nlassert( timeout >= -1 );
#endif

	TTime t0 = CTime::getLocalTime();

	//
	// The first time, we init time counters
	//
	if (_FirstUpdate)
	{
//		LNETL3_DEBUG("LNETL3NB: First update()");
		_FirstUpdate = false;
		_LastUpdateTime = t0;
		_LastMovedStringArray = t0;
	}

	/*
	 * timeout -1    =>  read one message in the queue or nothing if no message in queue
	 * timeout 0     =>  read all messages in the queue
	 * timeout other =>  read all messages in the queue until timeout expired (at least all one time)
	 */

	bool exit = false;

	while (!exit)
	{
		// process all messages in the queue
		while (dataAvailable ())
		{
			processOneMessage ();
			if (timeout == -1)
			{
				exit = true;
				break;
			}
		}

		// need to exit?
		if (timeout == 0 || (sint32)(CTime::getLocalTime() - t0) > timeout)
		{
			exit = true;
		}
		else
		{
			// enable multithreading on windows :-/
			// slow down the layer H_AUTO (CCallbackNetBase_baseUpdate_nlSleep);
			nlSleep (10);
		}
	}

#ifdef USE_MESSAGE_RECORDER
	_MR_UpdateCounter++;
#endif

}


/*
 * baseUpdate
 * Recorded : YES
 * Replayed : YES
 */
void CCallbackNetBase::baseUpdate2 (sint32 timeout, sint32 mintime)
{
	H_AUTO(L3UpdateCallbackNetBase2);
#ifdef NL_DEBUG
	nlassert( timeout >= -1 );
#endif

	TTime t0 = CTime::getLocalTime();

	//
	// The first time, we init time counters
	//
	if (_FirstUpdate)
	{
//		LNETL3_DEBUG("LNETL3NB: First update()");
		_FirstUpdate = false;
		_LastUpdateTime = t0;
		_LastMovedStringArray = t0;
	}

	/*
	 * Controlling the blocking time of this update loop:
	 *
	 * "GREEDY" MODE (legacy default mode)
	 *   timeout -1   => While data are available, read all messages in the queue,
	 *   mintime t>=0    return only when the queue is empty (and mintime is reached/exceeded).
	 *
	 * "CONSTRAINED" MODE (used by layer 5 with mintime=0)
	 *   timeout t>0  => Read messages in the queue, exit when the timeout is reached/exceeded,
	 *   mintime t>=0    or when there are no more data (and mintime is reached/exceeded).
	 *
	 * "ONE-SHOT"/"HARE AND TORTOISE" MODE
	 *   timeout 0    => Read up to one message in the queue (nothing if empty), then return at
	 *   mintime t>=0    once, or, if mintime not reached, sleep (yield cpu) and start again.
	 *
	 * Backward compatibility:		baseUpdate():		To have the same behaviour with baseUpdate2():
	 *   Warning! The semantics		timeout t>0			timeout 0, mintime t>0
	 *   of the timeout argument	timeout -1			timeout 0, mintime 0
	 *   has changed				timeout 0			timeout -1, mintime 0
	 *
	 * About "Reached/exceeded"
	 *   This loop does not control the time of the user-code in the callback. Thus if some data
	 *   are available, the callback may take more time than specified in timeout. Then the loop
	 *   will end when the timeout is "exceeded" instead of "reached". When yielding cpu, some
	 *   more time than specified may be spent as well.
	 *
	 * Flooding Detection Option (TODO)
	 *   _FloodingByteLimit => If setFloodingDetection() has been called, and the size of the
	 *                    queue exceeds the flooding limit, the connection will be dropped and
	 *                    the loop will return immediately.
	 *
	 * Message Frame Option (TODO)
 	 *   At the beginning of the loop, the number of pending messages would be read, and then
	 *   only these messages would be processed in this loop, no more. As a result, any messages
	 *   received in the meantime would be postponed until the next call.
	 *   However, to do this we need to add a fast method getMsgNb() in NLMISC::CBufFifo
	 *   (see more information in the class header of CBufFifo).
	 *
	 * Implementation notes:
	 *   As CTime::getLocalTime() may be slow on certain platforms, we test it only
	 *   if timeout > 0 or mintime > 0.
	 *   As CTime::getLocalTime() is not an accurate time measure (ms unit, resolution on several
	 *   ms on certain platforms), we compare with timeout & mintime using "greater or equal".
	 *
	 * Testing:
	 *  See nel\tools\nel_unit_test\net_ut\layer3_test.cpp
	 */

//	// TODO: Flooding Detection Option (would work best with the Message Frame Option, otherwise
//	// we won't detect here a flooding that would occur during the loop.
//	if ( _FloodingDetectionEnabled )
//	{
//		if ( getDataAvailableFlagV() )
//		{
//			uint64 nbBytesToHandle = getReceiveQueueSize(); // see above about a possible getMsgNb()
//			if ( nbBytesToHandle > _FloodingByteLimit )
//			{
//				// TODO: disconnect
//			}
//		}
//	}

	// Outer loop
	while ( true )
	{
		// Inner loop
		while ( dataAvailable () )
		{
			processOneMessage();

			// ONE-SHOT MODE/"HARE AND TORTOISE" (or CONSTRAINED with no more time): break
			if ( timeout == 0 )
				break;
			// CONTRAINED MODE: break if timeout reached even if more data are available
			if ( (timeout > 0) && ((sint32)(CTime::getLocalTime() - t0) >= timeout) )
				break;
			// GREEDY MODE (timeout -1): loop until no more data are available
		}

		// If mintime provided, loop until mintime reached, otherwise exit
		if ( mintime == 0 )
			break;
		if (((sint32)(CTime::getLocalTime() - t0) >= mintime))
			break;
		nlSleep( 0 ); // yield cpu
	}

#ifdef USE_MESSAGE_RECORDER
	_MR_UpdateCounter++;
#endif

}


const	CInetAddress& CCallbackNetBase::hostAddress (TSockId /* hostid */)
{
	// should never be called
	nlstop;
	static CInetAddress tmp;
	return tmp;
}

void	CCallbackNetBase::authorizeOnly (const char *callbackName, TSockId hostid)
{
	LNETL3_DEBUG ("LNETL3NB: authorizeOnly (%s, %s)", callbackName, hostid->asString().c_str());

	hostid = getSockId (hostid);

	nlassert (hostid != InvalidSockId);

	hostid->AuthorizedCallback = (callbackName == NULL)?"":callbackName;
}


#ifdef USE_MESSAGE_RECORDER

/*
 * Replay dataAvailable() in replay mode
 */
bool CCallbackNetBase::replayDataAvailable()
{
	nlassert( _MR_RecordingState == Replay );

	if ( _MR_Recorder.ReceivedMessages.empty() )
	{
		// Fill the queue of received messages related to the present update
		_MR_Recorder.replayNextDataAvailable( _MR_UpdateCounter );
	}

	return replaySystemCallbacks();
}


/*
 * Record or replay disconnection
 */
void CCallbackNetBase::noticeDisconnection( TSockId hostid )
{
	nlassert (hostid != InvalidSockId);	// invalid hostid
	if ( _MR_RecordingState != Replay )
	{
		if ( _MR_RecordingState == Record )
		{
			// Record disconnection
			CMessage emptymsg;
			_MR_Recorder.recordNext( _MR_UpdateCounter, Disconnecting, hostid, emptymsg );
		}
	}
	else
	{
		// Replay disconnection
		hostid->disconnect( false );
	}
}

#endif // USE_MESSAGE_RECORDER

} // NLNET

