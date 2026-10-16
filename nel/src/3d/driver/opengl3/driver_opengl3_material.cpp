// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include "stdopengl3.h"
#include "driver_opengl3.h"
#include "driver_opengl3_uniform_buffer.h"
#include "nel/3d/cube_map_builder.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/texture_bump.h"
#include "nel/3d/material.h"
#include "driver_opengl3_vertex_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

CMaterialDrvInfosGL3::~CMaterialDrvInfosGL3()
{
	CDriverGL3 *drv = static_cast<CDriverGL3 *>(getDriver());
	for (int i = 0; i < (int)MaterialUBOId.size(); ++i)
	{
		if (MaterialUBOId[i])
		{
			if (drv->_DriverGLStates.getCurrBoundUniformBufferBase(NL_BUILTIN_MATERIAL_BINDING) == MaterialUBOId[i])
				drv->_DriverGLStates.forceBindUniformBufferBase(NL_BUILTIN_MATERIAL_BINDING, 0);
			nglDeleteBuffers(1, &MaterialUBOId[i]);
			MaterialUBOId[i] = 0;
		}
	}
}

// Water fragment program GLSL source (4 variants via #defines)
static const char *WaterFPGLSL_Header = "#version 330\n"
                                        "#extension GL_ARB_separate_shader_objects : enable\n";

static const char *WaterFPGLSL_ES_Header = "#version 300 es\n"
                                           "precision highp float;\n"
                                           "precision highp int;\n";

// Water shader uses linear fog only; exp/exp2 fog modes are not supported here
static const char *WaterFPGLSL_Body = "layout(location = 8) smooth in vec4 texCoord0;\n"
                                      "layout(location = 9) smooth in vec4 texCoord1;\n"
                                      "layout(location = 10) smooth in vec4 texCoord2;\n"
                                      "#ifdef USE_DIFFUSE\n"
                                      "layout(location = 11) smooth in vec4 texCoord3;\n"
                                      "#endif\n"
                                      "#ifdef USE_FOG\n"
                                      "layout(location = 0) smooth in vec4 ecPos;\n"
                                      "#endif\n"
                                      "uniform sampler2D sampler0;\n"
                                      "uniform sampler2D sampler1;\n"
                                      "uniform sampler2D sampler2;\n"
                                      "#ifdef USE_DIFFUSE\n"
                                      "uniform sampler2D sampler3;\n"
                                      "#endif\n"
                                      "uniform vec4 bump0ScaleBias;\n"
                                      "uniform vec4 bump1ScaleBias;\n"
                                      "#ifdef USE_FOG\n"
                                      "uniform vec2 fogParams;\n"
                                      "uniform vec4 fogColor;\n"
                                      "#endif\n"
                                      "layout(location = 0) out vec4 fragColor;\n"
                                      "void main()\n"
                                      "{\n"
                                      "  vec2 b0 = texture(sampler0, texCoord0.xy).xy;\n"
                                      "  b0 = b0 * bump0ScaleBias.x + bump0ScaleBias.y;\n"
                                      "  vec2 uv1 = texCoord1.xy + b0;\n"
                                      "  vec2 b1 = texture(sampler1, uv1).xy;\n"
                                      "  b1 = b1 * bump1ScaleBias.x + bump1ScaleBias.y;\n"
                                      "  vec2 uv2 = texCoord2.xy + b1;\n"
                                      "  vec4 col = texture(sampler2, uv2);\n"
                                      "#ifdef USE_DIFFUSE\n"
                                      "  col *= texture(sampler3, texCoord3.xy);\n"
                                      "#endif\n"
                                      "#ifdef USE_FOG\n"
                                      "  float z = abs(ecPos.y / ecPos.w);\n"
                                      "  float fogFactor = clamp((fogParams.t - z) / (fogParams.t - fogParams.s), 0.0, 1.0);\n"
                                      "  col = vec4(mix(fogColor.rgb, col.rgb, fogFactor), col.a);\n"
                                      "#endif\n"
                                      "  fragColor = col;\n"
                                      "}\n";

// UBO-based water FP body: uses NlCamera UBO for fog, NlWater user UBO for bump params.
// Fog/diffuse variants are folded: runtime fog from camera UBO, diffuse via #define.
static const char *WaterFPGLSL_UBO_Body = "smooth in vec4 texCoord0;\n"
                                          "smooth in vec4 texCoord1;\n"
                                          "smooth in vec4 texCoord2;\n"
                                          "#ifdef USE_DIFFUSE\n"
                                          "smooth in vec4 texCoord3;\n"
                                          "#endif\n"
                                          "smooth in vec4 ecPos;\n"
                                          "uniform sampler2D sampler0;\n"
                                          "uniform sampler2D sampler1;\n"
                                          "uniform sampler2D sampler2;\n"
                                          "#ifdef USE_DIFFUSE\n"
                                          "uniform sampler2D sampler3;\n"
                                          "#endif\n"
                                          "// bump0ScaleBias and bump1ScaleBias from NlWater user UBO\n"
                                          "// fogParams and fogColor from NlCamera UBO\n"
                                          "layout(location = 0) out vec4 fragColor;\n"
                                          "void main()\n"
                                          "{\n"
                                          "  vec2 b0 = texture(sampler0, texCoord0.xy).xy;\n"
                                          "  b0 = b0 * bump0ScaleBias.x + bump0ScaleBias.y;\n"
                                          "  vec2 uv1 = texCoord1.xy + b0;\n"
                                          "  vec2 b1 = texture(sampler1, uv1).xy;\n"
                                          "  b1 = b1 * bump1ScaleBias.x + bump1ScaleBias.y;\n"
                                          "  vec2 uv2 = texCoord2.xy + b1;\n"
                                          "  vec4 col = texture(sampler2, uv2);\n"
                                          "#ifdef USE_DIFFUSE\n"
                                          "  col *= texture(sampler3, texCoord3.xy);\n"
                                          "#endif\n"
                                          "  float z = abs(ecPos.y / ecPos.w);\n"
                                          "  float fogFactor = clamp((fogParams.t - z) / (fogParams.t - fogParams.s), 0.0, 1.0);\n"
                                          "  col = vec4(mix(fogColor.rgb, col.rgb, fogFactor), col.a);\n"
                                          "  fragColor = col;\n"
                                          "}\n";

static void convBlend(CMaterial::TBlend blend, GLenum &glenum)
{
	H_AUTO_OGL(convBlend)
	switch (blend)
	{
	case CMaterial::one: glenum = GL_ONE; break;
	case CMaterial::zero: glenum = GL_ZERO; break;
	case CMaterial::srcalpha: glenum = GL_SRC_ALPHA; break;
	case CMaterial::invsrcalpha: glenum = GL_ONE_MINUS_SRC_ALPHA; break;
	case CMaterial::srccolor: glenum = GL_SRC_COLOR; break;
	case CMaterial::invsrccolor: glenum = GL_ONE_MINUS_SRC_COLOR; break;

	// Extended Blend modes.
	case CMaterial::blendConstantColor: glenum = GL_CONSTANT_COLOR_EXT; break;
	case CMaterial::blendConstantInvColor: glenum = GL_ONE_MINUS_CONSTANT_COLOR_EXT; break;
	case CMaterial::blendConstantAlpha: glenum = GL_CONSTANT_ALPHA_EXT; break;
	case CMaterial::blendConstantInvAlpha: glenum = GL_ONE_MINUS_CONSTANT_ALPHA_EXT; break;

	default: nlstop;
	}
}

static void convZFunction(CMaterial::ZFunc zfunc, GLenum &glenum)
{
	H_AUTO_OGL(convZFunction)
	switch (zfunc)
	{
	case CMaterial::lessequal: glenum = GL_LEQUAL; break;
	case CMaterial::less: glenum = GL_LESS; break;
	case CMaterial::always: glenum = GL_ALWAYS; break;
	case CMaterial::never: glenum = GL_NEVER; break;
	case CMaterial::equal: glenum = GL_EQUAL; break;
	case CMaterial::notequal: glenum = GL_NOTEQUAL; break;
	case CMaterial::greater: glenum = GL_GREATER; break;
	case CMaterial::greaterequal: glenum = GL_GEQUAL; break;
	default: nlstop;
	}
}

static void convColor(CRGBA col, GLfloat glcol[4])
{
	H_AUTO_OGL(convColor)
	static const float OO255 = 1.0f / 255;
	glcol[0] = col.R * OO255;
	glcol[1] = col.G * OO255;
	glcol[2] = col.B * OO255;
	glcol[3] = col.A * OO255;
}

static CMaterialDrvInfosGL3 *getMatDrv(const CMaterial &mat)
{
	return static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
}

static bool isSinglePass(CMaterial::TShader shader)
{
	switch (shader)
	{
	case CMaterial::Normal: // always single-pass, can skip setupPass
	case CMaterial::UserColor: // always single-pass, can skip setupPass
	case CMaterial::Specular: // single-pass in this driver, for correctness setupPass must be called as well
		return true;
	}
	return false;
}

// --------------------------------------------------
void CDriverGL3::setTexGenFunction(uint stage, CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL3_setTexGenFunction)
	ITexture *text = mat.getTexture(uint8(stage));
	if (text)
	{
		if (mat.getTexCoordGen(stage))
		{
			// set mode and enable.
			CMaterial::TTexCoordGenMode mode = mat.getTexCoordGenMode(stage);
			if (mode == CMaterial::TexCoordGenReflect)
			{
				// Cubic or normal ?
				if (text->isTextureCube())
					setTexGenModeVP(stage, TexGenReflectionMap);
				else
					setTexGenModeVP(stage, TexGenSphereMap);
			}
			else if (mode == CMaterial::TexCoordGenObjectSpace)
			{
				setTexGenModeVP(stage, TexGenObjectLinear);
			}
			else if (mode == CMaterial::TexCoordGenEyeSpace)
			{
				setTexGenModeVP(stage, TexGenEyeLinear);
			}
		}
		else
		{
			// Disable.
			setTexGenModeVP(stage, TexGenDisabled);
		}
	}
}

// --------------------------------------------------
CMaterial::TShader CDriverGL3::getSupportedShader(CMaterial::TShader shader)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	switch (shader) // Normal, UserColor, LightMap, Specular, Water
	{
	case CMaterial::Normal: // fixed pipeline texenv setup
		return CMaterial::Normal;
	// case CMaterial::Bump: // legacy bump technique, deprecated
	case CMaterial::UserColor:
		return CMaterial::UserColor; // similar to "normal" but trades a texture stage for a color constant
	case CMaterial::LightMap:
		return CMaterial::LightMap; // lighmap reflections
	case CMaterial::Specular:
		return CMaterial::Specular; // cubemap reflections
	// case CMaterial::Caustics: // not implemented anywhere, deprecated
	// case CMaterial::PerPixelLighting: // legacy ppl technique, deprecated
	// case CMaterial::PerPixelLightingNoSpec:
	// case CMaterial::Cloud: // fluffy snowballs clouds, deprecated
	case CMaterial::Water:
		return CMaterial::Water;
	default:
		return CMaterial::Normal;
	}
}

// --------------------------------------------------
bool CDriverGL3::setupMaterial(CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL3_setupMaterial)

	CMaterialDrvInfosGL3 *matDrv;
	CMaterial::TShader matShader;
	GLenum glenum = GL_ZERO;
	uint32 matTouched = mat.getTouched();

	// profile.
	_NbSetupMaterialCall++;

	// 0. Retrieve/Create driver shader.
	//==================================
	if (!mat._MatDrvInfo)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItMatDrvInfoPtrList it = _MatDrvInfos.insert(_MatDrvInfos.end(), (NL3D::IMaterialDrvInfos *)NULL);
		// create and set iterator, for future deletion.
		*it = mat._MatDrvInfo = new CMaterialDrvInfosGL3(this, it);

		// Must create all OpenGL shader states.
		matTouched = IDRV_TOUCHED_ALL;
	}
	matDrv = getMatDrv(mat);
	nlassert(matDrv->getDriver() == this);
	const uint32 touched = matTouched | matDrv->SetupMaterialTouched;

	// 1. Setup modified fields of material conversion cache
	//=====================================
	if (touched)
	{
		// Forward CMaterial::getTouched to setupPass
		matDrv->SetupPass0Touched |= matTouched;

		static const uint32 touchedConvState = IDRV_TOUCHED_BLENDFUNC | IDRV_TOUCHED_ZFUNC;
		static const uint32 touchedUBOState = IDRV_TOUCHED_SHADER | IDRV_TOUCHED_COLOR | IDRV_TOUCHED_LIGHTING
		    | IDRV_TOUCHED_ALPHA_TEST_THRE | IDRV_TOUCHED_ALPHA_TEST; // Flags touched on material UBO at the end
		static const uint32 touchedBuiltinPPState = IDRV_TOUCHED_SHADER | IDRV_TOUCHED_ALPHA_TEST; // Flags touched on PPBuiltin at the end
		if (touched & (touchedConvState | touchedUBOState | touchedBuiltinPPState))
		{
			CMaterialUBOData &matUBO = matDrv->MaterialUBO[0];
			CMaterial::TShader shader = getSupportedShader(mat.getShader());
			if (touched & IDRV_TOUCHED_SHADER)
			{
				matDrv->PPBuiltin.Shader = shader;
				matDrv->HasEMBM = false; // Reset shader-specific feature flags
				matDrv->MaterialUBOCurrent = 0;
				matDrv->SetupPass0Touched = IDRV_TOUCHED_ALL; // any downstream pass needs to be fully reset
				matUBO.nlShader = (sint32)shader;
				if (shader == CMaterial::Normal || shader == CMaterial::UserColor)
				{
					// TexEnv shader: clear flag so setupNormalPass writes the real values
					matDrv->TexEnvMatDefault = false;
				}
				else if (!matDrv->TexEnvMatDefault)
				{
					// Non-TexEnv shaders (LightMap, Specular, etc.) don't use texenv modes,
					// constants, EMBM, or texture matrices from the material UBO. Set them
					// to safe defaults so stale data from a prior Normal/UserColor usage of
					// the same CMaterialDrvInfosGL3 doesn't leak through.
					// PPBuiltin and UBO are already marked touched because IDRV_TOUCHED_SHADER.
					CMaterial::CTexEnv defaultEnv;
					defaultEnv.setDefault(); // Modulate(Texture, Previous), white constant
					for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
					{
						matDrv->PPBuiltin.TexEnvMode[stage] = defaultEnv.EnvPacked;
						matUBO.nlTexEnvMode[stage] = defaultEnv.EnvPacked;
						convColor(defaultEnv.ConstantColor, matUBO.constant[stage]);
					}
					// Zero EMBM / lightmap factor slots (constant[4-7])
					memset(&matUBO.constant[IDRV_MAT_MAXTEXTURES], 0,
					    IDRV_MAT_MAXTEXTURES * 4 * sizeof(float));
					// Identity texture matrices
					for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
						memcpy(matUBO.texMatrix[stage], CMatrix::Identity.get(), 16 * sizeof(float));
					matDrv->TexEnvMatDefault = true;
				}
			}

			// Convert Material to driver shader.
			if (touched & IDRV_TOUCHED_BLENDFUNC)
			{
				convBlend(mat.getSrcBlend(), glenum);
				matDrv->SrcBlend = glenum; // Driver state
				convBlend(mat.getDstBlend(), glenum);
				matDrv->DstBlend = glenum; // Driver state
			}
			if (touched & IDRV_TOUCHED_ZFUNC)
			{
				convZFunction(mat.getZFunc(), glenum);
				matDrv->ZComp = glenum; // Driver state
			}
			if (touched & IDRV_TOUCHED_COLOR)
			{
				convColor(mat.getColor(), matUBO.materialColor);
			}
			if (touched & IDRV_TOUCHED_LIGHTING)
			{
				matDrv->Ambient = NLMISC::CRGBAF(mat.getAmbient()); // Object UBO
				matDrv->Emissive = NLMISC::CRGBAF(mat.getEmissive()); // Object UBO
				convColor(mat.getDiffuse(), matUBO.materialDiffuse);
				convColor(mat.getSpecular(), matUBO.materialSpecular);
				matUBO.materialShininess = mat.getShininess();
			}
			if (touched & IDRV_TOUCHED_ALPHA_TEST_THRE)
			{
				matUBO.alphaRef = mat.getAlphaTestThreshold();
			}
			if (touched & IDRV_TOUCHED_ALPHA_TEST)
			{
				uint32 flags = mat.getFlags() & IDRV_MAT_ALPHA_TEST;
				matDrv->PPBuiltin.Flags = flags;
				matUBO.nlAlphaTest = flags ? 1 : 0;
			}
			// TexEnv, EMBM, and Texture matrices are handled in the setupNormalPass
		}

		if (touched & touchedUBOState)
			matDrv->MaterialUBOTouched[0] = true;
		if (touched & touchedBuiltinPPState)
			matDrv->PPBuiltin.Touched = true;

		matDrv->SetupMaterialTouched = 0;
		mat.clearTouched(IDRV_TOUCHED_ALL);
	}

	// 2b. User supplied pixel shader overrides material
	//==================================
	// Now we can get the supported shader from the cache.
	matShader = matDrv->PPBuiltin.Shader;

	// 2b. Update more shader state
	//==================================
	// if the shader has changed since last time
	if (matShader != _CurrentMaterialSupportedShader)
	{
		// if old was lightmap, restore standard lighting
		if (_CurrentMaterialSupportedShader == CMaterial::LightMap)
			setupLightMapDynamicLighting(false);

		// if new is lightmap, setup dynamic lighting
		if (matShader == CMaterial::LightMap)
			setupLightMapDynamicLighting(true);
	}

	// setup the global
	_CurrentMaterialSupportedShader = matShader;

	// 2. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame, even when the same CMaterial applied. (need to test if touched).
	// Must separate texture setup and texture activation in 2 "for"...
	// because setupTexture() may disable all stage.
	bool textureFailed = false;
	if (matShader != CMaterial::Water && matShader != CMaterial::Program)
	{
		for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		{
			ITexture *text = mat.getTexture(uint8(stage));
			if (text != NULL && !setupTexture(*text))
				textureFailed = true;
		}
	}
	// Here, for Lightmap materials, setup the lightmaps.
	if (matShader == CMaterial::LightMap)
	{
		for (uint stage = 0; stage < mat._LightMaps.size(); stage++)
		{
			ITexture *text = mat._LightMaps[stage].Texture;
			if (text != NULL && !setupTexture(*text))
				textureFailed = true;
		}
	}

	// NOTE: A vertex buffer MUST be enabled before calling setupMaterial!
	nlassert(_CurrentVertexBufferGL);
	uint16 vertexFormat = _CurrentVertexBufferGL->VB->getVertexFormat();

	// Activate the textures.
	// Do not do it for Lightmap and Water, because done in multipass in a very special fashion.
	// This avoids the useless multiple change of texture states per lightmapped object.
	if (matShader != CMaterial::LightMap
	    && matShader != CMaterial::Water)
	{
		uint maxTex = matShader == CMaterial::Specular ? 2 : IDRV_MAT_MAXTEXTURES;
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			ITexture *text = mat.getTexture(uint8(stage));

			// activate the texture, or disable texturing if NULL.
			activateTexture(stage, text);

			if (text && mat.getTexCoordGen(stage))
			{
				// Material explicitly requests texgen — use it even if VB has texcoords
				setTexGenFunction(stage, mat);
			}
			else
			{
				// No texgen requested — disable it (prevents stale texgen from previous materials)
				setTexGenModeVP(stage, TexGenDisabled);
			}
		}
	}
	else // LightMap, Water — main loop skipped, so reset all texgen modes
	{
		for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		{
			setTexGenModeVP(stage, TexGenDisabled);
		}
	}

	if (matShader == CMaterial::Specular)
	{
		setTexGenModeVP(0, TexGenDisabled);
		if (vertexFormat & g_VertexFlags[TexCoord1])
		{
			// nlwarning("GL3: Specular material with TexCoord1 provided in vertex buffer");
			setTexGenModeVP(1, TexGenDisabled);
		}
		else
		{
			ITexture *text = mat.getTexture(1);
			if (text)
			{
				if (text->isTextureCube())
					setTexGenModeVP(1, TexGenReflectionMap);
				else
					setTexGenModeVP(1, TexGenSphereMap);
			}
			else
			{
				setTexGenModeVP(1, TexGenDisabled);
			}
		}
	}
	static const uint32 touchedGLState = IDRV_TOUCHED_BLENDFUNC | IDRV_TOUCHED_ZFUNC | IDRV_TOUCHED_BLEND
		| IDRV_TOUCHED_DOUBLE_SIDED | IDRV_TOUCHED_ZWRITE | IDRV_TOUCHED_ZBIAS | IDRV_TOUCHED_LIGHTING;

	// 3. Bind OpenGL States.
	//=======================
	if ((touched & touchedGLState) || (_CurrentMaterial != &mat))
	{
		// Bind Blend Part.
		//=================
		bool blend = (mat.getFlags() & IDRV_MAT_BLEND) != 0;
		_DriverGLStates.enableBlend(blend);
		if (blend)
			_DriverGLStates.blendFunc(matDrv->SrcBlend, matDrv->DstBlend);

		// Double Sided Part.
		//===================
		// NB: inverse state: DoubleSided <=> !CullFace.
		uint32 twoSided = mat.getFlags() & IDRV_MAT_DOUBLE_SIDED;
		_DriverGLStates.enableCullFace(twoSided == 0);

		// Bind ZBuffer Part.
		//===================
		_DriverGLStates.enableZWrite(mat.getFlags() & IDRV_MAT_ZWRITE);
		_DriverGLStates.depthFunc(matDrv->ZComp);
		_DriverGLStates.setZBias(mat.getZBias() * _OODeltaZ);

		// Color-Lighting Part.
		//=====================
		// Light Part.
		enableLightingVP(mat.getFlags() & IDRV_MAT_LIGHTING);
		setVertexColorLightedVP(mat.isLightedVertexColor());

		// Fog Part.
		//=================
		enableFogVP(_FogEnabled);

		// Use black fog color for additive blend to avoid brightening the scene with fog
		{
			bool fogOverride = (blend && (matDrv->DstBlend == GL_ONE) && _FogEnabled);
			if (_FogColorOverrideBlack != fogOverride)
			{
				_FogColorOverrideBlack = fogOverride;
				_CameraUBODirty = true;
			}
		}

		// Done.
		_CurrentMaterial = &mat;
	}

	// 4. Misc
	//=====================================
	// Single-pass materials can be used without going through the multi-pass loop.
	// Texture matrices are read directly from the material in setupNormalMaterial.
	// Specular _SpecularTexMtx is in the camera UBO.
	switch (matShader)
	{
	case CMaterial::Normal:
	case CMaterial::UserColor:
		setupNormalMaterial();
		break;
	case CMaterial::Specular:
		setupSpecularMaterial();
		break;
	}

	// Programs are set up per-pass in setupPass() after per-pass staging is complete.
	if (!isSinglePass(matShader) || setupBuiltinPrograms())
		return !textureFailed;
	return false;
}

// ***************************************************************************
sint CDriverGL3::beginMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginMultiPass)

	// Depending on material type and hardware, return number of pass required to draw this material.
	switch (_CurrentMaterialSupportedShader)
	{
	case CMaterial::LightMap:
		return beginLightMapMultiPass();
	case CMaterial::Water:
		return beginWaterMultiPass();
	default:
		return 1;
	}
}

// ***************************************************************************
bool CDriverGL3::setupPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupPass)

	// 1. Per-pass staging: each pass function stages textures, MaterialUBO slots,
	//    PPBuiltin config, and override state (e.g. _LightMapUBOOverride, _FogColorOverrideBlack).
	switch (_CurrentMaterialSupportedShader)
	{
	case CMaterial::LightMap:
		setupLightMapPass(pass);
		break;
	case CMaterial::Water:
		setupWaterPass(pass);
		break;
	}

	// 2. Select/link programs and upload all uniforms and UBOs.
	return isSinglePass(_CurrentMaterialSupportedShader) || setupBuiltinPrograms();
}

// ***************************************************************************
void CDriverGL3::endMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endMultiPass)

	switch (_CurrentMaterialSupportedShader)
	{
	case CMaterial::LightMap:
		endLightMapMultiPass();
		break;
	case CMaterial::Water:
		endWaterMultiPass();
		return;
	default: return;
	}
}

void CDriverGL3::setupNormalMaterial()
{
	const CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = getMatDrv(mat);
	matDrv->MaterialUBOCurrent = 0;
	const uint32 touched = matDrv->SetupPass0Touched;

	static const uint32 touchedPP = IDRV_TOUCHED_ALLTEX | IDRV_TOUCHED_TEXENV;
	static const uint32 touchedUBO = IDRV_TOUCHED_ALLTEX | IDRV_TOUCHED_TEXENV | IDRV_TOUCHED_TEXMAT;
	if (touched & (touchedPP | touchedUBO))
	{
		CMaterial::TShader shader = matDrv->PPBuiltin.Shader;
		CMaterialUBOData &matUBO = matDrv->MaterialUBO[0];
		uint maxTex = maxTextures(shader);

		// --- PPBuiltin state: TextureActive, TexSamplerMode, TexEnvMode ---

		if (touched & IDRV_TOUCHED_ALLTEX)
		{
			uint32 textureActive = 0;
			uint64 texSamplerMode = 0;
			for (uint stage = 0; stage < maxTex; ++stage)
			{
				NL3D::ITexture *tex = mat._Textures[stage];
				if (tex)
				{
					textureActive |= (1 << stage);
					texSamplerMode |= (tex->isTextureCube() ? SamplerCube : Sampler2D) << (stage * 2);
				}
			}
			matDrv->PPBuiltin.TextureActive = textureActive;
			matDrv->PPBuiltin.TexSamplerMode = texSamplerMode;
			matUBO.nlTextureActive = textureActive;
		}

		if (touched & IDRV_TOUCHED_TEXENV)
		{
			bool hasEMBM = false;
			for (uint stage = 0; stage < maxTex; ++stage)
			{
				matDrv->PPBuiltin.TexEnvMode[stage] = mat._TexEnvs[stage].EnvPacked;
				if ((mat._TexEnvs[stage].EnvPacked & 0xF) == CMaterial::EMBM)
					hasEMBM = true;
			}
			matDrv->HasEMBM = hasEMBM;
		}

		// --- Material UBO: TexEnv block ---

		if (touched & IDRV_TOUCHED_TEXENV)
		{
			// TexEnvMode (packed uint per stage)
			for (uint stage = 0; stage < maxTex; ++stage)
				matUBO.nlTexEnvMode[stage] = matDrv->PPBuiltin.TexEnvMode[stage];

			// TexEnv constant colors (from material)
			for (uint stage = 0; stage < maxTex; ++stage)
				convColor(mat._TexEnvs[stage].ConstantColor, matUBO.constant[stage]);
		}

		// Texture matrices (from material)
		if (touched & IDRV_TOUCHED_TEXMAT)
		{
			uint texMatMask = (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) >> IDRV_MAT_USER_TEX_FIRST_BIT;
			for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
			{
				if (texMatMask & (1 << stage))
					memcpy(matUBO.texMatrix[stage], mat.getUserTexMat(stage).get(), 16 * sizeof(float));
				else
					memcpy(matUBO.texMatrix[stage], CMatrix::Identity.get(), 16 * sizeof(float));
			}
		}

		// Flag dirty
		if (touched & touchedPP)
			matDrv->PPBuiltin.Touched = true;
		matDrv->MaterialUBOTouched[0] = true;
	}

	// EMBM matrices (driver state, no touch flag — compare and update)
	// Stored in constant[4-7], shared with lightmap factors (mutually exclusive by shader type)
	if (matDrv->HasEMBM)
	{
		CMaterialUBOData &matUBO = matDrv->MaterialUBO[0];
		if (memcmp(&matUBO.constant[IDRV_MAT_MAXTEXTURES], _EMBMMatrix, sizeof(_EMBMMatrix)) != 0)
		{
			memcpy(&matUBO.constant[IDRV_MAT_MAXTEXTURES], _EMBMMatrix, sizeof(_EMBMMatrix));
			matDrv->MaterialUBOTouched[0] = true;
		}
	}

	matDrv->SetupPass0Touched = 0;
}

// ***************************************************************************
void CDriverGL3::setupSpecularMaterial()
{
	H_AUTO_OGL(CDriverGL3_setupSpecularMaterial)

	CMaterialDrvInfosGL3 *matDrv = getMatDrv(*_CurrentMaterial);
	matDrv->MaterialUBOCurrent = 0;
	const uint32 touched = matDrv->SetupPass0Touched;

	if (touched & IDRV_TOUCHED_ALLTEX)
	{
		// Specular uses textures activated earlier in setupMaterial.
		// No texenv, EMBM, or texture matrices — just TextureActive and TexSamplerMode.
		uint maxTex = maxTextures(_CurrentMaterialSupportedShader);
		uint32 textureActive = 0;
		uint64 texSamplerMode = 0;
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			ITexture *tex = _CurrentTexture[stage];
			if (tex)
			{
				textureActive |= (1 << stage);
				texSamplerMode |= (tex->isTextureCube() ? SamplerCube : Sampler2D) << (stage * 2);
			}
		}
		matDrv->PPBuiltin.TextureActive = textureActive;
		matDrv->PPBuiltin.TexSamplerMode = texSamplerMode;
		matDrv->PPBuiltin.Touched = true;
		CMaterialUBOData &matUBO = matDrv->MaterialUBO[0];
		matUBO.nlTextureActive = textureActive;
		matDrv->MaterialUBOTouched[0] = true;
	}

	matDrv->SetupPass0Touched = 0;
}

// ***************************************************************************
void CDriverGL3::computeLightMapInfos(const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL3_computeLightMapInfos)
	static const uint32 RGBMaskPacked = CRGBA(255, 255, 255, 0).getPacked();

	// For optimisation consideration, suppose there is not too much lightmap.
	nlassert(mat._LightMaps.size() <= NL3D_DRV_MAX_LIGHTMAP);

	// Compute number of lightmaps really used (ie factor not NULL), and build the LUT.
	_NLightMaps = 0;
	// For all lightmaps of the material.
	for (uint i = 0; i < mat._LightMaps.size(); ++i)
	{
		// If the lightmap's factor is not null.
		if (mat._LightMaps[i].Factor.getPacked() & RGBMaskPacked)
		{
			_LightMapLUT[_NLightMaps] = i;
			++_NLightMaps;
		}
	}

	// Compute how many pass, according to driver caps.
	_NLightMapPerPass = std::min(_Extensions.MaxFragmentTextureImageUnits, (GLint)IDRV_PROGRAM_MAXSAMPLERS) - 1;

	// Number of pass.
	_NLightMapPass = (_NLightMaps + _NLightMapPerPass - 1) / (_NLightMapPerPass);

	// NB: _NLightMaps==0 means there is no lightmaps at all.
}

// ***************************************************************************
sint CDriverGL3::beginLightMapMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginLightMapMultiPass)

	const CMaterial &mat = *_CurrentMaterial;

	// compute how many lightmap and pass we must process.
	computeLightMapInfos(mat);

	// always enable lighting for lightmap (because of dynamic light)
	enableLightingVP(true);
	setVertexColorLightedVP(false);

	// if the dynamic lightmap light has changed since the last render (should not happen), resetup
	// normal way is that setupLightMapDynamicLighting() is called in setupMaterial() if shader different from prec
	if (_LightMapDynamicLightDirty)
		setupLightMapDynamicLighting(true);

	// Ambient/specular zeroing: In GL3, selfIllumination is overridden per-pass in
	// setupLightMapPass() with the LMC ambient, and specular uniforms are zeroed there too.

	// Ensure per-material UBO arrays have enough slots for each lightmap pass.
	// Slot 0 = base material (from setupMaterial), slots 1..N = per-pass overrides.
	CMaterialDrvInfosGL3 *matDrv = getMatDrv(mat);
	uint numPasses = std::max(_NLightMapPass, (uint)1);
	uint neededSlots = numPasses + 1;
	if (matDrv->MaterialUBO.size() < neededSlots)
	{
		matDrv->MaterialUBO.resize(neededSlots);
		matDrv->MaterialUBOId.resize(neededSlots, 0);
		matDrv->MaterialUBOTouched.resize(neededSlots, false);
	}

	// Manage too if no lightmaps.
	return std::max(_NLightMapPass, (uint)1);
}

// ***************************************************************************
void CDriverGL3::setupLightMapPass(uint pass)
{
	nlassert(getProgram(PixelProgram));
	nlassert(!m_UserPixelProgram);

	H_AUTO_OGL(CDriverGL3_setupLightMapPass)

	// Additive passes (pass > 0) use black fog to avoid adding fog multiple times.
	{
		bool fogOverride = (pass > 0 && _FogEnabled);
		if (_FogColorOverrideBlack != fogOverride)
		{
			_FogColorOverrideBlack = fogOverride;
			_CameraUBODirty = true;
		}
	}

	const CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = getMatDrv(mat);

	// No lightmap or all blacks??, just setup "black texture" for stage 0.
	if (_NLightMaps == 0)
	{
		// nldebug("No lightmaps");

		ITexture *text = mat.getTexture(0);
		activateTexture(0, text);

		// Setup gen tex off
		setTexGenModeVP(0, TexGenDisabled);

		// And disable other stages.
		for (uint stage = 1; stage < IDRV_MAT_MAXTEXTURES; stage++)
		{
			// disable texturing.
			activateTexture(stage, NULL);
		}

		// Stage per-material UBO slot for this pass (slot 1, no-lightmap case)
		{
			uint slot = 1;
			matDrv->MaterialUBOCurrent = slot;
			CMaterialUBOData &matUBO = matDrv->MaterialUBO[slot];
			matUBO = matDrv->MaterialUBO[0]; // Copy base material state

			// White diffuse (material diffuse not applied to dynamic light)
			matUBO.materialDiffuse[0] = 1.0f;
			matUBO.materialDiffuse[1] = 1.0f;
			matUBO.materialDiffuse[2] = 1.0f;
			matUBO.materialDiffuse[3] = 1.0f;
			// Zero specular (lightmaps have no specular)
			memset(matUBO.materialSpecular, 0, sizeof(matUBO.materialSpecular));
			// No x2 scale when no lightmaps
			matUBO.nlLightMapScale = 1.0f;
			// Zero constants (no lightmap factors, also zeros EMBM slots 4-7)
			memset(matUBO.constant, 0, sizeof(matUBO.constant));
			// Identity texture matrices
			for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
				memcpy(matUBO.texMatrix[i], CMatrix::Identity.get(), 16 * sizeof(float));

			// TextureActive from activated textures
			uint maxSam = maxSamplers(matDrv->PPBuiltin.Shader, _Extensions);
			uint32 textureActive = 0;
			uint64 texSamplerMode = 0;
			for (uint stage = 0; stage < maxSam; ++stage)
			{
				ITexture *tex = _CurrentTexture[stage];
				if (tex)
				{
					textureActive |= (1 << stage);
					texSamplerMode |= (tex->isTextureCube() ? SamplerCube : Sampler2D) << (stage * 2);
				}
			}
			matUBO.nlTextureActive = textureActive;
			matDrv->MaterialUBOTouched[slot] = true;

			// Stage PPBuiltin
			matDrv->PPBuiltin.TextureActive = textureActive;
			matDrv->PPBuiltin.TexSamplerMode = texSamplerMode;
			if (matDrv->PPBuiltin.LightMapScale)
				matDrv->PPBuiltin.LightMapScale = false;
			matDrv->PPBuiltin.Touched = true;
		}

		// Object UBO override: selfIllumination = black, no light factor zeroing
		_LightMapUBOOverride.Active = true;
		memset(_LightMapUBOOverride.SelfIllumination, 0, sizeof(_LightMapUBOOverride.SelfIllumination));
		_LightMapUBOOverride.ZeroLightFactors = false;

		// Program setup and override clearing handled by setupPass() after return.
		return;
	}

	nlassert(pass < _NLightMapPass);

	// setup Texture Pass.
	//=========================
	uint lmapId;
	uint nstages;
	lmapId = pass * _NLightMapPerPass; // Nb lightmaps already processed
	// N lightmaps for this pass, plus the texture.
	nstages = std::min(_NLightMapPerPass, _NLightMaps - lmapId) + 1; // at most 4

	// For LMC (lightmap 8Bit compression) compute the total AmbientColor in vertex diffuse
	// need only if standard MulADD version
	uint32 r = 0;
	uint32 g = 0;
	uint32 b = 0;
	// sum only the ambient of lightmaps that will be drawn this pass
	for (uint sa = 0; sa < nstages - 1; ++sa)
	{
		uint wla = _LightMapLUT[lmapId + sa];
		// must mul them by their respective mapFactor too
		CRGBA ambFactor = mat._LightMaps[wla].Factor;
		CRGBA lmcAmb = mat._LightMaps[wla].LMCAmbient;
		r += ((uint32)ambFactor.R * ((uint32)lmcAmb.R + (lmcAmb.R >> 7))) >> 8;
		g += ((uint32)ambFactor.G * ((uint32)lmcAmb.G + (lmcAmb.G >> 7))) >> 8;
		b += ((uint32)ambFactor.B * ((uint32)lmcAmb.B + (lmcAmb.B >> 7))) >> 8;
	}
	r = std::min(r, (uint32)255);
	g = std::min(g, (uint32)255);
	b = std::min(b, (uint32)255);

	// this color will be added to the first lightmap (with help of emissive)
	CRGBA col((uint8)r, (uint8)g, (uint8)b, 255);

	// Lightmap factors
	NLMISC::CRGBAF selfIllumination(col);
	NLMISC::CRGBAF constant[IDRV_PROGRAM_MAXSAMPLERS];

	// setup all stages.
	for (uint stage = 0; stage < std::min(_Extensions.MaxFragmentTextureImageUnits, (GLint)IDRV_PROGRAM_MAXSAMPLERS); ++stage)
	{
		// if must setup a lightmap stage
		if (stage < nstages - 1) // last stage is user texture
		{
			// setup lightMap.
			uint whichLightMap = _LightMapLUT[lmapId];
			// get text and factor.
			ITexture *text = mat._LightMaps[whichLightMap].Texture;
			CRGBA lmapFactor = mat._LightMaps[whichLightMap].Factor;
			// Modulate the factor with LightMap compression Diffuse
			CRGBA lmcDiff = mat._LightMaps[whichLightMap].LMCDiffuse;

			lmapFactor.R = (uint8)(((uint32)lmapFactor.R * ((uint32)lmcDiff.R + (lmcDiff.R >> 7))) >> 8);
			lmapFactor.G = (uint8)(((uint32)lmapFactor.G * ((uint32)lmcDiff.G + (lmcDiff.G >> 7))) >> 8);
			lmapFactor.B = (uint8)(((uint32)lmapFactor.B * ((uint32)lmcDiff.B + (lmcDiff.B >> 7))) >> 8);
			lmapFactor.A = 255;

			activateTexture(stage, text);

			// If texture not NULL, Change texture env fonction.
			//==================================================
			if (text)
			{
				if (stage < IDRV_MAT_MAXTEXTURES)
				{
					// Setup env for texture stage.
					setTexGenModeVP(stage, TexGenDisabled);
				}

				// Setup constant color with Lightmap factor.
				constant[stage] = NLMISC::CRGBAF(lmapFactor);
			}

			// Next lightmap.
			lmapId++;
		}
		else if (stage < nstages)
		{
			// optim: do this only for first pass, and last pass only if stage != nLMapPerPass
			// (meaning not the same stage as preceding passes).
			if (pass == 0 || (pass == _NLightMapPass - 1 && stage != _NLightMapPerPass))
			{
				// activate the texture at last stage.
				ITexture *text = mat.getTexture(0);
				activateTexture(stage, text);

				if (stage < IDRV_MAT_MAXTEXTURES)
				{
					// Setup gen tex off
					setTexGenModeVP(stage, TexGenDisabled);
				}
			}
		}
		else
		{
			// else all other stages are disabled.
			activateTexture(stage, NULL);
		}
	}

	// For x2 mode (legacy MODULATE2X / GL_RGB_SCALE=2):
	// Dynamic light is halved in the VP (via material diffuse 0.5 or halved light colors),
	// selfIllumination and lightmap constants stay at normal scale, and the PP applies
	// a 2x scale to the entire (vertexLighting + lightmapSum) result. This matches the
	// legacy clamping behavior: 2 * clamp(dynLight/2 + lmcAmb, 0, 1) gives effective [0,2].
	float lightMapScale = mat._LightMapsMulx2 ? 2.0f : 1.0f;
	// Material diffuse: white normally, halved for x2 (halves dynamic light in VP)
	float lmDiffuse = 1.0f / lightMapScale;

	// setup blend / lighting.
	//=========================

	// Blend is different if the material is blended or not
	if (!mat.getBlend())
	{
		// Not blended, std case.
		if (pass == 0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(false);
		}
		else if (pass == 1)
		{
			// setup an Additive transparency (only for pass 1, will be kept for successives pass).
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_ONE, GL_ONE);
		}
	}
	else
	{
		/* 1st pass, std alphaBlend. 2nd pass, add to background. Demo:
		    T: texture.
		    l0: lightmap (or group of lightmap) of pass 0.
		    l1: lightmap (or group of lightmap) of pass 1. (same thing with 2,3 etc....)
		    B:	Background.
		    A:	Alpha of texture.

		    finalResult= T*(l0+l1) * A + B * (1-A).

		    We get it in two pass:
		        fint=			T*l0 * A + B * (1-A).
		        finalResult=	T*l1 * A + fint = T*l1 * A + T*l0 * A + B * (1-A)=
		            T* (l0+l1) * A + B * (1-A)
		*/
		if (pass == 0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (pass == 1)
		{
			// setup an Additive transparency (only for pass 1, will be kept for successives pass).
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE);
		}
	}

	// Stage per-material UBO slot for this pass
	{
		uint slot = pass + 1;
		matDrv->MaterialUBOCurrent = slot;
		CMaterialUBOData &matUBO = matDrv->MaterialUBO[slot];
		matUBO = matDrv->MaterialUBO[0]; // Copy base material state

		// Override per-pass fields
		matUBO.materialDiffuse[0] = lmDiffuse;
		matUBO.materialDiffuse[1] = lmDiffuse;
		matUBO.materialDiffuse[2] = lmDiffuse;
		matUBO.materialDiffuse[3] = 1.0f;
		// Zero specular (lightmaps have no specular contribution)
		memset(matUBO.materialSpecular, 0, sizeof(matUBO.materialSpecular));
		// Lightmap scale factor (x2 mode: 2.0, normal: 1.0)
		matUBO.nlLightMapScale = lightMapScale;
		// Stage lightmap factor constants 0-7
		for (int i = 0; i < IDRV_PROGRAM_MAXSAMPLERS; ++i)
		{
			matUBO.constant[i][0] = constant[i].R;
			matUBO.constant[i][1] = constant[i].G;
			matUBO.constant[i][2] = constant[i].B;
			matUBO.constant[i][3] = constant[i].A;
		}
		// No separate EMBM zeroing needed — lightmap factors in constant[4-7] overwrite EMBM slots
		// Identity texture matrices
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			memcpy(matUBO.texMatrix[i], CMatrix::Identity.get(), 16 * sizeof(float));

		// TextureActive from activated textures
		uint maxSam = maxSamplers(matDrv->PPBuiltin.Shader, _Extensions);
		uint32 textureActive = 0;
		uint64 texSamplerMode = 0;
		for (uint stage = 0; stage < maxSam; ++stage)
		{
			ITexture *tex = _CurrentTexture[stage];
			if (tex)
			{
				textureActive |= (1 << stage);
				texSamplerMode |= (tex->isTextureCube() ? SamplerCube : Sampler2D) << (stage * 2);
			}
		}
		matUBO.nlTextureActive = textureActive;
		matDrv->MaterialUBOTouched[slot] = true;

		// Stage PPBuiltin
		matDrv->PPBuiltin.TextureActive = textureActive;
		matDrv->PPBuiltin.TexSamplerMode = texSamplerMode;
		bool needLmScale = (lightMapScale != 1.0f);
		if (matDrv->PPBuiltin.LightMapScale != needLmScale)
			matDrv->PPBuiltin.LightMapScale = needLmScale;
		matDrv->PPBuiltin.Touched = true;
	}

	// Object UBO override: selfIllumination and light factor zeroing for pass > 0
	_LightMapUBOOverride.Active = true;
	_LightMapUBOOverride.SelfIllumination[0] = selfIllumination.R;
	_LightMapUBOOverride.SelfIllumination[1] = selfIllumination.G;
	_LightMapUBOOverride.SelfIllumination[2] = selfIllumination.B;
	_LightMapUBOOverride.SelfIllumination[3] = 0.0f;
	_LightMapUBOOverride.ZeroLightFactors = (pass > 0);

	// Program setup, override clearing, and fog restore handled by setupPass() after return.
}

// ***************************************************************************
void CDriverGL3::endLightMapMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endLightMapMultiPass)

	// Reset to base material UBO slot
	const CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = getMatDrv(mat);
	matDrv->MaterialUBOCurrent = 0;

	// Clear lightmap override state
	_LightMapUBOOverride.Active = false;
	if (_FogColorOverrideBlack)
	{
		_FogColorOverrideBlack = false;
		_CameraUBODirty = true;
	}
}

// ***************************************************************************
sint CDriverGL3::beginWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginWaterMultiPass)

	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);
	return 1;
}

// ***************************************************************************
void CDriverGL3::setupWaterPass(uint /* pass */)
{
	H_AUTO_OGL(CDriverGL3_setupWaterPass)
	nlassert(_CurrentMaterial);
	CMaterial &mat = *_CurrentMaterial;
	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);

	uint k;
	ITexture *tex = mat.getTexture(0);
	if (tex)
	{
		tex->setUploadFormat(ITexture::RGBA8888);
		setupTexture(*tex);
		activateTexture(0, tex);
	}
	tex = mat.getTexture(1);
	if (tex)
	{
		tex->setUploadFormat(ITexture::RGBA8888);
		setupTexture(*tex);
		activateTexture(1, tex);
	}
	tex = mat.getTexture(2);
	if (tex)
	{
		setupTexture(*tex);
		activateTexture(2, tex);
	}
	tex = mat.getTexture(3);
	if (tex)
	{
		setupTexture(*tex);
		activateTexture(3, tex);
	}
	for (k = 4; k < IDRV_PROGRAM_MAXSAMPLERS; ++k)
	{
		activateTexture(k, NULL);
	}

	// Select water FP variant: bit 0 = fog, bit 1 = diffuse
	uint fpIdx = (_FogEnabled ? 1 : 0) | (mat.getTexture(3) != NULL ? 2 : 0);

	// Lazy creation of water FP programs
	if (!_WaterFP[fpIdx])
	{
		// Build water UBO format (once, shared across all water FP variants)
		if (!_WaterUBFormat)
		{
			CUniformBufferFormat *fmt = new CUniformBufferFormat();
			fmt->Name = "NlWater";
			_WaterUBOOffsets.Bump0ScaleBias = fmt->push("bump0ScaleBias", CUniformBufferFormat::FloatVec4);
			_WaterUBOOffsets.Bump1ScaleBias = fmt->push("bump1ScaleBias", CUniformBufferFormat::FloatVec4);
			_WaterUBFormat = fmt;
			_WaterUB = new CUniformBuffer();
			_WaterUB->Format = *fmt;
		}

		std::string defines;
		if (fpIdx & 2) defines += "#define USE_DIFFUSE\n";

		_WaterFP[fpIdx] = new CPixelProgram();

		// Pipeline-stage UBO source (preferred for linked program path)
		{
			IProgram::CSource *s = new IProgram::CSource();
			s->Profile = IProgram::glsl300esf;
			s->Features.UsesCameraUBO = true;
			s->Features.OnlyUBOs = true;
			s->UniformBufferFormats[UBBindingPixelProgram] = _WaterUBFormat;
			s->DisplayName = NLMISC::toString("glsl300esf/WaterFP/%s",
			    (fpIdx & 2) ? "diffuse" : "noDiffuse");
			s->setSource(std::string(WaterFPGLSL_ES_Header) + defines + WaterFPGLSL_UBO_Body);
			_WaterFP[fpIdx]->addSource(s);
		}

		// SSO source (fallback)
		{
			std::string src = WaterFPGLSL_Header;
			if (fpIdx & 1) src += "#define USE_FOG\n";
			if (fpIdx & 2) src += "#define USE_DIFFUSE\n";
			src += WaterFPGLSL_Body;

			IProgram::CSource *s = new IProgram::CSource();
			s->Profile = IProgram::glsl330f;
			s->DisplayName = NLMISC::toString("glsl330f/WaterFP/%s/%s",
			    (fpIdx & 1) ? "fog" : "noFog",
			    (fpIdx & 2) ? "diffuse" : "noDiffuse");
			s->setSource(src);
			_WaterFP[fpIdx]->addSource(s);
		}

		if (!compilePixelProgram(_WaterFP[fpIdx]))
		{
#ifdef NL_DEBUG
			nlerror("GL3: Water FP compilation failed (variant %u)", fpIdx);
#endif
		}
	}

	// Set water FP as material pixel program — setupPass() will call
	// setupBuiltinPrograms() for activation, UBO flags, and uniform setup.
	m_MaterialPixelProgram = _WaterFP[fpIdx];

	// Compute bump factors
	float factor0 = 1.f, factor1 = 1.f;
	if (mat.getTexture(0) && mat.getTexture(0)->isBumpMap())
		factor0 = 0.25f * NLMISC::safe_cast<CTextureBump *>(mat.getTexture(0))->getNormalizationFactor();
	if (mat.getTexture(1) && mat.getTexture(1)->isBumpMap())
		factor1 = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(1))->getNormalizationFactor();

	// Stage water UBO data and bind it (always — harmless if SSO path is taken)
	_WaterUB->lock();
	_WaterUB->set(_WaterUBOOffsets.Bump0ScaleBias, 2.f * factor0, -factor0, 0.f, 0.f);
	_WaterUB->set(_WaterUBOOffsets.Bump1ScaleBias, 2.f * factor1, -factor1, 0.f, 0.f);
	_WaterUB->unlock();
	bindUniformBuffer(UBBindingPixelProgram, _WaterUB);

	// Program setup and SSO uniforms handled by setupPass() after return.
}

// ***************************************************************************
void CDriverGL3::endWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endWaterMultiPass);

	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);

	// Unbind water UBO if bound
	if (_BoundUserUB[UBBindingPixelProgram] == _WaterUB)
		bindUniformBuffer(UBBindingPixelProgram, NULL);

	// Clear material pixel program — next setupPass() will restore builtin/mega PP
	m_MaterialPixelProgram = NULL;
}

} // NLDRIVERGL3
} // NL3D
