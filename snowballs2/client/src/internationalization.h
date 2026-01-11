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
