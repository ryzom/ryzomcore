// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/3d/light.h"

#include <cmath>

namespace NL3D {
namespace NLDRIVERGL3 {

// ***************************************************************************
uint	CDriverGL3::getMaxLight () const
{
	H_AUTO_OGL(CDriverGL3_getMaxLight)
	// return MaxLight=8.
	return MaxLight;
}


// ***************************************************************************
void	CDriverGL3::setLight (uint8 num, const CLight& light)
{
	H_AUTO_OGL(CDriverGL3_setLight)
	// bkup real light, for lightmap dynamic lighting purpose
	if (num==0)
	{
		_UserLight0= light;
		// because the GL setup change, must dirt lightmap rendering
		_LightMapDynamicLightDirty= true;
	}

	setLightInternal(num, light);
}


CLight	CDriverGL3::getLight (uint8 num)
{
	return _UserLight[ num ];
}


// ***************************************************************************
void	CDriverGL3::setLightInternal(uint8 num, const CLight& light)
{
	H_AUTO_OGL(CDriverGL3_setLightInternal)
	// Check light count is good
	// nlassert(num < MaxLight);

	// Set the light
	if (num < MaxLight)
	{
		// GL light number
		GLenum lightNum = (GLenum)(GL_LIGHT0+num);

		// Get light mode
		CLight::TLightMode mode = light.getMode ();

		// Copy the mode
		_LightMode[num] = mode;
		_UserLight[num] = light;
		_UserLightUBODirty = true;
		touchLightVP(num);

		// Set the position
		if ((mode == CLight::DirectionalLight) || (mode == CLight::SpotLight))
		{
			// Get the direction of the light
			_WorldLightDirection[num] = light.getDirection();
		}

		if (mode != CLight::DirectionalLight)
		{
			// Get the position of the light
			_WorldLightPos[num] = light.getPosition();
		}

		if (mode == CLight::SpotLight)
		{
			// Get the exponent of the spot
			float exponent = light.getExponent();

			// Get the cutoff of the spot
			float cutoff = 180.f * (light.getCutoff() / (float)NLMISC::Pi);

		}

	}
}

// ***************************************************************************
void	CDriverGL3::enableLight(uint8 num, bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableLight)

	// User call => set the User flag (preserved across setupLightMapDynamicLighting)
	if (num < MaxLight)
		_UserLightEnable[num] = enable;

	// enable the light internally
	enableLightInternal(num, enable);

	// because the GL setup has changed, must dirt lightmap rendering
	_LightMapDynamicLightDirty = true;
}

bool	CDriverGL3::isLightEnabled (uint8 num)
{
	if (num < MaxLight)
		return _UserLightEnable[num];

	return false;
}


// ***************************************************************************
void	CDriverGL3::enableLightInternal(uint8 num, bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableLightInternal)
	// Check light count is good
	// nlassert(num < MaxLight);

	// Set internal/active light enable state (analogous to _DriverGLStates.enableLight in original GL driver).
	// Does NOT modify _UserLightEnable — that's only set by the public enableLight() API.
	if (num < MaxLight)
	{
		_LightEnable[num] = enable;
		_UserLightUBODirty = true;
		touchLightVP(num);
	}
}


// ***************************************************************************
uint	CDriverGL3::getMaxLightTableSize() const
{
	// Must match nlLights[] array size in GLSLBuiltinHeader
	return 128;
}

// ***************************************************************************
void	CDriverGL3::enableLightTableMode(bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableLightTableMode)
	_LightTableMode = enable;
}

// ***************************************************************************
void	CDriverGL3::setLightTableSize(uint count)
{
	H_AUTO_OGL(CDriverGL3_setLightTableSize)
	_LightTable.resize(count);
	_LightTableDirty = true;
	// Invalidate cached lightmap dynamic light table index (may have been in the old table range)
	_LightMapDynLightTableIndex = -1;
}

// ***************************************************************************
void	CDriverGL3::setLightTableEntry(uint index, const CLight &light)
{
	H_AUTO_OGL(CDriverGL3_setLightTableEntry)
	if (index < _LightTable.size())
	{
		_LightTable[index] = light;
		_LightTableDirty = true;
	}
}

// ***************************************************************************
void	CDriverGL3::setLights(
	const sint16 *tableIndices,
	const uint8 *factors,
	uint numLights,
	uint numPerPixelLights,
	NLMISC::CRGBA ambient)
{
	H_AUTO_OGL(CDriverGL3_setLights)

	// Clear all lights
	for (uint i = 0; i < MaxLight; ++i)
	{
		_LightEnable[i] = false;
		_LightTableObjIndices[i] = -1;
		_LightTableObjFactors[i] = 0.0f;
	}

	// Process each light (up to MaxLight)
	uint count = std::min(numLights, (uint)MaxLight);
	_LightTableObjCount = count;
	_NumPerPixelLights = std::min(numPerPixelLights, count);
	for (uint i = 0; i < count; ++i)
	{
		sint16 tableIndex = tableIndices[i];
		if (tableIndex < 0 || tableIndex >= (sint16)_LightTable.size())
			continue;

		// Store raw index and factor for UBO path
		_LightTableObjIndices[i] = tableIndex;
		_LightTableObjFactors[i] = (float)(factors[i] + (factors[i] >> 7)) / 256.0f;

		// Get raw light from table
		const CLight &rawLight = _LightTable[tableIndex];

		// Apply factor modulation (for legacy uniform path)
		uint ufactor = factors[i];
		ufactor += ufactor >> 7; // expand 0..255 to 0..256

		CRGBA slotAmbient;
		CRGBA diffuse, specular;

		if (i == 0)
			slotAmbient = ambient;
		else
			slotAmbient = CRGBA::Black;

		diffuse.modulateFromuiRGBOnly(rawLight.getDiffuse(), ufactor);
		specular.modulateFromuiRGBOnly(rawLight.getSpecular(), ufactor);

		// Build modulated CLight
		CLight::TLightMode mode = rawLight.getMode();
		_LightMode[i] = mode;

		if (mode == CLight::DirectionalLight)
		{
			_UserLight[i].setupDirectional(slotAmbient, diffuse, specular, rawLight.getDirection());
			_WorldLightDirection[i] = rawLight.getDirection();
		}
		else if (mode == CLight::SpotLight)
		{
			_UserLight[i].setupSpotLight(slotAmbient, diffuse, specular,
				rawLight.getPosition(), rawLight.getDirection(),
				rawLight.getExponent(), rawLight.getCutoff(),
				rawLight.getConstantAttenuation(),
				rawLight.getLinearAttenuation(),
				rawLight.getQuadraticAttenuation());
			_WorldLightPos[i] = rawLight.getPosition();
			_WorldLightDirection[i] = rawLight.getDirection();
		}
		else
		{
			_UserLight[i].setupPointLight(slotAmbient, diffuse, specular,
				rawLight.getPosition(), CVector::Null,
				rawLight.getConstantAttenuation(),
				rawLight.getLinearAttenuation(),
				rawLight.getQuadraticAttenuation());
			_WorldLightPos[i] = rawLight.getPosition();
		}

		_LightEnable[i] = true;
		touchLightVP(i);
	}

	// Touch remaining slots that were enabled before
	for (uint i = count; i < MaxLight; ++i)
	{
		touchLightVP(i);
	}

	// Synchronize user state for setupLightMapDynamicLighting restore
	for (uint i = 0; i < MaxLight; ++i)
		_UserLightEnable[i] = _LightEnable[i];
	if (count > 0)
		_UserLight0 = _UserLight[0];
	_LightMapDynamicLightDirty = true;
}


// ***************************************************************************

void	CDriverGL3::setAmbientColor (CRGBA color)
{
	H_AUTO_OGL(CDriverGL3_setAmbientColor)
	_AmbientGlobal = color;
}


// ***************************************************************************
void			CDriverGL3::setLightMapDynamicLight (bool enable, const CLight& light)
{
	H_AUTO_OGL(CDriverGL3_setLightMapDynamicLight)
	// just store, for future setup in lightmap material rendering
	_LightMapDynamicLightEnabled= enable;
	_LightMapDynamicLight= light;
	_LightMapDynamicLightDirty= true;
}


// ***************************************************************************
void			CDriverGL3::setupLightMapDynamicLighting(bool enable)
{
	H_AUTO_OGL(CDriverGL3_setupLightMapDynamicLighting)

	// start lightmap dynamic lighting
	if (enable)
	{
		// disable all lights but the 0th.
		for (uint i = 1; i < MaxLight; ++i)
			enableLightInternal(i, false);

		// if the dynamic light is really enabled
		if (_LightMapDynamicLightEnabled)
		{
			// then setup and enable
			setLightInternal(0, _LightMapDynamicLight);
			enableLightInternal(0, true);
		}
		// else just disable also the light 0
		else
		{
			enableLightInternal(0, false);
		}

		// In table mode, uploadObjectUBO() reads _LightTableObjIndices instead of
		// _LightEnable/_UserLight. Register the dynamic light in the table so the
		// UBO path can reference it.
		if (_LightTableMode)
		{
			// Clear all per-object table slots
			for (uint i = 0; i < MaxLight; ++i)
			{
				_LightTableObjIndices[i] = -1;
				_LightTableObjFactors[i] = 0.0f;
			}

			if (_LightMapDynamicLightEnabled)
			{
				// Lazily register the dynamic light in the table (once per table batch)
				if (_LightMapDynLightTableIndex < 0)
				{
					_LightMapDynLightTableIndex = (sint16)_LightTable.size();
					_LightTable.push_back(_LightMapDynamicLight);
					_LightTableDirty = true;
				}
				else
				{
					// Update the existing table entry (light may have changed)
					_LightTable[_LightMapDynLightTableIndex] = _LightMapDynamicLight;
					_LightTableDirty = true;
				}

				_LightTableObjIndices[0] = _LightMapDynLightTableIndex;
				_LightTableObjFactors[0] = 1.0f;
			}
		}

		// ok it has been setup
		_LightMapDynamicLightDirty = false;
	}
	// restore old lighting
	else
	{
		// restore the light 0
		setLightInternal(0, _UserLight0);

		// restore all standard light enable states from _UserLightEnable
		// (which is never modified by enableLightInternal, only by the public enableLight API)
		for (uint i = 0; i < MaxLight; ++i)
			enableLightInternal(i, _UserLightEnable[i]);
	}
}

// ***************************************************************************
// CPU-side struct matching the std140 NlLightInfo layout (96 bytes)
struct CLightTableEntry
{
	float dirOrPos[3];     // 0
	sint32 mode;           // 12
	float diffuse[4];      // 16
	float specular[4];     // 32
	float constAttn;       // 48
	float linAttn;         // 52
	float quadAttn;        // 56
	float spotExp;         // 60
	float spotDir[3];      // 64
	float spotCutoff;      // 76
	float ambient[4];      // 80
};                         // 96 bytes
static_assert(sizeof(CLightTableEntry) == 96, "CLightTableEntry must match std140 NlLightInfo layout");

static void packLightToEntry(CLightTableEntry &e, const CLight &light)
{
	CLight::TLightMode lmode = light.getMode();
	e.mode = (sint32)lmode;

	if (lmode == CLight::DirectionalLight)
	{
		CVector dir = light.getDirection();
		e.dirOrPos[0] = dir.x;
		e.dirOrPos[1] = dir.y;
		e.dirOrPos[2] = dir.z;
	}
	else
	{
		CVector pos = light.getPosition();
		e.dirOrPos[0] = pos.x;
		e.dirOrPos[1] = pos.y;
		e.dirOrPos[2] = pos.z;
	}

	NLMISC::CRGBAF diff(light.getDiffuse());
	e.diffuse[0] = diff.R;
	e.diffuse[1] = diff.G;
	e.diffuse[2] = diff.B;
	e.diffuse[3] = diff.A;

	NLMISC::CRGBAF spec(light.getSpecular());
	e.specular[0] = spec.R;
	e.specular[1] = spec.G;
	e.specular[2] = spec.B;
	e.specular[3] = spec.A;

	e.constAttn = light.getConstantAttenuation();
	e.linAttn = light.getLinearAttenuation();
	e.quadAttn = light.getQuadraticAttenuation();
	e.spotExp = light.getExponent();

	CVector spotDir = light.getDirection();
	e.spotDir[0] = spotDir.x;
	e.spotDir[1] = spotDir.y;
	e.spotDir[2] = spotDir.z;
	e.spotCutoff = cosf(light.getCutoff());

	NLMISC::CRGBAF amb(light.getAmbiant());
	e.ambient[0] = amb.R;
	e.ambient[1] = amb.G;
	e.ambient[2] = amb.B;
	e.ambient[3] = amb.A;
}

static void uploadLightUBOData(const CLightTableEntry *entries, sint count, GLuint uboId, sint &uboCapacity)
{
	GLsizeiptr dataSize = count * sizeof(CLightTableEntry);
	nglBindBuffer(GL_UNIFORM_BUFFER, uboId);

	if (count > uboCapacity)
	{
		// Grow buffer
		nglBufferData(GL_UNIFORM_BUFFER, dataSize, entries, GL_STREAM_DRAW);
		uboCapacity = count;
	}
	else
	{
		// Orphan + rewrite (avoids GPU sync)
		nglBufferData(GL_UNIFORM_BUFFER, uboCapacity * sizeof(CLightTableEntry), NULL, GL_STREAM_DRAW);
		nglBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, entries);
	}

	nglBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind to the light table binding point
	nglBindBufferBase(GL_UNIFORM_BUFFER, NL_BUILTIN_LIGHT_TABLE_BINDING, uboId);
}

void CDriverGL3::uploadLightTableUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadLightTableUBO)

	if (!_LightTableUBOId)
		return;

	if (_LightTableMode)
	{
		// Light table mode: upload from _LightTable[]
		if (!_LightTableDirty)
			return;

		sint count = (sint)_LightTable.size();
		if (count == 0)
		{
			_LightTableDirty = false;
			return;
		}

		// Cap at max UBO size (128 lights × 96 bytes = 12288, fits in GL 3.3 min 16KB)
		const sint maxLights = 128;
		if (count > maxLights)
			count = maxLights;

		std::vector<CLightTableEntry> entries(count);
		for (sint i = 0; i < count; ++i)
			packLightToEntry(entries[i], _LightTable[i]);

		uploadLightUBOData(&entries[0], count, _LightTableUBOId, _LightTableUBOCapacity);
		_LightTableDirty = false;
	}
	else
	{
		// Non-table mode: upload _UserLight[0..MaxLight-1] as a small 8-entry UBO
		if (!_UserLightUBODirty)
			return;

		CLightTableEntry entries[MaxLight];
		for (uint i = 0; i < MaxLight; ++i)
			packLightToEntry(entries[i], _UserLight[i]);

		uploadLightUBOData(entries, MaxLight, _LightTableUBOId, _LightTableUBOCapacity);
		_UserLightUBODirty = false;
	}
}

// ***************************************************************************
// Camera UBO data layout (std140, 240 bytes, matches GLSL NlCamera block)
struct CCameraUBOData
{
	float viewMatrix[16];    // 64
	float fogColor[4];       // 16
	float pzbCameraPos[3];   // 12
	float fogDensity;        //  4
	float fogParams[2];      //  8
	sint32 fogMode;          //  4
	sint32 clipPlaneMask;    //  4
	float clipPlane[6][4];   // 96
	float cameraWorldPos[3]; // 12  (inverse view translation: actual camera world position)
	float _pad0;             //  4
};                           // 224
static_assert(sizeof(CCameraUBOData) == 224, "Camera UBO layout mismatch");

void CDriverGL3::uploadCameraUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadCameraUBO)

	if (!_CameraUBOId || !_CameraUBODirty)
		return;

	CCameraUBOData data;

	// View matrix (column-major, CMatrix::get() returns column-major)
	const float *vm = _ViewMtx.get();
	memcpy(data.viewMatrix, vm, 16 * sizeof(float));

	// Fog color
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
	data.clipPlaneMask = (sint32)m_VPBuiltinCurrent.ClipPlaneMask;

	// Clip planes
	for (uint i = 0; i < MaxClipPlanes; ++i)
		memcpy(data.clipPlane[i], _ClipPlaneEye[i], 4 * sizeof(float));

	// Camera world position: -transpose(mat3(V)) * V[3]
	// With PZB: V[3]=0 → (0,0,0). Without PZB: actual camera world position.
	{
		CMatrix invView = _ViewMtx;
		invView.invert();
		CVector cwp = invView.getPos();
		data.cameraWorldPos[0] = cwp.x;
		data.cameraWorldPos[1] = cwp.y;
		data.cameraWorldPos[2] = cwp.z;
		data._pad0 = 0.f;
	}

	// Upload
	const GLsizeiptr dataSize = sizeof(CCameraUBOData);
	nglBindBuffer(GL_UNIFORM_BUFFER, _CameraUBOId);

	if (_CameraUBOCapacity < (sint)dataSize)
	{
		nglBufferData(GL_UNIFORM_BUFFER, dataSize, &data, GL_STREAM_DRAW);
		_CameraUBOCapacity = (sint)dataSize;
	}
	else
	{
		// Orphan + rewrite
		nglBufferData(GL_UNIFORM_BUFFER, _CameraUBOCapacity, NULL, GL_STREAM_DRAW);
		nglBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, &data);
	}

	nglBindBuffer(GL_UNIFORM_BUFFER, 0);
	nglBindBufferBase(GL_UNIFORM_BUFFER, NL_BUILTIN_CAMERA_BINDING, _CameraUBOId);

	_CameraUBODirty = false;
}

// ***************************************************************************
void CDriverGL3::enableClipPlane(uint index, bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableClipPlane)

	if (index >= MaxClipPlanes) return;

	_ClipPlaneEnabled[index] = enable;

	// Enable/disable GL clip distance (for builtin VPs that write gl_ClipDistance)
	if (enable)
		glEnable(GL_CLIP_DISTANCE0 + index);
	else
		glDisable(GL_CLIP_DISTANCE0 + index);

	// Trigger VP regeneration to include/exclude gl_ClipDistance output
	touchClipPlaneVP(index, enable);
	_CameraUBODirty = true;
}

// ***************************************************************************
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
// Per-Object UBO data layout (std140, 304 bytes, matches GLSL NlModel block)
struct CObjectUBOData
{
	float modelViewProjection[16]; // 64
	float modelView[16];           // 64
	float normalMatrix[12];        // 48 (3 cols × {x,y,z,pad} for std140 mat3)
	sint32 lightIndices01[4];      // 16
	sint32 lightIndices45[4];      // 16
	float lightFactors01[4];       // 16
	float lightFactors45[4];       // 16
	float selfIllumination[4];     // 16
	sint32 texGenMode[4];          // 16
	sint32 lighting;               // 4
	sint32 vertexColorLighted;     // 4
	sint32 vertexFormat;           // 4
	sint32 worldSpaceNormal;       // 4
	sint32 worldSpacePosition;     // 4
	sint32 numPerPixelLights;      // 4
	sint32 fogEnabled;             // 4
	sint32 _pad[1];                // 4 (pad to 16-byte std140 alignment)
};                                 // 304
static_assert(sizeof(CObjectUBOData) == 304, "Object UBO layout mismatch");

void CDriverGL3::uploadObjectUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadObjectUBO)

	if (!_ObjectUBOId)
		return;

	CMaterial &mat = *_CurrentMaterial;

	CObjectUBOData data;

	// ModelViewProjection
	CMatrix mvp = _GLProjMat * _ChangeBasis * _ModelViewMatrix;
	memcpy(data.modelViewProjection, mvp.get(), 16 * sizeof(float));

	// ModelView
	memcpy(data.modelView, _ModelViewMatrix.get(), 16 * sizeof(float));

	// NormalMatrix (3×3 → std140 mat3 = 3 columns of vec4, padded)
	const float *mv = _ModelViewMatrix.get();
	// Column 0
	data.normalMatrix[0] = mv[0]; data.normalMatrix[1] = mv[1]; data.normalMatrix[2] = mv[2]; data.normalMatrix[3] = 0.0f;
	// Column 1
	data.normalMatrix[4] = mv[4]; data.normalMatrix[5] = mv[5]; data.normalMatrix[6] = mv[6]; data.normalMatrix[7] = 0.0f;
	// Column 2
	data.normalMatrix[8] = mv[8]; data.normalMatrix[9] = mv[9]; data.normalMatrix[10] = mv[10]; data.normalMatrix[11] = 0.0f;

	// Light indices and factors (packed into ivec4 / vec4)
	for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
	{
		sint32 lightIdx;
		float lightFactor;
		if (_LightTableMode)
		{
			lightIdx = (sint32)_LightTableObjIndices[i];
			lightFactor = _LightMapUBOOverride.Active && _LightMapUBOOverride.ZeroLightFactors
				? 0.0f : _LightTableObjFactors[i];
		}
		else
		{
			lightIdx = _LightEnable[i] ? (sint32)i : -1;
			lightFactor = _LightMapUBOOverride.Active && _LightMapUBOOverride.ZeroLightFactors
				? 0.0f : 1.0f;
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
		selfIllumination *= NLMISC::CRGBAF(mat.getAmbient());
		if (mat.getShader() != CMaterial::LightMap)
			selfIllumination += NLMISC::CRGBAF(mat.getEmissive());
		data.selfIllumination[0] = selfIllumination.R;
		data.selfIllumination[1] = selfIllumination.G;
		data.selfIllumination[2] = selfIllumination.B;
		data.selfIllumination[3] = 0.0f;
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
	nglBindBuffer(GL_UNIFORM_BUFFER, _ObjectUBOId);

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

	nglBindBuffer(GL_UNIFORM_BUFFER, 0);
	nglBindBufferBase(GL_UNIFORM_BUFFER, NL_BUILTIN_MODEL_BINDING, _ObjectUBOId);
}

// ***************************************************************************
// Per-Material UBO data layout (std140, 96 bytes, matches GLSL NlMaterial block)
struct CMaterialUBOData
{
	float materialColor[4];        // 16
	float materialDiffuse[4];      // 16
	float materialSpecular[4];     // 16
	float materialShininess;       // 4
	float alphaRef;                // 4
	sint32 nlShader;               // 4
	sint32 nlTextureActive;        // 4
	sint32 nlAlphaTest;            // 4
	uint32 nlTexEnvMode[4];        // 16 (4 separate uint in GLSL, not an array — avoids std140 vec4 padding)
	float nlLightMapScale;         // 4
	sint32 _pad[2];               // 8
};                                 // 96
static_assert(sizeof(CMaterialUBOData) == 96, "Material UBO layout mismatch");

void CDriverGL3::uploadMaterialUBO()
{
	H_AUTO_OGL(CDriverGL3_uploadMaterialUBO)

	CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	if (!matDrv) return;

	// Lightmap multipass: upload to global override buffer, not per-material cache.
	// This avoids corrupting the per-material MaterialUBOId with per-pass overrides
	// (materialDiffuse/Specular change per pass). TextureActive also changes per
	// lightmap pass (via checkDriverMaterialStateTouched) but is read from PPBuiltin.
	if (_LightMapUBOOverride.Active)
	{
		CMaterialUBOData data;

		// Material color (from material)
		CRGBA col = mat.getColor();
		data.materialColor[0] = col.R / 255.0f;
		data.materialColor[1] = col.G / 255.0f;
		data.materialColor[2] = col.B / 255.0f;
		data.materialColor[3] = col.A / 255.0f;

		// Overridden diffuse and specular
		memcpy(data.materialDiffuse, _LightMapUBOOverride.MaterialDiffuse, sizeof(data.materialDiffuse));
		memcpy(data.materialSpecular, _LightMapUBOOverride.MaterialSpecular, sizeof(data.materialSpecular));

		data.materialShininess = mat.getShininess();
		data.alphaRef = mat.getAlphaTestThreshold();

		int shaderInt = 0;
		switch (matDrv->PPBuiltin.Shader)
		{
		case CMaterial::Normal:    shaderInt = 0; break;
		case CMaterial::UserColor: shaderInt = 1; break;
		case CMaterial::Specular:  shaderInt = 2; break;
		case CMaterial::LightMap:  shaderInt = 3; break;
		default:                   shaderInt = 0; break;
		}
		data.nlShader = shaderInt;
		data.nlTextureActive = (sint32)matDrv->PPBuiltin.TextureActive;
		data.nlAlphaTest = (matDrv->PPBuiltin.Flags & IDRV_MAT_ALPHA_TEST) ? 1 : 0;
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			data.nlTexEnvMode[i] = matDrv->PPBuiltin.TexEnvMode[i];
		data.nlLightMapScale = _LightMapUBOOverride.LightMapScale;
		data._pad[0] = 0; data._pad[1] = 0;

		const GLsizeiptr dataSize = sizeof(CMaterialUBOData);
		nglBindBuffer(GL_UNIFORM_BUFFER, _OverrideMaterialUBOId);
		nglBufferData(GL_UNIFORM_BUFFER, dataSize, &data, GL_STREAM_DRAW);
		nglBindBuffer(GL_UNIFORM_BUFFER, 0);
		nglBindBufferBase(GL_UNIFORM_BUFFER, NL_BUILTIN_MATERIAL_BINDING, _OverrideMaterialUBOId);
		return;
	}

	// Per-material cache: skip re-pack if nothing changed since last upload.
	// MaterialUBODirty is set by setupMaterial() (CMaterial property changes) and
	// by PP setup (PPBuiltin.MaterialUBOTouched for shader/flags/textureActive/texEnvMode).
	if (!matDrv->MaterialUBODirty && matDrv->MaterialUBOId)
	{
		// Just bind the existing buffer
		nglBindBufferBase(GL_UNIFORM_BUFFER, NL_BUILTIN_MATERIAL_BINDING, matDrv->MaterialUBOId);
		return;
	}

	// Create buffer if needed
	if (!matDrv->MaterialUBOId)
		nglGenBuffers(1, &matDrv->MaterialUBOId);

	CMaterialUBOData data;

	// Material color
	CRGBA col = mat.getColor();
	data.materialColor[0] = col.R / 255.0f;
	data.materialColor[1] = col.G / 255.0f;
	data.materialColor[2] = col.B / 255.0f;
	data.materialColor[3] = col.A / 255.0f;

	// Material diffuse (raw, not adjusted for vertexColorLighted)
	CRGBA diff = mat.getDiffuse();
	data.materialDiffuse[0] = diff.R / 255.0f;
	data.materialDiffuse[1] = diff.G / 255.0f;
	data.materialDiffuse[2] = diff.B / 255.0f;
	data.materialDiffuse[3] = diff.A / 255.0f;

	// Material specular
	CRGBA spec = mat.getSpecular();
	data.materialSpecular[0] = spec.R / 255.0f;
	data.materialSpecular[1] = spec.G / 255.0f;
	data.materialSpecular[2] = spec.B / 255.0f;
	data.materialSpecular[3] = spec.A / 255.0f;

	// Material shininess
	data.materialShininess = mat.getShininess();

	// Alpha ref
	data.alphaRef = mat.getAlphaTestThreshold();

	// Shader type
	int shaderInt = 0;
	switch (matDrv->PPBuiltin.Shader)
	{
	case CMaterial::Normal:    shaderInt = 0; break;
	case CMaterial::UserColor: shaderInt = 1; break;
	case CMaterial::Specular:  shaderInt = 2; break;
	case CMaterial::LightMap:  shaderInt = 3; break;
	default:                   shaderInt = 0; break;
	}
	data.nlShader = shaderInt;

	// Texture active
	data.nlTextureActive = (sint32)matDrv->PPBuiltin.TextureActive;

	// Alpha test
	data.nlAlphaTest = (matDrv->PPBuiltin.Flags & IDRV_MAT_ALPHA_TEST) ? 1 : 0;

	// TexEnv modes
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		data.nlTexEnvMode[i] = matDrv->PPBuiltin.TexEnvMode[i];

	// Lightmap scale (1.0 default; override path sets per-pass value)
	data.nlLightMapScale = 1.0f;

	// Padding
	data._pad[0] = 0;
	data._pad[1] = 0;

	// Upload
	const GLsizeiptr dataSize = sizeof(CMaterialUBOData);
	nglBindBuffer(GL_UNIFORM_BUFFER, matDrv->MaterialUBOId);
	nglBufferData(GL_UNIFORM_BUFFER, dataSize, &data, GL_STREAM_DRAW);
	nglBindBuffer(GL_UNIFORM_BUFFER, 0);
	nglBindBufferBase(GL_UNIFORM_BUFFER, NL_BUILTIN_MATERIAL_BINDING, matDrv->MaterialUBOId);

	matDrv->MaterialUBODirty = false;
}

} // NLDRIVERGL3
} // NL3D
