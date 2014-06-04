// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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
#include "nel/3d/cube_map_builder.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/texture_bump.h"
#include "nel/3d/material.h"
#include "driver_opengl_vertex_buffer.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

static void convBlend(CMaterial::TBlend blend, GLenum& glenum)
{
	H_AUTO_OGL(convBlend)
	switch(blend)
	{
		case CMaterial::one:		glenum=GL_ONE; break;
		case CMaterial::zero:		glenum=GL_ZERO; break;
		case CMaterial::srcalpha:	glenum=GL_SRC_ALPHA; break;
		case CMaterial::invsrcalpha:glenum=GL_ONE_MINUS_SRC_ALPHA; break;
		case CMaterial::srccolor:	glenum=GL_SRC_COLOR; break;
		case CMaterial::invsrccolor:glenum=GL_ONE_MINUS_SRC_COLOR; break;

		// Extended Blend modes.
		case CMaterial::blendConstantColor:		glenum=GL_CONSTANT_COLOR_EXT; break;
		case CMaterial::blendConstantInvColor:	glenum=GL_ONE_MINUS_CONSTANT_COLOR_EXT; break;
		case CMaterial::blendConstantAlpha:		glenum=GL_CONSTANT_ALPHA_EXT; break;
		case CMaterial::blendConstantInvAlpha:	glenum=GL_ONE_MINUS_CONSTANT_ALPHA_EXT; break;

		default: nlstop;
	}
}

static void convZFunction(CMaterial::ZFunc zfunc, GLenum& glenum)
{
	H_AUTO_OGL(convZFunction)
	switch(zfunc)
	{
		case CMaterial::lessequal:	glenum=GL_LEQUAL; break;
		case CMaterial::less:		glenum=GL_LESS; break;
		case CMaterial::always:		glenum=GL_ALWAYS; break;
		case CMaterial::never:		glenum=GL_NEVER; break;
		case CMaterial::equal:		glenum=GL_EQUAL; break;
		case CMaterial::notequal:	glenum=GL_NOTEQUAL; break;
		case CMaterial::greater:	glenum=GL_GREATER; break;
		case CMaterial::greaterequal:	glenum=GL_GEQUAL; break;
		default: nlstop;
	}
}

static void	convColor(CRGBA col, GLfloat glcol[4])
{
	H_AUTO_OGL(convColor)
	static	const float	OO255= 1.0f/255;
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
}

static inline void convTexAddr(ITexture *tex, CMaterial::TTexAddressingMode mode, GLenum &glenum)
{
	H_AUTO_OGL(convTexAddr)
	nlassert(mode < CMaterial::TexAddrCount);
	static const GLenum glTex2dAddrModesNV[] =
	{
		GL_NONE, GL_TEXTURE_2D,
		GL_PASS_THROUGH_NV, GL_CULL_FRAGMENT_NV,
		GL_OFFSET_TEXTURE_2D_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV,
		GL_DEPENDENT_AR_TEXTURE_2D_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV,
		GL_DOT_PRODUCT_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
		GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,
		GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV
	};

	static const GLenum glTexCubeAddrModesNV[] =
	{
		GL_NONE, GL_TEXTURE_CUBE_MAP,
		GL_PASS_THROUGH_NV, GL_CULL_FRAGMENT_NV,
		GL_OFFSET_TEXTURE_2D_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV,
		GL_DEPENDENT_AR_TEXTURE_2D_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV,
		GL_DOT_PRODUCT_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
		GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,
		GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV
	};

	if (!tex || !tex->isTextureCube())
	{
		glenum = glTex2dAddrModesNV[(uint) mode];
	}
	else
	{
		glenum = glTexCubeAddrModesNV[(uint) mode];
	}
}

// --------------------------------------------------
void CDriverGL3::setTexGenFunction(uint stage, CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL3_setTexGenFunction)
	ITexture *text = mat.getTexture(uint8(stage));
	if (text)
	{
		if (mat.getTexCoordGen(stage))
		{
			// set mode and enable.
			CMaterial::TTexCoordGenMode	mode = mat.getTexCoordGenMode(stage);
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

//--------------------------------
void CDriverGL3::setupUserTextureMatrix(uint numStages, CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL3_setupUserTextureMatrix)
	if (
		(_UserTexMatEnabled != 0 && (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) == 0)
		|| (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) != 0
	  )
	{
		// for each stage, setup the texture matrix if needed
		uint newMask = (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) >> IDRV_MAT_USER_TEX_FIRST_BIT;
		uint shiftMask = 1;
		for (uint k = 0; k < numStages ; ++k)
		{
			if (newMask & shiftMask) // user matrix for this stage
			{
				_UserTexMat[k] = mat.getUserTexMat(k);
				_UserTexMatEnabled |= shiftMask;
			}
			else
			{
				/// check if matrix disabled
				if (
					(newMask & shiftMask) != (_UserTexMatEnabled & shiftMask)
				  )
				{
					_UserTexMat[k].identity();
					_UserTexMatEnabled &= ~shiftMask;
				}
			}
			shiftMask <<= 1;
		}
	}
}

void CDriverGL3::disableUserTextureMatrix()
{
	H_AUTO_OGL(CDriverGL3_disableUserTextureMatrix)
	if (_UserTexMatEnabled != 0)
	{
		uint k = 0;
		do
		{
			if (_UserTexMatEnabled & (1 << k)) // user matrix for this stage
			{
				_UserTexMat[k].identity();
				_UserTexMatEnabled &= ~ (1 << k);
			}
			++k;
		}
		while (_UserTexMatEnabled != 0);
	}
}

// --------------------------------------------------
CMaterial::TShader	CDriverGL3::getSupportedShader(CMaterial::TShader shader)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	switch (shader)
	{
	case CMaterial::PerPixelLighting:
		return CMaterial::Normal; // FIXME GL3
	case CMaterial::PerPixelLightingNoSpec:
		return CMaterial::Normal; // FIXME GL3
	default:
		return shader;
	}
}

// --------------------------------------------------
bool CDriverGL3::setupMaterial(CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL3_setupMaterial)

	CMaterialDrvInfosGL3*	pShader;
	CMaterial::TShader matShader;
	GLenum		glenum = GL_ZERO;
	uint32		touched = mat.getTouched();

	// profile.
	_NbSetupMaterialCall++;


	// 0. Retrieve/Create driver shader.
	//==================================
	if (!mat._MatDrvInfo)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItMatDrvInfoPtrList		it= _MatDrvInfos.insert(_MatDrvInfos.end(), (NL3D::IMaterialDrvInfos*)NULL);
		// create and set iterator, for future deletion.
		*it= mat._MatDrvInfo= new CMaterialDrvInfosGL3(this, it);

		// Must create all OpenGL shader states.
		touched= IDRV_TOUCHED_ALL;
	}
	pShader=static_cast<CMaterialDrvInfosGL3*>((IMaterialDrvInfos*)(mat._MatDrvInfo));

	// 1. Setup modified fields of material conversion cache (FIXME GL3)
	//=====================================
	if (touched)
	{
		/* Exception: if only Textures are modified in the material, no need to "Bind OpenGL States", or even to test
			for change, because textures are activated alone, see below.
			No problem with delete/new problem (see below), because in this case, IDRV_TOUCHED_ALL is set (see above).
		*/
		// If any flag is set (but a flag of texture)
		if (touched & (~_MaterialAllTextureTouchedFlag))
		{
			// Convert Material to driver shader.
			if (touched & IDRV_TOUCHED_BLENDFUNC)
			{
				convBlend(mat.getSrcBlend(),glenum);
				pShader->SrcBlend=glenum;
				convBlend(mat.getDstBlend(),glenum);
				pShader->DstBlend=glenum;
			}
			if (touched & IDRV_TOUCHED_ZFUNC)
			{
				convZFunction(mat.getZFunc(),glenum);
				pShader->ZComp= glenum;
			}
			if (touched & IDRV_TOUCHED_LIGHTING)
			{
				convColor(mat.getEmissive(), pShader->Emissive);
				convColor(mat.getAmbient(), pShader->Ambient);
				convColor(mat.getDiffuse(), pShader->Diffuse);
				convColor(mat.getSpecular(), pShader->Specular);
				pShader->PackedEmissive= mat.getEmissive().getPacked();
				pShader->PackedAmbient= mat.getAmbient().getPacked();
				pShader->PackedDiffuse= mat.getDiffuse().getPacked();
				pShader->PackedSpecular= mat.getSpecular().getPacked();
			}
			if (touched & IDRV_TOUCHED_SHADER)
			{
				// Get shader. Fallback to other shader if not supported.
				pShader->SupportedShader= getSupportedShader(mat.getShader());
			}

			// Since modified, must rebind all openGL states. And do this also for the delete/new problem.
			/* If an old material is deleted, _CurrentMaterial is invalid. But this is grave only if a new
				material is created, with the same pointer (bad luck). Since an newly allocated material always
				pass here before use, we are sure to avoid any problems.
			*/
			_CurrentMaterial= NULL;
		}

		// Optimize: reset all flags at the end.
		// mat.clearTouched(0xFFFFFFFF); // FIXME GL3 THIS IS NOW DONE IN GENERATE OF PP DESC, THIS NEED RESTRUCTURING
	}

	// 2b. User supplied pixel shader overrides material
	//==================================
	// Now we can get the supported shader from the cache.
	matShader = pShader->SupportedShader;

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
	if (matShader != CMaterial::Water && matShader != CMaterial::Program)
	{
		for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		{
			ITexture *text = mat.getTexture(uint8(stage));
			if (text != NULL && !setupTexture(*text))
				return false;
		}
	}
	// Here, for Lightmap materials, setup the lightmaps.
	if (matShader == CMaterial::LightMap)
	{
		for (uint stage = 0; stage < mat._LightMaps.size(); stage++)
		{
			ITexture *text = mat._LightMaps[stage].Texture;
			if (text != NULL && !setupTexture(*text))
				return false;
		}
	}

	// Here, for caustic shader, setup the lightmaps
	/*if (matShader == CMaterial::Caustics)
	{
		if (mat.getTexture(stage))
	}*/

	// NOTE: A vertex buffer MUST be enabled before calling setupMaterial!
	nlassert(_CurrentVertexBufferHard);
	uint16 vertexFormat = _CurrentVertexBufferHard->VB->getVertexFormat();

	// Activate the textures.
	// Do not do it for Lightmap and per pixel lighting , because done in multipass in a very special fashion.
	// This avoid the useless multiple change of texture states per lightmapped object.
	if (matShader != CMaterial::LightMap
		&& matShader != CMaterial::PerPixelLighting
		/* && matShader != CMaterial::Caustics	*/
		&& matShader != CMaterial::Cloud
		&& matShader != CMaterial::Water
	  )
	{
		uint maxTex = matShader == CMaterial::Specular ? 2 : IDRV_MAT_MAXTEXTURES;
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			ITexture *text = mat.getTexture(uint8(stage));

			// activate the texture, or disable texturing if NULL.
			activateTexture(stage, text);
			
			if (vertexFormat & g_VertexFlags[TexCoord0 + stage])
			{
				// Do not allow TexGen when vertex flags set
				setTexGenModeVP(stage, TexGenDisabled);
			}
			else if (matShader != CMaterial::Specular) // Specular has it's own env function setup by startSpecularBatch
			{
				setTexGenFunction(stage, mat);
			}
		}
	}
	else if (matShader == CMaterial::Cloud)
	{
		activateTexture(0, mat.getTexture(0));
		setTexGenModeVP(0, TexGenDisabled);
		activateTexture(1, mat.getTexture(0));
		setTexGenModeVP(1, TexGenDisabled);
	}
	else if (matShader != CMaterial::Specular) // TEMP BUGFIX
	{// TEMP BUGFIX
		for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		{
			setTexGenModeVP(stage, TexGenDisabled);
		}
	}// TEMP BUGFIX

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
		}
	}

	// 3. Bind OpenGL States.
	//=======================
	if (_CurrentMaterial != &mat) // FIXME GL3: CMaterial may be touched...
	{
		// Bind Blend Part.
		//=================
		bool blend = (mat.getFlags() & IDRV_MAT_BLEND)!=0;
		_DriverGLStates.enableBlend(blend);
		if (blend)
			_DriverGLStates.blendFunc(pShader->SrcBlend, pShader->DstBlend);

		// Double Sided Part.
		//===================
		// NB: inverse state: DoubleSided <=> !CullFace.
		uint32 twoSided = mat.getFlags() & IDRV_MAT_DOUBLE_SIDED;
		_DriverGLStates.enableCullFace(twoSided == 0);

		// Bind ZBuffer Part.
		//===================
		_DriverGLStates.enableZWrite(mat.getFlags() & IDRV_MAT_ZWRITE);
		_DriverGLStates.depthFunc(pShader->ZComp);
		_DriverGLStates.setZBias(mat.getZBias() * _OODeltaZ);

		// Bind Stencil Buffer Part. // FIXME GL3: STENCIL TEST
		//===================
		/*
		_DriverGLStates.enableStencilTest();
		_DriverGLStates.stencilFunc();
		_DriverGLStates.stencilOp();
		*/

		// Color-Lighting Part.
		//=====================
		// Light Part.
		enableLightingVP(mat.getFlags() & IDRV_MAT_LIGHTING);

		// Fog Part.
		//=================
		// Disable fog if dest blend is ONE or restore fog state to its current value
		enableFogVP((blend && (pShader->DstBlend == GL_ONE)) ? false : _FogEnabled);

		// Done.
		_CurrentMaterial = &mat;
	}

	// 4. Misc
	//=====================================
	// Textures user matrix
	if (matShader == CMaterial::Normal)
	{
		setupUserTextureMatrix(IDRV_MAT_MAXTEXTURES, mat);
	}
	else // deactivate texture matrix
	{
		disableUserTextureMatrix();
	}

	// 5. Set up the program
	// =====================
	switch (matShader)
	{
	case CMaterial::LightMap:
		// Programs are setup in multipass
		return true;
	default:
		return setupBuiltinPrograms();
	}
}

// ***************************************************************************
sint CDriverGL3::beginMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginMultiPass)

	// Depending on material type and hardware, return number of pass required to draw this material.
	switch(_CurrentMaterialSupportedShader)
	{
	case CMaterial::LightMap:
		return  beginLightMapMultiPass();
	case CMaterial::Specular:
		return  beginSpecularMultiPass();
	case CMaterial::Water:
		return  beginWaterMultiPass();
	case CMaterial::PerPixelLighting:
		return  beginPPLMultiPass();
	case CMaterial::PerPixelLightingNoSpec:
		return  beginPPLNoSpecMultiPass();
	default: 
		return 1;
	}
}

// ***************************************************************************
bool CDriverGL3::setupPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupPass)
	
	switch(_CurrentMaterialSupportedShader)
	{
	case CMaterial::Normal:
		setupNormalPass();
		break;
	case CMaterial::LightMap:
		setupLightMapPass(pass);
		break;
	case CMaterial::Specular:
		setupSpecularPass(pass);
		break;
	case CMaterial::Water:
		setupWaterPass(pass);
		break;
	case CMaterial::PerPixelLighting:
		setupPPLPass(pass);
		break;
	case CMaterial::PerPixelLightingNoSpec:
		setupPPLNoSpecPass(pass);
		break;
	case CMaterial::Cloud:
		setupCloudPass();
		break;
	}

	return true;
}

// ***************************************************************************
void			CDriverGL3::endMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endMultiPass)

	switch(_CurrentMaterialSupportedShader)
	{
	case CMaterial::LightMap:
		endLightMapMultiPass();
		break;
	case CMaterial::Specular:
		endSpecularMultiPass();
		break;
	case CMaterial::Water:
		endWaterMultiPass();
		return;
	case CMaterial::PerPixelLighting:
		endPPLMultiPass();
		break;
	case CMaterial::PerPixelLightingNoSpec:
		endPPLNoSpecMultiPass();
		break;
	default: return;
	}
}

void CDriverGL3::setupNormalPass()
{
	nlassert(m_DriverPixelProgram);

	const CMaterial &mat = *_CurrentMaterial;
	
	for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
	{
		// Set pixel program constants for TexEnv
		uint constantIdx = m_DriverPixelProgram->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + stage));
		if (constantIdx != ~0)
		{
			GLfloat glCol[4];
			convColor(mat._TexEnvs[stage].ConstantColor, glCol);
			setUniform4f(IDriver::PixelProgram, constantIdx, glCol[0], glCol[1], glCol[2], glCol[3]);
		}

		// Set vertex program constants for TexMatrix
		uint texMatrixIdx = m_DriverVertexProgram->getUniformIndex((CProgramIndex::TName)(CProgramIndex::TexMatrix0 + stage));
		if (texMatrixIdx != ~0)
		{
			setUniform4x4f(IDriver::VertexProgram, texMatrixIdx, _UserTexMat[stage]);
		}
	}
}

// ***************************************************************************
void CDriverGL3::computeLightMapInfos(const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL3_computeLightMapInfos)
	static const uint32 RGBMaskPacked = CRGBA(255,255,255,0).getPacked();

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

	//setupBuiltinPrograms(); // FIXME GL3

	const CMaterial &mat= *_CurrentMaterial;

	// compute how many lightmap and pass we must process.
	computeLightMapInfos(mat);

	// always enable lighting for lightmap (because of dynamic light)
	enableLightingVP(true);

	// if the dynamic lightmap light has changed since the last render (should not happen), resetup
	// normal way is that setupLightMapDynamicLighting() is called in setupMaterial() if shader different from prec
	if (_LightMapDynamicLightDirty)
		setupLightMapDynamicLighting(true);

	// reset Ambient and specular lighting
	// static	uint32	packedColorBlack= CRGBA(0,0,0,255).getPacked();
	// static	GLfloat glcolBlack[4]= {0,0,0,1};
	// lightmap get no specular/ambient. Emissive and Diffuse are setuped in setupLightMapPass()
	// _DriverGLStates.setAmbient(packedColorBlack, glcolBlack);
	// _DriverGLStates.setSpecular(packedColorBlack, glcolBlack);
	// FIXME GL3 LIGHTMAP

	// Manage too if no lightmaps.
	return std::max(_NLightMapPass, (uint)1);
}

// ***************************************************************************
void CDriverGL3::setupLightMapPass(uint pass)
{
	nlassert(m_DriverPixelProgram);
	nlassert(!m_UserPixelProgram);

	H_AUTO_OGL(CDriverGL3_setupLightMapPass)
	const CMaterial &mat= *_CurrentMaterial;

	// common colors
	static const GLfloat glcolBlack[4] = { 0.f, 0.f, 0.f, 1.f };
	static const GLfloat glcolWhite[4] = { 1.f, 1.f, 1.f, 1.f };
	static const GLfloat glcolGrey[4] = { 0.5f, 0.5f, 0.5f, 1.f };

	// No lightmap or all blacks??, just setup "black texture" for stage 0.
	if (_NLightMaps == 0)
	{
		// nldebug("No lightmaps");

		ITexture *text = mat.getTexture(0);
		activateTexture(0, text);

		// setup std modulate env
		// CMaterial::CTexEnv	env;
		// activateTexEnvMode(0, env); // FIXME GL3: standard modulate env

		// Since Lighting is disabled, as well as colorArray, must setup alpha.
		// setup color to 0 => blackness. in emissive cause texture can still be lighted by dynamic light
		// _DriverGLStates.setEmissive(packedColorBlack, glcolBlack);
		// FIXME GL3 LIGHTMAP

		// Setup gen tex off
		setTexGenModeVP(0, TexGenDisabled);

		// And disable other stages.
		for (uint stage = 1; stage < IDRV_MAT_MAXTEXTURES; stage++)
		{
			// disable texturing.
			activateTexture(stage, NULL);
		}

		// Setup the programs now
		setupBuiltinPrograms();

		return;
	}

	nlassert(pass < _NLightMapPass);

	// setup Texture Pass.
	//=========================
	uint	lmapId;
	uint	nstages;
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
	CRGBA col((uint8)r,(uint8)g,(uint8)b,255);

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
			CRGBA lmcDiff= mat._LightMaps[whichLightMap].LMCDiffuse;

			lmapFactor.R = (uint8)(((uint32)lmapFactor.R  * ((uint32)lmcDiff.R+(lmcDiff.R>>7))) >>8);
			lmapFactor.G = (uint8)(((uint32)lmapFactor.G  * ((uint32)lmcDiff.G+(lmcDiff.G>>7))) >>8);
			lmapFactor.B = (uint8)(((uint32)lmapFactor.B  * ((uint32)lmcDiff.B+(lmcDiff.B>>7))) >>8);
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

				// FIXME GL3: builtin TexEnv[stage] = TexEnvSpecialLightMap

				// Setup constant color with Lightmap factor.
				constant[stage] = NLMISC::CRGBAF(lmapFactor);

				/*static CMaterial::CTexEnv	stdEnv;
				{
					// setup constant color with Lightmap factor.
					stdEnv.ConstantColor = lmapFactor;

					int cl = m_DriverPixelProgram->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + stage));
					if (cl != -1)
					{
						GLfloat glCol[ 4 ];
						convColor(lmapFactor, glCol);
						setUniform4f(IDriver::PixelProgram, cl, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
					}

					// activateTexEnvColor(stage, stdEnv); // FIXME GL3

					// setup TexEnvCombine4 (ignore alpha part).
					/*if (_CurrentTexEnvSpecial[stage] != TexEnvSpecialLightMap)
					{
						// TexEnv is special.
						_CurrentTexEnvSpecial[stage] = TexEnvSpecialLightMap;
					}*/ // FIXME GL3
				/*}*/
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

				// setup ModulateRGB/ReplaceAlpha env. (this may disable possible COMBINE4_NV setup).
				// activateTexEnvMode(stage, _LightMapLastStageEnv); // SHADER BUILTIN

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

	// setup blend / lighting.
	//=========================

	/* If multi-pass, then must setup a black Fog color for 1+ pass (just do it for the pass 1).
		This is because Transparency ONE/ONE is used.
	*/

	// Blend is different if the material is blended or not
	if (!mat.getBlend())
	{
		// Not blended, std case.
		if (pass==0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(false);
		}
		else if (pass==1)
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
		if (pass==0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (pass==1)
		{
			// setup an Additive transparency (only for pass 1, will be kept for successives pass).
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE);
		}
	}

	// Dynamic lighting: The influence of the dynamic light must be added only in the first pass (only one time)
	if (pass==0)
	{
		// If the lightmap is in x2 mode, then must divide effect of the dynamic light too
		// if (mat._LightMapsMulx2)
		// 	_DriverGLStates.setDiffuse(packedColorGrey, glcolGrey);
		// else
		// 	_DriverGLStates.setDiffuse(packedColorWhite, glcolWhite);
		// FIXME GL3 LIGHTMAP
	}
	// no need to reset for pass after 1, since same than prec pass (black)!
	else if (pass==1)
	{
		// _DriverGLStates.setDiffuse(packedColorBlack, glcolBlack);
		// FIXME GL3 LIGHTMAP
	}

	// Setup the programs now
	setupBuiltinPrograms();

	// Set constants
	for (uint stage = 0; stage < std::min(_Extensions.MaxFragmentTextureImageUnits, (GLint)IDRV_PROGRAM_MAXSAMPLERS); ++stage)
	{
		uint constantIdx = m_DriverPixelProgram->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + stage));
		if (constantIdx != ~0)
		{
			setUniform4f(IDriver::PixelProgram, constantIdx, constant[stage].R, constant[stage].G, constant[stage].B, constant[stage].A);
		}
	}

	// Set self illumination
	// FIXME GL3: selfIllumination
}

// ***************************************************************************
void			CDriverGL3::endLightMapMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endLightMapMultiPass)
}

// ***************************************************************************
void			CDriverGL3::startSpecularBatch()
{
	H_AUTO_OGL(CDriverGL3_startSpecularBatch)
	_SpecularBatchOn= true;

	setupSpecularBegin();
}

// ***************************************************************************
void			CDriverGL3::endSpecularBatch()
{
	H_AUTO_OGL(CDriverGL3_endSpecularBatch)
	_SpecularBatchOn= false;

	setupSpecularEnd();
}

// ***************************************************************************
void			CDriverGL3::setupSpecularBegin()
{
	H_AUTO_OGL(CDriverGL3_setupSpecularBegin)

	// setup the good matrix for stage 1.
	// NB: Cannot set uniforms here directly, because the program does not exist yet
	_UserTexMat[1] = _SpecularTexMtx;
}

// ***************************************************************************
void			CDriverGL3::setupSpecularEnd()
{
	H_AUTO_OGL(CDriverGL3_setupSpecularEnd)

	// Disable Texture coord generation // FIXME GL3: This should not be necessary...
	setTexGenModeVP(1, TexGenDisabled);

	// Happiness !!! we have already enabled the stage 1 - lolwhat
	_UserTexMat[1].identity();
}

// ***************************************************************************
sint			CDriverGL3::beginSpecularMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginSpecularMultiPass)
	const CMaterial &mat= *_CurrentMaterial;

	// End specular , only if not Batching mode.
	if (!_SpecularBatchOn)
		setupSpecularBegin();

	// Set shader values
	// OPTIMIZE GL3: Only need to do this when program changed since it was last set
	uint idx = m_DriverVertexProgram->getUniformIndex((CProgramIndex::TName)(CProgramIndex::TexMatrix1));
	if (idx != ~0)
		setUniform4x4f(IDriver::VertexProgram, idx, _UserTexMat[1]);

	// Only need one pass for specular
	return 1;
}

// ***************************************************************************
void			CDriverGL3::setupSpecularPass(uint pass)
{
	nlassert(m_DriverPixelProgram);
	nlassert(m_DriverVertexProgram);

	H_AUTO_OGL(CDriverGL3_setupSpecularPass)
}

// ***************************************************************************
void			CDriverGL3::endSpecularMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endSpecularMultiPass)

	// End specular , only if not Batching mode.
	if (!_SpecularBatchOn)
		setupSpecularEnd();
}

// ***************************************************************************
sint			CDriverGL3::beginPPLMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginPPLMultiPass)

	//setupBuiltinPrograms(); // FIXME GL3

	#ifdef NL_DEBUG
		nlassert(supportPerPixelLighting(true)); // make sure the hardware can do that
	#endif
	return 1;
}

// ***************************************************************************
void			CDriverGL3::setupPPLPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupPPLPass)
	const CMaterial &mat= *_CurrentMaterial;

	nlassert(pass == 0);

	/*ITexture *tex0 = getSpecularCubeMap(1);
	if (tex0) setupTexture(*tex0);
	ITexture *tex2 = getSpecularCubeMap((uint) mat.getShininess());
	if (tex2) setupTexture(*tex2);
	if (mat.getTexture(0)) setupTexture(*mat.getTexture(0));*/

	// tex coord 0 = texture coordinates
	// tex coord 1 = normal in tangent space
	// tex coord 2 = half angle vector in tangent space

	/*activateTexture(0, tex0);
	activateTexture(1, mat.getTexture(0));
	activateTexture(2, tex2);*/

	/*for (uint k = 3; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		activateTexture(k, NULL);
	}*/

	// setup the tex envs

	// Stage 0 is rgb = DiffuseCubeMap * LightColor + DiffuseGouraud * 1

	// activateTexEnvColor(0, _PPLightDiffuseColor); // FIXME GL3

	// Stage 1
	// alpha = diffuse alpha

	// Stage 2 is rgb = SpecularCubeMap * SpecularLightColor + Prec * 1
	// alpha = prec alpha

	// activateTexEnvColor(2, _PPLightSpecularColor); // FIXME GL3

}

// ***************************************************************************
void			CDriverGL3::endPPLMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endPPLMultiPass)
	// nothing to do there ...
}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
sint			CDriverGL3::beginPPLNoSpecMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginPPLNoSpecMultiPass)

	//setupBuiltinPrograms(); // FIXME GL3

	#ifdef NL_DEBUG
		nlassert(supportPerPixelLighting(false)); // make sure the hardware can do that
	#endif
	return 1;
}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
void			CDriverGL3::setupPPLNoSpecPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupPPLNoSpecPass)
	const CMaterial &mat= *_CurrentMaterial;

	nlassert(pass == 0);

/*	ITexture *tex0 = getSpecularCubeMap(1);
	if (tex0) setupTexture(*tex0);

	if (mat.getTexture(0)) setupTexture(*mat.getTexture(0));*/

	// tex coord 0 = texture coordinates
	// tex coord 1 = normal in tangent space

/*	activateTexture(0, tex0);
	activateTexture(1, mat.getTexture(0));
*/
	/*for (uint k = 2; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		activateTexture(k, NULL);
	}*/

	// setup the tex envs

	// Stage 0 is rgb = DiffuseCubeMap * LightColor + DiffuseGouraud * 1 (TODO : EnvCombine3)

	// activateTexEnvColor(0, _PPLightDiffuseColor); // FIXME GL3

	// Stage 1
	/*static CMaterial::CTexEnv	env;
	env.Env.SrcArg1Alpha = CMaterial::Diffuse;
	activateTexEnvMode(1, env);*/ // FIXME GL3

}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
void			CDriverGL3::endPPLNoSpecMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endPPLNoSpecMultiPass)
	// nothing to do there ...
}

// ***************************************************************************
void		CDriverGL3::setupCloudPass()
{
	H_AUTO_OGL(CDriverGL3_setupCloudPass)
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);
}

// ***************************************************************************
sint CDriverGL3::beginWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginWaterMultiPass)

	//setupBuiltinPrograms(); // FIXME GL3

	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);
	return 1;
}

// ***************************************************************************
/** Presetupped texture shader for water shader on NV20
  */
static const uint8 WaterNoDiffuseTexAddrMode[IDRV_MAT_MAXTEXTURES] =
{
	CMaterial::FetchTexture,
	CMaterial::OffsetTexture,
	CMaterial::OffsetTexture,
	CMaterial::TextureOff
};

static const uint8 WaterTexAddrMode[IDRV_MAT_MAXTEXTURES] =
{
	CMaterial::FetchTexture,
	CMaterial::OffsetTexture,
	CMaterial::OffsetTexture,
	CMaterial::FetchTexture
};

static const float IdentityTexMat[4] = { 1.f, 0.f, 0.f, 1.f };

// ***************************************************************************
void CDriverGL3::setupWaterPass(uint /* pass */)
{
	H_AUTO_OGL(CDriverGL3_setupWaterPass)
	nlassert (_CurrentMaterial);
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
	for (k = 4; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		activateTexture(k, NULL);
	}

}

// ***************************************************************************
void CDriverGL3::endWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endWaterMultiPass);

	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);
}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D
