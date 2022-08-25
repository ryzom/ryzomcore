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



#ifndef RY_BUILDING_MANAGER_H
#define RY_BUILDING_MANAGER_H

#include "nel/ligo/primitive.h"
#include "building_enums.h"
#include "egs_pd.h"
#include "game_share/timer.h"

class CGuild;
class CCharacter;
class IBuildingPhysical;
class IRoomInstance;
class CBuildingPhysicalGuild;
class CBuildingPhysicalPlayer;
class IDestination;
struct CBuildingParseData;

static const uint MaxEntryPerLiftPage = 8;


/**
 * Manager for buildings
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CBuildingManager
{
	friend class CBuildingTest;

public:
	///\name Low level/basic features management, interaction with the shard
	//@{
	/// return the singleton instance
	static inline CBuildingManager* getInstance();
	/// init the manager
	static void init();
	/// release the manager
	static void release();
	/// callback for player disconnection
	void playerDisconnects(const CCharacter * user);
	/// callback for GPMS connexion
	void gpmsConnection();
	//@}

	///\name triggers management
	//@{
	/// a user request a trigger destination list
	void addTriggerRequest( const TDataSetRow & rowId, sint32 triggerId );
	/// fill a player trigger page
	void fillTriggerPage(const NLMISC::CEntityId & eId, uint16 clientSession, bool resetPage );
	/// a user left a trigger, remove its request
	void removeTriggerRequest( const TDataSetRow & rowId);
	/// teleport a user through a lift
	void triggerTeleport(CCharacter * user, uint16 index);
	//@}


	///\name rooms / building management
	//@{
	/// remove a guild from the system ( clear its building )
	void removeGuildBuilding( uint32 guildId );
	/// remove a player from the system ( clear its building )
	void removePlayerBuilding( const NLMISC::CEntityId & userId );
	/// register a guild in the system
	void registerGuild( EGSPD::TGuildId guildId, TAIAlias building );
	/// register a player in the system
	void registerPlayer( CCharacter * user );
	/// get a building destination from its alias.
	IBuildingPhysical* getBuildingPhysicalsByAlias( TAIAlias Alias );
	/// get a building destination from its name.
	IBuildingPhysical* getBuildingPhysicalsByName( const std::string & name );
	/// remove a player from a room
	void removePlayerFromRoom( CCharacter * user );
	/// alocate a new room instance. Fills the room cell passed a sparam and returns a pointer on the instance
	IRoomInstance * allocateRoom( sint32 & cellRet, BUILDING_TYPES::TBuildingType type );
	/// get a room instance from its cell
	IRoomInstance * getRoomInstanceFromCell( sint32 cellId )
	{
		if ( !isRoomCell(cellId ) )
			return NULL;
		uint idx = getRoomIdxFromCell(cellId);
		if ( idx >= _RoomInstances.size() )
			return NULL;
		return _RoomInstances[idx].Ptr;
	}
	/// get the default exit zone of a cell
	uint16 getDefaultExitZone( sint32 cellId );
	/// return true if the param cell is a room cell. ( even values < -2 )
	bool isRoomCell( sint32 cellId )
	{
		return ( cellId <= -2 && ( cellId & 0x00000001) == 0 );
	}
	//@}


	///\name building trade management
	//@{
	/// parse a guild caretaker options. Return the destination
	CBuildingPhysicalGuild * parseGuildCaretaker( const std::string & script );
	/// parse a player caretaker options. Return the destination
	CBuildingPhysicalPlayer * parsePlayerCaretaker( const std::string & script );
	/// build a flat option list
	void buildBuildingTradeList(const NLMISC::CEntityId & userId, uint16 session);
	/// buy a player flat option
	void buyBuildingOption(const NLMISC::CEntityId & userId, uint8 idx);
	void buyBuilding(const NLMISC::CEntityId & userId, TAIAlias alias);
	//@}

private:

	///\name Singleton specifics
	//@{
	/// instance of the singleton
	static CBuildingManager * _Instance;
	/// ctor
	CBuildingManager(){}
	/// dtor
	~CBuildingManager(){}
	//@}
	
	/// a teleport trigger structure
	struct CTrigger
	{
		/// the entry destinations
		std::vector< IDestination* >	Destinations;
		/// true if auto teleportation
		bool							AutoTeleport;
	};

	/// an entry of a user request for infos concernig a trigger
	struct CTriggerRequestEntry
	{
		/// destination of the choice entry
		IDestination*					Destination;
		/// owner of the entry ( example : in destination "My Dest", guild number X )
		uint16							OwnerIndex;
	};

	/// a user request for infos concernig a trigger
	struct CTriggerRequest
	{
		CTriggerRequest()
		{
			Timer = NULL;
		}
		uint8								Page;
		uint16								Session;
		std::vector<CTriggerRequestEntry>	Entries;
		CTimer*								Timer;
	};

	///\name structs and members used during parsing only
	//@{
	// parse all building templates
	bool parseBuildingTemplates( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData );
	/// parse triggers to uninstanciated destinations
	bool parseTriggers( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData );
	/// parse "physical building" (see building_template.h for a description
	bool parsePhysicalBuildings( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData );
	/// parse a tp destination ( uninstanciate destinations )
	IDestination * parseTeleportDestination( const NLLIGO::IPrimitive* prim );
	//@}

	/// reallocate the room container
	void reallocRooms();


	/// return a room index from a cell values
	uint getRoomIdxFromCell( sint32 cell )
	{
		return uint(  ( -(cell+2) ) >> 1);
	}
	/// return a room cell from a room idx
	sint32 getRoomCellFromIdx( uint idx )
	{
		return ( -2 - sint32( idx << 1 ) );
	}
	
	/// container of triggers
	CHashMap< sint,CTrigger >	_Triggers;

	/// trigger requests of the players
	typedef CHashMap< TDataSetRow , CTriggerRequest , TDataSetRow::CHashCode > TTriggerRequestCont;
	TTriggerRequestCont		_TriggerRequests;

	/// physical buildings by Alias
	std::map<TAIAlias,IBuildingPhysical*> _BuildingPhysicals;

	/// physical buildings by name
	std::map<std::string,IBuildingPhysical*> _BuildingPhysicalsName;

	/// room instances
	struct CRoomInstanceEntry
	{
		uint32			NextFreeId;
		IRoomInstance*	Ptr;
	};
	std::vector< CRoomInstanceEntry > _RoomInstances;

	/// sheet id of the player building
	NLMISC::CSheetId		_PlayerBuildingSheet;
	/// sheet id of the guild building
	NLMISC::CSheetId		_GuildBuildingSheet;
	/// sheet ids of the rolemasters trade options
//	std::vector< NLMISC::CSheetId > _RoleMasterSheets;
	/// id of the first free building
	uint32					_FirstFreeRoomId;
	///allocation step in the building container
	uint32					_RoomAllocStep;

	/// copied bots in room instances, sorted by alias
	std::multimap<TAIAlias,TDataSetRow> _CopiedBots;
};

class CTriggerRequestTimoutEvent : public CTimerEvent
{
	NL_INSTANCE_COUNTER_DECL(CTriggerRequestTimoutEvent);
public:

	CTriggerRequestTimoutEvent( const TDataSetRow & userRow )
		:_UserRow( userRow ){}
	
	void timerCallback(CTimer* owner)
	{
		H_AUTO(CTriggerRequestTimoutEvent);

		CBuildingManager::getInstance()->removeTriggerRequest( _UserRow );
	}
	
private:
	TDataSetRow _UserRow;
	
};

//----------------------------------------------------------------------------
inline CBuildingManager* CBuildingManager::getInstance()
{
	nlassert( _Instance );
	return _Instance;
}

#endif // RY_BUILDING_MANAGER_H

/* End of building_manager.h */


