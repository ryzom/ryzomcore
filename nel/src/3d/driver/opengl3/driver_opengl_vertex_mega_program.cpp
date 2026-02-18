// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2025  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

/**
 * Megashader Vertex Program
 *
 * Split vs Fold Design:
 *
 *   SPLITS (separate shader variants — correspond to separate rendering passes):
 *   - fogOrPpl (on/off): Controls ecPos varying output needed by PP fog/PPL.
 *     fogOrPpl=0 for UI/sky (no fog, no PPL). fogOrPpl=1 for world geometry
 *     where fog and PPL are runtime-gated. Always separate passes. Free split.
 *   - Clip plane (none vs active): Requires static gl_ClipDistance[6] array,
 *     forces rasterizer to interpolate 6 extra floats per fragment. Only used
 *     during water reflection and R2 editor — always a separate pass. Free split.
 *   - tableUBO (on/off): When on, lights are read from a UBO with per-object
 *     indices and factors. When off, lights use individual pre-multiplied uniforms.
 *     Different uniform sets, so a separate variant avoids dead declarations.
 *
 *   Result: 8 VP variants — m_MegaVP[fogOrPpl][clip][tableUBO]
 *
 *   FOLDS (uniform-controlled branching — zero GPU cost):
 *   - VertexFormat: All 16 in attributes declared; unbound ones read GL default
 *     (0,0,0,1). nlVertexFormat bitmask uniform guards attribute usage.
 *   - Lighting on/off: Uniform bool, skip entire light block.
 *   - LightMode[8]: Uniform int per light slot; directional/point/spot/disabled.
 *     (non-table variant only)
 *   - TexGenMode[4]: Uniform int per stage; reflection/sphere/object/eye-linear.
 *   - Specular: Folded into lighting block.
 *   - VertexColorLighted: Uniform bool controlling vertex color * lighting.
 *   - Normalize: Always normalize — normalize() on unit vector is essentially
 *     free (rsqrt+mul).
 */

#include "stdopengl.h"

#include <sstream>

#include "nel/3d/vertex_program.h"
#include "nel/3d/light.h"

#include "driver_opengl.h"
#include "driver_opengl_program.h"
#include "driver_opengl_vertex_buffer.h"
#include "driver_opengl_uniform_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

namespace /* anonymous */ {



// Packed accessors for light indices/factors in object UBO
static const char *s_LightIdxAccess[8] = {
	"nlLightIndices01.x", "nlLightIndices01.y", "nlLightIndices01.z", "nlLightIndices01.w",
	"nlLightIndices45.x", "nlLightIndices45.y", "nlLightIndices45.z", "nlLightIndices45.w"
};
static const char *s_LightFacAccess[8] = {
	"nlLightFactors01.x", "nlLightFactors01.y", "nlLightFactors01.z", "nlLightFactors01.w",
	"nlLightFactors45.x", "nlLightFactors45.y", "nlLightFactors45.z", "nlLightFactors45.w"
};
static const char *s_TexGenAccess[4] = {
	"nlTexGenMode.x", "nlTexGenMode.y", "nlTexGenMode.z", "nlTexGenMode.w"
};

void megaVPGenerate(std::string &result, bool fogOrPpl, bool clip, bool tableUBO, bool cameraUBO, bool objectUBO, bool materialUBO)
{
	// Object UBO implies tableUBO and camera UBO
	if (objectUBO) { tableUBO = true; cameraUBO = true; }

	std::stringstream ss;
	ss << "// Megashader Vertex Program";
	ss << " (fogOrPpl=" << (int)fogOrPpl << ", clip=" << (int)clip << ", tableUBO=" << (int)tableUBO
	   << ", cameraUBO=" << (int)cameraUBO << ", objectUBO=" << (int)objectUBO
	   << ", materialUBO=" << (int)materialUBO << ")" << std::endl;
	ss << std::endl;
	ss << "#version 330" << std::endl;
	ss << "#extension GL_ARB_separate_shader_objects : enable" << std::endl;
	ss << std::endl;

	// gl_PerVertex output block
	ss << "out gl_PerVertex" << std::endl;
	ss << "{" << std::endl;
	ss << "  vec4 gl_Position;" << std::endl;
	if (clip)
		ss << "  float gl_ClipDistance[6];" << std::endl;
	ss << "};" << std::endl;
	ss << std::endl;

	// NlCamera UBO block is provided by GLSLCameraHeader
	// NlModel UBO block is provided by GLSLObjectHeader
	// NlMaterial UBO block is provided by GLSLMaterialHeader
	// (all prepended automatically by compileVertexProgram when respective flags are set).

	// Matrix uniforms (skip when object UBO provides them)
	if (!objectUBO)
	{
		ss << "uniform mat4 modelViewProjection;" << std::endl;
		ss << "uniform mat4 modelView;" << std::endl;
		ss << "uniform mat3 normalMatrix;" << std::endl;
	}
	if (!cameraUBO)
		ss << "uniform mat4 viewMatrix;" << std::endl;
	ss << std::endl;

	if (tableUBO)
	{
		// NlLightInfo struct and NlLightTable UBO block are provided by GLSLBuiltinHeader

		if (!objectUBO)
		{
			// Per-object light table uniforms
			for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
			{
				ss << "uniform int nlLightIndex" << i << ";" << std::endl;
				ss << "uniform float nlLightFactor" << i << ";" << std::endl;
			}
		}
		if (!materialUBO)
		{
			ss << "uniform vec4 nlMaterialDiffuse;" << std::endl;
			ss << "uniform vec4 nlMaterialSpecular;" << std::endl;
			ss << "uniform float nlMaterialShininess;" << std::endl;
		}
		if (!cameraUBO)
			ss << "uniform vec3 pzbCameraPos;" << std::endl;
		ss << std::endl;
	}
	else
	{
		// Per-light uniforms (all 8 lights, superset of all modes)
		for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
			ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
			ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
			ss << "uniform float light" << i << "Shininess;" << std::endl;
			ss << "uniform float light" << i << "ConstAttn;" << std::endl;
			ss << "uniform float light" << i << "LinAttn;" << std::endl;
			ss << "uniform float light" << i << "QuadAttn;" << std::endl;
			ss << "uniform vec3 light" << i << "SpotDir;" << std::endl;
			ss << "uniform float light" << i << "SpotCutoff;" << std::endl;
			ss << "uniform float light" << i << "SpotExp;" << std::endl;
		}
		ss << std::endl;
	}

	if (!objectUBO)
		ss << "uniform vec4 selfIllumination;" << std::endl;
	if (!materialUBO)
		ss << "uniform vec4 materialColor;" << std::endl;
	ss << std::endl;

	// TexGen uniforms (texMatrix always individual — not in UBOs)
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		ss << "uniform mat4 texMatrix" << i << ";" << std::endl;
	ss << std::endl;

	// Clip plane uniforms (individual uniforms only when no camera UBO)
	if (clip && !cameraUBO)
	{
		for (int i = 0; i < 6; ++i)
			ss << "uniform vec4 clipPlane" << i << ";" << std::endl;
		ss << std::endl;
	}

	// Megashader control uniforms (skip those in object/material UBO)
	if (!objectUBO)
	{
		ss << "uniform int nlLighting;" << std::endl;
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			ss << "uniform int nlTexGenMode" << i << ";" << std::endl;
		ss << "uniform int nlVertexColorLighted;" << std::endl;
		ss << "uniform int nlVertexFormat;" << std::endl;
	}
	if (!tableUBO && !objectUBO)
	{
		for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
			ss << "uniform int nlLightMode" << i << ";" << std::endl;
	}
	if (clip && !cameraUBO)
		ss << "uniform int nlClipPlaneMask;" << std::endl;
	if (!objectUBO)
	{
		ss << "uniform int nlWorldSpaceNormal;" << std::endl;
		if (fogOrPpl)
		{
			ss << "uniform int nlWorldSpacePosition;" << std::endl;
			// VP light upload is shifted to slot 0, but we still need the count
			// to decide whether to split vertexColor for PPL vertex color correctness
			ss << "uniform int nlNumPerPixelLights;" << std::endl;
		}
	}
	ss << std::endl;

	// Vertex format flag constants (matching g_VertexFlags / CVertexBuffer flags)
	ss << "const int NL_VP_NORMAL_FLAG       = " << CVertexBuffer::NormalFlag << ";" << std::endl;
	ss << "const int NL_VP_PRIMARY_COLOR_FLAG = " << CVertexBuffer::PrimaryColorFlag << ";" << std::endl;
	ss << "const int NL_VP_SECONDARY_COLOR_FLAG = " << CVertexBuffer::SecondaryColorFlag << ";" << std::endl;
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		ss << "const int NL_VP_TEXCOORD" << i << "_FLAG = " << (int)g_VertexFlags[TexCoord0 + i] << ";" << std::endl;
	ss << std::endl;

	// All 16 vertex inputs (unbound → GL default (0,0,0,1))
	for (int i = Position; i < NumOffsets; ++i)
		ss << "layout(location = " << i << ") in vec4 v" << g_AttribNames[i] << ";" << std::endl;
	ss << std::endl;

	// All varyings output unconditionally
	for (int i = Weight; i < NumOffsets; ++i)
	{
		if (i == PrimaryColor || i == SecondaryColor)
			continue;
		if (fogOrPpl && i == VaryingLocationVertexColor)
			continue; // Slot used by vertexColor
		ss << "layout(location = " << i << ") smooth out vec4 " << g_AttribNames[i] << ";" << std::endl;
	}
	if (fogOrPpl)
	{
		ss << "layout(location = " << VaryingLocationEcPos << ") smooth out vec4 ecPos;" << std::endl;
		ss << "layout(location = " << VaryingLocationVertexColor << ") smooth out vec4 vertexColor;" << std::endl;
	}
	ss << "layout(location = " << VaryingLocationDiffuseColor << ") smooth out vec4 diffuseColor;" << std::endl;
	ss << "layout(location = " << VaryingLocationSpecularColor << ") smooth out vec4 specularColor;" << std::endl;
	ss << std::endl;

	// Light computation function (handles all modes via switch)
	ss << "void computeLight(int lightMode, vec3 dirOrPos, vec4 colDiff, vec4 colSpec, float shininess," << std::endl;
	ss << "                   float constAttn, float linAttn, float quadAttn," << std::endl;
	ss << "                   vec3 spotDir, float spotCutoff, float spotExp," << std::endl;
	ss << "                   vec3 normal3, vec3 ecPos3, vec3 eyeDir," << std::endl;
	ss << "                   inout vec4 lightDiffuse, inout vec4 lightSpecular)" << std::endl;
	ss << "{" << std::endl;
	ss << "  if (lightMode < 0) return;" << std::endl;
	ss << std::endl;
	ss << "  vec3 lightDir;" << std::endl;
	ss << "  float attnFactor = 1.0;" << std::endl;
	ss << std::endl;
	ss << "  if (lightMode == " << (int)CLight::DirectionalLight << ") {" << std::endl;
	ss << "    lightDir = normalize(mat3(viewMatrix) * dirOrPos);" << std::endl;
	ss << "  } else {" << std::endl;
	ss << "    // Point or spot light" << std::endl;
	ss << "    vec4 lightPos4 = viewMatrix * vec4(dirOrPos, 1.0);" << std::endl;
	ss << "    vec3 lightPos = lightPos4.xyz / lightPos4.w;" << std::endl;
	ss << "    vec3 lightVec = lightPos - ecPos3;" << std::endl;
	ss << "    float lightDistance = length(lightVec);" << std::endl;
	ss << "    lightDir = lightVec / lightDistance;" << std::endl;
	ss << "    float attenuation = constAttn + linAttn * lightDistance + quadAttn * lightDistance * lightDistance;" << std::endl;
	ss << "    attnFactor = 1.0 / attenuation;" << std::endl;
	ss << "    if (lightMode == " << (int)CLight::SpotLight << ") {" << std::endl;
	ss << "      vec3 spotDirView = normalize(mat3(viewMatrix) * spotDir);" << std::endl;
	ss << "      float spotDot = dot(-lightDir, spotDirView);" << std::endl;
	ss << "      attnFactor *= (spotDot >= spotCutoff) ? pow(spotDot, spotExp) : 0.0;" << std::endl;
	ss << "    }" << std::endl;
	ss << "  }" << std::endl;
	ss << std::endl;
	ss << "  float diffAngle = max(0.0, dot(lightDir, normal3));" << std::endl;
	ss << "  lightDiffuse += diffAngle * attnFactor * colDiff;" << std::endl;
	ss << std::endl;
	ss << "  vec3 halfVector = normalize(lightDir + eyeDir);" << std::endl;
	ss << "  float specAngle = max(0.0, dot(normal3, halfVector));" << std::endl;
	ss << "  float specPow = diffAngle > 0.0 ? pow(specAngle, shininess) : 0.0; // GL1.x LIT: no specular when surface faces away from light" << std::endl;
	ss << "  lightSpecular += specPow * attnFactor * colSpec;" << std::endl;
	ss << "}" << std::endl;
	ss << std::endl;

	// Main function
	ss << "void main(void)" << std::endl;
	ss << "{" << std::endl;
	ss << "  gl_Position = modelViewProjection * vposition;" << std::endl;
	ss << std::endl;

	// Eye-space position (always needed: lighting, texgen, clip, fog)
	ss << "  vec4 ecPos4 = modelView * vposition;" << std::endl;
	if (fogOrPpl)
	{
		ss << "  if (nlWorldSpacePosition != 0)" << std::endl;
		ss << "    ecPos = vec4(transpose(mat3(viewMatrix)) * (ecPos4.xyz - viewMatrix[3].xyz * ecPos4.w), ecPos4.w);" << std::endl;
		ss << "  else" << std::endl;
		ss << "    ecPos = ecPos4;" << std::endl;
		// Default vertexColor to identity; overwritten when VertexColorLighted + PPL
		ss << "  vertexColor = vec4(1.0);" << std::endl;
	}
	ss << std::endl;

	// Pass through all varyings (always normalize normals, output world-space normal)
	for (int i = Weight; i < NumOffsets; ++i)
	{
		if (i == PrimaryColor || i == SecondaryColor)
			continue;
		if (fogOrPpl && i == VaryingLocationVertexColor)
			continue; // Slot used by vertexColor
		if (i == Normal)
		{
			ss << "  if ((nlVertexFormat & NL_VP_NORMAL_FLAG) != 0) {" << std::endl;
			ss << "    if (nlWorldSpaceNormal != 0)" << std::endl;
			// World-space normal: eye-space via normalMatrix, then undo view rotation
			ss << "      " << g_AttribNames[i] << " = vec4(transpose(mat3(viewMatrix)) * normalize(normalMatrix * (v" << g_AttribNames[i] << ".xyz / v" << g_AttribNames[i] << ".w)), 0.0);" << std::endl;
			ss << "    else" << std::endl;
			ss << "      " << g_AttribNames[i] << " = vec4(normalize(v" << g_AttribNames[i] << ".xyz), 0.0);" << std::endl;
			ss << "  } else" << std::endl;
			ss << "    " << g_AttribNames[i] << " = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
		}
		else
			ss << "  " << g_AttribNames[i] << " = v" << g_AttribNames[i] << ";" << std::endl;
	}
	ss << std::endl;

	// Lighting
	ss << "  vec4 diffuseVertex;" << std::endl;
	ss << "  vec4 specularVertex = vec4(0.0);" << std::endl;
	ss << "  bool doLighting = (nlLighting != 0) && ((nlVertexFormat & NL_VP_NORMAL_FLAG) != 0);" << std::endl;
	ss << std::endl;

	ss << "  if (doLighting) {" << std::endl;
	ss << "    vec3 normal3 = normalize(normalMatrix * (vnormal.xyz / vnormal.w));" << std::endl;
	ss << "    vec3 ecPos3 = ecPos4.xyz / ecPos4.w;" << std::endl;
	ss << "    vec3 eyeDir = normalize(-ecPos3);" << std::endl;
	ss << "    diffuseVertex = vec4(0.0);" << std::endl;
	ss << "    specularVertex = vec4(0.0);" << std::endl;

	// When materialUBO is active, compute effective diffuse from UBO members
	if (materialUBO)
	{
		ss << "    vec4 effectiveDiffuse = (nlVertexColorLighted != 0) ? vec4(1.0) : materialDiffuse;" << std::endl;
	}

	if (tableUBO)
	{
		// Material properties: from UBO or individual uniforms
		const char *matDiffStr = materialUBO ? "effectiveDiffuse" : "nlMaterialDiffuse";
		const char *matSpecStr = materialUBO ? "materialSpecular" : "nlMaterialSpecular";
		const char *matShinStr = materialUBO ? "materialShininess" : "nlMaterialShininess";

		// Unrolled light table lookups
		// objectUBO: VP and PP share UBO slots, guard with nlNumPerPixelLights
		// !objectUBO: upload path shifts VP lights to slot 0, no guard needed
		for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			// Light index/factor accessor: packed from UBO or individual uniforms
			const char *idxAccess = objectUBO ? s_LightIdxAccess[i] : NULL;
			const char *facAccess = objectUBO ? s_LightFacAccess[i] : NULL;

			if (objectUBO)
				ss << "    if (" << i << " >= nlNumPerPixelLights) {" << std::endl;
			ss << "    {" << std::endl;
			if (objectUBO)
				ss << "      int idx = " << idxAccess << ";" << std::endl;
			else
				ss << "      int idx = nlLightIndex" << i << ";" << std::endl;
			ss << "      if (idx >= 0) {" << std::endl;
			ss << "        NlLightInfo li = nlLights[idx];" << std::endl;
			if (objectUBO)
				ss << "        float factor = " << facAccess << ";" << std::endl;
			else
				ss << "        float factor = nlLightFactor" << i << ";" << std::endl;
			ss << "        vec3 adjDirOrPos;" << std::endl;
			ss << "        if (li.mode == " << (int)CLight::DirectionalLight << ")" << std::endl;
			ss << "          adjDirOrPos = -li.dirOrPos;" << std::endl;
			ss << "        else" << std::endl;
			ss << "          adjDirOrPos = li.dirOrPos - pzbCameraPos;" << std::endl;
			ss << "        computeLight(li.mode, adjDirOrPos," << std::endl;
			ss << "          li.diffuse * factor * " << matDiffStr << "," << std::endl;
			ss << "          li.specular * factor * " << matSpecStr << "," << std::endl;
			ss << "          " << matShinStr << "," << std::endl;
			ss << "          li.constAttn, li.linAttn, li.quadAttn," << std::endl;
			ss << "          li.spotDir, li.spotCutoff, li.spotExp," << std::endl;
			ss << "          normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);" << std::endl;
			ss << "      }" << std::endl;
			ss << "    }" << std::endl;
			if (objectUBO)
				ss << "    }" << std::endl;
		}
	}
	else
	{
		// Unrolled 8-light calls with individual uniforms
		// Upload path shifts VP lights to slot 0, no guard needed
		for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			ss << "    computeLight(nlLightMode" << i
				<< ", light" << i << "DirOrPos"
				<< ", light" << i << "ColDiff"
				<< ", light" << i << "ColSpec"
				<< ", light" << i << "Shininess"
				<< ", light" << i << "ConstAttn"
				<< ", light" << i << "LinAttn"
				<< ", light" << i << "QuadAttn"
				<< ", light" << i << "SpotDir"
				<< ", light" << i << "SpotCutoff"
				<< ", light" << i << "SpotExp"
				<< ", normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);" << std::endl;
		}
	}

	ss << "    diffuseVertex.a = 1.0;" << std::endl;
	ss << "    specularVertex.a = 1.0;" << std::endl;
	ss << std::endl;

	// Secondary color (lighted)
	ss << "    if ((nlVertexFormat & NL_VP_SECONDARY_COLOR_FLAG) != 0)" << std::endl;
	ss << "      specularVertex = specularVertex * vsecondaryColor;" << std::endl;
	ss << std::endl;

	// Add selfIllumination before vertex color multiply so it's also modulated
	// by vertex color (GL_COLOR_MATERIAL AMBIENT_AND_DIFFUSE behavior)
	ss << "    diffuseVertex.rgb = diffuseVertex.rgb + selfIllumination.rgb;" << std::endl;
	ss << std::endl;

	// Primary color × lighting interaction (GL_COLOR_MATERIAL behavior)
	// When VertexColorLighted: vertex color replaces material diffuse, multiply here.
	// When PPL active: also pass vertex color to PP via vertexColor for PPL term.
	if (fogOrPpl)
	{
		ss << "    if ((nlVertexFormat & NL_VP_PRIMARY_COLOR_FLAG) != 0 && nlVertexColorLighted != 0) {" << std::endl;
		ss << "      diffuseVertex = diffuseVertex * vprimaryColor;" << std::endl;
		ss << "      if (nlNumPerPixelLights > 0)" << std::endl;
		ss << "        vertexColor = vprimaryColor;" << std::endl;
		ss << "    }" << std::endl;
	}
	else
	{
		ss << "    if ((nlVertexFormat & NL_VP_PRIMARY_COLOR_FLAG) != 0 && nlVertexColorLighted != 0)" << std::endl;
		ss << "      diffuseVertex = diffuseVertex * vprimaryColor;" << std::endl;
	}
	// When lighting && !VertexColorLighted: vprimaryColor is ignored (matDiffuse pre-multiplied on CPU)

	ss << "  } else {" << std::endl;
	ss << "    // Unlit" << std::endl;
	ss << "    diffuseVertex = materialColor;" << std::endl;
	ss << "    if ((nlVertexFormat & NL_VP_PRIMARY_COLOR_FLAG) != 0)" << std::endl;
	ss << "      diffuseVertex = diffuseVertex * vprimaryColor;" << std::endl;
	ss << "    if ((nlVertexFormat & NL_VP_SECONDARY_COLOR_FLAG) != 0)" << std::endl;
	ss << "      specularVertex = vsecondaryColor;" << std::endl;
	ss << "  }" << std::endl;
	ss << std::endl;

	// Combine diffuse (clamp before texture), specular passed separately (added post-texture in PP)
	ss << "  diffuseColor = clamp(diffuseVertex, 0.0, 1.0);" << std::endl;
	ss << "  specularColor = clamp(vec4(specularVertex.rgb * specularVertex.a, 0.0), 0.0, 1.0);" << std::endl;
	ss << std::endl;

	// Compute reflection vector for texgen (shared by reflection/sphere stages)
	ss << "  vec3 refl_r;" << std::endl;
	ss << "  if ((nlVertexFormat & NL_VP_NORMAL_FLAG) != 0) {" << std::endl;
	ss << "    vec3 refl_n = normalize(normalMatrix * (vnormal.xyz / vnormal.w));" << std::endl;
	ss << "    vec3 refl_u = normalize(ecPos4.xyz);" << std::endl;
	ss << "    refl_r = reflect(refl_u, refl_n);" << std::endl;
	ss << "  } else {" << std::endl;
	ss << "    refl_r = vec3(0.0, 0.0, -1.0);" << std::endl;
	ss << "  }" << std::endl;
	ss << std::endl;

	// TexGen (unrolled per stage, uniform-switched)
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		// TexGen mode accessor: packed from UBO or individual uniforms
		const char *tgmAccess = objectUBO ? s_TexGenAccess[i] : NULL;

		if (objectUBO)
			ss << "  if (" << tgmAccess << " == " << TexGenObjectLinear << ")" << std::endl;
		else
			ss << "  if (nlTexGenMode" << i << " == " << TexGenObjectLinear << ")" << std::endl;
		ss << "    texCoord" << i << " = texMatrix" << i << " * vposition;" << std::endl;
		if (objectUBO)
			ss << "  else if (" << tgmAccess << " == " << TexGenEyeLinear << ")" << std::endl;
		else
			ss << "  else if (nlTexGenMode" << i << " == " << TexGenEyeLinear << ")" << std::endl;
		ss << "    texCoord" << i << " = texMatrix" << i << " * ecPos4;" << std::endl;
		if (objectUBO)
			ss << "  else if (" << tgmAccess << " == " << TexGenReflectionMap << ")" << std::endl;
		else
			ss << "  else if (nlTexGenMode" << i << " == " << TexGenReflectionMap << ")" << std::endl;
		ss << "    texCoord" << i << " = texMatrix" << i << " * vec4(refl_r, 0.0);" << std::endl;
		if (objectUBO)
			ss << "  else if (" << tgmAccess << " == " << TexGenSphereMap << ") {" << std::endl;
		else
			ss << "  else if (nlTexGenMode" << i << " == " << TexGenSphereMap << ") {" << std::endl;
		ss << "    float refl_m = 2.0 * sqrt(refl_r.x * refl_r.x + refl_r.y * refl_r.y + (refl_r.z + 1.0) * (refl_r.z + 1.0));" << std::endl;
		ss << "    texCoord" << i << " = texMatrix" << i << " * vec4(refl_r.x / refl_m + 0.5, refl_r.y / refl_m + 0.5, 0.0, 1.0);" << std::endl;
		ss << "  }" << std::endl;
		ss << "  // else: texCoord" << i << " already set from vertex attribute passthrough" << std::endl;
		ss << std::endl;
	}

	// Clip planes (clip variant only)
	if (clip)
	{
		for (int i = 0; i < 6; ++i)
		{
			ss << "  if ((nlClipPlaneMask & " << (1 << i) << ") != 0)" << std::endl;
			ss << "    gl_ClipDistance[" << i << "] = dot(clipPlane" << i << ", ecPos4);" << std::endl;
			ss << "  else" << std::endl;
			ss << "    gl_ClipDistance[" << i << "] = 1.0;" << std::endl;
		}
	}

	ss << "}" << std::endl;

	result = ss.str();
}

} /* anonymous namespace */

bool CDriverGL3::initMegaVertexPrograms()
{
	for (int fogOrPpl = 0; fogOrPpl < 2; ++fogOrPpl)
	{
		for (int clip = 0; clip < 2; ++clip)
		{
			for (int tableUBO = 0; tableUBO < 2; ++tableUBO)
			{
				for (int cameraUBO = 0; cameraUBO < 2; ++cameraUBO)
				{
					for (int objectUBO = 0; objectUBO < 2; ++objectUBO)
					{
						for (int materialUBO = 0; materialUBO < 2; ++materialUBO)
						{
							// objectUBO implies tableUBO and cameraUBO
							if (objectUBO && (!tableUBO || !cameraUBO))
								continue;

							std::string result;
							megaVPGenerate(result, fogOrPpl != 0, clip != 0, tableUBO != 0, cameraUBO != 0, objectUBO != 0, materialUBO != 0);

							CVertexProgram *vp = new CVertexProgram();
							IProgram::CSource *src = new IProgram::CSource();
							src->Profile = IProgram::glsl330v;
							src->DisplayName = NLMISC::toString("Mega VP (fogOrPpl=%d, clip=%d, tableUBO=%d, cam=%d, obj=%d, mat=%d)", fogOrPpl, clip, tableUBO, cameraUBO, objectUBO, materialUBO);
							src->Features.UsesLightTableUBO = (tableUBO != 0);
							src->Features.UsesCameraUBO = (cameraUBO != 0);
							src->Features.UsesObjectUBO = (objectUBO != 0);
							src->Features.UsesMaterialUBO = (materialUBO != 0);
							src->setSource(result);
							vp->addSource(src);

							nldebug("GL3: Compile '%s'", src->DisplayName.c_str());

							if (!compileVertexProgram(vp))
							{
								nlwarning("GL3: Mega VP compilation failed (%s)", src->DisplayName.c_str());
								delete vp;
								return false;
							}

							m_MegaVP[fogOrPpl][clip][tableUBO][cameraUBO][objectUBO][materialUBO] = vp;
						}
					}
				}
			}
		}
	}
	return true;
}

bool CDriverGL3::setupMegaVertexProgram()
{
	// Note: touchVertexFormatVP() already called by setupBuiltinVertexProgram()

	nlassert(!m_UserVertexProgram); // See setupBuiltinVertexProgram

	m_VPSpecularOutput = true; // Mega VP always outputs specularColor

	// Mega VP outputs world-space normal when requested by PP
	m_VPNormalOutput = false;
	if (m_UserPixelProgram)
		m_VPNormalOutput = m_UserPixelProgram->features().InputsWorldSpaceNormal;

	// Mega VP outputs world-space position when requested by PP
	m_VPWorldSpacePositionOutput = false;
	if (m_UserPixelProgram)
		m_VPWorldSpacePositionOutput = m_UserPixelProgram->features().InputsWorldSpacePosition;

	// Activate PPL only if the paired PP supports it
	// (m_VPBuiltinCurrent.NumPerPixelLights is the canonical value, set in setupBuiltinVertexProgram)
	bool pplActive = false;
	if (m_VPBuiltinCurrent.NumPerPixelLights > 0)
	{
		if (m_UserPixelProgram)
		{
			// User PP with object UBO: supports PPL dynamically
			if (m_UserPixelProgram->features().UsesObjectUBO)
				pplActive = true;
			// User PP without UBO: only if it statically requests world-space inputs
			else if (m_UserPixelProgram->features().InputsWorldSpacePosition
			      && m_UserPixelProgram->features().InputsWorldSpaceNormal)
				pplActive = true;
		}
		else
		{
			// Mega PP always supports PPL
			pplActive = true;
		}
	}
	if (pplActive)
	{
		m_VPWorldSpacePositionOutput = true;
		m_VPNormalOutput = true;
	}

	int fogOrPpl = (m_VPBuiltinCurrent.Fog || pplActive) ? 1 : 0;
	int clip = (m_VPBuiltinCurrent.ClipPlaneMask != 0) ? 1 : 0;
	int tableUBO = (m_UseMegaLightTableUBO || m_UseMegaObjectUBO) ? 1 : 0;
	int cameraUBO = (m_UseMegaCameraUBO || m_UseMegaObjectUBO) ? 1 : 0;
	int objectUBO = m_UseMegaObjectUBO ? 1 : 0;
	int materialUBO = m_UseMegaMaterialUBO ? 1 : 0;

	m_ProgramUsesLightTableUBO[VertexProgram] = tableUBO;
	m_ProgramUsesCameraUBO[VertexProgram] = cameraUBO;
	m_ProgramUsesObjectUBO[VertexProgram] = objectUBO;
	m_ProgramUsesMaterialUBO[VertexProgram] = materialUBO;

	CVertexProgram *vp = m_MegaVP[fogOrPpl][clip][tableUBO][cameraUBO][objectUBO][materialUBO];
	nlassert(vp);

	if (!activeVertexProgram(vp, true))
		return false;

	setupMegaVPUniforms();
	return true;
}

void CDriverGL3::setupMegaVPUniforms()
{
	// All mega VP uniforms are uploaded by setupUniforms(VertexProgram).
}

} // NLDRIVERGL3
} // NL3D

/* end of file */
