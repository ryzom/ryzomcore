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



#ifndef NL_WORLD_ENTITY_H
#define NL_WORLD_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/block_memory.h"
#include "nel/misc/debug.h"
#include "nel/misc/time_nl.h"

#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"

#include "gpm_utilities.h"
#include "gpm_defs.h"

#include "game_share/mirror_prop_value.h"
#include "game_share/ryzom_mirror_properties.h"
#include "server_share/msg_gpm_service.h"

#include <deque>

class CCell;
class CPlayerInfos;
class CWorldEntity;

typedef std::list<CWorldEntity*>	TWorldEntityList;

struct CVisionEntry
{

	CVisionEntry()	{ }
	CVisionEntry(CWorldEntity* e, uint32 m, uint32 d) : Entity(e), Mask(m), Distance(d)	{ }

	CWorldEntity*	Entity;
	uint32			Mask;
	uint32			Distance;
};

/**
 * World entity contained all properties positions in world for an entity
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CWorldEntity
{
	friend class NLMISC::CBlockMemory<CWorldEntity>;

public:
	typedef CSimpleSmartPointer<CWorldEntity>		CWorldEntitySmartPointer;

	enum TEntityType
	{
		Player = 0,
		Object,
		Trigger,
		AI,
		Unknown
	};

/*
	enum TVisionState														// Enum of vision state for this entity
	{
		Ready = 0,															// ready to use
		Checked,															// in check
		Seen,																// entity is seen by another
	};
*/

public:

	NLMISC::CEntityId								Id;						// Id of entity
	TDataSetRow										Index;

	CMirrorPropValue1DS<TYPE_POSX>					X;						// Coordinate X in world in unit
	CMirrorPropValue1DS<TYPE_POSY>					Y;						// Coordinate Y in world in unit
	CMirrorPropValue1DS<TYPE_POSZ>					Z;						// Coordinate Z in world in unit
	CMirrorPropValue1DS<TYPE_POSX>					LocalX;					// Local Coordinate X in world in unit
	CMirrorPropValue1DS<TYPE_POSY>					LocalY;					// Local Coordinate X in world in unit
	CMirrorPropValue1DS<TYPE_POSZ>					LocalZ;					// Local Coordinate X in world in unit
	CMirrorPropValue1DS<TYPE_ORIENTATION>			Theta;					// Heading in world
	CMirrorPropValue1DS<TYPE_SHEET>					Sheet;					// Id sheet	of entity
	CMirrorPropValue1DS<NLMISC::TGameCycle>			Tick;					// GameCycle of other properties
	CMirrorPropValue1DS<TYPE_CELL>					Cell;					// Current XY Cell where entity is
	CMirrorPropValue1DS<TYPE_VISION_COUNTER>		VisionCounter;			// Number of times this entity is seen by players
	uint32											PlayersSeeingMe;
	CWorldEntity*									ClosestPlayer;

	CMirrorPropValue1DS<TYPE_WHO_SEES_ME>			WhoSeesMe;

	uint32											PatatEntryIndex;		// The patat entry for the _PatatSubscribeManager

	CCell											*CellPtr;				// pointer on cell where entity is	

	TWorldEntityList::iterator						ListIterator;			// Iterator on entity in world entity list
	TWorldEntityList::iterator						PrimIterator;			// Iterator on entity in prmitived entity list

	uint8											Continent;				// index on the continent on which the player is located
	bool											PosInitialised;			// Pos was initialised by mirror
	bool											UsePrimitive;			// entity uses a primitive normally
	bool											ForceUsePrimitive;		// forces entity to use a primitive temporarily (mount, etc.)
	NLPACS::UMovePrimitive							*Primitive;				// Primitive for collision systeme (PACS)
	NLPACS::UMoveContainer							*MoveContainer;			// MoveContainer entity is in
	uint32											TickLock;

	CWorldEntitySmartPointer						Previous;
	CWorldEntitySmartPointer						Next;

	CPlayerInfos									*PlayerInfos;			// The player infos associated to this entity (if there is some)

	CWorldEntitySmartPointer						Parent;					// Which is entity we are in/on (ex: mount, ferry)
	std::vector<CWorldEntitySmartPointer>			Children;				// Which are the child we contain
	CWorldEntitySmartPointer						Control;				// Which entity is controlling us (ex: rider, pilot...)

	bool											ForceDontUsePrimitive;	// forces entity not to use a primitive temporarily (mount, etc.)
	bool											CheckMotion;
	bool											HasVision;				// Entity has vision

	std::vector<CEntitySheetId>						Content;

	bool											TempVisionState;		// temporary flag for vision delta, telling if the entity is now visible
	bool											TempControlInVision;	// temporary flag for vision delta, telling if the controller entity (if any) is in vision
	bool											TempParentInVision;		// temporary flag for vision delta, telling if the parent (controlled) entity (if any) is in vision

	sint32											RefCounter;				// Number of references on this entity -- used by smart pointer

public:
	/**
	 * destructor
	 */
	~CWorldEntity();

	/**
	 * Init
	 * \param id is entity's CEntityId
	 */
	void	init( const NLMISC::CEntityId& id, const TDataSetRow &index );

	/**
	 * Display debug
	 */
	void	display(NLMISC::CLog *log = NLMISC::InfoLog) const;

	/**
	 * create primitive for fiche type entity
	 * \param ficheId is sheet type Id of entity
	 * \param pMoveContainer adress of the move container
	 * \param worldImage numvber of the world image in which the primitive is to be inserted
	 * \return pointer on PACS primitve
	 */
	void	createPrimitive(NLPACS::UMoveContainer *pMoveContainer, uint8 worldImage);

	/**
	 * Removes primitive allocated previously
	 */
	void	removePrimitive();


	/**
	 * removes entity from the cell it is in
	 */
	//void	removeFromCellAsEntity();

	/**
	 * removes object from the cell it is in
	 */
	//void	removeFromCellAsObject();


	/// Test if entity is linked in a cell
	bool			isLinked() const	{ return CellPtr != NULL; }

	/// Get (const) CCell point in which entity is
	const CCell*	getCell() const		{ return CellPtr; }


	/// Tests if entity uses a pacs primitive
	bool	hasPrimitive() const	{ return Primitive != NULL && !ForceDontUsePrimitive; }

	/// local motion
	bool	localMotion() const		{ return Parent != NULL; }

	/// has control ?
	bool	hasControl() const		{ return Parent != NULL && Parent->Control == this; }

	/// is controlled ?
	bool	isControlled() const	{ return Control != NULL; }

	/// has children
	bool	hasChildren() const		{ return !Children.empty(); }

	/// remove from children
	void	removeFromChildren(CWorldEntity *entity)
	{
		std::vector<CWorldEntitySmartPointer>::iterator	it;
		for (it=Children.begin(); it!=Children.end(); ++it)
			if ((CWorldEntity*)(*it) == entity)
				it = Children.erase(it);

		entity->Parent = NULL;
		Control = NULL;
	}

	/// get controlled
	CWorldEntity	*getControlled()
	{
		if (!hasControl())
			return NULL;

		CWorldEntity	*parent = Parent;
		while (parent->hasControl())
			parent = parent->Parent;

		return parent;
	}

	/// update local or global position
	void	updatePosition(sint32 x, sint32 y, sint32 z, float theta, NLMISC::TGameCycle cycle, bool interior, bool water)
	{
		if (localMotion())
		{
			LocalX = x;
			LocalY = y;
			LocalZ = z;

			setPosition(x + Parent->X(), y + Parent->Y(), z + Parent->Z(), true, interior, water);
		}
		else
		{
			setPosition(x, y, z, false, interior, water);
		}

		Tick = cycle;
		Theta = theta;

		// force position as valid
		PosInitialised = true;
	}

	/// update global position for local motion
	void	updatePosition(bool interior, bool water)
	{
		if (localMotion())
		{
			setPosition(LocalX() + Parent->X(), LocalY() + Parent->Y(), LocalZ() + Parent->Z(), true, interior, water);
		}
	}

	/// Set position
	void	setPosition(sint32 x, sint32 y, sint32 z, bool local, bool interior, bool water)
	{
		X = x;
		Y = y;
		Z = (z&(~7)) + (local ? 1 : 0) + (interior ? 2 : 0) + (water ? 4 : 0);
	}

	/// update position using move primitive
	void	updatePositionUsingMovePrimitive(uint wi);



	/// get Type of the entity
	TEntityType		getType() const
	{
		return _Type;
	}


private:

	/// Is in interior
	bool	interior() const
	{
		return (Z()&2) != 0;
	}

	/// Type of the entity
	TEntityType										_Type;

public:

	/// Creates a new entity (new equivalent). This must be initialised later using init();
	static CWorldEntity*	create();

	/// Removes an entity (delete equivalent).
	static void				remove(CWorldEntity *entity);

protected:
	/**
	 * Default constructor, used because of CBlockMemory
	 */
	CWorldEntity() {}

private:

	/// Static cell allocator
	static NLMISC::CBlockMemory<CWorldEntity>	_EntityAllocator;
};

//
typedef	CWorldEntity::CWorldEntitySmartPointer		CWorldEntityPtr;

/// A list of CWorldEntity, referred by smart pointers. First template param is the pointed type, second param is the pointer storage type (here smart pointer)
typedef	CObjectList<CWorldEntity, CWorldEntityPtr>	TEntityList;


/**
 * Player Infos : contains all information specific to players (like vision and original front end)
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CPlayerInfos
{
	friend class NLMISC::CBlockMemory<CPlayerInfos>;

public:
	/// init
	void	init(const NLMISC::CEntityId &id, NLNET::TServiceId feId, CWorldEntity *entity)
	{
		WhoICanSee = 0xffffffff;

		Next = NULL;
		Previous = NULL;
		ActivateSlot0 = false;
		DesactivateSlot0 = false;
		Slot0Active = false;

		uint	i;
		for (i = MAX_SEEN_ENTITIES-1 ; i > 0; --i)
			FreeSlots.push_back( i );

		for (i = 0 ; i < MAX_SEEN_ENTITIES ; ++i)
			Slots[i] = NULL;

		LastVisionTick = 0;

		_PlayerId = id;
		FeId = feId;
		Entity = entity;

		CheckSpeed = true;
		EnableVisionProcessing = true;
	}

	/// get playerId
	inline const NLMISC::CEntityId &getPlayerId() const { return _PlayerId; }

	/**
	 * Display debug
	 */
	void	display(NLMISC::CLog *log = NLMISC::InfoLog) const;

private:
	/// the player Id
	NLMISC::CEntityId			_PlayerId;

	/// default constructor
	CPlayerInfos()	{ }

public:
	/// original front end Id
	NLNET::TServiceId			FeId;

	/// front end datas
	TMapFrontEndData::iterator	ItFrontEnd;

	/// iterator in the update player list
	TPlayerList::iterator		ItUpdatePlayer;

	/// tick at last vision update
	NLMISC::TGameCycle			LastVisionTick;

	/// Delay vision till cycle
	NLMISC::TGameCycle			DelayVision;

	///
	typedef CUnsafeConstantSizeStack<uint16, MAX_SEEN_ENTITIES+1>	TSlotStack;

	/// list of free slots for vision
	TSlotStack					FreeSlots;

	/// The world entity for this player
	CWorldEntityPtr				Entity;

	/// Previous player in list
	CPlayerInfos*				Previous;
	/// Next player in list
	CPlayerInfos*				Next;

	/// slots for this player
	CWorldEntityPtr				Slots[MAX_SEEN_ENTITIES];

	bool						ActivateSlot0;
	bool						DesactivateSlot0;
	bool						Slot0Active;

	bool						EnableVisionProcessing;

	bool						CheckSpeed;

	/// Who I can see flag field
	uint32						WhoICanSee;

#ifdef RECORD_LAST_PLAYER_POSITIONS
	/// Distance history
	std::deque< std::pair<NLMISC::CVectorD, uint> >		DistanceHistory;
#endif

	float						meanSpeed() const
	{
#ifdef RECORD_LAST_PLAYER_POSITIONS
		float	dist = 0.0f;
		uint	i;
		for (i=0; i+1<DistanceHistory.size(); ++i)
		{
			double	dx = DistanceHistory[i+1].first.x-DistanceHistory[i].first.x,
					dy = DistanceHistory[i+1].first.y-DistanceHistory[i].first.y;

			dist += (float)sqrt(dx*dx + dy*dy);
		}

		return DistanceHistory.size() > 1 ? (dist/(DistanceHistory.size()-1)) : 0.0f;
#else
		return 0.0f;
#endif
	}


	struct CPlayerPos
	{
		NLMISC::TGameCycle			AtTick;
		NLPACS::UGlobalPosition		GPos;
		NLMISC::CVectorD			Motion;
		float						Theta;
	};

	std::deque<CPlayerPos>			PosHistory;

public:
	/// Creates a new entity (new equivalent). This must be initialised later using init();
	static CPlayerInfos	*create()				{ return _PlayerAllocator.allocate(); }

	/// Removes an entity (delete equivalent).
	static void remove(CPlayerInfos *player)	{ _PlayerAllocator.free(player); }

private:

	/// Static cell allocator
	static NLMISC::CBlockMemory<CPlayerInfos>	_PlayerAllocator;
};





#endif // NL_WORLD_ENTITY_H

/* End of world_entity.h */
