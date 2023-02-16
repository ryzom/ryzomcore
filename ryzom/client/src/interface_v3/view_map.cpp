// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2023  Winch Gate Property Limited
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

#include "stdpch.h"

#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/xml_macros.h"

#include "view_map.h"

extern NL3D::UDriver *Driver;

NLMISC_REGISTER_OBJECT(CViewBase, CViewMap, std::string, "mapview");

// ***********************************************************************
CViewMap::CViewMap(const TCtorParam &param) : CViewBase(param),
	m_Position{0,0}, m_Offset{0,0}, m_WorldSize{0}, m_VisionReal{0, 0}, m_RotateZ{0},
	m_MinCorner{0,0}, m_MaxCorner{0,0}, m_MapWidth{0}, m_MapHeight{0},
	m_ReloadTexture{true}, m_TextureWidth{0}, m_TextureHeight{0},
	m_MapURatio{1.f}, m_MapVRatio{1.f}, m_TextureFile{NULL},
	m_ReloadTextureMask{true}, m_TextureMaskFile{NULL}
{
	m_Material = Driver->createMaterial();
	m_Material.initUnlit();
	m_Material.setSrcBlend(NL3D::UMaterial::srcalpha);
	m_Material.setDstBlend(NL3D::UMaterial::invsrcalpha);
	m_Material.setBlend(true);
	m_Material.setZFunc(NL3D::UMaterial::always);
	m_Material.setZWrite(false);

	m_MaskMaterial = Driver->createMaterial();
	m_MaskMaterial.initUnlit();
	m_MaskMaterial.setAlphaTest(true);
}

// ***********************************************************************
CViewMap::~CViewMap()
{
	if (m_TextureFile)
		Driver->deleteTextureFile(m_TextureFile);

	if (m_TextureMaskFile)
		Driver->deleteTextureFile(m_TextureMaskFile);

	if (!m_Material.empty())
		Driver->deleteMaterial(m_Material);

	if (!m_MaskMaterial.empty())
		Driver->deleteMaterial(m_MaskMaterial);
}

// ***********************************************************************
void CViewMap::setWorldSize(float ws)
{
	m_WorldSize = ws;

	updateVision();
}

// ***********************************************************************
void CViewMap::updateVision()
{
	m_VisionReal = {m_WorldSize / 2.f, m_WorldSize / 2.f};

	if (_WReal == 0 || _HReal == 0)
		return;

	// convert vision to same w/h ratio as gui widget
	if (_WReal > _HReal)
		m_VisionReal.x = m_VisionReal.x * ((float)_WReal / (float)_HReal);
	else
		m_VisionReal.y = m_VisionReal.y * ((float)_HReal / (float)_WReal);
}

// ***********************************************************************
void CViewMap::updateMapSize()
{
	if (m_MinCorner.x > m_MaxCorner.x) std::swap(m_MinCorner.x, m_MaxCorner.x);
	if (m_MinCorner.y > m_MaxCorner.y) std::swap(m_MinCorner.y, m_MaxCorner.y);
	m_MapWidth = m_MaxCorner.x - m_MinCorner.x;
	m_MapHeight = m_MaxCorner.y - m_MinCorner.y;
}

// ***********************************************************************
void CViewMap::setMap(const std::string &texture, NLMISC::CVector2f minCorner, NLMISC::CVector2f maxCorner)
{
	m_Texture = texture;
	m_MinCorner = minCorner;
	m_MaxCorner = maxCorner;
	updateMapSize();

	loadTexture();
}

// ***********************************************************************
static bool lookupTexture(const std::string &name, std::string &fullName, uint32 &w, uint32 &h)
{
	fullName = NLMISC::CPath::lookup(name, false, false);
	if (fullName.empty())
	{
		nlwarning("Map file '%s' not found", name.c_str());
		return false;
	}

	// verify file format
	NLMISC::CIFile bm;
	if (bm.open(fullName))
	{
		try
		{
			NLMISC::CBitmap::loadSize(fullName, w, h);
		}
		catch(const NLMISC::Exception &e)
		{
			nlwarning(e.what());
			return false;
		}
	}
	else
	{
		nlwarning("Can't open map %s", name.c_str());
		return false;
	}

	return true;
}

// ***********************************************************************
void CViewMap::unloadTexture()
{
	if (!m_TextureFile)
		return;

	Driver->deleteTextureFile(m_TextureFile);

	m_TextureFile = NULL;
	m_Material.setTexture(0, NULL);
}

// ***********************************************************************
void CViewMap::loadTexture()
{
	m_ReloadTexture = false;

	unloadTexture();

	// TODO: call lua for remap?

	uint32 w, h;
	std::string fullName;
	if (!lookupTexture(m_Texture, fullName, w, h)) {
		return;
	}

	// load texture
	m_TextureFile = Driver->createTextureFile(fullName);
	m_TextureFile->setWrapS(NL3D::UTexture::Clamp);
	m_TextureFile->setWrapT(NL3D::UTexture::Clamp);
	// map texture is shared with map, so pow2tex must be same as CGroupMap is using
	m_TextureFile->setEnlargeCanvasNonPOW2Tex(true);

	m_TextureWidth = w;
	m_TextureHeight = h;

	m_MapURatio = (float) w / (float) NLMISC::raiseToNextPowerOf2(m_TextureWidth);
	m_MapVRatio = (float) h / (float) NLMISC::raiseToNextPowerOf2(m_TextureHeight);

	// FIXME: islands

	m_Material.setTexture(0, m_TextureFile);
}

// ***********************************************************************
void CViewMap::unloadTextureMask()
{
	if (!m_TextureMaskFile)
		return;

	Driver->deleteTextureFile(m_TextureMaskFile);

	m_TextureMaskFile = NULL;
	m_MaskMaterial.setTexture(0, NULL);
}

// ***********************************************************************
void CViewMap::loadTextureMask()
{
	m_ReloadTextureMask = false;

	unloadTextureMask();

	// FIXME: use CViewRenderer for mask material
	uint32 w, h;
	std::string fullName;
	if (m_TextureMask.empty() || !lookupTexture(m_TextureMask, fullName, w, h))
		return;

	m_TextureMaskFile = Driver->createTextureFile(fullName);
	m_TextureMaskFile->setWrapS(NL3D::UTexture::Clamp);
	m_TextureMaskFile->setWrapT(NL3D::UTexture::Clamp);
	m_MaskMaterial.setTexture(0, m_TextureMaskFile);
}

// ***********************************************************************
bool CViewMap::parse(xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if (!CViewBase::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr prop;
	XML_READ_STRING(cur, "texture", m_Texture, "");
	XML_READ_STRING(cur, "texture_mask", m_TextureMask, "");
	XML_READ_FLOAT(cur, "posx", m_Position.x, 0);
	XML_READ_FLOAT(cur, "posy", m_Position.y, 0);
	XML_READ_FLOAT(cur, "left", m_MinCorner.x, 0);
	XML_READ_FLOAT(cur, "bottom", m_MinCorner.y, 0);
	XML_READ_FLOAT(cur, "right", m_MaxCorner.x, 0);
	XML_READ_FLOAT(cur, "top", m_MaxCorner.y, 0);
	XML_READ_FLOAT(cur, "offsetx", m_Offset.x, 0);
	XML_READ_FLOAT(cur, "offsety", m_Offset.y, 0);
	XML_READ_FLOAT(cur, "world_size", m_WorldSize, 0.f);

	updateVision();
	updateMapSize();

	return true;
}

// ***********************************************************************
void CViewMap::updateCoords()
{
	CViewBase::updateCoords();

	updateVision();
}

// ***********************************************************************
void CViewMap::computeUVRect(NLMISC::CUV &minUV, NLMISC::CUV &maxUV, sint32 &dX, sint32 &dY, sint32 &dW, sint32 &dH) const
{
	if (m_MapWidth == 0 || m_MapHeight == 0)
		return;

	float scaleX = (float)_WReal / m_VisionReal.x;
	float scaleY = (float)_HReal / m_VisionReal.y;
	float scale = std::min(scaleX, scaleY);

	float xRatio = _WReal / (m_VisionReal.x * 2);
	float yRatio = _HReal / (m_VisionReal.y * 2);

	float left = ((m_Position.x + m_Offset.x - m_VisionReal.x) - m_MinCorner.x);
	float bottom = (m_MaxCorner.y - (m_Position.y + m_Offset.y - m_VisionReal.y));

	float right = left + m_VisionReal.y * 2;
	float top = bottom - m_VisionReal.y * 2;

	if (left < 0)
	{
		float d = std::abs(left) * xRatio;
		dX += d; dW -= d;
		left = 0;
	}

	if (top < 0)
	{
		float d = std::abs(top) * yRatio;
		dH -= d;
		top = 0;
	}

	if (bottom > m_MapHeight)
	{
		float d = (bottom - m_MapHeight) * yRatio;
		dY += d; dH -= d;
		bottom = m_MapHeight;
	}

	if (right > m_MapWidth)
	{
		float d = (right - m_MapWidth) * xRatio;
		dW -= d;
		right = m_MapWidth;
	}

	// relative to map area
	minUV.U = left / m_MapWidth; maxUV.U = right / m_MapWidth;
	minUV.V = top / m_MapHeight; maxUV.V = bottom / m_MapHeight;

	// relative to pow2 texture
	minUV.U *= m_MapURatio;
	minUV.V *= m_MapVRatio;
	maxUV.U *= m_MapURatio;
	maxUV.V *= m_MapVRatio;
}

// ***********************************************************************
void CViewMap::draw()
{
	if (m_ReloadTexture)
		loadTexture();

	if (m_ReloadTextureMask)
		loadTextureMask();

	// nothing would be visible
	if (m_MapWidth == 0 || m_MapHeight == 0 || _WReal == 0 || _HReal == 0)
		return;

	// flush gui quads
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	rVR.flush();

	NL3D::CScissor oldScissor;
	oldScissor = Driver->getScissor();

	uint32 sw, sh;
	rVR.getScreenSize(sw, sh);

	sint32 sciX, sciY, sciW, sciH;
	rVR.getClipWindow(sciX, sciY, sciW, sciH);

	NL3D::CScissor newScissor;
	newScissor.X = sciX / (float) sw;
	newScissor.Width = sciW / (float) sw;
	newScissor.Y = sciY / (float) sh;
	newScissor.Height = sciH / (float) sh;
	Driver->setScissor(newScissor);

	// -----------------------------------------------------------------------
	// enable drawing to stencil
	const uint mask = 0x01;
	bool useStencil = m_TextureMaskFile != NULL;
	if (useStencil)
	{
		Driver->enableStencilTest(true);
		Driver->stencilMask(0xFF);
		Driver->stencilOp(NL3D::UDriver::replace, NL3D::UDriver::replace, NL3D::UDriver::replace);
		Driver->setColorMask(false, false, false, false);

		// -----------------------------------------------------------------------
		// draw mask
		Driver->stencilFunc(NL3D::UDriver::always, mask, 0xFF);

		rVR.drawCustom(_XReal, _YReal, _WReal, _HReal, NLMISC::CRGBA::White, m_MaskMaterial);
		rVR.flush();

		// -----------------------------------------------------------------------
		// drawp map, enable rendering only to area where mask was renderer
		Driver->setColorMask(true, true, true, true);
		Driver->stencilOp(NL3D::UDriver::keep, NL3D::UDriver::keep, NL3D::UDriver::keep);
		Driver->stencilFunc(NL3D::UDriver::equal, mask, 0xFF);
	}

	// TODO: parent container alpha, content alpha
	NLMISC::CRGBA color = NLMISC::CRGBA::White;
	color.A = CWidgetManager::getInstance()->getGlobalColorForContent().A;

	NLMISC::CQuadColorUV qcoluv;
	qcoluv.Color0 = qcoluv.Color1 = qcoluv.Color2 = qcoluv.Color3 = color;

	NLMISC::CUV uvMin, uvMax;
	sint32 dX = _XReal, dY = _YReal, dW = _WReal, dH = _HReal;
	computeUVRect(uvMin, uvMax, dX, dY, dW, dH);

	qcoluv.V0.set(dX,      dY,      0);
	qcoluv.V1.set(dX + dW, dY,      0);
	qcoluv.V2.set(dX + dW, dY + dH, 0);
	qcoluv.V3.set(dX,      dY + dH, 0);

	qcoluv.Uv0.U = uvMin.U; qcoluv.Uv0.V = uvMax.V;
	qcoluv.Uv1.U = uvMax.U; qcoluv.Uv1.V = uvMax.V;
	qcoluv.Uv2.U = uvMax.U; qcoluv.Uv2.V = uvMin.V;
	qcoluv.Uv3.U = uvMin.U; qcoluv.Uv3.V = uvMin.V;

	NLMISC::CMatrix mat;
	mat.identity();

	NLMISC::CVector2f pos{_XReal + _WReal * 0.5f, _YReal + _HReal * 0.5f};
	mat.translate(pos);
	mat.scale(1.f);
	mat.rotateZ(m_RotateZ);
	mat.translate(-pos);

	qcoluv.V0 = mat * qcoluv.V0;
	qcoluv.V1 = mat * qcoluv.V1;
	qcoluv.V2 = mat * qcoluv.V2;
	qcoluv.V3 = mat * qcoluv.V3;

	// translate gui to screen
	float oow, ooh;
	rVR.getScreenOOSize(oow, ooh);
	qcoluv.V0.x *= oow; qcoluv.V0.y *= ooh;
	qcoluv.V1.x *= oow; qcoluv.V1.y *= ooh;
	qcoluv.V2.x *= oow; qcoluv.V2.y *= ooh;
	qcoluv.V3.x *= oow; qcoluv.V3.y *= ooh;

	Driver->drawQuads(&qcoluv, 1, m_Material);

	// -----------------------------------------------------------------------
	// disable stencil test
	if (useStencil)
	{
		Driver->enableStencilTest(false);
		Driver->stencilMask(0xff);
		Driver->stencilOp(NL3D::UDriver::keep, NL3D::UDriver::keep, NL3D::UDriver::keep);
		Driver->stencilFunc(NL3D::UDriver::always, 0, 0xFF);
	}

	// restore
	Driver->setScissor(oldScissor);
}

