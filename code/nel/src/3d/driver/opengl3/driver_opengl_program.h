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

#ifndef NL_DRIVER_OPENGL_PROGRAM_H
#define NL_DRIVER_OPENGL_PROGRAM_H

#include "nel/misc/types_nl.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

class CDriverGL3;

static sint TexGenDisabled = -1;
static sint TexGenReflectionMap = 0; // GL_REFLECTION_MAP_ARB
static sint TexGenSphereMap = 1; // GL_SPHERE_MAP
static sint TexGenObjectLinear = 2; // GL_OBJECT_LINEAR
static sint TexGenEyeLinear = 3; // GL_EYE_LINEAR

/// Builtin vertex program description
struct CVPBuiltin
{
	CVPBuiltin() : VertexProgram(NULL) { }

	uint16 VertexFormat;
	bool Lighting;
	sint LightMode[NL_OPENGL3_MAX_LIGHT]; // -1 when disabled
	sint TexGenMode[IDRV_MAT_MAXTEXTURES]; // -1 when disabled
	bool Specular; // Reflection
	bool Fog;
	// bool VertexColorLighted;

	NLMISC::CRefPtr<CVertexProgram> VertexProgram;
};

bool operator<(const CVPBuiltin &left, const CVPBuiltin &right);

static const uint64 Sampler2D = 0;
static const uint64 SamplerCube = 1;

/// Builtin pixel program description
struct CPPBuiltin
{
	CPPBuiltin() : Touched(true) { }

	uint16 VertexFormat;
	bool Fog;

	CMaterial::TShader Shader;
	uint32 Flags;
	uint32 TextureActive;
	uint64 TexSamplerMode;
	uint32 TexEnvMode[IDRV_MAT_MAXTEXTURES]; // Normal, UserColor

	NLMISC::CRefPtr<CPixelProgram> PixelProgram;

	bool Touched;

	void checkDriverStateTouched(CDriverGL3 *driver);
	void checkDriverMaterialStateTouched(CDriverGL3 *driver, CMaterial &mat);
	void checkMaterialStateTouched(CMaterial &mat);
};

bool operator<(const CPPBuiltin &left, const CPPBuiltin &right);

enum TAttribOffset
{
	Position,
	Weight,
	Normal,
	PrimaryColor,
	SecondaryColor,
	Fog,
	PaletteSkin,
	Empty,
	TexCoord0,
	TexCoord1,
	TexCoord2,
	TexCoord3,
	TexCoord4,
	TexCoord5,
	TexCoord6,
	TexCoord7,
	NumOffsets
};

extern const uint16 g_VertexFlags[CVertexBuffer::NumValue];
extern const char *g_AttribNames[CVertexBuffer::NumValue];
extern const char *g_TexelNames[IDRV_PROGRAM_MAXSAMPLERS];
extern const char *g_ConstantNames[IDRV_PROGRAM_MAXSAMPLERS];

namespace /* anonymous */ {

inline bool hasFlag(uint32 data, uint32 flag)
{
	if ((data & flag) != 0)
		return true;
	else
		return false;
}

} /* anonymous namespace */

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

#endif // NL_DRIVER_OPENGL_PROGRAM_H

/* end of file */
