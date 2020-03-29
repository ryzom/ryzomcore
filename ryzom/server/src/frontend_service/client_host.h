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



#ifndef NL_CLIENT_HOST_H
#define NL_CLIENT_HOST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/time_nl.h"
#include "nel/net/inet_address.h"
#include "nel/net/login_cookie.h"

#include "fe_types.h"
#include "fe_receive_task.h"
#include "game_share/action.h"
#include "client_entity_id_translator.h"
#include "impulse_encoder.h"
#include "entity_container.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/entity_types.h"
#include "game_share/welcome_service_itf.h"

#include <vector>
#include <deque>


const uint32 FirstClientId = 1;
const uint16 InvalidClientId = 0xFFFF;

namespace NLNET
{
	class CUdpSock;
};

struct TPairState;


/**
 * CClientIdPool
 */
class CClientIdPool
{
public:

	/// Constructor
	CClientIdPool()
	{
		TClientId i;
		for ( i=0; i<=MaxNbClients; ++i )
		{
			_UsedPool[i] = false;
		}
	}

	/// Get a free Id
	TClientId	getNewClientId()
	{
		TClientId i;
		for ( i=FirstClientId; i<=MaxNbClients; ++i )
		{
			if ( ! _UsedPool[i] )
			{
				_UsedPool[i] = true;
				return i;
			}
		}
		return InvalidClientId;
	}

	/// Release an Id
	void		releaseId( TClientId id )
	{
		_UsedPool[id] = false;
	}

private:

	bool		_UsedPool [MAX_NB_CLIENTS+1];
};


/// Get string for association state
const char *associationStateToString( uint8 as );


/**
 * Client host
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CClientHost
{
public:
	/// Constructor
	CClientHost( const NLNET::CInetAddress& addr, TClientId id ) :
		Uid (0xFFFFFFFF),
		InstanceId(0xFFFFFFFF),
		StartupRole(WS::TUserRole::ur_player),
		NbFreeEntityItems( 255 ),
		PrioAmount( 0.0f ),
		_Address(addr),
		_ClientId(id),
		_Id(NLMISC::CEntityId::Unknown),
		_EntityIndex(),
		_SendNumber(0),
		_SendSyncTick(0),
		_Synchronized(false),
		_Disconnected(false),
		_FirstReceiveNumber(0),
		_ReceiveNumber(0xFFFFFFFF),
		_ReceiveTime(0),
		_DatagramLost(0),
		_DatagramRepeated(0),
		//SheetId(CLFECOMMON::INVALID_SHEETID),
		//_ToggleBit(false),
		//_OutBoxMeter(0),
		IdTranslator(),
		ImpulseEncoder(),
		LastReceivedAck(0xFFFFFFFF),
		ImpulseMultiPartNumber (0),
		AuthorizedCharSlot(~0),
		LastReceivedGameCycle(0),
		LastSentSync(1),
		//
		LastDummy(~0),
		LastSentDummy(~0),
		LastSentCounter(0),
		LastCounterTime(0),
		//
		//AvailableImpulseBitsize( "AvailImpulseBitsize", MaxImpulseBitSizes[2] ),
		NbActionsSentAtCycle(0),
		QuitId(0)
		{
			IdTranslator.setId( id );
			ImpulseEncoder.setClientHost( this );
			ConnectionState = Synchronize;
			initClientBandwidth();
		}

	/// Destructor
	~CClientHost();

	/// Return IP and port
	const NLNET::CInetAddress&	address() { return _Address; }

	/// Return client Id
	TClientId			clientId() const { return _ClientId; }

	/// Return the entity index (for access in the entity container)
	TEntityIndex		entityIndex() const { return _EntityIndex; }

	/// Return the id
	const NLMISC::CEntityId& eId() const { return _Id; }

	/// Set the entity index
	void				setEntityIndex( const TEntityIndex& ei );

	/// Set the CEntityId
	void				setEId( const NLMISC::CEntityId& assigned_id );

	/// Prepare a clean new outbox with current values
	void				setupOutBox( TOutBox& outbox );

	/// Prepare a clean system header
	void				setupSystemHeader( TOutBox& outbox, uint8 code);

	/// Compute host stats
	void				computeHostStats( const TReceivedMessage& msgin, uint32 currentcounter, bool updateAcknowledge );

	/// Increment send number and return it
	uint32				getNextSendNumber() { return ++_SendNumber; }

	/// R access to last send number
	uint32				sendNumber() const { return _SendNumber; }

	/// R/W access to first receive number
	uint32&				firstReceiveNumber() { return _FirstReceiveNumber; }

	/// R/W access to latest receive number
	uint32&				receiveNumber() { return _ReceiveNumber; }

	/// R/W access to the toggle bit
	//bool&				toggleBit() { return _ToggleBit; }

    /// Set receive time now
	void				setReceiveTimeNow();
  
	/// Return receive time
	NLMISC::TTime		receiveTime() const { return _ReceiveTime; }

	uint32				datagramLost() const { return _DatagramLost; }
	void				resetDatagramLost() { _DatagramLost = 0; }

	uint32				datagramRepeated() const { return _DatagramRepeated; }

	/// Setup sync for tick measures with client connection
	void				setFirstSentPacket(uint32 sentPacket, NLMISC::TGameCycle atTick)
	{
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
		_SendSyncTick = atTick - sentPacket*2;
#else
		_SendSyncTick = atTick - sentPacket;
#endif
	}

	/// Convert ack contained in client packet into tick date
	NLMISC::TGameCycle	getPacketTickDate(uint32 receivedPacketAck) const { return _SendSyncTick+receivedPacketAck; }


	/// Get sync value for this client;
	NLMISC::TGameCycle	getSync() const { return _SendSyncTick; }

	void				disconnect() { _Disconnected = true; }
	bool				isDisconnected() { return _Disconnected; }

	// Reset client vision
	void				resetClientVision();

	/// Set clienthost to synchronize state
	void				setSynchronizeState()
	{
		ConnectionState = Synchronize;
	}

	/// Set clienthost to synchronize state
	void				setConnectedState()
	{
		ConnectionState = Connected;
	}

	/// Set clienthost to synchronize state
	void				setStalledState()
	{
		ConnectionState = Stalled;
	}

	/// Set clienthost to probe state
	void				setProbeState()
	{
		ConnectionState = Probe;
		LastSentProbe = 0;
		LastProbeTime = 0;
		NumConsecutiveProbes = 0;
		LastReceivedProbe = 0;
	}

	/// Set clienthost to ForceSynchronize state (i.e. Synchronize must not be replaced by Connected, as a sync must be sent to the client)
	void				setForceSynchronizeState()
	{
		ConnectionState = ForceSynchronize;
	}

	/// Initialize the counter/flag 
	void				initSendCycle( bool initialState )
	{
		_WhenToSend = initialState;
	}

	/// Update the counter/flag
	void				incSendCycle()
	{
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
		_WhenToSend = !_WhenToSend;
#endif
	}

	/// Return true if the counter/flag state is "to send" for the current cycle
	bool				whenToSend()
	{
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
		return _WhenToSend;
#else
		return true;
#endif
	}

	/// display nlinfo
	void				displayClientProperties( bool full=true, bool allProps=false, bool sortByDistance=false,NLMISC::CLog *log = NLMISC::InfoLog ) const;

	/// display nlinfo for one slot
	void				displaySlotProperties( CLFECOMMON::TCLEntityId e, bool full=false, NLMISC::CLog *log = NLMISC::InfoLog ) const;

	/// display nlinfo (1 line only)
	void				displayShortProps(NLMISC::CLog *log = NLMISC::InfoLog) const;

	/// Return the cardinal direction from the player to the seen entity
	const char *		getDirection( CEntity *seenEntity, const TEntityIndex& seenEntityIndex ) const;

	/// Initialize the client bandwidth (calls setClientBandwidth)
	void				initClientBandwidth();

	/// Change the client bandwidth (set the nomimal size)
	void				setClientBandwidth( sint32 cbw )
	{
		_MaxOutboxSizeInBit = cbw;
		_BitBandwidthUsageAvg = _MaxOutboxSizeInBit;
		_BitImpulsionUsageAvg = MaxImpulseBitSizes[2];
		_ImpulsionPrevRemainingActions = 0;
	}

	/// Return the current maximum number of bits that can fit in the outbox
	sint32				getCurrentThrottle() const
	{
		return std::min( (sint32)(_MaxOutboxSizeInBit*2-_BitBandwidthUsageAvg), (sint32)(_MaxOutboxSizeInBit*3/2) );
	}

	/// Update the average bits filled that determine the throttle
	void				updateThrottle( TOutBox& outbox )
	{
#ifdef NL_DEBUG
		sint32 prevThrottle = getCurrentThrottle();
#endif
		// Update the average of bits sent
		_BitBandwidthUsageAvg = (_BitBandwidthUsageAvg*(_SendNumber-1) + outbox.getPosInBit()) / _SendNumber;
#ifdef NL_DEBUG
		sint32 currThrottle = getCurrentThrottle();
		if ( currThrottle != prevThrottle )
			nldebug( "NFC: Client %hu, packet %u: throttle %d, %d filled now, %d average (%d kbps), %d nominal", _ClientId, _SendNumber, currThrottle, outbox.getPosInBit(), _BitBandwidthUsageAvg, _BitBandwidthUsageAvg*5/1000, _MaxOutboxSizeInBit );
#endif
	}

	/**
	 * Calculate the current maximum number of bits that can fit for impulsions,
	 * and set AvailableImpulsionBitSize in the mirror, using:
	 * - The number of bits filled (possibly exceeding the max when sending forced actions (database))
	 * - The number of remaining actions not forced
	 */
	void				setImpulsionThrottle( sint32 currentNbBitsFilled, sint32 nominalBitSize, uint nbRemainingActions )
	{
		if ( !_EntityIndex.isValid() )
			return;
		
		if ( ((_ImpulsionPrevRemainingActions>12) && (nbRemainingActions > _ImpulsionPrevRemainingActions * 5/4))
			|| _ImpulsionPrevRemainingActions>100 )
		{
			// The connection does not manage to send all the impulsions that come from the back-end, stop the database impulsions
			CMirrorPropValue<uint16> availableImpulseBitsize( TheDataset, _EntityIndex, DSFirstPropertyAvailableImpulseBitSize );
			availableImpulseBitsize = 0;
#ifdef NL_DEBUG
			nldebug( "NFC: Client %hu: Blocking the AvailableImpulseBitsize to prevent impulsion congestion", _ClientId );
#endif
		}
		else
		{
			// The remaining actions number is stable, calculate the available bitsize
			_BitImpulsionUsageAvg = (_BitImpulsionUsageAvg*(_SendNumber-1) + currentNbBitsFilled) / _SendNumber;
			sint32 availBitsize;
			if ( _BitImpulsionUsageAvg < nominalBitSize )
				availBitsize = nominalBitSize;
			else
				availBitsize = std::max( (sint32)0, nominalBitSize*2 - _BitImpulsionUsageAvg );
			
			CMirrorPropValue<uint16> availableImpulseBitsize( TheDataset, _EntityIndex, DSFirstPropertyAvailableImpulseBitSize );
#ifdef NL_DEBUG
			if ( availBitsize != availableImpulseBitsize )
				nldebug( "NFC: Client %hu, packet %u: AvailableImpulseBitsize %u, %d filled now, %d average, %d nominal", _ClientId, _SendNumber, availBitsize, currentNbBitsFilled, _BitImpulsionUsageAvg, nominalBitSize );
#endif
			availableImpulseBitsize = (uint16)availBitsize;
		}
		_ImpulsionPrevRemainingActions = nbRemainingActions;
	}

	/// Force the impulsion throttle to 0 to prevent overflooding, when the FS does send any impulsion.
	void	setIdleImpulsionThrottle()
	{
		if ( !_EntityIndex.isValid() )
			return;
		
		CMirrorPropValue<uint16> availableImpulseBitsize( TheDataset, _EntityIndex, DSFirstPropertyAvailableImpulseBitSize );
		availableImpulseBitsize = 0;
#ifdef NL_DEBUG
		nldebug( "NFC: Client %hu: Blocking the AvailableImpulseBitsize to prevent impulsion congestion", _ClientId );
#endif
		
		// The FS must not send any impulsion at this time
		_ImpulsionPrevRemainingActions = 0;
		
	}

	// get Pair state
	TPairState&			getPairState(CLFECOMMON::TCLEntityId e);
	// get Pair state
	const TPairState&	getPairState(CLFECOMMON::TCLEntityId e) const;

	/// User identifier
	TUid				Uid;

	/// User name (put on the NeL Launcher, transmitted by the login system)
	std::string			UserName;

	/// User privilege (put on the NeL Launcher, transmitted by the login system)
	std::string			UserPriv;

	/// User extended data (put on the NeL Launcher, transmitted by the login system)
	std::string			UserExtended;

	/// Language Id
	std::string			LanguageId;

	/// Login cookie
	NLNET::CLoginCookie	LoginCookie;

	/// Startup instance
	uint32				InstanceId;

	/// Startup role
	WS::TUserRole		StartupRole;

	/// Sheet identifier
	//CLFECOMMON::TSheetId	SheetId;

	/// Number of free entity items
	uint16				NbFreeEntityItems;

	// Used to differenciate different multi part impulse
	uint8				ImpulseMultiPartNumber;

	// The only character slot that will be granted to connect [0..4], or 0xF (15) for any character
	uint8				AuthorizedCharSlot;

	/// States of the client connection
	enum
	{
		Synchronize = 0,
		Connected,
		Probe,
		Stalled,
//		Disconnect,				// Appears to be unused at least for now
		ServerDown,
		ForceSynchronize,		// prevents Synchronize to be replaced by Connected
	};

	/// The current state of the client connection
	uint				ConnectionState;

	/// The last received probe;
	uint32				LastReceivedProbe;

	/// The number of consecutive probes received lastly
	uint32				NumConsecutiveProbes;

	/// The last sent probe;
	uint32				LastSentProbe;

	/// The last sent sync
	uint32				LastSentSync;

	/// The time the last probe was sent
	NLMISC::TTime		LastProbeTime;

	/// Stat
	float				PrioAmount;


	/// Counter check
	uint32				LastSentCounter;

	/// Counter time
	NLMISC::TTime		LastCounterTime;

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
 
 		void set (CLFECOMMON::CActionGenericMultiPart *agmp, CClientHost *client);
 	};
 
 	std::vector<CGenericMultiPartTemp>	GenericMultiPartTemp;
 	//@}

	/// Quit Id
	uint32				QuitId;

private:

	/// Client IP and port
	NLNET::CInetAddress	_Address;

	/// Client Id
	TClientId			_ClientId;

	/// Entity Id
	TEntityIndex		_EntityIndex;

	/// CEntityId
	NLMISC::CEntityId	_Id;

	/// Latest send number
	uint32				_SendNumber;

	/// Counter (or flag) determining when to send the prioritized properties to the client
	bool				_WhenToSend;

	/// First tick on front end, corresponding to the first packet sent
	NLMISC::TGameCycle	_SendSyncTick;
	bool				_Synchronized;
	bool				_Disconnected;

	/// First receive number
	uint32				_FirstReceiveNumber;

	/// Latest receive number
	uint32				_ReceiveNumber;

	/// Bit for important actions
	//bool				_ToggleBit;

	/// Timestamp of latest receiving
	NLMISC::TTime		_ReceiveTime;

	/// Stat
	uint32				_DatagramLost;

	/// Stat
	uint32				_DatagramRepeated;

	/// Nominal size
	sint32				_MaxOutboxSizeInBit;

	/// Number of bits that can be added to the nominal size (>0), or that must be removed from the nominal size (<0)
	sint32				_DeltaBitsAllowed;

	/// Previous number of actions not sent by the impulse encoder
	uint32				_ImpulsionPrevRemainingActions;

	/// Average of total bits sent
	sint32				_BitBandwidthUsageAvg;

	/// Average of impulsion bits sent
	sint32				_BitImpulsionUsageAvg;

public:

	// Client <- frontend entity id translator
	CClientEntityIdTranslator	IdTranslator;

	/// Impulse management
	CImpulseEncoder		ImpulseEncoder;

	/// Last ack number received from client
	uint32				LastReceivedAck;

	///
	NLMISC::TGameCycle	LastReceivedGameCycle;

	uint32				LastDummy;
	uint32				LastSentDummy;

	// Estimation of available size in bit per cycle
	//CPropertyBaseType<uint16> AvailableImpulseBitsize;

	///
	uint				NbActionsSentAtCycle;

#ifdef NL_DEBUG
	uint8				MoveNumber;
#endif
};




/**
 *
 */
class CLimboClient
{
public:
	CLimboClient( CClientHost* client ) :
		AddrFrom(client->address()), Uid(client->Uid), UserName(client->UserName), UserPriv(client->UserPriv), UserExtended(client->UserExtended),
		LanguageId(client->LanguageId), QuitId(client->QuitId)
	{
		// Set limbo timeout start
		Timeout = NLMISC::CTime::getLocalTime();
		LoginCookie = client->LoginCookie;
	}

	NLNET::CInetAddress	AddrFrom;
	TUid				Uid;
	std::string			UserName, UserPriv, UserExtended, LanguageId;
	uint32				QuitId;
	NLMISC::TTime		Timeout;
	NLNET::CLoginCookie	LoginCookie;
};


#endif // NL_CLIENT_HOST_H

/* End of client_host.h */
