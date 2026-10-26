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

// a TGA header (18 bytes) of the given type, size and depth, with the origin at the top
static TBytes makeTGAHeader(uint8 type, uint32 width, uint32 height, uint8 depth)
{
	TBytes tga(18, 0);
	tga[2] = type;
	putLE16(tga, 12, width);
	putLE16(tga, 14, height);
	tga[16] = depth;
	tga[17] = 0x20;
	return tga;
}

TEST(CUTMiscBitmap, tgaRlePacketStopsAtTheImageEnd)
{
	// a 2x2 RLE image whose single packet repeats its pixel 128 times: the reader wrote 512 bytes into the 16 of the
	// bitmap
	TBytes tga = makeTGAHeader(10, 2, 2, 32);
	tga.push_back(0x80 | 127);
	tga.push_back(0x10);
	tga.push_back(0x20);
	tga.push_back(0x30);
	tga.push_back(0x40);
	CBitmap bitmap;
	EXPECT_EQ(32, loadImage(bitmap, tga));
	ASSERT_EQ(2u, bitmap.getWidth());
	ASSERT_EQ(2u, bitmap.getHeight());
	ASSERT_EQ(16u, bitmap.getPixels().size());
	for (uint i = 0; i < 4; ++i)
	{
		EXPECT_EQ(0x30, bitmap.getPixels()[i * 4]);
		EXPECT_EQ(0x40, bitmap.getPixels()[i * 4 + 3]);
	}
}

TEST(CUTMiscBitmap, tgaRgbOf8BitsIsRefused)
{
	// read 3 bytes per pixel from scanlines of 1
	TBytes tga = makeTGAHeader(2, 4, 1, 8);
	tga.resize(tga.size() + 4, 0x7f);
	CBitmap bitmap;
	EXPECT_EQ(0, loadImage(bitmap, tga));
}

TEST(CUTMiscBitmap, truncatedTgaThrows)
{
	TBytes tga = writeImage(FormatTGA, 64, 64);
	tga.resize(tga.size() / 2);
	CBitmap bitmap;
	EXPECT_THROW(loadImage(bitmap, tga), NLMISC::EStream);
}

TEST(CUTMiscBitmap, truncatedPngFails)
{
	// libpng's error longjmp leaked the rows already allocated
	TBytes png = writeImage(FormatPNG, 64, 64);
	png.resize(png.size() / 2);
	CBitmap bitmap;
	EXPECT_EQ(0, loadImage(bitmap, png));
}

TEST(CUTMiscBitmap, truncatedJpegEnds)
{
	// the JPEG reader returned an empty buffer as filled at the end of the data, and libjpeg read past it; the end of
	// the data now ends the image, the missing part is grey
	TBytes jpg = writeImage(FormatJPG, 64, 64);
	jpg.resize(jpg.size() / 2);
	CBitmap bitmap;
	EXPECT_NE(0, loadImage(bitmap, jpg));
	EXPECT_EQ(64u, bitmap.getWidth());
	EXPECT_EQ(64u, bitmap.getHeight());
}

TEST(CUTMiscBitmap, jpegMarkerPastTheEndEnds)
{
	// an APP1 marker of 65535 bytes in a 6 byte file: skipping it looped forever
	TBytes jpg;
	const uint8 bytes[] = { 0xff, 0xd8, 0xff, 0xe1, 0xff, 0xff };
	jpg.assign(bytes, bytes + sizeof(bytes));
	CBitmap bitmap;
	EXPECT_EQ(0, loadImage(bitmap, jpg));
}

// a grayscale progressive JPEG of size x size: one DC scan, then acScans empty AC scans
static TBytes makeProgressiveJPG(uint32 size, uint32 acScans)
{
	// clang-format off
	const uint8 head[] = { 0xff, 0xd8, 0xff, 0xdb, 0x00, 0x43, 0x00 }; // start, then a quantization table of 64 ones
	const uint8 frame[] = { 0xff, 0xc2, 0x00, 0x0b, 0x08, 0, 0, 0, 0, 0x01, 0x01, 0x11, 0x00 }; // progressive, 1 component
	const uint8 dcScan[] = { 0xff, 0xda, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	const uint8 acScan[] = { 0xff, 0xda, 0x00, 0x08, 0x01, 0x01, 0x00, 0x01, 0x3f, 0x00 };
	// clang-format on
	TBytes jpg(head, head + sizeof(head));
	jpg.insert(jpg.end(), 64, 1);
	size_t framePos = jpg.size();
	jpg.insert(jpg.end(), frame, frame + sizeof(frame));
	putBE16(jpg, framePos + 5, size);
	putBE16(jpg, framePos + 7, size);
	for (uint8 table = 0; table < 2; ++table)
	{
		// the DC then the AC Huffman table, of one code
		const uint8 huffman[] = { 0xff, 0xc4, 0x00, 0x14, (uint8)(table << 4), 1 };
		jpg.insert(jpg.end(), huffman, huffman + sizeof(huffman));
		jpg.insert(jpg.end(), 15, 0);
		jpg.push_back(0);
	}
	jpg.insert(jpg.end(), dcScan, dcScan + sizeof(dcScan));
	for (uint32 i = 0; i < acScans; ++i)
		jpg.insert(jpg.end(), acScan, acScan + sizeof(acScan));
	jpg.push_back(0xff);
	jpg.push_back(0xd9);
	return jpg;
}

TEST(CUTMiscBitmap, jpegScanCountIsLimited)
{
	// every scan walks every block of the image: 10000 scans, 100 KiB, took 8 s at 4096x4096
	CBitmap few;
	EXPECT_NE(0, loadImage(few, makeProgressiveJPG(64, 10)));
	EXPECT_EQ(64u, few.getWidth());
	CBitmap many;
	EXPECT_EQ(0, loadImage(many, makeProgressiveJPG(64, 600)));
}
