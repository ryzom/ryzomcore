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
#include "driver_opengl3.h"
#include "driver_opengl3_uniform_buffer.h"

#include <sstream>

#include <nel/misc/debug.h>

namespace NL3D {
namespace NLDRIVERGL3 {

std::string buildGLSLLightTableHeader(sint maxLights)
{
	// Light table UBO: shared across all objects, uploaded once when lights change.
	// User VPs can reference nlLights[] directly when UsesLightTableUBO is set.
	// Binding point is set from the CPU via glUniformBlockBinding in setupInitialUniforms.
	std::string result =
		"struct NlLightInfo {\n"
		"    vec3  dirOrPos;\n"
		"    int   mode;\n"        // 0=directional, 1=point, 2=spot
		"    vec4  diffuse;\n"
		"    vec4  specular;\n"
		"    float constAttn;\n"
		"    float linAttn;\n"
		"    float quadAttn;\n"
		"    float spotExp;\n"
		"    vec3  spotDir;\n"
		"    float spotCutoff;\n"  // cos(cutoff angle)
		"    vec4  ambient;\n"
		"};\n"
		"layout(std140) uniform NlLightTable {\n"
		"    NlLightInfo nlLights[";
	result += NLMISC::toString(maxLights);
	result +=
		"];\n"
		"};\n";
	return result;
}

const char *GLSLCameraHeader =
	// Camera/global state UBO: shared across all objects, uploaded once per frame.
	// Contains view matrix, fog params, PZB camera pos, clip planes, and control masks.
	// Binding point is set from the CPU via glUniformBlockBinding in setupInitialUniforms.
	"layout(std140) uniform NlCamera {\n"
	"    mat4  viewMatrix;\n"
	"    vec4  fogColor;\n"
	"    vec3  pzbCameraPos;\n"
	"    float fogDensity;\n"
	"    vec2  fogParams;\n"
	"    int   nlFogMode;\n"
	"    int   nlClipPlaneMask;\n"
	"    vec4  clipPlane0;\n"
	"    vec4  clipPlane1;\n"
	"    vec4  clipPlane2;\n"
	"    vec4  clipPlane3;\n"
	"    vec4  clipPlane4;\n"
	"    vec4  clipPlane5;\n"
	"    vec3  cameraWorldPos;\n"
	"    float _nlCamPad0;\n"
	"    mat4  specularTexMtx;\n"
	"    mat4  inverseProjectionBasis;\n" // inv(Projection * ChangeBasis): clip-space → NeL eye-space
	"};\n";

const char *GLSLObjectHeader =
	// Per-object UBO: matrices, light indices/factors, and per-draw-call state.
	// Uploaded every draw call. Binding point set via glUniformBlockBinding in setupInitialUniforms.
	"layout(std140) uniform NlModel {\n"
	"    mat4  modelViewProjection;\n"
	"    mat4  modelView;\n"
	"    mat3  normalMatrix;\n"
	"    ivec4 nlLightIndices01;\n"
	"    ivec4 nlLightIndices45;\n"
	"    vec4  nlLightFactors01;\n"
	"    vec4  nlLightFactors45;\n"
	"    vec4  selfIllumination;\n"
	"    ivec4 nlUVRouting;\n"
	"    ivec4 nlTexGenMode;\n"
	"    int   nlLighting;\n"
	"    int   nlVertexColorLighted;\n"
	"    int   nlVertexFormat;\n"
	"    int   nlWorldSpaceNormal;\n"
	"    int   nlWorldSpacePosition;\n"
	"    int   nlNumPerPixelLights;\n"
	"    int   nlFogEnabled;\n"
	"    int   _nlObjPad0;\n"
	"};\n";

const char *GLSLMaterialHeader =
	// Per-material UBO: material colors, alpha test, shader type, texenv modes,
	// TexEnv constants, EMBM matrices, and texture matrices.
	// Uploaded when material changes. Binding point set via glUniformBlockBinding in setupInitialUniforms.
	"layout(std140) uniform NlMaterial {\n"
	"    vec4  materialColor;\n"
	"    vec4  materialDiffuse;\n"
	"    vec4  materialSpecular;\n"
	"    float materialShininess;\n"
	"    float alphaRef;\n"
	"    int   nlShader;\n"
	"    int   nlTextureActive;\n"
	"    int   nlAlphaTest;\n"
	"    float nlLightMapScale;\n"
	"    int   _matPad0;\n"
	"    int   _matPad1;\n"
	"    uint  nlTexEnvMode0;\n"
	"    uint  nlTexEnvMode1;\n"
	"    uint  nlTexEnvMode2;\n"
	"    uint  nlTexEnvMode3;\n"
	"    vec4  constant0;\n"
	"    vec4  constant1;\n"
	"    vec4  constant2;\n"
	"    vec4  constant3;\n"
	"    vec4  constant4;\n"  // EMBM matrix stage 0 (Normal/UserColor) or lightmap factor 4 (LightMap)
	"    vec4  constant5;\n"  // EMBM matrix stage 1 or lightmap factor 5
	"    vec4  constant6;\n"  // EMBM matrix stage 2 or lightmap factor 6
	"    vec4  constant7;\n"  // EMBM matrix stage 3 or lightmap factor 7
	"    mat4  texMatrix0;\n"
	"    mat4  texMatrix1;\n"
	"    mat4  texMatrix2;\n"
	"    mat4  texMatrix3;\n"
	"};\n";

static const char *s_TypeKeyword[] = {
	"float", // float
	"vec2", // CVector2D
	"vec3",
	"vec4", // CVector
	"int", // sint32
	"ivec2",
	"ivec3",
	"ivec4",
	"uint", // uint32
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
	// Emit struct definitions
	for (sint i = 0; i < ubf.structCount(); ++i)
	{
		ss << "struct " << ubf.getStructName(i) << "\n";
		ss << "{\n";
		const CUniformBufferFormat &sf = ubf.getStructFormat(i);
		for (sint j = 0; j < (sint)sf.count(); ++j)
		{
			const CUniformBufferFormat::CEntry &field = sf.get(j);
			ss << "\t" << s_TypeKeyword[field.Type] << " " << NLMISC::CStringMapper::unmap(field.Name);
			if (field.Count != 1)
				ss << "[" << field.Count << "]";
			ss << ";\n";
		}
		ss << "};\n";
	}

	std::string blockName = ubf.Name.empty()
		? ("NlUBO" + NLMISC::toString(binding))
		: ubf.Name;
	ss << "layout(std140) uniform " << blockName << "\n";
	ss << "{\n";
	for (sint i = 0; i < (sint)ubf.count(); ++i)
	{
		const CUniformBufferFormat::CEntry &entry = ubf.get(i);
		if (entry.StructIndex >= 0)
			ss << "\t" << ubf.getStructName(entry.StructIndex);
		else
			ss << "\t" << s_TypeKeyword[entry.Type];
		ss << " " << NLMISC::CStringMapper::unmap(entry.Name);
		if (entry.Count != 1)
			ss << "[" << entry.Count << "]";
		ss << ";\n";
	}
	ss << "};\n";
}

// ***************************************************************************
// CUBDrvInfosGL3
// ***************************************************************************

CUBDrvInfosGL3::CUBDrvInfosGL3(IDriver *drv, ItUBDrvInfoPtrList it, CUniformBuffer *ub)
	: IUBDrvInfos(drv, it, ub)
	, _BufferId(0)
	, _Capacity(0)
{
	nglGenBuffers(1, &_BufferId);
}

CUBDrvInfosGL3::~CUBDrvInfosGL3()
{
	if (_BufferId)
		nglDeleteBuffers(1, &_BufferId);
}

// ***************************************************************************
// CDriverGL3::bindUniformBuffer
// ***************************************************************************

static GLenum usageHintToGL(CUniformBuffer::TUsageHint hint)
{
	switch (hint)
	{
	case CUniformBuffer::StreamDraw:  return GL_STREAM_DRAW;
	case CUniformBuffer::DynamicDraw: return GL_DYNAMIC_DRAW;
	case CUniformBuffer::StaticDraw:  return GL_STATIC_DRAW;
	default:                          return GL_STREAM_DRAW;
	}
}

static const sint s_UBBindingToGL[] = {
	NL_USER_VERTEX_PROGRAM_BINDING,  // UBBindingVertexProgram
	NL_USER_PIXEL_PROGRAM_BINDING,   // UBBindingPixelProgram
	NL_USER_SKELETON_BINDING,        // UBBindingSkeleton
};

bool CDriverGL3::bindUniformBuffer(TUBBinding binding, CUniformBuffer *ub)
{
	nlassert(binding < UBBindingCount);
	if (_BoundUserUB[binding] == ub)
		return true;

	_BoundUserUB[binding] = ub;

	if (!ub)
	{
		// Immediate unbind — avoid dangling pointer if buffer is released before next flush
		if (_UserUBBoundId[binding])
		{
			_DriverGLStates.forceBindUniformBufferBase(s_UBBindingToGL[binding], 0);
			_UserUBBoundId[binding] = 0;
		}
	}
	return true;
}

// ***************************************************************************
// CDriverGL3::flushUserUBOs
// ***************************************************************************

void CDriverGL3::flushUserUBOs()
{
	for (sint i = 0; i < UBBindingCount; ++i)
	{
		CUniformBuffer *ub = _BoundUserUB[i];

		if (!ub)
		{
			// Detect auto-nullification: CRefPtr cleared it behind our back
			if (_UserUBBoundId[i])
			{
				_DriverGLStates.forceBindUniformBufferBase(s_UBBindingToGL[i], 0);
				_UserUBBoundId[i] = 0;
			}
			continue;
		}

		// Create driver info on first use
		if (!ub->DrvInfos)
		{
			ItUBDrvInfoPtrList it = _UBDrvInfos.insert(_UBDrvInfos.end(), (IUBDrvInfos *)NULL);
			CUBDrvInfosGL3 *info = new CUBDrvInfosGL3(this, it, ub);
			*it = info;
			ub->DrvInfos = info;
		}

		CUBDrvInfosGL3 *info = static_cast<CUBDrvInfosGL3 *>((IUBDrvInfos *)ub->DrvInfos);

		// Upload if data dirty
		if (ub->Touched)
		{
			sint dataSize = ub->Format.size();
			GLenum usage = usageHintToGL(ub->UsageHint);

			_DriverGLStates.forceBindUniformBuffer(info->getBufferId());

			if (info->getCapacity() < dataSize)
			{
				// Allocate or grow
				nglBufferData(GL_UNIFORM_BUFFER, dataSize, ub->data(), usage);
				info->setCapacity(dataSize);
			}
			else
			{
				// Orphan + rewrite (same size)
				nglBufferData(GL_UNIFORM_BUFFER, dataSize, NULL, usage);
				nglBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, ub->data());
			}

			ub->Touched = false;
		}

		// Bind to indexed binding point only when GL buffer ID changed
		GLuint bufId = info->getBufferId();
		if (_UserUBBoundId[i] != bufId)
		{
			_DriverGLStates.forceBindUniformBufferBase(s_UBBindingToGL[i], bufId);
			_UserUBBoundId[i] = bufId;
		}
	}
}

} // NLDRIVERGL3
} // NL3D

/* end of file */
