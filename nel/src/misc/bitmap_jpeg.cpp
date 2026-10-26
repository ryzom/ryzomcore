// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
	#ifdef NL_COMP_MINGW
	#	define HAVE_BOOLEAN
	#endif
	#include <jpeglib.h>
	#include <jerror.h>
}
#endif

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

#ifdef USE_JPEG

// the stream and buffer of writeJPG (readJPG has its own, see CJPGSource)
static NLMISC::IStream *JPGStream = nullptr;
static const uint32 JPGBufferSize = 4096;
static char JPGBuffer[JPGBufferSize];

struct my_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

// libjpeg's messages (the warnings about corrupt data, and the error before my_error_exit) go to the log
static void jpgOutputMessage(j_common_ptr cinfo)
{
	char message[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, message);
	nlwarning("JPEG: %s", message);
}

void my_error_exit(j_common_ptr cinfo)
{
	my_error_mgr *myerr = (my_error_mgr *) cinfo->err;

	(*cinfo->err->output_message)(cinfo);

	longjmp(myerr->setjmp_buffer, 1);
}

// A progressive JPEG can repeat empty scans, and each walks every block of the image: 10000 of them, 100 KiB, took 8 s
// to decode at 4096x4096. TurboJPEG refuses more than 500 scans (TJFLAG_LIMITSCANS), readJPG does too.
static const int s_JPGMaxScans = 500;

static void jpgProgressMonitor(j_common_ptr cinfo)
{
	if (cinfo->is_decompressor && ((j_decompress_ptr)cinfo)->input_scan_number > s_JPGMaxScans)
	{
		nlwarning("JPEG: more than %d scans", s_JPGMaxScans);
		longjmp(((my_error_mgr *)cinfo->err)->setjmp_buffer, 1);
	}
}

// The source of one decompression: the stream and the buffer it is read into. They were statics shared by every
// decompression, which a texture loading thread can run beside the main thread.
struct CJPGSource
{
	jpeg_source_mgr Pub; // first, libjpeg only knows the source as cinfo->src
	NLMISC::IStream *Stream;
	uint32 StreamSize;
	JOCTET Buffer[JPGBufferSize];
};

static void jpgDecompressInit(j_decompress_ptr cinfo)
{
	CJPGSource *src = (CJPGSource *)cinfo->src;

	// get stream size if possible
	if (src->Stream->seek(0, IStream::end))
		src->StreamSize = src->Stream->getPos();
	else
		nlwarning("can't get JPEG stream size");

	// reset current position to the beginning
	src->Stream->seek(0, IStream::begin);

	src->Pub.next_input_byte = src->Buffer;
	src->Pub.bytes_in_buffer = 0;
}

static boolean jpgDecompressFill(j_decompress_ptr cinfo)
{
	CJPGSource *src = (CJPGSource *)cinfo->src;

	uint32 pos = (uint32)src->Stream->getPos();
	uint32 length = pos < src->StreamSize ? std::min(JPGBufferSize, src->StreamSize - pos) : 0;
	if (length > 0)
	{
		try
		{
			src->Stream->serialBuffer((uint8 *)src->Buffer, length);
		}
		catch (const NLMISC::Exception &e)
		{
			nlwarning("error while reading JPEG image: %s", e.what());
			length = 0;
		}
		catch (...)
		{
			// no exception may unwind through libjpeg
			nlwarning("error while reading JPEG image");
			length = 0;
		}
	}

	if (length == 0)
	{
		// The end of the data: libjpeg's convention is to warn and insert an end of image marker, which ends the image.
		// An empty buffer returned as filled made libjpeg read past it.
		WARNMS(cinfo, JWRN_JPEG_EOF);
		src->Buffer[0] = (JOCTET)0xff;
		src->Buffer[1] = (JOCTET)JPEG_EOI;
		length = 2;
	}

	src->Pub.next_input_byte = src->Buffer;
	src->Pub.bytes_in_buffer = length;
	return TRUE;
}

static void jpgDecompressSkip(j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes > 0)
	{
		// past the end of the data, each fill brings the 2 bytes of an end of image marker: this ends (it looped forever
		// when the fill brought nothing)
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

/*-------------------------------------------------------------------*\
							readJPG
\*-------------------------------------------------------------------*/
uint8 CBitmap::readJPG(NLMISC::IStream &f, uint32 maxSize)
{
	if(!f.isReading()) return false;

	struct jpeg_decompress_struct cinfo;

	// set up errors manager
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.output_message = jpgOutputMessage;

	// set the stream to read from
	CJPGSource source;
	source.Pub.next_input_byte = nullptr;
	source.Pub.bytes_in_buffer = 0;
	source.Pub.init_source = jpgDecompressInit;
	source.Pub.fill_input_buffer = jpgDecompressFill;
	source.Pub.skip_input_data = jpgDecompressSkip;
	source.Pub.resync_to_restart = jpeg_resync_to_restart;
	source.Pub.term_source = jpgDecompressTerm;
	source.Stream = &f;
	source.StreamSize = 0;

	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		nlwarning("Error while reading JPEG '%s'", f.getStreamName().c_str());
		return 0;
	}

	// init decompress
	jpeg_create_decompress(&cinfo);
	cinfo.src = &source.Pub;

	jpeg_progress_mgr progress;
	memset(&progress, 0, sizeof(progress));
	progress.progress_monitor = jpgProgressMonitor;
	cinfo.progress = &progress;

	// read header of image
	if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
	{
		jpeg_destroy_decompress(&cinfo);
		nlwarning("failed to read header");
		return 0;
	}

	if (cinfo.image_width > maxSize || cinfo.image_height > maxSize)
	{
		nlwarning("The JPEG image '%s' of %ux%u is larger than %u pixels a side", f.getStreamName().c_str(),
		    (uint)cinfo.image_width, (uint)cinfo.image_height, maxSize);
		jpeg_destroy_decompress(&cinfo);
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

static jpeg_destination_mgr jpgDestinationManager = { nullptr, 0,
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
	row_pointer[0] = nullptr;
	JPGStream = nullptr;

	return true;
}

#else

bool CBitmap::writeJPG( NLMISC::IStream &/* f */, uint8 /* quality */)
{
	nlwarning ("You must compile NLMISC with USE_JPEG if you want jpeg support");
	return false;
}

uint8 CBitmap::readJPG(NLMISC::IStream & /* f */, uint32 /* maxSize */)
{
	nlwarning ("You must compile NLMISC with USE_JPEG if you want jpeg support");
	return 0;
}

#endif

}
