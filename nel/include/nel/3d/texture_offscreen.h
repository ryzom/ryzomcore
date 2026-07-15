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

#ifndef NL_TEXTURE_OFFSCREEN_H
#define NL_TEXTURE_OFFSCREEN_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture_blank.h"


namespace NL3D {


/**
 * Offscreen render target texture. Renders directly into the texture
 * via FBO (GL) or render target surface (D3D), rather than copying
 * from the framebuffer.
 */
class CTextureOffscreen : public CTextureBlank
{
public:
NLMISC_DECLARE_CLASS(CTextureOffscreen);

	CTextureOffscreen() { m_NeedsDepthStencil = true; }
	virtual ~CTextureOffscreen() {}

	virtual bool isOffscreenTexture() const { return true; }

	void setNeedsDepthStencil(bool b) { m_NeedsDepthStencil = b; }
	bool needsDepthStencil() const { return m_NeedsDepthStencil; }

private:

	bool	m_NeedsDepthStencil;
};


} // NL3D


#endif // NL_TEXTURE_OFFSCREEN_H

/* End of texture_offscreen.h */
