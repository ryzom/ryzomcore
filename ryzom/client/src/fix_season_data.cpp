// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#include "stdpch.h"

#include "nel/3d/u_instance.h"

#include "fix_season_data.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/season_file_ext.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************

void SeasonPostfixTextureFilename (string &filename, EGSPD::CSeason::TSeason season)
{
	string ext = CFile::getExtension(filename);
	string name = CFile::getFilenameWithoutExtension(filename);
	filename = CFile::getPath (filename);
	filename += name;
	filename += CSeasonFileExt::getExtension (season);
	filename += ".";
	filename += ext;
}

// ***************************************************************************

