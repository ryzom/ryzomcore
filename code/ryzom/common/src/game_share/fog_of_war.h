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


#ifndef RY_FOG_OF_WAR_H
#define RY_FOG_OF_WAR_H

/**
 * Class to manage the fog of war over a continent
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date June 2004
 */
class IFogOfWar
{

protected:

	sint16 MapWidth, MapHeight;		// Size of the map in pixel
	float MinX, MinY, MaxX, MaxY;	// World Coords (same as continent ones)

public:

	IFogOfWar();
	virtual ~IFogOfWar();

	// Must be implemented in derived class
	// Return a pointer to the data (array of uint8 of size RealWidth * RealHeight)
	// in this array 0 is unexplored place, 255 is explored
	virtual uint8 *getData() = 0;
	// Create the data containing the
	virtual bool createData(sint16 w, sint16 h) = 0;
	// Real width and real height can be different from MapWidth and MapHeight if the Data
	// containing the map is larger (next power of 2 for instance)
	virtual sint16 getRealWidth() { return MapWidth; }
	virtual sint16 getRealHeight() { return MapHeight; }

	// Explore a position
	void explore(float worldPosX, float worldPosY);

	// Callback if new explored pos (used to unlock info, upload texture to VRAM, etc...)
	virtual void explored(sint16 /* mapPosX */, sint16 /* mapPosY */) { }

	float getMinX() const { return MinX; }
	float getMinY() const { return MinY; }
	float getMaxX() const { return MaxX; }
	float getMaxY() const { return MaxY; }
	sint16 getMapWidth() const { return MapWidth; }
	sint16 getMapHeight() const { return MapHeight; }


	// Serialize the map
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

#endif
