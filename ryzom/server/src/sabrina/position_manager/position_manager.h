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



#ifndef RY_POSITION_MANAGER_H
#define RY_POSITION_MANAGER_H

// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
//
#include "sabrina_actor.h"


/**
 * Entities position manager, provides an interface for positions related requests
 * \author Fleury David
 * \author Nevrax France
 * \date 2003
 */
class CPositionManager
{
	/// getInstance
	static inline CPositionManager *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CPositionManager();
	
		return _Instance;
	}

	/**
	 * get all the entities in the given area around a center entity (2D distance test)
	 * \param centerEntity pointer on the entity which is the area center
	 * \param radius selected area radius in meters
	 * \param selectedEntities vector that will receive all the selected entities
	 * \param testLOS flag indicating if we should test LOS (includes only entities which are in center's entity LOS)
	 */
	void getEntitiesInArea( const ISabrinaActor *centerEntity, double radius, std::vector<ISabrinaActor*> &selectedEntities, bool testLOS = true );

	/**
	 * test if an entity is in the line of sight of another entity (3D test always)
	 * \param sourceEntity pointer on the first entity (the one looking)
	 * \param targetEntity pointer on the targeted entity
	 * \return true if the source can see the target, false otherwise
	 */
	bool isInLineOfSight( const ISabrinaActor *sourceEntity, const ISabrinaActor *targetEntity );

private:
	/**
	 * private constructor (singleton)
	 */
	 CPositionManager() {}

private:
	/// unique instance
	static CPositionManager*	_Instance;
};

#endif// RY_POSITION_MANAGER_H
