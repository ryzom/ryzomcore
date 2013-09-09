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

#ifndef NL_MATERIAL_H
#define NL_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"
#include "nel/3d/texture.h"

#include <memory>

namespace NL3D {

using NLMISC::CRefCount;
using NLMISC::CRGBA;
using NLMISC::CSmartPtr;
using NLMISC::CRefPtr;

// --------------------------------------------------

const uint32 IDRV_MAT_MAXTEXTURES	=	4;

const uint32 IDRV_TOUCHED_BLENDFUNC			=	0x00000001;
const uint32 IDRV_TOUCHED_BLEND				=	0x00000002;
const uint32 IDRV_TOUCHED_SHADER			=	0x00000004;
const uint32 IDRV_TOUCHED_ZFUNC				=	0x00000008;
const uint32 IDRV_TOUCHED_ZBIAS				=	0x00000010;
const uint32 IDRV_TOUCHED_COLOR				=	0x00000020;
const uint32 IDRV_TOUCHED_LIGHTING			=	0x00000040;
const uint32 IDRV_TOUCHED_DEFMAT			=	0x00000080;
const uint32 IDRV_TOUCHED_ZWRITE			=	0x00000100;
const uint32 IDRV_TOUCHED_DOUBLE_SIDED		=	0x00000200;
const uint32 IDRV_TOUCHED_LIGHTMAP			=	0x00000400;
const uint32 IDRV_TOUCHED_ALPHA_TEST		=	0x00000800;
const uint32 IDRV_TOUCHED_ALPHA_TEST_THRE	=	0x00001000;
const uint32 IDRV_TOUCHED_TEXENV			=	0x00002000;
const uint32 IDRV_TOUCHED_TEXGEN			=	0x00004000;


// Start texture touch at 0x10000.
const uint32 IDRV_TOUCHED_TEX[IDRV_MAT_MAXTEXTURES]		=
	{0x00010000, 0x00020000, 0x00040000, 0x00080000};
const uint32 IDRV_TOUCHED_ALLTEX	=	0x000F0000;
const uint32 IDRV_TOUCHED_ALL		=	0xFFFFFFFF;


const uint32 IDRV_MAT_HIDE			=	0x00000001;
const uint32 IDRV_MAT_TSP			=	0x00000002;
const uint32 IDRV_MAT_ZWRITE		=	0x00000004;
const uint32 IDRV_MAT_ZLIST			=	0x00000008;
const uint32 IDRV_MAT_LIGHTING		=	0x00000010;
const uint32 IDRV_MAT_SPECULAR		=	0x00000020;
const uint32 IDRV_MAT_DEFMAT		=	0x00000040;	// NB: Deprecated, but may still exist streams.
const uint32 IDRV_MAT_BLEND			=	0x00000080;
const uint32 IDRV_MAT_DOUBLE_SIDED	=	0x00000100;
const uint32 IDRV_MAT_ALPHA_TEST	= 	0x00000200;
const uint32 IDRV_MAT_TEX_ADDR	    = 	0x00000400;
const uint32 IDRV_MAT_LIGHTED_VERTEX_COLOR	= 	0x00000800;
///   automatic texture coordinate generation
const uint32 IDRV_MAT_GEN_TEX_0		= 	0x00001000;
const uint32 IDRV_MAT_GEN_TEX_1		= 	0x00002000;
const uint32 IDRV_MAT_GEN_TEX_2		= 	0x00004000;
const uint32 IDRV_MAT_GEN_TEX_3		= 	0x00008000;
const uint32 IDRV_MAT_GEN_TEX_4		= 	0x00010000;
const uint32 IDRV_MAT_GEN_TEX_5		= 	0x00020000;
const uint32 IDRV_MAT_GEN_TEX_6		= 	0x00040000;
const uint32 IDRV_MAT_GEN_TEX_7		= 	0x00080000;
///   user texture matrix
const uint32 IDRV_MAT_USER_TEX_0_MAT	= 	0x00100000;
const uint32 IDRV_MAT_USER_TEX_1_MAT	= 	0x00200000;
const uint32 IDRV_MAT_USER_TEX_2_MAT	= 	0x00400000;
const uint32 IDRV_MAT_USER_TEX_3_MAT	= 	0x00800000;
const uint32 IDRV_MAT_USER_TEX_4_MAT	= 	0x01000000;
const uint32 IDRV_MAT_USER_TEX_5_MAT	= 	0x02000000;
const uint32 IDRV_MAT_USER_TEX_6_MAT	= 	0x04000000;
const uint32 IDRV_MAT_USER_TEX_7_MAT	= 	0x08000000;
const uint32 IDRV_MAT_USER_TEX_MAT_ALL  =   0x0FF00000;

const uint32 IDRV_MAT_USER_TEX_FIRST_BIT = 20;

// For TexCoordGen
const uint32 IDRV_MAT_TEX_GEN_SHIFT  =   2;
const uint32 IDRV_MAT_TEX_GEN_MASK  =   0x03;


// List typedef.
class	IMaterialDrvInfos;
typedef	std::list<IMaterialDrvInfos*>			TMatDrvInfoPtrList;
typedef	TMatDrvInfoPtrList::iterator	ItMatDrvInfoPtrList;

// ***************************************************************************
/**
 *  Driver info for the material
 */
class IMaterialDrvInfos : public CRefCount
{
private:
	IDriver				*_Driver;
	ItMatDrvInfoPtrList		_DriverIterator;

public:
	IMaterialDrvInfos(IDriver	*drv, ItMatDrvInfoPtrList it) {_Driver= drv; _DriverIterator= it;}
	// The virtual dtor is important.
	virtual ~IMaterialDrvInfos();

};

// ***************************************************************************
/**
 * A material represent ALL the states relatives to the aspect of a primitive.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CMaterial : public CRefCount
{
public:

	enum ZFunc				{ always=0,never,equal,notequal,less,lessequal,greater,greaterequal, zfuncCount };

	/** Blend enums. see setSrcBlend()/setDstBlend()/setBlendFunc().
	 *	blendConstant* enums are only valid if driver->supportBlendConstantColor().
	 *	\see IDriver::supportBlendConstantColor()
	 */
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor,
		blendConstantColor, blendConstantInvColor, blendConstantAlpha, blendConstantInvAlpha, blendCount };

	/**
	 * Normal shader:
	 *	- use simple multitexturing. see texEnv*() methods.
	 * Bump:
	 *	- not implemented yet.
	 * UserColor:
	 *	- UserColor (see setUserColor()) is blended with precomputed texture/textureAlpha.
	 *	- Alpha Blending ignore Alpha of texture (of course :) ), but use Alpha diffuse (vertex/material color).
	 * LightMap:
	 *	- Texture of stage 0 is blended with sum of lightmaps (see setLightmap()). Vertex Color (or color, or lighting)
	 *	doesn't affect the final result (neither diffuse part nor specular part).
	 *	Blending is special. If enabled, Lightmap shader apply a standard transparency srcalpha/invsrcalpha.
	 *	- NB: if no texture in stage 0, undefined result.
	 *	- UV0 is the UV for decal Texture. UV1 is the UVs for all the lightmaps.
	 * Specular:
	 *  - Texture of stage 0 is added to the multiplication of Texture Alpha of stage 0 and Texture of stage 1
	 *  - This is done in 2 passes
	 * PerPixelLighting :
	 *  - When not supported by the driver, this is equivalent to the normal shader. This can be querried from the driver
	 *  - When supported by the driver, the strongest light is rendered using per pixel lighting. The last tex coordinate must be the S vector
	 *    of the tangent space basis (oriented in the direction where the s texture coords grows). Other lights are rendered using gouraud shaing. The light setup is done in the driver.
	 * PerPixelLighting : The same as PerPixelLighting but with no specular
	 * Caustics: NOT IMPLEMENTED
	 * Cloud :
	 * - Alpha of texture in stage 0 is blended with alpha of texture in stage 1. Blend done with the alpha color of each
	 * stage and the whole is multiplied by the alpha in color vertex [AT0*ADiffuseCol+AT1*(1-ADiffuseCol)]*AStage
	 * - RGB still unchanged
	 * Water :
	 * - Water
	 */
	enum TShader			{ Normal=0,
							  Bump,
							  UserColor,
							  LightMap,
							  Specular,
							  Caustics,
							  PerPixelLighting,
							  PerPixelLightingNoSpec,
							  Cloud,
							  Water,
							  shaderCount,
							  Program /* internally used when a pixel program is active */ };

	/// \name Texture Env Modes.
	// @{
	/** Environements operators:
	 * Replace:			out= arg0
	 * Modulate:		out= arg0 * arg1
	 * Add:				out= arg0 + arg1
	 * AddSigned:		out= arg0 + arg1 -0.5
	 * Interpolate*:	out= arg0*As + arg1*(1-As),  where As is taken from the SrcAlpha of
	 *		Texture/Previous/Diffuse/Constant, respectively if operator is
	 *		InterpolateTexture/InterpolatePrevious/InterpolateDiffuse/InterpolateConstant.
	 * Multiply-Add (Mad) out= arg0 * arg1 + arg2. Must be supported by driver (see IDriver::supportMADOperator)
	 * EMBM : apply to both color and alpha : the current texture, whose format is DSDT, is used to offset the texture in the next stage, unless the EMBM unit is at the last stage, in which case it operates on texture at first stage
	 *  NB : for EMBM, this must be supported by driver.
	 */
	enum TTexOperator		{ Replace=0, Modulate, Add, AddSigned,
							  InterpolateTexture, InterpolatePrevious, InterpolateDiffuse, InterpolateConstant, EMBM, Mad, TexOperatorCount };

	/** Source argument.
	 * Texture:		the arg is taken from the current texture of the stage.
	 * Previous:	the arg is taken from the previous enabled stage. If stage 0, Previous==Diffuse.
	 * Diffuse:		the arg is taken from the primary color vertex.
	 * Constant:	the arg is taken from the constant color setuped for this texture stage.
	 */
	enum TTexSource			{ Texture=0, Previous, Diffuse, Constant, TexSourceCount };

	/** Operand for the argument.
	 * For Alpha arguments, only SrcAlpha and InvSrcAlpha are Valid!! \n
	 * SrcColor:	arg= ColorSource.
	 * InvSrcColor:	arg= 1-ColorSource.
	 * SrcAlpha:	arg= AlphaSource.
	 * InvSrcAlpha:	arg= 1-AlphaSource.
	 */
	enum TTexOperand		{ SrcColor=0, InvSrcColor, SrcAlpha, InvSrcAlpha, TexOperandCount };
	// @}

	/** \name Texture Addressing Modes. They are valid only with the normal texture shader.
	  *	All modes are not supported everywhere, so you should check for it in the driver.
	  * The modes are similar to those introduced with DirectX 8.0 Pixel Shaders and OpenGL
	  * TEXTURE_SHADERS_NV
	  */
	// @{
	enum TTexAddressingMode {
							 TextureOff = 0 /* no texture */, FetchTexture, PassThrough, CullFragment,
							 OffsetTexture, OffsetTextureScale,
							 DependentARTexture, DependentGBTexture,
							 DP3, DP3Texture2D,
							 DP3CubeMap, DP3ReflectCubeMap, DP3ConstEyeReflectCubeMap,
							 DP3DiffuseCubeMap,
							 DP3DepthReplace,
							 TexAddrCount
							};
	// @}

	/**	TexGen Mode.
	 *	TexCoordGenReflect: For Cube or Spherical EnvMapping.
	 *	TexCoordGenObjectSpace: The UVW are generated from the XYZ defined in ObjectSpace (before transformation)
	 *	TexCoordGenEyeSpace: The UVW are generated from the XYZ defined in EyeSpace (after ModelViewMatrix transformation)
	 *	NB: use the TextureMatrix for more control on the wanted effect (eg: shadowMap projection etc...)
	 */
	enum TTexCoordGenMode	{TexCoordGenReflect=0, TexCoordGenObjectSpace, TexCoordGenEyeSpace, numTexCoordGenMode};

public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/// \name Object.
	// @{
	/** ctor.
	 * By default, shader is normal, SrcBlend is srcalpha, dstblend is invsrcalpha, ZFunction is lessequal, ZBias is 0,
	 * Color is White: (255,255,255,255), not double sided.
	 */
	CMaterial();
	/// see operator=.
	CMaterial(const CMaterial &mat) : CRefCount() {_Touched= 0;_Flags=0; operator=(mat);}
	/// dtor.
	~CMaterial();
	/// Do not copy DrvInfos, copy all infos and set IDRV_TOUCHED_ALL.
	CMaterial				&operator=(const CMaterial &mat);
	// @}

	/** Set the shader for this material.
	 * All textures are reseted!!
	 */
	void					setShader(TShader val);
	/// get the current material shadertype.
	TShader					getShader() const {return _ShaderType;}

	/// \name Texture.
	// @{
	/**
	 * set a texture for a special stage. Different usage according to shader:
	 *	- Normal shader do multitexturing (see texEnv*() methods).
	 *	- UserColor assert if stage!=0. (NB: internal only: UserColor setup texture to stage 0 and 1).
	 *	- LightMap assert if stage!=0.
	 */
	void 					setTexture(uint8 stage, ITexture* ptex);

	ITexture*				getTexture(uint8 stage) const;
	bool					texturePresent(uint8 stage) const;
	// get the number of stages for which a texture is used
	uint					getNumUsedTextureStages() const;
	// @}


	/// \name Blending.
	// @{
	void					setBlend(bool active);
	void					setBlendFunc(TBlend src, TBlend dst);
	void					setSrcBlend(TBlend val);
	void					setDstBlend(TBlend val);

	bool					getBlend() const { return (_Flags&IDRV_MAT_BLEND)!=0; }
	TBlend					getSrcBlend(void)  const { return(_SrcBlend); }
	TBlend					getDstBlend(void)  const { return(_DstBlend); }
	// @}


	/// \name Texture Addressing Mode Method
	// @{
	/** enable / disable the use of special texture addressing modes
	  * When enabled, all texture addressing modes are set to 'None'
	  */
	void					enableTexAddrMode(bool enable = true);

	/// test whether texture addressing mode are enabled
	bool					texAddrEnabled() const;

	/** Set a texture addressing mode for the given stage.
	  * You should test if this mode is supported in the driver you plane to use.
	  * Texture addressing modes should have been enabled otherwise an assertion is raised
	  */
	void					setTexAddressingMode(uint8 stage, TTexAddressingMode mode);

	/// Get the texture addressing mode for the given stage
	TTexAddressingMode		getTexAddressingMode(uint8 stage);
	// @}


	/// \name Double sided.
	// @{
	void					setDoubleSided(bool active);
	bool					getDoubleSided() const { return (_Flags&IDRV_MAT_DOUBLE_SIDED)!=0; }
	// @}

	/// \name Alpha Test
	// @{
	void					setAlphaTest(bool active);
	bool					getAlphaTest() const { return (_Flags&IDRV_MAT_ALPHA_TEST)!=0; }

	/** change the threshold against alpha is tested. Default is 0.5f.
	 *	\param thre threshold, should be in [0..1], not clamped.
	 */
	void					setAlphaTestThreshold(float thre);
	float					getAlphaTestThreshold() const { return _AlphaTestThreshold; }

	// @}


	/// \name ZBuffer.
	// @{
	void					setZFunc(ZFunc val);
	void					setZWrite(bool active);

	/// The z bias is a z dispacement of the object to solve z precision problem. The bias is gived in world coordinate units.
	/// Positive bias give a lower z priority, negative bias give a higher bias priority.
	void					setZBias(float val);

	ZFunc					getZFunc(void)  const { return(_ZFunction); }
	bool					getZWrite(void)  const{ return (_Flags&IDRV_MAT_ZWRITE)!=0; }
	float					getZBias(void)  const { return(_ZBias); }
	// @}


	/// \name Color/Lighting..
	// @{
	/// The Color is used only if lighting is disabled. Also, color is replaced by per vertex color (if any).
	void					setColor(CRGBA rgba);

	/// Batch setup lighting. Opacity is in diffuse.A.
	void					setLighting(	bool active,
											CRGBA emissive=CRGBA(0,0,0),
											CRGBA ambient=CRGBA(0,0,0),
											CRGBA diffuse=CRGBA(0,0,0),
											CRGBA specular=CRGBA(0,0,0),
											float shininess= 10);

	/// Set the emissive part ot material. Useful only if setLighting(true) has been done.
	void					setEmissive( CRGBA emissive=CRGBA(0,0,0) );
	/// Set the Ambient part ot material. Useful only if setLighting(true) has been done.
	void					setAmbient( CRGBA ambient=CRGBA(0,0,0) );
	/// Set the Diffuse part ot material. Useful only if setLighting(true) has been done. NB: opacity is NOT copied from diffuse.A.
	void					setDiffuse( CRGBA diffuse=CRGBA(0,0,0) );
	/// Set the Opacity part ot material. Useful only if setLighting(true) has been done.
	void					setOpacity( uint8	opa );
	/// Set the specular part ot material. Useful only if setLighting(true) has been done.
	void					setSpecular( CRGBA specular=CRGBA(0,0,0) );
	/// Set the shininess part ot material. Useful only if setLighting(true) has been done.
	void					setShininess( float shininess );
	/// Set the color material flag. Used when the material is lighted. True to use the diffuse color of the material when lighted, false to use the color vertex.
	void					setLightedVertexColor (bool useLightedVertexColor);
	/// Get the lighted vertex color flag
	bool					getLightedVertexColor () const;


	bool					isLighted() const {return (_Flags&IDRV_MAT_LIGHTING)!=0;}

	/// Return true if this material uses color material as diffuse when lighted, else return false if it uses color vertex.
	bool					isLightedVertexColor () const { return (_Flags&IDRV_MAT_LIGHTED_VERTEX_COLOR)!=0;}

	CRGBA					getColor(void) const { return(_Color); }
	CRGBA					getEmissive() const { return _Emissive;}
	CRGBA					getAmbient() const { return _Ambient;}
	/// return diffuse part. NB: A==opacity.
	CRGBA					getDiffuse() const { return _Diffuse;}
	uint8					getOpacity() const { return _Diffuse.A;}
	CRGBA					getSpecular() const { return _Specular;}
	float					getShininess() const { return _Shininess;}
	// @}


	/// \name Texture environnement. Normal shader only.
	/** This part is valid for Normal shaders (nlassert). It maps the EXT_texture_env_combine opengl extension.
	 * A stage is enabled if his texture is !=NULL. By default, all stages are setup to Modulate style:
	 *  AlphaOp=RGBOp= Modulate, RGBArg0= TextureSrcColor, RGBArg1= PreviousSrcColor,
	 *  AlphaArg0= TextureSrcAlpha, AlphaArg1= PreviousSrcAlpha.  ConstantColor default to White(255,255,255,255).
	 *
	 * For compatibility problems:
	 * - no scaling is allowed (some cards do not implement this well).
	 * - Texture can be the source only for Arg0 (DirectX restriction). nlassert...
	 *
	 * NB: for Alpha Aguments, only operands SrcAlpha and InvSrcAlpha are valid (nlassert..).
	 */
	// @{
	void					texEnvOpRGB(uint stage, TTexOperator ope);
	void					texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvArg2RGB(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvOpAlpha(uint stage, TTexOperator ope);
	void					texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvArg2Alpha(uint stage, TTexSource src, TTexOperand oper);
	/// Setup the constant color for a stage. Used for the TTexSource:Constant.
	void					texConstantColor(uint stage, CRGBA color);
	/// For push/pop only, get the packed version of the environnment mode.
	uint32					getTexEnvMode(uint stage);
	/// For push/pop only, set the packed version of the environnment mode.
	void					setTexEnvMode(uint stage, uint32 packed);
	CRGBA					getTexConstantColor(uint stage);

	// Get rgb operation at the given stage. Must use Normal shader or nlassert(0)
	TTexOperator			getTexEnvOpRGB(uint stage) const;
	// Get alpha operation at the given stage. Must use Normal shader or nlassert(0)
	TTexOperator			getTexEnvOpAlpha(uint stage) const;

	// Enable or disable TexCoordGeneration
	void					setTexCoordGen(uint stage, bool generate);
	bool					getTexCoordGen(uint stage) const;

	// By default the mode is TexCoordGenReflect.
	void					setTexCoordGenMode(uint stage, TTexCoordGenMode mode);
	TTexCoordGenMode		getTexCoordGenMode(uint stage) const {return (TTexCoordGenMode)((_TexCoordGenMode >> (stage*IDRV_MAT_TEX_GEN_SHIFT))&IDRV_MAT_TEX_GEN_MASK);}

	// Enable a user texture matrix for the n-th stage. The initial matrix is set to identity.
	void                    enableUserTexMat(uint stage, bool enabled = true);
	// Test whether a user texture is enabled for the n-th stage
	bool                    isUserTexMatEnabled(uint stage) const;
	/// Set a new texture matrix for the given stage.
	void					setUserTexMat(uint stage, const NLMISC::CMatrix &m);
	/** Get a const ref. on the texture matrix of the n-th stage.
	  * User texture matrix must be enabled for that stage, otherwise an assertion is raised.
	  */
	const NLMISC::CMatrix  &getUserTexMat(uint stage) const;
	/// Decompose a user texture matrix, We assume that this is only a matrix for 2d texture.
	void					decompUserTexMat(uint stage, float &uTrans, float &vTrans, float &wRot, float &uScale, float &vScale);

	// @}


	/// \name Texture UserColor. UserColor shader only.
	/** This part is valid for Normal shaders (nlassert).
	 * \see TShader.
	 */
	// @{
	void					setUserColor(CRGBA userColor);
	CRGBA					getUserColor() const;
	// @}

	/// \name LightMap. LightMap shader only.
	/** This part is valid for LightMap shaders (nlassert).
	 * \see TShader.
	 */
	// @{
	/// Set the ith lightmap. undef results if holes in lightmap array.
	void					setLightMap(uint lmapId, ITexture *lmap);
	/// Get the ith lightmap. (NULL if none)
	ITexture				*getLightMap(uint lmapId) const;
	/// Set the lightmap intensity. (def : White)
	void					setLightMapFactor(uint lmapId, CRGBA factor);
	/// Set the multiply x 2 mode to burn colors (used with lightmaps 8 bits) (def: false)
	void					setLightMapMulx2(bool val) { _LightMapsMulx2 = val; }
	/// Used for LightMap Compression (LMC). Set the LMC color terms. default to black/white (no compression)
	void					setLMCColors(uint lmapId, CRGBA ambColor, CRGBA diffColor);
	// @}


	/// \name Tools..
	// @{
	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initUnlit();
	/** Init the material as default white lighted material. normal shader, lighting ....
	 * Default to: normal shader, full black lighting, no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initLighted();
	// @}

	bool					getStainedGlassWindow() { return _StainedGlassWindow; }
	void					setStainedGlassWindow(bool val) { _StainedGlassWindow = val; }

	/// Flush textures. Force texture generation.
	void					flushTextures (IDriver &driver, uint selectedTexture);

	void		serial(NLMISC::IStream &f);

	// \name Multiple texture set managment
	// @{
		/** Select one texture set for all the textures of this material.
		  * This is useful only if textures of this material support selection of course (such as CTextureMultiFile)
		  */
		void		selectTextureSet(uint index);
	// @}

	/** test if material a driver supports rendering of that material
	  * \param  forceBaseCaps When true, the driver is considered to have the most basic required caps (2 stages hardwares, no pixelShader, support for constant color blend & multiply-add texture operator), so that any fancy material will fail the test.
	  */
	bool			isSupportedByDriver(IDriver &drv, bool forceBaseCaps) const;

// **********************************
// Private part.
public:
	// Private. For Driver only.
	struct CTexEnv
	{
		union
		{
			uint32	EnvPacked;
			struct
			{
				uint32		OpRGB:4;
				uint32		SrcArg0RGB:2;
				uint32		OpArg0RGB:2;
				uint32		SrcArg1RGB:2;
				uint32		OpArg1RGB:2;
				uint32		SrcArg2RGB:2;
				uint32		OpArg2RGB:2;

				uint32		OpAlpha:4;
				uint32		SrcArg0Alpha:2;
				uint32		OpArg0Alpha:2;
				uint32		SrcArg1Alpha:2;
				uint32		OpArg1Alpha:2;
				uint32		SrcArg2Alpha:2;
				uint32		OpArg2Alpha:2;
			}		Env;
		};
		CRGBA		ConstantColor;

		void		setDefault()
		{
			// Don't worry, Visual optimize it quite well...
			// We cannot do better, because bit fields ordeinrg seems not to be standardized, so we can not
			// set Packed directly.
			Env.OpRGB= Modulate;
			Env.SrcArg0RGB= Texture;
			Env.OpArg0RGB= SrcColor;
			Env.SrcArg1RGB= Previous;
			Env.OpArg1RGB= SrcColor;
			Env.SrcArg2RGB= Previous;
			Env.OpArg2RGB= SrcColor;

			Env.OpAlpha= Modulate;
			Env.SrcArg0Alpha= Texture;
			Env.OpArg0Alpha= SrcAlpha;
			Env.SrcArg1Alpha= Previous;
			Env.OpArg1Alpha= SrcAlpha;
			Env.SrcArg2Alpha= Previous;
			Env.OpArg2Alpha= SrcAlpha;
			ConstantColor.set(255,255,255,255);
		}

		// Version added because only CMaterial has version number
		// Version 0 : binary ops & lerp
		// Version 1 : added 'mad' operator
		void		serial(NLMISC::IStream &f, sint version)
		{

			Env.OpRGB= f.serialBitField8(Env.OpRGB);
			Env.SrcArg0RGB= f.serialBitField8(Env.SrcArg0RGB);
			Env.OpArg0RGB= f.serialBitField8(Env.OpArg0RGB);
			Env.SrcArg1RGB= f.serialBitField8(Env.SrcArg1RGB);
			Env.OpArg1RGB= f.serialBitField8(Env.OpArg1RGB);
			if (version > 0)
			{
				Env.SrcArg2RGB= f.serialBitField8(Env.SrcArg2RGB);
				Env.OpArg2RGB= f.serialBitField8(Env.OpArg2RGB);
			}

			Env.OpAlpha= f.serialBitField8(Env.OpAlpha);
			Env.SrcArg0Alpha= f.serialBitField8(Env.SrcArg0Alpha);
			Env.OpArg0Alpha= f.serialBitField8(Env.OpArg0Alpha);
			Env.SrcArg1Alpha= f.serialBitField8(Env.SrcArg1Alpha);
			Env.OpArg1Alpha= f.serialBitField8(Env.OpArg1Alpha);
			if (version > 0)
			{
				Env.SrcArg2Alpha= f.serialBitField8(Env.SrcArg2Alpha);
				Env.OpArg2Alpha= f.serialBitField8(Env.OpArg2Alpha);
			}
			f.serial(ConstantColor);
		}


		CTexEnv()
		{
			setDefault();
		}

		// helpers
		inline uint getColorArg(uint index)
		{
			switch(index)
			{
				case 0: return Env.SrcArg0RGB;
				case 1: return Env.SrcArg1RGB;
				case 2: return Env.SrcArg2RGB;
				default:
					nlassert(0);
				break;
			}
			return 0;
		}
		inline uint getAlphaArg(uint index)
		{
			switch(index)
			{
				case 0: return Env.SrcArg0Alpha;
				case 1: return Env.SrcArg1Alpha;
				case 2: return Env.SrcArg2Alpha;
				default:
					nlassert(0);
				break;
			}
			return 0;
		}
		inline uint getColorOperand(uint index)
		{
			switch(index)
			{
				case 0: return Env.OpArg0RGB;
				case 1: return Env.OpArg1RGB;
				case 2: return Env.OpArg2RGB;
				default:
					nlassert(0);
				break;
			}
			return 0;
		}
		inline uint getAlphaOperand(uint index)
		{
			switch(index)
			{
				case 0: return Env.OpArg0Alpha;
				case 1: return Env.OpArg1Alpha;
				case 2: return Env.OpArg2Alpha;
				default:
					nlassert(0);
				break;
			}
			return 0;
		}

	};

private:

	TShader					_ShaderType;
	uint32					_Flags;
	TBlend					_SrcBlend,_DstBlend;
	ZFunc					_ZFunction;
	float					_ZBias;
	CRGBA					_Color;
	CRGBA					_Emissive, _Ambient, _Diffuse, _Specular;
	float					_Shininess;
	float					_AlphaTestThreshold;
	uint32					_Touched;
	bool					_StainedGlassWindow;
	// For each texture (8), the TexGen Mode.
	uint16					_TexCoordGenMode;
	struct	CUserTexMat
	{
		NLMISC::CMatrix		TexMat[IDRV_MAT_MAXTEXTURES];
	};
	std::auto_ptr<CUserTexMat>	_TexUserMat;		 // user texture matrix

public:
	// Private. For Driver only.
	CSmartPtr<ITexture>		_Textures[IDRV_MAT_MAXTEXTURES];
	uint8				    _TexAddrMode[IDRV_MAT_MAXTEXTURES]; // texture addressing enum packed as bytes
	CTexEnv					_TexEnvs[IDRV_MAT_MAXTEXTURES];
	CRefPtr<IMaterialDrvInfos>	_MatDrvInfo;

	// Private. For Driver only. LightMaps.
	struct	CLightMap
	{
		CSmartPtr<ITexture>		Texture;
		CRGBA					Factor;
		// Lightmap compression Factors (typically for 8 bits lightmaps)
		CRGBA					LMCAmbient;
		CRGBA					LMCDiffuse;
		CLightMap()
		{
			Factor.set(255, 255, 255, 255);
			LMCAmbient.set(0,0,0,0);
			LMCDiffuse.set(255, 255, 255, 255);
		}

		void	serial(NLMISC::IStream &f); // Deprecated...
		void	serial2(NLMISC::IStream &f); // New with version number
	};
	typedef	std::vector<CLightMap>	TTexturePtrs;
	TTexturePtrs			_LightMaps;
	bool					_LightMapsMulx2;


	uint32					getFlags() const {return _Flags;}
	uint32					getTouched(void)  const { return(_Touched); }
	void					clearTouched(uint32 flag) { _Touched&=~flag; }



};

} // NL3D

#include "driver_material_inline.h"

#endif // NL_MATERIAL_H

/* End of material.h */
