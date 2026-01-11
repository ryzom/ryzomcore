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


#ifndef BACKUP_FILE_ACCESS_H
#define BACKUP_FILE_ACCESS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/log.h"
#include "nel/misc/variable.h"
#include "nel/net/module.h"
#include "nel/net/buf_net_base.h"
#include "nel/net/callback_net_base.h"
#include "nel/net/buf_sock.h"

#include <string>
#include <vector>
#include <deque>
#include <map>

namespace NLMISC
{
	class CMemStream;
};


class IFileAccess;

extern NLMISC::CVariable<std::string>	BSFilePrefix;

std::string	getBackupFileName(const std::string& filename);


struct TRequester
{
	enum TRequesterType
	{
		rt_service,
		rt_layer3,
		rt_module
	};

	TRequesterType			RequesterType;
	// for service requester
	NLNET::TServiceId		ServiceId;
	// for layer 3 requester
	NLNET::TSockId			From;
	NLNET::CCallbackNetBase	*Netbase;

	// for module requester
	NLNET::TModuleProxyPtr	ModuleProxy;

	TRequester(NLNET::TServiceId serviceId)
		:	RequesterType(rt_service),
			ServiceId(serviceId)
	{}

	TRequester(NLNET::IModuleProxy *proxy)
		:	RequesterType(rt_module),
			ModuleProxy(proxy)
	{}

	TRequester(NLNET::TSockId from, NLNET::CCallbackNetBase *netBase)
		:	RequesterType(rt_layer3),
			From(from),
			Netbase(netBase)
	{}

	std::string toString()
	{
		switch (RequesterType)
		{
		case rt_service:
			return NLMISC::toString("%4d", ServiceId.get());
		case rt_layer3:
			return NLMISC::toString("%s, %p", From->asString().c_str(), Netbase);
		case rt_module:
			return NLMISC::toString("'%s'", ModuleProxy != NULL ? ModuleProxy->getModuleName().c_str() : "! NO MODULE !");
		}
		// This should not append !
		nlstop;
		return std::string();
	}

	bool operator ==(const TRequester &other) const
	{
		if (RequesterType != other.RequesterType)
			return false;

		if (RequesterType == rt_service)
			return ServiceId == other.ServiceId;
		else if (RequesterType == rt_layer3)
			return From == other.From && Netbase == other.Netbase;
		else
			return ModuleProxy == other.ModuleProxy;
	}
};



/**
 * CFileAccessManager
 * Perform all read/write access to files.
 * Provides a higher level of security by stacking file accesses
 *
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CFileAccessManager
{
public:
	typedef std::vector<TRequester>	TRequesters;

	/// Init File manager
	void		init();

	/// Add an access to perform to stack of accesses
	void		stackFileAccess(IFileAccess* access);

	/// Remove a file access
	void		removeFileAccess(IFileAccess* access);

	/// Flushes file accesses
	void		update();

	/// Release File manager
	void		release();


	/// display stacked accesses
	void		displayFileAccesses(NLMISC::CLog& log);

	enum TMode
	{
		Normal,
		Stalled,
	};

	/// Force manager mode
	void		setMode(TMode mode, const std::string& reason);

	/// Get current manager mode
	TMode		getMode()				{ return _Mode; }

	/// Forbid stall mode
	void		forbidStall(bool s = true)		{ _StallAllowed = !s; }

	/// Notify service connection
	void		notifyServiceConnection(NLNET::TServiceId serviceId, const std::string& serviceName);

private:

	std::deque<IFileAccess*>	_Accesses;

	bool						_StallAllowed;
	TMode						_Mode;
	std::string					_Reason;
};



class IFileAccess
{
public:

	IFileAccess(const std::string& filename, const TRequester &requester = NLNET::TServiceId(0xffff), uint32 requestid = 0xffffffff) 
		:	Requester(requester), 
			RequestId(requestid), 
			Filename(filename), 
			FailureMode(NeverFail)	
	{ }

	enum
	{
		NeverFail = 0
	};

	virtual ~IFileAccess()	{ }

	/// \name Standard file access info
	// @{

	TRequester		Requester;
	uint32			RequestId;
	std::string		Filename;
	uint32			FailureMode;

	std::string		FailureReason;

	// @}

	enum TReturnCode
	{
		Success,
		MinorFailure,
		MajorFailure		// Stall recommended
	};

	/**
	 * Execute access
	 * Returns true if executed successfully.
	 * WARNING: access may be called multiple times, in case previous call failed.
	 */
	virtual TReturnCode		execute(CFileAccessManager& manager) = 0;


protected:

	bool			checkFailureMode(uint32 flags)	{ return (flags & FailureMode) != 0; }
};


class CLoadFile : public IFileAccess
{
public:

	CLoadFile(const std::string& filename, const TRequester &requester, uint32 requestid) 
		:	IFileAccess(filename, requester, requestid)	
	{ }

	enum TFailureMode
	{
		MajorFailureIfFileNotExists = 1,
		MajorFailureIfFileUnreaddable = 4,

		MajorFailureMode = MajorFailureIfFileUnreaddable | MajorFailureIfFileNotExists,
	};

	/// Execute file loading
	virtual TReturnCode		execute(CFileAccessManager& manager);
};


class CWriteFile : public IFileAccess
{
public:

	CWriteFile(const std::string& filename, const TRequester &requester, uint32 requestid, NLMISC::CMemStream& data);
	CWriteFile(const std::string& filename, const TRequester &requester, uint32 requestid, uint8* data, uint dataSize);

	enum TFailureMode
	{
		MajorFailureIfFileNotExists = 1,
		MinorFailureIfFileNotExists = 2,
		MajorFailureIfFileExists = 4,
		MinorFailureIfFileExists = 8,

		MajorFailureIfFileUnwritable = 16,

		MajorFailureIfFileUnbackupable = 64,

		MajorFailureMode = MajorFailureIfFileUnwritable,
	};

	std::vector<uint8>		Data;

	bool					Append;
	bool					BackupFile;
	bool					CreateDir;

	/// Execute file writing
	virtual TReturnCode		execute(CFileAccessManager& manager);

};


class CDeleteFile : public IFileAccess
{
public:

	CDeleteFile(const std::string& filename, const TRequester &requester, uint32 requestid) 
		:	IFileAccess(filename, requester, requestid), BackupFile(true)	
	{ }

	enum TFailureMode
	{
		MajorFailureIfFileNotExists = 1,
		MinorFailureIfFileNotExists = 2,

		MajorFailureIfFileUnbackupable = 4,
		MajorFailureIfFileUnDeletable = 8,

		MajorFailureMode = MajorFailureIfFileNotExists,
	};

	/// Execute file writing
	virtual TReturnCode		execute(CFileAccessManager& manager);

	bool					BackupFile;
};


#endif
