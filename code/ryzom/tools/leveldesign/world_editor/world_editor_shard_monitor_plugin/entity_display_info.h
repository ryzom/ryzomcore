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

#ifndef RY_ENTITY_DISPLAY_INFO_H
#define RY_ENTITY_DISPLAY_INFO_H


#include "../../../../common/src/game_share/ryzom_entity_id.h"
#include "nel/misc/rgba.h"


// Icon for an entity, may be NULL icon is X is -1 or Y is -1
class CEntityIcon
{
public:
	CEntityIcon(sint x = -1, sint y = -1) : X(x), Y(y) {}
	sint X; // unit is ENTITY_ICON_SIZE pixels
	sint Y;
};

// various display mode for entities
enum TEntityDisplayMode 
{ 
	EntityType = 0, 
	EntityAlive, 
	EntityHitPoints,
	EntityMode,
	EntityDisplayModeCount
};


class CEntityDisplayInfo
{
public:
	uint				Value;
	const char			*Name;
	CEntityIcon			Icon;	
	NLMISC::CRGBA		Color;
	bool				Visible;
	CEntityDisplayInfo() {}
	CEntityDisplayInfo(uint value, const char *name, NLMISC::CRGBA color, sint iconX = -1, sint iconY = -1) 
		              : Value(value), Name(name), Color(color), Visible(true), Icon(iconX, iconY) {}
};

typedef std::vector<CEntityDisplayInfo> TEntityDisplayInfoVect;


const std::string ENTITY_ICONS_BITMAP = "entity_icons.tga";
const uint		  ENTITY_ICON_SIZE = 24;


#endif