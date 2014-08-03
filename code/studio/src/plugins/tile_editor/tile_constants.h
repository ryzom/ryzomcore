#ifndef TILE_CONSTANTS_H
#define TILE_CONSTANTS_H


namespace TileConstants
{
	enum TTileChannel
	{
		TileDiffuse = 0,
		TileAdditive = 1,
		TileAlpha = 2,
		TileChannelCount = 3
	};

	enum TNodeTileType
	{
		Tile128 = 0,
		Tile256 = 1,
		TileTransition = 2,
		TileDisplacement = 3,
		TileNodeTypeCount = 4
	};
}


#endif
