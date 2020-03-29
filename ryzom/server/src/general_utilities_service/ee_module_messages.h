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

#ifndef EE_MODULE_MESSAGES_H
#define EE_MODULE_MESSAGES_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/md5.h"
#include "game_share/file_description_container.h"
#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// class CMsgEELogin
//-----------------------------------------------------------------------------

class CMsgEELogin: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_LOGIN"; 
	}

	CMsgEELogin()
	{
	}

	
	CMsgEELogin(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEELogin(const NLMISC::CSString& user,const NLMISC::CSString& password)
	{
		_User=		user;
		_HashKey=	generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString	_User;
	NLMISC::CHashKeyMD5	_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ password;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEELogin> TMsgEELoginPtr;


//-----------------------------------------------------------------------------
// class CMsgEEUpload
//-----------------------------------------------------------------------------

class CMsgEEUpload: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_UPLOAD"; 
	}

	CMsgEEUpload()
	{
	}
	
	CMsgEEUpload(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEEUpload(const NLMISC::CSString& user,const NLMISC::CSString& password,const NLMISC::CSString& eventName,const CFileDescriptionContainer& fdc,const NLMISC::CVectorSString& fileBodies)
	{
		_User=			user;
		_EventName=		eventName;
		_Fdc=			fdc;
		_FileBodies=	fileBodies;
		_HashKey=		generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		stream.serial(_EventName);
		stream.serial(_Fdc);
		stream.serialCont(_FileBodies);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	const NLMISC::CSString& getEventName()	const	
	{
		return _EventName; 
	}

	const CFileDescriptionContainer& getFdc()	const	
	{
		return _Fdc; 
	}

	const NLMISC::CVectorSString& getFileBodies()	const	
	{
		return _FileBodies; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString			_User;
	NLMISC::CSString			_EventName;
	CFileDescriptionContainer	_Fdc;
	NLMISC::CVectorSString		_FileBodies;
	NLMISC::CHashKeyMD5			_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ _EventName;
		for (uint32 i=0;i<_Fdc.size();++i)
			s+= NLMISC::toString("%s%d%d",_Fdc[i].FileName.c_str(),_Fdc[i].FileTimeStamp,_Fdc[i].FileSize);
		s+= password;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEEUpload> TMsgEEUploadPtr;


////-----------------------------------------------------------------------------
//// class CMsgEERestartShard
////-----------------------------------------------------------------------------
//
//class CMsgEERestartShard: public NLMISC::CRefCount
//{
//public:
//	static const char* getName() 
//	{
//		return "EE_RESTART_SHARD"; 
//	}
//
//	CMsgEERestartShard()
//	{
//	}
//
//	
//	CMsgEERestartShard(GUSNET::TRawMsgBodyPtr msgBody)
//	{
//		msgBody->serial(*this);
//	}
//
//	CMsgEERestartShard(const NLMISC::CSString& user,const NLMISC::CSString& password)
//	{
//		_User=		user;
//		_HashKey=	generateHashKey(password);
//	}
//
//	void serial(NLMISC::IStream& stream)
//	{
//		stream.serial(_User);
//		std::string hashKeyString= _HashKey.toString();
//		stream.serial(hashKeyString);
//		_HashKey.fromString(hashKeyString);
//	}
//
//	const NLMISC::CSString& getUser()	const	
//	{
//		return _User; 
//	}
//
//	bool testPassword(const NLMISC::CSString& password) const
//	{
//		return ! (_HashKey != generateHashKey(password));
//	}
//
//private:
//	NLMISC::CSString	_User;
//	NLMISC::CHashKeyMD5	_HashKey;
//
//	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
//	{
//		NLMISC::CSString s= getName()+ _User+ password;
//		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
//	}
//};
//typedef NLMISC::CSmartPtr<CMsgEERestartShard> TMsgEERestartShardPtr;


//-----------------------------------------------------------------------------
// class CMsgEEPeek
//-----------------------------------------------------------------------------

class CMsgEEPeek: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_PEEK"; 
	}

	CMsgEEPeek()
	{
	}

	
	CMsgEEPeek(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEEPeek(const NLMISC::CSString& user,const NLMISC::CSString& password)
	{
		_User=		user;
		_HashKey=	generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString	_User;
	NLMISC::CHashKeyMD5	_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ password;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEEPeek> TMsgEEPeekPtr;


//-----------------------------------------------------------------------------
// class CMsgEEEventStart
//-----------------------------------------------------------------------------

class CMsgEEEventStart: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_EVENT_START"; 
	}

	CMsgEEEventStart()
	{
	}

	
	CMsgEEEventStart(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEEEventStart(const NLMISC::CSString& user,const NLMISC::CSString& password)
	{
		_User=		user;
		_HashKey=	generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString	_User;
	NLMISC::CHashKeyMD5	_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ password;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEEEventStart> TMsgEEEventStartPtr;


//-----------------------------------------------------------------------------
// class CMsgEEEventStop
//-----------------------------------------------------------------------------

class CMsgEEEventStop: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_EVENT_STOP"; 
	}

	CMsgEEEventStop()
	{
	}

	
	CMsgEEEventStop(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEEEventStop(const NLMISC::CSString& user,const NLMISC::CSString& password)
	{
		_User=		user;
		_HashKey=	generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString	_User;
	NLMISC::CHashKeyMD5	_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ password;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEEEventStop> TMsgEEEventStopPtr;


//-----------------------------------------------------------------------------
// class CMsgEEToolsUpdReq
//-----------------------------------------------------------------------------
// This class represents a 'tools update request'

class CMsgEEToolsUpdReq: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_TOOLS_UPDATE_REQUEST"; 
	}

	CMsgEEToolsUpdReq()
	{
	}
	
	CMsgEEToolsUpdReq(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEEToolsUpdReq(const NLMISC::CSString& user,const NLMISC::CSString& password)
	{
		_User=		user;
		_HashKey=	generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString	_User;
	NLMISC::CHashKeyMD5	_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ password;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEEToolsUpdReq> TMsgEEToolsUpdReqPtr;


//-----------------------------------------------------------------------------
// class CMsgEEToolsFileReq
//-----------------------------------------------------------------------------
// This class represents a 'tools update file request'

class CMsgEEToolsFileReq: public NLMISC::CRefCount
{
public:
	static const char* getName() 
	{
		return "EE_TOOLS_FILE_REQUEST"; 
	}

	CMsgEEToolsFileReq()
	{
	}
	
	CMsgEEToolsFileReq(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgEEToolsFileReq(const NLMISC::CSString& user,const NLMISC::CSString& password,const NLMISC::CSString& fileName)
	{
		_FileName=	fileName;
		_User=		user;
		_HashKey=	generateHashKey(password);
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_User);
		std::string hashKeyString= _HashKey.toString();
		stream.serial(hashKeyString);
		_HashKey.fromString(hashKeyString);
		stream.serial(_FileName);
	}

	const NLMISC::CSString& getFileName()	const	
	{
		return _FileName; 
	}

	const NLMISC::CSString& getUser()	const	
	{
		return _User; 
	}

	bool testPassword(const NLMISC::CSString& password) const
	{
		return ! (_HashKey != generateHashKey(password));
	}

private:
	NLMISC::CSString	_FileName;
	NLMISC::CSString	_User;
	NLMISC::CHashKeyMD5	_HashKey;

	NLMISC::CHashKeyMD5 generateHashKey(const NLMISC::CSString& password) const
	{
		NLMISC::CSString s= getName()+ _User+ password+ _FileName;
		return NLMISC::getMD5((const uint8 *)(s.c_str()),s.size());
	}
};
typedef NLMISC::CSmartPtr<CMsgEEToolsFileReq> TMsgEEToolsFileReqPtr;


//-----------------------------------------------------------------------------
#endif
