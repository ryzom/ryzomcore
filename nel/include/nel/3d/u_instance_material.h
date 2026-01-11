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

#ifndef NL_U_INSTANCE_MATERIAL_H
#define NL_U_INSTANCE_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"
#include "nel/misc/bitmap.h"


namespace NL3D
{


using NLMISC::CRGBA;


// ***************************************************************************
/**
 * Base interface for manipulating Material retrieved from UInstance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstanceMaterial
{
public:
	enum ZFunc				{ always=0,never,equal,notequal,less,lessequal,greater,greaterequal, zfuncCount };
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor, blendCount };

	// This enums MUST be the same than in ITexture!!
	enum	TWrapMode
	{
		Repeat= 0,
		Clamp,
		WrapModeCount
	};
public:

	/// \name Modes.
	// @{
	bool				isLighted() const;
	void					setLighting(bool active,
										CRGBA emissive=CRGBA(0,0,0),
										CRGBA ambient=CRGBA(0,0,0),
										CRGBA diffuse=CRGBA(0,0,0),
										CRGBA specular=CRGBA(0,0,0),
										float shininess= 10);
	bool				isUserColor() const;
	// @}

	/// \name Blending.
	// @{
	void			setBlend(bool active);
	void			setBlendFunc(TBlend src, TBlend dst);
	void			setSrcBlend(TBlend val);
	void			setDstBlend(TBlend val);
	void			setAlphaTestThreshold(float at);
	void			setZWrite(bool active);
	void			setZFunc(ZFunc val);
	float			getAlphaTestThreshold() const;
	void			setAlphaTest(bool active);

	bool			getBlend() const;
	TBlend			getSrcBlend(void)  const;
	TBlend			getDstBlend(void)  const;
	// @}


	/// \name Lighted material mgt. Has effect only if isLighted().
	// @{

	/// Set the emissive part ot material. Useful only if isLighted()
	void				setEmissive( CRGBA emissive=CRGBA(0,0,0) );
	/// Set the Ambient part ot material. Useful only if isLighted()
	void				setAmbient( CRGBA ambient=CRGBA(0,0,0) );
	/// Set the Diffuse part ot material. Useful only if isLighted()
	void				setDiffuse( CRGBA diffuse=CRGBA(0,0,0) );
	/// Set the Opacity part ot material. Useful only if isLighted()
	void				setOpacity( uint8	opa );
	/// Set the specular part ot material. Useful only if isLighted()
	void				setSpecular( CRGBA specular=CRGBA(0,0,0) );
	/// Set the shininess part ot material. Useful only if isLighted()
	void				setShininess( float shininess );

	CRGBA				getEmissive() const;
	CRGBA				getAmbient() const;
	/// return diffuse part. NB: A==opacity.
	CRGBA				getDiffuse() const;
	uint8				getOpacity() const;
	CRGBA				getSpecular() const;
	float				getShininess() const;

	// @}


	/// \name UnLighted material mgt. Has effect only if !isLighted().
	// @{
	void				setColor(CRGBA rgba);
	CRGBA				getColor(void) const;
	// @}

	/// \name Per stage constant color
	// @{
	void				setConstantColor(uint stage, NLMISC::CRGBA color);
	NLMISC::CRGBA		getConstantColor(uint stage) const;
	// @}


	/// \name Texture UserColor. No effect if !isUserColor(). (getUserColor() return CRGBA(0,0,0,0))
	// @{
	void				setUserColor(CRGBA userColor);
	CRGBA				getUserColor() const;
	// @}

	/// \name Texture files specific
	// @{
	/// Get the last stage that got a texture. -1 means there is no textures.
	sint				getLastTextureStage() const;
	/// Check whether the texture of the n-th stage is a texture file
	bool				isTextureFile(uint stage) const;
	/// Get the fileName used by the n-th texture file. (must be a texture file or an assertion is raised)
	std::string			getTextureFileName(uint stage) const;
	// Empty the texture at the given stage
	void				emptyTexture(uint stage = 0);
	/** Set the fileName used by the n-th texture file. (must be a texture file or an assertion is raised)
	 *	NB: if success and if instanceOwner->getAsyncTextureMode()==true, then instanceOwner->setAsyncTextureDirty(true)
	 *	is called
	 */
	void				setTextureFileName(const std::string &fileName, uint stage = 0);
	/** Set the texture datas to be read from memory rather than from a file. This erases any previous texture at that slot.
	  * The provided block can be a memory image of a file, however
	  * \param stage The stage at which texture must be set
	  * \param data Pointer of the file, or of the pixels datas
	  * \param length length, in bytes, of the datas.
	  * \param isFile is true if the data must be interpreted as a texture file. Otherwise, it is interpreted
	  *        as the raw datas of the texture, so the format and size of the texture must also have been set to match
	  *        the raw datas
	  * \param _delete Is true if the texture has ownership on the texture datas
	  * \param texType relevant only when isFile is set to false. Gives the format to expand the texture to when it is generated.
	  */
	void				setTextureMem(uint stage, uint8 *data, uint32 length, bool _delete, bool isFile = true, uint width = 0, uint height = 0, NLMISC::CBitmap::TType texType = NLMISC::CBitmap::RGBA);

	// Set wrapping mode for a texture
	void				setWrapS(uint stage, TWrapMode mode);
	void				setWrapT(uint stage, TWrapMode mode);
	TWrapMode			getWrapS(uint stage) const;
	TWrapMode			getWrapT(uint stage) const;


	// set

	/// \name Texture matrix
	// @{
	// Enable a user texture matrix for the n-th stage. The initial matrix is set to identity.
	void                    enableUserTexMat(uint stage, bool enabled = true);
	// Test whether a user texture is enabled for the n-th stage
	bool                    isUserTexMatEnabled(uint stage) const;
	/// Set a new texture matrix for the given stage.
	void					setUserTexMat(uint stage, const NLMISC::CMatrix &m);
	/** Get a const ref. on the texture matrix of the n-th stage.
	  */
	const NLMISC::CMatrix  &getUserTexMat(uint stage) const;
	// @}

	/// Proxy interface

	/// Constructors
	UInstanceMaterial()
	{
		_Object = NULL;
	}
	UInstanceMaterial(class CMeshBaseInstance	*mbi, class CMaterial *object, class CAsyncTextureBlock *asyncTextureBlock)
	{
		_MBI = mbi;
		_Object = object;
		_AsyncTextureBlock = asyncTextureBlock;
	};
	/// Attach an object to this proxy
	void			attach(class CMeshBaseInstance	*mbi, class CMaterial *object, class CAsyncTextureBlock *asyncTextureBlock)
	{
		_MBI = mbi;
		_Object = object;
		_AsyncTextureBlock = asyncTextureBlock;
	}
	/// Detach the object
	void			detach()
	{
		_MBI = NULL;
		_Object = NULL;
		_AsyncTextureBlock = NULL;
	}
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CMaterial	*getObjectPtr() const {return (CMaterial*)_Object;}
private:
	CMeshBaseInstance	*_MBI;
	CMaterial			*_Object;
	CAsyncTextureBlock	*_AsyncTextureBlock;
};


} // NL3D


#endif // NL_U_INSTANCE_MATERIAL_H

/* End of u_instance_material.h */
