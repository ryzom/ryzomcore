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



#ifndef TICK_S_H
#define TICK_S_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"

#include "nel/net/service.h"
#include "range_mirror_manager.h"
#include "game_share/tick_proxy_time_measure.h"


/**
 * CClientInfos
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CClientInfos
{
public :
	
	/// true if the tock has been received since the last tick
	bool TockReceived;

	/// true if this client is supposed to send a tock
	bool Registered;

	/// true if a tock from this client is not necessary to send a new tick
	bool Tocking;

	/// number of non received tock from this client allowed before freezing time service
	uint16 Threshold;

	/// count of missing tocks 
	uint16 TockMissingCount;
	
	/**
	 * default constructor
	 */
	CClientInfos() : TockReceived(true),Registered(false),Tocking(true),Threshold(0),TockMissingCount(0)
	{}
};




///
class CMirrorGameCycleTimeMeasureMS : public CMirrorGameCycleTimeMeasure
{
public:

	NLNET::TServiceId	MSId;	// not serialised
};


enum TTickServiceTimeMeasureType { PrevTotalTickDuration, NbTickServiceTimeMeasureTypes };

typedef CTimeMeasure<NbTickServiceTimeMeasureTypes> CTickServiceTimeMeasure;


enum TTimeMeasureHistoryStat { MHTSum, MHTMin, MHTMax, NbTimeMeasureHistoryStats };

/*
 *
 */
template <class T>
class CTimeMeasureHistory
{
public:
	NLNET::TServiceId	ServiceId;
	NLNET::TServiceId	ParentServiceId;
	uint16				NbMeasures;
	std::vector< T >	Stats; // indexed by NbTimeMeasureHistoryTypes

	///
	CTimeMeasureHistory( NLNET::TServiceId serviceId, NLNET::TServiceId parentServiceId, bool setFirst, const T *newMeasure=NULL )
	{
		ServiceId = serviceId;
		ParentServiceId = parentServiceId;
		reset( setFirst, newMeasure );
	}

	///
	void	reset( bool setFirst, const T *newMeasure=NULL )
	{
		if ( setFirst )
		{
			NbMeasures = 1;
			Stats.resize( NbTimeMeasureHistoryStats, *newMeasure );
		}
		else
		{
			NbMeasures = 0;
			Stats.resize( NbTimeMeasureHistoryStats );
			Stats[MHTSum] = 0;
			Stats[MHTMin] = std::numeric_limits<T>::max();
			Stats[MHTMax] = 0;
		}
	}

	///
	void	updateStats( const T& newMeasure )
	{
		++NbMeasures;
		for ( uint i=0; i!=newMeasure.size(); ++i )
		{
			Stats[MHTSum][i] += newMeasure[i];
			if ( newMeasure[i] < Stats[MHTMin][i] ) Stats[MHTMin][i] = newMeasure[i];

			//ldebug( "1. NEW: %hu MAX: %hu", newMeasure[i], Stats[MHTMax][i] );
			if ( newMeasure[i] > Stats[MHTMax][i] ) Stats[MHTMax][i] = newMeasure[i];
			//nldebug( "2. NEW: %hu MAX: %hu", newMeasure[i], Stats[MHTMax][i] );
		}
	}
};

typedef CTimeMeasureHistory<CMirrorTimeMeasure> CMirrorTimeMeasureHistory;
typedef CTimeMeasureHistory<CServiceTimeMeasure> CServiceTimeMeasureHistory;
typedef CTimeMeasureHistory<CTickServiceTimeMeasure> CTickServiceMeasureHistory;

/**
 *
 */
class CTickServiceGameCycleTimeMeasure
{
public:

	typedef std::vector<CMirrorGameCycleTimeMeasureMS>	TMirrorMeasures;
	TMirrorMeasures				CurrentMirrorMeasures;
	CTickServiceTimeMeasure		CurrentTickServiceMeasure;

	std::vector< CMirrorTimeMeasureHistory >	HistoryByMirror;
	std::vector< CServiceTimeMeasureHistory >	HistoryByService;
	CTickServiceMeasureHistory					HistoryMain;

	///
	CTickServiceGameCycleTimeMeasure();

	///
	void			beginNewCycle();

	///
	void			resetMeasures();

	///
	void			displayStats( NLMISC::CLog *log );

	///
	void			displayStat( NLMISC::CLog *log, TTimeMeasureHistoryStat stat );

protected:

	template <class HistoryItem, class Measure>
	void storeMeasureToHistory( std::vector<HistoryItem>& history, const Measure& newMeasure, NLNET::TServiceId serviceId, NLNET::TServiceId parentServiceId )
	{
		typename std::vector<HistoryItem>::iterator ihm;

		// Find the right history item
		for ( ihm=history.begin(); ihm!=history.end(); ++ihm )
		{
			if ( (*ihm).ServiceId == serviceId )
				break;
		}
		if ( ihm == history.end() )
		{
			// New in history => add it
			HistoryItem hist( serviceId, parentServiceId, true, &newMeasure );
			history.push_back( hist );
		}
		else
		{
			// Already in history => update stats
			(*ihm).updateStats( newMeasure );
		}
	}
};


/**
 * CTickService
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CTickService : public NLNET::IService
{
public :

	/**
	 * Execution mode : continuous or step by step(user chooses when to send a tick)
	 */
	enum TTickSendingMode	{ Continuous = 0, StepByStep, Fastest };

	/**
	 * State Mode
	 */
	enum TTickStateMode		{ TickRunning = 0, TickHalted };
	
	/// Initialise the service
	void init();

	/// Update
	bool update();

	/// Release
	void release();

	/**
	 *	Register a client
	 * \param serviceId is the unique id of the client service
	 * \param tocking true if we have to wait a tock from this client before to send another tick
	 * \param threshold is the max missing tock allowed
	 */
	void registerClient( NLNET::TServiceId serviceId, bool tocking, uint16 threshold );

	/**
	 * Unregister a client
	 * \param serviceId is the unique id of the client service
	 */
	void unregisterClient( NLNET::TServiceId serviceId );

	/**
	 * halt ticking
	 */
	void	haltTick(const std::string& reason);

	/**
	 * resume ticking
	 */
	void	resumeTick();

	/**
	 * broadcastTick
	 */
	void broadcastTick();

	/**
	 * A registered service sent a tock
	 */
	void addTock( NLNET::TServiceId serviceId );

	/**
	 *	Check if all tock have been received, broadcast a new tick if yes
	 */
	void checkTockReceived();

	/**
	 * give permission to send time in the step by step mode
	 */
	inline void enableSend() { _StepCount++; }

	/**
	 * Get the current game time step
	 */
	inline NLMISC::TGameTime getGameTimeStep() const { return _GameTimeStep; }

	/**
	 *	set the game time step
	 * \param gameTimeStep is the game time step
	 */
	inline void setGameTimeStep( NLMISC::TGameTime gameTimeStep ) { _GameTimeStep = gameTimeStep; }

	/**
	 *	Get the time step between two ticks
	 * \return the time step between 2 ticks
	 */
	inline NLMISC::TLocalTime getTickTimeStep() const { return _TickTimeStep; }

	/**
	 *	set the time step between two ticks
	 * \param timeStep is the time step between 2 ticks
	 */
	inline void setTickTimeStep( NLMISC::TLocalTime timeStep ) { _TickTimeStep = timeStep; }

	/**
	 * Get the current game time
	 * \return current game time
	 */
	inline NLMISC::TGameTime getGameTime() const { return _GameTime; }

	/**
	 * 	number of game cycle elapsed
	 */
	inline NLMISC::TGameCycle getGameCycle() const { return _GameCycle; }

	/**
	 * The tick service and all connected services display its current time
	 */
	void displayGameTime() const;

	/**
	 *	Get the number of registered clients
	 * \return number of registered clients
	 */
	uint16 getClientCount();

	/// Save to file
	bool saveGameCycle();

	/// Load from file
	bool loadGameCycle();
	void tickFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

	bool								FirstTime;

	TTickStateMode						CurrentMode;
	std::string							HaltedReason;

	/// Displayer of recent history
	NLMISC::CLightMemDisplayer			RecentHistory;

	/// Shard timings
	CTickServiceGameCycleTimeMeasure	MainTimeMeasures;

private :

	/// infos about the connected clients
	std::vector< CClientInfos > _ClientInfos;

	/// different from 0 if the service is allowed to send one or multiple ticks ( note : used in step by step only )
	uint16 _StepCount;

	/// time increment value (diff time between two ticks)
	NLMISC::TLocalTime _TickTimeStep;

	/// the value of the time step used in the last tick
	NLMISC::TLocalTime _LastTickTimeStep;

	/// number of game cycle elapsed
	NLMISC::TGameCycle _GameCycle;

	/// Saved game cycle
	NLMISC::TGameCycle _SavedGameCycle;

	/// game time
	NLMISC::TGameTime _GameTime;

	/// delta game time
	NLMISC::TGameTime _GameTimeStep;

	/// true if the value of the game timestep has changed since last tick send
	bool _GameTimeStepHasChanged;

	/// time when the last tick was sent
	NLMISC::TLocalTime _TickSendTime;

	/// Log to recent history
	NLMISC::CLog		_QuickLog;

	/// Row range manager for mirror system
	CRangeMirrorManager	_RangeMirrorManager;
};


#endif //TICK_S_H
