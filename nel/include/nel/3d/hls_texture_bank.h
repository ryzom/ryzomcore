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

#ifndef NL_HLS_TEXTURE_BANK_H
#define NL_HLS_TEXTURE_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/3d/hls_color_texture.h"


namespace NLMISC
{
	class	CBitmap;
	class	IStream;
}


namespace NL3D
{

// ***************************************************************************
/**
 * A bank of HLS colorisable textures
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CHLSTextureBank
{
public:
	class CTextureInstance
	{
	private:
		friend	class	CHLSTextureBank;
		// Point to _TextureInstanceData. In this array, First come the name (0-ended), then the CHLSColorDelta.
		uint32				_DataIndex;
		// The ref color texture
		uint32				_ColorTextureId;
		// compiled/loaded direct ptr to the data
		uint8				*_DataPtr;
		// compiled/loaded direct ptr to the color texture
		CHLSColorTexture	*_ColorTexturePtr;

	public:
		void		serial(NLMISC::IStream &f);

		// let _DataPtr to point on ptr.
		void		buildAsKey(const char *ptr)
		{
			_DataPtr= (uint8*)ptr;
		}

		const char	*getName() const {return (const char*)_DataPtr;}

		// used for sort()
		bool		operator<(const CTextureInstance &t) const;
		// used for searchLowerBound()
		bool		operator<=(const CTextureInstance &t) const;
		// return true if the instance has the same name has str.
		bool		sameName(const char *str);


		/// Build a colored version of this texture
		void		buildColorVersion(NLMISC::CBitmap &out);
	};

	class CTextureInstanceHandle
	{
	public:
		CTextureInstance	*Texture;

		// used for sort()
		bool		operator<(const CTextureInstanceHandle &t) const
		{
			// Compare the texture.
			return *Texture<*t.Texture;
		}
		// used for searchLowerBound()
		bool		operator<=(const CTextureInstanceHandle &t) const
		{
			// Compare the texture.
			return *Texture<=*t.Texture;
		}
	};

public:
	/// Constructor
	CHLSTextureBank();

	/// \name Build
	// @{
	/// clear the bank
	void			reset();

	/// Add a colorisable texture to the bank
	uint32			addColorTexture(const CHLSColorTexture &tex);

	/** Add an instance texture (ie colorised) to the bank
	 *	\param name name of the colored texture. NB: it is lowered in this method
	 *	\param colorTextureId index returned by addColorTexture()
     *	\param cols must be same size of number of mask of the colorTextureId pointed (nlassert)
	 */
	void			addTextureInstance(const std::string &name, uint32 colorTextureId, const std::vector<CHLSColorDelta> &cols);

	/// compile the bank.
	void			compile();

	/// serial. if loading, ptrs are correclty setuped => no compile() needed
	void			serial(NLMISC::IStream &f);
	// @}


	/// \name Usage
	// @{

	/// Append handle into the array (build of the manager)
	void			fillHandleArray(std::vector<CTextureInstanceHandle> &array);

	// @}


// ***************
private:
	/// Array of colorisable texture.
	std::vector<CHLSColorTexture>	_ColorTextures;

	/// Raw Array of Name+HLS color delta.
	std::vector<uint8>				_TextureInstanceData;

	/// Array of textureInstances
	std::vector<CTextureInstance>	_TextureInstances;

private:
	void			compilePtrs();
};


} // NL3D


#endif // NL_HLS_TEXTURE_BANK_H

/* End of hls_texture_bank.h */
