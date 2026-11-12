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

TEST(CUTMiscBitmap, dxt1BlockColorsAreOpaque)
{
	// 8x4, a block of four colors (the first color above the second) and a block of three colors and a transparent
	// one, every row with the indices 0, 1, 2, 3. Read as DXT1 with alpha, the two colors of a block were transparent.
	const uint8 expectedAlpha[8] = { 255, 255, 255, 255, 255, 255, 255, 0 };
	for (uint withAlpha = 0; withAlpha < 2; ++withAlpha)
	{
		SCOPED_TRACE(withAlpha);
		TBytes dds = makeDDS(124, 8, 4, 1);
		if (withAlpha)
			putLE32(dds, 4 + 21 * 4, 1); // CBitmap reads a DXT1 file with this header field set as DXT1 with alpha
		const size_t blocks = 4 + 124;
		putLE16(dds, blocks, 0xf800); // red
		putLE16(dds, blocks + 2, 0x001f); // blue
		putLE16(dds, blocks + 8, 0x001f);
		putLE16(dds, blocks + 8 + 2, 0xf800);
		for (uint row = 0; row < 4; ++row)
		{
			dds[blocks + 4 + row] = 0xe4;
			dds[blocks + 8 + 4 + row] = 0xe4;
		}

		CBitmap bitmap;
		EXPECT_NE(0, loadImage(bitmap, dds));
		ASSERT_EQ(withAlpha ? CBitmap::DXTC1Alpha : CBitmap::DXTC1, bitmap.getPixelFormat());

		// the texels as read from the compressed blocks, then the same after decompression
		NLMISC::CRGBA texels[8];
		for (uint x = 0; x < 8; ++x)
			texels[x] = bitmap.getPixelColor(x, 0);
		ASSERT_TRUE(bitmap.convertToType(CBitmap::RGBA));
		ASSERT_EQ(8u * 4u * 4u, bitmap.getPixels().size());
		for (uint y = 0; y < 4; ++y)
		{
			for (uint x = 0; x < 8; ++x)
			{
				const uint8 *pixel = &bitmap.getPixels()[(y * 8 + x) * 4];
				EXPECT_EQ(expectedAlpha[x], pixel[3]) << "alpha at " << x << ", " << y;
				EXPECT_EQ(texels[x], NLMISC::CRGBA(pixel[0], pixel[1], pixel[2], pixel[3])) << "at " << x << ", " << y;
			}
		}
		// the colors: red and blue, their blends, then blue and red, their blend and transparent black
		const uint8 *row = &bitmap.getPixels()[0];
		EXPECT_EQ(NLMISC::CRGBA(255, 0, 0, 255), NLMISC::CRGBA(row[0], row[1], row[2], row[3]));
		EXPECT_EQ(NLMISC::CRGBA(0, 0, 255, 255), NLMISC::CRGBA(row[4], row[5], row[6], row[7]));
		EXPECT_EQ(NLMISC::CRGBA(0, 0, 255, 255), NLMISC::CRGBA(row[16], row[17], row[18], row[19]));
		EXPECT_EQ(NLMISC::CRGBA(255, 0, 0, 255), NLMISC::CRGBA(row[20], row[21], row[22], row[23]));
		EXPECT_EQ(NLMISC::CRGBA(0, 0, 0, 0), NLMISC::CRGBA(row[28], row[29], row[30], row[31]));
	}
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

TEST(CUTMiscBitmap, tgaRleOf24BitsIsOpaque)
{
	// 3x1, an RLE packet of 2 pixels and a raw packet of 1, stored blue, green, red: the alpha was left at 0
	const uint8 expected24[] = { 0x30, 0x20, 0x10, 255, 0x30, 0x20, 0x10, 255, 0x60, 0x50, 0x40, 255 };
	TBytes tga24 = makeTGAHeader(10, 3, 1, 24);
	const uint8 data24[] = { 0x80 | 1, 0x10, 0x20, 0x30, 0x00, 0x40, 0x50, 0x60 };
	tga24.insert(tga24.end(), data24, data24 + sizeof(data24));
	CBitmap bitmap24;
	EXPECT_EQ(24, loadImage(bitmap24, tga24));
	ASSERT_EQ(sizeof(expected24), bitmap24.getPixels().size());
	for (uint i = 0; i < sizeof(expected24); ++i)
		EXPECT_EQ(expected24[i], bitmap24.getPixels()[i]) << "byte " << i;

	// with 32 bits per pixel the alpha is the one of the file
	const uint8 expected32[] = { 0x30, 0x20, 0x10, 0x70, 0x30, 0x20, 0x10, 0x70, 0x60, 0x50, 0x40, 0x00 };
	TBytes tga32 = makeTGAHeader(10, 3, 1, 32);
	const uint8 data32[] = { 0x80 | 1, 0x10, 0x20, 0x30, 0x70, 0x00, 0x40, 0x50, 0x60, 0x00 };
	tga32.insert(tga32.end(), data32, data32 + sizeof(data32));
	CBitmap bitmap32;
	EXPECT_EQ(32, loadImage(bitmap32, tga32));
	ASSERT_EQ(sizeof(expected32), bitmap32.getPixels().size());
	for (uint i = 0; i < sizeof(expected32); ++i)
		EXPECT_EQ(expected32[i], bitmap32.getPixels()[i]) << "byte " << i;
}

TEST(CUTMiscBitmap, tgaOf16BitsHasFiveBitsPerColor)
{
	// 2x2 of 16 bits per pixel (1 attribute bit, then 5 bits of red, green and blue), a row of each color:
	// uncompressed and RLE, from the top and from the bottom, with and without the attribute bit. The attribute bit
	// went into the red, the green was the blue for an uncompressed image from the top, and an RLE image was read as
	// if it were of 24 bits.
	const uint16 colors[2] = { (16 << 10) | (21 << 5) | 3, (5 << 10) | (10 << 5) | 20 };
	const uint8 expected[2][4] = { { 132, 173, 24, 255 }, { 41, 82, 165, 255 } };
	for (uint variant = 0; variant < 8; ++variant)
	{
		SCOPED_TRACE(variant);
		const bool rle = (variant & 1) != 0;
		const bool fromTop = (variant & 2) != 0;
		const uint16 attribute = (variant & 4) ? 0x8000 : 0;
		TBytes tga = makeTGAHeader(rle ? 10 : 2, 2, 2, 16);
		tga[17] = fromTop ? 0x20 : 0x00;
		for (uint row = 0; row < 2; ++row)
		{
			const uint16 color = colors[row] | attribute;
			if (rle && row == 0)
			{
				// an RLE packet of 2 pixels
				tga.push_back(0x80 | 1);
				tga.push_back((uint8)color);
				tga.push_back((uint8)(color >> 8));
				continue;
			}
			if (rle)
				tga.push_back(0x01); // a raw packet of 2 pixels
			for (uint x = 0; x < 2; ++x)
			{
				tga.push_back((uint8)color);
				tga.push_back((uint8)(color >> 8));
			}
		}
		CBitmap bitmap;
		EXPECT_EQ(16, loadImage(bitmap, tga));
		ASSERT_EQ(CBitmap::RGBA, bitmap.getPixelFormat());
		ASSERT_EQ(16u, bitmap.getPixels().size());
		for (uint y = 0; y < 2; ++y)
		{
			// the first row of the file is the bottom row of the image unless the header says otherwise
			const uint8 *row = expected[fromTop ? y : 1 - y];
			for (uint i = 0; i < 8; ++i)
				EXPECT_EQ(row[i % 4], bitmap.getPixels()[y * 8 + i]) << "row " << y << " byte " << i;
		}
	}
}

TEST(CUTMiscBitmap, tgaRleOf8BitsIsRefused)
{
	// an RLE RGB image of 8 bits per pixel was read as the blue of a transparent image
	TBytes tga = makeTGAHeader(10, 4, 1, 8);
	tga.push_back(0x80 | 3);
	tga.push_back(0x7f);
	CBitmap bitmap;
	EXPECT_EQ(0, loadImage(bitmap, tga));
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

// a JPEG segment: the marker, the length, which counts its own 2 bytes, and the data
static void pushJPGSegment(TBytes &jpg, uint8 marker, const TBytes &data)
{
	jpg.push_back(0xff);
	jpg.push_back(marker);
	jpg.push_back((uint8)((data.size() + 2) >> 8));
	jpg.push_back((uint8)(data.size() + 2));
	jpg.insert(jpg.end(), data.begin(), data.end());
}

// the data of a JPEG frame header of one component
static TBytes makeJPGFrame(uint32 width, uint32 height)
{
	TBytes frame(9, 0);
	frame[0] = 8;
	putBE16(frame, 1, height);
	putBE16(frame, 3, width);
	frame[5] = 1;
	frame[6] = 1;
	frame[7] = 0x11;
	return frame;
}

// a JPEG file of the segments given, between the start and the end of image markers
static TBytes makeJPG(const TBytes &segments)
{
	TBytes jpg;
	jpg.push_back(0xff);
	jpg.push_back(0xd8);
	jpg.insert(jpg.end(), segments.begin(), segments.end());
	jpg.push_back(0xff);
	jpg.push_back(0xd9);
	return jpg;
}

static void loadSize(const TBytes &bytes, uint32 &width, uint32 &height)
{
	CMemStream stream(true);
	stream.fill(&bytes[0], (uint32)bytes.size());
	width = ~0u;
	height = ~0u;
	CBitmap::loadSize(stream, width, height);
}

TEST(CUTMiscBitmap, jpegSizeIsNotTheOneOfTheThumbnail)
{
	// a photo of 1024x768 whose Exif segment holds a thumbnail of 160x120, which is a JPEG image of its own: the size
	// read was the one of the thumbnail
	const TBytes thumbnailFrame = makeJPGFrame(160, 120);
	TBytes thumbnail;
	pushJPGSegment(thumbnail, 0xc0, thumbnailFrame);
	thumbnail = makeJPG(thumbnail);
	const char exifHeader[] = "Exif\0\0II*\0\x08\0\0\0";
	TBytes exif(exifHeader, exifHeader + sizeof(exifHeader) - 1);
	exif.insert(exif.end(), thumbnail.begin(), thumbnail.end());
	const char jfifHeader[] = "JFIF\0\x01\x01\0\0\x01\0\x01\0\0";
	const TBytes jfif(jfifHeader, jfifHeader + sizeof(jfifHeader) - 1);

	for (uint withJfif = 0; withJfif < 2; ++withJfif)
	{
		SCOPED_TRACE(withJfif);
		TBytes segments;
		if (withJfif)
			pushJPGSegment(segments, 0xe0, jfif);
		pushJPGSegment(segments, 0xe1, exif);
		pushJPGSegment(segments, 0xdb, TBytes(65, 1));
		pushJPGSegment(segments, 0xc0, makeJPGFrame(1024, 768));
		uint32 width, height;
		loadSize(makeJPG(segments), width, height);
		EXPECT_EQ(1024u, width);
		EXPECT_EQ(768u, height);
	}

	// a thumbnail that ends before any frame header ended the search
	TBytes segments;
	pushJPGSegment(segments, 0xe1, makeJPG(TBytes()));
	pushJPGSegment(segments, 0xc2, makeJPGFrame(640, 480));
	uint32 width, height;
	loadSize(makeJPG(segments), width, height);
	EXPECT_EQ(640u, width);
	EXPECT_EQ(480u, height);
}

TEST(CUTMiscBitmap, jpegSizeOfEveryFrameType)
{
	// baseline, extended, progressive and lossless, then their differential and arithmetic coded variants, which
	// were not known
	const uint8 frames[] = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc5, 0xc6, 0xc7, 0xc9, 0xca, 0xcb, 0xcd, 0xce, 0xcf };
	for (uint i = 0; i < sizeof(frames); ++i)
	{
		SCOPED_TRACE(i);
		TBytes segments;
		pushJPGSegment(segments, 0xdb, TBytes(65, 1));
		pushJPGSegment(segments, frames[i], makeJPGFrame(300 + i, 200 + i));
		uint32 width, height;
		loadSize(makeJPG(segments), width, height);
		EXPECT_EQ(300 + i, width);
		EXPECT_EQ(200 + i, height);
	}

	// a Huffman table, a reserved segment and an arithmetic conditioning segment are no frame headers
	const uint8 others[] = { 0xc4, 0xc8, 0xcc };
	for (uint i = 0; i < sizeof(others); ++i)
	{
		SCOPED_TRACE(i);
		TBytes segments;
		pushJPGSegment(segments, others[i], makeJPGFrame(11, 22));
		pushJPGSegment(segments, 0xc2, makeJPGFrame(640, 480));
		uint32 width, height;
		loadSize(makeJPG(segments), width, height);
		EXPECT_EQ(640u, width);
		EXPECT_EQ(480u, height);
	}

	{
		// the frame header as the first segment, after fill bytes and a marker without data
		TBytes segments;
		segments.push_back(0xff);
		segments.push_back(0xff);
		segments.push_back(0x01);
		segments.push_back(0xff);
		pushJPGSegment(segments, 0xc0, makeJPGFrame(48, 32));
		uint32 width, height;
		loadSize(makeJPG(segments), width, height);
		EXPECT_EQ(48u, width);
		EXPECT_EQ(32u, height);
	}

	uint32 width, height;
	loadSize(makeProgressiveJPG(64, 1), width, height);
	EXPECT_EQ(64u, width);
	EXPECT_EQ(64u, height);
}

TEST(CUTMiscBitmap, jpegSizeSearchStaysInTheSegments)
{
	TBytes frame;
	pushJPGSegment(frame, 0xc0, makeJPGFrame(16, 16));
	const TBytes dqt(65, 1);

	// each of these has no size: 0x0
	std::vector<TBytes> files;
	{
		// a segment that runs past the end of the file: its bytes were searched for a frame header
		TBytes jpg = makeJPG(TBytes());
		jpg.resize(2);
		jpg.push_back(0xff);
		jpg.push_back(0xe1);
		jpg.push_back(0x00);
		jpg.push_back(0x40);
		jpg.insert(jpg.end(), frame.begin(), frame.end());
		files.push_back(jpg);
	}
	for (uint8 length = 0; length < 2; ++length)
	{
		// a segment length that does not count its own 2 bytes
		TBytes segments;
		segments.push_back(0xff);
		segments.push_back(0xe0);
		segments.push_back(0x00);
		segments.push_back(length);
		segments.insert(segments.end(), frame.begin(), frame.end());
		files.push_back(makeJPG(segments));
	}
	{
		// the image data, or the end of the image, before a frame header
		TBytes segments;
		pushJPGSegment(segments, 0xda, TBytes(6, 0));
		segments.insert(segments.end(), frame.begin(), frame.end());
		files.push_back(makeJPG(segments));
		TBytes ended;
		ended.push_back(0xff);
		ended.push_back(0xd9);
		ended.insert(ended.end(), frame.begin(), frame.end());
		files.push_back(makeJPG(ended));
	}
	{
		// a frame header too short for the size, and one cut by the end of the file
		TBytes segments;
		pushJPGSegment(segments, 0xc0, TBytes(3, 0x10));
		segments.insert(segments.end(), frame.begin(), frame.end());
		files.push_back(makeJPG(segments));
		TBytes cut = makeJPG(frame);
		cut.resize(2 + 7);
		files.push_back(cut);
	}
	{
		// bytes that are no marker up to the end of the file, a file that ends in a marker, and fill bytes only
		TBytes stray = makeJPG(TBytes());
		stray.resize(2);
		pushJPGSegment(stray, 0xdb, dqt);
		stray.resize(stray.size() + 1000, 0x55);
		files.push_back(stray);
		TBytes cut = makeJPG(TBytes());
		cut.resize(2);
		pushJPGSegment(cut, 0xdb, dqt);
		cut.push_back(0xff);
		files.push_back(cut);
		cut.resize(2);
		cut.push_back(0xff);
		cut.push_back(0xe0);
		cut.push_back(0x00);
		files.push_back(cut);
		files.push_back(TBytes(100000, 0xff));
		files[files.size() - 1][1] = 0xd8;
	}
	for (size_t i = 0; i < files.size(); ++i)
	{
		SCOPED_TRACE(i);
		uint32 width, height;
		loadSize(files[i], width, height);
		EXPECT_EQ(0u, width);
		EXPECT_EQ(0u, height);
	}
}

TEST(CUTMiscBitmap, jpegSizeSearchPassesOverStrayBytesAsTheDecoder)
{
	// bytes that belong to no segment before a marker are passed over by the decoder, with a warning: the size is read
	// from such a file too
	const uint8 zeros[] = { 0x00, 0x00, 0x00 };
	const uint8 stuffed[] = { 0xff, 0x00 };
	const TBytes strays[] = { TBytes(zeros, zeros + sizeof(zeros)), TBytes(stuffed, stuffed + sizeof(stuffed)) };
	for (uint i = 0; i < 2; ++i)
	{
		SCOPED_TRACE(i);

		// an image as written, with the bytes before its frame header: it decodes, to the size loadSize gives
		TBytes jpg = writeImage(FormatJPG, 40, 24);
		size_t framePos = 0;
		for (size_t pos = 2; pos + 1 < jpg.size() && !framePos; ++pos)
		{
			if (jpg[pos] == 0xff && jpg[pos + 1] == 0xc0)
				framePos = pos;
		}
		ASSERT_NE((size_t)0, framePos);
		jpg.insert(jpg.begin() + framePos, strays[i].begin(), strays[i].end());
		uint32 width, height;
		loadSize(jpg, width, height);
		EXPECT_EQ(40u, width);
		EXPECT_EQ(24u, height);
		CBitmap bitmap;
		EXPECT_NE(0, loadImage(bitmap, jpg));
		EXPECT_EQ(40u, bitmap.getWidth());
		EXPECT_EQ(24u, bitmap.getHeight());

		// the body of a segment is still skipped by its length: the frame header in it is not the one of the image
		TBytes thumbnail;
		pushJPGSegment(thumbnail, 0xc0, makeJPGFrame(160, 120));
		TBytes segments = strays[i];
		pushJPGSegment(segments, 0xe1, makeJPG(thumbnail));
		segments.insert(segments.end(), strays[i].begin(), strays[i].end());
		pushJPGSegment(segments, 0xc2, makeJPGFrame(640, 480));
		loadSize(makeJPG(segments), width, height);
		EXPECT_EQ(640u, width);
		EXPECT_EQ(480u, height);
	}
}

namespace {

// counts the reads and the seeks of a stream
class CCountingMemStream : public CMemStream
{
public:
	CCountingMemStream()
	    : CMemStream(true)
	    , Calls(0)
	{
	}

	void serialBuffer(uint8 *buf, uint len) override
	{
		++Calls;
		CMemStream::serialBuffer(buf, len);
	}

	bool seek(sint32 offset, TSeekOrigin origin) const override
	{
		++Calls;
		return CMemStream::seek(offset, origin);
	}

	mutable uint Calls;
};

} // namespace

TEST(CUTMiscBitmap, jpegSizeSearchReadsTheFileInBlocks)
{
	// 1 MiB of fill bytes, and 1 MiB of empty segments: every step of the search was a seek and a read of the stream,
	// a million of them, which took seconds on the largest file a download can have
	const size_t size = 1024 * 1024;
	TBytes fill(size, 0xff);
	fill[1] = 0xd8;
	TBytes empty(size, 0);
	for (size_t i = 0; i < size; i += 4)
	{
		empty[i] = 0xff;
		empty[i + 1] = i ? 0xe0 : 0xd8;
		empty[i + 2] = i ? 0x00 : 0xff;
		empty[i + 3] = i ? 0x02 : 0xff;
	}
	const TBytes *files[] = { &fill, &empty };
	for (uint i = 0; i < 2; ++i)
	{
		SCOPED_TRACE(i);
		CCountingMemStream stream;
		stream.fill(&(*files[i])[0], (uint32)files[i]->size());
		uint32 width = ~0u, height = ~0u;
		CBitmap::loadSize(stream, width, height);
		EXPECT_EQ(0u, width);
		EXPECT_EQ(0u, height);
		// a read per block of the file, of 1 KiB at least
		EXPECT_LE(stream.Calls, 1024u + 16u);
	}
}

// a GIF of a 2x2 canvas whose 2x2 first frame is at left, top
static TBytes makeGIF(uint32 left, uint32 top)
{
	// clang-format off
	const uint8 bytes[] = {
		'G', 'I', 'F', '8', '9', 'a', 2, 0, 2, 0, 0x80, 0, 0, // canvas of 2x2, global color map of 2 colors
		0, 0, 0, 0xff, 0xff, 0xff,                              // the color map
		0x2c, 0, 0, 0, 0, 2, 0, 2, 0, 0,                         // the frame
		2, 3, 0x04, 0x00, 0x05, 0,                               // its LZW data: clear, 4 times 0, end
		0x3b
	};
	// clang-format on
	TBytes gif(bytes, bytes + sizeof(bytes));
	putLE16(gif, 20, left);
	putLE16(gif, 22, top);
	return gif;
}

TEST(CUTMiscBitmap, gifCanvasHoldsTheFirstFrame)
{
	// a frame not within the canvas was copied past it: the canvas grows as in browsers (without GIF support, no GIF
	// loads)
	CBitmap grown;
	if (loadImage(grown, makeGIF(1, 1)) != 0)
	{
		EXPECT_EQ(3u, grown.getWidth());
		EXPECT_EQ(3u, grown.getHeight());
	}

	// a canvas that would be larger than the limit
	CBitmap refused;
	EXPECT_EQ(0, loadImage(refused, makeGIF(30000, 0)));
}

TEST(CUTMiscBitmap, resampleGrayFormats)
{
	// gray and alpha, wider than tall: the source was read as width x width pixels, past the bitmap
	CBitmap grayAlpha;
	grayAlpha.resize(16, 2, CBitmap::AlphaLuminance);
	grayAlpha.resample(8, 1);
	EXPECT_EQ(8u, grayAlpha.getWidth());
	EXPECT_EQ(1u, grayAlpha.getHeight());

	// alpha, as a grayscale image loads by default, which the drivers resample to degrade textures: it asserted
	CBitmap alpha;
	alpha.resize(8, 8, CBitmap::Alpha);
	alpha.resample(4, 4);
	EXPECT_EQ(4u, alpha.getWidth());
	EXPECT_EQ(4u, alpha.getHeight());
}

namespace {

bool loadUntrusted(CBitmap &bitmap, const TBytes &bytes, uint32 maxSize)
{
	CMemStream stream(true);
	stream.fill(&bytes[0], (uint32)bytes.size());
	return bitmap.loadUntrusted(stream, maxSize);
}

} // namespace

TEST(CUTMiscBitmap, untrustedLoadRefusesEverythingElse)
{
	CBitmap bitmap;
	EXPECT_TRUE(loadUntrusted(bitmap, writeImage(FormatPNG, 64, 32), 64));
	EXPECT_EQ(64u, bitmap.getWidth());

	// larger than the limit of the caller
	EXPECT_FALSE(loadUntrusted(bitmap, writeImage(FormatPNG, 64, 32), 63));
	EXPECT_EQ(0u, bitmap.getWidth());
	EXPECT_EQ(0u, bitmap.getPixels().size());

	// the exceptions of the readers
	EXPECT_FALSE(loadUntrusted(bitmap, makeDDS(16, 8, 8, 1), 64));
	TBytes tga = writeImage(FormatTGA, 16, 16);
	tga.resize(tga.size() / 2);
	EXPECT_FALSE(loadUntrusted(bitmap, tga, 64));
	EXPECT_EQ(0u, bitmap.getHeight());

	// not an image
	const char text[] = "<html>404</html>";
	EXPECT_FALSE(loadUntrusted(bitmap, TBytes(text, text + sizeof(text) - 1), 64));
}

TEST(CUTMiscBitmap, pngOfOneChannelSource)
{
	// 4x4 of one channel, written with a gray and an alpha channel, then as RGBA: the channel the source does not have
	// was the value of the next pixel, read past the end of the bitmap for the last one
	uint8 values[16];
	for (uint i = 0; i < 16; ++i)
		values[i] = (uint8)(10 + i * 15);
	for (uint format = 0; format < 2; ++format)
	{
		SCOPED_TRACE(format);
		const bool alpha = format != 0;
		CBitmap source;
		source.resize(4, 4, alpha ? CBitmap::Alpha : CBitmap::Luminance);
		memcpy(source.getPixels().getPtr(), values, sizeof(values));

		CMemStream grayAlpha(false);
		ASSERT_TRUE(source.writePNG(grayAlpha, 16));
		CBitmap read16;
		EXPECT_EQ(16, loadImage(read16, TBytes(grayAlpha.buffer(), grayAlpha.buffer() + grayAlpha.length())));
		ASSERT_EQ(CBitmap::AlphaLuminance, read16.getPixelFormat());
		ASSERT_EQ(4u, read16.getWidth());
		ASSERT_EQ(4u, read16.getHeight());
		for (uint i = 0; i < 16; ++i)
		{
			EXPECT_EQ(alpha ? 255 : values[i], read16.getPixels()[i * 2]) << "gray of pixel " << i;
			EXPECT_EQ(alpha ? values[i] : 255, read16.getPixels()[i * 2 + 1]) << "alpha of pixel " << i;
		}

		CMemStream rgba(false);
		ASSERT_TRUE(source.writePNG(rgba, 32));
		CBitmap read32;
		EXPECT_EQ(32, loadImage(read32, TBytes(rgba.buffer(), rgba.buffer() + rgba.length())));
		ASSERT_EQ(CBitmap::RGBA, read32.getPixelFormat());
		ASSERT_EQ(4u, read32.getWidth());
		ASSERT_EQ(4u, read32.getHeight());
		for (uint i = 0; i < 16; ++i)
		{
			for (uint channel = 0; channel < 3; ++channel)
				EXPECT_EQ(alpha ? 255 : values[i], read32.getPixels()[i * 4 + channel]) << "color of pixel " << i;
			EXPECT_EQ(alpha ? values[i] : 255, read32.getPixels()[i * 4 + 3]) << "alpha of pixel " << i;
		}
	}
}
