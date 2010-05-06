/**
 * \file game_time.h
 * \brief CGameTime
 * \date 2008-11-26 14:44GMT
 * \author Jan Boon (Kaetemi)
 * CGameTime
 */

/* 
 * Copyright (C) 2008  by authors
 * 
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

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

	/// Used when loading, this will skip changing animation time on the next update
	/// (updates aren't called during loading)
	static void skipAnimationOnce();

}; /* class CGameTime */

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_GAME_TIME_H */

/* end of file */
