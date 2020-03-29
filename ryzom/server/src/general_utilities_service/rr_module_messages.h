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

#ifndef RR_MODULE_MESSAGES_H
#define RR_MODULE_MESSAGES_H

//
////-----------------------------------------------------------------------------
//// includes
////-----------------------------------------------------------------------------
//
//#include "nel/misc/sstring.h"
//#include "nel/misc/md5.h"
//#include "gus_net_types.h"
//#include "repository.h"
//
//
////-----------------------------------------------------------------------------
//// class CMsgRRFileList
////-----------------------------------------------------------------------------
//
//class CMsgRRFileList
//{
//public:
//	struct TFileRecord
//	{
//		std::string FileName;
//		NLMISC::CHashKeyMD5 Checksum;
//
//		void serial(NLMISC::IStream& stream)
//		{
//			stream.serial(FileName);
//			stream.serial(Checksum);
//		}
//	};
//	typedef std::vector<TFileRecord> TFiles;
//
//public:
//	const char* getName() const { return "RR_LIST"; }
//
//	CMsgRRFileList()
//	{
//	}
//
//	CMsgRRFileList(GUSNET::TRawMsgBodyPtr msgBody)
//	{
//		// rewind the internal read pointer to the start of the msgBody
//		// (this is required as a single msgBody) may be dispatched to several different receiver classes
//		msgBody->resetBufPos();
//		// serial out the data...
//		msgBody->serial(*this);
//	}
//
//	CMsgRRFileList(const TFiles& files)
//	{
//		_Files= files;
//	}
//
//		(const CRepository& repository)
//	{
//		// start by clearing out any previous contents in the files vector
//		_Files.clear();
//
//		// iterate over the repository adding files to the files vector
//		CRepository::const_iterator it= repository.begin();
//		CRepository::const_iterator itEnd= repository.end();
//		for (;it!=itEnd;++it)
//		{
//			// append a new entry to the vector
//			vectAppend(_Files);
//			// setup data for the (new) back vector entry
//			_Files.back().FileName= it->first;
//			_Files.back().Checksum= it->second.Checksum;
//			nlinfo("sending info on file: %s",_Files.back().FileName.c_str());
//		}
//	}
//
//	void serial(NLMISC::IStream& stream)
//	{
//		stream.serialCont(_Files);
//	}
//
//	const TFiles& getFileList() const				{ return _Files; }
//	uint32 size() const								{ return _Files.size(); }
//	const TFileRecord& operator[](uint32 idx) const	{ return _Files[idx]; }
//
//private:
//	TFiles _Files;
//};
//
//
////-----------------------------------------------------------------------------
//// class CMsgRRBeginFile
////-----------------------------------------------------------------------------
//
//class CMsgRRBeginFile
//{
//public:
//	const char* getName() const { return "RR_FILE_BEGIN"; }
//
//	CMsgRRBeginFile()
//	{
//	}
//	
//	CMsgRRBeginFile(GUSNET::TRawMsgBodyPtr msgBody)
//	{
//		msgBody->serial(*this);
//	}
//
//	CMsgRRBeginFile(const NLMISC::CSString& fileName,uint32 fileSize)
//	{
//		// setup the other file parameters
//		_FileName= fileName;
//		_FileSize= fileSize;
//	}
//
//	void serial(NLMISC::IStream& stream)
//	{
//		stream.serial(_FileName);
//		stream.serial(_FileSize);
//	}
//
//	const NLMISC::CSString& getFileName() const	{ return _FileName; }
//	uint32 getFileSize() const					{ return _FileSize; }
//
//private:
//	NLMISC::CSString _FileName;		// name of the file
//	uint32 _FileSize;				// total file size for the file
//};
//
//
////-----------------------------------------------------------------------------
//// class CMsgRRFileData
////-----------------------------------------------------------------------------
//
//class CMsgRRFileData
//{
//public:
//	const char* getName() const { return "RR_FILE_DATA"; }
//
//	CMsgRRFileData()
//	{
//	}
//	
//	CMsgRRFileData(GUSNET::TRawMsgBodyPtr msgBody)
//	{
//		msgBody->serial(*this);
//	}
//
//	CMsgRRFileData(const NLMISC::CSString& fileName,const NLMISC::CSString& fileData)
//	{
//		// setup the other file parameters
//		_FileName= fileName;
//		_FileData= fileData;
//	}
//
//	void serial(NLMISC::IStream& stream)
//	{
//		stream.serial(_FileName);
//		stream.serial(_FileData);
//	}
//
//	const NLMISC::CSString& getFileName() const	{ return _FileName; }
//	const NLMISC::CSString& getData() const		{ return _FileData; }
//
//private:
//	NLMISC::CSString _FileName;		// name of the file
//	NLMISC::CSString _FileData;		// the data block
//};
//
//
////-----------------------------------------------------------------------------
//// class CMsgRREndFile
////-----------------------------------------------------------------------------
//
//class CMsgRREndFile
//{
//public:
//	const char* getName() const { return "RR_FILE_END"; }
//
//	CMsgRREndFile()
//	{
//	}
//	
//	CMsgRREndFile(GUSNET::TRawMsgBodyPtr msgBody)
//	{
//		msgBody->serial(*this);
//	}
//
//	CMsgRREndFile(const NLMISC::CSString& fileName)
//	{
//		_FileName= fileName;
//	}
//
//	void serial(NLMISC::IStream& stream)
//	{
//		stream.serial(_FileName);
//	}
//
//	const NLMISC::CSString& getFileName() const		{ return _FileName; }
//
//private:
//	NLMISC::CSString _FileName;		// name of the file
//};
//
//
//-----------------------------------------------------------------------------
#endif
