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

#include "nel/misc/types_nl.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/sheet_id.h"

#include "game_share/backup_service_messages.h"
#include "server_share/backup_service_itf.h"
#include "server_share/handy_commands.h"

#include "backup_service.h"
#include "web_connection.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}

//-------------------------------------------------------------------------------------------------
// struct CBackupMsgSaveFile
// For receiving only: first construct a CBackupMsgSaveFile object, then you have access to
// the Filename and the stream
//-------------------------------------------------------------------------------------------------
struct CBackupMsgSaveFileRecv
{
	// Constructor
	CBackupMsgSaveFileRecv( NLMISC::IStream& streamFrom )
	{
		streamFrom.serial(FileName);
	}

	// Filename
	std::string FileName;
};


extern CDirectoryRateStat	DirStats;
extern NLMISC::CVariable<std::string> SaveShardRoot;

using namespace NLNET;
using namespace NLMISC;
using namespace std;

void					cbReadState(IVariable&);

CVariable<std::string>	MasterBSHost("backup", "MasterBSHost", "Master backup Host address", "", 0, true);
CVariable<bool>			BSReadState("backup", "BSReadState", "Current read files state", false, 0, false, cbReadState);
CVariable<uint16>		L3ListeningPort("backup", "L3ListeningPort", "Port used for layer 3 listen socket", 0, 0, true);

bool					MasterBSUp = false;
bool					BSIsSlave = false;
NLMISC::TTime			LastMasterPing = 0;
bool					PongReceived = false;

//-----------------------------------------------------------------------------
static void cbConnection( const string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	if (serviceName == "BS" && BSIsSlave && serviceId != IService::getInstance()->getServiceId())
	{
		nlinfo("SLAVE BS: Master BS is up.");
		MasterBSUp = true;
		PongReceived = true;

		IService::getInstance()->clearCurrentStatus("WaitingMaster");
		IService::getInstance()->removeStatusTag("WaitingMaster");
		IService::getInstance()->addStatusTag("MasterRunning");
		IService::getInstance()->removeStatusTag("MasterDown");
	}

	// notify file manager a service connected
	CBackupService::getInstance()->FileManager.notifyServiceConnection(serviceId, serviceName);

} // cbConnection //


//-----------------------------------------------------------------------------
static void cbDisconnection( const string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	if (serviceName == "BS" && !MasterBSHost.get().empty())
	{
		nlwarning("SLAVE BS: MASTER BS IS DOWN!! File reading allowed!");
		MasterBSUp = false;
		BSReadState = true;

		IService::getInstance()->addStatusTag("MasterDown");
		IService::getInstance()->removeStatusTag("MasterRunning");
	}
} // cbDisconnection //

//-----------------------------------------------------------------------------
void	cbReadState(IVariable& v)
{
	if (!BSIsSlave)
		return;
	CBackupService::getInstance()->FileManager.forbidStall(!BSReadState.get());
}



//-----------------------------------------------------------------------------
static void cbBSPing( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CMessage	msgout("BS_PONG");
	CUnifiedNetwork::getInstance()->send(serviceId, msgout);
}

//-----------------------------------------------------------------------------
static void cbBSPong( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	// don't acknowledge my own pong
	if (serviceId == IService::getInstance()->getServiceId())
		return;

	PongReceived = true;
}

CVariable<string>	StatDirFilter("Stats", "StatDirFilter", "filter of the backup files path to be used", "save_shard", 0, true);


//-----------------------------------------------------------------------------
// cbSaveFile
//
// message format:
// - std::string: fileName
// - remaining of the stream: fileData
//
static void cbSaveFile( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		CBackupMsgSaveFileRecv msg( msgin );

		CWriteFile*	access = new CWriteFile(msg.FileName, serviceId, 0, /*msg.Data*/msgin);

		access->FailureMode = CWriteFile::MajorFailureIfFileUnwritable | CWriteFile::MajorFailureIfFileUnbackupable;
		access->BackupFile = false;
		access->Append = false;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbSaveFile()");
	}

}

//-----------------------------------------------------------------------------
// cbAppendFile
//
// message format:
// - std::string: fileName
// - remaining of the stream: fileData
//
static void cbAppendFile( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		CBackupMsgSaveFileRecv msg( msgin );

		CWriteFile*	access = new CWriteFile(msg.FileName, serviceId, 0, /*msg.Data*/msgin);

		access->FailureMode = CWriteFile::MajorFailureIfFileUnwritable | CWriteFile::MajorFailureIfFileUnbackupable;
		access->BackupFile = false;
		access->Append = true;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbAppendFile()");
	}

}

//-----------------------------------------------------------------------------
// cbAppendFileCheck
//
// message format:
// - std::string: fileName
// - remaining of the stream: fileData
//
static void cbAppendFileCheck( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		CBackupMsgSaveFileRecv msg( msgin );

		CWriteFile*	access = new CWriteFile(msg.FileName, serviceId, 0, msgin);

		access->CreateDir = true;
		access->FailureMode = CWriteFile::MajorFailureIfFileUnwritable | CWriteFile::MajorFailureIfFileUnbackupable;
		access->BackupFile = false;
		access->Append = true;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbAppendFileCheck()");
	}
}



//-----------------------------------------------------------------------------
// cbLoadFile

static void cbLoadFile( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	if (!BSReadState.get())
		return;

	try
	{
		CBackupMsgRequestFile	msg;
		msgin.serial(msg);

		CLoadFile*	access = new CLoadFile(msg.FileName, serviceId, msg.RequestId);

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbLoadFile()");
	}
}

static void cbReadMode( CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// encode the read mode and return
	CMessage msgout("BS_READ_MODE");
	bool readMode = BSReadState.get();
	nlWrite(msgout, serial, readMode);

	// send it back to sender
	netbase.send(msgout, from);
}


static void cbSyncLoadFile( CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	if (!BSReadState.get())
		return;

	try
	{
		CBackupMsgRequestFile	msg;
		msgin.serial(msg);

		CLoadFile*	access = new CLoadFile(msg.FileName, TRequester(from, &netbase), msg.RequestId);

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbLoadFile()");
	}
}


//-----------------------------------------------------------------------------
// cbDeleteFile

static void cbDeleteFile( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		std::string	fileToDelete;
		msgin.serial(fileToDelete);

		CDeleteFile*	access = new CDeleteFile(fileToDelete, serviceId, 0);

		access->BackupFile = true;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbDeleteFile()");
	}
}


//-----------------------------------------------------------------------------
// cbDeleteFileNoBackup

static void cbDeleteFileNoBackup( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		std::string	fileToDelete;
		msgin.serial(fileToDelete);

		CDeleteFile*	access = new CDeleteFile(fileToDelete, serviceId, 0);

		access->BackupFile = false;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbDeleteFile()");
	}

}

//-----------------------------------------------------------------------------
// cbSaveCheckFile

static void	cbSaveCheckFile( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		CBackupMsgSaveFileRecv msg( msgin );

		CWriteFile*	access = new CWriteFile(msg.FileName, serviceId, 0, /*msg.Data*/msgin);

		access->FailureMode = CWriteFile::MajorFailureIfFileUnwritable | CWriteFile::MajorFailureIfFileUnbackupable;
		access->BackupFile = false;
		access->Append = false;
		access->CreateDir = true;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbSaveFile()");
	}

/*
	if( CBackupService::getInstance()->getStall() == true )
	{
		// stall shard
		CBackupService::getInstance()->stallShard( std::string() );
		return;
	}

	CBackupMsgSaveFile msg;

	{
		H_AUTO(SaveCheckNetSerial);
		msgin.serial(msg);
	}
	nlinfo("SAVE: Saving file '%s' size %u", msg.FileName.c_str(), msg.Data.length());


	std::string	path = CFile::getPath(msg.FileName);

	if ((!CFile::isExists(path) || !CFile::isDirectory(path)) && (!CFile::createDirectoryTree(path) || !CFile::setRWAccess(path)))
	{
		nlwarning("Can't check directory '%s' existence (to write file '%s'), shard stalled until problem are resolved !!!", path.c_str(), msg.FileName.c_str());
		// stall shard
		CBackupService::getInstance()->stallShard( msg.FileName );
		return;
	}

	try
	{
		NLMISC::CFile::copyFile( msg.FileName + string(".backup"), msg.FileName );
	}
	catch(const Exception &e)
	{
		nlwarning("Can't write file '%s' size %u : '%s', shard stalled until problem are resolved !!!", ( msg.FileName + string(".backup") ).c_str(), msg.Data.length(), e.what() );
		// stall shard
		CBackupService::getInstance()->stallShard( msg.FileName );
		return;
	}

	COFile f;
	{
		H_AUTO(SaveCheckFileOpen);
		if(!f.open(msg.FileName))
		{
			nlwarning("Can't open file '%s' size %u, shard stalled until problem are resolved !!!", msg.FileName.c_str(), msg.Data.length());
			// stall shard
			CBackupService::getInstance()->stallShard( msg.FileName );
			return;
		}
	}

	try
	{
		H_AUTO(SaveCheckFileSerial);
		f.serialBuffer( (uint8*)msg.Data.buffer(), msg.Data.length() );

		DirStats.writeFile(msg.FileName, msg.Data.length());
	}
	catch(const Exception &e)
	{
		nlwarning("Can't write file '%s' size %u : '%s', shard stalled until problem are resolved !!!", msg.FileName.c_str(), msg.Data.length(), e.what());
		// stall shard
		CBackupService::getInstance()->stallShard( msg.FileName );
	}

	{
		H_AUTO(SaveCheckFileClose);
		f.close();
	}
*/
}

//-----------------------------------------------------------------------------
// cbGetFileClass

struct CClassResult
{
	CClassResult() : Timestamp(0)	{}
	CClassResult(const std::string& file, uint32 stamp) : Timestamp(stamp), File(file)	{}

	uint32		Timestamp;
	std::string	File;

	bool	operator < (const CClassResult& b) const	{ return Timestamp > b.Timestamp; }
};


static CMessage getFileClassImp( CMessage& msgin)
{
	// retrieve the info from the input message
	CBackupMsgFileClass inMsg;
	msgin.serial(inMsg);

	// setup the output message;
	CBackupMsgReceiveFileClass outMsg;
	outMsg.RequestId= inMsg.RequestId;

	bool	hasWildcard = false;

	{
		H_AUTO(GetFileClass_CheckWildcard);
		for (uint j=0; !hasWildcard && j<inMsg.Classes.size(); ++j)
		{
			const CBackupFileClass&	fclass = inMsg.Classes[j];

			uint	k;
			for (k=0; k<fclass.Patterns.size(); ++k)
			{
				if (fclass.Patterns[k].find('*') != std::string::npos)
				{
					hasWildcard = true;
					break;
				}
			}
		}
	}

	std::vector<std::vector<CClassResult> >	classes;

	classes.resize(inMsg.Classes.size());

	if (hasWildcard)
	{
		H_AUTO(GetFileClass_GetContent);

		std::vector<std::string>	files;
		NLMISC::CPath::getPathContent(getBackupFileName(inMsg.Directory), false, false, true, files); // caution: it returns full path names

		for (uint i=0; i<files.size(); ++i)
		{
			uint32		fstamp = CFile::getFileModificationDate(files[i]);
			std::string	fname = NLMISC::CFile::getFilename(files[i]);

			for (uint j=0; j<inMsg.Classes.size(); ++j)
			{
				const CBackupFileClass&	fclass = inMsg.Classes[j];

				uint	k;
				for (k=0; k<fclass.Patterns.size(); ++k)
				{
					if (NLMISC::testWildCard(fname, fclass.Patterns[k]))
					{
						classes[j].push_back(CClassResult(files[i], fstamp));
						break;
					}
				}

				if (k < fclass.Patterns.size())
					break;
			}
		}
	}
	else
	{
		H_AUTO(GetFileClass_GetFiles);

		for (uint j=0; j<inMsg.Classes.size(); ++j)
		{
			const CBackupFileClass&	fclass = inMsg.Classes[j];

			uint	k;
			for (k=0; k<fclass.Patterns.size(); ++k)
			{
				string	file = CPath::standardizePath(inMsg.Directory)+fclass.Patterns[k]; // relative filename
				string	rfile = getBackupFileName(file); // full filename
				if (CFile::isExists(rfile))
					classes[j].push_back(CClassResult(file, CFile::getFileModificationDate(rfile)));
			}
		}
	}

	CFileDescriptionContainer&	fdc = outMsg.Fdc;

	for (uint i=0; i<classes.size(); ++i)
	{
		std::sort(classes[i].begin(), classes[i].end());

		for (uint j=0; j<classes[i].size(); ++j)
		{
			std::string	rfile = classes[i][j].File;
			// if there's wildcard, the file is already full so we don't need to add again the backup path or we ll have "save_shard/save_shard/..." that is not valid
			if (!CFile::isExists(rfile))
				rfile = getBackupFileName(classes[i][j].File);
			fdc.addFile(classes[i][j].File, CFile::getFileModificationDate(rfile),CFile::getFileSize(rfile));
		}
	}
	// In case something like getPathContent() has returned full paths, make paths relative to match the requested filenames
	fdc.stripFilename(SaveShardRoot.get());

	// compose the output message
	CMessage	msgout("BS_FILE_CLASS");
	msgout.serial(outMsg);

	return msgout;
}


static void cbGetFileClass( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	if (!BSReadState.get())
		return;

	CMessage msgOut = getFileClassImp(msgin);

	// send the output message
	CUnifiedNetwork::getInstance()->send(serviceId, msgOut);
}

static void cbSyncGetFileClass( CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	if (!BSReadState.get())
		return;

	CMessage msgOut = getFileClassImp(msgin);

	// send the output message
	netbase.send(msgOut, from);
}



//-----------------------------------------------------------------------------
// cbAppend

static void cbAppend( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	try
	{
		CBackupMsgAppend	inMsg;
		msgin.serial(inMsg);

		std::string	append = inMsg.Append+'\n';
		uint8*		data = (uint8*)(const_cast<char*>(append.c_str()));
		uint		dataSize = (uint)append.size();

		CWriteFile*	access = new CWriteFile(inMsg.FileName, serviceId, 0, data, dataSize);

		access->FailureMode = CWriteFile::MajorFailureIfFileUnwritable;
		access->BackupFile = false;
		access->Append = true;

		CBackupService::getInstance()->FileManager.stackFileAccess(access);
	}
	catch (...)
	{
		nlwarning("WARNING: caught exception in cbAppendFile()");
	}
}


//-----------------------------------------------------------------------------
bool CBackupService::update()
{
	TTime	ptime = CTime::getLocalTime();
	if (BSIsSlave && MasterBSUp && ptime - LastMasterPing > 60*1000)
	{
		CMessage	msgout("BS_PING");
		CUnifiedNetwork::getInstance()->send("BS", msgout);
		PongReceived = false;
		LastMasterPing = ptime;
	}

	FileManager.update();
	updateWebConnection();

	_CallbackServer->update();

	if (BSReadState == true)
	{
		IService::getInstance()->addStatusTag("ReadWrite");
		IService::getInstance()->removeStatusTag("WriteOnly");
	}
	else
	{
		IService::getInstance()->removeStatusTag("ReadWrite");
		IService::getInstance()->addStatusTag("WriteOnly");
	}
	
	return true;
}


//-----------------------------------------------------------------------------
void CBackupService::release()
{
	FileManager.release();
	releaseWebConnection();
	delete _CallbackServer;
}


//-----------------------------------------------------------------------------
void CBackupService::stallShard( const std::string& fileName )
{
/*	CMessage msgOut("STALL_SHARD");
	std::string s = fileName;
	msgOut.serial( s );
	CUnifiedNetwork::getInstance()->send("EGS", msgOut );
	_SaveStall = true;
*/
	fileName.empty() ? nlwarning("BackupService are in stall mode !") : nlwarning("Backup service enter in stall mode when trying save file %s", fileName.c_str() );
}


void	CBackupService::onModuleDown(NLNET::IModuleProxy *proxy)
{
}


//-----------------------------------------------------------------------------
TUnifiedCallbackItem CbArray[]=
{
	{ "save_file",			cbSaveFile },
	{ "load_file",			cbLoadFile },
	{ "append_file",		cbAppendFile },
	{ "append_file_check",	cbAppendFileCheck },

	{ "SAVE_CHECK_FILE",	cbSaveCheckFile },
	{ "DELETE_FILE",		cbDeleteFile },
	{ "DELETE_FILE_NO_BACKUP",		cbDeleteFileNoBackup },

	{ "GET_FILE_CLASS",		cbGetFileClass },

	{ "APPEND",				cbAppend },

	{ "BS_PING",			cbBSPing },
	{ "BS_PONG",			cbBSPong },
};


TCallbackItem cbSyncArray[] =
{
	{ "GET_READ_MODE",		cbReadMode	},		// TODO : implement me !
	{ "load_file",			cbSyncLoadFile	},	
	{ "GET_FILE_CLASS",		cbSyncGetFileClass	},	
};

//-----------------------------------------------------------------------------
void CBackupService::init()
{
	FileManager.init();

	setUpdateTimeout(100);
	_SaveStall = false;

	// set the connection and disconnection callbacks
	CUnifiedNetwork::getInstance()->setServiceUpCallback( string("*"), cbConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbDisconnection, 0);

	CUnifiedNetwork::getInstance()->setServiceUpCallback( string("BS"), cbConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("BS"), cbDisconnection, 0);

	// Init the sheet Id 
	CSheetId::init(false);

	if (!MasterBSHost.get().empty())
	{
		IService::getInstance()->addStatusTag("SlaveMode");
		IService::getInstance()->setCurrentStatus("WaitingMaster");

		BSIsSlave = true;
		FileManager.forbidStall();
		// I'm a slave, try to contact master
		string	host = MasterBSHost;
		if (host.find (":") == string::npos)
			host += ":49990";

		CUnifiedNetwork::getInstance()->addService ("BS", CInetAddress(host));
	}

	// set the initial read state from the config file
	CConfigFile::CVar *readState = ConfigFile.getVarPtr("BSReadState");
	if (readState != NULL)
		BSReadState = readState->asBool();


	initWebConnection();

	_CallbackServer = new NLNET::CCallbackServer;
	_CallbackServer->addCallbackArray(cbSyncArray, sizeofarray(cbSyncArray));
	// open the layer 3 callback server if required
	if (L3ListeningPort != 0)
		_CallbackServer->init(L3ListeningPort);
}

static const char* getCompleteServiceName(const IService* theService, const char *defaultName)
{
	static std::string s;
	s= defaultName;

	if (theService->haveLongArg("name"))
	{
		s+= "_"+theService->getLongArg("name");
	}

	if (theService->haveLongArg("fullname"))
	{
		s= theService->getLongArg("fullname");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService, const char *defaultName)
{
	static std::string s;
	s= defaultName;

	if (theService->haveLongArg("shortname"))
	{
		s= theService->getLongArg("shortname");
	}

	return s.c_str();
}

NLNET_SERVICE_MAIN( CBackupService, getShortServiceName(scn, "BS"), getCompleteServiceName(scn, "backup_service"), 49990, CbArray, "", "" )


void	CDirectoryRateStat::clear()
{
	TDirectoryMap::iterator	first = _DirectoryMap.begin(), last = _DirectoryMap.end();
	for (; first != last; ++first)
		(*first).second.clear();
}

void	CDirectoryRateStat::readFile(const std::string& filename, uint32 filesize)
{
	NLMISC::TTime	now = NLMISC::CTime::getLocalTime();
	if (filename.find("www") != std::string::npos)
	{
		_DirectoryMap["www"].read(now, filesize);
	}
	else if (filename.find(StatDirFilter.get()) != std::string::npos)
	{
		_DirectoryMap[NLMISC::CFile::getPath(filename)].read(now, filesize);
	}
}

void	CDirectoryRateStat::writeFile(const std::string& filename, uint32 filesize)
{
	NLMISC::TTime	now = NLMISC::CTime::getLocalTime();
	if (filename.find("www") != std::string::npos)
	{
		_DirectoryMap["www"].write(now, filesize);
	}
	else if (filename.find(StatDirFilter.get()) != std::string::npos)
	{
		_DirectoryMap[NLMISC::CFile::getPath(filename)].write(now, filesize);
	}
}


uint	CDirectoryRateStat::getMeanReadRate()
{
	NLMISC::TTime	limit = NLMISC::CTime::getLocalTime()-60*1000;
	uint64	read = 0;
	TDirectoryMap::iterator	first = _DirectoryMap.begin(), last = _DirectoryMap.end();
	for (; first != last; ++first)
	{
		(*first).second.updateTime(limit);
		read += (*first).second.ReadBytes;
	}

	return (uint)(read / 60);
}

uint	CDirectoryRateStat::getMeanWriteRate()
{
	NLMISC::TTime	limit = NLMISC::CTime::getLocalTime()-60*1000;
	uint64	write = 0;
	TDirectoryMap::iterator	first = _DirectoryMap.begin(), last = _DirectoryMap.end();
	for (; first != last; ++first)
	{
		(*first).second.updateTime(limit);
		write += (*first).second.WrittenBytes;
	}

	return (uint)(write / 60);
}

void	CDirectoryRateStat::display(NLMISC::CLog& log)
{
	uint	pathsize = 0;
	TDirectoryMap::iterator	first = _DirectoryMap.begin(), last = _DirectoryMap.end();
	for (; first != last; ++first)
		if ((*first).first.size() > pathsize)
			pathsize = (uint)(*first).first.size();

	NLMISC::TTime	limit = NLMISC::CTime::getLocalTime()-60*1000;

	std::string	format = "%-"+NLMISC::toString(pathsize)+"s %6s %10s %6s %10s";
	log.displayNL(format.c_str(), "directory", "rdfile", "read", "wrfile", "write");
	for (first=_DirectoryMap.begin(); first != last; ++first)
	{
		(*first).second.updateTime(limit);
		uint64	rdrate = (*first).second.ReadBytes/60;
		uint64	wrrate = (*first).second.WrittenBytes/60;
		log.displayNL(format.c_str(), 
			(*first).first.c_str(), 
			NLMISC::toString((*first).second.ReadFiles).c_str(), 
			(NLMISC::bytesToHumanReadable(uint32(rdrate))+"/s").c_str(), 
			NLMISC::toString((*first).second.WrittenFiles).c_str(), 
			(NLMISC::bytesToHumanReadable(uint32(wrrate))+"/s").c_str());
	}
}



