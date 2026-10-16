// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2017  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// Define to enable hand-written UBO-based GLSL wind tree VP (GL3-only).
// When disabled, GL3 uses the nelvp source auto-converted to GLSL.
#define NL_WINDTREE_VP_UBO

#include "std3d.h"

#include "nel/3d/meshvp_wind_tree.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include <cmath>
#include "nel/misc/common.h"
#include "nel/3d/render_trav.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// Light VP fragment constants start at 24
static const uint	VPLightConstantStart = 24;


// ***************************************************************************
NLMISC::CSmartPtr<CVertexProgramWindTree> CMeshVPWindTree::_VertexProgram[CMeshVPWindTree::NumVp];
NLMISC::CSmartPtr<CVertexProgramWindTreeUBO> CMeshVPWindTree::_VertexProgramUBO;
NLMISC::CSmartPtr<CUniformBuffer> CMeshVPWindTree::_WindTreeUB;
CMeshVPWindTree::CWindTreeUBOOffsets CMeshVPWindTree::_UBOOffsets;
NLMISC::CSmartPtr<CUniformBufferFormat> CMeshVPWindTree::_WindTreeUBFormat;

// Precompiled Cg output for arbvp1 and vs_2_0 profiles
#include "shaders/wind_tree_vp_embedded.h"

// GLSL 330 source for GL3 driver (single source, #defines prepended at runtime)
static const char* WindTreeVPCodeGLSL_Header =
	"#version 330\n"
	"#extension GL_ARB_separate_shader_objects : enable\n";

// GLSL 300 es header for pipeline stage (linked program path)
static const char* WindTreeVPCodeGLSL_ES_Header =
	"#version 300 es\n"
	"precision highp float;\n"
	"precision highp int;\n";

static const char* WindTreeVPCodeGLSL_Body =
	"#ifndef NUM_POINT_LIGHTS\n"
	"#define NUM_POINT_LIGHTS 0\n"
	"#endif\n"
	"layout (location = 0) in vec4 vposition;\n"
	"layout (location = 2) in vec4 vnormal;\n"
	"layout (location = 3) in vec4 vprimaryColor;\n"
	"layout (location = 8) in vec4 vtexCoord0;\n"
	"out gl_PerVertex { vec4 gl_Position; };\n"
	"layout(location = 3) smooth out vec4 diffuseColor;\n"
	"#ifdef USE_SPECULAR\n"
	"layout(location = 4) smooth out vec4 specularColor;\n"
	"#endif\n"
	"layout(location = 8) smooth out vec4 texCoord0;\n"
	"layout(location = 0) smooth out vec4 ecPos;\n"
	"uniform mat4 modelViewProjection;\n"
	"uniform mat4 modelView;\n"
	"uniform vec3 windLevel1;\n"
	"uniform vec3 windLevel2[4];\n"
	"uniform vec3 windLevel3[4];\n"
	"uniform vec4 ambient;\n"
	"uniform vec4 diffuse0;\n"
	"#if NUM_POINT_LIGHTS >= 1\n"
	"uniform vec4 diffuse1;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 2\n"
	"uniform vec4 diffuse2;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 3\n"
	"uniform vec4 diffuse3;\n"
	"#endif\n"
	"uniform vec4 diffuseAlpha;\n"
	"#ifdef USE_SPECULAR\n"
	"uniform vec4 specular0;\n"
	"#if NUM_POINT_LIGHTS >= 1\n"
	"uniform vec4 specular1;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 2\n"
	"uniform vec4 specular2;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 3\n"
	"uniform vec4 specular3;\n"
	"#endif\n"
	"uniform vec3 sunDir;\n"
	"uniform vec3 eyePos;\n"
	"#if NUM_POINT_LIGHTS >= 1\n"
	"uniform vec3 plPos0;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 2\n"
	"uniform vec3 plPos1;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 3\n"
	"uniform vec3 plPos2;\n"
	"#endif\n"
	"#else\n"
	"uniform vec3 dirOrPos0;\n"
	"#if NUM_POINT_LIGHTS >= 1\n"
	"uniform vec3 dirOrPos1;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 2\n"
	"uniform vec3 dirOrPos2;\n"
	"#endif\n"
	"#if NUM_POINT_LIGHTS >= 3\n"
	"uniform vec3 dirOrPos3;\n"
	"#endif\n"
	"#endif\n"
	"void main()\n"
	"{\n"
	"  vec3 factors = clamp(vprimaryColor.xxx * 3.0 + vec3(0.0, -1.0, -2.0), 0.0, 1.0);\n"
	"  vec4 pos = vposition;\n"
	"  pos.xyz += windLevel1 * factors.x;\n"
	"  vec2 phase = vprimaryColor.yz * 3.99;\n"
	"  int idx2 = int(phase.x);\n"
	"  pos.xyz += windLevel2[idx2] * factors.y;\n"
	"  int idx3 = int(phase.y);\n"
	"  pos.xyz += windLevel3[idx3] * factors.z;\n"
	"  vec3 N = vnormal.xyz;\n"
	"#ifdef USE_NORMALIZE\n"
	"  N = normalize(N);\n"
	"#endif\n"
	"  vec4 litColor = ambient;\n"
	"#ifdef USE_SPECULAR\n"
	"  float shininess = specular0.w;\n"
	"  vec3 V = normalize(eyePos - pos.xyz);\n"
	"  vec3 H = normalize(sunDir + V);\n"
	"  float NdotL = max(dot(N, sunDir), 0.0);\n"
	"  float NdotH = max(dot(N, H), 0.0);\n"
	"  float specPow = NdotL > 0.0 ? pow(NdotH, shininess) : 0.0;\n"
	"  litColor += NdotL * diffuse0;\n"
	"  vec3 specAccum = specPow * specular0.xyz;\n"
	"  #if NUM_POINT_LIGHTS >= 1\n"
	"  {\n"
	"    vec3 L = normalize(plPos0 - pos.xyz);\n"
	"    H = normalize(L + V);\n"
	"    NdotL = max(dot(N, L), 0.0);\n"
	"    NdotH = max(dot(N, H), 0.0);\n"
	"    specPow = NdotL > 0.0 ? pow(NdotH, shininess) : 0.0;\n"
	"    litColor += NdotL * diffuse1;\n"
	"    specAccum += specPow * specular1.xyz;\n"
	"  }\n"
	"  #endif\n"
	"  #if NUM_POINT_LIGHTS >= 2\n"
	"  {\n"
	"    vec3 L = normalize(plPos1 - pos.xyz);\n"
	"    H = normalize(L + V);\n"
	"    NdotL = max(dot(N, L), 0.0);\n"
	"    NdotH = max(dot(N, H), 0.0);\n"
	"    specPow = NdotL > 0.0 ? pow(NdotH, shininess) : 0.0;\n"
	"    litColor += NdotL * diffuse2;\n"
	"    specAccum += specPow * specular2.xyz;\n"
	"  }\n"
	"  #endif\n"
	"  #if NUM_POINT_LIGHTS >= 3\n"
	"  {\n"
	"    vec3 L = normalize(plPos2 - pos.xyz);\n"
	"    H = normalize(L + V);\n"
	"    NdotL = max(dot(N, L), 0.0);\n"
	"    NdotH = max(dot(N, H), 0.0);\n"
	"    specPow = NdotL > 0.0 ? pow(NdotH, shininess) : 0.0;\n"
	"    litColor += NdotL * diffuse3;\n"
	"    specAccum += specPow * specular3.xyz;\n"
	"  }\n"
	"  #endif\n"
	"  diffuseColor = clamp(litColor * diffuseAlpha.zzzx + diffuseAlpha.xxxw, 0.0, 1.0);\n"
	"  specularColor = clamp(vec4(specAccum, 0.0), 0.0, 1.0);\n"
	"#else\n"
	"  litColor += max(dot(N, dirOrPos0), 0.0) * diffuse0;\n"
	"  #if NUM_POINT_LIGHTS >= 1\n"
	"  litColor += max(dot(N, normalize(dirOrPos1 - pos.xyz)), 0.0) * diffuse1;\n"
	"  #endif\n"
	"  #if NUM_POINT_LIGHTS >= 2\n"
	"  litColor += max(dot(N, normalize(dirOrPos2 - pos.xyz)), 0.0) * diffuse2;\n"
	"  #endif\n"
	"  #if NUM_POINT_LIGHTS >= 3\n"
	"  litColor += max(dot(N, normalize(dirOrPos3 - pos.xyz)), 0.0) * diffuse3;\n"
	"  #endif\n"
	"  diffuseColor = clamp(litColor * diffuseAlpha.zzzx + diffuseAlpha.xxxw, 0.0, 1.0);\n"
	"#endif\n"
	"  gl_Position = modelViewProjection * pos;\n"
	"  texCoord0 = vtexCoord0;\n"
	"  ecPos = modelView * pos;\n"
	"}\n";

// UBO-based GLSL body: uses NlCamera, NlLightTable, NlModel, NlWindTree UBOs.
// All 16 variants (light count, specular, normalize) folded into one shader.
// gl_PerVertex output block (SSO-only; 300 es has implicit gl_Position)
static const char* WindTreeVPCodeGLSL_UBO_PerVertex =
	"out gl_PerVertex { vec4 gl_Position; };\n";

// Output varying declarations — layout qualifiers required for SSO, not allowed for linked
static const char* WindTreeVPCodeGLSL_UBO_VaryingsSSO =
	"layout(location = 3) smooth out vec4 diffuseColor;\n"
	"layout(location = 4) smooth out vec4 specularColor;\n"
	"layout(location = 8) smooth out vec4 texCoord0;\n"
	"layout(location = 0) smooth out vec4 ecPos;\n";

static const char* WindTreeVPCodeGLSL_UBO_VaryingsLinked =
	"smooth out vec4 diffuseColor;\n"
	"smooth out vec4 specularColor;\n"
	"smooth out vec4 texCoord0;\n"
	"smooth out vec4 ecPos;\n";

static const char* WindTreeVPCodeGLSL_UBO_Body =
	"layout (location = 0) in vec4 vposition;\n"
	"layout (location = 2) in vec4 vnormal;\n"
	"layout (location = 3) in vec4 vprimaryColor;\n"
	"layout (location = 8) in vec4 vtexCoord0;\n"
	"\n"
	"// Wind and material uniforms provided by NlWindTree UBO (binding 4)\n"
	"// Declared via CSource::UniformBufferFormats, generated by insertBuiltinHeaders.\n"
	"\n"
	"void computeLight(int lightMode, vec3 dirOrPos, vec4 colDiff, vec4 colSpec, float shininess,\n"
	"                   float constAttn, float linAttn, float quadAttn,\n"
	"                   vec3 spotDir, float spotCutoff, float spotExp,\n"
	"                   vec3 normal3, vec3 ecPos3, vec3 eyeDir,\n"
	"                   inout vec4 lightDiffuse, inout vec4 lightSpecular)\n"
	"{\n"
	"  if (lightMode < 0) return;\n"
	"\n"
	"  vec3 lightDir;\n"
	"  float attnFactor = 1.0;\n"
	"\n"
	"  if (lightMode == 0) {\n"  // DirectionalLight
	"    lightDir = normalize(mat3(viewMatrix) * dirOrPos);\n"
	"  } else {\n"
	"    vec4 lightPos4 = viewMatrix * vec4(dirOrPos, 1.0);\n"
	"    vec3 lightPos = lightPos4.xyz / lightPos4.w;\n"
	"    vec3 lightVec = lightPos - ecPos3;\n"
	"    float lightDistance = length(lightVec);\n"
	"    lightDir = lightVec / lightDistance;\n"
	"    float attenuation = constAttn + linAttn * lightDistance + quadAttn * lightDistance * lightDistance;\n"
	"    attnFactor = 1.0 / attenuation;\n"
	"    if (lightMode == 2) {\n"  // SpotLight
	"      vec3 spotDirView = normalize(mat3(viewMatrix) * spotDir);\n"
	"      float spotDot = dot(-lightDir, spotDirView);\n"
	"      attnFactor *= (spotDot >= spotCutoff) ? pow(spotDot, spotExp) : 0.0;\n"
	"    }\n"
	"  }\n"
	"\n"
	"  float diffAngle = max(0.0, dot(lightDir, normal3));\n"
	"  lightDiffuse += diffAngle * attnFactor * colDiff;\n"
	"\n"
	"  vec3 halfVector = normalize(lightDir + eyeDir);\n"
	"  float specAngle = max(0.0, dot(normal3, halfVector));\n"
	"  float specPow = pow(specAngle, shininess);\n"
	"  lightSpecular += specPow * attnFactor * colSpec;\n"
	"}\n"
	"\n"
	"void main()\n"
	"{\n"
	"  // --- Wind Animation (object space) ---\n"
	"  vec3 factors = clamp(vprimaryColor.xxx * 3.0 + vec3(0.0, -1.0, -2.0), 0.0, 1.0);\n"
	"  vec4 pos = vposition;\n"
	"  pos.xyz += windLevel1 * factors.x;\n"
	"  vec2 phase = vprimaryColor.yz * 3.99;\n"
	"  int idx2 = int(phase.x);\n"
	"  pos.xyz += windLevel2[idx2] * factors.y;\n"
	"  int idx3 = int(phase.y);\n"
	"  pos.xyz += windLevel3[idx3] * factors.z;\n"
	"\n"
	"  // --- Transform ---\n"
	"  gl_Position = modelViewProjection * pos;\n"
	"  vec4 ecPos4 = modelView * pos;\n"
	"  ecPos = ecPos4;\n"
	"\n"
	"  // --- Lighting (eye space, light table) ---\n"
	"  vec3 normal3 = normalize(normalMatrix * vnormal.xyz);\n"
	"  vec3 ecPos3 = ecPos4.xyz / ecPos4.w;\n"
	"  vec3 eyeDir = normalize(-ecPos3);\n"
	"  vec4 diffuseVertex = vec4(0.0);\n"
	"  vec4 specularVertex = vec4(0.0);\n"
	"\n"
	"  // Unrolled 8 light slots from NlModel UBO\n"
	"  { int idx = nlLightIndices01.x;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.x;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices01.y;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.y;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices01.z;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.z;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices01.w;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.w;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices45.x;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.x;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices45.y;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.y;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices45.z;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.z;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"  { int idx = nlLightIndices45.w;\n"
	"    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.w;\n"
	"      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;\n"
	"      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,\n"
	"        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);\n"
	"  } }\n"
	"\n"
	"  // Self-illumination from NlModel UBO\n"
	"  diffuseVertex.rgb += selfIllumination.rgb;\n"
	"  diffuseVertex.a = 1.0;\n"
	"\n"
	"  diffuseColor = clamp(diffuseVertex, 0.0, 1.0);\n"
	"  specularColor = clamp(vec4(specularVertex.rgb, 0.0), 0.0, 1.0);\n"
	"  texCoord0 = vtexCoord0;\n"
	"}\n";

static const char*	WindTreeVPCodeWave=
"!!VP1.0																				\n\
  # extract from color.R the 3 factors into R0.xyz									\n\
	MAD	R0, v[3].x, c[9].x, c[9].yzww;	# col.R*3										\n\
	MIN	R0, R0, c[8].yyyy;				# clamp each to 0,1								\n\
	MAX	R0, R0, c[8].xxxx;																\n\
																						\n\
	# Add influence of Bone Level1														\n\
	MAD	R5, c[15], R0.x, v[0];															\n\
																						\n\
	# Sample LevelPhase into R7.yz: 0 to 3.												\n\
	MUL	R7, v[3].xyzw, c[10].x;															\n\
																						\n\
	# Add influence of Bone Level2														\n\
	ARL	A0.x, R7.y;																		\n\
	MAD	R5, c[A0.x+16], R0.y, R5;														\n\
																						\n\
	# Add influence of Bone Level3														\n\
	ARL	A0.x, R7.z;																		\n\
	MAD	R5, c[A0.x+20], R0.z, R5;														\n\
																						\n\
	# Get normal in R6 for lighting.													\n\
	MOV	R6, v[2];																		\n\
";

static const char*	WindTreeVPCodeEnd=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R5;															\n\
	DP4 o[HPOS].y, c[1], R5;															\n\
	DP4 o[HPOS].z, c[2], R5;															\n\
	DP4 o[HPOS].w, c[3], R5;															\n\
	MOV o[TEX0], v[8];																\n\
	# hulud : remove this line for the moment because it doesn't work under d3d, if it is needed, we will have to create 2 CVertexProgram objects.\n\
	#MOV o[TEX1], v[9];																\n\
	DP4	o[FOGC].x, c[6], R5;															\n\
	END																					\n\
";


// Shared index struct for wind tree VP uniforms
struct CWindTreeVPIdx
{
	uint ProgramConstants[3];
	uint WindLevel1;
	uint WindLevel2[4];
	uint WindLevel3[4];
	// Material (individual uniforms for legacy path, ~0 for UBO path)
	uint MaterialDiffuse;
	uint MaterialSpecular;
	uint MaterialShininess;
};

class CVertexProgramWindTree : public CVertexProgramLighted
{
public:
	typedef CWindTreeVPIdx CIdx;
	CVertexProgramWindTree(uint numPls, bool specular, bool normalize);
	virtual ~CVertexProgramWindTree() { };
	virtual void buildInfo();
	const CIdx &idx() const { return m_Idx; }

	bool PerMeshSetup;

private:
	CIdx m_Idx;

};

// UBO-based wind tree VP (extends CVertexProgram, not CVertexProgramLighted)
#ifdef NL_WINDTREE_VP_UBO
class CVertexProgramWindTreeUBO : public CVertexProgram
{
public:
	typedef CWindTreeVPIdx CIdx;
	CVertexProgramWindTreeUBO();
	virtual ~CVertexProgramWindTreeUBO() { };
	virtual void buildInfo();
	const CIdx &idx() const { return m_Idx; }

private:
	CIdx m_Idx;

};
#endif // NL_WINDTREE_VP_UBO

CVertexProgramWindTree::CVertexProgramWindTree(uint numPls, bool specular, bool normalize)
{
	// lighted settings
	m_FeaturesLighted.SupportSpecular = specular;
	m_FeaturesLighted.NumActivePointLights = numPls;
	m_FeaturesLighted.Normalize = normalize;
	m_FeaturesLighted.CtStartNeLVP = VPLightConstantStart;

	// constants cache
	PerMeshSetup = false;

	// Variant index: numPls * 4 + (specular ? 2 : 0) + (normalize ? 1 : 0)
	uint vpIdx = numPls * 4 + (specular ? 2 : 0) + (normalize ? 1 : 0);

#ifdef NL_WINDTREE_VP_GLSL
	// glsl330v source (for GL3 driver — single source with runtime #defines)
	{
		std::string defines;
		defines += "#define NUM_POINT_LIGHTS ";
		defines += NLMISC::toString(numPls);
		defines += "\n";
		if (specular) defines += "#define USE_SPECULAR\n";
		if (normalize) defines += "#define USE_NORMALIZE\n";

		CSource *source = new CSource();
		source->Features.OutputsSpecularColor = specular;
		source->DisplayName = NLMISC::toString("glsl330v/MeshVPWindTree/%i/%s/%s", numPls, specular ? "spec" : "nospec", normalize ? "normalize" : "nonormalize");
		source->Profile = CVertexProgram::glsl330v;
		source->setSource(std::string(WindTreeVPCodeGLSL_Header) + defines + WindTreeVPCodeGLSL_Body);
		addSource(source);
	}
#endif // NL_WINDTREE_VP_GLSL

#ifdef NL_WINDTREE_VP_ARB
	// arbvp1 source (preferred by GL ARB path)
	{
		CSource *source = new CSource();
		source->DisplayName = NLMISC::toString("arbvp1/MeshVPWindTree/%i/%s/%s", numPls, specular ? "spec" : "nospec", normalize ? "normalize" : "nonormalize");
		source->Profile = CVertexProgram::arbvp1;
		source->setSourcePtr(s_windTreeARBVP1[vpIdx]);
		source->ParamIndices["modelViewProjection"] = 0;
		source->ParamIndices["fog"] = 6;
		addSource(source);
	}

	// vs_2_0 source (preferred by D3D path)
	{
		CSource *source = new CSource();
		source->DisplayName = NLMISC::toString("vs_2_0/MeshVPWindTree/%i/%s/%s", numPls, specular ? "spec" : "nospec", normalize ? "normalize" : "nonormalize");
		source->Profile = CVertexProgram::vs_2_0;
		source->setSourcePtr(s_windTreeVS20[vpIdx]);
		source->ParamIndices["modelViewProjection"] = 0;
		source->ParamIndices["fog"] = 6;
		addSource(source);
	}
#endif // NL_WINDTREE_VP_ARB

	// nelvp source (fallback for NV VP / EXT vertex shader paths)
	{
		std::string vpCode = std::string(WindTreeVPCodeWave)
			+ CRenderTrav::getLightVPFragmentNeLVP(numPls, VPLightConstantStart, specular, normalize)
			+ WindTreeVPCodeEnd;

		CSource *source = new CSource();
		source->DisplayName = NLMISC::toString("nelvp/MeshVPWindTree/%i/%s/%s", numPls, specular ? "spec" : "nospec", normalize ? "normalize" : "nonormalize");
		source->Profile = CVertexProgram::nelvp;
		source->Features.NelvpRegisterCount = 40; // c[0..23] base + c[24..38] lighting (CTS+0..14)
		source->setSource(vpCode);
		source->ParamIndices["modelViewProjection"] = 0;
		source->ParamIndices["fog"] = 6;
		addSource(source);
	}
}

void CVertexProgramWindTree::buildInfo()
{
	CVertexProgramLighted::buildInfo();
	if (profile() == nelvp || profile() == arbvp1 || profile() == vs_2_0)
	{
		m_Idx.ProgramConstants[0] = 8;
		m_Idx.ProgramConstants[1] = 9;
		m_Idx.ProgramConstants[2] = 10;
		m_Idx.WindLevel1 = 15;
		m_Idx.WindLevel2[0] = 16;
		m_Idx.WindLevel2[1] = 17;
		m_Idx.WindLevel2[2] = 18;
		m_Idx.WindLevel2[3] = 19;
		m_Idx.WindLevel3[0] = 20;
		m_Idx.WindLevel3[1] = 21;
		m_Idx.WindLevel3[2] = 22;
		m_Idx.WindLevel3[3] = 23;
		// Assembly path uses pre-multiplied colors via changeVPLightSetupMaterial
		m_Idx.MaterialDiffuse = ~0;
		m_Idx.MaterialSpecular = ~0;
		m_Idx.MaterialShininess = ~0;
	}
	else
	{
		// GLSL: utility constants are literals, not uniforms
		m_Idx.ProgramConstants[0] = ~0;
		m_Idx.ProgramConstants[1] = ~0;
		m_Idx.ProgramConstants[2] = ~0;
		m_Idx.WindLevel1 = getUniformIndex("windLevel1");
		m_Idx.WindLevel2[0] = getUniformIndex("windLevel2[0]");
		m_Idx.WindLevel2[1] = getUniformIndex("windLevel2[1]");
		m_Idx.WindLevel2[2] = getUniformIndex("windLevel2[2]");
		m_Idx.WindLevel2[3] = getUniformIndex("windLevel2[3]");
		m_Idx.WindLevel3[0] = getUniformIndex("windLevel3[0]");
		m_Idx.WindLevel3[1] = getUniformIndex("windLevel3[1]");
		m_Idx.WindLevel3[2] = getUniformIndex("windLevel3[2]");
		m_Idx.WindLevel3[3] = getUniformIndex("windLevel3[3]");
		m_Idx.MaterialDiffuse = getUniformIndex("nlMaterialDiffuse");
		m_Idx.MaterialSpecular = getUniformIndex("nlMaterialSpecular");
		m_Idx.MaterialShininess = getUniformIndex("nlMaterialShininess");
	}
}

#ifdef NL_WINDTREE_VP_UBO
CVertexProgramWindTreeUBO::CVertexProgramWindTreeUBO()
{
	// Build UBO format (static, shared across all wind tree instances)
	if (!CMeshVPWindTree::_WindTreeUBFormat)
	{
		CUniformBufferFormat *fmt = new CUniformBufferFormat();
		fmt->Name = "NlWindTree";
		CMeshVPWindTree::_UBOOffsets.WindLevel1        = fmt->push("windLevel1", CUniformBufferFormat::FloatVec3);
		CMeshVPWindTree::_UBOOffsets.WindLevel2        = fmt->push("windLevel2", CUniformBufferFormat::FloatVec3, 4);
		CMeshVPWindTree::_UBOOffsets.WindLevel3        = fmt->push("windLevel3", CUniformBufferFormat::FloatVec3, 4);
		CMeshVPWindTree::_UBOOffsets.MaterialDiffuse   = fmt->push("nlMaterialDiffuse", CUniformBufferFormat::FloatVec4);
		CMeshVPWindTree::_UBOOffsets.MaterialSpecular  = fmt->push("nlMaterialSpecular", CUniformBufferFormat::FloatVec4);
		CMeshVPWindTree::_UBOOffsets.MaterialShininess = fmt->push("nlMaterialShininess", CUniformBufferFormat::Float);
		CMeshVPWindTree::_WindTreeUBFormat = fmt;
	}

	// Pipeline-stage UBO source (preferred for linked program path)
	{
		CSource *source = new CSource();
		source->Features.OutputsSpecularColor = true;
		source->Features.OnlyUBOs = true;
		source->Features.UsesLightTableUBO = true;
		source->Features.UsesCameraUBO = true;
		source->Features.UsesObjectUBO = true;
		source->UniformBufferFormats[UBBindingVertexProgram] = CMeshVPWindTree::_WindTreeUBFormat;
		source->DisplayName = "glsl300esv/MeshVPWindTree/UBO";
		source->Profile = CVertexProgram::glsl300esv;
		source->setSource(std::string(WindTreeVPCodeGLSL_ES_Header) + WindTreeVPCodeGLSL_UBO_VaryingsLinked + WindTreeVPCodeGLSL_UBO_Body);
		addSource(source);
	}

	// SSO UBO source (fallback for separate shader objects path)
	{
		CSource *source = new CSource();
		source->Features.OutputsSpecularColor = true;
		source->Features.OnlyUBOs = true;
		source->Features.UsesLightTableUBO = true;
		source->Features.UsesCameraUBO = true;
		source->Features.UsesObjectUBO = true;
		source->UniformBufferFormats[UBBindingVertexProgram] = CMeshVPWindTree::_WindTreeUBFormat;
		source->DisplayName = "glsl330v/MeshVPWindTree/UBO";
		source->Profile = CVertexProgram::glsl330v;
		source->setSource(std::string(WindTreeVPCodeGLSL_Header) + WindTreeVPCodeGLSL_UBO_PerVertex + WindTreeVPCodeGLSL_UBO_VaryingsSSO + WindTreeVPCodeGLSL_UBO_Body);
		addSource(source);
	}
}
#endif // NL_WINDTREE_VP_UBO

#ifdef NL_WINDTREE_VP_UBO
void CVertexProgramWindTreeUBO::buildInfo()
{
	// All uniforms now in NlWindTree UBO — no individual uniform indices needed
	m_Idx.ProgramConstants[0] = ~0;
	m_Idx.ProgramConstants[1] = ~0;
	m_Idx.ProgramConstants[2] = ~0;
	m_Idx.WindLevel1 = ~0;
	for (int i = 0; i < 4; ++i) m_Idx.WindLevel2[i] = ~0;
	for (int i = 0; i < 4; ++i) m_Idx.WindLevel3[i] = ~0;
	m_Idx.MaterialDiffuse = ~0;
	m_Idx.MaterialSpecular = ~0;
	m_Idx.MaterialShininess = ~0;
}
#endif // NL_WINDTREE_VP_UBO


// ***************************************************************************
float	CMeshVPWindTree::speedCos(float angle)
{
	// \todo yoyo TODO_OPTIM
	return cosf(angle * 2*(float)Pi);
}


// ***************************************************************************
CMeshVPWindTree::CMeshVPWindTree()
{
	for(uint i=0; i<HrcDepth; i++)
	{
		Frequency[i]= 1;
		FrequencyWindFactor[i]= 0;
		PowerXY[i]= 0;
		PowerZ[i]= 0;
		Bias[i]= 0;
		// Init currentTime.
		_CurrentTime[i]= 0;
	}
	SpecularLighting= false;

	_LastSceneTime= 0;
	_MaxVertexMove= 0;
}


// ***************************************************************************
CMeshVPWindTree::~CMeshVPWindTree()
{
}

// ***************************************************************************
const CWindTreeVPIdx &CMeshVPWindTree::activeIdx() const
{
	if (_ActiveVertexProgramUBO)
		return _ActiveVertexProgramUBO->idx();
	nlassert(_ActiveVertexProgram);
	return _ActiveVertexProgram->idx();
}

bool CMeshVPWindTree::isUBOActive() const
{
	return _ActiveVertexProgramUBO != NULL;
}


// ***************************************************************************
void	CMeshVPWindTree::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	nlassert(HrcDepth==3);
	for(uint i=0; i<HrcDepth; i++)
	{
		f.serial(Frequency[i]);
		f.serial(FrequencyWindFactor[i]);
		f.serial(PowerXY[i]);
		f.serial(PowerZ[i]);
		f.serial(Bias[i]);
	}
	f.serial(SpecularLighting);
}

void CMeshVPWindTree::initVertexPrograms()
{
	// init the vertexProgram code.
	static	bool	vpCreated= false;

	if(!vpCreated)
	{
		vpCreated= true;
		// All vpcode and begin() written for HrcDepth==3
		nlassert(HrcDepth==3);

		// For all possible VP.
		for(uint i=0;i<NumVp;i++)
		{
			// setup of the VPLight fragment
			uint	numPls= i/4;
			bool	normalize= (i&1)!=0;
			bool	specular= (i&2)!=0;

			// combine
			_VertexProgram[i] = new CVertexProgramWindTree(numPls, specular, normalize);
		}

#ifdef NL_WINDTREE_VP_UBO
		// Single UBO-based VP (all light/specular/normalize folded, GL3-only)
		_VertexProgramUBO = new CVertexProgramWindTreeUBO();

		// Shared UBO for wind + material data (set-and-discard per draw call)
		if (_WindTreeUBFormat)
		{
			_WindTreeUB = new CUniformBuffer();
			_WindTreeUB->Format = *_WindTreeUBFormat;
		}
#endif // NL_WINDTREE_VP_UBO
	}
}

// ***************************************************************************
void	CMeshVPWindTree::initInstance(CMeshBaseInstance *mbi)
{
	initVertexPrograms();

	// init a random phase.
	mbi->_VPWindTreePhase= frand(1);
}

// ***************************************************************************
inline void			CMeshVPWindTree::setupPerMesh(IDriver *driver, CScene *scene)
{
	// process current times and current power. Only one time per render() and per CMeshVPWindTree.
	if(scene->getCurrentTime() != _LastSceneTime)
	{
		// Get info from scene
		float	windPower= scene->getGlobalWindPower();

		float	dt= (float)(scene->getCurrentTime() - _LastSceneTime);
		_LastSceneTime= scene->getCurrentTime();

		// Update each boneLevel time according to frequency.
		uint i;
		for(i=0; i<HrcDepth; i++)
		{
			_CurrentTime[i]+= dt*(Frequency[i] + FrequencyWindFactor[i]*windPower);
			// get it between 0 and 1. Important for float precision problems.
			_CurrentTime[i]= (float)fmod(_CurrentTime[i], 1);
		}

		// Update each boneLevel maximum amplitude vector.
		for(i=0; i<HrcDepth; i++)
		{
			_MaxDeltaPos[i]= scene->getGlobalWindDirection() * PowerXY[i] * windPower;
			_MaxDeltaPos[i].z= PowerZ[i] * windPower;
		}

		/* Update the Max amplitude distance
			in world space, since maxdeltaPos are applied in world space, see setupPerInstanceConstants()
		*/
		_MaxVertexMove= 0;
		for(i=0; i<HrcDepth; i++)
		{
			_MaxVertexMove+= _MaxDeltaPos[i].norm();
		}
	}

	const CWindTreeVPIdx &vpIdx = activeIdx();

	// Setup common constants for each instances.
	// c[8] take useful constants.
	driver->setUniform4f(IDriver::VertexProgram, vpIdx.ProgramConstants[0],
		0, 1, 0.5f, 2);
	// c[9] take other useful constants.
	driver->setUniform4f(IDriver::VertexProgram, vpIdx.ProgramConstants[1],
		3.f, 0.f, -1.f, -2.f);
	// c[10] take Number of phase (4) for level2 and 3. -0.01 to avoid int value == 4.
	driver->setUniform4f(IDriver::VertexProgram, vpIdx.ProgramConstants[2],
		4-0.01f, 0, 0, 0);
}

// ***************************************************************************
inline	void		CMeshVPWindTree::setupPerInstanceConstants(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat)
{
	const CWindTreeVPIdx &vpIdx = activeIdx();

	// get instance info
	float	instancePhase= mbi->_VPWindTreePhase;


	// maxDeltaPos in ObjectSpace. So same world Wind direction is applied to all objects
	static	CMatrix		invWorldMatrix;
	// Keep only rotation part. (just need it and faster invert)
	invWorldMatrix.setRot(mbi->getWorldMatrix());
	invWorldMatrix.invert();
	static	CVector		maxDeltaPosOS[HrcDepth];
	for(uint i=0; i<HrcDepth; i++)
	{
		maxDeltaPosOS[i]= invWorldMatrix.mulVector(_MaxDeltaPos[i]);
	}


	if (!isUBOActive())
	{
		CVertexProgramWindTree *program = _ActiveVertexProgram;
		nlassert(program);

		// Legacy path: setup lighting, matrices, and fog as individual uniforms
		setupLighting(scene, mbi, invertedModelMat);

		// c[0..3] take the ModelViewProjection Matrix. After setupModelMatrix();
		driver->setUniformMatrix(IDriver::VertexProgram, program->getUniformIndex(CProgramIndex::ModelViewProjection),
			IDriver::ModelViewProjection, IDriver::Identity);
		// Fog: assembly profiles use dot(fog, pos), GLSL uses ecPos from modelView (no-op for ASM since index is ~0)
		driver->setUniformFog(IDriver::VertexProgram, program->getUniformIndex(CProgramIndex::Fog));
		driver->setUniformMatrix(IDriver::VertexProgram, program->getUniformIndex(CProgramIndex::ModelView),
			IDriver::ModelView, IDriver::Identity);
	}
	// UBO path: matrices, lighting, self-illumination, fog all handled by UBOs + setupUniforms()


	// Compute wind vectors
	float	f;

	// Level 0
	f= speedCos(_CurrentTime[0] + instancePhase) + Bias[0];
	CVector windL1 = maxDeltaPosOS[0] * f;

	// Level 1 (4 phases)
	float	instTime1= _CurrentTime[1] + instancePhase;
	CVector windL2[4];
	windL2[0] = maxDeltaPosOS[1] * (speedCos(instTime1 + 0.00f) + Bias[1]);
	windL2[1] = maxDeltaPosOS[1] * (speedCos(instTime1 + 0.25f) + Bias[1]);
	windL2[2] = maxDeltaPosOS[1] * (speedCos(instTime1 + 0.50f) + Bias[1]);
	windL2[3] = maxDeltaPosOS[1] * (speedCos(instTime1 + 0.75f) + Bias[1]);

	// Level 2 (4 phases)
	float	instTime2= _CurrentTime[2] + instancePhase;
	CVector windL3[4];
	windL3[0] = maxDeltaPosOS[2] * (speedCos(instTime2 + 0.00f) + Bias[2]);
	windL3[1] = maxDeltaPosOS[2] * (speedCos(instTime2 + 0.25f) + Bias[2]);
	windL3[2] = maxDeltaPosOS[2] * (speedCos(instTime2 + 0.50f) + Bias[2]);
	windL3[3] = maxDeltaPosOS[2] * (speedCos(instTime2 + 0.75f) + Bias[2]);

	if (isUBOActive())
	{
		// Write wind data into user VP UBO
		_WindTreeUB->lock();
		_WindTreeUB->set(_UBOOffsets.WindLevel1, windL1);
		for (int i = 0; i < 4; ++i)
			_WindTreeUB->set(_UBOOffsets.WindLevel2 + i * 16, windL2[i]);
		for (int i = 0; i < 4; ++i)
			_WindTreeUB->set(_UBOOffsets.WindLevel3 + i * 16, windL3[i]);
		_WindTreeUB->unlock();
	}
	else
	{
		// Legacy: individual uniforms
		driver->setUniform3f(IDriver::VertexProgram, vpIdx.WindLevel1, windL1);
		for (int i = 0; i < 4; ++i)
			driver->setUniform3f(IDriver::VertexProgram, vpIdx.WindLevel2[i], windL2[i]);
		for (int i = 0; i < 4; ++i)
			driver->setUniform3f(IDriver::VertexProgram, vpIdx.WindLevel3[i], windL3[i]);
	}
}

// ***************************************************************************
bool	CMeshVPWindTree::begin(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat, const NLMISC::CVector & /*viewerPos*/)
{
	if (driver->isVertexProgramEmulated()) return false;


	// Activate the good VertexProgram
	//===============

	// Try UBO program first (single program for all variants)
	if (_VertexProgramUBO && driver->supportBuiltinUBO()
		&& driver->activeVertexProgram(_VertexProgramUBO))
	{
		_ActiveVertexProgramUBO = _VertexProgramUBO;
		_ActiveVertexProgram = NULL;
		driver->bindUniformBuffer(UBBindingVertexProgram, _WindTreeUB);
	}
	else
	{
		_ActiveVertexProgramUBO = NULL;

		// Legacy: select from 16 variants based on numPls/specular/normalize
		nlassert(scene != NULL);
		CRenderTrav		*renderTrav= &scene->getRenderTrav();
		renderTrav->prepareVPLightSetup();
		sint	numPls= renderTrav->getNumVPLights()-1;
		clamp(numPls, 0, CRenderTrav::MaxVPLight-1);

		// Enable normalize only if requested by user. Because lighting don't manage correct "scale lighting"
		uint	idVP= (SpecularLighting?2:0) + (driver->isForceNormalize()?1:0) ;
		// correct VP id for correct number of pls.
		idVP= numPls*4 + idVP;
		// activate VP.
		if (driver->activeVertexProgram(_VertexProgram[idVP]))
		{
			_ActiveVertexProgram = _VertexProgram[idVP];
		}
		else
		{
			// vertex program not supported
			_ActiveVertexProgram = NULL;
			return false;
		}
	}


	// precompute mesh
	setupPerMesh(driver, scene);

	// Setup instance constants
	setupPerInstanceConstants(driver, scene, mbi, invertedModelMat);

	return true;
}

// ***************************************************************************
void	CMeshVPWindTree::end(IDriver *driver)
{
	// Disable the VertexProgram
	driver->activeVertexProgram(NULL);
	if (_ActiveVertexProgramUBO)
		driver->bindUniformBuffer(UBBindingVertexProgram, NULL);
	_ActiveVertexProgram = NULL;
	_ActiveVertexProgramUBO = NULL;
}

// ***************************************************************************
// tool fct
static inline void SetupForMaterial(const CMaterial &mat, CScene *scene)
{
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	renderTrav->changeVPLightSetupMaterial(mat, false /* don't exclude strongest */);
}

// ***************************************************************************
void	CMeshVPWindTree::setupForMaterial(const CMaterial &mat,
										  IDriver *drv,
									      CScene *scene,
										  CVertexBuffer *)
{
	if (isUBOActive())
	{
		// UBO path: write material properties into user VP UBO
		// When SpecularLighting is false, force specular to black (legacy path
		// selected a non-specular VP variant; UBO path folds via material=black).
		NLMISC::CRGBAF d(mat.getDiffuse());
		NLMISC::CRGBAF s = SpecularLighting ? NLMISC::CRGBAF(mat.getSpecular()) : NLMISC::CRGBAF(0.f, 0.f, 0.f, 0.f);
		_WindTreeUB->lock();
		_WindTreeUB->set(_UBOOffsets.MaterialDiffuse, d.R, d.G, d.B, d.A);
		_WindTreeUB->set(_UBOOffsets.MaterialSpecular, s.R, s.G, s.B, s.A);
		_WindTreeUB->set(_UBOOffsets.MaterialShininess, mat.getShininess());
		_WindTreeUB->unlock();
	}
	else
	{
		// Legacy: pre-multiply light colors by material via changeVPLightSetupMaterial
		SetupForMaterial(mat, scene);
	}
}

// ***************************************************************************
void	CMeshVPWindTree::setupLighting(CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat)
{
	nlassert(scene != NULL);
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	// setup cte for lighting
	CVertexProgramWindTree *program = _ActiveVertexProgram;
	renderTrav->beginVPLightSetup(program, invertedModelMat);
}


// ***************************************************************************
// ***************************************************************************
// MBR interface
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMeshVPWindTree::supportMeshBlockRendering() const
{
	return true;
}

// ***************************************************************************
bool	CMeshVPWindTree::isMBRVpOk(IDriver *driver) const
{
	initVertexPrograms();

	if (driver->isVertexProgramEmulated())
	{
		return false;
	}
	bool res = false;
	// Try to compile UBO program
	if (_VertexProgramUBO)
	{
		if (driver->compileVertexProgram(_VertexProgramUBO))
		{
			res = true;
		}
		else
		{
			nldebug("GL3 WindTree: UBO vertex program not available, using variant path");
		}
	}
	for (uint i = 0; i < NumVp; ++i)
	{
		if (!driver->compileVertexProgram(_VertexProgram[i]))
		{
			if (!res)
			{
				nlwarning("GL3 WindTree: compileVertexProgram failed for variant %u", i);
				return false;
			}
		}
	}
	return true;
}

// ***************************************************************************
void	CMeshVPWindTree::beginMBRMesh(IDriver *driver, CScene *scene)
{
	// Try UBO program first (single program for all variants, skip variant switching entirely)
	if (_VertexProgramUBO && driver->supportBuiltinUBO()
		&& driver->activeVertexProgram(_VertexProgramUBO))
	{
		_ActiveVertexProgramUBO = _VertexProgramUBO;
		_ActiveVertexProgram = NULL;
		_LastMBRIdVP = ~0u; // Sentinel: UBO path active, no variant switching
		driver->bindUniformBuffer(UBBindingVertexProgram, _WindTreeUB);
	}
	else
	{
		_ActiveVertexProgramUBO = NULL;

		/* Since need a VertexProgram Activation before activeVBHard, activate a default one
			bet the common one will be "NoPointLight, NoSpecular, No ForceNormalize" => 0.
		*/
		_LastMBRIdVP = 0;

		// activate VP.
		driver->activeVertexProgram(_VertexProgram[_LastMBRIdVP]);
		_ActiveVertexProgram = _VertexProgram[_LastMBRIdVP];
		_VertexProgram[_LastMBRIdVP]->PerMeshSetup = true;
	}

	// precompute mesh
	setupPerMesh(driver, scene);
}

// ***************************************************************************
void	CMeshVPWindTree::beginMBRInstance(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat)
{
	if (_LastMBRIdVP == ~0u)
	{
		// UBO path: single program for all instances, no variant switching
		setupPerInstanceConstants(driver, scene, mbi, invertedModelMat);
		return;
	}

	// Legacy variant path
	// Get how many pointLights are setuped now.
	nlassert(scene != NULL);
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	renderTrav->prepareVPLightSetup();
	sint	numPls= renderTrav->getNumVPLights()-1;
	clamp(numPls, 0, CRenderTrav::MaxVPLight-1);

	// Enable normalize only if requested by user. Because lighting don't manage correct "scale lighting"
	uint idVP = (SpecularLighting?2:0) + (driver->isForceNormalize()?1:0) ;
	// correct VP id for correct number of pls.
	idVP = numPls*4 + idVP;

	// re-activate VP if idVP different from last setup
	if (idVP != _LastMBRIdVP)
	{
		_LastMBRIdVP= idVP;
		driver->activeVertexProgram(_VertexProgram[_LastMBRIdVP]);
		_ActiveVertexProgram = _VertexProgram[_LastMBRIdVP];

		if (!_VertexProgram[_LastMBRIdVP]->PerMeshSetup)
		{
			// precompute mesh
			setupPerMesh(driver, scene);
			_VertexProgram[_LastMBRIdVP]->PerMeshSetup = true;
		}
	}

	// setup first constants for this instance
	setupPerInstanceConstants(driver, scene, mbi, invertedModelMat);
}

// ***************************************************************************
void	CMeshVPWindTree::endMBRMesh(IDriver *driver)
{
	// Disable the VertexProgram
	driver->activeVertexProgram(NULL);
	if (_ActiveVertexProgramUBO)
		driver->bindUniformBuffer(UBBindingVertexProgram, NULL);
	_ActiveVertexProgram = NULL;
	_ActiveVertexProgramUBO = NULL;
}

// ***************************************************************************
float	CMeshVPWindTree::getMaxVertexMove()
{
	return _MaxVertexMove;
}


} // NL3D
