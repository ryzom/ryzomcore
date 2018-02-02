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

#include "nel/3d/tile_bank.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/tile_noise_map.h"

#include "nel/misc/stream.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include <string>

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace	NL3D
{


// ***************************************************************************
// ***************************************************************************
// TileBankLand.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileLand::_Version=0;
// ***************************************************************************
void CTileLand::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(_Version);

	f.serial (_Name);
	f.serialCont (_TileSet);
}
// ***************************************************************************
void CTileLand::addTileSet (const std::string& name)
{
	_TileSet.insert (name);
}
// ***************************************************************************
void CTileLand::removeTileSet (const std::string& name)
{
	_TileSet.erase (name);
}
// ***************************************************************************
void CTileLand::setName (const std::string& name)
{
	_Name=name;
};
// ***************************************************************************


// ***************************************************************************
// ***************************************************************************
// CTileBank.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBank::_Version=4;
// ***************************************************************************
CTileBank::CTileBank ()
{
	// Default _DisplacementMap
	_DisplacementMap.resize (1);

	// Fill it with 0
	_DisplacementMap[0].setEmpty ();
}
// ***************************************************************************
void    CTileBank::serial(NLMISC::IStream &f)
{
	f.serialCheck (std::string ("BANK"));

	sint streamver = f.serialVersion(_Version);

	// Version 1 not compatible
	if (f.isReading())
	{
		if (streamver<2)
			throw EOlderStream(f);
	}

	switch (streamver)
	{
	case 4:
		// Displacement map array
		f.serialCont (_DisplacementMap);
		if (f.isReading())
		{
			// Checks
			nlassert (!_DisplacementMap.empty());

			// Set first empty
			_DisplacementMap[0].setEmpty ();
		}
	case 3:
		// Absolute path
		f.serial (_AbsPath);
	case 2:
		// Serial all containers
		f.serialCont (_LandVector);
		f.serialCont (_TileSetVector);
		f.serialCont (_TileVector);
	}

	// Compute XRef in read mode
	if (f.isReading())
		computeXRef ();

	// If Version<=2, remove diffuse and alpha tiles in transitions
	if (streamver<=2)
	{
		// Must be reading
		nlassert (f.isReading());

		// Reset _AbsPath
		_AbsPath.clear();

		// Remove diffuse and additive in transition
		uint tileCount=(uint)getTileCount ();
		for (uint i=0; i<tileCount; i++)
		{
			int tileSet;
			int number;
			TTileType type;

			// Get xref
			getTileXRef (i, tileSet, number, type);

			// Transition ?
			if (type==transition)
			{
				// Remove diffuse bitmap
				getTileSet(tileSet)->clearTransition ((CTileSet::TTransition)number, CTile::diffuse, *this);

				// Remove alpha bitmap
				getTileSet(tileSet)->clearTransition ((CTileSet::TTransition)number, CTile::alpha, *this);
			}
		}
	}
}
// ***************************************************************************
sint CTileBank::addLand (const std::string& name)
{
	sint last=(sint)_LandVector.size();
	_LandVector.push_back(CTileLand());
	_LandVector[last].setName (name);
	return last;
}
// ***************************************************************************
void CTileBank::removeLand (sint landIndex)
{
	// Check args
	nlassert (landIndex>=0);
	nlassert (landIndex<(sint)_LandVector.size());

	_LandVector.erase (_LandVector.begin ()+landIndex);
}
// ***************************************************************************
sint CTileBank::addTileSet (const std::string& name)
{
	sint last=(sint)_TileSetVector.size();
	_TileSetVector.push_back(CTileSet());
	_TileSetVector[last].setName (name);
	for (int i=0; i<CTileSet::count; i++)
	{
		_TileSetVector[last]._TileTransition[i]._Tile=createTile ();
	}
	return last;
}
// ***************************************************************************
void CTileBank::removeTileSet (sint setIndex)
{
	// Check args
	nlassert (setIndex>=0);
	nlassert (setIndex<(sint)_TileSetVector.size());

	for (int i=0; i<CTileSet::count; i++)
	{
		int index=_TileSetVector[setIndex]._TileTransition[i]._Tile;
		if (index!=-1)
			freeTile (index);
	}
	_TileSetVector.erase (_TileSetVector.begin ()+setIndex);
}
// ***************************************************************************
void CTileBank::clear ()
{
	_LandVector.clear ();
	_TileSetVector.clear ();
	_TileVector.clear ();
	_TileXRef.clear ();
	_DisplacementMap.clear ();
	_AbsPath.clear ();
}
// ***************************************************************************
sint CTileBank::createTile ()
{
	// Look for a free tile
	for (int i=0; i<(sint)_TileVector.size(); i++)
	{
		if (_TileVector[i].isFree())
		{
			_TileVector[i].setFileName (CTile::diffuse, "");
			_TileVector[i].setFileName (CTile::additive, "");
			_TileVector[i].setFileName (CTile::alpha, "");
			return i;
		}
	}

	// Nothing free, add a tile at the end
	_TileVector.push_back (CTile());
	_TileVector[_TileVector.size()-1].setFileName (CTile::diffuse, "");
	_TileVector[_TileVector.size()-1].setFileName (CTile::additive, "");
	_TileVector[_TileVector.size()-1].setFileName (CTile::alpha, "");
	return (sint)_TileVector.size()-1;
}
// ***************************************************************************
void CTileBank::freeTile (int tileIndex)
{
	// Check args
	nlassert (tileIndex>=0);
	nlassert (tileIndex<(sint)_TileVector.size());

	// Free
	_TileVector[tileIndex].freeBlock();

	// Resize tile table
	int i;
	for (i=(sint)_TileVector.size()-1; i>=0; i--)
	{
		if (!_TileVector[i].isFree ())
			break;
	}
	if (i<(sint)_TileVector.size()-1)
		_TileVector.resize (i+1);
}
// ***************************************************************************
sint CTileBank::getNumBitmap (CTile::TBitmap bitmap) const
{
	std::set<std::string> setString;
	for (int i=0; i<(sint)_TileVector.size(); i++)
	{
		if (!_TileVector[i].isFree())
		{
			const std::string &str=_TileVector[i].getRelativeFileName (bitmap);
			if (!str.empty())
			{
				std::vector<char> vect (str.length()+1);
				memcpy (&*vect.begin(), str.c_str(), str.length()+1);
				toLower(&*vect.begin());
				setString.insert (std::string (&*vect.begin()));
			}
		}
	}
	return (sint)setString.size();
}
// ***************************************************************************
void CTileBank::computeXRef ()
{
	// Resize
	_TileXRef.resize (_TileVector.size());

	// Erase number of the tileset in xref
	for (int tile=0; tile<(sint)_TileVector.size(); tile++)
		_TileXRef[tile]._XRefTileSet=-1;

	// Erase number of the tileset in xref
	for (int s=0; s<(sint)_TileSetVector.size(); s++)
	{
		int t;
		CTileSet *tileSet=getTileSet (s);
		for (t=0; t<tileSet->getNumTile128(); t++)
		{
			int index=tileSet->getTile128 (t);
			_TileXRef[index]._XRefTileSet=s;
			_TileXRef[index]._XRefTileNumber=t;
			_TileXRef[index]._XRefTileType=_128x128;
		}
		for (t=0; t<tileSet->getNumTile256(); t++)
		{
			int index=tileSet->getTile256 (t);
			_TileXRef[index]._XRefTileSet=s;
			_TileXRef[index]._XRefTileNumber=t;
			_TileXRef[index]._XRefTileType=_256x256;
		}
		for (t=0; t<CTileSet::count; t++)
		{
			int index=tileSet->getTransition (t)->getTile();
			_TileXRef[index]._XRefTileSet=s;
			_TileXRef[index]._XRefTileNumber=t;
			_TileXRef[index]._XRefTileType=transition;
		}
	}
}
// ***************************************************************************
void CTileBank::xchgTileset (sint firstTileSet, sint secondTileSet)
{
	// Some check
	nlassert ((firstTileSet>=0)&&(firstTileSet<(sint)_TileSetVector.size()));
	nlassert ((secondTileSet>=0)&&(secondTileSet<(sint)_TileSetVector.size()));

	// Xchange the sets
	CTileSet tmp=_TileSetVector[firstTileSet];
	_TileSetVector[firstTileSet]=_TileSetVector[secondTileSet];
	_TileSetVector[secondTileSet]=tmp;
}
// ***************************************************************************
void TroncFileName (char* sDest, const char* sSrc)
{
	const char* ptr=strrchr (sSrc, '\\');
	if (ptr==NULL)
		ptr=strrchr (sSrc, '/');
	if (ptr)
	{
		ptr++;
		strcpy (sDest, ptr);
	}
	else
	{
		strcpy (sDest, sSrc);
	}
}
// ***************************************************************************
// TODO: this is a temporary hack, see if it could be removed
void CTileBank::makeAllPathRelative ()
{
	// For all tiles
	for (sint nTile=0; nTile<(sint)_TileVector.size(); nTile++)
	{
		// Tronc filename
		char sTmpFileName[512];

		// Diffuse
		TroncFileName (sTmpFileName, _TileVector[nTile].getRelativeFileName (CTile::diffuse).c_str());
		_TileVector[nTile].setFileName (CTile::diffuse, sTmpFileName);

		// Additive
		TroncFileName (sTmpFileName, _TileVector[nTile].getRelativeFileName (CTile::additive).c_str());
		_TileVector[nTile].setFileName (CTile::additive, sTmpFileName);

		// Alpha
		TroncFileName (sTmpFileName, _TileVector[nTile].getRelativeFileName (CTile::alpha).c_str());
		_TileVector[nTile].setFileName (CTile::alpha, sTmpFileName);
	}

	// For all displaces
	for (uint i=0; i<_DisplacementMap.size(); i++)
	{
		// Tronc filename
		char sTmpFileName[512];

		TroncFileName (sTmpFileName, _DisplacementMap[i]._FileName.c_str());
		_DisplacementMap[i]._FileName = sTmpFileName;
	}
}


// ***************************************************************************
void CTileBank::makeAllExtensionDDS ()
{
	// For all tiles
	for (sint nTile=0; nTile<(sint)_TileVector.size(); nTile++)
	{
		string		  tmp;
		string::size_type pos;

		// Diffuse
		tmp= _TileVector[nTile].getRelativeFileName (CTile::diffuse);
		pos= tmp.rfind(".tga");
		if (pos == string::npos)
			pos = tmp.rfind(".png");
		if(pos!= string::npos)
		{
			tmp.replace(pos, 4, ".dds");
			_TileVector[nTile].setFileName (CTile::diffuse, tmp);
		}

		// Additive.
		tmp= _TileVector[nTile].getRelativeFileName (CTile::additive);
		pos= tmp.rfind(".tga");
		if (pos == string::npos)
			pos = tmp.rfind(".png");
		if(pos!= string::npos)
		{
			tmp.replace(pos, 4, ".dds");
			_TileVector[nTile].setFileName (CTile::additive, tmp);
		}

		// Alpha.
		tmp= _TileVector[nTile].getRelativeFileName (CTile::alpha);
		pos= tmp.rfind(".tga");
		if (pos == string::npos)
			pos = tmp.rfind(".png");
		if(pos!= string::npos)
		{
			tmp.replace(pos, 4, ".dds");
			_TileVector[nTile].setFileName (CTile::alpha, tmp);
		}

	}

}
// ***************************************************************************
void CTileBank::cleanUnusedData ()
{
	// Clean each tileset
	for (uint i=0; i<_TileSetVector.size(); i++)
	{
		// Clean the tileset
		_TileSetVector[i].cleanUnusedData ();
	}

	// Clear the land vector
	_LandVector.clear();
}
// ***************************************************************************
CTileNoiseMap *CTileBank::getTileNoiseMap (uint tileNumber, uint tileSubNoise)
{
	if (_DisplacementMap.empty())
	{
		// it happens when serial a tile bank with version < 4
		return NULL;
	}

	// Check tile number..
	if (tileNumber<_TileVector.size())
	{
		// Get tileset number
		uint tileSet=_TileXRef[tileNumber]._XRefTileSet;

		// Checks
		if (tileSet<_TileSetVector.size())
		{
			nlassert (tileSubNoise<CTileSet::CountDisplace);
			//nlassert (_TileSetVector[tileSet]._DisplacementBitmap[tileSubNoise]<_DisplacementMap.size());

			if (_TileSetVector[tileSet]._DisplacementBitmap[tileSubNoise]>=_DisplacementMap.size())
				return NULL;

			// Return the tile noise map
			CTileNoise &tileNoise=_DisplacementMap[_TileSetVector[tileSet]._DisplacementBitmap[tileSubNoise]];

			// Not loaded ?
			if (tileNoise._TileNoiseMap==NULL)
			{
				// Load a bitmap
				CTextureFile texture (getAbsPath()+tileNoise._FileName);
				texture.loadGrayscaleAsAlpha (false);
				texture.generate ();
				texture.convertToType (CBitmap::Luminance);

				// Alloc
				tileNoise._TileNoiseMap=new CTileNoiseMap;

				// Good size ?
				if ((texture.getWidth ()==NL3D_TILE_NOISE_MAP_SIZE)&&(texture.getHeight()==NL3D_TILE_NOISE_MAP_SIZE))
				{
					// Copy
					memcpy (tileNoise._TileNoiseMap->Pixels, &texture.getPixels()[0], NL3D_TILE_NOISE_MAP_SIZE*NL3D_TILE_NOISE_MAP_SIZE);

					// Remap lumels
					for (uint i=0; i<NL3D_TILE_NOISE_MAP_SIZE*NL3D_TILE_NOISE_MAP_SIZE; i++)
					{
						tileNoise._TileNoiseMap->Pixels[i]=(sint8)((uint8)tileNoise._TileNoiseMap->Pixels[i]-128);
						if (tileNoise._TileNoiseMap->Pixels[i]==-128)
							tileNoise._TileNoiseMap->Pixels[i]=-127;
					}
				}
				else
				{
					// This is not a normal behaviour.
					string	pathname= getAbsPath()+tileNoise._FileName;
					if( texture.getWidth ()==0 || texture.getHeight ()==0 )
						nlwarning("TileNoiseMap not found: %s.", pathname.c_str());
					else
						nlwarning("Bad TileNoiseMap size: %s.", pathname.c_str());

					// Not good size, copy a static map
					sint8 notGoodSizeForm[NL3D_TILE_NOISE_MAP_SIZE*NL3D_TILE_NOISE_MAP_SIZE]=
					{
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 99, 99, 99, 99, 99, 99, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 99, 99, 99, 99, 99, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 99, 00, 99, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 99, 99, 99, 99, 99, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 99, 99, 99, 99, 99, 99, 99, 99, 00,
						00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
					};

					// Copy
					memcpy (tileNoise._TileNoiseMap->Pixels, notGoodSizeForm, NL3D_TILE_NOISE_MAP_SIZE*NL3D_TILE_NOISE_MAP_SIZE);
				}
			}

			// Return the noise map
			return tileNoise._TileNoiseMap;
		}
	}

	if (_DisplacementMap.empty() || _DisplacementMap[0]._TileNoiseMap)
		return NULL;

	// Checks
	nlassert (_DisplacementMap[0]._TileNoiseMap);
	return _DisplacementMap[0]._TileNoiseMap;
}
// ***************************************************************************
void CTileBank::removeDisplacementMap (uint mapId)
{
	// Checks
	nlassert (mapId<_DisplacementMap.size());

	if (mapId!=0)
	{
		// Check if another tileSet uses it
		uint tileSet;
		for (tileSet=0; tileSet<_TileSetVector.size(); tileSet++)
		{
			// It uses it ?
			uint tile;
			for (tile=0; tile<CTileSet::CountDisplace; tile++)
			{
				// The same ?
				if (_TileSetVector[tileSet]._DisplacementBitmap[tile]==mapId)
					// Stop
					break;
			}
			if (tile!=CTileSet::CountDisplace)
				break;
		}
		if (tileSet==_TileSetVector.size())
		{
			// Remove it
			_DisplacementMap[mapId].reset();

			// Last element ?
			if (mapId==_DisplacementMap.size()-1)
			{
				// Resize the array ?
				while ((mapId>0)&&(_DisplacementMap[mapId]._FileName.empty()))
					_DisplacementMap.resize (mapId--);
			}
		}
	}
}
// ***************************************************************************
uint CTileBank::getDisplacementMap (const string &fileName)
{
	// Lower string
	string lower=toLower(fileName);

	// Look for this texture filename
	uint noiseTile;
	for (noiseTile=0; noiseTile<_DisplacementMap.size(); noiseTile++)
	{
		// Same name ?
		if (lower==_DisplacementMap[noiseTile]._FileName)
			return noiseTile;
	}

	// Look for a free space
	for (noiseTile=0; noiseTile<_DisplacementMap.size(); noiseTile++)
	{
		// Same name ?
		if (_DisplacementMap[noiseTile]._FileName.empty())
			break;
	}
	if (noiseTile==_DisplacementMap.size())
	{
		// Add a tile
		_DisplacementMap.resize (noiseTile+1);
	}

	// Set the file name
	_DisplacementMap[noiseTile]._FileName=lower;

	return noiseTile;
}
// ***************************************************************************
const char* CTileBank::getDisplacementMap (uint noiseMap)
{
	return _DisplacementMap[noiseMap]._FileName.c_str();
}
// ***************************************************************************
void CTileBank::setDisplacementMap (uint noiseMap, const char *newName)
{
	_DisplacementMap[noiseMap]._FileName=newName;
}
// ***************************************************************************
uint CTileBank::getDisplacementMapCount () const
{
	return (uint)_DisplacementMap.size();
}


// ***************************************************************************
const CTileVegetableDesc	&CTileBank::getTileVegetableDesc(uint tileNumber) const
{
	// Check tile number..
	if (tileNumber<_TileVector.size())
	{
		// Get tileset number
		uint tileSet=_TileXRef[tileNumber]._XRefTileSet;

		// Checks
		if (tileSet<_TileSetVector.size())
		{
			return _TileSetVector[tileSet].getTileVegetableDesc();
		}

	}

	// if fails for any reason, return an empty tileVegetableDesc;
	static	CTileVegetableDesc	emptyTvd;
	return emptyTvd;
}


// ***************************************************************************
void CTileBank::loadTileVegetableDescs()
{
	// For all tileSets.
	uint tileSet;

	for(tileSet=0; tileSet<_TileSetVector.size(); tileSet++)
	{
		// load their fileName
		_TileSetVector[tileSet].loadTileVegetableDesc();
	}
}


// ***************************************************************************
void	CTileBank::initTileVegetableDescs(CVegetableManager *vegetableManager)
{
	// For all tileSets.
	uint tileSet;

	for(tileSet=0; tileSet<_TileSetVector.size(); tileSet++)
	{
		CTileVegetableDesc	&tvd= _TileSetVector[tileSet].getTileVegetableDesc();
		tvd.registerToManager(vegetableManager);
	}
}


// ***************************************************************************
void	CTileBank::postfixTileFilename (const char *postfix)
{
	// For each tiles
	uint tile;
	for (tile=0; tile<_TileVector.size (); tile++)
	{
		// For each bitmap
		uint bitmap;
		for (bitmap=0; bitmap<CTile::bitmapCount; bitmap++)
		{
			string &filename = _TileVector[tile]._BitmapName[bitmap];
			if (!filename.empty())
			{
				string ext = CFile::getExtension(filename);
				string name = CFile::getFilenameWithoutExtension(filename);
				filename = CFile::getPath (filename);
				filename += name;
				filename += postfix;
				filename += ".";
				filename += ext;
			}
		}
	}
}


// ***************************************************************************
void	CTileBank::postfixTileVegetableDesc (const char *postfix)
{
	// For each tiles
	uint tileSet;
	for (tileSet=0; tileSet<_TileSetVector.size (); tileSet++)
	{
		string &filename = _TileSetVector[tileSet]._TileVegetableDescFileName;
		if (!filename.empty())
		{
			string ext = CFile::getExtension(filename);
			string name = CFile::getFilenameWithoutExtension(filename);
			filename = CFile::getPath (filename);
			filename += name;
			filename += postfix;
			filename += ".";
			filename += ext;
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// CTile.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTile::_Version=4;
// ***************************************************************************
void CTile::serial(NLMISC::IStream &f)
{
	sint streamver = f.serialVersion(_Version);

	// Tmp value
	bool tmp;
	string tmpStr;

	switch (streamver)
	{
	case 4:
	case 3:
	case 2:
		f.serial (_Flags);

		// Version 2, flags are not the same
		if (streamver==2)
			_Flags=(_Flags&NL3D_CTILE_ROT_MASK)|(_Flags&NL3D_CTILE_GROUP_MASK_V2)|(((_Flags&NL3D_CTILE_FREE_FLAG_V2)!=0)?NL3D_CTILE_FREE_FLAG:0);
		if (streamver==3)
			_Flags=(_Flags&NL3D_CTILE_ROT_MASK)|(_Flags&NL3D_CTILE_GROUP_MASK_V3)|(((_Flags&NL3D_CTILE_FREE_FLAG_V3)!=0)?NL3D_CTILE_FREE_FLAG:0);

		f.serial (_BitmapName[diffuse]);
		f.serial (_BitmapName[additive]);
		f.serial (_BitmapName[alpha]);
		break;
	case 1:
		// Don't need invert many more
		f.serial (tmp);
	case 0:
		// Initialize flags
		_Flags=0;

		// Initialize alpha name
		_BitmapName[alpha].clear();

		// Read free flag
		f.serial (tmp);

		// If free, set the flag
		if (tmp)
			_Flags|=NL3D_CTILE_FREE_FLAG;

		// Read diffuse bitmap and additive
		f.serial (_BitmapName[diffuse]);
		f.serial (_BitmapName[additive]);

		// Don't need bump name
		f.serial (tmpStr);

		break;
	}
}
// ***************************************************************************
void CTile::clearTile (CTile::TBitmap type)
{
	_BitmapName[type].clear();
}


// ***************************************************************************
// ***************************************************************************
// CTileSet.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileSet::_Version=5;
// ***************************************************************************
const char* CTileSet::_ErrorMessage[CTileSet::errorCount]=
{
	"No error.",								// ok
	"Top interface is incompatible.",			// topInterfaceProblem
	"Bottom interface is incompatible.",		// bottomInterfaceProblem
	"Left interface is incompatible.",			// leftInterfaceProblem
	"Right interface is incompatible.",			// rightInterfaceProblem
	"Add first a 128x128 tile.",				// addFirstA128128
	"Top and bottom interface not the same.",	// topBottomNotTheSame,
	"Right and left interface not the same.",	// rightLeftNotTheSame
	"Invalide bitmap size.",					// sizeInvalide
};
// ***************************************************************************
const CTileSet::TFlagBorder CTileSet::_TransitionFlags[CTileSet::count][4]=
{
	{_0000,_1111,_0111,_0111},	// tile 0
	{_0111,_1111,_0111,_1111},	// tile 1
	{_0000,_0111,_0000,_0111},	// tile 2
	{_1110,_1110,_1111,_0000},	// tile 3
	{_1110,_1111,_1111,_0111},	// tile 4
	{_0000,_1110,_0111,_0000},	// tile 5

	{_0000,_1111,_0001,_0001},	// tile 6
	{_0000,_1000,_0001,_0000},	// tile 7
	{_1111,_1000,_1111,_1000},	// tile 8
	{_1000,_1000,_1111,_0000},	// tile 9
	{_1000,_0000,_1000,_0000},	// tile 10
	{_1111,_0001,_1000,_1111},	// tile 11

	{_0000,_1111,_0111,_0001},	// tile 12
	{_0000,_1111,_0001,_0111},	// tile 13
	{_0111,_1111,_0001,_1111},	// tile 14
	{_1110,_1000,_1111,_0000},	// tile 15
	{_1000,_1110,_1111,_0000},	// tile 16
	{_1111,_0001,_1110,_1111},	// tile 17

	{_1000,_0000,_1110,_0000},	// tile 18
	{_0000,_0111,_0000,_0001},	// tile 19
	{_1111,_1000,_1111,_1110},	// tile 21
	{_0111,_0000,_0000,_1000},	// tile 21
	{_0000,_1000,_0111,_0000},	// tile 22
	{_1111,_0111,_1000,_1111},	// tile 23

	{_1111,_0000,_1110,_1110},	// tile 24
	{_1111,_1110,_1111,_1110},	// tile 25
	{_1110,_0000,_1110,_0000},	// tile 26
	{_0111,_0111,_0000,_1111},	// tile 27
	{_1111,_0111,_1110,_1111},	// tile 28
	{_0111,_0000,_0000,_1110},	// tile 29

	{_1111,_0000,_1000,_1000},	// tile 30
	{_0001,_0000,_0000,_1000},	// tile 31
	{_0001,_1111,_0001,_1111},	// tile 32
	{_0001,_0001,_0000,_1111},	// tile 33
	{_0000,_0001,_0000,_0001},	// tile 34
	{_1000,_1111,_1111,_0001},	// tile 35

	{_1111,_0000,_1000,_1110},	// tile 36
	{_1111,_0000,_1110,_1000},	// tile 37
	{_1000,_1111,_1111,_0111},	// tile 38
	{_0001,_0111,_0000,_1111},	// tile 39
	{_0111,_0001,_0000,_1111},	// tile 40
	{_1111,_1110,_1111,_1000},	// tile 41

	{_0000,_0001,_0000,_0111},	// tile 42
	{_1110,_0000,_1000,_0000},	// tile 43
	{_0001,_1111,_0111,_1111},	// tile 44
	{_0000,_1110,_0001,_0000},	// tile 45
	{_0001,_0000,_0000,_1110},	// tile 46
	{_1110,_1111,_1111,_0001}	// tile 47
};
// ***************************************************************************
CTileSet::CTileSet ()
{
	// Default, tileset 0
	_Oriented = false;
	uint displace;
	for (displace=FirstDisplace; displace<CountDisplace; displace++)
		_DisplacementBitmap[displace]=0;

	// Default user surface data
	SurfaceData = 0;
}
// ***************************************************************************
void CTileSet::setName (const std::string& name)
{
	_Name=name;
}
// ***************************************************************************
const std::string& CTileSet::getName () const
{
	return _Name;
}
// ***************************************************************************
void CTileSet::serial(NLMISC::IStream &f)
{
	sint streamver = f.serialVersion(_Version);

	CTileBorder tmp;

	// serial the user surface data
	if (streamver>=5)
	{
		f.serial (SurfaceData);
	}

	// serial the oriented info which tell if the tile has a special orientation
	if (streamver>=4)
	{
		f.serial (_Oriented);
	}

	// serial vegetable info.
	if (streamver>=3)
	{
		// serialisze only the FileName, not the descrpitor
		f.serial(_TileVegetableDescFileName);
	}

	// New version
	if (streamver>=2)
	{
		uint displace;
		for (displace=FirstDisplace; displace<CountDisplace; displace++)
			f.serial (_DisplacementBitmap[displace]);
	}

	// Serial displacement map filename, obsolete
	if (streamver==1)
	{
		// Remove obsolete data
		string tmp;
		for (uint displace=FirstDisplace; displace<CountDisplace; displace++)
			f.serial (tmp);
	}

	int i;
	f.serial (_Name);
	f.serialCont (_Tile128);
	f.serialCont (_Tile256);
	for (i=0; i<count; i++)
		f.serial (_TileTransition[i]);
	f.serialCont (_ChildName);
	f.serial (_Border128[CTile::diffuse]);
	f.serial (_Border128[CTile::additive]);

	// old field, border bump 128
	if (streamver==0)
		f.serial (tmp);

	f.serial (_Border256[CTile::diffuse]);
	f.serial (_Border256[CTile::additive]);

	// old field, border bump 256
	if (streamver==0)
		f.serial (tmp);

	for (i=0; i<count; i++)
	{
		f.serial (_BorderTransition[i][CTile::diffuse]);
		f.serial (_BorderTransition[i][CTile::additive]);
		f.serial (_BorderTransition[i][CTile::alpha]);

		// Reset the diffuse and alpha border if old version
		if (streamver==0)
		{
			_BorderTransition[i][CTile::diffuse].reset();
			_BorderTransition[i][CTile::alpha].reset();
		}
	}
}
// ***************************************************************************
void CTileSet::addTile128 (int& indexInTileSet, CTileBank& bank)
{
	// Create a tile
	sint index=bank.createTile ();

	// Index of the new tile
	indexInTileSet=(int)_Tile128.size();

	// Add to the end of the list
	_Tile128.push_back (index);
}
// ***************************************************************************
void CTileSet::setBorder (CTile::TBitmap type, const CTileBorder& border)
{
	// This is our new border desc
	_Border128[type]=border;
	_Border256[type]=border;
	_Border256[type].doubleSize ();
}
// ***************************************************************************
void CTileSet::setTile128 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank)
{
	// Edit a tile
	CTile *tile=bank.getTile (_Tile128[indexInTileSet]);
	tile->setFileName (type, name);
	tile->setRotAlpha (0);
}
// ***************************************************************************
CTileSet::TError CTileSet::checkTile128 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante)
{
	// Self check
	if ((border.getWidth()!=128)||(border.getHeight()!=128))
		return sizeInvalide;
	if (!CTileBorder::compare (border, border, CTileBorder::top, CTileBorder::bottom, pixel, composante))
		return topBottomNotTheSame;
	if (!CTileBorder::compare (border, border, CTileBorder::left, CTileBorder::right, pixel, composante))
		return rightLeftNotTheSame;

	// Check
	if (_Border128[type].isSet())
	{
		// Other check
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
		if (!CTileBorder::compare (border, _Border128[type], CTileBorder::right, CTileBorder::right, pixel, composante))
			return rightInterfaceProblem;
	}
	else
	{
		return addFirstA128128;
	}

	return ok;
}
// ***************************************************************************
void CTileSet::addTile256 (int& indexInTileSet, CTileBank& bank)
{
	// Create a tile
	sint index=bank.createTile ();

	// Index of the new tile
	indexInTileSet=(int)_Tile256.size();

	// Add to the end of the list
	_Tile256.push_back (index);
}
// ***************************************************************************
CTileSet::TError CTileSet::checkTile256 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante)
{
	// Self check
	if ((border.getWidth()!=256)||(border.getHeight()!=256))
		return sizeInvalide;
	if (!CTileBorder::compare (border, border, CTileBorder::top, CTileBorder::bottom, pixel, composante))
		return topBottomNotTheSame;
	if (!CTileBorder::compare (border, border, CTileBorder::left, CTileBorder::right, pixel, composante))
		return rightLeftNotTheSame;

	// Check if prb
	if ((!_Border256[type].isSet())&&(_Border128[type].isSet()))
	{
		_Border256[type]=_Border128[type];
		_Border256[type].doubleSize ();
	}

	// Check
	if (_Border256[type].isSet())
	{

		// Other check
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
		if (!CTileBorder::compare (border, _Border256[type], CTileBorder::right, CTileBorder::right, pixel, composante))
			return rightInterfaceProblem;
	}
	else
	{
		return addFirstA128128;
	}

	return ok;
}
// ***************************************************************************
void CTileSet::setTile256 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank)
{
	// Edit a tile
	CTile *tile=bank.getTile (_Tile256[indexInTileSet]);
	tile->setFileName (type, name);
	tile->setRotAlpha (0);
}
// ***************************************************************************
void CTileSet::setTileTransition (TTransition transition, const std::string& name, CTile::TBitmap type,	CTileBank& bank,
											  const CTileBorder& border)
{
	// Check is not an alpha channel
	nlassert (type!=CTile::alpha);		// use setTileTransitionAlpha

	// Create a tile
	_BorderTransition[transition][type]=border;

	// Set the tile file name
	CTile *tile=bank.getTile (_TileTransition[transition]._Tile);
	tile->setFileName (type, name);
}
// ***************************************************************************
void CTileSet::setTileTransitionAlpha (TTransition transition, const std::string& name, CTileBank& bank,
									   const CTileBorder& border, uint8 rot)
{
	// Check some args
	nlassert (rot<4);

	// Create a tile
	_BorderTransition[transition][CTile::alpha]=border;

	// Set the tile file name
	CTile *tile=bank.getTile (_TileTransition[transition]._Tile);
	tile->setFileName (CTile::alpha, name);
	tile->setRotAlpha (rot);
}
// ***************************************************************************
CTileSet::TError CTileSet::checkTileTransition (TTransition transition, CTile::TBitmap type, const CTileBorder& border, int& indexError,
		int& pixel, int& composante)
{
	nlassert (transition>=0);
	nlassert (transition<count);

	// Check
	indexError=-1;

	// Top
	indexError=getExistingTransitionTile ((TFlagBorder)_TransitionFlags[transition][top], dontcare, dontcare, dontcare, transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::top, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
	}
	indexError=getExistingTransitionTile (dontcare, (TFlagBorder)_TransitionFlags[transition][top], dontcare, dontcare, transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::top, CTileBorder::bottom, pixel, composante))
			return topInterfaceProblem;
	}
	indexError=-1;
	if (_TransitionFlags[transition][top]==_1111)
	{
		if (!CTileBorder::allAlphaSet (border, CTileBorder::top, pixel, composante))
			return topInterfaceProblem;
	}

	// Bottom
	indexError=getExistingTransitionTile (dontcare, (TFlagBorder)_TransitionFlags[transition][bottom], dontcare, dontcare, transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::bottom, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
	}
	indexError=getExistingTransitionTile ((TFlagBorder)_TransitionFlags[transition][bottom], dontcare, dontcare, dontcare, transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::bottom, CTileBorder::top, pixel, composante))
			return bottomInterfaceProblem;
	}
	indexError=-1;
	if (_TransitionFlags[transition][bottom]==_1111)
	{
		if (!CTileBorder::allAlphaSet (border, CTileBorder::bottom, pixel, composante))
			return bottomInterfaceProblem;
	}

	// Left
	indexError=getExistingTransitionTile (dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][left], dontcare, transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::left, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
	}
	indexError=getExistingTransitionTile (dontcare, dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][left], transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::left, CTileBorder::right, pixel, composante))
			return leftInterfaceProblem;
	}
	indexError=-1;
	if (_TransitionFlags[transition][left]==_1111)
	{
		if (!CTileBorder::allAlphaSet (border, CTileBorder::left, pixel, composante))
			return leftInterfaceProblem;
	}

	// Right
	indexError=getExistingTransitionTile (dontcare, dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][right], transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::right, CTileBorder::right, pixel, composante))
			return rightInterfaceProblem;
	}
	indexError=getExistingTransitionTile (dontcare, dontcare, (TFlagBorder)_TransitionFlags[transition][right], dontcare, transition, type);
	if (indexError!=-1)
	{
		if (!CTileBorder::compare (border, _BorderTransition[indexError][type], CTileBorder::right, CTileBorder::left, pixel, composante))
			return rightInterfaceProblem;
	}
	indexError=-1;
	if (_TransitionFlags[transition][right]==_1111)
	{
		if (!CTileBorder::allAlphaSet (border, CTileBorder::right, pixel, composante))
			return rightInterfaceProblem;
	}
	return ok;
}
// ***************************************************************************
void CTileSet::removeTile128 (int indexInTileSet, CTileBank& bank)
{
	// Check args
	nlassert (indexInTileSet>=0);
	nlassert (indexInTileSet<(sint)_Tile128.size());

	// Old index
	int index=_Tile128[indexInTileSet];

	// Erase
	_Tile128.erase (_Tile128.begin()+indexInTileSet);
	bank.freeTile (index);

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, CTile::diffuse);
	deleteBordersIfLast (bank, CTile::additive);
	deleteBordersIfLast (bank, CTile::alpha);
}
// ***************************************************************************
void CTileSet::removeTile256 (int indexInTileSet, CTileBank& bank)
{
	// Check args
	nlassert (indexInTileSet>=0);
	nlassert (indexInTileSet<(sint)_Tile256.size());

	// Old index
	int index=_Tile256[indexInTileSet];

	// Erase
	_Tile256.erase (_Tile256.begin()+indexInTileSet);
	bank.freeTile (index);

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, CTile::diffuse);
	deleteBordersIfLast (bank, CTile::additive);
	deleteBordersIfLast (bank, CTile::alpha);
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, TFlagBorder _right)
{
	for (int n=first; n<count; n++)
	{
		if (((_top==dontcare)||(_top==(TFlagBorder)_TransitionFlags[n][top]))&&
			((_bottom==dontcare)||(_bottom==(TFlagBorder)_TransitionFlags[n][bottom]))&&
			((_left==dontcare)||(_left==(TFlagBorder)_TransitionFlags[n][left]))&&
			((_right==dontcare)||(_right==(TFlagBorder)_TransitionFlags[n][right])))
		{
			return (TTransition)n;
		}
	}
	return notfound;
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getExistingTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left, TFlagBorder _right, int reject, CTile::TBitmap type)
{
	for (int n=first; n<count; n++)
	{
		if ((n!=reject)&&
			(_BorderTransition[n][type].isSet ())&&
			((_top==dontcare)||(_top==(TFlagBorder)_TransitionFlags[n][top]))&&
			((_bottom==dontcare)||(_bottom==(TFlagBorder)_TransitionFlags[n][bottom]))&&
			((_left==dontcare)||(_left==(TFlagBorder)_TransitionFlags[n][left]))&&
			((_right==dontcare)||(_right==(TFlagBorder)_TransitionFlags[n][right])))
		{
			return (TTransition)n;
		}
	}
	return notfound;
}
// ***************************************************************************
void CTileSet::addChild (const std::string& name)
{
	_ChildName.insert (name);
}
// ***************************************************************************
void CTileSet::removeChild (const std::string& name)
{
	_ChildName.erase (name);
}
// ***************************************************************************
CTileSet::TTransition CTileSet::getComplementaryTransition (TTransition transition)
{
	nlassert ((transition>=first)&&(transition<=last));
	TTransition trans=getTransitionTile (getComplementaryBorder (_TransitionFlags[transition][top]),
		getComplementaryBorder (_TransitionFlags[transition][bottom]),
		getComplementaryBorder (_TransitionFlags[transition][left]),
		getComplementaryBorder (_TransitionFlags[transition][right]));

	nlassert (trans!=notfound);

	return trans;
}
// ***************************************************************************
CTileSet::TFlagBorder CTileSet::getComplementaryBorder (TFlagBorder border)
{
	switch (border)
	{
	case _0000:
		return _1111;
	case _0001:
		return _1110;
	case _0111:
		return _1000;
	case _1000:
		return _0111;
	case _1110:
		return _0001;
	case _1111:
		return _0000;
	default:
		nlassert (0);	// no
	}
	return _0000;
}
// ***************************************************************************
CTileSet::TFlagBorder CTileSet::getInvertBorder (TFlagBorder border)
{
	switch (border)
	{
	case _0000:
		return _0000;
	case _0001:
		return _1000;
	case _0111:
		return _1110;
	case _1000:
		return _0001;
	case _1110:
		return _0111;
	case _1111:
		return _1111;
	default:
		nlassert (0);	// no
	}
	return _0000;
}
// ***************************************************************************
CTileSet::TFlagBorder CTileSet::getOrientedBorder (TBorder where, TFlagBorder border)
{
	switch (where)
	{
	case left:
	case bottom:
		return border;
	case top:
	case right:
		return getInvertBorder (border);
	default:
		nlassert (0);	// no
	}
	return _0000;
}
// ***************************************************************************
CTileSet::TTransition CTileSet::rotateTransition (TTransition transition)
{
	return getTransitionTile (
		getOrientedBorder (top, getOrientedBorder (right, _TransitionFlags[transition][right])),	// top
		getOrientedBorder (bottom, getOrientedBorder (left, _TransitionFlags[transition][left])),	// bottom
		getOrientedBorder (left, getOrientedBorder (top, _TransitionFlags[transition][top])),		// left
		getOrientedBorder (right, getOrientedBorder (bottom, _TransitionFlags[transition][bottom])) // right
		);
}
// ***************************************************************************
void CTileSet::clearTile128 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_Tile128[indexInTileSet];
	bank.getTile (nTile)->clearTile(type);

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, type);
}
// ***************************************************************************
void CTileSet::clearTile256 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_Tile256[indexInTileSet];
	bank.getTile (nTile)->clearTile(type);

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, type);
}
// ***************************************************************************
void CTileSet::clearTransition (TTransition transition, CTile::TBitmap type, CTileBank& bank)
{
	int nTile=_TileTransition[transition]._Tile;
	if (nTile!=-1)
		bank.getTile (nTile)->clearTile(type);
	_BorderTransition[transition][type].reset();

	// Erase border if it is the last texture
	deleteBordersIfLast (bank, type);
}
// ***************************************************************************
// Delete 128 and 256 borders if no more valid texture file name for each bitmap type.
void CTileSet::deleteBordersIfLast (const CTileBank& bank, CTile::TBitmap type)
{
	// delete is true
	bool bDelete=true;

	// iterator..
	std::vector<sint32>::iterator ite=_Tile128.begin();

	// Check all the 128x128 tiles
	while (ite!=_Tile128.end())
	{
		// If the file name is valid
		if (!bank.getTile (*ite)->getRelativeFileName(type).empty())
		{
			// Don't delete,
			bDelete=false;
			break;
		}
		ite++;
	}
	// If break, not empty, return
	if (ite!=_Tile128.end())
		return;

	// Check all the 256x256 tiles
	ite=_Tile256.begin();
	while (ite!=_Tile256.end())
	{
		// If the file name is valid
		if (!bank.getTile (*ite)->getRelativeFileName(type).empty())
		{
			// Don't delete,
			bDelete=false;
			break;
		}
		ite++;
	}
	// If break, not empty, return
	if (ite!=_Tile256.end())
		return;


	// Check all the transitions tiles
	sint trans;
	for (trans=0; trans<count; trans++)
	{
		// Get the tile associed with the transition
		int nTile=_TileTransition[trans]._Tile;

		// If it is not NULL..
		if (nTile!=-1)
		{
			// If the file name is valid
			if (!bank.getTile (nTile)->getRelativeFileName(type).empty())
			{
				// Don't delete,
				bDelete=false;
				break;
			}
		}
	}
	if (trans!=count)
		return;

	// Ok, erase borders because no tile use it anymore
	_Border128[type].reset();
	_Border256[type].reset();
}
// ***************************************************************************
void CTileSet::clearDisplacement (TDisplacement displacement, CTileBank& bank)
{
	// checks
	nlassert (displacement>=FirstDisplace);
	nlassert (displacement<=LastDisplace);

	// Backup the id
	int id=_DisplacementBitmap[displacement];

	// Clear map id
	_DisplacementBitmap[displacement]=0;

	// Tell the bank we remove it
	bank.removeDisplacementMap (id);
}
// ***************************************************************************
void CTileSet::setDisplacement (TDisplacement displacement, const std::string& fileName, CTileBank& bank)
{
	// checks
	nlassert (displacement>=FirstDisplace);
	nlassert (displacement<=LastDisplace);

	// Clear it
	bank.removeDisplacementMap (_DisplacementBitmap[displacement]);

	// Get displacement map
	_DisplacementBitmap[displacement]=bank.getDisplacementMap (fileName);
}
// ***************************************************************************
void CTileSet::cleanUnusedData ()
{
	_Name.clear();
	_ChildName.clear();
	_Border128[0].reset ();
	_Border128[1].reset ();
	_Border256[0].reset ();
	_Border256[1].reset ();
	for (uint i=0; i<count; i++)
	for (uint j=0; j<CTile::bitmapCount; j++)
		_BorderTransition[i][j].reset();
}


// ***************************************************************************
void CTileSet::setTileVegetableDescFileName (const std::string &fileName)
{
	_TileVegetableDescFileName= fileName;
}
// ***************************************************************************
const std::string& CTileSet::getTileVegetableDescFileName () const
{
	return _TileVegetableDescFileName;
}
// ***************************************************************************
void CTileSet::setTileVegetableDesc (const CTileVegetableDesc	&tvd)
{
	_TileVegetableDesc= tvd;
}

// ***************************************************************************
CTileVegetableDesc			&CTileSet::getTileVegetableDesc()
{
	return _TileVegetableDesc;
}

// ***************************************************************************
const CTileVegetableDesc	&CTileSet::getTileVegetableDesc() const
{
	return _TileVegetableDesc;
}

// ***************************************************************************
void CTileSet::loadTileVegetableDesc()
{
	if(!_TileVegetableDescFileName.empty())
	{
		try
		{
			string	fname= CPath::lookup(_TileVegetableDescFileName);
			CIFile	f(fname);
			// load the TileVegetableDesc
			f.serial(_TileVegetableDesc);
		}
		catch(const Exception &e)
		{
			nlinfo("Error loading TileVegetableDesc: %s", e.what());
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// CTileBorder.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileBorder::_Version=0;
// ***************************************************************************
void CTileBorder::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(_Version);

	f.serial (_Set);
	f.serial (_Width);
	f.serial (_Height);
	f.serialCont (_Borders[top]);
	f.serialCont (_Borders[bottom]);
	f.serialCont (_Borders[left]);
	f.serialCont (_Borders[right]);
}
// ***************************************************************************
void CTileBorder::set (int width, int height, const std::vector<CBGRA>& array)
{
	// Check array size
	nlassert (width>0);
	nlassert (height>0);
	nlassert ((sint)array.size()==width*height);

	// Copy size
	_Width=width;
	_Height=height;

	// Last line
	int lastLine=(_Height-1)*width;
	int lastCol=(_Width-1);
	_Borders[top].resize (_Width);
	_Borders[bottom].resize (_Width);
	_Borders[left].resize (_Height);
	_Borders[right].resize (_Height);

	// Copy top/bottom border
	for (int w=0; w<_Width; w++)
	{
		_Borders[top][w]=array[w];
		_Borders[bottom][w]=array[w+lastLine];
	}

	// Copy left/right border
	for (int h=0; h<_Height; h++)
	{
		_Borders[left][h]=array[h*_Width];
		_Borders[right][h]=array[h*_Width+lastCol];
	}

	// Set
	_Set=true;
}
// ***************************************************************************
void CTileBorder::get (int &width, int &height, std::vector<CBGRA>& array) const
{
	// Go
	if (_Set)
	{
		width=_Width;
		height=_Height;
		array.resize (0);
		array.resize (_Width*_Height);
		nlassert (_Borders[bottom].size()==(uint)_Width);
		nlassert (_Borders[top].size()==(uint)_Width);
		nlassert (_Borders[left].size()==(uint)_Height);
		nlassert (_Borders[right].size()==(uint)_Height);

		// Fill
		CBGRA black(0,0,0);
		for (int p=0; p<_Width*_Height; p++)
		{
			array[p]=black;
		}

		// Last line
		int lastLine=(_Height-1)*_Width;
		int lastCol=(_Width-1);

		// Copy top/bottom border
		for (int w=0; w<_Width; w++)
		{
			array[w]=_Borders[top][w];
			array[w+lastLine]=_Borders[bottom][w];
		}

		// Copy left/right border
		for (int h=0; h<_Height; h++)
		{
			array[h*_Width]=_Borders[left][h];
			array[h*_Width+lastCol]=_Borders[right][h];
		}
	}
	else
	{
		width=0;
		height=0;
		array.resize (0);
	}
}
// ***************************************************************************
bool CTileBorder::compare (const CTileBorder& border1, const CTileBorder& border2, TBorder where1, TBorder where2, int& pixel, int& composante)
{
	// Check border is initialized
	nlassert (border1.isSet());
	nlassert (border2.isSet());

	if (border1._Borders[where1].size()!=border2._Borders[where2].size())
		return false;
	for (pixel=0; pixel<(int)border1._Borders[where1].size(); pixel++)
	{
		if (border1._Borders[where1][pixel].R!=border2._Borders[where2][pixel].R)
		{
			composante=0;
			return false;
		}
		else if (border1._Borders[where1][pixel].G!=border2._Borders[where2][pixel].G)
		{
			composante=1;
			return false;
		}
		else if (border1._Borders[where1][pixel].B!=border2._Borders[where2][pixel].B)
		{
			composante=2;
			return false;
		}
		else if (border1._Borders[where1][pixel].A!=border2._Borders[where2][pixel].A)
		{
			composante=3;
			return false;
		}
	}

	return true;
}
// ***************************************************************************
bool CTileBorder::allAlphaSet (const CTileBorder& border, TBorder where, int& pixel, int& composante)
{
	// Check border is initialized
	nlassert (border.isSet());

	// always Alpha
	composante=3;

	for (pixel=0; pixel<(int)border._Borders[where].size(); pixel++)
	{
		if (border._Borders[where][pixel].A!=0xff)
			return false;
	}

	return true;
}
// ***************************************************************************
bool CTileBorder::operator== (const CTileBorder& border) const
{
	return (_Width==border._Width) && (_Height==border._Height) && (_Borders==border._Borders);
}
// ***************************************************************************
void CTileBorder::operator= (const CTileBorder& border)
{
	_Set=border._Set;
	_Width=border._Width;
	_Height=border._Width;
	_Borders[top]=border._Borders[top];
	_Borders[bottom]=border._Borders[bottom];
	_Borders[left]=border._Borders[left];
	_Borders[right]=border._Borders[right];
}

// ***************************************************************************
void CTileBorder::doubleSize ()
{
	_Borders[top].resize (_Width*2);
	_Borders[bottom].resize (_Width*2);
	_Borders[left].resize (_Height*2);
	_Borders[right].resize (_Height*2);

	for (int w=0; w<_Width; w++)
	{
		_Borders[top][w+_Width]=_Borders[top][w];
		_Borders[bottom][w+_Width]=_Borders[bottom][w];
	}
	for (int h=0; h<_Height; h++)
	{
		_Borders[left][h+_Height]=_Borders[left][h];
		_Borders[right][h+_Height]=_Borders[right][h];
	}
}
// ***************************************************************************
void CTileBorder::rotate()
{
	// Copy the right
	std::vector<NLMISC::CBGRA> tmpLeft=_Borders[left];

	// Top inverted becomes left
	uint i, size;
	size=(uint)_Borders[top].size();
	_Borders[left].resize (size);

	// copy inverted
	for (i=0; i<size; i++)
		_Borders[left][i]=_Borders[top][size-i-1];

	// Right become top
	_Borders[top]=_Borders[right];

	// bottom inverted becomes right
	size=(uint)_Borders[bottom].size();
	_Borders[right].resize (size);

	// copy inverted
	for (i=0; i<size; i++)
		_Borders[right][i]=_Borders[bottom][size-i-1];

	// Left become bottom
	_Borders[bottom]=tmpLeft;

	// Invert size
	sint32 tmpSize=_Width;
	_Width=_Height;
	_Height=tmpSize;
}

// ***************************************************************************
// ***************************************************************************
// CTileSetTransition.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const sint CTileSetTransition::_Version=1;
// ***************************************************************************
void CTileSetTransition::serial(NLMISC::IStream &f)
{
	sint streamver = f.serialVersion(_Version);

	switch (streamver)
	{
	case 0:
		{
			bool doomy;
			f.serial (_Tile);
			f.serial (doomy);		// skip the old argu
		}
		break;
	case 1:
		f.serial (_Tile);
		break;
	}
}
// ***************************************************************************


// ***************************************************************************
// ***************************************************************************
// CTileNoise.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTileNoise::CTileNoise ()
{
	// Not loaded
	_TileNoiseMap=NULL;
}
// ***************************************************************************
CTileNoise::CTileNoise (const CTileNoise &src)
{
	// Default ctor
	_TileNoiseMap=NULL;

	// Copy
	*this=src;
}
// ***************************************************************************
CTileNoise::~CTileNoise ()
{
	if (_TileNoiseMap)
	{
		delete _TileNoiseMap;
		_TileNoiseMap=NULL;
	}
}
// ***************************************************************************
CTileNoise& CTileNoise::operator= (const CTileNoise &src)
{
	// Copy the filename
	_FileName=src._FileName;

	// Tile noise map ?
	if (src._TileNoiseMap)
	{
		if (_TileNoiseMap==NULL)
		{
			// Allocate it
			_TileNoiseMap=new CTileNoiseMap;
		}

		// Copy the noise map
		*_TileNoiseMap=*src._TileNoiseMap;
	}
	else
	{
		// Erase the map
		if (_TileNoiseMap)
		{
			delete _TileNoiseMap;
			_TileNoiseMap=NULL;
		}
	}
	return *this;
}
// ***************************************************************************
void CTileNoise::serial (NLMISC::IStream& f)
{
	// Version
	f.serialVersion (0);

	// Serial the file name
	f.serial (_FileName);
}
// ***************************************************************************
void CTileNoise::setEmpty ()
{
	// Reset it
	reset();
	_FileName="EmptyDisplacementMap";
	_TileNoiseMap=new CTileNoiseMap();
	memset (_TileNoiseMap->Pixels, 0, NL3D_TILE_NOISE_MAP_SIZE*NL3D_TILE_NOISE_MAP_SIZE);
}
// ***************************************************************************
void CTileNoise::reset()
{
	// Erase the map
	if (_TileNoiseMap)
	{
		delete _TileNoiseMap;
		_TileNoiseMap=NULL;
	}

	// Erase filename
	_FileName.clear();
}
// ***************************************************************************

}	// NL3D
