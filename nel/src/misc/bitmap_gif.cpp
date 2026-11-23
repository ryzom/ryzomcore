// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2017  Winch Gate Property Limited
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

#include "stdmisc.h"
#include "nel/misc/bitmap.h"

#ifdef USE_GIF
#include <gif_lib.h>
#endif

using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC
{

#ifdef USE_GIF

// GIFLIB_MAJOR is defined from version 5
#ifndef GIFLIB_MAJOR
#define GIFLIB_MAJOR 4
#endif

static uint8 GIF_TRANSPARENT_MASK = 0x01;
static uint8 GIF_DISPOSE_MASK = 0x07;
static sint8 GIF_NOT_TRANSPARENT = -1;

static uint8 GIF_DISPOSE_NONE = 0;
static uint8 GIF_DISPOSE_LEAVE = 1;
static uint8 GIF_DISPOSE_BACKGROUND = 2;
static uint8 GIF_DISPOSE_RESTORE = 3;

static NLMISC::IStream *GIFStream = nullptr;

// the first row and the row step of the 4 passes of an interlaced image
static const uint8 INTERLACED_OFFSET[] = { 0, 4, 2, 1 };
static const uint8 INTERLACED_JUMP[] = { 8, 8, 4, 2 };

static int readGIFData(GifFileType *gif, GifByteType *data, int length)
{
	NLMISC::IStream *f = static_cast<NLMISC::IStream *>(gif->UserData);

	if(!f->isReading()) return 0;

	try
	{
		f->serialBuffer((uint8*) data, length);
	}
	catch(...)
	{
		nlwarning("error while reading GIF image");

		return 0;
	}

	return length;
}

/*-------------------------------------------------------------------*\
							readGIF
\*-------------------------------------------------------------------*/
uint8 CBitmap::readGIF(NLMISC::IStream &f, uint32 maxSize)
{
	if(!f.isReading()) return false;

	{
		// check gif canvas dimension
		uint16 ver;
		uint16 width;
		uint16 height;

		f.serial(ver);
		f.serial(width);
		f.serial(height);

		// limit image size as we are using 32bit pixels
		// 4000x4000x4 ~ 61MiB (the product is computed in uint32: two uint16 multiplied as int overflowed)
		if (width > maxSize || height > maxSize || (uint32)width * height > 4000 * 4000)
		{
			nlwarning("GIF image size is too big (width=%u, height=%u)", (uint)width, (uint)height);
			return 0;
		}

		// rewind for gif decoder
		f.seek(-10, IStream::current);
	}

#if GIFLIB_MAJOR >= 5
	sint32 errorCode;
	GifFileType *gif = DGifOpen(&f, readGIFData, &errorCode);
	if (gif == nullptr)
	{
		nlwarning("failed to open gif, error=%d", errorCode);
		return 0;
	}
#else
	GifFileType *gif = DGifOpen(&f, readGIFData);
	if (gif == NULL)
	{
		nlwarning("failed to open gif, error=%d", GifLastError());
		return 0;
	}
#endif

	// Read the records up to the first image, the only frame used. DGifSlurp decoded and kept every frame of an animation,
	// each as large as its descriptor said.
	sint32 transparency = GIF_NOT_TRANSPARENT;
	std::vector<GifPixelType> raster;
	ColorMapObject *ColorMap = nullptr;
	uint32 offset_x = 0, offset_y = 0, width = 0, height = 0;
	uint32 canvasWidth = 0, canvasHeight = 0;
	bool imageRead = false;
	bool failed = false;
	GifRecordType recordType = UNDEFINED_RECORD_TYPE;

	while (!imageRead && !failed && recordType != TERMINATE_RECORD_TYPE)
	{
		if (DGifGetRecordType(gif, &recordType) == GIF_ERROR)
		{
			failed = true;
		}
		else if (recordType == EXTENSION_RECORD_TYPE)
		{
			int extCode = 0;
			GifByteType *ext = nullptr;
			if (DGifGetExtension(gif, &extCode, &ext) == GIF_ERROR)
			{
				failed = true;
			}
			else
			{
				// the graphics control block of the next image: its length, the flags, the delay (2 bytes), and the
				// transparent color index (it was read without checking that the block holds it)
				if (extCode == GRAPHICS_EXT_FUNC_CODE && ext && ext[0] >= 4)
					transparency = (ext[1] & GIF_TRANSPARENT_MASK) ? ext[4] : GIF_NOT_TRANSPARENT;
				while (ext && !failed)
				{
					if (DGifGetExtensionNext(gif, &ext) == GIF_ERROR)
						failed = true;
				}
			}
		}
		else if (recordType == IMAGE_DESC_RECORD_TYPE)
		{
			if (DGifGetImageDesc(gif) == GIF_ERROR)
			{
				failed = true;
			}
			else
			{
				offset_x = gif->Image.Left;
				offset_y = gif->Image.Top;
				width = gif->Image.Width;
				height = gif->Image.Height;

				// the frame is copied into the canvas without clipping: the canvas grows to hold a frame that does not lie
				// within it, as browsers do, within the size limits
				canvasWidth = std::max((uint32)std::max(gif->SWidth, 0), offset_x + width);
				canvasHeight = std::max((uint32)std::max(gif->SHeight, 0), offset_y + height);
				if (gif->Image.Left < 0 || gif->Image.Top < 0 || gif->Image.Width <= 0 || gif->Image.Height <= 0
				    || canvasWidth > maxSize || canvasHeight > maxSize || canvasWidth * canvasHeight > 4000 * 4000)
				{
					nlwarning("The GIF image '%s' has a frame of %ux%u at %u,%u, the canvas would be too large",
					    f.getStreamName().c_str(), width, height, offset_x, offset_y);
					failed = true;
				}
				else
				{
					raster.resize(width * height);

					// the rows of an interlaced image come in 4 passes
					uint passCount = gif->Image.Interlace ? 4 : 1;
					for (uint pass = 0; pass < passCount && !failed; ++pass)
					{
						uint32 first = gif->Image.Interlace ? INTERLACED_OFFSET[pass] : 0;
						uint32 step = gif->Image.Interlace ? INTERLACED_JUMP[pass] : 1;
						for (uint32 y = first; y < height && !failed; y += step)
						{
							if (DGifGetLine(gif, &raster[y * width], (int)width) == GIF_ERROR)
								failed = true;
						}
					}

					// select color map for frame
					ColorMap = gif->Image.ColorMap ? gif->Image.ColorMap : gif->SColorMap;
					imageRead = !failed;
				}
			}
		}
	}

	if (!imageRead)
	{
		nlwarning("failed to read gif '%s'", f.getStreamName().c_str());
#if GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
		DGifCloseFile(gif, &errorCode);
#else
		DGifCloseFile(gif);
#endif
		return 0;
	}

	if (ColorMap == nullptr || ColorMap->ColorCount <= 0)
	{
		nlwarning("GIF has no global or local color map");
		ColorMap = nullptr;
	}

	// resize target buffer
	uint32 dstChannels = 4; // RGBA
	resize(canvasWidth, canvasHeight, RGBA);

	// make transparent
	_Data[0].fill(0);

	// copy frame to canvas
	uint8 r, g, b, a;
	for (uint32 y = 0; y < height; y++)
	{
		uint32 srcOffset = y * width;
		uint32 dstOffset = (y + offset_y) * canvasWidth * dstChannels + offset_x * dstChannels;
		for (uint32 x = 0; x < width; x++)
		{
			uint32 index = raster[srcOffset];

			if ((sint32)index != transparency)
			{
				// make sure color index is not outside colormap (an index equal to the color count was read past it)
				if (ColorMap)
				{
					if ((sint)index >= ColorMap->ColorCount)
					{
						index = 0;
					}
					r = ColorMap->Colors[index].Red;
					g = ColorMap->Colors[index].Green;
					b = ColorMap->Colors[index].Blue;
				}
				else
				{
					// broken gif, no colormap
					r = g = b = 0;
				}
				a = 255;
			}
			else
			{
				// transparent
				r = g = b = a = 0;
			}

			_Data[0][dstOffset] = r;
			_Data[0][dstOffset + 1] = g;
			_Data[0][dstOffset + 2] = b;
			_Data[0][dstOffset + 3] = a;

			srcOffset++;
			dstOffset += dstChannels;
		} // x loop
	} // y loop

	// clean up after the read, and free any memory allocated
#if GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
	DGifCloseFile(gif, &errorCode);
#else
	DGifCloseFile(gif);
#endif

	//return the size of a pixel as 32bits
	return 32;
}

#else

uint8 CBitmap::readGIF(NLMISC::IStream & /* f */, uint32 /* maxSize */)
{
	nlwarning ("You must compile NLMISC with USE_GIF if you want gif support");
	return 0;
}

#endif
}//namespace
