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

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdpch.h"
#include "_backup_service_interface_singleton.h"
#include "utils.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLNET;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// const FileNameValidator
//-------------------------------------------------------------------------------------------------

// little class designed to test validity of file names in a reasonably rapid manor
class CFileNameValidator
{
public:
	// setup the set of valid characters for the file name
	CFileNameValidator()
	{
		memset(&_Data,0,sizeof(_Data));
		for (uint32 i='A'; i<='Z';++i) _Data[i]= true;
		for (uint32 i='a'; i<='z';++i) _Data[i]= true;
		for (uint32 i='0'; i<='9';++i) _Data[i]= true;
		_Data[(uint32)'/']= true;
		_Data[(uint32)'.']= true;
		_Data[(uint32)'_']= true;
		_Data[(uint32)' ']= true;
	}

	// lookup a character to determine whether it's valid or not
	bool operator[](char c) const
	{
		return _Data[(uint8)c];
	}

	// check all of the characters in a file name to ensure that it is valid
	// return true if the file name is ok, otherwise false
	bool checkFileName(const std::string& fileName) const
	{
		for (uint32 i=(uint32)fileName.size();i--;)
		{
			if (operator[](fileName[i])==true)
				continue;

			nlwarning("FileNameValidator: refusing character '%c' (%u) in file name: %s",fileName[i],fileName[i],fileName.c_str());
			return false;
		}
		return true;
	}

private:
	// private data
	bool _Data[256];
};

// a constant instance of the validator class
static const CFileNameValidator FileNameValidator;


//-------------------------------------------------------------------------------------------------
// methods & globals CBackupMsgSaveFile
//-------------------------------------------------------------------------------------------------

CBackupMsgSaveFile::CBackupMsgSaveFile( const std::string& filename, TBackupMsgSaveFileType msgType, const CBackupServiceInterface& itf )
{
	//DataMsg.setType( getConversionTable().toString( msgType ) );
	DataMsg.setType( _TypesStr[msgType] );
	string pathFilename = itf.getRemotePath() + filename;
	DataMsg.serial( pathFilename );

	FileName = filename; // the filename with no heading remote path
	_MsgType = msgType;
}

const char *CBackupMsgSaveFile::_TypesStr [CBackupMsgSaveFile::NbTypes] =
{
	"save_file",
	"SAVE_CHECK_FILE",
	"append_file",
	"append_file_check",
};


//-------------------------------------------------------------------------------------------------
// methods CBackupServiceInterface
//-------------------------------------------------------------------------------------------------

void CBackupServiceInterface::init(const std::string& bsiname)
{
	_Name= bsiname;
}

void CBackupServiceInterface::setRemotePath( const std::string& remotePath )
{
	_RemotePath = CPath::standardizePath( remotePath );
}

void CBackupServiceInterface::setLocalPath( const std::string& localPath )
{
	_LocalPath = CPath::standardizePath( localPath );
}

void CBackupServiceInterface::requestFile(const std::string& fileName, NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb)
{
	H_AUTO(BSIF_RequestFile);

	// check that the file name is valid
	BOMB_IF(!FileNameValidator.checkFileName(fileName),"Failed to send get file request "+CSString(fileName).quote()+" due to invalid characters in file name",return);

	// if there's no BS connected then complain and queue the file for loading later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to request file "+CSString(fileName).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}

	// create a log message...
//	nldebug("BSIF: requestFile(): FileName: %s",fileName.quote().c_str(),msg.DataMsg.length());

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushFileCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchRequestFile(_Name, requestId, _RemotePath+fileName);
}

// load a file synchronously
void CBackupServiceInterface::syncLoadFile(const std::string& fileName, NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb)
{
	H_AUTO(BSIF_RequestFile);

	// check that the file name is valid
	BOMB_IF(!FileNameValidator.checkFileName(fileName),"Failed to send sync load file "+CSString(fileName).quote()+" due to invalid characters in file name",return);

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushFileCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchSyncLoadFile(_Name, requestId, _RemotePath+fileName, false);
}

// load a set of file synchronously
void CBackupServiceInterface::syncLoadFiles(const std::vector<std::string>& fileNames, NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb)
{
	H_AUTO(BSIF_RequestFile);

	// for each file, send a request request to the BS
	for (uint i=0; i<fileNames.size(); ++i)
	{
		const string &fileName = fileNames[i];
		// check that the file name is valid
		BOMB_IF(!FileNameValidator.checkFileName(fileName),"Failed to send sync load file "+CSString(fileName).quote()+" due to invalid characters in file name",continue);

		// store away the callback for this action and retrieve the associated request id
		uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushFileCallback(cb, this);

		// dispatch the request (not blocking)
		CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchSyncLoadFile(_Name, requestId, _RemotePath+fileName, true);
	}

	// block until all files are loaded
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->terminateSyncLoads();
}


void CBackupServiceInterface::sendFile(CBackupMsgSaveFile& msg, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb)
{
	H_AUTO(BSIF_SendFile);

	// check that the file name is valid
	BOMB_IF(!FileNameValidator.checkFileName(msg.FileName),"Failed to send save file request "+CSString(msg.FileName).quote()+" due to invalid characters in file name",return);

	// if there's no BS connected then complain and queue the file for saving later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to send file "+CSString(msg.FileName).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}

	// create a log message...
//	nldebug("BSIF: sendFile(): FileName: \"%s\", Size: %d bytes",msg.FileName.c_str(),msg.DataMsg.length());

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushGenericAckCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchSendFile(_Name,requestId,msg);
}

// Append a data stream to a file
void	CBackupServiceInterface::append(CBackupMsgSaveFile& msg, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb)
{
	H_AUTO(BSIF_AppendFile);

	// check that the file name is valid
	BOMB_IF(!FileNameValidator.checkFileName(msg.FileName),"Failed to send append request "+CSString(msg.FileName).quote()+" due to invalid characters in file name",return);

	// if there's no BS connected then complain and queue the file for saving later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to append to file "+CSString(msg.FileName).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}

	// create a log message...
//	nldebug("BSIF: append(): FileName: \"%s\", Size: %d bytes",msg.FileName.c_str(),msg.DataMsg.length());

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushGenericAckCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchAppendData(_Name,requestId,msg);
}

// Append a line to a file
void	CBackupServiceInterface::append(const std::string& filename, const std::string& line, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb)
{
	H_AUTO(BSIF_AppendText);

	// check that the file name is valid
	BOMB_IF(!FileNameValidator.checkFileName(filename),"Failed to send append file request "+CSString(filename).quote()+" due to invalid characters in file name",return);

	// if there's no BS connected then complain and queue the file for saving later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to append to file "+CSString(filename).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}


	// create a log message...
//	nldebug("BSIF: append(): FileName: %s, %s",filename.quote().c_str(),line.quote().c_str());

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushGenericAckCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchAppendText(_Name,requestId,_RemotePath+filename,line);
}

// request for a file to be deleted (WARNING: no archiving of the file)
void	CBackupServiceInterface::deleteFile(const std::string& fileName, bool keepBackupOfFile, NLMISC::CSmartPtr<IBackupGenericAckCallback> cb)
{
	H_AUTO(BSIF_DeleteFile);

	// check that the file name is valid
	BOMB_IF(!FileNameValidator.checkFileName(fileName),"Failed to send delete request "+CSString(fileName).quote()+" due to invalid characters in file name",return);

	// if there's no BS connected then complain and queue the file for deleting later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to delete file "+CSString(fileName).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}

	// create a log message...
//	nldebug("BSIF: deleteFile(): FileName: \"%s\", KeepBackup: %s",fileName.c_str(), keepBackupOfFile ? "true" : "false");

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushGenericAckCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchDeleteFile(_Name,requestId,_RemotePath+fileName,keepBackupOfFile);
}


// request BS sends me files matching the given file classes
void	CBackupServiceInterface::requestFileClass(const std::string& directory, const std::vector<CBackupFileClass>& classes, NLMISC::CSmartPtr<IBackupFileClassReceiveCallback> cb)
{
	H_AUTO(BSIF_RequestFileClass);

	// if there's no BS connected then complain and queue the file for retrieving later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to request file class in directory "+CSString(directory).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}

	// create a log message...
//	NLMISC::CSString patterns;
//	for (uint32 j=0;j<classes.size();++j)
//	{
//		for (uint32 i=0;i<classes[j].Patterns.size();++i)
//			patterns+= classes[j].Patterns[i]+';';
//		patterns= patterns.rightCrop(1)+'|';
//	}
//	nldebug("BSIF: requestFileClass(): Directory: \"%s\", Pattern: \"%s\", RequestId: %d",directory.c_str(),patterns.rightCrop(1).c_str(),msg.RequestId);

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushFileClassCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchRequestFileClass(_Name,requestId,_RemotePath+directory,classes);
}

// request BS sends me files matching the given file classes
void	CBackupServiceInterface::syncLoadFileClass(const std::string& directory, const std::vector<CBackupFileClass>& classes, NLMISC::CSmartPtr<IBackupFileClassReceiveCallback> cb)
{
	H_AUTO(BSIF_syncLoadFileClass);

	// if there's no BS connected then complain and queue the file for retrieving later
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		BOMB( ("Failed to request file class in directory "+CSString(directory).quote()+" because the backup services are all down!").c_str() , nlSleep(5000); exit(0) );
	}

	// create a log message...
//	NLMISC::CSString patterns;
//	for (uint32 j=0;j<classes.size();++j)
//	{
//		for (uint32 i=0;i<classes[j].Patterns.size();++i)
//			patterns+= classes[j].Patterns[i]+';';
//		patterns= patterns.rightCrop(1)+'|';
//	}
//	nldebug("BSIF: requestFileClass(): Directory: \"%s\", Pattern: \"%s\", RequestId: %d",directory.c_str(),patterns.rightCrop(1).c_str(),msg.RequestId);

	// store away the callback for this action and retrieve the associated request id
	uint32 requestId= CBackupInterfaceSingleton::getInstance()->pushFileClassCallback(cb, this);

	// dispatch the request
	CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->dispatchSyncLoadFileClass(_Name,requestId,_RemotePath+directory,classes);
}


NLMISC::TTime CBackupServiceInterface::getLastAckTime() const
{
	// if there's no BS connected then give up
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		return 0;
	}

	// delegate...
	return CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->getLastAckTime();
}

NLMISC::TTime CBackupServiceInterface::getLastAckDelay() const
{
	// if there's no BS connected then give up
	if (!CBackupInterfaceSingleton::getInstance()->isConnected())
	{
		// return 64 bit signed maxint (a very big number)
		return uint64(sint64(-1))>>1;
	}

	// delegate...
	return CBackupInterfaceSingleton::getInstance()->getBSIImplementation()->getLastAckDelay();
}

