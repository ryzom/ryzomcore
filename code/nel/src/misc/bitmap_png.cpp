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

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "nel/misc/rgba.h"
#include "nel/misc/dynloadlib.h"
#include <png.h>
#include <csetjmp>

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

static void readPNGData(png_structp png_ptr, png_bytep data, png_size_t length)
{
	IStream *stream = static_cast<IStream*>(png_get_io_ptr(png_ptr));

	try
	{
		if (stream)
			stream->serialBuffer((uint8*)data, (uint)length);
	}
	catch (...)
	{
		png_error(png_ptr, "Read error while decoding PNG file");
	}
}

static void writePNGData(png_structp png_ptr, png_bytep data, png_size_t length)
{
	IStream *stream = static_cast<IStream*>(png_get_io_ptr(png_ptr));
	if (stream)
		stream->serialBuffer((uint8*)data, (uint)length);
}

static void setPNGWarning(png_struct * /* png_ptr */, const char* message)
{
	nlwarning(message);
}

static void setPNGError(png_struct *png_ptr, const char* message)
{
	setPNGWarning(png_ptr, message);

	longjmp(png_jmpbuf(png_ptr), 1);
}

/*-------------------------------------------------------------------*\
							readPNG
\*-------------------------------------------------------------------*/
uint8 CBitmap::readPNG( NLMISC::IStream &f )
{
	if(!f.isReading()) return false;

	// initialize the info header
	png_struct *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, setPNGError, setPNGWarning);

	if (png_ptr == NULL)
	{
		nlwarning("failed to create the png read struct");
		return 0;
	}

	// allocate/initialize the memory for image information.
	png_info *info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		nlwarning("failed to create the png info struct");
		return 0;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// free all of the memory associated with the png_ptr and info_ptr
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		// if we get here, we had a problem reading the file
		nlwarning("Error while reading PNG");
		return 0;
	}

	// set the read function
	png_set_read_fn(png_ptr, (void*)&f, readPNGData);

	// set number of bit already read (in order to step back)
	png_set_sig_bytes(png_ptr, 4);

	// read header info and use it
	png_read_info(png_ptr, info_ptr);

	// get header infos
	png_uint_32 width, height;
	int iBitDepth, iColorType;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &iBitDepth, &iColorType, NULL, NULL, NULL);

	// expand images of all color-type and bit-depth to 3x8 bit RGB images
	// let the library process things like alpha, transparency, background
	double dGamma;

	// make sure it will use 8 bits per channel
	if (iBitDepth == 16)
		png_set_strip_16(png_ptr);
	else if (iBitDepth < 8)
		png_set_packing(png_ptr);

	// expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
	if (iBitDepth < 8 || iColorType == PNG_COLOR_TYPE_PALETTE || png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);

	// if required set gamma conversion
	if (png_get_gAMA(png_ptr, info_ptr, &dGamma))
		png_set_gamma(png_ptr, (double) 2.2, dGamma);

	// add alpha byte after each RGB triplet if it doesn't exist
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	// after the transformations have been registered update info_ptr data
	png_read_update_info(png_ptr, info_ptr);

	// get again width, height and the new bit-depth and color-type
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &iBitDepth, &iColorType, NULL, NULL, NULL);

	uint8 imageDepth;

	switch(iColorType)
	{
		case PNG_COLOR_TYPE_GRAY:
		imageDepth = iBitDepth;
		break;

		case PNG_COLOR_TYPE_PALETTE:
		imageDepth = iBitDepth;
		break;

		case PNG_COLOR_TYPE_RGB:
		imageDepth = iBitDepth * 3;
		break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
		imageDepth = iBitDepth * 4;
		break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
		imageDepth = iBitDepth * 2;
		break;

		default:
		imageDepth = iBitDepth * 4;
		nlwarning("Unable to determine PNG color type: %d, consider it as RGBA", iColorType);
		break;
	}

	// at this point, the image must be converted to an 24bit image RGB

	// rowbytes is the width x number of channels
	uint32 rowbytes = (uint32)png_get_rowbytes(png_ptr, info_ptr);
	uint32 srcChannels = png_get_channels(png_ptr, info_ptr);

	// allocates buffer to copy image data
	png_bytepp row_pointers = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));

	for (uint row = 0; row < height; row++)
		row_pointers[row] = (png_bytep)png_malloc(png_ptr, rowbytes);

	// effective read of the image
	png_read_image(png_ptr, row_pointers);

	// read rest of file, and get additional chunks in info_ptr
	png_read_end(png_ptr, info_ptr);

	uint32 dstChannels = 0, firstChannel = 0, lastChannel = 0;

	if (iColorType == PNG_COLOR_TYPE_RGBA || iColorType == PNG_COLOR_TYPE_RGB || iColorType == PNG_COLOR_TYPE_PALETTE)
	{
		// get all channels
		dstChannels = 4;
		firstChannel = 0;
		lastChannel = 3;
		resize (width, height, RGBA);
	}
	else if (iColorType == PNG_COLOR_TYPE_GRAY)
	{
		// only get gray channel
		dstChannels = 1;
		firstChannel = 0;
		lastChannel = 0;
		resize (width, height, _LoadGrayscaleAsAlpha ? Alpha : Luminance);
	}
	else if (iColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		// get gray and alpha channels
		dstChannels = 2;
		firstChannel = 0;
		lastChannel = 1;
		resize (width, height, AlphaLuminance);
	}
	else
	{
		png_error(png_ptr, "unknown iColorType");
	}

	for (uint32 y = 0; y < height; y++)
	{
		for (uint32 x = 0; x < width; x++)
		{
			uint32 dstOffset = y*width*dstChannels+x*dstChannels;
			const uint32 srcOffset = x*srcChannels;

			for (uint32 pix = firstChannel; pix <= lastChannel; pix++)
				_Data[0][dstOffset++] = row_pointers[y][srcOffset+pix];
		}
	}

	// free allocated memory to copy each rows
	for (uint row = 0; row < height; row++)
		png_free(png_ptr, row_pointers[row]);

	// free allocated memory to copy the image
	png_free(png_ptr, row_pointers);

	// clean up after the read, and free any memory allocated
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	//return the size of a pixel, either 8,24,32 bit
	return imageDepth;
}

// small helper to avoid local variables
static bool writePNGSetJmp(png_struct *png_ptr)
{
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// free all of the memory associated with the png_ptr
		png_destroy_write_struct(&png_ptr, (png_info**)NULL);
		// if we get here, we had a problem writing the file
		nlwarning("Error while writing PNG");
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------*\
							writePNG
\*-------------------------------------------------------------------*/
bool CBitmap::writePNG( NLMISC::IStream &f, uint32 d)
{
	if(f.isReading()) return false;

	if (PixelFormat > AlphaLuminance) return false;
	if (!_Width || !_Height) return false;

	if (d == 0) d = bitPerPixels[PixelFormat];

	if (d!=32 && d!=24 && d!=16 && d!=8) return false;

	// create image write structure
	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, setPNGError, setPNGWarning);

	if (!png_ptr)
	{
		nlwarning("couldn't save PNG image.");
		return false;
	}

	// create info structure
	png_info *info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
	{
		png_destroy_write_struct( &png_ptr, (png_info**)NULL );
		nlwarning("couldn't save PNG image.");
		return false;
	}

	if (!writePNGSetJmp(png_ptr)) return false;

	// set the write function
	png_set_write_fn(png_ptr, (void*)&f, writePNGData, NULL);

	int iColorType;
	
	// only RGBA color, RGB color and gray type are implemented
	if (d == 8)
	{
		iColorType = PNG_COLOR_TYPE_GRAY;
	}
	else if (d == 16)
	{
		iColorType = PNG_COLOR_TYPE_GRAY_ALPHA;
	}
	else if (d == 24)
	{
		iColorType = PNG_COLOR_TYPE_RGB;
	}
	else
	{
		iColorType = PNG_COLOR_TYPE_RGBA;
	}

	// we don't have to implement 16bits formats because NeL only uses 8bits
	const int iBitDepth = 8;

	// set correct values for PNG header
	png_set_IHDR(png_ptr, info_ptr, _Width, _Height, iBitDepth, iColorType,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_color_8 sig_bit;

	if (iColorType & PNG_COLOR_MASK_COLOR)
	{
		sig_bit.red = sig_bit.green = sig_bit.blue = (uint8)iBitDepth;
	}
	else
	{
		sig_bit.gray = (uint8)iBitDepth;
	}

	if (iColorType & PNG_COLOR_MASK_ALPHA)
	{
		sig_bit.alpha = (uint8)iBitDepth;
	}
	else
	{
		sig_bit.alpha = 0;
	}

	// set bit depths
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

	// Optional gamma chunk is strongly suggested if you have any guess
	// as to the correct gamma of the image.
//	double gamma = 2.2;
//	png_set_gAMA(png_ptr, info_ptr, gamma);

	// Optionally write comments into the image
//	png_text text_ptr[3];
//	text_ptr[0].key = "Title";
//	text_ptr[0].text = "Mona Lisa";
//	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
//	text_ptr[1].key = "Author";
//	text_ptr[1].text = "Leonardo DaVinci";
//	text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
//	text_ptr[2].key = "Description";
//	text_ptr[2].text = "<long text>";
//	text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;

//	png_set_text(png_ptr, info_ptr, text_ptr, 3);

	// write the file header information
	png_write_info(png_ptr, info_ptr);

	// shift the pixels up to a legal bit depth
	png_set_shift(png_ptr, &sig_bit);

	// pack pixels into bytes
	png_set_packing(png_ptr);

	// rowbytes is the width x number of channels
	uint32 rowbytes = (uint32)png_get_rowbytes(png_ptr, info_ptr);
	uint32 dstChannels = png_get_channels(png_ptr, info_ptr);

	// get channels number of bitmap
	sint srcChannels = bitPerPixels[PixelFormat]/8;

	// get size of bitmap
	sint height = _Height;
	sint width = _Width;

	// allocates buffer to copy image data
	png_bytepp row_pointers = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));

	for (sint row = 0; row < height; row++)
		row_pointers[row] = (png_bytep)png_malloc(png_ptr, rowbytes);

	sint y, x, i;

	for(y=0; y<height; ++y)
	{
		for(x=0; x<width; ++x)
		{
			const uint srcOffset = y*width*srcChannels + x*srcChannels;
			const uint dstOffset = x*dstChannels;

			if (dstChannels <= 2 && srcChannels == 4)
			{
				// convert colors to gray
				row_pointers[y][dstOffset] = CRGBA(_Data[0][srcOffset+0],
					_Data[0][srcOffset+1], _Data[0][srcOffset+2]).toGray();

				if (sig_bit.alpha)
				{
					// copy alpha value
					row_pointers[y][dstOffset+1] = _Data[0][srcOffset+3];
				}
			}
			else if (dstChannels >= 3 && srcChannels <= 2)
			{
				// convert gray to colors
				for(i=0; i<3; ++i)
				{
					row_pointers[y][dstOffset+i] = _Data[0][srcOffset];
				}

				if (sig_bit.alpha)
				{
					// copy alpha value
					row_pointers[y][dstOffset+3] = PixelFormat ==
						Luminance ? 255:_Data[0][srcOffset+1];
				}
			}
			else
			{
				// copy all values
				for(i=0; i<(sint)dstChannels; ++i)
				{
					row_pointers[y][dstOffset+i] = _Data[0][srcOffset+i];
				}
			}
		}
	}

	// writing image data
	png_write_image(png_ptr, row_pointers);

	// writing the rest of the file
	png_write_end(png_ptr, info_ptr);

	// free allocated memory to copy each rows
	for (sint row = 0; row < height; row++)
		png_free(png_ptr, row_pointers[row]);

	// free allocated memory to copy the image
	png_free(png_ptr, row_pointers);

	// clean up after the write, and free any memory allocated
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true;
}

}//namespace
