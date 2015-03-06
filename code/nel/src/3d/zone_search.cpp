// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include <cstdio>
#include <iostream>

#include "nel/3d/zone_search.h"

using namespace std;


namespace NL3D
{


/**
* Constructor : Initialize some privates members
*/
CZoneSearch::CZoneSearch()
{
	/// Size X is named of AA to ZZ, current size is IB = 26 * 8 + 2  (AA is zone number 1, AZ zone number 26, BA zone number 27...)
	// TMP fix for level designer (nico ...)
	_NbZoneX = 26 * ('Z'-'A') + ('Z'-'A');

	/// Number zones on Y axis of landscape
	_NbZoneY = 297;

	/// Size X of one zone (in meters)
	_SizeZoneX = 160;

	/// Size X of one zone (in meters)
	_SizeZoneY = 160;
}


/**
* Get the zone name corresponding to coordinate
* \param x is axis X coordinate (in meters)
* \param y is axis Y coordinate (in meters)
* \param cx is axis X coordinate of center area (in meters)
* \param cy is axis Y coordinate of center area (in meters)
* \return a pair of the zone name and square distance between zone and center area (in zone unit)
*/
pair<string, uint32> CZoneSearch::getZoneName(uint x, uint y, uint cx, uint cy)
{
	char name[13];

	uint zoneY = y / _SizeZoneY + 1;
	uint zoneX = x / _SizeZoneX;

	uint zoneCenterY = cy / _SizeZoneY + 1;
	uint zoneCenterX = cx / _SizeZoneX;

	uint32 distance = (zoneX - zoneCenterX) * (zoneX - zoneCenterX) + (zoneY - zoneCenterY) * (zoneY - zoneCenterY);

	char firstLetter = zoneX / 26 + 'A';
	char secondLetter = zoneX % 26 + 'A';

	sprintf(name, "%d_%c%c.zonel", zoneY, firstLetter, secondLetter);

	return std::pair<string, uint32>(string(name), distance);
}


/*
* Get a list of zone name around a position
* \param x is axis X ccordinate (in meter)
* \param y is axis Y coordinate (in meter)
* \param sizeArea is area of zone research (in meter)
* \param l is a reference to a list of pair of string and uint32
* \return a liste contained name of all zone around indicated position and and square distance between zone and center area (in zone unit)
*/
void CZoneSearch::getListZoneName(uint x, uint y, uint sizeArea, list<pair<string, uint32> >& l)
{
	sint startPosX, startPosY;
	uint lastPosX, lastPosY, sizeAreaX, sizeAreaY;

	startPosX = x - sizeArea;
	startPosY = y - sizeArea;

	sizeArea += sizeArea;
	sizeAreaX = sizeAreaY = sizeArea;

	if(startPosX < 0)
	{
		sizeAreaX += startPosX;
		startPosX = 0;
	}

	lastPosX = startPosX + sizeAreaX;
	if(lastPosX >= (_NbZoneX * _SizeZoneX))
	{
		sizeAreaX -= _NbZoneX * _SizeZoneX - lastPosX;
		lastPosX = _NbZoneX * _SizeZoneX - 1;
	}

	if(startPosY < 0)
	{
		sizeAreaY += startPosY;
		startPosY = 0;
	}

	lastPosY = startPosY + sizeAreaY;
	if(lastPosY >= (_NbZoneY * _SizeZoneY))
	{
		sizeAreaY -= _NbZoneY * _SizeZoneY - lastPosY;
		lastPosY = _NbZoneY * _SizeZoneY - 1;
	}

	l.clear();

	for(uint i = startPosY; i <= lastPosY; i += _SizeZoneY)
	{
		for(uint j = startPosX; j <= lastPosX; j += _SizeZoneX)
		{
			l.push_back(getZoneName(j, i, x, y));
		}
	}
}

uint16 CZoneSearch::getZoneId (uint x, uint y) const
{
	uint zoneY = y / _SizeZoneY;
	uint zoneX = x / _SizeZoneX;

	return (zoneX&255)+(zoneY<<8);
}

void CZoneSearch::getZonePos (uint16 zoneId, uint &x, uint &y) const
{
	x = _SizeZoneY*(zoneId&255);
	y = _SizeZoneY*(zoneId>>8);
}

void CZoneSearch::getListZoneId (uint x, uint y, uint sizeArea, vector<uint16> &l, const std::vector<uint16> *validZoneIds)
{
	sint startPosX, startPosY;
	uint lastPosX, lastPosY, sizeAreaX, sizeAreaY;

	startPosX = x - sizeArea;
	startPosY = y - sizeArea;

	sizeArea += sizeArea;
	sizeAreaX = sizeAreaY = sizeArea;

	if(startPosX < 0)
	{
		sizeAreaX += startPosX;
		startPosX = 0;
	}

	lastPosX = startPosX + sizeAreaX;
	if(lastPosX >= (_NbZoneX * _SizeZoneX))
	{
		sizeAreaX -= _NbZoneX * _SizeZoneX - lastPosX;
		lastPosX = _NbZoneX * _SizeZoneX - 1;
	}

	if(startPosY < 0)
	{
		sizeAreaY += startPosY;
		startPosY = 0;
	}

	lastPosY = startPosY + sizeAreaY;
	if(lastPosY >= (_NbZoneY * _SizeZoneY))
	{
		sizeAreaY -= _NbZoneY * _SizeZoneY - lastPosY;
		lastPosY = _NbZoneY * _SizeZoneY - 1;
	}

	l.clear();

	for(uint i = startPosY; i <= lastPosY; i += _SizeZoneY)
	{
		for(uint j = startPosX; j <= lastPosX; j += _SizeZoneX)
		{
			uint16 zoneId = getZoneId(j, i);
			if (validZoneIds)
			{
				bool found = false;
				for(uint k = 0; k < validZoneIds->size(); ++k)
				{
					if (zoneId == (*validZoneIds)[k])
					{
						found = true;
						break;
					}
				}
				if (!found) continue;
			}
			l.push_back(zoneId);
		}
	}
}

std::string CZoneSearch::getZoneNameFromId (uint16 zoneid)
{
	char name[16];

	sint	x = zoneid & 255;
	sint	y = zoneid >> 8;
	sprintf (name, "%d_%c%c.zonel", y+1, (char)('A'+(x/26)), (char)('A'+(x%26)));
	return string(name);
}


} // NL3D

