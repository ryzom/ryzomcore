#include "stdafx.h"

using namespace NL3D;
using namespace NLMISC;

#include "nel/3d/tile_element.h"
#include "nel/3d/tile_color.h"

typedef std::pair<int, int> CNelPatchKey;

// Value of the patch map item.
class CNelPatchValue
{
public:
	friend class CNelPatchChanger;
	CNelPatchValue ();
	~CNelPatchValue ();
private:
	bool											TileLoaded;
	bool											ColorLoaded;
	std::vector<CTileElement>						*Tiles;
	std::vector<CTileColor>							*TileColors;
};

typedef std::map<CNelPatchKey, CNelPatchValue> CNelPatchMap;

// Nel patch change manager
class CNelPatchChanger
{
public:
	// Construct the object
	CNelPatchChanger (CLandscape *landscape)
	{
		_Landscape=landscape;
	}

	// Clear
	void clear ();

	// Get a patch tile array
	std::vector<CTileElement>* getTileArray (int mesh, int patch);

	// Get a patch tile array
	std::vector<CTileColor>* getColorArray (int mesh, int patch);

	// Apply changes
	void applyChanges (bool displace);

private:

	CLandscape							*_Landscape;
	CNelPatchMap						_MapNeLPatchInfo;
};
