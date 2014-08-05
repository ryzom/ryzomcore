// Ryzom Core Studio - Tile Editor plugin
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
