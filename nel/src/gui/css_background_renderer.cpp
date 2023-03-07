// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2021  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/css_background_renderer.h"
#include "nel/gui/css_border_renderer.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_bitmap.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	// ----------------------------------------------------------------------------
	CSSBackgroundRenderer::CSSBackgroundRenderer()
	: CurrentAlpha(255), TextureId(-1),
		m_BorderX(0), m_BorderY(0), m_BorderW(0), m_BorderH(0),
		m_PaddingX(0), m_PaddingY(0), m_PaddingW(0), m_PaddingH(0),
		m_ContentX(0), m_ContentY(0), m_ContentW(0), m_ContentH(0),
		m_RootFontSize(16.f), m_FontSize(16.f), m_Viewport(NULL),
		m_RenderLayer(0), m_ModulateGlobalColor(false), m_FillViewport(false),
		m_Dirty(false)
	{
	}

	// ----------------------------------------------------------------------------
	CSSBackgroundRenderer::~CSSBackgroundRenderer()
	{
		if (TextureId != -1)
			CViewRenderer::getInstance()->deleteTexture(TextureId);
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::clear()
	{
		m_Dirty = true;

		if (TextureId != -1)
			CViewRenderer::getInstance()->deleteTexture(TextureId);

		TextureId = -1;
		m_Background.image.clear();
		m_Background.color.A = 0;
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::setBackground(const CSSBackground &bg)
	{
		m_Dirty = true;
		// TODO: CSSBackground should keep track of TextureId
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		if (bg.image != m_Background.image && TextureId != -1)
			rVR.deleteTexture(TextureId);

		m_Background = bg;
		// TODO: does not accept urls
		if (TextureId == -1 && !bg.image.empty())
		{
			// TODO: make CViewRenderer accept urls
			if (bg.image.find("://") != std::string::npos)
				TextureId = rVR.createTexture(bg.image, 0, 0, -1, -1, false);
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::setImage(const std::string &bgtex)
	{
		m_Dirty = true;
		// TODO: CSSBackground should keep track of TextureId
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		if (bgtex != m_Background.image && TextureId != -1)
		{
			rVR.deleteTexture(TextureId);
			TextureId = -1;
		}
		m_Background.image = bgtex;

		if (TextureId == -1 && !bgtex.empty())
		{
			// TODO: make CViewRenderer accept urls
			if (bgtex.find("://") != std::string::npos)
				TextureId = rVR.createTexture(bgtex, 0, 0, -1, -1, false);
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::setImageRepeat(bool b)
	{
		m_Background.repeatX = b ? CSS_VALUE_REPEAT : CSS_VALUE_NOREPEAT;
		m_Background.repeatY = b ? CSS_VALUE_REPEAT : CSS_VALUE_NOREPEAT;
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::setImageCover(bool b)
	{
		m_Background.size = b ? CSS_VALUE_COVER : CSS_VALUE_AUTO;
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::updateCoords()
	{
		m_Dirty = false;
		m_DrawQueue.clear();

		// TODO: color from last background layer
		buildColorQuads(m_Background);

		// -------------------------------------------------------------------
		// background-image
		buildImageQuads(m_Background, TextureId);
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::draw() {
		if (m_Dirty) updateCoords();
		if (m_DrawQueue.empty()) return;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		// flush draw cache to ensure correct draw order
		rVR.flush();

		// TODO: no need for widget manager, if global color is set from parent
		CRGBA globalColor;
		if (m_ModulateGlobalColor)
			globalColor = CWidgetManager::getInstance()->getGlobalColor();

		// TODO: there might be issue on draw order IF using multiple textures
		// and second (top) texture is created before first one.
		for(uint i = 0; i < m_DrawQueue.size(); ++i)
		{
			CRGBA color = m_DrawQueue[i].Color;
			if (m_ModulateGlobalColor)
				color.modulateFromColor (color, globalColor);

			color.A = (uint8) (((uint16) CurrentAlpha * (uint16) color.A) >> 8);
			rVR.drawQuad(m_RenderLayer, m_DrawQueue[i].Quad, m_DrawQueue[i].TextureId, color, false);
		}

		// flush draw cache to ensure correct draw order
		rVR.flush();
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::getPositioningArea(const CSSBackground &bg, sint32 &areaX, sint32 &areaY, sint32 &areaW, sint32 &areaH) const
	{
		switch(bg.origin)
		{
			case CSS_VALUE_PADDING_BOX:
				areaX = m_PaddingX;
				areaY = m_PaddingY;
				areaW = m_PaddingW;
				areaH = m_PaddingH;
				break;
			case CSS_VALUE_CONTENT_BOX:
				areaX = m_ContentX;
				areaY = m_ContentY;
				areaW = m_ContentW;
				areaH = m_ContentH;
				break;
			case CSS_VALUE_BORDER_BOX:
				// fall thru
			default:
				areaX = m_BorderX;
				areaY = m_BorderY;
				areaW = m_BorderW;
				areaH = m_BorderH;
				break;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::getPaintingArea(const CSSBackground &bg, sint32 &areaX, sint32 &areaY, sint32 &areaW, sint32 &areaH) const
	{
		switch(bg.clip)
		{
			case CSS_VALUE_PADDING_BOX:
				areaX = m_PaddingX;
				areaY = m_PaddingY;
				areaW = m_PaddingW;
				areaH = m_PaddingH;
				break;
			case CSS_VALUE_CONTENT_BOX:
				areaX = m_ContentX;
				areaY = m_ContentY;
				areaW = m_ContentW;
				areaH = m_ContentH;
				break;
			case CSS_VALUE_BORDER_BOX:
				// fall thru
			default:
				areaX = m_BorderX;
				areaY = m_BorderY;
				areaW = m_BorderW;
				areaH = m_BorderH;
				break;
		}

		if (m_FillViewport && m_Viewport)
		{
			sint32 newX = std::min(areaX, m_Viewport->getXReal());
			sint32 newY = std::min(areaY, m_Viewport->getYReal());
			areaW = std::max(areaX + areaW, m_Viewport->getXReal() + m_Viewport->getWReal()) - newX;
			areaH = std::max(areaY + areaH, m_Viewport->getYReal() + m_Viewport->getHReal()) - newY;
			areaX = newX;
			areaY = newY;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::calculateSize(const CSSBackground &bg, sint32 &texW, sint32 &texH) const
	{
		sint32 areaX, areaY, areaW, areaH;
		getPositioningArea(bg, areaX, areaY, areaW, areaH);

		sint32 vpW=0;
		sint32 vpH=0;
		if (m_Viewport)
		{
			vpW = m_Viewport->getWReal();
			vpH = m_Viewport->getHReal();
		}

		float whRatio = (float)texW / (float)texH;
		switch(bg.size)
		{
			case CSS_VALUE_LENGTH:
			{
				if (bg.width.isAuto() && bg.height.isAuto())
				{
					// no-op
				}
				else if (bg.width.isAuto())
				{
					texH = bg.height.calculate(areaH, m_FontSize, m_RootFontSize, vpW, vpH);
					texW = texH * whRatio;
				}
				else if (bg.height.isAuto())
				{
					// calculate Height
					texW = bg.width.calculate(areaW, m_FontSize, m_RootFontSize, vpW, vpH);
					texH = texW / whRatio;
				}
				else
				{
					texW = bg.width.calculate(areaW, m_FontSize, m_RootFontSize, vpW, vpH);
					texH = bg.height.calculate(areaH, m_FontSize, m_RootFontSize, vpW, vpH);
				}
				break;
			}
			case CSS_VALUE_AUTO:
			{
				// no-op
				break;
			}
			case CSS_VALUE_COVER:
			{
				float canvasRatio = (float)areaW / (float)areaH;
				if (whRatio < canvasRatio)
				{
					texW = areaW;
					texH = areaW / whRatio;
				} else {
					texW = areaH * whRatio;
					texH = areaH;
				}
				break;
			}
			case CSS_VALUE_CONTAIN:
			{
				// same as covert, but ratio check is reversed
				float canvasRatio = (float)areaW / (float)areaH;
				if (whRatio > canvasRatio)
				{
					texW = areaW;
					texH = areaW / whRatio;
				} else {
					texW = areaH * whRatio;
					texH = areaH;
				}
				break;
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::calculatePosition(const CSSBackground &bg, sint32 &texX, sint32 &texY, sint32 &texW, sint32 &texH) const
	{
		sint32 areaX, areaY, areaW, areaH;
		getPositioningArea(bg, areaX, areaY, areaW, areaH);

		sint32 vpW=0;
		sint32 vpH=0;
		if (m_Viewport)
		{
			vpW = m_Viewport->getWReal();
			vpH = m_Viewport->getHReal();
		}

		float ofsX = bg.xPosition.calculate(1, m_FontSize, m_RootFontSize, vpW, vpH);
		float ofsY = bg.yPosition.calculate(1, m_FontSize, m_RootFontSize, vpW, vpH);

		if (bg.xPosition.isPercent() || bg.xAnchor == CSS_VALUE_CENTER)
		{
			if (bg.xAnchor == CSS_VALUE_RIGHT)
				ofsX = 1.f - ofsX;
			else if (bg.xAnchor == CSS_VALUE_CENTER)
				ofsX = 0.5f;

			ofsX = (float)(areaW - texW) * ofsX;
		}
		else if (bg.xAnchor == CSS_VALUE_RIGHT)
		{
			ofsX = areaW - texW - ofsX;
		}

		// areaY is bottom edge, areaY+areaH is top edge
		if (bg.yPosition.isPercent() || bg.yAnchor == CSS_VALUE_CENTER)
		{
			if (bg.yAnchor == CSS_VALUE_TOP)
				ofsY = 1.f - ofsY;
			else if (bg.yAnchor == CSS_VALUE_CENTER)
				ofsY = 0.5f;

			ofsY = (float)(areaH - texH) * ofsY;
		}
		else if (bg.yAnchor == CSS_VALUE_TOP)
		{
			ofsY = areaH - texH - ofsY;
		}

		texX = areaX + ofsX;
		texY = areaY + ofsY;
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::getImageTile(sint32 &tilePos, sint32 &tileSize, sint32 &spacing, sint32 &tiles, sint32 areaPos, sint32 areaSize, CSSValueType repeat) const
	{
		switch(repeat)
		{
			case CSS_VALUE_NOREPEAT:
			{
				tiles = 1;
				spacing = 0;
				break;
			}
			case CSS_VALUE_SPACE:
			{
				// if no space for 2+ tiles, then show single one on calculated tilePos
				if (tileSize * 2 > areaSize)
				{
					// set spacing large enough to only display single tile
					tiles = 1;
					spacing = areaSize;
				}
				else
				{
					// available for middle tiles
					sint32 midSize = (areaSize - tileSize*2);
					// number of middle tiles
					sint32 midTiles = midSize / tileSize;

					tiles = 2 + midTiles;
					tilePos = areaPos;
					// int div for floor()
					spacing = ( midSize - tileSize * midTiles) / (midTiles + 1);
				}
				break;
			}
			case CSS_VALUE_ROUND:
				// fall-thru - size is already calculated
			case CSS_VALUE_REPEAT:
				// fall-thru
			default:
			{
				tilePos -= std::ceil(abs(tilePos - areaPos)/(float)tileSize)*tileSize;
				tiles = std::ceil((std::abs(areaPos - tilePos) + areaSize) / (float)tileSize);
				spacing = 0;
				break;
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::calculateTiles(const CSSBackground &bg, sint32 &texX, sint32 &texY, sint32 &texW, sint32 &texH, sint32 &tilesX, sint32 &tilesY, sint32 &spacingX, sint32 &spacingY) const
	{
		sint32 areaX, areaY, areaW, areaH;
		getPositioningArea(bg, areaX, areaY, areaW, areaH);

		// texX,texY is for position area (ie content-box), but painting area can be bigger (ie border-box)
		sint32 paintX, paintY, paintW, paintH;
		getPaintingArea(bg, paintX, paintY, paintW, paintH);
		if (paintX < areaX)
			areaX -= std::ceil((areaX - paintX) / (float)texW) * texW;
		if ((paintX + paintW) > (areaX + areaW))
			areaW += std::ceil(((paintX + paintW) - (areaX + areaW)) / (float)texW) * texW;
		if (paintY < areaY)
			areaY -= std::ceil((areaY - paintY) / (float)texH) * texH;
		if ((paintY + paintH) > (areaY + areaH))
			areaH += std::ceil(((paintY + paintH) - (areaY + areaH)) / (float)texH) * texH;

		if (texW <= 0 || texH <= 0 || areaW <= 0 || areaH <= 0)
		{
			tilesX = tilesY = 0;
			spacingX = spacingY = 0;
			return;
		}

		if (bg.repeatX == CSS_VALUE_ROUND)
		{
			sint numTiles = std::max(1, (sint)std::ceil(((float)areaW / texW) - 0.5f));
			texW = areaW / numTiles;
			if (bg.height.isAuto() && bg.repeatY != CSS_VALUE_ROUND)
			{
				float aspect = (float)areaW / (numTiles * texW);
				texH = texW * aspect;
			}
		}

		if (bg.repeatY == CSS_VALUE_ROUND)
		{
			sint numTiles = std::max(1, (sint)std::ceil(((float)areaH / texH) - 0.5f));
			texH = areaH / numTiles;
			if (bg.width.isAuto() && bg.repeatX != CSS_VALUE_ROUND)
			{
				float aspect = (float)areaH / (numTiles * texH);
				texW = texH * aspect;
			}
		}

		getImageTile(texX, texW, spacingX, tilesX, areaX, areaW, bg.repeatX);
		getImageTile(texY, texH, spacingY, tilesY, areaY, areaH, bg.repeatY);
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::buildColorQuads(const CSSBackground &bg)
	{
		if (bg.color.A == 0)
			return;

		// painting area defined with background-clip
		sint32 x, y, w, h;
		getPaintingArea(bg, x, y, w, h);

		if (w <= 0 || h <= 0)
			return;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		SDrawQueue shape;
		shape.Quad.Uv0.set(0.f, 1.f);
		shape.Quad.Uv1.set(1.f, 1.f);
		shape.Quad.Uv2.set(1.f, 0.f);
		shape.Quad.Uv3.set(0.f, 0.f);

		shape.Quad.V0.set(x,   y,   0);
		shape.Quad.V1.set(x+w, y,   0);
		shape.Quad.V2.set(x+w, y+h, 0);
		shape.Quad.V3.set(x,   y+h, 0);

		shape.Color = bg.color;
		shape.TextureId = rVR.getBlankTextureId();

		m_DrawQueue.push_back(shape);
	}

	// ----------------------------------------------------------------------------
	void CSSBackgroundRenderer::buildImageQuads(const CSSBackground &bg, sint32 textureId)
	{
		// TODO: m_Background should have textureId and that should be "reserved" in CViewRenderer
		// even before download is finished
		if (textureId < 0)
			return;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		sint32 texW = 0;
		sint32 texH = 0;
		rVR.getTextureSizeFromId(textureId, texW, texH);
		if (texW <= 0 || texH <= 0)
			return;

		// get requested texture size
		calculateSize(m_Background, texW, texH);
		if(texW <= 0 || texH <= 0)
			return;

		// get texture left/top corner
		sint32 texX, texY;
		calculatePosition(m_Background, texX, texY, texW, texH);

		sint32 tilesX, tilesY;
		sint32 spacingX, spacingY;
		calculateTiles(m_Background, texX, texY, texW, texH, tilesX, tilesY, spacingX, spacingY);

		sint32 clipL, clipB, clipR, clipT;
		getPaintingArea(m_Background, clipL, clipB, clipR, clipT);
		clipR += clipL;
		clipT += clipB;

		m_DrawQueue.reserve(tilesX * tilesY + m_DrawQueue.size());
		for(sint32 tileX = 0; tileX < tilesX; tileX++)
		{
			for(sint32 tileY = 0; tileY < tilesY; tileY++)
			{
				sint32 tileL = texX + tileX * (texW + spacingX);
				sint32 tileB = texY + tileY * (texH + spacingY);
				sint32 tileR = tileL + texW;
				sint32 tileT = tileB + texH;

				// tile is outside clip area
				if (tileT <= clipB || tileB >= clipT || tileR <= clipL || tileL >= clipR)
					continue;

				CUV uv0(0,1);
				CUV uv1(1,1);
				CUV uv2(1,0);
				CUV uv3(0,0);

				// clip if tile not totally inside clip area
				if (!(tileL >= clipL && tileR <= clipR && tileB >= clipB && tileT <= clipT))
				{
					float ratio;
					if (tileL < clipL)
					{
						ratio = ((float)(clipL - tileL))/((float)(tileR - tileL));
						tileL = clipL;
						uv0.U += ratio*(uv1.U-uv0.U);
						uv3.U += ratio*(uv2.U-uv3.U);
					}

					if (tileB < clipB)
					{
						ratio = ((float)(clipB - tileB))/((float)(tileT - tileB));
						tileB = clipB;
						uv0.V += ratio*(uv3.V-uv0.V);
						uv1.V += ratio*(uv2.V-uv1.V);
					}

					if (tileR > clipR)
					{
						ratio = ((float)(clipR - tileR))/((float)(tileL - tileR));
						tileR = clipR;
						uv2.U += ratio*(uv3.U-uv2.U);
						uv1.U += ratio*(uv0.U-uv1.U);
					}

					if (tileT > clipT)
					{
						ratio = ((float)(clipT - tileT))/((float)(tileB - tileT));
						tileT = clipT;
						uv2.V += ratio*(uv1.V-uv2.V);
						uv3.V += ratio*(uv0.V-uv3.V);
					}
				}

				SDrawQueue shape;
				shape.Quad.Uv0 = uv0;
				shape.Quad.Uv1 = uv1;
				shape.Quad.Uv2 = uv2;
				shape.Quad.Uv3 = uv3;

				shape.Color = CRGBA::White;
				shape.TextureId = textureId;

				shape.Quad.V0.set(tileL, tileB, 0);
				shape.Quad.V1.set(tileR, tileB, 0);
				shape.Quad.V2.set(tileR, tileT, 0);
				shape.Quad.V3.set(tileL, tileT, 0);

				m_DrawQueue.push_back(shape);
			}
		}
	}

}//namespace

