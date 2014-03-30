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
#include "nel/3d/dynamic_material.h"

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
void CDriverGL3::setTextureEnvFunction(uint stage, CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL3_setTextureEnvFunction)
	ITexture	*text= mat.getTexture(uint8(stage));
	if (text)
	{
		CMaterial::CTexEnv	&env= mat._TexEnvs[stage];

		// Activate the env for this stage.
		// NB: Thoses calls use caching.
		activateTexEnvMode(stage, env);
		activateTexEnvColor(stage, env);

		// Activate texture generation mapping
		_DriverGLStates.activeTexture(stage);
		if (mat.getTexCoordGen (stage))
		{
			// set mode and enable.
			CMaterial::TTexCoordGenMode	mode= mat.getTexCoordGenMode(stage);
			if (mode==CMaterial::TexCoordGenReflect)
			{
				// Cubic or normal ?
				if (text->isTextureCube ())
					_DriverGLStates.setTexGenMode (stage, GL_REFLECTION_MAP_ARB);
				else
					_DriverGLStates.setTexGenMode (stage, GL_SPHERE_MAP);
			}
			else if (mode==CMaterial::TexCoordGenObjectSpace)
			{
				_DriverGLStates.setTexGenMode (stage, GL_OBJECT_LINEAR);
			}
			else if (mode==CMaterial::TexCoordGenEyeSpace)
			{
				_DriverGLStates.setTexGenMode (stage, GL_EYE_LINEAR);
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
				_UserTexMat[ k ] = mat.getUserTexMat(k);
				_UserTexMatEnabled |= shiftMask;
			}
			else
			{
				/// check if matrix disabled
				if (
					(newMask & shiftMask) != (_UserTexMatEnabled & shiftMask)
				  )
				{
					_UserTexMat[ k ].identity();
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
				_UserTexMat[ k ].identity();
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
	// Lightmap and Specular work only if at least 2 text stages.
	case CMaterial::LightMap:
		return (inlGetNumTextStages() >= 2) ? CMaterial::LightMap : CMaterial::Normal;
	case CMaterial::Specular:
		return (inlGetNumTextStages() >= 2) ? CMaterial::Specular : CMaterial::Normal;
	default:
		return shader;
	}
}

// --------------------------------------------------
bool CDriverGL3::setupMaterial(CMaterial& mat)
{
	H_AUTO_OGL(CDriverGL3_setupMaterial)

	if (mat.getDynMat() != NULL)
	{
		_CurrentMaterial = &mat; 
		return true;
	}

	CShaderGL3*	pShader;
	GLenum		glenum = GL_ZERO;
	uint32		touched = mat.getTouched();
	uint		stage;

	// profile.
	_NbSetupMaterialCall++;


	// 0. Retrieve/Create driver shader.
	//==================================
	if (!mat._MatDrvInfo)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItMatDrvInfoPtrList		it= _MatDrvInfos.insert(_MatDrvInfos.end(), (NL3D::IMaterialDrvInfos*)NULL);
		// create and set iterator, for future deletion.
		*it= mat._MatDrvInfo= new CShaderGL3(this, it);

		// Must create all OpenGL shader states.
		touched= IDRV_TOUCHED_ALL;
	}
	pShader=static_cast<CShaderGL3*>((IMaterialDrvInfos*)(mat._MatDrvInfo));

	// 1. Setup modified fields of material.
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
		mat.clearTouched(0xFFFFFFFF);
	}

	// Now we can get the supported shader from the cache.
	CMaterial::TShader matShader = pShader->SupportedShader;

	// if the shader has changed since last time
	if (matShader != _CurrentMaterialSupportedShader)
	{
		// if old was lightmap, restore standard lighting
		if (_CurrentMaterialSupportedShader==CMaterial::LightMap)
			setupLightMapDynamicLighting(false);

		// if new is lightmap, setup dynamic lighting
		if (matShader==CMaterial::LightMap)
			setupLightMapDynamicLighting(true);
	}

	// setup the global
	_CurrentMaterialSupportedShader= matShader;

	// 2. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame. (need to test if touched).
	// Must separate texture setup and texture activation in 2 "for"...
	// because setupTexture() may disable all stage.
	if (matShader != CMaterial::Water)
	{
		for (stage=0 ; stage<inlGetNumTextStages() ; stage++)
		{
			ITexture	*text= mat.getTexture(uint8(stage));
			if (text != NULL && !setupTexture(*text))
				return false;
		}
	}
	// Here, for Lightmap materials, setup the lightmaps.
	if (matShader == CMaterial::LightMap)
	{
		for (stage = 0; stage < mat._LightMaps.size(); stage++)
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
	if (matShader != CMaterial::LightMap
		&& matShader != CMaterial::PerPixelLighting
		/* && matShader != CMaterial::Caustics	*/
		&& matShader != CMaterial::Cloud
		&& matShader != CMaterial::Water
		&& matShader != CMaterial::Specular
	  )
	{
		for (stage=0 ; stage<inlGetNumTextStages() ; stage++)
		{
			ITexture	*text= mat.getTexture(uint8(stage));

			// activate the texture, or disable texturing if NULL.
			activateTexture(stage,text);

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
		if (blend)
			_DriverGLStates.blendFunc(pShader->SrcBlend, pShader->DstBlend);

		// Double Sided Part.
		//===================
		// NB: inverse state: DoubleSided <=> !CullFace.
		uint32	twoSided= mat.getFlags()&IDRV_MAT_DOUBLE_SIDED;
		_DriverGLStates.enableCullFace(twoSided==0);


		// Alpha Test Part.
		//=================
		uint32	alphaTest= mat.getFlags()&IDRV_MAT_ALPHA_TEST;
		_DriverGLStates.enableAlphaTest(alphaTest);
		if (alphaTest)
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
		if ((mat.getFlags() & IDRV_MAT_LIGHTING) == 0)
			disableAllLights();

		if (mat.getFlags()&IDRV_MAT_LIGHTING)
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
			// Restaure fog state to its current value
			_DriverGLStates.enableFog(_FogEnabled);
		}


		_CurrentMaterial=&mat;
	}

	// 4. Misc
	//=====================================

	// Textures user matrix
	if (matShader == CMaterial::Normal)
	{
		setupUserTextureMatrix(inlGetNumTextStages(), mat);
	}
	else // deactivate texture matrix
	{
		disableUserTextureMatrix();
	}

	// 5. Set up the program
	// =====================
	bool programOK = setupProgram(mat);
	return programOK;
}

// ***************************************************************************
sint			CDriverGL3::beginMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginMultiPass)

	if (_CurrentMaterial->getDynMat() != NULL)
		return _CurrentMaterial->getDynMat()->getPassCount();

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
bool CDriverGL3::setupPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupPass)
	
	if (_CurrentMaterial->getDynMat() != NULL)
		return setupDynMatPass(pass);
	
	switch(_CurrentMaterialSupportedShader)
	{
	case CMaterial::Normal:
		setupNormalPass();
		break;
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
	}

	return true;
}

// ***************************************************************************
void			CDriverGL3::endMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endMultiPass)
	
	if (_CurrentMaterial->getDynMat() != NULL)
		return;

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

bool CDriverGL3::setupDynMatPass(uint pass)
{

	if (!setupDynMatProgram(*_CurrentMaterial, pass))
		return false;
	
	if ((_CurrentMaterial->getFlags() & IDRV_MAT_DOUBLE_SIDED) != 0)
		_DriverGLStates.enableCullFace(false);
	else
		_DriverGLStates.enableCullFace(true);

	CDynMaterial *m = _CurrentMaterial->getDynMat();
	SRenderPass *currentPass = m->getPass(pass);

	IProgram *p;

	IProgram* programs[ 2 ];
	programs[ 0 ] = currentProgram.vp;
	programs[ 1 ] = currentProgram.pp;
	
	IDriver::TProgram type[ 2 ];
	type[ 0 ] = IDriver::VertexProgram;
	type[ 1 ] = IDriver::PixelProgram;

	for (uint32 j = 0; j < 2; j++)
	{
		p = programs[ j ];

		for (uint32 i = 0; i < currentPass->count(); i++)
		{
			const SDynMaterialProp *prop = currentPass->getProperty(i);		
			int loc = getUniformLocation(type[ j ], prop->prop.c_str());
			if (loc == -1)
				continue;

			switch(prop->type)
			{
			case SDynMaterialProp::Float:
				setUniform1f(type[ j ], loc, prop->value.toFloat());
				break;

			case SDynMaterialProp::Int:
				setUniform1i(type[ j ], loc, prop->value.toInt());
				break;

			case SDynMaterialProp::Uint:
				setUniform1ui(type[ j ], loc, prop->value.toUInt());
				break;

			case SDynMaterialProp::Color:
				{
					float v[ 4 ];
					prop->value.getVector4(v);
				
					for (int k = 0; k < 4; k++)
						v[ k ] = v[ k ] / 255.0f;
				
					setUniform4f(type[ j ], loc, v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ]);
				}
				break;

			case SDynMaterialProp::Vector4:
				{
					float v[ 4 ];
					prop->value.getVector4(v);
					setUniform4f(type[ j ], loc, v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ]);
				}
				break;

			case SDynMaterialProp::Matrix4:
				{
					float m[ 16 ];
					prop->value.getMatrix4(m);
					setUniform4x4f(type[ j ], loc, m);
					break;
				}

			case SDynMaterialProp::Texture:
				break;
			}
		}

		////////////////////////////////// Set up some standard uniforms //////////////////////////////////

		int loc = -1;
		loc = getUniformLocation(type[ j ], "mvpMatrix");
		if (loc != -1)
		{
			CMatrix mat = _GLProjMat * _ModelViewMatrix;
			setUniform4x4f(type[ j ], loc, mat);
		}

		loc = getUniformLocation(type[ j ], "mvMatrix");
		if (loc != -1)
		{
			setUniform4x4f(type[ j ], loc, _ModelViewMatrix);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////

	}

	return true;
}

void CDriverGL3::setupNormalPass()
{
	const CMaterial &mat = *_CurrentMaterial;
	
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		// Set constant
		int cl = currentProgram.pp->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + i));
		if (cl != -1)
		{
			GLfloat glCol[ 4 ];
			convColor(mat._TexEnvs[ i ].ConstantColor, glCol);
			setUniform4f(IDriver::PixelProgram, cl, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
		}

		// Set texture
		ITexture *t = mat.getTexture(i);
		if (t == NULL)
			continue;
		
		int index = currentProgram.pp->getUniformIndex(CProgramIndex::TName(CProgramIndex::Sampler0 + i));
		if (index == -1)
			continue;
		
		setUniform1i(IDriver::PixelProgram, index, i);
	}
}

// ***************************************************************************
void CDriverGL3::computeLightMapInfos (const CMaterial &mat)
{
	H_AUTO_OGL(CDriverGL3_computeLightMapInfos)
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

	// Number of pass.
	_NLightMapPass = (_NLightMaps + _NLightMapPerPass-1)/(_NLightMapPerPass);

	// NB: _NLightMaps==0 means there is no lightmaps at all.
}

// ***************************************************************************
sint CDriverGL3::beginLightMapMultiPass ()
{
	H_AUTO_OGL(CDriverGL3_beginLightMapMultiPass)
	const CMaterial &mat= *_CurrentMaterial;

	// compute how many lightmap and pass we must process.
	computeLightMapInfos (mat);

	// always enable lighting for lightmap (because of dynamic light)
	_DriverGLStates.enableLighting(true);

	// if the dynamic lightmap light has changed since the last render (should not happen), resetup
	// normal way is that setupLightMapDynamicLighting() is called in setupMaterial() if shader different from prec
	if (_LightMapDynamicLightDirty)
		setupLightMapDynamicLighting(true);

	// reset Ambient and specular lighting
	static	uint32	packedColorBlack= CRGBA(0,0,0,255).getPacked();
	static	GLfloat glcolBlack[4]= {0,0,0,1};
	// lightmap get no specular/ambient. Emissive and Diffuse are setuped in setupLightMapPass()
	_DriverGLStates.setAmbient(packedColorBlack, glcolBlack);
	_DriverGLStates.setSpecular(packedColorBlack, glcolBlack);

	// Manage too if no lightmaps.
	return	std::max (_NLightMapPass, (uint)1);
}

// ***************************************************************************
void			CDriverGL3::setupLightMapPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupLightMapPass)
	const CMaterial &mat= *_CurrentMaterial;

	// common colors
	static	uint32	packedColorBlack= CRGBA(0,0,0,255).getPacked();
	static	GLfloat glcolBlack[4]= {0.f,0.f,0.f,1.f};
	static	uint32	packedColorWhite= CRGBA(255,255,255,255).getPacked();
	static	GLfloat glcolWhite[4]= {1.f,1.f,1.f,1.f};
	static	uint32	packedColorGrey= CRGBA(128,128,128,128).getPacked();
	static	GLfloat glcolGrey[4]= {0.5f,0.5f,0.5f,1.f};

	// No lightmap or all blacks??, just setup "black texture" for stage 0.
	if (_NLightMaps==0)
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
		_DriverGLStates.activeTexture(0);
		_DriverGLStates.setTexGenMode(0, 0);

		// And disable other stages.
		for (uint stage = 1; stage < inlGetNumTextStages(); stage++)
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
	{
		uint32	r=0;
		uint32	g=0;
		uint32	b=0;
		// sum only the ambient of lightmaps that will be drawn this pass
		for (uint sa=0;sa<nstages-1;sa++)
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
	for (uint stage= 0; stage<inlGetNumTextStages(); stage++)
	{
		// if must setup a lightmap stage.
		if (stage<nstages-1)
		{
			// setup lightMap.
			uint	whichLightMap= _LightMapLUT[lmapId];
			// get text and factor.
			ITexture *text	 = mat._LightMaps[whichLightMap].Texture;
			CRGBA lmapFactor = mat._LightMaps[whichLightMap].Factor;
			// Modulate the factor with LightMap compression Diffuse
			CRGBA lmcDiff= mat._LightMaps[whichLightMap].LMCDiffuse;

			lmapFactor.R = (uint8)(((uint32)lmapFactor.R  * ((uint32)lmcDiff.R+(lmcDiff.R>>7))) >>8);
			lmapFactor.G = (uint8)(((uint32)lmapFactor.G  * ((uint32)lmcDiff.G+(lmcDiff.G>>7))) >>8);
			lmapFactor.B = (uint8)(((uint32)lmapFactor.B  * ((uint32)lmcDiff.B+(lmcDiff.B>>7))) >>8);
			lmapFactor.A = 255;

			activateTexture(stage,text);

			// If texture not NULL, Change texture env fonction.
			//==================================================
			if (text)
			{
				static CMaterial::CTexEnv	stdEnv;

				{
					// Here, we are sure that texEnvCombine4 or texEnvCombine3 is OK.
					// nlassert(_Extensions.ATITextureEnvCombine3);

					// setup constant color with Lightmap factor.
					stdEnv.ConstantColor=lmapFactor;

					int cl = currentProgram.pp->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + stage));
					if (cl != -1)
					{
						GLfloat glCol[ 4 ];
						convColor(lmapFactor, glCol);
						setUniform4f(IDriver::PixelProgram, cl, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
					}

					int tl = currentProgram.pp->getUniformIndex(CProgramIndex::TName(CProgramIndex::Sampler0 + stage));
					if (tl != -1)
					{
						setUniform1i(IDriver::PixelProgram, tl, stage);
					}

					activateTexEnvColor(stage, stdEnv);

					// Setup env for texture stage.
					_DriverGLStates.activeTexture(stage);
					_DriverGLStates.setTexGenMode(stage, 0);

					// setup TexEnvCombine4 (ignore alpha part).
					if (_CurrentTexEnvSpecial[stage] != TexEnvSpecialLightMap)
					{
						// TexEnv is special.
						_CurrentTexEnvSpecial[stage] = TexEnvSpecialLightMap;
					}
				}
			}

			// Next lightmap.
			lmapId++;
		}
		else if (stage<nstages)
		{
			// optim: do this only for first pass, and last pass only if stage!=nLMapPerPass
			// (meaning not the same stage as preceding passes).
			if (pass==0 || (pass==_NLightMapPass-1 && stage!=_NLightMapPerPass))
			{
				// activate the texture at last stage.
				ITexture	*text= mat.getTexture(0);
				activateTexture(stage,text);

				// setup ModulateRGB/ReplaceAlpha env. (this may disable possible COMBINE4_NV setup).
				activateTexEnvMode(stage, _LightMapLastStageEnv);

				// Setup gen tex off
				_DriverGLStates.activeTexture(stage);
				_DriverGLStates.setTexGenMode(stage, 0);

				int tl = currentProgram.pp->getUniformIndex(CProgramIndex::TName(CProgramIndex::Sampler0 + stage));
				if (tl != -1)
				{
					setUniform1i(IDriver::PixelProgram, tl, stage);
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
		if (mat._LightMapsMulx2)
			_DriverGLStates.setDiffuse(packedColorGrey, glcolGrey);
		else
			_DriverGLStates.setDiffuse(packedColorWhite, glcolWhite);
	}
	// no need to reset for pass after 1, since same than prec pass (black)!
	else if (pass==1)
		_DriverGLStates.setDiffuse(packedColorBlack, glcolBlack);
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
	// ---- Reset any textures with id>=2
	uint stage = 2;
	for (; stage < inlGetNumTextStages(); stage++)
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
	_DriverGLStates.activeTexture(0);
	_DriverGLStates.setTexGenMode(0, 0);

	// ---- Stage 1 Common Setup.
	// NB don't setup the TexEnv here (stage1 setuped in setupSpecularPass() according to extensions)
	// For all cases, setup the TexCoord gen for stage1
	_DriverGLStates.activeTexture(1);

	// setup the good matrix for stage 1.
	_UserTexMat[ 1 ] = _SpecularTexMtx;

}

// ***************************************************************************
void			CDriverGL3::setupSpecularEnd()
{
	H_AUTO_OGL(CDriverGL3_setupSpecularEnd)
	// Disable Texture coord generation.
	_DriverGLStates.activeTexture(1);
	_DriverGLStates.setTexGenMode(1, 0);

	// Happiness !!! we have already enabled the stage 1
	_UserTexMat[ 1 ].identity();
}

// ***************************************************************************
sint			CDriverGL3::beginSpecularMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginSpecularMultiPass)
	const CMaterial &mat= *_CurrentMaterial;

	// activate the 2 textures here
	uint	stage;
	uint	numStages= std::min((uint)2, inlGetNumTextStages());
	for (stage=0 ; stage<numStages; stage++)
	{
		ITexture	*text= mat.getTexture(uint8(stage));

		// activate the texture, or disable texturing if NULL.
		activateTexture(stage,text);
	}

	// End specular , only if not Batching mode.
	if (!_SpecularBatchOn)
		setupSpecularBegin();

	// Manage the rare case when the SpecularMap is not provided (fault of graphist).
	if (mat.getTexture(1)==NULL)
		return 1;
	
	return 1;
}

// ***************************************************************************
void			CDriverGL3::setupSpecularPass(uint pass)
{
	H_AUTO_OGL(CDriverGL3_setupSpecularPass)
	const CMaterial &mat= *_CurrentMaterial;

	// Manage the rare case when the SpecularMap is not provided (error of a graphist).
	if (mat.getTexture(1)==NULL)
	{
		// Just display the texture
		// NB: setupMaterial() code has correclty setuped textures.
		return;
	}

	int sl0 = currentProgram.pp->getUniformIndex(CProgramIndex::Sampler0);
	if (sl0 != -1)
	{
		setUniform1i(IDriver::PixelProgram, sl0, 0);
	}

	int sl1 = currentProgram.pp->getUniformIndex(CProgramIndex::Sampler1);
	if (sl1 != -1)
	{
		setUniform1i(IDriver::PixelProgram, sl1, 1);
	}

	int tml = currentProgram.vp->getUniformIndex(CProgramIndex::TexMatrix0);
	if (tml != -1)
	{
		setUniform4x4f(IDriver::VertexProgram, tml, _UserTexMat[ 1 ]);
	}

	{
		// Ok we can do it in a single pass

		// Set Stage 1
		// Special: not the same special env if there is or not texture in stage 0.
		CTexEnvSpecial		newEnvStage1;
		if (mat.getTexture(0) == NULL)
			newEnvStage1= TexEnvSpecialSpecularStage1NoText;
		else
			newEnvStage1= TexEnvSpecialSpecularStage1;
		// Test if same env as prec.
		if (_CurrentTexEnvSpecial[1] != newEnvStage1)
		{
			// TexEnv is special.
			_CurrentTexEnvSpecial[1] = newEnvStage1;

			_DriverGLStates.activeTexture(1);

		}
	}
}

// ***************************************************************************
void			CDriverGL3::endSpecularMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endSpecularMultiPass)
	// End specular , only if not Batching mode.
	if (!_SpecularBatchOn)
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
CTextureCube	*CDriverGL3::getSpecularCubeMap(uint exp)
{
	H_AUTO_OGL(CDriverGL3__getSpecularCubeMap)
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
	const uint numCubeMap = sizeof(expToCubeMap) / sizeof(float);
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
sint			CDriverGL3::beginPPLMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginPPLMultiPass)
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

	activateTexEnvColor(0, _PPLightDiffuseColor);

	// Stage 1
	// alpha = diffuse alpha

	// Stage 2 is rgb = SpecularCubeMap * SpecularLightColor + Prec * 1
	// alpha = prec alpha

	activateTexEnvColor(2, _PPLightSpecularColor);

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

	activateTexEnvColor(0, _PPLightDiffuseColor);

	// Stage 1
	static CMaterial::CTexEnv	env;
	env.Env.SrcArg1Alpha = CMaterial::Diffuse;
	activateTexEnvMode(1, env);

}

// ******PER PIXEL LIGHTING, NO SPECULAR**************************************
void			CDriverGL3::endPPLNoSpecMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endPPLNoSpecMultiPass)
	// nothing to do there ...
}

// ***************************************************************************
/* sint		CDriverGL3::beginCausticsMultiPass(const CMaterial &mat)
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
/*inline void		CDriverGL3::setupCausticsFirstTex(const CMaterial &mat)
{
	/// setup texture 0
	activateTexture(0, mat.getTexture(0));

	/// texture environment 0
	setTextureEnvFunction(0, mat);

	/// texture matrix 0
	setupUserTextureMatrix(0, mat);
}

// ***************************************************************************
inline void		CDriverGL3::setupCausticsSecondTex(uint stage)
{
	activateTexture(stage, mat.getTexture(0));
	_CausticCubeMap
}

// ***************************************************************************
void		CDriverGL3::setupCausticsPass(const CMaterial &mat, uint pass)
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
void		CDriverGL3::endCausticsMultiPass(const CMaterial &mat)
{
	nlassert(mat.getShader() == CMaterial::Caustics);

}
*/

// ***************************************************************************
sint		CDriverGL3::beginCloudMultiPass ()
{
	H_AUTO_OGL(CDriverGL3_beginCloudMultiPass)
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);
	return 1;
}

// ***************************************************************************
void		CDriverGL3::setupCloudPass (uint /* pass */)
{
	H_AUTO_OGL(CDriverGL3_setupCloudPass)
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);

	const CMaterial &mat= *_CurrentMaterial;

	activateTexture(0, mat.getTexture(0));
	activateTexture(1, mat.getTexture(0));
}

// ***************************************************************************
void		CDriverGL3::endCloudMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endCloudMultiPass)
	nlassert(_CurrentMaterial->getShader() == CMaterial::Cloud);
}

// ***************************************************************************
sint CDriverGL3::beginWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL3_beginWaterMultiPass)
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
	for (k = 4; k < inlGetNumTextStages(); ++k)
	{
		activateTexture(k, NULL);
	}

}

// ***************************************************************************
void CDriverGL3::endWaterMultiPass()
{
	H_AUTO_OGL(CDriverGL3_endWaterMultiPass);

	nlassert(_CurrentMaterial->getShader() == CMaterial::Water);
	// NB : as fragment shaders / programs bypass the texture envs, no special env enum is added (c.f CTexEnvSpecial)

}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D
