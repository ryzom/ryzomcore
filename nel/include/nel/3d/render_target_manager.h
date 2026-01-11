/**
 * \file render_target_manager.h
 * \brief CRenderTargetManager
 * \date 2014-07-30 21:30GMT
 * \author Jan Boon (Kaetemi)
 * CRenderTargetManager
 */


// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL3D_RENDER_TARGET_MANAGER_H
#define NL3D_RENDER_TARGET_MANAGER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/misc/geom_ext.h>

// Project includes
#include <nel/3d/u_texture.h>

namespace NL3D {

class UDriver;
class ITexture;
class CTextureUser;
class CDriverUser;

struct CRenderTargetDescInt;

/**
 * \brief CRenderTargetManager
 * \date 2013-07-03 20:17GMT
 * \author Jan Boon (Kaetemi)
 * CRenderTargetManager
 * Usage: Call 'getRenderTarget' when you start using a render target,
 * call 'recycledRenderTarget' when the render target can be recycled.
 * At end of frame call cleanup.
 * Assumes semi-constant render target quantity between frames, 
 * except on changes of resolution or feature settings.
 */
class CRenderTargetManager
{
public:
	CRenderTargetManager();
	~CRenderTargetManager();

	NL3D::CTextureUser *getRenderTarget(uint width, uint height, bool mode2D = false, UTexture::TUploadFormat format = UTexture::Auto);
	void recycleRenderTarget(NL3D::CTextureUser *renderTarget);

	void cleanup();

private:
	friend class CDriverUser;
	NL3D::UDriver *m_Driver;
	std::vector<CRenderTargetDescInt *> m_RenderTargets;

}; /* class CRenderTargetManager */

} /* namespace NL3D */

#endif /* #ifndef NL3D_RENDER_TARGET_MANAGER_H */

/* end of file */
