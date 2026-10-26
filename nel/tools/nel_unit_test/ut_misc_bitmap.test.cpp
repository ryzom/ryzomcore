// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

#include <nel/misc/bitmap.h>
#include <nel/misc/common.h>
#include <nel/misc/mem_stream.h>

using NLMISC::CBitmap;
using NLMISC::CMemStream;

// The image files are built in memory: the CBitmap writers produce a small valid image, and the tests patch its header
// the way a hostile file (a web download) would.

namespace {

typedef std::vector<uint8> TBytes;

enum TFormat
{
	FormatTGA,
	FormatPNG,
	FormatJPG
};

TBytes writeImage(TFormat format, sint32 width, sint32 height)
{
	CBitmap bitmap;
	bitmap.resize(width, height, CBitmap::RGBA);
	uint8 *pixels = bitmap.getPixels().getPtr();
	for (sint32 i = 0; i < width * height * 4; ++i)
		pixels[i] = (uint8)(i * 7);

	CMemStream stream(false);
	bool written = false;
	switch (format)
	{
	case FormatTGA: written = bitmap.writeTGA(stream, 32); break;
	case FormatPNG: written = bitmap.writePNG(stream, 32); break;
	case FormatJPG: written = bitmap.writeJPG(stream); break;
	}
	EXPECT_TRUE(written);
	return TBytes(stream.buffer(), stream.buffer() + stream.length());
}

uint8 loadImage(CBitmap &bitmap, const TBytes &bytes)
{
	CMemStream stream(true);
	stream.fill(&bytes[0], (uint32)bytes.size());
	return bitmap.load(stream);
}

void putBE16(TBytes &bytes, size_t pos, uint32 value)
{
	bytes[pos] = (uint8)(value >> 8);
	bytes[pos + 1] = (uint8)value;
}

void putBE32(TBytes &bytes, size_t pos, uint32 value)
{
	putBE16(bytes, pos, value >> 16);
	putBE16(bytes, pos + 2, value & 0xffff);
}

void putLE16(TBytes &bytes, size_t pos, uint32 value)
{
	bytes[pos] = (uint8)value;
	bytes[pos + 1] = (uint8)(value >> 8);
}

void putLE32(TBytes &bytes, size_t pos, uint32 value)
{
	putLE16(bytes, pos, value & 0xffff);
	putLE16(bytes, pos + 2, value >> 16);
}

// the PNG chunk CRC, so that a patched header is not refused for its checksum
uint32 crc32(const uint8 *data, size_t length)
{
	uint32 crc = 0xffffffff;
	for (size_t i = 0; i < length; ++i)
	{
		crc ^= data[i];
		for (uint bit = 0; bit < 8; ++bit)
			crc = (crc >> 1) ^ (0xedb88320 & (0 - (crc & 1)));
	}
	return ~crc;
}

// size of the IHDR chunk of a PNG: 8 signature bytes, then length, type, 13 data bytes and the CRC
void setPNGSize(TBytes &png, uint32 width, uint32 height)
{
	ASSERT_EQ(0, memcmp(&png[12], "IHDR", 4));
	putBE32(png, 16, width);
	putBE32(png, 20, height);
	putBE32(png, 29, crc32(&png[12], 17));
}

// size in the baseline frame header (SOF0) of a JPEG
void setJPGSize(TBytes &jpg, uint32 width, uint32 height)
{
	for (size_t i = 2; i + 9 < jpg.size(); ++i)
	{
		if (jpg[i] == 0xff && jpg[i + 1] == 0xc0)
		{
			putBE16(jpg, i + 5, height);
			putBE16(jpg, i + 7, width);
			return;
		}
	}
	FAIL() << "no SOF0 marker";
}

void setTGASize(TBytes &tga, uint32 width, uint32 height)
{
	putLE16(tga, 12, width);
	putLE16(tga, 14, height);
}

// a DDS of one DXT1 level, with the header values given
TBytes makeDDS(uint32 headerSize, uint32 width, uint32 height, uint32 mipMaps)
{
	TBytes dds(4 + 124, 0);
	memcpy(&dds[0], "DDS ", 4);
	putLE32(dds, 4, headerSize);
	putLE32(dds, 4 + 1 * 4, DDSD_LINEARSIZE);
	putLE32(dds, 4 + 2 * 4, height);
	putLE32(dds, 4 + 3 * 4, width);
	putLE32(dds, 4 + 6 * 4, mipMaps);
	memcpy(&dds[4 + 20 * 4], "DXT1", 4);
	// the blocks of the first level, 8 bytes per 4x4 block
	dds.resize(dds.size() + ((width + 3) / 4) * ((height + 3) / 4) * 8, 0x55);
	return dds;
}

} // namespace

TEST(CUTMiscBitmap, validImagesLoad)
{
	const TFormat formats[] = { FormatTGA, FormatPNG, FormatJPG };
	for (uint i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i)
	{
		SCOPED_TRACE(i);
		CBitmap bitmap;
		EXPECT_NE(0, loadImage(bitmap, writeImage(formats[i], 16, 8)));
		EXPECT_EQ(16u, bitmap.getWidth());
		EXPECT_EQ(8u, bitmap.getHeight());

		// the lossless formats read back the pixels written
		if (formats[i] != FormatJPG)
		{
			ASSERT_EQ(CBitmap::RGBA, bitmap.getPixelFormat());
			const uint8 *pixels = bitmap.getPixels().getPtr();
			uint mismatches = 0;
			for (uint p = 0; p < 16 * 8 * 4; ++p)
				mismatches += pixels[p] != (uint8)(p * 7);
			EXPECT_EQ(0u, mismatches);
		}
	}

	CBitmap dds;
	EXPECT_NE(0, loadImage(dds, makeDDS(124, 8, 8, 1)));
	EXPECT_EQ(8u, dds.getWidth());
	EXPECT_EQ(8u, dds.getHeight());
}

// 16388x8192 RGBA is 2^27 + 2^15 pixels: the 32-bit size computation wrapped to a 128 KiB buffer, which the readers
// then wrote 512 MiB into. A header larger than MAX_BITMAP_SIZE is now refused before anything is allocated.
TEST(CUTMiscBitmap, oversizedHeadersAreRefused)
{
	{
		SCOPED_TRACE("jpg");
		TBytes jpg = writeImage(FormatJPG, 16, 8);
		setJPGSize(jpg, 16388, 8192);
		CBitmap bitmap;
		EXPECT_EQ(0, loadImage(bitmap, jpg));
	}
	{
		SCOPED_TRACE("png");
		TBytes png = writeImage(FormatPNG, 16, 8);
		setPNGSize(png, 16388, 8192);
		CBitmap bitmap;
		EXPECT_EQ(0, loadImage(bitmap, png));
	}
	{
		SCOPED_TRACE("tga");
		TBytes tga = writeImage(FormatTGA, 16, 8);
		setTGASize(tga, 16388, 8192);
		CBitmap bitmap;
		EXPECT_EQ(0, loadImage(bitmap, tga));
	}
	{
		SCOPED_TRACE("dds");
		CBitmap bitmap;
		EXPECT_THROW(loadImage(bitmap, makeDDS(124, 8, NLMISC::MAX_BITMAP_SIZE + 4, 1)), NLMISC::EStream);
	}
	{
		// a logical screen of 65535x65535: its pixel count overflowed the int product of the size check (refused
		// without GIF support too)
		SCOPED_TRACE("gif");
		TBytes gif(13, 0);
		memcpy(&gif[0], "GIF89a", 6);
		putLE16(gif, 6, 65535);
		putLE16(gif, 8, 65535);
		gif.push_back(0x3b);
		CBitmap bitmap;
		EXPECT_EQ(0, loadImage(bitmap, gif));
	}
}

TEST(CUTMiscBitmap, ddsHeaderIsChecked)
{
	CBitmap bitmap;
	// clang-format off
	EXPECT_THROW(loadImage(bitmap, makeDDS(0, 8, 8, 1)), NLMISC::EStream);   // wrapped the header read length
	EXPECT_THROW(loadImage(bitmap, makeDDS(16, 8, 8, 1)), NLMISC::EStream);  // fields read past the header
	EXPECT_THROW(loadImage(bitmap, makeDDS(124, 0, 8, 1)), NLMISC::EStream); // empty
	EXPECT_THROW(loadImage(bitmap, makeDDS(124, 8, 8, 17)), NLMISC::EStream); // more levels than CBitmap holds
	// clang-format on
}

TEST(CUTMiscBitmap, resizeBeyondMemoryThrows)
{
	CBitmap bitmap;
	EXPECT_THROW(bitmap.resize(65536, 65536, CBitmap::RGBA), NLMISC::Exception);
	EXPECT_THROW(bitmap.resize(-4, 4, CBitmap::RGBA), NLMISC::Exception);
	bitmap.resize(16, 16, CBitmap::RGBA);
	EXPECT_EQ(16u * 16u * 4u, bitmap.getPixels().size());
}
