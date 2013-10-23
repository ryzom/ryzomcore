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

#ifndef NL_TEXTURE_CUBE_H
#define NL_TEXTURE_CUBE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture.h"


namespace NL3D
{


// ****************************************************************************
/**
 * CTextureCube
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2000
 */
class CTextureCube : public ITexture
{
public:

	enum TFace { positive_x=0, negative_x, positive_y, negative_y, positive_z, negative_z };

public:

	/**
	 * Default constructor
	 */
	CTextureCube();


	/**
	 * Accessors
	 */
	void setTexture(TFace f, ITexture *t);
	ITexture* getTexture(TFace f) { return _Textures[f]; }


	/**
	 * Set the name of the file containing the texture
	 * \param name of the file
	 */
	//void setFileName(std::string s);


	/**
	 * sharing system.
	 */
	virtual bool			supportSharing() const {return true;}
	virtual std::string		getShareName() const;


	/**
	 * Generate the texture, looking in CPath if necessary.
	 */
	void doGenerate(bool async = false);

	virtual void release();
	/// Does this texture is a cube texture
	virtual bool isTextureCube() const { return true; }

	/// Save the texture file name.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CTextureCube);

	/// If the face support multiple texture (such has CTextureMultiFile), this allow to select the active set
	virtual void selectTexture(uint index);
	// from ITexture
	virtual bool isSelectable() const;
	// from ITexture
	virtual ITexture		*buildNonSelectableVersion(uint index);


private:
	NLMISC::CSmartPtr<ITexture> _Textures[6];
};


} // NL3D


#endif // NL_TEXTURE_CUBE_H

/* End of texture_cube.h */
