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

#ifndef NL_CALLBACK_NET_BASE_H
#define NL_CALLBACK_NET_BASE_H

#undef USE_MESSAGE_RECORDER

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"

#include "buf_net_base.h"
#include "message.h"
#include "inet_address.h"

#ifdef USE_MESSAGE_RECORDER
#include "message_recorder.h"
#include <queue>
#endif

#include <vector>


namespace NLNET {

class CCallbackNetBase;

/** Callback function type for message processing
 *
 * msgin contains parameters of the message
 * from is the SockId of the connection, for a client, from is always the same value
 */
typedef void (*TMsgCallback) (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);


/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
typedef struct
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	const char		*Key;
	/// The callback function
	TMsgCallback	Callback;

} TCallbackItem;


/**
 * Layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CCallbackNetBase
{
public:

	virtual ~CCallbackNetBase() {}

	/** Set the user data */
	void setUserData(void *userData);

	/** Get the user data */
	void *getUserData();

	/** Sends a message to special connection.
	 * On a client, the hostid isn't used.
	 * On a server, you must provide a hostid. If you hostid = InvalidSockId, the message will be sent to all connected client.
	 */
	virtual void	send (const CMessage &buffer, TSockId hostid = InvalidSockId, bool log = true) = 0;

	uint64	getBytesSent () { return _BytesSent; }
	uint64	getBytesReceived () { return _BytesReceived; }

	virtual uint64	getReceiveQueueSize () = 0;
	virtual uint64	getSendQueueSize () = 0;

	virtual void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog) = 0;
	virtual void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog, TSockId destid = InvalidSockId) = 0;

	virtual void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog) = 0;

	/** Force to send all data pending in the send queue.
	 * On a client, the hostid isn't used and must be InvalidSockId
	 * On a server, you must provide a hostid.
	 * If you provide a non-null pointer for nbBytesRemaining, the value will be filled*
	 * will the number of bytes that still remain in the sending queue after the
	 * non-blocking flush attempt.
	 */
	virtual bool	flush (TSockId hostid = InvalidSockId, uint *nbBytesRemaining=NULL) = 0;

	/**	Appends callback array with the specified array. You can add callback only *after* adding the server or the client.
	 * \param arraysize is the number of callback items.
	 */
	void	addCallbackArray (const TCallbackItem *callbackarray, sint arraysize);

	/// Sets default callback for unknown message types
	void	setDefaultCallback(TMsgCallback defaultCallback) { _DefaultCallback = defaultCallback; }

	/// Set the pre dispatch callback. This callback is called before each message is dispatched
	void	setPreDispatchCallback(TMsgCallback predispatchCallback) { _PreDispatchCallback = predispatchCallback;}

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { _DisconnectionCallback = cb; _DisconnectionCbArg = arg; }

	/// returns the sockid of a connection. On a server, this function returns the parameter. On a client, it returns the connection.
	virtual TSockId	getSockId (TSockId hostid = InvalidSockId) = 0;

	/** Sets the callback that you want the other side calls. If it didn't call this callback, it will be disconnected
	 * If cb is NULL, we authorize *all* callback.
	 * On a client, the hostid must be InvalidSockId (or ommited).
	 * On a server, you must provide a hostid.
	 */
	void	authorizeOnly (const char *callbackName, TSockId hostid = InvalidSockId);

	/// Returns true if this is a CCallbackServer
	bool	isAServer () const { return _IsAServer; }

	/// This function is implemented in the client and server class
	virtual bool	dataAvailable () = 0;
	/// This function is implemented in the client and server class
	virtual bool	getDataAvailableFlagV() const = 0;
	/// This function is implemented in the client and server class
	virtual void	update2 ( sint32 timeout=0, sint32 mintime=0 ) = 0;
	/// This function is implemented in the client and server class (legacy)
	virtual void	update ( sint32 timeout=0 ) = 0;
	/// This function is implemented in the client and server class
	virtual bool	connected () const = 0;
	/// This function is implemented in the client and server class
	virtual void	disconnect (TSockId hostid = InvalidSockId) = 0;

	/// Returns the address of the specified host
	virtual const	CInetAddress& hostAddress (TSockId hostid);

	// Defined even when USE_MESSAGE_RECORDER is not defined
	enum TRecordingState { Off, Record, Replay };

protected:

	uint64	_BytesSent, _BytesReceived;

	/// Used by client and server class
	TNetCallback _NewDisconnectionCallback;

	/// Constructor.
	CCallbackNetBase( TRecordingState rec=Off, const std::string& recfilename="", bool recordall=true );

	/** Used by client and server class
	 * More info about timeout and mintime in the code.
	 */
	void baseUpdate2 ( sint32 timeout=-1, sint32 mintime=0 );

	/// Used by client and server class (legacy)
	void baseUpdate ( sint32 timeout=0 );

	/// Read a message from the network and process it
	void processOneMessage ();

	/// On this layer, you can't call directly receive, It s the update() function that receive and call your callaback
	virtual void	receive (CMessage &buffer, TSockId *hostid) = 0;

	// contains callbacks
	std::vector<TCallbackItem>	_CallbackArray;

	// called if the received message is not found in the callback array
	TMsgCallback				_DefaultCallback;

	// If not null, called before each message is dispached to it's callback
	TMsgCallback				_PreDispatchCallback;

	bool _IsAServer;
	bool _FirstUpdate;

	// ---------------------------------------
#ifdef USE_MESSAGE_RECORDER
	bool			replayDataAvailable();
	virtual bool	replaySystemCallbacks() = 0;
	void			noticeDisconnection( TSockId hostid );

	TRecordingState						_MR_RecordingState;
	sint64								_MR_UpdateCounter;

	CMessageRecorder					_MR_Recorder;
#endif
	// ---------------------------------------

private:

	void				*_UserData;

	NLMISC::TTime		_LastUpdateTime;
	NLMISC::TTime		_LastMovedStringArray;

	TNetCallback		 _DisconnectionCallback;
	void				*_DisconnectionCbArg;

	friend void cbnbMessageAskAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
	friend void cbnbMessageRecvAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);

	friend void cbnbNewDisconnection (TSockId from, void *data);
};


} // NLNET


#endif // NL_CALLBACK_NET_BASE_H

/* End of callback_net_base.h */
