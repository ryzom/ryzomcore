// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// KTX1 file format identifier bytes [4..11] (first 4 bytes already consumed as KTX_HEADER)
static const uint8 KTX_ID_REST[8] = { 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

// GL constants for texture formats used in KTX files
static const uint32 GL_UNSIGNED_BYTE_                          = 0x1401;
static const uint32 GL_ALPHA_                                  = 0x1906;
static const uint32 GL_RGB_                                    = 0x1907;
static const uint32 GL_RGBA_                                   = 0x1908;
static const uint32 GL_LUMINANCE_                              = 0x1909;
static const uint32 GL_LUMINANCE_ALPHA_                        = 0x190A;
static const uint32 GL_COMPRESSED_RGB_S3TC_DXT1_EXT_           = 0x83F0;
static const uint32 GL_COMPRESSED_RGBA_S3TC_DXT1_EXT_          = 0x83F1;
static const uint32 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT_          = 0x83F2;
static const uint32 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT_          = 0x83F3;
static const uint32 GL_ETC1_RGB8_OES_                          = 0x8D64;

static void ktxSwap32(uint32 &val)
{
	val = ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

/*-------------------------------------------------------------------*\
							readKTX
\*-------------------------------------------------------------------*/
uint8 CBitmap::readKTX( NLMISC::IStream &f, uint mipMapSkip )
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	//-------------- Read and validate KTX identifier

	// First 4 bytes already consumed by load() as KTX_HEADER
	// Read and verify remaining 8 bytes of the KTX1 identifier
	uint8 idRest[8];
	f.serialBuffer(idRest, 8);
	if (memcmp(idRest, KTX_ID_REST, 8) != 0)
	{
		nlwarning("Invalid KTX identifier");
		return 0;
	}

	//-------------- Read KTX header fields

	uint32 endianness;
	uint32 glType;
	uint32 glTypeSize;
	uint32 glFormat;
	uint32 glInternalFormat;
	uint32 glBaseInternalFormat;
	uint32 pixelWidth;
	uint32 pixelHeight;
	uint32 pixelDepth;
	uint32 numberOfArrayElements;
	uint32 numberOfFaces;
	uint32 numberOfMipmapLevels;
	uint32 bytesOfKeyValueData;

	f.serial(endianness);
	bool mustSwap = (endianness == 0x01020304);

	f.serial(glType);
	f.serial(glTypeSize);
	f.serial(glFormat);
	f.serial(glInternalFormat);
	f.serial(glBaseInternalFormat);
	f.serial(pixelWidth);
	f.serial(pixelHeight);
	f.serial(pixelDepth);
	f.serial(numberOfArrayElements);
	f.serial(numberOfFaces);
	f.serial(numberOfMipmapLevels);
	f.serial(bytesOfKeyValueData);

	if (mustSwap)
	{
		ktxSwap32(glType);
		ktxSwap32(glTypeSize);
		ktxSwap32(glFormat);
		ktxSwap32(glInternalFormat);
		ktxSwap32(glBaseInternalFormat);
		ktxSwap32(pixelWidth);
		ktxSwap32(pixelHeight);
		ktxSwap32(pixelDepth);
		ktxSwap32(numberOfArrayElements);
		ktxSwap32(numberOfFaces);
		ktxSwap32(numberOfMipmapLevels);
		ktxSwap32(bytesOfKeyValueData);
	}

	//-------------- Validate basic constraints

	// Only 2D textures supported (no 3D, no arrays, no cubemaps)
	if (pixelDepth > 1)
	{
		nlwarning("KTX: 3D textures not supported");
		return 0;
	}
	if (numberOfArrayElements > 1)
	{
		nlwarning("KTX: texture arrays not supported");
		return 0;
	}
	if (numberOfFaces > 1)
	{
		nlwarning("KTX: cubemap textures not supported");
		return 0;
	}

	if (pixelWidth == 0 || pixelHeight == 0)
	{
		nlwarning("KTX: invalid texture dimensions");
		return 0;
	}

	// 0 means 1 mipmap level (auto-generate)
	if (numberOfMipmapLevels == 0)
		numberOfMipmapLevels = 1;

	//-------------- Skip key/value data
	if (bytesOfKeyValueData > 0)
	{
		if (!f.seek(bytesOfKeyValueData, IStream::current))
		{
			nlwarning("KTX: failed to skip key/value data");
			return 0;
		}
	}

	//-------------- Determine pixel format

	bool compressed = (glType == 0 && glFormat == 0);
	uint8 imageDepth = 0;
	uint32 srcBpp = 0; // source bytes per pixel for uncompressed formats

	if (compressed)
	{
		switch (glInternalFormat)
		{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT_:
			PixelFormat = DXTC1;
			imageDepth = 24;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT_:
			PixelFormat = DXTC1Alpha;
			imageDepth = 32;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT_:
			PixelFormat = DXTC3;
			imageDepth = 32;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT_:
			PixelFormat = DXTC5;
			imageDepth = 32;
			break;
		case GL_ETC1_RGB8_OES_:
			nlwarning("KTX: ETC1 compressed format not yet supported by bitmap loader");
			return 0;
		default:
			nlwarning("KTX: unsupported compressed glInternalFormat 0x%x", glInternalFormat);
			return 0;
		}
	}
	else
	{
		// Uncompressed formats
		switch (glBaseInternalFormat)
		{
		case GL_RGBA_:
			PixelFormat = RGBA;
			imageDepth = 32;
			srcBpp = 4;
			break;
		case GL_RGB_:
			// We will expand RGB to RGBA
			PixelFormat = RGBA;
			imageDepth = 24;
			srcBpp = 3;
			break;
		case GL_LUMINANCE_:
			if (_LoadGrayscaleAsAlpha)
			{
				PixelFormat = Alpha;
				imageDepth = 8;
			}
			else
			{
				PixelFormat = Luminance;
				imageDepth = 8;
			}
			srcBpp = 1;
			break;
		case GL_ALPHA_:
			PixelFormat = Alpha;
			imageDepth = 8;
			srcBpp = 1;
			break;
		case GL_LUMINANCE_ALPHA_:
			PixelFormat = AlphaLuminance;
			imageDepth = 16;
			srcBpp = 2;
			break;
		default:
			nlwarning("KTX: unsupported uncompressed glBaseInternalFormat 0x%x", glBaseInternalFormat);
			return 0;
		}
	}

	//-------------- Setup dimensions

	_Width = pixelWidth;
	_Height = pixelHeight;
	_MipMapCount = (uint8)numberOfMipmapLevels;
	if (_MipMapCount > MAX_MIPMAP)
		_MipMapCount = MAX_MIPMAP;

	//-------------- Handle mipmap skip for compressed formats

	if (compressed && _MipMapCount > 1 && mipMapSkip > 0)
	{
		uint minSizeLevel = min(_Width, _Height);
		minSizeLevel = getPowerOf2(minSizeLevel);

		if (minSizeLevel > 2)
		{
			mipMapSkip = min(mipMapSkip, minSizeLevel - 2);

			while (mipMapSkip > 0 && _MipMapCount > 1)
			{
				// Read imageSize for this mipmap level
				uint32 imageSize;
				f.serial(imageSize);
				if (mustSwap) ktxSwap32(imageSize);

				// Skip the data + padding
				uint32 imageSizePadded = (imageSize + 3) & ~3u;
				if (!f.seek(imageSizePadded, IStream::current))
				{
					nlwarning("KTX: failed to skip mipmap data");
					return 0;
				}

				_Width >>= 1;
				if (_Width == 0) _Width = 1;
				_Height >>= 1;
				if (_Height == 0) _Height = 1;
				_MipMapCount--;
				mipMapSkip--;
			}
		}
	}

	//-------------- Read mipmap levels

	for (uint8 m = 0; m < _MipMapCount; m++)
	{
		uint32 imageSize;
		f.serial(imageSize);
		if (mustSwap) ktxSwap32(imageSize);

		uint32 w = max(_Width >> m, 1u);
		uint32 h = max(_Height >> m, 1u);

		if (compressed)
		{
			// For DXTC compressed data, store directly
			_Data[m].resize(imageSize);
			f.serialBuffer(_Data[m].getPtr(), imageSize);
		}
		else
		{
			// For uncompressed data, read and convert to our internal format
			uint32 srcSize = w * h * srcBpp;
			if (imageSize < srcSize)
			{
				nlwarning("KTX: imageSize %u too small for %ux%u with %u bpp", imageSize, w, h, srcBpp);
				return 0;
			}

			vector<uint8> srcData(imageSize);
			f.serialBuffer(&srcData[0], imageSize);

			uint32 dstBpp = bitPerPixels[PixelFormat] / 8;
			uint32 dstSize = w * h * dstBpp;
			_Data[m].resize(dstSize);

			if (glBaseInternalFormat == GL_RGB_)
			{
				// Expand RGB to RGBA
				for (uint32 p = 0; p < w * h; p++)
				{
					_Data[m][p * 4 + 0] = srcData[p * 3 + 0];
					_Data[m][p * 4 + 1] = srcData[p * 3 + 1];
					_Data[m][p * 4 + 2] = srcData[p * 3 + 2];
					_Data[m][p * 4 + 3] = 255;
				}
			}
			else
			{
				// Direct copy (RGBA, Luminance, Alpha, LuminanceAlpha)
				memcpy(_Data[m].getPtr(), &srcData[0], dstSize);
			}
		}

		// Skip padding to 4-byte boundary
		uint32 imageSizePadded = (imageSize + 3) & ~3u;
		uint32 padding = imageSizePadded - imageSize;
		if (padding > 0)
		{
			if (!f.seek(padding, IStream::current))
			{
				nlwarning("KTX: failed to skip padding");
				return 0;
			}
		}
	}

	return imageDepth;
}

} // namespace NLMISC
