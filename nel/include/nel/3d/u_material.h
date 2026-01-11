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

#ifndef NL_U_MATERIAL_H
#define NL_U_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"


namespace NL3D
{

using NLMISC::CRGBA;


class	UTexture;
class   UDriver;


// ***************************************************************************
/**
 * Game Interface for Material. Material for gamers are Unlighted materials!! Only normal material unlighted is supported.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UMaterial
{
public:
	enum ZFunc				{ always=0,never,equal,notequal,less,lessequal,greater,greaterequal, zfuncCount };
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor, blendCount };

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
	 * Multiply-Add (Mad) out= arg0 * arg1 + arg2. Must be supported by driver
	 * EMBM : apply to both color and alpha : the current texture, whose format is DSDT, is used to offset the texture in the next stage.
	 *  NB : for EMBM and InterpolateConstant, this must be supported by driver.
	 */
	enum TTexOperator		{ Replace=0, Modulate, Add, AddSigned,
							  InterpolateTexture, InterpolatePrevious, InterpolateDiffuse, InterpolateConstant, EMBM, Mad };

	/** Source argument.
	 * Texture:		the arg is taken from the current texture of the stage.
	 * Previous:	the arg is taken from the previous enabled stage. If stage 0, Previous==Diffuse.
	 * Diffuse:		the arg is taken from the primary color vertex.
	 * Constant:	the arg is taken from the constant color setuped for this texture stage.
	 */
	enum TTexSource			{ Texture=0, Previous, Diffuse, Constant };

	/** Operand for the argument.
	 * For Alpha arguments, only SrcAlpha and InvSrcAlpha are Valid!! \n
	 * SrcColor:	arg= ColorSource.
	 * InvSrcColor:	arg= 1-ColorSource.
	 * SrcAlpha:	arg= AlphaSource.
	 * InvSrcAlpha:	arg= 1-AlphaSource.
	 */
	enum TTexOperand		{ SrcColor=0, InvSrcColor, SrcAlpha, InvSrcAlpha };
	// @}

public:
	/// \name Texture.
	// @{
	// Set a texture in a stage
	void 			setTexture(uint stage, UTexture* ptex);
	// Is a texture present in the stage ?
	bool			texturePresent (uint stage);
	/// select from a texture set for this material (if available)
	void			selectTextureSet(uint id);

	// Deprecated use setTexture(uint stage, UTexture* ptex)
	void 			setTexture(UTexture* ptex);
	// Deprecated use texturePresent (uint stage)
	bool			texturePresent();
	// @}


	/// \name Blending.
	// @{
	void			setBlend(bool active);
	void			setBlendFunc(TBlend src, TBlend dst);
	void			setSrcBlend(TBlend val);
	void			setDstBlend(TBlend val);

	bool			getBlend() const;
	TBlend			getSrcBlend(void)  const;
	TBlend			getDstBlend(void)  const;
	// @}

	/// \name Texture environnement.
	// @{
	void			texEnvOpRGB(uint stage, TTexOperator ope);
	void			texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper);
	void			texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper);
	void			texEnvArg2RGB(uint stage, TTexSource src, TTexOperand oper);
	void			texEnvOpAlpha(uint stage, TTexOperator ope);
	void			texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper);
	void			texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper);
	void			texEnvArg2Alpha(uint stage, TTexSource src, TTexOperand oper);
	// @}

	/// \name ZBuffer.
	// @{
	void			setZFunc(ZFunc val);
	void			setZWrite(bool active);
	void			setZBias(float val);

	ZFunc			getZFunc(void)  const ;
	bool			getZWrite(void)  const;
	float			getZBias(void)  const;
	// @}

	/// \name Alpha test.
	// @{
	void			setAlphaTest(bool active);
	bool			getAlphaTest() const;

	/** change the threshold against alpha is tested. Default is 0.5f.
	 *	\param thre threshold, should be in [0..1], not clamped.
	 */
	void			setAlphaTestThreshold(float threshold);
	float			getAlphaTestThreshold() const;
	// @}

	/// \name Color/Lighting..
	// @{
	/// The Color is used only if lighting is disabled. Also, color is replaced by per vertex color (if any).
	void			setColor(CRGBA rgba);

	CRGBA			getColor(void) const;


	// @}


	/// \name Lighted material mgt. Relevant only if isLighted(). Used for get of UShape
	// @{
	bool				isLighted() const;
	CRGBA				getEmissive() const;
	CRGBA				getAmbient() const;
	/// return diffuse part. NB: A==opacity.
	CRGBA				getDiffuse() const;
	uint8				getOpacity() const;
	CRGBA				getSpecular() const;
	float				getShininess() const;
	// @}


	/// \name Culling
	// @{
	void			setDoubleSided(bool doubleSided = true);
	bool			getDoubleSided() const;
	// @}


	/// \name Misc
	// @{
	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void			initUnlit();

	// test if the given driver will support rendering of that material
	bool			isSupportedByDriver(UDriver &drv, bool forceBaseCaps = false);
	// @}

	/// Proxy interface

	/// Constructors
	UMaterial() { _Object = NULL; }
	UMaterial(class CMaterial *object) { _Object = object; };
	/// Attach an object to this proxy
	void			attach(class CMaterial *object) { _Object = object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CMaterial	*getObjectPtr() const {return (CMaterial*)_Object;}
private:
	CMaterial	*_Object;
};


} // NL3D


#endif // NL_U_MATERIAL_H

/* End of u_material.h */
