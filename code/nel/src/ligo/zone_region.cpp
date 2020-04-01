// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#include "stdligo.h"
#include "nel/ligo/zone_region.h"

using namespace NLMISC;
using namespace std;

namespace NLLIGO
{

string CZoneRegion::_StringOutOfBound;

// ***************************************************************************
// SZoneUnit
// ***************************************************************************

// ---------------------------------------------------------------------------
CZoneRegion::SZoneUnit::SZoneUnit()
{
	ZoneName = STRING_UNUSED;
	PosX = PosY = 0;
	Rot = Flip = 0;
	SharingMatNames[0] = STRING_UNUSED;
	SharingMatNames[1] = STRING_UNUSED;
	SharingMatNames[2] = STRING_UNUSED;
	SharingMatNames[3] = STRING_UNUSED;
	SharingCutEdges[0] = 0;
	SharingCutEdges[1] = 0;
	SharingCutEdges[2] = 0;
	SharingCutEdges[3] = 0;
}

// ---------------------------------------------------------------------------
void CZoneRegion::SZoneUnit::serial (NLMISC::IStream &f)
{
	f.xmlSerial (ZoneName, "NAME");
	f.xmlSerial (PosX, "X");
	f.xmlSerial (PosY, "Y");
	f.xmlSerial (Rot, "ROT");
	f.xmlSerial (Flip, "FLIP");

	for (uint32 i = 0; i < 4; ++i)
	{
		f.xmlSerial (SharingMatNames[i], "MAT_NAMES");
		f.xmlSerial (SharingCutEdges[i], "CUR_EDGES");
	}
}

// ---------------------------------------------------------------------------
const CZoneRegion::SZoneUnit& CZoneRegion::SZoneUnit::operator=(const CZoneRegion::SZoneUnit&zu)
{
	this->ZoneName	= zu.ZoneName;
	this->PosX		= zu.PosX;
	this->PosY		= zu.PosY;
	this->Rot		= zu.Rot;
	this->Flip		= zu.Flip;
	for (uint32 i = 0; i < 4; ++i)
	{
		this->SharingMatNames[i] = zu.SharingMatNames[i];
		this->SharingCutEdges[i] = zu.SharingCutEdges[i];
	}
	return *this;
}


// ***************************************************************************
// SZoneUnit2
// ***************************************************************************

// ---------------------------------------------------------------------------
CZoneRegion::SZoneUnit2::SZoneUnit2()
{
	DateLow = 0;
	DateHigh = 0;
}

// ---------------------------------------------------------------------------
void CZoneRegion::SZoneUnit2::serial (NLMISC::IStream &f)
{
	/*sint32 version =*/ f.serialVersion (0);

	SZoneUnit::serial (f);
	f.xmlSerial (DateLow, "LOW");
	f.xmlSerial (DateHigh, "HIGH");
}

// ---------------------------------------------------------------------------
const CZoneRegion::SZoneUnit2& CZoneRegion::SZoneUnit2::operator=(const CZoneRegion::SZoneUnit2&zu)
{
	this->ZoneName	= zu.ZoneName;
	this->PosX		= zu.PosX;
	this->PosY		= zu.PosY;
	this->Rot		= zu.Rot;
	this->Flip		= zu.Flip;
	for (uint32 i = 0; i < 4; ++i)
	{
		this->SharingMatNames[i] = zu.SharingMatNames[i];
		this->SharingCutEdges[i] = zu.SharingCutEdges[i];
	}
	this->DateLow	= zu.DateLow;
	this->DateHigh	= zu.DateHigh;
	return *this;
}

// ---------------------------------------------------------------------------
const CZoneRegion::SZoneUnit2& CZoneRegion::SZoneUnit2::operator=(const CZoneRegion::SZoneUnit&zu)
{
	this->ZoneName	= zu.ZoneName;
	this->PosX		= zu.PosX;
	this->PosY		= zu.PosY;
	this->Rot		= zu.Rot;
	this->Flip		= zu.Flip;
	for (uint32 i = 0; i < 4; ++i)
	{
		this->SharingMatNames[i] = zu.SharingMatNames[i];
		this->SharingCutEdges[i] = zu.SharingCutEdges[i];
	}
	this->DateLow	= 0;
	this->DateHigh	= 0;
	return *this;
}

// ***************************************************************************
// CZoneRegion
// ***************************************************************************

// ---------------------------------------------------------------------------
CZoneRegion::CZoneRegion()
{
	_StringOutOfBound = STRING_OUT_OF_BOUND;
	_MinX = _MinY = 0;
	_MaxX = _MaxY = -1;
}

// ---------------------------------------------------------------------------
void CZoneRegion::serial (NLMISC::IStream &f)
{
	f.xmlPush ("LAND");

		sint32 version = f.serialVersion (1);
		f.serialCheck (NELID("DNAL"));

		f.xmlSerial (_MinX, "MIN_X");
		f.xmlSerial (_MinY, "MIN_Y");
		f.xmlSerial (_MaxX, "MAX_X");
		f.xmlSerial (_MaxY, "MAX_Y");

		if (version == 1)
		{
			f.serialCont (_Zones);
		}

		if (version == 0)
		{
			std::vector<SZoneUnit> vZonesTmp;
			f.serialCont (vZonesTmp);
			_Zones.resize (vZonesTmp.size());
			for (uint32 i = 0; i < vZonesTmp.size(); ++i)
				_Zones[i] = vZonesTmp[i];
		}

	f.xmlPop ();
}

// ---------------------------------------------------------------------------
const string &CZoneRegion::getName (sint32 x, sint32 y) const
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return _StringOutOfBound;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].ZoneName;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getPosX (sint32 x, sint32 y) const
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosX;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getPosY (sint32 x, sint32 y) const
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosY;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getRot (sint32 x, sint32 y) const
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Rot;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getFlip (sint32 x, sint32 y) const
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Flip;
	}
}

// ---------------------------------------------------------------------------
uint8 CZoneRegion::getCutEdge (sint32 x, sint32 y, uint8 pos) const
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].SharingCutEdges[pos];
	}
}

// ---------------------------------------------------------------------------
uint32 CZoneRegion::getDate (sint32 x, sint32 y, uint8 lowOrHigh) const // lowOrHigh == 0 -> low
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		if (lowOrHigh == 0)
			return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].DateLow;
		else
			return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].DateHigh;
	}
}

// ---------------------------------------------------------------------------
void CZoneRegion::resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	sint32 i, j;
	vector<SZoneUnit2> newZones;
	SZoneUnit2 zuTmp;

	newZones.resize ((1+newMaxX-newMinX)*(1+newMaxY-newMinY));
	sint32 newStride = 1+newMaxX-newMinX;
	sint32 Stride = 1+_MaxX-_MinX;
	for (j = newMinY; j <= newMaxY; ++j)
	for (i = newMinX; i <= newMaxX; ++i)
	{
		if ((i >= _MinX)&&(i <= _MaxX)&&(j >= _MinY)&&(j <= _MaxY))
		{
			newZones[(i-newMinX)+(j-newMinY)*newStride] = _Zones[(i-_MinX)+(j-_MinY)*Stride];
		}
		else
		{
			newZones[(i-newMinX)+(j-newMinY)*newStride] = zuTmp;
		}
	}
	_MinX = newMinX; _MaxX = newMaxX;
	_MinY = newMinY; _MaxY = newMaxY;
	_Zones = newZones;
}

// ---------------------------------------------------------------------------
void CZoneRegion::basicSet (sint32 x, sint32 y, sint32 PosX, sint32 PosY,  const std::string &ZoneName)
{
	// Do we need to resize ?
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		sint32 newMinX = (x<_MinX?x:_MinX), newMinY = (y<_MinY?y:_MinY);
		sint32 newMaxX = (x>_MaxX?x:_MaxX), newMaxY = (y>_MaxY?y:_MaxY);

		resize (newMinX, newMaxX, newMinY, newMaxY);
	}
	sint32 stride = (1+_MaxX-_MinX); // Nb to go to next line

	_Zones[(x-_MinX)+(y-_MinY)*stride].ZoneName = ZoneName;
	_Zones[(x-_MinX)+(y-_MinY)*stride].PosX = (uint8)PosX;
	_Zones[(x-_MinX)+(y-_MinY)*stride].PosY = (uint8)PosY;
}

// ---------------------------------------------------------------------------
void SPiece::rotFlip (uint8 rot, uint8 flip)
{
	uint8 nTmp;
	sint32 i, j;

	if (flip == 1)
	{
		for (j = 0; j < h; ++j)
		for (i = 0; i < (w/2); ++i)
		{
			nTmp = Tab[i+j*w];
			Tab[i+j*w] = Tab[(w-1-i)+j*w];
			Tab[(w-1-i)+j*w] = nTmp;
		}
	}

	if (rot == 1)
	{
		vector<uint8> TabDest;
		TabDest.resize (Tab.size());
		for (j = 0; j < h; ++j)
		for (i = 0; i < w;  ++i)
			TabDest[j+i*h] = Tab[i+(h-1-j)*w];
		Tab = TabDest;
		i = w;
		w = h;
		h = i;
	}

	if (rot == 2)
	{
		for (j = 0; j < (h/2); ++j)
		for (i = 0; i < w; ++i)
		{
			nTmp = Tab[i+j*w];
			Tab[i+j*w] = Tab[(w-1-i)+(h-1-j)*w];
			Tab[(w-1-i)+(h-1-j)*w] = nTmp;
		}
		if ((h/2)*2 != h)
		{
			j = (h/2);
			for (i = 0; i < (w/2); ++i)
			{
				nTmp = Tab[i+j*w];
				Tab[i+j*w] = Tab[(w-1-i)+j*w];
				Tab[(w-1-i)+j*w] = nTmp;
			}
		}
	}

	if (rot == 3)
	{
		vector<uint8> TabDest;
		TabDest.resize (Tab.size());
		for (j = 0; j < h; ++j)
		for (i = 0; i < w;  ++i)
			TabDest[j+i*h] = Tab[w-1-i+j*w];
		Tab = TabDest;
		i = w;
		w = h;
		h = i;
	}
}

// ***************************************************************************

std::string	CZoneRegion::getSharingMatNames (sint32 x, sint32 y, uint edge)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return _StringOutOfBound;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].SharingMatNames[edge];
	}
}

// ***************************************************************************

uint8 CZoneRegion::getSharingCutEdges (sint32 x, sint32 y, uint edge)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0xff;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].SharingCutEdges[edge];
	}
}

// ***************************************************************************

bool CZoneRegion::setName (sint32 x, sint32 y, const std::string &newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].ZoneName = newValue;
		return true;
	}
}

// ***************************************************************************

bool CZoneRegion::setPosX (sint32 x, sint32 y, uint8 newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosX = newValue;
		return true;
	}
}

// ***************************************************************************

bool CZoneRegion::setPosY (sint32 x, sint32 y, uint8 newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosY = newValue;
		return true;
	}
}

// ***************************************************************************

bool CZoneRegion::setRot (sint32 x, sint32 y, uint8 newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Rot = newValue;
		return true;
	}
}

// ***************************************************************************

bool CZoneRegion::setFlip (sint32 x, sint32 y, uint8 newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Flip = newValue;
		return true;
	}
}

// ***************************************************************************

bool CZoneRegion::setSharingMatNames (sint32 x, sint32 y, uint edge, const std::string &newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].SharingMatNames[edge] = newValue;
		return true;
	}
}

// ***************************************************************************

bool CZoneRegion::setSharingCutEdges (sint32 x, sint32 y, uint edge, uint8 newValue)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return false;
	}
	else
	{
		_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].SharingCutEdges[edge] = newValue;
		return true;
	}
}

// ***************************************************************************


} // namespace NLLIGO
