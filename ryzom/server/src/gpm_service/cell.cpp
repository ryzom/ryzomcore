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



#include "stdpch.h"
#include "cell.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;


/****************************************************************\
 ****************************************************************
							CCell
 ****************************************************************
\****************************************************************/

// Static cell allocator
CBlockMemory<CCell>			CCell::_CellAllocator;




/*
 * Adds an entity to the cell
 */
void	CCell::add(CWorldEntity* entity)
{
	// check entity is not null
	nlassert(entity != NULL);

	// check entity no longer linked
	nlassert(entity->CellPtr == NULL);
	nlassert(entity->Previous == NULL);
	nlassert(entity->Next == NULL);

	// insert entity in good list, depending on if it is an object or not
	if (entity->getType() == CWorldEntity::Object)
	{
		_ObjectsList.insertAtHead(entity);
	}
	else
	{
		_EntitiesList.insertAtHead(entity);

		if (entity->getType() == CWorldEntity::Player && entity->PlayerInfos != NULL)
		{
			_PlayersList.insertAtHead(entity->PlayerInfos);
		}
	}

	// set CellPtr and Cell id
	entity->CellPtr = this;
	entity->Cell = _CellId;
}



/*
 * Removes an entity from the cell
 */
void	CCell::remove(CWorldEntity* entity)
{
	// check entity is not null
	nlassert(entity != NULL);

	// check entity points to this cell
	nlassert(entity->CellPtr == this);

	// remove entity from good list, depending on if it is an object or not
	if (entity->getType() == CWorldEntity::Object)
	{
		_ObjectsList.remove(entity);
	}
	else
	{
		_EntitiesList.remove(entity);

		if (entity->getType() == CWorldEntity::Player && entity->PlayerInfos != NULL)
		{
			_PlayersList.remove(entity->PlayerInfos);
		}
	}

	// reset CellPtr and Cell id
	entity->CellPtr = NULL;
	//entity->Cell = 0;		// not set at this time, perhaps user somewhere else (CellId 0 might crash!)
}






/****************************************************************\
						updateCell()
\****************************************************************/
/*
void CCell::updateCell( CWorldEntity* pWorldEntity )
{
	// updateCell should not be called for objects
	nlassert(pWorldEntity != NULL && !pWorldEntity->IsStaticObject);

	// if entity didn't changed cell, no update
	if (pWorldEntity->CellPtr == this)
		return;

	// if previously was in a cell, remove it
	if( pWorldEntity->CellPtr != NULL )
		pWorldEntity->CellPtr->removeEntity( pWorldEntity );

	addEntity(pWorldEntity);
}
*/



/****************************************************************\
						addEntity()
\****************************************************************/
/*
void CCell::addEntity( CWorldEntity* pWorldEntity )
{
	nlassert(pWorldEntity != NULL && pWorldEntity->CellPtr == NULL);

	// if entity has vision, update vision stats of the cell
	if (pWorldEntity->HasVision)
	{
		nlassert(pWorldEntity->PlayerInfos != NULL);

		// add player to players list
		_PlayersList.insertAtHead(pWorldEntity->PlayerInfos);
	}

	_EntitiesList.insertAtHead(pWorldEntity);

//	if (pWorldEntity->IsInvisible)
//		_InvisiblesList.insertAtHead(pWorldEntity);
//	else
//		_EntitiesList.insertAtHead(pWorldEntity);

	pWorldEntity->CellPtr = this;
	pWorldEntity->Cell = _CellId;
	//CWorldPositionManager::cellUpdated( this );
}
*/

/****************************************************************\
						addObject()
\****************************************************************/
/*
void CCell::addObject( CWorldEntity* pWorldEntity )
{
	nlassert(pWorldEntity->CellPtr == NULL);

	_ObjectsList.insertAtHead(pWorldEntity);

	pWorldEntity->CellPtr = this;
	pWorldEntity->Cell = _CellId;
	//CWorldPositionManager::cellUpdated( this );
}
*/

/****************************************************************\
						removeEntity()
\****************************************************************/
/*
void CCell::removeEntity( CWorldEntity* pWorldEntity )
{
	nlassert(pWorldEntity != NULL && pWorldEntity->CellPtr == this);

	// if entity has vision
	if (pWorldEntity->HasVision)
	{
		nlassert(pWorldEntity->PlayerInfos != NULL);

		// removes player from players list
		_PlayersList.remove(pWorldEntity->PlayerInfos);
	}

	// finds the entity in entities list, and remove it
	pWorldEntity->removeFromCellAsEntity();

	//CWorldPositionManager::cellUpdated( this );
}
*/

/****************************************************************\
						removeObject()
\****************************************************************/
/*
void CCell::removeObject( CWorldEntity* pWorldEntity )
{
	nlassert(pWorldEntity != NULL && pWorldEntity->CellPtr == this);

	// finds the entity in entities list, and remove it
	pWorldEntity->removeFromCellAsObject();

	//CWorldPositionManager::cellUpdated( this );
}
*/
