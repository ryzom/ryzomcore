// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_DRIVER_OPENGL3_PROGRAM_H
#define NL_DRIVER_OPENGL3_PROGRAM_H

#include "nel/misc/types_nl.h"

namespace NL3D {
namespace NLDRIVERGL3 {

class CDriverGL3;

static sint TexGenDisabled = -1;
static sint TexGenReflectionMap = 0; // GL_REFLECTION_MAP_ARB
static sint TexGenSphereMap = 1; // GL_SPHERE_MAP
static sint TexGenObjectLinear = 2; // GL_OBJECT_LINEAR
static sint TexGenEyeLinear = 3; // GL_EYE_LINEAR

/// Builtin vertex program description
struct CVPBuiltin
{
	CVPBuiltin() : VertexProgram(NULL), VertexColorLighted(false), Normalize(false), WorldSpaceNormal(false), WorldSpacePosition(false), NumPerPixelLights(0), ClipPlaneMask(0), PPClipPlane(false) { UVRouting[0] = 0; UVRouting[1] = 1; UVRouting[2] = 2; UVRouting[3] = 3; }

	uint16 VertexFormat;
	bool Lighting;
	sint LightMode[NL_OPENGL3_MAX_LIGHT]; // -1 when disabled
	sint TexGenMode[IDRV_MAT_MAXTEXTURES]; // -1 when disabled
	uint8 UVRouting[IDRV_MAT_MAXTEXTURES]; // VB texcoord index per material stage
	bool Fog;
	bool VertexColorLighted;
	bool Normalize; // Force-normalize normals (for MRM geomorphing, scaled models)
	bool WorldSpaceNormal; // Output world-space normal at VaryingLocationNormal
	bool WorldSpacePosition; // Output world-space position at VaryingLocationEcPos (instead of eye-space)
	uint8 NumPerPixelLights; // First N lights evaluated per-pixel in PP (VP skips these)
	uint8 ClipPlaneMask; // Bitmask of enabled clip planes (0-5)
	bool PPClipPlane; // PP handles clip planes (output ecPos, skip gl_ClipDistance)

	NLMISC::CRefPtr<CVertexProgram> VertexProgram;
};

bool operator<(const CVPBuiltin &left, const CVPBuiltin &right);
bool operator==(const CVPBuiltin &left, const CVPBuiltin &right);

static const uint64 Sampler2D = 0;
static const uint64 SamplerCube = 1;

/// Builtin pixel program description.
/// Per-material struct cached on CMaterialDrvInfosGL3.
/// Tracks driver state (VertexFormat, Fog, FogMode, SpecularSeparate, WorldSpacePosition,
/// PPL, PPLVertexColor, PPClipPlane) and material-derived state (Shader, Flags, TextureActive,
/// TexEnvMode, TexSamplerMode, LightMapScale).
/// Non-mega path: all fields determine which compiled PP variant to use.
/// Mega path: only TexSamplerMode selects the cube split; Shader/Flags/TextureActive/TexEnvMode
/// are read by uploadMaterialUBO() to pack the NlMaterial UBO.
struct CPPBuiltin
{
	CPPBuiltin() : Touched(true), FogMode(0), SpecularSeparate(false), WorldSpacePosition(false), LightMapScale(false), PPL(false), PPLVertexColor(false), PPClipPlane(false) { }

	// Driver state (per-draw-call, pulled in by checkDriverStateTouched)
	uint16 VertexFormat;
	bool Fog;
	uint8 FogMode;
	bool SpecularSeparate; // Whether VP outputs specularColor varying
	bool WorldSpacePosition; // Whether VP outputs world-space position (affects fog calculation)
	bool PPL; // Whether PP has per-pixel lighting code (computeLightPP, ecPos/normal varyings)
	bool PPLVertexColor; // Whether PP declares vertexColor varying and multiplies PPL by it (PPL + VertexColorLighted)
	bool PPClipPlane; // PP handles clip plane discard (declares ecPos, clipPlane uniforms)

	// Material-derived state (pushed by setupMaterial/setupLightMapPass/setupNormalPass)
	CMaterial::TShader Shader;
	uint32 Flags;                               // Masked to IDRV_MAT_ALPHA_TEST
	uint32 TextureActive;                       // Bitmask of active texture stages
	uint64 TexSamplerMode;                      // 2D vs cube per stage (not in material UBO, selects mega PP cube split)
	uint32 TexEnvMode[IDRV_MAT_MAXTEXTURES];   // Packed TexEnv per stage (Normal, UserColor shaders)
	bool LightMapScale; // Whether PP uses nlLightMapScale uniform (lightmap x2 mode)

	NLMISC::CRefPtr<CPixelProgram> PixelProgram;

	// Touched: any field changed, triggers PP recompilation or re-selection
	bool Touched;

	// Update driver state fields and mark touched if necessary
	void checkDriverStateTouched(CDriverGL3 *driver);
};

bool operator<(const CPPBuiltin &left, const CPPBuiltin &right);
bool operator==(const CPPBuiltin &left, const CPPBuiltin &right);

enum TAttribOffset
{
	Position,
	Weight,
	Normal,
	PrimaryColor,
	SecondaryColor,
	Fog,
	PaletteSkin,
	Tangent,
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

// Varying locations for VP/PP interface matching (GL_ARB_separate_shader_objects).
// VB-sourced varyings use their TAttribOffset index as the location.
// ecPos and diffuseColor reuse slots that are never occupied by VB varyings.
static const int VaryingLocationEcPos = Position; // = 0, Position is never output as a varying
static const int VaryingLocationVertexColor = Weight; // = 1, vertex color for PPL modulation
static const int VaryingLocationNormal = Normal; // = 2, world-space normal (when WorldSpaceNormal is set)
static const int VaryingLocationDiffuseColor = PrimaryColor; // = 3, PrimaryColor is always skipped
static const int VaryingLocationSpecularColor = SecondaryColor; // = 4, SecondaryColor is always skipped
static const int VaryingLocationWorldPos = Fog; // = 5, world-space position for PPL (Fog VB slot is never a VP→PP varying)
static const int VaryingLocationTangent = Tangent; // = 7, world-space tangent (vec4: xyz = tangent, w = bitangent sign)

extern const uint16 g_VertexFlags[CVertexBuffer::NumValue];
extern const char *g_AttribNames[CVertexBuffer::NumValue];
extern const char *g_TexelNames[IDRV_PROGRAM_MAXSAMPLERS];
extern const char *g_ConstantNames[IDRV_PROGRAM_MAXSAMPLERS];

/// Generate a mega VP source string. Called from initMegaVertexPrograms and compileInsertVertexProgram.
void megaVPGenerate(std::string &result, bool fogOrPpl, bool hwClip, bool tableUBO, bool cameraUBO, bool objectUBO, bool materialUBO, bool linked = false, const char *insertSource = NULL);

namespace /* anonymous */ {

inline bool hasFlag(uint32 data, uint32 flag)
{
	if ((data & flag) != 0)
		return true;
	else
		return false;
}

} /* anonymous namespace */

/// Hash traits for CVPBuiltin (compatible with CHashSet across all compilers)
struct CVPBuiltinHashTraits
{
	enum { bucket_size = 4, min_buckets = 8 };
	size_t operator()(const CVPBuiltin &v) const;
	bool operator()(const CVPBuiltin &a, const CVPBuiltin &b) const { return a < b; }
};

/// Hash traits for CPPBuiltin (compatible with CHashSet across all compilers)
struct CPPBuiltinHashTraits
{
	enum { bucket_size = 4, min_buckets = 8 };
	size_t operator()(const CPPBuiltin &v) const;
	bool operator()(const CPPBuiltin &a, const CPPBuiltin &b) const { return a < b; }
};

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL3_PROGRAM_H

/* end of file */
