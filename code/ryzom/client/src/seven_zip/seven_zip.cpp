// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "seven_zip.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "7z.h"
#include "7zAlloc.h"
#include "7zCrc.h"
#include "7zVersion.h"
#include "LzmaLib.h"

#include <memory>

//
// Namespaces
//

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif


/// Input stream class for 7zip archive
class CNel7ZipInStream : public ISeekInStream
{
	NLMISC::IStream		*_Stream;

public:
	/// Constructor, only allow file stream because 7zip will 'seek' in the stream
	CNel7ZipInStream(NLMISC::IStream *s)
		:	_Stream(s)
	{
		Read = readFunc;
		Seek = seekFunc;
	}

	// the read function called by 7zip to read data
	static SRes readFunc(const ISeekInStream *object, void *buffer, size_t *size)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			uint len = (uint)*size;
			me->_Stream->serialBuffer((uint8*)buffer, len);
			return SZ_OK;
		}
		catch (...)
		{
			return SZ_ERROR_READ;
		}
	}

	// the seek function called by seven zip to seek inside stream
	static SRes seekFunc(const ISeekInStream *object, Int64 *pos, ESzSeek origin)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			sint32 offset = (sint32)*pos;
			bool ret = me->_Stream->seek(offset, (NLMISC::IStream::TSeekOrigin)origin);

			if (ret)
			{
				*pos = (Int64)me->_Stream->getPos();
				return SZ_OK;
			}
		}
		catch (...)
		{
		}

		return SZ_ERROR_READ;
	}
};

bool unpack7Zip(const std::string &sevenZipFile, const std::string &destFileName)
{
	nlinfo("Uncompressing 7zip archive '%s' to '%s'", sevenZipFile.c_str(),	destFileName.c_str());

	// init seven zip
	ISzAlloc allocImp;
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	ISzAlloc allocTempImp;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	// wrap file in a CIFile
	CIFile input(sevenZipFile);
	CNel7ZipInStream inStr(&input);

	CLookToRead2 lookStream;
	lookStream.realStream = &inStr;
	LookToRead2_CreateVTable(&lookStream, False);

	size_t bufferSize = 1024;

	{
		lookStream.buf = (Byte*)ISzAlloc_Alloc(&allocImp, bufferSize);

		if (!lookStream.buf)
		{
			nlerror("Unable to allocate %zu bytes", bufferSize);
			return false;
		}

		lookStream.bufSize = bufferSize;
		lookStream.realStream = &inStr;
		LookToRead2_Init(&lookStream);
	}

	CrcGenerateTable();

	CSzArEx db;
	SzArEx_Init(&db);

	// unpack the file using the 7zip API
	SRes res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);

	if (res != SZ_OK)
	{
		nlerror("Failed to open archive file %s", sevenZipFile.c_str());
		return false;
	}

	if (db.NumFiles != 1)
	{
		nlerror("Seven zip archive with more than 1 file are unsupported");
		return false;
	}

	UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize = 0; /* it can have any value before first call (if outBuffer = 0) */

	size_t offset;
	size_t outSizeProcessed = 0;

	// get the first file
	res = SzArEx_Extract(&db, &lookStream.vt, 0, &blockIndex, &outBuffer, &outBufferSize, &offset, &outSizeProcessed, &allocImp, &allocTempImp);

	// get the length of first file
	size_t nameLen = SzArEx_GetFileNameUtf16(&db, 0, NULL);

	ucstring filename;
	filename.resize(nameLen);

	// write filename into ucstring
	SzArEx_GetFileNameUtf16(&db, 0, &filename[0]);

	// write the extracted file
	FILE *outputHandle = nlfopen(destFileName, "wb+");

	if (outputHandle == 0)
	{
		nlerror("Can not open output file '%s'", destFileName.c_str());
		return false;
	}

	UInt32 processedSize = (UInt32)fwrite(outBuffer + offset, 1, outSizeProcessed, outputHandle);

	if (processedSize != outSizeProcessed)
	{
		nlerror("Failed to write %u char to output file '%s'", outSizeProcessed-processedSize, destFileName.c_str());
		return false;
	}

	fclose(outputHandle);

	IAlloc_Free(&allocImp, outBuffer);

	// free 7z context
	SzArEx_Free(&db, &allocImp);

	// ok, all is fine, file is unpacked
	return true;
}

bool unpackLZMA(const std::string &lzmaFile, const std::string &destFileName)
{
	nldebug("unpackLZMA: decompress LZMA file '%s' to '%s", lzmaFile.c_str(), destFileName.c_str());

	// open input file
	CIFile inStream(lzmaFile);
	uint32 inSize = inStream.getFileSize();

	if (inSize < LZMA_PROPS_SIZE + 8)
	{
		nlwarning("unpackLZMA: Invalid file size, too small file '%s'", lzmaFile.c_str());
		return false;
	}

	// allocate input buffer for props
	std::vector<uint8> propsBuffer(LZMA_PROPS_SIZE);

	// size of LZMA content
	inSize -= LZMA_PROPS_SIZE + 8;

	// allocate input buffer for lzma data
	std::vector<uint8> inBuffer(inSize);

	uint64 fileSize = 0;

	try
	{
		// read props
		inStream.serialBuffer(&propsBuffer[0], LZMA_PROPS_SIZE);

		// read uncompressed size
		inStream.serial(fileSize);

		// read lzma content
		inStream.serialBuffer(&inBuffer[0], inSize);
	}
	catch(const EReadError &e)
	{
		nlwarning("unpackLZMA: Error while reading '%s': %s", lzmaFile.c_str(), e.what());
		return false;
	}

	// allocate the output buffer
	std::vector<uint8> outBuffer(fileSize);

	// in and out file sizes
	SizeT outProcessed = (SizeT)fileSize;
	SizeT inProcessed = (SizeT)inSize;

	// decompress the file in memory
	sint res = LzmaUncompress(&outBuffer[0], &outProcessed, &inBuffer[0], &inProcessed, &propsBuffer[0], LZMA_PROPS_SIZE);

	if (res != 0 || outProcessed != fileSize)
	{
		nlwarning("unpackLZMA: Failed to decode lzma file '%s' with status %d", lzmaFile.c_str(), res);
		return false;
	}

	// store on output buffer
	COFile outStream(destFileName);

	try
	{
		// write content
		outStream.serialBuffer(&outBuffer[0], (uint)fileSize);
	}
	catch(const EFile &e)
	{
		nlwarning("unpackLZMA: Error while writing '%s': %s", destFileName.c_str(), e.what());
		CFile::deleteFile(destFileName);
		return false;
	}

	return true;
}

bool packLZMA(const std::string &srcFileName, const std::string &lzmaFileName)
{
	nldebug("packLZMA: compress '%s' to LZMA file '%s", srcFileName.c_str(), lzmaFileName.c_str());

	// open file
	CIFile inStream(srcFileName);
	size_t inSize = inStream.getFileSize();

	// file empty
	if (!inSize)
	{
		nlwarning("packLZMA: File '%s' not found or empty", srcFileName.c_str());
		return false;
	}

	// allocate input buffer
	std::vector<uint8> inBuffer(inSize);

	try
	{
		// read file in buffer
		inStream.serialBuffer(&inBuffer[0], inSize);
	}
	catch(const EReadError &e)
	{
		nlwarning("packLZMA: Error while reading '%s': %s", srcFileName.c_str(), e.what());
		return false;
	}

	// allocate output buffer
	size_t outSize = (11 * inSize / 10) + 65536; // worst case = 1.1 * size + 64K
	std::vector<uint8> outBuffer(outSize);

	// allocate buffer for props
	size_t outPropsSize = LZMA_PROPS_SIZE;
	std::vector<uint8> outProps(outPropsSize);

	// compress with best compression and other default settings
	sint res = LzmaCompress(&outBuffer[0], &outSize, &inBuffer[0], inSize, &outProps[0], &outPropsSize, 9, 1 << 27, -1, -1, -1, -1, 1);

	switch(res)
	{
		case SZ_OK:
		{
			// store on output buffer
			COFile outStream(lzmaFileName);

			// unable to create file
			if (!outStream.isOpen())
			{
				nlwarning("packLZMA: Unable to create '%s'", srcFileName.c_str());
				return false;
			}

			try
			{
				// write props
				outStream.serialBuffer(&outProps[0], (uint)outPropsSize);

				// write uncompressed size
				uint64 uncompressSize = inSize;
				outStream.serial(uncompressSize);

				// write content
				outStream.serialBuffer(&outBuffer[0], (uint)outSize);
			}
			catch(const EFile &e)
			{
				nlwarning("packLZMA: Error while writing '%s': %s", lzmaFileName.c_str(), e.what());
				CFile::deleteFile(lzmaFileName);
				return false;
			}

			return true;
		}

		case SZ_ERROR_MEM:
		nlwarning("packLZMA: Memory allocation error while compressing '%s' (input buffer size: %u, output buffer size: %u)", srcFileName.c_str(), (uint)inSize, (uint)outSize);
		break;

		case SZ_ERROR_PARAM:
		nlwarning("packLZMA: Incorrect parameter while compressing '%s'", srcFileName.c_str());
		break;

		case SZ_ERROR_OUTPUT_EOF:
		nlwarning("packLZMA: Output buffer overflow while compressing '%s' (input buffer size: %u, output buffer size: %u)", srcFileName.c_str(), (uint)inSize, (uint)outSize);
		break;

		case SZ_ERROR_THREAD:
		nlwarning("packLZMA: Errors in multithreading functions (only for Mt version)");
		break;

		default:
		nlwarning("packLZMA: Unknown error (%d) while compressing '%s'", res, srcFileName.c_str());
	}

	return false;
}
