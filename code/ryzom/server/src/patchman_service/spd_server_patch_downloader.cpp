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

// game share
#include "game_share/utils.h"

// local
#include "spd_server_patch_downloader.h"
#include "patchman_constants.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// methods CServerPatchDownloader
//-----------------------------------------------------------------------------

CServerPatchDownloader::CServerPatchDownloader()
{
}

bool CServerPatchDownloader::initModule(const TParsedCommandLine &initInfo)
{
	// setup a variable to build the init message in...
	NLMISC::CSString logMsg;

	// setup the root directory
	const TParsedCommandLine *targetDir = initInfo.getParam("path");
	if (targetDir != NULL)
	{
		_RootDirectory= CPath::standardizePath(targetDir->ParamValue, true);
	}
	else
	{
		_RootDirectory= NLMISC::CPath::getCurrentPath();
	}
	logMsg+=" Path: "+_RootDirectory;

	// setup the file spec we're intereted in
	CSString fileSpec= "*/*";
	const TParsedCommandLine *fileSpecParam = initInfo.getParam("filespec");
	if (fileSpecParam != NULL)
	{
		fileSpec= fileSpecParam->ParamValue;
	}
	logMsg+=" FileSpec: "+fileSpec;

	// let the base classes do their stuff
	bool ret = CModuleBase::initModule(initInfo);
	CFileReceiver::init(this,fileSpec);
	_AdministeredModuleStub.init(NULL);

	// we're all done so let the world know
	_AdministeredModuleStub.registerProgress("SPD Initialised: "+logMsg);
	_AdministeredModuleStub.broadcastStateInfo();
	return ret;
}

void CServerPatchDownloader::onModuleUp(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CFileReceiver::onModuleUp(module);
}

void CServerPatchDownloader::onModuleDown(IModuleProxy *module)
{
	// allow the base classes a chance to do their stuff
	CFileReceiver::onModuleDown(module);
}

//void CServerPatchDownloader::onProcessModuleMessage(IModuleProxy *sender, const CMessage &msg)
//{
//	if (CFileReceiverSkel::onDispatchMessage(sender, msg))
//		return;
//
//	_AdministeredModuleStub.registerError("ignoring MSG: "+msg.getName());
//	STOP("CServerPatchDownloader::onProcessModuleMessage : received unhandled message '"<<msg.getName()<<"'");
//}
	
void CServerPatchDownloader::onModuleUpdate()
{
	H_AUTO(CServerPatchDownloader_onModuleUpdate);

	// allow the base classes a chance to do their stuff
	CFileReceiver::onModuleUpdate();
}

std::string CServerPatchDownloader::buildModuleManifest() const
{
	return CFileReceiver::getModuleManifest();
}

void CServerPatchDownloader::cbValidateRequestMatches(TFileRequestMatches& requestMatches)
{
}

void CServerPatchDownloader::cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data)
{
	// if the latest info for the file (eg size) doesn't match with the data that we've received then consider that there's a problem and go round again
	SFileInfo fileInfo;
	CFileReceiver::getSingleFileInfo(fileName,fileInfo);
	if (fileInfo.FileSize!=data.size())
	{
		_AdministeredModuleStub.registerError(NLMISC::toString("Re-requesting file because data size (%d != %d): %s",data.size(),fileInfo.FileSize,fileName.c_str()));
		CFileReceiver::requestFile(fileName);
		return;
	}

	// save the recieved file
	CSString fullFileName= _RootDirectory+fileName;
	bool ok= CFileManager::getInstance().save(fullFileName,data);
	if (!ok)
	{
		_AdministeredModuleStub.registerError("failed to save file: "+fileName);
	}

	// make sure the state updates get dispatched properly even if the service is overloaded
	_AdministeredModuleStub.broadcastStateInfo();
}

void CServerPatchDownloader::cbRetryAfterFileDownloadFailure(const NLMISC::CSString& fileName)
{
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchDownloader, dump)
{
	if (args.size()!=0)
		return false;

	NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

	CFileReceiver::dump(log);

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchDownloader, getFile)
{
	if (args.size()!=1)
		return false;

	requestFile(args[0]);

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerPatchDownloader, getFileInfo)
{
	if (args.size()!=1)
		return true;

	dumpFileInfo(args[0],log);

	return true;
}


//-----------------------------------------------------------------------------
// CServerPatchDownloader registration
//-----------------------------------------------------------------------------

NLNET_REGISTER_MODULE_FACTORY(CServerPatchDownloader, "ServerPatchDownloader");

