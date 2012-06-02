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


#ifndef RY_SEASON_FILE_EXT_H
#define RY_SEASON_FILE_EXT_H

#include "nel/misc/types_nl.h"
#include "season.h"

/**
 * helper to get season file extension ( we have to do it there as season.h is a generated file )
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CSeasonFileExt
{
public:
	static const char *getExtension(uint16 s)
	{
		static const char *seasonEXT[] =
		{
			"_Sp",
			"_Su",
			"_Au",
			"_Wi",
			"Unknown"
		};
		if (s >= EGSPD::CSeason::Invalid) return seasonEXT[EGSPD::CSeason::Invalid];
		return seasonEXT[s];
	}


};


#endif // RY_SEASON_FILE_EXT_H

/* End of season_file_ext.h */
