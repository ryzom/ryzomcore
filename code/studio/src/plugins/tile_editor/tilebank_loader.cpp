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


#include "tilebank_loader.h"

#include "tile_model.h"
#include "nel/misc/file.h"

TileBankLoader::TileBankLoader()
{
}

TileBankLoader::~TileBankLoader()
{
}

bool TileBankLoader::load( const char *filename, TileModel *model )
{
	NLMISC::CIFile file;
	if( !file.open( filename, false ) )
		return false;

	model->serial( file );

	file.close();

	return true;
}
