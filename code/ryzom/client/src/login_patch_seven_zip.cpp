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

//
// Includes
//

#include "stdpch.h"
#include "login_patch.h"

#define RZ_USE_SEVENZIP 1
//#define RZ_USE_NEW_LZMA

#ifdef RZ_USE_SEVENZIP

#ifdef RZ_USE_NEW_LZMA

#include "seven_zip/7z.h"
#include "seven_zip/7zAlloc.h"
#include "seven_zip/7zBuf.h"
#include "seven_zip/7zCrc.h"
#include "seven_zip/7zFile.h"
#include "seven_zip/7zVersion.h"
#include "seven_zip/LzmaDec.h"

#else

#include "seven_zip/7zCrc.h"
#include "seven_zip/7zIn.h"
#include "seven_zip/7zExtract.h"
#include "seven_zip/LzmaDecode.h"

#endif

//
// Namespaces
//

using namespace std;
using namespace NLMISC;


#ifdef RZ_USE_NEW_LZMA

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
	static SRes readFunc(void *object, void *buffer, size_t *size)
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
	static SRes seekFunc(void *object, Int64 *pos, ESzSeek origin)
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

#else

/// Input stream class for 7zip archive
class CNel7ZipInStream : public _ISzInStream
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
	static SZ_RESULT readFunc(void *object, void *buffer, size_t size, size_t *processedSize)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			me->_Stream->serialBuffer((uint8*)buffer, (uint)size);
			*processedSize = size;
			return SZ_OK;
		}
		catch (...)
		{
			return SZE_FAIL;
		}
	}

	// the seek function called by seven zip to seek inside stream
	static SZ_RESULT seekFunc(void *object, CFileSize pos)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			bool ret= me->_Stream->seek(pos, NLMISC::IStream::begin);
			if (ret)
				return SZ_OK;
			else
				return SZE_FAIL;
		}
		catch (...)
		{
			return SZE_FAIL;
		}
	}
};

#endif

#endif

bool CPatchManager::unpack7Zip(const std::string &sevenZipFile, const std::string &destFileName)
{
#ifdef RZ_USE_SEVENZIP
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

#ifdef RZ_USE_NEW_LZMA

	CLookToRead lookStream;
	lookStream.realStream = &inStr;
	LookToRead_CreateVTable(&lookStream, False);
	LookToRead_Init(&lookStream);

	CrcGenerateTable();

	CSzArEx db;
	SzArEx_Init(&db);

	// unpack the file using the 7zip API
	SRes res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);

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
	res = SzArEx_Extract(&db, &lookStream.s, 0,
		&blockIndex, &outBuffer, &outBufferSize,
		&offset, &outSizeProcessed,
		&allocImp, &allocTempImp);

	// get the length of first file
	size_t nameLen = SzArEx_GetFileNameUtf16(&db, 0, NULL);

	ucstring filename;
	filename.resize(nameLen);

	// write filename into ucstring
	SzArEx_GetFileNameUtf16(&db, 0, &filename[0]);

	// write the extracted file
	FILE *outputHandle = fopen(destFileName.c_str(), "wb+");

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

#else

	InitCrcTable();

	CArchiveDatabaseEx db;
	SzArDbExInit(&db);

	// unpack the file using the 7zip API
	SZ_RESULT res = SzArchiveOpen(&inStr, &db, &allocImp, &allocTempImp);

	if (res != SZ_OK)
	{
		nlerror("Failed to open archive file %s", sevenZipFile.c_str());
		return false;
	}

	if (db.Database.NumFiles != 1)
	{
		nlerror("Seven zip archive with more than 1 file are unsupported");
		return false;
	}

	UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize = 0; /* it can have any value before first call (if outBuffer = 0) */

	size_t offset = 0;
	size_t outSizeProcessed = 0;

	// get the first file
	res = SzExtract(&inStr, &db, 0,
		&blockIndex, &outBuffer, &outBufferSize,
		&offset, &outSizeProcessed,
		&allocImp, &allocTempImp);

	// write the extracted file
	FILE *outputHandle = fopen(destFileName.c_str(), "wb+");

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
	allocImp.Free(outBuffer);

	// free 7z context
	SzArDbExFree(&db, allocImp.Free);

#endif

	// ok, all is fine, file is unpacked
	return true;
#else
	return false;
#endif
}

bool CPatchManager::unpackLZMA(const std::string &lzmaFile, const std::string &destFileName)
{
#ifdef RZ_USE_SEVENZIP
	nldebug("unpackLZMA : decompression the lzma file '%s' into output file '%s", lzmaFile.c_str(), destFileName.c_str());

	CIFile inStream(lzmaFile);
	uint32 inSize = inStream.getFileSize();

#ifdef RZ_USE_NEW_LZMA

	if (inSize < LZMA_PROPS_SIZE + 8)
	{
		nlwarning("Invalid file size, too small file '%s'", lzmaFile.c_str());
		return false;
	}

	// read the compressed file in buffer
	auto_ptr<uint8> inBuffer = auto_ptr<uint8>(new uint8[inSize]);
	inStream.serialBuffer(inBuffer.get(), inSize);

	uint8 *pos = inBuffer.get();

	pos += LZMA_PROPS_SIZE;

	// read the output file size
	size_t fileSize = 0;

	for (int i = 0; i < 8; ++i)
	{
		if (pos >= inBuffer.get()+inSize)
		{
			nlassert(false);
			return false;
		}

		fileSize |= ((UInt64)*pos++) << (8 * i);
	}

	// allocators
	ISzAlloc allocImp;
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	CLzmaDec state;
	LzmaDec_Construct(&state);

	// allocate and decode props and probs
	SRes res = LzmaDec_Allocate(&state, inBuffer.get(), LZMA_PROPS_SIZE, &allocImp);

	if (res != 0)
	{
		nlwarning("Failed to decode lzma properies in file '%s'!", lzmaFile.c_str());
		return false;
	}

	// allocate the output buffer
	auto_ptr<uint8> outBuffer = auto_ptr<uint8>(new uint8[fileSize]);

	// in and out file sizes
	SizeT outProcessed = fileSize;
	SizeT inProcessed = (SizeT)(inSize-(pos-inBuffer.get()));

	LzmaDec_Init(&state);

	// decompress the file in memory
	ELzmaStatus status;
	res = LzmaDec_DecodeToBuf(&state, (Byte*)outBuffer.get(), &outProcessed, (Byte*)pos, &inProcessed, LZMA_FINISH_ANY, &status);

	// free memory
	LzmaDec_Free(&state, &allocImp);

	if (res != 0 || outProcessed != fileSize)
	{
		nlwarning("Failed to decode lzma file '%s' with status %d", lzmaFile.c_str(), (sint)status);
		return false;
	}

#else

	auto_ptr<uint8> inBuffer = auto_ptr<uint8>(new uint8[inSize]);
	inStream.serialBuffer(inBuffer.get(), inSize);

	CLzmaDecoderState state;

	uint8 *pos = inBuffer.get();

	// read the lzma properties
	int res = LzmaDecodeProperties(&state.Properties, (unsigned char*) pos, LZMA_PROPERTIES_SIZE);
	if (res != 0)
	{
		nlwarning("Failed to decode lzma properies in file '%s'!", lzmaFile.c_str());
		return false;
	}

	if (inSize < LZMA_PROPERTIES_SIZE + 8)
	{
		nlwarning("Invalid file size, too small file '%s'", lzmaFile.c_str());
		return false;
	}

	// alloc the probs, making sure they are deleted in function exit
	size_t nbProb = LzmaGetNumProbs(&state.Properties);
	auto_ptr<CProb> probs = auto_ptr<CProb>(new CProb[nbProb]);
	state.Probs = probs.get();

	pos += LZMA_PROPERTIES_SIZE;

	// read the output file size
	size_t fileSize = 0;
	for (int i = 0; i < 8; i++)
	{
		if (pos >= inBuffer.get()+inSize)
		{
			nlassert(false);
			return false;
		}
		fileSize |= ((UInt64)*pos++) << (8 * i);
	}

	SizeT outProcessed = 0;
	SizeT inProcessed = 0;

	// allocate the output buffer
	auto_ptr<uint8> outBuffer = auto_ptr<uint8>(new uint8[fileSize]);

	// decompress the file in memory
	res = LzmaDecode(&state, (unsigned char*) pos, (SizeT)(inSize-(pos-inBuffer.get())), &inProcessed, (unsigned char*)outBuffer.get(), (SizeT)fileSize, &outProcessed);

	if (res != 0 || outProcessed != fileSize)
	{
		nlwarning("Failed to decode lzma file '%s'", lzmaFile.c_str());
		return false;
	}

#endif

	// store on output buffer
	COFile outStream(destFileName);
	outStream.serialBuffer(outBuffer.get(), (uint)fileSize);

	return true;
#else
	return false;
#endif
}
