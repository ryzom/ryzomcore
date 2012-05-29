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



#ifndef RY_WORLD_POSITION_MANAGER_H
#define RY_WORLD_POSITION_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/block_memory.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_collision_desc.h"
#include "nel/pacs/u_global_retriever.h"
#include "nel/net/message.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/player_vision_delta.h"
#include "server_share/continent_container.h"
#include "game_share/tick_event_handler.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/ryzom_mirror_properties.h"

#include "gpm_utilities.h"
#include "patat_subscribe_manager.h"


#include "variables.h"
#include "world_entity.h"
#include "cell.h"



#include <map>
#include <set>
#include <vector>
#include <list>

class CWorldPositionManager;
class ConstIteratorType;





/*
 * Temporary storage for positions received when entities not in mirror yet (workaround)
 */
/*
struct TEarlyPosition
{
	TEarlyPosition( bool takeFromMirror=true, sint32 x=0, sint32 y=0, sint32 z=0, float th=0.0f, NLMISC::TGameCycle tk=0 ) : TakeFromMirror(takeFromMirror), X(x), Y(y), Z(z), Theta(th), Tick(tk) {}

	sint32				X, Y, Z;
	float				Theta;
	NLMISC::TGameCycle	Tick;
	bool				TakeFromMirror;
};


typedef std::map< NLMISC::CEntityId, TEarlyPosition > CEarlyPositionMap;

extern CEarlyPositionMap EarlyPositions;
*/

/*
 * HANDLE_SLOT0_SPECIAL:
 * - defined: the slot0 is in a player's vision only in combat mode (called by activeSelfSlot callbacks)
 * - undefined: the slot0 is always in a player's vision
 */
#undef HANDLE_SLOT0_SPECIAL

extern uint32		NumEntities;
extern uint32		NumPlayers;

extern bool			EVSUp;










/**
 * World position manager, in charge of position of entities in world, her cells attachement, and cell / position updated fast acces.
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CWorldPositionManager
{
private:

	/// List of entity
	typedef std::list< CWorldEntityPtr >		TEntitySTLList;

	/// Info about the service that subscribed for this entity environment
	struct CAroundSubscriberInfo
	{
		/// The service id
		NLNET::TServiceId						ServiceId;
		/// The list of property it subscribed to (useless)
		//std::list< std::string >				Properties;
	};

	/// Info about entities around an entity
	struct CAroundEntityInfo
	{
		/// The list of entities around
		TEntitySTLList							WorldEntitiesAround;	// complex structure to ease both property emitter and vision
		/// The list of services that subscribed for this entity
		std::list< CAroundSubscriberInfo >		Subscribers;
	};

public:
	friend void				CWorldEntity::createPrimitive(NLPACS::UMoveContainer *pMoveContainer , uint8 worldImage);
	friend void				CWorldEntity::init( const NLMISC::CEntityId& id, const TDataSetRow &index );
	friend CWorldEntity*	CWorldEntity::create();
	friend void				CWorldEntity::remove(CWorldEntity* entity);
	//friend void				CWorldEntity::createPrimitive(NLPACS::UMoveContainer *pMoveContainer, uint8 worldImage);
	friend void				CWorldEntity::removePrimitive();

	class CEntityIdHash
	{
	public:
		size_t	operator () ( const NLMISC::CEntityId &id ) const { return (uint32)id.getShortId(); }
	};

	/// Container of entities (all entities are referenced by this container
	typedef CHashMap< NLMISC::CEntityId, CWorldEntity *, CEntityIdHash >		TWorldEntityContainerByEId;
	typedef CHashMap<TDataSetRow, CWorldEntity *, TDataSetRow::CHashCode>		TWorldEntityContainer;

	typedef CCell																	**TWorldCellsMap;

	struct CCellOffset
	{
		sint32		Offset;
		uint32		Mask;
		uint32		Distance;
	};

	//typedef std::vector< std::pair< sint32, uint32 > >							TCellOffsetContainer;
	typedef std::vector< CCellOffset >												TCellOffsetContainer;

	typedef std::vector< CCell* >													TIndoorCellContainer;

	typedef std::list< CWorldEntity * >												TRemovedEntityContainer;
	typedef std::set< NLMISC::CEntityId >											TSetId;
	typedef CHashMap< NLMISC::CEntityId, CAroundEntityInfo, CEntityIdHash >	TEntitiesAroundEntityContainer;

	//typedef std::hash_map<NLMISC::CEntityId, CPlayerInfos*, CEntityIdHash >			TMapIdToPlayerInfos;

	typedef std::map<std::string, NLPACS::UPrimitiveBlock*>							TPacsPrimMap;

	struct CPrimBlock
	{
		NLPACS::UMoveContainer					*MoveContainer;
		std::vector<NLPACS::UMovePrimitive*>	MovePrimitives;
	};

	typedef std::map<std::string, CPrimBlock >										TPrimBlockMap;

	enum { CELL_SIZE = 16000 };
private:

	static TWorldEntityContainer		_EntitiesInWorld;
	static TWorldEntityContainerByEId	_EntitiesInWorldByEId;
	static TWorldEntityList				_EntityList;
	static TWorldEntityList				_PrimitivedList;

	/// Cells map variables
	static TWorldCellsMap			_WorldCellsMap;				// the entire cell container
	static TWorldCellsMap			_WorldCellsEffectiveMap;	// the container (pointer to the previous) of effective cells
	static TIndoorCellContainer		_IndoorCellContainer;
	static uint32					_WorldMapX,
									_WorldMapEffectiveX,
									_WorldMapY,
									_WorldMapEffectiveY;
	static TCellOffsetContainer		_VisionCellOffsets;
	static TCellOffsetContainer		_ObjectVisionCellOffsets;

	//
	static TMapFrontEndData			_MapFrontEndData;
	static TMapServiceData			_MapServiceData;
	static sint32					_TotalPlayers;

	/// the association PlayerId/PlayersInfos. PlayerInfos contains player's Vision and original Front End Id
	//static TMapIdToPlayerInfos		_Players;
	static TRemovedEntityContainer	_RemovedEntities;
	static CObjectList<CWorldEntity>	_OutOfVisionEntities;
	static TPlayerList				_UpdatePlayerList;

	///
	static TPacsPrimMap				_PacsPrimMap;
	static TPrimBlockMap			_PrimBlockMap;

	/// Selected cells
	static std::list< CCell * >		_SelectedCells;			// Cells selected by one of select methode

	static TEntitiesAroundEntityContainer _EntitiesAround;	// asked list of entities around an entity for mirror properties delta update

	static CContinentContainer		_ContinentContainer;

	static uint32					_CellSize;				// Cell size in coordinate unit

	static double					_PrimitiveMaxSize;
	static uint8					_NbWorldImages;			// Number images in world for collide management
	static uint8					_NbDynamicWorldImages;	// Number of dynamic world image (must be _NbWorldImages - _FirstDynamicWorldImage )
	static uint8					_FirstDynamicWorldImage;// First dynamique world image;
	static uint8					_CurrentWorldImage;		// Current world image

	static uint16					_NbVisionPerTick;		// Number of visions computed per tick

	static CPatatSubscribeManager	_PatatSubscribeManager;
	static float					_fXMin;
	static float					_fYMin;
	static float					_fXMax;
	static float					_fYMax;

	static TCellOffsetContainer		_VisionCellOffsetsCheck;
	static TCellOffsetContainer		_ObjectVisionCellOffsetsCheck;

	static bool						_LoadPacsCol;

public:


	/**
	 * Init
	 * \param nbCellX is number cells in X axis
	 * \param nbCellY is number cells in Y axis
	 * \param CellSize is size in external unit (mm)
	 * \param IndiceXSpecialCells is first indice on X axis for special cell (no vision on other cell)
	 */	
	//default values is for 210*297 of 160m landscape zone with 16m x 16m internal cells with coordinate in milimeter, with 10 * 10 cells (10 landscape zones * 10 cells by landscape zone) * nbCellY for appartements
	static void init(	uint32	nbCellX, 
						uint32	nbCellY,
						uint32	visionDistance,
						uint32	PrimitiveMaxSize, 
						uint8	NbWorldImages,
						bool	loadPacsPrims,
						bool    loadPacsCol
					); 


	/**
	 * update, called every tick by the GPMS, 
	 */
	static void update();


	/**
	 * Release
	 */
	static void release();

	static uint32 getCellSize() { return CELL_SIZE; }













	/**
	 * load the cells skim order table used to compute vision
	 * \param visionDistance the max vision distance in millimeters (default : 250000 = 250m)
	 */
	static void loadCellsSkimTableForVision( uint32 visionDistance = 250000);

	/**
	 * Init pacs primitives
	 */
	static void initPacsPrim();

	/**
	 * Create indoor cell using a unique id
	 */
	static void createIndoorCell(sint32 cellId);

	/**
	 * Init patat manager
	 * Loads all prim file in data_leveldesign/leveldesign/World/
	 */
	static void	initPatatManager();

	/**
	 * Loads patats in file
	 */
	static void	loadPatatsInFile(const std::string &file);

	/**
	 * Loads all patats in path
	 */
	static void	loadPatatsInPath(const std::string &path);

	/**
	 * Load a patat manager file
	 */
	static void	loadPatatManagerFile(const std::string &file);

	/**
	 * Save a patat manager file
	 */
	static void	savePatatManagerFile(const std::string &file);

	/**
	 * Add CPrimZone class filter
	 */
	static void	addPrimZoneFilter(const std::string &filter)	{ _PatatSubscribeManager.addPrimZoneFilter(filter); }

	/**
	 * Remove CPrimZone class filter
	 */
	static void	removePrimZoneFilter(const std::string &filter)	{ _PatatSubscribeManager.removePrimZoneFilter(filter); }

	/**
	 * Reset CPrimZone class filter
	 */
	static void	resetPrimZoneFilter()							{ _PatatSubscribeManager.resetPrimZoneFilter(); }


	/**
	 * Subscribe to a trigger
	 */
	static void	triggerSubscribe(NLNET::TServiceId serviceId, const std::string &name, uint16 id);

	/**
	 * Unsubscribe to a trigger
	 */
	static void	triggerUnsubscribe(NLNET::TServiceId serviceId, uint16 id);

	/**
	 * Unsubscribe to a trigger
	 */
	static void	triggerUnsubscribe(NLNET::TServiceId serviceId);


	/**
	 * Create building instance
	 */
	static void	createBuildingInstance(uint8 continent, const std::string &id, const NLMISC::CVectorD &position);

	/**
	 * Instanciate Pacs prim bloc
	 */
	static void	instanciatePacsPrim(const std::string &id, const std::string &file, const NLMISC::CVectorD &pos, float angle);

	/**
	 * Remove Pacs prim bloc
	 */
	static void	removePacsPrim(const std::string &id);

	/**
	 * setObstacle
	 */
	static void	setObstacle(const std::string &id, bool obstacle);






	/**
	 * Load a particular continent
	 * \param name the name of the continent
	 * \param file the pacs filename prefix to use to file loading
	 * \param index the index of the continent to be used
	 */
	static void	loadContinent(const std::string &name, const std::string &file, sint index, bool allowAutoSpawn = true);

	/**
	 * Remove a continent
	 * \param index the index of the continent to be removed
	 */
	static void	removeContinent(sint index);

	/**
	 * Get the continent container
	 */
	static CContinentContainer		&getContinentContainer()	{ return _ContinentContainer; }

	/**
	 * Process PACS triggers
	 */
	static void	processPacsTriggers(NLPACS::UMoveContainer *moveContainer);

	

	/**
	 * Set game tick counter
	 *
	 * \param tick is current tick in game
	 */
	static void setCurrentTick( NLMISC::TGameCycle tick );



	/**
	 * Get the specified entity
	 *
	 * \param id is CEntityId of entity removed
	 * \return the entity, or NULL if not found
	 */
/*	static const CWorldEntity *getEntity( const NLMISC::CEntityId& id )
	{
		const TWorldEntityContainerByEId::iterator it = _EntitiesInWorldByEId.find( id );
		if( it == _EntitiesInWorldByEId.end() )
		{
			if (Verbose)
				nldebug("CWorldPositionManager::getEntity %s not in GPMS", id.toString().c_str() );
			return NULL;
		}
		else
			return (*it).second;
	}// getEntity
*/
	/**
	 * Get the specified entity
	 *
	 * \param index TDataSetRow of entity
	 * \return the entity, or NULL if not found
	 */
	static const CWorldEntity *getEntity(const TDataSetRow& index)
	{
		const TWorldEntityContainer::iterator it = _EntitiesInWorld.find(index);
		if( it == _EntitiesInWorld.end() )
		{
			if (Verbose)
				nldebug("CWorldPositionManager::getEntity E%u not in GPMS", index.getIndex() );
			return NULL;
		}
		else
			return (*it).second;
	}// getEntity

	/**
	 * Get the specified entity
	 *
	 * \param id is CEntityId of entity removed
	 * \return the entity, or NULL if not found
	 */
/*	static CWorldEntity *getEntityPtr( const NLMISC::CEntityId& id )
	{
		const TWorldEntityContainerByEId::iterator it = _EntitiesInWorldByEId.find( id );
		if( it == _EntitiesInWorldByEId.end() )
		{
			if (Verbose)
				nlwarning("CWorldPositionManager::getEntity %s not in GPMS", id.toString().c_str() );
			return NULL;
		}
		else
			return (*it).second;
	}// getEntity
*/
	/**
	 * Get the specified entity
	 *
	 * \param id is CEntityId of entity removed
	 * \return the entity, or NULL if not found
	 */
	static CWorldEntity *getEntityPtr( const TDataSetRow& index )
	{
		const TWorldEntityContainer::iterator it = _EntitiesInWorld.find( index );
		if( it == _EntitiesInWorld.end() )
		{
			if (Verbose)
				nlwarning("CWorldPositionManager::getEntity E%u not in GPMS", index.getIndex() );
			return NULL;
		}
		else
			return (*it).second;
	}// getEntity

	/**
	 * Get entity index
	 */
	static TDataSetRow	getEntityIndex(const NLMISC::CEntityId &id);


	/**
	 * Link entity/object in cell map
	 * This method inserts an entity/object in a cell (first removes it from previous cell if needed).
	 * It uses previous entity context to determine where link entity
	 *
	 * \param entity pointer to the entity/object to link
	 */
	static void	link(CWorldEntity* entity);

	/**
	 * Link entity/object in a specified cell
	 * This method inserts an entity/object in the cell (first removes it from previous cell if needed).
	 *
	 * \param entity pointer to the entity/object to link
	 * \param cell cell id to link in (0 means link using entity coordinates, 1 means no cell, whereas >1 means use indoor cell)
	 */
	static void	link(CWorldEntity* entity, sint32 cell);

	/**
	 * Link entity/object in cell
	 * This method inserts an entity/object in the cell (first removes it from previous cell if needed).
	 *
	 * \param entity pointer to the entity/object to link
	 * \param cell cell pointer (NULL means no cell)
	 */
	static void	link(CWorldEntity* entity, CCell *cell);

	/**
	 * Unlink entity/object of containing cell
	 * This method is a direct alias of link(entity, NULL)
	 *
	 * \param entity pointer to the entity/object to link
	 */
	static void	unlink(CWorldEntity* entity)	{ link(entity, (CCell*)NULL); }

	



	/**
	 * lock()
	 * Locks an entity for numticks
	 */
	static void	lock(const TDataSetRow &index, uint numticks)
	{
		CWorldEntity	*entity = getEntityPtr(index);
		if (entity)
			lock(entity, numticks);
	}

	/**
	 * lock()
	 * Locks an entity for numticks
	 */
	static void	lock(CWorldEntity *entity, uint numticks)
	{
		entity->TickLock = CTickEventHandler::getGameCycle()+numticks;
	}

	/**
	 * Enable/Disable player vision processing
	 */
	static void	setPlayerVisionProcessing(const TDataSetRow &index, bool enabled);




	/** Mirror version of addEntity */
	static void onAddEntity( const TDataSetRow& entityIndex ); //(const NLMISC::CEntityId& id, sint32 x, sint32 y, sint32 z, float theta, uint8 continent, sint32 cell, NLMISC::TGameCycle tick, NLMISC::CSheetId sheet, uint16 frontendId = 0, bool forceAgent = false, bool forceInvisible = false, bool forceObject = false);

	/**
	 * Remove entity of the world position manager
	 *
	 * \param id is CEntityId of entity removed
	 */
	static void cmdRemoveEntity(const TDataSetRow &index);

	/**
	 * remove an entity from its id
	 * \param id id of the entity
	 */
	static void onRemoveEntity(const TDataSetRow &index);

	/**
	 * Remove all entities in the world position manager
	  */
	static void removeAllEntities();




	/**
	 * Update allowed vision per tick for all front ends
	 */
	static void	updateMaxVisions();

	/**
	 * Compute vision for the next set of players, called every tick
	 */
	static void computeVision();

	/**
	 * Set entity content
	 */
	static void setContent(const TDataSetRow &index, const std::vector<CEntitySheetId> &content);

	/**
	 * Activate self slot for player
	 */
	static void	activateSelfSlot(CWorldEntity *entity);

	/**
	 * Desactivate self slot for player
	 */
	static void	desactivateSelfSlot(CWorldEntity *entity);

	/**
	 * Treats and removes player vision for a given entity (FE vision)
	 */
	static void	removePlayerVision(CWorldEntity *entity);

	/**
	 * Treats and removes combat vision for a given entity
	 */
	static void	removeCombatVision(CWorldEntity *entity);

	/**
	 * set the number of visions computed per tick
	 * \param nbVisionPerTick the new number of visions computed per tick
	 */
	inline static void setNbVisionPerTick( uint16 nbVisionPerTick ) { _NbVisionPerTick = nbVisionPerTick; }
	
	/**
	 * set the number of visions computed per tick
	 * \return the number of visions computed per tick
	 */
	inline static uint16 getNbVisionPerTick() { return _NbVisionPerTick; }






	/**
	 * Apply motion coming straigthly from the player
	 * The motion is controlled using PACS, and if the player is controlling a mount, apply motion to mount
	 *
	 * \param entity updated entity pointer
	 * \param x x coordinate of entity
	 * \param y y coordinate of entity
	 * \param z z coordinate of entity
	 * \param theta teta angle of entity
	 * \param tick game tick of update
	 */
	static void	movePlayer(CWorldEntity *entity, sint32 x, sint32 y, sint32 z, float theta, NLMISC::TGameCycle tick, bool forceTick=false );


	/**
	 * Update the position of an entity
	 * The position is taken from the mirror, and all map links are updated
	 */
	static void	updateEntityPosition(CWorldEntity *entity);

	/**
	 * Update the position of an entity, with a given cell
	 * The position is taken from the mirror, and all map links are updated
	 */
	static void	updateEntityPosition(CWorldEntity *entity, sint32 cell);






	/**
	 * Teleports an entity to a specified location
	 *
	 * \param entity the pointer to the entity to teleport
	 * \param x world coordinate to teleport to
	 * \param y world coordinate to teleport to
	 * \param z world coordinate to teleport to
	 * \param continent the index of the continent to teleport to
	 * \param tick the tick at which the entity was teleported
	 */
	static void	teleport(const TDataSetRow &index, sint32 x, sint32 y, sint32 z, float theta, uint8 continent, sint32 cell, NLMISC::TGameCycle tick);

	/**
	 * Teleports an entity to a specified location
	 *
	 * \param entity the pointer to the entity to teleport
	 * \param x world coordinate to teleport to
	 * \param y world coordinate to teleport to
	 * \param z world coordinate to teleport to
	 * \param continent the index of the continent to teleport to
	 * \param tick the tick at which the entity was teleported
	 */
	static void	teleport(CWorldEntity *entity, sint32 x, sint32 y, sint32 z, float theta, uint8 continent, sint32 cell, NLMISC::TGameCycle tick);


	/**
	 * enable entity PACS primitive
	 */
	static void	enablePrimitive(CWorldEntity *entity);

	/**
	 * Disable entity PACS primitive
	 */
	static void	disablePrimitive(CWorldEntity *entity);

	/**
	 * Resets pacs primitive of an entity, according to its current position
	 */
	static void	resetPrimitive(CWorldEntity *entity);



	/**
	 * forces a position in patat subscribe manager
	 */
	static uint32	getPatatIndex(const NLMISC::CEntityId &id, const NLMISC::CVector &pos, uint32 previousEntryIndex)
	{
		return _PatatSubscribeManager.getNewEntryIndex(id, pos, previousEntryIndex);
	}








	/**
	 * Attach a child entity to a father entity
	 *
	 * \param father id of the father entity to attach to
	 * \param child id of the child entity to be attached
	 */
	static void	attach(const TDataSetRow &father, const TDataSetRow &child, sint32 x, sint32 y, sint32 z);

	/**
	 * Detach a child entity of its father entity
	 *
	 * \param child id of the child entity to be detached
	 */
	static void	detach(const TDataSetRow &child);

	/**
	 * Give control of a slave  entity to a master entity
	 *
	 * \param slave id of the slave entity to control
	 * \param master id of the master entity that owns control
	 * \param x local x coordinate of the master over slave
	 * \param y local y coordinate of the master over slave
	 * \param z local z coordinate of the master over slave
	 */
	static void	acquireControl(const TDataSetRow &slave, const TDataSetRow &master, sint32 x, sint32 y, sint32 z);

	/**
	 * Recover control of a slave entity from its master
	 *
	 * \param master id of the master entity that owns control
	 */
	static void	leaveControl(const TDataSetRow &master);







	/**
	 * Clear previous selection
	 */
	static void clearPreviousSelection( void ) { _SelectedCells.clear(); }

	/**
	 * Select one cell by its pointer
	 * \param cell to select
	 */
	static void	selectCell(CCell *pCell);

	/**
	 * Select one cell by indexs
	 * \param x is x index
	 * \param y is y index
	 * \param alwaySelect if true for cell always selected else selected only if flag of cell VisionToOtherCell is true
	 */
	static void selectOneCellByIndex( uint32 x, uint32 y, bool alwaysSelect = true );

	/**
	 * select one cell
	 *
	 * \param x is a coordinate x in cell
	 * \param y is a coordinate y in cell
	 * \param alwaySelect if true for cell always selected else selected only if flag of cell VisionToOtherCell is true
	 */
	static void selectCell( sint32 x, sint32 y, bool alwaysSelect = true );

	/**
	 * select a groupe of cells
	 *
	 * \param x is a start x coordinate selection
	 * \param y is a start x coordinate selection
	 * \param d is distance to select
	 * \param alwaySelect if true for cell always selected else selected only if flag of cell VisionToOtherCell is true
	 */
	static void selectCells( sint32 x, sint32 y, sint32 d, bool alwaysSelect = true );

	/**
	 * select a groupe of cells
	 *
	 * \param x is a start x coordinate selection
	 * \param y is a start x coordinate selection
	 * \param d is distance to select
	 * \param alwaySelect if true for cell always selected else selected only if flag of cell VisionToOtherCell is true
	 */
	static void selectRoundCells( sint32 x, sint32 y, sint32 d, bool alwaysSelect = true );

	/**
	 * select a groupe of cells around an entity
	 *
	 * \param id is identifier of entity around selection
	 * \param alwaySelect if true for cell always selected else selected only if flag of cell VisionToOtherCell is true
	 */
	static void selectCellsAroundEntity(const TDataSetRow &index, bool alwaysSelect = true );








	/**
	 * resquest for received by mirror all update for asked properties of all entities around gived entity
	 * \param id is entity used for determine entities around
	 * \param propertiesName is list of properties subscribe
	 */
	static void requestForEntityAround( NLNET::TServiceId serviceId, const NLMISC::CEntityId& id, const std::list< std::string >& propertiesName );

	/**
	 * unrequest previous request mirror update for entities around another
	 * \param serviceId is service send previous request
	 * \param id is entity used for determine entities around
	 */
	static void unrequestForEntityAround( NLNET::TServiceId serviceId, const NLMISC::CEntityId& id );

	/**
	 * unrequest all previous requests mirror update for entities around another
	 * \param id is entity used for determine entities around
	 */
	static void unrequestAllForEntityAround( const NLMISC::CEntityId& id );


	/**
	 * vision request
	 */
	static void	visionRequest(sint32 x, sint32 y, sint32 range, std::vector<std::pair<NLMISC::CEntityId, sint32> > &entities);

	/**
	 * CEntityIterator purpose to give an accessor interface like STL for access to CWorldEntity* of a selection of cells
	 * The set of cell is intialized by any selects methodes
	 * \author Alain Saffray
	 * \author Nevrax France
	 * \date 2002
	 */
	class CEntityIterator
	{
	public:
		typedef std::list< CCell * >::iterator	TCellIterator;
		typedef CWorldEntity					*TWorldEntityPtr;

		CEntityIterator() {}

		/// begin initalise iteration on the selection
		void begin()
		{
			_CellIterator = CWorldPositionManager::getSelectedCells().begin();

			while (_CellIterator != CWorldPositionManager::getSelectedCells().end() && (_EntityIterator = (*_CellIterator)->getEntitiesList()) == NULL)
				++_CellIterator;
		}

		/// end return end iterator
		bool end() const
		{
			return _CellIterator == CWorldPositionManager::getSelectedCells().end();
		}

		/// operator == , Warning: This implementation of == operator is right only because CWorldPositionManager class is a segleton
		bool operator == ( const CEntityIterator& it ) const
		{ 
			return (_CellIterator == it._CellIterator) && (_EntityIterator == it._EntityIterator);
		}

		/// operator != , Warning: This implementation of != operator is right only because CWorldPositionManager class is a segleton
		bool operator != ( const CEntityIterator& it ) { return !(*this == it); }

		/// operator ++, pre incrementation
		void operator ++ ()
		{
			nlassert(_CellIterator != CWorldPositionManager::getSelectedCells().end() && _EntityIterator != NULL);
			_EntityIterator = _EntityIterator->Next;
			if (_EntityIterator == NULL)
			{
				++_CellIterator;
				while (_CellIterator != CWorldPositionManager::getSelectedCells().end() && (_EntityIterator = (*_CellIterator)->getEntitiesList()) == NULL)
					++_CellIterator;
			}
		}

		/// operator *
		CWorldEntity* operator * ()
		{
			nlassert(_CellIterator != CWorldPositionManager::getSelectedCells().end() && _EntityIterator != NULL);
			return _EntityIterator;
		}

	private:
		TCellIterator				_CellIterator;
		TWorldEntityPtr				_EntityIterator;
	};

	/**
	 * CObjectIterator purpose to give an accessor interface like STL for access to CWorldEntity* of a selection of cells
	 * The set of cell is intialized by any selects methodes
	 * \author Alain Saffray
	 * \author Nevrax France
	 * \date 2002
	 */
	class CObjectIterator
	{
	public:
		typedef std::list< CCell * >::iterator	TCellIterator;
		typedef CWorldEntity					*TWorldEntityPtr;

		CObjectIterator() {}

		/// begin initalise iteration on the selection
		void begin()
		{
			_CellIterator = CWorldPositionManager::getSelectedCells().begin();

			while (_CellIterator != CWorldPositionManager::getSelectedCells().end() && (_ObjectIterator = (*_CellIterator)->getObjectsList()) == NULL)
				++_CellIterator;
		}

		/// end return end iterator
		bool end() const
		{
			return _CellIterator == CWorldPositionManager::getSelectedCells().end();
		}

		/// operator == , Warning: This implementation of == operator is right only because CWorldPositionManager class is a segleton
		bool operator == ( const CObjectIterator& it ) const
		{ 
			return (_CellIterator == it._CellIterator) && (_ObjectIterator == it._ObjectIterator);
		}

		/// operator != , Warning: This implementation of != operator is right only because CWorldPositionManager class is a segleton
		bool operator != ( const CObjectIterator& it ) { return !(*this == it); }

		/// operator ++, pre incrementation
		void operator ++ ()
		{
			nlassert(_CellIterator != CWorldPositionManager::getSelectedCells().end() && _ObjectIterator != NULL);
			_ObjectIterator = _ObjectIterator->Next;
			if (_ObjectIterator == NULL)
			{
				++_CellIterator;
				while (_CellIterator != CWorldPositionManager::getSelectedCells().end() && (_ObjectIterator = (*_CellIterator)->getObjectsList()) == NULL)
					++_CellIterator;
			}
		}

		/// operator *
		CWorldEntity* operator * ()
		{
			nlassert(_CellIterator != CWorldPositionManager::getSelectedCells().end() && _ObjectIterator != NULL);
			return _ObjectIterator;
		}

	private:
		TCellIterator				_CellIterator;
		TWorldEntityPtr				_ObjectIterator;
	};

	inline static std::list< CCell * >& getSelectedCells() { return _SelectedCells; }


private:

	
	/**
	 * compute Cell Vision, 
	 * \param CCell the cell to compute vision on
	 */
	static void computeCellVision( CCell *cell, CVisionEntry* entitiesSeenFromCell, uint &numEntities);

	/**
	 * Update vision for this player
	 */
	static TPlayerList::iterator	updateVisionState(TPlayerList::iterator itpl);

	/**
	 * Set high priority for this player's vision
	 */
	static TPlayerList::iterator	prioritizeVisionState(TPlayerList::iterator itpl);

	/**
	 * set vision to player
	 */
	static void	setCellVisionToEntity( CWorldEntity *entity, CVisionEntry* cellVisionArray, uint numEntities);

	/**
	 * compute Vision, 
	 * \param infos the player info
	 * \param cellVision the vision from the cell
	 */
	static void computePlayerDeltaVision( CPlayerInfos *infos, CVisionEntry* entitiesInCellVision, uint &numEntities, CPlayerVisionDelta &visionDelta );


	/**
	 * move an entity
	 */
	//static void	moveEntity(CWorldEntity *entity, sint32 x, sint32 y, sint32 z, float theta, NLMISC::TGameCycle tick);

	/**
	 * add the entities waiting insertion and delte the entities waiting deletion (when the vision has been computed for everyone)
	 */
	static void processWaitingEntities();

	/**
	 * Update list of asked entities around an entity
	 */
	static void updateEntitiesAround( void );


	
	/**
	 * Check cell bounds
	 */
	static bool		checkCellBounds(uint32 x, uint32 y)
	{
		return (x < _WorldMapX && y < _WorldMapY);
	}

	/**
	 * Get cell by its coordinates
	 */
	static CCell	*getCell(uint32 x, uint32 y)
	{
		// ace ajout a changer
//		if (x >= _WorldMapX) x = _WorldMapX-1;
//		if (y >= _WorldMapY) y = _WorldMapY-1;
		nlassertex( x < _WorldMapX && y < _WorldMapY, ("x=%d y=%d _WorldMapX=%d _WorldMapY=%d", x, y, _WorldMapX, _WorldMapY) );
		return _WorldCellsEffectiveMap[ x + y*_WorldMapEffectiveX ];
	}

	/**
	 * set a cell using its coordinates
	 */
	static void		setCell(uint32 x, uint32 y, CCell *cell)
	{
		// ace ajout a changer
//		if (x >= _WorldMapX) x = _WorldMapX-1;
//		if (y >= _WorldMapY) y = _WorldMapY-1;
		nlassertex( x < _WorldMapX && y < _WorldMapY, ("x=%d y=%d _WorldMapX=%d _WorldMapY=%d", x, y, _WorldMapX, _WorldMapY) );
		nlassert( _WorldCellsEffectiveMap[ x + y*_WorldMapEffectiveX ] == NULL );
		_WorldCellsEffectiveMap[ x + y*_WorldMapEffectiveX ] = cell;
	}

	/**
	 * get a cell offset
	 */
	static uint32	getCellOffset(uint32 x, uint32 y)
	{
		// ace ajout a changer
//		if (x >= _WorldMapX) x = _WorldMapX-1;
//		if (y >= _WorldMapY) y = _WorldMapY-1;
		nlassertex( x < _WorldMapX && y < _WorldMapY, ("x=%d y=%d _WorldMapX=%d _WorldMapY=%d", x, y, _WorldMapX, _WorldMapY) );
		return x + y*_WorldMapEffectiveX;
	}

public:

	/// @name Display methods
	//@{

	/// Display player vision
	static void displayVision( const NLMISC::CEntityId &id, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		const CWorldEntity	*entity = getEntity( getEntityIndex(id) );
		if (entity == NULL || entity->PlayerInfos == NULL)
			return;

		CPlayerInfos	*infos = entity->PlayerInfos;

		//std::map<NLMISC::CEntityId, uint16>::const_iterator itVis;
		
		log->displayNL("Vision of player Id %s", id.toString().c_str() );
		log->displayNL("  %d free slots", infos->FreeSlots.size() );

		uint	i;
		for (i=0; i<MAX_SEEN_ENTITIES; ++i)
		{
			if (infos->Slots[i] == NULL)
				continue;
			log->displayNL("  slot %d: ent %s", i, infos->Slots[i]->Id.toString().c_str());
		}
	}


	/// Display all players visions
	static void displayAllVisions(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		TPlayerList::iterator	it;
		for (it=_UpdatePlayerList.begin(); it!=_UpdatePlayerList.end(); ++it)
			displayVision( (*it)->getPlayerId(), log );
	}


	/// display entities
	static void displayAllEntities(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("Entities in the GPMS (position are in meters!) :");
		for (TWorldEntityContainer::iterator it = _EntitiesInWorld.begin() ; it != _EntitiesInWorld.end() ; ++it)
			displayEntity((*it).second, log);
	}

	/// display entities
	static void displayAllEntitiesFullDebug(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("Entities in the GPMS (position are in meters!) :");
		for (TWorldEntityContainer::iterator it = _EntitiesInWorld.begin() ; it != _EntitiesInWorld.end() ; ++it)
			displayEntityFullDebug((*it).first, log);
	}

	/// display players
	static void displayAllPlayers(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("Players in the GPMS :");
		for (TPlayerList::iterator it=_UpdatePlayerList.begin(); it!=_UpdatePlayerList.end(); ++it)
		{
			CWorldEntity	*entity = (*it)->Entity;
			displayEntity(entity, log);
		}

	}

	/// Display single entity by its id
	static void displayEntityFullDebug(const TDataSetRow &index, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		const CWorldEntity	*entity = getEntity(index);
		if (entity == NULL)
		{
			log->displayNL("Couldn't find entity E%u for debug display");
			return;
		}
		entity->display(log);
	}

	/// Display single entity by its id
	static void displayEntity(const TDataSetRow &index, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("%s", debugString(index).c_str());
	}

	/// Display single entity by its iterator
	static void displayEntity(const CWorldEntity *entity, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("%s", debugString(entity).c_str());
	}


	/// Display single entity by its iterator
	static std::string debugString(const TDataSetRow &index)
	{
		const CWorldEntity	*entity = getEntity(index);
		if (entity == NULL)
			return std::string();
		return debugString(entity);
	}

	/// Display single entity by its iterator
	static std::string debugString(const CWorldEntity *entity)
	{
		if (entity == NULL)
			return "<NULL entity>";

		bool	interior = ((entity->Z() & 0x2) != 0);
		return NLMISC::toString("%s: Pos=(%+9.3f,%+9.3f,%+9.3f) Theta=%+4.2f %s Cell=%08X MeanSpeed=%.1fm/s", entity->Id.toString().c_str(), entity->X()*0.001, entity->Y()*0.001, entity->Z()*0.001, entity->Theta(), (interior ? "Interior" : "Outdoor"), entity->Cell(), (entity->PlayerInfos ? entity->PlayerInfos->meanSpeed() : 0.0));
	}



	/// Display Player pos history
	static void	displayPlayersPosHistory(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("Position history of players:");
		for (TPlayerList::iterator it=_UpdatePlayerList.begin(); it!=_UpdatePlayerList.end(); ++it)
		{
			CPlayerInfos	*player = (*it);

			log->displayNL("Player %s:", player->Entity->Id.toString().c_str());
			uint	i;
			for (i=0; i<player->PosHistory.size(); ++i)
			{
				log->displayNL("[%d] GPos=(%d/%d-%f,%f,%f) Motion=(%f,%f,%f) Theta=%f", player->PosHistory[i].AtTick, player->PosHistory[i].GPos.InstanceId, player->PosHistory[i].GPos.LocalPosition.Surface, player->PosHistory[i].GPos.LocalPosition.Estimation.x, player->PosHistory[i].GPos.LocalPosition.Estimation.y, player->PosHistory[i].GPos.LocalPosition.Estimation.z, player->PosHistory[i].Motion.x, player->PosHistory[i].Motion.y, player->PosHistory[i].Motion.z, player->PosHistory[i].Theta);
			}
		}
	}

	/// get Player pos history
	static std::string	getPlayersPosHistory()
	{
		std::string	result;
		for (TPlayerList::const_iterator it=_UpdatePlayerList.begin(); it!=_UpdatePlayerList.end(); ++it)
		{
			const CPlayerInfos	*player = (*it);

			result += NLMISC::toString(">>>>> Player %s:\n", player->Entity->Id.toString().c_str());
			uint	i;
			for (i=0; i<player->PosHistory.size(); ++i)
			{
				NLMISC::TGameCycle			AtTick = player->PosHistory[i].AtTick;
				NLPACS::UGlobalPosition		GPos = player->PosHistory[i].GPos;
				NLMISC::CVectorD			Motion = player->PosHistory[i].Motion;
				float						Theta = player->PosHistory[i].Theta;
				result += NLMISC::toString("[%d] GPos=(%d/%d-%f,%f,%f) Motion=(%f,%f,%f) Theta=%f\n",
					AtTick,
					GPos.InstanceId,
					GPos.LocalPosition.Surface,
					GPos.LocalPosition.Estimation.x, GPos.LocalPosition.Estimation.y, GPos.LocalPosition.Estimation.z,
					Motion.x, Motion.y, Motion.z,
					Theta);
			}
		}

		return result;
	}



	/// Display info for pacs trigger
	static void	displayPacsTriggers(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_ContinentContainer.displayTriggers(log);
	}

	/// Display info for trigger
	static void	displayTriggers(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_PatatSubscribeManager.displayTriggers(log);
	}

	/// Display info for trigger
	static void	displayTriggerInfo(const std::string &name, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_PatatSubscribeManager.displayTriggerInfo(name, log);
	}

	/// Display info for trigger
	static void	displaySubscribers(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_PatatSubscribeManager.displaySubscribers(log);
	}

	/// Display info for trigger
	static void	displaySubscriberInfo(NLNET::TServiceId service, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_PatatSubscribeManager.displaySubscriberInfo(service, log);
	}

	/// Display info for trigger
	static uint32	getEntryIndex(const NLMISC::CVector &pos)
	{
		return _PatatSubscribeManager.getEntryIndex(pos);
	}

	//@}




	/// Check
	static void		autoCheck(NLMISC::CLog *log = NLMISC::InfoLog);

	static void		displayVisionCells(NLMISC::CLog *log = NLMISC::InfoLog);
};


#endif // RY_WORLD_POSITION_MANAGER_H

/* End of world_position_manager.h */
