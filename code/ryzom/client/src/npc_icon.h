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


#ifndef CL_NPC_ICON_H
#define CL_NPC_ICON_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "game_share/msg_client_server.h"
#include "nel/misc/cdb.h"
#include "entity_cl.h"
#include "interface_v3/view_radar.h"
#include <string>
#include <map>
#include <set>

namespace NLMISC
{
	class CBitMemStream;
};

namespace NPC_ICON
{
	// This enum maps TNPCMissionGiverState and adds some new values
	enum TNPCIconId
	{
		IconNone = AwaitingFirstData,
		IconNotAMissionGiver = NotAMissionGiver,
		IconListHasOutOfReachMissions = ListHasOutOfReachMissions,
		IconListHasAlreadyTakenMissions = ListHasAlreadyTakenMissions,
		IconListHasAvailableMission = ListHasAvailableMission,
		IconAutoHasUnavailableMissions = AutoHasUnavailableMissions,
		IconAutoHasAvailableMission = AutoHasAvailableMission,
		IconStepMission,

		NbNPCIcons
	};
};

using NLMISC::ICDBNode;

/**
 * Description of a mission giver NPC.
 * Fed from the server on request by CNPCIconCache.
 */
class CNPCMissionGiverDesc
{
public:

	/// Constructor
	CNPCMissionGiverDesc() : _MissionGiverState(NPC_ICON::AwaitingFirstData), _LastUpdateTimestamp(0), _IsDescTransient(true), _HasChanged(false)
	{}

	// Current assignment operator: bitwise copy

	/// Return the current state
	NPC_ICON::TNPCMissionGiverState		getState() const { return _MissionGiverState; }

	/// Return the time of last update (only valid if getState() != NPC_ICON::AwaitingFirstData)
	NLMISC::TGameCycle					getLastUpdateTimestamp() const { return _LastUpdateTimestamp; }

	/// Return true if we are awaiting a description update
	bool								isDescTransient() const { return _IsDescTransient; }

	/// Set as transient when about to be updated
	void	setDescTransient() { _IsDescTransient = true; }

	/// Return true if state changed since the previous update
	bool								hasChanged() const { return _HasChanged; }

	/// Set if state changed or not
	void	setChanged(bool changed) { _HasChanged = changed; }

	/// Called on receival of update for this NPC. Return true if something has changed.
	bool	updateMissionAvailabilityForThisChar(NPC_ICON::TNPCMissionGiverState state);

	/// Get debug dump
	std::string getDump() const;

private:
	NPC_ICON::TNPCMissionGiverState	_MissionGiverState;
	NLMISC::TGameCycle				_LastUpdateTimestamp;
	bool							_IsDescTransient;
	bool							_HasChanged;
};


typedef uint32 TNPCIconCacheKey;


/**
 * NPC icon management system.
 * Tells which icons to display above NPCs, using data from either the server database
 * (for mission steps) or requesting and caching data from the server (for mission givers).
 */
class CNPCIconCache
{
public:

	/**
	 * Description of an icon (main texture + optional overlay).
	 * The spotId tells which small icon, and in which render layer, to display in the radar.
	 */
	class CNPCIconDesc
	{
	public:
		void				init(const std::string& main, const std::string& over, CViewRadar::TRadarSpotId spotId=CViewRadar::Std) { _TextureMain=main; _TextureOver=over; _SpotId=spotId; }
		const std::string&	getTextureMain() const { return _TextureMain; }
		const std::string&	getTextureOver() const { return _TextureOver; }
		CViewRadar::TRadarSpotId	getSpotId() const { return _SpotId; }

	private:
		std::string					_TextureMain;
		std::string					_TextureOver;
		CViewRadar::TRadarSpotId	_SpotId;
	};

	/// Accessor for singleton
	static CNPCIconCache& getInstance()
	{
		if (!_Instance)
			_Instance = new CNPCIconCache;
		return *_Instance;
	}

	/// Release singleton
	static void release();

	/// Return key by entity (or 0 if entity if icon cache is N/A for this entity)
	inline static TNPCIconCacheKey entityToKey(const CEntityCL *entity)
	{
		return entity->npcAlias();
	}

	/**
	 * Return the filename of the icon to display for the specified NPC, using the following precedence:
	 * 1. Icon for the case when the character has a mission step to do with the NPC (Step icon)
	 * 2. Icon for the case when the character may or may not take a mission with the NPC (Auto or List icon)
	 * 3. No mission related with this NPC (empty strings returned)
	 */
	const CNPCIconCache::CNPCIconDesc&  getNPCIcon(const CEntityCL *entity, bool bypassEnabled = false);

	/// Init the system in either enabled or disabled state
	void		init(bool enabled) { _Enabled = enabled; }

	/// Add DB observers
	void		addObservers();

	/// Remove DB observers
	void		removeObservers();

	/// Request an update about proposed missions of all qualifying npcs in vision, from the server
	void		onEventForMissionAvailabilityForThisChar();

	/// Update routine, to be called from main loop
	void		update();

	/// Enable or disable the system
	void		setEnabled(bool b);

	/// Return the main enabled state
	bool		enabled() const { return _Enabled; }

	/// Get debug dump
	std::string	getDump() const;

protected:

	class CMissionStartStopObserver : public ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(ICDBNode* node);
	}
		MissionStartStopObserver;

	class CMissionNpcAliasObserver : public ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(ICDBNode* node);
	}
		MissionNpcAliasObserver;

	class CMissionPrerequisitEventObserver : public ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(ICDBNode* node);
	}
		MissionPrerequisitEventObserver;

	/// Request an update about proposed missions of the specified npc, from the server.
	void		queryMissionGiverData(TNPCIconCacheKey npcIconCacheKey);
	friend struct commands_queryMissionGiverDataClass;

	/** Request an update about proposed missions on all qualifying visible NPCs, from the server.
	 * If olderThan==0, all qualifying (i.e. that have missions) NPCs are included (and a request
	 * is sent even for transient descriptions), whereas with a positive delay, only NPCs with
	 * information old than Now-olderThan are included and transient descriptions are ignored.
	 */
	void		queryAllVisibleMissionGiverData(NLMISC::TGameCycle olderThan);

	/** Update visible NPCs related to missions in progress, regarding
	 * both missions actors (Step icon) and mission givers' proposed missions (Auto/List icon):
	 * - when a mission is taken (started, new TITLE in database)
	 * - when a mission is completed or failed (FINISHED changed in database)
	 */
	void		onEventForMissionInProgress();
	friend class CMissionStartStopObserver;

	/// Update visible NPCs related to missions in progress, regarding missions actors (Step icon)
	void		onNpcAliasChangedInMissionGoals();
	friend class CMissionNpcAliasObserver;

	/// Called on receival of update for this NPC. Return true if something has changed.
	bool		onReceiveMissionAvailabilityForThisChar(TNPCIconCacheKey npcIconCacheKey, NPC_ICON::TNPCMissionGiverState state);
	friend void impulseSetNpcIconDesc(NLMISC::CBitMemStream &impulse);

	/// Called on receival of new timer delay
	void		setMissionGiverTimer(NLMISC::TGameCycle delay);
	friend void impulseSetNpcIconTimer(NLMISC::CBitMemStream &impulse);

	/// Return true if the specified NPC is involved in current mission goals
	bool		isNPCaCurrentGoal(TNPCIconCacheKey npcIconCacheKey) const;

	/// Store keys of current goals when current goals are changed
	void		storeKeysOfCurrentGoals();

	/// Called when data have been modified
	void		refreshIconsOfScene(bool force = false);


private:

	/// Private constructor (singleton)
	CNPCIconCache();

	typedef std::map<TNPCIconCacheKey, CNPCMissionGiverDesc> CMissionGiverMap;

	/// Map of mission giver descriptions
	CMissionGiverMap			_MissionGivers;

	typedef std::vector<TNPCIconCacheKey> CSmallKeyList;

	/// List of NPCs involved in current goals (typically, contains 0 to 3 elements)
	CSmallKeyList				_KeysOfCurrentGoals;

	/// List of NPCs for which the description is to be requested at the current update
	CSmallKeyList				_DescriptionsToRequest;

	/// Used by update() to do tick-update tasks
	NLMISC::TGameCycle			_LastRequestTimestamp;

	/// Used by update() to do low frequency tasks
	NLMISC::TGameCycle			_LastTimerUpdateTimestamp;

	/// Descriptions of icons for each case
	CNPCIconDesc				_Icons[NPC_ICON::NbNPCIcons];

	/// Main switch to disable the system
	bool						_Enabled;

	// See in .cpp
	static NLMISC::TGameCycle	_CacheRefreshTimerDelay;
	static NLMISC::TGameCycle	_CatchallTimerPeriod;

	/// Instance of singleton
	static CNPCIconCache *_Instance;
};

#endif // CL_NPC_ICON_H