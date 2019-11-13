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

#ifndef NL_TEXTURE_GROUPED_H
#define NL_TEXTURE_GROUPED_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture.h"
#include "nel/misc/uv.h"



namespace NL3D {


using NLMISC::CSmartPtr;
using NLMISC::CUV;

/**
 * This kind texture is used for grouping several other textures. Each texture must have the same size.
 * The textures are copied into one single surface, so  animation can be performed only by UV shifting (if there's no wrapping).
 * This is useful when objects sorting (by texture) is too complex or cost too much time (particles for examples...)
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CTextureGrouped : public ITexture
{
public:

	/// default ctor. by default, there are no texture present
	CTextureGrouped();

	/// copy ctor
	CTextureGrouped(const CTextureGrouped &src);


	/// = operator
	CTextureGrouped &operator=(const CTextureGrouped &src);

	/** Check if all the textures in a tab have the same size and the same pixel format
	 *  \param textureTab : pointer to a texture list*
	 *  \param nbTex the number of textures in the list (>0)
	 *  \see setTextures()
	 */
	bool areValid(CSmartPtr<ITexture> *textureTab, uint nbTex);

	/** This set the textures to be used. They all must have the same size.
	 *  An assertion is thrown otherwise.
	 *  WARNING : if you touch one of the textures in the tab later, you may need to touch this one if it changed
	 *  \param checkValid check that textures are valid
	 *  \param textureTab : pointer to a texture list
	 *  \param nbTex the number of textures in the list (>0)
	 *  \see haveValidSizes()
	 */
	void setTextures(CSmartPtr<ITexture> *textureTab, uint nbTex, bool checkValid = true);


	/// Retrieve the number of textures grouped in this one
	uint32 getNbTextures(void) const { return _NbTex; }


	/** Retrieve pointers to the textures.
	 *  \param textureTab a tab containing enough space for the pointers
	 *  \see getNbTextures()
	 */

	void getTextures(CSmartPtr<ITexture> *textureTab) const;

	// get a texture in the list
	CSmartPtr<ITexture> getTexture(uint32 index) { return _Textures[index]; }

	/** Get the U-delta and V delta in the groupedTexture for one unit texture (they all have the same size).
	 *  return (0, 0) if no textures have been set
	 */
	const CUV &getUVDelta(void) const
	{
		return _DeltaUV;
	}

	/// Get the origin UV for one texture. Its index is the same than in the tab that was sent to setTextures()
	const CUV &getUV(uint32 index) const
	{
		return _TexUVs[index].uv0;
	}


	/**
	 * sharing system.
	 */
	virtual bool			supportSharing() const;
	virtual std::string		getShareName() const;


	/**
	 * Generate the texture.
	 */
	void doGenerate(bool async = false);

	/// serialization
	virtual void	serial(NLMISC::IStream &f);


	/// a group of 4 uvs
	struct TFourUV
	{
		CUV uv0, uv1, uv2, uv3;
	};

	// a list of uv's
	typedef std::vector< TFourUV > TFourUVList;

	// Get a tab of 4 UVs for a texture in the group : 0 = top-left, 1 = top-right, 2 = bottom-right, 3 = bottom-left
	const TFourUV &getUVQuad(uint texIndex)
	{
		if (texIndex < _NbTex)
		{
			return _TexUVs[texIndex];
		}
		else
		{
			if (sint(texIndex) > 0)
			{
				return _TexUVs[texIndex % _NbTex];
			}
			else
			{
				return _TexUVs[_NbTex - 1 - (~texIndex % _NbTex)];
			}
		}
	}


	virtual void release();


	NLMISC_DECLARE_CLASS(CTextureGrouped);

protected:
	uint32 _NbTex; // for caching

	/// pointers to the original textures
	typedef std::vector< CSmartPtr<ITexture> > TTexList;
	TTexList _Textures;

	/// uv delta for one texture in the group
	CUV _DeltaUV;

	/// the UVs for each texture in the group
	TFourUVList _TexUVs;


	// cache sub bitmap size for each texture to avoid reloading of texture after each serial
	static std::map<std::string, uint> _NameToSize;

	// Copy this class attributes from src; Used by the = operator and the copy ctor
	void duplicate(const CTextureGrouped &src);

	// make textures as a group of dummy. This is used when textures formet are incompatible
	void makeDummies(CSmartPtr<ITexture> *textureTab, uint nbTex);

	// display a warning to tell that a set of grouped textures are incompatibles
	void displayIncompatibleTextureWarning(CSmartPtr<ITexture> *textureTab, uint nbTex);

	// generate uvs for each sub-texture
	void forceGenUVs();
	void genUVs(uint subBitmapHeight);
};



} // NL3D


#endif // NL_TEXTURE_GROUPED_H

/* End of texture_grouped.h */
