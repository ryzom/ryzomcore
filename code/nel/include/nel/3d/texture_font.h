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

#ifndef NL_TEXTURE_FONT_H
#define NL_TEXTURE_FONT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rect.h"
#include "nel/3d/texture.h"

namespace NL3D
{

class CFontGenerator;

// ****************************************************************************
/**
 * CTextureFont
 */
class CTextureFont : public ITexture
{

public:

	// Holds info for glyphs rendered on atlas
	struct SGlyphInfo
	{
		// font atlas info
		uint32 CacheVersion;

		// atlas region with padding
		uint32 X, Y, W, H;

		// rendered glyph size without padding
		uint32 CharWidth;
		uint32 CharHeight;

		// UV coords for rendered glyph without padding
		float U0, V0, U1, V1;

		uint32 GlyphIndex;
		sint Size;
		bool Embolden;
		bool Oblique;
		CFontGenerator *FontGenerator;

		SGlyphInfo()
			: CacheVersion(0),
			  U0(0.f), V0(0.f), U1(0.f), V1(0.f),
			  X(0), Y(0), W(0), H(0), CharWidth(0), CharHeight(0),
			  GlyphIndex(0), Size(0), Embolden(false), Oblique(false), FontGenerator(NULL)
		{
		}
	};

	// Holds info for glyphs displayed on screen
	struct SLetterInfo
	{
		ucchar Char;
		sint Size;
		bool Embolden;
		bool Oblique;
		CFontGenerator *FontGenerator;

		uint32 GlyphIndex;
		uint32 CharWidth;	// Displayed glyph height
		uint32 CharHeight;	// Displayed glyph height
		sint32 Top;			// Distance between origin and top of the texture
		sint32 Left;		// Distance between origin and left of the texture
		sint32 AdvX;		// Advance to the next caracter

		SGlyphInfo* glyph;

		SLetterInfo()
			: Char(0), Size(0), Embolden(false), Oblique(false), FontGenerator(NULL),
			  GlyphIndex(0), CharWidth(0), CharHeight(0), Top(0), Left(0), AdvX(0),
			  glyph(NULL)
		{
		}
	};

	struct SLetterKey
	{
		ucchar Char;
		sint Size;
		bool Embolden;
		bool Oblique;
		CFontGenerator *FontGenerator;

		// Does not use FontGenerator in return value
		uint32 getVal();

		SLetterKey():Char(0), FontGenerator(NULL), Size(0), Embolden(false), Oblique(false)
		{
		}
	};

public:

	/**
	 * Default constructor
	 */
	CTextureFont();
	virtual ~CTextureFont();

	// Generate the texture
	void doGenerate (bool async = false);

	// This function manage the cache if the letter wanted does not exist
	// \param render Set to true if letter is currently visible on screen
	SLetterInfo* getLetterInfo (SLetterKey& k, bool render);

	void dumpTextureFont (const char *filename);

	// Version is increased with each rebuild of font atlas
	uint32 getCacheVersion() const { return _CacheVersion; }

private:
	uint32 _CacheVersion;

	// current texture size
	uint32 _TextureSizeX;
	uint32 _TextureSizeY;

	// maximum texture size allowed
	uint32 _TextureMaxW;
	uint32 _TextureMaxH;

	// padding around glyphs
	uint8 _PaddingL, _PaddingT;
	uint8 _PaddingR, _PaddingB;

	// To find a letter in the texture

	// Keep track of available space in main texture
	std::vector<NLMISC::CRect> _AtlasNodes;

	std::vector <SLetterInfo> _Letters;

	// lookup letter from letter cache or create new
	SLetterInfo* findLetter(SLetterKey& k, bool insert);

	// lower/upper bound of glyphs to render, sizes outside are scaled bitmaps
	uint _MinGlyphSize;
	uint _MaxGlyphSize;
	// start using size stem from this font size
	uint _GlyphSizeStepMin;
	// every n'th font size is rendered, intermediates are using bitmap scaling
	uint _GlyphSizeStep;

	// rendered glyph cache
	std::list<SGlyphInfo> _GlyphCache;
	SGlyphInfo* findLetterGlyph(SLetterInfo *letter, bool insert);

	// render letter glyph into glyph cache
	SGlyphInfo* renderLetterGlyph(SLetterInfo *letter, uint32 bitmapFontSize);

	// copy glyph bitmap into texture and invalidate that region
	void copyGlyphBitmap(uint8* bitmap, uint32 bitmapW, uint32 bitmapH, uint32 atlasX, uint32 atlasY);

	// Find best fit for WxH rect in atlas
	uint fitRegion(uint index, uint width, uint height);

	// Return top/left from font texture or false if there is no more room
	bool reserveAtlas(const uint32 width, const uint32 height, uint32 &x, uint32 &y);

	// repack glyphs, resize texture, and invalidate unused glyphs.
	void repackAtlas();
	void repackAtlas(uint32 width, uint32 height);

	// resize texture,
	bool resizeAtlas();

	// remove all glyphs from atlas, clear glyph cache, letter info is kept
	void clearAtlas();

	// if return true: newW, newH contains next size font atlas should be resized
	// if return false: _TextureMaxW and _TextureMaxH is reached
	bool getNextTextureSize(uint32 &newW, uint32 &newH) const;

	/// Todo: serialize a font texture.
public:
	virtual void	serial(NLMISC::IStream &/* f */) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureFont);

};


} // NL3D


#endif // NL_TEXTURE_FONT_H

/* End of texture_font.h */
