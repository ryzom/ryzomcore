/**
 * \file neltexture.cpp
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
	purpose:	Implementation for main Nevrax Engine GUI texture class

	For use with GUI Library:
	Crazy Eddie's GUI System (http://crayzedsgui.sourceforge.net)
    Copyright (C)2004 Paul D Turner (crayzed@users.sourceforge.net)

	This file contains code that is specific to NeL (http://www.nevrax.org)
*************************************************************************/

// NeL Renderer includes
#include <nel/cegui/neltexture.h>
#include <nel/cegui/nelrenderer.h>

// CEGUI includes
#include "CEGUIExceptions.h"

// ditch min/max, they mess stuff up.
#undef max
#undef min

//nel includes
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>
#include <nel/3d/texture_mem.h> 
#include <nel/3d/material.h>

namespace CEGUI
{
	NeLTexture::NeLTexture(Renderer *owner) : Texture(owner)
	{
		m_Owner=dynamic_cast<NeLRenderer *>(owner);
		m_Material = m_Owner->getNeLDriver().createMaterial();
		m_TextureFile=NULL;
		m_TextureMem=NULL;
		m_UsingFile=false;
		m_UsingMem=false;
	}

	NeLTexture::~NeLTexture(void)
	{
		freeNeLTexture();
		// can't free material in the same phase as the texture.
		m_Owner->getNeLDriver().deleteMaterial(m_Material);
		m_Material=NULL;

	}

	void NeLTexture::loadFromFile(const String &filename, const String& resourceGroup)
	{
		String file = NLMISC::CPath::lookup(filename.c_str()).c_str();

		// this object can only contain one texture at a time, free the old one if it exists.
		freeNeLTexture();

		// create the texture from a file...

		/* TODO: Determine if Kervala was correct in removing setWrapS/T */
		m_TextureFile=m_Owner->getNeLDriver().createTextureFile(file.c_str());
		m_TextureFile->setWrapS(NL3D::UTexture::Clamp);
		m_TextureFile->setWrapT(NL3D::UTexture::Clamp);

		if(m_TextureFile == 0) { // failed to load the texture.
			nlinfo("Failed to load texture: %s",filename.c_str());
			return;
		}

		// because nel unloads the texture from RAM when it loads it into VRAM
		// we need to load the texture into a bitmap to get the sizes.
		NL3D::CBitmap tmpBitmap;
		NLMISC::CIFile nelfile(file.c_str());
		tmpBitmap.load(nelfile);
		d_width=tmpBitmap.getWidth();
		d_height=tmpBitmap.getHeight();

		// set the material up.
		m_UsingFile=true; m_UsingMem=false;
		m_Material.setTexture(m_TextureFile);
		m_Material.setBlend(true);
		m_Material.setBlendFunc(NL3D::UMaterial::srcalpha, NL3D::UMaterial::invsrcalpha);
		m_Material.setAlphaTest(false);
		m_Material.setZFunc(NL3D::UMaterial::always);
		m_Material.setDoubleSided();
	}

	void NeLTexture::loadFromMemory(const void *buffPtr, uint buffWidth, uint buffHeight, PixelFormat pixelFormat)
	{
		// this object can only contain one texture at a time, free the old one if it exists.
		freeNeLTexture();

		/**
		 * This debugging is handy when you're not sure if the memory buffer
		 * being sent to you from CEGUI is valid. Uncomment it and it'll create
		 * a file called loadfrommem###.tga in your working directory.
		 *
		 * NLMISC::CBitmap btm;
		 * btm.reset();
		 * btm.resize(buffWidth,buffHeight,NLMISC::CBitmap::RGBA);
		 * uint8 *dest=&(btm.getPixels()[0]);
		 * memcpy(dest,buffPtr,buffWidth*buffHeight*4);
		 * std::string filename = NLMISC::CFile::findNewFile("loadfrommem.tga");
		 * NLMISC::COFile fs(filename);
		 * btm.writeTGA(fs);
		 */

		int size=4;
               
		switch(pixelFormat)
		{
			case PF_RGB:
				size = 3;
				break;
			case PF_RGBA:
				size = 4;
				break;
		}

		// copy the memory stream for use in the NeL texture.
		uint8 *pTmpBuf=new uint8[buffWidth*buffHeight*size];
		memcpy(pTmpBuf,buffPtr,buffWidth*buffHeight*size);

		// create the texture
		m_TextureMem=new NL3D::CTextureMem( pTmpBuf,buffWidth*buffHeight*size,true,false,buffWidth,buffHeight,NLMISC::CBitmap::RGBA);
		m_TextureMem->setWrapS(NL3D::ITexture::Clamp);
		m_TextureMem->setWrapT(NL3D::ITexture::Clamp);
		m_TextureMem->setFilterMode(NL3D::ITexture::Linear, NL3D::ITexture::LinearMipMapOff);
		m_TextureMem->setReleasable(false);
		m_TextureMem->generate();
		if(m_TextureMem == 0) { // failed to load the texture.
			nlinfo("Failed to load texture from memory");
			return;
		}

		/**
		 * Configure the material. This is a little more complicated than loading a texture from
		 * a file, since CTextureFileUser and CTextureFile take care of prepping all of the
		 * necessary alpha settings.
		 */
		NL3D::CMaterial *mat = m_Material.getObjectPtr();
		mat->initUnlit();
		mat->setShader(NL3D::CMaterial::Normal);
		mat->setTexture(0, m_TextureMem);
               
		/**
		 * We still use alpha testing to cull out pixels to speed up
		 * blending and multitexturing.
		 */
		m_Material.setAlphaTest(true);
		m_Material.setAlphaTestThreshold(0.1f);
		m_Material.setBlend(true);
		m_Material.setBlendFunc(NL3D::UMaterial::srcalpha,NL3D::UMaterial::invsrcalpha);
		m_Material.texEnvOpRGB(0, NL3D::UMaterial::Modulate);
		m_Material.texEnvArg0RGB(0, NL3D::UMaterial::Texture, NL3D::UMaterial::SrcColor);
		m_Material.texEnvArg1RGB(0, NL3D::UMaterial::Diffuse, NL3D::UMaterial::SrcColor);
		m_Material.texEnvOpAlpha(0, NL3D::UMaterial::Modulate);
		m_Material.texEnvArg0Alpha(0, NL3D::UMaterial::Texture, NL3D::UMaterial::SrcAlpha);
		m_Material.texEnvArg1Alpha(0, NL3D::UMaterial::Diffuse, NL3D::UMaterial::SrcAlpha);
		m_Material.setZFunc(NL3D::UMaterial::always);
		m_Material.setDoubleSided(true);

		// make sure to record our changes.
		d_width=buffWidth;
		d_height=buffHeight;
		m_UsingFile=false; m_UsingMem=true;
	}

	NL3D::UMaterial NeLTexture::getNeLTexture(void) {
		return m_Material;
	}

	void NeLTexture::freeNeLTexture(void)
	{
		// never been populated.
		if(m_TextureMem==NULL && m_TextureFile==NULL)
			return;

		if(m_UsingFile==true) {
			m_Owner->getNeLDriver().deleteTextureFile(m_TextureFile);
			m_TextureFile=NULL;
			m_UsingFile=false;
		} else if(m_UsingMem==true) {
			//m_Owner->getNeLDriver().deleteTextureMem(m_TextureMem);
			m_TextureMem=NULL;
			m_UsingMem=false;
		} else {
			nlwarning("Something has gone horribly wrong, unable to free any type of texture.");
		}
	}

} // end namespace CEGUI
