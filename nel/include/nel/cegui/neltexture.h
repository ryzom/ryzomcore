/**
 * \file neltexture.h
 * \date November 2004
 * \author Matt Raykowski
 * \author Henri Kuuste
 */

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


/************************************************************************
	purpose:	Interface for main Nevrax Engine GUI texture class

	For use with GUI Library:
	Crazy Eddie's GUI System (http://crayzedsgui.sourceforge.net)
    Copyright (C)2004 Paul D Turner (crayzed@users.sourceforge.net)

	This file contains code that is specific to NeL (http://www.nevrax.org)
*************************************************************************/

#ifndef __NELTEXTURE_H__
#define __NELTEXTURE_H__

// cegui includes
#include "CEGUIBase.h"
#include "CEGUIRenderer.h"
#include "CEGUITexture.h"

// nelrenderer
#include "nelrenderer.h"

// standard includes
#include <list>

// nel includes
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/common.h>
#include <nel/misc/events.h>
#include <nel/misc/fast_mem.h>
#include <nel/misc/config_file.h>
#include <nel/misc/system_info.h>
#include <nel/misc/mem_displayer.h>

#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_particle_system_instance.h>
#include <nel/3d/texture_mem.h>

namespace CEGUI
{
	class NeLTexture : public Texture
	{
	private:
		friend Texture *NeLRenderer::createTexture(void);
		friend Texture *NeLRenderer::createTexture(const String &filename, const String& resourceGroup);
		friend Texture *NeLRenderer::createTexture(float size);
		friend void NeLRenderer::destroyTexture(Texture *texture);

		NeLTexture(Renderer *owner);
		virtual ~NeLTexture(void);

	public:
		virtual ushort getWidth(void) const { return d_width; }
		virtual ushort getHeight(void) const { return d_height; }
		virtual void loadFromFile(const String &filename, const String& resourceGroup);

		/**
		 * \brief Creates a NeL Texture from a raw memory buffer.
		 *
		 * \param buffPtr The pointer to the buffer containing the texture.
		 * \param buffWidth The width of the texture in pixels.
		 * \param buffHeight The height of the texture in pixels.
		 * \param pixelFormat The type of pixel packing used.
		 * \todo Clean up CTextureMem creation and registration.
		 */
		virtual void loadFromMemory(const void *buffPtr, uint buffWidth, uint buffHeight, PixelFormat pixelFormat = CEGUI::Texture::PF_RGBA);
		NL3D::UMaterial getNeLTexture(void);
		void setNeLTextureSize(uint size);

	private:
		/**
		 * \brief Frees the texture from the NeL driver and deletes it.
		 * \todo Fix the deletion of CTextureMem textures.
		 */
		void freeNeLTexture(void);
		NL3D::UTextureFile	*m_TextureFile;
		NL3D::CTextureMem	*m_TextureMem;
		bool	m_UsingFile;
		bool	m_UsingMem;
		NL3D::UMaterial m_Material;

		NeLRenderer *m_Owner;
		ushort d_width;
		ushort d_height;
	};
} // end namespace CEGUI

#endif // __NELTEXTURE_H__

