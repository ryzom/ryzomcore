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
#include "fog_of_war.h"

// ****************************************************************************
IFogOfWar::IFogOfWar()
{
	MapWidth = MapHeight = 0;
	MinX = MinY = MaxX = MaxY = 0.0f;
}

// ****************************************************************************
IFogOfWar::~IFogOfWar()
{
}

// ****************************************************************************
void IFogOfWar::explore(float worldPosX, float worldPosY)
{
	uint8 *pData = getData();
	if (pData == NULL)
		return;

	if ((worldPosX < MinX) ||
		(worldPosX > MaxX) ||
		(worldPosY < MinY) ||
		(worldPosY > MaxY))
		return;

	sint16 w = MapWidth;
	sint16 h = MapHeight;

	sint16 bmpPosX = sint16(sint32((w-1) * (worldPosX - MinX) / (MaxX - MinX) + 0.5));
	sint16 bmpPosY = sint16(sint32((h-1) * (worldPosY - MinY) / (MaxY - MinY) + 0.5));

	sint16 wReal = getRealWidth();
	if ((bmpPosX >= 0) && (bmpPosX < w) &&
		(bmpPosY >= 0) && (bmpPosY < h))
	if (pData[bmpPosX+bmpPosY*wReal] == 0)
	{
		pData[bmpPosX+bmpPosY*wReal] = 255;
		explored(bmpPosX, bmpPosY); // Launch callback
	}
}

// ****************************************************************************
void IFogOfWar::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);

	f.serial(MapWidth);
	f.serial(MapHeight);
	f.serial(MinX);
	f.serial(MinY);
	f.serial(MaxX);
	f.serial(MaxY);

	if (f.isReading())
		if ((MapWidth != 0) && (MapHeight != 0))
			createData(MapWidth, MapHeight);

	if (getData() == NULL)
	{
		nlwarning("cannot save fog of war texture");
		return;
	}

	for (uint16 i = 0; i < MapHeight; ++i)
		f.serialBuffer(getData()+getRealWidth()*i, MapWidth);
}

