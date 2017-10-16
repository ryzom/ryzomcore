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

static NLMISC::IStream *GIFStream = NULL;

#if GIFLIB_MAJOR < 5
static uint8 INTERLACED_OFFSET[] = { 0, 4, 2, 1 };
static uint8 INTERLACED_JUMP[] = { 8, 8, 4, 2 };
#endif

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
uint8 CBitmap::readGIF( NLMISC::IStream &f )
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
		// 4000x4000x4 ~ 61MiB
		if (width*height > 4000*4000)
		{
			nlwarning("GIF image size is too big (width=%d, height=%d)", width, height);
			return 0;
		}

		// rewind for gif decoder
		f.seek(-10, IStream::current);
	}

#if GIFLIB_MAJOR >= 5
	sint32 errorCode;
	GifFileType *gif = DGifOpen(&f, readGIFData, &errorCode);
	if (gif == NULL)
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

	// this will read and decode all frames
	sint32 ret = DGifSlurp(gif);
	if (ret != GIF_OK)
	{
		nlwarning("failed to read gif, error=%d", ret);
#if GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
		DGifCloseFile(gif, &errorCode);
#else
		DGifCloseFile(gif);
#endif
		return 0;
	}

	// resize target buffer
	uint32 dstChannels = 4; // RGBA
	resize (gif->SWidth, gif->SHeight, RGBA);

	// make transparent
	_Data[0].fill(0);

	// make sure background color index exists in global colormap
	if (gif->SColorMap && gif->SColorMap->ColorCount < gif->SBackGroundColor)
	{
		gif->SBackGroundColor = 0;
	}

	// merge all frames one by one into single target
	ColorMapObject *ColorMap;
	sint32 transparency = GIF_NOT_TRANSPARENT;
	uint8 r, g, b, a;
	uint32 offset_x, offset_y, width, height;

	// disable loop as we only interested in first frame
	// for (uint32 frame = 0; frame < gif->ImageCount; frame++)
	{
		uint32 frame = 0;
		SavedImage *curFrame = &gif->SavedImages[frame];

		if (curFrame->ExtensionBlockCount > 0)
		{
			for(sint e = 0; e < curFrame->ExtensionBlockCount; e++)
			{
				ExtensionBlock *ext = &curFrame->ExtensionBlocks[e];

				if (ext->Function == GRAPHICS_EXT_FUNC_CODE)
				{
					uint8 flag = ext->Bytes[0];
					//delay = (ext.Bytes[1] << 8) | ext.Bytes[2];
					transparency = (flag & GIF_TRANSPARENT_MASK) ? ext->Bytes[3] : GIF_NOT_TRANSPARENT;
					//dispose = ((flag >> 2) & GIF_DISPOSE_MASK);
				}
			}
		}

		// select color map for frame
		if (curFrame->ImageDesc.ColorMap)
		{
			ColorMap = curFrame->ImageDesc.ColorMap;
		}
		else
		if (gif->SColorMap)
		{
			ColorMap = gif->SColorMap;
		}
		else
		{
			nlwarning("GIF has no global or local color map");
			ColorMap = NULL;
		}

		// copy frame to canvas
		offset_x = curFrame->ImageDesc.Left;
		offset_y = curFrame->ImageDesc.Top;
		width = curFrame->ImageDesc.Width;
		height = curFrame->ImageDesc.Height;

#if GIFLIB_MAJOR < 5
		// giflib 4 does not handle interlaced images, so we must do it
		if (curFrame->ImageDesc.Interlace)
		{
			uint32 srcOffset = 0;
			for (uint8 pass = 0; pass < 4; pass++)
			{
				uint32 nextLine = INTERLACED_OFFSET[pass];

				// y is destination row
				for (uint32 y = 0; y < height; y++)
				{
					if (y != nextLine)
						continue;

					uint32 dstOffset = (y + offset_y)*gif->SWidth*dstChannels + offset_x*dstChannels;
					nextLine += INTERLACED_JUMP[pass];

					for (uint32 x = 0; x < width; x++)
					{
						uint32 index = curFrame->RasterBits[srcOffset];

						if ((sint32)index != transparency)
						{
							// make sure color index is not outside colormap
							if (ColorMap)
							{
								if ((sint)index > ColorMap->ColorCount)
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

						_Data[0][dstOffset]   = r;
						_Data[0][dstOffset+1] = g;
						_Data[0][dstOffset+2] = b;
						_Data[0][dstOffset+3] = a;

						srcOffset++;
						dstOffset+= dstChannels;
					} // x loop
				} // y loop
			} // pass loop
		}
		else
#endif
		for (uint32 y = 0; y < height; y++)
		{
			uint32 srcOffset = y*width;
			uint32 dstOffset = (y + offset_y)*gif->SWidth*dstChannels + offset_x*dstChannels;
			for (uint32 x = 0; x < width; x++)
			{
				uint32 index = curFrame->RasterBits[srcOffset];

				if ((sint32)index != transparency)
				{
					// make sure color index is not outside colormap
					if (ColorMap)
					{
						if ((sint)index > ColorMap->ColorCount)
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

				_Data[0][dstOffset]   = r;
				_Data[0][dstOffset+1] = g;
				_Data[0][dstOffset+2] = b;
				_Data[0][dstOffset+3] = a;

				srcOffset++;
				dstOffset+= dstChannels;
			} // x loop
		} // y loop
	}

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

uint8 CBitmap::readGIF( NLMISC::IStream &/* f */)
{
	nlwarning ("You must compile NLMISC with USE_GIF if you want gif support");
	return 0;
}

#endif
}//namespace
