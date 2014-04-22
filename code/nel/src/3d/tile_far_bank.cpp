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

#include "nel/3d/tile_far_bank.h"

using namespace NLMISC;

// Define this to force white far texture (debug)
// #define NEL_FORCE_WHITE_FAR_TEXTURE

namespace NL3D {


// ***************************************************************************
// ***************************************************************************
// CTileFarBank::CTileFar.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
const sint CTileFarBank::CTileFar::_Version=0x0;
// ***************************************************************************
void CTileFarBank::CTileFar::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serial version
	(void)f.serialVersion(_Version);

	// Serial pixels
	f.serialCont (_Pixels[diffuse][order0]);
	f.serialCont (_Pixels[diffuse][order1]);
	f.serialCont (_Pixels[diffuse][order2]);
	f.serialCont (_Pixels[additive][order0]);
	f.serialCont (_Pixels[additive][order1]);
	f.serialCont (_Pixels[additive][order2]);

#ifdef NEL_FORCE_WHITE_FAR_TEXTURE
	int size = _Pixels[diffuse][order0].size ();
	_Pixels[diffuse][order0].resize (0);
	_Pixels[diffuse][order0].resize (size, CRGBA::White);
	size = _Pixels[diffuse][order1].size ();
	_Pixels[diffuse][order1].resize (0);
	_Pixels[diffuse][order1].resize (size, CRGBA::White);
	size = _Pixels[diffuse][order2].size ();
	_Pixels[diffuse][order2].resize (0);
	_Pixels[diffuse][order2].resize (size, CRGBA::White);
#endif // NEL_FORCE_WHITE_FAR_TEXTURE
}
// ***************************************************************************
void CTileFarBank::CTileFar::setPixels (TFarType type, TFarOrder order, NLMISC::CRGBA* pixels, uint size)
{
	// Mode alpha ?
	if (type==alpha)
	{
		_Pixels[diffuse][order].resize (size);
		_Pixels[additive][order].resize (size);

		// Copy only the alpha channel
		for (uint p=0; p<size; p++)
		{
			_Pixels[diffuse][order][p].A=pixels[p].A;
			_Pixels[additive][order][p].A=pixels[p].A;
		}
	}
	else
	{
		// Resize this array
		_Pixels[type][order].resize (size);

		// Copy all the channels
		memcpy (&_Pixels[type][order][0], pixels, size*sizeof(NLMISC::CRGBA));
	}
}


// ***************************************************************************
// ***************************************************************************
// CTileFarBank.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CTileFarBank::CTileFarBank()
{
}

// ***************************************************************************
const sint CTileFarBank::_Version=0x0;
// ***************************************************************************
void CTileFarBank::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Write/Check "FAR_BANK" in header of the stream
	f.serialCheck (NELID("_RAF"));
	f.serialCheck (NELID("KNAB"));

	// Serial version
	(void)f.serialVersion(_Version);

	// Serial tiles
	f.serialCont (_TileVector);
}


} // NL3D
