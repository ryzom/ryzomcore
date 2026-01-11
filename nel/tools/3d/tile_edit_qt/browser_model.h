#ifndef BROWSERMODEL_H
#define BROWSERMODEL_H

#include <vector>
#include <string>
#include <nel/3d/tile_bank.h>


//TODO titegus: Equals TTileType?
enum TileType
{ 
	_128x128 = 0,
	_256x256 = 1,
	Transition = 2,
	Displace = 3,
	UnSet = 4
};


//TODO titegus: Equals TBitmap ?
enum TileTexture
{ 
	Diffuse = 0,
	Additive = 1,
	Alpha = 2
};


class TileInfo
{
public:
	TileInfo();
	TileInfo(int id, TileType tileType);

	void Init(int id, TileType tileType);
	bool Load (int index, std::vector<NLMISC::CBGRA>* Alpha);
	void Delete ();
	const std::string getRelativeFileName (TileTexture type, int index);

	//data
	std::vector<NLMISC::CBGRA> Bits;
	std::vector<NLMISC::CBGRA> alphaBits;
	std::vector<NLMISC::CBGRA> nightBits;

	int loaded, nightLoaded, alphaLoaded;	//tells if the tile was already loaded or not
	std::string path, nightPath, alphaPath;

	int getId() const	{	return id;	}
	void setId(int i) 	{	id = i;	}
	int getTileType() const	{	return tileType;	}

	static std::string fixPath(const std::string &path);

private:
	int id;									//tile index (in the Browser)
	TileType tileType;
};

typedef std::vector<TileInfo> tilelist;

class TileList
{
public:	
	TileList();

	int addTile128 ();
	int addTile256 ();

	void removeTile128 (int index);
	void removeTile256 (int index);

	bool setTile128 (int tile, const std::string& name, NL3D::CTile::TBitmap type);
	bool setTile256 (int tile, const std::string& name, NL3D::CTile::TBitmap type);
	bool setTileTransition (int tile, const std::string& name, NL3D::CTile::TBitmap type);
	bool setTileTransitionAlpha (int tile, const std::string& name, int rot);
	bool setDisplacement (int tile, const std::string& name, NL3D::CTile::TBitmap type);

	void clearTile128 (int index, NL3D::CTile::TBitmap bitmap);
	void clearTile256 (int index, NL3D::CTile::TBitmap bitmap);
	void clearTransition (int index, NL3D::CTile::TBitmap bitmap);
	void clearDisplacement (int index, NL3D::CTile::TBitmap bitmap);
	
	void Reload(int first, int last, TileType n);
	
	int  GetSize(int n);
	tilelist::iterator GetFirst(int n);
	tilelist::iterator GetLast(int n);
	tilelist::iterator Get(int i, int n);

public:
	tilelist theList[4];
#define theList128 theList[0]
#define theList256 theList[1]
#define theListTransition theList[2]
#define theListDisplacement theList[3]
	int _tileSet;
};

#endif
