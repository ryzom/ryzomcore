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

#ifndef NL_Tile_BANK_H
#define NL_Tile_BANK_H

#include "nel/misc/debug.h"
#include "nel/misc/stream.h"
#include "nel/misc/rgba.h"
#include <vector>
#include <set>
#include <string>
#include "nel/3d/tile_vegetable_desc.h"


namespace	NLMISC
{
	class	IStream;
}


namespace	NL3D
{

class CTileBank;
class CVegetableManager;


/**
 * Tiles
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTile
{
	// Mask for flags
#define NL3D_CTILE_ROT_MASK				0x0000000f
#define NL3D_CTILE_ROT_RSHIFT			0x0
#define NL3D_CTILE_GROUP_MASK			0x0000fff0
#define NL3D_CTILE_GROUP_MASK_V2		0x000000f0
#define NL3D_CTILE_GROUP_MASK_V3		0x00000ff0
#define NL3D_CTILE_GROUP_RSHIFT			0x4
#define NL3D_CTILE_FREE_FLAG			0x80000000
#define NL3D_CTILE_FREE_FLAG_V2			0x00000100
#define NL3D_CTILE_FREE_FLAG_V3			0x00001000
#define NL3D_CTILE_NUM_GROUP			12

public:
	friend class CTileSet;
	friend class CTileBank;
	enum TBitmap { diffuse=0, additive, alpha, bitmapCount };
public:
	CTile ()
	{
		_Flags=NL3D_CTILE_FREE_FLAG;
	}
	const std::string& getRelativeFileName (TBitmap bitmapType) const
	{
		return _BitmapName[bitmapType];
	}
	bool isFree () const
	{
		return (_Flags&NL3D_CTILE_FREE_FLAG)!=0;
	}
	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	void setFileName (TBitmap bitmapType, const std::string& name)
	{
		// not free
		_Flags&=~NL3D_CTILE_FREE_FLAG;

		// set filename
		_BitmapName[bitmapType]=name;
	}

	std::string getFileName (TBitmap bitmapType) const
	{
		return _BitmapName[bitmapType];
	}

	/// Get the additional orientation (CCW) for alpha texture.
	uint8	getRotAlpha ()
	{
		return (uint8)((_Flags&NL3D_CTILE_ROT_MASK)>>NL3D_CTILE_ROT_RSHIFT);
	}

	/// Set the additional orientation (CCW) for alpha texture.
	void	setRotAlpha (uint8 rot)
	{
		// Checks
		nlassert (rot<4);

		// Clear flags
		_Flags&=~NL3D_CTILE_ROT_MASK;

		// Set flags
		_Flags|=(((uint32)rot)<<NL3D_CTILE_ROT_RSHIFT);
	}

	/**
	  * Get the group flags for this tile.
	  *
	  * If the tile is in the I-ne gourp, the flag 1<<I is set. There are 12 groups.
	  */
	uint	getGroupFlags () const
	{
		return ((_Flags&NL3D_CTILE_GROUP_MASK)>>NL3D_CTILE_GROUP_RSHIFT);
	}

	/**
	  * Set the group flags for this tile.
	  *
	  * If the tile is in the I-ne gourp, the flag 1<<I is set. There are 12 groups.
	  */
	void	setGroupFlags (uint group)
	{
		// Checks
		nlassert (group<NL3D_CTILE_NUM_GROUP);

		// Clear flags
		_Flags&=~NL3D_CTILE_GROUP_MASK;

		// Set flags
		_Flags|=(((uint32)group)<<NL3D_CTILE_GROUP_RSHIFT);
	}

private:
	void	clearTile (CTile::TBitmap type);
	void	free ()
	{
		nlassert ((_Flags&=NL3D_CTILE_FREE_FLAG)==0);
		_Flags|=NL3D_CTILE_FREE_FLAG;
	}

	// Internal members
	uint32						_Flags;
	std::string					_BitmapName[bitmapCount];
	static const sint			_Version;
};

/**
 * Set of tiles for a land
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileLand
{
private:
	// Class CTileLand;
public:
	const std::string& getName () const
	{
		return _Name;
	};

	std::set<std::string> getTileSets() const{ return _TileSet; }

	void setName (const std::string& name);
	void addTileSet (const std::string& name);
	void removeTileSet (const std::string& name);
	bool isTileSet (const std::string& name)
	{
		return _TileSet.find (name)!=_TileSet.end();
	}

	void clear(){ _TileSet.clear(); }

	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
private:

	// internal use
	static void intersect (const std::set<sint32>& setSrc1, const std::set<sint32>& setSrc2, std::set<sint32>& setDst);

	std::string	_Name;
	std::set<std::string>	_TileSet;
	static const sint _Version;
};

/**
 * This class manage a transition tile.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileSetTransition
{
	friend class CTileSet;
	friend class CTileBank;
public:
	CTileSetTransition ()
	{
		_Tile=-1;
	}
	sint32 getTile () const
	{
		return _Tile;
	}
	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

private:
	sint32	_Tile;
	static const sint _Version;
};

/**
 * This class is a tile set. It handles all the tile of the same material.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBorder
{
public:
	enum TBorder { top=0, bottom, left, right, borderCount };
	CTileBorder ()
	{
		reset();
	}
	void set (int width, int height, const std::vector<NLMISC::CBGRA>& array);
	void get (int &width, int &height, std::vector<NLMISC::CBGRA>& array) const;
	void doubleSize ();
	bool operator== (const CTileBorder& border) const;
	void operator= (const CTileBorder& border);
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	bool isSet() const
	{
		return _Set;
	}
	void reset()
	{
		_Set=false;
		_Borders[top].clear();
		_Borders[bottom].clear();
		_Borders[left].clear();
		_Borders[right].clear();
	}
	sint32 getWidth() const
	{
		return _Width;
	}
	sint32 getHeight() const
	{
		return _Height;
	}
	void	rotate();

	static bool allAlphaSet (const CTileBorder& border, TBorder where, int& pixel, int& composante);
	static bool compare (const CTileBorder& border1, const CTileBorder& border2, TBorder where1, TBorder where2, int& pixel, int& composante);

private:
	bool _Set;
	sint32 _Width;
	sint32 _Height;
	std::vector<NLMISC::CBGRA> _Borders[borderCount];
	static const sint _Version;
};

/**
 * This class is a tile set. It handles all the tile of the same material.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileSet
{
	friend class CTileBank;
public:

	enum TError { ok=0, topInterfaceProblem, bottomInterfaceProblem, leftInterfaceProblem,
		rightInterfaceProblem, addFirstA128128, topBottomNotTheSame, rightLeftNotTheSame,
		sizeInvalide, errorCount };
	enum TTransition { first=0, last=47, count=48, notfound=-1 };
	enum TDisplacement { FirstDisplace=0, LastDisplace=15, CountDisplace=16 };
	enum TBorder { top=0, bottom, left, right, borderCount };
	enum TFlagBorder { _1111=0,	_0111, _1110, _0001, _1000, _0000, dontcare=-1 };

	// Ctor
	CTileSet ();

	// add
	void addTile128 (int& indexInTileSet, CTileBank& bank);
	void addTile256 (int& indexInTileSet, CTileBank& bank);

	// remove
	void removeTile128 (int indexInTileSet, CTileBank& bank);
	void removeTile256 (int indexInTileSet, CTileBank& bank);

	// clear
	void clearTile128 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank);
	void clearTile256 (int indexInTileSet, CTile::TBitmap type, CTileBank& bank);
	void clearTransition (TTransition transition, CTile::TBitmap type, CTileBank& bank);
	void clearDisplacement (TDisplacement displacement, CTileBank& bank);

	// set
	void setName (const std::string& name);
	void setOriented (bool oriented) { _Oriented = oriented; }
	void setTile128 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank);
	void setTile256 (int indexInTileSet, const std::string& name, CTile::TBitmap type, CTileBank& bank);
	void setTileTransition (TTransition transition, const std::string& name, CTile::TBitmap type, CTileBank& bank, const CTileBorder& border);
	void setTileTransitionAlpha (TTransition transition, const std::string& name, CTileBank& bank, const CTileBorder& border, uint8 rotAlpha);
	void setBorder (CTile::TBitmap type, const CTileBorder& border);
	void setDisplacement (TDisplacement displacement, const std::string& fileName, CTileBank& bank);
	// Set the fileName for TileVegetableDesc.
	void setTileVegetableDescFileName (const std::string &fileName);
	// For edition: change the tileVegetableDesc. NB: only the TileVegetableDescFileName is serialised.
	void setTileVegetableDesc (const CTileVegetableDesc	&tvd);
	/** try to load the vegetable tile desc associated with the fileName (nlinfo() if can't)
	 *	lookup into CPath. no-op if string=="".
	 */
	void loadTileVegetableDesc();

	// check
	TError checkTile128 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante);
	TError checkTile256 (CTile::TBitmap type, const CTileBorder& border, int& pixel, int& composante);
	TError checkTileTransition (TTransition transition, CTile::TBitmap type, const CTileBorder& border, int& indexError,
		int& pixel, int& composante);

	// get
	const std::string& getName () const;
	bool getOriented () const
	{
		return _Oriented;
	}
	sint getNumTile128 () const
	{
		return (sint)_Tile128.size();
	}
	sint getNumTile256 () const
	{
		return (sint)_Tile256.size();
	}
	sint32 getTile128 (sint index) const
	{
		return _Tile128[index];
	}
	sint32 getTile256 (sint index) const
	{
		return _Tile256[index];
	}
	CTileSetTransition* getTransition (sint index)
	{
		return _TileTransition+index;
	}
	const CTileSetTransition* getTransition (sint index) const
	{
		return _TileTransition+index;
	}
	const CTileBorder *getBorder128 (CTile::TBitmap bitmapType) const
	{
		return &(_Border128[bitmapType]);
	}
	const CTileBorder *getBorder256 (CTile::TBitmap bitmapType) const
	{
		return &(_Border256[bitmapType]);
	}
	const std::string& getTileVegetableDescFileName () const;

	/**
	  * Return the file name of the displacement map for the map nb displacement.
	  * This file name is relative at the absolute path.
	  */
	uint getDisplacementTile (TDisplacement displacement) const
	{
		// checks
		nlassert (displacement>=FirstDisplace);
		nlassert (displacement<=LastDisplace);

		// return file name
		return _DisplacementBitmap[displacement];
	}


	/// return the TileVegetable
	CTileVegetableDesc			&getTileVegetableDesc();
	const CTileVegetableDesc	&getTileVegetableDesc() const;

	// Static methods
	static const char* getErrorMessage (TError error)
	{
		return _ErrorMessage[error];
	}
	static TTransition getTransitionTile (TFlagBorder top, TFlagBorder bottom, TFlagBorder left, TFlagBorder right);
	TTransition getExistingTransitionTile (TFlagBorder _top, TFlagBorder _bottom, TFlagBorder _left,
		TFlagBorder _right, int reject, CTile::TBitmap type);
	static TTransition getComplementaryTransition (TTransition transition);
	static TFlagBorder getInvertBorder (TFlagBorder border);
	static TFlagBorder getOrientedBorder (TBorder where, TFlagBorder border);
	static TFlagBorder getEdgeType (TTransition _what, TBorder _where)
	{
		return _TransitionFlags[_what][_where];
	}
	static TTransition rotateTransition (TTransition transition);

	// other

	void cleanUnusedData ();

	void addChild (const std::string& name);
	void removeChild (const std::string& name);
	bool isChild (const std::string& name)
	{
		return _ChildName.find(name)!=_ChildName.end();
	}
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	static TFlagBorder getComplementaryBorder (TFlagBorder border);

	// Delete 128 and 256 borders if no more valid texture file name for each bitmap type.
	void			deleteBordersIfLast (const CTileBank& bank, CTile::TBitmap type);

private:
	std::string	_Name;
	bool _Oriented;
	std::vector<sint32>	_Tile128;
	std::vector<sint32>	_Tile256;
	CTileSetTransition _TileTransition[count];
	std::set<std::string> _ChildName;
	CTileBorder _Border128[2];
	CTileBorder _Border256[2];
	CTileBorder _BorderTransition[count][CTile::bitmapCount];
	uint32 _DisplacementBitmap[CTileSet::CountDisplace];
	// the info for TileVegetable
	CTileVegetableDesc		_TileVegetableDesc;
	std::string				_TileVegetableDescFileName;

public:

	// User surface data
	uint32		SurfaceData;

private:
	static const sint _Version;
	static const char* _ErrorMessage[CTileSet::errorCount];
	static const TFlagBorder _TransitionFlags[count][4];
};


/**
 * This class manage tile noise.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileNoise
{
	friend class CTileBank;
public:
	// Ctor
	CTileNoise ();

	// Copy ctor
	CTileNoise (const CTileNoise &src);

	// Dtor
	~CTileNoise ();

	// Copy operator
	CTileNoise& operator= (const CTileNoise &src);

	// Serial
	void serial (NLMISC::IStream& f);

	// Make a blank tile noise
	void setEmpty ();

	// Empty the tile noise
	void reset();
private:
	class CTileNoiseMap		*_TileNoiseMap;
	std::string				_FileName;
};


/**
 * This class manage tile texture. It can load banktile description file
 * (*.bank), and then gives access to land infos.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileBank
{
	friend class CTileSet;
public:
	enum TTileType { _128x128=0, _256x256, transition, undefined };

	// Ctor
	CTileBank ();

	// Get
	sint getLandCount () const
	{
		return (sint)_LandVector.size();
	}
	const CTileLand* getLand (int landIndex) const
	{
		return &_LandVector[landIndex];
	}
	CTileLand* getLand (int landIndex)
	{
		return &_LandVector[landIndex];
	}
	sint getTileSetCount () const
	{
		return (sint)_TileSetVector.size();
	}
	const CTileSet* getTileSet (int tileIndex) const
	{
		return &_TileSetVector[tileIndex];
	}
	CTileSet* getTileSet (int tileIndex)
	{
		return &_TileSetVector[tileIndex];
	}
	sint getTileCount () const
	{
		return (sint)_TileVector.size();
	}
	const CTile* getTile (int tileIndex) const
	{
		return &_TileVector[tileIndex];
	}
	CTile* getTile (int tileIndex)
	{
		return &_TileVector[tileIndex];
	}
	sint addLand (const std::string& name);
	void removeLand (sint landIndex);
	sint addTileSet (const std::string& name);
	void removeTileSet (sint landIndex);
	void xchgTileset (sint first, sint second);
	void clear ();
	sint getNumBitmap (CTile::TBitmap bitmap) const;
	void computeXRef ();

	// Remove data unused in realtime
	void cleanUnusedData ();

	/**
	  * Return the xref for a tile.
	  *
	  * \param tile is the tile number.
	  * \param tileSet will receive the tile set number in which the tile is. -1 if the tile is not used.
	  * \param number will receive the number of the tile in the tileset.
	  * \param type is the type of tile.
	  */
	void getTileXRef (int tile, int &tileSet, int &number, TTileType& type) const
	{
		nlassert (tile>=0);
		nlassert (tile<(sint)_TileXRef.size());
		tileSet=_TileXRef[tile]._XRefTileSet;
		number=_TileXRef[tile]._XRefTileNumber;
		type=_TileXRef[tile]._XRefTileType;
	}

	// Get number of displacement map
	uint getDisplacementMapCount () const;

	// Get filename
	const char* getDisplacementMap (uint noiseMap);

	// Get filename
	void setDisplacementMap (uint noiseMap, const char *newName);

	// Add a displacement map
	uint getDisplacementMap (const std::string &fileName);

	// Remove a displacement map
	void removeDisplacementMap (uint mapId);

	/// \name Vegetable
	// @{

	/**
	  * Return the tilenoisemap pointer for this tile and subnoise tile
	  */
	CTileNoiseMap *getTileNoiseMap (uint tileNumber, uint tileSubNoise);

	/**
	  * return the TileVegetable desc for this tile
	  */
	const CTileVegetableDesc	&getTileVegetableDesc(uint tileNumber) const;

	/**
	 *	you should call this method, after serialising the TileBank, and before CLandscape::initTileBanks()
	 *	You must call CLandscape::initTileBanks() after calling this method
	 *	for each tileSet call CTileSet::loadTileVegetableDesc()
	 */
	void loadTileVegetableDescs();

	/**
	 *	register all CVegetable to the Manager. called by CLandscape::initTileBanks()
	 */
	void initTileVegetableDescs(CVegetableManager *vegetableManager);

	// @}


	void makeAllPathRelative ();
	/// This method change ".tga" of texture filename, to ".dds". Do this only for Additive and Diffuse part (not alpha).
	void makeAllExtensionDDS ();
	void setAbsPath (const std::string& newPath)
	{
		_AbsPath=newPath;
	}
	const std::string& getAbsPath () const
	{
		return _AbsPath;
	}

	/// Postfix tile filename
	void	postfixTileFilename (const char *filename);

	/// Postfix tile vegetable desc
	void	postfixTileVegetableDesc (const char *filename);

	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
private:
	sint	createTile ();
	void	freeTile (int tileIndex);
private:
	struct CTileXRef
	{
		CTileXRef ()
		{
			_XRefTileType=undefined;
		}
		CTileXRef (int tileSet, int number, TTileType type)
		{
			_XRefTileSet=tileSet;
			_XRefTileNumber=number;
			_XRefTileType=type;
		}
		int	_XRefTileSet;
		int	_XRefTileNumber;
		TTileType	_XRefTileType;
	};
	std::vector<CTileLand>	_LandVector;
	std::vector<CTileSet>	_TileSetVector;
	std::vector<CTile>		_TileVector;
	std::vector<CTileXRef>	_TileXRef;
	std::vector<CTileNoise>	_DisplacementMap;
	std::string			_AbsPath;
	static const sint	_Version;
};


}

#endif // NL_Tile_BANK_H

/* End of tile_bank.h */
