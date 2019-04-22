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

#include "std3d.h"
#include <map>

#include "nel/3d/texture_font.h"
#include "nel/3d/font_generator.h"

#include "nel/misc/common.h"
#include "nel/misc/rect.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ---------------------------------------------------------------------------
CTextureFont::CTextureFont()
	: _CacheVersion(1),
	_TextureSizeX(512), _TextureSizeY(512), _TextureMaxW(4096), _TextureMaxH(4096),
	_PaddingL(0), _PaddingT(0), _PaddingR(1), _PaddingB(1),
	_MinGlyphSize(5), _MaxGlyphSize(200),
	_GlyphSizeStepMin(50), _GlyphSizeStep(5)
{
	setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);

	setWrapS (ITexture::Repeat);
	setWrapT (ITexture::Repeat);

	setUploadFormat (Alpha);

	setReleasable (false);

	resize (_TextureSizeX, _TextureSizeY, CBitmap::Alpha, true);

	_AtlasNodes.push_back(CRect(0, 0, _TextureSizeX, _TextureSizeY));
}


CTextureFont::~CTextureFont()
{
}

// ---------------------------------------------------------------------------
void CTextureFont::dumpTextureFont(const char *filename)
{
	CBitmap b;
	COFile f( filename );
	b.resize (_TextureSizeX, _TextureSizeY, CBitmap::RGBA);
	CObjectVector<uint8>&bits = b.getPixels();
	CObjectVector<uint8>&src = getPixels();

	for (uint i = 0; i < (_TextureSizeX*_TextureSizeY); ++i)
	{
		bits[i*4+0] = bits[i*4+1] = bits[i*4+2] = bits[i*4+3] = src[i];
	}

	b.writeTGA (f, 32);
}

// ---------------------------------------------------------------------------
bool CTextureFont::getNextTextureSize(uint32 &newW, uint32 &newH) const
{
	// width will be resized first (256x256 -> 512x256)
	if (_TextureSizeX <= _TextureSizeY)
	{
		newW = _TextureSizeX * 2;
		newH = _TextureSizeY;
	}
	else
	{
		newW = _TextureSizeX;
		newH = _TextureSizeY * 2;
	}

	// no more room
	return newW <= _TextureMaxW && newH <= _TextureMaxH;
}

// ---------------------------------------------------------------------------
// out of room, clear everything and rebuild glyphs on demand
// note: text will display wrong until glyphs get rendered again
void CTextureFont::clearAtlas()
{
	nlwarning("Glyph cache will be cleared.");

	_AtlasNodes.clear();
	_AtlasNodes.push_back(CRect(0, 0, _TextureSizeX, _TextureSizeY));

	// clear texture
	_Data[0].fill(0);

	// clear glyph cache
	for(uint i = 0; i< _Letters.size(); ++i)
	{
		_Letters[i].glyph = NULL;
	}
	_GlyphCache.clear();

	_CacheVersion++;

	touch();
}

// ---------------------------------------------------------------------------
void CTextureFont::repackAtlas()
{
	repackAtlas(_TextureSizeX, _TextureSizeY);
}

// ---------------------------------------------------------------------------
// backup old glyphs and move them to newly resized texture
// new atlas will be sorted if _GlyphCache is
void CTextureFont::repackAtlas(uint32 newW, uint32 newH)
{
	uint32 newCacheVersion = _CacheVersion+1;

	CBitmap btm;
	uint32 oldW, oldH;

	oldW = _TextureSizeX;
	oldH = _TextureSizeY;
	btm.resize(oldW, oldH, CBitmap::Alpha, true);
	btm.blit(this, 0, 0);

	// resize texture
	if (_TextureSizeX != newW || _TextureSizeY != newH)
	{
		_TextureSizeX = newW;
		_TextureSizeY = newH;
		resize (_TextureSizeX, _TextureSizeY, CBitmap::Alpha, true);
	}
	else
	{
		_Data[0].fill(0);
	}

	// release atlas and rebuild
	_AtlasNodes.clear();
	_AtlasNodes.push_back(CRect(0, 0, _TextureSizeX, _TextureSizeY));

	CObjectVector<uint8>&src = btm.getPixels();
	for(std::list<SGlyphInfo>::iterator it = _GlyphCache.begin(); it != _GlyphCache.end(); ++it)
	{
		if (it->CacheVersion != _CacheVersion)
		{
			// TODO: must remove glyph from all letters before removing glyph from cache
			//continue;
		}

		SGlyphInfo &glyph = *it;

		glyph.CacheVersion = newCacheVersion;

		uint32 atlasX, atlasY;
		if (reserveAtlas(glyph.W, glyph.H, atlasX, atlasY))
		{
			for (uint y = 0; y < glyph.H; ++y)
			{
				uint8 *pDst = &_Data[0][(atlasY + y) * _TextureSizeX + atlasX];
				for (uint x = 0; x < glyph.W; ++x)
				{
					*pDst = src[(glyph.Y + y) * oldW + glyph.X + x];
					++pDst;
				}
			}

			// TODO: dup code with renderGlyph
			glyph.U0 = (atlasX+_PaddingL) / (float)_TextureSizeX;
			glyph.V0 = (atlasY+_PaddingT) / (float)_TextureSizeY;
			glyph.U1 = (atlasX+_PaddingL+glyph.CharWidth) / (float)_TextureSizeX;
			glyph.V1 = (atlasY+_PaddingT+glyph.CharHeight) / (float)_TextureSizeY;

			glyph.X = atlasX;
			glyph.Y = atlasY;
		}
	}

	_CacheVersion = newCacheVersion;

	// invalidate full texture
	touch();
}

// ---------------------------------------------------------------------------
bool CTextureFont::resizeAtlas()
{
	uint32 newW, newH;
	if (!getNextTextureSize(newW, newH))
	{
		nlwarning("Font texture at maximum (%d,%d). Resize failed.", _TextureSizeX, _TextureSizeY);
		return false;
	}

	// resize and redraw
	repackAtlas(newW, newH);
	return true;
}

// ---------------------------------------------------------------------------
void CTextureFont::doGenerate(bool async)
{
	/*
	nlinfo("doGenerate: Letters(%d/%d), Glyphs(%d/%d)\n", _Letters.size(), _Letters.size() * sizeof(SLetterInfo),
			_GlyphCache.size(), _GlyphCache.size() * sizeof(SGlyphInfo));
	//std::string fname = CFile::findNewFile("/tmp/font-texture.tga");
	std::string fname = toString("/tmp/font-texture-%p-%03d.tga", this, _CacheVersion);
	dumpTextureFont (fname.c_str());
	*/
}

// ---------------------------------------------------------------------------
uint CTextureFont::fitRegion(uint index, uint width, uint height)
{
	if (_AtlasNodes[index].X + width > _TextureSizeX - 1)
	{
		return -1;
	}

	uint x = _AtlasNodes[index].X;
	uint y = _AtlasNodes[index].Y;
	sint widthLeft = width;

	while(widthLeft > 0)
	{
		if (_AtlasNodes[index].Y > y)
		{
			y = _AtlasNodes[index].Y;
		}

		// _AtlasNodes[0] for margin is not used here
		if (_AtlasNodes[index].Y + height > _TextureSizeY - 1)
		{
			return -1;
		}

		widthLeft -= _AtlasNodes[index].Width;
		index++;
	}

	return y;
}

bool CTextureFont::reserveAtlas(const uint32 width, const uint32 height, uint32 &x, uint32 &y)
{
	if (_AtlasNodes.empty())
	{
		nlwarning("No available space in texture atlas (_AtlasNodes.empty() == true)");
		return false;
	}

	x = 0;
	y = 0;

	sint bestIndex = -1;
	sint bestWidth = _TextureSizeX;
	sint bestHeight = _TextureSizeY;

	sint selY=0;

	for (uint i = 0; i < _AtlasNodes.size(); ++i)
	{
		selY = fitRegion(i, width, height);
		if (selY >=0)
		{
			if (((selY + height) < bestHeight) || ((selY + height) == bestHeight && _AtlasNodes[i].Width > 0 && _AtlasNodes[i].Width < bestWidth))
			{
				bestHeight = selY + height;
				bestIndex = i;
				bestWidth = _AtlasNodes[i].Width;
				x = _AtlasNodes[i].X;
				y = selY;
			}
		}
	}

	if (bestIndex == -1)
	{
		x = 0;
		y = 0;
		return false;
	}

	CRect r(x, y + height, width, 0);
	_AtlasNodes.insert(_AtlasNodes.begin() + bestIndex, r);

	// shrink or remove nodes overlaping with newly inserted node
	for(uint i = bestIndex+1; i< _AtlasNodes.size(); i++)
	{
		if (_AtlasNodes[i].X < (_AtlasNodes[i-1].X + _AtlasNodes[i-1].Width))
		{
			sint shrink = _AtlasNodes[i-1].X + _AtlasNodes[i-1].Width - _AtlasNodes[i].X;
			_AtlasNodes[i].X += shrink;
			if (_AtlasNodes[i].Width > shrink)
			{
				_AtlasNodes[i].Width -= shrink;
				break;
			}
			_AtlasNodes.erase(_AtlasNodes.begin() + i);
			i--;
		}
		else break;
	}

	// merge nearby nodes from same row
	for(uint i = 0; i < _AtlasNodes.size() - 1; i++)
	{
		if (_AtlasNodes[i].Y == _AtlasNodes[i+1].Y)
		{
			_AtlasNodes[i].Width += _AtlasNodes[i+1].Width;
			_AtlasNodes.erase(_AtlasNodes.begin() + i + 1);
			i--;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// bitmap : texture data
// bitmapW : bitmap width
// bitmapH : bitmap height
// atlasX : pos x in font texture
// atlasY : pos y in font texture
void CTextureFont::copyGlyphBitmap(uint8* bitmap, uint32 bitmapW, uint32 bitmapH, uint32 atlasX, uint32 atlasY)
{
	for (uint bY = 0; bY < bitmapH; ++bY)
	{
		uint8 *pDst = &_Data[0][(atlasY+_PaddingT+bY) * _TextureSizeX+atlasX+_PaddingL];
		for (uint bX = 0; bX < bitmapW; ++bX)
		{
			*pDst = bitmap[bY * bitmapW+bX];
			++pDst;
		}
	}

	if (_PaddingR > 0 || _PaddingB > 0 || _PaddingL > 0 || _PaddingT > 0)
	{
		for(uint i = 0; i<(bitmapH+_PaddingT+_PaddingB); ++i)
		{
			if (_PaddingT > 0) _Data[0][(atlasY + i) * _TextureSizeX + atlasX    ] = 0;
			if (_PaddingB > 0) _Data[0][(atlasY + i) * _TextureSizeX + atlasX + _PaddingL + bitmapW] = 0;
		}

		for (uint i = 0; i<(bitmapW+_PaddingL+_PaddingR); ++i)
		{
			if (_PaddingL > 0) _Data[0][atlasY * _TextureSizeX + atlasX + i] = 0;
			if (_PaddingB > 0) _Data[0][(atlasY + _PaddingT + bitmapH) * _TextureSizeX + atlasX + i] = 0;
		}
	}

	CRect r(atlasX, atlasY, bitmapW + _PaddingL + _PaddingR, bitmapH + _PaddingT + _PaddingB);
	touchRect(r);
}


// ---------------------------------------------------------------------------
CTextureFont::SGlyphInfo* CTextureFont::renderLetterGlyph(SLetterInfo *letter, uint bitmapFontSize)
{
	uint32 nPitch;
	sint32 left;
	sint32 top;
	sint32 advx;
	uint32 charWidth;
	uint32 charHeight;
	uint32 glyphIndex;

	uint8 *bitmap = letter->FontGenerator->getBitmap (letter->Char, bitmapFontSize, letter->Embolden, letter->Oblique,
													charWidth, charHeight,
													nPitch, left, top,
													advx, glyphIndex );

	uint32 atlasX, atlasY;
	uint32 rectW, rectH;
	rectW = charWidth + _PaddingL + _PaddingR;
	rectH = charHeight + _PaddingT + _PaddingB;

	if (!reserveAtlas(rectW, rectH, atlasX, atlasY))
	{
		// no room
		return NULL;
	}
	copyGlyphBitmap(bitmap, charWidth, charHeight, atlasX, atlasY);

	SGlyphInfo* glyphInfo = NULL;
	{
		// keep cache sorted by height (smaller first)
		std::list<SGlyphInfo>::iterator it = _GlyphCache.begin();
		while(it != _GlyphCache.end() && it->CharHeight < charHeight)
		{
			++it;
		}

		it = _GlyphCache.insert(it, SGlyphInfo());
		glyphInfo = &(*it);
	}

	glyphInfo->GlyphIndex = glyphIndex;
	glyphInfo->Size = bitmapFontSize;
	glyphInfo->Embolden = letter->Embolden;
	glyphInfo->Oblique = letter->Oblique;
	glyphInfo->FontGenerator = letter->FontGenerator;
	glyphInfo->CacheVersion = _CacheVersion;

	glyphInfo->U0 = (atlasX+_PaddingL) / (float)_TextureSizeX;
	glyphInfo->V0 = (atlasY+_PaddingT) / (float)_TextureSizeY;
	glyphInfo->U1 = (atlasX+_PaddingL+charWidth) / (float)_TextureSizeX;
	glyphInfo->V1 = (atlasY+_PaddingT+charHeight) / (float)_TextureSizeY;

	glyphInfo->CharWidth = charWidth;
	glyphInfo->CharHeight = charHeight;

	glyphInfo->X = atlasX;
	glyphInfo->Y = atlasY;
	glyphInfo->W = rectW;
	glyphInfo->H = rectH;

	return glyphInfo;
}


// ---------------------------------------------------------------------------
CTextureFont::SGlyphInfo* CTextureFont::findLetterGlyph(SLetterInfo *letter, bool insert)
{
	uint bitmapFontSize = max((sint)_MinGlyphSize, min((sint)_MaxGlyphSize, letter->Size));
	if (_GlyphSizeStep > 1 && bitmapFontSize > _GlyphSizeStepMin)
	{
		bitmapFontSize = (bitmapFontSize / _GlyphSizeStep) * _GlyphSizeStep;
	}

	// CacheVersion not checked, all glyphs in cache must be rendered on texture
	for(std::list<SGlyphInfo>::iterator it = _GlyphCache.begin(); it != _GlyphCache.end(); ++it)
	{
		if (it->GlyphIndex == letter->GlyphIndex &&
			it->Size == bitmapFontSize &&
			it->Embolden == letter->Embolden &&
			it->Oblique == letter->Oblique &&
			it->FontGenerator == letter->FontGenerator)
		{
			return &(*it);
		}
	}

	if (insert)
	{
		return renderLetterGlyph(letter, bitmapFontSize);
	}

	return NULL;
}

// ---------------------------------------------------------------------------
CTextureFont::SLetterInfo* CTextureFont::findLetter(SLetterKey &k, bool insert)
{
	// TODO: use std::map<uint64>
	for(uint i = 0; i < _Letters.size(); ++i)
	{
		if (_Letters[i].Char == k.Char && _Letters[i].Size == k.Size &&
			_Letters[i].Embolden == k.Embolden && _Letters[i].Oblique == k.Oblique &&
			_Letters[i].FontGenerator == k.FontGenerator)
		{
			return &_Letters[i];
		}
	}

	if (insert)
	{
		_Letters.push_back(SLetterInfo());
		SLetterInfo* letter = &_Letters.back();

		// get metrics for requested size
		letter->Char = k.Char;
		letter->Size = k.Size;
		letter->Embolden = k.Embolden;
		letter->Oblique = k.Oblique;
		letter->FontGenerator = k.FontGenerator;

		uint32 nPitch;
		letter->FontGenerator->getBitmap(letter->Char, letter->Size, letter->Embolden, letter->Oblique,
										letter->CharWidth, letter->CharHeight,
										nPitch, letter->Left, letter->Top,
										letter->AdvX, letter->GlyphIndex );

		return letter;
	}

	return NULL;
}

// ---------------------------------------------------------------------------
CTextureFont::SLetterInfo* CTextureFont::getLetterInfo (SLetterKey& k, bool render)
{
	// find already cached letter or create new one
	SLetterInfo* letter = findLetter(k, true);
	// letter not found (=NULL) or render not requested
	if (!letter || !render) return letter;

	// nothing to render, ie space char
	if (letter->CharWidth == 0 || letter->CharHeight == 0) return letter;

	if (!letter->glyph || letter->glyph->CacheVersion != _CacheVersion)
	{
		// render glyph
		letter->glyph = findLetterGlyph(letter, true);
		if (letter->glyph == NULL)
		{
			// resize/repack and try again
			if (!resizeAtlas()) repackAtlas();

			letter->glyph = findLetterGlyph(letter, true);
			if (letter->glyph == NULL)
			{
				// make room by clearing all glyphs and reduce max size for glyphs
				clearAtlas();
				if (_MaxGlyphSize > _MinGlyphSize)
				{
					_MaxGlyphSize = max(_MinGlyphSize, _MaxGlyphSize - 10);
				}

				letter->glyph = findLetterGlyph(letter, true);
			}
		}
	}

	return letter;
}

} // NL3D
