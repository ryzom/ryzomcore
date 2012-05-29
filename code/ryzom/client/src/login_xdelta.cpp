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

#include "login_xdelta.h"

#include "nel/misc/file.h"

#ifdef NL_OS_WINDOWS
#include <io.h>
#endif
#include <fcntl.h>
#include <errno.h>


// ---------------------------------------------------------------------------

using namespace NLMISC;
using namespace std;

// ---------------------------------------------------------------------------
// ntohl like
static uint32 netToHost(uint32 src)
{
#ifdef NL_LITTLE_ENDIAN
	return ((src & 0x000000ff) << 24) + ((src & 0x0000ff00) << 8) + ((src & 0x00ff0000) >> 8) + ((src & 0xff000000) >> 24);
#else
	return src;
#endif
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// CXDPFileReader
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CXDPFileReader::CXDPFileReader()
{
	_GzFile = NULL;
	_File = NULL;
	_Pos = 0;
	_Optimize = false;
	_OptimPage = 1024*1024; // 1 mo
}

// ---------------------------------------------------------------------------
CXDPFileReader::~CXDPFileReader()
{
	if (_Compressed)
	{
		if (_Optimize)
		{
			freeZipMem();
		}
		else
		{
			if (_GzFile != NULL)
				gzclose(_GzFile);
		}
	}
	else
	{
		if (_File != NULL)
			fclose(_File);
	}
}

// ---------------------------------------------------------------------------
bool CXDPFileReader::init(const std::string &sFilename, sint32 nLowerBound, sint32 nUpperBound, bool bCompressed)
{
	_LowerBound = nLowerBound;
	_UpperBound = nUpperBound;
	_Compressed = bCompressed;

	if (bCompressed)
	{
		// First open the file with a normal function
#ifdef NL_OS_WINDOWS
		int fd = _open(sFilename.c_str(), _O_BINARY | _O_RDONLY);
#else
		int fd = open(sFilename.c_str(), O_RDONLY);
#endif
		if (fd == -1)
			return false;
#ifdef NL_OS_WINDOWS
		if (_lseek (fd, nLowerBound, SEEK_SET) == -1L)
#else
		if (lseek (fd, nLowerBound, SEEK_SET) == -1L)
#endif
		{
			nlwarning("%s: corrupt or truncated delta: cannot seek to %d", sFilename.c_str(), nLowerBound);
			return false;
		}
		_GzFile = gzdopen(fd, "rb");
		if (_GzFile == NULL)
		{
			nlwarning("gzdopen failed");
			return false;
		}

		if (_Optimize)
		{
			freeZipMem();
			for(;;)
			{
				uint8 *newBuf = new uint8[_OptimPage];
				int nbBytesRead = gzread(_GzFile, newBuf, _OptimPage);
				if (nbBytesRead == 0)
				{
					delete [] newBuf;
					break;
				}
				else
				{
					_ZipMem.push_back(newBuf);
					if (nbBytesRead < int(_OptimPage))
						break;
				}
			}
			gzclose(_GzFile);
			_GzFile = NULL;
		}

	}
	else
	{
		_File = fopen(sFilename.c_str(), "rb");
		if (_File == NULL)
			return false;
		fseek(_File, nLowerBound, SEEK_SET);
	}
	_Pos = 0;
	return true;
}
// ---------------------------------------------------------------------------
bool CXDPFileReader::read(uint8 *pBuf, sint32 nSize)
{
	if (_Compressed)
	{
		if (_Optimize)
		{
			while (nSize > 0)
			{
				uint32 nPage = _Pos / _OptimPage;
				uint32 nOffset = _Pos % _OptimPage;
				nlassert(nPage < _ZipMem.size());

				uint32 nSizeToRead;
				uint32 nSizeLeftInPage = _OptimPage - nOffset;
				if (nSize < sint32(nSizeLeftInPage))
					nSizeToRead = nSize;
				else
					nSizeToRead = nSizeLeftInPage;

				memcpy(pBuf, _ZipMem[nPage]+nOffset, nSizeToRead);

				nSize -= nSizeToRead;
				_Pos += nSizeToRead;
				pBuf += nSizeToRead;
			}
			return true;
		}
		else
		{
			return gzread(_GzFile, pBuf, nSize) == nSize;
		}
	}
	else
	{
		if (!_File) return false;
		return fread(pBuf, 1, nSize, _File) == (uint32)nSize;
	}
}

// ---------------------------------------------------------------------------
bool CXDPFileReader::readUInt32(uint32 &val)
{
	if (!read((uint8*)&val,4)) return false;
	return true;
}

// ---------------------------------------------------------------------------
bool CXDPFileReader::readUInt(uint32 &val)
{
	// This is mostly because I dislike endian, and less to save space on small ints
	uint8	c;
	uint8	arr[16];
	int		i = 0;
	int		donebit = 1;
	int		bits;

	while (read(&c, 1))
	{
		donebit = c & 0x80;
		bits = c & 0x7f;

		arr[i++] = bits;

		if (!donebit)
			break;
    }

	if (donebit)
		return false;

	val = 0;

	for (i -= 1; i >= 0; i -= 1)
	{
		val <<= 7;
		val |= arr[i];
    }

	return true;
}

// ---------------------------------------------------------------------------
bool CXDPFileReader::readBool(bool &val)
{
	uint8 nTmp;
	if (!read((uint8*)&nTmp,1)) return false;
	val = (nTmp != 0);
	return true;
}

// ---------------------------------------------------------------------------
bool CXDPFileReader::readString(std::string &s)
{
	uint32 nLen;
	s = "";
	if (!readUInt(nLen)) return false;
	for (uint32 i = 0; i < nLen; ++i)
	{
		uint8 c;
		if (!read(&c,1)) return false;
		s += c;
	}
	return true;
}

// ---------------------------------------------------------------------------
uint32 CXDPFileReader::getFileSize()
{
	if (_Compressed)
	{
		nlassert(true); // Not implemented for the moment
		return 0;
	}
	else
	{
		sint32 nPos = ftell(_File);
		if (nPos == -1) return 0;
		fseek(_File, 0, SEEK_END);
		sint32 nFileSize = ftell(_File);
		fseek(_File, nPos, SEEK_SET);
		return nFileSize;
	}
}

// ---------------------------------------------------------------------------
bool CXDPFileReader::seek(uint32 pos)
{
	if (_Compressed)
	{
		if (_Optimize)
		{
			_Pos = pos;
		}
		else
		{
			if (gzseek(_GzFile, pos, SEEK_SET) == -1)
				return false;
		}
	}
	else
	{
		if (!_File) return false;
		if (fseek(_File, pos, SEEK_SET) != 0)
			return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
void CXDPFileReader::freeZipMem()
{
	for (uint32 i = 0; i < _ZipMem.size(); ++i)
	{
		delete [] _ZipMem[i];
	}
	_ZipMem.clear();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// CXDeltaCtrl
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// SSourceInfo
// ---------------------------------------------------------------------------
bool SXDeltaCtrl::SSourceInfo::read(CXDPFileReader &fr)
{
	if (!fr.readString(Name)) return false;
	if (!fr.read(MD5.Data, 16)) return false;
	if (!fr.readUInt(Len)) return false;
	if (!fr.readBool(IsData)) return false;
	if (!fr.readBool(Sequential)) return false;

	return true;
}

// SXDeltaInst
// ---------------------------------------------------------------------------
bool SXDeltaCtrl::SInstruction::read(CXDPFileReader &fr)
{
	if (!fr.readUInt(Index)) return false;
	if (!fr.readUInt(Offset)) return false;
	if (!fr.readUInt(Length)) return false;

	return true;
}

// ---------------------------------------------------------------------------
bool SXDeltaCtrl::read(CXDPFileReader &fr)
{
	uint32 nType, nSize;
	if (!fr.readUInt32(nType)) return false;
	nType = netToHost(nType);
	if (nType != XDELTA_TYPE_CONTROL)
	{
		nlwarning("Bad Control type found");
		return false;
	}
	if (!fr.readUInt32(nSize)) return false;
	nSize = netToHost(nSize);

	// ----

	if (!fr.read(ToMD5.Data, 16)) return false;
	if (!fr.readUInt(ToLen)) return false;
	if (!fr.readBool(HasData)) return false;

	uint32 i, nSourceInfoLen, nInstLen;

	if (!fr.readUInt(nSourceInfoLen)) return false;
	SourceInfo.resize(nSourceInfoLen);
	for (i = 0; i < nSourceInfoLen; ++i)
		SourceInfo[i].read(fr);

	if (!fr.readUInt(nInstLen)) return false;
	Inst.resize(nInstLen);
	for (i = 0; i < nInstLen; ++i)
		Inst[i].read(fr);

	// /////////////////// //
	// Unpack Instructions //
	// /////////////////// //

	for (i = 0; i < SourceInfo.size(); ++i)
	{
		SSourceInfo &rInfo = SourceInfo[i];
		rInfo.Position = 0;
		rInfo.Copies = 0;
		rInfo.CopyLength = 0;
	}

	for (i = 0; i < Inst.size(); ++i)
	{
		SSourceInfo *pInfo = NULL;
		SInstruction *pInst = &Inst[i];

		if (pInst->Index >= SourceInfo.size())
		{
			nlwarning("Out Of Range Source Index : %d", pInst->Index);
			return false;
		}

		pInfo = &SourceInfo[pInst->Index];

		if (pInfo->Sequential)
		{
			pInst->Offset = pInfo->Position;
			pInfo->Position = pInst->Offset + pInst->Length;
		}

		pInfo->Copies += 1;
		pInfo->CopyLength += pInst->Length;
	}

	return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// CXDeltaPatch
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

CXDeltaPatch::ICallBack *CXDeltaPatch::_CallBack = NULL;

// ---------------------------------------------------------------------------
bool CXDeltaPatch::load(const string &sFilename)
{
	uint32 i;
	uint8 c;
	CIFile in;

	_FileName = sFilename;
	if (!in.open(sFilename))
		return false;

	uint8 vMagicBuffer[XDELTA_PREFIX_LEN];
	in.serialBuffer(vMagicBuffer, XDELTA_PREFIX_LEN);

	uint32 vHeader[XDELTA_HEADER_WORDS];
	in.serialBuffer((uint8*)&vHeader[0], XDELTA_HEADER_SPACE);

	for (i = 0; i < XDELTA_HEADER_WORDS; ++i)
		vHeader[i] = netToHost(vHeader[i]);

	// Check the version
	if (strncmp ((const char *)&vMagicBuffer[0], XDELTA_110_PREFIX, XDELTA_PREFIX_LEN) != 0)
	{
		nlwarning("%s bad version or not a delta patch", sFilename.c_str());
		return false;
	}

	_Version = "1.1";
	_Flags = vHeader[0];

	// Get names
	uint32 nFromNameLen = vHeader[1] >> 16;
	uint32 nToNameLen = vHeader[1] & 0xffff;

	_FromName = "";
	_ToName = "";

	for (i = 0; i < nFromNameLen; ++i)
	{
		in.serial(c);
		_FromName += c;
	}

	for (i = 0; i < nToNameLen; ++i)
	{
		in.serial(c);
		_ToName += c;
	}

	_HeaderOffset = in.getPos();

	// Go to the end of the file
	in.seek (0, NLMISC::IStream::end);
	uint32 nFileSize = in.getPos();
	uint32 nEndCtrlOffset = nFileSize-(4+XDELTA_PREFIX_LEN);
	in.seek (nEndCtrlOffset, NLMISC::IStream::begin);

	uint32 nCtrlOffset;
	in.serialBuffer((uint8*)&nCtrlOffset, 4);
	nCtrlOffset = netToHost(nCtrlOffset);
	_CtrlOffset = nCtrlOffset;

	// Check at the end of the file if we got the same 'prefix'
	in.serialBuffer(vMagicBuffer, XDELTA_PREFIX_LEN);
	if (strncmp ((const char *)&vMagicBuffer[0], XDELTA_110_PREFIX, XDELTA_PREFIX_LEN) != 0)
	{
		nlwarning("%s has bad end of file delta is corrupted", sFilename.c_str());
		return false;
	}

	in.close();

	// if the flag patch compressed is on the part from nCtrlOffset to nEndCtrlOffset is the delta compressed
	CXDPFileReader frFile;
	if (!frFile.init(sFilename, nCtrlOffset, nEndCtrlOffset, (_Flags & XDELTA_FLAG_PATCH_COMPRESSED) != 0))
	{
		nlwarning("%s cannot init file reader", sFilename.c_str());
		return false;
	}

	_Ctrl.read(frFile);

	return true;
}

// ---------------------------------------------------------------------------
CXDeltaPatch::TApplyResult CXDeltaPatch::apply(const std::string &sFileToPatch, const std::string &sFileOutput, std::string &errorMsg)
{
	if ((_Flags & XDELTA_FLAG_FROM_COMPRESSED) || (_Flags & XDELTA_FLAG_TO_COMPRESSED))
	{
		errorMsg = "do not handle compressed from_file or to_file";
		return ApplyResult_UnsupportedXDeltaFormat;
	}

	if (_Ctrl.SourceInfo.size() == 0)
	{
		errorMsg = "no source info";
		return ApplyResult_Error;
	}

	if (_Ctrl.SourceInfo.size() > 2)
    {
		errorMsg = "incompatible delta";
		return ApplyResult_Error;
	}

	SXDeltaCtrl::SSourceInfo *pFromSource = NULL;
	SXDeltaCtrl::SSourceInfo *pDataSource = NULL;

	if (_Ctrl.SourceInfo.size() > 0)
    {
		SXDeltaCtrl::SSourceInfo &rInfo = _Ctrl.SourceInfo[0];

		if (rInfo.IsData)
		{
			pDataSource = &rInfo;
		}
		else
		{
			pFromSource = &rInfo;

			if (_Ctrl.SourceInfo.size() > 1)
			{
				errorMsg = "incompatible delta";
				return ApplyResult_Error;
			}
		}
	}

	if (_Ctrl.SourceInfo.size() > 1)
	{
		pFromSource = &_Ctrl.SourceInfo[1];
	}

	// ---

	// Open the file output
	if (NLMISC::CFile::fileExists(sFileOutput))
	{
		errorMsg = toString("output file %s already exists", sFileOutput.c_str());
		return ApplyResult_Error;
	}
	FILE *outFILE = fopen(sFileOutput.c_str(), "wb");
	if (outFILE == NULL)
	{
		errorMsg = toString("cant create %s", sFileOutput.c_str());
		return ApplyResult_Error;
	}

	// Open the file to patch
	FILE *ftpFILE = NULL;
	bool ftpPresent = false;
	if (pFromSource)
	{
		ftpFILE = fopen(sFileToPatch.c_str(), "rb");
		if (ftpFILE == NULL)
		{
			errorMsg = toString("expecting file %s", sFileToPatch.c_str());
			fclose(outFILE);
			return ApplyResult_Error;
		}
		fseek (ftpFILE, 0, SEEK_END);
		uint32 nFileSize = ftell(ftpFILE);
		fseek (ftpFILE, 0, SEEK_SET);

		fclose (ftpFILE);

		if (nFileSize != pFromSource->Len)
		{
			errorMsg = toString("expect from file (%s) of length %d bytes\n", sFileToPatch.c_str(), pFromSource->Len);
			fclose(outFILE);
			return ApplyResult_Error;
		}
		ftpPresent = true;
	}

	CXDPFileReader XDFR[2];

	if (_Ctrl.SourceInfo.size() == 1)
	{
		SXDeltaCtrl::SSourceInfo &rInfo = _Ctrl.SourceInfo[0];

		if (rInfo.IsData)
		{
			// index 0 == Data from patch file
			if (!XDFR[0].init(_FileName, _HeaderOffset, _CtrlOffset, isPatchCompressed()))
			{
				fclose(outFILE);
				errorMsg = toString("cant load file %s", _FileName.c_str());
				return ApplyResult_Error;
			}
		}
		else
		{
			// index 0 == Data from file to patch
			nlassert(ftpPresent); // If not should be returned before
			if (!XDFR[0].init(sFileToPatch, 0, 1024*1024*1024, false))
			{
				fclose(outFILE);
				errorMsg = toString("cant load file %s", sFileToPatch.c_str());
				return ApplyResult_Error;
			}
		}
	}

	if (_Ctrl.SourceInfo.size() == 2)
	{
		// _Ctrl.SourceInfo[0].IsData must be true
		nlassert(_Ctrl.SourceInfo[0].IsData);
		// index 0 == Data from patch file
		if (!XDFR[0].init(_FileName, _HeaderOffset, _CtrlOffset, isPatchCompressed()))
		{
			fclose(outFILE);
			errorMsg = toString("cant load file %s", _FileName.c_str());
			return ApplyResult_Error;
		}
		// index 1 == Data from file to patch
		if (!XDFR[1].init(sFileToPatch, 0, 1024*1024*1024, false))
		{
			fclose(outFILE);
			errorMsg = toString("cant load file %s", sFileToPatch.c_str());
			return ApplyResult_Error;
		}
	}

	// Apply Patch : Copy Delta Region
	uint nSaveWritten = 0;
	uint nLastSaveWritten = 0;
	uint nStep = _Ctrl.ToLen / 100;

	for (uint32 i = 0; i < _Ctrl.Inst.size(); ++i)
	{
		const SXDeltaCtrl::SInstruction *pInst = &_Ctrl.Inst[i];

		if (pInst->Index >= _Ctrl.SourceInfo.size())
		{
			fclose(outFILE);
			errorMsg = toString("Out Of Range Source Index (%d)", pInst->Index);
			return ApplyResult_Error;
		}

		// From
		CXDPFileReader &rFromXDFR = XDFR[pInst->Index];
		rFromXDFR.seek(pInst->Offset);

		uint8 buf[1024];
		uint32 len = pInst->Length;

		while (len > 0)
		{
			uint r = min((uint32)1024, len);

			if (!rFromXDFR.read(buf, r))
			{
				fclose(outFILE);
				errorMsg = ("problem reading source");
				return ApplyResult_Error;
			}

			if (fwrite(buf, 1, r, outFILE) != r)
			{
				errorMsg = ("problem writing dest");
				TApplyResult ar = ApplyResult_Error;
				if (ferror(outFILE))
				{
					errorMsg += std::string(" : ") + strerror(errno);
					if (errno == 28 /*ENOSPC*/)
					{
						ar = ApplyResult_DiskFull;
					}
					else
					{
						ar = ApplyResult_WriteError;
					}
				}
				fclose(outFILE);
				return ar;
			}
			len -= r;

			nSaveWritten += r;

			// Call back
			if ((nSaveWritten-nLastSaveWritten) > nStep)
			{
				nLastSaveWritten = nSaveWritten;
				if (_CallBack != NULL)
					if (_Ctrl.ToLen > 0)
						_CallBack->progress((float)nSaveWritten/(float)_Ctrl.ToLen);
			}

		}
    }

	fclose(outFILE);

	// Check output file
	CXDPFileReader xdfrOut;
	if (!xdfrOut.init(sFileOutput, 0, 1024*1024*1024, false))
	{
		errorMsg = toString("cant open file %s", sFileOutput.c_str());
		return ApplyResult_Error;
	}
	if (!checkIntegrity (xdfrOut, _Ctrl.ToMD5, _Ctrl.ToLen))
	{
		errorMsg = toString("integrity problem with output file %s", sFileOutput.c_str());

		// trap : ok cant do the following for the moment !

		// to better report errors, check if the inputs were invalid now
		/*
		for (i = 0; i < cont->source_info_len; i += 1)
		{
			check_stream_integrity (cont->source_info[i]->in,
			cont->source_info[i]->md5,
			cont->source_info[i]->len);
		}
		*/
		return ApplyResult_Error;
	}
	errorMsg = "";
	return ApplyResult_Ok;
}

// ---------------------------------------------------------------------------
bool CXDeltaPatch::checkIntegrity(CXDPFileReader &rFR, const CHashKeyMD5 &md5, uint32 nLength)
{
	if (nLength != rFR.getFileSize())
    {
		nlwarning("file size different from expected");
		return false;
    }

	CHashKeyMD5 fileMD5;
	CMD5Context ctx;
	ctx.init();

	uint8 buf[1024];
	while (nLength > 0)
	{
		uint r = min((uint32)1024, nLength);

		if (!rFR.read(buf, r))
		{
			nlwarning("problem reading file");
			return false;
		}

		ctx.update(buf, r);

		nLength -= r;
	}
	ctx.final(fileMD5);

	if (md5 != fileMD5)
	{
		nlwarning("integrity test failed");
		return false;
	}
	return true;
}

// Tools

// ---------------------------------------------------------------------------
CXDeltaPatch::TApplyResult CXDeltaPatch::apply(const string &sPatchFilename,
											   const string &sFileToPatchFilename,
											   const string &sOutputFilename,
											   std::string &errorMsg,
											   ICallBack *pCB)
{
	CXDeltaPatch patch;
	if (!patch.load(sPatchFilename))
	{
		errorMsg = toString("cant load patch %s", sPatchFilename.c_str());
		nlwarning(errorMsg.c_str());
		return ApplyResult_Error;
	}
	_CallBack = pCB;
	TApplyResult ar = patch.apply(sFileToPatchFilename, sOutputFilename, errorMsg);
	if (ar != ApplyResult_Ok)
	{
		nlwarning(errorMsg.c_str());
	}
	return ar;
}

// ---------------------------------------------------------------------------
bool CXDeltaPatch::info(const std::string &sPatchFilename)
{
	CXDeltaPatch patch;
	if (!patch.load(sPatchFilename))
	{
		nlwarning ("cant load patch %s",sPatchFilename.c_str());
		return false;
	}
	nlinfo("Patch Name : %s", sPatchFilename.c_str());
	nlinfo("Flag No Verify        : %s", patch.isNoVerify()?"on":"off");
	nlinfo("Flag From Compressed  : %s", patch.isFromCompressed()?"on":"off");
	nlinfo("Flag To Compressed    : %s", patch.isToCompressed()?"on":"off");
	nlinfo("Flag Patch Compressed : %s\n", patch.isPatchCompressed()?"on":"off");

	nlinfo("Output name   : %s", patch.getToName().c_str());
	nlinfo("Output length : %d", patch.getCtrl().ToLen);
	nlinfo("Output md5    : %s\n", patch.getCtrl().ToMD5.toString().c_str());

	nlinfo("Patch from segments: %d\n", patch.getCtrl().SourceInfo.size());
	nlinfo("MD5\t\t\t\t\tLength\tCopies\tUsed\tSeq?\tName");

	for (uint32 i = 0; i < patch.getCtrl().SourceInfo.size(); ++i)
	{
		const SXDeltaCtrl::SSourceInfo &rSI = patch.getCtrl().SourceInfo[i];

		nlinfo("%s\t%d\t%d\t%d\t%s\t%s\n",
			rSI.MD5.toString().c_str(),
			rSI.Len,
			rSI.Copies,
			rSI.CopyLength,
			rSI.Sequential ? "yes" : "no",
			rSI.Name.c_str());
	}
	return true;
}
