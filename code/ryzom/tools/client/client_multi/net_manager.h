/** \file net_manager.h
 * Manage the network connection on the client.
 *
 * $Id: net_manager.h,v 1.2 2004/03/01 19:20:42 lecroart Exp $
 */



#ifndef CL_NET_MANAGER_H
#define CL_NET_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"
// Client.
#include "network_connection.h"
// Game Share
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"



//////////////
// FUNCTION //
//////////////
void initializeNetwork();


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
	 * \return bool : 'true' if data were sent/received.
	 */
	bool update();

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
	 * Send
	 */
	void send();

	/**
	 * Disconnects the current connection
	 */
	void disconnect();


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
	void	init( const std::string& cookie, const std::string& addr )
	{
		uint nb;
		NLMISC::CConfigFile::CVar *var = ClientCfg.ConfigFile.getVarPtr( "NbConnections" );
		if ( var )
			nb = var->asInt();
		else
			nb = 1;
		nlinfo( "CNetManagerMulti: Creating %u connections...", nb );

		for ( uint i=0; i!=nb; ++i )
		{
			CNetManager *nm = new CNetManager();
			nm->init( cookie, addr );
			_NetManagers.push_back( nm );
		}
	}

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
	// Return an estimated smooth server tick. Increment only and accelerate/decelerate according to network
	sint32				getCurrentSmoothServerTick() const { return FIRST_MANAGER->getCurrentSmoothServerTick(); }
	// Convert a GameCycle to a SmoothServerTick
	sint32				convertToSmoothServerTick(NLMISC::TGameCycle t) const { return FIRST_MANAGER->convertToSmoothServerTick(t); }
	// @}
	
	
	/// Set the Lag Compensation Time in ms (default: 1000)
	void				setLCT( NLMISC::TTime lct ) { ALL_MANAGERS->setLCT( lct ); }
	
	const NLNET::CInetAddress&	getAddress() { return FIRST_MANAGER->getAddress(); }

	/// Set player's reference position
	void	setReferencePosition(const NLMISC::CVectorD &position) { ALL_MANAGERS->setReferencePosition( position ); }

	const std::vector<CNetworkConnection::CChange>			&getChanges() { return FIRST_MANAGER->getChanges(); }
	
	//void								clearChanges() { ALL_MANAGERS->clearChanges(); }
	
	void			setImpulseCallback(CNetworkConnection::TImpulseCallback callback, void *argument = NULL) { FIRST_MANAGER->setImpulseCallback( callback, argument ); }
	void			setDataBase(CCDBNodeBranch *database) { ALL_MANAGERS->setDataBase( database ); }
	bool			connect(std::string &result) { bool res = false; std::vector<CNetManager*>::iterator inm; for( inm=_NetManagers.begin(); inm!=_NetManagers.end(); ++inm ) if ( (*inm)->connect( result ) ) res = true; return res; }
	CNetworkConnection::TConnectionState	getConnectionState() const { return FIRST_MANAGER->getConnectionState(); }

	float				getMeanPacketLoss() { return FIRST_MANAGER->getMeanPacketLoss(); }
	float				getMeanUpload() { return FIRST_MANAGER->getMeanUpload(); }
	float				getMeanDownload() { return FIRST_MANAGER->getMeanDownload(); }
	
private:

	std::vector<CNetManager*>	_NetManagers;
};


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
	void	init( const std::string& cookie, const std::string& addr )
	{
		uint nb;
		NLMISC::CConfigFile::CVar *var = ClientCfg.ConfigFile.getVarPtr( "NbConnections" );
		if ( var )
			nb = var->asInt();
		else
			nb = 1;
		nlinfo( "CNetManagerMulti: Creating %u connections...", nb );

		for ( uint i=0; i!=nb; ++i )
		{
			CNetManager *nm = new CNetManager();
			nm->init( cookie, addr );
			_NetManagers.push_back( nm );
		}
	}

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
	// Return an estimated smooth server tick. Increment only and accelerate/decelerate according to network
	sint32				getCurrentSmoothServerTick() const { return FIRST_MANAGER->getCurrentSmoothServerTick(); }
	// Convert a GameCycle to a SmoothServerTick
	sint32				convertToSmoothServerTick(NLMISC::TGameCycle t) const { return FIRST_MANAGER->convertToSmoothServerTick(t); }
	// @}
	
	
	/// Set the Lag Compensation Time in ms (default: 1000)
	void				setLCT( NLMISC::TTime lct ) { ALL_MANAGERS->setLCT( lct ); }
	
	const NLNET::CInetAddress&	getAddress() { return FIRST_MANAGER->getAddress(); }

	/// Set player's reference position
	void	setReferencePosition(const NLMISC::CVectorD &position) { ALL_MANAGERS->setReferencePosition( position ); }

	const std::vector<CNetworkConnection::CChange>			&getChanges() { return FIRST_MANAGER->getChanges(); }
	
	//void								clearChanges() { ALL_MANAGERS->clearChanges(); }
	
	void			setImpulseCallback(CNetworkConnection::TImpulseCallback callback, void *argument = NULL) { FIRST_MANAGER->setImpulseCallback( callback, argument ); }
	void			setDataBase(CCDBNodeBranch *database) { ALL_MANAGERS->setDataBase( database ); }
	bool			connect(std::string &result) { bool res = false; std::vector<CNetManager*>::iterator inm; for( inm=_NetManagers.begin(); inm!=_NetManagers.end(); ++inm ) if ( (*inm)->connect( result ) ) res = true; return res; }
	CNetworkConnection::TConnectionState	getConnectionState() const { return FIRST_MANAGER->getConnectionState(); }

	float				getMeanPacketLoss() { return FIRST_MANAGER->getMeanPacketLoss(); }
	float				getMeanUpload() { return FIRST_MANAGER->getMeanUpload(); }
	float				getMeanDownload() { return FIRST_MANAGER->getMeanDownload(); }
	
private:

	std::vector<CNetManager*>	_NetManagers;
};


////////////
// GLOBAL //
////////////
extern CNetManagerMulti				NetMngr;
extern CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;


#endif // CL_NET_MANAGER_H

/* End of net_manager.h */
