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

#include "stdopengl.h"
#include "driver_opengl.h"
#include "driver_opengl_uniform_buffer.h"

#include <sstream>

#include <nel/misc/debug.h>

namespace NL3D {
namespace NLDRIVERGL3 {

const char *GLSLHeaderUniformBuffer =
	"#define NL_BUILTIN_CAMERA_BINDING " NL_MACRO_TO_STR(NL_BUILTIN_CAMERA_BIND) "\n"
	"#define NL_BUILTIN_MODEL_BINDING " NL_MACRO_TO_STR(NL_BUILTIN_MODEL_BIND) "\n"
	"#define NL_BUILTIN_MATERIAL_BINDING " NL_MACRO_TO_STR(NL_BUILTIN_MATERIAL_BIND) "\n"
	"#define NL_USER_ENV_BINDING " NL_MACRO_TO_STR(NL_USER_ENV_BIND) "\n"
	"#define NL_USER_VERTEX_PROGRAM_BINDING " NL_MACRO_TO_STR(NL_USER_VERTEX_PROGRAM_BIND) "\n"
	"#define NL_USER_GEOMETRY_PROGRAM_BINDING " NL_MACRO_TO_STR(NL_USER_GEOMETRY_PROGRAM_BIND) "\n"
	"#define NL_USER_PIXEL_PROGRAM_BINDING " NL_MACRO_TO_STR(NL_USER_PIXEL_PROGRAM_BIND) "\n"
	"#define NL_USER_MATERIAL_BINDING " NL_MACRO_TO_STR(NL_USER_MATERIAL_BIND) "\n";

static const char *s_UniformBufferBindDefine[] = {
	"NL_BUILTIN_CAMERA_BINDING",
	"NL_BUILTIN_MODEL_BINDING",
	"NL_BUILTIN_MATERIAL_BINDING",
	"NL_USER_ENV_BINDING",
	"NL_USER_VERTEX_PROGRAM_BINDING",
	"NL_USER_GEOMETRY_PROGRAM_BINDING",
	"NL_USER_PIXEL_PROGRAM_BINDING",
	"NL_USER_MATERIAL_BINDING",
};

static const char *s_UniformBufferName[] = {
	"BuiltinCamera",
	"BuiltinModel",
	"BuiltinMaterial",
	"UserEnv",
	"UserLocal", // Yes, there can only be one per stage here, as these are bound to the stage
	"UserLocal",
	"UserLocal",
	"UserMaterial",
};

static const char *s_TypeKeyword[] = {
	"float", // float
	"vec2", // CVector2D
	"vec3",
	"vec4", // CVector
	"int", // sint32
	"ivec2",
	"ivec3",
	"ivec3",
	"unsigned int", // uint32
	"uvec2",
	"uvec3",
	"uvec4",
	"bool",
	"bvec2",
	"bvec3",
	"bvec4",
	"mat2",
	"mat3",
	"mat4", // CMatrix
	"mat2x3",
	"mat2x4",
	"mat3x2",
	"mat3x4",
	"mat4x2",
	"mat4x3",
};

void generateUniformBufferGLSL(std::stringstream &ss, const CUniformBufferFormat &ubf, sint binding)
{
	ss << "layout(std140, binding = " << s_UniformBufferBindDefine[binding] << ") uniform " << s_UniformBufferName[binding] << "\n";
	ss << "{\n";
	for (sint i = 0; i < ubf.size(); ++i)
	{
		const CUniformBufferFormat::CEntry &entry = ubf.get(i);
		ss << "\t" << s_TypeKeyword[entry.Type] << " " << NLMISC::CStringMapper::unmap(entry.Name);
		if (entry.Count != 1)
			ss << "[" << entry.Count << "]";
		ss << ";\n";
	}
	ss << "}\n";
}

} // NLDRIVERGL3
} // NL3D

/* end of file */
