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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/md5.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

// game share
#include "game_share/utils.h"
#include "game_share/file_description_container.h"

// local
#include "gus_module_manager.h"
#include "gus_net.h"
#include "gus_net_messages.h"
#include "gus_net_remote_module.h"

#include "server_control_modules.h"
#include "repository.h"
#include "rr_module_itf.h"
#include "re_module_itf.h"



//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;
using namespace GUS_SCM;



//-----------------------------------------------------------------------------
// class CRepositoryReceiver
//-----------------------------------------------------------------------------

class CRepositoryReceiver: 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CRepositoryReceiverSkel
{
public:
	// IModule specialisation implementation
	bool initModule(const TParsedCommandLine &initInfo);
	void onModuleUp(IModuleProxy *module);
	void onModuleDown(IModuleProxy *module);
//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg);
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	std::string buildModuleManifest() const;
	void displayModule() const;

	static const std::string &getHelperString()
	{
		static string help = "path=<target directory> name=<emitter name>";
		return help;
	}

	virtual bool isImmediateDispatchingSupported() const { return false; }

public:
	// remaining public interface
	CRepositoryReceiver();

private:
	// private methods
	// 
	virtual void fileList(NLNET::IModuleProxy *sender, const std::vector < TFileRecord > &files);
	// 
	virtual void beginFile(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 fileSize);
	// 
	virtual void fileData(NLNET::IModuleProxy *sender, const std::string &fileName, const std::string &data);
	// 
	virtual void fileEnd(NLNET::IModuleProxy *sender, const std::string &fileName);

private:
	// private data
	mutable NLMISC::CSString _Manifest;
	
	NLMISC::CSString _EmitterName;			// the name of the emitter module that we are supposed to connect to

	CRepository _Repository;				// container containing info on files in local directory
	NLMISC::CSString _TargetDirectory;		// the directory that we manage
	NLMISC::CSString _Filespec;
 	NLMISC::CSString _LockFilespecs;

	TModuleProxyPtr	_EmitterProxy;
	NLMISC::CVectorSString _FileRequests;	// vector of files that need to be downloaded from RE module
	uint32 _FileRequestsIndex;				// index into file requests vector for file currently being treated

	NLMISC::CSString _CurrentFileName;		// name of the file currently being downloaded
	uint32 _CurrentFileReceived;			// bytes received so far for the current file
	uint32 _CurrentFileExpected;			// bytes expected total for the file
	FILE* _CurrentFileHandle;				// handle for the file currently being downloaded

	
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CRepositoryReceiver, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CRepositoryReceiver, dump, "Dump the current emiter status", "no args")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
};


//-----------------------------------------------------------------------------
// utility routines
//-----------------------------------------------------------------------------

static NLMISC::CSString rrTempFileName(const NLMISC::CSString& targetDirectory, const NLMISC::CSString& emitterName)
{
	return targetDirectory+"rr_"+emitterName+".tmp";
}


//-----------------------------------------------------------------------------
// methods CRepositoryReceiver
//-----------------------------------------------------------------------------

CRepositoryReceiver::CRepositoryReceiver()
{
	CRepositoryReceiverSkel::init();
	_FileRequestsIndex=0;
	_CurrentFileReceived= 0;
	_CurrentFileExpected= 0;
	_CurrentFileHandle= NULL;
}

bool CRepositoryReceiver::initModule(const TParsedCommandLine &initInfo)
{
	bool ret = CModuleBase::initModule(initInfo);

	const TParsedCommandLine *targetDir = initInfo.getParam("path");
	DROP_IF(targetDir == NULL,"path() parameter not found in command line", IModuleManager::getInstance().deleteModule(this); return false;);
	_TargetDirectory = CPath::standardizePath(targetDir->ParamValue, true);
	
	const TParsedCommandLine *name = initInfo.getParam("name");
	DROP_IF(name == NULL,"name() parameter not found in command line", IModuleManager::getInstance().deleteModule(this); return false;);
	_EmitterName= name->ParamValue;

	const TParsedCommandLine *filespec = initInfo.getParam("filespec");
	if (filespec)
		_Filespec = filespec->ParamValue;
	const TParsedCommandLine *lockFilespec = initInfo.getParam("lockfilespec");
	if (lockFilespec)
		_LockFilespecs = lockFilespec->ParamValue;

	nlinfo("RR %s: Initialising with target directory: %s (filespec: %s)",_EmitterName.c_str(),_TargetDirectory.c_str(),_Filespec.c_str());
	ret &= _Repository.init(_EmitterName,_TargetDirectory,_Filespec,_LockFilespecs);

	return ret;
}

void CRepositoryReceiver::onModuleUp(IModuleProxy *module)
{
	if (module->getModuleClassName()=="RepositoryEmitter")
	{
		TParsedCommandLine pci;
		pci.parseParamList(module->getModuleManifest());

		// check whether this RE module corresponds to the emitter name that we're managing
		DROP_IF(pci.getParam("name") == NULL 
				|| pci.getParam("name")->ParamValue != _EmitterName,
				"RR "+_EmitterName+": Ignoring RE Module: "+module->getModuleManifest(),
				return);

		// the names match so make sure there's currently no active emitter
		if (_EmitterProxy != NULL)
		{
			// we have a problem - this is not the first RE module to connect with this name so send it packing
			CRepositoryEmitterProxy re(module);
			re.duplicateModuleError(this);
		}

		// this one's for us so display a nice little message...
		nlinfo("RR %s: Establishing connection with RE module: %s",_EmitterName.c_str(),module->getModuleManifest().c_str());
		_EmitterProxy = module;
	}
}

void CRepositoryReceiver::onModuleDown(IModuleProxy *module)
{
	if (module == _EmitterProxy)
	{
		nlinfo("RR %s: Detecting deconnection of RE module", _EmitterName.c_str());

		// clear out the pending file buffers
		_FileRequests.clear();
		_CurrentFileName.clear();
		_CurrentFileReceived= 0;
		_CurrentFileExpected= 0;

		// if there's a file in the process of being downloaded then get rid of it
		if (_CurrentFileHandle!=NULL)
		{
			fclose(_CurrentFileHandle);
			NLMISC::CFile::deleteFile(rrTempFileName(_TargetDirectory,_EmitterName));
			_CurrentFileHandle= NULL;
		}

		// reset the emitter id to allow a new emitter to connect
		_EmitterProxy = NULL;
	}
}

//void CRepositoryReceiver::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CRepositoryReceiverSkel::onDispatchMessage(sender, msg))
//		return;
//
//	BOMB("Unhandled message "<<msg.getName(), return);
//}
	
NLMISC::CSString CRepositoryReceiver::getState() const
{
	return getName()+" "+(_CurrentFileName.empty()?"IDLE":NLMISC::toString("Downloading %d/%d: %s (%d/%d bytes)",_FileRequestsIndex+1,_FileRequests.size(),_CurrentFileName.c_str(),_CurrentFileReceived,_CurrentFileExpected));
}

NLMISC::CSString CRepositoryReceiver::getName() const
{
	return "RR";
}

std::string CRepositoryReceiver::buildModuleManifest() const
{
	_Manifest = string("path=")+_TargetDirectory+" name="+_EmitterName;
	return _Manifest;
}

NLMISC_CLASS_COMMAND_IMPL(CRepositoryReceiver, dump)
{
	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	log.displayNL("-----------------------------------");
	log.displayNL("Dumping Repository Receiver states :");
	log.displayNL("-----------------------------------");

	if (_EmitterProxy != NULL)
		NLMISC::InfoLog->displayNL("Emitter      : %s (%d)",_EmitterName.c_str(), _EmitterProxy->getModuleProxyId());
	else
		NLMISC::InfoLog->displayNL("Emitter      : not connected");

	NLMISC::InfoLog->displayNL("Repository   : \"%s\" (%d files)",_TargetDirectory.c_str(),_Repository.size());
	if (!_FileRequests.empty())
	{
		NLMISC::InfoLog->displayNL("File Requests: %d/%d",_FileRequestsIndex+1,_FileRequests.size());
		NLMISC::InfoLog->displayNL("Current File : %s (%d/%d bytes)",_CurrentFileName.c_str());
	}

	log.displayNL("-----------------------------------");
	return true;
}

void CRepositoryReceiver::fileList(NLNET::IModuleProxy *sender, const std::vector < TFileRecord > &files)
{
	// make sure the file list comes from our connected emitter
	DROP_IF(sender!= _EmitterProxy, "RR "+_EmitterName+": Received file list from unexpected emitter!!", _EmitterProxy = sender);

	// receiving a map of file names to checksums...
	nlinfo("RR %s: Treating incoming file list with %d entries",_EmitterName.c_str(), files.size());

	// prepare a vector of strings to hold list of files that we want updates for
	CVectorSString requestFiles;

	// run through the entries in the new file list looking for files that don't exist in our local repository
	for (uint32 i=0; i<files.size();++i)
	{
		// normalise the file name in the input list
		NLMISC::CSString fileName= files[i].getFileName();
		DROP_IF(fileName.right(4)==".tmp","Ignoring file list entry with '.tmp' extension: "+fileName,continue);
		DROP_IF(fileName==getRepositoryIndexFileName(_EmitterName),"Ignoring file list entry that clashes with our own index file: "+fileName,continue);

		// get hold of a reference to this file's entry in the index
		CRepository::iterator mapEntry=_Repository.find(fileName);

		// check whether the info in the file description corresponds to the index entry...
		if (mapEntry == _Repository.end() || mapEntry->second.Checksum != files[i].getChecksum())
		{
			// check to see whether this file is already in the pending files queue...
			uint32 j=_FileRequestsIndex+1;
			for (;j<_FileRequests.size();++j)
			{
				if (_FileRequests[j]==files[i].getFileName())
					break;
			}
			if (j<_FileRequests.size())
				continue;

			// the file index entry is not up to date so prepare a request for the new version of the file to be sent down
			nlinfo("RR %s: Require update for file: %s",_EmitterName.c_str(),(_TargetDirectory+fileName).c_str());
			requestFiles.push_back(files[i].getFileName());

			// if the file didn't exist previously then create a new stub in the repository for it
			if (mapEntry == _Repository.end())
			{
				_Repository.addFileStub(files[i].getFileName());
			}
		}
	}

	// if there are no new file requests then just drop out...
	if (requestFiles.empty())
		return;

	// get rid of out of date entries at the front of the old list
	if (_FileRequestsIndex>0)
	{
		for (uint32 i=_FileRequestsIndex;i<_FileRequests.size();++i)
		{
			_FileRequests[i-_FileRequestsIndex]= _FileRequests[i];
		}
		_FileRequests.resize(_FileRequests.size()-_FileRequestsIndex);
		_FileRequestsIndex= 0;
	}

	// display a little message...
	nlinfo("RR %s: Adding %d new file requests to previous %d requests for new request count of %d",_EmitterName.c_str(),requestFiles.size(),_FileRequests.size(),requestFiles.size()+_FileRequests.size());

	// append the new request list to the old list
	for (uint32 i=0;i<requestFiles.size();++i)
	{
		_FileRequests.push_back(requestFiles[i]);
	}

	// if the file request queue was previously empty then send out a first file request message...
	if (_FileRequests.size()== requestFiles.size())
	{
		nlinfo("RR %s: Sending request for file: '%s' to emitter module ",_EmitterName.c_str(),_FileRequests.front().c_str());
		CRepositoryEmitterProxy re(_EmitterProxy);
		re.requestFile(this, _FileRequests.front());

		// create a marker file to say that we're busy and allow software that's using this directory's contents to wait
		// until the download is complete
		NLMISC::CSString("downloading from RE module: "+_EmitterName).writeToFile(rrBusyMarkerFileName(_TargetDirectory,_EmitterName));
	}
}

void CRepositoryReceiver::beginFile(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 fileSize)
{
	// construct the full target file name
	_CurrentFileName = fileName;
	nlinfo("RR %s: Treating received 'begin file': '%s'",_EmitterName.c_str(),_CurrentFileName.c_str());

	// if we need to create a directory for the file to live in then do it
	NLMISC::CFile::createDirectoryTree(NLMISC::CFile::getPath(_TargetDirectory+_CurrentFileName));

	// get rid of the file if it already exists
	if (NLMISC::CFile::fileExists(_TargetDirectory+_CurrentFileName))
	{
		NLMISC::CFile::deleteFile(_TargetDirectory+_CurrentFileName);
		DROP_IF(NLMISC::CFile::fileExists(_TargetDirectory+_CurrentFileName),"Failed to delete out of date file: "+_TargetDirectory+_CurrentFileName,return);
	}

	// make sure temp file is closed etc...
	if (_CurrentFileHandle!=NULL)
	{
		nlwarning("Aborting previous file download (%s) due to start of new file: %s",_FileRequests[_FileRequestsIndex].c_str(),_CurrentFileName.c_str());

		// close and delete the temp file
		fclose(_CurrentFileHandle);
		NLMISC::CFile::deleteFile(rrTempFileName(_TargetDirectory,_EmitterName));
		_CurrentFileHandle= NULL;

		// add the aborted file request back to the end of the queue
		_FileRequests.push_back(_FileRequests[_FileRequestsIndex]);
	}

	// setup the file data size counters
	_CurrentFileReceived= 0;
	_CurrentFileExpected= fileSize;
	
	// open the temp file
	_CurrentFileHandle= fopen(rrTempFileName(_TargetDirectory,_EmitterName).c_str(),"wb");
	BOMB_IF(_CurrentFileHandle==NULL,"Failed to open temporary file for writing: "+rrTempFileName(_TargetDirectory,_EmitterName),return);
}


void CRepositoryReceiver::fileData(NLNET::IModuleProxy *sender, const std::string &fileName, const std::string &data)
{
	// contruct the full target file name
	DROP_IF(_CurrentFileHandle==NULL,"RR "+_EmitterName+": Ignoring file data block for file ("+_CurrentFileName+") because tmp file is not open",return);
	DROP_IF(fileName!=_CurrentFileName,"RR "+_EmitterName+": Ignoring file data block for wrong file (received: "+fileName+") expecting: "+_CurrentFileName,return);

	// write the new file data block to disk
	uint32 bytesWritten= fwrite(&data[0], 1, data.size(), _CurrentFileHandle);
	_CurrentFileReceived+= bytesWritten;

	// if we failed to write the data to disk then drop out with an error
	if (bytesWritten != data.size())
	{
		// close and delete the temp file
		fclose(_CurrentFileHandle);
		NLMISC::CFile::deleteFile(rrTempFileName(_TargetDirectory,_EmitterName));
		_CurrentFileHandle= NULL;

		// add the aborted file request back to the end of the queue
		_FileRequests.push_back(_FileRequests[_FileRequestsIndex]);

		// put up a little message and reset file-related variables
		nlwarning("RR %s: Aborting file download (\"%s\") due to disk write error at %d/%d bytes",_EmitterName.c_str(),_CurrentFileName.c_str(),_CurrentFileReceived,_CurrentFileExpected);
		_CurrentFileName.clear();
		_CurrentFileReceived= 0;
		_CurrentFileExpected= 0;

		// send an ack message to let the emitter know that the file block couldn't be treated
		CRepositoryEmitterProxy re(_EmitterProxy);
		re.fileDataAck(this, _CurrentFileName, false);

		return;
	}

	// send an ack message to let the emitter know that the file block arrived ok
	CRepositoryEmitterProxy re(_EmitterProxy);
	re.fileDataAck(this, _CurrentFileName, true);
}


void CRepositoryReceiver::fileEnd(NLNET::IModuleProxy *sender, const std::string &fileNameRec)
{
	NLMISC::CSString awaitedFileName= _CurrentFileName;

	// update the file request index...
	if (_FileRequestsIndex<_FileRequests.size())
	{
		++_FileRequestsIndex;
	}
	if (_FileRequestsIndex<_FileRequests.size())
	{
		// send a request to the emitter for the next file in our pending files list
		CRepositoryEmitterProxy re(_EmitterProxy);
		re.requestFile(this, _FileRequests[_FileRequestsIndex]);
	}
	else
	{
		// there are no more files to request so switch to 'idle' mode
		_CurrentFileName.clear();
		_FileRequests.clear();
		_FileRequestsIndex=0;

		// delete the 'busy' marker file
		NLMISC::CFile::deleteFile(rrBusyMarkerFileName(_TargetDirectory,_EmitterName));
	}

	// close the tmp file
	DROP_IF(_CurrentFileHandle==NULL,"RR "+_EmitterName+": Ignoring endFile() for file ("+awaitedFileName+") because tmp file is not open",return);
	fclose(_CurrentFileHandle);
	_CurrentFileHandle= NULL;
	
	// make sure the correct number of bytes of data were received and reset the data counter variables...
	BOMB_IF(_CurrentFileExpected!=_CurrentFileReceived,"We just received a file that was the wrong size - panic!",return);
	_CurrentFileExpected=0;
	_CurrentFileReceived=0;

	// contruct the full target file name
	DROP_IF(fileNameRec != awaitedFileName,"RR "+_EmitterName+": Ignoring endFile() for wrong file (received: "+fileNameRec+") expecting: "+awaitedFileName,return);

	// rename the temp file
	// note that the _receiveBeginFile() method will have removed any file that could be in the way...
	bool renameOk= NLMISC::CFile::moveFile((_TargetDirectory+awaitedFileName).c_str(),rrTempFileName(_TargetDirectory,_EmitterName).c_str());
	DROP_IF(!renameOk,"Failed to move tmp file ('"+rrTempFileName(_TargetDirectory,_EmitterName)+"') to : '"+fileNameRec+"'",return);

	// setup the index entry for this file and force an index file write
	_Repository.updateFile(awaitedFileName);
	_Repository.writeIndexFile();
}


//-----------------------------------------------------------------------------
// CRepositoryReceiver registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CRepositoryReceiver, "RepositoryReceiver");

