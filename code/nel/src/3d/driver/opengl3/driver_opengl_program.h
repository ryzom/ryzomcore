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

/// Builtin vertex program description
struct CVPBuiltin
{
	uint16 VertexFormat;
	bool Lighting;
	sint LightMode[NL_OPENGL3_MAX_LIGHT]; // -1 when disabled
	bool Specular; // Reflection
	bool Fog;
	// bool VertexColorLighted;

	CVertexProgram *VertexProgram;
};

bool operator<(const CVPBuiltin &left, const CVPBuiltin &right);

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
extern const char *g_TexelNames[IDRV_MAT_MAXTEXTURES];
extern const char *g_ConstantNames[4];

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
