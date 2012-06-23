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



#ifndef NL_CELL_H
#define NL_CELL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include "world_entity.h"

/**
 * CCell contained entity in this cell and updated entity
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CCell
{
public:
	/// default constructor
	CCell() : _LastVisionUpdate(0) {}

	/// initialisation
	void	init( sint32 cellId )
	{ 
		_LastVisionUpdate = 0;
		_CellId = cellId;
	}

	/// initialisation
	void	init( uint16 x, uint16 y )
	{
		nlassert(x <= 32767);
		nlassert(y <= 32767);

		_LastVisionUpdate = 0;
		_CellId = (x<<16) + y;
	}


	/// \name CCell content management
	//@{

	/**
	 * Adds an entity to the cell
	 *
	 * \param entity pointer to the entity to add
	 */
	void	add(CWorldEntity* entity);

	/**
	 * Removes an entity from the cell
	 *
	 * \param entity pointer to the entity to remove
	 */
	void	remove(CWorldEntity* entity);

	//@}


	/**
	 * Remove entity of cell
	 *
	 * \param pWorldEntity is pointer of entity's removed from cell
	 */
//	void	removeEntity( CWorldEntity* pWorldEntity );

	/**
	 * Add an entity in the cell
	 */
//	void	addEntity(CWorldEntity* pWorldEntity);

	/**
	 * Add an object in the cell
	 */
//	void	addObject(CWorldEntity* pWorldEntity);

	/**
	 * Add an object in the cell
	 */
//	void	removeObject(CWorldEntity* pWorldEntity);

	/*
	 * Update cell content
	 *
	 * \param pWorldEntity is updated entity
	 * \param pWorldPositionManager is entity position manager in world
	 */
//	void updateCell( CWorldEntity* pWorldEntity );



	/// Get a pointer on the first entity in cell
	CWorldEntity	*getEntitiesList() { return _EntitiesList.getHead(); }

	/// Get a pointer on the first object in cell
	CWorldEntity	*getObjectsList() { return _ObjectsList.getHead(); }

	/// Get a pointer to the first player in cell
	CPlayerInfos	*getPlayersList() { return _PlayersList.getHead(); }


	/// Gets at maximum MAX_SEEN_ENTITIES entities contained in the cell
	CVisionEntry*	addEntities(CVisionEntry* fillPtr, CVisionEntry* endPtr, uint32 cellMask, uint32 distance)
	{
		CWorldEntity	*ent = _EntitiesList.getHead();
		while (ent != NULL && fillPtr < endPtr)
		{
			sint32	mask = cellMask & (sint32)(ent->WhoSeesMe);
			//if (!ent->IsInvisibleToPlayer && mask != 0)
			if (mask != 0)
			{
				fillPtr->Entity = ent;
				fillPtr->Mask = mask;
				fillPtr->Distance = distance;
				++fillPtr;
			}
			ent = ent->Next;
		}
		return fillPtr;
	}
	/// Gets at maximum MAX_SEEN_ENTITIES entities contained in the cell
	CVisionEntry*	addObjects(CVisionEntry* fillPtr, CVisionEntry* endPtr, uint32 cellMask, uint32 distance)
	{
		CWorldEntity	*ent = _ObjectsList.getHead();
		while (ent != NULL && fillPtr < endPtr)
		{
			sint32	mask = cellMask & (sint32)(ent->WhoSeesMe);
			//if (!ent->IsInvisibleToPlayer && mask != 0)
			if (mask != 0)
			{
				fillPtr->Entity = ent;
				fillPtr->Mask = mask;
				fillPtr->Distance = distance;
				++fillPtr;
			}
			ent = ent->Next;
		}
		return fillPtr;
	}

	void				setVisionUpdateCycle(NLMISC::TGameCycle gc) { _LastVisionUpdate = gc; }
	NLMISC::TGameCycle	visionUpdateCycle() { return _LastVisionUpdate; }

	inline uint16		x() const { return (uint16)(_CellId>>16); };
	inline uint16		y() const { return (uint16)(_CellId&0xffff); };
	inline sint32		id() const { return _CellId; }
	inline bool			isIndoor() const { return _CellId < 0; }

	//static TVisionCellContainer	&getVisionCells() { return _VisionCells; }

private:
	//friend void	CWorldEntity::removeFromCellAsEntity();
	//friend void	CWorldEntity::removeFromCellAsObject();
	friend class CWorldPositionManager;

	sint32							_CellId;

	TEntityList						_EntitiesList;		// visible moving entities in cell
	TEntityList						_ObjectsList;		// objects in cell
	CObjectList<CPlayerInfos>		_PlayersList;		// players in cell
//	TEntityList						_InvisiblesList;	// invisible entities in cell

	/// last vision update tick for this cell
	NLMISC::TGameCycle				_LastVisionUpdate;

public:
	/// Creates a new entity (new equivalent). This must be initialised later using init();
	static CCell	*create()				{ return _CellAllocator.allocate(); }

	/// Removes an entity (delete equivalent).
	static void		remove(CCell *cell)		{ _CellAllocator.free(cell); }

private:

	/// Static cell allocator
	static NLMISC::CBlockMemory<CCell>	_CellAllocator;
};


#endif // NL_CELL_H

/* End of cell.h */
