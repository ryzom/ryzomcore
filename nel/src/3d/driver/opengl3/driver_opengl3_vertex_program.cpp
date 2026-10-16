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

#include "stdopengl3.h"

#include <sstream>

#include "nel/misc/wang_hash.h"

#include "nel/3d/vertex_program.h"
#include "nel/3d/light.h"

#include "driver_opengl3.h"
#include "driver_opengl3_program.h"
#include "driver_opengl3_vertex_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// Helper: get effective VP light mode at VP slot (shifted by NumPerPixelLights)
static inline sint vpLightMode(const CVPBuiltin &desc, sint vpSlot)
{
	sint drvSlot = vpSlot + desc.NumPerPixelLights;
	return (drvSlot < NL_OPENGL3_MAX_LIGHT) ? desc.LightMode[drvSlot] : -1;
}

// Whether PPL is active — changes shader structure (vertexColor varying)
static inline bool vpHasPPL(const CVPBuiltin &desc)
{
	return desc.NumPerPixelLights > 0;
}

bool operator<(const CVPBuiltin &left, const CVPBuiltin &right)
{
	if (left.VertexFormat != right.VertexFormat)
		return left.VertexFormat < right.VertexFormat;
	if (left.Lighting != right.Lighting)
		return right.Lighting;
	if (left.Lighting)
	{
		// Compare shifted VP light layout (NumPerPixelLights is implicit, not part of key)
		for (sint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			sint lm = vpLightMode(left, i);
			sint rm = vpLightMode(right, i);
			if (lm != rm)
				return lm < rm;
		}
		// PPL active changes shader structure (vertexColor varying output)
		if (vpHasPPL(left) != vpHasPPL(right))
			return vpHasPPL(right);
	}
	for (sint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		if (left.TexGenMode[i] != right.TexGenMode[i])
			return left.TexGenMode[i] < right.TexGenMode[i];
	for (sint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		if (left.UVRouting[i] != right.UVRouting[i])
			return left.UVRouting[i] < right.UVRouting[i];
	if (left.Fog != right.Fog)
		return right.Fog;
	if (left.VertexColorLighted != right.VertexColorLighted)
		return right.VertexColorLighted;
	if (left.Normalize != right.Normalize)
		return right.Normalize;
	if (left.WorldSpaceNormal != right.WorldSpaceNormal)
		return right.WorldSpaceNormal;
	if (left.WorldSpacePosition != right.WorldSpacePosition)
		return right.WorldSpacePosition;
	if (left.ClipPlaneMask != right.ClipPlaneMask)
		return left.ClipPlaneMask < right.ClipPlaneMask;
	if (left.PPClipPlane != right.PPClipPlane)
		return right.PPClipPlane;

	return false;
}

bool operator==(const CVPBuiltin &left, const CVPBuiltin &right)
{
	if (left.VertexFormat != right.VertexFormat)
		return false;
	if (left.Lighting != right.Lighting)
		return false;
	if (left.Lighting)
	{
		// Compare shifted VP light layout (NumPerPixelLights is implicit, not part of key)
		for (sint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
			if (vpLightMode(left, i) != vpLightMode(right, i))
				return false;
		// PPL active changes shader structure (vertexColor varying output)
		if (vpHasPPL(left) != vpHasPPL(right))
			return false;
	}
	for (sint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		if (left.TexGenMode[i] != right.TexGenMode[i])
			return false;
	for (sint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		if (left.UVRouting[i] != right.UVRouting[i])
			return false;
	if (left.Fog != right.Fog)
		return false;
	if (left.VertexColorLighted != right.VertexColorLighted)
		return false;
	if (left.Normalize != right.Normalize)
		return false;
	if (left.WorldSpaceNormal != right.WorldSpaceNormal)
		return false;
	if (left.WorldSpacePosition != right.WorldSpacePosition)
		return false;
	if (left.ClipPlaneMask != right.ClipPlaneMask)
		return false;
	if (left.PPClipPlane != right.PPClipPlane)
		return false;

	return true;
}

size_t CVPBuiltinHashTraits::operator()(const CVPBuiltin & v) const
{
	uint32 h;

	h = NLMISC::lowbias32(((uint32)v.VertexFormat) | (v.Lighting ? (1 << 16) : 0) | (v.Fog ? (1 << 17) : 0) | (v.VertexColorLighted ? (1 << 18) : 0) | ((uint32)v.ClipPlaneMask << 19) | (v.Normalize ? (1 << 25) : 0) | (v.WorldSpaceNormal ? (1 << 26) : 0) | (v.WorldSpacePosition ? (1 << 27) : 0) | (vpHasPPL(v) ? (1 << 28) : 0) | (v.PPClipPlane ? (1 << 29) : 0));
	if (v.Lighting)
		for (sint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
			h = NLMISC::lowbias32(h ^ vpLightMode(v, i));
	for (sint i = 0; i < NL3D::IDRV_MAT_MAXTEXTURES; ++i)
		h = NLMISC::lowbias32(h ^ v.TexGenMode[i]);
	for (sint i = 0; i < NL3D::IDRV_MAT_MAXTEXTURES; ++i)
		h = NLMISC::lowbias32(h ^ v.UVRouting[i]);

	nlctassert(sizeof(size_t) >= sizeof(uint32));
	return (size_t)h;
}

} // NLDRIVERGL3
} // NL3D

namespace NL3D {
namespace NLDRIVERGL3 {

namespace /* anonymous */ {

void vpLightUniforms(std::stringstream &ss, sint mode, int i)
{
	switch (mode)
	{
	case CLight::DirectionalLight:
		ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
		ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
		//ss << "uniform vec4 light" << i << "ColAmb;" << std::endl;
		ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
		ss << "uniform float light" << i << "Shininess;" << std::endl;
		break;
	case CLight::PointLight:
		ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
		ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
		//ss << "uniform vec4 light" << i << "ColAmb;" << std::endl;
		ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
		ss << "uniform float light" << i << "Shininess;" << std::endl;
		ss << "uniform float light" << i << "ConstAttn;" << std::endl;
		ss << "uniform float light" << i << "LinAttn;" << std::endl;
		ss << "uniform float light" << i << "QuadAttn;" << std::endl;
		break;
	case CLight::SpotLight:
		ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
		ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
		ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
		ss << "uniform float light" << i << "Shininess;" << std::endl;
		ss << "uniform float light" << i << "ConstAttn;" << std::endl;
		ss << "uniform float light" << i << "LinAttn;" << std::endl;
		ss << "uniform float light" << i << "QuadAttn;" << std::endl;
		ss << "uniform vec3 light" << i << "SpotDir;" << std::endl;
		ss << "uniform float light" << i << "SpotCutoff;" << std::endl; // cosine of cutoff angle
		ss << "uniform float light" << i << "SpotExp;" << std::endl;
		break;
	}
}

void vpLightFunctions(std::stringstream &ss, sint mode, int i)
{
	switch (mode)
	{
	case CLight::DirectionalLight:
		ss << "float getIntensity" << i << "(vec3 normal3, vec3 lightDir)" << std::endl;
		ss << "{" << std::endl;
		ss << "float angle = dot(lightDir, normal3);" << std::endl;
		ss << "angle = max(0.0, angle);" << std::endl;
		ss << "return angle;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "float getSpecIntensity" << i << "(vec3 normal3, vec3 lightDir, vec3 eyeDir)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 halfVector = normalize(lightDir + eyeDir);" << std::endl;
		ss << "float angle = dot(normal3, halfVector);" << std::endl;
		ss << "angle = max(0.0, angle);" << std::endl;
		ss << "float si = pow(angle, light" << i << "Shininess);" << std::endl;
		ss << "return si;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "void addLight" << i << "Color(inout vec4 lightDiffuse, inout vec4 lightSpecular)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 lightDir = mat3(viewMatrix) * light" << i << "DirOrPos;" << std::endl;
		ss << "lightDir = normalize(lightDir);" << std::endl;
		ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
		ss << "normal3 = normalMatrix * normal3;" << std::endl;
		ss << "normal3 = normalize(normal3);" << std::endl;
		ss << "vec3 eyeDir = normalize(-ecPos4.xyz);" << std::endl;

		ss << "float di = getIntensity" << i << "(normal3, lightDir);" << std::endl;
		ss << "lightDiffuse = lightDiffuse + di * light" << i << "ColDiff;" << std::endl;
		// GL1.x LIT: no specular when surface faces away from light
		ss << "lightSpecular = lightSpecular + (di > 0.0 ? getSpecIntensity" << i << "(normal3, lightDir, eyeDir) : 0.0) * light" << i << "ColSpec;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;
		break;
	case CLight::PointLight:
		ss << "float getIntensity" << i << "(vec3 normal3, vec3 direction3)" << std::endl;
		ss << "{" << std::endl;
		ss << "float angle = dot(direction3, normal3);" << std::endl;
		ss << "angle = max(0.0, angle);" << std::endl;
		ss << "return angle;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "float getSpecIntensity" << i << "(vec3 normal3, vec3 direction3, vec3 eyeDir)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 halfVector = normalize(direction3 + eyeDir);" << std::endl;
		ss << "float angle = dot(normal3, halfVector);" << std::endl;
		ss << "angle = max(0.0, angle);" << std::endl;
		ss << "float si = pow(angle, light" << i << "Shininess);" << std::endl;
		ss << "return si;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "void addLight" << i << "Color(inout vec4 lightDiffuse, inout vec4 lightSpecular)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 ecPos3 = ecPos4.xyz / ecPos4.w;" << std::endl;
		ss << "vec4 lightPos4 = viewMatrix * vec4(light" << i << "DirOrPos, 1.0);" << std::endl;
		ss << "vec3 lightPos = lightPos4.xyz / lightPos4.w;" << std::endl;
		ss << "vec3 lightDirection = lightPos - ecPos3;" << std::endl;
		ss << "float lightDistance = length(lightDirection);" << std::endl;
		ss << "lightDirection = normalize(lightDirection);" << std::endl;

		ss << "float attenuation = light" << i << "ConstAttn + ";
		ss << "light" << i << "LinAttn * lightDistance +";
		ss << "light" << i << "QuadAttn * lightDistance * lightDistance;" << std::endl;

		ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
		ss << "normal3 = normalMatrix * normal3;" << std::endl;
		ss << "normal3 = normalize(normal3);" << std::endl;
		ss << "vec3 eyeDir = normalize(-ecPos3);" << std::endl;

		ss << "float invattn = 1.0 / attenuation;" << std::endl;
		ss << "float di = getIntensity" << i << "(normal3, lightDirection);" << std::endl;
		ss << "lightDiffuse = lightDiffuse + di * invattn * light" << i << "ColDiff;" << std::endl;
		// GL1.x LIT: no specular when surface faces away from light
		ss << "lightSpecular = lightSpecular + (di > 0.0 ? getSpecIntensity" << i << "(normal3, lightDirection, eyeDir) : 0.0) * invattn * light" << i << "ColSpec;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;
		break;
	case CLight::SpotLight:
		ss << "float getIntensity" << i << "(vec3 normal3, vec3 direction3)" << std::endl;
		ss << "{" << std::endl;
		ss << "float angle = dot(direction3, normal3);" << std::endl;
		ss << "angle = max(0.0, angle);" << std::endl;
		ss << "return angle;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "float getSpecIntensity" << i << "(vec3 normal3, vec3 direction3, vec3 eyeDir)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 halfVector = normalize(direction3 + eyeDir);" << std::endl;
		ss << "float angle = dot(normal3, halfVector);" << std::endl;
		ss << "angle = max(0.0, angle);" << std::endl;
		ss << "float si = pow(angle, light" << i << "Shininess);" << std::endl;
		ss << "return si;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "void addLight" << i << "Color(inout vec4 lightDiffuse, inout vec4 lightSpecular)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 ecPos3 = ecPos4.xyz / ecPos4.w;" << std::endl;
		ss << "vec4 lightPos4 = viewMatrix * vec4(light" << i << "DirOrPos, 1.0);" << std::endl;
		ss << "vec3 lightPos = lightPos4.xyz / lightPos4.w;" << std::endl;
		ss << "vec3 lightDirection = lightPos - ecPos3;" << std::endl;
		ss << "float lightDistance = length(lightDirection);" << std::endl;
		ss << "lightDirection = normalize(lightDirection);" << std::endl;

		// Spot cone attenuation
		ss << "vec3 spotDir = normalize(mat3(viewMatrix) * light" << i << "SpotDir);" << std::endl;
		ss << "float spotDot = dot(-lightDirection, spotDir);" << std::endl;
		ss << "float spotAttn = (spotDot >= light" << i << "SpotCutoff) ? pow(spotDot, light" << i << "SpotExp) : 0.0;" << std::endl;

		// Distance attenuation
		ss << "float attenuation = light" << i << "ConstAttn + ";
		ss << "light" << i << "LinAttn * lightDistance +";
		ss << "light" << i << "QuadAttn * lightDistance * lightDistance;" << std::endl;

		ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
		ss << "normal3 = normalMatrix * normal3;" << std::endl;
		ss << "normal3 = normalize(normal3);" << std::endl;
		ss << "vec3 eyeDir = normalize(-ecPos3);" << std::endl;

		ss << "float invattn = spotAttn / attenuation;" << std::endl;
		ss << "float di = getIntensity" << i << "(normal3, lightDirection);" << std::endl;
		ss << "lightDiffuse = lightDiffuse + di * invattn * light" << i << "ColDiff;" << std::endl;
		// GL1.x LIT: no specular when surface faces away from light
		ss << "lightSpecular = lightSpecular + (di > 0.0 ? getSpecIntensity" << i << "(normal3, lightDirection, eyeDir) : 0.0) * invattn * light" << i << "ColSpec;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;
		break;
	}
}

void vpGenerate(std::string &result, const CVPBuiltin &desc)
{
	// Lighting requires normals in VB — fall back to unlit if missing
	bool lighting = desc.Lighting && hasFlag(desc.VertexFormat, g_VertexFlags[Normal]);

	// When PPL is active, vertexColor carries the vertex color (or vec4(1) identity).
	// PP uses: diffuseColor + pplDiff * vertexColor.
	bool hasPPL = desc.NumPerPixelLights > 0;
	bool splitVertexColor = lighting && desc.VertexColorLighted
		&& hasFlag(desc.VertexFormat, g_VertexFlags[PrimaryColor]);

	std::stringstream ss;
	ss << "// Builtin Vertex Shader" << std::endl;
	ss << std::endl;
	ss << "#version 330" << std::endl;
	ss << "#extension GL_ARB_separate_shader_objects : enable" << std::endl;
	ss << std::endl;

	// Count highest enabled clip plane for gl_ClipDistance array size
	int maxClipPlane = -1;
	if (desc.ClipPlaneMask)
		for (int i = 5; i >= 0; --i)
			if (desc.ClipPlaneMask & (1 << i)) { maxClipPlane = i; break; }

	ss << "out gl_PerVertex" << std::endl;
	ss << "{" << std::endl;
	ss << "vec4 gl_Position;" << std::endl;
	if (maxClipPlane >= 0)
		ss << "float gl_ClipDistance[" << (maxClipPlane + 1) << "];" << std::endl;
	ss << "};" << std::endl;
	ss << std::endl;
	ss << "uniform mat4 modelViewProjection;" << std::endl;
	ss << std::endl;

	for (int i = Position; i < NumOffsets; ++i)
		if (hasFlag(desc.VertexFormat, g_VertexFlags[i]))
			ss << "layout (location = " << i << ") " << "in vec4 " << "v" << g_AttribNames[i] << ";" << std::endl;
	ss << std::endl;

	for (int i = Weight; i < NumOffsets; ++i)
		if (hasFlag(desc.VertexFormat, g_VertexFlags[i]) && (i != PrimaryColor) && (i != SecondaryColor))
		{
			// Skip texcoord output when texgen overrides that stage
			if (i >= TexCoord0 && i <= TexCoord7 && desc.TexGenMode[i - TexCoord0] >= 0)
				continue;
			ss << "layout(location = " << i << ") smooth out vec4 " << g_AttribNames[i] << "; // vertex buffer" << std::endl;
		}
	ss << std::endl;

	bool needTexGen = false;
	bool needEyeLinear = false;
	bool needReflection = false;
	bool needSpecularTexMtx = false;
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		if (desc.TexGenMode[i] >= 0)
		{
			ss << "layout(location = " << (TexCoord0 + i) << ") smooth out vec4 texCoord" << i << "; // texgen" << std::endl;
			needTexGen = true;
			if (desc.TexGenMode[i] == TexGenObjectLinear || desc.TexGenMode[i] == TexGenEyeLinear
				|| desc.TexGenMode[i] == TexGenSphereMap)
				ss << "uniform mat4 texMatrix" << i << ";" << std::endl;
			if (desc.TexGenMode[i] == TexGenEyeLinear)
				needEyeLinear = true;
			if (desc.TexGenMode[i] == TexGenReflectionMap)
				needSpecularTexMtx = true;
			if (desc.TexGenMode[i] == TexGenReflectionMap || desc.TexGenMode[i] == TexGenSphereMap)
				needReflection = true;
		}
		else if (hasFlag(desc.VertexFormat, g_VertexFlags[TexCoord0 + i]))
		{
			// VB texcoord stage without texgen: declare texMatrix for user texture matrix support
			ss << "uniform mat4 texMatrix" << i << ";" << std::endl;
		}
	}
	if (needSpecularTexMtx)
		ss << "uniform mat4 specularTexMtx;" << std::endl;
	ss << std::endl;

	// Clip plane uniforms
	bool needClipPlanes = (desc.ClipPlaneMask != 0);
	if (needClipPlanes)
		for (int i = 0; i <= maxClipPlane; ++i)
			if (desc.ClipPlaneMask & (1 << i))
				ss << "uniform vec4 clipPlane" << i << ";" << std::endl;

	// Ambient color of all lights is precalculated and added with self illumination, and multiplied with the material ambient.
	if (lighting)
		ss << "uniform vec4 selfIllumination;" << std::endl;

	bool normalVarying = desc.WorldSpaceNormal && hasFlag(desc.VertexFormat, g_VertexFlags[Normal]);
	bool needPositionOutput = desc.Fog || desc.WorldSpacePosition || desc.PPClipPlane;
	bool needEcPos = needPositionOutput || lighting || needEyeLinear || needReflection || needClipPlanes;
	bool needNormalMatrix = lighting || needReflection || normalVarying;
	if (needEcPos)
		ss << "uniform mat4 modelView;" << std::endl;
	if (lighting || normalVarying || desc.WorldSpacePosition)
		ss << "uniform mat4 viewMatrix;" << std::endl;
	if (needEcPos)
		ss << "vec4 ecPos4;" << std::endl;
	if (needNormalMatrix && !lighting) // lighting block declares it separately (with light uniforms)
		ss << "uniform mat3 normalMatrix;" << std::endl;
	if (needPositionOutput)
		ss << "layout(location = " << VaryingLocationEcPos << ") smooth out vec4 ecPos;" << std::endl;
	if (desc.WorldSpacePosition)
		ss << "layout(location = " << VaryingLocationWorldPos << ") smooth out vec4 worldPos;" << std::endl;
	ss << std::endl;

	if (!lighting)
		ss << "uniform vec4 materialColor;" << std::endl; // Verify

	bool specularVertex = lighting || (desc.VertexFormat & g_VertexFlags[SecondaryColor]);
	if (hasPPL)
		ss << "layout(location = " << VaryingLocationVertexColor << ") smooth out vec4 vertexColor;" << std::endl;
	ss << "layout(location = " << VaryingLocationDiffuseColor << ") smooth out vec4 diffuseColor;" << std::endl;
	if (specularVertex)
		ss << "layout(location = " << VaryingLocationSpecularColor << ") smooth out vec4 specularColor;" << std::endl;
	ss << std::endl;

	if (lighting)
	{
		ss << "uniform mat3 normalMatrix;" << std::endl;
		// VP lights are shifted: driver slot [NumPerPixelLights..7] → VP slot [0..7-N]
		for (int vpSlot = 0, drvSlot = desc.NumPerPixelLights; drvSlot < NL_OPENGL3_MAX_LIGHT; ++vpSlot, ++drvSlot)
			vpLightUniforms(ss, desc.LightMode[drvSlot], vpSlot);

		for (int vpSlot = 0, drvSlot = desc.NumPerPixelLights; drvSlot < NL_OPENGL3_MAX_LIGHT; ++vpSlot, ++drvSlot)
			vpLightFunctions(ss, desc.LightMode[drvSlot], vpSlot);
		ss << std::endl;
	}

	ss << "void main(void)" << std::endl;
	ss << "{" << std::endl;
	ss << "gl_Position = modelViewProjection * " << "v" << g_AttribNames[0] << ";" << std::endl;
	ss << std::endl;

	if (needEcPos)
		ss << "ecPos4 = modelView * v" << g_AttribNames[0] << ";" << std::endl;
	if (needPositionOutput)
		ss << "ecPos = ecPos4;" << std::endl;
	if (desc.WorldSpacePosition)
		ss << "worldPos = vec4(transpose(mat3(viewMatrix)) * (ecPos4.xyz - viewMatrix[3].xyz * ecPos4.w), ecPos4.w);" << std::endl;
	ss << std::endl;

	ss << "vec4 diffuseVertex;" << std::endl;
	if (specularVertex) ss << "vec4 specularVertex;" << std::endl;
	ss << std::endl;

	if (lighting)
	{
		// Calculate lights
		ss << "diffuseVertex = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
		ss << "specularVertex = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
		ss << "vec4 diffuseLight;" << std::endl;
		ss << "vec4 specularLight;" << std::endl;
		for (int vpSlot = 0, drvSlot = desc.NumPerPixelLights; drvSlot < NL_OPENGL3_MAX_LIGHT; ++vpSlot, ++drvSlot)
			if (desc.LightMode[drvSlot] == CLight::DirectionalLight || desc.LightMode[drvSlot] == CLight::PointLight || desc.LightMode[drvSlot] == CLight::SpotLight)
				ss << "addLight" << vpSlot << "Color(diffuseVertex, specularVertex);" << std::endl;
		ss << "diffuseVertex.a = 1.0;" << std::endl;
		ss << "specularVertex.a = 1.0;" << std::endl;

		// Secondary color (lighted)
		if (desc.VertexFormat & g_VertexFlags[SecondaryColor])
		{
			ss << "specularVertex = specularVertex * vsecondaryColor;" << std::endl;
		}

		// Add selfIllumination before vertex color multiply so it's also modulated
		// by vertex color (GL_COLOR_MATERIAL AMBIENT_AND_DIFFUSE behavior)
		ss << "diffuseVertex.rgb = diffuseVertex.rgb + selfIllumination.rgb;" << std::endl;
	}
	else
	{
		// Unlit
		ss << "diffuseVertex = materialColor;" << std::endl;

		// Secondary color (unlit)
		if (desc.VertexFormat & g_VertexFlags[SecondaryColor])
		{
			nlwarning("VP: Secondary color in vertex buffer using material without lighting");
			ss << "specularVertex = vsecondaryColor;" << std::endl;
		}
	}

	// Primary color
	if (desc.VertexFormat & g_VertexFlags[PrimaryColor])
	{
		if (lighting && desc.VertexColorLighted)
		{
			// GL_COLOR_MATERIAL behavior: vertex color replaces material diffuse
			// CPU doesn't pre-multiply by matDiffuse when VertexColorLighted is set
			ss << "diffuseVertex = diffuseVertex * vprimaryColor;" << std::endl;
		}
		else if (!lighting)
		{
			// Unlit: vertex color replaces materialColor (fixed-function GL behavior)
			ss << "diffuseVertex = vprimaryColor;" << std::endl;
		}
		// When lighting && !VertexColorLighted: vprimaryColor is ignored (matDiffuse pre-multiplied on CPU)
	}

	// Combine diffuse (clamp before texture), specular passed separately (added post-texture in PP)
	ss << "diffuseColor = clamp(diffuseVertex, 0.0, 1.0);" << std::endl;

	// Pass vertex color to PP for PPL term multiplication
	if (hasPPL)
	{
		if (splitVertexColor)
			ss << "vertexColor = vprimaryColor;" << std::endl;
		else
			ss << "vertexColor = vec4(1.0);" << std::endl;
	}
	if (specularVertex)
		ss << "specularColor = clamp(vec4(specularVertex.rgb * specularVertex.a, 0.0), 0.0, 1.0);" << std::endl;
	ss << std::endl;

	for (int i = Weight; i < NumOffsets; i++)
	{
		if (hasFlag(desc.VertexFormat, g_VertexFlags[i]) && (i != PrimaryColor) && (i != SecondaryColor))
		{
			// Skip texcoord passthrough when texgen overrides that stage
			if (i >= TexCoord0 && i <= TexCoord7 && desc.TexGenMode[i - TexCoord0] >= 0)
				continue;
			if (i == Normal && normalVarying)
			{
				// World-space normal: eye-space via normalMatrix, then undo view rotation
				ss << "{" << std::endl;
				ss << "vec3 eyeN = normalize(normalMatrix * (v" << g_AttribNames[Normal] << ".xyz / v" << g_AttribNames[Normal] << ".w));" << std::endl;
				ss << g_AttribNames[i] << " = vec4(transpose(mat3(viewMatrix)) * eyeN, 0.0);" << std::endl;
				ss << "}" << std::endl;
			}
			else if (i == Normal && desc.Normalize)
				ss << g_AttribNames[i] << " = vec4(normalize(v" << g_AttribNames[i] << ".xyz), 0.0);" << std::endl;
			else if (i >= TexCoord0 && i <= TexCoord3)
				ss << g_AttribNames[i] << " = texMatrix" << (i - TexCoord0) << " * v" << g_AttribNames[TexCoord0 + desc.UVRouting[i - TexCoord0]] << ";" << std::endl;
			else
				ss << g_AttribNames[i] << " = " << "v" << g_AttribNames[i] << ";" << std::endl;
		}
	}

	// Compute eye-space reflection vector (shared by all reflection/sphere stages)
	if (needReflection)
	{
		if (hasFlag(desc.VertexFormat, g_VertexFlags[Normal]))
		{
			ss << "vec3 refl_n = normalize(normalMatrix * (v" << g_AttribNames[Normal] << ".xyz / v" << g_AttribNames[Normal] << ".w));" << std::endl;
			ss << "vec3 refl_u = normalize(ecPos4.xyz);" << std::endl; // incident: eye to vertex
			ss << "vec3 refl_r = reflect(refl_u, refl_n);" << std::endl;
		}
		else
		{
			ss << "vec3 refl_r = vec3(0.0, 0.0, -1.0);" << std::endl; // fallback when no normals
		}
		ss << std::endl;
	}

	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		if (desc.TexGenMode[i] == TexGenObjectLinear)
		{
			// Object-linear: texCoord = texMatrix * objectPosition (identity object planes)
			ss << "texCoord" << i << " = texMatrix" << i << " * v" << g_AttribNames[0] << ";" << std::endl;
		}
		else if (desc.TexGenMode[i] == TexGenEyeLinear)
		{
			// Eye-linear: texCoord = texMatrix * eyePosition (identity eye planes)
			ss << "texCoord" << i << " = texMatrix" << i << " * ecPos4;" << std::endl;
		}
		else if (desc.TexGenMode[i] == TexGenReflectionMap)
		{
			// Reflection map (cubemap): eye-space reflection vector, transformed by specularTexMtx (inverse view rotation)
			ss << "texCoord" << i << " = specularTexMtx * vec4(refl_r, 0.0);" << std::endl;
		}
		else if (desc.TexGenMode[i] == TexGenSphereMap)
		{
			// Sphere map (2D): OpenGL sphere mapping formula from the reflection vector
			ss << "{" << std::endl;
			ss << "float refl_m = 2.0 * sqrt(refl_r.x * refl_r.x + refl_r.y * refl_r.y + (refl_r.z + 1.0) * (refl_r.z + 1.0));" << std::endl;
			ss << "texCoord" << i << " = texMatrix" << i << " * vec4(refl_r.x / refl_m + 0.5, refl_r.y / refl_m + 0.5, 0.0, 1.0);" << std::endl;
			ss << "}" << std::endl;
		}
	}

	// Clip plane distance computation
	if (needClipPlanes)
	{
		for (int i = 0; i <= maxClipPlane; ++i)
		{
			if (desc.ClipPlaneMask & (1 << i))
				ss << "gl_ClipDistance[" << i << "] = dot(clipPlane" << i << ", ecPos4);" << std::endl;
			else
				ss << "gl_ClipDistance[" << i << "] = 1.0;" << std::endl;
		}
	}

	ss << "}" << std::endl;
	result = ss.str();
}

} /* anonymous namespace */

void CDriverGL3::generateBuiltinVertexProgram()
{
	CHashSet<CVPBuiltin, CVPBuiltinHashTraits>::iterator it = m_VPBuiltinCache.find(m_VPBuiltinCurrent);
	if (it != m_VPBuiltinCache.end())
	{
		m_VPBuiltinCurrent.VertexProgram = it->VertexProgram;
		return;
	}

	std::string result;
	vpGenerate(result, m_VPBuiltinCurrent);

	CVertexProgram *vertexProgram = new CVertexProgram();
	IProgram::CSource *src = new IProgram::CSource();
	src->Profile = IProgram::glsl330v;
	src->DisplayName = "Builtin Vertex Program (" + NLMISC::toString(m_VPBuiltinCache.size()) + ")";
	src->setSource(result);
	vertexProgram->addSource(src);

	nldebug("GL3: Generate '%s'", src->DisplayName.c_str());

	if (!compileVertexProgram(vertexProgram))
	{
		nlwarning("GL3: Builtin VP compilation failed (fmt=0x%x, lit=%d, fog=%d, vcl=%d)",
			m_VPBuiltinCurrent.VertexFormat, (int)m_VPBuiltinCurrent.Lighting,
			(int)m_VPBuiltinCurrent.Fog, (int)m_VPBuiltinCurrent.VertexColorLighted);
		delete vertexProgram; vertexProgram = NULL;
	}

	m_VPBuiltinCurrent.VertexProgram = vertexProgram;
	m_VPBuiltinCache.insert(m_VPBuiltinCurrent);
}

void CDriverGL3::enableFogVP(bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableFogInternal)
	if (m_VPBuiltinCurrent.Fog != enable)
	{
		m_VPBuiltinCurrent.Fog = enable;
		m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::enableLightingVP(bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableLightingVP)
	if (m_VPBuiltinCurrent.Lighting != enable)
	{
		m_VPBuiltinCurrent.Lighting = enable;
		m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::setVertexColorLightedVP(bool enable)
{
	H_AUTO_OGL(CDriverGL3_setVertexColorLightedVP)
	if (m_VPBuiltinCurrent.VertexColorLighted != enable)
	{
		m_VPBuiltinCurrent.VertexColorLighted = enable;
		m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::touchLightVP(int i)
{
	H_AUTO_OGL(CDriverGL3_touchLightVP)
	sint mode = _LightEnable[i] ? _LightMode[i] : -1;
	if (m_VPBuiltinCurrent.LightMode[i] != mode)
	{
		m_VPBuiltinCurrent.LightMode[i] = mode;
		if (m_VPBuiltinCurrent.Lighting)
			m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::touchVertexFormatVP()
{
	H_AUTO_OGL(CDriverGL3_touchLightVP)
	uint16 format = _CurrentVertexBufferGL->VB->getVertexFormat();
	if (m_VPBuiltinCurrent.VertexFormat != format)
	{
		m_VPBuiltinCurrent.VertexFormat = format;
		m_VPBuiltinTouched = true;
	}
	const uint8 *uvRouting = _CurrentVertexBufferGL->VB->getUVRouting();
	for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		if (m_VPBuiltinCurrent.UVRouting[i] != uvRouting[i])
		{
			m_VPBuiltinCurrent.UVRouting[i] = uvRouting[i];
			m_VPBuiltinTouched = true;
		}
	}
}

void CDriverGL3::setTexGenModeVP(uint stage, sint mode)
{
	H_AUTO_OGL(CDriverGL3_setTexGenModeVP)
	if (m_VPBuiltinCurrent.TexGenMode[stage] != mode)
	{
		//if (mode >= 0)
		//	nlwarning("enable texgen %i, %i, not implemented", stage, mode);
		m_VPBuiltinCurrent.TexGenMode[stage] = mode;
		m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::touchClipPlaneVP(uint index, bool enable)
{
	H_AUTO_OGL(CDriverGL3_touchClipPlaneVP)

	// Compute true mask from _ClipPlaneEnabled[]
	uint8 realMask = 0;
	for (uint i = 0; i < MaxClipPlanes; ++i)
		if (_ClipPlaneEnabled[i]) realMask |= (1 << i);

	// When PP clip planes are active, VP doesn't write gl_ClipDistance.
	// Zero ClipPlaneMask so the VP skips clip distance code,
	// and set PPClipPlane to trigger ecPos output instead.
	uint8 vpMask = m_PPClipPlanes ? 0 : realMask;
	bool ppClip = m_PPClipPlanes && (realMask != 0);

	if (m_VPBuiltinCurrent.ClipPlaneMask != vpMask || m_VPBuiltinCurrent.PPClipPlane != ppClip)
	{
		m_VPBuiltinCurrent.ClipPlaneMask = vpMask;
		m_VPBuiltinCurrent.PPClipPlane = ppClip;
		m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::setWorldSpaceNormalVP(bool enable)
{
	H_AUTO_OGL(CDriverGL3_setWorldSpaceNormalVP)
	if (m_VPBuiltinCurrent.WorldSpaceNormal != enable)
	{
		m_VPBuiltinCurrent.WorldSpaceNormal = enable;
		m_VPBuiltinTouched = true;
	}
}

void CDriverGL3::setWorldSpacePositionVP(bool enable)
{
	H_AUTO_OGL(CDriverGL3_setWorldSpacePositionVP)
	if (m_VPBuiltinCurrent.WorldSpacePosition != enable)
	{
		m_VPBuiltinCurrent.WorldSpacePosition = enable;
		m_VPBuiltinTouched = true;
	}
}

} // NLDRIVERGL3
} // NL3D

