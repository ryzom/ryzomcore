/** \file pixel_program.h
 * Pixel program definition
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2000-2001  Nevrax Ltd.
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_PIXEL_PROGRAM_H
#define NL_PIXEL_PROGRAM_H

#include <nel/misc/types_nl.h>
#include <nel/misc/smart_ptr.h>
#include <nel/3d/program.h>

#include <list>

namespace NL3D {

class CPixelProgram : public IProgram
{
public:
	/// Constructor
	CPixelProgram();
	/// Destructor
	virtual ~CPixelProgram ();
};

} // NL3D


#endif // NL_PIXEL_PROGRAM_H

/* End of vertex_program.h */
