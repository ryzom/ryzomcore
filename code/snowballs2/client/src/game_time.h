// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef SBCLIENT_GAME_TIME_H
#define SBCLIENT_GAME_TIME_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace SBCLIENT {

/**
 * \brief CGameTime
 * \date 2008-11-26 14:44GMT
 * \author Jan Boon (Kaetemi)
 * CGameTime
 */
class CGameTime
{
public:
	static void init();
	static void release();

	static void updateTime();

	/// Advance time to target time by factor f.
	static void advanceTime(double f);

	/// Used when loading, this will skip changing animation time on the next update
	/// (updates aren't called during loading)
	static void skipAnimationOnce();

}; /* class CGameTime */

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_GAME_TIME_H */

/* end of file */
