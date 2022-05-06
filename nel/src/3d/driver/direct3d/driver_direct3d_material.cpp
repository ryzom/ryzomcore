// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stddirect3d.h"

#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/texture_bump.h"
#include "nel/misc/rect.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;




namespace NL3D
{

// ***************************************************************************

const D3DBLEND RemapBlendTypeNeL2D3D[CMaterial::blendCount]=
{
	D3DBLEND_ONE,			// one
	D3DBLEND_ZERO,			// zero
	D3DBLEND_SRCALPHA,		// srcalpha
	D3DBLEND_INVSRCALPHA,	// invsrcalpha
	D3DBLEND_SRCCOLOR,		// srccolor
	D3DBLEND_INVSRCCOLOR,	// invsrccolor
	D3DBLEND_ONE,			// blendConstantColor
	D3DBLEND_ONE,			// blendConstantInvColor
	D3DBLEND_ONE,			// blendConstantAlpha
	D3DBLEND_ONE,			// blendConstantInvAlpha
};

// ***************************************************************************

const D3DCMPFUNC RemapZFuncTypeNeL2D3D[CMaterial::zfuncCount]=
{
	D3DCMP_ALWAYS,		// always
	D3DCMP_NEVER,		// never
	D3DCMP_EQUAL,		// equal
	D3DCMP_NOTEQUAL,	// notequal
	D3DCMP_LESS,		// less
	D3DCMP_LESSEQUAL,	// lessequal
	D3DCMP_GREATER,		// greater
	D3DCMP_GREATEREQUAL,// greaterequal
};

// ***************************************************************************

// For stage 0 only
const D3DTEXTUREOP RemapTexOpType0NeL2D3D[CMaterial::TexOperatorCount]=
{
	D3DTOP_SELECTARG1,			// Replace
	D3DTOP_MODULATE,			// Modulate
	D3DTOP_ADD,					// Add
	D3DTOP_ADDSIGNED,			// AddSigned
	D3DTOP_BLENDTEXTUREALPHA,	// InterpolateTexture
	D3DTOP_BLENDDIFFUSEALPHA,	// InterpolatePrevious
	D3DTOP_BLENDDIFFUSEALPHA,	// InterpolateDiffuse
	D3DTOP_LERP,				// InterpolateConstant
	D3DTOP_BUMPENVMAP,			// EMBM
	D3DTOP_MULTIPLYADD			// MAD
};

// ***************************************************************************

const D3DTEXTUREOP RemapTexOpTypeNeL2D3D[CMaterial::TexOperatorCount]=
{
	D3DTOP_SELECTARG1,			// Replace
	D3DTOP_MODULATE,			// Modulate
	D3DTOP_ADD,					// Add
	D3DTOP_ADDSIGNED,			// AddSigned
	D3DTOP_BLENDTEXTUREALPHA,	// InterpolateTexture
	D3DTOP_BLENDCURRENTALPHA,	// InterpolatePrevious
	D3DTOP_BLENDDIFFUSEALPHA,	// InterpolateDiffuse
	D3DTOP_LERP,				// InterpolateConstant
	D3DTOP_BUMPENVMAP,			// EMBM
	D3DTOP_MULTIPLYADD			// MAD
};

const uint OpNumArg[CMaterial::TexOperatorCount] =
{
	1,			// Replace
	2,			// Modulate
	2,					// Add
	2,			// AddSigned
	2,	// InterpolateTexture
	2,	// InterpolatePrevious
	2,	// InterpolateDiffuse
	3,				// InterpolateConstant
	2,			// EMBM
	3			// MAD
};


// ***************************************************************************

// For stage 0 only
const DWORD RemapTexArg0NeL2D3D[CMaterial::TexSourceCount]=
{
	D3DTA_TEXTURE,	// Texture
	D3DTA_DIFFUSE,	// Previous
	D3DTA_DIFFUSE,	// Diffuse
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,	// Constant
};

// ***************************************************************************

const DWORD RemapTexArgNeL2D3D[CMaterial::TexSourceCount]=
{
	D3DTA_TEXTURE,	// Texture
	D3DTA_CURRENT,	// Previous
	D3DTA_DIFFUSE,	// Diffuse
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,	// Constant
};

// ***************************************************************************
const DWORD RemapTexOpArgTypeNeL2D3D[CMaterial::TexOperandCount]=
{
	0,										// SrcColor
	D3DTA_COMPLEMENT,						// InvSrcColor
	D3DTA_ALPHAREPLICATE,					// SrcAlpha
	D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT,	// InvSrcAlpha
};


// ***************************************************************************

const DWORD RemapTexArg0TypeNeL2D3D[CMaterial::TexOperatorCount]=
{
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// Replace not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// Modulate not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// Add not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// AddSigned not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// InterpolateTexture not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// InterpolatePrevious not used
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// InterpolateDiffuse not used
	D3DTA_TFACTOR|D3DTA_ALPHAREPLICATE, // todo hulud constant color D3DTA_CONSTANT|D3DTA_ALPHAREPLICATE,	// InterpolateConstant
	D3DTA_TFACTOR, // todo hulud constant color D3DTA_CONSTANT,							// EMBM not used
	D3DTA_TFACTOR // todo hulud constant color D3DTA_CONSTANT // MAD
};

// ***************************************************************************

const DWORD RemapTexGenTypeNeL2D3D[CMaterial::numTexCoordGenMode]=
{
	D3DTSS_TCI_SPHEREMAP,					// TexCoordGenReflect
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenObjectSpace, not supported
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenEyeSpace
};

// ***************************************************************************

const DWORD RemapTexGenCubeTypeNeL2D3D[CMaterial::numTexCoordGenMode]=
{
	D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR,	// TexCoordGenReflect
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenObjectSpace, not supported
	D3DTSS_TCI_CAMERASPACEPOSITION,			// TexCoordGenEyeSpace
};

// ***************************************************************************

void CMaterialDrvInfosD3D::buildTexEnv (uint stage, const CMaterial::CTexEnv &env, bool textured)
{
	H_AUTO_D3D(CMaterialDrvInfosD3D_buildTexEnv)
	if (textured)
	{
		// The source operator pointer
		const DWORD *srcOp = (stage==0)?RemapTexArg0NeL2D3D:RemapTexArgNeL2D3D;

		ColorOp[stage] = ((stage==0)?RemapTexOpType0NeL2D3D:RemapTexOpTypeNeL2D3D)[env.Env.OpRGB];
		NumColorArg[stage] = OpNumArg[env.Env.OpRGB];
		if (env.Env.OpRGB == CMaterial::Mad)
		{
			ColorArg2[stage] = srcOp[env.Env.SrcArg0RGB];
			ColorArg2[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg0RGB];
			ColorArg1[stage] = srcOp[env.Env.SrcArg1RGB];
			ColorArg1[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg1RGB];
			ColorArg0[stage] = srcOp[env.Env.SrcArg2RGB];
			ColorArg0[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg2RGB];
		}
		else
		{
			// Only used for InterpolateConstant
			ColorArg0[stage] = RemapTexArg0TypeNeL2D3D[env.Env.OpRGB];
			ColorArg1[stage] = srcOp[env.Env.SrcArg0RGB];
			ColorArg1[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg0RGB];
			ColorArg2[stage] = srcOp[env.Env.SrcArg1RGB];
			ColorArg2[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg1RGB];
		}
		AlphaOp[stage] = ((stage==0)?RemapTexOpType0NeL2D3D:RemapTexOpTypeNeL2D3D)[env.Env.OpAlpha];
		NumAlphaArg[stage] = OpNumArg[env.Env.OpAlpha];
		if (env.Env.OpAlpha == CMaterial::Mad)
		{
			AlphaArg2[stage] = srcOp[env.Env.SrcArg0Alpha];
			AlphaArg2[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg0Alpha];
			AlphaArg1[stage] = srcOp[env.Env.SrcArg1Alpha];
			AlphaArg1[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg1Alpha];
			AlphaArg0[stage] = srcOp[env.Env.SrcArg2Alpha];
			AlphaArg0[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg2Alpha];
		}
		else
		{
			// Only used for InterpolateConstant
			AlphaArg0[stage] = RemapTexArg0TypeNeL2D3D[env.Env.OpAlpha];
			AlphaArg1[stage] = srcOp[env.Env.SrcArg0Alpha];
			AlphaArg1[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg0Alpha];
			AlphaArg2[stage] = srcOp[env.Env.SrcArg1Alpha];
			AlphaArg2[stage] |= RemapTexOpArgTypeNeL2D3D[env.Env.OpArg1Alpha];
		}
		ConstantColor[stage] = NL_D3DCOLOR_RGBA(env.ConstantColor);
	}
	else
	{
		// The stage is disabled, active only the lighting
		ColorOp[stage] = D3DTOP_SELECTARG1;
		DWORD opSrc = D3DTA_DIFFUSE;
		ColorArg0[stage] = opSrc;
		ColorArg1[stage] = opSrc;
		ColorArg2[stage] = opSrc;
		AlphaOp[stage] = D3DTOP_SELECTARG1;
		AlphaArg0[stage] = opSrc;
		AlphaArg1[stage] = opSrc;
		AlphaArg2[stage] = opSrc;
		ConstantColor[stage] = NL_D3DCOLOR_RGBA(CRGBA::White);
		NumColorArg[stage] = 1;
		NumAlphaArg[stage] = 1;
	}
}


// ***************************************************************************
static inline DWORD replaceDiffuseWithConstant(DWORD value)
{
	if ((value & D3DTA_SELECTMASK) == D3DTA_DIFFUSE)
	{
		return (value & ~D3DTA_SELECTMASK) | D3DTA_TFACTOR;
	}
	return value;
}




// helpers to set shaders parameters

// Set a color in the shader
static inline void setShaderParam(CMaterialDrvInfosD3D *pShader, uint index, INT value)
{
	nlassert(pShader->FXCache);
	pShader->FXCache->Params.setColor(index, value);
}

// Set a color
static inline void setShaderParam(CMaterialDrvInfosD3D *pShader, uint index, CRGBA color)
{
	nlassert(pShader->FXCache);
	float values[4];
    NL_FLOATS(values, color);
    pShader->FXCache->Params.setVector(index, D3DXVECTOR4(values[0], values[1], values[2], values[3]));
}


// Set a float value in the shader
static inline void setShaderParam(CMaterialDrvInfosD3D *pShader, uint index, FLOAT value)
{
	nlassert(pShader->FXCache);
	pShader->FXCache->Params.setFloat(index, value);
}


// Set a vector of floats
static inline void setShaderParam(CMaterialDrvInfosD3D *pShader, uint index, float vector[4])
{
	nlassert(pShader->FXCache); \
	pShader->FXCache->Params.setVector(index, D3DXVECTOR4(vector[0], vector[1], vector[2], vector[3]));
}

// ***************************************************************************
bool CDriverD3D::setupMaterial(CMaterial &mat)
{
	H_AUTO_D3D(CDriverD3D_setupMaterial)
	CMaterialDrvInfosD3D*	pShader;

	// Stats
	_NbSetupMaterialCall++;

	// Max texture
	const uint maxTexture = inlGetNumTextStages();

	// Update material
	uint32 touched=mat.getTouched();

	// No shader ?
	if (!mat._MatDrvInfo)
	{
		// Insert into driver list. (so it is deleted when driver is deleted).
		ItMatDrvInfoPtrList		it= _MatDrvInfos.insert(_MatDrvInfos.end(), (NL3D::IMaterialDrvInfos*)NULL);

		*it = mat._MatDrvInfo = new CMaterialDrvInfosD3D(this, it);

		// Must create all OpenGL shader states.
		touched = IDRV_TOUCHED_ALL;
	}
	pShader = static_cast<CMaterialDrvInfosD3D*>((IMaterialDrvInfos*)(mat._MatDrvInfo));

	// Now we can get the supported shader from the cache.
	CMaterial::TShader matShader = _PixelProgramUser ? CMaterial::Program : mat.getShader();

	if (_CurrentMaterialSupportedShader != CMaterial::Normal)
	{
		// because of multipass, some shader need to disable fog
		// restore fog here
		if(_FogEnabled && _FogColor != _RenderStateCache[D3DRS_FOGCOLOR].Value)
		{
			// restore fog
			setRenderState(D3DRS_FOGCOLOR, _FogColor);
		}
	}
	{
		H_AUTO_D3D(CDriverD3D_setupMaterial_light)
		// if the shader has changed since last time
		if(matShader != _CurrentMaterialSupportedShader)
		{
			// if current shader is normal shader, then must restore uv routing, because it may have been changed by a previous shader (such as lightmap)
			if (matShader == CMaterial::Normal)
			{
				for(uint k = 0; k < MaxTexture; ++k)
				{
					setTextureIndexUV (k, _CurrentUVRouting[k]);
				}
			}
			// if old was lightmap, restore standard lighting
			if(_CurrentMaterialSupportedShader==CMaterial::LightMap)
			{
				_MustRestoreLight = true;
			}

			// if new is lightmap, setup dynamic lighting
			if(matShader==CMaterial::LightMap)
			{
				setupLightMapDynamicLighting(true);
				_MustRestoreLight = false;
			}
		}
		if (mat.isLighted() && _MustRestoreLight)
		{
			setupLightMapDynamicLighting(false);
			_MustRestoreLight = false;
		}
		// setup the global
		_CurrentMaterialSupportedShader= matShader;
	}


	{
		// count number of tex stages
		uint numUsedTexStages = 0;
		for(numUsedTexStages = 0; numUsedTexStages < IDRV_MAT_MAXTEXTURES; ++numUsedTexStages)
		{
			if (mat.getTexture(uint8(numUsedTexStages)) == NULL) break;
		}

		H_AUTO_D3D(CDriverD3D_setupMaterial_touchupdate)
		if (pShader->NumUsedTexStages != numUsedTexStages)
		{
			touched |= IDRV_TOUCHED_TEXENV;
		}
		// Something to setup ?
		if (touched)
		{
			if (touched & IDRV_TOUCHED_SHADER)
			{
				delete pShader->FXCache;
				pShader->FXCache = NULL;
				// See if material actually needs a fx cache
				if (mat.getShader() != CMaterial::Specular && mat.getShader() != CMaterial::Normal)
				{
					pShader->FXCache = new CFXCache;
				}
			}
			/* Exception: if only Textures are modified in the material, no need to "Bind OpenGL States", or even to test
				for change, because textures are activated alone, see below.
				No problem with delete/new problem (see below), because in this case, IDRV_TOUCHED_ALL is set (see above).
			*/
			// If any flag is set (but a flag of texture).
			if(touched & (~_MaterialAllTextureTouchedFlag))
			{
				// Convert Material to driver shader.
				if (touched & IDRV_TOUCHED_BLENDFUNC)
				{
					pShader->SrcBlend = RemapBlendTypeNeL2D3D[mat.getSrcBlend()];
					pShader->DstBlend = RemapBlendTypeNeL2D3D[mat.getDstBlend()];
				}
				if (touched & IDRV_TOUCHED_ZFUNC)
				{
					pShader->ZComp = RemapZFuncTypeNeL2D3D[mat.getZFunc()];
				}
				if (touched & IDRV_TOUCHED_LIGHTING)
				{
					// Lighted material ?
					if (mat.isLighted())
					{
						// Setup the color
						NL_D3DCOLORVALUE_RGBA(pShader->Material.Diffuse, mat.getDiffuse());
						NL_D3DCOLORVALUE_RGBA(pShader->Material.Ambient, mat.getAmbient());
						NL_D3DCOLORVALUE_RGBA(pShader->Material.Emissive, mat.getEmissive());

						// Specular
						CRGBA spec = mat.getSpecular();
						if (spec != CRGBA::Black)
						{
							NL_D3DCOLORVALUE_RGBA(pShader->Material.Specular, spec);
							pShader->SpecularEnabled = TRUE;
							pShader->Material.Power = mat.getShininess();
						}
						else
							setRenderState (D3DRS_SPECULARENABLE, FALSE);
					}
					else
					{
						// No specular
						pShader->SpecularEnabled = FALSE;
					}
				}
				if (touched & IDRV_TOUCHED_COLOR)
				{
					// Setup the color
					pShader->UnlightedColor = NL_D3DCOLOR_RGBA(mat.getColor());
				}
				if (touched & IDRV_TOUCHED_ALPHA_TEST_THRE)
				{
					float alphaRef = (float)floor(mat.getAlphaTestThreshold() * 255.f + 0.5f);
					clamp (alphaRef, 0.f, 255.f);
					pShader->AlphaRef = (DWORD)alphaRef;
				}
				if (touched & IDRV_TOUCHED_SHADER)
				{
					// todo hulud d3d material shaders
					// Get shader. Fallback to other shader if not supported.
					// pShader->SupportedShader= getSupportedShader(mat.getShader());
				}
				if (touched & IDRV_TOUCHED_TEXENV)
				{
					// Build the tex env cache.
					// Do not do it for Lightmap and per pixel lighting , because done in multipass in a very special fashion.
					// This avoid the useless multiple change of texture states per lightmapped object.
					// Don't do it also for Specular because the EnvFunction and the TexGen may be special.
					if(matShader == CMaterial::Normal)
					{
						uint stage;
						for(stage=0 ; stage<maxTexture; ++stage)
						{
							// Build the tex env
							pShader->buildTexEnv (stage, mat._TexEnvs[stage], mat.getTexture(uint8(stage)) != NULL);
						}
					}
				}
				if (touched & (IDRV_TOUCHED_TEXGEN|IDRV_TOUCHED_ALLTEX))
				{
					uint stage;
					for(stage=0 ; stage<maxTexture; ++stage)
					{
						pShader->ActivateSpecularWorldTexMT[stage] = false;
						pShader->ActivateInvViewModelTexMT[stage] = false;

						// Build the tex env
						ITexture	*text= mat.getTexture(uint8(stage));
						if (text && text->isTextureCube())
						{
							if (mat.getTexCoordGen(stage))
							{
								// Texture cube
								pShader->TexGen[stage] = RemapTexGenCubeTypeNeL2D3D[mat.getTexCoordGenMode (stage)];

								// Texture matrix
								pShader->ActivateSpecularWorldTexMT[stage] = mat.getTexCoordGenMode (stage) == CMaterial::TexCoordGenReflect;
							}
							else
							{
								pShader->TexGen[stage] = mat.getTexCoordGen(stage);
							}
						}
						else
						{
							if (mat.getTexCoordGen(stage))
							{
								pShader->TexGen[stage] = RemapTexGenTypeNeL2D3D[mat.getTexCoordGenMode (stage)];

								// Texture matrix
								pShader->ActivateInvViewModelTexMT[stage] = mat.getTexCoordGenMode (stage) == CMaterial::TexCoordGenObjectSpace;
							}
							else
							{
								pShader->TexGen[stage] = mat.getTexCoordGen(stage);
							}
						}
					}
				}

				// Does this material needs a pixel shader ?
				if (touched & (IDRV_TOUCHED_TEXENV|IDRV_TOUCHED_BLEND|IDRV_TOUCHED_ALPHA_TEST))
				{
					std::fill(pShader->RGBPipe, pShader->RGBPipe + IDRV_MAT_MAXTEXTURES, true);
					std::fill(pShader->AlphaPipe, pShader->AlphaPipe + IDRV_MAT_MAXTEXTURES, true);

					pShader->MultipleConstantNoPixelShader = false;
					pShader->MultiplePerStageConstant = false;
					pShader->VertexColorLighted = mat.getLightedVertexColor();
					bool _needPixelShader = false;
					pShader->ConstantIndex = 0xff;
					pShader->ConstantIndex2 = 0xff;
					if (matShader == CMaterial::Normal)
					{
						// Need of constants ?
						uint numConstants;
						uint firstConstant;
						uint secondConstant;
						needsConstants (numConstants, firstConstant, secondConstant, mat);

						pShader->ConstantIndex = uint8(firstConstant);
						pShader->ConstantIndex2 = uint8(secondConstant);

						// Need a constant color for the diffuse component ?
						pShader->NeedsConstantForDiffuse = needsConstantForDiffuse (mat);

						// Need pixel shader ?
	#ifndef NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS
						_needPixelShader = (numConstants > 1) || ((numConstants==1) && pShader->NeedsConstantForDiffuse);
	#else // NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS
						_needPixelShader = true;
	#endif // NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS
						if (_needPixelShader)
						{
							// The shader description
							CNormalShaderDesc normalShaderDesc;

							// Setup descriptor
							uint stage;
							for(stage=0 ; stage<(uint)maxTexture; stage++)
							{
								// Stage used ?
								normalShaderDesc.StageUsed[stage] = mat.getTexture (uint8(stage)) != NULL;
								normalShaderDesc.TexEnvMode[stage] = mat.getTexEnvMode(uint8(stage));
							}

							if (_PixelProgram)
							{
								#ifdef NL_DEBUG_D3D
									// Check, should not occur
									nlassertex (_PixelShader, ("STOP : no pixel shader available. Can't render this material."));
								#endif // NL_DEBUG_D3D

								// Build the pixel shader
								pShader->PixelShader = buildPixelShader (normalShaderDesc, false);
								if (!mat.isLighted())
									pShader->PixelShaderUnlightedNoVertexColor = buildPixelShader (normalShaderDesc, true);
								else
									pShader->PixelShaderUnlightedNoVertexColor = NULL;
							}
							else
							{
								// Possible configurations are :
								// unlighted, 1 constant diffuse + 1 per stage constant
								// unlighted, 2 constants (second constant emulated with material emissive + diffuse), possibly with vertex color (aliased to specular)

								// If there are no pixel shader then we're likely to have three stage or less
								pShader->MultipleConstantNoPixelShader = true;
								pShader->MultiplePerStageConstant = numConstants > 1;
								#ifdef NL_DEBUG
									nlassert(numConstants <= 2);
								#endif
								if (secondConstant != 0xffffffff)
								{
									pShader->Constant2 = mat.getTexConstantColor(secondConstant);
								}
								pShader->PixelShader = NULL;
								pShader->PixelShaderUnlightedNoVertexColor = NULL;
								// compute relevant parts of the pipeline
								computeRelevantTexEnv(mat, pShader->RGBPipe, pShader->AlphaPipe);
							}
						}
						else
						{
							// compute relevant parts of the pipeline
							computeRelevantTexEnv(mat, pShader->RGBPipe, pShader->AlphaPipe);
						}
					}
					else
					{
						// Other shaders
						pShader->MultipleConstantNoPixelShader = false;
						pShader->NeedsConstantForDiffuse = false;
					}

					if (!_needPixelShader)
					{
						// Remove the old one
						pShader->PixelShader = NULL;
						pShader->PixelShaderUnlightedNoVertexColor = NULL;
						pShader->MultipleConstantNoPixelShader = 0;
					}
				}

				// Since modified, must rebind all D3D states. And do this also for the delete/new problem.
				/* If an old material is deleted, _CurrentMaterial is invalid. But this is grave only if a new
					material is created, with the same pointer (bad luck). Since an newly allocated material always
					pass here before use, we are sure to avoid any problems.
				*/
				_CurrentMaterial= NULL;
			}

			// Optimize: reset all flags at the end.
			mat.clearTouched(0xFFFFFFFF);
			pShader->NumUsedTexStages = numUsedTexStages;
		}
	}


	{
		H_AUTO_D3D(CDriverD3D_setupMaterial_bindTexture)
		// 2. Setup / Bind Textures.
		//==========================
		// Must setup textures each frame. (need to test if touched).
		// Must separate texture setup and texture activation in 2 "for"...
		// because setupTexture() may disable all stage.

		if (matShader == CMaterial::Normal
			|| ((matShader == CMaterial::Program) && (_PixelProgramUser->features().MaterialFlags & CProgramFeatures::TextureStages))
			)
		{
			uint stage;
			for(stage=0 ; stage<maxTexture; ++stage)
			{
				ITexture	*text= mat.getTexture(uint8(stage));
				if (!text) break;
				if (text != NULL && !setupTexture(*text))
					return(false);
			}
		}
	}


	// Activate the textures.
	// Do not do it for Lightmap and per pixel lighting , because done in multipass in a very special fashion.
	// This avoid the useless multiple change of texture states per lightmapped object.
	// Don't do it also for Specular because the EnvFunction and the TexGen may be special.
	{
		H_AUTO_D3D(CDriverD3D_setupMaterial_normalShaderActivateTextures)
		if (matShader == CMaterial::Normal
			|| ((matShader == CMaterial::Program) && (_PixelProgramUser->features().MaterialFlags & CProgramFeatures::TextureStages))
			)
		{
			uint stage;
			for(stage=0 ; stage<maxTexture; ++stage)
			{
				ITexture	*text= mat.getTexture(uint8(stage));
				if (text)
				{
					// activate the texture, or disable texturing if NULL.
					setTexture(stage, text);
				}
				else
				{
					setTexture (stage, (LPDIRECT3DBASETEXTURE9)NULL);
					if (stage != 0)
					{
						setTextureState (stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
						//setTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
						break; // stage after this one are ignored
					}
					else
					{
						setTextureState (1, D3DTSS_COLOROP, D3DTOP_DISABLE);
					}
				}

				// Set the texture states
				if (text || (stage == 0))
				{
					if (matShader == CMaterial::Program)
					{
						// Do nothing for user pixel shader
					}
					else if (!pShader->PixelShader)
					{
						// Doesn't use a pixel shader ? Set the textures stages
						if (pShader->RGBPipe[stage])
						{
							setTextureState (stage, D3DTSS_COLOROP, pShader->ColorOp[stage]);
							setTextureState (stage, D3DTSS_COLORARG1, pShader->ColorArg1[stage]);
							if (pShader->NumColorArg[stage] > 1)
							{
								setTextureState (stage, D3DTSS_COLORARG2, pShader->ColorArg2[stage]);
								if (pShader->NumColorArg[stage] > 2)
								{
									setTextureState (stage, D3DTSS_COLORARG0, pShader->ColorArg0[stage]);
								}
							}
						}
 						if (pShader->AlphaPipe[stage])
						{
							setTextureState (stage, D3DTSS_ALPHAOP, pShader->AlphaOp[stage]);
							setTextureState (stage, D3DTSS_ALPHAARG1, pShader->AlphaArg1[stage]);
							if (pShader->NumAlphaArg[stage] > 1)
							{
								setTextureState (stage, D3DTSS_ALPHAARG2, pShader->AlphaArg2[stage]);
								if (pShader->NumAlphaArg[stage] > 2)
								{
									setTextureState (stage, D3DTSS_ALPHAARG0, pShader->AlphaArg0[stage]);
								}
							}
						}
						// If there is one constant and the tfactor is not needed for diffuse, use the tfactor as constant
						if (pShader->ConstantIndex == stage)
								setRenderState (D3DRS_TEXTUREFACTOR, pShader->ConstantColor[stage]);
					}
					else
					{
						float colors[4];
						D3DCOLOR_FLOATS (colors, pShader->ConstantColor[stage]);
						setPixelShaderConstant (stage, colors);
					}
				}

				// Set texture generator state
				setTextureIndexMode (stage, pShader->TexGen[stage]!=D3DTSS_TCI_PASSTHRU, pShader->TexGen[stage]);

				// Need user matrix ?
				bool needUserMtx = mat.isUserTexMatEnabled(stage);
				D3DXMATRIX userMtx;
				if (needUserMtx)
				{
					// If tex gen mode is used, or a cube texture is used, then use the matrix 'as it'.
					// Must build a 3x3 matrix for 2D texture coordinates in D3D (which is kind of weird ...)
					if (pShader->TexGen[stage] != D3DTSS_TCI_PASSTHRU || (mat.getTexture(uint8(stage)) && mat.getTexture(uint8(stage))->isTextureCube()))
					{
						NL_D3D_MATRIX(userMtx, mat.getUserTexMat(stage));
					}
					else
					{
						const CMatrix &m = mat.getUserTexMat(stage);
						NL_D3D_TEX2D_MATRIX(userMtx, m);
					}
				}

				// Need cubic texture coord generator ?
				if (pShader->ActivateSpecularWorldTexMT[stage])
				{
					// Set the driver matrix
					setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
					if (!needUserMtx)
						setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), _D3DSpecularWorldTex);
					else
					{
						D3DXMatrixMultiply (&userMtx, &_D3DSpecularWorldTex, &userMtx);
						setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), userMtx);
					}
				}
				else if (pShader->ActivateInvViewModelTexMT[stage])
				{
					// Set the driver matrix
					setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
					if (!needUserMtx)
						setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), _D3DInvModelView);
					else
					{
						D3DXMatrixMultiply (&userMtx, &_D3DInvModelView, &userMtx);
						setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), userMtx);
					}
				}
				else
				{
					if (_NbNeLTextureStages == 3)
					{
						// Fix for Radeon 7xxx
						// In some situation, texture transform stays enabled after a material change, so enable it all the
						// time and use identity matrix instead of the D3DTTFF_DISABLE flag
						setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
						setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), needUserMtx ? userMtx : _D3DMatrixIdentity);
					}
					else
					{
						// Set the driver matrix
						if (needUserMtx)
						{
							setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
							setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+stage), userMtx);
						}
						else
						{
							setTextureState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
						}
					}
				}
				if (!text && stage == 0) break;
			}
		}
	}

	if (_CurrentMaterial != &mat)
	{
		// Material has changed ?
		// Restore fog state to its current value
		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_updateFog)
			setRenderState (D3DRS_FOGENABLE, _FogEnabled?TRUE:FALSE);
		}

		// Flags
		const uint32 flags = mat.getFlags();

		// Two sided
		_DoubleSided = (flags&IDRV_MAT_DOUBLE_SIDED)!=0;

		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_handleBackSide)
			// Handle backside
			if (_CullMode == CCW)
			{
				setRenderState (D3DRS_CULLMODE, _DoubleSided?D3DCULL_NONE:_InvertCullMode?D3DCULL_CCW:D3DCULL_CW);
			}
			else
			{
				setRenderState (D3DRS_CULLMODE, _DoubleSided?D3DCULL_NONE:_InvertCullMode?D3DCULL_CW:D3DCULL_CCW);
			}
		}


		bool blend = (flags&IDRV_MAT_BLEND) != 0;
		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_updateBlend)
			// Active states
			if (blend)
			{
				setRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
				setRenderState (D3DRS_SRCBLEND, pShader->SrcBlend);
				setRenderState (D3DRS_DESTBLEND, pShader->DstBlend);
			}
			else
				setRenderState (D3DRS_ALPHABLENDENABLE, FALSE);

			// Alpha test
			if (flags&IDRV_MAT_ALPHA_TEST)
			{
				setRenderState (D3DRS_ALPHATESTENABLE, TRUE);
				setRenderState (D3DRS_ALPHAREF, pShader->AlphaRef);
				setRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			}
			else
				setRenderState (D3DRS_ALPHATESTENABLE, FALSE);
		}

		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_updateZBuffer)
			// Z buffer
			setRenderState (D3DRS_ZWRITEENABLE, (flags&IDRV_MAT_ZWRITE)?TRUE:FALSE);
			setRenderState (D3DRS_ZFUNC, pShader->ZComp);
			float flt = mat.getZBias () * _OODeltaZ;
			setRenderState (D3DRS_DEPTHBIAS, FTODW(flt));
		}

		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_updateLighting)
			// Active lighting
			if (mat.isLighted())
			{
				setRenderState (D3DRS_LIGHTING, TRUE);
				setMaterialState(pShader->Material);
			}
			else
			{
				setRenderState (D3DRS_LIGHTING, FALSE);

				// Set pixel shader unlighted color ?
				if (pShader->PixelShader)
				{
					float colors[4];
					D3DCOLOR_FLOATS(colors,pShader->UnlightedColor);
					setPixelShaderConstant (5, colors);
				}
			}
			setRenderState (D3DRS_SPECULARENABLE, pShader->SpecularEnabled);
		}

		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_updateVertexColorLighted)
			// Active vertex color if not lighted or vertex color forced
			if (mat.isLightedVertexColor ())
			{
				setRenderState (D3DRS_COLORVERTEX, TRUE);

				setRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
			}
			else
			{
				setRenderState (D3DRS_COLORVERTEX, FALSE);
				setRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			}
		}

		{
			H_AUTO_D3D(CDriverD3D_setupMaterial_disableFog)
			// Disable fog if dest blend is ONE
			if (blend && (pShader->DstBlend == D3DBLEND_ONE))
			{
				setRenderState (D3DRS_FOGENABLE, FALSE);
			}
		}

		// Set the good shader
		switch (matShader)
		{
		case CMaterial::Normal:
			{
				H_AUTO_D3D(CDriverD3D_setupMaterial_setupNormalshader)
				// No shader
				activeShader (NULL);

				/* If unlighted trick is needed, set the shader later */
				if (!pShader->NeedsConstantForDiffuse && _PixelProgram)
					setPixelShader (pShader->PixelShader);
			}
			break;
		case CMaterial::LightMap:
			{
				H_AUTO_D3D(CDriverD3D_setupMaterial_setupLightmapShader)
				setPixelShader (NULL);
				static const uint32 RGBMaskPacked = CRGBA(255,255,255,0).getPacked();

				if (_NbNeLTextureStages == 3)
				{
					touchRenderVariable(&_MaterialState);
				}

				// if the dynamic lightmap light has changed since the last render (should not happen), resetup
				// normal way is that setupLightMapDynamicLighting() is called at begin of setupMaterial() if shader different from prec
				if(_LightMapDynamicLightDirty)
					setupLightMapDynamicLighting(true);

				// Count the lightmaps
				uint lightmap;
				uint lightmapCount = 0;
				uint lightmapMask = 0;
				for(lightmap=0 ; lightmap<(sint)mat._LightMaps.size() ; lightmap++)
				{
					if (mat._LightMaps[lightmap].Factor.getPacked() & RGBMaskPacked)
					{
						lightmapCount++;
						lightmapMask |= 1<<lightmap;
					}
				}


				// activate the appropriate shader
				if (mat.getBlend())
				{
					if(mat._LightMapsMulx2)
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0BlendX2); break;
						case 1:	activeShader (&_ShaderLightmap1BlendX2); break;
						case 2:	activeShader (&_ShaderLightmap2BlendX2); break;
						case 3:	activeShader (&_ShaderLightmap3BlendX2); break;
						default:	activeShader (&_ShaderLightmap4BlendX2); break;
						}
					}
					else
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0Blend); break;
						case 1:	activeShader (&_ShaderLightmap1Blend); break;
						case 2:	activeShader (&_ShaderLightmap2Blend); break;
						case 3:	activeShader (&_ShaderLightmap3Blend); break;
						default:	activeShader (&_ShaderLightmap4Blend); break;
						}
					}
				}
				else
				{
					if(mat._LightMapsMulx2)
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0X2); break;
						case 1:	activeShader (&_ShaderLightmap1X2); break;
						case 2:	activeShader (&_ShaderLightmap2X2); break;
						case 3:	activeShader (&_ShaderLightmap3X2); break;
						default:	activeShader (&_ShaderLightmap4X2); break;
						}
					}
					else
					{
						switch (lightmapCount)
						{
						case 0:	activeShader (&_ShaderLightmap0); break;
						case 1:	activeShader (&_ShaderLightmap1);	break;
						case 2:	activeShader (&_ShaderLightmap2); break;
						case 3:	activeShader (&_ShaderLightmap3); break;
						default:	activeShader (&_ShaderLightmap4); break;
						}
					}
				}

				// Setup the texture
				ITexture *texture = mat.getTexture(0);
				if (!setShaderTexture (0, texture, pShader->FXCache))
					return false;

				// Get the effect
				nlassert (_CurrentShader);
				CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
//				ID3DXEffect			*effect = shaderInfo->Effect;


				// Set the ambiant for 8Bit Light Compression
				{
					// Sum ambiant of all active lightmaps.
					uint32	r=0;
					uint32	g=0;
					uint32	b=0;
					for(lightmap=0 ; lightmap<(sint)mat._LightMaps.size() ; lightmap++)
					{
						if (lightmapMask & (1<<lightmap))
						{
							uint	wla= lightmap;
							// must mul them by their respective mapFactor too
							CRGBA ambFactor = mat._LightMaps[wla].Factor;
							CRGBA lmcAmb= mat._LightMaps[wla].LMCAmbient;
							r+= ((uint32)ambFactor.R  * ((uint32)lmcAmb.R+(lmcAmb.R>>7))) >>8;
							g+= ((uint32)ambFactor.G  * ((uint32)lmcAmb.G+(lmcAmb.G>>7))) >>8;
							b+= ((uint32)ambFactor.B  * ((uint32)lmcAmb.B+(lmcAmb.B>>7))) >>8;
						}
					}
					r= std::min(r, (uint32)255);
					g= std::min(g, (uint32)255);
					b= std::min(b, (uint32)255);
					CRGBA	lmcAmbient((uint8)r,(uint8)g,(uint8)b,255);

					// Set into FX shader
					setShaderParam(pShader, 0, (INT) NL_D3DCOLOR_RGBA(lmcAmbient));
					if (shaderInfo->FactorHandle[0])
					{
						setShaderParam(pShader, 0, lmcAmbient);
					}
				}


				// Set the lightmaps
				lightmapCount = 0;
				for(lightmap=0 ; lightmap<(sint)mat._LightMaps.size() ; lightmap++)
				{
					if (lightmapMask & (1<<lightmap))
					{
						// Set the lightmap texture
						ITexture	*text= mat._LightMaps[lightmap].Texture;
						if (text != NULL && !setShaderTexture (lightmapCount+1, text, pShader->FXCache))
							return false;

						// Get the lightmap color factor, and mul by lmcDiffuse compression
						CRGBA lmapFactor = mat._LightMaps[lightmap].Factor;
						CRGBA lmcDiff= mat._LightMaps[lightmap].LMCDiffuse;
						lmapFactor.R = (uint8)(((uint32)lmapFactor.R  * ((uint32)lmcDiff.R+(lmcDiff.R>>7))) >>8);
						lmapFactor.G = (uint8)(((uint32)lmapFactor.G  * ((uint32)lmcDiff.G+(lmcDiff.G>>7))) >>8);
						lmapFactor.B = (uint8)(((uint32)lmapFactor.B  * ((uint32)lmcDiff.B+(lmcDiff.B>>7))) >>8);
						lmapFactor.A = 255;

						// Set the lightmap color factor into shader
						setShaderParam(pShader, lightmapCount+1, (INT) NL_D3DCOLOR_RGBA(lmapFactor));
						if (shaderInfo->FactorHandle[lightmapCount+1])
						{
							setShaderParam(pShader, lightmapCount+1, lmapFactor);
						}
						lightmapCount++;
					}
				}
			}
			break;
		case CMaterial::Specular:
			{
				H_AUTO_D3D(CDriverD3D_setupMaterial_setupSpecularShader)
				// No shader
				activeShader (NULL);
				setPixelShader (NULL);

				// Setup the texture
				ITexture *texture = mat.getTexture(0);
				if (!texture || !setupTexture (*texture))
					return false;
				setTexture (0, texture);

				// Setup the texture
				texture = mat.getTexture(1);
				if (!texture || !setupTexture (*texture))
					return false;
				setTexture (1, texture);

				// Set the driver matrix
				if (texture->isTextureCube())
				{
					setTextureIndexMode (1, true, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
					setTextureState (1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
					setMatrix ((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+1), _D3DSpecularWorldTex);
				}

				setRenderState (D3DRS_LIGHTING, mat.isLighted()?TRUE:FALSE);
				setRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
				setTextureState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				setTextureState (0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				setTextureState (0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
				setTextureState (0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				setTextureState (0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				setTextureState (1, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
				setTextureState (1, D3DTSS_COLORARG0, D3DTA_CURRENT);
				setTextureState (1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				setTextureState (1, D3DTSS_COLORARG2, D3DTA_CURRENT|D3DTA_ALPHAREPLICATE);
				setTextureState (1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				setTextureState (1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

			}
			break;
		case CMaterial::Cloud:
			{
				H_AUTO_D3D(CDriverD3D_setupMaterial_setupCloudShader)
				activeShader (&_ShaderCloud);

				// Get the shader
				nlassert (_CurrentShader);
//				CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);

				// Set the constant
//				ID3DXEffect			*effect = shaderInfo->Effect;
				CRGBA color = mat.getColor();
				color.R = 255;
				color.G = 255;
				color.B = 255;
				setShaderParam(pShader, 0, color);

				// Set the texture
				if (!setShaderTexture (0, mat.getTexture(0), pShader->FXCache) || !setShaderTexture (1, mat.getTexture(1), pShader->FXCache))
					return false;
			}
			break;
		case CMaterial::Water:
			{
				H_AUTO_D3D(CDriverD3D_setupMaterial_setupWaterShader)
				activeShader(mat.getTexture(3) ? &_ShaderWaterDiffuse : &_ShaderWaterNoDiffuse);
				// Get the shader
				nlassert (_CurrentShader);
//				CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
				// Set the textures
				if (_PixelShaderVersion >= D3DPS_VERSION(1, 4))
				{
					ITexture *tex = mat.getTexture(0);
					if (tex)
					{
						tex->setUploadFormat(ITexture::RGBA8888);
//						if (tex->isBumpMap())
//						{
//							CTextureBump *tb = static_cast<CTextureBump *>(tex);
//						}
						setupTexture(*tex);
						setShaderTexture (0, tex, pShader->FXCache);
					}
				}
				ITexture *tex = mat.getTexture(1);
				if (tex)
				{
					if (_PixelShaderVersion < D3DPS_VERSION(1, 4))
					{
						tex->setUploadFormat(ITexture::DsDt);
//						if (tex->isBumpMap())
//						{
//							CTextureBump *tb = static_cast<CTextureBump *>(tex);
//						}
					}
					else
					{
						tex->setUploadFormat(ITexture::RGBA8888);
//						if (tex->isBumpMap())
//						{
//							CTextureBump *tb = static_cast<CTextureBump *>(tex);
//						}
					}
					setupTexture(*tex);
					setShaderTexture (1, tex, pShader->FXCache);
				}
				//
				tex = mat.getTexture(2);
				if (tex)
				{
					setupTexture(*tex);
					setShaderTexture (2, tex, pShader->FXCache);
				}
				tex = mat.getTexture(3);
				if (tex)
				{
					setupTexture(*tex);
					setShaderTexture (3, tex, pShader->FXCache);
				}

				// Set the constants
//				ID3DXEffect			*effect = shaderInfo->Effect;
				if (_PixelShaderVersion < D3DPS_VERSION(2, 0))
				{
					if (_PixelShaderVersion < D3DPS_VERSION(1, 4))
					{
						if (mat.getTexture(1) && mat.getTexture(1)->isBumpMap())
						{
							float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(1))->getNormalizationFactor();
							setShaderParam(pShader, 0, factor);
						}
						else
						{
							setShaderParam(pShader, 0, 1.f);
						}
					}
					else
					{
						if (mat.getTexture(0) && mat.getTexture(0)->isBumpMap())
						{
							float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(0))->getNormalizationFactor();
							float cst[4] = { factor, factor, factor, 0.f };
							setShaderParam(pShader, 0, cst);
						}
						else
						{
							float cst[4] = { 1.f, 1.f, 1.f, 0.f };
							setShaderParam(pShader, 0, cst);
						}
						//
						if (mat.getTexture(1) && mat.getTexture(1)->isBumpMap())
						{
							float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(1))->getNormalizationFactor();
							float cst[4] = { factor, factor, factor, 0.f };
							setShaderParam(pShader, 1, cst);
						}
						else
						{
							float cst[4] = { 1.f, 1.f, 1.f, 0.f };
							setShaderParam(pShader, 1, cst);
						}
					}
				}
				else
				{
					// setup the constant
					if (mat.getTexture(0) && mat.getTexture(0)->isBumpMap())
					{
						float factor = 0.25f * NLMISC::safe_cast<CTextureBump *>(mat.getTexture(0))->getNormalizationFactor();
						float bmScale[4] = { -1.f * factor, -1.f * factor, 2.f * factor, 0.f };
						setShaderParam(pShader, 0, bmScale);
					}
					else
					{
						float bmScale[4] = { -1.f, -1.f, 2.f, 0.f };
						setShaderParam(pShader, 0, bmScale);
					}
					// setup the constant
					if (mat.getTexture(1) && mat.getTexture(1)->isBumpMap())
					{
						float factor = NLMISC::safe_cast<CTextureBump *>(mat.getTexture(1))->getNormalizationFactor();
						float bmScale[4] = { -1.f * factor, -1.f * factor, 2.f * factor, 0.f };
						setShaderParam(pShader, 1, bmScale);
					}
					else
					{
						float bmScale[4] = { -1.f, -1.f, 2.f, 0.f };
						setShaderParam(pShader, 1, bmScale);
					}
				}
			}
			break; // CMaterial::Water
		case CMaterial::Program:
			{
				H_AUTO_D3D(CDriverD3D_setupMaterial_setupProgramshader)
					// No material shader
					activeShader(NULL);
			}
			break;
		}

		// New material setuped
		_CurrentMaterial = &mat;

		// Update variables
		_CurrentMaterialInfo = pShader;
	}

	return true;
}

// ***************************************************************************
// Add one set to another
static void add(std::set<uint> &dest, const std::set<uint> &toAdd)
{
	for (std::set<uint>::const_iterator it = toAdd.begin(); it != toAdd.end(); ++it)
	{
		dest.insert(*it);
	}
}


// ***************************************************************************
bool CDriverD3D::needsConstants (uint &numConstant, uint &firstConstant, uint &secondConstant, CMaterial &mat)
{
	H_AUTO_D3D(CDriverD3D_needsConstants)
	firstConstant = 0xffffffff;
	secondConstant = 0xffffffff;
	// Of course std::set could be avoided, but much simpler that way
	std::set<uint> rgbPipe[2];  // constant used to compute the rgb component
	std::set<uint> alphaPipe[2];  // constant used to compute the alpha component
	for (uint i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		if (!mat.getTexture(uint8(i))) break;
		CMaterial::CTexEnv texEnv;
		texEnv.EnvPacked = mat.getTexEnvMode(i);
		rgbPipe[1].clear();
		alphaPipe[1].clear();
		if (texEnv.Env.OpRGB == CMaterial::InterpolateConstant)
		{
			rgbPipe[1].insert(i);
		}
		if (texEnv.Env.OpRGB == CMaterial::InterpolatePrevious)
		{
			rgbPipe[1] = rgbPipe[0];
		}
		if (texEnv.Env.OpAlpha == CMaterial::InterpolateConstant)
		{
			alphaPipe[1].insert(i);
		}
		if (texEnv.Env.OpAlpha == CMaterial::InterpolatePrevious)
		{
			alphaPipe[1] = alphaPipe[0];
		}
		for(uint l = 0; l < OpNumArg[texEnv.Env.OpRGB]; ++l)
		{
			// color pipe
			if (texEnv.getColorArg(l) == CMaterial::Constant)
			{
				rgbPipe[1].insert(i);
			}
			else
			if (texEnv.getColorArg(l) == CMaterial::Previous)
			{
				if (texEnv.getColorOperand(l) == CMaterial::SrcColor || texEnv.getColorOperand(l) == CMaterial::InvSrcColor)
				{
					add(rgbPipe[1], rgbPipe[0]);
				}
				if (texEnv.getColorOperand(l) == CMaterial::SrcAlpha || texEnv.getColorOperand(l) == CMaterial::InvSrcColor)
				{
					add(rgbPipe[1], alphaPipe[0]);
				}
			}
		}
		for(uint l = 0; l < OpNumArg[texEnv.Env.OpAlpha]; ++l)
		{
			// alpha pipe
			if (texEnv.getAlphaArg(l) == CMaterial::Constant)
			{
				alphaPipe[1].insert(i);
			}
			if (texEnv.getAlphaArg(l) == CMaterial::Previous)
			{
				add(alphaPipe[1], alphaPipe[0]);
			}
		}
		alphaPipe[0].swap(alphaPipe[1]);
		rgbPipe[0].swap(rgbPipe[1]);
	}
	bool alphaRelevant = false;
	if (mat.getAlphaTest())
	{
		alphaRelevant = true;
	}
	else
	if (mat.getBlend())
	{
		// see if alpha is relevant to blending
		if (mat.getSrcBlend() == CMaterial::srcalpha ||
			mat.getSrcBlend() == CMaterial::invsrcalpha ||
			mat.getDstBlend() == CMaterial::srcalpha ||
			mat.getDstBlend() == CMaterial::invsrcalpha
		   )
		{
			alphaRelevant = true;
		}
	}
	if (!alphaRelevant)
	{
		alphaPipe[0].clear();
	}
	add(rgbPipe[0], alphaPipe[0]);
	numConstant = (uint)rgbPipe[0].size();
	if (numConstant)
	{
		firstConstant = *(rgbPipe[0].begin());
		if (numConstant == 2)
		{
			secondConstant = *(++rgbPipe[0].begin());
		}
		// can emulate up to 2 constants only ...
	}
	return false;

}

// ***************************************************************************
bool CDriverD3D::needsConstantForDiffuse (CMaterial &mat)
{
	H_AUTO_D3D(CDriverD3D_needsConstantForDiffuse)
	// If lighted, don't need the tfactor
	if (mat.isLighted())
		return false;
	if (mat.getTexture(0) == NULL) return true; // RGB diffuse is output
	// Inspect the RGB & alpha pipe : if a diffuse value manage to propagate to the last stage, and if the pipe is required for
	// fragment blending, then diffuse is useful in the shader, and thus must be replaced by constant for unlighted material
	bool propRGB = false;     // diffuse RGB propagated to current stage
	bool propAlpha = false;   // diffuse RGB propagated to current stage
	for (uint i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		if (!mat.getTexture(uint8(i))) break;
		CMaterial::CTexEnv texEnv;
		texEnv.EnvPacked = mat.getTexEnvMode(i);
		bool newPropRGB = false;
		bool newPropAlpha = false;
		if (texEnv.Env.OpRGB == CMaterial::InterpolateDiffuse || (texEnv.Env.OpRGB == CMaterial::InterpolatePrevious && i == 0))
		{
			newPropRGB = true;
		}
		if (texEnv.Env.OpRGB == CMaterial::InterpolatePrevious)
		{
			if (propAlpha) newPropRGB = true;
		}
		if (texEnv.Env.OpAlpha == CMaterial::InterpolateDiffuse || (texEnv.Env.OpAlpha == CMaterial::InterpolatePrevious && i == 0))
		{
			newPropAlpha = true;
		}
		if (texEnv.Env.OpAlpha == CMaterial::InterpolatePrevious)
		{
			if (propAlpha) newPropAlpha = true;
		}
		for(uint l = 0; l < OpNumArg[texEnv.Env.OpRGB]; ++l)
		{
			// color pipe
			if (texEnv.getColorArg(l) == CMaterial::Diffuse || (texEnv.getColorArg(l) == CMaterial::Previous && i == 0))
			{
				newPropRGB = true;
			}
			else
			if (texEnv.getColorArg(l) == CMaterial::Previous)
			{
				if (texEnv.getColorOperand(l) == CMaterial::SrcColor || texEnv.getColorOperand(l) == CMaterial::InvSrcColor)
				{
					if (propRGB) newPropRGB = true;
				}
				if (texEnv.getColorOperand(l) == CMaterial::SrcAlpha || texEnv.getColorOperand(l) == CMaterial::InvSrcColor)
				{
					if (propAlpha) newPropAlpha = true;
				}
			}
		}
		// alpha pipe
		for(uint l = 0; l < OpNumArg[texEnv.Env.OpAlpha]; ++l)
		{
			if (texEnv.getAlphaArg(l) == CMaterial::Diffuse || (texEnv.getAlphaArg(l) == CMaterial::Previous && i == 0))
			{
				newPropAlpha = true;

			}
			if (texEnv.getAlphaArg(l) == CMaterial::Previous)
			{
				if (propAlpha) newPropAlpha = true;
			}
		}

		propRGB = newPropRGB;
		propAlpha = newPropAlpha;
	}
	if (propRGB) return true;
	if (propAlpha)
	{
		if (mat.getAlphaTest()) return true;
		if (mat.getBlend())
		{
			// see if alpha is relevant to blending
			if (mat.getSrcBlend() == CMaterial::srcalpha ||
				mat.getSrcBlend() == CMaterial::invsrcalpha ||
				mat.getDstBlend() == CMaterial::srcalpha ||
				mat.getDstBlend() == CMaterial::invsrcalpha
			   )
			{
				return true;
			}
		}
	}
	return false;
}


// ***************************************************************************
void CDriverD3D::computeRelevantTexEnv(CMaterial &mat, bool rgbPipe[IDRV_MAT_MAXTEXTURES], bool alphaPipe[IDRV_MAT_MAXTEXTURES])
{
	// TODO : see if worth the trouble (during profile, I see a very small gain : 0.03% less time spent in driver for my test scene)
	// TODO : could be more efficient if I could see when alpha is useless...
	H_AUTO_D3D(CDriverD3D_computeRelevantTexEnv)

	static bool testStuff = false;
	if (testStuff)
	{
		std::fill(rgbPipe, rgbPipe + IDRV_MAT_MAXTEXTURES, true);
		std::fill(alphaPipe, alphaPipe + IDRV_MAT_MAXTEXTURES, true);
		return;
	}

	uint numStages = 0;
	for (uint i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		if (mat.getTexture(uint8(i)) == NULL) break;
		++ numStages;
	}
	std::fill(rgbPipe, rgbPipe + IDRV_MAT_MAXTEXTURES, false);
	std::fill(alphaPipe, alphaPipe + IDRV_MAT_MAXTEXTURES, false);
	bool rgbUsed = true;
	bool alphaUsed = false;

	/*
	if (mat.getAlphaTest())
	{
		alphaUsed = true;
	}
	else
	if (mat.getBlend())
	{
		// see if alpha is relevant to blending
		if (mat.getSrcBlend() == CMaterial::srcalpha ||
			mat.getSrcBlend() == CMaterial::invsrcalpha ||
			mat.getDstBlend() == CMaterial::srcalpha ||
			mat.getDstBlend() == CMaterial::invsrcalpha
		   )
		{
			alphaUsed = true;
		}
	}
	*/

	alphaUsed = true;

	if (numStages == 0)
	{
		// just diffuse material, no textures
		rgbPipe[0] = rgbUsed;
		alphaPipe[0] = alphaUsed;
		return;
	}
	sint currStage = numStages - 1;
	while (currStage >= 0)
	{
		rgbPipe[currStage] = rgbUsed;
		alphaPipe[currStage] = alphaUsed;
		bool newAlphaUsed = false;
		bool newRGBUsed = false;
		CMaterial::CTexEnv texEnv;
		texEnv.EnvPacked = mat.getTexEnvMode(currStage);
		// test for rgb propagate
		if (rgbUsed)
		{
			if (texEnv.Env.OpRGB == CMaterial::InterpolatePrevious)
			{
				newAlphaUsed = true;
			}
			for(uint l = 0; l < OpNumArg[texEnv.Env.OpRGB]; ++l)
			{
				if (texEnv.getColorArg(l) == CMaterial::Previous ||
					(currStage != 0 && texEnv.getColorArg(l) == CMaterial::Texture && mat.getTexEnvOpRGB(currStage - 1) == CMaterial::EMBM)
				   )
				{
					if (texEnv.getColorOperand(l) == CMaterial::SrcColor || texEnv.getColorOperand(l) == CMaterial::InvSrcColor)
					{
						newRGBUsed = true;
					}
					if (texEnv.getColorOperand(l) == CMaterial::SrcAlpha || texEnv.getColorOperand(l) == CMaterial::InvSrcAlpha)
					{
						newAlphaUsed = true;
					}
				}
			}
		}
		// test for alpha propagate
		if (alphaUsed)
		{
			if (texEnv.Env.OpAlpha == CMaterial::InterpolatePrevious)
			{
				newAlphaUsed = true;
			}
			for(uint l = 0; l < OpNumArg[texEnv.Env.OpAlpha]; ++l)
			{
				if (texEnv.getAlphaArg(l) == CMaterial::Previous ||
					(currStage != 0 && texEnv.getAlphaArg(l) == CMaterial::Texture && mat.getTexEnvOpRGB(currStage - 1) == CMaterial::EMBM)
				   )
				{
					if (texEnv.getAlphaOperand(l) == CMaterial::SrcAlpha || texEnv.getAlphaOperand(l) == CMaterial::InvSrcAlpha)
					{
						newAlphaUsed = true;
					}
				}
			}
		}
		alphaUsed = newAlphaUsed;
		rgbUsed = newRGBUsed;

		-- currStage;
	}
}


// ***************************************************************************

const char *RemapPSInstructions[CMaterial::TexOperatorCount]=
{
	"mov",	// Replace
	"mul",	// Modulate
	"add",	// Add
	"add",	// AddSigned
	"lrp",	// InterpolateTexture
	"lrp",	// InterpolatePrevious
	"lrp",	// InterpolateDiffuse
	"lrp",	// InterpolateConstant
	"bem",	// EMBM
	"mad"	// MAD
};

// ***************************************************************************

const char *RemapPSThirdArguments[CMaterial::TexOperatorCount][2]=
{
	{"", ""},		// Replace
	{"", ""},		// Modulate
	{"", ""},		// Add
	{"", ""},		// AddSigned
	{"t", "t"},		// InterpolateTexture
	{"r0", "r0"},	// InterpolatePrevious
	{"v0", "c5"},	// InterpolateDiffuse
	{"c", "c"},		// InterpolateConstant
	{"", ""},		// EMBM
	{"", ""}		// MAD (not used)
};

// ***************************************************************************

// Only for stage 0
const char *RemapPSThirdArguments0[CMaterial::TexOperatorCount][2]=
{
	{"", ""},		// Replace
	{"", ""},		// Modulate
	{"", ""},		// Add
	{"", ""},		// AddSigned
	{"t", "t"},		// InterpolateTexture
	{"v0", "c5"},	// InterpolatePrevious
	{"v0", "c5"},	// InterpolateDiffuse
	{"c", "c"},		// InterpolateConstant
	{"", ""},		// EMBM
	{"", ""}		// MAD (not used)
};

// ***************************************************************************

const char *RemapPSSecondRegisterModificator[CMaterial::TexOperatorCount]=
{
	"",		// Replace
	"",		// Modulate
	"",		// Add
	"_bias",	// AddSigned
	"",		// InterpolateTexture
	"",		// InterpolatePrevious
	"",		// InterpolateDiffuse
	"",		// InterpolateConstant
	"",		// EMBM
	""		// MAD
};

// ***************************************************************************

const char *RemapPSArguments[CMaterial::TexOperatorCount][2]=
{
	{"t", "t"},	// Texture
	{"r0", "r0"},	// Previous
	{"v0", "c5"},	// Diffuse
	{"c", "c"},	// Constant
};

// ***************************************************************************

// Only for stage 0
const char *RemapPSArguments0[CMaterial::TexOperatorCount][2]=
{
	{"t", "t"},	// Texture
	{"v0", "c5"},	// Previous
	{"v0", "c5"},	// Diffuse
	{"c", "c"},	// Constant
};

// ***************************************************************************

void buildColorOperation (string &dest, const char *prefix, const char *destSizzle, uint stage, CMaterial::TTexOperator &op, CMaterial::TTexSource src0, CMaterial::TTexSource src1, CMaterial::TTexSource src2, CMaterial::TTexOperand &op0, CMaterial::TTexOperand &op1, CMaterial::TTexOperand &op2, bool unlightedNoVertexColor)
{
	H_AUTO_D3D(buildColorOperation)
	// Refix
	dest += prefix;

	// RGB operation
	dest += RemapPSInstructions[op];

	// Destination argument
	dest += " r0";
	dest += destSizzle;

	// Need a third argument ?
	if (op != CMaterial::Mad)
	{
		const char *remapPSThirdArguments = ((stage==0)?RemapPSThirdArguments0:RemapPSThirdArguments)[op][(uint)unlightedNoVertexColor];
		if (remapPSThirdArguments[0] != 0)
		{
			dest += ", ";
			dest += remapPSThirdArguments;

			// Need stage postfix ?
			if ((op == CMaterial::InterpolateTexture) || (op == CMaterial::InterpolateConstant))
				dest += toString (stage);

			// Add alpha swizzle
			dest += ".w";
		}
	}

	// First argument
	dest += ", ";

	// Inverted ?
	if ((op0 == CMaterial::InvSrcColor) || (op0 == CMaterial::InvSrcAlpha))
		dest += "1-";

	// The first operator argument
	dest += ((stage==0)?RemapPSArguments0:RemapPSArguments)[src0][(uint)unlightedNoVertexColor];

	// Need stage postfix ?
	if ((src0 == CMaterial::Texture) || (src0 == CMaterial::Constant))
		dest += toString (stage);

	// Need alpha ?
	if ((op0 == CMaterial::SrcAlpha) || (op0 == CMaterial::InvSrcAlpha))
		dest += ".w";

	if (op != CMaterial::Replace)
	{
		dest += ", ";

		// Inverted ?
		if ((op1 == CMaterial::InvSrcColor) || (op1 == CMaterial::InvSrcAlpha))
			dest += "1-";

		dest += ((stage==0)?RemapPSArguments0:RemapPSArguments)[src1][(uint)unlightedNoVertexColor];

		// Second register modifier
		dest += RemapPSSecondRegisterModificator[op];

		// Need stage postfix ?
		if ((src1 == CMaterial::Texture) || (src1 == CMaterial::Constant))
			dest += toString (stage);

		// Need alpha ?
		if ((op1 == CMaterial::SrcAlpha) || (op1 == CMaterial::InvSrcAlpha))
			dest += ".w";
	}

	if (op == CMaterial::Mad)
	{
		dest += ", ";

		// Inverted ?
		if ((op2 == CMaterial::InvSrcColor) || (op2 == CMaterial::InvSrcAlpha))
			dest += "1-";

		dest += ((stage==0)?RemapPSArguments0:RemapPSArguments)[src2][(uint)unlightedNoVertexColor];

		// Second register modifier
		dest += RemapPSSecondRegisterModificator[op];

		// Need stage postfix ?
		if ((src2 == CMaterial::Texture) || (src2 == CMaterial::Constant))
			dest += toString (stage);

		// Need alpha ?
		if ((op2 == CMaterial::SrcAlpha) || (op2 == CMaterial::InvSrcAlpha))
			dest += ".w";
	}

	// End
	dest += ";\n";
}

// ***************************************************************************

IDirect3DPixelShader9	*CDriverD3D::buildPixelShader (const CNormalShaderDesc &normalShaderDesc, bool unlightedNoVertexColor)
{
	H_AUTO_D3D(CDriverD3D_buildPixelShader)
	static string shaderText;
	shaderText = "ps_1_1;\n";

	// Look for a shader already created
	std::list<CNormalShaderDesc> &normalPixelShaders = _NormalPixelShaders[(uint)unlightedNoVertexColor];
	std::list<CNormalShaderDesc>::iterator ite = normalPixelShaders.begin();
	while (ite != normalPixelShaders.end())
	{
		if (normalShaderDesc == *ite)
		{
			// Good one
			return (*ite).PixelShader;
		}
		ite++;
	}

	// For each state, texture lookup
	uint i;
	for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		// Stage used ?
		if (normalShaderDesc.StageUsed[i])
		{
			#ifdef NL_DEBUG
				CMaterial::CTexEnv texEnv;
				texEnv.EnvPacked = normalShaderDesc.TexEnvMode[i];
				nlassert(texEnv.Env.OpRGB != CMaterial::EMBM); // Pixel shader equivalent for EMBM not supported
																				   // For now this is not a problem because
																				   // buildPixelShader is used when the shader uses per stage color
																				   // (which is not always supported by the fixed pixel pipe),
																				   // and currently all our shaders with EMBM don't use that feature.
			#endif
			shaderText += "tex t"+toString (i)+";\n";
		}
	}

	// For each state, color operations
	for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		// Stage used ?
		if (normalShaderDesc.StageUsed[i])
		{
			// Texenv
			CMaterial::CTexEnv texEnv;
			texEnv.EnvPacked = normalShaderDesc.TexEnvMode[i];

			// RGB
			CMaterial::TTexOperator texOp = (CMaterial::TTexOperator)texEnv.Env.OpRGB;
			CMaterial::TTexSource src0 = (CMaterial::TTexSource)texEnv.Env.SrcArg0RGB;
			CMaterial::TTexSource src1 = (CMaterial::TTexSource)texEnv.Env.SrcArg1RGB;
			CMaterial::TTexSource src2 = (CMaterial::TTexSource)texEnv.Env.SrcArg2RGB;
			CMaterial::TTexOperand op0 = (CMaterial::TTexOperand)texEnv.Env.OpArg0RGB;
			CMaterial::TTexOperand op1 = (CMaterial::TTexOperand)texEnv.Env.OpArg1RGB;
			CMaterial::TTexOperand op2 = (CMaterial::TTexOperand)texEnv.Env.OpArg2RGB;
			buildColorOperation (shaderText, "", ".xyz", i, texOp, src0, src1, src2, op0, op1, op2, unlightedNoVertexColor);

			// Alpha
			texOp = (CMaterial::TTexOperator)texEnv.Env.OpAlpha;
			src0 = (CMaterial::TTexSource)texEnv.Env.SrcArg0Alpha;
			src1 = (CMaterial::TTexSource)texEnv.Env.SrcArg1Alpha;
			src2 = (CMaterial::TTexSource)texEnv.Env.SrcArg2Alpha;
			op0 = (CMaterial::TTexOperand)texEnv.Env.OpArg0Alpha;
			op1 = (CMaterial::TTexOperand)texEnv.Env.OpArg1Alpha;
			op2 = (CMaterial::TTexOperand)texEnv.Env.OpArg2Alpha;
			buildColorOperation (shaderText, "+", ".w", i, texOp, src0, src1, src2, op0, op1, op2, unlightedNoVertexColor);
		}
		// No texture shader ?
		else if (i == 0)
		{
			// RGB
			CMaterial::TTexOperator texOp = CMaterial::Replace;
			CMaterial::TTexSource src0 = CMaterial::Diffuse;
			CMaterial::TTexOperand op0 = CMaterial::SrcColor;
			buildColorOperation (shaderText, "", ".xyz", i, texOp, src0, src0, src0, op0, op0, op0, unlightedNoVertexColor);

			// Alpha
			texOp = CMaterial::Replace;
			src0 = CMaterial::Diffuse;
			op0 = CMaterial::SrcAlpha;
			buildColorOperation (shaderText, "+", ".w", i, texOp, src0, src0, src0, op0, op0, op0, unlightedNoVertexColor);
		}
	}

	// Add the specular
	shaderText += "add r0.rgb, r0, v1;\n";

	// Dump the pixel shader
#ifdef NL_DEBUG_D3D
	nlinfo("Assemble Pixel Shader : ");
	string::size_type lineBegin = 0;
	string::size_type lineEnd;
	while ((lineEnd = shaderText.find('\n', lineBegin)) != string::npos)
	{
		nlinfo(shaderText.substr (lineBegin, lineEnd-lineBegin).c_str());
		lineBegin = lineEnd+1;
	}
	nlinfo(shaderText.substr (lineBegin, lineEnd-lineBegin).c_str());
#endif // NL_DEBUG_D3D

	// Add the shader
	normalPixelShaders.push_back (normalShaderDesc);

	// Assemble and create the shader
	LPD3DXBUFFER pShader;
	LPD3DXBUFFER pErrorMsgs;
	if (D3DXAssembleShader (shaderText.c_str(), (UINT)shaderText.size(), NULL, NULL, 0, &pShader, &pErrorMsgs) == D3D_OK)
	{
		IDirect3DPixelShader9 *shader;
		if (_DeviceInterface->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), &shader) == D3D_OK)
			normalPixelShaders.back().PixelShader = shader;
		else
			normalPixelShaders.back().PixelShader = NULL;
	}
	else
	{
		nlwarning ("Can't assemble pixel shader:");
		nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
		normalPixelShaders.back().PixelShader = NULL;
	}

	return normalPixelShaders.back().PixelShader;
}

// ***************************************************************************

void CDriverD3D::startSpecularBatch()
{
	/* Not used in direct3d, use normal caching */
}

// ***************************************************************************

void CDriverD3D::endSpecularBatch()
{
	/* Not used in direct3d, use normal caching */
}

// ***************************************************************************

bool CDriverD3D::supportBlendConstantColor() const
{
	/* Not supported in D3D */
	return false;
};

// ***************************************************************************

void CDriverD3D::setBlendConstantColor(NLMISC::CRGBA /* col */)
{
	/* Not supported in D3D */
};

// ***************************************************************************

NLMISC::CRGBA CDriverD3D::getBlendConstantColor() const
{
	/* Not supported in D3D */
	return CRGBA::White;
};

// ***************************************************************************

void CDriverD3D::enablePolygonSmoothing(bool /* smooth */)
{
	/* Not supported in D3D */
}

// ***************************************************************************

bool CDriverD3D::isPolygonSmoothingEnabled() const
{
	/* Not supported in D3D */
	return false;
}

// ***************************************************************************

sint CDriverD3D::beginMaterialMultiPass()
{
	H_AUTO_D3D(CDriverD3D_beginMaterialMultiPass)
	beginMultiPass ();
	return _CurrentShaderPassCount;
}

// ***************************************************************************

void CDriverD3D::setupMaterialPass(uint pass)
{
	H_AUTO_D3D(CDriver3D_setupMaterialPass);
	activePass (pass);
}

// ***************************************************************************

void CDriverD3D::endMaterialMultiPass()
{
	H_AUTO_D3D(CDriver3D_endMaterialMultiPass);
	endMultiPass ();
}

// ***************************************************************************

bool CDriverD3D::supportCloudRenderSinglePass () const
{
	H_AUTO_D3D(CDriver3D_supportCloudRenderSinglePass);
	return _PixelProgram;
}

// ***************************************************************************

void CDriverD3D::getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const
{
	lightedMaterial = _MaxNumPerStageConstantLighted;
	unlightedMaterial = _MaxNumPerStageConstantUnlighted;
}



} // NL3D
