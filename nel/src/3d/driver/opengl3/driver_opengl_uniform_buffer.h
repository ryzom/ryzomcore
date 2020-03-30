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

#ifndef NL_DRIVER_OPENGL_UNIFORM_BUFFER_H
#define NL_DRIVER_OPENGL_UNIFORM_BUFFER_H

#include <nel/misc/types_nl.h>

#include <nel/3d/uniform_buffer_format.h>

#define NL3D_GL3_UNIFORM_BUFFER_DEBUG 1

namespace NL3D {
namespace NLDRIVERGL3 {

// NOTE: It is completely safe to reorder these indices.
// When changing, update:
//     - GLSLHeaderUniformBuffer
//     - s_UniformBufferBindDefine
//     - s_UniformBufferBindName
// Always use the defines.
#define NL_BUILTIN_CAMERA_BINDING 0 // Builtin uniform buffer bound by driver, set by camera transformation
#define NL_BUILTIN_MODEL_BINDING 1 // Builtin uniform buffer bound by driver, set by model transformation
#define NL_BUILTIN_MATERIAL_BINDING 2 // Builtin uniform buffer bound by material
#define NL_USER_ENV_BINDING 3 // User-specified uniform buffer bound by user
#define NL_USER_VERTEX_PROGRAM_BINDING 4 // User-specified uniform buffer bound by vertex program
#define NL_USER_GEOMETRY_PROGRAM_BINDING 5 // User-specified uniform buffer bound by geometry program
#define NL_USER_PIXEL_PROGRAM_BINDING 6 // User-specified uniform buffer bound by pixel program
#define NL_USER_MATERIAL_BINDING 7 // User-specified uniform buffer bound by material

extern const char *GLSLHeaderUniformBuffer;

void generateUniformBufferGLSL(std::stringstream &ss, const CUniformBufferFormat &ubf, sint binding);

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL_UNIFORM_BUFFER_H

/* end of file */
