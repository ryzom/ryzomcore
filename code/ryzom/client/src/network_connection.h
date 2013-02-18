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



#ifndef NL_NETWORK_CONNECTION_H
#define NL_NETWORK_CONNECTION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_set.h"

#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>

#include <nel/misc/bit_mem_stream.h>
#include <nel/net/inet_address.h>
#include <nel/net/login_cookie.h>
#include <nel/net/login_client.h>
#include <nel/net/udp_sim_sock.h>
#include <nel/misc/vectord.h>
#include <nel/misc/md5.h>

#include "time_client.h"
#include "game_share/entity_types.h"
#include "property_decoder.h"
#include "impulse_decoder.h"
#include "game_share/action.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/action_block.h"
#include "game_share/loot_harvest_state.h"

/*
 * ENABLE_INCOMING_MSG_RECORDER for recording/replaying incoming network messages
 */
#undef ENABLE_INCOMING_MSG_RECORDER

#ifdef ENABLE_INCOMING_MSG_RECORDER
#include <nel/misc/file.h>
#endif

/*
// UNUSED
const uint		PropertiesPerEntity = 16;
const uint		EntitiesPerClient = 256;
*/

namespace NLMISC{
class CCDBNodeBranch;
}

namespace CLFECOMMON
{
	class CActionGeneric;
	class CActionGenericMultiPart;
}

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Network_Connection_Set_Reference_Position )

//#define MEASURE_FE_SENDING

extern const char * ConnectionStateCStr [9];


/**
 * Exception thrown when the sending returns a "blocking operation interrupted".
 */
class EBlockedByFirewall : public NLNET::ESocket
{};


struct TNewEntityInfo
{
	void reset()
	{
		DataSetIndex = CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
		Alias = 0;
	}

	/// Compressed dataset row
	CLFECOMMON::TClientDataSetIndex		DataSetIndex;

	/// Alias (only for mission giver NPCs)
	uint32								Alias;
};



// ***************************************************************************
/**
 * Abstracts the connection client towards front-end
 * \author Benjamin Legros, Olivier Cado
 * \author Nevrax France
 * \date 2001-2002
 */
class CNetworkConnection
{
public:
	enum
	{
		/*
		 * DO LEAVE ENOUGH ROOM FOR FUTURE PROPERTIES !
		 */
		AddNewEntity = 32,
		RemoveOldEntity,
		ConnectionReady,
		LagDetected,
		ProbeReceived
	};


	struct TVPNodeClient : public CLFECOMMON::TVPNodeBase
	{
		struct TSlotContext
		{
			CNetworkConnection						*NetworkConnection;
			CLFECOMMON::TCLEntityId					Slot;
			NLMISC::TGameCycle						Timestamp;
		};

		// Static data
		static TSlotContext							SlotContext;

		///
		TVPNodeClient() : TVPNodeBase() {}

		TVPNodeClient	*a() { return (TVPNodeClient*)VPA; }
		TVPNodeClient	*b() { return (TVPNodeClient*)VPB; }
		TVPNodeClient	*parent() { return (TVPNodeClient*)VPParent; }

		///
		void			decodeDiscreetProperties( NLMISC::CBitMemStream& msgin )
		{
			msgin.serialBitAndLog( BranchHasPayload );
			if ( BranchHasPayload )
			{
				if ( isLeaf() )
				{
					SlotContext.NetworkConnection->decodeDiscreetProperty( msgin, PropIndex );
				}
				else
				{
					if ( a() ) a()->decodeDiscreetProperties( msgin );
					if ( b() ) b()->decodeDiscreetProperties( msgin );
				}
			}
		}

		void		makeChildren()
		{
			VPA = new TVPNodeClient();
			VPA->VPParent = this;
			VPB = new TVPNodeClient();
			VPB->VPParent = this;
		}

		void		makeDescendants( uint nbLevels )
		{
			makeChildren();
			if ( nbLevels > 1 )
			{
				a()->makeDescendants( nbLevels-1 );
				b()->makeDescendants( nbLevels-1 );
			}
		}

		void		deleteBranches()
		{
			if ( a() )
			{	a()->deleteBranches();
				delete a();
			}
			if ( b() )
			{	b()->deleteBranches();
				delete b();
			}
		}

		void		deleteA()
		{
			if ( a() )
			{	a()->deleteBranches();
				delete a();
			}
		}

		void		deleteB()
		{
			if ( b() )
			{	b()->deleteBranches();
				delete b();
			}
		}

	};

	/// Additional info for position updates
	struct TPositionInfo
	{
		/**
		 * Only for positions (property index 0): interval, in game cycle unit, between the current
		 * position update and the next predicted one. In most cases the next real position update
		 * will occur just before the predicted interval elapses.
		 *
		 * If the interval cannot be predicted yet (e.g. before having received two updates
		 * for the same entity), PredictedInterval is set to ~0.
		 */
		NLMISC::TGameCycle			PredictedInterval;

		/// Is position interior (only for position)
		bool						IsInterior;
	};

	/**
	 * A property change
	 */
	class CChange
	{
	public:

		/// Constructor
		CChange( CLFECOMMON::TCLEntityId id = 0, uint8 prop = 255, NLMISC::TGameCycle gc = 0 ) :
			ShortId(id), Property(prop), GameCycle(gc) {}

		/// Slot
		CLFECOMMON::TCLEntityId		ShortId;

		/// Property index
		uint8						Property;

		/// The tick when the property changed, in server time
		NLMISC::TGameCycle			GameCycle;

		// These contextual additional properties must be set by hand (not by constructor)
		union
		{
			/// Position additional information (prediction & interior)
			TPositionInfo						PositionInfo;

			/// Additional information when creating an entitys
			TNewEntityInfo						NewEntityInfo;
		};
	};


	/// The states of the connection to the server (if you change them, change ConnectionStateCStr)
	enum TConnectionState
	{
		NotInitialised = 0,		// nothing happened yet
		NotConnected,			// init() called
		Authenticate,			// connect() called, identified by the login server
		Login,					// connecting to the frontend, sending identification
		Synchronize,			// connection accepted by the frontend, synchronizing
		Connected,				// synchronized, connected, ready to work
		Probe,					// connection lost by frontend, probing for response
		Stalled,				// server is stalled
		Disconnect,				// disconnect() called, or timeout, or connection closed by frontend
		Quit					// quit() called
	};

	typedef void	(*TImpulseCallback)(NLMISC::CBitMemStream &, CLFECOMMON::TPacketNumber packet, void *arg);

	static bool					LoggingMode;

public:

	/// Constructor
	CNetworkConnection();

	/// Destructor
	virtual ~CNetworkConnection();

	/// @name Connection initialisation
	//@{

	/**
	 * Inits the client connection to the front-end.
	 * \param cookie it s the cookie in string format that was passed to the exe command line (given by the nel_launcher)
	 * \param addr it s the front end address in string format that was passed to the exe command line (given by the nel_launcher)
	 */
	void			init(const std::string &cookie, const std::string &addr);

	/**
	 * Sets the cookie and front-end address, resets the connection state.
	 */
	void			initCookie(const std::string &cookie, const std::string &addr);

	/**
	 * Connects to the front-end (using or not the login system, depending on what was specified at init())
	 * \param result the message returned in case of an error
	 * Returns true if no error occured.
	 */
	bool			connect(std::string &result);


	/**
	 * Sets the callback called when	a generic impulse comes
	 */
	void			setImpulseCallback(TImpulseCallback callback, void *argument = NULL);

	//@}


	/// @name Connection management
	//@{

	/**
	 * Tells if the client is yet connected or not
	 */
	bool			isConnected();


	/**
	 * Gets the connection state
	 */
	TConnectionState	getConnectionState() const { return _ConnectionState; }


	/**
	 * Updates the whole connection with the frontend.
	 *
	 * Behaviour in login state when a firewall does not grant the sending:
	 * - When a sending is refused (error "Blocking operation interrupted")
	 * the exception EBlockedByFirewall in thrown. The first time, a time-out is armed.
	 * - In a later attempt, if the time-out expired, the function sets state to
	 * Disconnect, then throws EBlockedByFirewall.
	 *
	 * \return bool 'true' if data were sent/received.
	 */
	bool			update();


	/**
	 * Disconnects the current connection
	 */
	void			disconnect();

	//@}


	/**
	 * Quit the game till the connection is reset
	 */
	void			quit();


	/// @name Database management
	//@{

	/**
	 * Set database entry point
	 */
	void			setDataBase(NLMISC::CCDBNodeBranch *database);

	//@}



	/// @name Push and Send
	//@{

	/**
	 * Buffers a bitmemstream, that will be converted into a generic action, to be sent later to the server (at next update).
	 */
	void			push (NLMISC::CBitMemStream &msg);

	/**
	 * Buffers an action to be sent at next update (or later updates if network overload occurs)
	 */
	void			push(CLFECOMMON::CAction *action);

	/**
	 * Buffers a target action (set targetOrPickup to true for target, false for pick-up
	 */
	void			pushTarget(CLFECOMMON::TCLEntityId slot, LHSTATE::TLHState targetOrPickup);

	/**
	 * Send
	 */
	void			send(NLMISC::TGameCycle);

	/**
	 * Send last information without changes (only acknowedges frontend)
	 */
	void			send();

	/**
	 * Clear not acknownledged actions in sending buffer
	 */
	void			flushSendBuffer() { _Actions.clear(); }

	/**
	 * Set player reference position
	 */
//	void			setReferencePosition(const NLMISC::CVectorD &position) { _ImpulseDecoder.setReferencePosition(position); }

	//@}


	/// @name Temp methods
	// @{

	const std::vector<CChange>			&getChanges() { return _Changes; }

	void	clearChanges() { _Changes.clear(); }

	// @}


	/// @name Client time hints
	// @{
	NLMISC::TTime		getCurrentClientTime() { return _CurrentClientTime; }		// the time currently played at this frame (in the past)
	NLMISC::TGameCycle	getCurrentClientTick() { return _CurrentClientTick; }		// the tick for the current frame (also in the past)
	NLMISC::TGameCycle	getCurrentServerTick() { return _CurrentServerTick; }		// the last tick sent by the server.
	NLMISC::TTime		getMachineTimeAtTick() { return  _MachineTimeAtTick; }		// the machine time at the beginning of the current tick
	NLMISC::TTicks		getMachineTicksAtTick() { return _MachineTicksAtTick; }
	sint32				getMsPerTick() { return _MsPerTick; }
	NLMISC::TGameCycle	getSynchronize() { return _Synchronize; }
	uint32				getBestPing() { return _BestPing; }
	uint32				getPing() const { return _InstantPing; }
	NLMISC::TTime		getLCT() { return _LCT; }											// Lag compensation time
	NLMISC::TTime		getUpdateTime() { return _UpdateTime; }
	NLMISC::TTicks		getUpdateTicks() { return _UpdateTicks; }
	// Return an estimated smooth server tick. Increment only and accelerate/decelerate according to network
	sint64				getCurrentSmoothServerTick() const { return _CurrentSmoothServerTick; }
	// Convert a GameCycle to a SmoothServerTick
	sint64				convertToSmoothServerTick(NLMISC::TGameCycle t) const;
	// @}


	/// Set the Lag Compensation Time in ms (default: 1000)
	void				setLCT( NLMISC::TTime lct ) { _LCT = lct; }

	/// Allocation statistics
	void				displayAllocationStats();

	/// Get Allocation statistics
	std::string			getAllocationStats();

	/// @name Network Stats Methods
	// @{

	uint32				getSentBytes()						{ return _TotalSentBytes; }
	uint32				getPartialSentBytes()				{ uint32 ret = _PartialSentBytes; _PartialSentBytes = 0; return ret; }
	uint32				getReceivedBytes()					{ return _TotalReceivedBytes; }
	uint32				getPartialReceivedBytes()			{ uint32 ret = _PartialReceivedBytes; _PartialReceivedBytes = 0; return ret; }
	bool				getConnectionQuality()				{ return _ConnectionQuality; }
	NLMISC::TTime		getTimeSinceLastReceivedPacket()	{ return NLMISC::CTime::getLocalTime()-_LastReceivedNormalTime; }


	/// Return the average billed download rate in kbps, including all headers (UDP+IP+Ethernet)
	float				getMeanDownload() { return (_MeanDownload.mean(ryzomGetLocalTime ())+20+8+14)*0.008f; }

	/// Return the average billed upload rate in kbps, including all headers (UDP+IP+Ethernet)
	float				getMeanUpload() { return (_MeanUpload.mean(ryzomGetLocalTime ())+20+8+14)*0.008f; }

	/// Return the packet loss average
	float				getMeanPacketLoss()
	{
		NLMISC::TTime	time = ryzomGetLocalTime ();
		_MeanPackets.check(time);
		_MeanLoss.check(time);
		float	dpk = _MeanPackets.Values.empty() ? 0.0f : _MeanPackets.Values.back().second-_MeanPackets.Values.front().second+1;
		float	tloss = _MeanLoss.Content;
		return (dpk > 0.0f) ? std::min(100.0f, tloss*100.0f/dpk) : 0.0f;
	}

	/// Get total lost packets
	uint32				getTotalLostPackets() const	{ return _TotalLostPackets; }

	// @}


	/// Get the NLMISC::CEntityId from the TCLEntityId
	CLFECOMMON::TSheetId	get(CLFECOMMON::TCLEntityId id) const	{ return _PropertyDecoder.sheet(id); }

	/// Get the TCLEntityId from the NLMISC::CEntityId
	sint				get(CLFECOMMON::TSheetId id) const
	{
		TIdMap::const_iterator	it = _IdMap.find(id);
		return (it != _IdMap.end()) ? (*it).second : -1;
	}

	/// Set player's reference position
	void	setReferencePosition(const NLMISC::CVectorD &position)
	{
		H_AUTO_USE ( RZ_Client_Network_Connection_Set_Reference_Position )
		_PropertyDecoder.setReferencePosition( position );
	}

	/// Get local IP address
	const NLNET::CInetAddress&	getAddress()
	{
		return _Connection.localAddr();
	}

	/// Get socket
	NLNET::CUdpSock&			getConnection()
	{
		return _Connection.UdpSock;
	}

	/// Get userId
	uint32						getUserId()
	{
		if(_LoginCookie.isValid())
			return _LoginCookie.getUserId();
		else
			return 0xFFFFFFFF;
	}

	/// Get connection state c_string
	const char *						getConnectionStateCStr()
	{
		return ConnectionStateCStr[_ConnectionState];
	}

	/// Return the login cookie
	const NLNET::CLoginCookie&	getLoginCookie() const
	{
		return _LoginCookie;
	}

#ifdef ENABLE_INCOMING_MSG_RECORDER
	/// Return 'true' if currently recording.
	bool						isRecording() const {return _RecordIncomingMessagesOn;}
	/// Return 'true' if currently replaying.
	bool						isReplaying() const {return _ReplayIncomingMessagesOn;}

	/// Start/stop recording the incoming messages (in online mode)
	void						setRecordingMode( bool onOff, const std::string& filename = "");

	/// Start/stop replaying the incoming messages (in offline mode)
	void						setReplayingMode( bool onOff, const std::string& filename = "");
#endif

	void						decodeDiscreetProperty( NLMISC::CBitMemStream& msgin, CLFECOMMON::TPropIndex propIndex );

	const std::string			&getFrontendAddress() { return _FrontendAddress; }

protected:
	/// The current state of the connection
	TConnectionState			_ConnectionState;

	/// Connection Quality check
	bool						_ConnectionQuality;

	/// The address of the frontend service
	std::string					_FrontendAddress;

	/// The cookie for the login service
	NLNET::CLoginCookie			_LoginCookie;

	/// The UDP connection to the frontend
	NLNET::CUdpSimSock			_Connection;

	/// The receive buffer
	uint32						_ReceiveBuffer[65536];

	/// The impulse decoder
	CImpulseDecoder				_ImpulseDecoder;

	/// The callback for generic actions
	TImpulseCallback			_ImpulseCallback;

	/// The callback argument
	void						*_ImpulseArg;


	/// Position update tick queues (used for statistical interval prediction), one per slot
	std::deque<NLMISC::TGameCycle>		_PosUpdateTicks[256];

	/// Position update interval sorted sets (used for statistical interval prediction), one per slot
	std::multiset<NLMISC::TGameCycle>	_PosUpdateIntervals[256];

	/// MD5 hash keys of msg.xml and database.xml
	NLMISC::CHashKeyMD5			_MsgXmlMD5;
	NLMISC::CHashKeyMD5			_DatabaseXmlMD5;
	// if prec don't work, those may (if the server run on windows, prec won't work)
	NLMISC::CHashKeyMD5			_AltMsgXmlMD5;
	NLMISC::CHashKeyMD5			_AltDatabaseXmlMD5;


	/// @name Network statistics
	//@{

	class CMeanComputer
	{
	public:
		CMeanComputer() : MeanPeriod(2000), Content(0.0f) {}
		std::deque< std::pair<NLMISC::TTime, float> >	Values;
		uint32											MeanPeriod; // in ms
		float											Content;

		// checks all values have valid time (inside the mean period)
		void					check(NLMISC::TTime time)
		{
			while (!Values.empty() && Values.front().first < time-MeanPeriod)
			{
				Content -= Values.front().second;
				Values.pop_front();
			}
		}

		// updates the mean with a new value and time
		void					update(float value, NLMISC::TTime time)
		{
			if (!Values.empty() && Values.back().first > time)
				return;

			check(time);
			Values.push_back(std::make_pair(time, value));
			Content += value;
		}

		// gets the mean
		float					mean(NLMISC::TTime time)
		{
			check(time);
			return Content*1000.0f/(float)MeanPeriod;
		}
	};

	/// The total received length (from the beginning)
	uint32						_TotalReceivedBytes;

	/// The partial received length (since last read)
	uint32						_PartialReceivedBytes;

	/// The total sent length
	uint32						_TotalSentBytes;

	/// Partial sent length
	uint32						_PartialSentBytes;

	/// Total lost packets
	uint32						_TotalLostPackets;

	/// Mean download (payload bytes)
	CMeanComputer				_MeanDownload;

	/// Mean upload (payload bytes)
	CMeanComputer				_MeanUpload;

	/// Mean packets
	CMeanComputer				_MeanPackets;

	/// Mean lost
	CMeanComputer				_MeanLoss;

	//@}


	/// The property decoder
	CPropertyDecoder			_PropertyDecoder;

	/// The database entry
	NLMISC::CCDBNodeBranch				*_DataBase;


	/// @name Header message decoding
	//@{

	/// Was the header decoded for the last received message?
	bool						_DecodedHeader;

	/// The received number lastly decoded
	CLFECOMMON::TPacketNumber	_CurrentReceivedNumber;

	/// The message is in system mode
	bool						_SystemMode;

	/// Did we received a probe since last update
	bool						_ReceivedSync;

	uint						_TotalMessages;

	//@}



	/// @name Client and Server packet numbering management
	//@{

	CLFECOMMON::TPacketNumber	_CurrentSendNumber;		// number that will be used to send the next packet

	CLFECOMMON::TPacketNumber	_LastReceivedNumber;	// number of the last packet receive from the server
	uint32						_AckBitMask;			// this mask contains ack of old messages received by the server
	uint32						_LastAckBit;			// a remember of the status of the previous received packet
	NLMISC::TTime				_LastReceivedTime;		// last time the client received something from the server

	NLMISC::CBitSet				_LongAckBitField;
	CLFECOMMON::TPacketNumber	_LastAckInLongAck;

	NLMISC::TGameCycle			_LastSentCycle;

	CLFECOMMON::TPacketNumber	_LastReceivedPacketInBothModes;

	NLMISC::TTime				_LastReceivedNormalTime;	// last time the client received valid normal message from the server

	uint8						_ImpulseMultiPartNumber;

	//@}


	/// @name Client and Server time management
	//@{

	/// The stamps queues
	typedef std::deque<std::pair<sint32, NLMISC::TTime> >	TStampQueue;

	// used for lag compensation
	NLMISC::TTime				_CurrentClientTime;		// the time currently played at this frame (in the past)
	NLMISC::TGameCycle			_CurrentClientTick;		// the tick for the current frame (also in the past)
	NLMISC::TGameCycle			_CurrentServerTick;		// the last tick sent by the server.
	NLMISC::TTime				_MachineTimeAtTick;		// the machine time at the beginning of the current tick
	NLMISC::TTicks				_MachineTicksAtTick;
	sint32						_MsPerTick;
	NLMISC::TGameCycle			_Synchronize;
	uint32						_BestPing;
	uint32						_InstantPing;
	NLMISC::TTime				_LCT;					// Lag compensation time
	TStampQueue					_PacketStamps;

	NLMISC::TTime				_UpdateTime;
	NLMISC::TTicks				_UpdateTicks;
	NLMISC::TTime				_LastSentSync;

	NLMISC::TTime				_LastSendTime;

	//@}

	std::list<CLFECOMMON::CActionBlock>	_Actions;

	/// Changes since
	std::vector<CChange>		_Changes;

	// TEMP
	uint32						_DummySend;

	static bool					_Registered;

	/// @name Generic action multipart handling structures
	//@{
	struct CGenericMultiPartTemp
	{
		CGenericMultiPartTemp () : NbBlock(0xFFFFFFFF) { }
		uint32								NbBlock;
		uint32								NbCurrentBlock;
		uint32								TempSize;
		std::vector<std::vector<uint8> >	Temp;

		std::vector<bool>					BlockReceived;

		void set (CLFECOMMON::CActionGenericMultiPart *agmp, CNetworkConnection *parent);
	};

	std::vector<CGenericMultiPartTemp>	_GenericMultiPartTemp;
	//@}

	/// @name NLMISC::CEntityId handling (for gamedev)
	//@{
	class CHash
	{
	public:
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;

		CHash() {}

		size_t	operator () (const CLFECOMMON::TSheetId &id) const { return id; }

		inline bool operator() (const CLFECOMMON::TSheetId &id1, const CLFECOMMON::TSheetId &id2) const { return (size_t)id1 < (size_t)id2; }
	};

	typedef	CHashMap<CLFECOMMON::TSheetId, CLFECOMMON::TCLEntityId, CHash>	TIdMap;

	TIdMap						_IdMap;
	//@}


	TVPNodeClient				*_VisualPropertyTreeRoot;


	/// \name Smooth ServerTick mgt.
	// @{
	sint64						_CurrentSmoothServerTick;
	NLMISC::TTime				_SSTLastLocalTime;

	void						updateSmoothServerTick();
	// @}

	void						reinit();

private:

	/// Set MsPerTick value
	void						setMsPerTick(sint32 msPerTick);


	/// Builds the CBitMemStream from the next incoming packet
	bool						buildStream(NLMISC::CBitMemStream &msgin);

	/// @name Client automaton states and actions
	//@{

	//
	void						sendSystemLogin();
	bool						stateLogin();
	NLMISC::TTime				_LatestLoginTime;

	//
	void						receiveSystemSync(NLMISC::CBitMemStream &msgin);
	void						sendSystemAckSync();
	bool						stateSynchronize();
	NLMISC::TTime				_LatestSyncTime;
	uint32						_LatestSync;

	//
	void						receiveNormalMessage(NLMISC::CBitMemStream &msgin);
	void						sendNormalMessage();
	bool						stateConnected();
	CLFECOMMON::TPacketNumber	_LastReceivedAck;
	uint						_NormalPacketsReceived;

	//
	void						receiveSystemProbe(NLMISC::CBitMemStream &msgin);
	void						sendSystemAckProbe();
	bool						stateProbe();
	std::vector<CLFECOMMON::TPacketNumber>	_LatestProbes;
	CLFECOMMON::TPacketNumber	_LatestProbe;
	NLMISC::TTime				_LatestProbeTime;

	//
	void						receiveSystemStalled(NLMISC::CBitMemStream &msgin);
	bool						stateStalled();

	//
	void						sendSystemDisconnection();

	//
	bool						stateQuit();
	void						receiveSystemAckQuit(NLMISC::CBitMemStream &msgin);
	void						sendSystemQuit();
	uint32						_QuitId;
	bool						_ReceivedAckQuit;
	NLMISC::TTime				_LatestQuitTime;

	//
	void						reset();
	void						initTicks();

	//@}

	//
	bool						decodeHeader(NLMISC::CBitMemStream &msgin, bool checkMessageNumber = true);

	//
	void						buildSystemHeader(NLMISC::CBitMemStream &msgout);

	//
	void						genericAction (CLFECOMMON::CActionGeneric *ag);
	void						genericAction (CLFECOMMON::CActionGenericMultiPart *agmp);

	//
	void						statsSend(uint32 bytes);
	void						statsReceive(uint32 bytes);

	bool						associationBitsHaveChanged( CLFECOMMON::TCLEntityId slot, uint32 associationBits )
	{
		bool res = ((uint16)associationBits != _PropertyDecoder.associationBits( slot ));
		_PropertyDecoder.associationBits( slot ) = (uint16)associationBits;
		return res;
	}

	void						decodeVisualProperties( NLMISC::CBitMemStream& msgin );

	/// Return true if there is some messages to replay (in replay mode)
	bool						dataToReplayAvailable();

	uint32						_AssociationBits;

#ifdef ENABLE_INCOMING_MSG_RECORDER
	NLMISC::COFile				_RecordedMessagesOut;

	NLMISC::CIFile				_RecordedMessagesIn;
	NLMISC::TGameCycle			_NextClientTickToReplay; // ~0 when no more message
	NLMISC::CBitMemStream		_NextMessageToReplay; // loaded at beginning of update(), read in buildStream()

	bool						_RecordIncomingMessagesOn;
	bool						_ReplayIncomingMessagesOn;

#endif

	//
	friend struct CGenericMultiPartTemp;
};


#endif // NL_NETWORK_CONNECTION_H

/* End of network_connection.h */
