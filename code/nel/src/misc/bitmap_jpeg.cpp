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

#ifdef USE_JPEG
#define XMD_H
#undef FAR
#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include <csetjmp>
extern "C"
{
	#include <jpeglib.h>
}
#endif

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

#ifdef USE_JPEG

static NLMISC::IStream *JPGStream = NULL;
static const uint32 JPGBufferSize = 4096;
static uint32 JPGStreamSize = 0;
static char JPGBuffer[JPGBufferSize];

struct my_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

void my_error_exit(j_common_ptr cinfo)
{
	my_error_mgr *myerr = (my_error_mgr *) cinfo->err;

	nlwarning("error while processing JPEG image");

	longjmp(myerr->setjmp_buffer, 1);
}

static void jpgDecompressInit(j_decompress_ptr cinfo)
{
	// get stream size if possible
	if (JPGStream->seek(0, IStream::end))
		JPGStreamSize = JPGStream->getPos();
	else
		nlwarning("can't get JPEG stream size");

	// reset current position to the beginning
	JPGStream->seek(0, IStream::begin);

	cinfo->src->next_input_byte = (unsigned char *)JPGBuffer;
	cinfo->src->bytes_in_buffer = 0;
}

static boolean jpgDecompressFill(j_decompress_ptr cinfo)
{
	uint length = std::min(JPGBufferSize, JPGStreamSize - JPGStream->getPos());

	try
	{
		JPGStream->serialBuffer((uint8*) JPGBuffer, length);
	}
	catch(...)
	{
		nlwarning("error while reading JPEG image");
		cinfo->src->next_input_byte = (unsigned char *)JPGBuffer;
		cinfo->src->bytes_in_buffer = 0;
		return FALSE;
	}

	cinfo->src->next_input_byte = (unsigned char *)JPGBuffer;
	cinfo->src->bytes_in_buffer = length;
	return TRUE;
}

static void jpgDecompressSkip(j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes > 0)
	{
		while (num_bytes > (long) cinfo->src->bytes_in_buffer)
		{
			num_bytes -= (long) cinfo->src->bytes_in_buffer;
			jpgDecompressFill(cinfo);
		}

		cinfo->src->next_input_byte += (size_t) num_bytes;
		cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
	}
}

static void jpgDecompressTerm(j_decompress_ptr /* cinfo */)
{
}

static jpeg_source_mgr jpgSourceManager = { NULL, 0,
	jpgDecompressInit, jpgDecompressFill, jpgDecompressSkip, jpeg_resync_to_restart, jpgDecompressTerm };

/*-------------------------------------------------------------------*\
							readJPG
\*-------------------------------------------------------------------*/
uint8 CBitmap::readJPG( NLMISC::IStream &f )
{
	if(!f.isReading()) return false;

	struct jpeg_decompress_struct cinfo;

	// set up errors manager
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		nlwarning("failed to setjump");
		return 0;
	}

	// set the stream to read from
	JPGStream = &f;

	// init decompress
	jpeg_create_decompress(&cinfo);
	cinfo.src = &jpgSourceManager;

	// read header of image
	if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
	{
		jpeg_destroy_decompress(&cinfo);
		nlwarning("failed to read header");
		return 0;
	}

	uint dstChannels, srcChannels;

	if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
	{
		dstChannels = 1;
		srcChannels = 1;
		resize (cinfo.image_width, cinfo.image_height, _LoadGrayscaleAsAlpha ? Alpha : Luminance);
	}
	else
	{
		// force conversion of color spaces in RGB
		dstChannels = 4;
		srcChannels = 3;
		cinfo.out_color_space = JCS_RGB;
		resize (cinfo.image_width, cinfo.image_height, RGBA);
	}

	// start decompression of image data
	if (!jpeg_start_decompress(&cinfo))
	{
		jpeg_destroy_decompress(&cinfo);
		nlwarning("failed to start decompressing");
		return 0;
	}

	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo,
		JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

	uint i, j;

	while (cinfo.output_scanline < cinfo.output_height)
	{
		const uint offset = cinfo.output_scanline * _Width * dstChannels;

		if (jpeg_read_scanlines(&cinfo, buffer, 1) != 1)
		{
			nlwarning("failed to read scanline");
			break;
		}

		for (i = 0; i < _Width; i++)
		{
			for (j = 0; j < srcChannels; ++j)
				_Data[0][offset+i*dstChannels+j] = buffer[0][i*srcChannels+j];

			if (PixelFormat == RGBA)
				_Data[0][offset+i*dstChannels+j] = 255;
		}
	}

	if (!jpeg_finish_decompress(&cinfo))
		nlwarning("failed to finish decompressing");

	jpeg_destroy_decompress(&cinfo);

	JPGStream = NULL;

	return uint8(srcChannels * 8);
}

static void jpgCompressInit(j_compress_ptr cinfo)
{
	cinfo->dest->next_output_byte = (unsigned char *)JPGBuffer;
	cinfo->dest->free_in_buffer = JPGBufferSize;
}

static boolean jpgCompressEmpty(j_compress_ptr cinfo)
{
	JPGStream->serialBuffer((uint8*) JPGBuffer, JPGBufferSize);
	cinfo->dest->next_output_byte = (unsigned char *)JPGBuffer;
	cinfo->dest->free_in_buffer = JPGBufferSize;
	return TRUE;
}

static void jpgCompressTerm(j_compress_ptr cinfo)
{
	if(JPGBufferSize - cinfo->dest->free_in_buffer > 0)
		JPGStream->serialBuffer((uint8*) JPGBuffer, (uint)(JPGBufferSize - cinfo->dest->free_in_buffer));
}

static jpeg_destination_mgr jpgDestinationManager = { 0, 0,
	jpgCompressInit, jpgCompressEmpty, jpgCompressTerm };

/*-------------------------------------------------------------------*\
							writeJPG
\*-------------------------------------------------------------------*/
bool CBitmap::writeJPG( NLMISC::IStream &f, uint8 quality)
{
	if (f.isReading()) return false;

	if (PixelFormat > AlphaLuminance) return false;
	if (!_Width || !_Height) return false;

	struct jpeg_compress_struct cinfo;

	// set up errors manager
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_compress(&cinfo);
		nlwarning("failed to setjump");
		return false;
	}

	// set the stream to write to
	JPGStream = &f;

	// init compress
	jpeg_create_compress(&cinfo);

	uint srcChannels, dstChannels;

	if (PixelFormat == RGBA)
	{
		srcChannels = 4;
		dstChannels = cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
	}
	else
	{
		srcChannels = PixelFormat == AlphaLuminance ? 2:1;
		dstChannels = cinfo.input_components = 1;
		cinfo.in_color_space = JCS_GRAYSCALE;
	}

	cinfo.image_width = _Width;
	cinfo.image_height = _Height;
	cinfo.dest = &jpgDestinationManager;

	// set default compression parameters
	jpeg_set_defaults(&cinfo);

	// set image quality
	jpeg_set_quality(&cinfo, quality, TRUE);

	// start to compress image
	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer[1];
	row_pointer[0] = new uint8[_Width*dstChannels];

	uint i, j;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		const uint offset = cinfo.next_scanline * _Width * srcChannels;

		for (i = 0; i < _Width; ++i)
		{
			for (j = 0; j < dstChannels; ++j)
			{
				row_pointer[0][i*dstChannels+j] = (uint8) _Data[0][offset + i*srcChannels+j];
			}
		}

		// write image scanline
		if (jpeg_write_scanlines(&cinfo, row_pointer, 1) != 1)
		{
			nlwarning("failed to write scanline");
			break;
		}
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	delete row_pointer[0];
	row_pointer[0] = NULL;
	JPGStream = NULL;

	return true;
}

#else

bool CBitmap::writeJPG( NLMISC::IStream &/* f */, uint8 /* quality */)
{
	nlwarning ("You must compile NLMISC with USE_JPEG if you want jpeg support");
	return false;
}

uint8 CBitmap::readJPG( NLMISC::IStream &/* f */)
{
	nlwarning ("You must compile NLMISC with USE_JPEG if you want jpeg support");
	return 0;
}

#endif

}
