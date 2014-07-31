/**
 * \file render_target_manager.cpp
 * \brief CRenderTargetManager
 * \date 2014-07-30 21:30GMT
 * \author Jan Boon (Kaetemi)
 * CRenderTargetManager
 */

/* 
 * Copyright (C) 2014  by authors
 * 
 * This file is part of NL3D.
 * NL3D is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * NL3D is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with NL3D.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include <nel/3d/render_target_manager.h>

// STL includes
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/material.h>
#include <nel/3d/texture_bloom.h>
#include <nel/3d/texture_user.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/u_texture.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NL3D {

struct CRenderTargetDescInt
{
public:
	uint Width;
	uint Height;
	bool Mode2D;
	NL3D::CTextureUser *TextureUser;
	NLMISC::CSmartPtr<NL3D::ITexture> TextureInterface;
	bool InUse;
	bool Used;
};

CRenderTargetManager::CRenderTargetManager() : m_Driver(NULL)
{
	
}

CRenderTargetManager::~CRenderTargetManager()
{
	// Call twice to reset counters and cleanup
	cleanup();
	cleanup();
}

NL3D::CTextureUser *CRenderTargetManager::getRenderTarget(uint width, uint height, bool mode2D)
{
	// Find or create a render target, short loop so no real optimization
	for (std::vector<CRenderTargetDescInt *>::iterator it(m_RenderTargets.begin()), end(m_RenderTargets.end()); it != end; ++it)
	{
		CRenderTargetDescInt *desc = *it;
		if (!desc->InUse && desc->Width == width && desc->Height == height && desc->Mode2D == mode2D)
		{
			desc->InUse = true;
			desc->Used = true;
			return desc->TextureUser;
		}
	}

	nldebug("3D: Create new render target (%u x %u)", width, height);
	NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(m_Driver))->getDriver();
	CRenderTargetDescInt *desc = new CRenderTargetDescInt();
	CTextureBloom *tex = new CTextureBloom(); // LOL
	tex->mode2D(mode2D);
	desc->TextureInterface = tex;
	desc->TextureInterface->setRenderTarget(true);
	desc->TextureInterface->setReleasable(false);
	desc->TextureInterface->resize(width, height);
	desc->TextureInterface->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	desc->TextureInterface->setWrapS(ITexture::Clamp);
	desc->TextureInterface->setWrapT(ITexture::Clamp);
	drvInternal->setupTexture(*desc->TextureInterface);
	desc->TextureUser = new CTextureUser(desc->TextureInterface);
	nlassert(!drvInternal->isTextureRectangle(desc->TextureInterface)); // Not allowed, we only support NPOT for render targets now.
	desc->Width = width;
	desc->Height = height;
	desc->Mode2D = mode2D;
	desc->Used = true;
	desc->InUse = true;
	m_RenderTargets.push_back(desc);
	return desc->TextureUser;
}

void CRenderTargetManager::recycleRenderTarget(NL3D::CTextureUser *renderTarget)
{
	for (std::vector<CRenderTargetDescInt *>::iterator it(m_RenderTargets.begin()), end(m_RenderTargets.end()); it != end; ++it)
	{
		CRenderTargetDescInt *desc = *it;
		if (desc->TextureUser == renderTarget)
		{
			desc->InUse = false;
			return;
		}
	}
	nlerror("3D: Render target not found");
}

void CRenderTargetManager::cleanup()
{
	for (sint i = 0; i < (sint)m_RenderTargets.size(); ++i)
	{
		CRenderTargetDescInt *desc = m_RenderTargets[i];
		nlassert(!desc->InUse); // Assert for debugging, to not allow textures being carried over between frames. Optional assert
		if (!desc->InUse)
		{
			if (!desc->Used)
			{
				// No longer in use
				nldebug("3D: Release render target (%u x %u)", desc->Width, desc->Height);
				delete desc->TextureUser;
				desc->TextureUser = NULL;
				desc->TextureInterface = NULL; // CSmartPtr
				m_RenderTargets.erase(m_RenderTargets.begin() + i);
				--i;
			}
			else
			{
				// Flag for next round
				desc->Used = false;
			}
		}
	}
}

} /* namespace NL3D */

/* end of file */
