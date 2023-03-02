// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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



#ifndef NL_CSS_BACKGROUND_RENDERER_H
#define NL_CSS_BACKGROUND_RENDERER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/geom_ext.h"
#include "nel/gui/css_types.h"
#include "nel/gui/css_background.h"

namespace NLGUI
{
class CInterfaceElement;

/**
 * \brief Border renderer for GUI classes
 * \date 2021-06-29 15:17 GMT
 * \author Meelis Mägi (Nimetu)
 */
class CSSBackgroundRenderer
{
public:
	// alpha value from parent
	uint8 CurrentAlpha;

	// TODO: should be moved to CSSBackground
	sint32 TextureId;

public:
	CSSBackgroundRenderer();
	~CSSBackgroundRenderer();

	// return true if no background is set
	bool isEmpty() const
	{
		return m_Background.image.empty() && m_Background.color.A == 0;
	}

	void setModulateGlobalColor(bool m) { m_ModulateGlobalColor = m; }

	void updateCoords();
	void invalidateCoords() { m_Dirty = true; }
	void invalidateContent() { m_Dirty = true; };

	void clear();
	void setBackground(const CSSBackground &bg);

	// helper function to change background image
	void setImage(const std::string &bgtex);
	void setImageRepeat(bool b);
	void setImageCover(bool b);

	// helper function to change background color
	void setColor(const NLMISC::CRGBA &color)
	{
		m_Dirty = true;
		m_Background.color = color;
	}

	NLMISC::CRGBA getColor() const
	{
		return m_Background.color;
	}

	// override painting area to be at least the size of viewport (ie, <html> background)
	void setFillViewport(bool b) {
		m_Dirty = true;
		m_FillViewport = b;
	}

	void setViewport(CInterfaceElement *root)
	{
		m_Dirty = true;
		m_Viewport = root;
	}

	void setBorderArea(sint32 x, sint32 y, sint32 w, sint32 h)
	{
		m_Dirty = true;
		m_BorderX = x;
		m_BorderY = y;
		m_BorderW = w;
		m_BorderH = h;
	}

	void setPaddingArea(sint32 x, sint32 y, sint32 w, sint32 h)
	{
		m_Dirty = true;
		m_PaddingX = x;
		m_PaddingY = y;
		m_PaddingW = w;
		m_PaddingH = h;
	}

	void setContentArea(sint32 x, sint32 y, sint32 w, sint32 h)
	{
		m_Dirty = true;
		m_ContentX = x;
		m_ContentY = y;
		m_ContentW = w;
		m_ContentH = h;
	}

	// sizes for em, rem units
	void setFontSize(float rootFontSize, float fontSize)
	{
		m_Dirty = true;
		m_RootFontSize = rootFontSize;
		m_FontSize = fontSize;
	}

	void draw();

private:
	sint32 m_BorderX, m_BorderY, m_BorderW, m_BorderH;
	sint32 m_PaddingX, m_PaddingY, m_PaddingW, m_PaddingH;
	sint32 m_ContentX, m_ContentY, m_ContentW, m_ContentH;

	// font size for 'rem'
	float m_RootFontSize;

	// font size for 'em'
	float m_FontSize;

	// viewport element for vw,wh,vmin,vmax
	CInterfaceElement* m_Viewport;

	struct SDrawQueue
	{
		sint32 TextureId;
		NLMISC::CQuadUV Quad;
		NLMISC::CRGBA Color;
	};
	std::vector<SDrawQueue> m_DrawQueue;

	const sint8 m_RenderLayer;
	bool m_ModulateGlobalColor;
	// if true, painting area returns area at least the size of viewport (ie, <html> background)
	bool m_FillViewport;

	// if true, then updateCoords() is called from draw()
	bool m_Dirty;

	CSSBackground m_Background;

	// get clip area based on background-clip
	void getPaintingArea(const CSSBackground &bg, sint32 &areaX, sint32 &areaY, sint32 &areaW, sint32 &areaH) const;

	// get positioning area based on background-origin
	void getPositioningArea(const CSSBackground &bg, sint32 &areaX, sint32 &areaY, sint32 &areaW, sint32 &areaH) const;

	// calculate image size based on background-size
	void calculateSize(const CSSBackground &bg, sint32 &texW, sint32 &texH) const;

	// calculate image position based on background-position
	void calculatePosition(const CSSBackground &bg, sint32 &texX, sint32 &texY, sint32 &texW, sint32 &texH) const;

	// calculate image tile position, size, count, and spacing based on background-repeat
	void calculateTiles(const CSSBackground &bg, sint32 &texX, sint32 &texY, sint32 &texW, sint32 &texH, sint32 &tilesX, sint32 &tilesY, sint32 &spacingX, sint32 &spacingY) const;

	// position, size, and count for first tile to cover an area
	void getImageTile(sint32 &tilePos, sint32 &tileSize, sint32 &spacing, sint32 &tiles, sint32 areaPos, sint32 areaSize, CSSValueType repeat) const;

	// push background color to draw queue
	void buildColorQuads(const CSSBackground &bg);

	// push background image to draw quque
	void buildImageQuads(const CSSBackground &bg, sint32 textureId);

}; // CSSBackgroundRenderer

}//namespace

#endif // NL_CSS_BACKGROUND_RENDERER_H


