// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  by authors
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

#ifndef NL_DRIVER_OPENGL_VERTEX_PROGRAM_H
#define NL_DRIVER_OPENGL_VERTEX_PROGRAM_H

#include "nel/misc/types_nl.h"

namespace NL3D {

class CVertexProgram;

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

class CDriverGL3;

// The default vertex program
class CDriverGL3VertexProgramDefault
{
	struct CDesc
	{

	};

public:
	CDriverGL3VertexProgramDefault();
	~CDriverGL3VertexProgramDefault();

	void setDriver(CDriverGL3 *driver);

	CVertexProgram *generate();

private:
	CDriverGL3 *m_Driver;

};

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

#endif // NL_DRIVER_OPENGL_VERTEX_PROGRAM_H

/* end of file */
