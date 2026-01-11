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

#ifndef NL_TEXTURE_MULTI_FILE_H
#define NL_TEXTURE_MULTI_FILE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture.h"

#include <string>


namespace NL3D {


/**
 * This kind of texture is like a texture file except that it can encode several texture at once.
 * Only ONE texture is active at a given time.
 * The texture being used is chosen by a call to selectTexture.
 * NB : This is not derived from CTextureMulti because we don't store a pointer on each texture, just the name.
 * Moreover the needed method are exposed by CTexture
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CTextureMultiFile : public ITexture
{
public:

	CTextureMultiFile(uint numTexs = 0);

	/// set the number of textures that are encoded in that texture
	void setNumTextures(uint numTexs);
	/**
	 * Set the name of the file containing the i-th texture
	 * \param name of the file
	 * \param index index of the texture
	 */
	void					setFileName(uint index, const char *);
	//
	uint					getNumFileName() const { return (uint)_FileNames.size(); }
	/**
	 * get the name of the file containing the texture for the given index
	 * \return name of the file
	 */
	const std::string		&getFileName(uint index) const { return _FileNames[index]; }



	virtual bool			supportSharing() const { return true; }
	virtual std::string		getShareName() const;
	virtual void			selectTexture(uint index);
	virtual bool			isSelectable() const { return true; }
	virtual ITexture		*buildNonSelectableVersion(uint index);


	/// Generate the current selected texture, looking in CPath if necessary.
	virtual void			doGenerate(bool async = false);
	/// Serial this object
	virtual void			serial(NLMISC::IStream &f);
	NLMISC_DECLARE_CLASS(CTextureMultiFile);

private:
	uint32					 _CurrSelectedTexture;
	std::vector<std::string> _FileNames;
private:
	sint					getTexIndex(uint index) const;
	const std::string		&getTexNameByIndex(uint index) const;
};


} // NL3D


#endif // NL_TEXTURE_MULTI_FILE_H

/* End of texture_multi_file.h */
