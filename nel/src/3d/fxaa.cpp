/**
 * \file fxaa.cpp
 * \brief CFXAA
 * \date 2014-08-03 21:41GMT
 * \author Jan Boon (Kaetemi)
 * CFXAA
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "std3d.h"
#include "nel/3d/fxaa.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "nel/3d/u_camera.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/vertex_program.h"
#include "nel/3d/pixel_program.h"
#include "nel/3d/material.h"
#include "nel/3d/texture_offscreen.h"
#include "nel/3d/texture_user.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/render_target_manager.h"
#include "nel/3d/uniform_buffer_format.h"
#include "nel/3d/uniform_buffer.h"

using namespace std;
// using namespace NLMISC;

namespace NL3D {

namespace {
#include "fxaa_program.h"

// FXAA PP UBO format and offsets
struct CFXAAUBOOffsets
{
	sint RcpFrame;
	sint Subpix;
	sint EdgeThreshold;
	sint EdgeThresholdMin;
};
static CFXAAUBOOffsets s_FXAAUBOOffsets;
static NLMISC::CSmartPtr<CUniformBufferFormat> s_FXAAUBFormat;
static NLMISC::CSmartPtr<CUniformBuffer> s_FXAAUB;
} /* anonymous namespace */

class CVertexProgramFXAA : public CVertexProgram
{
public:
	struct CIdx
	{
		uint ModelViewProjection;
	};

	virtual void buildInfo()
	{
		CVertexProgram::buildInfo();
		if (source() && source()->Features.OnlyUBOs)
			return;
		if (profile() == nelvp || profile() == arbvp1 || profile() == vs_2_0)
		{
			m_Idx.ModelViewProjection = 0;
		}
		else
		{
			m_Idx.ModelViewProjection = getUniformIndex("modelViewProjection");
		}
	}

	const CIdx &idx() const { return m_Idx; }

private:
	CIdx m_Idx;
};

class CPixelProgramFXAA : public CPixelProgram
{
public:
	struct CIdx
	{
		uint RcpFrame;
		uint Subpix;
		uint EdgeThreshold;
		uint EdgeThresholdMin;
	};

	virtual void buildInfo()
	{
		CPixelProgram::buildInfo();
		if (source() && source()->Features.OnlyUBOs)
			return;
		if (profile() == arbfp1 || profile() == ps_2_0)
		{
			m_Idx.RcpFrame = 0;
			m_Idx.Subpix = 1;
			m_Idx.EdgeThreshold = 2;
			m_Idx.EdgeThresholdMin = 3;
		}
		else
		{
			m_Idx.RcpFrame = getUniformIndex("fxaaQualityRcpFrame");
			m_Idx.Subpix = getUniformIndex("fxaaQualitySubpix");
			m_Idx.EdgeThreshold = getUniformIndex("fxaaQualityEdgeThreshold");
			m_Idx.EdgeThresholdMin = getUniformIndex("fxaaQualityEdgeThresholdMin");
		}
	}

	const CIdx &idx() const { return m_Idx; }

private:
	CIdx m_Idx;
};

}

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

CFXAA::CFXAA(NL3D::UDriver *driver) : m_Driver(driver), m_VP(NULL), m_PP(NULL), m_Width(~0), m_Height(~0)
{
	nldebug("3D: Create FXAA");

	CDriverUser *dru = static_cast<CDriverUser *>(driver);
	NL3D::IDriver *drv = (dru)->getDriver();

	if (drv->supportBloomEffect() && drv->supportNonPowerOfTwoTextures())
	{
		m_PP = new CPixelProgramFXAA();
		// Build FXAA PP UBO format (once)
		if (!s_FXAAUBFormat)
		{
			CUniformBufferFormat *fmt = new CUniformBufferFormat();
			fmt->Name = "NlFXAA";
			s_FXAAUBOOffsets.RcpFrame = fmt->push("fxaaQualityRcpFrame", CUniformBufferFormat::FloatVec2);
			s_FXAAUBOOffsets.Subpix = fmt->push("fxaaQualitySubpix", CUniformBufferFormat::Float);
			s_FXAAUBOOffsets.EdgeThreshold = fmt->push("fxaaQualityEdgeThreshold", CUniformBufferFormat::Float);
			s_FXAAUBOOffsets.EdgeThresholdMin = fmt->push("fxaaQualityEdgeThresholdMin", CUniformBufferFormat::Float);
			s_FXAAUBFormat = fmt;
			s_FXAAUB = new CUniformBuffer();
			s_FXAAUB->Format = *fmt;
		}
		// glsl300esf — pipeline stage UBO source (preferred for linked program path)
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Features.OnlyUBOs = true;
			source->Features.NoBuiltinUniforms = true;
			source->UniformBufferFormats[UBBindingPixelProgram] = s_FXAAUBFormat;
			source->Profile = IProgram::glsl300esf;
			source->DisplayName = "glsl300esf/FXAA_PP/UBO";
			source->setSource(std::string(a_glsl300esf) + a_fxaaBody);
			m_PP->addSource(source);
		}
		// glsl330f — SSO fallback
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Features.NoBuiltinUniforms = true;
			source->Profile = IProgram::glsl330f;
			source->setSourcePtr(a_glsl330f);
			m_PP->addSource(source);
		}
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
			nlwarning("3D: No supported pixel program for FXAA effect");

			delete m_PP;
			m_PP = NULL;
		}
		else
		{
			nldebug("3D: FXAA pixel program available");
		}
	}

	if (!m_PP)
	{
		return;
	}

	// create vp
	{
		m_VP = new CVertexProgramFXAA();
		// glsl300esv — pipeline stage UBO source (preferred for linked program path)
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Features.OnlyUBOs = true;
			source->Features.UsesObjectUBO = true;
			source->Profile = IProgram::glsl300esv;
			source->DisplayName = "glsl300esv/FXAA_VP/UBO";
			source->setSourcePtr(a_glsl300esv);
			m_VP->addSource(source);
		}
		// glsl330v — SSO fallback
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::glsl330v;
			source->setSourcePtr(a_glsl330v);
			m_VP->addSource(source);
		}
		// nelvp
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::nelvp;
			source->setSourcePtr(a_nelvp);
			m_VP->addSource(source);
		}
		if (!drv->compileVertexProgram(m_VP))
		{
			nlwarning("3D: No supported vertex program for FXAA effect");

			delete m_VP;
			m_VP = NULL;
			delete m_PP;
			m_PP = NULL;
		}
		else
		{
			nldebug("3D: FXAA vertex program available");
		}
	}

	if (!m_VP)
	{
		return;
	}
	
	// create material and vb
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

		/*if (drv->textureCoordinateAlternativeMode())
		{
			m_QuadUV.Uv0 = CUV(0.f,  1.f);
			m_QuadUV.Uv1 = CUV(1.f, 1.f);
			m_QuadUV.Uv2 = CUV(1.f, 0.f);
			m_QuadUV.Uv3 = CUV(0.f,  0.f);
		}
		else
		{*/
			m_QuadUV.Uv0 = CUV(0.f,  0.f);
			m_QuadUV.Uv1 = CUV(1.f, 0.f);
			m_QuadUV.Uv2 = CUV(1.f, 1.f);
			m_QuadUV.Uv3 = CUV(0.f,  1.f);
		/*}*/

		/*CVertexBuffer &vb = m_VB;
		vb.clearValueEx();
		vb.addValueEx(CVertexBuffer::Position, CVertexBuffer::Float3);
		vb.addValueEx(CVertexBuffer::TexCoord0, CVertexBuffer::Float2);
		vb.addValueEx(CVertexBuffer::TexCoord1, CVertexBuffer::Float4);
		vb.initEx();
		vb.setBufferUsage(CVertexBuffer::SmallStream, false);
		vb.setNumVertices(4);*/
	}
}

CFXAA::~CFXAA()
{
	nldebug("3D: Destroy FXAA");

	if (!m_Mat.empty())
	{
		m_Driver->deleteMaterial(m_Mat);
	}

	delete m_VP;
	m_VP = NULL;
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

	// backup
	bool fogEnabled = m_Driver->fogEnabled();
	m_Driver->enableFog(false);

	NL3D::ITexture *renderTarget = drv->getRenderTarget();
	nlassert(renderTarget);
	nlassert(renderTarget->isOffscreenTexture());

	uint width = renderTarget->getWidth();
	uint height = renderTarget->getHeight();
	bool needsDepthStencil = static_cast<CTextureOffscreen *>(renderTarget)->needsDepthStencil();
	nlassert(renderTarget->getUploadFormat() == ITexture::Auto);

	float fwidth = (float)width;
	float fheight = (float)height;
	float pwidth = 1.0f / fwidth;
	float pheight = 1.0f / fheight;
	float hpwidth = pwidth * 0.5f;
	float hpheight = pheight * 0.5f;
	float n = 0.5f;

	//if (width != m_Width || height != m_Height)
	/*{
		// Build VB
		m_Width = width;
		m_Height = height;
		CVertexBufferReadWrite vba;
		m_VB.lock(vba);
		vba.setValueFloat3Ex(CVertexBuffer::Position, 0, 0.f, 0.f, 0.5f); // BL
		vba.setValueFloat3Ex(CVertexBuffer::Position, 1, 1.f, 0.f, 0.5f); // BR
		vba.setValueFloat3Ex(CVertexBuffer::Position, 2, 1.f, 1.f, 0.5f); // TR
		vba.setValueFloat3Ex(CVertexBuffer::Position, 3, 0.f, 1.f, 0.5f); // TL
		vba.setValueFloat2Ex(CVertexBuffer::TexCoord0, 0, 0.f, 0.f);
		vba.setValueFloat2Ex(CVertexBuffer::TexCoord0, 1, 1.f, 0.f);
		vba.setValueFloat2Ex(CVertexBuffer::TexCoord0, 2, 1.f, 1.f);
		vba.setValueFloat2Ex(CVertexBuffer::TexCoord0, 3, 0.f, 1.f);
		vba.setValueFloat4Ex(CVertexBuffer::TexCoord1, 0, 0.f - hpwidth, 0.f - hpheight, 0.f + hpwidth, 0.f + hpheight);
		vba.setValueFloat4Ex(CVertexBuffer::TexCoord1, 1, 1.f - hpwidth, 0.f - hpheight, 1.f + hpwidth, 0.f + hpheight);
		vba.setValueFloat4Ex(CVertexBuffer::TexCoord1, 2, 1.f - hpwidth, 1.f - hpheight, 1.f + hpwidth, 1.f + hpheight);
		vba.setValueFloat4Ex(CVertexBuffer::TexCoord1, 3, 0.f - hpwidth, 1.f - hpheight, 0.f + hpwidth, 1.f + hpheight);
	}*/

	// create render target
	CTextureUser *otherRenderTarget = m_Driver->getRenderTargetManager().getRenderTarget(width, height, needsDepthStencil);
	nlassert(otherRenderTarget);

	// swap render target
	CTextureUser texNull;
	dru->setRenderTarget(texNull);
	drv->swapTextureHandle(*renderTarget, *otherRenderTarget->getITexture());
	drv->setRenderTarget(renderTarget);
	m_Driver->setMatrixMode2D11();

	// debug
	// m_Driver->clearBuffers(CRGBA(128, 128, 128, 128));

	// activate program
	bool vpok = drv->activeVertexProgram(m_VP);
	nlassert(vpok);
	bool ppok = drv->activePixelProgram(m_PP);
	nlassert(ppok);

	bool ppUBO = m_PP->source() && m_PP->source()->Features.OnlyUBOs;
	bool vpUBO = m_VP->source() && m_VP->source()->Features.OnlyUBOs;

	if (ppUBO)
	{
		s_FXAAUB->lock();
		s_FXAAUB->set(s_FXAAUBOOffsets.RcpFrame, 1.0f / fwidth, 1.0f / fheight);
		s_FXAAUB->set(s_FXAAUBOOffsets.Subpix, 0.75f);
		s_FXAAUB->set(s_FXAAUBOOffsets.EdgeThreshold, 0.166f);
		s_FXAAUB->set(s_FXAAUBOOffsets.EdgeThresholdMin, 0.0833f);
		s_FXAAUB->unlock();
		drv->bindUniformBuffer(UBBindingPixelProgram, s_FXAAUB);
	}
	else
	{
		const CPixelProgramFXAA::CIdx &ppIdx = static_cast<CPixelProgramFXAA *>(m_PP)->idx();
		drv->setUniform2f(IDriver::PixelProgram, ppIdx.RcpFrame, 1.0f / fwidth, 1.0f / fheight);
		drv->setUniform1f(IDriver::PixelProgram, ppIdx.Subpix, 0.75f);
		drv->setUniform1f(IDriver::PixelProgram, ppIdx.EdgeThreshold, 0.166f);
		drv->setUniform1f(IDriver::PixelProgram, ppIdx.EdgeThresholdMin, 0.0833f);
	}

	if (!vpUBO)
	{
		drv->setUniformMatrix(IDriver::VertexProgram, static_cast<CVertexProgramFXAA *>(m_VP)->idx().ModelViewProjection, IDriver::ModelViewProjection, IDriver::Identity);
	}

	// render effect
	m_Mat.getObjectPtr()->setTexture(0, otherRenderTarget->getITexture());
	/*drv->activeVertexBuffer(m_VB);
	drv->renderRawQuads(*m_Mat.getObjectPtr(), 0, 1);*/
	m_Driver->drawQuad(m_QuadUV, m_Mat);
	m_Mat.getObjectPtr()->setTexture(0, NULL);

	// deactivate program
	drv->activeVertexProgram(NULL);
	drv->activePixelProgram(NULL);
	if (ppUBO) drv->bindUniformBuffer(UBBindingPixelProgram, NULL);

	// restore
	m_Driver->enableFog(fogEnabled);

	// recycle render target
	m_Driver->getRenderTargetManager().recycleRenderTarget(otherRenderTarget);
}

} /* namespace NL3D */

/* end of file */
