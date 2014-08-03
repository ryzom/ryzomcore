/**
 * \file fxaa.cpp
 * \brief CFXAA
 * \date 2014-08-03 21:41GMT
 * \author Jan Boon (Kaetemi)
 * CFXAA
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
#include <nel/3d/fxaa.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/material.h>
#include <nel/3d/texture_bloom.h>
#include <nel/3d/texture_user.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/render_target_manager.h>

using namespace std;
// using namespace NLMISC;

namespace NL3D {

namespace {

#include "fxaa_program.h"

} /* anonymous namespace */

CFXAA::CFXAA(NL3D::UDriver *driver) : m_Driver(driver), m_PP(NULL), m_Width(~0), m_Height(~0)
{
	nldebug("3D: Create FXAA");

	CDriverUser *dru = static_cast<CDriverUser *>(driver);
	NL3D::IDriver *drv = (dru)->getDriver();

	if (drv->supportBloomEffect() && drv->supportNonPowerOfTwoTextures())
	{
		m_PP = new CPixelProgram();
		// arbfp1
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::arbfp1;
			source->setSourcePtr(a_arbfp1);
			m_PP->addSource(source);
		}
		// ps_2_0
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::ps_2_0;
			source->setSourcePtr(a_ps_2_0);
			m_PP->addSource(source);
		}
		if (!drv->compilePixelProgram(m_PP))
		{
			nlwarning("No supported pixel program for FXAA effect");

			delete m_PP;
			m_PP = NULL;
		}
	}

	if (m_PP)
	{
		m_Mat = m_Driver->createMaterial();
		m_Mat.initUnlit();
		m_Mat.setColor(CRGBA::White);
		m_Mat.setBlend (false);
		m_Mat.setAlphaTest (false);
		NL3D::CMaterial *mat = m_Mat.getObjectPtr();
		mat->setShader(NL3D::CMaterial::Normal);
		mat->setBlendFunc(CMaterial::one, CMaterial::zero);
		mat->setZWrite(false);
		mat->setZFunc(CMaterial::always);
		mat->setDoubleSided(true);

		m_QuadUV.V0 = CVector(0.f, 0.f, 0.5f);
		m_QuadUV.V1 = CVector(1.f, 0.f, 0.5f);
		m_QuadUV.V2 = CVector(1.f, 1.f, 0.5f);
		m_QuadUV.V3 = CVector(0.f, 1.f, 0.5f);

		m_QuadUV.Uv0 = CUV(0.f,  0.f);
		m_QuadUV.Uv1 = CUV(1.f, 0.f);
		m_QuadUV.Uv2 = CUV(1.f, 1.f);
		m_QuadUV.Uv3 = CUV(0.f,  1.f);

		CVertexBuffer &vb = m_VB;
		vb.clearValueEx();
		vb.addValueEx(CVertexBuffer::Position, CVertexBuffer::Float3);
		vb.addValueEx(CVertexBuffer::TexCoord0, CVertexBuffer::Float2);
		vb.addValueEx(CVertexBuffer::TexCoord1, CVertexBuffer::Float4);
		vb.initEx();
		vb.setPreferredMemory(CVertexBuffer::AGPPreferred, false);
		vb.setNumVertices(4);
		/*CVertexBufferReadWrite vba;
		vb.lock(vba);
		vba.setVertexCoord(0, 0.f, 0.f, 0.5f);
		vba.setVertexCoord(1, 1.f, 0.f, 0.5f);
		vba.setVertexCoord(2, 1.f, 1.f, 0.5f);
		vba.setVertexCoord(3, 0.f, 1.f, 0.5f);
		vba.setTexCoord(0, 0, 0.f, 0.f);
		vba.setTexCoord(1, 0, 1.f, 0.f);
		vba.setTexCoord(2, 0, 1.f, 1.f);
		vba.setTexCoord(3, 0, 0.f, 1.f);*/
		/*vba.setTexCoord(0, 1, 0.f, 0.f);
		vba.setTexCoord(1, 1, 1.f, 0.f);
		vba.setTexCoord(2, 1, 1.f, 1.f);
		vba.setTexCoord(3, 1, 0.f, 1.f);*/
	}
}

CFXAA::~CFXAA()
{
	nldebug("3D: Destroy FXAA");

	if (!m_Mat.empty())
	{
		m_Driver->deleteMaterial(m_Mat);
	}

	delete m_PP;
	m_PP = NULL;

	m_Driver = NULL;
}

void CFXAA::applyEffect()
{
	if (!m_PP)
		return;

	CDriverUser *dru = static_cast<CDriverUser *>(m_Driver);
	IDriver *drv = dru->getDriver();

	NL3D::ITexture *renderTarget = drv->getRenderTarget();
	nlassert(renderTarget);
	nlassert(renderTarget->isBloomTexture());

	uint width = renderTarget->getWidth();
	uint height = renderTarget->getHeight();
	bool mode2D = static_cast<CTextureBloom *>(renderTarget)->isMode2D();
	nlassert(renderTarget->getUploadFormat() == ITexture::Auto);

	float fwidth = (float)width;
	float fheight = (float)height;

	// create render target
	CTextureUser *otherRenderTarget = m_Driver->getRenderTargetManager().getRenderTarget(width, height, mode2D);

	// swap render target
	CTextureUser texNull;
	dru->setRenderTarget(texNull);
	drv->swapTextureHandle(*renderTarget, *otherRenderTarget->getITexture());
	drv->setRenderTarget(renderTarget);

	// render effect
	m_Mat.getObjectPtr()->setTexture(0, otherRenderTarget->getITexture());
	m_Driver->drawQuad(m_QuadUV, m_Mat);
	m_Mat.getObjectPtr()->setTexture(0, NULL);

	// recycle render target
	m_Driver->getRenderTargetManager().recycleRenderTarget(otherRenderTarget);
}

} /* namespace NL3D */

/* end of file */
