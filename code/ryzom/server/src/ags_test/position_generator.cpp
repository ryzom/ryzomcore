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




#include "position_generator.h"
#include <nel/misc/debug.h>

namespace AGS_TEST
{

// instantiating static class variables
int			CPositionGenerator::_x=0;
int			CPositionGenerator::_y=0;
int			CPositionGenerator::_dx=0;
int			CPositionGenerator::_dy=0;
int			CPositionGenerator::_spacing=0;
std::string CPositionGenerator::_pattern("grid");
int			CPositionGenerator::_linelen=0;
int			CPositionGenerator::_leftonline=0;
std::string CPositionGenerator::_spawnZoneName="";
CPositionGenerator::TSpawnZoneMap	CPositionGenerator::_spawnZones;
CPositionGenerator::TSpawnPointMap	CPositionGenerator::_spawnPoints;


void CPositionGenerator::setPositionInMM(int x, int y)
{
	_x=x;
	_y=y;

	_linelen=0;
	_leftonline=0;
	init();

	nlinfo("Set spawn position to: %d, %d", (_x+500)/1000, (_y+500)/1000);
}

void CPositionGenerator::setPosition(int x, int y)
{
	setPositionInMM(1000*x,1000*y);
}


void CPositionGenerator::setPosition(std::string spawnName)		// name of a spawn point or spawn zone
{
	if (_spawnPoints.find(spawnName)!=_spawnPoints.end())
	{
		setPositionInMM( (int)(_spawnPoints[spawnName].Point.x*1000.0f), (int)(_spawnPoints[spawnName].Point.y*1000.0f) );
	}
	else if (_spawnZones.find(spawnName)!=_spawnZones.end())
	{
		_spawnZoneName=spawnName;
		setPattern("spawnZone");
		NLMISC::CVector v=_spawnZones[_spawnZoneName].getNext();
		setPositionInMM((int)(v.x*1000.0f),(int)(v.y*1000.0f));

		nlinfo("Set spawn Zone to: %s",_spawnZoneName.c_str());
	}
	else
	{
		nlinfo("Unknown spawn zone/point: %s",spawnName.c_str());
	}
}

void CPositionGenerator::setSpacing(int spacing)
{
	_spacing=1000*spacing;
	init();
}

void CPositionGenerator::setPattern(const std::string &pattern)
{
	_pattern=pattern;
	init();
}

void CPositionGenerator::init()
{
	_dx=_spacing;
	_dy=0;

	_linelen=0;
	_leftonline=0;
}

void CPositionGenerator::next(int &x, int &y)
{
	x=_x;
	y=_y;

//	nlinfo("Pattern: %s: dx: %d dy: %d spacing: %d",_pattern.c_str(),_dx,_dy,_spacing);
	if (_pattern==std::string("grid"))
	{
		if (_leftonline)
			_leftonline--;
		else
		{
			if (_dx)
				_linelen++;
			_leftonline=_linelen;
			_dx+=_dy;
			_dy-=_dx;
			_dx+=_dy;
		}
		_x+=_dx;
		_y+=_dy;
	}
	else if (_pattern==std::string("line"))
	{
		_x+=_dx;
		_y+=_dy;
	}
	else if (_pattern==std::string("spawnZone"))
	{
		NLMISC::CVector v=_spawnZones[_spawnZoneName].getNext();
		_x=(int)(v.x*1000.0f);
		_y=(int)(v.y*1000.0f);
	}
	else
		nlwarning("Unknown pattern: %s",_pattern.c_str());
}

void CPositionGenerator::addSpawnZone(const NLLIGO::CPrimZone &zone)
{
	if (_spawnZones[zone.getName()].Points.size()!=0)
	{
		nlinfo("Spawn zone already exists: %s",zone.getName().c_str());
		return;
	}

	// determine the extents of the zone
	float minx=(float)(((uint)~0)>>1);
	float maxx=-minx-1.0f;
	float miny=minx;
	float maxy=maxx;
	for (uint i=0;i<zone.VPoints.size();i++)
	{
		if (zone.VPoints[i].x<minx) minx=zone.VPoints[i].x;
		if (zone.VPoints[i].x>maxx) maxx=zone.VPoints[i].x;
		if (zone.VPoints[i].y<miny) miny=zone.VPoints[i].y;
		if (zone.VPoints[i].y>maxy) maxy=zone.VPoints[i].y;
	}

	// round off the zone extents to centre the bounding box about the zone centre
	const float SPACING=4.0f;
	float xbase= ((maxx-minx)/2.0f+minx);
	float ybase= ((maxx-minx)/2.0f+miny);
	int xcount=(int)((maxx-xbase)/SPACING);
	int ycount=(int)((maxy-ybase)/SPACING);

	// loop through the points in the bounding box seeing whether or not they're in the zone
	for (int xi=-xcount;xi<xcount+1; xi++)
		for (int yi=-ycount;yi<ycount+1; yi++)
		{
			NLMISC::CVector v( xbase+SPACING*(float)xi, ybase+SPACING*(float)yi, 0);
			if (zone.contains(v))
			{
				// we've got a point that is in the zone so add it to the spawn zone point array
				_spawnZones[zone.getName()].Points.push_back(v);
			}
		}

	// add code here to shuffle the points in the zone


	// check that the zone's not empty
	if (_spawnZones[zone.getName()].Points.size()!=0)
	{
		nlinfo("Spawn zone '%s' contains %d spawn points",zone.getName().c_str(), _spawnZones[zone.getName()].Points.size());
	}
	else
	{
		nlwarning("FAILED to generate any points for spawn zone: %s", zone.getName().c_str());
		_spawnZones.erase(zone.getName());
	}

}

std::string CPositionGenerator::getSpawnName()
{
	if (_pattern==std::string("spawnZone"))
		return _spawnZoneName;
	else
	{
		char buf[100];
		sprintf(buf,"%f %f %f",double(_x)*0.001,double(_y)*0.001,0);
		return std::string(buf);
	}
}


} // end of namespace AGS_TEST
