/**
 * \file internationalization.h
 * \brief CInternationalization
 * \date 2008-11-26 14:48GMT
 * \author Jan Boon (Kaetemi)
 * CInternationalization
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

#ifndef SBCLIENT_INTERNATIONALIZATION_H
#define SBCLIENT_INTERNATIONALIZATION_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace SBCLIENT {

/**
 * \brief CInternationalization
 * \date 2008-11-07 16:59GMT
 * \author Jan Boon (Kaetemi)
 * CInternationalization
 */
class CInternationalization
{
public:
	static void init();
	static void release();

	static void enableCallback(void (*cb)());
	static void disableCallback(void (*cb)());

}; /* class CInternationalization */

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_INTERNATIONALIZATION_H */

/* end of file */
