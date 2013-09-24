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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/vertex_program.h"
#include "nel/3d/vertex_program_parse.h"
#include <algorithm>

// tmp
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

//#define DEBUG_SETUP_EXT_VERTEX_SHADER

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

void CDriverGL3::enableVertexProgramDoubleSidedColor(bool doubleSided)
{
	H_AUTO_OGL(CDriverGL3_enableVertexProgramDoubleSidedColor);

	// change mode (not cached because supposed to be rare)
	if(doubleSided)
		glEnable (GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
	else
		glDisable (GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
}


#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
