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

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
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
#ifndef USE_OPENGLES
		case CMaterial::blendConstantColor:		glenum=GL_CONSTANT_COLOR_EXT; break;
		case CMaterial::blendConstantInvColor:	glenum=GL_ONE_MINUS_CONSTANT_COLOR_EXT; break;
		case CMaterial::blendConstantAlpha:		glenum=GL_CONSTANT_ALPHA_EXT; break;
		case CMaterial::blendConstantInvAlpha:	glenum=GL_ONE_MINUS_CONSTANT_ALPHA_EXT; break;
#endif

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
#ifndef USE_OPENGLES
		GL_PASS_THROUGH_NV, GL_CULL_FRAGMENT_NV,
		GL_OFFSET_TEXTURE_2D_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV,
		GL_DEPENDENT_AR_TEXTURE_2D_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV,
		GL_DOT_PRODUCT_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
		GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,
		GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV
#endif
	};

	static const GLenum glTexCubeAddrModesNV[] =
	{
		GL_NONE, GL_TEXTURE_CUBE_MAP_ARB,
#ifndef USE_OPENGLES
		GL_PASS_THROUGH_NV, GL_CULL_FRAGMENT_NV,
		GL_OFFSET_TEXTURE_2D_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV,
		GL_DEPENDENT_AR_TEXTURE_2D_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV,
		GL_DOT_PRODUCT_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
		GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,
		GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV
#endif
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
void CDriverGL::setTextureEnvFunction(uint stage, CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL_setTextureEnvFunction)
	ITexture	*text= mat.getTexture(uint8(stage));
	if(text)
	{
		CMaterial::CTexEnv	&env= mat._TexEnvs[stage];

		// Activate the env for this stage.
		// NB: Thoses calls use caching.
		activateTexEnvMode(stage, env);
		activateTexEnvColor(stage, env);

		// Activate texture generation mapping
		_DriverGLStates.activeTextureARB(stage);
		if (mat.getTexCoordGen (stage))
		{
			// set mode and enable.
			CMaterial::TTexCoordGenMode	mode= mat.getTexCoordGenMode(stage);
			if(mode==CMaterial::TexCoordGenReflect)
			{
				// Cubic or normal ?
				if (text->isTextureCube ())
					_DriverGLStates.setTexGenMode (stage, GL_REFLECTION_MAP_ARB);
				else
#ifdef USE_OPENGLES
					_DriverGLStates.setTexGenMode (stage, GL_TEXTURE_CUBE_MAP_OES);
#else
					_DriverGLStates.setTexGenMode (stage, GL_SPHERE_MAP);
#endif
			}
			else if(mode==CMaterial::TexCoordGenObjectSpace)
			{
#ifdef USE_OPENGLES
				_DriverGLStates.setTexGenMode (stage, GL_NORMAL_MAP_OES);
#else
				_DriverGLStates.setTexGenMode (stage, GL_OBJECT_LINEAR);
#endif
			}
			else if(mode==CMaterial::TexCoordGenEyeSpace)
			{
#ifdef USE_OPENGLES
				_DriverGLStates.setTexGenMode (stage, GL_NORMAL_MAP_OES);
#else
				_DriverGLStates.setTexGenMode (stage, GL_EYE_LINEAR);
#endif
			}
		}
		else
		{
			// Disable.
			_DriverGLStates.setTexGenMode(stage, 0);
		}
	}
}

//--------------------------------
void CDriverGL::setupUserTextureMatrix(uint numStages, CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL_setupUserTextureMatrix)
	if (
		(_UserTexMatEnabled != 0 && (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) == 0)
		|| (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) != 0
	   )
	{
		glMatrixMode(GL_TEXTURE);

		// for each stage, setup the texture matrix if needed
		uint newMask = (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) >> IDRV_MAT_USER_TEX_FIRST_BIT;
		uint shiftMask = 1;
		for (uint k = 0; k < numStages ; ++k)
		{
			if (newMask & shiftMask) // user matrix for this stage
			{
				_DriverGLStates.activeTextureARB(k);
				glLoadMatrixf(mat.getUserTexMat(k).get());

				_UserTexMatEnabled |= shiftMask;
			}
			else
			{
				/// check if matrix disabled
				if (
					(newMask & shiftMask) != (_UserTexMatEnabled & shiftMask)
				   )
				{
					_DriverGLStates.activeTextureARB(k);
					glLoadIdentity();

					_UserTexMatEnabled &= ~shiftMask;
				}
			}
			shiftMask <<= 1;
		}
		glMatrixMode(GL_MODELVIEW);
	}
}

void CDriverGL::disableUserTextureMatrix()
{
	H_AUTO_OGL(CDriverGL_disableUserTextureMatrix)
	if (_UserTexMatEnabled != 0)
	{
		glMatrixMode(GL_TEXTURE);

		uint k = 0;
		do
		{
			if (_UserTexMatEnabled & (1 << k)) // user matrix for this stage
			{
				_DriverGLStates.activeTextureARB(k);
				glLoadIdentity();

				_UserTexMatEnabled &= ~ (1 << k);
			}
			++k;
		}
		while (_UserTexMatEnabled != 0);
		glMatrixMode(GL_MODELVIEW);
	}
}

// --------------------------------------------------
CMaterial::TShader	CDriverGL::getSupportedShader(CMaterial::TShader shader)
{
	H_AUTO_OGL(CDriverGL_CDriverGL)
	switch (shader)
	{
	case CMaterial::PerPixelLighting: return _SupportPerPixelShader ? CMaterial::PerPixelLighting : CMaterial::Normal;
	case CMaterial::PerPixelLightingNoSpec: return _SupportPerPixelShaderNoSpec ? CMaterial::PerPixelLightingNoSpec : CMaterial::Normal;
	// Lightmap and Specular work only if at least 2 text stages.
	case CMaterial::LightMap: return (inlGetNumTextStages()>=2) ? CMaterial::LightMap : CMaterial::Normal;
	case CMaterial::Specular: return (inlGetNumTextStages()>=2) ? CMaterial::Specular : CMaterial::Normal;
		default: return shader;
	}
}

// --------------------------------------------------
void CDriverGL::setTextureShaders(const uint8 *addressingModes, const CSmartPtr<ITexture> *textures)
{
	H_AUTO_OGL(CDriverGL_setTextureShaders)
	GLenum glAddrMode;
	for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
	{
		convTexAddr(textures[stage], (CMaterial::TTexAddressingMode) addressingModes[stage], glAddrMode);

		if (glAddrMode != _CurrentTexAddrMode[stage]) // addressing mode different from the one in the device?
		{
			_DriverGLStates.activeTextureARB(stage);
#ifndef USE_OPENGLES
			glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, glAddrMode);
#endif
			_CurrentTexAddrMode[stage] = glAddrMode;
		}
	}
}

// --------------------------------------------------
bool CDriverGL::setupMaterial(CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL_setupMaterial)

	// profile.
	_NbSetupMaterialCall++;

	CMaterial::TShader matShader;
	
	CShaderGL*	pShader;
	GLenum		glenum = GL_ZERO;
	uint32		touched = mat.getTouched();

	// 0. Retrieve/Create driver shader.
	//==================================
	if (!mat._MatDrvInfo)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItMatDrvInfoPtrList		it= _MatDrvInfos.insert(_MatDrvInfos.end(), (NL3D::IMaterialDrvInfos*)NULL);
		// create and set iterator, for future deletion.
		*it= mat._MatDrvInfo= new CShaderGL(this, it);

		// Must create all OpenGL shader states.
		touched= IDRV_TOUCHED_ALL;
	}
	pShader=static_cast<CShaderGL*>((IMaterialDrvInfos*)(mat._MatDrvInfo));

	// 1. Setup modified fields of material.
	//=====================================
	if( touched )
	{
		/* Exception: if only Textures are modified in the material, no need to "Bind OpenGL States", or even to test
			for change, because textures are activated alone, see below.
			No problem with delete/new problem (see below), because in this case, IDRV_TOUCHED_ALL is set (see above).
		*/
		// If any flag is set (but a flag of texture)
		if( touched & (~_MaterialAllTextureTouchedFlag) )
		{
			// Convert Material to driver shader.
			if (touched & IDRV_TOUCHED_BLENDFUNC)
			{
				convBlend( mat.getSrcBlend(),glenum );
				pShader->SrcBlend=glenum;
				convBlend( mat.getDstBlend(),glenum );
				pShader->DstBlend=glenum;
			}
			if (touched & IDRV_TOUCHED_ZFUNC)
			{
				convZFunction( mat.getZFunc(),glenum);
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
		mat.clearTouched(0xFFFFFFFF);
	}

	// 2b. User supplied pixel shader overrides material
	//==================================
	/*if (_VertexProgramEnabled)
	{
		if (!setUniformDriver(VertexProgram)) return false;
		if (!setUniformMaterialInternal(VertexProgram, mat)) return false;
	}*/
	if (_PixelProgramEnabled)
	{
		matShader = CMaterial::Program;

		// if (!setUniformDriver(PixelProgram)) return false;
		// if (!setUniformMaterialInternal(PixelProgram, mat)) return false;
		if (!_LastSetuppedPP) return false;
	}
	else
	{
		// Now we can get the supported shader from the cache.
		matShader = pShader->SupportedShader;
	}

	// 2b. Update more shader state
	//==================================
	// if the shader has changed since last time
	if(matShader != _CurrentMaterialSupportedShader)
	{
		// if old was lightmap, restore standard lighting
		if(_CurrentMaterialSupportedShader==CMaterial::LightMap)
			setupLightMapDynamicLighting(false);

		// if new is lightmap, setup dynamic lighting
		if(matShader==CMaterial::LightMap)
			setupLightMapDynamicLighting(true);
	}

	// setup the global
	_CurrentMaterialSupportedShader= matShader;

	// 2. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame. (need to test if touched).
	// Must separate texture setup and texture activation in 2 "for"...
	// because setupTexture() may disable all stage.
	if (matShader != CMaterial::Water
		&& ((matShader != CMaterial::Program) || (_LastSetuppedPP->features().MaterialFlags & CProgramFeatures::TextureStages))
		)
	{
		for (uint stage = 0; stage < inlGetNumTextStages(); ++stage)
		{
			ITexture	*text= mat.getTexture(uint8(stage));
			if (text != NULL && !setupTexture(*text))
				return false;
		}
	}
	// Here, for Lightmap materials, setup the lightmaps.
	if(matShader == CMaterial::LightMap)
	{
		for (uint stage = 0; stage < mat._LightMaps.size(); ++stage)
		{
			ITexture *text = mat._LightMaps[stage].Texture;
			if (text != NULL && !setupTexture(*text))
				return(false);
		}
	}

	// Here, for caustic shader, setup the lightmaps
	/*if (matShader == CMaterial::Caustics)
	{
		if (mat.getTexture(stage))
	}*/

	// Activate the textures.
	// Do not do it for Lightmap and per pixel lighting , because done in multipass in a very special fashion.
	// This avoid the useless multiple change of texture states per lightmapped object.
	// Don't do it also for Specular because the EnvFunction and the TexGen may be special.
	if(matShader != CMaterial::LightMap
		&& matShader != CMaterial::PerPixelLighting
		/* && matShader != CMaterial::Caustics	*/
		&& matShader != CMaterial::Cloud
		&& matShader != CMaterial::Water
		&& matShader != CMaterial::Specular
		&& ((matShader != CMaterial::Program) || (_LastSetuppedPP->features().MaterialFlags & CProgramFeatures::TextureStages))
	   )
	{
		for(uint stage=0 ; stage<inlGetNumTextStages() ; stage++)
		{
			ITexture	*text= mat.getTexture(uint8(stage));

			// activate the texture, or disable texturing if NULL.
			activateTexture(stage,text);

			// If texture not NULL, Change texture env function.
			//==================================================
			setTextureEnvFunction(stage, mat);
		}
	}

	// 3. Bind OpenGL States.
	//=======================
	if (_CurrentMaterial!=&mat)
	{
		// Bind Blend Part.
		//=================
		bool blend = (mat.getFlags()&IDRV_MAT_BLEND)!=0;
		_DriverGLStates.enableBlend(blend);
		if(blend)
			_DriverGLStates.blendFunc(pShader->SrcBlend, pShader->DstBlend);

		// Double Sided Part.
		//===================
		// NB: inverse state: DoubleSided <=> !CullFace.
		uint32	twoSided= mat.getFlags()&IDRV_MAT_DOUBLE_SIDED;
		_DriverGLStates.enableCullFace( twoSided==0 );


		// Alpha Test Part.
		//=================
		uint32	alphaTest= mat.getFlags()&IDRV_MAT_ALPHA_TEST;
		_DriverGLStates.enableAlphaTest(alphaTest);
		if(alphaTest)
		{
			// setup alphaTest threshold.
			_DriverGLStates.alphaFunc(mat.getAlphaTestThreshold());
		}

		// Bind ZBuffer Part.
		//===================
		_DriverGLStates.enableZWrite(mat.getFlags()&IDRV_MAT_ZWRITE);
		_DriverGLStates.depthFunc(pShader->ZComp);
		_DriverGLStates.setZBias (mat.getZBias () * _OODeltaZ);

		// Bind Stencil Buffer Part.
		//===================
		/*
		_DriverGLStates.enableStencilTest();
		_DriverGLStates.stencilFunc();
		_DriverGLStates.stencilOp();
		*/

		// Color-Lighting Part.
		//=====================

		// Light Part.
		_DriverGLStates.enableLighting(mat.getFlags()&IDRV_MAT_LIGHTING);
		if(mat.getFlags()&IDRV_MAT_LIGHTING)
		{
			_DriverGLStates.setEmissive(pShader->PackedEmissive, pShader->Emissive);
			_DriverGLStates.setAmbient(pShader->PackedAmbient, pShader->Ambient);
			_DriverGLStates.setDiffuse(pShader->PackedDiffuse, pShader->Diffuse);
			_DriverGLStates.setSpecular(pShader->PackedSpecular, pShader->Specular);
			_DriverGLStates.setShininess(mat.getShininess());
			_DriverGLStates.setVertexColorLighted(mat.isLightedVertexColor ());
		}
		else
		{
			// Color unlit part.
			CRGBA	col= mat.getColor();
			glColor4ub(col.R, col.G, col.B, col.A);

			_DriverGLStates.setVertexColorLighted(false);
		}

		// Fog Part.
		//=================

		// Disable fog if dest blend is ONE
		if (blend && (pShader->DstBlend == GL_ONE))
		{
			_DriverGLStates.enableFog(false);
		}
		else
		{
			// Restore fog state to its current value
			_DriverGLStates.enableFog(_FogEnabled);
		}

		// Texture shader part.
		//=====================

		if (_Extensions.NVTextureShader)
		{
			if (matShader == CMaterial::Normal)
			{
				// Texture addressing modes (support only via NVTextureShader for now)
				//===================================================================
				if ( mat.getFlags() & IDRV_MAT_TEX_ADDR )
				{
					enableNVTextureShader(true);
					setTextureShaders(&mat._TexAddrMode[0], &mat._Textures[0]);
				}
				else
				{
					enableNVTextureShader(false);
				}
			}
			else
			{
				enableNVTextureShader(false);
			}
		}

		_CurrentMaterial=&mat;
	}

	// 4. Misc
	//=====================================

	// If !lightMap and prec material was lihgtmap => vertex setup is dirty!
	if( matShader != CMaterial::LightMap && _LastVertexSetupIsLightMap )
		resetLightMapVertexSetup();

	// Textures user matrix
	if (matShader == CMaterial::Normal
		|| ((matShader == CMaterial::Program) && (_LastSetuppedPP->features().MaterialFlags & CProgramFeatures::TextureMatrices))
		)
	{
		setupUserTextureMatrix(inlGetNumTextStages(), mat);
	}
	else
	{
		disableUserTextureMatrix();
	}

	return true;
}

// ***************************************************************************
sint			CDriverGL::beginMultiPass()
{
	H_AUTO_OGL(CDriverGL_beginMultiPass)
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
	/* case CMaterial::Caustics:
		return  beginCausticsMultiPass(); */
	case CMaterial::Cloud:
		return  beginCloudMultiPass();

	// All others materials require just 1 pass.
	default: return 1;
	}
}

// ***************************************************************************
void			CDriverGL::setupPass(uint pass)
{
	H_AUTO_OGL(CDriverGL_setupPass)
	switch(_CurrentMaterialSupportedShader)
	{
	case CMaterial::LightMap:
		setupLightMapPass (pass);
		break;
	case CMaterial::Specular:
		setupSpecularPass (pass);
		break;
	case CMaterial::Water:
		setupWaterPass(pass);
		break;
	case CMaterial::PerPixelLighting:
		setupPPLPass (pass);
		break;
	case CMaterial::PerPixelLightingNoSpec:
		setupPPLNoSpecPass (pass);
		break;
	/* case CMaterial::Caustics:
		case CMaterial::Caustics:
		break; */
	case CMaterial::Cloud:
		setupCloudPass (pass);
		break;

	// All others materials do not require multi pass.
	default: return;
	}
}

// ***************************************************************************
void			CDriverGL::endMultiPass()
{
	H_AUTO_OGL(CDriverGL_endMultiPass)
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
	/* case CMaterial::Caustics:
		endCausticsMultiPass();
		break; */
	case CMaterial::Cloud:
		endCloudMultiPass();
		break;
	// All others materials do not require multi pass.
	default: return;
	}
}

// ***************************************************************************
void CDriverGL::computeLightMapInfos (const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL_computeLightMapInfos )
	static const uint32 RGBMaskPacked = CRGBA(255,255,255,0).getPacked();

	// For optimisation consideration, suppose there is not too much lightmap.
	nlassert(mat._LightMaps.size()<=NL3D_DRV_MAX_LIGHTMAP);

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
	_NLightMapPerPass = inlGetNumTextStages()-1;
	// Can do more than 2 texture stages only if NVTextureEnvCombine4 or ATITextureEnvCombine3
	if (!_Extensions.NVTextureEnvCombine4 && !_Extensions.ATITextureEnvCombine3)
	{
		_NLightMapPerPass = 1;
		_LightMapNoMulAddFallBack= true;
	}
	else
	{
		_LightMapNoMulAddFallBack= false;
	}

	// Number of pass.
	_NLightMapPass = (_NLightMaps + _NLightMapPerPass-1)/(_NLightMapPerPass);

	// NB: _NLightMaps==0 means there is no lightmaps at all.
}

// ***************************************************************************
sint CDriverGL::beginLightMapMultiPass ()
{
	H_AUTO_OGL(CDriverGL_beginLightMapMultiPass )
	const CMaterial &mat= *_CurrentMaterial;

	// compute how many lightmap and pass we must process.
	computeLightMapInfos (mat);

	// always enable lighting for lightmap (because of dynamic light)
	_DriverGLStates.enableLighting(true);

	// if the dynamic lightmap light has changed since the last render (should not happen), resetup
	// normal way is that setupLightMapDynamicLighting() is called in setupMaterial() if shader different from prec
	if(_LightMapDynamicLightDirty)
		setupLightMapDynamicLighting(true);

	// reset Ambient and specular lighting
	static	uint32	packedColorBlack= CRGBA(0,0,0,255).getPacked();
	static	GLfloat glcolBlack[4]= {0,0,0,1};
	// lightmap get no specular/ambient. Emissive and Diffuse are setuped in setupLightMapPass()
	_DriverGLStates.setAmbient(packedColorBlack, glcolBlack);
	_DriverGLStates.setSpecular(packedColorBlack, glcolBlack);

	// reset VertexColor array if necessary.
	if (_LastVB.VertexFormat & CVertexBuffer::PrimaryColorFlag)
		_DriverGLStates.enableColorArray(false);

	// Manage too if no lightmaps.
	return	std::max (_NLightMapPass, (uint)1);
}

// ***************************************************************************
void			CDriverGL::setupLightMapPass(uint pass)
{
	H_AUTO_OGL(CDriverGL_setupLightMapPass)
	const CMaterial &mat= *_CurrentMaterial;

	// common colors
	static	uint32	packedColorBlack= CRGBA(0,0,0,255).getPacked();
	static	GLfloat glcolBlack[4]= {0.f,0.f,0.f,1.f};
	static	uint32	packedColorWhite= CRGBA(255,255,255,255).getPacked();
	static	GLfloat glcolWhite[4]= {1.f,1.f,1.f,1.f};
	static	uint32	packedColorGrey= CRGBA(128,128,128,128).getPacked();
	static	GLfloat glcolGrey[4]= {0.5f,0.5f,0.5f,1.f};

	// No lightmap or all blacks??, just setup "black texture" for stage 0.
	if(_NLightMaps==0)
	{
		ITexture	*text= mat.getTexture(0);
		activateTexture(0,text);

		// setup std modulate env
		CMaterial::CTexEnv	env;
		activateTexEnvMode(0, env);

		// Since Lighting is disabled, as well as colorArray, must setup alpha.
		// setup color to 0 => blackness. in emissive cause texture can still be lighted by dynamic light
		_DriverGLStates.setEmissive(packedColorBlack, glcolBlack);

		// Setup gen tex off
		_DriverGLStates.activeTextureARB(0);
		_DriverGLStates.setTexGenMode(0, 0);

		// And disable other stages.
		for(uint stage = 1; stage < inlGetNumTextStages(); stage++)
		{
			// disable texturing.
			activateTexture(stage, NULL);
		}

		return;
	}

	nlassert(pass<_NLightMapPass);

	// setup Texture Pass.
	//=========================
	uint	lmapId;
	uint	nstages;
	lmapId= pass * _NLightMapPerPass; // Nb lightmaps already processed
	// N lightmaps for this pass, plus the texture.
	nstages= std::min(_NLightMapPerPass, _NLightMaps-lmapId) + 1;

	// For LMC (lightmap 8Bit compression) compute the total AmbientColor in vertex diffuse
	// need only if standard MulADD version
	if (!_LightMapNoMulAddFallBack)
	{
		uint32	r=0;
		uint32	g=0;
		uint32	b=0;
		// sum only the ambient of lightmaps that will be drawn this pass
		for(uint sa=0;sa<nstages-1;sa++)
		{
			uint	wla= _LightMapLUT[lmapId+sa];
			// must mul them by their respective mapFactor too
			CRGBA ambFactor = mat._LightMaps[wla].Factor;
			CRGBA lmcAmb= mat._LightMaps[wla].LMCAmbient;
			r+= ((uint32)ambFactor.R  * ((uint32)lmcAmb.R+(lmcAmb.R>>7))) >>8;
			g+= ((uint32)ambFactor.G  * ((uint32)lmcAmb.G+(lmcAmb.G>>7))) >>8;
			b+= ((uint32)ambFactor.B  * ((uint32)lmcAmb.B+(lmcAmb.B>>7))) >>8;
		}
		r= std::min(r, (uint32)255);
		g= std::min(g, (uint32)255);
		b= std::min(b, (uint32)255);

		// this color will be added to the first lightmap (with help of emissive)
		CRGBA	col((uint8)r,(uint8)g,(uint8)b,255);
		GLfloat	glcol[4];
		convColor(col, glcol);
		_DriverGLStates.setEmissive(col.getPacked(), glcol);
	}

	// setup all stages.
	for(uint stage= 0; stage<inlGetNumTextStages(); stage++)
	{
		// if must setup a lightmap stage.
		if(stage<nstages-1)
		{
			// setup lightMap.
			uint	whichLightMap= _LightMapLUT[lmapId];
			// get text and factor.
			ITexture *text	 = mat._LightMaps[whichLightMap].Texture;
			CRGBA lmapFactor = mat._LightMaps[whichLightMap].Factor;
			// Modulate the factor with LightMap compression Diffuse
			CRGBA lmcDiff= mat._LightMaps[whichLightMap].LMCDiffuse;
			// FallBack if the (very common) extension for MulADD was not found
			if(_LightMapNoMulAddFallBack)
			{
				lmcDiff.addRGBOnly(lmcDiff, mat._LightMaps[whichLightMap].LMCAmbient);
			}
			lmapFactor.R = (uint8)(((uint32)lmapFactor.R  * ((uint32)lmcDiff.R+(lmcDiff.R>>7))) >>8);
			lmapFactor.G = (uint8)(((uint32)lmapFactor.G  * ((uint32)lmcDiff.G+(lmcDiff.G>>7))) >>8);
			lmapFactor.B = (uint8)(((uint32)lmapFactor.B  * ((uint32)lmcDiff.B+(lmcDiff.B>>7))) >>8);
			lmapFactor.A = 255;

			activateTexture(stage,text);

			// If texture not NULL, Change texture env fonction.
			//==================================================
			if(text)
			{
				static CMaterial::CTexEnv	stdEnv;

				// fallBack if extension MulAdd not found. just mul factor with (Ambient+Diffuse)
				if(_LightMapNoMulAddFallBack)
				{
					// do not use constant color to blend lightmap, but incoming diffuse color, for stage0 only.
					GLfloat	glcol[4];
					convColor(lmapFactor, glcol);
					_DriverGLStates.setEmissive(lmapFactor.getPacked(), glcol);

					// Leave stage as default env (Modulate with previous)
					activateTexEnvMode(stage, stdEnv);

					// Setup gen tex off
					_DriverGLStates.activeTextureARB(stage);
					_DriverGLStates.setTexGenMode(stage, 0);
				}
				else
				{
					// Here, we are sure that texEnvCombine4 or texEnvCombine3 is OK.
					nlassert(_Extensions.NVTextureEnvCombine4 || _Extensions.ATITextureEnvCombine3);

					// setup constant color with Lightmap factor.
					stdEnv.ConstantColor=lmapFactor;
					activateTexEnvColor(stage, stdEnv);

					// Setup env for texture stage.
					_DriverGLStates.activeTextureARB(stage);
					_DriverGLStates.setTexGenMode(stage, 0);

					// setup TexEnvCombine4 (ignore alpha part).
					if(_CurrentTexEnvSpecial[stage] != TexEnvSpecialLightMap)
					{
						// TexEnv is special.
						_CurrentTexEnvSpecial[stage] = TexEnvSpecialLightMap;

#ifdef USE_OPENGLES
						// What we want to setup is  Texture*Constant + Previous.
						glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
						// Operator.
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
						// Arg0.
						glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
						// Arg1.
						glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
						// Arg2.
						glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
#else
						if (_Extensions.NVTextureEnvCombine4)
						{
							// What we want to setup is  Texture*Constant + Previous*1.
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

							// Operator.
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD );
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD );
							// Arg0.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
							// Arg1.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT );
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
							// Arg2.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT );
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
							// Arg3.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
						}
						else
						{
							// ATI EnvCombine3
							// What we want to setup is  Texture*Constant + Previous.
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
							// Operator.
							glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI);
							glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE_ADD_ATI);
							// Arg0.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
							// Arg1.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_CONSTANT_EXT );
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
							// Arg2.
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT );
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
						}
#endif
					}
				}

				// setup UV, with UV1. Only if needed (cached)
				if( !_LastVertexSetupIsLightMap || _LightMapUVMap[stage]!=1 )
				{
					setupUVPtr(stage, _LastVB, 1);
					_LightMapUVMap[stage]= 1;
				}
			}

			// Next lightmap.
			lmapId++;
		}
		else if(stage<nstages)
		{
			// optim: do this only for first pass, and last pass only if stage!=nLMapPerPass
			// (meaning not the same stage as preceding passes).
			if(pass==0 || (pass==_NLightMapPass-1 && stage!=_NLightMapPerPass))
			{
				// activate the texture at last stage.
				ITexture	*text= mat.getTexture(0);
				activateTexture(stage,text);

				// setup ModulateRGB/ReplaceAlpha env. (this may disable possible COMBINE4_NV setup).
				activateTexEnvMode(stage, _LightMapLastStageEnv);

				// Setup gen tex off
				_DriverGLStates.activeTextureARB(stage);
				_DriverGLStates.setTexGenMode(stage, 0);

				// setup UV, with UV0. Only if needed (cached)
				if( !_LastVertexSetupIsLightMap || _LightMapUVMap[stage]!=0 )
				{
					setupUVPtr(stage, _LastVB, 0);
					_LightMapUVMap[stage]= 0;
				}

				if (mat._LightMapsMulx2)
				{
					// Multiply x 2
					glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2);
				}
			}
		}
		else
		{
			// else all other stages are disabled.
			activateTexture(stage,NULL);
		}
	}

	// setup blend / lighting.
	//=========================

	/* If multi-pass, then must setup a black Fog color for 1+ pass (just do it for the pass 1).
		This is because Transparency ONE/ONE is used.
	*/
	if(pass==1 && _FogEnabled)
	{
		static	GLfloat		blackFog[4]= {0,0,0,0};
		glFogfv(GL_FOG_COLOR, blackFog);
	}

	// Blend is different if the material is blended or not
	if( !mat.getBlend() )
	{
		// Not blended, std case.
		if(pass==0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(false);
		}
		else if(pass==1)
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
		if(pass==0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if(pass==1)
		{
			// setup an Additive transparency (only for pass 1, will be kept for successives pass).
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE);
		}
	}

	// Dynamic lighting: The influence of the dynamic light must be added only in the first pass (only one time)
	if(pass==0)
	{
		// If the lightmap is in x2 mode, then must divide effect of the dynamic light too
		if (mat._LightMapsMulx2)
			_DriverGLStates.setDiffuse(packedColorGrey, glcolGrey);
		else
			_DriverGLStates.setDiffuse(packedColorWhite, glcolWhite);
	}
	// no need to reset for pass after 1, since same than prec pass (black)!
	else if(pass==1)
		_DriverGLStates.setDiffuse(packedColorBlack, glcolBlack);
}

// ***************************************************************************
void			CDriverGL::endLightMapMultiPass()
{
	H_AUTO_OGL(CDriverGL_endLightMapMultiPass)
	// Flag the fact that VertexSetup is dirty (special lightmap). reseted in activeVertexBuffer(), and setupMaterial()
	// NB: if no lightmaps, no setupUVPtr() has been called => don't need to flag
	// (important else crash if graphist error while exporting a Lightmap material, with a MeshVertexProgram (WindTree) )
	if(_NLightMaps!=0)
		_LastVertexSetupIsLightMap= true;

	// If multi-pass, then must reset the fog color
	if(_NLightMapPass>=2 && _FogEnabled)
	{
		glFogfv(GL_FOG_COLOR, _CurrentFogColor);
	}

	// nothing to do with blending/lighting, since always setuped in activeMaterial().
	// If material is the same, then it is still a lightmap material (if changed => touched => different!)
	// So no need to reset blending/lighting here.

	// Clean up all stage for Multiply x 2
	if (_CurrentMaterial->_LightMapsMulx2)
	{
		for (uint32 i = 0; i < (_NLightMapPerPass+1); ++i)
		{
			_DriverGLStates.activeTextureARB(i);
			glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1);
		}
	}
}

// ***************************************************************************
void			CDriverGL::resetLightMapVertexSetup()
{
	H_AUTO_OGL(CDriverGL_resetLightMapVertexSetup)
	// special for all stage, std UV behavior.
	for(uint i = 0; i < inlGetNumTextStages(); i++)
	{
		// normal behavior: each texture has its own UV.
		setupUVPtr(i, _LastVB, i);
		// reset cache
		_LightMapUVMap[i]= -1;
	}

	// pop VertexColor array if necessary.
	if (_LastVB.VertexFormat & CVertexBuffer::PrimaryColorFlag)
		_DriverGLStates.enableColorArray(true);

	// flag
	_LastVertexSetupIsLightMap= false;
}

// ***************************************************************************
void			CDriverGL::startSpecularBatch()
{
	H_AUTO_OGL(CDriverGL_startSpecularBatch)
	_SpecularBatchOn= true;

	setupSpecularBegin();
}

// ***************************************************************************
void			CDriverGL::endSpecularBatch()
{
	H_AUTO_OGL(CDriverGL_endSpecularBatch)
	_SpecularBatchOn= false;

	setupSpecularEnd();
}

// ***************************************************************************
void			CDriverGL::setupSpecularBegin()
{
	H_AUTO_OGL(CDriverGL_setupSpecularBegin)
	// ---- Reset any textures with id>=2
	uint stage = 2;
	for(; stage < inlGetNumTextStages(); stage++)
	{
		// disable texturing
		activateTexture(stage, NULL);
	}

	// ---- Stage 0 Common Setup.
	// Setup the env for stage 0 only.
	// Result RGB : Texture*Diffuse, Alpha : Texture
	CMaterial::CTexEnv	env;
	env.Env.OpAlpha= CMaterial::Replace;
	activateTexEnvMode(0, env);

	// Disable texGen for stage 0
	_DriverGLStates.activeTextureARB(0);
	_DriverGLStates.setTexGenMode(0, 0);

	// ---- Stage 1 Common Setup.
	// NB don't setup the TexEnv here (stage1 setuped in setupSpecularPass() according to extensions)
	// For all cases, setup the TexCoord gen for stage1
	_DriverGLStates.activeTextureARB(1);

	// todo hulud remove
	// _DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);
	_DriverGLStates.setTexGenMode (1, GL_REFLECTION_MAP_ARB);

	// setup the good matrix for stage 1.
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf( _SpecularTexMtx.get() );
	glMatrixMode(GL_MODELVIEW);
}

// ***************************************************************************
void			CDriverGL::setupSpecularEnd()
{
	H_AUTO_OGL(CDriverGL_setupSpecularEnd)
	// Disable Texture coord generation.
	_DriverGLStates.activeTextureARB(1);
	_DriverGLStates.setTexGenMode(1, 0);

	// Happiness !!! we have already enabled the stage 1
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

// ***************************************************************************
sint			CDriverGL::beginSpecularMultiPass()
{
	H_AUTO_OGL(CDriverGL_beginSpecularMultiPass)
	const CMaterial &mat= *_CurrentMaterial;

	// activate the 2 textures here
	uint	stage;
	uint	numStages= std::min((uint)2, inlGetNumTextStages());
	for(stage=0 ; stage<numStages; stage++)
	{
		ITexture	*text= mat.getTexture(uint8(stage));

		// activate the texture, or disable texturing if NULL.
		activateTexture(stage,text);
	}

	// End specular , only if not Batching mode.
	if(!_SpecularBatchOn)
		setupSpecularBegin();

	// Manage the rare case when the SpecularMap is not provided (fault of graphist).
	if(mat.getTexture(1)==NULL)
		return 1;

	if(!_Extensions.ARBTextureCubeMap)
		return 1;

	if( _Extensions.NVTextureEnvCombine4 || _Extensions.ATITextureEnvCombine3) // NVidia or ATI optimization
		return 1;
	else
		return 2;
}

// ***************************************************************************
void			CDriverGL::setupSpecularPass(uint pass)
{
	H_AUTO_OGL(CDriverGL_setupSpecularPass)
	const CMaterial &mat= *_CurrentMaterial;

	// Manage the rare case when the SpecularMap is not provided (error of a graphist).
	if(mat.getTexture(1)==NULL)
	{
		// Just display the texture
		// NB: setupMaterial() code has correclty setuped textures.
		return;
	}

#ifdef USE_OPENGLES
#if 0
	// Ok we can do it in a single pass

	// Set Stage 1
	// Special: not the same special env if there is or not texture in stage 0.
	CTexEnvSpecial		newEnvStage1;
	if( mat.getTexture(0) == NULL )
		newEnvStage1= TexEnvSpecialSpecularStage1NoText;
	else
		newEnvStage1= TexEnvSpecialSpecularStage1;
	// Test if same env as prec.
	if(_CurrentTexEnvSpecial[1] != newEnvStage1)
	{
		// TexEnv is special.
		_CurrentTexEnvSpecial[1] = newEnvStage1;

		_DriverGLStates.activeTextureARB(1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		// Operator Add (Arg0*Arg2+Arg1)
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
		// Arg0.
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		// Arg2.
		if( newEnvStage1 == TexEnvSpecialSpecularStage1NoText)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_ONE_MINUS_SRC_COLOR);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_PREVIOUS);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
		}
		// Arg1.
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		// Result : Texture*Previous.Alpha+Previous
		// Setup Alpha Diffuse Copy
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		// Arg2.
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_ZERO);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Arg1.
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_ZERO );
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	}
#endif
#else
	/// Support NVidia combine 4 extension to do specular map in a single pass
	if( _Extensions.NVTextureEnvCombine4 )
	{	// Ok we can do it in a single pass

		// Set Stage 1
		// Special: not the same sepcial env if there is or not texture in stage 0.
		CTexEnvSpecial		newEnvStage1;
		if( mat.getTexture(0) == NULL )
			newEnvStage1= TexEnvSpecialSpecularStage1NoText;
		else
			newEnvStage1= TexEnvSpecialSpecularStage1;
		// Test if same env as prec.
		if(_CurrentTexEnvSpecial[1] != newEnvStage1)
		{
			// TexEnv is special.
			_CurrentTexEnvSpecial[1] = newEnvStage1;

			_DriverGLStates.activeTextureARB(1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
			// Operator Add (Arg0*Arg1+Arg2*Arg3)
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD );
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD );
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR );
			// Arg1.
			if( newEnvStage1 == TexEnvSpecialSpecularStage1NoText )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_ALPHA );
			}
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR );
			// Arg3.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
			// Result : Texture*Previous.Alpha+Previous
			// Setup Alpha Diffuse Copy
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA );
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA );
			// Arg3.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		}
	}
	else if (_Extensions.ATITextureEnvCombine3)
	{
		// Ok we can do it in a single pass

		// Set Stage 1
		// Special: not the same special env if there is or not texture in stage 0.
		CTexEnvSpecial		newEnvStage1;
		if( mat.getTexture(0) == NULL )
			newEnvStage1= TexEnvSpecialSpecularStage1NoText;
		else
			newEnvStage1= TexEnvSpecialSpecularStage1;
		// Test if same env as prec.
		if(_CurrentTexEnvSpecial[1] != newEnvStage1)
		{
			// TexEnv is special.
			_CurrentTexEnvSpecial[1] = newEnvStage1;

			_DriverGLStates.activeTextureARB(1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			// Operator Add (Arg0*Arg2+Arg1)
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI );
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE_ADD_ATI );
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR );
			// Arg2.
			if( newEnvStage1 == TexEnvSpecialSpecularStage1NoText )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA );
			}
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR );
			// Result : Texture*Previous.Alpha+Previous
			// Setup Alpha Diffuse Copy
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA );
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA );
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
		}
	}
	else
#endif
	{
		// We have to do it in 2 passes
		// For Both Pass, setup correct Env.
		if( pass == 0 )
		{
			// Just display the texture
			_DriverGLStates.enableBlend(false);
			_DriverGLStates.activeTextureARB(1);
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
		}
		else
		{
// Disabled because of Intel GPU texture bug (issue 310)
// CMake options to debug
// -DDEBUG_OGL_SPECULAR_FALLBACK=ON enables this
// -DDEBUG_OGL_COMBINE43_DISABLE=ON disables GL_NV_texture_env_combine4/GL_ATI_texture_env_combine3
#ifdef DEBUG_OGL_SPECULAR_FALLBACK
			// Multiply texture1 by alpha_texture0 and display with add
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_ONE, GL_ONE);

			// Set stage 0
			_DriverGLStates.activeTextureARB(0);
			CMaterial::CTexEnv	env;

			env.Env.OpRGB = CMaterial::Replace;
			env.Env.SrcArg0RGB = CMaterial::Texture;
			env.Env.OpArg0RGB = CMaterial::SrcAlpha;

			activateTexEnvMode(0, env);

			// Set stage 1
			if( mat.getTexture(0) == NULL )
			{
				env.Env.OpRGB = CMaterial::Replace;
				env.Env.SrcArg0RGB = CMaterial::Texture;
				env.Env.OpArg0RGB = CMaterial::SrcColor;
			}
			else
			{
				env.Env.OpRGB = CMaterial::Modulate;
				env.Env.SrcArg0RGB = CMaterial::Texture;
				env.Env.OpArg0RGB = CMaterial::SrcColor;

				env.Env.SrcArg1RGB = CMaterial::Previous;
				env.Env.OpArg1RGB = CMaterial::SrcColor;
			}

			activateTexEnvMode(1, env);
#endif // DEBUG_OGL_SPECULAR_FALLBACK
		}
	}
}

// ***************************************************************************
void			CDriverGL::endSpecularMultiPass()
{
	H_AUTO_OGL(CDriverGL_endSpecularMultiPass)
	// End specular , only if not Batching mode.
	if(!_SpecularBatchOn)
		setupSpecularEnd();
}

// a functor that can is used to generate a cube map used for specular / diffuse lighting
struct CSpecCubeMapFunctor : ICubeMapFunctor
{
	CSpecCubeMapFunctor(float exp) : Exp(exp) {}
	virtual NLMISC::CRGBA operator()(const NLMISC::CVector &v)
	{
		H_AUTO_OGL(CSpecCubeMapFunctor_operator_parenthesis)
		uint8 intensity = (uint8) (255.f * ::powf(std::max(v.normed().z, 0.f), Exp));
		return NLMISC::CRGBA(intensity, intensity, intensity, intensity);
		//return Exp == 1.f ? CRGBA((uint8)(v.x*127+127), (uint8)(v.y*127+127), (uint8)(v.z*127+127), 0): CRGBA::Black;
	}
	virtual ~CSpecCubeMapFunctor() {}
	float Exp;
};

/* /// parameters for specular cube map generation
const uint MaxSpecularExp = 64;
const uint SpecularExpStep = 8;
const uint SpecularMapSize = 32; */

// ***************************************************************************
CTextureCube	*CDriverGL::getSpecularCubeMap(uint exp)
{
	H_AUTO_OGL(CDriverGL__getSpecularCubeMap)
	const uint DiffuseMapSize = 64;
	const uint SpecularMapSize = 32;
	const uint SpecularMapSizeHighExponent = 64;
	const float HighExponent = 128.f;
	const uint MaxExponent = 512;
	// this gives the cube map to use given an exponent (from 0 to 128)
	static uint16 expToCubeMap[MaxExponent];
	// this gives the exponent used by a given cube map (not necessarily ordered)
	static float cubeMapExp[] =
	{
		1.f, 4.f, 8.f, 24.f, 48.f, 128.f, 256.f, 511.f
	};
	const uint numCubeMap = sizeof(cubeMapExp) / sizeof(float);
	static bool tableBuilt = false;

	if (!tableBuilt)
	{
		for (uint k = 0; k < MaxExponent; ++k)
		{
			uint nearest = 0;
			float diff = (float) MaxExponent;
			// look for the nearest exponent
			for (uint l = 0; l < numCubeMap; ++l)
			{
				float newDiff = ::fabsf(k - cubeMapExp[l]);
				if (newDiff < diff)
				{
					diff = newDiff;
					nearest = l;
				}
			}
			expToCubeMap[k] = uint16(nearest);
		}
		tableBuilt = true;
	}

	if (_SpecularTextureCubes.empty())
	{
		_SpecularTextureCubes.resize(MaxExponent);
	}

	NLMISC::clamp(exp, 1u, (MaxExponent - 1));

	uint cubeMapIndex = expToCubeMap[exp];
	nlassert(cubeMapIndex < numCubeMap);

	if (_SpecularTextureCubes[cubeMapIndex] != NULL) // has the cube map already been cted ?
	{
		return _SpecularTextureCubes[cubeMapIndex];
	}
	else // build the cube map
	{
		float exponent	  = cubeMapExp[cubeMapIndex];
		CSpecCubeMapFunctor scmf(exponent);
		const uint bufSize = 128;
		char name[bufSize];
		NLMISC::smprintf(name, bufSize, "#SM%d", cubeMapIndex);
		CTextureCube *tc;
		if (exponent == 1)
		{
			tc = BuildCubeMap(DiffuseMapSize,  scmf, false, name);
		}
		else
		{
			tc = BuildCubeMap(exponent >= HighExponent ? SpecularMapSizeHighExponent
													  : SpecularMapSize,
							  scmf,
							  false,
							  name);
		}

		static const CTextureCube::TFace numToFace[] =
		{ CTextureCube::positive_x,
		  CTextureCube::negative_x,
		  CTextureCube::positive_y,
		  CTextureCube::negative_y,
		  CTextureCube::positive_z,
		  CTextureCube::negative_z
		};

		if (exponent != 1.f)
		{
			// force 16 bit for specular part, 32 bit if exponent is 1 (diffuse part)
			for (uint k = 0; k < 6; ++k)
			{
				nlassert(tc->getTexture(numToFace[k]));
				tc->getTexture(numToFace[k])->setUploadFormat(ITexture::RGB565);
			}
		}

		_SpecularTextureCubes[cubeMapIndex] = tc;
		return tc;
	}
}

// ***************************************************************************
sint			CDriverGL::beginPPLMultiPass()
{
	H_AUTO_OGL(CDriverGL_beginPPLMultiPass)
	#ifdef NL_DEBUG
		nlassert(supportPerPixelLighting(true)); // make sure the hardware can do that
	#endif
	return 1;
}

// ***************************************************************************
void			CDriverGL::setupPPLPass(uint pass)
{
	H_AUTO_OGL(CDriverGL_setupPPLPass)
	const CMaterial &mat= *_CurrentMaterial;

	nlassert(pass == 0);

/*	ITexture *tex0 = getSpecularCubeMap(1);
	if (tex0) setupTexture(*tex0);
	activateTexture(0, tex0);


	static CMaterial::CTexEnv	env;
	env.Env.SrcArg0Alpha = CMaterial::Diffuse;
	env.Env.SrcArg1Alpha = CMaterial::Constant;
	env.Env.SrcArg0RGB = CMaterial::Diffuse;
	env.Env.SrcArg1RGB = CMaterial::Constant;
	env.Env.OpRGB = CMaterial::Replace;
	env.Env.OpAlpha = CMaterial::Replace;
	activateTexEnvMode(0, env);

	return;*/

	ITexture *tex0 = getSpecularCubeMap(1);
	if (tex0) setupTexture(*tex0);
	ITexture *tex2 = getSpecularCubeMap((uint) mat.getShininess());
	if (tex2) setupTexture(*tex2);
	if (mat.getTexture(0)) setupTexture(*mat.getTexture(0));

	// tex coord 0 = texture coordinates
	// tex coord 1 = normal in tangent space
	// tex coord 2 = half angle vector in tangent space

	activateTexture(0, tex0);
	activateTexture(1, mat.getTexture(0));
	activateTexture(2, tex2);

	for (uint k = 3; k < inlGetNumTextStages(); ++k)
	{
		activateTexture(k, NULL);
	}

	// setup the tex envs

	// Stage 0 is rgb = DiffuseCubeMap * LightColor + DiffuseGouraud * 1
	if(_CurrentTexEnvSpecial[0] != TexEnvSpecialPPLStage0)
	{
		// TexEnv is special.
		_CurrentTexEnvSpecial[0] = TexEnvSpecialPPLStage0;
		_DriverGLStates.activeTextureARB(0);

#ifdef USE_OPENGLES
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		// Arg0 = Diffuse read in cube map
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		// Arg1 = Light color
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
		// Arg2 = Primary color (other light diffuse and
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
#else
		if (_Extensions.NVTextureEnvCombine4)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = Diffuse read in cube map
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = Light color
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = Primary color (other light diffuse and
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = White (= ~ Black)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
		}
		else // use ATI extension
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI);
			// Arg0 = Diffuse read in cube map
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = Light color
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = Primary color (other light diffuse and
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
		}
#endif
	}
	activateTexEnvColor(0, _PPLightDiffuseColor);

	// Stage 1
	static CMaterial::CTexEnv	env;
	env.Env.SrcArg1Alpha = CMaterial::Diffuse;
	activateTexEnvMode(1, env);

	// Stage 2 is rgb = SpecularCubeMap * SpecularLightColor + Prec * 1
	// alpha = prec alpha

	if(_CurrentTexEnvSpecial[2] != TexEnvSpecialPPLStage2)
	{
		// TexEnv is special.
		_CurrentTexEnvSpecial[2] = TexEnvSpecialPPLStage2;
		_DriverGLStates.activeTextureARB(2);

#ifdef USE_OPENGLES
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		//== colors ==
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		// Arg0 = Specular read in cube map
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		// Arg2 = Light color
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
		// Arg1 = Primary color ( + other light diffuse)
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		//== alpha ==
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		// Arg0 = PREVIOUS ALPHA
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_COLOR);
		// Arg2 = 1
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_ZERO);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_ONE_MINUS_SRC_COLOR);
		// Arg1 = 0
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_ZERO);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_COLOR);
#else
		if (_Extensions.NVTextureEnvCombine4)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
			//== colors ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = Specular read in cube map
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = Light color
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = Primary color ( + other light diffuse )
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = White (= ~ Black)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);

			//== alpha ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = PREVIOUS ALPHA
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_COLOR);
			// Arg1 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_COLOR);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_COLOR);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_COLOR);
		}
		else // ATI EnvCombine3
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			//== colors ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI);
			// Arg0 = Specular read in cube map
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = Light color
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = Primary color ( + other light diffuse)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

			//== alpha ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE_ADD_ATI);
			// Arg0 = PREVIOUS ALPHA
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_COLOR);
			// Arg2 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_ONE_MINUS_SRC_COLOR);
			// Arg1 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_COLOR);
		}
#endif
	}
	activateTexEnvColor(2, _PPLightSpecularColor);

}

// ***************************************************************************
void			CDriverGL::endPPLMultiPass()
{
	H_AUTO_OGL(CDriverGL_endPPLMultiPass)
	// nothing to do there ...
}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
sint			CDriverGL::beginPPLNoSpecMultiPass()
{
	H_AUTO_OGL(CDriverGL_beginPPLNoSpecMultiPass)
	#ifdef NL_DEBUG
		nlassert(supportPerPixelLighting(false)); // make sure the hardware can do that
	#endif
	return 1;
}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
void			CDriverGL::setupPPLNoSpecPass(uint pass)
{
	H_AUTO_OGL(CDriverGL_setupPPLNoSpecPass)
	const CMaterial &mat= *_CurrentMaterial;

	nlassert(pass == 0);

	ITexture *tex0 = getSpecularCubeMap(1);
	if (tex0) setupTexture(*tex0);

	if (mat.getTexture(0)) setupTexture(*mat.getTexture(0));

	// tex coord 0 = texture coordinates
	// tex coord 1 = normal in tangent space

	activateTexture(0, tex0);
	activateTexture(1, mat.getTexture(0));

	for (uint k = 2; k < inlGetNumTextStages(); ++k)
	{
		activateTexture(k, NULL);
	}

	// setup the tex envs

	// Stage 0 is rgb = DiffuseCubeMap * LightColor + DiffuseGouraud * 1 (TODO : EnvCombine3)
	if(_CurrentTexEnvSpecial[0] != TexEnvSpecialPPLStage0)
	{
		// TexEnv is special.
		_CurrentTexEnvSpecial[0] = TexEnvSpecialPPLStage0;
		_DriverGLStates.activeTextureARB(0);

#ifdef USE_OPENGLES
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		// Arg0 = Diffuse read in cube map alpha
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		// Arg2 = Light color
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
		// Arg1 = Primary color (other light diffuse and
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
#else
		if (_Extensions.NVTextureEnvCombine4)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = Diffuse read in cube map alpha
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = Light color
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = Primary color (other light diffuse and
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = White (= ~ Black)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI);
			// Arg0 = Diffuse read in cube map alpha
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = Light color
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = Primary color (other light diffuse and
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
		}
#endif
	}
	activateTexEnvColor(0, _PPLightDiffuseColor);

	// Stage 1
	static CMaterial::CTexEnv	env;
	env.Env.SrcArg1Alpha = CMaterial::Diffuse;
	activateTexEnvMode(1, env);

}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
void			CDriverGL::endPPLNoSpecMultiPass()
{
	H_AUTO_OGL(CDriverGL_endPPLNoSpecMultiPass)
	// nothing to do there ...
}

// ***************************************************************************
/* sint		CDriverGL::beginCausticsMultiPass(const CMaterial &mat)
{
	nlassert(mat.getShader() == CMaterial::Caustics);
	if (!_Extensions.ARBTextureCubeMap) return 1;
	switch (inlGetNumTextStages())
	{
		case 1: return 3;
		case 2: return 2;
		default:
				return 1;
	}
}*/

// ***************************************************************************
/*inline void		CDriverGL::setupCausticsFirstTex(const CMaterial &mat)
{
	/// setup texture 0
	activateTexture(0, mat.getTexture(0));

	/// texture environment 0
	setTextureEnvFunction(0, mat);

	/// texture matrix 0
	setupUserTextureMatrix(0, mat);
}

// ***************************************************************************
inline void		CDriverGL::setupCausticsSecondTex(uint stage)
{
	activateTexture(stage, mat.getTexture(0));
	_CausticCubeMap
}

// ***************************************************************************
void		CDriverGL::setupCausticsPass(const CMaterial &mat, uint pass)
{
	nlassert(mat.getShader() == CMaterial::Caustics);

	if (inlGetNumTextStages() == 1 || !_Extensions.ARBTextureCubeMap)
	{
		setupCausticsFirstTex(mat);
	}
	else
	if (inlGetNumTextStages() >= 3) /// do it in one pass
	{
		nlassert(pass == 0);

		setupCausticsFirstTex(mat);
	}
	else if (inlGetNumTextStages() == 2) /// do in in 2 pass
	{
		nlassert(pass < 2);
		if (pass == 0)
		{
			setupCausticsFirstTex(mat);
		}
		else /// caustics setup
		{
			/// setup additif blending
			_DriverGLStates.enableBlend();
			_DriverGLStates.blendFunc(pShader->SrcBlend, pShader->DstBlend);
		}
	}
}

// ***************************************************************************
void		CDriverGL::endCausticsMultiPass(const CMaterial &mat)
{
	nlassert(mat.getShader() == CMaterial::Caustics);

}
*/

// ***************************************************************************
sint		CDriverGL::beginCloudMultiPass ()
{
	H_AUTO_OGL(CDriverGL_beginCloudMultiPass )
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);
	return 1;
}

// ***************************************************************************
void		CDriverGL::setupCloudPass (uint /* pass */)
{
	H_AUTO_OGL(CDriverGL_setupCloudPass )
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);

	const CMaterial &mat= *_CurrentMaterial;

	activateTexture(0, mat.getTexture(0));
	activateTexture(1, mat.getTexture(0));

	if (_CurrentTexEnvSpecial[0] != TexEnvSpecialCloudStage0)
	{
#ifndef USE_OPENGLES
		if (_Extensions.NVTextureEnvCombine4)
		{
			_CurrentTexEnvSpecial[0] = TexEnvSpecialCloudStage0;
			_CurrentTexEnvSpecial[1] = TexEnvSpecialCloudStage1;

			// Setup 1st Stage
			_DriverGLStates.activeTextureARB(0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
			//== colors ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);

			//== alpha ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = AT0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE0_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg1 = AWPOS
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg2 = AT1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_TEXTURE1_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg3 = 1-AWPOS
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_ONE_MINUS_SRC_ALPHA);

			// Setup 2nd Stage
			_DriverGLStates.activeTextureARB(1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
			//== colors ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			// Arg1 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);

			//== alpha ==
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = AT0*AWPOS+AT1*(1-AWPOS)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg1 = AINT
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
			activateTexEnvColor (1, mat.getColor());
		}
		else if (ATICloudShaderHandle)
		{
			// TODO : for now the state is not cached in _CurrentTexEnvSpecial
			nglBindFragmentShaderATI(ATICloudShaderHandle);
			glEnable(GL_FRAGMENT_SHADER_ATI);
			float cst[4] = { 0.f, 0.f, 0.f, mat.getColor().A / 255.f };
			nglSetFragmentShaderConstantATI(GL_CON_0_ATI, cst);
		}
		else
		{
			_DriverGLStates.activeTextureARB(0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			// Operator.
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INTERPOLATE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_INTERPOLATE_EXT);
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE0_ARB );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_TEXTURE1_ARB );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			_DriverGLStates.activeTextureARB(1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			// Operator.
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_CONSTANT_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
		}
#endif
	}
	if (_Extensions.NVTextureEnvCombine4)
		activateTexEnvColor (1, mat.getColor());
}

// ***************************************************************************
void		CDriverGL::endCloudMultiPass()
{
	H_AUTO_OGL(CDriverGL_endCloudMultiPass)
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);
	if (ATICloudShaderHandle)
	{
#ifndef USE_OPENGLES
		glDisable(GL_FRAGMENT_SHADER_ATI);
#endif
	}
}

// ***************************************************************************
sint CDriverGL::beginWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL_beginWaterMultiPass)
	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);
	return 1;
}


// ***************************************************************************
/** water setup for ATI
  */
void CDriverGL::setupWaterPassR200(const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL_setupWaterPassR200);

#ifndef USE_OPENGLES
	uint k;
	ITexture *tex = mat.getTexture(0);
	if (tex)
	{
//		if (tex->isBumpMap())
//		{
//			CTextureBump *tb = static_cast<CTextureBump *>(tex);
//		}
		setupTexture(*tex);
		activateTexture(0, tex);
	}
	tex = mat.getTexture(1);
	if (tex)
	{
//		if (tex->isBumpMap())
//		{
//			CTextureBump *tb = static_cast<CTextureBump *>(tex);
//		}
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
	for (k = 4; k < inlGetNumTextStages(); ++k)
	{
		activateTexture(k, NULL);
	}
	if (mat.getTexture(3) != NULL) // is there a diffuse map ?
	{
		nglBindFragmentShaderATI(ATIWaterShaderHandle);
	}
	else
	{
		nglBindFragmentShaderATI(ATIWaterShaderHandleNoDiffuseMap);
	}
	glEnable(GL_FRAGMENT_SHADER_ATI);

	// set constants
	if (mat.getTexture(0) && mat.getTexture(0)->isBumpMap())
	{
		float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(0))->getNormalizationFactor();
		float cst[4] = { factor, factor, factor, 0.f };
		nglSetFragmentShaderConstantATI(GL_CON_0_ATI, cst);
	}
	else
	{
		float cst[4] = { 1.f, 1.f, 1.f, 0.f };
		nglSetFragmentShaderConstantATI(GL_CON_0_ATI, cst);
	}
	//
	if (mat.getTexture(1) && mat.getTexture(1)->isBumpMap())
	{
		float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(1))->getNormalizationFactor();
		float cst[4] = { factor, factor, factor, 0.f };
		nglSetFragmentShaderConstantATI(GL_CON_1_ATI, cst);
	}
	else
	{
		float cst[4] = { 1.f, 1.f, 1.f, 0.f };
		nglSetFragmentShaderConstantATI(GL_CON_0_ATI, cst);
	}
#endif
}

// ***************************************************************************
/** water setup for ARB_fragment_program
  */
void CDriverGL::setupWaterPassARB(const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL_setupWaterPassARB);

#ifndef USE_OPENGLES
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
	for (k = 4; k < inlGetNumTextStages(); ++k)
	{
		activateTexture(k, NULL);
	}
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ARBWaterShader[(_FogEnabled ? 1 : 0) | (mat.getTexture(3) != NULL ? 2 : 0)]);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	// setup the constant
	if (mat.getTexture(0) && mat.getTexture(0)->isBumpMap())
	{
		float factor = 0.25f * NLMISC::safe_cast<CTextureBump *>(mat.getTexture(0))->getNormalizationFactor();
		nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 2.f * factor, -1.f * factor, 0.f, 0.f); // scale_bias from [0, 1] to [-1, 1] and factor applied
	}
	else
	{
		nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 2.f, -1.f, 0.f, 0.f); // scale_bias from [0, 1] to [-1, 1] and factor applied
	}

	// setup the constant
	if (mat.getTexture(1) && mat.getTexture(1)->isBumpMap())
	{
		float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(1))->getNormalizationFactor();
		nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, 2.f * factor, -1.f * factor, 0.f, 0.f); // scale_bias from [0, 1] to [-1, 1] and factor applied
	}
	else
	{
		nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, 2.f, -1.f, 0.f, 0.f); // scale_bias from [0, 1] to [-1, 1] and factor applied
	}

	if (_FogEnabled)
	{
		if (_FogStart == _FogEnd)
		{
			nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, 0.f, 0.f, 0.f, 0.f);
		}
		else
		{
			/** Unfortunately, the EXT_vertex_shader extension has to output the fog values in the [0, 1] range to work with the standard pipeline.
			  * So we must add a special path for this case, where the fog coordinate is 'unscaled' again.
			  * NB : this is fixed in later drivers (from 6.14.10.6343), so check this
			  */
			if (_Extensions.EXTVertexShader && !_ATIFogRangeFixed)
			{
				nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, 1.f, 0.f, 0.f, 0.f);
			}
			else
			{
				nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, - 1.f/  (_FogEnd - _FogStart), _FogEnd / (_FogEnd - _FogStart), 0.f, 0.f);
			}
		}
	}
#endif
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
void CDriverGL::setupWaterPassNV20(const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL_setupWaterPassNV20);

#ifndef USE_OPENGLES
	static bool setupDone = false;
	static CMaterial::CTexEnv texEnvReplace;
	static CMaterial::CTexEnv texEnvModulate;

	if (!setupDone)
	{
		texEnvReplace.Env.OpRGB   = CMaterial::Replace;
		texEnvReplace.Env.OpAlpha = CMaterial::Replace;
		// use default setup for texenv modulate
		setupDone = true;
	}

	// activate the textures & set the matrixs
	ITexture *tex = mat.getTexture(0);
	if (tex)
	{
		setupTexture(*tex);
		activateTexture(0, tex);
		_DriverGLStates.activeTextureARB(1);
		if (tex->isBumpMap())
		{
			CTextureBump *tb = static_cast<CTextureBump *>(tex);
			// set the matrix for the texture shader
			float factor = tb->getNormalizationFactor();
			float tsMatrix[4] = { 0.25f * factor, 0.f, 0.f, 0.25f * factor };
			glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, tsMatrix);
		}
		else
		{
			glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, IdentityTexMat);
		}
	}
	tex = mat.getTexture(1);
	if (tex)
	{
		setupTexture(*tex);
		activateTexture(1, tex);
		_DriverGLStates.activeTextureARB(2);
		if (tex->isBumpMap())
		{
			CTextureBump *tb = static_cast<CTextureBump *>(tex);
			// set the matrix for the texture shader
			float factor = tb->getNormalizationFactor();
			float tsMatrix[4] = { factor, 0.f, 0.f, factor };
			glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, tsMatrix);
		}
		else
		{
			glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, IdentityTexMat);
		}
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
	for (uint k = 4; k < inlGetNumTextStages(); ++k)
	{
		activateTexture(k, NULL);
	}

	// setup the texture shaders
	enableNVTextureShader(true);
	activateTexEnvMode(0, texEnvReplace);
	activateTexEnvMode(1, texEnvReplace);
	nlctassert(IDRV_MAT_MAXTEXTURES == 4); // if this value changes, may have to change the arrays WaterNoDiffuseTexAddrMode & WaterTexAddrMode
	if (mat.getTexture(3) == NULL)
	{
		setTextureShaders(WaterNoDiffuseTexAddrMode, mat._Textures);
		activateTexEnvMode(2, texEnvReplace);
	}
	else
	{
		setTextureShaders(WaterTexAddrMode, mat._Textures);
		activateTexEnvMode(2, texEnvReplace);
		activateTexEnvMode(3, texEnvModulate);
	}
#endif
}

// ***************************************************************************
void CDriverGL::setupWaterPass(uint /* pass */)
{
	H_AUTO_OGL(CDriverGL_setupWaterPass)
	nlassert (_CurrentMaterial);
	CMaterial &mat = *_CurrentMaterial;
	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);

	if (_Extensions.NVTextureShader)
	{
		setupWaterPassNV20(mat);
	}
	else if (ARBWaterShader[0])
	{
		setupWaterPassARB(mat);
	}
	else if (ATIWaterShaderHandleNoDiffuseMap)
	{
		setupWaterPassR200(mat);
	}
}

// ***************************************************************************
void CDriverGL::endWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL_endWaterMultiPass);

#ifndef USE_OPENGLES
	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);
	// NB : as fragment shaders / programs bypass the texture envs, no special env enum is added (c.f CTexEnvSpecial)
	if (_Extensions.NVTextureShader) return;
	if (ARBWaterShader[0])
	{
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
	else if (ATIWaterShaderHandleNoDiffuseMap)
	{
		glDisable(GL_FRAGMENT_SHADER_ATI);
	}
#endif
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
