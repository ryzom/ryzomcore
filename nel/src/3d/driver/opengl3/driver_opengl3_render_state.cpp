// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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

// Render state: fog, stencil, depth/cull, polygon mode, blend constant,
// clip planes, and builtin UBO uploads (camera, object, material).

#include "stdopengl3.h"
#include "driver_opengl3.h"
#include "driver_opengl3_vertex_buffer.h"
#include "driver_opengl3_uniform_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// ***************************************************************************
// Fog
// ***************************************************************************

bool CDriverGL3::fogEnabled()
{
	H_AUTO_OGL(CDriverGL3_fogEnabled)
	return _FogEnabled;
}

void CDriverGL3::enableFog(bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableFog)
	_FogEnabled = enable;
	enableFogVP(enable);
}

void CDriverGL3::setupFog(float start, float end, NLMISC::CRGBA color)
{
	H_AUTO_OGL(CDriverGL3_setupFog)

	_CurrentFogColor[0] = color.R / 255.0f;
	_CurrentFogColor[1] = color.G / 255.0f;
	_CurrentFogColor[2] = color.B / 255.0f;
	_CurrentFogColor[3] = color.A / 255.0f;

	_FogStart = start;
	_FogEnd = end;
	_CameraUBODirty = true;
}

float CDriverGL3::getFogStart() const
{
	H_AUTO_OGL(CDriverGL3_getFogStart)
	return _FogStart;
}

float CDriverGL3::getFogEnd() const
{
	H_AUTO_OGL(CDriverGL3_getFogEnd)
	return _FogEnd;
}

NLMISC::CRGBA CDriverGL3::getFogColor() const
{
	H_AUTO_OGL(CDriverGL3_getFogColor)
	NLMISC::CRGBA ret;
	ret.R = (uint8)(_CurrentFogColor[0] * 255);
	ret.G = (uint8)(_CurrentFogColor[1] * 255);
	ret.B = (uint8)(_CurrentFogColor[2] * 255);
	ret.A = (uint8)(_CurrentFogColor[3] * 255);
	return ret;
}

void CDriverGL3::setupFogMode(TFogMode mode, float density)
{
	H_AUTO_OGL(CDriverGL3_setupFogMode)
	_FogMode = mode;
	_FogDensity = density;
	_CameraUBODirty = true;
}

IDriver::TFogMode CDriverGL3::getFogMode() const
{
	return _FogMode;
}

float CDriverGL3::getFogDensity() const
{
	return _FogDensity;
}

// ***************************************************************************
// Polygon mode
// ***************************************************************************

void CDriverGL3::setPolygonMode(TPolygonMode mode)
{
	H_AUTO_OGL(CDriverGL3_setPolygonMode)
	IDriver::setPolygonMode(mode);

	// Set the polygon mode
	switch (_PolygonMode)
	{
	case Filled:
		_DriverGLStates.polygonMode(GL_FILL);
		break;
	case Line:
		_DriverGLStates.polygonMode(GL_LINE);
		break;
	case Point:
		_DriverGLStates.polygonMode(GL_POINT);
		break;
	}
}

// GL_POLYGON_SMOOTH is not available in GL 3.3 core profile.
// Only caller is CShadowMapManager, which uses it to smooth shadow polygon edges.
// Alternative: render shadow maps to an MSAA FBO and resolve, or apply a
// post-process blur/edge-detection filter on the shadow map.
void CDriverGL3::enablePolygonSmoothing(bool smooth)
{
	H_AUTO_OGL(CDriverGL3_enablePolygonSmoothing);
	_PolygonSmooth = smooth;
}

bool CDriverGL3::isPolygonSmoothingEnabled() const
{
	H_AUTO_OGL(CDriverGL3_isPolygonSmoothingEnabled)

	return _PolygonSmooth;
}

// ***************************************************************************
// Blend constant color
// ***************************************************************************

bool CDriverGL3::supportBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL3_supportBlendConstantColor)
	return _Extensions.GLCore;
}

void CDriverGL3::setBlendConstantColor(NLMISC::CRGBA col)
{
	H_AUTO_OGL(CDriverGL3_setBlendConstantColor)

	_DriverGLStates.blendColor(col);
}

NLMISC::CRGBA CDriverGL3::getBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	return _DriverGLStates.getBlendColor();
}

// ***************************************************************************
// Depth range and cull mode
// ***************************************************************************

void CDriverGL3::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGL3_setDepthRange)
	_DriverGLStates.setDepthRange(znear, zfar);
}

void CDriverGL3::getDepthRange(float &znear, float &zfar) const
{
	H_AUTO_OGL(CDriverGL3_getDepthRange)
	_DriverGLStates.getDepthRange(znear, zfar);
}

void CDriverGL3::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGL3_setCullMode)
	_DriverGLStates.setCullMode((CDriverGLStates3::TCullMode)cullMode);
}

CDriverGL3::TCullMode CDriverGL3::getCullMode() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	return (CDriverGL3::TCullMode)_DriverGLStates.getCullMode();
}

// ***************************************************************************
// Stencil
// ***************************************************************************

void CDriverGL3::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	_DriverGLStates.enableStencilTest(enable);
}

bool CDriverGL3::isStencilTestEnabled() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	return _DriverGLStates.isStencilTestEnabled();
}

void CDriverGL3::stencilFunc(TStencilFunc stencilFunc, int ref, uint mask)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	GLenum glstencilFunc = 0;

	switch (stencilFunc)
	{
	case IDriver::never: glstencilFunc = GL_NEVER; break;
	case IDriver::less: glstencilFunc = GL_LESS; break;
	case IDriver::lessequal: glstencilFunc = GL_LEQUAL; break;
	case IDriver::equal: glstencilFunc = GL_EQUAL; break;
	case IDriver::notequal: glstencilFunc = GL_NOTEQUAL; break;
	case IDriver::greaterequal: glstencilFunc = GL_GEQUAL; break;
	case IDriver::greater: glstencilFunc = GL_GREATER; break;
	case IDriver::always: glstencilFunc = GL_ALWAYS; break;
	default: nlstop;
	}

	_DriverGLStates.stencilFunc(glstencilFunc, (GLint)ref, (GLuint)mask);
}

void CDriverGL3::stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	GLenum glFail = 0, glZFail = 0, glZPass = 0;

	switch (fail)
	{
	case IDriver::keep: glFail = GL_KEEP; break;
	case IDriver::zero: glFail = GL_ZERO; break;
	case IDriver::replace: glFail = GL_REPLACE; break;
	case IDriver::incr: glFail = GL_INCR; break;
	case IDriver::decr: glFail = GL_DECR; break;
	case IDriver::invert: glFail = GL_INVERT; break;
	default: nlstop;
	}

	switch (zfail)
	{
	case IDriver::keep: glZFail = GL_KEEP; break;
	case IDriver::zero: glZFail = GL_ZERO; break;
	case IDriver::replace: glZFail = GL_REPLACE; break;
	case IDriver::incr: glZFail = GL_INCR; break;
	case IDriver::decr: glZFail = GL_DECR; break;
	case IDriver::invert: glZFail = GL_INVERT; break;
	default: nlstop;
	}

	switch (zpass)
	{
	case IDriver::keep: glZPass = GL_KEEP; break;
	case IDriver::zero: glZPass = GL_ZERO; break;
	case IDriver::replace: glZPass = GL_REPLACE; break;
	case IDriver::incr: glZPass = GL_INCR; break;
	case IDriver::decr: glZPass = GL_DECR; break;
	case IDriver::invert: glZPass = GL_INVERT; break;
	default: nlstop;
	}

	_DriverGLStates.stencilOp(glFail, glZFail, glZPass);
}

void CDriverGL3::stencilMask(uint mask)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	_DriverGLStates.stencilMask((GLuint)mask);
}

// ***************************************************************************
// Clip planes
// ***************************************************************************

void CDriverGL3::enableClipPlane(uint index, bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableClipPlane)

	if (index >= MaxClipPlanes) return;

	_ClipPlaneEnabled[index] = enable;

	// Enable/disable GL clip distance (for builtin VPs that write gl_ClipDistance)
	if (!m_PPClipPlanes)
	{
		_DriverGLStates.enableClipDistance(index, enable);
	}

	// Trigger VP regeneration to include/exclude gl_ClipDistance output
	touchClipPlaneVP(index, enable);
	_CameraUBODirty = true;
}

void CDriverGL3::setClipPlane(uint index, const NLMISC::CPlane &plane)
{
	H_AUTO_OGL(CDriverGL3_setClipPlane)

	if (index >= MaxClipPlanes) return;

	// Plane is in NeL world space. Adjust d for _PZBCameraPos precision optimization,
	// then transform to eye space for the vertex shader.
	float pa = plane.a;
	float pb = plane.b;
	float pc = plane.c;
	float pd = plane.d + pa * _PZBCameraPos.x + pb * _PZBCameraPos.y + pc * _PZBCameraPos.z;

	// Transform plane from PZB-adjusted world space to eye space:
	// plane_eye = transpose(inverse(_ViewMtx)) * plane_pzb_world
	// For rigid body transform V: (V^-1)^T maps covariant vectors (planes/normals).
	CMatrix invView = _ViewMtx;
	invView.invert();
	CVector invI = invView.getI();
	CVector invJ = invView.getJ();
	CVector invK = invView.getK();
	CVector invPos = invView.getPos();

	_ClipPlaneEye[index][0] = invI.x * pa + invI.y * pb + invI.z * pc;
	_ClipPlaneEye[index][1] = invJ.x * pa + invJ.y * pb + invJ.z * pc;
	_ClipPlaneEye[index][2] = invK.x * pa + invK.y * pb + invK.z * pc;
	_ClipPlaneEye[index][3] = invPos.x * pa + invPos.y * pb + invPos.z * pc + pd;
	_CameraUBODirty = true;
}

// ***************************************************************************
// Camera UBO
// ***************************************************************************

void CDriverGL3::stageCameraUBO()
{
	CCameraUBOData &data = _CameraUBOData;

	// View matrix (column-major, CMatrix::get() returns column-major)
	const float *vm = _ViewMtx.get();
	memcpy(data.viewMatrix, vm, 16 * sizeof(float));

	// Fog color (black for additive lightmap passes to avoid adding fog twice)
	if (_FogColorOverrideBlack)
		memset(data.fogColor, 0, 4 * sizeof(float));
	else
		memcpy(data.fogColor, _CurrentFogColor, 4 * sizeof(float));

	// PZB camera position
	data.pzbCameraPos[0] = _PZBCameraPos.x;
	data.pzbCameraPos[1] = _PZBCameraPos.y;
	data.pzbCameraPos[2] = _PZBCameraPos.z;

	// Fog density
	data.fogDensity = _FogDensity;

	// Fog params (start, end)
	data.fogParams[0] = _FogStart;
	data.fogParams[1] = _FogEnd;

	// Fog mode and clip plane mask
	data.fogMode = (sint32)_FogMode;
	{
		sint32 mask = 0;
		for (uint i = 0; i < MaxClipPlanes; ++i)
			if (_ClipPlaneEnabled[i]) mask |= (1 << i);
		data.clipPlaneMask = mask;
	}

	// Clip planes
	for (uint i = 0; i < MaxClipPlanes; ++i)
		memcpy(data.clipPlane[i], _ClipPlaneEye[i], 4 * sizeof(float));

	// Camera world position
	{
		CMatrix invView = _ViewMtx;
		invView.invert();
		CVector cwp = invView.getPos();
		data.cameraWorldPos[0] = cwp.x;
		data.cameraWorldPos[1] = cwp.y;
		data.cameraWorldPos[2] = cwp.z;
		data._pad0 = 0.f;
	}

	// Specular texture matrix: inverse view rotation for eye-space reflection → world-space cubemap lookup
	memcpy(data.specularTexMtx, _SpecularTexMtx.get(), 16 * sizeof(float));

	// Inverse projection basis: inv(Projection * ChangeBasis) transforms clip-space → NeL eye-space.
	// Used by nelvp-converted VPs to synthesize ecPos from gl_Position for fog computation.
	{
		CMatrix invProjBasis = _GLProjMat * _ChangeBasis;
		invProjBasis.invert();
		memcpy(data.inverseProjectionBasis, invProjBasis.get(), 16 * sizeof(float));
	}

	_CameraUBODirty = false;
	_CameraUBOUploadDirty = true;
}

void CDriverGL3::uploadCameraUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadCameraUBO)

	if (!_CameraUBOId || !_CameraUBOUploadDirty)
		return;

	const GLsizeiptr dataSize = sizeof(CCameraUBOData);
	_DriverGLStates.forceBindUniformBuffer(_CameraUBOId);

	if (_CameraUBOCapacity < (sint)dataSize)
	{
		nglBufferData(GL_UNIFORM_BUFFER, dataSize, &_CameraUBOData, GL_STREAM_DRAW);
		_CameraUBOCapacity = (sint)dataSize;
	}
	else
	{
		nglBufferData(GL_UNIFORM_BUFFER, _CameraUBOCapacity, NULL, GL_STREAM_DRAW);
		nglBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, &_CameraUBOData);
	}

	_DriverGLStates.forceBindUniformBuffer(0);
	_DriverGLStates.forceBindUniformBufferBase(NL_BUILTIN_CAMERA_BINDING, _CameraUBOId);

	_CameraUBOUploadDirty = false;
}

// ***************************************************************************
// Object UBO (per-draw-call)
// ***************************************************************************

void CDriverGL3::uploadObjectUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadObjectUBO)

	if (!_ObjectUBOId)
		return;

	CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));

	CObjectUBOData data;

	// ModelViewProjection
	CMatrix mvp = _GLProjMat * _ChangeBasis * _ModelViewMatrix;
	memcpy(data.modelViewProjection, mvp.get(), 16 * sizeof(float));

	// ModelView
	memcpy(data.modelView, _ModelViewMatrix.get(), 16 * sizeof(float));

	// NormalMatrix (3×3 → std140 mat3 = 3 columns of vec4, padded)
	const float *mv = _ModelViewMatrix.get();
	// Column 0
	data.normalMatrix[0] = mv[0];
	data.normalMatrix[1] = mv[1];
	data.normalMatrix[2] = mv[2];
	data.normalMatrix[3] = 0.0f;
	// Column 1
	data.normalMatrix[4] = mv[4];
	data.normalMatrix[5] = mv[5];
	data.normalMatrix[6] = mv[6];
	data.normalMatrix[7] = 0.0f;
	// Column 2
	data.normalMatrix[8] = mv[8];
	data.normalMatrix[9] = mv[9];
	data.normalMatrix[10] = mv[10];
	data.normalMatrix[11] = 0.0f;

	// Light indices and factors (packed into ivec4 / vec4)
	for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
	{
		sint32 lightIdx;
		float lightFactor;
		if (_LightTableMode)
		{
			lightIdx = (sint32)_LightTableObjIndices[i];
			lightFactor = _LightMapUBOOverride.Active && _LightMapUBOOverride.ZeroLightFactors
			    ? 0.0f
			    : _LightTableObjFactors[i];
		}
		else
		{
			lightIdx = _LightEnable[i] ? (sint32)i : -1;
			lightFactor = _LightMapUBOOverride.Active && _LightMapUBOOverride.ZeroLightFactors
			    ? 0.0f
			    : 1.0f;
		}

		if (i < 4)
		{
			data.lightIndices01[i] = lightIdx;
			data.lightFactors01[i] = lightFactor;
		}
		else
		{
			data.lightIndices45[i - 4] = lightIdx;
			data.lightFactors45[i - 4] = lightFactor;
		}
	}

	// Self illumination (precomputed on CPU, or overridden for lightmap passes)
	if (_LightMapUBOOverride.Active)
	{
		memcpy(data.selfIllumination, _LightMapUBOOverride.SelfIllumination, sizeof(data.selfIllumination));
	}
	else
	{
		NLMISC::CRGBAF selfIllumination = NLMISC::CRGBAF(_AmbientGlobal);
		for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			if (_LightEnable[i])
				selfIllumination += NLMISC::CRGBAF(_UserLight[i].getAmbiant());
		}
		selfIllumination *= matDrv->Ambient;
		if (matDrv->PPBuiltin.Shader != CMaterial::LightMap)
			selfIllumination += matDrv->Emissive;
		data.selfIllumination[0] = selfIllumination.R;
		data.selfIllumination[1] = selfIllumination.G;
		data.selfIllumination[2] = selfIllumination.B;
		data.selfIllumination[3] = 0.0f;
	}

	// UV routing (from vertex buffer, only first 4 stages used by materials)
	if (_CurrentVertexBufferGL)
	{
		const uint8 *uvr = _CurrentVertexBufferGL->VB->getUVRouting();
		for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			data.uvRouting[i] = (sint32)uvr[i];
	}
	else
	{
		for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			data.uvRouting[i] = (sint32)i;
	}

	// TexGen modes
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		data.texGenMode[i] = m_VPBuiltinCurrent.TexGenMode[i];

	// Scalar state
	data.lighting = m_VPBuiltinCurrent.Lighting ? 1 : 0;
	data.vertexColorLighted = m_VPBuiltinCurrent.VertexColorLighted ? 1 : 0;
	data.vertexFormat = (sint32)m_VPBuiltinCurrent.VertexFormat;
	data.worldSpaceNormal = m_VPNormalOutput ? 1 : 0;
	data.worldSpacePosition = m_VPWorldSpacePositionOutput ? 1 : 0;
	data.numPerPixelLights = (sint32)m_VPBuiltinCurrent.NumPerPixelLights;
	data.fogEnabled = m_VPBuiltinCurrent.Fog ? 1 : 0;
	data._pad[0] = 0;

	// Upload
	const GLsizeiptr dataSize = sizeof(CObjectUBOData);
	_DriverGLStates.forceBindUniformBuffer(_ObjectUBOId);

	if (_ObjectUBOCapacity < (sint)dataSize)
	{
		nglBufferData(GL_UNIFORM_BUFFER, dataSize, &data, GL_STREAM_DRAW);
		_ObjectUBOCapacity = (sint)dataSize;
	}
	else
	{
		// Orphan + rewrite
		nglBufferData(GL_UNIFORM_BUFFER, _ObjectUBOCapacity, NULL, GL_STREAM_DRAW);
		nglBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, &data);
	}

	_DriverGLStates.forceBindUniformBuffer(0);
	_DriverGLStates.forceBindUniformBufferBase(NL_BUILTIN_MODEL_BINDING, _ObjectUBOId);
}

// ***************************************************************************
// Material UBO
// ***************************************************************************

void CDriverGL3::uploadMaterialUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadMaterialUBO)

	CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	if (!matDrv) return;

	// Upload and bind the active material UBO slot.
	// Slot 0 = base material (from setupMaterial/setupNormalPass).
	// Slots 1..N = per-pass overrides (from setupLightMapPass, etc.).
	uint slot = matDrv->MaterialUBOCurrent;
	if (matDrv->MaterialUBOTouched[slot])
	{
		if (!matDrv->MaterialUBOId[slot])
			nglGenBuffers(1, &matDrv->MaterialUBOId[slot]);

		const GLsizeiptr dataSize = sizeof(CMaterialUBOData);
		_DriverGLStates.forceBindUniformBuffer(matDrv->MaterialUBOId[slot]);
		nglBufferData(GL_UNIFORM_BUFFER, dataSize, &matDrv->MaterialUBO[slot], GL_STREAM_DRAW);
		_DriverGLStates.forceBindUniformBuffer(0);
		matDrv->MaterialUBOTouched[slot] = false;
	}

	_DriverGLStates.bindUniformBufferBase(NL_BUILTIN_MATERIAL_BINDING, matDrv->MaterialUBOId[slot]);
}

} // NLDRIVERGL3
} // NL3D
