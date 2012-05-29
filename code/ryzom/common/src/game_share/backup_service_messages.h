// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef BACKUP_SERVICE_MESSAGES_H
#define	BACKUP_SERVICE_MESSAGES_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/net/unified_network.h"
#include "nel/misc/hierarchical_timer.h"
#include "file_description_container.h"
#include "backup_service_interface.h"


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgRequestFile
//-------------------------------------------------------------------------------------------------

struct CBackupMsgRequestFile
{
	uint32 RequestId;
	std::string FileName;

	CBackupMsgRequestFile()
	{
		RequestId=0;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(RequestId);
		stream.serial(FileName);
	}
};


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgReceiveFile
//-------------------------------------------------------------------------------------------------

struct CBackupMsgReceiveFile
{
	uint32 RequestId;
	CFileDescription FileDescription;
	NLMISC::CMemStream Data;

	CBackupMsgReceiveFile()
	{
		RequestId=0;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(RequestId);
		stream.serial(FileDescription);
		if (Data.isReading()!=stream.isReading())
			Data.invert();
		stream.serialMemStream(Data);
	}
};


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgReceiveFileList
//-------------------------------------------------------------------------------------------------

struct CBackupMsgReceiveFileList
{
	uint32 RequestId;
	CFileDescriptionContainer Fdc;

	CBackupMsgReceiveFileList()
	{
		RequestId=0;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(RequestId);
		stream.serial(Fdc);
	}

	void send(const char* serviceName)
	{
		H_AUTO(CBackupMsgReceiveFileListSend0);
		NLNET::CMessage msgOut("bs_file_list");
		serial(msgOut);
		NLNET::CUnifiedNetwork::getInstance()->send( serviceName, msgOut );
	}

	void send(NLNET::TServiceId serviceId)
	{
		H_AUTO(CBackupMsgReceiveFileListSend1);
		NLNET::CMessage msgOut("bs_file_list");
		serial(msgOut);
		NLNET::CUnifiedNetwork::getInstance()->send( serviceId, msgOut );
	}
};


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgFileClass
//-------------------------------------------------------------------------------------------------

struct CBackupMsgFileClass
{
	uint32							RequestId;
	std::string						Directory;
	std::vector<CBackupFileClass>	Classes;

	CBackupMsgFileClass()
	{
		RequestId=0;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(RequestId);
		stream.serial(Directory);
		stream.serialCont(Classes);
	}
};


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgReceiveFileClass
//-------------------------------------------------------------------------------------------------

struct CBackupMsgReceiveFileClass
{
	uint32						RequestId;
	CFileDescriptionContainer	Fdc;

	CBackupMsgReceiveFileClass()
	{
		RequestId=0;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(RequestId);
		stream.serial(Fdc);
	}
};


//-------------------------------------------------------------------------------------------------
// struct CBackupMsgAppend
//-------------------------------------------------------------------------------------------------

struct CBackupMsgAppend
{
	std::string					FileName;
	std::string					Append;

	CBackupMsgAppend()
	{
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(FileName);
		stream.serial(Append);
	}
};

//-------------------------------------------------------------------------------------------------
// struct CBackupMsgAppendCallback
//-------------------------------------------------------------------------------------------------

struct CBackupMsgAppendCallback
{
	std::string					FileName;
	std::vector<std::string>	Appends;

	CBackupMsgAppendCallback()
	{
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(FileName);
		stream.serialCont(Appends);
	}
};

typedef void	(*TBackupAppendCallback)(CBackupMsgAppendCallback& append);


//-------------------------------------------------------------------------------------------------
#endif
