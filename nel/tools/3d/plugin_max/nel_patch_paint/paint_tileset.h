#include "stdafx.h"

using namespace NL3D;
using namespace NLMISC;

class CTileSetSelection
{
public:
	// Set current sub selection of tileSet
	void	setSelection (int selection, CTileBank& bank);

	// Get tileSet by id. Return -1 if the tileset asked for doesn't exist.
	int		getTileSet (int id);

	// Get tileSet by id. Return -1 if the tileset asked for doesn't exist.
	bool	isInArray (int id);

	// Get tileSet count in this selection
	uint	getTileCount ()
	{
		return _TileSetArray.size();
	}

private:
	std::vector<int>	_TileSetArray;
};
