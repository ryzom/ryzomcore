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




#ifndef FRONTEND_SERVICE_H
#define FRONTEND_SERVICE_H

#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/file.h"
#include "nel/misc/stop_watch.h"
#include "nel/misc/command.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/net/login_cookie.h"
#include "nel/net/service.h"
#include "nel/net/udp_sock.h"

#include "fe_receive_sub.h"
#include "fe_send_sub.h"
#include "fe_types.h"
#include "history.h"
#include "prio_sub.h"
#include "client_id_lookup.h"


#include <vector>
#include <deque>


#define DECLARE_CF_CALLBACK( varname ) \
friend void cfcb##varname( NLMISC::CConfigFile::CVar& var )


class CModuleManager;

namespace CLFECOMMON
{
	class CAction;
}


typedef CHashMap< TDataSetIndex, std::string> TEntityNamesMap;

extern NLMISC::CVariable<bool>			UseWebPatchServer;
extern NLMISC::CVariable<bool>			AcceptClientsAtStartup;
extern NLMISC::CVariable<std::string>	PatchingURLFooter;


// Return the name of an entity (if previously retrieved or "" if no name)
std::string getEntityName( const TDataSetRow& entityIndex );

// Conditional beep
void beepIfAllowed( uint freq, uint duration );

// Disconnection by UserId (requested by the login system)
void cbDisconnectClient (TUid userId, const std::string &reqServiceName);

// send an impulsion to a client
void sendImpulsion( TClientId clientid, NLNET::CMessage& msgin, uint8 channel, const char *extendedMsg, bool forceSingleShot );



//#define MEASURE_SENDING

//extern CDebugDisplayer	FEDebugDisplayer;

/**
 * CFrontEndService, based on IService5
 */
class CFrontEndService : public NLNET::IService, public NLMISC::ICommandsHandler
{
public:

	/// Return the instance of the service
	static CFrontEndService *instance() { return (CFrontEndService*)IService::getInstance(); }
	
	/// Initialization
	virtual void	init();

	/// Release
	virtual void	release();

	/// Update
	virtual bool	update();

	/// After mirror system is ready
	void			postInit();

	CFeReceiveSub	*receiveSub() { return &_ReceiveSub; }
	CFeSendSub		*sendSub()    { return &_SendSub; }
	CHistory		*history()    { return &_History; }
	CEntityContainer& entityContainer() { return _EntityContainer; }

	/// Priority Subsystem
	CPrioSub		PrioSub;

	/// Constructor
	CFrontEndService() :
		ReceiveWatch(10),
		SendWatch(10),
		UserLWatch(10),
		CycleWatch(10),
		UserDurationPAverage(10),
		ProcessVisionWatch(10),
		BackEndRecvWatch1(5),
		BackEndRecvWatch2(5),
		BackEndRecvWatch3(5),
		//HeadWatch(),
		//FillWatch(),
		//SndtWatch(),
		SentActionsLastCycle(0),
		//ScannedPropsLastCycle(0),
		VisibilityDistance(250000), // 250 m
		MonitoredClient(0),
		AcceptClients(false),
		PrioSub(),
		_UpdateDuration(100),
		_DgramLength(0),
		_ClientLagTime(3000),
		_ClientTimeOut(10000),
		_ReceiveSub(),
		_SendSub(),
		_History(),
		_GCCount(0),
		_GCRatio(1)
		{}

	/// Called when there is a tick
	void			onTick();

	/// returns the number of bit that the level can manage (biggest action it can manage)
	uint			getImpulseMaxBitSize(uint level) { return CImpulseEncoder::maxBitSize (level); }

	/// Send an impulse to a given client (0<=level<=3)
	void			addImpulseToClient(TClientId client, CLFECOMMON::CActionImpulsion *action, uint level);

	/// Send an impulse to a given entity, provided it is a client (0<=level<=3)
	void			addImpulseToEntity( const TEntityIndex& entity, CLFECOMMON::CActionImpulsion *action, uint level);

	/// Remove clients that do not send datagrams anymore
	void			updateClientsStates();

	/// Set clients to stalled or "server down" mode (and send stalled msg immediately)
	void			sendServerProblemStateToClients( uint connectionState );

	/// Set clients to synchronize mode
	void			setClientsToSynchronizeState();

	/// Monitor client (disabled if 0)
	void			monitorClient( TClientId id ) { MonitoredClient = id; }

	/// Set game cycle ratio
	void			setGameCycleRatio( sint gcratio ) { _GCRatio = gcratio; _GCCount = 0; }

	/// Callback called when a cookie become acceptable (a new player as logged
	/// in and will connect here soon).
	static void		newCookieCallback(const NLNET::CLoginCookie &cookie);


	/// StopWatch value for stats
	NLMISC::CStopWatch	ReceiveWatch;				// All Receive Sub
	NLMISC::CStopWatch  SendWatch;					// All Send Sub
	NLMISC::CStopWatch	UserLWatch;
	NLMISC::CStopWatch	CycleWatch;
	NLMISC::TMsDuration	UserDurationPAverage;		// Userloop

	//NLMISC::CStopWatch	HeadWatch;				// Sending: Setup header
	//NLMISC::CStopWatch	FillWatch;				// Sending: Filling impulse and prioritized
	//NLMISC::CStopWatch	SndtWatch;				// Sending: Flushing

	NLMISC::CStopWatch	ProcessVisionWatch;			// PrioSub: Process Vision
	NLMISC::CStopWatch	BackEndRecvWatch1;			// Netloop: cbDeltaUpdate
	NLMISC::CStopWatch  BackEndRecvWatch2;			// Netloop: cbDeltaUpdateRemove
	NLMISC::CStopWatch	BackEndRecvWatch3;			// Netlopp: cbDeltaNewVision

	uint32		SentActionsLastCycle;

	NLMISC::TTime	LastTickTime;
	bool			StalledMode;
	NLMISC::TTime	LastStallTime;

	/// Visibility distance
	CLFECOMMON::TCoord	VisibilityDistance;

	/// Flag set at init and when the shard is known as down by a disconnection callback (EGS/IOS)
	bool				ShardDown;

	/// Entity names (debugging purpose)
	TEntityNamesMap		EntityNames;

	/// If not null, output stats about this client (debugging purpose)
	TClientId			MonitoredClient;

	/// Accept Clients
	bool				AcceptClients;

protected:

	/// Initialises module callback and module manager
	void			initModuleManagers();

	DECLARE_CF_CALLBACK( PriorityMode );
	DECLARE_CF_CALLBACK( TotalBandwidth );
	DECLARE_CF_CALLBACK( ClientBandwidth );
	DECLARE_CF_CALLBACK( LimboTimeOut );
	DECLARE_CF_CALLBACK( ClientTimeOut );
	DECLARE_CF_CALLBACK( AllowBeep );
	DECLARE_CF_CALLBACK( GameCycleRatio );
	DECLARE_CF_CALLBACK( CalcDistanceExecutionPeriod );
	DECLARE_CF_CALLBACK( SortPrioExecutionPeriod );
	DECLARE_CF_CALLBACK( DistanceDeltaRatioForPos );
	/*DECLARE_CF_CALLBACK( PositionPrioExecutionPeriod );
	DECLARE_CF_CALLBACK( OrientationPrioExecutionPeriod );
	DECLARE_CF_CALLBACK( DiscreetPrioExecutionPeriod );*/

private:

	CEntityContainer			_EntityContainer;

	/// Update duration (ms)
	uint32						_UpdateDuration;

	/// Length of datagrams
	uint32						_DgramLength;

	/// Lag time to probe for a client that does not send datagrams anymore (in ms)
	uint32						_ClientLagTime;

	/// Time-out to "disconnect" a client that does not send datagrams anymore (in ms)
	uint32						_ClientTimeOut;

	/// Time-out to "disconnect" a client that is in limbo mode and that doesn't ack
	uint32						_LimboTimeOut;

	/// Receive Subsystem
	CFeReceiveSub				_ReceiveSub;

	/// Send Subsystem
	CFeSendSub					_SendSub;

	/// Packet History
	CHistory					_History;

	/// Module managers
	std::vector< ::CModuleManager*>	_ModuleManagers;

	/// Counter for game cycle ratio
	sint						_GCCount;

	/// Game cycle ratio
	sint						_GCRatio;

	virtual const std::string &getCommandHandlerName() const
	{
		static std::string name("fe");
		return name;
	}

	NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CFrontEndService)
		NLMISC_COMMAND_HANDLER_ADD(CFrontEndService, dump, "dump the frontend internal state", "no param");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
};


#endif
