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


#include "tilebank_saver.h"
#include "tile_model.h"
#include "tile_item.h"

#include "nel/3d/tile_bank.h"
#include "nel/misc/file.h"

class TileBankSaverPvt
{
public:

};

TileBankSaver::TileBankSaver()
{
	p = new TileBankSaverPvt();
}

TileBankSaver::~TileBankSaver()
{
	delete p;
	p = NULL;
}

bool TileBankSaver::save( const char *fileName, const TileModel* model, const QList< Land > &lands )
{
	// Save to file
	NLMISC::COFile f;
	bool b = f.open( fileName, false, false, false );
	if( !b )
		return false;

	//p->bank.serial( f );

	f.flush();
	f.close();

	return true;
}

