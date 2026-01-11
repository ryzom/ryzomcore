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

#ifndef NL_TEXTURE_BLOOM_H
#define NL_TEXTURE_BLOOM_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture_blank.h"


namespace NL3D {


/**
 * This texture does generate nothing, and it is releasable. It is used for bloom effect.
 */
class CTextureBloom : public CTextureBlank
{
public:
NLMISC_DECLARE_CLASS(CTextureBloom);

	CTextureBloom() {Mode2D=false;}
	virtual ~CTextureBloom() {}

	virtual bool isBloomTexture() const { return true; }

	void mode2D(bool isMode2D) {Mode2D=isMode2D;}
	bool isMode2D() {return Mode2D;}

private:

	bool	Mode2D;
};


} // NL3D


#endif // NL_TEXTURE_BLOOM_H

/* End of texture_bloom.h */
