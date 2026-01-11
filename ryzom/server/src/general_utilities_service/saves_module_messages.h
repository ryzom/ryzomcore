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

#ifndef SAVES_MODULE_MESSAGES_H
#define SAVES_MODULE_MESSAGES_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// class CMsgSavesRegister
//-----------------------------------------------------------------------------

class CMsgSavesRegister: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "SAVES_REGISTER"; 
	}

	CMsgSavesRegister()
	{
	}
	
	CMsgSavesRegister(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	void serial(NLMISC::IStream& stream)
	{
	}
};
typedef NLMISC::CSmartPtr<CMsgSavesRegister> TMsgSavesRegisterPtr;


//-----------------------------------------------------------------------------
// class CMsgSavesUnregister
//-----------------------------------------------------------------------------

class CMsgSavesUnregister: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "SAVES_UNREGISTER"; 
	}

	CMsgSavesUnregister()
	{
	}
	
	CMsgSavesUnregister(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	void serial(NLMISC::IStream& stream)
	{
	}
};
typedef NLMISC::CSmartPtr<CMsgSavesUnregister> TMsgSavesUnregisterPtr;


//-----------------------------------------------------------------------------
// class CMsgSavesFileRequest
//-----------------------------------------------------------------------------

class CMsgSavesFileRequest: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "SAVES_FILE_REQUEST"; 
	}

	CMsgSavesFileRequest()
	{
	}
	
	CMsgSavesFileRequest(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgSavesFileRequest(uint32 requestId,const NLMISC::CSString& fileName)
	{
		_RequestId=	requestId;
		_FileName=	fileName;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_RequestId);
		stream.serial(_FileName);
	}

	const uint32 getRequestId()	const	
	{
		return _RequestId;
	}

	const NLMISC::CSString& getFileName()	const	
	{
		return _FileName; 
	}

private:
	uint32				_RequestId;
	NLMISC::CSString	_FileName;
};
typedef NLMISC::CSmartPtr<CMsgSavesFileRequest> TMsgSavesFileRequestPtr;


//-----------------------------------------------------------------------------
// class CMsgSavesUpload
//-----------------------------------------------------------------------------

class CMsgSavesUpload: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "SAVES_UPLOAD"; 
	}

	CMsgSavesUpload()
	{
	}
	
	CMsgSavesUpload(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgSavesUpload(uint32 requestId,const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody)
	{
		_RequestId=	requestId;
		_FileName=	fileName;
		_FileBody=	fileBody;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_RequestId);
		stream.serial(_FileName);
		stream.serial(_FileBody);
	}

	const uint32 getRequestId()	const	
	{
		return _RequestId;
	}

	const NLMISC::CSString& getFileName()	const	
	{
		return _FileName; 
	}

	const NLMISC::CSString& getFileBody()	const	
	{
		return _FileBody; 
	}

private:
	uint32				_RequestId;
	NLMISC::CSString	_FileName;
	NLMISC::CSString	_FileBody;
};
typedef NLMISC::CSmartPtr<CMsgSavesUpload> TMsgSavesUploadPtr;


//-----------------------------------------------------------------------------
// class CMsgSavesDelete
//-----------------------------------------------------------------------------

class CMsgSavesDelete: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "SAVES_FILE_DELETE"; 
	}

	CMsgSavesDelete()
	{
	}
	
	CMsgSavesDelete(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgSavesDelete(uint32 requestId,const NLMISC::CSString& fileName)
	{
		_RequestId=	requestId;
		_FileName=	fileName;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_RequestId);
		stream.serial(_FileName);
	}

	const uint32 getRequestId()	const	
	{
		return _RequestId;
	}

	const NLMISC::CSString& getFileName()	const	
	{
		return _FileName; 
	}

private:
	uint32				_RequestId;
	NLMISC::CSString	_FileName;
};
typedef NLMISC::CSmartPtr<CMsgSavesDelete> TMsgSavesDeletePtr;


//-----------------------------------------------------------------------------
// class CMsgSavesMove
//-----------------------------------------------------------------------------

class CMsgSavesMove: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "SAVES_MOVE"; 
	}

	CMsgSavesMove()
	{
	}
	
	CMsgSavesMove(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgSavesMove(uint32 requestId,const NLMISC::CSString& fileName,const NLMISC::CSString& destination)
	{
		_RequestId=		requestId;
		_FileName=		fileName;
		_Destination=	destination;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_RequestId);
		stream.serial(_FileName);
		stream.serial(_Destination);
	}

	const uint32 getRequestId()	const	
	{
		return _RequestId;
	}

	const NLMISC::CSString& getFileName()	const	
	{
		return _FileName; 
	}

	const NLMISC::CSString& getDestination()	const	
	{
		return _Destination; 
	}

private:
	uint32				_RequestId;
	NLMISC::CSString	_FileName;
	NLMISC::CSString	_Destination;
};
typedef NLMISC::CSmartPtr<CMsgSavesMove> TMsgSavesMovePtr;


//-----------------------------------------------------------------------------
#endif
