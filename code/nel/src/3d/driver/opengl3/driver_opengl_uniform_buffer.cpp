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

#include "driver_opengl.h"
#include "driver_opengl_uniform_buffer.h"

#include <nel/misc/debug.h>

namespace NL3D {
namespace NLDRIVERGL3 {

const char *GLSLHeaderUniformBuffer =
	"#define NL_BUILTIN_CAMERA_BIND " NL_MACRO_TO_STR(NL_BUILTIN_CAMERA_BIND) "\n"
	"#define NL_BUILTIN_MODEL_BIND " NL_MACRO_TO_STR(NL_BUILTIN_MODEL_BIND) "\n"
	"#define NL_BUILTIN_MATERIAL_BIND " NL_MACRO_TO_STR(NL_BUILTIN_MATERIAL_BIND) "\n"
	"#define NL_USER_ENV_BIND " NL_MACRO_TO_STR(NL_USER_ENV_BIND) "\n"
	"#define NL_USER_VERTEX_PROGRAM_BIND " NL_MACRO_TO_STR(NL_USER_VERTEX_PROGRAM_BIND) "\n"
	"#define NL_USER_GEOMETRY_PROGRAM_BIND " NL_MACRO_TO_STR(NL_USER_GEOMETRY_PROGRAM_BIND) "\n"
	"#define NL_USER_PIXEL_PROGRAM_BIND " NL_MACRO_TO_STR(NL_USER_PIXEL_PROGRAM_BIND) "\n"
	"#define NL_USER_MATERIAL_BIND " NL_MACRO_TO_STR(NL_USER_MATERIAL_BIND) "\n";

} // NLDRIVERGL3
} // NL3D

/* end of file */
