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

#ifndef CL_LOGIN_XDELTA_H
#define CL_LOGIN_XDELTA_H

#include "nel/misc/types_nl.h"
#include "nel/misc/md5.h"
#include <string>
#include <zlib.h>

#define XDELTA_PREFIX_LEN	8
#define XDELTA_HEADER_WORDS	6
#define XDELTA_HEADER_SPACE	(XDELTA_HEADER_WORDS*4)
#define XDELTA_110_PREFIX	"%XDZ004%"
#define XDELTA_FLAG_NO_VERIFY			1
#define XDELTA_FLAG_FROM_COMPRESSED		2
#define XDELTA_FLAG_TO_COMPRESSED		4
#define XDELTA_FLAG_PATCH_COMPRESSED	8

#define XDELTA_TYPE_CHECKSUM	((1<<(1+8))+3)
#define XDELTA_TYPE_INDEX		((1<<(2+8))+3)
#define XDELTA_TYPE_SOURCEINFO	((1<<(3+8))+3)
#define XDELTA_TYPE_CONTROL		((1<<(7+8))+3)
#define XDELTA_TYPE_INSTRUCTION	((1<<(8+8))+3)

class CXDPFileReader;

/**
 * Class representing the xdelta control
 * it can be 2 sources max : the patch and the file to patch
 * Applying patch is copying from the source 'Index' at offset 'Offset' an amount of data
 * of length 'Length'.
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date July 2004
 */
struct SXDeltaCtrl
{
	struct SSourceInfo
	{
		std::string	Name;
		NLMISC::CHashKeyMD5	MD5;
		uint32		Len;
		bool		IsData;
		bool		Sequential;
		uint32		Position;
		uint32		Copies;
		uint32		CopyLength;

		bool read(CXDPFileReader &fr);
	};

	struct SInstruction
	{
		uint32	Index;
		uint32	Offset;
		uint32	Length;

		bool read(CXDPFileReader &fr);
	};

	NLMISC::CHashKeyMD5			ToMD5;
	uint32						ToLen;
	bool						HasData;
	std::vector<SSourceInfo>	SourceInfo;
	std::vector<SInstruction>	Inst;

	bool read(CXDPFileReader &fr);
};

/**
 * Class representing a delta patch
 * We read only the 1.1 version. 'Compressed From' and 'Compressed To' are not supported.
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date July 2004
 */
class CXDeltaPatch
{
public:

	enum TApplyResult
	{
		ApplyResult_Ok = 0,
		ApplyResult_DiskFull,
		ApplyResult_WriteError,
		ApplyResult_UnsupportedXDeltaFormat,
		ApplyResult_Error, // misc error
	};

	// Callback progress for the file patching
	class ICallBack
	{
	public:
		// f is from 0 to 1 (0% to 100% (complete))
		virtual void progress(float f)=0;
	};

	// Tools
	static TApplyResult apply(	const std::string &sPatchFilename, const std::string &sFileToPatchFilename,
						const std::string &sOutputFilename, std::string &errorMsg, ICallBack *pCallBack = NULL);
	static bool info(const std::string &sPatchFilename);


	// Load a patch file
	bool load(const std::string &sFilename);

	TApplyResult apply(const std::string &sFileToPatch, const std::string &sFileOutput, std::string &errorMsg);

	bool isNoVerify()			{ return (_Flags&XDELTA_FLAG_NO_VERIFY)!=0; }
	bool isFromCompressed()		{ return (_Flags&XDELTA_FLAG_FROM_COMPRESSED)!=0; }
	bool isToCompressed()		{ return (_Flags&XDELTA_FLAG_TO_COMPRESSED)!=0; }
	bool isPatchCompressed()	{ return (_Flags&XDELTA_FLAG_PATCH_COMPRESSED)!=0; }

	const std::string &getToName() const { return _ToName; }
	const std::string &getFromName() const { return _FromName; }
	const SXDeltaCtrl &getCtrl() const { return _Ctrl; }

	bool checkIntegrity (CXDPFileReader &rFR, const NLMISC::CHashKeyMD5 &md5, uint32 nLength);

private:
	std::string		_FileName;

	uint32			_Flags;
	std::string		_Version; // Look XDELTA_FLAG_*
	uint32			_HeaderOffset;
	uint32			_CtrlOffset;

	std::string		_FromName;
	std::string		_ToName;

	SXDeltaCtrl		_Ctrl;

	static ICallBack *_CallBack;
};

/**
 * Class representing a stream
 * Help us to read from zipped or not file seemlessly
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date July 2004
 */
class CXDPFileReader
{
public:
	CXDPFileReader();
	~CXDPFileReader();
	bool	init(const std::string &sFilename, sint32 nLowerBound, sint32 nUpperBound, bool bCompressed);
	bool	read(uint8 *pBuf, sint32 nSize);
	bool	readUInt32(uint32 &val);
	bool	readUInt(uint32 &val);
	bool	readBool(bool &val);
	bool	readString(std::string &s);
	uint32	getFileSize();
	bool	seek(uint32 pos);
private:
	sint32	_LowerBound, _UpperBound;
	bool	_Compressed;
	FILE	*_File;
	gzFile	_GzFile;

	void freeZipMem();
	bool				_Optimize;
	uint32				_OptimPage;
	std::vector<uint8*> _ZipMem;
	sint32				_Pos;
};



#endif // CL_LOGIN_XDELTA_H

