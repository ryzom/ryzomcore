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



#ifndef CL_NET_MANAGER_H
#define CL_NET_MANAGER_H

//#define CLIENT_MULTI

/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
// Client.
#include "network_connection.h"
// Game Share
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/chat_group.h"



//////////////
// FUNCTION //
//////////////
void initializeNetwork();


namespace NLMISC
{
	class CBitMemStream;
	class CCDBNodeLeaf;
};


///////////
// CLASS //
///////////
/**
 * Class to manage the network.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CNetManager : public CNetworkConnection
{
public:
	/// Constructor.
	CNetManager();

	/**
	 * Updates the whole connection with the frontend.
	 * Call this method evently.
	 *
	 * Behaviour in login state when a firewall does not grant the sending:
	 * - When a sending is refused (error "Blocking operation interrupted")
	 * the exception EBlockedByFirewall in thrown. The first time, a time-out is armed.
	 * - In a later attempt, if the time-out expired, the function sets state to
	 * Disconnect, then throws EBlockedByFirewall.
	 *
	 * \return bool 'true' if data were sent/received.
	 */
	bool update();

	bool getConnectionQuality();

	/**
	 * Buffers a bitmemstream, that will be converted into a generic action, to be sent later to the server (at next update).
	 */
	void push (NLMISC::CBitMemStream &msg);

	/**
	 * Buffers a target action
	 */
	void pushTarget(CLFECOMMON::TCLEntityId slot);

	/**
	 * Buffers a pick-up action
	 */
	void pushPickup(CLFECOMMON::TCLEntityId slot, LHSTATE::TLHState lootOrHarvest);

	/**
	 * Send
	 */
	void send(NLMISC::TGameCycle);

	/**
	 * Send last information without changes (only acknowedges frontend)
	 */
	void send();

	/**
	 * Disconnects the current connection
	 */
	void disconnect();

	/**
	 * Reset data and reinit the socket
	 */
	void reinit();

	/**
	 * Waits for server to answer
	 */
	void waitForServer();

#ifdef ENABLE_INCOMING_MSG_RECORDER
	/// Start/stop replaying the incoming messages (in offline mode)
	void setReplayingMode( bool onOff, const std::string& filename = "");
	bool isReplayStarting() const {return _IsReplayStarting;}
	void startReplay();

protected:
	bool _IsReplayStarting;
#endif

	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_PingLeaf;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_UploadLeaf;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_DownloadLeaf;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_PacketLostLeaf;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_ServerStateLeaf;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> m_ConnectionQualityLeaf;
};


//////////////
// FUNCTION //
//////////////
// The impulse callback to receive all msg from the frontend.
void impulseCallBack(NLMISC::CBitMemStream &, sint32 packet, void *arg);

#define ALL_MANAGERS std::vector<CNetManager*>::iterator inm; for( inm=_NetManagers.begin(); inm!=_NetManagers.end(); ++inm ) (*inm)
#define FIRST_MANAGER _NetManagers[0]

/*
 *
 */
class CNetManagerMulti
{
public:
	/// Constructor.
	CNetManagerMulti() {}

	/// Create the net managers
	void	init( const std::string& cookie, const std::string& addr );

	/// Destructor
	~CNetManagerMulti() { std::vector<CNetManager*>::iterator inm; for( inm=_NetManagers.begin(); inm!=_NetManagers.end(); ++inm ) delete (*inm); }

	/**
	 * Updates the whole connection with the frontend.
	 * Call this method evently.
	 * \return bool : 'true' if data were sent/received.
	 */
	bool update() { ALL_MANAGERS->update(); return true; }

	/**
	 * Buffers a bitmemstream, that will be converted into a generic action, to be sent later to the server (at next update).
	 */
	void push (NLMISC::CBitMemStream &msg) { ALL_MANAGERS->push( msg ); }

	/**
	 * Buffers a target action
	 */
	void pushTarget(CLFECOMMON::TCLEntityId slot) { ALL_MANAGERS->pushTarget( slot ); }

	/**
	 * Buffers a pick-up action
	 */
	void pushPickup(CLFECOMMON::TCLEntityId slot, LHSTATE::TLHState lootOrHarvest) { ALL_MANAGERS->pushPickup( slot, lootOrHarvest ); }

	/**
	 * Send
	 */
	void send(NLMISC::TGameCycle gc) {	ALL_MANAGERS->send( gc ); }

	/**
	 * Send
	 */
	void send() { ALL_MANAGERS->send(); }

	/**
	 * Disconnects the current connection
	 */
	void disconnect() { ALL_MANAGERS->disconnect(); }


	/**
	 * Waits for server to answer
	 */
	void waitForServer() { ALL_MANAGERS->waitForServer(); }

#ifdef ENABLE_INCOMING_MSG_RECORDER
	void setRecordingMode( bool onOff, const std::string& filename = "") {}
	void setReplayingMode( bool onOff, const std::string& filename = "") {}
	bool isReplayStarting() const {return false;}
	void startReplay()  {}
	bool isRecording() const {return false;}
	bool isReplaying() const {return false;}
#endif


	NLMISC::TTime		getCurrentClientTime() { return FIRST_MANAGER->getCurrentClientTime(); }		// the time currently played at this frame (in the past)
	NLMISC::TGameCycle	getCurrentClientTick() { return FIRST_MANAGER->getCurrentClientTick(); }		// the tick for the current frame (also in the past)
	NLMISC::TGameCycle	getCurrentServerTick() { return FIRST_MANAGER->getCurrentServerTick(); }		// the last tick sent by the server.
	NLMISC::TTime		getMachineTimeAtTick() { return  FIRST_MANAGER->getMachineTimeAtTick(); }		// the machine time at the beginning of the current tick
	NLMISC::TTicks		getMachineTicksAtTick() { return FIRST_MANAGER->getMachineTicksAtTick(); }
	sint32				getMsPerTick() { return FIRST_MANAGER->getMsPerTick(); }
	NLMISC::TGameCycle	getSynchronize() { return FIRST_MANAGER->getSynchronize(); }
	uint32				getBestPing() { return FIRST_MANAGER->getBestPing(); }
	uint32				getPing() const { return FIRST_MANAGER->getPing(); }
	NLMISC::TTime		getLCT() { return FIRST_MANAGER->getLCT(); }											// Lag compensation time
	NLMISC::TTime		getUpdateTime() { return FIRST_MANAGER->getUpdateTime(); }
	NLMISC::TTicks		getUpdateTicks() { return FIRST_MANAGER->getUpdateTicks(); }
	uint32				getUserId() { return 0; }
	const char*			getConnectionStateCStr() { return ""; }
	bool				getConnectionQuality() { return true; }

	// Return an estimated smooth server tick. Increment only and accelerate/decelerate according to network
	sint64				getCurrentSmoothServerTick() const { return FIRST_MANAGER->getCurrentSmoothServerTick(); }
	// Convert a GameCycle to a SmoothServerTick
	sint32				convertToSmoothServerTick(NLMISC::TGameCycle t) const { return (sint32)(FIRST_MANAGER->convertToSmoothServerTick(t)); }
	// @}

	void				flushSendBuffer() { ALL_MANAGERS->flushSendBuffer(); }
	void				quit() { ALL_MANAGERS->quit(); }

	/// Set the Lag Compensation Time in ms (default: 1000)
	void				setLCT( NLMISC::TTime lct ) { ALL_MANAGERS->setLCT( lct ); }

	const NLNET::CInetAddress&	getAddress() { return FIRST_MANAGER->getAddress(); }

	/// Set player's reference position
	void				setReferencePosition(const NLMISC::CVectorD &position) { ALL_MANAGERS->setReferencePosition( position ); }

	const std::vector<CNetworkConnection::CChange>			&getChanges() { return FIRST_MANAGER->getChanges(); }

	//void								clearChanges() { ALL_MANAGERS->clearChanges(); }

	void				setImpulseCallback(CNetworkConnection::TImpulseCallback callback, void *argument = NULL) { FIRST_MANAGER->setImpulseCallback( callback, argument ); }
	void				setDataBase(NLMISC::CCDBNodeBranch *database) { ALL_MANAGERS->setDataBase( database ); }
	bool				connect(std::string &result) { bool res = false; std::vector<CNetManager*>::iterator inm; for( inm=_NetManagers.begin(); inm!=_NetManagers.end(); ++inm ) if ( (*inm)->connect( result ) ) res = true; return res; }
	CNetworkConnection::TConnectionState	getConnectionState() const { return FIRST_MANAGER->getConnectionState(); }

	float				getMeanPacketLoss() { return FIRST_MANAGER->getMeanPacketLoss(); }
	uint32				getTotalLostPackets() { return FIRST_MANAGER->getTotalLostPackets(); }
	float				getMeanUpload() { return FIRST_MANAGER->getMeanUpload(); }
	float				getMeanDownload() { return FIRST_MANAGER->getMeanDownload(); }

	const NLNET::CLoginCookie&	getLoginCookie() const { return FIRST_MANAGER->getLoginCookie(); }
private:

	std::vector<CNetManager*>	_NetManagers;
};

/* ----------------------------
 * Access to web, for mail/forum
 */

extern uint32					ShardId;
extern TSessionId				CharacterHomeSessionId;
extern std::string				WebServer;

////////////
// GLOBAL //
////////////
#ifdef CLIENT_MULTI
extern CNetManagerMulti			NetMngr;
#else
extern CNetManager				NetMngr;
#endif
extern CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;

extern bool UseFemaleTitles;


#endif // CL_NET_MANAGER_H

/* End of net_manager.h */
