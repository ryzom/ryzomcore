#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_tileset.h"

/*-------------------------------------------------------------------*/

// Set current sub selection of tileSet
void CTileSetSelection::setSelection (int selection, CTileBank& bank)
{
	// Reset the array
	_TileSetArray.resize (0);

	// Get the land selected
	CTileLand* pTileLand=NULL;
	if ((selection>=0)&&(selection<bank.getLandCount ()))
		pTileLand=bank.getLand (selection);

	// For each tileSet in the bank
	for (int t=0; t<bank.getTileSetCount (); t++)
	{
		// Get the tile set
		CTileSet *pTileSet=bank.getTileSet (t);
		nlassert (pTileSet);

		// Is this tileSet in the land ?
		if (pTileLand)
		{
			if (pTileLand->isTileSet (pTileSet->getName()))
				// Add this tileset
				_TileSetArray.push_back (t);
		}
		else
			// By default, insert it
			_TileSetArray.push_back (t);
	}
}

/*-------------------------------------------------------------------*/

// Get tileSet by id
int CTileSetSelection::getTileSet (int id)
{
	// If in the array ?
	if ((id>=0)&&(id<(sint)_TileSetArray.size()))
		return _TileSetArray[id];
	// No, return -1 as not found
	else
		return -1;
}

/*-------------------------------------------------------------------*/

// Get tileSet by id. Return -1 if the tileset asked for doesn't exist.
bool CTileSetSelection::isInArray (int id)
{
	// Number of tileset
	uint count=getTileCount ();

	// Search this in the array
	for (uint t=0; t<count; t++)
	{
		if (_TileSetArray[t]==id)
			return true;
	}
	return false;
}
