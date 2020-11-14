// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <string>
//#include <sstream>

#include "nel/3d/font_manager.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/texture_font.h"
#include "nel/3d/computed_string.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/material.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"

#include "nel/misc/file.h"

using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {



// ***************************************************************************
CMaterial* CFontManager::getFontMaterial()
{
	if (_TexFont == NULL)
	{
		_TexFont = new CTextureFont;
		_TexCacheNr++;
	}

	if (_MatFont == NULL)
	{
		_MatFont= new CMaterial;
		_MatFont->initUnlit();
		_MatFont->setSrcBlend(CMaterial::srcalpha);
		_MatFont->setDstBlend(CMaterial::invsrcalpha);
		_MatFont->setBlend(true);
		_MatFont->setTexture(0, _TexFont);
		_MatFont->texEnvOpRGB(0, CMaterial::Replace);
		_MatFont->texEnvArg0RGB(0, CMaterial::Diffuse, CMaterial::SrcColor);
	}
	return _MatFont;
}

// ***************************************************************************
void CFontManager::computeString (NLMISC::CUtfStringView sv,
								  CFontGenerator *fontGen,
								  const NLMISC::CRGBA &color,
								  uint32 fontSize,
								  bool embolden,
								  bool oblique,
								  IDriver *driver,
								  CComputedString &output,
								  bool	keep800x600Ratio)
{
	output.Color = color;

	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		uint32 width, height;
		driver->getWindowSize (width, height);
		if ((height == 0) || (width == 0))
			return;

		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}

	// Setting vertices format
	output.Vertices.setNumVertices (4 * (uint32)sv.largestSize());

	// 1 character <-> 1 quad
	sint32 penx = 0, dx;
	sint32 penz = 0, dz;
	float x1, z1, x2, z2;
	float u1, v1, u2, v2;
	CMaterial		*pMatFont = getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));
	float TexRatioW = 1.0f / pTexFont->getWidth();
	float TexRatioH = 1.0f / pTexFont->getHeight();
	/*float hlfPixTexW = 0.5f * TexRatioW;
	float hlfPixTexH = 0.5f * TexRatioH;
	float hlfPixScrW = 0.5f;
	float hlfPixScrH = 0.5f;*/
	// Yoyo: Do not need Half Pixel/Texel displacement!!
	float hlfPixTexW = 0;
	float hlfPixTexH = 0;
	float hlfPixScrW = 0;
	float hlfPixScrH = 0;


	CTextureFont::SLetterKey k;

	// string bound.
	output.XMin= FLT_MAX;
	output.XMax= -FLT_MAX;
	output.ZMin= FLT_MAX;
	output.ZMax= -FLT_MAX;

	// string info.
	sint32 nMaxZ = -1000000, nMinZ = 1000000;
	output.StringHeight = 0;

	// save string info for later rebuild as needed
	if (sv.ptr() != output.Text.c_str()) // don't resave if rebuilding
		output.Text = sv.toUtf8();
	output.CacheVersion = getCacheVersion();

	uint j = 0;
	size_t idx = 0;
	{
		CVertexBufferReadWrite vba;
		output.Vertices.lock (vba);

		hlfPixScrW = 0.f;
		hlfPixScrH = 0.f;

		// For all chars
		for (NLMISC::CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it, ++idx)
		{
			// Creating font
			k.Char = *it;
			if (k.Char < 0x20) // Control Characters
				k.Char += 0x2400;
			if (k.Char == 0x7F) // DEL
				k.Char = 0x2421;
			k.FontGenerator = fontGen;
			k.Size = fontSize;
			k.Embolden = embolden;
			k.Oblique = oblique;
			// render letter
			CTextureFont::SLetterInfo *pLI = pTexFont->getLetterInfo (k, true);
			if(pLI != NULL)
			{
				if (pLI->glyph)
				{
					// If letter is heavily upscaled, then there is noticeable clipping on edges
					// fixing UV will make it bit better
					if ((pLI->Size >> 1) > pLI->glyph->Size)
					{
						hlfPixTexW = 0.5f * TexRatioW;
						hlfPixTexH = 0.5f * TexRatioH;
					}

					// Creating vertices
					dx = pLI->Left;
					dz = -((sint32)pLI->CharHeight - (sint32)(pLI->Top));

					x1 = (penx + dx) - hlfPixScrW;
					z1 = (penz + dz) - hlfPixScrH;
					x2 = (penx + dx + (sint32)pLI->CharWidth) + hlfPixScrW;
					z2 = (penz + dz + (sint32)pLI->CharHeight) + hlfPixScrH;

					vba.setVertexCoord	(j, x1, 0, z1);
					vba.setTexCoord		(j, 0, pLI->glyph->U0-hlfPixTexW, pLI->glyph->V1+hlfPixTexH);
					++j;

					vba.setVertexCoord	(j, x2, 0, z1);
					vba.setTexCoord		(j, 0, pLI->glyph->U1+hlfPixTexW, pLI->glyph->V1+hlfPixTexH);
					++j;

					vba.setVertexCoord	(j, x2, 0, z2);
					vba.setTexCoord		(j, 0, pLI->glyph->U1+hlfPixTexW, pLI->glyph->V0-hlfPixTexH);
					++j;

					vba.setVertexCoord	(j, x1, 0, z2);
					vba.setTexCoord		(j, 0, pLI->glyph->U0-hlfPixTexW, pLI->glyph->V0-hlfPixTexH);
					++j;

					// String Bound
					output.XMin= min(output.XMin, x1);
					output.XMin= min(output.XMin, x2);
					output.XMax= max(output.XMax, x1);
					output.XMax= max(output.XMax, x2);
					output.ZMin= min(output.ZMin, z1);
					output.ZMin= min(output.ZMin, z2);
					output.ZMax= max(output.ZMax, z1);
					output.ZMax= max(output.ZMax, z2);

					// String info
					sint32	nZ1 = (sint32)pLI->Top-(sint32)pLI->CharHeight;
					sint32	nZ2 = pLI->Top;

					if (nZ1 < nMinZ) nMinZ = nZ1;
					if (nZ2 > nMaxZ) nMaxZ = nZ2;
				}
				penx += pLI->AdvX;
			}

			// Building Material
			output.Material = pMatFont;
		}
	}
	output.Vertices.setNumVertices (j);
	output.Length = idx;
	nlassert(output.Length == NLMISC::CUtfStringView(output.Text).count());

	// compile string info
	output.StringWidth = (float)penx;
	if(nMaxZ>nMinZ)
	{
		output.StringHeight = (float)(nMaxZ - nMinZ);
		output.StringLine = -(float)nMinZ;
	}
	else
	{
		output.StringHeight = 0;
		output.StringLine = 0;
	}
}


// ***************************************************************************
void CFontManager::computeStringInfo (	NLMISC::CUtfStringView sv,
										CFontGenerator *fontGen,
										const NLMISC::CRGBA &color,
										uint32 fontSize,
										bool embolden,
										bool oblique,
										IDriver *driver,
										CComputedString &output,
										bool keep800x600Ratio	)
{
	computeStringInfo(sv, sv.largestSize(), fontGen, color, fontSize, embolden, oblique, driver, output, keep800x600Ratio);
}


// ***************************************************************************
void CFontManager::computeStringInfo (	NLMISC::CUtfStringView sv,
										size_t len,
										CFontGenerator *fontGen,
										const NLMISC::CRGBA &color,
										uint32 fontSize,
										bool embolden,
										bool oblique,
										IDriver *driver,
										CComputedString &output,
										bool keep800x600Ratio	)
{
	output.Color = color;

	// save string info for later rebuild as needed
	if (sv.ptr() != output.Text.c_str()) // don't resave if rebuilding
		output.Text = sv.toUtf8();
	output.CacheVersion = 0;

	if (sv.empty())
	{
		output.StringWidth = 0.f;
		output.StringHeight = 0;
		output.StringLine = 0;

		return;
	}

	// resize fontSize if window not of 800x600.
	if (keep800x600Ratio)
	{
		uint32 width, height;
		driver->getWindowSize (width, height);
		if ((height == 0) || (width == 0))
			return;
		// keep the 800*600 ratio
		fontSize = (uint32)floor(fontSize*height/600.f);
		fontSize = max(fontSize, (uint32)2);
	}

	sint32 penx = 0;
	sint32 nMaxZ = -1000000, nMinZ = 1000000;
	CMaterial		*pMatFont = getFontMaterial();
	CTextureFont	*pTexFont = (CTextureFont*)(pMatFont->getTexture (0));

	CTextureFont::SLetterKey k;
	CTextureFont::SLetterInfo *pLI;

	size_t idx = 0;
	for (NLMISC::CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end && idx < len; ++it, ++idx)
	{
		// Creating font
		k.Char = *it;
		if (k.Char < 0x20)
			k.Char += 0x2400;
		k.FontGenerator = fontGen;
		k.Size = fontSize;
		k.Embolden = embolden;
		k.Oblique = oblique;
		pLI = pTexFont->getLetterInfo (k, false);
		if(pLI != NULL)
		{
			if ((pLI->CharWidth > 0) && (pLI->CharHeight > 0))
			{
				// String info
				sint32	nZ1 = (sint32)pLI->Top-(sint32)pLI->CharHeight;
				sint32	nZ2 = pLI->Top;

				if (nZ1 < nMinZ) nMinZ = nZ1;
				if (nZ2 > nMaxZ) nMaxZ = nZ2;
			}
			penx += pLI->AdvX;
		}
	}
	output.Length = idx;
	nlassert(output.Length == std::min(len, NLMISC::CUtfStringView(output.Text).count()));

	// compile string info
	output.StringWidth = (float)penx;
	if(nMaxZ>nMinZ)
	{
		output.StringHeight = (float)(nMaxZ - nMinZ);
		output.StringLine = -(float)nMinZ;
	}
	else
	{
		output.StringHeight = 0;
		output.StringLine = 0;
	}

}


// ***************************************************************************
string CFontManager::getCacheInformation() const
{
	string str;
	str = "MaxMemory: " + NLMISC::toString(_MaxMemory) + " MemSize: " + NLMISC::toString(_MemSize) + " NbChar: " + NLMISC::toString(_NbChar);
	return str;
}

// ***************************************************************************
void CFontManager::invalidate()
{
	if (_TexFont)
		_TexFont = NULL;

	_TexFont = new CTextureFont;
	_TexCacheNr++;

	getFontMaterial()->setTexture(0, _TexFont);
}



} // NL3D
