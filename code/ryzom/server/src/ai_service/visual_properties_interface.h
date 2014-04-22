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




#ifndef RYAI_VISUAL_PROPERTIES_INTERFACE_H
#define RYAI_VISUAL_PROPERTIES_INTERFACE_H

// Includes
/*#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "ai.h"
*/
//#include "ai_entity_id.h"

#include <string>

// the class
class	CVisualPropertiesInterface
{
public:
	// classic init(), update() and release()
	static void init();
	static void update();
	static void release();

	// set different visual properties for an entity
	static	void setName(const TDataSetRow&	dataSetRow, ucstring name);
	
//	static void setMode(CAIEntityId id,MBEHAV::EMode mode);
//	static void setBehaviour(CAIEntityId id,MBEHAV::EBehaviour behaviour);

	// clear all visual property info for an entity when it despawns
	/*static void removeEntity(CAIEntityId id);*/
	static bool UseIdForName;
	static bool ForceNames;
};

#endif
